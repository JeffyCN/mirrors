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

#ifndef _rockchip_rga_api_h_
#define _rockchip_rga_api_h_
#ifndef __cplusplus
extern "C" {
#endif
int         RgaInit(void **ctx);
int         RgaDeInit(void *ctx);
int         RgaBlit(rga_info *src, rga_info *dst, rga_info *src1);
#ifdef __cplusplus
int         RgaBlit(buffer_handle_t src, buffer_handle_t dst,
                                  drm_rga_t *rects, int rotation, int blend);
int         RgaBlit(void *src, buffer_handle_t dst,
                                  drm_rga_t *rects, int rotation, int blend);
int         RgaBlit(buffer_handle_t src, void *dst,
                                  drm_rga_t *rects, int rotation, int blend);
int         RgaBlit(void *src, void *dst,
                                  drm_rga_t *rects, int rotation, int blend);
#endif
#ifndef __cplusplus
}
#endif
#endif
