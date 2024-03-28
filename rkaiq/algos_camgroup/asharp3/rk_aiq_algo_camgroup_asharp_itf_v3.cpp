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

#include "rk_aiq_algo_camgroup_asharp_itf_v3.h"
#include "rk_aiq_algo_camgroup_types.h"
#include "rk_aiq_types_camgroup_asharp_prvt_v3.h"
#include "asharp3/rk_aiq_asharp_algo_itf_v3.h"
#include "asharp3/rk_aiq_asharp_algo_v3.h"



RKAIQ_BEGIN_DECLARE


static XCamReturn groupAsharpV3CreateCtx(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{
    LOGI_ASHARP("%s enter \n", __FUNCTION__ );

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    CamGroup_AsharpV3_Contex_t *asharp_group_contex = NULL;
    AlgoCtxInstanceCfgCamGroup *cfgInt = (AlgoCtxInstanceCfgCamGroup*)cfg;


    if(CHECK_ISP_HW_V21()) {
        asharp_group_contex = (CamGroup_AsharpV3_Contex_t*)malloc(sizeof(CamGroup_AsharpV3_Contex_t));
#if ASHARP_USE_JSON_FILE_V3
        Asharp3_result_t ret_v3 = ASHARP3_RET_SUCCESS;
        ret_v3 = Asharp_Init_V3(&(asharp_group_contex->asharp_contex_v3), (void *)cfgInt->s_calibv2);
        if(ret_v3 != ASHARP3_RET_SUCCESS) {
            ret = XCAM_RETURN_ERROR_FAILED;
            LOGE_ASHARP("%s: Initializaion asharp failed (%d)\n", __FUNCTION__, ret);
        }
#endif
    }
    else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE_ASHARP("module_hw_version of asharp is invalid!!!!");
    }

    if(ret != XCAM_RETURN_NO_ERROR) {
        LOGE_ASHARP("%s: Initializaion group sharp failed (%d)\n", __FUNCTION__, ret);
    } else {
        // to do got asharpSurrViewClib and initinal paras for for surround view
        asharp_group_contex->group_CalibV2.groupMethod = CalibDbV2_CAMGROUP_ASHARPV3_METHOD_MEAN;// to do from json
        asharp_group_contex->camera_Num = cfgInt->camIdArrayLen;

        *context = (RkAiqAlgoContext *)(asharp_group_contex);

        LOGI_ASHARP("%s:%d surrViewMethod(1-mean):%d, cameraNum %d \n",
                    __FUNCTION__, __LINE__,
                    asharp_group_contex->group_CalibV2.groupMethod,
                    asharp_group_contex->camera_Num);
    }

    LOGI_ASHARP("%s exit ret:%d\n", __FUNCTION__, ret);
    return ret;

}

static XCamReturn groupAsharpV3DestroyCtx(RkAiqAlgoContext *context)
{
    LOGI_ASHARP("%s enter \n", __FUNCTION__ );

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    CamGroup_AsharpV3_Contex_t *asharp_group_contex = (CamGroup_AsharpV3_Contex_t*)context;

    if(CHECK_ISP_HW_V21()) {
        Asharp3_result_t ret_v3 = ASHARP3_RET_SUCCESS;
        ret_v3 = Asharp_Release_V3(asharp_group_contex->asharp_contex_v3);
        if(ret_v3 != ASHARP3_RET_SUCCESS) {
            ret = XCAM_RETURN_ERROR_FAILED;
            LOGE_ASHARP("%s: Initializaion ASHARP failed (%d)\n", __FUNCTION__, ret);
        }
    }
    else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE_ASHARP("module_hw_version of asharp is isvalid!!!!");
    }

    if(ret != XCAM_RETURN_NO_ERROR) {
        LOGE_ASHARP("%s: release asharp group failed (%d)\n", __FUNCTION__, ret);
    } else {
        free(asharp_group_contex);
    }


    LOGI_ASHARP("%s exit ret:%d\n", __FUNCTION__, ret);
    return ret;
}

static XCamReturn groupAsharpV3Prepare(RkAiqAlgoCom* params)
{
    LOGI_ASHARP("%s enter \n", __FUNCTION__ );

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    CamGroup_AsharpV3_Contex_t * asharp_group_contex = (CamGroup_AsharpV3_Contex_t *)params->ctx;
    RkAiqAlgoCamGroupPrepare* para = (RkAiqAlgoCamGroupPrepare*)params;

    if(CHECK_ISP_HW_V21()) {
        Asharp_Context_V3_t * asharp_contex_v3 = asharp_group_contex->asharp_contex_v3;
		asharp_contex_v3->prepare_type = params->u.prepare.conf_type;
        if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )) {
            // todo  update calib pars for surround view
#if ASHARP_USE_JSON_FILE_V3
            void *pCalibdbV2 = (void*)(para->s_calibv2);
            CalibDbV2_SharpV3_t *sharp_v3 = (CalibDbV2_SharpV3_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCalibdbV2, sharp_v3));
            asharp_contex_v3->sharp_v3 = *sharp_v3;
            asharp_contex_v3->isIQParaUpdate = true;
            asharp_contex_v3->isReCalculate |= 1;
#endif
        }
        Asharp_Config_V3_t stAsharpConfigV3;
        stAsharpConfigV3.rawHeight =  params->u.prepare.sns_op_height;
        stAsharpConfigV3.rawWidth = params->u.prepare.sns_op_width;
        Asharp3_result_t ret_v3 = ASHARP3_RET_SUCCESS;
        ret_v3 = Asharp_Prepare_V3(asharp_contex_v3, &stAsharpConfigV3);
        if(ret_v3 != ASHARP3_RET_SUCCESS) {
            ret = XCAM_RETURN_ERROR_FAILED;
            LOGE_ASHARP("%s: config asharp group failed (%d)\n", __FUNCTION__, ret);
        }
    }
    else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE_ASHARP("module_hw_version of asharp  is isvalid!!!!");
    }

    LOGI_ASHARP("%s exit ret:%d\n", __FUNCTION__, ret);
    return ret;
}

static XCamReturn groupAsharpV3Processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    LOGI_ASHARP("%s enter \n", __FUNCTION__ );
    LOGI_ASHARP("----------------------------------------------frame_id (%d)----------------------------------------------\n", inparams->frame_id);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoCamGroupProcIn* procParaGroup = (RkAiqAlgoCamGroupProcIn*)inparams;
    RkAiqAlgoCamGroupProcOut* procResParaGroup = (RkAiqAlgoCamGroupProcOut*)outparams;
    CamGroup_AsharpV3_Contex_t * asharp_group_contex = (CamGroup_AsharpV3_Contex_t *)inparams->ctx;
    int deltaIso = 0;

    //method error
    if (asharp_group_contex->group_CalibV2.groupMethod <= CalibDbV2_CAMGROUP_ASHARPV3_METHOD_MIN
            ||  asharp_group_contex->group_CalibV2.groupMethod >=  CalibDbV2_CAMGROUP_ASHARPV3_METHOD_MAX) {
        return (ret);
    }

    //group empty
    if(procParaGroup->camgroupParmasArray == nullptr) {
        LOGE_ASHARP("camgroupParmasArray is null");
        return(XCAM_RETURN_ERROR_FAILED);
    }

    //get cur ae exposure
    Asharp3_ExpInfo_t stExpInfoV3;
    memset(&stExpInfoV3, 0x00, sizeof(Asharp3_ExpInfo_t));
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
            stExpInfoV3.arDGain[0] = pCurExp->LinearExp.exp_real_params.digital_gain * pCurExp->LinearExp.exp_real_params.isp_dgain;
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



    if(CHECK_ISP_HW_V21()) {
        Asharp_Context_V3_t * asharp_contex_v3 = asharp_group_contex->asharp_contex_v3;
        Asharp_ProcResult_V3_t stAsharpResultV3;
        RK_SHARP_Fix_V3_t stFix;
        stAsharpResultV3.stFix = &stFix;

        deltaIso = abs(stExpInfoV3.arIso[stExpInfoV3.hdr_mode] - asharp_contex_v3->stExpInfo.arIso[stExpInfoV3.hdr_mode]);
        if(deltaIso > ASHARPV3_RECALCULATE_DELTA_ISO) {
            asharp_contex_v3->isReCalculate |= 1;
        }
        if(asharp_contex_v3->isReCalculate) {
            Asharp3_result_t ret_v3 = ASHARP3_RET_SUCCESS;
            ret_v3 = Asharp_Process_V3(asharp_contex_v3, &stExpInfoV3);
            if(ret_v3 != ASHARP3_RET_SUCCESS) {
                ret = XCAM_RETURN_ERROR_FAILED;
                LOGE_ASHARP("%s: processing ASHARP failed (%d)\n", __FUNCTION__, ret);
            }
            outparams->cfg_update = true;
            LOGD_ASHARP("recalculate: %d delta_iso:%d \n ", asharp_contex_v3->isReCalculate, deltaIso);
        } else {
            outparams->cfg_update = false;
        }
        Asharp_GetProcResult_V3(asharp_contex_v3, &stAsharpResultV3);
        for (int i = 0; i < procResParaGroup->arraySize; i++) {
            *(procResParaGroup->camgroupParmasArray[i]->asharp._asharp_procRes_v3) = *stAsharpResultV3.stFix;
            IS_UPDATE_MEM((procResParaGroup->camgroupParmasArray[i]->asharp._asharp_procRes_v3), procParaGroup->_offset_is_update) =
                outparams->cfg_update;
        }
        asharp_contex_v3->isReCalculate = 0;
    }
    else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE_ASHARP("module_hw_version of asharp is isvalid!!!!");
    }

    LOGI_ASHARP("%s exit\n", __FUNCTION__);
    return ret;
}

RkAiqAlgoDescription g_RkIspAlgoDescCamgroupAsharpV3 = {
    .common = {
        .version = RKISP_ALGO_CAMGROUP_ASHARPV3_VERSION,
        .vendor  = RKISP_ALGO_CAMGROUP_ASHARPV3_VENDOR,
        .description = RKISP_ALGO_CAMGROUP_ASHARPV3_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_ASHARP,
        .id      = 0,
        .create_context  = groupAsharpV3CreateCtx,
        .destroy_context = groupAsharpV3DestroyCtx,
    },
    .prepare = groupAsharpV3Prepare,
    .pre_process = NULL,
    .processing = groupAsharpV3Processing,
    .post_process = NULL,
};

RKAIQ_END_DECLARE
