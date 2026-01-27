/*
 * ldc_types_prvt.h
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

#ifndef __RKAIQ_TYPES_LDC_ALGO_PRVT_H__
#define __RKAIQ_TYPES_LDC_ALGO_PRVT_H__

#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"
#include "include/algos/rk_aiq_api_types_ldc.h"

typedef struct LdcContext_s {
    const RkAiqAlgoCom_prepare_t* prepare_params;
    ldc_api_attrib_t* ldc_attrib;
    void* handler;
} LdcContext_t;

#endif  //__RKAIQ_TYPES_lDC_ALGO_PRVT_H__
