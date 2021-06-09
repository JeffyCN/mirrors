/*
 * Copyright (C) 2016 Rockchip Electronics Co., Ltd.
 * Authors:
 *  Zhiqin Wei <wzq@rock-chips.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <linux/stddef.h>

#include "RgaUtils.h"
#include "RockchipRga.h"
#include "core/NormalRga.h"

struct format_table_entry {
    int format;
    const char *name;
};

struct format_table_entry format_table[] = {
    { RK_FORMAT_RGBA_8888,          "rgba8888" },
    { RK_FORMAT_RGBX_8888,          "rgbx8888" },
    { RK_FORMAT_RGB_888,            "rgb888" },
    { RK_FORMAT_BGRA_8888,          "bgra8888" },
    { RK_FORMAT_RGB_565,            "rgb565" },
    { RK_FORMAT_RGBA_5551,          "rgba5551" },
    { RK_FORMAT_RGBA_4444,          "rgba4444" },
    { RK_FORMAT_BGR_888,            "bgr888" },
    { RK_FORMAT_YCbCr_422_SP,       "cbcr422sp" },
    { RK_FORMAT_YCbCr_422_P,        "cbcr422p" },
    { RK_FORMAT_YCbCr_420_SP,       "nv12" },
    { RK_FORMAT_YCbCr_420_P,        "crcb420p" },
    { RK_FORMAT_YCrCb_422_SP,       "crcb422sp" },
    { RK_FORMAT_YCrCb_422_P,        "crcb422p" },
    { RK_FORMAT_YCrCb_420_SP,       "crcb420sp" },

    { RK_FORMAT_YCrCb_420_P,        "crcb422p" },
    { RK_FORMAT_BPP1,               "bpp1" },
    { RK_FORMAT_BPP2,               "bpp2" },
    { RK_FORMAT_BPP4,               "bpp4" },
    { RK_FORMAT_BPP8,               "bpp8" },
    { RK_FORMAT_Y4,                 "y4" },

    { RK_FORMAT_YCbCr_400,          "cbcr400" },
    { RK_FORMAT_BGRX_8888,          "bgrx8888" },
    { RK_FORMAT_YVYU_422,           "yvyu422" },
    { RK_FORMAT_YVYU_420,           "yvyuv420" },
    { RK_FORMAT_VYUY_422,           "vyuy422" },
    { RK_FORMAT_VYUY_420,           "vyuy420" },

    { RK_FORMAT_YUYV_422,           "yuyv422" },
    { RK_FORMAT_YUYV_420,           "yuyv420" },
    { RK_FORMAT_UYVY_422,           "uyvy422" },
    { RK_FORMAT_UYVY_420,           "uyvy420" },
    { RK_FORMAT_YCbCr_420_SP_10B,   "nv12_10" },
    { RK_FORMAT_YCrCb_420_SP_10B,   "crcb420sp_10" },
    { RK_FORMAT_YCbCr_422_10b_SP,   "cbcr422_10b" },
    { RK_FORMAT_YCrCb_422_10b_SP,   "crcb422_10b" },
    { RK_FORMAT_UNKNOWN,            "unknown" }
};

float get_bpp_from_format(int format) {
    float bpp = 0;

#ifdef LINUX
    if (!(format & 0xFF00 || format == 0)) {
        format = RkRgaCompatibleFormat(format);
    }
#endif

    switch (format) {
#ifdef ANDROID
        case HAL_PIXEL_FORMAT_RGB_565:
            bpp = 2;
            break;
        case HAL_PIXEL_FORMAT_RGB_888:
            bpp = 3;
            break;
        case HAL_PIXEL_FORMAT_RGBA_8888:
        case HAL_PIXEL_FORMAT_RGBX_8888:
        case HAL_PIXEL_FORMAT_BGRA_8888:
            bpp = 4;
            break;
        case HAL_PIXEL_FORMAT_YCrCb_420_SP:
        case HAL_PIXEL_FORMAT_YCrCb_NV12:
        case HAL_PIXEL_FORMAT_YCrCb_NV12_VIDEO:
            bpp = 1.5;
            break;
        case HAL_PIXEL_FORMAT_YCrCb_NV12_10:
            /*RK encoder requires alignment of odd multiples of 256.*/
            /*Here bpp=2 guarantee to read complete data.*/
            bpp = 2;
            break;
#endif
        case RK_FORMAT_Y4:
            bpp = 0.5;
            break;
        case RK_FORMAT_BPP1:
        case RK_FORMAT_BPP2:
        case RK_FORMAT_BPP4:
        case RK_FORMAT_BPP8:
        case RK_FORMAT_YCbCr_400:
            bpp = 1;
            break;
        case RK_FORMAT_YCbCr_420_SP:
        case RK_FORMAT_YCbCr_420_P:
        case RK_FORMAT_YCrCb_420_P:
        case RK_FORMAT_YCrCb_420_SP:
        /* yuyv */
        case RK_FORMAT_YVYU_420:
        case RK_FORMAT_VYUY_420:
        case RK_FORMAT_YUYV_420:
        case RK_FORMAT_UYVY_420:
            bpp = 1.5;
            break;
        case RK_FORMAT_RGB_565:
        case RK_FORMAT_RGBA_5551:
        case RK_FORMAT_RGBA_4444:
        case RK_FORMAT_YCbCr_422_SP:
        case RK_FORMAT_YCbCr_422_P:
        case RK_FORMAT_YCrCb_422_SP:
        case RK_FORMAT_YCrCb_422_P:
        /* yuyv */
        case RK_FORMAT_YVYU_422:
        case RK_FORMAT_VYUY_422:
        case RK_FORMAT_YUYV_422:
        case RK_FORMAT_UYVY_422:
            bpp = 2;
            break;
        /*RK encoder requires alignment of odd multiples of 256.*/
        /*Here bpp=2 guarantee to read complete data.*/
        case RK_FORMAT_YCbCr_420_SP_10B:
        case RK_FORMAT_YCrCb_420_SP_10B:
            bpp = 2;
            break;
        case RK_FORMAT_YCbCr_422_10b_SP:
        case RK_FORMAT_YCrCb_422_10b_SP:
            bpp = 2.5;
            break;
        case RK_FORMAT_BGR_888:
        case RK_FORMAT_RGB_888:
            bpp = 3;
            break;
        case RK_FORMAT_RGBA_8888:
        case RK_FORMAT_RGBX_8888:
        case RK_FORMAT_BGRA_8888:
        case RK_FORMAT_BGRX_8888:
            bpp = 4;
            break;
        default:
            printf("Is unsupport format now, please fix \n");
            return 0;
    }

    return bpp;
}

int get_buf_size_by_w_h_f(int w, int h, int f) {
    float bpp = get_bpp_from_format(f);
    int size = 0;

    size = (int)w * h * bpp;
    return size;
}

int get_string_by_format(char *value, int format) {
    if (!value)
        return -EINVAL;

#ifdef LINUX
        if (!(format & 0xFF00 || format == 0)) {
            format = RkRgaCompatibleFormat(format);
        }
#endif

    switch (format) {
#ifdef ANDROID
        case HAL_PIXEL_FORMAT_RGB_565:
            memcpy(value, "rgb565", sizeof("rgb565"));
            break;
        case HAL_PIXEL_FORMAT_RGB_888:
            memcpy(value, "rgb888", sizeof("rgb888"));
            break;
        case HAL_PIXEL_FORMAT_RGBA_8888:
            memcpy(value, "rgba8888", sizeof("rgba8888"));
            break;
        case HAL_PIXEL_FORMAT_RGBX_8888:
            memcpy(value, "rgbx8888", sizeof("rgbx8888"));
            break;
        case HAL_PIXEL_FORMAT_BGRA_8888:
            memcpy(value, "bgra8888", sizeof("bgra8888"));
            break;
        case HAL_PIXEL_FORMAT_YCrCb_420_SP:
            memcpy(value, "crcb420sp", sizeof("crcb420sp"));
            break;
        case HAL_PIXEL_FORMAT_YCrCb_NV12:
            memcpy(value, "nv12", sizeof("nv12"));
            break;
        case HAL_PIXEL_FORMAT_YCrCb_NV12_VIDEO:
            memcpy(value, "nv12", sizeof("nv12"));
            break;
        case HAL_PIXEL_FORMAT_YCrCb_NV12_10:
            memcpy(value, "nv12_10", sizeof("nv12_10"));
            break;
#endif
        case RK_FORMAT_BPP1:
            memcpy(value, "bpp1", sizeof("bpp1"));
            break;
        case RK_FORMAT_BPP2:
            memcpy(value, "bpp2", sizeof("bpp2"));
            break;
        case RK_FORMAT_BPP4:
            memcpy(value, "bpp4", sizeof("bpp4"));
            break;
        case RK_FORMAT_BPP8:
            memcpy(value, "bpp8", sizeof("bpp8"));
            break;
        case RK_FORMAT_RGB_565:
            memcpy(value, "rgb565", sizeof("rgb565"));
            break;
        case RK_FORMAT_RGB_888:
            memcpy(value, "rgb888", sizeof("rgb888"));
            break;
        case RK_FORMAT_RGBA_8888:
            memcpy(value, "rgba8888", sizeof("rgba8888"));
            break;
        case RK_FORMAT_RGBX_8888:
            memcpy(value, "rgbx8888", sizeof("rgbx8888"));
            break;
        case RK_FORMAT_BGRA_8888:
            memcpy(value, "bgra8888", sizeof("bgra8888"));
            break;
        case RK_FORMAT_RGBA_5551:
            memcpy(value, "rgba5551", sizeof("rgba5551"));
            break;
        case RK_FORMAT_RGBA_4444:
            memcpy(value, "rgba4444", sizeof("rgba4444"));
            break;
        case RK_FORMAT_YCrCb_420_SP:
            memcpy(value, "crcb420sp", sizeof("crcb420sp"));
            break;
        case RK_FORMAT_BGR_888:
            memcpy(value, "bgr888", sizeof("bgr888"));
            break;
        case RK_FORMAT_YCrCb_420_P:
            memcpy(value, "crcb420p", sizeof("crcb420p"));
            break;
        case RK_FORMAT_YCbCr_422_SP:
            memcpy(value, "cbcr422sp", sizeof("cbcr422sp"));
            break;
        case RK_FORMAT_YCbCr_422_P:
            memcpy(value, "cbcr422p", sizeof("cbcr422p"));
            break;
        case RK_FORMAT_YCbCr_420_SP:
            memcpy(value, "nv12", sizeof("nv12"));
            break;
        case RK_FORMAT_YCbCr_420_P:
            memcpy(value, "cbcr420p", sizeof("cbcr420p"));
            break;
        case RK_FORMAT_YCrCb_422_SP:
            memcpy(value, "crcb422sp", sizeof("crcb422sp"));
            break;
        case RK_FORMAT_YCrCb_422_P:
            memcpy(value, "crcb422p", sizeof("crcb422p"));
            break;
        case RK_FORMAT_Y4:
            memcpy(value, "y4", sizeof("y4"));
            break;
        case RK_FORMAT_YCbCr_400:
            memcpy(value, "cbcr400", sizeof("cbcr400"));
            break;
        case RK_FORMAT_YVYU_422:
            memcpy(value, "yvyu422", sizeof("yvyu422"));
            break;
        case RK_FORMAT_YVYU_420:
            memcpy(value, "yvyu420", sizeof("yvyu420"));
            break;
        case RK_FORMAT_VYUY_422:
            memcpy(value, "vyuy422", sizeof("vyuy422"));
            break;
        case RK_FORMAT_VYUY_420:
            memcpy(value, "vyuy420", sizeof("vyuy420"));
            break;
        case RK_FORMAT_YUYV_422:
            memcpy(value, "yuyv422", sizeof("yuyv422"));
            break;
        case RK_FORMAT_YUYV_420:
            memcpy(value, "yuyv420", sizeof("yuyv420"));
            break;
        case RK_FORMAT_UYVY_422:
            memcpy(value, "uyvy422", sizeof("uyvy422"));
            break;
        case RK_FORMAT_UYVY_420:
            memcpy(value, "uyvy420", sizeof("uyvy420"));
            break;
        case RK_FORMAT_YCbCr_420_SP_10B:
            memcpy(value, "nv12_10", sizeof("nv12_10"));
            break;
        case RK_FORMAT_YCrCb_420_SP_10B:
            memcpy(value, "crcb420sp_10", sizeof("crcb420sp_10"));
            break;
        case RK_FORMAT_YCbCr_422_10b_SP:
            memcpy(value, "cbcr422_10b", sizeof("cbcr422_10b"));
            break;
        case RK_FORMAT_YCrCb_422_10b_SP:
            memcpy(value, "crcb422_10b", sizeof("crcb422_10b"));
            break;
        default:
            printf("Is unsupport format now, please fix");
            return 0;
    }

    return 0;
}

int get_buf_from_file(void *buf, int f, int sw, int sh, int index) {
#ifdef ANDROID
    const char *inputFilePath = "/data/in%dw%d-h%d-%s.bin";
#endif

#ifdef LINUX
    const char *inputFilePath = "/usr/data/in%dw%d-h%d-%s.bin";
#endif

    char filePath[100];
    char fstring[30];
    int ret = 0;

    ret = get_string_by_format(fstring, f);
    snprintf(filePath, 100, inputFilePath, index, sw, sh, fstring);

    FILE *file = fopen(filePath, "rb");
    if (!file) {
        fprintf(stderr, "Could not open %s\n", filePath);
        return -EINVAL;
    }
    fread(buf, get_buf_size_by_w_h_f(sw, sh, f), 1, file);
    fclose(file);

    return 0;
}

int output_buf_data_to_file(void *buf, int f, int sw, int sh, int index) {
#ifdef ANDROID
    const char *outputFilePath = "/data/out%dw%d-h%d-%s.bin";
#endif

#ifdef LINUX
    const char *outputFilePath = "/usr/data/out%dw%d-h%d-%s.bin";
#endif

    char filePath[100];
    char fstring[30];
    int ret = 0;

    ret = get_string_by_format(fstring, f);
    snprintf(filePath, 100, outputFilePath, index, sw, sh, fstring);

    FILE *file = fopen(filePath, "wb+");
    if (!file) {
        fprintf(stderr, "Could not open %s\n", filePath);
        return false;
    } else
        fprintf(stderr, "open %s and write ok\n", filePath);
    fwrite(buf, get_buf_size_by_w_h_f(sw, sh, f), 1, file);
    fclose(file);

    return 0;
}

const char *translate_format_str(int format) {
    format = RkRgaGetRgaFormat(format);

    for (int i = 0; i < sizeof(format_table) / sizeof(format_table[0]); i++)
        if (format_table[i].format == format)
            return format_table[i].name;

    return "unknow";
}

