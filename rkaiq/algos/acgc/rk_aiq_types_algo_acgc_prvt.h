/*
 *rk_aiq_types_acgc_algo_prvt.h
 *
 *  Copyright (c) 2022 Rockchip Corporation
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
#ifndef __RKAIQ_TYPES_ALGO_ACGC_PRVT_H__
#define __RKAIQ_TYPES_ALGO_ACGC_PRVT_H__

#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"
#include "base/xcam_common.h"
#include "rk_aiq_types_acgc_algo.h"
#include "xcam_log.h"

typedef struct AcgcContext_s {
    CamCalibDbV2Context_t* calibv2;
    rk_aiq_acgc_params_t params;
    bool isReCal_ = true;
} AcgcContext_t;

typedef struct _RkAiqAlgoContext {
    AcgcContext_t acgcCtx;
} RkAiqAlgoContext;

#endif  //__RKAIQ_TYPES_ALGO_ACGC_PRVT_H__
