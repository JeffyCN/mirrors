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

#include <RockchipRga.h>
#include "normal/NormalRga.h"

#define ALIGN(val, align) (((val) + ((align) - 1)) & ~((align) - 1))

using namespace android;

RockchipRga& rkRga(RockchipRga::get());

IM_API buffer_t warpbuffer_virtualaddr(void* vir_addr, int width, int height, int wstride, int hstride, int format)
{
    buffer_t buffer;

    memset(&buffer, 0, sizeof(buffer_t));

    buffer.vir_addr = vir_addr;
    buffer.width = width;
    buffer.height = height;
    buffer.wstride = wstride;
    buffer.hstride = hstride;
    buffer.format = format;

    return buffer;
}

IM_API buffer_t warpbuffer_physicaladdr(void* phy_addr, int width, int height, int wstride, int hstride, int format)
{
    buffer_t buffer;

    memset(&buffer, 0, sizeof(buffer_t));

    buffer.phy_addr = phy_addr;
    buffer.width = width;
    buffer.height = height;
    buffer.wstride = wstride;
    buffer.hstride = hstride;
    buffer.format = format;

    return buffer;
}

IM_API buffer_t warpbuffer_fd(int fd, int width, int height, int wstride, int hstride, int format)
{
    buffer_t buffer;

    memset(&buffer, 0, sizeof(buffer_t));

    buffer.fd = fd;
    buffer.width = width;
    buffer.height = height;
    buffer.wstride = wstride;
    buffer.hstride = hstride;
    buffer.format = format;

    return buffer;
}

IM_API int rga_set_buffer_info(buffer_t dst, rga_info_t* dstinfo)
{
    if(dst.phy_addr != NULL)
        dstinfo->phyAddr= dst.phy_addr;
    else if(dst.fd > 0)
    {
        dstinfo->fd = dst.fd;
        dstinfo->mmuFlag = 1;
    }
    else if(dst.vir_addr != NULL)
    {
        dstinfo->virAddr = dst.vir_addr;
        dstinfo->mmuFlag = 1;
    }
    else
    {
        ALOGE("rga_im2d: invaild dst buffer");
        return IM_STATUS_INVALID_PARAM;
    }

    return IM_STATUS_SUCCESS;
}

IM_API int rga_set_buffer_info(const buffer_t src, buffer_t dst, rga_info_t* srcinfo, rga_info_t* dstinfo)
{
    if(src.phy_addr != NULL)
        srcinfo->phyAddr = src.phy_addr;
    else if(src.fd > 0)
    {
        srcinfo->fd = src.fd;
        srcinfo->mmuFlag = 1;
    }
    else if(src.vir_addr != NULL)
    {
        srcinfo->virAddr = src.vir_addr;
        srcinfo->mmuFlag = 1;
    }
    else
    {
        ALOGE("rga_im2d: invaild src buffer");
        return IM_STATUS_INVALID_PARAM;
    }

    if(dst.phy_addr != NULL)
        dstinfo->phyAddr= dst.phy_addr;
    else if(dst.fd > 0)
    {
        dstinfo->fd = dst.fd;
        dstinfo->mmuFlag = 1;
    }
    else if(dst.vir_addr != NULL)
    {
        dstinfo->virAddr = dst.vir_addr;
        dstinfo->mmuFlag = 1;
    }
    else
    {
        ALOGE("rga_im2d: invaild dst buffer");
        return IM_STATUS_INVALID_PARAM;
    }

    return IM_STATUS_SUCCESS;
}

IM_API const char* querystring(int name)
{
    const char* temp = "rk-debug";

    return temp;
}

IM_API IM_STATUS imresize_t(const buffer_t src, buffer_t dst, double fx, double fy, int interpolation, int sync)
{
    rga_info_t srcinfo;
    rga_info_t dstinfo;
    int ret;

    memset(&srcinfo, 0, sizeof(rga_info_t));
    memset(&dstinfo, 0, sizeof(rga_info_t));

    ret = rga_set_buffer_info(src, dst, &srcinfo, &dstinfo);
    if (ret < 0)
        return IM_STATUS_INVALID_PARAM;

    if (fx > 0 || fy > 0)
    {
        if (fx == 0) fx = 1;
        if (fy == 0) fy = 1;

        dst.width = (int)(src.width * fx);
        dst.height = (int)(src.height * fy);

        if(NormalRgaIsYuvFormat(RkRgaGetRgaFormat(src.format)))
            dst.width = ALIGN(dst.width, 2);
    }

    rga_set_rect(&srcinfo.rect, 0, 0, src.width, src.height, src.wstride, src.hstride, src.format);
    rga_set_rect(&dstinfo.rect, 0, 0, dst.width, dst.height, dst.wstride, dst.hstride, dst.format);

    if (sync == 0)
        dstinfo.sync_mode = RGA_BLIT_ASYNC;

    ret = rkRga.RkRgaBlit(&srcinfo, &dstinfo, NULL);
    if (ret)
        return IM_STATUS_FAILED;
    else
        return IM_STATUS_SUCCESS;
}

IM_API IM_STATUS imcrop_t(const buffer_t src, buffer_t dst, im_rect rect, int sync)
{
    rga_info_t srcinfo;
    rga_info_t dstinfo;
    int ret;

    memset(&srcinfo, 0, sizeof(rga_info_t));
    memset(&dstinfo, 0, sizeof(rga_info_t));

    ret = rga_set_buffer_info(src, dst, &srcinfo, &dstinfo);
    if (ret < 0)
        return IM_STATUS_INVALID_PARAM;

    if ((rect.width + rect.x > src.width) || (rect.height + rect.y > src.height))
    {
        ALOGE("rga_im2d: invaild rect");
        return IM_STATUS_INVALID_PARAM;
    }

    rga_set_rect(&srcinfo.rect, rect.x, rect.y, rect.width, rect.height, src.wstride, src.hstride, src.format);
    rga_set_rect(&dstinfo.rect, 0, 0, rect.width, rect.height, dst.wstride, dst.hstride, dst.format);

    if (sync == 0)
        dstinfo.sync_mode = RGA_BLIT_ASYNC;

    ret = rkRga.RkRgaBlit(&srcinfo, &dstinfo, NULL);
    if (ret)
        return IM_STATUS_FAILED;
    else
        return IM_STATUS_SUCCESS;
}

IM_API IM_STATUS imrotate_t(const buffer_t src, buffer_t dst, int rotation, int sync)
{
    rga_info_t srcinfo;
    rga_info_t dstinfo;
    int ret;

    memset(&srcinfo, 0, sizeof(rga_info_t));
    memset(&dstinfo, 0, sizeof(rga_info_t));

    ret = rga_set_buffer_info(src, dst, &srcinfo, &dstinfo);
    if (ret < 0)
        return IM_STATUS_INVALID_PARAM;

    srcinfo.rotation = rotation;

    rga_set_rect(&srcinfo.rect, 0, 0, src.width, src.height, src.wstride, src.hstride, src.format);
    rga_set_rect(&dstinfo.rect, 0, 0, dst.width, dst.height, dst.wstride, dst.hstride, dst.format);

    if (sync == 0)
        dstinfo.sync_mode = RGA_BLIT_ASYNC;

    ret = rkRga.RkRgaBlit(&srcinfo, &dstinfo, NULL);
    if (ret)
        return IM_STATUS_FAILED;
    else
        return IM_STATUS_SUCCESS;
}

IM_API IM_STATUS imflip_t (const buffer_t src, buffer_t dst, int mode, int sync)
{
    rga_info_t srcinfo;
    rga_info_t dstinfo;
    int ret;

    memset(&srcinfo, 0, sizeof(rga_info_t));
    memset(&dstinfo, 0, sizeof(rga_info_t));

    ret = rga_set_buffer_info(src, dst, &srcinfo, &dstinfo);
    if (ret < 0)
        return IM_STATUS_INVALID_PARAM;

    srcinfo.rotation = mode;

    rga_set_rect(&srcinfo.rect, 0, 0, src.width, src.height, src.wstride, src.hstride, src.format);
    rga_set_rect(&dstinfo.rect, 0, 0, dst.width, dst.height, dst.wstride, dst.hstride, dst.format);

    if (sync == 0)
        dstinfo.sync_mode = RGA_BLIT_ASYNC;

    ret = rkRga.RkRgaBlit(&srcinfo, &dstinfo, NULL);
    if (ret)
        return IM_STATUS_FAILED;
    else
        return IM_STATUS_SUCCESS;
}

IM_API IM_STATUS imfill_t(const buffer_t src, buffer_t dst, im_rect rect, unsigned char color, int sync)
{
    rga_info_t dstinfo;
    int ret;

    memset(&dstinfo, 0, sizeof(rga_info_t));

    ret = rga_set_buffer_info(dst, &dstinfo);
    if (ret < 0)
        return IM_STATUS_INVALID_PARAM;

    dstinfo.color = color;

    rga_set_rect(&dstinfo.rect, rect.x, rect.y, rect.width, rect.height, dst.wstride, dst.hstride, dst.format);

    if (sync == 0)
        dstinfo.sync_mode = RGA_BLIT_ASYNC;

    ret = rkRga.RkRgaCollorFill(&dstinfo);
    if (ret)
        return IM_STATUS_FAILED;
    else
        return IM_STATUS_SUCCESS;

    return IM_STATUS_SUCCESS;
}

IM_API IM_STATUS imtranslate_t(const buffer_t src, buffer_t dst, int x, int y, int sync)
{
    return IM_STATUS_SUCCESS;
}

IM_API IM_STATUS imcopy_t(const buffer_t src, buffer_t dst, int sync)
{
    rga_info_t srcinfo;
    rga_info_t dstinfo;
    int ret;

    memset(&srcinfo, 0, sizeof(rga_info_t));
    memset(&dstinfo, 0, sizeof(rga_info_t));

    ret = rga_set_buffer_info(src, dst, &srcinfo, &dstinfo);
    if (ret < 0)
        return IM_STATUS_INVALID_PARAM;

    if (src.width != dst.width || src.height != dst.height)
        return IM_STATUS_INVALID_PARAM;

    rga_set_rect(&srcinfo.rect, 0, 0, src.width, src.height, src.wstride, src.hstride, src.format);
    rga_set_rect(&dstinfo.rect, 0, 0, dst.width, dst.height, dst.wstride, dst.hstride, dst.format);

    if (sync == 0)
        dstinfo.sync_mode = RGA_BLIT_ASYNC;

    ret = rkRga.RkRgaBlit(&srcinfo, &dstinfo, NULL);
    if (ret)
        return IM_STATUS_FAILED;
    else
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
    int ret = 0;
    ret = rkRga.RkRgaFlush();
    if (ret)
        return IM_STATUS_FAILED;

    return IM_STATUS_SUCCESS;
}