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


#include <android/native_window.h>

#include <utils/Log.h>

#include <vector>

#include "DrmRgaApi.h"
#include "DrmRgaContext.h"

/*get it from drm_gralloc*/
enum {
	GRALLOC_MODULE_PERFORM_GET_DRM_FD                = 0x80000002,
	GRALLOC_MODULE_PERFORM_GET_HADNLE_PRIME_FD       = 0x81000002,
	GRALLOC_MODULE_PERFORM_GET_HADNLE_ATTRIBUTES     = 0x81000004,
};

/*
@func getHandleAttributes:get attributes from handle.Before call this api,As far as now,
    we need register the buffer first.May be the register is good for processer I think

@param hnd:
@param attrs: if size of attrs is small than 5,it will return EINVAL else
    width  = attrs[0]
    height = attrs[1]
    stride = attrs[2]
    format = attrs[3]
    size   = attrs[4]
*/
int getHandleAttributes(struct rgaContext * ctx, buffer_handle_t hnd, std::vector<int> *attrs)
{
    int ret = 0;
    int op = GRALLOC_MODULE_PERFORM_GET_HADNLE_ATTRIBUTES;

    if (!ctx)
        return -EINVAL;

    if(ctx->mAllocMod && ctx->mAllocMod->perform)
        ret = ctx->mAllocMod->perform(ctx->mAllocMod,op, hnd, attrs);
    else
        ret = -EINVAL;

    if(!ret) {
        ALOGD("%d,%d,%d,%d,%d",attrs->at(0),attrs->at(1),attrs->at(2),attrs->at(3),attrs->at(4));
        fprintf(stderr,"%d,%d,%d,%d,%d\n",
                               attrs->at(0),attrs->at(1),attrs->at(2),attrs->at(3),attrs->at(4));
    } else
        ALOGE("getHandleAttributes fail %d for:%s",ret,strerror(ret));
    return ret;
}

/*
@func getHandlePrimeFd:get prime_fd  from handle.Before call this api,As far as now, we
    need register the buffer first.May be the register is good for processer I think

@param hnd:
@param fd: prime_fd. and driver can call the dma_buf_get to get the buffer

*/
int getHandlePrimeFd(struct rgaContext * ctx, buffer_handle_t hnd, int *fd)
{
    int ret = 0;
    int op = GRALLOC_MODULE_PERFORM_GET_HADNLE_PRIME_FD;

    if (!ctx)
        return -EINVAL;

    if(ctx->mAllocMod && ctx->mAllocMod->perform)
        ret = ctx->mAllocMod->perform(ctx->mAllocMod, op, hnd, fd);
    else
        ret = -EINVAL;

    return ret;
}
