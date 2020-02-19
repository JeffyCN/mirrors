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

#define LOG_NDEBUG 0
#define LOG_TAG "rgaTestCache"

#include <stdint.h>
#include <sys/types.h>
#include <math.h>
#include <fcntl.h>
#include <utils/misc.h>
#include <signal.h>
#include <time.h>

#include <cutils/properties.h>

#ifndef ANDROID_8

#include <binder/IPCThreadState.h>

#endif
#include <utils/Atomic.h>
#include <utils/Errors.h>
#include <utils/Log.h>

#include <ui/PixelFormat.h>
#include <ui/Rect.h>
#include <ui/Region.h>
#include <ui/DisplayInfo.h>
#include <ui/GraphicBufferMapper.h>
#include <RockchipRga.h>

#include <gui/ISurfaceComposer.h>

#include <GLES/gl.h>
#include <GLES/glext.h>
#include <EGL/eglext.h>

#include <stdint.h>
#include <sys/types.h>

//#include <system/window.h>

#include <utils/Thread.h>

#include <EGL/egl.h>
#include <GLES/gl.h>

#include <malloc.h>

///////////////////////////////////////////////////////
//#include "../drmrga.h"
#include <hardware/hardware.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>

#include <sys/mman.h>
#include <linux/stddef.h>
#include "RockchipFileOps.h"
///////////////////////////////////////////////////////

using namespace android;

int main()
{
    int ret = 0;
    int srcWidth,srcHeight,srcFormat;
    int dstWidth,dstHeight,dstFormat;
	
	void *src = NULL;
    void *dst = NULL;
	void *src2 = NULL;
    void *dst2 = NULL;
	
	/********** SrcInfo set **********/
    srcWidth = 1280;
    srcHeight = 720;
	srcFormat = HAL_PIXEL_FORMAT_RGBA_8888;
	
	/********** DstInfo set **********/
    dstWidth = 1280;
    dstHeight = 720;
	dstFormat = HAL_PIXEL_FORMAT_RGBA_8888;

	/********** apply for src_buffer **********/
#ifdef ANDROID_7_DRM
    sp<GraphicBuffer> gbs(new GraphicBuffer(srcWidth,srcHeight,srcFormat,
        GRALLOC_USAGE_SW_WRITE_OFTEN | GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_HW_FB));
#else
    sp<GraphicBuffer> gbs(new GraphicBuffer(srcWidth,srcHeight,srcFormat,
        GRALLOC_USAGE_SW_WRITE_OFTEN | GRALLOC_USAGE_SW_READ_OFTEN));
#endif

    if (gbs->initCheck()) {
        printf("GraphicBuffer_src1 error : %s\n",strerror(errno));
        return ret;
    } else
        printf("GraphicBuffer_src1 ok \n");
	
#ifdef ANDROID_7_DRM
    sp<GraphicBuffer> gbs2(new GraphicBuffer(srcWidth,srcHeight,srcFormat,
        GRALLOC_USAGE_SW_WRITE_OFTEN | GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_HW_FB));
#else
    sp<GraphicBuffer> gbs2(new GraphicBuffer(srcWidth,srcHeight,srcFormat,
        GRALLOC_USAGE_SW_WRITE_OFTEN | GRALLOC_USAGE_SW_READ_OFTEN));
#endif
    if (gbs2->initCheck()) {
        printf("GraphicBuffer_src2 error : %s\n",strerror(errno));
        return ret;
    } else
        printf("GraphicBuffer_src2 ok\n");

	/********** write data to src_buffer or init buffer**********/
    ret = gbs->lock(GRALLOC_USAGE_SW_WRITE_OFTEN | GRALLOC_USAGE_SW_READ_OFTEN, (void**)&src);
    if (ret) {
        printf("lock buffer_src1 error : %s\n",strerror(errno));
        return ret;
    } else 
        printf("lock buffer_src1 ok\n");
	
	char* buf = (char *)src;
	
    struct timeval tpend1, tpend2;
	long usec1 = 0;
	//gettimeofday(&tpend1, NULL);
	
    memset(buf,0x00,4*1280*720);
	
    //gettimeofday(&tpend2, NULL);
	//usec1 = 1000000 * (tpend2.tv_sec - tpend1.tv_sec) + (tpend2.tv_usec - tpend1.tv_usec);
	//printf("gralloc_cost_time=%ld us\n", usec1);
    ret = gbs->unlock();
	if (ret) {
		printf("unlock buffer_src1 error : %s\n",strerror(errno));
		return ret;
	} else 
		printf("unlock buffer_src1 %s \n","ok");
	
	/********** write data to src_buffer or init buffer**********/
    ret = gbs2->lock(GRALLOC_USAGE_SW_WRITE_OFTEN | GRALLOC_USAGE_SW_READ_OFTEN, (void**)&src2);
    if (ret) {
        printf("lock buffer_src2 error : %s\n",strerror(errno));
        return ret;
    } else 
        printf("lock buffer_src2 ok : \n");
	
	char* buf2 = (char *)src2;
	
	//usec1 = 0;
	//gettimeofday(&tpend1, NULL);
	
    memset(buf2,0x55,4*1280*720);
	
    //gettimeofday(&tpend2, NULL);
	//usec1 = 1000000 * (tpend2.tv_sec - tpend1.tv_sec) + (tpend2.tv_usec - tpend1.tv_usec);
	//printf("gralloc_cost_time=%ld us\n", usec1);
    ret = gbs->unlock();
	if (ret) {
		printf("unlock buffer_src2 error : %s\n",strerror(errno));
		return ret;
	} else 
		printf("unlock buffer_src2 %s \n","ok");
	
	printf("gralloc_buf1_address=%p \n", buf);
	printf("gralloc_buf2_address=%p \n", buf2);
	usleep(50);
	
	ret = gbs->lock(GRALLOC_USAGE_SW_WRITE_OFTEN | GRALLOC_USAGE_SW_READ_OFTEN, (void**)&src);
    if (ret) {
        printf("lock buffer_src error : %s\n",strerror(errno));
        return ret;
    } else 
        printf("lock buffer_src ok \n");
	
	buf = (char *)src;
	
    ret = gbs2->lock(GRALLOC_USAGE_SW_WRITE_OFTEN | GRALLOC_USAGE_SW_READ_OFTEN, (void**)&src2);
    if (ret) {
        printf("lock buffer_src2 error : %s\n",strerror(errno));
        return ret;
    } else 
        printf("lock buffer_src2 ok\n");
	
	buf2 = (char *)src2;
	
	usec1 = 0;
	gettimeofday(&tpend1, NULL);
	
    memcpy(buf,buf2,4*1280*720);
	
    gettimeofday(&tpend2, NULL);
	usec1 = 1000000 * (tpend2.tv_sec - tpend1.tv_sec) + (tpend2.tv_usec - tpend1.tv_usec);
	printf("<<<<<------ memcpy gralloc cost_time=%ld us ------>>>>>\n", usec1);
	
	ret = gbs->unlock();
	if (ret) {
        printf("unlock buffer_src error : %s\n",strerror(errno));
        return ret;
    } else 
        printf("unlock buffer_src ok\n");
	ret = gbs2->unlock();
	if (ret) {
        printf("unlock buffer_src2 error : %s\n",strerror(errno));
        return ret;
    } else 
        printf("unlock buffer_src2 ok\n");
	
	
	/********** apply for dst_buffer **********/
    dst = malloc(dstWidth * dstHeight * 4);
    if (!dst) {
        free(src);
        return -ENOMEM;
    }
	dst2 = malloc(dstWidth * dstHeight * 4);
    if (!dst2) {
        free(src);
        return -ENOMEM;
    }
	
	/********** write data to dst_buffer or init buffer **********/
    buf = (char *)dst;
	printf("malloc_address1=%p \n", buf);
	buf2 = (char *)dst2;
	printf("malloc_address2=%p \n", buf2);
	
	usec1 = 0;
	struct timeval tpend3, tpend4;
	//gettimeofday(&tpend3, NULL);
	
    memset(buf,0x00,4*1280*720);
	
    //gettimeofday(&tpend4, NULL);
	//usec1 = 1000000 * (tpend4.tv_sec - tpend3.tv_sec) + (tpend4.tv_usec - tpend3.tv_usec) ;
	//ALOGD("malloc_cost_time=%ld us\n", usec1);
	//printf("malloc_cost_time=%ld us\n", usec1);
	
	//usec1 = 0;
	//gettimeofday(&tpend3, NULL);
	
    memset(buf2,0x55,4*1280*720);
	
    //gettimeofday(&tpend4, NULL);
	//usec1 = 1000000 * (tpend4.tv_sec - tpend3.tv_sec) + (tpend4.tv_usec - tpend3.tv_usec) ;
	//printf("malloc_cost_time=%ld us\n", usec1);
	
	usec1 = 0;
	gettimeofday(&tpend3, NULL);
	
    memcpy(buf2,buf,4*1280*720);
	
    gettimeofday(&tpend4, NULL);
	usec1 = 1000000 * (tpend4.tv_sec - tpend3.tv_sec) + (tpend4.tv_usec - tpend3.tv_usec) ;
	printf("<<<<<------ memcpy malloc  cost_time=%ld us ------>>>>>\n\n", usec1);

	free(dst);
	dst = NULL;
	buf = NULL;
	free(dst2);
	dst2 = NULL;
	buf2 = NULL;
	
	printf("threadloop\n");
    usleep(500000);

    return 0;
}
