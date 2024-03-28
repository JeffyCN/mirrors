/*
 * dma_buffer.cpp - DMA Buffer Implementation
 *
 *  Copyright (c) 2021 Rockchip Electronics Co., Ltd
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
#include "dma_buffer.h"

#include <fcntl.h>
#include <linux/dma-buf.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include <cassert>

#include "xcam_log.h"
#include "xcam_std.h"

namespace XCam {

DmaBuffer::DmaBuffer(int fd, size_t size) : fd_(fd), size_(size), ptr_(nullptr) {}

DmaBuffer::~DmaBuffer() {
    if (ptr_ != nullptr) {
        unmap();
    }
}

XCamReturn DmaBuffer::sync(int fd, DmaBufferDirection direction, bool start) {
    int ret;
    struct dma_buf_sync sync = {0};

    XCAM_ASSERT(fd >= 0);

    if (direction == DmaBufferDirection::kDeviceToCPU) {
        sync.flags = DMA_BUF_SYNC_READ;
    } else if (direction == DmaBufferDirection::kCPUToDevice) {
        sync.flags = DMA_BUF_SYNC_WRITE;
    } else {
        sync.flags = DMA_BUF_SYNC_RW;
    }

    if (start) {
        sync.flags |= DMA_BUF_SYNC_START;
    } else {
        sync.flags |= DMA_BUF_SYNC_END;
    }

    ret = ioctl(fd, DMA_BUF_IOCTL_SYNC, &sync);
    if (ret) {
        LOGE("DMA_BUF_IOCTL_SYNC ioctl failed %s", strerror(errno));
        return XCAM_RETURN_ERROR_IOCTL;
    }

    LOGI("%s CPU access dir %d for BO fd %d", start ? "start" : "end", direction, fd);
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn DmaBuffer::beginCpuAccess(DmaBufferDirection direction) {
    XCAM_ASSERT(ptr_ != nullptr);
    return sync(fd_.Get(), direction, true);
}

XCamReturn DmaBuffer::endCpuAccess(DmaBufferDirection direction) {
    XCAM_ASSERT(ptr_ != nullptr);
    return sync(fd_.Get(), direction, false);
}

void* DmaBuffer::map() {
    assert(((void)"could not map invalid dma_buf", fd_.Get() > 0 && ptr_ == nullptr && size_ > 0));

    ptr_ = mmap(0, size_, PROT_READ | PROT_WRITE, MAP_SHARED, fd_.Get(), 0);
    if (ptr_ == MAP_FAILED) {
        LOGE("dma_buf map failed %s", strerror(errno));
        return nullptr;
    }

    return ptr_;
}

void DmaBuffer::unmap() {
    assert(((void)"unmap dma_buf in wrong state", fd_.Get() > 0 && ptr_ != nullptr));

    munmap(ptr_, size_);
    ptr_ = nullptr;
}

int DmaBuffer::getFd() { return fd_.Get(); }

size_t DmaBuffer::getSize() { return size_; }

int DmaBuffer::release() { return fd_.Release(); }

bool DmaBuffer::mapped() { return ptr_ == nullptr ? false : true; }

}  // namespace XCam
