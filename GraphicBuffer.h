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

#ifndef _rk_graphic_buffer_h_
#define _rk_graphic_buffer_h_

#ifdef ANDROID

#include <stdint.h>
#include <vector>
#include <sys/types.h>

//#include <system/window.h>
#include <system/graphics.h>

#include <utils/Thread.h>

#include <EGL/egl.h>
#include <GLES/gl.h>

//////////////////////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>

#include <sys/mman.h>
#include <linux/stddef.h>

#include <hardware/rga.h>
#include "stdio.h"

#include <utils/Atomic.h>
#include <utils/Errors.h>
#include <android/log.h>
#include <utils/Log.h>
#include <log/log_main.h>

#include "drmrga.h"

#ifndef RK_FORMAT_YCbCr_420_SP_10B
#define RK_FORMAT_YCbCr_420_SP_10B  0x20
#endif

#ifndef RK_FORMAT_YCrCb_420_SP_10B
#define RK_FORMAT_YCrCb_420_SP_10B  0x21
#endif

#ifndef RK3368

#ifdef ANDROID_7_DRM
#include <hardware/gralloc.h>
#else
#include <gralloc_priv.h>
#endif

#else
#include <hardware/gralloc.h>
#include <hardware/img_gralloc_public.h>

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
#endif

// -------------------------------------------------------------------------------
int         RkRgaGetHandleFd(buffer_handle_t handle, int *fd);
int         RkRgaGetHandleAttributes(buffer_handle_t handle,
                                     std::vector<int> *attrs);
int         RkRgaGetHandleMapAddress(buffer_handle_t handle,
                                     void **buf);

int         RkRgaGetRgaFormat(int format);
#endif

#endif
