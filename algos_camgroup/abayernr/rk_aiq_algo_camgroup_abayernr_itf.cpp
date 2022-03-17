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

#include "rk_aiq_algo_camgroup_abayernr_itf.h"
#include "rk_aiq_algo_camgroup_types.h"
#include "rk_aiq_types_camgroup_abayernr_prvt.h"
#include "abayer2dnr2/rk_aiq_abayer2dnr_algo_itf_v2.h"
#include "abayer2dnr2/rk_aiq_abayer2dnr_algo_v2.h"
#include "arawnr2/rk_aiq_abayernr_algo_itf_v2.h"
#include "arawnr2/rk_aiq_abayernr_algo_v2.h"



RKAIQ_BEGIN_DECLARE

static abayernr_hardware_version_t g_abayernr_hw_ver = ABAYERNR_HARDWARE_V2;

static XCamReturn groupAbayernrCreateCtx(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{
    LOGI_ANR("%s enter \n", __FUNCTION__ );

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    CamGroup_Abayernr_Contex_t *abayernr_group_contex = NULL;
    AlgoCtxInstanceCfgCamGroup *cfgInt = (AlgoCtxInstanceCfgCamGroup*)cfg;
    //g_abayernr_hw_ver = (abayernr_hardware_version_t)(cfgInt->cfg_com.module_hw_version);

    if (CHECK_ISP_HW_V21()) {
        g_abayernr_hw_ver = ABAYERNR_HARDWARE_V1;
    } else if(CHECK_ISP_HW_V3X()) {
        g_abayernr_hw_ver = ABAYERNR_HARDWARE_V2;
    } else {
        g_abayernr_hw_ver = ABAYERNR_HARDWARE_MIN;
    }

    if(g_abayernr_hw_ver == ABAYERNR_HARDWARE_V2) {
        abayernr_group_contex = (CamGroup_Abayernr_Contex_t*)malloc(sizeof(CamGroup_Abayernr_Contex_t));
#if ABAYER2DNR_USE_JSON_FILE_V2
        Abayer2dnr_result_V2_t ret_v2 = ABAYER2DNR_RET_SUCCESS;
        ret_v2 = Abayer2dnr_Init_V2(&(abayernr_group_contex->abayernr_contex_v2), (void *)cfgInt->s_calibv2);
        if(ret_v2 != ABAYER2DNR_RET_SUCCESS) {
            ret = XCAM_RETURN_ERROR_FAILED;
            LOGE_ANR("%s: Initializaion ANR failed (%d)\n", __FUNCTION__, ret);
        }
#endif
    }
    else if(g_abayernr_hw_ver ==  ABAYERNR_HARDWARE_V1) {
        abayernr_group_contex = (CamGroup_Abayernr_Contex_t*)malloc(sizeof(CamGroup_Abayernr_Contex_t));
#if (ABAYERNR_USE_JSON_FILE_V2)
        Abayernr_result_t ret_v1 = ABAYERNR_RET_SUCCESS;
        ret_v1 = Abayernr_Init_V2(&(abayernr_group_contex->abayernr_contex_v1), (void *)cfgInt->s_calibv2);
        if(ret_v1 != ABAYERNR_RET_SUCCESS) {
            ret = XCAM_RETURN_ERROR_FAILED;
            LOGE_ANR("%s: Initializaion ANR failed (%d)\n", __FUNCTION__, ret);
        }
#endif
    } else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE_ANR("module_hw_version of abayernr (%d) is invalid!!!!", g_abayernr_hw_ver);
    }

    if(ret != XCAM_RETURN_NO_ERROR) {
        LOGE_ANR("%s: Initializaion group bayernr failed (%d)\n", __FUNCTION__, ret);
    } else {
        // to do got abayernrSurrViewClib and initinal paras for for surround view
        abayernr_group_contex->group_CalibV2.groupMethod = CalibDbV2_CAMGROUP_ABAYERNR_METHOD_MEAN;// to do from json
        abayernr_group_contex->camera_Num = cfgInt->camIdArrayLen;

        *context = (RkAiqAlgoContext *)(abayernr_group_contex);

        LOGI_ANR("%s:%d surrViewMethod(1-mean):%d, cameraNum %d \n",
                 __FUNCTION__, __LINE__,
                 abayernr_group_contex->group_CalibV2.groupMethod,
                 abayernr_group_contex->camera_Num);
    }

    LOGI_ANR("%s exit ret:%d\n", __FUNCTION__, ret);
    return ret;

}

static XCamReturn groupAbayernrDestroyCtx(RkAiqAlgoContext *context)
{
    LOGI_ANR("%s enter \n", __FUNCTION__ );

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    CamGroup_Abayernr_Contex_t *abayernr_group_contex = (CamGroup_Abayernr_Contex_t*)context;

    if(g_abayernr_hw_ver == ABAYERNR_HARDWARE_V2) {
        Abayer2dnr_result_V2_t ret_v2 = ABAYER2DNR_RET_SUCCESS;
        ret_v2 = Abayer2dnr_Release_V2(abayernr_group_contex->abayernr_contex_v2);
        if(ret_v2 != ABAYER2DNR_RET_SUCCESS) {
            ret = XCAM_RETURN_ERROR_FAILED;
            LOGE_ANR("%s: Initializaion ANR failed (%d)\n", __FUNCTION__, ret);
        }
    }
    else if(g_abayernr_hw_ver == ABAYERNR_HARDWARE_V1) {
        Abayernr_result_t ret_v1 = ABAYERNR_RET_SUCCESS;
        ret_v1  = Abayernr_Release_V2(abayernr_group_contex->abayernr_contex_v1);
        if(ret_v1  != ABAYERNR_RET_SUCCESS) {
            ret = XCAM_RETURN_ERROR_FAILED;
            LOGE_ANR("%s: Initializaion ANR failed (%d)\n", __FUNCTION__, ret);
        }
    }
    else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE_ANR("module_hw_version of awb (%d) is isvalid!!!!", g_abayernr_hw_ver);
    }

    if(ret != XCAM_RETURN_NO_ERROR) {
        LOGE_ANR("%s: release ANR failed (%d)\n", __FUNCTION__, ret);
    } else {
        free(abayernr_group_contex);
    }


    LOGI_ANR("%s exit ret:%d\n", __FUNCTION__, ret);
    return ret;
}

static XCamReturn groupAbayernrPrepare(RkAiqAlgoCom* params)
{
    LOGI_ANR("%s enter \n", __FUNCTION__ );

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    CamGroup_Abayernr_Contex_t * abayernr_group_contex = (CamGroup_Abayernr_Contex_t *)params->ctx;
    RkAiqAlgoCamGroupPrepare* para = (RkAiqAlgoCamGroupPrepare*)params;

    if(g_abayernr_hw_ver == ABAYERNR_HARDWARE_V2) {
        Abayer2dnr_Context_V2_t * abayer2dnr_contex_v2 = abayernr_group_contex->abayernr_contex_v2;
        if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )) {
            // todo  update calib pars for surround view
#if ABAYER2DNR_USE_JSON_FILE_V2
            void *pCalibdbV2 = (void*)(para->s_calibv2);
            CalibDbV2_Bayer2dnr_V2_t *bayer2dnr_v2 = (CalibDbV2_Bayer2dnr_V2_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCalibdbV2, bayer2dnr_v2));
            abayer2dnr_contex_v2->bayernr_v2 = *bayer2dnr_v2;
            abayer2dnr_contex_v2->isIQParaUpdate = true;
            abayer2dnr_contex_v2->isReCalculate |= 1;
#endif
        }
        Abayer2dnr_Config_V2_t stAbayer2dnrConfigV2;
        Abayer2dnr_result_V2_t ret_v2 = ABAYER2DNR_RET_SUCCESS;
        ret_v2 = Abayer2dnr_Prepare_V2(abayer2dnr_contex_v2, &stAbayer2dnrConfigV2);
        if(ret_v2 != ABAYER2DNR_RET_SUCCESS) {
            ret = XCAM_RETURN_ERROR_FAILED;
            LOGE_ANR("%s: config ANR failed (%d)\n", __FUNCTION__, ret);
        }
    }
    else if(g_abayernr_hw_ver == ABAYERNR_HARDWARE_V1) {
        Abayernr_Context_V2_t * abayernr_contex_v2 = abayernr_group_contex->abayernr_contex_v1;
        if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )) {
            // todo  update calib pars for surround view
#if ABAYERNR_USE_JSON_FILE_V2
            void *pCalibdbV2 = (void*)(para->s_calibv2);
            CalibDbV2_BayerNrV2_t *bayernr_v2 = (CalibDbV2_BayerNrV2_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCalibdbV2, bayernr_v2));
            abayernr_contex_v2->bayernr_v2 = *bayernr_v2;
            abayernr_contex_v2->isIQParaUpdate = true;
            abayernr_contex_v2->isReCalculate |= 1;
#endif
        }
        Abayernr_Config_V2_t stAbayernrConfigV2;
        Abayernr_result_t ret_v1 = ABAYERNR_RET_SUCCESS;
        ret_v1 = Abayernr_Prepare_V2(abayernr_contex_v2, &stAbayernrConfigV2);
        if(ret_v1 != ABAYERNR_RET_SUCCESS) {
            ret = XCAM_RETURN_ERROR_FAILED;
            LOGE_ANR("%s: config ANR failed (%d)\n", __FUNCTION__, ret);
        }
    }
    else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE_ANR("module_hw_version of awb (%d) is isvalid!!!!", g_abayernr_hw_ver);
    }

    LOGI_ANR("%s exit ret:%d\n", __FUNCTION__, ret);
    return ret;
}

static XCamReturn groupAbayernrProcessing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    LOGI_ANR("%s enter \n", __FUNCTION__ );
    LOGI_ANR("----------------------------------------------frame_id (%d)----------------------------------------------\n", inparams->frame_id);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoCamGroupProcIn* procParaGroup = (RkAiqAlgoCamGroupProcIn*)inparams;
    RkAiqAlgoCamGroupProcOut* procResParaGroup = (RkAiqAlgoCamGroupProcOut*)outparams;
    CamGroup_Abayernr_Contex_t * abayernr_group_contex = (CamGroup_Abayernr_Contex_t *)inparams->ctx;
    int deltaIso = 0;

    //method error
    if (abayernr_group_contex->group_CalibV2.groupMethod <= CalibDbV2_CAMGROUP_ABAYERNR_METHOD_MIN
            ||  abayernr_group_contex->group_CalibV2.groupMethod >=  CalibDbV2_CAMGROUP_ABAYERNR_METHOD_MAX) {
        return (ret);
    }

    //group empty
    if(procParaGroup->camgroupParmasArray == nullptr) {
        LOGE_ANR("camgroupParmasArray is null");
        return(XCAM_RETURN_ERROR_FAILED);
    }

    //get cur ae exposure
    Abayer2dnr_ExpInfo_V2_t stExpInfoV2;
    memset(&stExpInfoV2, 0x00, sizeof(stExpInfoV2));
    stExpInfoV2.hdr_mode = 0; //pAnrProcParams->hdr_mode;
    stExpInfoV2.snr_mode = 0;
    for(int i = 0; i < 3; i++) {
        stExpInfoV2.arIso[i] = 50;
        stExpInfoV2.arAGain[i] = 1.0;
        stExpInfoV2.arDGain[i] = 1.0;
        stExpInfoV2.arTime[i] = 0.01;
    }


    //merge ae result, iso mean value
    rk_aiq_singlecam_3a_result_t* scam_3a_res = procParaGroup->camgroupParmasArray[0];
    if(scam_3a_res->aec._bEffAecExpValid) {
        RKAiqAecExpInfo_t* pCurExp = &scam_3a_res->aec._effAecExpInfo;
        stExpInfoV2.snr_mode = pCurExp->CISFeature.SNR;
        if((rk_aiq_working_mode_t)procParaGroup->working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
            stExpInfoV2.hdr_mode = 0;
            stExpInfoV2.arAGain[0] = pCurExp->LinearExp.exp_real_params.analog_gain;
            stExpInfoV2.arDGain[0] = pCurExp->LinearExp.exp_real_params.digital_gain;
            stExpInfoV2.arTime[0] = pCurExp->LinearExp.exp_real_params.integration_time;
            stExpInfoV2.arIso[0] = stExpInfoV2.arAGain[0] * stExpInfoV2.arDGain[0] * 50;

        } else {
            if(procParaGroup->working_mode == RK_AIQ_ISP_HDR_MODE_2_FRAME_HDR
                    || procParaGroup->working_mode == RK_AIQ_ISP_HDR_MODE_2_LINE_HDR)
                stExpInfoV2.hdr_mode = 1;
            else if (procParaGroup->working_mode == RK_AIQ_ISP_HDR_MODE_3_FRAME_HDR
                     || procParaGroup->working_mode == RK_AIQ_ISP_HDR_MODE_3_LINE_HDR)
                stExpInfoV2.hdr_mode = 2;
            else {
                stExpInfoV2.hdr_mode = 0;
                LOGE_ANR("mode error\n");
            }

            for(int i = 0; i < 3; i++) {
                stExpInfoV2.arAGain[i] = pCurExp->HdrExp[i].exp_real_params.analog_gain;
                stExpInfoV2.arDGain[i] = pCurExp->HdrExp[i].exp_real_params.digital_gain;
                stExpInfoV2.arTime[i] = pCurExp->HdrExp[i].exp_real_params.integration_time;
                stExpInfoV2.arIso[i] = stExpInfoV2.arAGain[i] * stExpInfoV2.arDGain[i] * 50;
            }

        }
    } else {
        LOGW("fail to get sensor gain form AE module,use default value ");
    }



    if(g_abayernr_hw_ver == ABAYERNR_HARDWARE_V2) {
        Abayer2dnr_Context_V2_t * abayer2dnr_contex_v2 = abayernr_group_contex->abayernr_contex_v2;
        Abayer2dnr_ProcResult_V2_t stAbayer2dnrResultV2;
        deltaIso = abs(stExpInfoV2.arIso[stExpInfoV2.hdr_mode] - abayer2dnr_contex_v2->stExpInfo.arIso[stExpInfoV2.hdr_mode]);
        if(deltaIso > ABAYERNRV2_RECALCULATE_DELTA_ISO) {
            abayer2dnr_contex_v2->isReCalculate |= 1;
        }
        if(abayer2dnr_contex_v2->isReCalculate) {
            Abayer2dnr_result_V2_t ret_v2 = ABAYER2DNR_RET_SUCCESS;
            ret_v2 = Abayer2dnr_Process_V2(abayer2dnr_contex_v2, &stExpInfoV2);
            if(ret_v2 != ABAYER2DNR_RET_SUCCESS) {
                ret = XCAM_RETURN_ERROR_FAILED;
                LOGE_ANR("%s: processing ANR failed (%d)\n", __FUNCTION__, ret);
            }
            Abayer2dnr_GetProcResult_V2(abayer2dnr_contex_v2, &stAbayer2dnrResultV2);
            stAbayer2dnrResultV2.isNeedUpdate = true;
            LOGD_ANR("recalculate: %d delta_iso:%d \n ", abayer2dnr_contex_v2->isReCalculate, deltaIso);
        } else {
            stAbayer2dnrResultV2 = abayer2dnr_contex_v2->stProcResult;
            stAbayer2dnrResultV2.isNeedUpdate = true;
        }

        for (int i = 0; i < procResParaGroup->arraySize; i++) {
            *(procResParaGroup->camgroupParmasArray[i]->abayernr._abayer2dnr_procRes_v2) = stAbayer2dnrResultV2.st2DFix;
        }
        abayer2dnr_contex_v2->isReCalculate = 0;
    }
    else if(g_abayernr_hw_ver == ABAYERNR_HARDWARE_V1) {
        Abayernr_Context_V2_t * abayernr_contex_v2 = abayernr_group_contex->abayernr_contex_v1;
        Abayernr_ProcResult_V2_t stAbayernrResultV2;
        Abayernr_ExpInfo_t stExpInfoV1;
        memset(&stExpInfoV1, 0x00, sizeof(stExpInfoV1));
        stExpInfoV1.hdr_mode = stExpInfoV2.hdr_mode; //pAnrProcParams->hdr_mode;
        stExpInfoV1.snr_mode = stExpInfoV2.snr_mode;
        for(int i = 0; i < 3; i++) {
            stExpInfoV1.arIso[i] = stExpInfoV2.arIso[i];
            stExpInfoV1.arAGain[i] = stExpInfoV2.arAGain[i];
            stExpInfoV1.arDGain[i] = stExpInfoV2.arDGain[i];
            stExpInfoV1.arTime[i] = stExpInfoV2.arTime[i];
        }
        deltaIso = abs(stExpInfoV1.arIso[stExpInfoV1.hdr_mode] - abayernr_contex_v2->stExpInfo.arIso[stExpInfoV1.hdr_mode]);
        if(deltaIso > ABAYERNRV2_RECALCULATE_DELTA_ISO) {
            abayernr_contex_v2->isReCalculate |= 1;
        }
        if(abayernr_contex_v2->isReCalculate) {
            Abayernr_result_t ret_v1 = ABAYERNR_RET_SUCCESS;
            ret_v1 = Abayernr_Process_V2(abayernr_contex_v2, &stExpInfoV1);
            if(ret_v1 != ABAYERNR_RET_SUCCESS) {
                ret = XCAM_RETURN_ERROR_FAILED;
                LOGE_ANR("%s: processing ANR failed (%d)\n", __FUNCTION__, ret);
            }
            stAbayernrResultV2.isNeedUpdate = true;
            LOGD_ANR("recalculate: %d delta_iso:%d \n ", abayernr_contex_v2->isReCalculate, deltaIso);
        } else {
            stAbayernrResultV2.isNeedUpdate = false;
        }
        Abayernr_GetProcResult_V2(abayernr_contex_v2, &stAbayernrResultV2);
        for (int i = 0; i < procResParaGroup->arraySize; i++) {
            procResParaGroup->camgroupParmasArray[i]->abayernr._abayernr_procRes_v1->st2DParam = stAbayernrResultV2.st2DFix;
            procResParaGroup->camgroupParmasArray[i]->abayernr._abayernr_procRes_v1->st3DParam = stAbayernrResultV2.st3DFix;
        }
        abayernr_contex_v2->isReCalculate = 0;

    }
    else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE_ANR("module_hw_version of awb (%d) is isvalid!!!!", g_abayernr_hw_ver);
    }

    LOGI_ANR("%s exit\n", __FUNCTION__);
    return ret;
}

RkAiqAlgoDescription g_RkIspAlgoDescCamgroupAbayernr = {
    .common = {
        .version = RKISP_ALGO_CAMGROUP_ABAYERNR_VERSION,
        .vendor  = RKISP_ALGO_CAMGROUP_ABAYERNR_VENDOR,
        .description = RKISP_ALGO_CAMGROUP_ABAYERNR_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_ARAWNR,
        .id      = 0,
        .create_context  = groupAbayernrCreateCtx,
        .destroy_context = groupAbayernrDestroyCtx,
    },
    .prepare = groupAbayernrPrepare,
    .pre_process = NULL,
    .processing = groupAbayernrProcessing,
    .post_process = NULL,
};

RKAIQ_END_DECLARE
