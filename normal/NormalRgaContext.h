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

#ifndef _rockchip_normal_rga_context_h_
#define _rockchip_normal_rga_context_h_
#define UNUSED(...) (void)(__VA_ARGS__)

struct rgaContext{
    int rgaFd;
    int mLogAlways;
    int mLogOnce;
    float mVersion;
	int Is_debug;
};
#endif