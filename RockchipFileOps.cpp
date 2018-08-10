#include <stdint.h>
#include <sys/types.h>
#include <math.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>

#include "RockchipRga.h"

#include <stdint.h>
#include <sys/types.h>

///////////////////////////////////////////////////////
//#include "../drmrga.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>

#include <sys/mman.h>
#include <linux/stddef.h>

float get_bpp_from_format(int format)
{
    float bpp = 0;

    switch (format) {
        case RK_FORMAT_RGB_565:
            bpp = 2;
            break;
        case RK_FORMAT_BGR_888:
        case RK_FORMAT_RGB_888:
            bpp = 3;
            break;
        case RK_FORMAT_RGBA_8888:
            bpp = 4;
            break;
        case RK_FORMAT_RGBX_8888:
            bpp = 4;
            break;
        case RK_FORMAT_BGRA_8888:
            bpp = 4;
            break;
        case RK_FORMAT_YCbCr_422_SP:
        case RK_FORMAT_YCbCr_422_P:
        case RK_FORMAT_YCbCr_420_SP:
        case RK_FORMAT_YCbCr_420_P:
        case RK_FORMAT_YCrCb_422_SP:
        case RK_FORMAT_YCrCb_422_P:
        case RK_FORMAT_YCrCb_420_P:
	    case RK_FORMAT_YCrCb_420_SP:
            bpp = 1.5;
            break;
        //case RK_FORMAT_YCrCb_NV12:
            //bpp = 1.5;
            break;
    	//case RK_FORMAT_YCrCb_NV12_VIDEO:
            //bpp = 1.5;
            break;
        //case RK_FORMAT_YCrCb_NV12_10:
            //bpp = 1.875;
            break;
        default:
    	    printf("Is unsupport format now,please fix \n");
            return 0;
    }

    return bpp;
}

int get_buf_size_by_w_h_f(int w, int h, int f)
{
    float bpp = get_bpp_from_format(f);
    int size = 0;

    size = (int)w * h * bpp;
    return size;
}

int get_string_by_format(char *value, int format)
{
    if (!value)
        return -EINVAL;

    switch (format) {
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
            memcpy(value, "cbcr420sp", sizeof("cbcr420sp"));
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
        //case RK_FORMAT_YCrCb_NV12:
            //memcpy(value, "nv12", sizeof("nv12"));
            break;
    	//case RK_FORMAT_YCrCb_NV12_VIDEO:
            //memcpy(value, "nv12", sizeof("nv12"));
            break;
        //case RK_FORMAT_YCrCb_NV12_10:
            //memcpy(value, "nv12_10", sizeof("nv12_10"));
            break;
        default:
    	    printf("Is unsupport format now,please fix");
            return 0;
    }

    return 0;
}

int get_buf_from_file(void *buf, int f, int sw, int sh, int index)
{
    const char *inputFilePath = "/data/in%dw%d-h%d-%s.bin";
    char filePath[100];
    char fstring[30];
    int ret = 0;

    ret = get_string_by_format(fstring, f);
    snprintf(filePath, 100, inputFilePath, index, sw, sh, fstring);
#if 1
    FILE *file = fopen(filePath, "rb");
    if (!file) {
        fprintf(stderr, "Could not open %s\n", filePath);
        return -EINVAL;
    }
    fread(buf, get_buf_size_by_w_h_f(sw, sh, f), 1, file);
    fclose(file);
#else
    {
        char *pbuf = (char*)malloc(2 * mHeight * 4864);
        for (int i = 0; i < 2160 * 1.6; i++)
            memcpy(pbuf+i*4800,buf+i*6080,4800);
        const char *outFilePath = "/data/fb3840x2160-2.yuv";
        FILE *file = fopen(outFilePath, "wb+");
        if (!file) {
            fprintf(stderr, "Could not open %s\n", outFilePath);
            return false;
        }
        fwrite(pbuf, 2 * 4864 * 2160, 1, file);
        free(pbuf);
        fclose(file);
    }
#endif

    return 0;
}

int output_buf_data_to_file(void *buf, int f, int sw, int sh, int index)
{
    const char *outputFilePath = "/data/out%dw%d-h%d-%s.bin";
    char filePath[100];
    char fstring[30];
    int ret = 0;

    ret = get_string_by_format(fstring, f);
    snprintf(filePath, 100, outputFilePath, index, sw, sh, fstring);
#if 0
    FILE *file = fopen(yuvFilePath, "rb");
    if (!file) {
        fprintf(stderr, "Could not open %s\n", yuvFilePath);
        return false;
    }
    #if 0
    {
        char *pbuf = (char*)malloc(2 * mHeight * 4864);
        fread(pbuf, 2 * 4864 * 2160, 1, file);
        for (int i = 0; i < 2160; i++)
            memcpy(buf+i*4800,pbuf+i*6080,4800);
        memset(buf+2160*4800,0x80,4800 * 2160);
    }
    #else
    fread(dstbuf, 2 * 1920 * 1088, 1, file);
    fclose(file);
    #endif
#else
    FILE *file = fopen(filePath, "wb+");
    if (!file) {
        fprintf(stderr, "Could not open %s\n", filePath);
        return false;
    } else
	    fprintf(stderr, "open %s and write ok\n", filePath);
    fwrite(buf, get_buf_size_by_w_h_f(sw, sh, f), 1, file);
    fclose(file);
#endif

    return 0;
}

