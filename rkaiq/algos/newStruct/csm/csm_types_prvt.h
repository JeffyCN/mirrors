/*
* csm_types_prvt.h

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

#ifndef __RKAIQ_TYPES_CSM_ALGO_PRVT_H__
#define __RKAIQ_TYPES_CSM_ALGO_PRVT_H__

/**
 * @file csm_types_prvt.h
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
 * @defgroup Csm Auto
 * @{
 *
 */
#include "include/csm_algo_api.h"
#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"
#include "xcam_log.h"

#define LOG2(x)                             (log((double)x) / log((double)2))

#define DEFAULT_RECALCULATE_DELTA_ISO (0.01)

typedef struct CsmContext_s {
    csm_api_attrib_t* csm_attrib;
    int iso;
    bool isReCal_;
} CsmContext_t;

#endif//__RKAIQ_TYPES_CSM_ALGO_PRVT_H__
