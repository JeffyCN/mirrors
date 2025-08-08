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

#ifndef _RK_AIQ_API_TYPES_LUT3D_H_
#define _RK_AIQ_API_TYPES_LUT3D_H_

#include "common/rk_aiq_comm.h"
//#include "isp/rk_aiq_isp_lut3d20.h"

#define RK_AIQ_3DLUT_ILLUMINANT_MAX   9

#define ALUT3D_ILLUM_NUM          4
#define ALUT3D_ILLUM_NAME_LEN     8
#define ALUT3D_CALIBDB_LEN        (ALUT3D_ILLUM_NUM)

RKAIQ_BEGIN_DECLARE

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_lut3dC_illu_name),
        M4_TYPE(string),
        M4_UI_PARAM(name),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,8),
        M4_DEFAULT("D50"),
        M4_HIDE_EX(1),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The illumiantion name.))  */
    char sw_lut3dC_illu_name[ALUT3D_ILLUM_NAME_LEN];
    /* M4_GENERIC_DESC(
        M4_ALIAS(meshGain),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The 3dlut profiles elements.))  */
    lut3d_meshGain_t meshGain;
} alut3d_tableAll_t;

typedef struct {
	/* M4_GENERIC_DESC(
        M4_ALIAS(tableAll),
        M4_TYPE(struct_list),
        M4_UI_MODULE(illu_index_list),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(3dlut profiles.\nFreq of use: high))  */
	alut3d_tableAll_t tableAll[ALUT3D_CALIBDB_LEN];
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_lut3dC_tblAll_len),
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
	int sw_lut3dC_tblAll_len;
} alut3d_lut3dCalib_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_lut3dT_isoIdx_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,4),
        M4_RANGE_EX(0,4096),
        M4_DEFAULT([1, 4, 8, 16]),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The gain array for lut3d strength array.\nFreq of use: high))  */
    float sw_lut3dT_isoIdx_val[4];
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
        M4_NOTES(The lut3d strength array corresponded with gain array. \n
        Freq of use: high))  */
    float sw_lut3dT_alpha_val[4];	
} alut3d_gain2StrgCurve_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_lut3dC_illu_name),
        M4_TYPE(string),
        M4_UI_PARAM(name),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,8),
        M4_DEFAULT("D50"),
        M4_HIDE_EX(1),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The illumiantion name.))  */
    char sw_lut3dC_illu_name[ALUT3D_ILLUM_NAME_LEN];
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_lut3dC_wbGainR_val),
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
    float sw_lut3dC_wbGainR_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_lut3dC_wbGainB_val),
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
    float sw_lut3dC_wbGainB_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(gain2StrgCurve),
        M4_TYPE(struct),
        M4_UI_MODULE(array_table_ui),
        M4_INDEX_DEFAULT,
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The lut3d strength curve based on gain dot.\nFreq of use: high))  */
    alut3d_gain2StrgCurve_t gain2StrgCurve;
} alut3d_param_illuLink_t;

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
    alut3d_param_illuLink_t illuLink[ALUT3D_ILLUM_NUM];
	/* M4_GENERIC_DESC(
        M4_ALIAS(sw_lut3dT_illuLink_len),
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
    uint8_t sw_lut3dT_illuLink_len;
} alut3d_param_dyn_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(damp_en),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(enable software damping.))  */
    bool sw_lut3dT_damp_en;
} alut3d_param_static_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sta),
        M4_TYPE(struct),
        M4_UI_MODULE(static_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    alut3d_param_static_t sta;
    /* M4_GENERIC_DESC(
        M4_ALIAS(dyn),
        M4_TYPE(struct),
        M4_UI_MODULE(dynamic_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO))  */
    alut3d_param_dyn_t dyn;
} lut3d_param_auto_t;

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
    lut3d_param_auto_t stAuto;
    /* M4_GENERIC_DESC(
        M4_ALIAS(stMan),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(4),
        M4_NOTES(The current params of module))  */
    lut3d_param_t stMan;
} lut3d_api_attrib_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(tunning),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    lut3d_api_attrib_t tunning;
    /* M4_GENERIC_DESC(
        M4_ALIAS(calibdb),
        M4_TYPE(struct),
        M4_UI_MODULE(calib_param),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO))  */
    alut3d_lut3dCalib_t calibdb;
} lut3d_calib_attrib_t;

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
    char sw_lut3dC_illuUsed_name[ALUT3D_ILLUM_NAME_LEN];
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
    float sw_lut3dT_alpha_val;
} alut3d_status_t;

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
        M4_ALIAS(stMan),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(The current params of module))  */
    lut3d_param_t stMan;
    /* M4_GENERIC_DESC(
        M4_ALIAS(alut3dStatus),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(4),
        M4_NOTES(The current rk swparams of module)) */
    alut3d_status_t alut3dStatus;
} lut3d_status_t;

RKAIQ_END_DECLARE

#endif
