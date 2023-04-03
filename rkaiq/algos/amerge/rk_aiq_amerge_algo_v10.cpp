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

/******************************************************************************
 * GetCurrParaV10()
 *****************************************************************************/
float GetCurrParaV10(float inPara, float* inMatrixX, float* inMatrixY, int Max_Knots) {
    LOG1_AMERGE("%s:enter!\n", __FUNCTION__);
    float x1      = 0.0f;
    float x2      = 0.0f;
    float value1  = 0.0f;
    float value2  = 0.0f;
    float outPara = 0.0f;

    if (inPara < inMatrixX[0])
        outPara = inMatrixY[0];
    else if (inPara >= inMatrixX[Max_Knots - 1])
        outPara = inMatrixY[Max_Knots - 1];
    else
        for (int i = 0; i < Max_Knots - 1; i++) {
            if (inPara >= inMatrixX[i] && inPara < inMatrixX[i + 1]) {
                x1      = inMatrixX[i];
                x2      = inMatrixX[i + 1];
                value1  = inMatrixY[i];
                value2  = inMatrixY[i + 1];
                outPara = value1 + (inPara - x1) * (value1 - value2) / (x1 - x2);
                break;
            } else
                continue;
        }

    return outPara;
    LOG1_AMERGE("%s:exit!\n", __FUNCTION__);
}

void AmergeGetEnvLvV10(AmergeContext_t* pAmergeCtx, AecPreResult_t AecHdrPreResult) {
    LOG1_AMERGE("%s:enter!\n", __FUNCTION__);

    // transfer AeResult data into AhdrHandle
    switch (pAmergeCtx->FrameNumber) {
        case LINEAR_NUM:
            pAmergeCtx->NextData.CtrlData.ExpoData.EnvLv = AecHdrPreResult.GlobalEnvLv[0];
            break;
        case HDR_2X_NUM:
            pAmergeCtx->NextData.CtrlData.ExpoData.EnvLv = AecHdrPreResult.GlobalEnvLv[1];
            break;
        case HDR_3X_NUM:
            pAmergeCtx->NextData.CtrlData.ExpoData.EnvLv = AecHdrPreResult.GlobalEnvLv[1];
            break;
        default:
            LOGE_AMERGE("%s(%d):  Wrong frame number in HDR mode!!!\n", __FUNCTION__, __LINE__);
            break;
    }

    // Normalize the current envLv for AEC
    pAmergeCtx->NextData.CtrlData.ExpoData.EnvLv =
        (pAmergeCtx->NextData.CtrlData.ExpoData.EnvLv - MIN_ENV_LV) / (MAX_ENV_LV - MIN_ENV_LV);
    pAmergeCtx->NextData.CtrlData.ExpoData.EnvLv =
        LIMIT_VALUE(pAmergeCtx->NextData.CtrlData.ExpoData.EnvLv, ENVLVMAX, ENVLVMIN);

    LOG1_AMERGE("%s:exit!\n", __FUNCTION__);
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
void AmergeGetTuningProcResV10(AmergeContext_t* pAmergeCtx) {
    LOG1_AMERGE("%s:Enter!\n", __FUNCTION__);

    pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_mode =
        pAmergeCtx->NextData.HandleData.Merge_v10.MergeMode;
    if (CHECK_ISP_HW_V21())
        pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_mode =
            LIMIT_VALUE(pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_mode, 1, 0);
    pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_lm_dif_0p9 = SW_HDRMGE_LM_DIF_0P9_FIX;
    pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_ms_dif_0p8 = SW_HDRMGE_MS_DIF_0P8_FIX;
    pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_lm_dif_0p15 =
        (int)(pAmergeCtx->NextData.HandleData.Merge_v10.MDCurveLM_offset * MDCURVEOFFSETMAX);
    pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_ms_dif_0p15 =
        (int)(pAmergeCtx->NextData.HandleData.Merge_v10.MDCurveMS_offset * MDCURVEOFFSETMAX);

    if (pAmergeCtx->NextData.CtrlData.ExpoData.LongFrmMode) {
        for (int i = 0; i < HDRMGE_V10_OE_CURVE_NUM; i++)
            pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_e_y[i] = HDR_LONG_FRMAE_MODE_OECURVE;
    } else {
        CalibrateOECurveV10(pAmergeCtx->NextData.HandleData.Merge_v10.OECurve_smooth,
                            pAmergeCtx->NextData.HandleData.Merge_v10.OECurve_offset,
                            pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_e_y);
        CalibrateMDCurveV10(pAmergeCtx->NextData.HandleData.Merge_v10.MDCurveLM_smooth,
                            pAmergeCtx->NextData.HandleData.Merge_v10.MDCurveLM_offset,
                            pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_l1_y);
        CalibrateMDCurveV10(pAmergeCtx->NextData.HandleData.Merge_v10.MDCurveMS_smooth,
                            pAmergeCtx->NextData.HandleData.Merge_v10.MDCurveMS_offset,
                            pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_l0_y);
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
            (1 - pAmergeCtx->NextData.CtrlData.MergeOEDamp) *
                pAmergeCtx->CurrData.HandleData.Merge_v10.OECurve_smooth;
        pAmergeCtx->NextData.HandleData.Merge_v10.OECurve_offset =
            pAmergeCtx->NextData.CtrlData.MergeOEDamp *
                pAmergeCtx->NextData.HandleData.Merge_v10.OECurve_offset +
            (1 - pAmergeCtx->NextData.CtrlData.MergeOEDamp) *
                pAmergeCtx->CurrData.HandleData.Merge_v10.OECurve_offset;
        pAmergeCtx->NextData.HandleData.Merge_v10.MDCurveLM_smooth =
            pAmergeCtx->NextData.CtrlData.MergeMDDampLM *
                pAmergeCtx->NextData.HandleData.Merge_v10.MDCurveLM_smooth +
            (1 - pAmergeCtx->NextData.CtrlData.MergeMDDampLM) *
                pAmergeCtx->CurrData.HandleData.Merge_v10.MDCurveLM_smooth;
        pAmergeCtx->NextData.HandleData.Merge_v10.MDCurveLM_offset =
            pAmergeCtx->NextData.CtrlData.MergeMDDampLM *
                pAmergeCtx->NextData.HandleData.Merge_v10.MDCurveLM_offset +
            (1 - pAmergeCtx->NextData.CtrlData.MergeMDDampLM) *
                pAmergeCtx->CurrData.HandleData.Merge_v10.MDCurveLM_offset;
        pAmergeCtx->NextData.HandleData.Merge_v10.MDCurveMS_smooth =
            pAmergeCtx->NextData.CtrlData.MergeMDDampMS *
                pAmergeCtx->NextData.HandleData.Merge_v10.MDCurveMS_smooth +
            (1 - pAmergeCtx->NextData.CtrlData.MergeMDDampMS) *
                pAmergeCtx->CurrData.HandleData.Merge_v10.MDCurveMS_smooth;
        pAmergeCtx->NextData.HandleData.Merge_v10.MDCurveMS_offset =
            pAmergeCtx->NextData.CtrlData.MergeMDDampMS *
                pAmergeCtx->NextData.HandleData.Merge_v10.MDCurveMS_offset +
            (1 - pAmergeCtx->NextData.CtrlData.MergeMDDampMS) *
                pAmergeCtx->CurrData.HandleData.Merge_v10.MDCurveMS_offset;
    }

    LOG1_AMERGE("%s:Eixt!\n", __FUNCTION__);
}

/******************************************************************************
 * AmergeTuningProcessing()
 *get handle para by config and current variate
 *****************************************************************************/
void AmergeTuningProcessing(AmergeContext_t* pAmergeCtx) {
    LOG1_AMERGE("%s:enter!\n", __FUNCTION__);

    if (pAmergeCtx->mergeAttrV10.opMode == MERGE_OPMODE_AUTO) {
        float CtrlValue = pAmergeCtx->NextData.CtrlData.ExpoData.EnvLv;
        if (pAmergeCtx->mergeAttrV10.stAuto.MergeTuningPara.CtrlDataType == CTRLDATATYPE_ISO)
            CtrlValue = pAmergeCtx->NextData.CtrlData.ExpoData.ISO;

        // get Current merge OECurve
        pAmergeCtx->NextData.HandleData.Merge_v10.OECurve_smooth = GetCurrParaV10(
            CtrlValue, pAmergeCtx->mergeAttrV10.stAuto.MergeTuningPara.OECurve.CtrlData,
            pAmergeCtx->mergeAttrV10.stAuto.MergeTuningPara.OECurve.Smooth, MERGE_ENVLV_STEP_MAX);
        pAmergeCtx->NextData.HandleData.Merge_v10.OECurve_offset = GetCurrParaV10(
            CtrlValue, pAmergeCtx->mergeAttrV10.stAuto.MergeTuningPara.OECurve.CtrlData,
            pAmergeCtx->mergeAttrV10.stAuto.MergeTuningPara.OECurve.Offset, MERGE_ENVLV_STEP_MAX);

        // get Current merge MDCurve
        pAmergeCtx->NextData.HandleData.Merge_v10.MDCurveLM_smooth =
            GetCurrParaV10(pAmergeCtx->NextData.CtrlData.MoveCoef,
                           pAmergeCtx->mergeAttrV10.stAuto.MergeTuningPara.MDCurve.MoveCoef,
                           pAmergeCtx->mergeAttrV10.stAuto.MergeTuningPara.MDCurve.LM_smooth,
                           MERGE_ENVLV_STEP_MAX);
        pAmergeCtx->NextData.HandleData.Merge_v10.MDCurveLM_offset =
            GetCurrParaV10(pAmergeCtx->NextData.CtrlData.MoveCoef,
                           pAmergeCtx->mergeAttrV10.stAuto.MergeTuningPara.MDCurve.MoveCoef,
                           pAmergeCtx->mergeAttrV10.stAuto.MergeTuningPara.MDCurve.LM_offset,
                           MERGE_ENVLV_STEP_MAX);
        pAmergeCtx->NextData.HandleData.Merge_v10.MDCurveMS_smooth =
            GetCurrParaV10(pAmergeCtx->NextData.CtrlData.MoveCoef,
                           pAmergeCtx->mergeAttrV10.stAuto.MergeTuningPara.MDCurve.MoveCoef,
                           pAmergeCtx->mergeAttrV10.stAuto.MergeTuningPara.MDCurve.MS_smooth,
                           MERGE_ENVLV_STEP_MAX);
        pAmergeCtx->NextData.HandleData.Merge_v10.MDCurveMS_offset =
            GetCurrParaV10(pAmergeCtx->NextData.CtrlData.MoveCoef,
                           pAmergeCtx->mergeAttrV10.stAuto.MergeTuningPara.MDCurve.MoveCoef,
                           pAmergeCtx->mergeAttrV10.stAuto.MergeTuningPara.MDCurve.MS_offset,
                           MERGE_ENVLV_STEP_MAX);

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
    AmergeGetTuningProcResV10(pAmergeCtx);

    // transfer control data to api
    pAmergeCtx->mergeAttrV10.Info.Envlv    = pAmergeCtx->NextData.CtrlData.ExpoData.EnvLv;
    pAmergeCtx->mergeAttrV10.Info.ISO      = pAmergeCtx->NextData.CtrlData.ExpoData.ISO;
    pAmergeCtx->mergeAttrV10.Info.MoveCoef = pAmergeCtx->NextData.CtrlData.MoveCoef;

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
void AmergeExpoProcessing(AmergeContext_t* pAmergeCtx, MergeExpoData_t* pExpoData) {
    LOG1_AMERGE("%s:enter!\n", __FUNCTION__);

    // get sw_hdrmge_gain0
    pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_gain0 = (int)(64.0f * pExpoData->RatioLS);
    if (pExpoData->RatioLS == 1.0f)
        pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_gain0_inv =
            (int)(4096.0f * (1.0f / pExpoData->RatioLS) - 1.0f);
    else
        pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_gain0_inv =
            (int)(4096.0f * (1.0f / pExpoData->RatioLS));

    // get sw_hdrmge_gain1
    pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_gain1     = SW_HDRMGE_GAIN_FIX;
    pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_gain1_inv = SW_HDRMGE_GAIN_INV_FIX;

    // get sw_hdrmge_gain2
    pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_gain2 = SW_HDRMGE_GAIN_FIX;

    // store next data
    pAmergeCtx->CurrData.CtrlData.ExpoData.LongFrmMode =
        pAmergeCtx->NextData.CtrlData.ExpoData.LongFrmMode;
    pAmergeCtx->CurrData.CtrlData.ExpoData.RatioLS = pAmergeCtx->NextData.CtrlData.ExpoData.RatioLS;
    pAmergeCtx->CurrData.CtrlData.ExpoData.RatioLM = pAmergeCtx->NextData.CtrlData.ExpoData.RatioLM;

    LOGV_AMERGE(
        "%s: sw_hdrmge_mode:%d sw_hdrmge_ms_dif_0p8:%d sw_hdrmge_lm_dif_0p9:%d "
        "sw_hdrmge_ms_dif_0p15:%d sw_hdrmge_lm_dif_0p15:%d\n",
        __FUNCTION__, pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_mode,
        pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_ms_dif_0p8,
        pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_lm_dif_0p9,
        pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_ms_dif_0p15,
        pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_lm_dif_0p15);
    LOGV_AMERGE(
        "%s: RatioLS:%f sw_hdrmge_gain0:%d sw_hdrmge_gain0_inv:%d RatioLM:%f "
        "sw_hdrmge_gain1:%d sw_hdrmge_gain1_inv:%d sw_hdrmge_gain2:%d\n",
        __FUNCTION__, pExpoData->RatioLS, pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_gain0,
        pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_gain0_inv, pExpoData->RatioLM,
        pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_gain1,
        pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_gain1_inv,
        pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_gain2);
    LOGV_AMERGE("%s: sw_hdrmge_e_y: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
                __FUNCTION__, pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_e_y[0],
                pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_e_y[1],
                pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_e_y[2],
                pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_e_y[3],
                pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_e_y[4],
                pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_e_y[5],
                pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_e_y[6],
                pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_e_y[7],
                pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_e_y[8],
                pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_e_y[9],
                pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_e_y[10],
                pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_e_y[11],
                pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_e_y[12],
                pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_e_y[13],
                pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_e_y[14],
                pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_e_y[15],
                pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_e_y[16]);
    LOGV_AMERGE("%s: sw_hdrmge_l0_y: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
                __FUNCTION__, pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_l0_y[0],
                pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_l0_y[1],
                pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_l0_y[2],
                pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_l0_y[3],
                pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_l0_y[4],
                pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_l0_y[5],
                pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_l0_y[6],
                pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_l0_y[7],
                pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_l0_y[8],
                pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_l0_y[9],
                pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_l0_y[10],
                pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_l0_y[11],
                pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_l0_y[12],
                pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_l0_y[13],
                pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_l0_y[14],
                pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_l0_y[15],
                pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_l0_y[16]);
    LOGV_AMERGE("%s: sw_hdrmge_l1_y: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
                __FUNCTION__, pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_l1_y[0],
                pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_l1_y[1],
                pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_l1_y[2],
                pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_l1_y[3],
                pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_l1_y[4],
                pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_l1_y[5],
                pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_l1_y[6],
                pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_l1_y[7],
                pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_l1_y[8],
                pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_l1_y[9],
                pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_l1_y[10],
                pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_l1_y[11],
                pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_l1_y[12],
                pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_l1_y[13],
                pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_l1_y[14],
                pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_l1_y[15],
                pAmergeCtx->ProcRes.Merge_v10.sw_hdrmge_l1_y[16]);

    LOG1_AMERGE("%s:exit!\n", __FUNCTION__);
}

/******************************************************************************
 * AmergeByPassProcessing()
 *get handle para by config and current variate
 *****************************************************************************/
bool AmergeByPassProcessing(AmergeContext_t* pAmergeCtx, AecPreResult_t AecHdrPreResult) {
    LOG1_AMERGE("%s:enter!\n", __FUNCTION__);

    bool bypass = false;
    float diff  = 0.0f;

    if (pAmergeCtx->FrameID <= 2)
        bypass = false;
    else if (pAmergeCtx->mergeAttrV10.opMode != pAmergeCtx->CurrData.CtrlData.ApiMode)
        bypass = false;
    else if (pAmergeCtx->mergeAttrV10.opMode == MERGE_OPMODE_MANUAL)
        bypass = !pAmergeCtx->ifReCalcStManual;
    else if (pAmergeCtx->mergeAttrV10.opMode == MERGE_OPMODE_AUTO) {
        // get Current hdr mode
        pAmergeCtx->NextData.HandleData.Merge_v10.MergeMode = pAmergeCtx->FrameNumber - 1;
        LOG1_AMERGE("%s:  Current MergeMode: %d \n", __FUNCTION__,
                    pAmergeCtx->NextData.HandleData.Merge_v10.MergeMode);

        // get envlv from AecPreRes
        AmergeGetEnvLvV10(pAmergeCtx, AecHdrPreResult);
        pAmergeCtx->NextData.CtrlData.ExpoData.EnvLv =
            LIMIT_VALUE(pAmergeCtx->NextData.CtrlData.ExpoData.EnvLv, ENVLVMAX, ENVLVMIN);

        pAmergeCtx->NextData.CtrlData.MoveCoef = MOVE_COEF_DEFAULT;
        pAmergeCtx->NextData.CtrlData.MoveCoef =
            LIMIT_VALUE(pAmergeCtx->NextData.CtrlData.MoveCoef, MOVECOEFMAX, MOVECOEFMIN);

        if (pAmergeCtx->mergeAttrV10.stAuto.MergeTuningPara.CtrlDataType == CTRLDATATYPE_ENVLV) {
            diff = pAmergeCtx->CurrData.CtrlData.ExpoData.EnvLv -
                   pAmergeCtx->NextData.CtrlData.ExpoData.EnvLv;
            if (pAmergeCtx->CurrData.CtrlData.ExpoData.EnvLv == 0.0) {
                diff = pAmergeCtx->NextData.CtrlData.ExpoData.EnvLv;
                if (diff == 0.0)
                    bypass = true;
                else
                    bypass = false;
            } else {
                diff /= pAmergeCtx->CurrData.CtrlData.ExpoData.EnvLv;
                if (diff >= pAmergeCtx->mergeAttrV10.stAuto.MergeTuningPara.ByPassThr ||
                    diff <= (0 - pAmergeCtx->mergeAttrV10.stAuto.MergeTuningPara.ByPassThr))
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
                diff <= (0 - pAmergeCtx->mergeAttrV10.stAuto.MergeTuningPara.ByPassThr))
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

    memset(pAmergeCtx, 0x00, sizeof(AmergeContext_t));
    *ppAmergeCtx      = pAmergeCtx;
    pAmergeCtx->state = AMERGE_STATE_INITIALIZED;

    CalibDbV2_merge_v10_t* calibv2_amerge_calib =
        (CalibDbV2_merge_v10_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibV2, amerge_calib));
    memcpy(&pAmergeCtx->mergeAttrV10.stAuto, calibv2_amerge_calib,
           sizeof(CalibDbV2_merge_v10_t));  // set deafault stAuto

    // set default para
    // config default CurrData data
    pAmergeCtx->NextData.CtrlData.ExpoData.EnvLv               = 0;
    pAmergeCtx->CurrData.CtrlData.MoveCoef                     = 0;
    pAmergeCtx->CurrData.CtrlData.ApiMode                      = MERGE_OPMODE_AUTO;
    pAmergeCtx->CurrData.HandleData.Merge_v10.MergeMode        = 1;
    pAmergeCtx->CurrData.HandleData.Merge_v10.OECurve_smooth   = 0.4;
    pAmergeCtx->CurrData.HandleData.Merge_v10.OECurve_offset   = 210;
    pAmergeCtx->CurrData.HandleData.Merge_v10.MDCurveLM_smooth = 0.4;
    pAmergeCtx->CurrData.HandleData.Merge_v10.MDCurveLM_offset = 0.38;
    pAmergeCtx->CurrData.HandleData.Merge_v10.MDCurveMS_smooth = 0.4;
    pAmergeCtx->CurrData.HandleData.Merge_v10.MDCurveMS_offset = 0.38;

    // set default ctrl info
    pAmergeCtx->mergeAttrV10.opMode           = MERGE_OPMODE_AUTO;
    pAmergeCtx->mergeAttrV10.Info.Envlv       = 1.0;
    pAmergeCtx->mergeAttrV10.Info.MoveCoef    = 0.0;

    pAmergeCtx->mergeAttrV10.stManual.OECurve.Smooth    = 0.4;
    pAmergeCtx->mergeAttrV10.stManual.OECurve.Offset    = 210;
    pAmergeCtx->mergeAttrV10.stManual.MDCurve.LM_smooth = 0.4;
    pAmergeCtx->mergeAttrV10.stManual.MDCurve.LM_offset = 0.38;
    pAmergeCtx->mergeAttrV10.stManual.MDCurve.MS_smooth = 0.4;
    pAmergeCtx->mergeAttrV10.stManual.MDCurve.MS_offset = 0.38;

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
