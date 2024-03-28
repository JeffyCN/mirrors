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

#include "acnrV30/rk_aiq_acnr_algo_itf_v30.h"
#include "acnrV30/rk_aiq_acnr_algo_v30.h"
#include "rk_aiq_algo_types.h"

RKAIQ_BEGIN_DECLARE

typedef struct _RkAiqAlgoContext {
    Acnr_Context_V30_t AcnrCtx;
} RkAiqAlgoContext;


static XCamReturn
create_context(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{

    XCamReturn result = XCAM_RETURN_NO_ERROR;
    LOGI_ANR("%s: (enter)\n", __FUNCTION__ );

#if 1
    Acnr_Context_V30_t* pAcnrCtx = NULL;
#if ACNR_USE_JSON_FILE_V30
    AcnrV30_result_t ret = Acnr_Init_V30(&pAcnrCtx, cfg->calibv2);
#endif

    if(ret != ACNRV30_RET_SUCCESS) {
        result = XCAM_RETURN_ERROR_FAILED;
        LOGE_ANR("%s: Initializaion ANR failed (%d)\n", __FUNCTION__, ret);
    } else {
        *context = (RkAiqAlgoContext *)(pAcnrCtx);
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
    Acnr_Context_V30_t* pAcnrCtx = (Acnr_Context_V30_t*)context;
    AcnrV30_result_t ret = Acnr_Release_V30(pAcnrCtx);
    if(ret != ACNRV30_RET_SUCCESS) {
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

    Acnr_Context_V30_t* pAcnrCtx = (Acnr_Context_V30_t *)params->ctx;
    RkAiqAlgoConfigAcnrV30* pCfgParam = (RkAiqAlgoConfigAcnrV30*)params;
    pAcnrCtx->prepare_type = params->u.prepare.conf_type;

    if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )) {
#if ACNR_USE_JSON_FILE_V30
        void *pCalibDbV2 = (void*)(pCfgParam->com.u.prepare.calibv2);
        CalibDbV2_CNRV30_t *cnr_v30 =
            (CalibDbV2_CNRV30_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCalibDbV2, cnr_v30));
        pAcnrCtx->cnr_v30 = *cnr_v30;
#endif
        // just update calib ptr
        if (params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB_PTR)
            return XCAM_RETURN_NO_ERROR;
        pAcnrCtx->isIQParaUpdate = true;
        pAcnrCtx->isReCalculate |= 1;

    }

    AcnrV30_result_t ret = Acnr_Prepare_V30(pAcnrCtx, &pCfgParam->stAcnrConfig);
    if(ret != ACNRV30_RET_SUCCESS) {
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
    Acnr_Context_V30_t* pAcnrCtx = (Acnr_Context_V30_t *)inparams->ctx;

    RkAiqAlgoPreAcnrV30* pAnrPreParams = (RkAiqAlgoPreAcnrV30*)inparams;

    oldGrayMode = pAcnrCtx->isGrayMode;
    if (pAnrPreParams->com.u.proc.gray_mode) {
        pAcnrCtx->isGrayMode = true;
    } else {
        pAcnrCtx->isGrayMode = false;
    }

    if(oldGrayMode != pAcnrCtx->isGrayMode) {
        pAcnrCtx->isReCalculate |= 1;
    }

    AcnrV30_result_t ret = Acnr_PreProcess_V30(pAcnrCtx);
    if(ret != ACNRV30_RET_SUCCESS) {
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
    int DeltaISO = 0;
    LOGI_ANR("%s: (enter)\n", __FUNCTION__ );

#if 1
    RkAiqAlgoProcAcnrV30* pAcnrProcParams = (RkAiqAlgoProcAcnrV30*)inparams;
    RkAiqAlgoProcResAcnrV30* pAcnrProcResParams = (RkAiqAlgoProcResAcnrV30*)outparams;
    Acnr_Context_V30_t* pAcnrCtx = (Acnr_Context_V30_t *)inparams->ctx;
    AcnrV30_ExpInfo_t stExpInfo;
    memset(&stExpInfo, 0x00, sizeof(AcnrV30_ExpInfo_t));

    bool oldGrayMode = false;
    oldGrayMode = pAcnrCtx->isGrayMode;
    if (inparams->u.proc.gray_mode) {
        pAcnrCtx->isGrayMode = true;
    } else {
        pAcnrCtx->isGrayMode = false;
    }

    if(oldGrayMode != pAcnrCtx->isGrayMode) {
        pAcnrCtx->isReCalculate |= 1;
    }

    AcnrV30_result_t ret = Acnr_PreProcess_V30(pAcnrCtx);
    if(ret != ACNRV30_RET_SUCCESS) {
        result = XCAM_RETURN_ERROR_FAILED;
        LOGE_ANR("%s: ANRPreProcess failed (%d)\n", __FUNCTION__, ret);
    }

    if (pAcnrProcParams == NULL) {
        LOGD_ANR("%s:%d pointer pAcnrProcParams is NULL, return bypass", __FUNCTION__, __LINE__);
        return XCAM_RETURN_BYPASS;
    }

    LOGD_ANR("%s:%d init:%d hdr mode:%d  \n",
             __FUNCTION__, __LINE__,
             inparams->u.proc.init,
             pAcnrProcParams->hdr_mode);

    stExpInfo.hdr_mode = 0; //pAnrProcParams->hdr_mode;
    for(int i = 0; i < 3; i++) {
        stExpInfo.arIso[i] = 50;
        stExpInfo.arAGain[i] = 1.0;
        stExpInfo.arDGain[i] = 1.0;
        stExpInfo.arTime[i] = 0.01;
    }

    if(pAcnrProcParams->hdr_mode == RK_AIQ_WORKING_MODE_NORMAL) {
        stExpInfo.hdr_mode = 0;
    } else if(pAcnrProcParams->hdr_mode == RK_AIQ_ISP_HDR_MODE_2_FRAME_HDR
              || pAcnrProcParams->hdr_mode == RK_AIQ_ISP_HDR_MODE_2_LINE_HDR ) {
        stExpInfo.hdr_mode = 1;
    } else if(pAcnrProcParams->hdr_mode == RK_AIQ_ISP_HDR_MODE_3_FRAME_HDR
              || pAcnrProcParams->hdr_mode == RK_AIQ_ISP_HDR_MODE_3_LINE_HDR ) {
        stExpInfo.hdr_mode = 2;
    }
    stExpInfo.snr_mode = 0;

    stExpInfo.blc_ob_predgain = 1.0;
    if(pAcnrProcParams != NULL) {
        stExpInfo.blc_ob_predgain = pAcnrProcParams->stAblcV32_proc_res->isp_ob_predgain;
        if(stExpInfo.blc_ob_predgain != pAcnrCtx->stExpInfo.blc_ob_predgain) {
            pAcnrCtx->isReCalculate |= 1;
        }
    }

#if 0// TODO Merge:
    XCamVideoBuffer* xCamAePreRes = pAcnrProcParams->com.u.proc.res_comb->ae_pre_res;
    RkAiqAlgoPreResAe* pAEPreRes = nullptr;
    if (xCamAePreRes) {
        // xCamAePreRes->ref(xCamAePreRes);
        pAEPreRes = (RkAiqAlgoPreResAe*)xCamAePreRes->map(xCamAePreRes);
        if (!pAEPreRes) {
            LOGE_ANR("ae pre result is null");
        } else {
        }
        // xCamAePreRes->unref(xCamAePreRes);
    }
#endif


    RKAiqAecExpInfo_t *curExp = pAcnrProcParams->com.u.proc.curExp;
    if(curExp != NULL) {
        stExpInfo.snr_mode = curExp->CISFeature.SNR;
        if(pAcnrProcParams->hdr_mode == RK_AIQ_WORKING_MODE_NORMAL) {
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
            if(stExpInfo.blc_ob_predgain  < 1.0) {
                stExpInfo.blc_ob_predgain  = 1.0;
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

    if(anr_cnt % 1 == 0) {
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

    DeltaISO = abs(stExpInfo.arIso[stExpInfo.hdr_mode] - pAcnrCtx->stExpInfo.arIso[pAcnrCtx->stExpInfo.hdr_mode]);
    if(DeltaISO > ACNRV30_RECALCULATE_DELTA_ISO) {
        pAcnrCtx->isReCalculate |= 1;
    }

    if(pAcnrCtx->isReCalculate) {
        AcnrV30_result_t ret = Acnr_Process_V30(pAcnrCtx, &stExpInfo);
        if(ret != ACNRV30_RET_SUCCESS) {
            result = XCAM_RETURN_ERROR_FAILED;
            LOGE_ANR("%s: processing ANR failed (%d)\n", __FUNCTION__, ret);
        }

        Acnr_GetProcResult_V30(pAcnrCtx, &pAcnrProcResParams->stAcnrProcResult);
        outparams->cfg_update = true;
        LOGD_ANR("recalculate: %d delta_iso:%d \n ", pAcnrCtx->isReCalculate, DeltaISO);
    } else {
        outparams->cfg_update = false;
    }
#endif

    pAcnrCtx->isReCalculate = 0;
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

RkAiqAlgoDescription g_RkIspAlgoDescAcnrV30 = {
    .common = {
        .version = RKISP_ALGO_ACNR_VERSION_V30,
        .vendor  = RKISP_ALGO_ACNR_VENDOR_V30,
        .description = RKISP_ALGO_ACNR_DESCRIPTION_V30,
        .type    = RK_AIQ_ALGO_TYPE_ACNR,
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
