/*
 *  Copyright (c) 2024 Rockchip Electronics Co.,Ltd
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

#ifndef _LDC_ALOG_API_H_
#define _LDC_ALOG_API_H_

#include "algos/newStruct/ldc/include/ldc_lut_buffer.h"
#include "algos/rk_aiq_api_types_ldc.h"
#include "isp/rk_aiq_isp_ldc22.h"
#include "rk_aiq_algo_des.h"

extern RkAiqAlgoDescription g_RkIspAlgoDescLdc;

typedef enum {
    kAiqLdcUpdMeshAuto = 0,    // auto generate mesh inside ldc algo
    kAiqLdcUpdMeshFromExtBuf,  // import mesh from extern buffer
    kAiqLdcUpdMeshFromFile,    // import mesh from file
} AiqLdcUpdMeshMode;

LdcLutBuffer* algo_ldc_getLdchFreeLutBuf(RkAiqAlgoContext* ctx);
#if RKAIQ_HAVE_LDCV
LdcLutBuffer* algo_ldc_getLdcvFreeLutBuf(RkAiqAlgoContext* ctx);
#endif

#endif  // _LDC_ALOG_API_H_
