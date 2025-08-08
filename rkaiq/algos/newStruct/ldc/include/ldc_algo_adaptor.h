/*
 * ldc_algo_adaptor.h
 *
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
#ifndef ALGOS_LDC_ALGO_H
#define ALGOS_LDC_ALGO_H

#include "algos/algo_types_priv.h"
#include "algos/newStruct/ldc/include/ldc_gen_mesh.h"
#include "algos/newStruct/ldc/include/ldc_ldch_adaptee.h"
#include "algos/newStruct/ldc/include/ldc_ldcv_adaptee.h"
#include "algos/newStruct/ldc/include/ldc_lut_buffer.h"
#include "algos/rk_aiq_algo_des.h"
#include "common/rk_aiq_types_priv_c.h"
#include "iq_parser_v2/RkAiqCalibDbTypesV2.h"
#include "xcore/base/xcam_common.h"

typedef enum LdcModType_e {
    kLdch = 0,
    kLdcv = 1,
} LdcModType;

typedef struct LdcAlgoAdaptor_s {
    bool enable_;
    bool ldch_en_;
    bool ldcv_en_;
    bool started_;
    bool valid_;

    LdchAdaptee* ldch_adaptee_;
    LdcvAdaptee* ldcv_adaptee_;

    uint8_t correct_level_;
    LdcAlgoGenMesh* gen_mesh_;
    LdcGenMeshHelperThd_t* gen_mesh_helper_;
    GenMeshCb gen_mesh_cb_;
    ldc_api_attrib_t* calib;
    int32_t width;
    int32_t height;
} LdcAlgoAdaptor;

static inline bool LdcAlgoAdaptor_isEnabled(LdcAlgoAdaptor* ldc) { return ldc->enable_; }

static inline bool LdcAlgoAdaptor_IsStarted(LdcAlgoAdaptor* ldc) { return ldc->started_; }

void LdcAlgoAdaptor_init(LdcAlgoAdaptor* adaptor);

void LdcAlgoAdaptor_deinit(LdcAlgoAdaptor* adaptor);

XCamReturn LdcAlgoAdaptor_config(LdcAlgoAdaptor* ldc, const ldc_api_attrib_t* calib);

XCamReturn LdcAlgoAdaptor_prepare(LdcAlgoAdaptor* ldc, const RkAiqAlgoConfigLdc* config);

void LdcAlgoAdaptor_onFrameEvent(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams);

LdcLutBuffer* LdcAlgoAdaptor_getFreeLutBuf(LdcAlgoAdaptor* adp, LdcModType type, int8_t isp_id);

#endif  // ALGOS_LDC_ALGO_H
