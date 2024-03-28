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

#include "rk_aiq_algo_camgroup_acnr_itf_v30.h"
#include "rk_aiq_algo_camgroup_types.h"
#include "rk_aiq_types_camgroup_acnr_prvt_v30.h"
#include "acnrV30/rk_aiq_acnr_algo_itf_v30.h"
#include "acnrV30/rk_aiq_acnr_algo_v30.h"


RKAIQ_BEGIN_DECLARE


static XCamReturn groupAcnrV30CreateCtx(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{
    LOGI_ANR("%s enter \n", __FUNCTION__ );

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    CamGroup_AcnrV30_Contex_t *acnr_group_contex = NULL;
    AlgoCtxInstanceCfgCamGroup *cfgInt = (AlgoCtxInstanceCfgCamGroup*)cfg;


    if(CHECK_ISP_HW_V39() || CHECK_ISP_HW_V32() || CHECK_ISP_HW_V32_LITE()) {
        acnr_group_contex = (CamGroup_AcnrV30_Contex_t*)malloc(sizeof(CamGroup_AcnrV30_Contex_t));
#if ACNR_USE_JSON_FILE_V30
        AcnrV30_result_t ret_v30 = ACNRV30_RET_SUCCESS;
        ret_v30 = Acnr_Init_V30(&(acnr_group_contex->acnr_contex_v30), (void *)cfgInt->s_calibv2);
        if(ret_v30 != ACNRV30_RET_SUCCESS) {
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
        acnr_group_contex->group_CalibV30.groupMethod = CalibDbV2_CAMGROUP_ACNRV30_METHOD_MEAN;// to do from json
        acnr_group_contex->camera_Num = cfgInt->camIdArrayLen;

        *context = (RkAiqAlgoContext *)(acnr_group_contex);

        LOGI_ANR("%s:%d surrViewMethod(1-mean):%d, cameraNum %d \n",
                 __FUNCTION__, __LINE__,
                 acnr_group_contex->group_CalibV30.groupMethod,
                 acnr_group_contex->camera_Num);
    }

    LOGI_ANR("%s exit ret:%d\n", __FUNCTION__, ret);
    return ret;

}

static XCamReturn groupAcnrV30DestroyCtx(RkAiqAlgoContext *context)
{
    LOGI_ANR("%s enter \n", __FUNCTION__ );

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    CamGroup_AcnrV30_Contex_t *acnr_group_contex = (CamGroup_AcnrV30_Contex_t*)context;

    if(CHECK_ISP_HW_V39() || CHECK_ISP_HW_V32() || CHECK_ISP_HW_V32_LITE()) {
        AcnrV30_result_t ret_v30 = ACNRV30_RET_SUCCESS;
        ret_v30 = Acnr_Release_V30(acnr_group_contex->acnr_contex_v30);
        if(ret_v30 != ACNRV30_RET_SUCCESS) {
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

static XCamReturn groupAcnrV30Prepare(RkAiqAlgoCom* params)
{
    LOGI_ANR("%s enter \n", __FUNCTION__ );

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    CamGroup_AcnrV30_Contex_t * acnr_group_contex = (CamGroup_AcnrV30_Contex_t *)params->ctx;
    RkAiqAlgoCamGroupPrepare* para = (RkAiqAlgoCamGroupPrepare*)params;

    if(CHECK_ISP_HW_V39() || CHECK_ISP_HW_V32() || CHECK_ISP_HW_V32_LITE()) {
        Acnr_Context_V30_t * acnr_contex_v30 = acnr_group_contex->acnr_contex_v30;
        acnr_contex_v30->prepare_type = params->u.prepare.conf_type;
        if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )) {
            // todo  update calib pars for surround view
#if ACNR_USE_JSON_FILE_V30
            void *pCalibdbV30 = (void*)(para->s_calibv2);
            CalibDbV2_CNRV30_t *cnr_v30 = (CalibDbV2_CNRV30_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCalibdbV30, cnr_v30));
            acnr_contex_v30->cnr_v30 = *cnr_v30;
            acnr_contex_v30->isIQParaUpdate = true;
            acnr_contex_v30->isReCalculate |= 1;
#endif
        }
        Acnr_Config_V30_t stAcnrV30ConfigV30;
        stAcnrV30ConfigV30.rawHeight =  params->u.prepare.sns_op_height;
        stAcnrV30ConfigV30.rawWidth = params->u.prepare.sns_op_width;
        AcnrV30_result_t ret_v30 = ACNRV30_RET_SUCCESS;
        ret_v30 = Acnr_Prepare_V30(acnr_contex_v30, &stAcnrV30ConfigV30);
        if(ret_v30 != ACNRV30_RET_SUCCESS) {
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

static XCamReturn groupAcnrV30Processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    LOGI_ANR("%s enter \n", __FUNCTION__ );
    LOGI_ANR("----------------------------------------------frame_id (%d)----------------------------------------------\n", inparams->frame_id);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoCamGroupProcIn* procParaGroup = (RkAiqAlgoCamGroupProcIn*)inparams;
    RkAiqAlgoCamGroupProcOut* procResParaGroup = (RkAiqAlgoCamGroupProcOut*)outparams;
    CamGroup_AcnrV30_Contex_t * acnr_group_contex = (CamGroup_AcnrV30_Contex_t *)inparams->ctx;
    int deltaIso = 0;

    //method error
    if (acnr_group_contex->group_CalibV30.groupMethod <= CalibDbV2_CAMGROUP_ACNRV30_METHOD_MIN
            ||  acnr_group_contex->group_CalibV30.groupMethod >=  CalibDbV2_CAMGROUP_ACNRV30_METHOD_MAX) {
        return (ret);
    }

    //group empty
    if(procParaGroup == nullptr || procParaGroup->camgroupParmasArray == nullptr) {
        LOGE_ANR("procParaGroup or camgroupParmasArray is null");
        return(XCAM_RETURN_ERROR_FAILED);
    }

    //get cur ae exposure
    AcnrV30_ExpInfo_t stExpInfoV30;
    memset(&stExpInfoV30, 0x00, sizeof(AcnrV30_ExpInfo_t));
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
            stExpInfoV30.blc_ob_predgain = 1.0;
            for(int i = 0; i < 3; i++) {
                stExpInfoV30.arAGain[i] = pCurExp->HdrExp[i].exp_real_params.analog_gain > 1.0f ?
                                          pCurExp->HdrExp[i].exp_real_params.analog_gain : 1.0f;
                stExpInfoV30.arDGain[i] = pCurExp->HdrExp[i].exp_real_params.digital_gain > 1.0f ?
                                          pCurExp->HdrExp[i].exp_real_params.digital_gain : 1.0f;
                stExpInfoV30.arTime[i] = pCurExp->HdrExp[i].exp_real_params.integration_time;
                stExpInfoV30.arIso[i] = stExpInfoV30.arAGain[i] * stExpInfoV30.arDGain[i] * 50;
            }

        }
    } else {
        LOGW("fail to get sensor gain form AE module,use default value ");
    }



    if(CHECK_ISP_HW_V39() || CHECK_ISP_HW_V32() || CHECK_ISP_HW_V32_LITE()) {
        Acnr_Context_V30_t * acnr_contex_v30 = acnr_group_contex->acnr_contex_v30;
        Acnr_ProcResult_V30_t stAcnrResultV30;
        RK_CNR_Fix_V30_t stFix;
        stAcnrResultV30.stFix = &stFix;

        deltaIso = abs(stExpInfoV30.arIso[stExpInfoV30.hdr_mode] - acnr_contex_v30->stExpInfo.arIso[stExpInfoV30.hdr_mode]);
        if(deltaIso > ACNRV30_RECALCULATE_DELTA_ISO) {
            acnr_contex_v30->isReCalculate |= 1;
        }
        if(stExpInfoV30.blc_ob_predgain != acnr_contex_v30->stExpInfo.blc_ob_predgain) {
            acnr_contex_v30->isReCalculate |= 1;
        }
        if(acnr_contex_v30->isReCalculate) {
            AcnrV30_result_t ret_v30 = ACNRV30_RET_SUCCESS;
            ret_v30 = Acnr_Process_V30(acnr_contex_v30, &stExpInfoV30);
            if(ret_v30 != ACNRV30_RET_SUCCESS) {
                ret = XCAM_RETURN_ERROR_FAILED;
                LOGE_ANR("%s: processing ANR failed (%d)\n", __FUNCTION__, ret);
            }
            outparams->cfg_update = true;
            LOGD_ANR("recalculate: %d delta_iso:%d \n ", acnr_contex_v30->isReCalculate, deltaIso);
        } else {
            outparams->cfg_update = false;
        }
        Acnr_GetProcResult_V30(acnr_contex_v30, &stAcnrResultV30);
        for (int i = 0; i < procResParaGroup->arraySize; i++) {
            *(procResParaGroup->camgroupParmasArray[i]->acnr._acnr_procRes_v30) = *stAcnrResultV30.stFix;
            IS_UPDATE_MEM((procResParaGroup->camgroupParmasArray[i]->acnr._acnr_procRes_v30), procParaGroup->_offset_is_update) =
                outparams->cfg_update;
        }
        acnr_contex_v30->isReCalculate = 0;
    }
    else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE_ANR("module_hw_version of acnr is isvalid!!!!");
    }

    LOGI_ANR("%s exit\n", __FUNCTION__);
    return ret;
}

RkAiqAlgoDescription g_RkIspAlgoDescCamgroupAcnrV30 = {
    .common = {
        .version = RKISP_ALGO_CAMGROUP_ACNRV30_VERSION,
        .vendor  = RKISP_ALGO_CAMGROUP_ACNRV30_VENDOR,
        .description = RKISP_ALGO_CAMGROUP_ACNRV30_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_ACNR,
        .id      = 0,
        .create_context  = groupAcnrV30CreateCtx,
        .destroy_context = groupAcnrV30DestroyCtx,
    },
    .prepare = groupAcnrV30Prepare,
    .pre_process = NULL,
    .processing = groupAcnrV30Processing,
    .post_process = NULL,
};

RKAIQ_END_DECLARE
