/*
 * rk_aiq_algo_anr_itf.c
 *
 *  Copyright (c) 2024 Rockchip Corporation
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

#include "ayuvmeV1/rk_aiq_ayuvme_algo_itf_v1.h"
#include "ayuvmeV1/rk_aiq_ayuvme_algo_v1.h"
#include "rk_aiq_algo_types.h"

RKAIQ_BEGIN_DECLARE

typedef struct _RkAiqAlgoContext {
    Ayuvme_Context_V1_t AyuvmeCtx;
} RkAiqAlgoContext;


static XCamReturn
create_context(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{

    XCamReturn result = XCAM_RETURN_NO_ERROR;
    LOGI_ANR("%s: (enter)\n", __FUNCTION__ );

#if 1
    Ayuvme_Context_V1_t* pAyuvmeCtx = NULL;
#if (AYUVME_USE_JSON_FILE_V1)
    Ayuvme_result_V1_t ret = Ayuvme_Init_V1(&pAyuvmeCtx, cfg->calibv2);
#endif
    if(ret != AYUVMEV1_RET_SUCCESS) {
        result = XCAM_RETURN_ERROR_FAILED;
        LOGE_ANR("%s: Initializaion ANR failed (%d)\n", __FUNCTION__, ret);
    } else {
        *context = (RkAiqAlgoContext *)(pAyuvmeCtx);
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
    Ayuvme_Context_V1_t* pAyuvmeCtx = (Ayuvme_Context_V1_t*)context;
    Ayuvme_result_V1_t ret = Ayuvme_Release_V1(pAyuvmeCtx);
    if(ret != AYUVMEV1_RET_SUCCESS) {
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

    Ayuvme_Context_V1_t* pAyuvmeCtx = (Ayuvme_Context_V1_t *)params->ctx;
    RkAiqAlgoConfigAyuvmeV1* pCfgParam = (RkAiqAlgoConfigAyuvmeV1*)params;
    pAyuvmeCtx->prepare_type = params->u.prepare.conf_type;

    if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )) {
        // just update calib ptr
        if (params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB_PTR) {
            return XCAM_RETURN_NO_ERROR;
        }
#if AYUVME_USE_JSON_FILE_V1
        void *pCalibdbV2 = (void*)(pCfgParam->com.u.prepare.calibv2);
        CalibDbV2_YuvmeV1_t *yuvme_v1 = (CalibDbV2_YuvmeV1_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCalibdbV2, yuvme_v1));
        pAyuvmeCtx->yuvme_v1 = *yuvme_v1;
#endif
        pAyuvmeCtx->isIQParaUpdate = true;
        pAyuvmeCtx->isReCalculate |= 1;
    }

    Ayuvme_result_V1_t ret = Ayuvme_Prepare_V1(pAyuvmeCtx, &pCfgParam->stAyuvmeConfig);
    if(ret != AYUVMEV1_RET_SUCCESS) {
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
    Ayuvme_Context_V1_t* pAyuvmeCtx = (Ayuvme_Context_V1_t *)inparams->ctx;

    RkAiqAlgoPreAyuvmeV1* pAnrPreParams = (RkAiqAlgoPreAyuvmeV1*)inparams;

    oldGrayMode = pAyuvmeCtx->isGrayMode;
    if (pAnrPreParams->com.u.proc.gray_mode) {
        pAyuvmeCtx->isGrayMode = true;
    } else {
        pAyuvmeCtx->isGrayMode = false;
    }

    if(oldGrayMode != pAyuvmeCtx->isGrayMode) {
        pAyuvmeCtx->isReCalculate |= 1;
    }

    Ayuvme_result_V1_t ret = Ayuvme_PreProcess_V1(pAyuvmeCtx);
    if(ret != AYUVMEV1_RET_SUCCESS) {
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
    int deltaIso = 0;
    LOGI_ANR("%s: (enter)\n", __FUNCTION__ );

#if 1
    RkAiqAlgoProcAyuvmeV1* pAyuvmeProcParams = (RkAiqAlgoProcAyuvmeV1*)inparams;
    RkAiqAlgoProcResAyuvmeV1* pAyuvmeProcResParams = (RkAiqAlgoProcResAyuvmeV1*)outparams;
    Ayuvme_Context_V1_t* pAyuvmeCtx = (Ayuvme_Context_V1_t *)inparams->ctx;
    Ayuvme_ExpInfo_V1_t stExpInfo;
    memset(&stExpInfo, 0x00, sizeof(Ayuvme_ExpInfo_V1_t));

    LOGD_ANR("%s:%d init:%d hdr mode:%d  \n",
             __FUNCTION__, __LINE__,
             inparams->u.proc.init,
             pAyuvmeProcParams->hdr_mode);

    bool oldGrayMode = false;
    oldGrayMode = pAyuvmeCtx->isGrayMode;
    if (inparams->u.proc.gray_mode) {
        pAyuvmeCtx->isGrayMode = true;
    } else {
        pAyuvmeCtx->isGrayMode = false;
    }

    if(oldGrayMode != pAyuvmeCtx->isGrayMode) {
        pAyuvmeCtx->isReCalculate |= 1;
    }

    Ayuvme_result_V1_t ret = Ayuvme_PreProcess_V1(pAyuvmeCtx);
    if(ret != AYUVMEV1_RET_SUCCESS) {
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

    if(pAyuvmeProcParams->hdr_mode == RK_AIQ_WORKING_MODE_NORMAL) {
        stExpInfo.hdr_mode = 0;
    } else if(pAyuvmeProcParams->hdr_mode == RK_AIQ_ISP_HDR_MODE_2_FRAME_HDR
              || pAyuvmeProcParams->hdr_mode == RK_AIQ_ISP_HDR_MODE_2_LINE_HDR ) {
        stExpInfo.hdr_mode = 1;
    } else if(pAyuvmeProcParams->hdr_mode == RK_AIQ_ISP_HDR_MODE_3_FRAME_HDR
              || pAyuvmeProcParams->hdr_mode == RK_AIQ_ISP_HDR_MODE_3_LINE_HDR ) {
        stExpInfo.hdr_mode = 2;
    }
    stExpInfo.snr_mode = 0;
	stExpInfo.bayertnr_en = 1;

    stExpInfo.blc_ob_predgain = 1.0;
    if(pAyuvmeProcParams != NULL) {
        if (pAyuvmeProcParams->stAblcV32_proc_res)
            stExpInfo.blc_ob_predgain = pAyuvmeProcParams->stAblcV32_proc_res->isp_ob_predgain;
		stExpInfo.bayertnr_en = pAyuvmeProcParams->bayertnr_en;
    }
#if 0// TODO Merge:
    XCamVideoBuffer* xCamAePreRes = pAyuvmeProcParams->com.u.proc.res_comb->ae_pre_res;
    RkAiqAlgoPreResAe* pAEPreRes = nullptr;
    if (xCamAePreRes) {
        // xCamAePreRes->ref(xCamAePreRes);
        pAEPreRes = (RkAiqAlgoPreResAe*)xCamAePreRes->map(xCamAePreRes);
        if (!pAEPreRes) {
            LOGE_ANR("ae pre result is null");
        } else {
            LOGD_ANR("ae_pre_result: meanluma:%f time:%f gain:%f env_luma:%f \n",
                     pAEPreRes->ae_pre_res_rk.MeanLuma[0],
                     pAEPreRes->ae_pre_res_rk.LinearExp.exp_real_params.integration_time,
                     pAEPreRes->ae_pre_res_rk.LinearExp.exp_real_params.analog_gain,
                     pAEPreRes->ae_pre_res_rk.GlobalEnvLux);
        }
        // xCamAePreRes->unref(xCamAePreRes);
    }
#endif

    RKAiqAecExpInfo_t *curExp = pAyuvmeProcParams->com.u.proc.curExp;
    if(curExp != NULL) {
        stExpInfo.snr_mode = curExp->CISFeature.SNR;
        if(pAyuvmeProcParams->hdr_mode == RK_AIQ_WORKING_MODE_NORMAL) {
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

    deltaIso = abs(stExpInfo.arIso[stExpInfo.hdr_mode] - pAyuvmeCtx->stExpInfo.arIso[stExpInfo.hdr_mode]);
    if(deltaIso > AYUVMEV1_RECALCULATE_DELTA_ISO) {
        pAyuvmeCtx->isReCalculate |= 1;
    }
    if(stExpInfo.blc_ob_predgain != pAyuvmeCtx->stExpInfo.blc_ob_predgain) {
        pAyuvmeCtx->isReCalculate |= 1;
    }
	if(stExpInfo.bayertnr_en != pAyuvmeCtx->stExpInfo.bayertnr_en) {
        pAyuvmeCtx->isReCalculate |= 1;
    }
    if(pAyuvmeCtx->isReCalculate) {
        Ayuvme_result_V1_t ret = Ayuvme_Process_V1(pAyuvmeCtx, &stExpInfo);
        if(ret != AYUVMEV1_RET_SUCCESS) {
            result = XCAM_RETURN_ERROR_FAILED;
            LOGE_ANR("%s: processing ANR failed (%d)\n", __FUNCTION__, ret);
        }
        Ayuvme_GetProcResult_V1(pAyuvmeCtx, &pAyuvmeProcResParams->stAyuvmeProcResult);
        outparams->cfg_update = true;
        LOGD_ANR("recalculate: %d delta_iso:%d \n ", pAyuvmeCtx->isReCalculate, deltaIso);
    } else {
        outparams->cfg_update = false;
    }
#endif


    pAyuvmeCtx->isReCalculate = 0;

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

RkAiqAlgoDescription g_RkIspAlgoDescAyuvmeV1 = {
    .common = {
        .version = RKISP_ALGO_AYUVME_VERSION_V1,
        .vendor  = RKISP_ALGO_AYUVME_VENDOR_V1,
        .description = RKISP_ALGO_AYUVME_DESCRIPTION_V1,
        .type    = RK_AIQ_ALGO_TYPE_AMD,
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
