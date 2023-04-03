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
 * @file rk_aiq_amerge_algo_v12.cpp
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
 * GetCurrPara()
 *****************************************************************************/
float GetCurrPara(float inPara, float* inMatrixX, float* inMatrixY, int Max_Knots) {
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

void AmergeGetEnvLv(AmergeContext_t* pAmergeCtx, AecPreResult_t AecHdrPreResult) {
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
            LOGE_AMERGE("%s(%d): Wrong frame number in HDR mode!!!\n", __FUNCTION__, __LINE__);
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
 * CalibrateOECurve()
 *****************************************************************************/
void CalibrateOECurve(float smooth, float offset, unsigned short* OECurve) {
    LOG1_AMERGE("%s:Enter!\n", __FUNCTION__);

    int step    = 32;
    float curve = 0.0f;
    float k     = 511.0f;

    for (int i = 0; i < HDRMGE_V12_OE_CURVE_NUM; ++i) {
        curve      = 1 + exp(-smooth * OECURVESMOOTHMAX * (k / 1023.0f - offset / 256.0f));
        curve      = 1024.0f / curve;
        OECurve[i] = round(curve);
        OECurve[i] = MIN(OECurve[i], 1023);
        k += step;
    }

    LOG1_AMERGE("%s:Eixt!\n", __FUNCTION__);
}

/******************************************************************************
 * CalibrateEachChnCurve()
 *****************************************************************************/
void CalibrateEachChnCurve(float smooth, float offset, unsigned short* OECurve) {
    LOG1_AMERGE("%s:Enter!\n", __FUNCTION__);

    int step    = 64;
    float curve = 0.0f;
    float k     = 0.0f;

    for (int i = 0; i < HDRMGE_V12_OE_CURVE_NUM; ++i) {
        curve      = 1 + exp(-smooth * EACHOECURVESMOOTHMAX * (k / 1023.0f - offset));
        curve      = 1024.0f / curve;
        OECurve[i] = round(curve);
        OECurve[i] = MIN(OECurve[i], 1023);
        k += step;
    }

    LOG1_AMERGE("%s:Eixt!\n", __FUNCTION__);
}

/******************************************************************************
 * CalibrateMDCurveLongFrmMode()
 *****************************************************************************/
void CalibrateMDCurveLongFrmMode(float smooth, float offset, unsigned short* MDCurve) {
    LOG1_AMERGE("%s:Enter!\n", __FUNCTION__);

    int step    = 16;
    float curve = 0.0f;
    float k     = 0.0f;

    for (int i = 0; i < HDRMGE_V12_MD_CURVE_NUM; ++i) {
        curve =
            1 + exp(-smooth * MDCURVESMOOTHMAX * (k / 1023.0f - offset * MDCURVEOFFSETMAX / 256.0f));
        curve      = 1024.0f / curve;
        MDCurve[i] = round(curve);
        MDCurve[i] = MIN(MDCurve[i], 1023);
        k += step;
    }

    LOG1_AMERGE("%s:Eixt!\n", __FUNCTION__);
}
/******************************************************************************
 * CalibrateMDCurveShortFrmMode()
 *****************************************************************************/
void CalibrateMDCurveShortFrmMode(float smooth, float offset, unsigned short* MDCurveLM,
                                  unsigned short* MDCurveMS) {
    LOG1_AMERGE("%s:Enter!\n", __FUNCTION__);

    float step  = 1.0f / 16.0f;
    float curve = 0.0f;

    for (int i = 0; i < HDRMGE_V12_MD_CURVE_NUM; ++i) {
        curve        = 0.01f + pow(i * step, 2.0f);
        curve        = 1024.0f * pow(i * step, 2.0f) / curve;
        MDCurveLM[i] = round(curve);
        MDCurveLM[i] = MIN(MDCurveLM[i], 1023);
        MDCurveMS[i] = MDCurveLM[i];
    }

    LOG1_AMERGE("%s:Eixt!\n", __FUNCTION__);
}
/******************************************************************************
 * AmergeGetTuningProcResV12()
 *****************************************************************************/
void AmergeGetTuningProcResV12(AmergeContext_t* pAmergeCtx) {
    LOG1_AMERGE("%s:Enter!\n", __FUNCTION__);

    pAmergeCtx->ProcRes.Merge_v12.mode   = pAmergeCtx->NextData.HandleData.Merge_v12.MergeMode;
    pAmergeCtx->ProcRes.Merge_v12.s_base = pAmergeCtx->NextData.HandleData.Merge_v12.BaseFrm;
    pAmergeCtx->ProcRes.Merge_v12.each_raw_en =
        pAmergeCtx->NextData.HandleData.Merge_v12.EnableEachChn;
    pAmergeCtx->ProcRes.Merge_v12.lm_dif_0p9 = SW_HDRMGE_LM_DIF_0P9_FIX;
    pAmergeCtx->ProcRes.Merge_v12.ms_dif_0p8 = SW_HDRMGE_MS_DIF_0P8_FIX;
    pAmergeCtx->ProcRes.Merge_v12.lm_dif_0p15 =
        (int)(pAmergeCtx->NextData.HandleData.Merge_v12.MDCurveLM_offset * MDCURVEOFFSETMAX);
    pAmergeCtx->ProcRes.Merge_v12.ms_dif_0p15 =
        (int)(pAmergeCtx->NextData.HandleData.Merge_v12.MDCurveMS_offset * MDCURVEOFFSETMAX);

    if (pAmergeCtx->NextData.CtrlData.ExpoData.LongFrmMode) {
        for (int i = 0; i < HDRMGE_V12_OE_CURVE_NUM; i++)
            pAmergeCtx->ProcRes.Merge_v12.e_y[i] = HDR_LONG_FRMAE_MODE_OECURVE;
    } else {
        CalibrateOECurve(pAmergeCtx->NextData.HandleData.Merge_v12.OECurve_smooth,
                         pAmergeCtx->NextData.HandleData.Merge_v12.OECurve_offset,
                         pAmergeCtx->ProcRes.Merge_v12.e_y);
    }
    if (pAmergeCtx->NextData.HandleData.Merge_v12.BaseFrm == BASEFRAME_LONG) {
        CalibrateMDCurveLongFrmMode(pAmergeCtx->NextData.HandleData.Merge_v12.MDCurveLM_smooth,
                                    pAmergeCtx->NextData.HandleData.Merge_v12.MDCurveLM_offset,
                                    pAmergeCtx->ProcRes.Merge_v12.l1_y);
        CalibrateMDCurveLongFrmMode(pAmergeCtx->NextData.HandleData.Merge_v12.MDCurveMS_smooth,
                                    pAmergeCtx->NextData.HandleData.Merge_v12.MDCurveMS_offset,
                                    pAmergeCtx->ProcRes.Merge_v12.l0_y);
        // merge v12 add
        if (pAmergeCtx->ProcRes.Merge_v12.each_raw_en) {
            CalibrateEachChnCurve(pAmergeCtx->NextData.HandleData.Merge_v12.EachChnCurve_smooth,
                                  pAmergeCtx->NextData.HandleData.Merge_v12.EachChnCurve_offset,
                                  pAmergeCtx->ProcRes.Merge_v12.l_raw0);
            CalibrateEachChnCurve(pAmergeCtx->NextData.HandleData.Merge_v12.EachChnCurve_smooth,
                                  pAmergeCtx->NextData.HandleData.Merge_v12.EachChnCurve_offset,
                                  pAmergeCtx->ProcRes.Merge_v12.l_raw1);
        }
    } else if (pAmergeCtx->NextData.HandleData.Merge_v12.BaseFrm == BASEFRAME_SHORT) {
        CalibrateMDCurveShortFrmMode(pAmergeCtx->NextData.HandleData.Merge_v12.MDCurveLM_smooth,
                                     pAmergeCtx->NextData.HandleData.Merge_v12.MDCurveLM_offset,
                                     pAmergeCtx->ProcRes.Merge_v12.l1_y,
                                     pAmergeCtx->ProcRes.Merge_v12.l0_y);
    }

    LOG1_AMERGE("%s:Eixt!\n", __FUNCTION__);
}

/******************************************************************************
 * MergeDampingV12()
 *****************************************************************************/
void MergeDampingV12(AmergeContext_t* pAmergeCtx) {
    LOG1_AMERGE("%s:Enter!\n", __FUNCTION__);

    if (pAmergeCtx->FrameID && (pAmergeCtx->NextData.HandleData.Merge_v12.MergeMode ==
                                pAmergeCtx->CurrData.HandleData.Merge_v12.MergeMode)) {
        pAmergeCtx->NextData.HandleData.Merge_v12.OECurve_smooth =
            pAmergeCtx->NextData.CtrlData.MergeOEDamp *
                pAmergeCtx->NextData.HandleData.Merge_v12.OECurve_smooth +
            (1 - pAmergeCtx->NextData.CtrlData.MergeOEDamp) *
                pAmergeCtx->CurrData.HandleData.Merge_v12.OECurve_smooth;
        pAmergeCtx->NextData.HandleData.Merge_v12.OECurve_offset =
            pAmergeCtx->NextData.CtrlData.MergeOEDamp *
                pAmergeCtx->NextData.HandleData.Merge_v12.OECurve_offset +
            (1 - pAmergeCtx->NextData.CtrlData.MergeOEDamp) *
                pAmergeCtx->CurrData.HandleData.Merge_v12.OECurve_offset;
        pAmergeCtx->NextData.HandleData.Merge_v12.MDCurveLM_smooth =
            pAmergeCtx->NextData.CtrlData.MergeMDDampLM *
                pAmergeCtx->NextData.HandleData.Merge_v12.MDCurveLM_smooth +
            (1 - pAmergeCtx->NextData.CtrlData.MergeMDDampLM) *
                pAmergeCtx->CurrData.HandleData.Merge_v12.MDCurveLM_smooth;
        pAmergeCtx->NextData.HandleData.Merge_v12.MDCurveLM_offset =
            pAmergeCtx->NextData.CtrlData.MergeMDDampLM *
                pAmergeCtx->NextData.HandleData.Merge_v12.MDCurveLM_offset +
            (1 - pAmergeCtx->NextData.CtrlData.MergeMDDampLM) *
                pAmergeCtx->CurrData.HandleData.Merge_v12.MDCurveLM_offset;
        pAmergeCtx->NextData.HandleData.Merge_v12.MDCurveMS_smooth =
            pAmergeCtx->NextData.CtrlData.MergeMDDampMS *
                pAmergeCtx->NextData.HandleData.Merge_v12.MDCurveMS_smooth +
            (1 - pAmergeCtx->NextData.CtrlData.MergeMDDampMS) *
                pAmergeCtx->CurrData.HandleData.Merge_v12.MDCurveMS_smooth;
        pAmergeCtx->NextData.HandleData.Merge_v12.MDCurveMS_offset =
            pAmergeCtx->NextData.CtrlData.MergeMDDampMS *
                pAmergeCtx->NextData.HandleData.Merge_v12.MDCurveMS_offset +
            (1 - pAmergeCtx->NextData.CtrlData.MergeMDDampMS) *
                pAmergeCtx->CurrData.HandleData.Merge_v12.MDCurveMS_offset;

        pAmergeCtx->NextData.HandleData.Merge_v12.MDCurve_Coef =
            pAmergeCtx->NextData.CtrlData.MergeMDDampMS *
                pAmergeCtx->NextData.HandleData.Merge_v12.MDCurve_Coef +
            (1 - pAmergeCtx->NextData.CtrlData.MergeMDDampMS) *
                pAmergeCtx->CurrData.HandleData.Merge_v12.MDCurve_Coef;
        pAmergeCtx->NextData.HandleData.Merge_v12.MDCurve_ms_thd0 =
            pAmergeCtx->NextData.CtrlData.MergeMDDampMS *
                pAmergeCtx->NextData.HandleData.Merge_v12.MDCurve_ms_thd0 +
            (1 - pAmergeCtx->NextData.CtrlData.MergeMDDampMS) *
                pAmergeCtx->CurrData.HandleData.Merge_v12.MDCurve_ms_thd0;
        pAmergeCtx->NextData.HandleData.Merge_v12.MDCurve_lm_thd0 =
            pAmergeCtx->NextData.CtrlData.MergeMDDampMS *
                pAmergeCtx->NextData.HandleData.Merge_v12.MDCurve_lm_thd0 +
            (1 - pAmergeCtx->NextData.CtrlData.MergeMDDampMS) *
                pAmergeCtx->CurrData.HandleData.Merge_v12.MDCurve_lm_thd0;
    }

    LOG1_AMERGE("%s:Eixt!\n", __FUNCTION__);
}

/******************************************************************************
 * AmergeTuningProcessing()
 *get handle para by config and current variate
 *****************************************************************************/
void AmergeTuningProcessing(AmergeContext_t* pAmergeCtx) {
    LOG1_AMERGE("%s:enter!\n", __FUNCTION__);

    if (pAmergeCtx->mergeAttrV12.opMode == MERGE_OPMODE_AUTO) {
        float CtrlValue = pAmergeCtx->NextData.CtrlData.ExpoData.EnvLv;
        if (pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.CtrlDataType == CTRLDATATYPE_ISO)
            CtrlValue = pAmergeCtx->NextData.CtrlData.ExpoData.ISO;

        pAmergeCtx->NextData.HandleData.Merge_v12.BaseFrm =
            pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.BaseFrm;
        if (pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.BaseFrm == BASEFRAME_LONG) {
            // get Current merge OECurve
            pAmergeCtx->NextData.HandleData.Merge_v12.OECurve_smooth = GetCurrPara(
                CtrlValue,
                pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.LongFrmModeData.OECurve.CtrlData,
                pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.LongFrmModeData.OECurve.Smooth,
                MERGE_ENVLV_STEP_MAX);
            pAmergeCtx->NextData.HandleData.Merge_v12.OECurve_offset = GetCurrPara(
                CtrlValue,
                pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.LongFrmModeData.OECurve.CtrlData,
                pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.LongFrmModeData.OECurve.Offset,
                MERGE_ENVLV_STEP_MAX);

            // get Current merge MDCurve
            pAmergeCtx->NextData.HandleData.Merge_v12.MDCurveLM_smooth = GetCurrPara(
                pAmergeCtx->NextData.CtrlData.MoveCoef,
                pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.LongFrmModeData.MDCurve.MoveCoef,
                pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.LongFrmModeData.MDCurve.LM_smooth,
                MERGE_ENVLV_STEP_MAX);
            pAmergeCtx->NextData.HandleData.Merge_v12.MDCurveLM_offset = GetCurrPara(
                pAmergeCtx->NextData.CtrlData.MoveCoef,
                pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.LongFrmModeData.MDCurve.MoveCoef,
                pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.LongFrmModeData.MDCurve.LM_offset,
                MERGE_ENVLV_STEP_MAX);
            pAmergeCtx->NextData.HandleData.Merge_v12.MDCurveMS_smooth = GetCurrPara(
                pAmergeCtx->NextData.CtrlData.MoveCoef,
                pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.LongFrmModeData.MDCurve.MoveCoef,
                pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.LongFrmModeData.MDCurve.MS_smooth,
                MERGE_ENVLV_STEP_MAX);
            pAmergeCtx->NextData.HandleData.Merge_v12.MDCurveMS_offset = GetCurrPara(
                pAmergeCtx->NextData.CtrlData.MoveCoef,
                pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.LongFrmModeData.MDCurve.MoveCoef,
                pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.LongFrmModeData.MDCurve.MS_offset,
                MERGE_ENVLV_STEP_MAX);

            pAmergeCtx->NextData.CtrlData.MergeOEDamp =
                pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.LongFrmModeData.OECurve_damp;
            pAmergeCtx->NextData.CtrlData.MergeMDDampLM =
                pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.LongFrmModeData.MDCurveLM_damp;
            pAmergeCtx->NextData.CtrlData.MergeMDDampMS =
                pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.LongFrmModeData.MDCurveMS_damp;

            // merge v12 add, get Current merge each channel
            pAmergeCtx->NextData.HandleData.Merge_v12.EnableEachChn =
                pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.LongFrmModeData.EnableEachChn;
            if (pAmergeCtx->NextData.HandleData.Merge_v12.EnableEachChn) {
                pAmergeCtx->NextData.HandleData.Merge_v12.EachChnCurve_smooth =
                    GetCurrPara(CtrlValue,
                                pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.LongFrmModeData
                                    .EachChnCurve.CtrlData,
                                pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.LongFrmModeData
                                    .EachChnCurve.Smooth,
                                MERGE_ENVLV_STEP_MAX);
                pAmergeCtx->NextData.HandleData.Merge_v12.EachChnCurve_offset =
                    GetCurrPara(CtrlValue,
                                pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.LongFrmModeData
                                    .EachChnCurve.CtrlData,
                                pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.LongFrmModeData
                                    .EachChnCurve.Offset,
                                MERGE_ENVLV_STEP_MAX);
            }
        } else if (pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.BaseFrm == BASEFRAME_SHORT) {
            // get Current merge OECurve
            pAmergeCtx->NextData.HandleData.Merge_v12.OECurve_smooth = GetCurrPara(
                CtrlValue,
                pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.ShortFrmModeData.OECurve.CtrlData,
                pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.ShortFrmModeData.OECurve.Smooth,
                MERGE_ENVLV_STEP_MAX);
            pAmergeCtx->NextData.HandleData.Merge_v12.OECurve_offset = GetCurrPara(
                CtrlValue,
                pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.ShortFrmModeData.OECurve.CtrlData,
                pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.ShortFrmModeData.OECurve.Offset,
                MERGE_ENVLV_STEP_MAX);

            // get Current merge MDCurve
            pAmergeCtx->NextData.HandleData.Merge_v12.MDCurve_Coef = GetCurrPara(
                pAmergeCtx->NextData.CtrlData.MoveCoef,
                pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.ShortFrmModeData.MDCurve.MoveCoef,
                pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.ShortFrmModeData.MDCurve.Coef,
                MERGE_ENVLV_STEP_MAX);
            pAmergeCtx->NextData.HandleData.Merge_v12.MDCurve_ms_thd0 = GetCurrPara(
                pAmergeCtx->NextData.CtrlData.MoveCoef,
                pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.ShortFrmModeData.MDCurve.MoveCoef,
                pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.ShortFrmModeData.MDCurve.ms_thd0,
                MERGE_ENVLV_STEP_MAX);
            pAmergeCtx->NextData.HandleData.Merge_v12.MDCurve_lm_thd0 = GetCurrPara(
                pAmergeCtx->NextData.CtrlData.MoveCoef,
                pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.ShortFrmModeData.MDCurve.MoveCoef,
                pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.ShortFrmModeData.MDCurve.lm_thd0,
                MERGE_ENVLV_STEP_MAX);

            pAmergeCtx->NextData.CtrlData.MergeOEDamp =
                pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.ShortFrmModeData.OECurve_damp;
            pAmergeCtx->NextData.CtrlData.MergeMDDampLM =
                pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.ShortFrmModeData.MDCurve_damp;
            pAmergeCtx->NextData.CtrlData.MergeMDDampMS =
                pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.ShortFrmModeData.MDCurve_damp;
        }
        // merge damp
        MergeDampingV12(pAmergeCtx);
    } else if (pAmergeCtx->mergeAttrV12.opMode == MERGE_OPMODE_MANUAL) {
        pAmergeCtx->NextData.HandleData.Merge_v12.BaseFrm =
            pAmergeCtx->mergeAttrV12.stManual.BaseFrm;
        if (pAmergeCtx->mergeAttrV12.stManual.BaseFrm == BASEFRAME_LONG) {
            // get Current merge OECurve
            pAmergeCtx->NextData.HandleData.Merge_v12.OECurve_smooth =
                pAmergeCtx->mergeAttrV12.stManual.LongFrmModeData.OECurve.Smooth;
            pAmergeCtx->NextData.HandleData.Merge_v12.OECurve_offset =
                pAmergeCtx->mergeAttrV12.stManual.LongFrmModeData.OECurve.Offset;

            // get Current merge MDCurve
            pAmergeCtx->NextData.HandleData.Merge_v12.MDCurveLM_smooth =
                pAmergeCtx->mergeAttrV12.stManual.LongFrmModeData.MDCurve.LM_smooth;
            pAmergeCtx->NextData.HandleData.Merge_v12.MDCurveLM_offset =
                pAmergeCtx->mergeAttrV12.stManual.LongFrmModeData.MDCurve.LM_offset;
            pAmergeCtx->NextData.HandleData.Merge_v12.MDCurveMS_smooth =
                pAmergeCtx->mergeAttrV12.stManual.LongFrmModeData.MDCurve.MS_smooth;
            pAmergeCtx->NextData.HandleData.Merge_v12.MDCurveMS_offset =
                pAmergeCtx->mergeAttrV12.stManual.LongFrmModeData.MDCurve.MS_offset;

            // get Current merge each channel
            pAmergeCtx->NextData.HandleData.Merge_v12.EnableEachChn =
                pAmergeCtx->mergeAttrV12.stManual.LongFrmModeData.EnableEachChn;
            if (pAmergeCtx->NextData.HandleData.Merge_v12.EnableEachChn) {
                pAmergeCtx->NextData.HandleData.Merge_v12.EachChnCurve_smooth =
                    pAmergeCtx->mergeAttrV12.stManual.LongFrmModeData.EachChnCurve.Smooth;
                pAmergeCtx->NextData.HandleData.Merge_v12.EachChnCurve_offset =
                    pAmergeCtx->mergeAttrV12.stManual.LongFrmModeData.EachChnCurve.Offset;
            }
        } else if (pAmergeCtx->mergeAttrV12.stManual.BaseFrm == BASEFRAME_SHORT) {
            // get Current merge OECurve
            pAmergeCtx->NextData.HandleData.Merge_v12.OECurve_smooth =
                pAmergeCtx->mergeAttrV12.stManual.ShortFrmModeData.OECurve.Smooth;
            pAmergeCtx->NextData.HandleData.Merge_v12.OECurve_offset =
                pAmergeCtx->mergeAttrV12.stManual.ShortFrmModeData.OECurve.Offset;

            // get Current merge MDCurve
            pAmergeCtx->NextData.HandleData.Merge_v12.MDCurve_Coef =
                pAmergeCtx->mergeAttrV12.stManual.ShortFrmModeData.MDCurve.Coef;
            pAmergeCtx->NextData.HandleData.Merge_v12.MDCurve_ms_thd0 =
                pAmergeCtx->mergeAttrV12.stManual.ShortFrmModeData.MDCurve.ms_thd0;
            pAmergeCtx->NextData.HandleData.Merge_v12.MDCurve_lm_thd0 =
                pAmergeCtx->mergeAttrV12.stManual.ShortFrmModeData.MDCurve.lm_thd0;
        }
    }

    LOGD_AMERGE("%s: Current BaseFrm:%d OECurve_smooth:%f OECurve_offset:%f \n", __FUNCTION__,
                pAmergeCtx->NextData.HandleData.Merge_v12.BaseFrm,
                pAmergeCtx->NextData.HandleData.Merge_v12.OECurve_smooth,
                pAmergeCtx->NextData.HandleData.Merge_v12.OECurve_offset);
    if (pAmergeCtx->NextData.HandleData.Merge_v12.BaseFrm == BASEFRAME_LONG) {
        LOGD_AMERGE(
            "%s: Current MDCurveMS_smooth:%f MDCurveMS_offset:%f MDCurveLM_smooth:%f "
            "MDCurveLM_offset:%f \n",
            __FUNCTION__, pAmergeCtx->NextData.HandleData.Merge_v12.MDCurveMS_smooth,
            pAmergeCtx->NextData.HandleData.Merge_v12.MDCurveMS_offset,
            pAmergeCtx->NextData.HandleData.Merge_v12.MDCurveLM_smooth,
            pAmergeCtx->NextData.HandleData.Merge_v12.MDCurveLM_offset);
        LOGD_AMERGE("%s: Current EnableEachChn:%d EachChnCurve_smooth:%f EachChnCurve_offset:%f\n",
                    __FUNCTION__, pAmergeCtx->NextData.HandleData.Merge_v12.EnableEachChn,
                    pAmergeCtx->NextData.HandleData.Merge_v12.EachChnCurve_smooth,
                    pAmergeCtx->NextData.HandleData.Merge_v12.EachChnCurve_offset);
    } else if (pAmergeCtx->NextData.HandleData.Merge_v12.BaseFrm == BASEFRAME_SHORT) {
        LOGD_AMERGE("%s: Current MDCurve_Coef:%f MDCurve_ms_thd0:%f MDCurve_lm_thd0:%f\n",
                    __FUNCTION__, pAmergeCtx->NextData.HandleData.Merge_v12.MDCurve_Coef,
                    pAmergeCtx->NextData.HandleData.Merge_v12.MDCurve_ms_thd0,
                    pAmergeCtx->NextData.HandleData.Merge_v12.MDCurve_lm_thd0);
    }

    // get current IO data
    AmergeGetTuningProcResV12(pAmergeCtx);

    // transfer data to api
    pAmergeCtx->mergeAttrV12.Info.Envlv    = pAmergeCtx->NextData.CtrlData.ExpoData.EnvLv;
    pAmergeCtx->mergeAttrV12.Info.ISO      = pAmergeCtx->NextData.CtrlData.ExpoData.ISO;
    pAmergeCtx->mergeAttrV12.Info.MoveCoef = pAmergeCtx->NextData.CtrlData.MoveCoef;

    // store current handle data to pre data for next loop
    pAmergeCtx->CurrData.CtrlData.ExpoData.EnvLv = pAmergeCtx->NextData.CtrlData.ExpoData.EnvLv;
    pAmergeCtx->CurrData.CtrlData.ExpoData.ISO   = pAmergeCtx->NextData.CtrlData.ExpoData.ISO;
    pAmergeCtx->CurrData.CtrlData.MoveCoef       = pAmergeCtx->NextData.CtrlData.MoveCoef;
    pAmergeCtx->CurrData.CtrlData.ApiMode        = pAmergeCtx->mergeAttrV12.opMode;
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
    pAmergeCtx->ProcRes.Merge_v12.gain0 = (int)(64.0f * pExpoData->RatioLS);
    if (pExpoData->RatioLS == 1.0f)
        pAmergeCtx->ProcRes.Merge_v12.gain0_inv =
            (int)(4096.0f * (1.0f / pExpoData->RatioLS) - 1.0f);
    else
        pAmergeCtx->ProcRes.Merge_v12.gain0_inv = (int)(4096.0f * (1.0f / pExpoData->RatioLS));

    // get sw_hdrmge_gain1
    pAmergeCtx->ProcRes.Merge_v12.gain1     = SW_HDRMGE_GAIN_FIX;
    pAmergeCtx->ProcRes.Merge_v12.gain1_inv = SW_HDRMGE_GAIN_INV_FIX;

    // get sw_hdrmge_gain2
    pAmergeCtx->ProcRes.Merge_v12.gain2 = SW_HDRMGE_GAIN_FIX;

    // merge v11 add
    if (pAmergeCtx->NextData.HandleData.Merge_v12.BaseFrm == BASEFRAME_SHORT) {
        float Coef = pAmergeCtx->NextData.HandleData.Merge_v12.MDCurve_Coef * SHORT_MODE_COEF_MAX;
        float sw_hdrmge_ms_thd0 = pAmergeCtx->NextData.HandleData.Merge_v12.MDCurve_ms_thd0;
        float sw_hdrmge_lm_thd0 = pAmergeCtx->NextData.HandleData.Merge_v12.MDCurve_lm_thd0;
        float sw_hdrmge_ms_thd1 = pow(100.0f * Coef * pExpoData->SGain, 0.5f);
        float sw_hdrmge_lm_thd1 = pow(100.0f * Coef * pExpoData->MGain, 0.5f);
        float sw_hdrmge_ms_scl  = (sw_hdrmge_ms_thd1 == sw_hdrmge_ms_thd0)
                                     ? 0.0f
                                     : (1.0f / (sw_hdrmge_ms_thd1 - sw_hdrmge_ms_thd0));
        float sw_hdrmge_lm_scl = (sw_hdrmge_lm_thd1 == sw_hdrmge_lm_thd0)
                                     ? 0.0f
                                     : (1.0f / (sw_hdrmge_lm_thd1 - sw_hdrmge_lm_thd0));
        pAmergeCtx->ProcRes.Merge_v12.ms_thd0 = (int)(1024.0f * sw_hdrmge_ms_thd0);
        pAmergeCtx->ProcRes.Merge_v12.ms_thd1 = (int)(1024.0f * sw_hdrmge_ms_thd1);
        pAmergeCtx->ProcRes.Merge_v12.ms_scl  = (int)(64.0f * sw_hdrmge_ms_scl);
        pAmergeCtx->ProcRes.Merge_v12.lm_thd0 = (int)(1024.0f * sw_hdrmge_lm_thd0);
        pAmergeCtx->ProcRes.Merge_v12.lm_thd1 = (int)(1024.0f * sw_hdrmge_lm_thd1);
        pAmergeCtx->ProcRes.Merge_v12.lm_scl  = (int)(64.0f * sw_hdrmge_lm_scl);
    }

    // merge v12 add
    if (pAmergeCtx->NextData.HandleData.Merge_v12.BaseFrm == BASEFRAME_LONG &&
        pAmergeCtx->ProcRes.Merge_v12.each_raw_en) {
        pAmergeCtx->ProcRes.Merge_v12.each_raw_gain0 =
            (int)(64.0f * pExpoData->RatioLS);  // ratio between middle and short
        pAmergeCtx->ProcRes.Merge_v12.each_raw_gain1 =
            SW_HDRMGE_GAIN_FIX;  // ratio between middle and long
    }

    // store next data
    pAmergeCtx->CurrData.CtrlData.ExpoData.LongFrmMode =
        pAmergeCtx->NextData.CtrlData.ExpoData.LongFrmMode;
    pAmergeCtx->CurrData.CtrlData.ExpoData.RatioLS = pAmergeCtx->NextData.CtrlData.ExpoData.RatioLS;
    pAmergeCtx->CurrData.CtrlData.ExpoData.RatioLM = pAmergeCtx->NextData.CtrlData.ExpoData.RatioLM;
    pAmergeCtx->CurrData.CtrlData.ExpoData.SGain   = pAmergeCtx->NextData.CtrlData.ExpoData.SGain;
    pAmergeCtx->CurrData.CtrlData.ExpoData.MGain   = pAmergeCtx->NextData.CtrlData.ExpoData.MGain;

    LOGV_AMERGE(
        "%s: RatioLS:%f sw_hdrmge_gain0:%d sw_hdrmge_gain0_inv:%d RatioLM:%f "
        "sw_hdrmge_gain1:%d sw_hdrmge_gain1_inv:%d sw_hdrmge_gain2:%d\n",
        __FUNCTION__, pExpoData->RatioLS, pAmergeCtx->ProcRes.Merge_v12.gain0,
        pAmergeCtx->ProcRes.Merge_v12.gain0_inv, pExpoData->RatioLM,
        pAmergeCtx->ProcRes.Merge_v12.gain1, pAmergeCtx->ProcRes.Merge_v12.gain1_inv,
        pAmergeCtx->ProcRes.Merge_v12.gain2);
    LOGV_AMERGE("%s: sw_hdrmge_mode:%d s_base:%d each_raw_en:%d\n", __FUNCTION__,
                pAmergeCtx->ProcRes.Merge_v12.mode, pAmergeCtx->ProcRes.Merge_v12.s_base,
                pAmergeCtx->ProcRes.Merge_v12.each_raw_en);
    if (!(pAmergeCtx->ProcRes.Merge_v12.s_base)) {
        LOGV_AMERGE(
            "%s: sw_hdrmge_ms_dif_0p8:%d sw_hdrmge_lm_dif_0p9:%d "
            "sw_hdrmge_ms_dif_0p15:%d sw_hdrmge_lm_dif_0p15:%d\n",
            __FUNCTION__, pAmergeCtx->ProcRes.Merge_v12.ms_dif_0p8,
            pAmergeCtx->ProcRes.Merge_v12.lm_dif_0p9, pAmergeCtx->ProcRes.Merge_v12.ms_dif_0p15,
            pAmergeCtx->ProcRes.Merge_v12.lm_dif_0p15);

        if (pAmergeCtx->ProcRes.Merge_v12.each_raw_en) {
            LOGV_AMERGE("%s: each_raw_gain0:%d each_raw_gain1:%d\n", __FUNCTION__,
                        pAmergeCtx->ProcRes.Merge_v12.each_raw_gain0,
                        pAmergeCtx->ProcRes.Merge_v12.each_raw_gain1);
            LOGV_AMERGE(
                "%s: sw_hdrmge_l_raw0: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
                __FUNCTION__, pAmergeCtx->ProcRes.Merge_v12.l0_y[0],
                pAmergeCtx->ProcRes.Merge_v12.l_raw0[1], pAmergeCtx->ProcRes.Merge_v12.l_raw0[2],
                pAmergeCtx->ProcRes.Merge_v12.l_raw0[3], pAmergeCtx->ProcRes.Merge_v12.l_raw0[4],
                pAmergeCtx->ProcRes.Merge_v12.l_raw0[5], pAmergeCtx->ProcRes.Merge_v12.l_raw0[6],
                pAmergeCtx->ProcRes.Merge_v12.l_raw0[7], pAmergeCtx->ProcRes.Merge_v12.l_raw0[8],
                pAmergeCtx->ProcRes.Merge_v12.l_raw0[9], pAmergeCtx->ProcRes.Merge_v12.l_raw0[10],
                pAmergeCtx->ProcRes.Merge_v12.l_raw0[11], pAmergeCtx->ProcRes.Merge_v12.l_raw0[12],
                pAmergeCtx->ProcRes.Merge_v12.l_raw0[13], pAmergeCtx->ProcRes.Merge_v12.l_raw0[14],
                pAmergeCtx->ProcRes.Merge_v12.l_raw0[15], pAmergeCtx->ProcRes.Merge_v12.l_raw0[16]);
            LOGV_AMERGE(
                "%s: sw_hdrmge_l_raw1: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
                __FUNCTION__, pAmergeCtx->ProcRes.Merge_v12.l1_y[0],
                pAmergeCtx->ProcRes.Merge_v12.l_raw1[1], pAmergeCtx->ProcRes.Merge_v12.l_raw1[2],
                pAmergeCtx->ProcRes.Merge_v12.l_raw1[3], pAmergeCtx->ProcRes.Merge_v12.l_raw1[4],
                pAmergeCtx->ProcRes.Merge_v12.l_raw1[5], pAmergeCtx->ProcRes.Merge_v12.l_raw1[6],
                pAmergeCtx->ProcRes.Merge_v12.l_raw1[7], pAmergeCtx->ProcRes.Merge_v12.l_raw1[8],
                pAmergeCtx->ProcRes.Merge_v12.l_raw1[9], pAmergeCtx->ProcRes.Merge_v12.l_raw1[10],
                pAmergeCtx->ProcRes.Merge_v12.l_raw1[11], pAmergeCtx->ProcRes.Merge_v12.l_raw1[12],
                pAmergeCtx->ProcRes.Merge_v12.l_raw1[13], pAmergeCtx->ProcRes.Merge_v12.l_raw1[14],
                pAmergeCtx->ProcRes.Merge_v12.l_raw1[15], pAmergeCtx->ProcRes.Merge_v12.l_raw1[16]);
        }
    } else {
        LOGV_AMERGE("%s: sw_hdrmge_ms_thd0:%d sw_hdrmge_ms_thd1:%d sw_hdrmge_ms_scl:%d\n",
                    __FUNCTION__, pAmergeCtx->ProcRes.Merge_v12.ms_thd0,
                    pAmergeCtx->ProcRes.Merge_v12.ms_thd1, pAmergeCtx->ProcRes.Merge_v12.ms_scl);
        LOGV_AMERGE("%s: sw_hdrmge_lm_thd0:%d sw_hdrmge_lm_thd1:%d sw_hdrmge_lm_scl:%d\n",
                    __FUNCTION__, pAmergeCtx->ProcRes.Merge_v12.lm_thd0,
                    pAmergeCtx->ProcRes.Merge_v12.lm_thd1, pAmergeCtx->ProcRes.Merge_v12.lm_scl);
    }
    LOGV_AMERGE("%s: sw_hdrmge_e_y: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
                __FUNCTION__, pAmergeCtx->ProcRes.Merge_v12.e_y[0],
                pAmergeCtx->ProcRes.Merge_v12.e_y[1], pAmergeCtx->ProcRes.Merge_v12.e_y[2],
                pAmergeCtx->ProcRes.Merge_v12.e_y[3], pAmergeCtx->ProcRes.Merge_v12.e_y[4],
                pAmergeCtx->ProcRes.Merge_v12.e_y[5], pAmergeCtx->ProcRes.Merge_v12.e_y[6],
                pAmergeCtx->ProcRes.Merge_v12.e_y[7], pAmergeCtx->ProcRes.Merge_v12.e_y[8],
                pAmergeCtx->ProcRes.Merge_v12.e_y[9], pAmergeCtx->ProcRes.Merge_v12.e_y[10],
                pAmergeCtx->ProcRes.Merge_v12.e_y[11], pAmergeCtx->ProcRes.Merge_v12.e_y[12],
                pAmergeCtx->ProcRes.Merge_v12.e_y[13], pAmergeCtx->ProcRes.Merge_v12.e_y[14],
                pAmergeCtx->ProcRes.Merge_v12.e_y[15], pAmergeCtx->ProcRes.Merge_v12.e_y[16]);
    LOGV_AMERGE("%s: sw_hdrmge_l0_y: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
                __FUNCTION__, pAmergeCtx->ProcRes.Merge_v12.l0_y[0],
                pAmergeCtx->ProcRes.Merge_v12.l0_y[1], pAmergeCtx->ProcRes.Merge_v12.l0_y[2],
                pAmergeCtx->ProcRes.Merge_v12.l0_y[3], pAmergeCtx->ProcRes.Merge_v12.l0_y[4],
                pAmergeCtx->ProcRes.Merge_v12.l0_y[5], pAmergeCtx->ProcRes.Merge_v12.l0_y[6],
                pAmergeCtx->ProcRes.Merge_v12.l0_y[7], pAmergeCtx->ProcRes.Merge_v12.l0_y[8],
                pAmergeCtx->ProcRes.Merge_v12.l0_y[9], pAmergeCtx->ProcRes.Merge_v12.l0_y[10],
                pAmergeCtx->ProcRes.Merge_v12.l0_y[11], pAmergeCtx->ProcRes.Merge_v12.l0_y[12],
                pAmergeCtx->ProcRes.Merge_v12.l0_y[13], pAmergeCtx->ProcRes.Merge_v12.l0_y[14],
                pAmergeCtx->ProcRes.Merge_v12.l0_y[15], pAmergeCtx->ProcRes.Merge_v12.l0_y[16]);
    LOGV_AMERGE("%s: sw_hdrmge_l1_y: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
                __FUNCTION__, pAmergeCtx->ProcRes.Merge_v12.l1_y[0],
                pAmergeCtx->ProcRes.Merge_v12.l1_y[1], pAmergeCtx->ProcRes.Merge_v12.l1_y[2],
                pAmergeCtx->ProcRes.Merge_v12.l1_y[3], pAmergeCtx->ProcRes.Merge_v12.l1_y[4],
                pAmergeCtx->ProcRes.Merge_v12.l1_y[5], pAmergeCtx->ProcRes.Merge_v12.l1_y[6],
                pAmergeCtx->ProcRes.Merge_v12.l1_y[7], pAmergeCtx->ProcRes.Merge_v12.l1_y[8],
                pAmergeCtx->ProcRes.Merge_v12.l1_y[9], pAmergeCtx->ProcRes.Merge_v12.l1_y[10],
                pAmergeCtx->ProcRes.Merge_v12.l1_y[11], pAmergeCtx->ProcRes.Merge_v12.l1_y[12],
                pAmergeCtx->ProcRes.Merge_v12.l1_y[13], pAmergeCtx->ProcRes.Merge_v12.l1_y[14],
                pAmergeCtx->ProcRes.Merge_v12.l1_y[15], pAmergeCtx->ProcRes.Merge_v12.l1_y[16]);

    LOG1_AMERGE("%s:exit!\n", __FUNCTION__);
}

/******************************************************************************
 * AmergeByPassProcessing()
 *get handle para by config and current variate
 *****************************************************************************/
bool AmergeByPassProcessing(AmergeContext_t* pAmergeCtx, AecPreResult_t AecHdrPreResult) {
    LOG1_AMERGE("%s:enter!\n", __FUNCTION__);

    bool bypass = false;
    float diff  = 0.0;

    if (pAmergeCtx->FrameID <= 2)
        bypass = false;
    else if (pAmergeCtx->mergeAttrV12.opMode != pAmergeCtx->CurrData.CtrlData.ApiMode)
        bypass = false;
    else if (pAmergeCtx->mergeAttrV12.opMode == MERGE_OPMODE_MANUAL)
        bypass = !pAmergeCtx->ifReCalcStManual;
    else if (pAmergeCtx->mergeAttrV12.opMode == MERGE_OPMODE_AUTO) {
        pAmergeCtx->NextData.HandleData.Merge_v12.MergeMode = pAmergeCtx->FrameNumber - 1;
        LOG1_AMERGE("%s:  Current MergeMode: %d \n", __FUNCTION__,
                    pAmergeCtx->NextData.HandleData.Merge_v12.MergeMode);

        // get envlv from AecPreRes
        AmergeGetEnvLv(pAmergeCtx, AecHdrPreResult);
        pAmergeCtx->NextData.CtrlData.ExpoData.EnvLv =
            LIMIT_VALUE(pAmergeCtx->NextData.CtrlData.ExpoData.EnvLv, ENVLVMAX, ENVLVMIN);

        pAmergeCtx->NextData.CtrlData.MoveCoef = MOVE_COEF_DEFAULT;
        pAmergeCtx->NextData.CtrlData.MoveCoef =
            LIMIT_VALUE(pAmergeCtx->NextData.CtrlData.MoveCoef, MOVECOEFMAX, MOVECOEFMIN);

        if (pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.CtrlDataType == CTRLDATATYPE_ENVLV) {
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
                if (diff >= pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.ByPassThr ||
                    diff <= (0 - pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.ByPassThr))
                    bypass = false;
                else
                    bypass = true;
            }
        } else if (pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.CtrlDataType ==
                   CTRLDATATYPE_ISO) {
            diff = pAmergeCtx->CurrData.CtrlData.ExpoData.ISO -
                   pAmergeCtx->NextData.CtrlData.ExpoData.ISO;
            diff /= pAmergeCtx->CurrData.CtrlData.ExpoData.ISO;
            if (diff >= pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.ByPassThr ||
                diff <= (0 - pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.ByPassThr))
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
        pAmergeCtx->NextData.CtrlData.ExpoData.LongFrmMode, pAmergeCtx->mergeAttrV12.opMode,
        pAmergeCtx->ifReCalcStAuto, pAmergeCtx->ifReCalcStManual,
        pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.CtrlDataType,
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

    CalibDbV2_merge_v12_t* calibv2_amerge_calib =
        (CalibDbV2_merge_v12_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibV2, amerge_calib));
    memcpy(&pAmergeCtx->mergeAttrV12.stAuto, calibv2_amerge_calib,
           sizeof(CalibDbV2_merge_v12_t));  // set  default stAuto

    // set default para
    // config default CurrData data
    pAmergeCtx->CurrData.CtrlData.ExpoData.EnvLv               = 0;
    pAmergeCtx->CurrData.CtrlData.ExpoData.LongFrmMode         = false;
    pAmergeCtx->CurrData.CtrlData.MoveCoef                     = 0;
    pAmergeCtx->CurrData.CtrlData.ApiMode                      = MERGE_OPMODE_AUTO;
    pAmergeCtx->CurrData.HandleData.Merge_v12.MergeMode        = 1;
    pAmergeCtx->CurrData.HandleData.Merge_v12.OECurve_smooth   = 0.4;
    pAmergeCtx->CurrData.HandleData.Merge_v12.OECurve_offset   = 210;
    pAmergeCtx->CurrData.HandleData.Merge_v12.MDCurveLM_smooth = 0.4;
    pAmergeCtx->CurrData.HandleData.Merge_v12.MDCurveLM_offset = 0.38;
    pAmergeCtx->CurrData.HandleData.Merge_v12.MDCurveMS_smooth = 0.4;
    pAmergeCtx->CurrData.HandleData.Merge_v12.MDCurveMS_offset = 0.38;
    pAmergeCtx->CurrData.HandleData.Merge_v12.MDCurve_Coef     = 0.05;
    pAmergeCtx->CurrData.HandleData.Merge_v12.MDCurve_ms_thd0  = 0;
    pAmergeCtx->CurrData.HandleData.Merge_v12.MDCurve_lm_thd0  = 0;

    // set default ctrl info
    pAmergeCtx->mergeAttrV12.opMode           = MERGE_OPMODE_AUTO;

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
