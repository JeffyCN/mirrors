/******************************************************************************
 *
 * Copyright 2019, Fuzhou Rockchip Electronics Co.Ltd. All rights reserved.
 * No part of this work may be reproduced, modified, distributed, transmitted,
 * transcribed, or translated into any language or computer format, in any form
 * or by any means without written permission of:
 * Fuzhou Rockchip Electronics Co.Ltd .
 *
 *
 *****************************************************************************/
/**
 * @file rk_aiq_adrc_algo_v12.cpp
 *
 * @brief
 *   ADD_DESCRIPTION_HERE
 *
 *****************************************************************************/
#include "math.h"
#include "rk_aiq_types_adrc_algo_prvt.h"
#include "xcam_log.h"

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

float DrcGetInterpRatioV12(float* pX, int& lo, int& hi, float CtrlValue, int length_max) {
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

void AdrcV12ClipStAutoParams(AdrcContext_t* pAdrcCtx) {
    LOG1_ATMO("%s:enter!\n", __FUNCTION__);

    for (int i = 0; i < ADRC_ENVLV_STEP_MAX; i++) {
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.DrcGain.DrcGain[i] = LIMIT_VALUE(
            pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.DrcGain.DrcGain[i], DRCGAINMAX, DRCGAINMIN);
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.DrcGain.Alpha[i] =
            LIMIT_VALUE(pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.DrcGain.Alpha[i], NORMALIZE_MAX,
                        NORMALIZE_MIN);
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.DrcGain.Clip[i] = LIMIT_VALUE(
            pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.DrcGain.Clip[i], CLIPMAX, CLIPMIN);

        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.HiLight.HiLightData.Strength[i] =
            LIMIT_VALUE(pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.HiLight.HiLightData.Strength[i],
                        NORMALIZE_MAX, NORMALIZE_MIN);
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.HiLight.HiLightData.gas_t[i] =
            LIMIT_VALUE(pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.HiLight.HiLightData.gas_t[i],
                        GAS_T_MAX, GAS_T_MIN);

        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.LocalData.LocalWeit[i] = LIMIT_VALUE(
            pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.LocalData.LocalWeit[i],
            NORMALIZE_MAX, NORMALIZE_MIN);
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.LocalData.GlobalContrast[i] =
            LIMIT_VALUE(
                pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.LocalData.GlobalContrast[i],
                NORMALIZE_MAX, NORMALIZE_MIN);
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.LocalData.LoLitContrast[i] =
            LIMIT_VALUE(
                pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.LocalData.LoLitContrast[i],
                NORMALIZE_MAX, NORMALIZE_MIN);
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.LocalData.LocalAutoEnable[i] =
            LIMIT_VALUE(
                pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.LocalData.LocalAutoEnable[i],
                NORMALIZE_MAX, NORMALIZE_MIN);
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.LocalData.LocalAutoWeit[i] =
            LIMIT_VALUE(
                pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.LocalData.LocalAutoWeit[i],
                NORMALIZE_MAX, NORMALIZE_MIN);

        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.MotionData.MotionCoef[i] =
            LIMIT_VALUE(
                pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.MotionData.MotionCoef[i],
                NORMALIZE_MAX, NORMALIZE_MIN);
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.MotionData.MotionStr[i] =
            LIMIT_VALUE(
                pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.MotionData.MotionStr[i],
                NORMALIZE_MAX, NORMALIZE_MIN);
    }

    pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.curPixWeit =
        LIMIT_VALUE(pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.curPixWeit,
                    NORMALIZE_MAX, NORMALIZE_MIN);

#if RKAIQ_HAVE_DRC_V12
    pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.preFrameWeit =
        LIMIT_VALUE(pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.preFrameWeit,
                    NORMALIZE_MAX, NORMALIZE_MIN);
    pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.Range_sgm_pre =
        LIMIT_VALUE(pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.Range_sgm_pre,
                    NORMALIZE_MAX, NORMALIZE_MIN);
    pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.Space_sgm_pre =
        LIMIT_VALUE(pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.Space_sgm_pre,
                    SPACESGMMAX, SPACESGMMIN);
#endif
    pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.Range_force_sgm =
        LIMIT_VALUE(pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.Range_force_sgm,
                    NORMALIZE_MAX, NORMALIZE_MIN);
    pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.Range_sgm_cur =
        LIMIT_VALUE(pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.Range_sgm_cur,
                    NORMALIZE_MAX, NORMALIZE_MIN);
    pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.Space_sgm_cur =
        LIMIT_VALUE(pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.Space_sgm_cur,
                    SPACESGMMAX, SPACESGMMIN);
    for (int i = 0; i < ADRC_Y_NUM; i++) {
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.CompressSetting.Manual_curve[i] =
            LIMIT_VALUE_UNSIGNED(
                pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.CompressSetting.Manual_curve[i],
                MANUALCURVEMAX);
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.Scale_y[i] =
            LIMIT_VALUE(pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.Scale_y[i], SCALEYMAX, SCALEYMIN);
    }

    pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.ByPassThr = LIMIT_VALUE(
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.ByPassThr, NORMALIZE_MAX, NORMALIZE_MIN);
    pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.Edge_Weit = LIMIT_VALUE(
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.Edge_Weit, NORMALIZE_MAX, NORMALIZE_MIN);
    pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.IIR_frame =
        LIMIT_VALUE(pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.IIR_frame, IIRFRAMEMAX, IIRFRAMEMIN);
    pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.damp =
        LIMIT_VALUE(pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.damp, NORMALIZE_MAX, NORMALIZE_MIN);

    // drc v12 add
    pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.HiLight.gas_l0 =
        LIMIT_VALUE(pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.HiLight.gas_l0, GAS_L_MAX, GAS_L_MIN);
    pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.HiLight.gas_l1 =
        LIMIT_VALUE(pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.HiLight.gas_l1, GAS_L_MAX, GAS_L_MIN);
    pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.HiLight.gas_l2 =
        LIMIT_VALUE(pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.HiLight.gas_l2, GAS_L_MAX, GAS_L_MIN);
    pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.HiLight.gas_l3 =
        LIMIT_VALUE(pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.HiLight.gas_l3, GAS_L_MAX, GAS_L_MIN);

    LOG1_ATMO("%s:exit!\n", __FUNCTION__);
}

/******************************************************************************
 * CalibrateDrcGainYV12()
 *****************************************************************************/
void CalibrateDrcGainYV12(DrcProcRes_t* para, float DrcGain, float alpha, bool OB_enable,
                          float predgain) {
    LOG1_ATMO("%s:Enter!\n", __FUNCTION__);

    float tmp = 0.0f;
    /*luma[i] = pow((1.0f - luma[i] / 4096.0f), 2.0f)*/
    float luma[DRC_V12_Y_NUM] = {1.0f,    0.8789f, 0.7656f, 0.6602f, 0.5625f, 0.4727f,
                                 0.3906f, 0.3164f, 0.2500f, 0.1914f, 0.1406f, 0.0977f,
                                 0.0625f, 0.0352f, 0.0156f, 0.0039f, 0.0f};

    for (int i = 0; i < DRC_V12_Y_NUM; ++i) {
        if (OB_enable)
            tmp = 1024.0f * pow(DrcGain, 1.0f - alpha * luma[i]) * pow(predgain, -alpha * luma[i]);
        else
            tmp = 1024.0f * pow(DrcGain, 1.0f - alpha * luma[i]);
        para->Drc_v12.gain_y[i] = (unsigned short)(tmp);
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
            if (pAdrcCtx->drcAttrV12.opMode == DRC_OPMODE_AUTO) {
                pAdrcProcRes->bDrcEn = pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.Enable;
            } else if (pAdrcCtx->drcAttrV12.opMode == DRC_OPMODE_MANUAL) {
                pAdrcProcRes->bDrcEn = pAdrcCtx->drcAttrV12.stManual.Enable;
            } else {
                LOGE_ATMO("%s: Drc api in WRONG MODE!!!, drc by pass!!!\n", __FUNCTION__);
                pAdrcProcRes->bDrcEn = false;
            }
        }
    }
    // store bDrcEn for AdrcV12Params2Api
    pAdrcCtx->NextData.bDrcEn = pAdrcProcRes->bDrcEn;

    LOG1_ATMO("%s:exit!\n", __FUNCTION__);
    return pAdrcProcRes->bDrcEn;
}
/******************************************************************************
 * AdrcDampingV12()
 *****************************************************************************/
bool AdrcDampingV12(NextData_t* pNextData, CurrData_t* pCurrData, int FrameID,
                    CtrlDataType_t CtrlDataType) {
    LOG1_ATMO("%s:Enter!\n", __FUNCTION__);
    bool isDampStable = false;

    if (FrameID > INIT_CALC_PARAMS_NUM) {
        bool isDampStable_DrcGain = false, isDampStable_Alpha = false, isDampStable_Clip = false,
             isDampStable_Strength = false, isDampStable_LocalWeit = false,
             isDampStable_LocalAutoWeit = false, isDampStable_GlobalContrast = false,
             isDampStable_LoLitContrast = false, isDampStable_gas_t = false,
             isDampStable_MotionStr = false;

        if ((pNextData->dynParams.Drc_v12.DrcGain - pCurrData->dynParams.Drc_v12.DrcGain) <=
                DAMP_STABLE_THR &&
            (pNextData->dynParams.Drc_v12.DrcGain - pCurrData->dynParams.Drc_v12.DrcGain) >=
                -DAMP_STABLE_THR) {
            isDampStable_DrcGain = true;
        } else {
            pNextData->dynParams.Drc_v12.DrcGain =
                pNextData->staticParams.damp * pNextData->dynParams.Drc_v12.DrcGain +
                (1.0f - pNextData->staticParams.damp) * pCurrData->dynParams.Drc_v12.DrcGain;
            isDampStable_DrcGain = false;
        }
        if ((pNextData->dynParams.Drc_v12.Alpha - pCurrData->dynParams.Drc_v12.Alpha) <=
                DAMP_STABLE_THR &&
            (pNextData->dynParams.Drc_v12.Alpha - pCurrData->dynParams.Drc_v12.Alpha) >=
                -DAMP_STABLE_THR) {
            isDampStable_Alpha = true;
        } else {
            pNextData->dynParams.Drc_v12.Alpha =
                pNextData->staticParams.damp * pNextData->dynParams.Drc_v12.Alpha +
                (1.0f - pNextData->staticParams.damp) * pCurrData->dynParams.Drc_v12.Alpha;
            isDampStable_Alpha = false;
        }
        if ((pNextData->dynParams.Drc_v12.Clip - pCurrData->dynParams.Drc_v12.Clip) <=
                DAMP_STABLE_THR &&
            (pNextData->dynParams.Drc_v12.Clip - pCurrData->dynParams.Drc_v12.Clip) >=
                -DAMP_STABLE_THR) {
            isDampStable_Clip = true;
        } else {
            pNextData->dynParams.Drc_v12.Clip =
                pNextData->staticParams.damp * pNextData->dynParams.Drc_v12.Clip +
                (1.0f - pNextData->staticParams.damp) * pCurrData->dynParams.Drc_v12.Clip;
            isDampStable_Clip = false;
        }
        if ((pNextData->dynParams.Drc_v12.Strength - pCurrData->dynParams.Drc_v12.Strength) <=
                DAMP_STABLE_THR &&
            (pNextData->dynParams.Drc_v12.Strength - pCurrData->dynParams.Drc_v12.Strength) >=
                -DAMP_STABLE_THR) {
            isDampStable_Strength = true;
        } else {
            pNextData->dynParams.Drc_v12.Strength =
                pNextData->staticParams.damp * pNextData->dynParams.Drc_v12.Strength +
                (1.0f - pNextData->staticParams.damp) * pCurrData->dynParams.Drc_v12.Strength;
            isDampStable_Strength = false;
        }
        if ((pNextData->dynParams.Drc_v12.LocalWeit - pCurrData->dynParams.Drc_v12.LocalWeit) <=
                DAMP_STABLE_THR &&
            (pNextData->dynParams.Drc_v12.LocalWeit - pCurrData->dynParams.Drc_v12.LocalWeit) >=
                -DAMP_STABLE_THR) {
            isDampStable_LocalWeit = true;
        } else {
            pNextData->dynParams.Drc_v12.LocalWeit =
                pNextData->staticParams.damp * pNextData->dynParams.Drc_v12.LocalWeit +
                (1.0f - pNextData->staticParams.damp) * pCurrData->dynParams.Drc_v12.LocalWeit;
            isDampStable_LocalWeit = false;
        }
        if ((pNextData->dynParams.Drc_v12.LocalAutoWeit -
             pCurrData->dynParams.Drc_v12.LocalAutoWeit) <= DAMP_STABLE_THR &&
            (pNextData->dynParams.Drc_v12.LocalAutoWeit -
             pCurrData->dynParams.Drc_v12.LocalAutoWeit) >= -DAMP_STABLE_THR) {
            isDampStable_LocalAutoWeit = true;
        } else {
            pNextData->dynParams.Drc_v12.LocalAutoWeit =
                pNextData->staticParams.damp * pNextData->dynParams.Drc_v12.LocalAutoWeit +
                (1.0f - pNextData->staticParams.damp) * pCurrData->dynParams.Drc_v12.LocalAutoWeit;
            isDampStable_LocalAutoWeit = false;
        }
        if ((pNextData->dynParams.Drc_v12.GlobalContrast -
             pCurrData->dynParams.Drc_v12.GlobalContrast) <= DAMP_STABLE_THR &&
            (pNextData->dynParams.Drc_v12.GlobalContrast -
             pCurrData->dynParams.Drc_v12.GlobalContrast) >= -DAMP_STABLE_THR) {
            isDampStable_GlobalContrast = true;
        } else {
            pNextData->dynParams.Drc_v12.GlobalContrast =
                pNextData->staticParams.damp * pNextData->dynParams.Drc_v12.GlobalContrast +
                (1.0f - pNextData->staticParams.damp) * pCurrData->dynParams.Drc_v12.GlobalContrast;
            isDampStable_GlobalContrast = false;
        }
        if ((pNextData->dynParams.Drc_v12.LoLitContrast -
             pCurrData->dynParams.Drc_v12.LoLitContrast) <= DAMP_STABLE_THR &&
            (pNextData->dynParams.Drc_v12.LoLitContrast -
             pCurrData->dynParams.Drc_v12.LoLitContrast) >= -DAMP_STABLE_THR) {
            isDampStable_LoLitContrast = true;
        } else {
            pNextData->dynParams.Drc_v12.LoLitContrast =
                pNextData->staticParams.damp * pNextData->dynParams.Drc_v12.LoLitContrast +
                (1.0f - pNextData->staticParams.damp) * pCurrData->dynParams.Drc_v12.LoLitContrast;
            isDampStable_LoLitContrast = false;
        }
        // drc v12
        if ((pNextData->dynParams.Drc_v12.gas_t - pCurrData->dynParams.Drc_v12.gas_t) <=
                DAMP_STABLE_THR &&
            (pNextData->dynParams.Drc_v12.gas_t - pCurrData->dynParams.Drc_v12.gas_t) >=
                -DAMP_STABLE_THR) {
            isDampStable_gas_t = true;
        } else {
            pNextData->dynParams.Drc_v12.gas_t =
                pNextData->staticParams.damp * pNextData->dynParams.Drc_v12.gas_t +
                (1.0f - pNextData->staticParams.damp) * pCurrData->dynParams.Drc_v12.gas_t;
            isDampStable_gas_t = false;
        }
        if ((pNextData->dynParams.Drc_v12.MotionStr - pCurrData->dynParams.Drc_v12.MotionStr) <=
                DAMP_STABLE_THR &&
            (pNextData->dynParams.Drc_v12.MotionStr - pCurrData->dynParams.Drc_v12.MotionStr) >=
                -DAMP_STABLE_THR) {
            isDampStable_MotionStr = true;
        } else {
            pNextData->dynParams.Drc_v12.MotionStr =
                pNextData->staticParams.damp * pNextData->dynParams.Drc_v12.MotionStr +
                (1.0f - pNextData->staticParams.damp) * pCurrData->dynParams.Drc_v12.MotionStr;
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

/******************************************************************************
 * AdrcGetTuningProcResV12()
 *****************************************************************************/
void AdrcGetTuningProcResV12(AdrcContext_t* pAdrcCtx, RkAiqAdrcProcResult_t* pAdrcProcRes) {
    LOG1_ATMO("%s:enter!\n", __FUNCTION__);

    pAdrcProcRes->DrcProcRes.Drc_v12.gas_t =
        (unsigned short)(SHIFT11BIT(pAdrcCtx->NextData.dynParams.Drc_v12.gas_t) + 0.5);
    pAdrcProcRes->DrcProcRes.Drc_v12.gas_t =
        LIMIT_VALUE_UNSIGNED(pAdrcProcRes->DrcProcRes.Drc_v12.gas_t, BIT_13_MAX);
    pAdrcProcRes->DrcProcRes.Drc_v12.position =
        (unsigned short)(SHIFT8BIT(pAdrcCtx->NextData.dynParams.Drc_v12.Clip) + 0.5f);
    pAdrcProcRes->DrcProcRes.Drc_v12.hpdetail_ratio = LIMIT_VALUE_UNSIGNED(
        pAdrcCtx->NextData.dynParams.Drc_v12.LoLitContrast * BIT_12_MAX, BIT_12_MAX);
    pAdrcProcRes->DrcProcRes.Drc_v12.lpdetail_ratio = LIMIT_VALUE_UNSIGNED(
        pAdrcCtx->NextData.dynParams.Drc_v12.GlobalContrast * BIT_12_MAX, BIT_12_MAX);
    pAdrcProcRes->DrcProcRes.Drc_v12.bilat_wt_off =
        LIMIT_VALUE_UNSIGNED(pAdrcCtx->NextData.dynParams.Drc_v12.MotionStr * BIT_8_MAX, BIT_8_MAX);
    pAdrcProcRes->DrcProcRes.Drc_v12.weig_maxl =
        (unsigned char)(SHIFT4BIT(pAdrcCtx->NextData.dynParams.Drc_v12.Strength) + 0.5f);
    pAdrcProcRes->DrcProcRes.Drc_v12.weig_bilat = LIMIT_VALUE_UNSIGNED(
        pAdrcCtx->NextData.dynParams.Drc_v12.LocalWeit * (BIT_4_MAX + 1), BIT_4_MAX + 1);
    pAdrcProcRes->DrcProcRes.Drc_v12.enable_soft_thd =
        pAdrcCtx->NextData.dynParams.Drc_v12.LocalAutoEnable;
    pAdrcProcRes->DrcProcRes.Drc_v12.bilat_soft_thd = LIMIT_VALUE_UNSIGNED(
        pAdrcCtx->NextData.dynParams.Drc_v12.LocalAutoWeit * BIT_14_MAX, BIT_14_MAX);
    pAdrcProcRes->DrcProcRes.Drc_v12.bilat_soft_thd =
        LIMIT_VALUE_UNSIGNED(pAdrcProcRes->DrcProcRes.Drc_v12.bilat_soft_thd, BIT_14_MAX);
    // get sw_drc_gain_y
    CalibrateDrcGainYV12(&pAdrcProcRes->DrcProcRes, pAdrcCtx->NextData.dynParams.Drc_v12.DrcGain,
                         pAdrcCtx->NextData.dynParams.Drc_v12.Alpha,
                         pAdrcCtx->ablcV32_proc_res.blc_ob_enable,
                         pAdrcCtx->ablcV32_proc_res.isp_ob_predgain);

    pAdrcProcRes->DrcProcRes.Drc_v12.gas_l0 = (unsigned char)pAdrcCtx->NextData.staticParams.gas_l0;
    pAdrcProcRes->DrcProcRes.Drc_v12.gas_l1 = (unsigned char)pAdrcCtx->NextData.staticParams.gas_l1;
    pAdrcProcRes->DrcProcRes.Drc_v12.gas_l2 = (unsigned char)pAdrcCtx->NextData.staticParams.gas_l2;
    pAdrcProcRes->DrcProcRes.Drc_v12.gas_l3 = (unsigned char)pAdrcCtx->NextData.staticParams.gas_l3;
    pAdrcProcRes->DrcProcRes.Drc_v12.weicur_pix =
        LIMIT_VALUE_UNSIGNED(pAdrcCtx->NextData.staticParams.curPixWeit * BIT_8_MAX, BIT_8_MAX);
    pAdrcProcRes->DrcProcRes.Drc_v12.weipre_frame =
        LIMIT_VALUE_UNSIGNED(pAdrcCtx->NextData.staticParams.preFrameWeit * BIT_8_MAX, BIT_8_MAX);

    pAdrcProcRes->DrcProcRes.Drc_v12.force_sgm_inv0 =
        (unsigned short)(SHIFT13BIT(pAdrcCtx->NextData.staticParams.Range_force_sgm) + 0.5f);
    pAdrcProcRes->DrcProcRes.Drc_v12.edge_scl =
        LIMIT_VALUE_UNSIGNED(pAdrcCtx->NextData.staticParams.Edge_Weit * BIT_8_MAX, BIT_8_MAX);
    pAdrcProcRes->DrcProcRes.Drc_v12.motion_scl = SW_DRC_MOTION_SCL_FIX;
    pAdrcProcRes->DrcProcRes.Drc_v12.space_sgm_inv1 =
        (unsigned short)(pAdrcCtx->NextData.staticParams.Space_sgm_cur);
    pAdrcProcRes->DrcProcRes.Drc_v12.space_sgm_inv0 =
        (unsigned short)(pAdrcCtx->NextData.staticParams.Space_sgm_pre);
    pAdrcProcRes->DrcProcRes.Drc_v12.range_sgm_inv1 =
        (unsigned short)(SHIFT13BIT(pAdrcCtx->NextData.staticParams.Range_sgm_cur) + 0.5f);
    pAdrcProcRes->DrcProcRes.Drc_v12.range_sgm_inv0 =
        (unsigned short)(SHIFT13BIT(pAdrcCtx->NextData.staticParams.Range_sgm_pre) + 0.5f);
    for (int i = 0; i < DRC_V12_Y_NUM; ++i) {
        pAdrcProcRes->DrcProcRes.Drc_v12.scale_y[i] =
            (unsigned short)(pAdrcCtx->NextData.staticParams.Scale_y[i]);
    }
    float iir_frame =
        (float)(MIN(pAdrcCtx->FrameID + 1, (uint32_t)pAdrcCtx->NextData.staticParams.IIR_frame));
    pAdrcProcRes->DrcProcRes.Drc_v12.iir_weight =
        (unsigned char)(SHIFT6BIT((iir_frame - 1.0f) / iir_frame) + 0.5f);
    // set FIX reg
    pAdrcProcRes->DrcProcRes.Drc_v12.delta_scalein = DELTA_SCALEIN_FIX;
    pAdrcProcRes->DrcProcRes.Drc_v12.bypass_en     = FUNCTION_DISABLE;
    pAdrcProcRes->DrcProcRes.Drc_v12.offset_pow2   = SW_DRC_OFFSET_POW2_FIX;

    LOG1_ATMO("%s:exit!\n", __FUNCTION__);
}
#if RKAIQ_HAVE_DRC_V12
void AdrcV12Params2Api(AdrcContext_t* pAdrcCtx, DrcInfoV12_t* pDrcInfo) {
    LOG1_ATMO("%s:enter!\n", __FUNCTION__);

    // ctrl info
    pDrcInfo->CtrlInfo.ISO   = pAdrcCtx->NextData.AEData.ISO;
    pDrcInfo->CtrlInfo.EnvLv = pAdrcCtx->NextData.AEData.EnvLv;

    // paras
    pDrcInfo->ValidParams.Enable          = pAdrcCtx->NextData.bDrcEn;
    pDrcInfo->ValidParams.DrcGain.Alpha   = pAdrcCtx->NextData.dynParams.Drc_v12.Alpha;
    pDrcInfo->ValidParams.DrcGain.DrcGain = pAdrcCtx->NextData.dynParams.Drc_v12.DrcGain;
    pDrcInfo->ValidParams.DrcGain.Clip    = pAdrcCtx->NextData.dynParams.Drc_v12.Clip;
    pDrcInfo->ValidParams.HiLight.HiLightData.Strength =
        pAdrcCtx->NextData.dynParams.Drc_v12.Strength;
    pDrcInfo->ValidParams.HiLight.HiLightData.gas_t = pAdrcCtx->NextData.dynParams.Drc_v12.gas_t;
    pDrcInfo->ValidParams.HiLight.gas_l0            = pAdrcCtx->NextData.staticParams.gas_l0;
    pDrcInfo->ValidParams.HiLight.gas_l1            = pAdrcCtx->NextData.staticParams.gas_l1;
    pDrcInfo->ValidParams.HiLight.gas_l2            = pAdrcCtx->NextData.staticParams.gas_l2;
    pDrcInfo->ValidParams.HiLight.gas_l3            = pAdrcCtx->NextData.staticParams.gas_l3;
    pDrcInfo->ValidParams.LocalSetting.LocalData.LocalWeit =
        pAdrcCtx->NextData.dynParams.Drc_v12.LocalWeit;
    pDrcInfo->ValidParams.LocalSetting.LocalData.LocalAutoEnable =
        pAdrcCtx->NextData.dynParams.Drc_v12.LocalAutoEnable;
    pDrcInfo->ValidParams.LocalSetting.LocalData.LocalAutoWeit =
        pAdrcCtx->NextData.dynParams.Drc_v12.LocalAutoWeit;
    pDrcInfo->ValidParams.LocalSetting.LocalData.GlobalContrast =
        pAdrcCtx->NextData.dynParams.Drc_v12.GlobalContrast;
    pDrcInfo->ValidParams.LocalSetting.LocalData.LoLitContrast =
        pAdrcCtx->NextData.dynParams.Drc_v12.LoLitContrast;
    pDrcInfo->ValidParams.LocalSetting.MotionData.MotionStr =
        pAdrcCtx->NextData.dynParams.Drc_v12.MotionStr;
    pDrcInfo->ValidParams.LocalSetting.curPixWeit   = pAdrcCtx->NextData.staticParams.curPixWeit;
    pDrcInfo->ValidParams.LocalSetting.preFrameWeit = pAdrcCtx->NextData.staticParams.preFrameWeit;
    pDrcInfo->ValidParams.LocalSetting.Range_sgm_pre =
        pAdrcCtx->NextData.staticParams.Range_sgm_pre;
    pDrcInfo->ValidParams.LocalSetting.Space_sgm_pre =
        pAdrcCtx->NextData.staticParams.Space_sgm_pre;
    pDrcInfo->ValidParams.LocalSetting.Range_force_sgm =
        pAdrcCtx->NextData.staticParams.Range_force_sgm;
    pDrcInfo->ValidParams.LocalSetting.Range_sgm_cur =
        pAdrcCtx->NextData.staticParams.Range_sgm_cur;
    pDrcInfo->ValidParams.LocalSetting.Space_sgm_cur =
        pAdrcCtx->NextData.staticParams.Space_sgm_cur;
    pDrcInfo->ValidParams.Edge_Weit            = pAdrcCtx->NextData.staticParams.Edge_Weit;
    pDrcInfo->ValidParams.OutPutLongFrame      = pAdrcCtx->NextData.staticParams.OutPutLongFrame;
    pDrcInfo->ValidParams.IIR_frame            = pAdrcCtx->NextData.staticParams.IIR_frame;
    pDrcInfo->ValidParams.CompressSetting.Mode = pAdrcCtx->NextData.staticParams.CompressMode;
    for (int i = 0; i < ADRC_Y_NUM; i++) {
        pDrcInfo->ValidParams.CompressSetting.Manual_curve[i] =
            pAdrcCtx->NextData.staticParams.compresManuCurve[i];
        pDrcInfo->ValidParams.Scale_y[i] = pAdrcCtx->NextData.staticParams.Scale_y[i];
    }

    LOG1_ATMO("%s:exit!\n", __FUNCTION__);
}
#endif
#if RKAIQ_HAVE_DRC_V12_LITE
void AdrcV12LiteParams2Api(AdrcContext_t* pAdrcCtx, DrcInfoV12Lite_t* pDrcInfo) {
    LOG1_ATMO("%s:enter!\n", __FUNCTION__);

    // ctrl info
    pDrcInfo->CtrlInfo.ISO   = pAdrcCtx->NextData.AEData.ISO;
    pDrcInfo->CtrlInfo.EnvLv = pAdrcCtx->NextData.AEData.EnvLv;

    // params
    pDrcInfo->ValidParams.Enable          = pAdrcCtx->NextData.bDrcEn;
    pDrcInfo->ValidParams.DrcGain.Alpha   = pAdrcCtx->NextData.dynParams.Drc_v12.Alpha;
    pDrcInfo->ValidParams.DrcGain.DrcGain = pAdrcCtx->NextData.dynParams.Drc_v12.DrcGain;
    pDrcInfo->ValidParams.DrcGain.Clip    = pAdrcCtx->NextData.dynParams.Drc_v12.Clip;
    pDrcInfo->ValidParams.HiLight.HiLightData.Strength =
        pAdrcCtx->NextData.dynParams.Drc_v12.Strength;
    pDrcInfo->ValidParams.HiLight.HiLightData.gas_t = pAdrcCtx->NextData.dynParams.Drc_v12.gas_t;
    pDrcInfo->ValidParams.HiLight.gas_l0            = pAdrcCtx->NextData.staticParams.gas_l0;
    pDrcInfo->ValidParams.HiLight.gas_l1            = pAdrcCtx->NextData.staticParams.gas_l1;
    pDrcInfo->ValidParams.HiLight.gas_l2            = pAdrcCtx->NextData.staticParams.gas_l2;
    pDrcInfo->ValidParams.HiLight.gas_l3            = pAdrcCtx->NextData.staticParams.gas_l3;
    pDrcInfo->ValidParams.LocalSetting.LocalData.LocalWeit =
        pAdrcCtx->NextData.dynParams.Drc_v12.LocalWeit;
    pDrcInfo->ValidParams.LocalSetting.LocalData.LocalAutoEnable =
        pAdrcCtx->NextData.dynParams.Drc_v12.LocalAutoEnable;
    pDrcInfo->ValidParams.LocalSetting.LocalData.LocalAutoWeit =
        pAdrcCtx->NextData.dynParams.Drc_v12.LocalAutoWeit;
    pDrcInfo->ValidParams.LocalSetting.LocalData.GlobalContrast =
        pAdrcCtx->NextData.dynParams.Drc_v12.GlobalContrast;
    pDrcInfo->ValidParams.LocalSetting.LocalData.LoLitContrast =
        pAdrcCtx->NextData.dynParams.Drc_v12.LoLitContrast;
    pDrcInfo->ValidParams.LocalSetting.MotionData.MotionStr =
        pAdrcCtx->NextData.dynParams.Drc_v12.MotionStr;
    pDrcInfo->ValidParams.LocalSetting.curPixWeit = pAdrcCtx->NextData.staticParams.curPixWeit;
    pDrcInfo->ValidParams.LocalSetting.Range_force_sgm =
        pAdrcCtx->NextData.staticParams.Range_force_sgm;
    pDrcInfo->ValidParams.LocalSetting.Range_sgm_cur =
        pAdrcCtx->NextData.staticParams.Range_sgm_cur;
    pDrcInfo->ValidParams.LocalSetting.Space_sgm_cur =
        pAdrcCtx->NextData.staticParams.Space_sgm_cur;
    pDrcInfo->ValidParams.Edge_Weit            = pAdrcCtx->NextData.staticParams.Edge_Weit;
    pDrcInfo->ValidParams.OutPutLongFrame      = pAdrcCtx->NextData.staticParams.OutPutLongFrame;
    pDrcInfo->ValidParams.IIR_frame            = pAdrcCtx->NextData.staticParams.IIR_frame;
    pDrcInfo->ValidParams.CompressSetting.Mode = pAdrcCtx->NextData.staticParams.CompressMode;
    for (int i = 0; i < ADRC_Y_NUM; i++) {
        pDrcInfo->ValidParams.CompressSetting.Manual_curve[i] =
            pAdrcCtx->NextData.staticParams.compresManuCurve[i];
        pDrcInfo->ValidParams.Scale_y[i] = pAdrcCtx->NextData.staticParams.Scale_y[i];
    }

    LOG1_ATMO("%s:exit!\n", __FUNCTION__);
}
#endif
/******************************************************************************
 * AdrcTuningParaProcessing()
 *get handle para by config and current variate
 *****************************************************************************/
void AdrcTuningParaProcessing(AdrcContext_t* pAdrcCtx, RkAiqAdrcProcResult_t* pAdrcProcRes) {
    LOG1_ATMO("%s:enter!\n", __FUNCTION__);

    // para setting
    if (pAdrcCtx->drcAttrV12.opMode == DRC_OPMODE_AUTO) {
        int lo = 0, hi = 0;
        float ratio     = 0.0f;
        float CtrlValue = pAdrcCtx->NextData.AEData.EnvLv;
        if (pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.CtrlDataType == CTRLDATATYPE_ISO)
            CtrlValue = pAdrcCtx->NextData.AEData.ISO;

        // get Drc gain
        ratio = DrcGetInterpRatioV12(pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.DrcGain.CtrlData, lo,
                                     hi, CtrlValue, ADRC_ENVLV_STEP_MAX);
        pAdrcCtx->NextData.dynParams.Drc_v12.DrcGain =
            ratio * (pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.DrcGain.DrcGain[hi] -
                     pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.DrcGain.DrcGain[lo]) +
            pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.DrcGain.DrcGain[lo];
        pAdrcCtx->NextData.dynParams.Drc_v12.Alpha =
            ratio * (pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.DrcGain.Alpha[hi] -
                     pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.DrcGain.Alpha[lo]) +
            pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.DrcGain.Alpha[lo];
        pAdrcCtx->NextData.dynParams.Drc_v12.Clip =
            ratio * (pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.DrcGain.Clip[hi] -
                     pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.DrcGain.Clip[lo]) +
            pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.DrcGain.Clip[lo];

        // get hi lit
        ratio = DrcGetInterpRatioV12(
            pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.HiLight.HiLightData.CtrlData, lo, hi,
            CtrlValue, ADRC_ENVLV_STEP_MAX);
        pAdrcCtx->NextData.dynParams.Drc_v12.Strength =
            ratio * (pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.HiLight.HiLightData.Strength[hi] -
                     pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.HiLight.HiLightData.Strength[lo]) +
            pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.HiLight.HiLightData.Strength[lo];
        pAdrcCtx->NextData.dynParams.Drc_v12.gas_t =
            ratio * (pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.HiLight.HiLightData.gas_t[hi] -
                     pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.HiLight.HiLightData.gas_t[lo]) +
            pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.HiLight.HiLightData.gas_t[lo];

        // get local
        ratio = DrcGetInterpRatioV12(
            pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.LocalData.CtrlData, lo, hi,
            CtrlValue, ADRC_ENVLV_STEP_MAX);
        pAdrcCtx->NextData.dynParams.Drc_v12.LocalWeit =
            ratio *
                (pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.LocalData.LocalWeit[hi] -
                 pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.LocalData.LocalWeit[lo]) +
            pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.LocalData.LocalWeit[lo];
        pAdrcCtx->NextData.dynParams.Drc_v12.GlobalContrast =
            ratio * (pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.LocalData
                         .GlobalContrast[hi] -
                     pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.LocalData
                         .GlobalContrast[lo]) +
            pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.LocalData.GlobalContrast[lo];
        pAdrcCtx->NextData.dynParams.Drc_v12.LoLitContrast =
            ratio * (pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.LocalData
                         .LoLitContrast[hi] -
                     pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.LocalData
                         .LoLitContrast[lo]) +
            pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.LocalData.LoLitContrast[lo];
        pAdrcCtx->NextData.dynParams.Drc_v12.LocalAutoEnable =
            pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.LocalData.LocalAutoEnable[lo];
        pAdrcCtx->NextData.dynParams.Drc_v12.LocalAutoWeit =
            ratio * (pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.LocalData
                         .LocalAutoWeit[hi] -
                     pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.LocalData
                         .LocalAutoWeit[lo]) +
            pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.LocalData.LocalAutoWeit[lo];
        // get MotionStr
        pAdrcCtx->NextData.dynParams.Drc_v12.MotionStr =
            pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.MotionData.MotionStr[0];
        // compress
        pAdrcCtx->NextData.staticParams.CompressMode =
            pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.CompressSetting.Mode;
        for (int i = 0; i < ADRC_Y_NUM; i++)
            pAdrcCtx->NextData.staticParams.compresManuCurve[i] =
                pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.CompressSetting.Manual_curve[i];
        pAdrcCtx->NextData.staticParams.curPixWeit =
            pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.curPixWeit;
#if RKAIQ_HAVE_DRC_V12
        pAdrcCtx->NextData.staticParams.preFrameWeit =
            pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.preFrameWeit;
        pAdrcCtx->NextData.staticParams.Range_sgm_pre =
            pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.Range_sgm_pre;
        pAdrcCtx->NextData.staticParams.Space_sgm_pre =
            pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.Space_sgm_pre;
#else
        pAdrcCtx->NextData.staticParams.preFrameWeit  = 0x0;
        pAdrcCtx->NextData.staticParams.Range_sgm_pre = 0x0;
        pAdrcCtx->NextData.staticParams.Space_sgm_pre = 0x0;
#endif
        pAdrcCtx->NextData.staticParams.Range_force_sgm =
            pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.Range_force_sgm;
        pAdrcCtx->NextData.staticParams.Range_sgm_cur =
            pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.Range_sgm_cur;
        pAdrcCtx->NextData.staticParams.Space_sgm_cur =
            pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.Space_sgm_cur;
        // scale y
        for (int i = 0; i < ADRC_Y_NUM; i++)
            pAdrcCtx->NextData.staticParams.Scale_y[i] =
                (unsigned short)pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.Scale_y[i];

        pAdrcCtx->NextData.staticParams.ByPassThr =
            pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.ByPassThr;
        pAdrcCtx->NextData.staticParams.Edge_Weit =
            pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.Edge_Weit;
        pAdrcCtx->NextData.staticParams.IIR_frame =
            pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.IIR_frame;
        pAdrcCtx->NextData.staticParams.damp = pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.damp;
        // drc v12 add
        pAdrcCtx->NextData.staticParams.gas_l0 =
            pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.HiLight.gas_l0;
        pAdrcCtx->NextData.staticParams.gas_l1 =
            pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.HiLight.gas_l1;
        pAdrcCtx->NextData.staticParams.gas_l2 =
            pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.HiLight.gas_l2;
        pAdrcCtx->NextData.staticParams.gas_l3 =
            pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.HiLight.gas_l3;
        pAdrcCtx->NextData.staticParams.OutPutLongFrame =
            pAdrcCtx->NextData.AEData.LongFrmMode ||
            pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.OutPutLongFrame;

        // damp
        pAdrcCtx->isDampStable =
            AdrcDampingV12(&pAdrcCtx->NextData, &pAdrcCtx->CurrData, pAdrcCtx->FrameID,
                           pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.CtrlDataType);
    } else if (pAdrcCtx->drcAttrV12.opMode == DRC_OPMODE_MANUAL) {
        // update drc gain
        pAdrcCtx->NextData.dynParams.Drc_v12.DrcGain =
            LIMIT_VALUE(pAdrcCtx->drcAttrV12.stManual.DrcGain.DrcGain, DRCGAINMAX, DRCGAINMIN);
        pAdrcCtx->NextData.dynParams.Drc_v12.Alpha =
            LIMIT_VALUE(pAdrcCtx->drcAttrV12.stManual.DrcGain.Alpha, NORMALIZE_MAX, NORMALIZE_MIN);
        pAdrcCtx->NextData.dynParams.Drc_v12.Clip =
            LIMIT_VALUE(pAdrcCtx->drcAttrV12.stManual.DrcGain.Clip, CLIPMAX, CLIPMIN);

        // update hight light
        pAdrcCtx->NextData.dynParams.Drc_v12.Strength =
            LIMIT_VALUE(pAdrcCtx->drcAttrV12.stManual.HiLight.HiLightData.Strength, NORMALIZE_MAX,
                        NORMALIZE_MIN);
        pAdrcCtx->NextData.dynParams.Drc_v12.gas_t = LIMIT_VALUE(
            pAdrcCtx->drcAttrV12.stManual.HiLight.HiLightData.gas_t, GAS_T_MAX, GAS_T_MIN);

        // update local
        pAdrcCtx->NextData.dynParams.Drc_v12.LocalWeit =
            LIMIT_VALUE(pAdrcCtx->drcAttrV12.stManual.LocalSetting.LocalData.LocalWeit,
                        NORMALIZE_MAX, NORMALIZE_MIN);
        pAdrcCtx->NextData.dynParams.Drc_v12.LocalAutoEnable =
            LIMIT_VALUE(pAdrcCtx->drcAttrV12.stManual.LocalSetting.LocalData.LocalAutoEnable,
                        ADRCNORMALIZEINTMAX, ADRCNORMALIZEINTMIN);
        pAdrcCtx->NextData.dynParams.Drc_v12.LocalAutoWeit =
            LIMIT_VALUE(pAdrcCtx->drcAttrV12.stManual.LocalSetting.LocalData.LocalAutoWeit,
                        NORMALIZE_MAX, NORMALIZE_MIN);
        pAdrcCtx->NextData.dynParams.Drc_v12.GlobalContrast =
            LIMIT_VALUE(pAdrcCtx->drcAttrV12.stManual.LocalSetting.LocalData.GlobalContrast,
                        NORMALIZE_MAX, NORMALIZE_MIN);
        pAdrcCtx->NextData.dynParams.Drc_v12.LoLitContrast =
            LIMIT_VALUE(pAdrcCtx->drcAttrV12.stManual.LocalSetting.LocalData.LoLitContrast,
                        NORMALIZE_MAX, NORMALIZE_MIN);
        pAdrcCtx->NextData.dynParams.Drc_v12.LoLitContrast =
            LIMIT_VALUE(pAdrcCtx->drcAttrV12.stManual.LocalSetting.MotionData.MotionStr,
                        NORMALIZE_MAX, NORMALIZE_MIN);

        // damp
        pAdrcCtx->isDampStable = true;

        // others
        // compress
        pAdrcCtx->NextData.staticParams.CompressMode =
            pAdrcCtx->drcAttrV12.stManual.CompressSetting.Mode;
        for (int i = 0; i < ADRC_Y_NUM; i++)
            pAdrcCtx->NextData.staticParams.compresManuCurve[i] = LIMIT_VALUE_UNSIGNED(
                pAdrcCtx->drcAttrV12.stManual.CompressSetting.Manual_curve[i], MANUALCURVEMAX);

        pAdrcCtx->NextData.staticParams.OutPutLongFrame =
            pAdrcCtx->NextData.AEData.LongFrmMode || pAdrcCtx->drcAttrV12.stManual.OutPutLongFrame;
        pAdrcCtx->NextData.staticParams.curPixWeit = LIMIT_VALUE(
            pAdrcCtx->drcAttrV12.stManual.LocalSetting.curPixWeit, NORMALIZE_MAX, NORMALIZE_MIN);
#if RKAIQ_HAVE_DRC_V12
        pAdrcCtx->NextData.staticParams.preFrameWeit = LIMIT_VALUE(
            pAdrcCtx->drcAttrV12.stManual.LocalSetting.preFrameWeit, NORMALIZE_MAX, NORMALIZE_MIN);
        pAdrcCtx->NextData.staticParams.Range_sgm_pre = LIMIT_VALUE(
            pAdrcCtx->drcAttrV12.stManual.LocalSetting.Range_sgm_pre, NORMALIZE_MAX, NORMALIZE_MIN);
        pAdrcCtx->NextData.staticParams.Space_sgm_pre = LIMIT_VALUE(
            pAdrcCtx->drcAttrV12.stManual.LocalSetting.Space_sgm_pre, SPACESGMMAX, SPACESGMMIN);
#else
        pAdrcCtx->NextData.staticParams.preFrameWeit = 0x0;
        pAdrcCtx->NextData.staticParams.Range_sgm_pre = 0x0;
        pAdrcCtx->NextData.staticParams.Space_sgm_pre = 0x0;
#endif
        pAdrcCtx->NextData.staticParams.Range_force_sgm =
            LIMIT_VALUE(pAdrcCtx->drcAttrV12.stManual.LocalSetting.Range_force_sgm, NORMALIZE_MAX,
                        NORMALIZE_MIN);
        pAdrcCtx->NextData.staticParams.Range_sgm_cur = LIMIT_VALUE(
            pAdrcCtx->drcAttrV12.stManual.LocalSetting.Range_sgm_cur, NORMALIZE_MAX, NORMALIZE_MIN);

        pAdrcCtx->NextData.staticParams.Space_sgm_cur = LIMIT_VALUE(
            pAdrcCtx->drcAttrV12.stManual.LocalSetting.Space_sgm_cur, SPACESGMMAX, SPACESGMMIN);
        for (int i = 0; i < ADRC_Y_NUM; i++)
            pAdrcCtx->NextData.staticParams.Scale_y[i] =
                LIMIT_VALUE(pAdrcCtx->drcAttrV12.stManual.Scale_y[i], SCALEYMAX, SCALEYMIN);
        pAdrcCtx->NextData.staticParams.Edge_Weit =
            LIMIT_VALUE(pAdrcCtx->drcAttrV12.stManual.Edge_Weit, NORMALIZE_MAX, NORMALIZE_MIN);
        pAdrcCtx->NextData.staticParams.IIR_frame =
            LIMIT_VALUE(pAdrcCtx->drcAttrV12.stManual.IIR_frame, IIRFRAMEMAX, IIRFRAMEMIN);

        // drc v12 add
        pAdrcCtx->NextData.staticParams.gas_l0 =
            LIMIT_VALUE(pAdrcCtx->drcAttrV12.stManual.HiLight.gas_l0, GAS_L_MAX, GAS_L_MIN);
        pAdrcCtx->NextData.staticParams.gas_l1 =
            LIMIT_VALUE(pAdrcCtx->drcAttrV12.stManual.HiLight.gas_l1, GAS_L_MAX, GAS_L_MIN);
        pAdrcCtx->NextData.staticParams.gas_l2 =
            LIMIT_VALUE(pAdrcCtx->drcAttrV12.stManual.HiLight.gas_l2, GAS_L_MAX, GAS_L_MIN);
        pAdrcCtx->NextData.staticParams.gas_l3 =
            LIMIT_VALUE(pAdrcCtx->drcAttrV12.stManual.HiLight.gas_l3, GAS_L_MAX, GAS_L_MIN);
    }

    // clip drc gain
    if (pAdrcCtx->FrameNumber == HDR_2X_NUM || pAdrcCtx->FrameNumber == HDR_3X_NUM) {
        if (pAdrcCtx->NextData.AEData.L2S_Ratio * pAdrcCtx->NextData.dynParams.Drc_v12.DrcGain >
            MAX_AE_DRC_GAIN) {
            pAdrcCtx->NextData.dynParams.Drc_v12.DrcGain =
                MAX(MAX_AE_DRC_GAIN / pAdrcCtx->NextData.AEData.L2S_Ratio, GAINMIN);
            LOGI_ATMO("%s:  AERatio*DrcGain > 256x, DrcGain Clip to %f!!!\n", __FUNCTION__,
                      pAdrcCtx->NextData.dynParams.Drc_v12.DrcGain);
        }
    } else if (pAdrcCtx->FrameNumber == LINEAR_NUM) {
        if (pAdrcCtx->ablcV32_proc_res.isp_ob_predgain *
                pAdrcCtx->NextData.dynParams.Drc_v12.DrcGain >
            MAX_AE_DRC_GAIN) {
            if (pAdrcCtx->ablcV32_proc_res.isp_ob_predgain > MAX_AE_DRC_GAIN)
                LOGE_ATMO("%s:  predgain > 256x!!!\n", __FUNCTION__);
            else
                pAdrcCtx->NextData.dynParams.Drc_v12.DrcGain =
                    MAX(MAX_AE_DRC_GAIN / pAdrcCtx->ablcV32_proc_res.isp_ob_predgain, GAINMIN);
            LOGI_ATMO("%s:  predgain*DrcGain > 256x, DrcGain clip to %f!!!\n", __FUNCTION__,
                      pAdrcCtx->NextData.dynParams.Drc_v12.DrcGain);
        }
    } else if (pAdrcCtx->FrameNumber == SENSOR_MGE) {
        if (pow(2.0f, float(pAdrcCtx->compr_bit - ISP_HDR_BIT_NUM_MIN)) *
                pAdrcCtx->NextData.dynParams.Drc_v12.DrcGain >
            MAX_AE_DRC_GAIN) {
            if (pow(2.0f, float(pAdrcCtx->compr_bit - ISP_HDR_BIT_NUM_MIN)) > MAX_AE_DRC_GAIN)
                LOGE_ATMO("%s:  SensorMgeRatio > 256x!!!\n", __FUNCTION__);
            else
                pAdrcCtx->NextData.dynParams.Drc_v12.DrcGain = MAX(
                    MAX_AE_DRC_GAIN / pow(2.0f, float(pAdrcCtx->compr_bit - ISP_HDR_BIT_NUM_MIN)),
                    GAINMIN);
            LOGI_ATMO("%s:  SensorMgeRatio*DrcGain > 256x, DrcGain clip to %f!!!\n", __FUNCTION__,
                      pAdrcCtx->NextData.dynParams.Drc_v12.DrcGain);
        }
    }
    // clip gas_l0~3
    if (pAdrcCtx->NextData.staticParams.gas_l0 == GAS_L_MAX) {
        LOGE_ATMO("%s: gas_l0 equals %d, use default value\n", __FUNCTION__, GAS_L_MAX);
        pAdrcCtx->NextData.staticParams.gas_l0 = GAS_L0_DEFAULT;
        pAdrcCtx->NextData.staticParams.gas_l1 = GAS_L1_DEFAULT;
        pAdrcCtx->NextData.staticParams.gas_l2 = GAS_L2_DEFAULT;
        pAdrcCtx->NextData.staticParams.gas_l3 = GAS_L3_DEFAULT;
    }
    if ((pAdrcCtx->NextData.staticParams.gas_l0 + 2 * pAdrcCtx->NextData.staticParams.gas_l1 +
         pAdrcCtx->NextData.staticParams.gas_l2 + 2 * pAdrcCtx->NextData.staticParams.gas_l3) !=
        GAS_L_MAX) {
        LOGE_ATMO("%s: gas_l0 + gas_l1 + gas_l2 + gas_l3 DO NOT equal %d, use default value\n",
                  __FUNCTION__, GAS_L_MAX);
        pAdrcCtx->NextData.staticParams.gas_l0 = GAS_L0_DEFAULT;
        pAdrcCtx->NextData.staticParams.gas_l1 = GAS_L1_DEFAULT;
        pAdrcCtx->NextData.staticParams.gas_l2 = GAS_L2_DEFAULT;
        pAdrcCtx->NextData.staticParams.gas_l3 = GAS_L3_DEFAULT;
    }

    LOGD_ATMO(
        "%s: Current ob_on:%d predgain:%f DrcGain:%f Alpha:%f Clip:%f CompressMode:%d "
        "OutPutLongFrame:%d\n",
        __FUNCTION__, pAdrcCtx->ablcV32_proc_res.blc_ob_enable,
        pAdrcCtx->ablcV32_proc_res.isp_ob_predgain, pAdrcCtx->NextData.dynParams.Drc_v12.DrcGain,
        pAdrcCtx->NextData.dynParams.Drc_v12.Alpha, pAdrcCtx->NextData.dynParams.Drc_v12.Clip,
        pAdrcCtx->NextData.staticParams.CompressMode,
        pAdrcCtx->NextData.staticParams.OutPutLongFrame);
    LOGD_ATMO("%s: Current HiLight Strength:%f gas_t:%f\n", __FUNCTION__,
              pAdrcCtx->NextData.dynParams.Drc_v12.Strength,
              pAdrcCtx->NextData.dynParams.Drc_v12.gas_t);
    LOGD_ATMO(
        "%s: Current LocalWeit:%f LocalAutoEnable:%d LocalAutoWeit:%f GlobalContrast:%f "
        "LoLitContrast:%f MotionStr:%f\n",
        __FUNCTION__, pAdrcCtx->NextData.dynParams.Drc_v12.LocalWeit,
        pAdrcCtx->NextData.dynParams.Drc_v12.LocalAutoEnable,
        pAdrcCtx->NextData.dynParams.Drc_v12.LocalAutoWeit,
        pAdrcCtx->NextData.dynParams.Drc_v12.GlobalContrast,
        pAdrcCtx->NextData.dynParams.Drc_v12.LoLitContrast,
        pAdrcCtx->NextData.dynParams.Drc_v12.MotionStr);

    // get io data
    AdrcGetTuningProcResV12(pAdrcCtx, pAdrcProcRes);

    // store current handle data to pre data for next loop
    pAdrcCtx->CurrData.AEData.EnvLv = pAdrcCtx->NextData.AEData.EnvLv;
    pAdrcCtx->CurrData.AEData.ISO   = pAdrcCtx->NextData.AEData.ISO;
    pAdrcCtx->CurrData.MotionCoef   = pAdrcCtx->NextData.MotionCoef;
    pAdrcCtx->CurrData.ApiMode      = pAdrcCtx->drcAttrV12.opMode;
    if (pAdrcCtx->drcAttrV12.opMode == DRC_OPMODE_AUTO && !pAdrcCtx->isDampStable) {
        pAdrcCtx->CurrData.dynParams.Drc_v12.DrcGain = pAdrcCtx->NextData.dynParams.Drc_v12.DrcGain;
        pAdrcCtx->CurrData.dynParams.Drc_v12.Alpha   = pAdrcCtx->NextData.dynParams.Drc_v12.Alpha;
        pAdrcCtx->CurrData.dynParams.Drc_v12.Clip    = pAdrcCtx->NextData.dynParams.Drc_v12.Clip;
        pAdrcCtx->CurrData.dynParams.Drc_v12.Strength =
            pAdrcCtx->NextData.dynParams.Drc_v12.Strength;
        pAdrcCtx->CurrData.dynParams.Drc_v12.LocalWeit =
            pAdrcCtx->NextData.dynParams.Drc_v12.LocalWeit;
        pAdrcCtx->CurrData.dynParams.Drc_v12.LocalAutoWeit =
            pAdrcCtx->NextData.dynParams.Drc_v12.LocalAutoWeit;
        pAdrcCtx->CurrData.dynParams.Drc_v12.GlobalContrast =
            pAdrcCtx->NextData.dynParams.Drc_v12.GlobalContrast;
        pAdrcCtx->CurrData.dynParams.Drc_v12.LoLitContrast =
            pAdrcCtx->NextData.dynParams.Drc_v12.LoLitContrast;
        pAdrcCtx->CurrData.dynParams.Drc_v12.gas_t = pAdrcCtx->NextData.dynParams.Drc_v12.gas_t;
        pAdrcCtx->CurrData.dynParams.Drc_v12.MotionStr =
            pAdrcCtx->NextData.dynParams.Drc_v12.MotionStr;
    }

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
        adrc_gain = pAdrcCtx->NextData.dynParams.Drc_v12.DrcGain *
                    pAdrcCtx->ablcV32_proc_res.isp_ob_predgain;
    else
        adrc_gain = pAdrcCtx->NextData.dynParams.Drc_v12.DrcGain;
    float log_ratio2     = log(pAdrcCtx->NextData.AEData.L2S_Ratio * adrc_gain) / log(2.0f) + 12.0f;
    float offsetbits_int = (float)(pAdrcProcRes->DrcProcRes.Drc_v12.offset_pow2);
    float offsetbits     = offsetbits_int * (1 << MFHDR_LOG_Q_BITS);
    float hdrbits        = log_ratio2 * (1 << MFHDR_LOG_Q_BITS);
    float hdrvalidbits   = hdrbits - offsetbits;
    float compres_scl    = (12.0f * (1 << (MFHDR_LOG_Q_BITS * 2))) / hdrvalidbits;
    pAdrcProcRes->DrcProcRes.Drc_v12.compres_scl = (unsigned short)(compres_scl);

    // get sw_drc_min_ogain
    if (pAdrcCtx->NextData.staticParams.OutPutLongFrame)
        pAdrcProcRes->DrcProcRes.Drc_v12.min_ogain = 1 << 15;
    else {
        float sw_drc_min_ogain = 1.0f / (pAdrcCtx->NextData.AEData.L2S_Ratio * adrc_gain);
        pAdrcProcRes->DrcProcRes.Drc_v12.min_ogain =
            (unsigned short)(sw_drc_min_ogain * (1 << 15) + 0.5f);
    }

    // get sw_drc_compres_y
    if (pAdrcCtx->NextData.staticParams.CompressMode == COMPRESS_AUTO) {
        float curveparam, curveparam2, curveparam3, tmp;
        float luma2[DRC_V12_Y_NUM] = {0.0f,     1024.0f,  2048.0f,  3072.0f,  4096.0f,  5120.0f,
                                      6144.0f,  7168.0f,  8192.0f,  10240.0f, 12288.0f, 14336.0f,
                                      16384.0f, 18432.0f, 20480.0f, 22528.0f, 24576.0f};
        float curveTable;
        float dstbits   = ISP_RAW_BIT * (1 << MFHDR_LOG_Q_BITS);
        float validbits = dstbits - offsetbits;
        for (int i = 0; i < DRC_V12_Y_NUM; ++i) {
            curveparam =
                (float)(validbits - 0.0f) / (hdrvalidbits - validbits + DRC_COMPRESS_Y_OFFSET);
            curveparam2 = validbits * (1.0f + curveparam);
            curveparam3   = hdrvalidbits * curveparam;
            tmp           = luma2[i] * hdrvalidbits / 24576.0f;
            curveTable    = (tmp * curveparam2 / (tmp + curveparam3));
            pAdrcProcRes->DrcProcRes.Drc_v12.compres_y[i] = (unsigned short)(curveTable);
        }
    } else if (pAdrcCtx->NextData.staticParams.CompressMode == COMPRESS_MANUAL) {
        for (int i = 0; i < DRC_V12_Y_NUM; ++i)
            pAdrcProcRes->DrcProcRes.Drc_v12.compres_y[i] =
                (unsigned short)(pAdrcCtx->NextData.staticParams.compresManuCurve[i]);
    }

    LOGV_ATMO("%s: nextRatioLS:%f sw_drc_position:%d sw_drc_compres_scl:%d sw_drc_offset_pow2:%d\n",
              __FUNCTION__, pAdrcCtx->NextData.AEData.L2S_Ratio,
              pAdrcProcRes->DrcProcRes.Drc_v12.position,
              pAdrcProcRes->DrcProcRes.Drc_v12.compres_scl,
              pAdrcProcRes->DrcProcRes.Drc_v12.offset_pow2);
    LOGV_ATMO("%s: blc_ob_enable:%d OB_predgain:%f DrcGain:%f TotalDgain:%f\n", __FUNCTION__,
              pAdrcCtx->ablcV32_proc_res.blc_ob_enable, pAdrcCtx->ablcV32_proc_res.isp_ob_predgain,
              pAdrcCtx->NextData.dynParams.Drc_v12.DrcGain, adrc_gain);
    LOGV_ATMO("%s: sw_drc_lpdetail_ratio:%d sw_drc_hpdetail_ratio:%d sw_drc_delta_scalein:%d\n",
              __FUNCTION__, pAdrcProcRes->DrcProcRes.Drc_v12.lpdetail_ratio,
              pAdrcProcRes->DrcProcRes.Drc_v12.hpdetail_ratio,
              pAdrcProcRes->DrcProcRes.Drc_v12.delta_scalein);
    LOGV_ATMO("%s: sw_drc_bilat_wt_off:%d sw_drc_weipre_frame:%d sw_drc_weicur_pix:%d\n",
              __FUNCTION__, pAdrcProcRes->DrcProcRes.Drc_v12.bilat_wt_off,
              pAdrcProcRes->DrcProcRes.Drc_v12.weipre_frame,
              pAdrcProcRes->DrcProcRes.Drc_v12.weicur_pix);
    LOGV_ATMO("%s: sw_drc_edge_scl:%d sw_drc_motion_scl:%d sw_drc_force_sgm_inv0:%d\n",
              __FUNCTION__, pAdrcProcRes->DrcProcRes.Drc_v12.edge_scl,
              pAdrcProcRes->DrcProcRes.Drc_v12.motion_scl,
              pAdrcProcRes->DrcProcRes.Drc_v12.force_sgm_inv0);
    LOGV_ATMO("%s: sw_drc_space_sgm_inv0:%d sw_drc_space_sgm_inv1:%d\n", __FUNCTION__,
              pAdrcProcRes->DrcProcRes.Drc_v12.space_sgm_inv0,
              pAdrcProcRes->DrcProcRes.Drc_v12.space_sgm_inv1);
    LOGV_ATMO("%s: sw_drc_range_sgm_inv0:%d sw_drc_range_sgm_inv1:%d\n", __FUNCTION__,
              pAdrcProcRes->DrcProcRes.Drc_v12.range_sgm_inv0,
              pAdrcProcRes->DrcProcRes.Drc_v12.range_sgm_inv1);
    LOGV_ATMO(
        "%s: sw_drc_weig_bilat:%d sw_drc_weig_maxl:%d sw_drc_bilat_soft_thd:%d "
        "sw_drc_enable_soft_thd:%d\n",
        __FUNCTION__, pAdrcProcRes->DrcProcRes.Drc_v12.weig_bilat,
        pAdrcProcRes->DrcProcRes.Drc_v12.weig_maxl, pAdrcProcRes->DrcProcRes.Drc_v12.bilat_soft_thd,
        pAdrcProcRes->DrcProcRes.Drc_v12.enable_soft_thd);
    LOGV_ATMO("%s: sw_drc_min_ogain:%d sw_drc_iir_weight:%d\n", __FUNCTION__,
              pAdrcProcRes->DrcProcRes.Drc_v12.min_ogain,
              pAdrcProcRes->DrcProcRes.Drc_v12.iir_weight);
    LOGV_ATMO("%s: gas_t:%d gas_l0:%d gas_l1:%d gas_l2:%d gas_l3:%d\n", __FUNCTION__,
              pAdrcProcRes->DrcProcRes.Drc_v12.gas_t, pAdrcProcRes->DrcProcRes.Drc_v12.gas_l0,
              pAdrcProcRes->DrcProcRes.Drc_v12.gas_l1, pAdrcProcRes->DrcProcRes.Drc_v12.gas_l2,
              pAdrcProcRes->DrcProcRes.Drc_v12.gas_l3);
    LOGV_ATMO(
        "%s: sw_drc_gain_y: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n", __FUNCTION__,
        pAdrcProcRes->DrcProcRes.Drc_v12.gain_y[0], pAdrcProcRes->DrcProcRes.Drc_v12.gain_y[1],
        pAdrcProcRes->DrcProcRes.Drc_v12.gain_y[2], pAdrcProcRes->DrcProcRes.Drc_v12.gain_y[3],
        pAdrcProcRes->DrcProcRes.Drc_v12.gain_y[4], pAdrcProcRes->DrcProcRes.Drc_v12.gain_y[5],
        pAdrcProcRes->DrcProcRes.Drc_v12.gain_y[6], pAdrcProcRes->DrcProcRes.Drc_v12.gain_y[7],
        pAdrcProcRes->DrcProcRes.Drc_v12.gain_y[8], pAdrcProcRes->DrcProcRes.Drc_v12.gain_y[9],
        pAdrcProcRes->DrcProcRes.Drc_v12.gain_y[10], pAdrcProcRes->DrcProcRes.Drc_v12.gain_y[11],
        pAdrcProcRes->DrcProcRes.Drc_v12.gain_y[12], pAdrcProcRes->DrcProcRes.Drc_v12.gain_y[13],
        pAdrcProcRes->DrcProcRes.Drc_v12.gain_y[14], pAdrcProcRes->DrcProcRes.Drc_v12.gain_y[15],
        pAdrcProcRes->DrcProcRes.Drc_v12.gain_y[16]);
    LOGV_ATMO(
        "%s: sw_drc_scale_y: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n", __FUNCTION__,
        pAdrcProcRes->DrcProcRes.Drc_v12.scale_y[0], pAdrcProcRes->DrcProcRes.Drc_v12.scale_y[1],
        pAdrcProcRes->DrcProcRes.Drc_v12.scale_y[2], pAdrcProcRes->DrcProcRes.Drc_v12.scale_y[3],
        pAdrcProcRes->DrcProcRes.Drc_v12.scale_y[4], pAdrcProcRes->DrcProcRes.Drc_v12.scale_y[5],
        pAdrcProcRes->DrcProcRes.Drc_v12.scale_y[6], pAdrcProcRes->DrcProcRes.Drc_v12.scale_y[7],
        pAdrcProcRes->DrcProcRes.Drc_v12.scale_y[8], pAdrcProcRes->DrcProcRes.Drc_v12.scale_y[9],
        pAdrcProcRes->DrcProcRes.Drc_v12.scale_y[10], pAdrcProcRes->DrcProcRes.Drc_v12.scale_y[11],
        pAdrcProcRes->DrcProcRes.Drc_v12.scale_y[12], pAdrcProcRes->DrcProcRes.Drc_v12.scale_y[13],
        pAdrcProcRes->DrcProcRes.Drc_v12.scale_y[14], pAdrcProcRes->DrcProcRes.Drc_v12.scale_y[15],
        pAdrcProcRes->DrcProcRes.Drc_v12.scale_y[16]);
    LOGV_ATMO(
        "%s: CompressMode:%d sw_drc_compres_y: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d "
        "%d\n",
        __FUNCTION__, pAdrcCtx->NextData.staticParams.CompressMode,
        pAdrcProcRes->DrcProcRes.Drc_v12.compres_y[0],
        pAdrcProcRes->DrcProcRes.Drc_v12.compres_y[1],
        pAdrcProcRes->DrcProcRes.Drc_v12.compres_y[2],
        pAdrcProcRes->DrcProcRes.Drc_v12.compres_y[3],
        pAdrcProcRes->DrcProcRes.Drc_v12.compres_y[4],
        pAdrcProcRes->DrcProcRes.Drc_v12.compres_y[5],
        pAdrcProcRes->DrcProcRes.Drc_v12.compres_y[6],
        pAdrcProcRes->DrcProcRes.Drc_v12.compres_y[7],
        pAdrcProcRes->DrcProcRes.Drc_v12.compres_y[8],
        pAdrcProcRes->DrcProcRes.Drc_v12.compres_y[9],
        pAdrcProcRes->DrcProcRes.Drc_v12.compres_y[10],
        pAdrcProcRes->DrcProcRes.Drc_v12.compres_y[11],
        pAdrcProcRes->DrcProcRes.Drc_v12.compres_y[12],
        pAdrcProcRes->DrcProcRes.Drc_v12.compres_y[13],
        pAdrcProcRes->DrcProcRes.Drc_v12.compres_y[14],
        pAdrcProcRes->DrcProcRes.Drc_v12.compres_y[15],
        pAdrcProcRes->DrcProcRes.Drc_v12.compres_y[16]);

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
    else if (pAdrcCtx->drcAttrV12.opMode != pAdrcCtx->CurrData.ApiMode)
        bypass = false;
    else if (pAdrcCtx->drcAttrV12.opMode == DRC_OPMODE_MANUAL)
        bypass = !pAdrcCtx->ifReCalcStManual;
    else if (pAdrcCtx->drcAttrV12.opMode == DRC_OPMODE_AUTO) {
        if (pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.CtrlDataType == CTRLDATATYPE_ENVLV) {
            diff = pAdrcCtx->CurrData.AEData.EnvLv - pAdrcCtx->NextData.AEData.EnvLv;
            if (pAdrcCtx->CurrData.AEData.EnvLv <= FLT_EPSILON) {
                diff = pAdrcCtx->NextData.AEData.EnvLv;
                if (diff <= FLT_EPSILON)
                    bypass = true;
                else
                    bypass = false;
            } else {
                diff /= pAdrcCtx->CurrData.AEData.EnvLv;
                if (diff >= pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.ByPassThr ||
                    diff <= -pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.ByPassThr)
                    bypass = false;
                else
                    bypass = true;
            }
        } else if (pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.CtrlDataType == CTRLDATATYPE_ISO) {
            diff = pAdrcCtx->CurrData.AEData.ISO - pAdrcCtx->NextData.AEData.ISO;
            diff /= pAdrcCtx->CurrData.AEData.ISO;
            if (diff >= pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.ByPassThr ||
                diff <= -pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.ByPassThr)
                bypass = false;
            else
                bypass = true;
        }
        bypass = bypass && !pAdrcCtx->ifReCalcStAuto;
    }

    LOGD_ATMO(
        "%s: FrameID:%d HDRFrameNum:%d LongFrmMode:%d DRCApiMode:%d ifReCalcStAuto:%d "
        "ifReCalcStManual:%d CtrlDataType:%d EnvLv:%f "
        "ISO:%f bypass:%d\n",
        __FUNCTION__, pAdrcCtx->FrameID, pAdrcCtx->FrameNumber,
        pAdrcCtx->NextData.AEData.LongFrmMode, pAdrcCtx->drcAttrV12.opMode,
        pAdrcCtx->ifReCalcStAuto, pAdrcCtx->ifReCalcStManual,
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.CtrlDataType, pAdrcCtx->NextData.AEData.EnvLv,
        pAdrcCtx->NextData.AEData.ISO, bypass);

    LOG1_ATMO(
        "%s: NextEnvLv:%f CurrEnvLv:%f NextISO:%f CurrISO:%f diff:%f ByPassThr:%f opMode:%d "
        "bypass:%d!\n",
        __FUNCTION__, pAdrcCtx->NextData.AEData.EnvLv, pAdrcCtx->CurrData.AEData.EnvLv,
        pAdrcCtx->NextData.AEData.ISO, pAdrcCtx->CurrData.AEData.ISO, diff,
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.ByPassThr, pAdrcCtx->drcAttrV12.opMode, bypass);

    LOG1_ATMO("%s:exit!\n", __FUNCTION__);
    return bypass;
}

/******************************************************************************
 * AdrcSetDefaultManuAttrParmasV12()
 *****************************************************************************/
void AdrcSetDefaultManuAttrParmasV12(AdrcContext_t* pAdrcCtx) {
    LOG1_ATMO("%s:enter!\n", __FUNCTION__);

#if RKAIQ_HAVE_DRC_V12
    pAdrcCtx->drcAttrV12.stManual.Enable = pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.Enable;
    pAdrcCtx->drcAttrV12.stManual.DrcGain.Alpha =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.DrcGain.Alpha[0];
    pAdrcCtx->drcAttrV12.stManual.DrcGain.DrcGain =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.DrcGain.DrcGain[0];
    pAdrcCtx->drcAttrV12.stManual.DrcGain.Clip =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.DrcGain.Clip[0];
    pAdrcCtx->drcAttrV12.stManual.HiLight.HiLightData.Strength =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.HiLight.HiLightData.Strength[0];
    pAdrcCtx->drcAttrV12.stManual.HiLight.HiLightData.gas_t =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.HiLight.HiLightData.gas_t[0];
    pAdrcCtx->drcAttrV12.stManual.HiLight.gas_l0 =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.HiLight.gas_l0;
    pAdrcCtx->drcAttrV12.stManual.HiLight.gas_l1 =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.HiLight.gas_l1;
    pAdrcCtx->drcAttrV12.stManual.HiLight.gas_l2 =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.HiLight.gas_l2;
    pAdrcCtx->drcAttrV12.stManual.HiLight.gas_l3 =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.HiLight.gas_l3;
    pAdrcCtx->drcAttrV12.stManual.LocalSetting.LocalData.LocalWeit =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.LocalData.LocalWeit[0];
    pAdrcCtx->drcAttrV12.stManual.LocalSetting.LocalData.LocalAutoEnable =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.LocalData.LocalAutoEnable[0];
    pAdrcCtx->drcAttrV12.stManual.LocalSetting.LocalData.LocalAutoWeit =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.LocalData.LocalAutoWeit[0];
    pAdrcCtx->drcAttrV12.stManual.LocalSetting.LocalData.GlobalContrast =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.LocalData.GlobalContrast[0];
    pAdrcCtx->drcAttrV12.stManual.LocalSetting.LocalData.LoLitContrast =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.LocalData.LoLitContrast[0];
    pAdrcCtx->drcAttrV12.stManual.LocalSetting.MotionData.MotionStr =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.MotionData.MotionStr[0];
    pAdrcCtx->drcAttrV12.stManual.LocalSetting.curPixWeit =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.curPixWeit;
    pAdrcCtx->drcAttrV12.stManual.LocalSetting.preFrameWeit =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.preFrameWeit;
    pAdrcCtx->drcAttrV12.stManual.LocalSetting.Range_force_sgm =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.Range_force_sgm;
    pAdrcCtx->drcAttrV12.stManual.LocalSetting.Range_sgm_cur =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.Range_sgm_cur;
    pAdrcCtx->drcAttrV12.stManual.LocalSetting.Range_sgm_pre =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.Range_sgm_pre;
    pAdrcCtx->drcAttrV12.stManual.LocalSetting.Space_sgm_cur =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.Space_sgm_cur;
    pAdrcCtx->drcAttrV12.stManual.LocalSetting.Space_sgm_pre =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.Space_sgm_pre;
    pAdrcCtx->drcAttrV12.stManual.CompressSetting.Mode =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.CompressSetting.Mode;
    pAdrcCtx->drcAttrV12.stManual.CompressSetting.Manual_curve[0] =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.CompressSetting.Manual_curve[0];
    pAdrcCtx->drcAttrV12.stManual.CompressSetting.Manual_curve[1] =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.CompressSetting.Manual_curve[1];
    pAdrcCtx->drcAttrV12.stManual.CompressSetting.Manual_curve[2] =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.CompressSetting.Manual_curve[2];
    pAdrcCtx->drcAttrV12.stManual.CompressSetting.Manual_curve[3] =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.CompressSetting.Manual_curve[3];
    pAdrcCtx->drcAttrV12.stManual.CompressSetting.Manual_curve[4] =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.CompressSetting.Manual_curve[4];
    pAdrcCtx->drcAttrV12.stManual.CompressSetting.Manual_curve[5] =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.CompressSetting.Manual_curve[5];
    pAdrcCtx->drcAttrV12.stManual.CompressSetting.Manual_curve[6] =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.CompressSetting.Manual_curve[6];
    pAdrcCtx->drcAttrV12.stManual.CompressSetting.Manual_curve[7] =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.CompressSetting.Manual_curve[7];
    pAdrcCtx->drcAttrV12.stManual.CompressSetting.Manual_curve[8] =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.CompressSetting.Manual_curve[8];
    pAdrcCtx->drcAttrV12.stManual.CompressSetting.Manual_curve[9] =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.CompressSetting.Manual_curve[9];
    pAdrcCtx->drcAttrV12.stManual.CompressSetting.Manual_curve[10] =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.CompressSetting.Manual_curve[10];
    pAdrcCtx->drcAttrV12.stManual.CompressSetting.Manual_curve[11] =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.CompressSetting.Manual_curve[11];
    pAdrcCtx->drcAttrV12.stManual.CompressSetting.Manual_curve[12] =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.CompressSetting.Manual_curve[12];
    pAdrcCtx->drcAttrV12.stManual.CompressSetting.Manual_curve[13] =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.CompressSetting.Manual_curve[13];
    pAdrcCtx->drcAttrV12.stManual.CompressSetting.Manual_curve[14] =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.CompressSetting.Manual_curve[14];
    pAdrcCtx->drcAttrV12.stManual.CompressSetting.Manual_curve[15] =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.CompressSetting.Manual_curve[15];
    pAdrcCtx->drcAttrV12.stManual.CompressSetting.Manual_curve[16] =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.CompressSetting.Manual_curve[16];
    pAdrcCtx->drcAttrV12.stManual.Scale_y[0] = pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.Scale_y[0];
    pAdrcCtx->drcAttrV12.stManual.Scale_y[1] = pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.Scale_y[1];
    pAdrcCtx->drcAttrV12.stManual.Scale_y[2] = pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.Scale_y[2];
    pAdrcCtx->drcAttrV12.stManual.Scale_y[3] = pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.Scale_y[3];
    pAdrcCtx->drcAttrV12.stManual.Scale_y[4] = pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.Scale_y[4];
    pAdrcCtx->drcAttrV12.stManual.Scale_y[5] = pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.Scale_y[5];
    pAdrcCtx->drcAttrV12.stManual.Scale_y[6] = pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.Scale_y[6];
    pAdrcCtx->drcAttrV12.stManual.Scale_y[7] = pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.Scale_y[7];
    pAdrcCtx->drcAttrV12.stManual.Scale_y[8] = pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.Scale_y[8];
    pAdrcCtx->drcAttrV12.stManual.Scale_y[9] = pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.Scale_y[9];
    pAdrcCtx->drcAttrV12.stManual.Scale_y[10] =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.Scale_y[10];
    pAdrcCtx->drcAttrV12.stManual.Scale_y[11] =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.Scale_y[11];
    pAdrcCtx->drcAttrV12.stManual.Scale_y[12] =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.Scale_y[12];
    pAdrcCtx->drcAttrV12.stManual.Scale_y[13] =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.Scale_y[13];
    pAdrcCtx->drcAttrV12.stManual.Scale_y[14] =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.Scale_y[14];
    pAdrcCtx->drcAttrV12.stManual.Scale_y[15] =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.Scale_y[15];
    pAdrcCtx->drcAttrV12.stManual.Scale_y[16] =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.Scale_y[16];
    pAdrcCtx->drcAttrV12.stManual.Edge_Weit = pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.Edge_Weit;
    pAdrcCtx->drcAttrV12.stManual.OutPutLongFrame =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.OutPutLongFrame;
    pAdrcCtx->drcAttrV12.stManual.IIR_frame = pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.IIR_frame;
#endif
#if RKAIQ_HAVE_DRC_V12_LITE
    pAdrcCtx->drcAttrV12.stManual.Enable = pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.Enable;
    pAdrcCtx->drcAttrV12.stManual.DrcGain.Alpha =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.DrcGain.Alpha[0];
    pAdrcCtx->drcAttrV12.stManual.DrcGain.DrcGain =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.DrcGain.DrcGain[0];
    pAdrcCtx->drcAttrV12.stManual.DrcGain.Clip =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.DrcGain.Clip[0];
    pAdrcCtx->drcAttrV12.stManual.HiLight.HiLightData.Strength =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.HiLight.HiLightData.Strength[0];
    pAdrcCtx->drcAttrV12.stManual.HiLight.HiLightData.gas_t =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.HiLight.HiLightData.gas_t[0];
    pAdrcCtx->drcAttrV12.stManual.HiLight.gas_l0 =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.HiLight.gas_l0;
    pAdrcCtx->drcAttrV12.stManual.HiLight.gas_l1 =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.HiLight.gas_l1;
    pAdrcCtx->drcAttrV12.stManual.HiLight.gas_l2 =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.HiLight.gas_l2;
    pAdrcCtx->drcAttrV12.stManual.HiLight.gas_l3 =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.HiLight.gas_l3;
    pAdrcCtx->drcAttrV12.stManual.LocalSetting.LocalData.LocalWeit =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.LocalData.LocalWeit[0];
    pAdrcCtx->drcAttrV12.stManual.LocalSetting.LocalData.LocalAutoEnable =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.LocalData.LocalAutoEnable[0];
    pAdrcCtx->drcAttrV12.stManual.LocalSetting.LocalData.LocalAutoWeit =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.LocalData.LocalAutoWeit[0];
    pAdrcCtx->drcAttrV12.stManual.LocalSetting.LocalData.GlobalContrast =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.LocalData.GlobalContrast[0];
    pAdrcCtx->drcAttrV12.stManual.LocalSetting.LocalData.LoLitContrast =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.LocalData.LoLitContrast[0];
    pAdrcCtx->drcAttrV12.stManual.LocalSetting.MotionData.MotionStr =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.MotionData.MotionStr[0];
    pAdrcCtx->drcAttrV12.stManual.LocalSetting.curPixWeit =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.curPixWeit;
    pAdrcCtx->drcAttrV12.stManual.LocalSetting.Range_force_sgm =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.Range_force_sgm;
    pAdrcCtx->drcAttrV12.stManual.LocalSetting.Range_sgm_cur =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.Range_sgm_cur;
    pAdrcCtx->drcAttrV12.stManual.LocalSetting.Space_sgm_cur =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.LocalSetting.Space_sgm_cur;
    pAdrcCtx->drcAttrV12.stManual.CompressSetting.Mode =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.CompressSetting.Mode;
    pAdrcCtx->drcAttrV12.stManual.CompressSetting.Manual_curve[0] =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.CompressSetting.Manual_curve[0];
    pAdrcCtx->drcAttrV12.stManual.CompressSetting.Manual_curve[1] =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.CompressSetting.Manual_curve[1];
    pAdrcCtx->drcAttrV12.stManual.CompressSetting.Manual_curve[2] =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.CompressSetting.Manual_curve[2];
    pAdrcCtx->drcAttrV12.stManual.CompressSetting.Manual_curve[3] =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.CompressSetting.Manual_curve[3];
    pAdrcCtx->drcAttrV12.stManual.CompressSetting.Manual_curve[4] =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.CompressSetting.Manual_curve[4];
    pAdrcCtx->drcAttrV12.stManual.CompressSetting.Manual_curve[5] =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.CompressSetting.Manual_curve[5];
    pAdrcCtx->drcAttrV12.stManual.CompressSetting.Manual_curve[6] =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.CompressSetting.Manual_curve[6];
    pAdrcCtx->drcAttrV12.stManual.CompressSetting.Manual_curve[7] =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.CompressSetting.Manual_curve[7];
    pAdrcCtx->drcAttrV12.stManual.CompressSetting.Manual_curve[8] =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.CompressSetting.Manual_curve[8];
    pAdrcCtx->drcAttrV12.stManual.CompressSetting.Manual_curve[9] =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.CompressSetting.Manual_curve[9];
    pAdrcCtx->drcAttrV12.stManual.CompressSetting.Manual_curve[10] =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.CompressSetting.Manual_curve[10];
    pAdrcCtx->drcAttrV12.stManual.CompressSetting.Manual_curve[11] =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.CompressSetting.Manual_curve[11];
    pAdrcCtx->drcAttrV12.stManual.CompressSetting.Manual_curve[12] =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.CompressSetting.Manual_curve[12];
    pAdrcCtx->drcAttrV12.stManual.CompressSetting.Manual_curve[13] =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.CompressSetting.Manual_curve[13];
    pAdrcCtx->drcAttrV12.stManual.CompressSetting.Manual_curve[14] =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.CompressSetting.Manual_curve[14];
    pAdrcCtx->drcAttrV12.stManual.CompressSetting.Manual_curve[15] =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.CompressSetting.Manual_curve[15];
    pAdrcCtx->drcAttrV12.stManual.CompressSetting.Manual_curve[16] =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.CompressSetting.Manual_curve[16];
    pAdrcCtx->drcAttrV12.stManual.Scale_y[0] = pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.Scale_y[0];
    pAdrcCtx->drcAttrV12.stManual.Scale_y[1] = pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.Scale_y[1];
    pAdrcCtx->drcAttrV12.stManual.Scale_y[2] = pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.Scale_y[2];
    pAdrcCtx->drcAttrV12.stManual.Scale_y[3] = pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.Scale_y[3];
    pAdrcCtx->drcAttrV12.stManual.Scale_y[4] = pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.Scale_y[4];
    pAdrcCtx->drcAttrV12.stManual.Scale_y[5] = pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.Scale_y[5];
    pAdrcCtx->drcAttrV12.stManual.Scale_y[6] = pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.Scale_y[6];
    pAdrcCtx->drcAttrV12.stManual.Scale_y[7] = pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.Scale_y[7];
    pAdrcCtx->drcAttrV12.stManual.Scale_y[8] = pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.Scale_y[8];
    pAdrcCtx->drcAttrV12.stManual.Scale_y[9] = pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.Scale_y[9];
    pAdrcCtx->drcAttrV12.stManual.Scale_y[10] =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.Scale_y[10];
    pAdrcCtx->drcAttrV12.stManual.Scale_y[11] =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.Scale_y[11];
    pAdrcCtx->drcAttrV12.stManual.Scale_y[12] =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.Scale_y[12];
    pAdrcCtx->drcAttrV12.stManual.Scale_y[13] =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.Scale_y[13];
    pAdrcCtx->drcAttrV12.stManual.Scale_y[14] =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.Scale_y[14];
    pAdrcCtx->drcAttrV12.stManual.Scale_y[15] =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.Scale_y[15];
    pAdrcCtx->drcAttrV12.stManual.Scale_y[16] =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.Scale_y[16];
    pAdrcCtx->drcAttrV12.stManual.Edge_Weit = pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.Edge_Weit;
    pAdrcCtx->drcAttrV12.stManual.OutPutLongFrame =
        pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.OutPutLongFrame;
    pAdrcCtx->drcAttrV12.stManual.IIR_frame = pAdrcCtx->drcAttrV12.stAuto.DrcTuningPara.IIR_frame;
#endif

    LOG1_ATMO("%s:exit!\n", __FUNCTION__);
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

    *ppAdrcCtx      = pAdrcCtx;
    pAdrcCtx->state = ADRC_STATE_INITIALIZED;

#if RKAIQ_HAVE_DRC_V12
    CalibDbV2_drc_V12_t* calibv2_adrc_calib =
        (CalibDbV2_drc_V12_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibDb, adrc_calib));
    memcpy(&pAdrcCtx->drcAttrV12.stAuto, calibv2_adrc_calib, sizeof(CalibDbV2_drc_V12_t));
    pAdrcCtx->drcAttrV12.opMode = DRC_OPMODE_AUTO;
#endif
#if RKAIQ_HAVE_DRC_V12_LITE
    CalibDbV2_drc_v12_lite_t* calibv2_adrc_calib =
        (CalibDbV2_drc_v12_lite_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibDb, adrc_calib));
    memcpy(&pAdrcCtx->drcAttrV12.stAuto, calibv2_adrc_calib, sizeof(CalibDbV2_drc_v12_lite_t));
    pAdrcCtx->drcAttrV12.opMode = DRC_OPMODE_AUTO;
#endif
    AdrcSetDefaultManuAttrParmasV12(pAdrcCtx);
    pAdrcCtx->ifReCalcStAuto   = true;
    pAdrcCtx->ifReCalcStManual = false;
    pAdrcCtx->isCapture        = false;
    pAdrcCtx->isDampStable     = true;

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
