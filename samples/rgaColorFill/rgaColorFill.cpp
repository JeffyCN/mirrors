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
#define LOG_TAG "rgaBlit"

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

using namespace android;

int main() {
    int ret = 0;
    int dstWidth,dstHeight,dstFormat;
    char* buf = NULL;


    dstWidth = 1280;
    dstHeight = 720;
    dstFormat = HAL_PIXEL_FORMAT_RGBA_8888;

    /********** instantiation RockchipRga **********/
    RockchipRga& rkRga(RockchipRga::get());

    /********** instantiation GraphicBufferMapper **********/
//    GraphicBufferMapper &mgbMapper = GraphicBufferMapper::get();

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
        buffer_handle_t importedHandle_dst;
        mgbMapper.importBuffer(gbd->handle, &importedHandle_dst);
        gbd->handle = importedHandle_dst;
    #else
        mgbMapper.registerBuffer(gbd->handle);
    #endif
    */
    /********** write data to src_buffer or init buffer**********/
    ret = gbd->lock(GRALLOC_USAGE_SW_WRITE_OFTEN, (void**)&buf);
    if (ret) {
        printf("lock buffer_dst error : %s\n",strerror(errno));
        return ret;
    } else
        printf("lock buffer_dst %s \n","ok");


    memset(buf,0x00,4*1280*720);


    ret = gbd->unlock();
    if (ret) {
        printf("unlock buffer_dst error : %s\n",strerror(errno));
        return ret;
    } else
        printf("unlock buffer_dst %s \n","ok");

    while(1) {
        /********** rga_info_t Init **********/
        rga_info_t dst;


        memset(&dst, 0, sizeof(rga_info_t));
        dst.fd = -1;
        dst.mmuFlag = 1;
        dst.hnd = gbd->handle;

        /********** get dst_Fd **********/
        ret = rkRga.RkRgaGetBufferFd(gbd->handle, &dst.fd);
        printf("dst.fd =%d \n",dst.fd);
        if (ret) {
            printf("rgaGetdstFd error : %s,hnd=%p\n",
                   strerror(errno),(void*)(gbd->handle));
        }
        /********** fd is valid ? **************/
#ifndef RK3188
        if(dst.fd <= 0)
#endif
        {
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
        /********** set the rect_info **********/
        rga_set_rect(&dst.rect, 0,0,dstWidth,dstHeight,dstWidth/*stride*/,dstHeight,dstFormat);

        /************ set the rga_mod  **********/
        dst.color = 0xffff0000;
        //dst.color = 0xff00ff00;
        //dst.color = 0xff0000ff;

        /********** call rga_Interface **********/
        struct timeval tpend1, tpend2;
        long usec1 = 0;
        gettimeofday(&tpend1, NULL);

        ret = rkRga.RkRgaCollorFill(&dst);
        if (ret) {
            printf("rgaFillColor error : %s,hnd=%p\n",
                   strerror(errno),(void*)(gbd->handle));
        }

        gettimeofday(&tpend2, NULL);
        usec1 = 1000 * (tpend2.tv_sec - tpend1.tv_sec) + (tpend2.tv_usec - tpend1.tv_usec) / 1000;
        printf("cost_time=%ld ms\n", usec1);


        {
            /********** output buf data to file **********/
            char* dstbuf = NULL;
            ret = gbd->lock(GRALLOC_USAGE_SW_WRITE_OFTEN, (void**)&dstbuf);
            //for(int i =0; i < mHeight * 1.5; i++)
            //    memcpy(dstbuf + i * 2400,buf + i * 3000,2400);
            output_buf_data_to_file(dstbuf, dstFormat, dstWidth, dstHeight, 0);
            ret = gbd->unlock();
        }
        printf("threadloop\n");
        usleep(500000);
        break;
    }
    return 0;
}
