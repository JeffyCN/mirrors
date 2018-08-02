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
#define LOG_TAG "rgaClip"

#include <stdint.h>
#include <sys/types.h>
#include <math.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/time.h>

#include <RockchipRga.h>

#include <stdint.h>
#include <sys/types.h>

///////////////////////////////////////////////////////
//#include "../drmrga.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>

#include <sys/mman.h>
#include <linux/stddef.h>
#include <RockchipFileOps.h>
///////////////////////////////////////////////////////

int main()
{
    int ret = 0;
    int srcWidth,srcHeight,srcFormat;
	int dstWidth,dstHeight,dstFormat;
	bo_t bo_src, bo_dst;

	void *src_v = NULL;
    void *dst_v = NULL;

    srcWidth = 1280;
    srcHeight = 720;
	srcFormat = RK_FORMAT_RGBA_8888;
	//srcFormat = RK_FORMAT_RGB_565;
	//srcFormat = RK_FORMAT_YCrCb_420_SP;

    dstWidth = 1280;
    dstHeight = 720;
	dstFormat = RK_FORMAT_RGBA_8888;
	//dstFormat = RK_FORMAT_YCrCb_420_SP;

	RockchipRga rkRga;
	rkRga.RkRgaInit();
	/********** apply for buffer **********/
    src_v = malloc(srcWidth * srcHeight * 4);
    if (!src_v)
        return -ENOMEM;
    
	/********** apply for buffer **********/
    dst_v = malloc(dstWidth * dstHeight * 4);
    if (!dst_v) {
        free(src_v);
        return -ENOMEM;
    }
	
	/********** get data to src_buffer or init buffer**********/
    char* buf = (char *)src_v;
#if 1
    get_buf_from_file(buf, srcFormat, srcWidth, srcHeight, 1);
#else
    memset(buf,0x55,4*1200*1920);
#endif

	/********** get data to dst_buffer or init buffer **********/
    buf = (char *)dst_v;
#if 1
    get_buf_from_file(buf, srcFormat, srcWidth, srcHeight, 0);
#else
    memset(buf,0x00,4*2560*1440);
#endif

    while(1) {
		/********** rga_info_t Init **********/
    	rga_info_t src;
    	rga_info_t dst;
		
    	memset(&src, 0, sizeof(rga_info_t));
    	src.fd = -1;
    	src.mmuFlag = 1;
		src.virAddr = src_v;
		
    	memset(&dst, 0, sizeof(rga_info_t));
    	dst.fd = -1;
    	dst.mmuFlag = 1;
		dst.virAddr = dst_v;

        /********** set the rect_info **********/
        rga_set_rect(&src.rect, 0,0,srcWidth,srcHeight,srcWidth/*stride*/,srcHeight,srcFormat);
        rga_set_rect(&dst.rect, 0,0,dstWidth,dstHeight,dstWidth/*stride*/,dstHeight,dstFormat);
		
		/************ set the rga_mod ,rotation\composition\scale\copy .... **********/

		
		/********** call rga_Interface **********/
        struct timeval tpend1, tpend2;
		long usec1 = 0;
		gettimeofday(&tpend1, NULL);
        ret = rkRga.RkRgaBlit(&src, &dst, NULL);
		gettimeofday(&tpend2, NULL);
		usec1 = 1000 * (tpend2.tv_sec - tpend1.tv_sec) + (tpend2.tv_usec - tpend1.tv_usec) / 1000;
		printf("cost_time=%ld ms\n", usec1);
        if (ret) {
            printf("rgaFillColor error : %s\n",
                                            strerror(errno));
        }

        {
			/********** output buf data to file **********/
            char* dstbuf = (char *)dst_v;
            output_buf_data_to_file(dstbuf, dstFormat, dstWidth, dstHeight, 0);
        }
        printf("threadloop\n");
        usleep(500000);
	break;
    }
    return 0;
}
