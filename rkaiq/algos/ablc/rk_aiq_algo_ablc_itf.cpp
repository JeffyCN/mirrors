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

#include "ablc/rk_aiq_algo_ablc_itf.h"
#include "ablc/rk_aiq_ablc_algo.h"
#include "rk_aiq_algo_types.h"

RKAIQ_BEGIN_DECLARE

typedef struct _RkAiqAlgoContext {
    AblcContext_t AblcCtx;
} RkAiqAlgoContext;


static XCamReturn
create_context(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{   XCamReturn result = XCAM_RETURN_NO_ERROR;

    LOG1_ABLC("%s: (enter)\n", __FUNCTION__ );

    AblcContext_t* pAblcCtx = NULL;
    AblcResult_t ret = AblcInit(&pAblcCtx, cfg->calibv2);
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
    RkAiqAlgoConfigAblc* pCfgParam = (RkAiqAlgoConfigAblc*)params;
    pAblcCtx->prepare_type = params->u.prepare.conf_type;

    if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )) {
        CalibDbV2_Ablc_t* calibv2_ablc_calib =
            (CalibDbV2_Ablc_t*)(CALIBDBV2_GET_MODULE_PTR((void*)(pCfgParam->com.u.prepare.calibv2), ablc_calib));

        LOGD_ABLC("%s: Ablc Reload Para!\n", __FUNCTION__);
        memcpy(&pAblcCtx->stBlcCalib, calibv2_ablc_calib, sizeof(pAblcCtx->stBlcCalib));
        // just update calib ptr
        if (params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB_PTR)
            return XCAM_RETURN_NO_ERROR;

        pAblcCtx->isUpdateParam = true;
        pAblcCtx->isReCalculate |= 1;
    }

	if(pAblcCtx->isUpdateParam) {
        AblcParamsUpdate(pAblcCtx, &pAblcCtx->stBlcCalib);
        pAblcCtx->isReCalculate |= 1;
        pAblcCtx->isUpdateParam = false;
    }

    LOG1_ABLC("%s: (exit)\n", __FUNCTION__ );
    return result;
}

static XCamReturn
pre_process(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    LOG1_ABLC("%s: (enter)\n", __FUNCTION__ );
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    LOG1_ABLC("%s: (exit)\n", __FUNCTION__ );
    return result;
}

static XCamReturn
processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    int delta_iso = 0;
    LOG1_ABLC("%s: (enter)\n", __FUNCTION__ );


    RkAiqAlgoProcAblc* pAblcProcParams = (RkAiqAlgoProcAblc*)inparams;
    RkAiqAlgoProcResAblc* pAblcProcResParams = (RkAiqAlgoProcResAblc*)outparams;
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

    RKAiqAecExpInfo_t *curExp = pAblcProcParams->com.u.proc.curExp;
    if(curExp != NULL) {
        if(pAblcProcParams->hdr_mode == RK_AIQ_WORKING_MODE_NORMAL) {
            if(curExp->LinearExp.exp_real_params.analog_gain < 1.0) {
                stExpInfo.arAGain[0] = 1.0;
                LOGW_ANR("leanr mode again is wrong, use 1.0 instead\n");
            } else {
                stExpInfo.arAGain[0] = curExp->LinearExp.exp_real_params.analog_gain;
            }
            if(curExp->LinearExp.exp_real_params.digital_gain < 1.0) {
                stExpInfo.arDGain[0] = 1.0;
                LOGW_ANR("leanr mode dgain is wrong, use 1.0 instead\n");
            } else {
                stExpInfo.arDGain[0] = curExp->LinearExp.exp_real_params.digital_gain;
            }
            if (curExp->LinearExp.exp_real_params.isp_dgain < 1.0) {
                stExpInfo.isp_dgain[0] = 1.0;
                LOGW_ANR("leanr mode isp_dgain is wrong, use 1.0 instead\n");
            } else {
                stExpInfo.isp_dgain[0] = curExp->LinearExp.exp_real_params.isp_dgain;
            }
            // stExpInfo.arDGain[0] = curExp->LinearExp.exp_real_params.digital_gain;
            stExpInfo.arTime[0] = curExp->LinearExp.exp_real_params.integration_time;
            stExpInfo.arIso[0] = stExpInfo.arAGain[0] * stExpInfo.arDGain[0] * stExpInfo.isp_dgain[0] * 50;
        } else {
            for(int i = 0; i < 3; i++) {
                if(curExp->HdrExp[i].exp_real_params.analog_gain < 1.0) {
                    stExpInfo.arAGain[i] = 1.0;
                    LOGW_ANR("hdr mode again is wrong, use 1.0 instead\n");
                } else {
                    stExpInfo.arAGain[i] = curExp->HdrExp[i].exp_real_params.analog_gain;
                }
                if(curExp->HdrExp[i].exp_real_params.digital_gain < 1.0) {
                    stExpInfo.arDGain[i] = 1.0;
                    LOGW_ANR("hdr mode dgain is wrong, use 1.0 instead\n");
                } else {
                    stExpInfo.arDGain[i] = curExp->HdrExp[i].exp_real_params.digital_gain;
                }
                if (curExp->HdrExp[i].exp_real_params.isp_dgain < 1.0) {
                    stExpInfo.isp_dgain[i] = 1.0;
                    LOGW_ANR("hdr mode isp_dgain is wrong, use 1.0 instead\n");
                } else {
                    stExpInfo.isp_dgain[i] = curExp->HdrExp[i].exp_real_params.isp_dgain;
                }
                stExpInfo.arTime[i] = curExp->HdrExp[i].exp_real_params.integration_time;
                stExpInfo.arIso[i] = stExpInfo.arAGain[i] * stExpInfo.arDGain[i] * stExpInfo.isp_dgain[i] * 50;

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
        pAblcProcResParams->res_com.cfg_update = true;
        memcpy(pAblcProcResParams->ablc_proc_res, &pAblcCtx->ProcRes, sizeof(AblcProc_t));
        LOGD_ABLC("%s:%d processing ABLC recalculate delta_iso:%d \n", __FUNCTION__, __LINE__, delta_iso);
    } else {
        pAblcProcResParams->res_com.cfg_update = false;
    }

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
