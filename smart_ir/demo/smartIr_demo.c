/*
 *  Copyright (c) 2024 Rockchip Corporation
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
 *
 */

#include "smartIr_demo.h"

#define FMT_NUM_PLANES 1
#define CLEAR(x)       memset(&(x), 0, sizeof(x))
#define BUFFER_COUNT   3

static bool g_quit = false;
static smartIr_demo_context_t* ctx = NULL;

static void sigterm_handler(int sig)
{
    fprintf(stderr, "signal %d\n", sig);
    g_quit = true;
}

static int xioctl(int fh, int request, void* arg)
{
    int r;
    do {
        r = ioctl(fh, request, arg);
    } while (-1 == r && EINTR == errno);
    return r;
}

static void init_mmap()
{
    struct v4l2_requestbuffers req;
    int fd_tmp = -1;

    CLEAR(req);

    fd_tmp = ctx->fd;

    req.count = BUFFER_COUNT;
    req.type = ctx->buf_type;
    req.memory = V4L2_MEMORY_MMAP;

    struct buffer* tmp_buffers = NULL;

    if (-1 == xioctl(fd_tmp, VIDIOC_REQBUFS, &req)) {
        if (EINVAL == errno) {
            printf("%s: %s does not support memory mapping\n", ctx->sns_name, ctx->dev_name);
        } else {
            printf("%s: VIDIOC_REQBUFS error\n", ctx->sns_name);
        }
    }

    if (req.count < 2) {
        printf("%s: Insufficient buffer memory on %s\n", ctx->sns_name, ctx->dev_name);
    }

    tmp_buffers = (struct buffer*)calloc(req.count, sizeof(struct buffer));

    if (!tmp_buffers) {
        printf("%s: Out of memory\n", ctx->sns_name);
    }

    ctx->buffers = tmp_buffers;

    for (ctx->n_buffers = 0; ctx->n_buffers < req.count; ++ctx->n_buffers) {
        struct v4l2_buffer buf;
        struct v4l2_plane planes[FMT_NUM_PLANES];
        CLEAR(buf);
        CLEAR(planes);

        buf.type = ctx->buf_type;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = ctx->n_buffers;

        if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == ctx->buf_type) {
            buf.m.planes = planes;
            buf.length = FMT_NUM_PLANES;
        }

        if (-1 == xioctl(fd_tmp, VIDIOC_QUERYBUF, &buf))
            printf("%s: VIDIOC_QUERYBUF error\n", ctx->sns_name);

        if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == ctx->buf_type) {
            tmp_buffers[ctx->n_buffers].length = buf.m.planes[0].length;
            tmp_buffers[ctx->n_buffers].start =
                mmap(NULL /* start anywhere */,
                     buf.m.planes[0].length,
                     PROT_READ | PROT_WRITE /* required */,
                     MAP_SHARED /* recommended */,
                     fd_tmp, buf.m.planes[0].m.mem_offset);
        } else {
            tmp_buffers[ctx->n_buffers].length = buf.length;
            tmp_buffers[ctx->n_buffers].start =
                mmap(NULL /* start anywhere */,
                     buf.length,
                     PROT_READ | PROT_WRITE /* required */,
                     MAP_SHARED /* recommended */,
                     fd_tmp, buf.m.offset);
        }

        if (MAP_FAILED == tmp_buffers[ctx->n_buffers].start)
            printf("%s: mmap error\n", ctx->sns_name);

        // export buf dma fd
        struct v4l2_exportbuffer expbuf;
        CLEAR(expbuf);
        expbuf.type = ctx->buf_type;
        expbuf.index = ctx->n_buffers;
        expbuf.flags = O_CLOEXEC;
        if (xioctl(fd_tmp, VIDIOC_EXPBUF, &expbuf) < 0) {
            printf("%s: get dma buf failed\n", ctx->sns_name);
        } else {
            printf("%s: get dma buf(%u)-fd: %d\n", ctx->sns_name, ctx->n_buffers, expbuf.fd);
        }
        tmp_buffers[ctx->n_buffers].export_fd = expbuf.fd;
    }
}

static void init_device()
{
    struct v4l2_capability cap;
    struct v4l2_format fmt;

    if (-1 == xioctl(ctx->fd, VIDIOC_QUERYCAP, &cap)) {
        if (EINVAL == errno) {
            printf("%s: %s is no V4L2 device\n", ctx->sns_name, ctx->dev_name);
        } else {
            printf("%s: VIDIOC_QUERYCAP error\n", ctx->sns_name);
        }
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) && !(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE)) {
        printf("%s: %s is not a video capture device, capabilities: %x\n", ctx->sns_name, ctx->dev_name, cap.capabilities);
    }

    if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
        printf("%s: %s does not support streaming i/o\n", ctx->sns_name, ctx->dev_name);
    }

    if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) {
        ctx->buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        CLEAR(fmt);
        fmt.type = ctx->buf_type;
        fmt.fmt.pix.width = ctx->width;
        fmt.fmt.pix.height = ctx->height;
        fmt.fmt.pix.pixelformat = ctx->format;
        fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
        if (ctx->limit_range)
            fmt.fmt.pix.quantization = V4L2_QUANTIZATION_LIM_RANGE;
        else
            fmt.fmt.pix.quantization = V4L2_QUANTIZATION_FULL_RANGE;

    } else if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE) {
        ctx->buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        CLEAR(fmt);
        fmt.type = ctx->buf_type;
        fmt.fmt.pix_mp.width = ctx->width;
        fmt.fmt.pix_mp.height = ctx->height;
        fmt.fmt.pix_mp.pixelformat = ctx->format;
        fmt.fmt.pix_mp.field = V4L2_FIELD_INTERLACED;
        if (ctx->limit_range)
            fmt.fmt.pix_mp.quantization = V4L2_QUANTIZATION_LIM_RANGE;
        else
            fmt.fmt.pix_mp.quantization = V4L2_QUANTIZATION_FULL_RANGE;
    }

    if (-1 == xioctl(ctx->fd, VIDIOC_S_FMT, &fmt))
        printf("%s: VIDIOC_S_FMT error\n", ctx->sns_name);

    init_mmap();
}

static void deInit_device()
{
    unsigned int i;
    if (ctx->n_buffers == 0)
        return;

    for (i = 0; i < ctx->n_buffers; ++i) {
        if (-1 == munmap(ctx->buffers[i].start, ctx->buffers[i].length))
            printf("%s: munmap error\n", ctx->sns_name);

        close(ctx->buffers[i].export_fd);
    }

    free(ctx->buffers);
    ctx->n_buffers = 0;
}

static void open_device()
{
    printf("-------- open output dev_name:%s -------------\n", ctx->dev_name);
    ctx->fd = open(ctx->dev_name, O_RDWR | O_CLOEXEC | O_NONBLOCK, 0);
    if (-1 == ctx->fd) {
        printf("Cannot open '%s': %d, %s\n", ctx->dev_name, errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
}

static void close_device()
{
    if (-1 == close(ctx->fd))
        printf("%s: close error\n", ctx->sns_name);

    ctx->fd = -1;
}

static void start_capturing()
{
    unsigned int i;
    enum v4l2_buf_type type;

    for (i = 0; i < ctx->n_buffers; ++i) {
        struct v4l2_buffer buf;

        CLEAR(buf);
        buf.type = ctx->buf_type;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == ctx->buf_type) {
            struct v4l2_plane planes[FMT_NUM_PLANES];

            buf.m.planes = planes;
            buf.length = FMT_NUM_PLANES;
        }
        if (-1 == xioctl(ctx->fd, VIDIOC_QBUF, &buf))
            printf("%s: VIDIOC_QBUF error\n", ctx->sns_name);
    }

    type = ctx->buf_type;
    printf("-------- stream on output -------------\n");

    if (-1 == xioctl(ctx->fd, VIDIOC_STREAMON, &type))
        printf("%s: VIDIOC_STREAMON error\n", ctx->sns_name);
}

static void stop_capturing()
{
    enum v4l2_buf_type type;

    type = ctx->buf_type;
    if (-1 == xioctl(ctx->fd, VIDIOC_STREAMOFF, &type))
        printf("%s: VIDIOC_STREAMOFF error\n", ctx->sns_name);
}

static int rkaiq_start()
{
    setlinebuf(stdout);
    if (strlen(ctx->iqpath) == 0) {
        printf("%s not start\n", __func__);
        ctx->aiq_ctx = NULL;
        return 0;
    }

    rk_aiq_static_info_t aiq_static_info;
    rk_aiq_uapi2_sysctl_enumStaticMetas(0, &aiq_static_info);
    printf("sensor_name: %s, iqfiles: %s\n", aiq_static_info.sensor_info.sensor_name, ctx->iqpath);

    char* sensor_name;
    sensor_name = aiq_static_info.sensor_info.sensor_name;
    memcpy(ctx->sns_name, sensor_name, sizeof(ctx->sns_name));

    rk_aiq_uapi2_sysctl_preInit_scene(sensor_name, "normal", "day");
    ctx->aiq_ctx = rk_aiq_uapi2_sysctl_init(sensor_name, ctx->iqpath, NULL, NULL);
    rk_aiq_uapi2_sysctl_prepare(ctx->aiq_ctx, 0, 0, (rk_aiq_working_mode_t)ctx->hdr_mode);
    rk_aiq_uapi2_sysctl_start(ctx->aiq_ctx);

    return 0;
}

static int rkaiq_stop()
{
    rk_aiq_uapi2_sysctl_stop(ctx->aiq_ctx, false);
    rk_aiq_uapi2_sysctl_deinit(ctx->aiq_ctx);
    ctx->aiq_ctx = NULL;

    return 0;
}

static void smartIr_cb(rk_smart_ir_result_t result)
{
    if (result.status == RK_SMART_IR_STATUS_NIGHT) {
        if (result.is_status_change) {
            printf("SMART_IR: switch to Night\n");
            // 1) switch isp night params
            rk_aiq_uapi2_sysctl_switch_scene(ctx->aiq_ctx, "normal", "night");
            // 2) ir-cutter off
            // TODO: user should define ir-cutter control func here
        }
        if (result.is_fill_change) {
            // 3) manual/auto ir-led, set result.fill_value
            // TODO: user should define led control func here
        }

    } else if (result.status == RK_SMART_IR_STATUS_DAY && result.is_status_change) {
        printf("SMART_IR: switch to Day\n");
        // 1) ir-cutter on
        // TODO: user should define ir-cutter control func here
        // 2) ir-led off
        // TODO: user should define led control func here
        // 3) switch isp day params
        rk_aiq_uapi2_sysctl_switch_scene(ctx->aiq_ctx, "normal", "day");
    }
}

static void smartIr_start()
{
    ctx->smartIr_ctx = rk_smart_ir_init(ctx->aiq_ctx);
    /* NOTE:
     * The API `rk_smart_ir_iniCfg` reads configuration from an INI file located at the configured path.
     * If the API `rk_smart_ir_setAttr` is called after `rk_smart_ir_iniCfg`, the configuration read from
     * the INI file might be overwritten by the new settings.
     */
    rk_smart_ir_iniCfg(ctx->smartIr_ctx, "tmp/smart_ir.ini");

    rk_smart_ir_attr_t attr;
    //memset(&attr, 0, sizeof(attr));
    rk_smart_ir_getAttr(ctx->smartIr_ctx, &attr);
    attr.init_status = RK_SMART_IR_STATUS_DAY;
    attr.switch_mode = RK_SMART_IR_SWITCH_MODE_AUTO;
    attr.light_mode = RK_SMART_IR_LIGHT_MODE_MANUAL;
    attr.light_type = RK_SMART_IR_LIGHT_TYPE_IR;
    attr.light_value = 100;
    attr.params.d2n_envL_th = 0.04f;
    attr.params.n2d_envL_th = 0.20f;
    attr.params.rggain_base = 1.00f;
    attr.params.bggain_base = 1.00f;
    attr.params.awbgain_rad = 0.10f;
    attr.params.awbgain_dis = 0.20f;
    attr.params.switch_cnts_th = 50;
    rk_smart_ir_setAttr(ctx->smartIr_ctx, &attr);

    rk_smart_ir_runCb(ctx->smartIr_ctx, ctx->camGroup, smartIr_cb);
}

static void smartIr_stop()
{
    rk_smart_ir_deInit(ctx->smartIr_ctx);
    ctx->smartIr_ctx = NULL;
}

static void smartIr_calib()
{
    ctx->smartIr_ctx = rk_smart_ir_init(ctx->aiq_ctx);
    rk_smart_ir_calib_t calib_cfg;
    calib_cfg.calib_en = true;

    /* calib d2n_envL_th
     * 1. Switch to ISP day mode
     * 2. Enable IR-cutter, disable IR-LED
     * 3. Adjust brightness threshold for day-to-night switch
     */
    calib_cfg.calib_mode = RK_SMART_IR_CALIB_MODE_D2N;
    calib_cfg.calib_params.d2n_envL_th = 0.04f; //need cfg
    rk_smart_ir_calib(ctx->smartIr_ctx, &calib_cfg);

    /* calib n2d_envL_th
     * 1. Switch to ISP night mode
     * 2. Disable IR-cutter, enable IR-LED
     * 3. Adjust brightness threshold for night-to-day switch
     */
    calib_cfg.calib_mode = RK_SMART_IR_CALIB_MODE_N2D;
    calib_cfg.calib_params.n2d_envL_th = 0.2f; //need cfg
    rk_smart_ir_calib(ctx->smartIr_ctx, &calib_cfg);

    /* calib awbgain base
     * 1. Switch to ISP night mode
     * 2. Disable IR-cutter, enable IR-LED
     * 3. Ensure no visible light
     */
    calib_cfg.calib_mode = RK_SMART_IR_CALIB_MODE_BASE;
    rk_smart_ir_calib(ctx->smartIr_ctx, &calib_cfg);

    /* tune awbgain dis
     * 1. Switch to ISP night mode
     * 2. Disable IR-cutter, enable IR-LED
     * 3. Test various scenarios and get the maximum value
     */
    calib_cfg.calib_mode = RK_SMART_IR_CALIB_MODE_DIS;
    calib_cfg.calib_params.rggain_base = 1.00f; //need cfg
    calib_cfg.calib_params.bggain_base = 1.00f; //need cfg
    calib_cfg.calib_params.awbgain_rad = 0.10f; //need cfg
    rk_smart_ir_calib(ctx->smartIr_ctx, &calib_cfg);

    rk_smart_ir_deInit(ctx->smartIr_ctx);
    ctx->smartIr_ctx = NULL;
}

static char optstr[] = "?:h:d:a:t:c:";

static void print_usage(const char* name)
{
    printf("Usage example:\n");
    printf("\t-d | --device: required, main path of video device\n");
    printf("\t-a | --aiq: iq file path, default: /etc/iqfiles\n");
    printf("\t-t | --time: run time, default: -1 seconds\n");
    printf("\t-c | --calib: calib params, default: 0 disable\n");
}

int main(int argc, char* argv[])
{
    ctx = (smartIr_demo_context_t*)calloc(1, sizeof(smartIr_demo_context_t));

    strcpy(ctx->dev_name, "/dev/video0");
    strcpy(ctx->sns_name, "m00_b_sc200ai");
    strcpy(ctx->iqpath, "/etc/iqfiles");
    ctx->width = 640;
    ctx->height = 480;
    ctx->format = V4L2_PIX_FMT_NV12;
    ctx->fd = -1;
    ctx->buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ctx->buffers = NULL;
    ctx->n_buffers = 0;
    ctx->aiq_ctx = NULL;
    ctx->smartIr_ctx = NULL;
    ctx->hdr_mode = RK_AIQ_WORKING_MODE_NORMAL;
    ctx->limit_range = 0;
    ctx->run_time = -1;
    ctx->camGroup = false;

    int c;

    while ((c = getopt(argc, argv, optstr)) != -1) {
        switch (c) {
        case 'a':
            if (optarg)
                strcpy(ctx->iqpath, optarg);
            break;
        case 't':
            if (optarg)
                ctx->run_time = atoi(optarg);
            break;
        case 'd':
            if (optarg)
                strcpy(ctx->dev_name, optarg);
            break;
        case 'c':
            if (optarg)
                ctx->en_calib = atoi(optarg);
            break;
        case '?':
        case 'h':
        default:
            print_usage(argv[0]);
            return 0;
        }
    }

    printf("iq file path: %s, device name: %s, run time: %d, en calib: %d\n\n",
            ctx->iqpath, ctx->dev_name, ctx->run_time, ctx->en_calib);

    signal(SIGINT, sigterm_handler);

    open_device();
    rkaiq_start();
    init_device();
    start_capturing();

    if (!ctx->en_calib)
        smartIr_start();

    while (!g_quit && ((ctx->run_time == -1) || (ctx->run_time-- > 0))) {
        usleep(1000 * 1000);

        if (ctx->en_calib)
            smartIr_calib();
    }

    if (!ctx->en_calib)
        smartIr_stop();

    stop_capturing();
    rkaiq_stop();
    deInit_device();
    close_device();

    free(ctx);

    return 0;
}
