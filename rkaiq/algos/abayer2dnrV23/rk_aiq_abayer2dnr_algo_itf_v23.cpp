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

#include "abayer2dnrV23/rk_aiq_abayer2dnr_algo_itf_v23.h"
#include "abayer2dnrV23/rk_aiq_abayer2dnr_algo_v23.h"
#include "rk_aiq_algo_types.h"

RKAIQ_BEGIN_DECLARE

typedef struct _RkAiqAlgoContext {
    Abayer2dnr_Context_V23_t AbayernrCtx;
} RkAiqAlgoContext;


static XCamReturn
create_context(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{

    XCamReturn result = XCAM_RETURN_NO_ERROR;
    LOGI_ANR("%s: (enter)\n", __FUNCTION__ );

#if 1
    Abayer2dnr_Context_V23_t* pAbayernrCtx = NULL;
#if (ABAYER2DNR_USE_JSON_FILE_V23)
    Abayer2dnr_result_V23_t ret = Abayer2dnr_Init_V23(&pAbayernrCtx, cfg->calibv2);
#else
    Abayer2dnr_result_V23_t ret = Abayer2dnr_Init_V23(&pAbayernrCtx, cfg->calib);
#endif
    if(ret != ABAYER2DNR_V23_RET_SUCCESS) {
        result = XCAM_RETURN_ERROR_FAILED;
        LOGE_ANR("%s: Initializaion ANR failed (%d)\n", __FUNCTION__, ret);
    } else {
        *context = (RkAiqAlgoContext *)(pAbayernrCtx);
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
    Abayer2dnr_Context_V23_t* pAbayernrCtx = (Abayer2dnr_Context_V23_t*)context;
    Abayer2dnr_result_V23_t ret = Abayer2dnr_Release_V23(pAbayernrCtx);
    if(ret != ABAYER2DNR_V23_RET_SUCCESS) {
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

    Abayer2dnr_Context_V23_t* pAbayernrCtx = (Abayer2dnr_Context_V23_t *)params->ctx;
    RkAiqAlgoConfigAbayer2dnrV23* pCfgParam = (RkAiqAlgoConfigAbayer2dnrV23*)params;
    pAbayernrCtx->prepare_type = params->u.prepare.conf_type;

    if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )) {
#if ABAYER2DNR_USE_JSON_FILE_V23
        void *pCalibDbV2 = (void*)(pCfgParam->com.u.prepare.calibv2);
        CalibDbV2_Bayer2dnrV23_t *bayernr_v23 = (CalibDbV2_Bayer2dnrV23_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCalibDbV2, bayer2dnr_v23));
        pAbayernrCtx->bayernr_v23 = *bayernr_v23;
#endif
        // just update calib ptr
        if (params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB_PTR)
            return XCAM_RETURN_NO_ERROR;

        pAbayernrCtx->isIQParaUpdate = true;
        pAbayernrCtx->isReCalculate |= 1;
    }

    Abayer2dnr_result_V23_t ret = Abayer2dnr_Prepare_V23(pAbayernrCtx, &pCfgParam->stArawnrConfig);
    if(ret != ABAYER2DNR_V23_RET_SUCCESS) {
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
    Abayer2dnr_Context_V23_t* pAbayernrCtx = (Abayer2dnr_Context_V23_t *)inparams->ctx;

    RkAiqAlgoPreAbayer2dnrV23* pAbayernrPreParams = (RkAiqAlgoPreAbayer2dnrV23*)inparams;

    oldGrayMode = pAbayernrCtx->isGrayMode;
    if (pAbayernrPreParams->com.u.proc.gray_mode) {
        pAbayernrCtx->isGrayMode = true;
    } else {
        pAbayernrCtx->isGrayMode = false;
    }

    if(oldGrayMode != pAbayernrCtx->isGrayMode) {
        pAbayernrCtx->isReCalculate |= 1;
    }

    Abayer2dnr_result_V23_t ret = Abayer2dnr_PreProcess_V23(pAbayernrCtx);
    if(ret != ABAYER2DNR_V23_RET_SUCCESS) {
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
    RkAiqAlgoProcAbayer2dnrV23* pAbayernrProcParams = (RkAiqAlgoProcAbayer2dnrV23*)inparams;
    RkAiqAlgoProcResAbayer2dnrV23* pAbayernrProcResParams = (RkAiqAlgoProcResAbayer2dnrV23*)outparams;
    Abayer2dnr_Context_V23_t* pAbayernrCtx = (Abayer2dnr_Context_V23_t *)inparams->ctx;
    Abayer2dnr_ExpInfo_V23_t stExpInfo;
    memset(&stExpInfo, 0x00, sizeof(Abayer2dnr_ExpInfo_V23_t));

    bool oldGrayMode = false;
    oldGrayMode = pAbayernrCtx->isGrayMode;
    if (inparams->u.proc.gray_mode) {
        pAbayernrCtx->isGrayMode = true;
    } else {
        pAbayernrCtx->isGrayMode = false;
    }

    if(oldGrayMode != pAbayernrCtx->isGrayMode) {
        pAbayernrCtx->isReCalculate |= 1;
    }

    Abayer2dnr_result_V23_t ret = Abayer2dnr_PreProcess_V23(pAbayernrCtx);
    if(ret != ABAYER2DNR_V23_RET_SUCCESS) {
        result = XCAM_RETURN_ERROR_FAILED;
        LOGE_ANR("%s: ANRPreProcess failed (%d)\n", __FUNCTION__, ret);
    }

    if (pAbayernrProcParams == NULL) {
            LOGE_ANR("%s: ANRProcessing pAbayernrProcParams is NULL\n", __FUNCTION__);
        return XCAM_RETURN_BYPASS;
    }

    LOGD_ANR("%s:%d init:%d hdr mode:%d  \n",
             __FUNCTION__, __LINE__,
             inparams->u.proc.init,
             pAbayernrProcParams->hdr_mode);

    stExpInfo.hdr_mode = 0; //pAnrProcParams->hdr_mode;
    for(int i = 0; i < 3; i++) {
        stExpInfo.arIso[i] = 50;
        stExpInfo.arAGain[i] = 1.0;
        stExpInfo.arDGain[i] = 1.0;
        stExpInfo.arTime[i] = 0.01;
    }
    stExpInfo.blc_ob_predgain = 1.0;
    stExpInfo.bayertnr_en = 1;

    if(pAbayernrProcParams != NULL) {
        stExpInfo.blc_ob_predgain = pAbayernrProcParams->stAblcV32_proc_res->isp_ob_predgain;
        stExpInfo.bayertnr_en = pAbayernrProcParams->bayertnr_en;
        if(stExpInfo.blc_ob_predgain != pAbayernrCtx->stExpInfo.blc_ob_predgain
                || stExpInfo.bayertnr_en != pAbayernrCtx->stExpInfo.bayertnr_en) {
            pAbayernrCtx->isReCalculate |= 1;
        }
    }

    if(pAbayernrProcParams->hdr_mode == RK_AIQ_WORKING_MODE_NORMAL) {
        stExpInfo.hdr_mode = 0;
    } else if(pAbayernrProcParams->hdr_mode == RK_AIQ_ISP_HDR_MODE_2_FRAME_HDR
              || pAbayernrProcParams->hdr_mode == RK_AIQ_ISP_HDR_MODE_2_LINE_HDR ) {
        stExpInfo.hdr_mode = 1;
    } else if(pAbayernrProcParams->hdr_mode == RK_AIQ_ISP_HDR_MODE_3_FRAME_HDR
              || pAbayernrProcParams->hdr_mode == RK_AIQ_ISP_HDR_MODE_3_LINE_HDR ) {
        stExpInfo.hdr_mode = 2;
    }
    stExpInfo.snr_mode = 0;
    stExpInfo.gray_mode = pAbayernrCtx->isGrayMode;

#if 0// TODO Merge:
    XCamVideoBuffer* xCamAePreRes = pAbayernrProcParams->com.u.proc.res_comb->ae_pre_res;
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

    RKAiqAecExpInfo_t *curExp = pAbayernrProcParams->com.u.proc.curExp;
    if(curExp != NULL) {
        stExpInfo.snr_mode = curExp->CISFeature.SNR;
        if(pAbayernrProcParams->hdr_mode == RK_AIQ_WORKING_MODE_NORMAL) {
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
            float tmp_preDgain = stExpInfo.blc_ob_predgain;
            if(tmp_preDgain < 1.0) {
                tmp_preDgain = 1.0;
            }
            // stExpInfo.arAGain[0] = 64.0;
            stExpInfo.arTime[0] = curExp->LinearExp.exp_real_params.integration_time;
            stExpInfo.arIso[0] = stExpInfo.arAGain[0] * stExpInfo.arDGain[0] * tmp_preDgain * 50 * stExpInfo.isp_dgain[0];
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
                if(curExp->HdrExp[i].exp_real_params.isp_dgain < 1.0) {
                    stExpInfo.isp_dgain[i] = 1.0;
                    LOGW_ANR("hdr mode isp_dgain is wrong, use 1.0 instead\n");
                } else {
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
    delta_iso = abs(stExpInfo.arIso[stExpInfo.hdr_mode] - pAbayernrCtx->stExpInfo.arIso[stExpInfo.hdr_mode]);
    if(delta_iso > ABAYER2DNRV23_RECALCULATE_DELTA_ISO) {
        pAbayernrCtx->isReCalculate |= 1;
    }

    if(pAbayernrCtx->isReCalculate) {
        Abayer2dnr_result_V23_t ret = Abayer2dnr_Process_V23(pAbayernrCtx, &stExpInfo);
        if(ret != ABAYER2DNR_V23_RET_SUCCESS) {
            result = XCAM_RETURN_ERROR_FAILED;
            LOGE_ANR("%s: processing ANR failed (%d)\n", __FUNCTION__, ret);
        }

        Abayer2dnr_GetProcResult_V23(pAbayernrCtx, &pAbayernrProcResParams->stArawnrProcResult);

        LOGD_ANR("recalculate: %d delta_iso:%d \n ", pAbayernrCtx->isReCalculate, delta_iso);

        pAbayernrProcResParams->res_com.cfg_update = true;
    } else {
        pAbayernrProcResParams->res_com.cfg_update = false;
    }

    pAbayernrCtx->isReCalculate = 0;

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

RkAiqAlgoDescription g_RkIspAlgoDescAbayer2dnrV23 = {
    .common = {
        .version = RKISP_ALGO_ABAYER2DNR_VERSION_V23,
        .vendor  = RKISP_ALGO_ABAYER2DNR_VENDOR_V23,
        .description = RKISP_ALGO_ABAYER2DNR_DESCRIPTION_V23,
        .type    = RK_AIQ_ALGO_TYPE_ARAWNR,
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
