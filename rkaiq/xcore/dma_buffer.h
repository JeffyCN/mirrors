/*
 * dma_buffer.h - DMA Buffer Implementation
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
#ifndef _DMA_BUFFER_H_
#define _DMA_BUFFER_H_

#include <memory>

#include "unique_fd.h"
#include "xcam_common.h"

namespace XCam {

enum class DmaBufferDirection { kBidirectional, kDeviceToCPU, kCPUToDevice, kNone };

class DmaBuffer {
 public:
    DmaBuffer(int fd, size_t size);
    virtual ~DmaBuffer();
    DmaBuffer(const DmaBuffer&) = delete;
    DmaBuffer& operator=(const DmaBuffer&) = delete;

    static XCamReturn sync(int fd, DmaBufferDirection direction, bool start);

    XCamReturn beginCpuAccess(DmaBufferDirection direction);
    XCamReturn endCpuAccess(DmaBufferDirection direction);

    void* map();
    void unmap();
    int getFd();
    size_t getSize();
    int release();
    bool mapped();

 private:

    UniqueFd fd_;
    size_t size_;
    void* ptr_;
    // TODO(Cody): modern implement of dma_buf
    // does not need name to import/export?
    // std::string name_;
};

}  // namespace XCam

#endif  // _DMA_BUFFER_H_

