/*
* cac_types_prvt.h

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

#ifndef __RKAIQ_TYPES_CAC_ALGO_PRVT_H__
#define __RKAIQ_TYPES_CAC_ALGO_PRVT_H__

#include "include/cac_algo_api.h"
#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"
#include "xcam_log.h"

#define DEFAULT_RECALCULATE_DELTA_ISO (0.01)
#define RKCAC_MAX_PATH_LEN       255
#define RKCAC_STRENGTH_FIX_BITS    7
#define RKCAC_EDGE_DETECT_FIX_BITS 4

#define INTERP_CAC(x0, x1, ratio) ((ratio) * ((x1) - (x0)) + (x0))

typedef struct CacContext_s {
    const RkAiqAlgoCom_prepare_t* prepare_params;
    cac_api_attrib_t* cac_attrib;
    int iso;
    bool isReCal_;
} CacContext_t;

XCamReturn CacSelectParam(CacContext_t* pCacCtx, cac_param_t* out, int iso);
#endif//__RKAIQ_TYPES_CAC_ALGO_PRVT_H__