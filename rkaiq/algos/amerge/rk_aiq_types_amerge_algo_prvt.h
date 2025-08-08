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
#include <stdlib.h>
#include <string.h>

#include "RkAiqCalibDbV2Helper.h"
#include "ae/rk_aiq_types_ae_algo_int.h"
#include "af/rk_aiq_types_af_algo_int.h"
#include "rk_aiq_algo_types.h"
#include "amerge/rk_aiq_types_amerge_hw.h"
#include "xcam_log.h"

/********************also use for gamma, dehaze, tmo and drc*******************/
// for noramlize EnvLv
#define MAX_ENV_LUMA (65)
#define MAX_ENV_EXPO (10.0*1.0)
#define MIN_ENV_LUMA (0)
#define MIN_ENV_EXPO (30.0*256.0)
#define MAX_ENV_LV (MAX_ENV_LUMA / MAX_ENV_EXPO)
#define MIN_ENV_LV (MIN_ENV_LUMA / MIN_ENV_EXPO)
#define ENVLVMAX     (1.0)
#define ENVLVMIN     (0.0)
#define ISOMIN                (50.0)
#define ISOMAX                (10000000)
#define BIGMODE     (2560)
#define FUNCTION_DISABLE (0)
#define FUNCTION_ENABLE (1)
#define MOVE_COEF_DEFAULT (0)
#define NORMALIZE_MAX         (1.0)
#define NORMALIZE_MIN         (0.0)
#define SHIFT4BIT(A)          (A * 16.0)
#define SHIFT6BIT(A)          (A * 64.0)
#define SHIFT7BIT(A)          (A * 128.0)
#define SHIFT8BIT(A)          (A * 256.0)
#define SHIFT9BIT(A)          (A * 512.0)
#define SHIFT10BIT(A)         (A * 1024.0)
#define SHIFT11BIT(A)         (A * 2048.0)
#define SHIFT12BIT(A)         (A * 4096.0)
#define SHIFT13BIT(A)         (A * 8191.0)
#define SHIFT14BIT(A)         (A * 16383.0)
#define SHIFT15BIT(A)         (A * 32767.0)
#define BIT_MIN               (0)
#define BIT_3_MAX             (7)
#define BIT_4_MAX             (15)
#define BIT_8_MAX             (255)
#define BIT_9_MAX             (511)
#define BIT_10_MAX            (1023)
#define BIT_11_MAX            (2047)
#define BIT_12_MAX            (4095)
#define BIT_13_MAX            (8191)
#define BIT_14_MAX            (16383)
#define BIT_17_MAX            (131071)
#define RATIO_DEFAULT         (1.0f)
#define AE_RATIO_MAX          (256)
#define AE_RATIO_L2M_MAX      (32.0f)
#define LONG_FRAME_MODE_RATIO (1.0f)
#define ISP_PREDGAIN_DEFAULT  (1.0f)
#define INIT_CALC_PARAMS_NUM  (2)
#define ISP_HDR_BIT_NUM_MAX   (20)
#define ISP_HDR_BIT_NUM_MIN   (12)
#define LIMIT_VALUE(value, max_value, min_value) \
    (value > max_value ? max_value : value > min_value ? value : min_value)
#define LIMIT_VALUE_UNSIGNED(value, max_value) (value > max_value ? max_value : value)

typedef enum FrameNumber_e {
    LINEAR_NUM = 1,
    HDR_2X_NUM = 2,
    HDR_3X_NUM = 3,
    SENSOR_MGE = 4,
    HDR_NUM_MAX
} FrameNumber_t;
/****************************************************************************/

/********************merge params clip*******************/
#define OECURVESMOOTHMAX            (200)
#define OECURVESMOOTHMIN            (20)
#define OECURVEOFFSETMAX            (280)
#define OECURVEOFFSETMIN            (108)
#define MDCURVESMOOTHMAX            (200)
#define MDCURVESMOOTHMIN            (20)
#define MDCURVEOFFSETMAX            (100)
#define MDCURVEOFFSETMIN            (26)
#define EACHOECURVESMOOTHMAX        (50)
/********************************************************/

/********************merge fix params*******************/
#define SW_HDRMGE_GAIN_FIX          (0x40)
#define SW_HDRMGE_GAIN_INV_FIX      (0xfff)
#define SW_HDRMGE_LM_DIF_0P9_FIX    (255)
#define SW_HDRMGE_MS_DIF_0P8_FIX    (255)
#define HDR_LONG_FRMAE_MODE_OECURVE (0)
/*******************************************************/

#define SHORT_MODE_COEF_MAX (0.001)

typedef enum AmergeState_e {
    AMERGE_STATE_INVALID       = 0,
    AMERGE_STATE_INITIALIZED   = 1,
    AMERGE_STATE_STOPPED       = 2,
    AMERGE_STATE_RUNNING       = 3,
    AMERGE_STATE_LOCKED        = 4,
    AMERGE_STATE_MAX
} AmergeState_t;

typedef struct MergeExpoData_s {
    bool LongFrmMode;
    float EnvLv;
    float ISO;

    float LExpo;
    float MExpo;
    float SExpo;
    float SGain;
    float MGain;
    float RatioLS;
    float RatioLM;
} MergeExpoData_t;

typedef struct MergeHandleDataV10_s {
    int MergeMode;
    float OECurve_smooth;
    float OECurve_offset;
    float MDCurveLM_smooth;
    float MDCurveLM_offset;
    float MDCurveMS_smooth;
    float MDCurveMS_offset;
} MergeHandleDataV10_t;

typedef struct MergeHandleDataV11_s {
    MergeBaseFrame_t BaseFrm;
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
} MergeHandleDataV11_t;

typedef struct MergeHandleDataV12_s {
    MergeBaseFrame_t BaseFrm;
    int MergeMode;
    bool EnableEachChn;
    float OECurve_smooth;
    float OECurve_offset;
    float MDCurveLM_smooth;
    float MDCurveLM_offset;
    float MDCurveMS_smooth;
    float MDCurveMS_offset;
    float MDCurve_Coef;
    float MDCurve_ms_thd0;
    float MDCurve_lm_thd0;
    float EachChnCurve_smooth;
    float EachChnCurve_offset;
} MergeHandleDataV12_t;

typedef struct MergeHandleData_s {
#if RKAIQ_HAVE_MERGE_V10
    MergeHandleDataV10_t Merge_v10;
#endif
#if RKAIQ_HAVE_MERGE_V11
    MergeHandleDataV11_t Merge_v11;
#endif
#if RKAIQ_HAVE_MERGE_V12
    MergeHandleDataV12_t Merge_v12;
#endif
} MergeHandleData_t;

typedef struct MergeCurrCtrlData_s {
    int MergeMode;
    float MoveCoef;
    MergeExpoData_t ExpoData;
    merge_OpMode_t ApiMode;
} MergeCurrCtrlData_t;

typedef struct AmergeCurrData_s {
    MergeCurrCtrlData_t CtrlData;
    MergeHandleData_t HandleData;
} AmergeCurrData_t;

typedef struct MergeNextCtrlData_s {
    MergeExpoData_t ExpoData;
    float MoveCoef;
    float MergeOEDamp;
    float MergeMDDampLM;
    float MergeMDDampMS;
} MergeNextCtrlData_t;

typedef struct AmergeNextData_s {
    MergeNextCtrlData_t CtrlData;
    MergeHandleData_t HandleData;
} AmergeNextData_t;

typedef struct AmergeContext_s {
    bool isCapture;
    bool ifReCalcStAuto;
    bool ifReCalcStManual;
    bool SceneChange;
    uint32_t FrameID;
#if RKAIQ_HAVE_MERGE_V10
    mergeAttrV10_t mergeAttrV10;
#endif
#if RKAIQ_HAVE_MERGE_V11
    mergeAttrV11_t mergeAttrV11;
#endif
#if RKAIQ_HAVE_MERGE_V12
    mergeAttrV12_t mergeAttrV12;
#endif
    AmergeState_t state;
    AmergeCurrData_t CurrData;
    AmergeNextData_t NextData;
    FrameNumber_t FrameNumber;
} AmergeContext_t;

#endif
