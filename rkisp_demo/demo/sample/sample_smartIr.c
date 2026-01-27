/*
 *  Copyright (c) 2019 Rockchip Corporation
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
#include "sample_comm.h"

#ifdef SAMPLE_SMART_IR

#include <fcntl.h>
#include <linux/v4l2-subdev.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "rk_smart_ir_api.h"
#include "uAPI2/rk_aiq_user_api2_ae.h"
#include "uAPI2/rk_aiq_user_api2_awb.h"

#define RK_SMART_IR_AUTO_IRLED false

typedef struct sample_smartIr_s {
    pthread_t tid;
    bool tquit;
    bool started;
    const rk_aiq_sys_ctx_t* aiq_ctx;
    rk_smart_ir_ctx_t* ir_ctx;
    rk_smart_ir_params_t ir_configs;
    bool camGroup;
} sample_smartIr_t;

static sample_smartIr_t g_sample_smartIr_ctx;

typedef struct smartIr_dualCam_s {
    pthread_t tid;
    bool tquit;
    bool started;
    const rk_aiq_sys_ctx_t* aiq_ctx0;
    rk_smart_ir_ctx_t* ir_ctx0;
    const rk_aiq_sys_ctx_t* aiq_ctx1;
    rk_smart_ir_ctx_t* ir_ctx1;
} smartIr_dualCam_t;

static smartIr_dualCam_t g_smartIr_dualCam_ctx;

static void ir_cutter_ctrl(bool on)
{
    sample_smartIr_t* smartIr_ctx = &g_sample_smartIr_ctx;

    const char* ir_cut_v4ldev = NULL;
    ir_cut_v4ldev = "/dev/v4l-subdev3";

    struct v4l2_control control;

    control.id = V4L2_CID_BAND_STOP_FILTER;
    if (on)
        control.value = 3; // filter ir
    else
        control.value = 0; // ir in

    int _fd = open(ir_cut_v4ldev, O_RDWR | O_CLOEXEC);
    if (_fd != -1) {
        if (ioctl(_fd, VIDIOC_S_CTRL, &control) < 0) {
            printf("failed to set ircut value %d to device!\n", control.value);
        }
        close(_fd);
    }
}

#if 0
// SMARTIR_VERSION 1.0.0
static void* switch_irled_thread(void* args)
{
    sample_smartIr_t* smartIr_ctx = &g_sample_smartIr_ctx;
    rk_smart_ir_result_t ir_res;
    rk_aiq_isp_stats_t *stats_ref = NULL;
    rk_smart_ir_query_info_t query_info;
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    // auto irled
    rk_smart_ir_autoled_t auto_irled;
    memset(&auto_irled, 0, sizeof(auto_irled));
    int irled_cur_value = 100;
    if (RK_SMART_IR_AUTO_IRLED) {
        // TODO: set init irled pwm duty
        auto_irled.is_smooth_convert = false;
        auto_irled.auto_irled_val = irled_cur_value;
        auto_irled.auto_irled_min = 10;
        auto_irled.auto_irled_max = 100;
    }
    // cam group
    rk_aiq_camgroup_ctx_t* camgroup_ctx = NULL;
    rk_aiq_camgroup_camInfos_t camInfos;
    rk_aiq_sys_ctx_t* group_ctxs[RK_AIQ_CAM_GROUP_MAX_CAMS];
    rk_aiq_isp_stats_t* group_stats[RK_AIQ_CAM_GROUP_MAX_CAMS];

    while (!smartIr_ctx->tquit) {

        if (smartIr_ctx->camGroup) {
            camgroup_ctx = (rk_aiq_camgroup_ctx_t *)smartIr_ctx->aiq_ctx;
            ret = rk_aiq_uapi2_camgroup_getCamInfos(camgroup_ctx, &camInfos);
            if (ret != XCAM_RETURN_NO_ERROR) {
                printf("ret=%d, getCamInfos fail!\n", ret);
                break;
            }
            for (int i = 0; i < camInfos.valid_sns_num; i++) {
                group_ctxs[i] = rk_aiq_uapi2_camgroup_getAiqCtxBySnsNm(camgroup_ctx, camInfos.sns_ent_nm[i]);
                if (group_ctxs[i] == NULL) {
                    printf("getAiqCtxBySnsNm fail!\n");
                    break;
                }
                ret = rk_aiq_uapi2_sysctl_get3AStatsBlk(group_ctxs[i], &group_stats[i], -1);
                if (ret != XCAM_RETURN_NO_ERROR || group_stats[i] == NULL) {
                    printf("ret=%d, get3AStatsBlk fail!\n", ret);
                    break;
                }
            }
            rk_smart_ir_groupRunOnce(smartIr_ctx->ir_ctx, group_stats, camInfos.valid_sns_num, &ir_res);
            for (int i = 0; i < camInfos.valid_sns_num; i++) {
                rk_aiq_uapi2_sysctl_release3AStatsRef(group_ctxs[i], group_stats[i]);
            }

        } else {
            ret = rk_aiq_uapi2_sysctl_get3AStatsBlk(smartIr_ctx->aiq_ctx, &stats_ref, -1);
            if (ret != XCAM_RETURN_NO_ERROR || stats_ref == NULL) {
                printf("ret=%d, get3AStatsBlk fail!\n", ret);
                break;
            }
            rk_smart_ir_runOnce(smartIr_ctx->ir_ctx, stats_ref, &ir_res);
            rk_aiq_uapi2_sysctl_release3AStatsRef(smartIr_ctx->aiq_ctx, stats_ref);
        }

        if (RK_SMART_IR_AUTO_IRLED) {
            rk_smart_ir_auto_irled(smartIr_ctx->ir_ctx, &auto_irled);
            if (irled_cur_value != auto_irled.auto_irled_val) {
                irled_cur_value = auto_irled.auto_irled_val;
                // TODO: update irled pwm duty
            }
        }

        if (ir_res.status == RK_SMART_IR_STATUS_DAY) {
            // 1) ir-cutter on
            ir_cutter_ctrl(true);
            // 2) ir-led off
            // 3) switch to isp day params
            rk_aiq_uapi2_sysctl_switch_scene(smartIr_ctx->aiq_ctx, "normal", "day");


        } else if (ir_res.status == RK_SMART_IR_STATUS_NIGHT) {
            // 1) switch to isp night params
            rk_aiq_uapi2_sysctl_switch_scene(smartIr_ctx->aiq_ctx, "normal", "night");
            // 2) ir-cutter off
            ir_cutter_ctrl(false);
            // 3) ir-led on
        }

        printf("SAMPLE_SMART_IR: switch to %s\n", ir_res.status == RK_SMART_IR_STATUS_DAY ? "DAY" : "Night");

        rk_smart_ir_queryInfo(smartIr_ctx->ir_ctx, &query_info);

    }

    return NULL;
}

static void sample_smartIr_start(const void* arg)
{
    sample_smartIr_t* smartIr_ctx = &g_sample_smartIr_ctx;

    // 1) init
    smartIr_ctx->ir_ctx = rk_smart_ir_init((rk_aiq_sys_ctx_t*)arg);
    /* NOTE:
     * The API `rk_smart_ir_iniCfg` reads configuration from an INI file located at the configured path.
     * If the API `rk_smart_ir_setAttr` is called after `rk_smart_ir_iniCfg`, the configuration read from
     * the INI file might be overwritten by the new settings.
     */
    rk_smart_ir_iniCfg(smartIr_ctx->ir_ctx, "tmp/smart_ir.ini");

    // 2) load ir configs
    smartIr_ctx->ir_configs.d2n_envL_th = 0.04f;
    smartIr_ctx->ir_configs.n2d_envL_th = 0.20f;
    smartIr_ctx->ir_configs.rggain_base = 1.0f;
    smartIr_ctx->ir_configs.bggain_base = 1.0f;
    smartIr_ctx->ir_configs.awbgain_rad = 0.10f;
    smartIr_ctx->ir_configs.awbgain_dis = 0.22f;
    smartIr_ctx->ir_configs.switch_cnts_th = 100;
    rk_smart_ir_config(smartIr_ctx->ir_ctx, &smartIr_ctx->ir_configs);

    // 3)  set initial status to day
    rk_smart_ir_result_t ir_init_res;
    ir_init_res.status = RK_SMART_IR_STATUS_DAY;
    rk_smart_ir_set_status(smartIr_ctx->ir_ctx, ir_init_res);

    // 4) create thread
    smartIr_ctx->tquit = false;
    pthread_create(&smartIr_ctx->tid, NULL, switch_irled_thread, NULL);
    smartIr_ctx->started = true;
}
#endif

// SMARTIR_VERSION 2.0.0
static void* switch_thread_irled(void* args)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    sample_smartIr_t* smartIr_ctx = &g_sample_smartIr_ctx;
    rk_smart_ir_result_t result;

    while (!smartIr_ctx->tquit) {

        rk_smart_ir_run(smartIr_ctx->ir_ctx, smartIr_ctx->camGroup, &result);

        if (result.status == RK_SMART_IR_STATUS_NIGHT) {
            if (result.is_status_change) {
                printf("SAMPLE_SMART_IR: switch to Night\n");
                // 1) switch isp night params
                rk_aiq_uapi2_sysctl_switch_scene(smartIr_ctx->aiq_ctx, "normal", "night");
                // 2) ir-cutter off
                ir_cutter_ctrl(false);
            }
            if (result.is_fill_change) {
                // 3) manual/auto ir-led, set result.fill_value
                // TODO: user should define led control func here
            }

        } else if (result.status == RK_SMART_IR_STATUS_DAY && result.is_status_change) {
            printf("SAMPLE_SMART_IR: switch to Day\n");
            // 1) ir-cutter on
            ir_cutter_ctrl(true);
            // 2) ir-led off
            // TODO: user should define led control func here
            // 3) switch isp day params
            rk_aiq_uapi2_sysctl_switch_scene(smartIr_ctx->aiq_ctx, "normal", "day");
        }
    }

    return NULL;
}

static void* switch_thread_visled(void* args)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    sample_smartIr_t* smartIr_ctx = &g_sample_smartIr_ctx;
    rk_smart_ir_result_t result;

    // ir-cutter on
    ir_cutter_ctrl(true);
    // switch isp day params
    rk_aiq_uapi2_sysctl_switch_scene(smartIr_ctx->aiq_ctx, "normal", "day");

    while (!smartIr_ctx->tquit) {

        rk_smart_ir_run(smartIr_ctx->ir_ctx, smartIr_ctx->camGroup, &result);

        if (result.status == RK_SMART_IR_STATUS_NIGHT) {
            if (result.is_status_change) {
                printf("SAMPLE_SMART_IR: switch to Night\n");
            }
            if (result.is_fill_change) {
                // manual/auto ir-led, set result.fill_value
                // TODO: user should define led control func here
            }

        } else if (result.status == RK_SMART_IR_STATUS_DAY && result.is_status_change) {
            printf("SAMPLE_SMART_IR: switch to Day\n");
            // vis-led off
            // TODO: user should define led control func here
        }
    }

    return NULL;
}

static void sample_smartIr_start_irled(const void* arg)
{
    sample_smartIr_t* smartIr_ctx = &g_sample_smartIr_ctx;

    // 1) init
    smartIr_ctx->ir_ctx = rk_smart_ir_init((rk_aiq_sys_ctx_t*)arg);
    /* NOTE:
     * The API `rk_smart_ir_iniCfg` reads configuration from an INI file located at the configured path.
     * If the API `rk_smart_ir_setAttr` is called after `rk_smart_ir_iniCfg`, the configuration read from
     * the INI file might be overwritten by the new settings.
     */
    rk_smart_ir_iniCfg(smartIr_ctx->ir_ctx, "tmp/smart_ir.ini");

    // 2) load configs: auto switch, manual ir led
    rk_smart_ir_attr_t attr;
    //memset(&attr, 0, sizeof(attr));
    rk_smart_ir_getAttr(smartIr_ctx->ir_ctx, &attr);
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
    rk_smart_ir_setAttr(smartIr_ctx->ir_ctx, &attr);

    // 3) create thread
    smartIr_ctx->tquit = false;
    pthread_create(&smartIr_ctx->tid, NULL, switch_thread_irled, NULL);
    smartIr_ctx->started = true;
}

static void sample_smartIr_start_visled(const void* arg)
{
    sample_smartIr_t* smartIr_ctx = &g_sample_smartIr_ctx;

    // 1) init
    smartIr_ctx->ir_ctx = rk_smart_ir_init((rk_aiq_sys_ctx_t*)arg);
    /* NOTE:
     * The API `rk_smart_ir_iniCfg` reads configuration from an INI file located at the configured path.
     * If the API `rk_smart_ir_setAttr` is called after `rk_smart_ir_iniCfg`, the configuration read from
     * the INI file might be overwritten by the new settings.
     */
    rk_smart_ir_iniCfg(smartIr_ctx->ir_ctx, "tmp/smart_ir.ini");

    // 2) load configs: auto switch, auto vis led
    rk_smart_ir_attr_t attr;
    //memset(&attr, 0, sizeof(attr));
    rk_smart_ir_getAttr(smartIr_ctx->ir_ctx, &attr);
    attr.init_status = RK_SMART_IR_STATUS_DAY;
    attr.switch_mode = RK_SMART_IR_SWITCH_MODE_AUTO;
    attr.light_mode = RK_SMART_IR_LIGHT_MODE_AUTO;
    attr.light_type = RK_SMART_IR_LIGHT_TYPE_VIS;
    attr.light_value = 100;
    attr.params.d2n_envL_th = 0.04f;
    attr.params.n2d_envL_th = 0.60f;
    attr.params.rggain_base = 0.0f;
    attr.params.bggain_base = 0.0f;
    attr.params.awbgain_rad = 0.0f;
    attr.params.awbgain_dis = 0.0f;
    attr.params.switch_cnts_th = 50;
    attr.en_auto_n2dth = true;
    rk_smart_ir_setAttr(smartIr_ctx->ir_ctx, &attr);

    // 3) create thread
    smartIr_ctx->tquit = false;
    pthread_create(&smartIr_ctx->tid, NULL, switch_thread_visled, NULL);
    smartIr_ctx->started = true;
}

static void sample_smartIr_stop(const void* arg)
{
    sample_smartIr_t* smartIr_ctx = &g_sample_smartIr_ctx;

    if (smartIr_ctx->started) {
        smartIr_ctx->tquit = true;
        pthread_join(smartIr_ctx->tid, NULL);
    }
    smartIr_ctx->started = false;

    if (smartIr_ctx->ir_ctx) {
        rk_smart_ir_deInit(smartIr_ctx->ir_ctx);
        smartIr_ctx->ir_ctx = NULL;
    }

    printf("stop smartIr\n");
}

static void sample_smartIr_calib(const void* arg)
{
    g_sample_smartIr_ctx.ir_ctx = rk_smart_ir_init((rk_aiq_sys_ctx_t*)arg);
    rk_smart_ir_calib_t calib_cfg;
    calib_cfg.calib_en = true;

    /* calib d2n_envL_th
     * 1. Switch to ISP day mode
     * 2. Enable IR-cutter, disable IR-LED
     * 3. Adjust brightness threshold for day-to-night switch
     */
    calib_cfg.calib_mode = RK_SMART_IR_CALIB_MODE_D2N;
    calib_cfg.calib_params.d2n_envL_th = 0.04f; //need cfg, init d2n_envL_th param
    rk_smart_ir_calib(g_sample_smartIr_ctx.ir_ctx, &calib_cfg);
    printf("calib result: d2n_envL_th = %f\n", calib_cfg.calib_result.d2n_envL_th);

    /* calib n2d_envL_th
     * 1. Switch to ISP night mode
     * 2. Disable IR-cutter, enable IR-LED
     * 3. Adjust brightness threshold for night-to-day switch
     */
    calib_cfg.calib_mode = RK_SMART_IR_CALIB_MODE_N2D;
    calib_cfg.calib_params.n2d_envL_th = 0.2f; //need cfg, init n2d_envL_th param
    rk_smart_ir_calib(g_sample_smartIr_ctx.ir_ctx, &calib_cfg);
    printf("calib result: n2d_envL_th = %f\n", calib_cfg.calib_result.n2d_envL_th);

    /* calib awbgain base
     * 1. Switch to ISP night mode
     * 2. Disable IR-cutter, enable IR-LED
     * 3. Ensure no visible light
     */
    calib_cfg.calib_mode = RK_SMART_IR_CALIB_MODE_BASE;
    rk_smart_ir_calib(g_sample_smartIr_ctx.ir_ctx, &calib_cfg);
    printf("calib result: awbgain_base = %f, %f\n", calib_cfg.calib_result.rggain_base,
           calib_cfg.calib_result.bggain_base);

    /* tune awbgain dis
     * 1. Switch to ISP night mode
     * 2. Disable IR-cutter, enable IR-LED
     * 3. Test various scenarios and get the maximum value
     */
    calib_cfg.calib_mode = RK_SMART_IR_CALIB_MODE_DIS;
    calib_cfg.calib_params.rggain_base = 1.00f; //need cfg, init rggain_base param
    calib_cfg.calib_params.bggain_base = 1.00f; //need cfg, init bggain_base param
    calib_cfg.calib_params.awbgain_rad = 0.10f; //need cfg, init awbgain_rad param
    rk_smart_ir_calib(g_sample_smartIr_ctx.ir_ctx, &calib_cfg);
    printf("calib result: awbgain_dis = %f\n", calib_cfg.calib_result.awbgain_dis);

    if (g_sample_smartIr_ctx.ir_ctx) {
        rk_smart_ir_deInit(g_sample_smartIr_ctx.ir_ctx);
        g_sample_smartIr_ctx.ir_ctx = NULL;
    }
}

void sample_smartIr_cb(rk_smart_ir_result_t result)
{
    if (result.status == RK_SMART_IR_STATUS_NIGHT) {
        if (result.is_status_change) {
            printf("SAMPLE_SMART_IR: switch to Night\n");
            // 1) switch isp night params
            rk_aiq_uapi2_sysctl_switch_scene(g_sample_smartIr_ctx.aiq_ctx, "normal", "night");
            // 2) ir-cutter off
            ir_cutter_ctrl(false);
        }
        if (result.is_fill_change) {
            // 3) manual/auto ir-led, set result.fill_value
            // TODO: user should define led control func here
        }

    } else if (result.status == RK_SMART_IR_STATUS_DAY && result.is_status_change) {
        printf("SAMPLE_SMART_IR: switch to Day\n");
        // 1) ir-cutter on
        ir_cutter_ctrl(true);
        // 2) ir-led off
        // TODO: user should define led control func here
        // 3) switch isp day params
        rk_aiq_uapi2_sysctl_switch_scene(g_sample_smartIr_ctx.aiq_ctx, "normal", "day");
    }
}

static void sample_smartIr_start(const void* arg)
{
    // 1) init
    g_sample_smartIr_ctx.ir_ctx = rk_smart_ir_init((rk_aiq_sys_ctx_t*)arg);
    /* NOTE:
     * The API `rk_smart_ir_iniCfg` reads configuration from an INI file located at the configured path.
     * If the API `rk_smart_ir_setAttr` is called after `rk_smart_ir_iniCfg`, the configuration read from
     * the INI file might be overwritten by the new settings.
     */
    rk_smart_ir_iniCfg(g_sample_smartIr_ctx.ir_ctx, "tmp/smart_ir.ini");

    // 2) load configs: auto switch, manual ir led
    rk_smart_ir_attr_t attr;
    //memset(&attr, 0, sizeof(attr));
    rk_smart_ir_getAttr(g_sample_smartIr_ctx.ir_ctx, &attr);
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
    rk_smart_ir_setAttr(g_sample_smartIr_ctx.ir_ctx, &attr);

    rk_smart_ir_runCb(g_sample_smartIr_ctx.ir_ctx, g_sample_smartIr_ctx.camGroup, sample_smartIr_cb);
}

static void* smartIr_dualCam_thread(void* args)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    smartIr_dualCam_t* smartIr_ctx = &g_smartIr_dualCam_ctx;
    rk_smart_ir_result_t result;

    // ir-cutter on
    ir_cutter_ctrl(true);
    // switch isp day params
    rk_aiq_uapi2_sysctl_switch_scene(smartIr_ctx->aiq_ctx0, "normal", "day");
    rk_aiq_uapi2_sysctl_switch_scene(smartIr_ctx->aiq_ctx1, "normal", "day");

    while (!smartIr_ctx->tquit) {

        rk_smart_ir_dualRun(smartIr_ctx->ir_ctx0, smartIr_ctx->ir_ctx1, &result);

        if (result.status == RK_SMART_IR_STATUS_NIGHT && result.is_status_change) {
            printf("SAMPLE_SMART_IR: switch to Night\n");
            // 1) switch isp night params
            rk_aiq_uapi2_sysctl_switch_scene(smartIr_ctx->aiq_ctx0, "normal", "night");
            rk_aiq_uapi2_sysctl_switch_scene(smartIr_ctx->aiq_ctx1, "normal", "night");
            // 2) ir-cutter off
            // TODO: user should define ircut control func here
            // 3) ir-led on
            // TODO: user should define led control func here

        } else if (result.status == RK_SMART_IR_STATUS_DAY && result.is_status_change) {
            printf("SAMPLE_SMART_IR: switch to Day\n");
            // 1) ir-cutter on
            // TODO: user should define ircut control func here
            // 2) ir-led off
            // TODO: user should define led control func here
            // 3) switch isp day params
            rk_aiq_uapi2_sysctl_switch_scene(smartIr_ctx->aiq_ctx0, "normal", "day");
            rk_aiq_uapi2_sysctl_switch_scene(smartIr_ctx->aiq_ctx1, "normal", "day");
        }

    }

    return NULL;
}

static void smartIr_dualCam_start(const void* arg)
{
    smartIr_dualCam_t* smartIr_ctx = &g_smartIr_dualCam_ctx;

    // 1) cam0 cfg
    smartIr_ctx->aiq_ctx0 = (rk_aiq_sys_ctx_t*)arg; //need to modify
    smartIr_ctx->ir_ctx0 = rk_smart_ir_init(smartIr_ctx->aiq_ctx0);

    rk_smart_ir_attr_t attr0;
    rk_smart_ir_getAttr(smartIr_ctx->ir_ctx0, &attr0);
    attr0.init_status = RK_SMART_IR_STATUS_DAY;
    attr0.switch_mode = RK_SMART_IR_SWITCH_MODE_AUTO;
    attr0.light_mode = RK_SMART_IR_LIGHT_MODE_MANUAL;
    attr0.light_type = RK_SMART_IR_LIGHT_TYPE_IR;
    attr0.light_value = 100;
    attr0.params.d2n_envL_th = 0.04f; //need to tune
    attr0.params.n2d_envL_th = 0.20f; //need to tune
    attr0.params.rggain_base = 1.00f; //need to tune
    attr0.params.bggain_base = 1.00f; //need to tune
    attr0.params.awbgain_rad = 0.10f; //need to tune
    attr0.params.awbgain_dis = 0.20f; //need to tune
    attr0.params.switch_cnts_th = 50;
    rk_smart_ir_setAttr(smartIr_ctx->ir_ctx0, &attr0);

    // 2) cam1 cfg
    smartIr_ctx->aiq_ctx1 = (rk_aiq_sys_ctx_t*)arg; //need to modify
    smartIr_ctx->ir_ctx1 = rk_smart_ir_init(smartIr_ctx->aiq_ctx1);

    rk_smart_ir_attr_t attr1;
    rk_smart_ir_getAttr(smartIr_ctx->ir_ctx1, &attr1);
    attr1.init_status = RK_SMART_IR_STATUS_DAY;
    attr1.switch_mode = RK_SMART_IR_SWITCH_MODE_AUTO;
    attr1.light_mode = RK_SMART_IR_LIGHT_MODE_MANUAL;
    attr1.light_type = RK_SMART_IR_LIGHT_TYPE_IR;
    attr1.light_value = 100;
    attr1.params.d2n_envL_th = 0.04f; //need to tune
    attr1.params.n2d_envL_th = 0.20f; //need to tune
    attr1.params.rggain_base = 1.00f; //need to tune
    attr1.params.bggain_base = 1.00f; //need to tune
    attr1.params.awbgain_rad = 0.10f; //need to tune
    attr1.params.awbgain_dis = 0.20f; //need to tune
    attr1.params.switch_cnts_th = 50;
    rk_smart_ir_setAttr(smartIr_ctx->ir_ctx1, &attr1);

    // 3) create thread
    smartIr_ctx->tquit = false;
    pthread_create(&smartIr_ctx->tid, NULL, smartIr_dualCam_thread, NULL);
    smartIr_ctx->started = true;
}

static void smartIr_dualCam_stop(const void* arg)
{
    smartIr_dualCam_t* smartIr_ctx = &g_smartIr_dualCam_ctx;

    if (smartIr_ctx->started) {
        smartIr_ctx->tquit = true;
        pthread_join(smartIr_ctx->tid, NULL);
    }
    smartIr_ctx->started = false;

    if (smartIr_ctx->ir_ctx0) {
        rk_smart_ir_deInit(smartIr_ctx->ir_ctx0);
        smartIr_ctx->ir_ctx0 = NULL;
    }

    if (smartIr_ctx->ir_ctx1) {
        rk_smart_ir_deInit(smartIr_ctx->ir_ctx1);
        smartIr_ctx->ir_ctx1 = NULL;
    }

    printf("stop smartIr\n");
}

static void smartIr_test_switch(const void* arg)
{
    sample_smartIr_start(arg);

    rk_smart_ir_attr_t attr;
    //memset(&attr, 0, sizeof(attr));
    rk_smart_ir_getAttr(g_sample_smartIr_ctx.ir_ctx, &attr);
    attr.switch_test.switch_en = true;
    attr.switch_test.switch_interval = 100;
    rk_smart_ir_setAttr(g_sample_smartIr_ctx.ir_ctx, &attr);
}

static XCamReturn sample_getIspStats(const rk_aiq_sys_ctx_t* ctx)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    uint32_t R_blk[225] = { 0 }, G_blk[225] = { 0 }, B_blk[225] = { 0 };
    uint32_t WpNo_blk[225] = { 0 };
    float rgain = 0.0f, bgain = 0.0f;
    int cnt = 0;
    uint16_t Y_blk[225] = { 0 };

#ifdef USE_NEWSTRUCT
    // 1) get isp stats
    rk_aiq_isp_statistics_t isp_stats;
    isp_stats.bValid_aec_stats = false;
    isp_stats.bValid_awb_stats = false;
    ret = rk_aiq_uapi2_stats_getIspStats(ctx, &isp_stats, 1000);
    if (ret == XCAM_RETURN_NO_ERROR && isp_stats.bValid_aec_stats && isp_stats.bValid_awb_stats) {
        // do nothing
    } else {
        printf("ret=%d, stats_id=%d, valid=%d,%d, getIspStats fail!\n", ret, isp_stats.frame_id,
               isp_stats.bValid_aec_stats, isp_stats.bValid_awb_stats);
        return ret;
    }

    // 2) calc awb gain
    for (int i = 0; i < RAWAEBIG_WIN_NUM; i++) {
        R_blk[i] = isp_stats.awb_stats.com.pixEngine.zonePix[i].hw_awbCfg_rSum_val;
        G_blk[i] = isp_stats.awb_stats.com.pixEngine.zonePix[i].hw_awbCfg_gSum_val;
        B_blk[i] = isp_stats.awb_stats.com.pixEngine.zonePix[i].hw_awbCfg_bSum_val;
        WpNo_blk[i] = isp_stats.awb_stats.com.pixEngine.zonePix[i].hw_awbCfg_statsPix_count;
        if (G_blk[i] > 0) {
            rgain = rgain + (float)R_blk[i] / (float)G_blk[i];
            bgain = bgain + (float)B_blk[i] / (float)G_blk[i];
            cnt++;
        }
    }
    if (cnt > 0) {
        rgain = rgain / cnt;
        bgain = bgain / cnt;
    }

    // 3) get ae stats
    Uapi_RkAeStats_t AeHwStats;
    rk_aiq_user_api2_ae_getRKAeStats(ctx, &AeHwStats);
    memcpy(Y_blk, AeHwStats.chn[0].rawae_big.channely_xy, sizeof(Y_blk));

    // 4) querry ae/awb info
    rk_aiq_wb_querry_info_t wbInfo;
    ae_api_queryInfo_t expInfo;
    rk_aiq_user_api2_awb_QueryWBInfo(ctx, &wbInfo);
    rk_aiq_user_api2_ae_queryExpResInfo(ctx, &expInfo);

    printf("frame_id=%d,cnt=%d,rgain=%f,bgain=%f,wbrgain=%f,wbbgain=%f\nae_converged=%d,meanluma=%f,exp=[%f,%f,%f]\n",
           isp_stats.frame_id, cnt, rgain, bgain, wbInfo.stat_gain_blk.rgain, wbInfo.stat_gain_blk.bgain,
           expInfo.isConverged, expInfo.linExpInfo.meanLuma, expInfo.linExpInfo.expParam.integration_time,
           expInfo.linExpInfo.expParam.analog_gain, expInfo.linExpInfo.expParam.isp_dgain);

#else
    // 1) get isp stats
    rk_aiq_isp_stats_t *stats_ref = NULL;
    ret = rk_aiq_uapi2_sysctl_get3AStatsBlk(ctx, &stats_ref, 1000);
    if (ret != XCAM_RETURN_NO_ERROR || stats_ref == NULL) {
        printf("ret=%d, get3AStatsBlk fail!\n", ret);
        rk_aiq_uapi2_sysctl_release3AStatsRef(ctx, stats_ref);
        return ret;
    }

    // 2) calc awb gain
#if defined(ISP_HW_V39) || defined(ISP_HW_V33) || defined(ISP_HW_V35)
    for (int i = 0; i < RAWAEBIG_WIN_NUM; i++) {
        R_blk[i] = (float)stats_ref->awb_stats_v39.com.pixEngine.zonePix[i].hw_awbCfg_rSum_val;
        G_blk[i] = (float)stats_ref->awb_stats_v39.com.pixEngine.zonePix[i].hw_awbCfg_gSum_val;
        B_blk[i] = (float)stats_ref->awb_stats_v39.com.pixEngine.zonePix[i].hw_awbCfg_bSum_val;
        WpNo_blk[i] = (float)stats_ref->awb_stats_v39.com.pixEngine.zonePix[i].hw_awbCfg_statsPix_count;
    }
#endif
#if defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
    for (int i = 0; i < RAWAEBIG_WIN_NUM; i++) {
        R_blk[i] = (float)stats_ref->awb_stats_v32.blockResult[i].Rvalue;
        G_blk[i] = (float)stats_ref->awb_stats_v32.blockResult[i].Gvalue;
        B_blk[i] = (float)stats_ref->awb_stats_v32.blockResult[i].Bvalue;
        WpNo_blk[i] = (float)stats_ref->awb_stats_v32.blockResult[i].WpNo;
    }
#endif
#if defined(ISP_HW_V30)
    for (int i = 0; i < RAWAEBIG_WIN_NUM; i++) {
        R_blk[i] = (float)stats_ref->awb_stats_v3x.blockResult[i].Rvalue;
        G_blk[i] = (float)stats_ref->awb_stats_v3x.blockResult[i].Gvalue;
        B_blk[i] = (float)stats_ref->awb_stats_v3x.blockResult[i].Bvalue;
        WpNo_blk[i] = (float)stats_ref->awb_stats_v3x.blockResult[i].WpNo;
    }
#endif
#if defined(ISP_HW_V21)
    for (int i = 0; i < RAWAEBIG_WIN_NUM; i++) {
        R_blk[i] = (float)stats_ref->awb_stats_v21.blockResult[i].Rvalue;
        G_blk[i] = (float)stats_ref->awb_stats_v21.blockResult[i].Gvalue;
        B_blk[i] = (float)stats_ref->awb_stats_v21.blockResult[i].Bvalue;
        WpNo_blk[i] = (float)stats_ref->awb_stats_v21.blockResult[i].WpNo;
    }
#endif
    for (int i = 0; i < RAWAEBIG_WIN_NUM; i++) {
        if (G_blk[i] > 0) {
            rgain = rgain + (float)R_blk[i] / (float)G_blk[i];
            bgain = bgain + (float)B_blk[i] / (float)G_blk[i];
            cnt++;
        }
    }
    if (cnt > 0) {
        rgain = rgain / cnt;
        bgain = bgain / cnt;
    }

    // 3) get ae stats
    memcpy(Y_blk, stats_ref->aec_stats.ae_data.chn[0].rawae_big.channely_xy, sizeof(Y_blk));
    rk_aiq_uapi2_sysctl_release3AStatsRef(ctx, stats_ref);

    // 4) querry ae/awb info
    rk_aiq_wb_querry_info_t wb_info;
    Uapi_ExpQueryInfo_t exp_info;
    rk_aiq_user_api2_awb_QueryWBInfo(ctx, &wb_info);
    rk_aiq_user_api2_ae_queryExpResInfo(ctx, &exp_info);

    printf("frame_id=%d,cnt=%d,rgain=%f,bgain=%f,wbrgain=%f,wbbgain=%f\nae_converged=%d,meanluma=%f,exp=[%f,%f,%f]\n",
           stats_ref->frame_id, cnt, rgain, bgain, wb_info.stat_gain_blk.rgain, wb_info.stat_gain_blk.bgain,
           exp_info.IsConverged, exp_info.LinAeInfo.MeanLuma, exp_info.LinAeInfo.LinearExp.integration_time,
           exp_info.LinAeInfo.LinearExp.analog_gain, exp_info.LinAeInfo.LinearExp.isp_dgain);
#endif

    printf("================================= Source statistics log =================================\n");
    printf("block luma:\n");
    for (int i = 0; i < 15; i++) {
        printf("%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
               Y_blk[15 * i + 0], Y_blk[15 * i + 1], Y_blk[15 * i + 2], Y_blk[15 * i + 3], Y_blk[15 * i + 4],
               Y_blk[15 * i + 5], Y_blk[15 * i + 6], Y_blk[15 * i + 7], Y_blk[15 * i + 8], Y_blk[15 * i + 9],
               Y_blk[15 * i + 10], Y_blk[15 * i + 11], Y_blk[15 * i + 12], Y_blk[15 * i + 13], Y_blk[15 * i + 14]);
    }

    printf("block wbR:\n");
    for (int i = 0; i < 15; i++) {
        printf("%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
               R_blk[15 * i + 0], R_blk[15 * i + 1], R_blk[15 * i + 2], R_blk[15 * i + 3], R_blk[15 * i + 4],
               R_blk[15 * i + 5], R_blk[15 * i + 6], R_blk[15 * i + 7], R_blk[15 * i + 8], R_blk[15 * i + 9],
               R_blk[15 * i + 10], R_blk[15 * i + 11], R_blk[15 * i + 12], R_blk[15 * i + 13], R_blk[15 * i + 14]);
    }

    printf("block wbG:\n");
    for (int i = 0; i < 15; i++) {
        printf("%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
               G_blk[15 * i + 0], G_blk[15 * i + 1], G_blk[15 * i + 2], G_blk[15 * i + 3], G_blk[15 * i + 4],
               G_blk[15 * i + 5], G_blk[15 * i + 6], G_blk[15 * i + 7], G_blk[15 * i + 8], G_blk[15 * i + 9],
               G_blk[15 * i + 10], G_blk[15 * i + 11], G_blk[15 * i + 12], G_blk[15 * i + 13], G_blk[15 * i + 14]);
    }

    printf("block wbB:\n");
    for (int i = 0; i < 15; i++) {
        printf("%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
               B_blk[15 * i + 0], B_blk[15 * i + 1], B_blk[15 * i + 2], B_blk[15 * i + 3], B_blk[15 * i + 4],
               B_blk[15 * i + 5], B_blk[15 * i + 6], B_blk[15 * i + 7], B_blk[15 * i + 8], B_blk[15 * i + 9],
               B_blk[15 * i + 10], B_blk[15 * i + 11], B_blk[15 * i + 12], B_blk[15 * i + 13], B_blk[15 * i + 14]);
    }

    printf("block wpNo: %d,%d\n", WpNo_blk[0], WpNo_blk[224]);
    printf("========================================== end ==========================================\n\n");

    return ret;
}

static void sample_smartIr_usage()
{
    printf("Usage : \n");
    printf("  SmartIr API: \n");
    printf("\t i) SmartIr:         Start smartIr irled test.\n");
    printf("\t v) SmartIr:         Start smartIr visled test.\n");
    printf("\t s) SmartIr:         Start smartIr (callback).\n");
    printf("\t e) SmartIr:         Stop smartIr test.\n");
    printf("\t a) SmartIr:         Start duamcam smartIr.\n");
    printf("\t b) SmartIr:         Stop duamcam smartIr.\n");
    printf("\t c) SmartIr:         Calib params.\n");
    printf("\t t) SmartIr:         Test switch smartIr.\n");
    printf("\t g) SmartIr:         Get isp stats.\n");

    printf("\n");
    printf("\t h) SmartIr:         help.\n");
    printf("\t q) SmartIr:         return to main sample screen.\n");
    printf("\n");
    printf("\t please press the key: ");

    return;
}

void sample_print_smartIr_info(const void* arg)
{
    printf("enter SmartIr modult test!\n");
}

XCamReturn sample_smartIr_module(const void* arg)
{
    int key = -1;
    CLEAR();

    const demo_context_t *demo_ctx = (demo_context_t *)arg;
    const rk_aiq_sys_ctx_t* ctx;
    if (demo_ctx->camGroup) {
        ctx = (rk_aiq_sys_ctx_t*)(demo_ctx->camgroup_ctx);
    } else {
        ctx = (rk_aiq_sys_ctx_t*)(demo_ctx->aiq_ctx);
    }

    if (ctx == NULL) {
        ERR("%s, ctx is nullptr\n", __FUNCTION__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    sample_smartIr_usage();

    g_sample_smartIr_ctx.tquit = false;
    g_sample_smartIr_ctx.started = false;
    g_sample_smartIr_ctx.aiq_ctx = ctx;
    g_sample_smartIr_ctx.ir_ctx = NULL;
    g_sample_smartIr_ctx.camGroup = demo_ctx->camGroup;

    do {
        key = getchar();
        while (key == '\n' || key == '\r')
            key = getchar();
        printf("\n");

        switch (key) {
        case 'h':
            CLEAR();
            sample_smartIr_usage();
            break;
        case 'i':
            sample_smartIr_start_irled(ctx);
            break;
        case 'v':
            sample_smartIr_start_visled(ctx);
            break;
        case 's':
            sample_smartIr_start(ctx);
            break;
        case 'e':
            sample_smartIr_stop(ctx);
            break;
        case 'a':
            smartIr_dualCam_start(ctx);
            break;
        case 'b':
            smartIr_dualCam_stop(ctx);
            break;
        case 'c':
            sample_smartIr_calib(ctx);
            break;
        case 't':
            smartIr_test_switch(ctx);
            break;
        case 'g':
            sample_getIspStats(ctx);
            break;
        default:
            break;
        }
    } while (key != 'q' && key != 'Q');

    sample_smartIr_stop(ctx);

    return XCAM_RETURN_NO_ERROR;
}

#else
void sample_print_smartIr_info(const void* arg)
{
    printf("enter SmartIr modult test!\n");
}

XCamReturn sample_smartIr_module(const void* arg)
{
    printf("Not enabled! Add option SAMPLE_SMART_IR in makefile \n");
    return XCAM_RETURN_NO_ERROR;
}
#endif
