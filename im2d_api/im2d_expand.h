/*
 * Copyright (C) 2022 Rockchip Electronics Co., Ltd.
 * Authors:
 *  Cerf Yu <cerf.yu@rock-chips.com>
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
 */
#ifndef _im2d_expand_h_
#define _im2d_expand_h_

#ifdef __cplusplus

#include "im2d_type.h"

#if ANDROID

#include <ui/GraphicBuffer.h>

using namespace android;

IM_API rga_buffer_handle_t importbuffer_GraphicBuffer_handle(buffer_handle_t hnd);
IM_API rga_buffer_handle_t importbuffer_GraphicBuffer(sp<GraphicBuffer> buf);

IM_API rga_buffer_t wrapbuffer_handle(buffer_handle_t hnd);
IM_API rga_buffer_t wrapbuffer_GraphicBuffer(sp<GraphicBuffer> buf);

#if USE_AHARDWAREBUFFER
#include <android/hardware_buffer.h>
IM_API rga_buffer_handle_t importbuffer_AHardwareBuffer(AHardwareBuffer *buf);
IM_API rga_buffer_t wrapbuffer_AHardwareBuffer(AHardwareBuffer *buf);

#endif /* #if USE_AHARDWAREBUFFER */
#endif /* #if ANDROID */

namespace rga {

/**
 * Add color fill task array
 *
 * @param src
 *      The input source image.
 * @param dst
 *      The output destination image.
 * @param top
 *      the top pixels
 * @param bottom
 *      the bottom pixels
 * @param left
 *      the left pixels
 * @param right
 *      the right pixels
 * @param border_type
 *      Border type.
 * @param value
 *      The pixel value at which the border is filled.
 *
 * @returns success or else negative error code.
 */
IM_API IM_STATUS immakeBorder(rga_buffer_t src, rga_buffer_t dst,
                              int top, int bottom, int left, int right,
                              int border_type, int value = 0,
                              int sync = 1, int *release_fence_fd = NULL);

};

#endif /* #ifdef __cplusplus */

#endif
