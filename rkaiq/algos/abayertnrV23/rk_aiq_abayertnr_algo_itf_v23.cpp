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

#include "abayertnrV23/rk_aiq_abayertnr_algo_itf_v23.h"
#include "abayertnrV23/rk_aiq_abayertnr_algo_v23.h"
#include "rk_aiq_algo_types.h"

RKAIQ_BEGIN_DECLARE

typedef struct _RkAiqAlgoContext {
    Abayertnr_Context_V23_t AbayertnrCtx;
} RkAiqAlgoContext;


static XCamReturn
create_context(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{

    XCamReturn result = XCAM_RETURN_NO_ERROR;
    LOGI_ANR("%s: (enter)\n", __FUNCTION__ );

#if 1
    Abayertnr_Context_V23_t* pAbayertnrCtx = NULL;
#if (ABAYERTNR_USE_JSON_FILE_V23)
    Abayertnr_result_V23_t ret = Abayertnr_Init_V23(&pAbayertnrCtx, cfg->calibv2);
#endif
    if(ret != ABAYERTNRV23_RET_SUCCESS) {
        result = XCAM_RETURN_ERROR_FAILED;
        LOGE_ANR("%s: Initializaion ANR failed (%d)\n", __FUNCTION__, ret);
    } else {
        *context = (RkAiqAlgoContext *)(pAbayertnrCtx);
    }
#endif

    LOGI_ANR("%s: (exit)\n", __FUNCTION__ );
    return result;
}

static XCamReturn
destroy_context(RkAiqAlgoContext *context)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    LOGI_ANR("%s: (enter)\n", __FUNCTION__ );

#if 1
    Abayertnr_Context_V23_t* pAbayertnrCtx = (Abayertnr_Context_V23_t*)context;
    Abayertnr_result_V23_t ret = Abayertnr_Release_V23(pAbayertnrCtx);
    if(ret != ABAYERTNRV23_RET_SUCCESS) {
        result = XCAM_RETURN_ERROR_FAILED;
        LOGE_ANR("%s: release ANR failed (%d)\n", __FUNCTION__, ret);
    }
#endif

    LOGI_ANR("%s: (exit)\n", __FUNCTION__ );
    return result;
}

static XCamReturn
prepare(RkAiqAlgoCom* params)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    LOGI_ANR("%s: (enter)\n", __FUNCTION__ );

    Abayertnr_Context_V23_t* pAbayertnrCtx = (Abayertnr_Context_V23_t *)params->ctx;
    RkAiqAlgoConfigAbayertnrV23* pCfgParam = (RkAiqAlgoConfigAbayertnrV23*)params;
    pAbayertnrCtx->prepare_type = params->u.prepare.conf_type;

    if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )) {
#if ABAYERTNR_USE_JSON_FILE_V23
        void* pCalibDbV2 = (void*)(pCfgParam->com.u.prepare.calibv2);
#if (RKAIQ_HAVE_BAYERTNR_V23)
        CalibDbV2_BayerTnrV23_t *bayertnr_v23 = (CalibDbV2_BayerTnrV23_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCalibDbV2, bayertnr_v23));
#else
        CalibDbV2_BayerTnrV23Lite_t* bayertnr_v23 =
            (CalibDbV2_BayerTnrV23Lite_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCalibDbV2,
                                                                    bayertnr_v23));
#endif
        pAbayertnrCtx->bayertnr_v23 = *bayertnr_v23;
#endif
        // just update calib ptr
        if (params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB_PTR)
            return XCAM_RETURN_NO_ERROR;

        pAbayertnrCtx->isIQParaUpdate = true;
        pAbayertnrCtx->isReCalculate |= 1;
    }

    Abayertnr_result_V23_t ret = Abayertnr_Prepare_V23(pAbayertnrCtx, &pCfgParam->stAbayertnrConfig);
    if(ret != ABAYERTNRV23_RET_SUCCESS) {
        result = XCAM_RETURN_ERROR_FAILED;
        LOGE_ANR("%s: config ANR failed (%d)\n", __FUNCTION__, ret);
    }

    LOGI_ANR("%s: (exit)\n", __FUNCTION__ );
    return result;
}
#if 0
static XCamReturn
pre_process(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    bool oldGrayMode = false;

    LOGI_ANR("%s: (enter)\n", __FUNCTION__ );
    Abayertnr_Context_V23_t* pAbayertnrCtx = (Abayertnr_Context_V23_t *)inparams->ctx;

    RkAiqAlgoPreAbayertnrV23* pAbayertnrPreParams = (RkAiqAlgoPreAbayertnrV23*)inparams;

    oldGrayMode = pAbayertnrCtx->isGrayMode;
    if (pAbayertnrPreParams->com.u.proc.gray_mode) {
        pAbayertnrCtx->isGrayMode = true;
    } else {
        pAbayertnrCtx->isGrayMode = false;
    }

    if(oldGrayMode != pAbayertnrCtx->isGrayMode) {
        pAbayertnrCtx->isReCalculate |= 1;
    }

    Abayertnr_result_V23_t ret = Abayertnr_PreProcess_V23(pAbayertnrCtx);
    if(ret != ABAYERTNRV23_RET_SUCCESS) {
        result = XCAM_RETURN_ERROR_FAILED;
        LOGE_ANR("%s: ANRPreProcess failed (%d)\n", __FUNCTION__, ret);
    }

    LOGI_ANR("%s: (exit)\n", __FUNCTION__ );
    return result;
}
#endif
static XCamReturn
processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    int delta_iso = 0;

    LOGI_ANR("%s: (enter)\n", __FUNCTION__ );

#if 1
    RkAiqAlgoProcAbayertnrV23* pAbayertnrProcParams = (RkAiqAlgoProcAbayertnrV23*)inparams;
    RkAiqAlgoProcResAbayertnrV23* pAbayertnrProcResParams = (RkAiqAlgoProcResAbayertnrV23*)outparams;
    Abayertnr_Context_V23_t* pAbayertnrCtx = (Abayertnr_Context_V23_t *)inparams->ctx;
    Abayertnr_ExpInfo_V23_t stExpInfo;
    memset(&stExpInfo, 0x00, sizeof(Abayertnr_ExpInfo_V23_t));

    if (pAbayertnrProcParams == NULL) {
        LOGE_ANR("%s: ANRProcessing pAbayertnrProcParams is NULL\n", __FUNCTION__);
        return XCAM_RETURN_BYPASS;
    }

    LOGD_ANR("%s:%d init:%d hdr mode:%d  \n",
             __FUNCTION__, __LINE__,
             inparams->u.proc.init,
             pAbayertnrProcParams->hdr_mode);

    bool oldGrayMode = false;
    oldGrayMode = pAbayertnrCtx->isGrayMode;
    if (inparams->u.proc.gray_mode) {
        pAbayertnrCtx->isGrayMode = true;
    } else {
        pAbayertnrCtx->isGrayMode = false;
    }

    if(oldGrayMode != pAbayertnrCtx->isGrayMode) {
        pAbayertnrCtx->isReCalculate |= 1;
    }

    Abayertnr_result_V23_t ret = Abayertnr_PreProcess_V23(pAbayertnrCtx);
    if(ret != ABAYERTNRV23_RET_SUCCESS) {
        result = XCAM_RETURN_ERROR_FAILED;
        LOGE_ANR("%s: ANRPreProcess failed (%d)\n", __FUNCTION__, ret);
    }

    stExpInfo.hdr_mode = 0; //pAnrProcParams->hdr_mode;
    for(int i = 0; i < 3; i++) {
        stExpInfo.arIso[i] = 50;
        stExpInfo.arAGain[i] = 1.0;
        stExpInfo.arDGain[i] = 1.0;
        stExpInfo.arTime[i] = 0.01;
    }

    if(pAbayertnrProcParams->hdr_mode == RK_AIQ_WORKING_MODE_NORMAL) {
        stExpInfo.hdr_mode = 0;
    } else if(pAbayertnrProcParams->hdr_mode == RK_AIQ_ISP_HDR_MODE_2_FRAME_HDR
              || pAbayertnrProcParams->hdr_mode == RK_AIQ_ISP_HDR_MODE_2_LINE_HDR ) {
        stExpInfo.hdr_mode = 1;
    } else if(pAbayertnrProcParams->hdr_mode == RK_AIQ_ISP_HDR_MODE_3_FRAME_HDR
              || pAbayertnrProcParams->hdr_mode == RK_AIQ_ISP_HDR_MODE_3_LINE_HDR ) {
        stExpInfo.hdr_mode = 2;
    }
    stExpInfo.snr_mode = 0;

    stExpInfo.blc_ob_predgain = 1.0f;

    LOGD_ANR(" predgain:%f\n",
             pAbayertnrProcParams->stAblcV32_proc_res->isp_ob_predgain);
    stExpInfo.blc_ob_predgain = pAbayertnrProcParams->stAblcV32_proc_res->isp_ob_predgain;
    if(stExpInfo.blc_ob_predgain != pAbayertnrCtx->stExpInfo.blc_ob_predgain) {
        pAbayertnrCtx->isReCalculate |= 1;
    }

#if 0// TODO Merge:
    XCamVideoBuffer* xCamAePreRes = pAbayertnrProcParams->com.u.proc.res_comb->ae_pre_res;
    RkAiqAlgoPreResAe* pAEPreRes = nullptr;
    if (xCamAePreRes) {
        // xCamAePreRes->ref(xCamAePreRes);
        pAEPreRes = (RkAiqAlgoPreResAe*)xCamAePreRes->map(xCamAePreRes);
        if (!pAEPreRes) {
            LOGE_ANR("ae pre result is null");
        } else {

        }
    }
#endif

    RKAiqAecExpInfo_t *curExp = pAbayertnrProcParams->com.u.proc.curExp;
    if(curExp != NULL) {
        stExpInfo.snr_mode = curExp->CISFeature.SNR;
        if(pAbayertnrProcParams->hdr_mode == RK_AIQ_WORKING_MODE_NORMAL) {
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
                stExpInfo.isp_dgain[0] = 1.0;
                LOGW_ANR("leanr mode isp_dgain is wrong, use 1.0 instead\n");
            } else {
                stExpInfo.isp_dgain[0] = curExp->LinearExp.exp_real_params.isp_dgain;
            }
            if(stExpInfo.blc_ob_predgain < 1.0) {
                stExpInfo.blc_ob_predgain = 1.0;
            }
            // stExpInfo.arAGain[0] = 64.0;
            stExpInfo.arTime[0] = curExp->LinearExp.exp_real_params.integration_time;
            stExpInfo.arIso[0] = stExpInfo.arAGain[0] * stExpInfo.arDGain[0] * stExpInfo.blc_ob_predgain * 50 * stExpInfo.isp_dgain[0];
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
                if(curExp->HdrExp[i].exp_real_params.isp_dgain < 1.0) {
                    stExpInfo.isp_dgain[i] = 1.0;
                } else {
                    LOGW_ANR("hdr mode isp_dgain is wrong, use 1.0 instead\n");
                    stExpInfo.isp_dgain[i] = curExp->HdrExp[i].exp_real_params.isp_dgain;
                }
                stExpInfo.blc_ob_predgain = 1.0;
                stExpInfo.arTime[i] = curExp->HdrExp[i].exp_real_params.integration_time;
                stExpInfo.arIso[i] = stExpInfo.arAGain[i] * stExpInfo.arDGain[i] * 50 * stExpInfo.isp_dgain[i];

                LOGD_ANR("%s:%d index:%d again:%f dgain:%f isp_dgain:%f time:%f iso:%d hdr_mode:%d\n",
                         __FUNCTION__, __LINE__,
                         i,
                         stExpInfo.arAGain[i],
                         stExpInfo.arDGain[i],
                         stExpInfo.isp_dgain[i],
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
    delta_iso = abs(stExpInfo.arIso[stExpInfo.hdr_mode] - pAbayertnrCtx->stExpInfo.arIso[stExpInfo.hdr_mode]);
    if(delta_iso > ABAYERTNRV23_RECALCULATE_DELTA_ISO) {
        pAbayertnrCtx->isReCalculate |= 1;
    }

    if(pAbayertnrCtx->isReCalculate) {
        Abayertnr_result_V23_t ret = Abayertnr_Process_V23(pAbayertnrCtx, &stExpInfo);
        if(ret != ABAYERTNRV23_RET_SUCCESS) {
            result = XCAM_RETURN_ERROR_FAILED;
            LOGE_ANR("%s: processing ANR failed (%d)\n", __FUNCTION__, ret);
        }

        Abayertnr_GetProcResult_V23(pAbayertnrCtx, &pAbayertnrProcResParams->stAbayertnrProcResult);

        LOGD_ANR("recalculate: %d delta_iso:%d \n ", pAbayertnrCtx->isReCalculate, delta_iso);

        outparams->cfg_update = true;
    } else {
        outparams->cfg_update = false;
    }

    pAbayertnrCtx->isReCalculate = 0;

#endif

    LOGI_ANR("%s: (exit)\n", __FUNCTION__ );
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
post_process(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    LOGI_ANR("%s: (enter)\n", __FUNCTION__ );

    //nothing todo now

    LOGI_ANR("%s: (exit)\n", __FUNCTION__ );
    return XCAM_RETURN_NO_ERROR;
}

RkAiqAlgoDescription g_RkIspAlgoDescAbayertnrV23 = {
    .common = {
        .version = RKISP_ALGO_ABAYERTNR_VERSION_V23,
        .vendor  = RKISP_ALGO_ABAYERTNR_VENDOR_V23,
        .description = RKISP_ALGO_ABAYERTNR_DESCRIPTION_V23,
        .type    = RK_AIQ_ALGO_TYPE_AMFNR,
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
