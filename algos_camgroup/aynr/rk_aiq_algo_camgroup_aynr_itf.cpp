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

#include "rk_aiq_algo_camgroup_aynr_itf.h"
#include "rk_aiq_algo_camgroup_types.h"
#include "rk_aiq_types_camgroup_aynr_prvt.h"
#include "aynr3/rk_aiq_aynr_algo_itf_v3.h"
#include "aynr3/rk_aiq_aynr_algo_v3.h"
#include "aynr2/rk_aiq_aynr_algo_itf_v2.h"
#include "aynr2/rk_aiq_aynr_algo_v2.h"



RKAIQ_BEGIN_DECLARE

static aynr_hardware_version_t g_aynr_hw_ver = AYNR_HARDWARE_V3;

static XCamReturn groupAynrCreateCtx(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{
    LOGI_ANR("%s enter \n", __FUNCTION__ );

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    CamGroup_Aynr_Contex_t *aynr_group_contex = NULL;
    AlgoCtxInstanceCfgCamGroup *cfgInt = (AlgoCtxInstanceCfgCamGroup*)cfg;
    //g_aynr_hw_ver = (aynr_hardware_version_t)(cfgInt->cfg_com.module_hw_version);

    if (CHECK_ISP_HW_V21()) {
        g_aynr_hw_ver = AYNR_HARDWARE_V2;
    } else if(CHECK_ISP_HW_V3X()) {
        g_aynr_hw_ver = AYNR_HARDWARE_V3;
    } else {
        g_aynr_hw_ver = AYNR_HARDWARE_MIN;
    }

    if(g_aynr_hw_ver == AYNR_HARDWARE_V3) {
        aynr_group_contex = (CamGroup_Aynr_Contex_t*)malloc(sizeof(CamGroup_Aynr_Contex_t));
#if AYNR_USE_JSON_FILE_V3
        Aynr_result_V3_t ret_v3 = AYNRV3_RET_SUCCESS;
        ret_v3 = Aynr_Init_V3(&(aynr_group_contex->aynr_contex_v3), (void *)cfgInt->s_calibv2);
        if(ret_v3 != AYNRV3_RET_SUCCESS) {
            ret = XCAM_RETURN_ERROR_FAILED;
            LOGE_ANR("%s: Initializaion ynr group v3 failed (%d)\n", __FUNCTION__, ret);
        }
#endif
    }
    else if(g_aynr_hw_ver ==  AYNR_HARDWARE_V2) {
        aynr_group_contex = (CamGroup_Aynr_Contex_t*)malloc(sizeof(CamGroup_Aynr_Contex_t));
#if (AYNR_USE_JSON_FILE_V2)
        Aynr_result_t ret_v2 = AYNR_RET_SUCCESS;
        ret_v2 = Aynr_Init_V2(&(aynr_group_contex->aynr_contex_v2), (void *)cfgInt->s_calibv2);
        if(ret_v2 != AYNR_RET_SUCCESS) {
            ret = XCAM_RETURN_ERROR_FAILED;
            LOGE_ANR("%s: Initializaion ynr group v2 failed (%d)\n", __FUNCTION__, ret);
        }
#endif
    } else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE_ANR("module_hw_version of aynr (%d) is invalid!!!!", g_aynr_hw_ver);
    }

    if(ret != XCAM_RETURN_NO_ERROR) {
        LOGE_ANR("%s: Initializaion group ynr failed (%d)\n", __FUNCTION__, ret);
    } else {
        // to do got aynrSurrViewClib and initinal paras for for surround view
        aynr_group_contex->group_CalibV2.groupMethod = CalibDbV2_CAMGROUP_AYNR_METHOD_MEAN;// to do from json
        aynr_group_contex->camera_Num = cfgInt->camIdArrayLen;

        *context = (RkAiqAlgoContext *)(aynr_group_contex);

        LOGI_ANR("%s:%d surrViewMethod(1-mean):%d, cameraNum %d \n",
                 __FUNCTION__, __LINE__,
                 aynr_group_contex->group_CalibV2.groupMethod,
                 aynr_group_contex->camera_Num);
    }

    LOGI_ANR("%s exit ret:%d\n", __FUNCTION__, ret);
    return ret;

}

static XCamReturn groupAynrDestroyCtx(RkAiqAlgoContext *context)
{
    LOGI_ANR("%s enter \n", __FUNCTION__ );

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    CamGroup_Aynr_Contex_t *aynr_group_contex = (CamGroup_Aynr_Contex_t*)context;

    if(g_aynr_hw_ver == AYNR_HARDWARE_V3) {
        Aynr_result_V3_t ret_v3 = AYNRV3_RET_SUCCESS;
        ret_v3 = Aynr_Release_V3(aynr_group_contex->aynr_contex_v3);
        if(ret_v3 != AYNRV3_RET_SUCCESS) {
            ret = XCAM_RETURN_ERROR_FAILED;
            LOGE_ANR("%s: Initializaion ANR failed (%d)\n", __FUNCTION__, ret);
        }
    }
    else if(g_aynr_hw_ver == AYNR_HARDWARE_V2) {
        Aynr_result_t ret_v2 = AYNR_RET_SUCCESS;
        ret_v2 = Aynr_Release_V2(aynr_group_contex->aynr_contex_v2);
        if(ret_v2 != AYNR_RET_SUCCESS) {
            ret = XCAM_RETURN_ERROR_FAILED;
            LOGE_ANR("%s: Initializaion ANR failed (%d)\n", __FUNCTION__, ret);
        }
    }
    else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE_ANR("module_hw_version of awb (%d) is isvalid!!!!", g_aynr_hw_ver);
    }

    if(ret != XCAM_RETURN_NO_ERROR) {
        LOGE_ANR("%s: release ANR failed (%d)\n", __FUNCTION__, ret);
    } else {
        free(aynr_group_contex);
    }


    LOGI_ANR("%s exit ret:%d\n", __FUNCTION__, ret);
    return ret;
}

static XCamReturn groupAynrPrepare(RkAiqAlgoCom* params)
{
    LOGI_ANR("%s enter \n", __FUNCTION__ );

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    CamGroup_Aynr_Contex_t * aynr_group_contex = (CamGroup_Aynr_Contex_t *)params->ctx;
    RkAiqAlgoCamGroupPrepare* para = (RkAiqAlgoCamGroupPrepare*)params;

    if(g_aynr_hw_ver == AYNR_HARDWARE_V3) {
        Aynr_Context_V3_t * aynr_contex_v3 = aynr_group_contex->aynr_contex_v3;
        if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )) {
            // todo  update calib pars for surround view
#if AYNR_USE_JSON_FILE_V3
            void *pCalibdbV2 = (void*)(para->s_calibv2);
            CalibDbV2_YnrV3_t *ynr_v3 = (CalibDbV2_YnrV3_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCalibdbV2, ynr_v3));
            aynr_contex_v3->ynr_v3 = *ynr_v3;
            aynr_contex_v3->isIQParaUpdate = true;
            aynr_contex_v3->isReCalculate |= 1;
#endif
        }
        Aynr_Config_V3_t stAynrConfigV3;
        stAynrConfigV3.rawHeight =  params->u.prepare.sns_op_height;
        stAynrConfigV3.rawWidth = params->u.prepare.sns_op_width;
        Aynr_result_V3_t ret_v3 = AYNRV3_RET_SUCCESS;
        ret_v3 = Aynr_Prepare_V3(aynr_contex_v3, &stAynrConfigV3);
        if(ret_v3 != AYNRV3_RET_SUCCESS) {
            ret = XCAM_RETURN_ERROR_FAILED;
            LOGE_ANR("%s: config ANR failed (%d)\n", __FUNCTION__, ret);
        }
    }
    else if(g_aynr_hw_ver == AYNR_HARDWARE_V2) {
        Aynr_Context_V2_t * aynr_contex_v2 = aynr_group_contex->aynr_contex_v2;
        if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )) {
            // todo  update calib pars for surround view
#if AYNR_USE_JSON_FILE_V2
            void *pCalibdbV2 = (void*)(para->s_calibv2);
            CalibDbV2_YnrV2_t *ynr_v2 = (CalibDbV2_YnrV2_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCalibdbV2, ynr_v2));
            aynr_contex_v2->ynr_v2 = *ynr_v2;
            aynr_contex_v2->isIQParaUpdate = true;
            aynr_contex_v2->isReCalculate |= 1;
#endif
        }
        Aynr_Config_V2_t stAynrConfigV2;
        stAynrConfigV2.rawHeight =  params->u.prepare.sns_op_height;
        stAynrConfigV2.rawWidth = params->u.prepare.sns_op_width;
        Aynr_result_t ret_v2 = AYNR_RET_SUCCESS;
        ret_v2 = Aynr_Prepare_V2(aynr_contex_v2, &stAynrConfigV2);
        if(ret_v2 != AYNR_RET_SUCCESS) {
            ret = XCAM_RETURN_ERROR_FAILED;
            LOGE_ANR("%s: config ANR failed (%d)\n", __FUNCTION__, ret);
        }
    }
    else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE_ANR("module_hw_version of awb (%d) is isvalid!!!!", g_aynr_hw_ver);
    }

    LOGI_ANR("%s exit ret:%d\n", __FUNCTION__, ret);
    return ret;
}

static XCamReturn groupAynrProcessing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    LOGI_ANR("%s enter \n", __FUNCTION__ );
    LOGI_ANR("----------------------------------------------frame_id (%d)----------------------------------------------\n", inparams->frame_id);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoCamGroupProcIn* procParaGroup = (RkAiqAlgoCamGroupProcIn*)inparams;
    RkAiqAlgoCamGroupProcOut* procResParaGroup = (RkAiqAlgoCamGroupProcOut*)outparams;
    CamGroup_Aynr_Contex_t * aynr_group_contex = (CamGroup_Aynr_Contex_t *)inparams->ctx;
    int deltaIso = 0;

    //method error
    if (aynr_group_contex->group_CalibV2.groupMethod <= CalibDbV2_CAMGROUP_AYNR_METHOD_MIN
            ||  aynr_group_contex->group_CalibV2.groupMethod >=  CalibDbV2_CAMGROUP_AYNR_METHOD_MAX) {
        return (ret);
    }

    //group empty
    if(procParaGroup->camgroupParmasArray == nullptr) {
        LOGE_ANR("camgroupParmasArray is null");
        return(XCAM_RETURN_ERROR_FAILED);
    }

    //get cur ae exposure
    Aynr_ExpInfo_V3_t stExpInfoV3;
    memset(&stExpInfoV3, 0x00, sizeof(Aynr_ExpInfo_V3_t));
    stExpInfoV3.hdr_mode = 0; //pAnrProcParams->hdr_mode;
    stExpInfoV3.snr_mode = 0;
    for(int i = 0; i < 3; i++) {
        stExpInfoV3.arIso[i] = 50;
        stExpInfoV3.arAGain[i] = 1.0;
        stExpInfoV3.arDGain[i] = 1.0;
        stExpInfoV3.arTime[i] = 0.01;
    }


    //merge ae result, iso mean value
    rk_aiq_singlecam_3a_result_t* scam_3a_res = procParaGroup->camgroupParmasArray[0];
    if(scam_3a_res->aec._bEffAecExpValid) {
        RKAiqAecExpInfo_t* pCurExp = &scam_3a_res->aec._effAecExpInfo;
        stExpInfoV3.snr_mode = pCurExp->CISFeature.SNR;
        if((rk_aiq_working_mode_t)procParaGroup->working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
            stExpInfoV3.hdr_mode = 0;
            stExpInfoV3.arAGain[0] = pCurExp->LinearExp.exp_real_params.analog_gain;
            stExpInfoV3.arDGain[0] = pCurExp->LinearExp.exp_real_params.digital_gain;
            stExpInfoV3.arTime[0] = pCurExp->LinearExp.exp_real_params.integration_time;
            stExpInfoV3.arIso[0] = stExpInfoV3.arAGain[0] * stExpInfoV3.arDGain[0] * 50;

        } else {
            if(procParaGroup->working_mode == RK_AIQ_ISP_HDR_MODE_2_FRAME_HDR
                    || procParaGroup->working_mode == RK_AIQ_ISP_HDR_MODE_2_LINE_HDR)
                stExpInfoV3.hdr_mode = 1;
            else if (procParaGroup->working_mode == RK_AIQ_ISP_HDR_MODE_3_FRAME_HDR
                     || procParaGroup->working_mode == RK_AIQ_ISP_HDR_MODE_3_LINE_HDR)
                stExpInfoV3.hdr_mode = 2;
            else {
                stExpInfoV3.hdr_mode = 0;
                LOGE_ANR("mode error\n");
            }

            for(int i = 0; i < 3; i++) {
                stExpInfoV3.arAGain[i] = pCurExp->HdrExp[i].exp_real_params.analog_gain;
                stExpInfoV3.arDGain[i] = pCurExp->HdrExp[i].exp_real_params.digital_gain;
                stExpInfoV3.arTime[i] = pCurExp->HdrExp[i].exp_real_params.integration_time;
                stExpInfoV3.arIso[i] = stExpInfoV3.arAGain[i] * stExpInfoV3.arDGain[i] * 50;
            }

        }
    } else {
        LOGW("fail to get sensor gain form AE module,use default value ");
    }



    if(g_aynr_hw_ver == AYNR_HARDWARE_V3) {
        Aynr_Context_V3_t * aynr_contex_v3 = aynr_group_contex->aynr_contex_v3;
        Aynr_ProcResult_V3_t stAynrResultV3;
        deltaIso = abs(stExpInfoV3.arIso[stExpInfoV3.hdr_mode] - aynr_contex_v3->stExpInfo.arIso[stExpInfoV3.hdr_mode]);
        if(deltaIso > AYNRV3_RECALCULATE_DELTA_ISO) {
            aynr_contex_v3->isReCalculate |= 1;
        }
        if(aynr_contex_v3->isReCalculate) {
            Aynr_result_V3_t ret_v3 = AYNRV3_RET_SUCCESS;
            ret_v3 = Aynr_Process_V3(aynr_contex_v3, &stExpInfoV3);
            if(ret_v3 != AYNRV3_RET_SUCCESS) {
                ret = XCAM_RETURN_ERROR_FAILED;
                LOGE_ANR("%s: processing ANR failed (%d)\n", __FUNCTION__, ret);
            }
            Aynr_GetProcResult_V3(aynr_contex_v3, &stAynrResultV3);
            stAynrResultV3.isNeedUpdate = true;
            LOGD_ANR("recalculate: %d delta_iso:%d \n ", aynr_contex_v3->isReCalculate, deltaIso);
        } else {
            stAynrResultV3 = aynr_contex_v3->stProcResult;
            stAynrResultV3.isNeedUpdate = true;
        }

        for (int i = 0; i < procResParaGroup->arraySize; i++) {
            *(procResParaGroup->camgroupParmasArray[i]->aynr._aynr_procRes_v3) = stAynrResultV3.stFix;
        }
        aynr_contex_v3->isReCalculate = 0;
    }
    else if(g_aynr_hw_ver == AYNR_HARDWARE_V2) {
        Aynr_Context_V2_t * aynr_contex_v2 = aynr_group_contex->aynr_contex_v2;
        Aynr_ProcResult_V2_t stAynrResultV2;
        Aynr_ExpInfo_t stExpInfoV2;
        memset(&stExpInfoV2, 0x00, sizeof(stExpInfoV2));
        stExpInfoV2.hdr_mode = stExpInfoV3.hdr_mode; //pAnrProcParams->hdr_mode;
        stExpInfoV2.snr_mode = stExpInfoV3.snr_mode;
        for(int i = 0; i < 3; i++) {
            stExpInfoV2.arIso[i] = stExpInfoV3.arIso[i];
            stExpInfoV2.arAGain[i] = stExpInfoV3.arAGain[i];
            stExpInfoV2.arDGain[i] = stExpInfoV3.arDGain[i];
            stExpInfoV2.arTime[i] = stExpInfoV3.arTime[i];
        }
        deltaIso = abs(stExpInfoV2.arIso[stExpInfoV2.hdr_mode] - aynr_contex_v2->stExpInfo.arIso[stExpInfoV2.hdr_mode]);
        if(deltaIso > AYNRV3_RECALCULATE_DELTA_ISO) {
            aynr_contex_v2->isReCalculate |= 1;
        }
        if(aynr_contex_v2->isReCalculate) {
            Aynr_result_t ret_v2 = AYNR_RET_SUCCESS;
            ret_v2 = Aynr_Process_V2(aynr_contex_v2, &stExpInfoV2);
            if(ret_v2 != AYNR_RET_SUCCESS) {
                ret = XCAM_RETURN_ERROR_FAILED;
                LOGE_ANR("%s: processing ANR failed (%d)\n", __FUNCTION__, ret);
            }
            stAynrResultV2.isNeedUpdate = true;
            LOGD_ANR("recalculate: %d delta_iso:%d \n ", aynr_contex_v2->isReCalculate, deltaIso);
        } else {
            stAynrResultV2.isNeedUpdate = false;
        }
        Aynr_GetProcResult_V2(aynr_contex_v2, &stAynrResultV2);
        for (int i = 0; i < procResParaGroup->arraySize; i++) {
            *(procResParaGroup->camgroupParmasArray[i]->aynr._aynr_procRes_v2) = stAynrResultV2.stFix;
        }
        aynr_contex_v2->isReCalculate = 0;

    }
    else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE_ANR("module_hw_version of awb (%d) is isvalid!!!!", g_aynr_hw_ver);
    }

    LOGI_ANR("%s exit\n", __FUNCTION__);
    return ret;
}

RkAiqAlgoDescription g_RkIspAlgoDescCamgroupAynr = {
    .common = {
        .version = RKISP_ALGO_CAMGROUP_AYNR_VERSION,
        .vendor  = RKISP_ALGO_CAMGROUP_AYNR_VENDOR,
        .description = RKISP_ALGO_CAMGROUP_AYNR_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_AYNR,
        .id      = 0,
        .create_context  = groupAynrCreateCtx,
        .destroy_context = groupAynrDestroyCtx,
    },
    .prepare = groupAynrPrepare,
    .pre_process = NULL,
    .processing = groupAynrProcessing,
    .post_process = NULL,
};

RKAIQ_END_DECLARE
