/*
* dpc_types_prvt.h

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

#ifndef __RKAIQ_TYPES_DPC_ALGO_PRVT_H__
#define __RKAIQ_TYPES_DPC_ALGO_PRVT_H__

#include "include/dpc_algo_api.h"
#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"
#include "xcam_log.h"

#define DEFAULT_RECALCULATE_DELTA_ISO (0.01)
#define LIMIT_VALUE(value,max_value,min_value)      (value > max_value? max_value : value < min_value ? min_value : value)
#define FASTMODELEVELMAX     (10)
#define FASTMODELEVELMIN     (1)

typedef struct DpcContext_s {
    const RkAiqAlgoCom_prepare_t* prepare_params;
    dpc_api_attrib_t* dpc_attrib;
    int iso;
    bool isReCal_;
    bool isBlackSensor;
} DpcContext_t;

XCamReturn DpcSelectParam(DpcContext_t* pDpcCtx, dpc_param_t* out, int iso);
#endif//__RKAIQ_TYPES_DPC_ALGO_PRVT_H__