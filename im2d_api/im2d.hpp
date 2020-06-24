/*
 * Copyright (C) 2016 Rockchip Electronics Co.Ltd
 * Authors:
 *	PutinLee <putin.lee@rock-chips.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 */

#include "im2d.h"
#include <ui/GraphicBuffer.h>

using namespace android;

#if 1 //Android
IM_API rga_buffer_t wrapbuffer_GraphicBuffer(sp<GraphicBuffer> buf);
IM_API rga_buffer_t wrapbuffer_AHardwareBuffer(AHardwareBuffer *buf);
#endif

