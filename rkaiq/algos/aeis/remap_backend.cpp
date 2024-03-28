/*
 * remap_backend.cpp - The backend hardware/software to do remap
 *
 *  Copyright (c) 2021 Rockchip Electronics Co., Ltd.
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
#include "remap_backend.h"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <fstream>
#include <iostream>

#include "dvs_app.h"
#include "xcam_log.h"

namespace RkCam {

constexpr static const uint8_t fec_mesh_skipped             = 3;
constexpr static const uint8_t fec_mesh_hold_by_algo        = 2;
constexpr static const uint8_t fec_hw_mesh_used_by_hardware = 1;
constexpr static const uint8_t fec_mesh_available           = 0;

static void ReadBinary(const std::string& path, void* buf, size_t size) {
    std::ifstream ifs(path, std::ios::binary);
    if (!ifs.is_open()) {
        LOGE_AEIS("Failed to open file %s", path.c_str());
        return;
    } else {
        ifs.read(reinterpret_cast<char*>(buf), size);
    }
}

static void WriteBinary(const std::string& path, void* buf, size_t size) {
    std::ofstream ofs(path, std::ios::binary);
    if (!ofs.is_open()) {
        LOGE_AEIS("Failed to open file %s", path.c_str());
        return;
    } else {
        ofs.write(reinterpret_cast<char*>(buf), size);
    }
}

FecRemapBackend::FecRemapBackend(const FecMeshConfig& config,
                                 const isp_drv_share_mem_ops_t* mem_ops)
    : config_(config),
      mem_ops_(mem_ops),
      user_buffer_index_(-1),
      hw_buffer_index_(-1),
      last_result_id_(-1) {
    assert(mem_ops != nullptr);

    ImportHwBuffers();
}

FecRemapBackend::~FecRemapBackend() { ReleaseHwBuffers(); }

FecMeshBuffer* FecRemapBackend::AllocUserBuffer() {
    FecMeshBuffer* buf = new FecMeshBuffer();
    assert(buf != nullptr);
    buf->Fd      = -1;
    buf->Size    = config_.MeshSize * (sizeof(*buf->MeshXi) + sizeof(*buf->MeshXf) +
                                    sizeof(*buf->MeshYi) + sizeof(*buf->MeshYf));
    buf->UserPtr = calloc(1, buf->Size + 1);
    if (!buf->UserPtr) {
        delete buf;
        return nullptr;
    }
    buf->MeshXi   = reinterpret_cast<unsigned short*>(buf->UserPtr);
    buf->MeshYi   = buf->MeshXi + config_.MeshSize;
    buf->MeshXf   = reinterpret_cast<unsigned char*>(buf->MeshYi + config_.MeshSize);
    buf->MeshYf   = buf->MeshXf + config_.MeshSize;
    buf->State    = reinterpret_cast<char*>(buf->UserPtr) + buf->Size;
    buf->State[0] = fec_mesh_hold_by_algo;
    buf->Index    = ++user_buffer_index_;

    std::unique_lock<std::mutex> lk(user_mtx_);
    user_buffers_.emplace_back(buf);

    return buf;
}

void FecRemapBackend::FreeUserBuffer(FecMeshBuffer* buf) {
    assert(buf != nullptr && buf->Fd == -1);
    std::unique_lock<std::mutex> lk(user_mtx_);
    auto it = std::remove_if(
        user_buffers_.begin(), user_buffers_.end(),
        [&buf](const std::unique_ptr<FecMeshBuffer>& p) { return (buf->Index == p->Index); });
    user_buffers_.erase(it, user_buffers_.end());
}

FecMeshBuffer* FecRemapBackend::GetAvailUserBuffer() {
    std::unique_lock<std::mutex> lk(user_mtx_);
    for (auto it = user_buffers_.begin(); it != user_buffers_.end(); it++) {
        if ((*it)->State[0] == fec_mesh_available) {
            (*it)->State[0] = fec_mesh_hold_by_algo;
            return (*it).get();
        }
    }
    return nullptr;
}

void FecRemapBackend::ImportHwBuffers() {
    assert(mem_ops_ != nullptr);
    rk_aiq_share_mem_config_t hw_config_;
    hw_config_.mem_type                = MEM_TYPE_FEC;
    hw_config_.alloc_param.width       = config_.Width;
    hw_config_.alloc_param.height      = config_.Height;
    hw_config_.alloc_param.reserved[0] = config_.MeshDensity;

    mem_ops_->alloc_mem(0, (void*)mem_ops_, &hw_config_, &mem_ctx_);
}

void FecRemapBackend::ReleaseHwBuffers() {
    if (mem_ctx_ && mem_ops_) mem_ops_->release_mem(0, mem_ctx_);
}

FecMeshBuffer* FecRemapBackend::GetFreeHwBuffer() {
    if (mem_ops_ == nullptr || mem_ctx_ == nullptr) {
        return nullptr;
    }

    const auto mem_info =
        reinterpret_cast<rk_aiq_fec_share_mem_info_t*>(mem_ops_->get_free_item(0, mem_ctx_));
    if (mem_info != nullptr) {
        return new FecMeshBuffer(mem_info);
    }
    return nullptr;
}

void FecRemapBackend::GetMeshFromFile(MeshBuffer* info) {
    FecMeshBuffer* buf = reinterpret_cast<FecMeshBuffer*>(info);
    ReadBinary("/data/meshxi.bin", buf->MeshXi, sizeof(*buf->MeshXi) * config_.MeshSize);
    ReadBinary("/data/meshxf.bin", buf->MeshXf, sizeof(*buf->MeshXf) * config_.MeshSize);
    ReadBinary("/data/meshyi.bin", buf->MeshYi, sizeof(*buf->MeshYi) * config_.MeshSize);
    ReadBinary("/data/meshyf.bin", buf->MeshYf, sizeof(*buf->MeshYf) * config_.MeshSize);
}

void FecRemapBackend::WriteMeshToFile(MeshBuffer* info) {
    FecMeshBuffer* buf = reinterpret_cast<FecMeshBuffer*>(info);
    std::string path   = "/data/dvs_mesh_";
    path.append(std::to_string(buf->Fd));
    std::string pathxi = path;
    pathxi.append("_xi.bin");
    WriteBinary(pathxi, buf->MeshXi, sizeof(*buf->MeshXi) * config_.MeshSize);
    std::string pathxf = path;
    pathxf.append("_xf.bin");
    WriteBinary(pathxf, buf->MeshXf, sizeof(*buf->MeshXf) * config_.MeshSize);
    std::string pathyi = path;
    pathyi.append("_yi.bin");
    WriteBinary(pathyi, buf->MeshYi, sizeof(*buf->MeshYi) * config_.MeshSize);
    std::string pathyf = path;
    pathyf.append("_yf.bin");
    WriteBinary(pathyf, buf->MeshYf, sizeof(*buf->MeshYf) * config_.MeshSize);
}

void FecRemapBackend::Remap(meshxyFEC* mesh) {
    std::unique_lock<std::mutex> lk(user_mtx_);
    auto it = std::find_if(user_buffers_.begin(), user_buffers_.end(),
                           [&mesh](const std::unique_ptr<FecMeshBuffer>& buf) {
                               return buf->Index == (size_t)mesh->mesh_buffer_index;
                           });
    if (it != user_buffers_.end()) {
        (*it)->ImageBufferIndex = mesh->image_buffer_index;
        (*it)->FrameId          = mesh->image_index;
        if (mesh->is_skip) {
            (*it)->State[0] = fec_mesh_skipped;
        } else {
            (*it)->State[0] = fec_hw_mesh_used_by_hardware;
        }
    }
}

// Does not do actual remap, but put result to hardware
void FecRemapBackend::Remap(MeshBuffer* info) {
    std::unique_lock<std::mutex> lk(user_mtx_);
    auto buf      = reinterpret_cast<FecMeshBuffer*>(info);
    buf->State[0] = fec_hw_mesh_used_by_hardware;
}

FecMeshBuffer* FecRemapBackend::GetPendingHwResult() {
    FecMeshBuffer* buf    = nullptr;
    FecMeshBuffer* hw_buf = nullptr;
    uint32_t min_id       = (uint32_t)(-1);
    {
        std::unique_lock<std::mutex> lk(user_mtx_);
        std::for_each(user_buffers_.begin(), user_buffers_.end(),
                      [&](const std::unique_ptr<FecMeshBuffer>& p) {
                          if (p->State[0] == fec_hw_mesh_used_by_hardware ||
                              p->State[0] == fec_mesh_skipped) {
                              if (p->FrameId != (uint32_t)(-1) && p->FrameId <= last_result_id_) {
                                  LOGW_AEIS("Get pending result id %u PASSED !!!", p->FrameId);
                                  p->State[0] = fec_mesh_available;
                              } else if (last_result_id_ != (uint32_t)(-1) && p->FrameId - last_result_id_ > 1) {
                                  LOGV_AEIS("pending result id %u in FUTURE!!!", p->FrameId);
                              } else {
                                  if (min_id >= p->FrameId) {
                                      min_id = p->FrameId;
                                      buf    = p.get();
                                  }
                              }
                          }
                      });
    }
    LOGV_AEIS("Get Pending result min id %u", min_id);

    if (buf != nullptr) {
        if (buf->State[0] == fec_mesh_skipped) {
            LOGW_AEIS("Get pending result id %u SKIPPED ...", buf->FrameId);
            auto* mesh = AllocUserBuffer();
            if (mesh != nullptr) {
                mesh->Fd               = -1;
                mesh->FrameId          = buf->FrameId;
                mesh->ImageBufferIndex = buf->ImageBufferIndex;
                mesh->ImageBufferSize  = buf->ImageBufferSize;
                mesh->State[0]         = fec_hw_mesh_used_by_hardware;
                std::unique_lock<std::mutex> lk(user_mtx_);
                buf->State[0] = fec_mesh_available;
                last_result_id_ = buf->FrameId;
            }
            return mesh;
        } else {
            hw_buf = GetFreeHwBuffer();
            if (hw_buf != nullptr) {
                LOGD_AEIS("Get pending result id %u HW ", buf->FrameId);
                memcpy(hw_buf->MeshXi, buf->MeshXi, (sizeof(*buf->MeshXi) * config_.MeshSize));
                memcpy(hw_buf->MeshYi, buf->MeshYi, (sizeof(*buf->MeshYi) * config_.MeshSize));
                memcpy(hw_buf->MeshXf, buf->MeshXf, (sizeof(*buf->MeshXf) * config_.MeshSize));
                memcpy(hw_buf->MeshYf, buf->MeshYf, (sizeof(*buf->MeshYf) * config_.MeshSize));
                hw_buf->FrameId          = buf->FrameId;
                hw_buf->ImageBufferIndex = buf->ImageBufferIndex;
                hw_buf->ImageBufferSize  = buf->ImageBufferSize;
                hw_buf->State[0]         = fec_hw_mesh_used_by_hardware;
                std::unique_lock<std::mutex> lk(user_mtx_);
                buf->State[0]   = fec_mesh_available;
                last_result_id_ = buf->FrameId;
            } else {
                LOGW_AEIS("Get pending result id %u HW no buffer", buf->FrameId);
            }
            return hw_buf;
        }
    }

    return nullptr;
}

}  // namespace RkCam
