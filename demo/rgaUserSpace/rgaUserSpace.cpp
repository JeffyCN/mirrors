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
#define LOG_TAG "rgaUserSpace"

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

#include <GLES/gl.h>
#include <GLES/glext.h>
#include <EGL/eglext.h>

#include <stdint.h>
#include <sys/types.h>

//#include <system/window.h>

#include <utils/Thread.h>

#include <EGL/egl.h>
#include <GLES/gl.h>

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

    srcWidth = 1280;
    srcHeight = 720;
	srcFormat = HAL_PIXEL_FORMAT_RGBA_8888;

    dstWidth = 1280;
    dstHeight = 720;
	dstFormat = HAL_PIXEL_FORMAT_RGBA_8888;

	/********** apply for buffer **********/
    src = malloc(srcWidth * srcHeight * 4);
    if (!src)
        return -ENOMEM;
    
	/********** apply for buffer **********/
    dst = malloc(dstWidth * dstHeight * 4);
    if (!dst) {
        free(src);
        return -ENOMEM;
    }

    RockchipRga& rkRga(RockchipRga::get());
	
	/********** get data to src_buffer or init buffer**********/
    char* buf = (char *)src;
#if 1
    get_buf_from_file(buf, srcFormat, srcWidth, srcHeight, 1);
#else
    memset(buf,0x55,4*1280*720);
#endif

	/********** get data to dst_buffer or init buffer **********/
    buf = (char *)dst;
#if 1
    get_buf_from_file(buf, srcFormat, srcWidth, srcHeight, 0);
#else
    memset(buf,0x00,4*1280*720);
#endif
	
    while(1) {
		/********** rga_info_t Init **********/
    	rga_info_t rgasrc;
    	rga_info_t rgadst;
		
    	memset(&rgasrc, 0, sizeof(rga_info_t));
    	rgasrc.fd = -1;
    	rgasrc.mmuFlag = 1;
		rgasrc.virAddr = src;
		
    	memset(&rgadst, 0, sizeof(rga_info_t));
    	rgadst.fd = -1;
    	rgadst.mmuFlag = 1;
		rgadst.virAddr = dst;
		
        /********** set the rect_info **********/
        rga_set_rect(&rgasrc.rect, 0,0,srcWidth,srcHeight,srcWidth/*stride*/,srcHeight,srcFormat);
        rga_set_rect(&rgadst.rect, 0,0,dstWidth,dstHeight,dstWidth/*stride*/,dstHeight,dstFormat);
		
		/************ set the rga_mod ,rotation\composition\scale\copy .... **********/
		rgasrc.blend = 0xff0105;
	
        /********** call rga_Interface **********/
        struct timeval tpend1, tpend2;
		long usec1 = 0;
		gettimeofday(&tpend1, NULL);
        ret = rkRga.RkRgaBlit(&rgasrc, &rgadst, NULL);
        gettimeofday(&tpend2, NULL);
		usec1 = 1000 * (tpend2.tv_sec - tpend1.tv_sec) + (tpend2.tv_usec - tpend1.tv_usec) / 1000;
		printf("cost_time=%ld ms\n", usec1);

        if (ret) {
            printf("rgaFillColor error : %s\n",
                                            strerror(errno));
        }

        {
			/********** output buf data to file **********/
            char* dstbuf = (char *)dst;
            output_buf_data_to_file(dstbuf, dstFormat, dstWidth, dstHeight, 0);
        }
        printf("threadloop\n");
        usleep(500000);
	break;
    }
    return 0;
}
