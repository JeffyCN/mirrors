/*
 * rk_aiq_asharp_algo_itf_v33.cpp
 *
 *  Copyright (c) 2022 Rockchip Corporation
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
#include "asharpV33/rk_aiq_asharp_algo_itf_v33.h"

#include "asharpV33/rk_aiq_asharp_algo_v33.h"
#include "rk_aiq_algo_types.h"

RKAIQ_BEGIN_DECLARE

typedef struct _RkAiqAlgoContext {
    Asharp_Context_V33_t AsharpCtx;
} RkAiqAlgoContext;

static XCamReturn create_context(RkAiqAlgoContext** context, const AlgoCtxInstanceCfg* cfg) {
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    LOGI_ASHARP("%s:(enter)\n", __FUNCTION__);

#if 1
    Asharp_Context_V33_t* pAsharpCtx = NULL;
#if (ASHARP_USE_JSON_FILE_V33)
    Asharp_result_V33_t ret = Asharp_Init_V33(&pAsharpCtx, cfg->calibv2);
#else
    Asharp_result_V33_t ret = Asharp_Init_V33(&pAsharpCtx, cfg->calib);
#endif

    if (ret != ASHARP_V33_RET_SUCCESS) {
        result = XCAM_RETURN_ERROR_FAILED;
        LOGE_ASHARP("%s: Initializaion ASHARP failed (%d)\n", __FUNCTION__, ret);
    } else {
        *context = (RkAiqAlgoContext*)(pAsharpCtx);
    }
#endif

    LOGI_ASHARP("%s:(exit)\n", __FUNCTION__);
    return result;
}

static XCamReturn destroy_context(RkAiqAlgoContext* context) {
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    LOGI_ASHARP("%s: oyyf (enter)\n", __FUNCTION__);

#if 1
    Asharp_Context_V33_t* pAsharpCtx = (Asharp_Context_V33_t*)context;
    Asharp_result_V33_t ret          = Asharp_Release_V33(pAsharpCtx);
    if (ret != ASHARP_V33_RET_SUCCESS) {
        result = XCAM_RETURN_ERROR_FAILED;
        LOGE_ASHARP("%s: release ANR failed (%d)\n", __FUNCTION__, ret);
    }
#endif

    LOGI_ASHARP("%s: (exit)\n", __FUNCTION__);
    return result;
}

static XCamReturn prepare(RkAiqAlgoCom* params) {
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    LOGI_ASHARP("%s: (enter)\n", __FUNCTION__);

    Asharp_Context_V33_t* pAsharpCtx    = (Asharp_Context_V33_t*)params->ctx;
    RkAiqAlgoConfigAsharpV33* pCfgParam = (RkAiqAlgoConfigAsharpV33*)params;
    pAsharpCtx->prepare_type            = params->u.prepare.conf_type;

    if (!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB)) {
#if (ASHARP_USE_JSON_FILE_V33)
#if RKAIQ_HAVE_SHARP_V33
        CalibDbV2_SharpV33_t* calibv2_sharp = (CalibDbV2_SharpV33_t*)(CALIBDBV2_GET_MODULE_PTR(
                pCfgParam->com.u.prepare.calibv2, sharp_v33));
#else
        CalibDbV2_SharpV33Lite_t* calibv2_sharp =
            (CalibDbV2_SharpV33Lite_t*)(CALIBDBV2_GET_MODULE_PTR(pCfgParam->com.u.prepare.calibv2,
                                                                 sharp_v33));
#endif
        pAsharpCtx->sharp_v33               = *calibv2_sharp;
        // just update calib ptr
        if (params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB_PTR) {
            return XCAM_RETURN_NO_ERROR;
        }
#endif
        pAsharpCtx->isIQParaUpdate = true;
        pAsharpCtx->isReCalculate |= 1;
    }

    Asharp_result_V33_t ret = Asharp_Prepare_V33(pAsharpCtx, &pCfgParam->stAsharpConfig);
    if (ret != ASHARP_V33_RET_SUCCESS) {
        result = XCAM_RETURN_ERROR_FAILED;
        LOGE_ASHARP("%s: config ANR failed (%d)\n", __FUNCTION__, ret);
    }

    LOGI_ASHARP("%s: (exit)\n", __FUNCTION__);
    return result;
}
#if 0
static XCamReturn pre_process(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams) {
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    bool oldGrayMode  = false;

    LOGD_ASHARP("%s: oyyf (enter)\n", __FUNCTION__);
    Asharp_Context_V33_t* pAsharpCtx = (Asharp_Context_V33_t*)inparams->ctx;

    RkAiqAlgoPreAsharpV33* pAsharpPreParams = (RkAiqAlgoPreAsharpV33*)inparams;

    oldGrayMode = pAsharpCtx->isGrayMode;
    if (pAsharpPreParams->com.u.proc.gray_mode) {
        pAsharpCtx->isGrayMode = true;
    } else {
        pAsharpCtx->isGrayMode = false;
    }

    if (oldGrayMode != pAsharpCtx->isGrayMode) {
        pAsharpCtx->isReCalculate |= 1;
    }

    Asharp_result_V33_t ret = Asharp_PreProcess_V33(pAsharpCtx);
    if (ret != ASHARP_V33_RET_SUCCESS) {
        result = XCAM_RETURN_ERROR_FAILED;
        LOGE_ASHARP("%s: ANRPreProcess failed (%d)\n", __FUNCTION__, ret);
    }

    LOGI_ASHARP("%s: (exit)\n", __FUNCTION__);
    return result;
}
#endif
static XCamReturn processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams) {
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    int DeltaIso      = 0;

    LOGD_ASHARP("%s:oyyf (enter)\n", __FUNCTION__);

#if 1
    RkAiqAlgoProcAsharpV33* pAsharpProcParams       = (RkAiqAlgoProcAsharpV33*)inparams;
    RkAiqAlgoProcResAsharpV33* pAsharpProcResParams = (RkAiqAlgoProcResAsharpV33*)outparams;
    Asharp_Context_V33_t* pAsharpCtx                = (Asharp_Context_V33_t*)inparams->ctx;
    Asharp_ExpInfo_V33_t stExpInfo;
    memset(&stExpInfo, 0x00, sizeof(Asharp_ExpInfo_V33_t));

    if (pAsharpProcParams == NULL) {
        LOGE_ASHARP("%s:%d pointer pAsharpProcParams is NULL", __FUNCTION__, __LINE__);
        return XCAM_RETURN_BYPASS;
    }

    LOGD_ASHARP("%s:%d init:%d hdr mode:%d  \n", __FUNCTION__, __LINE__, inparams->u.proc.init,
                pAsharpProcParams->hdr_mode);

    bool oldGrayMode  = false;
    oldGrayMode = pAsharpCtx->isGrayMode;
    if (inparams->u.proc.gray_mode) {
        pAsharpCtx->isGrayMode = true;
    } else {
        pAsharpCtx->isGrayMode = false;
    }

    if (oldGrayMode != pAsharpCtx->isGrayMode) {
        pAsharpCtx->isReCalculate |= 1;
    }

    Asharp_result_V33_t ret = Asharp_PreProcess_V33(pAsharpCtx);
    if (ret != ASHARP_V33_RET_SUCCESS) {
        result = XCAM_RETURN_ERROR_FAILED;
        LOGE_ASHARP("%s: ANRPreProcess failed (%d)\n", __FUNCTION__, ret);
    }

    stExpInfo.hdr_mode = 0;  // pAsharpProcParams->hdr_mode;
    for (int i = 0; i < 3; i++) {
        stExpInfo.arIso[i]   = 50;
        stExpInfo.arAGain[i] = 1.0;
        stExpInfo.arDGain[i] = 1.0;
        stExpInfo.arTime[i]  = 0.01;
    }

    if (pAsharpProcParams->hdr_mode == RK_AIQ_WORKING_MODE_NORMAL) {
        stExpInfo.hdr_mode = 0;
    } else if (pAsharpProcParams->hdr_mode == RK_AIQ_ISP_HDR_MODE_2_FRAME_HDR ||
               pAsharpProcParams->hdr_mode == RK_AIQ_ISP_HDR_MODE_2_LINE_HDR) {
        stExpInfo.hdr_mode = 1;
    } else if (pAsharpProcParams->hdr_mode == RK_AIQ_ISP_HDR_MODE_3_FRAME_HDR ||
               pAsharpProcParams->hdr_mode == RK_AIQ_ISP_HDR_MODE_3_LINE_HDR) {
        stExpInfo.hdr_mode = 2;
    }
    stExpInfo.snr_mode = 0;

    stExpInfo.blc_ob_predgain = 1.0;

    stExpInfo.blc_ob_predgain = pAsharpProcParams->stAblcV32_proc_res->isp_ob_predgain;
    if(stExpInfo.blc_ob_predgain != pAsharpCtx->stExpInfo.blc_ob_predgain) {
        pAsharpCtx->isReCalculate |= 1;
    }
#if 0  // TODO Merge:
    XCamVideoBuffer* xCamAePreRes = pAsharpProcParams->com.u.proc.res_comb->ae_pre_res;
    RkAiqAlgoPreResAe* pAEPreRes  = nullptr;
    if (xCamAePreRes) {
        // xCamAePreRes->ref(xCamAePreRes);
        pAEPreRes = (RkAiqAlgoPreResAe*)xCamAePreRes->map(xCamAePreRes);
        if (!pAEPreRes) {
            LOGE_ASHARP("ae pre result is null");
        } else {
            LOGD_ASHARP("ae_pre_result: meanluma:%f time:%f gain:%f env_luma:%f \n",
                        pAEPreRes->ae_pre_res_rk.MeanLuma[0],
                        pAEPreRes->ae_pre_res_rk.LinearExp.exp_real_params.integration_time,
                        pAEPreRes->ae_pre_res_rk.LinearExp.exp_real_params.analog_gain,
                        pAEPreRes->ae_pre_res_rk.GlobalEnvLux);
        }
        // xCamAePreRes->unref(xCamAePreRes);
    }
#endif

    RKAiqAecExpInfo_t* curExp = pAsharpProcParams->com.u.proc.curExp;
    if (curExp != NULL) {
        stExpInfo.snr_mode = curExp->CISFeature.SNR;
        if (pAsharpProcParams->hdr_mode == RK_AIQ_WORKING_MODE_NORMAL) {
            stExpInfo.hdr_mode = 0;
            if (curExp->LinearExp.exp_real_params.analog_gain < 1.0) {
                stExpInfo.arAGain[0] = 1.0;
                LOGW_ASHARP("leanr mode again is wrong, use 1.0 instead\n");
            } else {
                stExpInfo.arAGain[0] = curExp->LinearExp.exp_real_params.analog_gain;
            }
            if (curExp->LinearExp.exp_real_params.digital_gain < 1.0) {
                stExpInfo.arDGain[0] = 1.0;
                LOGW_ASHARP("leanr mode dgain is wrong, use 1.0 instead\n");
            } else {
                stExpInfo.arDGain[0] = curExp->LinearExp.exp_real_params.digital_gain;
            }
            if (curExp->LinearExp.exp_real_params.isp_dgain < 1.0) {
                stExpInfo.isp_dgain[0] = 1.0;
                LOGW_ASHARP("leanr mode isp_dgain is wrong, use 1.0 instead\n");
            } else {
                stExpInfo.isp_dgain[0] = curExp->LinearExp.exp_real_params.isp_dgain;
            }
            if(stExpInfo.blc_ob_predgain < 1.0) {
                stExpInfo.blc_ob_predgain = 1.0;
            }
            stExpInfo.arTime[0] = curExp->LinearExp.exp_real_params.integration_time;
            stExpInfo.arIso[0]  = stExpInfo.arAGain[0] * stExpInfo.arDGain[0] * stExpInfo.blc_ob_predgain * 50 * stExpInfo.isp_dgain[0];
            LOGD_ASHARP("new snr mode:%d old:%d  gain:%f isp_dgain:%f iso:%d time:%f\n",
                        stExpInfo.snr_mode,
                        pAsharpCtx->stExpInfo.snr_mode,
                        stExpInfo.arDGain[0],
                        stExpInfo.isp_dgain[0],
                        stExpInfo.arIso[0],
                        stExpInfo.arTime[0]
                       );
        } else {
            for (int i = 0; i < 3; i++) {
                if (curExp->HdrExp[i].exp_real_params.analog_gain < 1.0) {
                    stExpInfo.arAGain[i] = 1.0;
                    LOGW_ASHARP("hdr mode again is wrong, use 1.0 instead\n");
                } else {
                    stExpInfo.arAGain[i] = curExp->HdrExp[i].exp_real_params.analog_gain;
                }
                if (curExp->HdrExp[i].exp_real_params.digital_gain < 1.0) {
                    stExpInfo.arDGain[i] = 1.0;
                    LOGW_ASHARP("hdr mode dgain is wrong, use 1.0 instead\n");
                } else {
                    stExpInfo.arDGain[i] = curExp->HdrExp[i].exp_real_params.digital_gain;
                }
                if (curExp->HdrExp[i].exp_real_params.isp_dgain < 1.0) {
                    stExpInfo.isp_dgain[i] = 1.0;
                    LOGW_ASHARP("hdr mode isp_dgain is wrong, use 1.0 instead\n");
                } else {
                    stExpInfo.arDGain[i] = curExp->HdrExp[i].exp_real_params.isp_dgain;
                }
                stExpInfo.blc_ob_predgain = 1.0;
                stExpInfo.arTime[i] = curExp->HdrExp[i].exp_real_params.integration_time;
                stExpInfo.arIso[i]  = stExpInfo.arAGain[i] * stExpInfo.arDGain[i] * 50 * stExpInfo.arDGain[i];

                LOGD_ASHARP("%s:%d index:%d again:%f dgain:%f isp_dgain:%f time:%f iso:%d hdr_mode:%d\n",
                            __FUNCTION__, __LINE__, i, stExpInfo.arAGain[i], stExpInfo.arDGain[i], stExpInfo.arDGain[i],
                            stExpInfo.arTime[i], stExpInfo.arIso[i], stExpInfo.hdr_mode);
            }
        }
    } else {
        LOGE_ASHARP("%s:%d curExp is NULL, so use default instead \n", __FUNCTION__, __LINE__);
    }

    DeltaIso =
        abs(stExpInfo.arIso[stExpInfo.hdr_mode] - pAsharpCtx->stExpInfo.arIso[stExpInfo.hdr_mode]);
    if (DeltaIso > ASHARPV33_RECALCULATE_DELTA_ISO) {
        pAsharpCtx->isReCalculate |= 1;
    }

    if (pAsharpCtx->isReCalculate) {
        Asharp_result_V33_t ret = Asharp_Process_V33(pAsharpCtx, &stExpInfo);
        if (ret != ASHARP_V33_RET_SUCCESS) {
            result = XCAM_RETURN_ERROR_FAILED;
            LOGE_ASHARP("%s: processing ANR failed (%d)\n", __FUNCTION__, ret);
        }

        Asharp_GetProcResult_V33(pAsharpCtx, &pAsharpProcResParams->stAsharpProcResult);
        outparams->cfg_update = true;

        LOGD_ASHARP("recalculate: %d delta_iso:%d \n ", pAsharpCtx->isReCalculate, DeltaIso);
    } else {
        outparams->cfg_update = false;
    }

    pAsharpCtx->isReCalculate = 0;
#endif
    LOGI_ASHARP("%s: (exit)\n", __FUNCTION__);
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn post_process(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams) {
    LOGI_ASHARP("%s: (enter)\n", __FUNCTION__);

    // nothing todo now

    LOGI_ASHARP("%s: (exit)\n", __FUNCTION__);
    return XCAM_RETURN_NO_ERROR;
}

RkAiqAlgoDescription g_RkIspAlgoDescAsharpV33 = {
    .common =
    {
        .version         = RKISP_ALGO_ASHARP_VERSION_V33,
        .vendor          = RKISP_ALGO_ASHARP_VENDOR_V33,
        .description     = RKISP_ALGO_ASHARP_DESCRIPTION_V33,
        .type            = RK_AIQ_ALGO_TYPE_ASHARP,
        .id              = 0,
        .create_context  = create_context,
        .destroy_context = destroy_context,
    },
    .prepare      = prepare,
    .pre_process  = NULL,
    .processing   = processing,
    .post_process = NULL,
};

RKAIQ_END_DECLARE
