/*
 * adrc_head.h
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

#ifndef __CALIBDBV2_ADRC_HEADER_H__
#define __CALIBDBV2_ADRC_HEADER_H__

#include "adehaze_head.h"

#define ADRC_Y_NUM             17
#define ADRC_ENVLV_STEP_MAX    13

// drc v10
typedef struct AdrcGain_s {
    // M4_ARRAY_DESC("CtrlData", "f32", M4_SIZE(1,13), M4_RANGE(0,10000000), "[0, 0.005, 0.01, 0.05, 0.1, 0.15, 0.2, 0.3, 0.4, 0.5, 0.6, 0.8, 1]",M4_DIGIT(3), M4_DYNAMIC(0), 0)
    float CtrlData[ADRC_ENVLV_STEP_MAX];
    // M4_ARRAY_DESC("DrcGain", "f32", M4_SIZE(1,13), M4_RANGE(1,8), "[1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1]",M4_DIGIT(2), M4_DYNAMIC(0), 0)
    float DrcGain[ADRC_ENVLV_STEP_MAX]; 
    // M4_ARRAY_DESC("Alpha", "f32", M4_SIZE(1,13), M4_RANGE(0,1), "[0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2]",M4_DIGIT(2), M4_DYNAMIC(0), 0)
    float Alpha[ADRC_ENVLV_STEP_MAX];
    // M4_ARRAY_DESC("Clip", "f32", M4_SIZE(1,13), M4_RANGE(0,64), "[16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16]",M4_DIGIT(2), M4_DYNAMIC(0), 0)
    float Clip[ADRC_ENVLV_STEP_MAX];  
} AdrcGain_t;

typedef struct HighLight_s {
    // M4_ARRAY_DESC("CtrlData", "f32", M4_SIZE(1,13), M4_RANGE(0,10000000), "[0, 0.005, 0.01, 0.05, 0.1, 0.15, 0.2, 0.3, 0.4, 0.5, 0.6, 0.8, 1]",M4_DIGIT(3), M4_DYNAMIC(0), 0)
    float CtrlData[ADRC_ENVLV_STEP_MAX];
    // M4_ARRAY_DESC("Strength", "f32", M4_SIZE(1,13), M4_RANGE(0,1), "[0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]",M4_DIGIT(2), M4_DYNAMIC(0), 0)
    float Strength[ADRC_ENVLV_STEP_MAX]; 
} HighLight_t;

typedef struct LocalData_s{
    // M4_ARRAY_DESC("CtrlData", "f32", M4_SIZE(1,13), M4_RANGE(0,10000000), "[0, 0.005, 0.01, 0.05, 0.1, 0.15, 0.2, 0.3, 0.4, 0.5, 0.6, 0.8, 1]",M4_DIGIT(2), M4_DYNAMIC(0), 0)
    float CtrlData[ADRC_ENVLV_STEP_MAX];
    // M4_ARRAY_DESC("LocalWeit", "f32", M4_SIZE(1,13), M4_RANGE(0,1), "[1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1]",M4_DIGIT(2), M4_DYNAMIC(0), 0)
    float LocalWeit[ADRC_ENVLV_STEP_MAX];  
    // M4_ARRAY_DESC("GlobalContrast", "f32", M4_SIZE(1,13), M4_RANGE(0,1), "[0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]",M4_DIGIT(3), M4_DYNAMIC(0), 0)
    float GlobalContrast[ADRC_ENVLV_STEP_MAX]; 
    // M4_ARRAY_DESC("LoLitContrast", "f32", M4_SIZE(1,13), M4_RANGE(0,1), "[0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]",M4_DIGIT(3), M4_DYNAMIC(0), 0)
    float LoLitContrast[ADRC_ENVLV_STEP_MAX];
} LocalData_t;

typedef struct local_s {
    // M4_ARRAY_TABLE_DESC("LocalTMOData", "array_table_ui", "none")
    LocalData_t LocalTMOData;
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
} local_t;

typedef enum CompressMode_e {
    COMPRESS_AUTO     = 0,
    COMPRESS_MANUAL   = 1,
} CompressMode_t;

typedef struct Compress_s {
    // M4_ENUM_DESC("Mode", "CompressMode_t", "COMPRESS_AUTO")
    CompressMode_t Mode;
    // M4_ARRAY_MARK_DESC("Manual_curve", "u32", M4_SIZE(1,17),  M4_RANGE(0, 8192), "[0, 558, 1087, 1588, 2063, 2515, 2944, 3353, 3744, 4473, 5139, 5751, 6316, 6838, 7322, 7772, 8192]", M4_DIGIT(0), M4_DYNAMIC(0), "drc_compress_curve")
    uint16_t       Manual_curve[ADRC_Y_NUM];
} Compress_t;

typedef struct CalibDbV2_Adrc_V10_s {
    // M4_BOOL_DESC("Enable", "1")
    bool Enable;
    // M4_ENUM_DESC("CtrlDataType", "CtrlDataType_t", "CTRLDATATYPE_ENVLV")
    CtrlDataType_t CtrlDataType;
    // M4_ARRAY_TABLE_DESC("DrcGain", "array_table_ui", "none")
    AdrcGain_t DrcGain;
    // M4_ARRAY_TABLE_DESC("HiLight", "array_table_ui", "none")
    HighLight_t HiLight;
    // M4_STRUCT_DESC("LocalTMOSetting", "normal_ui_style")
    local_t LocalTMOSetting;
    // M4_STRUCT_DESC("CompressSetting", "normal_ui_style")
    Compress_t CompressSetting;
    // M4_ARRAY_DESC("Scale_y", "u16", M4_SIZE(1,17),  M4_RANGE(0, 2048), "[0,2,20,76,193,381,631,772,919,1066,1211,1479,1700,1863,1968,2024,2048]", M4_DIGIT(0), M4_DYNAMIC(0))
    int Scale_y[ADRC_Y_NUM];
    // M4_NUMBER_DESC("ByPassThr", "f32", M4_RANGE(0,1), "0", M4_DIGIT(4))
    float ByPassThr;
    // M4_NUMBER_DESC("Edge_Weit", "f32",  M4_RANGE(0,1), "1",M4_DIGIT(3))
    float Edge_Weit; 
    // M4_BOOL_DESC("OutPutLongFrame", "0")
    bool  OutPutLongFrame;  
    // M4_NUMBER_DESC("IIR_frame", "u8", M4_RANGE(1,1000), "2", M4_DIGIT(0))
    int IIR_frame; 
    // M4_NUMBER_DESC("damp", "f32", M4_RANGE(0,1), "0.9", M4_DIGIT(3))
    float damp;
} CalibDbV2_Adrc_V10_t;

typedef struct CalibDbV2_drc_V10_s {
    // M4_STRUCT_DESC("DrcTuningPara", "normal_ui_style")
    CalibDbV2_Adrc_V10_t DrcTuningPara;
} CalibDbV2_drc_V10_t;

// drc v11
typedef struct LocalDataV2_s{
    // M4_ARRAY_DESC("CtrlData", "f32", M4_SIZE(1,13), M4_RANGE(0,10000000), "[0, 0.005, 0.01, 0.05, 0.1, 0.15, 0.2, 0.3, 0.4, 0.5, 0.6, 0.8, 1]",M4_DIGIT(2), M4_DYNAMIC(0), 0)
    float CtrlData[ADRC_ENVLV_STEP_MAX];
    // M4_ARRAY_DESC("LocalWeit", "f32", M4_SIZE(1,13), M4_RANGE(0,1), "[1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1]",M4_DIGIT(2), M4_DYNAMIC(0), 0)
    float LocalWeit[ADRC_ENVLV_STEP_MAX];  
    // M4_ARRAY_DESC("LocalAutoEnable", "u8", M4_SIZE(1,13), M4_RANGE(0,1), "[1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1]",M4_DIGIT(0), M4_DYNAMIC(0), 0)
    int LocalAutoEnable[ADRC_ENVLV_STEP_MAX];  
    // M4_ARRAY_DESC("LocalAutoWeit", "f32", M4_SIZE(1,13), M4_RANGE(0,1), "[0.037477, 0.037477, 0.037477, 0.037477, 0.037477, 0.037477, 0.037477, 0.037477, 0.037477, 0.037477, 0.037477, 0.037477, 0.037477]",M4_DIGIT(5), M4_DYNAMIC(0), 0)
    float LocalAutoWeit[ADRC_ENVLV_STEP_MAX];  
    // M4_ARRAY_DESC("GlobalContrast", "f32", M4_SIZE(1,13), M4_RANGE(0,1), "[0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]",M4_DIGIT(3), M4_DYNAMIC(0), 0)
    float GlobalContrast[ADRC_ENVLV_STEP_MAX]; 
    // M4_ARRAY_DESC("LoLitContrast", "f32", M4_SIZE(1,13), M4_RANGE(0,1), "[0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]",M4_DIGIT(3), M4_DYNAMIC(0), 0)
    float LoLitContrast[ADRC_ENVLV_STEP_MAX]; 
} LocalDataV2_t;

typedef struct localV11_s {
    // M4_ARRAY_TABLE_DESC("LocalData", "array_table_ui", "none")
    LocalDataV2_t LocalData;
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
} localV11_t;

typedef struct CalibDbV2_Adrc_V11_s {
    // M4_BOOL_DESC("Enable", "1")
    bool Enable;
    // M4_ENUM_DESC("CtrlDataType", "CtrlDataType_t", "CTRLDATATYPE_ENVLV")
    CtrlDataType_t CtrlDataType;
    // M4_ARRAY_TABLE_DESC("DrcGain", "array_table_ui", "none")
    AdrcGain_t DrcGain;
    // M4_ARRAY_TABLE_DESC("HiLight", "array_table_ui", "none")
    HighLight_t HiLight;
    // M4_STRUCT_DESC("LocalSetting", "normal_ui_style")
    localV11_t LocalSetting;
    // M4_STRUCT_DESC("CompressSetting", "normal_ui_style")
    Compress_t CompressSetting;
    // M4_ARRAY_DESC("Scale_y", "u16", M4_SIZE(1,17),  M4_RANGE(0, 2048), "[0,2,20,76,193,381,631,772,919,1066,1211,1479,1700,1863,1968,2024,2048]", M4_DIGIT(0), M4_DYNAMIC(0))
    int Scale_y[ADRC_Y_NUM];
    // M4_NUMBER_DESC("ByPassThr", "f32", M4_RANGE(0,1), "0", M4_DIGIT(4))
    float ByPassThr;
    // M4_NUMBER_DESC("Edge_Weit", "f32",  M4_RANGE(0,1), "1",M4_DIGIT(3))
    float Edge_Weit; 
    // M4_BOOL_DESC("OutPutLongFrame", "0")
    bool  OutPutLongFrame;  
    // M4_NUMBER_DESC("IIR_frame", "u8", M4_RANGE(1,1000), "2", M4_DIGIT(0))
    int IIR_frame; 
    // M4_NUMBER_DESC("damp", "f32", M4_RANGE(0,1), "0.9", M4_DIGIT(3))
    float damp;
} CalibDbV2_Adrc_V11_t;

typedef struct CalibDbV2_drc_V11_s {
    // M4_STRUCT_DESC("DrcTuningPara", "normal_ui_style")
    CalibDbV2_Adrc_V11_t DrcTuningPara;
} CalibDbV2_drc_V11_t;

// drc v12
typedef struct MotionData_s {
    // M4_ARRAY_DESC("MotionCoef", "f32", M4_SIZE(1,13), M4_RANGE(0,1), "[0, 0.005, 0.01, 0.05, 0.1, 0.15, 0.2, 0.3, 0.4, 0.5, 0.6, 0.8, 1]",M4_DIGIT(2), M4_DYNAMIC(0), 0)
    float MotionCoef[ADRC_ENVLV_STEP_MAX];
    // M4_ARRAY_DESC("MotionStr", "f32", M4_SIZE(1,13), M4_RANGE(0,1), "[0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]",M4_DIGIT(3), M4_DYNAMIC(0), 0)
    float MotionStr[ADRC_ENVLV_STEP_MAX];  
} MotionData_t;

typedef struct localV12_s {
    // M4_ARRAY_TABLE_DESC("LocalData", "array_table_ui", "none")
    LocalDataV2_t LocalData;
    // M4_ARRAY_TABLE_DESC("MotionData", "array_table_ui", "none")
    MotionData_t MotionData;
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
} localV12_t;

typedef struct HighLightDataV12_s {
    // M4_ARRAY_DESC("CtrlData", "f32", M4_SIZE(1,13), M4_RANGE(0,10000000), "[0, 0.005, 0.01, 0.05, 0.1, 0.15, 0.2, 0.3, 0.4, 0.5, 0.6, 0.8, 1]",M4_DIGIT(3), M4_DYNAMIC(0), 0)
    float CtrlData[ADRC_ENVLV_STEP_MAX];
    // M4_ARRAY_DESC("Strength", "f32", M4_SIZE(1,13), M4_RANGE(0,1), "[1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1]",M4_DIGIT(2), M4_DYNAMIC(0), 0)
    float Strength[ADRC_ENVLV_STEP_MAX];  
    // M4_ARRAY_DESC("gas_t", "f32", M4_SIZE(1,13), M4_RANGE(0,2), "[0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]",M4_DIGIT(3), M4_DYNAMIC(0), 0)
    float gas_t[ADRC_ENVLV_STEP_MAX];  
} HighLightDataV12_t;

typedef struct HighLightV12_s {
    // M4_ARRAY_TABLE_DESC("HiLightData", "array_table_ui", "none")
    HighLightDataV12_t HiLightData;
    // M4_NUMBER_DESC("gas_l0", "u8", M4_RANGE(0,64), "24", M4_DIGIT(0))
    int gas_l0;
    // M4_NUMBER_DESC("gas_l1", "u8", M4_RANGE(0,64), "10", M4_DIGIT(0))
    int gas_l1;
    // M4_NUMBER_DESC("gas_l2", "u8", M4_RANGE(0,64), "10", M4_DIGIT(0))
    int gas_l2;
    // M4_NUMBER_DESC("gas_l3", "u8", M4_RANGE(0,64), "5", M4_DIGIT(0))
    int gas_l3;
} HighLightV12_t;

typedef struct CalibDbV2_Adrc_V12_s {
    // M4_BOOL_DESC("Enable", "1")
    bool Enable;
    // M4_ENUM_DESC("CtrlDataType", "CtrlDataType_t", "CTRLDATATYPE_ENVLV")
    CtrlDataType_t CtrlDataType;
    // M4_ARRAY_TABLE_DESC("DrcGain", "array_table_ui", "none")
    AdrcGain_t DrcGain;
    // M4_ARRAY_TABLE_DESC("HiLight", "normal_ui_style")
    HighLightV12_t HiLight;
    // M4_STRUCT_DESC("LocalSetting", "normal_ui_style")
    localV12_t LocalSetting;
    // M4_STRUCT_DESC("CompressSetting", "normal_ui_style")
    Compress_t CompressSetting;
    // M4_ARRAY_DESC("Scale_y", "u16", M4_SIZE(1,17),  M4_RANGE(0, 2048), "[0,2,20,76,193,381,631,772,919,1066,1211,1479,1700,1863,1968,2024,2048]", M4_DIGIT(0), M4_DYNAMIC(0))
    int Scale_y[ADRC_Y_NUM];
    // M4_NUMBER_DESC("ByPassThr", "f32", M4_RANGE(0,1), "0", M4_DIGIT(4))
    float ByPassThr;
    // M4_NUMBER_DESC("Edge_Weit", "f32",  M4_RANGE(0,1), "1",M4_DIGIT(3))
    float Edge_Weit;  
    // M4_BOOL_DESC("OutPutLongFrame", "0")
    bool OutPutLongFrame;  
    // M4_NUMBER_DESC("IIR_frame", "u8", M4_RANGE(1,1000), "2", M4_DIGIT(0))
    int IIR_frame;  
    // M4_NUMBER_DESC("damp", "f32", M4_RANGE(0,1), "0.9", M4_DIGIT(3))
    float damp;
} CalibDbV2_Adrc_V12_t;

typedef struct CalibDbV2_drc_V12_s {
    // M4_STRUCT_DESC("DrcTuningPara", "normal_ui_style")
    CalibDbV2_Adrc_V12_t DrcTuningPara;
} CalibDbV2_drc_V12_t;

typedef struct localV12Lite_s {
    // M4_ARRAY_TABLE_DESC("LocalData", "array_table_ui", "none")
    LocalDataV2_t LocalData;
    // M4_ARRAY_TABLE_DESC("MotionData", "array_table_ui", "none")
    MotionData_t MotionData;
    // M4_NUMBER_DESC("curPixWeit", "f32", M4_RANGE(0,1), "0.37", M4_DIGIT(3))
    float curPixWeit;
    // M4_NUMBER_DESC("Range_force_sgm", "f32", M4_RANGE(0,1), "0.0", M4_DIGIT(4))
    float Range_force_sgm;
    // M4_NUMBER_DESC("Range_sgm_cur", "f32", M4_RANGE(0,1), "0.125", M4_DIGIT(4))
    float Range_sgm_cur;
    // M4_NUMBER_DESC("Space_sgm_cur", "u16", M4_RANGE(0,4095), "4068", M4_DIGIT(0))
    int Space_sgm_cur;
} localV12Lite_t;

typedef struct CalibDbV2_Adrc_v12_lite_s {
    // M4_BOOL_DESC("Enable", "1")
    bool Enable;
    // M4_ENUM_DESC("CtrlDataType", "CtrlDataType_t", "CTRLDATATYPE_ENVLV")
    CtrlDataType_t CtrlDataType;
    // M4_ARRAY_TABLE_DESC("DrcGain", "array_table_ui", "none")
    AdrcGain_t DrcGain;
    // M4_ARRAY_TABLE_DESC("HiLight", "normal_ui_style")
    HighLightV12_t HiLight;
    // M4_STRUCT_DESC("LocalSetting", "normal_ui_style")
    localV12Lite_t LocalSetting;
    // M4_STRUCT_DESC("CompressSetting", "normal_ui_style")
    Compress_t CompressSetting;
    // M4_ARRAY_DESC("Scale_y", "u16", M4_SIZE(1,17),  M4_RANGE(0, 2048), "[0,2,20,76,193,381,631,772,919,1066,1211,1479,1700,1863,1968,2024,2048]", M4_DIGIT(0), M4_DYNAMIC(0))
    int Scale_y[ADRC_Y_NUM];
    // M4_NUMBER_DESC("ByPassThr", "f32", M4_RANGE(0,1), "0", M4_DIGIT(4))
    float ByPassThr;
    // M4_NUMBER_DESC("Edge_Weit", "f32",  M4_RANGE(0,1), "1",M4_DIGIT(3))
    float Edge_Weit;
    // M4_BOOL_DESC("OutPutLongFrame", "0")
    bool OutPutLongFrame;
    // M4_NUMBER_DESC("IIR_frame", "u8", M4_RANGE(1,1000), "2", M4_DIGIT(0))
    int IIR_frame;
    // M4_NUMBER_DESC("damp", "f32", M4_RANGE(0,1), "0.9", M4_DIGIT(3))
    float damp;
} CalibDbV2_Adrc_v12_lite_t;

typedef struct CalibDbV2_drc_v12_lite_s {
    // M4_STRUCT_DESC("DrcTuningPara", "normal_ui_style")
    CalibDbV2_Adrc_v12_lite_t DrcTuningPara;
} CalibDbV2_drc_v12_lite_t;

typedef struct LocalDataV20_s {
    // M4_ARRAY_DESC("iso", "f32", M4_SIZE(1,13), M4_RANGE(50,10000000), "[50, 100, 200, 400, 800, 1600, 3200, 6400, 12800, 25600, 51200, 102400, 204800]",M4_DIGIT(2), M4_DYNAMIC(0), 0)
    float iso[ADRC_ENVLV_STEP_MAX];
    // M4_ARRAY_DESC("LocalWeit", "f32", M4_SIZE(1,13), M4_RANGE(0,16), "[16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16]",M4_DIGIT(0), M4_DYNAMIC(0), 0)
    uint8_t hw_adrc_bifilt_wgt[ADRC_ENVLV_STEP_MAX];
    // M4_ARRAY_DESC("hw_adrc_bifiltSoftThred_en", "u8", M4_SIZE(1,13), M4_RANGE(0,1), "[1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1]",M4_DIGIT(0), M4_DYNAMIC(0), 0)
    uint8_t hw_adrc_bifiltSoftThred_en[ADRC_ENVLV_STEP_MAX];
    // M4_ARRAY_DESC("hw_adrc_bifilt_softThred", "u16", M4_SIZE(1,13), M4_RANGE(0,2047), "[119, 119, 119, 119, 119, 119, 119, 119, 119, 119, 119, 119, 119]",M4_DIGIT(0), M4_DYNAMIC(0), 0)
    uint16_t hw_adrc_bifilt_softThred[ADRC_ENVLV_STEP_MAX];
} LocalDataV20_t;

typedef enum thumb_thd_table_mode_e {
    THUMB_THD_TABLE_AUTO   = 0,
    THUMB_THD_TABLE_MANUAL = 1,
} thumb_thd_table_mode_t;

typedef struct thumb_thd_auto_table_setting_s {
    // M4_NUMBER_DESC("sw_adrc_thumbThred_minLimit", "f32",  M4_RANGE(0,2), "1.0",M4_DIGIT(3))
    float sw_adrc_thumbThred_minLimit;
    // M4_NUMBER_DESC("sw_adrc_thumbThred_maxLimit", "f32",  M4_RANGE(0,2), "1.0",M4_DIGIT(3))
    float sw_adrc_thumbThred_maxLimit;
    // M4_NUMBER_DESC("sw_adrc_thumbThredCurve_slope", "f32",  M4_RANGE(0,50), "10",M4_DIGIT(3))
    float sw_adrc_thumbThredCurve_slope;
    // M4_NUMBER_DESC("sw_adrc_thumbThredCurve_offset", "f32",  M4_RANGE(0,1), "0.35",M4_DIGIT(3))
    float sw_adrc_thumbThredCurve_offset;
} thumb_thd_auto_table_setting_t;

typedef struct thumb_thd_table_setting_s {
    // M4_ENUM_DESC("sw_adrc_autoCurve_mode", "thumb_thd_table_mode_t", "THUMB_THD_TABLE_AUTO")
    thumb_thd_table_mode_t sw_adrc_autoCurve_mode;
    // M4_ARRAY_MARK_DESC("hw_adrc_thumbLumaDiff2thrd_mVal", "u32", M4_SIZE(1,17),  M4_RANGE(0, 511), "[256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256]", M4_DIGIT(0), M4_DYNAMIC(0), "drc_compress_curve")
    uint16_t hw_adrc_thumbLumaDiff2thrd_mVal[ADRC_Y_NUM];
    // M4_STRUCT_DESC("sw_adrc_thumbLumaDiff2thrd_aVal", "normal_ui_style")
    thumb_thd_auto_table_setting_t sw_adrc_thumbLumaDiff2thrd_aVal;
} thumb_thd_table_setting_t;

typedef struct thumbSetting_s {
    // M4_BOOL_DESC("hw_adrc_thumbThred_en", "0")
    bool hw_adrc_thumbThred_en;
    // M4_NUMBER_DESC("hw_adrc_thumb_maxLimit", "u16",  M4_RANGE(0,4095), "1024",M4_DIGIT(0))
    uint16_t hw_adrc_thumb_maxLimit;
    // M4_NUMBER_DESC("hw_adrc_thumb_scale", "u8",  M4_RANGE(0,255), "16",M4_DIGIT(0))
    uint8_t hw_adrc_thumb_scale;
    // M4_NUMBER_DESC("hw_adrc_thumbThred_neg", "u16",  M4_RANGE(0,511), "119",M4_DIGIT(0))
    uint16_t hw_adrc_thumbThred_neg;
    // M4_STRUCT_DESC("thumb_thd_table_setting", "normal_ui_style")
    thumb_thd_table_setting_t thumb_thd_table_setting;
} thumbSetting_t;

typedef struct bifltSettingV20_s {
    // M4_ARRAY_TABLE_DESC("hw_adrc_biflt_params", "array_table_ui", "none")
    LocalDataV20_t hw_adrc_biflt_params;
    // M4_STRUCT_DESC("hw_adrc_thumb_setting", "normal_ui_style")
    thumbSetting_t hw_adrc_thumb_setting;
    // M4_NUMBER_DESC("hw_adrc_gas_t", "u16", M4_RANGE(0,1023), "0", M4_DIGIT(0))
    uint16_t hw_adrc_gas_t;
    // M4_NUMBER_DESC("hw_adrc_bifilt_curPixelWgt", "u8", M4_RANGE(0,255), "16", M4_DIGIT(0))
    uint8_t hw_adrc_bifilt_curPixelWgt;
    // M4_NUMBER_DESC("hw_adrc_bifilt_hiWgt", "u8", M4_RANGE(0,255), "255", M4_DIGIT(0))
    uint8_t hw_adrc_bifilt_hiWgt;
    // M4_NUMBER_DESC("hw_adrc_bifilt_wgtOffset", "u8", M4_RANGE(0,255), "0", M4_DIGIT(0))
    uint8_t hw_adrc_bifilt_wgtOffset;
    // M4_NUMBER_DESC("hw_adrc_hiRange_invSigma", "u16", M4_RANGE(0,1023), "256", M4_DIGIT(0))
    uint16_t hw_adrc_hiRange_invSigma;
    // M4_NUMBER_DESC("hw_adrc_loRange_invSigma", "u16", M4_RANGE(0,1023), "256", M4_DIGIT(0))
    uint16_t hw_adrc_loRange_invSigma;
} bifltSettingV20_t;

typedef enum drc_gain_mode_e {
    DRC_GAIN_TABLE_AUTO   = 0,
    DRC_GAIN_TABLE_MANUAL = 1,
} drc_gain_mode_t;

typedef struct drc_gain_drcGain_auto_s {
    // M4_ARRAY_DESC("iso", "f32", M4_SIZE(1,13), M4_RANGE(50,10000000), "[50, 100, 200, 400, 800, 1600, 3200, 6400, 12800, 25600, 51200, 102400, 204800]",M4_DIGIT(2), M4_DYNAMIC(0), 0)
    float iso[ADRC_ENVLV_STEP_MAX];
    // M4_ARRAY_DESC("sw_adrc_drcGain_maxLimit", "f32", M4_SIZE(1,13), M4_RANGE(1,8), "[1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1]",M4_DIGIT(2), M4_DYNAMIC(0), 0)
    float sw_adrc_drcGain_maxLimit[ADRC_ENVLV_STEP_MAX];
    // M4_ARRAY_DESC("sw_adrc_drcGainLumaAdj_scale", "f32", M4_SIZE(1,13), M4_RANGE(0,1), "[0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2]",M4_DIGIT(2), M4_DYNAMIC(0), 0)
    float sw_adrc_drcGainLumaAdj_scale[ADRC_ENVLV_STEP_MAX];
} drc_gain_drcGain_auto_t;

typedef struct AdrcGainV20_s {
    // M4_ENUM_DESC("sw_adrc_drcGain_mode", "drc_gain_mode_t", "DRC_GAIN_TABLE_AUTO")
    drc_gain_mode_t sw_adrc_drcGain_mode;
    // M4_NUMBER_DESC("hw_adrc_drcGainIdxLuma_scale", "f32", M4_RANGE(0,64), "1.00", M4_DIGIT(2))
    float hw_adrc_drcGainIdxLuma_scale;
    // M4_ARRAY_MARK_DESC("hw_adrc_luma2drcGain_manualVal", "u32", M4_SIZE(1,17),  M4_RANGE(0, 8191), "[1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024]", M4_DIGIT(0), M4_DYNAMIC(0))
    uint16_t hw_adrc_luma2drcGain_manualVal[ADRC_Y_NUM];
    // M4_ARRAY_TABLE_DESC("sw_adrc_luma2drcGain_autoVal", "array_table_ui", "none")
    drc_gain_drcGain_auto_t sw_adrc_luma2drcGain_autoVal;
} AdrcGainV20_t;

typedef struct CompressV20_s {
    // M4_ENUM_DESC("sw_adrc_luma2compsLuma_mode", "CompressMode_t", "COMPRESS_AUTO")
    CompressMode_t sw_adrc_luma2compsLuma_mode;
    // M4_ARRAY_MARK_DESC("hw_adrc_luma2compsLuma_mVal", "u32", M4_SIZE(1,17),  M4_RANGE(0, 2047), "[0, 101, 183, 250, 306, 353, 395, 432, 466, 528, 587, 647, 710, 778, 852, 934, 1024]", M4_DIGIT(0), M4_DYNAMIC(0), "drc_compress_curve")
    uint16_t hw_adrc_luma2compsLuma_mVal[ADRC_Y_NUM];
    // M4_NUMBER_DESC("hw_adrc_logTransform_offset", "u8", M4_RANGE(0,15), "8", M4_DIGIT(0))
    uint8_t hw_adrc_logTransform_offset;
} CompressV20_t;

typedef enum drc_compsGain_mode_e {
    DRC_COMPSGAIN_NORMAL0    = 0,
    DRC_COMPSGAIN_NORMAL1    = 1,
    DRC_COMPSGAIN_LONG_FRAME = 2,
} drc_compsGain_mode_t;

typedef struct hw_adrc_adjCompsGain_params_s {
    // M4_ARRAY_DESC("iso", "f32", M4_SIZE(1,13), M4_RANGE(50,10000000), "[50, 100, 200, 400, 800, 1600, 3200, 6400, 12800, 25600, 51200, 102400, 204800]",M4_DIGIT(2), M4_DYNAMIC(0), 0)
    float iso[ADRC_ENVLV_STEP_MAX];
    // M4_ARRAY_DESC("hw_adrc_loDetail_ratio", "u16", M4_SIZE(1,13), M4_RANGE(0,4095), "[0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]",M4_DIGIT(0), M4_DYNAMIC(0), 0)
    uint16_t hw_adrc_loDetail_ratio[ADRC_ENVLV_STEP_MAX];
    // M4_ARRAY_DESC("hw_adrc_hiDetail_ratio", "u16", M4_SIZE(1,13), M4_RANGE(0,4095), "[0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]",M4_DIGIT(0), M4_DYNAMIC(0), 0)
    uint16_t hw_adrc_hiDetail_ratio[ADRC_ENVLV_STEP_MAX];
} hw_adrc_adjCompsGain_params_t;

typedef struct hw_adrc_adjCompsGain_setting_s {
    // M4_ENUM_DESC("hw_adrc_compsGain_minLimit", "drc_compsGain_mode_t", "DRC_COMPSGAIN_NORMAL1")
    drc_compsGain_mode_t hw_adrc_compsGain_minLimit;
    // M4_ARRAY_TABLE_DESC("hw_adrc_adjCompsGain_params", "array_table_ui", "none")
    hw_adrc_adjCompsGain_params_t hw_adrc_adjCompsGain_params;
    // M4_NUMBER_DESC("hw_adrc_adjCompsGainIdxLuma_scl", "u8", M4_RANGE(0,255), "64", M4_DIGIT(0))
    uint8_t hw_adrc_adjCompsGainIdxLuma_scl;
    // M4_ARRAY_DESC("hw_adrc_luma2compsGainScale_val", "u16", M4_SIZE(1,17),  M4_RANGE(0, 2048),"[0,2,20,76,193,381,631,772,919,1066,1211,1479,1700,1863,1968,2024,2048]", M4_DIGIT(0), M4_DYNAMIC(0))
    uint16_t hw_adrc_luma2compsGainScale_val[ADRC_Y_NUM];
} hw_adrc_adjCompsGain_setting_t;

typedef enum comps_mode_e {
    COMPS_MODE_LOG_I4F8 = 0,
    COMPS_MODE_LOG_I3F9 = 1,
} comps_mode_t;

typedef struct adrcCmpsSetting_s {
    // M4_ENUM_DESC("hw_adrc_comps_mode", "comps_mode_t", "COMPS_MODE_LOG_I4F8")
    comps_mode_t hw_adrc_comps_mode;
    // M4_NUMBER_DESC("hw_adrc_comps_offset", "u8", M4_RANGE(0,15), "8", M4_DIGIT(0))
    uint8_t hw_adrc_comps_offset;
} adrcCmpsSetting_t;

typedef struct CalibDbV2_Adrc_V20_s {
    // M4_BOOL_DESC("hw_adrc_en", "0")
    bool hw_adrc_en;
    // M4_STRUCT_DESC("hw_adrc_cmps_setting", "normal_ui_style")
    adrcCmpsSetting_t hw_adrc_cmps_setting;
    // M4_STRUCT_DESC("hw_adrc_luma2drcGain_setting", "normal_ui_style")
    AdrcGainV20_t hw_adrc_luma2drcGain_setting;
    // M4_STRUCT_DESC("hw_adrc_biflt_setting", "normal_ui_style")
    bifltSettingV20_t hw_adrc_biflt_setting;
    // M4_STRUCT_DESC("hw_adrc_luma2compsLuma_setting", "normal_ui_style")
    CompressV20_t hw_adrc_luma2compsLuma_setting;
    // M4_STRUCT_DESC("hw_adrc_adjCompsGain_setting", "normal_ui_style")
    hw_adrc_adjCompsGain_setting_t hw_adrc_adjCompsGain_setting;
    // M4_NUMBER_DESC("sw_adrc_byPass_thred", "f32", M4_RANGE(0,1), "0", M4_DIGIT(4))
    float sw_adrc_byPass_thred;
    // M4_NUMBER_DESC("sw_adrc_damp_coef", "f32", M4_RANGE(0,1), "0.9", M4_DIGIT(3))
    float sw_adrc_damp_coef;
} CalibDbV2_Adrc_V20_t;

typedef struct CalibDbV2_drc_V20_s {
    // M4_STRUCT_DESC("DrcTuningPara", "normal_ui_style")
    CalibDbV2_Adrc_V20_t DrcTuningPara;
} CalibDbV2_drc_V20_t;

#endif
