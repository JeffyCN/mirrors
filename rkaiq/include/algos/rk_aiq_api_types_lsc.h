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
#ifndef _RK_AIQ_API_TYPES_LSC_H_
#define _RK_AIQ_API_TYPES_LSC_H_

#include "isp/rk_aiq_isp_lsc21.h"

RKAIQ_BEGIN_DECLARE

#define ALSC_ILLUM_NUM                  14
#define ALSC_ILLUM_NAME_LEN             8
#define ALSC_CALIBDB_LEN                (ALSC_ILLUM_NUM*3)

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_lscC_illu_name),
        M4_TYPE(string),
        M4_UI_PARAM(name),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,8),
        M4_DEFAULT("D50"),
        M4_HIDE_EX(1),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The illumiantion name.))  */
    char sw_lscC_illu_name[ALSC_ILLUM_NAME_LEN];
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_lscC_vignetting_val),
        M4_TYPE(f32),
        M4_UI_PARAM(second_name),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,100),
        M4_DEFAULT(100),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The vignetting value of the Lsc gain profiles\n
        Freq of use: low))  */
    float sw_lscC_vignetting_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(meshGain),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    lsc_meshGain_t meshGain;
} alsc_tableAll_t;

typedef struct {
	/* M4_GENERIC_DESC(
        M4_ALIAS(tableAll),
        M4_TYPE(struct_list),
        M4_UI_MODULE(illu_index_list),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
	alsc_tableAll_t *tableAll;
    /* M4_GENERIC_DESC(
        M4_ALIAS(tableAll_len),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,42),
        M4_DEFAULT(14),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(\n
        Freq of use: low))  */
	int tableAll_len;
} alsc_lscCalib_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_lscT_isoIdx_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,4),
        M4_RANGE_EX(1,4096),
        M4_DEFAULT([1, 4, 8, 16]),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The gain array for the lsc gain table vignetting value array. \n
        Freq of use: high))  */
    float sw_lscT_isoIdx_val[4];
    /* M4_GENERIC_DESC(
        M4_ALIAS(vig),
        M4_TYPE(f32),
        M4_SIZE_EX(1,4),
        M4_RANGE_EX(0,100),
        M4_DEFAULT(100),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The lsc gain table vignetting value array corresponded with gain array. \n
        Freq of use: high))  */
    float sw_lscT_vignetting_val[4];	
} alsc_gain2VigCurve_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_lscC_illu_name),
        M4_TYPE(string),
        M4_UI_PARAM(name),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,8),
        M4_DEFAULT("D50"),
        M4_HIDE_EX(1),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The illumiantion name.))  */
    char sw_lscC_illu_name[ALSC_ILLUM_NAME_LEN];
        /* M4_GENERIC_DESC(
        M4_ALIAS(sw_lscC_wbGainR_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,16.0),
        M4_DEFAULT(1),
        M4_DIGIT_EX(4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(\n
        Freq of use: low))  */
    float sw_lscC_wbGainR_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_lscC_wbGainB_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,16.0),
        M4_DEFAULT(1),
        M4_DIGIT_EX(4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(\n
        Freq of use: low))  */
    float sw_lscC_wbGainB_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(gain2VigCurve),
        M4_TYPE(struct),
        M4_UI_MODULE(array_table_ui),
        M4_INDEX_DEFAULT,
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The vignetting value curve of the lsc gain table based on gain dot.\nFreq of use: high))  */
    alsc_gain2VigCurve_t gain2VigCurve;
} alsc_param_illuLink_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(stAuto.illuLink),
        M4_TYPE(struct_list),
        M4_SIZE_EX(dynamic),
        M4_UI_MODULE(illu_index_list),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The dynamic params array corresponded with illumination array.))  */
    alsc_param_illuLink_t illuLink[ALSC_ILLUM_NUM];
	/* M4_GENERIC_DESC(
        M4_ALIAS(sw_lscT_illuLink_len),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,14.0),
        M4_DEFAULT(7),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(\n
        Freq of use: low))  */
    uint8_t sw_lscT_illuLink_len;
} alsc_param_dyn_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(damp_en),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(enable software damping.))  */
    bool sw_lscT_damp_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(lscCfg),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    lsc_param_static_t lscCfg;
} alsc_param_static_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sta),
        M4_TYPE(struct),
        M4_UI_MODULE(static_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    alsc_param_static_t sta;
    /* M4_GENERIC_DESC(
        M4_ALIAS(dyn),
        M4_TYPE(struct),
        M4_UI_MODULE(dynamic_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO))  */
    alsc_param_dyn_t dyn;
} lsc_param_auto_t;

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
        M4_ORDER(1),
        M4_NOTES(The hardware enable flag))  */
    bool en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(bypass),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The hardware bypass flag))  */
    bool bypass;
    /* M4_GENERIC_DESC(
        M4_ALIAS(stAuto),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(The params of auto mode, be only valid for auto mode))  */
    lsc_param_auto_t stAuto;
    /* M4_GENERIC_DESC(
        M4_ALIAS(stMan),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(4),
        M4_NOTES(TODO))  */
    lsc_param_t stMan;
} lsc_api_attrib_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(tunning),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    lsc_api_attrib_t tunning;
    /* M4_GENERIC_DESC(
        M4_ALIAS(calibdb),
        M4_TYPE(struct),
        M4_UI_MODULE(calib_param),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO))  */
    alsc_lscCalib_t calibdb;
} lsc_calib_attrib_t;

typedef struct alsc_status_s {
	/* M4_GENERIC_DESC(
        M4_ALIAS(sw_lscC_illu_name),
        M4_TYPE(string),
        M4_UI_PARAM(name),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,8),
        M4_DEFAULT("D50"),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The illumiantion name.))  */
    char sw_lscC_illuUsed_name[ALSC_ILLUM_NAME_LEN];
	/* M4_GENERIC_DESC(
        M4_ALIAS(vig),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,100.0),
        M4_DEFAULT(100),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(\n
        Freq of use: low))  */
    float sw_lscC_vignetting_val;
} alsc_status_t;

typedef struct lsc_status_s{
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
        M4_ORDER(1),
        M4_NOTES(The hardware enable flag))  */
    bool en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(bypass),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The hardware bypass flag))  */
    bool bypass;
    /* M4_GENERIC_DESC(
        M4_ALIAS(lscStatus),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(TODO))  */
    lsc_param_t stMan;
    /* M4_GENERIC_DESC(
        M4_ALIAS(alscStatus),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(4),
        M4_NOTES(TODO))  */
    alsc_status_t alscStatus;
} lsc_status_t;
RKAIQ_END_DECLARE

#endif
