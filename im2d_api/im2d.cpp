/*
 * Copyright (C) 2016 Rockchip Electronics Co.Ltd
 * Authors:
 *	PutinLee <putin.lee@rock-chips.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 */

#include "im2d.h"


IM_API const char* querystring(int name)
{
    const char* temp = "rk-debug";

    return temp;
}

IM_API IM_STATUS imresize_t(const buffer_t src, buffer_t dst, double fx, double fy, int interpolation, int sync)
{
    return IM_STATUS_SUCCESS;
}

IM_API IM_STATUS imcrop_t(const buffer_t src, buffer_t dst, rga_rect srect, rga_rect drect, int sync)
{
    return IM_STATUS_SUCCESS;
}

IM_API IM_STATUS imrotate_t(const buffer_t src, buffer_t dst, int rotation, int sync)
{
    return IM_STATUS_SUCCESS;
}

IM_API IM_STATUS imflip_t (const buffer_t src, buffer_t dst, int mode, int sync)
{
    return IM_STATUS_SUCCESS;
}

IM_API IM_STATUS imfill_t(const buffer_t src, buffer_t dst, rga_rect rect, unsigned char color, int sync)
{
    return IM_STATUS_SUCCESS;
}

IM_API IM_STATUS imreset_t(const buffer_t src, buffer_t dst, rga_rect rect, unsigned char color, int sync)
{
    return IM_STATUS_SUCCESS;
}

IM_API IM_STATUS imdraw_t(const buffer_t src, buffer_t dst, rga_rect rect, unsigned char color, int sync)
{
    return IM_STATUS_SUCCESS;
}

IM_API IM_STATUS imtranslate_t(const buffer_t src, buffer_t dst, int x, int y, int sync)
{
    return IM_STATUS_SUCCESS;
}

IM_API IM_STATUS imcopy_t(const buffer_t src, buffer_t dst, int sync)
{
    return IM_STATUS_SUCCESS;
}

IM_API IM_STATUS imblend_t(const buffer_t srcA, const buffer_t srcB, buffer_t dst, int mode, int sync)
{
    return IM_STATUS_SUCCESS;
}

IM_API IM_STATUS imcvtcolor_t(const buffer_t src, buffer_t dst, int sfmt, int dfmt, int sync)
{
    return IM_STATUS_SUCCESS;
}

IM_API IM_STATUS imquantize_t(const buffer_t src, buffer_t dst, rga_nn_t nn_info, int sync)
{
    return IM_STATUS_SUCCESS;
}

IM_API IM_STATUS improcess_t(const buffer_t src, buffer_t dst, int sync, ...)
{
    return IM_STATUS_SUCCESS;
}

IM_API IM_STATUS imsync(void)
{
    return IM_STATUS_SUCCESS;
}