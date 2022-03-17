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


#define LIMIT_VALUE(value,max_value,min_value)      (value > max_value? max_value : value < min_value ? min_value : value)
#define SHIFT6BIT(A)         (A*64)
#define SHIFT7BIT(A)         (A*128)
#define SHIFT10BIT(A)         (A*1024)
#define SHIFT11BIT(A)         (A*2048)
#define SHIFT12BIT(A)         (A*4096)

#define LIMIT_PARA(a,b,c,d,e)      (c+(a-e)*(b-c)/(d -e))


#define AMERGE_RET_SUCCESS             0   //!< this has to be 0, if clauses rely on it
#define AMERGE_RET_FAILURE             1   //!< general failure
#define AMERGE_RET_NOTSUPP             2   //!< feature not supported
#define AMERGE_RET_BUSY                3   //!< there's already something going on...
#define AMERGE_RET_CANCELED            4   //!< operation canceled
#define AMERGE_RET_OUTOFMEM            5   //!< out of memory
#define AMERGE_RET_OUTOFRANGE          6   //!< parameter/value out of range
#define AMERGE_RET_IDLE                7   //!< feature/subsystem is in idle state
#define AMERGE_RET_WRONG_HANDLE        8   //!< handle is wrong
#define AMERGE_RET_NULL_POINTER        9   //!< the/one/all parameter(s) is a(are) NULL pointer(s)
#define AMERGE_RET_NOTAVAILABLE       10   //!< profile not available
#define AMERGE_RET_DIVISION_BY_ZERO   11   //!< a divisor equals ZERO
#define AMERGE_RET_WRONG_STATE        12   //!< state machine in wrong state
#define AMERGE_RET_INVALID_PARM       13   //!< invalid parameter
#define AMERGE_RET_PENDING            14   //!< command pending
#define AMERGE_RET_WRONG_CONFIG       15   //!< given configuration is invalid

#define MAXLUMAK     (1.5)
#define MAXLUMAB     (30)
#define ENVLVMAX     (1.0)
#define ENVLVMIN     (0.0)
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
#define AMERGE_MAX_IQ_DOTS (13)


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
    AMERGE_HW_MAX
} AmergeVersion_t;

typedef struct MergeHandleData_s
{
    int MergeMode;
    float OECurve_smooth;
    float OECurve_offset;
    float MDCurveLM_smooth;
    float MDCurveLM_offset;
    float MDCurveMS_smooth;
    float MDCurveMS_offset;
} MergeHandleData_t;

typedef struct MergePrevCtrlData_s
{
    int MergeMode;
    float L2S_ratio;
    float LExpo;
    float EnvLv;
    float MoveCoef;
    merge_OpMode_t ApiMode;
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
    float L2M_Ratio;
    float M2S_Ratio;
    float Lv_fac;
    float Contrast_fac;
} MergeCurrAeResult_t;

typedef struct MergeCurrCtrlData_s
{
    float L2S_Ratio;
    float L2M_Ratio;
    float L2L_Ratio;
    float LExpo;
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
    MgeProcRes_t Res;
    bool LongFrameMode;
} AmergeProcResData_t;

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

typedef struct AmergeContext_s
{
    //api
    mergeAttr_t mergeAttr;
    CalibDbV2_merge_t pCalibDB;
    AmergeState_t state;
    AmergeConfig_t Config;
    AmergePrevData_t PrevData ;
    AmergeProcResData_t ProcRes;
    MergeCurrAeResult_t AeResult;
    MergeCurrData_t CurrData;
    SensorInfo_t SensorInfo;
    AmergeVersion_t HWversion;
    bool SceneChange;
    uint32_t width;
    uint32_t height;
    int frameCnt;
    int FrameNumber;
} AmergeContext_t;

typedef AmergeContext_t* AmergeHandle_t;

typedef struct AmergeInstanceConfig_s {
    AmergeHandle_t              hAmerge;
} AmergeInstanceConfig_t;

typedef struct _RkAiqAlgoContext {
    AmergeInstanceConfig_t AmergeInstConfig;
    //void* place_holder[0];
} RkAiqAlgoContext;

#endif
