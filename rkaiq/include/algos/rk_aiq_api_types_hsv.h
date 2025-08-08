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

#ifndef _RK_AIQ_API_TYPES_HSV_H_
#define _RK_AIQ_API_TYPES_HSV_H_

#include "common/rk_aiq_comm.h"

#define AHSV_ILLUM_NUM          7
#define AHSV_ILLUM_NAME_LEN     8
#define AHSV_CALIBDB_LEN        (AHSV_ILLUM_NUM)

RKAIQ_BEGIN_DECLARE

typedef struct {
    bool en;
    int16_t hueOffset;
} ahsv_hueOffset_t;

typedef struct {
    bool en;
    float satStrg;
} ahsv_satStrg_t;

typedef struct {
    bool en;
    int16_t valOffset;
} ahsv_valOffset_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_hsvC_illu_name),
        M4_TYPE(string),
        M4_UI_PARAM(name),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,8),
        M4_DEFAULT("D50"),
        M4_HIDE_EX(1),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The illumiantion name.))  */
    char sw_hsvC_illu_name[AHSV_ILLUM_NAME_LEN];
    /* M4_GENERIC_DESC(
        M4_ALIAS(meshGain),
        M4_TYPE(struct),
        M4_UI_MODULE(HSV_Curve),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The hsv profiles elements.)) */
    hsv_param_dyn_t meshGain;
} ahsv_tableAll_t;

typedef struct {
	/* M4_GENERIC_DESC(
        M4_ALIAS(tableAll),
        M4_TYPE(struct_list),
        M4_UI_MODULE(illu_index_list),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(hsv profiles.\nFreq of use: high))  */
	ahsv_tableAll_t tableAll[AHSV_CALIBDB_LEN];
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_hsvC_tblAll_len),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,4),
        M4_DEFAULT(1),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(\n
        Freq of use: low))  */
	int sw_hsvCfg_tblAll_len;
} ahsv_hsvCalib_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_hsvT_isoIdx_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,4),
        M4_RANGE_EX(0,4096),
        M4_DEFAULT([1, 4, 8, 16]),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The gain array for hsv strength array.\nFreq of use: high))  */
    float sw_hsvT_isoIdx_val[4];
    /* M4_GENERIC_DESC(
        M4_ALIAS(alpha),
        M4_TYPE(f32),
        M4_SIZE_EX(1,4),
        M4_RANGE_EX(0,1.0),
        M4_DEFAULT(1),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The hsv strength array corresponded with gain array. \n
        Freq of use: high))  */
    float sw_hsvT_alpha_val[4];	
} ahsv_gain2StrgCurve_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_hsvC_illu_name),
        M4_TYPE(string),
        M4_UI_PARAM(name),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,8),
        M4_DEFAULT("D50"),
        M4_HIDE_EX(1),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The illumiantion name.))  */
    char sw_hsvC_illu_name[AHSV_ILLUM_NAME_LEN];
        /* M4_GENERIC_DESC(
        M4_ALIAS(sw_hsvC_wbGainR_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,16.0),
        M4_DEFAULT(1),
        M4_DIGIT_EX(4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(\n
        Freq of use: low))  */
    float sw_hsvC_wbGainR_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_hsvC_wbGainB_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,16.0),
        M4_DEFAULT(1),
        M4_DIGIT_EX(4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(\n
        Freq of use: low))  */
    float sw_hsvC_wbGainB_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(gain2StrgCurve),
        M4_TYPE(struct),
        M4_UI_MODULE(array_table_ui),
        M4_INDEX_DEFAULT,
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The hsv strength curve based on gain dot.\nFreq of use: high))  */
    ahsv_gain2StrgCurve_t gain2StrgCurve;
} ahsv_param_illuLink_t;

typedef struct {
	/* M4_GENERIC_DESC(
        M4_ALIAS(illuLink),
        M4_TYPE(struct_list),
        M4_UI_MODULE(illu_index_list),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(hsv profiles.\nFreq of use: high))  */
	ahsv_param_illuLink_t illuLink[AHSV_ILLUM_NUM];
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_hsvCfg_illuLink_len),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,7),
        M4_DEFAULT(1),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(\n
        Freq of use: low))  */
    uint8_t sw_hsvCfg_illuLink_len;
} ahsv_param_dyn_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(wbgainDiff th),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,5),
        M4_DEFAULT(0.05),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The threshold of wbgain difference for determining whether to update lut params. \n
        Freq of use: low))  */
    float sw_hsvT_wbgainDiff_th;
    /* M4_GENERIC_DESC(
        M4_ALIAS(gainDiff th),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,16),
        M4_DEFAULT(0.2),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The threshold of gain difference for determining whether to update lut params. \n
        Freq of use: low))  */
    float sw_hsvT_gainDiff_th;
    /* M4_GENERIC_DESC(
        M4_ALIAS(damp_en),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(enable software damping. \nFreq of use: low))  */
    bool sw_hsvT_damp_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hsvCfg),
        M4_TYPE(struct),
        M4_UI_MODULE(static_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    hsv_param_static_t hsvCfg;
} ahsv_param_static_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sta),
        M4_TYPE(struct),
        M4_UI_MODULE(static_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    ahsv_param_static_t sta;
    /* M4_GENERIC_DESC(
        M4_ALIAS(dyn),
        M4_TYPE(struct),
        M4_UI_MODULE(dynamic_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO))  */
    ahsv_param_dyn_t dyn;
} hsv_param_auto_t;

typedef struct hsv_api_attrib_s {
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
    hsv_param_auto_t stAuto;
    /* M4_GENERIC_DESC(
        M4_ALIAS(stMan),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The current params of module))  */
    hsv_param_t stMan;
} hsv_api_attrib_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(tunning),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    hsv_api_attrib_t tunning;
    /* M4_GENERIC_DESC(
        M4_ALIAS(calibdb),
        M4_TYPE(struct),
        M4_UI_MODULE(calib_param),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO))  */
    ahsv_hsvCalib_t calibdb;
} hsv_calib_attrib_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(illu_name),
        M4_TYPE(string),
        M4_UI_PARAM(name),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,8),
        M4_DEFAULT("D50"),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The current illumiantion name.))  */
    char sw_hsvC_illuUsed_name[AHSV_ILLUM_NAME_LEN];
    /* M4_GENERIC_DESC(
        M4_ALIAS(alpha),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The current lut alpha corresponded with gain.))  */
    float sw_hsvT_alpha_val;
} ahsv_status_t;

typedef struct hsv_status_s {
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
        M4_ORDER(0),
        M4_NOTES(The current params of module))  */
    hsv_param_t stMan;
    /* M4_GENERIC_DESC(
        M4_ALIAS(ahsvStatus),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The current rk swparams of module)) */
    ahsv_status_t ahsvStatus;
} hsv_status_t;

RKAIQ_END_DECLARE

#endif
