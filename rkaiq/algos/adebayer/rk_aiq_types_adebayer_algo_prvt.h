/*
* rk_aiq_types_adebayer_algo_prvt.h

* for rockchip v2.0.0
*
*  Copyright (c) 2019 Rockchip Corporation
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
/* for rockchip v2.0.0*/

#ifndef __RKAIQ_TYPES_ADEBAYER_ALGO_PRVT_H__
#define __RKAIQ_TYPES_ADEBAYER_ALGO_PRVT_H__

/**
 * @file rk_aiq_types_adebayer_algo_prvt.h
 *
 * @brief
 *
 *****************************************************************************/
/**
 * @page module_name_page Module Name
 * Describe here what this module does.
 *
 * For a detailed list of functions and implementation detail refer to:
 * - @ref module_name
 *
 * @defgroup ADEBAYER Auto debayer
 * @{
 *
 */
#include <atomic>
#include "rk_aiq_types_adebayer_algo_int.h"
#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"
#include "xcam_log.h"

#define INTERP_DEBAYER_V2(x0, x1, ratio)    ((ratio) * ((x1) - (x0)) + x0)
#define LOG2(x)                             (log((double)x) / log((double)2))

#define RK_DEBAYER_ISO_STEP_MAX 13


typedef enum AdebayerState_e {
    ADEBAYER_STATE_INVALID           = 0,
    ADEBAYER_STATE_INITIALIZED       = 1,
    ADEBAYER_STATE_STOPPED           = 2,
    ADEBAYER_STATE_RUNNING           = 3,
    ADEBAYER_STATE_LOCKED            = 4,
    ADEBAYER_STATE_MAX
} AdebayerState_t;

typedef struct AdebayerFullParam_s {
    unsigned char enable;
    int iso[9];
    signed char filter1[5];
    signed char filter2[5];
    unsigned char gain_offset;
    unsigned char sharp_strength[9];
    unsigned char offset;
    unsigned short hf_offset[9];
    unsigned char clip_en;
    unsigned char filter_g_en;
    unsigned char filter_c_en;
    unsigned char thed0;
    unsigned char thed1;
    unsigned char dist_scale;
    unsigned char cnr_strength;
    unsigned char shift_num;
} AdebayerFullParam_t;

typedef struct AdebayerContext_s {
    std::atomic<bool> is_reconfig; //used for api
    AdebayerState_t state;
    rk_aiq_debayer_op_mode_t mode;


#if RKAIQ_HAVE_DEBAYER_V1
    AdebayerHwConfigV1_t config; //result params
    AdebayerFullParam_t full_param; //json params
    AdebayerSeletedParamV1_t manualAttrib;
#endif

#if RKAIQ_HAVE_DEBAYER_V2
    AdebayerHwConfigV2_t config; //result params
    CalibDbV2_Debayer_Tuning_t full_param_v2; //json params
    AdebayerSeletedParamV2_t   select_param_v2;//actually use params
#endif

#if RKAIQ_HAVE_DEBAYER_V2_LITE
    AdebayerHwConfigV2_t config; //result params
    CalibDbV2_Debayer_Tuning_Lite_t full_param_v2_lite; //json params
    AdebayerSeletedParamV2Lite_t   select_param_v2_lite;//actually use params
#endif

#if RKAIQ_HAVE_DEBAYER_V3
    AdebayerHwConfigV3_t config; //result params
    CalibDbV2_Debayer_Tuning_V3_t full_param_v3; //json params
    AdebayerSeletedParamV3_t   select_param_v3;//actually use params
    int                  compr_bit;
#endif

    int iso;
} AdebayerContext_t;

typedef struct _RkAiqAlgoContext {
    AdebayerContext_t adebayerCtx;
} RkAiqAlgoContext;

#endif//__RKAIQ_TYPES_ADEBAYER_ALGO_PRVT_H__
