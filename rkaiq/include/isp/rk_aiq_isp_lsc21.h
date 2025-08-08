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

#ifndef _RK_AIQ_PARAM_LSC21_H_
#define _RK_AIQ_PARAM_LSC21_H_

#define LSC_LSCTABLE_SIZE          289
#define LSC_MESHGRID_SIZE           16
#define LSC_MESHGRID_NUM (LSC_MESHGRID_SIZE+1)

typedef struct lsc_meshGrid_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(posX),
        M4_TYPE(f32),
        M4_SIZE_EX(1,17),
        M4_RANGE_EX(0, 1),
        M4_DEFAULT([0,0.0625,0.125,0.1875,0.25,0.3125,0.375,0.4375,0.5,0.5625,0.625,0.6875,0.75,0.8125,0.875,0.9375,1]),
        M4_DIGIT_EX(5),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(meshGrid_mode_group:lsc_usrConfig_mode),
        M4_NOTES(The normalized X-axis value of the i-th sampling point,
        begin with 0, end with 1, (posX_f[i+1] x width - posX_f[i] x width) >= 12.\n
        Freq of use: low))  */
    //reg: x_sect_size_0 ~ x_sect_size_f=(int)(posX_f[i+1]*width)-(int)(posX_f[i]*width)
    float posX_f[LSC_MESHGRID_NUM];
    /* M4_GENERIC_DESC(
        M4_ALIAS(posY),
        M4_TYPE(f32),
        M4_SIZE_EX(1,17),
        M4_RANGE_EX(0, 1),
        M4_DEFAULT([0,0.0625,0.125,0.1875,0.25,0.3125,0.375,0.4375,0.5,0.5625,0.625,0.6875,0.75,0.8125,0.875,0.9375,1]),
        M4_DIGIT_EX(5),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_GROUP(meshGrid_mode_group:lsc_usrConfig_mode),
        M4_NOTES(The normalized Y-axis value of the i-th sampling point,
        begin with 0, end with 1, (posY_f[i+1] x height - posY_f[i] x height) >= 8.\n
        Freq of use: low))  */
    //reg: y_sect_size_0 ~ y_sect_size_f=(int)(posY_f[i+1]*height)-(int)(posY_f[i]*height)
    float posY_f[LSC_MESHGRID_NUM];
} lsc_meshGrid_t;

typedef enum lsc_meshGrid_mode_e {
    // by user config
    lsc_usrConfig_mode = 0,
    // @reg: x_sect_size = width/LSC_MESHGRID_SIZE
    // @reg: y_sect_size = height/LSC_MESHGRID_SIZE
    lsc_equalSector_mode = 1,
    // default set to lsc_equalSector_mode
    lsc_vendorDefault_mode,
} lsc_meshGrid_mode_t;

typedef struct lsc_meshGain_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_lscC_gainR_val),
        M4_TYPE(u16),
        M4_SIZE_EX(17,17),
        M4_RANGE_EX(1024, 8191),
        M4_DEFAULT(1024),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Correction factor at R channel of sample point.\n
        Freq of use: low))  */
    //reg: r_sample_0
    uint16_t hw_lscC_gainR_val[LSC_LSCTABLE_SIZE];
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_lscC_gainGr_val),
        M4_TYPE(u16),
        M4_SIZE_EX(17,17),
        M4_RANGE_EX(1024, 8191),
        M4_DEFAULT(1024),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(Correction factor at Gr channel of sample point.\n
        Freq of use: low))  */
    //reg: gr_sample_0
    uint16_t hw_lscC_gainGr_val[LSC_LSCTABLE_SIZE];
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_lscC_gainB_val),
        M4_TYPE(u16),
        M4_SIZE_EX(17,17),
        M4_RANGE_EX(1024, 8191),
        M4_DEFAULT(1024),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(Correction factor at B channel of sample point.\n
        Freq of use: low))  */
    //reg: b_sample_0
    uint16_t hw_lscC_gainB_val[LSC_LSCTABLE_SIZE];
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_lscC_gainGb_val),
        M4_TYPE(u16),
        M4_SIZE_EX(17,17),
        M4_RANGE_EX(1024, 8191),
        M4_DEFAULT(1024),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(Correction factor at Gb channel of sample point.\n
        Freq of use: low))  */
    //reg: gb_sample_0
    uint16_t hw_lscC_gainGb_val[LSC_LSCTABLE_SIZE];
} lsc_meshGain_t;

typedef struct lsc_param_static_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_lscT_meshGrid_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(lsc_meshGrid_mode_t),
        M4_DEFAULT(lsc_equalSector_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP_CTRL(meshGrid_mode_group),
        M4_NOTES(The meshGrid distribution mode.\n
        Freq of use: low))  */
    lsc_meshGrid_mode_t sw_lscT_meshGrid_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(meshGrid),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The meshGrid distribution config by user. Only be configured in lsc_usrConfig_mode.\n
        Freq of use: low))  */
    lsc_meshGrid_t meshGrid;
} lsc_param_static_t;

typedef struct lsc_param_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(meshGain),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    lsc_meshGain_t meshGain;
} lsc_param_dyn_t;

typedef struct lsc_param_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sta),
        M4_TYPE(struct),
        M4_UI_MODULE(static_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    lsc_param_static_t sta;
    /* M4_GENERIC_DESC(
        M4_ALIAS(dyn),
        M4_TYPE(struct),
        M4_UI_MODULE(dynamic_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO))  */
    lsc_param_dyn_t dyn;
} lsc_param_t;

#endif

