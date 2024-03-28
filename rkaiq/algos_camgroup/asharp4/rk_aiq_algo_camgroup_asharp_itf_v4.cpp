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

#include "rk_aiq_algo_camgroup_asharp_itf_v4.h"
#include "rk_aiq_algo_camgroup_types.h"
#include "rk_aiq_types_camgroup_asharp_prvt_v4.h"
#include "asharp4/rk_aiq_asharp_algo_itf_v4.h"
#include "asharp4/rk_aiq_asharp_algo_v4.h"



RKAIQ_BEGIN_DECLARE


static XCamReturn groupAsharpV4CreateCtx(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{
    LOGI_ASHARP("%s enter \n", __FUNCTION__ );

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    CamGroup_AsharpV4_Contex_t *asharp_group_contex = NULL;
    AlgoCtxInstanceCfgCamGroup *cfgInt = (AlgoCtxInstanceCfgCamGroup*)cfg;

    if(CHECK_ISP_HW_V30()) {
        asharp_group_contex = (CamGroup_AsharpV4_Contex_t*)malloc(sizeof(CamGroup_AsharpV4_Contex_t));
#if (ASHARP_USE_JSON_FILE_V4)
        Asharp4_result_t ret_v4 = ASHARP4_RET_SUCCESS;
        ret_v4 = Asharp_Init_V4(&(asharp_group_contex->asharp_contex_v4), (void *)cfgInt->s_calibv2);
        if(ret_v4 != ASHARP4_RET_SUCCESS) {
            ret = XCAM_RETURN_ERROR_FAILED;
            LOGE_ASHARP("%s: Initializaion ASHARP failed (%d)\n", __FUNCTION__, ret);
        }
#endif
    } else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE_ASHARP("module_hw_version of asharp  is invalid!!!!");
    }

    if(ret != XCAM_RETURN_NO_ERROR) {
        LOGE_ASHARP("%s: Initializaion group sharp failed (%d)\n", __FUNCTION__, ret);
    } else {
        // to do got asharpSurrViewClib and initinal paras for for surround view
        asharp_group_contex->group_CalibV2.groupMethod = CalibDbV2_CAMGROUP_ASHARPV4_METHOD_MEAN;// to do from json
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

static XCamReturn groupAsharpV4DestroyCtx(RkAiqAlgoContext *context)
{
    LOGI_ASHARP("%s enter \n", __FUNCTION__ );

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    CamGroup_AsharpV4_Contex_t *asharp_group_contex = (CamGroup_AsharpV4_Contex_t*)context;

    if(CHECK_ISP_HW_V30()) {
        Asharp4_result_t ret_v4 = ASHARP4_RET_SUCCESS;
        ret_v4 = Asharp_Release_V4(asharp_group_contex->asharp_contex_v4);
        if(ret_v4 != ASHARP4_RET_SUCCESS) {
            ret = XCAM_RETURN_ERROR_FAILED;
            LOGE_ASHARP("%s: Initializaion ASHARP failed (%d)\n", __FUNCTION__, ret);
        }
    }
    else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE_ASHARP("module_hw_version of asharp  is isvalid!!!!");
    }

    if(ret != XCAM_RETURN_NO_ERROR) {
        LOGE_ASHARP("%s: release asharp group failed (%d)\n", __FUNCTION__, ret);
    } else {
        free(asharp_group_contex);
    }


    LOGI_ASHARP("%s exit ret:%d\n", __FUNCTION__, ret);
    return ret;
}

static XCamReturn groupAsharpV4Prepare(RkAiqAlgoCom* params)
{
    LOGI_ASHARP("%s enter \n", __FUNCTION__ );

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    CamGroup_AsharpV4_Contex_t * asharp_group_contex = (CamGroup_AsharpV4_Contex_t *)params->ctx;
    RkAiqAlgoCamGroupPrepare* para = (RkAiqAlgoCamGroupPrepare*)params;

    if(CHECK_ISP_HW_V30()) {
        Asharp_Context_V4_t * asharp_contex_v4 = asharp_group_contex->asharp_contex_v4;
        if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )) {
            // todo  update calib pars for surround view
#if ASHARP_USE_JSON_FILE_V4
            void *pCalibdbV2 = (void*)(para->s_calibv2);
            CalibDbV2_SharpV4_t *sharp_v4 = (CalibDbV2_SharpV4_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCalibdbV2, sharp_v4));
            asharp_contex_v4->sharp_v4 = *sharp_v4;
            asharp_contex_v4->isIQParaUpdate = true;
            asharp_contex_v4->isReCalculate |= 1;
#endif
        }
        Asharp_Config_V4_t stAsharpConfigV4;
        stAsharpConfigV4.rawHeight =  params->u.prepare.sns_op_height;
        stAsharpConfigV4.rawWidth = params->u.prepare.sns_op_width;
        Asharp4_result_t ret_v4 = ASHARP4_RET_SUCCESS;
        ret_v4 = Asharp_Prepare_V4(asharp_contex_v4, &stAsharpConfigV4);
        if(ret_v4 != ASHARP4_RET_SUCCESS) {
            ret = XCAM_RETURN_ERROR_FAILED;
            LOGE_ASHARP("%s: config ASHARP failed (%d)\n", __FUNCTION__, ret);
        }
    }
    else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE_ASHARP("module_hw_version of asharp is isvalid!!!!");
    }

    LOGI_ASHARP("%s exit ret:%d\n", __FUNCTION__, ret);
    return ret;
}

static XCamReturn groupAsharpV4Processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    LOGI_ASHARP("%s enter \n", __FUNCTION__ );
    LOGI_ASHARP("----------------------------------------------frame_id (%d)----------------------------------------------\n", inparams->frame_id);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoCamGroupProcIn* procParaGroup = (RkAiqAlgoCamGroupProcIn*)inparams;
    RkAiqAlgoCamGroupProcOut* procResParaGroup = (RkAiqAlgoCamGroupProcOut*)outparams;
    CamGroup_AsharpV4_Contex_t * asharp_group_contex = (CamGroup_AsharpV4_Contex_t *)inparams->ctx;
    int deltaIso = 0;

    //method error
    if (asharp_group_contex->group_CalibV2.groupMethod <= CalibDbV2_CAMGROUP_ASHARPV4_METHOD_MIN
            ||  asharp_group_contex->group_CalibV2.groupMethod >=  CalibDbV2_CAMGROUP_ASHARPV4_METHOD_MAX) {
        return (ret);
    }

    //group empty
    if(procParaGroup->camgroupParmasArray == nullptr) {
        LOGE_ASHARP("camgroupParmasArray is null");
        return(XCAM_RETURN_ERROR_FAILED);
    }

    //get cur ae exposure
    Asharp4_ExpInfo_t stExpInfoV4;
    memset(&stExpInfoV4, 0x00, sizeof(Asharp3_ExpInfo_t));
    stExpInfoV4.hdr_mode = 0; //pAnrProcParams->hdr_mode;
    stExpInfoV4.snr_mode = 0;
    for(int i = 0; i < 3; i++) {
        stExpInfoV4.arIso[i] = 50;
        stExpInfoV4.arAGain[i] = 1.0;
        stExpInfoV4.arDGain[i] = 1.0;
        stExpInfoV4.arTime[i] = 0.01;
    }


    //merge ae result, iso mean value
    rk_aiq_singlecam_3a_result_t* scam_3a_res = procParaGroup->camgroupParmasArray[0];
    if(scam_3a_res->aec._bEffAecExpValid) {
        RKAiqAecExpInfo_t* pCurExp = &scam_3a_res->aec._effAecExpInfo;
        stExpInfoV4.snr_mode = pCurExp->CISFeature.SNR;
        if((rk_aiq_working_mode_t)procParaGroup->working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
            stExpInfoV4.hdr_mode = 0;
            stExpInfoV4.arAGain[0] = pCurExp->LinearExp.exp_real_params.analog_gain;
            stExpInfoV4.arDGain[0] = pCurExp->LinearExp.exp_real_params.digital_gain;
            stExpInfoV4.arTime[0] = pCurExp->LinearExp.exp_real_params.integration_time;
            stExpInfoV4.arIso[0] = stExpInfoV4.arAGain[0] * stExpInfoV4.arDGain[0] * 50;

        } else {
            if(procParaGroup->working_mode == RK_AIQ_ISP_HDR_MODE_2_FRAME_HDR
                    || procParaGroup->working_mode == RK_AIQ_ISP_HDR_MODE_2_LINE_HDR)
                stExpInfoV4.hdr_mode = 1;
            else if (procParaGroup->working_mode == RK_AIQ_ISP_HDR_MODE_3_FRAME_HDR
                     || procParaGroup->working_mode == RK_AIQ_ISP_HDR_MODE_3_LINE_HDR)
                stExpInfoV4.hdr_mode = 2;
            else {
                stExpInfoV4.hdr_mode = 0;
                LOGE_ANR("mode error\n");
            }

            for(int i = 0; i < 3; i++) {
                stExpInfoV4.arAGain[i] = pCurExp->HdrExp[i].exp_real_params.analog_gain;
                stExpInfoV4.arDGain[i] = pCurExp->HdrExp[i].exp_real_params.digital_gain;
                stExpInfoV4.arTime[i] = pCurExp->HdrExp[i].exp_real_params.integration_time;
                stExpInfoV4.arIso[i] = stExpInfoV4.arAGain[i] * stExpInfoV4.arDGain[i] * 50;
            }

        }
    } else {
        LOGW("fail to get sensor gain form AE module,use default value ");
    }



    if(CHECK_ISP_HW_V30()) {
        Asharp_Context_V4_t * asharp_contex_v4 = asharp_group_contex->asharp_contex_v4;
        Asharp_ProcResult_V4_t stAsharpResultV4;
        RK_SHARP_Fix_V4_t stFix;
        stAsharpResultV4.stFix = &stFix;

        deltaIso = abs(stExpInfoV4.arIso[stExpInfoV4.hdr_mode] - asharp_contex_v4->stExpInfo.arIso[stExpInfoV4.hdr_mode]);
        if(deltaIso > ASHARPV4_RECALCULATE_DELTA_ISO) {
            asharp_contex_v4->isReCalculate |= 1;
        }
        if(asharp_contex_v4->isReCalculate) {
            Asharp4_result_t ret_v4 = ASHARP4_RET_SUCCESS;
            ret_v4 = Asharp_Process_V4(asharp_contex_v4, &stExpInfoV4);
            if(ret_v4 != ASHARP4_RET_SUCCESS) {
                ret = XCAM_RETURN_ERROR_FAILED;
                LOGE_ASHARP("%s: processing ASHARP failed (%d)\n", __FUNCTION__, ret);
            }
            outparams->cfg_update = true;
            LOGD_ASHARP("recalculate: %d delta_iso:%d \n ", asharp_contex_v4->isReCalculate, deltaIso);
        } else {
            outparams->cfg_update = false;
        }
        Asharp_GetProcResult_V4(asharp_contex_v4, &stAsharpResultV4);
        for (int i = 0; i < procResParaGroup->arraySize; i++) {
            *(procResParaGroup->camgroupParmasArray[i]->asharp._asharp_procRes_v4) = *stAsharpResultV4.stFix;
            IS_UPDATE_MEM((procResParaGroup->camgroupParmasArray[i]->asharp._asharp_procRes_v4), procParaGroup->_offset_is_update) =
                outparams->cfg_update;
        }
        asharp_contex_v4->isReCalculate = 0;

    }
    else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE_ASHARP("module_hw_version of asharp is isvalid!!!!");
    }

    LOGI_ASHARP("%s exit\n", __FUNCTION__);
    return ret;
}

RkAiqAlgoDescription g_RkIspAlgoDescCamgroupAsharpV4 = {
    .common = {
        .version = RKISP_ALGO_CAMGROUP_ASHARPV4_VERSION,
        .vendor  = RKISP_ALGO_CAMGROUP_ASHARPV4_VENDOR,
        .description = RKISP_ALGO_CAMGROUP_ASHARPV4_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_ASHARP,
        .id      = 0,
        .create_context  = groupAsharpV4CreateCtx,
        .destroy_context = groupAsharpV4DestroyCtx,
    },
    .prepare = groupAsharpV4Prepare,
    .pre_process = NULL,
    .processing = groupAsharpV4Processing,
    .post_process = NULL,
};

RKAIQ_END_DECLARE
