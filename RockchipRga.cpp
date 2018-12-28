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
#include <signal.h>
#include <time.h>

#include "RockchipRga.h"
//#include "RgaApi.h"
#include "version.h"
#include "normal/NormalRga.h"

#include <drm.h>
#include "drm_mode.h"
#include "xf86drm.h"


// ---------------------------------------------------------------------------

RockchipRga::RockchipRga():
    mSupportRga(false),
    mLogOnce(0),
    mLogAlways(0),
    mContext(NULL)
{
    RkRgaInit();
    printf("Rga built version:%s \n", RK_GRAPHICS_VER);
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

    return ret;
}

int RockchipRga::RkRgaAllocBuffer(int drm_fd, bo_t *bo_info, int width,
                                  int height, int bpp) {
    struct drm_mode_create_dumb arg;
    int ret;

    memset(&arg, 0, sizeof(arg));
    arg.bpp = bpp;
    arg.width = width;
    arg.height = height;

    ret = drmIoctl(drm_fd, DRM_IOCTL_MODE_CREATE_DUMB, &arg);
    if (ret) {
        fprintf(stderr, "failed to create dumb buffer: %s\n", strerror(errno));
        return ret;
    }

    bo_info->handle = arg.handle;
    bo_info->size = arg.size;
    bo_info->pitch = arg.pitch;

    return 0;
}

int RockchipRga::RkRgaFreeBuffer(int drm_fd, bo_t *bo_info) {
    struct drm_mode_destroy_dumb arg;
    int ret;

    if (bo_info->handle <= 0)
        return -EINVAL;
    memset(&arg, 0, sizeof(arg));
    arg.handle = bo_info->handle;
    ret = drmIoctl(drm_fd, DRM_IOCTL_MODE_DESTROY_DUMB, &arg);
    if (ret){
        fprintf(stderr, "failed to destroy dumb buffer: %s\n", strerror(errno));
        return -errno;
    }
    bo_info->handle = 0;

    return 0;
}

int RockchipRga::RkRgaGetAllocBuffer(bo_t *bo_info, int width, int height, int bpp)
{
    static const char* card = "/dev/dri/card0";
    int ret;
    int drm_fd;
    int flag = O_RDWR;
#ifdef O_CLOEXEC
    flag |= O_CLOEXEC;
#endif
    bo_info->fd = -1;
    bo_info->handle = 0;
    drm_fd = open(card, flag);
    if (drm_fd < 0) {
        fprintf(stderr, "Fail to open %s: %m\n", card);
        return -errno;
    }
    ret = RkRgaAllocBuffer(drm_fd, bo_info, width, height, bpp);
    if (ret) {
        close(drm_fd);
        return ret;
    }
    bo_info->fd = drm_fd;
    return 0;
}

int RockchipRga::RkRgaGetMmap(bo_t *bo_info)
{
    struct drm_mode_map_dumb arg;
    void *map;
    int ret;

    memset(&arg, 0, sizeof(arg));
    arg.handle = bo_info->handle;
    ret = drmIoctl(bo_info->fd, DRM_IOCTL_MODE_MAP_DUMB, &arg);
    if (ret)
        return ret;
    map = mmap64(0, bo_info->size, PROT_READ | PROT_WRITE, MAP_SHARED, bo_info->fd, arg.offset);
    if (map == MAP_FAILED)
       return -EINVAL;
    bo_info->ptr = map;
    return 0;
}

int RockchipRga::RkRgaUnmap(bo_t *bo_info)
{
    munmap(bo_info->ptr, bo_info->size);
    bo_info->ptr = NULL;
    return 0;
}

int RockchipRga::RkRgaFree(bo_t *bo_info)
{
    int ret;
    if (bo_info->fd < 0)
        return -EINVAL;
    ret = RkRgaFreeBuffer(bo_info->fd, bo_info);
    close(bo_info->fd);
    bo_info->fd = -1;
    return ret;
}

int RockchipRga::RkRgaGetBufferFd(bo_t *bo_info, int *fd)
{
    int ret = 0;
    ret = drmPrimeHandleToFD(bo_info->fd, bo_info->handle, 0, fd);
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
        printf("This output the user patamaters when rga call blit fail \n");
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

    printf("fd-vir-phy-hnd-format[%d, %p, %p, %p, %d] \n", rgaInfo->fd,
	rgaInfo->virAddr, rgaInfo->phyAddr, (void*)rgaInfo->hnd, rgaInfo->format);
    printf("rect[%d, %d, %d, %d, %d, %d, %d, %d] \n",
        rgaInfo->rect.xoffset, rgaInfo->rect.yoffset,
        rgaInfo->rect.width,   rgaInfo->rect.height, rgaInfo->rect.wstride,
        rgaInfo->rect.hstride, rgaInfo->rect.format, rgaInfo->rect.size);
    printf("f-blend-size-rotation-col-log-mmu[%d, %x, %d, %d, %d, %d, %d] \n",
        rgaInfo->format, rgaInfo->blend, rgaInfo->bufferSize,
        rgaInfo->rotation, rgaInfo->color, rgaInfo->testLog, rgaInfo->mmuFlag);
    return 0;
}

// ---------------------------------------------------------------------------



