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

#include "rk_aiq_algo_camgroup_abayer2dnr_itf_v23.h"
#include "rk_aiq_algo_camgroup_types.h"
#include "rk_aiq_types_camgroup_abayer2dnr_prvt_v23.h"
#include "abayer2dnrV23/rk_aiq_abayer2dnr_algo_itf_v23.h"
#include "abayer2dnrV23/rk_aiq_abayer2dnr_algo_v23.h"



RKAIQ_BEGIN_DECLARE

#if RKAIQ_HAVE_BAYER2DNR_V23
static XCamReturn groupAbayer2dnrV23CreateCtx(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{
    LOGI_ANR("%s enter \n", __FUNCTION__ );

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    CamGroup_Abayer2dnrV23_Contex_t *abayernr_group_contex = NULL;
    AlgoCtxInstanceCfgCamGroup *cfgInt = (AlgoCtxInstanceCfgCamGroup*)cfg;


    if(CHECK_ISP_HW_V39() || CHECK_ISP_HW_V32() || CHECK_ISP_HW_V32_LITE()) {
        abayernr_group_contex = (CamGroup_Abayer2dnrV23_Contex_t*)malloc(sizeof(CamGroup_Abayer2dnrV23_Contex_t));
#if ABAYER2DNR_USE_JSON_FILE_V23
        Abayer2dnr_result_V23_t ret_v23 = ABAYER2DNR_V23_RET_SUCCESS;
        ret_v23 = Abayer2dnr_Init_V23(&(abayernr_group_contex->abayer2dnr_contex_v23), (void *)cfgInt->s_calibv2);
        if(ret_v23 != ABAYER2DNR_V23_RET_SUCCESS) {
            ret = XCAM_RETURN_ERROR_FAILED;
            LOGE_ANR("%s: Initializaion ANR failed (%d)\n", __FUNCTION__, ret);
        }
#endif
    }
    else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE_ANR("module_hw_version of abayernr is invalid!!!!");
    }

    if(ret != XCAM_RETURN_NO_ERROR) {
        LOGE_ANR("%s: Initializaion group bayernr failed (%d)\n", __FUNCTION__, ret);
    } else {
        // to do got abayernrSurrViewClib and initinal paras for for surround view
        abayernr_group_contex->group_CalibV23.groupMethod = CalibDbV2_CAMGROUP_ABAYER2DNRV23_METHOD_MEAN;// to do from json
        abayernr_group_contex->camera_Num = cfgInt->camIdArrayLen;

        *context = (RkAiqAlgoContext *)(abayernr_group_contex);

        LOGI_ANR("%s:%d surrViewMethod(1-mean):%d, cameraNum %d \n",
                 __FUNCTION__, __LINE__,
                 abayernr_group_contex->group_CalibV23.groupMethod,
                 abayernr_group_contex->camera_Num);
    }

    LOGI_ANR("%s exit ret:%d\n", __FUNCTION__, ret);
    return ret;

}

static XCamReturn groupAbayer2dnrV23DestroyCtx(RkAiqAlgoContext *context)
{
    LOGI_ANR("%s enter \n", __FUNCTION__ );

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    CamGroup_Abayer2dnrV23_Contex_t *abayernr_group_contex = (CamGroup_Abayer2dnrV23_Contex_t*)context;

    if(CHECK_ISP_HW_V39() || CHECK_ISP_HW_V32() || CHECK_ISP_HW_V32_LITE()) {
        Abayer2dnr_result_V23_t ret_v23 = ABAYER2DNR_V23_RET_SUCCESS;
        ret_v23 = Abayer2dnr_Release_V23(abayernr_group_contex->abayer2dnr_contex_v23);
        if(ret_v23 != ABAYER2DNR_V23_RET_SUCCESS) {
            ret = XCAM_RETURN_ERROR_FAILED;
            LOGE_ANR("%s: Initializaion ANR failed (%d)\n", __FUNCTION__, ret);
        }
    }
    else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE_ANR("module_hw_version of abayer2dnr is isvalid!!!!");
    }

    if(ret != XCAM_RETURN_NO_ERROR) {
        LOGE_ANR("%s: release ANR failed (%d)\n", __FUNCTION__, ret);
    } else {
        free(abayernr_group_contex);
    }


    LOGI_ANR("%s exit ret:%d\n", __FUNCTION__, ret);
    return ret;
}

static XCamReturn groupAbayer2dnrV23Prepare(RkAiqAlgoCom* params)
{
    LOGI_ANR("%s enter \n", __FUNCTION__ );

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    CamGroup_Abayer2dnrV23_Contex_t * abayernr_group_contex = (CamGroup_Abayer2dnrV23_Contex_t *)params->ctx;
    RkAiqAlgoCamGroupPrepare* para = (RkAiqAlgoCamGroupPrepare*)params;

    if(CHECK_ISP_HW_V39() || CHECK_ISP_HW_V32() || CHECK_ISP_HW_V32_LITE()) {
        Abayer2dnr_Context_V23_t * abayer2dnr_contex_v23 = abayernr_group_contex->abayer2dnr_contex_v23;
        abayer2dnr_contex_v23->prepare_type = params->u.prepare.conf_type;
        if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )) {
            // todo  update calib pars for surround view
#if ABAYER2DNR_USE_JSON_FILE_V23
            void *pCalibdbV2 = (void*)(para->s_calibv2);
            CalibDbV2_Bayer2dnrV23_t *bayer2dnr_v23 = (CalibDbV2_Bayer2dnrV23_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCalibdbV2, bayer2dnr_v23));
            abayer2dnr_contex_v23->bayernr_v23 = *bayer2dnr_v23;
            abayer2dnr_contex_v23->isIQParaUpdate = true;
            abayer2dnr_contex_v23->isReCalculate |= 1;
#endif
        }
        Abayer2dnr_Config_V23_t stAbayer2dnrConfigV23;
        Abayer2dnr_result_V23_t ret_v23 = ABAYER2DNR_V23_RET_SUCCESS;
        ret_v23 = Abayer2dnr_Prepare_V23(abayer2dnr_contex_v23, &stAbayer2dnrConfigV23);
        if(ret_v23 != ABAYER2DNR_V23_RET_SUCCESS) {
            ret = XCAM_RETURN_ERROR_FAILED;
            LOGE_ANR("%s: config ANR failed (%d)\n", __FUNCTION__, ret);
        }
    }
    else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE_ANR("module_hw_version of abayer2dnr is isvalid!!!!");
    }

    LOGI_ANR("%s exit ret:%d\n", __FUNCTION__, ret);
    return ret;
}

static XCamReturn groupAbayer2dnrV23Processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    LOGI_ANR("%s enter \n", __FUNCTION__ );
    LOGI_ANR("----------------------------------------------frame_id (%d)----------------------------------------------\n", inparams->frame_id);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoCamGroupProcIn* procParaGroup = (RkAiqAlgoCamGroupProcIn*)inparams;
    RkAiqAlgoCamGroupProcOut* procResParaGroup = (RkAiqAlgoCamGroupProcOut*)outparams;
    CamGroup_Abayer2dnrV23_Contex_t * abayernr_group_contex = (CamGroup_Abayer2dnrV23_Contex_t *)inparams->ctx;
    int deltaIso = 0;

    //method error
    if (abayernr_group_contex->group_CalibV23.groupMethod <= CalibDbV2_CAMGROUP_ABAYER2DNRV23_METHOD_MIN
            ||  abayernr_group_contex->group_CalibV23.groupMethod >=  CalibDbV2_CAMGROUP_ABAYER2DNRV23_METHOD_MAX) {
        return (ret);
    }

    //group empty
    if(procParaGroup == nullptr || procParaGroup->camgroupParmasArray == nullptr) {
        LOGE_ANR("procParaGroup or camgroupParmasArray is null");
        return(XCAM_RETURN_ERROR_FAILED);
    }

    //get cur ae exposure
    Abayer2dnr_ExpInfo_V23_t stExpInfoV23;
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
            float tmp_predgain = stExpInfoV23.blc_ob_predgain;
            if(tmp_predgain < 1.0)
                tmp_predgain = 1.0;
            stExpInfoV23.arIso[0] = stExpInfoV23.arAGain[0] * stExpInfoV23.arDGain[0] * tmp_predgain * 50;

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



    if(CHECK_ISP_HW_V39() || CHECK_ISP_HW_V32() || CHECK_ISP_HW_V32_LITE()) {
        Abayer2dnr_Context_V23_t * abayer2dnr_contex_v23 = abayernr_group_contex->abayer2dnr_contex_v23;
        Abayer2dnr_ProcResult_V23_t stAbayer2dnrResultV23;
        RK_Bayer2dnr_Fix_V23_t st2DFix;
        stAbayer2dnrResultV23.st2DFix = &st2DFix;

        if(stExpInfoV23.blc_ob_predgain != abayer2dnr_contex_v23->stExpInfo.blc_ob_predgain) {
            abayer2dnr_contex_v23->isReCalculate |= 1;
        }
        deltaIso = abs(stExpInfoV23.arIso[stExpInfoV23.hdr_mode] - abayer2dnr_contex_v23->stExpInfo.arIso[stExpInfoV23.hdr_mode]);
        if(deltaIso > ABAYER2DNRV23_RECALCULATE_DELTA_ISO) {
            abayer2dnr_contex_v23->isReCalculate |= 1;
        }
        if(abayer2dnr_contex_v23->isReCalculate) {
            Abayer2dnr_result_V23_t ret_v23 = ABAYER2DNR_V23_RET_SUCCESS;
            ret_v23 = Abayer2dnr_Process_V23(abayer2dnr_contex_v23, &stExpInfoV23);
            if(ret_v23 != ABAYER2DNR_V23_RET_SUCCESS) {
                ret = XCAM_RETURN_ERROR_FAILED;
                LOGE_ANR("%s: processing ANR failed (%d)\n", __FUNCTION__, ret);
            }
            outparams->cfg_update = true;
            LOGD_ANR("recalculate: %d delta_iso:%d \n ", abayer2dnr_contex_v23->isReCalculate, deltaIso);
        } else {
            outparams->cfg_update = false;
        }
        Abayer2dnr_GetProcResult_V23(abayer2dnr_contex_v23, &stAbayer2dnrResultV23);
        for (int i = 0; i < procResParaGroup->arraySize; i++) {
            *(procResParaGroup->camgroupParmasArray[i]->abayernr._abayer2dnr_procRes_v23) = *stAbayer2dnrResultV23.st2DFix;
            IS_UPDATE_MEM((procResParaGroup->camgroupParmasArray[i]->abayernr._abayer2dnr_procRes_v23), procParaGroup->_offset_is_update) =
                outparams->cfg_update;
        }
        abayer2dnr_contex_v23->isReCalculate = 0;
    }
    else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE_ANR("module_hw_version of abayer2dnr is isvalid!!!!");
    }

    LOGI_ANR("%s exit\n", __FUNCTION__);
    return ret;
}

RkAiqAlgoDescription g_RkIspAlgoDescCamgroupAbayer2dnrV23 = {
    .common = {
        .version = RKISP_ALGO_CAMGROUP_ABAYER2DNRV23_VERSION,
        .vendor  = RKISP_ALGO_CAMGROUP_ABAYER2DNRV23_VENDOR,
        .description = RKISP_ALGO_CAMGROUP_ABAYER2DNRV23_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_ARAWNR,
        .id      = 0,
        .create_context  = groupAbayer2dnrV23CreateCtx,
        .destroy_context = groupAbayer2dnrV23DestroyCtx,
    },
    .prepare = groupAbayer2dnrV23Prepare,
    .pre_process = NULL,
    .processing = groupAbayer2dnrV23Processing,
    .post_process = NULL,
};
#endif

RKAIQ_END_DECLARE
