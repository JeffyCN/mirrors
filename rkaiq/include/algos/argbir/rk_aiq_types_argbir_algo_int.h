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
#ifndef __RK_AIQ_TYPES_ARGBIR_ALGO_INT_H__
#define __RK_AIQ_TYPES_ARGBIR_ALGO_INT_H__

#include "RkAiqCalibDbTypes.h"
#include "argbir_head.h"
#include "argbir_uapi_head.h"

typedef enum rgbir_OpMode_e {
    RGBIR_OPMODE_AUTO   = 0,  // run auto drc
    RGBIR_OPMODE_MANUAL = 1,  // run manual drc
} rgbir_OpMode_t;

// rgbir attr V10
typedef struct RgbirAttrV10_s {
    rk_aiq_uapi_sync_t sync;
    rgbir_OpMode_t opMode;
    CalibDbV2_argbir_v10_t stAuto;
    mRgbirAttr_v10_t stManual;
    RgbirInfoV10_t info;
} RgbirAttrV10_t;

#endif
