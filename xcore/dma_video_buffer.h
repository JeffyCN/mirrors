/*
 * dma_video_buffer.h - dma video buffer
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
#ifndef XCAM_DMA_VIDEO_BUFFER_H
#define XCAM_DMA_VIDEO_BUFFER_H

#include <dma_buffer.h>
#include <unistd.h>
#include <video_buffer.h>
#include <xcam_std.h>

namespace XCam {

class DmaVideoBuffer : virtual public DmaBuffer, public VideoBuffer {
 public:
    DmaVideoBuffer(const VideoBufferInfo& info, int dma_fd, bool need_close_fd = false);
    DmaVideoBuffer(const DmaVideoBuffer&) = delete;
    const DmaVideoBuffer& operator=(const DmaVideoBuffer&) = delete;

    virtual ~DmaVideoBuffer();

    virtual uint8_t* map();
    virtual bool unmap();
    virtual int get_fd();

 private:
    int _dma_fd;
    bool _need_close_fd;
};

}  // namespace XCam

#endif  // XCAM_DMA_VIDEO_BUFFER_H
