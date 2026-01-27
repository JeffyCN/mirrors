/*
 * dma_video_buffer.cpp - dma buffer
 *
 *  Copyright (c) 2016 Intel Corporation
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
 * Author: Wind Yuan <feng.yuan@intel.com>
 * Author: Cody Xie <cody.xie@rock-chips.com>
 */

#include "dma_video_buffer.h"

#include "xcam_log.h"

namespace XCam {

DmaVideoBuffer::DmaVideoBuffer(const VideoBufferInfo& info, int dma_fd, bool need_close_fd)
    : DmaBuffer(dma_fd, info.size), VideoBuffer(info), _need_close_fd(need_close_fd) {
    XCAM_ASSERT(dma_fd >= 0);
    _dma_fd = dma_fd;
}

DmaVideoBuffer::~DmaVideoBuffer() {
    if (!_need_close_fd) (void)(DmaBuffer::release());
}

uint8_t* DmaVideoBuffer::map() { return static_cast<uint8_t*>(DmaBuffer::map()); }

bool DmaVideoBuffer::unmap() {
    DmaBuffer::unmap();
    return true;
}

int DmaVideoBuffer::get_fd() { return DmaBuffer::getFd(); }

}  // namespace XCam
