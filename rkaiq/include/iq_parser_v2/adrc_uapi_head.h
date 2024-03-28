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

#ifndef __ADRC_UAPI_HEAD_H__
#define __ADRC_UAPI_HEAD_H__

#include "adrc_head.h"

// drc v10
typedef struct mDrcGain_t {
    // M4_NUMBER_DESC("DrcGain", "f32", M4_RANGE(1,8), "1.00", M4_DIGIT(2))
    float DrcGain;
    // M4_NUMBER_DESC("Alpha", "f32", M4_RANGE(0,1), "0.20", M4_DIGIT(2))
    float Alpha;
    // M4_NUMBER_DESC("Clip", "f32", M4_RANGE(0,64), "16.00", M4_DIGIT(2))
    float Clip;
} mDrcGain_t;

typedef struct mDrcHiLit_s {
    // M4_NUMBER_DESC("Strength", "f32", M4_RANGE(0,1), "1.00", M4_DIGIT(2))
    float Strength;
} mDrcHiLit_t;

typedef struct mLocalDataV10_s {
    // M4_NUMBER_DESC("LocalWeit", "f32", M4_RANGE(0,1), "1.00", M4_DIGIT(2))
    float LocalWeit;
    // M4_NUMBER_DESC("GlobalContrast", "f32", M4_RANGE(0,1), "0.00", M4_DIGIT(3))
    float GlobalContrast;
    // M4_NUMBER_DESC("LoLitContrast", "f32", M4_RANGE(0,1), "0.00", M4_DIGIT(3))
    float LoLitContrast;
} mLocalDataV10_t;

typedef struct mDrcLocalV10_s {
    // M4_STRUCT_DESC("LocalTMOData", "normal_ui_style")
    mLocalDataV10_t LocalTMOData;
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
} mDrcLocalV10_t;

typedef struct mDrcCompress_s {
    // M4_ENUM_DESC("Mode", "CompressMode_t", "COMPRESS_AUTO")
    CompressMode_t Mode;
    // M4_ARRAY_MARK_DESC("Manual_curve", "u32", M4_SIZE(1,17),  M4_RANGE(0, 8192), "[0, 558, 1087, 1588, 2063, 2515, 2944, 3353, 3744, 4473, 5139, 5751, 6316, 6838, 7322, 7772, 8192]", M4_DIGIT(0), M4_DYNAMIC(0), "drc_compress_curve")
    uint16_t Manual_curve[ADRC_Y_NUM];
} mDrcCompress_t;

typedef struct mdrcAttr_V10_s {
    // M4_BOOL_DESC("Enable", "1")
    bool Enable;
    // M4_STRUCT_DESC("DrcGain", "normal_ui_style")
    mDrcGain_t DrcGain;
    // M4_STRUCT_DESC("HiLight", "normal_ui_style")
    mDrcHiLit_t HiLight;
    // M4_STRUCT_DESC("LocalTMOSetting", "normal_ui_style")
    mDrcLocalV10_t LocalTMOSetting;
    // M4_STRUCT_DESC("CompressSetting", "normal_ui_style")
    mDrcCompress_t CompressSetting;
    // M4_ARRAY_DESC("Scale_y", "u16", M4_SIZE(1,17),  M4_RANGE(0, 2048), "[0,2,20,76,193,381,631,772,919,1066,1211,1479,1700,1863,1968,2024,2048]", M4_DIGIT(0), M4_DYNAMIC(0))
    int Scale_y[ADRC_Y_NUM];
    // M4_NUMBER_DESC("Edge_Weit", "f32",  M4_RANGE(0,1), "1",M4_DIGIT(3))
    float Edge_Weit;
    // M4_BOOL_DESC("OutPutLongFrame", "0")
    bool OutPutLongFrame;
    // M4_NUMBER_DESC("IIR_frame", "u8", M4_RANGE(1,1000), "2", M4_DIGIT(0))
    int IIR_frame;
} mdrcAttr_V10_t;

typedef struct DrcInfo_s {
    // M4_NUMBER_DESC("EnvLv", "f32", M4_RANGE(0,1), "0", M4_DIGIT(6))
    float EnvLv;
    // M4_NUMBER_DESC("ISO", "f32", M4_RANGE(50,1000000000), "0", M4_DIGIT(4))
    float ISO;
} DrcInfo_t;

typedef struct DrcInfoV10_s {
    // M4_STRUCT_DESC("CtrlInfo", "normal_ui_style")
    DrcInfo_t CtrlInfo;
    // M4_STRUCT_DESC("ValidParams", "normal_ui_style")
    mdrcAttr_V10_t ValidParams;
} DrcInfoV10_t;

// drc v11
typedef struct mLocalDataV11_s {
    // M4_NUMBER_DESC("LocalWeit", "f32", M4_RANGE(0,1), "1.00", M4_DIGIT(2))
    float LocalWeit;
    // M4_NUMBER_DESC("LocalAutoEnable", "u8", M4_RANGE(0,1), "1", M4_DIGIT(0))
    int LocalAutoEnable;
    // M4_NUMBER_DESC("LocalAutoWeit", "f32", M4_RANGE(0,1), "0.037477", M4_DIGIT(5))
    float LocalAutoWeit;
    // M4_NUMBER_DESC("GlobalContrast", "f32", M4_RANGE(0,1), "0.00", M4_DIGIT(3))
    float GlobalContrast;
    // M4_NUMBER_DESC("LoLitContrast", "f32", M4_RANGE(0,1), "0.00", M4_DIGIT(3))
    float LoLitContrast;
} mLocalDataV11_t;

typedef struct mDrcLocalV11_s {
    // M4_STRUCT_DESC("LocalData", "normal_ui_style")
    mLocalDataV11_t LocalData;
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
} mDrcLocalV11_t;

typedef struct mdrcAttr_V11_s {
    // M4_BOOL_DESC("Enable", "1")
    bool Enable;
    // M4_STRUCT_DESC("DrcGain", "normal_ui_style")
    mDrcGain_t DrcGain;
    // M4_STRUCT_DESC("HiLight", "normal_ui_style")
    mDrcHiLit_t HiLight;
    // M4_STRUCT_DESC("LocalSetting", "normal_ui_style")
    mDrcLocalV11_t LocalSetting;
    // M4_STRUCT_DESC("CompressSetting", "normal_ui_style")
    mDrcCompress_t CompressSetting;
    // M4_ARRAY_DESC("Scale_y", "u16", M4_SIZE(1,17),  M4_RANGE(0, 2048), "[0,2,20,76,193,381,631,772,919,1066,1211,1479,1700,1863,1968,2024,2048]", M4_DIGIT(0), M4_DYNAMIC(0))
    int Scale_y[ADRC_Y_NUM];
    // M4_NUMBER_DESC("Edge_Weit", "f32",  M4_RANGE(0,1), "1",M4_DIGIT(3))
    float Edge_Weit;
    // M4_BOOL_DESC("OutPutLongFrame", "0")
    bool OutPutLongFrame;
    // M4_NUMBER_DESC("IIR_frame", "u8", M4_RANGE(1,1000), "2", M4_DIGIT(0))
    int IIR_frame;
} mdrcAttr_V11_t;

typedef struct DrcInfoV11_s {
    // M4_STRUCT_DESC("CtrlInfo", "normal_ui_style")
    DrcInfo_t CtrlInfo;
    // M4_STRUCT_DESC("ValidParams", "normal_ui_style")
    mdrcAttr_V11_t ValidParams;
} DrcInfoV11_t;

// drc attr V12
typedef struct mHighLightDataV12_s {
    // M4_NUMBER_DESC("Strength", "f32",  M4_RANGE(0,1), "0",M4_DIGIT(2))
    float Strength;
    // M4_NUMBER_DESC("gas_t", "f32",  M4_RANGE(0,2), "0",M4_DIGIT(3))
    float gas_t;
} mHighLightDataV12_t;

typedef struct mHighLightV12_s {
    // M4_STRUCT_DESC("HiLightData", "normal_ui_style")
    mHighLightDataV12_t HiLightData;
    // M4_NUMBER_DESC("gas_l0", "u8", M4_RANGE(0,64), "24", M4_DIGIT(0))
    int gas_l0;
    // M4_NUMBER_DESC("gas_l1", "u8", M4_RANGE(0,64), "10", M4_DIGIT(0))
    int gas_l1;
    // M4_NUMBER_DESC("gas_l2", "u8", M4_RANGE(0,64), "10", M4_DIGIT(0))
    int gas_l2;
    // M4_NUMBER_DESC("gas_l3", "u8", M4_RANGE(0,64), "5", M4_DIGIT(0))
    int gas_l3;
} mHighLightV12_t;

typedef struct mMotionData_s {
    // M4_NUMBER_DESC("Strength", "f32",  M4_RANGE(0,1), "0",M4_DIGIT(3))
    float MotionStr;
} mMotionData_t;

typedef struct mDrcLocalV12_s {
    // M4_STRUCT_DESC("LocalData", "normal_ui_style")
    mLocalDataV11_t LocalData;
    // M4_STRUCT_DESC("MotionData", "normal_ui_style")
    mMotionData_t MotionData;
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
} mDrcLocalV12_t;

typedef struct mdrcAttr_V12_s {
    // M4_BOOL_DESC("Enable", "1")
    bool Enable;
    // M4_STRUCT_DESC("DrcGain", "normal_ui_style")
    mDrcGain_t DrcGain;
    // M4_STRUCT_DESC("HiLight", "normal_ui_style")
    mHighLightV12_t HiLight;
    // M4_STRUCT_DESC("LocalSetting", "normal_ui_style")
    mDrcLocalV12_t LocalSetting;
    // M4_STRUCT_DESC("CompressSetting", "normal_ui_style")
    mDrcCompress_t CompressSetting;
    // M4_ARRAY_DESC("Scale_y", "u16", M4_SIZE(1,17),  M4_RANGE(0, 2048), "[0,2,20,76,193,381,631,772,919,1066,1211,1479,1700,1863,1968,2024,2048]", M4_DIGIT(0), M4_DYNAMIC(0))
    int Scale_y[ADRC_Y_NUM];
    // M4_NUMBER_DESC("Edge_Weit", "f32",  M4_RANGE(0,1), "1",M4_DIGIT(3))
    float Edge_Weit;
    // M4_BOOL_DESC("OutPutLongFrame", "0")
    bool OutPutLongFrame;
    // M4_NUMBER_DESC("IIR_frame", "u8", M4_RANGE(1,1000), "2", M4_DIGIT(0))
    int IIR_frame;
} mdrcAttr_V12_t;

typedef struct DrcInfoV12_s {
    // M4_STRUCT_DESC("CtrlInfo", "normal_ui_style")
    DrcInfo_t CtrlInfo;
    // M4_STRUCT_DESC("ValidParams", "normal_ui_style")
    mdrcAttr_V12_t ValidParams;
} DrcInfoV12_t;

// drc attr V12 lite
typedef struct mDrcLocalV12Lite_s {
    // M4_STRUCT_DESC("LocalData", "normal_ui_style")
    mLocalDataV11_t LocalData;
    // M4_STRUCT_DESC("MotionData", "normal_ui_style")
    mMotionData_t MotionData;
    // M4_NUMBER_DESC("curPixWeit", "f32", M4_RANGE(0,1), "0.37", M4_DIGIT(3))
    float curPixWeit;
    // M4_NUMBER_DESC("Range_force_sgm", "f32", M4_RANGE(0,1), "0.0", M4_DIGIT(4))
    float Range_force_sgm;
    // M4_NUMBER_DESC("Range_sgm_cur", "f32", M4_RANGE(0,1), "0.125", M4_DIGIT(4))
    float Range_sgm_cur;
    // M4_NUMBER_DESC("Space_sgm_cur", "u16", M4_RANGE(0,4095), "4068", M4_DIGIT(0))
    int Space_sgm_cur;
} mDrcLocalV12Lite_t;

typedef struct mdrcAttr_v12_lite_s {
    // M4_BOOL_DESC("Enable", "1")
    bool Enable;
    // M4_STRUCT_DESC("DrcGain", "normal_ui_style")
    mDrcGain_t DrcGain;
    // M4_STRUCT_DESC("HiLight", "normal_ui_style")
    mHighLightV12_t HiLight;
    // M4_STRUCT_DESC("LocalSetting", "normal_ui_style")
    mDrcLocalV12Lite_t LocalSetting;
    // M4_STRUCT_DESC("CompressSetting", "normal_ui_style")
    mDrcCompress_t CompressSetting;
    // M4_ARRAY_DESC("Scale_y", "u16", M4_SIZE(1,17),  M4_RANGE(0, 2048), "[0,2,20,76,193,381,631,772,919,1066,1211,1479,1700,1863,1968,2024,2048]", M4_DIGIT(0), M4_DYNAMIC(0))
    int Scale_y[ADRC_Y_NUM];
    // M4_NUMBER_DESC("Edge_Weit", "f32",  M4_RANGE(0,1), "1",M4_DIGIT(3))
    float Edge_Weit;
    // M4_BOOL_DESC("OutPutLongFrame", "0")
    bool OutPutLongFrame;
    // M4_NUMBER_DESC("IIR_frame", "u8", M4_RANGE(1,1000), "2", M4_DIGIT(0))
    int IIR_frame;
} mdrcAttr_v12_lite_t;

typedef struct DrcInfoV12Lite_s {
    // M4_STRUCT_DESC("CtrlInfo", "normal_ui_style")
    DrcInfo_t CtrlInfo;
    // M4_STRUCT_DESC("ValidParams", "normal_ui_style")
    mdrcAttr_v12_lite_t ValidParams;
} DrcInfoV12Lite_t;

typedef struct mDrc_gain_drcGain_auto_s {
    // M4_NUMBER_DESC("sw_adrc_drcGain_maxLimit", "f32", M4_RANGE(1,8), "1.00", M4_DIGIT(2))
    float sw_adrc_drcGain_maxLimit;
    // M4_NUMBER_DESC("sw_adrc_drcGainLumaAdj_scale", "f32", M4_RANGE(0,1), "0.20", M4_DIGIT(2))
    float sw_adrc_drcGainLumaAdj_scale;
} mDrc_gain_drcGain_auto_t;

typedef struct mDrcGainV20_s {
    // M4_ENUM_DESC("sw_adrc_drcGain_mode", "drc_gain_mode_t", "DRC_GAIN_TABLE_AUTO")
    drc_gain_mode_t sw_adrc_drcGain_mode;
    // M4_NUMBER_DESC("hw_adrc_drcGainIdxLuma_scale", "f32", M4_RANGE(0,64), "1.00", M4_DIGIT(2))
    float hw_adrc_drcGainIdxLuma_scale;
    // M4_ARRAY_MARK_DESC("hw_adrc_luma2drcGain_manualVal", "u32", M4_SIZE(1,17),  M4_RANGE(0, 8191), "[1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024]", M4_DIGIT(0), M4_DYNAMIC(0))
    uint16_t hw_adrc_luma2drcGain_manualVal[ADRC_Y_NUM];
    mDrc_gain_drcGain_auto_t sw_adrc_luma2drcGain_autoVal;
} mDrcGainV20_t;

typedef struct mLocalDataV20_s {
    // M4_NUMBER_DESC("sw_adrc_bifilt_wgt", "u8", M4_RANGE(0,16), "16", M4_DIGIT(0))
    uint8_t hw_adrc_bifilt_wgt;
    // M4_NUMBER_DESC("hw_adrc_bifiltSoftThred_en", "u8", M4_RANGE(0,1), "1", M4_DIGIT(0))
    uint8_t hw_adrc_bifiltSoftThred_en;
    // M4_NUMBER_DESC("sw_adrc_bifilt_softThred", "u16", M4_RANGE(0,2047), "119", M4_DIGIT(0))
    uint16_t hw_adrc_bifilt_softThred;
} mLocalDataV20_t;

typedef struct mBifltSettingV20_s {
    // M4_ARRAY_TABLE_DESC("hw_adrc_biflt_params", "array_table_ui", "none")
    mLocalDataV20_t hw_adrc_biflt_params;
    // M4_ARRAY_TABLE_DESC("hw_adrc_thumb_setting", "array_table_ui", "none")
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
} mBifltSettingV20_t;

typedef struct hw_mdrc_adjCompsGain_params_s {
    // M4_NUMBER_DESC("sw_adrc_loDetail_ratio", "u16", M4_RANGE(0,4095), "0.00", M4_DIGIT(0))
    uint16_t hw_adrc_loDetail_ratio;
    // M4_NUMBER_DESC("sw_adrc_hiDetail_ratio", "u16", M4_RANGE(0,4095), "0.00", M4_DIGIT(0))
    uint16_t hw_adrc_hiDetail_ratio;
} hw_mdrc_adjCompsGain_params_t;

typedef struct hw_mdrc_adjCompsGain_setting_s {
    // M4_ENUM_DESC("hw_adrc_compsGain_minLimit", "drc_compsGain_mode_t", "DRC_COMPSGAIN_NORMAL1")
    drc_compsGain_mode_t hw_adrc_compsGain_minLimit;
    // M4_STRUCT_DESC("hw_adrc_adjCompsGain_params_t", "array_table_ui", "none")
    hw_mdrc_adjCompsGain_params_t hw_adrc_adjCompsGain_params;
    // M4_NUMBER_DESC("hw_adrc_adjCompsGainIdxLuma_scl", "u8", M4_RANGE(0,255), "64", M4_DIGIT(0))
    uint8_t hw_adrc_adjCompsGainIdxLuma_scl;
    // M4_ARRAY_DESC("hw_adrc_luma2compsGainScale_val", "u16", M4_SIZE(1,17),  M4_RANGE(0, 2048), "[0,2,20,76,193,381,631,772,919,1066,1211,1479,1700,1863,1968,2024,2048]", M4_DIGIT(0), M4_DYNAMIC(0))
    uint16_t hw_adrc_luma2compsGainScale_val[ADRC_Y_NUM];
} hw_mdrc_adjCompsGain_setting_t;

typedef struct mdrcAttr_V20_s {
    // M4_BOOL_DESC("hw_adrc_en", "1")
    bool hw_adrc_en;
    // M4_ARRAY_TABLE_DESC("hw_adrc_cmps_setting", "array_table_ui", "none")
    adrcCmpsSetting_t hw_adrc_cmps_setting;
    // M4_ARRAY_TABLE_DESC("hw_adrc_luma2drcGain_setting", "array_table_ui", "none")
    mDrcGainV20_t hw_adrc_luma2drcGain_setting;
    // M4_STRUCT_DESC("LocalSetting", "normal_ui_style")
    mBifltSettingV20_t hw_adrc_biflt_setting;
    // M4_STRUCT_DESC("hw_adrc_luma2compsLuma_setting", "normal_ui_style")
    CompressV20_t hw_adrc_luma2compsLuma_setting;
    // M4_STRUCT_DESC("hw_adrc_adjCompsGain_setting", "normal_ui_style")
    hw_mdrc_adjCompsGain_setting_t hw_adrc_adjCompsGain_setting;
} mdrcAttr_V20_t;

typedef struct DrcInfoV20_s {
    // M4_STRUCT_DESC("CtrlInfo", "normal_ui_style")
    DrcInfo_t CtrlInfo;
    // M4_STRUCT_DESC("ValidParams", "normal_ui_style")
    mdrcAttr_V20_t ValidParams;
} DrcInfoV20_t;

#endif /*__ADRC_UAPI_HEAD_H__*/
