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

#include "im2d.h"

#ifdef ANDROID

#include <ui/GraphicBuffer.h>

using namespace android;

IM_API rga_buffer_t wrapbuffer_GraphicBuffer(sp<GraphicBuffer> buf);

#if USE_AHARDWAREBUFFER
#include <android/hardware_buffer.h>
IM_API rga_buffer_t wrapbuffer_AHardwareBuffer(AHardwareBuffer *buf);
#endif

#endif

