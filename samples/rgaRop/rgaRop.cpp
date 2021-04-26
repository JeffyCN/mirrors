/*
 * Copyright (C) 2018  Rockchip Electronics Co., Ltd.
 * Authors:
 *     lihuang <putin.li@rock-chips.com>
 *     libin <bin.li@rock-chips.com>
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
#define LOG_TAG "rgaBlit"

#include <stdint.h>
#include <sys/types.h>
#include <math.h>
#include <fcntl.h>
#include <utils/misc.h>
#include <signal.h>
#include <time.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <sys/mman.h>
#include <linux/stddef.h>

#include <utils/Thread.h>
#include <utils/Atomic.h>
#include <utils/Errors.h>
#include <utils/Log.h>
#include <cutils/properties.h>
#include <ui/PixelFormat.h>
#include <ui/Rect.h>
#include <ui/Region.h>
#include <ui/DisplayInfo.h>
#include <ui/GraphicBuffer.h>
#include <ui/GraphicBufferMapper.h>
#include <gui/ISurfaceComposer.h>
#include <hardware/hardware.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES/gl.h>
#include <GLES/glext.h>

#ifndef ANDROID_8
#include <binder/IPCThreadState.h>
#endif

#include "RockchipRga.h"
#include "RgaUtils.h"
///////////////////////////////////////////////////////
using namespace android;
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
    srcWidth = 480;
    srcHeight = 480;
    srcFormat = HAL_PIXEL_FORMAT_RGBA_8888;
    dstWidth = 480;
    dstHeight = 480;
    dstFormat = HAL_PIXEL_FORMAT_RGBA_8888;

    /******************************
     * Instantiation RockchipRga:
     * Singleton pattern instantiates an interface,you can use it to call RGA_interface
     ******************************/
    RockchipRga& rkRga(RockchipRga::get());

    /******************************
     * Instantiation GraphicBufferMapper:
     * Instantiation Android's tool of GraphicBufferMapper,
     * It provides a mapping of buffer address into a user's address space
     ******************************/
    //GraphicBufferMapper &mgbMapper = GraphicBufferMapper::get();
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
     * Map Buffer_address To Userspace:
     ******************************/
    /*
    #ifdef ANDROID_8
        buffer_handle_t importedHandle_src;
        buffer_handle_t importedHandle_dst;
        mgbMapper.importBuffer(gbs->handle, &importedHandle_src);
        mgbMapper.importBuffer(gbd->handle, &importedHandle_dst);
        gbs->handle = importedHandle_src;
        gbd->handle = importedHandle_dst;
    #else
        mgbMapper.registerBuffer(gbs->handle);
        mgbMapper.registerBuffer(gbd->handle);
    #endif
    */
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
    get_buf_from_file(buf, srcFormat, srcWidth, srcHeight, 1);
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
    get_buf_from_file(buf, dstFormat, dstWidth, dstHeight, 0);
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
         * Set The rga_mod:
         * Configure RGA work mode,here is Compositing Mode
         ******************************/
        /*
         * rop_code
         * 0x88 : Dest=Dest AND Src
         * 0xee : Dest=Dest OR Src
         * 0x55 : Dest=NOT Dest
         * 0x33 : Dest= NOT Src
         * 0xf6 : Dest=(Dest XOR Src)
         * 0xf9 : Dest=NOT (Dest XOR Src)
         */
        src.rop_code = 0x88; // src & dst

        /******************************
         * Calling Rga_Interface:
         * 1.Deliver two configured data structures to Rga_Interface and Waiting RGA to finish work.
         * 2.Currently using A + B = B mode ,it mean the output will write to the address of dst.
         * 3.Print the time-consuming of RGA
         ******************************/
        struct timeval tpend1, tpend2;
        long usec1 = 0;
        gettimeofday(&tpend1, NULL);

        ret = rkRga.RkRgaBlit(&src, &dst, NULL);
        if (ret) {
            printf("rgaFillColor error : %s,hnd=%p\n",
                   strerror(errno),(void*)(gbd->handle));
        }

        gettimeofday(&tpend2, NULL);
        usec1 = 1000 * (tpend2.tv_sec - tpend1.tv_sec) + (tpend2.tv_usec - tpend1.tv_usec) / 1000;
        printf("cost_time=%ld ms\n", usec1);
        {
            /******************************
             * Output Buf_data To File:
             * Print out the output as a file , can use 7YUV to check.
             ******************************/
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
