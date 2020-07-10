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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <getopt.h>
#include "args.h"
#include "im2d_api/im2d.h"

void help_function() {
    printf("\n=============================================================================================\n");
    printf( "   usage: rgaImDemo [--help/-h] [--querystring/--querystring=<options>]\n"
            "                    [--copy] [--resize=<up/down>] [--crop] [--rotate=90/180/270]\n"
            "                    [--flip=H/V] [--translate] [--blend] [--cvtcolor]\n"
            "                    [--fill=blue/green/red]\n");
    printf( "\t --help/-h     Call help\n"
            "\t --querystring You can print the version or support information corresponding to the current version of RGA according to the options.\n"
            "\t               If there is no input options, all versions and support information of the current version of RGA will be printed.\n"
            "\t --copy        Copy the image by RGA.The default is 720p to 720p.\n"
            "\t --resize      resize the image by RGA.You can choose to up(720p->1080p) or down(720p->480p).\n"
            "\t --crop        Crop the image by RGA.By default, a picture of 300*300 size is cropped from (100,100).\n"
            "\t --rotate      Rotate the image by RGA.You can choose to rotate 90/180/270 degrees.\n"
            "\t --flip        Flip the image by RGA.You can choice of horizontal flip or vertical flip.\n"
            "\t --translate   Translate the image by RGA.Default translation (300,300).\n"
            "\t --blend       Blend the image by RGA.Default, Porter-Duff 'SRC over DST'.\n"
            "\t --cvtcolor    Modify the image format and color space by RGA.The default is RGBA8888 to NV12.\n"
            "\t --fill        Fill the image by RGA to blue, green, red, when you set the option to the corresponding color.\n");
    printf("=============================================================================================\n\n");
}

MODE_CODE readArguments(int argc, char *argv[], int* parm) {
    int opt = 0;
    int option_index = 0;

    char string[] = "h";
    static struct option mode_options[] = {
        { "querystring", optional_argument, NULL, MODE_QUERYSTRING_CHAR   },
        {        "copy",       no_argument, NULL, MODE_COPY_CHAR          },
        {      "resize", required_argument, NULL, MODE_RESIZE_CHAR        },
        {        "crop",       no_argument, NULL, MODE_CROP_CHAR          },
        {      "rotate", required_argument, NULL, MODE_ROTATE_CHAR        },
        {        "flip", required_argument, NULL, MODE_FLIP_CHAR          },
        {   "translate",       no_argument, NULL, MODE_TRANSLATE_CHAR     },
        {       "blend",       no_argument, NULL, MODE_BLEND_CHAR         },
        {    "cvtcolor",       no_argument, NULL, MODE_CVTCOLOR_CHAR      },
        {        "fill", required_argument, NULL, MODE_FILL_CHAR          },
        {        "help",       no_argument, NULL, 'h'                     },
        {         NULL,                 0, NULL, 0                       },
    };

    while((opt = getopt_long(argc, argv, string, mode_options, &option_index))!= -1) {
        printf("Start selecting mode\n");
        switch (opt) {
            case MODE_QUERYSTRING_CHAR :
                printf("im2d querystring ..\n");

                if (optarg != NULL)
                    parm[MODE_QUERYSTRING] = readInfo(optarg);
                else
                    parm[MODE_QUERYSTRING] = INFO_ALL;

                return MODE_QUERYSTRING;

            case MODE_COPY_CHAR :
                printf("im2d copy ..\n");
                return MODE_COPY;

            case MODE_RESIZE_CHAR :
                printf("im2d resize ..\n");

                if (optarg != NULL)
                    parm[MODE_RESIZE] = readParm(optarg);
                if (parm[MODE_RESIZE] == -1)
                    goto out;

                return MODE_RESIZE;

            case MODE_CROP_CHAR :
                printf("im2d crop ..\n");
                return MODE_CROP;

            case MODE_ROTATE_CHAR :
                printf("im2d rotate ..\n");

                if (optarg != NULL)
                    parm[MODE_ROTATE] = readParm(optarg);
                if (parm[MODE_ROTATE] == -1)
                    goto out;

                return MODE_ROTATE;

            case MODE_FLIP_CHAR :
                printf("im2d flip ..\n");

                if (optarg != NULL)
                    parm[MODE_FLIP] = readParm(optarg);
                if (parm[MODE_FLIP] == -1)
                    goto out;

                return MODE_FLIP;

            case MODE_TRANSLATE_CHAR :
                printf("im2d translate ..\n");
                return MODE_TRANSLATE;

            case MODE_BLEND_CHAR :
                printf("im2d blend ..\n");
                return MODE_BLEND;

            case MODE_CVTCOLOR_CHAR :
                printf("im2d cvtcolor ..\n");
                return MODE_CVTCOLOR;

            case MODE_FILL_CHAR :
                printf("im2d fill ..\n");

                if (optarg != NULL)
                    parm[MODE_FILL] = readParm(optarg);
                if (parm[MODE_FILL] == -1)
                    goto out;

                return MODE_FILL;

            case 'h' :
                help_function();
                return MODE_NONE;

            default :
                printf("%s, no options!\n", __FUNCTION__);
                help_function();
                return MODE_NONE;
        }
    }
out:
    help_function();
    return MODE_NONE;
}

QUERYSTRING_INFO readInfo(char* targ) {
    if (strcmp(targ,"d") == 0 || strcmp(targ,"vendor") == 0 ) {
        printf("im2d querystring .. vendor ...\n");
        return INFO_VENDOR;
    } else if (strcmp(targ,"v") == 0 || strcmp(targ,"version") == 0 ) {
        printf("im2d querystring .. vendor ...\n");
        return INFO_VERSION;
    } else if (strcmp(targ,"i") == 0 || strcmp(targ,"maxinput") == 0 ) {
        printf("im2d querystring .. max input ...\n");
        return INFO_MAX_INPUT;
    } else if (strcmp(targ,"o") == 0 || strcmp(targ,"maxoutput") == 0 ) {
        printf("im2d querystring .. max output ...\n");
        return INFO_MAX_OUTPUT;
    } else if (strcmp(targ,"s") == 0 || strcmp(targ,"scalelimit") == 0 ) {
        printf("im2d querystring .. scale limit ...\n");
        return INFO_SCALE_LIMIT;
    } else if (strcmp(targ,"f") == 0 || strcmp(targ,"inputformat") == 0 ) {
        printf("im2d querystring .. input format ...\n");
        return INFO_INPUT_FORMAT;
    } else if (strcmp(targ,"f") == 0 || strcmp(targ,"outputformat") == 0 ) {
        printf("im2d querystring .. output format ...\n");
        return INFO_OUTPUT_FORMAT;
    } else if (strcmp(targ,"a") == 0 || strcmp(targ,"all") == 0 || strcmp(targ," ") == 0) {
        printf("im2d querystring .. all ...\n");
        return INFO_ALL;
    } else {
        printf("%s, Invalid infomation, print all\n", __FUNCTION__);
        return INFO_ERR;
    }
}

int readParm(char* targ) {
    if (strcmp(targ,"up") == 0 ) {
        printf("up resize ...\n");
        return IM_UP_SCALE;
    } else if (strcmp(targ,"down") == 0) {
        printf("down resize ...\n");
        return IM_DOWN_SCALE;
    } else if (strcmp(targ,"90") == 0) {
        printf("rotation 90 ...\n");
        return IM_HAL_TRANSFORM_ROT_90;
    } else if (strcmp(targ,"180") == 0) {
        printf("nrotation 180 ...\n");
        return IM_HAL_TRANSFORM_ROT_180;
    } else if (strcmp(targ,"-270") == 0) {
        printf("rotation 270 ...\n");
        return IM_HAL_TRANSFORM_ROT_270;
    } else if (strcmp(targ,"H") == 0) {
        printf("flip H ...\n");
        return IM_HAL_TRANSFORM_FLIP_H;
    } else if (strcmp(targ,"V") == 0) {
        printf("flip V ...\n");
        return IM_HAL_TRANSFORM_FLIP_V;
    } else if (strcmp(targ,"blue") == 0) {
        printf("fill blue ...\n");
        return BLUE_COLOR;
    } else if (strcmp(targ,"green") == 0) {
        printf("fill green ...\n");
        return GREEN_COLOR;
    } else if (strcmp(targ,"red") == 0) {
        printf("fill red ...\n");
        return RED_COLOR;
    } else {
        printf("%s, Invalid parameter\n", __FUNCTION__);
        return -1;
    }
}

