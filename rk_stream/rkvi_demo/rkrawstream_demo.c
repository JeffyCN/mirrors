/*
 *  Copyright (c) 2023 Rockchip Corporation
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
 * DB Yang <db.yang@rock-chips.com>
 * 2023/2/27
 */

// example command:
// rkvi_demo --entity="m00_b_imx415 5-001a"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <getopt.h>
#include <ctype.h>
#include <stdbool.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <linux/videodev2.h>
#include <linux/media-bus-format.h>
#include "rkrawstream_user_api.h"

#include "rkisp2-config.h"
#include "rkrawstream_demo.h"
#include "rkrawstream_video_out.h"

#ifdef HAVE_RKAIQ
#include "uAPI2/rk_aiq_user_api2_sysctl.h"
#endif

#define CLEAR(x) memset(&(x), 0, sizeof(x))

static int xioctl(int fh, int request, void *arg)
{
    int r;

    do {
        r = ioctl(fh, request, arg);
    } while (-1 == r && EINTR == errno);

    return r;
}

static int wait_stream_event(int fd, unsigned int event_type, int time_out_ms)
{
    int ret;
    struct v4l2_event event;

    CLEAR(event);

    do {
        /*
         * xioctl instead of poll.
         * Since poll() cannot wait for input before stream on,
         * it will return an error directly. So, use ioctl to
         * dequeue event and block until sucess.
         */
        ret = xioctl(fd, VIDIOC_DQEVENT, &event);
        if (ret == 0 && event.type == event_type)
            return 0;
    } while (true);

    return -1;

}

static int subscrible_stream_event(const char *dev_path, int fd, bool subs)
{
    struct v4l2_event_subscription sub;
    int ret = 0;

    CLEAR(sub);
    sub.type = CIFISP_V4L2_EVENT_STREAM_START;
    ret = xioctl(fd,
                 subs ? VIDIOC_SUBSCRIBE_EVENT : VIDIOC_UNSUBSCRIBE_EVENT,
                 &sub);
    if (ret) {
        printf("can't subscribe %s start event!\n", dev_path);
        exit(EXIT_FAILURE);
    }

    CLEAR(sub);
    sub.type = CIFISP_V4L2_EVENT_STREAM_STOP;
    ret = xioctl(fd,
                 subs ? VIDIOC_SUBSCRIBE_EVENT : VIDIOC_UNSUBSCRIBE_EVENT,
                 &sub);
    if (ret) {
        printf("can't subscribe %s stop event!\n", dev_path);
    }

    printf("subscribe events from %s success !\n", dev_path);

    return 0;
}


struct demo_ctx{
    rkraw_vi_ctx_t *victx;
    char *entity;
    char *isp_driver;
    char *mode;

    int vicap_w;
    int vicap_h;
    int vicap_pixfmt;
    int vicap_memtype;
    int vicap_buftype;

    int readback_w;
    int readback_h;
    int readback_pixfmt;
    int readback_memtype;
    int readback_buftype;

    int frame_num;
    int frame_skip;
    char *frame_dir;
    int frame_cnt;

    char *raw_file;
    FILE *rkraw_fp;
    uint8_t *rkraw_buffer;
    uint32_t rkraw_size;

    uint8_t use_aiq;
    uint8_t use_rockit;
    char *iqpath;

    char* mainpath_out;
    int mp_w;
    int mp_h;
    int mp_p;
    int mp_buftype;

#ifdef HAVE_RKAIQ
    rk_aiq_sys_ctx_t* aiqctx;
#endif
};

static struct option long_options[] = {
    {"rkaiq", no_argument, 0, OptUseRkaiq},
    {"iqpath", required_argument, 0, OptSetIqfileDir},
    {"entity", required_argument, 0, OptSetEntityName},
    {"help", no_argument, 0, OptHelp},
    {"mode", required_argument, 0, OptSetMode},
    {"vicap_fmt", required_argument, 0, OptSetVicapFmt},
    {"readback_fmt", required_argument, 0, OptSetReadbackFmt},
    {"frame_num", required_argument, 0, OptSetFrameNum},
    {"frame_skip", required_argument, 0, OptSetFrameSkip},
    {"frame_dir", required_argument, 0, OptSetFrameDir},
    {"raw_file", required_argument, 0, OptSetRawFile},
    {"isp_driver", required_argument, 0, OptSetIspDriver},
    {"mainpath_out", required_argument, 0, OptSetMainpathOut},
    {"mainpath_fmt", required_argument, 0, OptSetMainpathFmt},
    {0, 0, 0, 0}
};

static void help_info() {
    printf("rkrawstream_demo help info:\n"
           "        --rkaiq             optional, enable aiq\n"
           "        --iqpath            optional, path of iqfile\n"
           "        --entity            optional, sensor entity name\n"
           "        --help              optional, help info\n"
           "        --mode              optional, rkrawstream_demo working mode: half, offline, rawcap\n"
           "        --vicap_fmt         optional, vicap format info\n"
           "                                      format can be set: width, height, pixelformat, buftype, memtype\n"
           "        --readback_fmt      optional, readback modul format info\n"
           "                                      format can be set: width, height, pixelformat, buftype, memtype\n"
           "        --frame_num         required, demo run frame count\n"
           "        --frame_skip        optional, ISP frame output skip frame\n"
           "        --frame_dir         optional, raw dir\n"
           "        --raw_file          optional, path of raw\n"
           "        --isp_driver        optional, ISP driver name\n"
           "        --mainpath_out      optional, ISP output video path\n"
           "        --mainpath_fmt      optional, ISP output video format\n"
           "                                      format can be set: width, height, pixelformat\n");
}

int parse_subopt(char **subs, const char * const *subopts, char **value)
{
	int opt = getsubopt(subs, (char * const *)subopts, value);

	if (opt == -1) {
		fprintf(stderr, "Invalid suboptions specified\n");
		return -1;
	}
	if (*value == NULL) {
		fprintf(stderr, "No value given to suboption <%s>\n",
				subopts[opt]);
		return -1;
	}
	return opt;
}

int parse_fmt(char *optarg, int *width, int *height, int *pixelformat,
                int *memtype, int *buftype)
{
	char *value, *subs;
	int fmts = 0;

	subs = optarg;
	while (*subs != '\0') {
		static const char *const subopts[] = {
			"width",
			"height",
			"pixelformat",
			"memtype",
			"buftype",
			NULL
		};

		switch (parse_subopt(&subs, subopts, &value)) {
		case 0:
			*width = strtoul(value, 0L, 0);
			fmts |= FmtWidth;
			break;
		case 1:
			*height = strtoul(value, 0L, 0);
			fmts |= FmtHeight;
			break;
		case 2:
            //*pixelformat = strtol(value, 0L, 0);
            *pixelformat = v4l2_fourcc(value[0], value[1], value[2], value[3]);
			fmts |= FmtPixelFormat;
			break;
		case 3:
            if (memtype)
                *memtype = strtol(value, 0L, 0);
			break;
		case 4:
            if (buftype)
                *buftype = strtol(value, 0L, 0);
			break;
		default:
			return 0;
		}
	}
	return fmts;
}


static int rawcap_test(struct demo_ctx *ctx);
static int half_offline_test(struct demo_ctx *ctx);
static int half_offline_test_with_mainpath_output(struct demo_ctx *ctx);
static int offline_test(struct demo_ctx *ctx);
static int offline_test_with_mainpath_output(struct demo_ctx *ctx);

int main(int argc, char **argv)
{
    int i;
    char short_options[26 * 2 * 3 + 1];
    int idx = 0;
    int ch;
    struct demo_ctx ctx = {0};

    /* for short options */
    for (i = 0; long_options[i].name; i++) {
        if (!isalpha(long_options[i].val))
            continue;
        short_options[idx++] = long_options[i].val;
        if (long_options[i].has_arg == required_argument) {
            short_options[idx++] = ':';
        } else if (long_options[i].has_arg == optional_argument) {
            short_options[idx++] = ':';
            short_options[idx++] = ':';
        }
    }

    while (1) {
        int option_index = 0;

        short_options[idx] = 0;
        ch = getopt_long(argc, argv, short_options,
                 long_options, &option_index);
        if (ch == -1)
            break;

        if (!option_index) {
            for (i = 0; long_options[i].val; i++) {
                if (long_options[i].val == ch) {
                    option_index = i;
                    break;
                }
            }
        }

        if (long_options[option_index].has_arg == optional_argument &&
            !optarg && argv[optind] && argv[optind][0] != '-')
            optarg = argv[optind++];

        switch (ch) {
        case OptHelp:
            printf("printf help here\n");
            help_info();
            return 0;
        case OptSetEntityName:
            ctx.entity = optarg;
            break;
        case OptSetIspDriver:
            ctx.isp_driver = optarg;
            break;
        case OptSetMode:
            ctx.mode = optarg;
            break;
        case OptSetVicapFmt:
            parse_fmt(optarg, &ctx.vicap_w, &ctx.vicap_h, &ctx.vicap_pixfmt,
                            &ctx.vicap_memtype, &ctx.vicap_buftype);
            break;
        case OptSetReadbackFmt:
            parse_fmt(optarg, &ctx.readback_w, &ctx.readback_h, &ctx.readback_pixfmt,
                            &ctx.readback_memtype, &ctx.readback_buftype);
            break;
        case OptSetMainpathFmt:
            parse_fmt(optarg, &ctx.mp_w, &ctx.mp_h, &ctx.mp_p,
                            NULL, &ctx.mp_buftype);
            break;
        case OptSetFrameNum:
            ctx.frame_num = strtoul(optarg, 0L, 0);
            break;
        case OptSetFrameSkip:
            ctx.frame_skip = strtoul(optarg, 0L, 0);
            break;
        case OptSetFrameDir:
            ctx.frame_dir = optarg;
            break;
        case OptSetRawFile:
            ctx.raw_file = optarg;
            break;
        case OptUseRkaiq:
            ctx.use_aiq = 1;
            break;
        case OptSetIqfileDir:
            ctx.iqpath = optarg;
            break;
        case OptSetMainpathOut:
            ctx.mainpath_out = optarg;
            break;
        default:
            help_info();
            return 1;
        }
    }

    printf("*********** rkrawstream_demo ***********\n");
    printf("sensor entity name %s\n", ctx.entity);
    printf("isp driver entity name %s\n", ctx.isp_driver);
    printf("test mode %s\n", ctx.mode);
    printf("vicap %dx%d 0x%x, mem: %d, buf %d\n", ctx.vicap_w, ctx.vicap_h,
            ctx.vicap_pixfmt, ctx.vicap_memtype, ctx.vicap_buftype);
    printf("readback %dx%d 0x%x, mem: %d, buf %d\n", ctx.readback_w, ctx.readback_h,
            ctx.readback_pixfmt, ctx.readback_memtype, ctx.readback_buftype);
    printf("frame num %d, skip %d, dir %s raw_file %s\n", ctx.frame_num, ctx.frame_skip, ctx.frame_dir, ctx.raw_file);
    printf("RKAIQ %d, iqpath %s\n", ctx.use_aiq,  ctx.iqpath);
    printf("****************************************\n");

    if (strcmp(ctx.mode, "rawcap") == 0){
        rawcap_test(&ctx);
    }
    if (strcmp(ctx.mode, "half") == 0){
        if (ctx.mainpath_out) {
            half_offline_test_with_mainpath_output(&ctx);
        } else
            half_offline_test(&ctx);
    }
    if (strcmp(ctx.mode, "offline") == 0){
        if (ctx.mainpath_out) {
            offline_test_with_mainpath_output(&ctx);
        } else {
            offline_test(&ctx);
        }
    }
    return 0;
}

#ifdef HAVE_RKAIQ

static rk_aiq_frame_info_t expinfo[2];

static int start_rkaiq(struct demo_ctx *ctx)
{
    expinfo[0].normal_exp = 0.03;
    expinfo[0].normal_gain = 2.91;
    expinfo[1].normal_exp = 0.03;
    expinfo[1].normal_gain = 2.91;

    const char* sensor_name = NULL;
    rk_aiq_control_preinit_t ctrl;
    memset(&ctrl, 0, sizeof(ctrl));
    ctrl.mode =  RK_AIQ_CONTROL_CIS_ISP_PARAM;
    if (strcmp(ctx->mode, "offline") == 0){
        printf("this is AIQ with offline test.\n");
        ctrl.mode = RK_AIQ_CONTROL_ISP_PARAM_ONLY;
        ctrl.isp_param_only_info.first_exp_info = expinfo[0];
        ctrl.isp_param_only_info.second_exp_info = expinfo[1];
    }
    if (!ctx->isp_driver) {
        printf("isp_driver input is NULL, please set isp driver name if you want run with AIQ\n");
        return 0;
    }
    sensor_name = (char *)rk_aiq_uapi2_sysctl_rawReproc_preInit(ctx->isp_driver, ctrl);
    ctx->aiqctx = rk_aiq_uapi2_sysctl_init(sensor_name, ctx->iqpath, NULL, NULL);
    //rk_aiq_uapi2_sysctl_setUserSensorFormat(ctx->aiqctx, ctx->readback_w, ctx->readback_h, 0x300e);
    rk_aiq_uapi2_sysctl_prepare(ctx->aiqctx, ctx->readback_w, ctx->readback_h, 0);
    rk_aiq_uapi2_sysctl_start(ctx->aiqctx);
    return 0;
}

static int stop_rkaiq(struct demo_ctx *ctx)
{
    if (!ctx->aiqctx) {
        return 0;
    }
    rk_aiq_uapi2_sysctl_stop(ctx->aiqctx, 0);
    rk_aiq_uapi2_sysctl_deinit(ctx->aiqctx);
    return 0;
}
#endif

static int capture_raw(rkrawstream_rkraw2_t *rkraw, struct demo_ctx *ctx)
{
    char filname[32];
    int num = ctx-> frame_cnt;
    sprintf(filname, "%s/%d.raw", ctx->frame_dir, num);
    FILE *fp = fopen(filname,"wb");
    if(fp){
        int ret = 0;
        ret += fwrite((void *)rkraw->plane[0].addr, 1, rkraw->plane[0].size, fp);
        printf("%s:save raw %s, ret %d\n", __func__, filname, ret);
        fclose(fp);
    }

    return 0;
}

static int rawcap_test_callback(rkrawstream_vicap_cb_param_t *param)
{
    struct demo_ctx *ctx = param->user_data;
    rkrawstream_rkraw2_t *rkraw2 = &param->rkraw2;

    printf("recv_frame %s %s index %d\n", ctx->entity, ctx->mode, rkraw2->plane[0].idx);
    if (ctx->frame_cnt > ctx->frame_skip);
        capture_raw(rkraw2, ctx);
    ctx->frame_cnt ++;
    return 0;
}

static int rawcap_test(struct demo_ctx *ctx)
{
    rkraw_vi_init_params_t init_p;
    rkraw_vi_prepare_params_t prepare_p;

    /* init rkraw_vi user api */
    ctx->victx = rkrawstream_uapi_init();

    // vicap only can init with key of sensor name
    init_p.sns_ent_name = ctx->entity;
    init_p.use_offline = 0;
    init_p.user_data = ctx;
    rkrawstream_vicap_init(ctx->victx, &init_p);

    prepare_p.width = ctx->vicap_w;
    prepare_p.height = ctx->vicap_h;
    prepare_p.pix_fmt = ctx->vicap_pixfmt;
    prepare_p.hdr_mode = 0;
    prepare_p.mem_mode = ctx->vicap_memtype;
    prepare_p.buf_memory_type = ctx->vicap_buftype;
    rkrawstream_vicap_prepare(ctx->victx, &prepare_p);

    rkrawstream_vicap_start(ctx->victx, rawcap_test_callback);

    while(ctx->frame_cnt < ctx->frame_num)
        usleep(10000);

    rkrawstream_vicap_stop(ctx->victx);
    /* clean up library. */
    rkrawstream_uapi_deinit(ctx->victx);
    return 0;
}

static uint32_t v4l2_format_to_media_format(uint32_t pixelformat)
{
    uint32_t pixelcode = -1;

    switch (pixelformat) {
        case V4L2_PIX_FMT_SRGGB8:
            pixelcode = MEDIA_BUS_FMT_SRGGB8_1X8;
            break;
        case V4L2_PIX_FMT_SBGGR8:
            pixelcode = MEDIA_BUS_FMT_SBGGR8_1X8;
            break;
        case V4L2_PIX_FMT_SGBRG8:
            pixelcode = MEDIA_BUS_FMT_SGBRG8_1X8;
            break;
        case V4L2_PIX_FMT_SGRBG8:
            pixelcode = MEDIA_BUS_FMT_SGRBG8_1X8;
            break;
        case V4L2_PIX_FMT_SBGGR10:
            pixelcode = MEDIA_BUS_FMT_SBGGR10_1X10;
            break;
        case V4L2_PIX_FMT_SRGGB10:
            pixelcode = MEDIA_BUS_FMT_SRGGB10_1X10;
            break;
        case V4L2_PIX_FMT_SGBRG10:
            pixelcode = MEDIA_BUS_FMT_SGBRG10_1X10;
            break;
        case V4L2_PIX_FMT_SGRBG10:
            pixelcode = MEDIA_BUS_FMT_SGRBG10_1X10;
            break;
        case V4L2_PIX_FMT_SRGGB12:
            pixelcode = MEDIA_BUS_FMT_SRGGB12_1X12;
            break;
        case V4L2_PIX_FMT_SBGGR12:
            pixelcode = MEDIA_BUS_FMT_SBGGR12_1X12;
            break;
        case V4L2_PIX_FMT_SGBRG12:
            pixelcode = MEDIA_BUS_FMT_SGBRG12_1X12;
            break;
        case V4L2_PIX_FMT_SGRBG12:
            pixelcode = MEDIA_BUS_FMT_SGRBG12_1X12;
            break;
        case V4L2_PIX_FMT_GREY:
            pixelcode = MEDIA_BUS_FMT_Y8_1X8;
            break;
        case V4L2_PIX_FMT_Y10:
            pixelcode = MEDIA_BUS_FMT_Y10_1X10;
            break;
        case V4L2_PIX_FMT_Y12:
            pixelcode = MEDIA_BUS_FMT_Y12_1X12;
            break;
        case V4L2_PIX_FMT_SBGGR16:
            pixelcode = MEDIA_BUS_FMT_SBGGR16_1X16;
            break;
        case V4L2_PIX_FMT_SGBRG16:
            pixelcode = MEDIA_BUS_FMT_SGBRG16_1X16;
            break;
        case V4L2_PIX_FMT_SGRBG16:
            pixelcode = MEDIA_BUS_FMT_SGRBG16_1X16;
            break;
        case V4L2_PIX_FMT_SRGGB16:
            pixelcode = MEDIA_BUS_FMT_SRGGB16_1X16;
            break;
        default:
            // TODO add other
            printf("%s no support pixelfmt:0x%x\n", __func__, pixelformat);
    }

    return pixelcode;
}

static int half_offline_vicap_callback(rkrawstream_vicap_cb_param_t *param)
{
    struct demo_ctx *ctx = param->user_data;
    rkraw_vi_ctx_t *victx = ctx->victx;
    rkrawstream_rkraw2_t *rkraw2 = &param->rkraw2;

    printf("capture callback %s dev %d\n", ctx->entity, param->rkraw2._rawfmt.frame_id);
    rkrawstream_vicap_buf_take(victx);
    rkrawstream_readback_set_rkraw2(victx, rkraw2);

    ctx->frame_cnt++;
    return 0;
}

static void half_offline_readback_callback(rkrawstream_readback_cb_param_t *param)
{
    struct demo_ctx *ctx = param->user_data;
    rkraw_vi_ctx_t *victx = ctx->victx;
    printf("readback callback %s dev %d\n", ctx->entity, param->dev_index);

    rkrawstream_vicap_buf_return(victx, param->dev_index);
}

static int mainpath_out_process_callback(rkrawstream_isp_cb_param_t *param)
{
    struct demo_ctx *ctx = param->user_data;
    rkraw_vi_ctx_t *victx = ctx->victx;
    printf("mainpath get frame %d\n", param->seq);
}

static int half_offline_test(struct demo_ctx *ctx)
{
    rkraw_vi_init_params_t init_p;
    rkraw_vi_prepare_params_t prepare_p;
    const char* inputparam_path = NULL;
    int ispparam_fd = -1;

    /* init rkraw_vi user api */
    ctx->victx = rkrawstream_uapi_init();

    memset(&init_p, 0, sizeof(init_p));
    // vicap only can init with key of sensor name
    init_p.sns_ent_name = ctx->entity;
    init_p.use_offline = 0;
    init_p.user_data = ctx;
    rkrawstream_vicap_init(ctx->victx, &init_p);

    // readback can init with sensor name(prior) or isp driver name
    init_p.sns_ent_name = ctx->entity;
    init_p.isp_driver_name = ctx->isp_driver;
    rkrawstream_readback_init(ctx->victx, &init_p);

    memset(&prepare_p, 0, sizeof(prepare_p));
    prepare_p.hdr_mode = 0;
    prepare_p.width = ctx->vicap_w;
    prepare_p.height = ctx->vicap_h;
    prepare_p.pix_fmt = ctx->vicap_pixfmt;
    prepare_p.mem_mode = ctx->vicap_memtype;
    prepare_p.buf_memory_type = ctx->vicap_buftype;
    rkrawstream_vicap_prepare(ctx->victx, &prepare_p);

    // readback prepare will change the pipeline, link isp to readback mode, disable vicap link
    prepare_p.width = ctx->readback_w;
    prepare_p.height = ctx->readback_h;
    prepare_p.pix_fmt = ctx->readback_pixfmt;
    prepare_p.mem_mode = ctx->readback_memtype;
    prepare_p.buf_memory_type = ctx->readback_buftype;
    rkrawstream_readback_prepare(ctx->victx, &prepare_p);

    // set isp format, scr crop and sink crop no be used if use this api set isp fmt
    uint32_t pixcode = v4l2_format_to_media_format(ctx->readback_pixfmt);
    if (pixcode != -1 && ctx->readback_w && ctx->readback_h) {
        rkrawstream_setup_pipline_fmt(ctx->victx, ctx->readback_w, ctx->readback_h, pixcode);
    }

    // rawstream run with aiq
    // aiq should prepare after rawstream prepare(set pipeline fmt already)
    // aiq should start before stream on
    if (ctx->use_aiq) {
#ifdef HAVE_RKAIQ
        start_rkaiq(ctx);
#else
        printf("**** error! arg: --rkaiq without HAVE_RKAIQ!\n");
#endif
    }

    inputparam_path = rkrawstream_get_inputparam_path_by_vi(ctx->victx);
    if (inputparam_path) {
        ispparam_fd = open(inputparam_path, O_RDWR);
    }

    if (ispparam_fd >= 0) {
        subscrible_stream_event(inputparam_path, ispparam_fd, true);
        wait_stream_event(ispparam_fd, CIFISP_V4L2_EVENT_STREAM_START, -1);
    }

    // vicap should stream on after mainpath stream on
    // if not maybe cause the first frame of mainpath output no 0
    rkrawstream_vicap_start(ctx->victx, half_offline_vicap_callback);
    rkrawstream_readback_start(ctx->victx, half_offline_readback_callback);

    if (ispparam_fd >= 0) {
        wait_stream_event(ispparam_fd, CIFISP_V4L2_EVENT_STREAM_STOP, -1);
        subscrible_stream_event(inputparam_path, ispparam_fd, false);
        close(ispparam_fd);
    }


    rkrawstream_vicap_stop(ctx->victx);
    rkrawstream_readback_stop(ctx->victx);
    /* clean up library. */
    rkrawstream_uapi_deinit(ctx->victx);

    if (ctx->use_aiq) {
#ifdef HAVE_RKAIQ
        stop_rkaiq(ctx);
#endif
    }
    return 0;
}

static int half_offline_test_with_mainpath_output(struct demo_ctx *ctx)
{
    rkraw_vi_init_params_t init_p;
    rkraw_vi_prepare_params_t prepare_p;
    const char* inputparam_path = NULL;
    int ispparam_fd = -1;

    /* init rkraw_vi user api */
    ctx->victx = rkrawstream_uapi_init();

    memset(&init_p, 0, sizeof(init_p));
    // vicap only can init with key of sensor name
    init_p.sns_ent_name = ctx->entity;
    init_p.use_offline = 0;
    init_p.user_data = ctx;
    rkrawstream_vicap_init(ctx->victx, &init_p);

    // readback can init with sensor name(prior) or isp driver name
    init_p.sns_ent_name = ctx->entity;
    init_p.isp_driver_name = ctx->isp_driver;
    rkrawstream_readback_init(ctx->victx, &init_p);

    memset(&prepare_p, 0, sizeof(prepare_p));
    prepare_p.hdr_mode = 0;
    prepare_p.width = ctx->vicap_w;
    prepare_p.height = ctx->vicap_h;
    prepare_p.pix_fmt = ctx->vicap_pixfmt;
    prepare_p.mem_mode = ctx->vicap_memtype;
    prepare_p.buf_memory_type = ctx->vicap_buftype;
    rkrawstream_vicap_prepare(ctx->victx, &prepare_p);

    // readback prepare will change the pipeline, link isp to readback mode, disable vicap link
    prepare_p.width = ctx->readback_w;
    prepare_p.height = ctx->readback_h;
    prepare_p.pix_fmt = ctx->readback_pixfmt;
    prepare_p.mem_mode = ctx->readback_memtype;
    prepare_p.buf_memory_type = ctx->readback_buftype;
    rkrawstream_readback_prepare(ctx->victx, &prepare_p);

    // set isp format, scr crop and sink crop no be used if use this api set isp fmt
    uint32_t pixcode = v4l2_format_to_media_format(ctx->readback_pixfmt);
    if (pixcode != -1 && ctx->readback_w && ctx->readback_h) {
        rkrawstream_setup_pipline_fmt(ctx->victx, ctx->readback_w, ctx->readback_h, pixcode);
    }

    // rawstream run with aiq
    // aiq should prepare after rawstream prepare(set pipeline fmt already)
    // aiq should start before stream on
    if (ctx->use_aiq) {
#ifdef HAVE_RKAIQ
        start_rkaiq(ctx);
#else
        printf("**** error! arg: --rkaiq without HAVE_RKAIQ!\n");
#endif
    }

    VI_stream_context_t *mp_ctx = VI_stream_init(ctx->mainpath_out, ctx->mp_w, ctx->mp_h, ctx->mp_p, false);
    VI_stream_start(mp_ctx);

    // vicap should stream on after mainpath stream on
    // if not maybe cause the first frame of mainpath output no 0
    rkrawstream_vicap_start(ctx->victx, half_offline_vicap_callback);
    rkrawstream_readback_start(ctx->victx, half_offline_readback_callback);

    while(ctx->frame_cnt < ctx->frame_num) {
        VI_buffer_t *buf = VI_streama_dequeue_buf(mp_ctx);
        if (!buf)
            break;
        printf("%s: dequeue buf %d\n", ctx->mainpath_out, buf->sequence);
        VI_streama_queue_buf(mp_ctx, buf);
    }
    VI_stream_stop(mp_ctx);
    VI_stream_deinit(mp_ctx);

    rkrawstream_vicap_stop(ctx->victx);
    rkrawstream_readback_stop(ctx->victx);


    /* clean up library. */
    rkrawstream_uapi_deinit(ctx->victx);

    if (ctx->use_aiq) {
#ifdef HAVE_RKAIQ
        stop_rkaiq(ctx);
#endif
    }
    return 0;
}

static int read_rkraw_file(struct demo_ctx *ctx, char *rawfile)
{
    int ret;
    char filename[1024];
    if (ctx->rkraw_fp){
        fclose(ctx->rkraw_fp);
        ctx->rkraw_fp = NULL;
    }
    if (ctx->rkraw_buffer){
        free(ctx->rkraw_buffer);
        ctx->rkraw_buffer = NULL;
    }
    if (rawfile) {
        snprintf(filename, 1024, "%s", ctx->raw_file);
    } else {
        snprintf(filename, 1024, "%s/%d.raw", ctx->frame_dir, ctx->frame_cnt);
    }
    printf("reading file %s...\n", filename);

    ctx->rkraw_fp = fopen(filename, "rb");
    if (!ctx->rkraw_fp) {
        printf("open %s failed!\n", filename);
        return -1;
    }

    fseek(ctx->rkraw_fp, 0, SEEK_END);
    ctx->rkraw_size = ftell(ctx->rkraw_fp);
    fseek(ctx->rkraw_fp, 0, SEEK_SET);

    ctx->rkraw_buffer = malloc(ctx->rkraw_size);
    if (!ctx->rkraw_buffer) {
        printf("malloc rkraw buffer %d failed!\n", ctx->rkraw_size);
        return -1;
    }
    ret = fread(ctx->rkraw_buffer, 1, ctx->rkraw_size, ctx->rkraw_fp);

    printf("read rkraw file done, size %d.\n", ctx->rkraw_size);
    return 0;
}

static int offline_test_process_one(struct demo_ctx *ctx)
{
    int ret;
    rkrawstream_rkraw2_t rkraw2;
    //ret = read_rkraw_file(ctx, NULL);

    rkraw2.plane[0].mode = 1;
    rkraw2.plane[0].size = ctx->rkraw_size;
    rkraw2.plane[0].addr = (uint64_t)ctx->rkraw_buffer;
    rkraw2._rawfmt.frame_id = ctx->frame_cnt;

#ifdef HAVE_RKAIQ
    if (ctx->use_aiq && ctx->aiqctx) {
        rk_aiq_frame_info_t nex_exp;
        nex_exp = expinfo[0];
        rk_aiq_uapi2_sysctl_rawReproc_genIspParams(ctx->aiqctx, ctx->frame_cnt, &nex_exp, 1);
    }
#endif
    rkrawstream_readback_set_rkraw2(ctx->victx, &rkraw2);
    ctx->frame_cnt++;
    return ret;
}

static void offline_readback_callback(rkrawstream_readback_cb_param_t *param)
{
    struct demo_ctx *ctx = param->user_data;
    rkraw_vi_ctx_t *victx = ctx->victx;
    printf("offline readback callback %s dev %d\n", ctx->entity, param->dev_index);
}

static int offline_test(struct demo_ctx *ctx)
{
    rkraw_vi_init_params_t init_p;
    rkraw_vi_prepare_params_t prepare_p;
    const char* inputparam_path = NULL;
    int ispparam_fd = -1;

    /* init rkraw_vi user api */
    ctx->victx = rkrawstream_uapi_init();
    memset(&init_p, 0, sizeof(init_p));
    init_p.sns_ent_name    = ctx->entity;
    init_p.isp_driver_name = ctx->isp_driver;
    init_p.user_data = ctx;
    rkrawstream_readback_init(ctx->victx, &init_p);

    memset(&prepare_p, 0, sizeof(prepare_p));
    prepare_p.hdr_mode = 0;
    prepare_p.width = ctx->readback_w;
    prepare_p.height = ctx->readback_h;
    prepare_p.pix_fmt = ctx->readback_pixfmt;
    prepare_p.mem_mode = ctx->readback_memtype;
    prepare_p.buf_memory_type = ctx->readback_buftype;
    rkrawstream_readback_prepare(ctx->victx, &prepare_p);

    uint32_t pixcode = v4l2_format_to_media_format(ctx->readback_pixfmt);
    if (pixcode != -1 && ctx->readback_w && ctx->readback_h) {
        rkrawstream_setup_pipline_fmt(ctx->victx, ctx->readback_w, ctx->readback_h, pixcode);
    }

    if (ctx->use_aiq) {
#ifdef HAVE_RKAIQ
        start_rkaiq(ctx);
#else
        printf("**** error! arg: --rkaiq without HAVE_RKAIQ!\n");
#endif
    }

    if (!ctx->mainpath_out) {
        inputparam_path = rkrawstream_get_inputparam_path_by_vi(ctx->victx);
        if (inputparam_path) {
            ispparam_fd = open(inputparam_path, O_RDWR);
        }
    }

    if (ispparam_fd >= 0) {
        subscrible_stream_event(inputparam_path, ispparam_fd, true);
        wait_stream_event(ispparam_fd, CIFISP_V4L2_EVENT_STREAM_START, -1);
    }

    rkrawstream_readback_start(ctx->victx, offline_readback_callback);

    int ret = 0;
    if (ctx->raw_file)
        ret = read_rkraw_file(ctx, ctx->raw_file);
    if (ret < 0)
        return -1;

    while(ctx->frame_cnt < ctx->frame_num){
        if( offline_test_process_one(ctx) < 0)
            break;

        usleep(100000);
    }

    if (ispparam_fd >= 0) {
        wait_stream_event(ispparam_fd, CIFISP_V4L2_EVENT_STREAM_STOP, -1);
        subscrible_stream_event(inputparam_path, ispparam_fd, false);
        close(ispparam_fd);
    }

    rkrawstream_readback_stop(ctx->victx);
    /* clean up library. */
    rkrawstream_uapi_deinit(ctx->victx);

    if (ctx->rkraw_fp){
        fclose(ctx->rkraw_fp);
        ctx->rkraw_fp = NULL;
    }
    if (ctx->rkraw_buffer){
        free(ctx->rkraw_buffer);
        ctx->rkraw_buffer = NULL;
    }
    if (ctx->use_aiq) {
#ifdef HAVE_RKAIQ
        stop_rkaiq(ctx);
#endif
    }
    return 0;
}

static int offline_test_with_mainpath_output(struct demo_ctx *ctx)
{
    rkraw_vi_init_params_t init_p;
    rkraw_vi_prepare_params_t prepare_p;
    const char* inputparam_path = NULL;
    int ispparam_fd = -1;

    /* init rkraw_vi user api */
    ctx->victx = rkrawstream_uapi_init();
    memset(&init_p, 0, sizeof(init_p));
    init_p.sns_ent_name    = ctx->entity;
    init_p.isp_driver_name = ctx->isp_driver;
    init_p.user_data = ctx;
    rkrawstream_readback_init(ctx->victx, &init_p);

    memset(&prepare_p, 0, sizeof(prepare_p));
    prepare_p.hdr_mode = 0;
    prepare_p.width = ctx->readback_w;
    prepare_p.height = ctx->readback_h;
    prepare_p.pix_fmt = ctx->readback_pixfmt;
    prepare_p.mem_mode = ctx->readback_memtype;
    prepare_p.buf_memory_type = ctx->readback_buftype;
    rkrawstream_readback_prepare(ctx->victx, &prepare_p);

    uint32_t pixcode = v4l2_format_to_media_format(ctx->readback_pixfmt);
    if (pixcode != -1 && ctx->readback_w && ctx->readback_h) {
        rkrawstream_setup_pipline_fmt(ctx->victx, ctx->readback_w, ctx->readback_h, pixcode);
    }

    if (ctx->use_aiq) {
#ifdef HAVE_RKAIQ
        start_rkaiq(ctx);
#else
        printf("**** error! arg: --rkaiq without HAVE_RKAIQ!\n");
#endif
    }

    VI_stream_context_t *mp_ctx = VI_stream_init(ctx->mainpath_out, ctx->mp_w, ctx->mp_h, ctx->mp_p, false);
    VI_stream_start(mp_ctx);

    rkrawstream_readback_start(ctx->victx, offline_readback_callback);

    int ret = 0;
    if (ctx->raw_file)
        ret = read_rkraw_file(ctx, ctx->raw_file);
    if (ret < 0)
        return -1;

    while(ctx->frame_cnt < ctx->frame_num){
        if( offline_test_process_one(ctx) < 0)
            break;

        VI_buffer_t *buf = VI_streama_dequeue_buf(mp_ctx);
        if (!buf)
            break;
        printf("%s: dequeue buf %d\n", __FUNCTION__, buf->sequence);
        VI_streama_queue_buf(mp_ctx, buf);

        usleep(100000);
    }

    VI_stream_stop(mp_ctx);
    VI_stream_deinit(mp_ctx);

    rkrawstream_readback_stop(ctx->victx);
    /* clean up library. */
    rkrawstream_uapi_deinit(ctx->victx);

    if (ctx->rkraw_fp){
        fclose(ctx->rkraw_fp);
        ctx->rkraw_fp = NULL;
    }
    if (ctx->rkraw_buffer){
        free(ctx->rkraw_buffer);
        ctx->rkraw_buffer = NULL;
    }
    if (ctx->use_aiq) {
#ifdef HAVE_RKAIQ
        stop_rkaiq(ctx);
#endif
    }
    return 0;
}
