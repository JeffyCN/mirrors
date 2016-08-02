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

#ifndef _rockchip_rga_test_
#define _rockchip_rga_test_

#include <stdint.h>
#include <sys/types.h>

#include <system/window.h>

#include <androidfw/AssetManager.h>
#include <utils/Thread.h>

#include <EGL/egl.h>
#include <GLES/gl.h>

///////////////////////////////////////////////////////
#include "../drmrga.h"
#include <hardware/hardware.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>

#include <sys/mman.h>
#include <linux/stddef.h>
///////////////////////////////////////////////////////

struct _rga_context {
    gralloc_module_t const          *mAllocMod;
    alloc_device_t                  *mAllocDev;

    hw_module_t const               *mRgaMod;
    rga_device_t                    *mRga;
    unsigned int                    crcTable[256];
    unsigned int                    crcStd;
};

static void init_crc_table(void);
static unsigned int crc32(unsigned int crc,unsigned char *buffer, unsigned int size);

bool rgaTest(bool prepare);

int readyToRun();
int runToExit();
int rgaNv12ScaleTest(bool prepare);

static int gralloc_alloc_buffer(buffer_handle_t *hnd, int w,int h,int *s,int fmt,int usage);
static int gralloc_free_buffer(buffer_handle_t hnd);

#endif // ANDROID_BOOTANIMATION_H
