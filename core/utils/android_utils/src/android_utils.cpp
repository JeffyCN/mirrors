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

#include <unordered_map>

#include "rga.h"

#ifdef ANDROID
#include <ui/PixelFormat.h>
#include <hardware/hardware.h>

#ifdef ANDROID_12
#include <hardware/hardware_rockchip.h>
#endif

const static std::unordered_map<uint32_t, uint32_t> android_hal_table = {
    // { , RK_FORMAT_ABGR_8888 },
    // { , RK_FORMAT_ARGB_8888 },
    { HAL_PIXEL_FORMAT_BGRA_8888, RK_FORMAT_BGRA_8888 },
    { HAL_PIXEL_FORMAT_RGBA_8888, RK_FORMAT_RGBA_8888 },
    // { , RK_FORMAT_XBGR_8888 },
    // { , RK_FORMAT_XRGB_8888 },
    // { , RK_FORMAT_BGRX_8888 },
    { HAL_PIXEL_FORMAT_RGBX_8888, RK_FORMAT_RGBX_8888 },

    // { , RK_FORMAT_ABGR_5551 },
    // { , RK_FORMAT_ARGB_5551 },
    // { , RK_FORMAT_BGRA_5551 },
    // { , RK_FORMAT_RGBA_5551 },
    // { , RK_FORMAT_ABGR_4444 },
    // { , RK_FORMAT_ARGB_4444 },
    // { , RK_FORMAT_BGRA_4444 },
    // { , RK_FORMAT_RGBA_4444 },

    // { , RK_FORMAT_BGR_888 },
    { HAL_PIXEL_FORMAT_RGB_888, RK_FORMAT_RGB_888 },
    // { , RK_FORMAT_BGR_565 },
    { HAL_PIXEL_FORMAT_RGB_565, RK_FORMAT_RGB_565 },

    // { , RK_FORMAT_YCbCr_422_SP },
    // { , RK_FORMAT_YCrCb_422_SP },
    // { , RK_FORMAT_YCbCr_422_P },
    // { , RK_FORMAT_YCrCb_422_P },
    // { , RK_FORMAT_YCbCr_422_SP_10B },
    // { , RK_FORMAT_YCrCb_422_SP_10B },
    { HAL_PIXEL_FORMAT_YCrCb_NV12, RK_FORMAT_YCbCr_420_SP },
    { HAL_PIXEL_FORMAT_YCrCb_420_SP, RK_FORMAT_YCrCb_420_SP },
    // { , RK_FORMAT_YCbCr_420_P },
    // { , RK_FORMAT_YCrCb_420_P },
    { HAL_PIXEL_FORMAT_YCrCb_NV12_10, RK_FORMAT_YCbCr_420_SP_10B },
    // { , RK_FORMAT_YCrCb_420_SP_10B },

    // { , RK_FORMAT_YUYV_422 },
    // { , RK_FORMAT_YVYU_422 },
    // { , RK_FORMAT_UYVY_422 },
    // { , RK_FORMAT_VYUY_422 },
    // { , RK_FORMAT_YUYV_420 },
    // { , RK_FORMAT_YVYU_420 },
    // { , RK_FORMAT_UYVY_420 },
    // { , RK_FORMAT_VYUY_420 },

    // { , RK_FORMAT_Y4 },
    // { , RK_FORMAT_YCbCr_400 },

    // { , RK_FORMAT_BPP1 },
    // { , RK_FORMAT_BPP2 },
    // { , RK_FORMAT_BPP4 },
    // { , RK_FORMAT_BPP8 },
    // { , RK_FORMAT_RGBA2BPP },
    // { , RK_FORMAT_UNKNOWN },
};
#else
const static std::unordered_map<uint32_t, uint32_t> android_hal_table;
#endif

uint32_t get_format_from_android_hal(uint32_t android_hal_format) {
    auto entry = android_hal_table.find(android_hal_format);
    if (entry == android_hal_table.end())
        return RK_FORMAT_UNKNOWN;

    return entry->second;
}
