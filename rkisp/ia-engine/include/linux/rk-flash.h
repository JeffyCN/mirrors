/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (c) 2018 Fuzhou Rockchip Electronics Co., Ltd. */
#ifndef RK_FLASH_H
#define RK_FLASH_H

#define RK_VIDIOC_FLASH_TIMEINFO \
	_IOR('V', BASE_VIDIOC_PRIVATE + 0, struct timeval)

#ifdef CONFIG_COMPAT
#define RK_VIDIOC_COMPAT_FLASH_TIMEINFO \
	_IOR('V', BASE_VIDIOC_PRIVATE + 0, struct compat_timeval)
#endif

#endif
