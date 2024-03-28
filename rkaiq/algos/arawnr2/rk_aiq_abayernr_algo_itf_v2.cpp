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

#include "arawnr2/rk_aiq_abayernr_algo_itf_v2.h"
#include "arawnr2/rk_aiq_abayernr_algo_v2.h"
#include "rk_aiq_algo_types.h"
#include "RkAiqCalibDbV2Helper.h"

RKAIQ_BEGIN_DECLARE

typedef struct _RkAiqAlgoContext {
    Abayernr_Context_V2_t AbayernrCtx;
} RkAiqAlgoContext;


static XCamReturn
create_context(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{

    XCamReturn result = XCAM_RETURN_NO_ERROR;
    LOGI_ANR("%s: (enter)\n", __FUNCTION__ );

#if 1
    Abayernr_Context_V2_t* pAbayernrCtx = NULL;
#if (ABAYERNR_USE_JSON_FILE_V2)
    Abayernr_result_t ret = Abayernr_Init_V2(&pAbayernrCtx, cfg->calibv2);
#else
    Abayernr_result_t ret = Abayernr_Init_V2(&pAbayernrCtx, cfg->calib);
#endif
    if(ret != ABAYERNR_RET_SUCCESS) {
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
    Abayernr_Context_V2_t* pAbayernrCtx = (Abayernr_Context_V2_t*)context;
    Abayernr_result_t ret = Abayernr_Release_V2(pAbayernrCtx);
    if(ret != ABAYERNR_RET_SUCCESS) {
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

    Abayernr_Context_V2_t* pAbayernrCtx = (Abayernr_Context_V2_t *)params->ctx;
    RkAiqAlgoConfigArawnrV2* pCfgParam = (RkAiqAlgoConfigArawnrV2*)params;
    pAbayernrCtx->prepare_type = params->u.prepare.conf_type;

    if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )) {
#if ABAYERNR_USE_JSON_FILE_V2
        // just update calib ptr
        if (params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB_PTR) {
            return XCAM_RETURN_NO_ERROR;
        }
        void *pCalibDbV2 = (void*)(pCfgParam->com.u.prepare.calibv2);
        CalibDbV2_BayerNrV2_t *bayernr_v2 = (CalibDbV2_BayerNrV2_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCalibDbV2, bayernr_v2));
        pAbayernrCtx->bayernr_v2 = *bayernr_v2;
#else
        void *pCalibDb = (void*)(pCfgParam->com.u.prepare.calib);
        pAbayernrCtx->list_bayernr_v2 =
            (struct list_head*)(CALIBDB_GET_MODULE_PTR((void*)pCalibDb, list_bayernr_v2));
        // just update calib ptr
        if (params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB_PTR) {
            return XCAM_RETURN_NO_ERROR;
        }
#endif
        pAbayernrCtx->isIQParaUpdate = true;
        pAbayernrCtx->isReCalculate |= 1;
    }

    Abayernr_result_t ret = Abayernr_Prepare_V2(pAbayernrCtx, &pCfgParam->stArawnrConfig);
    if(ret != ABAYERNR_RET_SUCCESS) {
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
    Abayernr_Context_V2_t* pAbayernrCtx = (Abayernr_Context_V2_t *)inparams->ctx;

    RkAiqAlgoPreArawnrV2* pAbayernrPreParams = (RkAiqAlgoPreArawnrV2*)inparams;

    oldGrayMode = pAbayernrCtx->isGrayMode;
    if (pAbayernrPreParams->com.u.proc.gray_mode) {
        pAbayernrCtx->isGrayMode = true;
    } else {
        pAbayernrCtx->isGrayMode = false;
    }

    if(oldGrayMode != pAbayernrCtx->isGrayMode) {
        pAbayernrCtx->isReCalculate |= 1;
    }

    Abayernr_result_t ret = Abayernr_PreProcess_V2(pAbayernrCtx);
    if(ret != ABAYERNR_RET_SUCCESS) {
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
    int delta_iso = 0;

    LOGI_ANR("%s: (enter)\n", __FUNCTION__ );

#if 1
    RkAiqAlgoProcArawnrV2* pAbayernrProcParams = (RkAiqAlgoProcArawnrV2*)inparams;
    RkAiqAlgoProcResArawnrV2* pAbayernrProcResParams = (RkAiqAlgoProcResArawnrV2*)outparams;
    Abayernr_Context_V2_t* pAbayernrCtx = (Abayernr_Context_V2_t *)inparams->ctx;
    Abayernr_ExpInfo_t stExpInfo;
    memset(&stExpInfo, 0x00, sizeof(Abayernr_ExpInfo_t));

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
    delta_iso = abs(stExpInfo.arIso[stExpInfo.hdr_mode] - pAbayernrCtx->stExpInfo.arIso[stExpInfo.hdr_mode]);
    if(delta_iso > ABAYERNRV2_DELTA_ISO) {
        pAbayernrCtx->isReCalculate |= 1;
    }

    if(pAbayernrCtx->isReCalculate) {
        Abayernr_result_t ret = Abayernr_Process_V2(pAbayernrCtx, &stExpInfo);
        if(ret != ABAYERNR_RET_SUCCESS) {
            result = XCAM_RETURN_ERROR_FAILED;
            LOGE_ANR("%s: processing ANR failed (%d)\n", __FUNCTION__, ret);
        }

        Abayernr_GetProcResult_V2(pAbayernrCtx, &pAbayernrProcResParams->stArawnrProcResult);

        LOGD_ANR("recalculate: %d delta_iso:%d \n ", pAbayernrCtx->isReCalculate, delta_iso);

        outparams->cfg_update = true;
    } else {
        outparams->cfg_update = false;
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

RkAiqAlgoDescription g_RkIspAlgoDescArawnrV2 = {
    .common = {
        .version = RKISP_ALGO_ABAYERNR_VERSION_V2,
        .vendor  = RKISP_ALGO_ABAYERNR_VENDOR_V2,
        .description = RKISP_ALGO_ABAYERNR_DESCRIPTION_V2,
        .type    = RK_AIQ_ALGO_TYPE_ARAWNR,
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
