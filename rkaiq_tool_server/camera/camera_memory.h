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
typedef struct RkToolExpToolParam_s
{
    float integration_time;
    float analog_gain;
    float digital_gain;
    float isp_dgain;
    int iso;
    int dcg_mode;
    int longfrm_mode;
} RkToolExpRealParam_t;

typedef struct RkToolExpSensorParam_s
{
    uint32_t fine_integration_time;
    uint32_t coarse_integration_time;
    uint32_t analog_gain_code_global;
    uint32_t digital_gain_global;
    uint32_t isp_digital_gain;
} RkToolExpSensorParam_t;
typedef struct RkToolExpParam_s
{
    RkToolExpRealParam_t exp_real_params;     // real value
    RkToolExpSensorParam_t exp_sensor_params; // reg value
} RkToolExpParam_t;

typedef struct rk_aiq_isp_tool_stats_s
{
    uint16_t version; // 0x0100 = v1.0
    uint32_t frameID;
    RkToolExpParam_t linearExp; // from RKAiqAecStats_t::ae_exp.LinearExp.exp_real_params
    RkToolExpParam_t hdrExp[3];
    uint8_t reserved[256];
} rk_aiq_isp_tool_stats_t;
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
    vector<rk_aiq_isp_tool_stats_t> ispStatsList;

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
