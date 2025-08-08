/*
 * drm_buffer.cpp - DRM Buffer Implementation
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
#include "drm_buffer.h"

#include <drm/drm.h>
#include <sys/mman.h>
#include <xf86drm.h>

#include <memory>
#include <string>

#include "dma_buffer.h"
#include "drm_device.h"
#include "xcam_defs.h"
#include "xcam_log.h"

namespace XCam {

DrmBuffer::~DrmBuffer() {
    for (auto it = dma_bufs_.begin(); it != dma_bufs_.end();) {
        dma_bufs_.erase(it);
    }
    auto dev = drm_device_.lock();
    if (dev != nullptr) {
        dev->DestroyDumbObject(dumb_object_);
    }
}

DrmBuffer::DrmBuffer(const std::shared_ptr<DrmDevice>& dev,
                     std::unique_ptr<DrmDumbObject> dumb_object)
    : drm_device_(std::weak_ptr<DrmDevice>(dev)) {
    dumb_object_ = std::move(dumb_object);
    for (int i = 0; i < dumb_object_->num_planes; i++) {
        dma_bufs_.push_back(std::unique_ptr<DmaBuffer>(
            new DmaBuffer(dumb_object_->fds[i], dumb_object_->sizes[i])));
    }
}

int DrmBuffer::numPlanes() { return dma_bufs_.size(); }

uint8_t* DrmBuffer::map(unsigned int plane) {
    if (plane >= dma_bufs_.size()) {
        return nullptr;
    }
    auto&& buf = dma_bufs_.at(plane);
    if (!buf->mapped()) {
        auto dev = drm_device_.lock();
        dev->RequestMapDumbObject(dumb_object_, plane);
    }
    auto ptr = reinterpret_cast<uint8_t*>(buf->map());
    buf->beginCpuAccess(DmaBufferDirection::kBidirectional);
    return ptr;
}

uint8_t* DrmBuffer::map() { return map(0); }

bool DrmBuffer::unmap(unsigned int plane) {
    if (plane >= dma_bufs_.size()) {
        return false;
    }
    auto&& buf = dma_bufs_.at(plane);
    buf->endCpuAccess(DmaBufferDirection::kBidirectional);
    buf->unmap();
    return true;
}

bool DrmBuffer::unmap() { return unmap(0); }

int DrmBuffer::getFd(unsigned int plane) {
    if (plane >= dma_bufs_.size()) {
        return -1;
    }
    return dma_bufs_.at(plane)->getFd();
}

size_t DrmBuffer::getSize() { return getSize(0); }

size_t DrmBuffer::getSize(unsigned int plane) { return dma_bufs_.at(plane)->getSize(); }

int DrmBuffer::get_fd() { return getFd(0); }

DrmDumbObject* DrmBuffer::get_bo() { return dumb_object_.get(); }

DrmBufferProxy::DrmBufferProxy(const VideoBufferInfo& info, const SmartPtr<DrmBuffer>& data)
    : BufferProxy(info, data) {
    XCAM_ASSERT(data.ptr());
}

DrmDumbObject* DrmBufferProxy::get_bo() {
    auto data   = get_buffer_data();
    auto buffer = data.dynamic_cast_ptr<DrmBuffer>();

    XCAM_FAIL_RETURN(WARNING, buffer.ptr(), NULL, "DrmBuffer get_buffer_data failed with NULL");

    return buffer->get_bo();
}

int DrmBufferProxy::GetFd() {
    auto data   = get_buffer_data();
    auto buffer = data.dynamic_cast_ptr<DrmBuffer>();

    return buffer->get_fd();
}

DrmBufferPool::DrmBufferPool(std::shared_ptr<DrmDevice> device) : drm_device_(device) {}

bool DrmBufferPool::fixate_video_info(VideoBufferInfo& info) {
    VideoBufferInfo out_info;

    out_info.init(info.format, info.width, info.height, info.aligned_width, info.aligned_height);

    info = out_info;

    return true;
}

SmartPtr<BufferData> DrmBufferPool::allocate_data(const VideoBufferInfo& buffer_info) {
    SmartPtr<DrmBuffer> buffer_data;
    auto bo = drm_device_->CreateDumbObject(buffer_info.aligned_width, buffer_info.aligned_height,
                                            buffer_info.color_bits, 1);
    if (bo != nullptr) {
        buffer_data = new DrmBuffer(drm_device_, std::move(bo));
    }
    return buffer_data;
}

SmartPtr<BufferProxy> DrmBufferPool::create_buffer_from_data(SmartPtr<BufferData>& data) {
    const VideoBufferInfo& info     = get_video_info();
    SmartPtr<DrmBuffer> buffer_data = data.dynamic_cast_ptr<DrmBuffer>();
    XCAM_ASSERT(buffer_data.ptr());

    SmartPtr<DrmBufferProxy> out_buf = new DrmBufferProxy(info, buffer_data);
    XCAM_ASSERT(out_buf.ptr());
    return out_buf;
}

}  // namespace XCam
