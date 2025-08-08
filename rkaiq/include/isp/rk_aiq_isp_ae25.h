/*
 * rk_aiq_isp_ae25.h
 *
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
#ifndef _RK_AIQ_PARAM_AE25_H_
#define _RK_AIQ_PARAM_AE25_H_

RKAIQ_BEGIN_DECLARE

/***************************  ATTENTION:  ************************************/
/**
 * @brief   AEC Params for ISP3.9 use New Rule (isp_para_style.txt)
 */
/*****************************************************************************/
#define AE_HDRFRAME_MAX_NUM     (3)
#define AE_STRUCT_MAX_NUM       (6)
#define AE_ROUTE_DOT_MAX_NUM    (12)
#define AE_GRIDWEIGHT_MAX_NUM   (225)
#define AE_HDCIRIS_DOT_MAX_NUM  (256)
#define AE_PIRIS_TABLE_MAX_STEP (256)

/*****************************************************************************/
/**
 * @brief   AEC Params: AEC enum Params
 */
/*****************************************************************************/
typedef enum ae_opt_mode_e {
    ae_opt_auto_mode = 1,
    ae_opt_manual_mode = 2,
} ae_opt_mode_t;

typedef enum ae_strategy_mode_e {
    ae_strategy_lowlight_mode = 0,
    ae_strategy_highlight_mode = 1,
} ae_strategy_mode_t;

typedef enum ae_delay_mode_e {
    ae_delay_frame_mode = 0,
    ae_delay_time_mode = 1,
} ae_delay_mode_t;

typedef enum ae_longFrm_mode_e {
    ae_longFrm_disable_mode = 0,
    ae_longFrm_auto_mode = 1,
    ae_longFrm_enable_mode = 2,
} ae_longFrm_mode_t;

typedef enum ae_expRatio_mode_e {
    ae_expRatio_auto_mode = 0,
    ae_expRatio_fix_mode = 1,
} ae_expRatio_mode_t;

typedef enum ae_measArea_mode_e {
    ae_measArea_auto_mode = 0,
    ae_measArea_up_mode = 1,
    ae_measArea_bottom_mode = 2,
    ae_measArea_left_mode = 3,
    ae_measArea_right_mode = 4,
    ae_measArea_center_mode = 5,
} ae_measArea_mode_t;

typedef enum ae_antiFlicker_freq_e {
    ae_antiFlicker_off_freq = 0,
    ae_antiFlicker_50hz_freq = 1,
    ae_antiFlicker_60hz_freq = 2,
    ae_antiFlicker_auto_freq = 3,
} ae_antiFlicker_freq_t;

typedef enum ae_antiFlicker_mode_e {
    ae_antiFlicker_auto_mode = 0,
    ae_antiFlicker_normal_mode = 1,
} ae_antiFlicker_mode_t;

typedef enum ae_frmRate_mode_e {
    ae_frmRate_auto_mode = 0,
    ae_frmRate_fix_mode = 1,
} ae_frmRate_mode_t;

typedef enum ae_iris_type_e {
    ae_iris_dc_type = 0,
    ae_iris_p_type = 1,
    ae_iris_hdc_type = 2,
} ae_iris_type_t;

typedef enum ae_yRange_mode_e {
    ae_yRange_full_mode = 1,
    ae_yRange_limited_mode = 2,
} ae_yRange_mode_t;

typedef enum ae_rawStats_mode_e {
    ae_rawStats_r_mode = 1,
    ae_rawStats_g_mode = 2,
    ae_rawStats_b_mode = 3,
    ae_rawStats_y_mode = 4,
} ae_rawStats_mode_t;

typedef enum ae_histStats_mode_e {
    ae_histStats_r_mode = 2,
    ae_histStats_g_mode = 3,
    ae_histStats_b_mode = 4,
    ae_histStats_y_mode = 5,
} ae_histStats_mode_t;

/*****************************************************************************/
/**
 * @brief   AEC Params: AEC algo CommonCtrl Config Params
 */
/*****************************************************************************/
typedef struct ae_delay_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_delay_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(ae_delay_mode_t),
        M4_DEFAULT(ae_delay_frame_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The working mode of ae delay.\n
        Freq of use: high))  */
    ae_delay_mode_t sw_aeT_delay_mode;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_blackDelay_val),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The delay value used in dark environment.\n
        Freq of use: high))  */
    uint8_t sw_aeT_blackDelay_val;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_whiteDelay_val),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The delay value used in bright environment.\n
        Freq of use: high))  */
    uint8_t sw_aeT_whiteDelay_val;
} ae_delay_t;

typedef struct ae_dynDamp_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_dynDamp_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The enable bit of dynamic damp control.\n
        Freq of use: high))  */
    bool sw_aeT_dynDamp_en;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_slowOpt_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(ae_opt_mode_t),
        M4_DEFAULT(ae_opt_auto_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
       M4_GROUP_CTRL(slow_mode_group),
        M4_NOTES(The working mode of slow speed control.\n
        Freq of use: low))  */
    ae_opt_mode_t sw_aeT_slowOpt_mode;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_slowRange_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,100),
        M4_DEFAULT(15),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
       M4_GROUP(slow_mode_group:ae_opt_manual_mode),
        M4_NOTES(The range value for running slow speed control.\n
        Freq of use: low))  */
    float sw_aeT_slowRange_val;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_slowDamp_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0.95),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
       M4_GROUP(slow_mode_group:ae_opt_manual_mode),
        M4_NOTES(The damp value for slow speed control.\n
        Freq of use: low))  */
    float sw_aeT_slowDamp_val;
} ae_dynDamp_t;

typedef struct ae_speed_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_smooth_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The enable bit of ae smooth.\n
        Freq of use: high))  */
    bool sw_aeT_smooth_en;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_damp_under),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0.45),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The damp value for dark environment used in speed control.\n
        Freq of use: high))  */
    float sw_aeT_damp_under;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_damp_over),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0.15),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The damp value for bright environment used in speed control.\n
        Freq of use: high))  */
    float sw_aeT_damp_over;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_damp_dark2Bright),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0.15),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(The damp value for dark2bright environment used in speed control.\n
        Freq of use: low))  */
    float sw_aeT_damp_dark2Bright;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_damp_bright2Dark),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0.45),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(4),
        M4_NOTES(The damp value for bright2dark environment used in speed control.\n
        Freq of use: low))  */
    float sw_aeT_damp_bright2Dark;

    /* M4_GENERIC_DESC(
        M4_ALIAS(dynDamp),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(5),
        M4_NOTES(The params for slow speed control.))  */
    ae_dynDamp_t dynDamp;
} ae_speed_t;

typedef struct ae_frmRate_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_frmRate_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(ae_frmRate_mode_t),
        M4_DEFAULT(ae_frmRate_fix_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The working mode of frame rate control.\n
        Freq of use: high))  */
    ae_frmRate_mode_t sw_aeT_frmRate_mode;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_frmRate_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(0),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The value of frame rate.\n
        Freq of use: high))  */
    float sw_aeT_frmRate_val;
} ae_frmRate_t;

typedef struct ae_antiFlicker_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_antiFlicker_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
       M4_GROUP_CTRL(anti_flicker_group),
        M4_NOTES(The enable bit of anti-flicker.\n
        Freq of use: high))  */
    bool sw_aeT_antiFlicker_en;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_antiFlicker_freq),
        M4_TYPE(enum),
        M4_ENUM_DEF(ae_antiFlicker_freq_t),
        M4_DEFAULT(ae_antiFlicker_50hz_freq),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
       M4_GROUP(anti_flicker_group),
        M4_NOTES(The frequence of flicker.\n
        Freq of use: high))  */
    ae_antiFlicker_freq_t sw_aeT_antiFlicker_freq;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_antiFlicker_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(ae_antiFlicker_mode_t),
        M4_DEFAULT(ae_antiFlicker_auto_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
       M4_GROUP(anti_flicker_group),
        M4_NOTES(The working mode of anti-flicker.\n
        Freq of use: high))  */
    ae_antiFlicker_mode_t sw_aeT_antiFlicker_mode;
} ae_antiFlicker_t;

typedef struct ae_deMotionBlur_strg_s {
    /* M4_GENERIC_DESC(
            M4_ALIAS(sw_aeT_dyStrg_len),
            M4_TYPE(u8),
            M4_SIZE_EX(1,1),
            M4_RANGE_EX(1,12),
            M4_DEFAULT(6),
            M4_DIGIT_EX(0),
            M4_HIDE_EX(0),
            M4_RO(0),
            M4_ORDER(0),
            M4_NOTES(The actual length of dot used.\n
            Freq of use: low))  */
    uint8_t sw_aeT_dyStrg_len;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_expLevel_dot),
        M4_TYPE(f32),
        M4_SIZE_EX(1,12),
        M4_RANGE_EX(0,300),
        M4_DEFAULT(0),
        M4_DIGIT_EX(6),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The dot of exposure level = sum of exposure value.\n
        Freq of use: low))  */
    float sw_aeT_expLevel_dot[AE_ROUTE_DOT_MAX_NUM];
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_deBlurScale_dot),
        M4_TYPE(f32),
        M4_SIZE_EX(1,12),
        M4_RANGE_EX(0,5),
        M4_DEFAULT(1),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The dot of dynamic scale coeff.\n
        Freq of use: low))  */
    float sw_aeT_deBlurScale_dot[AE_ROUTE_DOT_MAX_NUM];
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_deBlurMinLim_dot),
        M4_TYPE(f32),
        M4_SIZE_EX(1,12),
        M4_RANGE_EX(1,200),
        M4_DEFAULT(10),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The dot of time min-limit.\n
        Freq of use: low))  */
    float sw_aeT_deBlurMinLim_dot[AE_ROUTE_DOT_MAX_NUM];
} ae_deMotionBlur_strg_t;

typedef struct ae_deMotionBlur_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_deMotionBlur_en),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP_CTRL(de_motionblur_group),
        M4_NOTES(The enable bit of de-MotionBlur control.\n
        Freq of use: high))  */
    bool sw_aeT_deMotionBlur_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(dyStrg),
        M4_TYPE(struct),
        M4_UI_MODULE(array_table_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_GROUP(de_motionblur_group),
           M4_NOTES(The strength params for de-MotionBlur control.))  */
    ae_deMotionBlur_strg_t dyStrg;
} ae_deMotionBlur_t;


// manual exposure
typedef struct ae_linMe_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_manTime_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
       M4_GROUP_CTRL(man_time_group),
        M4_NOTES(The enable bit of exposure time manual control.\n
        Freq of use: high))  */
    bool sw_aeT_manTime_en;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_manGain_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_GROUP_CTRL(man_gain_group),
        M4_NOTES(The enable bit of exposure gain manual control.\n
        Freq of use: high))  */
    bool sw_aeT_manGain_en;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_manIspDGain_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_GROUP_CTRL(man_ispG_group),
        M4_NOTES(The enable bit of isp dgain manual control.\n
        Freq of use: high))  */
    bool sw_aeT_manIspDGain_en;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_manTime_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,5),
        M4_DEFAULT(0.003),
        M4_DIGIT_EX(6),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_GROUP(man_time_group),
        M4_NOTES(The value of exposure time.\n
        Freq of use: high))  */
    float sw_aeT_manTime_val;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_manGain_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,4096),
        M4_DEFAULT(1),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(4),
        M4_GROUP(man_gain_group),
        M4_NOTES(The value of exposure gain.\n
        Freq of use: high))  */
    float sw_aeT_manGain_val;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_manIspDGain_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,32),
        M4_DEFAULT(1),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(5),
        M4_GROUP(man_ispG_group),
        M4_NOTES(The value of isp dgain.\n
        Freq of use: high))  */
    float sw_aeT_manIspDGain_val;
} ae_linMe_t;

typedef struct ae_hdrMe_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_manTime_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP_CTRL(man_time_hdr_group),
        M4_NOTES(The enable bit of exposure time manual control.\n
        Freq of use: high))  */
    bool sw_aeT_manTime_en;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_manGain_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_GROUP_CTRL(man_gain_hdr_group),
        M4_NOTES(The enable bit of exposure gain manual control.\n
        Freq of use: high))  */
    bool sw_aeT_manGain_en;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_manIspDGain_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_GROUP_CTRL(man_ispG_hdr_group),
        M4_NOTES(The enable bit of isp dgain manual control.\n
        Freq of use: high))  */
    bool sw_aeT_manIspDGain_en;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_manTime_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,3),
        M4_RANGE_EX(0,5),
        M4_DEFAULT(0.003),
        M4_DIGIT_EX(6),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_GROUP(man_time_hdr_group),
        M4_MARK(2frm:S/L/-,3frm:S/M/L),
        M4_NOTES(The value of exposure time.\n
        Freq of use: high))  */
    float sw_aeT_manTime_val[AE_HDRFRAME_MAX_NUM];

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_manGain_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,3),
        M4_RANGE_EX(1,4096),
        M4_DEFAULT(1),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(4),
        M4_GROUP(man_gain_hdr_group),
        M4_MARK(2frm:S/L/-,3frm:S/M/L),
        M4_NOTES(The value of exposure gain.\n
        Freq of use: high))  */
    float sw_aeT_manGain_val[AE_HDRFRAME_MAX_NUM];

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_manIspDGain_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,3),
        M4_RANGE_EX(1,32),
        M4_DEFAULT(1),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(5),
        M4_GROUP(man_ispG_hdr_group),
        M4_MARK(2frm:S/L/-,3frm:S/M/L),
        M4_NOTES(The value of isp dgain.\n
        Freq of use: high))  */
    float sw_aeT_manIspDGain_val[AE_HDRFRAME_MAX_NUM];
} ae_hdrMe_t;

typedef struct ae_meCtrl_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(linMe),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The params for manual linear ae control.))  */
    ae_linMe_t linMe;

    /* M4_GENERIC_DESC(
        M4_ALIAS(hdrMe),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The params for manual hdr ae control.))  */
    ae_hdrMe_t hdrMe;
} ae_meCtrl_t;

typedef struct ae_winRatio_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_aeCfg_win_x),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The horizontal offset of ae stats win.\n
        Freq of use: low))  */
    float hw_aeCfg_win_x;

    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_aeCfg_win_y),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The vertical offset of ae stats win.\n
        Freq of use: low))  */
    float hw_aeCfg_win_y;

    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_aeCfg_win_width),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(1),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The width of ae stats win, 1 = horizontal resolution.\n
        Freq of use: low))  */
    float hw_aeCfg_win_width;

    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_aeCfg_win_height),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(1),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(The height of ae stats win, 1 = vertical resolution.\n
        Freq of use: low))  */
    float hw_aeCfg_win_height;
} ae_winRatio_t;

typedef struct ae_winScale_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(inRawWinScale),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The windows scale for input raw.))  */
    ae_winRatio_t inRawWinScale;

    /* M4_GENERIC_DESC(
        M4_ALIAS(tmoRawWinScale),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The windows scale after tmo.))  */
    ae_winRatio_t tmoRawWinScale;

    /* M4_GENERIC_DESC(
        M4_ALIAS(yuvWinScale),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The windows scale for yuv.))  */
    ae_winRatio_t yuvWinScale;
} ae_winScale_t;

typedef struct ae_envLvCalib_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_envCalib_en),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The enable bit of environment luma calibration (no used yet).\n
        Freq of use: low))  */
    bool sw_aeT_envCalib_en;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeC_envCalib_Fn),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(1.6),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The F number of lens used in environment calibration.\n
        Freq of use: low))  */
    float sw_aeC_envCalib_Fn;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeC_envCalib_coeff),
        M4_TYPE(f32),
        M4_SIZE_EX(1,2),
        M4_RANGE_EX(0,1024),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(5),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The coefficient used to refer to current luminance.\n
        Freq of use: low))  */
    float sw_aeC_envCalib_coeff[2];
} ae_envLvCalib_t;

typedef struct ae_commCtrl_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_algo_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The enable bit of ae algorithm.\n
        Freq of use: low))  */
    bool sw_aeT_algo_en;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_algo_interval),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The running intervel of ae algorithm.\n
        Freq of use: low))  */
    uint8_t sw_aeT_algo_interval;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_opt_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(rk_aiq_op_mode_t),
        M4_DEFAULT(RK_AIQ_OP_MODE_AUTO),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
       M4_GROUP_CTRL(op_mode_group),
        M4_NOTES(The working mode of ae algorithm.\n
        Freq of use: high))  */
    rk_aiq_op_mode_t sw_aeT_opt_mode;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_histStats_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(ae_histStats_mode_t),
        M4_DEFAULT(ae_histStats_y_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(The channel mode of histogram statistics.\n
        Freq of use: low))  */
    ae_histStats_mode_t sw_aeT_histStats_mode;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_rawStats_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(ae_rawStats_mode_t),
        M4_DEFAULT(ae_rawStats_y_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(4),
        M4_NOTES(The channel mode of raw luma statistics.\n
        Freq of use: low))  */
    ae_rawStats_mode_t sw_aeT_rawStats_mode;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_yRange_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(ae_yRange_mode_t),
        M4_DEFAULT(ae_yRange_full_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(5),
        M4_NOTES(The range mode of Y calculation.\n
        Freq of use: low))  */
    ae_yRange_mode_t sw_aeT_yRange_mode;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_grid_wgt),
        M4_TYPE(u8),
        M4_SIZE_EX(15,15),
        M4_RANGE_EX(0,32),
        M4_DEFAULT(1),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(6),
        M4_UI_MODULE(grid_weight_table),
        M4_NOTES(The grid weight used in ae algorithm.\n
        Freq of use: high))  */
    uint8_t sw_aeT_grid_wgt[AE_GRIDWEIGHT_MAX_NUM];

    /* M4_GENERIC_DESC(
        M4_ALIAS(meCtrl),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(7),
        M4_GROUP(op_mode_group:RK_AIQ_OP_MODE_MANUAL),
        M4_NOTES(The params for manual ae control.))  */
    ae_meCtrl_t meCtrl;

    /* M4_GENERIC_DESC(
        M4_ALIAS(speed),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(8),
        M4_NOTES(The params for ae speed control.))  */
    ae_speed_t speed;

    /* M4_GENERIC_DESC(
        M4_ALIAS(delay),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(9),
        M4_NOTES(The params for ae delay control.))  */
    ae_delay_t delay;

    /* M4_GENERIC_DESC(
        M4_ALIAS(frmRate),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(10),
        M4_NOTES(The params for frame rate control.))  */
    ae_frmRate_t frmRate;

    /* M4_GENERIC_DESC(
        M4_ALIAS(antiFlicker),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(11),
        M4_NOTES(The params for anti-flicker control.))  */
    ae_antiFlicker_t antiFlicker;

    /* M4_GENERIC_DESC(
        M4_ALIAS(deMotionBlur),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(12),
        M4_NOTES(The params for de-MotionBlur control.))  */
    ae_deMotionBlur_t deMotionBlur;

    /* M4_GENERIC_DESC(
        M4_ALIAS(envLvCalib),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(13),
        M4_NOTES(The params for environment luma calibration (no used yet).\n
        Freq of use: low))  */
    ae_envLvCalib_t envLvCalib;

    /* M4_GENERIC_DESC(
        M4_ALIAS(winScale),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(14),
        M4_NOTES(The params for windows scale.\n
        Freq of use: low))  */
    ae_winScale_t winScale;
} ae_commCtrl_t;

/*****************************************************************************/
/**
 * @brief  AEC Params: AEC algo LinAE Config Params
 */
/*****************************************************************************/
typedef struct ae_linInitExp_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_initTime_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,5),
        M4_DEFAULT(0.003),
        M4_DIGIT_EX(6),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The initial exposure time value.\n
        Freq of use: high))  */
    float sw_aeT_initTime_val;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_initGain_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,4096),
        M4_DEFAULT(1),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The initial exposure gain value.\n
        Freq of use: high))  */
    float sw_aeT_initGain_val;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_initIspDGain_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,32),
        M4_DEFAULT(1),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The initial isp dgain value.\n
        Freq of use: high))  */
    float sw_aeT_initIspDGain_val;
} ae_linInitExp_t;

typedef struct ae_linRoute_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_route_len),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,12),
        M4_DEFAULT(6),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The actual length of dot used.\n
        Freq of use: high))  */
    uint8_t sw_aeT_route_len;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_time_dot),
        M4_TYPE(f32),
        M4_SIZE_EX(1,12),
        M4_RANGE_EX(0,5),
        M4_DEFAULT(0.003),
        M4_DIGIT_EX(6),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The dot of exposure time value.\n
        Freq of use: high))  */
    float sw_aeT_time_dot[AE_ROUTE_DOT_MAX_NUM];

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_gain_dot),
        M4_TYPE(f32),
        M4_SIZE_EX(1,12),
        M4_RANGE_EX(1,4096),
        M4_DEFAULT(1),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The dot of exposure gain value.\n
        Freq of use: high))  */
    float sw_aeT_gain_dot[AE_ROUTE_DOT_MAX_NUM];

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_ispDGain_dot),
        M4_TYPE(f32),
        M4_SIZE_EX(1,12),
        M4_RANGE_EX(1,32),
        M4_DEFAULT(1),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(The dot of isp dgain value.\n
        Freq of use: high))  */
    float sw_aeT_ispDGain_dot[AE_ROUTE_DOT_MAX_NUM];

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_pIrisGain_dot),
        M4_TYPE(s32),
        M4_SIZE_EX(1,12),
        M4_RANGE_EX(1,1024),
        M4_DEFAULT(512),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(4),
        M4_NOTES(The dot of piris gain value.\n
        Freq of use: high))  */
    int sw_aeT_pIrisGain_dot[AE_ROUTE_DOT_MAX_NUM];
} ae_linRoute_t;

typedef struct ae_dynSetpoint_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_dynSetpoint_len),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,12),
        M4_DEFAULT(6),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The actual length of dot used.\n
        Freq of use: high))  */
    uint8_t sw_aeT_dynSetpoint_len;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_expLevel_dot),
        M4_TYPE(f32),
        M4_SIZE_EX(1,12),
        M4_RANGE_EX(0,300),
        M4_DEFAULT(0),
        M4_DIGIT_EX(6),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The dot of exposure level = sum of exposure value.\n
        Freq of use: high))  */
    float sw_aeT_expLevel_dot[AE_ROUTE_DOT_MAX_NUM];

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_dynSetpoint_dot),
        M4_TYPE(f32),
        M4_SIZE_EX(1,12),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(40),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The dot of dynamic setpoint.\n
        Freq of use: high))  */
    float sw_aeT_dynSetpoint_dot[AE_ROUTE_DOT_MAX_NUM];
} ae_dynSetpoint_t;

typedef struct ae_backLitSetpoint_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_backLitSetpoint_len),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,12),
        M4_DEFAULT(6),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The actual length of dot used.\n
        Freq of use: high))  */
    uint8_t sw_aeT_backLitSetpoint_len;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_expLevel_dot),
        M4_TYPE(f32),
        M4_SIZE_EX(1,12),
        M4_RANGE_EX(0,300),
        M4_DEFAULT(0),
        M4_DIGIT_EX(6),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The dot of exposure level = sum of exposure value.\n
        Freq of use: high))  */
    float sw_aeT_expLevel_dot[AE_ROUTE_DOT_MAX_NUM];

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_nonOEPdfTh_dot),
        M4_TYPE(f32),
        M4_SIZE_EX(1,12),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0.4),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The threshold dot of non over-exposured pdf.\n
        Freq of use: low))  */
    float sw_aeT_nonOEPdfTh_dot[AE_ROUTE_DOT_MAX_NUM];

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_loLitPdfTh_dot),
        M4_TYPE(f32),
        M4_SIZE_EX(1,12),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0.2),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(The threshold dot of lowlight pdf.\n
        Freq of use: low))  */
    float sw_aeT_loLitPdfTh_dot[AE_ROUTE_DOT_MAX_NUM];

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_loLitSetpoint_dot),
        M4_TYPE(f32),
        M4_SIZE_EX(1,12),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(20),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(1),
        M4_RO(0),
        M4_ORDER(4),
        M4_NOTES(The setpoint dot of lowlight area.\n
        Freq of use: high))  */
    float sw_aeT_loLitSetpoint_dot[AE_ROUTE_DOT_MAX_NUM];
} ae_backLitSetpoint_t;

typedef struct ae_backLitCtrl_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_backLit_en),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP_CTRL(back_lit_group),
        M4_NOTES(The enable bit of backlight control.\n
        Freq of use: high))  */
    bool sw_aeT_backLit_en;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_backLitBias_strg),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(-500,500),
        M4_DEFAULT(0),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(1),
        M4_RO(0),
        M4_ORDER(1),
        M4_GROUP(back_lit_group),
        M4_NOTES(The strength bias of back-lit control.\n
        Freq of use: high))  */
    float sw_aeT_backLitBias_strg;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_measArea_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(ae_measArea_mode_t),
        M4_DEFAULT(ae_measArea_auto_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_GROUP(back_lit_group),
        M4_NOTES(The measuring mode of lowlight area in back-lit control.\n
        Freq of use: high))  */
    ae_measArea_mode_t sw_aeT_measArea_mode;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_oeROILow_thred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(150),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_GROUP(back_lit_group),
        M4_NOTES(The luma threshold of over-exposure ROI.\n
        Freq of use: high))  */
    float sw_aeT_oeROILow_thred;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_lumaDist_thred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,100),
        M4_DEFAULT(10),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(4),
        M4_GROUP(back_lit_group),
        M4_NOTES(The luma distance threshhold used in backlit control.\n
        Freq of use: low))  */
    float sw_aeT_lumaDist_thred;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_loLv_thred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,100),
        M4_DEFAULT(0.3125),
        M4_DIGIT_EX(4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(5),
        M4_GROUP(back_lit_group),
        M4_NOTES(The threshold of lowlight luminance value.\n
        Freq of use: high))  */
    float sw_aeT_loLv_thred;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_hiLv_thred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,100),
        M4_DEFAULT(7.5),
        M4_DIGIT_EX(4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(6),
        M4_GROUP(back_lit_group),
        M4_NOTES(The threshold of highlight luminance value.\n
        Freq of use: high))  */
    float sw_aeT_hiLv_thred;

    /* M4_GENERIC_DESC(
        M4_ALIAS(backLitSetpoint),
        M4_TYPE(struct),
        M4_UI_MODULE(array_table_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(7),
        M4_GROUP(back_lit_group),
        M4_NOTES(The setpoint params for back-lit control.))  */
    ae_backLitSetpoint_t backLitSetpoint;
} ae_backLitCtrl_t;

typedef struct ae_overExpSetpoint_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_overExpSetpoint_len),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,12),
        M4_DEFAULT(6),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The actual length of dot used.\n
        Freq of use: high))  */
    uint8_t sw_aeT_overExpSetpoint_len;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_oePdf_dot),
        M4_TYPE(f32),
        M4_SIZE_EX(1,12),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The dot params of over-exposure pdf.\n
        Freq of use: high))  */
    float sw_aeT_oePdf_dot[AE_ROUTE_DOT_MAX_NUM];

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_loLitWgt_dot),
        M4_TYPE(f32),
        M4_SIZE_EX(1,12),
        M4_RANGE_EX(0,20),
        M4_DEFAULT(1),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The dot params of weight for lowlight .\n
        Freq of use: high))  */
    float sw_aeT_loLitWgt_dot[AE_ROUTE_DOT_MAX_NUM];

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_hiLitWgt_dot),
        M4_TYPE(f32),
        M4_SIZE_EX(1,12),
        M4_RANGE_EX(0,20),
        M4_DEFAULT(2),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(The dot params of weight for highlight .\n
        Freq of use: high))  */
    float sw_aeT_hiLitWgt_dot[AE_ROUTE_DOT_MAX_NUM];
} ae_overExpSetpoint_t;

typedef struct ae_overExpCtrl_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_overExp_en),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP_CTRL(over_exp_group),
        M4_NOTES(The enable bit of over-exposure control.\n
        Freq of use: high))  */
    bool sw_aeT_overExp_en;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_overExpBias_strg),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(-500,500),
        M4_DEFAULT(0),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(1),
        M4_RO(0),
        M4_ORDER(1),
        M4_GROUP(over_exp_group),
        M4_NOTES(The strength bias of over-exposure control.\n
        Freq of use: high))  */
    float sw_aeT_overExpBias_strg;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_overExpWgt_max),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,5),
        M4_DEFAULT(2),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_GROUP(over_exp_group),
        M4_NOTES(The max weight used in over-exposure control to limit strength.\n
        Freq of use: high))  */
    float sw_aeT_overExpWgt_max;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_loLit_thred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(30),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_GROUP(over_exp_group),
        M4_NOTES(The threshold to judging lowlight area.\n
        Freq of use: high))  */
    float sw_aeT_loLit_thred;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_hiLit_thred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(150),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(4),
        M4_GROUP(over_exp_group),
        M4_NOTES(The threshold to judging highlight area.\n
        Freq of use: high))  */
    float sw_aeT_hiLit_thred;

    /* M4_GENERIC_DESC(
        M4_ALIAS(overExpSetpoint),
        M4_TYPE(struct),
        M4_UI_MODULE(array_table_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(5),
        M4_GROUP(over_exp_group),
        M4_NOTES(The setpoint params for over-exposure control.))  */
    ae_overExpSetpoint_t overExpSetpoint;
} ae_overExpCtrl_t;

typedef struct ae_linAeCtrl_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_tolerance_in),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,100),
        M4_DEFAULT(10),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The tolerance of luma before converged.\n
        Freq of use: high))  */
    float sw_aeT_tolerance_in;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_tolerance_out),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,100),
        M4_DEFAULT(15),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The tolerance of luma after converged.\n
        Freq of use: high))  */
    float sw_aeT_tolerance_out;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_evBias_strg),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(-500,500),
        M4_DEFAULT(0),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(1),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The strength bias of linear ae setpoint control.\n
        Freq of use: high))  */
    float sw_aeT_evBias_strg;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_strategy_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(ae_strategy_mode_t),
        M4_DEFAULT(ae_strategy_lowlight_mode),
        M4_HIDE_EX(1),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(The working mode of exposure strategy.\n
        Freq of use: low))  */
    ae_strategy_mode_t sw_aeT_strategy_mode;

    /* M4_GENERIC_DESC(
        M4_ALIAS(initExp),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(4),
        M4_NOTES(The params for initial exposure value.))  */
    ae_linInitExp_t initExp;

    /* M4_GENERIC_DESC(
        M4_ALIAS(route),
        M4_TYPE(struct),
        M4_UI_MODULE(array_table_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(5),
        M4_NOTES(The params for exposure route setting.))  */
    ae_linRoute_t route;

    /* M4_GENERIC_DESC(
        M4_ALIAS(dynSetpoint),
        M4_TYPE(struct),
        M4_UI_MODULE(array_table_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(6),
        M4_NOTES(The params for dynamic setpoint control.))  */
    ae_dynSetpoint_t dynSetpoint;

    /* M4_GENERIC_DESC(
        M4_ALIAS(backLightCtrl),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(7),
        M4_NOTES(The params for back-light control.))  */
    ae_backLitCtrl_t backLightCtrl;

    /* M4_GENERIC_DESC(
        M4_ALIAS(overExpCtrl),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(8),
        M4_NOTES(The params for over exposure control.))  */
    ae_overExpCtrl_t overExpCtrl;
} ae_linAeCtrl_t;

/*****************************************************************************/
/**
 * @brief   AEC Params: AEC algo HdrAE Config Params
 */
/*****************************************************************************/
typedef struct ae_hdrInitExp_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_initTime_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,3),
        M4_RANGE_EX(0,5),
        M4_DEFAULT(0.003),
        M4_DIGIT_EX(6),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The initial exposure time value.\n
        Freq of use: high))  */
    float sw_aeT_initTime_val[AE_HDRFRAME_MAX_NUM];

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_initGain_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,3),
        M4_RANGE_EX(1,4096),
        M4_DEFAULT(1),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The initial exposure gain value.\n
        Freq of use: high))  */
    float sw_aeT_initGain_val[AE_HDRFRAME_MAX_NUM];

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_initIspDGain_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,3),
        M4_RANGE_EX(1,32),
        M4_DEFAULT(1),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The initial isp dgain value.\n
        Freq of use: high))  */
    float sw_aeT_initIspDGain_val[AE_HDRFRAME_MAX_NUM];
} ae_hdrInitExp_t;

typedef struct ae_hdrExpRatio_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_expRatio_len),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,12),
        M4_DEFAULT(6),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The actual length of dot used.\n
        Freq of use: high))  */
    uint8_t sw_aeT_expRatio_len;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_expLevel_dot),
        M4_TYPE(f32),
        M4_SIZE_EX(1,12),
        M4_RANGE_EX(0,300),
        M4_DEFAULT(0),
        M4_DIGIT_EX(6),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The dot of exposure level = sum of exposure value.\n
        Freq of use: high))  */
    float sw_aeT_expLevel_dot[AE_ROUTE_DOT_MAX_NUM];

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_m2sRatioFix_dot),
        M4_TYPE(f32),
        M4_SIZE_EX(1,12),
        M4_RANGE_EX(1,256),
        M4_DEFAULT(16),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_GROUP(exp_ratio_group:ae_expRatio_fix_mode),
        M4_NOTES(The dot of fixed exp-ratio between middle and short frame.\n
        Freq of use: high))  */
    float sw_aeT_m2sRatioFix_dot[AE_ROUTE_DOT_MAX_NUM];

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_l2mRatioFix_dot),
        M4_TYPE(f32),
        M4_SIZE_EX(1,12),
        M4_RANGE_EX(1,256),
        M4_DEFAULT(16),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_GROUP(exp_ratio_group:ae_expRatio_fix_mode),
        M4_NOTES(The dot of fixed exp-ratio between long and middle frame.\n
        Freq of use: high))  */
    float sw_aeT_l2mRatioFix_dot[AE_ROUTE_DOT_MAX_NUM];

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_m2sRatioMax_dot),
        M4_TYPE(f32),
        M4_SIZE_EX(1,12),
        M4_RANGE_EX(1,256),
        M4_DEFAULT(64),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(4),
        M4_GROUP(exp_ratio_group:ae_expRatio_auto_mode),
        M4_NOTES(The dot of max exp-ratio between middle and short frame.\n
        Freq of use: high))  */
    float sw_aeT_m2sRatioMax_dot[AE_ROUTE_DOT_MAX_NUM];

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_l2mRatioMax_dot),
        M4_TYPE(f32),
        M4_SIZE_EX(1,12),
        M4_RANGE_EX(1,256),
        M4_DEFAULT(64),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(5),
        M4_GROUP(exp_ratio_group:ae_expRatio_auto_mode),
        M4_NOTES(The dot of max exp-ratio between long and middle frame.\n
        Freq of use: high))  */
    float sw_aeT_l2mRatioMax_dot[AE_ROUTE_DOT_MAX_NUM];
} ae_hdrExpRatio_t;

typedef struct ae_hdrExpRatioCtrl_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_expRatio_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(ae_expRatio_mode_t),
        M4_DEFAULT(ae_expRatio_auto_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP_CTRL(exp_ratio_group),
        M4_NOTES(The working mode of exp-ratio in hdrae control.\n
        Freq of use: high))  */
    ae_expRatio_mode_t sw_aeT_expRatio_mode;

    /* M4_GENERIC_DESC(
        M4_ALIAS(expRatio),
        M4_TYPE(struct),
        M4_UI_MODULE(array_table_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The params for exp-ratio control.))  */
    ae_hdrExpRatio_t expRatio;
} ae_hdrExpRatioCtrl_t;

typedef struct ae_hdrRoute_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_route_len),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,12),
        M4_DEFAULT(6),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The actual length of dot used.\n
        Freq of use: high))  */
    uint8_t sw_aeT_route_len;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_frm0Time_dot),
        M4_TYPE(f32),
        M4_SIZE_EX(1,12),
        M4_RANGE_EX(0,5),
        M4_DEFAULT(0.003),
        M4_DIGIT_EX(6),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The dot of exposure time in frm0.\n
        Freq of use: high))  */
    float sw_aeT_frm0Time_dot[AE_ROUTE_DOT_MAX_NUM];

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_frm0Gain_dot),
        M4_TYPE(f32),
        M4_SIZE_EX(1,12),
        M4_RANGE_EX(1,4096),
        M4_DEFAULT(1),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The dot of exposure gain in frm0.\n
        Freq of use: high))  */
    float sw_aeT_frm0Gain_dot[AE_ROUTE_DOT_MAX_NUM];

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_frm0IspDGain_dot),
        M4_TYPE(f32),
        M4_SIZE_EX(1,12),
        M4_RANGE_EX(1,32),
        M4_DEFAULT(1),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(The dot of isp dgain in frm0.\n
        Freq of use: high))  */
    float sw_aeT_frm0IspDGain_dot[AE_ROUTE_DOT_MAX_NUM];

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_frm1Time_dot),
        M4_TYPE(f32),
        M4_SIZE_EX(1,12),
        M4_RANGE_EX(0,5),
        M4_DEFAULT(0.03),
        M4_DIGIT_EX(6),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(4),
        M4_NOTES(The dot of exposure time in frm1.\n
        Freq of use: high))  */
    float sw_aeT_frm1Time_dot[AE_ROUTE_DOT_MAX_NUM];

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_frm1Gain_dot),
        M4_TYPE(f32),
        M4_SIZE_EX(1,12),
        M4_RANGE_EX(1,4096),
        M4_DEFAULT(1),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(5),
        M4_NOTES(The dot of exposure gain in frm1.\n
        Freq of use: high))  */
    float sw_aeT_frm1Gain_dot[AE_ROUTE_DOT_MAX_NUM];

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_frm1IspDGain_dot),
        M4_TYPE(f32),
        M4_SIZE_EX(1,12),
        M4_RANGE_EX(1,32),
        M4_DEFAULT(1),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(6),
        M4_NOTES(The dot of isp dgain in frm1.\n
        Freq of use: high))  */
    float sw_aeT_frm1IspDGain_dot[AE_ROUTE_DOT_MAX_NUM];

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_frm2Time_dot),
        M4_TYPE(f32),
        M4_SIZE_EX(1,12),
        M4_RANGE_EX(0,5),
        M4_DEFAULT(0.03),
        M4_DIGIT_EX(6),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(7),
        M4_NOTES(The dot of exposure time in frm2.\n
        Freq of use: high))  */
    float sw_aeT_frm2Time_dot[AE_ROUTE_DOT_MAX_NUM];

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_frm2Gain_dot),
        M4_TYPE(f32),
        M4_SIZE_EX(1,12),
        M4_RANGE_EX(1,4096),
        M4_DEFAULT(1),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(8),
        M4_NOTES(The dot of exposure gain in frm2.\n
        Freq of use: high))  */
    float sw_aeT_frm2Gain_dot[AE_ROUTE_DOT_MAX_NUM];

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_frm2IspDGain_dot),
        M4_TYPE(f32),
        M4_SIZE_EX(1,12),
        M4_RANGE_EX(1,32),
        M4_DEFAULT(1),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(9),
        M4_NOTES(The dot of isp dgain in frm2.\n
        Freq of use: high))  */
    float sw_aeT_frm2IspDGain_dot[AE_ROUTE_DOT_MAX_NUM];

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_pIrisGain_dot),
        M4_TYPE(s32),
        M4_SIZE_EX(1,12),
        M4_RANGE_EX(1,1024),
        M4_DEFAULT(512),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(10),
        M4_NOTES(The dot of piris gain.\n
        Freq of use: low))  */
    int sw_aeT_pIrisGain_dot[AE_ROUTE_DOT_MAX_NUM];
} ae_hdrRoute_t;

typedef struct ae_hdrLfrmSetpoint_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_lfrmSetpoint_len),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,12),
        M4_DEFAULT(6),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The actual length of dot used.\n
        Freq of use: high))  */
    uint8_t sw_aeT_lfrmSetpoint_len;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_expLevel_dot),
        M4_TYPE(f32),
        M4_SIZE_EX(1,12),
        M4_RANGE_EX(0,300),
        M4_DEFAULT(0),
        M4_DIGIT_EX(6),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The dot of exposure level = sum of exposure value.\n
        Freq of use: high))  */
    float sw_aeT_expLevel_dot[AE_ROUTE_DOT_MAX_NUM];

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_nonOEPdfTh_dot),
        M4_TYPE(f32),
        M4_SIZE_EX(1,12),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0.4),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The dot of non-overexposure pdf threshold.\n
        Freq of use: low))  */
    float sw_aeT_nonOEPdfTh_dot[AE_ROUTE_DOT_MAX_NUM];

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_loLitPdfTh_dot),
        M4_TYPE(f32),
        M4_SIZE_EX(1,12),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0.2),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(The dot of lowlight pdf threshold.\n
        Freq of use: low))  */
    float sw_aeT_loLitPdfTh_dot[AE_ROUTE_DOT_MAX_NUM];

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_lfrmSetpoint_dot),
        M4_TYPE(f32),
        M4_SIZE_EX(1,12),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(40),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(4),
        M4_NOTES(The dot of long frame setpoint.\n
        Freq of use: high))  */
    float sw_aeT_lfrmSetpoint_dot[AE_ROUTE_DOT_MAX_NUM];

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_loLitSetpoint_dot),
        M4_TYPE(f32),
        M4_SIZE_EX(1,12),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(20),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(5),
        M4_NOTES(The dot of lowlight setpoint in longframe.\n
        Freq of use: high))  */
    float sw_aeT_loLitSetpoint_dot[AE_ROUTE_DOT_MAX_NUM];
} ae_hdrLfrmSetpoint_t;

typedef struct ae_hdrLfrmCtrl_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_oeROILow_thred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(150),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The threshold of over-exposure ROI.\n
        Freq of use: high))  */
    float sw_aeT_oeROILow_thred;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_loLv_thred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,100),
        M4_DEFAULT(0.3125),
        M4_DIGIT_EX(4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The threshold of lowlight luminance value.\n
        Freq of use: high))  */
    float sw_aeT_loLv_thred;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_hiLv_thred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,100),
        M4_DEFAULT(7.5),
        M4_DIGIT_EX(4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The threshold of highlight luminance value.\n
        Freq of use: high))  */
    float sw_aeT_hiLv_thred;

    /* M4_GENERIC_DESC(
        M4_ALIAS(lfrmSetpoint),
        M4_TYPE(struct),
        M4_UI_MODULE(array_table_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(The setpoint params for long frame.))  */
    ae_hdrLfrmSetpoint_t lfrmSetpoint;
} ae_hdrLfrmCtrl_t;

typedef struct ae_hdrMfrmCtrl_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_mfrmCtrl_len),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,12),
        M4_DEFAULT(6),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The actual length of dot used.\n
        Freq of use: high))  */
    uint8_t sw_aeT_mfrmCtrl_len;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_expLevel_dot),
        M4_TYPE(f32),
        M4_SIZE_EX(1,12),
        M4_RANGE_EX(0,300),
        M4_DEFAULT(0),
        M4_DIGIT_EX(6),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The dot of exposure level = sum of exposure value.\n
        Freq of use: high))  */
    float sw_aeT_expLevel_dot[AE_ROUTE_DOT_MAX_NUM];

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_mfrmSetpoint_dot),
        M4_TYPE(f32),
        M4_SIZE_EX(1,12),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(40),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The dot of setpoint for middle frame.\n
        Freq of use: high))  */
    float sw_aeT_mfrmSetpoint_dot[AE_ROUTE_DOT_MAX_NUM];
} ae_hdrMfrmCtrl_t;

typedef struct ae_hdrSfrmSetpoint_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_sfrmSetpoint_len),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,12),
        M4_DEFAULT(6),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The actual length of dot used.\n
        Freq of use: high))  */
    uint8_t sw_aeT_sfrmSetpoint_len;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_expLevel_dot),
        M4_TYPE(f32),
        M4_SIZE_EX(1,12),
        M4_RANGE_EX(0,300),
        M4_DEFAULT(0),
        M4_DIGIT_EX(6),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The dot of exposure level = sum of exposure value.\n
        Freq of use: high))  */
    float sw_aeT_expLevel_dot[AE_ROUTE_DOT_MAX_NUM];

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_sfrmSetpoint_dot),
        M4_TYPE(f32),
        M4_SIZE_EX(1,12),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(20),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The dot of setpoint for short frame.\n
        Freq of use: high))  */
    float sw_aeT_sfrmSetpoint_dot[AE_ROUTE_DOT_MAX_NUM];

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_hiLitSetpoint_dot),
        M4_TYPE(f32),
        M4_SIZE_EX(1,12),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(150),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(The dot of setpoint for highlight area in short frame.\n
        Freq of use: high))  */
    float sw_aeT_hiLitSetpoint_dot[AE_ROUTE_DOT_MAX_NUM];
} ae_hdrSfrmSetpoint_t;

typedef struct ae_hdrSfrmCtrl_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_hiLitROIExpd_en),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The enable bit of high light ROI expansion.\n
        Freq of use: low))  */
    bool sw_aeT_hiLitROIExpd_en;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_hiLit_tolerance),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,100),
        M4_DEFAULT(12),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The luma tolerance to judge highlight area.\n
        Freq of use: low))  */
    float sw_aeT_hiLit_tolerance;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sfrmSetpoint),
        M4_TYPE(struct),
        M4_UI_MODULE(array_table_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The setpoint params for short frame.))  */
    ae_hdrSfrmSetpoint_t sfrmSetpoint;
} ae_hdrSfrmCtrl_t;

typedef struct ae_hdrLfrmMode_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_lfrm_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(ae_longFrm_mode_t),
        M4_DEFAULT(ae_longFrm_disable_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP_CTRL(lfrm_mode_group),
        M4_NOTES(The working mode of longframe mode.\n
        Freq of use: low))  */
    ae_longFrm_mode_t sw_aeT_lfrm_mode;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_sfrmTimeReg_min),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1024),
        M4_DEFAULT(2),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_GROUP(lfrm_mode_group:ae_longFrm_auto_mode),
        M4_NOTES(The min exposure time register value for short frame.\n
        Freq of use: low))  */
    uint16_t sw_aeT_sfrmTimeReg_min;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_lfrmModeExp_thred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,10),
        M4_DEFAULT(0.62),
        M4_DIGIT_EX(4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_GROUP(lfrm_mode_group:ae_longFrm_auto_mode),
        M4_NOTES(The expsure value of lfm to switch to lfrmmode, \n
        only valid when sw_aeT_lfrm_mode = ae_longFrm_auto_mode .\n
        Freq of use: low))  */
    float sw_aeT_lfrmModeExp_thred;
} ae_hdrLfrmMode_t;

typedef struct ae_hdrAeCtrl_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_tolerance_in),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,100),
        M4_DEFAULT(10),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The tolerance of luma before converged.\n
        Freq of use: high))  */
    float sw_aeT_tolerance_in;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_tolerance_out),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,100),
        M4_DEFAULT(15),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The tolerance of luma after converged.\n
        Freq of use: high))  */
    float sw_aeT_tolerance_out;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_evBias_strg),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(-500,500),
        M4_DEFAULT(0),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(1),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The strength bias of hdr ae setpoint control.\n
        Freq of use: low))  */
    float sw_aeT_evBias_strg;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_strategy_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(ae_strategy_mode_t),
        M4_DEFAULT(ae_strategy_lowlight_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(The working mode of exposure strategy.\n
        Freq of use: low))  */
    ae_strategy_mode_t sw_aeT_strategy_mode;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_lumaDist_thred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,100),
        M4_DEFAULT(10),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(4),
        M4_NOTES(The luma distance threshhold used in hdrae control.\n
        Freq of use: low))  */
    float sw_aeT_lumaDist_thred;

    /* M4_GENERIC_DESC(
        M4_ALIAS(initExp),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(5),
        M4_NOTES(The params for initial exposure value.))  */
    ae_hdrInitExp_t initExp;

    /* M4_GENERIC_DESC(
        M4_ALIAS(route),
        M4_TYPE(struct),
        M4_UI_MODULE(array_table_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(6),
        M4_NOTES(The params for exposure route setting.))  */
    ae_hdrRoute_t route;

    /* M4_GENERIC_DESC(
        M4_ALIAS(expRatioCtrl),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(7),
        M4_NOTES(The params for exp-ratio control.))  */
    ae_hdrExpRatioCtrl_t expRatioCtrl;

    /* M4_GENERIC_DESC(
        M4_ALIAS(lfrmMode),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(8),
        M4_NOTES(The params for longframe mode control.\n
        Freq of use: low))  */
    ae_hdrLfrmMode_t lfrmMode;

    /* M4_GENERIC_DESC(
        M4_ALIAS(lfrmCtrl),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(9),
        M4_NOTES(The params for long frame control.))  */
    ae_hdrLfrmCtrl_t lfrmCtrl;

    /* M4_GENERIC_DESC(
        M4_ALIAS(mfrmCtrl),
        M4_TYPE(struct),
        M4_UI_MODULE(array_table_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(10),
        M4_NOTES(The params for middle frame control.))  */
    ae_hdrMfrmCtrl_t mfrmCtrl;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sfrmCtrl),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(11),
        M4_NOTES(The params for short frame control.))  */
    ae_hdrSfrmCtrl_t sfrmCtrl;
} ae_hdrAeCtrl_t;

/*****************************************************************************/
/**
 * @brief   AEC Params: AEC algo SyncTest Config Params
 */
/*****************************************************************************/
typedef struct ae_linAlterExp_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_time_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,5),
        M4_DEFAULT(0.01),
        M4_DIGIT_EX(6),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The value of exposure time.\n
        Freq of use: high))  */
    float sw_aeT_time_val;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_gain_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,4096),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The value of exposure gain.\n
        Freq of use: high))  */
    float sw_aeT_gain_val;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_ispDGain_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,32),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The value of isp dgain.\n
        Freq of use: high))  */
    float sw_aeT_ispDGain_val;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_dcg_mode),
        M4_TYPE(s32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(-1,1),
        M4_DEFAULT(-1),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(4),
        M4_NOTES(The value of dcg mode.\n
        Freq of use: high))  */
    int sw_aeT_dcg_mode;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_pIrisGain_val),
        M4_TYPE(s32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,1024),
        M4_DEFAULT(512),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(The value of p-iris gain.\n
        Freq of use: low))  */
    int sw_aeT_pIrisGain_val;
} ae_linAlterExp_t;

typedef struct ae_hdrAlterExp_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_time_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,3),
        M4_RANGE_EX(0,5),
        M4_DEFAULT(0.01),
        M4_DIGIT_EX(6),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_MARK(2frm:S/L/-,3frm:S/M/L),
        M4_NOTES(The value of exposure time.\n
        Freq of use: high))  */
    float sw_aeT_time_val[AE_HDRFRAME_MAX_NUM];

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_gain_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,3),
        M4_RANGE_EX(1,4096),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_MARK(2frm:S/L/-,3frm:S/M/L),
        M4_NOTES(The value of exposure gain.\n
        Freq of use: high))  */
    float sw_aeT_gain_val[AE_HDRFRAME_MAX_NUM];

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_ispDGain_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,3),
        M4_RANGE_EX(1,32),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_MARK(2frm:S/L/-,3frm:S/M/L),
        M4_NOTES(The value of isp dgain.\n
        Freq of use: high))  */
    float sw_aeT_ispDGain_val[AE_HDRFRAME_MAX_NUM];

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_dcg_mode),
        M4_TYPE(s32),
        M4_SIZE_EX(1,3),
        M4_RANGE_EX(-1,1),
        M4_DEFAULT(-1),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_MARK(2frm:S/L/-,3frm:S/M/L),
        M4_NOTES(The value of dcg mode.\n
        Freq of use: high))  */
    int sw_aeT_dcg_mode[AE_HDRFRAME_MAX_NUM];

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_pIrisGain_val),
        M4_TYPE(s32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,1024),
        M4_DEFAULT(512),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(4),
        M4_MARK(S/M/L),
        M4_NOTES(The value of p-iris gain.\n
        Freq of use: low))  */
    int sw_aeT_pIrisGain_val;

} ae_hdrAlterExp_t;

typedef struct ae_alterExp_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_linAlterExp_len),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,6),
        M4_DEFAULT(2),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The number of linear-exposure group actually used to alteration.\n
        Freq of use: high))  */
    uint8_t sw_aeT_linAlterExp_len;

    /* M4_GENERIC_DESC(
        M4_ALIAS(linAlterExp),
       M4_TYPE(struct_list),
       M4_SIZE_EX(1,6),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The params for linear-exposure group alteration.))  */
    ae_linAlterExp_t linAlterExp[AE_STRUCT_MAX_NUM];

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_hdrAlterExp_len),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,6),
        M4_DEFAULT(2),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The number of hdr-exposure group actually used to alteration.\n
        Freq of use: high))  */
    uint8_t sw_aeT_hdrAlterExp_len;

    /* M4_GENERIC_DESC(
        M4_ALIAS(hdrAlterExp),
        M4_TYPE(struct_list),
       M4_SIZE_EX(1,6),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(The params for hdr-exposure group alteration.))  */
    ae_hdrAlterExp_t hdrAlterExp[AE_STRUCT_MAX_NUM];
} ae_alterExp_t;


typedef struct ae_syncTest_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_syncTest_en),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP_CTRL(sync_test_group),
        M4_NOTES(The enable bit of exposure sync test (for debug).\n
        Freq of use: high))  */
    bool sw_aeT_syncTest_en;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_syncTest_interval),
        M4_TYPE(s32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,256),
        M4_DEFAULT(5),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_GROUP(sync_test_group),
        M4_NOTES(The frame inverval value of switching exposure for sync test.\n
        Freq of use: high))  */
    int sw_aeT_syncTest_interval;

    /* M4_GENERIC_DESC(
        M4_ALIAS(alterExp),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_GROUP(sync_test_group),
        M4_NOTES(The params for exposure group alteration.))  */
    ae_alterExp_t alterExp;
} ae_syncTest_t;

/*****************************************************************************/
/**
 * @brief   AEC Params: AEC algo IrisCtrl Config Params
 */
/*****************************************************************************/
typedef struct ae_manIris_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_manIris_en),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP_CTRL(man_iris_group),
        M4_NOTES(The enable bit of iris control in manual mode.\n
        Freq of use: high))  */
    bool sw_aeT_manIris_en;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_manPIrisGain_val),
        M4_TYPE(s32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,1024),
        M4_DEFAULT(512),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_GROUP(man_iris_group),
        M4_NOTES(The gain value of precise iris in manual mode.\n
        Freq of use: high))  */
    int sw_aeT_manPIrisGain_val;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_manDCIrisHold_val),
        M4_TYPE(s32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,100),
        M4_DEFAULT(100),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_GROUP(man_iris_group),
        M4_NOTES(The hold value of dc iris without hall in manual mode.\n
        Freq of use: high))  */
    int sw_aeT_manDCIrisHold_val;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_manHDCIrisGain_val),
        M4_TYPE(s32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,512),
        M4_DEFAULT(512),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_GROUP(man_iris_group),
        M4_NOTES(The gain value of dc iris with hall in manual mode.\n
        Freq of use: high))  */
    int sw_aeT_manHDCIrisGain_val;
} ae_manIris_t;

typedef struct ae_initIris_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_initPIrisGain_val),
        M4_TYPE(s32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,1024),
        M4_DEFAULT(512),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The initial gain value of precise iris.\n
        Freq of use: high))  */
    int sw_aeT_initPIrisGain_val;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_initDCIrisHold_val),
        M4_TYPE(s32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,100),
        M4_DEFAULT(100),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The initial hold value of dc iris without hall.\n
        Freq of use: high))  */
    int sw_aeT_initDCIrisHold_val;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_initHDCIrisGain_val),
        M4_TYPE(s32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,512),
        M4_DEFAULT(512),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The initial gain value of dc iris with hall.\n
        Freq of use: high))  */
    int sw_aeT_initHDCIrisGain_val;
} ae_initIris_t;

typedef struct ae_pIrisCtrl_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_totalStep_val),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,1024),
        M4_DEFAULT(81),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The total step of p-iris, should refer to the iris datasheet.\n
        Freq of use: high))  */
    uint16_t sw_aeT_totalStep_val;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_effcStep_val),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,1024),
        M4_DEFAULT(64),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The effective step of p-iris, always less than sw_aeT_totalStep_val.\n
        Freq of use: high))  */
    uint16_t sw_aeT_effcStep_val;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_zeroIsMax_en),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The step "0" whether represents max aperture.\n
        Freq of use: high))  */
    bool sw_aeT_zeroIsMax_en;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_step2Gain_table),
        M4_TYPE(u16),
        M4_SIZE_EX(1,256),
        M4_RANGE_EX(1,1024),
        M4_DEFAULT(512),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(The step table of p-iris, should refer to the iris datasheet.\n
        Freq of use: high))  */
    uint16_t sw_aeT_step2Gain_table[AE_PIRIS_TABLE_MAX_STEP];
} ae_pIrisCtrl_t;

typedef struct ae_dcIrisCtrl_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_dcIris_Kp),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,10),
        M4_DEFAULT(1),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The propotion coeff of pid used in dc-iris control.\n
        Freq of use: high))  */
    float sw_aeT_dcIris_Kp;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_dcIris_Ki),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,10),
        M4_DEFAULT(0.2),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The integratial coeff of pid used in dc-iris control.\n
        Freq of use: high))  */
    float sw_aeT_dcIris_Ki;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_dcIris_Kd),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,10),
        M4_DEFAULT(0.5),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The differential coeff of pid used in dc-iris control.\n
        Freq of use: high))  */
    float sw_aeT_dcIris_Kd;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_pwmDuty_min),
        M4_TYPE(s32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,100),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(The min pwm duty used in dc-iris control.\n
        Freq of use: high))  */
    int sw_aeT_pwmDuty_min;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_pwmDuty_max),
        M4_TYPE(s32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,100),
        M4_DEFAULT(100),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(4),
        M4_NOTES(The max pwm duty used in dc-iris control.\n
        Freq of use: high))  */
    int sw_aeT_pwmDuty_max;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_pwmDuty_open),
        M4_TYPE(s32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,100),
        M4_DEFAULT(60),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(5),
        M4_NOTES(The pwm duty representing iris open in dc-iris control.\n
        Freq of use: high))  */
    int sw_aeT_pwmDuty_open;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_pwmDuty_close),
        M4_TYPE(s32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,100),
        M4_DEFAULT(40),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(6),
        M4_NOTES(The pwm duty representing iris close in dc-iris control.\n
        Freq of use: high))  */
    int sw_aeT_pwmDuty_close;
} ae_dcIrisCtrl_t;

typedef struct ae_hdcIrisZoom_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeC_zoom2Iris_len),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,256),
        M4_DEFAULT(12),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The actual length of dot used in zoom2Iris.
        \nFreq of use: low))  */
    int sw_aeC_zoom2Iris_len;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeC_zoom2Iris_idx),
        M4_TYPE(s32),
        M4_SIZE_EX(1,256),
        M4_RANGE_EX(-32768,32767),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The zoom position at each lens magnification.
        \nFreq of use: low))  */
    int sw_aeC_zoom2Iris_idx[AE_HDCIRIS_DOT_MAX_NUM];

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeC_zoom2Iris_val),
        M4_TYPE(s32),
        M4_SIZE_EX(1,256),
        M4_RANGE_EX(0,1023),
        M4_DEFAULT(1023),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The best hdc-iris target position at each lens magnification.\n
        Freq of use: low))  */
    int sw_aeC_zoom2Iris_val[AE_HDCIRIS_DOT_MAX_NUM];
} ae_hdcIrisZoom_t;

typedef struct ae_hdcIrisGain_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeC_iris2Gain_len),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,256),
        M4_DEFAULT(12),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The actual length of dot used in iris2Gain.\n
        Freq of use: low))  */
    int sw_aeC_iris2Gain_len;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeC_iris2Gain_idx),
        M4_TYPE(s32),
        M4_SIZE_EX(1,256),
        M4_RANGE_EX(0,1023),
        M4_DEFAULT(1023),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The adjustment node of hdc-iris target value.\n
        Freq of use: low))  */
    int sw_aeC_iris2Gain_idx[AE_HDCIRIS_DOT_MAX_NUM];

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeC_iris2Gain_val),
        M4_TYPE(s32),
        M4_SIZE_EX(1,256),
        M4_RANGE_EX(1,512),
        M4_DEFAULT(512),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The equivalent gain of hdc-iris target value.\n
        Freq of use: low))  */
    int sw_aeC_iris2Gain_val[AE_HDCIRIS_DOT_MAX_NUM];
} ae_hdcIrisGain_t;

typedef struct ae_hdcIrisCtrl_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_damp_over),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0.15),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The damping coeff value for hdc-iris opening.\n
        Freq of use: low))  */
    float sw_aeT_damp_over;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_damp_under),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0.45),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The damping coeff value for hdc-iris closing.\n
        Freq of use: low))  */
    float sw_aeT_damp_under;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_zeroIsMax_en),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The target "0" whether represents max aperture.\n
        Freq of use: low))  */
    bool sw_aeT_zeroIsMax_en;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_target_min),
        M4_TYPE(s32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1023),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(The min adjustable hdc-iris target value.\n
        Freq of use: low))  */
    int sw_aeT_target_min;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_target_max),
        M4_TYPE(s32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1023),
        M4_DEFAULT(1023),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(4),
        M4_NOTES(The max adjustable hdc-iris target value.\n
        Freq of use: low))  */
    int sw_aeT_target_max;

    /* M4_GENERIC_DESC(
        M4_ALIAS(zoom2Iris),
        M4_TYPE(struct),
        M4_UI_MODULE(array_table_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(5),
        M4_NOTES(The mapping table between zoom and best iris value.))  */
    ae_hdcIrisZoom_t zoom2Iris;

    /* M4_GENERIC_DESC(
        M4_ALIAS(iris2Gain),
        M4_TYPE(struct),
        M4_UI_MODULE(array_table_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(6),
        M4_NOTES(The mapping table between iris and equivalent gain.))  */
    ae_hdcIrisGain_t iris2Gain;
} ae_hdcIrisCtrl_t;

typedef struct ae_irisCtrl_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_iris_en),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP_CTRL(iris_ctrl_group),
        M4_NOTES(The enable bit of iris control.\n
        Freq of use: high))  */
    bool sw_aeT_iris_en;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aeT_iris_type),
        M4_TYPE(enum),
        M4_ENUM_DEF(ae_iris_type_t),
        M4_DEFAULT(ae_iris_dc_type),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_GROUP(iris_ctrl_group),
        M4_GROUP_CTRL(iris_type_group),
        M4_NOTES(The type of iris.\n
        Freq of use: high))  */
    ae_iris_type_t sw_aeT_iris_type;

    /* M4_GENERIC_DESC(
        M4_ALIAS(initIris),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_GROUP(iris_ctrl_group),
        M4_NOTES(The params for initial iris value.))  */
    ae_initIris_t initIris;

    /* M4_GENERIC_DESC(
        M4_ALIAS(manIris),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_GROUP(iris_ctrl_group),
        M4_NOTES(The params for manual iris value.))  */
    ae_manIris_t manIris;

    /* M4_GENERIC_DESC(
        M4_ALIAS(pIrisCtrl),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(4),
        M4_GROUP(iris_ctrl_group;iris_type_group:ae_iris_p_type),
        M4_NOTES(The params for precise iris control.))  */
    ae_pIrisCtrl_t pIrisCtrl;

    /* M4_GENERIC_DESC(
        M4_ALIAS(dcIrisCtrl),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(5),
        M4_GROUP(iris_ctrl_group;iris_type_group:ae_iris_dc_type),
        M4_NOTES(The params for dc iris without hall control.))  */
    ae_dcIrisCtrl_t dcIrisCtrl;

    /* M4_GENERIC_DESC(
        M4_ALIAS(hdcIrisCtrl),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(6),
        M4_GROUP(iris_ctrl_group;iris_type_group:ae_iris_hdc_type),
        M4_NOTES(The params for dc iris with hall control.))  */
    ae_hdcIrisCtrl_t hdcIrisCtrl;
} ae_irisCtrl_t;

/*****************************************************************************/
/**
 * @brief   AEC Params: Global AEC tuning structure
 */
/*****************************************************************************/

typedef struct ae_param_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(commCtrl),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The params for ae common control.))  */
    ae_commCtrl_t commCtrl;

    /* M4_GENERIC_DESC(
        M4_ALIAS(linAeCtrl),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The params for linear ae control.))  */
    ae_linAeCtrl_t linAeCtrl;

    /* M4_GENERIC_DESC(
        M4_ALIAS(hdrAeCtrl),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The params for hdr ae control.))  */
    ae_hdrAeCtrl_t hdrAeCtrl;

    /* M4_GENERIC_DESC(
        M4_ALIAS(irisCtrl),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(The params for iris control.))  */
    ae_irisCtrl_t irisCtrl;

    /* M4_GENERIC_DESC(
        M4_ALIAS(syncTest),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(4),
        M4_NOTES(The params for exposure sync test (for debug).))  */
    ae_syncTest_t syncTest;
} ae_param_t;

RKAIQ_END_DECLARE

#endif
