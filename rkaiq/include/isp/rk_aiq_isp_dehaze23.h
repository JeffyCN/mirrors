/*
 * Copyright (c) 2023 Rockchip Eletronics Co., Ltd.
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
 */

#ifndef _RK_AIQ_PARAM_DEHAZE23_H_
#define _RK_AIQ_PARAM_DEHAZE23_H_

#define DHAZ_ENHANCE_CURVE_KNOTS_NUM 17

typedef enum {
    // @note: dhaz work in dehaze mode
    dhaz_dehaze_mode = 0,
    // @note: dhaz work in enhance mode
    dhaz_enhance_mode = 1
} dhaz_work_mode_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(iir_wt_sigma),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,256.0),
        M4_DEFAULT(8.0),
        M4_DIGIT_EX(2f3b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The sigma of iir WgtMax.\n
        Freq of use: low))  */
    float hw_dhazT_iirWgtMaxSgm_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(iir_air_sigma),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(4.0,1024.0),
        M4_DEFAULT(120.0),
        M4_DIGIT_EX(0f8b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The sigma of iir AirLight.\n
        Freq of use: low))  */
    float hw_dhazT_iirAirLightSgm_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(iir_tmax_sigma),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(4.0,1024.0),
        M4_DEFAULT(120.0),
        M4_DIGIT_EX(0f8b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The sigma of iir TransRatMin.\n
        Freq of use: low))  */
    float hw_dhazT_iirTransRatMinSgm_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(stab_fnum),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,32),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The frame num for iir stable.\n
        Freq of use: low))  */
    uint8_t hw_dhazT_iirFrm_maxLimit;
} dhaz_dehaze_iir_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(dc_min_th),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(16,120),
        M4_DEFAULT(64),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The min thread of dark channel.\n
                Freq of use: low))  */
    uint8_t hw_dhazT_darkCh_minThred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(dc_max_th),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(170,255),
        M4_DEFAULT(192),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The max thread of dark channel.\n
                Freq of use: low))  */
    uint8_t hw_dhazT_darkCh_maxThred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(yhist_th),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(170,255),
        M4_DEFAULT(249),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The max thread of Y channel block.\n
                Freq of use: low))  */
    uint8_t hw_dhazT_lumaCount_maxThred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(yblk_th),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.002,0.1),
        M4_DEFAULT(0.002),
        M4_DIGIT_EX(4f9b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The ratio of Y channel block.\n
        Freq of use: low))  */
    float hw_dhazT_lumaCount_minRatio;
    /* M4_GENERIC_DESC(
        M4_ALIAS(dark_th),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(230,250),
        M4_DEFAULT(250),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The nin block value of Y channel for auto wgtMax.\n
                Freq of use: low))  */
    uint8_t hw_dhazT_darkArea_thred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(wt_max),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,1.0),
        M4_DEFAULT(0.9),
        M4_DIGIT_EX(2f8b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The scale value of auto wgtMax.\n
        Freq of use: low))  */
    float hw_dhazT_wgtMax_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(cfg_wt),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,1.0),
        M4_DEFAULT(0.9),
        M4_DIGIT_EX(2f8b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The user-config value of WgtMax.\n
        Freq of use: high))  */
    float hw_dhazT_userWgtMax_val;
} dhaz_dehaze_wgtMax_dyn_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(air_lc_en),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The enable of airBaseLimit.\n
        Freq of use: low))  */
    bool hw_dhazT_airBaseLimit_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(bright_min),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(160,200),
        M4_DEFAULT(180),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The min limit thread value of auto AirLight.\n
                Freq of use: low))  */
    uint8_t hw_dhazT_bright_minLimit;
    /* M4_GENERIC_DESC(
        M4_ALIAS(bright_max),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(210,250),
        M4_DEFAULT(240),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The max limit thread value of auto AirLight.\n
                Freq of use: low))  */
    uint8_t hw_dhazT_bright_maxLimit;
    /* M4_GENERIC_DESC(
        M4_ALIAS(air_min),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(200,220),
        M4_DEFAULT(200),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The min limit value of auto AirLight.\n
                Freq of use: low))  */
    uint8_t hw_dhazT_airLight_minLimit;
    /* M4_GENERIC_DESC(
        M4_ALIAS(air_max),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(230,250),
        M4_DEFAULT(250),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The max limit value of auto AirLight.\n
        Freq of use: low))  */
    uint8_t hw_dhazT_airLight_maxLimit;
    /* M4_GENERIC_DESC(
        M4_ALIAS(cfg_air),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(200),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The user-config value of AirLight.\n
                Freq of use: high))  */
    uint8_t hw_dhazT_userAirLight_val;
} dhaz_dehaze_airLight_dyn_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(tmax_base),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(105,131),
        M4_DEFAULT(125),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The scale value of auto TransRatMin.\n
                Freq of use: low))  */
    uint8_t hw_dhazT_transRatMin_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(tmax_off),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.1,0.5),
        M4_DEFAULT(0.1),
        M4_DIGIT_EX(2f10b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The offset value of auto TransRatMin.\n
        Freq of use: low))  */
    float hw_dhazT_transRatMin_offset;
    /* M4_GENERIC_DESC(
        M4_ALIAS(tmax_max),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.1,0.5),
        M4_DEFAULT(0.5),
        M4_DIGIT_EX(2f10b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The max limit value of auto TransRatMin.\n
        Freq of use: low))  */
    float hw_dhazT_transRatMin_maxLimit;
    /* M4_GENERIC_DESC(
        M4_ALIAS(cfg_tmax),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,1.0),
        M4_DEFAULT(0.0),
        M4_DIGIT_EX(2f10b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The user-config value of TransRatMin.\n
        Freq of use: high))  */
    float hw_dhazT_userTransRatMin_val;
} dhaz_dehaze_transRatMin_dyn_t;

typedef struct {
    /* M4_GENERIC_DESC(
    M4_ALIAS(enh_luma_en),
    M4_TYPE(bool),
    M4_DEFAULT(0),
    M4_GROUP_CTRL(luma2strg_en_group),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(1),
    M4_NOTES(The enable of using luma as index to control the enhance strength.\n
    Freq of use: low))  */
    bool hw_dhazT_luma2strg_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(color_deviate_en),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_GROUP_CTRL(cProtect_en_group),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The enable of color protection.\n
        Freq of use: low))  */
    bool hw_dhazT_cProtect_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(enh_luma),
        M4_TYPE(f32),
        M4_UI_MODULE(drc_curve),
        M4_SIZE_EX(1,17),
        M4_RANGE_EX(0.0,16.0),
        M4_DEFAULT(0.0),
        M4_DIGIT_EX(2f6b),
        M4_HIDE_EX([1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0]),
        M4_DATAX([0, 64, 128, 192, 256, 320, 384, 448, 512, 576, 640, 704, 768, 832, 896, 960, 1024]),
        M4_GROUP(luma2strg_en_group),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The contrast strength of enhance when hw_dhazT_luma2strg_en == 1.\n
        Freq of use: high))  */
    float hw_dhazT_luma2strg_val[DHAZ_ENHANCE_CURVE_KNOTS_NUM];
    /* M4_GENERIC_DESC(
        M4_ALIAS(enhance_value),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,16.0),
        M4_DEFAULT(0.0),
        M4_DIGIT_EX(2f14b),
        M4_HIDE_EX(0),
        M4_GROUP(!luma2strg_en_group),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The contrast strength of enhance when hw_dhazT_luma2strg_en == 0.\n
        Freq of use: high))  */
    float hw_dhazT_contrast_strg;
    /* M4_GENERIC_DESC(
        M4_ALIAS(enhance_chroma),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,16.0),
        M4_DEFAULT(0.0),
        M4_DIGIT_EX(2f14b),
        M4_GROUP(cProtect_en_group),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The saturate strength of enhance.\n
        Freq of use: high))  */
    float hw_dhazT_saturate_strg;
} dhaz_enhance_dyn_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(dc_weitcur),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,1.0),
        M4_DEFAULT(0.1),
        M4_DIGIT_EX(2f8b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The bilateral sigma of dark channel.\n
        Freq of use: low))  */
    float hw_dhazT_bifilt_wgt;
    /* M4_GENERIC_DESC(
        M4_ALIAS(bf_weight),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,1.0),
        M4_DEFAULT(0.5),
        M4_DIGIT_EX(2f8b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(In the fusion operation of the cur bifilt and the iir bifilt, the fusion weight
       value of the iir bifilt.\n Freq of use: low))  */
    float hw_dhazT_bifilt_alpha;
    /* M4_GENERIC_DESC(
        M4_ALIAS(space_sigma_pre),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,1.0),
        M4_DEFAULT(0.5),
        M4_DIGIT_EX(2f8b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The pre frame sapce sigma of the bilateral Filter.\n
        Freq of use: low))  */
    float hw_dhazT_preSpatialSgm_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(space_sigma_cur),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,1.0),
        M4_DEFAULT(0.5),
        M4_DIGIT_EX(2f8b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The cur frame sapce sigma of the bilateral Filter.\n
        Freq of use: low))  */
    float hw_dhazT_curSpatialSgm_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(range_sima),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,1.0),
        M4_DEFAULT(0.5),
        M4_DIGIT_EX(2f8b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The range sigma of the thumb filter.\n
        Freq of use: low))  */
    float hw_dhazT_rgeSgm_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(iir_pre_wet),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,16),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The max value of frame stable.\n
        Freq of use: low))  */
    uint8_t hw_dhazT_iirFilt_wgt;
} dhaz_bifilt_dyn_t;

typedef struct {
    /* M4_GENERIC_DESC(
    M4_ALIAS(sw_dhazT_work_mode),
    M4_TYPE(enum),
    M4_ENUM_DEF(dhaz_work_mode_t),
    M4_DEFAULT(dhaz_enhance_mode),
    M4_GROUP_CTRL(dhazT_work_mode_group),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(0),
    M4_NOTES(The working mode of dehaze module.\n
    Freq of use: low))  */
    dhaz_work_mode_t sw_dhazT_work_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(cfg_alpha),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,1.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(2f8b),
        M4_GROUP(dhazT_work_mode_group:dhaz_dehaze_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(In the fusion operation of auto params and user-config params in dehaze fuction,
       the fusion weight value of user-config params.\n Freq of use: low))  */
    float hw_dhazT_paramMerge_alpha;
    /* M4_GENERIC_DESC(
        M4_ALIAS(enh_curve),
        M4_TYPE(u16),
        M4_UI_MODULE(drc_curve),
        M4_SIZE_EX(1,17),
        M4_RANGE_EX(0,1023),
        M4_DEFAULT([0, 64, 128, 192, 256, 320, 384, 448, 512, 576, 640, 704, 768, 832, 896, 960, 1023]),
        M4_DATAX([0, 64, 128, 192, 256, 320, 384, 448, 512, 576, 640, 704, 768, 832, 896, 960, 1024]),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The mapping curve after luma filter.\n
        Freq of use: low))  */
    uint16_t hw_dhazT_loLumaConvert_val[DHAZ_ENHANCE_CURVE_KNOTS_NUM];
    /* M4_GENERIC_DESC(
        M4_ALIAS(wgtMax),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_GROUP(dhazT_work_mode_group:dhaz_dehaze_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The weight max params of dehaze fuction))  */
    dhaz_dehaze_wgtMax_dyn_t dehaze_wgtMax;
    /* M4_GENERIC_DESC(
        M4_ALIAS(airLight),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_GROUP(dhazT_work_mode_group:dhaz_dehaze_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The air light params of dehaze fuction))  */
    dhaz_dehaze_airLight_dyn_t dehaze_airLight;
    /* M4_GENERIC_DESC(
        M4_ALIAS(transRatMin),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_GROUP(dhazT_work_mode_group:dhaz_dehaze_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The trans params of dehaze fuction))  */
    dhaz_dehaze_transRatMin_dyn_t dehaze_transRatMin;
    /* M4_GENERIC_DESC(
        M4_ALIAS(enhance),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_GROUP(dhazT_work_mode_group:dhaz_enhance_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The dynamic params of enhance fuction))  */
    dhaz_enhance_dyn_t enhance;
    /* M4_GENERIC_DESC(
        M4_ALIAS(bifilt),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The bifilter params for both dehaze and enhance fuction))  */
    dhaz_bifilt_dyn_t bifilt;
    /* M4_GENERIC_DESC(
        M4_ALIAS(iir),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The iir params for both dehaze fuction))  */
    dhaz_dehaze_iir_t iir;
} dehaze_params_dyn_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(dynamic_param),
        M4_TYPE(struct),
        M4_UI_MODULE(dynamic_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(All dynamic params array corresponded with iso array))  */
    dehaze_params_dyn_t dyn;
} dehaze_param_t;

#endif
