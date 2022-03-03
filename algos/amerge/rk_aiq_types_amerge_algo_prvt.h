/******************************************************************************
 *
 * Copyright 2019, Fuzhou Rockchip Electronics Co.Ltd . All rights reserved.
 * No part of this work may be reproduced, modified, distributed, transmitted,
 * transcribed, or translated into any language or computer format, in any form
 * or by any means without written permission of:
 * Fuzhou Rockchip Electronics Co.Ltd .
 *
 *
 *****************************************************************************/
#ifndef __RK_AIQ_TYPES_AMERGE_ALGO_PRVT_H__
#define __RK_AIQ_TYPES_AMERGE_ALGO_PRVT_H__

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "ae/rk_aiq_types_ae_algo_int.h"
#include "af/rk_aiq_types_af_algo_int.h"
#include "rk_aiq_algo_types.h"
#include "RkAiqCalibDbV2Helper.h"
#include "xcam_log.h"


/********************also use for dehaze, tmo and drc*******************/
//for noramlize EnvLv
#define MAX_ENV_LUMA (65)
#define MAX_ENV_EXPO (10.0*1.0)
#define MIN_ENV_LUMA (0)
#define MIN_ENV_EXPO (30.0*256.0)
#define MAX_ENV_LV (MAX_ENV_LUMA / MAX_ENV_EXPO)
#define MIN_ENV_LV (MIN_ENV_LUMA / MIN_ENV_EXPO)
#define ENVLVMAX     (1.0)
#define ENVLVMIN     (0.0)
#define ISOMIN     (50)
#define ISOMAX     (204800)
#define BIGMODE     (2560)
#define FUNCTION_DISABLE (0)
#define FUNCTION_ENABLE (1)
#define MOVE_COEF_DEFAULT (0)
#define SHIFT4BIT(A)         (A*16)
#define SHIFT6BIT(A)         (A*64)
#define SHIFT7BIT(A)         (A*128)
#define SHIFT8BIT(A)         (A*256)
#define SHIFT10BIT(A)         (A*1024)
#define SHIFT11BIT(A)         (A*2048)
#define SHIFT12BIT(A)         (A*4096)
#define SHIFT13BIT(A)         (A*8191)
#define SHIFT14BIT(A)         (A*16383)
#define SHIFT15BIT(A)         (A*32767)
#define LIMIT_VALUE(value,max_value,min_value)      (value > max_value? max_value : value < min_value ? min_value : value)
#define LIMIT_PARA(a,b,c,d,e)      (c+(a-e)*(b-c)/(d -e))

typedef enum FrameNumber_e {
    LINEAR_NUM   = 1,
    HDR_2X_NUM   = 2,
    HDR_3X_NUM   = 3,
    HDR_NUM_MAX
} FrameNumber_t;
/****************************************************************************/

#define MAXLUMAK     (1.5)
#define MAXLUMAB     (30)
#define MOVECOEFMAX     (1.0)
#define MOVECOEFMIN     (0.0)
#define OECURVESMOOTHMAX     (200)
#define OECURVESMOOTHMIN     (20)
#define OECURVEOFFSETMAX     (300)
#define OECURVEOFFSETMIN     (108)
#define MDCURVESMOOTHMAX     (200)
#define MDCURVESMOOTHMIN     (20)
#define MDCURVEOFFSETMAX     (100)
#define MDCURVEOFFSETMIN     (26)
#define DAMPMAX     (1.0)
#define DAMPMIN     (0.0)
#define TOLERANCEMAX     (20.0)
#define TOLERANCEMIN     (0.0)
#define IQPARAMAX     (1)
#define IQPARAMIN     (0)
#define IQ_TH0_PARAMAX     (1023)
#define IQ_TH0_PARAMIN     (0)
#define AMERGE_MAX_IQ_DOTS (13)
#define SW_HDRMGE_GAIN_FIX (0x40)
#define SW_HDRMGE_GAIN_INV_FIX (0xfff)
#define SW_HDRMGE_LM_DIF_0P9_FIX (255)
#define SW_HDRMGE_MS_DIF_0P8_FIX (255)
#define SHORT_MODE_COEF_MAX (0.001)
#define HDR_LONG_FRMAE_MODE_OECURVE (0)

typedef enum AmergeState_e {
    AMERGE_STATE_INVALID       = 0,
    AMERGE_STATE_INITIALIZED   = 1,
    AMERGE_STATE_STOPPED       = 2,
    AMERGE_STATE_RUNNING       = 3,
    AMERGE_STATE_LOCKED        = 4,
    AMERGE_STATE_MAX
} AmergeState_t;

typedef enum AmergeVersion_e {
    AMERGE_ISP20      = 0,
    AMERGE_ISP21      = 1,
    AMERGE_ISP30      = 2,
    AMERGE_HW_MAX
} AmergeVersion_t;

typedef struct MergeExpoData_s {
    float nextLExpo;
    float nextMExpo;
    float nextSExpo;
    float nextSGain;
    float nextMGain;
    float nextRatioLS;
    float nextRatioLM;
} MergeExpoData_t;

typedef struct MergeHandleDataV20_s
{
    int MergeMode;
    float OECurve_smooth;
    float OECurve_offset;
    float MDCurveLM_smooth;
    float MDCurveLM_offset;
    float MDCurveMS_smooth;
    float MDCurveMS_offset;
} MergeHandleDataV20_t;

typedef struct MergeHandleDataV30_s
{
    int MergeMode;
    float OECurve_smooth;
    float OECurve_offset;
    float MDCurveLM_smooth;
    float MDCurveLM_offset;
    float MDCurveMS_smooth;
    float MDCurveMS_offset;
    float MDCurve_Coef;
    float MDCurve_ms_thd0;
    float MDCurve_lm_thd0;
} MergeHandleDataV30_t;

typedef struct MergeHandleData_s {
    union {
        MergeHandleDataV20_t Merge_v20;
        MergeHandleDataV30_t Merge_v30;
    };
} MergeHandleData_t;

typedef struct MergePrevCtrlData_s
{
    int MergeMode;
    float EnvLv;
    float MoveCoef;
    merge_OpModeV21_t ApiMode;
} MergePrevCtrlData_t;

typedef struct AmergePrevData_s
{
    MergePrevCtrlData_t CtrlData;
    MergeHandleData_t HandleData;
} AmergePrevData_t;

typedef struct MergeCurrAeResult_s {
    float MeanLuma[3];
    float LfrmDarkLuma;
    float LfrmDarkPdf;
    float LfrmOverExpPdf;
    float SfrmMaxLuma;
    float SfrmMaxLumaPdf;
    float GlobalEnvLv;
    float Lv_fac;
    float Contrast_fac;
} MergeCurrAeResult_t;

typedef struct MergeCurrCtrlData_s
{
    float EnvLv;
    float MoveCoef;
    float MergeOEDamp;
    float MergeMDDampLM;
    float MergeMDDampMS;
} MergeCurrCtrlData_t;

typedef struct MergeCurrData_s
{
    MergeCurrCtrlData_t CtrlData;
    MergeHandleData_t HandleData;
} MergeCurrData_t;

typedef struct AmergeProcResData_s
{
    union {
        MgeProcRes_t Merge_v20;
        MgeProcResV2_t Merge_v30;
    };
    bool update;
    bool LongFrameMode;
} AmergeProcResData_t;

typedef struct AmergeConfigV20_s {
    MergeBaseFrame_t BaseFrm;
    int MergeMode;
    int MaxEnvLvKnots;
    int MaxMoveCoefKnots;
    float* EnvLv; //0: dark 1:bright
    float* MoveCoef;
    float* OECurve_smooth;  //current over exposure curve slope
    float* OECurve_offset; //current over exposure curve offset
    float* MDCurveLM_smooth;  //Move Detect curve slope betwwen long frame and middle frame
    float* MDCurveLM_offset;  //Move Detect curve offset betwwen long frame and middle frame
    float* MDCurveMS_smooth;  //Move Detect curve slope betwwen middle frame and short frame
    float* MDCurveMS_offset;  //Move Detect curve slope betwwen middle frame and short frame
    float ByPassThr;
    float OECurve_damp;
    float MDCurveLM_damp;
    float MDCurveMS_damp ;
} AmergeConfigV20_t;

typedef struct AmergeConfigV30_s {
    MergeBaseFrame_t BaseFrm;
    float ByPassThr;
    int MergeMode;

    int LongFrmMaxEnvLvKnots;
    int LongFrmMaxMoveCoefKnots;
    float* LongFrmEnvLv; //0: dark 1:bright
    float* LongFrmOECurve_smooth;  //current over exposure curve slope
    float* LongFrmOECurve_offset; //current over exposure curve offset
    float* LongFrmMoveCoef;
    float* LongFrmMDCurveLM_smooth;  //Move Detect curve slope betwwen long frame and middle frame
    float* LongFrmMDCurveLM_offset;  //Move Detect curve offset betwwen long frame and middle frame
    float* LongFrmMDCurveMS_smooth;  //Move Detect curve slope betwwen middle frame and short frame
    float* LongFrmMDCurveMS_offset;  //Move Detect curve slope betwwen middle frame and short frame
    float LongFrmOECurve_damp;
    float LongFrmMDCurveLM_damp;
    float LongFrmMDCurveMS_damp ;

    int ShortFrmMaxEnvLvKnots;
    int ShortFrmMaxMoveCoefKnots;
    float* ShortFrmEnvLv; //0: dark 1:bright
    float* ShortFrmOECurve_smooth;  //current over exposure curve slope
    float* ShortFrmOECurve_offset; //current over exposure curve offset
    float* ShortFrmMoveCoef;
    float* ShortFrmMDCurve_Coef;  //Move Detect curve slope betwwen long frame and middle frame
    float* ShortFrmMDCurve_ms_thd0;  //Move Detect curve offset betwwen long frame and middle frame
    float* ShortFrmMDCurve_lm_thd0;  //Move Detect curve slope betwwen middle frame and short frame
    float ShortFrmOECurve_damp;
    float ShortFrmMDCurve_damp;
} AmergeConfigV30_t;

typedef struct AmergeConfig_s {
    union {
        AmergeConfigV20_t Merge_v20;
        AmergeConfigV30_t Merge_v30;
    };
} AmergeConfig_t;

typedef struct SensorInfo_s
{
    bool  LongFrmMode;
    float HdrMinGain[MAX_HDR_FRAMENUM];
    float HdrMaxGain[MAX_HDR_FRAMENUM];
    float HdrMinIntegrationTime[MAX_HDR_FRAMENUM];
    float HdrMaxIntegrationTime[MAX_HDR_FRAMENUM];

    float MaxExpoL;
    float MinExpoL;
    float MaxExpoM;
    float MinExpoM;
    float MaxExpoS;
    float MinExpoS;
} SensorInfo_t;

typedef struct AmergeCalib_s {
    union {
        CalibDbV2_merge_t Merge_v20;
        CalibDbV2_merge_V2_t Merge_v30;
    };
} AmergeCalib_t;

typedef struct AmergeContext_s
{
    //api
    mergeAttr_t mergeAttr;
    AmergeCalib_t pCalibDB;
    AmergeState_t state;
    AmergeConfig_t Config;
    AmergePrevData_t PrevData ;
    AmergeProcResData_t ProcRes;
    MergeCurrAeResult_t AeResult;
    MergeCurrData_t CurrData;
    SensorInfo_t SensorInfo;
    bool SceneChange;
    int frameCnt;
    FrameNumber_t FrameNumber;
} AmergeContext_t;

#endif
