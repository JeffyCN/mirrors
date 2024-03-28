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
#include "rkrawstream_user_api.h"

#include "rkrawstream_demo.h"

#ifdef HAVE_RKAIQ
#include "uAPI2/rk_aiq_user_api2_sysctl.h"
#endif

#define v4l2_fourcc(a, b, c, d)\
	((uint32_t)(a) | ((uint32_t)(b) << 8) | ((uint32_t)(c) << 16) | ((uint32_t)(d) << 24))

struct demo_ctx{
    rkraw_vi_ctx_t *victx;
    char *entity;
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
    {0, 0, 0, 0}
};

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
            *memtype = strtol(value, 0L, 0);
			break;
		case 4:
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
static int offline_test(struct demo_ctx *ctx);

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
            return 0;
        case OptSetEntityName:
            ctx.entity = optarg;
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

        default:
            break;
        }
    }

    printf("*********** rkrawstream_demo ***********\n");
    printf("entity name %s\n", ctx.entity);
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
        half_offline_test(&ctx);
    }
    if (strcmp(ctx.mode, "offline") == 0){
        offline_test(&ctx);
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

    if (strcmp(ctx->mode, "offline") == 0){
        printf("this is AIQ with offline test.\n");
        ctx->entity = (char *)rk_aiq_uapi2_sysctl_rawReproc_preInit("rkisp-vir0", "imx415", expinfo);
    }
    ctx->aiqctx = rk_aiq_uapi2_sysctl_init(ctx->entity, ctx->iqpath, NULL, NULL);
    //rk_aiq_uapi2_sysctl_setUserSensorFormat(ctx->aiqctx, ctx->readback_w, ctx->readback_h, 0x300e);
    rk_aiq_uapi2_sysctl_prepare(ctx->aiqctx, ctx->readback_w, ctx->readback_h, 0);
    rk_aiq_uapi2_sysctl_start(ctx->aiqctx);
    return 0;
}

static int stop_rkaiq(struct demo_ctx *ctx)
{
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

static int half_offline_vicap_callback(rkrawstream_vicap_cb_param_t *param)
{
    struct demo_ctx *ctx = param->user_data;
    rkraw_vi_ctx_t *victx = ctx->victx;
    rkrawstream_rkraw2_t *rkraw2 = &param->rkraw2;

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

static int half_offline_test(struct demo_ctx *ctx)
{
    rkraw_vi_init_params_t init_p;
    rkraw_vi_prepare_params_t prepare_p;

    if (ctx->use_aiq) {
#ifdef HAVE_RKAIQ
        start_rkaiq(ctx);
#else
        printf("**** error! arg: --rkaiq without HAVE_RKAIQ!\n");
#endif
    }
    /* init rkraw_vi user api */
    ctx->victx = rkrawstream_uapi_init();
    init_p.sns_ent_name = ctx->entity;
    init_p.use_offline = 0;
    init_p.user_data = ctx;
    rkrawstream_vicap_init(ctx->victx, &init_p);
    rkrawstream_readback_init(ctx->victx, &init_p);

    prepare_p.hdr_mode = 0;
    prepare_p.width = ctx->vicap_w;
    prepare_p.height = ctx->vicap_h;
    prepare_p.pix_fmt = ctx->vicap_pixfmt;
    prepare_p.mem_mode = ctx->vicap_memtype;
    prepare_p.buf_memory_type = ctx->vicap_buftype;
    rkrawstream_vicap_prepare(ctx->victx, &prepare_p);

    prepare_p.width = ctx->readback_w;
    prepare_p.height = ctx->readback_h;
    prepare_p.pix_fmt = ctx->readback_pixfmt;
    prepare_p.mem_mode = ctx->readback_memtype;
    prepare_p.buf_memory_type = ctx->readback_buftype;
    rkrawstream_readback_prepare(ctx->victx, &prepare_p);

    rkrawstream_vicap_start(ctx->victx, half_offline_vicap_callback);
    rkrawstream_readback_start(ctx->victx, half_offline_readback_callback);

    while(ctx->frame_cnt < ctx->frame_num)
        usleep(10000);

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
    char filename[64];
    if (ctx->rkraw_fp){
        fclose(ctx->rkraw_fp);
        ctx->rkraw_fp = NULL;
    }
    if (ctx->rkraw_buffer){
        free(ctx->rkraw_buffer);
        ctx->rkraw_buffer = NULL;
    }
    if (rawfile) {
        sprintf(filename, "%s", ctx->raw_file);
    } else {
        sprintf(filename, "%s/%d.raw", ctx->frame_dir, ctx->frame_cnt);
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

    if (ctx->use_aiq) {
#ifdef HAVE_RKAIQ
        start_rkaiq(ctx);
#else
        printf("**** error! arg: --rkaiq without HAVE_RKAIQ!\n");
#endif
    }
    /* init rkraw_vi user api */
    //ctx->victx = rkrawstream_uapi_init_offline("rkisp-vir0", "imx415");
    init_p.sns_ent_name = ctx->entity;
    init_p.use_offline = 1;
    init_p.user_data = ctx;
    rkrawstream_readback_init(ctx->victx, &init_p);

    prepare_p.hdr_mode = 0;
    prepare_p.width = ctx->readback_w;
    prepare_p.height = ctx->readback_h;
    prepare_p.pix_fmt = ctx->readback_pixfmt;
    prepare_p.mem_mode = ctx->readback_memtype;
    prepare_p.buf_memory_type = ctx->readback_buftype;
    rkrawstream_readback_prepare(ctx->victx, &prepare_p);

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
