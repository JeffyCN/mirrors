/*
 *rk_aiq_types_asharp_algo_int_v34.h
 *
 *  Copyright (c) 2024 Rockchip Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef _RK_AIQ_TYPES_ASHARP_ALOG_INT_V34_H_
#define _RK_AIQ_TYPES_ASHARP_ALOG_INT_V34_H_

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "RkAiqCalibDbTypes.h"
#include "asharpV34/rk_aiq_types_asharp_algo_v34.h"
#include "rk_aiq_comm.h"
#include "sharp_head_v34.h"
#include "sharp_uapi_head_v34.h"

#define ASHARPV34_RECALCULATE_DELTA_ISO (10)
#define RK_SHARP_V34_MAX_ISO_NUM        CALIBDB_MAX_ISO_LEVEL

#define RK_SHARP_V34_SHARP_STRG_FIX_BITS 2
#define RK_SHARP_V34_GAUS_RATIO_FIX_BITS 7
#define RK_SHARP_V34_BF_RATIO_FIX_BITS   7
#define RK_SHARP_V34_PBFCOEFF_FIX_BITS   7
#define RK_SHARP_V34_RFCOEFF_FIX_BITS    7
#define RK_SHARP_V34_HBFCOEFF_FIX_BITS   7

#define RK_SHARP_V34_GLOBAL_GAIN_FIX_BITS       4
#define RK_SHARP_V34_GLOBAL_GAIN_ALPHA_FIX_BITS 3
#define RK_SHARP_V34_LOCAL_GAIN_FIX_BITS        4
#define RK_SHARP_V34_LOCAL_GAIN_SACLE_FIX_BITS  7
#define RK_SHARP_V34_ADJ_GAIN_FIX_BITS          10
#define RK_SHARP_V34_STRENGTH_TABLE_FIX_BITS    7
#define RK_SHARP_V34_TEX_FIX_BITS               10

#define INTERP_V4(x0, x1, ratio) ((ratio) * ((x1) - (x0)) + x0)
#define CLIP(a, min_v, max_v)    (((a) < (min_v)) ? (min_v) : (((a) > (max_v)) ? (max_v) : (a)))

typedef enum Asharp_result_V34_e {
    ASHARP_V34_RET_SUCCESS          = 0,  // this has to be 0, if clauses rely on it
    ASHARP_V34_RET_FAILURE          = 1,  // process failure
    ASHARP_V34_RET_INVALID_PARM     = 2,  // invalid parameter
    ASHARP_V34_RET_WRONG_CONFIG     = 3,  // feature not supported
    ASHARP_V34_RET_BUSY             = 4,  // there's already something going on...
    ASHARP_V34_RET_CANCELED         = 5,  // operation canceled
    ASHARP_V34_RET_OUTOFMEM         = 6,  // out of memory
    ASHARP_V34_RET_OUTOFRANGE       = 7,  // parameter/value out of range
    ASHARP_V34_RET_NULL_POINTER     = 8,  // the/one/all parameter(s) is a(are) NULL pointer(s)
    ASHARP_V34_RET_DIVISION_BY_ZERO = 9,  // a divisor equals ZERO
    ASHARP_V34_RET_NO_INPUTIMAGE    = 10  // no input image
} Asharp_result_V34_t;

typedef enum Asharp_State_V34_e {
    ASHARP_V34_STATE_INVALID     = 0, /**< initialization value */
    ASHARP_V34_STATE_INITIALIZED = 1, /**< instance is created, but not initialized */
    ASHARP_V34_STATE_STOPPED     = 2, /**< instance is confiured (ready to start) or stopped */
    ASHARP_V34_STATE_RUNNING     = 3, /**< instance is running (processes frames) */
    ASHARP_V34_STATE_LOCKED      = 4, /**< instance is locked (for taking snapshots) */
    ASHARP_V34_STATE_MAX              /**< max */
} Asharp_State_V34_t;

typedef enum Asharp_OPMode_V34_e {
    ASHARP_V34_OP_MODE_INVALID    = 0, /**< initialization value */
    ASHARP_V34_OP_MODE_AUTO       = 1, /**< instance is created, but not initialized */
    ASHARP_V34_OP_MODE_MANUAL     = 2, /**< instance is confiured (ready to start) or stopped */
    ASHARP_V34_OP_MODE_REG_MANUAL = 3,
    ASHARP_V34_OP_MODE_MAX /**< max */
} Asharp_OPMode_V34_t;

typedef enum Asharp_ParamMode_V34_e {
    ASHARP_V34_PARAM_MODE_INVALID = 0,
    ASHARP_V34_PARAM_MODE_NORMAL  = 1, /**< initialization value */
    ASHARP_V34_PARAM_MODE_HDR     = 2, /**< instance is created, but not initialized */
    ASHARP_V34_PARAM_MODE_GRAY    = 3, /**< instance is confiured (ready to start) or stopped */
    ASHARP_V34_PARAM_MODE_MAX          /**< max */
} Asharp_ParamMode_V34_t;

#if 0
// sharp params related to ISO
typedef struct RK_SHARP_V34_Select_ISO_s {
    int luma_sigma[RK_SHARP_V34_LUMA_POINT_NUM];
    int luma_point[RK_SHARP_V34_LUMA_POINT_NUM];
    float pbf_gain;
    float pbf_add;
    float pbf_ratio;
    float gaus_ratio;
    float sharp_ratio;
    int lum_clip_h[RK_SHARP_V34_LUMA_POINT_NUM];
    float bf_gain;
    float bf_add;
    float bf_ratio;
    int global_clip_pos;

    float prefilter_coeff[RK_SHARP_V34_PBF_DIAM * RK_SHARP_V34_PBF_DIAM];
    float GaussianFilter_coeff[RK_SHARP_V34_RF_DIAM * RK_SHARP_V34_RF_DIAM];
    float hfBilateralFilter_coeff[RK_SHARP_V34_BF_DIAM * RK_SHARP_V34_BF_DIAM];

    float global_gain;
    float global_gain_alpha;
    float local_gainscale;

    float gain_adj_sharp_strength[RK_SHARP_V34_SHARP_ADJ_GAIN_TABLE_LEN];
    int exgain_bypass;

    float dis_adj_sharp_strength[RK_SHARP_V34_STRENGTH_TABLE_LEN];

    float noiseclip_strength;
    int enhance_bit;
    int noiseclip_mode;
    int noise_sigma_clip;

    float prefilter_sigma;
    float hfBilateralFilter_sigma;
    float GaussianFilter_sigma;
    float GaussianFilter_radius;

} RK_SHARP_V34_Select_ISO_t;

// sharp params select
typedef struct RK_SHARP_Params_V34_Select_s {
    int enable;
    int kernel_sigma_enable;

    int center_mode;
    int center_x;
    int center_y;

    RK_SHARP_V34_Select_ISO_t sharpParamsSelectISO;

} RK_SHARP_Params_V34_Select_t;
#endif

// sharp params
typedef struct RK_SHARP_Params_V34_s {
    int enable;
    // int sharp_ratio_seperate_en;
    int sw_sharp_filtCoeff_mode;
    char version[64];

    int hw_sharp_centerPosition_mode;
    uint8_t hw_sharp_texWgt_mode;
    int center_x;
    int center_y;

    int iso[RK_SHARP_V34_MAX_ISO_NUM];
    RK_SHARP_Params_V34_Select_t sharpParamsISO[RK_SHARP_V34_MAX_ISO_NUM];

} RK_SHARP_Params_V34_t;

typedef struct Asharp_Manual_Attr_V34_s {
    RK_SHARP_Params_V34_Select_t stSelect;

    RK_SHARP_Fix_V34_t stFix;

} Asharp_Manual_Attr_V34_t;

typedef struct Asharp_Auto_Attr_V34_s {
    // all ISO params and select param

    RK_SHARP_Params_V34_t stParams;
    RK_SHARP_Params_V34_Select_t stSelect;

} Asharp_Auto_Attr_V34_t;

typedef struct Asharp_ProcResult_V34_s {
    // for hw register
    RK_SHARP_Fix_V34_t* stFix;
} Asharp_ProcResult_V34_t;

typedef struct Asharp_Config_V34_s {
    Asharp_State_V34_t eState;
    Asharp_OPMode_V34_t eMode;
    int rawHeight;
    int rawWidth;
} Asharp_Config_V34_t;

typedef struct rk_aiq_sharp_attrib_v34_s {
    rk_aiq_uapi_sync_t sync;

    Asharp_OPMode_V34_t eMode;
    Asharp_Auto_Attr_V34_t stAuto;
    Asharp_Manual_Attr_V34_t stManual;
} rk_aiq_sharp_attrib_v34_t;

typedef struct rk_aiq_sharp_strength_v34_s {
    rk_aiq_uapi_sync_t sync;

    float percent;
    bool strength_enable;
} rk_aiq_sharp_strength_v34_t;

#endif
