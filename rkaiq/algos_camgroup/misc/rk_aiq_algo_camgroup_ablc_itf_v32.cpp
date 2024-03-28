/*
 * rk_aiq_algo_camgroup_ablc_itf.c
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

#include "rk_aiq_algo_camgroup_types.h"
#include "misc/rk_aiq_algo_camgroup_misc_itf.h"
#include "ablcV32/rk_aiq_ablc_algo_itf_v32.h"
#include "ablcV32/rk_aiq_ablc_algo_v32.h"
#include "ablcV32/rk_aiq_types_ablc_algo_prvt_v32.h"


RKAIQ_BEGIN_DECLARE


static XCamReturn
create_context(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    LOGI_ABLC("%s: (enter)\n", __FUNCTION__ );
    AlgoCtxInstanceCfgCamGroup *cfgInt = (AlgoCtxInstanceCfgCamGroup*)cfg;

    AblcContext_V32_t* pAblcCtx = NULL;
    AblcResult_V32_t ret = AblcV32Init(&pAblcCtx, (CamCalibDbV2Context_t*)(cfgInt->s_calibv2));//load json paras
    if(ret != ABLC_V32_RET_SUCCESS) {
        result = XCAM_RETURN_ERROR_FAILED;
        LOGE_ABLC("%s: Initializaion Ablc failed (%d)\n", __FUNCTION__, ret);
    } else {
        *context = (RkAiqAlgoContext *)(pAblcCtx);
    }

    LOGI_ABLC("%s: (exit)\n", __FUNCTION__ );
    return result;
}

static XCamReturn
destroy_context(RkAiqAlgoContext *context)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    LOGI_ABLC("%s: (enter)\n", __FUNCTION__ );

#if 1
    AblcContext_V32_t* pAblcCtx = (AblcContext_V32_t*)context;
    AblcResult_V32_t ret = AblcV32Release(pAblcCtx);
    if(ret != ABLC_V32_RET_SUCCESS) {
        result = XCAM_RETURN_ERROR_FAILED;
        LOGE_ABLC("%s: release Ablc failed (%d)\n", __FUNCTION__, ret);
    }
#endif

    LOGI_ABLC("%s: (exit)\n", __FUNCTION__ );
    return result;
}

static XCamReturn
prepare(RkAiqAlgoCom* params)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    LOGD_ABLC("%s: (enter)\n", __FUNCTION__ );

    AblcContext_V32_t* pAblcCtx = (AblcContext_V32_t *)params->ctx;
    RkAiqAlgoCamGroupPrepare* pCfgParam = (RkAiqAlgoCamGroupPrepare*)params;
    pAblcCtx->prepare_type = params->u.prepare.conf_type;

    if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )) {
        CalibDbV2_Blc_V32_t* calibv2_ablc_calib =
            (CalibDbV2_Blc_V32_t*)(CALIBDBV2_GET_MODULE_PTR((void*)(pCfgParam->s_calibv2), ablcV32_calib));
        memcpy(&pAblcCtx->stBlcCalib, calibv2_ablc_calib, sizeof(CalibDbV2_Blc_V32_t));
        pAblcCtx->isUpdateParam = true;
        pAblcCtx->isReCalculate |= 1;
    }

    LOGI_ABLC("%s: (exit)\n", __FUNCTION__ );
    return result;
}

static XCamReturn
processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    int delta_iso = 0;
    LOGI_ABLC("%s: (enter)\n", __FUNCTION__ );

#if 1
    RkAiqAlgoCamGroupProcIn* procParaGroup = (RkAiqAlgoCamGroupProcIn*)inparams;
    RkAiqAlgoCamGroupProcOut* procResParaGroup = (RkAiqAlgoCamGroupProcOut*)outparams;
    AblcContext_V32_t* pAblcCtx = (AblcContext_V32_t *)inparams->ctx;
    AblcExpInfo_V32_t stExpInfo;
    AblcProc_V32_t   ablcV32_proc_res;

    memset(&stExpInfo, 0x00, sizeof(AblcExpInfo_t));

    LOGD_ABLC("%s:%d init:%d hdr mode:%d  \n",
              __FUNCTION__, __LINE__,
              inparams->u.proc.init,
              procParaGroup->working_mode);

    stExpInfo.hdr_mode = 0; //pAnrProcParams->hdr_mode;
    for(int i = 0; i < 3; i++) {
        stExpInfo.arIso[i] = 50;
        stExpInfo.arAGain[i] = 1.0;
        stExpInfo.arDGain[i] = 1.0;
        stExpInfo.arTime[i] = 0.01;
        stExpInfo.isp_dgain[i] = 1.0;
    }

    if(procParaGroup->working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
        stExpInfo.hdr_mode = 0;
    } else if(procParaGroup->working_mode == RK_AIQ_ISP_HDR_MODE_2_FRAME_HDR
              || procParaGroup->working_mode == RK_AIQ_ISP_HDR_MODE_2_LINE_HDR ) {
        stExpInfo.hdr_mode = 1;
    } else if(procParaGroup->working_mode == RK_AIQ_ISP_HDR_MODE_3_FRAME_HDR
              || procParaGroup->working_mode == RK_AIQ_ISP_HDR_MODE_3_LINE_HDR ) {
        stExpInfo.hdr_mode = 2;
    }

    rk_aiq_singlecam_3a_result_t* scam_3a_res = procParaGroup->camgroupParmasArray[0];

    if(scam_3a_res->aec._bEffAecExpValid) {
        RKAiqAecExpInfo_t* pAERes = &scam_3a_res->aec._effAecExpInfo;
        if(procParaGroup->working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
            stExpInfo.arAGain[0] = pAERes->LinearExp.exp_real_params.analog_gain;
            stExpInfo.arDGain[0] = pAERes->LinearExp.exp_real_params.digital_gain;
            stExpInfo.arTime[0] = pAERes->LinearExp.exp_real_params.integration_time;
            stExpInfo.isp_dgain[0] = pAERes->LinearExp.exp_real_params.isp_dgain;
            stExpInfo.arIso[0] = stExpInfo.arAGain[0] * stExpInfo.arDGain[0] * 50;
        } else {
            for(int i = 0; i < 3; i++) {
                stExpInfo.arAGain[i] = pAERes->HdrExp[i].exp_real_params.analog_gain;
                stExpInfo.arDGain[i] = pAERes->HdrExp[i].exp_real_params.digital_gain;
                stExpInfo.arTime[i] = pAERes->HdrExp[i].exp_real_params.integration_time;
                stExpInfo.isp_dgain[i] = pAERes->HdrExp[i].exp_real_params.isp_dgain;
                stExpInfo.arIso[i] = stExpInfo.arAGain[i] * stExpInfo.arDGain[i] * 50;

                LOGD_ABLC("%s:%d index:%d again:%f dgain:%f time:%f iso:%d hdr_mode:%d\n",
                          __FUNCTION__, __LINE__,
                          i,
                          stExpInfo.arAGain[i],
                          stExpInfo.arDGain[i],
                          stExpInfo.arTime[i],
                          stExpInfo.arIso[i],
                          stExpInfo.hdr_mode);
            }
        }
    } else {
        LOGE_ABLC("%s:%d pAERes is NULL, so use default instead \n", __FUNCTION__, __LINE__);
    }

    delta_iso = abs(stExpInfo.arIso[stExpInfo.hdr_mode] - pAblcCtx->stExpInfo.arIso[pAblcCtx->stExpInfo.hdr_mode]);
    if(delta_iso > ABLC_V32_RECALCULATE_DELTE_ISO) {
        pAblcCtx->isReCalculate |= 1;
    }


    if(pAblcCtx->isReCalculate ) {
        AblcResult_V32_t ret = AblcV32Process(pAblcCtx, &stExpInfo);
        if(ret != ABLC_V32_RET_SUCCESS) {
            result = XCAM_RETURN_ERROR_FAILED;
            LOGE_ABLC("%s: processing Ablc failed (%d)\n", __FUNCTION__, ret);
        }
        LOGD_ABLC("%s:%d processing ABLC recalculate delta_iso:%d \n", __FUNCTION__, __LINE__, delta_iso);
    }

    Ablc_GetProcResult_V32(pAblcCtx, &ablcV32_proc_res);

    for (int i = 0; i < procResParaGroup->arraySize; i++) {
        if (pAblcCtx->isReCalculate)
            *(procResParaGroup->camgroupParmasArray[i]->ablc._blcConfig_v32) = ablcV32_proc_res;
        //TODO
        IS_UPDATE_MEM((procResParaGroup->camgroupParmasArray[i]->ablc._blcConfig_v32), procParaGroup->_offset_is_update) =
            pAblcCtx->isReCalculate;
    }

    pAblcCtx->isReCalculate = 0;
#if 0 //TODO: Is this necessary for group algo ?
    //sensor blc setting
    pAblcProcResParams->ablc_proc_res_com.SenDpccRes.enable = pAblcCtx->SenDpccRes.enable;
    pAblcProcResParams->ablc_proc_res_com.SenDpccRes.total_blc = pAblcCtx->SenDpccRes.total_blc;
    pAblcProcResParams->ablc_proc_res_com.SenDpccRes.cur_single_blc = pAblcCtx->SenDpccRes.cur_single_blc;
    pAblcProcResParams->ablc_proc_res_com.SenDpccRes.cur_multiple_blc = pAblcCtx->SenDpccRes.cur_multiple_blc;
#endif

#endif

    LOGI_ABLC("%s: (exit)\n", __FUNCTION__ );
    return XCAM_RETURN_NO_ERROR;
}

RkAiqAlgoDescription g_RkIspAlgoDescCamgroupAblcV32 = {
    .common = {
        .version = RKISP_ALGO_ABLC_V32_VERSION,
        .vendor  = RKISP_ALGO_ABLC_V32_VENDOR,
        .description = RKISP_ALGO_ABLC_V32_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_ABLC,
        .id      = 0,
        .create_context  = create_context,
        .destroy_context = destroy_context,
    },
    .prepare = prepare,
    .pre_process = NULL,
    .processing = processing,
    .post_process = NULL,
};

RKAIQ_END_DECLARE
