/*
 * Copyright (C) 2016 Rockchip Electronics Co.Ltd
 * Authors:
 *  Zhiqin Wei <wzq@rock-chips.com>
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
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <sys/mman.h>
#include <linux/stddef.h>

#include "drmrga.h"
#include "GrallocOps.h"

//////////////////////////////////////////////////////////////////////////////////
#ifndef ANDROID
#include "RgaSingleton.h"
#endif

#ifdef ANDROID
#include <utils/Singleton.h>
#include <utils/Thread.h>
#include <hardware/hardware.h>
#include <hardware/rga.h>

namespace android {
#endif

    class RockchipRga :public Singleton<RockchipRga> {
      public:

        static inline RockchipRga& get() {
            return getInstance();
        }

        int         RkRgaInit();
#ifdef LINUX
        void        RkRgaDeInit();
        int         RkRgaAllocBuffer(int drm_fd /* input */, bo_t *bo_info,
                                     int width, int height, int bpp, int flags);
        int         RkRgaFreeBuffer(int drm_fd /* input */, bo_t *bo_info);
        int         RkRgaGetAllocBuffer(bo_t *bo_info, int width, int height, int bpp);
		int         RkRgaGetAllocBufferExt(bo_t *bo_info, int width, int height, int bpp, int flags);
		int         RkRgaGetAllocBufferCache(bo_t *bo_info, int width, int height, int bpp);
        int         RkRgaGetMmap(bo_t *bo_info);
        int         RkRgaUnmap(bo_t *bo_info);
        int         RkRgaFree(bo_t *bo_info);
        int         RkRgaGetBufferFd(bo_t *bo_info, int *fd);
#elif ANDROID
        int         RkRgaGetBufferFd(buffer_handle_t handle, int *fd);
#endif
        int         RkRgaBlit(rga_info *src, rga_info *dst, rga_info *src1);
        int         RkRgaSrcOver(rga_info *src, rga_info *dst, rga_info *src1);
        int         RkRgaCollorFill(rga_info *dst);
        int         RkRgaCollorPalette(rga_info *src, rga_info *dst, rga_info *lut);
        int         RkRgaFlush();


        void        RkRgaSetLogOnceFlag(int log) {
            mLogOnce = log;
        }
        void        RkRgaSetAlwaysLogFlag(bool log) {
            mLogAlways = log;
        }
        void        RkRgaLogOutRgaReq(struct rga_req rgaReg);
        int         RkRgaLogOutUserPara(rga_info *rgaInfo);
        inline bool RkRgaIsReady() {
            return mSupportRga;
        }

      private:
        bool                            mSupportRga;
        int                             mLogOnce;
        int                             mLogAlways;
        void *                          mContext;

        friend class Singleton<RockchipRga>;
        RockchipRga();
        ~RockchipRga();
    };

#ifdef ANDROID
}; // namespace android
#endif

#endif

