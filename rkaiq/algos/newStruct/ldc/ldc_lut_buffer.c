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

#include "algos/newStruct/ldc/include/ldc_lut_buffer.h"

#include "common/rk_aiq_types_priv_c.h"

/*
 *
 * L D C   L U T   B U F F E R   A P I
 *
 */

void LdcLutBuffer_init(LdcLutBuffer* b, LdcLutBufferConfig config,
                       const rk_aiq_lut_share_mem_info_t* mem_info) {
    b->Config = config;
    b->State  = (LdcLutBufferState)(*mem_info->state);
    b->Fd     = mem_info->fd;
    b->Addr   = mem_info->addr;
    b->Size   = mem_info->size;
}

/*
 *
 * L D C   L U T   B U F F E R   M A N A G E R   A P I
 *
 */

void LdcLutBufMng_releaseHwBuffers(LdcLutBufferManager* m, uint8_t isp_id) {
    if (m->mem_ctx_ != NULL && m->mem_ops_ != NULL) m->mem_ops_->release_mem(isp_id, m->mem_ctx_);
}

XCamReturn LdcLutBufMng_getFreeHwBuffer(LdcLutBufferManager* m, uint8_t isp_id, LdcLutBuffer* buf) {
    if (m->mem_ops_ == NULL || m->mem_ctx_ == NULL) return XCAM_RETURN_ERROR_PARAM;

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    rk_aiq_lut_share_mem_info_t* mem_info =
        (rk_aiq_lut_share_mem_info_t*)(m->mem_ops_->get_free_item(isp_id, m->mem_ctx_));
    if (mem_info != NULL) {
        *mem_info->state      = MESH_BUF_WAIT2CHIP;
        LdcLutBuffer_init(buf, m->config_, mem_info);
    } else {
        LOGE_ALDC("Can't get free lut buf for LDC");
        ret = XCAM_RETURN_ERROR_MEM;
    }

    return ret;
}

void LdcLutBufMng_init(LdcLutBufferManager* m, LdcLutBufferConfig config,
                       const isp_drv_share_mem_ops_t* mem_ops) {
    m->mem_ops_ = mem_ops;
    m->mem_ctx_ = NULL;
    m->config_  = config;
}

void LdcLutBufMng_deinit(LdcLutBufferManager* m) { LdcLutBufMng_releaseHwBuffers(m, 0); }

void LdcLutBufMng_importHwBuffers(LdcLutBufferManager* m, uint8_t isp_id, int32_t mem_type) {
    assert(m->mem_ops_ != NULL);
    rk_aiq_share_mem_config_t hw_config;
    hw_config.mem_type                = (rk_aiq_drv_share_mem_type_t)(mem_type);
    hw_config.alloc_param.width       = m->config_.Width;
    hw_config.alloc_param.height      = m->config_.Height;
    hw_config.alloc_param.reserved[0] = 1;

    m->mem_ops_->alloc_mem(isp_id, (void*)(m->mem_ops_), &hw_config, &m->mem_ctx_);
}
