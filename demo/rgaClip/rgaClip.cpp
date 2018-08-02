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

    srcWidth = 1280;
    srcHeight = 720;
	srcFormat = RK_FORMAT_RGBA_8888;

    dstWidth = 360;
    dstHeight = 360;
	dstFormat = RK_FORMAT_RGBA_8888;
	//dstFormat = RK_FORMAT_YCrCb_420_SP;

	RockchipRga rkRga;
	rkRga.RkRgaInit();
	/********** apply for src buffer and dst buffer **********/
    //char* buf = NULL;
	ret = rkRga.RkRgaGetAllocBuffer(&bo_src, srcWidth, srcHeight, 32);
	ret = rkRga.RkRgaGetAllocBuffer(&bo_dst, dstWidth, dstHeight, 32);

	/********** map buffer_address to userspace **********/
	rkRga.RkRgaGetMmap(&bo_src);
	rkRga.RkRgaGetMmap(&bo_dst);

	/********** read data from *.bin file **********/   
#if 1
    get_buf_from_file(bo_src.ptr, srcFormat, srcWidth, srcHeight, 0);
#else
    memset(buf,0x55,4*1280*720);
#endif

#if 1
    //get_buf_from_file(bo_dst.ptr, dstFormat, dstWidth, dstHeight, 1);
#else
    memset(buf,0x00,4*360*360);
#endif

    while(1) {
		/********** rga_info_t Init **********/
    	rga_info_t src;
    	rga_info_t dst;
		
    	memset(&src, 0, sizeof(rga_info_t));
    	src.fd = -1;
    	src.mmuFlag = 1;
		
    	memset(&dst, 0, sizeof(rga_info_t));
    	dst.fd = -1;
    	dst.mmuFlag = 1;
		
		/********** get src_Fd **********/
    	ret = rkRga.RkRgaGetBufferFd(&bo_src, &src.fd);
        printf("src.fd =%d \n",src.fd);
        if (ret) {
            printf("rgaGetsrcFd fail : %s\n", strerror(errno));
        }
		/********** get dst_Fd **********/
		ret = rkRga.RkRgaGetBufferFd(&bo_dst, &dst.fd);
		printf("dst.fd =%d \n",dst.fd);
        if (ret) {
            printf("rgaGetdstFd error : %s\n", strerror(errno));
        }
		/********** if not fd, try to check phyAddr and virAddr **************/
        if(src.fd <= 0|| dst.fd <= 0)
		{
		/********** check phyAddr and virAddr ,if none to get virAddr **********/
			if (( src.phyAddr != 0 || src.virAddr != 0 ) || src.hnd != NULL ){
				//ret = RkRgaGetHandleMapAddress( gbs->handle, &src.virAddr );
				printf("src.virAddr =%p\n",src.virAddr);
				if(!src.virAddr){
					printf("err! src has not fd and address for render ,Stop!\n");
					break;
				}
			}
			
			/********** check phyAddr and virAddr ,if none to get virAddr **********/
			if (( dst.phyAddr != 0 || dst.virAddr != 0 ) || dst.hnd != NULL ){
				//ret = RkRgaGetHandleMapAddress( gbd->handle, &dst.virAddr );
				printf("dst.virAddr =%p\n",dst.virAddr);
				if(!dst.virAddr){
					printf("err! dst has not fd and address for render ,Stop!\n");
					break;
				}		
			}
		}

        /********** set the rect_info **********/
        rga_set_rect(&src.rect, 400,200,dstWidth,dstHeight,srcWidth/*stride*/,srcHeight,srcFormat);
        rga_set_rect(&dst.rect, 0,0,dstWidth,dstHeight,dstWidth/*stride*/,dstHeight,dstFormat);
		
		/************ set the rga_mod ,rotation\composition\scale\copy .... **********/
		//src.blend = 0xff0105;
		//src.rotation = HAL_TRANSFORM_FLIP_H;
		//src.rotation = HAL_TRANSFORM_ROT_270;
		
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
            //char* dstbuf = NULL;
            output_buf_data_to_file(bo_dst.ptr, dstFormat, dstWidth, dstHeight, 0);
        }
        printf("threadloop\n");
        usleep(500000);
	break;
    }
    return 0;
}
