/*
 * Copyright (C) 2020 Rockchip Electronics Co., Ltd.
 * Authors:
 *  PutinLee <putin.lee@rock-chips.com>
 *  Cerf Yu <cerf.yu@rock-chips.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "im2d.hpp"

#ifdef ANDROID
#include <RockchipRga.h>
#include "core/NormalRga.h"
#endif

#ifdef LINUX
#include "../include/RockchipRga.h"
#include "../core/NormalRga.h"
#endif

#include "version.h"

#include <sstream>

#ifdef ANDROID
#include <cutils/properties.h>

using namespace android;

#endif

#ifdef LINUX

#include <sys/ioctl.h>

#define ALOGE(...) { printf(__VA_ARGS__); printf("\n"); }

#endif

typedef enum {
    RGA_LIB = 0,
    RGA_IM2D,
    RGA_BUILT,
} QUERYSTRING_API;

#define ALIGN(val, align) (((val) + ((align) - 1)) & ~((align) - 1))
#define DOWN_ALIGN(val, align) ((val) & ~((align) - 1))

#define UNUSED(...) (void)(__VA_ARGS__)
#define ERR_MSG_LEN 256

using namespace std;

ostringstream err_msg;
char *err_str = NULL;

IM_API void imErrorMsg(const char* msg) {
    err_msg.str("");
    err_msg << msg << endl;
}

IM_API const char* imStrError_t(IM_STATUS status) {
    const char *error_type[] = {
        "No errors during operation",
        "Run successfully",
        "Unsupported function: ",
        "Memory overflow: ",
        "Invalid parameters: ",
        "Illegal parameters: ",
        "Fatal error: ",
        "unkown status"
    };
    ostringstream error;

    if (err_str == NULL)
        err_str = (char *)malloc(ERR_MSG_LEN*sizeof(char));

    switch(status) {
        case IM_STATUS_NOERROR :
            error << error_type[0] << endl;
            break;
        case IM_STATUS_SUCCESS :
            error << error_type[1] << endl;
            break;

        case IM_STATUS_NOT_SUPPORTED :
            error << error_type[2] << err_msg.str().c_str() << endl;
            break;

        case IM_STATUS_OUT_OF_MEMORY :
            error << error_type[3] << err_msg.str().c_str() << endl;
            break;

        case IM_STATUS_INVALID_PARAM :
            error << error_type[4] << err_msg.str().c_str() << endl;
            break;

        case IM_STATUS_ILLEGAL_PARAM :
            error << error_type[5] << err_msg.str().c_str() << endl;
            break;

        case IM_STATUS_FAILED :
            error << error_type[6] << err_msg.str().c_str() << endl;
            break;
        default :
            error << error_type[7] << endl;
    }

    memcpy(err_str, error.str().c_str(), ERR_MSG_LEN);
    imErrorMsg("No error message, it has been cleared.");

    return err_str;
}

IM_API rga_buffer_t wrapbuffer_virtualaddr_t(void* vir_addr, int width, int height, int wstride, int hstride, int format) {
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

IM_API rga_buffer_t wrapbuffer_physicaladdr_t(void* phy_addr, int width, int height, int wstride, int hstride, int format) {
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

IM_API rga_buffer_t wrapbuffer_fd_t(int fd, int width, int height, int wstride, int hstride, int format) {
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
IM_API rga_buffer_t wrapbuffer_handle(buffer_handle_t hnd) {
    int ret = 0;
    rga_buffer_t buffer;
    std::vector<int> dstAttrs;

    RockchipRga& rkRga(RockchipRga::get());

    memset(&buffer, 0, sizeof(rga_buffer_t));

    ret = rkRga.RkRgaGetBufferFd(hnd, &buffer.fd);
    if (ret)
        ALOGE("rga_im2d: get buffer fd fail: %s, hnd=%p", strerror(errno), (void*)(hnd));

    if (buffer.fd <= 0) {
        ret = rkRga.RkRgaGetHandleMapCpuAddress(hnd, &buffer.vir_addr);
        if(!buffer.vir_addr) {
            ALOGE("rga_im2d: invaild GraphicBuffer, can not get fd and virtual address.");
            imErrorMsg("invaild GraphicBuffer, can not get fd and virtual address.");
            goto INVAILD;
        }
    }

    ret = RkRgaGetHandleAttributes(hnd, &dstAttrs);
    if (ret) {
        ALOGE("rga_im2d: handle get Attributes fail ret = %d,hnd=%p", ret, &hnd);
        imErrorMsg("handle get Attributes fail.");
        goto INVAILD;
    }

    buffer.width   = dstAttrs.at(AWIDTH);
    buffer.height  = dstAttrs.at(AHEIGHT);
    buffer.wstride = dstAttrs.at(ASTRIDE);
    buffer.hstride = dstAttrs.at(AHEIGHT);
    buffer.format  = dstAttrs.at(AFORMAT);

    if (buffer.width % 16) {
        ALOGE("rga_im2d: Graphicbuffer wstride needs align to 16, please align to 16 or use other buffer types.");
        imErrorMsg("Graphicbuffer wstride needs align to 16, please align to 16 or use other buffer types.");
        goto INVAILD;
    }

INVAILD:
    return buffer;
}

IM_API rga_buffer_t wrapbuffer_GraphicBuffer(sp<GraphicBuffer> buf) {
    rga_buffer_t buffer;
    int ret = 0;

    RockchipRga& rkRga(RockchipRga::get());

    memset(&buffer, 0, sizeof(rga_buffer_t));

    ret = rkRga.RkRgaGetBufferFd(buf->handle, &buffer.fd);
    if (ret)
        ALOGE("rga_im2d: get buffer fd fail: %s, hnd=%p", strerror(errno), (void*)(buf->handle));

    if (buffer.fd <= 0) {
        ret = rkRga.RkRgaGetHandleMapCpuAddress(buf->handle, &buffer.vir_addr);
        if(!buffer.vir_addr) {
            ALOGE("rga_im2d: invaild GraphicBuffer, can not get fd and virtual address.");
            imErrorMsg("invaild GraphicBuffer, can not get fd and virtual address.");
            goto INVAILD;
        }
    }
    if (buf->getWidth() % 16) {
        ALOGE("rga_im2d: Graphicbuffer wstride needs align to 16, please align to 16 or use other buffer types.");
        imErrorMsg("Graphicbuffer wstride needs align to 16, please align to 16 or use other buffer types.");
    }

    buffer.width   = buf->getWidth();
    buffer.height  = buf->getHeight();
    buffer.wstride = buf->getStride();
    buffer.hstride = buf->getHeight();
    buffer.format  = buf->getPixelFormat();

INVAILD:
    return buffer;
}

#if USE_AHARDWAREBUFFER
#include <android/hardware_buffer.h>
IM_API rga_buffer_t wrapbuffer_AHardwareBuffer(AHardwareBuffer *buf) {
    rga_buffer_t buffer;
    int ret = 0;

    RockchipRga& rkRga(RockchipRga::get());

    memset(&buffer, 0, sizeof(rga_buffer_t));

    GraphicBuffer *gbuffer = reinterpret_cast<GraphicBuffer*>(buf);

    ret = rkRga.RkRgaGetBufferFd(gbuffer->handle, &buffer.fd);
    if (ret)
        ALOGE("rga_im2d: get buffer fd fail: %s, hnd=%p", strerror(errno), (void*)(gbuffer->handle));

    if (buffer.fd <= 0) {
        ret = rkRga.RkRgaGetHandleMapCpuAddress(gbuffer->handle, &buffer.vir_addr);
        if(!buffer.vir_addr) {
            ALOGE("rga_im2d: invaild GraphicBuffer, can not get fd and virtual address.");
            imErrorMsg("invaild GraphicBuffer, can not get fd and virtual address.");
            goto INVAILD;
        }
    }

    if (gbuffer->getWidth() % 16) {
        ALOGE("rga_im2d: Graphicbuffer wstride needs align to 16, please align to 16or use other buffer types");
        imErrorMsg("Graphicbuffer wstride needs align to 16, please align to 16 or use other buffer types.");
        goto INVAILD;
    }

    buffer.width   = gbuffer->getWidth();
    buffer.height  = gbuffer->getHeight();
    buffer.wstride = gbuffer->getStride();
    buffer.hstride = gbuffer->getHeight();
    buffer.format  = gbuffer->getPixelFormat();

INVAILD:
    return buffer;
}
#endif
#endif

IM_API static void empty_structure(rga_buffer_t *src, rga_buffer_t *dst, rga_buffer_t *pat, im_rect *srect, im_rect *drect, im_rect *prect) {
    if (src != NULL)
        memset(src, 0, sizeof(*src));
    if (dst != NULL)
        memset(dst, 0, sizeof(*dst));
    if (pat != NULL)
        memset(pat, 0, sizeof(*pat));
    if (srect != NULL)
        memset(srect, 0, sizeof(*srect));
    if (drect != NULL)
        memset(drect, 0, sizeof(*drect));
    if (prect != NULL)
        memset(prect, 0, sizeof(*prect));
}

IM_API static bool rga_is_buffer_valid(rga_buffer_t buf) {
    return (buf.phy_addr != NULL || buf.fd > 0 || buf.vir_addr != NULL);
}

IM_API static bool rga_is_rect_valid(im_rect rect) {
    return (rect.x > 0 || rect.y > 0 || (rect.width > 0 && rect.height > 0));
}

IM_API IM_STATUS rga_set_buffer_info(rga_buffer_t dst, rga_info_t* dstinfo) {
    if(NULL == dstinfo) {
        ALOGE("rga_im2d: invaild dstinfo");
        imErrorMsg("Dst structure address is NULL.");
        return IM_STATUS_INVALID_PARAM;
    }

    if(dst.phy_addr != NULL)
        dstinfo->phyAddr= dst.phy_addr;
    else if(dst.fd > 0) {
        dstinfo->fd = dst.fd;
        dstinfo->mmuFlag = 1;
    } else if(dst.vir_addr != NULL) {
        dstinfo->virAddr = dst.vir_addr;
        dstinfo->mmuFlag = 1;
    } else {
        ALOGE("rga_im2d: invaild dst buffer");
        imErrorMsg("No address available in dst buffer.");
        return IM_STATUS_INVALID_PARAM;
    }

    return IM_STATUS_SUCCESS;
}

IM_API IM_STATUS rga_set_buffer_info(const rga_buffer_t src, rga_buffer_t dst, rga_info_t* srcinfo, rga_info_t* dstinfo) {
    if(NULL == srcinfo) {
        ALOGE("rga_im2d: invaild srcinfo");
        imErrorMsg("Src structure address is NULL.");
        return IM_STATUS_INVALID_PARAM;
    }
    if(NULL == dstinfo) {
        ALOGE("rga_im2d: invaild dstinfo");
        imErrorMsg("Dst structure address is NULL.");
        return IM_STATUS_INVALID_PARAM;
    }

    if(src.phy_addr != NULL)
        srcinfo->phyAddr = src.phy_addr;
    else if(src.fd > 0) {
        srcinfo->fd = src.fd;
        srcinfo->mmuFlag = 1;
    } else if(src.vir_addr != NULL) {
        srcinfo->virAddr = src.vir_addr;
        srcinfo->mmuFlag = 1;
    } else {
        ALOGE("rga_im2d: invaild src buffer");
        imErrorMsg("No address available in src buffer.");
        return IM_STATUS_INVALID_PARAM;
    }

    if(dst.phy_addr != NULL)
        dstinfo->phyAddr= dst.phy_addr;
    else if(dst.fd > 0) {
        dstinfo->fd = dst.fd;
        dstinfo->mmuFlag = 1;
    } else if(dst.vir_addr != NULL) {
        dstinfo->virAddr = dst.vir_addr;
        dstinfo->mmuFlag = 1;
    } else {
        ALOGE("rga_im2d: invaild dst buffer");
        imErrorMsg("No address available in dst buffer.");
        return IM_STATUS_INVALID_PARAM;
    }

    return IM_STATUS_SUCCESS;
}

IM_API long rga_get_info() {
    char buf[16];
    int  rgafd, rga_version = 0;
    long usage = 0;

    /*open /dev/rga node in order to get rga vesion*/
    rgafd = open("/dev/rga", O_RDWR, 0);
    if (rgafd < 0) {
        ALOGE("rga_im2d: failed to open /dev/rga: %s.",strerror(errno));
        return -1;
    }
    if (ioctl(rgafd, RGA_GET_VERSION, buf)) {
        ALOGE("rga_im2d: rga get version fail: %s",strerror(errno));
        return -1;
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

    switch(rga_version) {
        case RGA_1 :
            usage |= IM_RGA_INFO_VERSION_RGA_1;
            usage |= IM_RGA_INFO_RESOLUTION_INPUT_8192;
            usage |= IM_RGA_INFO_RESOLUTION_OUTPUT_2048;
            usage |= IM_RGA_INFO_SCALE_LIMIT_8;
            usage |= IM_RGA_INFO_SUPPORT_FORMAT_INPUT_RGB;
            usage |= IM_RGA_INFO_SUPPORT_FORMAT_INPUT_BP;
            usage |= IM_RGA_INFO_SUPPORT_FORMAT_INPUT_YUV_8;
            usage |= IM_RGA_INFO_SUPPORT_FORMAT_OUTPUT_RGB;
            usage |= IM_RGA_INFO_SUPPORT_FORMAT_OUTPUT_YUV_8;
            usage |= IM_RGA_INFO_PERFORMANCE_300;
            break;
        case RGA_1_PLUS :
            usage |= IM_RGA_INFO_VERSION_RGA_1_PLUS;
            usage |= IM_RGA_INFO_RESOLUTION_INPUT_8192;
            usage |= IM_RGA_INFO_RESOLUTION_OUTPUT_2048;
            usage |= IM_RGA_INFO_SCALE_LIMIT_8;
            usage |= IM_RGA_INFO_SUPPORT_FORMAT_INPUT_RGB;
            usage |= IM_RGA_INFO_SUPPORT_FORMAT_INPUT_BP;
            usage |= IM_RGA_INFO_SUPPORT_FORMAT_INPUT_YUV_8;
            usage |= IM_RGA_INFO_SUPPORT_FORMAT_OUTPUT_RGB;
            usage |= IM_RGA_INFO_SUPPORT_FORMAT_OUTPUT_YUV_8;
            usage |= IM_RGA_INFO_PERFORMANCE_300;
            break;
        case RGA_2 :
            usage |= IM_RGA_INFO_VERSION_RGA_2;
            usage |= IM_RGA_INFO_RESOLUTION_INPUT_8192;
            usage |= IM_RGA_INFO_RESOLUTION_OUTPUT_4096;
            usage |= IM_RGA_INFO_SCALE_LIMIT_16;
            usage |= IM_RGA_INFO_SUPPORT_FORMAT_INPUT_RGB;
            usage |= IM_RGA_INFO_SUPPORT_FORMAT_INPUT_YUV_8;
            usage |= IM_RGA_INFO_SUPPORT_FORMAT_OUTPUT_RGB;
            usage |= IM_RGA_INFO_SUPPORT_FORMAT_OUTPUT_YUV_8;
            usage |= IM_RGA_INFO_PERFORMANCE_600;
            break;
        case RGA_2_LITE0 :
            usage |= IM_RGA_INFO_VERSION_RGA_2_LITE0;
            usage |= IM_RGA_INFO_RESOLUTION_INPUT_8192;
            usage |= IM_RGA_INFO_RESOLUTION_OUTPUT_4096;
            usage |= IM_RGA_INFO_SCALE_LIMIT_8;
            usage |= IM_RGA_INFO_SUPPORT_FORMAT_INPUT_RGB;
            usage |= IM_RGA_INFO_SUPPORT_FORMAT_INPUT_YUV_8;
            usage |= IM_RGA_INFO_SUPPORT_FORMAT_OUTPUT_RGB;
            usage |= IM_RGA_INFO_SUPPORT_FORMAT_OUTPUT_YUV_8;
            usage |= IM_RGA_INFO_PERFORMANCE_520;
            break;
        case RGA_2_LITE1 :
            usage |= IM_RGA_INFO_VERSION_RGA_2_LITE1;
            usage |= IM_RGA_INFO_RESOLUTION_INPUT_8192;
            usage |= IM_RGA_INFO_RESOLUTION_OUTPUT_4096;
            usage |= IM_RGA_INFO_SCALE_LIMIT_8;
            usage |= IM_RGA_INFO_SUPPORT_FORMAT_INPUT_RGB;
            usage |= IM_RGA_INFO_SUPPORT_FORMAT_INPUT_YUV_8;
            usage |= IM_RGA_INFO_SUPPORT_FORMAT_INPUT_YUV_10;
            usage |= IM_RGA_INFO_SUPPORT_FORMAT_OUTPUT_RGB;
            usage |= IM_RGA_INFO_SUPPORT_FORMAT_OUTPUT_YUV_8;
            usage |= IM_RGA_INFO_PERFORMANCE_520;
            break;
        case RGA_2_ENHANCE :
            usage |= IM_RGA_INFO_VERSION_RGA_2_ENHANCE;
            usage |= IM_RGA_INFO_RESOLUTION_INPUT_8192;
            usage |= IM_RGA_INFO_RESOLUTION_OUTPUT_4096;
            usage |= IM_RGA_INFO_SCALE_LIMIT_16;
            usage |= IM_RGA_INFO_SUPPORT_FORMAT_INPUT_RGB;
            usage |= IM_RGA_INFO_SUPPORT_FORMAT_INPUT_YUV_8;
            usage |= IM_RGA_INFO_SUPPORT_FORMAT_INPUT_YUV_10;
            usage |= IM_RGA_INFO_SUPPORT_FORMAT_OUTPUT_RGB;
            usage |= IM_RGA_INFO_SUPPORT_FORMAT_OUTPUT_YUV_8;
            usage |= IM_RGA_INFO_SUPPORT_FORMAT_OUTPUT_YUYV;
            usage |= IM_RGA_INFO_SUPPORT_FORMAT_OUTPUT_YUV400;
            usage |= IM_RGA_INFO_PERFORMANCE_600;
            break;
        case RGA_V_ERR :
            usage = IM_STATUS_FAILED;
            break;
        default:
            return IM_STATUS_FAILED;
    }

    return usage;
}


IM_API const char* querystring(int name) {
    bool all_output = 0, all_output_prepared = 0;
    int rga_version = 0;
    long usage = 0;
    const char *temp;
    const char *output_vendor = "Rockchip Electronics Co.,Ltd.";
    const char *output_name[] = {
        "RGA vendor            : ",
        "RGA version           : ",
        "Max input             : ",
        "Max output            : ",
        "Scale limit           : ",
        "Input support format  : ",
        "output support format : ",
        "expected performance  : ",
    };
    const char *version_name[] = {
        "RGA_lib_version       : ",
        "RGA_im2d_version      : ",
        "RGA_built_version     : "
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
    const char *performance[] = {
        "unknown",
        "300M pix/s ",
        "520M pix/s ",
        "600M pix/s ",
    };
    ostringstream out;
    static string info;

    usage = rga_get_info();
    if (IM_STATUS_FAILED == usage) {
        ALOGE("rga im2d: rga2 get info failed!\n");
        return "get info failed";
    }

    switch(usage & IM_RGA_INFO_VERSION_MASK) {
        case IM_RGA_INFO_VERSION_RGA_1 :
            rga_version = RGA_1;
            break;
        case IM_RGA_INFO_VERSION_RGA_1_PLUS :
            rga_version = RGA_1_PLUS;
            break;
        case IM_RGA_INFO_VERSION_RGA_2 :
            rga_version = RGA_2;
            break;
        case IM_RGA_INFO_VERSION_RGA_2_LITE0 :
            rga_version = RGA_2_LITE0;
            break;
        case IM_RGA_INFO_VERSION_RGA_2_LITE1 :
            rga_version = RGA_2_LITE1;
            break;
        case IM_RGA_INFO_VERSION_RGA_2_ENHANCE :
            rga_version = RGA_2_ENHANCE;
            break;
        default :
            rga_version = RGA_V_ERR;
            break;
    }

    do {
        switch(name) {
            case RGA_VENDOR :
                out << output_name[name] << output_vendor << endl;
                break;

            case RGA_VERSION :
                out << version_name[RGA_LIB] << "v" << RGA_LIB_VERSION << endl;
                out << version_name[RGA_IM2D] << "v" << RGA_IM2D_VERSION << endl;
                out << version_name[RGA_BUILT] << RGA_BUILT_VERSION <<endl;
                out << output_name[name] << output_version[rga_version] << endl;
                break;

            case RGA_MAX_INPUT :
                switch(usage & IM_RGA_INFO_RESOLUTION_INPUT_MASK) {
                    case IM_RGA_INFO_RESOLUTION_INPUT_2048 :
                        out << output_name[name] << output_resolution[1] << endl;
                        break;
                    case IM_RGA_INFO_RESOLUTION_INPUT_4096 :
                        out << output_name[name] << output_resolution[2] << endl;
                        break;
                    case IM_RGA_INFO_RESOLUTION_INPUT_8192 :
                        out << output_name[name] << output_resolution[3] << endl;
                        break;
                    default :
                        out << output_name[name] << output_resolution[RGA_V_ERR] << endl;
                        break;
                }
                break;

            case RGA_MAX_OUTPUT :
                switch(usage & IM_RGA_INFO_RESOLUTION_OUTPUT_MASK) {
                    case IM_RGA_INFO_RESOLUTION_OUTPUT_2048 :
                        out << output_name[name] << output_resolution[1] << endl;
                        break;
                    case IM_RGA_INFO_RESOLUTION_OUTPUT_4096 :
                        out << output_name[name] << output_resolution[2] << endl;
                        break;
                    case IM_RGA_INFO_RESOLUTION_OUTPUT_8192 :
                        out << output_name[name] << output_resolution[3] << endl;
                        break;
                    default :
                        out << output_name[name] << output_resolution[RGA_V_ERR] << endl;
                        break;
                }
                break;

            case RGA_SCALE_LIMIT :
                switch(usage & IM_RGA_INFO_SCALE_LIMIT_MASK) {
                    case IM_RGA_INFO_SCALE_LIMIT_8 :
                        out << output_name[name] << output_scale_limit[1] << endl;
                        break;
                    case IM_RGA_INFO_SCALE_LIMIT_16 :
                        out << output_name[name] << output_scale_limit[2] << endl;
                        break;
                    default :
                        out << output_name[name] << output_scale_limit[RGA_V_ERR] << endl;
                        break;
                }
                break;

            case RGA_INPUT_FORMAT :
                out << output_name[name];
                if(usage & IM_RGA_INFO_SUPPORT_FORMAT_INPUT_RGB)
                    out << output_format[1];
                if(usage & IM_RGA_INFO_SUPPORT_FORMAT_INPUT_BP)
                    out << output_format[2];
                if(usage & IM_RGA_INFO_SUPPORT_FORMAT_INPUT_YUV_8)
                    out << output_format[3];
                if(usage & IM_RGA_INFO_SUPPORT_FORMAT_INPUT_YUV_10)
                    out << output_format[4];
                if(usage & IM_RGA_INFO_SUPPORT_FORMAT_INPUT_YUYV)
                    out << output_format[5];
                if(usage & IM_RGA_INFO_SUPPORT_FORMAT_INPUT_YUV400)
                    out << output_format[6];
                if(!(usage & IM_RGA_INFO_SUPPORT_FORMAT_INPUT_MASK))
                    out << output_format[RGA_V_ERR];
                out << endl;
                break;

            case RGA_OUTPUT_FORMAT :
                out << output_name[name];
                if(usage & IM_RGA_INFO_SUPPORT_FORMAT_OUTPUT_RGB)
                    out << output_format[1];
                if(usage & IM_RGA_INFO_SUPPORT_FORMAT_OUTPUT_BP)
                    out << output_format[2];
                if(usage & IM_RGA_INFO_SUPPORT_FORMAT_OUTPUT_YUV_8)
                    out << output_format[3];
                if(usage & IM_RGA_INFO_SUPPORT_FORMAT_OUTPUT_YUV_10)
                    out << output_format[4];
                if(usage & IM_RGA_INFO_SUPPORT_FORMAT_OUTPUT_YUYV)
                    out << output_format[5];
                if(usage & IM_RGA_INFO_SUPPORT_FORMAT_OUTPUT_YUV400)
                    out << output_format[6];
                if(!(usage & IM_RGA_INFO_SUPPORT_FORMAT_OUTPUT_MASK))
                    out << output_format[RGA_V_ERR];
                out << endl;
                break;

            case RGA_EXPECTED :
				switch(usage & IM_RGA_INFO_PERFORMANCE_MASK) {
                    case IM_RGA_INFO_PERFORMANCE_300 :
                        out << output_name[name] << performance[1] << endl;
                        break;
                    case IM_RGA_INFO_PERFORMANCE_520 :
                        out << output_name[name] << performance[2] << endl;
                        break;
                    case IM_RGA_INFO_PERFORMANCE_600 :
                        out << output_name[name] << performance[3] << endl;
                        break;
                    default :
                        out << output_name[name] << performance[RGA_V_ERR] << endl;
                        break;
                }
                break;

            case RGA_ALL :
                if (!all_output) {
                    all_output = 1;
                    name = 0;
                } else
                    all_output_prepared = 1;
                break;

            default:
                return "Invalid instruction";
        }

        info = out.str();

        if (all_output_prepared)
            break;
        else if (all_output && strcmp(info.c_str(),"0")>0)
            name++;

    } while(all_output);

    temp = info.c_str();

    return temp;
}

IM_API IM_STATUS imcheck_t(const rga_buffer_t src, const rga_buffer_t dst, const rga_buffer_t pat,
                           im_rect src_rect, const im_rect dst_rect, const im_rect pat_rect, int mode_usage) {
    bool src_isRGB = 0, src_isBP = 0, src_isYUV_8 = 0, src_isYUV_10 = 0, src_isYUYV = 0, src_isYUV400 = 0;
    bool dst_isRGB = 0, dst_isBP = 0, dst_isYUV_8 = 0, dst_isYUV_10 = 0, dst_isYUYV = 0, dst_isYUV400 = 0;
    bool pat_isRGB = 0, pat_isBP = 0, pat_isYUV_8 = 0, pat_isYUV_10 = 0, pat_isYUYV = 0, pat_isYUV400 = 0;
    bool pat_buffer_isValid = 0, pat_rect_isValid = 0;
    int src_fmt, dst_fmt, pat_fmt;
    long usage = 0;

    usage = rga_get_info();
    if (IM_STATUS_FAILED == usage) {
        imErrorMsg("Get rga info failed, can not continue check.");
        return IM_STATUS_FAILED;
    }

    if (mode_usage & IM_ALPHA_BLEND_MASK) {
        if (rga_is_buffer_valid(pat))
            pat_buffer_isValid = 1;
        if (rga_is_rect_valid(pat_rect))
            pat_rect_isValid = 1;
    }

    /**************** src/dst judgment ****************/
    if (~mode_usage & IM_COLOR_FILL) {
        if (src.width <= 0 || src.height <= 0 || src.format < 0) {
            imErrorMsg("Illegal src, the parameter cannot be negative or 0.");
            return IM_STATUS_ILLEGAL_PARAM;
        }

        if (src.wstride < src.width || src.hstride < src.height) {
            imErrorMsg("Invaild src, Virtual width or height is less than actual width and height.");
            return IM_STATUS_INVALID_PARAM;
        }
    }

    if (dst.width <= 0 || dst.height <= 0 || dst.format < 0) {
        imErrorMsg("Illegal dst, the parameter cannot be negative or 0.");
        return IM_STATUS_ILLEGAL_PARAM;
    }

    if (dst.wstride < dst.width || dst.hstride < dst.height) {
        imErrorMsg("Invaild dst, virtual width or height is less than actual width or height.");
        return IM_STATUS_INVALID_PARAM;
    }

    if (pat_buffer_isValid) {
        if (pat.width <= 0 || pat.height <= 0 || pat.format < 0) {
            imErrorMsg("Illegal pat, the parameter cannot be negative or 0.");
            return IM_STATUS_ILLEGAL_PARAM;
        }

        if (pat.wstride < pat.width || pat.hstride < pat.height) {
            imErrorMsg("Invaild pat, virtual width or height is less than actual width or height.");
            return IM_STATUS_INVALID_PARAM;
        }
    }

    /**************** rect judgment ****************/
    if ((src_rect.width > 0  && src_rect.width < 2) ||
        (src_rect.height > 0 && src_rect.height < 2)) {
        imErrorMsg("Invaild src rect, unsupported width and height less than 2.");
        return IM_STATUS_INVALID_PARAM;
    }

    if (src_rect.width < 0 || src_rect.height < 0 || src_rect.x < 0 || src_rect.y < 0) {
        imErrorMsg("Illegal src rect, the parameter cannot be negative.");
        return IM_STATUS_ILLEGAL_PARAM;
    }

    if ((src_rect.width + src_rect.x > src.wstride) || (src_rect.height + src_rect.y > src.hstride)) {
        imErrorMsg("Invaild src rect, the sum of widtn and height of rect needs to be less than src wstride or hstride.");
        return IM_STATUS_INVALID_PARAM;
    }

    if ((dst_rect.width > 0  && dst_rect.width < 2) ||
        (dst_rect.height > 0 && dst_rect.height < 2)) {
        imErrorMsg("Invaild dst rect, unsupported width and height less than 2.");
        return IM_STATUS_INVALID_PARAM;
    }

    if (dst_rect.width < 0 || dst_rect.height < 0 || dst_rect.x < 0 || dst_rect.y < 0) {
        imErrorMsg("Illegal dst rect, the parameter cannot be negative.");
        return IM_STATUS_ILLEGAL_PARAM;
    }

    if ((dst_rect.width + dst_rect.x > dst.wstride) || (dst_rect.height + dst_rect.y > dst.hstride)) {
        imErrorMsg("Invaild dst rect, the sum of widtn and height of rect needs to be less than dst wstride or hstride.");
        return IM_STATUS_INVALID_PARAM;
    }

    if (pat_rect_isValid) {
        if ((pat_rect.width > 0  && pat_rect.width < 2) ||
            (pat_rect.height > 0 && pat_rect.height < 2)) {
            imErrorMsg("Invaild pat rect, unsupported width and height less than 2.");
            return IM_STATUS_INVALID_PARAM;
        }

        if (pat_rect.width < 0 || pat_rect.height < 0 || pat_rect.x < 0 || pat_rect.y < 0) {
            imErrorMsg("Illegal pat rect, the parameter cannot be negative.");
            return IM_STATUS_ILLEGAL_PARAM;
        }

        if ((pat_rect.width + pat_rect.x > pat.wstride) || (pat_rect.height + pat_rect.y > pat.hstride)) {
            imErrorMsg("Invaild pat rect, the sum of widtn and height of rect needs to be less than dst wstride or hstride.");
            return IM_STATUS_INVALID_PARAM;
        }
    }

    /**************** resolution check ****************/
    switch (usage & IM_RGA_INFO_RESOLUTION_INPUT_MASK) {
        case IM_RGA_INFO_RESOLUTION_INPUT_2048 :
            if (src.width > 2048 || src.height > 2048) {
                imErrorMsg("Unsupported src to input resolution more than 2048.");
                return IM_STATUS_NOT_SUPPORTED;
            } else if ((src_rect.width > 0 && src_rect.width > 2048) ||
                    (src_rect.height > 0 && src_rect.height > 2048)) {
                imErrorMsg("Unsupported src rect to output resolution more than 2048.");
                return IM_STATUS_NOT_SUPPORTED;
            }
            break;

        case IM_RGA_INFO_RESOLUTION_INPUT_4096 :
            if (src.width > 4096 || src.height > 4096) {
                imErrorMsg("Unsupported src to input resolution more than 4096.");
                return IM_STATUS_NOT_SUPPORTED;
            } else if ((src_rect.width > 0 && src_rect.width > 4096) ||
                       (src_rect.height > 0 && src_rect.height > 4096)) {
                imErrorMsg("Unsupported src rect to output resolution more than 4096.");
                return IM_STATUS_NOT_SUPPORTED;
            }
            break;

        case IM_RGA_INFO_RESOLUTION_INPUT_8192 :
            if (src.width > 8192 || src.height > 8192) {
                imErrorMsg("Unsupported src to input resolution more than 8192.");
                return IM_STATUS_NOT_SUPPORTED;
            } else if ((src_rect.width > 0 && src_rect.width > 8192) ||
                       (src_rect.height > 0 && src_rect.height > 8192)) {
                imErrorMsg("Unsupported src rect to output resolution more than 8192.");
                return IM_STATUS_NOT_SUPPORTED;
            }
            break;
    }

    switch (usage & IM_RGA_INFO_RESOLUTION_OUTPUT_MASK) {
        case IM_RGA_INFO_RESOLUTION_OUTPUT_2048 :
            if (dst.width > 2048 || dst.height > 2048) {
                imErrorMsg("Unsupported dst to output resolution more than 2048.");
                return IM_STATUS_NOT_SUPPORTED;
            } else if ((dst_rect.width > 0 && dst_rect.width > 2048) ||
                       (dst_rect.height > 0 && dst_rect.height > 2048)) {
                imErrorMsg("Unsupported dst rect to output resolution more than 2048.");
                return IM_STATUS_NOT_SUPPORTED;
            }
            break;

        case IM_RGA_INFO_RESOLUTION_OUTPUT_4096 :
            if (dst.width > 4096 || dst.height > 4096) {
                imErrorMsg("Unsupported dst to output resolution more than 4096.");
                return IM_STATUS_NOT_SUPPORTED;
            } else if ((dst_rect.width > 0 && dst_rect.width > 4096) ||
                       (dst_rect.height > 0 && dst_rect.height > 4096)) {
                imErrorMsg("Unsupported dst rect to output resolution more than 4096.");
                return IM_STATUS_NOT_SUPPORTED;
            }
            break;

        case IM_RGA_INFO_RESOLUTION_OUTPUT_8192 :
            if (dst.width > 8192 || dst.height > 8192) {
                imErrorMsg("Unsupported dst to output resolution more than 8192.");
                return IM_STATUS_NOT_SUPPORTED;
            } else if ((dst_rect.width > 0 && dst_rect.width > 8192) ||
                       (dst_rect.height > 0 && dst_rect.height > 8192)) {
                imErrorMsg("Unsupported dst rect to output resolution more than 4096.");
                return IM_STATUS_NOT_SUPPORTED;
            }
            break;
    }

    if (pat_buffer_isValid) {
        switch (usage & IM_RGA_INFO_RESOLUTION_OUTPUT_MASK) {
            case IM_RGA_INFO_RESOLUTION_INPUT_2048 :
                if (pat.width > 2048 || pat.height > 2048) {
                    imErrorMsg("Unsupported pat to input resolution more than 2048.");
                    return IM_STATUS_NOT_SUPPORTED;
                } else if ((pat_rect.width > 0 && pat_rect.width > 2048) ||
                           (pat_rect.height > 0 && pat_rect.height > 2048)) {
                    imErrorMsg("Unsupported pat rect to input resolution more than 2048.");
                    return IM_STATUS_NOT_SUPPORTED;
                }
                break;

            case IM_RGA_INFO_RESOLUTION_INPUT_4096 :
                if (pat.width > 4096 || pat.height > 4096) {
                    imErrorMsg("Unsupported pat to input resolution more than 4096.");
                    return IM_STATUS_NOT_SUPPORTED;
                } else if ((pat_rect.width > 0 && pat_rect.width > 4096) ||
                           (pat_rect.height > 0 && pat_rect.height > 4096)) {
                    imErrorMsg("Unsupported pat rect to input resolution more than 4096.");
                    return IM_STATUS_NOT_SUPPORTED;
                }
                break;

            case IM_RGA_INFO_RESOLUTION_INPUT_8192 :
                if (pat.width > 8192 || pat.height > 8192) {
                    imErrorMsg("Unsupported pat to input resolution more than 8192.");
                    return IM_STATUS_NOT_SUPPORTED;
                } else if ((pat_rect.width > 0 && pat_rect.width > 8192) ||
                           (pat_rect.height > 0 && pat_rect.height > 8192)) {
                    imErrorMsg("Unsupported pat rect to input resolution more than 8192.");
                    return IM_STATUS_NOT_SUPPORTED;
                }
                break;
        }
    }

    /**************** scale check ****************/
    if ((~mode_usage & IM_COLOR_FILL) && (~mode_usage & IM_CROP)) {
        switch (usage & IM_RGA_INFO_SCALE_LIMIT_MASK) {
            case IM_RGA_INFO_SCALE_LIMIT_8 :
                if (((src.width >> 3) > dst.width) || ((src.height >> 3) > dst.height)) {
                    imErrorMsg("Unsupported to scaling less than 1/8 times.");
                    return IM_STATUS_NOT_SUPPORTED;
                }
                if (((dst.width >> 3) > src.width) || ((dst.height >> 3) > src.height)) {
                    imErrorMsg("Unsupported to scaling more than 8 times.");
                    return IM_STATUS_NOT_SUPPORTED;
                }
                break;

            case IM_RGA_INFO_SCALE_LIMIT_16 :
                if (((src.width >> 4) > dst.width) || ((src.height >> 4) > dst.height)) {
                    imErrorMsg("Unsupported to scaling less than 1/16 times.");
                    return IM_STATUS_NOT_SUPPORTED;
                }
                if (((dst.width >> 4) > src.width) || ((dst.height >> 4) > src.height)) {
                    imErrorMsg("Unsupported to scaling more than 16 times.");
                    return IM_STATUS_NOT_SUPPORTED;
                }
                break;
        }
    }

    /**************** format check ****************/
    if (~mode_usage & IM_COLOR_FILL) {
        src_fmt = RkRgaGetRgaFormat(RkRgaCompatibleFormat(src.format));
        if (-1 == src_fmt) {
            imErrorMsg("Is src unsupport format ,please query and fix.");
            return IM_STATUS_NOT_SUPPORTED;
        }

        if (src_fmt == RK_FORMAT_RGB_565   || src_fmt == RK_FORMAT_RGB_888   ||
            src_fmt == RK_FORMAT_BGR_888   || src_fmt == RK_FORMAT_RGBX_8888 ||
            src_fmt == RK_FORMAT_BGRA_8888 || src_fmt == RK_FORMAT_RGBA_8888 ||
            src_fmt == RK_FORMAT_RGBA_4444 || src_fmt == RK_FORMAT_RGBA_5551) {
            if (~usage & IM_RGA_INFO_SUPPORT_FORMAT_INPUT_RGB) {
                imErrorMsg("Src unsupported input RGB format.");
                return IM_STATUS_NOT_SUPPORTED;
            }
            src_isRGB = 1;
        } else if (src_fmt == RK_FORMAT_BPP1 || src_fmt == RK_FORMAT_BPP2 ||
                   src_fmt == RK_FORMAT_BPP4 || src_fmt == RK_FORMAT_BPP8) {
            if ((~usage & IM_RGA_INFO_SUPPORT_FORMAT_INPUT_BP) && !(mode_usage & IM_COLOR_PALETTE)) {
                imErrorMsg("Src unsupported input BP format.");
                return IM_STATUS_NOT_SUPPORTED;
            }
            src_isBP = 1;
        } else if (src_fmt == RK_FORMAT_YCrCb_420_SP || src_fmt == RK_FORMAT_YCbCr_420_SP ||
                   src_fmt == RK_FORMAT_YCrCb_420_P  || src_fmt == RK_FORMAT_YCbCr_420_P  ||
                   src_fmt == RK_FORMAT_YCrCb_422_SP || src_fmt == RK_FORMAT_YCbCr_422_SP ||
                   src_fmt == RK_FORMAT_YCrCb_422_P  || src_fmt == RK_FORMAT_YCbCr_422_P) {
            if (~usage & IM_RGA_INFO_SUPPORT_FORMAT_INPUT_YUV_8) {
                imErrorMsg("Src unsupported input YUV 8bit format.");
                return IM_STATUS_NOT_SUPPORTED;
            }
            /*Align check*/
            if ((src.wstride % 2) || (src.hstride % 2) ||
                (src.width % 2)  || (src.height % 2) ||
                (src_rect.x % 2) || (src_rect.y % 2) ||
                (src_rect.width % 2) || (src_rect.height % 2)) {
                imErrorMsg("Err yuv not align to 2.");
                return IM_STATUS_INVALID_PARAM;
            }
            src_isYUV_8 = 1;
        } else if (src_fmt == RK_FORMAT_YCbCr_420_SP_10B || src_fmt == RK_FORMAT_YCrCb_420_SP_10B) {
            if (~usage & IM_RGA_INFO_SUPPORT_FORMAT_INPUT_YUV_10) {
                imErrorMsg("Src unsupported input YUV 10bit format.");
                return IM_STATUS_NOT_SUPPORTED;
            }
            /*Align check*/
            if ((src.wstride % 16) || (src.hstride % 2) ||
                (src.width % 2)  || (src.height % 2) ||
                (src_rect.x % 2) || (src_rect.y % 2) ||
                (src_rect.width % 2) || (src_rect.height % 2)) {
                imErrorMsg("Err src wstride is not align to 16 or yuv not align to 2.");
                return IM_STATUS_INVALID_PARAM;
            }
            ALOGE("If it is an RK encoder output, it needs to be aligned with an odd multiple of 256.\n");
            src_isYUV_10 = 1;
        } else if (src_fmt == RK_FORMAT_YUYV_420 || src_fmt == RK_FORMAT_YUYV_422 ||
                   src_fmt == RK_FORMAT_YVYU_420 || src_fmt == RK_FORMAT_YVYU_422) {
            if (~usage & IM_RGA_INFO_SUPPORT_FORMAT_INPUT_YUYV) {
                imErrorMsg("Src unsupported input YUYV format.");
                return IM_STATUS_NOT_SUPPORTED;
            }
            src_isYUYV = 1;
        } else if (src_fmt == RK_FORMAT_YCbCr_400 || src_fmt == RK_FORMAT_Y4) {
            if (~usage & IM_RGA_INFO_SUPPORT_FORMAT_INPUT_YUV400) {
                imErrorMsg("Src unsupported input YUV400 format.");
                return IM_STATUS_NOT_SUPPORTED;
            }
            src_isYUV400 = 1;
        } else {
            imErrorMsg("Src unsupported input this format.");
            return IM_STATUS_NOT_SUPPORTED;
        }
    }

    if (pat_buffer_isValid) {
        pat_fmt = RkRgaGetRgaFormat(RkRgaCompatibleFormat(src.format));
        if (-1 == pat_fmt) {
            imErrorMsg("Is pat unsupport format ,please query and fix.");
            return IM_STATUS_NOT_SUPPORTED;
        }

        if (pat_fmt == RK_FORMAT_RGB_565   || pat_fmt == RK_FORMAT_RGB_888	 ||
            pat_fmt == RK_FORMAT_BGR_888   || pat_fmt == RK_FORMAT_RGBX_8888 ||
            pat_fmt == RK_FORMAT_BGRA_8888 || pat_fmt == RK_FORMAT_RGBA_8888 ||
            pat_fmt == RK_FORMAT_RGBA_4444 || pat_fmt == RK_FORMAT_RGBA_5551) {
            if (~usage & IM_RGA_INFO_SUPPORT_FORMAT_INPUT_RGB) {
                imErrorMsg("Pat unsupported input RGB format.");
                return IM_STATUS_NOT_SUPPORTED;
            }
            pat_isRGB = 1;
        } else if (pat_fmt == RK_FORMAT_BPP1 || pat_fmt == RK_FORMAT_BPP2 ||
                   pat_fmt == RK_FORMAT_BPP4 || pat_fmt == RK_FORMAT_BPP8) {
            if ((~usage & IM_RGA_INFO_SUPPORT_FORMAT_INPUT_BP) && !(mode_usage & IM_COLOR_PALETTE)) {
                imErrorMsg("Pat unsupported input BP format.");
                return IM_STATUS_NOT_SUPPORTED;
            }
            pat_isBP = 1;
        } else if (pat_fmt == RK_FORMAT_YCrCb_420_SP || pat_fmt == RK_FORMAT_YCbCr_420_SP ||
                   pat_fmt == RK_FORMAT_YCrCb_420_P  || pat_fmt == RK_FORMAT_YCbCr_420_P  ||
                   pat_fmt == RK_FORMAT_YCrCb_422_SP || pat_fmt == RK_FORMAT_YCbCr_422_SP ||
                   pat_fmt == RK_FORMAT_YCrCb_422_P  || pat_fmt == RK_FORMAT_YCbCr_422_P) {
            if (~usage & IM_RGA_INFO_SUPPORT_FORMAT_INPUT_YUV_8) {
                imErrorMsg("Pat unsupported input YUV 8bit format.");
                return IM_STATUS_NOT_SUPPORTED;
            }
            /*Align check*/
            if ((pat.wstride % 2) || (pat.hstride % 2) ||
                (pat.width % 2)  || (pat.height % 2) ||
                (pat_rect.x % 2) || (pat_rect.y % 2) ||
                (pat_rect.width % 2) || (pat_rect.height % 2)) {
                imErrorMsg("Err pat yuv not align to 2.");
                return IM_STATUS_INVALID_PARAM;
            }
            pat_isYUV_8 = 1;
        } else if (pat_fmt == RK_FORMAT_YCbCr_420_SP_10B || pat_fmt == RK_FORMAT_YCrCb_420_SP_10B) {
            if (~usage & IM_RGA_INFO_SUPPORT_FORMAT_INPUT_YUV_10) {
                imErrorMsg("Pat unsupported input YUV 10bit format.");
                return IM_STATUS_NOT_SUPPORTED;
            }
            /*Align check*/
            if ((pat.wstride % 16) || (pat.hstride % 2) ||
                (pat.width % 2)  || (pat.height % 2) ||
                (pat_rect.x % 2) || (pat_rect.y % 2) ||
                (pat_rect.width % 2) || (pat_rect.height % 2)) {
                imErrorMsg("Err pat wstride is not align to 16 or yuv not align to 2.");
                return IM_STATUS_INVALID_PARAM;
            }
            ALOGE("If it is an RK encoder output, it needs to be aligned with an odd multiple of 256.\n");
            pat_isYUV_10 = 1;
        } else if (pat_fmt == RK_FORMAT_YUYV_420 || pat_fmt == RK_FORMAT_YUYV_422 ||
                   pat_fmt == RK_FORMAT_YVYU_420 || pat_fmt == RK_FORMAT_YVYU_422) {
            if (~usage & IM_RGA_INFO_SUPPORT_FORMAT_INPUT_YUYV) {
                imErrorMsg("Pat unsupported input YUYV format.");
                return IM_STATUS_NOT_SUPPORTED;
            }
            src_isYUYV = 1;
        } else if (pat_fmt == RK_FORMAT_YCbCr_400 || pat_fmt == RK_FORMAT_Y4) {
            if (~usage & IM_RGA_INFO_SUPPORT_FORMAT_INPUT_YUV400) {
                imErrorMsg("Pat unsupported input YUV400 format.");
                return IM_STATUS_NOT_SUPPORTED;
            }
            pat_isYUV400 = 1;
        } else {
            imErrorMsg("Pat unsupported input this format.");
            return IM_STATUS_NOT_SUPPORTED;
        }
    }

    dst_fmt = RkRgaGetRgaFormat(RkRgaCompatibleFormat(dst.format));
    if (-1 == dst_fmt) {
        imErrorMsg("Is dst unsupport format ,please query and fix.");
        return IM_STATUS_NOT_SUPPORTED;
    }

    if (dst_fmt == RK_FORMAT_RGB_565   || dst_fmt == RK_FORMAT_RGB_888   ||
        dst_fmt == RK_FORMAT_BGR_888   || dst_fmt == RK_FORMAT_RGBX_8888 ||
        dst_fmt == RK_FORMAT_BGRA_8888 || dst_fmt == RK_FORMAT_RGBA_8888 ||
        dst_fmt == RK_FORMAT_RGBA_4444 || dst_fmt == RK_FORMAT_RGBA_5551) {
        if (~usage & IM_RGA_INFO_SUPPORT_FORMAT_OUTPUT_RGB) {
            imErrorMsg("Dst unsupported output RGB format.");
            return IM_STATUS_NOT_SUPPORTED;
        }
        dst_isRGB = 1;
    } else if (dst_fmt == RK_FORMAT_BPP1 || dst_fmt == RK_FORMAT_BPP2 ||
               dst_fmt == RK_FORMAT_BPP4 || dst_fmt == RK_FORMAT_BPP8) {
        if (~usage & IM_RGA_INFO_SUPPORT_FORMAT_OUTPUT_BP) {
            imErrorMsg("Dst unsupported output BP format.");
            return IM_STATUS_NOT_SUPPORTED;
        }
        dst_isBP = 1;
    } else if (dst_fmt == RK_FORMAT_YCrCb_420_SP || dst_fmt == RK_FORMAT_YCbCr_420_SP ||
               dst_fmt == RK_FORMAT_YCrCb_420_P  || dst_fmt == RK_FORMAT_YCbCr_420_P  ||
               dst_fmt == RK_FORMAT_YCrCb_422_SP || dst_fmt == RK_FORMAT_YCbCr_422_SP ||
               dst_fmt == RK_FORMAT_YCrCb_422_P  || dst_fmt == RK_FORMAT_YCbCr_422_P) {
        if (~usage & IM_RGA_INFO_SUPPORT_FORMAT_OUTPUT_YUV_8) {
            imErrorMsg("Dst unsupported output YUV 8bit format.");
            return IM_STATUS_NOT_SUPPORTED;
        }

        /*Align check*/
        if ((dst.wstride % 2) || (dst.hstride % 2) ||
            (dst.width % 2)  || (dst.height % 2) ||
            (dst_rect.x % 2) || (dst_rect.y % 2) ||
            (dst_rect.width % 2) || (dst_rect.height % 2)) {
            imErrorMsg("Err dst yuv not align to 2.");
            return IM_STATUS_INVALID_PARAM;
        }
        dst_isYUV_8 = 1;
    } else if (dst_fmt == RK_FORMAT_YCrCb_420_SP_10B || dst_fmt == RK_FORMAT_YCbCr_420_SP_10B) {
        if (~usage & IM_RGA_INFO_SUPPORT_FORMAT_OUTPUT_YUV_10) {
            imErrorMsg("Dst unsupported output YUV 10bit format.");
            return IM_STATUS_NOT_SUPPORTED;
        }

        /*Align check*/
        if ((dst.wstride % 16) || (dst.hstride % 2) ||
            (dst.width % 2)  || (dst.height % 2) ||
            (dst_rect.x % 2) || (dst_rect.y % 2) ||
            (dst_rect.width % 2) || (dst_rect.height % 2)) {
            imErrorMsg("Err dst wstride is not align to 16 or yuv not align to 2.");
            return IM_STATUS_INVALID_PARAM;
        }
        ALOGE("If you want to input to the RK encoder, you need to align with an odd multiple of 256.\n");
        dst_isYUV_10 = 1;
    } else if (dst_fmt == RK_FORMAT_YUYV_420 || dst_fmt == RK_FORMAT_YUYV_422 ||
               dst_fmt == RK_FORMAT_YVYU_420 || dst_fmt == RK_FORMAT_YVYU_422) {
        if (~usage & IM_RGA_INFO_SUPPORT_FORMAT_OUTPUT_YUYV) {
            imErrorMsg("Dst unsupported output YUYV format.");
            return IM_STATUS_NOT_SUPPORTED;
        }
        dst_isYUYV = 1;
    } else if (dst_fmt == RK_FORMAT_YCbCr_400 || dst_fmt == RK_FORMAT_Y4) {
        if (~usage & IM_RGA_INFO_SUPPORT_FORMAT_OUTPUT_YUV400) {
            imErrorMsg("Dst unsupported output YUV400 format.");
            return IM_STATUS_NOT_SUPPORTED;
        }
        dst_isYUV400 = 1;
    } else {
        imErrorMsg("Dst unsupported output this format.");
        return IM_STATUS_NOT_SUPPORTED;
    }

    /**************** blend mode check ****************/
    if (mode_usage & IM_ALPHA_BLEND_MASK) {
        if (!dst_isRGB || (pat_buffer_isValid && !pat_isRGB)) {
            imErrorMsg("dst/pat channel unsupported formats other than RGB.");
            return IM_STATUS_NOT_SUPPORTED;
        }
        switch (mode_usage & IM_ALPHA_BLEND_MASK) {
            case IM_ALPHA_BLEND_SRC :
            case IM_ALPHA_BLEND_DST :
                break;
            case IM_ALPHA_BLEND_SRC_OVER :
                if (!src_isRGB || (src_fmt == RK_FORMAT_RGB_565 ||
                    src_fmt == RK_FORMAT_RGB_888 || src_fmt == RK_FORMAT_BGR_888)) {
                    imErrorMsg("Blend mode 'src_over' unsupported src format without alpha.");
                    return IM_STATUS_NOT_SUPPORTED;
                }
                break;
            case IM_ALPHA_BLEND_DST_OVER :
                if (pat_buffer_isValid) {
                    if (!pat_isRGB || (pat_fmt == RK_FORMAT_RGB_565 ||
                        pat_fmt == RK_FORMAT_RGB_888 || pat_fmt == RK_FORMAT_BGR_888)) {
                        imErrorMsg("Blend mode 'dst_over' unsupported pat format without alpha.");
                        return IM_STATUS_NOT_SUPPORTED;
                    }
                } else {
                    if (!dst_isRGB || (dst_fmt == RK_FORMAT_RGB_565 ||
                        dst_fmt == RK_FORMAT_RGB_888 || dst_fmt == RK_FORMAT_BGR_888)) {
                        imErrorMsg("Blend mode 'dst_over' unsupported dst format without alpha.");
                        return IM_STATUS_NOT_SUPPORTED;
                    }
                }

                break;
            default :
                if (!(src_isRGB && dst_isRGB) ||
                   (src_fmt == RK_FORMAT_RGB_565 || src_fmt == RK_FORMAT_RGB_888 ||
                    src_fmt == RK_FORMAT_BGR_888 || dst_fmt == RK_FORMAT_RGB_565 ||
                    dst_fmt == RK_FORMAT_RGB_888 || dst_fmt == RK_FORMAT_BGR_888)) {
                    imErrorMsg("Blend mode unsupported format without alpha.");
                    return IM_STATUS_NOT_SUPPORTED;
                }
                break;
        }
    }
    /**************** rotate mode check ****************/
    if ((mode_usage & IM_HAL_TRANSFORM_ROT_90) || (mode_usage & IM_HAL_TRANSFORM_ROT_270)) {
        if ((src.width != dst.height) || (src.height != dst.width) ||
            (src_rect.width != dst_rect.height) || (src_rect.height != dst_rect.width)) {
            imErrorMsg("Rotate 90 or 270 need to exchange width and height.");
            return IM_STATUS_INVALID_PARAM;
        }
    }

    return IM_STATUS_NOERROR;
}

IM_API IM_STATUS imresize_t(const rga_buffer_t src, rga_buffer_t dst, double fx, double fy, int interpolation, int sync) {
    int usage = 0;
    int width = 0, height = 0;
    IM_STATUS ret = IM_STATUS_NOERROR;

    rga_buffer_t pat;

    im_rect srect;
    im_rect drect;
    im_rect prect;

    empty_structure(NULL, NULL, &pat, &srect, &drect, &prect);

    if (fx > 0 || fy > 0) {
        if (fx == 0) fx = 1;
        if (fy == 0) fy = 1;

        dst.width = (int)(src.width * fx);
        dst.height = (int)(src.height * fy);

        if(NormalRgaIsYuvFormat(RkRgaGetRgaFormat(src.format))) {
            width = dst.width;
            height = dst.height;
            dst.width = DOWN_ALIGN(dst.width, 2);
            dst.height = DOWN_ALIGN(dst.height, 2);

            ret = imcheck(src, dst, srect, drect, usage);
            if (ret != IM_STATUS_NOERROR) {
                ALOGE("imresize error, factor[fx,fy]=[%lf,%lf], ALIGN[dw,dh]=[%d,%d][%d,%d]", fx, fy, width, height, dst.width, dst.height);
                return ret;
            }
        }
    }
    UNUSED(interpolation);

    if (sync == 0)
        usage |= IM_SYNC;

    ret = improcess(src, dst, pat, srect, drect, prect, usage);

    return ret;
}

IM_API IM_STATUS imcrop_t(const rga_buffer_t src, rga_buffer_t dst, im_rect rect, int sync) {
    int usage = 0;
    IM_STATUS ret = IM_STATUS_NOERROR;

    rga_buffer_t pat;

    im_rect drect;
    im_rect prect;

    empty_structure(NULL, NULL, &pat, NULL, &drect, &prect);

    usage |= IM_CROP;

    if (sync == 0)
        usage |= IM_SYNC;

    ret = improcess(src, dst, pat, rect, drect, prect, usage);

    return ret;
}

IM_API IM_STATUS imrotate_t(const rga_buffer_t src, rga_buffer_t dst, int rotation, int sync) {
    int usage = 0;
    IM_STATUS ret = IM_STATUS_NOERROR;

    rga_buffer_t pat;

    im_rect srect;
    im_rect drect;
    im_rect prect;

    empty_structure(NULL, NULL, &pat, &srect, &drect, &prect);

    usage |= rotation;

    if (sync == 0)
        usage |= IM_SYNC;

    ret = improcess(src, dst, pat, srect, drect, prect, usage);

    return ret;
}

IM_API IM_STATUS imflip_t (const rga_buffer_t src, rga_buffer_t dst, int mode, int sync) {
    int usage = 0;
    IM_STATUS ret = IM_STATUS_NOERROR;

    rga_buffer_t pat;

    im_rect srect;
    im_rect drect;
    im_rect prect;

    empty_structure(NULL, NULL, &pat, &srect, &drect, &prect);

    usage |= mode;

    if (sync == 0)
        usage |= IM_SYNC;

    ret = improcess(src, dst, pat, srect, drect, prect, usage);

    return ret;
}

IM_API IM_STATUS imfill_t(rga_buffer_t dst, im_rect rect, int color, int sync) {
    int usage = 0;
    IM_STATUS ret = IM_STATUS_NOERROR;

    rga_buffer_t pat;
    rga_buffer_t src;

    im_rect srect;
    im_rect prect;

    empty_structure(&src, NULL, &pat, &srect, NULL, &prect);

    memset(&src, 0, sizeof(rga_buffer_t));

    usage |= IM_COLOR_FILL;

    dst.color = color;

    if (sync == 0)
        usage |= IM_SYNC;

    ret = improcess(src, dst, pat, srect, rect, prect, usage);

    return ret;
}

IM_API IM_STATUS impalette_t(rga_buffer_t src, rga_buffer_t dst, rga_buffer_t lut, int sync) {
    int usage = 0;
    IM_STATUS ret = IM_STATUS_NOERROR;
    im_rect srect;
    im_rect drect;
    im_rect prect;

    empty_structure(NULL, NULL, NULL, &srect, &drect, &prect);

    /*Don't know if it supports zooming.*/
    if ((src.width != dst.width) || (src.height != dst.height))
        return IM_STATUS_INVALID_PARAM;

    usage |= IM_COLOR_PALETTE;

    if (sync == 0)
        usage |= IM_SYNC;

    ret = improcess(src, dst, lut, srect, drect, prect, usage);

    return ret;
}

IM_API IM_STATUS imtranslate_t(const rga_buffer_t src, rga_buffer_t dst, int x, int y, int sync) {
    int usage = 0;
    IM_STATUS ret = IM_STATUS_NOERROR;

    rga_buffer_t pat;

    im_rect srect;
    im_rect drect;
    im_rect prect;

    empty_structure(NULL, NULL, &pat, &srect, &drect, &prect);

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

    ret = improcess(src, dst, pat, srect, drect, prect, usage);

    return ret;
}

IM_API IM_STATUS imcopy_t(const rga_buffer_t src, rga_buffer_t dst, int sync) {
    int usage = 0;
    IM_STATUS ret = IM_STATUS_NOERROR;

    rga_buffer_t pat;

    im_rect srect;
    im_rect drect;
    im_rect prect;

    empty_structure(NULL, NULL, &pat, &srect, &drect, &prect);

    if ((src.width != dst.width) || (src.height != dst.height))
        return IM_STATUS_INVALID_PARAM;

    if (sync == 0)
        usage |= IM_SYNC;

    ret = improcess(src, dst, pat, srect, drect, prect, usage);

    return ret;
}

IM_API IM_STATUS imcolorkey_t(const rga_buffer_t src, rga_buffer_t dst, im_colorkey_range range, int mode, int sync) {
    int usage = 0;
    IM_STATUS ret = IM_STATUS_NOERROR;

    rga_buffer_t pat;

    im_rect srect;
    im_rect drect;
    im_rect prect;

    empty_structure(NULL, NULL, &pat, &srect, &drect, &prect);

    usage |= mode;

    dst.colorkey_range = range;

    if (sync == 0)
        usage |= IM_SYNC;

    ret = improcess(src, dst, pat, srect, drect, prect, usage);

    return ret;
}

IM_API IM_STATUS imblend_t(const rga_buffer_t srcA, const rga_buffer_t srcB, rga_buffer_t dst, int mode, int sync) {
    int usage = 0;
    IM_STATUS ret = IM_STATUS_NOERROR;
    im_rect srect;
    im_rect drect;
    im_rect prect;

    empty_structure(NULL, NULL, NULL, &srect, &drect, &prect);

    usage |= mode;

    if (sync == 0)
        usage |= IM_SYNC;

    ret = improcess(srcA, dst, srcB, srect, drect, prect, usage);

    return ret;
}

IM_API IM_STATUS imcvtcolor_t(rga_buffer_t src, rga_buffer_t dst, int sfmt, int dfmt, int mode, int sync) {
    int usage = 0;
    IM_STATUS ret = IM_STATUS_NOERROR;

    rga_buffer_t pat;

    im_rect srect;
    im_rect drect;
    im_rect prect;

    empty_structure(NULL, NULL, &pat, &srect, &drect, &prect);

    src.format = sfmt;
    dst.format = dfmt;

    dst.color_space_mode = mode;

    if (sync == 0)
        usage |= IM_SYNC;

    ret = improcess(src, dst, pat, srect, drect, prect, usage);

    return ret;
}

IM_API IM_STATUS imquantize_t(const rga_buffer_t src, rga_buffer_t dst, im_nn_t nn_info, int sync) {
    int usage = 0;
    IM_STATUS ret = IM_STATUS_NOERROR;

    rga_buffer_t pat;

    im_rect srect;
    im_rect drect;
    im_rect prect;

    empty_structure(NULL, NULL, &pat, &srect, &drect, &prect);

    usage |= IM_NN_QUANTIZE;

    dst.nn = nn_info;

    if (sync == 0)
        usage |= IM_SYNC;

    ret = improcess(src, dst, pat, srect, drect, prect, usage);

    return ret;
}

IM_API IM_STATUS imrop_t(const rga_buffer_t src, rga_buffer_t dst, int rop_code, int sync) {
    int usage = 0;
    IM_STATUS ret = IM_STATUS_NOERROR;

    rga_buffer_t pat;

    im_rect srect;
    im_rect drect;
    im_rect prect;

    empty_structure(NULL, NULL, &pat, &srect, &drect, &prect);

    usage |= IM_ROP;

    dst.rop_code = rop_code;

    if (sync == 0)
        usage |= IM_SYNC;

    ret = improcess(src, dst, pat, srect, drect, prect, usage);

    return ret;
}

IM_API IM_STATUS improcess(rga_buffer_t src, rga_buffer_t dst, rga_buffer_t pat, im_rect srect, im_rect drect, im_rect prect, int usage) {
    rga_info_t srcinfo;
    rga_info_t dstinfo;
    rga_info_t patinfo;
    int ret;

    RockchipRga& rkRga(RockchipRga::get());

    src.format = RkRgaCompatibleFormat(src.format);
    dst.format = RkRgaCompatibleFormat(dst.format);
    pat.format = RkRgaCompatibleFormat(pat.format);

    memset(&srcinfo, 0, sizeof(rga_info_t));
    memset(&dstinfo, 0, sizeof(rga_info_t));
    memset(&patinfo, 0, sizeof(rga_info_t));

    if (usage & IM_COLOR_FILL)
        ret = rga_set_buffer_info(dst, &dstinfo);
    else
        ret = rga_set_buffer_info(src, dst, &srcinfo, &dstinfo);

    if (ret <= 0)
        return (IM_STATUS)ret;

    if ((usage & IM_ALPHA_BLEND_MASK) && rga_is_buffer_valid(pat)) /* A+B->C */
        imcheck_composite(src, dst, pat, srect, drect, prect, usage);
    else
        ret = imcheck(src, dst, srect, drect, usage);
    if(ret <= 0)
        return (IM_STATUS)ret;

    if (srect.width > 0 && srect.height > 0) {
        src.width = srect.width;
        src.height = srect.height;
        /* for imcrop_t api */
        if (usage & IM_CROP) {
            dst.width = srect.width < dst.width ? srect.width : dst.width;
            dst.height = srect.height < dst.height ? srect.height : dst.height;
        }
    }

    if (drect.width > 0 && drect.height > 0) {
        dst.width = drect.width;
        dst.height = drect.height;
    }

    rga_set_rect(&srcinfo.rect, srect.x, srect.y, src.width, src.height, src.wstride, src.hstride, src.format);
    rga_set_rect(&dstinfo.rect, drect.x, drect.y, dst.width, dst.height, dst.wstride, dst.hstride, dst.format);

    if (((usage & IM_COLOR_PALETTE) || (usage & IM_ALPHA_BLEND_MASK)) &&
        rga_is_buffer_valid(pat)) {

        ret = rga_set_buffer_info(pat, &patinfo);
        if (ret <= 0)
            return (IM_STATUS)ret;

        if (prect.width > 0 && prect.height > 0) {
            pat.width = prect.width;
            pat.height = prect.height;
        }

        rga_set_rect(&patinfo.rect, prect.x, prect.y, pat.width, pat.height, pat.wstride, pat.hstride, pat.format);
    }

    if((usage & (IM_ALPHA_BLEND_MASK+IM_HAL_TRANSFORM_MASK)) != 0) {
        /* Transform */
        switch(usage & IM_HAL_TRANSFORM_MASK) {
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
        switch(usage & IM_ALPHA_BLEND_MASK) {
            case IM_ALPHA_BLEND_SRC:
                srcinfo.blend = 0xff0001;
                break;
            case IM_ALPHA_BLEND_DST:
                srcinfo.blend = 0xff0002;
                break;
            case IM_ALPHA_BLEND_SRC_OVER:
                srcinfo.blend = 0xff0105;
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

    /* color key */
    if (usage & IM_ALPHA_COLORKEY_MASK) {
        srcinfo.blend = 0xff0105;

        srcinfo.colorkey_en = 1;
        srcinfo.colorkey_min = dst.colorkey_range.min;
        srcinfo.colorkey_max = dst.colorkey_range.max;
        switch (usage & IM_ALPHA_COLORKEY_MASK) {
            case IM_ALPHA_COLORKEY_NORMAL:
                srcinfo.colorkey_mode = 0;
                break;
            case IM_ALPHA_COLORKEY_INVERTED:
                srcinfo.colorkey_mode = 1;
                break;
        }
    }

    /* set NN quantize */
    if (usage & IM_NN_QUANTIZE) {
        dstinfo.nn.nn_flag = 1;
        dstinfo.nn.scale_r = dst.nn.scale_r;
        dstinfo.nn.scale_g = dst.nn.scale_g;
        dstinfo.nn.scale_b = dst.nn.scale_b;
        dstinfo.nn.offset_r = dst.nn.offset_r;
        dstinfo.nn.offset_g = dst.nn.offset_g;
        dstinfo.nn.offset_b = dst.nn.offset_b;
    }

    /* set ROP */
    if (usage & IM_ROP) {
        srcinfo.rop_code = dst.rop_code;
    }

    /* set global alpha */
    if ((src.global_alpha > 0) && (usage & IM_ALPHA_BLEND_MASK))
        srcinfo.blend &= src.global_alpha << 16;

    /* special config for yuv to rgb */
    if (dst.color_space_mode & (IM_YUV_TO_RGB_MASK)) {
        if (NormalRgaIsYuvFormat(RkRgaGetRgaFormat(src.format)) &&
            NormalRgaIsRgbFormat(RkRgaGetRgaFormat(dst.format)))
            dstinfo.color_space_mode = dst.color_space_mode;
        else {
            imErrorMsg("Not yuv to rgb does not need for color_sapce_mode, please fix.");
            return IM_STATUS_ILLEGAL_PARAM;
        }

    }

    /* special config for rgb to yuv */
    if (dst.color_space_mode & (IM_RGB_TO_YUV_MASK)) {
        if (NormalRgaIsRgbFormat(RkRgaGetRgaFormat(src.format)) &&
            NormalRgaIsYuvFormat(RkRgaGetRgaFormat(dst.format)))
            dstinfo.color_space_mode = dst.color_space_mode;
        else {
            imErrorMsg("Not rgb to yuv does not need for color_sapce_mode, please fix.");
            return IM_STATUS_ILLEGAL_PARAM;
        }
    }

    if (dst.format == RK_FORMAT_Y4) {
        switch (dst.color_space_mode) {
            case IM_RGB_TO_Y4 :
                dstinfo.dither.enable = 0;
                dstinfo.dither.mode = 0;
                break;
            case IM_RGB_TO_Y4_DITHER :
                dstinfo.dither.enable = 1;
                dstinfo.dither.mode = 0;
                break;
            case IM_RGB_TO_Y1_DITHER :
                dstinfo.dither.enable = 1;
                dstinfo.dither.mode = 1;
                break;
            default :
                dstinfo.dither.enable = 1;
                dstinfo.dither.mode = 0;
                break;
        }
        dstinfo.dither.lut0_l = 0x3210;
        dstinfo.dither.lut0_h = 0x7654;
        dstinfo.dither.lut1_l = 0xba98;
        dstinfo.dither.lut1_h = 0xfedc;
    }

    if (usage & IM_SYNC)
        dstinfo.sync_mode = RGA_BLIT_ASYNC;

    if (usage & IM_COLOR_FILL) {
        dstinfo.color = dst.color;
        ret = rkRga.RkRgaCollorFill(&dstinfo);
    } else if (usage & IM_COLOR_PALETTE) {
        ret = rkRga.RkRgaCollorPalette(&srcinfo, &dstinfo, &patinfo);
    } else if ((usage & IM_ALPHA_BLEND_MASK) && rga_is_buffer_valid(pat)) {
        dstinfo.color_space_mode = IM_COLOR_SPACE_DEFAULT;
        ret = rkRga.RkRgaBlit(&srcinfo, &dstinfo, &patinfo);
    }else {
        ret = rkRga.RkRgaBlit(&srcinfo, &dstinfo, NULL);
    }

    if (ret) {
        imErrorMsg("Failed to call Blit/ColorFill, query log to find the cause of failure.");
        return IM_STATUS_FAILED;
    }

    return IM_STATUS_SUCCESS;
}

IM_API IM_STATUS imsync(void) {
    int ret = 0;

    RockchipRga& rkRga(RockchipRga::get());

    ret = rkRga.RkRgaFlush();
    if (ret)
        return IM_STATUS_FAILED;

    return IM_STATUS_SUCCESS;
}

