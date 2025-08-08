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
 * @file rk_aiq_adrc_algo_v11.cpp
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

float DrcGetInterpRatioV11(float* pX, int& lo, int& hi, float CtrlValue, int length_max) {
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

void AdrcV11ClipStAutoParams(AdrcContext_t* pAdrcCtx) {
    LOG1_ATMO("%s:enter!\n", __FUNCTION__);

    for (int i = 0; i < ADRC_ENVLV_STEP_MAX; i++) {
        pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.DrcGain.DrcGain[i] = LIMIT_VALUE(
            pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.DrcGain.DrcGain[i], DRCGAINMAX, DRCGAINMIN);
        pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.DrcGain.Alpha[i] =
            LIMIT_VALUE(pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.DrcGain.Alpha[i], NORMALIZE_MAX,
                        NORMALIZE_MIN);
        pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.DrcGain.Clip[i] = LIMIT_VALUE(
            pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.DrcGain.Clip[i], CLIPMAX, CLIPMIN);

        pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.HiLight.Strength[i] =
            LIMIT_VALUE(pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.HiLight.Strength[i],
                        NORMALIZE_MAX, NORMALIZE_MIN);

        pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.LocalSetting.LocalData.LocalWeit[i] = LIMIT_VALUE(
            pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.LocalSetting.LocalData.LocalWeit[i],
            NORMALIZE_MAX, NORMALIZE_MIN);
        pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.LocalSetting.LocalData.GlobalContrast[i] =
            LIMIT_VALUE(
                pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.LocalSetting.LocalData.GlobalContrast[i],
                NORMALIZE_MAX, NORMALIZE_MIN);
        pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.LocalSetting.LocalData.LoLitContrast[i] =
            LIMIT_VALUE(
                pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.LocalSetting.LocalData.LoLitContrast[i],
                NORMALIZE_MAX, NORMALIZE_MIN);
        pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.LocalSetting.LocalData.LocalAutoEnable[i] =
            LIMIT_VALUE(
                pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.LocalSetting.LocalData.LocalAutoEnable[i],
                NORMALIZE_MAX, NORMALIZE_MIN);
        pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.LocalSetting.LocalData.LocalAutoWeit[i] =
            LIMIT_VALUE(
                pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.LocalSetting.LocalData.LocalAutoWeit[i],
                NORMALIZE_MAX, NORMALIZE_MIN);
    }
    pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.LocalSetting.curPixWeit =
        LIMIT_VALUE(pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.LocalSetting.curPixWeit,
                    NORMALIZE_MAX, NORMALIZE_MIN);
    pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.LocalSetting.preFrameWeit =
        LIMIT_VALUE(pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.LocalSetting.preFrameWeit,
                    NORMALIZE_MAX, NORMALIZE_MIN);
    pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.LocalSetting.Range_force_sgm =
        LIMIT_VALUE(pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.LocalSetting.Range_force_sgm,
                    NORMALIZE_MAX, NORMALIZE_MIN);
    pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.LocalSetting.Range_sgm_cur =
        LIMIT_VALUE(pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.LocalSetting.Range_sgm_cur,
                    NORMALIZE_MAX, NORMALIZE_MIN);
    pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.LocalSetting.Range_sgm_pre =
        LIMIT_VALUE(pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.LocalSetting.Range_sgm_pre,
                    NORMALIZE_MAX, NORMALIZE_MIN);
    pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.LocalSetting.Space_sgm_cur =
        LIMIT_VALUE(pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.LocalSetting.Space_sgm_cur,
                    SPACESGMMAX, SPACESGMMIN);
    pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.LocalSetting.Space_sgm_pre =
        LIMIT_VALUE(pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.LocalSetting.Space_sgm_pre,
                    SPACESGMMAX, SPACESGMMIN);
    for (int i = 0; i < ADRC_Y_NUM; i++) {
        pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.Scale_y[i] =
            LIMIT_VALUE(pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.Scale_y[i], SCALEYMAX, SCALEYMIN);
        pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.CompressSetting.Manual_curve[i] =
            LIMIT_VALUE_UNSIGNED(
                pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.CompressSetting.Manual_curve[i],
                MANUALCURVEMAX);
    }
    pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.ByPassThr = LIMIT_VALUE(
        pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.ByPassThr, NORMALIZE_MAX, NORMALIZE_MIN);
    pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.Edge_Weit = LIMIT_VALUE(
        pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.Edge_Weit, NORMALIZE_MAX, NORMALIZE_MIN);
    pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.IIR_frame =
        LIMIT_VALUE(pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.IIR_frame, IIRFRAMEMAX, IIRFRAMEMIN);
    pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.damp =
        LIMIT_VALUE(pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.damp, NORMALIZE_MAX, NORMALIZE_MIN);

    LOG1_ATMO("%s:exit!\n", __FUNCTION__);
}

/******************************************************************************
 * CalibrateDrcGainYV11()
 *****************************************************************************/
void CalibrateDrcGainYV11(DrcProcRes_t* para, float DraGain, float alpha) {
    LOG1_ATMO("%s:Enter!\n", __FUNCTION__);

    float tmp = 0.0f;
    /*luma[i] = pow((1.0f - luma[i] / 4096.0f), 2.0f)*/
    float luma[DRC_V11_Y_NUM] = {1.0f,    0.8789f, 0.7656f, 0.6602f, 0.5625f, 0.4727f,
                                 0.3906f, 0.3164f, 0.2500f, 0.1914f, 0.1406f, 0.0977f,
                                 0.0625f, 0.0352f, 0.0156f, 0.0039f, 0.0f};

    for (int i = 0; i < DRC_V11_Y_NUM; ++i) {
        tmp                     = 1.0f - alpha * luma[i];
        tmp                     = 1024.0f * pow(DraGain, tmp);
        para->Drc_v11.gain_y[i] = (unsigned short)(tmp);
    }

    LOG1_ATMO("%s:Eixt!\n", __FUNCTION__);
}

/******************************************************************************
 * DrcEnableSetting()
 *
 *****************************************************************************/
bool DrcEnableSetting(AdrcContext_t* pAdrcCtx, RkAiqAdrcProcResult_t* pAdrcProcRes) {
    LOG1_ATMO("%s:enter!\n", __FUNCTION__);

    if (pAdrcCtx->FrameNumber == HDR_2X_NUM || pAdrcCtx->FrameNumber == HDR_3X_NUM)
        pAdrcProcRes->bDrcEn = true;
    else if (pAdrcCtx->FrameNumber == LINEAR_NUM) {
        if (pAdrcCtx->drcAttrV11.opMode == DRC_OPMODE_AUTO)
            pAdrcProcRes->bDrcEn = pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.Enable;
        else if (pAdrcCtx->drcAttrV11.opMode == DRC_OPMODE_MANUAL)
            pAdrcProcRes->bDrcEn = pAdrcCtx->drcAttrV11.stManual.Enable;
        else {
            LOGE_ATMO("%s: Drc api in WRONG MODE!!!, drc by pass!!!\n", __FUNCTION__);
            pAdrcProcRes->bDrcEn = false;
        }
    }
    // store bDrcEn for AdrcV11Params2Api
    pAdrcCtx->NextData.bDrcEn = pAdrcProcRes->bDrcEn;

    LOG1_ATMO("%s:exit!\n", __FUNCTION__);
    return pAdrcProcRes->bDrcEn;
}
/******************************************************************************
 * AdrcDampingV11()
 *****************************************************************************/
bool AdrcDampingV11(NextData_t* pNextData, CurrData_t* pCurrData, int FrameID,
                    CtrlDataType_t CtrlDataType) {
    LOG1_ATMO("%s:Enter!\n", __FUNCTION__);
    bool isDampStable = false;

    if (FrameID > INIT_CALC_PARAMS_NUM) {
        bool isDampStable_DrcGain = false, isDampStable_Alpha = false, isDampStable_Clip = false,
             isDampStable_Strength = false, isDampStable_LocalWeit = false,
             isDampStable_LocalAutoWeit = false, isDampStable_GlobalContrast = false,
             isDampStable_LoLitContrast = false;

        if ((pNextData->dynParams.Drc_v11.DrcGain - pCurrData->dynParams.Drc_v11.DrcGain) <=
                DAMP_STABLE_THR &&
            (pNextData->dynParams.Drc_v11.DrcGain - pCurrData->dynParams.Drc_v11.DrcGain) >=
                -DAMP_STABLE_THR) {
            isDampStable_DrcGain = true;
        } else {
            pNextData->dynParams.Drc_v11.DrcGain =
                pNextData->staticParams.damp * pNextData->dynParams.Drc_v11.DrcGain +
                (1.0f - pNextData->staticParams.damp) * pCurrData->dynParams.Drc_v11.DrcGain;
            isDampStable_DrcGain = false;
        }
        if ((pNextData->dynParams.Drc_v11.Alpha - pCurrData->dynParams.Drc_v11.Alpha) <=
                DAMP_STABLE_THR &&
            (pNextData->dynParams.Drc_v11.Alpha - pCurrData->dynParams.Drc_v11.Alpha) >=
                -DAMP_STABLE_THR) {
            isDampStable_Alpha = true;
        } else {
            pNextData->dynParams.Drc_v11.Alpha =
                pNextData->staticParams.damp * pNextData->dynParams.Drc_v11.Alpha +
                (1.0f - pNextData->staticParams.damp) * pCurrData->dynParams.Drc_v11.Alpha;
            isDampStable_Alpha = false;
        }
        if ((pNextData->dynParams.Drc_v11.Clip - pCurrData->dynParams.Drc_v11.Clip) <=
                DAMP_STABLE_THR &&
            (pNextData->dynParams.Drc_v11.Clip - pCurrData->dynParams.Drc_v11.Clip) >=
                -DAMP_STABLE_THR) {
            isDampStable_Clip = true;
        } else {
            pNextData->dynParams.Drc_v11.Clip =
                pNextData->staticParams.damp * pNextData->dynParams.Drc_v11.Clip +
                (1.0f - pNextData->staticParams.damp) * pCurrData->dynParams.Drc_v11.Clip;
            isDampStable_Clip = false;
        }
        if ((pNextData->dynParams.Drc_v11.Strength - pCurrData->dynParams.Drc_v11.Strength) <=
                DAMP_STABLE_THR &&
            (pNextData->dynParams.Drc_v11.Strength - pCurrData->dynParams.Drc_v11.Strength) >=
                -DAMP_STABLE_THR) {
            isDampStable_Strength = true;
        } else {
            pNextData->dynParams.Drc_v11.Strength =
                pNextData->staticParams.damp * pNextData->dynParams.Drc_v11.Strength +
                (1.0f - pNextData->staticParams.damp) * pCurrData->dynParams.Drc_v11.Strength;
            isDampStable_Strength = false;
        }
        if ((pNextData->dynParams.Drc_v11.LocalWeit - pCurrData->dynParams.Drc_v11.LocalWeit) <=
                DAMP_STABLE_THR &&
            (pNextData->dynParams.Drc_v11.LocalWeit - pCurrData->dynParams.Drc_v11.LocalWeit) >=
                -DAMP_STABLE_THR) {
            isDampStable_LocalWeit = true;
        } else {
            pNextData->dynParams.Drc_v11.LocalWeit =
                pNextData->staticParams.damp * pNextData->dynParams.Drc_v11.LocalWeit +
                (1.0f - pNextData->staticParams.damp) * pCurrData->dynParams.Drc_v11.LocalWeit;
            isDampStable_LocalWeit = false;
        }
        if ((pNextData->dynParams.Drc_v11.LocalAutoWeit -
             pCurrData->dynParams.Drc_v11.LocalAutoWeit) <= DAMP_STABLE_THR &&
            (pNextData->dynParams.Drc_v11.LocalAutoWeit -
             pCurrData->dynParams.Drc_v11.LocalAutoWeit) >= -DAMP_STABLE_THR) {
            isDampStable_LocalAutoWeit = true;
        } else {
            pNextData->dynParams.Drc_v11.LocalAutoWeit =
                pNextData->staticParams.damp * pNextData->dynParams.Drc_v11.LocalAutoWeit +
                (1.0f - pNextData->staticParams.damp) * pCurrData->dynParams.Drc_v11.LocalAutoWeit;
            isDampStable_LocalAutoWeit = false;
        }
        if ((pNextData->dynParams.Drc_v11.GlobalContrast -
             pCurrData->dynParams.Drc_v11.GlobalContrast) <= DAMP_STABLE_THR &&
            (pNextData->dynParams.Drc_v11.GlobalContrast -
             pCurrData->dynParams.Drc_v11.GlobalContrast) >= -DAMP_STABLE_THR) {
            isDampStable_GlobalContrast = true;
        } else {
            pNextData->dynParams.Drc_v11.GlobalContrast =
                pNextData->staticParams.damp * pNextData->dynParams.Drc_v11.GlobalContrast +
                (1.0f - pNextData->staticParams.damp) * pCurrData->dynParams.Drc_v11.GlobalContrast;
            isDampStable_GlobalContrast = false;
        }
        if ((pNextData->dynParams.Drc_v11.LoLitContrast -
             pCurrData->dynParams.Drc_v11.LoLitContrast) <= DAMP_STABLE_THR &&
            (pNextData->dynParams.Drc_v11.LoLitContrast -
             pCurrData->dynParams.Drc_v11.LoLitContrast) >= -DAMP_STABLE_THR) {
            isDampStable_LoLitContrast = true;
        } else {
            pNextData->dynParams.Drc_v11.LoLitContrast =
                pNextData->staticParams.damp * pNextData->dynParams.Drc_v11.LoLitContrast +
                (1.0f - pNextData->staticParams.damp) * pCurrData->dynParams.Drc_v11.LoLitContrast;
            isDampStable_LoLitContrast = false;
        }
        isDampStable = isDampStable_DrcGain && isDampStable_Alpha && isDampStable_Clip &&
                       isDampStable_Strength && isDampStable_LocalWeit &&
                       isDampStable_LocalAutoWeit && isDampStable_GlobalContrast &&
                       isDampStable_LoLitContrast;
    } else
        isDampStable = false;

    LOG1_ATMO("%s:Eixt!\n", __FUNCTION__);
    return isDampStable;
}
/******************************************************************************
 * AdrcGetTuningProcResV11()
 *****************************************************************************/
void AdrcGetTuningProcResV11(AdrcContext_t* pAdrcCtx, RkAiqAdrcProcResult_t* pAdrcProcRes) {
    LOG1_ATMO("%s:enter!\n", __FUNCTION__);

    pAdrcProcRes->DrcProcRes.Drc_v11.position =
        (unsigned short)(SHIFT8BIT(pAdrcCtx->NextData.dynParams.Drc_v11.Clip) + 0.5f);
    pAdrcProcRes->DrcProcRes.Drc_v11.hpdetail_ratio =
        (unsigned short)(SHIFT12BIT(pAdrcCtx->NextData.dynParams.Drc_v11.LoLitContrast) + 0.5f);
    pAdrcProcRes->DrcProcRes.Drc_v11.lpdetail_ratio =
        (unsigned short)(SHIFT12BIT(pAdrcCtx->NextData.dynParams.Drc_v11.GlobalContrast) + 0.5f);
    pAdrcProcRes->DrcProcRes.Drc_v11.weipre_frame =
        LIMIT_VALUE_UNSIGNED(pAdrcProcRes->DrcProcRes.Drc_v11.weipre_frame, BIT_8_MAX);
    pAdrcProcRes->DrcProcRes.Drc_v11.weig_maxl =
        (unsigned char)(SHIFT4BIT(pAdrcCtx->NextData.dynParams.Drc_v11.Strength) + 0.5f);
    pAdrcProcRes->DrcProcRes.Drc_v11.weig_bilat =
        (unsigned char)(SHIFT4BIT(pAdrcCtx->NextData.dynParams.Drc_v11.LocalWeit) + 0.5f);
    pAdrcProcRes->DrcProcRes.Drc_v11.enable_soft_thd =
        pAdrcCtx->NextData.dynParams.Drc_v11.LocalAutoEnable;
    pAdrcProcRes->DrcProcRes.Drc_v11.bilat_soft_thd =
        (unsigned short)(SHIFT14BIT(pAdrcCtx->NextData.dynParams.Drc_v11.LocalAutoWeit) + 0.5f);
    pAdrcProcRes->DrcProcRes.Drc_v11.bilat_soft_thd =
        LIMIT_VALUE_UNSIGNED(pAdrcProcRes->DrcProcRes.Drc_v11.bilat_soft_thd, BIT_14_MAX);
    // get sw_drc_gain_y
    CalibrateDrcGainYV11(&pAdrcProcRes->DrcProcRes, pAdrcCtx->NextData.dynParams.Drc_v11.DrcGain,
                         pAdrcCtx->NextData.dynParams.Drc_v11.Alpha);

    pAdrcProcRes->DrcProcRes.Drc_v11.bilat_wt_off =
        SW_DRC_BILAT_WT_OFF_FIX;  // LIMIT_VALUE(pAdrcCtx->NextData.dynParams.Drc_v11.MotionStr
                                  // *BIT_8_MAX, BIT_8_MAX, 0);
    pAdrcProcRes->DrcProcRes.Drc_v11.weicur_pix =
        (unsigned char)(SHIFT8BIT(pAdrcCtx->NextData.staticParams.curPixWeit) + 0.5f);
    pAdrcProcRes->DrcProcRes.Drc_v11.weipre_frame =
        (unsigned char)(SHIFT8BIT(pAdrcCtx->NextData.staticParams.preFrameWeit) + 0.5f);
    pAdrcProcRes->DrcProcRes.Drc_v11.force_sgm_inv0 =
        (unsigned short)(SHIFT13BIT(pAdrcCtx->NextData.staticParams.Range_force_sgm) + 0.5f);
    pAdrcProcRes->DrcProcRes.Drc_v11.edge_scl =
        (unsigned char)(SHIFT8BIT(pAdrcCtx->NextData.staticParams.Edge_Weit) + 0.5f);
    pAdrcProcRes->DrcProcRes.Drc_v11.space_sgm_inv1 =
        (unsigned short)(pAdrcCtx->NextData.staticParams.Space_sgm_cur);
    pAdrcProcRes->DrcProcRes.Drc_v11.space_sgm_inv0 =
        (unsigned short)(pAdrcCtx->NextData.staticParams.Space_sgm_pre);
    pAdrcProcRes->DrcProcRes.Drc_v11.range_sgm_inv1 =
        (unsigned short)(SHIFT13BIT(pAdrcCtx->NextData.staticParams.Range_sgm_cur) + 0.5f);
    pAdrcProcRes->DrcProcRes.Drc_v11.range_sgm_inv0 =
        (unsigned short)(SHIFT13BIT(pAdrcCtx->NextData.staticParams.Range_sgm_pre) + 0.5f);
    for (int i = 0; i < DRC_V11_Y_NUM; ++i) {
        pAdrcProcRes->DrcProcRes.Drc_v11.scale_y[i] =
            (unsigned short)(pAdrcCtx->NextData.staticParams.Scale_y[i]);
    }
    float iir_frame =
        (float)(MIN(pAdrcCtx->FrameID + 1, (uint32_t)pAdrcCtx->NextData.staticParams.IIR_frame));
    pAdrcProcRes->DrcProcRes.Drc_v11.iir_weight =
        (unsigned short)(SHIFT6BIT((iir_frame - 1.0f) / iir_frame) + 0.5f);
    // set FIX reg
    pAdrcProcRes->DrcProcRes.Drc_v11.bypass_en     = FUNCTION_DISABLE;
    pAdrcProcRes->DrcProcRes.Drc_v11.offset_pow2   = SW_DRC_OFFSET_POW2_FIX;
    pAdrcProcRes->DrcProcRes.Drc_v11.delta_scalein = DELTA_SCALEIN_FIX;
    pAdrcProcRes->DrcProcRes.Drc_v11.wr_cycle      = FUNCTION_DISABLE;
    pAdrcProcRes->DrcProcRes.Drc_v11.motion_scl    = SW_DRC_MOTION_SCL_FIX;

    LOG1_ATMO("%s:exit!\n", __FUNCTION__);
}

void AdrcV11Params2Api(AdrcContext_t* pAdrcCtx, DrcInfoV11_t* pDrcInfo) {
    LOG1_ATMO("%s:enter!\n", __FUNCTION__);

    // ctrl info
    pDrcInfo->CtrlInfo.ISO   = pAdrcCtx->NextData.AEData.ISO;
    pDrcInfo->CtrlInfo.EnvLv = pAdrcCtx->NextData.AEData.EnvLv;

    // params
    pDrcInfo->ValidParams.Enable           = pAdrcCtx->NextData.bDrcEn;
    pDrcInfo->ValidParams.DrcGain.Alpha    = pAdrcCtx->NextData.dynParams.Drc_v11.Alpha;
    pDrcInfo->ValidParams.DrcGain.DrcGain  = pAdrcCtx->NextData.dynParams.Drc_v11.DrcGain;
    pDrcInfo->ValidParams.DrcGain.Clip     = pAdrcCtx->NextData.dynParams.Drc_v11.Clip;
    pDrcInfo->ValidParams.HiLight.Strength = pAdrcCtx->NextData.dynParams.Drc_v11.Strength;
    pDrcInfo->ValidParams.LocalSetting.LocalData.LocalWeit =
        pAdrcCtx->NextData.dynParams.Drc_v11.LocalWeit;
    pDrcInfo->ValidParams.LocalSetting.LocalData.LocalAutoEnable =
        pAdrcCtx->NextData.dynParams.Drc_v11.LocalAutoEnable;
    pDrcInfo->ValidParams.LocalSetting.LocalData.LocalAutoWeit =
        pAdrcCtx->NextData.dynParams.Drc_v11.LocalAutoWeit;
    pDrcInfo->ValidParams.LocalSetting.LocalData.GlobalContrast =
        pAdrcCtx->NextData.dynParams.Drc_v11.GlobalContrast;
    pDrcInfo->ValidParams.LocalSetting.LocalData.LoLitContrast =
        pAdrcCtx->NextData.dynParams.Drc_v11.LoLitContrast;
    pDrcInfo->ValidParams.LocalSetting.curPixWeit   = pAdrcCtx->NextData.staticParams.curPixWeit;
    pDrcInfo->ValidParams.LocalSetting.preFrameWeit = pAdrcCtx->NextData.staticParams.preFrameWeit;
    pDrcInfo->ValidParams.LocalSetting.Range_force_sgm =
        pAdrcCtx->NextData.staticParams.Range_force_sgm;
    pDrcInfo->ValidParams.LocalSetting.Range_sgm_cur =
        pAdrcCtx->NextData.staticParams.Range_sgm_cur;
    pDrcInfo->ValidParams.LocalSetting.Range_sgm_pre =
        pAdrcCtx->NextData.staticParams.Range_sgm_pre;
    pDrcInfo->ValidParams.LocalSetting.Space_sgm_cur =
        pAdrcCtx->NextData.staticParams.Space_sgm_cur;
    pDrcInfo->ValidParams.LocalSetting.Space_sgm_pre =
        pAdrcCtx->NextData.staticParams.Space_sgm_pre;
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
/******************************************************************************
 * AdrcTuningParaProcessing()
 *get handle para by config and current variate
 *****************************************************************************/
void AdrcTuningParaProcessing(AdrcContext_t* pAdrcCtx, RkAiqAdrcProcResult_t* pAdrcProcRes) {
    LOG1_ATMO("%s:enter!\n", __FUNCTION__);

    // para setting
    if (pAdrcCtx->drcAttrV11.opMode == DRC_OPMODE_AUTO) {
        int lo = 0, hi = 0;
        float ratio     = 0.0f;
        float CtrlValue = pAdrcCtx->NextData.AEData.EnvLv;
        if (pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.CtrlDataType == CTRLDATATYPE_ISO)
            CtrlValue = pAdrcCtx->NextData.AEData.ISO;

        // get Drc gain
        ratio = DrcGetInterpRatioV11(pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.DrcGain.CtrlData, lo,
                                     hi, CtrlValue, ADRC_ENVLV_STEP_MAX);
        pAdrcCtx->NextData.dynParams.Drc_v11.DrcGain =
            ratio * (pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.DrcGain.DrcGain[hi] -
                     pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.DrcGain.DrcGain[lo]) +
            pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.DrcGain.DrcGain[lo];
        pAdrcCtx->NextData.dynParams.Drc_v11.Alpha =
            ratio * (pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.DrcGain.Alpha[hi] -
                     pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.DrcGain.Alpha[lo]) +
            pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.DrcGain.Alpha[lo];
        pAdrcCtx->NextData.dynParams.Drc_v11.Clip =
            ratio * (pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.DrcGain.Clip[hi] -
                     pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.DrcGain.Clip[lo]) +
            pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.DrcGain.Clip[lo];

        // get hi lit
        ratio = DrcGetInterpRatioV11(pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.HiLight.CtrlData, lo,
                                     hi, CtrlValue, ADRC_ENVLV_STEP_MAX);
        pAdrcCtx->NextData.dynParams.Drc_v11.Strength =
            ratio * (pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.HiLight.Strength[hi] -
                     pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.HiLight.Strength[lo]) +
            pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.HiLight.Strength[lo];

        // get local
        ratio = DrcGetInterpRatioV11(
            pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.LocalSetting.LocalData.CtrlData, lo, hi,
            CtrlValue, ADRC_ENVLV_STEP_MAX);
        pAdrcCtx->NextData.dynParams.Drc_v11.LocalWeit =
            ratio *
                (pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.LocalSetting.LocalData.LocalWeit[hi] -
                 pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.LocalSetting.LocalData.LocalWeit[lo]) +
            pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.LocalSetting.LocalData.LocalWeit[lo];
        pAdrcCtx->NextData.dynParams.Drc_v11.GlobalContrast =
            ratio * (pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.LocalSetting.LocalData
                         .GlobalContrast[hi] -
                     pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.LocalSetting.LocalData
                         .GlobalContrast[lo]) +
            pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.LocalSetting.LocalData.GlobalContrast[lo];
        pAdrcCtx->NextData.dynParams.Drc_v11.LoLitContrast =
            ratio * (pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.LocalSetting.LocalData
                         .LoLitContrast[hi] -
                     pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.LocalSetting.LocalData
                         .LoLitContrast[lo]) +
            pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.LocalSetting.LocalData.LoLitContrast[lo];
        pAdrcCtx->NextData.dynParams.Drc_v11.LocalAutoEnable =
            pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.LocalSetting.LocalData.LocalAutoEnable[lo];
        pAdrcCtx->NextData.dynParams.Drc_v11.LocalAutoWeit =
            ratio * (pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.LocalSetting.LocalData
                         .LocalAutoWeit[hi] -
                     pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.LocalSetting.LocalData
                         .LocalAutoWeit[lo]) +
            pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.LocalSetting.LocalData.LocalAutoWeit[lo];
        // compress
        pAdrcCtx->NextData.staticParams.CompressMode =
            pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.CompressSetting.Mode;
        for (int i = 0; i < ADRC_Y_NUM; i++)
            pAdrcCtx->NextData.staticParams.compresManuCurve[i] =
                pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.CompressSetting.Manual_curve[i];
        pAdrcCtx->NextData.staticParams.curPixWeit =
            pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.LocalSetting.curPixWeit;
        pAdrcCtx->NextData.staticParams.preFrameWeit =
            pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.LocalSetting.preFrameWeit;
        pAdrcCtx->NextData.staticParams.Range_force_sgm =
            pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.LocalSetting.Range_force_sgm;
        pAdrcCtx->NextData.staticParams.Range_sgm_cur =
            pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.LocalSetting.Range_sgm_cur;
        pAdrcCtx->NextData.staticParams.Range_sgm_pre =
            pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.LocalSetting.Range_sgm_pre;
        pAdrcCtx->NextData.staticParams.Space_sgm_cur =
            pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.LocalSetting.Space_sgm_cur;
        pAdrcCtx->NextData.staticParams.Space_sgm_pre =
            pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.LocalSetting.Space_sgm_pre;
        // scale y
        for (int i = 0; i < ADRC_Y_NUM; i++)
            pAdrcCtx->NextData.staticParams.Scale_y[i] =
                pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.Scale_y[i];
        pAdrcCtx->NextData.staticParams.ByPassThr =
            pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.ByPassThr;
        pAdrcCtx->NextData.staticParams.Edge_Weit =
            pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.Edge_Weit;
        pAdrcCtx->NextData.staticParams.IIR_frame =
            pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.IIR_frame;
        pAdrcCtx->NextData.staticParams.damp = pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.damp;
        pAdrcCtx->NextData.staticParams.OutPutLongFrame =
            pAdrcCtx->NextData.AEData.LongFrmMode ||
            pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.OutPutLongFrame;

        // damp
        pAdrcCtx->isDampStable =
            AdrcDampingV11(&pAdrcCtx->NextData, &pAdrcCtx->CurrData, pAdrcCtx->FrameID,
                           pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.CtrlDataType);
    } else if (pAdrcCtx->drcAttrV11.opMode == DRC_OPMODE_MANUAL) {
        // update drc gain
        pAdrcCtx->NextData.dynParams.Drc_v11.DrcGain =
            LIMIT_VALUE(pAdrcCtx->drcAttrV11.stManual.DrcGain.DrcGain, DRCGAINMAX, DRCGAINMIN);
        pAdrcCtx->NextData.dynParams.Drc_v11.Alpha =
            LIMIT_VALUE(pAdrcCtx->drcAttrV11.stManual.DrcGain.Alpha, NORMALIZE_MAX, NORMALIZE_MIN);
        pAdrcCtx->NextData.dynParams.Drc_v11.Clip =
            LIMIT_VALUE(pAdrcCtx->drcAttrV11.stManual.DrcGain.Clip, CLIPMAX, CLIPMIN);

        // update hight light
        pAdrcCtx->NextData.dynParams.Drc_v11.Strength = LIMIT_VALUE(
            pAdrcCtx->drcAttrV11.stManual.HiLight.Strength, NORMALIZE_MAX, NORMALIZE_MIN);

        // update local
        pAdrcCtx->NextData.dynParams.Drc_v11.LocalWeit =
            LIMIT_VALUE(pAdrcCtx->drcAttrV11.stManual.LocalSetting.LocalData.LocalWeit,
                        NORMALIZE_MAX, NORMALIZE_MIN);
        pAdrcCtx->NextData.dynParams.Drc_v11.LocalAutoEnable =
            LIMIT_VALUE(pAdrcCtx->drcAttrV11.stManual.LocalSetting.LocalData.LocalAutoEnable,
                        ADRCNORMALIZEINTMAX, ADRCNORMALIZEINTMIN);
        pAdrcCtx->NextData.dynParams.Drc_v11.LocalAutoWeit =
            LIMIT_VALUE(pAdrcCtx->drcAttrV11.stManual.LocalSetting.LocalData.LocalAutoWeit,
                        NORMALIZE_MAX, NORMALIZE_MIN);
        pAdrcCtx->NextData.dynParams.Drc_v11.GlobalContrast =
            LIMIT_VALUE(pAdrcCtx->drcAttrV11.stManual.LocalSetting.LocalData.GlobalContrast,
                        NORMALIZE_MAX, NORMALIZE_MIN);
        pAdrcCtx->NextData.dynParams.Drc_v11.LoLitContrast =
            LIMIT_VALUE(pAdrcCtx->drcAttrV11.stManual.LocalSetting.LocalData.LoLitContrast,
                        NORMALIZE_MAX, NORMALIZE_MIN);

        // compress
        pAdrcCtx->NextData.staticParams.CompressMode =
            pAdrcCtx->drcAttrV11.stManual.CompressSetting.Mode;
        for (int i = 0; i < ADRC_Y_NUM; i++)
            pAdrcCtx->NextData.staticParams.compresManuCurve[i] =
                LIMIT_VALUE(pAdrcCtx->drcAttrV11.stManual.CompressSetting.Manual_curve[i],
                            MANUALCURVEMAX, MANUALCURVEMIN);

        // others
        pAdrcCtx->NextData.staticParams.OutPutLongFrame =
            pAdrcCtx->NextData.AEData.LongFrmMode || pAdrcCtx->drcAttrV11.stManual.OutPutLongFrame;
        pAdrcCtx->NextData.staticParams.curPixWeit = LIMIT_VALUE(
            pAdrcCtx->drcAttrV11.stManual.LocalSetting.curPixWeit, NORMALIZE_MAX, NORMALIZE_MIN);
        pAdrcCtx->NextData.staticParams.preFrameWeit = LIMIT_VALUE(
            pAdrcCtx->drcAttrV11.stManual.LocalSetting.preFrameWeit, NORMALIZE_MAX, NORMALIZE_MIN);
        pAdrcCtx->NextData.staticParams.Range_force_sgm =
            LIMIT_VALUE(pAdrcCtx->drcAttrV11.stManual.LocalSetting.Range_force_sgm, NORMALIZE_MAX,
                        NORMALIZE_MIN);
        pAdrcCtx->NextData.staticParams.Range_sgm_cur = LIMIT_VALUE(
            pAdrcCtx->drcAttrV11.stManual.LocalSetting.Range_sgm_cur, NORMALIZE_MAX, NORMALIZE_MIN);
        pAdrcCtx->NextData.staticParams.Range_sgm_pre = LIMIT_VALUE(
            pAdrcCtx->drcAttrV11.stManual.LocalSetting.Range_sgm_pre, NORMALIZE_MAX, NORMALIZE_MIN);
        pAdrcCtx->NextData.staticParams.Space_sgm_cur = LIMIT_VALUE(
            pAdrcCtx->drcAttrV11.stManual.LocalSetting.Space_sgm_cur, SPACESGMMAX, SPACESGMMIN);
        pAdrcCtx->NextData.staticParams.Space_sgm_pre = LIMIT_VALUE(
            pAdrcCtx->drcAttrV11.stManual.LocalSetting.Space_sgm_pre, SPACESGMMAX, SPACESGMMIN);
        for (int i = 0; i < ADRC_Y_NUM; i++)
            pAdrcCtx->NextData.staticParams.Scale_y[i] =
                LIMIT_VALUE(pAdrcCtx->drcAttrV11.stManual.Scale_y[i], SCALEYMAX, SCALEYMIN);
        pAdrcCtx->NextData.staticParams.Edge_Weit =
            LIMIT_VALUE(pAdrcCtx->drcAttrV11.stManual.Edge_Weit, NORMALIZE_MAX, NORMALIZE_MIN);
        pAdrcCtx->NextData.staticParams.IIR_frame =
            LIMIT_VALUE(pAdrcCtx->drcAttrV11.stManual.IIR_frame, IIRFRAMEMAX, IIRFRAMEMIN);

        // damp
        pAdrcCtx->isDampStable = true;
    }

    // clip drc gain
    if (pAdrcCtx->NextData.AEData.L2S_Ratio * pAdrcCtx->NextData.dynParams.Drc_v11.DrcGain >
        MAX_AE_DRC_GAIN) {
        pAdrcCtx->NextData.dynParams.Drc_v11.DrcGain =
            MAX(MAX_AE_DRC_GAIN / pAdrcCtx->NextData.AEData.L2S_Ratio, GAINMIN);
        LOGI_ATMO("%s:  AERatio*DrcGain > 256x, clip to %f!!!\n", __FUNCTION__,
                  pAdrcCtx->NextData.dynParams.Drc_v11.DrcGain);
    }

    LOGD_ATMO(
        "%s: Current Enable:%d DrcGain:%f Alpha:%f Clip:%f Strength:%f CompressMode:%d "
        "OutPutLongFrame:%d\n",
        __FUNCTION__, pAdrcProcRes->bDrcEn, pAdrcCtx->NextData.dynParams.Drc_v11.DrcGain,
        pAdrcCtx->NextData.dynParams.Drc_v11.Alpha, pAdrcCtx->NextData.dynParams.Drc_v11.Clip,
        pAdrcCtx->NextData.dynParams.Drc_v11.Strength, pAdrcCtx->NextData.staticParams.CompressMode,
        pAdrcCtx->NextData.staticParams.OutPutLongFrame);
    LOGD_ATMO(
        "%s: Current LocalWeit:%f LocalAutoEnable:%d LocalAutoWeit:%f GlobalContrast:%f "
        "LoLitContrast:%f\n",
        __FUNCTION__, pAdrcCtx->NextData.dynParams.Drc_v11.LocalWeit,
        pAdrcCtx->NextData.dynParams.Drc_v11.LocalAutoEnable,
        pAdrcCtx->NextData.dynParams.Drc_v11.LocalAutoWeit,
        pAdrcCtx->NextData.dynParams.Drc_v11.GlobalContrast,
        pAdrcCtx->NextData.dynParams.Drc_v11.LoLitContrast);

    // get io data
    AdrcGetTuningProcResV11(pAdrcCtx, pAdrcProcRes);

    // store current handle data to pre data for next loop
    pAdrcCtx->CurrData.AEData.EnvLv = pAdrcCtx->NextData.AEData.EnvLv;
    pAdrcCtx->CurrData.AEData.ISO   = pAdrcCtx->NextData.AEData.ISO;
    pAdrcCtx->CurrData.MotionCoef   = pAdrcCtx->NextData.MotionCoef;
    pAdrcCtx->CurrData.ApiMode      = pAdrcCtx->drcAttrV11.opMode;
    if (pAdrcCtx->drcAttrV11.opMode == DRC_OPMODE_AUTO && !pAdrcCtx->isDampStable) {
        pAdrcCtx->CurrData.dynParams.Drc_v11.DrcGain = pAdrcCtx->NextData.dynParams.Drc_v11.DrcGain;
        pAdrcCtx->CurrData.dynParams.Drc_v11.Alpha   = pAdrcCtx->NextData.dynParams.Drc_v11.Alpha;
        pAdrcCtx->CurrData.dynParams.Drc_v11.Clip    = pAdrcCtx->NextData.dynParams.Drc_v11.Clip;
        pAdrcCtx->CurrData.dynParams.Drc_v11.Strength =
            pAdrcCtx->NextData.dynParams.Drc_v11.Strength;
        pAdrcCtx->CurrData.dynParams.Drc_v11.LocalWeit =
            pAdrcCtx->NextData.dynParams.Drc_v11.LocalWeit;
        pAdrcCtx->CurrData.dynParams.Drc_v11.LocalAutoWeit =
            pAdrcCtx->NextData.dynParams.Drc_v11.LocalAutoWeit;
        pAdrcCtx->CurrData.dynParams.Drc_v11.GlobalContrast =
            pAdrcCtx->NextData.dynParams.Drc_v11.GlobalContrast;
        pAdrcCtx->CurrData.dynParams.Drc_v11.LoLitContrast =
            pAdrcCtx->NextData.dynParams.Drc_v11.LoLitContrast;
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
    float adrc_gain      = pAdrcCtx->NextData.dynParams.Drc_v11.DrcGain;
    float log_ratio2     = log(pAdrcCtx->NextData.AEData.L2S_Ratio * adrc_gain) / log(2.0f) + 12.0f;
    float offsetbits_int = (float)(pAdrcProcRes->DrcProcRes.Drc_v11.offset_pow2);
    float offsetbits     = offsetbits_int * (1 << MFHDR_LOG_Q_BITS);
    float hdrbits        = log_ratio2 * (1 << MFHDR_LOG_Q_BITS);
    float hdrvalidbits   = hdrbits - offsetbits;
    float compres_scl    = (12.0f * (1 << (MFHDR_LOG_Q_BITS * 2))) / hdrvalidbits;
    pAdrcProcRes->DrcProcRes.Drc_v11.compres_scl = (unsigned short)(compres_scl);

    // get sw_drc_min_ogain
    if (pAdrcCtx->NextData.staticParams.OutPutLongFrame)
        pAdrcProcRes->DrcProcRes.Drc_v11.min_ogain = 1 << 15;
    else {
        float sw_drc_min_ogain = 1.0f / (pAdrcCtx->NextData.AEData.L2S_Ratio * adrc_gain);
        pAdrcProcRes->DrcProcRes.Drc_v11.min_ogain =
            (unsigned short)(sw_drc_min_ogain * (1 << 15) + 0.5f);
    }

    // get sw_drc_compres_y
    if (pAdrcCtx->NextData.staticParams.CompressMode == COMPRESS_AUTO) {
        float curveparam, curveparam2, curveparam3, tmp;
        float luma2[DRC_V11_Y_NUM] = {0.0f,     1024.0f,  2048.0f,  3072.0f,  4096.0f,  5120.0f,
                                      6144.0f,  7168.0f,  8192.0f,  10240.0f, 12288.0f, 14336.0f,
                                      16384.0f, 18432.0f, 20480.0f, 22528.0f, 24576.0f};
        float curveTable;
        float dstbits   = ISP_RAW_BIT * (1 << MFHDR_LOG_Q_BITS);
        float validbits = dstbits - offsetbits;
        for (int i = 0; i < DRC_V11_Y_NUM; ++i) {
            curveparam =
                (float)(validbits - 0.0f) / (hdrvalidbits - validbits + DRC_COMPRESS_Y_OFFSET);
            curveparam2 = validbits * (1.0f + curveparam);
            curveparam3   = hdrvalidbits * curveparam;
            tmp           = luma2[i] * hdrvalidbits / 24576.0f;
            curveTable    = (tmp * curveparam2 / (tmp + curveparam3));
            pAdrcProcRes->DrcProcRes.Drc_v11.compres_y[i] = (unsigned short)(curveTable);
        }
    } else if (pAdrcCtx->NextData.staticParams.CompressMode == COMPRESS_MANUAL) {
        for (int i = 0; i < DRC_V11_Y_NUM; ++i)
            pAdrcProcRes->DrcProcRes.Drc_v11.compres_y[i] =
                (unsigned short)(pAdrcCtx->NextData.staticParams.compresManuCurve[i]);
    }

    LOGV_ATMO("%s: nextRatioLS:%f sw_drc_position:%d sw_drc_compres_scl:%d sw_drc_offset_pow2:%d\n",
              __FUNCTION__, pAdrcCtx->NextData.AEData.L2S_Ratio,
              pAdrcProcRes->DrcProcRes.Drc_v11.position,
              pAdrcProcRes->DrcProcRes.Drc_v11.compres_scl,
              pAdrcProcRes->DrcProcRes.Drc_v11.offset_pow2);
    LOGV_ATMO("%s: sw_drc_lpdetail_ratio:%d sw_drc_hpdetail_ratio:%d sw_drc_delta_scalein:%d\n",
              __FUNCTION__, pAdrcProcRes->DrcProcRes.Drc_v11.lpdetail_ratio,
              pAdrcProcRes->DrcProcRes.Drc_v11.hpdetail_ratio,
              pAdrcProcRes->DrcProcRes.Drc_v11.delta_scalein);
    LOGV_ATMO("%s: sw_drc_bilat_wt_off:%d sw_drc_weipre_frame:%d sw_drc_weicur_pix:%d\n",
              __FUNCTION__, pAdrcProcRes->DrcProcRes.Drc_v11.bilat_wt_off,
              pAdrcProcRes->DrcProcRes.Drc_v11.weipre_frame,
              pAdrcProcRes->DrcProcRes.Drc_v11.weicur_pix);
    LOGV_ATMO("%s: sw_drc_edge_scl:%d sw_drc_motion_scl:%d sw_drc_force_sgm_inv0:%d\n",
              __FUNCTION__, pAdrcProcRes->DrcProcRes.Drc_v11.edge_scl,
              pAdrcProcRes->DrcProcRes.Drc_v11.motion_scl,
              pAdrcProcRes->DrcProcRes.Drc_v11.force_sgm_inv0);
    LOGV_ATMO("%s: sw_drc_space_sgm_inv0:%d sw_drc_space_sgm_inv1:%d\n", __FUNCTION__,
              pAdrcProcRes->DrcProcRes.Drc_v11.space_sgm_inv0,
              pAdrcProcRes->DrcProcRes.Drc_v11.space_sgm_inv1);
    LOGV_ATMO("%s: sw_drc_range_sgm_inv0:%d sw_drc_range_sgm_inv1:%d\n", __FUNCTION__,
              pAdrcProcRes->DrcProcRes.Drc_v11.range_sgm_inv0,
              pAdrcProcRes->DrcProcRes.Drc_v11.range_sgm_inv1);
    LOGV_ATMO(
        "%s: sw_drc_weig_bilat:%d sw_drc_weig_maxl:%d sw_drc_bilat_soft_thd:%d "
        "sw_drc_enable_soft_thd:%d\n",
        __FUNCTION__, pAdrcProcRes->DrcProcRes.Drc_v11.weig_bilat,
        pAdrcProcRes->DrcProcRes.Drc_v11.weig_maxl, pAdrcProcRes->DrcProcRes.Drc_v11.bilat_soft_thd,
        pAdrcProcRes->DrcProcRes.Drc_v11.enable_soft_thd);
    LOGV_ATMO("%s: sw_drc_min_ogain:%d sw_drc_iir_weight:%d\n", __FUNCTION__,
              pAdrcProcRes->DrcProcRes.Drc_v11.min_ogain,
              pAdrcProcRes->DrcProcRes.Drc_v11.iir_weight);
    LOGV_ATMO(
        "%s: sw_drc_gain_y: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n", __FUNCTION__,
        pAdrcProcRes->DrcProcRes.Drc_v11.gain_y[0], pAdrcProcRes->DrcProcRes.Drc_v11.gain_y[1],
        pAdrcProcRes->DrcProcRes.Drc_v11.gain_y[2], pAdrcProcRes->DrcProcRes.Drc_v11.gain_y[3],
        pAdrcProcRes->DrcProcRes.Drc_v11.gain_y[4], pAdrcProcRes->DrcProcRes.Drc_v11.gain_y[5],
        pAdrcProcRes->DrcProcRes.Drc_v11.gain_y[6], pAdrcProcRes->DrcProcRes.Drc_v11.gain_y[7],
        pAdrcProcRes->DrcProcRes.Drc_v11.gain_y[8], pAdrcProcRes->DrcProcRes.Drc_v11.gain_y[9],
        pAdrcProcRes->DrcProcRes.Drc_v11.gain_y[10], pAdrcProcRes->DrcProcRes.Drc_v11.gain_y[11],
        pAdrcProcRes->DrcProcRes.Drc_v11.gain_y[12], pAdrcProcRes->DrcProcRes.Drc_v11.gain_y[13],
        pAdrcProcRes->DrcProcRes.Drc_v11.gain_y[14], pAdrcProcRes->DrcProcRes.Drc_v11.gain_y[15],
        pAdrcProcRes->DrcProcRes.Drc_v11.gain_y[16]);
    LOGV_ATMO(
        "%s: sw_drc_scale_y: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n", __FUNCTION__,
        pAdrcProcRes->DrcProcRes.Drc_v11.scale_y[0], pAdrcProcRes->DrcProcRes.Drc_v11.scale_y[1],
        pAdrcProcRes->DrcProcRes.Drc_v11.scale_y[2], pAdrcProcRes->DrcProcRes.Drc_v11.scale_y[3],
        pAdrcProcRes->DrcProcRes.Drc_v11.scale_y[4], pAdrcProcRes->DrcProcRes.Drc_v11.scale_y[5],
        pAdrcProcRes->DrcProcRes.Drc_v11.scale_y[6], pAdrcProcRes->DrcProcRes.Drc_v11.scale_y[7],
        pAdrcProcRes->DrcProcRes.Drc_v11.scale_y[8], pAdrcProcRes->DrcProcRes.Drc_v11.scale_y[9],
        pAdrcProcRes->DrcProcRes.Drc_v11.scale_y[10], pAdrcProcRes->DrcProcRes.Drc_v11.scale_y[11],
        pAdrcProcRes->DrcProcRes.Drc_v11.scale_y[12], pAdrcProcRes->DrcProcRes.Drc_v11.scale_y[13],
        pAdrcProcRes->DrcProcRes.Drc_v11.scale_y[14], pAdrcProcRes->DrcProcRes.Drc_v11.scale_y[15],
        pAdrcProcRes->DrcProcRes.Drc_v11.scale_y[16]);
    LOGV_ATMO(
        "%s: CompressMode:%d sw_drc_compres_y: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d "
        "%d\n",
        __FUNCTION__, pAdrcCtx->NextData.staticParams.CompressMode,
        pAdrcProcRes->DrcProcRes.Drc_v11.compres_y[0],
        pAdrcProcRes->DrcProcRes.Drc_v11.compres_y[1],
        pAdrcProcRes->DrcProcRes.Drc_v11.compres_y[2],
        pAdrcProcRes->DrcProcRes.Drc_v11.compres_y[3],
        pAdrcProcRes->DrcProcRes.Drc_v11.compres_y[4],
        pAdrcProcRes->DrcProcRes.Drc_v11.compres_y[5],
        pAdrcProcRes->DrcProcRes.Drc_v11.compres_y[6],
        pAdrcProcRes->DrcProcRes.Drc_v11.compres_y[7],
        pAdrcProcRes->DrcProcRes.Drc_v11.compres_y[8],
        pAdrcProcRes->DrcProcRes.Drc_v11.compres_y[9],
        pAdrcProcRes->DrcProcRes.Drc_v11.compres_y[10],
        pAdrcProcRes->DrcProcRes.Drc_v11.compres_y[11],
        pAdrcProcRes->DrcProcRes.Drc_v11.compres_y[12],
        pAdrcProcRes->DrcProcRes.Drc_v11.compres_y[13],
        pAdrcProcRes->DrcProcRes.Drc_v11.compres_y[14],
        pAdrcProcRes->DrcProcRes.Drc_v11.compres_y[15],
        pAdrcProcRes->DrcProcRes.Drc_v11.compres_y[16]);

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
    else if (pAdrcCtx->drcAttrV11.opMode != pAdrcCtx->CurrData.ApiMode)
        bypass = false;
    else if (pAdrcCtx->drcAttrV11.opMode == DRC_OPMODE_MANUAL)
        bypass = !pAdrcCtx->ifReCalcStManual;
    else if (pAdrcCtx->drcAttrV11.opMode == DRC_OPMODE_AUTO) {
        if (pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.CtrlDataType == CTRLDATATYPE_ENVLV) {
            diff = pAdrcCtx->CurrData.AEData.EnvLv - pAdrcCtx->NextData.AEData.EnvLv;
            if (pAdrcCtx->CurrData.AEData.EnvLv <= FLT_EPSILON) {
                diff = pAdrcCtx->NextData.AEData.EnvLv;
                if (diff <= FLT_EPSILON)
                    bypass = true;
                else
                    bypass = false;
            } else {
                diff /= pAdrcCtx->CurrData.AEData.EnvLv;
                if (diff >= pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.ByPassThr ||
                    diff <= -pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.ByPassThr)
                    bypass = false;
                else
                    bypass = true;
            }
        } else if (pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.CtrlDataType == CTRLDATATYPE_ISO) {
            diff = pAdrcCtx->CurrData.AEData.ISO - pAdrcCtx->NextData.AEData.ISO;
            diff /= pAdrcCtx->CurrData.AEData.ISO;
            if (diff >= pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.ByPassThr ||
                diff <= -pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.ByPassThr)
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
        pAdrcCtx->NextData.AEData.LongFrmMode, pAdrcCtx->drcAttrV11.opMode,
        pAdrcCtx->ifReCalcStAuto, pAdrcCtx->ifReCalcStManual,
        pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.CtrlDataType, pAdrcCtx->NextData.AEData.EnvLv,
        pAdrcCtx->NextData.AEData.ISO, bypass);

    LOG1_ATMO(
        "%s: NextEnvLv:%f CurrEnvLv:%f NextISO:%f CurrISO:%f diff:%f ByPassThr:%f opMode:%d "
        "bypass:%d!\n",
        __FUNCTION__, pAdrcCtx->NextData.AEData.EnvLv, pAdrcCtx->CurrData.AEData.EnvLv,
        pAdrcCtx->NextData.AEData.ISO, pAdrcCtx->CurrData.AEData.ISO, diff,
        pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.ByPassThr, pAdrcCtx->drcAttrV11.opMode, bypass);

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
    *ppAdrcCtx      = pAdrcCtx;
    pAdrcCtx->state = ADRC_STATE_INITIALIZED;

    CalibDbV2_drc_V11_t* calibv2_adrc_calib =
        (CalibDbV2_drc_V11_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibDb, adrc_calib));
    memcpy(&pAdrcCtx->drcAttrV11.stAuto, calibv2_adrc_calib, sizeof(CalibDbV2_drc_V11_t));
    pAdrcCtx->drcAttrV11.opMode = DRC_OPMODE_AUTO;
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
        LOGE_ATMO("%s: AHDRStop() failed!\n", __FUNCTION__);
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
