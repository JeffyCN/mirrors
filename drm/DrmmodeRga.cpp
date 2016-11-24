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

#include "DrmmodeRga.h"
#include "../RgaApi.h"

volatile int32_t refCount = 0;
struct rgaContext *rgaCtx = NULL;

int RgaBlit(rga_info *src, rga_info *dst, rga_info *src1)
{
	struct rgaContext *ctx = rgaCtx;
	struct rga_image srcImage, dstImage, src1Image;
	rga_rect_t relSrcRect,tmpSrcRect,relDstRect,tmpDstRect;
	rga_rect_t relSrc1Rect,tmpSrc1Rect;
	int srcFd,dstFd,src1Fd;
	int degree = 0;
	int ret = 0;
	int planeAlpha;
	int rotation = 0;
	unsigned int blend;
    bool perpixelAlpha;
    drm_rga_t *pRects = NULL;
    drm_rga_t rects;

	if (!ctx) {
		ALOGE("Try to use uninit rgaCtx=%p",ctx);
		return -ENODEV;
	}

    if (!src && !dst && !src1) {
        ALOGE("src = %p, dst = %p, src1 = %p", src, dst, src1);
        return -EINVAL;
    }

    if (!src && !dst) {
        ALOGE("src = %p, dst = %p", src, dst);
        return -EINVAL;
    }

    if (src) {
        rotation = src->rotation;
        blend = src->blend;
        memcpy(&relSrcRect, &src->rect, sizeof(rga_rect_t));
    }
    if (dst)
        memcpy(&relDstRect, &dst->rect, sizeof(rga_rect_t));
    if (src1)
        memcpy(&relSrc1Rect, &src1->rect, sizeof(rga_rect_t));

    srcFd = dstFd = src1Fd = -1;
    if (src && src->hnd) {
    	ret = RkRgaGetHandleFd(src->hnd, &srcFd);
    	if (ret) {
    		ALOGE("dst handle get fd fail ret = %d,hnd=%p", ret, src->hnd);
    		printf("-dst handle get fd fail ret = %d,hnd=%p", ret, src->hnd);
    		return ret;
    	} else if (srcFd < 0)
		ALOGE("dst handle get srcFd = %d,hnd=%p", srcFd, src->hnd);
    	if (!isRectValid(relSrcRect)) {
    	    ret = getRgaRect(src->hnd, &tmpSrcRect);
    	    if (ret)
    	        return ret;
            memcpy(&relSrcRect, &tmpSrcRect, sizeof(rga_rect_t));
    	}
    }

    if (dst && dst->hnd) {
    	ret = RkRgaGetHandleFd(dst->hnd, &dstFd);
    	if (ret) {
    		ALOGE("dst handle get fd fail ret = %d,hnd=%p", ret, &dst->hnd);
    		printf("-dst handle get fd fail ret = %d,hnd=%p", ret, &dst->hnd);
    		return ret;
    	}
    	if (!isRectValid(relDstRect)) {
    	    ret = getRgaRect(dst->hnd, &tmpDstRect);
    	    if (ret)
    	        return ret;
            memcpy(&relDstRect, &tmpDstRect, sizeof(rga_rect_t));
    	}
    }

    if (src1 && src1->hnd) {
    	ret = RkRgaGetHandleFd(src1->hnd, &src1Fd);
    	if (ret) {
    		ALOGE("dst handle get fd fail ret = %d,hnd=%p", ret, &src1->hnd);
    		printf("-dst handle get fd fail ret = %d,hnd=%p", ret, &src1->hnd);
    		return ret;
    	}
    	if (!isRectValid(relSrcRect)) {
    	    ret = getRgaRect(src1->hnd, &tmpSrc1Rect);
    	    if (ret)
    	        return ret;
            memcpy(&relSrc1Rect, &tmpSrc1Rect, sizeof(rga_rect_t));
    	}
    }

    if (src && srcFd < 0)
        srcFd = src->fd;

    if (dst && dstFd < 0)
        dstFd = dst->fd;

    if (src1 && src1Fd < 0)
         src1Fd = src1->fd;
    
    if (src && srcFd < 0) {
        ALOGE("Has not support without src fd now");
        return -EINVAL;
    }

    if (dst && dstFd < 0) {
        ALOGE("Has not support without dst fd now");
        return -EINVAL;
    }

    if (src1 && src1Fd < 0) {
        ALOGE("Has not support without src fd now");
        return -EINVAL;
    }
	//check rects
	//check buffer_handle_t with rects

	memset(&srcImage, 0, sizeof(struct rga_image));
	memset(&dstImage, 0, sizeof(struct rga_image));
	memset(&src1Image, 0, sizeof(struct rga_image));

    planeAlpha = (blend & 0xFF0000) >> 16;
    perpixelAlpha = relSrcRect.format == HAL_PIXEL_FORMAT_RGBA_8888 ||
                        relSrcRect.format == HAL_PIXEL_FORMAT_BGRA_8888;

	dstImage.bo[0] = dstFd;
	srcImage.bo[0] = srcFd;
	src1Image.bo[0] = src1Fd;

	srcImage.buf_type = RGA_IMGBUF_GEM;
	srcImage.width = relSrcRect.width;
	srcImage.height = relSrcRect.height;
	srcImage.stride = computeRgaStrideByAndroidFormat(relSrcRect.wstride,
	                                                  relSrcRect.format);
	srcImage.color_mode = getDrmFomatFromAndroidFormat(relSrcRect.format);

	if (!srcImage.color_mode) {
		ALOGE("bad format : %d",srcImage.color_mode);
		return -EINVAL;
	}

	dstImage.buf_type = RGA_IMGBUF_GEM;
	dstImage.width = relDstRect.width;
	dstImage.height = relDstRect.height;
	dstImage.stride = computeRgaStrideByAndroidFormat(relDstRect.wstride,
	                                                  relDstRect.format);
	dstImage.color_mode = getDrmFomatFromAndroidFormat(relDstRect.format);

	if (!dstImage.color_mode) {
		ALOGE("bad format : %d",dstImage.color_mode);
		return -EINVAL;
	}

    if (src1) {
    	src1Image.buf_type = RGA_IMGBUF_GEM;
    	src1Image.width = relSrc1Rect.width;
    	src1Image.height = relSrc1Rect.height;
    	src1Image.stride = computeRgaStrideByAndroidFormat(relSrc1Rect.wstride,
    	                                                  relSrc1Rect.format);
    	src1Image.color_mode = getDrmFomatFromAndroidFormat(relSrc1Rect.format);

    	if (!src1Image.color_mode) {
    		ALOGE("bad format : %d",src1Image.color_mode);
    		return -EINVAL;
    	}
	}

	if ((rotation & DRM_RGA_TRANSFORM_ROT_MASK) == DRM_RGA_TRANSFORM_ROT_90)
		degree = 90;

	if ((rotation & DRM_RGA_TRANSFORM_ROT_MASK) == DRM_RGA_TRANSFORM_ROT_180)
		degree = 180;

	if ((rotation & DRM_RGA_TRANSFORM_ROT_MASK) == DRM_RGA_TRANSFORM_ROT_270)
		degree = 270;

	printf("[0x%x]src[%d,%d]+[%d,%d],dst[%d,%d]+[%d,%d]\n",rotation,
                                relSrcRect.xoffset,relSrcRect.yoffset,
                                relSrcRect.width,  relSrcRect.height,
                                relDstRect.xoffset,relDstRect.yoffset,
                                relDstRect.width,  relDstRect.height);

	rotation &= DRM_RGA_TRANSFORM_FLIP_MASK;

	fprintf(stderr, "ctx=%p,ctx->ctx=%p\n",ctx,ctx->ctx);
	ctx->ctx->log = 0;
	rga_multiple_transform(ctx->ctx, &srcImage, &dstImage,
                                relSrcRect.xoffset, relSrcRect.yoffset,
                                relSrcRect.width,   relSrcRect.height,
                                relDstRect.xoffset, relDstRect.yoffset,
                                relDstRect.width,   relDstRect.height,
                                degree, rotation & DRM_RGA_TRANSFORM_FLIP_H,
                                        rotation & DRM_RGA_TRANSFORM_FLIP_V);

	ret = rga_exec(ctx->ctx);
	if (ret < 0) {
		ALOGE("Handle by rga error");
		return ret;
	}
	return 0;
}

int drmRgaOpen(void **context)
{

	struct rockchip_device *dev = NULL;
	struct rgaContext *ctx = NULL;
	int fd = -1;
	int ret = 0;

    if (!context) {
        ret = -EINVAL;
        goto mallocErr;
    }

	if (!rgaCtx) {
		ctx = (struct rgaContext *)malloc(sizeof(struct rgaContext));
		if(!ctx) {
			ret = -ENOMEM;
			ALOGE("malloc fail:%s.",strerror(errno));
			goto mallocErr;
		}
	} else {
		ctx = rgaCtx;
		ALOGW("Had init the rga dev ctx = %p",ctx);
		goto init;
	}

	//fd = drmOpen(DRM_MODULE_NAME, NULL);
	fd = open("/dev/dri/renderD128", O_RDWR);
	if (fd < 0) {
		ret = -ENODEV;
		ALOGE("failed to open DRM:%s.",strerror(errno));
		goto drmOpenErr;
	}

	dev = rockchip_device_create(fd);
	if (!dev) {
		ret = -ENODEV;
		ALOGE("failed to create DRM device:%s.",strerror(errno));
		goto devCreateErr;
	}

	ctx->ctx = rga_init(dev->fd);
	if (!ctx->ctx) {
		ret = -ENODEV;
		ALOGE("failed to init rga dev:%s.",strerror(errno));
		goto rgaInitErr;
	}

	fprintf(stderr, "ctx=%p,ctx->ctx=%p\n",ctx,ctx->ctx);
	rgaCtx = ctx;

init:
	android_atomic_inc(&refCount);
	*context = (void *)ctx;
	return ret;

loadModErr:
rgaInitErr:
	rockchip_device_destroy(dev);
devCreateErr:
	drmClose(fd);
drmOpenErr:
	free(ctx);
mallocErr:
	return ret;
}

int drmRgaClose(void *context)
{
	struct rgaContext *ctx = rgaCtx;

	if (!ctx) {
		ALOGE("Try to exit uninit rgaCtx=%p", ctx);
		return -ENODEV;
	}

    if (context) {
        ALOGE("Try to uninit rgaCtx=%p", context);
		return -ENODEV;
    }
    
	if (context != ctx) {
		ALOGE("Try to exit wrong ctx=%p",ctx);
		return -ENODEV;
	}

	if (refCount <= 0) {
		ALOGW("This can not be happened");
		return 0;
	}

	if (refCount > 0 && android_atomic_dec(&refCount) != 1)
		return 0;

	rgaCtx = NULL;

	rga_fini(ctx->ctx);

	drmClose(ctx->dev->fd);
	rockchip_device_destroy(ctx->dev);

	free(ctx);

	return 0;
}

int RgaInit(void **ctx)
{
    int ret = 0;
    ret = drmRgaOpen(ctx);
    return ret;
}

int RgaDeInit(void *ctx)
{
    int ret = 0;
    ret = drmRgaClose(ctx);
    return ret;
}

static int dumpRgaRects(drm_rga_t tmpRects)
{
	ALOGD("Src:[%d,%d,%d,%d][%d,%d,%d]=>Dst:[%d,%d,%d,%d][%d,%d,%d]",
			tmpRects.src.xoffset,tmpRects.src.yoffset,
			tmpRects.src.width, tmpRects.src.height, 
			tmpRects.src.wstride,tmpRects.src.format, tmpRects.src.size,
			tmpRects.dst.xoffset,tmpRects.dst.yoffset,
			tmpRects.dst.width, tmpRects.dst.height,
			tmpRects.dst.wstride,tmpRects.dst.format, tmpRects.dst.size);
	return 0;
}

static int isRectValid(rga_rect_t rect)
{
    return rect.width > 0 && rect.height > 0;
}

static int getRgaRect(buffer_handle_t hnd, rga_rect_t *rect)
{
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

    if (dstAttrs.size() <= 0) {
        ALOGE("SIZE = %d", dstAttrs.size());
        return -EINVAL;
    }

	memset(rect, 0, sizeof(rga_rect_t));

	rect->size = dstAttrs.at(ASIZE);
	rect->width   = dstAttrs.at(AWIDTH);
	rect->height  = dstAttrs.at(AHEIGHT);
	rect->wstride = dstAttrs.at(ASTRIDE);
	rect->format  = dstAttrs.at(AFORMAT);

	return ret;
}

static int getRgaRects(buffer_handle_t src,
                                buffer_handle_t dst, drm_rga_t* tmpRects)
{
	int ret = 0;
	std::vector<int> srcAttrs,dstAttrs;
	ret = RkRgaGetHandleAttributes(src, &srcAttrs);
	if (ret) {
		ALOGE("dst handle get Attributes fail ret = %d,hnd=%p",ret,&src);
		printf("dst handle get Attributes fail ret = %d,hnd=%p",ret,&src);
		return ret;
	}
	ret = RkRgaGetHandleAttributes(dst, &dstAttrs);
	if (ret) {
		ALOGE("dst handle get Attributes fail ret = %d,hnd=%p",ret,&dst);
		printf("dst handle get Attributes fail ret = %d,hnd=%p",ret,&dst);
		return ret;
	}

	memset(tmpRects,0,sizeof(drm_rga_t));

	tmpRects->src.size = srcAttrs.at(ASIZE);
	tmpRects->src.width   = srcAttrs.at(AWIDTH);
	tmpRects->src.height  = srcAttrs.at(AHEIGHT);
	tmpRects->src.wstride = srcAttrs.at(ASTRIDE);
	tmpRects->src.format  = srcAttrs.at(AFORMAT);

	tmpRects->dst.size = dstAttrs.at(ASIZE);
	tmpRects->dst.width   = dstAttrs.at(AWIDTH);
	tmpRects->dst.height  = dstAttrs.at(AHEIGHT);
	tmpRects->dst.wstride = dstAttrs.at(ASTRIDE);
	tmpRects->dst.format  = dstAttrs.at(AFORMAT);

	return ret;
}

static int getDrmFomatFromAndroidFormat(int format)
{
	int ret = 0;

	switch (format) {
		case HAL_PIXEL_FORMAT_RGBA_8888:
			ret = DRM_FORMAT_RGBA8888;
			break;

		case HAL_PIXEL_FORMAT_RGBX_8888:
			ret = DRM_FORMAT_RGBX8888;
			break;

		case HAL_PIXEL_FORMAT_RGB_888:
			ret = DRM_FORMAT_RGB888;
			break;

		case HAL_PIXEL_FORMAT_RGB_565:
			ret = DRM_FORMAT_RGB565;
			break;

		case HAL_PIXEL_FORMAT_BGRA_8888:
			ret = DRM_FORMAT_BGRA8888;
			break;

		case HAL_PIXEL_FORMAT_sRGB_A_8888:
			//ret = what?;
			break;

		case HAL_PIXEL_FORMAT_sRGB_X_8888:
			//ret = what?;
			break;

		case HAL_PIXEL_FORMAT_YCbCr_422_SP:
			ret = DRM_FORMAT_NV16;
			break;

		case HAL_PIXEL_FORMAT_YCrCb_420_SP:
			ret = DRM_FORMAT_NV21;
			break;

		case HAL_PIXEL_FORMAT_YCbCr_422_I:
			//ret = DRM_FORMAT_RGBX8888;
			break;

		case HAL_PIXEL_FORMAT_YCrCb_NV12:
			ret = DRM_FORMAT_NV12;
			break;

		case HAL_PIXEL_FORMAT_YCrCb_NV12_VIDEO:
			ret = DRM_FORMAT_NV12;
			break;

		case HAL_PIXEL_FORMAT_YCrCb_NV12_10:
			//ret = DRM_FORMAT_RGBX8888;
			break;

		case HAL_PIXEL_FORMAT_YCbCr_422_SP_10:
			//ret = DRM_FORMAT_RGBX8888;
			break;


		case HAL_PIXEL_FORMAT_YCrCb_420_SP_10:
			//ret = DRM_FORMAT_RGBX8888;
			break;

		default:
			break;
	}
	return ret;
}

static int getPixelWidthByDrmFormat(int format)
{
	return format;
}

static int getPixelWidthByAndroidFormat(int format)
{
	int pixelWidth = -1;
	switch (format) {
		case HAL_PIXEL_FORMAT_RGBA_8888:
		case HAL_PIXEL_FORMAT_RGBX_8888:
		case HAL_PIXEL_FORMAT_BGRA_8888:
			pixelWidth = 4;
			break;

		case HAL_PIXEL_FORMAT_RGB_888:
			pixelWidth = 3;
			break;

		case HAL_PIXEL_FORMAT_RGB_565:
			pixelWidth = 2;
			break;

		case HAL_PIXEL_FORMAT_sRGB_A_8888:
		case HAL_PIXEL_FORMAT_sRGB_X_8888:
			ALOGE("format 0x%x not support",format);
			break;

		case HAL_PIXEL_FORMAT_YCbCr_422_SP:
		case HAL_PIXEL_FORMAT_YCrCb_420_SP:
		case HAL_PIXEL_FORMAT_YCbCr_422_I:
		case HAL_PIXEL_FORMAT_YCrCb_NV12:
		case HAL_PIXEL_FORMAT_YCrCb_NV12_VIDEO:
			pixelWidth = 1;
			break;

		case HAL_PIXEL_FORMAT_YCrCb_NV12_10:
		case HAL_PIXEL_FORMAT_YCbCr_422_SP_10:
		case HAL_PIXEL_FORMAT_YCrCb_420_SP_10:
			pixelWidth = 1;
			break;

		default:
			ALOGE("format 0x%x not support",format);
			break;
	}
	return pixelWidth;
}

static int computeRgaStrideByDrmFormat(int stride, int format)
{
	return stride * format;
}

static int computeRgaStrideByAndroidFormat(int stride, int format)
{
	int pixelWidth = 0;
	int widthStride = 0;

	pixelWidth = getPixelWidthByAndroidFormat(format);

	if (pixelWidth < 0)
		return -EINVAL;

	widthStride = stride * pixelWidth;

	return widthStride;
}
