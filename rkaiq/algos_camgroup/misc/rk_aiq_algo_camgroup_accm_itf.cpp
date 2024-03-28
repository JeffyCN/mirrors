/*
 * rk_aiq_algo_camgroup_accm_itf.c
 *
 *  Copyright (c) 2021 Rockchip Corporation
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

#include "accm/rk_aiq_accm_algo.h"
#include "accm/rk_aiq_algo_accm_itf.h"
#include "misc/rk_aiq_algo_camgroup_misc_itf.h"
#include "rk_aiq_algo_camgroup_types.h"
#include "xcam_log.h"

RKAIQ_BEGIN_DECLARE

static XCamReturn
create_context(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{
    LOG1_ACCM( "%s: (enter)\n", __FUNCTION__);

    AlgoCtxInstanceCfgCamGroup *cfgInt = (AlgoCtxInstanceCfgCamGroup*)cfg;
    RkAiqAlgoContext *ctx = new RkAiqAlgoContext();
    if (ctx == NULL) {
        LOGE_ACCM( "%s: create camgroup accm context fail!\n", __FUNCTION__);
        return XCAM_RETURN_ERROR_MEM;
    }
    AccmInit(&ctx->accm_para, cfgInt->s_calibv2);
    *context = ctx;
    LOG1_ACCM( "%s: (exit)\n", __FUNCTION__);
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
destroy_context(RkAiqAlgoContext *context)
{
    LOG1_ACCM( "%s: (enter)\n", __FUNCTION__);

    AccmRelease((accm_handle_t)context->accm_para);
    delete context;
    context = NULL;
    LOG1_ACCM( "%s: (exit)\n", __FUNCTION__);
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
prepare(RkAiqAlgoCom* params)
{
    LOG1_ACCM( "%s: (enter)\n", __FUNCTION__);
    accm_handle_t hAccm = (accm_handle_t)(params->ctx->accm_para);

    RkAiqAlgoCamGroupPrepare *para = (RkAiqAlgoCamGroupPrepare *)params;
    hAccm->accmSwInfo.prepare_type = params->u.prepare.conf_type;
    if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )) {
#if RKAIQ_HAVE_CCM_V1
        hAccm->ccm_v1 = (CalibDbV2_Ccm_Para_V2_t*)(CALIBDBV2_GET_MODULE_PTR(
                                (CamCalibDbV2Context_t*)(para->s_calibv2), ccm_calib));
#endif

#if RKAIQ_HAVE_CCM_V2
        hAccm->ccm_v2 = (CalibDbV2_Ccm_Para_V32_t*)(CALIBDBV2_GET_MODULE_PTR(
                                (CamCalibDbV2Context_t*)(para->s_calibv2), ccm_calib_v2));
#endif

#if RKAIQ_HAVE_CCM_V3
        hAccm->ccm_v3 = (CalibDbV2_Ccm_Para_V39_t*)(CALIBDBV2_GET_MODULE_PTR(
                            (CamCalibDbV2Context_t*)(para->s_calibv2), ccm_calib_v2));
#endif
    }
    AccmPrepare((accm_handle_t)(params->ctx->accm_para));

    LOG1_ACCM( "%s: (exit)\n", __FUNCTION__);
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    LOG1_ACCM( "%s: (enter)\n", __FUNCTION__);

    RkAiqAlgoCamGroupProcIn* procParaGroup = (RkAiqAlgoCamGroupProcIn*)inparams;
    RkAiqAlgoCamGroupProcOut* procResParaGroup = (RkAiqAlgoCamGroupProcOut*)outparams;
    accm_handle_t hAccm = (accm_handle_t)(inparams->ctx->accm_para);

    hAccm->isReCal_ = hAccm->isReCal_ ||
            (hAccm->accmSwInfo.grayMode != procParaGroup->_gray_mode);
    hAccm->accmSwInfo.grayMode = procParaGroup->_gray_mode;

    if (!procParaGroup->arraySize) {
        LOG1_ACCM( "%s: do nothing \n", __FUNCTION__);
        return XCAM_RETURN_NO_ERROR;
    }

    rk_aiq_singlecam_3a_result_t* scam_3a_res = procParaGroup->camgroupParmasArray[0];
    RkAiqAlgoProcResAwbShared_t* awb_proc_res = &scam_3a_res->awb._awbProcRes;
    if (awb_proc_res) {
        RkAiqAlgoProcResAwbShared_t* awb_res = (RkAiqAlgoProcResAwbShared_t*)awb_proc_res;
        if(awb_res) {
            if(awb_res->awb_gain_algo.grgain < DIVMIN ||
                    awb_res->awb_gain_algo.gbgain < DIVMIN ) {
                LOGW_ACCM("get wrong awb gain from AWB module ,use default value ");
            } else {
                hAccm->accmSwInfo.awbGain[0] =
                    awb_res->awb_gain_algo.rgain / awb_res->awb_gain_algo.grgain;

                hAccm->accmSwInfo.awbGain[1] =
                    awb_res->awb_gain_algo.bgain / awb_res->awb_gain_algo.gbgain;
            }
            hAccm->accmSwInfo.awbIIRDampCoef = awb_res->awb_smooth_factor;
            hAccm->accmSwInfo.varianceLuma = awb_res->varianceLuma;
            hAccm->accmSwInfo.awbConverged = awb_res->awbConverged;
        } else {
            LOGW_ACCM("fail to get awb gain form AWB module,use default value ");
        }
    } else {
        LOGW_ACCM("fail to get awb gain form AWB module,use default value ");
    }

    if(scam_3a_res->aec._bEffAecExpValid) {
        RKAiqAecExpInfo_t* pCurExp = &scam_3a_res->aec._effAecExpInfo;
        if((rk_aiq_working_mode_t)procParaGroup->working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
            hAccm->accmSwInfo.sensorGain = pCurExp->LinearExp.exp_real_params.analog_gain
                                           * pCurExp->LinearExp.exp_real_params.digital_gain
                                           * pCurExp->LinearExp.exp_real_params.isp_dgain;
        } else if((rk_aiq_working_mode_t)procParaGroup->working_mode >= RK_AIQ_WORKING_MODE_ISP_HDR2
                  && (rk_aiq_working_mode_t)procParaGroup->working_mode < RK_AIQ_WORKING_MODE_ISP_HDR3)  {
            LOGD_ACCM("sensor gain choose from second hdr frame for accm");
            hAccm->accmSwInfo.sensorGain = pCurExp->HdrExp[1].exp_real_params.analog_gain
                                           * pCurExp->HdrExp[1].exp_real_params.digital_gain
                                           * pCurExp->HdrExp[1].exp_real_params.isp_dgain;
        } else if((rk_aiq_working_mode_t)procParaGroup->working_mode >= RK_AIQ_WORKING_MODE_ISP_HDR2
                  && (rk_aiq_working_mode_t)procParaGroup->working_mode >= RK_AIQ_WORKING_MODE_ISP_HDR3)  {
            LOGD_ACCM("sensor gain choose from third hdr frame for accm");
            hAccm->accmSwInfo.sensorGain = pCurExp->HdrExp[2].exp_real_params.analog_gain
                                           * pCurExp->HdrExp[2].exp_real_params.digital_gain
                                           * pCurExp->HdrExp[2].exp_real_params.isp_dgain;
        } else {
            LOGW_ACCM("working_mode (%d) is invaild ,fail to get sensor gain form AE module,use default value ",
                      procParaGroup->working_mode);
        }
    } else {
        LOGW_ACCM("fail to get sensor gain form AE module,use default value ");
    }

    //LOGI_ACCM( "%s accm_proc_com.u.init:%d \n", __FUNCTION__, inparams->u.proc.init);
    LOGD_ACCM( "%s: awbIIRDampCoef:%f\n", __FUNCTION__, hAccm->accmSwInfo.awbIIRDampCoef);

    AccmConfig(hAccm);
    outparams->cfg_update = hAccm->isReCal_;
    for (int i = 0; i < procResParaGroup->arraySize; i++) {
#if defined(ISP_HW_V39) || defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
        if (outparams->cfg_update) {
            *(procResParaGroup->camgroupParmasArray[i]->accm._ccmCfg_v2) =
                hAccm->ccmHwConf_v2;
        }
        //TODO
        IS_UPDATE_MEM((procResParaGroup->camgroupParmasArray[i]->accm._ccmCfg_v2), procParaGroup->_offset_is_update) =
            outparams->cfg_update;
#else
        if (outparams->cfg_update) {
            *(procResParaGroup->camgroupParmasArray[i]->accm._ccmCfg) =
                hAccm->ccmHwConf;
        }
        //TODO
        IS_UPDATE_MEM((procResParaGroup->camgroupParmasArray[i]->accm._ccmCfg), procParaGroup->_offset_is_update) =
            outparams->cfg_update;
#endif
    }
    hAccm->isReCal_ = false;
    LOG1_ACCM( "%s: (exit)\n", __FUNCTION__);
    return XCAM_RETURN_NO_ERROR;
}

RkAiqAlgoDescription g_RkIspAlgoDescCamgroupAccm = {
    .common = {
        .version = RKISP_ALGO_ACCM_VERSION,
        .vendor  = RKISP_ALGO_ACCM_VENDOR,
        .description = RKISP_ALGO_ACCM_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_ACCM,
        .id      = 0,
        .create_context  = create_context,
        .destroy_context = destroy_context,
    },
    .prepare = prepare,
    .pre_process = NULL,
    .processing = processing,
    .post_process = NULL,
};

RKAIQ_END_DECLARE
