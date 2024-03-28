/*
 * remap_backend.h - The backend hardware/software to do remap
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
#ifndef ALGOS_AEIS_REMAP_BACKEND_H
#define ALGOS_AEIS_REMAP_BACKEND_H

#include <cstdint>
#include <memory>
#include <vector>
#include <mutex>

#include "rk_aiq_types_priv.h"

#ifdef OPENCV3_SUPPORT
#include <opencv3/opencv.hpp>
#endif

struct meshxyFEC;

namespace RkCam {

struct MeshBuffer {};

class RemapBackend {
 public:
    RemapBackend()                    = default;
    virtual ~RemapBackend()           = default;
    RemapBackend(const RemapBackend&) = delete;
    const RemapBackend& operator=(const RemapBackend&) = delete;

    virtual void Remap(MeshBuffer* info) = 0;
};

#ifdef OPENCV3_SUPPORT
struct CvMeshBuffer : public MeshBuffer {
    Cv::Mat MapX;
    Cv::Mat MapY;
};

class CvRemapBackend final : public RemapBackend {};
#endif

struct FecMeshConfig {
    int Width;
    int Height;
    unsigned char MeshDensity;
    unsigned long MeshSize;
};

struct FecMeshBuffer final : public MeshBuffer {
    FecMeshBuffer() = default;
    explicit FecMeshBuffer(const rk_aiq_fec_share_mem_info_t* mem)
        : FrameId(-1),
          ImageBufferIndex(-1),
          ImageBufferSize(0),
          Index(-1),
          Size(mem->size),
          Fd(mem->fd),
          UserPtr(mem->map_addr),
          MeshXi(mem->meshxi),
          MeshXf(mem->meshxf),
          MeshYi(mem->meshyi),
          MeshYf(mem->meshyf),
          State(mem->state){};
    virtual ~FecMeshBuffer() {
        // Free user malloced memory
        if (Fd < 0 && UserPtr != nullptr) {
            free(UserPtr);
        }
    }
    FecMeshBuffer(const FecMeshBuffer&) = default;
    FecMeshBuffer& operator=(const FecMeshBuffer&) = default;
    uint32_t FrameId;
    int32_t ImageBufferIndex;
    int32_t ImageBufferSize;
    // Buffer Index
    size_t Index;
    // Total size of UserPtr
    size_t Size;
    // Fd < 0 - if UserPtr valid, means malloced address
    int Fd;
    // Mapped address if it is imported dma_buf
    // Virtual address if it is malloc backed buf
    void* UserPtr;
    // All these are offset of UserPtr
    // Memory layout:
    // | MeshXi | MeshYi | MeshXf | MeshYf
    unsigned short* MeshXi;
    unsigned char* MeshXf;
    unsigned short* MeshYi;
    unsigned char* MeshYf;
    // State[0] = 1 - Used by hardware
    // State[0] = 0 - Used by user
    char* State;
};

class FecRemapBackend final : public RemapBackend {
 public:
    FecRemapBackend(const FecMeshConfig& config, const isp_drv_share_mem_ops_t* mem_ops);
    virtual ~FecRemapBackend();

    FecMeshConfig& GetConfig() { return config_; }

    FecMeshBuffer* AllocUserBuffer();
    void FreeUserBuffer(FecMeshBuffer* buf);
    FecMeshBuffer* GetAvailUserBuffer();

    // Does not do actual remap, but put result to hardware
    void Remap(MeshBuffer* info) override;
    void Remap(struct meshxyFEC* mesh);

    void ImportHwBuffers();
    void ReleaseHwBuffers();
    FecMeshBuffer* GetFreeHwBuffer();
    FecMeshBuffer* GetPendingHwResult();

    // Debug only
    void GetMeshFromFile(MeshBuffer* info);
    void WriteMeshToFile(MeshBuffer* info);

 private:
    FecMeshConfig config_;
    const isp_drv_share_mem_ops_t* mem_ops_;
    void* mem_ctx_;
    std::mutex user_mtx_;
    std::vector<std::unique_ptr<FecMeshBuffer>> user_buffers_;
    size_t user_buffer_index_;
    std::vector<std::unique_ptr<FecMeshBuffer>> hw_buffers_;
    size_t hw_buffer_index_;
    uint32_t last_result_id_;
};

}  // namespace RkCam

#endif  // ALGOS_AEIS_REMAP_BACKEND_H
