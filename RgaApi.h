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
#ifdef __cplusplus
extern "C" {
#endif
int         RgaInit(void **ctx);
int         RgaDeInit(void *ctx);
int         RgaBlit(rga_info_t *src, rga_info_t *dst, rga_info_t *src1);
int         RgaCollorFill(rga_info_t *dst);
int         RgaFlush();
#ifdef __cplusplus
}
#endif
#endif
