/*
 *  Copyright (c) 2022 Rockchip Electronics Co.,Ltd
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
 * Author: Cody Xie <cody.xie@rock-chips.com>
 */
#ifndef ALGOS_ACAC_LUT_BUFFER_H
#define ALGOS_ACAC_LUT_BUFFER_H

#include <cstdint>

#include "common/rk_aiq_types_priv.h"

namespace RkCam {

enum class LutBufferState {
    kInitial   = 0,
    kWait2Chip = 1,
    kChipInUse = 2,
};

struct LutBufferConfig {
    bool IsBigMode;
    uint32_t Width;
    uint32_t Height;
    uint32_t LutHCount;
    uint32_t LutVCount;
    uint16_t ScaleFactor;
    uint16_t PsfCfgCount;
};

struct LutBuffer {
    LutBuffer() = delete;
    explicit LutBuffer(const LutBufferConfig& config)
        : State(LutBufferState::kInitial), Config(config), Fd(-1), Size(0), Addr(nullptr) {}
    LutBuffer(const LutBufferConfig& config, const rk_aiq_cac_share_mem_info_t* mem_info)
        : Config(config) {
        State = static_cast<LutBufferState>(*mem_info->state);
        Fd    = mem_info->fd;
        Addr  = mem_info->addr;
        Size  = mem_info->size;
    }
    LutBuffer(const LutBuffer&) = delete;
    LutBuffer& operator=(const LutBuffer&) = delete;

    LutBufferState State;
    LutBufferConfig Config;
    int Fd;
    int Size;
    void* Addr;
};

class LutBufferManager {
 public:
    LutBufferManager() = delete;
    LutBufferManager(const LutBufferConfig& config, const isp_drv_share_mem_ops_t* mem_ops)
        : mem_ops_(mem_ops), mem_ctx_(nullptr), config_(config) {}
    LutBufferManager(const LutBufferManager&) = delete;
    LutBufferManager& operator=(const LutBufferManager&) = delete;
    ~LutBufferManager() {
        // TODO(Cody)
        ReleaseHwBuffers(0);
        ReleaseHwBuffers(1);
    }

    void ImportHwBuffers(uint8_t isp_id) {
        assert(mem_ops_ != nullptr);
        rk_aiq_share_mem_config_t hw_config;
        hw_config.mem_type           = MEM_TYPE_CAC;
        hw_config.alloc_param.width  = config_.Width;
        hw_config.alloc_param.height = config_.Height;
        hw_config.alloc_param.reserved[0] = 1;

        mem_ops_->alloc_mem(isp_id, (void*)(mem_ops_), &hw_config, &mem_ctx_);
    }

    void ReleaseHwBuffers(uint8_t isp_id) {
        if (mem_ctx_ != nullptr && mem_ops_ != nullptr) mem_ops_->release_mem(isp_id, mem_ctx_);
    }

    LutBuffer* GetFreeHwBuffer(uint8_t isp_id) {
        if (mem_ops_ == nullptr || mem_ctx_ == nullptr) {
            return nullptr;
        }

        const auto* mem_info = static_cast<const rk_aiq_cac_share_mem_info_t*>(
            mem_ops_->get_free_item(isp_id, mem_ctx_));
        if (mem_info != nullptr) {
            auto* lut_buf = new LutBuffer(config_, mem_info);
            if (lut_buf != nullptr) {
                return lut_buf;
            }
        }
        return nullptr;
    }

 private:
    const isp_drv_share_mem_ops_t* mem_ops_;
    void* mem_ctx_;
    LutBufferConfig config_;
};

}  // namespace RkCam

#endif  // ALGOS_ACAC_LUT_BUFFER_H
