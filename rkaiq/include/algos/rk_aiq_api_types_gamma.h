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
#ifndef _RK_AIQ_API_TYPES_GAMMA_H_
#define _RK_AIQ_API_TYPES_GAMMA_H_

#include "common/rk_aiq_comm.h"
#include "isp/rk_aiq_isp_gamma21.h"

#define GAMMA_ISO_STEP_MAX 4

RKAIQ_BEGIN_DECLARE

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(iso),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 204800),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(iso.\n
        Freq of use: low))  */
    uint16_t iso;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_gamma_out_offset),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 4095),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Offset value of gamma_out curve.\n
        Freq of use: low))  */
    // reg: sw_gamma_out_offset
    uint16_t hw_gammaT_outCurve_offset;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_gamma_out_y),
        M4_TYPE(u16),
        M4_SIZE_EX(1,49),
        M4_UI_MODULE(gamma_curve),
        M4_RANGE_EX(0, 4095),
        M4_DEFAULT([[0, 93, 128, 154, 175, 194, 211, 226, 240, 266, 289, 310, 329, 365, 396, 425, 451, 499, 543, 582, 618, 684, 744, 798, 848, 938, 1019, 1093, 1161, 1285, 1396, 1498, 1592, 1761, 1914, 2052, 2181, 2414, 2622, 2813, 2989, 3153, 3308, 3454, 3593, 3727, 3854, 3977, 4095]]), 
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0), 
        M4_ORDER(0),
        M4_NOTES(GAMMA curve y-axis point definition for gamma_out input pixels.\n Freq of use: high))  */
    // reg: sw_gamma_out_y0~sw_gamma_out_y48
    uint16_t hw_gammaT_outCurve_val[CALIBDB_GAMMA_KNOTS_NUM_V11];
} agamma_params_dyn_t;

typedef struct gamma_param_auto_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(dyn),
        M4_TYPE(struct_list),
        M4_SIZE_EX(1,4),
        M4_UI_MODULE(dynamic_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(All dynamic params array corresponded with iso array))  */
    agamma_params_dyn_t dyn[GAMMA_ISO_STEP_MAX];
} agamma_param_auto_t;

typedef struct gamma_api_attrib_s {
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
    agamma_param_auto_t stAuto;
    /* M4_GENERIC_DESC(
        M4_ALIAS(stMan),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The current params of module))  */
    gamma_param_t stMan;
} gamma_api_attrib_t;

typedef struct gamma_status_s {
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
    gamma_param_t stMan;
} gamma_status_t;

RKAIQ_END_DECLARE

#endif