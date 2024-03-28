/*
 * rk_aiq_algo_camgroup_awb_itf.c
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

#include "rk_aiq_algo_camgroup_abayertnr_itf_v23.h"
#include "rk_aiq_algo_camgroup_types.h"
#include "rk_aiq_types_camgroup_abayertnr_prvt_v23.h"
#include "abayertnrV23/rk_aiq_abayertnr_algo_itf_v23.h"
#include "abayertnrV23/rk_aiq_abayertnr_algo_v23.h"



RKAIQ_BEGIN_DECLARE


static XCamReturn groupAbayertnrV23CreateCtx(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{
    LOGI_ANR("%s enter \n", __FUNCTION__ );

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    CamGroup_AbayertnrV23_Contex_t *abayertnr_group_contex = NULL;
    AlgoCtxInstanceCfgCamGroup *cfgInt = (AlgoCtxInstanceCfgCamGroup*)cfg;


    if(CHECK_ISP_HW_V32() || CHECK_ISP_HW_V32_LITE()) {
        abayertnr_group_contex = (CamGroup_AbayertnrV23_Contex_t*)malloc(sizeof(CamGroup_AbayertnrV23_Contex_t));
#if ABAYERTNR_USE_JSON_FILE_V23
        Abayertnr_result_V23_t ret_v23 = ABAYERTNRV23_RET_SUCCESS;
        ret_v23 = Abayertnr_Init_V23(&(abayertnr_group_contex->abayertnr_contex_v23), (void *)cfgInt->s_calibv2);
        if(ret_v23 != ABAYERTNRV23_RET_SUCCESS) {
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
        abayertnr_group_contex->group_CalibV23.groupMethod = CalibDbV2_CAMGROUP_ABAYERTNRV23_METHOD_MEAN;// to do from json
        abayertnr_group_contex->camera_Num = cfgInt->camIdArrayLen;

        *context = (RkAiqAlgoContext *)(abayertnr_group_contex);

        LOGI_ANR("%s:%d surrViewMethod(1-mean):%d, cameraNum %d \n",
                 __FUNCTION__, __LINE__,
                 abayertnr_group_contex->group_CalibV23.groupMethod,
                 abayertnr_group_contex->camera_Num);
    }

    LOGI_ANR("%s exit ret:%d\n", __FUNCTION__, ret);
    return ret;

}

static XCamReturn groupAbayertnrV23DestroyCtx(RkAiqAlgoContext *context)
{
    LOGI_ANR("%s enter \n", __FUNCTION__ );

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    CamGroup_AbayertnrV23_Contex_t *abayertnr_group_contex = (CamGroup_AbayertnrV23_Contex_t*)context;

    if(CHECK_ISP_HW_V32() || CHECK_ISP_HW_V32_LITE()) {
        Abayertnr_result_V23_t ret_v23 = ABAYERTNRV23_RET_SUCCESS;
        ret_v23 = Abayertnr_Release_V23(abayertnr_group_contex->abayertnr_contex_v23);
        if(ret_v23 != ABAYERTNRV23_RET_SUCCESS) {
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

static XCamReturn groupAbayertnrV23Prepare(RkAiqAlgoCom* params)
{
    LOGI_ANR("%s enter \n", __FUNCTION__ );

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    CamGroup_AbayertnrV23_Contex_t * abayertnr_group_contex = (CamGroup_AbayertnrV23_Contex_t *)params->ctx;
    RkAiqAlgoCamGroupPrepare* para = (RkAiqAlgoCamGroupPrepare*)params;

    if(CHECK_ISP_HW_V32() || CHECK_ISP_HW_V32_LITE()) {
        Abayertnr_Context_V23_t * abayertnr_contex_v23 = abayertnr_group_contex->abayertnr_contex_v23;
        abayertnr_contex_v23->prepare_type = params->u.prepare.conf_type;
        if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )) {
            // todo  update calib pars for surround view
#if ABAYERTNR_USE_JSON_FILE_V23
            void *pCalibdbV23 = (void*)(para->s_calibv2);
#if (RKAIQ_HAVE_BAYERTNR_V23)
            CalibDbV2_BayerTnrV23_t* bayertnr_v23 =
                (CalibDbV2_BayerTnrV23_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCalibdbV23,
                                           bayertnr_v23));
#else
            CalibDbV2_BayerTnrV23Lite_t* bayertnr_v23 =
                (CalibDbV2_BayerTnrV23Lite_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCalibdbV23,
                                               bayertnr_v23));
#endif

            abayertnr_contex_v23->bayertnr_v23   = *bayertnr_v23;
            abayertnr_contex_v23->isIQParaUpdate = true;
            abayertnr_contex_v23->isReCalculate |= 1;
#endif
        }
        Abayertnr_Config_V23_t stAbayertnrConfigV23;
        Abayertnr_result_V23_t ret_v23 = ABAYERTNRV23_RET_SUCCESS;
        ret_v23 = Abayertnr_Prepare_V23(abayertnr_contex_v23, &stAbayertnrConfigV23);
        if(ret_v23 != ABAYERTNRV23_RET_SUCCESS) {
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

static XCamReturn groupAbayertnrV23Processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    LOGI_ANR("%s enter \n", __FUNCTION__ );
    LOGI_ANR("----------------------------------------------frame_id (%d)----------------------------------------------\n", inparams->frame_id);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoCamGroupProcIn* procParaGroup = (RkAiqAlgoCamGroupProcIn*)inparams;
    RkAiqAlgoCamGroupProcOut* procResParaGroup = (RkAiqAlgoCamGroupProcOut*)outparams;
    CamGroup_AbayertnrV23_Contex_t * abayertnr_group_contex = (CamGroup_AbayertnrV23_Contex_t *)inparams->ctx;
    int deltaIso = 0;

    //method error
    if (abayertnr_group_contex->group_CalibV23.groupMethod <= CalibDbV2_CAMGROUP_ABAYERTNRV23_METHOD_MIN
            ||  abayertnr_group_contex->group_CalibV23.groupMethod >=  CalibDbV2_CAMGROUP_ABAYERTNRV23_METHOD_MAX) {
        return (ret);
    }

    //group empty
    if(procParaGroup == nullptr || procParaGroup->camgroupParmasArray == nullptr) {
        LOGE_ANR("procParaGroup or camgroupParmasArray is null");
        return(XCAM_RETURN_ERROR_FAILED);
    }

    //get cur ae exposure
    Abayertnr_ExpInfo_V23_t stExpInfoV23;
    memset(&stExpInfoV23, 0x00, sizeof(stExpInfoV23));
    stExpInfoV23.hdr_mode = 0; //pAnrProcParams->hdr_mode;
    stExpInfoV23.snr_mode = 0;
    for(int i = 0; i < 3; i++) {
        stExpInfoV23.arIso[i] = 50;
        stExpInfoV23.arAGain[i] = 1.0;
        stExpInfoV23.arDGain[i] = 1.0;
        stExpInfoV23.arTime[i] = 0.01;
    }

    stExpInfoV23.blc_ob_predgain = 1.0f;
    if(procParaGroup != NULL) {
        LOGD_ANR(" predgain:%f\n",
                 procParaGroup->stAblcV32_proc_res.isp_ob_predgain);
        stExpInfoV23.blc_ob_predgain = procParaGroup->stAblcV32_proc_res.isp_ob_predgain;

    }

    //merge ae result, iso mean value
    rk_aiq_singlecam_3a_result_t* scam_3a_res = procParaGroup->camgroupParmasArray[0];
    if(scam_3a_res->aec._bEffAecExpValid) {
        RKAiqAecExpInfo_t* pCurExp = &scam_3a_res->aec._effAecExpInfo;
        stExpInfoV23.snr_mode = pCurExp->CISFeature.SNR;
        if((rk_aiq_working_mode_t)procParaGroup->working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
            stExpInfoV23.hdr_mode = 0;
            stExpInfoV23.arAGain[0] = pCurExp->LinearExp.exp_real_params.analog_gain;
            stExpInfoV23.arDGain[0] = pCurExp->LinearExp.exp_real_params.digital_gain;
            stExpInfoV23.arTime[0] = pCurExp->LinearExp.exp_real_params.integration_time;
            if(stExpInfoV23.arAGain[0] < 1.0) {
                stExpInfoV23.arAGain[0] = 1.0;
            }
            if(stExpInfoV23.arDGain[0] < 1.0) {
                stExpInfoV23.arDGain[0] = 1.0;
            }
            if(stExpInfoV23.blc_ob_predgain < 1.0) {
                stExpInfoV23.blc_ob_predgain = 1.0;
            }
            stExpInfoV23.arIso[0] = stExpInfoV23.arAGain[0] * stExpInfoV23.arDGain[0] * stExpInfoV23.blc_ob_predgain * 50;

        } else {
            if(procParaGroup->working_mode == RK_AIQ_ISP_HDR_MODE_2_FRAME_HDR
                    || procParaGroup->working_mode == RK_AIQ_ISP_HDR_MODE_2_LINE_HDR)
                stExpInfoV23.hdr_mode = 1;
            else if (procParaGroup->working_mode == RK_AIQ_ISP_HDR_MODE_3_FRAME_HDR
                     || procParaGroup->working_mode == RK_AIQ_ISP_HDR_MODE_3_LINE_HDR)
                stExpInfoV23.hdr_mode = 2;
            else {
                stExpInfoV23.hdr_mode = 0;
                LOGE_ANR("mode error\n");
            }

            for(int i = 0; i < 3; i++) {
                stExpInfoV23.arAGain[i] = pCurExp->HdrExp[i].exp_real_params.analog_gain;
                stExpInfoV23.arDGain[i] = pCurExp->HdrExp[i].exp_real_params.digital_gain;
                stExpInfoV23.arTime[i] = pCurExp->HdrExp[i].exp_real_params.integration_time;
                if(stExpInfoV23.arAGain[i] < 1.0) {
                    stExpInfoV23.arAGain[i] = 1.0;
                }
                if(stExpInfoV23.arDGain[i] < 1.0) {
                    stExpInfoV23.arDGain[i] = 1.0;
                }
                stExpInfoV23.blc_ob_predgain = 1.0;
                stExpInfoV23.arIso[i] = stExpInfoV23.arAGain[i] * stExpInfoV23.arDGain[i] * 50;
            }

        }
    } else {
        LOGW("fail to get sensor gain form AE module,use default value ");
    }



    if(CHECK_ISP_HW_V32() || CHECK_ISP_HW_V32_LITE()) {
        Abayertnr_Context_V23_t * abayertnr_contex_v23 = abayertnr_group_contex->abayertnr_contex_v23;
        Abayertnr_ProcResult_V23_t stAbayertnrResultV23;
        RK_Bayertnr_Fix_V23_t st3DFix;
        stAbayertnrResultV23.st3DFix = &st3DFix;
        if(stExpInfoV23.blc_ob_predgain != abayertnr_contex_v23->stExpInfo.blc_ob_predgain) {
            abayertnr_contex_v23->isReCalculate |= 1;
        }
        deltaIso = abs(stExpInfoV23.arIso[stExpInfoV23.hdr_mode] - abayertnr_contex_v23->stExpInfo.arIso[stExpInfoV23.hdr_mode]);
        if(deltaIso > ABAYERTNRV23_RECALCULATE_DELTA_ISO) {
            abayertnr_contex_v23->isReCalculate |= 1;
        }
        if(abayertnr_contex_v23->isReCalculate) {
            Abayertnr_result_V23_t ret_v23 = ABAYERTNRV23_RET_SUCCESS;
            ret_v23 = Abayertnr_Process_V23(abayertnr_contex_v23, &stExpInfoV23);
            if(ret_v23 != ABAYERTNRV23_RET_SUCCESS) {
                ret = XCAM_RETURN_ERROR_FAILED;
                LOGE_ANR("%s: processing ANR failed (%d)\n", __FUNCTION__, ret);
            }
            outparams->cfg_update = true;
            LOGD_ANR("recalculate: %d delta_iso:%d \n ", abayertnr_contex_v23->isReCalculate, deltaIso);
        } else {
            outparams->cfg_update = false;
        }
        Abayertnr_GetProcResult_V23(abayertnr_contex_v23, &stAbayertnrResultV23);
        for (int i = 0; i < procResParaGroup->arraySize; i++) {
            *(procResParaGroup->camgroupParmasArray[i]->abayertnr._abayertnr_procRes_v23) = *stAbayertnrResultV23.st3DFix;
            IS_UPDATE_MEM((procResParaGroup->camgroupParmasArray[i]->abayertnr._abayertnr_procRes_v23), procParaGroup->_offset_is_update) =
                outparams->cfg_update;
        }
        abayertnr_contex_v23->isReCalculate = 0;
    }


    else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE_ANR("module_hw_version of abayertnr is isvalid!!!!");
    }

    LOGI_ANR("%s exit\n", __FUNCTION__);
    return ret;
}

RkAiqAlgoDescription g_RkIspAlgoDescCamgroupAbayertnrV23 = {
    .common = {
        .version = RKISP_ALGO_CAMGROUP_ABAYERTNRV23_VERSION,
        .vendor  = RKISP_ALGO_CAMGROUP_ABAYERTNRV23_VENDOR,
        .description = RKISP_ALGO_CAMGROUP_ABAYERTNRV23_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_AMFNR,
        .id      = 0,
        .create_context  = groupAbayertnrV23CreateCtx,
        .destroy_context = groupAbayertnrV23DestroyCtx,
    },
    .prepare = groupAbayertnrV23Prepare,
    .pre_process = NULL,
    .processing = groupAbayertnrV23Processing,
    .post_process = NULL,
};

RKAIQ_END_DECLARE
