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

#ifndef _rk_drm_rga_rga_context_
#define _rk_drm_rga_rga_context_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>

#include <sys/mman.h>
#include <linux/stddef.h>

#include <xf86drm.h>
#include <xf86drmMode.h>
//#include <libkms.h>
#include <drm_fourcc.h>

#ifdef __cplusplus
extern "C" {
#endif
#include "rockchip_drm.h"
#include "rockchip_drmif.h"
#include "rockchip_rga.h"
#ifdef __cplusplus
}
#endif

#include <utils/Mutex.h>

#define DRM_MODULE_NAME		"rockchip"

#define UNUSED(...) (void)(__VA_ARGS__)

struct rgaContext{
    rga_device_t device;

    rga_context *ctx;
    struct rockchip_device *dev;
    gralloc_module_t const *mAllocMod;
};
#endif
