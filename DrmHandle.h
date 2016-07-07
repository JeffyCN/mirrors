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

/*
@enum the index of the std::vector<int> *attrs A is the meaning of attributes and WIDTH is
    for the width.AWIDTH can be use like that attrs->at(AWIDTH) and return the the type
    value of vector.
*/
enum {
	AWIDTH = 0,
	AHEIGHT,
	ASTRIDE,
	AFORMAT,
	ASIZE,
};

int getHandleAttributes(struct rgaContext * ctx, buffer_handle_t hnd, std::vector<int> *attrs);
int getHandlePrimeFd(struct rgaContext * ctx, buffer_handle_t hnd, int *fd);