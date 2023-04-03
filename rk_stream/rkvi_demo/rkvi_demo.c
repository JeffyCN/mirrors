#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/videodev2.h>
#include <linux/v4l2-subdev.h>
#include <string.h>
#include "rk_aiq_types.h"
#include "rk_aiq_offline_raw.h"
#include "rk_vi_user_api2_stream_cfg.h"
#include "uAPI2/rk_aiq_user_api2_sysctl.h"
#include "rk_aiq_offline_raw.h"
#include "rkcif-config.h"
//#include <iostream>
#include <getopt.h>

#define USE_LIBAIQ

#define SENSOR_ENTNAME "m01_b_ov50c40 4-0036"

#define VIDEO_DEVNAME "/dev/video24"
#define VIDEO_WIDTH 4096
#define VIDEO_HEIGHT 3072
#define VIDEO_WORKMODE RK_AIQ_WORKING_MODE_NORMAL

#define VIDEO_WIDTH_2 3840
#define VIDEO_HEIGHT_2 2160
#define VIDEO_WORKMODE_2 RK_AIQ_WORKING_MODE_NORMAL

#define RAWCAP_SAVE_NUM    30
#define STREAM_SAVE_NUM    30

struct rk_aiq_vbuf_info {
    uint32_t frame_id;
    uint32_t timestamp;
    float    exp_time;
    float    exp_gain;
    uint32_t exp_time_reg;
    uint32_t exp_gain_reg;
    uint32_t data_fd;
    uint8_t *data_addr;
    uint32_t data_length;
    bool valid;
};

struct rk_aiq_vbuf {
    void *base_addr;
    uint32_t frame_width;
    uint32_t frame_height;
    struct rk_aiq_vbuf_info buf_info[3];/*index: 0-short,1-medium,2-long*/
};


static rk_aiq_rawbuf_type_t _rawbuf_type;
static struct _st_addrinfo _st_addr[3];
static struct _raw_format _rawfmt;
static rk_aiq_frame_info_t _finfo;

static rkraw_vi_ctx_t *g_vi_ctx;

static int g_quit = 0;
static int g_aiq_quit = 0;
static int g_aiq_pause = 0;
static int g_aiq_test_mode = 0;

static int saved_num = 0;
static char g_sns_entity_name[64] = {0};
static char g_sns_name[64] = {0};

static bool use_isp_driver = false;
static int raw_width = 3840;
static int raw_height = 2160;
static int video_width = 640;
static int video_height = 480;
static int modul_select = 0;
static int hdr_mode = 0;
static const char *isp_vir;
static const char *real_sns;
static const char *iq_file_name;
static const char *video_name;
static uint32_t pix_fmt = V4L2_PIX_FMT_SBGGR10;

static FILE* g_rkraw_fp;
static uint8_t* g_rkraw_buffer;
static uint32_t g_rkraw_size;
static int g_rkraw_index;

static struct timeval start_t, cur_t, prev_t;

static rk_aiq_sys_ctx_t* g_aiq_ctx;

// enum v4l2_memory _tx_memory_type;
// enum v4l2_memory _rx_memory_type;
// SmartPtr<V4l2Device> g_vi_ctx->_mRawProcUnit->_mipi_rx_devs[3];
// SmartPtr<V4l2Device> _mipi_tx_devs[3];

static int capture_raw(uint8_t *rkraw_data, uint32_t rkraw_len)
{
    rkrawstream_rkraw2_t rkraw;
    struct _block_header _header;
    _header.block_id = NORMAL_RAW_TAG;
    uint16_t end_tag = END_TAG;

    rkrawstream_uapi_parase_rkraw2(rkraw_data, &rkraw);

    char filname[32];
    sprintf(filname, "/tmp/raw%d", saved_num);
    FILE *fp = fopen(filname,"wb");
    if(fp){
        int ret = 0;
        ret += fwrite((void *)rkraw.plane[0].addr, 1, rkraw.plane[0].size, fp);
        printf("%s:save raw %s, ret %d\n", __func__, filname, ret);
        fclose(fp);
    }

    return 0;
}

static int on_frame_ready_capture(uint8_t *rkraw_data, uint32_t rkraw_len)
{
    if(g_quit == 0){

        capture_raw(rkraw_data, rkraw_len);
        saved_num++;
        if(saved_num == RAWCAP_SAVE_NUM)
            g_quit = 1;
    }
    return 0;
}

static int on_frame_ready_streaming(uint8_t *rkraw_data, uint32_t rkraw_len)
{
    printf("on_frame_ready_streaming11111\n");
    //capture_raw(rkraw_data, rkraw_len);

    rkrawstream_rkraw2_t rkraw;
    rkrawstream_uapi_parase_rkraw2(rkraw_data, &rkraw);
    rkrawstream_vicap_buf_take(g_vi_ctx);
    rkrawstream_readback_set_rkraw2(g_vi_ctx, &rkraw);

    saved_num++;
    if(saved_num == STREAM_SAVE_NUM)
        g_quit = 1;
    return 0;
}

static void on_isp_process_done_streaming(int dev_index)
{
    printf("on_isp_process_done_streaming\n");
    rkrawstream_vicap_buf_return(g_vi_ctx, dev_index);
}

static void on_isp_process_done_offline(int dev_index)
{
    printf("%s:index %d\n", __func__, dev_index);
}

void print_sensor_info(rk_aiq_static_info_t *s_info)
{
    int i = 0;
    printf("\n\n****** sensor %s infos: *********\n",s_info->sensor_info.sensor_name);
    for(i=0; i<10; i++){
        if(s_info->sensor_info.support_fmt[i].width >0)
         printf("format%d: %dx%d, format 0x%x, fps %d hdr %d\n",i,
            s_info->sensor_info.support_fmt[i].width,
            s_info->sensor_info.support_fmt[i].height,
            s_info->sensor_info.support_fmt[i].format,
            s_info->sensor_info.support_fmt[i].fps,
            s_info->sensor_info.support_fmt[i].hdr_mode);
    }
}

#include "rkvi_demo_isp.c_part"

int rawcap_test(void)
{
    strcpy(g_sns_entity_name, SENSOR_ENTNAME);
    printf("g_sns_entity_name:%s\n", g_sns_entity_name);
    sscanf(&g_sns_entity_name[6], "%s", g_sns_name);
    printf("sns_name:%s\n", g_sns_name);

    rkraw_vi_init_params_t init_p;
    rkraw_vi_prepare_params_t prepare_p;

    /* init rkraw_vi user api */
    g_vi_ctx = rkrawstream_uapi_init();
    init_p.sns_ent_name = g_sns_entity_name;
    init_p.use_offline = false;
    rkrawstream_vicap_init(g_vi_ctx, &init_p);

    prepare_p.width = VIDEO_WIDTH;
    prepare_p.height = VIDEO_HEIGHT;
    prepare_p.pix_fmt = V4L2_PIX_FMT_SGBRG10;
    prepare_p.hdr_mode = VIDEO_WORKMODE;
    prepare_p.mem_mode = CSI_LVDS_MEM_WORD_LOW_ALIGN;
    prepare_p.buf_memory_type = 0;
    rkrawstream_vicap_prepare(g_vi_ctx, &prepare_p);

    rkrawstream_vicap_start(g_vi_ctx, on_frame_ready_capture);

    while(g_quit == 0)
        usleep(10000);

    g_aiq_quit = 1;
    rkrawstream_vicap_stop(g_vi_ctx);
    /* clean up library. */
    rkrawstream_uapi_deinit(g_vi_ctx);
    return 0;
}


int stream_test(void)
{
    pthread_t isp_tid;
    strcpy(g_sns_entity_name, SENSOR_ENTNAME);
    printf("g_sns_entity_name:%s\n", g_sns_entity_name);
    sscanf(&g_sns_entity_name[6], "%s", g_sns_name);
    printf("sns_name:%s\n", g_sns_name);

    rkraw_vi_init_params_t init_p;
    rkraw_vi_prepare_params_t prepare_p;
    int param[2];

    g_ispfd = open(VIDEO_DEVNAME, O_RDWR /* required */ /*| O_NONBLOCK*/, 0);
    if (-1 == g_ispfd) {
        printf("Cannot open '%s'\n", VIDEO_DEVNAME);
        return 0;
    }

    /* init rkraw_vi user api */
    g_vi_ctx = rkrawstream_uapi_init();
    init_p.sns_ent_name = g_sns_entity_name;
    init_p.use_offline = false;
    rkrawstream_vicap_init(g_vi_ctx, &init_p);
    rkrawstream_readback_init(g_vi_ctx, &init_p);

    prepare_p.width = VIDEO_WIDTH;
    prepare_p.height = VIDEO_HEIGHT;
    prepare_p.pix_fmt = V4L2_PIX_FMT_SGBRG10;
    prepare_p.hdr_mode = VIDEO_WORKMODE;
    prepare_p.mem_mode = 0;
    prepare_p.buf_memory_type = 0;
    rkrawstream_vicap_prepare(g_vi_ctx, &prepare_p);

    prepare_p.width = 4096;
    prepare_p.height = 3072;
    prepare_p.buf_memory_type = V4L2_MEMORY_DMABUF;
    rkrawstream_readback_prepare(g_vi_ctx, &prepare_p);

    //g_aiq_ctx = rk_aiq_uapi2_sysctl_init(g_sns_entity_name, "/etc/iqfiles", NULL, NULL);

    g_aiq_pause = 1;
    g_aiq_quit = 0;
    param[0] = 4096;
    param[1] = 3072;
    pthread_create(&isp_tid, NULL, isp_thread, param);


    //rk_aiq_uapi2_sysctl_prepare(g_aiq_ctx, VIDEO_WIDTH, VIDEO_HEIGHT, VIDEO_WORKMODE);
    rkrawstream_setup_pipline_fmt(g_vi_ctx, 4096, 3072);

    //rk_aiq_uapi2_sysctl_start(g_aiq_ctx);

    printf("rkisp_init_device %d %d\n", param[0], param[1]);
    rkisp_init_device(param[0], param[1]);
    rkisp_start_capturing();
    g_aiq_pause = 0;
    printf("sleeping...\n");
    sleep(3);

    rkrawstream_vicap_start(g_vi_ctx, on_frame_ready_streaming);
    rkrawstream_readback_start(g_vi_ctx, on_isp_process_done_streaming);


    while(g_quit == 0)
        usleep(10000);

    g_aiq_quit = 1;
    rkrawstream_vicap_stop(g_vi_ctx);
    rkrawstream_readback_stop(g_vi_ctx);
    //rk_aiq_uapi2_sysctl_stop(g_aiq_ctx, false);
    /* clean up library. */
    rkrawstream_uapi_deinit(g_vi_ctx);

    pthread_join(isp_tid, NULL);

    rkisp_stop_capturing();
    rkisp_uninit_device();
    //rk_aiq_uapi2_sysctl_deinit(g_aiq_ctx);

    close(g_ispfd);
    return 0;
}

void release_buffer(void *addr) {
    printf("release buffer called: addr=%p\n", addr);
}

void print_help()
{
    printf("\n\n****** librkraw_vi demo by ydb *********\n\n");
    printf("--rawcap      capture rkraw file in /tmp.\n");
    printf("--stream      live stream to isp.\n");
    printf("--offaiq      run aiq and isp offline.\n");
    printf("you must choose one from rawcap, stream and offaiq, and you just can choose one of them\n");
    printf("\nif you choose --offaiq, there are some params you can config\n");
    printf("--width       video width, default value 640\n");
    printf("--height      video height, default value 480\n");
    printf("--rawfmt      raw width and height, config it such as 640x480, use 'x', don't use 'X'\n");
    printf("--iqfile      enter iqfile name, iq file can be found at /etc/iqfiles/\n");
    printf("--ispdriver   isp driver module name, such as rkisp0-vir0\n");
    printf("--video       mainpath video on the isp driver you choosed, such as /dev/video8\n");
    printf("--sensor      what sensor name of raw, such as imx415\n");
    printf("--pix         raw pix fmt, such as GB10(SGBRG10), default value BG10(SBGGR10)\n");
    printf("              all pix fmt BG10, GB10, BA10, RG10, BG12, GB12, RG12, BA12, BG14, GB14, RG14, BA14\n");
    printf("--har         optional, hdr mode, val 2 means hdrx2, 3 means hdrx3, default value 0(normal mode)");
    printf("--help        print help info\n");
}

static void parse_args(int argc, char **argv)
{
    int c;
    int digit_optind = 0;
    optind = 0;
    while (1) {
        int this_option_optind = optind ? optind : 1;
        int option_index = 0;
        static struct option long_options[] = {
            {"width",    required_argument, 0, 'w' },
            {"height",   required_argument, 0, 'h' },
            {"offaiq",   no_argument, 0, 'f' },
            {"video",    required_argument, 0, 'v' },
            {"stream",   no_argument, 0, 'o' },
            {"rawcap",   no_argument, 0, 'r' },
            {"help",     no_argument,       0, 'p' },
            {"iqfile",   required_argument, 0, 'i' },
            {"sensor",   required_argument, 0, 's' },
            {"ispdriver", required_argument, 0, 'd' },
            {"hdr",      required_argument, 0, 'a'},
            {"pix",      required_argument, 0, 'x'},
            {"rawfmt",   required_argument, 0, 't'},
            {0,          0,                 0,  0  }
        };

        //c = getopt_long(argc, argv, "w:h:f:i:d:o:c:ps",
        c = getopt_long(argc, argv, "w:h:v:d:i:s:t:orfp",
                        long_options, &option_index);
        if (c == -1)
            break;

        switch (c)
        {
        case 'w':
            video_width = atoi(optarg);
            break;
        case 'h':
            video_height = atoi(optarg);
            break;
        case 'f':
        {
            use_isp_driver = true;
            modul_select = 1;
        }
            break;
        case 'v':
            video_name = optarg;
            break;
        case 'd':
            isp_vir = optarg;
            break;
        case 'i':
            iq_file_name = optarg;
            break;
        case 's':
            real_sns = optarg;
            break;
        case 'o':
            modul_select = 2;
            break;
        case 'r':
            modul_select = 3;
            break;
        case 'p':
            print_help();
            break;
        case 'a':
        {
            int mode_val = atoi(optarg);
            if (mode_val == 2) {
                hdr_mode = RK_AIQ_WORKING_MODE_ISP_HDR2;
            }else if (mode_val == 3) {
                hdr_mode = RK_AIQ_WORKING_MODE_ISP_HDR3;
            } else {
                hdr_mode = RK_AIQ_WORKING_MODE_NORMAL;
            }

        }
            break;
        case 'x':
        {
            if (strcmp(optarg, "BG10") == 0)
                    pix_fmt = V4L2_PIX_FMT_SBGGR10;
                else if (strcmp(optarg, "GB10") == 0)
                    pix_fmt = V4L2_PIX_FMT_SGBRG10;
                else if (strcmp(optarg, "RG10") == 0)
                    pix_fmt = V4L2_PIX_FMT_SRGGB10;
                else if (strcmp(optarg, "BA10") == 0)
                    pix_fmt = V4L2_PIX_FMT_SGRBG10;
                else if (strcmp(optarg, "BG12") == 0)
                    pix_fmt = V4L2_PIX_FMT_SBGGR12;
                else if (strcmp(optarg, "GB12") == 0)
                    pix_fmt = V4L2_PIX_FMT_SGBRG12;
                else if (strcmp(optarg, "RG12") == 0)
                    pix_fmt = V4L2_PIX_FMT_SRGGB12;
                else if (strcmp(optarg, "BA12") == 0)
                    pix_fmt = V4L2_PIX_FMT_SGRBG12;
                else if (strcmp(optarg, "BG14") == 0)
                    pix_fmt = V4L2_PIX_FMT_SBGGR14;
                else if (strcmp(optarg, "GB14") == 0)
                    pix_fmt = V4L2_PIX_FMT_SGBRG14;
                else if (strcmp(optarg, "RG14") == 0)
                    pix_fmt = V4L2_PIX_FMT_SRGGB14;
                else if (strcmp(optarg, "BA14") == 0)
                    pix_fmt = V4L2_PIX_FMT_SGRBG14;
                else
                    pix_fmt = V4L2_PIX_FMT_SBGGR10;
        }
            break;
        case 't':
        {
            raw_width = atoi(optarg);
            char *raw_fmt = strstr(optarg, "x");
            raw_height = atoi(raw_fmt + 1);
        }
        default:
            break;
        }
    }
}

int main(int argc, char **argv)
{
    bool use_aiq = false;
    bool use_rawcap = false;
    bool use_rawstream = false;
    bool test_switch = false;
    bool use_offline = false;
    pthread_t isp_tid;
    g_aiq_test_mode = 0;

    //rkraw_vi_sensor_info_t m_info = {0};
    rk_aiq_static_info_t s_info;

    parse_args(argc, argv);
    if (!modul_select)
        return 0;
    switch (modul_select)
    {
    case 2:
        stream_test();
        break;
    case 3:
        rawcap_test();
        break;
    default:
        break;
    }
    return 0;
}

