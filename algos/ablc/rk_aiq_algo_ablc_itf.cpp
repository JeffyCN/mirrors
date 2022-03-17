/*
 * rk_aiq_algo_ablc_itf.c
 *
 *  Copyright (c) 2019 Rockchip Corporation
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

#include "rk_aiq_algo_types_int.h"
#include "ablc/rk_aiq_algo_ablc_itf.h"
#include "ablc/rk_aiq_ablc_algo.h"

RKAIQ_BEGIN_DECLARE

typedef struct _RkAiqAlgoContext {
    void* place_holder[0];
} RkAiqAlgoContext;


static XCamReturn
create_context(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{   XCamReturn result = XCAM_RETURN_NO_ERROR;

    LOG1_ABLC("%s: (enter)\n", __FUNCTION__ );
    AlgoCtxInstanceCfgInt *cfgInt = (AlgoCtxInstanceCfgInt*)cfg;

    AblcContext_t* pAblcCtx = NULL;
    AblcResult_t ret = AblcInit(&pAblcCtx, cfgInt->calibv2);
    if(ret != ABLC_RET_SUCCESS) {
        result = XCAM_RETURN_ERROR_FAILED;
        LOGE_ABLC("%s: Initializaion Ablc failed (%d)\n", __FUNCTION__, ret);
    } else {
        *context = (RkAiqAlgoContext *)(pAblcCtx);
    }


    LOG1_ABLC("%s: (exit)\n", __FUNCTION__ );
    return result;

}

static XCamReturn
destroy_context(RkAiqAlgoContext *context)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    LOG1_ABLC("%s: (enter)\n", __FUNCTION__ );

#if 1
    AblcContext_t* pAblcCtx = (AblcContext_t*)context;
    AblcResult_t ret = AblcRelease(pAblcCtx);
    if(ret != ABLC_RET_SUCCESS) {
        result = XCAM_RETURN_ERROR_FAILED;
        LOGE_ABLC("%s: release Ablc failed (%d)\n", __FUNCTION__, ret);
    }
#endif

    LOG1_ABLC("%s: (exit)\n", __FUNCTION__ );
    return result;

}

static XCamReturn
prepare(RkAiqAlgoCom* params)
{
    LOG1_ABLC("%s: (enter)\n", __FUNCTION__ );
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    AblcContext_t* pAblcCtx = (AblcContext_t *)params->ctx;
    RkAiqAlgoConfigAblcInt* pCfgParam = (RkAiqAlgoConfigAblcInt*)params;
    pAblcCtx->prepare_type = params->u.prepare.conf_type;

    if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )) {
        CalibDbV2_Ablc_t* calibv2_ablc_calib =
            (CalibDbV2_Ablc_t*)(CALIBDBV2_GET_MODULE_PTR((void*)(pCfgParam->rk_com.u.prepare.calibv2), ablc_calib));

        LOGD_ABLC("%s: Ablc Reload Para!\n", __FUNCTION__);
        memcpy(&pAblcCtx->stBlcCalib, calibv2_ablc_calib, sizeof(CalibDbV2_Ablc_t));
        pAblcCtx->isUpdateParam = true;
        pAblcCtx->isReCalculate |= 1;
    }

    LOG1_ABLC("%s: (exit)\n", __FUNCTION__ );
    return result;
}

static XCamReturn
pre_process(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    LOG1_ABLC("%s: (enter)\n", __FUNCTION__ );
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    AblcContext_t* pAblcCtx = (AblcContext_t *)inparams->ctx;

    if(pAblcCtx->isUpdateParam) {
        if(pAblcCtx->attr.eMode == ABLC_OP_MODE_API_TOOL) {
            BlcNewMalloc(&pAblcCtx->config, &pAblcCtx->attr.stTool);
            AblcResult_t ret = AblcConfig(&pAblcCtx->config, &pAblcCtx->attr.stTool);
            if(ret != ABLC_RET_SUCCESS) {
                result = XCAM_RETURN_ERROR_FAILED;
                LOGE_ABLC("%s: config Ablc failed (%d)\n", __FUNCTION__, ret);
            }
        }
        else {
            BlcNewMalloc(&pAblcCtx->config, &pAblcCtx->stBlcCalib);
            AblcResult_t ret = AblcConfig(&pAblcCtx->config, &pAblcCtx->stBlcCalib);
            if(ret != ABLC_RET_SUCCESS) {
                result = XCAM_RETURN_ERROR_FAILED;
                LOGE_ABLC("%s: config Ablc failed (%d)\n", __FUNCTION__, ret);
            }
        }
        pAblcCtx->isUpdateParam = false;
    }

    LOG1_ABLC("%s: (exit)\n", __FUNCTION__ );
    return result;
}

static XCamReturn
processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    int iso;
    int delta_iso = 0;
    LOG1_ABLC("%s: (enter)\n", __FUNCTION__ );


    RkAiqAlgoProcAblcInt* pAblcProcParams = (RkAiqAlgoProcAblcInt*)inparams;
    RkAiqAlgoProcResAblcInt* pAblcProcResParams = (RkAiqAlgoProcResAblcInt*)outparams;
    AblcContext_t* pAblcCtx = (AblcContext_t *)inparams->ctx;
    AblcExpInfo_t stExpInfo;
    memset(&stExpInfo, 0x00, sizeof(AblcExpInfo_t));

    stExpInfo.hdr_mode = 0; //pAnrProcParams->hdr_mode;
    for(int i = 0; i < 3; i++) {
        stExpInfo.arIso[i] = 50;
        stExpInfo.arAGain[i] = 1.0;
        stExpInfo.arDGain[i] = 1.0;
        stExpInfo.arTime[i] = 0.01;
    }

    if(pAblcProcParams->hdr_mode == RK_AIQ_WORKING_MODE_NORMAL) {
        stExpInfo.hdr_mode = 0;
    } else if(pAblcProcParams->hdr_mode == RK_AIQ_ISP_HDR_MODE_2_FRAME_HDR
              || pAblcProcParams->hdr_mode == RK_AIQ_ISP_HDR_MODE_2_LINE_HDR ) {
        stExpInfo.hdr_mode = 1;
    } else if(pAblcProcParams->hdr_mode == RK_AIQ_ISP_HDR_MODE_3_FRAME_HDR
              || pAblcProcParams->hdr_mode == RK_AIQ_ISP_HDR_MODE_3_LINE_HDR ) {
        stExpInfo.hdr_mode = 2;
    }

    RKAiqAecExpInfo_t *curExp = pAblcProcParams->rk_com.u.proc.curExp;
    if(curExp != NULL) {
        if(pAblcProcParams->hdr_mode == RK_AIQ_WORKING_MODE_NORMAL) {
            stExpInfo.arAGain[0] = curExp->LinearExp.exp_real_params.analog_gain;
            stExpInfo.arDGain[0] = curExp->LinearExp.exp_real_params.digital_gain;
            stExpInfo.arTime[0] = curExp->LinearExp.exp_real_params.integration_time;
            stExpInfo.arIso[0] = stExpInfo.arAGain[0] * stExpInfo.arDGain[0] * 50;
        } else {
            for(int i = 0; i < 3; i++) {
                stExpInfo.arAGain[i] = curExp->HdrExp[i].exp_real_params.analog_gain;
                stExpInfo.arDGain[i] = curExp->HdrExp[i].exp_real_params.digital_gain;
                stExpInfo.arTime[i] = curExp->HdrExp[i].exp_real_params.integration_time;
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
        LOGE_ABLC("%s:%d curExp is NULL, so use default instead \n", __FUNCTION__, __LINE__);
    }

    delta_iso = abs(stExpInfo.arIso[stExpInfo.hdr_mode] - pAblcCtx->stExpInfo.arIso[pAblcCtx->stExpInfo.hdr_mode]);
    if(delta_iso > ABLC_RECALCULATE_DELTE_ISO) {
        pAblcCtx->isReCalculate |= 1;
    }

    if(pAblcCtx->isReCalculate ) {
        AblcResult_t ret = AblcProcess(pAblcCtx, &stExpInfo);
        if(ret != ABLC_RET_SUCCESS) {
            result = XCAM_RETURN_ERROR_FAILED;
            LOGE_ABLC("%s: processing ABLC failed (%d)\n", __FUNCTION__, ret);
        }
        pAblcCtx->ProcRes.isNeedUpdate = true;
        LOGD_ABLC("%s:%d processing ABLC recalculate delta_iso:%d \n", __FUNCTION__, __LINE__, delta_iso);
    } else {
        pAblcCtx->ProcRes.isNeedUpdate = false;
    }
    memcpy(&pAblcProcResParams->ablc_proc_res, &pAblcCtx->ProcRes, sizeof(AblcProc_t));

    pAblcCtx->isReCalculate = 0;

    LOG1_ABLC("%s: (exit)\n", __FUNCTION__ );
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
post_process(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    return XCAM_RETURN_NO_ERROR;
}

RkAiqAlgoDescription g_RkIspAlgoDescAblc = {
    .common = {
        .version = RKISP_ALGO_ABLC_VERSION,
        .vendor  = RKISP_ALGO_ABLC_VENDOR,
        .description = RKISP_ALGO_ABLC_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_ABLC,
        .id      = 0,
        .create_context  = create_context,
        .destroy_context = destroy_context,
    },
    .prepare = prepare,
    .pre_process = pre_process,
    .processing = processing,
    .post_process = post_process,
};

RKAIQ_END_DECLARE
