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
#define DHAZ_HIST_WR_CURVE_NUM   17
#define DHAZ_SIGMA_IDX_NUM     15
#define DHAZ_SIGMA_LUT_NUM     17
#define DHAZ_HIST_WR_NUM       64
#define HIST_OUTMERGE_ALPHA_NUM 17

typedef struct Dehaze_Data_s {
    // M4_ARRAY_DESC("hw_dehaze_darkCh_minThed", "u8", M4_SIZE(1,13), M4_RANGE(0,255), "[64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64]",M4_DIGIT(0), M4_DYNAMIC(0), 0)
    uint8_t hw_dehaze_darkCh_minThed;
    // M4_ARRAY_DESC("hw_dehaze_darkCh_maxThed", "u8", M4_SIZE(1,13), M4_RANGE(0,255), "[192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192]",M4_DIGIT(0), M4_DYNAMIC(0), 0)
    uint8_t hw_dehaze_darkCh_maxThed;
    // M4_ARRAY_DESC("hw_dehaze_lumaCount_maxThed", "u8", M4_SIZE(1,13), M4_RANGE(0,255), "[249, 249, 249, 249, 249, 249, 249, 249, 249, 249, 249, 249, 249]",M4_DIGIT(0), M4_DYNAMIC(0), 0)
    uint8_t hw_dehaze_lumaCount_maxThed;
    // M4_ARRAY_DESC("sw_dehaze_lumaCount_minRatio", "u8", M4_SIZE(1,13), M4_RANGE(0,512), "[0.002, 0.002, 0.002, 0.002, 0.002, 0.002, 0.002, 0.002, 0.002, 0.002, 0.002, 0.002, 0.002]",M4_DIGIT(4), M4_DYNAMIC(0), 0)
    uint8_t sw_dehaze_lumaCount_minRatio;
    // M4_ARRAY_DESC("hw_dehaze_darkArea_thed	", "u8", M4_SIZE(1,13), M4_RANGE(0,255), "[250, 250, 250, 250, 250, 250, 250, 250, 250 250, 250, 250, 250]",M4_DIGIT(0), M4_DYNAMIC(0), 0)
    uint8_t hw_dehaze_darkArea_thed;
    // M4_ARRAY_DESC("hw_dehaze_bright_minLimit", "u8", M4_SIZE(1,13), M4_RANGE(0,255), "[180, 180, 180, 180, 180, 180, 180, 180, 180, 180, 180, 180, 180]",M4_DIGIT(0), M4_DYNAMIC(0), 0)
    uint8_t hw_dehaze_bright_minLimit;
    // M4_ARRAY_DESC("hw_dehaze_bright_maxLimit", "u8", M4_SIZE(1,13), M4_RANGE(0,255), "[240, 240, 240, 240, 240, 240, 240, 240, 240, 240, 240, 240, 240]",M4_DIGIT(0), M4_DYNAMIC(0), 0)
    uint8_t hw_dehaze_bright_maxLimit;
    // M4_ARRAY_DESC("sw_dehaze_invContrast_scale", "f32", M4_SIZE(1,13), M4_RANGE(0,1), "[0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9]",M4_DIGIT(4), M4_DYNAMIC(0), 0)
    float sw_dehaze_invContrast_scale;
    // M4_ARRAY_DESC("hw_dehaze_airLight_minLimit", "u8", M4_SIZE(1,13), M4_RANGE(0,255), "[200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200]",M4_DIGIT(4), M4_DYNAMIC(0), 0)
    uint8_t hw_dehaze_airLight_minLimit;
    // M4_ARRAY_DESC("hw_dehaze_airLight_maxLimit", "u8", M4_SIZE(1,13), M4_RANGE(0,255), "[250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250]",M4_DIGIT(4), M4_DYNAMIC(0), 0)
    uint8_t hw_dehaze_airLight_maxLimit;
    // M4_ARRAY_DESC("sw_dehaze_airLight_scale", "u8", M4_SIZE(1,13), M4_RANGE(0,255), "[125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125]",M4_DIGIT(4), M4_DYNAMIC(0), 0)
    uint8_t sw_dehaze_airLight_scale;
    // M4_ARRAY_DESC("sw_dehaze_transRatio_offset", "f32", M4_SIZE(1,13), M4_RANGE(0,1), "[0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1]",M4_DIGIT(4), M4_DYNAMIC(0), 0)
    float sw_dehaze_transRatio_offset;
    // M4_ARRAY_DESC("sw_dehaze_transRatio_maxLimit", "f32", M4_SIZE(1,13), M4_RANGE(0,1), "[0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8]",M4_DIGIT(4), M4_DYNAMIC(0), 0)
    float sw_dehaze_transRatio_maxLimit;
    // M4_ARRAY_DESC("sw_dehaze_paramMerge_alpha", "u8", M4_SIZE(1,13), M4_RANGE(0,255), "[0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]",M4_DIGIT(0), M4_DYNAMIC(0), 0)
    uint8_t sw_dehaze_paramMerge_alpha;
    // M4_ARRAY_DESC("sw_dehaze_userInvContrast", "f32", M4_SIZE(1,13), M4_RANGE(0,1), "[0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8]",M4_DIGIT(4), M4_DYNAMIC(0), 0)
    float sw_dehaze_userInvContrast;
    // M4_ARRAY_DESC("hw_dehaze_userAirLight", "u8", M4_SIZE(1,13), M4_RANGE(0,255), "[210, 210, 210, 210, 210, 210, 210, 210, 210, 210, 210, 210, 210]",M4_DIGIT(4), M4_DYNAMIC(0), 0)
    uint8_t hw_dehaze_userAirLight;
    // M4_ARRAY_DESC("sw_dehaze_userTransRatio", "f32", M4_SIZE(1,13), M4_RANGE(0,1), "[0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2]",M4_DIGIT(4), M4_DYNAMIC(0), 0)
    float sw_dehaze_userTransRatio;
} Dehaze_Data_t;

typedef struct Dehaze_Setting_s {
    // M4_BOOL_DESC("en", "1")
    bool en;
    // M4_BOOL_DESC("hw_dehaze_luma_mode", "1")
    bool hw_dehaze_luma_mode;
    // M4_NUMBER_DESC("sw_dhaz_invContrastTflt_invSgm", "f32", M4_RANGE(0,256), "8.0", M4_DIGIT(4))
    float sw_dhaz_invContrastTflt_invSgm;
    // M4_NUMBER_DESC("sw_dhaz_airLightTflt_invSgm", "f32", M4_RANGE(4,1024), "120.0", M4_DIGIT(4))
    float sw_dhaz_airLightTflt_invSgm;
    // M4_NUMBER_DESC("sw_dhaz_transRatioTflt_invSgm", "f32", M4_RANGE(0.0004,1), "0.0100", M4_DIGIT(4))
    float sw_dhaz_transRatioTflt_invSgm;
} Dehaze_Setting_t;

typedef struct Enhance_Data_s {
    // M4_NUMBER_MARK_DESC("iso", "f32", M4_RANGE(50,1000000000), "0", M4_DIGIT(4), "index1")
    float iso;
    // M4_ARRAY_MARK_DESC("hw_enhance_loLumaConvert_val", "f32", M4_SIZE(1,17),  M4_RANGE(0, 1023), "[0, 64, 128, 192, 256, 320, 384, 448, 512, 576, 640, 704, 768, 832, 896, 960, 1023]", M4_DIGIT(0), M4_DYNAMIC(0), 0)
    float hw_enhance_loLumaConvert_val[DHAZ_ENHANCE_CURVE_KNOTS_NUM];
    // M4_ARRAY_MARK_DESC("sw_enhance_luma2strg_val", "f32", M4_SIZE(1,17),  M4_RANGE(0, 16), "[1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1]", M4_DIGIT(2), M4_DYNAMIC(0), 0)
    float sw_enhance_luma2strg_val[DHAZ_ENHANCE_CURVE_KNOTS_NUM];
    // M4_NUMBER_DESC("sw_enhance_contrast_strg", "f32", M4_RANGE(0,16), "1", M4_DIGIT(4))
    float sw_enhance_contrast_strg;
    // M4_NUMBER_DESC("enhance_chroma", "f32", M4_RANGE(0,16), "1", M4_DIGIT(4))
    float sw_enhance_saturate_strg;
} Enhance_Data_t;

typedef struct Enhance_Setting_s {
    // M4_BOOL_DESC("en", "1")
    bool en;
    // M4_BOOL_DESC("hw_enhance_cProtect_en", "0")
    bool hw_enhance_cProtect_en;
    // M4_BOOL_DESC("hw_enhance_luma2strg_en", "0")
    bool hw_enhance_luma2strg_en;
} Enhance_Setting_t;

typedef struct Hist_Data_s {
    // M4_NUMBER_MARK_DESC("iso", "f32", M4_RANGE(50,1000000000), "0", M4_DIGIT(4), "index1")
    float iso;
    // M4_ARRAY_DESC("sw_hist_mapUserSet", "f32", M4_SIZE(1,1), M4_RANGE(0,32), "2",M4_DIGIT(4), M4_DYNAMIC(0), 0)
    float sw_hist_mapUserSet;
    // M4_ARRAY_DESC("hw_histc_noiseCount_offset", "u8", M4_SIZE(1,1), M4_RANGE(0,255), "64",M4_DIGIT(4), M4_DYNAMIC(0), 0)
    uint8_t hw_histc_noiseCount_offset;
    // M4_ARRAY_DESC("sw_histc_noiseCount_scale", "f32", M4_SIZE(1,1), M4_RANGE(0,8), "2",M4_DIGIT(4), M4_DYNAMIC(0), 0)
    float sw_histc_noiseCount_scale;
    // M4_ARRAY_DESC("sw_histc_countWgt_minLimit", "f32", M4_SIZE(1,1), M4_RANGE(0,2), "0.015",M4_DIGIT(4), M4_DYNAMIC(0), 0)
    float sw_histc_countWgt_minLimit;
    // M4_ARRAY_DESC("sw_hist_mapCount_scale", "f32", M4_SIZE(1,1), M4_RANGE(0,32), "0.09",M4_DIGIT(4), M4_DYNAMIC(0), 0)
    float sw_hist_mapCount_scale;
    // M4_ARRAY_DESC("sw_hist_mapMerge_alpha", "f32", M4_SIZE(1,1), M4_RANGE(0,32), "2",M4_DIGIT(4), M4_DYNAMIC(0), 0)
    float sw_hist_mapMerge_alpha;
    // M4_ARRAY_MARK_DESC("sw_dehaze_outputMerge_alpha", "f32", M4_SIZE(1,17),  M4_RANGE(0, 16), "[1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1]", M4_DIGIT(2), M4_DYNAMIC(0), 0)
    float sw_dehaze_outputMerge_alpha[HIST_OUTMERGE_ALPHA_NUM];
} Hist_Data_t;

typedef struct Hist_setting_s {
    // M4_BOOL_DESC("en", "1")
    bool en;
    // M4_NUMBER_DESC("sw_hist_MapTflt_invSigma", "f32", M4_RANGE(1,255), "6.0", M4_DIGIT(0))
    float sw_hist_MapTflt_invSigma;
    // M4_NUMBER_DESC("hw_hist_imgMap_mode", "u8", M4_RANGE(0,1), "0", M4_DIGIT(0))
    uint8_t hw_hist_imgMap_mode;
    // M4_NUMBER_DESC("hw_histc_blocks_cols", "u8", M4_RANGE(4,10), "4", M4_DIGIT(0))
    uint8_t hw_histc_blocks_cols;
    // M4_NUMBER_DESC("hw_histc_blocks_rows", "u8", M4_RANGE(4,8), "5", M4_DIGIT(0))
    uint8_t hw_histc_blocks_rows;
    unsigned short blk_het;
    unsigned short blk_wid;
} Hist_setting_t;

typedef struct commom_setting_s {
    // M4_ARRAY_DESC("hw_dehaze_user_gainFuse", "f32", M4_SIZE(1,13), M4_RANGE(0,1), "[1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1]",M4_DIGIT(3), M4_DYNAMIC(0), 0)
    float hw_dehaze_user_gainFuse;
    // M4_ARRAY_DESC("hw_contrast_thumbFlt_curWgt", "f32", M4_SIZE(1,13), M4_RANGE(0,1), "[0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1]",M4_DIGIT(4), M4_DYNAMIC(0), 0)
    float hw_contrast_thumbFlt_curWgt;
    // M4_ARRAY_DESC("hw_contrast_thumbMerge_wgt", "f32", M4_SIZE(1,13), M4_RANGE(0,1), "[0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5]",M4_DIGIT(4), M4_DYNAMIC(0), 0)
    float hw_contrast_thumbMerge_wgt;
    // M4_ARRAY_DESC("hw_contrast_preThumbFlt_invRsgm", "f32", M4_SIZE(1,13), M4_RANGE(0,1), "[0.14, 0.14, 0.14, 0.14, 0.14, 0.14, 0.14, 0.14, 0.14, 0.14, 0.14, 0.14, 0.14]",M4_DIGIT(4), M4_DYNAMIC(0), 0)
    float hw_contrast_preThumbFlt_invRsgm;
    // M4_ARRAY_DESC("hw_contrast_curThumbFlt_invRsgm", "f32", M4_SIZE(1,13), M4_RANGE(0,1), "[0.14, 0.14, 0.14, 0.14, 0.14, 0.14, 0.14, 0.14, 0.14, 0.14, 0.14, 0.14, 0.14]",M4_DIGIT(4), M4_DYNAMIC(0), 0)
    float hw_contrast_curThumbFlt_invRsgm;
    // M4_ARRAY_DESC("hw_contrast_ThumbFlt_invVsigma", "f32", M4_SIZE(1,13), M4_RANGE(0,1), "[0.14, 0.14, 0.14, 0.14, 0.14, 0.14, 0.14, 0.14, 0.14, 0.14, 0.14, 0.14, 0.14]",M4_DIGIT(4), M4_DYNAMIC(0), 0)
    float hw_contrast_ThumbFlt_invVsigma;
    // M4_ARRAY_DESC("sw_contrast_paramTflt_curWgt", "u16", M4_SIZE(1,13), M4_RANGE(1,31), "[8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8]",M4_DIGIT(0), M4_DYNAMI,0)
    uint16_t sw_contrast_paramTflt_curWgt;
    // M4_ARRAY_DESC("sw_contrast_thumbTflt_curWgt", "u16", M4_SIZE(1,13), M4_RANGE(1,16), "[8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8]",M4_DIGIT(0), M4_DYNAMI,0)
    uint16_t sw_contrast_thumbTflt_curWgt;
} commom_setting_t;

typedef struct {
    // M4_STRUCT_DESC("hw_commom_setting", "normal_ui_style")
    commom_setting_t hw_commom_setting;
    // M4_STRUCT_LIST_DESC("hw_hist_params", M4_SIZE(1,13), "normal_ui_style")
    Hist_Data_t hw_hist_params;
    // M4_STRUCT_LIST_DESC("hw_enhance_params", M4_SIZE(1,13), "normal_ui_style")
    Enhance_Data_t hw_enhance_params;
    // M4_ARRAY_TABLE_DESC("hw_dehaze_params", "array_table_ui", "none")
    Dehaze_Data_t hw_dehaze_params;
} dehaze_params_dyn_t;

typedef struct {
    // M4_NUMBER_DESC("sw_dehaze_byPass_thred", "f32", M4_RANGE(0,1), "0", M4_DIGIT(4))
    float sw_dehaze_byPass_thred;
    // M4_STRUCT_DESC("hw_dehaze_setting", "normal_ui_style")
    Dehaze_Setting_t hw_dehaze_setting;
    // M4_STRUCT_DESC("hw_enhance_setting", "normal_ui_style")
    Enhance_Setting_t hw_enhance_setting;
    // M4_STRUCT_DESC("hw_hist_setting", "normal_ui_style")
    Hist_setting_t hw_hist_setting;
    unsigned short sigma_lut[DHAZ_SIGMA_LUT_NUM];
    unsigned char sigma_idx[DHAZ_SIGMA_IDX_NUM];
    unsigned char dc_en;
    unsigned char enhance_en;
    unsigned char hist_en;
    unsigned char round_en;
    unsigned char mem_mode;
    unsigned char mem_force;
    unsigned char gaus_h0;
    unsigned char gaus_h1;
    unsigned char gaus_h2;
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
