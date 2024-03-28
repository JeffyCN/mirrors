/*
* demosaic_types_prvt.h

* for rockchip v2.0.0
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
/* for rockchip v2.0.0*/

#ifndef __RKAIQ_TYPES_Dm_ALGO_PRVT_H__
#define __RKAIQ_TYPES_Dm_ALGO_PRVT_H__

/**
 * @file demosaic_types_prvt.h
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
 * @defgroup Dm Auto debayer
 * @{
 *
 */
#include "include/demosaic_algo_api.h"
#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"
#include "xcam_log.h"

#define INTERP_DEBAYER_V2(x0, x1, ratio)    ((ratio) * ((x1) - (x0)) + x0)
#define LOG2(x)                             (log((double)x) / log((double)2))

#define RK_DEBAYER_ISO_STEP_MAX 13
#define DEFAULT_RECALCULATE_DELTA_ISO (0.01)

typedef struct DmContext_s {
    const RkAiqAlgoCom_prepare_t* prepare_params;
    dm_api_attrib_t* dm_attrib;
    int iso;
    bool isReCal_;
} DmContext_t;

#endif//__RKAIQ_TYPES_Dm_ALGO_PRVT_H__
