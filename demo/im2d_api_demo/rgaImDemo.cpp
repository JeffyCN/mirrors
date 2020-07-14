/*
 * Copyright (C) 2020 Rockchip Electronics Co.Ltd
 * Authors:
 *  PutinLee <putin.lee@rock-chips.com>
 *  Cerf Yu <cerf.yu@rock-chips.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 */

#include "im2d_api/im2d.hpp"
#include "args.h"
#include <stdint.h>
#include <sys/types.h>
#include <math.h>
#include <fcntl.h>
#include "RockchipFileOps.h"
#include <ui/GraphicBuffer.h>

#define ERROR               -1
#define USE_AHARDWAREBUFFER  0

/********** SrcInfo set **********/
#define SRC_WIDTH  1280
#define SRC_HEIGHT 720
#define SRC_FORMAT HAL_PIXEL_FORMAT_RGBA_8888
/********** DstInfo set **********/
#define DST_WIDTH  1280
#define DST_HEIGHT 720
#define DST_FORMAT HAL_PIXEL_FORMAT_RGBA_8888

enum {
    FILL_BUFF  = 0,
    EMPTY_BUFF = 1
};

sp<GraphicBuffer> GraphicBuffer_Init(int width, int height,int format) {
#ifdef ANDROID_7_DRM
    sp<GraphicBuffer> gb(new GraphicBuffer(width,height,format,
                                           GRALLOC_USAGE_SW_WRITE_OFTEN | GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_HW_FB));
#else
    sp<GraphicBuffer> gb(new GraphicBuffer(width,height,format,
                                           GRALLOC_USAGE_SW_WRITE_OFTEN | GRALLOC_USAGE_SW_READ_OFTEN));
#endif

    if (gb->initCheck()) {
        printf("GraphicBuffer check error : %s\n",strerror(errno));
        return NULL;
    } else
        printf("GraphicBuffer check %s \n","ok");

    return gb;
}

/********** write data to buffer or init buffer**********/
int GraphicBuffer_Fill(sp<GraphicBuffer> gb, int flag, int index) {
    int ret;
    char* buf = NULL;
    ret = gb->lock(GRALLOC_USAGE_SW_WRITE_OFTEN, (void**)&buf);

    if (ret) {
        printf("lock buffer error : %s\n",strerror(errno));
        return ERROR;
    } else
        printf("lock buffer %s \n","ok");

    if(flag)
        memset(buf,0x00,gb->getPixelFormat()*gb->getWidth()*gb->getHeight());
    else {
        ret = get_buf_from_file(buf, gb->getPixelFormat(), gb->getWidth(), gb->getHeight(), index);
        if (!ret)
            printf("open file %s \n", "ok");
        else {
            printf ("open file %s \n", "fault");
            return ERROR;
        }
    }

    ret = gb->unlock();
    if (ret) {
        printf("unlock buffer error : %s\n",strerror(errno));
        return ERROR;
    } else
        printf("unlock buffer %s \n","ok");

    return 0;
}

#if USE_AHARDWAREBUFFER
int AHardwareBuffer_Init(int width, int height, int format, AHardwareBuffer** outBuffer) {
    sp<GraphicBuffer> gbuffer;
    gbuffer = GraphicBuffer_Init(width, height, format);
    if(gbuffer == NULL) {
        return ERROR;
    }

    *outBuffer = gbuffer.get()->toAHardwareBuffer();
    // Ensure the buffer doesn't get destroyed when the sp<> goes away.
    AHardwareBuffer_acquire(*outBuffer);
    printf("AHardwareBuffer init ok!\n");
    return 0;
}

int AHardwareBuffer_Fill(AHardwareBuffer** buffer, int flag, int index) {
    sp<GraphicBuffer> gbuffer;

    gbuffer = GraphicBuffer::fromAHardwareBuffer(*buffer);

    if(ERROR == GraphicBuffer_Fill(gbuffer, flag, index)) {
        printf("%s, write Graphicbuffer error!\n", __FUNCTION__);
        return ERROR;
    }

    *buffer = gbuffer.get()->toAHardwareBuffer();
    // Ensure the buffer doesn't get destroyed when the sp<> goes away.

    AHardwareBuffer_acquire(*buffer);
    printf("AHardwareBuffer %s ok!\n", flag==0?"fill":"empty");
    return 0;
}

#endif

int main(int argc, char*  argv[]) {
    int ret = 0;
    int parm_data[MODE_MAX] = {0};

    int               COLOR;
    IM_USAGE          ROTATE;
    IM_USAGE          FLIP;

    MODE_CODE         MODE;
    QUERYSTRING_INFO  IM_INFO;
    IM_STATUS         STATUS;

    im_rect src_rect;
    im_rect dst_rect;
    rga_buffer_t src;
    rga_buffer_t dst;

#if USE_AHARDWAREBUFFER
    AHardwareBuffer* src_buf = nullptr;
    AHardwareBuffer* dst_buf = nullptr;
#else
    sp<GraphicBuffer> src_buf;
    sp<GraphicBuffer> dst_buf;
#endif

    MODE = readArguments(argc, argv, parm_data);
    if(MODE_NONE == MODE) {
        printf("%s, Unknow RGA mode\n", __FUNCTION__);
        return ERROR;
    }
    /********** Get parameters **********/
    if(MODE != MODE_QUERYSTRING) {
#if USE_AHARDWAREBUFFER
        if(ERROR == AHardwareBuffer_Init(SRC_WIDTH, SRC_HEIGHT, SRC_FORMAT, &src_buf)) {
            printf("AHardwareBuffer init error!\n");
            return ERROR;
        }
        if(ERROR == AHardwareBuffer_Init(DST_WIDTH, DST_HEIGHT, DST_FORMAT, &dst_buf)) {
            printf("AHardwareBuffer init error!\n");
            return ERROR;
        }

        if(ERROR == AHardwareBuffer_Fill(&src_buf, FILL_BUFF, 0)) {
            printf("%s, write AHardwareBuffer error!\n", __FUNCTION__);
            return -1;
        }
        if(MODE == MODE_BLEND || MODE == MODE_FILL) {
            if(ERROR == AHardwareBuffer_Fill(&dst_buf, FILL_BUFF, 1)) {
                printf("%s, write AHardwareBuffer error!\n", __FUNCTION__);
                return ERROR;
            }
        } else {
            if(ERROR == AHardwareBuffer_Fill(&dst_buf, EMPTY_BUFF, 1)) {
                printf("%s, write AHardwareBuffer error!\n", __FUNCTION__);
                return ERROR;
            }
        }

        src = wrapbuffer_AHardwareBuffer(src_buf);
        dst = wrapbuffer_AHardwareBuffer(dst_buf);
        if(src.width == 0 || dst.width == 0) {
            printf("%s, %s", __FUNCTION__, imStrError());
            return ERROR;
        }
#else
        src_buf = GraphicBuffer_Init(SRC_WIDTH, SRC_HEIGHT, SRC_FORMAT);
        dst_buf = GraphicBuffer_Init(DST_WIDTH, DST_HEIGHT, DST_FORMAT);
        if (src_buf == NULL || dst_buf == NULL) {
            printf("GraphicBuff init error!\n");
            return ERROR;
        }

        if(ERROR == GraphicBuffer_Fill(src_buf, FILL_BUFF, 0)) {
            printf("%s, write Graphicbuffer error!\n", __FUNCTION__);
            return -1;
        }
        if(MODE == MODE_BLEND || MODE == MODE_FILL) {
            if(ERROR == GraphicBuffer_Fill(dst_buf, FILL_BUFF, 1)) {
                printf("%s, write Graphicbuffer error!\n", __FUNCTION__);
                return ERROR;
            }
        } else {
            if(ERROR == GraphicBuffer_Fill(dst_buf, EMPTY_BUFF, 1)) {
                printf("%s, write Graphicbuffer error!\n", __FUNCTION__);
                return ERROR;
            }
        }

        src = wrapbuffer_GraphicBuffer(src_buf);
        dst = wrapbuffer_GraphicBuffer(dst_buf);
        if(src.width == 0 || dst.width == 0) {
            printf("%s, %s\n", __FUNCTION__, imStrError());
            return ERROR;
        }
#endif
    }

    /********** Execution function according to mode **********/
    switch(MODE) {
        case MODE_QUERYSTRING :

            IM_INFO = (QUERYSTRING_INFO)parm_data[MODE_QUERYSTRING];
            printf("\n%s\n", querystring(IM_INFO));

            break;

        case MODE_COPY :      //rgaImDemo --copy

            ret = imcheck(src, dst, src_rect, dst_rect);
            if (IM_STATUS_NOERROR != ret) {
                printf("%d, check error! %s", __LINE__, imStrError((IM_STATUS)ret));
                return -1;
            }

            STATUS = imcopy(src, dst);
            printf("copying .... %s\n", imStrError(STATUS));

            break;

        case MODE_RESIZE :    //rgaImDemo --resize=up/down

            switch(parm_data[MODE_RESIZE]) {
                case IM_UP_SCALE :

#if USE_AHARDWAREBUFFER
                    if(ERROR == AHardwareBuffer_Init(1920, 1080, DST_FORMAT, &dst_buf)) {
                        printf("AHardwareBuffer init error!\n");
                        return ERROR;
                    }

                    if(ERROR == AHardwareBuffer_Fill(&dst_buf, EMPTY_BUFF, 0)) {
                        printf("%s, write AHardwareBuffer error!\n", __FUNCTION__);
                        return ERROR;
                    }
                    dst = wrapbuffer_AHardwareBuffer(dst_buf);
                    if(dst.width == 0) {
                        printf("%s, dst: %s\n", __FUNCTION__, imStrError());
                        return ERROR;
                    }
#else
                    dst_buf = GraphicBuffer_Init(1920, 1080, DST_FORMAT);
                    if (dst_buf == NULL) {
                        printf("dst GraphicBuff init error!\n");
                        return ERROR;
                    }
                    if(ERROR == GraphicBuffer_Fill(dst_buf, EMPTY_BUFF, 1)) {
                        printf("%s, write Graphicbuffer error!\n", __FUNCTION__);
                        return ERROR;
                    }
                    dst = wrapbuffer_GraphicBuffer(dst_buf);
                    if(dst.width == 0) {
                        printf("%s, dst: %s\n", __FUNCTION__, imStrError());
                        return ERROR;
                    }
#endif

                    break;
                case IM_DOWN_SCALE :

#if USE_AHARDWAREBUFFER
                    if(ERROR == AHardwareBuffer_Init(720, 480, DST_FORMAT, &dst_buf)) {
                        printf("AHardwareBuffer init error!\n");
                        return ERROR;
                    }

                    if(ERROR == AHardwareBuffer_Fill(&dst_buf, EMPTY_BUFF, 0)) {
                        printf("%s, write AHardwareBuffer error!\n", __FUNCTION__);
                        return ERROR;
                    }

                    dst = wrapbuffer_AHardwareBuffer(dst_buf);
                    if(dst.width == 0) {
                        printf("%s, dst: %s\n", __FUNCTION__, imStrError());
                        return ERROR;
                    }
#else
                    dst_buf = GraphicBuffer_Init(720, 480, DST_FORMAT);
                    if (dst_buf == NULL) {
                        printf("dst GraphicBuff init error!\n");
                        return ERROR;
                    }
                    if(ERROR == GraphicBuffer_Fill(dst_buf, EMPTY_BUFF, 1)) {
                        printf("%s, write Graphicbuffer error!\n", __FUNCTION__);
                        return ERROR;
                    }
                    dst = wrapbuffer_GraphicBuffer(dst_buf);
                    if(dst.width == 0) {
                        printf("%s, dst: %s\n", __FUNCTION__, imStrError());
                        return ERROR;
                    }
#endif
                    break;
            }

            ret = imcheck(src, dst, src_rect, dst_rect);
            if (IM_STATUS_NOERROR != ret) {
                printf("%d, check error! %s", __LINE__, imStrError((IM_STATUS)ret));
                return -1;
            }

            STATUS = imresize(src, dst);
            printf("resizing .... %s\n", imStrError(STATUS));

            break;

        case MODE_CROP :      //rgaImDemo --crop

            src_rect.x      = 100;
            src_rect.y      = 100;
            src_rect.width  = 300;
            src_rect.height = 300;

            ret = imcheck(src, dst, src_rect, dst_rect);
            if (IM_STATUS_NOERROR != ret) {
                printf("%d, check error! %s", __LINE__, imStrError((IM_STATUS)ret));
                return -1;
            }

            STATUS = imcrop(src, dst, src_rect);
            printf("cropping .... %s\n", imStrError(STATUS));

            break;

        case MODE_ROTATE :    //rgaImDemo --rotate=90/180/270

            ROTATE = (IM_USAGE)parm_data[MODE_ROTATE];

            ret = imcheck(src, dst, src_rect, dst_rect);
            if (IM_STATUS_NOERROR != ret) {
                printf("%d, check error! %s", __LINE__, imStrError((IM_STATUS)ret));
                return -1;
            }

            STATUS = imrotate(src, dst, ROTATE);
            printf("rotating .... %s\n", imStrError(STATUS));

            break;

        case MODE_FLIP :      //rgaImDemo --flip=H/V

            FLIP = (IM_USAGE)parm_data[MODE_FLIP];

            ret = imcheck(src, dst, src_rect, dst_rect);
            if (IM_STATUS_NOERROR != ret) {
                printf("%d, check error! %s", __LINE__, imStrError((IM_STATUS)ret));
                return -1;
            }

            STATUS = imflip(src, dst, FLIP);
            printf("flipping .... %s\n", imStrError(STATUS));

            break;

        case MODE_TRANSLATE : //rgaImDemo --translate

            src_rect.x = 300;
            src_rect.y = 300;

            ret = imcheck(src, dst, src_rect, dst_rect);
            if (IM_STATUS_NOERROR != ret) {
                printf("%d, check error! %s", __LINE__, imStrError((IM_STATUS)ret));
                return -1;
            }

            STATUS = imtranslate(src, dst, src_rect.x, src_rect.y);
            printf("translating .... %s\n", imStrError(STATUS));

            break;

        case MODE_BLEND :     //rgaImDemo --blend

            ret = imcheck(src, dst, src_rect, dst_rect);
            if (IM_STATUS_NOERROR != ret) {
                printf("%d, check error! %s", __LINE__, imStrError((IM_STATUS)ret));
                return -1;
            }

            STATUS = imblend(src, src, dst);
            printf("blending .... %s\n", imStrError(STATUS));

            break;

        case MODE_CVTCOLOR :  //rgaImDemo --cvtcolor

            src.format = HAL_PIXEL_FORMAT_RGBA_8888;
            dst.format = HAL_PIXEL_FORMAT_YCrCb_NV12;

            ret = imcheck(src, dst, src_rect, dst_rect);
            if (IM_STATUS_NOERROR != ret) {
                printf("%d, check error! %s", __LINE__, imStrError((IM_STATUS)ret));
                return -1;
            }

            STATUS = imcvtcolor(src, dst, src.format, dst.format);
            printf("cvtcolor .... %s\n", imStrError(STATUS));

            break;

        case MODE_FILL :      //rgaImDemo --fill=blue/green/red

            COLOR = parm_data[MODE_FILL];

            dst_rect.x      = 100;
            dst_rect.y      = 100;
            dst_rect.width  = 300;
            dst_rect.height = 300;

            ret = imcheck(src, dst, src_rect, dst_rect);
            if (IM_STATUS_NOERROR != ret) {
                printf("%d, check error! %s", __LINE__, imStrError((IM_STATUS)ret));
                return -1;
            }

            STATUS = imfill(dst, dst_rect, COLOR);
            printf("filling .... %s\n", imStrError(STATUS));

            break;

        case MODE_NONE :

            printf("%s, Unknown mode\n", __FUNCTION__);

            break;

        default :

            printf("%s, Invalid mode\n", __FUNCTION__);

            break;
    }

    /********** output buf data to file **********/
    char* outbuf = NULL;
#if USE_AHARDWAREBUFFER
    sp<GraphicBuffer> gbuffer = GraphicBuffer::fromAHardwareBuffer(dst_buf);
    if (gbuffer != NULL) {
        ret = gbuffer->lock(GRALLOC_USAGE_SW_WRITE_OFTEN, (void**)&outbuf);
        output_buf_data_to_file(outbuf, dst.format, dst.width, dst.height, 0);
        ret = gbuffer->unlock();
    }
#else
    if (dst_buf != NULL) {
        ret = dst_buf->lock(GRALLOC_USAGE_SW_WRITE_OFTEN, (void**)&outbuf);
        output_buf_data_to_file(outbuf, dst.format, dst.width, dst.height, 0);
        ret = dst_buf->unlock();
    }
#endif

    return 0;
}

