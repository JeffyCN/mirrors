/*
 * Copyright (C) 2020 Rockchip Electronics Co.Ltd
 * Authors:
 *	PutinLee <putin.lee@rock-chips.com>
 *  Cerf Yu <cerf.yu@rock-chips.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 */

#include "im2d.hpp"

#ifdef ANDROID
#include <RockchipRga.h>
#include "normal/NormalRga.h"
#endif

#ifdef LINUX
#include "../RockchipRga.h"
#include "../normal/NormalRga.h"
#endif

#include <sstream>

#ifdef ANDROID
#include <cutils/properties.h>

using namespace android;

RockchipRga& rkRga(RockchipRga::get());

#endif

#ifdef LINUX

#include <sys/ioctl.h>

#define ALOGE(...) printf(__VA_ARGS__)

RockchipRga rkRga;

#endif

#define ALIGN(val, align) (((val) + ((align) - 1)) & ~((align) - 1))

using namespace std;

IM_API rga_buffer_t wrapbuffer_virtualaddr(void* vir_addr, int width, int height, int wstride, int hstride, int format)
{
    rga_buffer_t buffer;

    memset(&buffer, 0, sizeof(rga_buffer_t));

    buffer.vir_addr = vir_addr;
    buffer.width    = width;
    buffer.height   = height;
    buffer.wstride  = wstride;
    buffer.hstride  = hstride;
    buffer.format   = format;

    return buffer;
}

IM_API rga_buffer_t wrapbuffer_physicaladdr(void* phy_addr, int width, int height, int wstride, int hstride, int format)
{
    rga_buffer_t buffer;

    memset(&buffer, 0, sizeof(rga_buffer_t));

    buffer.phy_addr = phy_addr;
    buffer.width    = width;
    buffer.height   = height;
    buffer.wstride  = wstride;
    buffer.hstride  = hstride;
    buffer.format   = format;

    return buffer;
}

IM_API rga_buffer_t wrapbuffer_fd(int fd, int width, int height, int wstride, int hstride, int format)
{
    rga_buffer_t buffer;

    memset(&buffer, 0, sizeof(rga_buffer_t));

    buffer.fd      = fd;
    buffer.width   = width;
    buffer.height  = height;
    buffer.wstride = wstride;
    buffer.hstride = hstride;
    buffer.format  = format;

    return buffer;
}

#ifdef ANDROID
/*When wrapbuffer_GraphicBuffer and wrapbuffer_AHardwareBuffer are used, */
/*it is necessary to check whether fd and virtual address of the return rga_buffer_t are valid parameters*/
IM_API rga_buffer_t wrapbuffer_GraphicBuffer(sp<GraphicBuffer> buf)
{
    rga_buffer_t buffer;
    int ret = 0;

    memset(&buffer, 0, sizeof(rga_buffer_t));

    ret = rkRga.RkRgaGetBufferFd(buf->handle, &buffer.fd);
    if (ret)
        ALOGE("rga_im2d: get buffer fd fail: %s, hnd=%p", strerror(errno), (void*)(buf->handle));

    if (buffer.fd <= 0)
    {
        ret = RkRgaGetHandleMapAddress(buf->handle, &buffer.vir_addr);
        if(!buffer.vir_addr)
            ALOGE("rga_im2d: invaild GraphicBuffer");
    }

    buffer.width   = buf->getWidth();
    buffer.height  = buf->getHeight();
    buffer.wstride = buf->getStride();
    buffer.hstride = buf->getHeight();
    buffer.format  = buf->getPixelFormat();

    return buffer;
}

IM_API rga_buffer_t wrapbuffer_AHardwareBuffer(AHardwareBuffer *buf)
{
	rga_buffer_t buffer;
	int ret = 0;

    memset(&buffer, 0, sizeof(rga_buffer_t));

    GraphicBuffer *gbuffer = GraphicBuffer::fromAHardwareBuffer(buf);

    ret = rkRga.RkRgaGetBufferFd(gbuffer->handle, &buffer.fd);
    if (ret)
        ALOGE("rga_im2d: get buffer fd fail: %s, hnd=%p", strerror(errno), (void*)(gbuffer->handle));

    if (buffer.fd <= 0)
    {
        ret = RkRgaGetHandleMapAddress(gbuffer->handle, &buffer.vir_addr);
        if(!buffer.vir_addr)
            ALOGE("rga_im2d: invaild GraphicBuffer");
    }

    buffer.width   = gbuffer->getWidth();
    buffer.height  = gbuffer->getHeight();
    buffer.wstride = gbuffer->getStride();
    buffer.hstride = gbuffer->getHeight();
    buffer.format  = gbuffer->getPixelFormat();

	return buffer;
}
#endif

IM_API IM_STATUS rga_set_buffer_info(rga_buffer_t dst, rga_info_t* dstinfo)
{
    if(NULL == dstinfo)
    {
        ALOGE("rga_im2d: invaild dstinfo");
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

IM_API IM_STATUS rga_set_buffer_info(const rga_buffer_t src, rga_buffer_t dst, rga_info_t* srcinfo, rga_info_t* dstinfo)
{
    if(NULL == srcinfo)
    {
        ALOGE("rga_im2d: invaild srcinfo");
        return IM_STATUS_INVALID_PARAM;
    }
    if(NULL == dstinfo)
    {
        ALOGE("rga_im2d: invaild dstinfo");
        return IM_STATUS_INVALID_PARAM;
    }

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

#define LIBRGA    0
#define RGA_IM2D  1

IM_API const char* querystring(int name)
{
    bool all_output = 0, all_output_prepared = 0;
    char buf[16];
#ifdef ANDROID
    char version_value[PROPERTY_VALUE_MAX];
#endif
    int rgafd, rga_version = 0;
    const char *temp;
    const char *output_vendor = "Rockchip Electronics Co.,Ltd.";
    const char *output_name[] = {
        "RGA vendor : ",
        "RGA version: ",
        "Max input  : ",
        "Max output : ",
        "Scale limit: ",
        "Input support format : ",
        "output support format: "
    };
    const char *version_name[] = {
        "librga version   : ",
        "rga_im2d version : "
    };
    const char *output_version[] = {
        "unknown",
        "RGA_1",
        "RGA_1_plus",
        "RGA_2",
        "RGA_2_lite0",
        "RGA_2_lite1",
        "RGA_2_Enhance"
    };
    const char *output_resolution[] = {
        "unknown",
        "2048x2048",
        "4096x4096",
        "8192x8192"
    };
    const char *output_scale_limit[] = {
        "unknown",
        "0.125 ~ 8",
        "0.0625 ~ 16"
    };
    const char *output_format[] = {
        "unknown",
        "RGBA_8888 RGBA_4444 RGBA_5551 RGB_565 RGB_888 ",
        "BPP8 BPP4 BPP2 BPP1 ",
        "YUV420/YUV422 ",
        "YUV420_10bit/YUV422_10bit ",
        "YUYV ",
        "YUV400/Y4 "
    };
    ostringstream out;
    string info;

#ifdef ANDROID
    property_set("vendor.rga_im2d.version", RGA_IM2D_VERSION);
#endif

    /*open /dev/rga node in order to get rga vesion*/
    rgafd = open("/dev/rga", O_RDWR, 0);
    if (rgafd < 0)
    {
        ALOGE("rga_im2d: failed to open /dev/rga: %s.",strerror(errno));
        return "err";
    }
    if (ioctl(rgafd, RGA_GET_VERSION, buf))
    {
        ALOGE("rga_im2d: rga get version fail: %s",strerror(errno));
        return "err";
    }
    if (strncmp(buf,"1.3",3) == 0)
        rga_version = RGA_1;
    else if (strncmp(buf,"1.6",3) == 0)
        rga_version = RGA_1_PLUS;
    /*3288 vesion is 2.00*/
    else if (strncmp(buf,"2.00",4) == 0)
        rga_version = RGA_2;
    /*3288w version is 3.00*/
    else if (strncmp(buf,"3.00",4) == 0)
        rga_version = RGA_2;
    else if (strncmp(buf,"3.02",4) == 0)
        rga_version = RGA_2_ENHANCE;
    else if (strncmp(buf,"4.00",4) == 0)
        rga_version = RGA_2_LITE0;
    /*The version number of lite1 cannot be obtained temporarily.*/
    else if (strncmp(buf,"4.00",4) == 0)
        rga_version = RGA_2_LITE1;
    else
        rga_version = RGA_V_ERR;

    close(rgafd);
    rgafd = -1;

    do{
        switch(name)
        {
            case RGA_VENDOR :
                    out << output_name[name] << output_vendor << endl;
                break;

            case RGA_VERSION :
                switch(rga_version)
                {
                    case RGA_1 :
                        out << output_name[name] << output_version[RGA_1] << endl;
                        break;
                    case RGA_1_PLUS :
                        out << output_name[name] << output_version[RGA_1_PLUS] << endl;
                        break;
                    case RGA_2 :
                        out << output_name[name] << output_version[RGA_2] << endl;
                        break;
                    case RGA_2_LITE0 :
                        out << output_name[name] << output_version[RGA_2_LITE0] << endl;
                        break;
                    case RGA_2_LITE1 :
                        out << output_name[name] << output_version[RGA_2_LITE1] << endl;
                        break;
                    case RGA_2_ENHANCE :
                        out << output_name[name] << output_version[RGA_2_ENHANCE] << endl;
                        break;
                    case RGA_V_ERR :
                        out << output_name[name] << output_version[RGA_V_ERR] << endl;
                        break;
                    default:
                        return "err";
                }

#ifdef ANDROID
                property_get("vendor.rga.version", version_value, "0");
                out << version_name[LIBRGA] << version_value << endl;
                property_get("vendor.rga_im2d.version", version_value, "0");
                out << version_name[RGA_IM2D] << version_value << endl;
#endif
                break;

            case RGA_MAX_INPUT :
                switch(rga_version)
                {
                    case RGA_1 :
                    case RGA_1_PLUS :
                    case RGA_2 :
                    case RGA_2_LITE0 :
                    case RGA_2_LITE1 :
                    case RGA_2_ENHANCE :
                        out << output_name[name] << output_resolution[3] << endl;
                        break;
                    case RGA_V_ERR :
                        out << output_name[name] << output_resolution[0] << endl;
                        break;
                    default:
                        return "err";
                }
                break;

            case RGA_MAX_OUTPUT :
                switch(rga_version)
                {
                    case RGA_1 :
                    case RGA_1_PLUS :
                        out << output_name[name] << output_resolution[1] << endl;
                        break;
                    case RGA_2 :
                    case RGA_2_LITE0 :
                    case RGA_2_LITE1 :
                    case RGA_2_ENHANCE :
                        out << output_name[name] << output_resolution[2] << endl;
                        break;
                    case RGA_V_ERR :
                        out << output_name[name] << output_resolution[0] << endl;
                        break;
                    default:
                        return "err";
                }
                break;

            case RGA_SCALE_LIMIT :
                switch(rga_version)
                {
                    case RGA_1 :
                    case RGA_1_PLUS :
                    case RGA_2_LITE0 :
                    case RGA_2_LITE1 :
                        out << output_name[name] << output_scale_limit[1] << endl;
                    case RGA_2 :
                    case RGA_2_ENHANCE :
                        out << output_name[name] << output_scale_limit[2] << endl;
                        break;
                    case RGA_V_ERR :
                        out << output_name[name] << output_resolution[0] << endl;
                        break;
                    default:
                        return "err";
                }
                break;

            case RGA_INPUT_FORMAT :
                switch(rga_version)
                {
                    case RGA_1 :
                    case RGA_1_PLUS :
                        out << output_name[name] << output_format[1] << output_format[2] << output_format[3] << endl;
                    case RGA_2 :
                    case RGA_2_LITE0 :
                        out << output_name[name] << output_format[1] << output_format[3] << endl;
                        break;
                    case RGA_2_LITE1 :
                    case RGA_2_ENHANCE :
                        out << output_name[name] << output_format[1] << output_format[3] << output_format[4] << endl;
                        break;
                    case RGA_V_ERR :
                        out << output_name[name] << output_format[0] << endl;
                        break;
                    default:
                        return "err";
                }
                break;

            case RGA_OUTPUT_FORMAT :
                switch(rga_version)
                {
                    case RGA_1 :
                        out << output_name[name] << output_format[1] << output_format[3] << endl;
                        break;
                    case RGA_1_PLUS :
                        out << output_name[name] << output_format[1] << output_format[3] << endl;
                        break;
                    case RGA_2 :
                        out << output_name[name] << output_format[1] << output_format[3] << endl;
                        break;
                    case RGA_2_LITE0 :
                        out << output_name[name] << output_format[1] << output_format[3] << endl;
                        break;
                    case RGA_2_LITE1 :
                        out << output_name[name] << output_format[1] << output_format[3] << output_format[4] << endl;
                        break;
                    case RGA_2_ENHANCE :
                        out << output_name[name] << output_format[1] << output_format[3] << output_format[4] << output_format[5] << endl;
                        break;
                    case RGA_V_ERR :
                        out << output_name[name] << output_format[0] << endl;
                        break;
                    default:
                        return "err";
                }
                break;

            case RGA_ALL :
                if (!all_output)
                {
                    all_output = 1;
                    name = 0;
                }
                else
                    all_output_prepared = 1;
                break;

            default:
                return "err";
        }

        info = out.str();

        if (all_output_prepared)
            break;
        else if (all_output && strcmp(info.c_str(),"0")>0)
            name++;

    }while(all_output);

    temp = info.c_str();

    return temp;
}

IM_API IM_STATUS imresize_t(const rga_buffer_t src, rga_buffer_t dst, double fx, double fy, int interpolation, int sync)
{
    int usage = 0;
    int ret = IM_STATUS_NOERROR;
    im_rect srect;
    im_rect drect;

    if (fx > 0 || fy > 0)
    {
        if (fx == 0) fx = 1;
        if (fy == 0) fy = 1;

        dst.width = (int)(src.width * fx);
        dst.height = (int)(src.height * fy);

        if(NormalRgaIsYuvFormat(RkRgaGetRgaFormat(src.format)))
            dst.width = ALIGN(dst.width, 2);
    }

    if (sync == 0)
        usage |= IM_SYNC;

    ret = improcess(src, dst, srect, drect, usage);
    if (!ret)
        return IM_STATUS_FAILED;

    return IM_STATUS_SUCCESS;
}

IM_API IM_STATUS imcrop_t(const rga_buffer_t src, rga_buffer_t dst, im_rect rect, int sync)
{
    int usage = 0;
    int ret = IM_STATUS_NOERROR;

    im_rect drect;

    usage |= IM_CROP;

    if (sync == 0)
        usage |= IM_SYNC;

    ret = improcess(src, dst, rect, drect, usage);
    if (!ret)
        return IM_STATUS_FAILED;

    return IM_STATUS_SUCCESS;
}

IM_API IM_STATUS imrotate_t(const rga_buffer_t src, rga_buffer_t dst, int rotation, int sync)
{
    int usage = 0;
    int ret = IM_STATUS_NOERROR;
    im_rect srect;
    im_rect drect;

    usage |= rotation;

    if (sync == 0)
        usage |= IM_SYNC;

    ret = improcess(src, dst, srect, drect, usage);
    if (!ret)
        return IM_STATUS_FAILED;

    return IM_STATUS_SUCCESS;
}

IM_API IM_STATUS imflip_t (const rga_buffer_t src, rga_buffer_t dst, int mode, int sync)
{
    int usage = 0;
    int ret = IM_STATUS_NOERROR;
    im_rect srect;
    im_rect drect;

    usage |= mode;

    if (sync == 0)
        usage |= IM_SYNC;

    ret = improcess(src, dst, srect, drect, usage);
    if (!ret)
        return IM_STATUS_FAILED;

    return IM_STATUS_SUCCESS;
}

IM_API IM_STATUS imfill_t(rga_buffer_t dst, im_rect rect, int color, int sync)
{
    int usage = 0;
    int ret = IM_STATUS_NOERROR;
    im_rect srect;
    im_rect drect;

    rga_buffer_t src;

    memset(&src, 0, sizeof(rga_buffer_t));

    usage |= IM_COLOR_FILL;

    dst.color = color;

    if (sync == 0)
        usage |= IM_SYNC;

    ret = improcess(src, dst, srect, rect, usage);
    if (!ret)
        return IM_STATUS_FAILED;

    return IM_STATUS_SUCCESS;
}

IM_API IM_STATUS imtranslate_t(const rga_buffer_t src, rga_buffer_t dst, int x, int y, int sync)
{
    int usage = 0;
    int ret = IM_STATUS_NOERROR;
    im_rect srect;
    im_rect drect;

    if ((src.width != dst.width) || (src.height != dst.height))
        return IM_STATUS_INVALID_PARAM;

    if (sync == 0)
        usage |= IM_SYNC;

    srect.width = src.width - x;
    srect.height = src.height - y;
    drect.x = x;
    drect.y = y;
    drect.width = src.width - x;
    drect.height = src.height - y;

    ret = improcess(src, dst, srect, drect, usage);
    if (!ret)
        return IM_STATUS_FAILED;

    return IM_STATUS_SUCCESS;
}

IM_API IM_STATUS imcopy_t(const rga_buffer_t src, rga_buffer_t dst, int sync)
{
    int usage = 0;
    int ret = IM_STATUS_NOERROR;
    im_rect srect;
    im_rect drect;

    if ((src.width != dst.width) || (src.height != dst.height))
        return IM_STATUS_INVALID_PARAM;

    if (sync == 0)
        usage |= IM_SYNC;

    ret = improcess(src, dst, srect, drect, usage);
    if (!ret)
        return IM_STATUS_FAILED;

    return IM_STATUS_SUCCESS;
}

IM_API IM_STATUS imblend_t(const rga_buffer_t srcA, const rga_buffer_t srcB, rga_buffer_t dst, int mode, int sync)
{
    int usage = 0;
    int ret = IM_STATUS_NOERROR;
    im_rect srect;
    im_rect drect;

    usage |= mode;

    if (sync == 0)
        usage |= IM_SYNC;

    ret = improcess(srcA, dst, srect, drect, usage);
    if (!ret)
        return IM_STATUS_FAILED;

    return IM_STATUS_SUCCESS;
}

IM_API IM_STATUS imcvtcolor_t(rga_buffer_t src, rga_buffer_t dst, int sfmt, int dfmt, int mode, int sync)
{
    int usage = 0;
    int ret = IM_STATUS_NOERROR;
    im_rect srect;
    im_rect drect;

    src.format = sfmt;
    dst.format = dfmt;

    dst.color_space_mode = mode;

    if (sync == 0)
        usage |= IM_SYNC;

    ret = improcess(src, dst, srect, drect, usage);
    if (!ret)
        return IM_STATUS_FAILED;

    return IM_STATUS_SUCCESS;
}

IM_API IM_STATUS imquantize_t(const rga_buffer_t src, rga_buffer_t dst, rga_nn_t nn_info, int sync)
{
    return IM_STATUS_SUCCESS;
}

IM_API IM_STATUS imcheck(rga_buffer_t src, rga_buffer_t dst, im_rect srect, im_rect drect, int usage)
{
    if ((srect.width + srect.x > src.width) || (srect.height + srect.y > src.height))
    {
        ALOGE("rga_im2d: invaild src rect");
        return IM_STATUS_INVALID_PARAM;
    }

    /* rect judgment */
    if ((drect.width + drect.x > dst.width) || (drect.height + drect.y > dst.height))
    {
        ALOGE("rga_im2d: invaild dst rect");
        return IM_STATUS_INVALID_PARAM;
    }

    return IM_STATUS_NOERROR;
}

IM_API IM_STATUS improcess(rga_buffer_t src, rga_buffer_t dst, im_rect srect, im_rect drect, int usage)
{
    rga_info_t srcinfo;
    rga_info_t dstinfo;
    int ret;

    memset(&srcinfo, 0, sizeof(rga_info_t));
    memset(&dstinfo, 0, sizeof(rga_info_t));

    if (usage & IM_COLOR_FILL)
        ret = rga_set_buffer_info(dst, &dstinfo);
    else
        ret = rga_set_buffer_info(src, dst, &srcinfo, &dstinfo);

    if (ret < 0)
        return IM_STATUS_INVALID_PARAM;

    if(IM_STATUS_NOERROR != imcheck(src, dst, srect, drect, usage))
        return IM_STATUS_INVALID_PARAM;

    if (srect.width > 0 && srect.height > 0)
    {
        src.width = srect.width;
        src.height = srect.height;
        /* for imcrop_t api */
        if (usage & IM_CROP)
        {
            dst.width = srect.width;
            dst.height = srect.height;
        }
    }

    if (drect.width > 0 && drect.height > 0)
    {
        dst.width = drect.width;
        dst.height = drect.height;
    }

    rga_set_rect(&srcinfo.rect, srect.x, srect.y, src.width, src.height, src.wstride, src.hstride, src.format);
    rga_set_rect(&dstinfo.rect, drect.x, drect.y, dst.width, dst.height, dst.wstride, dst.hstride, dst.format);

    if((usage & (IM_ALPHA_BLEND_MASK+IM_HAL_TRANSFORM_MASK)) != 0)
    {
        /* Transform */
        switch(usage & IM_HAL_TRANSFORM_MASK)
        {
            case IM_HAL_TRANSFORM_ROT_90:
                srcinfo.rotation = HAL_TRANSFORM_ROT_90;
                break;
            case IM_HAL_TRANSFORM_ROT_180:
                srcinfo.rotation = HAL_TRANSFORM_ROT_180;
                break;
            case IM_HAL_TRANSFORM_ROT_270:
                srcinfo.rotation = HAL_TRANSFORM_ROT_270;
                break;
            case IM_HAL_TRANSFORM_FLIP_V:
                srcinfo.rotation = HAL_TRANSFORM_FLIP_V;
                break;
            case IM_HAL_TRANSFORM_FLIP_H:
                srcinfo.rotation = HAL_TRANSFORM_FLIP_H;
                break;
        }

        /* Blend */
        switch(usage & IM_ALPHA_BLEND_MASK)
        {
            case IM_ALPHA_BLEND_DST:
                srcinfo.blend = 0xff0105;
                break;
            case IM_ALPHA_BLEND_SRC_OVER:
                break;
            case IM_ALPHA_BLEND_SRC_IN:
                break;
            case IM_ALPHA_BLEND_DST_IN:
                break;
            case IM_ALPHA_BLEND_SRC_OUT:
                break;
            case IM_ALPHA_BLEND_DST_OVER:
                srcinfo.blend = 0xff0501;
                break;
            case IM_ALPHA_BLEND_SRC_ATOP:
                break;
            case IM_ALPHA_BLEND_DST_OUT:
                srcinfo.blend = 0xff0405;
                break;
            case IM_ALPHA_BLEND_XOR:
                break;
        }
        if(srcinfo.blend == 0 && srcinfo.rotation ==0)
            ALOGE("rga_im2d: Could not find blend/rotate/flip usage : 0x%x \n", usage);
    }

    /* set global alpha */
    if ((src.global_alpha > 0) && (usage & IM_ALPHA_BLEND_MASK))
        srcinfo.blend &= src.global_alpha << 16;

    /* special config for yuv to rgb */
    if (dst.color_space_mode & (IM_YUV_TO_RGB_MASK))
    {
        if (NormalRgaIsYuvFormat(RkRgaGetRgaFormat(src.format)) &&
			NormalRgaIsRgbFormat(RkRgaGetRgaFormat(dst.format)))
            dstinfo.color_space_mode = dst.color_space_mode;
        else
            return IM_STATUS_INVALID_PARAM;
    }

    /* special config for rgb to yuv */
    if (dst.color_space_mode & (IM_RGB_TO_YUV_MASK))
    {
        if (NormalRgaIsRgbFormat(RkRgaGetRgaFormat(src.format)) &&
			NormalRgaIsYuvFormat(RkRgaGetRgaFormat(dst.format)))
            dstinfo.color_space_mode = dst.color_space_mode;
        else
            return IM_STATUS_INVALID_PARAM;
    }

    if (usage & IM_SYNC)
        dstinfo.sync_mode = RGA_BLIT_ASYNC;

    if (usage & IM_COLOR_FILL)
    {
        dstinfo.color = dst.color;
        ret = rkRga.RkRgaCollorFill(&dstinfo);
    }
    else
        ret = rkRga.RkRgaBlit(&srcinfo, &dstinfo, NULL);

    if (ret)
        return IM_STATUS_FAILED;

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
