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

#include <math.h>
#include <sstream>

#include "im2d.hpp"
#include "RgaUtils.h"
#include "core/rga_sync.h"

#ifdef ANDROID
#include <cutils/properties.h>

#include "core/NormalRga.h"
#include "RockchipRga.h"

using namespace android;
#endif

#ifdef LINUX
#include <sys/ioctl.h>

#include "../core/NormalRga.h"
#include "../include/RockchipRga.h"

#define ALOGE(...) { printf(__VA_ARGS__); printf("\n"); }
#endif

using namespace std;

#define ERR_MSG_LEN 300

extern struct rgaContext *rgaCtx;
__thread char rga_err_str[ERR_MSG_LEN] = "The current error message is empty!";
__thread im_context_t g_im2d_context;

#define ALIGN(val, align) (((val) + ((align) - 1)) & ~((align) - 1))
#define DOWN_ALIGN(val, align) ((val) & ~((align) - 1))
#define UNUSED(...) (void)(__VA_ARGS__)
#define imSetErrorMsg(...) \
({ \
    int ret = 0; \
    ret = snprintf(rga_err_str, ERR_MSG_LEN, __VA_ARGS__); \
    ret; \
})

IM_API const char* imStrError_t(IM_STATUS status) {
    const char *error_type[] = {
        "No errors during operation",
        "Run successfully",
        "Unsupported function",
        "Memory overflow",
        "Invalid parameters",
        "Illegal parameters",
        "verify librga and driver version",
        "Fatal error",
        "unkown status"
    };
    static __thread char error_str[ERR_MSG_LEN] = "The current error message is empty!";
    const char *ptr = NULL;

    switch(status) {
        case IM_STATUS_NOERROR :
            return error_type[0];

        case IM_STATUS_SUCCESS :
            return error_type[1];

        case IM_STATUS_NOT_SUPPORTED :
            ptr = error_type[2];
            break;

        case IM_STATUS_OUT_OF_MEMORY :
            ptr = error_type[3];
            break;

        case IM_STATUS_INVALID_PARAM :
            ptr = error_type[4];
            break;

        case IM_STATUS_ILLEGAL_PARAM :
            ptr = error_type[5];
            break;

        case IM_STATUS_ERROR_VERSION :
            ptr = error_type[6];
            break;

        case IM_STATUS_FAILED :
            ptr = error_type[7];
            break;

        default :
            return error_type[8];
    }

    snprintf(error_str, ERR_MSG_LEN, "%s: %s", ptr, rga_err_str);
    imSetErrorMsg("No error message, it has been cleared.");

    return error_str;
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
            imSetErrorMsg("invaild GraphicBuffer, can not get fd and virtual address, hnd = %p", (void *)hnd);
            goto INVAILD;
        }
    }

    ret = RkRgaGetHandleAttributes(hnd, &dstAttrs);
    if (ret) {
        ALOGE("rga_im2d: handle get Attributes fail ret = %d,hnd=%p", ret, &hnd);
        imSetErrorMsg("handle get Attributes fail, ret = %d,hnd = %p", ret, (void *)hnd);
        goto INVAILD;
    }

    buffer.width   = dstAttrs.at(AWIDTH);
    buffer.height  = dstAttrs.at(AHEIGHT);
    buffer.wstride = dstAttrs.at(ASTRIDE);
    buffer.hstride = dstAttrs.at(AHEIGHT);
    buffer.format  = dstAttrs.at(AFORMAT);

    if (buffer.wstride % 16) {
        ALOGE("rga_im2d: Graphicbuffer wstride needs align to 16, please align to 16 or use other buffer types.");
        imSetErrorMsg("Graphicbuffer wstride needs align to 16, please align to 16 or use other buffer types, wstride = %d", buffer.wstride);
        goto INVAILD;
    }

INVAILD:
    return buffer;
}

IM_API rga_buffer_t wrapbuffer_GraphicBuffer(sp<GraphicBuffer> buf) {
    int ret = 0;
    rga_buffer_t buffer;
    std::vector<int> dstAttrs;

    RockchipRga& rkRga(RockchipRga::get());

    memset(&buffer, 0, sizeof(rga_buffer_t));

    ret = rkRga.RkRgaGetBufferFd(buf->handle, &buffer.fd);
    if (ret)
        ALOGE("rga_im2d: get buffer fd fail: %s, hnd=%p", strerror(errno), (void*)(buf->handle));

    if (buffer.fd <= 0) {
        ret = rkRga.RkRgaGetHandleMapCpuAddress(buf->handle, &buffer.vir_addr);
        if(!buffer.vir_addr) {
            ALOGE("rga_im2d: invaild GraphicBuffer, can not get fd and virtual address.");
            imSetErrorMsg("invaild GraphicBuffer, can not get fd and virtual address, hnd = %p", (void *)(buf->handle));
            goto INVAILD;
        }
    }

    ret = RkRgaGetHandleAttributes(buf->handle, &dstAttrs);
    if (ret) {
        ALOGE("rga_im2d: handle get Attributes fail ret = %d,hnd=%p", ret, &buf->handle);
        imSetErrorMsg("handle get Attributes fail, ret = %d, hnd = %p", ret, (void *)(buf->handle));
        goto INVAILD;
    }

    buffer.width   = dstAttrs.at(AWIDTH);
    buffer.height  = dstAttrs.at(AHEIGHT);
    buffer.wstride = dstAttrs.at(ASTRIDE);
    buffer.hstride = dstAttrs.at(AHEIGHT);
    buffer.format  = dstAttrs.at(AFORMAT);

    if (buffer.wstride % 16) {
        ALOGE("rga_im2d: Graphicbuffer wstride needs align to 16, please align to 16 or use other buffer types, wstride = %d", buffer.wstride);
        imSetErrorMsg("Graphicbuffer wstride needs align to 16, please align to 16 or use other buffer types, wstride = %d", buffer.wstride);
        goto INVAILD;
    }

INVAILD:
    return buffer;
}

#if USE_AHARDWAREBUFFER
#include <android/hardware_buffer.h>
IM_API rga_buffer_t wrapbuffer_AHardwareBuffer(AHardwareBuffer *buf) {
    int ret = 0;
    rga_buffer_t buffer;
    std::vector<int> dstAttrs;

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
            imSetErrorMsg("invaild GraphicBuffer, can not get fd and virtual address, hnd = %p", (void *)(gbuffer->handle));
            goto INVAILD;
        }
    }

    ret = RkRgaGetHandleAttributes(gbuffer->handle, &dstAttrs);
    if (ret) {
        ALOGE("rga_im2d: handle get Attributes fail ret = %d,hnd=%p", ret, &gbuffer->handle);
        imSetErrorMsg("handle get Attributes fail, ret = %d, hnd = %p", ret, (void *)(gbuffer->handle));
        goto INVAILD;
    }

    buffer.width   = dstAttrs.at(AWIDTH);
    buffer.height  = dstAttrs.at(AHEIGHT);
    buffer.wstride = dstAttrs.at(ASTRIDE);
    buffer.hstride = dstAttrs.at(AHEIGHT);
    buffer.format  = dstAttrs.at(AFORMAT);

    if (buffer.wstride % 16) {
        ALOGE("rga_im2d: Graphicbuffer wstride needs align to 16, please align to 16 or use other buffer types, wstride = %d", buffer.wstride);
        imSetErrorMsg("Graphicbuffer wstride needs align to 16, please align to 16 or use other buffer types, wstride = %d", buffer.wstride);
        goto INVAILD;
    }

INVAILD:
    return buffer;
}
#endif
#endif

IM_API static void empty_structure(rga_buffer_t *src, rga_buffer_t *dst, rga_buffer_t *pat,
                                im_rect *srect, im_rect *drect, im_rect *prect, im_opt_t *opt) {
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
    if (opt != NULL)
        memset(opt, 0, sizeof(*opt));
}

IM_API static bool rga_is_buffer_valid(rga_buffer_t buf) {
    return (buf.phy_addr != NULL || buf.fd > 0 || buf.vir_addr != NULL);
}

IM_API static bool rga_is_rect_valid(im_rect rect) {
    return (rect.x > 0 || rect.y > 0 || (rect.width > 0 && rect.height > 0));
}

IM_API static IM_STATUS rga_align_check_yuv_8(const char *name, rga_buffer_t info, im_rect rect) {
    if ((info.wstride % 4) || (info.hstride % 2) ||
        (info.width % 2)  || (info.height % 2) ||
        (rect.x % 2) || (rect.y % 2) ||
        (rect.width % 2) || (rect.height % 2)) {
        imSetErrorMsg("%s, Error yuv not align to 2 or width stride not align to 4, "
                        "rect[x,y,w,h] = [%d, %d, %d, %d], "
                        "wstride = %d, hstride = %d, format = 0x%x(%s)\n%s",
                        name, rect.x, rect.y, info.width, info.height, info.wstride, info.hstride,
                        info.format, translate_format_str(info.format),
                        querystring((strcmp("dst", name) == 0) ? RGA_OUTPUT_FORMAT : RGA_INPUT_FORMAT));
        return IM_STATUS_INVALID_PARAM;
    }

    return IM_STATUS_SUCCESS;
}

IM_API static IM_STATUS rga_align_check_yuv_10(const char *name, rga_buffer_t info, im_rect rect) {
    if ((info.wstride % 16) || (info.hstride % 2) ||
        (info.width % 2)  || (info.height % 2) ||
        (rect.x % 2) || (rect.y % 2) ||
        (rect.width % 2) || (rect.height % 2)) {
        imSetErrorMsg("%s, Err src wstride is not align to 16 or yuv not align to 2, "
                        "rect[x,y,w,h] = [%d, %d, %d, %d], "
                        "wstride = %d, hstride = %d, format = 0x%x(%s)\n%s",
                        name, rect.x, rect.y, info.width, info.height, info.wstride, info.hstride,
                        info.format, translate_format_str(info.format),
                        querystring((strcmp("dst", name) == 0) ? RGA_OUTPUT_FORMAT : RGA_INPUT_FORMAT));
        return IM_STATUS_INVALID_PARAM;
    }

    return IM_STATUS_SUCCESS;
}

IM_API static IM_STATUS rga_support_info_merge_table(rga_info_table_entry *dst_table, rga_info_table_entry *merge_table) {
    if (dst_table == NULL || merge_table == NULL) {
        ALOGE("%s[%d] dst or merge table is NULL!\n", __FUNCTION__, __LINE__);
        return IM_STATUS_FAILED;
    }

    dst_table->version |= merge_table->version;
    dst_table->input_format |= merge_table->input_format;
    dst_table->output_format |= merge_table->output_format;
    dst_table->feature |= merge_table->feature;

    dst_table->input_resolution = dst_table->input_resolution > merge_table->input_resolution ?
                                  dst_table->input_resolution : merge_table->input_resolution;
    dst_table->output_resolution = dst_table->output_resolution > merge_table->output_resolution ?
                                   dst_table->output_resolution : merge_table->output_resolution;
    dst_table->scale_limit = dst_table->scale_limit > merge_table->scale_limit ?
                             dst_table->scale_limit : merge_table->scale_limit;
    dst_table->performance = dst_table->performance > merge_table->performance ?
                             dst_table->performance : merge_table->performance;

    return IM_STATUS_SUCCESS;
}

/*
 * rga_version_compare() - Used to compare two struct rga_version_t.
 * @version1
 * @version2
 *
 * if version1 > version2, return >0;
 * if version1 = version2, return 0;
 * if version1 < version2, retunr <0.
 */
IM_API static inline int rga_version_compare(struct rga_version_t version1, struct rga_version_t version2) {
    if (version1.major > version2.major)
        return 1;
    else if (version1.major == version2.major && version1.minor > version2.minor)
        return 1;
    else if (version1.major == version2.major && version1.minor == version2.minor && version1.revision > version2.revision)
        return 1;
    else if (version1.major == version2.major && version1.minor == version2.minor && version1.revision == version2.revision)
        return 0;

    return -1;
}

IM_API IM_STATUS rga_set_buffer_info(rga_buffer_t dst, rga_info_t* dstinfo) {
    if(NULL == dstinfo) {
        ALOGE("rga_im2d: invaild dstinfo");
        imSetErrorMsg("Dst structure address is NULL.");
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
        imSetErrorMsg("No address available in dst buffer, phy_addr = %ld, fd = %d, vir_addr = %ld",
                      (unsigned long)dst.phy_addr, dst.fd, (unsigned long)dst.vir_addr);
        return IM_STATUS_INVALID_PARAM;
    }

    return IM_STATUS_SUCCESS;
}

IM_API IM_STATUS rga_set_buffer_info(const rga_buffer_t src, rga_buffer_t dst, rga_info_t* srcinfo, rga_info_t* dstinfo) {
    if(NULL == srcinfo) {
        ALOGE("rga_im2d: invaild srcinfo");
        imSetErrorMsg("Src structure address is NULL.");
        return IM_STATUS_INVALID_PARAM;
    }
    if(NULL == dstinfo) {
        ALOGE("rga_im2d: invaild dstinfo");
        imSetErrorMsg("Dst structure address is NULL.");
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
        imSetErrorMsg("No address available in src buffer, phy_addr = %ld, fd = %d, vir_addr = %ld",
                      (unsigned long)src.phy_addr, src.fd, (unsigned long)src.vir_addr);
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
        imSetErrorMsg("No address available in dst buffer, phy_addr = %ld, fd = %d, vir_addr = %ld",
                      (unsigned long)dst.phy_addr, dst.fd, (unsigned long)dst.vir_addr);
        return IM_STATUS_INVALID_PARAM;
    }

    return IM_STATUS_SUCCESS;
}

IM_API static inline void rga_apply_rect(rga_buffer_t *image, im_rect *rect) {
    if (rect->width > 0 && rect->height > 0) {
        image->width = rect->width;
        image->height = rect->height;
    }
}

IM_API void rga_check_perpare(rga_buffer_t *src, rga_buffer_t *dst, rga_buffer_t *pat,
                              im_rect *src_rect, im_rect *dst_rect, im_rect *pat_rect, int mode_usage) {

      if (mode_usage & IM_CROP) {
          dst_rect->width = src_rect->width;
          dst_rect->height = src_rect->height;
      }

      rga_apply_rect(src, src_rect);
      rga_apply_rect(dst, dst_rect);
      if (rga_is_buffer_valid(*pat))
          rga_apply_rect(pat, pat_rect);
}

IM_API IM_STATUS rga_get_info(rga_info_table_entry *return_table) {
    rga_info_table_entry merge_table;
    int  rga_version = 0;

    static rga_info_table_entry table[] = {
        { IM_RGA_HW_VERSION_RGA_V_ERR       ,    0,     0,  0, 0,   0, 0, 0, {0} },
        {   IM_RGA_HW_VERSION_RGA_1         , 8192, 2048,   8, 1,
                                            /* input format */
                                            IM_RGA_SUPPORT_FORMAT_RGB |
                                            IM_RGA_SUPPORT_FORMAT_RGB_OTHER |
                                            IM_RGA_SUPPORT_FORMAT_BPP |
                                            IM_RGA_SUPPORT_FORMAT_YUV_420_SEMI_PLANNER_8_BIT |
                                            IM_RGA_SUPPORT_FORMAT_YUV_420_PLANNER_8_BIT |
                                            IM_RGA_SUPPORT_FORMAT_YUV_422_SEMI_PLANNER_8_BIT |
                                            IM_RGA_SUPPORT_FORMAT_YUV_422_PLANNER_8_BIT,
                                            /* output format */
                                            IM_RGA_SUPPORT_FORMAT_RGB |
                                            IM_RGA_SUPPORT_FORMAT_RGB_OTHER |
                                            IM_RGA_SUPPORT_FORMAT_YUV_420_SEMI_PLANNER_8_BIT |
                                            IM_RGA_SUPPORT_FORMAT_YUV_420_PLANNER_8_BIT |
                                            IM_RGA_SUPPORT_FORMAT_YUV_422_SEMI_PLANNER_8_BIT |
                                            IM_RGA_SUPPORT_FORMAT_YUV_422_PLANNER_8_BIT,
                                            /* feature */
                                            IM_RGA_SUPPORT_FEATURE_COLOR_FILL |
                                            IM_RGA_SUPPORT_FEATURE_COLOR_PALETTE |
                                            IM_RGA_SUPPORT_FEATURE_ROP,
                                            /* reserved */
                                            {0} },
        { IM_RGA_HW_VERSION_RGA_1_PLUS      , 8192, 2048,   8, 1,
                                            /* input format */
                                            IM_RGA_SUPPORT_FORMAT_RGB |
                                            IM_RGA_SUPPORT_FORMAT_RGB_OTHER |
                                            IM_RGA_SUPPORT_FORMAT_BPP |
                                            IM_RGA_SUPPORT_FORMAT_YUV_420_SEMI_PLANNER_8_BIT |
                                            IM_RGA_SUPPORT_FORMAT_YUV_420_PLANNER_8_BIT |
                                            IM_RGA_SUPPORT_FORMAT_YUV_422_SEMI_PLANNER_8_BIT |
                                            IM_RGA_SUPPORT_FORMAT_YUV_422_PLANNER_8_BIT,
                                            /* output format */
                                            IM_RGA_SUPPORT_FORMAT_RGB |
                                            IM_RGA_SUPPORT_FORMAT_RGB_OTHER |
                                            IM_RGA_SUPPORT_FORMAT_YUV_420_SEMI_PLANNER_8_BIT |
                                            IM_RGA_SUPPORT_FORMAT_YUV_420_PLANNER_8_BIT |
                                            IM_RGA_SUPPORT_FORMAT_YUV_422_SEMI_PLANNER_8_BIT |
                                            IM_RGA_SUPPORT_FORMAT_YUV_422_PLANNER_8_BIT,
                                            /* feature */
                                            IM_RGA_SUPPORT_FEATURE_COLOR_FILL |
                                            IM_RGA_SUPPORT_FEATURE_COLOR_PALETTE,
                                            /* reserved */
                                            {0} },
        { IM_RGA_HW_VERSION_RGA_2           , 8192, 4096, 16, 2,
                                            /* input format */
                                            IM_RGA_SUPPORT_FORMAT_RGB |
                                            IM_RGA_SUPPORT_FORMAT_RGB_OTHER |
                                            IM_RGA_SUPPORT_FORMAT_YUV_420_SEMI_PLANNER_8_BIT |
                                            IM_RGA_SUPPORT_FORMAT_YUV_420_PLANNER_8_BIT |
                                            IM_RGA_SUPPORT_FORMAT_YUV_422_SEMI_PLANNER_8_BIT |
                                            IM_RGA_SUPPORT_FORMAT_YUV_422_PLANNER_8_BIT,
                                            /* output format */
                                            IM_RGA_SUPPORT_FORMAT_RGB |
                                            IM_RGA_SUPPORT_FORMAT_RGB_OTHER |
                                            IM_RGA_SUPPORT_FORMAT_YUV_420_SEMI_PLANNER_8_BIT |
                                            IM_RGA_SUPPORT_FORMAT_YUV_420_PLANNER_8_BIT |
                                            IM_RGA_SUPPORT_FORMAT_YUV_422_SEMI_PLANNER_8_BIT |
                                            IM_RGA_SUPPORT_FORMAT_YUV_422_PLANNER_8_BIT,
                                            /* feature */
                                            IM_RGA_SUPPORT_FEATURE_COLOR_FILL |
                                            IM_RGA_SUPPORT_FEATURE_COLOR_PALETTE |
                                            IM_RGA_SUPPORT_FEATURE_ROP,
                                            /* reserved */
                                            {0} },
        { IM_RGA_HW_VERSION_RGA_2_LITE0     , 8192, 4096,   8, 2,
                                            /* input format */
                                            IM_RGA_SUPPORT_FORMAT_RGB |
                                            IM_RGA_SUPPORT_FORMAT_RGB_OTHER |
                                            IM_RGA_SUPPORT_FORMAT_YUV_420_SEMI_PLANNER_8_BIT |
                                            IM_RGA_SUPPORT_FORMAT_YUV_420_PLANNER_8_BIT |
                                            IM_RGA_SUPPORT_FORMAT_YUV_422_SEMI_PLANNER_8_BIT |
                                            IM_RGA_SUPPORT_FORMAT_YUV_422_PLANNER_8_BIT,
                                            /* output format */
                                            IM_RGA_SUPPORT_FORMAT_RGB |
                                            IM_RGA_SUPPORT_FORMAT_RGB_OTHER |
                                            IM_RGA_SUPPORT_FORMAT_YUV_420_SEMI_PLANNER_8_BIT |
                                            IM_RGA_SUPPORT_FORMAT_YUV_420_PLANNER_8_BIT |
                                            IM_RGA_SUPPORT_FORMAT_YUV_422_SEMI_PLANNER_8_BIT |
                                            IM_RGA_SUPPORT_FORMAT_YUV_422_PLANNER_8_BIT,
                                            /* feature */
                                            IM_RGA_SUPPORT_FEATURE_COLOR_FILL |
                                            IM_RGA_SUPPORT_FEATURE_COLOR_PALETTE |
                                            IM_RGA_SUPPORT_FEATURE_ROP,
                                            /* reserved */
                                            {0} },
        { IM_RGA_HW_VERSION_RGA_2_LITE1     , 8192, 4096,   8, 2,
                                            /* input format */
                                            IM_RGA_SUPPORT_FORMAT_RGB |
                                            IM_RGA_SUPPORT_FORMAT_RGB_OTHER |
                                            IM_RGA_SUPPORT_FORMAT_YUV_420_SEMI_PLANNER_8_BIT |
                                            IM_RGA_SUPPORT_FORMAT_YUV_420_PLANNER_8_BIT |
                                            IM_RGA_SUPPORT_FORMAT_YUV_422_SEMI_PLANNER_8_BIT |
                                            IM_RGA_SUPPORT_FORMAT_YUV_422_PLANNER_8_BIT |
                                            IM_RGA_SUPPORT_FORMAT_YUV_420_SEMI_PLANNER_10_BIT |
                                            IM_RGA_SUPPORT_FORMAT_YUV_420_PLANNER_10_BIT |
                                            IM_RGA_SUPPORT_FORMAT_YUV_422_SEMI_PLANNER_10_BIT |
                                            IM_RGA_SUPPORT_FORMAT_YUV_422_PLANNER_10_BIT,
                                            /* output format */
                                            IM_RGA_SUPPORT_FORMAT_RGB |
                                            IM_RGA_SUPPORT_FORMAT_RGB_OTHER |
                                            IM_RGA_SUPPORT_FORMAT_YUV_420_SEMI_PLANNER_8_BIT |
                                            IM_RGA_SUPPORT_FORMAT_YUV_420_PLANNER_8_BIT |
                                            IM_RGA_SUPPORT_FORMAT_YUV_422_SEMI_PLANNER_8_BIT |
                                            IM_RGA_SUPPORT_FORMAT_YUV_422_PLANNER_8_BIT,
                                            /* feature */
                                            IM_RGA_SUPPORT_FEATURE_COLOR_FILL |
                                            IM_RGA_SUPPORT_FEATURE_COLOR_PALETTE,
                                            /* reserved */
                                            {0} },
        { IM_RGA_HW_VERSION_RGA_2_ENHANCE   , 8192, 4096, 16,  2,
                                            /* input format */
                                            IM_RGA_SUPPORT_FORMAT_RGB |
                                            IM_RGA_SUPPORT_FORMAT_RGB_OTHER |
                                            IM_RGA_SUPPORT_FORMAT_YUV_420_SEMI_PLANNER_8_BIT |
                                            IM_RGA_SUPPORT_FORMAT_YUV_420_PLANNER_8_BIT |
                                            IM_RGA_SUPPORT_FORMAT_YUV_422_SEMI_PLANNER_8_BIT |
                                            IM_RGA_SUPPORT_FORMAT_YUV_422_PLANNER_8_BIT |
                                            IM_RGA_SUPPORT_FORMAT_YUV_420_SEMI_PLANNER_10_BIT |
                                            IM_RGA_SUPPORT_FORMAT_YUV_420_PLANNER_10_BIT |
                                            IM_RGA_SUPPORT_FORMAT_YUV_422_SEMI_PLANNER_10_BIT |
                                            IM_RGA_SUPPORT_FORMAT_YUV_422_PLANNER_10_BIT,
                                            /* output format */
                                            IM_RGA_SUPPORT_FORMAT_RGB |
                                            IM_RGA_SUPPORT_FORMAT_RGB_OTHER |
                                            IM_RGA_SUPPORT_FORMAT_YUV_420_SEMI_PLANNER_8_BIT |
                                            IM_RGA_SUPPORT_FORMAT_YUV_420_PLANNER_8_BIT |
                                            IM_RGA_SUPPORT_FORMAT_YUV_422_SEMI_PLANNER_8_BIT |
                                            IM_RGA_SUPPORT_FORMAT_YUV_422_PLANNER_8_BIT |
                                            IM_RGA_SUPPORT_FORMAT_YUYV_420 |
                                            IM_RGA_SUPPORT_FORMAT_YUYV_422,
                                            /* feature */
                                            IM_RGA_SUPPORT_FEATURE_COLOR_FILL |
                                            IM_RGA_SUPPORT_FEATURE_COLOR_PALETTE |
                                            IM_RGA_SUPPORT_FEATURE_ROP,
                                            /* reserved */
                                            {0} },
        { IM_RGA_HW_VERSION_RGA_3           , 8176, 8128,  8,  4,
                                            /* input format */
                                            IM_RGA_SUPPORT_FORMAT_RGB |
                                            IM_RGA_SUPPORT_FORMAT_YUV_420_SEMI_PLANNER_8_BIT |
                                            IM_RGA_SUPPORT_FORMAT_YUV_422_SEMI_PLANNER_8_BIT |
                                            IM_RGA_SUPPORT_FORMAT_YUV_420_SEMI_PLANNER_10_BIT |
                                            IM_RGA_SUPPORT_FORMAT_YUV_422_SEMI_PLANNER_10_BIT |
                                            IM_RGA_SUPPORT_FORMAT_YUYV_422,
                                            /* output format */
                                            IM_RGA_SUPPORT_FORMAT_RGB |
                                            IM_RGA_SUPPORT_FORMAT_YUV_420_SEMI_PLANNER_8_BIT |
                                            IM_RGA_SUPPORT_FORMAT_YUV_422_SEMI_PLANNER_8_BIT |
                                            IM_RGA_SUPPORT_FORMAT_YUV_420_SEMI_PLANNER_10_BIT |
                                            IM_RGA_SUPPORT_FORMAT_YUV_422_SEMI_PLANNER_10_BIT |
                                            IM_RGA_SUPPORT_FORMAT_YUYV_422,
                                            /* feature */
                                            IM_RGA_SUPPORT_FEATURE_FBC |
                                            IM_RGA_SUPPORT_FEATURE_BLEND_YUV |
                                            IM_RGA_SUPPORT_FEATURE_BT2020,
                                            /* reserved */
                                            {0} },
    };

    /* Get RGA context */
    if (rgaCtx == NULL) {
        RockchipRga& rkRga(RockchipRga::get());
        if (rgaCtx == NULL) {
            memcpy(return_table, &table[IM_RGA_HW_VERSION_RGA_V_ERR_INDEX], sizeof(return_table));

            ALOGE("rga_im2d: The current RockchipRga singleton is destroyed. "
                  "Please check if RkRgaInit/RkRgaDeInit are called, if so, please disable them.");
            imSetErrorMsg("The current RockchipRga singleton is destroyed."
                          "Please check if RkRgaInit/RkRgaDeInit are called, if so, please disable them.");
            return IM_STATUS_FAILED;
        }
    }

    memset(&merge_table, 0x0, sizeof(merge_table));

    for (int i = 0; i < rgaCtx->mHwVersions.size; i++) {
        if (rgaCtx->mHwVersions.version[i].major == 2 &&
            rgaCtx->mHwVersions.version[i].minor == 0) {
            if (rgaCtx->mHwVersions.version[i].revision == 0) {
                rga_version = IM_RGA_HW_VERSION_RGA_2_INDEX;
                memcpy(&merge_table, &table[rga_version], sizeof(merge_table));
            } else {
                goto TRY_TO_COMPATIBLE;
            }
        } else if (rgaCtx->mHwVersions.version[i].major == 3 &&
                   rgaCtx->mHwVersions.version[i].minor == 0) {
            switch (rgaCtx->mHwVersions.version[i].revision) {
                case 0x16445 :
                    rga_version = IM_RGA_HW_VERSION_RGA_2_INDEX;
                    memcpy(&merge_table, &table[rga_version], sizeof(merge_table));
                    break;
                case 0x22245 :
                    rga_version = IM_RGA_HW_VERSION_RGA_2_ENHANCE_INDEX;
                    memcpy(&merge_table, &table[rga_version], sizeof(merge_table));
                    break;
                case 0x76831 :
                    rga_version = IM_RGA_HW_VERSION_RGA_3_INDEX;
                    memcpy(&merge_table, &table[rga_version], sizeof(merge_table));
                    break;
                default :
                    goto TRY_TO_COMPATIBLE;
            }
        } else if (rgaCtx->mHwVersions.version[i].major == 3 &&
                   rgaCtx->mHwVersions.version[i].minor == 2) {
            switch (rgaCtx->mHwVersions.version[i].revision) {
                case 0x18218 :
                    rga_version = IM_RGA_HW_VERSION_RGA_2_ENHANCE_INDEX;
                    memcpy(&merge_table, &table[rga_version], sizeof(merge_table));

                    merge_table.feature |= IM_RGA_SUPPORT_FEATURE_ROP;
                    break;
                case 0x56726 :
                case 0x63318 :
                    rga_version = IM_RGA_HW_VERSION_RGA_2_ENHANCE_INDEX;
                    memcpy(&merge_table, &table[rga_version], sizeof(merge_table));

                    merge_table.input_format |= IM_RGA_SUPPORT_FORMAT_YUYV_422 |
                                                 IM_RGA_SUPPORT_FORMAT_YUV_400;
                    merge_table.output_format |= IM_RGA_SUPPORT_FORMAT_YUV_400 |
                                                  IM_RGA_SUPPORT_FORMAT_Y4;
                    merge_table.feature |= IM_RGA_SUPPORT_FEATURE_QUANTIZE |
                                            IM_RGA_SUPPORT_FEATURE_SRC1_R2Y_CSC |
                                            IM_RGA_SUPPORT_FEATURE_DST_FULL_CSC;
                    break;
                default :
                    goto TRY_TO_COMPATIBLE;
            }
        } else if (rgaCtx->mHwVersions.version[i].major == 4 &&
                   rgaCtx->mHwVersions.version[i].minor == 0) {
            switch (rgaCtx->mHwVersions.version[i].revision) {
                case 0x18632 :
                    rga_version = IM_RGA_HW_VERSION_RGA_2_LITE0_INDEX;
                    memcpy(&merge_table, &table[rga_version], sizeof(merge_table));
                    break;
                case 0x23998 :
                    rga_version = IM_RGA_HW_VERSION_RGA_2_LITE1_INDEX;
                    memcpy(&merge_table, &table[rga_version], sizeof(merge_table));

                    merge_table.feature |= IM_RGA_SUPPORT_FEATURE_SRC1_R2Y_CSC;
                    break;
                default :
                    goto TRY_TO_COMPATIBLE;
            }
        } else if (rgaCtx->mHwVersions.version[i].major == 42 &&
                   rgaCtx->mHwVersions.version[i].minor == 0) {
            if (rgaCtx->mHwVersions.version[i].revision == 0x17760) {
                rga_version = IM_RGA_HW_VERSION_RGA_2_LITE1_INDEX;
                memcpy(&merge_table, &table[rga_version], sizeof(merge_table));
            } else {
                goto TRY_TO_COMPATIBLE;
            }
        } else {
            goto TRY_TO_COMPATIBLE;
        }

        rga_support_info_merge_table(return_table, &merge_table);
    }

    return IM_STATUS_SUCCESS;

TRY_TO_COMPATIBLE:
    if (strncmp((char *)rgaCtx->mHwVersions.version[0].str, "1.3", 3) == 0)
        rga_version = IM_RGA_HW_VERSION_RGA_1_INDEX;
    else if (strncmp((char *)rgaCtx->mHwVersions.version[0].str, "1.6", 3) == 0)
        rga_version = IM_RGA_HW_VERSION_RGA_1_PLUS_INDEX;
    /*3288 vesion is 2.00*/
    else if (strncmp((char *)rgaCtx->mHwVersions.version[0].str, "2.00", 4) == 0)
        rga_version = IM_RGA_HW_VERSION_RGA_2_INDEX;
    /*3288w version is 3.00*/
    else if (strncmp((char *)rgaCtx->mHwVersions.version[0].str, "3.00", 4) == 0)
        rga_version = IM_RGA_HW_VERSION_RGA_2_INDEX;
    else if (strncmp((char *)rgaCtx->mHwVersions.version[0].str, "3.02", 4) == 0)
        rga_version = IM_RGA_HW_VERSION_RGA_2_ENHANCE_INDEX;
    else if (strncmp((char *)rgaCtx->mHwVersions.version[0].str, "4.00", 4) == 0)
        rga_version = IM_RGA_HW_VERSION_RGA_2_LITE0_INDEX;
    /*The version number of lite1 cannot be obtained temporarily.*/
    else if (strncmp((char *)rgaCtx->mHwVersions.version[0].str, "4.00", 4) == 0)
        rga_version = IM_RGA_HW_VERSION_RGA_2_LITE1_INDEX;
    else
        rga_version = IM_RGA_HW_VERSION_RGA_V_ERR_INDEX;

    memcpy(return_table, &table[rga_version], sizeof(rga_info_table_entry));

    if (rga_version == IM_RGA_HW_VERSION_RGA_V_ERR_INDEX) {
        ALOGE("rga_im2d: Can not get the correct RGA version, please check the driver, version=%s\n",
              rgaCtx->mHwVersions.version[0].str);
        imSetErrorMsg("Can not get the correct RGA version, please check the driver, version=%s",
                      rgaCtx->mHwVersions.version[0].str);
        return IM_STATUS_FAILED;
    }

    return IM_STATUS_SUCCESS;
}

IM_API const char* querystring(int name) {
    bool all_output = 0, all_output_prepared = 0;
    int rga_version = 0;
    long usage = 0;
    enum {
        RGA_API = 0,
    };
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
        "RGA feature           : ",
        "expected performance  : ",
    };
    const char *version_name[] = {
        "RGA_api version       : ",
    };
    const char *output_version[] = {
        "unknown ",
        "RGA_1 ",
        "RGA_1_plus ",
        "RGA_2 ",
        "RGA_2_lite0 ",
        "RGA_2_lite1 ",
        "RGA_2_Enhance ",
        "RGA_3 ",
    };
    const char *output_resolution[] = {
        "unknown",
        "2048x2048",
        "4096x4096",
        "8192x8192",
        "8128x8128",
    };
    const char *output_scale_limit[] = {
        "unknown",
        "0.125 ~ 8",
        "0.0625 ~ 16"
    };
    const char *output_format[] = {
        "unknown",
        "RGBA_8888 RGB_888 RGB_565 ",
        "RGBA_4444 RGBA_5551 ",
        "BPP8 BPP4 BPP2 BPP1 ",
        "YUV420_sp_8bit ",
        "YUV420_sp_10bit ",
        "YUV420_p_8bit ",
        "YUV420_p_10bit ",
        "YUV422_sp_8bit ",
        "YUV422_sp_10bit ",
        "YUV422_p_8bit ",
        "YUV422_p_10bit ",
        "YUYV420 ",
        "YUYV422 ",
        "YUV400/Y4 "
    };
    const char *feature[] = {
        "unknown ",
        "color_fill ",
        "color_palette ",
        "ROP ",
        "quantize ",
        "src1_r2y_csc ",
        "dst_full_csc ",
        "FBC_mode ",
        "blend_in_YUV ",
        "BT.2020 ",
    };
    const char *performance[] = {
        "unknown",
        "max 1 pixel/cycle ",
        "max 2 pixel/cycle ",
        "max 4 pixel/cycle ",
    };
    ostringstream out;
    static string info;

    rga_info_table_entry rga_info;

    usage = rga_get_info(&rga_info);
    if (IM_STATUS_FAILED == usage) {
        ALOGE("rga im2d: rga2 get info failed!\n");
        return "get info failed";
    }

    do {
        switch(name) {
            case RGA_VENDOR :
                out << output_name[name] << output_vendor << endl;
                break;

            case RGA_VERSION :
                out << version_name[RGA_API] << "v" << RGA_API_VERSION << endl;

                out << output_name[name];
                if (rga_info.version == IM_RGA_HW_VERSION_RGA_V_ERR) {
                    out << output_version[IM_RGA_HW_VERSION_RGA_V_ERR_INDEX];
                } else {
                    if (rga_info.version & IM_RGA_HW_VERSION_RGA_1)
                        out << output_version[IM_RGA_HW_VERSION_RGA_1_INDEX];
                    if (rga_info.version & IM_RGA_HW_VERSION_RGA_1_PLUS)
                        out << output_version[IM_RGA_HW_VERSION_RGA_1_PLUS_INDEX];
                    if (rga_info.version & IM_RGA_HW_VERSION_RGA_2)
                        out << output_version[IM_RGA_HW_VERSION_RGA_2_INDEX];
                    if (rga_info.version & IM_RGA_HW_VERSION_RGA_2_LITE0)
                        out << output_version[IM_RGA_HW_VERSION_RGA_2_LITE0_INDEX];
                    if (rga_info.version & IM_RGA_HW_VERSION_RGA_2_LITE1)
                        out << output_version[IM_RGA_HW_VERSION_RGA_2_LITE1_INDEX];
                    if (rga_info.version & IM_RGA_HW_VERSION_RGA_2_ENHANCE)
                        out << output_version[IM_RGA_HW_VERSION_RGA_2_ENHANCE_INDEX];
                    if (rga_info.version & IM_RGA_HW_VERSION_RGA_3)
                        out << output_version[IM_RGA_HW_VERSION_RGA_3_INDEX];
                }
                out << endl;
                break;

            case RGA_MAX_INPUT :
                switch (rga_info.input_resolution) {
                    case 2048 :
                        out << output_name[name] << output_resolution[1] << endl;
                        break;
                    case 4096 :
                        out << output_name[name] << output_resolution[2] << endl;
                        break;
                    case 8192 :
                        out << output_name[name] << output_resolution[3] << endl;
                        break;
                    case 8128 :
                        out << output_name[name] << output_resolution[4] << endl;
                        break;
                    default :
                        out << output_name[name] << output_resolution[IM_RGA_HW_VERSION_RGA_V_ERR_INDEX] << endl;
                        break;
                }
                break;

            case RGA_MAX_OUTPUT :
                switch(rga_info.output_resolution) {
                    case 2048 :
                        out << output_name[name] << output_resolution[1] << endl;
                        break;
                    case 4096 :
                        out << output_name[name] << output_resolution[2] << endl;
                        break;
                    case 8192 :
                        out << output_name[name] << output_resolution[3] << endl;
                        break;
                    case 8128 :
                        out << output_name[name] << output_resolution[4] << endl;
                        break;
                    default :
                        out << output_name[name] << output_resolution[IM_RGA_HW_VERSION_RGA_V_ERR_INDEX] << endl;
                        break;
                }
                break;

            case RGA_SCALE_LIMIT :
                switch(rga_info.scale_limit) {
                    case 8 :
                        out << output_name[name] << output_scale_limit[1] << endl;
                        break;
                    case 16 :
                        out << output_name[name] << output_scale_limit[2] << endl;
                        break;
                    default :
                        out << output_name[name] << output_scale_limit[IM_RGA_HW_VERSION_RGA_V_ERR_INDEX] << endl;
                        break;
                }
                break;

            case RGA_INPUT_FORMAT :
                out << output_name[name];
                if(rga_info.input_format & IM_RGA_SUPPORT_FORMAT_RGB)
                    out << output_format[IM_RGA_SUPPORT_FORMAT_RGB_INDEX];
                if(rga_info.input_format & IM_RGA_SUPPORT_FORMAT_RGB_OTHER)
                    out << output_format[IM_RGA_SUPPORT_FORMAT_RGB_OTHER_INDEX];
                if(rga_info.input_format & IM_RGA_SUPPORT_FORMAT_BPP)
                    out << output_format[IM_RGA_SUPPORT_FORMAT_BPP_INDEX];
                if(rga_info.input_format & IM_RGA_SUPPORT_FORMAT_YUV_420_SEMI_PLANNER_8_BIT)
                    out << output_format[IM_RGA_SUPPORT_FORMAT_YUV_420_SEMI_PLANNER_8_BIT_INDEX];
                if(rga_info.input_format & IM_RGA_SUPPORT_FORMAT_YUV_420_SEMI_PLANNER_10_BIT)
                    out << output_format[IM_RGA_SUPPORT_FORMAT_YUV_420_SEMI_PLANNER_10_BIT_INDEX];
                if(rga_info.input_format & IM_RGA_SUPPORT_FORMAT_YUV_420_PLANNER_8_BIT)
                    out << output_format[IM_RGA_SUPPORT_FORMAT_YUV_420_PLANNER_8_BIT_INDEX];
                if(rga_info.input_format & IM_RGA_SUPPORT_FORMAT_YUV_420_PLANNER_10_BIT)
                    out << output_format[IM_RGA_SUPPORT_FORMAT_YUV_420_PLANNER_10_BIT_INDEX];
                if(rga_info.input_format & IM_RGA_SUPPORT_FORMAT_YUV_422_SEMI_PLANNER_8_BIT)
                    out << output_format[IM_RGA_SUPPORT_FORMAT_YUV_422_SEMI_PLANNER_8_BIT_INDEX];
                if(rga_info.input_format & IM_RGA_SUPPORT_FORMAT_YUV_422_SEMI_PLANNER_10_BIT)
                    out << output_format[IM_RGA_SUPPORT_FORMAT_YUV_422_SEMI_PLANNER_10_BIT_INDEX];
                if(rga_info.input_format & IM_RGA_SUPPORT_FORMAT_YUV_422_PLANNER_8_BIT)
                    out << output_format[IM_RGA_SUPPORT_FORMAT_YUV_422_PLANNER_8_BIT_INDEX];
                if(rga_info.input_format & IM_RGA_SUPPORT_FORMAT_YUV_422_PLANNER_10_BIT)
                    out << output_format[IM_RGA_SUPPORT_FORMAT_YUV_422_PLANNER_10_BIT_INDEX];
                if(rga_info.input_format & IM_RGA_SUPPORT_FORMAT_YUYV_420)
                    out << output_format[IM_RGA_SUPPORT_FORMAT_YUYV_420_INDEX];
                if(rga_info.input_format & IM_RGA_SUPPORT_FORMAT_YUYV_422)
                    out << output_format[IM_RGA_SUPPORT_FORMAT_YUYV_422_INDEX];
                if(rga_info.input_format & IM_RGA_SUPPORT_FORMAT_YUV_400)
                    out << output_format[IM_RGA_SUPPORT_FORMAT_YUV_400_INDEX];
                if(!(rga_info.input_format & IM_RGA_SUPPORT_FORMAT_MASK))
                    out << output_format[IM_RGA_SUPPORT_FORMAT_ERROR_INDEX];
                out << endl;
                break;

            case RGA_OUTPUT_FORMAT :
                out << output_name[name];
                if(rga_info.output_format & IM_RGA_SUPPORT_FORMAT_RGB)
                    out << output_format[IM_RGA_SUPPORT_FORMAT_RGB_INDEX];
                if(rga_info.output_format & IM_RGA_SUPPORT_FORMAT_RGB_OTHER)
                    out << output_format[IM_RGA_SUPPORT_FORMAT_RGB_OTHER_INDEX];
                if(rga_info.output_format & IM_RGA_SUPPORT_FORMAT_BPP)
                    out << output_format[IM_RGA_SUPPORT_FORMAT_BPP_INDEX];
                if(rga_info.output_format & IM_RGA_SUPPORT_FORMAT_YUV_420_SEMI_PLANNER_8_BIT)
                    out << output_format[IM_RGA_SUPPORT_FORMAT_YUV_420_SEMI_PLANNER_8_BIT_INDEX];
                if(rga_info.output_format & IM_RGA_SUPPORT_FORMAT_YUV_420_SEMI_PLANNER_10_BIT)
                    out << output_format[IM_RGA_SUPPORT_FORMAT_YUV_420_SEMI_PLANNER_10_BIT_INDEX];
                if(rga_info.output_format & IM_RGA_SUPPORT_FORMAT_YUV_420_PLANNER_8_BIT)
                    out << output_format[IM_RGA_SUPPORT_FORMAT_YUV_420_PLANNER_8_BIT_INDEX];
                if(rga_info.output_format & IM_RGA_SUPPORT_FORMAT_YUV_420_PLANNER_10_BIT)
                    out << output_format[IM_RGA_SUPPORT_FORMAT_YUV_420_PLANNER_10_BIT_INDEX];
                if(rga_info.output_format & IM_RGA_SUPPORT_FORMAT_YUV_422_SEMI_PLANNER_8_BIT)
                    out << output_format[IM_RGA_SUPPORT_FORMAT_YUV_422_SEMI_PLANNER_8_BIT_INDEX];
                if(rga_info.output_format & IM_RGA_SUPPORT_FORMAT_YUV_422_SEMI_PLANNER_10_BIT)
                    out << output_format[IM_RGA_SUPPORT_FORMAT_YUV_422_SEMI_PLANNER_10_BIT_INDEX];
                if(rga_info.output_format & IM_RGA_SUPPORT_FORMAT_YUV_422_PLANNER_8_BIT)
                    out << output_format[IM_RGA_SUPPORT_FORMAT_YUV_422_PLANNER_8_BIT_INDEX];
                if(rga_info.output_format & IM_RGA_SUPPORT_FORMAT_YUV_422_PLANNER_10_BIT)
                    out << output_format[IM_RGA_SUPPORT_FORMAT_YUV_422_PLANNER_10_BIT_INDEX];
                if(rga_info.output_format & IM_RGA_SUPPORT_FORMAT_YUYV_420)
                    out << output_format[IM_RGA_SUPPORT_FORMAT_YUYV_420_INDEX];
                if(rga_info.output_format & IM_RGA_SUPPORT_FORMAT_YUYV_422)
                    out << output_format[IM_RGA_SUPPORT_FORMAT_YUYV_422_INDEX];
                if(rga_info.output_format & IM_RGA_SUPPORT_FORMAT_YUV_400)
                    out << output_format[IM_RGA_SUPPORT_FORMAT_YUV_400_INDEX];
                if(!(rga_info.output_format & IM_RGA_SUPPORT_FORMAT_MASK))
                    out << output_format[IM_RGA_SUPPORT_FORMAT_ERROR_INDEX];
                out << endl;
                break;

            case RGA_FEATURE :
                out << output_name[name];
                if(rga_info.feature & IM_RGA_SUPPORT_FEATURE_COLOR_FILL)
                    out << feature[IM_RGA_SUPPORT_FEATURE_COLOR_FILL_INDEX];
                if(rga_info.feature & IM_RGA_SUPPORT_FEATURE_COLOR_PALETTE)
                    out << feature[IM_RGA_SUPPORT_FEATURE_COLOR_PALETTE_INDEX];
                if(rga_info.feature & IM_RGA_SUPPORT_FEATURE_ROP)
                    out << feature[IM_RGA_SUPPORT_FEATURE_ROP_INDEX];
                if(rga_info.feature & IM_RGA_SUPPORT_FEATURE_QUANTIZE)
                    out << feature[IM_RGA_SUPPORT_FEATURE_QUANTIZE_INDEX];
                if(rga_info.feature & IM_RGA_SUPPORT_FEATURE_SRC1_R2Y_CSC)
                    out << feature[IM_RGA_SUPPORT_FEATURE_SRC1_R2Y_CSC_INDEX];
                if(rga_info.feature & IM_RGA_SUPPORT_FEATURE_DST_FULL_CSC)
                    out << feature[IM_RGA_SUPPORT_FEATURE_DST_FULL_CSC_INDEX];
                if(rga_info.feature & IM_RGA_SUPPORT_FEATURE_FBC)
                    out << feature[IM_RGA_SUPPORT_FEATURE_FBC_INDEX];
                if(rga_info.feature & IM_RGA_SUPPORT_FEATURE_BLEND_YUV)
                    out << feature[IM_RGA_SUPPORT_FEATURE_BLEND_YUV_INDEX];
                if(rga_info.feature & IM_RGA_SUPPORT_FEATURE_BT2020)
                    out << feature[IM_RGA_SUPPORT_FEATURE_BT2020_INDEX];
                out << endl;
                break;

            case RGA_EXPECTED :
                switch(rga_info.performance) {
                    case 1 :
                        out << output_name[name] << performance[1] << endl;
                        break;
                    case 2 :
                        out << output_name[name] << performance[2] << endl;
                        break;
                    case 4 :
                        out << output_name[name] << performance[3] << endl;
                        break;
                    default :
                        out << output_name[name] << performance[IM_RGA_HW_VERSION_RGA_V_ERR_INDEX] << endl;
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

IM_API IM_STATUS rga_check_info(const char *name, const rga_buffer_t info, const im_rect rect, int resolution_usage) {
    /**************** src/dst judgment ****************/
    if (info.width <= 0 || info.height <= 0 || info.format < 0) {
        imSetErrorMsg("Illegal %s, the parameter cannot be negative or 0, width = %d, height = %d, format = 0x%x(%s)",
                      name, info.width, info.height, info.format, translate_format_str(info.format));
        return IM_STATUS_ILLEGAL_PARAM;
    }

    if (info.width < 2 || info.height < 2) {
        imSetErrorMsg("Hardware limitation %s, unsupported operation of images smaller than 2 pixels, "
                      "width = %d, height = %d",
                      name, info.width, info.height);
        return IM_STATUS_ILLEGAL_PARAM;
    }

    if (info.wstride < info.width || info.hstride < info.height) {
        imSetErrorMsg("Invaild %s, Virtual width or height is less than actual width and height, "
                      "wstride = %d, width = %d, hstride = %d, height = %d",
                      name, info.wstride, info.width, info.hstride, info.height);
        return IM_STATUS_INVALID_PARAM;
    }

    /**************** rect judgment ****************/
    if (rect.width < 0 || rect.height < 0 || rect.x < 0 || rect.y < 0) {
        imSetErrorMsg("Illegal %s rect, the parameter cannot be negative, rect[x,y,w,h] = [%d, %d, %d, %d]",
                      name, rect.x, rect.y, rect.width, rect.height);
        return IM_STATUS_ILLEGAL_PARAM;
    }

    if ((rect.width > 0  && rect.width < 2) || (rect.height > 0 && rect.height < 2) ||
        (rect.x > 0 && rect.x < 2)          || (rect.y > 0 && rect.y < 2)) {
        imSetErrorMsg("Hardware limitation %s rect, unsupported operation of images smaller than 2 pixels, "
                      "rect[x,y,w,h] = [%d, %d, %d, %d]",
                      name, rect.x, rect.y, rect.width, rect.height);
        return IM_STATUS_INVALID_PARAM;
    }

    if ((rect.width + rect.x > info.wstride) || (rect.height + rect.y > info.hstride)) {
        imSetErrorMsg("Invaild %s rect, the sum of width and height of rect needs to be less than wstride or hstride, "
                      "rect[x,y,w,h] = [%d, %d, %d, %d], wstride = %d, hstride = %d",
                      name, rect.x, rect.y, rect.width, rect.height, info.wstride, info.hstride);
        return IM_STATUS_INVALID_PARAM;
    }

    /**************** resolution check ****************/
    if (info.width > resolution_usage ||
        info.height > resolution_usage) {
        imSetErrorMsg("Unsupported %s to input resolution more than %d, width = %d, height = %d",
                      name, resolution_usage, info.width, info.height);
        return IM_STATUS_NOT_SUPPORTED;
    } else if ((rect.width > 0 && rect.width > resolution_usage) ||
               (rect.height > 0 && rect.height > resolution_usage)) {
        imSetErrorMsg("Unsupported %s rect to output resolution more than %d, rect[x,y,w,h] = [%d, %d, %d, %d]",
                      name, resolution_usage, rect.x, rect.y, rect.width, rect.height);
        return IM_STATUS_NOT_SUPPORTED;
    }

    return IM_STATUS_NOERROR;
}

IM_API IM_STATUS rga_check_limit(rga_buffer_t src, rga_buffer_t dst, int scale_usage, int mode_usage) {
    int src_width = 0, src_height = 0;
    int dst_width = 0, dst_height = 0;

    src_width = src.width;
    src_height = src.height;

    if (mode_usage & IM_HAL_TRANSFORM_ROT_270 || mode_usage & IM_HAL_TRANSFORM_ROT_90) {
        dst_width = dst.height;
        dst_height = dst.width;
    } else {
        dst_width = dst.width;
        dst_height = dst.height;
    }
    if (((src_width >> (int)(log(scale_usage)/log(2))) > dst_width) ||
       ((src_height >> (int)(log(scale_usage)/log(2))) > dst_height)) {
        imSetErrorMsg("Unsupported to scaling less than 1/%d ~ %d times, src[w,h] = [%d, %d], dst[w,h] = [%d, %d]",
                      scale_usage, scale_usage, src.width, src.height, dst.width, dst.height);
        return IM_STATUS_NOT_SUPPORTED;
    }
    if (((dst_width >> (int)(log(scale_usage)/log(2))) > src_width) ||
       ((dst_height >> (int)(log(scale_usage)/log(2))) > src_height)) {
        imSetErrorMsg("Unsupported to scaling more than 1/%d ~ %d times, src[w,h] = [%d, %d], dst[w,h] = [%d, %d]",
                      scale_usage, scale_usage, src.width, src.height, dst.width, dst.height);
        return IM_STATUS_NOT_SUPPORTED;
    }

    return IM_STATUS_NOERROR;
}

IM_API IM_STATUS rga_check_format(const char *name, rga_buffer_t info, im_rect rect, int format_usage, int mode_usgae) {
    IM_STATUS ret;
    int format = -1;

    format = RkRgaGetRgaFormat(RkRgaCompatibleFormat(info.format));
    if (-1 == format) {
        imSetErrorMsg("illegal %s format, please query and fix, format = 0x%x(%s)\n%s",
                      name, info.format, translate_format_str(info.format),
                      querystring((strcmp("dst", name) == 0) ? RGA_OUTPUT_FORMAT : RGA_INPUT_FORMAT));
        return IM_STATUS_NOT_SUPPORTED;
    }

    if (format == RK_FORMAT_RGBA_8888 || format == RK_FORMAT_BGRA_8888 ||
        format == RK_FORMAT_RGBX_8888 || format == RK_FORMAT_BGRX_8888 ||
        format == RK_FORMAT_ARGB_8888 || format == RK_FORMAT_ABGR_8888 ||
        format == RK_FORMAT_XRGB_8888 || format == RK_FORMAT_XBGR_8888 ||
        format == RK_FORMAT_RGB_888   || format == RK_FORMAT_BGR_888   ||
        format == RK_FORMAT_RGB_565   || format == RK_FORMAT_BGR_565) {
        if (~format_usage & IM_RGA_SUPPORT_FORMAT_RGB) {
            imSetErrorMsg("%s unsupported RGB format, format = 0x%x(%s)\n%s",
                          name, info.format, translate_format_str(info.format),
                          querystring((strcmp("dst", name) == 0) ? RGA_OUTPUT_FORMAT : RGA_INPUT_FORMAT));
            return IM_STATUS_NOT_SUPPORTED;
        }
    } else if (format == RK_FORMAT_RGBA_4444 || format == RK_FORMAT_BGRA_4444 ||
               format == RK_FORMAT_RGBA_5551 || format == RK_FORMAT_BGRA_5551 ||
               format == RK_FORMAT_ARGB_4444 || format == RK_FORMAT_ABGR_4444 ||
               format == RK_FORMAT_ARGB_5551 || format == RK_FORMAT_ABGR_5551) {
        if (~format_usage & IM_RGA_SUPPORT_FORMAT_RGB_OTHER) {
            imSetErrorMsg("%s unsupported RGBA 4444/5551 format, format = 0x%x(%s)\n%s",
                          name, info.format, translate_format_str(info.format),
                          querystring((strcmp("dst", name) == 0) ? RGA_OUTPUT_FORMAT : RGA_INPUT_FORMAT));
            return IM_STATUS_NOT_SUPPORTED;
        }
    } else if (format == RK_FORMAT_BPP1 || format == RK_FORMAT_BPP2 ||
               format == RK_FORMAT_BPP4 || format == RK_FORMAT_BPP8) {
        if ((~format_usage & IM_RGA_SUPPORT_FORMAT_BPP) && !(mode_usgae & IM_COLOR_PALETTE)) {
            imSetErrorMsg("%s unsupported BPP format, format = 0x%x(%s)\n%s",
                          name, info.format, translate_format_str(info.format),
                          querystring((strcmp("dst", name) == 0) ? RGA_OUTPUT_FORMAT : RGA_INPUT_FORMAT));
            return IM_STATUS_NOT_SUPPORTED;
        }
    } else if (format == RK_FORMAT_YCrCb_420_SP || format == RK_FORMAT_YCbCr_420_SP) {
        if (~format_usage & IM_RGA_SUPPORT_FORMAT_YUV_420_SEMI_PLANNER_8_BIT) {
            imSetErrorMsg("%s unsupported YUV420 semi-planner 8bit format, format = 0x%x(%s)\n%s",
                          name, info.format, translate_format_str(info.format),
                          querystring((strcmp("dst", name) == 0) ? RGA_OUTPUT_FORMAT : RGA_INPUT_FORMAT));
            return IM_STATUS_NOT_SUPPORTED;
        }

        ret = rga_align_check_yuv_8(name, info, rect);
        if (ret != IM_STATUS_SUCCESS)
            return ret;
    } else if (format == RK_FORMAT_YCrCb_420_P  || format == RK_FORMAT_YCbCr_420_P) {
        if (~format_usage & IM_RGA_SUPPORT_FORMAT_YUV_420_PLANNER_8_BIT) {
            imSetErrorMsg("%s unsupported YUV420 planner 8bit format, format = 0x%x(%s)\n%s",
                          name, info.format, translate_format_str(info.format),
                          querystring((strcmp("dst", name) == 0) ? RGA_OUTPUT_FORMAT : RGA_INPUT_FORMAT));
            return IM_STATUS_NOT_SUPPORTED;
        }

        ret = rga_align_check_yuv_8(name, info, rect);
        if (ret != IM_STATUS_SUCCESS)
            return ret;
    } else if (format == RK_FORMAT_YCrCb_422_SP || format == RK_FORMAT_YCbCr_422_SP) {
        if (~format_usage & IM_RGA_SUPPORT_FORMAT_YUV_422_SEMI_PLANNER_8_BIT) {
            imSetErrorMsg("%s unsupported YUV422 semi-planner 8bit format, format = 0x%x(%s)\n%s",
                          name, info.format, translate_format_str(info.format),
                          querystring((strcmp("dst", name) == 0) ? RGA_OUTPUT_FORMAT : RGA_INPUT_FORMAT));
            return IM_STATUS_NOT_SUPPORTED;
        }

        ret = rga_align_check_yuv_8(name, info, rect);
        if (ret != IM_STATUS_SUCCESS)
            return ret;
    } else if (format == RK_FORMAT_YCrCb_422_P  || format == RK_FORMAT_YCbCr_422_P) {
        if (~format_usage & IM_RGA_SUPPORT_FORMAT_YUV_422_PLANNER_8_BIT) {
            imSetErrorMsg("%s unsupported YUV422 planner 8bit format, format = 0x%x(%s)\n%s",
                          name, info.format, translate_format_str(info.format),
                          querystring((strcmp("dst", name) == 0) ? RGA_OUTPUT_FORMAT : RGA_INPUT_FORMAT));
            return IM_STATUS_NOT_SUPPORTED;
        }

        ret = rga_align_check_yuv_8(name, info, rect);
        if (ret != IM_STATUS_SUCCESS)
            return ret;
    } else if (format == RK_FORMAT_YCrCb_420_SP_10B || format == RK_FORMAT_YCbCr_420_SP_10B) {
        if (~format_usage & IM_RGA_SUPPORT_FORMAT_YUV_420_SEMI_PLANNER_10_BIT) {
            imSetErrorMsg("%s unsupported YUV420 semi-planner 10bit format, format = 0x%x(%s)\n%s",
                          name, info.format, translate_format_str(info.format),
                          querystring((strcmp("dst", name) == 0) ? RGA_OUTPUT_FORMAT : RGA_INPUT_FORMAT));
            return IM_STATUS_NOT_SUPPORTED;
        }

        ret = rga_align_check_yuv_10(name, info, rect);
        if (ret != IM_STATUS_SUCCESS)
            return ret;
        ALOGE("If it is an RK encoder output, it needs to be aligned with an odd multiple of 256.\n");
    } else if (format == RK_FORMAT_YUYV_420 || format == RK_FORMAT_YVYU_420 ||
               format == RK_FORMAT_UYVY_420 || format == RK_FORMAT_VYUY_420) {
        if (~format_usage & IM_RGA_SUPPORT_FORMAT_YUYV_420) {
            imSetErrorMsg("%s unsupported YUYV format, format = 0x%x(%s)\n%s",
                          name, info.format, translate_format_str(info.format),
                          querystring((strcmp("dst", name) == 0) ? RGA_OUTPUT_FORMAT : RGA_INPUT_FORMAT));
            return IM_STATUS_NOT_SUPPORTED;
        }

        ret = rga_align_check_yuv_8(name, info, rect);
        if (ret != IM_STATUS_SUCCESS)
            return ret;
    } else if (format == RK_FORMAT_YUYV_422 || format == RK_FORMAT_YVYU_422 ||
               format == RK_FORMAT_UYVY_422 || format == RK_FORMAT_VYUY_422) {
        if (~format_usage & IM_RGA_SUPPORT_FORMAT_YUYV_422) {
            imSetErrorMsg("%s unsupported YUYV format, format = 0x%x(%s)\n%s",
                          name, info.format, translate_format_str(info.format),
                          querystring((strcmp("dst", name) == 0) ? RGA_OUTPUT_FORMAT : RGA_INPUT_FORMAT));
            return IM_STATUS_NOT_SUPPORTED;
        }

        ret = rga_align_check_yuv_8(name, info, rect);
        if (ret != IM_STATUS_SUCCESS)
            return ret;
    } else if (format == RK_FORMAT_YCbCr_400) {
        if (~format_usage & IM_RGA_SUPPORT_FORMAT_YUV_400) {
            imSetErrorMsg("%s unsupported YUV400 format, format = 0x%x(%s)\n%s",
                          name, info.format, translate_format_str(info.format),
                          querystring((strcmp("dst", name) == 0) ? RGA_OUTPUT_FORMAT : RGA_INPUT_FORMAT));
            return IM_STATUS_NOT_SUPPORTED;
        }

        ret = rga_align_check_yuv_8(name, info, rect);
        if (ret != IM_STATUS_SUCCESS)
            return ret;
    } else if (format == RK_FORMAT_Y4) {
        if (~format_usage & IM_RGA_SUPPORT_FORMAT_Y4) {
            imSetErrorMsg("%s unsupported Y4/Y1 format, format = 0x%x(%s)\n%s",
                          name, info.format, translate_format_str(info.format),
                          querystring((strcmp("dst", name) == 0) ? RGA_OUTPUT_FORMAT : RGA_INPUT_FORMAT));
            return IM_STATUS_NOT_SUPPORTED;
        }

        ret = rga_align_check_yuv_8(name, info, rect);
        if (ret != IM_STATUS_SUCCESS)
            return ret;
    } else {
        imSetErrorMsg("%s unsupported this format, format = 0x%x(%s)\n%s",
                      name, info.format, translate_format_str(info.format),
                      querystring((strcmp("dst", name) == 0) ? RGA_OUTPUT_FORMAT : RGA_INPUT_FORMAT));
        return IM_STATUS_NOT_SUPPORTED;
    }

    return IM_STATUS_NOERROR;
}

IM_API IM_STATUS rga_check_blend(rga_buffer_t src, rga_buffer_t pat, rga_buffer_t dst, int pat_enable, int mode_usage) {
    int src_fmt, pat_fmt, dst_fmt;
    bool src_isRGB, pat_isRGB, dst_isRGB;

    src_fmt = RkRgaGetRgaFormat(RkRgaCompatibleFormat(src.format));
    pat_fmt = RkRgaGetRgaFormat(RkRgaCompatibleFormat(pat.format));
    dst_fmt = RkRgaGetRgaFormat(RkRgaCompatibleFormat(dst.format));

    src_isRGB = NormalRgaIsRgbFormat(src_fmt);
    pat_isRGB = NormalRgaIsRgbFormat(pat_fmt);
    dst_isRGB = NormalRgaIsRgbFormat(dst_fmt);

    /**************** blend mode check ****************/
    switch (mode_usage & IM_ALPHA_BLEND_MASK) {
        case IM_ALPHA_BLEND_SRC :
        case IM_ALPHA_BLEND_DST :
            break;
        case IM_ALPHA_BLEND_SRC_OVER :
            if (!NormalRgaFormatHasAlpha(src_fmt)) {
                imSetErrorMsg("Blend mode 'src_over' unsupported src format without alpha, "
                              "format[src,src1,dst] = [0x%x(%s), 0x%x(%s), 0x%x(%s)]",
                              src_fmt, translate_format_str(src_fmt),
                              pat_fmt, translate_format_str(pat_fmt),
                              dst_fmt, translate_format_str(dst_fmt));
                return IM_STATUS_NOT_SUPPORTED;
            }
            break;
        case IM_ALPHA_BLEND_DST_OVER :
            if (pat_enable) {
                if (!NormalRgaFormatHasAlpha(pat_fmt)) {
                    imSetErrorMsg("Blend mode 'dst_over' unsupported pat format without alpha, "
                                  "format[src,src1,dst] = [0x%x(%s), 0x%x(%s), 0x%x(%s)]",
                                  src_fmt, translate_format_str(src_fmt),
                                  pat_fmt, translate_format_str(pat_fmt),
                                  dst_fmt, translate_format_str(dst_fmt));
                    return IM_STATUS_NOT_SUPPORTED;
                }
            } else {
                if (!NormalRgaFormatHasAlpha(dst_fmt)) {
                    imSetErrorMsg("Blend mode 'dst_over' unsupported dst format without alpha, "
                                  "format[src,src1,dst] = [0x%x(%s), 0x%x(%s), 0x%x(%s)]",
                                  src_fmt, translate_format_str(src_fmt),
                                  pat_fmt, translate_format_str(pat_fmt),
                                  dst_fmt, translate_format_str(dst_fmt));
                    return IM_STATUS_NOT_SUPPORTED;
                }
            }
            break;
        default :
            if (!(NormalRgaFormatHasAlpha(src_fmt) || NormalRgaFormatHasAlpha(dst_fmt))) {
                imSetErrorMsg("Blend mode unsupported format without alpha, "
                              "format[src,src1,dst] = [0x%x(%s), 0x%x(%s), 0x%x(%s)]",
                              src_fmt, translate_format_str(src_fmt),
                              pat_fmt, translate_format_str(pat_fmt),
                              dst_fmt, translate_format_str(dst_fmt));
                return IM_STATUS_NOT_SUPPORTED;
            }
            break;
    }

    /* src1 don't support scale, and src1's size must aqual to dst.' */
    if (pat_enable && (pat.width != dst.width || pat.height != dst.height)) {
        imSetErrorMsg("In the three-channel mode Alapha blend, the width and height of the src1 channel "
                      "must be equal to the dst channel, src1[w,h] = [%d, %d], dst[w,h] = [%d, %d]",
                      pat.width, pat.height, dst.width, dst.height);
        return IM_STATUS_NOT_SUPPORTED;
    }

    return IM_STATUS_NOERROR;
}

IM_API IM_STATUS rga_check_rotate(int mode_usage, rga_info_table_entry &table) {
    if (table.version & (IM_RGA_HW_VERSION_RGA_1 | IM_RGA_HW_VERSION_RGA_1_PLUS)) {
        if (mode_usage & IM_HAL_TRANSFORM_FLIP_H_V) {
            imSetErrorMsg("RGA1/RGA1_PLUS cannot support H_V mirror.");
            return IM_STATUS_NOT_SUPPORTED;
        }

        if ((mode_usage & (IM_HAL_TRANSFORM_ROT_90 + IM_HAL_TRANSFORM_ROT_180 + IM_HAL_TRANSFORM_ROT_270)) &&
            (mode_usage & (IM_HAL_TRANSFORM_FLIP_H + IM_HAL_TRANSFORM_FLIP_V + IM_HAL_TRANSFORM_FLIP_H_V))) {
            imSetErrorMsg("RGA1/RGA1_PLUS cannot support rotate with mirror.");
            return IM_STATUS_NOT_SUPPORTED;
        }
    }

    return IM_STATUS_NOERROR;
}

IM_API IM_STATUS rga_check_feature(rga_buffer_t src, rga_buffer_t pat, rga_buffer_t dst,
                                   int pat_enable, int mode_usage, int feature_usage) {
    if (mode_usage == IM_COLOR_FILL && (~feature_usage & IM_RGA_SUPPORT_FEATURE_COLOR_FILL)) {
        imSetErrorMsg("The platform does not support color fill featrue. \n%s",
                      querystring(RGA_FEATURE));
        return IM_STATUS_NOT_SUPPORTED;
    }

    if (mode_usage == IM_COLOR_PALETTE && (~feature_usage & IM_RGA_SUPPORT_FEATURE_COLOR_PALETTE)) {
        imSetErrorMsg("The platform does not support color palette featrue. \n%s",
                      querystring(RGA_FEATURE));
        return IM_STATUS_NOT_SUPPORTED;
    }

    if (mode_usage == IM_ROP && (~feature_usage & IM_RGA_SUPPORT_FEATURE_ROP)) {
        imSetErrorMsg("The platform does not support ROP featrue. \n%s",
                      querystring(RGA_FEATURE));
        return IM_STATUS_NOT_SUPPORTED;
    }

    if (mode_usage == IM_NN_QUANTIZE && (~feature_usage & IM_RGA_SUPPORT_FEATURE_QUANTIZE)) {
        imSetErrorMsg("The platform does not support quantize featrue. \n%s",
                      querystring(RGA_FEATURE));
        return IM_STATUS_NOT_SUPPORTED;
    }

    if ((pat_enable ? (pat.color_space_mode & IM_RGB_TO_YUV_MASK) : 0) && (~feature_usage & IM_RGA_SUPPORT_FEATURE_SRC1_R2Y_CSC)) {
        imSetErrorMsg("The platform does not support src1 channel RGB2YUV color space convert featrue. \n%s",
                      querystring(RGA_FEATURE));
        return IM_STATUS_NOT_SUPPORTED;
    }

    if ((src.color_space_mode & IM_FULL_CSC_MASK ||
        dst.color_space_mode & IM_FULL_CSC_MASK ||
        (pat_enable ? (pat.color_space_mode & IM_FULL_CSC_MASK) : 0)) &&
        (~feature_usage & IM_RGA_SUPPORT_FEATURE_DST_FULL_CSC)) {
        imSetErrorMsg("The platform does not support dst channel full color space convert(Y2Y/Y2R) featrue. \n%s",
                      querystring(RGA_FEATURE));
        return IM_STATUS_NOT_SUPPORTED;
    }

    return IM_STATUS_NOERROR;
}

IM_API static IM_STATUS rga_check_driver(void) {
    int table_size, bind_index, least_index;
    bool user_bind = false;

    if (rgaCtx == NULL) {
        ALOGE("rga context is NULL!");
        imSetErrorMsg("rga context is NULL!");
        return IM_STATUS_FAILED;
    }

    /* The range of the version is [min, max), that is version >= min, version < max. */
    static const rga_dirver_bind_table_entry driver_bind_table[] = {
        { { 0, 0, 0, "0.0.0" }, {0, 0, 0, "0.0.0" } },
        { { 1, 0, 3, "1.0.3" }, {0, 0, 0, "0.0.0" } },
        { { 1, 6, 0, "1.6.0" }, {1, 1, 5, "1.1.5" } },
    };

    table_size = sizeof(driver_bind_table) / sizeof(rga_dirver_bind_table_entry);

    /* First, find the driver version corresponding to librga. */
    for (int i = (table_size - 1); i >= 0; i--) {
        if (rga_version_compare({ RGA_API_MAJOR_VERSION,
                                  RGA_API_MINOR_VERSION,
                                  RGA_API_REVISION_VERSION,
                                  RGA_API_VERSION },
                                driver_bind_table[i].user) >= 0) {
            if (i == (table_size - 1)) {
                user_bind = true;
                bind_index = i;

                break;
            } else if (rga_version_compare(driver_bind_table[i + 1].user,
                                           { RGA_API_MAJOR_VERSION,
                                             RGA_API_MINOR_VERSION,
                                             RGA_API_REVISION_VERSION,
                                             RGA_API_VERSION }) > 0) {
                user_bind = true;
                bind_index = i;

                break;
            }
        }
    }

    if (user_bind) {
        /* Second, check whether the current driver version matches. */
        if (rga_version_compare(rgaCtx->mDriverVersion, driver_bind_table[bind_index].driver) >= 0) {
            if (bind_index == table_size - 1) {
                return IM_STATUS_SUCCESS;
            } else if (rga_version_compare(driver_bind_table[bind_index + 1].driver, rgaCtx->mDriverVersion) > 0) {
                return IM_STATUS_SUCCESS;
            } else {
                /* find needs to be update version at least. */
                least_index = table_size - 1;
                for (int i = (table_size - 1); i >= 0; i--) {
                    if (rga_version_compare(rgaCtx->mDriverVersion, driver_bind_table[i].driver) >= 0) {
                        if (i == (table_size - 1)) {
                            least_index = i;
                            break;
                        } else if (rga_version_compare(driver_bind_table[i + 1].driver, rgaCtx->mDriverVersion) > 0) {
                            least_index = i;
                            break;
                        }
                    }
                }

                ALOGE("The librga needs to be updated to version %s at least. "
                      "current version: librga %s, driver %s.",
                      driver_bind_table[least_index].user.str,
                      RGA_API_VERSION, rgaCtx->mDriverVersion.str);
                imSetErrorMsg("The librga needs to be updated to version %s at least. "
                              "current version: librga %s, driver %s.",
                              driver_bind_table[least_index].user.str,
                              RGA_API_VERSION, rgaCtx->mDriverVersion.str);

                return IM_STATUS_ERROR_VERSION;
            }
        } else {
            ALOGE("The driver may be compatible, "
                  "but it is best to update the driver to version %s. "
                  "current version: librga %s, driver %s.",
                  driver_bind_table[bind_index].driver.str,
                  RGA_API_VERSION, rgaCtx->mDriverVersion.str);
            imSetErrorMsg("The driver may be compatible, "
                          "but it is best to update the driver to version %s. "
                          "current version: librga %s, driver %s.",
                          driver_bind_table[bind_index].driver.str,
                          RGA_API_VERSION, rgaCtx->mDriverVersion.str);

            /* Sometimes it is possible to enter compatibility mode. */
            return IM_STATUS_NOERROR;
        }
    } else {
        ALOGE("Failed to get the version binding table of librga, "
            "current version: librga: %s, driver: %s",
            RGA_API_VERSION, rgaCtx->mDriverVersion.str);
        imSetErrorMsg("Failed to get the version binding table of librga, "
                    "current version: librga: %s, driver: %s",
                    RGA_API_VERSION, rgaCtx->mDriverVersion.str);

        return IM_STATUS_ERROR_VERSION;
    }
}

IM_API IM_STATUS imcheck_t(const rga_buffer_t src, const rga_buffer_t dst, const rga_buffer_t pat,
                           const im_rect src_rect, const im_rect dst_rect, const im_rect pat_rect, int mode_usage) {
    bool pat_enable = 0;
    IM_STATUS ret = IM_STATUS_NOERROR;
    rga_info_table_entry rga_info;

    ret = rga_get_info(&rga_info);
    if (IM_STATUS_FAILED == ret) {
        ALOGE("rga im2d: rga2 get info failed!\n");
        return IM_STATUS_FAILED;
    }

    /* check driver version. */
    ret = rga_check_driver();
    if (ret == IM_STATUS_ERROR_VERSION)
        return ret;

    if (mode_usage & IM_ALPHA_BLEND_MASK) {
        if (rga_is_buffer_valid(pat))
            pat_enable = 1;
    }

    /**************** feature judgment ****************/
    ret = rga_check_feature(src, pat, dst, pat_enable, mode_usage, rga_info.feature);
    if (ret != IM_STATUS_NOERROR)
        return ret;

    /**************** info judgment ****************/
    if (~mode_usage & IM_COLOR_FILL) {
        ret = rga_check_info("src", src, src_rect, rga_info.input_resolution);
        if (ret != IM_STATUS_NOERROR)
            return ret;
        ret = rga_check_format("src", src, src_rect, rga_info.input_format, mode_usage);
        if (ret != IM_STATUS_NOERROR)
            return ret;
    }
    if (pat_enable) {
        /* RGA1 cannot support src1. */
        if (rga_info.version & (IM_RGA_HW_VERSION_RGA_1 | IM_RGA_HW_VERSION_RGA_1_PLUS)) {
            imSetErrorMsg("RGA1/RGA1_PLUS cannot support src1.");
            return IM_STATUS_NOT_SUPPORTED;
        }


        ret = rga_check_info("pat", pat, pat_rect, rga_info.input_resolution);
        if (ret != IM_STATUS_NOERROR)
            return ret;
        ret = rga_check_format("pat", pat, pat_rect, rga_info.input_format, mode_usage);
        if (ret != IM_STATUS_NOERROR)
            return ret;
    }
    ret = rga_check_info("dst", dst, dst_rect, rga_info.output_resolution);
    if (ret != IM_STATUS_NOERROR)
        return ret;
    ret = rga_check_format("dst", dst, dst_rect, rga_info.output_format, mode_usage);
    if (ret != IM_STATUS_NOERROR)
        return ret;

    if ((~mode_usage & IM_COLOR_FILL)) {
        ret = rga_check_limit(src, dst, rga_info.scale_limit, mode_usage);
        if (ret != IM_STATUS_NOERROR)
            return ret;
    }

    if (mode_usage & IM_ALPHA_BLEND_MASK) {
        ret = rga_check_blend(src, pat, dst, pat_enable, mode_usage);
        if (ret != IM_STATUS_NOERROR)
            return ret;
    }

    ret = rga_check_rotate(mode_usage, rga_info);
    if (ret != IM_STATUS_NOERROR)
        return ret;

    return IM_STATUS_NOERROR;
}

IM_API IM_STATUS imresize_t(const rga_buffer_t src, rga_buffer_t dst, double fx, double fy, int interpolation, int sync) {
    int usage = 0;
    int width = 0, height = 0;
    IM_STATUS ret = IM_STATUS_NOERROR;
    int out_fence_fd;

    im_opt_t opt;

    rga_buffer_t pat;

    im_rect srect;
    im_rect drect;
    im_rect prect;

    empty_structure(NULL, NULL, &pat, &srect, &drect, &prect, &opt);

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
        usage |= IM_ASYNC;
    else if (sync == 1)
        usage |= IM_SYNC;

    ret = improcess_t(src, dst, pat, srect, drect, prect, -1, &out_fence_fd, &opt, usage);

    return ret;
}

IM_API IM_STATUS imcrop_t(const rga_buffer_t src, rga_buffer_t dst, im_rect rect, int sync) {
    int usage = 0;
    IM_STATUS ret = IM_STATUS_NOERROR;

    int out_fence_fd;

    im_opt_t opt;


    rga_buffer_t pat;

    im_rect drect;
    im_rect prect;

    empty_structure(NULL, NULL, &pat, NULL, &drect, &prect, &opt);

    drect.width = rect.width;
    drect.height = rect.height;

    if (sync == 0)
        usage |= IM_ASYNC;
    else if (sync == 1)
        usage |= IM_SYNC;

    ret = improcess_t(src, dst, pat, rect, drect, prect, -1, &out_fence_fd, &opt, usage);

    return ret;
}

IM_API IM_STATUS imrotate_t(const rga_buffer_t src, rga_buffer_t dst, int rotation, int sync) {
    int usage = 0;
    IM_STATUS ret = IM_STATUS_NOERROR;
    int out_fence_fd;

    im_opt_t opt;


    rga_buffer_t pat;

    im_rect srect;
    im_rect drect;
    im_rect prect;

    empty_structure(NULL, NULL, &pat, &srect, &drect, &prect, &opt);

    usage |= rotation;

    if (sync == 0)
        usage |= IM_ASYNC;
    else if (sync == 1)
        usage |= IM_SYNC;

    ret = improcess_t(src, dst, pat, srect, drect, prect, -1, &out_fence_fd, &opt, usage);

    return ret;
}

IM_API IM_STATUS imflip_t (const rga_buffer_t src, rga_buffer_t dst, int mode, int sync) {
    int usage = 0;
    IM_STATUS ret = IM_STATUS_NOERROR;
    int out_fence_fd;

    im_opt_t opt;

    rga_buffer_t pat;

    im_rect srect;
    im_rect drect;
    im_rect prect;

    empty_structure(NULL, NULL, &pat, &srect, &drect, &prect, &opt);

    usage |= mode;

    if (sync == 0)
        usage |= IM_ASYNC;
    else if (sync == 1)
        usage |= IM_SYNC;

    ret = improcess_t(src, dst, pat, srect, drect, prect, -1, &out_fence_fd, &opt, usage);

    return ret;
}

IM_API IM_STATUS imfill_t(rga_buffer_t dst, im_rect rect, int color, int sync) {
    int usage = 0;
    IM_STATUS ret = IM_STATUS_NOERROR;
    int out_fence_fd;

    im_opt_t opt;

    rga_buffer_t pat;
    rga_buffer_t src;

    im_rect srect;
    im_rect prect;

    empty_structure(&src, NULL, &pat, &srect, NULL, &prect, &opt);

    memset(&src, 0, sizeof(rga_buffer_t));

    usage |= IM_COLOR_FILL;

    opt.color = color;

    if (sync == 0)
        usage |= IM_ASYNC;
    else if (sync == 1)
        usage |= IM_SYNC;

    ret = improcess_t(src, dst, pat, srect, rect, prect, -1, &out_fence_fd, &opt, usage);

    return ret;
}

IM_API IM_STATUS impalette_t(rga_buffer_t src, rga_buffer_t dst, rga_buffer_t lut, int sync) {
    int usage = 0;
    IM_STATUS ret = IM_STATUS_NOERROR;
    im_rect srect;
    im_rect drect;
    im_rect prect;

    int out_fence_fd;

    im_opt_t opt;

    empty_structure(NULL, NULL, NULL, &srect, &drect, &prect, &opt);

    /*Don't know if it supports zooming.*/
    if ((src.width != dst.width) || (src.height != dst.height))
        return IM_STATUS_INVALID_PARAM;

    usage |= IM_COLOR_PALETTE;

    if (sync == 0)
        usage |= IM_ASYNC;
    else if (sync == 1)
        usage |= IM_SYNC;

    ret = improcess_t(src, dst, lut, srect, drect, prect, -1, &out_fence_fd, &opt, usage);

    return ret;
}

IM_API IM_STATUS imtranslate_t(const rga_buffer_t src, rga_buffer_t dst, int x, int y, int sync) {
    int usage = 0;
    IM_STATUS ret = IM_STATUS_NOERROR;
    int out_fence_fd;

    im_opt_t opt;

    rga_buffer_t pat;

    im_rect srect;
    im_rect drect;
    im_rect prect;

    empty_structure(NULL, NULL, &pat, &srect, &drect, &prect, &opt);

    if ((src.width != dst.width) || (src.height != dst.height))
        return IM_STATUS_INVALID_PARAM;

    if (sync == 0)
        usage |= IM_ASYNC;
    else if (sync == 1)
        usage |= IM_SYNC;

    srect.width = src.width - x;
    srect.height = src.height - y;
    drect.x = x;
    drect.y = y;
    drect.width = src.width - x;
    drect.height = src.height - y;

    ret = improcess_t(src, dst, pat, srect, drect, prect, -1, &out_fence_fd, &opt, usage);

    return ret;
}

IM_API IM_STATUS imcopy_t(const rga_buffer_t src, rga_buffer_t dst, int sync) {
    int usage = 0;
    IM_STATUS ret = IM_STATUS_NOERROR;
    int out_fence_fd;

    im_opt_t opt;

    rga_buffer_t pat;

    im_rect srect;
    im_rect drect;
    im_rect prect;

    empty_structure(NULL, NULL, &pat, &srect, &drect, &prect, &opt);

    if ((src.width != dst.width) || (src.height != dst.height)) {
        imSetErrorMsg("imcopy cannot support scale, src[w,h] = [%d, %d], dst[w,h] = [%d, %d]",
                      src.width, src.height, dst.width, dst.height);
        return IM_STATUS_INVALID_PARAM;
    }

    if (sync == 0)
        usage |= IM_ASYNC;
    else if (sync == 1)
        usage |= IM_SYNC;

    ret = improcess_t(src, dst, pat, srect, drect, prect, -1, &out_fence_fd, &opt, usage);

    return ret;
}

IM_API IM_STATUS imcolorkey_t(const rga_buffer_t src, rga_buffer_t dst, im_colorkey_range range, int mode, int sync) {
    int usage = 0;
    IM_STATUS ret = IM_STATUS_NOERROR;
    int out_fence_fd;

    im_opt_t opt;

    rga_buffer_t pat;

    im_rect srect;
    im_rect drect;
    im_rect prect;

    empty_structure(NULL, NULL, &pat, &srect, &drect, &prect, &opt);

    usage |= mode;

    opt.colorkey_range = range;

    if (sync == 0)
        usage |= IM_ASYNC;
    else if (sync == 1)
        usage |= IM_SYNC;

    ret = improcess_t(src, dst, pat, srect, drect, prect, -1, &out_fence_fd, &opt, usage);

    return ret;
}

IM_API IM_STATUS imblend_t(const rga_buffer_t srcA, const rga_buffer_t srcB, rga_buffer_t dst, int mode, int sync) {
    int usage = 0;
    IM_STATUS ret = IM_STATUS_NOERROR;
    int out_fence_fd;

    im_opt_t opt;

    im_rect srect;
    im_rect drect;
    im_rect prect;

    empty_structure(NULL, NULL, NULL, &srect, &drect, &prect, &opt);

    usage |= mode;

    if (sync == 0)
        usage |= IM_ASYNC;
    else if (sync == 1)
        usage |= IM_SYNC;

    ret = improcess_t(srcA, dst, srcB, srect, drect, prect, -1, &out_fence_fd, &opt, usage);

    return ret;
}



IM_API IM_STATUS imcvtcolor_t(rga_buffer_t src, rga_buffer_t dst, int sfmt, int dfmt, int mode, int sync) {
    int usage = 0;
    IM_STATUS ret = IM_STATUS_NOERROR;
    int out_fence_fd;

    im_opt_t opt;

    rga_buffer_t pat;

    im_rect srect;
    im_rect drect;
    im_rect prect;

    empty_structure(NULL, NULL, &pat, &srect, &drect, &prect, &opt);

    src.format = sfmt;
    dst.format = dfmt;

    dst.color_space_mode = mode;

    if (sync == 0)
        usage |= IM_ASYNC;
    else if (sync == 1)
        usage |= IM_SYNC;

    ret = improcess_t(src, dst, pat, srect, drect, prect, -1, &out_fence_fd, &opt, usage);

    return ret;
}

IM_API IM_STATUS imquantize_t(const rga_buffer_t src, rga_buffer_t dst, im_nn_t nn_info, int sync) {
    int usage = 0;
    IM_STATUS ret = IM_STATUS_NOERROR;
    int out_fence_fd;

    im_opt_t opt;

    rga_buffer_t pat;

    im_rect srect;
    im_rect drect;
    im_rect prect;

    empty_structure(NULL, NULL, &pat, &srect, &drect, &prect, &opt);

    usage |= IM_NN_QUANTIZE;

    opt.nn = nn_info;

    if (sync == 0)
        usage |= IM_ASYNC;
    else if (sync == 1)
        usage |= IM_SYNC;

    ret = improcess_t(src, dst, pat, srect, drect, prect, -1, &out_fence_fd, &opt, usage);

    return ret;
}

IM_API IM_STATUS imrop_t(const rga_buffer_t src, rga_buffer_t dst, int rop_code, int sync) {
    int usage = 0;
    IM_STATUS ret = IM_STATUS_NOERROR;
    int out_fence_fd;

    im_opt_t opt;

    rga_buffer_t pat;

    im_rect srect;
    im_rect drect;
    im_rect prect;

    empty_structure(NULL, NULL, &pat, &srect, &drect, &prect, &opt);

    usage |= IM_ROP;

    opt.rop_code = rop_code;

    if (sync == 0)
        usage |= IM_ASYNC;
    else if (sync == 1)
        usage |= IM_SYNC;

    ret = improcess_t(src, dst, pat, srect, drect, prect, -1, &out_fence_fd, &opt, usage);

    return ret;
}

IM_API IM_STATUS improcess(rga_buffer_t src, rga_buffer_t dst, rga_buffer_t pat,
                        im_rect srect, im_rect drect, im_rect prect, int usage) {
    IM_STATUS ret = IM_STATUS_NOERROR;
    int out_fence_fd;
    im_opt_t opt;

    memset(&opt, 0, sizeof(opt));

    ret = improcess_t(src, dst, pat, srect, drect, prect, -1, &out_fence_fd, &opt, usage);

    return ret;
}

IM_API IM_STATUS improcess_t(rga_buffer_t src, rga_buffer_t dst, rga_buffer_t pat,
                        im_rect srect, im_rect drect, im_rect prect,
                        int in_fence_fd, int *out_fence_fd, im_opt_t *opt, int usage) {
    rga_info_t srcinfo;
    rga_info_t dstinfo;
    rga_info_t patinfo;
    int ret;

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

    rga_apply_rect(&src, &srect);
    rga_apply_rect(&dst, &drect);

    rga_set_rect(&srcinfo.rect, srect.x, srect.y, src.width, src.height, src.wstride, src.hstride, src.format);
    rga_set_rect(&dstinfo.rect, drect.x, drect.y, dst.width, dst.height, dst.wstride, dst.hstride, dst.format);

    if (((usage & IM_COLOR_PALETTE) || (usage & IM_ALPHA_BLEND_MASK)) &&
        rga_is_buffer_valid(pat)) {

        ret = rga_set_buffer_info(pat, &patinfo);
        if (ret <= 0)
            return (IM_STATUS)ret;

        rga_apply_rect(&pat, &prect);

        rga_set_rect(&patinfo.rect, prect.x, prect.y, pat.width, pat.height, pat.wstride, pat.hstride, pat.format);
    }

    if ((usage & IM_ALPHA_BLEND_MASK) && rga_is_buffer_valid(pat)) /* A+B->C */
        ret = imcheck_composite(src, dst, pat, srect, drect, prect, usage);
    else
        ret = imcheck(src, dst, srect, drect, usage);
    if(ret != IM_STATUS_NOERROR)
        return (IM_STATUS)ret;

    /* Transform */
    if (usage & IM_HAL_TRANSFORM_MASK) {
        switch (usage & (IM_HAL_TRANSFORM_ROT_90 + IM_HAL_TRANSFORM_ROT_180 + IM_HAL_TRANSFORM_ROT_270)) {
            case IM_HAL_TRANSFORM_ROT_90:
                srcinfo.rotation = HAL_TRANSFORM_ROT_90;
                break;
            case IM_HAL_TRANSFORM_ROT_180:
                srcinfo.rotation = HAL_TRANSFORM_ROT_180;
                break;
            case IM_HAL_TRANSFORM_ROT_270:
                srcinfo.rotation = HAL_TRANSFORM_ROT_270;
                break;
        }

        switch (usage & (IM_HAL_TRANSFORM_FLIP_V + IM_HAL_TRANSFORM_FLIP_H + IM_HAL_TRANSFORM_FLIP_H_V)) {
            case IM_HAL_TRANSFORM_FLIP_V:
                srcinfo.rotation |= srcinfo.rotation ?
                                    HAL_TRANSFORM_FLIP_V << 4 :
                                    HAL_TRANSFORM_FLIP_V;
                break;
            case IM_HAL_TRANSFORM_FLIP_H:
                srcinfo.rotation |= srcinfo.rotation ?
                                    HAL_TRANSFORM_FLIP_H << 4 :
                                    HAL_TRANSFORM_FLIP_H;
                break;
            case IM_HAL_TRANSFORM_FLIP_H_V:
                srcinfo.rotation |= srcinfo.rotation ?
                                    HAL_TRANSFORM_FLIP_H_V << 4 :
                                    HAL_TRANSFORM_FLIP_H_V;
                break;
        }

        if(srcinfo.rotation ==0)
            ALOGE("rga_im2d: Could not find rotate/flip usage : 0x%x \n", usage);
    }

    /* Blend */
    if (usage & IM_ALPHA_BLEND_MASK) {
        switch(usage & IM_ALPHA_BLEND_MASK) {
            case IM_ALPHA_BLEND_SRC:
                srcinfo.blend = 0x1;
                break;
            case IM_ALPHA_BLEND_DST:
                srcinfo.blend = 0x2;
                break;
            case IM_ALPHA_BLEND_SRC_OVER:
                srcinfo.blend = (usage & IM_ALPHA_BLEND_PRE_MUL) ? 0x405 : 0x105;
                break;
            case IM_ALPHA_BLEND_SRC_IN:
                break;
            case IM_ALPHA_BLEND_DST_IN:
                break;
            case IM_ALPHA_BLEND_SRC_OUT:
                break;
            case IM_ALPHA_BLEND_DST_OVER:
                srcinfo.blend = (usage & IM_ALPHA_BLEND_PRE_MUL) ? 0x504 : 0x501;
                break;
            case IM_ALPHA_BLEND_SRC_ATOP:
                break;
            case IM_ALPHA_BLEND_DST_OUT:
                break;
            case IM_ALPHA_BLEND_XOR:
                break;
        }

        if(srcinfo.blend == 0)
            ALOGE("rga_im2d: Could not find blend usage : 0x%x \n", usage);

        /* set global alpha */
        if (src.global_alpha > 0)
            srcinfo.blend ^= src.global_alpha << 16;
        else {
            srcinfo.blend ^= 0xFF << 16;
        }
    }

    /* color key */
    if (usage & IM_ALPHA_COLORKEY_MASK) {
        srcinfo.blend = 0xff0105;

        srcinfo.colorkey_en = 1;
        srcinfo.colorkey_min = opt->colorkey_range.min;
        srcinfo.colorkey_max = opt->colorkey_range.max;
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
        dstinfo.nn.scale_r  = opt->nn.scale_r;
        dstinfo.nn.scale_g  = opt->nn.scale_g;
        dstinfo.nn.scale_b  = opt->nn.scale_b;
        dstinfo.nn.offset_r = opt->nn.offset_r;
        dstinfo.nn.offset_g = opt->nn.offset_g;
        dstinfo.nn.offset_b = opt->nn.offset_b;
    }

    /* set ROP */
    if (usage & IM_ROP) {
        srcinfo.rop_code = opt->rop_code;
    }

    /* special config for color space convert */
    if ((dst.color_space_mode & IM_YUV_TO_RGB_MASK) && (dst.color_space_mode & IM_RGB_TO_YUV_MASK)) {
        if (rga_is_buffer_valid(pat) &&
            NormalRgaIsYuvFormat(RkRgaGetRgaFormat(src.format)) &&
            NormalRgaIsRgbFormat(RkRgaGetRgaFormat(pat.format)) &&
            NormalRgaIsYuvFormat(RkRgaGetRgaFormat(dst.format))) {
            dstinfo.color_space_mode = dst.color_space_mode;
        } else {
            imSetErrorMsg("Not yuv + rgb -> yuv does not need for color_sapce_mode R2Y & Y2R, please fix, "
                          "src_fromat = 0x%x(%s), src1_format = 0x%x(%s), dst_format = 0x%x(%s)",
                          src.format, translate_format_str(src.format),
                          pat.format, translate_format_str(pat.format),
                          dst.format, translate_format_str(dst.format));
            return IM_STATUS_ILLEGAL_PARAM;
        }
    } else if (dst.color_space_mode & (IM_YUV_TO_RGB_MASK)) {
        if (rga_is_buffer_valid(pat) &&
            NormalRgaIsYuvFormat(RkRgaGetRgaFormat(src.format)) &&
            NormalRgaIsRgbFormat(RkRgaGetRgaFormat(pat.format)) &&
            NormalRgaIsRgbFormat(RkRgaGetRgaFormat(dst.format))) {
            dstinfo.color_space_mode = dst.color_space_mode;
        } else if (NormalRgaIsYuvFormat(RkRgaGetRgaFormat(src.format)) &&
                   NormalRgaIsRgbFormat(RkRgaGetRgaFormat(dst.format))) {
            dstinfo.color_space_mode = dst.color_space_mode;
        } else {
            imSetErrorMsg("Not yuv to rgb does not need for color_sapce_mode, please fix, "
                          "src_fromat = 0x%x(%s), src1_format = 0x%x(%s), dst_format = 0x%x(%s)",
                          src.format, translate_format_str(src.format),
                          pat.format, rga_is_buffer_valid(pat) ? translate_format_str(pat.format) : "none",
                          dst.format, translate_format_str(dst.format));
            return IM_STATUS_ILLEGAL_PARAM;
        }
    } else if (dst.color_space_mode & (IM_RGB_TO_YUV_MASK)) {
        if (rga_is_buffer_valid(pat) &&
            NormalRgaIsRgbFormat(RkRgaGetRgaFormat(src.format)) &&
            NormalRgaIsRgbFormat(RkRgaGetRgaFormat(pat.format)) &&
            NormalRgaIsYuvFormat(RkRgaGetRgaFormat(dst.format))) {
            dstinfo.color_space_mode = dst.color_space_mode;
        } else if (NormalRgaIsRgbFormat(RkRgaGetRgaFormat(src.format)) &&
                   NormalRgaIsYuvFormat(RkRgaGetRgaFormat(dst.format))) {
            dstinfo.color_space_mode = dst.color_space_mode;
        } else {
            imSetErrorMsg("Not rgb to yuv does not need for color_sapce_mode, please fix, "
                          "src_fromat = 0x%x(%s), src1_format = 0x%x(%s), dst_format = 0x%x(%s)",
                          src.format, translate_format_str(src.format),
                          pat.format, rga_is_buffer_valid(pat) ? translate_format_str(pat.format) : "none",
                          dst.format, translate_format_str(dst.format));
            return IM_STATUS_ILLEGAL_PARAM;
        }
    } else if (src.color_space_mode & IM_FULL_CSC_MASK ||
               dst.color_space_mode & IM_FULL_CSC_MASK) {
        /* Get default color space */
        if (src.color_space_mode == IM_COLOR_SPACE_DEFAULT) {
            if  (NormalRgaIsRgbFormat(RkRgaGetRgaFormat(src.format))) {
                src.color_space_mode = IM_RGB_FULL;
            } else if (NormalRgaIsYuvFormat(RkRgaGetRgaFormat(src.format))) {
                src.color_space_mode = IM_YUV_BT601_LIMIT_RANGE;
            }
        }

        if (dst.color_space_mode == IM_COLOR_SPACE_DEFAULT) {
            if  (NormalRgaIsRgbFormat(RkRgaGetRgaFormat(dst.format))) {
                src.color_space_mode = IM_RGB_FULL;
            } else if (NormalRgaIsYuvFormat(RkRgaGetRgaFormat(dst.format))) {
                src.color_space_mode = IM_YUV_BT601_LIMIT_RANGE;
            }
        }

        if (src.color_space_mode == IM_RGB_FULL &&
            dst.color_space_mode == IM_YUV_BT709_FULL_RANGE) {
            dstinfo.color_space_mode = rgb2yuv_709_full;
        } else if (src.color_space_mode == IM_YUV_BT601_FULL_RANGE &&
                   dst.color_space_mode == IM_YUV_BT709_LIMIT_RANGE) {
            dstinfo.color_space_mode = yuv2yuv_601_full_2_709_limit;
        } else if (src.color_space_mode == IM_YUV_BT709_LIMIT_RANGE &&
                   dst.color_space_mode == IM_YUV_BT601_LIMIT_RANGE) {
            dstinfo.color_space_mode = yuv2yuv_709_limit_2_601_limit;
        } else if (src.color_space_mode == IM_YUV_BT709_FULL_RANGE &&
                   dst.color_space_mode == IM_YUV_BT601_LIMIT_RANGE) {
            dstinfo.color_space_mode = yuv2yuv_709_full_2_601_limit;
        } else if (src.color_space_mode == IM_YUV_BT709_FULL_RANGE &&
                   dst.color_space_mode == IM_YUV_BT601_FULL_RANGE) {
            dstinfo.color_space_mode = yuv2yuv_709_full_2_601_full;
        } else {
            imSetErrorMsg("Unsupported full csc mode! src_csm = 0x%x, dst_csm = 0x%x",
                          src.color_space_mode, dst.color_space_mode);
            return IM_STATUS_NOT_SUPPORTED;
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

	srcinfo.rd_mode = src.rd_mode;
	dstinfo.rd_mode = dst.rd_mode;
	if (rga_is_buffer_valid(pat))
		patinfo.rd_mode = pat.rd_mode;

    RockchipRga& rkRga(RockchipRga::get());

    if (usage & IM_ASYNC)
        dstinfo.sync_mode = RGA_BLIT_ASYNC;
    else if (usage & IM_SYNC)
        dstinfo.sync_mode = RGA_BLIT_SYNC;

    dstinfo.in_fence_fd = in_fence_fd;
    dstinfo.core = opt->core ? opt->core : g_im2d_context.core;
    dstinfo.priority = opt->priority ? opt->priority : g_im2d_context.priority;

    if (usage & IM_COLOR_FILL) {
        dstinfo.color = opt->color;
        ret = rkRga.RkRgaCollorFill(&dstinfo);
    } else if (usage & IM_COLOR_PALETTE) {
        ret = rkRga.RkRgaCollorPalette(&srcinfo, &dstinfo, &patinfo);
    } else if ((usage & IM_ALPHA_BLEND_MASK) && rga_is_buffer_valid(pat)) {
        ret = rkRga.RkRgaBlit(&srcinfo, &dstinfo, &patinfo);
    }else {
        ret = rkRga.RkRgaBlit(&srcinfo, &dstinfo, NULL);
    }

    if (ret) {
        imSetErrorMsg("Failed to call RockChipRga interface, query log to find the cause of failure.");
        ALOGE("srect[x,y,w,h] = [%d, %d, %d, %d] src[w,h,ws,hs] = [%d, %d, %d, %d]\n",
               srect.x, srect.y, srect.width, srect.height, src.width, src.height, src.wstride, src.hstride);
        if (rga_is_buffer_valid(pat))
           ALOGE("s1/prect[x,y,w,h] = [%d, %d, %d, %d] src1/pat[w,h,ws,hs] = [%d, %d, %d, %d]\n",
               prect.x, prect.y, prect.width, prect.height, pat.width, pat.height, pat.wstride, pat.hstride);
        ALOGE("drect[x,y,w,h] = [%d, %d, %d, %d] dst[w,h,ws,hs] = [%d, %d, %d, %d]\n",
               drect.x, drect.y, drect.width, drect.height, dst.width, dst.height, dst.wstride, dst.hstride);
        ALOGE("usage[0x%x]", usage);
        return IM_STATUS_FAILED;
    }

    *out_fence_fd = dstinfo.out_fence_fd;

    return IM_STATUS_SUCCESS;
}

IM_API IM_STATUS imsync(int fence_fd) {
    int ret = 0;

    ret = rga_sync_wait(fence_fd, -1);
    if (ret) {
        ALOGE("Failed to wait for out fence = %d, ret = %d", fence_fd, ret);
        return IM_STATUS_FAILED;
    }

    close(fence_fd);

    return IM_STATUS_SUCCESS;
}

IM_API IM_STATUS imconfig(IM_CONFIG_NAME name, uint64_t value) {

    switch (name) {
        case IM_CONFIG_SCHEDULER_CORE :
            if (value & IM_SCHEDULER_MASK) {
                g_im2d_context.core = (IM_SCHEDULER_CORE)value;
            } else {
                ALOGE("IM2D: It's not legal rga_core, it needs to be a 'IM_SCHEDULER_CORE'.");
                return IM_STATUS_ILLEGAL_PARAM;
            }
            break;
        case IM_CONFIG_PRIORITY :
            if (value >= 0 && value <= 6) {
                g_im2d_context.priority = (int)value;
            } else {
                ALOGE("IM2D: It's not legal priority, it needs to be a 'int', and it should be in the range of 0~6.");
                return IM_STATUS_ILLEGAL_PARAM;
            }
            break;
        case IM_CHECK_CONFIG :
            if (value == false || value == true) {
                g_im2d_context.check_mode = (bool)value;
            } else {
                ALOGE("IM2D: It's not legal check config, it needs to be a 'bool'.");
                return IM_STATUS_ILLEGAL_PARAM;
            }
            break;
        default :
            ALOGE("IM2D: Unsupported config name!");
            return IM_STATUS_NOT_SUPPORTED;
    }

    return IM_STATUS_SUCCESS;
}
