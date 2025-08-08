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
 */
#ifndef _RK_AIQ_API_TYPES_MERGE_H_
#define _RK_AIQ_API_TYPES_MERGE_H_

#include "common/rk_aiq_comm.h"
#if ISP_HW_V35
#include "isp/rk_aiq_isp_merge23.h"
#else
#include "isp/rk_aiq_isp_merge22.h"
#endif

#define MERGE_LINK_NUM 13

RKAIQ_BEGIN_DECLARE

typedef enum amge_paraLink_mode_s {

    amge_isoLink_mode = 0,

    amge_envLink_mode = 1
} amge_paraLink_mode_t;

typedef struct amge_paraLinkCfg_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_mgeT_paraLink_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(amge_paraLink_mode_t),
        M4_DEFAULT(amge_isoLink_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO\n
        Reference enum types.\n
        Freq of use: low))  */
    amge_paraLink_mode_t sw_mgeT_paraLink_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_mgeT_isoLink_val),
        M4_TYPE(s32),
        M4_SIZE_EX(1,13),
        M4_RANGE_EX(0,1000000),
        M4_DEFAULT([50, 100, 200, 400, 800, 1600, 3200, 6400, 12800, 25600, 51200, 102400, 204800]),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(paraLink_mode == amge_isoLink_mode.\n
        Freq of use: low))  */
    int sw_mgeT_isoLink_val[MERGE_LINK_NUM];
    /* M4_GENERIC_DESC(
        M4_ALIAS( sw_mgeT_envLink_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,13),
        M4_RANGE_EX(0,65535),
        M4_DEFAULT([0, 0.005, 0.01, 0.05, 0.1, 0.15, 0.2, 0.3, 0.4, 0.5, 0.6, 0.8, 1]),
        M4_DIGIT_EX(4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(paraLink_mode == amge_envLink_mode.\n
        Freq of use: low))  */
    float sw_mgeT_envLink_val[MERGE_LINK_NUM];
} amge_paraLinkCfg_t;

typedef struct amge_param_static_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(mgeCfg),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.\n
        Freq of use: low))  */
    mge_params_static_t mgeCfg;
    /* M4_GENERIC_DESC(
        M4_ALIAS(paraLinkCfg),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.\n
        Freq of use: low))  */
    amge_paraLinkCfg_t paraLinkCfg;
    /* M4_GENERIC_DESC(
    M4_ALIAS( sw_mgeT_oeDamp_val),
    M4_TYPE(f32),
    M4_SIZE_EX(1,1),
    M4_RANGE_EX(0,1),
    M4_DEFAULT(0.1),
    M4_DIGIT_EX(3),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(0),
    M4_NOTES(In order to avoid oeWgt from changing too fast, which leads to flicker. The sameller
    the value, the slower the oeWgt change.\n Freq of use: low))  */
    float sw_mgeT_oeDamp_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS( sw_mgeT_mdDamp_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0.1),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(In order to avoid mdWgt from changing too fast, which leads to flicker. The
       sameller the value, the slower the mdWgt change.\n Freq of use: low))  */
    float sw_mgeT_mdDamp_val;
} amge_param_static_t;

typedef struct amge_params_dyn_s {
    /* M4_GENERIC_DESC(
       M4_ALIAS(sw_mgeT_baseFrm_mode),
       M4_TYPE(enum),
       M4_ENUM_DEF(mge_baseFrm_mode_t),
       M4_DEFAULT(mge_baseHdrL_mode),
       M4_GROUP_CTRL(baseFrm_mode_group),
       M4_HIDE_EX(0),
       M4_RO(0),
       M4_ORDER(0),
       M4_NOTES(TODO\n
       Reference enum types.\n
       Freq of use: high))  */
    mge_baseFrm_mode_t sw_mgeT_baseFrm_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_mgeT_baseHdrL_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(mge_baseHdrL_mode_t),
        M4_DEFAULT(mge_oeMdByLuma_mode),
        M4_GROUP_CTRL(baseHdrL_mode_group),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.\n
        Reference enum types.\n
        Freq of use: low))  */
    mge_baseHdrL_mode_t sw_mgeT_baseHdrL_mode;
#if ISP_HW_V35
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_mgeT_baseHdrS_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(mge_baseHdrS_mode_t),
        M4_DEFAULT(mge_motionDR_SPrior_mode),
        M4_GROUP_CTRL(baseHdrS_mode_group),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.\n
        Reference enum types.\n
        Freq of use: low))  */
    mge_baseHdrS_mode_t sw_mgeT_baseHdrS_mode;
#endif
    /* M4_GENERIC_DESC(
        M4_ALIAS(oeWgt),
        M4_TYPE(struct),
        M4_UI_MODULE(dynamic_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO.\n Freq of use: high))  */
    mge_oeWgt_t oeWgt;
    /* M4_GENERIC_DESC(
        M4_ALIAS(mdWgt_baseHdrL),
        M4_TYPE(struct),
        M4_UI_MODULE(dynamic_ui),
        M4_GROUP(baseFrm_mode_group:mge_baseHdrL_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO.\n Freq of use: high))  */
    mge_mdWgt_baseHdrL_t mdWgt_baseHdrL;
    /* M4_GENERIC_DESC(
        M4_ALIAS(mdWgt_baseHdrS),
        M4_TYPE(struct),
        M4_UI_MODULE(dynamic_ui),
        M4_GROUP(baseFrm_mode_group:mge_baseHdrS_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO.\n Freq of use: low))  */
    mge_mdWgt_baseHdrS_t mdWgt_baseHdrS;
} amge_params_dyn_t;

typedef struct mge_param_auto_s {
    /* M4_GENERIC_DESC(
       M4_ALIAS(sta),
       M4_SIZE_EX(1,1),
       M4_TYPE(struct),
       M4_UI_MODULE(static_ui),
       M4_HIDE_EX(0),
       M4_RO(0),
       M4_ORDER(2),
       M4_NOTES(TODO))  */
    amge_param_static_t sta;
    /* M4_GENERIC_DESC(
        M4_ALIAS(dyn),
        M4_SIZE_EX(1,13),
        M4_TYPE(struct_list),
        M4_UI_MODULE(dynamic_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    amge_params_dyn_t dyn[MERGE_LINK_NUM];
} mge_param_auto_t;

typedef struct mge_api_attrib_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(opMode),
        M4_TYPE(enum),
        M4_ENUM_DEF(rk_aiq_op_mode_t),
        M4_DEFAULT(RK_AIQ_OP_MODE_AUTO),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The current operation mode))  */
    rk_aiq_op_mode_t opMode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(enable),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The hardware enable flag))  */
    bool en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(enable),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The hardware enable flag))  */
    bool bypass;
    /* M4_GENERIC_DESC(
        M4_ALIAS(stAuto),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The params of auto mode, be only valid for auto mode))  */
    mge_param_auto_t stAuto;
    /* M4_GENERIC_DESC(
        M4_ALIAS(stMan),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The current params of module))  */
    mge_param_t stMan;
} mge_api_attrib_t;

typedef struct mge_status_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(opMode),
        M4_TYPE(enum),
        M4_ENUM_DEF(rk_aiq_op_mode_t),
        M4_DEFAULT(RK_AIQ_OP_MODE_AUTO),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The current operation mode))  */
    rk_aiq_op_mode_t opMode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(enable),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The hardware enable flag))  */
    bool en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(enable),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The hardware enable flag))  */
    bool bypass;
    /* M4_GENERIC_DESC(
        M4_ALIAS(stMan),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The current params of module))  */
    mge_param_t stMan;
} mge_status_t;

RKAIQ_END_DECLARE

#endif
