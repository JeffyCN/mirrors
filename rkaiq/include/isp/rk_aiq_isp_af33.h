/*
 * rk_aiq_isp_af33.h
 *
 *  Copyright (c) 2025 Rockchip Corporation
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
#ifndef _RK_AIQ_PARAM_AF33_H_
#define _RK_AIQ_PARAM_AF33_H_

RKAIQ_BEGIN_DECLARE

#include "isp/rk_aiq_stats_af33.h"

#define AF_MAX_ISO_LEVEL                    (13)
#define AF_MAX_MEAS_CONFIG_NUM              (32)
#define AF_ZOOM_SEARCH_STEP_NUM             (32)
#define AF_QUICK_FOUND_THRED_NUM            (32)
#define AF_VAR_STEP_CFG_NUM                 (10)
#define AF_ADAPT_RANGE_TBL_SIZE             (65)
#define AF_WEIGHT_TBL_SIZE                  (225)
#define AF_LASERAF_DOT_NUM                  (7)
#define AF_PDAF_FINE_SEARCH_TBL_SIZE        (10)
#define AF_PDAF_STEP_RATIO_TBL_SIZE         (7)
#define AF_PDAF_RESOLUTION_INFO_TBL_SIZE    (16)
#define AF_ZOOM_MOVE_STEP_NUM               (32)
#define AF_ZOOM_SEARCH_POS_NUM_MAX          (32)
#define AF_ZOOM_CURVE_FILENAME_LEN          (64)
#define AF_QUERY_MEAS_TBL_SIZE              (8)

typedef enum af_searchDirection_e {
    af_positive_search,
    af_negative_search,
    af_adaptive_search
} af_searchDirection_t;

typedef enum af_search_strategy_e {
    af_search_strategy_fullRange,
    af_search_strategy_adaptRange,
    af_search_strategy_zoom_fixStep,
    af_search_strategy_zoom_varStep
} af_search_strategy_t;

typedef enum af_mode_e {
    af_mode_not_set = -1,
    af_mode_auto,
    af_mode_macro,
    af_mode_infinity,
    af_mode_fixed,
    af_mode_edof,
    af_mode_continue_video,
    af_mode_continue_picture,
    af_mode_zoom_oneshot
} af_mode_t;

typedef enum pdaf_direction_e {
    pdaf_direction_invalid,
    pdaf_direction_left,
    pdaf_direction_right,
    pdaf_direction_top,
    pdaf_direction_down
} pdaf_direction_t;

typedef enum pdaf_sensor_type_e {
    pdaf_sensor_type1,
    pdaf_sensor_type2,
    pdaf_sensor_type3
} pdaf_sensor_type_t;

typedef enum pdaf_binning_mode_e {
    pdaf_binning_off,
    pdaf_binning_mode2,
    pdaf_binning_mode4,
    pdaf_binning_mode8,
    pdaf_binning_mode16
} pdaf_binning_mode_t;

typedef struct af_zoomVarStepCfg_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_queryZoom_idx),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,100000),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Zoom index to get search config.\n
        Freq of use: low))  */
    unsigned short sw_afT_queryZoom_idx;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_queryIso_val),
        M4_TYPE(s32),
        M4_SIZE_EX(1,13),
        M4_RANGE_EX(0,1000000),
        M4_DEFAULT([50, 100, 200, 400, 800, 1600, 3200, 6400, 12800, 25600, 51200, 102400, 204800]),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_DYNAMIC_EX(0),
        M4_ORDER(1),
        M4_NOTES(Iso value to get search config.\n
        Freq of use: low))  */
    int sw_afT_queryIso_val[AF_MAX_ISO_LEVEL];
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_search_step),
        M4_TYPE(u16),
        M4_SIZE_EX(1,13),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(1),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_DYNAMIC_EX(0),
        M4_ORDER(2),
        M4_NOTES(Search step value.\n
        Freq of use: low))  */
    unsigned short sw_afT_search_step[AF_MAX_ISO_LEVEL];
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_stage1Skip_thred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,13),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0.02),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_DYNAMIC_EX(0),
        M4_ORDER(3),
        M4_NOTES(Stage 1 skip threshold value.\n
        Freq of use: low))  */
    float sw_afT_stage1Skip_thred[AF_MAX_ISO_LEVEL];
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_stage2QuickEnd_thred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,13),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0.02),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_DYNAMIC_EX(0),
        M4_ORDER(4),
        M4_NOTES(Stage 2 quick found threshold value.\n
        Freq of use: low))  */
    float sw_afT_stage2QuickEnd_thred[AF_MAX_ISO_LEVEL];
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_skipStep_cnt),
        M4_TYPE(u16),
        M4_SIZE_EX(1,13),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(4),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_DYNAMIC_EX(0),
        M4_ORDER(5),
        M4_NOTES(Skip next step count value.\n
        Freq of use: low))  */
    unsigned short sw_afT_skipStep_cnt[AF_MAX_ISO_LEVEL];
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_skipStep_thred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,13),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0.02),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_DYNAMIC_EX(0),
        M4_ORDER(6),
        M4_NOTES(Skip next step threshold value.\n
        Freq of use: low))  */
    float sw_afT_skipStep_thred[AF_MAX_ISO_LEVEL];
} af_zoomVarStepCfg_t;

typedef struct af_zoomSearchStep_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_searchStepTbl_len),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,32),
        M4_DEFAULT(1),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Length of zoom index table to find search step.\n
        Freq of use: low))  */
    int sw_afT_searchStepTbl_len;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_searchStepZoomIdx_dot),
        M4_TYPE(u16),
        M4_SIZE_EX(1,32),
        M4_RANGE_EX(0,65535),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(Zoom index to find search step.\n
        Freq of use: low))  */
    unsigned short sw_afT_searchStepZoomIdx_dot[AF_ZOOM_SEARCH_STEP_NUM];
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_searchStep_dot),
        M4_TYPE(u16),
        M4_SIZE_EX(1,32),
        M4_RANGE_EX(0,65535),
        M4_DEFAULT(16),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(Search step value table.\n
        Freq of use: low))  */
    unsigned short sw_afT_searchStep_dot[AF_ZOOM_SEARCH_STEP_NUM];
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_stopStep_dot),
        M4_TYPE(u16),
        M4_SIZE_EX(1,32),
        M4_RANGE_EX(0,65535),
        M4_DEFAULT(4),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(Stop step value table.\n
        Freq of use: low))  */
    unsigned short sw_afT_stopStep_dot[AF_ZOOM_SEARCH_STEP_NUM];
} af_zoomSearchStep_t;

typedef struct af_zoomQuickFound_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_zoomQuickFoundTbl_len),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,32),
        M4_DEFAULT(1),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Length of zoom index table to search quick found threshold value.\n
        Freq of use: low))  */
    int sw_afT_zoomQuickFoundTbl_len;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_quickFoundZoomIdx_dot),
        M4_TYPE(u16),
        M4_SIZE_EX(1,32),
        M4_RANGE_EX(0,65535),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(Zoom index to search quick found threshold value.\n
        Freq of use: low))  */
    unsigned short sw_afT_quickFoundZoomIdx_dot[AF_QUICK_FOUND_THRED_NUM];
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_quickFoundThred_dot),
        M4_TYPE(f32),
        M4_SIZE_EX(1,32),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0.05),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(Quick found threshold value table.\n
        Freq of use: low))  */
    float sw_afT_quickFoundThred_dot[AF_QUICK_FOUND_THRED_NUM];
} af_zoomQuickFound_t;

typedef struct af_contrastZoom_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(zoomSearchStep),
        M4_TYPE(struct),
        M4_UI_MODULE(array_table_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Search Step table for zoom.))  */
    af_zoomSearchStep_t zoomSearchStep;
    /* M4_GENERIC_DESC(
        M4_ALIAS(zoomQuickFound),
        M4_TYPE(struct),
        M4_UI_MODULE(array_table_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(Quick found table for zoom.))  */
    af_zoomQuickFound_t zoomQuickFound;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_switchDirZoom_idx),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,65535),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(Zoom index to switch search direction(positive and adaptive).\n
        Freq of use: low))  */
    unsigned short sw_afT_switchDirZoom_idx;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_spotlightHighlight_ratio),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0.014),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(Threshold for mean high light value.\n
        Freq of use: low))  */
    float sw_afT_spotlightHighlight_ratio;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_spotlightLuma_ratio),
        M4_TYPE(f32),
        M4_SIZE_EX(1,3),
        M4_RANGE_EX(0,1),
        M4_DEFAULT([0.3, 0.5, 0.8]),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(4),
        M4_NOTES(Threshold for luma ratio(=cur luma blk/max luma blk), (low/mid/high).\n
        Freq of use: low))  */
    float sw_afT_spotlightLuma_ratio[3];
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_spotlightBlk_cnt),
        M4_TYPE(f32),
        M4_SIZE_EX(1,3),
        M4_RANGE_EX(0,1),
        M4_DEFAULT([0.2, 0.5, 0.25]),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(5),
        M4_NOTES(Threshold for block count ratio(low/mid/high).\n
        Freq of use: low))  */
    float sw_afT_spotlightBlk_cnt[3];
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_varStepCfg),
        M4_TYPE(struct_list),
        M4_SIZE_EX(1,10),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_DYNAMIC_EX(1),
        M4_ORDER(6),
        M4_NOTES(Parameter for variable step search.\n
        Freq of use: low))  */
    af_zoomVarStepCfg_t varStepCfg[AF_VAR_STEP_CFG_NUM];
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_varStepCfg_len),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,32),
        M4_DEFAULT(1),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(7),
        M4_NOTES(Length of  variable step search table.\n
        Freq of use: low))  */
    int sw_afT_varStepCfg_len;
} af_contrastZoom_t;

typedef struct af_adaptRangeSearchTbl_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_adaptRangeSearchTbl_len),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(2,33),
        M4_DEFAULT(9),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Length of rough search table for adaptive search.\n
        Freq of use: high))  */
    int sw_afT_adaptRangeSearchTbl_len;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_adaptRange_dot),
        M4_TYPE(u16),
        M4_SIZE_EX(2,33),
        M4_RANGE_EX(0,1023),
        M4_DEFAULT([0,8,16,24,32,40,48,56,64]),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(Rough search table for adaptive search.\n
        Freq of use: high))  */
    unsigned short sw_afT_adaptRange_dot[AF_ADAPT_RANGE_TBL_SIZE];
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_fineSearchStep_dot),
        M4_TYPE(u16),
        M4_SIZE_EX(2,33),
        M4_RANGE_EX(0,1023),
        M4_DEFAULT([4,4,4,4,4,4,4,4,4]),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(Fine search table for adaptive search.\n
        Freq of use: high))  */
    unsigned short sw_afT_fineSearchStep_dot[AF_ADAPT_RANGE_TBL_SIZE];
} af_adaptRangeSearchTbl_t;

typedef struct af_contrast_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_contrastAf_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP_CTRL(contrast_en_group),
        M4_NOTES(Enable contrast af algo.\n
        Freq of use: high))  */
    bool sw_afT_contrastAf_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_searchStrategy_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(af_search_strategy_t),
        M4_DEFAULT(af_search_strategy_adaptRange),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(Search strategy of contrast af.\n
        Freq of use: low))  */
    af_search_strategy_t sw_afT_searchStrategy_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_fullRangeSearchDir_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(af_searchDirection_t),
        M4_DEFAULT(af_adaptive_search),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(Search direction for full range search.\n
        Freq of use: low))  */
    af_searchDirection_t sw_afT_fullRangeSearchDir_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_fullRange_tbl),
        M4_TYPE(s16),
        M4_SIZE_EX(1,3),
        M4_RANGE_EX(-1023,1023),
        M4_DEFAULT([0,1,64]),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(Full range search table(start/step/end).\n
        Freq of use: low))  */
    short sw_afT_fullRange_tbl[3];
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_adaptiveSearchDir_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(af_searchDirection_t),
        M4_DEFAULT(af_adaptive_search),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(4),
        M4_NOTES(Search direction for adaptive search.\n
        Freq of use: low))  */
    af_searchDirection_t sw_afT_adaptiveSearchDir_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(adaptRangeSearchTbl),
        M4_TYPE(struct),
        M4_UI_MODULE(array_table_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(5),
        M4_NOTES(Adaptive range search table.))  */
    af_adaptRangeSearchTbl_t adaptRangeSearchTbl;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_skipCurveFitGain_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1000),
        M4_DEFAULT(0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(6),
        M4_NOTES(Sensor gain value to skip curve fit.\n
        Freq of use: low))  */
    float sw_afT_skipCurveFitGain_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_fvTrig_thred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0.075),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(7),
        M4_NOTES(Focus value trigger threshold table.\n
        Freq of use: high))  */
    float sw_afT_fvTrig_thred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_lumaTrig_thred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(1),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(8),
        M4_NOTES(Luma trigger threshold.\n
        Freq of use: high))  */
    float sw_afT_lumaTrig_thred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_expTrig_thred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(1),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(9),
        M4_NOTES(Exposure trigger threshold.\n
        Freq of use: high))  */
    float sw_afT_expTrig_thred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_triggerChangedFrame_num),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1000),
        M4_DEFAULT(10),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(10),
        M4_NOTES(Changed frames to trigger af search.\n
        Freq of use: high))  */
    unsigned short sw_afT_triggerChangedFrame_num;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_fvStable_thred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0.02),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(11),
        M4_NOTES(Af stable threshold.\n
        Freq of use: high))  */
    float sw_afT_fvStable_thred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_fvStableFrame_num),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1023),
        M4_DEFAULT(3),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(12),
        M4_NOTES(Af stable status must hold frame number.\n
        Freq of use: high))  */
    unsigned short sw_afT_fvStableFrame_num;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_outFocusFv_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1000000),
        M4_DEFAULT(50),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(13),
        M4_NOTES(Focus value when out of focus.\n
        Freq of use: low))  */
    float sw_afT_outFocusFv_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_outFocus_pos),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1023),
        M4_DEFAULT(64),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(14),
        M4_NOTES(Focus position when out of focus.\n
        Freq of use: low))  */
    unsigned short sw_afT_outFocus_pos;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_lowLuma_thred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,4095),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(15),
        M4_NOTES(Low luma scene vlaue.\n
        Freq of use: low))  */
    float sw_afT_lowLuma_thred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_lowLuma_pos),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1023),
        M4_DEFAULT(64),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(16),
        M4_NOTES(Focus position when in low luma scene.\n
        Freq of use: low))  */
    unsigned short sw_afT_lowLuma_pos;
    /* M4_GENERIC_DESC(
        M4_ALIAS(WeightEnable),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(17),
        M4_NOTES(Enable weight matrix.\n
        Freq of use: low))  */
    bool sw_afT_weight_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(Weight),
        M4_TYPE(u16),
        M4_SIZE_EX(15,15),
        M4_RANGE_EX(0,32),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_DYNAMIC_EX(0),
        M4_ORDER(18),
        M4_UI_MODULE(grid_weight_table),
        M4_NOTES(Weight matrix to get focus value.\n
        Freq of use: low))  */
    unsigned char sw_afT_weight_tbl[AF_WEIGHT_TBL_SIZE];
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_searchPauseLuma_en),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(19),
        M4_NOTES(Enable to check luma change in search.\n
        Freq of use: low))  */
    bool sw_afT_searchPauseLuma_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_searchPauseLuma_thred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0.05),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(20),
        M4_NOTES(Search pause threshold when luma change much in search.\n
        Freq of use: low))  */
    float sw_afT_searchPauseLuma_thred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_reSearchLumaStabFrm_num),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1023),
        M4_DEFAULT(3),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(21),
        M4_NOTES(Frame number to judge luma is stable.\n
        Freq of use: low))  */
    unsigned short sw_afT_reSearchLumaStabFrm_num;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_reSearchLumaStable_thred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0.02),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(22),
        M4_NOTES(Luma change threshold to judge luma is stable.\n
        Freq of use: low))  */
    float sw_afT_reSearchLumaStable_thred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_stage1QuickEnd_thred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0.02),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(23),
        M4_NOTES(Quick found threshold value in search stage1.\n
        Freq of use: high))  */
    float sw_afT_stage1QuickEnd_thred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_stage2QuickEnd_thred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0.04),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(24),
        M4_NOTES(Quick found threshold value in search stage2.\n
        Freq of use: high))  */
    float sw_afT_stage2QuickEnd_thred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_flatSceneFv_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1000000),
        M4_DEFAULT(0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(25),
        M4_NOTES(Focus value in centor threshold to judge flat scene.\n
        Freq of use: low))  */
    float sw_afT_flatSceneFv_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_validBlkMaxMin_ratio),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(26),
        M4_NOTES(Valid block ratio in flat scene.(max-min)/(max+min)\n
        Freq of use: low))  */
    float sw_afT_validBlkMaxMin_ratio;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_validBlkFv_thred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1000000),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(27),
        M4_NOTES(Valid focus value in flat scene.\n
        Freq of use: low))  */
    float sw_afT_validBlkFv_thred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_pointLightLuma_thred),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,4095),
        M4_DEFAULT(4095),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(28),
        M4_NOTES(Luma threshold in point light scene.\n
        Freq of use: low))  */
    unsigned short sw_afT_pointLightLuma_thred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_pointLightCnt_thred),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,225),
        M4_DEFAULT(225),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(29),
        M4_NOTES(Block count threshold in point light scene.\n
        Freq of use: low))  */
    unsigned short sw_afT_pointLightCnt_thred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(ZoomConfig),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(30),
        M4_NOTES(Optical zoom config.\n
        Freq of use: low))  */
    af_contrastZoom_t zoomConfig;
} af_contrast_t;

typedef struct af_laser_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(Enable),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP_CTRL(laser_en_group),
        M4_NOTES(Enable laser af algo.\n
        Freq of use: low))  */
    bool sw_afT_laserAf_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(VcmDot),
        M4_TYPE(f32),
        M4_SIZE_EX(1,7),
        M4_RANGE_EX(0,1023),
        M4_DEFAULT([0, 16, 32, 40, 48, 56, 64]),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(Motor position table.\n
        Freq of use: low))  */
    float sw_afT_vcm_dot[AF_LASERAF_DOT_NUM];
    /* M4_GENERIC_DESC(
        M4_ALIAS(DistanceDot),
        M4_TYPE(f32),
        M4_SIZE_EX(1,7),
        M4_RANGE_EX(0,65535),
        M4_DEFAULT([0.2, 0.24, 0.34, 0.4, 0.66, 1, 3]),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(Distance table.\n
        Freq of use: low))  */
    float sw_afT_distance_dot[AF_LASERAF_DOT_NUM];
} af_laser_t;

typedef struct af_pdafFineSearch_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_pdafFineSearch_len),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,10),
        M4_DEFAULT(1),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Length of fine search table.\n
        Freq of use: high))  */
    uint8_t sw_afT_pdafFineSearch_len;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_pdafConfidence_dot),
        M4_TYPE(f32),
        M4_SIZE_EX(1,10),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0.5),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(Confidence of pdaf algo.\n
        Freq of use: high))  */
    float sw_afT_pdafConfidence_dot[AF_PDAF_FINE_SEARCH_TBL_SIZE];
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_pdafSearchRange_dot),
        M4_TYPE(s32),
        M4_SIZE_EX(1,10),
        M4_RANGE_EX(0,1023),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(Search range in fine search.\n
        Freq of use: high))  */
    int sw_afT_pdafSearchRange_dot[AF_PDAF_FINE_SEARCH_TBL_SIZE];
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_pdafSearchStep_dot),
        M4_TYPE(s32),
        M4_SIZE_EX(1,10),
        M4_RANGE_EX(0,1023),
        M4_DEFAULT(16),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(Search step in fine search.\n
        Freq of use: high))  */
    int sw_afT_pdafSearchStep_dot[AF_PDAF_FINE_SEARCH_TBL_SIZE];
} af_pdafFineSearch_t;

typedef struct af_pdafIsoPara_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(Iso),
        M4_TYPE(s32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1000000),
        M4_DEFAULT(50),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Iso value to query.\n
        Freq of use: high))  */
    int sw_afT_pdIso_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(PdConfdRatio0),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,256),
        M4_DEFAULT(1),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(Confidence adjust ratio 0.\n
        Freq of use: high))  */
    float sw_afT_pdConfd0_ratio;
    /* M4_GENERIC_DESC(
        M4_ALIAS(PdConfdRatio1),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,256),
        M4_DEFAULT(1),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(Confidence adjust ratio 1.\n
        Freq of use: low))  */
    float sw_afT_pdConfd1_ratio;
    /* M4_GENERIC_DESC(
        M4_ALIAS(PdConfdRatio2),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,256),
        M4_DEFAULT(1),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(Confidence adjust ratio 2.\n
        Freq of use: low))  */
    float sw_afT_pdConfd2_ratio;
    /* M4_GENERIC_DESC(
        M4_ALIAS(PdConfdRatio3),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,256),
        M4_DEFAULT(1),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(4),
        M4_NOTES(Confidence adjust ratio 3.\n
        Freq of use: high))  */
    float sw_afT_pdConfd3_ratio;
    /* M4_GENERIC_DESC(
        M4_ALIAS(PdConfdThresh),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0.4),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(5),
        M4_NOTES(Confidence threshold to trust pdaf algo.\n
        Freq of use: high))  */
    float sw_afT_pdConfd_thred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_convergedInfPd_thred),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1023),
        M4_DEFAULT(12),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(6),
        M4_NOTES(Converged motor position threshold to stop search(infinity side).\n
        Freq of use: high))  */
    unsigned short sw_afT_convergedInfPd_thred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_convergedMacPd_thred),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1023),
        M4_DEFAULT(12),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(7),
        M4_NOTES(Converged motor position threshold to stop search(macro side).\n
        Freq of use: high))  */
    unsigned short sw_afT_convergedMacPd_thred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_defocusInfPd_thred),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1023),
        M4_DEFAULT(24),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(8),
        M4_NOTES(Defocus motor position threshold to stop search(infinity side).\n
        Freq of use: high))  */
    unsigned short sw_afT_defocusInfPd_thred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_defocusMacPd_thred),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1023),
        M4_DEFAULT(24),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(9),
        M4_NOTES(Defocus motor position threshold to stop search(macro side).\n
        Freq of use: high))  */
    unsigned short sw_afT_defocusMacPd_thred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_stablePd_ratio),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(0.125),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(10),
        M4_NOTES(Ratio value to judge pd stable.\n
        Freq of use: low))  */
    float sw_afT_stablePd_ratio;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_stablePd_offset),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1023),
        M4_DEFAULT(8),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(11),
        M4_NOTES(Offset value to judge pd stable.\n
        Freq of use: low))  */
    unsigned short sw_afT_stablePd_offset;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_stableCnt_ratio),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1023),
        M4_DEFAULT(1.5),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(12),
        M4_NOTES(Ratio to decide stable frame count threshold.\n
        Freq of use: low))  */
    float sw_afT_stableCnt_ratio;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_noConfCnt_thred),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(4),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(13),
        M4_NOTES(No confidence frame count to turn to contrast af.\n
        Freq of use: low))  */
    unsigned short sw_afT_noConfCnt_thred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_inRoiBlkHor_num),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,10),
        M4_DEFAULT(3),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(14),
        M4_NOTES(Block count in roi at horizontal direction.\n
        Freq of use: low))  */
    unsigned char sw_afT_inRoiBlkHor_num;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_inRoiBlkVer_num),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,10),
        M4_DEFAULT(3),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(15),
        M4_NOTES(Block count in roi at vertical direction.\n
        Freq of use: low))  */
    unsigned char sw_afT_inRoiBlkVer_num;
    /* M4_GENERIC_DESC(
        M4_ALIAS(fineSearchTbl),
        M4_TYPE(struct),
        M4_UI_MODULE(array_table_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(16),
        M4_NOTES(Fine search table.\n
        Freq of use: low))  */
    af_pdafFineSearch_t fineSearchTbl;
} af_pdafIsoPara_t;

typedef struct af_pdafCalibInf_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afC_pdGainMap_width),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,1000),
        M4_DEFAULT(32),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Gainmap width in calibration.\n
        Freq of use: high))  */
    unsigned short sw_afC_pdGainMap_width;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afC_pdGainMap_height),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,1000),
        M4_DEFAULT(32),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(Gainmap height in calibration.\n
        Freq of use: high))  */
    unsigned short sw_afC_pdGainMap_height;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afC_pdDccMap_width),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,1000),
        M4_DEFAULT(32),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(Dccmap width in calibration.\n
        Freq of use: high))  */
    unsigned short sw_afC_pdDccMap_width;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afC_pdDccMap_height),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,1000),
        M4_DEFAULT(32),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(Dccmap height in calibration.\n
        Freq of use: high))  */
    unsigned short sw_afC_pdDccMap_height;
} af_pdafCalibInf_t;

typedef struct af_pdafResolution_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_pdSensorType_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(pdaf_sensor_type_t),
        M4_DEFAULT(pdaf_sensor_type2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Pdaf sensor type.\n
        Freq of use: high))  */
    pdaf_sensor_type_t sw_afT_pdSensorType_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_image_width),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,65535),
        M4_DEFAULT(4096),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(Image width of sensor input.\n
        Freq of use: high))  */
    unsigned short sw_afT_image_width;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_image_height),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,65535),
        M4_DEFAULT(3072),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(Image height of sensor input.\n
        Freq of use: high))  */
    unsigned short sw_afT_image_height;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_pdOut_width),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,65535),
        M4_DEFAULT(2048),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(Pd data width of current sensor setting.\n
        Freq of use: high))  */
    unsigned short sw_afT_pdOut_width;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_pdOut_height),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,65535),
        M4_DEFAULT(768),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(4),
        M4_NOTES(Pd data height of current sensor setting.\n
        Freq of use: high))  */
    unsigned short sw_afT_pdOut_height;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_pdCrop_x),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,65535),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(5),
        M4_NOTES(Pd data start x position of current setting vs calibration setting.\n
        Freq of use: high))  */
    unsigned short sw_afT_pdCrop_x;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_pdCrop_y),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,65535),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(6),
        M4_NOTES(Pd data start y position of current setting vs calibration setting.\n
        Freq of use: high))  */
    unsigned short sw_afT_pdCrop_y;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_pdBase_width),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,65535),
        M4_DEFAULT(2048),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(7),
        M4_NOTES(Pd data width in calibration.\n
        Freq of use: high))  */
    unsigned short sw_afT_pdBase_width;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_pdBase_height),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,65535),
        M4_DEFAULT(768),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(8),
        M4_NOTES(Pd data height in calibration.\n
        Freq of use: high))  */
    unsigned short sw_afT_pdBase_height;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_pdHBin_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(pdaf_binning_mode_t),
        M4_DEFAULT(pdaf_binning_off),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Pdaf binning mode.\n
        Freq of use: high))  */
    pdaf_binning_mode_t sw_afT_pdHBin_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_pdVBin_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(pdaf_binning_mode_t),
        M4_DEFAULT(pdaf_binning_off),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Pdaf binning mode.\n
        Freq of use: high))  */
    pdaf_binning_mode_t sw_afT_pdVBin_mode;
} af_pdafResolution_t;

typedef struct af_pdafStepRatio_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_pdStepDefocus_dot),
        M4_TYPE(u16),
        M4_SIZE_EX(1,7),
        M4_RANGE_EX(0,1023),
        M4_DEFAULT([32, 64, 96, 128, 160, 192, 224]),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Defous step table to search PdStepRatio.\n
        Freq of use: high))  */
    unsigned short sw_afT_pdStepDefocus_dot[AF_PDAF_STEP_RATIO_TBL_SIZE];
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_pdStepRatio_dot),
        M4_TYPE(f32),
        M4_SIZE_EX(1,7),
        M4_RANGE_EX(0,1),
        M4_DEFAULT([1.0, 1.0, 1.0, 0.9, 0.8, 0.7, 0.7]),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(Discount ratio table for motor move.\n
        Freq of use: high))  */
    float sw_afT_pdStepRatio_dot[AF_PDAF_STEP_RATIO_TBL_SIZE];
} af_pdafStepRatio_t;

typedef struct af_pdaf_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_pdAf_en),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP_CTRL(pdaf_en_group),
        M4_NOTES(Enable pdaf algo.\n
        Freq of use: high))  */
    bool sw_afT_pdAf_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_pdVsCdDebug_en),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(Debug flag to compare pdaf and cdaf.\n
        Freq of use: low))  */
    bool sw_afT_pdVsCdDebug_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_pdDumpDebug_en),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(Debug flag to dump some data.\n
        Freq of use: low))  */
    bool sw_afT_pdDumpDebug_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_pdDumpMaxFrm_num),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,10000),
        M4_DEFAULT(300),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(Max dump frames when debug flag is on.\n
        Freq of use: low))  */
    unsigned short sw_afT_pdDumpMaxFrm_num;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_pdDataBit_num),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,16),
        M4_DEFAULT(10),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(4),
        M4_NOTES(Bit number of one pd data.\n
        Freq of use: low))  */
    unsigned short sw_afT_pdDataBit_num;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_pdBlackLevel_val),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1023),
        M4_DEFAULT(64),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(5),
        M4_NOTES(Black level of pd data.\n
        Freq of use: low))  */
    unsigned short sw_afT_pdBlackLevel_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_pdSearchRadius_val),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,32),
        M4_DEFAULT(3),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(6),
        M4_NOTES(Search radius.\n
        Freq of use: low))  */
    unsigned short sw_afT_pdSearchRadius_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_pdMirrorInCalib_val),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,3),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(7),
        M4_NOTES(Mirror/flip difference with used setting in calibration.\n
        Freq of use: high))  */
    unsigned char sw_afT_pdMirrorInCalib_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_pdChangeLeftRight_en),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(8),
        M4_NOTES(Change left/right pd data when input pd data to pdaf algo.\n
        Freq of use: high))  */
    bool sw_afT_pdChangeLeftRight_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_pdVsImgoutMirror_val),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,3),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(9),
        M4_NOTES(Mirror/flip difference with normal picture, used in get search roi.\n
        Freq of use: low))  */
    unsigned char sw_afT_pdVsImgoutMirror_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_pdLRInDiffLine_en),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(10),
        M4_NOTES(Weather left/right pd data in one line, used to separate left/right pd data.\n
        Freq of use: high))  */
    bool sw_afT_pdLRInDiffLine_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_pdMax_width),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,65535),
        M4_DEFAULT(2048),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(11),
        M4_NOTES(Max pd data width, used to alloc buffer for pd data.\n
        Freq of use: high))  */
    unsigned short sw_afT_pdMax_width;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_pdMax_height),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,65535),
        M4_DEFAULT(768),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(12),
        M4_NOTES(Max pd data height, used to alloc buffer for pd data.\n
        Freq of use: high))  */
    unsigned short sw_afT_pdMax_height;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_pdVerBinning_val),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(13),
        M4_NOTES(Vertical binning setting.\n
        Freq of use: low))  */
    unsigned char sw_afT_pdVerBinning_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_pdFrmInValid_num),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,4),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(14),
        M4_NOTES(Invalid frame number after judge motor is stable.\n
        Freq of use: low))  */
    unsigned char sw_afT_pdFrmInValid_num;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_pdDgainValid_en),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(15),
        M4_NOTES(Weather sensor dgain is valid for pd data.\n
        Freq of use: low))  */
    bool sw_afT_pdDgainValid_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_pdGainMapNorm_en),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(16),
        M4_NOTES(Enable normalize sensor gainmap.\n
        Freq of use: low))  */
    bool sw_afT_pdGainMapNorm_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_pdConfd_mode),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(17),
        M4_NOTES(Mode to caculate confidence.\n
        Freq of use: low))  */
    unsigned char sw_afT_pdConfd_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_pdNoiseSn_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,100),
        M4_DEFAULT(0.207),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(18),
        M4_NOTES(Pd data noise factor0.\n
        Freq of use: low))  */
    float sw_afT_pdNoiseSn_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_pdNoiseRn_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,2),
        M4_RANGE_EX(0,100),
        M4_DEFAULT([0.14, 2.8]),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(19),
        M4_NOTES(Pd data noise factor1.\n
        Freq of use: low))  */
    float sw_afT_pdNoiseRn_val[2];
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_pdNoisePn_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,100),
        M4_DEFAULT(0.25),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(20),
        M4_NOTES(Pd data noise factor2.\n
        Freq of use: low))  */
    float sw_afT_pdNoisePn_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_pdSatVal_ratio),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0.92),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(21),
        M4_NOTES(Saturation value ratio for pd data.\n
        Freq of use: low))  */
    float sw_afT_pdSatVal_ratio;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_pdSatCnt_ratio),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0.06),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(22),
        M4_NOTES(Saturation count ratio for pd data.\n
        Freq of use: low))  */
    float sw_afT_pdSatCnt_ratio;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_pdDiscardRegion_en),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(23),
        M4_NOTES(Weather to discard less texture block.\n
        Freq of use: low))  */
    bool sw_afT_pdDiscardRegion_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_pdLessTexture_ratio),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,5),
        M4_DEFAULT(0.25),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(24),
        M4_NOTES(Ratio to judge less texture block.\n
        Freq of use: low))  */
    float sw_afT_pdLessTexture_ratio;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_pdTarget_offset),
        M4_TYPE(s16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(-4096,4096),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(25),
        M4_NOTES(Offet to adjust infocus position, we should use value in otp.\n
        Freq of use: low))  */
    short sw_afT_pdTarget_offset;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_pdNumJobs),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,64),
        M4_DEFAULT(1),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(26),
        M4_NOTES(Thread number to do calculate.\n
        Freq of use: low))  */
    unsigned short sw_afT_pdNumJobs;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_pdDiscardRegion_en),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(27),
        M4_NOTES(Weather to use cdaf fv to trigger in monitor stat.\n
        Freq of use: low))  */
    bool sw_afT_pdUseFvTrigger;
    /* M4_GENERIC_DESC(
        M4_ALIAS(PdCalibInf),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(28),
        M4_NOTES(Calibration information.\n
        Freq of use: high))  */
    af_pdafCalibInf_t pdCalibInf;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_pdConfdMwinFactor_val),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,225),
        M4_DEFAULT(3),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(29),
        M4_NOTES(Confidence adjust factor in muti-window in roi case.\n
        Freq of use: low))  */
    unsigned short sw_afT_pdConfdMwinFactor_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_pdCenterMinFv_val),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,100000),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(30),
        M4_NOTES(Min fv to switch roi, keep 0 generally.\n
        Freq of use: low))  */
    unsigned int sw_afT_pdCenterMinFv_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_pdCenterMin_ratio),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(31),
        M4_NOTES(Ratio to switch roi, keep 0 generally.\n
        Freq of use: low))  */
    float sw_afT_pdCenterMin_ratio;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_pdHighlight_ratio),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0.5),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(32),
        M4_NOTES(Ratio to judge high light scene.\n
        Freq of use: low))  */
    float sw_afT_pdHighlight_ratio;
    /* M4_GENERIC_DESC(
        M4_ALIAS(pdafStepRatio),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(33),
        M4_NOTES(Pdaf step ratio.))  */
    af_pdafStepRatio_t pdafStepRatio;
    /* M4_GENERIC_DESC(
        M4_ALIAS(pdIsoParaTbl),
        M4_TYPE(struct_list),
        M4_SIZE_EX(1,13),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_DYNAMIC_EX(1),
        M4_ORDER(34),
        M4_NOTES(Parameter table changed with iso.\n
        Freq of use: high))  */
    af_pdafIsoPara_t pdIsoParaTbl[AF_MAX_ISO_LEVEL];
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_pdIsoParaTbl_len),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,13),
        M4_DEFAULT(1),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(35),
        M4_NOTES(Length of parameter table changed with iso.\n
        Freq of use: high))  */
    int sw_afT_pdIsoParaTbl_len;
    /* M4_GENERIC_DESC(
        M4_ALIAS(pdResoInfTbl),
        M4_TYPE(struct_list),
        M4_SIZE_EX(1,16),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_DYNAMIC_EX(1),
        M4_ORDER(36),
        M4_NOTES(Resolution information table.\n
        Freq of use: high))  */
    af_pdafResolution_t pdResoInfTbl[AF_PDAF_RESOLUTION_INFO_TBL_SIZE];
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_pdResoInfTbl_len),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,16),
        M4_DEFAULT(1),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(37),
        M4_NOTES(Length of resolution information table.\n
        Freq of use: high))  */
    int sw_afT_pdResoInfTbl_len;
} af_pdaf_t;

typedef struct af_vcmCfg_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_maxLogicalPos_val),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1023),
        M4_DEFAULT(64),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(1),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Max logical position.\n
        Freq of use: low))  */
    unsigned int sw_afT_maxLogicalPos_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_startCurrent_val),
        M4_TYPE(s32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(-1, 1023),
        M4_DEFAULT(-1),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(1),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(Start current.\n
        Freq of use: low))  */
    int sw_afT_startCurrent_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_ratedCurrent_val),
        M4_TYPE(s32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(-1, 1023),
        M4_DEFAULT(-1),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(1),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(Rated current.\n
        Freq of use: low))  */
    int sw_afT_ratedCurrent_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_stepMode_val),
        M4_TYPE(s32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(-1, 1000),
        M4_DEFAULT(-1),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(1),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(Step mode.\n
        Freq of use: low))  */
    int sw_afT_stepMode_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_extraDelay_val),
        M4_TYPE(s32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(-10000,10000),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(4),
        M4_NOTES(Extra delay to judge motor stable.\n
        Freq of use: high))  */
    int sw_afT_extraDelay_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_postureDiff_ratio),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(1),
        M4_RO(0),
        M4_ORDER(5),
        M4_NOTES(Posture difference ratio.\n
        Freq of use: low))  */
    float sw_afT_postureDiff_ratio;
} af_vcmCfg_t;

typedef struct af_zoomMoveTbl_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_zoomMoveTbl_len),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,32),
        M4_DEFAULT(1),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Length of zoom index table to search zoom move step.\n
        Freq of use: low))  */
    int sw_afT_zoomMoveTbl_len;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_zoomIndex_dot),
        M4_TYPE(s32),
        M4_SIZE_EX(1,32),
        M4_RANGE_EX(0, 1000000),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(Zoom index table to search zoom move step.\n
        Freq of use: low))  */
    int sw_afT_zoomIndex_dot[AF_ZOOM_MOVE_STEP_NUM];
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_zoomMove_step),
        M4_TYPE(s32),
        M4_SIZE_EX(1,32),
        M4_RANGE_EX(0, 1000000),
        M4_DEFAULT(16),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(Zoom move step table when change focal length.\n
        Freq of use: low))  */
    int sw_afT_zoomMove_step[AF_ZOOM_MOVE_STEP_NUM];
} af_zoomMoveTbl_t;

typedef struct af_zoomSearchTbl_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_zoomSearchTbl_len),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,32),
        M4_DEFAULT(1),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Length of zoom point table in Calibration.\n
        Freq of use: low))  */
    int sw_afT_zoomSearchTbl_len;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_zoomSearchPos_dot),
        M4_TYPE(s32),
        M4_SIZE_EX(1,32),
        M4_RANGE_EX(-32768,32767),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(Zoom point table in Calibration.\n
        Freq of use: low))  */
    int sw_afT_zoomSearchPos_dot[AF_ZOOM_SEARCH_POS_NUM_MAX];
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_focusSearchPlusRange_dot),
        M4_TYPE(s32),
        M4_SIZE_EX(1,32),
        M4_RANGE_EX(0,32767),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(Plus range table to search clear point in calibration.\n
        Freq of use: low))  */
    int sw_afT_focusSearchPlusRange_dot[AF_ZOOM_SEARCH_POS_NUM_MAX];
} af_zoomSearchTbl_t;

typedef struct af_zoomFocusTbl_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(zoomMoveTbl),
        M4_TYPE(struct),
        M4_UI_MODULE(array_table_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Zoom move table.\n
        Freq of use: high))  */
    af_zoomMoveTbl_t zoomMoveTbl;
    /* M4_GENERIC_DESC(
        M4_ALIAS(zoomSearchTbl),
        M4_TYPE(struct),
        M4_UI_MODULE(array_table_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(Zoom search table.\n
        Freq of use: high))  */
    af_zoomSearchTbl_t zoomSearchTbl;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_wideModuleDeviate_val),
        M4_TYPE(s32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 1000),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(Module deviated length at wide side.\n
        Freq of use: low))  */
    int sw_afT_wideModuleDeviate_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_teleModuleDeviate_val),
        M4_TYPE(s32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 1000),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(Module deviated length at tele side.\n
        Freq of use: low))  */
    int sw_afT_teleModuleDeviate_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_motorFocusBack_val),
        M4_TYPE(s32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 1000),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(4),
        M4_NOTES(Focue back value when motor change direction.\n
        Freq of use: low))  */
    int sw_afT_motorFocusBack_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_zoomSearchRefCurve_idx),
        M4_TYPE(s32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,32),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(5),
        M4_NOTES(Reference curve index in calibration.\n
        Freq of use: low))  */
    int sw_afT_zoomSearchRefCurve_idx;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_focusSearchMargin_val),
        M4_TYPE(s32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,100000),
        M4_DEFAULT(50),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(6),
        M4_NOTES(Focus search margin to avoid motor conflict with module in calibration.\n
        Freq of use: low))  */
    int sw_afT_focusSearchMargin_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_focusStage1_step),
        M4_TYPE(s32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 100),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(7),
        M4_NOTES(Focus step in stage1.\n
        Freq of use: low))  */
    int sw_afT_focusStage1_step;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_quickFound_ratio),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 1),
        M4_DEFAULT(0.2),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(8),
        M4_NOTES(Focus value drop rate for quick found in calibration.\n
        Freq of use: low))  */
    float sw_afT_quickFound_ratio;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_quickFoundMinFv_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1000000),
        M4_DEFAULT(1000),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(9),
        M4_NOTES(Min focus value for quick found in calibration.\n
        Freq of use: low))  */
    float sw_afT_quickFoundMinFv_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_searchZoomRange_val),
        M4_TYPE(s32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 100000),
        M4_DEFAULT(100),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(10),
        M4_NOTES(Search zoom range in calibration.\n
        Freq of use: low))  */
    int sw_afT_searchZoomRange_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_searchFocusRange_val),
        M4_TYPE(s32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 100000),
        M4_DEFAULT(100),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(11),
        M4_NOTES(Search focus range in calibration.\n
        Freq of use: low))  */
    int sw_afT_searchFocusRange_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_searchEmax_thred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,100000),
        M4_DEFAULT(100),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(12),
        M4_NOTES(Max error threshold in calibration.\n
        Freq of use: low))  */
    float sw_afT_searchEmax_thred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_searchEavg_thred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,100000),
        M4_DEFAULT(100),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(13),
        M4_NOTES(Average error threshold in calibration.\n
        Freq of use: low))  */
    float sw_afT_searchEavg_thred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_isZoomFocusRec_en),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(14),
        M4_NOTES(Weather to reaord zoom focus position.\n
        Freq of use: low))  */
    bool sw_afT_isZoomFocusRec_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_zoomInfoDir_name),
        M4_TYPE(string),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 64),
        M4_DEFAULT("/data/"),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(15),
        M4_NOTES(Zoom information directory.\n
        Freq of use: low))  */
    char sw_afT_zoomInfoDir_name[64];
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_zoomInit_idx),
        M4_TYPE(s32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 100000),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(16),
        M4_NOTES(Zoom index when initialization.\n
        Freq of use: low))  */
    int sw_afT_zoomInit_idx;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_zoomCurveFile_name),
        M4_TYPE(string),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 64),
        M4_DEFAULT("/data/zoomcurve.bin"),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(17),
        M4_NOTES(Zoom curve file.\n
        Freq of use: low))  */
    char sw_afT_zoomCurveFile_name[AF_ZOOM_CURVE_FILENAME_LEN];
} af_zoomFocusTbl_t;

typedef struct af_window_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_afT_win_x),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,2000),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The horizontal offset of af stats win.\n
        Freq of use: high))  */
    unsigned short hw_afT_win_x;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_afT_win_y),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,2000),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The vertical offset of af stats win.\n
        Freq of use: high))  */
    unsigned short hw_afT_win_y;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_afT_win_width),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,2000),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The width of af stats win.\n
        Freq of use: high))  */
    unsigned short hw_afT_win_width;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_afT_win_height),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,2000),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(The height of af stats win.\n
        Freq of use: high))  */
    unsigned short hw_afT_win_height;
} af_window_t;

typedef struct af_defaultMotorPos_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_fixedMode_pos),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 1023),
        M4_DEFAULT(8),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Motor position in fixed mode.\n
        Freq of use: low))  */
    unsigned short sw_afT_fixedMode_pos;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_macroMode_pos),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 1023),
        M4_DEFAULT(32),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(Max motor position in macro mode.\n
        Freq of use: low))  */
    unsigned short sw_afT_macroMode_pos;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_infinityMode_pos),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 1023),
        M4_DEFAULT(32),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(Min motor position in infinity mode.\n
        Freq of use: low))  */
    unsigned short sw_afT_infinityMode_pos;
} af_defaultMotorPos_t;

typedef struct af_common_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_af_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(af_mode_t),
        M4_DEFAULT(af_mode_continue_picture),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(AF mode.\n
        Freq of use: high))  */
    af_mode_t sw_afT_af_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(normalWindow),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The windows scale for normal mode.\n
        Freq of use: high))  */
    af_window_t normalWindow;
    /* M4_GENERIC_DESC(
        M4_ALIAS(videoWindow),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The windows scale for video mode.\n
        Freq of use: high))  */
    af_window_t videoWindow;
    /* M4_GENERIC_DESC(
        M4_ALIAS(defaultMotorPos),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(Default motor position.\n
        Freq of use: low))  */
    af_defaultMotorPos_t defaultMotorPos;
} af_common_t;

typedef struct af_queryMeas_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_queryZoom_idx),
        M4_TYPE(s32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,100000),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Zoom index to get measurement index.\n
        Freq of use: high))  */
    int sw_afT_queryZoom_idx;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_queryIso_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,13),
        M4_RANGE_EX(50, 10000000),
        M4_DEFAULT([50, 100, 200, 400, 800, 1600, 3200, 6400, 12800, 25600, 51200, 102400, 204800]),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(Iso value to get measurement index.\n
        Freq of use: high))  */
    float sw_afT_queryIso_dot[AF_MAX_ISO_LEVEL];
    /* M4_GENERIC_DESC(
        M4_ALIAS(MeasIndex),
        M4_TYPE(s32),
        M4_SIZE_EX(1,13),
        M4_RANGE_EX(0,32),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(Index of af measurement table(normal scene).\n
        Freq of use: high))  */
    int sw_afT_measTblIdx_dot[AF_MAX_ISO_LEVEL];
    /* M4_GENERIC_DESC(
        M4_ALIAS(SpotlightMeasIndex),
        M4_TYPE(s32),
        M4_SIZE_EX(1,13),
        M4_RANGE_EX(0,32),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(Index of af measurement table(spot light scene).\n
        Freq of use: high))  */
    int sw_afT_spotLtMeasTblIdx_dot[AF_MAX_ISO_LEVEL];
} af_queryMeas_t;

typedef struct af_measCfg_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_measCfgTbl_idx),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,31),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Table index.\n
        Freq of use: low))  */
    unsigned int sw_afT_measCfgTbl_idx;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_measCfgV1_band),
        M4_TYPE(f32),
        M4_SIZE_EX(1,2),
        M4_RANGE_EX(0, 1),
        M4_DEFAULT([0.042,0.140]),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(Band range for v1 filter(just for understand easily).\n
        Freq of use: low))  */
    float sw_afT_measCfgV1_band[2];
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_measCfgV2_band),
        M4_TYPE(f32),
        M4_SIZE_EX(1,2),
        M4_RANGE_EX(0, 1),
        M4_DEFAULT([0.042,0.140]),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(Band range for v2 filter(just for understand easily).\n
        Freq of use: low))  */
    float sw_afT_measCfgV2_band[2];
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_measCfgH1_band),
        M4_TYPE(f32),
        M4_SIZE_EX(1,2),
        M4_RANGE_EX(0, 1),
        M4_DEFAULT([0.042,0.140]),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(Band range for h1 filter(just for understand easily).\n
        Freq of use: low))  */
    float sw_afT_measCfgH1_band[2];
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_measCfgH2_band),
        M4_TYPE(f32),
        M4_SIZE_EX(1,2),
        M4_RANGE_EX(0, 1),
        M4_DEFAULT([0.042,0.140]),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(4),
        M4_NOTES(Band range for h2 filter(just for understand easily).\n
        Freq of use: low))  */
    float sw_afT_measCfgH2_band[2];
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_measCfgV1fv_ratio),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 1),
        M4_DEFAULT(0.5),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(5),
        M4_NOTES(Discount ratio for v1 fv, used in optical zoom.\n
        Freq of use: low))  */
    float sw_afT_measCfgV1fv_ratio;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_measCfgV2fv_ratio),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 1),
        M4_DEFAULT(0.5),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(6),
        M4_NOTES(Discount ratio for v2 fv, used in optical zoom.\n
        Freq of use: low))  */
    float sw_afT_measCfgV2fv_ratio;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_measCfgVerFv_ratio),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 1),
        M4_DEFAULT(0.5),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(7),
        M4_NOTES(Discount ratio for v1/v2 fv, used in non-optical zoom.\n
        Freq of use: low))  */
    float sw_afT_measCfgVerFv_ratio;
    /* M4_GENERIC_DESC(
        M4_ALIAS(statsCfg),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(8),
        M4_NOTES(Hardware measurement config.\n
        Freq of use: low))  */
    afStats_cfg_t statsCfg;
} af_measCfg_t;

typedef struct af_meas_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(queryMeasTbl),
        M4_TYPE(struct_list),
        M4_SIZE_EX(1,8),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_DYNAMIC_EX(1),
        M4_ORDER(0),
        M4_NOTES(Index table for search measurement config table.\n
        Freq of use: high))  */
    af_queryMeas_t queryMeasTbl[AF_QUERY_MEAS_TBL_SIZE];
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_queryMeasTbl_len),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,8),
        M4_DEFAULT(1),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(Length of index table for search measurement config table.\n
        Freq of use: high))  */
    int sw_afT_queryMeasTbl_len;
    /* M4_GENERIC_DESC(
        M4_ALIAS(measCfgTbl),
        M4_TYPE(struct_list),
        M4_SIZE_EX(1,32),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_DYNAMIC_EX(1),
        M4_ORDER(2),
        M4_NOTES(Measurement config table.\n
        Freq of use: high))  */
    af_measCfg_t measCfgTbl[AF_MAX_MEAS_CONFIG_NUM];
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_afT_measCfgTbl_len),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,32),
        M4_DEFAULT(1),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(Length of measurement config table.\n
        Freq of use: high))  */
    int sw_afT_measCfgTbl_len;
} af_meas_t;

typedef struct af_param_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(common),
        M4_TYPE(struct),
        M4_SIZE_EX(1,1),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Common config.\n
        Freq of use: high))  */
    af_common_t common;
    /* M4_GENERIC_DESC(
        M4_ALIAS(contrastAf),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_GROUP(contrast_en_group),
        M4_NOTES(Contrast af config in preview mode.\n
        Freq of use: high))  */
    af_contrast_t contrastAf;
    /* M4_GENERIC_DESC(
        M4_ALIAS(videoContrastAf),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_GROUP(contrast_en_group),
        M4_NOTES(Contrast af config in video mode.\n
        Freq of use: high))  */
    af_contrast_t videoContrastAf;
    /* M4_GENERIC_DESC(
        M4_ALIAS(laserAf),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(1),
        M4_RO(0),
        M4_ORDER(3),
        M4_GROUP(laser_en_group),
        M4_NOTES(Laser af config.\n
        Freq of use: high))  */
    af_laser_t laserAf;
    /* M4_GENERIC_DESC(
        M4_ALIAS(pdaf),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(4),
        M4_GROUP(pdaf_en_group),
        M4_NOTES(Pdaf config.\n
        Freq of use: high))  */
    af_pdaf_t pdaf;
    /* M4_GENERIC_DESC(
        M4_ALIAS(vcmCfg),
        M4_TYPE(struct),
        M4_SIZE_EX(1,1),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(5),
        M4_NOTES(Vcm config.\n
        Freq of use: low))  */
    af_vcmCfg_t vcmCfg;
    /* M4_GENERIC_DESC(
        M4_ALIAS(zoomFocusTbl),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(6),
        M4_GROUP(contrast_en_group),
        M4_NOTES(Optical zoom config.\n
        Freq of use: low))  */
    af_zoomFocusTbl_t zoomFocusTbl;
    /* M4_GENERIC_DESC(
        M4_ALIAS(measCfg),
        M4_TYPE(struct),
        M4_SIZE_EX(1,1),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(7),
        M4_NOTES(Af measurement config.\n
        Freq of use: low))  */
    af_meas_t measCfg;
} af_param_t;

RKAIQ_END_DECLARE

#endif
