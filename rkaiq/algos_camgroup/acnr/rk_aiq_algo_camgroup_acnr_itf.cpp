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

#include "rk_aiq_algo_camgroup_acnr_itf.h"
#include "rk_aiq_algo_camgroup_types.h"
#include "rk_aiq_types_camgroup_acnr_prvt.h"
#include "acnr/rk_aiq_acnr_algo_itf_v1.h"
#include "acnr/rk_aiq_acnr_algo_v1.h"



RKAIQ_BEGIN_DECLARE


static XCamReturn groupAcnrCreateCtx(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{
    LOGI_ANR("%s enter \n", __FUNCTION__ );

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    CamGroup_Acnr_Contex_t *acnr_group_contex = NULL;
    AlgoCtxInstanceCfgCamGroup *cfgInt = (AlgoCtxInstanceCfgCamGroup*)cfg;
    //g_acnr_hw_ver = (acnr_hardware_version_t)(cfgInt->cfg_com.module_hw_version);


    if(CHECK_ISP_HW_V21()) {
        acnr_group_contex = (CamGroup_Acnr_Contex_t*)malloc(sizeof(CamGroup_Acnr_Contex_t));
#if (ACNR_USE_JSON_FILE_V1)
        Acnr_result_t ret_v1 = ACNR_RET_SUCCESS;
        ret_v1 = Acnr_Init_V1(&(acnr_group_contex->acnr_contex_v1), (void *)cfgInt->s_calibv2);
        if(ret_v1 != ACNR_RET_SUCCESS) {
            ret = XCAM_RETURN_ERROR_FAILED;
            LOGE_ANR("%s: Initializaion ANR failed (%d)\n", __FUNCTION__, ret);
        }
#endif
    } else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE_ANR("module_hw_version of acnr is invalid!!!!");
    }

    if(ret != XCAM_RETURN_NO_ERROR) {
        LOGE_ANR("%s: Initializaion group cnr failed (%d)\n", __FUNCTION__, ret);
    } else {
        // to do got acnrSurrViewClib and initinal paras for for surround view
        acnr_group_contex->group_CalibV2.groupMethod = CalibDbV2_CAMGROUP_ACNR_METHOD_MEAN;// to do from json
        acnr_group_contex->camera_Num = cfgInt->camIdArrayLen;

        *context = (RkAiqAlgoContext *)(acnr_group_contex);

        LOGI_ANR("%s:%d surrViewMethod(1-mean):%d, cameraNum %d \n",
                 __FUNCTION__, __LINE__,
                 acnr_group_contex->group_CalibV2.groupMethod,
                 acnr_group_contex->camera_Num);
    }

    LOGI_ANR("%s exit ret:%d\n", __FUNCTION__, ret);
    return ret;

}

static XCamReturn groupAcnrDestroyCtx(RkAiqAlgoContext *context)
{
    LOGI_ANR("%s enter \n", __FUNCTION__ );

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    CamGroup_Acnr_Contex_t *acnr_group_contex = (CamGroup_Acnr_Contex_t*)context;

    if(CHECK_ISP_HW_V21()) {
        Acnr_result_t ret_v1 = ACNR_RET_SUCCESS;
        ret_v1  = Acnr_Release_V1(acnr_group_contex->acnr_contex_v1);
        if(ret_v1  != ACNR_RET_SUCCESS) {
            ret = XCAM_RETURN_ERROR_FAILED;
            LOGE_ANR("%s: Initializaion ANR failed (%d)\n", __FUNCTION__, ret);
        }
    }
    else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE_ANR("module_hw_version of acnr is isvalid!!!!");
    }

    if(ret != XCAM_RETURN_NO_ERROR) {
        LOGE_ANR("%s: release ANR failed (%d)\n", __FUNCTION__, ret);
    } else {
        free(acnr_group_contex);
    }


    LOGI_ANR("%s exit ret:%d\n", __FUNCTION__, ret);
    return ret;
}

static XCamReturn groupAcnrPrepare(RkAiqAlgoCom* params)
{
    LOGI_ANR("%s enter \n", __FUNCTION__ );

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    CamGroup_Acnr_Contex_t * acnr_group_contex = (CamGroup_Acnr_Contex_t *)params->ctx;
    RkAiqAlgoCamGroupPrepare* para = (RkAiqAlgoCamGroupPrepare*)params;

    if(CHECK_ISP_HW_V21()) {
        Acnr_Context_V1_t * acnr_contex_v1 = acnr_group_contex->acnr_contex_v1;
        acnr_contex_v1->prepare_type = params->u.prepare.conf_type;
        if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )) {
            // todo  update calib pars for surround view
#if ACNR_USE_JSON_FILE_V1
            void *pCalibdbV2 = (void*)(para->s_calibv2);
            CalibDbV2_CNR_t *cnr_v1 = (CalibDbV2_CNR_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCalibdbV2, cnr_v1));
            acnr_contex_v1->cnr_v1 = *cnr_v1;
            acnr_contex_v1->isIQParaUpdate = true;
            acnr_contex_v1->isReCalculate |= 1;
#endif
        }
        Acnr_Config_V1_t stAcnrConfigV1;
        stAcnrConfigV1.rawHeight =  params->u.prepare.sns_op_height;
        stAcnrConfigV1.rawWidth = params->u.prepare.sns_op_width;
        Acnr_result_t ret_v1 = ACNR_RET_SUCCESS;
        ret_v1 = Acnr_Prepare_V1(acnr_contex_v1, &stAcnrConfigV1);
        if(ret_v1 != ACNR_RET_SUCCESS) {
            ret = XCAM_RETURN_ERROR_FAILED;
            LOGE_ANR("%s: config ANR failed (%d)\n", __FUNCTION__, ret);
        }
    }
    else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE_ANR("module_hw_version of acnr is isvalid!!!!");
    }

    LOGI_ANR("%s exit ret:%d\n", __FUNCTION__, ret);
    return ret;
}

static XCamReturn groupAcnrProcessing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    LOGI_ANR("%s enter \n", __FUNCTION__ );
    LOGI_ANR("----------------------------------------------frame_id (%d)----------------------------------------------\n", inparams->frame_id);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoCamGroupProcIn* procParaGroup = (RkAiqAlgoCamGroupProcIn*)inparams;
    RkAiqAlgoCamGroupProcOut* procResParaGroup = (RkAiqAlgoCamGroupProcOut*)outparams;
    CamGroup_Acnr_Contex_t * acnr_group_contex = (CamGroup_Acnr_Contex_t *)inparams->ctx;
    int deltaIso = 0;

    //method error
    if (acnr_group_contex->group_CalibV2.groupMethod <= CalibDbV2_CAMGROUP_ACNR_METHOD_MIN
            ||  acnr_group_contex->group_CalibV2.groupMethod >=  CalibDbV2_CAMGROUP_ACNR_METHOD_MAX) {
        return (ret);
    }

    //group empty
    if(procParaGroup->camgroupParmasArray == nullptr) {
        LOGE_ANR("camgroupParmasArray is null");
        return(XCAM_RETURN_ERROR_FAILED);
    }

    //get cur ae exposure
    Acnr_ExpInfo_t stExpInfoV1;
    memset(&stExpInfoV1, 0x00, sizeof(stExpInfoV1));
    stExpInfoV1.hdr_mode = 0; //pAnrProcParams->hdr_mode;
    stExpInfoV1.snr_mode = 0;
    for(int i = 0; i < 3; i++) {
        stExpInfoV1.arIso[i] = 50;
        stExpInfoV1.arAGain[i] = 1.0;
        stExpInfoV1.arDGain[i] = 1.0;
        stExpInfoV1.arTime[i] = 0.01;
    }


    //merge ae result, iso mean value
    rk_aiq_singlecam_3a_result_t* scam_3a_res = procParaGroup->camgroupParmasArray[0];
    if(scam_3a_res->aec._bEffAecExpValid) {
        RKAiqAecExpInfo_t* pCurExp = &scam_3a_res->aec._effAecExpInfo;
        stExpInfoV1.snr_mode = pCurExp->CISFeature.SNR;
        if((rk_aiq_working_mode_t)procParaGroup->working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
            stExpInfoV1.hdr_mode = 0;
            stExpInfoV1.arAGain[0] = pCurExp->LinearExp.exp_real_params.analog_gain;
            stExpInfoV1.arDGain[0] = pCurExp->LinearExp.exp_real_params.digital_gain * pCurExp->LinearExp.exp_real_params.isp_dgain;
            stExpInfoV1.arTime[0] = pCurExp->LinearExp.exp_real_params.integration_time;
            stExpInfoV1.arIso[0] = stExpInfoV1.arAGain[0] * stExpInfoV1.arDGain[0] * 50;

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
                stExpInfoV1.arIso[i] = stExpInfoV1.arAGain[i] * stExpInfoV1.arDGain[i] * 50;
            }

        }
    } else {
        LOGW("fail to get sensor gain form AE module,use default value ");
    }



    if(CHECK_ISP_HW_V21()) {
        Acnr_Context_V1_t * acnr_contex_v1 = acnr_group_contex->acnr_contex_v1;
        Acnr_ProcResult_V1_t stAcnrResultV1;
        RK_CNR_Fix_V1_t stFix;
        stAcnrResultV1.stFix = &stFix;

        deltaIso = abs(stExpInfoV1.arIso[stExpInfoV1.hdr_mode] - acnr_contex_v1->stExpInfo.arIso[stExpInfoV1.hdr_mode]);
        if(deltaIso > ACNRV1_RECALCULATE_DELTA_ISO) {
            acnr_contex_v1->isReCalculate |= 1;
        }
        if(acnr_contex_v1->isReCalculate) {
            Acnr_result_t ret_v1 = ACNR_RET_SUCCESS;
            ret_v1 = Acnr_Process_V1(acnr_contex_v1, &stExpInfoV1);
            if(ret_v1 != ACNR_RET_SUCCESS) {
                ret = XCAM_RETURN_ERROR_FAILED;
                LOGE_ANR("%s: processing ANR failed (%d)\n", __FUNCTION__, ret);
            }
            outparams->cfg_update = true;
            LOGD_ANR("recalculate: %d delta_iso:%d \n ", acnr_contex_v1->isReCalculate, deltaIso);
        } else {
            outparams->cfg_update = false;
        }
        Acnr_GetProcResult_V1(acnr_contex_v1, &stAcnrResultV1);
        for (int i = 0; i < procResParaGroup->arraySize; i++) {
            *(procResParaGroup->camgroupParmasArray[i]->acnr._acnr_procRes_v1) = *stAcnrResultV1.stFix;
            IS_UPDATE_MEM((procResParaGroup->camgroupParmasArray[i]->acnr._acnr_procRes_v1), procParaGroup->_offset_is_update) =
                outparams->cfg_update;
        }
        acnr_contex_v1->isReCalculate = 0;

    }
    else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE_ANR("module_hw_version of acnr is isvalid!!!!");
    }

    LOGI_ANR("%s exit\n", __FUNCTION__);
    return ret;
}

RkAiqAlgoDescription g_RkIspAlgoDescCamgroupAcnr = {
    .common = {
        .version = RKISP_ALGO_CAMGROUP_ACNR_VERSION,
        .vendor  = RKISP_ALGO_CAMGROUP_ACNR_VENDOR,
        .description = RKISP_ALGO_CAMGROUP_ACNR_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_ACNR,
        .id      = 0,
        .create_context  = groupAcnrCreateCtx,
        .destroy_context = groupAcnrDestroyCtx,
    },
    .prepare = groupAcnrPrepare,
    .pre_process = NULL,
    .processing = groupAcnrProcessing,
    .post_process = NULL,
};

RKAIQ_END_DECLARE
