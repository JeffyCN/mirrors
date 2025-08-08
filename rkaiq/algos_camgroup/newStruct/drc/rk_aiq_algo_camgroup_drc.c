/*
 * rk_aiq_algo_camgroup_drc.c
 *
 *  Copyright (c) 2024 Rockchip Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "c_base/aiq_base.h"
#include "newStruct/drc/drc_types_prvt.h"
#include "newStruct/rk_aiq_algo_camgroup_common.h"
#include "rk_aiq_algo_camgroup_types.h"

#define DEFAULT_RECALCULATE_DELTA_ISO (0.01)

typedef DrcContext_t DrcGroupContext_t;

static void DrcStatsConfig(RkAiqAlgoCamGroupProcIn* procParaGroup,
                           rkisp_adrc_stats_t* drc_stats) {

    if (procParaGroup->camgroupParmasArray[0]->aec.aec_stats_v25) {
        drc_stats->stats_true = true;
        bool isHDR                           = false;
        if (procParaGroup->working_mode >= RK_AIQ_WORKING_MODE_ISP_HDR2)
            isHDR = true;

        for (int i = 0; i < DRC_AE_HIST_BIN_NUM; i++) {
            if (isHDR) {
                drc_stats->aeHiatBins[i] =
                    procParaGroup->camgroupParmasArray[0]->aec.aec_stats_v25->ae_data.entityGroup.entities.entity3.hist
                        .hw_ae_histBin_val[i];
            } else {
                drc_stats->aeHiatBins[i] =
                    procParaGroup->camgroupParmasArray[0]->aec.aec_stats_v25->ae_data.entityGroup.entities.entity0.hist
                        .hw_ae_histBin_val[i];
            }
            int hist_total_num = 0;
            for (int i = 0; i < DRC_AE_HIST_BIN_NUM; ++i)
                hist_total_num += drc_stats->aeHiatBins[i];
            drc_stats->ae_hist_total_num = hist_total_num;
        }
    }
    else {
        drc_stats->stats_true        = false;
        drc_stats->ae_hist_total_num = 0;
        for (int i = 0; i < DRC_AE_HIST_BIN_NUM; i++) drc_stats->aeHiatBins[i] = 0;
    }

#if 0
    printf("%s: frame_id:%d stats_true:%d ae_hist_total_num:%d\n", __FUNCTION__,
           drc_stats->frame_id, drc_stats->stats_true,
           drc_stats->ae_hist_total_num);
    for (int m = 0; m < 16; m++) {
        printf("%s: aeHiatBins[%d]: %d ", __FUNCTION__, m,
               drc_stats->aeHiatBins[16 * m]);
        for (int n = 1; n < 15; n++)
            printf("%d ", drc_stats->aeHiatBins[16 * m + n]);
        printf("%d \n", drc_stats->aeHiatBins[16 * m + 15]);
    }
#endif
}

static XCamReturn groupDrcProcessing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    LOGD_ATMO("%s enter", __FUNCTION__);

    DrcGroupContext_t* pDrcGroupCtx            = (DrcGroupContext_t*)inparams->ctx;
    RkAiqAlgoCamGroupProcIn* procParaGroup     = (RkAiqAlgoCamGroupProcIn*)inparams;
    pDrcGroupCtx->FrameID                      = inparams->frame_id;
    RkAiqAlgoCamGroupProcOut* procResParaGroup = (RkAiqAlgoCamGroupProcOut*)outparams;
    pDrcGroupCtx->blc_ob_enable                = procParaGroup->stAblcV32_proc_res.blc_ob_enable;
    pDrcGroupCtx->isp_ob_predgain              = procParaGroup->stAblcV32_proc_res.isp_ob_predgain;
    drc_api_attrib_t* drc_attrib               = pDrcGroupCtx->drc_attrib;

    // TODO
    // pDrcGroupCtx->compr_bit = procParaGroup->camgroupParmasArray[0]->aec.compr_bit;
    // group empty
    if (procParaGroup == NULL || procParaGroup->camgroupParmasArray == NULL) {
        LOGE_ATMO("procParaGroup or camgroupParmasArray is null");
        return (XCAM_RETURN_ERROR_FAILED);
    }

    // skip group algo if in MANUAL mode
    if (pDrcGroupCtx->drc_attrib->opMode != RK_AIQ_OP_MODE_AUTO) {
        return XCAM_RETURN_NO_ERROR;
    }

    if (pDrcGroupCtx->isCapture) {
        LOGD_ATMO("%s: It's capturing, using pre frame params\n", __func__);
        pDrcGroupCtx->isCapture = false;
        return XCAM_RETURN_NO_ERROR;
    }

    if (procParaGroup->working_mode < RK_AIQ_WORKING_MODE_ISP_HDR2)
        pDrcGroupCtx->FrameNumber = LINEAR_NUM;
    else if (procParaGroup->working_mode < RK_AIQ_WORKING_MODE_ISP_HDR3 &&
             procParaGroup->working_mode >= RK_AIQ_WORKING_MODE_ISP_HDR2)
        pDrcGroupCtx->FrameNumber = HDR_2X_NUM;
    else
        pDrcGroupCtx->FrameNumber = HDR_3X_NUM;

    if (pDrcGroupCtx->FrameNumber == HDR_2X_NUM || pDrcGroupCtx->FrameNumber == HDR_3X_NUM ||
        pDrcGroupCtx->FrameNumber == SENSOR_MGE)
        outparams->en = true;
    else if (pDrcGroupCtx->FrameNumber == LINEAR_NUM) {
        if (pDrcGroupCtx->blc_ob_enable)
            outparams->en = true;
        else {
            if (drc_attrib->opMode == RK_AIQ_OP_MODE_AUTO) {
                outparams->en = drc_attrib->en;
            } else {
                outparams->en = false;
            }
        }
    }

    if (outparams->en) {
        RkAiqAlgoProcResAeShared_t* pAEProcRes =
            &procParaGroup->camgroupParmasArray[0]->aec._aeProcRes;
        pDrcGroupCtx->NextData.AEData.LongFrmMode = pAEProcRes->LongFrmMode;
    }
    RKAiqAecExpInfo_t pExpInfo = procParaGroup->camgroupParmasArray[0]->aec._effAecExpInfo;
    if (inparams->u.proc.init) {
        pExpInfo = procParaGroup->camgroupParmasArray[0]->aec.exp_tbl[0];
    }

    if (pDrcGroupCtx->FrameNumber == LINEAR_NUM) {
        pDrcGroupCtx->NextData.AEData.SExpo =
            pExpInfo.LinearExp.exp_real_params.analog_gain *
            pExpInfo.LinearExp.exp_real_params.digital_gain *
            pExpInfo.LinearExp.exp_real_params.isp_dgain *
            pExpInfo.LinearExp.exp_real_params.integration_time;
        pDrcGroupCtx->NextData.AEData.MExpo = pDrcGroupCtx->NextData.AEData.SExpo;
        pDrcGroupCtx->NextData.AEData.LExpo = pDrcGroupCtx->NextData.AEData.SExpo;
    }
    else if (pDrcGroupCtx->FrameNumber == HDR_2X_NUM) {
        pDrcGroupCtx->NextData.AEData.SExpo = pExpInfo.HdrExp[0]
                                                  .exp_real_params.analog_gain *
                                              pExpInfo.HdrExp[0]
                                                  .exp_real_params.digital_gain *
                                              pExpInfo.HdrExp[0]
                                                  .exp_real_params.isp_dgain *
                                              pExpInfo.HdrExp[0]
                                                  .exp_real_params.integration_time;
        pDrcGroupCtx->NextData.AEData.MExpo = pExpInfo.HdrExp[1]
                                                  .exp_real_params.analog_gain *
                                              pExpInfo.HdrExp[1]
                                                  .exp_real_params.digital_gain *
                                              pExpInfo.HdrExp[1]
                                                  .exp_real_params.isp_dgain *
                                              pExpInfo.HdrExp[1]
                                                  .exp_real_params.integration_time;
        pDrcGroupCtx->NextData.AEData.LExpo = pDrcGroupCtx->NextData.AEData.MExpo;
    } else if (pDrcGroupCtx->FrameNumber == HDR_3X_NUM) {
        pDrcGroupCtx->NextData.AEData.SExpo = pExpInfo.HdrExp[0]
                                                  .exp_real_params.analog_gain *
                                              pExpInfo.HdrExp[0]
                                                  .exp_real_params.digital_gain *
                                              pExpInfo.HdrExp[0]
                                                  .exp_real_params.isp_dgain *
                                              pExpInfo.HdrExp[0]
                                                  .exp_real_params.integration_time;
        pDrcGroupCtx->NextData.AEData.MExpo = pExpInfo.HdrExp[1]
                                                  .exp_real_params.analog_gain *
                                              pExpInfo.HdrExp[1]
                                                  .exp_real_params.digital_gain *
                                              pExpInfo.HdrExp[1]
                                                  .exp_real_params.isp_dgain *
                                              pExpInfo.HdrExp[1]
                                                  .exp_real_params.integration_time;
        pDrcGroupCtx->NextData.AEData.LExpo = pExpInfo.HdrExp[2]
                                                  .exp_real_params.analog_gain *
                                              pExpInfo.HdrExp[2]
                                                  .exp_real_params.digital_gain *
                                              pExpInfo.HdrExp[2]
                                                  .exp_real_params.isp_dgain *
                                              pExpInfo.HdrExp[2]
                                                  .exp_real_params.integration_time;
    } else if (pDrcGroupCtx->FrameNumber == SENSOR_MGE) {
        pDrcGroupCtx->NextData.AEData.MExpo =
            pExpInfo.LinearExp.exp_real_params.analog_gain *
            pExpInfo.LinearExp.exp_real_params.digital_gain *
            pExpInfo.LinearExp.exp_real_params.isp_dgain *
            pExpInfo.LinearExp.exp_real_params.integration_time;
        pDrcGroupCtx->NextData.AEData.LExpo = pDrcGroupCtx->NextData.AEData.MExpo;
        pDrcGroupCtx->NextData.AEData.SExpo =
            pDrcGroupCtx->NextData.AEData.MExpo /
            pow(2.0f, (float)(pDrcGroupCtx->compr_bit - ISP_HDR_BIT_NUM_MIN));
    }
    if (pDrcGroupCtx->NextData.AEData.SExpo > FLT_EPSILON) {
        pDrcGroupCtx->NextData.AEData.L2S_Ratio =
            pDrcGroupCtx->NextData.AEData.LExpo / pDrcGroupCtx->NextData.AEData.SExpo;
        if (pDrcGroupCtx->NextData.AEData.L2S_Ratio < RATIO_DEFAULT) {
            LOGE_ATMO("%s: Next L2S_Ratio:%f is less than 1.0x, clip to 1.0x!!!\n", __FUNCTION__,
                      pDrcGroupCtx->NextData.AEData.L2S_Ratio);
            pDrcGroupCtx->NextData.AEData.L2S_Ratio = RATIO_DEFAULT;
        }
        pDrcGroupCtx->NextData.AEData.M2S_Ratio =
            pDrcGroupCtx->NextData.AEData.MExpo / pDrcGroupCtx->NextData.AEData.SExpo;
        if (pDrcGroupCtx->NextData.AEData.M2S_Ratio < RATIO_DEFAULT) {
            LOGE_ATMO("%s: Next M2S_Ratio:%f is less than 1.0x, clip to 1.0x!!!\n", __FUNCTION__,
                      pDrcGroupCtx->NextData.AEData.M2S_Ratio);
            pDrcGroupCtx->NextData.AEData.M2S_Ratio = RATIO_DEFAULT;
        }
    } else
        LOGE_ATMO("%s: Next Short frame for drc expo sync is %f!!!\n", __FUNCTION__,
                  pDrcGroupCtx->NextData.AEData.SExpo);
    if (pDrcGroupCtx->NextData.AEData.MExpo > FLT_EPSILON) {
        pDrcGroupCtx->NextData.AEData.L2M_Ratio =
            pDrcGroupCtx->NextData.AEData.LExpo / pDrcGroupCtx->NextData.AEData.MExpo;
        if (pDrcGroupCtx->NextData.AEData.L2M_Ratio < RATIO_DEFAULT) {
            LOGE_ATMO("%s: Next L2M_Ratio:%f is less than 1.0x, clip to 1.0x!!!\n", __FUNCTION__,
                      pDrcGroupCtx->NextData.AEData.L2M_Ratio);
            pDrcGroupCtx->NextData.AEData.L2M_Ratio = RATIO_DEFAULT;
        }
    } else
        LOGE_ATMO("%s: Next Midlle frame for drc expo sync is %f!!!\n", __FUNCTION__,
                  pDrcGroupCtx->NextData.AEData.MExpo);
    // clip for long frame mode
    if (pDrcGroupCtx->NextData.AEData.LongFrmMode) {
        pDrcGroupCtx->NextData.AEData.L2S_Ratio = LONG_FRAME_MODE_RATIO;
        pDrcGroupCtx->NextData.AEData.M2S_Ratio = LONG_FRAME_MODE_RATIO;
        pDrcGroupCtx->NextData.AEData.L2M_Ratio = LONG_FRAME_MODE_RATIO;
    }
    // clip L2M_ratio to 32x
    if (pDrcGroupCtx->NextData.AEData.L2M_Ratio > AE_RATIO_L2M_MAX) {
        LOGE_ATMO("%s: Next L2M_ratio:%f out of range, clip to 32.0x!!!\n", __FUNCTION__,
                  pDrcGroupCtx->NextData.AEData.L2M_Ratio);
        pDrcGroupCtx->NextData.AEData.L2M_Ratio = AE_RATIO_L2M_MAX;
    }
    // clip L2S_ratio
    if (pDrcGroupCtx->NextData.AEData.L2S_Ratio > AE_RATIO_MAX) {
        LOGE_ATMO("%s: Next L2S_Ratio:%f out of range, clip to 256.0x!!!\n", __FUNCTION__,
                  pDrcGroupCtx->NextData.AEData.L2S_Ratio);
        pDrcGroupCtx->NextData.AEData.L2S_Ratio = AE_RATIO_MAX;
    }
    // clip L2M_ratio and M2S_Ratio
    if (pDrcGroupCtx->NextData.AEData.L2M_Ratio * pDrcGroupCtx->NextData.AEData.M2S_Ratio >
        AE_RATIO_MAX) {
        LOGE_ATMO(
            "%s: Next L2M_Ratio*M2S_Ratio:%f out of range, clip to 256.0x!!!\n", __FUNCTION__,
            pDrcGroupCtx->NextData.AEData.L2M_Ratio * pDrcGroupCtx->NextData.AEData.M2S_Ratio);
        pDrcGroupCtx->NextData.AEData.M2S_Ratio =
            AE_RATIO_MAX / pDrcGroupCtx->NextData.AEData.L2M_Ratio;
    }

    int iso = 50;
    rk_aiq_singlecam_3a_result_t* scam_3a_res = procParaGroup->camgroupParmasArray[0];
    if (scam_3a_res->aec._bEffAecExpValid) {
        iso = scam_3a_res->hdrIso;
    }

    rkisp_adrc_stats_t drc_stats;
    memset(&drc_stats, 0, sizeof(rkisp_adrc_stats_t));
    DrcStatsConfig(procParaGroup, &drc_stats);
    pDrcGroupCtx->drc_stats = &drc_stats;

    if (procParaGroup->attribUpdated) {
        LOGI("%s attribUpdated", __func__);
        pDrcGroupCtx->isReCal_ = true;
    }

    int delta_iso = abs(iso - pDrcGroupCtx->iso);
    if (delta_iso > DEFAULT_RECALCULATE_DELTA_ISO ||
        !procParaGroup->camgroupParmasArray[0]->aec._aeProcRes.IsConverged) 
        pDrcGroupCtx->isReCal_ = true;

#if RKAIQ_HAVE_DRC_V20
    bool isIIRReclac = false;
    if (pDrcGroupCtx->CurrData.autoCurveIIRParams.sw_drcT_drcCurve_mode == adrc_auto_mode) {
        if (procParaGroup->camgroupParmasArray[0]->aec._aeProcRes.IsConverged) {
            pDrcGroupCtx->CurrData.autoCurveIIRParams.reCalcNum++;
            if (pDrcGroupCtx->CurrData.autoCurveIIRParams.reCalcNum <=
                2 * pDrcGroupCtx->CurrData.autoCurveIIRParams.sw_drcT_iirFrm_maxLimit)
                isIIRReclac = true;
        } else {
            pDrcGroupCtx->CurrData.autoCurveIIRParams.reCalcNum = 0;
        }
    } else {
        pDrcGroupCtx->CurrData.autoCurveIIRParams.reCalcNum = 0;
    }
    pDrcGroupCtx->isReCal_ = pDrcGroupCtx->isReCal_ || isIIRReclac;
#endif

    rk_aiq_isp_drc_v39_t* drc_param = procResParaGroup->camgroupParmasArray[0]->drc;

    if (pDrcGroupCtx->isReCal_) {
#if RKAIQ_HAVE_DRC_V12
        DrcSelectParam(pDrcGroupCtx, &drc_param->result, iso);
        DrcExpoParaProcessing(pDrcGroupCtx, &drc_param->result);
#endif
#if RKAIQ_HAVE_DRC_V20
        drc_param->L2S_Ratio = pDrcGroupCtx->NextData.AEData.L2S_Ratio;
        drc_param->compr_bit = pDrcGroupCtx->compr_bit;
        DrcSelectParam(pDrcGroupCtx, &drc_param->drc_param, &drc_param->trans_attr.stMan.sta, iso);
#endif
        outparams->cfg_update = true;
    } else {
        outparams->cfg_update = false;
    }

    void* gp_ptrs[procResParaGroup->arraySize];
    int gp_size = sizeof(*procResParaGroup->camgroupParmasArray[0]->drc);
    for (int i = 0; i < procResParaGroup->arraySize; i++)
        gp_ptrs[i] = procResParaGroup->camgroupParmasArray[i]->drc;

    algo_camgroup_update_results(inparams, outparams, gp_ptrs, gp_size);

    pDrcGroupCtx->iso                         = iso;
    pDrcGroupCtx->isReCal_                    = false;
    pDrcGroupCtx->CurrData.AEData.LongFrmMode = pDrcGroupCtx->NextData.AEData.LongFrmMode;
    pDrcGroupCtx->CurrData.AEData.L2M_Ratio   = pDrcGroupCtx->NextData.AEData.L2M_Ratio;
    pDrcGroupCtx->CurrData.AEData.M2S_Ratio   = pDrcGroupCtx->NextData.AEData.M2S_Ratio;
    pDrcGroupCtx->CurrData.AEData.L2S_Ratio = pDrcGroupCtx->NextData.AEData.L2S_Ratio;
    LOGD_ATMO("%s exit\n", __FUNCTION__);
    return ret;
}

#define RKISP_ALGO_CAMGROUP_DRC_VERSION     "v0.0.1"
#define RKISP_ALGO_CAMGROUP_DRC_VENDOR      "Rockchip"
#define RKISP_ALGO_CAMGROUP_DRC_DESCRIPTION "Rockchip Drc camgroup algo for ISP2.0"

RkAiqAlgoDescription g_RkIspAlgoDescCamgroupDrc = {
    .common =
        {
            .version         = RKISP_ALGO_CAMGROUP_DRC_VERSION,
            .vendor          = RKISP_ALGO_CAMGROUP_DRC_VENDOR,
            .description     = RKISP_ALGO_CAMGROUP_DRC_DESCRIPTION,
            .type            = RK_AIQ_ALGO_TYPE_ADRC,
            .id              = 0,
            .create_context  = algo_camgroup_CreateCtx,
            .destroy_context = algo_camgroup_DestroyCtx,
        },
    .prepare      = algo_camgroup_Prepare,
    .pre_process  = NULL,
    .processing   = groupDrcProcessing,
    .post_process = NULL,
};
