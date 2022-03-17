/*
 * af_head.h
 *
 *  Copyright (c) 2021 Rockchip Corporation
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

#ifndef __CALIBDBV2_AF_HEADER_H__
#define __CALIBDBV2_AF_HEADER_H__

#include "rk_aiq_comm.h"

RKAIQ_BEGIN_DECLARE

#define CALIBDBV2_MAX_ISO_LEVEL 13

typedef enum CalibDbV2_Af_SearchDir_s {
    CalibDbV2_CAM_AFM_POSITIVE_SEARCH     = 0,
    CalibDbV2_CAM_AFM_NEGATIVE_SEARCH     = 1,
    CalibDbV2_CAM_AFM_ADAPTIVE_SEARCH     = 2
} CalibDbV2_Af_SearchDir_t;

typedef enum CalibDbV2_Af_SS_s {
    CalibDbV2_CAM_AFM_FSS_INVALID         = 0,
    CalibDbV2_CAM_AFM_FSS_FULLRANGE       = 1,    /**< scan the full focus range to find the point of best focus */
    CalibDbV2_CAM_AFM_FSS_HILLCLIMBING    = 2,    /**< use hillclimbing search */
    CalibDbV2_CAM_AFM_FSS_ADAPTIVE_RANGE  = 3,    /**< similar to full range search, but with multiple subsequent scans
                                                       with decreasing range and step size will be performed. */
    CalibDbV2_CAM_AFM_FSS_MUTIWINDOW      = 4,    /**< search by muti-window statistics */
    CalibDbV2_CAM_AFM_FSS_MAX
} CalibDbV2_Af_SS_t;

typedef enum CalibDbV2_AF_MODE_s
{
    CalibDbV2_AF_MODE_NOT_SET = -1,
    CalibDbV2_AF_MODE_AUTO,
    CalibDbV2_AF_MODE_MACRO,
    CalibDbV2_AF_MODE_INFINITY,
    CalibDbV2_AF_MODE_FIXED,
    CalibDbV2_AF_MODE_EDOF,
    CalibDbV2_AF_MODE_CONTINUOUS_VIDEO,
    CalibDbV2_AF_MODE_CONTINUOUS_PICTURE,
    CalibDbV2_AF_MODE_ONESHOT_AFTER_ZOOM,
} CalibDbV2_AF_MODE_t;

typedef struct CalibDbV2_Af_Contrast_s {
    // M4_NUMBER_DESC("Enable", "u8", M4_RANGE(0, 1), "0", M4_DIGIT(0))
    unsigned char           enable;
    // M4_ENUM_DESC("SearchStrategy", "CalibDbV2_Af_SS_t", "CalibDbV2_CAM_AFM_FSS_ADAPTIVE_RANGE")
    CalibDbV2_Af_SS_t         Afss;                       /**< enumeration type for search strategy */
    // M4_ENUM_DESC("FullDir", "CalibDbV2_Af_SearchDir_t", "CalibDbV2_CAM_AFM_ADAPTIVE_SEARCH")
    CalibDbV2_Af_SearchDir_t  FullDir;
    // M4_NUMBER_DESC("FullSteps", "u8", M4_RANGE(0, 65), "0", M4_DIGIT(0))
    unsigned char           FullSteps;
    // M4_ARRAY_DESC("FullRangeTbl", "u16", M4_SIZE(1,65), M4_RANGE(0,64), "0", M4_DIGIT(0), M4_DYNAMIC(0))
    unsigned short          FullRangeTbl[65];             /**< full range search table*/
    // M4_ENUM_DESC("AdaptiveDir", "CalibDbV2_Af_SearchDir_t", "CalibDbV2_CAM_AFM_ADAPTIVE_SEARCH")
    CalibDbV2_Af_SearchDir_t  AdaptiveDir;
    // M4_NUMBER_DESC("AdaptiveSteps", "u8", M4_RANGE(0, 65), "0", M4_DIGIT(0))
    unsigned char           AdaptiveSteps;
    // M4_ARRAY_DESC("AdaptRangeTbl", "u16", M4_SIZE(1,65), M4_RANGE(0,64), "0", M4_DIGIT(0), M4_DYNAMIC(0))
    unsigned short          AdaptRangeTbl[65];            /**< adaptive range search table*/
    // M4_NUMBER_DESC("TrigThers", "f32", M4_RANGE(0,1), "0", M4_DIGIT(3),M4_HIDE(0))
    float                   TrigThers;                    /**< AF trigger threshold */
    // M4_NUMBER_DESC("LumaTrigThers", "f32", M4_RANGE(0,1), "0", M4_DIGIT(3),M4_HIDE(0))
    float                   LumaTrigThers;

    // M4_NUMBER_DESC("StableThers", "f32", M4_RANGE(0,1), "0", M4_DIGIT(3),M4_HIDE(0))
    float                   StableThers;                  /**< AF stable threshold */
    // M4_NUMBER_DESC("StableFrames", "u16", M4_RANGE(0,65535), "0", M4_DIGIT(0),M4_HIDE(0))
    unsigned short          StableFrames;                 /**< AF stable  status must hold frames */
    // M4_NUMBER_DESC("StableTime", "u16", M4_RANGE(0,65535), "0", M4_DIGIT(3),M4_HIDE(0))
    unsigned short          StableTime;                   /**< AF stable status must hold time */

    // M4_NUMBER_DESC("SceneDiffEnable", "u8", M4_RANGE(0, 1), "0", M4_DIGIT(0),M4_HIDE(0))
    unsigned char           SceneDiffEnable;
    // M4_NUMBER_DESC("SceneDiffThers", "f32", M4_RANGE(0,1), "0", M4_DIGIT(3),M4_HIDE(0))
    float                   SceneDiffThers;
    // M4_NUMBER_DESC("SceneDiffBlkThers", "u16", M4_RANGE(0,225), "0", M4_DIGIT(0),M4_HIDE(0))
    unsigned short          SceneDiffBlkThers;
    // M4_NUMBER_DESC("CenterSceneDiffThers", "f32", M4_RANGE(0,1), "0", M4_DIGIT(3),M4_HIDE(0))
    float                   CenterSceneDiffThers;

    // M4_NUMBER_DESC("ValidMaxMinRatio", "f32", M4_RANGE(0,1), "0", M4_DIGIT(3),M4_HIDE(0))
    float                   ValidMaxMinRatio;
    // M4_NUMBER_DESC("ValidValueThers", "f32", M4_RANGE(0,1), "0", M4_DIGIT(3),M4_HIDE(0))
    float                   ValidValueThers;

    // M4_NUMBER_DESC("OutFocusValue", "f32", M4_RANGE(0,1000000), "0", M4_DIGIT(3),M4_HIDE(0))
    float                   OutFocusValue;                /**< out of focus vlaue*/
    // M4_NUMBER_DESC("OutFocusPos", "u16", M4_RANGE(0,65), "0", M4_DIGIT(0),M4_HIDE(0))
    unsigned short          OutFocusPos;                  /**< out of focus position*/

    // M4_NUMBER_DESC("WeightEnable", "u8", M4_RANGE(0, 1), "0", M4_DIGIT(0),M4_HIDE(0))
    unsigned char           WeightEnable;
    // M4_ARRAY_DESC("Weight", "u16", M4_SIZE(15,15), M4_RANGE(0,65535), "0", M4_DIGIT(0), M4_DYNAMIC(0))
    unsigned short          Weight[225];                  /**< weight */

    // M4_NUMBER_DESC("SearchPauseLumaEnable", "u8", M4_RANGE(0, 1), "0", M4_DIGIT(0),M4_HIDE(0))
    unsigned char           SearchPauseLumaEnable;
    // M4_NUMBER_DESC("SearchPauseLumaThers", "f32", M4_RANGE(0,1), "0", M4_DIGIT(3),M4_HIDE(0))
    float                   SearchPauseLumaThers;
    // M4_NUMBER_DESC("StableFrames", "u16", M4_RANGE(0,65535), "0", M4_DIGIT(3),M4_HIDE(0))
    unsigned short          SearchLumaStableFrames;
    // M4_NUMBER_DESC("SearchLumaStableThers", "f32", M4_RANGE(0,1), "0", M4_DIGIT(3),M4_HIDE(0))
    float                   SearchLumaStableThers;

    // M4_NUMBER_DESC("FlatValue", "f32", M4_RANGE(0,65535), "0", M4_DIGIT(3),M4_HIDE(0))
    float                   FlatValue;
} CalibDbV2_Af_Contrast_t;

typedef struct CalibDbV2_Af_Laser_s {
    // M4_NUMBER_DESC("enable", "u8", M4_RANGE(0, 1), "0", M4_DIGIT(0))
    unsigned char enable;
    // M4_ARRAY_DESC("vcmDot", "f32", M4_SIZE(1,7), M4_RANGE(0,64), "0", M4_DIGIT(0), M4_DYNAMIC(0))
    float vcmDot[7];
    // M4_ARRAY_DESC("distanceDot", "f32", M4_SIZE(1,7), M4_RANGE(0,65535), "0", M4_DIGIT(4), M4_DYNAMIC(0))
    float distanceDot[7];
} CalibDbV2_Af_Laser_t;

typedef struct CalibDbV2_Af_Pdaf_s {
    // M4_NUMBER_DESC("enable", "u8", M4_RANGE(0, 1), "0", M4_DIGIT(0))
    unsigned char enable;
} CalibDbV2_Af_Pdaf_t;

typedef struct CalibDbV2_Af_VcmCfg_s {
    // M4_NUMBER_DESC("start current", "s32", M4_RANGE(-1, 1000), "-1", M4_DIGIT(0))
    int start_current;
    // M4_NUMBER_DESC("rated current", "s32", M4_RANGE(-1, 1000), "-1", M4_DIGIT(0))
    int rated_current;
    // M4_NUMBER_DESC("step mode", "s32", M4_RANGE(-1, 1000), "-1", M4_DIGIT(0))
    int step_mode;
    // M4_NUMBER_DESC("extra delay", "s32", M4_RANGE(-10000,10000), "0", M4_DIGIT(0))
    int extra_delay;
} CalibDbV2_Af_VcmCfg_t;

typedef struct CalibDbV2_Af_MeasIsoCfg_s {
    // M4_NUMBER_DESC("iso", "u32", M4_RANGE(0, 1000000), "0", M4_DIGIT(0))
    int iso;
    // M4_NUMBER_DESC("afmThres", "u16", M4_RANGE(0, 255), "4", M4_DIGIT(0))
    unsigned short afmThres;
    // M4_ARRAY_MARK_DESC("Gamma_curve", "u16", M4_SIZE(1,17),  M4_RANGE(0, 1023), "[0,45,108,179,245,344,409,459,500,567,622,676,759,833,896,962,1023]", M4_DIGIT(0), M4_DYNAMIC(0), "curve_table")
    unsigned short gammaY[17];
    // M4_ARRAY_DESC("gaussWeight", "u8", M4_SIZE(1,3), M4_RANGE(0,128), "[16,14,14]", M4_DIGIT(0), M4_DYNAMIC(0))
    unsigned char gaussWeight[3];
} CalibDbV2_Af_MeasIsoCfg_t;

typedef struct CalibDbV2_Af_DefCode_s {
    // M4_NUMBER_DESC("code", "u8", M4_RANGE(0, 64), "0", M4_DIGIT(0))
    unsigned char code;
} CalibDbV2_Af_DefCode_t;

typedef struct CalibDbV2_Af_ZoomFocusTbl_s {
    // M4_NUMBER_DESC("table length", "u32", M4_RANGE(0, 1000000), "0", M4_DIGIT(0))
    int tbl_len;
    // M4_ARRAY_DESC("focal length", "f32", M4_SIZE(1,15), M4_RANGE(0,1000), "0", M4_DIGIT(3), M4_DYNAMIC(1))
    float *focal_length;
    int focal_length_len;
    // M4_ARRAY_DESC("zoom pos", "s32", M4_SIZE(1,15), M4_RANGE(-1000000,1000000), "0", M4_DIGIT(0), M4_DYNAMIC(1))
    int *zoom_pos;
    int zoom_pos_len;
    // M4_ARRAY_DESC("focus infpos", "s32", M4_SIZE(1,256), M4_RANGE(-1000000,1000000), "0", M4_DIGIT(0), M4_DYNAMIC(1))
    int *focus_infpos;
    int focus_infpos_len;
    // M4_ARRAY_DESC("focus macropos", "s32", M4_SIZE(1,256), M4_RANGE(-1000000,1000000), "0", M4_DIGIT(0), M4_DYNAMIC(1))
    int *focus_macropos;
    int focus_macropos_len;
} CalibDbV2_Af_ZoomFocusTbl_t;

typedef struct CalibDbV2_AF_Tuning_Para_s {
    // M4_ENUM_DESC("mode", "CalibDbV2_AF_MODE_t", "CalibDbV2_AF_MODE_CONTINUOUS_PICTURE")
    CalibDbV2_AF_MODE_t af_mode;
    // M4_NUMBER_DESC("win_h_offs", "u16", M4_RANGE(0,2048), "0", M4_DIGIT(0),M4_HIDE(0))
    unsigned short win_h_offs;
    // M4_NUMBER_DESC("win_v_offs", "u16", M4_RANGE(0,2048), "0", M4_DIGIT(0),M4_HIDE(0))
    unsigned short win_v_offs;
    // M4_NUMBER_DESC("win_h_size", "u16", M4_RANGE(0,2048), "0", M4_DIGIT(0),M4_HIDE(0))
    unsigned short win_h_size;
    // M4_NUMBER_DESC("win_v_size", "u16", M4_RANGE(0,2048), "0", M4_DIGIT(0),M4_HIDE(0))
    unsigned short win_v_size;
    // M4_STRUCT_DESC("fixed mode", "normal_ui_style")
    CalibDbV2_Af_DefCode_t fixed_mode;
    // M4_STRUCT_DESC("macro mode", "normal_ui_style")
    CalibDbV2_Af_DefCode_t macro_mode;
    // M4_STRUCT_DESC("infinity mode", "normal_ui_style")
    CalibDbV2_Af_DefCode_t infinity_mode;
    // M4_STRUCT_DESC("contrast af", "normal_ui_style")
    CalibDbV2_Af_Contrast_t contrast_af;
    // M4_STRUCT_DESC("laser af", "normal_ui_style")
    CalibDbV2_Af_Laser_t laser_af;
    // M4_STRUCT_DESC("pdaf", "normal_ui_style")
    CalibDbV2_Af_Pdaf_t pdaf;
    // M4_STRUCT_DESC("vcmcfg", "normal_ui_style")
    CalibDbV2_Af_VcmCfg_t vcmcfg;
    // M4_STRUCT_LIST_DESC("meas iso config", M4_SIZE(1,13), "normal_ui_style")
    CalibDbV2_Af_MeasIsoCfg_t measiso_cfg[CALIBDBV2_MAX_ISO_LEVEL];
    // M4_STRUCT_DESC("zoomfocus_tbl", "normal_ui_style")
    CalibDbV2_Af_ZoomFocusTbl_t zoomfocus_tbl;
} CalibDbV2_AF_Tuning_Para_t;

typedef struct {
    // M4_STRUCT_DESC("TuningPara", "normal_ui_style")
    CalibDbV2_AF_Tuning_Para_t TuningPara;
} CalibDbV2_AF_t;

RKAIQ_END_DECLARE

#endif
