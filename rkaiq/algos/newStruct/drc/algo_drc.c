/*
 * algo_drc.c
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

#include <math.h>

#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"
#include "algo_types_priv.h"
#include "c_base/aiq_base.h"
#include "drc_types_prvt.h"
#include "interpolation.h"
#include "xcam_log.h"

#ifndef LIMIT_VALUE
#define LIMIT_VALUE(value,max_value,min_value)      (value > max_value? max_value : value < min_value ? min_value : value)
#endif

#ifndef LIMIT_VALUE_UNSIGNED
#define LIMIT_VALUE_UNSIGNED(value, max_value) (value > max_value ? max_value : value)
#endif
#define MANUALCURVEMAX (8192)
#define POW_E_NEG6     (0.0025f)

// RKAIQ_BEGIN_DECLARE
static XCamReturn create_context(RkAiqAlgoContext** context, const AlgoCtxInstanceCfg* cfg) {
    XCamReturn result                 = XCAM_RETURN_NO_ERROR;
    CamCalibDbV2Context_t* pCalibDbV2 = cfg->calibv2;

    DrcContext_t* ctx = aiq_mallocz(sizeof(DrcContext_t));
    if (ctx == NULL) {
        LOGE_ATMO("%s: create Drc context fail!\n", __FUNCTION__);
        return XCAM_RETURN_ERROR_MEM;
    }

    ctx->isCapture                  = false;
    ctx->isDampStable               = true;
    ctx->isReCal_                   = true;
    ctx->prepare_params             = NULL;
    ctx->strg.darkAreaBoostEn       = false;
    ctx->strg.darkAreaBoostStrength = 50;
    ctx->strg.hdrStrengthEn         = false;
    ctx->strg.hdrStrength           = 50;
    ctx->drc_attrib     = (drc_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibDbV2, drc));

    *context = (RkAiqAlgoContext*)ctx;
    LOGV_ATMO("%s: (exit)\n", __FUNCTION__);
    return result;
}

static XCamReturn destroy_context(RkAiqAlgoContext* context) {
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    DrcContext_t* pDrcCtx = (DrcContext_t*)context;
    aiq_free(pDrcCtx);
    return result;
}

static XCamReturn prepare(RkAiqAlgoCom* params) {
    XCamReturn result               = XCAM_RETURN_NO_ERROR;
    DrcContext_t* pDrcCtx           = (DrcContext_t*)params->ctx;
    RkAiqAlgoConfigDrc* DrcCfgParam = (RkAiqAlgoConfigDrc*)params;

    if (!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB)) {
        // just update calib ptr
        if (params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB_PTR) {
            pDrcCtx->drc_attrib =
                (drc_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(params->u.prepare.calibv2, drc));
            pDrcCtx->iso_list = params->u.prepare.calibv2->sensor_info->iso_list;
            return XCAM_RETURN_NO_ERROR;
        }
    } else if (params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_CHANGERES) {
        pDrcCtx->isCapture = true;
    }

    pDrcCtx->drc_attrib =
        (drc_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(params->u.prepare.calibv2, drc));
    pDrcCtx->iso_list = params->u.prepare.calibv2->sensor_info->iso_list;
    pDrcCtx->prepare_params = &params->u.prepare;
    pDrcCtx->isReCal_       = true;

    return result;
}

static XCamReturn processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams) {
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    DrcContext_t* pDrcCtx            = (DrcContext_t*)inparams->ctx;
    pDrcCtx->FrameID                 = inparams->frame_id;
    drc_api_attrib_t* drc_attrib     = pDrcCtx->drc_attrib;
    RkAiqAlgoProcDrc* drc_proc_param = (RkAiqAlgoProcDrc*)inparams;
    drc_param_t* drcRes              = outparams->algoRes;
    if (drc_proc_param->LongFrmMode != pDrcCtx->NextData.AEData.LongFrmMode) {
        pDrcCtx->isReCal_ = true;
    }
    pDrcCtx->NextData                = drc_proc_param->NextData;
    pDrcCtx->drc_stats               = &drc_proc_param->drc_stats;

    LOGV_ATMO("%s: (enter)\n", __FUNCTION__);

    if (!drc_attrib) {
        LOGE_ATMO("drc_attrib is NULL !");
        return XCAM_RETURN_ERROR_MEM;
    }

    if (drc_attrib->opMode != RK_AIQ_OP_MODE_AUTO) {
        LOGE_ATMO("mode is %d, not auto mode, ignore", drc_attrib->opMode);
        return XCAM_RETURN_NO_ERROR;
    }

    if (inparams->u.proc.is_attrib_update) {
        pDrcCtx->isReCal_ = true;
    }

    if (pDrcCtx->isCapture) {
        LOGD_ATMO("%s: It's capturing, using pre frame params\n", __func__);
        pDrcCtx->isCapture = false;
        return XCAM_RETURN_NO_ERROR;
    }

    if (drc_proc_param->FrameNumber == HDR_2X_NUM || drc_proc_param->FrameNumber == HDR_3X_NUM ||
        drc_proc_param->FrameNumber == SENSOR_MGE)
        outparams->en = true;
    else if (drc_proc_param->FrameNumber == LINEAR_NUM) {
        if (drc_attrib->opMode == RK_AIQ_OP_MODE_AUTO) {
            outparams->en = drc_attrib->en;
        } else {
            outparams->en = false;
        }
    }

    int iso         = inparams->u.proc.iso;
    bool init       = inparams->u.proc.init;
    float delta_iso = (float)abs(iso - pDrcCtx->iso);
    delta_iso /= (float)pDrcCtx->iso;

    outparams->cfg_update = false;

#if 0
    if (inparams->u.proc.is_bw_sensor) {
        drc_attrib->en        = false;
        outparams->cfg_update = init ? true : false;
        return XCAM_RETURN_NO_ERROR;
    }
#endif
    if (delta_iso > DEFAULT_RECALCULATE_DELTA_ISO || !drc_proc_param->aeIsConverged) {
        pDrcCtx->isReCal_ = true;
    }

#if defined(RKAIQ_HAVE_DRC_V20) || defined(RKAIQ_HAVE_DRC_V21)
    bool isIIRReclac = false;
    if (pDrcCtx->CurrData.autoCurveIIRParams.sw_drcT_drcCurve_mode == adrc_auto_mode) {
        if (drc_proc_param->aeIsConverged) {
            pDrcCtx->CurrData.autoCurveIIRParams.reCalcNum++;
            if (pDrcCtx->CurrData.autoCurveIIRParams.reCalcNum <=
                2 * pDrcCtx->CurrData.autoCurveIIRParams.sw_drcT_iirFrm_maxLimit)
                isIIRReclac = true;
        } else {
            pDrcCtx->CurrData.autoCurveIIRParams.reCalcNum = 0;
        }
    } else {
        pDrcCtx->CurrData.autoCurveIIRParams.reCalcNum = 0;
    }
    pDrcCtx->isReCal_ = pDrcCtx->isReCal_ || isIIRReclac;
#endif

    if (pDrcCtx->isReCal_) {
#if RKAIQ_HAVE_DRC_V12
        DrcSelectParam(pDrcCtx, outparams->algoRes, iso);
        DrcExpoParaProcessing(pDrcCtx, outparams->algoRes);
#endif
#if defined(RKAIQ_HAVE_DRC_V20) || defined(RKAIQ_HAVE_DRC_V21)
#if ISP_HW_V33
        drcRes->sta = pDrcCtx->drc_attrib->stAuto.sta;
#endif
        DrcSelectParam(pDrcCtx, outparams->algoRes, &drc_proc_param->staTrans, iso);
        drcApplyStrength(pDrcCtx, outparams->algoRes);
#endif
        outparams->cfg_update = true;
        outparams->en         = drc_attrib->en;
        outparams->bypass     = drc_attrib->bypass;
        LOGI_ATMO("drc en:%d, bypass:%d", outparams->en, outparams->bypass);
    }

    pDrcCtx->iso      = iso;
    pDrcCtx->isReCal_ = false;

    pDrcCtx->CurrData.AEData.LongFrmMode = pDrcCtx->NextData.AEData.LongFrmMode;
    pDrcCtx->CurrData.AEData.L2M_Ratio   = pDrcCtx->NextData.AEData.L2M_Ratio;
    pDrcCtx->CurrData.AEData.M2S_Ratio   = pDrcCtx->NextData.AEData.M2S_Ratio;
    pDrcCtx->CurrData.AEData.L2S_Ratio   = pDrcCtx->NextData.AEData.L2S_Ratio;

    LOGV_ATMO("%s: (exit)\n", __FUNCTION__);
    return XCAM_RETURN_NO_ERROR;
}

#if RKAIQ_HAVE_DRC_V12
XCamReturn DrcSelectParam(DrcContext_t* pDrcCtx, drc_param_t* out, int iso) {
    LOGI_ATMO("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    if (pDrcCtx == NULL) {
        LOGE_ATMO("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    drc_param_auto_t* paut = &pDrcCtx->drc_attrib->stAuto;
    int i                  = 0;
    int iso_low = 0, iso_high = 0, ilow = 0, ihigh = 0;
    float ratio = 0.0f;
    pre_interp(iso, pDrcCtx->iso_list, 13, &ilow, &ihigh, &ratio);

    out->dyn.DrcGain.DrcGain =
        interpolation_f32(paut->dyn[ilow].DrcGain.DrcGain, paut->dyn[ihigh].DrcGain.DrcGain, ratio);
    out->dyn.DrcGain.Alpha =
        interpolation_f32(paut->dyn[ilow].DrcGain.Alpha, paut->dyn[ihigh].DrcGain.Alpha, ratio);
    out->dyn.DrcGain.Clip =
        interpolation_f32(paut->dyn[ilow].DrcGain.Clip, paut->dyn[ihigh].DrcGain.Clip, ratio);
    out->dyn.HiLightData.Strength = interpolation_f32(paut->dyn[ilow].HiLightData.Strength,
                                                      paut->dyn[ihigh].HiLightData.Strength, ratio);
    out->dyn.HiLightData.gas_t    = interpolation_f32(paut->dyn[ilow].HiLightData.gas_t,
                                                   paut->dyn[ihigh].HiLightData.gas_t, ratio);

    out->dyn.LocalData.LocalWeit      = interpolation_f32(paut->dyn[ilow].LocalData.LocalWeit,
                                                     paut->dyn[ihigh].LocalData.LocalWeit, ratio);
    out->dyn.LocalData.GlobalContrast = interpolation_f32(
        paut->dyn[ilow].LocalData.GlobalContrast, paut->dyn[ihigh].LocalData.GlobalContrast, ratio);
    out->dyn.LocalData.LoLitContrast = interpolation_f32(
        paut->dyn[ilow].LocalData.LoLitContrast, paut->dyn[ihigh].LocalData.LoLitContrast, ratio);
    out->dyn.LocalData.LocalAutoEnable = paut->dyn[ilow].LocalData.LocalAutoEnable;
    out->dyn.LocalData.LocalAutoWeit   = interpolation_f32(
        paut->dyn[ilow].LocalData.LocalAutoWeit, paut->dyn[ihigh].LocalData.LocalAutoWeit, ratio);
    // get MotionStr
    out->dyn.MotionData.MotionStr = paut->dyn[0].MotionData.MotionStr;
    // compress
    out->sta.CompressSetting.Mode = paut->sta.CompressSetting.Mode;
    for (int i = 0; i < ADRC_Y_NUM; i++)
        out->sta.CompressSetting.Manual_curve[i] = paut->sta.CompressSetting.Manual_curve[i];
    out->sta.LocalSetting.curPixWeit    = paut->sta.LocalSetting.curPixWeit;
    out->sta.LocalSetting.preFrameWeit  = paut->sta.LocalSetting.preFrameWeit;
    out->sta.LocalSetting.Range_sgm_pre = paut->sta.LocalSetting.Range_sgm_pre;
    out->sta.LocalSetting.Space_sgm_pre = paut->sta.LocalSetting.Space_sgm_pre;

    out->sta.LocalSetting.Range_force_sgm = paut->sta.LocalSetting.Range_force_sgm;
    out->sta.LocalSetting.Range_sgm_cur   = paut->sta.LocalSetting.Range_sgm_cur;
    out->sta.LocalSetting.Space_sgm_cur   = paut->sta.LocalSetting.Space_sgm_cur;
    // scale y
    for (int i = 0; i < DRC_Y_NUM; i++) out->sta.Scale_y[i] = (unsigned short)paut->sta.Scale_y[i];

    out->sta.sw_drc_byPass_thred = paut->sta.sw_drc_byPass_thred;
    out->sta.Edge_Weit           = paut->sta.Edge_Weit;
    out->sta.IIR_frame           = (float)MIN(pDrcCtx->FrameID + 1, (uint32_t)paut->sta.IIR_frame);
    out->sta.damp                = paut->sta.damp;
    out->sta.OutPutLongFrame = pDrcCtx->NextData.AEData.LongFrmMode || paut->sta.OutPutLongFrame;
    // drc v12 add
    out->sta.HiLight.gas_l0 = paut->sta.HiLight.gas_l0;
    out->sta.HiLight.gas_l1 = paut->sta.HiLight.gas_l1;
    out->sta.HiLight.gas_l2 = paut->sta.HiLight.gas_l2;
    out->sta.HiLight.gas_l3 = paut->sta.HiLight.gas_l3;
    pDrcCtx->isDampStable   = DrcDamping(out, &pDrcCtx->CurrData, pDrcCtx->FrameID);

    // clip drc gain
    if (pDrcCtx->FrameNumber == HDR_2X_NUM || pDrcCtx->FrameNumber == HDR_3X_NUM) {
        if (pDrcCtx->NextData.AEData.L2S_Ratio * out->dyn.DrcGain.DrcGain > MAX_AE_DRC_GAIN) {
            out->dyn.DrcGain.DrcGain =
                MAX(MAX_AE_DRC_GAIN / pDrcCtx->NextData.AEData.L2S_Ratio, GAINMIN);
            LOGI_ATMO("%s:  AERatio*DrcGain > 256x, DrcGain Clip to %f!!!\n", __FUNCTION__,
                      out->dyn.DrcGain.DrcGain);
        }
    } else if (pDrcCtx->FrameNumber == LINEAR_NUM) {
        if (pDrcCtx->isp_ob_predgain * out->dyn.DrcGain.DrcGain > MAX_AE_DRC_GAIN) {
            if (pDrcCtx->isp_ob_predgain > MAX_AE_DRC_GAIN)
                LOGE_ATMO("%s:  predgain > 256x!!!\n", __FUNCTION__);
            else
                out->dyn.DrcGain.DrcGain = MAX(MAX_AE_DRC_GAIN / pDrcCtx->isp_ob_predgain, GAINMIN);
            LOGI_ATMO("%s:  predgain*DrcGain > 256x, DrcGain clip to %f!!!\n", __FUNCTION__,
                      out->dyn.DrcGain.DrcGain);
        }
    } else if (pDrcCtx->FrameNumber == SENSOR_MGE) {
        if (pow(2.0f, (float)(pDrcCtx->compr_bit - ISP_HDR_BIT_NUM_MIN)) *
                out->dyn.DrcGain.DrcGain >
            MAX_AE_DRC_GAIN) {
            if (pow(2.0f, (float)(pDrcCtx->compr_bit - ISP_HDR_BIT_NUM_MIN)) > MAX_AE_DRC_GAIN)
                LOGE_ATMO("%s:  SensorMgeRatio > 256x!!!\n", __FUNCTION__);
            else
                out->dyn.DrcGain.DrcGain = MAX(
                    MAX_AE_DRC_GAIN / pow(2.0f, (float)(pDrcCtx->compr_bit - ISP_HDR_BIT_NUM_MIN)),
                    GAINMIN);
            LOGI_ATMO("%s:  SensorMgeRatio*DrcGain > 256x, DrcGain clip to %f!!!\n", __FUNCTION__,
                      out->dyn.DrcGain.DrcGain);
        }
    }
    // clip gas_l0~3
    if (out->sta.HiLight.gas_l0 == GAS_L_MAX) {
        LOGE_ATMO("%s: gas_l0 equals %d, use default value\n", __FUNCTION__, GAS_L_MAX);
        out->sta.HiLight.gas_l0 = GAS_L0_DEFAULT;
        out->sta.HiLight.gas_l1 = GAS_L1_DEFAULT;
        out->sta.HiLight.gas_l2 = GAS_L2_DEFAULT;
        out->sta.HiLight.gas_l3 = GAS_L3_DEFAULT;
    }
    if ((out->sta.HiLight.gas_l0 + 2 * out->sta.HiLight.gas_l1 + out->sta.HiLight.gas_l2 +
         2 * out->sta.HiLight.gas_l3) != GAS_L_MAX) {
        // LOGE_ATMO("%s: gas_l0 + gas_l1 + gas_l2 + gas_l3 DO NOT equal %d, use default value\n",
        //           __FUNCTION__, GAS_L_MAX);
        out->sta.HiLight.gas_l0 = GAS_L0_DEFAULT;
        out->sta.HiLight.gas_l1 = GAS_L1_DEFAULT;
        out->sta.HiLight.gas_l2 = GAS_L2_DEFAULT;
        out->sta.HiLight.gas_l3 = GAS_L3_DEFAULT;
    }
    out->sta.delta_scalein = DELTA_SCALEIN_FIX;
    out->sta.offset_pow2   = SW_DRC_OFFSET_POW2_FIX;
    float tmp              = 0.0f;
    /*luma[i] = pow((1.0f - luma[i] / 4096.0f), 2.0f)*/
    float luma[DRC_Y_NUM] = {1.0f,    0.8789f, 0.7656f, 0.6602f, 0.5625f, 0.4727f,
                             0.3906f, 0.3164f, 0.2500f, 0.1914f, 0.1406f, 0.0977f,
                             0.0625f, 0.0352f, 0.0156f, 0.0039f, 0.0f};

    for (int i = 0; i < DRC_Y_NUM; ++i) {
        tmp = 1 - out->dyn.DrcGain.Alpha * luma[i];
        if (pDrcCtx->blc_ob_enable)
            tmp = 1024.0f * pow(out->dyn.DrcGain.DrcGain, tmp) *
                  pow(pDrcCtx->isp_ob_predgain, -out->dyn.DrcGain.Alpha * luma[i]);
        else
            tmp = 1024.0f * pow(out->dyn.DrcGain.DrcGain, tmp);
        out->sta.gain_y[i] = (unsigned short)(tmp);
    }

    if (pDrcCtx->drc_attrib->opMode == RK_AIQ_OP_MODE_AUTO && !pDrcCtx->isDampStable)
        pDrcCtx->CurrData.dynParams = out->dyn;

    LOG1_ATMO("%s:exit!\n", __FUNCTION__);

    return XCAM_RETURN_NO_ERROR;
}

bool DrcDamping(drc_param_t* out, CurrData_t* pCurrData, int FrameID) {
    LOG1_ATMO("%s:Enter!\n", __FUNCTION__);
    bool isDampStable = false;

    if (FrameID > INIT_CALC_PARAMS_NUM) {
        bool isDampStable_DrcGain = false, isDampStable_Alpha = false, isDampStable_Clip = false,
             isDampStable_Strength = false, isDampStable_LocalWeit = false,
             isDampStable_LocalAutoWeit = false, isDampStable_GlobalContrast = false,
             isDampStable_LoLitContrast = false, isDampStable_gas_t = false,
             isDampStable_MotionStr = false;

        if ((out->dyn.DrcGain.DrcGain - pCurrData->dynParams.DrcGain.DrcGain) <= FLT_EPSILON &&
            (out->dyn.DrcGain.DrcGain - pCurrData->dynParams.DrcGain.DrcGain) >= -FLT_EPSILON) {
            isDampStable_DrcGain = true;
        } else {
            out->dyn.DrcGain.DrcGain =
                out->sta.damp * out->dyn.DrcGain.DrcGain +
                (1.0f - out->sta.damp) * pCurrData->dynParams.DrcGain.DrcGain;
            isDampStable_DrcGain = false;
        }
        if ((out->dyn.DrcGain.Alpha - pCurrData->dynParams.DrcGain.Alpha) <= FLT_EPSILON &&
            (out->dyn.DrcGain.Alpha - pCurrData->dynParams.DrcGain.Alpha) >= -FLT_EPSILON) {
            isDampStable_Alpha = true;
        } else {
            out->dyn.DrcGain.Alpha = out->sta.damp * out->dyn.DrcGain.Alpha +
                                     (1.0f - out->sta.damp) * pCurrData->dynParams.DrcGain.Alpha;
            isDampStable_Alpha = false;
        }
        if ((out->dyn.DrcGain.Clip - pCurrData->dynParams.DrcGain.Clip) <= FLT_EPSILON &&
            (out->dyn.DrcGain.Clip - pCurrData->dynParams.DrcGain.Clip) >= -FLT_EPSILON) {
            isDampStable_Clip = true;
        } else {
            out->dyn.DrcGain.Clip = out->sta.damp * out->dyn.DrcGain.Clip +
                                    (1.0f - out->sta.damp) * pCurrData->dynParams.DrcGain.Clip;
            isDampStable_Clip = false;
        }
        if ((out->dyn.HiLightData.Strength - pCurrData->dynParams.HiLightData.Strength) <=
                FLT_EPSILON &&
            (out->dyn.HiLightData.Strength - pCurrData->dynParams.HiLightData.Strength) >=
                -FLT_EPSILON) {
            isDampStable_Strength = true;
        } else {
            out->dyn.HiLightData.Strength =
                out->sta.damp * out->dyn.HiLightData.Strength +
                (1.0f - out->sta.damp) * pCurrData->dynParams.HiLightData.Strength;
            isDampStable_Strength = false;
        }
        if ((out->dyn.LocalData.LocalWeit - pCurrData->dynParams.LocalData.LocalWeit) <=
                FLT_EPSILON &&
            (out->dyn.LocalData.LocalWeit - pCurrData->dynParams.LocalData.LocalWeit) >=
                -FLT_EPSILON) {
            isDampStable_LocalWeit = true;
        } else {
            out->dyn.LocalData.LocalWeit =
                out->sta.damp * out->dyn.LocalData.LocalWeit +
                (1.0f - out->sta.damp) * pCurrData->dynParams.LocalData.LocalWeit;
            isDampStable_LocalWeit = false;
        }
        if ((out->dyn.LocalData.LocalAutoWeit - pCurrData->dynParams.LocalData.LocalAutoWeit) <=
                FLT_EPSILON &&
            (out->dyn.LocalData.LocalAutoWeit - pCurrData->dynParams.LocalData.LocalAutoWeit) >=
                -FLT_EPSILON) {
            isDampStable_LocalAutoWeit = true;
        } else {
            out->dyn.LocalData.LocalAutoWeit =
                out->sta.damp * out->dyn.LocalData.LocalAutoWeit +
                (1.0f - out->sta.damp) * pCurrData->dynParams.LocalData.LocalAutoWeit;
            isDampStable_LocalAutoWeit = false;
        }
        if ((out->dyn.LocalData.GlobalContrast - pCurrData->dynParams.LocalData.GlobalContrast) <=
                FLT_EPSILON &&
            (out->dyn.LocalData.GlobalContrast - pCurrData->dynParams.LocalData.GlobalContrast) >=
                -FLT_EPSILON) {
            isDampStable_GlobalContrast = true;
        } else {
            out->dyn.LocalData.GlobalContrast =
                out->sta.damp * out->dyn.LocalData.GlobalContrast +
                (1.0f - out->sta.damp) * pCurrData->dynParams.LocalData.GlobalContrast;
            isDampStable_GlobalContrast = false;
        }
        if ((out->dyn.LocalData.LoLitContrast - pCurrData->dynParams.LocalData.LoLitContrast) <=
                FLT_EPSILON &&
            (out->dyn.LocalData.LoLitContrast - pCurrData->dynParams.LocalData.LoLitContrast) >=
                -FLT_EPSILON) {
            isDampStable_LoLitContrast = true;
        } else {
            out->dyn.LocalData.LoLitContrast =
                out->sta.damp * out->dyn.LocalData.LoLitContrast +
                (1.0f - out->sta.damp) * pCurrData->dynParams.LocalData.LoLitContrast;
            isDampStable_LoLitContrast = false;
        }
        // drc v12
        if ((out->dyn.HiLightData.gas_t - pCurrData->dynParams.HiLightData.gas_t) <= FLT_EPSILON &&
            (out->dyn.HiLightData.gas_t - pCurrData->dynParams.HiLightData.gas_t) >= -FLT_EPSILON) {
            isDampStable_gas_t = true;
        } else {
            out->dyn.HiLightData.gas_t =
                out->sta.damp * out->dyn.HiLightData.gas_t +
                (1.0f - out->sta.damp) * pCurrData->dynParams.HiLightData.gas_t;
            isDampStable_gas_t = false;
        }
        if ((out->dyn.MotionData.MotionStr - pCurrData->dynParams.MotionData.MotionStr) <=
                FLT_EPSILON &&
            (out->dyn.MotionData.MotionStr - pCurrData->dynParams.MotionData.MotionStr) >=
                -FLT_EPSILON) {
            isDampStable_MotionStr = true;
        } else {
            out->dyn.MotionData.MotionStr =
                out->sta.damp * out->dyn.MotionData.MotionStr +
                (1.0f - out->sta.damp) * pCurrData->dynParams.MotionData.MotionStr;
            isDampStable_MotionStr = false;
        }
        isDampStable = isDampStable_DrcGain && isDampStable_Alpha && isDampStable_Clip &&
                       isDampStable_Strength && isDampStable_LocalWeit &&
                       isDampStable_LocalAutoWeit && isDampStable_GlobalContrast &&
                       isDampStable_LoLitContrast && isDampStable_gas_t && isDampStable_MotionStr;
    } else
        isDampStable = false;

    LOG1_ATMO("%s:Eixt!\n", __FUNCTION__);
    return isDampStable;
}

void DrcExpoParaProcessing(DrcContext_t* pDrcCtx, drc_param_t* out) {
    LOG1_ATMO("%s:enter!\n", __FUNCTION__);

    // get sw_drc_compres_scl
    float adrc_gain = 1.0f;
    if (pDrcCtx->FrameNumber == LINEAR_NUM && pDrcCtx->blc_ob_enable)
        adrc_gain = out->dyn.DrcGain.DrcGain * pDrcCtx->isp_ob_predgain;
    else
        adrc_gain = out->dyn.DrcGain.DrcGain;
    float log_ratio2     = log(pDrcCtx->NextData.AEData.L2S_Ratio * adrc_gain) / log(2.0f) + 12.0f;
    float offsetbits_int = out->sta.offset_pow2;
    float offsetbits     = offsetbits_int * (1 << MFHDR_LOG_Q_BITS);
    float hdrbits        = log_ratio2 * (1 << MFHDR_LOG_Q_BITS);
    float hdrvalidbits   = hdrbits - offsetbits;
    out->sta.compres_scl = (12.0f * (1 << (MFHDR_LOG_Q_BITS * 2))) / hdrvalidbits;

    // get sw_drc_min_ogain
    if (out->sta.OutPutLongFrame)
        out->sta.sw_drc_min_ogain = 1;
    else {
        out->sta.sw_drc_min_ogain = 1.0f / (pDrcCtx->NextData.AEData.L2S_Ratio * adrc_gain);
    }

    // get sw_drc_compres_y
    if (out->sta.CompressSetting.Mode == drc_COMPRESS_AUTO) {
        float curveparam, curveparam2, curveparam3, tmp;
        float luma2[DRC_Y_NUM] = {0.0f,     1024.0f,  2048.0f,  3072.0f,  4096.0f,  5120.0f,
                                  6144.0f,  7168.0f,  8192.0f,  10240.0f, 12288.0f, 14336.0f,
                                  16384.0f, 18432.0f, 20480.0f, 22528.0f, 24576.0f};
        float curveTable;
        float dstbits   = ISP_RAW_BIT * (1 << MFHDR_LOG_Q_BITS);
        float validbits = dstbits - offsetbits;
        for (int i = 0; i < DRC_Y_NUM; ++i) {
            curveparam =
                (float)(validbits - 0.0f) / (hdrvalidbits - validbits + DRC_COMPRESS_Y_OFFSET);
            curveparam2           = validbits * (1.0f + curveparam);
            curveparam3           = hdrvalidbits * curveparam;
            tmp                   = luma2[i] * hdrvalidbits / 24576.0f;
            curveTable            = (tmp * curveparam2 / (tmp + curveparam3));
            out->sta.compres_y[i] = (unsigned short)(curveTable);
        }
    } else if (out->sta.CompressSetting.Mode == drc_COMPRESS_MANUAL) {
        for (int i = 0; i < DRC_Y_NUM; ++i)
            out->sta.compres_y[i] = (unsigned short)(out->sta.CompressSetting.Manual_curve[i]);
    }
}
#endif

#if defined(RKAIQ_HAVE_DRC_V20) || defined(RKAIQ_HAVE_DRC_V21)
XCamReturn DrcSelectParam(DrcContext_t* pDrcCtx, drc_param_t* out, trans_params_static_t* pstaTrans,
                          int iso) {
    LOGI_ATMO("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    if (pDrcCtx == NULL) {
        LOGE_ATMO("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    drc_param_auto_t* paut = &pDrcCtx->drc_attrib->stAuto;
    int i                  = 0;
    int iso_low = 0, iso_high = 0, ilow = 0, ihigh = 0, inear = 0;
    float ratio = 0.0f;
    uint16_t uratio;
    pre_interp(iso, pDrcCtx->iso_list, 13, &ilow, &ihigh, &ratio);
    uratio = ratio * (1 << RATIO_FIXBIT);

    if (ratio > 0.5)
        inear = ihigh;
    else
        inear = ilow;

    // get preProc
    // get sw_drcT_toneGain_maxLimit
    out->dyn.preProc.toneCurveCtrl.sw_drcT_toneGain_maxLimit =
        interpolation_f32(paut->dyn[ilow].preProc.toneCurveCtrl.sw_drcT_toneGain_maxLimit,
                          paut->dyn[ihigh].preProc.toneCurveCtrl.sw_drcT_toneGain_maxLimit, ratio);
    // get sw_drcT_toneCurveK_coeff
    out->dyn.preProc.toneCurveCtrl.sw_drcT_toneCurveK_coeff =
        interpolation_f32(paut->dyn[ilow].preProc.toneCurveCtrl.sw_drcT_toneCurveK_coeff,
                          paut->dyn[ihigh].preProc.toneCurveCtrl.sw_drcT_toneCurveK_coeff, ratio);
    // get sw_drcT_toneCurve_mode
    out->dyn.preProc.sw_drcT_toneCurve_mode = paut->dyn[inear].preProc.sw_drcT_toneCurve_mode;
    // get hw_drcT_toneCurveIdx_scale
    out->dyn.preProc.hw_drcT_toneCurveIdx_scale =
        interpolation_f32(paut->dyn[ilow].preProc.hw_drcT_toneCurveIdx_scale,
                          paut->dyn[ihigh].preProc.hw_drcT_toneCurveIdx_scale, ratio);
    // get hw_drcT_lpfSoftThd_thred
    out->dyn.preProc.hw_drcT_lpfSoftThd_thred =
        interpolation_f32(paut->dyn[ilow].preProc.hw_drcT_lpfSoftThd_thred,
                          paut->dyn[ihigh].preProc.hw_drcT_lpfSoftThd_thred, ratio);
    // get hw_drcT_luma2ToneGain_val
    for (int i = 0; i < DRC_CURVE_LEN; i++)
        out->dyn.preProc.hw_drcT_luma2ToneGain_val[i] =
            interpolation_f32(paut->dyn[ilow].preProc.hw_drcT_luma2ToneGain_val[i],
                              paut->dyn[ihigh].preProc.hw_drcT_luma2ToneGain_val[i], ratio);
#if RKAIQ_HAVE_DRC_V21
    out->dyn.preProc.hw_drcT_lumaFilt_maxWgt =
        interpolation_f32(paut->dyn[ilow].preProc.hw_drcT_lumaFilt_maxWgt,
                          paut->dyn[ihigh].preProc.hw_drcT_lumaFilt_maxWgt, ratio);
    out->dyn.preProc.hw_drcT_lumaFilt_midWgt =
        interpolation_f32(paut->dyn[ilow].preProc.hw_drcT_lumaFilt_midWgt,
                          paut->dyn[ihigh].preProc.hw_drcT_lumaFilt_midWgt, ratio);
    out->dyn.preProc.hw_drcT_lumaFilt_minWgt =
        interpolation_f32(paut->dyn[ilow].preProc.hw_drcT_lumaFilt_minWgt,
                          paut->dyn[ihigh].preProc.hw_drcT_lumaFilt_minWgt, ratio);
#endif

    // get bifilt_filter
    // get hw_drcT_bifiltOut_alpha
    out->dyn.bifilt_filter.hw_drcT_bifiltOut_alpha =
        interpolation_f32(paut->dyn[ilow].bifilt_filter.hw_drcT_bifiltOut_alpha,
                          paut->dyn[ihigh].bifilt_filter.hw_drcT_bifiltOut_alpha, ratio);
    // get hw_drcT_softThd_en
    out->dyn.bifilt_filter.hw_drcT_softThd_en = paut->dyn[ilow].bifilt_filter.hw_drcT_softThd_en;
    // get hw_drcT_softThd_thred
    out->dyn.bifilt_filter.hw_drcT_softThd_thred =
        interpolation_f32(paut->dyn[ilow].bifilt_filter.hw_drcT_softThd_thred,
                          paut->dyn[ihigh].bifilt_filter.hw_drcT_softThd_thred, ratio);
    // get hw_drcT_centerPixel_wgt
    out->dyn.bifilt_filter.hw_drcT_centerPixel_wgt =
        interpolation_f32(paut->dyn[ilow].bifilt_filter.hw_drcT_centerPixel_wgt,
                          paut->dyn[ihigh].bifilt_filter.hw_drcT_centerPixel_wgt, ratio);
    // get hw_drcT_midWgt_alpha
    out->dyn.bifilt_filter.hw_drcT_midWgt_alpha =
        interpolation_f32(paut->dyn[ilow].bifilt_filter.hw_drcT_midWgt_alpha,
                          paut->dyn[ihigh].bifilt_filter.hw_drcT_midWgt_alpha, ratio);
    // get hw_drcT_rgeWgt_negOff
    out->dyn.bifilt_filter.hw_drcT_rgeWgt_negOff =
        interpolation_f32(paut->dyn[ilow].bifilt_filter.hw_drcT_rgeWgt_negOff,
                          paut->dyn[ihigh].bifilt_filter.hw_drcT_rgeWgt_negOff, ratio);
    // get hw_drcT_midRgeSgm_val
    out->dyn.bifilt_filter.hw_drcT_midRgeSgm_val =
        interpolation_f32(paut->dyn[ilow].bifilt_filter.hw_drcT_midRgeSgm_val,
                          paut->dyn[ihigh].bifilt_filter.hw_drcT_midRgeSgm_val, ratio);
    // get hw_drcT_loRgeSgm_val
    out->dyn.bifilt_filter.hw_drcT_loRgeSgm_val =
        interpolation_f32(paut->dyn[ilow].bifilt_filter.hw_drcT_loRgeSgm_val,
                          paut->dyn[ihigh].bifilt_filter.hw_drcT_loRgeSgm_val, ratio);

    // get bifilt_guideDiff
    // get hw_drc_thumb_maxLimit
    out->dyn.bifilt_guideDiff.hw_drcT_guideDiffLmt_en =
        interpolation_bool(paut->dyn[ilow].bifilt_guideDiff.hw_drcT_guideDiffLmt_en,
                           paut->dyn[ihigh].bifilt_guideDiff.hw_drcT_guideDiffLmt_en, uratio);
    out->dyn.bifilt_guideDiff.hw_drcT_guideLuma_maxLimit =
        interpolation_f32(paut->dyn[ilow].bifilt_guideDiff.hw_drcT_guideLuma_maxLimit,
                          paut->dyn[ihigh].bifilt_guideDiff.hw_drcT_guideLuma_maxLimit, ratio);
    // get hw_drcT_maxLutIdx_scale
    out->dyn.bifilt_guideDiff.hw_drcT_maxLutIdx_scale =
        interpolation_f32(paut->dyn[ilow].bifilt_guideDiff.hw_drcT_maxLutIdx_scale,
                          paut->dyn[ihigh].bifilt_guideDiff.hw_drcT_maxLutIdx_scale, ratio);
    // get hw_drcT_guideDiff_minLimit
    out->dyn.bifilt_guideDiff.hw_drcT_guideDiff_minLimit =
        interpolation_f32(paut->dyn[ilow].bifilt_guideDiff.hw_drcT_guideDiff_minLimit,
                          paut->dyn[ihigh].bifilt_guideDiff.hw_drcT_guideDiff_minLimit, ratio);
    // get sw_drcT_gdDiffMaxLut_mode
    out->dyn.bifilt_guideDiff.sw_drcT_gdDiffMaxLut_mode =
        paut->dyn[inear].bifilt_guideDiff.sw_drcT_gdDiffMaxLut_mode;
    // get hw_drcT_gdLuma2DiffMax_lut
    for (int i = 0; i < DRC_CURVE_LEN; i++)
        out->dyn.bifilt_guideDiff.hw_drcT_gdLuma2DiffMax_lut[i] = interpolation_f32(
            paut->dyn[ilow].bifilt_guideDiff.hw_drcT_gdLuma2DiffMax_lut[i],
            paut->dyn[ihigh].bifilt_guideDiff.hw_drcT_gdLuma2DiffMax_lut[i], ratio);
    // get sw_drc_thumbThred_minLimit
    out->dyn.bifilt_guideDiff.gdDiffMaxCurveCtrl.sw_drcT_maxLutCreate_minLimit = interpolation_f32(
        paut->dyn[ilow].bifilt_guideDiff.gdDiffMaxCurveCtrl.sw_drcT_maxLutCreate_minLimit,
        paut->dyn[ihigh].bifilt_guideDiff.gdDiffMaxCurveCtrl.sw_drcT_maxLutCreate_minLimit, ratio);
    // get sw_drc_thumbThred_maxLimit
    out->dyn.bifilt_guideDiff.gdDiffMaxCurveCtrl.sw_drcT_maxLutCreate_maxLimit = interpolation_f32(
        paut->dyn[ilow].bifilt_guideDiff.gdDiffMaxCurveCtrl.sw_drcT_maxLutCreate_maxLimit,
        paut->dyn[ihigh].bifilt_guideDiff.gdDiffMaxCurveCtrl.sw_drcT_maxLutCreate_maxLimit, ratio);
    // get sw_drc_thumbThredCurve_slope
    out->dyn.bifilt_guideDiff.gdDiffMaxCurveCtrl.sw_drcT_maxLutCreate_slope = interpolation_f32(
        paut->dyn[ilow].bifilt_guideDiff.gdDiffMaxCurveCtrl.sw_drcT_maxLutCreate_slope,
        paut->dyn[ihigh].bifilt_guideDiff.gdDiffMaxCurveCtrl.sw_drcT_maxLutCreate_slope, ratio);
    // get sw_drc_thumbThredCurve_offset
    out->dyn.bifilt_guideDiff.gdDiffMaxCurveCtrl.sw_drcT_maxLutCreate_offset = interpolation_f32(
        paut->dyn[ilow].bifilt_guideDiff.gdDiffMaxCurveCtrl.sw_drcT_maxLutCreate_offset,
        paut->dyn[ihigh].bifilt_guideDiff.gdDiffMaxCurveCtrl.sw_drcT_maxLutCreate_offset, ratio);

    // get drcProc
    // get hw_drcT_locDetail_strg
    out->dyn.drcProc.hw_drcT_locDetail_strg =
        interpolation_f32(paut->dyn[ilow].drcProc.hw_drcT_locDetail_strg,
                          paut->dyn[ihigh].drcProc.hw_drcT_locDetail_strg, ratio);
    // get hw_drcT_hfDarkRegion_strg
    out->dyn.drcProc.hw_drcT_hfDarkRegion_strg =
        interpolation_f32(paut->dyn[ilow].drcProc.hw_drcT_hfDarkRegion_strg,
                          paut->dyn[ihigh].drcProc.hw_drcT_hfDarkRegion_strg, ratio);
    // get hw_drcT_drcStrgLutLuma_scale
    out->dyn.drcProc.hw_drcT_drcStrgLutLuma_scale =
        interpolation_f32(paut->dyn[ilow].drcProc.hw_drcT_drcStrgLutLuma_scale,
                          paut->dyn[ihigh].drcProc.hw_drcT_drcStrgLutLuma_scale, ratio);
    // get sw_drcT_drcStrgLut_mode
    out->dyn.drcProc.sw_drcT_drcStrgLut_mode = paut->dyn[inear].drcProc.sw_drcT_drcStrgLut_mode;
    // get hw_drc_luma2compsGainScale_val
    for (int i = 0; i < DRC_CURVE_LEN; i++)
        out->dyn.drcProc.hw_drcT_luma2DrcStrg_val[i] =
            interpolation_f32(paut->dyn[ilow].drcProc.hw_drcT_luma2DrcStrg_val[i],
                              paut->dyn[ihigh].drcProc.hw_drcT_luma2DrcStrg_val[i], ratio);
	
    // get sw_drcT_drcCurve_mode
    out->dyn.drcProc.sw_drcT_drcCurve_mode = paut->dyn[inear].drcProc.sw_drcT_drcCurve_mode;
    if (out->dyn.drcProc.sw_drcT_drcCurve_mode < adrc_auto_mode) {
        for (int i = 0; i < DRC_CURVE_LEN; ++i)
            out->dyn.drcProc.hw_drcT_hdr2Sdr_curve[i] =
                interpolation_u16(paut->dyn[ilow].drcProc.hw_drcT_hdr2Sdr_curve[i],
                                  paut->dyn[ihigh].drcProc.hw_drcT_hdr2Sdr_curve[i], uratio);
        // get sw_drcT_drcGainLimit_mode
        out->dyn.drcProc.sw_drcT_drcGainLimit_mode =
            paut->dyn[inear].drcProc.sw_drcT_drcGainLimit_mode;
        out->dyn.drcProc.hw_drcT_drcGain_minLimit =
            interpolation_f32(paut->dyn[ilow].drcProc.hw_drcT_drcGain_minLimit,
                              paut->dyn[ihigh].drcProc.hw_drcT_drcGain_minLimit, ratio);
    } else {
        out->dyn.drcProc.sw_drcT_drcGainLimit_mode = drc_drcGainLmt_manual_mode;
        drcApplyStats(pDrcCtx, out, pstaTrans, ilow, ihigh, ratio);
    }
    pDrcCtx->CurrData.autoCurveIIRParams.sw_drcT_drcCurve_mode =
        out->dyn.drcProc.sw_drcT_drcCurve_mode;

    // pDrcCtx->isDampStable = DrcDamping(out, &pDrcCtx->CurrData, pDrcCtx->FrameID);

    if (pDrcCtx->drc_attrib->opMode == RK_AIQ_OP_MODE_AUTO) pDrcCtx->CurrData.dynParams = out->dyn;

    LOG1_ATMO("%s:exit!\n", __FUNCTION__);

    return XCAM_RETURN_NO_ERROR;
}

static void drc_create_curve(float* x, float* y, float* input_luma, float* cmps_curve) {
    float out_x[17];
    float out_y[17];
    float n     = 4;
    float nk[5] = {1, 4, 6, 4, 1};

    for (int i = 0; i < 17; ++i) {
        out_x[i] = 0;
        out_y[i] = 0;
        float t  = (float)i / 16.0f;
        for (int j = 0; j < 5; ++j) {
            float nj = nk[j];
            float bnj;
            bnj = nj * pow(1 - t, n - j) * pow(t, j);
            out_x[i] += bnj * x[j];
            out_y[i] += bnj * y[j];
        }
    }

    for (int i = 0; i < 17; ++i) {
        float idx_luma = input_luma[i];
        int idx        = 0;
        for (int j = 0; j < 17; ++j) {
            if (idx_luma <= out_x[j]) {
                idx = j - 1;
                break;
            }
        }
        idx      = MAX(idx, 0);
        int idx2 = idx + 1;

        float w1      = (out_x[idx2] - idx_luma) / (out_x[idx2] - out_x[idx]);
        cmps_curve[i] = w1 * out_y[idx] + (1 - w1) * out_y[idx2];
    }
}

static void drcApplyStats(DrcContext_t* pDrcCtx, drc_param_t* out, trans_params_static_t* pstaTrans,
                          int ilow, int ihigh, float ratio) {
    drc_param_auto_t* paut        = &pDrcCtx->drc_attrib->stAuto;
    rkisp_adrc_stats_t* drc_stats = pDrcCtx->drc_stats;

    int cmps_fix_bit     = 8 + pstaTrans->hw_transCfg_trans_mode;
    int cmps_offset_bits = pstaTrans->hw_transCfg_lscOutTrans_offset << cmps_fix_bit;
    int cmps_offset      = 1 << pstaTrans->hw_transCfg_lscOutTrans_offset;
    int dstbits          = 12 << cmps_fix_bit;
    int offsetbits_int   = pstaTrans->hw_transCfg_transOfDrc_offset;
    int offsetbits       = offsetbits_int << cmps_fix_bit;
    int validbits        = dstbits - offsetbits;

    float adrc_gain = 0.0f;
    if (out->dyn.preProc.sw_drcT_toneCurve_mode == drc_cfgCurveCtrlCoeff_mode)
        adrc_gain = out->dyn.preProc.toneCurveCtrl.sw_drcT_toneGain_maxLimit;
    else if (out->dyn.preProc.sw_drcT_toneCurve_mode == drc_cfgCurveDirect_mode)
        adrc_gain = out->dyn.preProc.hw_drcT_luma2ToneGain_val[16];

    float luma2[DRC_V12_Y_NUM] = {0.0f,     1024.0f,  2048.0f,  3072.0f,  4096.0f,  5120.0f,
                                  6144.0f,  7168.0f,  8192.0f,  10240.0f, 12288.0f, 14336.0f,
                                  16384.0f, 18432.0f, 20480.0f, 22528.0f, 24576.0f};

    float percentage =
        interpolation_f32(paut->dyn[ilow].drcProc.sw_drcT_drcCurve_auto.sw_drcT_wp_val,
                          paut->dyn[ihigh].drcProc.sw_drcT_drcCurve_auto.sw_drcT_wp_val, ratio);
    float percentage2 = interpolation_f32(
        paut->dyn[ilow].drcProc.sw_drcT_drcCurve_auto.sw_drcT_anchorPoint_x1,
        paut->dyn[ihigh].drcProc.sw_drcT_drcCurve_auto.sw_drcT_anchorPoint_x1, ratio);
    float percentage3 = interpolation_f32(
        paut->dyn[ilow].drcProc.sw_drcT_drcCurve_auto.sw_drcT_anchorPoint_x2,
        paut->dyn[ihigh].drcProc.sw_drcT_drcCurve_auto.sw_drcT_anchorPoint_x2, ratio);
    float percentage4 = interpolation_f32(
        paut->dyn[ilow].drcProc.sw_drcT_drcCurve_auto.sw_drcT_anchorPoint_x3,
        paut->dyn[ihigh].drcProc.sw_drcT_drcCurve_auto.sw_drcT_anchorPoint_x3, ratio);
    int thd_num  = drc_stats->ae_hist_total_num * percentage;
    int thd_num2 = drc_stats->ae_hist_total_num * percentage2;
    int thd_num3 = drc_stats->ae_hist_total_num * percentage3;
    int thd_num4 = drc_stats->ae_hist_total_num * percentage4;
    int cnt = 0, cnt2 = 0, cnt3 = 0, cnt4 = 0;
    float quantile = 1.0f, quantile2 = 0.2f, quantile3 = 0.5f, quantile4 = 0.8f;
    if (drc_stats->stats_true) {
        for (int i = 0; i < DRC_AE_HIST_BIN_NUM; ++i) {
            cnt += drc_stats->aeHiatBins[i];
            if (cnt > thd_num) {
                quantile = (float)i / 255.0f;
                break;
            }
        }
        for (int i = 0; i < DRC_AE_HIST_BIN_NUM; ++i) {
            cnt2 += drc_stats->aeHiatBins[i];
            if (cnt2 >= thd_num2) {
                quantile2 = (float)i / 255.0f;
                break;
            }
        }
        for (int i = 0; i < DRC_AE_HIST_BIN_NUM; ++i) {
            cnt3 += drc_stats->aeHiatBins[i];
            if (cnt3 >= thd_num3) {
                quantile3 = (float)i / 255.0f;
                break;
            }
        }
        for (int i = 0; i < DRC_AE_HIST_BIN_NUM; ++i) {
            cnt4 += drc_stats->aeHiatBins[i];
            if (cnt4 >= thd_num4) {
                quantile4 = (float)i / 255.0f;
                break;
            }
        }
    }
    float x[5];
    float y[5];
    float wp         = quantile;
    float cmps_max   = log(1.0f + (1 << offsetbits_int) / 4096.0f) / log(2.0f) + 12.0f;
    cmps_max -= offsetbits_int;
    float log_max =
        log(pDrcCtx->NextData.AEData.L2S_Ratio * adrc_gain + (1 << offsetbits_int) / 4096.0f) /
            log(2.0f) +
        12.0f;
    log_max -= offsetbits_int;
    float log_wp =
        log(wp * pDrcCtx->NextData.AEData.L2S_Ratio * adrc_gain + (1 << offsetbits_int) / 4096.0f) /
            log(2.0f) +
        12.0f;
    log_wp           = log_wp - offsetbits_int;
    float luma_scale = log_max / log_wp;
    float anchor_point[2][3];
    anchor_point[0][0] = quantile2 * adrc_gain;
    anchor_point[0][1] = quantile3 * adrc_gain;
    anchor_point[0][2] = quantile4 * adrc_gain;
    anchor_point[1][0] = interpolation_f32(
        paut->dyn[ilow].drcProc.sw_drcT_drcCurve_auto.sw_drcT_anchorPoint_y1,
        paut->dyn[ihigh].drcProc.sw_drcT_drcCurve_auto.sw_drcT_anchorPoint_y1, ratio);
    anchor_point[1][1] = interpolation_f32(
        paut->dyn[ilow].drcProc.sw_drcT_drcCurve_auto.sw_drcT_anchorPoint_y2,
        paut->dyn[ihigh].drcProc.sw_drcT_drcCurve_auto.sw_drcT_anchorPoint_y2, ratio);
    anchor_point[1][2] = interpolation_f32(
        paut->dyn[ilow].drcProc.sw_drcT_drcCurve_auto.sw_drcT_anchorPoint_y3,
        paut->dyn[ihigh].drcProc.sw_drcT_drcCurve_auto.sw_drcT_anchorPoint_y3, ratio);
    x[0] = 0.0f;
    x[1] =
        (log(anchor_point[0][0] * 4096.0f + (1 << offsetbits_int)) / log(2.0f) - offsetbits_int) /
        log_max;  // 1.3785 / log_max;
    x[2] =
        (log(anchor_point[0][1] * 4096.0f + (1 << offsetbits_int)) / log(2.0f) - offsetbits_int) /
        log_max;  // 2.0704 / log_max;
    x[3] =
        (log(anchor_point[0][2] * 4096.0f + (1 << offsetbits_int)) / log(2.0f) - offsetbits_int) /
        log_max;  // 3.1699 / log_max;
    x[4] = 1.0f;
    y[0] = 0.0f;
    y[1] =
        (log(anchor_point[1][0] * 4096.0f + (1 << offsetbits_int)) / log(2.0f) - offsetbits_int) /
        cmps_max;  // 1.3785 / cmps_max;
    y[2] =
        (log(anchor_point[1][1] * 4096.0f + (1 << offsetbits_int)) / log(2.0f) - offsetbits_int) /
        cmps_max;  // 2.0704 / cmps_max;
    y[3] =
        (log(anchor_point[1][2] * 4096.0f + (1 << offsetbits_int)) / log(2.0f) - offsetbits_int) /
        cmps_max;  // 3.1699 / cmps_max;
    y[4] = 1.0f;
    x[1] = MIN(x[1], 1.0f);
    x[2] = MIN(x[2], 1.0f);
    x[3] = MIN(x[3], 1.0f);
    y[1] = MIN(y[1], 1.0f);
    y[2] = MIN(y[2], 1.0f);
    y[3] = MIN(y[3], 1.0f);
    float cmps_curve[DRC_V12_Y_NUM];
    float input_luma[DRC_V12_Y_NUM];
    for (int i = 0; i < DRC_V12_Y_NUM; ++i) {
        float tmp     = (float)luma2[i] / 24576.0f;
        input_luma[i] = MIN(tmp * luma_scale, 1.0f);
    }
    drc_create_curve(x, y, input_luma, cmps_curve);
    drc_auto_curve_iir_t autoCurve;
    for (int i = 0; i < DRC_V12_Y_NUM; ++i) {
        autoCurve.hw_drcT_hdr2Sdr_curve[i] = cmps_curve[i] * validbits;
        autoCurve.hw_drcT_hdr2Sdr_curve[i] =
            LIMIT_VALUE_UNSIGNED(autoCurve.hw_drcT_hdr2Sdr_curve[i], MANUALCURVEMAX);
    }
    // change min gain for this mode
    float tmp                                 = wp * pDrcCtx->NextData.AEData.L2S_Ratio * adrc_gain;
    tmp                                       = tmp > POW_E_NEG6 ? tmp : POW_E_NEG6;
    autoCurve.hw_drcT_drcGain_minLimit        = MIN(1.0f / tmp, 1.0f);

    // damp
    int iir_frame = interpolation_f32(
        paut->dyn[ilow].drcProc.sw_drcT_drcCurve_auto.sw_drcT_iirFrm_maxLimit,
        paut->dyn[ihigh].drcProc.sw_drcT_drcCurve_auto.sw_drcT_iirFrm_maxLimit, ratio);
    pDrcCtx->CurrData.autoCurveIIRParams.sw_drcT_iirFrm_maxLimit = iir_frame;

    iir_frame = MIN((int)pDrcCtx->FrameID, iir_frame);
    iir_frame = MAX(1, iir_frame);

    bool isDrcCurveModeChange =
        pDrcCtx->CurrData.autoCurveIIRParams.sw_drcT_drcCurve_mode != adrc_auto_mode;

    if (drc_stats->stats_true && !isDrcCurveModeChange) {
        out->dyn.drcProc.hw_drcT_drcGain_minLimit =
            pDrcCtx->CurrData.autoCurveIIRParams.hw_drcT_drcGain_minLimit * (iir_frame - 1) /
                iir_frame +
            autoCurve.hw_drcT_drcGain_minLimit / iir_frame;
        for (int i = 0; i < DRC_V12_Y_NUM; ++i) {
            out->dyn.drcProc.hw_drcT_hdr2Sdr_curve[i] =
                pDrcCtx->CurrData.autoCurveIIRParams.hw_drcT_hdr2Sdr_curve[i] * (iir_frame - 1) /
                    iir_frame +
                autoCurve.hw_drcT_hdr2Sdr_curve[i] / iir_frame;
        }
    } else {
        out->dyn.drcProc.hw_drcT_drcGain_minLimit = autoCurve.hw_drcT_drcGain_minLimit;
        for (int i = 0; i < DRC_V12_Y_NUM; ++i) {
            out->dyn.drcProc.hw_drcT_hdr2Sdr_curve[i] = autoCurve.hw_drcT_hdr2Sdr_curve[i];
        }
    }

#if 0
    printf("iir_frame:%d x[0~1]:%f %f %f %f %f\n", iir_frame, x[0], x[1], x[2], x[3], x[4]);
    printf("wp:%f y[0~1]:%f %f %f %f %f\n", wp, y[0], y[1], y[2], y[3], y[4]);
    printf(
        "%s: cur curve: %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f "
        "%f\n",
        __FUNCTION__, autoCurve.hw_drcT_hdr2Sdr_curve[0], autoCurve.hw_drcT_hdr2Sdr_curve[1],
        autoCurve.hw_drcT_hdr2Sdr_curve[2], autoCurve.hw_drcT_hdr2Sdr_curve[3],
        autoCurve.hw_drcT_hdr2Sdr_curve[4], autoCurve.hw_drcT_hdr2Sdr_curve[5],
        autoCurve.hw_drcT_hdr2Sdr_curve[6], autoCurve.hw_drcT_hdr2Sdr_curve[7],
        autoCurve.hw_drcT_hdr2Sdr_curve[8], autoCurve.hw_drcT_hdr2Sdr_curve[9],
        autoCurve.hw_drcT_hdr2Sdr_curve[10], autoCurve.hw_drcT_hdr2Sdr_curve[11],
        autoCurve.hw_drcT_hdr2Sdr_curve[12], autoCurve.hw_drcT_hdr2Sdr_curve[13],
        autoCurve.hw_drcT_hdr2Sdr_curve[14], autoCurve.hw_drcT_hdr2Sdr_curve[15],
        autoCurve.hw_drcT_hdr2Sdr_curve[16]);
    printf(
        "%s: iir curve: %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f "
        "%f\n",
        __FUNCTION__, pDrcCtx->CurrData.autoCurveIIRParams.hw_drcT_hdr2Sdr_curve[0],
        pDrcCtx->CurrData.autoCurveIIRParams.hw_drcT_hdr2Sdr_curve[1],
        pDrcCtx->CurrData.autoCurveIIRParams.hw_drcT_hdr2Sdr_curve[2],
        pDrcCtx->CurrData.autoCurveIIRParams.hw_drcT_hdr2Sdr_curve[3],
        pDrcCtx->CurrData.autoCurveIIRParams.hw_drcT_hdr2Sdr_curve[4],
        pDrcCtx->CurrData.autoCurveIIRParams.hw_drcT_hdr2Sdr_curve[5],
        pDrcCtx->CurrData.autoCurveIIRParams.hw_drcT_hdr2Sdr_curve[6],
        pDrcCtx->CurrData.autoCurveIIRParams.hw_drcT_hdr2Sdr_curve[7],
        pDrcCtx->CurrData.autoCurveIIRParams.hw_drcT_hdr2Sdr_curve[8],
        pDrcCtx->CurrData.autoCurveIIRParams.hw_drcT_hdr2Sdr_curve[9],
        pDrcCtx->CurrData.autoCurveIIRParams.hw_drcT_hdr2Sdr_curve[10],
        pDrcCtx->CurrData.autoCurveIIRParams.hw_drcT_hdr2Sdr_curve[11],
        pDrcCtx->CurrData.autoCurveIIRParams.hw_drcT_hdr2Sdr_curve[12],
        pDrcCtx->CurrData.autoCurveIIRParams.hw_drcT_hdr2Sdr_curve[13],
        pDrcCtx->CurrData.autoCurveIIRParams.hw_drcT_hdr2Sdr_curve[14],
        pDrcCtx->CurrData.autoCurveIIRParams.hw_drcT_hdr2Sdr_curve[15],
        pDrcCtx->CurrData.autoCurveIIRParams.hw_drcT_hdr2Sdr_curve[16]);
    printf(
        "%s: finnal curve: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d "
        "%d\n",
        __FUNCTION__, out->dyn.drcProc.hw_drcT_hdr2Sdr_curve[0],
        out->dyn.drcProc.hw_drcT_hdr2Sdr_curve[1], out->dyn.drcProc.hw_drcT_hdr2Sdr_curve[2],
        out->dyn.drcProc.hw_drcT_hdr2Sdr_curve[3], out->dyn.drcProc.hw_drcT_hdr2Sdr_curve[4],
        out->dyn.drcProc.hw_drcT_hdr2Sdr_curve[5], out->dyn.drcProc.hw_drcT_hdr2Sdr_curve[6],
        out->dyn.drcProc.hw_drcT_hdr2Sdr_curve[7], out->dyn.drcProc.hw_drcT_hdr2Sdr_curve[8],
        out->dyn.drcProc.hw_drcT_hdr2Sdr_curve[9], out->dyn.drcProc.hw_drcT_hdr2Sdr_curve[10],
        out->dyn.drcProc.hw_drcT_hdr2Sdr_curve[11], out->dyn.drcProc.hw_drcT_hdr2Sdr_curve[12],
        out->dyn.drcProc.hw_drcT_hdr2Sdr_curve[13], out->dyn.drcProc.hw_drcT_hdr2Sdr_curve[14],
        out->dyn.drcProc.hw_drcT_hdr2Sdr_curve[15], out->dyn.drcProc.hw_drcT_hdr2Sdr_curve[16]);
#endif

    pDrcCtx->CurrData.autoCurveIIRParams.hw_drcT_drcGain_minLimit =
        out->dyn.drcProc.hw_drcT_drcGain_minLimit;
    for (int i = 0; i < DRC_V12_Y_NUM; ++i) {
        pDrcCtx->CurrData.autoCurveIIRParams.hw_drcT_hdr2Sdr_curve[i] =
            out->dyn.drcProc.hw_drcT_hdr2Sdr_curve[i];
    }
}

static XCamReturn drcApplyStrength(DrcContext_t* pDrcCtx, drc_param_t* out) {
    bool level_up;
    unsigned int level_diff;

    adrc_strength_t* strg = &pDrcCtx->strg;

    if (strg->darkAreaBoostEn) {
        LOG1_ADEHAZE("darkAreaBoostStrength %d\n", strg->darkAreaBoostStrength);

        level_diff = strg->darkAreaBoostStrength > 50 ? (strg->darkAreaBoostStrength - 50)
                                                      : (50 - strg->darkAreaBoostStrength);
        level_up = strg->darkAreaBoostStrength > 50;
        if (level_up) {
            if (out->dyn.preProc.sw_drcT_toneCurve_mode == drc_cfgCurveCtrlCoeff_mode) {
                out->dyn.preProc.toneCurveCtrl.sw_drcT_toneGain_maxLimit += level_diff * 0.05;
            } else if (out->dyn.preProc.sw_drcT_toneCurve_mode == drc_cfgCurveCtrlCoeff_mode) {
                for (int i = 0; i < 17; i++)
                    out->dyn.preProc.hw_drcT_luma2ToneGain_val[i] +=
                        level_diff * 0.05;
            }
        } else {
            if (out->dyn.preProc.sw_drcT_toneCurve_mode == drc_cfgCurveCtrlCoeff_mode) {
                out->dyn.preProc.toneCurveCtrl.sw_drcT_toneGain_maxLimit -= level_diff * 0.05;
            } else if (out->dyn.preProc.sw_drcT_toneCurve_mode == drc_cfgCurveCtrlCoeff_mode) {
                for (int i = 0; i < 17; i++)
                    out->dyn.preProc.hw_drcT_luma2ToneGain_val[i] -=
                        level_diff * 0.05;
            }
        }
        out->dyn.preProc.toneCurveCtrl.sw_drcT_toneGain_maxLimit =
            LIMIT_VALUE(out->dyn.preProc.toneCurveCtrl.sw_drcT_toneGain_maxLimit, 8.0f, 1.0f);
        for (int i = 0; i < 17; i++)
            out->dyn.preProc.hw_drcT_luma2ToneGain_val[i] = LIMIT_VALUE(
                out->dyn.preProc.hw_drcT_luma2ToneGain_val[i], 8.0f, 1.0f);
    }

    if (strg->hdrStrengthEn) {
        LOG1_ADEHAZE("hdrStrength %d\n", strg->hdrStrength);
        out->dyn.bifilt_filter.hw_drcT_softThd_en = false;
        level_diff = strg->hdrStrength > 50 ? (strg->hdrStrength - 50) : (50 - strg->hdrStrength);
        level_up   = strg->hdrStrength > 50;
        if (level_up) {
            out->dyn.bifilt_filter.hw_drcT_bifiltOut_alpha += level_diff * 0.002;
        } else {
            out->dyn.bifilt_filter.hw_drcT_bifiltOut_alpha -= level_diff * 0.002;
        }
        out->dyn.bifilt_filter.hw_drcT_bifiltOut_alpha =
            LIMIT_VALUE(out->dyn.bifilt_filter.hw_drcT_bifiltOut_alpha, 1.0f, 0.0f);
    }

    return XCAM_RETURN_NO_ERROR;
}
bool DrcDamping(drc_param_t* out, CurrData_t* pCurrData, int FrameID) {
    LOG1_ATMO("%s:Enter!\n", __FUNCTION__);
    bool isDampStable = false;

    // if (FrameID > 2) {
    //     bool isDampStable_DrcGain = false, isDampStable_Alpha = false,
    //          isDampStable_LocalAutoWeitEn = false, isDampStable_LocalWeit = false,
    //          isDampStable_LocalAutoWeit = false, isDampStable_GlobalContrast = false,
    //          isDampStable_LoLitContrast = false;

    //     // isDampStable_DrcGain
    //     if ((out->dyn.preProc.sw_drcT_toneGain_maxLimit -
    //          pCurrData->dynParams.preProc
    //              .sw_drcT_toneGain_maxLimit) <= FLT_EPSILON &&
    //         (out->dyn.preProc.sw_drcT_toneGain_maxLimit -
    //          pCurrData->dynParams.preProc
    //              .sw_drcT_toneGain_maxLimit) >= -FLT_EPSILON) {
    //         isDampStable_DrcGain = true;
    //     } else {
    //         out->dyn.preProc.sw_drcT_toneGain_maxLimit =
    //             out->sta.sw_drc_damp_coef * out->dyn.preProc.sw_drcT_toneGain_maxLimit +
    //             (1.0f - out->sta.sw_drc_damp_coef) * out->dyn.preProc.sw_drcT_toneGain_maxLimit;
    //         isDampStable_DrcGain = false;
    //     }
    //     // isDampStable_Alpha
    //     if ((out->dyn.preProc.hw_drcT_toneCurveIdx_scale -
    //          pCurrData->dynParams.preProc.hw_drcT_toneCurveIdx_scale) <= FLT_EPSILON &&
    //         (out->dyn.preProc.hw_drcT_toneCurveIdx_scale -
    //          pCurrData->dynParams.preProc.hw_drcT_toneCurveIdx_scale) >= -FLT_EPSILON) {
    //         isDampStable_Alpha = true;
    //     } else {
    //         out->dyn.preProc.hw_drcT_toneCurveIdx_scale =
    //             out->sta.sw_drc_damp_coef * out->dyn.preProc.hw_drcT_toneCurveIdx_scale +
    //             (1.0f - out->sta.sw_drc_damp_coef)
    //             * pCurrData->dynParams.preProc.hw_drcT_toneCurveIdx_scale;
    //         isDampStable_Alpha = false;
    //     }
    //     // isDampStable_LocalWeit
    //     if (out->dyn.bifilt_filter.hw_drcT_bifiltOut_alpha ==
    //          pCurrData->dynParams.bifilt_filter.hw_drcT_bifiltOut_alpha) {
    //         isDampStable_LocalWeit = true;
    //     } else {
    //         out->dyn.bifilt_filter.hw_drcT_bifiltOut_alpha =
    //             out->sta.sw_drc_damp_coef * out->dyn.bifilt_filter.hw_drcT_bifiltOut_alpha +
    //             (1.0f - out->sta.sw_drc_damp_coef) *
    //             pCurrData->dynParams.bifilt_filter.hw_drcT_bifiltOut_alpha;
    //         isDampStable_LocalWeit = false;
    //     }
    //     // isDampStable_LocalAutoWeit
    //     if (out->dyn.bifilt_filter.hw_adrc_softThd_thred ==
    //          pCurrData->dynParams.bifilt_filter.hw_adrc_softThd_thred) {
    //         isDampStable_LocalAutoWeit = true;
    //     } else {
    //         out->dyn.bifilt_filter.hw_adrc_softThd_thred =
    //             out->sta.sw_drc_damp_coef * out->dyn.bifilt_filter.hw_adrc_softThd_thred +
    //             (1.0f - out->sta.sw_drc_damp_coef) *
    //             pCurrData->dynParams.bifilt_filter.hw_adrc_softThd_thred;
    //         isDampStable_LocalAutoWeit = false;
    //     }
    //     // isDampStable_LocalAutoWeitEn
    //     if (out->dyn.bifilt_filter.hw_adrc_softThd_en ==
    //          pCurrData->dynParams.bifilt_filter.hw_adrc_softThd_en) {
    //         isDampStable_LocalAutoWeitEn = true;
    //     } else {
    //         out->dyn.bifilt_filter.hw_adrc_softThd_en =
    //             out->sta.sw_drc_damp_coef * out->dyn.bifilt_filter.hw_adrc_softThd_en +
    //             (1.0f - out->sta.sw_drc_damp_coef) *
    //             pCurrData->dynParams.bifilt_filter.hw_adrc_softThd_en;
    //         isDampStable_LocalAutoWeitEn = false;
    //     }
    //     // isDampStable_GlobalContrast
    //     if (out->dyn.drcProc.hw_drcT_drcStrg_alpha ==
    //          pCurrData->dynParams.drcProc.hw_drcT_drcStrg_alpha) {
    //         isDampStable_GlobalContrast = true;
    //     } else {
    //         out->dyn.drcProc.hw_drcT_drcStrg_alpha =
    //             out->sta.sw_drc_damp_coef * out->dyn.drcProc.hw_drcT_drcStrg_alpha +
    //             (1.0f - out->sta.sw_drc_damp_coef) *
    //             pCurrData->dynParams.drcProc.hw_drcT_drcStrg_alpha;
    //         isDampStable_GlobalContrast = false;
    //     }
    //     // isDampStable_LoLitContrast
    //     if (out->dyn.drcProc.hw_drcT_locDetail_strg ==
    //          pCurrData->dynParams.drcProc.hw_drcT_locDetail_strg) {
    //         isDampStable_LoLitContrast = true;
    //     } else {
    //         out->dyn.drcProc.hw_drcT_locDetail_strg =
    //             out->sta.sw_drc_damp_coef * out->dyn.drcProc.hw_drcT_locDetail_strg +
    //             (1.0f - out->sta.sw_drc_damp_coef) *
    //             pCurrData->dynParams.drcProc.hw_drcT_locDetail_strg;
    //         isDampStable_LoLitContrast = false;
    //     }

    //     isDampStable = isDampStable_DrcGain && isDampStable_Alpha && isDampStable_LocalAutoWeitEn
    //     &&
    //                    isDampStable_LocalWeit && isDampStable_LocalAutoWeit &&
    //                    isDampStable_GlobalContrast && isDampStable_LoLitContrast;
    // } else
    //     isDampStable = false;

    // LOG1_ATMO("%s:Eixt!\n", __FUNCTION__);
    return isDampStable;
}

#endif
#if 0
XCamReturn algo_drc_SetAttrib(RkAiqAlgoContext* ctx, drc_api_attrib_t* attr) {
    if (ctx == NULL || attr == NULL) {
        LOGE_ATMO("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    DrcContext_t* pDrcCtx        = (DrcContext_t*)ctx;
    drc_api_attrib_t* drc_attrib = pDrcCtx->drc_attrib;

    if (attr->opMode != RK_AIQ_OP_MODE_AUTO) {
        LOGE_ATMO("not auto mode: %d", attr->opMode);
        return XCAM_RETURN_ERROR_PARAM;
    }

    drc_attrib->opMode = attr->opMode;
    drc_attrib->en     = attr->en;
    drc_attrib->bypass = attr->bypass;

    if (attr->opMode == RK_AIQ_OP_MODE_AUTO)
        drc_attrib->stAuto = attr->stAuto;
    else if (attr->opMode == RK_AIQ_OP_MODE_MANUAL)
        drc_attrib->stMan = attr->stMan;
    else {
        LOGW_ATMO("wrong mode: %d\n", attr->opMode);
    }

    pDrcCtx->isReCal_ = true;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn algo_drc_GetAttrib(RkAiqAlgoContext* ctx, drc_api_attrib_t* attr) {
    if (ctx == NULL || attr == NULL) {
        LOGE_ATMO("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    DrcContext_t* pDrcCtx        = (DrcContext_t*)ctx;
    drc_api_attrib_t* drc_attrib = pDrcCtx->drc_attrib;

#if 0
    if (drc_attrib->opMode != RK_AIQ_OP_MODE_AUTO) {
        LOGE_ATMO("not auto mode: %d", drc_attrib->opMode);
        return XCAM_RETURN_ERROR_PARAM;
    }
#endif

    attr->opMode = drc_attrib->opMode;
    attr->en     = drc_attrib->en;
    attr->bypass = drc_attrib->bypass;
    memcpy(&attr->stAuto, &drc_attrib->stAuto, sizeof(drc_param_auto_t));

    return XCAM_RETURN_NO_ERROR;
}
#endif

XCamReturn algo_drc_SetStrength(RkAiqAlgoContext* ctx, adrc_strength_t* ctrl) {
    if (ctx == NULL || ctrl == NULL) {
        LOGE_ATMO("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    DrcContext_t* pDrcCtx = (DrcContext_t*)ctx;
    pDrcCtx->strg         = *ctrl;
    pDrcCtx->isReCal_     = true;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn algo_drc_GetStrength(RkAiqAlgoContext* ctx, adrc_strength_t* ctrl) {
    if (ctx == NULL || ctrl == NULL) {
        LOGE_ATMO("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    DrcContext_t* pDrcCtx = (DrcContext_t*)ctx;
    *ctrl                 = pDrcCtx->strg;

    return XCAM_RETURN_NO_ERROR;
}

#define RKISP_ALGO_DRC_VERSION     "v0.1.0"
#define RKISP_ALGO_DRC_VENDOR      "Rockchip"
#define RKISP_ALGO_DRC_DESCRIPTION "Rockchip drc algo for ISP2.0"

RkAiqAlgoDescription g_RkIspAlgoDescDrc = {
    .common =
        {
            .version         = RKISP_ALGO_DRC_VERSION,
            .vendor          = RKISP_ALGO_DRC_VENDOR,
            .description     = RKISP_ALGO_DRC_DESCRIPTION,
            .type            = RK_AIQ_ALGO_TYPE_ADRC,
            .id              = 0,
            .create_context  = create_context,
            .destroy_context = destroy_context,
        },
    .prepare      = prepare,
    .pre_process  = NULL,
    .processing   = processing,
    .post_process = NULL,
};

// RKAIQ_END_DECLARE
