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

#ifndef _RK_AIQ_TYPE_CCM21_H_
#define _RK_AIQ_TYPE_CCM21_H_

#define ACCM_ILLUM_NAME_LEN                      8
#define ACCM_ILLUM_NUM                           9
#define ACCM_CALIBDB_LEN                        (ACCM_ILLUM_NUM*4)
#define ACCM_ISO_NUM                            13

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ccmC_illu_name),
        M4_TYPE(string),
        M4_UI_PARAM(name),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,8),
        M4_DEFAULT("D50"),
        M4_HIDE_EX(1),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The illumiantion name.\nFreq of use: low))  */
    char sw_ccmC_illu_name[ACCM_ILLUM_NAME_LEN];
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ccmC_ccmSat_val),
        M4_TYPE(f32),
        M4_UI_PARAM(second_name),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,200),
        M4_DEFAULT(100),
        M4_DIGIT_EX(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The saturation corresponding to the matix.\nFreq of use: low))  */
    float sw_ccmC_ccmSat_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ccmT_mat),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The ccm matrix elements.\nFreq of use: high))  */
    ccm_matrix_t ccMatrix;
} accm_matrixAll_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(matrixAll),
        M4_TYPE(struct_list),
        M4_UI_MODULE(illu_index_list),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Ccm matrix profiles.\nFreq of use: high))  */
    accm_matrixAll_t matrixAll[ACCM_CALIBDB_LEN];
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ccmC_matrixAll_len),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,36),
        M4_DEFAULT(14),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO.))  */
    uint8_t sw_ccmC_matrixAll_len;
} accm_ccmCalib_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(damp_en),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(enable software damping.\nFreq of use: low))  */
    bool sw_ccmT_damp_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(ccmCfg),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO
        \nFreq of use: low))  */
    ccm_param_static_t ccmCfg;
} accm_param_static_t;

typedef struct {    
    /* M4_GENERIC_DESC(
        M4_ALIAS(scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(1),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The global scale array corresponded with iso array.Freq of use: high))  */
    float sw_ccmT_glbCcm_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(enhance),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The dynamic enhance params corresponded with iso array.\nFreq of use: low)) */
    ccm_enhance_t enhance;
    /* M4_GENERIC_DESC(
        M4_ALIAS(ccmAlpha_yFac),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(The dynamic ccmAlpha_yFac params corresponded with iso array.\nFreq of use: high)) */
    ccm_ccmAlpha_yFac_t ccmAlpha_yFac;
    /* M4_GENERIC_DESC(
        M4_ALIAS(ccmAlpha_satFac),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(4),
        M4_NOTES(The dynamic ccmAlpha_satFac params corresponded with iso array..\nFreq of use: low)) */
    ccm_ccmAlpha_satFac_t ccmAlpha_satFac;
} accm_param_isoLink_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ccmT_isoIdx_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,4),
        M4_RANGE_EX(1,4096),
        M4_DEFAULT([1, 4, 8, 16]),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The gain array for matrix sat array. Freq of use: high))  */
    float sw_ccmT_isoIdx_val[4];
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ccmT_glbSat_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,4),
        M4_RANGE_EX(0,200),
        M4_DEFAULT(100),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The ccm matrix sat array corresponded with gain array. Freq of use: high))  */
    float sw_ccmT_glbSat_val[4];
} accm_gain2SatCurve_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ccmC_illu_name),
        M4_TYPE(string),
        M4_UI_PARAM(name),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,8),
        M4_DEFAULT("D50"),
        M4_HIDE_EX(1),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The illumiantion name.\nFreq of use: low))  */
    char sw_ccmC_illu_name[ACCM_ILLUM_NAME_LEN];
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ccmC_wbGainR_val),
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
    float sw_ccmC_wbGainR_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ccmC_wbGainB_val),
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
    float sw_ccmC_wbGainB_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(gain2SatCurve),
        M4_TYPE(struct),
        M4_UI_MODULE(array_table_ui),
        M4_INDEX_DEFAULT,
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The sat curve based on gain dot.\nFreq of use: high))  */
    accm_gain2SatCurve_t gain2SatCurve;
} accm_param_illuLink_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(stAuto.illuLink),
        M4_TYPE(struct_list),
        M4_SIZE_EX(dynamic),
        M4_UI_MODULE(illu_index_list),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The dynamic params array corresponded with illumination array. \n
        Freq of use: high))  */
    accm_param_illuLink_t illuLink[ACCM_ILLUM_NUM];
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ccmT_illuLink_len),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,9),
        M4_DEFAULT(1),
        M4_DYNAMIC(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO.))  */
    uint8_t sw_ccmT_illuLink_len;
    /* M4_GENERIC_DESC(
        M4_ALIAS(stAuto.isoLink),
        M4_TYPE(struct_list),
        M4_SIZE_EX(1,13),
        M4_UI_MODULE(iso_index_list),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The dynamic params array corresponded with iso array. \n
        Freq of use: high))  */
    accm_param_isoLink_t isoLink[ACCM_ISO_NUM];
} accm_param_dyn_t;

typedef struct {
     /* M4_GENERIC_DESC(
        M4_ALIAS(sta),
        M4_TYPE(struct),
        M4_UI_MODULE(static_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    accm_param_static_t sta;
    /* M4_GENERIC_DESC(
        M4_ALIAS(dyn),
        M4_TYPE(struct),
        M4_UI_MODULE(dynamic_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO))  */
    accm_param_dyn_t dyn;
} ccm_param_auto_t;

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
    ccm_param_auto_t stAuto;
    /* M4_GENERIC_DESC(
        M4_ALIAS(stMan),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(4),
        M4_NOTES(The params of manual mode))  */
    ccm_param_t stMan;
} ccm_api_attrib_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(tunning),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    ccm_api_attrib_t tunning;
    /* M4_GENERIC_DESC(
        M4_ALIAS(calibdb),
        M4_TYPE(struct),
        M4_UI_MODULE(calib_param),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO))  */
    accm_ccmCalib_t calibdb;
} ccm_calib_attrib_t;

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
    char sw_ccmC_illuUsed_name[ACCM_ILLUM_NAME_LEN];
    /* M4_GENERIC_DESC(
        M4_ALIAS(ccmSat_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,200),
        M4_DEFAULT(100),
        M4_DIGIT_EX(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The current saturation corresponding to the used matix.))  */
    float sw_ccmC_ccmSat_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The current global scale corresponded with gain.))  */
    float sw_ccmT_glbCcm_scale;
} accm_status_t;


typedef struct ccm_status_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(opMode),
        M4_TYPE(enum),
        M4_ENUM_DEF(rk_aiq_op_mode_t),
        M4_DEFAULT(RK_AIQ_OP_MODE_AUTO),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The current operation mode)) */
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
        M4_ALIAS(ccmStatus),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(The current hwparams of module)) */
    ccm_param_t stMan;    
    /* M4_GENERIC_DESC(
        M4_ALIAS(accmStatus),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(4),
        M4_NOTES(The current rk swparams of module)) */
    accm_status_t accmStatus;
} ccm_status_t;

#endif

