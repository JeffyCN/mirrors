/******************************************************************************
 *
 * Copyright 2024, Fuzhou Rockchip Electronics Co.Ltd . All rights reserved.
 * No part of this work may be reproduced, modified, distributed, transmitted,
 * transcribed, or translated into any language or computer format, in any form
 * or by any means without written permission of:
 * Fuzhou Rockchip Electronics Co.Ltd .
 *
 *
 *****************************************************************************/
#ifndef __RK_AIQ_TYPES_ARGBIR_ALGO_PRVT_H__
#define __RK_AIQ_TYPES_ARGBIR_ALGO_PRVT_H__

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "RkAiqCalibDbV2Helper.h"
#include "ae/rk_aiq_types_ae_algo_int.h"
#include "amerge/rk_aiq_types_amerge_algo_prvt.h"
#include "rk_aiq_algo_types.h"
#include "rk_aiq_types_argbir_algo_int.h"
#include "rk_aiq_types_argbir_hw.h"
#include "xcam_log.h"

#define RGBIR_BYPASS_THREAD (0.01f)

typedef enum aRgbIrState_e {
    ARGBIR_STATE_INVALID     = 0,
    ARGBIR_STATE_INITIALIZED = 1,
    ARGBIR_STATE_STOPPED     = 2,
    ARGBIR_STATE_RUNNING     = 3,
    ARGBIR_STATE_LOCKED      = 4,
    ARGBIR_STATE_MAX
} aRgbIrState_t;

typedef struct RgbirCurrData_s {
    float iso;
    rgbir_OpMode_t ApiMode;
    mRgbirAttr_v10_t HandleData;
} RgbirCurrData_t;

typedef struct aRgbirContext_s {
    bool isCapture;
    bool ifReCalcStAuto;
    bool ifReCalcStManual;
#if RKAIQ_HAVE_RGBIR_REMOSAIC_V10
    RgbirAttrV10_t RgbirAttrV10;
#endif
    aRgbIrState_t state;
    RgbirCurrData_t CurrData;
    RgbirCurrData_t NextData;
    uint32_t FrameID;
    FrameNumber_t FrameNumber;
} aRgbirContext_t;

#endif
