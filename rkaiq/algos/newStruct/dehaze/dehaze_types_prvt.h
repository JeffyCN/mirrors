/*
* demosaic_types_prvt.h

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

#ifndef __RKAIQ_TYPES_Dehaze_ALGO_PRVT_H__
#define __RKAIQ_TYPES_Dehaze_ALGO_PRVT_H__

/**
 * @file dehaze_types_prvt.h
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
 * @defgroup Dehaze Auto dehaze
 * @{
 *
 */
#include "include/dehaze_algo_api.h"
#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"
#include "xcam_log.h"

#define DEFAULT_RECALCULATE_DELTA_ISO (0.01)
#define ISP_PREDGAIN_DEFAULT  (1.0f)
#define FUNCTION_ENABLE (1)
#define FUNCTION_DISABLE (0)
#define YNR_CURVE_STEP             (16)
#define LIMIT_VALUE(value,max_value,min_value)      (value > max_value? max_value : value < min_value ? min_value : value)
#define LIMIT_VALUE_UNSIGNED(value, max_value) (value > max_value ? max_value : value)
#define DEHAZE_GAUS_H0                              (2)
#define DEHAZE_GAUS_H1                              (4)
#define DEHAZE_GAUS_H2                              (2)
#define DEHAZE_GAUS_H3                              (4)
#define DEHAZE_GAUS_H4                              (8)
#define DEHAZE_GAUS_H5                              (4)
#define DEHAZE_GAUS_H6                              (2)
#define DEHAZE_GAUS_H7                              (4)
#define DEHAZE_GAUS_H8                              (2)
#define DHAZ_BLEND_WET_NUM     17

typedef struct DehazeContext_s {
    const RkAiqAlgoCom_prepare_t* prepare_params;
    dehaze_api_attrib_t* dehaze_attrib;
    int iso;
    bool isReCal_;
    int width;
    int height;
    int strength;
    int working_mode;
    float YnrProcRes_sigma[YNR_ISO_CURVE_POINT_NUM];
} DehazeContext_t;

#if RKAIQ_HAVE_DEHAZE_V12
XCamReturn DehazeSelectParam(DehazeContext_t *pDehazeCtx, dehaze_param_t* out, int iso, bool stats_true,
    dehaze_stats_v12_t* pStats);
#endif
#if RKAIQ_HAVE_DEHAZE_V14
XCamReturn DehazeSelectParam(DehazeContext_t *pDehazeCtx, dehaze_param_t* out, int iso);
#endif

#endif//__RKAIQ_TYPES_Dehaze_ALGO_PRVT_H__
