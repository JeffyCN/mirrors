/******************************************************************************
 *
 * Copyright 2014, Fuzhou Rockchip Electronics Co.Ltd. All rights reserved.
 * No part of this work may be reproduced, modified, distributed, transmitted,
 * transcribed, or translated into any language or computer format, in any form
 * or by any means without written permission of:
 * Fuzhou Rockchip Electronics Co.Ltd .
 *
 *
 *****************************************************************************/
/**
 * @file rk_aiq_adrc_algo_v20.cpp
 *
 * @brief
 *   ADD_DESCRIPTION_HERE
 *
 *****************************************************************************/
#include "math.h"
#include "rk_aiq_types_adrc_algo_prvt.h"
#include "xcam_log.h"

int adrcClipFloatValueV20(float posx, int BitInt, int BitFloat, bool fullMax) {
    int yOutInt    = 0;
    int yOutIntMin = 0;
    int yOutIntMax = 0;
    if (fullMax)
        yOutIntMax = 1 << (BitFloat + BitInt);
    else
        yOutIntMax = (1 << (BitFloat + BitInt)) - 1;

    yOutInt = LIMIT_VALUE((int)(posx * (1 << BitFloat)), yOutIntMax, yOutIntMin);

    return yOutInt;
}

/******************************************************************************
 * AdrcStart()
 *****************************************************************************/
XCamReturn AdrcStart(AdrcContext_t* pAdrcCtx) {
    LOG1_ATMO("%s:enter!\n", __FUNCTION__);

    // initial checks
    if (pAdrcCtx == NULL) {
        return (XCAM_RETURN_ERROR_MEM);
    }

    if ((ADRC_STATE_RUNNING == pAdrcCtx->state) || (ADRC_STATE_LOCKED == pAdrcCtx->state)) {
        return (XCAM_RETURN_ERROR_PARAM);
    }

    pAdrcCtx->state = ADRC_STATE_RUNNING;

    LOG1_ATMO("%s:exit!\n", __FUNCTION__);
    return (XCAM_RETURN_NO_ERROR);
}

float DrcGetInterpRatioV20(float* pX, int& lo, int& hi, float CtrlValue, int length_max) {
    float ratio = 0.0f;

    if (CtrlValue < pX[0]) {
        lo    = 0;
        hi    = 0;
        ratio = 0.0f;
    } else if (CtrlValue >= pX[length_max - 1]) {
        lo    = length_max - 1;
        hi    = length_max - 1;
        ratio = 0.0f;
    } else {
        for (int i = 0; i < length_max - 1; i++) {
            if (CtrlValue >= pX[i] && CtrlValue < pX[i + 1]) {
                lo    = i;
                hi    = i + 1;
                ratio = (CtrlValue - pX[i]) / (pX[i + 1] - pX[i]);
                break;
            } else
                continue;
        }
    }

    return ratio;
}

/******************************************************************************
 * AdrcStop()
 *****************************************************************************/
XCamReturn AdrcStop(AdrcContext_t* pAdrcCtx) {
    LOG1_ATMO("%s:enter!\n", __FUNCTION__);

    // initial checks
    if (pAdrcCtx == NULL) {
        return (XCAM_RETURN_ERROR_MEM);
    }

    // before stopping, unlock the AHDR if locked
    if (ADRC_STATE_LOCKED == pAdrcCtx->state) {
        return (XCAM_RETURN_ERROR_PARAM);
    }

    pAdrcCtx->state = ADRC_STATE_STOPPED;

    LOG1_ATMO("%s:exit!\n", __FUNCTION__);

    return (XCAM_RETURN_NO_ERROR);
}

void AdrcV20ClipStAutoParams(AdrcContext_t* pAdrcCtx) {
    LOG1_ATMO("%s:enter!\n", __FUNCTION__);

    /*
for (int i = 0; i < ADRC_ENVLV_STEP_MAX; i++) {

    pAdrcCtx->drcAttrV20.stAuto.DrcTuningPara.DrcGain.DrcGain[i] = LIMIT_VALUE(
        pAdrcCtx->drcAttrV20.stAuto.DrcTuningPara.DrcGain.DrcGain[i], DRCGAINMAX, DRCGAINMIN);
    pAdrcCtx->drcAttrV20.stAuto.DrcTuningPara.DrcGain.Alpha[i] =
        LIMIT_VALUE(pAdrcCtx->drcAttrV20.stAuto.DrcTuningPara.DrcGain.Alpha[i], NORMALIZE_MAX,
                    NORMALIZE_MIN);
    pAdrcCtx->drcAttrV20.stAuto.DrcTuningPara.DrcGain.Clip[i] = LIMIT_VALUE(
        pAdrcCtx->drcAttrV20.stAuto.DrcTuningPara.DrcGain.Clip[i], CLIPMAX, CLIPMIN);

    pAdrcCtx->drcAttrV20.stAuto.DrcTuningPara.HiLight.HiLightData.Strength[i] =
        LIMIT_VALUE(pAdrcCtx->drcAttrV20.stAuto.DrcTuningPara.HiLight.HiLightData.Strength[i],
                    NORMALIZE_MAX, NORMALIZE_MIN);
    pAdrcCtx->drcAttrV20.stAuto.DrcTuningPara.HiLight.HiLightData.gas_t[i] =
        LIMIT_VALUE(pAdrcCtx->drcAttrV20.stAuto.DrcTuningPara.HiLight.HiLightData.gas_t[i],
                    GAS_T_MAX, GAS_T_MIN);

    pAdrcCtx->drcAttrV20.stAuto.DrcTuningPara.LocalSetting.hw_adrc_biflt_params.LocalWeit[i] =
        LIMIT_VALUE(pAdrcCtx->drcAttrV20.stAuto.DrcTuningPara.LocalSetting.hw_adrc_biflt_params
                        .LocalWeit[i],
                    NORMALIZE_MAX, NORMALIZE_MIN);
    pAdrcCtx->drcAttrV20.stAuto.DrcTuningPara.LocalSetting.hw_adrc_biflt_params
        .GlobalContrast[i] = LIMIT_VALUE(pAdrcCtx->drcAttrV20.stAuto.DrcTuningPara.LocalSetting
                                             .hw_adrc_biflt_params.GlobalContrast[i],
                                         NORMALIZE_MAX, NORMALIZE_MIN);
    pAdrcCtx->drcAttrV20.stAuto.DrcTuningPara.LocalSetting.hw_adrc_biflt_params
        .LoLitContrast[i] = LIMIT_VALUE(pAdrcCtx->drcAttrV20.stAuto.DrcTuningPara.LocalSetting
                                            .hw_adrc_biflt_params.LoLitContrast[i],
                                        NORMALIZE_MAX, NORMALIZE_MIN);
    pAdrcCtx->drcAttrV20.stAuto.DrcTuningPara.LocalSetting.hw_adrc_biflt_params
        .LocalAutoEnable[i] = LIMIT_VALUE(pAdrcCtx->drcAttrV20.stAuto.DrcTuningPara.LocalSetting
                                              .hw_adrc_biflt_params.LocalAutoEnable[i],
                                          NORMALIZE_MAX, NORMALIZE_MIN);
    pAdrcCtx->drcAttrV20.stAuto.DrcTuningPara.LocalSetting.hw_adrc_biflt_params
        .LocalAutoWeit[i] = LIMIT_VALUE(pAdrcCtx->drcAttrV20.stAuto.DrcTuningPara.LocalSetting
                                            .hw_adrc_biflt_params.LocalAutoWeit[i],
                                        NORMALIZE_MAX, NORMALIZE_MIN);

    pAdrcCtx->drcAttrV20.stAuto.DrcTuningPara.LocalSetting.MotionData.MotionCoef[i] =
        LIMIT_VALUE(
            pAdrcCtx->drcAttrV20.stAuto.DrcTuningPara.LocalSetting.MotionData.MotionCoef[i],
            NORMALIZE_MAX, NORMALIZE_MIN);
    pAdrcCtx->drcAttrV20.stAuto.DrcTuningPara.LocalSetting.MotionData.MotionStr[i] =
        LIMIT_VALUE(
            pAdrcCtx->drcAttrV20.stAuto.DrcTuningPara.LocalSetting.MotionData.MotionStr[i],
            NORMALIZE_MAX, NORMALIZE_MIN);
}

pAdrcCtx->drcAttrV20.stAuto.DrcTuningPara.LocalSetting.curPixWeit =
    LIMIT_VALUE(pAdrcCtx->drcAttrV20.stAuto.DrcTuningPara.LocalSetting.curPixWeit,
                NORMALIZE_MAX, NORMALIZE_MIN);

#if RKAIQ_HAVE_DRC_V12
pAdrcCtx->drcAttrV20.stAuto.DrcTuningPara.LocalSetting.preFrameWeit =
    LIMIT_VALUE(pAdrcCtx->drcAttrV20.stAuto.DrcTuningPara.LocalSetting.preFrameWeit,
                NORMALIZE_MAX, NORMALIZE_MIN);
pAdrcCtx->drcAttrV20.stAuto.DrcTuningPara.LocalSetting.Range_sgm_pre =
    LIMIT_VALUE(pAdrcCtx->drcAttrV20.stAuto.DrcTuningPara.LocalSetting.Range_sgm_pre,
                NORMALIZE_MAX, NORMALIZE_MIN);
pAdrcCtx->drcAttrV20.stAuto.DrcTuningPara.LocalSetting.Space_sgm_pre =
    LIMIT_VALUE(pAdrcCtx->drcAttrV20.stAuto.DrcTuningPara.LocalSetting.Space_sgm_pre,
                SPACESGMMAX, SPACESGMMIN);
#endif
pAdrcCtx->drcAttrV20.stAuto.DrcTuningPara.LocalSetting.Range_force_sgm =
    LIMIT_VALUE(pAdrcCtx->drcAttrV20.stAuto.DrcTuningPara.LocalSetting.Range_force_sgm,
                NORMALIZE_MAX, NORMALIZE_MIN);
pAdrcCtx->drcAttrV20.stAuto.DrcTuningPara.LocalSetting.Range_sgm_cur =
    LIMIT_VALUE(pAdrcCtx->drcAttrV20.stAuto.DrcTuningPara.LocalSetting.Range_sgm_cur,
                NORMALIZE_MAX, NORMALIZE_MIN);
pAdrcCtx->drcAttrV20.stAuto.DrcTuningPara.LocalSetting.Space_sgm_cur =
    LIMIT_VALUE(pAdrcCtx->drcAttrV20.stAuto.DrcTuningPara.LocalSetting.Space_sgm_cur,
                SPACESGMMAX, SPACESGMMIN);
for (int i = 0; i < ADRC_Y_NUM; i++) {
    pAdrcCtx->drcAttrV20.stAuto.DrcTuningPara.CompressSetting.Manual_curve[i] =
        LIMIT_VALUE(pAdrcCtx->drcAttrV20.stAuto.DrcTuningPara.CompressSetting.Manual_curve[i],
                    SCALEYMAX, SCALEYMIN);
    pAdrcCtx->drcAttrV20.stAuto.DrcTuningPara.Scale_y[i] =
        LIMIT_VALUE(pAdrcCtx->drcAttrV20.stAuto.DrcTuningPara.Scale_y[i], SCALEYMAX, SCALEYMIN);
}

pAdrcCtx->drcAttrV20.stAuto.DrcTuningPara.ByPassThr = LIMIT_VALUE(
    pAdrcCtx->drcAttrV20.stAuto.DrcTuningPara.ByPassThr, NORMALIZE_MAX, NORMALIZE_MIN);
pAdrcCtx->drcAttrV20.stAuto.DrcTuningPara.Edge_Weit = LIMIT_VALUE(
    pAdrcCtx->drcAttrV20.stAuto.DrcTuningPara.Edge_Weit, NORMALIZE_MAX, NORMALIZE_MIN);
pAdrcCtx->drcAttrV20.stAuto.DrcTuningPara.IIR_frame =
    LIMIT_VALUE(pAdrcCtx->drcAttrV20.stAuto.DrcTuningPara.IIR_frame, IIRFRAMEMAX, IIRFRAMEMIN);
pAdrcCtx->drcAttrV20.stAuto.DrcTuningPara.damp =
    LIMIT_VALUE(pAdrcCtx->drcAttrV20.stAuto.DrcTuningPara.damp, NORMALIZE_MAX, NORMALIZE_MIN);
    */

    LOG1_ATMO("%s:exit!\n", __FUNCTION__);
}

/******************************************************************************
 * CalibrateDrcGainYV20()
 *****************************************************************************/
void CalibrateDrcGainYV20(DrcProcRes_t* para, float DrcGain, float alpha, bool OB_enable,
                          float predgain) {
    LOG1_ATMO("%s:Enter!\n", __FUNCTION__);

    float tmp = 0.0f;
    /*luma[i] = pow((1.0f - luma[i] / 4096.0f), 2.0f)*/
    float luma[DRC_V20_Y_NUM] = {1.0f,    0.8789f, 0.7656f, 0.6602f, 0.5625f, 0.4727f,
                                 0.3906f, 0.3164f, 0.2500f, 0.1914f, 0.1406f, 0.0977f,
                                 0.0625f, 0.0352f, 0.0156f, 0.0039f, 0.0f};

    for (int i = 0; i < DRC_V20_Y_NUM; ++i) {
        if (OB_enable)
            tmp = 1024.0f * pow(DrcGain, 1.0f - alpha * luma[i]) * pow(predgain, -alpha * luma[i]);
        else
            tmp = 1024.0f * pow(DrcGain, 1.0f - alpha * luma[i]);
        para->Drc_v20.gain_y[i] = (unsigned short)(tmp);
    }

    LOG1_ATMO("%s:Eixt!\n", __FUNCTION__);
}

/******************************************************************************
 * DrcEnableSetting()
 *
 *****************************************************************************/
bool DrcEnableSetting(AdrcContext_t* pAdrcCtx, RkAiqAdrcProcResult_t* pAdrcProcRes) {
    LOG1_ATMO("%s:enter!\n", __FUNCTION__);

    if (pAdrcCtx->FrameNumber == HDR_2X_NUM || pAdrcCtx->FrameNumber == HDR_3X_NUM ||
        pAdrcCtx->FrameNumber == SENSOR_MGE)
        pAdrcProcRes->bDrcEn = true;
    else if (pAdrcCtx->FrameNumber == LINEAR_NUM) {
        if (pAdrcCtx->ablcV32_proc_res.blc_ob_enable)
            pAdrcProcRes->bDrcEn = true;
        else {
            if (pAdrcCtx->drcAttrV20.opMode == DRC_OPMODE_AUTO) {
                pAdrcProcRes->bDrcEn = pAdrcCtx->drcAttrV20.stAuto.DrcTuningPara.hw_adrc_en;
            } else if (pAdrcCtx->drcAttrV20.opMode == DRC_OPMODE_MANUAL) {
                pAdrcProcRes->bDrcEn = pAdrcCtx->drcAttrV20.stManual.hw_adrc_en;
            } else {
                LOGE_ATMO("%s: Drc api in WRONG MODE!!!, drc by pass!!!\n", __FUNCTION__);
                pAdrcProcRes->bDrcEn = false;
            }
        }
    }
    pAdrcProcRes->DrcProcRes.Drc_v20.cmps_byp_en =
        pAdrcProcRes->bDrcEn ? FUNCTION_DISABLE : FUNCTION_ENABLE;
    // store bDrcEn for AdrcV12Params2Api
    pAdrcCtx->NextData.dynParams.Drc_v20.hw_adrc_en = pAdrcProcRes->bDrcEn;

    return pAdrcProcRes->bDrcEn;
    LOG1_ATMO("%s:exit!\n", __FUNCTION__);
}
/******************************************************************************
 * AdrcDampingV20()
 *****************************************************************************/
bool AdrcDampingV20(NextData_t* pNextData, CurrData_t* pCurrData, int FrameID) {
    LOG1_ATMO("%s:Enter!\n", __FUNCTION__);
    bool isDampStable = false;

    if (FrameID > INIT_CALC_PARAMS_NUM) {
        bool isDampStable_DrcGain = false, isDampStable_Alpha = false,
             isDampStable_LocalAutoWeitEn = false, isDampStable_LocalWeit = false,
             isDampStable_LocalAutoWeit = false, isDampStable_GlobalContrast = false,
             isDampStable_LoLitContrast = false;

        // isDampStable_DrcGain
        if ((pNextData->dynParams.Drc_v20.hw_adrc_luma2drcGain_setting.sw_adrc_luma2drcGain_autoVal
                 .sw_adrc_drcGain_maxLimit -
             pNextData->dynParams.Drc_v20.hw_adrc_luma2drcGain_setting.sw_adrc_luma2drcGain_autoVal
                 .sw_adrc_drcGain_maxLimit) <= FLT_EPSILON &&
            (pNextData->dynParams.Drc_v20.hw_adrc_luma2drcGain_setting.sw_adrc_luma2drcGain_autoVal
                 .sw_adrc_drcGain_maxLimit -
             pNextData->dynParams.Drc_v20.hw_adrc_luma2drcGain_setting.sw_adrc_luma2drcGain_autoVal
                 .sw_adrc_drcGain_maxLimit) >= -FLT_EPSILON) {
            isDampStable_DrcGain = true;
        } else {
            pNextData->dynParams.Drc_v20.hw_adrc_luma2drcGain_setting.sw_adrc_luma2drcGain_autoVal
                .sw_adrc_drcGain_maxLimit =
                pNextData->dynParams.damp *
                    pNextData->dynParams.Drc_v20.hw_adrc_luma2drcGain_setting
                        .sw_adrc_luma2drcGain_autoVal.sw_adrc_drcGain_maxLimit +
                (1.0f - pNextData->dynParams.damp) *
                    pNextData->dynParams.Drc_v20.hw_adrc_luma2drcGain_setting
                        .sw_adrc_luma2drcGain_autoVal.sw_adrc_drcGain_maxLimit;
            isDampStable_DrcGain = false;
        }
        // isDampStable_Alpha
        if ((pNextData->dynParams.Drc_v20.hw_adrc_luma2drcGain_setting.sw_adrc_luma2drcGain_autoVal
                 .sw_adrc_drcGainLumaAdj_scale -
             pCurrData->dynParams.Drc_v20.hw_adrc_luma2drcGain_setting.sw_adrc_luma2drcGain_autoVal
                 .sw_adrc_drcGainLumaAdj_scale) <= FLT_EPSILON &&
            (pNextData->dynParams.Drc_v20.hw_adrc_luma2drcGain_setting.sw_adrc_luma2drcGain_autoVal
                 .sw_adrc_drcGainLumaAdj_scale -
             pCurrData->dynParams.Drc_v20.hw_adrc_luma2drcGain_setting.sw_adrc_luma2drcGain_autoVal
                 .sw_adrc_drcGainLumaAdj_scale) >= -FLT_EPSILON) {
            isDampStable_Alpha = true;
        } else {
            pNextData->dynParams.Drc_v20.hw_adrc_luma2drcGain_setting.sw_adrc_luma2drcGain_autoVal
                .sw_adrc_drcGainLumaAdj_scale =
                pNextData->dynParams.damp *
                    pNextData->dynParams.Drc_v20.hw_adrc_luma2drcGain_setting
                        .sw_adrc_luma2drcGain_autoVal.sw_adrc_drcGainLumaAdj_scale +
                (1.0f - pNextData->dynParams.damp) *
                    pCurrData->dynParams.Drc_v20.hw_adrc_luma2drcGain_setting
                        .sw_adrc_luma2drcGain_autoVal.sw_adrc_drcGainLumaAdj_scale;
            isDampStable_Alpha = false;
        }
        // isDampStable_LocalWeit
        if (pNextData->dynParams.Drc_v20.hw_adrc_biflt_setting.hw_adrc_biflt_params
                .hw_adrc_bifilt_wgt == pCurrData->dynParams.Drc_v20.hw_adrc_biflt_setting
                                           .hw_adrc_biflt_params.hw_adrc_bifilt_wgt) {
            isDampStable_LocalWeit = true;
        } else {
            pNextData->dynParams.Drc_v20.hw_adrc_biflt_setting.hw_adrc_biflt_params
                .hw_adrc_bifilt_wgt =
                pNextData->dynParams.damp * pNextData->dynParams.Drc_v20.hw_adrc_biflt_setting
                                                .hw_adrc_biflt_params.hw_adrc_bifilt_wgt +
                (1.0f - pNextData->dynParams.damp) *
                    pCurrData->dynParams.Drc_v20.hw_adrc_biflt_setting.hw_adrc_biflt_params
                        .hw_adrc_bifilt_wgt;
            isDampStable_LocalWeit = false;
        }
        // isDampStable_LocalAutoWeit
        if (pNextData->dynParams.Drc_v20.hw_adrc_biflt_setting.hw_adrc_biflt_params
                .hw_adrc_bifilt_softThred == pCurrData->dynParams.Drc_v20.hw_adrc_biflt_setting
                                                 .hw_adrc_biflt_params.hw_adrc_bifilt_softThred) {
            isDampStable_LocalAutoWeit = true;
        } else {
            pNextData->dynParams.Drc_v20.hw_adrc_biflt_setting.hw_adrc_biflt_params
                .hw_adrc_bifilt_softThred =
                pNextData->dynParams.damp * pNextData->dynParams.Drc_v20.hw_adrc_biflt_setting
                                                .hw_adrc_biflt_params.hw_adrc_bifilt_softThred +
                (1.0f - pNextData->dynParams.damp) *
                    pCurrData->dynParams.Drc_v20.hw_adrc_biflt_setting.hw_adrc_biflt_params
                        .hw_adrc_bifilt_softThred;
            isDampStable_LocalAutoWeit = false;
        }
        // isDampStable_LocalAutoWeitEn
        if (pNextData->dynParams.Drc_v20.hw_adrc_biflt_setting.hw_adrc_biflt_params
                .hw_adrc_bifiltSoftThred_en ==
            pCurrData->dynParams.Drc_v20.hw_adrc_biflt_setting.hw_adrc_biflt_params
                .hw_adrc_bifiltSoftThred_en) {
            isDampStable_LocalAutoWeitEn = true;
        } else {
            pNextData->dynParams.Drc_v20.hw_adrc_biflt_setting.hw_adrc_biflt_params
                .hw_adrc_bifiltSoftThred_en =
                pNextData->dynParams.damp * pNextData->dynParams.Drc_v20.hw_adrc_biflt_setting
                                                .hw_adrc_biflt_params.hw_adrc_bifiltSoftThred_en +
                (1.0f - pNextData->dynParams.damp) *
                    pCurrData->dynParams.Drc_v20.hw_adrc_biflt_setting.hw_adrc_biflt_params
                        .hw_adrc_bifiltSoftThred_en;
            isDampStable_LocalAutoWeitEn = false;
        }
        // isDampStable_GlobalContrast
        if (pNextData->dynParams.Drc_v20.hw_adrc_adjCompsGain_setting.hw_adrc_adjCompsGain_params
                .hw_adrc_hiDetail_ratio ==
            pCurrData->dynParams.Drc_v20.hw_adrc_adjCompsGain_setting.hw_adrc_adjCompsGain_params
                .hw_adrc_hiDetail_ratio) {
            isDampStable_GlobalContrast = true;
        } else {
            pNextData->dynParams.Drc_v20.hw_adrc_adjCompsGain_setting.hw_adrc_adjCompsGain_params
                .hw_adrc_hiDetail_ratio =
                pNextData->dynParams.damp *
                    pNextData->dynParams.Drc_v20.hw_adrc_adjCompsGain_setting
                        .hw_adrc_adjCompsGain_params.hw_adrc_hiDetail_ratio +
                (1.0f - pNextData->dynParams.damp) *
                    pCurrData->dynParams.Drc_v20.hw_adrc_adjCompsGain_setting
                        .hw_adrc_adjCompsGain_params.hw_adrc_hiDetail_ratio;
            isDampStable_GlobalContrast = false;
        }
        // isDampStable_LoLitContrast
        if (pNextData->dynParams.Drc_v20.hw_adrc_adjCompsGain_setting.hw_adrc_adjCompsGain_params
                .hw_adrc_loDetail_ratio ==
            pCurrData->dynParams.Drc_v20.hw_adrc_adjCompsGain_setting.hw_adrc_adjCompsGain_params
                .hw_adrc_loDetail_ratio) {
            isDampStable_LoLitContrast = true;
        } else {
            pNextData->dynParams.Drc_v20.hw_adrc_adjCompsGain_setting.hw_adrc_adjCompsGain_params
                .hw_adrc_loDetail_ratio =
                pNextData->dynParams.damp *
                    pNextData->dynParams.Drc_v20.hw_adrc_adjCompsGain_setting
                        .hw_adrc_adjCompsGain_params.hw_adrc_loDetail_ratio +
                (1.0f - pNextData->dynParams.damp) *
                    pCurrData->dynParams.Drc_v20.hw_adrc_adjCompsGain_setting
                        .hw_adrc_adjCompsGain_params.hw_adrc_loDetail_ratio;
            isDampStable_LoLitContrast = false;
        }

        isDampStable = isDampStable_DrcGain && isDampStable_Alpha && isDampStable_LocalAutoWeitEn &&
                       isDampStable_LocalWeit && isDampStable_LocalAutoWeit &&
                       isDampStable_GlobalContrast && isDampStable_LoLitContrast;
    } else
        isDampStable = false;

    LOG1_ATMO("%s:Eixt!\n", __FUNCTION__);
    return isDampStable;
}

/******************************************************************************
 * AdrcGetTuningProcResV20()
 *****************************************************************************/
void AdrcGetTuningProcResV20(mdrcAttr_V20_t* pdynParamsV20, adrc_blcRes_V32_t* pObData,
                             RkAiqAdrcProcResult_t* pAdrcProcRes) {
    LOG1_ATMO("%s:enter!\n", __FUNCTION__);

    pAdrcProcRes->DrcProcRes.Drc_v20.bypass_en   = FUNCTION_DISABLE;
    pAdrcProcRes->DrcProcRes.Drc_v20.gainx32_en  = FUNCTION_DISABLE;
    pAdrcProcRes->DrcProcRes.Drc_v20.raw_dly_dis = FUNCTION_DISABLE;
    pAdrcProcRes->DrcProcRes.Drc_v20.position    = adrcClipFloatValueV20(
        pdynParamsV20->hw_adrc_luma2drcGain_setting.hw_adrc_drcGainIdxLuma_scale, 6, 8, false);
    pAdrcProcRes->DrcProcRes.Drc_v20.offset_pow2 = adrcClipFloatValueV20(
        pdynParamsV20->hw_adrc_luma2compsLuma_setting.hw_adrc_logTransform_offset, 4, 0, false);
    pAdrcProcRes->DrcProcRes.Drc_v20.lpdetail_ratio =
        LIMIT_VALUE_UNSIGNED(pdynParamsV20->hw_adrc_adjCompsGain_setting.hw_adrc_adjCompsGain_params
                                 .hw_adrc_loDetail_ratio,
                             BIT_12_MAX);
    pAdrcProcRes->DrcProcRes.Drc_v20.hpdetail_ratio =
        LIMIT_VALUE_UNSIGNED(pdynParamsV20->hw_adrc_adjCompsGain_setting.hw_adrc_adjCompsGain_params
                                 .hw_adrc_hiDetail_ratio,
                             BIT_12_MAX);
    pAdrcProcRes->DrcProcRes.Drc_v20.delta_scalein =
        pdynParamsV20->hw_adrc_adjCompsGain_setting.hw_adrc_adjCompsGainIdxLuma_scl;
    pAdrcProcRes->DrcProcRes.Drc_v20.bilat_wt_off =
        pdynParamsV20->hw_adrc_biflt_setting.hw_adrc_bifilt_wgtOffset;
    pAdrcProcRes->DrcProcRes.Drc_v20.thumb_thd_neg = LIMIT_VALUE_UNSIGNED(
        pdynParamsV20->hw_adrc_biflt_setting.hw_adrc_thumb_setting.hw_adrc_thumbThred_neg,
        BIT_9_MAX);
    pAdrcProcRes->DrcProcRes.Drc_v20.thumb_thd_enable =
        pdynParamsV20->hw_adrc_biflt_setting.hw_adrc_thumb_setting.hw_adrc_thumbThred_en
            ? FUNCTION_ENABLE
            : FUNCTION_DISABLE;
    pAdrcProcRes->DrcProcRes.Drc_v20.weicur_pix =
        pdynParamsV20->hw_adrc_biflt_setting.hw_adrc_bifilt_curPixelWgt;
    pAdrcProcRes->DrcProcRes.Drc_v20.cmps_offset_bits_int =
        LIMIT_VALUE_UNSIGNED(pdynParamsV20->hw_adrc_cmps_setting.hw_adrc_comps_offset, BIT_4_MAX);
    pAdrcProcRes->DrcProcRes.Drc_v20.cmps_fixbit_mode =
        pdynParamsV20->hw_adrc_cmps_setting.hw_adrc_comps_mode == COMPS_MODE_LOG_I3F9
            ? FUNCTION_ENABLE
            : FUNCTION_DISABLE;
    pAdrcProcRes->DrcProcRes.Drc_v20.drc_gas_t =
        LIMIT_VALUE_UNSIGNED(pdynParamsV20->hw_adrc_biflt_setting.hw_adrc_gas_t, BIT_10_MAX);
    pAdrcProcRes->DrcProcRes.Drc_v20.thumb_clip = LIMIT_VALUE_UNSIGNED(
        pdynParamsV20->hw_adrc_biflt_setting.hw_adrc_thumb_setting.hw_adrc_thumb_maxLimit,
        BIT_12_MAX);
    pAdrcProcRes->DrcProcRes.Drc_v20.thumb_scale =
        pdynParamsV20->hw_adrc_biflt_setting.hw_adrc_thumb_setting.hw_adrc_thumb_scale;
    pAdrcProcRes->DrcProcRes.Drc_v20.range_sgm_inv0 = LIMIT_VALUE_UNSIGNED(
        pdynParamsV20->hw_adrc_biflt_setting.hw_adrc_hiRange_invSigma, BIT_10_MAX);
    pAdrcProcRes->DrcProcRes.Drc_v20.range_sgm_inv1 = LIMIT_VALUE_UNSIGNED(
        pdynParamsV20->hw_adrc_biflt_setting.hw_adrc_loRange_invSigma, BIT_10_MAX);
    pAdrcProcRes->DrcProcRes.Drc_v20.weig_bilat = LIMIT_VALUE_UNSIGNED(
        pdynParamsV20->hw_adrc_biflt_setting.hw_adrc_biflt_params.hw_adrc_bifilt_wgt,
        BIT_4_MAX + 1);
    pAdrcProcRes->DrcProcRes.Drc_v20.weight_8x8thumb =
        pdynParamsV20->hw_adrc_biflt_setting.hw_adrc_bifilt_hiWgt;
    pAdrcProcRes->DrcProcRes.Drc_v20.bilat_soft_thd = LIMIT_VALUE_UNSIGNED(
        pdynParamsV20->hw_adrc_biflt_setting.hw_adrc_biflt_params.hw_adrc_bifilt_softThred,
        BIT_11_MAX);
    pAdrcProcRes->DrcProcRes.Drc_v20.enable_soft_thd =
        pdynParamsV20->hw_adrc_biflt_setting.hw_adrc_biflt_params.hw_adrc_bifiltSoftThred_en;
    // get sw_drc_gain_y
    if (pdynParamsV20->hw_adrc_luma2drcGain_setting.sw_adrc_drcGain_mode == DRC_GAIN_TABLE_AUTO) {
        CalibrateDrcGainYV20(&pAdrcProcRes->DrcProcRes,
                             pdynParamsV20->hw_adrc_luma2drcGain_setting
                                 .sw_adrc_luma2drcGain_autoVal.sw_adrc_drcGain_maxLimit,
                             pdynParamsV20->hw_adrc_luma2drcGain_setting
                                 .sw_adrc_luma2drcGain_autoVal.sw_adrc_drcGainLumaAdj_scale,
                             pObData->blc_ob_enable, pObData->isp_ob_predgain);
    } else if (pdynParamsV20->hw_adrc_luma2drcGain_setting.sw_adrc_drcGain_mode ==
               DRC_GAIN_TABLE_MANUAL) {
        for (int i = 0; i < DRC_V20_Y_NUM; ++i) {
                pAdrcProcRes->DrcProcRes.Drc_v20.gain_y[i] =
                    pdynParamsV20->hw_adrc_luma2drcGain_setting.hw_adrc_luma2drcGain_manualVal[i];
        }
    }

    for (int i = 0; i < DRC_V20_Y_NUM; ++i) {
        pAdrcProcRes->DrcProcRes.Drc_v20.scale_y[i] =
            (unsigned short)(pdynParamsV20->hw_adrc_adjCompsGain_setting
                                 .hw_adrc_luma2compsGainScale_val[i]);
    }

    if (pdynParamsV20->hw_adrc_biflt_setting.hw_adrc_thumb_setting.thumb_thd_table_setting
            .sw_adrc_autoCurve_mode == THUMB_THD_TABLE_AUTO) {
        float tmp = 0.0f;
        for (int i = 0; i < DRC_V20_Y_NUM; ++i) {
            tmp =
                pdynParamsV20->hw_adrc_biflt_setting.hw_adrc_thumb_setting.thumb_thd_table_setting
                    .sw_adrc_thumbLumaDiff2thrd_aVal.sw_adrc_thumbThred_maxLimit -
                (pdynParamsV20->hw_adrc_biflt_setting.hw_adrc_thumb_setting.thumb_thd_table_setting
                     .sw_adrc_thumbLumaDiff2thrd_aVal.sw_adrc_thumbThred_maxLimit -
                 pdynParamsV20->hw_adrc_biflt_setting.hw_adrc_thumb_setting.thumb_thd_table_setting
                     .sw_adrc_thumbLumaDiff2thrd_aVal.sw_adrc_thumbThred_minLimit) /
                    (1.0f +
                     pow(2.718f, -pdynParamsV20->hw_adrc_biflt_setting.hw_adrc_thumb_setting
                                         .thumb_thd_table_setting.sw_adrc_thumbLumaDiff2thrd_aVal
                                         .sw_adrc_thumbThredCurve_slope *
                                     (i / 16.0f -
                                      pdynParamsV20->hw_adrc_biflt_setting.hw_adrc_thumb_setting
                                          .thumb_thd_table_setting.sw_adrc_thumbLumaDiff2thrd_aVal
                                          .sw_adrc_thumbThredCurve_offset)));
            pAdrcProcRes->DrcProcRes.Drc_v20.sfthd_y[i] = (unsigned short)(tmp * (BIT_8_MAX + 1));
        }
    } else if (pdynParamsV20->hw_adrc_biflt_setting.hw_adrc_thumb_setting.thumb_thd_table_setting
                   .sw_adrc_autoCurve_mode == THUMB_THD_TABLE_MANUAL) {
        for (int i = 0; i < DRC_V20_Y_NUM; ++i) {
            pAdrcProcRes->DrcProcRes.Drc_v20.sfthd_y[i] =
                (unsigned short)(pdynParamsV20->hw_adrc_biflt_setting.hw_adrc_thumb_setting
                                     .thumb_thd_table_setting.hw_adrc_thumbLumaDiff2thrd_mVal[i]);
        }
    }

    LOG1_ATMO("%s:exit!\n", __FUNCTION__);
}

void AdrcV20Params2Api(AdrcContext_t* pAdrcCtx, DrcInfoV20_t* pDrcInfo) {
    LOG1_ATMO("%s:enter!\n", __FUNCTION__);

    // ctrl info
    pDrcInfo->CtrlInfo.ISO   = pAdrcCtx->NextData.AEData.ISO;
    pDrcInfo->CtrlInfo.EnvLv = pAdrcCtx->NextData.AEData.EnvLv;

    // paras
    /*
    pDrcInfo->ValidParams.Enable = pAdrcCtx->NextData.bDrcEn;
    pDrcInfo->ValidParams.hw_adrc_luma2drcGain_setting.sw_adrc_drcGain_maxLimit =
        pAdrcCtx->NextData.dynParams.Drc_v20.sw_adrc_luma2drcGain_autoVal.sw_adrc_drcGain_maxLimit;
    pDrcInfo->ValidParams.hw_adrc_luma2drcGain_setting.sw_adrc_drcGainLumaAdj_scale =
        pAdrcCtx->NextData.dynParams.Drc_v20.sw_adrc_luma2drcGain_autoVal
            .sw_adrc_drcGainLumaAdj_scale;
    pDrcInfo->ValidParams.hw_adrc_luma2drcGain_setting.hw_adrc_drcGainIdxLuma_scale =
        pAdrcCtx->NextData.dynParams.Drc_v20.sw_adrc_luma2drcGain_autoVal
            .hw_adrc_drcGainIdxLuma_scale;
    pDrcInfo->ValidParams.hw_adrc_biflt_setting.hw_adrc_biflt_params.hw_adrc_bifilt_wgt =
        pAdrcCtx->NextData.dynParams.Drc_v20.hw_adrc_biflt_params.hw_adrc_bifilt_wgt;
    pDrcInfo->ValidParams.hw_adrc_biflt_setting.hw_adrc_biflt_params.hw_adrc_bifiltSoftThred_en =
        pAdrcCtx->NextData.dynParams.Drc_v20.hw_adrc_biflt_params.hw_adrc_bifiltSoftThred_en;
    pDrcInfo->ValidParams.hw_adrc_biflt_setting.hw_adrc_biflt_params.hw_adrc_bifilt_softThred =
        pAdrcCtx->NextData.dynParams.Drc_v20.hw_adrc_biflt_params.hw_adrc_bifilt_softThred;
    pDrcInfo->ValidParams.hw_adrc_biflt_setting.hw_adrc_biflt_params.hw_adrc_hiDetail_ratio =
        pAdrcCtx->NextData.dynParams.Drc_v20.hw_adrc_biflt_params.hw_adrc_hiDetail_ratio;
    pDrcInfo->ValidParams.hw_adrc_biflt_setting.hw_adrc_biflt_params.hw_adrc_loDetail_ratio =
        pAdrcCtx->NextData.dynParams.Drc_v20.hw_adrc_biflt_params.hw_adrc_loDetail_ratio;
    pDrcInfo->ValidParams.hw_adrc_biflt_setting.hw_adrc_biflt_params.hw_adrc_gas_t =
        pAdrcCtx->NextData.dynParams.Drc_v20.hw_adrc_biflt_params.hw_adrc_gas_t;
    pDrcInfo->ValidParams.LocalSetting.curPixWeit =
        pAdrcCtx->NextData.dynParams.Drc_v20.curPixWeit;
    pDrcInfo->ValidParams.LocalSetting.preFrameWeit =
        pAdrcCtx->NextData.dynParams.Drc_v20.preFrameWeit;
    pDrcInfo->ValidParams.LocalSetting.Range_sgm_pre =
        pAdrcCtx->NextData.dynParams.Drc_v20.Range_sgm_pre;
    pDrcInfo->ValidParams.LocalSetting.Range_sgm_cur =
        pAdrcCtx->NextData.dynParams.Drc_v20.Range_sgm_cur;
    pDrcInfo->ValidParams.Edge_Weit       = pAdrcCtx->NextData.dynParams.Drc_v20.Edge_Weit;
    pDrcInfo->ValidParams.OutPutLongFrame = pAdrcCtx->NextData.dynParams.Drc_v20.OutPutLongFrame;
    pDrcInfo->ValidParams.IIR_frame       = pAdrcCtx->NextData.dynParams.Drc_v20.IIR_frame;
    pDrcInfo->ValidParams.hw_adrc_luma2compsLuma_setting.sw_adrc_luma2compsLuma_mode =
        pAdrcCtx->NextData.dynParams.Drc_v20.hw_adrc_luma2compsLuma_setting
            .sw_adrc_luma2compsLuma_mode;
    for (int i = 0; i < ADRC_Y_NUM; i++) {
        pDrcInfo->ValidParams.hw_adrc_luma2compsLuma_setting.hw_adrc_luma2compsLuma_mVal[i] =
            pAdrcCtx->NextData.dynParams.Drc_v20.hw_adrc_luma2compsLuma_setting
                .hw_adrc_luma2compsLuma_mVal[i];
        pDrcInfo->ValidParams.hw_adrc_luma2compsGainScale_val[i] =
            pAdrcCtx->NextData.dynParams.Drc_v20.hw_adrc_luma2compsGainScale_val[i];
    }
    */

    LOG1_ATMO("%s:exit!\n", __FUNCTION__);
}

/******************************************************************************
 * AdrcAutoProcessing()
 *get handle para by config and current variate
 *****************************************************************************/
void AdrcAutoProcessing(adrcDynParams_t* pAdrcHanleData, CalibDbV2_Adrc_V20_t* pDrcTuningPara,
                        float CtrlValue) {
    LOG1_ATMO("%s:enter!\n", __FUNCTION__);

    int lo = 0, hi = 0;
    float ratio = 0.0f;

    // get hw_adrc_cmps_setting
    // get hw_adrc_comps_mode
    pAdrcHanleData->Drc_v20.hw_adrc_cmps_setting.hw_adrc_comps_mode =
        pDrcTuningPara->hw_adrc_cmps_setting.hw_adrc_comps_mode;
    // get hw_adrc_comps_offset
    pAdrcHanleData->Drc_v20.hw_adrc_cmps_setting.hw_adrc_comps_offset =
        pDrcTuningPara->hw_adrc_cmps_setting.hw_adrc_comps_offset;

    // get hw_adrc_luma2drcGain_setting
    ratio = DrcGetInterpRatioV20(
        pDrcTuningPara->hw_adrc_luma2drcGain_setting.sw_adrc_luma2drcGain_autoVal.iso, lo, hi,
        CtrlValue, ADRC_ENVLV_STEP_MAX);
    // get sw_adrc_drcGain_maxLimit
    pAdrcHanleData->Drc_v20.hw_adrc_luma2drcGain_setting.sw_adrc_luma2drcGain_autoVal
        .sw_adrc_drcGain_maxLimit =
        ratio * (pDrcTuningPara->hw_adrc_luma2drcGain_setting.sw_adrc_luma2drcGain_autoVal
                     .sw_adrc_drcGain_maxLimit[hi] -
                 pDrcTuningPara->hw_adrc_luma2drcGain_setting.sw_adrc_luma2drcGain_autoVal
                     .sw_adrc_drcGain_maxLimit[lo]) +
        pDrcTuningPara->hw_adrc_luma2drcGain_setting.sw_adrc_luma2drcGain_autoVal
            .sw_adrc_drcGain_maxLimit[lo];
    // get sw_adrc_drcGainLumaAdj_scale
    pAdrcHanleData->Drc_v20.hw_adrc_luma2drcGain_setting.sw_adrc_luma2drcGain_autoVal
        .sw_adrc_drcGainLumaAdj_scale =
        ratio * (pDrcTuningPara->hw_adrc_luma2drcGain_setting.sw_adrc_luma2drcGain_autoVal
                     .sw_adrc_drcGainLumaAdj_scale[hi] -
                 pDrcTuningPara->hw_adrc_luma2drcGain_setting.sw_adrc_luma2drcGain_autoVal
                     .sw_adrc_drcGainLumaAdj_scale[lo]) +
        pDrcTuningPara->hw_adrc_luma2drcGain_setting.sw_adrc_luma2drcGain_autoVal
            .sw_adrc_drcGainLumaAdj_scale[lo];
    // get sw_adrc_drcGain_mode
    pAdrcHanleData->Drc_v20.hw_adrc_luma2drcGain_setting.sw_adrc_drcGain_mode =
        pDrcTuningPara->hw_adrc_luma2drcGain_setting.sw_adrc_drcGain_mode;
    // get hw_adrc_drcGainIdxLuma_scale
    pAdrcHanleData->Drc_v20.hw_adrc_luma2drcGain_setting.hw_adrc_drcGainIdxLuma_scale =
        pDrcTuningPara->hw_adrc_luma2drcGain_setting.hw_adrc_drcGainIdxLuma_scale;
    // get hw_adrc_luma2drcGain_manualVal
    for (int i = 0; i < ADRC_Y_NUM; i++)
        pAdrcHanleData->Drc_v20.hw_adrc_luma2drcGain_setting.hw_adrc_luma2drcGain_manualVal[i] =
            pDrcTuningPara->hw_adrc_luma2drcGain_setting.hw_adrc_luma2drcGain_manualVal[i];

    // get hw_adrc_biflt_setting
    ratio = DrcGetInterpRatioV20(pDrcTuningPara->hw_adrc_biflt_setting.hw_adrc_biflt_params.iso, lo,
                                 hi, CtrlValue, ADRC_ENVLV_STEP_MAX);
    // get hw_adrc_bifilt_wgt
    pAdrcHanleData->Drc_v20.hw_adrc_biflt_setting.hw_adrc_biflt_params.hw_adrc_bifilt_wgt =
        ratio *
            (pDrcTuningPara->hw_adrc_biflt_setting.hw_adrc_biflt_params.hw_adrc_bifilt_wgt[hi] -
             pDrcTuningPara->hw_adrc_biflt_setting.hw_adrc_biflt_params.hw_adrc_bifilt_wgt[lo]) +
        pDrcTuningPara->hw_adrc_biflt_setting.hw_adrc_biflt_params.hw_adrc_bifilt_wgt[lo];
    // get hw_adrc_bifiltSoftThred_en
    pAdrcHanleData->Drc_v20.hw_adrc_biflt_setting.hw_adrc_biflt_params.hw_adrc_bifiltSoftThred_en =
        pDrcTuningPara->hw_adrc_biflt_setting.hw_adrc_biflt_params.hw_adrc_bifiltSoftThred_en[lo];
    // get hw_adrc_bifilt_softThred
    pAdrcHanleData->Drc_v20.hw_adrc_biflt_setting.hw_adrc_biflt_params.hw_adrc_bifilt_softThred =
        ratio * (pDrcTuningPara->hw_adrc_biflt_setting.hw_adrc_biflt_params
                     .hw_adrc_bifilt_softThred[hi] -
                 pDrcTuningPara->hw_adrc_biflt_setting.hw_adrc_biflt_params
                     .hw_adrc_bifilt_softThred[lo]) +
        pDrcTuningPara->hw_adrc_biflt_setting.hw_adrc_biflt_params.hw_adrc_bifilt_softThred[lo];
    // get hw_adrc_thumbThred_en
    pAdrcHanleData->Drc_v20.hw_adrc_biflt_setting.hw_adrc_thumb_setting.hw_adrc_thumbThred_en =
        pDrcTuningPara->hw_adrc_biflt_setting.hw_adrc_thumb_setting.hw_adrc_thumbThred_en;
    // get hw_adrc_thumb_maxLimit
    pAdrcHanleData->Drc_v20.hw_adrc_biflt_setting.hw_adrc_thumb_setting.hw_adrc_thumb_maxLimit =
        pDrcTuningPara->hw_adrc_biflt_setting.hw_adrc_thumb_setting.hw_adrc_thumb_maxLimit;
    // get hw_adrc_thumb_scale
    pAdrcHanleData->Drc_v20.hw_adrc_biflt_setting.hw_adrc_thumb_setting.hw_adrc_thumb_scale =
        pDrcTuningPara->hw_adrc_biflt_setting.hw_adrc_thumb_setting.hw_adrc_thumb_scale;
    // get hw_adrc_thumbThred_neg
    pAdrcHanleData->Drc_v20.hw_adrc_biflt_setting.hw_adrc_thumb_setting.hw_adrc_thumbThred_neg =
        pDrcTuningPara->hw_adrc_biflt_setting.hw_adrc_thumb_setting.hw_adrc_thumbThred_neg;
    // get sw_adrc_autoCurve_mode
    pAdrcHanleData->Drc_v20.hw_adrc_biflt_setting.hw_adrc_thumb_setting.thumb_thd_table_setting
        .sw_adrc_autoCurve_mode = pDrcTuningPara->hw_adrc_biflt_setting.hw_adrc_thumb_setting
                                      .thumb_thd_table_setting.sw_adrc_autoCurve_mode;
    // get hw_adrc_thumbLumaDiff2thrd_mVal
    for (int i = 0; i < ADRC_Y_NUM; i++)
        pAdrcHanleData->Drc_v20.hw_adrc_biflt_setting.hw_adrc_thumb_setting.thumb_thd_table_setting
            .hw_adrc_thumbLumaDiff2thrd_mVal[i] =
            pDrcTuningPara->hw_adrc_biflt_setting.hw_adrc_thumb_setting.thumb_thd_table_setting
                .hw_adrc_thumbLumaDiff2thrd_mVal[i];
    // get sw_adrc_thumbThred_minLimit
    pAdrcHanleData->Drc_v20.hw_adrc_biflt_setting.hw_adrc_thumb_setting.thumb_thd_table_setting
        .sw_adrc_thumbLumaDiff2thrd_aVal.sw_adrc_thumbThred_minLimit =
        pDrcTuningPara->hw_adrc_biflt_setting.hw_adrc_thumb_setting.thumb_thd_table_setting
            .sw_adrc_thumbLumaDiff2thrd_aVal.sw_adrc_thumbThred_minLimit;
    // get sw_adrc_thumbThred_maxLimit
    pAdrcHanleData->Drc_v20.hw_adrc_biflt_setting.hw_adrc_thumb_setting.thumb_thd_table_setting
        .sw_adrc_thumbLumaDiff2thrd_aVal.sw_adrc_thumbThred_maxLimit =
        pDrcTuningPara->hw_adrc_biflt_setting.hw_adrc_thumb_setting.thumb_thd_table_setting
            .sw_adrc_thumbLumaDiff2thrd_aVal.sw_adrc_thumbThred_maxLimit;
    // get sw_adrc_thumbThredCurve_slope
    pAdrcHanleData->Drc_v20.hw_adrc_biflt_setting.hw_adrc_thumb_setting.thumb_thd_table_setting
        .sw_adrc_thumbLumaDiff2thrd_aVal.sw_adrc_thumbThredCurve_slope =
        pDrcTuningPara->hw_adrc_biflt_setting.hw_adrc_thumb_setting.thumb_thd_table_setting
            .sw_adrc_thumbLumaDiff2thrd_aVal.sw_adrc_thumbThredCurve_slope;
    // get sw_adrc_thumbThredCurve_offset
    pAdrcHanleData->Drc_v20.hw_adrc_biflt_setting.hw_adrc_thumb_setting.thumb_thd_table_setting
        .sw_adrc_thumbLumaDiff2thrd_aVal.sw_adrc_thumbThredCurve_offset =
        pDrcTuningPara->hw_adrc_biflt_setting.hw_adrc_thumb_setting.thumb_thd_table_setting
            .sw_adrc_thumbLumaDiff2thrd_aVal.sw_adrc_thumbThredCurve_offset;
    // get hw_adrc_gas_t
    pAdrcHanleData->Drc_v20.hw_adrc_biflt_setting.hw_adrc_gas_t =
        pDrcTuningPara->hw_adrc_biflt_setting.hw_adrc_gas_t;
    // get hw_adrc_bifilt_curPixelWgt
    pAdrcHanleData->Drc_v20.hw_adrc_biflt_setting.hw_adrc_bifilt_curPixelWgt =
        pDrcTuningPara->hw_adrc_biflt_setting.hw_adrc_bifilt_curPixelWgt;
    // get hw_adrc_bifilt_hiWgt
    pAdrcHanleData->Drc_v20.hw_adrc_biflt_setting.hw_adrc_bifilt_hiWgt =
        pDrcTuningPara->hw_adrc_biflt_setting.hw_adrc_bifilt_hiWgt;
    // get hw_adrc_bifilt_wgtOffset
    pAdrcHanleData->Drc_v20.hw_adrc_biflt_setting.hw_adrc_bifilt_wgtOffset =
        pDrcTuningPara->hw_adrc_biflt_setting.hw_adrc_bifilt_wgtOffset;
    // get hw_adrc_hiRange_invSigma
    pAdrcHanleData->Drc_v20.hw_adrc_biflt_setting.hw_adrc_hiRange_invSigma =
        pDrcTuningPara->hw_adrc_biflt_setting.hw_adrc_hiRange_invSigma;
    // get hw_adrc_loRange_invSigma
    pAdrcHanleData->Drc_v20.hw_adrc_biflt_setting.hw_adrc_loRange_invSigma =
        pDrcTuningPara->hw_adrc_biflt_setting.hw_adrc_loRange_invSigma;

    // get hw_adrc_luma2compsLuma_setting
    pAdrcHanleData->Drc_v20.hw_adrc_luma2compsLuma_setting =
        pDrcTuningPara->hw_adrc_luma2compsLuma_setting;

    // get hw_adrc_adjCompsGain_setting
    // get hw_adrc_compsGain_minLimit
    pAdrcHanleData->Drc_v20.hw_adrc_adjCompsGain_setting.hw_adrc_compsGain_minLimit =
        pDrcTuningPara->hw_adrc_adjCompsGain_setting.hw_adrc_compsGain_minLimit;
    // get hw_adrc_loDetail_ratio
    ratio = DrcGetInterpRatioV20(
        pDrcTuningPara->hw_adrc_adjCompsGain_setting.hw_adrc_adjCompsGain_params.iso, lo, hi,
        CtrlValue, ADRC_ENVLV_STEP_MAX);
    pAdrcHanleData->Drc_v20.hw_adrc_adjCompsGain_setting.hw_adrc_adjCompsGain_params
        .hw_adrc_loDetail_ratio =
        ratio * (pDrcTuningPara->hw_adrc_adjCompsGain_setting.hw_adrc_adjCompsGain_params
                     .hw_adrc_loDetail_ratio[hi] -
                 pDrcTuningPara->hw_adrc_adjCompsGain_setting.hw_adrc_adjCompsGain_params
                     .hw_adrc_loDetail_ratio[lo]) +
        pDrcTuningPara->hw_adrc_adjCompsGain_setting.hw_adrc_adjCompsGain_params
            .hw_adrc_loDetail_ratio[lo];
    // get hw_adrc_hiDetail_ratio
    pAdrcHanleData->Drc_v20.hw_adrc_adjCompsGain_setting.hw_adrc_adjCompsGain_params
        .hw_adrc_hiDetail_ratio =
        ratio * (pDrcTuningPara->hw_adrc_adjCompsGain_setting.hw_adrc_adjCompsGain_params
                     .hw_adrc_hiDetail_ratio[hi] -
                 pDrcTuningPara->hw_adrc_adjCompsGain_setting.hw_adrc_adjCompsGain_params
                     .hw_adrc_hiDetail_ratio[lo]) +
        pDrcTuningPara->hw_adrc_adjCompsGain_setting.hw_adrc_adjCompsGain_params
            .hw_adrc_hiDetail_ratio[lo];
    // get hw_adrc_adjCompsGainIdxLuma_scl
    pAdrcHanleData->Drc_v20.hw_adrc_adjCompsGain_setting.hw_adrc_adjCompsGainIdxLuma_scl =
        pDrcTuningPara->hw_adrc_adjCompsGain_setting.hw_adrc_adjCompsGainIdxLuma_scl;
    // get hw_adrc_luma2compsGainScale_val
    for (int i = 0; i < ADRC_Y_NUM; i++)
        pAdrcHanleData->Drc_v20.hw_adrc_adjCompsGain_setting.hw_adrc_luma2compsGainScale_val[i] =
            pDrcTuningPara->hw_adrc_adjCompsGain_setting.hw_adrc_luma2compsGainScale_val[i];

    // damp
    pAdrcHanleData->damp = pDrcTuningPara->sw_adrc_damp_coef;

    LOG1_ATMO("%s:exit!\n", __FUNCTION__);
}

/******************************************************************************
 * AdrcTuningParaProcessing()
 *get handle para by config and current variate
 *****************************************************************************/
void AdrcTuningParaProcessing(AdrcContext_t* pAdrcCtx, RkAiqAdrcProcResult_t* pAdrcProcRes) {
    LOG1_ATMO("%s:enter!\n", __FUNCTION__);

    // para setting
    if (pAdrcCtx->drcAttrV20.opMode == DRC_OPMODE_AUTO) {
        AdrcAutoProcessing(&pAdrcCtx->NextData.dynParams,
                           &pAdrcCtx->drcAttrV20.stAuto.DrcTuningPara,
                           pAdrcCtx->NextData.AEData.ISO);
        pAdrcCtx->isDampStable =
            AdrcDampingV20(&pAdrcCtx->NextData, &pAdrcCtx->CurrData, pAdrcCtx->FrameID);
    } else if (pAdrcCtx->drcAttrV20.opMode == DRC_OPMODE_MANUAL) {
        pAdrcCtx->NextData.dynParams.Drc_v20 = pAdrcCtx->drcAttrV20.stManual;
        pAdrcCtx->isDampStable               = true;
    }

    // clip drc gain
    if (pAdrcCtx->FrameNumber == HDR_2X_NUM || pAdrcCtx->FrameNumber == HDR_3X_NUM) {
        if (pAdrcCtx->NextData.AEData.L2S_Ratio *
                pAdrcCtx->NextData.dynParams.Drc_v20.hw_adrc_luma2drcGain_setting
                    .sw_adrc_luma2drcGain_autoVal.sw_adrc_drcGain_maxLimit >
            MAX_AE_DRC_GAIN) {
            LOGE_ATMO("%s:  AERatio*sw_adrc_drcGain_maxLimit > 256!!!\n", __FUNCTION__);
            pAdrcCtx->NextData.dynParams.Drc_v20.hw_adrc_luma2drcGain_setting
                .sw_adrc_luma2drcGain_autoVal.sw_adrc_drcGain_maxLimit =
                MAX(MAX_AE_DRC_GAIN / pAdrcCtx->NextData.AEData.L2S_Ratio, GAINMIN);
        }
    } else if (pAdrcCtx->FrameNumber == LINEAR_NUM) {
        if (pAdrcCtx->ablcV32_proc_res.isp_ob_predgain *
                pAdrcCtx->NextData.dynParams.Drc_v20.hw_adrc_luma2drcGain_setting
                    .sw_adrc_luma2drcGain_autoVal.sw_adrc_drcGain_maxLimit >
            MAX_AE_DRC_GAIN) {
            LOGE_ATMO("%s:  predgain*sw_adrc_drcGain_maxLimit > 256!!!\n", __FUNCTION__);
            if (pAdrcCtx->ablcV32_proc_res.isp_ob_predgain > MAX_AE_DRC_GAIN)
                LOGE_ATMO("%s:  predgain > 256!!!\n", __FUNCTION__);
            else
                pAdrcCtx->NextData.dynParams.Drc_v20.hw_adrc_luma2drcGain_setting
                    .sw_adrc_luma2drcGain_autoVal.sw_adrc_drcGain_maxLimit =
                    MAX(MAX_AE_DRC_GAIN / pAdrcCtx->ablcV32_proc_res.isp_ob_predgain, GAINMIN);
        }
    } else if (pAdrcCtx->FrameNumber == SENSOR_MGE) {
        if (pow(2.0f, float(pAdrcCtx->compr_bit - ISP_HDR_BIT_NUM_MIN)) *
                pAdrcCtx->NextData.dynParams.Drc_v20.hw_adrc_luma2drcGain_setting
                    .sw_adrc_luma2drcGain_autoVal.sw_adrc_drcGain_maxLimit >
            MAX_AE_DRC_GAIN) {
            if (pow(2.0f, float(pAdrcCtx->compr_bit - ISP_HDR_BIT_NUM_MIN)) > MAX_AE_DRC_GAIN)
                LOGE_ATMO("%s:  SensorMgeRatio > 256x!!!\n", __FUNCTION__);
            else
                pAdrcCtx->NextData.dynParams.Drc_v20.hw_adrc_luma2drcGain_setting
                    .sw_adrc_luma2drcGain_autoVal.sw_adrc_drcGain_maxLimit = MAX(
                    MAX_AE_DRC_GAIN / pow(2.0f, float(pAdrcCtx->compr_bit - ISP_HDR_BIT_NUM_MIN)),
                    GAINMIN);
            LOGI_ATMO(
                "%s:  SensorMgeRatio*sw_adrc_drcGain_maxLimit > 256x, sw_adrc_drcGain_maxLimit "
                "clip to %f!!!\n",
                __FUNCTION__,
                pAdrcCtx->NextData.dynParams.Drc_v20.hw_adrc_luma2drcGain_setting
                    .sw_adrc_luma2drcGain_autoVal.sw_adrc_drcGain_maxLimit);
        }
    }
    // clip hw_adrc_compsGain_minLimit
    if (pAdrcCtx->NextData.AEData.LongFrmMode)
        pAdrcCtx->NextData.dynParams.Drc_v20.hw_adrc_adjCompsGain_setting
            .hw_adrc_compsGain_minLimit = DRC_COMPSGAIN_LONG_FRAME;

    LOGD_ATMO(
        "%s: Current ob_on:%d predgain:%f sw_adrc_drcGain_mode:%d sw_adrc_drcGain_maxLimit:%f "
        "sw_adrc_drcGainLumaAdj_scale:%f hw_adrc_drcGainIdxLuma_scale:%f \n",
        __FUNCTION__, pAdrcCtx->ablcV32_proc_res.blc_ob_enable,
        pAdrcCtx->ablcV32_proc_res.isp_ob_predgain,
        pAdrcCtx->NextData.dynParams.Drc_v20.hw_adrc_luma2drcGain_setting.sw_adrc_drcGain_mode,
        pAdrcCtx->NextData.dynParams.Drc_v20.hw_adrc_luma2drcGain_setting
            .sw_adrc_luma2drcGain_autoVal.sw_adrc_drcGain_maxLimit,
        pAdrcCtx->NextData.dynParams.Drc_v20.hw_adrc_luma2drcGain_setting
            .sw_adrc_luma2drcGain_autoVal.sw_adrc_drcGainLumaAdj_scale,
        pAdrcCtx->NextData.dynParams.Drc_v20.hw_adrc_luma2drcGain_setting
            .hw_adrc_drcGainIdxLuma_scale);
    LOGD_ATMO(
        "%s: hw_adrc_bifiltSoftThred_en:%d hw_adrc_bifilt_softThred:%d hw_adrc_bifilt_wgt:%d "
        "hw_adrc_hiDetail_ratio:%d hw_adrc_loDetail_ratio:%d\n",
        __FUNCTION__,
        pAdrcCtx->NextData.dynParams.Drc_v20.hw_adrc_biflt_setting.hw_adrc_biflt_params
            .hw_adrc_bifiltSoftThred_en,
        pAdrcCtx->NextData.dynParams.Drc_v20.hw_adrc_biflt_setting.hw_adrc_biflt_params
            .hw_adrc_bifiltSoftThred_en,
        pAdrcCtx->NextData.dynParams.Drc_v20.hw_adrc_biflt_setting.hw_adrc_biflt_params
            .hw_adrc_bifilt_wgt,
        pAdrcCtx->NextData.dynParams.Drc_v20.hw_adrc_adjCompsGain_setting
            .hw_adrc_adjCompsGain_params.hw_adrc_hiDetail_ratio,
        pAdrcCtx->NextData.dynParams.Drc_v20.hw_adrc_adjCompsGain_setting
            .hw_adrc_adjCompsGain_params.hw_adrc_loDetail_ratio);

    // get io data
    AdrcGetTuningProcResV20(&pAdrcCtx->NextData.dynParams.Drc_v20, &pAdrcCtx->ablcV32_proc_res,
                            pAdrcProcRes);

    // store current handle data to pre data for next loop
    pAdrcCtx->CurrData.AEData.EnvLv = pAdrcCtx->NextData.AEData.EnvLv;
    pAdrcCtx->CurrData.AEData.ISO   = pAdrcCtx->NextData.AEData.ISO;
    pAdrcCtx->CurrData.MotionCoef   = pAdrcCtx->NextData.MotionCoef;
    pAdrcCtx->CurrData.ApiMode      = pAdrcCtx->drcAttrV20.opMode;
    if (pAdrcCtx->drcAttrV20.opMode == DRC_OPMODE_AUTO && !pAdrcCtx->isDampStable)
        pAdrcCtx->CurrData.dynParams.Drc_v20 = pAdrcCtx->NextData.dynParams.Drc_v20;

    LOG1_ATMO("%s:exit!\n", __FUNCTION__);
}

/******************************************************************************
 * AdrcExpoParaProcessing()
 *get handle para by config and current variate
 *****************************************************************************/
void AdrcExpoParaProcessing(AdrcContext_t* pAdrcCtx, RkAiqAdrcProcResult_t* pAdrcProcRes) {
    LOG1_ATMO("%s:enter!\n", __FUNCTION__);

    // get sw_drc_compres_scl
    float adrc_gain = 1.0f;
    if (pAdrcCtx->FrameNumber == LINEAR_NUM && pAdrcCtx->ablcV32_proc_res.blc_ob_enable)
        adrc_gain = pAdrcCtx->NextData.dynParams.Drc_v20.hw_adrc_luma2drcGain_setting
                        .sw_adrc_luma2drcGain_autoVal.sw_adrc_drcGain_maxLimit *
                    pAdrcCtx->ablcV32_proc_res.isp_ob_predgain;
    else
        adrc_gain = pAdrcCtx->NextData.dynParams.Drc_v20.hw_adrc_luma2drcGain_setting
                        .sw_adrc_luma2drcGain_autoVal.sw_adrc_drcGain_maxLimit;
    float log_ratio2     = log(pAdrcCtx->NextData.AEData.L2S_Ratio * adrc_gain) / log(2.0f) + 12.0f;
    float offsetbits_int = (float)(pAdrcProcRes->DrcProcRes.Drc_v20.offset_pow2);
    int cmps_fix_bit     = 8 + pAdrcProcRes->DrcProcRes.Drc_v20.cmps_fixbit_mode;
    float offsetbits     = offsetbits_int * (1 << cmps_fix_bit);
    float hdrbits        = log_ratio2 * (1 << cmps_fix_bit);
    float hdrvalidbits   = hdrbits - offsetbits;
    float compres_scl    = (12.0f * (1 << (cmps_fix_bit + 11))) / hdrvalidbits;
    pAdrcProcRes->DrcProcRes.Drc_v20.compres_scl = (unsigned short)(compres_scl);

    // get sw_drc_min_ogain
    if (pAdrcCtx->NextData.dynParams.Drc_v20.hw_adrc_adjCompsGain_setting
            .hw_adrc_compsGain_minLimit == DRC_COMPSGAIN_NORMAL0) {
        pAdrcProcRes->DrcProcRes.Drc_v20.min_ogain = FUNCTION_DISABLE;
    } else if (pAdrcCtx->NextData.dynParams.Drc_v20.hw_adrc_adjCompsGain_setting
                   .hw_adrc_compsGain_minLimit == DRC_COMPSGAIN_NORMAL1) {
        float sw_drc_min_ogain = 1.0f / (pAdrcCtx->NextData.AEData.L2S_Ratio * adrc_gain);
        pAdrcProcRes->DrcProcRes.Drc_v20.min_ogain =
            adrcClipFloatValueV20(sw_drc_min_ogain, 1, 15, true);
    } else if (pAdrcCtx->NextData.dynParams.Drc_v20.hw_adrc_adjCompsGain_setting
                   .hw_adrc_compsGain_minLimit == DRC_COMPSGAIN_LONG_FRAME) {
        pAdrcProcRes->DrcProcRes.Drc_v20.min_ogain = 1 << 15;
    }

    // get sw_drc_compres_y
    if (pAdrcCtx->NextData.dynParams.Drc_v20.hw_adrc_luma2compsLuma_setting
            .sw_adrc_luma2compsLuma_mode == COMPRESS_AUTO) {
        float luma2[DRC_V20_Y_NUM] = {0.0f,     1024.0f,  2048.0f,  3072.0f,  4096.0f,  5120.0f,
                                      6144.0f,  7168.0f,  8192.0f,  10240.0f, 12288.0f, 14336.0f,
                                      16384.0f, 18432.0f, 20480.0f, 22528.0f, 24576.0f};
        float dstbits              = (float)(ISP_RAW_BIT << cmps_fix_bit);
        float validbits            = dstbits - offsetbits;
        float curveparam  = (float)(validbits - 0.0f) / (hdrvalidbits - validbits + 0.0156f);
        float curveparam2 = validbits * (1.0f + curveparam);
        float curveparam3 = hdrvalidbits * curveparam;
        for (int i = 0; i < DRC_V20_Y_NUM; ++i) {
            float tmp                                     = luma2[i] * hdrvalidbits / 24576.0f;
            float curveTable                              = tmp * curveparam2 / (tmp + curveparam3);
            pAdrcProcRes->DrcProcRes.Drc_v20.compres_y[i] =
                ((unsigned short)(curveTable)) >> pAdrcProcRes->DrcProcRes.Drc_v20.cmps_fixbit_mode;
        }
    } else if (pAdrcCtx->NextData.dynParams.Drc_v20.hw_adrc_luma2compsLuma_setting
                   .sw_adrc_luma2compsLuma_mode == COMPRESS_MANUAL) {
        for (int i = 0; i < DRC_V20_Y_NUM; ++i)
            pAdrcProcRes->DrcProcRes.Drc_v20.compres_y[i] =
                (unsigned short)(pAdrcCtx->NextData.dynParams.Drc_v20.hw_adrc_luma2compsLuma_setting
                                     .hw_adrc_luma2compsLuma_mVal[i]);
    }

    LOGV_ATMO("%s: blc_ob_enable:%d OB_predgain:%f DrcGain:%f nextRatioLS:%f TotalDgain:%f\n",
              __FUNCTION__, pAdrcCtx->ablcV32_proc_res.blc_ob_enable,
              pAdrcCtx->ablcV32_proc_res.isp_ob_predgain,
              pAdrcCtx->NextData.dynParams.Drc_v20.hw_adrc_luma2drcGain_setting
                  .sw_adrc_luma2drcGain_autoVal.sw_adrc_drcGain_maxLimit,
              pAdrcCtx->NextData.AEData.L2S_Ratio, adrc_gain);
    LOGV_ATMO(
        "%s: bypass_en:%d gainx32_en:%d raw_dly_dis:%d sw_drc_position:%d sw_drc_compres_scl:%d "
        "sw_drc_offset_pow2:%d\n",
        __FUNCTION__, pAdrcProcRes->DrcProcRes.Drc_v20.bypass_en,
        pAdrcProcRes->DrcProcRes.Drc_v20.gainx32_en, pAdrcProcRes->DrcProcRes.Drc_v20.raw_dly_dis,
        pAdrcProcRes->DrcProcRes.Drc_v20.position, pAdrcProcRes->DrcProcRes.Drc_v20.compres_scl,
        pAdrcProcRes->DrcProcRes.Drc_v20.offset_pow2);
    LOGV_ATMO("%s: sw_drc_lpdetail_ratio:%d sw_drc_hpdetail_ratio:%d sw_drc_delta_scalein:%d\n",
              __FUNCTION__, pAdrcProcRes->DrcProcRes.Drc_v20.lpdetail_ratio,
              pAdrcProcRes->DrcProcRes.Drc_v20.hpdetail_ratio,
              pAdrcProcRes->DrcProcRes.Drc_v20.delta_scalein);
    LOGV_ATMO(
        "%s: sw_drc_bilat_wt_off:%d thumb_thd_neg:%d thumb_thd_enable:%d sw_drc_weicur_pix:%d\n",
        __FUNCTION__, pAdrcProcRes->DrcProcRes.Drc_v20.bilat_wt_off,
        pAdrcProcRes->DrcProcRes.Drc_v20.thumb_thd_neg,
        pAdrcProcRes->DrcProcRes.Drc_v20.thumb_thd_enable,
        pAdrcProcRes->DrcProcRes.Drc_v20.weicur_pix);
    LOGV_ATMO(
        "%s: cmps_byp_en:%d cmps_offset_bits_int:%d cmps_fixbit_mode:%d drc_gas_t:%d thumb_clip:%d "
        "thumb_scale:%d "
        "\n",
        __FUNCTION__, pAdrcProcRes->DrcProcRes.Drc_v20.cmps_byp_en,
        pAdrcProcRes->DrcProcRes.Drc_v20.cmps_offset_bits_int,
        pAdrcProcRes->DrcProcRes.Drc_v20.cmps_fixbit_mode,
        pAdrcProcRes->DrcProcRes.Drc_v20.drc_gas_t, pAdrcProcRes->DrcProcRes.Drc_v20.thumb_clip,
        pAdrcProcRes->DrcProcRes.Drc_v20.thumb_scale);
    LOGV_ATMO(
        "%s: sw_drc_range_sgm_inv0:%d sw_drc_range_sgm_inv1:%d weig_bilat:%d weight_8x8thumb:%d\n",
        __FUNCTION__, pAdrcProcRes->DrcProcRes.Drc_v20.range_sgm_inv0,
        pAdrcProcRes->DrcProcRes.Drc_v20.range_sgm_inv1,
        pAdrcProcRes->DrcProcRes.Drc_v20.weig_bilat,
        pAdrcProcRes->DrcProcRes.Drc_v20.weight_8x8thumb);
    LOGV_ATMO(
        "%s: sw_drc_bilat_soft_thd:%d "
        "sw_drc_enable_soft_thd:%d sw_drc_min_ogain:%d\n",
        __FUNCTION__, pAdrcProcRes->DrcProcRes.Drc_v20.bilat_soft_thd,
        pAdrcProcRes->DrcProcRes.Drc_v20.enable_soft_thd,
        pAdrcProcRes->DrcProcRes.Drc_v20.min_ogain);
    LOGV_ATMO(
        "%s: sw_drc_gain_y: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n", __FUNCTION__,
        pAdrcProcRes->DrcProcRes.Drc_v20.gain_y[0], pAdrcProcRes->DrcProcRes.Drc_v20.gain_y[1],
        pAdrcProcRes->DrcProcRes.Drc_v20.gain_y[2], pAdrcProcRes->DrcProcRes.Drc_v20.gain_y[3],
        pAdrcProcRes->DrcProcRes.Drc_v20.gain_y[4], pAdrcProcRes->DrcProcRes.Drc_v20.gain_y[5],
        pAdrcProcRes->DrcProcRes.Drc_v20.gain_y[6], pAdrcProcRes->DrcProcRes.Drc_v20.gain_y[7],
        pAdrcProcRes->DrcProcRes.Drc_v20.gain_y[8], pAdrcProcRes->DrcProcRes.Drc_v20.gain_y[9],
        pAdrcProcRes->DrcProcRes.Drc_v20.gain_y[10], pAdrcProcRes->DrcProcRes.Drc_v20.gain_y[11],
        pAdrcProcRes->DrcProcRes.Drc_v20.gain_y[12], pAdrcProcRes->DrcProcRes.Drc_v20.gain_y[13],
        pAdrcProcRes->DrcProcRes.Drc_v20.gain_y[14], pAdrcProcRes->DrcProcRes.Drc_v20.gain_y[15],
        pAdrcProcRes->DrcProcRes.Drc_v20.gain_y[16]);
    LOGV_ATMO(
        "%s: sw_drc_scale_y: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n", __FUNCTION__,
        pAdrcProcRes->DrcProcRes.Drc_v20.scale_y[0], pAdrcProcRes->DrcProcRes.Drc_v20.scale_y[1],
        pAdrcProcRes->DrcProcRes.Drc_v20.scale_y[2], pAdrcProcRes->DrcProcRes.Drc_v20.scale_y[3],
        pAdrcProcRes->DrcProcRes.Drc_v20.scale_y[4], pAdrcProcRes->DrcProcRes.Drc_v20.scale_y[5],
        pAdrcProcRes->DrcProcRes.Drc_v20.scale_y[6], pAdrcProcRes->DrcProcRes.Drc_v20.scale_y[7],
        pAdrcProcRes->DrcProcRes.Drc_v20.scale_y[8], pAdrcProcRes->DrcProcRes.Drc_v20.scale_y[9],
        pAdrcProcRes->DrcProcRes.Drc_v20.scale_y[10], pAdrcProcRes->DrcProcRes.Drc_v20.scale_y[11],
        pAdrcProcRes->DrcProcRes.Drc_v20.scale_y[12], pAdrcProcRes->DrcProcRes.Drc_v20.scale_y[13],
        pAdrcProcRes->DrcProcRes.Drc_v20.scale_y[14], pAdrcProcRes->DrcProcRes.Drc_v20.scale_y[15],
        pAdrcProcRes->DrcProcRes.Drc_v20.scale_y[16]);
    LOGV_ATMO(
        "%s: CompressMode:%d sw_drc_compres_y: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d "
        "%d\n",
        __FUNCTION__,
        pAdrcCtx->NextData.dynParams.Drc_v20.hw_adrc_luma2compsLuma_setting
            .sw_adrc_luma2compsLuma_mode,
        pAdrcProcRes->DrcProcRes.Drc_v20.compres_y[0],
        pAdrcProcRes->DrcProcRes.Drc_v20.compres_y[1],
        pAdrcProcRes->DrcProcRes.Drc_v20.compres_y[2],
        pAdrcProcRes->DrcProcRes.Drc_v20.compres_y[3],
        pAdrcProcRes->DrcProcRes.Drc_v20.compres_y[4],
        pAdrcProcRes->DrcProcRes.Drc_v20.compres_y[5],
        pAdrcProcRes->DrcProcRes.Drc_v20.compres_y[6],
        pAdrcProcRes->DrcProcRes.Drc_v20.compres_y[7],
        pAdrcProcRes->DrcProcRes.Drc_v20.compres_y[8],
        pAdrcProcRes->DrcProcRes.Drc_v20.compres_y[9],
        pAdrcProcRes->DrcProcRes.Drc_v20.compres_y[10],
        pAdrcProcRes->DrcProcRes.Drc_v20.compres_y[11],
        pAdrcProcRes->DrcProcRes.Drc_v20.compres_y[12],
        pAdrcProcRes->DrcProcRes.Drc_v20.compres_y[13],
        pAdrcProcRes->DrcProcRes.Drc_v20.compres_y[14],
        pAdrcProcRes->DrcProcRes.Drc_v20.compres_y[15],
        pAdrcProcRes->DrcProcRes.Drc_v20.compres_y[16]);
    LOGV_ATMO(
        "%s: sfthd_y: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d "
        "%d\n",
        __FUNCTION__, pAdrcProcRes->DrcProcRes.Drc_v20.sfthd_y[0],
        pAdrcProcRes->DrcProcRes.Drc_v20.sfthd_y[1], pAdrcProcRes->DrcProcRes.Drc_v20.sfthd_y[2],
        pAdrcProcRes->DrcProcRes.Drc_v20.sfthd_y[3], pAdrcProcRes->DrcProcRes.Drc_v20.sfthd_y[4],
        pAdrcProcRes->DrcProcRes.Drc_v20.sfthd_y[5], pAdrcProcRes->DrcProcRes.Drc_v20.sfthd_y[6],
        pAdrcProcRes->DrcProcRes.Drc_v20.sfthd_y[7], pAdrcProcRes->DrcProcRes.Drc_v20.sfthd_y[8],
        pAdrcProcRes->DrcProcRes.Drc_v20.sfthd_y[9], pAdrcProcRes->DrcProcRes.Drc_v20.sfthd_y[10],
        pAdrcProcRes->DrcProcRes.Drc_v20.sfthd_y[11], pAdrcProcRes->DrcProcRes.Drc_v20.sfthd_y[12],
        pAdrcProcRes->DrcProcRes.Drc_v20.sfthd_y[13], pAdrcProcRes->DrcProcRes.Drc_v20.sfthd_y[14],
        pAdrcProcRes->DrcProcRes.Drc_v20.sfthd_y[15], pAdrcProcRes->DrcProcRes.Drc_v20.sfthd_y[16]);

    LOG1_ATMO("%s:exit!\n", __FUNCTION__);
}

/******************************************************************************
 * AdrcByPassTuningProcessing()
 *get handle para by config and current variate
 *****************************************************************************/
bool AdrcByPassTuningProcessing(AdrcContext_t* pAdrcCtx) {
    LOG1_ATMO("%s:enter!\n", __FUNCTION__);

    bool bypass = false;
    float diff  = 0.0f;

    if (pAdrcCtx->FrameID <= INIT_CALC_PARAMS_NUM)
        bypass = false;
    else if (pAdrcCtx->drcAttrV20.opMode != pAdrcCtx->CurrData.ApiMode)
        bypass = false;
    else if (pAdrcCtx->drcAttrV20.opMode == DRC_OPMODE_MANUAL)
        bypass = !pAdrcCtx->ifReCalcStManual;
    else if (pAdrcCtx->drcAttrV20.opMode == DRC_OPMODE_AUTO) {
        diff = pAdrcCtx->CurrData.AEData.ISO - pAdrcCtx->NextData.AEData.ISO;
        diff /= pAdrcCtx->CurrData.AEData.ISO;
        if (diff >= pAdrcCtx->drcAttrV20.stAuto.DrcTuningPara.sw_adrc_byPass_thred ||
            diff <= -pAdrcCtx->drcAttrV20.stAuto.DrcTuningPara.sw_adrc_byPass_thred)
            bypass = false;
        else
            bypass = true;
        bypass = bypass && !pAdrcCtx->ifReCalcStAuto;
    }

    LOGD_ATMO(
        "%s: FrameID:%d HDRFrameNum:%d LongFrmMode:%d DRCApiMode:%d ifReCalcStAuto:%d "
        "ifReCalcStManual:%d EnvLv:%f "
        "bypass:%d\n",
        __FUNCTION__, pAdrcCtx->FrameID, pAdrcCtx->FrameNumber,
        pAdrcCtx->NextData.AEData.LongFrmMode, pAdrcCtx->drcAttrV20.opMode,
        pAdrcCtx->ifReCalcStAuto, pAdrcCtx->ifReCalcStManual, pAdrcCtx->NextData.AEData.ISO,
        bypass);

    LOG1_ATMO(
        "%s: NextEnvLv:%f CurrEnvLv:%f NextISO:%f CurrISO:%f diff:%f ByPassThr:%f opMode:%d "
        "bypass:%d!\n",
        __FUNCTION__, pAdrcCtx->NextData.AEData.EnvLv, pAdrcCtx->CurrData.AEData.EnvLv,
        pAdrcCtx->NextData.AEData.ISO, pAdrcCtx->CurrData.AEData.ISO, diff,
        pAdrcCtx->drcAttrV20.stAuto.DrcTuningPara.sw_adrc_byPass_thred, pAdrcCtx->drcAttrV20.opMode,
        bypass);

    LOG1_ATMO("%s:exit!\n", __FUNCTION__);
    return bypass;
}
/******************************************************************************
 * AdrcInit()
 *****************************************************************************/
XCamReturn AdrcInit(AdrcContext_t** ppAdrcCtx, CamCalibDbV2Context_t* pCalibDb) {
    LOG1_ATMO("%s:enter!\n", __FUNCTION__);
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    AdrcContext_t* pAdrcCtx;
    pAdrcCtx = (AdrcContext_t*)malloc(sizeof(AdrcContext_t));
    if (pAdrcCtx == NULL) {
        LOGE_ATMO("%s(%d): invalid inputparams\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_MEM;
    }

    memset(pAdrcCtx, 0x00, sizeof(AdrcContext_t));
    *ppAdrcCtx      = pAdrcCtx;
    pAdrcCtx->state = ADRC_STATE_INITIALIZED;

    CalibDbV2_drc_V20_t* calibv2_adrc_calib =
        (CalibDbV2_drc_V20_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibDb, adrc_calib));
    memcpy(&pAdrcCtx->drcAttrV20.stAuto, calibv2_adrc_calib, sizeof(CalibDbV2_drc_V20_t));
    pAdrcCtx->drcAttrV20.opMode = DRC_OPMODE_AUTO;
    pAdrcCtx->ifReCalcStAuto    = true;
    pAdrcCtx->ifReCalcStManual  = false;
    pAdrcCtx->isCapture         = false;
    pAdrcCtx->isDampStable      = true;

    LOG1_ATMO("%s:exit!\n", __FUNCTION__);

    return (XCAM_RETURN_NO_ERROR);
}
/******************************************************************************
 * AhdrRelease()
 *****************************************************************************/
XCamReturn AdrcRelease(AdrcContext_t* pAdrcCtx) {
    LOG1_ATMO("%s:enter!\n", __FUNCTION__);
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    // initial checks
    if (NULL == pAdrcCtx) {
        return (XCAM_RETURN_ERROR_PARAM);
    }

    result = AdrcStop(pAdrcCtx);
    if (result != XCAM_RETURN_NO_ERROR) {
        LOGE_ATMO("%s: DRC Stop() failed!\n", __FUNCTION__);
        return (result);
    }

    // check state
    if ((ADRC_STATE_RUNNING == pAdrcCtx->state) || (ADRC_STATE_LOCKED == pAdrcCtx->state)) {
        return (XCAM_RETURN_ERROR_MEM);
    }

    free(pAdrcCtx);
    pAdrcCtx = NULL;

    LOG1_ATMO("%s:exit!\n", __FUNCTION__);

    return (XCAM_RETURN_NO_ERROR);
}
