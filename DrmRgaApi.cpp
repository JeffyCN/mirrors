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

#include "DrmRgaApi.h"

android::Mutex mMutex;
volatile int32_t refCount = 0;
struct rgaContext *rgaCtx = NULL;

int rgaCopy(struct rga_device *dev,
               buffer_handle_t src,buffer_handle_t dst,drm_rga_t* rects)
{
    android::Mutex::Autolock lock(mMutex);
    struct rgaContext *ctx = rgaCtx;
    drm_rga_t tmpRects,relRects;
    int ret = 0;

    if (!ctx) {
        ALOGE("Try to use uninit rgaCtx=%p",ctx);
        return -ENODEV;
    }

    if (dev != &ctx->device) {
        ALOGE("Try to use wrong device ctx=%p",ctx);
        return -ENODEV;
    }

    ret = getRgaRects(ctx,src,dst,&tmpRects);
    if (ret)
       return ret;

    if (rects)
        memcpy(&relRects,rects,sizeof(drm_rga_t));
    else
        memcpy(&relRects,&tmpRects,sizeof(drm_rga_t));

    //check rects
    //check buffer_handle_t with rects

	struct rga_image srcImage, dstImage;
	int dstFd, srcFd;

    memset(&srcImage,0,sizeof(struct rga_image));
    memset(&dstImage,0,sizeof(struct rga_image));

    ret = getHandlePrimeFd(ctx, src, &srcFd);
    if (ret) {
        ALOGE("dst handle get fd fail ret = %d,hnd=%p",ret,&src);
        printf("-dst handle get fd fail ret = %d,hnd=%p",ret,&src);
        return ret;
    }

    ret = getHandlePrimeFd(ctx, dst, &dstFd);
    if (ret) {
        ALOGE("dst handle get fd fail ret = %d,hnd=%p",ret,&dst);
        printf("-dst handle get fd fail ret = %d,hnd=%p",ret,&dst);
        return ret;
    }
    ALOGD("dst handle get fd  = %d",dstFd);

	dstImage.bo[0] = dstFd;
	srcImage.bo[0] = srcFd;

	srcImage.buf_type = RGA_IMGBUF_GEM;
	srcImage.width = relRects.src.width;
	srcImage.height = relRects.src.height;
	srcImage.stride = computeRgaStrideByAndroidFormat(relRects.src.wstride,
	                                                  relRects.src.format);
	srcImage.color_mode = getDrmFomatFromAndroidFormat(relRects.src.format);


    if (!srcImage.color_mode) {
        ALOGE("bad format : %d",srcImage.color_mode);
        return -EINVAL;
    }

	dstImage.buf_type = RGA_IMGBUF_GEM;
	dstImage.width = relRects.dst.width;
	dstImage.height = relRects.dst.height;
	dstImage.stride = computeRgaStrideByAndroidFormat(relRects.dst.wstride,
	                                                  relRects.dst.format);
	dstImage.color_mode = getDrmFomatFromAndroidFormat(relRects.dst.format);

    if (!dstImage.color_mode < 0) {
        ALOGE("bad format : %d",dstImage.color_mode);
        return -EINVAL;
    }

    rga_copy(ctx->ctx, &srcImage, &dstImage,
        relRects.src.xoffset, relRects.src.yoffset,
        relRects.dst.xoffset,
        relRects.dst.yoffset, relRects.dst.width, relRects.dst.height);

	ret = rga_exec(ctx->ctx);
	if (ret < 0) {
	    ALOGE("Handle by rga error");
		return ret;
    }

	return 0;
}

int rgaFillColor(struct rga_device *dev,
                    buffer_handle_t handle,int data,drm_rga_t* rects)
{
    android::Mutex::Autolock lock(mMutex);
    struct rgaContext *ctx = rgaCtx;
    struct rga_image dstImage;
    
    drm_rga_t tmpRects,relRects;
    unsigned int img_w, img_h;
    int dstFd;
    int ret = 0;

    std::vector<int> attrs;

    if(!ctx) {
        ALOGE("Try to use uninit rgaCtx=%p",ctx);
        return -ENODEV;
    }

    if(dev != &ctx->device) {
        ALOGE("Try to use wrong device ctx=%p",ctx);
        return -ENODEV;
    }

    memset(&dstImage,0,sizeof(struct rga_image)); 

    ret = getHandlePrimeFd(ctx, handle, &dstFd);
    if (ret) {
        ALOGE("dst handle get fd fail ret = %d,hnd=%p",ret,&handle);
        printf("-dst handle get fd fail ret = %d,hnd=%p",ret,&handle);
        return ret;
    }
    ALOGD("dst handle get fd  = %d",dstFd);

    ret = getRgaRects(ctx,handle,&tmpRects);
    if (ret)
       return ret;

    if (rects)
        memcpy(&relRects,rects,sizeof(drm_rga_t));
    else
        memcpy(&relRects,&tmpRects,sizeof(drm_rga_t));

    if (true)
        dumpRgaRects(tmpRects);

    dstImage.bo[0] = dstFd;

	dstImage.buf_type = RGA_IMGBUF_GEM;
	dstImage.width = relRects.dst.width;
	dstImage.height = relRects.dst.height;
	dstImage.stride = computeRgaStrideByAndroidFormat(relRects.dst.wstride,
	                                                  relRects.dst.format);
	dstImage.color_mode = getDrmFomatFromAndroidFormat(relRects.dst.format);

    if (!dstImage.color_mode) {
        ALOGE("bad format : %d",dstImage.color_mode);
        return -EINVAL;
    }

    dstImage.fill_color = data;
    rga_solid_fill(ctx->ctx, &dstImage, 0, 0, dstImage.width, dstImage.height);
    ret = rga_exec(ctx->ctx);
    if (ret)
		return ret;

	return 0;
}

int rgaConvert(struct rga_device *dev,
               buffer_handle_t src,buffer_handle_t dst,drm_rga_t* rects)
{
    android::Mutex::Autolock lock(mMutex);
    struct rgaContext *ctx = rgaCtx;
    drm_rga_t tmpRects,relRects;
    int ret = 0;

    if (!ctx) {
        ALOGE("Try to use uninit rgaCtx=%p",ctx);
        return -ENODEV;
    }
    
    if (dev != &ctx->device) {
        ALOGE("Try to use wrong device ctx=%p",ctx);
        return -ENODEV;
    }

    ret = getRgaRects(ctx,src,dst,&tmpRects);
    if (ret)
       return ret;

    if (rects)
        memcpy(&relRects,rects,sizeof(drm_rga_t));
    else
        memcpy(&relRects,&tmpRects,sizeof(drm_rga_t));

    //check rects
    //check buffer_handle_t with rects

	struct rga_image srcImage, dstImage;
	int dstFd, srcFd;

    memset(&srcImage,0,sizeof(struct rga_image));
    memset(&dstImage,0,sizeof(struct rga_image));

    ret = getHandlePrimeFd(ctx, src, &srcFd);
    if (ret) {
        ALOGE("dst handle get fd fail ret = %d,hnd=%p",ret,&src);
        printf("-dst handle get fd fail ret = %d,hnd=%p",ret,&src);
        return ret;
    }

    ret = getHandlePrimeFd(ctx, dst, &dstFd);
    if (ret) {
        ALOGE("dst handle get fd fail ret = %d,hnd=%p",ret,&dst);
        printf("-dst handle get fd fail ret = %d,hnd=%p",ret,&dst);
        return ret;
    }
    ALOGD("dst handle get fd  = %d",dstFd);

	dstImage.bo[0] = dstFd;
	srcImage.bo[0] = srcFd;

	srcImage.buf_type = RGA_IMGBUF_GEM;
	srcImage.width = relRects.src.width;
	srcImage.height = relRects.src.height;
	srcImage.stride = computeRgaStrideByAndroidFormat(relRects.src.wstride,
	                                                  relRects.src.format);
	srcImage.color_mode = getDrmFomatFromAndroidFormat(relRects.src.format);

    if (!srcImage.color_mode) {
        ALOGE("bad format : %d",srcImage.color_mode);
        return -EINVAL;
    }


	dstImage.buf_type = RGA_IMGBUF_GEM;
	dstImage.width = relRects.dst.width;
	dstImage.height = relRects.dst.height;
	dstImage.stride = computeRgaStrideByAndroidFormat(relRects.dst.wstride,
	                                                  relRects.dst.format);
	dstImage.color_mode = getDrmFomatFromAndroidFormat(relRects.dst.format);

    if (!dstImage.color_mode < 0) {
        ALOGE("bad format : %d",dstImage.color_mode);
        return -EINVAL;
    }

    printf("src[%d,%d]+[%d,%d],dst[%d,%d]+[%d,%d]\n",
                                relRects.src.xoffset,relRects.src.yoffset,
                                relRects.src.width,  relRects.src.height,
                                relRects.dst.xoffset,relRects.dst.yoffset,
                                relRects.dst.width,  relRects.dst.height);

    rga_multiple_transform(ctx->ctx, &srcImage, &dstImage,
                            relRects.src.xoffset, relRects.src.yoffset,
                            relRects.src.width,   relRects.src.height,
                            relRects.dst.xoffset, relRects.dst.yoffset,
                            relRects.dst.width,   relRects.dst.height,
                            0,0,0/*degree,x_mirr,y_mirr*/);

	ret = rga_exec(ctx->ctx);
	if (ret < 0) {
	    ALOGE("Handle by rga error");
		return ret;
    }

	return 0;
}

int rgaScale(struct rga_device *dev,
               buffer_handle_t src,buffer_handle_t dst,drm_rga_t* rects)
{
    android::Mutex::Autolock lock(mMutex);
    struct rgaContext *ctx = rgaCtx;
    drm_rga_t tmpRects,relRects;
    int ret = 0;

    if (!ctx) {
        ALOGE("Try to use uninit rgaCtx=%p",ctx);
        return -ENODEV;
    }
    
    if (dev != &ctx->device) {
        ALOGE("Try to use wrong device ctx=%p",ctx);
        return -ENODEV;
    }

    ret = getRgaRects(ctx,src,dst,&tmpRects);
    if (ret)
       return ret;

    if (rects)
        memcpy(&relRects,rects,sizeof(drm_rga_t));
    else
        memcpy(&relRects,&tmpRects,sizeof(drm_rga_t));

    //check rects
    //check buffer_handle_t with rects

	struct rga_image srcImage, dstImage;
	int dstFd, srcFd;

    memset(&srcImage,0,sizeof(struct rga_image));
    memset(&dstImage,0,sizeof(struct rga_image));

    ret = getHandlePrimeFd(ctx, src, &srcFd);
    if (ret) {
        ALOGE("dst handle get fd fail ret = %d,hnd=%p",ret,&src);
        printf("-dst handle get fd fail ret = %d,hnd=%p",ret,&src);
        return ret;
    }

    ret = getHandlePrimeFd(ctx, dst, &dstFd);
    if (ret) {
        ALOGE("dst handle get fd fail ret = %d,hnd=%p",ret,&dst);
        printf("-dst handle get fd fail ret = %d,hnd=%p",ret,&dst);
        return ret;
    }
    ALOGD("dst handle get fd  = %d",dstFd);

	dstImage.bo[0] = dstFd;
	srcImage.bo[0] = srcFd;

	srcImage.buf_type = RGA_IMGBUF_GEM;
	srcImage.width = relRects.src.width;
	srcImage.height = relRects.src.height;
	srcImage.stride = computeRgaStrideByAndroidFormat(relRects.src.wstride,
	                                                  relRects.src.format);
	srcImage.color_mode = getDrmFomatFromAndroidFormat(relRects.src.format);

    if (!srcImage.color_mode) {
        ALOGE("bad format : %d",srcImage.color_mode);
        return -EINVAL;
    }

	dstImage.buf_type = RGA_IMGBUF_GEM;
	dstImage.width = relRects.dst.width;
	dstImage.height = relRects.dst.height;
	dstImage.stride = computeRgaStrideByAndroidFormat(relRects.dst.wstride,
	                                                  relRects.dst.format);
	dstImage.color_mode = getDrmFomatFromAndroidFormat(relRects.dst.format);

    if (!dstImage.color_mode < 0) {
        ALOGE("bad format : %d",dstImage.color_mode);
        return -EINVAL;
    }

    printf("src[%d,%d]+[%d,%d],dst[%d,%d]+[%d,%d]\n",
                                relRects.src.xoffset,relRects.src.yoffset,
                                relRects.src.width,  relRects.src.height,
                                relRects.dst.xoffset,relRects.dst.yoffset,
                                relRects.dst.width,  relRects.dst.height);

    fprintf(stderr, "ctx=%p,ctx->ctx=%p\n",ctx,ctx->ctx);
    rga_multiple_transform(ctx->ctx, &srcImage, &dstImage,
                                relRects.src.xoffset, relRects.src.yoffset,
                                relRects.src.width,   relRects.src.height,
                                relRects.dst.xoffset, relRects.dst.yoffset,
                                relRects.dst.width,   relRects.dst.height,
                                0,0,0/*degree,x_mirr,y_mirr*/);

	ret = rga_exec(ctx->ctx);
	if (ret < 0) {
	    ALOGE("Handle by rga error");
		return ret;
    }

	return 0;
}

int rgaRotateScale(struct rga_device *dev,
       buffer_handle_t src,buffer_handle_t dst,drm_rga_t* rects,int rotation)
{
    android::Mutex::Autolock lock(mMutex);
    struct rgaContext *ctx = rgaCtx;
    drm_rga_t tmpRects,relRects;
    int degree = 0;
    int ret = 0;

    if (!ctx) {
        ALOGE("Try to use uninit rgaCtx=%p",ctx);
        return -ENODEV;
    }
    
    if (dev != &ctx->device) {
        ALOGE("Try to use wrong device ctx=%p",ctx);
        return -ENODEV;
    }

    ret = getRgaRects(ctx,src,dst,&tmpRects);
    if (ret)
       return ret;

    if (rects)
        memcpy(&relRects,rects,sizeof(drm_rga_t));
    else
        memcpy(&relRects,&tmpRects,sizeof(drm_rga_t));

    //check rects
    //check buffer_handle_t with rects

	struct rga_image srcImage, dstImage;
	int dstFd, srcFd;

    memset(&srcImage,0,sizeof(struct rga_image));
    memset(&dstImage,0,sizeof(struct rga_image));

    ret = getHandlePrimeFd(ctx, src, &srcFd);
    if (ret) {
        ALOGE("dst handle get fd fail ret = %d,hnd=%p",ret,&src);
        printf("-dst handle get fd fail ret = %d,hnd=%p",ret,&src);
        return ret;
    }

    ret = getHandlePrimeFd(ctx, dst, &dstFd);
    if (ret) {
        ALOGE("dst handle get fd fail ret = %d,hnd=%p",ret,&dst);
        printf("-dst handle get fd fail ret = %d,hnd=%p",ret,&dst);
        return ret;
    }
    ALOGD("dst handle get fd  = %d",dstFd);

	dstImage.bo[0] = dstFd;
	srcImage.bo[0] = srcFd;

	srcImage.buf_type = RGA_IMGBUF_GEM;
	srcImage.width = relRects.src.width;
	srcImage.height = relRects.src.height;
	srcImage.stride = computeRgaStrideByAndroidFormat(relRects.src.wstride,
	                                                  relRects.src.format);
	srcImage.color_mode = getDrmFomatFromAndroidFormat(relRects.src.format);

    if (!srcImage.color_mode) {
        ALOGE("bad format : %d",srcImage.color_mode);
        return -EINVAL;
    }


	dstImage.buf_type = RGA_IMGBUF_GEM;
	dstImage.width = relRects.dst.width;
	dstImage.height = relRects.dst.height;
	dstImage.stride = computeRgaStrideByAndroidFormat(relRects.dst.wstride,
	                                                  relRects.dst.format);
	dstImage.color_mode = getDrmFomatFromAndroidFormat(relRects.dst.format);

    if (!dstImage.color_mode < 0) {
        ALOGE("bad format : %d",dstImage.color_mode);
        return -EINVAL;
    }

    if ((rotation & DRM_RGA_TRANSFORM_ROT_MASK) == DRM_RGA_TRANSFORM_ROT_90)
        degree = 90;

    if ((rotation & DRM_RGA_TRANSFORM_ROT_MASK) == DRM_RGA_TRANSFORM_ROT_180)
        degree = 180;

    if ((rotation & DRM_RGA_TRANSFORM_ROT_MASK) == DRM_RGA_TRANSFORM_ROT_270)
        degree = 270;

    printf("[0x%x]src[%d,%d]+[%d,%d],dst[%d,%d]+[%d,%d]\n",rotation,
                                relRects.src.xoffset,relRects.src.yoffset,
                                relRects.src.width,  relRects.src.height,
                                relRects.dst.xoffset,relRects.dst.yoffset,
                                relRects.dst.width,  relRects.dst.height);

    rotation &= DRM_RGA_TRANSFORM_FLIP_MASK;

    fprintf(stderr, "ctx=%p,ctx->ctx=%p\n",ctx,ctx->ctx);
    rga_multiple_transform(ctx->ctx, &srcImage, &dstImage,
                                relRects.src.xoffset, relRects.src.yoffset,
                                relRects.src.width,   relRects.src.height,
                                relRects.dst.xoffset, relRects.dst.yoffset,
                                relRects.dst.width,   relRects.dst.height,
                                degree,rotation & DRM_RGA_TRANSFORM_FLIP_H,
                                       rotation & DRM_RGA_TRANSFORM_FLIP_V);

	ret = rga_exec(ctx->ctx);
	if (ret < 0) {
	    ALOGE("Handle by rga error");
		return ret;
    }

	return 0;
}

int drmRgaClose(struct hw_device_t *dev)
{
    rga_device_t *device = (rga_device_t*)dev;
    android::Mutex::Autolock lock(mMutex);

    struct rgaContext *ctx = rgaCtx;

    if(!ctx) {
        ALOGE("Try to exit uninit rgaCtx=%p",ctx);
        return -ENODEV;
    }
    
    if(device != &ctx->device) {
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

int drmRgaOpen(const struct hw_module_t* module,
                    const char * name,struct hw_device_t** device)
{
    android::Mutex::Autolock lock(mMutex);

    struct rockchip_device *dev = NULL;
    struct rgaContext *ctx = NULL;
    const hw_module_t *allocMod = NULL;

    int fd = -1;
    int ret = 0;

    ALOGI("Open Moudle[%s]=%p,",name,module);

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

	fd = drmOpen(DRM_MODULE_NAME, NULL);
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

    ret= hw_get_module(GRALLOC_HARDWARE_MODULE_ID, &allocMod);
    ALOGE_IF(ret, "FATAL:can't find the %s module",GRALLOC_HARDWARE_MODULE_ID);
    if (ret == 0)
        ctx->mAllocMod = reinterpret_cast<gralloc_module_t const *>(allocMod);
    else
        goto loadModErr;

    ctx->device.common.close = drmRgaClose;
    ctx->device.rgaCopy = rgaCopy;
    ctx->device.rgaConvert = rgaConvert;
    ctx->device.rgaScale = rgaScale;
    ctx->device.rgaRotateScale = rgaRotateScale;
    ctx->device.rgaFillColor = rgaFillColor;
    ctx->device.rgaQuery = NULL;
    ctx->device.rgaControl = NULL;
    ctx->device.rgaDump = NULL;

    ctx->dev = dev;

    *device = &(ctx->device.common);
    fprintf(stderr, "ctx=%p,ctx->ctx=%p\n",ctx,ctx->ctx);
    rgaCtx = ctx;

init:
    android_atomic_inc(&refCount);
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

static struct hw_module_methods_t rga_module_methods =
{
    open: drmRgaOpen
};

rga_module_t HAL_MODULE_INFO_SYM =
{
    common:
    {
        tag:           HARDWARE_MODULE_TAG,
        version_major: 2,
        version_minor: 0,
        id:            DRMRGA_HARDWARE_MODULE_ID,
        name:          "RGA Module",
        author:        "Rockchip Corporation Ltd.",
        methods:       &rga_module_methods,
        dso:           NULL,
        reserved:      {0, }
    }
};

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

static int getRgaRects(struct rgaContext * ctx, 
                    buffer_handle_t dst,drm_rga_t* tmpRects)
{
    int ret = 0;
    std::vector<int> dstAttrs;

    ret = getHandleAttributes(ctx, dst, &dstAttrs);
    if (ret) {
        ALOGE("dst handle get Attributes fail ret = %d,hnd=%p",ret,&dst);
        printf("dst handle get Attributes fail ret = %d,hnd=%p",ret,&dst);
        return ret;
    }

    memset(tmpRects,0,sizeof(drm_rga_t));

    tmpRects->dst.size = dstAttrs.at(ASIZE);
    tmpRects->dst.width   = dstAttrs.at(AWIDTH);
    tmpRects->dst.height  = dstAttrs.at(AHEIGHT);
    tmpRects->dst.wstride = dstAttrs.at(ASTRIDE);
    tmpRects->dst.format  = dstAttrs.at(AFORMAT);

    return ret;
}

static int getRgaRects(struct rgaContext * ctx, 
                   buffer_handle_t src,buffer_handle_t dst,drm_rga_t* tmpRects)
{
    int ret = 0;
    std::vector<int> srcAttrs,dstAttrs;
    ret = getHandleAttributes(ctx, src, &srcAttrs);
    if (ret) {
        ALOGE("dst handle get Attributes fail ret = %d,hnd=%p",ret,&src);
        printf("dst handle get Attributes fail ret = %d,hnd=%p",ret,&src);
        return ret;
    }
    ret = getHandleAttributes(ctx, dst, &dstAttrs);
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
