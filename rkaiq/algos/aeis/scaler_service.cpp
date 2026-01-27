/*
 * scaler_service.cpp
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
#include "scaler_service.h"

#include "image_processor.h"
#include "dma_video_buffer.h"

using namespace XCam;

namespace RkCam {

template <typename To, typename From>
To convert(From&);

template <>
img_buffer_t convert(std::shared_ptr<DmaVideoBuffer>& dma) {
    auto info        = dma->get_video_info();
    img_buffer_t buf = {
        .vir_addr = nullptr,
        .phy_addr = nullptr,
        .fd      = -1,
        .width   = (int)info.width,
        .height  = (int)info.height,
        .wstride = (int)info.aligned_width,
        .hstride = (int)info.aligned_height,
        .format  = RK_PIX_FMT_NV12,
    };

    buf.fd     = dma->get_fd();
    buf.format = static_cast<rk_aiq_format_t>(info.format);

    return buf;
}

ScalerTask::ScalerTask(std::unique_ptr<ImageProcessor> proc) : proc_(std::move(proc)) {}

TaskResult ScalerTask::operator()(ServiceParam<scaler_param>& p) {
    auto full   = p.payload->input_image;
    auto scales = p.payload->scaled_images;
    auto last   = full;
    for (auto it = scales.begin(); it != scales.end(); it++) {
        img_buffer_t src = convert<img_buffer_t>(last);
        img_buffer_t dst = convert<img_buffer_t>(*it);
        proc_->resize(src, dst, 0, 0);
        last = *it;
    }

    return TaskResult::kSuccess;
}

}  // namespace RkCam
