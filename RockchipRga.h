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

#ifndef _rockchip_rga_h_
#define _rockchip_rga_h_

#include <stdint.h>
#include <vector>
#include <sys/types.h>

//#include <system/window.h>

#include <utils/Thread.h>

#ifndef UN_NEED_GL
#include <EGL/egl.h>
#include <GLES/gl.h>
#endif

//////////////////////////////////////////////////////////////////////////////////
#include <hardware/hardware.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <utils/Singleton.h>

#include <sys/mman.h>
#include <linux/stddef.h>

#include <hardware/rga.h>
#include "stdio.h"

#include "drmrga.h"
#ifndef UN_NEED_GL
#include "GraphicBuffer.h"
#endif
//////////////////////////////////////////////////////////////////////////////////

namespace android {
// -------------------------------------------------------------------------------

class RockchipRga :public Singleton<RockchipRga>
{
/************************************public**************************************/

public:

    static inline RockchipRga& get() {return getInstance();}

    int         RkRgaInit();
    int         RkRgaGetBufferFd(buffer_handle_t handle, int *fd);
    int         RkRgaBlit(rga_info *src, rga_info *dst, rga_info *src1);
    int         RkRgaCollorFill(rga_info *dst);
    int         RkRgaFlush();


    void        RkRgaSetLogOnceFlag(int log) {mLogOnce = log;}
    void        RkRgaSetAlwaysLogFlag(bool log) {mLogAlways = log;}
    void        RkRgaLogOutRgaReq(struct rga_req rgaReg);
    int         RkRgaLogOutUserPara(rga_info *rgaInfo);
    inline bool RkRgaIsReady() { return mSupportRga; }

/************************************private***********************************/
private:
    bool                            mSupportRga;
    int                             mLogOnce;
    int                             mLogAlways;
    void *                          mContext;
    static Mutex                    mMutex;

    friend class Singleton<RockchipRga>;
                RockchipRga();
                 ~RockchipRga();



};

// ---------------------------------------------------------------------------

}; // namespace android

#endif // ANDROID_BOOTANIMATION_H

