/*
 * rk_aiq_algo_camgroup_argbir_itf.cpp
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

#include "algos/argbir/rk_aiq_algo_argbir_itf.h"
#include "rk_aiq_algo_camgroup_types.h"
#include "xcam_log.h"
#if RKAIQ_HAVE_RGBIR_REMOSAIC_V10
#include "argbir/rk_aiq_argbir_algo_v10.h"
#endif

RKAIQ_BEGIN_DECLARE

void RgbirProcessGetExpoGroup(aRgbirContext_t* pArgbirGrpCtx,
                              RkAiqAlgoCamGroupProcIn* pAdrcGrpParams) {
    LOG1_ARGBIR("%s:enter!\n", __FUNCTION__);

    // get eff expo
    if (pArgbirGrpCtx->FrameNumber == LINEAR_NUM) {
        pArgbirGrpCtx->NextData.iso =
            pAdrcGrpParams->camgroupParmasArray[0]
                ->aec._effAecExpInfo.LinearExp.exp_real_params.analog_gain *
            pAdrcGrpParams->camgroupParmasArray[0]
                ->aec._effAecExpInfo.LinearExp.exp_real_params.digital_gain *
            pAdrcGrpParams->camgroupParmasArray[0]
                ->aec._effAecExpInfo.LinearExp.exp_real_params.isp_dgain *
            ISOMIN;
    } else if (pArgbirGrpCtx->FrameNumber == HDR_2X_NUM) {
        pArgbirGrpCtx->NextData.iso = pAdrcGrpParams->camgroupParmasArray[0]
                                          ->aec._effAecExpInfo.HdrExp[1]
                                          .exp_real_params.analog_gain *
                                      pAdrcGrpParams->camgroupParmasArray[0]
                                          ->aec._effAecExpInfo.HdrExp[1]
                                          .exp_real_params.digital_gain *
                                      pAdrcGrpParams->camgroupParmasArray[0]
                                          ->aec._effAecExpInfo.HdrExp[1]
                                          .exp_real_params.isp_dgain *
                                      ISOMIN;
    } else if (pArgbirGrpCtx->FrameNumber == HDR_3X_NUM) {
        pArgbirGrpCtx->NextData.iso = pAdrcGrpParams->camgroupParmasArray[0]
                                          ->aec._effAecExpInfo.HdrExp[1]
                                          .exp_real_params.analog_gain *
                                      pAdrcGrpParams->camgroupParmasArray[0]
                                          ->aec._effAecExpInfo.HdrExp[1]
                                          .exp_real_params.digital_gain *
                                      pAdrcGrpParams->camgroupParmasArray[0]
                                          ->aec._effAecExpInfo.HdrExp[1]
                                          .exp_real_params.isp_dgain *
                                      ISOMIN;
    }
    pArgbirGrpCtx->NextData.iso = LIMIT_VALUE(pArgbirGrpCtx->NextData.iso, ISOMAX, ISOMIN);

    LOG1_ARGBIR("%s:exit!\n", __FUNCTION__);
}

static XCamReturn create_context(RkAiqAlgoContext** context, const AlgoCtxInstanceCfg* cfg) {
    LOG1_ARGBIR(" %s:Enter!\n", __FUNCTION__);
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    AlgoCtxInstanceCfgCamGroup* instanc_int = (AlgoCtxInstanceCfgCamGroup*)cfg;
    aRgbirContext_t* pArgbirGrpCtx          = NULL;

    result = aRgbirInit(&pArgbirGrpCtx, (CamCalibDbV2Context_t*)(instanc_int->s_calibv2));

    if (result != XCAM_RETURN_NO_ERROR) {
        LOGE_ARGBIR("%s ARGBIR Init failed: %d", __FUNCTION__, result);
        return (XCAM_RETURN_ERROR_FAILED);
    }
    *context = (RkAiqAlgoContext*)(pArgbirGrpCtx);

    LOG1_ARGBIR(" %s:Exit!\n", __FUNCTION__);
    return result;
}

static XCamReturn destroy_context(RkAiqAlgoContext* context) {
    LOG1_ARGBIR("%s:Enter!\n", __FUNCTION__);
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    if (context != NULL) {
        aRgbirContext_t* pArgbirGrpCtx = (aRgbirContext_t*)context;
        result                         = aRgbirRelease(pArgbirGrpCtx);
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

    aRgbirContext_t* pArgbirGrpCtx = (aRgbirContext_t*)params->ctx;
    RkAiqAlgoCamGroupPrepare* AdrcCfgParam =
        (RkAiqAlgoCamGroupPrepare*)params;  // come from params in html
    const CamCalibDbV2Context_t* pCalibDb = AdrcCfgParam->s_calibv2;

    if (AdrcCfgParam->gcom.com.u.prepare.working_mode < RK_AIQ_WORKING_MODE_ISP_HDR2)
        pArgbirGrpCtx->FrameNumber = LINEAR_NUM;
    else if (AdrcCfgParam->gcom.com.u.prepare.working_mode < RK_AIQ_WORKING_MODE_ISP_HDR3 &&
             AdrcCfgParam->gcom.com.u.prepare.working_mode >= RK_AIQ_WORKING_MODE_ISP_HDR2)
        pArgbirGrpCtx->FrameNumber = HDR_2X_NUM;
    else
        pArgbirGrpCtx->FrameNumber = HDR_3X_NUM;

    if (!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB)) {
        LOGV_ARGBIR("%s: ARGBIR Reload Para!\n", __FUNCTION__);
#if RKAIQ_HAVE_RGBIR_REMOSAIC_V10
        CalibDbV2_argbir_v10_t* calibv2_argbir_calib =
            (CalibDbV2_argbir_v10_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCalibDb, argbir_calib));
        memcpy(&pArgbirGrpCtx->RgbirAttrV10.stAuto, calibv2_argbir_calib,
               sizeof(CalibDbV2_argbir_v10_t));  // reload stAuto
#endif
        pArgbirGrpCtx->ifReCalcStAuto = true;
    } else if (params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_CHANGERES) {
        pArgbirGrpCtx->isCapture = true;
    }

    if (/* !params->u.prepare.reconfig*/ true) {
        aRgbirStop(pArgbirGrpCtx);  // stop firstly for re-preapre
        result = aRgbirStart(pArgbirGrpCtx);
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
    XCamReturn result                         = XCAM_RETURN_NO_ERROR;
    bool bypass_tuning_params                 = true;
    aRgbirContext_t* pArgbirGrpCtx            = (aRgbirContext_t*)inparams->ctx;
    pArgbirGrpCtx->FrameID                    = inparams->frame_id;
    RkAiqAlgoCamGroupProcIn* pAdrcGrpParams   = (RkAiqAlgoCamGroupProcIn*)inparams;
    RkAiqAlgoCamGroupProcOut* pAdrcGrpProcRes = (RkAiqAlgoCamGroupProcOut*)outparams;

    LOGI_ARGBIR(
        "%s:////////////////////////////////////////////ARGBIR Group "
        "Start//////////////////////////////////////////// \n",
        __func__);

    if (pArgbirGrpCtx->isCapture) {
        LOGV_ARGBIR("%s: It's capturing, using pre frame params\n", __func__);
        pArgbirGrpCtx->isCapture = false;
    } else {
        if (RgbirEnableSetting(pArgbirGrpCtx,
                               pAdrcGrpProcRes->camgroupParmasArray[0]->_aRgbirConfig)) {
            RgbirProcessGetExpoGroup(pArgbirGrpCtx, pAdrcGrpParams);

            // get bypass_tuning_params
            bypass_tuning_params = ArgbirByPassTuningProcessing(pArgbirGrpCtx);

            // get tuning paras
            if (!bypass_tuning_params)
                ArgbirTuningParaProcessing(pArgbirGrpCtx,
                                         pAdrcGrpProcRes->camgroupParmasArray[0]->_aRgbirConfig);
        } else {
            LOGI_ARGBIR("%s: Group RGBIR Enable is OFF, Bypass Drc !!! \n", __func__);
        }
    }
    LOGI_ARGBIR(
        "%s:////////////////////////////////////////////ARGBIR Group "
        "Over//////////////////////////////////////////// \n",
        __func__);

    IS_UPDATE_MEM((pAdrcGrpProcRes->camgroupParmasArray[0]->_aRgbirConfig), pAdrcGrpParams->_offset_is_update) =
        !bypass_tuning_params || pArgbirGrpCtx->ifReCalcStAuto ||
        pArgbirGrpCtx->ifReCalcStManual;
    // output ProcRes
    for (int i = 1; i < pAdrcGrpProcRes->arraySize; i++) {
        IS_UPDATE_MEM((pAdrcGrpProcRes->camgroupParmasArray[i]->_aRgbirConfig), pAdrcGrpParams->_offset_is_update) =
            !bypass_tuning_params || pArgbirGrpCtx->ifReCalcStAuto ||
            pArgbirGrpCtx->ifReCalcStManual;
        pAdrcGrpProcRes->camgroupParmasArray[i]->_aRgbirConfig->bRgbirEn =
            pAdrcGrpProcRes->camgroupParmasArray[0]->_aRgbirConfig->bRgbirEn;
        pAdrcGrpProcRes->camgroupParmasArray[i]->_aRgbirConfig->RgbirProcRes =
            pAdrcGrpProcRes->camgroupParmasArray[0]->_aRgbirConfig->RgbirProcRes;
    }
    // update curr data 2 api info
    if (pArgbirGrpCtx->ifReCalcStAuto) pArgbirGrpCtx->ifReCalcStAuto = false;
    if (pArgbirGrpCtx->ifReCalcStManual) pArgbirGrpCtx->ifReCalcStManual = false;

    LOG1_ARGBIR("%s:Exit!\n", __FUNCTION__);
    return XCAM_RETURN_NO_ERROR;
}

RkAiqAlgoDescription g_RkIspAlgoDescCamgroupArgbir = {
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
