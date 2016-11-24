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

#include <binder/IPCThreadState.h>
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
#include <gui/Surface.h>
#include <gui/SurfaceComposerClient.h>

#include "RockchipRga.h"
#include "RgaApi.h"

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
    return ret;
}
// ---------------------------------------------------------------------------

}
; // namespace android

