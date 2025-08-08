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

#ifndef ALGOS_ALDC_LUT_BUFFER_H
#define ALGOS_ALDC_LUT_BUFFER_H

#include <stdint.h>

#include "xcore/base/xcam_common.h"

#define LDC_BUF_FD_DEFAULT (-1)

typedef struct isp_drv_share_mem_ops_s isp_drv_share_mem_ops_t;
typedef struct rk_aiq_lut_share_mem_info_s rk_aiq_lut_share_mem_info_t;

typedef enum LdcLutBufferState_e {
    kLdcLutBufInitial   = 0,
    kLdcLutBufWait2Chip = 1,
    kLdcLutBufChipInUse = 2,
} LdcLutBufferState;

typedef struct LdcLutBufferConfig_s {
    uint32_t Width;
    uint32_t Height;
    uint32_t LutHCount;
    uint32_t LutVCount;
    uint32_t LutHStep;
    uint32_t LutVStep;
} LdcLutBufferConfig;

typedef struct LdcLutBuffer_s {
    LdcLutBufferState State;
    LdcLutBufferConfig Config;
    int Fd;
    int Size;
    void* Addr;
    int32_t mem_type;
} LdcLutBuffer;

void LdcLutBuffer_init(LdcLutBuffer* b, LdcLutBufferConfig config,
                       const rk_aiq_lut_share_mem_info_t* mem_info);

typedef struct LdcLutBufferManager_s {
    const isp_drv_share_mem_ops_t* mem_ops_;
    void* mem_ctx_;
    LdcLutBufferConfig config_;
} LdcLutBufferManager;

void LdcLutBufMng_init(LdcLutBufferManager* m, LdcLutBufferConfig config,
                       const isp_drv_share_mem_ops_t* mem_ops);
void LdcLutBufMng_deinit(LdcLutBufferManager* m);
void LdcLutBufMng_importHwBuffers(LdcLutBufferManager* m, uint8_t isp_id, int32_t mem_type);
void LdcLutBufMng_releaseHwBuffers(LdcLutBufferManager* m, uint8_t isp_id);
XCamReturn LdcLutBufMng_getFreeHwBuffer(LdcLutBufferManager* m, uint8_t isp_id, LdcLutBuffer* buf);

#endif  // ALGOS_ALDC_LUT_BUFFER_H
