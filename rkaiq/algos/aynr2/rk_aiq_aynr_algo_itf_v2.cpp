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

#include "aynr2/rk_aiq_aynr_algo_itf_v2.h"
#include "aynr2/rk_aiq_aynr_algo_v2.h"
#include "rk_aiq_algo_types.h"
#include "RkAiqCalibDbV2Helper.h"

RKAIQ_BEGIN_DECLARE

typedef struct _RkAiqAlgoContext {
    Aynr_Context_V2_t AynrCtx;
} RkAiqAlgoContext;


static XCamReturn
create_context(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{

    XCamReturn result = XCAM_RETURN_NO_ERROR;
    LOGI_ANR("%s: (enter)\n", __FUNCTION__ );

#if 1
    Aynr_Context_V2_t* pAynrCtx = NULL;
#if (AYNR_USE_JSON_FILE_V2)
    Aynr_result_t ret = Aynr_Init_V2(&pAynrCtx, cfg->calibv2);
#else
    Aynr_result_t ret = Aynr_Init_V2(&pAynrCtx, cfg->calib);
#endif
    if(ret != AYNR_RET_SUCCESS) {
        result = XCAM_RETURN_ERROR_FAILED;
        LOGE_ANR("%s: Initializaion ANR failed (%d)\n", __FUNCTION__, ret);
    } else {
        *context = (RkAiqAlgoContext *)(pAynrCtx);
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
    Aynr_Context_V2_t* pAynrCtx = (Aynr_Context_V2_t*)context;
    Aynr_result_t ret = Aynr_Release_V2(pAynrCtx);
    if(ret != AYNR_RET_SUCCESS) {
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

    Aynr_Context_V2_t* pAynrCtx = (Aynr_Context_V2_t *)params->ctx;
    RkAiqAlgoConfigAynrV2* pCfgParam = (RkAiqAlgoConfigAynrV2*)params;
    pAynrCtx->prepare_type = params->u.prepare.conf_type;

    if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )) {
#if AYNR_USE_JSON_FILE_V2
        void *pCalibDbV2 = (void*)(pCfgParam->com.u.prepare.calibv2);
        CalibDbV2_YnrV2_t *ynr_v2 = (CalibDbV2_YnrV2_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCalibDbV2, ynr_v2));
        pAynrCtx->ynr_v2 = *ynr_v2;
        // just update calib ptr
        if (params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB_PTR) {
            return XCAM_RETURN_NO_ERROR;
        }
#else
        void *pCalibDb = (void*)(pCfgParam->com.u.prepare.calib);
        pAynrCtx->list_ynr_v2 =
            (struct list_head*)(CALIBDB_GET_MODULE_PTR((void*)pCalibDb, list_ynr_v2));
        // just update calib ptr
        if (params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB_PTR) {
            return XCAM_RETURN_NO_ERROR;
        }
#endif
        pAynrCtx->isIQParaUpdate = true;
        pAynrCtx->isReCalculate |= 1;
    }

    Aynr_result_t ret = Aynr_Prepare_V2(pAynrCtx, &pCfgParam->stAynrConfig);
    if(ret != AYNR_RET_SUCCESS) {
        result = XCAM_RETURN_ERROR_FAILED;
        LOGE_ANR("%s: config ANR failed (%d)\n", __FUNCTION__, ret);
    }

    LOGI_ANR("%s: (exit)\n", __FUNCTION__ );
    return result;
}

static XCamReturn
pre_process(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    bool oldGrayMode = false;

    LOGI_ANR("%s: (enter)\n", __FUNCTION__ );
    Aynr_Context_V2_t* pAynrCtx = (Aynr_Context_V2_t *)inparams->ctx;

    RkAiqAlgoPreAynrV2* pAnrPreParams = (RkAiqAlgoPreAynrV2*)inparams;

    oldGrayMode = pAynrCtx->isGrayMode;
    if (pAnrPreParams->com.u.proc.gray_mode) {
        pAynrCtx->isGrayMode = true;
    } else {
        pAynrCtx->isGrayMode = false;
    }

    if(oldGrayMode != pAynrCtx->isGrayMode) {
        pAynrCtx->isReCalculate |= 1;
    }

    Aynr_result_t ret = Aynr_PreProcess_V2(pAynrCtx);
    if(ret != AYNR_RET_SUCCESS) {
        result = XCAM_RETURN_ERROR_FAILED;
        LOGE_ANR("%s: ANRPreProcess failed (%d)\n", __FUNCTION__, ret);
    }

    LOGI_ANR("%s: (exit)\n", __FUNCTION__ );
    return result;
}

static XCamReturn
processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    int deltaIso = 0;
    LOGI_ANR("%s: (enter)\n", __FUNCTION__ );

#if 1
    RkAiqAlgoProcAynrV2* pAynrProcParams = (RkAiqAlgoProcAynrV2*)inparams;
    RkAiqAlgoProcResAynrV2* pAynrProcResParams = (RkAiqAlgoProcResAynrV2*)outparams;
    Aynr_Context_V2_t* pAynrCtx = (Aynr_Context_V2_t *)inparams->ctx;
    Aynr_ExpInfo_t stExpInfo;
    memset(&stExpInfo, 0x00, sizeof(Aynr_ExpInfo_t));

    LOGD_ANR("%s:%d init:%d hdr mode:%d  \n",
             __FUNCTION__, __LINE__,
             inparams->u.proc.init,
             pAynrProcParams->hdr_mode);

    stExpInfo.hdr_mode = 0; //pAnrProcParams->hdr_mode;
    for(int i = 0; i < 3; i++) {
        stExpInfo.arIso[i] = 50;
        stExpInfo.arAGain[i] = 1.0;
        stExpInfo.arDGain[i] = 1.0;
        stExpInfo.arTime[i] = 0.01;
    }

    if(pAynrProcParams->hdr_mode == RK_AIQ_WORKING_MODE_NORMAL) {
        stExpInfo.hdr_mode = 0;
    } else if(pAynrProcParams->hdr_mode == RK_AIQ_ISP_HDR_MODE_2_FRAME_HDR
              || pAynrProcParams->hdr_mode == RK_AIQ_ISP_HDR_MODE_2_LINE_HDR ) {
        stExpInfo.hdr_mode = 1;
    } else if(pAynrProcParams->hdr_mode == RK_AIQ_ISP_HDR_MODE_3_FRAME_HDR
              || pAynrProcParams->hdr_mode == RK_AIQ_ISP_HDR_MODE_3_LINE_HDR ) {
        stExpInfo.hdr_mode = 2;
    }
    stExpInfo.snr_mode = 0;

#if 0// TODO Merge:
    XCamVideoBuffer* xCamAePreRes = pAynrProcParams->com.u.proc.res_comb->ae_pre_res;
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

    RKAiqAecExpInfo_t *curExp = pAynrProcParams->com.u.proc.curExp;
    if(curExp != NULL) {
        stExpInfo.snr_mode = curExp->CISFeature.SNR;
        if(pAynrProcParams->hdr_mode == RK_AIQ_WORKING_MODE_NORMAL) {
            stExpInfo.hdr_mode = 0;
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

    deltaIso = abs(stExpInfo.arIso[stExpInfo.hdr_mode] - pAynrCtx->stExpInfo.arIso[stExpInfo.hdr_mode]);
    if(deltaIso > AYNRV2_RECALCULATE_DELTA_ISO) {
        pAynrCtx->isReCalculate |= 1;
    }

    if(pAynrCtx->isReCalculate) {
        Aynr_result_t ret = Aynr_Process_V2(pAynrCtx, &stExpInfo);
        if(ret != AYNR_RET_SUCCESS) {
            result = XCAM_RETURN_ERROR_FAILED;
            LOGE_ANR("%s: processing ANR failed (%d)\n", __FUNCTION__, ret);
        }

        Aynr_GetProcResult_V2(pAynrCtx, &pAynrProcResParams->stAynrProcResult);
        outparams->cfg_update = true;
        LOGD_ANR("recalculate: %d delta_iso:%d \n ", pAynrCtx->isReCalculate, deltaIso);
    } else {
        outparams->cfg_update = false;
    }
#endif

    pAynrCtx->isReCalculate = 0;

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

RkAiqAlgoDescription g_RkIspAlgoDescAynrV2 = {
    .common = {
        .version = RKISP_ALGO_AYNR_VERSION_V2,
        .vendor  = RKISP_ALGO_AYNR_VENDOR_V2,
        .description = RKISP_ALGO_AYNR_DESCRIPTION_V2,
        .type    = RK_AIQ_ALGO_TYPE_AYNR,
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
