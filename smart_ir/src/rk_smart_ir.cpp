/*
 *  Copyright (c) 2022 Rockchip Corporation
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

#include <stdio.h>
#include "math.h"
#include "rk_smart_ir_api.h"
#include "uAPI2/rk_aiq_user_api2_ae.h"
#include "uAPI2/rk_aiq_user_api2_awb.h"
#include "uAPI2/rk_aiq_user_api2_sysctl.h"

RKAIQ_BEGIN_DECLARE

#ifndef Android
#ifndef NDEBUG
#define SMARTIR_LOG printf
#else //NDEBUG
#define SMARTIR_LOG
#endif
#else //Android
#define SMARTIR_LOG LOGD
#endif

#define SMARTIR_LOG_BITS (1ULL << 35) //reuse ASD
static bool g_smart_ir_log = false;

bool smart_ir_get_env_value(const char* variable, unsigned long long* value)
{
    if (!variable || !value) {

        return false;
    }

    char* valueStr = getenv(variable);
    if (valueStr) {
        *value = strtoull(valueStr, nullptr, 16);

        return true;
    }

    return false;
}

int smart_ir_get_log_level()
{
    unsigned long long cam_engine_log_level = 0xff0;

#ifdef ANDROID_OS
    char property_value[PROPERTY_VALUE_MAX] = {0};

    property_get("persist.vendor.rkisp.log", property_value, "0");
    cam_engine_log_level = strtoull(property_value, nullptr, 16);

#else
    smart_ir_get_env_value("persist_camera_engine_log",
                              &cam_engine_log_level);
#endif
    printf("rkaiq log level %llx\n", cam_engine_log_level);

    if (cam_engine_log_level & SMARTIR_LOG_BITS) {
        g_smart_ir_log = true;
    }

    return 0;
}

typedef struct rk_smart_ir_ctx_s {
    const rk_aiq_sys_ctx_t* aiq_ctx;
    int cur_working_mode;
    rk_smart_ir_params_t ir_configs;
    RK_SMART_IR_STATUS_t state;
    uint16_t switch_cnts;
} rk_smart_ir_ctx_t;

rk_smart_ir_ctx_t*
rk_smart_ir_init(const rk_aiq_sys_ctx_t* ctx)
{
    SMARTIR_LOG("%s: (enter)\n", __FUNCTION__ );
    rk_smart_ir_ctx_t* ir_ctx = (rk_smart_ir_ctx_t*)malloc(sizeof(rk_smart_ir_ctx_t));
    if (ir_ctx) {
        memset(ir_ctx, 0, sizeof(rk_smart_ir_ctx_t));
        ir_ctx->aiq_ctx = ctx;
        ir_ctx->ir_configs.d2n_envL_th = 0.04f;
        ir_ctx->ir_configs.n2d_envL_th = 0.2f;
        ir_ctx->ir_configs.rggain_base = 1.0f;
        ir_ctx->ir_configs.bggain_base = 1.0f;
        ir_ctx->ir_configs.awbgain_rad = 0.1f;
        ir_ctx->ir_configs.awbgain_dis = 0.3f;
        ir_ctx->cur_working_mode = -1;
    }
    smart_ir_get_log_level();

    SMARTIR_LOG("%s: (exit)\n", __FUNCTION__ );
    return ir_ctx;
}

XCamReturn
rk_smart_ir_deInit(const rk_smart_ir_ctx_t* ctx)
{
    SMARTIR_LOG("%s: (enter)\n", __FUNCTION__ );
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (ctx)
        free((void*)ctx);

    SMARTIR_LOG("%s: (exit)\n", __FUNCTION__ );

    return ret;
}

XCamReturn
rk_smart_ir_config(rk_smart_ir_ctx_t* ctx, rk_smart_ir_params_t* config)
{
    SMARTIR_LOG("%s: (enter)\n", __FUNCTION__ );

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ctx->ir_configs = *config;

    ctx->switch_cnts = 0;
    ctx->cur_working_mode = -1;

    if (g_smart_ir_log)
        printf("[SMARTIR] ir configs: \n"
                "\t d2n_envL_th: %0.3f, n2d_envL_th: %0.3f\n"
                "\t rggain_base: %0.3f, bggain_base: %0.3f\n"
                "\t awbgain_rad: %0.3f, awbgain_dis: %0.3f\n"
                "\t switch_cnts_th: %d",
                config->d2n_envL_th, config->n2d_envL_th,
                config->rggain_base, config->bggain_base,
                config->awbgain_rad, config->awbgain_dis,
                config->switch_cnts_th);
    SMARTIR_LOG("%s: (exit)\n", __FUNCTION__ );

    return ret;
}

XCamReturn
rk_smart_ir_set_status(rk_smart_ir_ctx_t* ctx, rk_smart_ir_result_t result)
{
    SMARTIR_LOG("%s: (enter)\n", __FUNCTION__ );

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ctx->state = result.status;

    SMARTIR_LOG("%s: (exit)\n", __FUNCTION__ );

    return ret;
}

enum RK_SMART_IR_ALGO_RESULT_e {
    RK_SMART_IR_ALGO_RESULT_KEEP = 0,
    RK_SMART_IR_ALGO_RESULT_DAY,
    RK_SMART_IR_ALGO_RESULT_NIGHT,
};

#define RK_SMART_IR_BLK_NUM_D2N_THRES (0.8 * RK_AIQ_AWB_GRID_NUM_TOTAL)
#define RK_SMART_IR_BLK_NUM_N2D_THRES (0.6 * RK_AIQ_AWB_GRID_NUM_TOTAL)

static int
_rk_smart_ir_day_or_night(rk_smart_ir_ctx_t* ctx, float envL, float distance)
{
    int ret = RK_SMART_IR_ALGO_RESULT_KEEP;
    rk_smart_ir_params_t* ir_configs = &ctx->ir_configs;

    if (envL < ctx->ir_configs.d2n_envL_th) {
        ret = RK_SMART_IR_ALGO_RESULT_NIGHT;
    } else if (envL > ctx->ir_configs.n2d_envL_th) {
        if (ctx->state == RK_SMART_IR_STATUS_NIGHT) {
            if (distance < ctx->ir_configs.awbgain_dis) {
                ret = RK_SMART_IR_ALGO_RESULT_NIGHT;
            } else {
                ret = RK_SMART_IR_ALGO_RESULT_DAY;
            }
        } else {
            ret = RK_SMART_IR_ALGO_RESULT_KEEP;
        }
    } else {
        ret = RK_SMART_IR_ALGO_RESULT_KEEP;
    }
    return ret;
}

XCamReturn
rk_smart_ir_runOnce(rk_smart_ir_ctx_t* ctx, rk_aiq_isp_stats_t* stats_ref, rk_smart_ir_result_t* result)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (ctx->cur_working_mode  == -1) {
        rk_aiq_working_mode_t working_mode;

        ret = rk_aiq_uapi2_sysctl_getWorkingMode(ctx->aiq_ctx, &working_mode);
        if (ret == XCAM_RETURN_NO_ERROR)
            ctx->cur_working_mode = working_mode;
    }

    float avg_envL = 0;

    SMARTIR_LOG("===== runOnce for stats frame id %d \n", stats_ref->frame_id);
    int blk_res[3];
    memset(blk_res, 0, sizeof(blk_res));
    if (stats_ref->awb_hw_ver == 4) { //isp32
        float int_time = 0, again = 0, envL_blk = 0;
        int algo_status = 0;
        double Rvalue_blk = 0, Gvalue_blk = 0, Bvalue_blk = 0, Yvalue_blk = 0;
        float RGgain_blk = 0, BGgain_blk = 0;
        float all_RGgain = 0.0f, all_BGgain = 0.0f, all_Dis = 0.0f;
        float part_RGgain = 0.0f, part_BGgain = 0.0f, part_Dis = 0.0f;
        float dis_tmp = 0;
        int cnt = 0;
        Uapi_ExpQueryInfo_t exp_info;
        rk_aiq_user_api2_ae_queryExpResInfo(ctx->aiq_ctx, &exp_info);

        for (int i = 0; i < RK_AIQ_AWB_GRID_NUM_TOTAL; i++) {
            Rvalue_blk = stats_ref->awb_stats_v32.blockResult[i].Rvalue;
            Gvalue_blk = stats_ref->awb_stats_v32.blockResult[i].Gvalue;
            Bvalue_blk = stats_ref->awb_stats_v32.blockResult[i].Bvalue;
            Yvalue_blk = stats_ref->aec_stats.ae_data.chn[0].rawae_big.channely_xy[i];
            RGgain_blk = Rvalue_blk / Gvalue_blk;
            BGgain_blk = Bvalue_blk / Gvalue_blk;
            all_RGgain += RGgain_blk / RK_AIQ_AWB_GRID_NUM_TOTAL;
            all_BGgain += BGgain_blk / RK_AIQ_AWB_GRID_NUM_TOTAL;
            dis_tmp = pow(RGgain_blk - ctx->ir_configs.rggain_base, 2) +
                        pow(BGgain_blk - ctx->ir_configs.bggain_base, 2);
            dis_tmp = pow(dis_tmp, 0.5);
            if (dis_tmp >= ctx->ir_configs.awbgain_rad /*&& Yvalue_blk > 10 && Yvalue_blk < 200*/) {
                part_RGgain += RGgain_blk;
                part_BGgain += BGgain_blk;
                cnt++;
            }
        }

        all_Dis = pow(all_RGgain - ctx->ir_configs.rggain_base, 2) +
                    pow(all_BGgain - ctx->ir_configs.bggain_base, 2);
        all_Dis = pow(all_Dis, 0.5);
        if (cnt < 1)
            cnt = 1;
        part_Dis = pow(part_RGgain / cnt - ctx->ir_configs.rggain_base, 2) +
                    pow(part_BGgain / cnt - ctx->ir_configs.bggain_base, 2);
        part_Dis = pow(part_Dis, 0.5);

        if (ctx->state == RK_SMART_IR_STATUS_NIGHT && !exp_info.IsConverged) {
            ctx->switch_cnts = 0;
        }

        for (int i = 0; i < RK_AIQ_AWB_GRID_NUM_TOTAL; i++) {
           if (ctx->cur_working_mode  == RK_AIQ_WORKING_MODE_NORMAL) {
               int_time = stats_ref->aec_stats.ae_exp.LinearExp.exp_real_params.integration_time * 1000;
               again = stats_ref->aec_stats.ae_exp.LinearExp.exp_real_params.analog_gain;
               envL_blk = stats_ref->aec_stats.ae_data.chn[0].rawae_big.channely_xy[i] / (int_time * again);
           } else if (ctx->cur_working_mode  == RK_AIQ_WORKING_MODE_ISP_HDR2) { // using long frame
               int_time = stats_ref->aec_stats.ae_exp.HdrExp[1].exp_real_params.integration_time * 1000;
               again = stats_ref->aec_stats.ae_exp.HdrExp[1].exp_real_params.analog_gain;
               envL_blk = stats_ref->aec_stats.ae_data.chn[1].rawae_big.channely_xy[i] / (int_time * again);
           } else {
               // hdr3
               int_time = stats_ref->aec_stats.ae_exp.HdrExp[2].exp_real_params.integration_time * 1000;
               again = stats_ref->aec_stats.ae_exp.HdrExp[2].exp_real_params.analog_gain;
               envL_blk = stats_ref->aec_stats.ae_data.chn[2].rawae_big.channely_xy[i] / (int_time * again);
           }
           algo_status = _rk_smart_ir_day_or_night(ctx, envL_blk, part_Dis);
           blk_res[algo_status]++;
           avg_envL += envL_blk;
        }
        avg_envL /= RK_AIQ_AWB_GRID_NUM_TOTAL;

        SMARTIR_LOG(">>> avg_envL:%0.3f, Cur:%s, Night:%d, Day:%d, keep:%d \n", avg_envL,
           ctx->state == RK_SMART_IR_STATUS_DAY ? "Day" : "Night",
           blk_res[2], blk_res[1], blk_res[0]);

        if (ctx->state == RK_SMART_IR_STATUS_DAY) {
             if (blk_res[RK_SMART_IR_ALGO_RESULT_NIGHT] > blk_res[RK_SMART_IR_ALGO_RESULT_DAY] &&
                 blk_res[RK_SMART_IR_ALGO_RESULT_NIGHT] > RK_SMART_IR_BLK_NUM_D2N_THRES)
                 ctx->switch_cnts++;
             else
                 ctx->switch_cnts = 0;
        } else {
             if (blk_res[RK_SMART_IR_ALGO_RESULT_DAY] > blk_res[RK_SMART_IR_ALGO_RESULT_NIGHT] &&
                 blk_res[RK_SMART_IR_ALGO_RESULT_DAY] > RK_SMART_IR_BLK_NUM_N2D_THRES)
                 ctx->switch_cnts++;
             else
                 ctx->switch_cnts = 0;
        }
        if (g_smart_ir_log)
            printf("[SMARTIR] FraID=%d,AEConv=%d,Luma=%0.4f,Exp=[%0.4f,%0.4f],AWBGain=[%0.4f,%0.4f],AllDis=%0.4f,PartDis=%0.4f,Blk=[%d,%d,%d],Cnts=%d,Cur=%s\n",
                    stats_ref->frame_id,
                    exp_info.IsConverged,
                    exp_info.LinAeInfo.MeanLuma,
                    exp_info.LinAeInfo.LinearExp.integration_time, exp_info.LinAeInfo.LinearExp.analog_gain,
                    all_RGgain, all_BGgain,
                    all_Dis,
                    part_Dis,
                    blk_res[0], blk_res[1], blk_res[2],
                    ctx->switch_cnts,
                    ctx->state == RK_SMART_IR_STATUS_DAY ? "Day" : "Night");
    }

#if 0
    Uapi_ExpQueryInfo_t exp_info;

    ret = rk_aiq_user_api2_ae_queryExpResInfo(ctx->aiq_ctx, &exp_info);
    if (ret == XCAM_RETURN_NO_ERROR) {
        SMARTIR_LOG("Ae converge:%d, EnvL:%0.3f, avg_envL:%0.03f\n",
               exp_info.IsConverged, exp_info.GlobalEnvLv, avg_envL);

        SMARTIR_LOG("Ae time:%0.3f , gain:%0.3f, meanluma:%0.3f, LumaDeviation:%0.3f \n",
               exp_info.LinAeInfo.LinearExp.integration_time,
               exp_info.LinAeInfo.LinearExp.analog_gain,
               exp_info.LinAeInfo.MeanLuma,
               exp_info.LinAeInfo.LumaDeviation);
    } else
        return XCAM_RETURN_ERROR_FAILED;

    rk_aiq_wb_querry_info_t wb_info;
    float RGgain = 0.0f, BGgain = 0.0f;
    ret = rk_aiq_user_api2_awb_QueryWBInfo(ctx->aiq_ctx, &wb_info);
    if (ret == XCAM_RETURN_NO_ERROR) {
        RGgain = 1.0 / wb_info.stat_gain_blk.rgain;
        BGgain = 1.0 / wb_info.stat_gain_blk.bgain;
        SMARTIR_LOG("Awb converge:%d, RGgain:%0.3f, BGgain: %0.3f\n",
               wb_info.awbConverged, RGgain, BGgain);
    } else
        return XCAM_RETURN_ERROR_FAILED;

#endif
    if (ctx->switch_cnts > ctx->ir_configs.switch_cnts_th) {
        ctx->state = ctx->state == RK_SMART_IR_STATUS_DAY ? RK_SMART_IR_STATUS_NIGHT : RK_SMART_IR_STATUS_DAY;
        ctx->switch_cnts = 0;
        SMARTIR_LOG("++++++ switch status: %s ! \n", ctx->state == RK_SMART_IR_STATUS_DAY ? "Day" : "Night");
    }

    result->status = ctx->state;

    return ret;
}

RKAIQ_END_DECLARE
