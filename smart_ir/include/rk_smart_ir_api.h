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

#ifndef _RK_AIQ_SMART_IR_API_H_
#define _RK_AIQ_SMART_IR_API_H_

#include "common/rk_aiq_types.h"

#define SMARTIR_VERSION "SMARTIR v2.0.6"

typedef struct rk_aiq_sys_ctx_s rk_aiq_sys_ctx_t;
typedef struct rk_smart_ir_ctx_s rk_smart_ir_ctx_t;

RKAIQ_BEGIN_DECLARE

typedef enum RK_SMART_IR_STATUS_e {
    RK_SMART_IR_STATUS_INVALID          = 0,
    RK_SMART_IR_STATUS_DAY              = 1,
    RK_SMART_IR_STATUS_NIGHT            = 2,
    RK_SMART_IR_STATUS_MAX
} RK_SMART_IR_STATUS_t;

typedef enum RK_SMART_IR_SWTICH_MODE_e {
    RK_SMART_IR_SWITCH_MODE_INVALID     = 0,
    RK_SMART_IR_SWITCH_MODE_AUTO        = 1,
    RK_SMART_IR_SWITCH_MODE_DAY         = 2,
    RK_SMART_IR_SWITCH_MODE_NIGHT       = 3,
    RK_SMART_IR_SWITCH_MODE_TIME        = 4,
    RK_SMART_IR_SWITCH_MODE_MAX
} RK_SMART_IR_SWTICH_MODE_t;

typedef enum RK_SMART_IR_LIGHT_TYPE_e {
    RK_SMART_IR_LIGHT_TYPE_INVALID      = 0,
    RK_SMART_IR_LIGHT_TYPE_VIS          = 1,
    RK_SMART_IR_LIGHT_TYPE_IR           = 2,
    RK_SMART_IR_LIGHT_TYPE_NONE         = 3,
    RK_SMART_IR_LIGHT_TYPE_MIX          = 4,
    RK_SMART_IR_LIGHT_TYPE_MAX
} RK_SMART_IR_LIGHT_TYPE_t;

typedef enum RK_SMART_IR_LIGHT_MODE_e {
    RK_SMART_IR_LIGHT_MODE_INVALID      = 0,
    RK_SMART_IR_LIGHT_MODE_AUTO         = 1,
    RK_SMART_IR_LIGHT_MODE_MANUAL       = 2,
    RK_SMART_IR_LIGHT_MODE_MAX
} RK_SMART_IR_LIGHT_MODE_t;

typedef enum RK_SMART_IR_CALIB_MODE_e {
    RK_SMART_IR_CALIB_MODE_INVALID      = 0,
    RK_SMART_IR_CALIB_MODE_D2N          = 1,
    RK_SMART_IR_CALIB_MODE_N2D          = 2,
    RK_SMART_IR_CALIB_MODE_BASE         = 3,
    RK_SMART_IR_CALIB_MODE_DIS          = 4,
    RK_SMART_IR_CALIB_MODE_MAX
} RK_SMART_IR_CALIB_MODE_t;

typedef struct rk_smart_ir_params_s {
    float d2n_envL_th;
    float n2d_envL_th;
    float rggain_base;
    float bggain_base;
    float awbgain_rad;
    float awbgain_dis;
    uint16_t switch_cnts_th;
} rk_smart_ir_params_t;

typedef struct rk_smart_ir_light_cfg_s {
    bool en_light_cfg;
    float light_expLow_th;
    float light_expHigh_th;
    float light_up_speed;
    float light_down_speed;
    float light_step;
    float light_min;
} rk_smart_ir_light_cfg_t;

typedef struct rk_smart_ir_attr_s {
    RK_SMART_IR_STATUS_t init_status;
    RK_SMART_IR_SWTICH_MODE_t switch_mode;
    RK_SMART_IR_LIGHT_MODE_t light_mode;
    RK_SMART_IR_LIGHT_TYPE_t light_type;
    uint8_t light_value;
    rk_smart_ir_params_t params;
    bool en_quick_switch;
    bool en_grid_weight;
    bool en_auto_n2dth;
    rk_smart_ir_light_cfg_t light_cfg;
} rk_smart_ir_attr_t;

typedef struct rk_smart_ir_result_s {
    RK_SMART_IR_STATUS_t status;
    bool gray_on;
    float fill_value;
    bool is_status_change;
    bool is_gray_change;
    bool is_fill_change;
} rk_smart_ir_result_t;

typedef struct rk_smart_ir_calib_s {
    bool calib_en;
    RK_SMART_IR_CALIB_MODE_t calib_mode;
    rk_smart_ir_params_t calib_params;
    rk_smart_ir_params_t calib_result;
} rk_smart_ir_calib_t;

typedef struct rk_smart_ir_query_info_s {
    float rggain;
    float bggain;
    float ir_strength;
    float all_dis;
    float part_dis;
    uint16_t blk_res[3];
} rk_smart_ir_query_info_t;

typedef struct rk_smart_ir_autoled_s {
    bool is_smooth_convert;
    float auto_irled_val;
    float auto_irled_min;
    float auto_irled_max;
} rk_smart_ir_autoled_t;

rk_smart_ir_ctx_t* rk_smart_ir_init(const rk_aiq_sys_ctx_t* aiq_ctx);
XCamReturn rk_smart_ir_deInit(rk_smart_ir_ctx_t* ctx);

// SMARTIR_VERSION 2.0.0
XCamReturn rk_smart_ir_iniCfg(rk_smart_ir_ctx_t* ctx, const char* config_file);
XCamReturn rk_smart_ir_setAttr(rk_smart_ir_ctx_t* ctx, rk_smart_ir_attr_t* attr);
XCamReturn rk_smart_ir_getAttr(rk_smart_ir_ctx_t* ctx, rk_smart_ir_attr_t* attr);
XCamReturn rk_smart_ir_queryInfo(rk_smart_ir_ctx_t* ctx, rk_smart_ir_query_info_t* query_info);
XCamReturn rk_smart_ir_run(rk_smart_ir_ctx_t* ctx, bool camGroup, rk_smart_ir_result_t* result);
XCamReturn rk_smart_ir_runCb(rk_smart_ir_ctx_t* ctx, bool camGroup, void (*smart_ir_cb)(rk_smart_ir_result_t));
XCamReturn rk_smart_ir_dualRun(rk_smart_ir_ctx_t* ctx0, rk_smart_ir_ctx_t* ctx1, rk_smart_ir_result_t* result);
XCamReturn rk_smart_ir_calib(rk_smart_ir_ctx_t* ctx, rk_smart_ir_calib_t* cfg);

// SMARTIR_VERSION 1.0.0
XCamReturn rk_smart_ir_runOnce(rk_smart_ir_ctx_t* ctx, rk_aiq_isp_stats_t* stats_ref, rk_smart_ir_result_t* result);
XCamReturn rk_smart_ir_groupRunOnce(rk_smart_ir_ctx_t* ctx, rk_aiq_isp_stats_t** grp_stats, int cam_num, rk_smart_ir_result_t* result);
XCamReturn rk_smart_ir_config(rk_smart_ir_ctx_t* ctx, rk_smart_ir_params_t* config);
XCamReturn rk_smart_ir_set_status(rk_smart_ir_ctx_t* ctx, rk_smart_ir_result_t result);
XCamReturn rk_smart_ir_auto_irled(rk_smart_ir_ctx_t* ctx, rk_smart_ir_autoled_t* auto_irled);

RKAIQ_END_DECLARE

#endif
