/*
 * Copyright (C) 2016 Rockchip Electronics Co., Ltd.
 * Authors:
 *  Zhiqin Wei <wzq@rock-chips.com>
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
#define LOG_TAG "rgaSlt"

#include <stdint.h>
#include <sys/types.h>
#include <math.h>
#include <fcntl.h>
#include <utils/misc.h>
#include <signal.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <linux/stddef.h>

#include <utils/Atomic.h>
#include <utils/Errors.h>
#include <utils/Log.h>
#include <utils/Thread.h>
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

#define EN_GRAPHICBUFFER 0

using namespace android;

int main() {
    int ret = 0;
    int srcWidth,srcHeight,srcFormat;
    int dstWidth,dstHeight,dstFormat;

    srcWidth = 1280;
    srcHeight = 720;
    srcFormat = HAL_PIXEL_FORMAT_RGBA_8888;

    dstWidth = 1280;
    dstHeight = 720;
    dstFormat = HAL_PIXEL_FORMAT_RGBA_8888;

    RockchipRga& rkRga(RockchipRga::get());

#if EN_GRAPHICBUFFER
    char* buf = NULL;
//    GraphicBufferMapper &mgbMapper = GraphicBufferMapper::get();

    /********** apply for src_buffer **********/
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
    } else
        printf("GraphicBuffer_src %s \n","ok");

    /********** apply for dst_buffer **********/
#ifdef ANDROID_7_DRM
    sp<GraphicBuffer> gbd(new GraphicBuffer(dstWidth,dstHeight,dstFormat,
                                            GRALLOC_USAGE_SW_WRITE_OFTEN | GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_HW_FB));
#else
    sp<GraphicBuffer> gbd(new GraphicBuffer(dstWidth,dstHeight,dstFormat,
                                            GRALLOC_USAGE_SW_WRITE_OFTEN | GRALLOC_USAGE_SW_READ_OFTEN));
#endif

    if (gbd->initCheck()) {
        printf("GraphicBuffer_dst error : %s\n",strerror(errno));
        return ret;
    } else
        printf("GraphicBuffer_dst %s \n","ok");

    /********** map buffer_address to userspace **********/

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

    /********** write data to src_buffer or init buffer**********/
    ret = gbs->lock(GRALLOC_USAGE_SW_WRITE_OFTEN, (void**)&buf);

    if (ret) {
        printf("lock buffer_src error : %s\n",strerror(errno));
        return ret;
    } else
        printf("lock buffer_src %s \n","ok");

#if 1
    get_buf_from_file(buf, srcFormat, srcWidth, srcHeight, 0);
#else
    memset(buf,0x55,4*1280*720);
#endif
    ret = gbs->unlock();
    if (ret) {
        printf("unlock buffer_src error : %s\n",strerror(errno));
        return ret;
    } else
        printf("unlock buffer_src %s \n","ok");

    /********** write data to dst_buffer or init buffer **********/
    ret = gbd->lock(GRALLOC_USAGE_SW_WRITE_OFTEN, (void**)&buf);
    if (ret) {
        printf("lock buffer_dst error : %s\n",strerror(errno));
        return ret;
    } else
        printf("lock buffer_dst %s \n","ok");

    memset(buf,0x00,4*1280*720);

    ret = gbd->unlock();
    if (ret) {
        printf("unlock buffer_src error : %s\n",strerror(errno));
        return ret;
    } else
        printf("unlock buffer_src %s \n","ok");

    /********** rga_info_t Init **********/
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

    /********** get src_Fd **********/
    ret = rkRga.RkRgaGetBufferFd(gbs->handle, &src.fd);
    printf("src.fd =%d\n",src.fd);
    ALOGD("src.fd =%d\n",src.fd);
    if (ret) {
        printf("rgaGetsrcFd fail : %s,hnd=%p \n",
               strerror(errno),(void*)(gbd->handle));
    }
    /********** get dst_Fd **********/
    ret = rkRga.RkRgaGetBufferFd(gbd->handle, &dst.fd);
    printf("dst.fd =%d \n",dst.fd);
    if (ret) {
        printf("rgaGetdstFd error : %s,hnd=%p\n",
               strerror(errno),(void*)(gbd->handle));
    }
    /********** if not fd, try to check phyAddr and virAddr **************/
#ifndef RK3188
    if(src.fd <= 0|| dst.fd <= 0)

    {
        /********** check phyAddr and virAddr ,if none to get virAddr **********/
        if (( src.phyAddr != 0 || src.virAddr != 0 ) || src.hnd != NULL ) {
            ret = RkRgaGetHandleMapAddress( gbs->handle, &src.virAddr );
            printf("src.virAddr =%p\n",src.virAddr);
            if(!src.virAddr) {
                printf("err! src has not fd and address for render ,Stop!\n");
                break;
            }
        }

        /********** check phyAddr and virAddr ,if none to get virAddr **********/
        if (( dst.phyAddr != 0 || dst.virAddr != 0 ) || dst.hnd != NULL ) {
            ret = RkRgaGetHandleMapAddress( gbd->handle, &dst.virAddr );
            printf("dst.virAddr =%p\n",dst.virAddr);
            if(!dst.virAddr) {
                printf("err! dst has not fd and address for render ,Stop!\n");
                break;
            }
        }
    }
#endif /* #ifndef RK3188 */
#else
    char *src_va = NULL;
    char *dst_va = NULL;

    src_va = (char *)malloc(srcWidth*srcHeight*get_bpp_from_format(srcFormat));
    dst_va = (char *)malloc(dstWidth*dstHeight*get_bpp_from_format(dstFormat));

#if 1
    get_buf_from_file(src_va, srcFormat, srcWidth, srcHeight, 0);
#else
    memset(src_va,0x00,srcWidth*srcHeight*get_bpp_from_format(srcFormat));
#endif

#if 0
    get_buf_from_file(dst_va, dstFormat, dstWidth, dstHeight, 0);
#else
    memset(dst_va,0x00,dstWidth*dstHeight*get_bpp_from_format(dstFormat));
#endif

    /********** rga_info_t Init **********/
    rga_info_t src;
    rga_info_t dst;

    memset(&src, 0, sizeof(rga_info_t));
    memset(&dst, 0, sizeof(rga_info_t));

    src.virAddr = src_va;
    src.mmuFlag = 1;

    dst.virAddr =dst_va;
    dst.mmuFlag = 1;
#endif /* #if EN_GRAPHICBUFFER */

    while(1) {
        /********** set the rect_info **********/
        rga_set_rect(&src.rect, 0,0,srcWidth,srcHeight,srcWidth/*stride*/,srcHeight,srcFormat);
        rga_set_rect(&dst.rect, 0,0,dstWidth,dstHeight,dstWidth/*stride*/,dstHeight,dstFormat);

        /************ set the rga_mod ,rotation\composition\scale\copy .... **********/

        /********** call rga_Interface **********/
        ret = rkRga.RkRgaBlit(&src, &dst, NULL);
        if (ret) {
            printf("rgaFillColor error : %s\n",
                   strerror(errno));
        }

#if EN_GRAPHICBUFFER
        {
            /********** output buf data to file **********/
            char* dstbuf = NULL;

            ret = gbd->lock(GRALLOC_USAGE_SW_WRITE_OFTEN, (void**)&dstbuf);
            output_buf_data_to_file(dstbuf, dstFormat, dstWidth, dstHeight, 0);
            ret = gbd->unlock();

            /********** rga slt check **********/
            ret = gbd->lock(GRALLOC_USAGE_SW_WRITE_OFTEN, (void**)&dstbuf);
            if (ret) {
                printf("lock buffer_dst error : %s\n",strerror(errno));
                return ret;
            } else
                printf("lock buffer_dst %s \n","ok");

            ret = gbs->lock(GRALLOC_USAGE_SW_WRITE_OFTEN, (void**)&buf);
            if (ret) {
                printf("lock buffer_src error : %s\n",strerror(errno));
                return ret;
            } else
                printf("lock buffer_src %s \n","ok");

            int size = dstWidth * dstHeight * 4;
            unsigned int *pstd = (unsigned int *)buf;
            unsigned int *pnow = (unsigned int *)dstbuf;
            int errCount = 0;
            int rightCount = 0;
            printf("[  num   : srcInfo    dstInfo ] \n");
            for (int i = 0; i < size / 4; i++) {
                if (*pstd != *pnow) {
                    printf("[X%.8d:0x%x 0x%x]  ", i, *pstd,*pnow);
                    if (i % 4 == 0 )
                        printf("\n");
                    errCount ++;

                } else {
                    if (i % (640*1024) == 0)
                        printf("[Y%.8d:0x%.8x 0x%.8x]\n", i, *pstd,*pnow);
                    rightCount++;
                }
                pstd++;
                pnow++;
                if (errCount > 64)
                    break;
            }
            ret = gbs->unlock();
            if (ret) {
                printf("unlock buffer_src error : %s\n",strerror(errno));
                return ret;
            }

            ret = gbd->unlock();
            if (ret) {
                printf("unlock buffer_dst error : %s\n",strerror(errno));
                return ret;
            }

            printf("errCount=%d,rightCount=%d\n", errCount, rightCount);
            if(errCount != 0) {
                printf("rga slt err !! \n");
                return 1;
            } else {
                printf("rga slt sucess !! \n");
                return 0;
            }
        }
#else
        {
            /********** output buf data to file **********/
            output_buf_data_to_file(dst_va, dstFormat, dstWidth, dstHeight, 0);

            int size = dstWidth * dstHeight * 4;
            unsigned int *pstd = (unsigned int *)src_va;
            unsigned int *pnow = (unsigned int *)dst_va;
            int errCount = 0;
            int rightCount = 0;
            printf("[  num   : srcInfo    dstInfo ] \n");
            for (int i = 0; i < size / 4; i++) {
                if (*pstd != *pnow) {
                    printf("[X%.8d:0x%x 0x%x]  ", i, *pstd,*pnow);
                    if (i % 4 == 0 )
                        printf("\n");
                    errCount ++;

                } else {
                    if (i % (640*1024) == 0)
                        printf("[Y%.8d:0x%.8x 0x%.8x]\n", i, *pstd,*pnow);
                    rightCount++;
                }
                pstd++;
                pnow++;
                if (errCount > 64)
                    break;
            }

            printf("errCount=%d,rightCount=%d\n", errCount, rightCount);
            if(errCount != 0) {
                printf("rga slt err !! \n");
                return 1;
            } else {
                printf("rga slt sucess !! \n");
                return 0;
            }
        }
#endif /* #if EN_GRAPHICBUFFER */
        break;
    }

#if !EN_GRAPHICBUFFER
    free(src_va);
    free(dst_va);
#endif
    return 0;
}
