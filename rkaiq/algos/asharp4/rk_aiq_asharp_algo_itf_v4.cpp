/*
 * rk_aiq_algo_anr_itf.c
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

#include "asharp4/rk_aiq_asharp_algo_itf_v4.h"
#include "asharp4/rk_aiq_asharp_algo_v4.h"
#include "rk_aiq_algo_types.h"

RKAIQ_BEGIN_DECLARE

typedef struct _RkAiqAlgoContext {
    Asharp_Context_V4_t AsharpCtx;
} RkAiqAlgoContext;


static XCamReturn
create_context(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{

    XCamReturn result = XCAM_RETURN_NO_ERROR;
    LOGI_ASHARP("%s:oyyf (enter)\n", __FUNCTION__ );

#if 1
    Asharp_Context_V4_t* pAsharpCtx = NULL;
#if ASHARP_USE_JSON_FILE_V4
    Asharp4_result_t ret = Asharp_Init_V4(&pAsharpCtx, cfg->calibv2);
#endif

    if(ret != ASHARP4_RET_SUCCESS) {
        result = XCAM_RETURN_ERROR_FAILED;
        LOGE_ASHARP("%s: Initializaion ANR failed (%d)\n", __FUNCTION__, ret);
    } else {
        *context = (RkAiqAlgoContext *)(pAsharpCtx);
    }
#endif

    LOGI_ASHARP("%s:oyyf (exit)\n", __FUNCTION__ );
    return result;
}

static XCamReturn
destroy_context(RkAiqAlgoContext *context)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    LOGI_ASHARP("%s: oyyf (enter)\n", __FUNCTION__ );

#if 1
    Asharp_Context_V4_t* pAsharpCtx = (Asharp_Context_V4_t*)context;
    Asharp4_result_t ret = Asharp_Release_V4(pAsharpCtx);
    if(ret != ASHARP4_RET_SUCCESS) {
        result = XCAM_RETURN_ERROR_FAILED;
        LOGE_ASHARP("%s: release ANR failed (%d)\n", __FUNCTION__, ret);
    }
#endif

    LOGI_ASHARP("%s: (exit)\n", __FUNCTION__ );
    return result;
}

static XCamReturn
prepare(RkAiqAlgoCom* params)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    LOGI_ASHARP("%s: oyyf (enter)\n", __FUNCTION__ );

    Asharp_Context_V4_t* pAsharpCtx = (Asharp_Context_V4_t *)params->ctx;
    RkAiqAlgoConfigAsharpV4* pCfgParam = (RkAiqAlgoConfigAsharpV4*)params;
    pAsharpCtx->prepare_type = params->u.prepare.conf_type;

    if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )) {
#if(ASHARP_USE_JSON_FILE_V4)
        CalibDbV2_SharpV4_t *calibv2_sharp =
            (CalibDbV2_SharpV4_t *)(CALIBDBV2_GET_MODULE_PTR(pCfgParam->com.u.prepare.calibv2, sharp_v4));
        pAsharpCtx->sharp_v4 = *calibv2_sharp;
#endif
        // just update calib ptr
        if (params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB_PTR) {
            return XCAM_RETURN_NO_ERROR;
        }
        pAsharpCtx->isIQParaUpdate = true;
        pAsharpCtx->isReCalculate |= 1;
    }

    Asharp4_result_t ret = Asharp_Prepare_V4(pAsharpCtx, &pCfgParam->stAsharpConfig);
    if(ret != ASHARP4_RET_SUCCESS) {
        result = XCAM_RETURN_ERROR_FAILED;
        LOGE_ASHARP("%s: config ANR failed (%d)\n", __FUNCTION__, ret);
    }

    LOGI_ASHARP("%s: oyyf (exit)\n", __FUNCTION__ );
    return result;
}

static XCamReturn
pre_process(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    bool oldGrayMode = false;

    LOGD_ASHARP("%s: oyyf (enter)\n", __FUNCTION__ );
    Asharp_Context_V4_t* pAsharpCtx = (Asharp_Context_V4_t *)inparams->ctx;

    RkAiqAlgoPreAsharpV4* pAsharpPreParams = (RkAiqAlgoPreAsharpV4*)inparams;

    oldGrayMode = pAsharpCtx->isGrayMode;
    if (pAsharpPreParams->com.u.proc.gray_mode) {
        pAsharpCtx->isGrayMode = true;
    } else {
        pAsharpCtx->isGrayMode = false;
    }

    if(oldGrayMode != pAsharpCtx->isGrayMode) {
        pAsharpCtx->isReCalculate |= 1;
    }

    Asharp4_result_t ret = Asharp_PreProcess_V4(pAsharpCtx);
    if(ret != ASHARP4_RET_SUCCESS) {
        result = XCAM_RETURN_ERROR_FAILED;
        LOGE_ASHARP("%s: ANRPreProcess failed (%d)\n", __FUNCTION__, ret);
    }

    LOGD_ASHARP("%s: oyyf (exit)\n", __FUNCTION__ );
    return result;
}

static XCamReturn
processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    int DeltaIso = 0;

    LOGD_ASHARP("%s:oyyf (enter)\n", __FUNCTION__ );

#if 1
    RkAiqAlgoProcAsharpV4* pAsharpProcParams = (RkAiqAlgoProcAsharpV4*)inparams;
    RkAiqAlgoProcResAsharpV4* pAsharpProcResParams = (RkAiqAlgoProcResAsharpV4*)outparams;
    Asharp_Context_V4_t* pAsharpCtx = (Asharp_Context_V4_t *)inparams->ctx;
    Asharp4_ExpInfo_t stExpInfo;
    memset(&stExpInfo, 0x00, sizeof(Asharp4_ExpInfo_t));

    LOGD_ASHARP("%s:%d init:%d hdr mode:%d  \n",
                __FUNCTION__, __LINE__,
                inparams->u.proc.init,
                pAsharpProcParams->hdr_mode);

    stExpInfo.hdr_mode = 0; //pAsharpProcParams->hdr_mode;
    for(int i = 0; i < 3; i++) {
        stExpInfo.arIso[i] = 50;
        stExpInfo.arAGain[i] = 1.0;
        stExpInfo.arDGain[i] = 1.0;
        stExpInfo.arTime[i] = 0.01;
    }

    if(pAsharpProcParams->hdr_mode == RK_AIQ_WORKING_MODE_NORMAL) {
        stExpInfo.hdr_mode = 0;
    } else if(pAsharpProcParams->hdr_mode == RK_AIQ_ISP_HDR_MODE_2_FRAME_HDR
              || pAsharpProcParams->hdr_mode == RK_AIQ_ISP_HDR_MODE_2_LINE_HDR ) {
        stExpInfo.hdr_mode = 1;
    } else if(pAsharpProcParams->hdr_mode == RK_AIQ_ISP_HDR_MODE_3_FRAME_HDR
              || pAsharpProcParams->hdr_mode == RK_AIQ_ISP_HDR_MODE_3_LINE_HDR ) {
        stExpInfo.hdr_mode = 2;
    }
    stExpInfo.snr_mode = 0;

#if 0// TODO Merge:
    XCamVideoBuffer* xCamAePreRes = pAsharpProcParams->com.u.proc.res_comb->ae_pre_res;
    RkAiqAlgoPreResAe* pAEPreRes = nullptr;
    if (xCamAePreRes) {
        // xCamAePreRes->ref(xCamAePreRes);
        pAEPreRes = (RkAiqAlgoPreResAe*)xCamAePreRes->map(xCamAePreRes);
        if (!pAEPreRes) {
            LOGE_ASHARP("ae pre result is null");
        } else {

        }
        // xCamAePreRes->unref(xCamAePreRes);
    }
#endif

    RKAiqAecExpInfo_t *curExp = pAsharpProcParams->com.u.proc.curExp;
    if(curExp != NULL) {
        stExpInfo.snr_mode = curExp->CISFeature.SNR;
        if(pAsharpProcParams->hdr_mode == RK_AIQ_WORKING_MODE_NORMAL) {
            stExpInfo.hdr_mode = 0;
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
            if(curExp->LinearExp.exp_real_params.isp_dgain < 1.0) {
                stExpInfo.arDGain[0] *= 1.0;
                LOGW_ANR("leanr mode dgain is wrong, use 1.0 instead\n");
            } else {
                stExpInfo.arDGain[0] *= curExp->LinearExp.exp_real_params.isp_dgain;
            }
            stExpInfo.arTime[0] = curExp->LinearExp.exp_real_params.integration_time;
            stExpInfo.arIso[0] = stExpInfo.arAGain[0] * stExpInfo.arDGain[0] * 50;
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
                } else {
                    LOGW_ANR("hdr mode dgain is wrong, use 1.0 instead\n");
                    stExpInfo.arDGain[i] = curExp->HdrExp[i].exp_real_params.digital_gain;
                }
                stExpInfo.arTime[i] = curExp->HdrExp[i].exp_real_params.integration_time;
                stExpInfo.arIso[i] = stExpInfo.arAGain[i] * stExpInfo.arDGain[i] * 50;

                LOGD_ANR("%s:%d index:%d again:%f dgain:%f time:%f iso:%d hdr_mode:%d\n",
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
        LOGE_ANR("%s:%d curExp is NULL, so use default instead \n", __FUNCTION__, __LINE__);
    }

#if 0
    static int anr_cnt = 0;
    anr_cnt++;

    if(anr_cnt % 50 == 0) {
        for(int i = 0; i < stExpInfo.hdr_mode + 1; i++) {
            printf("%s:%d index:%d again:%f dgain:%f time:%f iso:%d hdr_mode:%d snr_mode:%d\n",
                   __FUNCTION__, __LINE__,
                   i,
                   stExpInfo.arAGain[i],
                   stExpInfo.arDGain[i],
                   stExpInfo.arTime[i],
                   stExpInfo.arIso[i],
                   stExpInfo.hdr_mode,
                   stExpInfo.snr_mode);
        }
    }
#endif

    DeltaIso = abs(stExpInfo.arIso[stExpInfo.hdr_mode] - pAsharpCtx->stExpInfo.arIso[stExpInfo.hdr_mode]);
    if(DeltaIso > ASHARPV4_RECALCULATE_DELTA_ISO) {
        pAsharpCtx->isReCalculate |= 1;
    }

    if(pAsharpCtx->isReCalculate) {
        Asharp4_result_t ret = Asharp_Process_V4(pAsharpCtx, &stExpInfo);
        if(ret != ASHARP4_RET_SUCCESS) {
            result = XCAM_RETURN_ERROR_FAILED;
            LOGE_ASHARP("%s: processing ANR failed (%d)\n", __FUNCTION__, ret);
        }

        Asharp_GetProcResult_V4(pAsharpCtx, &pAsharpProcResParams->stAsharpProcResult);
        outparams->cfg_update = true;

        LOGD_ASHARP("recalculate: %d delta_iso:%d \n ", pAsharpCtx->isReCalculate, DeltaIso);
    } else {
        outparams->cfg_update = false;
    }

    pAsharpCtx->isReCalculate = 0;
#endif

    LOGD_ASHARP("%s: oyyf(exit)\n", __FUNCTION__ );
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
post_process(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    LOGI_ASHARP("%s: (enter)\n", __FUNCTION__ );

    //nothing todo now

    LOGI_ASHARP("%s: (exit)\n", __FUNCTION__ );
    return XCAM_RETURN_NO_ERROR;
}

RkAiqAlgoDescription g_RkIspAlgoDescAsharpV4 = {
    .common = {
        .version = RKISP_ALGO_ASHARP_VERSION_V4,
        .vendor  = RKISP_ALGO_ASHARP_VENDOR_V4,
        .description = RKISP_ALGO_ASHARP_DESCRIPTION_V4,
        .type    = RK_AIQ_ALGO_TYPE_ASHARP,
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
