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

#ifndef _rockchip_rga_drm_gralloc_h_
#define _rockchip_rga_drm_gralloc_h_

#ifndef RK3368
#include <gralloc_priv.h>

#else
#include <hardware/img_gralloc_public.h>
#include <hardware/gralloc.h>

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
#define	GRALLOC_MODULE_PERFORM_GET_USAGE 0x0feeff03
#endif
#endif

#endif

