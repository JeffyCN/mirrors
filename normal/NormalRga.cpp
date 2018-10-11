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
#include "NormalRga.h"
#include "NormalRgaContext.h"
//#include "../GraphicBuffer.h"
//#include "../RgaApi.h"
#include <sys/ioctl.h> 
//#include <cutils/properties.h>

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
			DEBUG("malloc fail:%s. \n",strerror(errno));
			goto mallocErr;
		}
	} else {
		ctx = rgaCtx;
		DEBUG("Had init the rga dev ctx = %p \n",ctx);
		goto init;
	}

	fd = open("/dev/rga", O_RDWR, 0);
	if (fd < 0) {
		ret = -ENODEV;
		DEBUG("failed to open device or no device point:%s. \n",strerror(errno));
		goto drmOpenErr;
	}
	ctx->rgaFd = fd;

	ret = ioctl(fd, RGA_GET_VERSION, buf);
	ctx->mVersion = atof(buf);
	fprintf(stderr, "librga:RGA_GET_VERSION:%s,%f \n", buf, ctx->mVersion);

	NormalRgaInitTables();

	fprintf(stderr, "ctx=%p,ctx->rgaFd=%d \n",ctx, ctx->rgaFd );
	rgaCtx = ctx;

init:
	//android_atomic_inc(&refCount);
	*context = (void *)ctx;
	return ret;

drmOpenErr:
	free(ctx);
mallocErr:
	return ret;
}

int NormalRgaClose(void *context)
{
	struct rgaContext *ctx = rgaCtx;

	if (!ctx) {
		DEBUG("Try to exit uninit rgaCtx=%p \n", ctx);
		return -ENODEV;
	}

	if (!context) {
		DEBUG("Try to uninit rgaCtx=%p \n", context);
		return -ENODEV;
	}

	if (context != ctx) {
		DEBUG("Try to exit wrong ctx=%p \n",ctx);
		return -ENODEV;
	}

	if (refCount <= 0) {
		DEBUG("This can not be happened \n");
		return 0;
	}

	if (refCount > 0)
	//if (refCount > 0 && android_atomic_dec(&refCount) != 1)
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

int RgaBlit(rga_info *src, rga_info *dst, rga_info *src1)
{
	//check rects
	//check buffer_handle_t with rects
	struct rgaContext *ctx = rgaCtx;
	int srcVirW,srcVirH,srcActW,srcActH,srcXPos,srcYPos;
	int dstVirW,dstVirH,dstActW,dstActH,dstXPos,dstYPos;
	//int src1VirW,src1VirH,src1ActW,src1ActH,src1XPos,src1YPos;
	int scaleMode,rotateMode,orientation,ditherEn;
	int srcType,dstType,src1Type,srcMmuFlag,dstMmuFlag,src1MmuFlag;
	int planeAlpha;
	int dstFd = -1;
	int srcFd = -1;
	int src1Fd = -1;
	int rotation;
    int stretch = 0;
    float hScale = 1;
    float vScale = 1;
	int ret = 0;
	rga_rect_t relSrcRect,tmpSrcRect,relDstRect,tmpDstRect;
	rga_rect_t relSrc1Rect,tmpSrc1Rect;
	struct rga_req rgaReg,tmprgaReg;
	unsigned int blend;
	unsigned int yuvToRgbMode;
	bool perpixelAlpha = 0;
	void *srcBuf = NULL;
	void *dstBuf = NULL;
	void *src1Buf = NULL;
	RECT_t clip;

	if (!ctx) {
		DEBUG("Try to use uninit rgaCtx=%p \n",ctx);
		return -ENODEV;
	}

	memset(&rgaReg, 0, sizeof(struct rga_req));

	srcType = dstType = srcMmuFlag = dstMmuFlag = 0;
	src1Type = src1MmuFlag = 0;
	rotation = 0;
	blend = 0;
	yuvToRgbMode = 0;
    
	if (!src && !dst && !src1) {
		DEBUG("src = %p, dst = %p, src1 = %p \n", src, dst, src1);
		return -EINVAL;
	}

	if (!src && !dst) {
		DEBUG("src = %p, dst = %p \n", src, dst);
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
  /*
	if (src && src->hnd) {
        if(src->fd <= 0 ){
    		//ret = RkRgaGetHandleFd(src->hnd, &srcFd);
    		if (ret) {
    			DEBUG("dst handle get fd fail ret = %d,hnd=%p", ret, &src->hnd);
    			return ret;
    		}
        }
		if (!isRectValid(relSrcRect)) {
			ret = NormalRgaGetRect(src->hnd, &tmpSrcRect);
			if (ret){
			    DEBUG("src handleGetRect fail ,ret = %d,hnd=%p", ret, &src->hnd);   
				return ret;
			}
			memcpy(&relSrcRect, &tmpSrcRect, sizeof(rga_rect_t));
		}
		NormalRgaGetMmuType(src->hnd, &srcType);
	}

	if (dst && dst->hnd) {
        if(src->fd <= 0 ){
    		//ret = RkRgaGetHandleFd(dst->hnd, &dstFd);
    		if (ret) {
    			DEBUG("dst handle get fd fail ret = %d,hnd=%p", ret, &dst->hnd);
    			return ret;
    		}
        }
		if (!isRectValid(relDstRect)) {
			ret = NormalRgaGetRect(dst->hnd, &tmpDstRect);
			if (ret){
			    DEBUG("dst handleGetRect fail ,ret = %d,hnd=%p", ret, &dst->hnd);   
				return ret;
			}
			memcpy(&relDstRect, &tmpDstRect, sizeof(rga_rect_t));
		}
		NormalRgaGetMmuType(dst->hnd, &dstType);
	}
*/
	if (src && srcFd < 0)
		srcFd = src->fd;

	if (src && src->phyAddr)
		srcBuf = src->phyAddr;
	else if (src && src->virAddr)
		srcBuf = src->virAddr;
	//else if (src && src->hnd)
		//ret = RkRgaGetHandleMapAddress(src->hnd, &srcBuf);

	if (srcFd == -1 && !srcBuf) {
		DEBUG("%d:src has not fd and address for render \n", __LINE__);
		return ret;
	}

	if (srcFd == 0 && !srcBuf) {
		DEBUG("srcFd is zero, now driver not support \n");
		return -EINVAL;
	}

	if (srcFd == 0)
		srcFd = -1;

	if (dst && dstFd < 0)
		dstFd = dst->fd;
    
    DEBUG("dstFd = %.2d , phyAddr = %x , virAddr = %x \n",dstFd,dst->phyAddr,dst->virAddr);

	if (dst && dst->phyAddr)
		dstBuf = dst->phyAddr;
	else if (dst && dst->virAddr)
		dstBuf = dst->virAddr;
	//else if (dst && dst->hnd)
		//ret = RkRgaGetHandleMapAddress(dst->hnd, &dstBuf);
    
	if (dst && dstFd == -1 && !dstBuf) {
		DEBUG("%d:dst has not fd and address for render \n", __LINE__);
		return ret;
	}

	if (dst && dstFd == 0 && !dstBuf) {
		DEBUG("dstFd is zero, now driver not support \n");
		return -EINVAL;
	}

	if (dstFd == 0)
		dstFd = -1;

	if (src1Fd == 0)
		src1Fd = -1;

	planeAlpha = (blend & 0xFF0000) >> 16;
	perpixelAlpha = relSrcRect.format == RK_FORMAT_RGBA_8888 ||
		relSrcRect.format == RK_FORMAT_BGRA_8888;

    DEBUG("blend = %x , perpixelAlpha = %d \n",blend ,perpixelAlpha);

	switch ((blend & 0xFFFF)) {
		case 0x0105:
			if (perpixelAlpha && planeAlpha < 255){
				NormalRgaSetAlphaEnInfo(&rgaReg, 1, 2, planeAlpha , 1, 9, 0);
			}else if (perpixelAlpha)
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

	if (relDstRect.hstride == 0)
		relDstRect.hstride = relDstRect.height;

	//if (relSrcRect.format == HAL_PIXEL_FORMAT_YCrCb_NV12_10)
	//	    relSrcRect.wstride = relSrcRect.wstride * 5 / 4;

	if (src) {
		ret = checkRectForRga(relSrcRect);
		if (ret) {
			DEBUG("[%s,%d]Error srcRect \n", __func__, __LINE__);
			return ret;
		}
	}

	if (dst) {
		ret = checkRectForRga(relDstRect);
		if (ret) {
			DEBUG("[%s,%d]Error dstRect \n", __func__, __LINE__);
			return ret;
		}
	}

	if (src && dst) {
		hScale = (float)relSrcRect.width / relDstRect.width;
		vScale = (float)relSrcRect.height / relDstRect.height;
		if (rotation == HAL_TRANSFORM_ROT_90 || rotation == HAL_TRANSFORM_ROT_270) {
			hScale = (float)relSrcRect.width / relDstRect.height;
			vScale = (float)relSrcRect.height / relDstRect.width;
		}
		if (hScale < 1/16 || hScale > 16 || vScale < 1/16 || vScale > 16) {
			DEBUG("Error scale[%f,%f] line %d \n", hScale, vScale, __LINE__);
			return -EINVAL;
		}
		if (ctx->mVersion <= 2.0 && (hScale < 1/8 ||
					hScale > 8 || vScale < 1/8 || vScale > 8)) {
			DEBUG("Error scale[%f,%f] line %d \n", hScale, vScale, __LINE__);
			return -EINVAL;
		}
		if (ctx->mVersion <= 1.003 && (hScale < 1/2 || vScale < 1/2)) {
			DEBUG("e scale[%f,%f] ver[%f] \n", hScale, vScale, ctx->mVersion);
			return -EINVAL;
		}
	}
    scaleMode = 0;
    stretch = (hScale != 1.0f) || (vScale != 1.0f);
    //scale up use bicubic
	if (hScale < 1 || vScale < 1)
    {
		scaleMode = 2;
        if((src->format == RK_FORMAT_RGBA_8888  ||src->format == RK_FORMAT_BGRA_8888)){
            scaleMode = 0;     //  force change scale_mode to 0 ,for rga not support
        }
	}

    DEBUG("scaleMode = %d , stretch = %d; \n",scaleMode,stretch);

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
			rotateMode = stretch;
			srcVirW = relSrcRect.wstride;
			srcVirH = relSrcRect.hstride;
			srcXPos = relSrcRect.xoffset;
			srcYPos = relSrcRect.yoffset;
			srcActW = relSrcRect.width;
			srcActH = relSrcRect.height;

			dstVirW = relDstRect.wstride;
			dstVirH = relDstRect.hstride;
			dstXPos = relDstRect.xoffset;
			dstYPos = relDstRect.yoffset;
			dstActW = relDstRect.width;
			dstActH = relDstRect.height;
			break;
	}

	clip.xmin = 0;
	clip.xmax = dstVirW - 1;
	clip.ymin = 0;
	clip.ymax = dstVirH - 1;
	

	ditherEn = (bytesPerPixel(relSrcRect.format) 
			!= bytesPerPixel(relSrcRect.format) ? 1 : 0);

    DEBUG("rgaVersion = %lf  , ditherEn =%d \n",ctx->mVersion,ditherEn);
    
    if (ctx->mVersion <= (float)1.003) {
        srcMmuFlag = dstMmuFlag = 1;

//#if defined(__arm64__) || defined(__aarch64__)
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
/*
#else
        NormalRgaSetSrcVirtualInfo(&rgaReg, (unsigned long)srcBuf,
        		(unsigned int)srcBuf + srcVirW * srcVirH, 
        		(unsigned int)srcBuf + srcVirW * srcVirH * 5/4,
        		srcVirW, srcVirH,
        		RkRgaGetRgaFormat(relSrcRect.format),0);
        NormalRgaSetDstVirtualInfo(&rgaReg, (unsigned long)dstBuf,
        		(unsigned int)dstBuf + dstVirW * dstVirH,
        		(unsigned int)dstBuf + dstVirW * dstVirH * 5/4,
        		dstVirW, dstVirH, &clip,
        		RkRgaGetRgaFormat(relDstRect.format),0);
        
#endif
*/
        } else if (ctx->mVersion < (float)1.6) {
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
//#if defined(__arm64__) || defined(__aarch64__)
            	NormalRgaSetSrcVirtualInfo(&rgaReg, (unsigned long)srcBuf,
            			(unsigned long)srcBuf + srcVirW * srcVirH, 
            			(unsigned long)srcBuf + srcVirW * srcVirH * 5/4,
            			srcVirW, srcVirH,
            			RkRgaGetRgaFormat(relSrcRect.format),0);
/*
#else
            	NormalRgaSetSrcVirtualInfo(&rgaReg, (unsigned int)srcBuf,
            			(unsigned int)srcBuf + srcVirW * srcVirH, 
            			(unsigned int)srcBuf + srcVirW * srcVirH * 5/4,
            			srcVirW, srcVirH,
            			RkRgaGetRgaFormat(relSrcRect.format),0);
#endif
*/
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
//#if defined(__arm64__) || defined(__aarch64__)
            	NormalRgaSetDstVirtualInfo(&rgaReg, (unsigned long)dstBuf,
            			(unsigned long)dstBuf + dstVirW * dstVirH,
            			(unsigned long)dstBuf + dstVirW * dstVirH * 5/4,
            			dstVirW, dstVirH, &clip,
            			RkRgaGetRgaFormat(relDstRect.format),0);
/*
#else
            	NormalRgaSetDstVirtualInfo(&rgaReg, (unsigned int)dstBuf,
            			(unsigned int)dstBuf + dstVirW * dstVirH,
            			(unsigned int)dstBuf + dstVirW * dstVirH * 5/4,
            			dstVirW, dstVirH, &clip,
            			RkRgaGetRgaFormat(relDstRect.format),0);
#endif
*/
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

        
//#if defined(__arm64__) || defined(__aarch64__)
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
/*
#else
        NormalRgaSetSrcVirtualInfo(&rgaReg, srcFd != -1 ? srcFd : 0,
        		(unsigned int)srcBuf, 
        		(unsigned int)srcBuf + srcVirW * srcVirH,
        		srcVirW, srcVirH,
        		RkRgaGetRgaFormat(relSrcRect.format),0);
        NormalRgaSetDstVirtualInfo(&rgaReg, dstFd != -1 ? dstFd : 0,
        		(unsigned int)dstBuf,
        		(unsigned int)dstBuf + dstVirW * dstVirH,
        		dstVirW, dstVirH, &clip,
        		RkRgaGetRgaFormat(relDstRect.format),0);
#endif
*/
        }

	NormalRgaSetSrcActiveInfo(&rgaReg, srcActW, srcActH, srcXPos, srcYPos);
	NormalRgaSetDstActiveInfo(&rgaReg, dstActW, dstActH, dstXPos, dstYPos);
//    NormalRgaSetPatActiveInfo(&rgaReg, src1ActW, src1ActH, src1XPos, src1YPos);

	if (NormalRgaIsYuvFormat(RkRgaGetRgaFormat(relSrcRect.format)) &&
			NormalRgaIsRgbFormat(RkRgaGetRgaFormat(relDstRect.format)))
		yuvToRgbMode |= 0x1 << 0;

	if (NormalRgaIsRgbFormat(RkRgaGetRgaFormat(relSrcRect.format)) &&
			NormalRgaIsYuvFormat(RkRgaGetRgaFormat(relDstRect.format)))
		yuvToRgbMode |= 0x2 << 4;

	/*mode*/
	NormalRgaSetBitbltMode(&rgaReg, scaleMode, rotateMode, orientation,
			ditherEn, 0, yuvToRgbMode);

	if (srcMmuFlag || dstMmuFlag) {
		NormalRgaMmuInfo(&rgaReg, 1, 0, 0, 0, 0, 2);
		NormalRgaMmuFlag(&rgaReg, srcMmuFlag, dstMmuFlag);
	}

	DEBUG("srcMmuFlag = %d , dstMmuFlag = %d , rotateMode = %d \n", srcMmuFlag, dstMmuFlag,rotateMode);
    DEBUG("<<<<-------- rgaReg -------->>>>\n");
	NormalRgaLogOutRgaReq(rgaReg);

	if(ioctl(ctx->rgaFd, RGA_BLIT_SYNC, &rgaReg)) {
		DEBUG(" %s(%d) RGA_BLIT fail: %s \n",__FUNCTION__, __LINE__,strerror(errno));
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
	RECT_t  clip;

	if (!ctx) {
		DEBUG("Try to use uninit rgaCtx=%p \n",ctx);
		return -ENODEV;
	}

	memset(&rgaReg, 0, sizeof(struct rga_req));

	dstType = dstMmuFlag = 0;

	if (!dst) {
		DEBUG("src = %p, dst = %p \n", dst, dst);
		return -EINVAL;
	}

	if (dst) {
		color = dst->color;
		memcpy(&relDstRect, &dst->rect, sizeof(rga_rect_t));
	}

	dstFd = -1;

	if (relDstRect.hstride == 0)
		relDstRect.hstride = relDstRect.height;
/*
	if (dst && dst->hnd) {
		//ret = RkRgaGetHandleFd(dst->hnd, &dstFd);
		if (ret) {
			DEBUG("dst handle get fd fail ret = %d,hnd=%p", ret, &dst->hnd);
			DEBUG("-dst handle get fd fail ret = %d,hnd=%p", ret, &dst->hnd);
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
*/

	if (dst && dstFd < 0)
		dstFd = dst->fd;

	if (dst && dst->phyAddr)
		dstBuf = dst->phyAddr;
	else if (dst && dst->virAddr)
		dstBuf = dst->virAddr;
	//else if (dst && dst->hnd)
		//ret = RkRgaGetHandleMapAddress(dst->hnd, &dstBuf);

	if (dst && dstFd == -1 && !dstBuf) {
		DEBUG("%d:dst has not fd and address for render \n", __LINE__);
		return ret;
	}

	if (dst && dstFd == 0 && !dstBuf) {
		DEBUG("dstFd is zero, now driver not support \n");
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
//#if defined(__arm64__) || defined(__aarch64__)
		/*dst*/
		NormalRgaSetDstVirtualInfo(&rgaReg, (unsigned long)dstBuf,
				(unsigned long)dstBuf + dstVirW * dstVirH,
				(unsigned long)dstBuf + dstVirW * dstVirH * 5/4,
				dstVirW, dstVirH, &clip,
				RkRgaGetRgaFormat(relDstRect.format),0);
/*
#else
		NormalRgaSetDstVirtualInfo(&rgaReg, (unsigned int)dstBuf,
				(unsigned int)dstBuf + dstVirW * dstVirH,
				(unsigned int)dstBuf + dstVirW * dstVirH * 5/4,
				dstVirW, dstVirH, &clip,
				RkRgaGetRgaFormat(relDstRect.format),0);
#endif
*/
	} else if (ctx->mVersion < 1.6 ) {
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
//#if defined(__arm64__) || defined(__aarch64__)
			NormalRgaSetDstVirtualInfo(&rgaReg, (unsigned long)dstBuf,
					(unsigned long)dstBuf + dstVirW * dstVirH,
					(unsigned long)dstBuf + dstVirW * dstVirH * 5/4,
					dstVirW, dstVirH, &clip,
					RkRgaGetRgaFormat(relDstRect.format),0);
/*
#else
			NormalRgaSetDstVirtualInfo(&rgaReg, (unsigned int)dstBuf,
					(unsigned int)dstBuf + dstVirW * dstVirH,
					(unsigned int)dstBuf + dstVirW * dstVirH * 5/4,
					dstVirW, dstVirH, &clip,
					RkRgaGetRgaFormat(relDstRect.format),0);
#endif
*/
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
//#if defined(__arm64__) || defined(__aarch64__)
		/*dst*/
		NormalRgaSetDstVirtualInfo(&rgaReg, dstFd != -1 ? dstFd : 0,
				(unsigned long)dstBuf,
				(unsigned long)dstBuf + dstVirW * dstVirH,
				dstVirW, dstVirH, &clip,
				RkRgaGetRgaFormat(relDstRect.format),0);
/*
#else
		NormalRgaSetDstVirtualInfo(&rgaReg, dstFd != -1 ? dstFd : 0,
				(unsigned int)dstBuf,
				(unsigned int)dstBuf + dstVirW * dstVirH,
				dstVirW, dstVirH, &clip,
				RkRgaGetRgaFormat(relDstRect.format),0);
#endif
*/
	}

	NormalRgaSetDstActiveInfo(&rgaReg, dstActW, dstActH, dstXPos, dstYPos);

	memset(&fillColor , 0x0, sizeof(COLOR_FILL));

	/*mode*/
	NormalRgaSetColorFillMode(&rgaReg, &fillColor, 0, 0, color, 0, 0, 0, 0, 0);

	if (dstMmuFlag) {
		NormalRgaMmuInfo(&rgaReg, 1, 0, 0, 0, 0, 2);
		NormalRgaMmuFlag(&rgaReg, dstMmuFlag, dstMmuFlag);
	}

	//DEBUG("%d,%d,%d", srcMmuFlag, dstMmuFlag,rotateMode);
	//NormalRgaLogOutRgaReq(rgaReg);

#ifndef RK3368	
#ifdef	ANDROID_7_DRM
	rgaReg.render_mode |= RGA_BUF_GEM_TYPE_DMA;
#endif
#endif

	if(ioctl(ctx->rgaFd, RGA_BLIT_SYNC, &rgaReg)) {
		DEBUG(" %s(%d) RGA_BLIT fail: %s \n",__FUNCTION__, __LINE__,strerror(errno));
	}

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
