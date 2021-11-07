/*
 * Copyright (C) 2018  Rockchip Electronics Co., Ltd.
 * Authors:
 *     Yu Qiaowei <cerf.yu@rock-chips.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#define LOG_NDEBUG 0
#define LOG_TAG "rgaColorKey"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#include <sys/mman.h>
#include <sys/types.h>
#include <utils/misc.h>
#include <utils/Thread.h>
#include <linux/stddef.h>
#include <ui/GraphicBuffer.h>
#include <hardware/hardware.h>

#include "RockchipRga.h"
#include "RgaUtils.h"
/////////////////////////////////////////////////////
using namespace android;

int draw_color_chart(char *buf, int width, int height) {
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width/8; j++) {
            buf[(i*width*4) + j*4 + 0] = 0x11;   //R
            buf[(i*width*4) + j*4 + 1] = 0x11;   //G
            buf[(i*width*4) + j*4 + 2] = 0x11;   //B
            buf[(i*width*4) + j*4 + 3] = 0xff;   //A
        }
        for (int j = width/8; j < width/4; j++) {
            buf[(i*width*4) + j*4 + 0] = 0x33;
            buf[(i*width*4) + j*4 + 1] = 0x33;
            buf[(i*width*4) + j*4 + 2] = 0x33;
            buf[(i*width*4) + j*4 + 3] = 0xff;
        }
        for (int j = width/4; j < width/8*3; j++) {
            buf[(i*width*4) + j*4 + 0] = 0x66;
            buf[(i*width*4) + j*4 + 1] = 0x66;
            buf[(i*width*4) + j*4 + 2] = 0x66;
            buf[(i*width*4) + j*4 + 3] = 0xff;
        }
        for (int j = width/8*3; j < width/2; j++) {
            buf[(i*width*4) + j*4 + 0] = 0xab;
            buf[(i*width*4) + j*4 + 1] = 0xab;
            buf[(i*width*4) + j*4 + 2] = 0xab;
            buf[(i*width*4) + j*4 + 3] = 0xff;
        }
        for (int j = width/8*4; j < width/8*5; j++) {
            buf[(i*width*4) + j*4 + 0] = 0xff;
            buf[(i*width*4) + j*4 + 1] = 0xff;
            buf[(i*width*4) + j*4 + 2] = 0xff;
            buf[(i*width*4) + j*4 + 3] = 0xff;
        }
    }
    return 0;
}

int main() {
    /******************************
     * Define Variable:
     * Define variable to describe the frame
     *******************************/
    int ret = 0;
    int srcWidth,srcHeight,srcFormat;
    int dstWidth,dstHeight,dstFormat;
    char* buf = NULL;

    /******************************
     * Initialize variable:
     * Set up variable according to requirements
     ******************************/
    srcWidth = 1280;
    srcHeight = 720;
    srcFormat = HAL_PIXEL_FORMAT_RGBA_8888;
    dstWidth = 1280;
    dstHeight = 720;
    dstFormat = HAL_PIXEL_FORMAT_RGBA_8888;

    /******************************
     * Instantiation RockchipRga:
     * Singleton pattern instantiates an interface,you can use it to call RGA_interface
     ******************************/
    RockchipRga& rkRga(RockchipRga::get());

    /******************************
     * Apply For Src_buffer:
     ******************************/
#ifdef ANDROID_7_DRM
    sp<GraphicBuffer> gbs(new GraphicBuffer(srcWidth,srcHeight,srcFormat,
                                            GRALLOC_USAGE_SW_WRITE_OFTEN | GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_HW_FB));
#else
    sp<GraphicBuffer> gbs(new GraphicBuffer(srcWidth,srcHeight,srcFormat,
                                            GRALLOC_USAGE_SW_WRITE_OFTEN | GRALLOC_USAGE_SW_READ_OFTEN));
#endif
    if (gbs->initCheck()) {
        printf("GraphicBuffer_src error : %s\n",strerror(errno));
        return ret;
    } else {
        printf("GraphicBuffer_src %s \n","ok");
    }

    /******************************
     * Apply For Dst_buffer:
     ******************************/
#ifdef ANDROID_7_DRM
    sp<GraphicBuffer> gbd(new GraphicBuffer(dstWidth,dstHeight,dstFormat,
                                            GRALLOC_USAGE_SW_WRITE_OFTEN | GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_HW_FB));
#else
    sp<GraphicBuffer> gbd(new GraphicBuffer(dstWidth,dstHeight,dstFormat,
                                            GRALLOC_USAGE_SW_WRITE_OFTEN | GRALLOC_USAGE_SW_READ_OFTEN));
#endif

    /******************************
     * Validity Check:
     ******************************/
    if (gbd->initCheck()) {
        printf("GraphicBuffer_dst error : %s\n",strerror(errno));
        return ret;
    } else
        printf("GraphicBuffer_dst %s \n","ok");

    /******************************
     * Lock Src_buffer:
     ******************************/
    ret = gbs->lock(GRALLOC_USAGE_SW_WRITE_OFTEN, (void**)&buf);
    if (ret) {
        printf("lock buffer_src error : %s\n",strerror(errno));
        return ret;
    } else
    printf("lock buffer_src %s \n","ok");

    /******************************
     * Write Data To Src_buffer:
     ******************************/
    get_buf_from_file(buf, srcFormat, srcWidth, srcHeight, 0);
    /* Draw a color chart */
    draw_color_chart(buf, dstWidth, dstHeight);

    /******************************
     * Unlock Src_buffer:
     * You shoud immediately unlock when you have operated buffer.If you don't ,maybe cause memory leaks.
     ******************************/
    ret = gbs->unlock();
    if (ret) {
        printf("unlock buffer_src error : %s\n",strerror(errno));
        return ret;
    } else
        printf("unlock buffer_src %s \n","ok");

    /******************************
     * Lock Dst_buffer:
     ******************************/
    ret = gbd->lock(GRALLOC_USAGE_SW_WRITE_OFTEN, (void**)&buf);
    if (ret) {
        printf("lock buffer_dst error : %s\n",strerror(errno));
        return ret;
    } else
        printf("lock buffer_dst %s \n","ok");

    /******************************
     * Write Data To Dst_buffer:
     ******************************/
    get_buf_from_file(buf, dstFormat, dstWidth, dstHeight, 1);
//    memset(buf, 0x00, dstWidth*dstHeight*get_bpp_from_format(dstFormat));

    /******************************
     * Unlock Dst_buffer:
     * You shoud immediately unlock when you have operated buffer.If you don't ,maybe cause memory leaks.
     ******************************/
    ret = gbd->unlock();
    if (ret) {
        printf("unlock buffer_src error : %s\n",strerror(errno));
        return ret;
    } else
        printf("unlock buffer_src %s \n","ok");


    while(1) {
        /******************************
         * Instantiation rga_info_t:
         * Packaging information about source and target buffer.
         ******************************/
        rga_info_t src;
        rga_info_t dst;

        memset(&src, 0, sizeof(rga_info_t));
        src.fd = -1;
        src.mmuFlag = 1;
        src.hnd = gbs->handle;

        memset(&dst, 0, sizeof(rga_info_t));
        dst.fd = -1;
        dst.mmuFlag = 1;
        dst.hnd = gbd->handle;

        /******************************
         * Get Src_fd:
         * Calling RkRgaGetBufferFd interface of rkRga to get fd by handle
         ******************************/
        ret = rkRga.RkRgaGetBufferFd(gbs->handle, &src.fd);
        printf("src.fd =%d\n",src.fd);
        if (ret) {
            printf("rgaGetsrcFd fail : %s,hnd=%p \n",
                   strerror(errno),(void*)(gbd->handle));
        }

        /******************************
         * Get Dst_fd:
         * Calling RkRgaGetBufferFd interface of rkRga to get fd by handle
         ******************************/
        ret = rkRga.RkRgaGetBufferFd(gbd->handle, &dst.fd);
        printf("dst.fd =%d \n",dst.fd);
        if (ret) {
            printf("rgaGetdstFd error : %s,hnd=%p\n",
                   strerror(errno),(void*)(gbd->handle));
        }

        /******************************
         * If src.fd or dst.fd is illegal,attemp to get virtual address
         ******************************/
#ifndef RK3188
        if(src.fd <= 0|| dst.fd <= 0)
#endif
        {
            /******************************
             * Check PhyAddr And VirAddr ,If None To Get VirAddr:
             * If phyAddr or virAddr is illegal and handle isn't NULL ,
             * Calling RkRgaGetHandleMapAddress interface to get virAddr by handle
             ******************************/
            if (( src.phyAddr != 0 || src.virAddr != 0 ) || src.hnd != NULL ) {
                ret = RkRgaGetHandleMapAddress( gbs->handle, &src.virAddr );
                printf("src.virAddr =%p\n",src.virAddr);
                if(!src.virAddr) {
                    printf("err! src has not fd and address for render ,Stop!\n");
                    break;
                }
            }
            /******************************
             * Check PhyAddr And VirAddr ,If None To Get VirAddr:
             * If phyAddr or virAddr is illegal and handle isn't NULL ,
             * Calling RkRgaGetHandleMapAddress interface to get virAddr by handle
             ******************************/
            if (( dst.phyAddr != 0 || dst.virAddr != 0 ) || dst.hnd != NULL ) {
                ret = RkRgaGetHandleMapAddress( gbd->handle, &dst.virAddr );
                printf("dst.virAddr =%p\n",dst.virAddr);
                if(!dst.virAddr) {
                    printf("err! dst has not fd and address for render ,Stop!\n");
                    break;
                }
            }
        }

        /******************************
         * Set The Rect_info:
         * Calling rga_set_rect interface to set up data structure src.rect.
         ******************************/
        rga_set_rect(&src.rect, 0,0,srcWidth,srcHeight,srcWidth/*stride*/,srcHeight,srcFormat);
        rga_set_rect(&dst.rect, 0,0,dstWidth,dstHeight,dstWidth/*stride*/,dstHeight,dstFormat);

        /******************************
         * Enable RGA ColorKey mode.
         * Set color interval.
         ******************************/
        src.colorkey_en = 1;            /* Enable colorkey */
        src.colorkey_mode = 0;          /* Select mode : [0] normal [1] inverted */
        src.colorkey_min = 0xff333333;
        src.colorkey_max = 0xff666666;

        src.blend = 0xFF0105;

        struct timeval tpend1, tpend2;
        long usec1 = 0;
        gettimeofday(&tpend1, NULL);

        ret = rkRga.RkRgaBlit(&src, &dst, NULL);
        if (ret) {
            printf("rgaColorKey error : %s,hnd=%p\n",
                   strerror(errno),(void*)(gbd->handle));
        }

        gettimeofday(&tpend2, NULL);
        usec1 = 1000000 * (tpend2.tv_sec - tpend1.tv_sec) + (tpend2.tv_usec - tpend1.tv_usec);
        printf("cost_time=%ld us\n", usec1);
        {
            char* dstbuf = NULL;
            ret = gbd->lock(GRALLOC_USAGE_SW_WRITE_OFTEN, (void**)&dstbuf);
            output_buf_data_to_file(dstbuf, dstFormat, dstWidth, dstHeight, 0);
            ret = gbd->unlock();
        }
        printf("threadloop\n");
        usleep(500000);
        break;
    }

    return 0;
}
