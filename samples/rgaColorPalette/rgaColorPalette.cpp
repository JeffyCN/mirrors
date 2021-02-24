/*
 * Copyright (C) 2020 Rockchip Electronics Co., Ltd.
 * Authors:
 *  PutinLee <putin.lee@rock-chips.com>
 *  Cerf Yu <cerf.yu@rock-chips.com>
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
#define LOG_TAG "rgaColorPalette"

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include <gui/ISurfaceComposer.h>
#include <RockchipRga.h>
#include "RgaUtils.h"
#include "core/NormalRga.h"

#include "im2d_api/im2d.hpp"

using namespace android;

#define IS_IM2D_API 1

sp<GraphicBuffer> GraphicBuffer_Init(int width, int height,int format)
{
#ifdef ANDROID_7_DRM
    sp<GraphicBuffer> gb(new GraphicBuffer(width,height,format,
        GRALLOC_USAGE_SW_WRITE_OFTEN | GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_HW_FB | GRALLOC_USAGE_TO_USE_PHY_CONT));
#else
    sp<GraphicBuffer> gb(new GraphicBuffer(width,height,format,
        GRALLOC_USAGE_SW_WRITE_OFTEN | GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_TO_USE_PHY_CONT));
#endif

    if (gb->initCheck())
    {
        printf("GraphicBuffer check error : %s\n",strerror(errno));
        return NULL;
    }

    return gb;
}

/********** write data to buffer or init buffer**********/
int GraphicBuffer_Fill(sp<GraphicBuffer> gb, int flag, int index) {
    int ret;
    char* buf = NULL;
    ret = gb->lock(GRALLOC_USAGE_SW_WRITE_OFTEN, (void**)&buf);
    if (ret) {
        printf("lock buffer error : %s\n",strerror(errno));
        return -1;
    } else
        printf("lock buffer %s \n","ok");

    if(flag) {
        memset(buf,0xFF,get_bpp_from_format(RkRgaGetRgaFormatFromAndroid(gb->getPixelFormat()))*gb->getWidth()*gb->getHeight());
    }
    else {
        ret = get_buf_from_file(buf, RkRgaGetRgaFormatFromAndroid(gb->getPixelFormat()), gb->getWidth(), gb->getHeight(), index);
        if (!ret)
            printf("open file %s \n", "ok");
        else {
            printf ("open file %s \n", "fault");
            return -1;
        }
    }

    ret = gb->unlock();
    if (ret) {
        printf("unlock buffer error : %s\n",strerror(errno));
        return -1;
    } else
        printf("unlock buffer %s \n","ok");

    return 0;
}

int GraphicBuffer_Fill_LUT(sp<GraphicBuffer> gb, int flag, int index) {
    int ret, i;
    char* buf = NULL;
    ret = gb->lock(GRALLOC_USAGE_SW_WRITE_OFTEN, (void**)&buf);

    if (ret) {
        printf("lock buffer error : %s\n",strerror(errno));
        return -1;
    } else
        printf("lock buffer %s \n","ok");

	/*Draw lut table*/
    if(flag) {
          for (i = 0; i < 32*4; i+=4) {
              buf[i+0] = 0xFF;
              buf[i+1] = 0x00;
              buf[i+2] = 0x00;
              buf[i+3] = 0x00;
          }
          for (; i < 32*2*4; i+=4) {
              buf[i+0] = 0x00;
              buf[i+1] = 0xFF;
              buf[i+2] = 0x00;
              buf[i+3] = 0x00;
          }
          for (; i < 32*3*4; i+=4) {
              buf[i+0] = 0x00;
              buf[i+1] = 0x00;
              buf[i+2] = 0xFF;
              buf[i+3] = 0x00;
          }
          for (; i < 32*4*4; i+=4) {
              buf[i+0] = 0xaf;
              buf[i+1] = 0xaf;
              buf[i+2] = 0xaf;
              buf[i+3] = 0x00;
          }
          for (; i < 32*5*4; i+=4) {
              buf[i+0] = 0xFF;
              buf[i+1] = 0xFF;
              buf[i+2] = 0x00;
              buf[i+3] = 0x00;
          }
          for (; i < 32*6*4; i+=4) {
              buf[i+0] = 0xFF;
              buf[i+1] = 0x00;
              buf[i+2] = 0xFF;
              buf[i+3] = 0x00;
          }
          for (; i < 32*7*4; i+=4) {
              buf[i+0] = 0x00;
              buf[i+1] = 0xFF;
              buf[i+2] = 0xFF;
              buf[i+3] = 0x00;
          }
          for (; i < 32*8*4; i+=4) {
              buf[i+0] = 0xFF;
              buf[i+1] = 0xFF;
              buf[i+2] = 0xFF;
              buf[i+3] = 0x00;
          }
    }
    else {
        ret = get_buf_from_file(buf, gb->getPixelFormat(), gb->getWidth(), gb->getHeight(), index);
        if (!ret)
            printf("open file %s \n", "ok");
        else {
            printf ("open file %s \n", "fault");
            return -1;
        }
    }

    ret = gb->unlock();
    if (ret) {
        printf("unlock buffer error : %s\n",strerror(errno));
        return -1;
    } else
        printf("unlock buffer %s \n","ok");

    return 0;
}

/********** output buf data to file **********/
int Out2Bin(sp<GraphicBuffer> gb, int index)
{
    char* dstbuf = NULL;
    int ret = 0;
    if (gb != NULL)
    {
        ret = gb->lock(GRALLOC_USAGE_SW_WRITE_OFTEN, (void**)&dstbuf);
        if (ret)
    		{
      			printf("lock buffer_src error : %s\n",strerror(errno));
      			return ret;
    		}
        output_buf_data_to_file(dstbuf, gb->getPixelFormat(), gb->getWidth(), gb->getHeight(), index);
        ret = gb->unlock();
        if (ret)
    		{
      			printf("unlock buffer_dst error : %s\n",strerror(errno));
      			return ret;
    		}
    }
    return 0;
}

int main()
{
    int ret = 0;
    int srcWidth,srcHeight,srcFormat;
    int dstWidth,dstHeight,dstFormat;
    int lutWidth,lutHeight,lutFormat;

    sp<GraphicBuffer> src_buf;
    sp<GraphicBuffer> dst_buf;
    sp<GraphicBuffer> lut_buf;

#if !IS_IM2D_API
    rga_info_t src;
    rga_info_t dst;
    rga_info_t lut;

    RockchipRga& rkRga(RockchipRga::get());
#else
    rga_buffer_t imsrc;
    rga_buffer_t imdst;
    rga_buffer_t imlut;
#endif

	/********** SrcInfo set **********/
    srcWidth = 1280;
    srcHeight = 720;
    srcFormat = HAL_PIXEL_FORMAT_BPP_8;

    /********** DstInfo set **********/
    dstWidth = 1280;
    dstHeight = 720;
    dstFormat = HAL_PIXEL_FORMAT_RGBA_8888;

    /*
     * The minimum page size set by mmu is 64k,
     * and mmu will report an error if it is
     * smaller than this size. Actually only
     * use 256*1*4 byte.
     */
    /********** LutInfo set **********/
    lutWidth = 256;
    lutHeight = 64;
    lutFormat = HAL_PIXEL_FORMAT_RGBA_8888;

    /*********** Init GraphicBuffer ***********/
    src_buf = GraphicBuffer_Init(srcWidth, srcHeight, srcFormat);
    dst_buf = GraphicBuffer_Init(dstWidth, dstHeight, dstFormat);
    lut_buf = GraphicBuffer_Init(lutWidth, lutHeight, lutFormat);
    if (src_buf == NULL || dst_buf == NULL || lut_buf == NULL)
    {
        printf("GraphicBuff init error!\n");
        return -1;
    }

	/*********** fill or empty GraphicBuffer ***********/
    if(-1 == GraphicBuffer_Fill(src_buf, 0, 0))
    {
        printf("%s, src write Graphicbuffer error!\n", __FUNCTION__);
        return -1;
    }
    if(-1 == GraphicBuffer_Fill(dst_buf, 1, 0))
    {
        printf("%s, dst empty Graphicbuffer error!\n", __FUNCTION__);
        return -1;
    }
    if(-1 == GraphicBuffer_Fill_LUT(lut_buf, 1, 0))
    {
        printf("%s, lut table write Graphicbuffer error!\n", __FUNCTION__);
        return -1;
    }

	/*********** color palette ***********/
#if !IS_IM2D_API
    memset(&src, 0, sizeof(rga_info_t));
    memset(&dst, 0, sizeof(rga_info_t));
    memset(&lut, 0, sizeof(rga_info_t));

    rga_set_rect(&src.rect, 0,0,srcWidth,srcHeight,srcWidth/*stride*/,srcHeight,srcFormat);
    rga_set_rect(&dst.rect, 0, 0,srcWidth,srcHeight,dstWidth/*stride*/,dstHeight,dstFormat);
    rga_set_rect(&lut.rect, 0, 0,lutWidth,lutHeight,lutWidth/*stride*/,lutHeight,lutFormat);

    src.hnd = src_buf->handle;
    dst.hnd = dst_buf->handle;
    lut.hnd = lut_buf->handle;

    ret = rkRga.RkRgaCollorPalette(&src, &dst, &lut);
#else
    imsrc = wrapbuffer_GraphicBuffer(src_buf);
    imdst = wrapbuffer_GraphicBuffer(dst_buf);
    imlut = wrapbuffer_GraphicBuffer(lut_buf);
    if(imsrc.width == 0 || imdst.width == 0) {
        printf("%s, %s\n", __FUNCTION__, imStrError());
        return -1;
    }

    ret = impalette(imsrc, imdst, imlut);
    printf("paletting .... %s\n", imStrError(ret));
#endif

	/*********** Out to binary ***********/
    Out2Bin(lut_buf, 0);
    Out2Bin(dst_buf, 0);
}

