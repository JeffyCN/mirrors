#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * @file raw_reader.cpp
 * @brief RAW图像文件读取和处理模块
 * 
 * 本模块负责从RAW格式图像文件中读取数据，进行预处理，
 * 并通过定时器回调机制周期性地处理图像帧。
 * 支持RAW12/RAW14格式的灰度图像处理。
 */

// 全局变量定义
static uint16_t *process_buffer = NULL; ///< 图像处理缓冲区(16位像素数据)
static FILE *rawfp = NULL;             ///< RAW文件指针
static uint32_t rawsize = 0;           ///< RAW文件总大小(字节)
static uint32_t rawcnt = 0;            ///< RAW文件中包含的图像帧数
static int image_pixels = 0;
static int image_size = 0;

uint16_t *rawreader_getFrame(void)
{
    // 从RAW文件中读取一帧图像数据
    int ret = fread(process_buffer, 1, image_size, rawfp);
    if (ret == image_size)
        return process_buffer;
    return NULL;
}

/**
 * @brief RAW读取器初始化函数
 * 
 * 初始化RAW文件读取环境，包括:
 * 1. 设置图像参数(宽度、高度、像素数、帧大小)
 * 2. 分配处理缓冲区
 * 3. 打开RAW文件并获取文件信息
 * 4. 初始化图像处理模块
 * 5. 设置定时器进行周期性图像处理
 * 
 * @return 成功返回0，失败返回-1
 */
int rawreader_setup(const char *fn, int w, int h)
{
    // 设置图像参数(假设为400x300分辨率)
    image_pixels = w * h;
    image_size = w * h * 2;  // 每个像素2字节(16位)

    // 分配图像处理缓冲区(16位像素数据)
    process_buffer = (uint16_t *)malloc(image_pixels * 2);


    // 打开RAW文件(二进制只读模式)
    rawfp = fopen(fn, "rb");
    if(rawfp == NULL){
        fprintf(stderr, "open file %s error!!", fn);
        return -1;
    }

    // 获取文件大小和帧数
    fseek(rawfp,0,SEEK_END);
    rawsize = ftell(rawfp);
    fseek(rawfp, 0, SEEK_SET);
    rawcnt = rawsize / image_size;  // 计算总帧数
    
    printf("file %s, rawszie %d, rawcnt = %d\n", fn, rawsize, rawcnt);
    return 0;
}

/**
 * @brief RAW读取器资源清理函数
 * 
 * 释放RAW读取器占用的所有资源，包括:
 * 1. 关闭RAW文件
 * 2. 反初始化图像处理模块
 * 3. 释放处理缓冲区内存
 */
void rawreader_reset(void)
{
    // 关闭RAW文件(如果已打开)
    if (rawfp)
        fclose(rawfp);
    
    // 释放处理缓冲区内存
    free(process_buffer);
}

void debug_save_y16(const char *name, int16_t *data, uint32_t seq)
{
    char fn[128];
    FILE *fp = NULL;
    sprintf(fn, "/tmp/%s_y16.raw", name);
    if (seq == 0) {
        printf("savefn %s\n", fn);
        fp = fopen(fn, "wb");
    } else {
        fp = fopen(fn, "ab");
    }
    fwrite(data, image_pixels, 2, fp);
    fclose(fp);
}

void debug_save_y8(const char *name, uint8_t *data, uint32_t seq)
{
    char fn[128];
    FILE *fp = NULL;
    sprintf(fn, "/tmp/%s_y8.raw", name);
    if (seq == 0) {
        printf("savefn %s\n", fn);
        fp = fopen(fn, "wb");
    } else {
        fp = fopen(fn, "ab");
    }
    fwrite(data, image_pixels, 1, fp);
    fclose(fp);
}

void debug_save_rgb(const char *name, uint32_t *data, uint32_t seq)
{
    char fn[128];
    FILE *fp = NULL;
    sprintf(fn, "/tmp/%s_rgb.raw", name);
    if (seq == 0) {
        printf("savefn %s\n", fn);
        fp = fopen(fn, "wb");
    } else {
        fp = fopen(fn, "ab");
    }
    fwrite(data, image_pixels, 4, fp);
    fclose(fp);
}
