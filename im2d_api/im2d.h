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

#ifdef __cplusplus
extern "C" {
#endif

#if 1 //ANDROID
#include <hardware/rga.h>

//#include <hardware_buffer_jni.h>
#else

#endif

#ifndef IM_API
#define IM_API /* define API export as needed */
#endif

/* Additional blend modes, can be used with both source and target configs.
   If none of the below is set, the default "SRC over DST" is applied. */
typedef enum {
    IM_ALPHA_BLEND_SRC_OVER,     /* Default, Porter-Duff "SRC over DST" */
    IM_ALPHA_BLEND_SRC,          /* Porter-Duff "SRC" */
    IM_ALPHA_BLEND_SRC_IN,       /* Porter-Duff "SRC in DST" */
    IM_ALPHA_BLEND_DST_IN,       /* Porter-Duff "DST in SRC" */
    IM_ALPHA_BLEND_SRC_OUT,      /* Porter-Duff "SRC out DST" */
    IM_ALPHA_BLEND_DST_OUT,      /* Porter-Duff "DST out SRC" */
    IM_ALPHA_BLEND_DST_OVER,     /* Porter-Duff "DST over SRC" */
    IM_ALPHA_BLEND_SRC_ATOP,     /* Porter-Duff "SRC ATOP" */
    IM_ALPHA_BLEND_DST_ATOP,     /* Porter-Duff "DST ATOP" */
    IM_ALPHA_BLEND_XOR,          /* Xor */
    IM_ALPHA_BLEND_NONE,         /* disables alpha blending */
} IM_ALPHA_BLEND_MODE;

/* Status codes, returned by any blit function */
typedef enum {
    IM_STATUS_SUCCESS         = 1,
    IM_STATUS_NOT_SUPPORTED   = -1,
    IM_STATUS_OUT_OF_MEMORY   = -2,
    IM_STATUS_INVALID_PARAM   = -3,
    IM_STATUS_FAILED          = 0,
} IM_STATUS;

/* Status codes, returned by any blit function */
typedef enum {
    IM_YUV_TO_RGB_BY_PASS         = 0 << 0,
    IM_YUV_TO_RGB_BT601_LIMIT     = 1 << 0,
    IM_YUV_TO_RGB_BT601_FULL      = 2 << 0,
    IM_YUV_TO_RGB_BT709_LIMIT     = 3 << 0,
    IM_RGB_TO_YUV_BY_PASS         = 0 << 4,
    IM_RGB_TO_YUV_BT601_LIMIT     = 1 << 4,
    IM_RGB_TO_YUV_BT601_FULL      = 2 << 4,
    IM_RGB_TO_YUV_BT709_LIMIT     = 3 << 4,
    IM_COLOR_SPACE_DEFAULT        = -1,
} IM_COLOR_SPACE_MODE;

//struct AHardwareBuffer AHardwareBuffer;

/* Rectangle definition */
typedef struct {
    int x;        /* upper-left x */
    int y;        /* upper-left y */
    int width;    /* width */
    int height;   /* height */
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
} buffer_t;

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
 * @return buffer_t
 */
IM_API buffer_t warpbuffer_virtualaddr(void* vir_addr, int width, int height, int wstride, int hstride, int format);
IM_API buffer_t warpbuffer_physicaladdr(void* phy_addr, int width, int height, int wstride, int hstride, int format);
IM_API buffer_t warpbuffer_fd(int fd, int width, int height, int wstride, int hstride, int format);

#if 0 //Android
IM_API buffer_t* warpbuffer_GraphicBuffer(sp<GraphicBuffer> buf);
IM_API buffer_t* warpbuffer_AHardwareBuffer(AHardwareBuffer buf);
#endif

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
#define impyramid(src, dst) imresize_t(src, dst, 0.5, 0.5, INTER_LINEAR, 1)

IM_API IM_STATUS imresize_t(const buffer_t src, buffer_t dst, double fx, double fy, int interpolation, int sync);

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

IM_API IM_STATUS imcrop_t(const buffer_t src, buffer_t dst, im_rect rect, int sync);

/*
 * rotation
 *
 * @param src
 * @param dst
 * @param rotation
 *      HAL_TRANSFORM_ROT_90
 *      HAL_TRANSFORM_ROT_180
 *      HAL_TRANSFORM_ROT_270
 * @param sync
 *      wait until operation complete
 *
 * @returns success or else negative error code.
 */
#define imrotate(src, dst, rotation) imrotate_t(src, dst, rotation, 1)

IM_API IM_STATUS imrotate_t(const buffer_t src, buffer_t dst, int rotation, int sync);

/*
 * flip
 *
 * @param src
 * @param dst
 * @param mode
 *      HAL_TRANSFORM_FLIP_H
 *      HAL_TRANSFORM_FLIP_V
 * @param sync
 *      wait until operation complete
 *
 * @returns success or else negative error code.
 */
#define imflip(src, dst, mode) imflip_t(src, dst, mode, 1)

IM_API IM_STATUS imflip_t (const buffer_t src, buffer_t dst, int mode, int sync);

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

IM_API IM_STATUS imfill_t(buffer_t dst, im_rect rect, unsigned char color, int sync);

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

IM_API IM_STATUS imtranslate_t(const buffer_t src, buffer_t dst, int x, int y, int sync);

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

IM_API IM_STATUS imcopy_t(const buffer_t src, buffer_t dst, int sync);

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
#define imblend(srcA, srcB, dst) imblend_t(srcA, srcB, dst, RGA_ALPHA_BLEND_SRC_OVER, 1)

IM_API IM_STATUS imblend_t(const buffer_t srcA, const buffer_t srcB, buffer_t dst, int mode, int sync);

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

IM_API IM_STATUS imcvtcolor_t(const buffer_t src, buffer_t dst, int sfmt, int dfmt, int mode, int sync);

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

IM_API IM_STATUS imquantize_t(const buffer_t src, buffer_t dst, rga_nn_t nn_info, int sync);

/*
 * process
 *
 * @param src
 * @param dst
 * @param sync
 * @param ...
 *      wait until operation complete
 *
 * @returns success or else negative error code.
 */
#define improcess(src, dst, ...) improcess_t(src, dst, 1, ...)

IM_API IM_STATUS improcess_t(const buffer_t src, buffer_t dst, int sync, ...);

/*
 * block until all execution is complete
 *
 * @returns success or else negative error code.
 */
IM_API IM_STATUS imsync(void);

#ifdef __cplusplus
}
#endif