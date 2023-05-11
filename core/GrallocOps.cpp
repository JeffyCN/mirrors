/*
 * Copyright (C) 2016 Rockchip Electronics Co., Ltd.
 * Authors:
 *  Zhiqin Wei <wzq@rock-chips.com>
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
#ifdef ANDROID

#include "GrallocOps.h"

#if USE_GRALLOC_4

#include "platform_gralloc4.h"

int RkRgaGetHandleFd(buffer_handle_t handle, int *fd) {
    int err = 0;

    err = gralloc4::get_share_fd(handle, fd);
    if (err != android::OK)
    {
        ALOGE("Failed to get buffer share_fd, err : %d", err);
        return -1;
    }

    return err;
}

int RkRgaGetHandleAttributes(buffer_handle_t handle,
                             std::vector<int> *attrs) {
    uint64_t w, h, size;
    int pixel_stride, format;
    int err = 0;

    err = gralloc4::get_width(handle, &w);
    if (err != android::OK)
    {
        ALOGE("Failed to get buffer width, err : %d", err);
        return -1;
    }

    err = gralloc4::get_height(handle, &h);
    if (err != android::OK)
    {
        ALOGE("Failed to get buffer height, err : %d", err);
        return -1;
    }

    err = gralloc4::get_pixel_stride(handle, &pixel_stride);
    if (err != android::OK)
    {
        ALOGE("Failed to get buffer pixel_stride, err : %d", err);
        return -1;
    }

    err = gralloc4::get_format_requested(handle, &format);
    if (err != android::OK)
    {
        ALOGE("Failed to get buffer format, err : %d", err);
        return -1;
    }

    err = gralloc4::get_allocation_size(handle, &size);
    if (err != android::OK)
    {
        ALOGE("Failed to get buffer size, err : %d", err);
        return -1;
    }

    //add to attrs.
    attrs->emplace_back(w);
    attrs->emplace_back(h);
    attrs->emplace_back(pixel_stride);
    attrs->emplace_back(format);
    attrs->emplace_back(size);

    return err;
}

int RkRgaGetHandleMapAddress(buffer_handle_t handle,
                             void **buf) {
    int err;
    uint64_t w, h;

    err = gralloc4::get_width(handle, &w);
    if (err != android::OK)
    {
        ALOGE("Failed to get buffer width, err : %d", err);
        return -1;
    }

    err = gralloc4::get_height(handle, &h);
    if (err != android::OK)
    {
        ALOGE("Failed to get buffer height, err : %d", err);
        return -1;
    }

    err = gralloc4::lock(handle, GRALLOC_USAGE_SW_READ_MASK, 0, 0, w, h,
                    buf);
    if (err != android::OK)
    {
        ALOGE("Failed to lock buffer, err : %d", err);
        return -1;
    }

    gralloc4::unlock(handle);

    return err;
}

/* ========================================================================== */
#else /* #if USE_GRALLOC_4, else is gralloc 0.3 */

#ifndef RK3368

#ifdef ANDROID_7_DRM
#include <hardware/gralloc.h>
#else
#include <gralloc_priv.h>
#endif

#else
#include <hardware/gralloc.h>
#include <hardware/img_gralloc_public.h>

#endif /* #ifndef RK3368 */

/*
 *   Only these macros in rockchip's private gralloc-0.3 header are
 * needed in librga, and these are defined in different paths for
 * different Android versions.
 *   So when librga refers to gralloc0.3, it will judge whether
 * there is a reference to the corresponding path, and if not,
 * define these macros by itself.
 */
#ifndef GRALLOC_MODULE_PERFORM_GET_HADNLE_PRIME_FD
#define GRALLOC_MODULE_PERFORM_GET_HADNLE_PRIME_FD 0x08100002
#endif

#ifndef GRALLOC_MODULE_PERFORM_GET_HADNLE_ATTRIBUTES
#define GRALLOC_MODULE_PERFORM_GET_HADNLE_ATTRIBUTES 0x08100004
#endif

#ifndef GRALLOC_MODULE_PERFORM_GET_INTERNAL_FORMAT
#define GRALLOC_MODULE_PERFORM_GET_INTERNAL_FORMAT 0x08100006
#endif

#ifndef GRALLOC_MODULE_PERFORM_GET_USAGE
#define GRALLOC_MODULE_PERFORM_GET_USAGE 0x0feeff03
#endif

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
    if (mAllocMod->perform) {
        mAllocMod->perform(mAllocMod, GRALLOC_MODULE_PERFORM_GET_HADNLE_PRIME_FD, handle, fd);
	}
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

#endif //USE_GRALLOC_4
#endif /* ANDROID */
