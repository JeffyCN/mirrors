/*
 * Copyright (c) 2019-2022 Rockchip Eletronics Co., Ltd.
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
 */
#ifndef __RKAIQ_TYPES_ALGO_AGIC_PRVT_H__
#define __RKAIQ_TYPES_ALGO_AGIC_PRVT_H__

#include "algos/agic/rk_aiq_types_algo_agic_int.h"
#include "iq_parser_v2/RkAiqCalibDbTypesV2.h"
#include "iq_parser_v2/RkAiqCalibDbV2Helper.h"
#include "xcore/base/xcam_common.h"
#include "xcore/base/xcam_log.h"

#define RKAIQ_GIC_BITS (12)

#define GIC_SWAP(_T_, A, B) \
    {                       \
        _T_ tmp = (A);      \
        (A)     = (B);      \
        (B)     = tmp;      \
    }

typedef enum AgicState_e {
    AGIC_STATE_INVALID     = 0,
    AGIC_STATE_INITIALIZED = 1,
    AGIC_STATE_STOPPED     = 2,
    AGIC_STATE_RUNNING     = 3,
    AGIC_STATE_LOCKED      = 4,
    AGIC_STATE_MAX
} AgicState_t;

typedef struct AgicFullParam_s {
    union {
        CalibDbV2_Gic_V20_t* gic_v20;
        CalibDbV2_Gic_V21_t* gic_v21;
    };
} AgicFullParam_t;

typedef struct AgicContext_s {
    AgicConfig_t ConfigData;
    AgicState_t state;
    AgicFullParam_t full_param;
    union {
        rkaiq_gic_v1_api_attr_t v1;
        rkaiq_gic_v2_api_attr_t v2;
    } attr;
    int Gic_Scene_mode;
    int working_mode;
    int last_iso;
    bool calib_changed;
    uint8_t raw_bits;
} AgicContext_t;

typedef struct _RkAiqAlgoContext {
    AgicContext_t agicCtx;
} RkAiqAlgoContext;

XCamReturn AgicInit(AgicContext_t* ppAgicCtx, CamCalibDbV2Context_t* calib);
XCamReturn AgicRelease(AgicContext_t* pAgicCtx);
XCamReturn AgicStop(AgicContext_t* pAgicCtx);
XCamReturn AgicConfig(AgicContext_t* pAgicCtx, AgicConfig_t* pAgicConfig);
XCamReturn AgicReConfig(AgicContext_t* pAgicCtx, AgicConfig_t* pAgicConfig);
XCamReturn AgicPreProcess(AgicContext_t* pAgicCtx);
XCamReturn AgicProcess(AgicContext_t* pAgicCtx, int ISO, int mode);
XCamReturn AgicGetProcResult(AgicContext_t* pAgicCtx, AgicProcResult_t* pAgicRes);
void GicV1CalibToAttr(CalibDbV2_Gic_V20_t* calib, rkaiq_gic_v1_api_attr_t* attr);
void GicV2CalibToAttr(CalibDbV2_Gic_V21_t* calib, rkaiq_gic_v2_api_attr_t* attr);

#endif  //__RKAIQ_TYPES_ALGO_AGIC_PRVT_H__
