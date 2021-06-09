/*
 * Copyright (C) 2019-2020 RockChip Limited. All rights reserved.
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

/*  --------------------------------------------------------------------------------------------------------
 *  File:   platform_gralloc4.c
 *
 *  Desc:   具体实现 platform_gralloc4.h 定义的接口, 基于 IMapper 4.0.
 *
 *          -----------------------------------------------------------------------------------
 *          < 习语 和 缩略语 > :
 *
 *          -----------------------------------------------------------------------------------
 *          < 内部模块 or 对象的层次结构 > :
 *
 *          -----------------------------------------------------------------------------------
 *          < 功能实现的基本流程 > :
 *
 *          -----------------------------------------------------------------------------------
 *          < 关键标识符 > :
 *
 *          -----------------------------------------------------------------------------------
 *          < 本模块实现依赖的外部模块 > :
 *              ...
 *          -----------------------------------------------------------------------------------
 *  Note:
 *
 *  Author: ChenZhen
 *
 *  Log:
 *          init
    ----Fri Aug 28 10:17:46 2020
 */


/* ---------------------------------------------------------------------------------------------------------
 * Include Files
 * ---------------------------------------------------------------------------------------------------------
 */

#define LOG_TAG "platform_gralloc4"
#define ENABLE_DEBUG_LOG
#include <custom_log.h>

#include <sync/sync.h>

#include "mali_gralloc_formats.h"
#include <drm_fourcc.h>

#include "platform_gralloc4.h"

#include <android/hardware/graphics/mapper/4.0/IMapper.h>
#include <gralloctypes/Gralloc4.h>
// #include <aidl/arm/graphics/ArmMetadataType.h>

using android::hardware::graphics::mapper::V4_0::Error;
using android::hardware::graphics::mapper::V4_0::IMapper;
using android::hardware::hidl_vec;

using android::gralloc4::MetadataType_PlaneLayouts;
using android::gralloc4::decodePlaneLayouts;
using android::gralloc4::MetadataType_Usage;
using android::gralloc4::decodeUsage;
using android::gralloc4::MetadataType_PlaneLayouts;
using android::gralloc4::decodePlaneLayouts;
using android::gralloc4::MetadataType_PixelFormatFourCC;
using android::gralloc4::decodePixelFormatFourCC;
using android::gralloc4::MetadataType_PixelFormatModifier;
using android::gralloc4::decodePixelFormatModifier;
using android::gralloc4::MetadataType_PixelFormatRequested;
using android::gralloc4::decodePixelFormatRequested;
using android::gralloc4::MetadataType_AllocationSize;
using android::gralloc4::decodeAllocationSize;
using android::gralloc4::MetadataType_LayerCount;
using android::gralloc4::decodeLayerCount;
using android::gralloc4::MetadataType_Dataspace;
using android::gralloc4::decodeDataspace;
using android::gralloc4::MetadataType_Crop;
using android::gralloc4::decodeCrop;
using android::gralloc4::MetadataType_Width;
using android::gralloc4::decodeWidth;
using android::gralloc4::MetadataType_Height;
using android::gralloc4::decodeHeight;

using aidl::android::hardware::graphics::common::Dataspace;
using aidl::android::hardware::graphics::common::PlaneLayout;
using aidl::android::hardware::graphics::common::ExtendableType;
using aidl::android::hardware::graphics::common::PlaneLayout;
using aidl::android::hardware::graphics::common::Rect;
using android::hardware::graphics::common::V1_2::PixelFormat;


#define GRALLOC_ARM_METADATA_TYPE_NAME "arm.graphics.ArmMetadataType"
const static IMapper::MetadataType ArmMetadataType_PLANE_FDS
{
    GRALLOC_ARM_METADATA_TYPE_NAME,
    // static_cast<int64_t>(aidl::arm::graphics::ArmMetadataType::PLANE_FDS)
    1   // 就是上面的 'PLANE_FDS'
};


/* ---------------------------------------------------------------------------------------------------------
 * External Function Prototypes (referenced in this file)
 * ---------------------------------------------------------------------------------------------------------
 */

/* ---------------------------------------------------------------------------------------------------------
 * Local Macros
 * ---------------------------------------------------------------------------------------------------------
 */

namespace gralloc4 {
/* ---------------------------------------------------------------------------------------------------------
 * Local Typedefs
 * ---------------------------------------------------------------------------------------------------------
 */


/* ---------------------------------------------------------------------------------------------------------
 * Local Function Prototypes
 * ---------------------------------------------------------------------------------------------------------
 */


/* ---------------------------------------------------------------------------------------------------------
 * Local Variables
 * ---------------------------------------------------------------------------------------------------------
 */
static constexpr Error kTransactionError = Error::NO_RESOURCES;

/* ---------------------------------------------------------------------------------------------------------
 * Global Variables
 * ---------------------------------------------------------------------------------------------------------
 */

/* ---------------------------------------------------------------------------------------------------------
 * Local Functions Implementation
 * ---------------------------------------------------------------------------------------------------------
 */

static IMapper &get_service()
{
    static android::sp<IMapper> cached_service = IMapper::getService();
    return *cached_service;
}

template <typename T>
static int get_metadata(IMapper &mapper, buffer_handle_t handle, IMapper::MetadataType type,
                        android::status_t (*decode)(const hidl_vec<uint8_t> &, T *), T *value)
{
    void *handle_arg = const_cast<native_handle_t *>(handle);
    assert(handle_arg);
    assert(value);
    assert(decode);

    int err = 0;
    mapper.get(handle_arg, type, [&err, value, decode](Error error, const hidl_vec<uint8_t> &metadata)
                {
                    if (error != Error::NONE)
                    {
                        err = android::BAD_VALUE;
                        return;
                    }
                    err = decode(metadata, value);
                });
    return err;
}

/*
 * 参考 b_r25p1 gralloc 中的 drm_fourcc_from_handle() 实现.
 * 目前未考虑 'modifier'
 */
uint64_t get_internal_format_from_fourcc(uint32_t fourcc, uint64_t modifier)
{
    /* Clean the modifier bits in the internal format. */
    struct table_entry
    {
        uint64_t internal; // 不带 modifier_bits
        uint32_t fourcc;
    };

    static table_entry table[] = {
        { MALI_GRALLOC_FORMAT_INTERNAL_RAW16, DRM_FORMAT_R16 },
        { MALI_GRALLOC_FORMAT_INTERNAL_RGBA_8888, DRM_FORMAT_ABGR8888 },
        { MALI_GRALLOC_FORMAT_INTERNAL_BGRA_8888, DRM_FORMAT_ARGB8888 },
        { MALI_GRALLOC_FORMAT_INTERNAL_RGB_565, DRM_FORMAT_RGB565 },
        { MALI_GRALLOC_FORMAT_INTERNAL_RGBX_8888, DRM_FORMAT_XBGR8888 },
        { MALI_GRALLOC_FORMAT_INTERNAL_RGB_888, DRM_FORMAT_BGR888 },
        { MALI_GRALLOC_FORMAT_INTERNAL_RGBA_1010102, DRM_FORMAT_ABGR2101010 },
        { MALI_GRALLOC_FORMAT_INTERNAL_RGBA_16161616, DRM_FORMAT_ABGR16161616F },
        { MALI_GRALLOC_FORMAT_INTERNAL_YV12, DRM_FORMAT_YVU420 },
        { MALI_GRALLOC_FORMAT_INTERNAL_NV12, DRM_FORMAT_NV12 },
        { MALI_GRALLOC_FORMAT_INTERNAL_NV16, DRM_FORMAT_NV16 },
        { MALI_GRALLOC_FORMAT_INTERNAL_NV21, DRM_FORMAT_NV21 },
        { MALI_GRALLOC_FORMAT_INTERNAL_Y0L2, DRM_FORMAT_Y0L2 },
        { MALI_GRALLOC_FORMAT_INTERNAL_Y210, DRM_FORMAT_Y210 },
        { MALI_GRALLOC_FORMAT_INTERNAL_P010, DRM_FORMAT_P010 },
        { MALI_GRALLOC_FORMAT_INTERNAL_P210, DRM_FORMAT_P210 },
        { MALI_GRALLOC_FORMAT_INTERNAL_Y410, DRM_FORMAT_Y410 },
        { MALI_GRALLOC_FORMAT_INTERNAL_YUV422_8BIT, DRM_FORMAT_YUYV },
        { MALI_GRALLOC_FORMAT_INTERNAL_YUV420_8BIT_I, DRM_FORMAT_YUV420_8BIT },
        { MALI_GRALLOC_FORMAT_INTERNAL_YUV420_10BIT_I, DRM_FORMAT_YUV420_10BIT },

        /* Deprecated legacy formats, mapped to MALI_GRALLOC_FORMAT_INTERNAL_YUV422_8BIT. */
        { HAL_PIXEL_FORMAT_YCbCr_422_I, DRM_FORMAT_YUYV },
        /* Deprecated legacy formats, mapped to MALI_GRALLOC_FORMAT_INTERNAL_NV21. */
        { HAL_PIXEL_FORMAT_YCrCb_420_SP, DRM_FORMAT_NV21 },
        /* Format introduced in Android P, mapped to MALI_GRALLOC_FORMAT_INTERNAL_P010. */
        { HAL_PIXEL_FORMAT_YCBCR_P010, DRM_FORMAT_P010 },
    };

    for (size_t i = 0; i < sizeof(table) / sizeof(table[0]); i++)
    {
        if (table[i].fourcc == fourcc)
        {
            return table[i].internal;
        }
    }

    LOG_ALWAYS_FATAL("unexpected fourcc : 0x%x", fourcc);
    return 0;
}

android::status_t static decodeArmPlaneFds(const hidl_vec<uint8_t>& input, std::vector<int64_t>* fds)
{
    assert (fds != nullptr);
    int64_t size = 0;

    memcpy(&size, input.data(), sizeof(int64_t));
    if (size < 0)
    {
        return android::BAD_VALUE;
    }

    fds->resize(size);

    const uint8_t *tmp = input.data() + sizeof(int64_t);
    memcpy(fds->data(), tmp, sizeof(int64_t) * size);

    return android::NO_ERROR;
}

/* ---------------------------------------------------------------------------------------------------------
 * Global Functions Implementation
 * ---------------------------------------------------------------------------------------------------------
 */

uint64_t get_internal_format(buffer_handle_t handle)
{
    auto &mapper = get_service();
    uint32_t fourcc;
    uint64_t modifier;

    /* 获取 format_fourcc. */
    int err = get_metadata(mapper, handle, MetadataType_PixelFormatFourCC, decodePixelFormatFourCC, &fourcc);
    assert(err == android::NO_ERROR);

    /* 获取 format_modifier. */
    err = get_metadata(mapper, handle, MetadataType_PixelFormatModifier, decodePixelFormatModifier, &modifier);
    assert(err == android::NO_ERROR);

    /* 从 fourcc 得到 internal_format. */
    return (get_internal_format_from_fourcc(fourcc, modifier) );
}

int get_width(buffer_handle_t handle, uint64_t* width)
{
    auto &mapper = get_service();

    int err = get_metadata(mapper, handle, MetadataType_Width, decodeWidth, width);
    if (err != android::OK)
    {
        E("err : %d", err);
    }

    return err;
}

int get_height(buffer_handle_t handle, uint64_t* height)
{
    auto &mapper = get_service();

    int err = get_metadata(mapper, handle, MetadataType_Height, decodeHeight, height);
    if (err != android::OK)
    {
        E("err : %d", err);
    }

    return err;
}

int get_pixel_stride(buffer_handle_t handle, int* pixel_stride)
{
    auto &mapper = get_service();
    std::vector<PlaneLayout> layouts;
    int format_requested;

    int err = get_format_requested(handle, &format_requested);
    if (err != android::OK )
    {
        E("err : %d", err);
        return err;
    }

    /* 若 'format_requested' "不是" HAL_PIXEL_FORMAT_YCrCb_NV12_10, 则 ... */
    if ( format_requested != HAL_PIXEL_FORMAT_YCrCb_NV12_10 )
    {
        err = get_metadata(mapper, handle, MetadataType_PlaneLayouts, decodePlaneLayouts, &layouts);
        if (err != android::OK || layouts.size() < 1)
        {
            E("Failed to get plane layouts. err : %d", err);
            return err;
        }

        if ( layouts.size() > 1 )
        {
            W("it's not reasonable to get global pixel_stride of buffer with planes more than 1.");
        }

        *pixel_stride = (layouts[0].widthInSamples);
    }
    /* 否则, 即 'format_requested' "是" HAL_PIXEL_FORMAT_YCrCb_NV12_10, 则 ... */
    else
    {
        uint64_t width;
        int byte_stride;

        err = get_width(handle, &width);
        if (err != android::OK )
        {
            E("err : %d", err);
            return err;
        }

        // .trick : from CSY : 分配 rk_video_decoder 输出 buffers 时, 要求的 byte_stride of buffer in NV12_10, 已经通过 width 传入.
        //          原理上, NV12_10 的 pixel_stride 和 byte_stride 是不同的, 但是这里保留 之前 rk_drm_gralloc 的赋值方式.
        byte_stride = (int)width;
        *pixel_stride = byte_stride;
    }

    return err;
}

int get_byte_stride(buffer_handle_t handle, int* byte_stride)
{
    auto &mapper = get_service();
    std::vector<PlaneLayout> layouts;
    int format_requested;

    int err = get_format_requested(handle, &format_requested);
    if (err != android::OK )
    {
        E("err : %d", err);
        return err;
    }

    /* 若 'format_requested' "不是" HAL_PIXEL_FORMAT_YCrCb_NV12_10, 则 ... */
    if ( format_requested != HAL_PIXEL_FORMAT_YCrCb_NV12_10 )
    {
        err = get_metadata(mapper, handle, MetadataType_PlaneLayouts, decodePlaneLayouts, &layouts);
        if (err != android::OK || layouts.size() < 1)
        {
            E("Failed to get plane layouts. err : %d", err);
            return err;
        }

        if ( layouts.size() > 1 )
        {
            W("it's not reasonable to get global byte_stride of buffer with planes more than 1.");
        }

        *byte_stride = (layouts[0].strideInBytes);
    }
    /* 否则, 即 'format_requested' "是" HAL_PIXEL_FORMAT_YCrCb_NV12_10, 则 ... */
    else
    {
        uint64_t width;

        err = get_width(handle, &width);
        if (err != android::OK )
        {
            E("err : %d", err);
            return err;
        }

        // .KP : from CSY : 分配 rk_video_decoder 输出 buffers 时, 要求的 byte_stride of buffer in NV12_10, 已经通过 width 传入.
        *byte_stride = (int)width;
    }

    return err;
}

int get_format_requested(buffer_handle_t handle, int* format_requested)
{
    auto &mapper = get_service();
    // android::PixelFormat format;    // *format_requested
    PixelFormat format;    // *format_requested

    int err = get_metadata(mapper, handle, MetadataType_PixelFormatRequested, decodePixelFormatRequested, &format);
    if (err != android::OK)
    {
        E("Failed to get pixel_format_requested. err : %d", err);
        return err;
    }

    *format_requested = (int)format;

    return err;
}

int get_usage(buffer_handle_t handle, uint64_t* usage)
{
    auto &mapper = get_service();

    int err = get_metadata(mapper, handle, MetadataType_Usage, decodeUsage, usage);
    if (err != android::OK)
    {
        E("Failed to get pixel_format_requested. err : %d", err);
        return err;
    }

    return err;
}

int get_allocation_size(buffer_handle_t handle, uint64_t* allocation_size)
{
    auto &mapper = get_service();

    int err = get_metadata(mapper, handle, MetadataType_AllocationSize, decodeAllocationSize, allocation_size);
    if (err != android::OK)
    {
        E("Failed to get allocation_size. err : %d", err);
        return err;
    }

    return err;
}

int get_share_fd(buffer_handle_t handle, int* share_fd)
{
    auto &mapper = get_service();
    std::vector<int64_t> fds;

    int err = get_metadata(mapper, handle, ArmMetadataType_PLANE_FDS, decodeArmPlaneFds, &fds);
    if (err != android::OK)
    {
        E("Failed to get plane_fds. err : %d", err);
        return err;
    }
    assert (fds.size() > 0);

    *share_fd = (int)(fds[0]);

    return err;
}

status_t importBuffer(buffer_handle_t rawHandle, buffer_handle_t* outHandle)
{
    auto &mapper = get_service();
    Error error;
    auto ret = mapper.importBuffer(android::hardware::hidl_handle(rawHandle),
                                   [&](const auto& tmpError, const auto& tmpBuffer)
                                   {
        error = tmpError;
        if (error != Error::NONE)
        {
            return;
        }
        *outHandle = static_cast<buffer_handle_t>(tmpBuffer);
                                    });

    return static_cast<status_t>((ret.isOk()) ? error : kTransactionError);
}

void freeBuffer(buffer_handle_t handle)
{
    auto &mapper = get_service();
    auto buffer = const_cast<native_handle_t*>(handle);
    auto ret = mapper.freeBuffer(buffer);

    auto error = (ret.isOk()) ? static_cast<Error>(ret) : kTransactionError;
}

status_t lock(buffer_handle_t bufferHandle,
              uint64_t usage,
              int x,
              int y,
              int w,
              int h,
              void** outData)
{
    auto &mapper = get_service();
    auto buffer = const_cast<native_handle_t*>(bufferHandle);

    IMapper::Rect accessRegion = {x, y, w, h};

    android::hardware::hidl_handle acquireFenceHandle; // dummy

    Error error;
    auto ret = mapper.lock(buffer,
                           usage,
                           accessRegion,
                           acquireFenceHandle,
                           [&](const auto& tmpError, const auto& tmpData) {
                                error = tmpError;
                                if (error != Error::NONE) {
                                    return;
                                }
                                *outData = tmpData;
                           });

    error = (ret.isOk()) ? error : kTransactionError;

    ALOGW_IF(error != Error::NONE, "lock(%p, ...) failed: %d", bufferHandle, error);

    return static_cast<status_t>(error);
}

void unlock(buffer_handle_t bufferHandle)
{
    auto &mapper = get_service();
    auto buffer = const_cast<native_handle_t*>(bufferHandle);

    int releaseFence = -1;
    Error error;
    auto ret = mapper.unlock(buffer,
                             [&](const auto& tmpError, const auto& tmpReleaseFence)
                             {
        error = tmpError;
        if (error != Error::NONE) {
            return;
        }

        auto fenceHandle = tmpReleaseFence.getNativeHandle(); // 预期 unlock() 不会返回有效的 release_fence.
        if (fenceHandle && fenceHandle->numFds == 1)
        {
            E("got unexpected valid fd of release_fence : %d", fenceHandle->data[0]);

            int fd = dup(fenceHandle->data[0]);
            if (fd >= 0) {
                releaseFence = fd;
            } else {
                ALOGD("failed to dup unlock release fence");
                sync_wait(fenceHandle->data[0], -1);
            }
        }
                             });

    if (!ret.isOk()) {
        error = kTransactionError;
    }

    if (error != Error::NONE) {
        ALOGE("unlock(%p) failed with %d", buffer, error);
    }

    return;
}

} // namespace gralloc4
