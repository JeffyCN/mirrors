/*
 * ldc_ldcv_adaptee.h
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

#ifndef _RK_AIQ_ALGO_LDC_LDCV_H_
#define _RK_AIQ_ALGO_LDC_LDCV_H_

#include "algos/algo_types_priv.h"
#include "algos/newStruct/ldc/include/ldc_gen_mesh.h"
#include "algos/newStruct/ldc/include/ldc_lut_buffer.h"
#include "algos/newStruct/ldc/ldc_types_prvt.h"

typedef struct LdcvAdaptee_s {
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
} LdcvAdaptee;

static inline bool LdcvAdaptee_isEnabled(LdcvAdaptee* ldcv) { return ldcv->enable_; }

static inline bool LdcvAdaptee_isStarted(LdcvAdaptee* ldcv) { return ldcv->started_; }

static inline uint8_t LdcvAdaptee_getCorrectLevel(LdcvAdaptee* ldcv) {
    return ldcv->correct_level_;
}

static inline void LdcvAdaptee_setGenMeshState(LdcvAdaptee* ldcv, LdcGenMeshState state) {
    aiqMutex_lock(&ldcv->_mutex);
    ldcv->gen_mesh_state_ = state;
    aiqMutex_unlock(&ldcv->_mutex);
}

const LdcLutBuffer* LdcvAdaptee_getFreeLutBuf(LdcvAdaptee* ldcv, int8_t isp_id);

XCamReturn LdcvAdaptee_updateMesh(LdcvAdaptee* ldcv, uint8_t level, LdcMeshBufInfo_t buf_info);

void LdcvAdaptee_init(LdcvAdaptee* ldcv);

void LdcvAdaptee_deinit(LdcvAdaptee* ldcv);

XCamReturn LdcvAdaptee_config(LdcvAdaptee* ldcv, const ldc_api_attrib_t* calib);

XCamReturn LdcvAdaptee_prepare(LdcvAdaptee* ldcv, const RkAiqAlgoConfigLdc* config);

void LdcvAdaptee_onFrameEvent(LdcvAdaptee* ldcv, const RkAiqAlgoCom* inparams,
                              RkAiqAlgoResCom* outparams);

#endif  // _RK_AIQ_ALGO_LDC_LDCV_H_
