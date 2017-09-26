/*
 * Copyright (C) 2016 Rockchip Electronics Co.Ltd
 * Authors:
 *	Zhiqin Wei <wzq@rock-chips.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 */

#ifndef _rk_graphic_buffer_h_
#define _rk_graphic_buffer_h_
#include <stdint.h>
#include <vector>
#include <sys/types.h>

#include <system/window.h>
#include <system/graphics.h>

#include <utils/Thread.h>

#include <EGL/egl.h>
#include <GLES/gl.h>

//////////////////////////////////////////////////////////////////////////////////
#include <hardware/hardware.h>
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
#include <utils/Log.h>

#include "drmrga.h"

#ifndef RK_FORMAT_YCbCr_420_SP_10B
#define RK_FORMAT_YCbCr_420_SP_10B  0x20
#endif

#ifndef RK_FORMAT_YCrCb_420_SP_10B
#define RK_FORMAT_YCrCb_420_SP_10B  0x21
#endif
// -------------------------------------------------------------------------------
int         RkRgaGetHandleFd(buffer_handle_t handle, int *fd);
int         RkRgaGetHandleAttributes(buffer_handle_t handle,
							std::vector<int> *attrs);
int         RkRgaGetHandleMapAddress(buffer_handle_t handle,
									void **buf);

int         RkRgaGetRgaFormat(int format);
#endif
