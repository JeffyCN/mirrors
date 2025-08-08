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

#ifndef _RK_AIQ_API_TYPES_POSTISP_H_
#define _RK_AIQ_API_TYPES_POSTISP_H_

#include "common/rk_aiq_comm.h"

#define POSTISP_ISO_STEP_MAX 13

RKAIQ_BEGIN_DECLARE

typedef enum {
    postisp_nr_mode0 = 0,
    postisp_nr_mode1,
    postisp_nr_mode2,
    postisp_nr_mode3
} postisp_nr_mode_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(postisp_nr_mode_t),
        M4_DEFAULT(postisp_nr_mode0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The current nr mode))  */
    postisp_nr_mode_t mode;
    // M4_NUMBER_DESC("gain_max", "f32", M4_RANGE(0, 8192.0), "1", M4_DIGIT(1))
    float gain_max;
    // M4_NUMBER_DESC("gain_tab_len", "u8", M4_RANGE(1, 13), "13", M4_DIGIT(0))
    uint8_t gain_tab_len;
    // M4_BOOL_DESC("tuning_visual_flag", "0")
    bool tuning_visual_flag;
    // M4_ARRAY_DESC("dynamicSw", "f32", M4_SIZE(1,2), M4_RANGE(0,1024), "[60, 68]", M4_DIGIT(1), M4_DYNAMIC(0))
    float dynamicSw[2];
    // M4_ARRAY_DESC("luma_point", "u16", M4_SIZE(1,6), M4_RANGE(0,1024), "[0, 16, 64, 128, 196, 256]", M4_DIGIT(1), M4_DYNAMIC(0))
    uint16_t luma_point[6];
} postisp_static_data_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(data),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    postisp_static_data_t data;
} postisp_params_static_t;

typedef struct {
    // M4_NUMBER_DESC("sigma", "f32", M4_RANGE(0.0, 32.0), "2.0", M4_DIGIT(2))
    float sigma;
    // M4_NUMBER_DESC("shade", "f32", M4_RANGE(-4.0, 4.0), "0.0", M4_DIGIT(2))
    float shade;
    // M4_NUMBER_DESC("sharp", "f32", M4_RANGE(0.0, 4.0), "1.0", M4_DIGIT(2))
    float sharp;
    // M4_NUMBER_DESC("min_luma", "f32", M4_RANGE(0.0, 256.0), "0.0", M4_DIGIT(2))
    float min_luma;
    // M4_NUMBER_DESC("sat_scale", "f32", M4_RANGE(1.0, 3.0), "1", M4_DIGIT(2))
    float sat_scale;
    // M4_NUMBER_DESC("dark_contrast", "f32", M4_RANGE(0.0, 32.0), "0", M4_DIGIT(2))
    float dark_contrast;
    // M4_NUMBER_DESC("ai_ratio", "f32", M4_RANGE(0.0, 1.0), "0", M4_DIGIT(2))
    float ai_ratio;
    // M4_NUMBER_DESC("mot_thresh", "f32", M4_RANGE(0.0, 32.0), "5", M4_DIGIT(2))
    float mot_thresh;
    // M4_NUMBER_DESC("static_thresh", "f32", M4_RANGE(0.0, 32.0), "10", M4_DIGIT(2))
    float static_thresh;
    // M4_NUMBER_DESC("mot_nr_stren", "f32", M4_RANGE(0.0, 2.0), "0", M4_DIGIT(2))
    float mot_nr_stren;
    // M4_ARRAY_DESC("luma_sigma", "f32", M4_SIZE(1,6), M4_RANGE(0,1023), "[3, 3, 3, 3, 3, 3]", M4_DIGIT(0), M4_DYNAMIC(0))
    float luma_sigma[6];
} postisp_dyn_data_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(data),
        M4_TYPE(struct),
        M4_UI_MODULE(dynamic_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    postisp_dyn_data_t data;
} postisp_params_dyn_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sta),
        M4_TYPE(struct),
        M4_UI_MODULE(static_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    postisp_params_static_t sta;
    /* M4_GENERIC_DESC(
        M4_ALIAS(dyn),
        M4_TYPE(struct),
        M4_UI_MODULE(dynamic_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    postisp_params_dyn_t dyn;
} postisp_param_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(static_param),
        M4_TYPE(struct),
        M4_UI_MODULE(static_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The static params of postisp module))  */
    postisp_params_static_t sta;
    /* M4_GENERIC_DESC(
        M4_ALIAS(dynamic_param),
        M4_TYPE(struct_list),
        M4_SIZE_EX(1,13),
        M4_UI_MODULE(dynamic_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(All dynamic params array corresponded with iso array))  */
    postisp_params_dyn_t dyn[POSTISP_ISO_STEP_MAX];
} postisp_param_auto_t;

typedef struct {
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
    postisp_param_auto_t stAuto;
    /* M4_GENERIC_DESC(
        M4_ALIAS(stMan),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The current params of module))  */
    postisp_param_t stMan;
} postisp_api_attrib_t;

typedef struct {
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
    postisp_param_t stMan;
} postisp_status_t;

RKAIQ_END_DECLARE

#endif
