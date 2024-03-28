/*
 * rk_aiq_algo_camgroup_awb_itf.c
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

#include "rk_aiq_algo_camgroup_abayertnr_itf_v30.h"

#include "abayertnrV30/rk_aiq_abayertnr_algo_itf_v30.h"
#include "abayertnrV30/rk_aiq_abayertnr_algo_v30.h"
#include "iq_parser_v2/bayertnr_uapi_head_v30.h"
#include "rk_aiq_algo_camgroup_types.h"
#include "rk_aiq_types_camgroup_abayertnr_prvt_v30.h"

RKAIQ_BEGIN_DECLARE


static XCamReturn groupAbayertnrV30CreateCtx(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{
    LOGI_ANR("%s enter \n", __FUNCTION__ );

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    CamGroup_AbayertnrV30_Contex_t *abayertnr_group_contex = NULL;
    AlgoCtxInstanceCfgCamGroup *cfgInt = (AlgoCtxInstanceCfgCamGroup*)cfg;


    if(CHECK_ISP_HW_V39()) {
        abayertnr_group_contex = (CamGroup_AbayertnrV30_Contex_t*)malloc(sizeof(CamGroup_AbayertnrV30_Contex_t));
#if ABAYERTNR_USE_JSON_FILE_V30
        Abayertnr_result_V30_t ret_v30 = ABAYERTNRV30_RET_SUCCESS;
        ret_v30 = Abayertnr_Init_V30(&(abayertnr_group_contex->abayertnr_contex_v30), (void *)cfgInt->s_calibv2);
        if(ret_v30 != ABAYERTNRV30_RET_SUCCESS) {
            ret = XCAM_RETURN_ERROR_FAILED;
            LOGE_ANR("%s: Initializaion ANR failed (%d)\n", __FUNCTION__, ret);
        }
#endif
    } else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE_ANR("module_hw_version of abayertnr  is invalid!!!!");
    }

    if(ret != XCAM_RETURN_NO_ERROR) {
        LOGE_ANR("%s: Initializaion group bayertnr failed (%d)\n", __FUNCTION__, ret);
    } else {
        // to do got abayertnrSurrViewClib and initinal paras for for surround view
        abayertnr_group_contex->group_CalibV30.groupMethod = CalibDbV2_CAMGROUP_ABAYERTNRV30_METHOD_MEAN;// to do from json
        abayertnr_group_contex->camera_Num = cfgInt->camIdArrayLen;

        *context = (RkAiqAlgoContext *)(abayertnr_group_contex);

        LOGI_ANR("%s:%d surrViewMethod(1-mean):%d, cameraNum %d \n",
                 __FUNCTION__, __LINE__,
                 abayertnr_group_contex->group_CalibV30.groupMethod,
                 abayertnr_group_contex->camera_Num);
    }

    LOGI_ANR("%s exit ret:%d\n", __FUNCTION__, ret);
    return ret;

}

static XCamReturn groupAbayertnrV30DestroyCtx(RkAiqAlgoContext *context)
{
    LOGI_ANR("%s enter \n", __FUNCTION__ );

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    CamGroup_AbayertnrV30_Contex_t *abayertnr_group_contex = (CamGroup_AbayertnrV30_Contex_t*)context;

    if(CHECK_ISP_HW_V39()) {
        Abayertnr_result_V30_t ret_v30 = ABAYERTNRV30_RET_SUCCESS;
        ret_v30 = Abayertnr_Release_V30(abayertnr_group_contex->abayertnr_contex_v30);
        if(ret_v30 != ABAYERTNRV30_RET_SUCCESS) {
            ret = XCAM_RETURN_ERROR_FAILED;
            LOGE_ANR("%s: Initializaion ANR failed (%d)\n", __FUNCTION__, ret);
        }
    }
    else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE_ANR("module_hw_version of abayertnr is isvalid!!!!");
    }

    if(ret != XCAM_RETURN_NO_ERROR) {
        LOGE_ANR("%s: release abayertnr failed (%d)\n", __FUNCTION__, ret);
    } else {
        free(abayertnr_group_contex);
    }


    LOGI_ANR("%s exit ret:%d\n", __FUNCTION__, ret);
    return ret;
}

static XCamReturn groupAbayertnrV30CombineStats(const RkAiqAlgoCamGroupProcIn* procParaIn,
                                                Abayertnr_Stats_V30_t* stats) {
    if (!procParaIn || !stats) return XCAM_RETURN_ERROR_PARAM;

    uint64_t auto_sigma_count                        = 0;
    uint32_t auto_sigma_calc[ISP39_BAY3D_TNRSIG_NUM] = {0};
    Abayertnr_Stats_V30_t* tnrStats                  = nullptr;
    for (int i = 0; i < procParaIn->arraySize; i++) {
        XCamVideoBuffer* tnrStatsBuf = procParaIn->camgroupParmasArray[i]->abayertnr._tnr_stats;
        if (tnrStatsBuf) {
            tnrStats = (Abayertnr_Stats_V30_t*)tnrStatsBuf->map(tnrStatsBuf);
            if (!tnrStats) {
                LOGE_ANR("tnr stats is null");
                break;
            } else {
                auto_sigma_count += tnrStats->tnr_auto_sigma_count;
                for (uint32_t j = 0; j < ISP39_BAY3D_TNRSIG_NUM; j++)
                    auto_sigma_calc[j] += tnrStats->tnr_auto_sigma_calc[j];
            }
        } else {
            LOGE_ANR("the xcamvideobuffer of tnrStatsBuf is null");
            break;
        }
    }

    if (tnrStats) {
        stats->isValid              = true;
        stats->frame_id             = tnrStats->frame_id;
        stats->tnr_auto_sigma_count = auto_sigma_count / procParaIn->arraySize;
        for (uint32_t i = 0; i < ISP39_BAY3D_TNRSIG_NUM; i++)
            stats->tnr_auto_sigma_calc[i] = auto_sigma_calc[i] / procParaIn->arraySize;

        LOGV_ANR(
            "camgroup tnr stats: frame_id: %d, sigma count: %d, "
            "sigma_calc[0-3]: %d, %d, %d, %d, "
            "sigma_calc[16-20]: %d, %d, %d, %d",
            stats->frame_id, stats->tnr_auto_sigma_count, stats->tnr_auto_sigma_calc[0],
            stats->tnr_auto_sigma_calc[1], stats->tnr_auto_sigma_calc[2],
            stats->tnr_auto_sigma_calc[3], stats->tnr_auto_sigma_calc[16],
            stats->tnr_auto_sigma_calc[17], stats->tnr_auto_sigma_calc[18],
            stats->tnr_auto_sigma_calc[19]);

        return XCAM_RETURN_NO_ERROR;
    } else {
        stats->isValid = false;
    }

    return XCAM_RETURN_ERROR_FAILED;
}

static XCamReturn groupAbayertnrV30Prepare(RkAiqAlgoCom* params)
{
    LOGI_ANR("%s enter \n", __FUNCTION__ );

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    CamGroup_AbayertnrV30_Contex_t * abayertnr_group_contex = (CamGroup_AbayertnrV30_Contex_t *)params->ctx;
    RkAiqAlgoCamGroupPrepare* para = (RkAiqAlgoCamGroupPrepare*)params;

    if(CHECK_ISP_HW_V39()) {
        Abayertnr_Context_V30_t * abayertnr_contex_v30 = abayertnr_group_contex->abayertnr_contex_v30;
        if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )) {
            // todo  update calib pars for surround view
#if ABAYERTNR_USE_JSON_FILE_V30
            void *pCalibdbV30 = (void*)(para->s_calibv2);
#if (RKAIQ_HAVE_BAYERTNR_V30)
            CalibDbV2_BayerTnrV30_t* bayertnr_v30 =
                (CalibDbV2_BayerTnrV30_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCalibdbV30,
                                           bayertnr_v30));


            abayertnr_contex_v30->bayertnr_v30   = *bayertnr_v30;
            abayertnr_contex_v30->isIQParaUpdate = true;
            abayertnr_contex_v30->isReCalculate |= 1;
#endif
#endif
        }
        Abayertnr_Config_V30_t stAbayertnrConfigV30;
        Abayertnr_result_V30_t ret_v30 = ABAYERTNRV30_RET_SUCCESS;
        ret_v30 = Abayertnr_Prepare_V30(abayertnr_contex_v30, &stAbayertnrConfigV30);
        if(ret_v30 != ABAYERTNRV30_RET_SUCCESS) {
            ret = XCAM_RETURN_ERROR_FAILED;
            LOGE_ANR("%s: config ANR failed (%d)\n", __FUNCTION__, ret);
        }
    }
    else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE_ANR("module_hw_version of abayertnr is isvalid!!!!");
    }

    LOGI_ANR("%s exit ret:%d\n", __FUNCTION__, ret);
    return ret;
}

static XCamReturn groupAbayertnrV30Processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    LOGI_ANR("%s enter \n", __FUNCTION__ );
    LOGI_ANR("----------------------------------------------frame_id (%d)----------------------------------------------\n", inparams->frame_id);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoCamGroupProcIn* procParaGroup = (RkAiqAlgoCamGroupProcIn*)inparams;
    RkAiqAlgoCamGroupProcOut* procResParaGroup = (RkAiqAlgoCamGroupProcOut*)outparams;
    CamGroup_AbayertnrV30_Contex_t * abayertnr_group_contex = (CamGroup_AbayertnrV30_Contex_t *)inparams->ctx;
    int deltaIso = 0;

    //method error
    if (abayertnr_group_contex->group_CalibV30.groupMethod <= CalibDbV2_CAMGROUP_ABAYERTNRV30_METHOD_MIN
            ||  abayertnr_group_contex->group_CalibV30.groupMethod >=  CalibDbV2_CAMGROUP_ABAYERTNRV30_METHOD_MAX) {
        return (ret);
    }

    //group empty
    if(procParaGroup->camgroupParmasArray == nullptr) {
        LOGE_ANR("camgroupParmasArray is null");
        return(XCAM_RETURN_ERROR_FAILED);
    }

    //get cur ae exposure
    Abayertnr_ExpInfo_V30_t stExpInfoV30;
    memset(&stExpInfoV30, 0x00, sizeof(stExpInfoV30));
    stExpInfoV30.hdr_mode = 0; //pAnrProcParams->hdr_mode;
    stExpInfoV30.snr_mode = 0;
    for(int i = 0; i < 3; i++) {
        stExpInfoV30.arIso[i] = 50;
        stExpInfoV30.arAGain[i] = 1.0;
        stExpInfoV30.arDGain[i] = 1.0;
        stExpInfoV30.arTime[i] = 0.01;
    }

    stExpInfoV30.blc_ob_predgain = 1.0f;
    if(procParaGroup != NULL) {
        LOGD_ANR(" predgain:%f\n",
                 procParaGroup->stAblcV32_proc_res.isp_ob_predgain);
        stExpInfoV30.blc_ob_predgain = procParaGroup->stAblcV32_proc_res.isp_ob_predgain;

    }

    //merge ae result, iso mean value
    rk_aiq_singlecam_3a_result_t* scam_3a_res = procParaGroup->camgroupParmasArray[0];
    if(scam_3a_res->aec._bEffAecExpValid) {
        RKAiqAecExpInfo_t* pCurExp = &scam_3a_res->aec._effAecExpInfo;
        stExpInfoV30.snr_mode = pCurExp->CISFeature.SNR;
        if((rk_aiq_working_mode_t)procParaGroup->working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
            stExpInfoV30.hdr_mode = 0;
            stExpInfoV30.arAGain[0] = pCurExp->LinearExp.exp_real_params.analog_gain;
            stExpInfoV30.arDGain[0] = pCurExp->LinearExp.exp_real_params.digital_gain;
            stExpInfoV30.arTime[0] = pCurExp->LinearExp.exp_real_params.integration_time;
            if(stExpInfoV30.arAGain[0] < 1.0) {
                stExpInfoV30.arAGain[0] = 1.0;
            }
            if(stExpInfoV30.arDGain[0] < 1.0) {
                stExpInfoV30.arDGain[0] = 1.0;
            }
            if(stExpInfoV30.blc_ob_predgain < 1.0) {
                stExpInfoV30.blc_ob_predgain = 1.0;
            }
            stExpInfoV30.arIso[0] = stExpInfoV30.arAGain[0] * stExpInfoV30.arDGain[0] * stExpInfoV30.blc_ob_predgain * 50;

        } else {
            if(procParaGroup->working_mode == RK_AIQ_ISP_HDR_MODE_2_FRAME_HDR
                    || procParaGroup->working_mode == RK_AIQ_ISP_HDR_MODE_2_LINE_HDR)
                stExpInfoV30.hdr_mode = 1;
            else if (procParaGroup->working_mode == RK_AIQ_ISP_HDR_MODE_3_FRAME_HDR
                     || procParaGroup->working_mode == RK_AIQ_ISP_HDR_MODE_3_LINE_HDR)
                stExpInfoV30.hdr_mode = 2;
            else {
                stExpInfoV30.hdr_mode = 0;
                LOGE_ANR("mode error\n");
            }

            for(int i = 0; i < 3; i++) {
                stExpInfoV30.arAGain[i] = pCurExp->HdrExp[i].exp_real_params.analog_gain;
                stExpInfoV30.arDGain[i] = pCurExp->HdrExp[i].exp_real_params.digital_gain;
                stExpInfoV30.arTime[i] = pCurExp->HdrExp[i].exp_real_params.integration_time;
                if(stExpInfoV30.arAGain[i] < 1.0) {
                    stExpInfoV30.arAGain[i] = 1.0;
                }
                if(stExpInfoV30.arDGain[i] < 1.0) {
                    stExpInfoV30.arDGain[i] = 1.0;
                }
                stExpInfoV30.blc_ob_predgain = 1.0;
                stExpInfoV30.arIso[i] = stExpInfoV30.arAGain[i] * stExpInfoV30.arDGain[i] * 50;
            }

        }
    } else {
        LOGW("fail to get sensor gain form AE module,use default value ");
    }



    if(CHECK_ISP_HW_V39()) {
        Abayertnr_Context_V30_t * abayertnr_contex_v30 = abayertnr_group_contex->abayertnr_contex_v30;
        Abayertnr_ProcResult_V30_t stAbayertnrResultV30;
        stAbayertnrResultV30.st3DFix = scam_3a_res->abayertnr._abayertnr_procRes_v30;
        if(stExpInfoV30.blc_ob_predgain != abayertnr_contex_v30->stExpInfo.blc_ob_predgain) {
            abayertnr_contex_v30->isReCalculate |= 1;
        }
        deltaIso = abs(stExpInfoV30.arIso[stExpInfoV30.hdr_mode] - abayertnr_contex_v30->stExpInfo.arIso[stExpInfoV30.hdr_mode]);
        if(deltaIso > ABAYERTNRV30_RECALCULATE_DELTA_ISO) {
            abayertnr_contex_v30->isReCalculate |= 1;
        }

        // tnr must run evry frame, update sigma and sigma_sq
        abayertnr_contex_v30->isReCalculate |= 1;

        if(abayertnr_contex_v30->isReCalculate) {
            if (groupAbayertnrV30CombineStats(procParaGroup, &abayertnr_contex_v30->stStats) < 0)
                LOGE_ANR("Failed to combine bay3dnr stats\n", __FUNCTION__);

            Abayertnr_result_V30_t ret_v30 = ABAYERTNRV30_RET_SUCCESS;
            ret_v30 = Abayertnr_Process_V30(abayertnr_contex_v30, &stExpInfoV30);
            if(ret_v30 != ABAYERTNRV30_RET_SUCCESS) {
                ret = XCAM_RETURN_ERROR_FAILED;
                LOGE_ANR("%s: processing ANR failed (%d)\n", __FUNCTION__, ret);
            }
            outparams->cfg_update = true;
            LOGD_ANR("recalculate: %d delta_iso:%d \n ", abayertnr_contex_v30->isReCalculate, deltaIso);
        } else {
            outparams->cfg_update = false;
        }
        Abayertnr_GetProcResult_V30(abayertnr_contex_v30, &stAbayertnrResultV30);
        for (int i = 0; i < procResParaGroup->arraySize; i++) {
            *(procResParaGroup->camgroupParmasArray[i]->abayertnr._abayertnr_procRes_v30) = *stAbayertnrResultV30.st3DFix;
            IS_UPDATE_MEM((procResParaGroup->camgroupParmasArray[i]->abayertnr._abayertnr_procRes_v30), procParaGroup->_offset_is_update) =
                outparams->cfg_update;
        }
        abayertnr_contex_v30->isReCalculate = 0;
    }


    else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE_ANR("module_hw_version of abayertnr is isvalid!!!!");
    }

    LOGI_ANR("%s exit\n", __FUNCTION__);
    return ret;
}

RkAiqAlgoDescription g_RkIspAlgoDescCamgroupAbayertnrV30 = {
    .common = {
        .version = RKISP_ALGO_CAMGROUP_ABAYERTNRV30_VERSION,
        .vendor  = RKISP_ALGO_CAMGROUP_ABAYERTNRV30_VENDOR,
        .description = RKISP_ALGO_CAMGROUP_ABAYERTNRV30_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_AMFNR,
        .id      = 0,
        .create_context  = groupAbayertnrV30CreateCtx,
        .destroy_context = groupAbayertnrV30DestroyCtx,
    },
    .prepare = groupAbayertnrV30Prepare,
    .pre_process = NULL,
    .processing = groupAbayertnrV30Processing,
    .post_process = NULL,
};

RKAIQ_END_DECLARE
