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
#include "drm_fourcc.h"

#include "rga.h"

enum format_colormodel
{
    rgb,
    yuv,
};

struct table_entry
{
    uint32_t fourcc;
    format_colormodel colormodel;
};

const static std::unordered_map<uint32_t, uint32_t> drm_fourcc_table = {
    { DRM_FORMAT_RGBA8888, RK_FORMAT_ABGR_8888 },
    { DRM_FORMAT_BGRA8888, RK_FORMAT_ARGB_8888 },
    { DRM_FORMAT_ARGB8888, RK_FORMAT_BGRA_8888 },
    { DRM_FORMAT_ABGR8888, RK_FORMAT_RGBA_8888 },
    { DRM_FORMAT_RGBX8888, RK_FORMAT_XBGR_8888 },
    { DRM_FORMAT_BGRX8888, RK_FORMAT_XRGB_8888 },
    { DRM_FORMAT_XRGB8888, RK_FORMAT_BGRX_8888 },
    { DRM_FORMAT_XBGR8888, RK_FORMAT_RGBX_8888 },

    { DRM_FORMAT_RGBA5551, RK_FORMAT_ABGR_5551 },
    { DRM_FORMAT_BGRA5551, RK_FORMAT_ARGB_5551 },
    { DRM_FORMAT_ARGB1555, RK_FORMAT_BGRA_5551 },
    { DRM_FORMAT_ABGR1555, RK_FORMAT_RGBA_5551 },
    { DRM_FORMAT_RGBA4444, RK_FORMAT_ABGR_4444 },
    { DRM_FORMAT_BGRA4444, RK_FORMAT_ARGB_4444 },
    { DRM_FORMAT_ARGB4444, RK_FORMAT_BGRA_4444 },
    { DRM_FORMAT_ABGR4444, RK_FORMAT_RGBA_4444 },

    { DRM_FORMAT_RGB888, RK_FORMAT_BGR_888 },
    { DRM_FORMAT_BGR888, RK_FORMAT_RGB_888 },
    { DRM_FORMAT_RGB565, RK_FORMAT_BGR_565 },
    { DRM_FORMAT_BGR565, RK_FORMAT_RGB_565 },

    { DRM_FORMAT_NV16, RK_FORMAT_YCbCr_422_SP },
    { DRM_FORMAT_NV61, RK_FORMAT_YCrCb_422_SP },
    { DRM_FORMAT_YUV422, RK_FORMAT_YCbCr_422_P },
    { DRM_FORMAT_YVU422, RK_FORMAT_YCrCb_422_P },
    // { , RK_FORMAT_YCbCr_422_SP_10B },
    // { , RK_FORMAT_YCrCb_422_SP_10B },
    { DRM_FORMAT_NV12, RK_FORMAT_YCbCr_420_SP },
    { DRM_FORMAT_NV21, RK_FORMAT_YCrCb_420_SP },
    { DRM_FORMAT_YUV420, RK_FORMAT_YCbCr_420_P },
    { DRM_FORMAT_YVU420, RK_FORMAT_YCrCb_420_P },
    { DRM_FORMAT_NV15, RK_FORMAT_YCbCr_420_SP_10B },
    // { , RK_FORMAT_YCrCb_420_SP_10B },

    { DRM_FORMAT_YUYV, RK_FORMAT_YUYV_422 },
    { DRM_FORMAT_YVYU, RK_FORMAT_YVYU_422 },
    { DRM_FORMAT_UYVY, RK_FORMAT_UYVY_422 },
    { DRM_FORMAT_VYUY, RK_FORMAT_VYUY_422 },
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
    { DRM_FORMAT_INVALID, RK_FORMAT_UNKNOWN },
};

uint32_t get_format_from_drm_fourcc(uint32_t drm_fourcc) {
    auto entry = drm_fourcc_table.find(drm_fourcc);
    if (entry == drm_fourcc_table.end())
        return RK_FORMAT_UNKNOWN;

    return entry->second;
}
