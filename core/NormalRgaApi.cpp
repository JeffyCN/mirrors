/*
 * Copyright (C) 2016 Rockchip Electronics Co., Ltd.
 * Authors:
 *  Zhiqin Wei <wzq@rock-chips.com>
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

#include "NormalRga.h"
#include "NormalRgaContext.h"
#include "rga.h"
#ifdef ANDROID
#include "GrallocOps.h"
#endif

int         sina_table[360];
int         cosa_table[360];
/**********************************************************************
  =======================================================================
 **********************************************************************/

/* In order to be compatible with RK_FORMAT_XX and HAL_PIXEL_FORMAT_XX,
 * RK_FORMAT_XX is shifted to the left by 8 bits to distinguish. In order
 * to be compatible with the old RK_FORMAT_XX definition, a conversion
 * function is made here. */
int RkRgaCompatibleFormat(int format) {
#if LINUX
    if (format == 0)
        return format;

    if ((format >> 8) != 0) {
        return format;
    } else {
        return format << 8;
    }
#endif
    return format;
}

#ifdef ANDROID
int RkRgaGetRgaFormatFromAndroid(int format) {
    switch (format) {
        case HAL_PIXEL_FORMAT_RGB_565:
            return RK_FORMAT_RGB_565;
        case HAL_PIXEL_FORMAT_RGB_888:
            return RK_FORMAT_RGB_888;
        case HAL_PIXEL_FORMAT_RGBA_8888:
            return RK_FORMAT_RGBA_8888;
        case HAL_PIXEL_FORMAT_RGBX_8888:
            return RK_FORMAT_RGBX_8888;
        case HAL_PIXEL_FORMAT_BGRA_8888:
            return RK_FORMAT_BGRA_8888;
        case HAL_PIXEL_FORMAT_YCrCb_420_SP:
            return RK_FORMAT_YCrCb_420_SP;
        case HAL_PIXEL_FORMAT_YCrCb_NV12:
            return RK_FORMAT_YCbCr_420_SP;
        case HAL_PIXEL_FORMAT_YCrCb_NV12_VIDEO:
            return RK_FORMAT_YCbCr_420_SP;
        case HAL_PIXEL_FORMAT_YCrCb_NV12_10:
            return RK_FORMAT_YCbCr_420_SP_10B; //0x20
        default:
            ALOGE("%x is unsupport format now,pilese fix.", format);
            return -1;
    }
}
#endif

int RkRgaGetRgaFormat(int format) {
    /* Because the format of librga is the value of driver format << 8 . */
#ifdef ANDROID
    if (format & 0xFF) {
        format = RkRgaGetRgaFormatFromAndroid(format);
        if (format < 0)
            return -1;
    }
#endif
    if (format & 0xFF00 || format == 0)
        return format;
    else {
        format = RkRgaCompatibleFormat(format);
        if (format & 0xFF00 || format == 0)
            return format;
    }

    ALOGE("%x is unsupport format now,pilese fix.", format);
    return -1;
}

uint32_t bytesPerPixel(int format) {
    if (!(format & 0xFF00 || format == 0)) {
        format = RkRgaCompatibleFormat(format);
    }

    switch (format) {
        case RK_FORMAT_RGBA_8888:
        case RK_FORMAT_RGBX_8888:
        case RK_FORMAT_BGRA_8888:
        case RK_FORMAT_BGRX_8888:
        case RK_FORMAT_ARGB_8888:
        case RK_FORMAT_XRGB_8888:
        case RK_FORMAT_ABGR_8888:
        case RK_FORMAT_XBGR_8888:
            return 4;
        case RK_FORMAT_RGB_888:
        case RK_FORMAT_BGR_888:
            return 3;
        case RK_FORMAT_RGB_565:
        case RK_FORMAT_RGBA_5551:
        case RK_FORMAT_RGBA_4444:
        case RK_FORMAT_BGR_565:
        case RK_FORMAT_BGRA_5551:
        case RK_FORMAT_BGRA_4444:
        case RK_FORMAT_ARGB_5551:
        case RK_FORMAT_ARGB_4444:
        case RK_FORMAT_ABGR_5551:
        case RK_FORMAT_ABGR_4444:
            return 2;
        case RK_FORMAT_BPP1:
        case RK_FORMAT_BPP2:
        case RK_FORMAT_BPP4:
        case RK_FORMAT_BPP8:
            return 1;
    }
    return 0;
}

int checkRectForRga(rga_rect_t rect) {
    if (rect.xoffset < 0 || rect.yoffset < 0) {
        ALOGE("err offset[%d,%d]", rect.xoffset, rect.yoffset);
        return -EINVAL;
    }

    if (rect.width < 2 || rect.height < 2) {
        ALOGE("err act[%d,%d]", rect.width, rect.height);
        return -EINVAL;
    }

    if (rect.xoffset + rect.width > rect.wstride) {
        ALOGE("err ws[%d,%d,%d]", rect.xoffset, rect.width, rect.wstride);
        return -EINVAL;
    }

    if (rect.yoffset + rect.height > rect.hstride) {
        ALOGE("err hs[%d,%d,%d]", rect.yoffset, rect.height, rect.hstride);
        return -EINVAL;
    }

    if (NormalRgaIsYuvFormat(RkRgaGetRgaFormat(rect.format)) &&
        ((rect.wstride % 4) ||(rect.xoffset % 2) || (rect.width % 2) ||
         (rect.yoffset % 2) || (rect.height % 2) || (rect.hstride % 2))) {
        ALOGE("err yuv not align to 2");
        return -EINVAL;
    }

    return 0;
}

int isRectValid(rga_rect_t rect) {
    return rect.width > 0 && rect.height > 0;
}

#ifdef ANDROID
int NormalRgaGetRects(buffer_handle_t src,
                      buffer_handle_t dst,int* sType,int* dType,drm_rga_t* tmpRects) {
    int ret = 0;
    std::vector<int> srcAttrs,dstAttrs;
    if (src)
        ret = RkRgaGetHandleAttributes(src, &srcAttrs);
    if (ret) {
        ALOGE("dst handle get Attributes fail ret = %d,hnd=%p",ret,&src);
        printf("dst handle get Attributes fail ret = %d,hnd=%p",ret,&src);
        return ret;
    }

    if (dst)
        ret = RkRgaGetHandleAttributes(dst, &dstAttrs);
    if (ret) {
        ALOGE("dst handle get Attributes fail ret = %d,hnd=%p",ret,&dst);
        printf("dst handle get Attributes fail ret = %d,hnd=%p",ret,&dst);
        return ret;
    }

    memset(tmpRects,0,sizeof(drm_rga_t));

    if (src) {
        tmpRects->src.size = srcAttrs.at(ASIZE);
        tmpRects->src.width   = srcAttrs.at(AWIDTH);
        tmpRects->src.height  = srcAttrs.at(AHEIGHT);
        tmpRects->src.wstride = srcAttrs.at(ASTRIDE);
        tmpRects->src.format  = srcAttrs.at(AFORMAT);
        if (sType)
            *sType = srcAttrs.at(ATYPE);
    }

    if (dst) {
        tmpRects->dst.size = dstAttrs.at(ASIZE);
        tmpRects->dst.width   = dstAttrs.at(AWIDTH);
        tmpRects->dst.height  = dstAttrs.at(AHEIGHT);
        tmpRects->dst.wstride = dstAttrs.at(ASTRIDE);
        tmpRects->dst.format  = dstAttrs.at(AFORMAT);
        if (dType)
            *dType = dstAttrs.at(ATYPE);
    }

    return ret;
}

int NormalRgaGetRect(buffer_handle_t hnd, rga_rect_t *rect) {
    int ret = 0;
    std::vector<int> dstAttrs;

    if (!rect) {
        ALOGE("Get rect but rect[%p] is null point", rect);
        return -EINVAL;
    }

    ret = RkRgaGetHandleAttributes(hnd, &dstAttrs);
    if (ret) {
        ALOGE("dst handle get Attributes fail ret = %d,hnd=%p", ret, &hnd);
        printf("dst handle get Attributes fail ret = %d,hnd=%p", ret, &hnd);
        return ret;
    }

    memset(rect, 0, sizeof(rga_rect_t));

    rect->size = dstAttrs.at(ASIZE);
    rect->width   = dstAttrs.at(AWIDTH);
    rect->height  = dstAttrs.at(AHEIGHT);
    rect->wstride = dstAttrs.at(ASTRIDE);
    rect->format  = dstAttrs.at(AFORMAT);

    return ret;
}

int NormalRgaGetMmuType(buffer_handle_t hnd, int *mmuType) {
    int ret = 0;
    std::vector<int> dstAttrs;

    if (!mmuType) {
        ALOGE("Get rect but mmuType[%p] is null point", mmuType);
        return -EINVAL;
    }

    ret = RkRgaGetHandleAttributes(hnd, &dstAttrs);
    if (ret) {
        ALOGE("dst handle get Attributes fail ret = %d,hnd=%p", ret, &hnd);
        printf("dst handle get Attributes fail ret = %d,hnd=%p", ret, &hnd);
        return ret;
    }

    if (mmuType && dstAttrs.size() >= 5) {
#ifdef ANDROID_8
        *mmuType = dstAttrs.at(ASIZE);
#else
        *mmuType = dstAttrs.at(ATYPE);
#endif
    }

    return ret;
}
#endif

int NormalRgaSetRect(rga_rect_t *rect, int x, int y,
                     int w, int h, int s, int f) {
    if (!rect)
        return -EINVAL;

    rect->xoffset = x;
    rect->yoffset = y;
    rect->width = w;
    rect->height = h;
    rect->wstride = s;
    rect->format = f;

    return 0;
}

int NormalRgaSetSrcActiveInfo(struct rga_req *req,
                              unsigned int width, unsigned int height,
                              unsigned int x_off, unsigned int y_off) {
    req->src.act_w = width;
    req->src.act_h = height;
    req->src.x_offset = x_off;
    req->src.y_offset = y_off;

    return 1;
}

int NormalRgaSetFdsOffsets(struct rga_req *req,
                           uint16_t src_fd,     uint16_t dst_fd,
                           uint32_t src_offset, uint32_t dst_offset) {
    req->line_draw_info.color = src_fd | (dst_fd << 16);
    req->line_draw_info.flag = src_offset;
    req->line_draw_info.line_width = dst_offset;
    return 0;
}

#if defined(__arm64__) || defined(__aarch64__)
int NormalRgaSetSrcVirtualInfo(struct rga_req *req,
                               unsigned long yrgb_addr,unsigned long uv_addr,unsigned long v_addr,
                               unsigned int vir_w,unsigned int vir_h, unsigned int format,
                               unsigned char a_swap_en)
#else
int NormalRgaSetSrcVirtualInfo(struct rga_req *req,
                               unsigned int yrgb_addr, unsigned int uv_addr,unsigned int v_addr,
                               unsigned int vir_w, unsigned int vir_h, unsigned int format,
                               unsigned char a_swap_en)
#endif
{
    req->src.yrgb_addr = yrgb_addr;
    req->src.uv_addr  = uv_addr;
    req->src.v_addr   = v_addr;
    req->src.vir_w = vir_w;
    req->src.vir_h = vir_h;
    req->src.format = format >> 8;
    req->src.alpha_swap |= (a_swap_en & 1);

    return 1;
}

int NormalRgaSetDstActiveInfo(struct rga_req *req,
                              unsigned int width, unsigned int height,
                              unsigned int x_off, unsigned int y_off) {
    req->dst.act_w = width;
    req->dst.act_h = height;
    req->dst.x_offset = x_off;
    req->dst.y_offset = y_off;

    return 1;
}

#if defined(__arm64__) || defined(__aarch64__)
int NormalRgaSetDstVirtualInfo(struct rga_req *msg,
                               unsigned long yrgb_addr,unsigned long uv_addr,unsigned long v_addr,
                               unsigned int  vir_w,    unsigned int vir_h,
                               RECT *clip,
                               unsigned int format, unsigned char a_swap_en)
#else
int NormalRgaSetDstVirtualInfo(struct rga_req *msg,
                               unsigned int yrgb_addr,unsigned int uv_addr,  unsigned int v_addr,
                               unsigned int vir_w,    unsigned int vir_h,
                               RECT *clip,
                               unsigned int  format, unsigned char a_swap_en)
#endif
{
    msg->dst.yrgb_addr = yrgb_addr;
    msg->dst.uv_addr  = uv_addr;
    msg->dst.v_addr   = v_addr;
    msg->dst.vir_w = vir_w;
    msg->dst.vir_h = vir_h;
    msg->dst.format = format >> 8;

    msg->clip.xmin = clip->xmin;
    msg->clip.xmax = clip->xmax;
    msg->clip.ymin = clip->ymin;
    msg->clip.ymax = clip->ymax;

    msg->dst.alpha_swap |= (a_swap_en & 1);

    return 1;
}
int NormalRgaSetPatActiveInfo(struct rga_req *req,
                              unsigned int width, unsigned int height,
                              unsigned int x_off, unsigned int y_off) {
    req->pat.act_w = width;
    req->pat.act_h = height;
    req->pat.x_offset = x_off;
    req->pat.y_offset = y_off;

    return 1;
}

#if defined(__arm64__) || defined(__aarch64__)
int NormalRgaSetPatVirtualInfo(struct rga_req *msg,
                               unsigned long yrgb_addr,unsigned long uv_addr,unsigned long v_addr,
                               unsigned int  vir_w,    unsigned int vir_h,
                               RECT *clip,
                               unsigned int format, unsigned char a_swap_en)
#else
int NormalRgaSetPatVirtualInfo(struct rga_req *msg,
                               unsigned int yrgb_addr,unsigned int uv_addr,  unsigned int v_addr,
                               unsigned int vir_w,    unsigned int vir_h,
                               RECT *clip,
                               unsigned int  format, unsigned char a_swap_en)
#endif
{
    msg->pat.yrgb_addr = yrgb_addr;
    msg->pat.uv_addr  = uv_addr;
    msg->pat.v_addr   = v_addr;
    msg->pat.vir_w = vir_w;
    msg->pat.vir_h = vir_h;
    msg->pat.format = format >> 8;

    msg->clip.xmin = clip->xmin;
    msg->clip.xmax = clip->xmax;
    msg->clip.ymin = clip->ymin;
    msg->clip.ymax = clip->ymax;

    msg->pat.alpha_swap |= (a_swap_en & 1);

    return 1;
}

int NormalRgaSetPatInfo(struct rga_req *msg,
                        unsigned int width,unsigned int height,unsigned int x_off,
                        unsigned int y_off, unsigned int pat_format) {
    msg->pat.act_w = width;
    msg->pat.act_h = height;
    msg->pat.x_offset = x_off;
    msg->pat.y_offset = y_off;

    msg->pat.format = pat_format >> 8;

    return 1;
}

#if defined(__arm64__) || defined(__aarch64__)
int NormalRgaSetRopMaskInfo(struct rga_req *msg,
                            unsigned long rop_mask_addr,unsigned int rop_mask_endian_mode)
#else
int NormalRgaSetRopMaskInfo(struct rga_req *msg,
                            unsigned int rop_mask_addr,unsigned int rop_mask_endian_mode)
#endif
{
    msg->rop_mask_addr = rop_mask_addr;
    msg->endian_mode = rop_mask_endian_mode;
    return 1;
}

/* 0:alpha' = alpha + (alpha>>7) | alpha' = alpha */
/* 0 global alpha / 1 per pixel alpha / 2 mix mode */

/* porter duff alpha mode en */

/* use dst alpha  */

int NormalRgaSetAlphaEnInfo(struct rga_req *msg,
                            unsigned int alpha_cal_mode, unsigned int alpha_mode,
                            unsigned int global_a_value, unsigned int PD_en,
                            unsigned int PD_mode,        unsigned int dst_alpha_en ) {
    msg->alpha_rop_flag |= 1;
    msg->alpha_rop_flag |= ((PD_en & 1) << 3);
    msg->alpha_rop_flag |= ((alpha_cal_mode & 1) << 4);

    msg->alpha_global_value = global_a_value;
    msg->alpha_rop_mode |= (alpha_mode & 3);
    msg->alpha_rop_mode |= (dst_alpha_en << 5);

    msg->PD_mode = PD_mode;


    return 1;
}


int NormalRgaSetRopEnInfo(struct rga_req *msg,
                          unsigned int ROP_mode, unsigned int ROP_code,
                          unsigned int color_mode,unsigned int solid_color) {
    msg->alpha_rop_flag |= (0x3);
    msg->alpha_rop_mode |= ((ROP_mode & 3) << 2);

    msg->rop_code = ROP_code;
    msg->color_fill_mode = color_mode;
    msg->fg_color = solid_color;
    return 1;
}

int NormalRgaSetFadingEnInfo(struct rga_req *msg,
                             unsigned char r,unsigned char g,unsigned char b) {
    msg->alpha_rop_flag |= (0x1 << 2);

    msg->fading.b = b;
    msg->fading.g = g;
    msg->fading.r = r;
    return 1;
}

int NormalRgaSetSrcTransModeInfo(struct rga_req *msg,
                                 unsigned char trans_mode,unsigned char a_en,unsigned char b_en,
                                 unsigned char g_en,unsigned char r_en,unsigned int color_key_min,
                                 unsigned int color_key_max,unsigned char zero_mode_en
                                ) {
    msg->src_trans_mode = ((a_en & 1) << 4) | ((b_en & 1) << 3) |
                          ((g_en & 1) << 2) | ((r_en & 1) << 1) | (trans_mode & 1);

    msg->color_key_min = color_key_min;
    msg->color_key_max = color_key_max;
    msg->alpha_rop_mode |= (zero_mode_en << 4);
    return 1;
}

bool NormalRgaIsBppFormat(int format) {
    bool ret = false;

    switch (format) {
        case RK_FORMAT_BPP1:
        case RK_FORMAT_BPP2:
        case RK_FORMAT_BPP4:
        case RK_FORMAT_BPP8:
            ret = true;
            break;
        default:
            break;
    }

    return ret;
}

bool NormalRgaIsYuvFormat(int format) {
    bool ret = false;

    switch (format) {
        case RK_FORMAT_YCbCr_422_SP:
        case RK_FORMAT_YCbCr_422_P:
        case RK_FORMAT_YCbCr_420_SP:
        case RK_FORMAT_YCbCr_420_P:
        case RK_FORMAT_YCrCb_422_SP:
        case RK_FORMAT_YCrCb_422_P:
        case RK_FORMAT_YCrCb_420_SP:
        case RK_FORMAT_YCrCb_420_P:
        case RK_FORMAT_YVYU_422:
        case RK_FORMAT_YVYU_420:
        case RK_FORMAT_VYUY_422:
        case RK_FORMAT_VYUY_420:
        case RK_FORMAT_YUYV_422:
        case RK_FORMAT_YUYV_420:
        case RK_FORMAT_UYVY_422:
        case RK_FORMAT_UYVY_420:
        case RK_FORMAT_Y4:
        case RK_FORMAT_YCbCr_400:
        case RK_FORMAT_YCbCr_420_SP_10B:
        case RK_FORMAT_YCrCb_420_SP_10B:
        case RK_FORMAT_YCrCb_422_10b_SP:
        case RK_FORMAT_YCbCr_422_10b_SP:
            ret = true;
            break;
    }

    return ret;
}

bool NormalRgaIsRgbFormat(int format) {
    bool ret = false;

    switch (format) {
        case RK_FORMAT_RGBA_8888:
        case RK_FORMAT_RGBX_8888:
        case RK_FORMAT_RGBA_5551:
        case RK_FORMAT_RGBA_4444:
        case RK_FORMAT_RGB_888:
        case RK_FORMAT_RGB_565:
        case RK_FORMAT_BGRA_8888:
        case RK_FORMAT_BGRX_8888:
        case RK_FORMAT_BGRA_5551:
        case RK_FORMAT_BGRA_4444:
        case RK_FORMAT_BGR_888:
        case RK_FORMAT_BGR_565:
        /*ARGB*/
        case RK_FORMAT_ARGB_8888:
        case RK_FORMAT_XRGB_8888:
        case RK_FORMAT_ARGB_5551:
        case RK_FORMAT_ARGB_4444:
        case RK_FORMAT_ABGR_8888:
        case RK_FORMAT_XBGR_8888:
        case RK_FORMAT_ABGR_5551:
        case RK_FORMAT_ABGR_4444:
            ret = true;
            break;
        default:
            break;
    }

    return ret;
}

bool NormalRgaFormatHasAlpha(int format) {
    bool ret = false;

    switch (format) {
        case RK_FORMAT_RGBA_8888:
        case RK_FORMAT_RGBA_5551:
        case RK_FORMAT_RGBA_4444:
        case RK_FORMAT_BGRA_8888:
        case RK_FORMAT_BGRA_5551:
        case RK_FORMAT_BGRA_4444:
        case RK_FORMAT_ARGB_8888:
        case RK_FORMAT_ARGB_5551:
        case RK_FORMAT_ARGB_4444:
        case RK_FORMAT_ABGR_8888:
        case RK_FORMAT_ABGR_5551:
        case RK_FORMAT_ABGR_4444:
        case RK_FORMAT_RGBA2BPP:
            ret = true;
            break;
        default:
            break;
    }

    return ret;
}

// 0/near  1/bilnear  2/bicubic
// 0/copy 1/rotate_scale 2/x_mirror 3/y_mirror
// rotate angle
// dither en flag
// AA flag
int NormalRgaSetBitbltMode(struct rga_req *msg,
                           unsigned char scale_mode,  unsigned char rotate_mode,
                           unsigned int  angle,       unsigned int  dither_en,
                           unsigned int  AA_en,       unsigned int  yuv2rgb_mode) {
    unsigned int alpha_mode;
    msg->render_mode = bitblt_mode;

    msg->scale_mode = scale_mode;
    msg->rotate_mode = rotate_mode;

    msg->sina = sina_table[angle];
    msg->cosa = cosa_table[angle];

    msg->yuv2rgb_mode = yuv2rgb_mode;

    msg->alpha_rop_flag |= ((dither_en << 5) & 0x20);
    msg->alpha_rop_flag |= ((AA_en << 7) & 0x80);

    alpha_mode = msg->alpha_rop_mode & 3;
    if(rotate_mode == BB_ROTATE) {
        if (AA_en == ENABLE) {
            if ((msg->alpha_rop_flag & 0x3) == 0x1) {
                if (alpha_mode == 0) {
                    msg->alpha_rop_mode = 0x2;
                } else if (alpha_mode == 1) {
                    msg->alpha_rop_mode = 0x1;
                }
            } else {
                msg->alpha_rop_flag |= 1;
                msg->alpha_rop_mode = 1;
            }
        }
    }

    if (msg->src_trans_mode)
        msg->scale_mode = 0;

    return 0;
}

/* 1bpp/2bpp/4bpp/8bpp */
/* src endian mode sel */
/* BPP1 = 0 */
/* BPP1 = 1 */
int NormalRgaSetColorPaletteMode(struct rga_req *msg,
                                 unsigned char  palette_mode,unsigned char  endian_mode,
                                 unsigned int  bpp1_0_color, unsigned int  bpp1_1_color) {
    msg->render_mode = color_palette_mode;

    msg->palette_mode = palette_mode;
    msg->endian_mode = endian_mode;
    msg->fg_color = bpp1_0_color;
    msg->bg_color = bpp1_1_color;

    return 1;
}

/* gradient color part         */
/* saturation mode             */
/* patten fill or solid fill   */
/* solid color                 */
/* pattern width               */
/* pattern height              */
/* pattern x offset            */
/* pattern y offset            */
/* alpha en                    */
int NormalRgaSetColorFillMode(
    struct rga_req *msg,                COLOR_FILL  *gr_color,
    unsigned char  gr_satur_mode,       unsigned char  cf_mode,
    unsigned int color,                 unsigned short pat_width,
    unsigned short pat_height,          unsigned char pat_x_off,
    unsigned char pat_y_off,            unsigned char aa_en) {
    msg->render_mode = color_fill_mode;

    msg->gr_color.gr_x_a = ((int)(gr_color->gr_x_a * 256.0))& 0xffff;
    msg->gr_color.gr_x_b = ((int)(gr_color->gr_x_b * 256.0))& 0xffff;
    msg->gr_color.gr_x_g = ((int)(gr_color->gr_x_g * 256.0))& 0xffff;
    msg->gr_color.gr_x_r = ((int)(gr_color->gr_x_r * 256.0))& 0xffff;

    msg->gr_color.gr_y_a = ((int)(gr_color->gr_y_a * 256.0))& 0xffff;
    msg->gr_color.gr_y_b = ((int)(gr_color->gr_y_b * 256.0))& 0xffff;
    msg->gr_color.gr_y_g = ((int)(gr_color->gr_y_g * 256.0))& 0xffff;
    msg->gr_color.gr_y_r = ((int)(gr_color->gr_y_r * 256.0))& 0xffff;

    msg->color_fill_mode = cf_mode;

    msg->pat.act_w = pat_width;
    msg->pat.act_h = pat_height;

    msg->pat.x_offset = pat_x_off;
    msg->pat.y_offset = pat_y_off;

    msg->fg_color = color;

    msg->alpha_rop_flag |= ((gr_satur_mode & 1) << 6);

    if(aa_en) {
        msg->alpha_rop_flag |= 0x1;
        msg->alpha_rop_mode  = 1;
    }
    return 1;
}

/* start point              */
/* end   point              */
/* line point drawing color */
/* line width               */
/* AA en                    */
/* last point en            */
int NormalRgaSetLineDrawingMode(struct rga_req *msg,
                                POINT sp,                     POINT  ep,
                                unsigned int color,           unsigned int line_width,
                                unsigned char AA_en,          unsigned char last_point_en)

{
    msg->render_mode = line_point_drawing_mode;

    msg->line_draw_info.start_point.x = sp.x;
    msg->line_draw_info.start_point.y = sp.y;
    msg->line_draw_info.end_point.x = ep.x;
    msg->line_draw_info.end_point.y = ep.y;

    msg->line_draw_info.color = color;
    msg->line_draw_info.line_width = line_width;
    msg->line_draw_info.flag |= (AA_en & 1);
    msg->line_draw_info.flag |= ((last_point_en & 1) << 1);

    if (AA_en == 1) {
        msg->alpha_rop_flag = 1;
        msg->alpha_rop_mode = 0x1;
    }

    return 1;
}

/* blur/sharpness   */
/* filter intensity */
/* dither_en flag   */

int NormalRgaSetBlurSharpFilterMode(
    struct rga_req *msg,         unsigned char filter_mode,
    unsigned char filter_type,   unsigned char dither_en) {
    msg->render_mode = blur_sharp_filter_mode;

    msg->bsfilter_flag |= (filter_type & 3);
    msg->bsfilter_flag |= ((filter_mode & 1) << 2);
    msg->alpha_rop_flag |= ((dither_en & 1) << 5);
    return 1;
}

int NormalRgaSetPreScalingMode(
    struct rga_req *msg, unsigned char dither_en) {
    msg->render_mode = pre_scaling_mode;

    msg->alpha_rop_flag |= ((dither_en & 1) << 5);
    return 1;
}

/* LUT table addr      */
/* 1bpp/2bpp/4bpp/8bpp */
#if defined(__arm64__) || defined(__aarch64__)
int NormalRgaUpdatePaletteTableMode(
    struct rga_req *msg,unsigned long LUT_addr,unsigned int palette_mode)
#else
int NormalRgaUpdatePaletteTableMode(
    struct rga_req *msg,unsigned int LUT_addr, unsigned int palette_mode)
#endif
{
    msg->render_mode = update_palette_table_mode;

    msg->LUT_addr = LUT_addr;
    msg->palette_mode = palette_mode;
    return 1;
}

/* patten addr    */
/* patten width   */
/* patten height  */
/* patten format  */

int NormalRgaUpdatePattenBuffMode(struct rga_req *msg,
                                  unsigned int pat_addr, unsigned int w,
                                  unsigned int h,        unsigned int format) {
    msg->render_mode = update_patten_buff_mode;

    msg->pat.yrgb_addr   = pat_addr;
    msg->pat.act_w  = w*h;
    msg->pat.act_h  = 1;
    msg->pat.format = format;
    return 1;
}

#if defined(__arm64__) || defined(__aarch64__)
int NormalRgaMmuInfo(struct rga_req *msg,
                     unsigned char  mmu_en,   unsigned char  src_flush,
                     unsigned char  dst_flush,unsigned char  cmd_flush,
                     unsigned long base_addr, unsigned char  page_size)
#else
int NormalRgaMmuInfo(struct rga_req *msg,
                     unsigned char  mmu_en,   unsigned char  src_flush,
                     unsigned char  dst_flush,unsigned char  cmd_flush,
                     unsigned int base_addr,  unsigned char  page_size)
#endif
{
    msg->mmu_info.mmu_en    = mmu_en;
    msg->mmu_info.base_addr = base_addr;
    msg->mmu_info.mmu_flag  = ((page_size & 0x3) << 4) |
                              ((cmd_flush & 0x1) << 3) |
                              ((dst_flush & 0x1) << 2) |
                              ((src_flush & 0x1) << 1) | mmu_en;
    return 1;
}

int NormalRgaMmuFlag(struct rga_req *msg,
                     int  src_mmu_en,   int  dst_mmu_en) {
    if (src_mmu_en || dst_mmu_en)
        msg->mmu_info.mmu_flag |= (0x1 << 31);

    if (src_mmu_en)
        msg->mmu_info.mmu_flag |= (0x1 << 8);

    if (dst_mmu_en)
        msg->mmu_info.mmu_flag |= (0x1 << 10);

    return 1;
}

int NormalRgaNNQuantizeMode(struct rga_req *msg, rga_info *dst) {
    if (dst->nn.nn_flag == 1) {
        msg->alpha_rop_flag |= (dst->nn.nn_flag << 8);

        msg->gr_color.gr_x_r = dst->nn.scale_r;
        msg->gr_color.gr_x_g = dst->nn.scale_g;
        msg->gr_color.gr_x_b = dst->nn.scale_b;

        msg->gr_color.gr_y_r = dst->nn.offset_r;
        msg->gr_color.gr_y_g = dst->nn.offset_g;
        msg->gr_color.gr_y_b = dst->nn.offset_b;
    }

    return 0;
}

int NormalRgaFullColorSpaceConvert(struct rga_req *msg, int color_space_mode) {
    typedef struct csc_coe_float_t {
        float r_v;
        float g_y;
        float b_u;
        float off;
    } csc_coe_float_t;

    typedef struct full_csc_float_t {
        csc_coe_float_t coe_y;
        csc_coe_float_t coe_u;
        csc_coe_float_t coe_v;
    } full_csc_float_t;

    int factor = 0;
    full_csc_float_t *fptr = NULL;
    full_csc_t default_csc_table;

    /* ABGR => AUYV */
    static full_csc_float_t default_csc_float_table[] = {
        /* coe_00 * R + coe_01 * G + coe_02 * B + coe_off */
        { { 0.299, 0.587, 0.114, 0 }, { -0.169, -0.331, 0.5, 128 }, { 0.5, -0.419, -0.081, 128 } }, //R2Y 601 full
        { { 0.213, 0.715, 0.072, 0 }, { -0.115, -0.385, 0.5, 128 }, { 0.5, -0.454, -0.046, 128 } }, //R2Y 709 full
        /* coe_00 * V + coe_01 * Y + coe_02 * U + coe_off */
        { { -0.1826, 0.8588, -0.1014, 52.3554 }, { 0.1007, -0.0004, 0.8948, 0.5781 }, { 0.9005, 0, 0.0653, 4.3855 } },    //601 full range => 709 limit range
        { { 0.1916, 1, 0.0993, -37.2476 }, { -0.1106, 0, 0.9895, 15.4669 }, { 0.9827, 0.0002, -0.0723, 11.4231 } },       //709  limit range => 601 limit range
        { { 0.1685, 0.8588, 0.0872, -16.7232 }, { -0.0971, 0, 0.8695, 29.1335 }, { 0.8638, 0, -0.0637, 25.5824 } },       //709  full range => 601 limit range
        { { 0.1955, 1, 0.1019, -38.0729 }, { -0.1104, 0, 0.9899, 15.4218 }, { 0.9836, 0, -0.0716, 11.2587 } },            //709  full range => 601 full range
    };

    factor = 0x3ff;

    switch (color_space_mode) {
        case rgb2yuv_601_full :
            fptr = &(default_csc_float_table[0]);
            break;

        case rgb2yuv_709_full :
            fptr = &(default_csc_float_table[1]);
            break;

        case yuv2yuv_709_limit_2_601_limit :
            fptr = &(default_csc_float_table[3]);
            break;

        case yuv2yuv_601_full_2_709_limit :
            fptr = &(default_csc_float_table[2]);
            break;

        case yuv2yuv_709_full_2_601_limit :
            fptr = &(default_csc_float_table[4]);
            break;

        case yuv2yuv_709_full_2_601_full :
            fptr = &(default_csc_float_table[5]);
            break;

        case yuv2yuv_601_limit_2_709_limit :
        case yuv2yuv_601_limit_2_709_full :
        case yuv2yuv_601_full_2_709_full :
        case yuv2yuv_709_limit_2_601_full :
        default :
            printf("Not support full csc mode [%x]\n", color_space_mode);
            return -1;
    }

    /* enable full csc */
    default_csc_table.flag = 1;

    /* full csc coefficient */
    default_csc_table.coe_y.r_v = (int)(fptr->coe_y.r_v * factor +0.5);
    default_csc_table.coe_y.g_y = (int)(fptr->coe_y.g_y * factor +0.5);
    default_csc_table.coe_y.b_u = (int)(fptr->coe_y.b_u * factor +0.5);
    default_csc_table.coe_y.off = (int)(fptr->coe_y.off * factor +0.5);

    default_csc_table.coe_u.r_v = (int)(fptr->coe_u.r_v * factor +0.5);
    default_csc_table.coe_u.g_y = (int)(fptr->coe_u.g_y * factor +0.5);
    default_csc_table.coe_u.b_u = (int)(fptr->coe_u.b_u * factor +0.5);
    default_csc_table.coe_u.off = (int)(fptr->coe_u.off * factor +0.5);

    default_csc_table.coe_v.r_v = (int)(fptr->coe_v.r_v * factor +0.5);
    default_csc_table.coe_v.g_y = (int)(fptr->coe_v.g_y * factor +0.5);
    default_csc_table.coe_v.b_u = (int)(fptr->coe_v.b_u * factor +0.5);
    default_csc_table.coe_v.off = (int)(fptr->coe_v.off * factor +0.5);

    if (color_space_mode >> 8) {
        msg->full_csc.flag = 1;
        memcpy(&msg->full_csc, &default_csc_table, sizeof(full_csc_t));
    }

    return 0;
}


int NormalRgaDitherMode(struct rga_req *msg, rga_info *dst, int format)
{
    if (dst->dither.enable == 1)
    {
        msg->alpha_rop_flag = 1;
        msg->alpha_rop_flag |= (dst->dither.enable << 5);
    }

    if (format == RK_FORMAT_Y4)
    {
        msg->dither_mode = dst->dither.mode;

        msg->gr_color.gr_x_r = dst->dither.lut0_l;
        msg->gr_color.gr_x_g = dst->dither.lut0_h;
        msg->gr_color.gr_y_r = dst->dither.lut1_l;
        msg->gr_color.gr_y_g = dst->dither.lut1_h;
    }

    return 0;
}

int NormalRgaInitTables() {
    int sinaTable[360] = {
        0,   1144,   2287,   3430,   4572,   5712,   6850,   7987,   9121,  10252,
        11380,  12505,  13626,  14742,  15855,  16962,  18064,  19161,  20252,  21336,
        22415,  23486,  24550,  25607,  26656,  27697,  28729,  29753,  30767,  31772,
        32768,  33754,  34729,  35693,  36647,  37590,  38521,  39441,  40348,  41243,
        42126,  42995,  43852,  44695,  45525,  46341,  47143,  47930,  48703,  49461,
        50203,  50931,  51643,  52339,  53020,  53684,  54332,  54963,  55578,  56175,
        56756,  57319,  57865,  58393,  58903,  59396,  59870,  60326,  60764,  61183,
        61584,  61966,  62328,  62672,  62997,  63303,  63589,  63856,  64104,  64332,
        64540,  64729,  64898,  65048,  65177,  65287,  65376,  65446,  65496,  65526,
        65536,  65526,  65496,  65446,  65376,  65287,  65177,  65048,  64898,  64729,
        64540,  64332,  64104,  63856,  63589,  63303,  62997,  62672,  62328,  61966,
        61584,  61183,  60764,  60326,  59870,  59396,  58903,  58393,  57865,  57319,
        56756,  56175,  55578,  54963,  54332,  53684,  53020,  52339,  51643,  50931,
        50203,  49461,  48703,  47930,  47143,  46341,  45525,  44695,  43852,  42995,
        42126,  41243,  40348,  39441,  38521,  37590,  36647,  35693,  34729,  33754,
        32768,  31772,  30767,  29753,  28729,  27697,  26656,  25607,  24550,  23486,
        22415,  21336,  20252,  19161,  18064,  16962,  15855,  14742,  13626,  12505,
        11380,  10252,   9121,   7987,   6850,   5712,   4572,   3430,   2287,   1144,
        0,  -1144,  -2287,  -3430,  -4572,  -5712,  -6850,  -7987,  -9121, -10252,
        -11380, -12505, -13626, -14742, -15855, -16962, -18064, -19161, -20252, -21336,
        -22415, -23486, -24550, -25607, -26656, -27697, -28729, -29753, -30767, -31772,
        -32768, -33754, -34729, -35693, -36647, -37590, -38521, -39441, -40348, -41243,
        -42126, -42995, -43852, -44695, -45525, -46341, -47143, -47930, -48703, -49461,
        -50203, -50931, -51643, -52339, -53020, -53684, -54332, -54963, -55578, -56175,
        -56756, -57319, -57865, -58393, -58903, -59396, -59870, -60326, -60764, -61183,
        -61584, -61966, -62328, -62672, -62997, -63303, -63589, -63856, -64104, -64332,
        -64540, -64729, -64898, -65048, -65177, -65287, -65376, -65446, -65496, -65526,
        -65536, -65526, -65496, -65446, -65376, -65287, -65177, -65048, -64898, -64729,
        -64540, -64332, -64104, -63856, -63589, -63303, -62997, -62672, -62328, -61966,
        -61584, -61183, -60764, -60326, -59870, -59396, -58903, -58393, -57865, -57319,
        -56756, -56175, -55578, -54963, -54332, -53684, -53020, -52339, -51643, -50931,
        -50203, -49461, -48703, -47930, -47143, -46341, -45525, -44695, -43852, -42995,
        -42126, -41243, -40348, -39441, -38521, -37590, -36647, -35693, -34729, -33754,
        -32768, -31772, -30767, -29753, -28729, -27697, -26656, -25607, -24550, -23486,
        -22415, -21336, -20252, -19161, -18064, -16962, -15855, -14742, -13626, -12505,
        -11380, -10252, -9121,   -7987,  -6850,  -5712,  -4572,  -3430,  -2287,  -1144
    };
    int cosaTable[360] = {
        65536,  65526,  65496,  65446,  65376,  65287,  65177,  65048,  64898,  64729,
        64540,  64332,  64104,  63856,  63589,  63303,  62997,  62672,  62328,  61966,
        61584,  61183,  60764,  60326,  59870,  59396,  58903,  58393,  57865,  57319,
        56756,  56175,  55578,  54963,  54332,  53684,  53020,  52339,  51643,  50931,
        50203,  49461,  48703,  47930,  47143,  46341,  45525,  44695,  43852,  42995,
        42126,  41243,  40348,  39441,  38521,  37590,  36647,  35693,  34729,  33754,
        32768,  31772,  30767,  29753,  28729,  27697,  26656,  25607,  24550,  23486,
        22415,  21336,  20252,  19161,  18064,  16962,  15855,  14742,  13626,  12505,
        11380,  10252,   9121,   7987,   6850,   5712,   4572,   3430,   2287,   1144,
        0,  -1144,  -2287,  -3430,  -4572,  -5712,  -6850,  -7987,  -9121, -10252,
        -11380, -12505, -13626, -14742, -15855, -16962, -18064, -19161, -20252, -21336,
        -22415, -23486, -24550, -25607, -26656, -27697, -28729, -29753, -30767, -31772,
        -32768, -33754, -34729, -35693, -36647, -37590, -38521, -39441, -40348, -41243,
        -42126, -42995, -43852, -44695, -45525, -46341, -47143, -47930, -48703, -49461,
        -50203, -50931, -51643, -52339, -53020, -53684, -54332, -54963, -55578, -56175,
        -56756, -57319, -57865, -58393, -58903, -59396, -59870, -60326, -60764, -61183,
        -61584, -61966, -62328, -62672, -62997, -63303, -63589, -63856, -64104, -64332,
        -64540, -64729, -64898, -65048, -65177, -65287, -65376, -65446, -65496, -65526,
        -65536, -65526, -65496, -65446, -65376, -65287, -65177, -65048, -64898, -64729,
        -64540, -64332, -64104, -63856, -63589, -63303, -62997, -62672, -62328, -61966,
        -61584, -61183, -60764, -60326, -59870, -59396, -58903, -58393, -57865, -57319,
        -56756, -56175, -55578, -54963, -54332, -53684, -53020, -52339, -51643, -50931,
        -50203, -49461, -48703, -47930, -47143, -46341, -45525, -44695, -43852, -42995,
        -42126, -41243, -40348, -39441, -38521, -37590, -36647, -35693, -34729, -33754,
        -32768, -31772, -30767, -29753, -28729, -27697, -26656, -25607, -24550, -23486,
        -22415, -21336, -20252, -19161, -18064, -16962, -15855, -14742, -13626, -12505,
        -11380, -10252,  -9121,  -7987,  -6850,  -5712,  -4572,  -3430,  -2287,  -1144,
        0,   1144,   2287,   3430,   4572,   5712,   6850,   7987,   9121,  10252,
        11380,  12505,  13626,  14742,  15855,  16962,  18064,  19161,  20252,  21336,
        22415,  23486,  24550,  25607,  26656,  27697,  28729,  29753,  30767,  31772,
        32768,  33754,  34729,  35693,  36647,  37590,  38521,  39441,  40348,  41243,
        42126,  42995,  43852,  44695,  45525,  46341,  47143,  47930,  48703,  49461,
        50203,  50931,  51643,  52339,  53020,  53684,  54332,  54963,  55578,  56175,
        56756,  57319,  57865,  58393,  58903,  59396,  59870,  60326,  60764,  61183,
        61584,  61966,  62328,  62672,  62997,  63303,  63589,  63856,  64104,  64332,
        64540,  64729,  64898,  65048,  65177,  65287,  65376,  65446,  65496,  65526
    };
    memcpy(sina_table, sinaTable, sizeof(sina_table));
    memcpy(cosa_table, cosaTable, sizeof(cosa_table));
    return 0;
}

void NormalRgaLogOutRgaReq(struct rga_req rgaReg) {
    ALOGE("render_mode = %d rotate_mode = %d in_fence_fd = %d handle_flag = 0x%x",
          rgaReg.render_mode, rgaReg.rotate_mode, rgaReg.in_fence_fd, rgaReg.handle_flag);
    ALOGE("src:[%lx,%lx,%lx],x-y[%d,%d],w-h[%d,%d],vw-vh[%d,%d],f=%d, rd_mode = %d",
          (unsigned long)rgaReg.src.yrgb_addr, (unsigned long)rgaReg.src.uv_addr, (unsigned long)rgaReg.src.v_addr,
          rgaReg.src.x_offset, rgaReg.src.y_offset,
          rgaReg.src.act_w, rgaReg.src.act_h,
          rgaReg.src.vir_w, rgaReg.src.vir_h, rgaReg.src.format, rgaReg.src.rd_mode);
    ALOGE("dst:[%lx,%lx,%lx],x-y[%d,%d],w-h[%d,%d],vw-vh[%d,%d],f=%d, rd_mode = %d",
          (unsigned long)rgaReg.dst.yrgb_addr, (unsigned long)rgaReg.dst.uv_addr, (unsigned long)rgaReg.dst.v_addr,
          rgaReg.dst.x_offset, rgaReg.dst.y_offset,
          rgaReg.dst.act_w, rgaReg.dst.act_h,
          rgaReg.dst.vir_w, rgaReg.dst.vir_h, rgaReg.dst.format, rgaReg.dst.rd_mode);
    ALOGE("pat:[%lx,%lx,%lx],x-y[%d,%d],w-h[%d,%d],vw-vh[%d,%d],f=%d, rd_mode = %d",
          (unsigned long)rgaReg.pat.yrgb_addr, (unsigned long)rgaReg.pat.uv_addr, (unsigned long)rgaReg.pat.v_addr,
          rgaReg.pat.x_offset, rgaReg.pat.y_offset,
          rgaReg.pat.act_w, rgaReg.pat.act_h,
          rgaReg.pat.vir_w, rgaReg.pat.vir_h, rgaReg.pat.format, rgaReg.pat.rd_mode);
    ALOGE("ROP:[%lx,%x,%x],LUT[%lx]", (unsigned long)rgaReg.rop_mask_addr, rgaReg.alpha_rop_flag,
          rgaReg.rop_code, (unsigned long)rgaReg.LUT_addr);

    ALOGE("color:[%x,%x,%x,%x,%x]", rgaReg.color_key_max, rgaReg.color_key_min,
          rgaReg.fg_color, rgaReg.bg_color, rgaReg.color_fill_mode);

    ALOGE("MMU:[%d,%lx,%x]", rgaReg.mmu_info.mmu_en,
          (unsigned long)rgaReg.mmu_info.base_addr, rgaReg.mmu_info.mmu_flag);


    ALOGE("mode[%d,%d,%d,%d,%d]", rgaReg.palette_mode, rgaReg.yuv2rgb_mode,
          rgaReg.endian_mode, rgaReg.src_trans_mode,rgaReg.scale_mode);

    ALOGE("Full CSC : EN[%d] FACTOR[%d, %d, %d, %d], [%d, %d, %d, %d], [%d, %d, %d, %d]",
          rgaReg.full_csc.flag,
          rgaReg.full_csc.coe_y.r_v, rgaReg.full_csc.coe_y.g_y, rgaReg.full_csc.coe_y.b_u, rgaReg.full_csc.coe_y.off,
          rgaReg.full_csc.coe_u.r_v, rgaReg.full_csc.coe_u.g_y, rgaReg.full_csc.coe_u.b_u, rgaReg.full_csc.coe_u.off,
          rgaReg.full_csc.coe_v.r_v, rgaReg.full_csc.coe_v.g_y, rgaReg.full_csc.coe_v.b_u, rgaReg.full_csc.coe_v.off);

    ALOGE("gr_color_x [%x, %x, %x]", rgaReg.gr_color.gr_x_r, rgaReg.gr_color.gr_x_g, rgaReg.gr_color.gr_x_b);
    ALOGE("gr_color_x [%x, %x, %x]", rgaReg.gr_color.gr_y_r, rgaReg.gr_color.gr_y_g, rgaReg.gr_color.gr_y_b);

    return;
}

static inline void NormalRgaCompatModeConvertRga2ImgeInfo(rga2_img_info_t *info, rga_img_info_t *orig_info) {
    info->yrgb_addr = orig_info->yrgb_addr;
    info->uv_addr = orig_info->uv_addr;
    info->v_addr = orig_info->v_addr;
    info->format = orig_info->format;
    info->act_w = orig_info->act_w;
    info->act_h = orig_info->act_h;
    info->x_offset = orig_info->x_offset;
    info->y_offset = orig_info->y_offset;
    info->vir_w = orig_info->vir_w;
    info->vir_h = orig_info->vir_h;
    info->endian_mode = orig_info->endian_mode;
    info->alpha_swap = orig_info->alpha_swap;
}

static inline void NormalRgaCompatModeConvertRga2Rect(rga2_rect_t *clip, RECT *orig_clip) {
    clip->xmax = orig_clip->xmax;
    clip->ymax = orig_clip->ymax;
    clip->xmin = orig_clip->xmin;
    clip->ymax = orig_clip->ymax;
}

static inline void NormalRgaCompatModeConvertRga2ColorFill(rga2_color_fill_t *color, COLOR_FILL *orig_color) {
    color->gr_x_a = orig_color->gr_x_a;
    color->gr_y_a = orig_color->gr_y_a;
    color->gr_x_b = orig_color->gr_x_b;
    color->gr_y_b = orig_color->gr_y_b;
    color->gr_x_g = orig_color->gr_x_g;
    color->gr_y_g = orig_color->gr_y_g;
    color->gr_x_r = orig_color->gr_x_r;
    color->gr_y_r = orig_color->gr_y_r;
}

static inline void NormalRgaCompatModeConvertRga2LineDrawInfo(rga2_line_draw_t *info, line_draw_t *orig_info) {
    info->start_point.x = orig_info->start_point.x;
    info->start_point.y = orig_info->start_point.y;
    info->end_point.x = orig_info->end_point.x;
    info->end_point.y = orig_info->end_point.y;
    info->color = orig_info->color;
    info->flag = orig_info->flag;
    info->line_width = orig_info->line_width;
}

static inline void NormalRgaCompatModeConvertRga2Fading(rga2_fading_t *fading, FADING *orig_fading) {
    fading->b = orig_fading->b;
    fading->g = orig_fading->g;
    fading->r = orig_fading->r;
    fading->res = orig_fading->res;
}

static inline void NormalRgaCompatModeConvertRga2Mmu(rga2_mmu_t *mmu, MMU *orig_mmu) {
    mmu->mmu_en = orig_mmu->mmu_en;
#if defined(__arm64__) || defined(__aarch64__)
    mmu->base_addr = (unsigned long)orig_mmu->base_addr;
#else
    mmu->base_addr = (unsigned int)orig_mmu->base_addr;
#endif
    mmu->mmu_flag = orig_mmu->mmu_flag;
}

static inline void NormalRgaCompatModeConvertRga2FullCscCoe(rga2_csc_coe_t *coe, csc_coe_t *orig_coe) {
    coe->r_v = orig_coe->r_v;
    coe->g_y = orig_coe->g_y;
    coe->b_u = orig_coe->b_u;
    coe->off = orig_coe->off;
}

static inline void NormalRgaCompatModeConvertRga2FullCsc(rga2_full_csc_t *csc, full_csc_t *orig_csc) {
    csc->flag = orig_csc->flag;

    NormalRgaCompatModeConvertRga2FullCscCoe(&csc->coe_y, &orig_csc->coe_y);
    NormalRgaCompatModeConvertRga2FullCscCoe(&csc->coe_u, &orig_csc->coe_u);
    NormalRgaCompatModeConvertRga2FullCscCoe(&csc->coe_v, &orig_csc->coe_v);
}

void NormalRgaCompatModeConvertRga2(rga2_req *req, rga_req *orig_req) {
    req->render_mode = orig_req->render_mode;

    NormalRgaCompatModeConvertRga2ImgeInfo(&req->src, &orig_req->src);
    NormalRgaCompatModeConvertRga2ImgeInfo(&req->dst, &orig_req->dst);
    NormalRgaCompatModeConvertRga2ImgeInfo(&req->pat, &orig_req->pat);

#if defined(__arm64__) || defined(__aarch64__)
    req->rop_mask_addr = (unsigned long)orig_req->rop_mask_addr;
    req->LUT_addr = (unsigned long)orig_req->LUT_addr;
#else
    req->rop_mask_addr = (unsigned int)orig_req->rop_mask_addr;
    req->LUT_addr = (unsigned int)orig_req->LUT_addr;
#endif

    NormalRgaCompatModeConvertRga2Rect(&req->clip, &orig_req->clip);

    req->sina = orig_req->sina;
    req->cosa = orig_req->cosa;
    req->alpha_rop_flag = orig_req->alpha_rop_flag;
    req->scale_mode = orig_req->scale_mode;
    req->color_key_max = orig_req->color_key_max;
    req->color_key_min = orig_req->color_key_min;
    req->fg_color = orig_req->fg_color;
    req->bg_color = orig_req->bg_color;

    NormalRgaCompatModeConvertRga2ColorFill(&req->gr_color, &orig_req->gr_color);
    NormalRgaCompatModeConvertRga2LineDrawInfo(&req->line_draw_info, &orig_req->line_draw_info);
    NormalRgaCompatModeConvertRga2Fading(&req->fading, &orig_req->fading);

    req->PD_mode = orig_req->PD_mode;
    req->alpha_global_value = orig_req->alpha_global_value;
    req->rop_code = orig_req->rop_code;
    req->bsfilter_flag = orig_req->bsfilter_flag;
    req->palette_mode = orig_req->palette_mode;
    req->yuv2rgb_mode = orig_req->yuv2rgb_mode;
    req->endian_mode = orig_req->endian_mode;
    req->rotate_mode = orig_req->rotate_mode;
    req->color_fill_mode = orig_req->color_fill_mode;

    NormalRgaCompatModeConvertRga2Mmu(&req->mmu_info, &orig_req->mmu_info);

    req->alpha_rop_mode = orig_req->alpha_rop_mode;
    req->src_trans_mode = orig_req->src_trans_mode;
    req->dither_mode = orig_req->dither_mode;

    NormalRgaCompatModeConvertRga2FullCsc(&req->full_csc, &orig_req->full_csc);

    /* multi_rga moved this part of the processing to the driver. */
    if (req->sina == 65536 && req->cosa == 0) {
        /* rotate 90 */
        req->dst.x_offset = req->dst.x_offset + req->dst.act_h - 1;
    } else if (req->sina == 0 && req->cosa == -65536) {
        /* rotate 180 */
        req->dst.x_offset = req->dst.x_offset + req->dst.act_w - 1;
        req->dst.y_offset = req->dst.y_offset + req->dst.act_h - 1;
    } else if (req->sina == -65536 && req->cosa == 0) {
        /* totate 270 */
        req->dst.y_offset = req->dst.y_offset + req->dst.act_w - 1;
    }
}
