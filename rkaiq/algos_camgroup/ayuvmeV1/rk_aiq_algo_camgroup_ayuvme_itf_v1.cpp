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

#include "rk_aiq_algo_camgroup_ayuvme_itf_v1.h"
#include "rk_aiq_algo_camgroup_types.h"
#include "rk_aiq_types_camgroup_ayuvme_prvt_v1.h"
#include "ayuvmeV1/rk_aiq_ayuvme_algo_itf_v1.h"
#include "ayuvmeV1/rk_aiq_ayuvme_algo_v1.h"



RKAIQ_BEGIN_DECLARE


static XCamReturn groupAyuvmeV1CreateCtx(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{
    LOGI_ANR("%s enter \n", __FUNCTION__ );

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    CamGroup_AyuvmeV1_Contex_t *ayuvme_group_contex = NULL;
    AlgoCtxInstanceCfgCamGroup *cfgInt = (AlgoCtxInstanceCfgCamGroup*)cfg;

    if(CHECK_ISP_HW_V39()) {
        ayuvme_group_contex = (CamGroup_AyuvmeV1_Contex_t*)malloc(sizeof(CamGroup_AyuvmeV1_Contex_t));
#if AYUVME_USE_JSON_FILE_V1
        Ayuvme_result_V1_t ret_v1 = AYUVMEV1_RET_SUCCESS;
        ret_v1 = Ayuvme_Init_V1(&(ayuvme_group_contex->ayuvme_contex_v1), (void *)cfgInt->s_calibv2);
        if(ret_v1 != AYUVMEV1_RET_SUCCESS) {
            ret = XCAM_RETURN_ERROR_FAILED;
            LOGE_ANR("%s: Initializaion yuvme group v1 failed (%d)\n", __FUNCTION__, ret);
        }
#endif
    }
    else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE_ANR("module_hw_version of ayuvme  is invalid!!!!");
    }

    if(ret != XCAM_RETURN_NO_ERROR) {
        LOGE_ANR("%s: Initializaion group yuvme failed (%d)\n", __FUNCTION__, ret);
    } else {
        // to do got ayuvmeSurrViewClib and initinal paras for for surround view
        ayuvme_group_contex->group_CalibV2.groupMethod = CalibDbV2_CAMGROUP_AYUVMEV1_METHOD_MEAN;// to do from json
        ayuvme_group_contex->camera_Num = cfgInt->camIdArrayLen;

        *context = (RkAiqAlgoContext *)(ayuvme_group_contex);

        LOGI_ANR("%s:%d surrViewMethod(1-mean):%d, cameraNum %d \n",
                 __FUNCTION__, __LINE__,
                 ayuvme_group_contex->group_CalibV2.groupMethod,
                 ayuvme_group_contex->camera_Num);
    }

    LOGI_ANR("%s exit ret:%d\n", __FUNCTION__, ret);
    return ret;

}

static XCamReturn groupAyuvmeV1DestroyCtx(RkAiqAlgoContext *context)
{
    LOGI_ANR("%s enter \n", __FUNCTION__ );

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    CamGroup_AyuvmeV1_Contex_t *ayuvme_group_contex = (CamGroup_AyuvmeV1_Contex_t*)context;

    if(CHECK_ISP_HW_V39()) {
        Ayuvme_result_V1_t ret_v1 = AYUVMEV1_RET_SUCCESS;
        ret_v1 = Ayuvme_Release_V1(ayuvme_group_contex->ayuvme_contex_v1);
        if(ret_v1 != AYUVMEV1_RET_SUCCESS) {
            ret = XCAM_RETURN_ERROR_FAILED;
            LOGE_ANR("%s: Initializaion ANR failed (%d)\n", __FUNCTION__, ret);
        }
    }
    else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE_ANR("module_hw_version of ayuvme is isvalid!!!!");
    }

    if(ret != XCAM_RETURN_NO_ERROR) {
        LOGE_ANR("%s: release ANR failed (%d)\n", __FUNCTION__, ret);
    } else {
        free(ayuvme_group_contex);
    }


    LOGI_ANR("%s exit ret:%d\n", __FUNCTION__, ret);
    return ret;
}

static XCamReturn groupAyuvmeV1Prepare(RkAiqAlgoCom* params)
{
    LOGI_ANR("%s enter \n", __FUNCTION__ );

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    CamGroup_AyuvmeV1_Contex_t * ayuvme_group_contex = (CamGroup_AyuvmeV1_Contex_t *)params->ctx;
    RkAiqAlgoCamGroupPrepare* para = (RkAiqAlgoCamGroupPrepare*)params;

    if(CHECK_ISP_HW_V39()) {
        Ayuvme_Context_V1_t * ayuvme_contex_v1 = ayuvme_group_contex->ayuvme_contex_v1;
        if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )) {
            // todo  update calib pars for surround view
#if AYUVME_USE_JSON_FILE_V1
            void *pCalibdbV2 = (void*)(para->s_calibv2);
            CalibDbV2_YuvmeV1_t *yuvme_v1 = (CalibDbV2_YuvmeV1_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCalibdbV2, yuvme_v1));
            ayuvme_contex_v1->yuvme_v1 = *yuvme_v1;
            ayuvme_contex_v1->isIQParaUpdate = true;
            ayuvme_contex_v1->isReCalculate |= 1;
#endif
        }
        Ayuvme_Config_V1_t stAyuvmeConfigV1;
        stAyuvmeConfigV1.rawHeight =  params->u.prepare.sns_op_height;
        stAyuvmeConfigV1.rawWidth = params->u.prepare.sns_op_width;
        Ayuvme_result_V1_t ret_v1 = AYUVMEV1_RET_SUCCESS;
        ret_v1 = Ayuvme_Prepare_V1(ayuvme_contex_v1, &stAyuvmeConfigV1);
        if(ret_v1 != AYUVMEV1_RET_SUCCESS) {
            ret = XCAM_RETURN_ERROR_FAILED;
            LOGE_ANR("%s: config ANR failed (%d)\n", __FUNCTION__, ret);
        }
    }
    else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE_ANR("module_hw_version of ayuvme is isvalid!!!!");
    }

    LOGI_ANR("%s exit ret:%d\n", __FUNCTION__, ret);
    return ret;
}

static XCamReturn groupAyuvmeV1Processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    LOGI_ANR("%s enter \n", __FUNCTION__ );
    LOGI_ANR("----------------------------------------------frame_id (%d)----------------------------------------------\n", inparams->frame_id);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoCamGroupProcIn* procParaGroup = (RkAiqAlgoCamGroupProcIn*)inparams;
    RkAiqAlgoCamGroupProcOut* procResParaGroup = (RkAiqAlgoCamGroupProcOut*)outparams;
    CamGroup_AyuvmeV1_Contex_t * ayuvme_group_contex = (CamGroup_AyuvmeV1_Contex_t *)inparams->ctx;
    int deltaIso = 0;

    //method error
    if (ayuvme_group_contex->group_CalibV2.groupMethod <= CalibDbV2_CAMGROUP_AYUVMEV1_METHOD_MIN
            ||  ayuvme_group_contex->group_CalibV2.groupMethod >=  CalibDbV2_CAMGROUP_AYUVMEV1_METHOD_MAX) {
        return (ret);
    }

    //group empty
    if(procParaGroup->camgroupParmasArray == nullptr) {
        LOGE_ANR("camgroupParmasArray is null");
        return(XCAM_RETURN_ERROR_FAILED);
    }

    //get cur ae exposure
    Ayuvme_ExpInfo_V1_t stExpInfoV1;
    memset(&stExpInfoV1, 0x00, sizeof(Ayuvme_ExpInfo_V1_t));
    stExpInfoV1.hdr_mode = 0; //pAnrProcParams->hdr_mode;
    stExpInfoV1.snr_mode = 0;
    for(int i = 0; i < 3; i++) {
        stExpInfoV1.arIso[i] = 50;
        stExpInfoV1.arAGain[i] = 1.0;
        stExpInfoV1.arDGain[i] = 1.0;
        stExpInfoV1.arTime[i] = 0.01;
    }

    stExpInfoV1.blc_ob_predgain = 1.0f;
    if(procParaGroup != NULL) {
        LOGD_ANR(" predgain:%f\n",
                 procParaGroup->stAblcV32_proc_res.isp_ob_predgain);
        stExpInfoV1.blc_ob_predgain = procParaGroup->stAblcV32_proc_res.isp_ob_predgain;
    }
    //merge ae result, iso mean value
    rk_aiq_singlecam_3a_result_t* scam_3a_res = procParaGroup->camgroupParmasArray[0];
    if(scam_3a_res->aec._bEffAecExpValid) {
        RKAiqAecExpInfo_t* pCurExp = &scam_3a_res->aec._effAecExpInfo;
        stExpInfoV1.snr_mode = pCurExp->CISFeature.SNR;
        if((rk_aiq_working_mode_t)procParaGroup->working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
            stExpInfoV1.hdr_mode = 0;
            stExpInfoV1.arAGain[0] = pCurExp->LinearExp.exp_real_params.analog_gain;
            stExpInfoV1.arDGain[0] = pCurExp->LinearExp.exp_real_params.digital_gain;
            stExpInfoV1.arTime[0] = pCurExp->LinearExp.exp_real_params.integration_time;
            if(stExpInfoV1.arAGain[0] < 1.0) {
                stExpInfoV1.arAGain[0] = 1.0;
            }
            if(stExpInfoV1.arDGain[0] < 1.0) {
                stExpInfoV1.arDGain[0] = 1.0;
            }
            if(stExpInfoV1.blc_ob_predgain < 1.0) {
                stExpInfoV1.blc_ob_predgain = 1.0;
            }
            stExpInfoV1.arIso[0] = stExpInfoV1.arAGain[0] * stExpInfoV1.arDGain[0] * stExpInfoV1.blc_ob_predgain * 50;

        } else {
            if(procParaGroup->working_mode == RK_AIQ_ISP_HDR_MODE_2_FRAME_HDR
                    || procParaGroup->working_mode == RK_AIQ_ISP_HDR_MODE_2_LINE_HDR)
                stExpInfoV1.hdr_mode = 1;
            else if (procParaGroup->working_mode == RK_AIQ_ISP_HDR_MODE_3_FRAME_HDR
                     || procParaGroup->working_mode == RK_AIQ_ISP_HDR_MODE_3_LINE_HDR)
                stExpInfoV1.hdr_mode = 2;
            else {
                stExpInfoV1.hdr_mode = 0;
                LOGE_ANR("mode error\n");
            }

            for(int i = 0; i < 3; i++) {
                stExpInfoV1.arAGain[i] = pCurExp->HdrExp[i].exp_real_params.analog_gain;
                stExpInfoV1.arDGain[i] = pCurExp->HdrExp[i].exp_real_params.digital_gain;
                stExpInfoV1.arTime[i] = pCurExp->HdrExp[i].exp_real_params.integration_time;
                if(stExpInfoV1.arAGain[i] < 1.0) {
                    stExpInfoV1.arAGain[i] = 1.0;
                }
                if(stExpInfoV1.arDGain[i] < 1.0) {
                    stExpInfoV1.arDGain[i] = 1.0;
                }
                stExpInfoV1.blc_ob_predgain = 1.0;
                stExpInfoV1.arIso[i] = stExpInfoV1.arAGain[i] * stExpInfoV1.arDGain[i] * 50;
            }

        }
    } else {
        LOGW("fail to get sensor gain form AE module,use default value ");
    }



    if(CHECK_ISP_HW_V39()) {
        Ayuvme_Context_V1_t * ayuvme_contex_v1 = ayuvme_group_contex->ayuvme_contex_v1;
        Ayuvme_ProcResult_V1_t stAyuvmeResultV1;
        stAyuvmeResultV1.stFix = procResParaGroup->camgroupParmasArray[0]->ayuvme._ayuvme_procRes_v1;
        deltaIso = abs(stExpInfoV1.arIso[stExpInfoV1.hdr_mode] - ayuvme_contex_v1->stExpInfo.arIso[stExpInfoV1.hdr_mode]);
        if(deltaIso > AYUVMEV1_RECALCULATE_DELTA_ISO) {
            ayuvme_contex_v1->isReCalculate |= 1;
        }
        if(stExpInfoV1.blc_ob_predgain != ayuvme_contex_v1->stExpInfo.blc_ob_predgain) {
            ayuvme_contex_v1->isReCalculate |= 1;
        }	
        if(ayuvme_contex_v1->isReCalculate) {
            Ayuvme_result_V1_t ret_v1 = AYUVMEV1_RET_SUCCESS;
            ret_v1 = Ayuvme_Process_V1(ayuvme_contex_v1, &stExpInfoV1);
            if(ret_v1 != AYUVMEV1_RET_SUCCESS) {
                ret = XCAM_RETURN_ERROR_FAILED;
                LOGE_ANR("%s: processing ANR failed (%d)\n", __FUNCTION__, ret);
            }
            outparams->cfg_update = true;
            LOGD_ANR("recalculate: %d delta_iso:%d \n ", ayuvme_contex_v1->isReCalculate, deltaIso);
        } else {
            outparams->cfg_update = false;
        }
        Ayuvme_GetProcResult_V1(ayuvme_contex_v1, &stAyuvmeResultV1);
        for (int i = 0; i < procResParaGroup->arraySize; i++) {
            *(procResParaGroup->camgroupParmasArray[i]->ayuvme._ayuvme_procRes_v1) =
                *stAyuvmeResultV1.stFix;
            IS_UPDATE_MEM((procResParaGroup->camgroupParmasArray[i]->ayuvme._ayuvme_procRes_v1), procParaGroup->_offset_is_update) =
                outparams->cfg_update;
        }
        ayuvme_contex_v1->isReCalculate = 0;
    }
    else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE_ANR("module_hw_version of ayuvme is isvalid!!!!");
    }

    LOGI_ANR("%s exit\n", __FUNCTION__);
    return ret;
}

RkAiqAlgoDescription g_RkIspAlgoDescCamgroupAyuvmeV1 = {
    .common = {
        .version = RKISP_ALGO_CAMGROUP_AYUVMEV1_VERSION,
        .vendor  = RKISP_ALGO_CAMGROUP_AYUVMEV1_VENDOR,
        .description = RKISP_ALGO_CAMGROUP_AYUVMEV1_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_AMD,
        .id      = 0,
        .create_context  = groupAyuvmeV1CreateCtx,
        .destroy_context = groupAyuvmeV1DestroyCtx,
    },
    .prepare = groupAyuvmeV1Prepare,
    .pre_process = NULL,
    .processing = groupAyuvmeV1Processing,
    .post_process = NULL,
};

RKAIQ_END_DECLARE
