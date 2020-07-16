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

#ifndef ARGS_H
#define ARGS_H

typedef enum _mode_code {
    MODE_QUERYSTRING = 0,
    MODE_COPY,
    MODE_RESIZE,
    MODE_CROP,
    MODE_ROTATE,
    MODE_FLIP,
    MODE_TRANSLATE,
    MODE_BLEND,
    MODE_CVTCOLOR,
    MODE_FILL,
    MODE_NONE,
    MODE_MAX
} MODE_CODE;

typedef enum {
    INFO_VENDOR = 0,
    INFO_VERSION,
    INFO_MAX_INPUT,
    INFO_MAX_OUTPUT,
    INFO_SCALE_LIMIT,
    INFO_INPUT_FORMAT,
    INFO_OUTPUT_FORMAT,
    INFO_ALL,
    INFO_ERR
} QUERYSTRING_INFO;

#define MODE_QUERYSTRING_CHAR     (char) (MODE_QUERYSTRING+'0')
#define MODE_COPY_CHAR            (char) (MODE_COPY       +'0')
#define MODE_RESIZE_CHAR          (char) (MODE_RESIZE     +'0')
#define MODE_CROP_CHAR            (char) (MODE_CROP       +'0')
#define MODE_ROTATE_CHAR          (char) (MODE_ROTATE     +'0')
#define MODE_FLIP_CHAR            (char) (MODE_FLIP       +'0')
#define MODE_TRANSLATE_CHAR       (char) (MODE_TRANSLATE  +'0')
#define MODE_BLEND_CHAR           (char) (MODE_BLEND      +'0')
#define MODE_CVTCOLOR_CHAR        (char) (MODE_CVTCOLOR   +'0')
#define MODE_FILL_CHAR            (char) (MODE_FILL       +'0')
#define MODE_NONE_CHAR            (char) (MODE_NONE       +'0')

#define BLUE_COLOR  0xffff0000
#define GREEN_COLOR 0xff00ff00
#define RED_COLOR   0xff0000ff

MODE_CODE readArguments(int argc, char *argv[], int* parm);
QUERYSTRING_INFO readInfo(char* targ);
int readParm(char* targ);

#endif

