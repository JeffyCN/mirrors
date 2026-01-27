/*
 *  ldc_ldch_adaptee.h
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

#ifndef _RK_AIQ_ALGO_LDC_LDCH_H_
#define _RK_AIQ_ALGO_LDC_LDCH_H_

#include "algos/algo_types_priv.h"
#include "algos/newStruct/ldc/include/ldc_gen_mesh.h"
#include "algos/newStruct/ldc/include/ldc_lut_buffer.h"
#include "algos/newStruct/ldc/ldc_types_prvt.h"

typedef struct LdchAdaptee_s {
    bool enable_;
    bool started_;
    bool valid_;
    bool isReCal_;
    uint8_t correct_level_;

    AiqMutex_t _mutex;
    LdcGenMeshState gen_mesh_state_;

    const ldc_api_attrib_t* calib_;
    const RkAiqAlgoConfigLdc* config_;
    LdcLutBufferManager* lut_manger_;
    LdcLutBuffer cached_lut_[2];
    LdcMeshBufInfo_t chip_in_use_lut[2];
} LdchAdaptee;

static inline bool LdchAdaptee_isEnabled(LdchAdaptee* ldch) { return ldch->enable_; }

static inline bool LdchAdaptee_isStarted(LdchAdaptee* ldch) { return ldch->started_; }

static inline uint8_t LdchAdaptee_getCorrectLevel(LdchAdaptee* ldch) {
    return ldch->correct_level_;
}

static inline void LdchAdaptee_setGenMeshState(LdchAdaptee* ldch, LdcGenMeshState state) {
    aiqMutex_lock(&ldch->_mutex);
    ldch->gen_mesh_state_ = state;
    aiqMutex_unlock(&ldch->_mutex);
}

LdcLutBuffer* LdchAdaptee_getFreeLutBuf(LdchAdaptee* ldch, int8_t isp_id);

XCamReturn LdchAdaptee_updateMesh(LdchAdaptee* ldch, uint8_t level, LdcMeshBufInfo_t buf_info);

void LdchAdaptee_init(LdchAdaptee* ldch);

void LdchAdaptee_deinit(LdchAdaptee* ldch);

XCamReturn LdchAdaptee_config(LdchAdaptee* ldch, const ldc_api_attrib_t* calib);

XCamReturn LdchAdaptee_prepare(LdchAdaptee* ldch, const RkAiqAlgoConfigLdc* config);

void LdchAdaptee_onFrameEvent(LdchAdaptee* ldch, const RkAiqAlgoCom* inparams,
                              RkAiqAlgoResCom* outparams);

#endif  // _RK_AIQ_ALGO_LDC_LDCH_H_
