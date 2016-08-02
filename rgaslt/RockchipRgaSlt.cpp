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
#define LOG_TAG "RockchipRgaTest"

#include <stdint.h>
#include <sys/types.h>
#include <math.h>
#include <fcntl.h>
#include <utils/misc.h>
#include <signal.h>
#include <time.h>

#include <cutils/properties.h>

#include <androidfw/AssetManager.h>
#include <binder/IPCThreadState.h>
#include <utils/Atomic.h>
#include <utils/Errors.h>
#include <utils/Log.h>

#include <ui/PixelFormat.h>
#include <ui/Rect.h>
#include <ui/Region.h>
#include <ui/DisplayInfo.h>
#include <ui/GraphicBufferMapper.h>

#include <gui/ISurfaceComposer.h>
#include <gui/Surface.h>
#include <gui/SurfaceComposerClient.h>

#include <GLES/gl.h>
#include <GLES/glext.h>
#include <EGL/eglext.h>

#include <vector>

#include "RockchipRgaSlt.h"

struct _rga_context mCtx;

int main()
{
    int ret = 0;
    memset(&mCtx, 0, sizeof(struct _rga_context));

    init_crc_table();

    mCtx.crcStd = 0x37812b01;

    readyToRun();

    rgaNv12ScaleTest(false);

    runToExit();
    return 0;
}

static void init_crc_table(void)
{
	unsigned int c;
	unsigned int i, j;

	for (i = 0; i < 256; i ++) {
		c = (unsigned int)i;
		for (j = 0; j < 8; j++) {
			if (c & 1)
				c = 0xedb88320L ^ (c >> 1);
			else
			    c = c >> 1;
		}
		mCtx.crcTable[i] = c;
	}
}

static unsigned int crc32(unsigned int crc,unsigned char *buffer, unsigned int size)
{
	unsigned int i;
	for (i = 0; i < size; i++) {
		crc = mCtx.crcTable[(crc ^ buffer[i]) & 0xff] ^ (crc >> 8);
	}
	return crc;
}

int readyToRun()
{
    hw_module_t const* module;
    int ret = -1;

    int index = 0;
    drm_rga_t rects;
    int data[4] = {0x5000000,0x00ff0000,0x0000ff00,0x000000ff};

    /*
    *init rga moudle
    */
    ret = hw_get_module(DRMRGA_HARDWARE_MODULE_ID, &mCtx.mRgaMod);
    if (ret) {
        printf("%s,%d faile get hw moudle\n",__func__,__LINE__);
        return -1;
    }

    ret = rga_open(mCtx.mRgaMod, &mCtx.mRga);
    if (ret) {
        printf("device failed to initialize (%s)\n", strerror(-ret));
        return -1;
    }
    printf("init rga ok copy=%p,FillColor=%p\n",mCtx.mRga->rgaCopy,mCtx.mRga->rgaFillColor);

    /*
    *init gralloc moudle
    */
    ret = hw_get_module(GRALLOC_HARDWARE_MODULE_ID, &module);
    if (ret)
        printf("FATAL: can't find the %s module\n", GRALLOC_HARDWARE_MODULE_ID);
    else {
        gralloc_open(module, &mCtx.mAllocDev);
        mCtx.mAllocMod = reinterpret_cast<gralloc_module_t const *>(module);
    }

    return ret;
}

int runToExit()
{
    int ret = 0;
    rga_close(mCtx.mRga);
    gralloc_close(mCtx.mAllocDev);
    return ret;
}

static int gralloc_alloc_buffer(buffer_handle_t *hnd, int w,int h,int *s,int fmt,int usage)
{
    int stride_gr = 0;

    int err = mCtx.mAllocDev->alloc(mCtx.mAllocDev, w, h, fmt, usage, hnd,
                                                                       &stride_gr);
    if (!err)
        printf("gralloc_alloc_buffer handle=%p\n", hnd);
    else
        printf("gralloc_alloc_buffer alloc faild %s\n", strerror(-err));

    return err;
}

static int gralloc_free_buffer(buffer_handle_t hnd)
{
	if (hnd) {
        int err = mCtx.mAllocDev->free(mCtx.mAllocDev, hnd);
        if (err)
            printf("free mDimHandle failed %d (%s)", err, strerror(-err));
	}

    return 0;
}

int get_src_data_from_file(char *buf)
{
#if 1
    const char *yuvFilePath = "/data/fb1280x720.yuv";
    FILE *file = fopen(yuvFilePath, "rb");
    if (!file) {
        fprintf(stderr, "Could not open %s\n", yuvFilePath);
        return false;
    }
    fread(buf, 2 * 1280 * 720, 1, file);
    fclose(file);
#else
    memset(buf,0x55,4*1200*1920);
#endif
    return 0;
}

int get_buffer_size(buffer_handle_t hnd)
{
    int ret = 0;
    int op = 0x81000004;
    std::vector<int> attrs;

#ifdef GRALLOC_MODULE_PERFORM_GET_HADNLE_ATTRIBUTES
    op = GRALLOC_MODULE_PERFORM_GET_HADNLE_ATTRIBUTES
#endif

    if(mCtx.mAllocMod && mCtx.mAllocMod->perform)
        ret = mCtx.mAllocMod->perform(mCtx.mAllocMod, op, hnd, &attrs);
    else
        ret = -EINVAL;

    if(!ret) {
        ALOGD("%d,%d,%d,%d,%d",attrs.at(0),attrs.at(1),attrs.at(2),attrs.at(3),attrs.at(4));
        fprintf(stderr,"%d,%d,%d,%d,%d\n",
                               attrs.at(0),attrs.at(1),attrs.at(2),attrs.at(3),attrs.at(4));
        ret = attrs.at(4);
    } else {
        ALOGE("getHandleAttributes fail %d for:%s",ret,strerror(ret));
        ret = -1;
    }

    return ret;
}

int crc_verify(char* buf, int size)
{
    int ret = 255;
    unsigned int crcdata = 0;
#if 0
    const char *yuvFilePath = "/data/fb1280x720.yuv";
    FILE *file = fopen(yuvFilePath, "rb");
    if (!file) {
        fprintf(stderr, "Could not open %s\n", yuvFilePath);
        return false;
    }
    fread(buf, 2 * 1280 * 720, 1, file);
    fclose(file);
#elif 1
    const char *outFilePath = "/data/outBuffer.bin";
    FILE *file = fopen(outFilePath, "wb+");
    if (!file) {
        fprintf(stderr, "Could not open %s\n", outFilePath);
        return false;
    }
    if (buf)
        fwrite(buf, 1.5 * 1920 * 1088, 1, file);
    fclose(file);
#else

#endif
    if (!buf)
        return ret;

    crcdata = crc32(0xffffffff, (unsigned char*)buf, size);
    if (mCtx.crcStd != crcdata) {
        ret = 255;
        printf("Error now crc=0x%x,stdcrc=0x%x\n", crcdata, mCtx.crcStd);
    } else {
        printf("right now crc=0x%x,stdcrc=0x%x\n", crcdata, mCtx.crcStd);
        ret = 0;
    }

    return ret;
}

int rgaNv12ScaleTest(bool prepare)
{
    buffer_handle_t srcHnd;
    buffer_handle_t dstHnd;

    drm_rga_t rects;

    char *buf;

    int w,h,l,t,r,b,s;
    int srcUsage,dstUsage;
    int index = 0;
    int ret = 0;
    int srcFmt,dstFmt;
    int size;

    w = 1280;
    h = 720;
    l = 0;
    t = 0;
    r = w;
    b = h;

    srcUsage = GRALLOC_USAGE_SW_WRITE_OFTEN;
    srcFmt = HAL_PIXEL_FORMAT_YCrCb_NV12;

    ret = gralloc_alloc_buffer(&srcHnd, w, h, &s, srcFmt, srcUsage);
    if (ret)
        goto srcErr;

    dstUsage = GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_HW_RENDER;
    dstFmt = HAL_PIXEL_FORMAT_RGBX_8888;

    ret = gralloc_alloc_buffer(&dstHnd, w, h, &s, dstFmt, dstUsage);
    if (ret)
        goto dstErr;

    rga_set_rect(&rects.src,0,0,1280,720,1280,srcFmt);
    //rga_set_rect(&rects.dst,0,0,1280,720,1280,HAL_PIXEL_FORMAT_YCrCb_NV12);
    rga_set_rect(&rects.dst,0,0,1280,720,1280,dstFmt);

    printf("GraphicBuffer ok : %s\n","*************************************");

    ret = mCtx.mAllocMod->lock(mCtx.mAllocMod, srcHnd, static_cast<int>(srcUsage),
                                                      l, t, r, b, (void **)&buf);

    if (ret)
        printf("lock(...) failed %d (%s)\n", ret, strerror(-ret));

    if (ret) {
        printf("lock(...) failed %d (%s)\n", ret, strerror(-ret));
        goto lockErr;
    } else
        printf("lock buffer ok %p: %s\n",buf,"**************************************");

    get_src_data_from_file(buf);

    ret = mCtx.mAllocMod->unlock(mCtx.mAllocMod, srcHnd);

    if (ret) {
        printf("unlock buffer error : %s\n",strerror(errno));
        goto unLockErr;
    } else
        printf("unlock buffer ok : %s\n","*************************************");

    while(1) {
        ret = mCtx.mRga->rgaScale(mCtx.mRga,srcHnd,dstHnd,&rects);

        if (ret) {
            printf("rgaScale error:%s,hnd=%p\n", strerror(errno),(void*)(dstHnd));
        }

        buf = NULL;

        ret = mCtx.mAllocMod->lock(mCtx.mAllocMod, dstHnd, static_cast<int>(dstUsage),
                                                        l, t, r, b, (void **)&buf);
        if (ret) {
            printf("lock error:%s,hnd=%p\n", strerror(errno),(void*)(dstHnd));
            goto lockErr;
        } else
            printf("lock buf = %p\n", buf);

        size = get_buffer_size(dstHnd);

        ret = crc_verify(buf, size);

        ret = ret | mCtx.mAllocMod->unlock(mCtx.mAllocMod, dstHnd);
	while (1) {
        printf("rga test exit\n");
	usleep(50000);
	}
        break;
    }
unLockErr:
lockErr:
    gralloc_free_buffer(dstHnd);
dstErr:
    gralloc_free_buffer(srcHnd);
srcErr:
    return ret;
}
