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
#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "rockchiprga"
#endif

#include <stdint.h>
#include <sys/types.h>
#include <math.h>
#include <fcntl.h>
#include <utils/misc.h>
#include <signal.h>
#include <time.h>

#include <cutils/properties.h>

#ifndef RK3368_ANDROID_8

#include <binder/IPCThreadState.h>
#include <gui/Surface.h>
#include <gui/SurfaceComposerClient.h>

#endif

#include <utils/Atomic.h>
#include <utils/Errors.h>
#include <utils/Log.h>
#include <utils/Mutex.h>
#include <utils/Singleton.h>

#include <ui/PixelFormat.h>
#include <ui/Rect.h>
#include <ui/Region.h>
#include <ui/DisplayInfo.h>
#include <ui/GraphicBufferMapper.h>

#include <gui/ISurfaceComposer.h>


#include "RockchipRga.h"
#include "RgaApi.h"
#include "version.h"
#include "normal/NormalRga.h"

namespace android {

// ---------------------------------------------------------------------------
ANDROID_SINGLETON_STATIC_INSTANCE(RockchipRga)

Mutex RockchipRga::mMutex;

RockchipRga::RockchipRga():
    mSupportRga(false),
    mLogOnce(0),
    mLogAlways(0),
    mContext(NULL)
{
    RkRgaInit();
    ALOGI("Rga built version:%s", RK_GRAPHICS_VER);
}

RockchipRga::~RockchipRga()
{
    RgaDeInit(mContext);
}

int RockchipRga::RkRgaInit()
{
    int ret = 0;

    ret = RgaInit(&mContext);
    if(ret == 0)
        mSupportRga = true;
    else
        mSupportRga = false;

    return 0;
}

int RockchipRga::RkRgaGetBufferFd(buffer_handle_t handle, int *fd)
{
    int ret = 0;
    ret = RkRgaGetHandleFd(handle, fd);
    return ret;
}

int RockchipRga::RkRgaBlit(rga_info *src, rga_info *dst, rga_info *src1)
{
    int ret = 0;
    ret = RgaBlit(src, dst, src1);
    if (ret) {
        RkRgaLogOutUserPara(src);
        RkRgaLogOutUserPara(dst);
        RkRgaLogOutUserPara(src1);
        ALOGE("This output the user patamaters when rga call blit fail");
    }
    return ret;
}


int RockchipRga::RkRgaCollorFill(rga_info *dst)
{
    int ret = 0;
    ret = RgaCollorFill(dst);
    return ret;
}

int RockchipRga::RkRgaLogOutUserPara(rga_info *rgaInfo)
{
    if (!rgaInfo)
        return -EINVAL;

    ALOGD("fd-vir-phy-hnd-format[%d, %p, %p, %p, %d]", rgaInfo->fd,
	rgaInfo->virAddr, rgaInfo->phyAddr, (void*)rgaInfo->hnd, rgaInfo->format);
    ALOGD("rect[%d, %d, %d, %d, %d, %d, %d, %d]",
        rgaInfo->rect.xoffset, rgaInfo->rect.yoffset,
        rgaInfo->rect.width,   rgaInfo->rect.height, rgaInfo->rect.wstride,
        rgaInfo->rect.hstride, rgaInfo->rect.format, rgaInfo->rect.size);
    ALOGD("f-blend-size-rotation-col-log-mmu[%d, %x, %d, %d, %d, %d, %d]",
        rgaInfo->format, rgaInfo->blend, rgaInfo->bufferSize,
        rgaInfo->rotation, rgaInfo->color, rgaInfo->testLog, rgaInfo->mmuFlag);
    return 0;
}

// ---------------------------------------------------------------------------

}
; // namespace android

