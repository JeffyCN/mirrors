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
#ifndef __RK_AIQ_TYPES_ADRC_ALGO_PRVT_H__
#define __RK_AIQ_TYPES_ADRC_ALGO_PRVT_H__

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "RkAiqCalibDbV2Helper.h"
#include "ae/rk_aiq_types_ae_algo_int.h"
#include "af/rk_aiq_types_af_algo_int.h"
#include "amerge/rk_aiq_types_amerge_algo_prvt.h"
#include "rk_aiq_algo_types.h"
#include "rk_aiq_types_adrc_algo_int.h"
#include "rk_aiq_types_adrc_hw.h"
#include "xcam_log.h"

#define GAINMIN                 (1.0f)
#define DRCGAINMAX     (8)
#define DRCGAINMIN              (GAINMIN)
#define CLIPMAX     (64.0)
#define CLIPMIN     (0.0)
#define MINOGAINMAX     (2.0)
#define MINOGAINMIN     (0.0)
#define ADRCNORMALIZEINTMAX     (1)
#define ADRCNORMALIZEINTMIN     (0)
#define SPACESGMMAX     (4095)
#define SPACESGMMIN     (0)
#define SCALEYMAX     (2048)
#define SCALEYMIN     (0)
#define MANUALCURVEMAX     (8192)
#define MANUALCURVEMIN     (0)
#define IIRFRAMEMAX     (1000)
#define IIRFRAMEMIN     (0)
#define INTMIN                  (0)
#define INT4BITMAX              (15)
#define INT8BITMAX     (255)
#define INT12BITMAX             (4095)
#define INT13BITMAX             (8191)
#define INT14BITMAX     (16383)
#define INT16BITMAX             (65535)
#define SW_DRC_OFFSET_POW2_FIX (8)
#define SW_DRC_MOTION_SCL_FIX (0)
#define SW_DRC_BILAT_WT_OFF_FIX (255)
#define MAX_AE_DRC_GAIN         (256.0f)
#define OB_PREDGAIN_MAX         (255.9)
#define OB_PREDGAIN_MIN         (GAINMIN)
#define GAS_T_MAX               (4)
#define GAS_T_MIN               (0)
#define GAS_L_MAX               (64)
#define GAS_L_MIN               (0)
#define GAS_L0_DEFAULT          (24)
#define GAS_L1_DEFAULT          (10)
#define GAS_L2_DEFAULT          (10)
#define GAS_L3_DEFAULT          (5)

#define ISP_RAW_BIT (12)
#define MFHDR_LOG_Q_BITS (11)
#define DSTBITS (ISP_RAW_BIT << MFHDR_LOG_Q_BITS)
#define OFFSETBITS_INT (SW_DRC_OFFSET_POW2_FIX)
#define OFFSETBITS (OFFSETBITS_INT << MFHDR_LOG_Q_BITS)
#define VALIDBITS (DSTBITS - OFFSETBITS)
#define DELTA_SCALEIN_FIX ((256 << MFHDR_LOG_Q_BITS) / VALIDBITS)

typedef enum AdrcState_e {
    ADRC_STATE_INVALID       = 0,
    ADRC_STATE_INITIALIZED   = 1,
    ADRC_STATE_STOPPED       = 2,
    ADRC_STATE_RUNNING       = 3,
    ADRC_STATE_LOCKED        = 4,
    ADRC_STATE_MAX
} AdrcState_t;

typedef struct DrcOhters_s {
    bool OutPutLongFrame;
    float curPixWeit;
    float preFrameWeit;
    float Range_force_sgm;
    float Range_sgm_cur;
    float Range_sgm_pre;
    int Space_sgm_cur;
    int Space_sgm_pre;
    int Scale_y[ADRC_Y_NUM];
    float ByPassThr;
    float Edge_Weit;
    float Tolerance;
    int IIR_frame;
    float damp;
} DrcOhters_t;

typedef struct CompressConfig_s {
    CompressMode_t Mode;
    uint16_t       Manual_curve[ADRC_Y_NUM];
} CompressConfig_t;

typedef struct DrcHandleDataV10_s {
    float DrcGain;
    float Alpha;
    float Clip;
    float Strength;
    float LocalWeit;
    float GlobalContrast;
    float LoLitContrast;
    CompressMode_t Mode;
    uint16_t Manual_curve[ADRC_Y_NUM];
} DrcHandleDataV10_t;

typedef struct DrcHandleDataV11_s {
    float DrcGain;
    float Alpha;
    float Clip;
    float Strength;
    float LocalWeit;
    int LocalAutoEnable;
    float LocalAutoWeit;
    float GlobalContrast;
    float LoLitContrast;
    CompressMode_t Mode;
    uint16_t       Manual_curve[ADRC_Y_NUM];
} DrcHandleDataV11_t;

typedef struct DrcHandleDataV12_s {
    float DrcGain;
    float Alpha;
    float Clip;
    float Strength;
    float LocalWeit;
    int LocalAutoEnable;
    float LocalAutoWeit;
    float GlobalContrast;
    float LoLitContrast;
    CompressMode_t Mode;
    uint16_t       Manual_curve[ADRC_Y_NUM];
    // v12 add
    float gas_t;
    int gas_l0;
    int gas_l1;
    int gas_l2;
    int gas_l3;
    float MotionStr;
} DrcHandleDataV12_t;

typedef struct DrcHandleData_s {
#if RKAIQ_HAVE_DRC_V10
    DrcHandleDataV10_t Drc_v10;
#endif
#if RKAIQ_HAVE_DRC_V11
    DrcHandleDataV11_t Drc_v11;
#endif
#if RKAIQ_HAVE_DRC_V12 || RKAIQ_HAVE_DRC_V12_LITE
    DrcHandleDataV12_t Drc_v12;
#endif
} DrcHandleData_t;

typedef struct AdrcAEData_s {
    bool LongFrmMode;
    float ISO;  // invaild in Curr
    float EnvLv;

    float L2M_Ratio;
    float M2S_Ratio;
    float L2S_Ratio;
    float LExpo;  // invaild in Curr
    float MExpo;  // invaild in Curr
    float SExpo;  // invaild in Curr
} AdrcAEData_t;

typedef struct CurrData_s {
    bool Enable;
    float MotionCoef;
    drc_OpMode_t ApiMode;
    int FrameID;
    AdrcAEData_t AEData;
    FrameNumber_t FrameNumber;
    DrcHandleData_t HandleData;
} CurrData_t;

typedef struct NextData_s {
    bool Enable;
    float MotionCoef;
    float Damp;
    float LumaWeight[225];
    int FrameID;
    FrameNumber_t FrameNumber;
    AdrcAEData_t AEData;
    DrcHandleData_t HandleData;
    DrcOhters_t Others;
} NextData_t;

typedef struct AdrcContext_s {
    bool isCapture;
    bool ifReCalcStAuto;
    bool ifReCalcStManual;
    bool isDampStable;
#if RKAIQ_HAVE_DRC_V10
    drcAttrV10_t drcAttrV10;
#endif
#if RKAIQ_HAVE_DRC_V11
    drcAttrV11_t drcAttrV11;
#endif
#if RKAIQ_HAVE_DRC_V12
    drcAttrV12_t drcAttrV12;
    adrc_blcRes_V32_t ablcV32_proc_res;
#endif
#if RKAIQ_HAVE_DRC_V12_LITE
    drcAttrV12Lite_t drcAttrV12;
    adrc_blcRes_V32_t ablcV32_proc_res;
#endif
    AdrcState_t state;
    CurrData_t CurrData;
    NextData_t NextData;
    RkAiqAdrcProcResult_t AdrcProcRes;
    uint32_t FrameID;
    FrameNumber_t FrameNumber;
} AdrcContext_t;

#endif
