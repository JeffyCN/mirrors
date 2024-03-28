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

#ifndef _RK_AIQ_PARAM_DRC32_H_
#define _RK_AIQ_PARAM_DRC32_H_
#define DRC_Y_NUM 17

typedef enum drc_CompressMode_e {
    drc_COMPRESS_AUTO     = 0,
    drc_COMPRESS_MANUAL   = 1,
} drc_CompressMode_t;

typedef struct drc_Compress_s {
    // M4_ENUM_DESC("Mode", "drc_CompressMode_t", "drc_COMPRESS_AUTO")
    drc_CompressMode_t Mode;
    // M4_ARRAY_MARK_DESC("Manual_curve", "u32", M4_SIZE(1,17),  M4_RANGE(0, 8192), "[0, 558, 1087, 1588, 2063, 2515, 2944, 3353, 3744, 4473, 5139, 5751, 6316, 6838, 7322, 7772, 8192]", M4_DIGIT(0), M4_DYNAMIC(0), "drc_compress_curve")
    uint16_t       Manual_curve[DRC_Y_NUM];
} drc_Compress_t;

typedef struct drc_local_s {
    // M4_NUMBER_DESC("curPixWeit", "f32", M4_RANGE(0,1), "0.37", M4_DIGIT(3))
    float curPixWeit;  
    // M4_NUMBER_DESC("preFrameWeit", "f32", M4_RANGE(0,1), "1.0", M4_DIGIT(3))
    float preFrameWeit;  
    // M4_NUMBER_DESC("Range_force_sgm", "f32", M4_RANGE(0,1), "0.0", M4_DIGIT(4))
    float Range_force_sgm;  
    // M4_NUMBER_DESC("Range_sgm_cur", "f32", M4_RANGE(0,1), "0.125", M4_DIGIT(4))
    float Range_sgm_cur; 
    // M4_NUMBER_DESC("Range_sgm_pre", "f32", M4_RANGE(0,1), "0.125", M4_DIGIT(4))
    float Range_sgm_pre;  
    // M4_NUMBER_DESC("Space_sgm_cur", "u16", M4_RANGE(0,4095), "4068", M4_DIGIT(0))
    int Space_sgm_cur;  
    // M4_NUMBER_DESC("Space_sgm_pre", "u16", M4_RANGE(0,4095), "3968", M4_DIGIT(0))
    int Space_sgm_pre;  
} drc_local_t;

typedef struct drc_HighLight_s {
    // M4_NUMBER_DESC("gas_l0", "u8", M4_RANGE(0,64), "24", M4_DIGIT(0))
    int gas_l0;
    // M4_NUMBER_DESC("gas_l1", "u8", M4_RANGE(0,64), "10", M4_DIGIT(0))
    int gas_l1;
    // M4_NUMBER_DESC("gas_l2", "u8", M4_RANGE(0,64), "10", M4_DIGIT(0))
    int gas_l2;
    // M4_NUMBER_DESC("gas_l3", "u8", M4_RANGE(0,64), "5", M4_DIGIT(0))
    int gas_l3;
} drc_HighLight_t;

typedef struct {
    // M4_ARRAY_TABLE_DESC("HiLight", "normal_ui_style")
    drc_HighLight_t HiLight;
    // M4_STRUCT_DESC("LocalSetting", "normal_ui_style")
    drc_local_t LocalSetting;
    // M4_STRUCT_DESC("CompressSetting", "normal_ui_style")
    drc_Compress_t CompressSetting;
    // M4_NUMBER_DESC("sw_drc_byPass_thred", "f32", M4_RANGE(0,1), "0", M4_DIGIT(4))
    float sw_drc_byPass_thred;
    // M4_NUMBER_DESC("Edge_Weit", "f32",  M4_RANGE(0,1), "1",M4_DIGIT(3))
    float Edge_Weit;  
    // M4_BOOL_DESC("OutPutLongFrame", "0")
    bool OutPutLongFrame;  
    // M4_NUMBER_DESC("IIR_frame", "f32", M4_RANGE(0,1), "0.1", M4_DIGIT(0))
    float IIR_frame;  
    // M4_NUMBER_DESC("damp", "f32", M4_RANGE(0,1), "0.9", M4_DIGIT(3))
    float damp;
    // M4_ARRAY_DESC("Scale_y", "u16", M4_SIZE(1,17),  M4_RANGE(0, 2048), "[0,2,20,76,193,381,631,772,919,1066,1211,1479,1700,1863,1968,2024,2048]", M4_DIGIT(0), M4_DYNAMIC(0))
    int Scale_y[DRC_Y_NUM];
    unsigned char offset_pow2;
    unsigned short delta_scalein;
    float compres_scl;
    float sw_drc_min_ogain;
    unsigned short compres_y[DRC_Y_NUM];
    unsigned short gain_y[DRC_Y_NUM];
} drc_params_static_t;

typedef struct drc_LocalData_s{
    // M4_ARRAY_DESC("LocalWeit", "f32", M4_SIZE(1,13), M4_RANGE(0,1), "[1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1]",M4_DIGIT(2), M4_DYNAMIC(0), 0)
    float LocalWeit;  
    // M4_ARRAY_DESC("LocalAutoEnable", "u8", M4_SIZE(1,13), M4_RANGE(0,1), "[1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1]",M4_DIGIT(0), M4_DYNAMIC(0), 0)
    int LocalAutoEnable;
    // M4_ARRAY_DESC("LocalAutoWeit", "f32", M4_SIZE(1,13), M4_RANGE(0,1), "[0.037477, 0.037477, 0.037477, 0.037477, 0.037477, 0.037477, 0.037477, 0.037477, 0.037477, 0.037477, 0.037477, 0.037477, 0.037477]",M4_DIGIT(5), M4_DYNAMIC(0), 0)
    float LocalAutoWeit;  
    // M4_ARRAY_DESC("GlobalContrast", "f32", M4_SIZE(1,13), M4_RANGE(0,1), "[0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]",M4_DIGIT(3), M4_DYNAMIC(0), 0)
    float GlobalContrast; 
    // M4_ARRAY_DESC("LoLitContrast", "f32", M4_SIZE(1,13), M4_RANGE(0,1), "[0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]",M4_DIGIT(3), M4_DYNAMIC(0), 0)
    float LoLitContrast; 
} drc_LocalData_t;

typedef struct drc_MotionData_s {
    // M4_ARRAY_DESC("MotionCoef", "f32", M4_SIZE(1,13), M4_RANGE(0,1), "[0, 0.005, 0.01, 0.05, 0.1, 0.15, 0.2, 0.3, 0.4, 0.5, 0.6, 0.8, 1]",M4_DIGIT(2), M4_DYNAMIC(0), 0)
    float MotionCoef;
    // M4_ARRAY_DESC("MotionStr", "f32", M4_SIZE(1,13), M4_RANGE(0,1), "[0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]",M4_DIGIT(3), M4_DYNAMIC(0), 0)
    float MotionStr;  
} drc_MotionData_t;

typedef struct drc_HighLightData_s {
    // M4_ARRAY_DESC("Strength", "f32", M4_SIZE(1,13), M4_RANGE(0,1), "[1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1]",M4_DIGIT(2), M4_DYNAMIC(0), 0)
    float Strength;  
    // M4_ARRAY_DESC("gas_t", "f32", M4_SIZE(1,13), M4_RANGE(0,2), "[0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]",M4_DIGIT(3), M4_DYNAMIC(0), 0)
    float gas_t;  
} drc_HighLightData_t;

typedef struct DrcGain_s {
    // M4_ARRAY_DESC("DrcGain", "f32",  M4_RANGE(1,8), "1",M4_DIGIT(2))
    float DrcGain; 
    // M4_ARRAY_DESC("Alpha", "f32", M4_RANGE(0,1), "0.2",M4_DIGIT(2))
    float Alpha;
    // M4_ARRAY_DESC("Clip", "f32", M4_RANGE(0,64), "16",M4_DIGIT(2))
    float Clip;  
} DrcGain_t;

typedef struct {
    // M4_ARRAY_TABLE_DESC("DrcGain", "array_table_ui", "none")
    DrcGain_t DrcGain;
    // M4_ARRAY_TABLE_DESC("HiLightData", "array_table_ui", "none")
    drc_HighLightData_t HiLightData;
    // M4_ARRAY_TABLE_DESC("LocalData", "array_table_ui", "none")
    drc_LocalData_t LocalData;
    // M4_ARRAY_TABLE_DESC("MotionData", "array_table_ui", "none")
    drc_MotionData_t MotionData;
} drc_params_dyn_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(static_param),
        M4_TYPE(struct),
        M4_UI_MODULE(static_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The static params of drc module))  */
    drc_params_static_t sta;
    /* M4_GENERIC_DESC(
        M4_ALIAS(dynamic_param),
        M4_TYPE(struct),
        M4_UI_MODULE(dynamic_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(All dynamic params array corresponded with iso array))  */
    drc_params_dyn_t dyn;
} drc_param_t;

#endif
