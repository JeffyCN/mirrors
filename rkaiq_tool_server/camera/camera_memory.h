#ifndef _CAMERA_MEMORY_H_
#define _CAMERA_MEMORY_H_

#include <assert.h>
#include <fcntl.h>  /* low-level i/o */
#include <getopt.h> /* getopt_long() */
#include <linux/v4l2-controls.h>
#include <linux/v4l2-subdev.h>
#include <linux/videodev2.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
using namespace std;
#include "rk_aiq_types.h"

#include "logger/log.h"

#define FMT_NUM_PLANES 1
#define CLEAR(x) memset(&(x), 0, sizeof(x))
#define DEV_PATH_LEN 64
#define MAX_MEDIA_INDEX 20

enum io_method
{
    IO_METHOD_READ,
    IO_METHOD_MMAP,
    IO_METHOD_USERPTR,
};

enum sensor_link
{
    link_to_vicap,
    link_to_isp,
    link_to_dvp,
};

struct buffer
{
    void* start;
    size_t length;
    int socket;
    unsigned short checksum;
};

struct isp_t
{
    char media_dev_path[DEV_PATH_LEN];
    char isp_main_path[DEV_PATH_LEN];
    char isp_sd_path[DEV_PATH_LEN];
    int width;
    int height;
    __u32 sd_fmt;
};

struct vicap_t
{
    char cif_video_path[DEV_PATH_LEN];
    int width;
    int height;
    __u32 sd_fmt;
};

struct sensor_t
{
    char device_name[DEV_PATH_LEN];
    char sensor_name[DEV_PATH_LEN];
    int width;
    int height;
    int bits;
    __u32 sen_fmt;
};

struct lens_t
{
    char lens_device_name[DEV_PATH_LEN];
};

#pragma pack(1)
typedef struct RkAiqExpRealParam_s1
{

    // M4_NUMBER_DESC("CISTime", "f32", M4_RANGE(0,1), "0", M4_DIGIT(6))
    float integration_time;

    // M4_NUMBER_DESC("CISGain", "f32", M4_RANGE(0,4096), "0", M4_DIGIT(3))
    float analog_gain;

    // M4_NUMBER_DESC("digital_gain", "f32", M4_RANGE(0,4096), "0", M4_DIGIT(3),M4_HIDE(1))
    float digital_gain;

    // M4_NUMBER_DESC("isp_dgain", "f32", M4_RANGE(0,256), "0", M4_DIGIT(3),M4_HIDE(1))
    float isp_dgain;

    // M4_NUMBER_DESC("iso", "s32", M4_RANGE(0,524288), "0", M4_DIGIT(0),M4_HIDE(1))
    int iso;

    // M4_NUMBER_DESC("DcgMode", "s32", M4_RANGE(-1,1), "0", M4_DIGIT(0))
    int dcg_mode;

    // M4_NUMBER_DESC("longfrm_mode", "s32", M4_RANGE(0,1), "0", M4_DIGIT(0),M4_HIDE(1))
    int longfrm_mode;
} RkAiqExpRealParam_t1;

typedef struct RkAiqExpSensorParam_s1
{

    // M4_NUMBER_DESC("fine_integration_time", "u32", M4_RANGE(0,65535), "0", M4_DIGIT(0),M4_HIDE(1))
    unsigned int fine_integration_time;

    // M4_NUMBER_DESC("coarse_integration_time", "u32", M4_RANGE(0,65535), "0", M4_DIGIT(0),M4_HIDE(1))
    unsigned int coarse_integration_time;

    // M4_NUMBER_DESC("analog_gain_code_global", "u32", M4_RANGE(0,524288), "0", M4_DIGIT(0),M4_HIDE(1))
    unsigned int analog_gain_code_global;

    // M4_NUMBER_DESC("digital_gain_global", "u32", M4_RANGE(0,65535), "0", M4_DIGIT(0),M4_HIDE(1))
    unsigned int digital_gain_global;

    // M4_NUMBER_DESC("isp_digital_gain", "u32", M4_RANGE(0,65535), "0", M4_DIGIT(0),M4_HIDE(1))
    unsigned int isp_digital_gain;
} RkAiqExpSensorParam_t1;

typedef struct
{
    // M4_STRUCT_DESC("RealPara", "normal_ui_style")
    RkAiqExpRealParam_t1 exp_real_params; // real value

    // M4_STRUCT_DESC("RegPara", "normal_ui_style",M4_HIDE(1))
    RkAiqExpSensorParam_t1 exp_sensor_params; // reg value
} RkAiqExpParamComb_t1;

typedef struct ExpInfo_s1
{
    uint32_t frameId;
    RkAiqExpParamComb_t1 linearExp;
    RkAiqExpParamComb_t1 hdrExp[3];
} ExpInfo_t1;
#pragma pack()

struct capture_info
{
    const char* dev_name;
    int dev_fd;
    int subdev_fd;
    int lensdev_fd;
    enum io_method io;
    struct buffer* buffers;
    struct isp_t vd_path;
    struct vicap_t cif_path;
    struct sensor_t sd_path;
    struct lens_t lens_path;
    unsigned int n_buffers;
    __u32 format;
    int width;
    int height;
    int lhcg;

    uint sequence;
    vector<rk_aiq_isp_stats_t> ispStatsList;

    enum sensor_link link;
    enum v4l2_buf_type capture_buf_type;
    int frame_count;
    char out_file[255];
    FILE* out_fp;
};

int xioctl(int fh, int request, void* arg);
void init_read(struct capture_info* cap_info, unsigned int buffer_size);
void init_mmap(struct capture_info* cap_info);
void init_userp(struct capture_info* cap_info, unsigned int buffer_size);

int check_io_method(enum io_method io, unsigned int capabilities);
int init_io_method(struct capture_info* cap_info, unsigned int size);

#endif
