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
 * @file rk_aiq_amerge_algo_v10.cpp
 *
 * @brief
 *   ADD_DESCRIPTION_HERE
 *
 *****************************************************************************/
#include "math.h"
#include "rk_aiq_types_amerge_algo_int.h"
#include "rk_aiq_types_amerge_algo_prvt.h"
#include "xcam_log.h"

/******************************************************************************
 * AmergeStart()
 *****************************************************************************/
XCamReturn AmergeStart(AmergeContext_t* pAmergeCtx) {
    LOG1_AMERGE("%s:enter!\n", __FUNCTION__);

    // initial checks
    if (pAmergeCtx == NULL) {
        return (XCAM_RETURN_ERROR_MEM);
    }

    if ((AMERGE_STATE_RUNNING == pAmergeCtx->state) || (AMERGE_STATE_LOCKED == pAmergeCtx->state)) {
        return (XCAM_RETURN_ERROR_FAILED);
    }

    pAmergeCtx->state = AMERGE_STATE_RUNNING;

    LOG1_AMERGE("%s:exit!\n", __FUNCTION__);
    return (XCAM_RETURN_NO_ERROR);
}
/******************************************************************************
 * AmergeStop()
 *****************************************************************************/
XCamReturn AmergeStop(AmergeContext_t* pAmergeCtx) {
    LOG1_AMERGE("%s:enter!\n", __FUNCTION__);

    // initial checks
    if (pAmergeCtx == NULL) {
        return (XCAM_RETURN_ERROR_MEM);
    }

    // before stopping, unlock the AHDR if locked
    if (AMERGE_STATE_LOCKED == pAmergeCtx->state) {
        return (XCAM_RETURN_ERROR_FAILED);
    }

    pAmergeCtx->state = AMERGE_STATE_STOPPED;

    LOG1_AMERGE("%s:exit!\n", __FUNCTION__);

    return (XCAM_RETURN_NO_ERROR);
}

int mergeClipValueV10(float posx, int BitInt, int BitFloat, bool ifBitMax) {
    int yOutInt = 0, yOutIntMin = 0, yOutIntMax = 0;

    if (ifBitMax)
        yOutIntMax = (int)(pow(2, (BitFloat + BitInt)));
    else
        yOutIntMax = (int)(pow(2, (BitFloat + BitInt)) - 1);
    yOutInt = LIMIT_VALUE((int)(posx * pow(2, BitFloat)), yOutIntMax, yOutIntMin);

    return yOutInt;
}

float MergeGetInterpRatioV10(float* pX, int& lo, int& hi, float CtrlValue, int length_max) {
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
 * CalibrateOECurveV10()
 *****************************************************************************/
void CalibrateOECurveV10(float smooth, float offset, unsigned short* OECurve) {
    LOG1_AMERGE("%s:Enter!\n", __FUNCTION__);

    int step    = 32;
    float curve = 0.0f;
    float k     = 511.0f;

    for (int i = 0; i < HDRMGE_V10_OE_CURVE_NUM; ++i) {
        curve      = 1.0f + exp(-smooth * OECURVESMOOTHMAX * (k / 1023.0f - offset / 256.0f));
        curve      = 1024.0f / curve;
        OECurve[i] = round(curve);
        OECurve[i] = MIN(OECurve[i], 1023);
        k += step;
    }

    LOG1_AMERGE("%s:Eixt!\n", __FUNCTION__);
}
/******************************************************************************
 * CalibrateMDCurveV10()
 *****************************************************************************/
void CalibrateMDCurveV10(float smooth, float offset, unsigned short* MDCurve) {
    LOG1_AMERGE("%s:Enter!\n", __FUNCTION__);

    int step    = 16;
    float curve = 0.0f;
    float k     = 0.0f;

    for (int i = 0; i < HDRMGE_V10_OE_CURVE_NUM; ++i) {
        curve      = 1.0f + exp(-smooth * MDCURVESMOOTHMAX *
                           (k / 1023.0f - offset * MDCURVEOFFSETMAX / 256.0f));
        curve      = 1024.0f / curve;
        MDCurve[i] = round(curve);
        MDCurve[i] = MIN(MDCurve[i], 1023);
        k += step;
    }

    LOG1_AMERGE("%s:Eixt!\n", __FUNCTION__);
}

/******************************************************************************
 * AmergeGetTuningProcResV10()
 *****************************************************************************/
void AmergeGetTuningProcResV10(AmergeContext_t* pAmergeCtx,
                               RkAiqAmergeProcResult_t* pAmergeProcRes) {
    LOG1_AMERGE("%s:Enter!\n", __FUNCTION__);

    pAmergeProcRes->Merge_v10.sw_hdrmge_mode = pAmergeCtx->NextData.HandleData.Merge_v10.MergeMode;
    if (CHECK_ISP_HW_V21())
        pAmergeProcRes->Merge_v10.sw_hdrmge_mode =
            LIMIT_VALUE_UNSIGNED(pAmergeProcRes->Merge_v10.sw_hdrmge_mode, 1);
    pAmergeProcRes->Merge_v10.sw_hdrmge_lm_dif_0p9 = SW_HDRMGE_LM_DIF_0P9_FIX;
    pAmergeProcRes->Merge_v10.sw_hdrmge_ms_dif_0p8 = SW_HDRMGE_MS_DIF_0P8_FIX;
    pAmergeProcRes->Merge_v10.sw_hdrmge_lm_dif_0p15 =
        (unsigned char)(pAmergeCtx->NextData.HandleData.Merge_v10.MDCurveLM_offset *
                        MDCURVEOFFSETMAX);
    pAmergeProcRes->Merge_v10.sw_hdrmge_ms_dif_0p15 =
        (unsigned char)(pAmergeCtx->NextData.HandleData.Merge_v10.MDCurveMS_offset *
                        MDCURVEOFFSETMAX);

    if (pAmergeCtx->NextData.CtrlData.ExpoData.LongFrmMode) {
        for (int i = 0; i < HDRMGE_V10_OE_CURVE_NUM; i++)
            pAmergeProcRes->Merge_v10.sw_hdrmge_e_y[i] = HDR_LONG_FRMAE_MODE_OECURVE;
    } else {
        CalibrateOECurveV10(pAmergeCtx->NextData.HandleData.Merge_v10.OECurve_smooth,
                            pAmergeCtx->NextData.HandleData.Merge_v10.OECurve_offset,
                            pAmergeProcRes->Merge_v10.sw_hdrmge_e_y);
        CalibrateMDCurveV10(pAmergeCtx->NextData.HandleData.Merge_v10.MDCurveLM_smooth,
                            pAmergeCtx->NextData.HandleData.Merge_v10.MDCurveLM_offset,
                            pAmergeProcRes->Merge_v10.sw_hdrmge_l1_y);
        CalibrateMDCurveV10(pAmergeCtx->NextData.HandleData.Merge_v10.MDCurveMS_smooth,
                            pAmergeCtx->NextData.HandleData.Merge_v10.MDCurveMS_offset,
                            pAmergeProcRes->Merge_v10.sw_hdrmge_l0_y);
    }

    LOG1_AMERGE("%s:Eixt!\n", __FUNCTION__);
}

/******************************************************************************
 * MergeDampingV10()
 *****************************************************************************/
void MergeDampingV10(AmergeContext_t* pAmergeCtx) {
    LOG1_AMERGE("%s:Enter!\n", __FUNCTION__);

    if (pAmergeCtx->FrameID && (pAmergeCtx->NextData.HandleData.Merge_v10.MergeMode ==
                                pAmergeCtx->CurrData.HandleData.Merge_v10.MergeMode)) {
        pAmergeCtx->NextData.HandleData.Merge_v10.OECurve_smooth =
            pAmergeCtx->NextData.CtrlData.MergeOEDamp *
                pAmergeCtx->NextData.HandleData.Merge_v10.OECurve_smooth +
            (1.0f - pAmergeCtx->NextData.CtrlData.MergeOEDamp) *
                pAmergeCtx->CurrData.HandleData.Merge_v10.OECurve_smooth;
        pAmergeCtx->NextData.HandleData.Merge_v10.OECurve_offset =
            pAmergeCtx->NextData.CtrlData.MergeOEDamp *
                pAmergeCtx->NextData.HandleData.Merge_v10.OECurve_offset +
            (1.0f - pAmergeCtx->NextData.CtrlData.MergeOEDamp) *
                pAmergeCtx->CurrData.HandleData.Merge_v10.OECurve_offset;
        pAmergeCtx->NextData.HandleData.Merge_v10.MDCurveLM_smooth =
            pAmergeCtx->NextData.CtrlData.MergeMDDampLM *
                pAmergeCtx->NextData.HandleData.Merge_v10.MDCurveLM_smooth +
            (1.0f - pAmergeCtx->NextData.CtrlData.MergeMDDampLM) *
                pAmergeCtx->CurrData.HandleData.Merge_v10.MDCurveLM_smooth;
        pAmergeCtx->NextData.HandleData.Merge_v10.MDCurveLM_offset =
            pAmergeCtx->NextData.CtrlData.MergeMDDampLM *
                pAmergeCtx->NextData.HandleData.Merge_v10.MDCurveLM_offset +
            (1.0f - pAmergeCtx->NextData.CtrlData.MergeMDDampLM) *
                pAmergeCtx->CurrData.HandleData.Merge_v10.MDCurveLM_offset;
        pAmergeCtx->NextData.HandleData.Merge_v10.MDCurveMS_smooth =
            pAmergeCtx->NextData.CtrlData.MergeMDDampMS *
                pAmergeCtx->NextData.HandleData.Merge_v10.MDCurveMS_smooth +
            (1.0f - pAmergeCtx->NextData.CtrlData.MergeMDDampMS) *
                pAmergeCtx->CurrData.HandleData.Merge_v10.MDCurveMS_smooth;
        pAmergeCtx->NextData.HandleData.Merge_v10.MDCurveMS_offset =
            pAmergeCtx->NextData.CtrlData.MergeMDDampMS *
                pAmergeCtx->NextData.HandleData.Merge_v10.MDCurveMS_offset +
            (1.0f - pAmergeCtx->NextData.CtrlData.MergeMDDampMS) *
                pAmergeCtx->CurrData.HandleData.Merge_v10.MDCurveMS_offset;
    }

    LOG1_AMERGE("%s:Eixt!\n", __FUNCTION__);
}

/******************************************************************************
 * AmergeTuningProcessing()
 *get handle para by config and current variate
 *****************************************************************************/
void AmergeTuningProcessing(AmergeContext_t* pAmergeCtx, RkAiqAmergeProcResult_t* pAmergeProcRes) {
    LOG1_AMERGE("%s:enter!\n", __FUNCTION__);

    pAmergeCtx->NextData.HandleData.Merge_v10.MergeMode = pAmergeCtx->FrameNumber - 1;

    if (pAmergeCtx->mergeAttrV10.opMode == MERGE_OPMODE_AUTO) {
        int lo = 0, hi = 0;
        float ratio     = 0.0f;
        float CtrlValue = pAmergeCtx->NextData.CtrlData.ExpoData.EnvLv;
        if (pAmergeCtx->mergeAttrV10.stAuto.MergeTuningPara.CtrlDataType == CTRLDATATYPE_ISO)
            CtrlValue = pAmergeCtx->NextData.CtrlData.ExpoData.ISO;

        // get Current merge OECurve
        ratio =
            MergeGetInterpRatioV10(pAmergeCtx->mergeAttrV10.stAuto.MergeTuningPara.OECurve.CtrlData,
                                   lo, hi, CtrlValue, MERGE_ENVLV_STEP_MAX);
        pAmergeCtx->NextData.HandleData.Merge_v10.OECurve_smooth =
            ratio * (pAmergeCtx->mergeAttrV10.stAuto.MergeTuningPara.OECurve.Smooth[hi] -
                     pAmergeCtx->mergeAttrV10.stAuto.MergeTuningPara.OECurve.Smooth[lo]) +
            pAmergeCtx->mergeAttrV10.stAuto.MergeTuningPara.OECurve.Smooth[lo];
        pAmergeCtx->NextData.HandleData.Merge_v10.OECurve_offset =
            ratio * (pAmergeCtx->mergeAttrV10.stAuto.MergeTuningPara.OECurve.Offset[hi] -
                     pAmergeCtx->mergeAttrV10.stAuto.MergeTuningPara.OECurve.Offset[lo]) +
            pAmergeCtx->mergeAttrV10.stAuto.MergeTuningPara.OECurve.Offset[lo];

        // get Current merge MDCurve
        ratio = MergeGetInterpRatioV10(
            pAmergeCtx->mergeAttrV10.stAuto.MergeTuningPara.MDCurve.MoveCoef, lo, hi,
            pAmergeCtx->NextData.CtrlData.MoveCoef, MERGE_ENVLV_STEP_MAX);
        pAmergeCtx->NextData.HandleData.Merge_v10.MDCurveLM_smooth =
            ratio * (pAmergeCtx->mergeAttrV10.stAuto.MergeTuningPara.MDCurve.LM_smooth[hi] -
                     pAmergeCtx->mergeAttrV10.stAuto.MergeTuningPara.MDCurve.LM_smooth[lo]) +
            pAmergeCtx->mergeAttrV10.stAuto.MergeTuningPara.MDCurve.LM_smooth[lo];
        pAmergeCtx->NextData.HandleData.Merge_v10.MDCurveLM_offset =
            ratio * (pAmergeCtx->mergeAttrV10.stAuto.MergeTuningPara.MDCurve.LM_offset[hi] -
                     pAmergeCtx->mergeAttrV10.stAuto.MergeTuningPara.MDCurve.LM_offset[lo]) +
            pAmergeCtx->mergeAttrV10.stAuto.MergeTuningPara.MDCurve.LM_offset[lo];
        pAmergeCtx->NextData.HandleData.Merge_v10.MDCurveMS_smooth =
            ratio * (pAmergeCtx->mergeAttrV10.stAuto.MergeTuningPara.MDCurve.MS_smooth[hi] -
                     pAmergeCtx->mergeAttrV10.stAuto.MergeTuningPara.MDCurve.MS_smooth[lo]) +
            pAmergeCtx->mergeAttrV10.stAuto.MergeTuningPara.MDCurve.MS_smooth[lo];
        pAmergeCtx->NextData.HandleData.Merge_v10.MDCurveMS_offset =
            ratio * (pAmergeCtx->mergeAttrV10.stAuto.MergeTuningPara.MDCurve.MS_offset[hi] -
                     pAmergeCtx->mergeAttrV10.stAuto.MergeTuningPara.MDCurve.MS_offset[lo]) +
            pAmergeCtx->mergeAttrV10.stAuto.MergeTuningPara.MDCurve.MS_offset[lo];

        // others
        pAmergeCtx->NextData.CtrlData.MergeOEDamp =
            pAmergeCtx->mergeAttrV10.stAuto.MergeTuningPara.OECurve_damp;
        pAmergeCtx->NextData.CtrlData.MergeMDDampLM =
            pAmergeCtx->mergeAttrV10.stAuto.MergeTuningPara.MDCurveLM_damp;
        pAmergeCtx->NextData.CtrlData.MergeMDDampMS =
            pAmergeCtx->mergeAttrV10.stAuto.MergeTuningPara.MDCurveMS_damp;

        // merge damp
        MergeDampingV10(pAmergeCtx);
    } else if (pAmergeCtx->mergeAttrV10.opMode == MERGE_OPMODE_MANUAL) {
        pAmergeCtx->NextData.HandleData.Merge_v10.OECurve_smooth =
            pAmergeCtx->mergeAttrV10.stManual.OECurve.Smooth;
        pAmergeCtx->NextData.HandleData.Merge_v10.OECurve_offset =
            pAmergeCtx->mergeAttrV10.stManual.OECurve.Offset;

        // get Current merge MDCurve
        pAmergeCtx->NextData.HandleData.Merge_v10.MDCurveLM_smooth =
            pAmergeCtx->mergeAttrV10.stManual.MDCurve.LM_smooth;
        pAmergeCtx->NextData.HandleData.Merge_v10.MDCurveLM_offset =
            pAmergeCtx->mergeAttrV10.stManual.MDCurve.LM_offset;
        pAmergeCtx->NextData.HandleData.Merge_v10.MDCurveMS_smooth =
            pAmergeCtx->mergeAttrV10.stManual.MDCurve.MS_smooth;
        pAmergeCtx->NextData.HandleData.Merge_v10.MDCurveMS_offset =
            pAmergeCtx->mergeAttrV10.stManual.MDCurve.MS_offset;
    } else
        LOGE_AMERGE("%s(%d): Amerge wrong mode!!!\n", __FUNCTION__, __LINE__);

    LOGD_AMERGE("%s: Current OECurve_smooth:%f OECurve_offset:%f \n", __FUNCTION__,
                pAmergeCtx->NextData.HandleData.Merge_v10.OECurve_smooth,
                pAmergeCtx->NextData.HandleData.Merge_v10.OECurve_offset);
    LOGD_AMERGE(
        "%s: Current MDCurveMS_smooth:%f MDCurveMS_offset:%f MDCurveLM_smooth:%f "
        "MDCurveLM_offset:%f \n",
        __FUNCTION__, pAmergeCtx->NextData.HandleData.Merge_v10.MDCurveMS_smooth,
        pAmergeCtx->NextData.HandleData.Merge_v10.MDCurveMS_offset,
        pAmergeCtx->NextData.HandleData.Merge_v10.MDCurveLM_smooth,
        pAmergeCtx->NextData.HandleData.Merge_v10.MDCurveLM_offset);

    // get current IO data
    AmergeGetTuningProcResV10(pAmergeCtx, pAmergeProcRes);

    // store current handle data to pre data for next loop
    pAmergeCtx->NextData.CtrlData.ExpoData.EnvLv = pAmergeCtx->NextData.CtrlData.ExpoData.EnvLv;
    pAmergeCtx->NextData.CtrlData.ExpoData.ISO   = pAmergeCtx->NextData.CtrlData.ExpoData.ISO;
    pAmergeCtx->CurrData.CtrlData.MoveCoef       = pAmergeCtx->NextData.CtrlData.MoveCoef;
    pAmergeCtx->CurrData.CtrlData.ApiMode        = pAmergeCtx->mergeAttrV10.opMode;
    memcpy(&pAmergeCtx->CurrData.HandleData, &pAmergeCtx->NextData.HandleData,
           sizeof(MergeHandleData_s));

    LOG1_AMERGE("%s:exit!\n", __FUNCTION__);
}

/******************************************************************************
 * AmergeExpoProcessing()
 *get handle para by config and current variate
 *****************************************************************************/
void AmergeExpoProcessing(AmergeContext_t* pAmergeCtx, MergeExpoData_t* pExpoData,
                          RkAiqAmergeProcResult_t* pAmergeProcRes) {
    LOG1_AMERGE("%s:enter!\n", __FUNCTION__);

    // get sw_hdrmge_gain0
    pAmergeProcRes->Merge_v10.sw_hdrmge_gain0 = mergeClipValueV10(pExpoData->RatioLS, 8, 6, false);
    pAmergeProcRes->Merge_v10.sw_hdrmge_gain0_inv =
        mergeClipValueV10(RATIO_DEFAULT / pExpoData->RatioLS, 0, 12, false);

    // get sw_hdrmge_gain1
    pAmergeProcRes->Merge_v10.sw_hdrmge_gain1     = SW_HDRMGE_GAIN_FIX;
    pAmergeProcRes->Merge_v10.sw_hdrmge_gain1_inv = SW_HDRMGE_GAIN_INV_FIX;

    // get sw_hdrmge_gain2
    pAmergeProcRes->Merge_v10.sw_hdrmge_gain2 = SW_HDRMGE_GAIN_FIX;

    // store next data
    pAmergeCtx->CurrData.CtrlData.ExpoData.LongFrmMode =
        pAmergeCtx->NextData.CtrlData.ExpoData.LongFrmMode;
    pAmergeCtx->CurrData.CtrlData.ExpoData.RatioLS = pAmergeCtx->NextData.CtrlData.ExpoData.RatioLS;
    pAmergeCtx->CurrData.CtrlData.ExpoData.RatioLM = pAmergeCtx->NextData.CtrlData.ExpoData.RatioLM;

    LOGV_AMERGE(
        "%s: sw_hdrmge_mode:%d sw_hdrmge_ms_dif_0p8:%d sw_hdrmge_lm_dif_0p9:%d "
        "sw_hdrmge_ms_dif_0p15:%d sw_hdrmge_lm_dif_0p15:%d\n",
        __FUNCTION__, pAmergeProcRes->Merge_v10.sw_hdrmge_mode,
        pAmergeProcRes->Merge_v10.sw_hdrmge_ms_dif_0p8,
        pAmergeProcRes->Merge_v10.sw_hdrmge_lm_dif_0p9,
        pAmergeProcRes->Merge_v10.sw_hdrmge_ms_dif_0p15,
        pAmergeProcRes->Merge_v10.sw_hdrmge_lm_dif_0p15);
    LOGV_AMERGE(
        "%s: RatioLS:%f sw_hdrmge_gain0:%d sw_hdrmge_gain0_inv:%d RatioLM:%f "
        "sw_hdrmge_gain1:%d sw_hdrmge_gain1_inv:%d sw_hdrmge_gain2:%d\n",
        __FUNCTION__, pExpoData->RatioLS, pAmergeProcRes->Merge_v10.sw_hdrmge_gain0,
        pAmergeProcRes->Merge_v10.sw_hdrmge_gain0_inv, pExpoData->RatioLM,
        pAmergeProcRes->Merge_v10.sw_hdrmge_gain1, pAmergeProcRes->Merge_v10.sw_hdrmge_gain1_inv,
        pAmergeProcRes->Merge_v10.sw_hdrmge_gain2);
    LOGV_AMERGE(
        "%s: sw_hdrmge_e_y: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n", __FUNCTION__,
        pAmergeProcRes->Merge_v10.sw_hdrmge_e_y[0], pAmergeProcRes->Merge_v10.sw_hdrmge_e_y[1],
        pAmergeProcRes->Merge_v10.sw_hdrmge_e_y[2], pAmergeProcRes->Merge_v10.sw_hdrmge_e_y[3],
        pAmergeProcRes->Merge_v10.sw_hdrmge_e_y[4], pAmergeProcRes->Merge_v10.sw_hdrmge_e_y[5],
        pAmergeProcRes->Merge_v10.sw_hdrmge_e_y[6], pAmergeProcRes->Merge_v10.sw_hdrmge_e_y[7],
        pAmergeProcRes->Merge_v10.sw_hdrmge_e_y[8], pAmergeProcRes->Merge_v10.sw_hdrmge_e_y[9],
        pAmergeProcRes->Merge_v10.sw_hdrmge_e_y[10], pAmergeProcRes->Merge_v10.sw_hdrmge_e_y[11],
        pAmergeProcRes->Merge_v10.sw_hdrmge_e_y[12], pAmergeProcRes->Merge_v10.sw_hdrmge_e_y[13],
        pAmergeProcRes->Merge_v10.sw_hdrmge_e_y[14], pAmergeProcRes->Merge_v10.sw_hdrmge_e_y[15],
        pAmergeProcRes->Merge_v10.sw_hdrmge_e_y[16]);
    LOGV_AMERGE(
        "%s: sw_hdrmge_l0_y: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n", __FUNCTION__,
        pAmergeProcRes->Merge_v10.sw_hdrmge_l0_y[0], pAmergeProcRes->Merge_v10.sw_hdrmge_l0_y[1],
        pAmergeProcRes->Merge_v10.sw_hdrmge_l0_y[2], pAmergeProcRes->Merge_v10.sw_hdrmge_l0_y[3],
        pAmergeProcRes->Merge_v10.sw_hdrmge_l0_y[4], pAmergeProcRes->Merge_v10.sw_hdrmge_l0_y[5],
        pAmergeProcRes->Merge_v10.sw_hdrmge_l0_y[6], pAmergeProcRes->Merge_v10.sw_hdrmge_l0_y[7],
        pAmergeProcRes->Merge_v10.sw_hdrmge_l0_y[8], pAmergeProcRes->Merge_v10.sw_hdrmge_l0_y[9],
        pAmergeProcRes->Merge_v10.sw_hdrmge_l0_y[10], pAmergeProcRes->Merge_v10.sw_hdrmge_l0_y[11],
        pAmergeProcRes->Merge_v10.sw_hdrmge_l0_y[12], pAmergeProcRes->Merge_v10.sw_hdrmge_l0_y[13],
        pAmergeProcRes->Merge_v10.sw_hdrmge_l0_y[14], pAmergeProcRes->Merge_v10.sw_hdrmge_l0_y[15],
        pAmergeProcRes->Merge_v10.sw_hdrmge_l0_y[16]);
    LOGV_AMERGE(
        "%s: sw_hdrmge_l1_y: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n", __FUNCTION__,
        pAmergeProcRes->Merge_v10.sw_hdrmge_l1_y[0], pAmergeProcRes->Merge_v10.sw_hdrmge_l1_y[1],
        pAmergeProcRes->Merge_v10.sw_hdrmge_l1_y[2], pAmergeProcRes->Merge_v10.sw_hdrmge_l1_y[3],
        pAmergeProcRes->Merge_v10.sw_hdrmge_l1_y[4], pAmergeProcRes->Merge_v10.sw_hdrmge_l1_y[5],
        pAmergeProcRes->Merge_v10.sw_hdrmge_l1_y[6], pAmergeProcRes->Merge_v10.sw_hdrmge_l1_y[7],
        pAmergeProcRes->Merge_v10.sw_hdrmge_l1_y[8], pAmergeProcRes->Merge_v10.sw_hdrmge_l1_y[9],
        pAmergeProcRes->Merge_v10.sw_hdrmge_l1_y[10], pAmergeProcRes->Merge_v10.sw_hdrmge_l1_y[11],
        pAmergeProcRes->Merge_v10.sw_hdrmge_l1_y[12], pAmergeProcRes->Merge_v10.sw_hdrmge_l1_y[13],
        pAmergeProcRes->Merge_v10.sw_hdrmge_l1_y[14], pAmergeProcRes->Merge_v10.sw_hdrmge_l1_y[15],
        pAmergeProcRes->Merge_v10.sw_hdrmge_l1_y[16]);

    LOG1_AMERGE("%s:exit!\n", __FUNCTION__);
}

/******************************************************************************
 * AmergeByPassProcessing()
 *get handle para by config and current variate
 *****************************************************************************/
bool AmergeByPassProcessing(AmergeContext_t* pAmergeCtx) {
    LOG1_AMERGE("%s:enter!\n", __FUNCTION__);

    bool bypass = false;
    float diff  = 0.0f;

    if (pAmergeCtx->FrameID <= INIT_CALC_PARAMS_NUM)
        bypass = false;
    else if (pAmergeCtx->mergeAttrV10.opMode != pAmergeCtx->CurrData.CtrlData.ApiMode)
        bypass = false;
    else if (pAmergeCtx->mergeAttrV10.opMode == MERGE_OPMODE_MANUAL)
        bypass = !pAmergeCtx->ifReCalcStManual;
    else if (pAmergeCtx->mergeAttrV10.opMode == MERGE_OPMODE_AUTO) {
        if (pAmergeCtx->mergeAttrV10.stAuto.MergeTuningPara.CtrlDataType == CTRLDATATYPE_ENVLV) {
            diff = pAmergeCtx->CurrData.CtrlData.ExpoData.EnvLv -
                   pAmergeCtx->NextData.CtrlData.ExpoData.EnvLv;
            if (pAmergeCtx->CurrData.CtrlData.ExpoData.EnvLv <= FLT_EPSILON) {
                diff = pAmergeCtx->NextData.CtrlData.ExpoData.EnvLv;
                if (diff <= FLT_EPSILON)
                    bypass = true;
                else
                    bypass = false;
            } else {
                diff /= pAmergeCtx->CurrData.CtrlData.ExpoData.EnvLv;
                if (diff >= pAmergeCtx->mergeAttrV10.stAuto.MergeTuningPara.ByPassThr ||
                    diff <= -pAmergeCtx->mergeAttrV10.stAuto.MergeTuningPara.ByPassThr)
                    bypass = false;
                else
                    bypass = true;
            }
        } else if (pAmergeCtx->mergeAttrV10.stAuto.MergeTuningPara.CtrlDataType ==
                   CTRLDATATYPE_ISO) {
            diff = pAmergeCtx->CurrData.CtrlData.ExpoData.ISO -
                   pAmergeCtx->NextData.CtrlData.ExpoData.ISO;
            diff /= pAmergeCtx->CurrData.CtrlData.ExpoData.ISO;
            if (diff >= pAmergeCtx->mergeAttrV10.stAuto.MergeTuningPara.ByPassThr ||
                diff <= -pAmergeCtx->mergeAttrV10.stAuto.MergeTuningPara.ByPassThr)
                bypass = false;
            else
                bypass = true;
        }
        bypass = bypass && !pAmergeCtx->ifReCalcStAuto;
    }

    LOGD_AMERGE(
        "%s: FrameID:%d HDRFrameNum:%d LongFrmMode:%d MergeApiMode:%d ifReCalcStAuto:%d "
        "ifReCalcStManual:%d CtrlDataType:%d EnvLv:%f ISO:%f MoveCoef:%f "
        "bypass:%d\n",
        __FUNCTION__, pAmergeCtx->FrameID, pAmergeCtx->FrameNumber,
        pAmergeCtx->NextData.CtrlData.ExpoData.LongFrmMode, pAmergeCtx->mergeAttrV10.opMode,
        pAmergeCtx->ifReCalcStAuto, pAmergeCtx->ifReCalcStManual,
        pAmergeCtx->mergeAttrV10.stAuto.MergeTuningPara.CtrlDataType,
        pAmergeCtx->NextData.CtrlData.ExpoData.EnvLv, pAmergeCtx->NextData.CtrlData.ExpoData.ISO,
        pAmergeCtx->NextData.CtrlData.MoveCoef, bypass);

    LOG1_AMERGE("%s:exit!\n", __FUNCTION__);
    return bypass;
}

/******************************************************************************
 * AmergeInit()
 *****************************************************************************/
XCamReturn AmergeInit(AmergeContext_t** ppAmergeCtx, CamCalibDbV2Context_t* pCalibV2) {
    LOG1_AMERGE("%s:enter!\n", __FUNCTION__);

    XCamReturn result = XCAM_RETURN_NO_ERROR;

    AmergeContext_t* pAmergeCtx;
    pAmergeCtx = (AmergeContext_t*)malloc(sizeof(AmergeContext_t));
    if (pAmergeCtx == NULL) {
        LOGE_ATMO("%s(%d): invalid inputparams\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_MEM;
    }
    *ppAmergeCtx      = pAmergeCtx;
    pAmergeCtx->state = AMERGE_STATE_INITIALIZED;

    CalibDbV2_merge_v10_t* calibv2_amerge_calib =
        (CalibDbV2_merge_v10_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibV2, amerge_calib));
    memcpy(&pAmergeCtx->mergeAttrV10.stAuto, calibv2_amerge_calib, sizeof(CalibDbV2_merge_v10_t));
    pAmergeCtx->mergeAttrV10.opMode = MERGE_OPMODE_AUTO;
    pAmergeCtx->ifReCalcStAuto      = true;
    pAmergeCtx->ifReCalcStManual    = false;
    pAmergeCtx->isCapture           = false;

    LOG1_AMERGE("%s:exit!\n", __FUNCTION__);
    return (result);
}
/******************************************************************************
 * AmergeRelease()
 *****************************************************************************/
XCamReturn AmergeRelease(AmergeContext_t* pAmergeCtx) {
    LOG1_AMERGE("%s:enter!\n", __FUNCTION__);
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    // initial checks
    if (NULL == pAmergeCtx) {
        return (XCAM_RETURN_ERROR_MEM);
    }

    result = AmergeStop(pAmergeCtx);
    if (result != XCAM_RETURN_NO_ERROR) {
        LOGE_AMERGE("%s(%d): Amerge Stop() failed!\n", __FUNCTION__, __LINE__);
        return (result);
    }

    // check state
    if ((AMERGE_STATE_RUNNING == pAmergeCtx->state) || (AMERGE_STATE_LOCKED == pAmergeCtx->state)) {
        return (XCAM_RETURN_ERROR_FAILED);
    }

    free(pAmergeCtx);
    pAmergeCtx = NULL;

    LOG1_AMERGE("%s:exit!\n", __FUNCTION__);

    return (result);
}
