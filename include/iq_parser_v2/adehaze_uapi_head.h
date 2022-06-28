/*
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

#ifndef __ADEHAZE_UAPI_HEAD_H__
#define __ADEHAZE_UAPI_HEAD_H__

#include "adehaze_head.h"

typedef struct mDehazeDataV21_s {
    // M4_NUMBER_DESC("dc_min_th", "f32", M4_RANGE(0,255), "64", M4_DIGIT(4))
    float dc_min_th;
    // M4_NUMBER_DESC("dc_max_th", "f32", M4_RANGE(0,255), "192", M4_DIGIT(4))
    float dc_max_th;
    // M4_NUMBER_DESC("yhist_th", "f32", M4_RANGE(0,255), "249", M4_DIGIT(4))
    float yhist_th;
    // M4_NUMBER_DESC("yblk_th", "f32", M4_RANGE(0,512), "0.002", M4_DIGIT(4))
    float yblk_th;
    // M4_NUMBER_DESC("dark_th", "f32", M4_RANGE(0,255), "250", M4_DIGIT(4))
    float dark_th;
    // M4_NUMBER_DESC("bright_min", "f32", M4_RANGE(0,255), "180", M4_DIGIT(4))
    float bright_min;
    // M4_NUMBER_DESC("bright_max", "f32", M4_RANGE(0,255), "240", M4_DIGIT(4))
    float bright_max;
    // M4_NUMBER_DESC("wt_max", "f32", M4_RANGE(0,1), "0.9", M4_DIGIT(4))
    float wt_max;
    // M4_NUMBER_DESC("air_min", "f32", M4_RANGE(0,255), "200", M4_DIGIT(4))
    float air_min;
    // M4_NUMBER_DESC("air_max", "f32", M4_RANGE(0,255), "250", M4_DIGIT(4))
    float air_max;
    // M4_NUMBER_DESC("tmax_base", "f32", M4_RANGE(0,255), "125", M4_DIGIT(4))
    float tmax_base;
    // M4_NUMBER_DESC("tmax_off", "f32", M4_RANGE(0,1), "0.1", M4_DIGIT(4))
    float tmax_off;
    // M4_NUMBER_DESC("tmax_max", "f32", M4_RANGE(0,1), "0.8", M4_DIGIT(4))
    float tmax_max;
    // M4_NUMBER_DESC("cfg_wt", "f32", M4_RANGE(0,1), "0.8", M4_DIGIT(4))
    float cfg_wt;
    // M4_NUMBER_DESC("cfg_air", "f32", M4_RANGE(0,255), "210", M4_DIGIT(4))
    float cfg_air;
    // M4_NUMBER_DESC("cfg_tmax", "f32", M4_RANGE(0,1), "0.2", M4_DIGIT(4))
    float cfg_tmax;
    // M4_NUMBER_DESC("dc_weitcur", "f32", M4_RANGE(0,1), "1", M4_DIGIT(4))
    float dc_weitcur;
    // M4_NUMBER_DESC("bf_weight", "f32", M4_RANGE(0,1), "0.5", M4_DIGIT(4))
    float bf_weight;
    // M4_NUMBER_DESC("range_sigma", "f32", M4_RANGE(0,1), "0.04", M4_DIGIT(4))
    float range_sigma;
    // M4_NUMBER_DESC("space_sigma_pre", "f32", M4_RANGE(0,1), "0.4", M4_DIGIT(4))
    float space_sigma_pre;
    // M4_NUMBER_DESC("space_sigma_cur", "f32", M4_RANGE(0,1), "0.8", M4_DIGIT(4))
    float space_sigma_cur;
} mDehazeDataV21_t;

typedef struct mDehaze_Setting_V21_s {
    // M4_BOOL_DESC("en", "1")
    bool en;
    // M4_BOOL_DESC("air_lc_en", "1")
    bool air_lc_en;
    // M4_NUMBER_DESC("stab_fnum", "f32", M4_RANGE(0,31), "8.0", M4_DIGIT(4))
    float stab_fnum;
    // M4_NUMBER_DESC("sigma", "f32", M4_RANGE(0,255), "6.0", M4_DIGIT(4))
    float sigma;
    // M4_NUMBER_DESC("wt_sigma", "f32", M4_RANGE(0,256), "8.0", M4_DIGIT(4))
    float wt_sigma;
    // M4_NUMBER_DESC("air_sigma", "f32", M4_RANGE(0,255), "120.0", M4_DIGIT(4))
    float air_sigma;
    // M4_NUMBER_DESC("tmax_sigma", "f32", M4_RANGE(0,1), "0.0100", M4_DIGIT(4))
    float tmax_sigma;
    // M4_NUMBER_DESC("pre_wet", "f32", M4_RANGE(0,1), "0.0100", M4_DIGIT(4))
    float pre_wet;
    // M4_STRUCT_DESC("DehazeData", "normal_ui_style")
    mDehazeDataV21_t DehazeData;
} mDehaze_Setting_V21_t;

typedef struct mEnhanceDataV21_s {
    // M4_NUMBER_DESC("enhance_value", "f32", M4_RANGE(0,32), "1", M4_DIGIT(4))
    float enhance_value;
    // M4_NUMBER_DESC("enhance_chroma", "f32", M4_RANGE(0,32), "1", M4_DIGIT(4))
    float enhance_chroma;
} mEnhanceDataV21_t;

typedef struct mEnhance_Setting_V21_s {
    // M4_BOOL_DESC("en", "1")
    bool en;
    // M4_ARRAY_DESC("enhance_curve", "f32", M4_SIZE(1,17),  M4_RANGE(0, 1024), "[0,64,128,192,256,320,384,448,512,576,640,704,768,832,896,960,1023]", M4_DIGIT(0), M4_DYNAMIC(0))
    float enhance_curve[CALIBDB_ADEHAZE_ENHANCE_CURVE_KNOTS_NUM];
    // M4_STRUCT_DESC("EnhanceData", "normal_ui_style")
    mEnhanceDataV21_t EnhanceData;
} mEnhance_Setting_V21_t;

typedef struct mHistDataV21_s {
    // M4_NUMBER_DESC("hist_gratio", "f32", M4_RANGE(0,32), "4", M4_DIGIT(4))
    float hist_gratio;
    // M4_NUMBER_DESC("hist_th_off", "f32", M4_RANGE(0,255), "64", M4_DIGIT(4))
    float hist_th_off;
    // M4_NUMBER_DESC("hist_k", "f32", M4_RANGE(0,8), "2", M4_DIGIT(4))
    float hist_k;
    // M4_NUMBER_DESC("hist_min", "f32", M4_RANGE(0,2), "0.016", M4_DIGIT(4))
    float hist_min;
    // M4_NUMBER_DESC("hist_scale", "f32", M4_RANGE(0,32), "0.09", M4_DIGIT(4))
    float hist_scale;
    // M4_NUMBER_DESC("cfg_gratio", "f32", M4_RANGE(0,32), "2", M4_DIGIT(4))
    float cfg_gratio;
} mHistDataV21_t;

typedef struct mHist_setting_V21_s {
    // M4_BOOL_DESC("en", "1")
    bool en;
    // M4_BOOL_DESC("hist_para_en", "1")
    bool hist_para_en;
    // M4_STRUCT_DESC("HistData", "normal_ui_style")
    mHistDataV21_t HistData;
} mHist_setting_V21_t;

typedef struct mDehazeAttr_s {
    // M4_BOOL_DESC("Enable", "1")
    bool Enable;
    // M4_NUMBER_DESC("cfg_alpha", "f32", M4_RANGE(0,1), "1", M4_DIGIT(4))
    float cfg_alpha;
    // M4_STRUCT_DESC("dehaze_setting", "normal_ui_style")
    mDehaze_Setting_V21_t dehaze_setting;
    // M4_STRUCT_DESC("enhance_setting", "normal_ui_style")
    mEnhance_Setting_V21_t enhance_setting;
    // M4_STRUCT_DESC("hist_setting", "normal_ui_style")
    mHist_setting_V21_t hist_setting;
    // M4_ARRAY_DESC("sigma_curve", "f32", M4_SIZE(1,5), M4_RANGE(-65535.0, 65535), "0.0", M4_DIGIT(6), M4_DYNAMIC(0))
    double sigma_curve[5];
} mDehazeAttr_t;

#endif /*__ADEHAZE_UAPI_HEAD_H__*/
