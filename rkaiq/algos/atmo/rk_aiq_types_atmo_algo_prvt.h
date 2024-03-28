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
#ifndef __RK_AIQ_TYPES_ATMO_ALGO_PRVT_H__
#define __RK_AIQ_TYPES_ATMO_ALGO_PRVT_H__

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "ae/rk_aiq_types_ae_algo_int.h"
#include "af/rk_aiq_types_af_algo_int.h"
#include "rk_aiq_algo_types.h"
#include "RkAiqCalibDbV2Helper.h"
#include "xcam_log.h"
#include "rk_aiq_types_atmo_stat_v200.h"
#include "amerge/rk_aiq_types_amerge_algo_prvt.h"


#define ATMO_RET_SUCCESS             0   //!< this has to be 0, if clauses rely on it
#define ATMO_RET_FAILURE             1   //!< general failure
#define ATMO_RET_NOTSUPP             2   //!< feature not supported
#define ATMO_RET_BUSY                3   //!< there's already something going on...
#define ATMO_RET_CANCELED            4   //!< operation canceled
#define ATMO_RET_OUTOFMEM            5   //!< out of memory
#define ATMO_RET_OUTOFRANGE          6   //!< parameter/value out of range
#define ATMO_RET_IDLE                7   //!< feature/subsystem is in idle state
#define ATMO_RET_WRONG_HANDLE        8   //!< handle is wrong
#define ATMO_RET_NULL_POINTER        9   //!< the/one/all parameter(s) is a(are) NULL pointer(s)
#define ATMO_RET_NOTAVAILABLE       10   //!< profile not available
#define ATMO_RET_DIVISION_BY_ZERO   11   //!< a divisor equals ZERO
#define ATMO_RET_WRONG_STATE        12   //!< state machine in wrong state
#define ATMO_RET_INVALID_PARM       13   //!< invalid parameter
#define ATMO_RET_PENDING            14   //!< command pending
#define ATMO_RET_WRONG_CONFIG       15   //!< given configuration is invalid

#define MAXLUMAK     (1.5)
#define MAXLUMAB     (30)
#define OEPDFMAX     (1.0)
#define OEPDFMIN     (0.0)
#define FOCUSLUMAMAX     (100)
#define FOCUSLUMAMIN     (1)
#define DAMPMAX     (1.0)
#define DAMPMIN     (0.0)
#define FASTMODELEVELMAX     (100.0)
#define FASTMODELEVELMIN     (1.0)
#define IIRMAX     (1000)
#define IIRMIN     (0)
#define DARKPDFTHMAX     (1.0)
#define DARKPDFTHMIN     (0.0)
#define TOLERANCEMAX     (20.0)
#define TOLERANCEMIN     (0.0)
#define GLOBEMAXLUMAMAX     (1023)
#define GLOBEMAXLUMAMIN     (51)
#define GLOBELUMAMAX     (737)
#define GLOBELUMAMIN     (51)
#define DETAILSHIGHLIGHTMAX     (1023)
#define DETAILSHIGHLIGHTMIN     (51)
#define DARKPDFMAX     (1)
#define DARKPDFMIN     (0)
#define DETAILSLOWLIGHTMAX     (63)
#define DETAILSLOWLIGHTMIN     (16)
#define DYNAMICRANGEMAX     (84)
#define DYNAMICRANGEMIN     (1)
#define TMOCONTRASTMAX     (255)
#define TMOCONTRASTMIN     (0)
#define IQPARAMAX     (1)
#define IQPARAMIN     (0)
#define IQDETAILSLOWLIGHTMAX     (4)
#define IQDETAILSLOWLIGHTMIN     (1)
#define ATMO_MAX_IQ_DOTS (13)

typedef enum AtmoState_e {
    ATMO_STATE_INVALID       = 0,
    ATMO_STATE_INITIALIZED   = 1,
    ATMO_STATE_STOPPED       = 2,
    ATMO_STATE_RUNNING       = 3,
    ATMO_STATE_LOCKED        = 4,
    ATMO_STATE_MAX
} AtmoState_t;

typedef struct TmoHandleData_s
{
    int MergeMode;
    float GlobeMaxLuma;
    float GlobeLuma;
    float DetailsHighLight;
    float DetailsLowLight;
    float LocalTmoStrength;
    float GlobalTmoStrength;
} TmoHandleData_t;

typedef struct TmoPrevCtrlData_s
{
    int FrameCnt;
    float L2S_ratio;
    float LExpo;
    float EnvLv;
    float OEPdf;
    float FocusLuma;
    float DarkPdf;
    float ISO;
    float DynamicRange;
} TmoPrevCtrlData_t;

typedef struct AtmoPrevData_s
{
    TmoPrevCtrlData_t CtrlData;
    TmoHandleData_t HandleData;
    unsigned short ro_hdrtmo_lgmean;
} AtmoPrevData_t;

typedef struct TmoCurrAeResult_s {
    //TODO
    float MeanLuma[3];
    float LfrmDarkLuma;
    float LfrmDarkPdf;
    float LfrmOverExpPdf;
    float SfrmMaxLuma;
    float SfrmMaxLumaPdf;
    float GlobalEnvLv;
    float L2M_Ratio;
    float M2S_Ratio;
    float DynamicRange;
    float OEPdf; //the pdf of over exposure in long frame
    float DarkPdf; //the pdf of dark region in long frame
    float ISO; //use long frame

    float Lv_fac;
    float DarkPdf_fac;
    float Contrast_fac;
    float BlockLumaS[225];
    float BlockLumaM[25];
    float BlockLumaL[225];

    //aec delay frame
    int AecDelayframe;

    //aec LumaDeviation
    float LumaDeviationL;
    float LumaDeviationM;
    float LumaDeviationS;
    float LumaDeviationLinear;
} TmoCurrAeResult_t;

typedef struct {
    unsigned char valid;
    int id;
    int depth;
} AfDepthInfo_t;

typedef struct TmoCurrAfResult_s {
    unsigned int CurrAfTargetPos;
    unsigned int CurrAfTargetWidth;
    unsigned int CurrAfTargetHeight;
    AfDepthInfo_t AfDepthInfo[225];
    unsigned int GlobalSharpnessCompensated[225];
} TmoCurrAfResult_t;

typedef struct TmoCurrCtrlData_s
{
    int FrameCnt;
    float L2S_Ratio;
    float L2M_Ratio;
    float L2L_Ratio;
    float LExpo;
    float EnvLv;
    float DynamicRange;
    float OEPdf;
    float DarkPdf;
    float ISO;
    float FocusLuma;
    float TmoDamp;
    float LgMean;
    float LumaWeight[225];
} TmoCurrCtrlData_t;

typedef struct AtmoCurrData_t
{
    TmoCurrCtrlData_t CtrlData;
    TmoHandleData_t HandleData;
} AtmoCurrData_t;

typedef struct AtmoProcResData_s
{
    TmoProcRes_t Res;
    bool LongFrameMode;
    bool isHdrGlobalTmo;
    bool bTmoEn;
    bool isLinearTmo;
    TmoFlickerPara_t TmoFlicker;
} AtmoProcResData_t;

typedef struct AtmoContext_s
{
    //api
    tmoAttr_t tmoAttr;
    CalibDbV2_tmo_t pCalibDB;
    AtmoState_t state;
    AtmoConfig_t AtmoConfig;
    AtmoPrevData_t PrevData ;
    AtmoProcResData_t ProcRes;
    TmoCurrAeResult_t CurrAeResult;
    TmoCurrAfResult_t CurrAfResult;
    AtmoCurrData_t CurrData;
    rkisp_atmo_stats_t CurrStatsData;
    SensorInfo_t SensorInfo;
    uint32_t width;
    uint32_t height;
    int frameCnt;
    FrameNumber_t FrameNumber;
} AtmoContext_t;

typedef AtmoContext_t* AtmoHandle_t;

typedef struct AtmoInstanceConfig_s {
    AtmoHandle_t              hAtmo;
} AtmoInstanceConfig_t;

typedef struct _RkAiqAlgoContext {
    AtmoInstanceConfig_t AtmoInstConfig;
    //void* place_holder[0];
} RkAiqAlgoContext;

#endif
