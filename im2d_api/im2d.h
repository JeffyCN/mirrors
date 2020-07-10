/*
 * Copyright (C) 2020 Rockchip Electronics Co.Ltd
 * Authors:
 *  PutinLee <putin.lee@rock-chips.com>
 *  Cerf Yu <cerf.yu@rock-chips.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifdef ANDROID
#include <hardware/rga.h>
#endif

#ifndef IM_API
#define IM_API /* define API export as needed */
#endif

#define RGA_IM2D_VERSION "1.00"

typedef enum {
    /* Rotation */
    IM_HAL_TRANSFORM_ROT_90     = 1 << 0,
    IM_HAL_TRANSFORM_ROT_180    = 1 << 1,
    IM_HAL_TRANSFORM_ROT_270    = 1 << 2,
    IM_HAL_TRANSFORM_FLIP_H     = 1 << 3,
    IM_HAL_TRANSFORM_FLIP_V     = 1 << 4,
    IM_HAL_TRANSFORM_MASK       = 0x1f,

    /*
     * Blend
     * Additional blend usage, can be used with both source and target configs.
     * If none of the below is set, the default "SRC over DST" is applied.
     */
    IM_ALPHA_BLEND_SRC_OVER     = 1 << 5,     /* Default, Porter-Duff "SRC over DST" */
    IM_ALPHA_BLEND_DST          = 1 << 6,     /* Porter-Duff "DST" */
    IM_ALPHA_BLEND_SRC_IN       = 1 << 7,     /* Porter-Duff "SRC in DST" */
    IM_ALPHA_BLEND_DST_IN       = 1 << 8,     /* Porter-Duff "DST in SRC" */
    IM_ALPHA_BLEND_SRC_OUT      = 1 << 9,     /* Porter-Duff "SRC out DST" */
    IM_ALPHA_BLEND_DST_OUT      = 1 << 10,    /* Porter-Duff "DST out SRC" */
    IM_ALPHA_BLEND_DST_OVER     = 1 << 11,    /* Porter-Duff "DST over SRC" */
    IM_ALPHA_BLEND_SRC_ATOP     = 1 << 12,    /* Porter-Duff "SRC ATOP" */
    IM_ALPHA_BLEND_DST_ATOP     = 1 << 13,    /* Porter-Duff "DST ATOP" */
    IM_ALPHA_BLEND_XOR          = 1 << 14,    /* Xor */
    IM_ALPHA_BLEND_MASK         = 0x7fe0,

    IM_SYNC                     = 1 << 15,
    IM_CROP                     = 1 << 16,
    IM_COLOR_FILL               = 1 << 17,
} IM_USAGE;

/* Status codes, returned by any blit function */
typedef enum {
    IM_STATUS_NOERROR         =  2,
    IM_STATUS_SUCCESS         =  1,
    IM_STATUS_NOT_SUPPORTED   = -1,
    IM_STATUS_OUT_OF_MEMORY   = -2,
    IM_STATUS_INVALID_PARAM   = -3,
    IM_STATUS_FAILED          =  0,
} IM_STATUS;

/* Status codes, returned by any blit function */
typedef enum {
    IM_YUV_TO_RGB_BT601_LIMIT     = 1 << 0,
    IM_YUV_TO_RGB_BT601_FULL      = 2 << 0,
    IM_YUV_TO_RGB_BT709_LIMIT     = 3 << 0,
    IM_YUV_TO_RGB_MASK            = 3 << 0,
    IM_RGB_TO_YUV_BT601_LIMIT     = 1 << 4,
    IM_RGB_TO_YUV_BT601_FULL      = 2 << 4,
    IM_RGB_TO_YUV_BT709_LIMIT     = 3 << 4,
    IM_RGB_TO_YUV_MASK            = 3 << 4,
    IM_COLOR_SPACE_DEFAULT        = 0,
} IM_COLOR_SPACE_MODE;

typedef enum {
    IM_UP_SCALE,
    IM_DOWN_SCALE,
} IM_SCALE;

typedef enum {
    INTER_NEAREST,
    INTER_LINEAR,
    INTER_CUBIC,
} IM_SCALE_MODE;

/* Get RGA basic information index */
typedef enum {
    RGA_VENDOR = 0,
    RGA_VERSION,
    RGA_MAX_INPUT,
    RGA_MAX_OUTPUT,
    RGA_SCALE_LIMIT,
    RGA_INPUT_FORMAT,
    RGA_OUTPUT_FORMAT,
    RGA_ALL,
} IM_INFORMATION;

/*rga version index*/
typedef enum {
    RGA_V_ERR                  = 0x0,
    RGA_1                      = 0x1,
    RGA_1_PLUS                 = 0x2,
    RGA_2                      = 0x3,
    RGA_2_LITE0                = 0x4,
    RGA_2_LITE1                = 0x5,
    RGA_2_ENHANCE              = 0x6,
} RGA_VERSION_NUM;

//struct AHardwareBuffer AHardwareBuffer;

/* Rectangle definition */
typedef struct {
    int x = 0;        /* upper-left x */
    int y = 0;        /* upper-left y */
    int width = 0;    /* width */
    int height = 0;   /* height */
} im_rect;

/* im_info definition */
typedef struct {
    void* vir_addr;                     /* virtual address */
    void* phy_addr;                     /* physical address */
    int fd;                             /* shared fd */
    int width;                          /* width */
    int height;                         /* height */
    int wstride;                        /* wstride */
    int hstride;                        /* hstride */
    int format;                         /* format */
    int color_space_mode;               /* color_space_mode */
    int color;                          /* color, used by color fill */
    int global_alpha;                   /* global_alpha */
} rga_buffer_t;

typedef struct rga_nn {
    int nn_flag;                /* enable nn */
    int scale_r;                /* scaling factor on R channal */
    int scale_g;                /* scaling factor on G channal */
    int scale_b;                /* scaling factor on B channal */
    int offset_r;               /* offset on R channal */
    int offset_g;               /* offset on G channal */
    int offset_b;               /* offset on B channal */
} rga_nn_t;

/*
 * @return rga_buffer_t
 */
#define wrapbuffer_viraddr_i(vir_addr, width, height, format) wrapbuffer_virtualaddr(vir_addr, width, height, width, height, format)
#define wrapbuffer_phyaddr_i(phy_addr, width, height, format) wrapbuffer_physicaladdr(phy_addr, width, height, width, height, format)
#define wrapbuffer_fd_i(fd, width, height, format) wrapbuffer_fd(fd, width, height, width, height, format)

IM_API rga_buffer_t wrapbuffer_virtualaddr(void* vir_addr, int width, int height, int wstride, int hstride, int format);
IM_API rga_buffer_t wrapbuffer_physicaladdr(void* phy_addr, int width, int height, int wstride, int hstride, int format);
IM_API rga_buffer_t wrapbuffer_fd(int fd, int width, int height, int wstride, int hstride, int format);

/*
 * Query RGA basic information, supported resolution, supported format, etc.
 *
 * @param name
 *      RGA_VENDOR
 *      RGA_VERSION
 *      RGA_MAX_INPUT
 *      RGA_MAX_OUTPUT
 *      RGA_INPUT_FORMAT
 *      RGA_OUTPUT_FORMAT
 *      RGA_ALL
 *
 * @returns a string describing properties of RGA.
 */
IM_API const char* querystring(int name);

/*
 * Resize
 *
 * @param src
 * @param dst
 * @param fx
 * @param fy
 * @param interpolation
 * @param sync
 *      wait until operation complete
 *
 * @returns success or else negative error code.
 */
#define imresize(src, dst) imresize_t(src, dst, 0, 0, INTER_LINEAR, 1)
#define impyramid(src, dst, direction) \
        imresize_t(src, \
                   dst, \
                   direction == IM_UP_SCALE ? 0.5 : 2, \
                   direction == IM_UP_SCALE ? 0.5 : 2, \
                   INTER_LINEAR, 1)

IM_API IM_STATUS imresize_t(const rga_buffer_t src, rga_buffer_t dst, double fx, double fy, int interpolation, int sync);

/*
 * Crop
 *
 * @param src
 * @param dst
 * @param rect
 * @param sync
 *      wait until operation complete
 *
 * @returns success or else negative error code.
 */
#define imcrop(src, dst, rect) imcrop_t(src, dst, rect, 1)

IM_API IM_STATUS imcrop_t(const rga_buffer_t src, rga_buffer_t dst, im_rect rect, int sync);

/*
 * rotation
 *
 * @param src
 * @param dst
 * @param rotation
 *      IM_HAL_TRANSFORM_ROT_90
 *      IM_HAL_TRANSFORM_ROT_180
 *      IM_HAL_TRANSFORM_ROT_270
 * @param sync
 *      wait until operation complete
 *
 * @returns success or else negative error code.
 */
#define imrotate(src, dst, rotation) imrotate_t(src, dst, rotation, 1)

IM_API IM_STATUS imrotate_t(const rga_buffer_t src, rga_buffer_t dst, int rotation, int sync);

/*
 * flip
 *
 * @param src
 * @param dst
 * @param mode
 *      IM_HAL_TRANSFORM_FLIP_H
 *      IM_HAL_TRANSFORM_FLIP_V
 * @param sync
 *      wait until operation complete
 *
 * @returns success or else negative error code.
 */
#define imflip(src, dst, mode) imflip_t(src, dst, mode, 1)

IM_API IM_STATUS imflip_t (const rga_buffer_t src, rga_buffer_t dst, int mode, int sync);

/*
 * fill/reset/draw
 *
 * @param src
 * @param dst
 * @param rect
 * @param color
 * @param sync
 *      wait until operation complete
 *
 * @returns success or else negative error code.
 */
#define imfill(dst, rect, color) imfill_t(dst, rect, color, 1)
#define imreset(dst, rect, color) imfill_t(dst, rect, color, 1)
#define imdraw(dst, rect, color) imfill_t(dst, rect, color, 1)

IM_API IM_STATUS imfill_t(rga_buffer_t dst, im_rect rect, int color, int sync);

/*
 * translate
 *
 * @param src
 * @param dst
 * @param x
 * @param y
 * @param sync
 *      wait until operation complete
 *
 * @returns success or else negative error code.
 */
#define imtranslate(src, dst, x, y) imtranslate_t(src, dst, x, y, 1)

IM_API IM_STATUS imtranslate_t(const rga_buffer_t src, rga_buffer_t dst, int x, int y, int sync);

/*
 * copy
 *
 * @param src
 * @param dst
 * @param sync
 *      wait until operation complete
 *
 * @returns success or else negative error code.
 */
#define imcopy(src, dst) imcopy_t(src, dst, 1)

IM_API IM_STATUS imcopy_t(const rga_buffer_t src, rga_buffer_t dst, int sync);

/*
 * blend (SRC + DST -> DST or SRCA + SRCB -> DST)
 *
 * @param srcA
 * @param srcB can be NULL.
 * @param dst
 * @param mode
 *      IM_ALPHA_BLEND_MODE
 * @param sync
 *      wait until operation complete
 *
 * @returns success or else negative error code.
 */
#define imblend(srcA, srcB, dst) imblend_t(srcA, srcB, dst, IM_ALPHA_BLEND_DST, 1)

IM_API IM_STATUS imblend_t(const rga_buffer_t srcA, const rga_buffer_t srcB, rga_buffer_t dst, int mode, int sync);

/*
 * format convert
 *
 * @param src
 * @param dst
 * @param sfmt
 * @param dfmt
 * @param mode
 *      color space mode: IM_COLOR_SPACE_MODE
 * @param sync
 *      wait until operation complete
 *
 * @returns success or else negative error code.
 */
#define imcvtcolor(src, dst, sfmt, dfmt) imcvtcolor_t(src, dst, sfmt, dfmt, IM_COLOR_SPACE_DEFAULT, 1)

IM_API IM_STATUS imcvtcolor_t(rga_buffer_t src, rga_buffer_t dst, int sfmt, int dfmt, int mode, int sync);

/*
 * nn quantize
 *
 * @param src
 * @param dst
 * @param nninfo
 * @param sync
 *      wait until operation complete
 *
 * @returns success or else negative error code.
 */
#define imquantize(src, dst, nn_info) imquantize_t(src, dst, nn_info, 1)

IM_API IM_STATUS imquantize_t(const rga_buffer_t src, rga_buffer_t dst, rga_nn_t nn_info, int sync);

/*
 * process
 *
 * @param src
 * @param dst
 * @param usage
 * @param ...
 *      wait until operation complete
 *
 * @returns success or else negative error code.
 */
IM_API IM_STATUS improcess(rga_buffer_t src, rga_buffer_t dst, im_rect srect, im_rect drect, int usage);

/*
 * block until all execution is complete
 *
 * @returns success or else negative error code.
 */
IM_API IM_STATUS imsync(void);

#ifdef __cplusplus
}
#endif
