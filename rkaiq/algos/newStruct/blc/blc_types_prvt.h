/*
* blc_types_prvt.h

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

#ifndef __RKAIQ_TYPES_BLC_ALGO_PRVT_H__
#define __RKAIQ_TYPES_BLC_ALGO_PRVT_H__

/**
 * @file blc_types_prvt.h
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
 * @defgroup Blc auto
 * @{
 *
 */
#include "include/blc_algo_api.h"
#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"
#include "xcam_log.h"

#define INTERP_DEBAYER_V2(x0, x1, ratio)    ((ratio) * ((x1) - (x0)) + x0)
#define LOG2(x)                             (log((double)x) / log((double)2))

typedef struct BlcContext_s {
    const RkAiqAlgoCom_prepare_t* prepare_params;
    blc_api_attrib_t* blc_attrib;
    int iso;
    bool isReCal_;
    bool isAiIsp;
} BlcContext_t;

XCamReturn BlcSelectParam(BlcContext_t *pBlcCtx, blc_param_t* out, int iso);

#endif//__RKAIQ_TYPES_BLC_ALGO_PRVT_H__
