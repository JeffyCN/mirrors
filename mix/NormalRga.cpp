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
#include "../normal/NormalRga.h"
#include "../normal/NormalRgaContext.h"
#include "../GraphicBuffer.h"
#include "../RgaApi.h"

#define RGA_BUF_GEM_TYPE_MASK      0xC0
#define RGA_BUF_GEM_TYPE_DMA       0x80

volatile int32_t refCount = 0;
struct rgaContext *rgaCtx = NULL;

void NormalRgaSetLogOnceFlag(int log)
{
	struct rgaContext *ctx = NULL;

    ctx->mLogOnce = log;
    return;
}

void NormalRgaSetAlwaysLogFlag(int log)
{
	struct rgaContext *ctx = NULL;

    ctx->mLogAlways = log;
    return;
}

int NormalRgaOpen(void **context)
{
	struct rgaContext *ctx = NULL;
    char buf[30];
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

	fd = open("/dev/rga", O_RDWR, 0);
	if (fd < 0) {
		ret = -ENODEV;
		ALOGE("failed to open DRM:%s.",strerror(errno));
		goto drmOpenErr;
	}
    ctx->rgaFd = fd;

    ret = ioctl(fd, RGA_GET_VERSION, buf);
    ctx->mVersion = atof(buf);
    fprintf(stderr, "librga:RGA_GET_VERSION:%s,%f\n", buf, ctx->mVersion);

    NormalRgaInitTables();

	fprintf(stderr, "ctx=%p,ctx->rgaFd=%d\n",ctx, ctx->rgaFd );
	rgaCtx = ctx;

init:
	android_atomic_inc(&refCount);
	*context = (void *)ctx;
	return ret;

loadModErr:
rgaInitErr:
devCreateErr:
	close(fd);
drmOpenErr:
	free(ctx);
mallocErr:
	return ret;
}

int NormalRgaClose(void *context)
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

	close(ctx->rgaFd);

	free(ctx);

	return 0;
}

int RgaInit(void **ctx)
{
    int ret = 0;
    ret = NormalRgaOpen(ctx);
    return ret;
}

int RgaDeInit(void *ctx)
{
    int ret = 0;
    ret = NormalRgaClose(ctx);
    return ret;
}

int NormalRgaPaletteTable(buffer_handle_t dst, 
                                              unsigned int v, drm_rga_t *rects)
{
    //check rects
    //check buffer_handle_t with rects
    struct rgaContext *ctx = rgaCtx;
    int srcVirW,srcVirH,srcActW,srcActH,srcXPos,srcYPos;
    int dstVirW,dstVirH,dstActW,dstActH,dstXPos,dstYPos;
    int scaleMode,rotateMode,orientation,ditherEn;
    int srcType,dstType,srcMmuFlag,dstMmuFlag;
    int planeAlpha;
    int dstFd = -1;
    int srcFd = -1;
    int ret = 0;
    drm_rga_t tmpRects,relRects;
    struct rga_req rgaReg;
    bool perpixelAlpha;
    void *srcBuf = NULL;
    void *dstBuf = NULL;
    RECT clip;

	if (!ctx) {
		ALOGE("Try to use uninit rgaCtx=%p",ctx);
		return -ENODEV;
	}

    if (rects && (ctx->mLogAlways || ctx->mLogOnce)) {
        ALOGD("Src:[%d,%d,%d,%d][%d,%d,%d]=>Dst:[%d,%d,%d,%d][%d,%d,%d]",
            rects->src.xoffset,rects->src.yoffset,
            rects->src.width, rects->src.height, 
            rects->src.wstride,rects->src.format, rects->src.size,
            rects->dst.xoffset,rects->dst.yoffset,
            rects->dst.width, rects->dst.height,
            rects->dst.wstride,rects->dst.format, rects->dst.size);
    }

    memset(&rgaReg, 0, sizeof(struct rga_req));

    srcType = dstType = srcMmuFlag = dstMmuFlag = 0;

    ret = NormalRgaGetRects(NULL, dst, &srcType, &dstType, &tmpRects);
    if (ret && !rects) {
        ALOGE("%d:Has not rects for render", __LINE__);
        return ret;
    }

    if (rects) {
        if (rects->src.wstride > 0 && rects->dst.wstride > 0)
            memcpy(&relRects, rects, sizeof(drm_rga_t));
        else if (rects->src.wstride > 0) {
            memcpy(&(relRects.src), &(rects->src), sizeof(rga_rect_t));
            memcpy(&(relRects.dst), &(tmpRects.dst), sizeof(rga_rect_t));
        } else if (rects->dst.wstride > 0) {
            memcpy(&(relRects.src), &(tmpRects.src), sizeof(rga_rect_t));
            memcpy(&(relRects.dst), &(rects->dst), sizeof(rga_rect_t));
        }
    } else
        memcpy(&relRects, &tmpRects, sizeof(drm_rga_t));

    if (ctx->mLogAlways || ctx->mLogOnce) {
        ALOGD("Src:[%d,%d,%d,%d][%d,%d,%d]=>Dst:[%d,%d,%d,%d][%d,%d,%d]",
            tmpRects.src.xoffset,tmpRects.src.yoffset,
            tmpRects.src.width, tmpRects.src.height,
            tmpRects.src.wstride,tmpRects.src.format, tmpRects.src.size,
            tmpRects.dst.xoffset,tmpRects.dst.yoffset,
            tmpRects.dst.width, tmpRects.dst.height,
            tmpRects.dst.wstride,tmpRects.dst.format, tmpRects.dst.size);
        ALOGD("Src:[%d,%d,%d,%d][%d,%d,%d]=>Dst:[%d,%d,%d,%d][%d,%d,%d]",
            relRects.src.xoffset,relRects.src.yoffset,
            relRects.src.width, relRects.src.height,
            relRects.src.wstride,relRects.src.format, relRects.src.size,
            relRects.dst.xoffset,relRects.dst.yoffset,
            relRects.dst.width, relRects.dst.height,
            relRects.dst.wstride,relRects.dst.format, relRects.dst.size);
    }

    RkRgaGetHandleMapAddress(dst, &dstBuf);
    RkRgaGetHandleFd(dst, &dstFd);
    if (dstFd == -1 && !dstBuf) {
        ALOGE("%d:dst has not fd and address for render", __LINE__);
        return ret;
    }

    if (dstFd == 0 && !dstBuf) {
        ALOGE("dstFd is zero, now driver not support");
        return -EINVAL;
    } else
        dstFd = -1;

    orientation = 0;
    rotateMode = 0;
    srcVirW = relRects.src.wstride;
    srcVirH = relRects.src.height;
    srcXPos = relRects.src.xoffset;
    srcYPos = relRects.src.yoffset;
    srcActW = relRects.src.width;
    srcActH = relRects.src.height;

    dstVirW = relRects.dst.wstride;
    dstVirH = relRects.dst.height;
    dstXPos = relRects.dst.xoffset;
    dstYPos = relRects.dst.yoffset;
    dstActW = relRects.dst.width;
    dstActH = relRects.dst.height;

    NormalRgaSetSrcActiveInfo(&rgaReg, srcActW, srcActH, srcXPos, srcYPos);
    NormalRgaSetDstActiveInfo(&rgaReg, dstActW, dstActH, dstXPos, dstYPos);
    NormalRgaSetSrcVirtualInfo(&rgaReg, (unsigned long)srcBuf,
                                    (unsigned long)srcBuf + srcVirW * srcVirH, 
                                    (unsigned long)srcBuf + srcVirW * srcVirH * 5/4,
                                    srcVirW, srcVirH,
                                    RkRgaGetRgaFormat(relRects.src.format),0);
    /*dst*/
    NormalRgaSetDstVirtualInfo(&rgaReg, (unsigned long)dstBuf,
                                    (unsigned long)dstBuf + dstVirW * dstVirH,
                                    (unsigned long)dstBuf + dstVirW * dstVirH * 5/4,
                                    dstVirW, dstVirH, &clip,
                                    RkRgaGetRgaFormat(relRects.dst.format),0);
    NormalRgaSetPatInfo(&rgaReg, dstVirW, dstVirH,
                                     dstXPos, dstYPos, relRects.dst.format);
    NormalRgaSetFadingEnInfo(&rgaReg, v & 0xFF000000, v & 0xFF0000, v & 0xFF00);

    /*mode*/
    NormalRgaUpdatePaletteTableMode(&rgaReg, 0, v & 0xFF);

    if (srcMmuFlag || dstMmuFlag) {
        NormalRgaMmuInfo(&rgaReg, 1, 0, 0, 0, 0, 2);
        NormalRgaMmuFlag(&rgaReg, srcMmuFlag, dstMmuFlag);
    }

    if (ctx->mLogAlways || ctx->mLogOnce) 
        NormalRgaLogOutRgaReq(rgaReg);

    if(ioctl(ctx->rgaFd, RGA_BLIT_SYNC, &rgaReg)) {
        printf(" %s(%d) RGA_BLIT fail: %s",__FUNCTION__, __LINE__,strerror(errno));
        ALOGE(" %s(%d) RGA_BLIT fail: %s",__FUNCTION__, __LINE__,strerror(errno));
    }

    if (ctx->mLogOnce)
        ctx->mLogOnce = 0;

    return 0;
}

int RgaBlit(rga_info *src, rga_info *dst, rga_info *src1)
{
    //check rects
    //check buffer_handle_t with rects
    struct rgaContext *ctx = rgaCtx;
    int srcVirW,srcVirH,srcActW,srcActH,srcXPos,srcYPos;
    int dstVirW,dstVirH,dstActW,dstActH,dstXPos,dstYPos;
    int src1VirW,src1VirH,src1ActW,src1ActH,src1XPos,src1YPos;
    int scaleMode,rotateMode,orientation,ditherEn;
    int srcType,dstType,src1Type,srcMmuFlag,dstMmuFlag,src1MmuFlag;
    int planeAlpha;
    int dstFd = -1;
    int srcFd = -1;
    int src1Fd = -1;
    int rotation;
    int ret = 0;
	rga_rect_t relSrcRect,tmpSrcRect,relDstRect,tmpDstRect;
	rga_rect_t relSrc1Rect,tmpSrc1Rect;
    struct rga_req rgaReg;
    unsigned int yuvToRgbMode;
    unsigned int blend;
    bool perpixelAlpha;
    void *srcBuf = NULL;
    void *dstBuf = NULL;
    void *src1Buf = NULL;
    RECT clip;

	if (!ctx) {
		ALOGE("Try to use uninit rgaCtx=%p",ctx);
		return -ENODEV;
	}

    memset(&rgaReg, 0, sizeof(struct rga_req));

    srcType = dstType = srcMmuFlag = dstMmuFlag = 0;
    src1Type = src1MmuFlag = 0;
    yuvToRgbMode = 0;
    rotation = 0;
    blend = 0;

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
    		ALOGE("dst handle get fd fail ret = %d,hnd=%p", ret, &src->hnd);
    		printf("-dst handle get fd fail ret = %d,hnd=%p", ret, &src->hnd);
    		return ret;
    	}
    	if (!isRectValid(relSrcRect)) {
    	    ret = NormalRgaGetRect(src->hnd, &tmpSrcRect);
    	    if (ret)
    	        return ret;
            memcpy(&relSrcRect, &tmpSrcRect, sizeof(rga_rect_t));
    	}
    	NormalRgaGetMmuType(src->hnd, &srcType);
    }

    if (dst && dst->hnd) {
    	ret = RkRgaGetHandleFd(dst->hnd, &dstFd);
    	if (ret) {
    		ALOGE("dst handle get fd fail ret = %d,hnd=%p", ret, &dst->hnd);
    		printf("-dst handle get fd fail ret = %d,hnd=%p", ret, &dst->hnd);
    		return ret;
    	}
    	if (!isRectValid(relDstRect)) {
    	    ret = NormalRgaGetRect(dst->hnd, &tmpDstRect);
    	    if (ret)
    	        return ret;
            memcpy(&relDstRect, &tmpDstRect, sizeof(rga_rect_t));
    	}
    	NormalRgaGetMmuType(dst->hnd, &dstType);
    }

    if (src1 && src1->hnd) {
    	ret = RkRgaGetHandleFd(src1->hnd, &src1Fd);
    	if (ret) {
    		ALOGE("dst handle get fd fail ret = %d,hnd=%p", ret, &src1->hnd);
    		printf("-dst handle get fd fail ret = %d,hnd=%p", ret, &src1->hnd);
    		return ret;
    	}
    	if (!isRectValid(relSrcRect)) {
    	    ret = NormalRgaGetRect(src1->hnd, &tmpSrc1Rect);
    	    if (ret)
    	        return ret;
            memcpy(&relSrc1Rect, &tmpSrc1Rect, sizeof(rga_rect_t));
    	}
    	NormalRgaGetMmuType(src1->hnd, &src1Type);
    }

    if (src && srcFd < 0)
        srcFd = src->fd;

    if (src && src->phyAddr)
        srcBuf = src->phyAddr;
    else if (src && src->virAddr)
        srcBuf = src->virAddr;

    if (srcFd == -1 && !srcBuf) {
        ALOGE("%d:src has not fd and address for render", __LINE__);
        return ret;
    }

    if (srcFd == 0 && !srcBuf) {
        ALOGE("srcFd is zero, now driver not support");
        return -EINVAL;
    }

    if (srcFd == 0)
        srcFd = -1;

    if (dst && dstFd < 0)
        dstFd = dst->fd;

    if (dst && dst->phyAddr)
        dstBuf = dst->phyAddr;
    else if (dst && dst->virAddr)
        dstBuf = dst->virAddr;

    if (dst && dstFd == -1 && !dstBuf) {
        ALOGE("%d:dst has not fd and address for render", __LINE__);
        return ret;
    }

    if (dst && dstFd == 0 && !dstBuf) {
        ALOGE("dstFd is zero, now driver not support");
        return -EINVAL;
    }

    if (dstFd == 0)
        dstFd = -1;

    if (src1 && src1Fd < 0)
        src1Fd = src1->fd;

    if (src1 && src1->phyAddr)
        src1Buf = src1->phyAddr;
    else if (src1 && src1->virAddr)
        src1Buf = src1->virAddr;

    if (src1 && src1Fd == -1 && !src1Buf) {
        ALOGE("%d:dst has not fd and address for render", __LINE__);
        return ret;
    }

    if (src1 && src1Fd == 0 && !src1Buf) {
        ALOGE("dstFd is zero, now driver not support");
        return -EINVAL;
    }

    if (src1Fd == 0)
        src1Fd = -1;

    planeAlpha = (blend & 0xFF0000) >> 16;
    perpixelAlpha = relSrcRect.format == HAL_PIXEL_FORMAT_RGBA_8888 ||
                    relSrcRect.format == HAL_PIXEL_FORMAT_BGRA_8888;

    switch ((blend & 0xFFFF)) {
        case 0x0105:
            if (perpixelAlpha && planeAlpha < 255)
                NormalRgaSetAlphaEnInfo(&rgaReg, 1, 2, planeAlpha , 1, 9, 0);
            else if (perpixelAlpha)
                NormalRgaSetAlphaEnInfo(&rgaReg, 1, 1, 0, 1, 3, 0);
            else
                NormalRgaSetAlphaEnInfo(&rgaReg, 1, 0, planeAlpha , 0, 0, 0);
            break;

        case 0x0405:
            if (perpixelAlpha && planeAlpha < 255)
                NormalRgaSetAlphaEnInfo(&rgaReg, 1, 2, planeAlpha , 0, 0, 0);
            else if (perpixelAlpha)
                NormalRgaSetAlphaEnInfo(&rgaReg, 1, 1, 0, 0, 0, 0);
            else
                NormalRgaSetAlphaEnInfo(&rgaReg, 1, 0, planeAlpha , 0, 0, 0);
            break;

        case 0x0100:
        default:
            /* Tips: BLENDING_NONE is non-zero value, handle zero value as
             * BLENDING_NONE. */
            /* C = Cs
             * A = As */
            break;
    }

    if (relSrcRect.hstride == 0)
	    relSrcRect.hstride = relSrcRect.height;

    //if (relSrcRect.format == HAL_PIXEL_FORMAT_YCrCb_NV12_10)
    //	    relSrcRect.wstride = relSrcRect.wstride * 5 / 4;

    if (src) {
        ret = checkRectForRga(relSrcRect);
        if (ret) {
            ALOGE("[%s,%d]Error src rect for rga blit", __func__, __LINE__);
            return ret;
        }
    }

    if (dst) {
        ret = checkRectForRga(relDstRect);
        if (ret) {
            ALOGE("[%s,%d]Error dst rect for rga blit", __func__, __LINE__);
            return ret;
        }
    }

    if (src1) {
        ret = checkRectForRga(relSrc1Rect);
        if (ret) {
            ALOGE("[%s,%d]Error src1 rect for rga blit", __func__, __LINE__);
            return ret;
        }
    }

    if (src && dst) {
        float hScale = (float)relSrcRect.width / relDstRect.width;
        float vScale = (float)relSrcRect.height / relDstRect.height;
        if (rotation & HAL_TRANSFORM_ROT_90) {
            hScale = (float)relSrcRect.width / relDstRect.height;
            vScale = (float)relSrcRect.height / relDstRect.width;
        }
        if (hScale < 1/16 || hScale > 16 || vScale < 1/16 || vScale > 16) {
            ALOGE("Error scale[%f,%f] line %d", hScale, vScale, __LINE__);
            return -EINVAL;
        }
        if (ctx->mVersion <= 2.0 && (hScale < 1/8 ||
                                hScale > 8 || vScale < 1/8 || vScale > 8)) {
            ALOGE("Error scale[%f,%f] line %d", hScale, vScale, __LINE__);
            return -EINVAL;
        }
        if (ctx->mVersion <= 1.003 && (hScale < 1/2 || vScale < 1/2)) {
            ALOGE("e scale[%f,%f] ver[%f]", hScale, vScale, ctx->mVersion);
            return -EINVAL;
        }
    }

    switch (rotation) {
        case HAL_TRANSFORM_FLIP_H:
            orientation = 0;
            rotateMode = 2;
            srcVirW = relSrcRect.wstride;
            srcVirH = relSrcRect.hstride;
            srcXPos = relSrcRect.xoffset;
            srcYPos = relSrcRect.yoffset;
            srcActW = relSrcRect.width;
            srcActH = relSrcRect.height;

            dstVirW = relDstRect.wstride;
            dstVirH = relDstRect.height;
            dstXPos = relDstRect.xoffset;
            dstYPos = relDstRect.yoffset;
            dstActW = relDstRect.width;
            dstActH = relDstRect.height;
            break;
        case HAL_TRANSFORM_FLIP_V:
            orientation = 0;
            rotateMode = 3;
            srcVirW = relSrcRect.wstride;
            srcVirH = relSrcRect.hstride;
            srcXPos = relSrcRect.xoffset;
            srcYPos = relSrcRect.yoffset;
            srcActW = relSrcRect.width;
            srcActH = relSrcRect.height;

            dstVirW = relDstRect.wstride;
            dstVirH = relDstRect.height;
            dstXPos = relDstRect.xoffset;
            dstYPos = relDstRect.yoffset;
            dstActW = relDstRect.width;
            dstActH = relDstRect.height;
            break;
        case HAL_TRANSFORM_ROT_90:
            orientation = 90;
            rotateMode = 1;
            srcVirW = relSrcRect.wstride;
            srcVirH = relSrcRect.hstride;
            srcXPos = relSrcRect.xoffset;
            srcYPos = relSrcRect.yoffset;
            srcActW = relSrcRect.width;
            srcActH = relSrcRect.height;

            dstVirW = relDstRect.wstride;
            dstVirH = relDstRect.height;
            dstXPos = relDstRect.width - 1;
            //dstYPos = relDstRect.yoffset;
            dstYPos = 0;
            dstActW = relDstRect.height;
            dstActH = relDstRect.width;
            break;
        case HAL_TRANSFORM_ROT_180:
            orientation = 180;
            rotateMode = 1;
            srcVirW = relSrcRect.wstride;
            srcVirH = relSrcRect.hstride;
            srcXPos = relSrcRect.xoffset;
            srcYPos = relSrcRect.yoffset;
            srcActW = relSrcRect.width;
            srcActH = relSrcRect.height;

            dstVirW = relDstRect.wstride;
            dstVirH = relDstRect.height;
            dstXPos = relDstRect.width - 1;
            dstYPos = relDstRect.height - 1;
            dstActW = relDstRect.width;
            dstActH = relDstRect.height;
            break;
        case HAL_TRANSFORM_ROT_270:
            orientation = 270;
            rotateMode = 1;
            srcVirW = relSrcRect.wstride;
            srcVirH = relSrcRect.hstride;
            srcXPos = relSrcRect.xoffset;
            srcYPos = relSrcRect.yoffset;
            srcActW = relSrcRect.width;
            srcActH = relSrcRect.height;

            dstVirW = relDstRect.wstride;
            dstVirH = relDstRect.height;
            //dstXPos = relDstRect.xoffset;
            dstXPos = 0;
            dstYPos = relDstRect.height - 1;
            dstActW = relDstRect.height;
            dstActH = relDstRect.width;
            break;
        default:
            orientation = 0;
            rotateMode = 0;
            srcVirW = relSrcRect.wstride;
            srcVirH = relSrcRect.hstride;
            srcXPos = relSrcRect.xoffset;
            srcYPos = relSrcRect.yoffset;
            srcActW = relSrcRect.width;
            srcActH = relSrcRect.height;

            dstVirW = relDstRect.wstride;
            dstVirH = relDstRect.height;
            dstXPos = relDstRect.xoffset;
            dstYPos = relDstRect.yoffset;
            dstActW = relDstRect.width;
            dstActH = relDstRect.height;
            break;
    }

    clip.xmin = 0;
    clip.xmax = dstActW - 1;
    clip.ymin = 0;
    clip.ymax = dstActH - 1;

    scaleMode = 0;
    //scale up use bicubic
    if (srcActW / dstActW < 1 || srcActH / dstActH < 1)
        scaleMode = 2;

    /*
    if (scaleMode && (srcFormat == RK_FORMAT_RGBA_8888 ||
                                            srcFormat == RK_FORMAT_BGRA_8888)) {
        scale_mode = 0;     //  force change scale_mode to 0 ,for rga not support
    }*/

    ditherEn = (android::bytesPerPixel(relSrcRect.format) 
                        != android::bytesPerPixel(relSrcRect.format) ? 1 : 0);


    if (ctx->mVersion <= 1.003) {
        srcMmuFlag = dstMmuFlag = 1;

#if defined(__arm64__) || defined(__aarch64__)
        NormalRgaSetSrcVirtualInfo(&rgaReg, (unsigned long)srcBuf,
                                        (unsigned long)srcBuf + srcVirW * srcVirH, 
                                        (unsigned long)srcBuf + srcVirW * srcVirH * 5/4,
                                        srcVirW, srcVirH,
                                        RkRgaGetRgaFormat(relSrcRect.format),0);
        /*dst*/
        NormalRgaSetDstVirtualInfo(&rgaReg, (unsigned long)dstBuf,
                                        (unsigned long)dstBuf + dstVirW * dstVirH,
                                        (unsigned long)dstBuf + dstVirW * dstVirH * 5/4,
                                        dstVirW, dstVirH, &clip,
                                        RkRgaGetRgaFormat(relDstRect.format),0);
#else
        NormalRgaSetSrcVirtualInfo(&rgaReg, (unsigned int)srcBuf,
                                        (unsigned int)srcBuf + srcVirW * srcVirH, 
                                        (unsigned int)srcBuf + srcVirW * srcVirH * 5/4,
                                        srcVirW, srcVirH,
                                        RkRgaGetRgaFormat(relSrcRect.format),0);
        /*dst*/
        NormalRgaSetDstVirtualInfo(&rgaReg, (unsigned int)dstBuf,
                                        (unsigned int)dstBuf + dstVirW * dstVirH,
                                        (unsigned int)dstBuf + dstVirW * dstVirH * 5/4,
                                        dstVirW, dstVirH, &clip,
                                        RkRgaGetRgaFormat(relDstRect.format),0);
#endif
    } else if (ctx->mVersion < 2.0) {
        /*Src*/
        if (srcFd != -1) {
                srcMmuFlag = srcType ? 1 : 0;
            if (src && srcFd == src->fd)
                srcMmuFlag = src->mmuFlag ? 1 : 0;
            NormalRgaSetSrcVirtualInfo(&rgaReg, 0, 0, 0, srcVirW, srcVirH,
                                        RkRgaGetRgaFormat(relSrcRect.format),0);
            NormalRgaSetFdsOffsets(&rgaReg, srcFd, 0, 0, 0);
        } else {
            if (src && src->hnd)
                srcMmuFlag = srcType ? 1 : 0;
            if (src && srcBuf == src->virAddr)
                srcMmuFlag = 1;
            if (src && srcBuf == src->phyAddr)
                srcMmuFlag = 0;
#if defined(__arm64__) || defined(__aarch64__)
            NormalRgaSetSrcVirtualInfo(&rgaReg, (unsigned long)srcBuf,
                                        (unsigned long)srcBuf + srcVirW * srcVirH, 
                                        (unsigned long)srcBuf + srcVirW * srcVirH * 5/4,
                                        srcVirW, srcVirH,
                                        RkRgaGetRgaFormat(relSrcRect.format),0);
#else
            NormalRgaSetSrcVirtualInfo(&rgaReg, (unsigned int)srcBuf,
                                        (unsigned int)srcBuf + srcVirW * srcVirH, 
                                        (unsigned int)srcBuf + srcVirW * srcVirH * 5/4,
                                        srcVirW, srcVirH,
                                        RkRgaGetRgaFormat(relSrcRect.format),0);
#endif
        }
        /*dst*/
        if (dstFd != -1) {
                dstMmuFlag = dstType ? 1 : 0;
            if (dst && dstFd == dst->fd)
                dstMmuFlag = dst->mmuFlag ? 1 : 0;
            NormalRgaSetDstVirtualInfo(&rgaReg, 0, 0, 0, dstVirW, dstVirH, &clip,
                                        RkRgaGetRgaFormat(relDstRect.format),0);
    	    /*src dst fd*/
            NormalRgaSetFdsOffsets(&rgaReg, 0, dstFd, 0, 0);
        } else {
            if (dst && dst->hnd)
                dstMmuFlag = dstType ? 1 : 0;
            if (dst && dstBuf == dst->virAddr)
                dstMmuFlag = 1;
            if (dst && dstBuf == dst->phyAddr)
                dstMmuFlag = 0;
#if defined(__arm64__) || defined(__aarch64__)
            NormalRgaSetDstVirtualInfo(&rgaReg, (unsigned long)dstBuf,
                                        (unsigned long)dstBuf + dstVirW * dstVirH,
                                        (unsigned long)dstBuf + dstVirW * dstVirH * 5/4,
                                        dstVirW, dstVirH, &clip,
                                        RkRgaGetRgaFormat(relDstRect.format),0);
#else
            NormalRgaSetDstVirtualInfo(&rgaReg, (unsigned int)dstBuf,
                                        (unsigned int)dstBuf + dstVirW * dstVirH,
                                        (unsigned int)dstBuf + dstVirW * dstVirH * 5/4,
                                        dstVirW, dstVirH, &clip,
                                        RkRgaGetRgaFormat(relDstRect.format),0);
#endif
        }
    } else {
        if (src && src->hnd)
            srcMmuFlag = srcType ? 1 : 0;
        if (src && srcBuf == src->virAddr)
            srcMmuFlag = 1;
        if (src && srcBuf == src->phyAddr)
            srcMmuFlag = 0;
        if (srcFd != -1)
            srcMmuFlag = srcType ? 1 : 0;
        if (src && srcFd == src->fd)
            srcMmuFlag = src->mmuFlag ? 1 : 0;

        if (dst && dst->hnd)
            dstMmuFlag = dstType ? 1 : 0;
        if (dst && dstBuf == dst->virAddr)
            dstMmuFlag = 1;
        if (dst && dstBuf == dst->phyAddr)
            dstMmuFlag = 0;
        if (dstFd != -1)
            dstMmuFlag = dstType ? 1 : 0;
        if (dst && dstFd == dst->fd)
            dstMmuFlag = dst->mmuFlag ? 1 : 0;
#if defined(__arm64__) || defined(__aarch64__)
        NormalRgaSetSrcVirtualInfo(&rgaReg, srcFd != -1 ? srcFd : 0,
                                        (unsigned long)srcBuf, 
                                        (unsigned long)srcBuf + srcVirW * srcVirH,
                                        srcVirW, srcVirH,
                                        RkRgaGetRgaFormat(relSrcRect.format),0);
        /*dst*/
        NormalRgaSetDstVirtualInfo(&rgaReg, dstFd != -1 ? dstFd : 0,
                                        (unsigned long)dstBuf,
                                        (unsigned long)dstBuf + dstVirW * dstVirH,
                                        dstVirW, dstVirH, &clip,
                                        RkRgaGetRgaFormat(relDstRect.format),0);
#else
        NormalRgaSetSrcVirtualInfo(&rgaReg, srcFd != -1 ? srcFd : 0,
                                        (unsigned int)srcBuf, 
                                        (unsigned int)srcBuf + srcVirW * srcVirH,
                                        srcVirW, srcVirH,
                                        RkRgaGetRgaFormat(relSrcRect.format),0);
        /*dst*/
        NormalRgaSetDstVirtualInfo(&rgaReg, dstFd != -1 ? dstFd : 0,
                                        (unsigned int)dstBuf,
                                        (unsigned int)dstBuf + dstVirW * dstVirH,
                                        dstVirW, dstVirH, &clip,
                                        RkRgaGetRgaFormat(relDstRect.format),0);
#endif
    }

    NormalRgaSetSrcActiveInfo(&rgaReg, srcActW, srcActH, srcXPos, srcYPos);
    NormalRgaSetDstActiveInfo(&rgaReg, dstActW, dstActH, dstXPos, dstYPos);

    if (NormalRgaIsYuvFormat(RkRgaGetRgaFormat(relSrcRect.format)) &&
                    NormalRgaIsRgbFormat(RkRgaGetRgaFormat(relDstRect.format)))
        yuvToRgbMode |= 0x1 << 0;

    if (NormalRgaIsRgbFormat(RkRgaGetRgaFormat(relSrcRect.format)) &&
                    NormalRgaIsYuvFormat(RkRgaGetRgaFormat(relDstRect.format)))
        yuvToRgbMode |= 0x2 << 4;

    /*mode*/
    NormalRgaSetBitbltMode(&rgaReg, scaleMode, rotateMode,
					orientation, ditherEn, 0, yuvToRgbMode);

    /*force to mmu flag to mask*/
    srcMmuFlag = dstMmuFlag = 1;

    if (srcMmuFlag || dstMmuFlag) {
        NormalRgaMmuInfo(&rgaReg, 1, 0, 0, 0, 0, 2);
        NormalRgaMmuFlag(&rgaReg, srcMmuFlag, dstMmuFlag);
    }

    rgaReg.render_mode |= RGA_BUF_GEM_TYPE_DMA;

    //ALOGD("%d,%d,%d", srcMmuFlag, dstMmuFlag,rotateMode);
    //NormalRgaLogOutRgaReq(rgaReg);

    if(ioctl(ctx->rgaFd, RGA_BLIT_SYNC, &rgaReg)) {
        printf(" %s(%d) RGA_BLIT fail: %s",__FUNCTION__, __LINE__,strerror(errno));
        ALOGE(" %s(%d) RGA_BLIT fail: %s",__FUNCTION__, __LINE__,strerror(errno));
        ALOGE("%d,%d,%d", srcMmuFlag, dstMmuFlag, rotateMode);
        NormalRgaLogOutRgaReq(rgaReg);
    }

    return 0;
}

int RgaCollorFill(rga_info *dst)
{
    //check rects
    //check buffer_handle_t with rects
    struct rgaContext *ctx = rgaCtx;
    int dstVirW,dstVirH,dstActW,dstActH,dstXPos,dstYPos;
    int scaleMode,ditherEn;
    int dstType,dstMmuFlag;
    int dstFd = -1;
    int ret = 0;
    unsigned int color = 0x00000000;
    rga_rect_t relDstRect,tmpDstRect;
    struct rga_req rgaReg;
    COLOR_FILL fillColor ;
    void *dstBuf = NULL;
    RECT clip;

    if (!ctx) {
	    ALOGE("Try to use uninit rgaCtx=%p",ctx);
	    return -ENODEV;
    }

    memset(&rgaReg, 0, sizeof(struct rga_req));

    dstType = dstMmuFlag = 0;

    if (!dst) {
        ALOGE("src = %p, dst = %p", dst, dst);
        return -EINVAL;
    }

    if (dst) {
        color = dst->color;
        memcpy(&relDstRect, &dst->rect, sizeof(rga_rect_t));
    }

    dstFd = -1;

    if (relDstRect.hstride == 0)
	    relDstRect.hstride = relDstRect.height;

    if (dst && dst->hnd) {
	ret = RkRgaGetHandleFd(dst->hnd, &dstFd);
	if (ret) {
		ALOGE("dst handle get fd fail ret = %d,hnd=%p", ret, &dst->hnd);
		printf("-dst handle get fd fail ret = %d,hnd=%p", ret, &dst->hnd);
		return ret;
	}
	if (!isRectValid(relDstRect)) {
            ret = NormalRgaGetRect(dst->hnd, &tmpDstRect);
	    if (ret)
		    return ret;
	    memcpy(&relDstRect, &tmpDstRect, sizeof(rga_rect_t));
	}
	NormalRgaGetMmuType(dst->hnd, &dstType);
    }


    if (dst && dstFd < 0)
        dstFd = dst->fd;

    if (dst && dst->phyAddr)
        dstBuf = dst->phyAddr;
    else if (dst && dst->virAddr)
        dstBuf = dst->virAddr;

    if (dst && dstFd == -1 && !dstBuf) {
        ALOGE("%d:dst has not fd and address for render", __LINE__);
        return ret;
    }

    if (dst && dstFd == 0 && !dstBuf) {
        ALOGE("dstFd is zero, now driver not support");
        return -EINVAL;
    }

    if (dstFd == 0)
        dstFd = -1;

    if (relDstRect.hstride == 0)
	relDstRect.hstride = relDstRect.height;

    dstVirW = relDstRect.wstride;
    dstVirH = relDstRect.hstride;
    dstXPos = relDstRect.xoffset;
    dstYPos = relDstRect.yoffset;
    dstActW = relDstRect.width;
    dstActH = relDstRect.height;

    clip.xmin = 0;
    clip.xmax = dstActW - 1;
    clip.ymin = 0;
    clip.ymax = dstActH - 1;

    if (ctx->mVersion <= 1.003) {
#if defined(__arm64__) || defined(__aarch64__)
        /*dst*/
        NormalRgaSetDstVirtualInfo(&rgaReg, (unsigned long)dstBuf,
                                        (unsigned long)dstBuf + dstVirW * dstVirH,
                                        (unsigned long)dstBuf + dstVirW * dstVirH * 5/4,
                                        dstVirW, dstVirH, &clip,
                                        RkRgaGetRgaFormat(relDstRect.format),0);
#else
        /*dst*/
        NormalRgaSetDstVirtualInfo(&rgaReg, (unsigned int)dstBuf,
                                        (unsigned int)dstBuf + dstVirW * dstVirH,
                                        (unsigned int)dstBuf + dstVirW * dstVirH * 5/4,
                                        dstVirW, dstVirH, &clip,
                                        RkRgaGetRgaFormat(relDstRect.format),0);
#endif
    } else if (ctx->mVersion < 2.0) {
        /*dst*/
        if (dstFd != -1) {
                dstMmuFlag = dstType ? 1 : 0;
            if (dst && dstFd == dst->fd)
                dstMmuFlag = dst->mmuFlag ? 1 : 0;
            NormalRgaSetDstVirtualInfo(&rgaReg, 0, 0, 0, dstVirW, dstVirH, &clip,
                                        RkRgaGetRgaFormat(relDstRect.format),0);
	    /*src dst fd*/
            NormalRgaSetFdsOffsets(&rgaReg, 0, dstFd, 0, 0);
        } else {
            if (dst && dst->hnd)
                dstMmuFlag = dstType ? 1 : 0;
            if (dst && dstBuf == dst->virAddr)
                dstMmuFlag = 1;
            if (dst && dstBuf == dst->phyAddr)
                dstMmuFlag = 0;
#if defined(__arm64__) || defined(__aarch64__)
            NormalRgaSetDstVirtualInfo(&rgaReg, (unsigned long)dstBuf,
                                        (unsigned long)dstBuf + dstVirW * dstVirH,
                                        (unsigned long)dstBuf + dstVirW * dstVirH * 5/4,
                                        dstVirW, dstVirH, &clip,
                                        RkRgaGetRgaFormat(relDstRect.format),0);
#else
            NormalRgaSetDstVirtualInfo(&rgaReg, (unsigned int)dstBuf,
                                        (unsigned int)dstBuf + dstVirW * dstVirH,
                                        (unsigned int)dstBuf + dstVirW * dstVirH * 5/4,
                                        dstVirW, dstVirH, &clip,
                                        RkRgaGetRgaFormat(relDstRect.format),0);
#endif
        }
    } else {
        if (dst && dst->hnd)
            dstMmuFlag = dstType ? 1 : 0;
        if (dst && dstBuf == dst->virAddr)
            dstMmuFlag = 1;
        if (dst && dstBuf == dst->phyAddr)
            dstMmuFlag = 0;
        if (dstFd != -1)
            dstMmuFlag = dstType ? 1 : 0;
        if (dst && dstFd == dst->fd)
            dstMmuFlag = dst->mmuFlag ? 1 : 0;
#if defined(__arm64__) || defined(__aarch64__)
        /*dst*/
        NormalRgaSetDstVirtualInfo(&rgaReg, dstFd != -1 ? dstFd : 0,
                                        (unsigned long)dstBuf,
                                        (unsigned long)dstBuf + dstVirW * dstVirH,
                                        dstVirW, dstVirH, &clip,
                                        RkRgaGetRgaFormat(relDstRect.format),0);
#else
        /*dst*/
        NormalRgaSetDstVirtualInfo(&rgaReg, dstFd != -1 ? dstFd : 0,
                                        (unsigned int)dstBuf,
                                        (unsigned int)dstBuf + dstVirW * dstVirH,
                                        dstVirW, dstVirH, &clip,
                                        RkRgaGetRgaFormat(relDstRect.format),0);
#endif
    }

    NormalRgaSetDstActiveInfo(&rgaReg, dstActW, dstActH, dstXPos, dstYPos);

    memset(&fillColor , 0x0, sizeof(COLOR_FILL));

    /*mode*/
    NormalRgaSetColorFillMode(&rgaReg, &fillColor, 0, 0, color, 0, 0, 0, 0, 0);

    if (dstMmuFlag) {
        NormalRgaMmuInfo(&rgaReg, 1, 0, 0, 0, 0, 2);
        NormalRgaMmuFlag(&rgaReg, dstMmuFlag, dstMmuFlag);
    }

    //ALOGD("%d,%d,%d", srcMmuFlag, dstMmuFlag, rotateMode);
    //NormalRgaLogOutRgaReq(rgaReg);

    if(ioctl(ctx->rgaFd, RGA_BLIT_SYNC, &rgaReg)) {
        printf("%s(%d) RGA_BLIT fail: %s",__FUNCTION__, __LINE__,strerror(errno));
        ALOGE("%s(%d) RGA_BLIT fail: %s",__FUNCTION__, __LINE__,strerror(errno));
        NormalRgaLogOutRgaReq(rgaReg);
    }

    return 0;
}

int RgaBlit(buffer_handle_t src,
                 buffer_handle_t dst, drm_rga_t *rects, int rotation, int blend)
{

    //check rects
    //check buffer_handle_t with rects
    struct rgaContext *ctx = rgaCtx;
    int srcVirW,srcVirH,srcActW,srcActH,srcXPos,srcYPos;
    int dstVirW,dstVirH,dstActW,dstActH,dstXPos,dstYPos;
    int scaleMode,rotateMode,orientation,ditherEn;
    int srcType,dstType,srcMmuFlag,dstMmuFlag;
    int planeAlpha;
    int dstFd = -1;
    int srcFd = -1;
    int ret = 0;
    drm_rga_t tmpRects,relRects;
    struct rga_req rgaReg;
    bool perpixelAlpha;
    void *srcBuf = NULL;
    void *dstBuf = NULL;
    RECT clip;

	if (!ctx) {
		ALOGE("Try to use uninit rgaCtx=%p",ctx);
		return -ENODEV;
	}

    if (rects && (ctx->mLogAlways || ctx->mLogOnce)) {
        ALOGD("Src:[%d,%d,%d,%d][%d,%d,%d]=>Dst:[%d,%d,%d,%d][%d,%d,%d]",
            rects->src.xoffset,rects->src.yoffset,
            rects->src.width, rects->src.height, 
            rects->src.wstride,rects->src.format, rects->src.size,
            rects->dst.xoffset,rects->dst.yoffset,
            rects->dst.width, rects->dst.height,
            rects->dst.wstride,rects->dst.format, rects->dst.size);
    }

    memset(&rgaReg, 0, sizeof(struct rga_req));

    srcType = dstType = srcMmuFlag = dstMmuFlag = 0;

    ret = NormalRgaGetRects(src, dst, &srcType, &dstType, &tmpRects);
    if (ret && !rects) {
        ALOGE("%d:Has not rects for render", __LINE__);
        return ret;
    }

    if (rects) {
        if (rects->src.wstride > 0 && rects->dst.wstride > 0)
            memcpy(&relRects, rects, sizeof(drm_rga_t));
        else if (rects->src.wstride > 0) {
            memcpy(&(relRects.src), &(rects->src), sizeof(rga_rect_t));
            memcpy(&(relRects.dst), &(tmpRects.dst), sizeof(rga_rect_t));
        } else if (rects->dst.wstride > 0) {
            memcpy(&(relRects.src), &(tmpRects.src), sizeof(rga_rect_t));
            memcpy(&(relRects.dst), &(rects->dst), sizeof(rga_rect_t));
        }
    } else
        memcpy(&relRects, &tmpRects, sizeof(drm_rga_t));

    if (ctx->mLogAlways || ctx->mLogOnce) {
        ALOGD("Src:[%d,%d,%d,%d][%d,%d,%d]=>Dst:[%d,%d,%d,%d][%d,%d,%d]",
            tmpRects.src.xoffset,tmpRects.src.yoffset,
            tmpRects.src.width, tmpRects.src.height, 
            tmpRects.src.wstride,tmpRects.src.format, tmpRects.src.size,
            tmpRects.dst.xoffset,tmpRects.dst.yoffset,
            tmpRects.dst.width, tmpRects.dst.height,
            tmpRects.dst.wstride,tmpRects.dst.format, tmpRects.dst.size);
        ALOGD("Src:[%d,%d,%d,%d][%d,%d,%d]=>Dst:[%d,%d,%d,%d][%d,%d,%d]",
            relRects.src.xoffset,relRects.src.yoffset,
            relRects.src.width, relRects.src.height, 
            relRects.src.wstride,relRects.src.format, relRects.src.size,
            relRects.dst.xoffset,relRects.dst.yoffset,
            relRects.dst.width, relRects.dst.height,
            relRects.dst.wstride,relRects.dst.format, relRects.dst.size);
    }

    RkRgaGetHandleMapAddress(src, &srcBuf);
    RkRgaGetHandleFd(src, &srcFd);
    if (srcFd == -1 && !srcBuf) {
        ALOGE("%d:src has not fd and address for render", __LINE__);
        return ret;
    }

    if (srcFd == 0 && !srcBuf) {
        ALOGE("srcFd is zero, now driver not support");
        return -EINVAL;
    } else
        srcFd = -1;

    RkRgaGetHandleMapAddress(dst, &dstBuf);
    RkRgaGetHandleFd(dst, &dstFd);
    if (dstFd == -1 && !dstBuf) {
        ALOGE("%d:dst has not fd and address for render", __LINE__);
        return ret;
    }

    if (dstFd == 0 && !dstBuf) {
        ALOGE("dstFd is zero, now driver not support");
        return -EINVAL;
    } else
        dstFd = -1;

    planeAlpha = (blend & 0xFF0000) >> 16;
    perpixelAlpha = relRects.src.format == HAL_PIXEL_FORMAT_RGBA_8888 ||
                    relRects.src.format == HAL_PIXEL_FORMAT_BGRA_8888;

    switch ((blend & 0xFFFF)) {
        case 0x0105:
            if (perpixelAlpha && planeAlpha < 255)
                NormalRgaSetAlphaEnInfo(&rgaReg, 1, 2, planeAlpha , 1, 9, 0);
            else if (perpixelAlpha)
                NormalRgaSetAlphaEnInfo(&rgaReg, 1, 1, 0, 1, 3, 0);
            else
                NormalRgaSetAlphaEnInfo(&rgaReg, 1, 0, planeAlpha , 0, 0, 0);
            break;

        case 0x0405:
            if (perpixelAlpha && planeAlpha < 255)
                NormalRgaSetAlphaEnInfo(&rgaReg, 1, 2, planeAlpha , 0, 0, 0);
            else if (perpixelAlpha)
                NormalRgaSetAlphaEnInfo(&rgaReg, 1, 1, 0, 0, 0, 0);
            else
                NormalRgaSetAlphaEnInfo(&rgaReg, 1, 0, planeAlpha , 0, 0, 0);
            break;

        case 0x0100:
        default:
            /* Tips: BLENDING_NONE is non-zero value, handle zero value as
             * BLENDING_NONE. */
            /* C = Cs
             * A = As */
            break;
    }

    switch (rotation) {
        case HAL_TRANSFORM_FLIP_H:
            orientation = 0;
            rotateMode = 2;
            srcVirW = relRects.src.wstride;
            srcVirH = relRects.src.height;
            srcXPos = relRects.src.xoffset;
            srcYPos = relRects.src.yoffset;
            srcActW = relRects.src.width;
            srcActH = relRects.src.height;

            dstVirW = relRects.dst.wstride;
            dstVirH = relRects.dst.height;
            dstXPos = relRects.dst.xoffset;
            dstYPos = relRects.dst.yoffset;
            dstActW = relRects.dst.width;
            dstActH = relRects.dst.height;
            break;
        case HAL_TRANSFORM_FLIP_V:
            orientation = 0;
            rotateMode = 3;
            srcVirW = relRects.src.wstride;
            srcVirH = relRects.src.height;
            srcXPos = relRects.src.xoffset;
            srcYPos = relRects.src.yoffset;
            srcActW = relRects.src.width;
            srcActH = relRects.src.height;

            dstVirW = relRects.dst.wstride;
            dstVirH = relRects.dst.height;
            dstXPos = relRects.dst.xoffset;
            dstYPos = relRects.dst.yoffset;
            dstActW = relRects.dst.width;
            dstActH = relRects.dst.height;
            break;
        case HAL_TRANSFORM_ROT_90:
            orientation = 90;
            rotateMode = 1;
            srcVirW = relRects.src.wstride;
            srcVirH = relRects.src.height;
            srcXPos = relRects.src.xoffset;
            srcYPos = relRects.src.yoffset;
            srcActW = relRects.src.width;
            srcActH = relRects.src.height;

            dstVirW = relRects.dst.wstride;
            dstVirH = relRects.dst.height;
            dstXPos = relRects.dst.width - 1;
            //dstYPos = relRects.dst.yoffset;
            dstYPos = 0;
            dstActW = relRects.dst.height;
            dstActH = relRects.dst.width;
            break;
        case HAL_TRANSFORM_ROT_180:
            orientation = 180;
            rotateMode = 1;
            srcVirW = relRects.src.wstride;
            srcVirH = relRects.src.height;
            srcXPos = relRects.src.xoffset;
            srcYPos = relRects.src.yoffset;
            srcActW = relRects.src.width;
            srcActH = relRects.src.height;

            dstVirW = relRects.dst.wstride;
            dstVirH = relRects.dst.height;
            dstXPos = relRects.dst.width - 1;
            dstYPos = relRects.dst.height - 1;
            dstActW = relRects.dst.width;
            dstActH = relRects.dst.height;
            break;
        case HAL_TRANSFORM_ROT_270:
            orientation = 270;
            rotateMode = 1;
            srcVirW = relRects.src.wstride;
            srcVirH = relRects.src.height;
            srcXPos = relRects.src.xoffset;
            srcYPos = relRects.src.yoffset;
            srcActW = relRects.src.width;
            srcActH = relRects.src.height;

            dstVirW = relRects.dst.wstride;
            dstVirH = relRects.dst.height;
            //dstXPos = relRects.dst.xoffset;
            dstXPos = 0;
            dstYPos = relRects.dst.height - 1;
            dstActW = relRects.dst.height;
            dstActH = relRects.dst.width;
            break;
        default:
            orientation = 0;
            rotateMode = 0;
            srcVirW = relRects.src.wstride;
            srcVirH = relRects.src.height;
            srcXPos = relRects.src.xoffset;
            srcYPos = relRects.src.yoffset;
            srcActW = relRects.src.width;
            srcActH = relRects.src.height;

            dstVirW = relRects.dst.wstride;
            dstVirH = relRects.dst.height;
            dstXPos = relRects.dst.xoffset;
            dstYPos = relRects.dst.yoffset;
            dstActW = relRects.dst.width;
            dstActH = relRects.dst.height;
            break;
    }

    clip.xmin = 0;
    clip.xmax = dstActW - 1;
    clip.ymin = 0;
    clip.ymax = dstActH - 1;

    scaleMode = 0;
    //scale up use bicubic
    if (srcActW / dstActW < 1 || srcActH / dstActH < 1)
        scaleMode = 2;

    /*
    if (scaleMode && (srcFormat == RK_FORMAT_RGBA_8888 ||
                                            srcFormat == RK_FORMAT_BGRA_8888)) {
        scale_mode = 0;     //  force change scale_mode to 0 ,for rga not support
    }*/

    ditherEn = (android::bytesPerPixel(relRects.src.format) 
                        != android::bytesPerPixel(relRects.src.format) ? 1 : 0);

    if (ctx->mVersion <= 1.003) {
        srcMmuFlag = dstMmuFlag = 1;

#if defined(__arm64__) || defined(__aarch64__)
        NormalRgaSetSrcVirtualInfo(&rgaReg, (unsigned long)srcBuf,
                                        (unsigned long)srcBuf + srcVirW * srcVirH, 
                                        (unsigned long)srcBuf + srcVirW * srcVirH * 5/4,
                                        srcVirW, srcVirH,
                                        RkRgaGetRgaFormat(relRects.src.format),0);
        /*dst*/
        NormalRgaSetDstVirtualInfo(&rgaReg, (unsigned long)dstBuf,
                                        (unsigned long)dstBuf + dstVirW * dstVirH,
                                        (unsigned long)dstBuf + dstVirW * dstVirH * 5/4,
                                        dstVirW, dstVirH, &clip,
                                        RkRgaGetRgaFormat(relRects.dst.format),0);
#else
        NormalRgaSetSrcVirtualInfo(&rgaReg, (unsigned int)srcBuf,
                                        (unsigned int)srcBuf + srcVirW * srcVirH, 
                                        (unsigned int)srcBuf + srcVirW * srcVirH * 5/4,
                                        srcVirW, srcVirH,
                                        RkRgaGetRgaFormat(relRects.src.format),0);
        /*dst*/
        NormalRgaSetDstVirtualInfo(&rgaReg, (unsigned int)dstBuf,
                                        (unsigned int)dstBuf + dstVirW * dstVirH,
                                        (unsigned int)dstBuf + dstVirW * dstVirH * 5/4,
                                        dstVirW, dstVirH, &clip,
                                        RkRgaGetRgaFormat(relRects.dst.format),0);
#endif
    } else if (ctx->mVersion < 2.0) {
        /*Src*/
        if (srcFd != -1) {
            srcMmuFlag = srcType ? 1 : 0;
            NormalRgaSetSrcVirtualInfo(&rgaReg, 0, 0, 0, srcVirW, srcVirH,
                                        RkRgaGetRgaFormat(relRects.src.format),0);
            NormalRgaSetFdsOffsets(&rgaReg, srcFd, 0, 0, 0);
        } else {
            srcMmuFlag = 1;

#if defined(__arm64__) || defined(__aarch64__)
            NormalRgaSetSrcVirtualInfo(&rgaReg, (unsigned long)srcBuf,
                                        (unsigned long)srcBuf + srcVirW * srcVirH, 
                                        (unsigned long)srcBuf + srcVirW * srcVirH * 5/4,
                                        srcVirW, srcVirH,
                                        RkRgaGetRgaFormat(relRects.src.format),0);
#else
            NormalRgaSetSrcVirtualInfo(&rgaReg, (unsigned int)srcBuf,
                                        (unsigned int)srcBuf + srcVirW * srcVirH, 
                                        (unsigned int)srcBuf + srcVirW * srcVirH * 5/4,
                                        srcVirW, srcVirH,
                                        RkRgaGetRgaFormat(relRects.src.format),0);
#endif
        }
        /*dst*/
        if (dstFd != -1) {
            dstMmuFlag = srcType ? 1 : 0;
            NormalRgaSetDstVirtualInfo(&rgaReg, 0, 0, 0, dstVirW, dstVirH, &clip,
                                        RkRgaGetRgaFormat(relRects.dst.format),0);
    	    /*src dst fd*/
            NormalRgaSetFdsOffsets(&rgaReg, 0, dstFd, 0, 0);
        } else {
            dstMmuFlag = 1;
#if defined(__arm64__) || defined(__aarch64__)
            NormalRgaSetDstVirtualInfo(&rgaReg, (unsigned long)dstBuf,
                                        (unsigned long)dstBuf + dstVirW * dstVirH,
                                        (unsigned long)dstBuf + dstVirW * dstVirH * 5/4,
                                        dstVirW, dstVirH, &clip,
                                        RkRgaGetRgaFormat(relRects.dst.format),0);
#else
            NormalRgaSetDstVirtualInfo(&rgaReg, (unsigned int)dstBuf,
                                        (unsigned int)dstBuf + dstVirW * dstVirH,
                                        (unsigned int)dstBuf + dstVirW * dstVirH * 5/4,
                                        dstVirW, dstVirH, &clip,
                                        RkRgaGetRgaFormat(relRects.dst.format),0);
#endif
        }
    } else {
        srcMmuFlag = ((srcFd != -1 && srcType) || srcFd == -1) ? 1 : 0;
        dstMmuFlag = ((dstFd != -1 && dstType) || dstFd == -1) ? 1 : 0;
#if defined(__arm64__) || defined(__aarch64__)
        NormalRgaSetSrcVirtualInfo(&rgaReg, srcFd != -1 ? srcFd : 0,
                                        (unsigned long)srcBuf, 
                                        (unsigned long)srcBuf + srcVirW * srcVirH,
                                        srcVirW, srcVirH,
                                        RkRgaGetRgaFormat(relRects.src.format),0);
        /*dst*/
        NormalRgaSetDstVirtualInfo(&rgaReg, dstFd != -1 ? dstFd : 0,
                                        (unsigned long)dstBuf,
                                        (unsigned long)dstBuf + dstVirW * dstVirH,
                                        dstVirW, dstVirH, &clip,
                                        RkRgaGetRgaFormat(relRects.dst.format),0);
#else
        NormalRgaSetSrcVirtualInfo(&rgaReg, srcFd != -1 ? srcFd : 0,
                                        (unsigned int)srcBuf, 
                                        (unsigned int)srcBuf + srcVirW * srcVirH,
                                        srcVirW, srcVirH,
                                        RkRgaGetRgaFormat(relRects.src.format),0);
        /*dst*/
        NormalRgaSetDstVirtualInfo(&rgaReg, dstFd != -1 ? dstFd : 0,
                                        (unsigned int)dstBuf,
                                        (unsigned int)dstBuf + dstVirW * dstVirH,
                                        dstVirW, dstVirH, &clip,
                                        RkRgaGetRgaFormat(relRects.dst.format),0);
#endif
    }

    NormalRgaSetSrcActiveInfo(&rgaReg, srcActW, srcActH, srcXPos, srcYPos);
    NormalRgaSetDstActiveInfo(&rgaReg, dstActW, dstActH, dstXPos, dstYPos);

    /*mode*/
    NormalRgaSetBitbltMode(&rgaReg, scaleMode, rotateMode, orientation, ditherEn, 0, 0);

    if (srcMmuFlag || dstMmuFlag) {
        NormalRgaMmuInfo(&rgaReg, 1, 0, 0, 0, 0, 2);
        NormalRgaMmuFlag(&rgaReg, srcMmuFlag, dstMmuFlag);
    }

    if(ioctl(ctx->rgaFd, RGA_BLIT_SYNC, &rgaReg)) {
        printf(" %s(%d) RGA_BLIT fail: %s",__FUNCTION__, __LINE__,strerror(errno));
        ALOGE(" %s(%d) RGA_BLIT fail: %s",__FUNCTION__, __LINE__,strerror(errno));
    }

    if (ctx->mLogOnce)
        ctx->mLogOnce = 0;

    return 0;
}

int RgaBlit(void *src,
                    buffer_handle_t dst, drm_rga_t *rects, int rotation, int blend)
{
    //check rects
    //check buffer_handle_t with rects
    struct rgaContext *ctx = rgaCtx;
    int srcVirW,srcVirH,srcActW,srcActH,srcXPos,srcYPos;
    int dstVirW,dstVirH,dstActW,dstActH,dstXPos,dstYPos;
    int scaleMode,rotateMode,orientation,ditherEn;
    int srcType,dstType,srcMmuFlag,dstMmuFlag;
    int planeAlpha;
    int dstFd = -1;
    int srcFd = -1;
    int ret = 0;
    drm_rga_t tmpRects,relRects;
    struct rga_req rgaReg;
    bool perpixelAlpha;
    void *srcBuf = NULL;
    void *dstBuf = NULL;
    RECT clip;

	if (!ctx) {
		ALOGE("Try to use uninit rgaCtx=%p",ctx);
		return -ENODEV;
	}

    memset(&rgaReg, 0, sizeof(struct rga_req));

    srcType = dstType = srcMmuFlag = dstMmuFlag = 0;

    if (!rects) {
        ALOGE("%d:Has not user rects for render", __LINE__);
        return ret;
    }

    if (rects->src.wstride <= 0) {
        ALOGE("%d:Has invalid rects for render", __LINE__);
        return -EINVAL;
    }

    ret = NormalRgaGetRects(NULL, dst, &srcType, &dstType, &tmpRects);
    if (ret) {
        ALOGE("%d:Has not rects for render", __LINE__);
        return ret;
    }

    if (rects->dst.wstride > 0)
        memcpy(&relRects, rects, sizeof(drm_rga_t));
    else {
        memcpy(&(relRects.src), &(rects->src), sizeof(rga_rect_t));
        memcpy(&(relRects.dst), &(tmpRects.dst), sizeof(rga_rect_t));
    }

    srcBuf = src;
    if (!srcBuf) {
        ALOGE("%d:src has not fd and address for render", __LINE__);
        return ret;
    }

    RkRgaGetHandleMapAddress(dst, &dstBuf);
    RkRgaGetHandleFd(dst, &dstFd);
    if (dstFd == -1 && !dstBuf) {
        ALOGE("%d:dst has not fd and address for render", __LINE__);
        return ret;
    }

    if (dstFd == 0 && !dstBuf) {
        ALOGE("dstFd is zero, now driver not support");
        return -EINVAL;
    } else
        dstFd = -1;

    planeAlpha = (blend & 0xFF0000) >> 16;
    perpixelAlpha = relRects.src.format == HAL_PIXEL_FORMAT_RGBA_8888 ||
                    relRects.src.format == HAL_PIXEL_FORMAT_BGRA_8888;

    switch ((blend & 0xFFFF)) {
        case 0x0105:
            if (perpixelAlpha && planeAlpha < 255)
                NormalRgaSetAlphaEnInfo(&rgaReg, 1, 2, planeAlpha , 1, 9, 0);
            else if (perpixelAlpha)
                NormalRgaSetAlphaEnInfo(&rgaReg, 1, 1, 0, 1, 3, 0);
            else
                NormalRgaSetAlphaEnInfo(&rgaReg, 1, 0, planeAlpha , 0, 0, 0);
            break;

        case 0x0405:
            if (perpixelAlpha && planeAlpha < 255)
                NormalRgaSetAlphaEnInfo(&rgaReg, 1, 2, planeAlpha , 0, 0, 0);
            else if (perpixelAlpha)
                NormalRgaSetAlphaEnInfo(&rgaReg, 1, 1, 0, 0, 0, 0);
            else
                NormalRgaSetAlphaEnInfo(&rgaReg, 1, 0, planeAlpha , 0, 0, 0);
            break;

        case 0x0100:
        default:
            /* Tips: BLENDING_NONE is non-zero value, handle zero value as
             * BLENDING_NONE. */
            /* C = Cs
             * A = As */
            break;
    }


    switch (rotation) {
        case HAL_TRANSFORM_FLIP_H:
            orientation = 0;
            rotateMode = 2;
            srcVirW = relRects.src.wstride;
            srcVirH = relRects.src.height;
            srcXPos = relRects.src.xoffset;
            srcYPos = relRects.src.yoffset;
            srcActW = relRects.src.width;
            srcActH = relRects.src.height;

            dstVirW = relRects.dst.wstride;
            dstVirH = relRects.dst.height;
            dstXPos = relRects.dst.xoffset;
            dstYPos = relRects.dst.yoffset;
            dstActW = relRects.dst.width;
            dstActH = relRects.dst.height;
            break;
        case HAL_TRANSFORM_FLIP_V:
            orientation = 0;
            rotateMode = 3;
            srcVirW = relRects.src.wstride;
            srcVirH = relRects.src.height;
            srcXPos = relRects.src.xoffset;
            srcYPos = relRects.src.yoffset;
            srcActW = relRects.src.width;
            srcActH = relRects.src.height;

            dstVirW = relRects.dst.wstride;
            dstVirH = relRects.dst.height;
            dstXPos = relRects.dst.xoffset;
            dstYPos = relRects.dst.yoffset;
            dstActW = relRects.dst.width;
            dstActH = relRects.dst.height;
            break;
        case HAL_TRANSFORM_ROT_90:
            orientation = 90;
            rotateMode = 1;
            srcVirW = relRects.src.wstride;
            srcVirH = relRects.src.height;
            srcXPos = relRects.src.xoffset;
            srcYPos = relRects.src.yoffset;
            srcActW = relRects.src.width;
            srcActH = relRects.src.height;

            dstVirW = relRects.dst.wstride;
            dstVirH = relRects.dst.height;
            dstXPos = relRects.dst.width - 1;
            //dstYPos = relRects.dst.yoffset;
            dstYPos = 0;
            dstActW = relRects.dst.height;
            dstActH = relRects.dst.width;
            break;
        case HAL_TRANSFORM_ROT_180:
            orientation = 180;
            rotateMode = 1;
            srcVirW = relRects.src.wstride;
            srcVirH = relRects.src.height;
            srcXPos = relRects.src.xoffset;
            srcYPos = relRects.src.yoffset;
            srcActW = relRects.src.width;
            srcActH = relRects.src.height;

            dstVirW = relRects.dst.wstride;
            dstVirH = relRects.dst.height;
            dstXPos = relRects.dst.width - 1;
            dstYPos = relRects.dst.height - 1;
            dstActW = relRects.dst.width;
            dstActH = relRects.dst.height;
            break;
        case HAL_TRANSFORM_ROT_270:
            orientation = 270;
            rotateMode = 1;
            srcVirW = relRects.src.wstride;
            srcVirH = relRects.src.height;
            srcXPos = relRects.src.xoffset;
            srcYPos = relRects.src.yoffset;
            srcActW = relRects.src.width;
            srcActH = relRects.src.height;

            dstVirW = relRects.dst.wstride;
            dstVirH = relRects.dst.height;
            //dstXPos = relRects.dst.xoffset;
            dstXPos = 0;
            dstYPos = relRects.dst.height - 1;
            dstActW = relRects.dst.height;
            dstActH = relRects.dst.width;
            break;
        default:
            orientation = 0;
            rotateMode = 0;
            srcVirW = relRects.src.wstride;
            srcVirH = relRects.src.height;
            srcXPos = relRects.src.xoffset;
            srcYPos = relRects.src.yoffset;
            srcActW = relRects.src.width;
            srcActH = relRects.src.height;

            dstVirW = relRects.dst.wstride;
            dstVirH = relRects.dst.height;
            dstXPos = relRects.dst.xoffset;
            dstYPos = relRects.dst.yoffset;
            dstActW = relRects.dst.width;
            dstActH = relRects.dst.height;
            break;
    }

    clip.xmin = 0;
    clip.xmax = dstActW - 1;
    clip.ymin = 0;
    clip.ymax = dstActH - 1;

    scaleMode = 0;
    //scale up use bicubic
    if (srcActW / dstActW < 1 || srcActH / dstActH < 1)
        scaleMode = 2;

    /*
    if (scaleMode && (srcFormat == RK_FORMAT_RGBA_8888 ||
                                            srcFormat == RK_FORMAT_BGRA_8888)) {
        scale_mode = 0;     //  force change scale_mode to 0 ,for rga not support
    }*/

    ditherEn = (android::bytesPerPixel(relRects.src.format) 
                        != android::bytesPerPixel(relRects.src.format) ? 1 : 0);

    if (ctx->mVersion <= 1.003) {
        srcMmuFlag = dstMmuFlag = 1;

#if defined(__arm64__) || defined(__aarch64__)
        NormalRgaSetSrcVirtualInfo(&rgaReg, (unsigned long)srcBuf,
                                        (unsigned long)srcBuf + srcVirW * srcVirH, 
                                        (unsigned long)srcBuf + srcVirW * srcVirH * 5/4,
                                        srcVirW, srcVirH,
                                        RkRgaGetRgaFormat(relRects.src.format),0);
        /*dst*/
        NormalRgaSetDstVirtualInfo(&rgaReg, (unsigned long)dstBuf,
                                        (unsigned long)dstBuf + dstVirW * dstVirH,
                                        (unsigned long)dstBuf + dstVirW * dstVirH * 5/4,
                                        dstVirW, dstVirH, &clip,
                                        RkRgaGetRgaFormat(relRects.dst.format),0);
#else
        NormalRgaSetSrcVirtualInfo(&rgaReg, (unsigned int)srcBuf,
                                        (unsigned int)srcBuf + srcVirW * srcVirH, 
                                        (unsigned int)srcBuf + srcVirW * srcVirH * 5/4,
                                        srcVirW, srcVirH,
                                        RkRgaGetRgaFormat(relRects.src.format),0);
        /*dst*/
        NormalRgaSetDstVirtualInfo(&rgaReg, (unsigned int)dstBuf,
                                        (unsigned int)dstBuf + dstVirW * dstVirH,
                                        (unsigned int)dstBuf + dstVirW * dstVirH * 5/4,
                                        dstVirW, dstVirH, &clip,
                                        RkRgaGetRgaFormat(relRects.dst.format),0);
#endif
    } else if (ctx->mVersion < 2.0) {
        /*Src*/
        if (srcFd != -1) {
            srcMmuFlag = srcType ? 1 : 0;
            NormalRgaSetSrcVirtualInfo(&rgaReg, 0, 0, 0, srcVirW, srcVirH,
                                        RkRgaGetRgaFormat(relRects.src.format),0);
            NormalRgaSetFdsOffsets(&rgaReg, srcFd, 0, 0, 0);
        } else {
            srcMmuFlag = 1;

#if defined(__arm64__) || defined(__aarch64__)
            NormalRgaSetSrcVirtualInfo(&rgaReg, (unsigned long)srcBuf,
                                        (unsigned long)srcBuf + srcVirW * srcVirH, 
                                        (unsigned long)srcBuf + srcVirW * srcVirH * 5/4,
                                        srcVirW, srcVirH,
                                        RkRgaGetRgaFormat(relRects.src.format),0);
#else
            NormalRgaSetSrcVirtualInfo(&rgaReg, (unsigned int)srcBuf,
                                        (unsigned int)srcBuf + srcVirW * srcVirH, 
                                        (unsigned int)srcBuf + srcVirW * srcVirH * 5/4,
                                        srcVirW, srcVirH,
                                        RkRgaGetRgaFormat(relRects.src.format),0);
#endif
        }
        /*dst*/
        if (dstFd != -1) {
            dstMmuFlag = srcType ? 1 : 0;
            NormalRgaSetDstVirtualInfo(&rgaReg, 0, 0, 0, dstVirW, dstVirH, &clip,
                                        RkRgaGetRgaFormat(relRects.dst.format),0);
    	    /*src dst fd*/
            NormalRgaSetFdsOffsets(&rgaReg, 0, dstFd, 0, 0);
        } else {
            dstMmuFlag = 1;
#if defined(__arm64__) || defined(__aarch64__)
            NormalRgaSetDstVirtualInfo(&rgaReg, (unsigned long)dstBuf,
                                        (unsigned long)dstBuf + dstVirW * dstVirH,
                                        (unsigned long)dstBuf + dstVirW * dstVirH * 5/4,
                                        dstVirW, dstVirH, &clip,
                                        RkRgaGetRgaFormat(relRects.dst.format),0);
#else
            NormalRgaSetDstVirtualInfo(&rgaReg, (unsigned int)dstBuf,
                                        (unsigned int)dstBuf + dstVirW * dstVirH,
                                        (unsigned int)dstBuf + dstVirW * dstVirH * 5/4,
                                        dstVirW, dstVirH, &clip,
                                        RkRgaGetRgaFormat(relRects.dst.format),0);
#endif
        }
    } else {
        srcMmuFlag = ((srcFd != -1 && srcType) || srcFd == -1) ? 1 : 0;
        dstMmuFlag = ((dstFd != -1 && dstType) || dstFd == -1) ? 1 : 0;
#if defined(__arm64__) || defined(__aarch64__)
        NormalRgaSetSrcVirtualInfo(&rgaReg, srcFd != -1 ? srcFd : 0,
                                        (unsigned long)srcBuf, 
                                        (unsigned long)srcBuf + srcVirW * srcVirH,
                                        srcVirW, srcVirH,
                                        RkRgaGetRgaFormat(relRects.src.format),0);
        /*dst*/
        NormalRgaSetDstVirtualInfo(&rgaReg, dstFd != -1 ? dstFd : 0,
                                        (unsigned long)dstBuf,
                                        (unsigned long)dstBuf + dstVirW * dstVirH,
                                        dstVirW, dstVirH, &clip,
                                        RkRgaGetRgaFormat(relRects.dst.format),0);
#else
        NormalRgaSetSrcVirtualInfo(&rgaReg, srcFd != -1 ? srcFd : 0,
                                        (unsigned int)srcBuf, 
                                        (unsigned int)srcBuf + srcVirW * srcVirH,
                                        srcVirW, srcVirH,
                                        RkRgaGetRgaFormat(relRects.src.format),0);
        /*dst*/
        NormalRgaSetDstVirtualInfo(&rgaReg, dstFd != -1 ? dstFd : 0,
                                        (unsigned int)dstBuf,
                                        (unsigned int)dstBuf + dstVirW * dstVirH,
                                        dstVirW, dstVirH, &clip,
                                        RkRgaGetRgaFormat(relRects.dst.format),0);
#endif
    }

    NormalRgaSetSrcActiveInfo(&rgaReg, srcActW, srcActH, srcXPos, srcYPos);
    NormalRgaSetDstActiveInfo(&rgaReg, dstActW, dstActH, dstXPos, dstYPos);

    /*mode*/
    NormalRgaSetBitbltMode(&rgaReg, scaleMode, rotateMode, orientation, ditherEn, 0, 0);

    if (srcMmuFlag || dstMmuFlag) {
        NormalRgaMmuInfo(&rgaReg, 1, 0, 0, 0, 0, 2);
        NormalRgaMmuFlag(&rgaReg, srcMmuFlag, dstMmuFlag);
    }

    if(ioctl(ctx->rgaFd, RGA_BLIT_SYNC, &rgaReg)) {
        printf(" %s(%d) RGA_BLIT fail: %s",__FUNCTION__, __LINE__,strerror(errno));
        ALOGE(" %s(%d) RGA_BLIT fail: %s",__FUNCTION__, __LINE__,strerror(errno));
    }

    return 0;
}

int RgaBlit(buffer_handle_t src,
                            void *dst, drm_rga_t *rects, int rotation, int blend)
{
    //check rects
    //check buffer_handle_t with rects
    struct rgaContext *ctx = rgaCtx;
    int srcVirW,srcVirH,srcActW,srcActH,srcXPos,srcYPos;
    int dstVirW,dstVirH,dstActW,dstActH,dstXPos,dstYPos;
    int scaleMode,rotateMode,orientation,ditherEn;
    int srcType,dstType,srcMmuFlag,dstMmuFlag;
    int planeAlpha;
    int dstFd = -1;
    int srcFd = -1;
    int ret = 0;
    drm_rga_t tmpRects,relRects;
    struct rga_req rgaReg;
    bool perpixelAlpha;
    void *srcBuf = NULL;
    void *dstBuf = NULL;
    RECT clip;

	if (!ctx) {
		ALOGE("Try to use uninit rgaCtx=%p",ctx);
		return -ENODEV;
	}

    memset(&rgaReg, 0, sizeof(struct rga_req));

    srcType = dstType = srcMmuFlag = dstMmuFlag = 0;

    if (!rects) {
        ALOGE("%d:Has not user rects for render", __LINE__);
        return ret;
    }

    if (rects->dst.wstride <= 0) {
        ALOGE("%d:Has invalid rects for render", __LINE__);
        return -EINVAL;
    }

    ret = NormalRgaGetRects(src, NULL, &srcType, &dstType, &tmpRects);
    if (ret) {
        ALOGE("%d:Has not rects for render", __LINE__);
        return ret;
    }

    if (rects->src.wstride > 0)
        memcpy(&relRects, rects, sizeof(drm_rga_t));
    else {
        memcpy(&(relRects.src), &(tmpRects.src), sizeof(rga_rect_t));
        memcpy(&(relRects.dst), &(rects->dst), sizeof(rga_rect_t));
    }

    RkRgaGetHandleMapAddress(src, &srcBuf);
    RkRgaGetHandleFd(src, &srcFd);
    if (srcFd == -1 && !srcBuf) {
        ALOGE("%d:src has not fd and address for render", __LINE__);
        return ret;
    }

    if (srcFd == 0 && !srcBuf) {
        ALOGE("srcFd is zero, now driver not support");
        return -EINVAL;
    } else
        srcFd = -1;

    dstBuf = dst;
    if (!dstBuf) {
        ALOGE("%d:dst has not fd and address for render", __LINE__);
        return ret;
    }

    planeAlpha = (blend & 0xFF0000) >> 16;
    perpixelAlpha = relRects.src.format == HAL_PIXEL_FORMAT_RGBA_8888 ||
                    relRects.src.format == HAL_PIXEL_FORMAT_BGRA_8888;

    switch ((blend & 0xFFFF)) {
        case 0x0105:
            if (perpixelAlpha && planeAlpha < 255)
                NormalRgaSetAlphaEnInfo(&rgaReg, 1, 2, planeAlpha , 1, 9, 0);
            else if (perpixelAlpha)
                NormalRgaSetAlphaEnInfo(&rgaReg, 1, 1, 0, 1, 3, 0);
            else
                NormalRgaSetAlphaEnInfo(&rgaReg, 1, 0, planeAlpha , 0, 0, 0);
            break;

        case 0x0405:
            if (perpixelAlpha && planeAlpha < 255)
                NormalRgaSetAlphaEnInfo(&rgaReg, 1, 2, planeAlpha , 0, 0, 0);
            else if (perpixelAlpha)
                NormalRgaSetAlphaEnInfo(&rgaReg, 1, 1, 0, 0, 0, 0);
            else
                NormalRgaSetAlphaEnInfo(&rgaReg, 1, 0, planeAlpha , 0, 0, 0);
            break;

        case 0x0100:
        default:
            /* Tips: BLENDING_NONE is non-zero value, handle zero value as
             * BLENDING_NONE. */
            /* C = Cs
             * A = As */
            break;
    }


    switch (rotation) {
        case HAL_TRANSFORM_FLIP_H:
            orientation = 0;
            rotateMode = 2;
            srcVirW = relRects.src.wstride;
            srcVirH = relRects.src.height;
            srcXPos = relRects.src.xoffset;
            srcYPos = relRects.src.yoffset;
            srcActW = relRects.src.width;
            srcActH = relRects.src.height;

            dstVirW = relRects.dst.wstride;
            dstVirH = relRects.dst.height;
            dstXPos = relRects.dst.xoffset;
            dstYPos = relRects.dst.yoffset;
            dstActW = relRects.dst.width;
            dstActH = relRects.dst.height;
            break;
        case HAL_TRANSFORM_FLIP_V:
            orientation = 0;
            rotateMode = 3;
            srcVirW = relRects.src.wstride;
            srcVirH = relRects.src.height;
            srcXPos = relRects.src.xoffset;
            srcYPos = relRects.src.yoffset;
            srcActW = relRects.src.width;
            srcActH = relRects.src.height;

            dstVirW = relRects.dst.wstride;
            dstVirH = relRects.dst.height;
            dstXPos = relRects.dst.xoffset;
            dstYPos = relRects.dst.yoffset;
            dstActW = relRects.dst.width;
            dstActH = relRects.dst.height;
            break;
        case HAL_TRANSFORM_ROT_90:
            orientation = 90;
            rotateMode = 1;
            srcVirW = relRects.src.wstride;
            srcVirH = relRects.src.height;
            srcXPos = relRects.src.xoffset;
            srcYPos = relRects.src.yoffset;
            srcActW = relRects.src.width;
            srcActH = relRects.src.height;

            dstVirW = relRects.dst.wstride;
            dstVirH = relRects.dst.height;
            dstXPos = relRects.dst.width - 1;
            //dstYPos = relRects.dst.yoffset;
            dstYPos = 0;
            dstActW = relRects.dst.height;
            dstActH = relRects.dst.width;
            break;
        case HAL_TRANSFORM_ROT_180:
            orientation = 180;
            rotateMode = 1;
            srcVirW = relRects.src.wstride;
            srcVirH = relRects.src.height;
            srcXPos = relRects.src.xoffset;
            srcYPos = relRects.src.yoffset;
            srcActW = relRects.src.width;
            srcActH = relRects.src.height;

            dstVirW = relRects.dst.wstride;
            dstVirH = relRects.dst.height;
            dstXPos = relRects.dst.width - 1;
            dstYPos = relRects.dst.height - 1;
            dstActW = relRects.dst.width;
            dstActH = relRects.dst.height;
            break;
        case HAL_TRANSFORM_ROT_270:
            orientation = 270;
            rotateMode = 1;
            srcVirW = relRects.src.wstride;
            srcVirH = relRects.src.height;
            srcXPos = relRects.src.xoffset;
            srcYPos = relRects.src.yoffset;
            srcActW = relRects.src.width;
            srcActH = relRects.src.height;

            dstVirW = relRects.dst.wstride;
            dstVirH = relRects.dst.height;
            //dstXPos = relRects.dst.xoffset;
            dstXPos = 0;
            dstYPos = relRects.dst.height - 1;
            dstActW = relRects.dst.height;
            dstActH = relRects.dst.width;
            break;
        default:
            orientation = 0;
            rotateMode = 0;
            srcVirW = relRects.src.wstride;
            srcVirH = relRects.src.height;
            srcXPos = relRects.src.xoffset;
            srcYPos = relRects.src.yoffset;
            srcActW = relRects.src.width;
            srcActH = relRects.src.height;

            dstVirW = relRects.dst.wstride;
            dstVirH = relRects.dst.height;
            dstXPos = relRects.dst.xoffset;
            dstYPos = relRects.dst.yoffset;
            dstActW = relRects.dst.width;
            dstActH = relRects.dst.height;
            break;
    }

    clip.xmin = 0;
    clip.xmax = dstActW - 1;
    clip.ymin = 0;
    clip.ymax = dstActH - 1;

    scaleMode = 0;
    //scale up use bicubic
    if (srcActW / dstActW < 1 || srcActH / dstActH < 1)
        scaleMode = 2;

    /*
    if (scaleMode && (srcFormat == RK_FORMAT_RGBA_8888 ||
                                            srcFormat == RK_FORMAT_BGRA_8888)) {
        scale_mode = 0;     //  force change scale_mode to 0 ,for rga not support
    }*/

    ditherEn = (android::bytesPerPixel(relRects.src.format) 
                        != android::bytesPerPixel(relRects.src.format) ? 1 : 0);

    if (ctx->mVersion <= 1.003) {
        srcMmuFlag = dstMmuFlag = 1;

#if defined(__arm64__) || defined(__aarch64__)
        NormalRgaSetSrcVirtualInfo(&rgaReg, (unsigned long)srcBuf,
                                        (unsigned long)srcBuf + srcVirW * srcVirH, 
                                        (unsigned long)srcBuf + srcVirW * srcVirH * 5/4,
                                        srcVirW, srcVirH,
                                        RkRgaGetRgaFormat(relRects.src.format),0);
        /*dst*/
        NormalRgaSetDstVirtualInfo(&rgaReg, (unsigned long)dstBuf,
                                        (unsigned long)dstBuf + dstVirW * dstVirH,
                                        (unsigned long)dstBuf + dstVirW * dstVirH * 5/4,
                                        dstVirW, dstVirH, &clip,
                                        RkRgaGetRgaFormat(relRects.dst.format),0);
#else
        NormalRgaSetSrcVirtualInfo(&rgaReg, (unsigned int)srcBuf,
                                        (unsigned int)srcBuf + srcVirW * srcVirH, 
                                        (unsigned int)srcBuf + srcVirW * srcVirH * 5/4,
                                        srcVirW, srcVirH,
                                        RkRgaGetRgaFormat(relRects.src.format),0);
        /*dst*/
        NormalRgaSetDstVirtualInfo(&rgaReg, (unsigned int)dstBuf,
                                        (unsigned int)dstBuf + dstVirW * dstVirH,
                                        (unsigned int)dstBuf + dstVirW * dstVirH * 5/4,
                                        dstVirW, dstVirH, &clip,
                                        RkRgaGetRgaFormat(relRects.dst.format),0);
#endif
    } else if (ctx->mVersion < 2.0) {
        /*Src*/
        if (srcFd != -1) {
            srcMmuFlag = srcType ? 1 : 0;
            NormalRgaSetSrcVirtualInfo(&rgaReg, 0, 0, 0, srcVirW, srcVirH,
                                        RkRgaGetRgaFormat(relRects.src.format),0);
            NormalRgaSetFdsOffsets(&rgaReg, srcFd, 0, 0, 0);
        } else {
            srcMmuFlag = 1;

#if defined(__arm64__) || defined(__aarch64__)
            NormalRgaSetSrcVirtualInfo(&rgaReg, (unsigned long)srcBuf,
                                        (unsigned long)srcBuf + srcVirW * srcVirH, 
                                        (unsigned long)srcBuf + srcVirW * srcVirH * 5/4,
                                        srcVirW, srcVirH,
                                        RkRgaGetRgaFormat(relRects.src.format),0);
#else
            NormalRgaSetSrcVirtualInfo(&rgaReg, (unsigned int)srcBuf,
                                        (unsigned int)srcBuf + srcVirW * srcVirH, 
                                        (unsigned int)srcBuf + srcVirW * srcVirH * 5/4,
                                        srcVirW, srcVirH,
                                        RkRgaGetRgaFormat(relRects.src.format),0);
#endif
        }
        /*dst*/
        if (dstFd != -1) {
            dstMmuFlag = srcType ? 1 : 0;
            NormalRgaSetDstVirtualInfo(&rgaReg, 0, 0, 0, dstVirW, dstVirH, &clip,
                                        RkRgaGetRgaFormat(relRects.dst.format),0);
    	    /*src dst fd*/
            NormalRgaSetFdsOffsets(&rgaReg, 0, dstFd, 0, 0);
        } else {
            dstMmuFlag = 1;
#if defined(__arm64__) || defined(__aarch64__)
            NormalRgaSetDstVirtualInfo(&rgaReg, (unsigned long)dstBuf,
                                        (unsigned long)dstBuf + dstVirW * dstVirH,
                                        (unsigned long)dstBuf + dstVirW * dstVirH * 5/4,
                                        dstVirW, dstVirH, &clip,
                                        RkRgaGetRgaFormat(relRects.dst.format),0);
#else
            NormalRgaSetDstVirtualInfo(&rgaReg, (unsigned int)dstBuf,
                                        (unsigned int)dstBuf + dstVirW * dstVirH,
                                        (unsigned int)dstBuf + dstVirW * dstVirH * 5/4,
                                        dstVirW, dstVirH, &clip,
                                        RkRgaGetRgaFormat(relRects.dst.format),0);
#endif
        }
    } else {
        srcMmuFlag = ((srcFd != -1 && srcType) || srcFd == -1) ? 1 : 0;
        dstMmuFlag = ((dstFd != -1 && dstType) || dstFd == -1) ? 1 : 0;
#if defined(__arm64__) || defined(__aarch64__)
        NormalRgaSetSrcVirtualInfo(&rgaReg, srcFd != -1 ? srcFd : 0,
                                        (unsigned long)srcBuf, 
                                        (unsigned long)srcBuf + srcVirW * srcVirH,
                                        srcVirW, srcVirH,
                                        RkRgaGetRgaFormat(relRects.src.format),0);
        /*dst*/
        NormalRgaSetDstVirtualInfo(&rgaReg, dstFd != -1 ? dstFd : 0,
                                        (unsigned long)dstBuf,
                                        (unsigned long)dstBuf + dstVirW * dstVirH,
                                        dstVirW, dstVirH, &clip,
                                        RkRgaGetRgaFormat(relRects.dst.format),0);
#else
        NormalRgaSetSrcVirtualInfo(&rgaReg, srcFd != -1 ? srcFd : 0,
                                        (unsigned int)srcBuf, 
                                        (unsigned int)srcBuf + srcVirW * srcVirH,
                                        srcVirW, srcVirH,
                                        RkRgaGetRgaFormat(relRects.src.format),0);
        /*dst*/
        NormalRgaSetDstVirtualInfo(&rgaReg, dstFd != -1 ? dstFd : 0,
                                        (unsigned int)dstBuf,
                                        (unsigned int)dstBuf + dstVirW * dstVirH,
                                        dstVirW, dstVirH, &clip,
                                        RkRgaGetRgaFormat(relRects.dst.format),0);
#endif
    }

    NormalRgaSetSrcActiveInfo(&rgaReg, srcActW, srcActH, srcXPos, srcYPos);
    NormalRgaSetDstActiveInfo(&rgaReg, dstActW, dstActH, dstXPos, dstYPos);

    /*mode*/
    NormalRgaSetBitbltMode(&rgaReg, scaleMode, rotateMode, orientation, ditherEn, 0, 0);

    if (srcMmuFlag || dstMmuFlag) {
        NormalRgaMmuInfo(&rgaReg, 1, 0, 0, 0, 0, 2);
        NormalRgaMmuFlag(&rgaReg, srcMmuFlag, dstMmuFlag);
    }

    if(ioctl(ctx->rgaFd, RGA_BLIT_SYNC, &rgaReg)) {
        printf(" %s(%d) RGA_BLIT fail: %s",__FUNCTION__, __LINE__,strerror(errno));
        ALOGE(" %s(%d) RGA_BLIT fail: %s",__FUNCTION__, __LINE__,strerror(errno));
    }

    return 0;
}

int RgaBlit(void *src, void *dst,
                                      drm_rga_t *rects, int rotation, int blend)
{
    //check rects
    //check buffer_handle_t with rects
    struct rgaContext *ctx = rgaCtx;
    int srcVirW,srcVirH,srcActW,srcActH,srcXPos,srcYPos;
    int dstVirW,dstVirH,dstActW,dstActH,dstXPos,dstYPos;
    int scaleMode,rotateMode,orientation,ditherEn;
    int srcType,dstType,srcMmuFlag,dstMmuFlag;
    int planeAlpha;
    int dstFd = -1;
    int srcFd = -1;
    int ret = 0;
    drm_rga_t relRects;
    struct rga_req rgaReg;
    bool perpixelAlpha;
    void *srcBuf = NULL;
    void *dstBuf = NULL;
    RECT clip;

	if (!ctx) {
		ALOGE("Try to use uninit rgaCtx=%p",ctx);
		return -ENODEV;
	}

    if (rects && (ctx->mLogAlways || ctx->mLogOnce)) {
        ALOGD("Src:[%d,%d,%d,%d][%d,%d,%d]=>Dst:[%d,%d,%d,%d][%d,%d,%d]",
            rects->src.xoffset,rects->src.yoffset,
            rects->src.width, rects->src.height,
            rects->src.wstride,rects->src.format, rects->src.size,
            rects->dst.xoffset,rects->dst.yoffset,
            rects->dst.width, rects->dst.height,
            rects->dst.wstride,rects->dst.format, rects->dst.size);
    }

    memset(&rgaReg, 0, sizeof(struct rga_req));

    srcType = dstType = srcMmuFlag = dstMmuFlag = 0;

    if (!rects) {
        ALOGE("%d:Has not user rects for render", __LINE__);
        return ret;
    }

    if (rects->src.wstride <= 0 || rects->dst.wstride <= 0) {
        ALOGE("%d:Has invalid rects for render", __LINE__);
        return -EINVAL;
    }

    memcpy(&relRects, rects, sizeof(drm_rga_t));

    srcBuf = src;
    if (!srcBuf) {
        ALOGE("%d:src has not fd and address for render", __LINE__);
        return ret;
    }

    dstBuf = dst;
    if (!dstBuf) {
        ALOGE("%d:dst has not fd and address for render", __LINE__);
        return ret;
    }

    planeAlpha = (blend & 0xFF0000) >> 16;
    perpixelAlpha = relRects.src.format == HAL_PIXEL_FORMAT_RGBA_8888 ||
                    relRects.src.format == HAL_PIXEL_FORMAT_BGRA_8888;

    switch ((blend & 0xFFFF)) {
        case 0x0105:
            if (perpixelAlpha && planeAlpha < 255)
                NormalRgaSetAlphaEnInfo(&rgaReg, 1, 2, planeAlpha , 1, 9, 0);
            else if (perpixelAlpha)
                NormalRgaSetAlphaEnInfo(&rgaReg, 1, 1, 0, 1, 3, 0);
            else
                NormalRgaSetAlphaEnInfo(&rgaReg, 1, 0, planeAlpha , 0, 0, 0);
            break;

        case 0x0405:
            if (perpixelAlpha && planeAlpha < 255)
                NormalRgaSetAlphaEnInfo(&rgaReg, 1, 2, planeAlpha , 0, 0, 0);
            else if (perpixelAlpha)
                NormalRgaSetAlphaEnInfo(&rgaReg, 1, 1, 0, 0, 0, 0);
            else
                NormalRgaSetAlphaEnInfo(&rgaReg, 1, 0, planeAlpha , 0, 0, 0);
            break;

        case 0x0100:
        default:
            /* Tips: BLENDING_NONE is non-zero value, handle zero value as
             * BLENDING_NONE. */
            /* C = Cs
             * A = As */
            break;
    }


    switch (rotation) {
        case HAL_TRANSFORM_FLIP_H:
            orientation = 0;
            rotateMode = 2;
            srcVirW = relRects.src.wstride;
            srcVirH = relRects.src.height;
            srcXPos = relRects.src.xoffset;
            srcYPos = relRects.src.yoffset;
            srcActW = relRects.src.width;
            srcActH = relRects.src.height;

            dstVirW = relRects.dst.wstride;
            dstVirH = relRects.dst.height;
            dstXPos = relRects.dst.xoffset;
            dstYPos = relRects.dst.yoffset;
            dstActW = relRects.dst.width;
            dstActH = relRects.dst.height;
            break;
        case HAL_TRANSFORM_FLIP_V:
            orientation = 0;
            rotateMode = 3;
            srcVirW = relRects.src.wstride;
            srcVirH = relRects.src.height;
            srcXPos = relRects.src.xoffset;
            srcYPos = relRects.src.yoffset;
            srcActW = relRects.src.width;
            srcActH = relRects.src.height;

            dstVirW = relRects.dst.wstride;
            dstVirH = relRects.dst.height;
            dstXPos = relRects.dst.xoffset;
            dstYPos = relRects.dst.yoffset;
            dstActW = relRects.dst.width;
            dstActH = relRects.dst.height;
            break;
        case HAL_TRANSFORM_ROT_90:
            orientation = 90;
            rotateMode = 1;
            srcVirW = relRects.src.wstride;
            srcVirH = relRects.src.height;
            srcXPos = relRects.src.xoffset;
            srcYPos = relRects.src.yoffset;
            srcActW = relRects.src.width;
            srcActH = relRects.src.height;

            dstVirW = relRects.dst.wstride;
            dstVirH = relRects.dst.height;
            dstXPos = relRects.dst.width - 1;
            //dstYPos = relRects.dst.yoffset;
            dstYPos = 0;
            dstActW = relRects.dst.height;
            dstActH = relRects.dst.width;
            break;
        case HAL_TRANSFORM_ROT_180:
            orientation = 180;
            rotateMode = 1;
            srcVirW = relRects.src.wstride;
            srcVirH = relRects.src.height;
            srcXPos = relRects.src.xoffset;
            srcYPos = relRects.src.yoffset;
            srcActW = relRects.src.width;
            srcActH = relRects.src.height;

            dstVirW = relRects.dst.wstride;
            dstVirH = relRects.dst.height;
            dstXPos = relRects.dst.width - 1;
            dstYPos = relRects.dst.height - 1;
            dstActW = relRects.dst.width;
            dstActH = relRects.dst.height;
            break;
        case HAL_TRANSFORM_ROT_270:
            orientation = 270;
            rotateMode = 1;
            srcVirW = relRects.src.wstride;
            srcVirH = relRects.src.height;
            srcXPos = relRects.src.xoffset;
            srcYPos = relRects.src.yoffset;
            srcActW = relRects.src.width;
            srcActH = relRects.src.height;

            dstVirW = relRects.dst.wstride;
            dstVirH = relRects.dst.height;
            //dstXPos = relRects.dst.xoffset;
            dstXPos = 0;
            dstYPos = relRects.dst.height - 1;
            dstActW = relRects.dst.height;
            dstActH = relRects.dst.width;
            break;
        default:
            orientation = 0;
            rotateMode = 0;
            srcVirW = relRects.src.wstride;
            srcVirH = relRects.src.height;
            srcXPos = relRects.src.xoffset;
            srcYPos = relRects.src.yoffset;
            srcActW = relRects.src.width;
            srcActH = relRects.src.height;

            dstVirW = relRects.dst.wstride;
            dstVirH = relRects.dst.height;
            dstXPos = relRects.dst.xoffset;
            dstYPos = relRects.dst.yoffset;
            dstActW = relRects.dst.width;
            dstActH = relRects.dst.height;
            break;
    }

    clip.xmin = 0;
    clip.xmax = dstActW - 1;
    clip.ymin = 0;
    clip.ymax = dstActH - 1;

    scaleMode = 0;
    //scale up use bicubic
    if (srcActW / dstActW < 1 || srcActH / dstActH < 1)
        scaleMode = 2;

    /*
    if (scaleMode && (srcFormat == RK_FORMAT_RGBA_8888 ||
                                            srcFormat == RK_FORMAT_BGRA_8888)) {
        scale_mode = 0;     //  force change scale_mode to 0 ,for rga not support
    }*/

    ditherEn = (android::bytesPerPixel(relRects.src.format) 
                        != android::bytesPerPixel(relRects.src.format) ? 1 : 0);

    if (ctx->mVersion <= 1.003) {
        srcMmuFlag = dstMmuFlag = 1;

#if defined(__arm64__) || defined(__aarch64__)
        NormalRgaSetSrcVirtualInfo(&rgaReg, (unsigned long)srcBuf,
                                        (unsigned long)srcBuf + srcVirW * srcVirH, 
                                        (unsigned long)srcBuf + srcVirW * srcVirH * 5/4,
                                        srcVirW, srcVirH,
                                        RkRgaGetRgaFormat(relRects.src.format),0);
        /*dst*/
        NormalRgaSetDstVirtualInfo(&rgaReg, (unsigned long)dstBuf,
                                        (unsigned long)dstBuf + dstVirW * dstVirH,
                                        (unsigned long)dstBuf + dstVirW * dstVirH * 5/4,
                                        dstVirW, dstVirH, &clip,
                                        RkRgaGetRgaFormat(relRects.dst.format),0);
#else
        NormalRgaSetSrcVirtualInfo(&rgaReg, (unsigned int)srcBuf,
                                        (unsigned int)srcBuf + srcVirW * srcVirH, 
                                        (unsigned int)srcBuf + srcVirW * srcVirH * 5/4,
                                        srcVirW, srcVirH,
                                        RkRgaGetRgaFormat(relRects.src.format),0);
        /*dst*/
        NormalRgaSetDstVirtualInfo(&rgaReg, (unsigned int)dstBuf,
                                        (unsigned int)dstBuf + dstVirW * dstVirH,
                                        (unsigned int)dstBuf + dstVirW * dstVirH * 5/4,
                                        dstVirW, dstVirH, &clip,
                                        RkRgaGetRgaFormat(relRects.dst.format),0);
#endif
    } else if (ctx->mVersion < 2.0) {
        /*Src*/
        if (srcFd != -1) {
            srcMmuFlag = srcType ? 1 : 0;
            NormalRgaSetSrcVirtualInfo(&rgaReg, 0, 0, 0, srcVirW, srcVirH,
                                        RkRgaGetRgaFormat(relRects.src.format),0);
            NormalRgaSetFdsOffsets(&rgaReg, srcFd, 0, 0, 0);
        } else {
            srcMmuFlag = 1;

#if defined(__arm64__) || defined(__aarch64__)
            NormalRgaSetSrcVirtualInfo(&rgaReg, (unsigned long)srcBuf,
                                        (unsigned long)srcBuf + srcVirW * srcVirH, 
                                        (unsigned long)srcBuf + srcVirW * srcVirH * 5/4,
                                        srcVirW, srcVirH,
                                        RkRgaGetRgaFormat(relRects.src.format),0);
#else
            NormalRgaSetSrcVirtualInfo(&rgaReg, (unsigned int)srcBuf,
                                        (unsigned int)srcBuf + srcVirW * srcVirH, 
                                        (unsigned int)srcBuf + srcVirW * srcVirH * 5/4,
                                        srcVirW, srcVirH,
                                        RkRgaGetRgaFormat(relRects.src.format),0);
#endif
        }
        /*dst*/
        if (dstFd != -1) {
            dstMmuFlag = srcType ? 1 : 0;
            NormalRgaSetDstVirtualInfo(&rgaReg, 0, 0, 0, dstVirW, dstVirH, &clip,
                                        RkRgaGetRgaFormat(relRects.dst.format),0);
    	    /*src dst fd*/
            NormalRgaSetFdsOffsets(&rgaReg, 0, dstFd, 0, 0);
        } else {
            dstMmuFlag = 1;
#if defined(__arm64__) || defined(__aarch64__)
            NormalRgaSetDstVirtualInfo(&rgaReg, (unsigned long)dstBuf,
                                        (unsigned long)dstBuf + dstVirW * dstVirH,
                                        (unsigned long)dstBuf + dstVirW * dstVirH * 5/4,
                                        dstVirW, dstVirH, &clip,
                                        RkRgaGetRgaFormat(relRects.dst.format),0);
#else
            NormalRgaSetDstVirtualInfo(&rgaReg, (unsigned int)dstBuf,
                                        (unsigned int)dstBuf + dstVirW * dstVirH,
                                        (unsigned int)dstBuf + dstVirW * dstVirH * 5/4,
                                        dstVirW, dstVirH, &clip,
                                        RkRgaGetRgaFormat(relRects.dst.format),0);
#endif
        }
    } else {
        srcMmuFlag = ((srcFd != -1 && srcType) || srcFd == -1) ? 1 : 0;
        dstMmuFlag = ((dstFd != -1 && dstType) || dstFd == -1) ? 1 : 0;
#if defined(__arm64__) || defined(__aarch64__)
        NormalRgaSetSrcVirtualInfo(&rgaReg, srcFd != -1 ? srcFd : 0,
                                        (unsigned long)srcBuf, 
                                        (unsigned long)srcBuf + srcVirW * srcVirH,
                                        srcVirW, srcVirH,
                                        RkRgaGetRgaFormat(relRects.src.format),0);
        /*dst*/
        NormalRgaSetDstVirtualInfo(&rgaReg, dstFd != -1 ? dstFd : 0,
                                        (unsigned long)dstBuf,
                                        (unsigned long)dstBuf + dstVirW * dstVirH,
                                        dstVirW, dstVirH, &clip,
                                        RkRgaGetRgaFormat(relRects.dst.format),0);
#else
        NormalRgaSetSrcVirtualInfo(&rgaReg, srcFd != -1 ? srcFd : 0,
                                        (unsigned int)srcBuf, 
                                        (unsigned int)srcBuf + srcVirW * srcVirH,
                                        srcVirW, srcVirH,
                                        RkRgaGetRgaFormat(relRects.src.format),0);
        /*dst*/
        NormalRgaSetDstVirtualInfo(&rgaReg, dstFd != -1 ? dstFd : 0,
                                        (unsigned int)dstBuf,
                                        (unsigned int)dstBuf + dstVirW * dstVirH,
                                        dstVirW, dstVirH, &clip,
                                        RkRgaGetRgaFormat(relRects.dst.format),0);
#endif
    }

    NormalRgaSetSrcActiveInfo(&rgaReg, srcActW, srcActH, srcXPos, srcYPos);
    NormalRgaSetDstActiveInfo(&rgaReg, dstActW, dstActH, dstXPos, dstYPos);

    /*mode*/
    NormalRgaSetBitbltMode(&rgaReg, scaleMode, rotateMode, orientation, ditherEn, 0, 0);

    if (srcMmuFlag || dstMmuFlag) {
        NormalRgaMmuInfo(&rgaReg, 1, 0, 0, 0, 0, 2);
        NormalRgaMmuFlag(&rgaReg, srcMmuFlag, dstMmuFlag);
    }

    if(ioctl(ctx->rgaFd, RGA_BLIT_SYNC, &rgaReg)) {
        printf(" %s(%d) RGA_BLIT fail: %s",__FUNCTION__, __LINE__,strerror(errno));
        ALOGE(" %s(%d) RGA_BLIT fail: %s",__FUNCTION__, __LINE__,strerror(errno));
    }

    if (ctx->mLogOnce)
        ctx->mLogOnce = 0;

    return 0;
}

int NormalRgaScale()
{
    return 1;
}

int NormalRgaRoate()
{
    return 1;
}

int NormalRgaRoateScale()
{
    return 1;
}
