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

#ifndef __RKAIQ_TYPES_Gamma_ALGO_PRVT_H__
#define __RKAIQ_TYPES_Gamma_ALGO_PRVT_H__

/**
 * @file gamma_types_prvt.h
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
 * @defgroup Gamma Auto gamma
 * @{
 *
 */
#include "include/gamma_algo_api.h"
#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"
#include "xcam_log.h"

#define ISP3X_SEGNUM_LOG_49     (2)
#define ISP3X_SEGNUM_LOG_45     (0)
#define ISP3X_SEGNUM_EQU_45     (1)

typedef struct GammaContext_s {
    const RkAiqAlgoCom_prepare_t* prepare_params;
    gamma_api_attrib_t* gamma_attrib;
    bool isReCal_;
} GammaContext_t;

#endif//__RKAIQ_TYPES_Gamma_ALGO_PRVT_H__
