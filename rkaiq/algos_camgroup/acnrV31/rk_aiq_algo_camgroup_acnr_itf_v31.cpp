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

#include "rk_aiq_algo_camgroup_acnr_itf_v31.h"
#include "rk_aiq_algo_camgroup_types.h"
#include "rk_aiq_types_camgroup_acnr_prvt_v31.h"
#include "acnrV31/rk_aiq_acnr_algo_itf_v31.h"
#include "acnrV31/rk_aiq_acnr_algo_v31.h"


RKAIQ_BEGIN_DECLARE


static XCamReturn groupAcnrV31CreateCtx(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{
    LOGI_ANR("%s enter \n", __FUNCTION__ );

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    CamGroup_AcnrV31_Contex_t *acnr_group_contex = NULL;
    AlgoCtxInstanceCfgCamGroup *cfgInt = (AlgoCtxInstanceCfgCamGroup*)cfg;


    if(CHECK_ISP_HW_V39()) {
        acnr_group_contex = (CamGroup_AcnrV31_Contex_t*)malloc(sizeof(CamGroup_AcnrV31_Contex_t));
#if ACNR_USE_JSON_FILE_V31
        AcnrV31_result_t ret_v31 = ACNRV31_RET_SUCCESS;
        ret_v31 = Acnr_Init_V31(&(acnr_group_contex->acnr_contex_v31), (void *)cfgInt->s_calibv2);
        if(ret_v31 != ACNRV31_RET_SUCCESS) {
            ret = XCAM_RETURN_ERROR_FAILED;
            LOGE_ANR("%s: Initializaion ANR failed (%d)\n", __FUNCTION__, ret);
        }
#endif
    }
    else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE_ANR("module_hw_version of acnr  is invalid!!!!");
    }

    if(ret != XCAM_RETURN_NO_ERROR) {
        LOGE_ANR("%s: Initializaion group cnr failed (%d)\n", __FUNCTION__, ret);
    } else {
        // to do got acnrSurrViewClib and initinal paras for for surround view
        acnr_group_contex->group_CalibV31.groupMethod = CalibDbV2_CAMGROUP_ACNRV31_METHOD_MEAN;// to do from json
        acnr_group_contex->camera_Num = cfgInt->camIdArrayLen;

        *context = (RkAiqAlgoContext *)(acnr_group_contex);

        LOGI_ANR("%s:%d surrViewMethod(1-mean):%d, cameraNum %d \n",
                 __FUNCTION__, __LINE__,
                 acnr_group_contex->group_CalibV31.groupMethod,
                 acnr_group_contex->camera_Num);
    }

    LOGI_ANR("%s exit ret:%d\n", __FUNCTION__, ret);
    return ret;

}

static XCamReturn groupAcnrV31DestroyCtx(RkAiqAlgoContext *context)
{
    LOGI_ANR("%s enter \n", __FUNCTION__ );

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    CamGroup_AcnrV31_Contex_t *acnr_group_contex = (CamGroup_AcnrV31_Contex_t*)context;

    if(CHECK_ISP_HW_V39()) {
        AcnrV31_result_t ret_v31 = ACNRV31_RET_SUCCESS;
        ret_v31 = Acnr_Release_V31(acnr_group_contex->acnr_contex_v31);
        if(ret_v31 != ACNRV31_RET_SUCCESS) {
            ret = XCAM_RETURN_ERROR_FAILED;
            LOGE_ANR("%s: Initializaion ANR failed (%d)\n", __FUNCTION__, ret);
        }
    }
    else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE_ANR("module_hw_version of acr is isvalid!!!!");
    }

    if(ret != XCAM_RETURN_NO_ERROR) {
        LOGE_ANR("%s: release ANR failed (%d)\n", __FUNCTION__, ret);
    } else {
        free(acnr_group_contex);
    }


    LOGI_ANR("%s exit ret:%d\n", __FUNCTION__, ret);
    return ret;
}

static XCamReturn groupAcnrV31Prepare(RkAiqAlgoCom* params)
{
    LOGI_ANR("%s enter \n", __FUNCTION__ );

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    CamGroup_AcnrV31_Contex_t * acnr_group_contex = (CamGroup_AcnrV31_Contex_t *)params->ctx;
    RkAiqAlgoCamGroupPrepare* para = (RkAiqAlgoCamGroupPrepare*)params;

    if(CHECK_ISP_HW_V39()) {
        Acnr_Context_V31_t * acnr_contex_v31 = acnr_group_contex->acnr_contex_v31;
        if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )) {
            // todo  update calib pars for surround view
#if ACNR_USE_JSON_FILE_V31
            void *pCalibdbV31 = (void*)(para->s_calibv2);
            CalibDbV2_CNRV31_t *cnr_v31 = (CalibDbV2_CNRV31_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCalibdbV31, cnr_v31));
            acnr_contex_v31->cnr_v31 = *cnr_v31;
            acnr_contex_v31->isIQParaUpdate = true;
            acnr_contex_v31->isReCalculate |= 1;
#endif
        }
        Acnr_Config_V31_t stAcnrV31ConfigV31;
        stAcnrV31ConfigV31.rawHeight =  params->u.prepare.sns_op_height;
        stAcnrV31ConfigV31.rawWidth = params->u.prepare.sns_op_width;
        AcnrV31_result_t ret_v31 = ACNRV31_RET_SUCCESS;
        ret_v31 = Acnr_Prepare_V31(acnr_contex_v31, &stAcnrV31ConfigV31);
        if(ret_v31 != ACNRV31_RET_SUCCESS) {
            ret = XCAM_RETURN_ERROR_FAILED;
            LOGE_ANR("%s: config ANR failed (%d)\n", __FUNCTION__, ret);
        }
    }
    else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE_ANR("module_hw_version of acnr (%d) is isvalid!!!!");
    }

    LOGI_ANR("%s exit ret:%d\n", __FUNCTION__, ret);
    return ret;
}

static XCamReturn groupAcnrV31Processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    LOGI_ANR("%s enter \n", __FUNCTION__ );
    LOGI_ANR("----------------------------------------------frame_id (%d)----------------------------------------------\n", inparams->frame_id);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoCamGroupProcIn* procParaGroup = (RkAiqAlgoCamGroupProcIn*)inparams;
    RkAiqAlgoCamGroupProcOut* procResParaGroup = (RkAiqAlgoCamGroupProcOut*)outparams;
    CamGroup_AcnrV31_Contex_t * acnr_group_contex = (CamGroup_AcnrV31_Contex_t *)inparams->ctx;
    int deltaIso = 0;

    //method error
    if (acnr_group_contex->group_CalibV31.groupMethod <= CalibDbV2_CAMGROUP_ACNRV31_METHOD_MIN
            ||  acnr_group_contex->group_CalibV31.groupMethod >=  CalibDbV2_CAMGROUP_ACNRV31_METHOD_MAX) {
        return (ret);
    }

    //group empty
    if(procParaGroup->camgroupParmasArray == nullptr) {
        LOGE_ANR("camgroupParmasArray is null");
        return(XCAM_RETURN_ERROR_FAILED);
    }

    //get cur ae exposure
    AcnrV31_ExpInfo_t stExpInfoV31;
    memset(&stExpInfoV31, 0x00, sizeof(AcnrV31_ExpInfo_t));
    stExpInfoV31.hdr_mode = 0; //pAnrProcParams->hdr_mode;
    stExpInfoV31.snr_mode = 0;
    for(int i = 0; i < 3; i++) {
        stExpInfoV31.arIso[i] = 50;
        stExpInfoV31.arAGain[i] = 1.0;
        stExpInfoV31.arDGain[i] = 1.0;
        stExpInfoV31.arTime[i] = 0.01;
    }

#ifndef USE_NEWSTRUCT
    stExpInfoV31.blc_ob_predgain = 1.0f;
    if(procParaGroup != NULL) {
        LOGD_ANR(" predgain:%f\n",
                 procParaGroup->stAblcV32_proc_res.isp_ob_predgain);
        stExpInfoV31.blc_ob_predgain = procParaGroup->stAblcV32_proc_res.isp_ob_predgain;

    }
#endif

    //merge ae result, iso mean value
    rk_aiq_singlecam_3a_result_t* scam_3a_res = procParaGroup->camgroupParmasArray[0];
    if(scam_3a_res->aec._bEffAecExpValid) {
        RKAiqAecExpInfo_t* pCurExp = &scam_3a_res->aec._effAecExpInfo;
        stExpInfoV31.snr_mode = pCurExp->CISFeature.SNR;
        if((rk_aiq_working_mode_t)procParaGroup->working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
            stExpInfoV31.hdr_mode = 0;
            stExpInfoV31.arAGain[0] = pCurExp->LinearExp.exp_real_params.analog_gain;
            stExpInfoV31.arDGain[0] = pCurExp->LinearExp.exp_real_params.digital_gain;
            stExpInfoV31.arTime[0] = pCurExp->LinearExp.exp_real_params.integration_time;
            if(stExpInfoV31.arAGain[0] < 1.0) {
                stExpInfoV31.arAGain[0] = 1.0;
            }
            if(stExpInfoV31.arDGain[0] < 1.0) {
                stExpInfoV31.arDGain[0] = 1.0;
            }
            if(stExpInfoV31.blc_ob_predgain < 1.0) {
                stExpInfoV31.blc_ob_predgain = 1.0;
            }
            stExpInfoV31.arIso[0] = stExpInfoV31.arAGain[0] * stExpInfoV31.arDGain[0] * stExpInfoV31.blc_ob_predgain * 50;

        } else {
            if(procParaGroup->working_mode == RK_AIQ_ISP_HDR_MODE_2_FRAME_HDR
                    || procParaGroup->working_mode == RK_AIQ_ISP_HDR_MODE_2_LINE_HDR)
                stExpInfoV31.hdr_mode = 1;
            else if (procParaGroup->working_mode == RK_AIQ_ISP_HDR_MODE_3_FRAME_HDR
                     || procParaGroup->working_mode == RK_AIQ_ISP_HDR_MODE_3_LINE_HDR)
                stExpInfoV31.hdr_mode = 2;
            else {
                stExpInfoV31.hdr_mode = 0;
                LOGE_ANR("mode error\n");
            }
            stExpInfoV31.blc_ob_predgain = 1.0;
            for(int i = 0; i < 3; i++) {
                stExpInfoV31.arAGain[i] = pCurExp->HdrExp[i].exp_real_params.analog_gain > 1.0f ?
                                          pCurExp->HdrExp[i].exp_real_params.analog_gain : 1.0f;
                stExpInfoV31.arDGain[i] = pCurExp->HdrExp[i].exp_real_params.digital_gain > 1.0f ?
                                          pCurExp->HdrExp[i].exp_real_params.digital_gain : 1.0f;
                stExpInfoV31.arTime[i] = pCurExp->HdrExp[i].exp_real_params.integration_time;
                stExpInfoV31.arIso[i] = stExpInfoV31.arAGain[i] * stExpInfoV31.arDGain[i] * 50;
            }

        }
    } else {
        LOGW("fail to get sensor gain form AE module,use default value ");
    }



    if(CHECK_ISP_HW_V39()) {
        Acnr_Context_V31_t * acnr_contex_v31 = acnr_group_contex->acnr_contex_v31;
        Acnr_ProcResult_V31_t stAcnrResultV31;
        stAcnrResultV31.stFix = scam_3a_res->acnr._acnr_procRes_v31;
        deltaIso = abs(stExpInfoV31.arIso[stExpInfoV31.hdr_mode] - acnr_contex_v31->stExpInfo.arIso[stExpInfoV31.hdr_mode]);
        if(deltaIso > ACNRV31_RECALCULATE_DELTA_ISO) {
            acnr_contex_v31->isReCalculate |= 1;
        }
        if(stExpInfoV31.blc_ob_predgain != acnr_contex_v31->stExpInfo.blc_ob_predgain) {
            acnr_contex_v31->isReCalculate |= 1;
        }
        if(acnr_contex_v31->isReCalculate) {
            AcnrV31_result_t ret_v31 = ACNRV31_RET_SUCCESS;
            ret_v31 = Acnr_Process_V31(acnr_contex_v31, &stExpInfoV31);
            if(ret_v31 != ACNRV31_RET_SUCCESS) {
                ret = XCAM_RETURN_ERROR_FAILED;
                LOGE_ANR("%s: processing ANR failed (%d)\n", __FUNCTION__, ret);
            }
            outparams->cfg_update = true;
            LOGD_ANR("recalculate: %d delta_iso:%d \n ", acnr_contex_v31->isReCalculate, deltaIso);
        } else {
            outparams->cfg_update = false;
        }
        Acnr_GetProcResult_V31(acnr_contex_v31, &stAcnrResultV31);
        for (int i = 0; i < procResParaGroup->arraySize; i++) {
            *(procResParaGroup->camgroupParmasArray[i]->acnr._acnr_procRes_v31) = *stAcnrResultV31.stFix;
            IS_UPDATE_MEM((procResParaGroup->camgroupParmasArray[i]->acnr._acnr_procRes_v31), procParaGroup->_offset_is_update) =
                outparams->cfg_update;
        }
        acnr_contex_v31->isReCalculate = 0;
    }
    else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE_ANR("module_hw_version of acnr is isvalid!!!!");
    }

    LOGI_ANR("%s exit\n", __FUNCTION__);
    return ret;
}

RkAiqAlgoDescription g_RkIspAlgoDescCamgroupAcnrV31 = {
    .common = {
        .version = RKISP_ALGO_CAMGROUP_ACNRV31_VERSION,
        .vendor  = RKISP_ALGO_CAMGROUP_ACNRV31_VENDOR,
        .description = RKISP_ALGO_CAMGROUP_ACNRV31_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_ACNR,
        .id      = 0,
        .create_context  = groupAcnrV31CreateCtx,
        .destroy_context = groupAcnrV31DestroyCtx,
    },
    .prepare = groupAcnrV31Prepare,
    .pre_process = NULL,
    .processing = groupAcnrV31Processing,
    .post_process = NULL,
};

RKAIQ_END_DECLARE
