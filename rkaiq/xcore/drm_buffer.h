/*
 * DrmBuffer.h - DRM Buffer Implementation
 *
 *  Copyright (c) 2021 Rockchip Corporation
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
#ifndef _DRM_BUFFER_H_
#define _DRM_BUFFER_H_

extern "C" {
#include <drm/drm_fourcc.h>
}
#include <sys/types.h>

#include <memory>
#include <vector>

#include "buffer_pool.h"

extern "C" {
struct DrmDumbObject;
}

namespace XCam {

class DmaBuffer;
class DrmDevice;
class DrmBufferProxy;
class DrmBufferPool;

class DrmBuffer : public BufferData {
 public:
    DrmBuffer()                 = delete;
    DrmBuffer(const DrmBuffer&) = delete;
    DrmBuffer& operator=(const DrmBuffer&) = delete;
    DrmBuffer(const std::shared_ptr<DrmDevice>& dev, std::unique_ptr<DrmDumbObject> dumb_object);
    virtual ~DrmBuffer();

    virtual uint8_t* map();
    virtual bool unmap();
    virtual int get_fd();
    size_t getSize();
    // default single plane
    uint8_t* map(unsigned int plane);
    bool unmap(unsigned int plane);
    int getFd(unsigned int plane);
    size_t getSize(unsigned int plane);
    int numPlanes();
    DrmDumbObject* get_bo();

 private:
    std::weak_ptr<DrmDevice> drm_device_;
    std::unique_ptr<DrmDumbObject> dumb_object_;
    std::vector<std::unique_ptr<DmaBuffer>> dma_bufs_;
};

class DrmBufferPool : public BufferPool {
    friend class DrmBuffer;

 public:
    explicit DrmBufferPool(std::shared_ptr<DrmDevice> device);
    virtual ~DrmBufferPool() = default;
    DrmBufferPool(const DrmBufferPool&) = delete;
    DrmBufferPool& operator=(const DrmBufferPool&) = delete;

 protected:
    virtual bool fixate_video_info (VideoBufferInfo &info);
    virtual SmartPtr<BufferData> allocate_data(const VideoBufferInfo& buffer_info);
    virtual SmartPtr<BufferProxy> create_buffer_from_data(SmartPtr<BufferData>& data);

 private:
    std::shared_ptr<DrmDevice> drm_device_;
};

class DrmBufferProxy : public virtual BufferProxy {
    friend class DrmBufferPool;
    friend class DrmDevice;

 public:
    explicit DrmBufferProxy(const VideoBufferInfo& info, const SmartPtr<DrmBuffer>& data);
    virtual ~DrmBufferProxy() = default;
    DrmDumbObject* get_bo();

    int GetFd();
};

}  // namespace XCam

#endif  // _DRM_BUFFER_H_

