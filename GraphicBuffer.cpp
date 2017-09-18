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
#include "GraphicBuffer.h"
#include <normal/gralloc_rga.h>
gralloc_module_t const *mAllocMod = NULL;

enum {
	GET_HANDLE_FD           = GRALLOC_MODULE_PERFORM_GET_HADNLE_PRIME_FD,
	GET_HADNLE_ATTRIBUTES   = GRALLOC_MODULE_PERFORM_GET_HADNLE_ATTRIBUTES,
	//GET_INTERNAL_FORMAT     = GRALLOC_MODULE_PERFORM_GET_INTERNAL_FORMAT,
	//GET_HADNLE_USAGE        = GRALLOC_MODULE_PERFORM_GET_USAGE,
};
// ---------------------------------------------------------------------------
int RkInitAllocModle()
{
	const hw_module_t *allocMod = NULL;
	int ret = 0;

	if (mAllocMod)
		return 0;

	ret= hw_get_module(GRALLOC_HARDWARE_MODULE_ID, &allocMod);
	ALOGE_IF(ret, "FATAL:can't find the %s module",GRALLOC_HARDWARE_MODULE_ID);
	if (ret == 0)
		mAllocMod = reinterpret_cast<gralloc_module_t const *>(allocMod);

	return ret;
}

int RkRgaGetHandleFd(buffer_handle_t handle, int *fd)
{
	int op = GET_HANDLE_FD;
	int ret = 0;

	if (!mAllocMod)
		ret = RkInitAllocModle();

	if (ret)
		return ret;

	if (mAllocMod->perform)
		mAllocMod->perform(mAllocMod, op, handle, fd);
	else
		return -ENODEV;

	if (ret)
		ALOGE("GraphicBufferGetHandldFd fail %d for:%s",ret,strerror(ret));
	else if (false) {
		ALOGD("fd = %d",*fd);
		fprintf(stderr,"fd = %d\n", *fd);
	}

	return ret;
}

int RkRgaGetHandleAttributes(buffer_handle_t handle,
		std::vector<int> *attrs)
{
	int op = GET_HADNLE_ATTRIBUTES;
	int ret = 0;

	if (!mAllocMod)
		ret = RkInitAllocModle();

	if (ret)
		return ret;

	if(!mAllocMod->perform)
		return -ENODEV;

#if RK3368_DRM
	int w,h,pixel_stride,format,size;

	op = GRALLOC_MODULE_PERFORM_GET_HADNLE_WIDTH;
	mAllocMod->perform(mAllocMod, op, handle, &w);
	op = GRALLOC_MODULE_PERFORM_GET_HADNLE_HEIGHT;
	mAllocMod->perform(mAllocMod, op, handle, &h);
	op = GRALLOC_MODULE_PERFORM_GET_HADNLE_STRIDE;
	mAllocMod->perform(mAllocMod, op, handle, &pixel_stride);
	op = GRALLOC_MODULE_PERFORM_GET_HADNLE_FORMAT;
	mAllocMod->perform(mAllocMod, op, handle, &format);
	op = GRALLOC_MODULE_PERFORM_GET_HADNLE_SIZE;
	mAllocMod->perform(mAllocMod, op, handle, &size);

	//add to attrs.
	attrs->emplace_back(w);
	attrs->emplace_back(h);
	attrs->emplace_back(pixel_stride);
	attrs->emplace_back(format);
	attrs->emplace_back(size);

#else
	mAllocMod->perform(mAllocMod, op, handle, attrs);

	if (ret)
		ALOGE("GraphicBufferGetHandldAttributes fail %d for:%s",ret,strerror(ret));
	else if (false) {
		ALOGD("%d,%d,%d,%d,%d,%d",attrs->at(0),attrs->at(1),attrs->at(2),
				attrs->at(3),attrs->at(4),attrs->at(5));
		fprintf(stderr,"%d,%d,%d,%d,%d,%d\n",
				attrs->at(0),attrs->at(1),attrs->at(2),
				attrs->at(3),attrs->at(4),attrs->at(5));
	}
#endif

	return ret;
}

int RkRgaGetHandleMapAddress(buffer_handle_t handle,
		void **buf)
{
	int usage = GRALLOC_USAGE_SW_READ_MASK | GRALLOC_USAGE_SW_WRITE_MASK;
	int ret = 0;

	if (!mAllocMod)
		ret = RkInitAllocModle();

	if (ret)
		return ret;

	if (mAllocMod->lock)
		ret = mAllocMod->lock(mAllocMod, handle, usage, 0, 0, 0, 0, buf);
	else
		return -ENODEV;

	if (ret)
		ALOGE("GetHandleMapAddress fail %d for:%s",ret,strerror(ret));

	return ret;
}

int RkRgaGetRgaFormat(int format)
{
	switch (format)
	{
		case HAL_PIXEL_FORMAT_RGB_565:
			return RK_FORMAT_RGB_565;
		case HAL_PIXEL_FORMAT_RGB_888:
			return RK_FORMAT_RGB_888;
		case HAL_PIXEL_FORMAT_RGBA_8888:
			return RK_FORMAT_RGBA_8888;
		case HAL_PIXEL_FORMAT_RGBX_8888:
			return RK_FORMAT_RGBX_8888;
		case HAL_PIXEL_FORMAT_BGRA_8888:
			return RK_FORMAT_BGRA_8888;
		case HAL_PIXEL_FORMAT_YCrCb_420_SP:
			return RK_FORMAT_YCrCb_420_SP;
		case HAL_PIXEL_FORMAT_YCrCb_NV12:
			return RK_FORMAT_YCbCr_420_SP;
		case HAL_PIXEL_FORMAT_YCrCb_NV12_VIDEO:
			return RK_FORMAT_YCbCr_420_SP;
		case HAL_PIXEL_FORMAT_YCrCb_NV12_10:
			return RK_FORMAT_YCbCr_420_SP_10B; //0x20
		default:
			ALOGE("Is unsupport format now,please fix");
			return -1;
	}
}


int RkRgaGetRgaFormatFromAndroid(int format)
{
	switch (format)
	{
		case HAL_PIXEL_FORMAT_RGB_565:
			return RK_FORMAT_RGB_565;
		case HAL_PIXEL_FORMAT_RGB_888:
			return RK_FORMAT_RGB_888;
		case HAL_PIXEL_FORMAT_RGBA_8888:
			return RK_FORMAT_RGBA_8888;
		case HAL_PIXEL_FORMAT_RGBX_8888:
			return RK_FORMAT_RGBX_8888;
		case HAL_PIXEL_FORMAT_BGRA_8888:
			return RK_FORMAT_BGRA_8888;
		case HAL_PIXEL_FORMAT_YCrCb_420_SP:
			return RK_FORMAT_YCrCb_420_SP;
		case HAL_PIXEL_FORMAT_YCrCb_NV12:
			return RK_FORMAT_YCbCr_420_SP;
		case HAL_PIXEL_FORMAT_YCrCb_NV12_VIDEO:
			return RK_FORMAT_YCbCr_420_SP;
		case HAL_PIXEL_FORMAT_YCrCb_NV12_10:
			return RK_FORMAT_YCbCr_420_SP_10B;//0x20
		default:
			ALOGE("Is unsupport format now,please fix");
			return -1;
	}
}
