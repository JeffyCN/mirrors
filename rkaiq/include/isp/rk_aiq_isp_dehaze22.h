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

#ifndef _RK_AIQ_PARAM_DEHAZE22_H_
#define _RK_AIQ_PARAM_DEHAZE22_H_

#define DHAZ_ENHANCE_CURVE_KNOTS_NUM 17
#define DHAZ_HIST_WR_CURVE_NUM   17
#define DHAZ_SIGMA_IDX_NUM     15
#define DHAZ_SIGMA_LUT_NUM     17
#define DHAZ_HIST_WR_NUM       64

typedef struct dehaze_mhist_wr_semiauto_s {
    // M4_NUMBER_DESC("clim0", "f32", M4_RANGE(0.1,32), "1", M4_DIGIT(4))
    float clim0;
    // M4_NUMBER_DESC("clim1", "f32", M4_RANGE(0.1,32), "1", M4_DIGIT(4))
    float clim1;
    // M4_NUMBER_DESC("dark_th", "f32", M4_RANGE(0,1023), "1", M4_DIGIT(0))
    float dark_th;
} dehaze_mhist_wr_semiauto_t;

typedef enum dehaze_HistWrMode_e {
    dehaze_HIST_WR_AUTO     = 0,
    dehaze_HIST_WR_MANUAL   = 1,
    dehaze_HIST_WR_SEMIAUTO = 2,
} dehaze_HistWrMode_t;

typedef struct dehaze_Manual_curve_s {
    // M4_NUMBER_MARK_DESC("CtrlData", "f32", M4_RANGE(0,1000000000), "0", M4_DIGIT(4), "index1")
    float CtrlData;
    // M4_ARRAY_MARK_DESC("curve_x", "u32", M4_SIZE(1,17),  M4_RANGE(0, 1023), "[0, 64, 128, 192, 256, 320, 384, 448, 512, 576, 640, 704, 768, 832, 896, 960, 1023]", M4_DIGIT(0), M4_DYNAMIC(0), 0)
    int curve_x[DHAZ_HIST_WR_CURVE_NUM];
    // M4_ARRAY_MARK_DESC("curve_y", "u32", M4_SIZE(1,17),  M4_RANGE(0, 1023), "[0, 64, 128, 192, 256, 320, 384, 448, 512, 576, 640, 704, 768, 832, 896, 960, 1023]", M4_DIGIT(0), M4_DYNAMIC(0), 0)
    int curve_y[DHAZ_HIST_WR_CURVE_NUM];
} dehaze_Manual_curve_t;

typedef struct dehaze_hist_wr_semiauto_s {
    // M4_ARRAY_DESC("CtrlData", "f32", M4_SIZE(1,13), M4_RANGE(0,10000000), "[0, 0.005, 0.01, 0.05, 0.1, 0.15, 0.2, 0.3, 0.4, 0.5, 0.6, 0.8, 1]",M4_DIGIT(4), M4_DYNAMIC(0), 0)
    float CtrlData;
    // M4_ARRAY_DESC("clim0", "f32", M4_SIZE(1,13), M4_RANGE(0.1,32), "[1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1]",M4_DIGIT(4), M4_DYNAMIC(0), 0)
    float clim0;
    // M4_ARRAY_DESC("clim1", "f32", M4_SIZE(1,13), M4_RANGE(0.1,32), "[1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1]",M4_DIGIT(4), M4_DYNAMIC(0), 0)
    float clim1;
    // M4_ARRAY_DESC("dark_th", "f32", M4_SIZE(1,13), M4_RANGE(0,1023), "[0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]",M4_DIGIT(0), M4_DYNAMIC(0), 0)
    float dark_th;
} dehaze_hist_wr_semiauto_t;

typedef struct dehaze_HistWr_dyn_s {
    // M4_STRUCT_LIST_DESC("manual_curve", M4_SIZE(1,13), "normal_ui_style")
    dehaze_Manual_curve_t manual_curve;
    // M4_ARRAY_TABLE_DESC("semiauto_curve", "array_table_ui","none")
    dehaze_hist_wr_semiauto_t semiauto_curve;
} dehaze_HistWr_dyn_t;

typedef struct dehaze_HistWr_sta_s {
    // M4_ENUM_DESC("mode", "dehaze_HistWrMode_t", "dehaze_HIST_WR_MANUAL")
    dehaze_HistWrMode_t mode;
    unsigned short hist_wr[DHAZ_HIST_WR_NUM];
} dehaze_HistWr_sta_t;

typedef struct dehaze_HistDataV11_s {
    // M4_ARRAY_DESC("CtrlData", "f32", M4_SIZE(1,13), M4_RANGE(0,10000000), "[0, 0.005, 0.01, 0.05, 0.1, 0.15, 0.2, 0.3, 0.4, 0.5, 0.6, 0.8, 1]",M4_DIGIT(4), M4_DYNAMIC(0), 0)
    float CtrlData;
    // M4_ARRAY_DESC("hist_gratio", "f32", M4_SIZE(1,13), M4_RANGE(0,32), "[2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2]",M4_DIGIT(4), M4_DYNAMIC(0), 0)
    float hist_gratio;
    // M4_ARRAY_DESC("hist_th_off", "f32", M4_SIZE(1,13), M4_RANGE(0,255), "[64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64]",M4_DIGIT(4), M4_DYNAMIC(0), 0)
    float hist_th_off;
    // M4_ARRAY_DESC("hist_k", "f32", M4_SIZE(1,13), M4_RANGE(0,8), "[2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2]",M4_DIGIT(4), M4_DYNAMIC(0), 0)
    float hist_k;
    // M4_ARRAY_DESC("hist_min", "f32", M4_SIZE(1,13), M4_RANGE(0,2), "[0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015, 0.015]",M4_DIGIT(4), M4_DYNAMIC(0), 0)
    float hist_min;
    // M4_ARRAY_DESC("hist_scale", "f32", M4_SIZE(1,13), M4_RANGE(0,32), "[0.09, 0.09, 0.09, 0.09, 0.09, 0.09, 0.09, 0.09, 0.09, 0.09, 0.09, 0.09, 0.09]",M4_DIGIT(4), M4_DYNAMIC(0), 0)
    float hist_scale;
    // M4_ARRAY_DESC("cfg_gratio", "f32", M4_SIZE(1,13), M4_RANGE(0,32), "[2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2]",M4_DIGIT(4), M4_DYNAMIC(0), 0)
    float cfg_gratio;
} dehaze_HistData_t;

typedef struct dehaze_Hist_setting_s {
    // M4_BOOL_DESC("en", "1")
    bool en;
    // M4_BOOL_DESC("hist_para_en", "1")
    bool hist_para_en;
    // M4_STRUCT_DESC("hist_wr", "normal_ui_style")
    dehaze_HistWr_sta_t hist_sta_wr;
} dehaze_Hist_setting_t;

typedef struct dehaze_EnhanceDataV12_s {
    // M4_NUMBER_MARK_DESC("CtrlData", "f32", M4_RANGE(0,1000000000), "0", M4_DIGIT(4), "index1")
    float CtrlData;
    // M4_ARRAY_MARK_DESC("enhance_curve", "f32", M4_SIZE(1,17),  M4_RANGE(0, 1023), "[0, 64, 128, 192, 256, 320, 384, 448, 512, 576, 640, 704, 768, 832, 896, 960, 1023]", M4_DIGIT(0), M4_DYNAMIC(0), 0)
    float enhance_curve[DHAZ_ENHANCE_CURVE_KNOTS_NUM];
    // M4_ARRAY_MARK_DESC("enh_luma", "f32", M4_SIZE(1,17),  M4_RANGE(0, 16), "[1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1]", M4_DIGIT(2), M4_DYNAMIC(0), 0)
    float enh_luma[DHAZ_ENHANCE_CURVE_KNOTS_NUM];
    // M4_NUMBER_DESC("enhance_value", "f32", M4_RANGE(0,16), "1", M4_DIGIT(4))
    float enhance_value;
    // M4_NUMBER_DESC("enhance_chroma", "f32", M4_RANGE(0,16), "1", M4_DIGIT(4))
    float enhance_chroma;
} dehaze_EnhanceData_t;

typedef struct dehaze_Enhance_Setting_s {
    // M4_BOOL_DESC("en", "1")
    bool en;
    // M4_BOOL_DESC("color_deviate_en", "0")
    bool color_deviate_en;
    // M4_BOOL_DESC("enh_luma_en", "0")
    bool enh_luma_en;
} dehaze_Enhance_Setting_t;

typedef struct dehaze_DehazeDataV11_s {
    // M4_ARRAY_DESC("CtrlData", "f32", M4_SIZE(1,13), M4_RANGE(0,10000000), "[0, 0.005, 0.01, 0.05, 0.1, 0.15, 0.2, 0.3, 0.4, 0.5, 0.6, 0.8, 1]",M4_DIGIT(4), M4_DYNAMIC(0), 0)
    float CtrlData;
    // M4_ARRAY_DESC("dc_min_th", "f32", M4_SIZE(1,13), M4_RANGE(0,255), "[64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64]",M4_DIGIT(4), M4_DYNAMIC(0), 0)
    float dc_min_th;
    // M4_ARRAY_DESC("dc_max_th", "f32", M4_SIZE(1,13), M4_RANGE(0,255), "[192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192]",M4_DIGIT(4), M4_DYNAMIC(0), 0)
    float dc_max_th;
    // M4_ARRAY_DESC("yhist_th", "f32", M4_SIZE(1,13), M4_RANGE(0,255), "[249, 249, 249, 249, 249, 249, 249, 249, 249, 249, 249, 249, 249]",M4_DIGIT(4), M4_DYNAMIC(0), 0)
    float yhist_th;
    // M4_ARRAY_DESC("yblk_th", "f32", M4_SIZE(1,13), M4_RANGE(0,512), "[0.002, 0.002, 0.002, 0.002, 0.002, 0.002, 0.002, 0.002, 0.002, 0.002, 0.002, 0.002, 0.002]",M4_DIGIT(4), M4_DYNAMIC(0), 0)
    float yblk_th;
    // M4_ARRAY_DESC("dark_th", "f32", M4_SIZE(1,13), M4_RANGE(0,255), "[250, 250, 250, 250, 250, 250, 250, 250, 250 250, 250, 250, 250]",M4_DIGIT(4), M4_DYNAMIC(0), 0)
    float dark_th;
    // M4_ARRAY_DESC("bright_min", "f32", M4_SIZE(1,13), M4_RANGE(0,255), "[180, 180, 180, 180, 180, 180, 180, 180, 180, 180, 180, 180, 180]",M4_DIGIT(4), M4_DYNAMIC(0), 0)
    float bright_min;
    // M4_ARRAY_DESC("bright_max", "f32", M4_SIZE(1,13), M4_RANGE(0,255), "[240, 240, 240, 240, 240, 240, 240, 240, 240, 240, 240, 240, 240]",M4_DIGIT(4), M4_DYNAMIC(0), 0)
    float bright_max;
    // M4_ARRAY_DESC("wt_max", "f32", M4_SIZE(1,13), M4_RANGE(0,1), "[0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9]",M4_DIGIT(4), M4_DYNAMIC(0), 0)
    float wt_max;
    // M4_ARRAY_DESC("air_min", "f32", M4_SIZE(1,13), M4_RANGE(0,255), "[200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200]",M4_DIGIT(4), M4_DYNAMIC(0), 0)
    float air_min;
    // M4_ARRAY_DESC("air_max", "f32", M4_SIZE(1,13), M4_RANGE(0,255), "[250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250]",M4_DIGIT(4), M4_DYNAMIC(0), 0)
    float air_max;
    // M4_ARRAY_DESC("tmax_base", "f32", M4_SIZE(1,13), M4_RANGE(0,255), "[125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125]",M4_DIGIT(4), M4_DYNAMIC(0), 0)
    float tmax_base;
    // M4_ARRAY_DESC("tmax_off", "f32", M4_SIZE(1,13), M4_RANGE(0,1), "[0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1]",M4_DIGIT(4), M4_DYNAMIC(0), 0)
    float tmax_off;
    // M4_ARRAY_DESC("tmax_max", "f32", M4_SIZE(1,13), M4_RANGE(0,1), "[0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8]",M4_DIGIT(4), M4_DYNAMIC(0), 0)
    float tmax_max;
    // M4_ARRAY_DESC("cfg_wt", "f32", M4_SIZE(1,13), M4_RANGE(0,1), "[0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8]",M4_DIGIT(4), M4_DYNAMIC(0), 0)
    float cfg_wt;
    // M4_ARRAY_DESC("cfg_air", "f32", M4_SIZE(1,13), M4_RANGE(0,255), "[210, 210, 210, 210, 210, 210, 210, 210, 210, 210, 210, 210, 210]",M4_DIGIT(4), M4_DYNAMIC(0), 0)
    float cfg_air;
    // M4_ARRAY_DESC("cfg_tmax", "f32", M4_SIZE(1,13), M4_RANGE(0,1), "[0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2]",M4_DIGIT(4), M4_DYNAMIC(0), 0)
    float cfg_tmax;
    // M4_ARRAY_DESC("dc_weitcur", "f32", M4_SIZE(1,13), M4_RANGE(0,1), "[0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]",M4_DIGIT(4), M4_DYNAMIC(0), 0)
    float dc_weitcur;
    // M4_ARRAY_DESC("bf_weight", "f32", M4_SIZE(1,13), M4_RANGE(0,1), "[0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5]",M4_DIGIT(4), M4_DYNAMIC(0), 0)
    float bf_weight;
    // M4_ARRAY_DESC("range_sigma", "f32", M4_SIZE(1,13), M4_RANGE(0,1), "[0.14, 0.14, 0.14, 0.14, 0.14, 0.14, 0.14, 0.14, 0.14, 0.14, 0.14, 0.14, 0.14]",M4_DIGIT(4), M4_DYNAMIC(0), 0)
    float range_sigma;
    // M4_ARRAY_DESC("space_sigma_pre", "f32", M4_SIZE(1,13), M4_RANGE(0,1), "[0.14, 0.14, 0.14, 0.14, 0.14, 0.14, 0.14, 0.14, 0.14, 0.14, 0.14, 0.14, 0.14]",M4_DIGIT(4), M4_DYNAMIC(0), 0)
    float space_sigma_pre;
    // M4_ARRAY_DESC("space_sigma_cur", "f32", M4_SIZE(1,13), M4_RANGE(0,1), "[0.14, 0.14, 0.14, 0.14, 0.14, 0.14, 0.14, 0.14, 0.14, 0.14, 0.14, 0.14, 0.14]",M4_DIGIT(4), M4_DYNAMIC(0), 0)
    float space_sigma_cur;
} dehaze_DehazeData_t;

typedef struct dehaze_Dehaze_Setting_s {
    // M4_BOOL_DESC("en", "1")
    bool en;
    // M4_BOOL_DESC("air_lc_en", "1")
    bool air_lc_en;
    // M4_NUMBER_DESC("stab_fnum", "f32", M4_RANGE(0,31), "8.0", M4_DIGIT(0))
    float stab_fnum;
    // M4_NUMBER_DESC("iir_sigma", "f32", M4_RANGE(1,255), "6.0", M4_DIGIT(0))
    float iir_sigma;
    // M4_NUMBER_DESC("wt_sigma", "f32", M4_RANGE(0,256), "8.0", M4_DIGIT(4))
    float iir_wt_sigma;
    // M4_NUMBER_DESC("air_sigma", "f32", M4_RANGE(4,1024), "120.0", M4_DIGIT(4))
    float iir_air_sigma;
    // M4_NUMBER_DESC("tmax_sigma", "f32", M4_RANGE(0.0004,1), "0.0100", M4_DIGIT(4))
    float iir_tmax_sigma;
    // M4_NUMBER_DESC("pre_wet", "f32", M4_RANGE(0,16), "8", M4_DIGIT(0))
    float iir_pre_wet;
    unsigned char gaus_h0;
    unsigned char gaus_h1;
    unsigned char gaus_h2;
} dehaze_Dehaze_Setting_t;

typedef struct {
    // M4_STRUCT_DESC("hist_dyn_wr", "normal_ui_style")
    dehaze_HistWr_dyn_t hist_dyn_wr;
    // M4_ARRAY_TABLE_DESC("HistData", "array_table_ui","none")
    dehaze_HistData_t HistData;
    // M4_STRUCT_LIST_DESC("EnhanceData", M4_SIZE(1,13), "normal_ui_style")
    dehaze_EnhanceData_t EnhanceData;
    // M4_ARRAY_TABLE_DESC("DehazeData", "array_table_ui", "none")
    dehaze_DehazeData_t DehazeData;
} dehaze_params_dyn_t;

typedef enum dehaze_CtrlDataType_e {
    dehaze_CTRLDATATYPE_ENVLV   = 0,
    dehaze_CTRLDATATYPE_ISO     = 1,
} dehaze_CtrlDataType_t;

typedef struct {
    // M4_ENUM_DESC("CtrlDataType", "dehaze_CtrlDataType_t", "dehaze_CTRLDATATYPE_ISO")
    dehaze_CtrlDataType_t CtrlDataType;
    // M4_NUMBER_DESC("cfg_alpha", "f32", M4_RANGE(0,1), "1", M4_DIGIT(4))
    float cfg_alpha;
    // M4_NUMBER_DESC("sw_dehaze_byPass_thred", "f32", M4_RANGE(0,1), "0", M4_DIGIT(4))
    float sw_dehaze_byPass_thred;
    // M4_STRUCT_DESC("dehaze_setting", "normal_ui_style")
    dehaze_Dehaze_Setting_t dehaze_setting;
    // M4_STRUCT_DESC("enhance_setting", "normal_ui_style")
    dehaze_Enhance_Setting_t enhance_setting;
    // M4_STRUCT_DESC("hist_setting", "normal_ui_style")
    dehaze_Hist_setting_t hist_setting;
    unsigned char dc_en;
    unsigned char enhance_en;
    unsigned char hist_en;
    unsigned char round_en;
    unsigned short sigma_lut[DHAZ_SIGMA_LUT_NUM];
    unsigned char sigma_idx[DHAZ_SIGMA_IDX_NUM];
    int soft_wr_en;
} dehaze_params_static_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(static_param),
        M4_TYPE(struct),
        M4_UI_MODULE(static_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The static params of dehaze module))  */
    dehaze_params_static_t sta;
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
