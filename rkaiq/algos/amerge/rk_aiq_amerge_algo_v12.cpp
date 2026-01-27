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

float MergeGetInterpRatioV12(float* pX, int& lo, int& hi, float CtrlValue, int length_max) {
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

int mergeClipValueV12(float posx, int BitInt, int BitFloat, bool ifBitMax) {
    int yOutInt = 0, yOutIntMin = 0, yOutIntMax = 0;

    if (ifBitMax)
        yOutIntMax = (int)(pow(2, (BitFloat + BitInt)));
    else
        yOutIntMax = (int)(pow(2, (BitFloat + BitInt)) - 1);
    yOutInt = LIMIT_VALUE((int)(posx * pow(2, BitFloat)), yOutIntMax, yOutIntMin);

    return yOutInt;
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
        curve      = 1.0f + exp(-smooth * OECURVESMOOTHMAX * (k / 1023.0f - offset / 256.0f));
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
        curve      = 1.0f + exp(-smooth * EACHOECURVESMOOTHMAX * (k / 1023.0f - offset));
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
void AmergeGetTuningProcResV12(AmergeContext_t* pAmergeCtx,
                               RkAiqAmergeProcResult_t* pAmergeProcRes) {
    LOG1_AMERGE("%s:Enter!\n", __FUNCTION__);

    pAmergeProcRes->Merge_v12.mode        = pAmergeCtx->NextData.HandleData.Merge_v12.MergeMode;
    pAmergeProcRes->Merge_v12.s_base      = pAmergeCtx->NextData.HandleData.Merge_v12.BaseFrm;
    pAmergeProcRes->Merge_v12.each_raw_en = pAmergeCtx->NextData.HandleData.Merge_v12.EnableEachChn;
    pAmergeProcRes->Merge_v12.lm_dif_0p9  = SW_HDRMGE_LM_DIF_0P9_FIX;
    pAmergeProcRes->Merge_v12.ms_dif_0p8  = SW_HDRMGE_MS_DIF_0P8_FIX;
    pAmergeProcRes->Merge_v12.lm_dif_0p15 =
        (unsigned char)(pAmergeCtx->NextData.HandleData.Merge_v12.MDCurveLM_offset *
                        MDCURVEOFFSETMAX);
    pAmergeProcRes->Merge_v12.ms_dif_0p15 =
        (unsigned char)(pAmergeCtx->NextData.HandleData.Merge_v12.MDCurveMS_offset *
                        MDCURVEOFFSETMAX);

    if (pAmergeCtx->NextData.CtrlData.ExpoData.LongFrmMode) {
        for (int i = 0; i < HDRMGE_V12_OE_CURVE_NUM; i++)
            pAmergeProcRes->Merge_v12.e_y[i] = HDR_LONG_FRMAE_MODE_OECURVE;
    } else {
        CalibrateOECurve(pAmergeCtx->NextData.HandleData.Merge_v12.OECurve_smooth,
                         pAmergeCtx->NextData.HandleData.Merge_v12.OECurve_offset,
                         pAmergeProcRes->Merge_v12.e_y);
    }
    if (pAmergeCtx->NextData.HandleData.Merge_v12.BaseFrm == BASEFRAME_LONG) {
        CalibrateMDCurveLongFrmMode(pAmergeCtx->NextData.HandleData.Merge_v12.MDCurveLM_smooth,
                                    pAmergeCtx->NextData.HandleData.Merge_v12.MDCurveLM_offset,
                                    pAmergeProcRes->Merge_v12.l1_y);
        CalibrateMDCurveLongFrmMode(pAmergeCtx->NextData.HandleData.Merge_v12.MDCurveMS_smooth,
                                    pAmergeCtx->NextData.HandleData.Merge_v12.MDCurveMS_offset,
                                    pAmergeProcRes->Merge_v12.l0_y);
        // merge v12 add
        if (pAmergeProcRes->Merge_v12.each_raw_en) {
            CalibrateEachChnCurve(pAmergeCtx->NextData.HandleData.Merge_v12.EachChnCurve_smooth,
                                  pAmergeCtx->NextData.HandleData.Merge_v12.EachChnCurve_offset,
                                  pAmergeProcRes->Merge_v12.l_raw0);
            CalibrateEachChnCurve(pAmergeCtx->NextData.HandleData.Merge_v12.EachChnCurve_smooth,
                                  pAmergeCtx->NextData.HandleData.Merge_v12.EachChnCurve_offset,
                                  pAmergeProcRes->Merge_v12.l_raw1);
        }
    } else if (pAmergeCtx->NextData.HandleData.Merge_v12.BaseFrm == BASEFRAME_SHORT) {
        CalibrateMDCurveShortFrmMode(pAmergeCtx->NextData.HandleData.Merge_v12.MDCurveLM_smooth,
                                     pAmergeCtx->NextData.HandleData.Merge_v12.MDCurveLM_offset,
                                     pAmergeProcRes->Merge_v12.l1_y,
                                     pAmergeProcRes->Merge_v12.l0_y);
    }

    LOG1_AMERGE("%s:Eixt!\n", __FUNCTION__);
}

/******************************************************************************
 * MergeDampingV12()
 *****************************************************************************/
void MergeDampingV12(AmergeContext_t* pAmergeCtx) {
    LOG1_AMERGE("%s:Enter!\n", __FUNCTION__);

    if (pAmergeCtx->FrameID > 2) {
        pAmergeCtx->NextData.HandleData.Merge_v12.OECurve_smooth =
            pAmergeCtx->NextData.CtrlData.MergeOEDamp *
                pAmergeCtx->NextData.HandleData.Merge_v12.OECurve_smooth +
            (1.0f - pAmergeCtx->NextData.CtrlData.MergeOEDamp) *
                pAmergeCtx->CurrData.HandleData.Merge_v12.OECurve_smooth;
        pAmergeCtx->NextData.HandleData.Merge_v12.OECurve_offset =
            pAmergeCtx->NextData.CtrlData.MergeOEDamp *
                pAmergeCtx->NextData.HandleData.Merge_v12.OECurve_offset +
            (1.0f - pAmergeCtx->NextData.CtrlData.MergeOEDamp) *
                pAmergeCtx->CurrData.HandleData.Merge_v12.OECurve_offset;
        pAmergeCtx->NextData.HandleData.Merge_v12.MDCurveLM_smooth =
            pAmergeCtx->NextData.CtrlData.MergeMDDampLM *
                pAmergeCtx->NextData.HandleData.Merge_v12.MDCurveLM_smooth +
            (1.0f - pAmergeCtx->NextData.CtrlData.MergeMDDampLM) *
                pAmergeCtx->CurrData.HandleData.Merge_v12.MDCurveLM_smooth;
        pAmergeCtx->NextData.HandleData.Merge_v12.MDCurveLM_offset =
            pAmergeCtx->NextData.CtrlData.MergeMDDampLM *
                pAmergeCtx->NextData.HandleData.Merge_v12.MDCurveLM_offset +
            (1.0f - pAmergeCtx->NextData.CtrlData.MergeMDDampLM) *
                pAmergeCtx->CurrData.HandleData.Merge_v12.MDCurveLM_offset;
        pAmergeCtx->NextData.HandleData.Merge_v12.MDCurveMS_smooth =
            pAmergeCtx->NextData.CtrlData.MergeMDDampMS *
                pAmergeCtx->NextData.HandleData.Merge_v12.MDCurveMS_smooth +
            (1.0f - pAmergeCtx->NextData.CtrlData.MergeMDDampMS) *
                pAmergeCtx->CurrData.HandleData.Merge_v12.MDCurveMS_smooth;
        pAmergeCtx->NextData.HandleData.Merge_v12.MDCurveMS_offset =
            pAmergeCtx->NextData.CtrlData.MergeMDDampMS *
                pAmergeCtx->NextData.HandleData.Merge_v12.MDCurveMS_offset +
            (1.0f - pAmergeCtx->NextData.CtrlData.MergeMDDampMS) *
                pAmergeCtx->CurrData.HandleData.Merge_v12.MDCurveMS_offset;

        pAmergeCtx->NextData.HandleData.Merge_v12.MDCurve_Coef =
            pAmergeCtx->NextData.CtrlData.MergeMDDampMS *
                pAmergeCtx->NextData.HandleData.Merge_v12.MDCurve_Coef +
            (1.0f - pAmergeCtx->NextData.CtrlData.MergeMDDampMS) *
                pAmergeCtx->CurrData.HandleData.Merge_v12.MDCurve_Coef;
        pAmergeCtx->NextData.HandleData.Merge_v12.MDCurve_ms_thd0 =
            pAmergeCtx->NextData.CtrlData.MergeMDDampMS *
                pAmergeCtx->NextData.HandleData.Merge_v12.MDCurve_ms_thd0 +
            (1.0f - pAmergeCtx->NextData.CtrlData.MergeMDDampMS) *
                pAmergeCtx->CurrData.HandleData.Merge_v12.MDCurve_ms_thd0;
        pAmergeCtx->NextData.HandleData.Merge_v12.MDCurve_lm_thd0 =
            pAmergeCtx->NextData.CtrlData.MergeMDDampMS *
                pAmergeCtx->NextData.HandleData.Merge_v12.MDCurve_lm_thd0 +
            (1.0f - pAmergeCtx->NextData.CtrlData.MergeMDDampMS) *
                pAmergeCtx->CurrData.HandleData.Merge_v12.MDCurve_lm_thd0;
    }

    LOG1_AMERGE("%s:Eixt!\n", __FUNCTION__);
}

/******************************************************************************
 * AmergeTuningProcessing()
 *get handle para by config and current variate
 *****************************************************************************/
void AmergeTuningProcessing(AmergeContext_t* pAmergeCtx, RkAiqAmergeProcResult_t* pAmergeProcRes) {
    LOG1_AMERGE("%s:enter!\n", __FUNCTION__);

    pAmergeCtx->NextData.HandleData.Merge_v12.MergeMode = pAmergeCtx->FrameNumber - 1;

    if (pAmergeCtx->mergeAttrV12.opMode == MERGE_OPMODE_AUTO) {
        int lo = 0, hi = 0;
        float ratio     = 0.0f;
        float CtrlValue = pAmergeCtx->NextData.CtrlData.ExpoData.EnvLv;
        if (pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.CtrlDataType == CTRLDATATYPE_ISO)
            CtrlValue = pAmergeCtx->NextData.CtrlData.ExpoData.ISO;

        pAmergeCtx->NextData.HandleData.Merge_v12.BaseFrm =
            pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.BaseFrm;
        if (pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.BaseFrm == BASEFRAME_LONG) {
            // get Current merge OECurve
            ratio = MergeGetInterpRatioV12(
                pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.LongFrmModeData.OECurve.CtrlData,
                lo, hi, CtrlValue, MERGE_ENVLV_STEP_MAX);
            pAmergeCtx->NextData.HandleData.Merge_v12.OECurve_smooth =
                ratio * (pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.LongFrmModeData.OECurve
                             .Smooth[hi] -
                         pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.LongFrmModeData.OECurve
                             .Smooth[lo]) +
                pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.LongFrmModeData.OECurve.Smooth[lo];
            pAmergeCtx->NextData.HandleData.Merge_v12.OECurve_offset =
                ratio * (pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.LongFrmModeData.OECurve
                             .Offset[hi] -
                         pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.LongFrmModeData.OECurve
                             .Offset[lo]) +
                pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.LongFrmModeData.OECurve.Offset[lo];

            // get Current merge MDCurve
            ratio = MergeGetInterpRatioV12(
                pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.LongFrmModeData.MDCurve.MoveCoef,
                lo, hi, pAmergeCtx->NextData.CtrlData.MoveCoef, MERGE_ENVLV_STEP_MAX);
            pAmergeCtx->NextData.HandleData.Merge_v12.MDCurveLM_smooth =
                ratio * (pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.LongFrmModeData.MDCurve
                             .LM_smooth[hi] -
                         pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.LongFrmModeData.MDCurve
                             .LM_smooth[lo]) +
                pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.LongFrmModeData.MDCurve
                    .LM_smooth[lo];
            pAmergeCtx->NextData.HandleData.Merge_v12.MDCurveLM_offset =
                ratio * (pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.LongFrmModeData.MDCurve
                             .LM_offset[hi] -
                         pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.LongFrmModeData.MDCurve
                             .LM_offset[lo]) +
                pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.LongFrmModeData.MDCurve
                    .LM_offset[lo];
            pAmergeCtx->NextData.HandleData.Merge_v12.MDCurveMS_smooth =
                ratio * (pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.LongFrmModeData.MDCurve
                             .MS_smooth[hi] -
                         pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.LongFrmModeData.MDCurve
                             .MS_smooth[lo]) +
                pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.LongFrmModeData.MDCurve
                    .MS_smooth[lo];
            pAmergeCtx->NextData.HandleData.Merge_v12.MDCurveMS_offset =
                ratio * (pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.LongFrmModeData.MDCurve
                             .MS_offset[hi] -
                         pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.LongFrmModeData.MDCurve
                             .MS_offset[lo]) +
                pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.LongFrmModeData.MDCurve
                    .MS_offset[lo];

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
                ratio = MergeGetInterpRatioV12(pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara
                                                   .LongFrmModeData.EachChnCurve.CtrlData,
                                               lo, hi, CtrlValue, MERGE_ENVLV_STEP_MAX);
                pAmergeCtx->NextData.HandleData.Merge_v12.EachChnCurve_smooth =
                    ratio * (pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.LongFrmModeData
                                 .EachChnCurve.Smooth[hi] -
                             pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.LongFrmModeData
                                 .EachChnCurve.Smooth[lo]) +
                    pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.LongFrmModeData.EachChnCurve
                        .Smooth[lo];
                pAmergeCtx->NextData.HandleData.Merge_v12.EachChnCurve_offset =
                    ratio * (pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.LongFrmModeData
                                 .EachChnCurve.Offset[hi] -
                             pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.LongFrmModeData
                                 .EachChnCurve.Offset[lo]) +
                    pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.LongFrmModeData.EachChnCurve
                        .Offset[lo];
            }
        } else if (pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.BaseFrm == BASEFRAME_SHORT) {
            // get Current merge OECurve
            ratio = MergeGetInterpRatioV12(pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara
                                               .LongFrmModeData.EachChnCurve.CtrlData,
                                           lo, hi, CtrlValue, MERGE_ENVLV_STEP_MAX);
            pAmergeCtx->NextData.HandleData.Merge_v12.OECurve_smooth =
                ratio * (pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.ShortFrmModeData.OECurve
                             .Smooth[hi] -
                         pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.ShortFrmModeData.OECurve
                             .Smooth[lo]) +
                pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.ShortFrmModeData.OECurve.Smooth[lo];
            pAmergeCtx->NextData.HandleData.Merge_v12.OECurve_offset =
                ratio * (pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.ShortFrmModeData.OECurve
                             .Offset[hi] -
                         pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.ShortFrmModeData.OECurve
                             .Offset[lo]) +
                pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.ShortFrmModeData.OECurve.Offset[lo];

            // get Current merge MDCurve
            ratio = MergeGetInterpRatioV12(
                pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.ShortFrmModeData.MDCurve.MoveCoef,
                lo, hi, pAmergeCtx->NextData.CtrlData.MoveCoef, MERGE_ENVLV_STEP_MAX);
            pAmergeCtx->NextData.HandleData.Merge_v12.MDCurve_Coef =
                ratio * (pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.ShortFrmModeData.MDCurve
                             .Coef[hi] -
                         pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.ShortFrmModeData.MDCurve
                             .Coef[lo]) +
                pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.ShortFrmModeData.MDCurve.Coef[lo];
            pAmergeCtx->NextData.HandleData.Merge_v12.MDCurve_ms_thd0 =
                ratio * (pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.ShortFrmModeData.MDCurve
                             .ms_thd0[hi] -
                         pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.ShortFrmModeData.MDCurve
                             .ms_thd0[lo]) +
                pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.ShortFrmModeData.MDCurve
                    .ms_thd0[lo];
            pAmergeCtx->NextData.HandleData.Merge_v12.MDCurve_lm_thd0 =
                ratio * (pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.ShortFrmModeData.MDCurve
                             .lm_thd0[hi] -
                         pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.ShortFrmModeData.MDCurve
                             .lm_thd0[lo]) +
                pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.ShortFrmModeData.MDCurve
                    .lm_thd0[lo];

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
    AmergeGetTuningProcResV12(pAmergeCtx, pAmergeProcRes);

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
void AmergeExpoProcessing(AmergeContext_t* pAmergeCtx, MergeExpoData_t* pExpoData,
                          RkAiqAmergeProcResult_t* pAmergeProcRes) {
    LOG1_AMERGE("%s:enter!\n", __FUNCTION__);

    // get sw_hdrmge_gain0
    pAmergeProcRes->Merge_v12.gain0 = mergeClipValueV12(pExpoData->RatioLS, 8, 6, false);
    pAmergeProcRes->Merge_v12.gain0_inv =
        mergeClipValueV12(RATIO_DEFAULT / pExpoData->RatioLS, 0, 12, false);

    // get sw_hdrmge_gain1
    pAmergeProcRes->Merge_v12.gain1     = SW_HDRMGE_GAIN_FIX;
    pAmergeProcRes->Merge_v12.gain1_inv = SW_HDRMGE_GAIN_INV_FIX;

    // get sw_hdrmge_gain2
    pAmergeProcRes->Merge_v12.gain2 = SW_HDRMGE_GAIN_FIX;

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
        pAmergeProcRes->Merge_v12.ms_thd0 = mergeClipValueV12(sw_hdrmge_ms_thd0, 0, 10, false);
        pAmergeProcRes->Merge_v12.ms_thd1 = mergeClipValueV12(sw_hdrmge_ms_thd1, 0, 10, false);
        pAmergeProcRes->Merge_v12.ms_scl  = (unsigned short)(64.0f * sw_hdrmge_ms_scl);
        pAmergeProcRes->Merge_v12.ms_scl =
            LIMIT_VALUE_UNSIGNED(pAmergeProcRes->Merge_v12.ms_scl, 0x7ff);
        pAmergeProcRes->Merge_v12.lm_thd0 = mergeClipValueV12(sw_hdrmge_lm_thd0, 0, 10, false);
        pAmergeProcRes->Merge_v12.lm_thd1 = mergeClipValueV12(sw_hdrmge_lm_thd1, 0, 10, false);
        pAmergeProcRes->Merge_v12.lm_scl  = (unsigned short)(64.0f * sw_hdrmge_lm_scl);
        pAmergeProcRes->Merge_v12.lm_scl =
            LIMIT_VALUE_UNSIGNED(pAmergeProcRes->Merge_v12.lm_scl, 0x7ff);
    }

    // merge v12 add
    if (pAmergeCtx->NextData.HandleData.Merge_v12.BaseFrm == BASEFRAME_LONG &&
        pAmergeProcRes->Merge_v12.each_raw_en) {
        pAmergeProcRes->Merge_v12.each_raw_gain0 =
            mergeClipValueV12(pExpoData->RatioLS, 8, 6, false);  // ratio between middle and short
        pAmergeProcRes->Merge_v12.each_raw_gain1 =
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
        __FUNCTION__, pExpoData->RatioLS, pAmergeProcRes->Merge_v12.gain0,
        pAmergeProcRes->Merge_v12.gain0_inv, pExpoData->RatioLM, pAmergeProcRes->Merge_v12.gain1,
        pAmergeProcRes->Merge_v12.gain1_inv, pAmergeProcRes->Merge_v12.gain2);
    LOGV_AMERGE("%s: sw_hdrmge_mode:%d s_base:%d each_raw_en:%d\n", __FUNCTION__,
                pAmergeProcRes->Merge_v12.mode, pAmergeProcRes->Merge_v12.s_base,
                pAmergeProcRes->Merge_v12.each_raw_en);
    if (!(pAmergeProcRes->Merge_v12.s_base)) {
        LOGV_AMERGE(
            "%s: sw_hdrmge_ms_dif_0p8:%d sw_hdrmge_lm_dif_0p9:%d "
            "sw_hdrmge_ms_dif_0p15:%d sw_hdrmge_lm_dif_0p15:%d\n",
            __FUNCTION__, pAmergeProcRes->Merge_v12.ms_dif_0p8,
            pAmergeProcRes->Merge_v12.lm_dif_0p9, pAmergeProcRes->Merge_v12.ms_dif_0p15,
            pAmergeProcRes->Merge_v12.lm_dif_0p15);

        if (pAmergeProcRes->Merge_v12.each_raw_en) {
            LOGV_AMERGE("%s: each_raw_gain0:%d each_raw_gain1:%d\n", __FUNCTION__,
                        pAmergeProcRes->Merge_v12.each_raw_gain0,
                        pAmergeProcRes->Merge_v12.each_raw_gain1);
            LOGV_AMERGE(
                "%s: sw_hdrmge_l_raw0: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
                __FUNCTION__, pAmergeProcRes->Merge_v12.l0_y[0],
                pAmergeProcRes->Merge_v12.l_raw0[1], pAmergeProcRes->Merge_v12.l_raw0[2],
                pAmergeProcRes->Merge_v12.l_raw0[3], pAmergeProcRes->Merge_v12.l_raw0[4],
                pAmergeProcRes->Merge_v12.l_raw0[5], pAmergeProcRes->Merge_v12.l_raw0[6],
                pAmergeProcRes->Merge_v12.l_raw0[7], pAmergeProcRes->Merge_v12.l_raw0[8],
                pAmergeProcRes->Merge_v12.l_raw0[9], pAmergeProcRes->Merge_v12.l_raw0[10],
                pAmergeProcRes->Merge_v12.l_raw0[11], pAmergeProcRes->Merge_v12.l_raw0[12],
                pAmergeProcRes->Merge_v12.l_raw0[13], pAmergeProcRes->Merge_v12.l_raw0[14],
                pAmergeProcRes->Merge_v12.l_raw0[15], pAmergeProcRes->Merge_v12.l_raw0[16]);
            LOGV_AMERGE(
                "%s: sw_hdrmge_l_raw1: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
                __FUNCTION__, pAmergeProcRes->Merge_v12.l1_y[0],
                pAmergeProcRes->Merge_v12.l_raw1[1], pAmergeProcRes->Merge_v12.l_raw1[2],
                pAmergeProcRes->Merge_v12.l_raw1[3], pAmergeProcRes->Merge_v12.l_raw1[4],
                pAmergeProcRes->Merge_v12.l_raw1[5], pAmergeProcRes->Merge_v12.l_raw1[6],
                pAmergeProcRes->Merge_v12.l_raw1[7], pAmergeProcRes->Merge_v12.l_raw1[8],
                pAmergeProcRes->Merge_v12.l_raw1[9], pAmergeProcRes->Merge_v12.l_raw1[10],
                pAmergeProcRes->Merge_v12.l_raw1[11], pAmergeProcRes->Merge_v12.l_raw1[12],
                pAmergeProcRes->Merge_v12.l_raw1[13], pAmergeProcRes->Merge_v12.l_raw1[14],
                pAmergeProcRes->Merge_v12.l_raw1[15], pAmergeProcRes->Merge_v12.l_raw1[16]);
        }
    } else {
        LOGV_AMERGE("%s: sw_hdrmge_ms_thd0:%d sw_hdrmge_ms_thd1:%d sw_hdrmge_ms_scl:%d\n",
                    __FUNCTION__, pAmergeProcRes->Merge_v12.ms_thd0,
                    pAmergeProcRes->Merge_v12.ms_thd1, pAmergeProcRes->Merge_v12.ms_scl);
        LOGV_AMERGE("%s: sw_hdrmge_lm_thd0:%d sw_hdrmge_lm_thd1:%d sw_hdrmge_lm_scl:%d\n",
                    __FUNCTION__, pAmergeProcRes->Merge_v12.lm_thd0,
                    pAmergeProcRes->Merge_v12.lm_thd1, pAmergeProcRes->Merge_v12.lm_scl);
    }
    LOGV_AMERGE("%s: sw_hdrmge_e_y: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
                __FUNCTION__, pAmergeProcRes->Merge_v12.e_y[0], pAmergeProcRes->Merge_v12.e_y[1],
                pAmergeProcRes->Merge_v12.e_y[2], pAmergeProcRes->Merge_v12.e_y[3],
                pAmergeProcRes->Merge_v12.e_y[4], pAmergeProcRes->Merge_v12.e_y[5],
                pAmergeProcRes->Merge_v12.e_y[6], pAmergeProcRes->Merge_v12.e_y[7],
                pAmergeProcRes->Merge_v12.e_y[8], pAmergeProcRes->Merge_v12.e_y[9],
                pAmergeProcRes->Merge_v12.e_y[10], pAmergeProcRes->Merge_v12.e_y[11],
                pAmergeProcRes->Merge_v12.e_y[12], pAmergeProcRes->Merge_v12.e_y[13],
                pAmergeProcRes->Merge_v12.e_y[14], pAmergeProcRes->Merge_v12.e_y[15],
                pAmergeProcRes->Merge_v12.e_y[16]);
    LOGV_AMERGE("%s: sw_hdrmge_l0_y: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
                __FUNCTION__, pAmergeProcRes->Merge_v12.l0_y[0], pAmergeProcRes->Merge_v12.l0_y[1],
                pAmergeProcRes->Merge_v12.l0_y[2], pAmergeProcRes->Merge_v12.l0_y[3],
                pAmergeProcRes->Merge_v12.l0_y[4], pAmergeProcRes->Merge_v12.l0_y[5],
                pAmergeProcRes->Merge_v12.l0_y[6], pAmergeProcRes->Merge_v12.l0_y[7],
                pAmergeProcRes->Merge_v12.l0_y[8], pAmergeProcRes->Merge_v12.l0_y[9],
                pAmergeProcRes->Merge_v12.l0_y[10], pAmergeProcRes->Merge_v12.l0_y[11],
                pAmergeProcRes->Merge_v12.l0_y[12], pAmergeProcRes->Merge_v12.l0_y[13],
                pAmergeProcRes->Merge_v12.l0_y[14], pAmergeProcRes->Merge_v12.l0_y[15],
                pAmergeProcRes->Merge_v12.l0_y[16]);
    LOGV_AMERGE("%s: sw_hdrmge_l1_y: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
                __FUNCTION__, pAmergeProcRes->Merge_v12.l1_y[0], pAmergeProcRes->Merge_v12.l1_y[1],
                pAmergeProcRes->Merge_v12.l1_y[2], pAmergeProcRes->Merge_v12.l1_y[3],
                pAmergeProcRes->Merge_v12.l1_y[4], pAmergeProcRes->Merge_v12.l1_y[5],
                pAmergeProcRes->Merge_v12.l1_y[6], pAmergeProcRes->Merge_v12.l1_y[7],
                pAmergeProcRes->Merge_v12.l1_y[8], pAmergeProcRes->Merge_v12.l1_y[9],
                pAmergeProcRes->Merge_v12.l1_y[10], pAmergeProcRes->Merge_v12.l1_y[11],
                pAmergeProcRes->Merge_v12.l1_y[12], pAmergeProcRes->Merge_v12.l1_y[13],
                pAmergeProcRes->Merge_v12.l1_y[14], pAmergeProcRes->Merge_v12.l1_y[15],
                pAmergeProcRes->Merge_v12.l1_y[16]);

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
    else if (pAmergeCtx->mergeAttrV12.opMode != pAmergeCtx->CurrData.CtrlData.ApiMode)
        bypass = false;
    else if (pAmergeCtx->mergeAttrV12.opMode == MERGE_OPMODE_MANUAL)
        bypass = !pAmergeCtx->ifReCalcStManual;
    else if (pAmergeCtx->mergeAttrV12.opMode == MERGE_OPMODE_AUTO) {
        if (pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.CtrlDataType == CTRLDATATYPE_ENVLV) {
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
                if (diff >= pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.ByPassThr ||
                    diff <= -pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.ByPassThr)
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
                diff <= -pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.ByPassThr)
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
 * AmergeSetDefaultManuAttrParmasV12()
 *****************************************************************************/
void AmergeSetDefaultManuAttrParmasV12(AmergeContext_t* pAmergeCtx) {
    LOG1_AMERGE("%s:enter!\n", __FUNCTION__);

    pAmergeCtx->mergeAttrV12.stManual.BaseFrm =
        pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.BaseFrm;
    pAmergeCtx->mergeAttrV12.stManual.LongFrmModeData.EnableEachChn =
        pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.LongFrmModeData.EnableEachChn;
    pAmergeCtx->mergeAttrV12.stManual.LongFrmModeData.OECurve.Smooth =
        pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.LongFrmModeData.OECurve.Smooth[0];
    pAmergeCtx->mergeAttrV12.stManual.LongFrmModeData.OECurve.Offset =
        pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.LongFrmModeData.OECurve.Offset[0];
    pAmergeCtx->mergeAttrV12.stManual.LongFrmModeData.MDCurve.LM_smooth =
        pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.LongFrmModeData.MDCurve.LM_smooth[0];
    pAmergeCtx->mergeAttrV12.stManual.LongFrmModeData.MDCurve.LM_offset =
        pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.LongFrmModeData.MDCurve.LM_offset[0];
    pAmergeCtx->mergeAttrV12.stManual.LongFrmModeData.MDCurve.MS_smooth =
        pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.LongFrmModeData.MDCurve.MS_smooth[0];
    pAmergeCtx->mergeAttrV12.stManual.LongFrmModeData.MDCurve.MS_offset =
        pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.LongFrmModeData.MDCurve.MS_offset[0];
    pAmergeCtx->mergeAttrV12.stManual.LongFrmModeData.EachChnCurve.Smooth =
        pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.LongFrmModeData.EachChnCurve.Smooth[0];
    pAmergeCtx->mergeAttrV12.stManual.LongFrmModeData.EachChnCurve.Offset =
        pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.LongFrmModeData.EachChnCurve.Offset[0];
    pAmergeCtx->mergeAttrV12.stManual.ShortFrmModeData.OECurve.Smooth =
        pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.ShortFrmModeData.OECurve.Smooth[0];
    pAmergeCtx->mergeAttrV12.stManual.ShortFrmModeData.OECurve.Offset =
        pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.ShortFrmModeData.OECurve.Offset[0];
    pAmergeCtx->mergeAttrV12.stManual.ShortFrmModeData.MDCurve.Coef =
        pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.ShortFrmModeData.MDCurve.Coef[0];
    pAmergeCtx->mergeAttrV12.stManual.ShortFrmModeData.MDCurve.ms_thd0 =
        pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.ShortFrmModeData.MDCurve.ms_thd0[0];
    pAmergeCtx->mergeAttrV12.stManual.ShortFrmModeData.MDCurve.lm_thd0 =
        pAmergeCtx->mergeAttrV12.stAuto.MergeTuningPara.ShortFrmModeData.MDCurve.lm_thd0[0];

    LOG1_AMERGE("%s:exit!\n", __FUNCTION__);
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

    CalibDbV2_merge_v12_t* calibv2_amerge_calib =
        (CalibDbV2_merge_v12_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibV2, amerge_calib));
    memcpy(&pAmergeCtx->mergeAttrV12.stAuto, calibv2_amerge_calib, sizeof(CalibDbV2_merge_v12_t));
    pAmergeCtx->mergeAttrV12.opMode = MERGE_OPMODE_AUTO;
    AmergeSetDefaultManuAttrParmasV12(pAmergeCtx);
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
