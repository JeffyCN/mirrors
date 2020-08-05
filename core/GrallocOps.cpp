/*
 * Copyright (C) 2016 Rockchip Electronics Co.Ltd
 * Authors:
 *  Zhiqin Wei <wzq@rock-chips.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 */
#ifdef ANDROID

#include "GrallocOps.h"

gralloc_module_t const *mAllocMod = NULL;

#ifdef RK3368
#define private_handle_t IMG_native_handle_t
#endif

// ---------------------------------------------------------------------------
int RkInitAllocModle() {
    const hw_module_t *allocMod = NULL;
    int ret = 0;

    if (mAllocMod)
        return 0;

    ret= hw_get_module(GRALLOC_HARDWARE_MODULE_ID, &allocMod);
    ALOGE_IF(ret, "FATAL:can't find the %s module",GRALLOC_HARDWARE_MODULE_ID);
    if (ret == 0)
        mAllocMod = reinterpret_cast<gralloc_module_t const *>(allocMod);

    return ret;
}

#ifndef ANDROID_7_DRM

int gralloc_backend_get_fd(private_handle_t* hnd, int *fd) {
    *fd = hnd->share_fd;
    return 0;
}

int gralloc_backend_get_attrs(private_handle_t* hnd, void *attrs) {
    std::vector<int> *attributes = (std::vector<int> *)attrs;
    attributes->clear();
    attributes->push_back(hnd->width);
    attributes->push_back(hnd->height);
    attributes->push_back(hnd->stride);
    attributes->push_back(hnd->format);
    attributes->push_back(hnd->size);
    attributes->push_back(hnd->type);
    return 0;
}

#else

#ifdef RK3368

int gralloc_backend_get_fd(private_handle_t* hnd, int *fd) {
    *fd = hnd->fd[0];
    return 0;
}

int gralloc_backend_get_attrs(private_handle_t* hnd, void *attrs) {
    std::vector<int> *attributes = (std::vector<int> *)attrs;
    attributes->clear();
    attributes->push_back(hnd->width);
    attributes->push_back(hnd->height);
    attributes->push_back(hnd->stride);
    attributes->push_back(hnd->format);
    attributes->push_back(hnd->size);
    attributes->push_back(hnd->type);
    return 0;
}

#endif      //RK3368

#endif      //ANDROID_7_DRM

int RkRgaGetHandleFd(buffer_handle_t handle, int *fd) {
    int ret = 0;

    if (!mAllocMod)
        ret = RkInitAllocModle();

    if (ret)
        return ret;

#ifdef ANDROID_7_DRM

#ifndef RK3368

    int op = GRALLOC_MODULE_PERFORM_GET_HADNLE_PRIME_FD;
    if (mAllocMod->perform)
        mAllocMod->perform(mAllocMod, op, handle, fd);
    else
        return -ENODEV;
#else
    private_handle_t* hnd = (private_handle_t*)handle;
    ret = gralloc_backend_get_fd(hnd,fd);
#endif      //RK3368

#else
    private_handle_t* hnd = (private_handle_t*)handle;
    ret = gralloc_backend_get_fd(hnd,fd);
#endif      //ANDROID_7_DRM

    if (ret)
        ALOGE("GraphicBufferGetHandldFd fail %d for:%s",ret,strerror(ret));
    else if (false) {
        ALOGD("fd = %d",*fd);
        fprintf(stderr,"fd = %d\n", *fd);
    }

    return ret;
}

int RkRgaGetHandleAttributes(buffer_handle_t handle,
                             std::vector<int> *attrs) {
    int ret = 0;

    if (!mAllocMod)
        ret = RkInitAllocModle();

    if (ret)
        return ret;

#if RK3368_DRM
    int w,h,pixel_stride,format,size;

    op = GRALLOC_MODULE_PERFORM_GET_HADNLE_WIDTH;
    mAllocMod->perform(mAllocMod, op, handle, &w);
    op = GRALLOC_MODULE_PERFORM_GET_HADNLE_HEIGHT;
    mAllocMod->perform(mAllocMod, op, handle, &h);
    op = GRALLOC_MODULE_PERFORM_GET_HADNLE_STRIDE;
    mAllocMod->perform(mAllocMod, op, handle, &pixel_stride);
    op = GRALLOC_MODULE_PERFORM_GET_HADNLE_FORMAT;
    mAllocMod->perform(mAllocMod, op, handle, &format);
    op = GRALLOC_MODULE_PERFORM_GET_HADNLE_SIZE;
    mAllocMod->perform(mAllocMod, op, handle, &size);

    //add to attrs.
    attrs->emplace_back(w);
    attrs->emplace_back(h);
    attrs->emplace_back(pixel_stride);
    attrs->emplace_back(format);
    attrs->emplace_back(size);

#else

#ifdef ANDROID_7_DRM

#ifndef RK3368
    int op = GRALLOC_MODULE_PERFORM_GET_HADNLE_ATTRIBUTES;
    if(!mAllocMod->perform)
        return -ENODEV;

    mAllocMod->perform(mAllocMod, op, handle, attrs);
#else
    private_handle_t* hnd = (private_handle_t*)handle;
    ret = gralloc_backend_get_attrs(hnd, (void*)attrs);
#endif      //RK3368

#else
    private_handle_t* hnd = (private_handle_t*)handle;
    ret = gralloc_backend_get_attrs(hnd, (void*)attrs);
#endif      //ANDROID_7_DRM


    if (ret)
        ALOGE("GraphicBufferGetHandldAttributes fail %d for:%s",ret,strerror(ret));
    else if (false) {
        ALOGD("%d,%d,%d,%d,%d,%d",attrs->at(0),attrs->at(1),attrs->at(2),
              attrs->at(3),attrs->at(4),attrs->at(5));
        fprintf(stderr,"%d,%d,%d,%d,%d,%d\n",
                attrs->at(0),attrs->at(1),attrs->at(2),
                attrs->at(3),attrs->at(4),attrs->at(5));
    }
#endif

    return ret;
}

int RkRgaGetHandleMapAddress(buffer_handle_t handle,
                             void **buf) {
    int usage = GRALLOC_USAGE_SW_READ_MASK | GRALLOC_USAGE_SW_WRITE_MASK;
#ifdef ANDROID_7_DRM
    usage |= GRALLOC_USAGE_HW_FB;
#endif
    int ret = 0;

    if (!mAllocMod)
        ret = RkInitAllocModle();

    if (ret)
        return ret;

    if (mAllocMod->lock)
        ret = mAllocMod->lock(mAllocMod, handle, usage, 0, 0, 0, 0, buf);
    else
        return -ENODEV;

    if (ret)
        ALOGE("GetHandleMapAddress fail %d for:%s",ret,strerror(ret));

    return ret;
}

int RkRgaGetRgaFormat(int format) {
    switch (format) {
        case HAL_PIXEL_FORMAT_BPP_1:
            return RK_FORMAT_BPP1;
        case HAL_PIXEL_FORMAT_BPP_2:
            return RK_FORMAT_BPP2;
        case HAL_PIXEL_FORMAT_BPP_4:
            return RK_FORMAT_BPP4;
        case HAL_PIXEL_FORMAT_BPP_8:
            return RK_FORMAT_BPP8;
        case HAL_PIXEL_FORMAT_RGB_565:
            return RK_FORMAT_RGB_565;
        case HAL_PIXEL_FORMAT_RGB_888:
            return RK_FORMAT_RGB_888;
        case HAL_PIXEL_FORMAT_RGBA_8888:
            return RK_FORMAT_RGBA_8888;
        case HAL_PIXEL_FORMAT_RGBX_8888:
            return RK_FORMAT_RGBX_8888;
        case HAL_PIXEL_FORMAT_BGRA_8888:
            return RK_FORMAT_BGRA_8888;
        case HAL_PIXEL_FORMAT_YCrCb_420_SP:
            return RK_FORMAT_YCrCb_420_SP;
        case HAL_PIXEL_FORMAT_YCrCb_NV12:
            return RK_FORMAT_YCbCr_420_SP;
        case HAL_PIXEL_FORMAT_YCrCb_NV12_VIDEO:
            return RK_FORMAT_YCbCr_420_SP;
        case HAL_PIXEL_FORMAT_YCrCb_NV12_10:
            return RK_FORMAT_YCbCr_420_SP_10B; //0x20
        default:
            ALOGE("Is unsupport format now,please fix");
            return -1;
    }
}

int RkRgaGetRgaFormatFromAndroid(int format) {
    switch (format) {
        case HAL_PIXEL_FORMAT_BPP_1:
            return RK_FORMAT_BPP1;
        case HAL_PIXEL_FORMAT_BPP_2:
            return RK_FORMAT_BPP2;
        case HAL_PIXEL_FORMAT_BPP_4:
            return RK_FORMAT_BPP4;
        case HAL_PIXEL_FORMAT_BPP_8:
            return RK_FORMAT_BPP8;
        case HAL_PIXEL_FORMAT_RGB_565:
            return RK_FORMAT_RGB_565;
        case HAL_PIXEL_FORMAT_RGB_888:
            return RK_FORMAT_RGB_888;
        case HAL_PIXEL_FORMAT_RGBA_8888:
            return RK_FORMAT_RGBA_8888;
        case HAL_PIXEL_FORMAT_RGBX_8888:
            return RK_FORMAT_RGBX_8888;
        case HAL_PIXEL_FORMAT_BGRA_8888:
            return RK_FORMAT_BGRA_8888;
        case HAL_PIXEL_FORMAT_YCrCb_420_SP:
            return RK_FORMAT_YCrCb_420_SP;
        case HAL_PIXEL_FORMAT_YCrCb_NV12:
            return RK_FORMAT_YCbCr_420_SP;
        case HAL_PIXEL_FORMAT_YCrCb_NV12_VIDEO:
            return RK_FORMAT_YCbCr_420_SP;
        case HAL_PIXEL_FORMAT_YCrCb_NV12_10:
            return RK_FORMAT_YCbCr_420_SP_10B;//0x20
        default:
            ALOGE("Is unsupport format now,please fix");
            return -1;
    }
}

#endif
