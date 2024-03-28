/*
 * rk_aiq_algo_argbir_itf.cpp
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

#include "argbir/rk_aiq_algo_argbir_itf.h"

#include "xcam_log.h"
#if RKAIQ_HAVE_RGBIR_REMOSAIC_V10
#include "argbir/rk_aiq_argbir_algo_v10.h"
#endif
#include "argbir/rk_aiq_types_argbir_algo_prvt.h"

RKAIQ_BEGIN_DECLARE

void RgbirProcessGetExpo(aRgbirContext_t* pArgbirCtx, RkAiqAlgoProcArgbir* pArgbirParams) {
    LOG1_ARGBIR("%s:enter!\n", __FUNCTION__);

    // get eff expo data
    if (pArgbirCtx->FrameNumber == LINEAR_NUM) {
        pArgbirCtx->NextData.iso =
            pArgbirParams->com.u.proc.nxtExp->LinearExp.exp_real_params.analog_gain *
            pArgbirParams->com.u.proc.nxtExp->LinearExp.exp_real_params.digital_gain *
            pArgbirParams->com.u.proc.nxtExp->LinearExp.exp_real_params.isp_dgain * ISOMIN;
        if (pArgbirCtx->NextData.iso < FLT_EPSILON) {
            pArgbirCtx->NextData.iso =
                pArgbirParams->com.u.proc.curExp->LinearExp.exp_real_params.analog_gain *
                pArgbirParams->com.u.proc.curExp->LinearExp.exp_real_params.digital_gain *
                pArgbirParams->com.u.proc.curExp->LinearExp.exp_real_params.isp_dgain * ISOMIN;
        }
    } else if (pArgbirCtx->FrameNumber == HDR_2X_NUM) {
        pArgbirCtx->NextData.iso =
            pArgbirParams->com.u.proc.nxtExp->HdrExp[1].exp_real_params.analog_gain *
            pArgbirParams->com.u.proc.nxtExp->HdrExp[1].exp_real_params.digital_gain *
            pArgbirParams->com.u.proc.nxtExp->HdrExp[1].exp_real_params.isp_dgain * ISOMIN;
        if (pArgbirCtx->NextData.iso < FLT_EPSILON) {
            pArgbirCtx->NextData.iso =
                pArgbirParams->com.u.proc.curExp->HdrExp[1].exp_real_params.analog_gain *
                pArgbirParams->com.u.proc.curExp->HdrExp[1].exp_real_params.digital_gain *
                pArgbirParams->com.u.proc.curExp->HdrExp[1].exp_real_params.isp_dgain * ISOMIN;
        }
    } else if (pArgbirCtx->FrameNumber == HDR_3X_NUM) {
        pArgbirCtx->NextData.iso =
            pArgbirParams->com.u.proc.nxtExp->HdrExp[1].exp_real_params.analog_gain *
            pArgbirParams->com.u.proc.nxtExp->HdrExp[1].exp_real_params.digital_gain *
            pArgbirParams->com.u.proc.nxtExp->HdrExp[1].exp_real_params.isp_dgain * ISOMIN;
        if (pArgbirCtx->NextData.iso < FLT_EPSILON) {
            pArgbirCtx->NextData.iso =
                pArgbirParams->com.u.proc.curExp->HdrExp[1].exp_real_params.analog_gain *
                pArgbirParams->com.u.proc.curExp->HdrExp[1].exp_real_params.digital_gain *
                pArgbirParams->com.u.proc.curExp->HdrExp[1].exp_real_params.isp_dgain * ISOMIN;
        }
    }
    pArgbirCtx->NextData.iso = LIMIT_VALUE(pArgbirCtx->NextData.iso, ISOMAX, ISOMIN);

    LOG1_ARGBIR("%s:exit!\n", __FUNCTION__);
}

static XCamReturn create_context(RkAiqAlgoContext** context, const AlgoCtxInstanceCfg* cfg) {
    LOG1_ARGBIR(" %s:Enter!\n", __FUNCTION__);
    XCamReturn result           = XCAM_RETURN_NO_ERROR;
    aRgbirContext_t* pArgbirCtx = NULL;

    result = aRgbirInit(&pArgbirCtx, (CamCalibDbV2Context_t*)(cfg->calibv2));

    if (result != XCAM_RETURN_NO_ERROR) {
        LOGE_ARGBIR("%s ARGBIR Init failed: %d", __FUNCTION__, result);
        return (XCAM_RETURN_ERROR_FAILED);
    }
    *context = (RkAiqAlgoContext*)(pArgbirCtx);

    LOG1_ARGBIR(" %s:Exit!\n", __FUNCTION__);
    return result;
}

static XCamReturn destroy_context(RkAiqAlgoContext* context) {
    LOG1_ARGBIR("%s:Enter!\n", __FUNCTION__);
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    if (context != NULL) {
        aRgbirContext_t* pArgbirCtx = (aRgbirContext_t*)context;
        result                      = aRgbirRelease(pArgbirCtx);
        if (result != XCAM_RETURN_NO_ERROR) {
            LOGE_ARGBIR("%s ARGBIR Release failed: %d", __FUNCTION__, result);
            return (XCAM_RETURN_ERROR_FAILED);
        }
        context = NULL;
    }

    LOG1_ARGBIR("%s:Exit!\n", __FUNCTION__);
    return result;
}

static XCamReturn prepare(RkAiqAlgoCom* params) {
    LOG1_ARGBIR("%s:Enter!\n", __FUNCTION__);
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    aRgbirContext_t* pArgbirCtx       = (aRgbirContext_t*)params->ctx;
    RkAiqAlgoConfigAdrc* AdrcCfgParam = (RkAiqAlgoConfigAdrc*)params;  // come from params in html
    const CamCalibDbV2Context_t* pCalibDb = AdrcCfgParam->com.u.prepare.calibv2;

    if (AdrcCfgParam->working_mode < RK_AIQ_WORKING_MODE_ISP_HDR2)
        pArgbirCtx->FrameNumber = LINEAR_NUM;
    else if (AdrcCfgParam->working_mode < RK_AIQ_WORKING_MODE_ISP_HDR3 &&
             AdrcCfgParam->working_mode >= RK_AIQ_WORKING_MODE_ISP_HDR2)
        pArgbirCtx->FrameNumber = HDR_2X_NUM;
    else
        pArgbirCtx->FrameNumber = HDR_3X_NUM;

    if (!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB)) {
        LOGV_ARGBIR("%s: ARGBIR Reload Para!\n", __FUNCTION__);
        // just update calib ptr
        if (params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB_PTR) {
            return XCAM_RETURN_NO_ERROR;
        }
#if RKAIQ_HAVE_RGBIR_REMOSAIC_V10
        CalibDbV2_argbir_v10_t* calibv2_argbir_calib =
            (CalibDbV2_argbir_v10_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCalibDb, argbir_calib));
        memcpy(&pArgbirCtx->RgbirAttrV10.stAuto, calibv2_argbir_calib,
               sizeof(CalibDbV2_argbir_v10_t));
#endif
        pArgbirCtx->ifReCalcStAuto = true;
    } else if (params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_CHANGERES) {
        pArgbirCtx->isCapture = true;
    }

    if (/* !params->u.prepare.reconfig*/ true) {
        aRgbirStop(pArgbirCtx);  // stop firstly for re-preapre
        result = aRgbirStart(pArgbirCtx);
        if (result != XCAM_RETURN_NO_ERROR) {
            LOGE_ARGBIR("%s ARGBIR Start failed: %d", __FUNCTION__, result);
            return (XCAM_RETURN_ERROR_FAILED);
        }
    }

    LOG1_ARGBIR("%s:Exit!\n", __FUNCTION__);
    return result;
}

static XCamReturn processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams) {
    LOG1_ARGBIR("%s:Enter!\n", __FUNCTION__);
    XCamReturn result                      = XCAM_RETURN_NO_ERROR;
    bool bypass_tuning_params              = true;
    aRgbirContext_t* pArgbirCtx            = (aRgbirContext_t*)inparams->ctx;
    RkAiqAlgoProcArgbir* pArgbirParams     = (RkAiqAlgoProcArgbir*)inparams;
    RkAiqAlgoProcResArgbir* pArgbirProcRes = (RkAiqAlgoProcResArgbir*)outparams;
    pArgbirCtx->FrameID                    = inparams->frame_id;

    LOGI_ARGBIR(
        "%s:////////////////////////////////////////////ARGBIR "
        "Start//////////////////////////////////////////// \n",
        __func__);

    if (pArgbirCtx->isCapture) {
        LOGV_ARGBIR("%s: It's capturing, using pre frame params\n", __func__);
        pArgbirCtx->isCapture = false;
    } else {
        if (RgbirEnableSetting(pArgbirCtx, pArgbirProcRes->ArgbirProcRes)) {
            RgbirProcessGetExpo(pArgbirCtx, pArgbirParams);

            // get bypass_tuning_params
            bypass_tuning_params = ArgbirByPassTuningProcessing(pArgbirCtx);

            // get tuning paras
            if (!bypass_tuning_params)
                ArgbirTuningParaProcessing(pArgbirCtx, pArgbirProcRes->ArgbirProcRes);
        } else {
            LOGI_ARGBIR("%s: RGBIR Enable is OFF, Bypass RGBIR !!! \n", __func__);
        }
    }
    LOGI_ARGBIR(
        "%s:////////////////////////////////////////////ARGBIR "
        "Over//////////////////////////////////////////// \n",
        __func__);

    // output ProcRes
    outparams->cfg_update =
        !bypass_tuning_params || pArgbirCtx->ifReCalcStAuto || pArgbirCtx->ifReCalcStManual;
    if (pArgbirCtx->ifReCalcStAuto) pArgbirCtx->ifReCalcStAuto = false;
    if (pArgbirCtx->ifReCalcStManual) pArgbirCtx->ifReCalcStManual = false;

    LOG1_ARGBIR("%s:Exit!\n", __FUNCTION__);
    return XCAM_RETURN_NO_ERROR;
}

RkAiqAlgoDescription g_RkIspAlgoDescARGBIR = {
    .common =
        {
            .version         = RKISP_ALGO_ARGBIR_VERSION,
            .vendor          = RKISP_ALGO_ARGBIR_VENDOR,
            .description     = RKISP_ALGO_ARGBIR_DESCRIPTION,
            .type            = RK_AIQ_ALGO_TYPE_ARGBIR,
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
