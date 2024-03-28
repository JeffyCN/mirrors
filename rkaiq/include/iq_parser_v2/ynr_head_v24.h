/*
 * ynr_head_v24.h
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

#ifndef __CALIBDBV2_YNRV24_HEADER_H__
#define __CALIBDBV2_YNRV24_HEADER_H__

#include "rk_aiq_comm.h"

RKAIQ_BEGIN_DECLARE

///////////////////////////ynr v1//////////////////////////////////////

typedef struct CalibDbV2_YnrV24_C_ISO_s {
    // M4_NUMBER_MARK_DESC("iso", "f32", M4_RANGE(50, 204800), "50", M4_DIGIT(1), "index2")
    float iso;
    // M4_ARRAY_DESC("sw_ynr_luma2sigma_curve", "f32", M4_SIZE(1,5), M4_RANGE(-65535.0, 65535), "0.0", M4_DIGIT(20), M4_DYNAMIC(0))
    double sw_ynr_luma2sigma_curve[5];
    // M4_NUMBER_DESC("ynr_lci", "f32", M4_RANGE(0.0, 2.0), "0.5", M4_DIGIT(3))
    float ynr_lci;
} CalibDbV2_YnrV24_C_ISO_t;

typedef struct CalibDbV2_YnrV24_C_Set_s {
    // M4_STRING_MARK_DESC("SNR_Mode", M4_SIZE(1,1), M4_RANGE(0, 64), "LSNR",M4_DYNAMIC(0), "index1")
    char* SNR_Mode;
    // M4_STRING_DESC("Sensor_Mode", M4_SIZE(1,1), M4_RANGE(0, 64), "lcg", M4_DYNAMIC(0))
    char* Sensor_Mode;
    // M4_STRUCT_LIST_DESC("Calib_ISO", M4_SIZE_DYNAMIC, "double_index_list")
    CalibDbV2_YnrV24_C_ISO_t* Calib_ISO;
    int Calib_ISO_len;
} CalibDbV2_YnrV24_C_Set_t;

typedef struct CalibDbV2_YnrV24_Calib_s {
    // M4_STRUCT_LIST_DESC("Setting", M4_SIZE_DYNAMIC, "double_index_list")
    CalibDbV2_YnrV24_C_Set_t* Setting;
    int Setting_len;
} CalibDbV2_YnrV24_Calib_t;

typedef struct CalibDbV2_YnrV24_LumaParam_s {
    // M4_ARRAY_DESC("sw_ynr_luma2loStrg_lumaPoint", "u16", M4_SIZE(1,6), M4_RANGE(0,256), "[0,32,64,128,192,256]", M4_DIGIT(0), M4_DYNAMIC(0))
    uint16_t sw_ynr_luma2loStrg_lumaPoint[6];
    // M4_ARRAY_DESC("sw_ynr_luma2loStrg_val", "f32", M4_SIZE(1,6), M4_RANGE(0,2.0), "[1.0,1.0,1.0,1.0,1.0,1.0]", M4_DIGIT(2), M4_DYNAMIC(0))
    float sw_ynr_luma2loStrg_val[6];
} CalibDbV2_YnrV24_LumaParam_t;

typedef struct CalibDbV2_YnrV24_T_ISO_s {
    // M4_NUMBER_MARK_DESC("iso", "f32", M4_RANGE(50, 204800), "50", M4_DIGIT(1), "index2")
    float iso;

    // M4_BOOL_DESC("sw_ynr_loSpnr_bypass", "0")
    bool sw_ynr_loSpnr_bypass;
    // M4_BOOL_DESC("sw_ynr_hiSpnr_bypass", "0")
    bool sw_ynr_hiSpnr_bypass;

    // M4_NUMBER_DESC("sw_ynr_gainMerge_alpha", "f32", M4_RANGE(0, 1.0), "0", M4_DIGIT(2))
    float sw_ynr_gainMerge_alpha;
    // M4_NUMBER_DESC("sw_ynr_globalSet_gain", "f32", M4_RANGE(0, 64), "1", M4_DIGIT(2))
    float sw_ynr_globalSet_gain;

    // sw_ynr_hiSpnr_ctrl
    // M4_NUMBER_DESC("sw_ynr_hiSpnr_strg", "f32", M4_RANGE(0, 16.0), "0.4", M4_DIGIT(2))
    float sw_ynr_hiSpnr_strg;
    // M4_NUMBER_DESC("sw_ynr_hiSpnrLocalGain_alpha", "f32", M4_RANGE(0, 1.0), "0.25", M4_DIGIT(2))
    float sw_ynr_hiSpnrLocalGain_alpha;
    // M4_NUMBER_DESC("sw_ynr_hiSpnrFilt_centerWgt", "f32", M4_RANGE(0, 128.0), "2.0", M4_DIGIT(2))
    float sw_ynr_hiSpnrFilt_centerWgt;
    // M4_NUMBER_DESC("sw_ynr_hiSpnrFilt_wgtOffset", "f32", M4_RANGE(0, 1.0), "0.0", M4_DIGIT(2))
    float sw_ynr_hiSpnrFilt_wgtOffset;
    // M4_NUMBER_DESC("sw_ynr_hiSpnrSigma_minLimit", "f32", M4_RANGE(0, 1.0), "0.0028", M4_DIGIT(4))
    float sw_ynr_hiSpnrSigma_minLimit;
    // M4_NUMBER_DESC("sw_ynr_hiSpnrFilt_wgt", "f32", M4_RANGE(0, 1.0), "0.45", M4_DIGIT(2))
    float sw_ynr_hiSpnrFilt_wgt;
    // M4_NUMBER_DESC("sw_ynr_hiSpnr_gainThred", "f32", M4_RANGE(0, 64.0), "2.0", M4_DIGIT(2))
    float sw_ynr_hiSpnr_gainThred;
    // M4_NUMBER_DESC("sw_ynr_hiSpnrStrongEdge_scale", "f32", M4_RANGE(0, 32.0), "4.0", M4_DIGIT(2))
    float sw_ynr_hiSpnrStrongEdge_scale;
    // M4_NUMBER_DESC("sw_ynr_hiSpnrFilt_coeff0", "f32", M4_RANGE(0, 15), "7", M4_DIGIT(0))
    float sw_ynr_hiSpnrFilt_coeff0;
    // M4_NUMBER_DESC("sw_ynr_hiSpnrFilt_coeff1", "f32", M4_RANGE(0, 15), "6", M4_DIGIT(0))
    float sw_ynr_hiSpnrFilt_coeff1;
    // M4_NUMBER_DESC("sw_ynr_hiSpnrFilt_coeff2", "f32", M4_RANGE(0, 15), "3", M4_DIGIT(0))
    float sw_ynr_hiSpnrFilt_coeff2;
    // M4_NUMBER_DESC("sw_ynr_hiSpnrFilt_coeff3", "f32", M4_RANGE(0, 15), "6", M4_DIGIT(0))
    float sw_ynr_hiSpnrFilt_coeff3;
    // M4_NUMBER_DESC("sw_ynr_hiSpnrFilt_coeff4", "f32", M4_RANGE(0, 15), "5", M4_DIGIT(0))
    float sw_ynr_hiSpnrFilt_coeff4;
    // M4_NUMBER_DESC("sw_ynr_hiSpnrFilt_coeff5", "f32", M4_RANGE(0, 15), "3", M4_DIGIT(0))
    float sw_ynr_hiSpnrFilt_coeff5;

    // sw_ynr_dsFilt_ctrl
    // M4_NUMBER_DESC("sw_ynr_dsImg_edgeScale", "f32", M4_RANGE(0, 6), "1", M4_DIGIT(0))
    float sw_ynr_dsImg_edgeScale;
    // M4_NUMBER_DESC("sw_ynr_dsFiltSoftThred_scale", "f32", M4_RANGE(0, 16.0), "0.25", M4_DIGIT(2))
    float sw_ynr_dsFiltSoftThred_scale;
    // M4_NUMBER_DESC("sw_ynr_dsFiltWgtThred_scale", "f32", M4_RANGE(0, 8.0), "0.1875", M4_DIGIT(2))
    float sw_ynr_dsFiltWgtThred_scale;
    // M4_NUMBER_DESC("sw_ynr_dsFilt_centerWgt", "f32", M4_RANGE(0, 2.0), "0.025", M4_DIGIT(4))
    float sw_ynr_dsFilt_centerWgt;
    // M4_NUMBER_DESC("sw_ynr_dsFilt_strg", "f32", M4_RANGE(0.0, 32.0), "0.3", M4_DIGIT(3))
    float sw_ynr_dsFilt_strg;
    // M4_NUMBER_DESC("sw_ynr_dsIIRinitWgt_scale", "f32", M4_RANGE(0, 1.0), "0.06", M4_DIGIT(2))
    float sw_ynr_dsIIRinitWgt_scale;
    // M4_NUMBER_DESC("sw_ynr_dsFiltLocalGain_alpha", "f32", M4_RANGE(0, 1.0), "0.25", M4_DIGIT(2))
    float sw_ynr_dsFiltLocalGain_alpha;

    // sw_ynr_loSpnr_ctrl
    // M4_NUMBER_DESC("sw_ynr_preFilt_strg", "f32", M4_RANGE(0, 16.0), "1", M4_DIGIT(2))
    float sw_ynr_preFilt_strg;
    // M4_NUMBER_DESC("sw_ynr_loSpnr_wgt", "f32", M4_RANGE(0, 1.0), "0.5", M4_DIGIT(2))
    float sw_ynr_loSpnr_wgt;
    // M4_NUMBER_DESC("sw_ynr_loSpnr_centerWgt", "f32", M4_RANGE(0, 4.0), "1.2", M4_DIGIT(2))
    float sw_ynr_loSpnr_centerWgt;
    // M4_NUMBER_DESC("sw_ynr_loSpnr_strg", "f32", M4_RANGE(0, 128.0), "0.45", M4_DIGIT(2))
    float sw_ynr_loSpnr_strg;
    // M4_NUMBER_DESC("sw_ynr_loSpnrDistVstrg_scale", "f32", M4_RANGE(0, 8.0), "0.0625", M4_DIGIT(4))
    float sw_ynr_loSpnrDistVstrg_scale;
    // M4_NUMBER_DESC("sw_ynr_loSpnrDistHstrg_scale", "f32", M4_RANGE(0, 8.0), "0.75", M4_DIGIT(4))
    float sw_ynr_loSpnrDistHstrg_scale;

    // M4_ARRAY_TABLE_DESC("lumaPara", "array_table_ui", "none")
    CalibDbV2_YnrV24_LumaParam_t lumaPara;

    // M4_ARRAY_DESC("sw_ynr_radius2strg_val", "f32", M4_SIZE(1,17), M4_RANGE(0,16), "1.0", M4_DIGIT(3), M4_DYNAMIC(0))
    float sw_ynr_radius2strg_val[17];
    // M4_ARRAY_DESC("sw_ynr_loSpnrGain2Strg_val", "f32", M4_SIZE(1,9), M4_RANGE(0,16), "[0,1,2,4,8,16,32,64,128]", M4_DIGIT(3), M4_DYNAMIC(0))
    float sw_ynr_loSpnrGain2Strg_val[9];
} CalibDbV2_YnrV24_T_ISO_t;

typedef struct CalibDbV2_YnrV24_T_Set_s {
    // M4_STRING_MARK_DESC("SNR_Mode", M4_SIZE(1,1), M4_RANGE(0, 64), "LSNR",M4_DYNAMIC(0), "index1")
    char* SNR_Mode;
    // M4_STRING_DESC("Sensor_Mode", M4_SIZE(1,1), M4_RANGE(0, 64), "lcg", M4_DYNAMIC(0))
    char* Sensor_Mode;
    // M4_STRUCT_LIST_DESC("Tuning_ISO", M4_SIZE_DYNAMIC, "double_index_list")
    CalibDbV2_YnrV24_T_ISO_t* Tuning_ISO;
    int Tuning_ISO_len;
} CalibDbV2_YnrV24_T_Set_t;

typedef struct CalibDbV2_YnrV24_Tuning_s {
    // M4_BOOL_DESC("enable", "1")
    bool enable;
    // M4_STRUCT_LIST_DESC("Setting", M4_SIZE_DYNAMIC, "double_index_list")
    CalibDbV2_YnrV24_T_Set_t* Setting;
    int Setting_len;
} CalibDbV2_YnrV24_Tuning_t;

typedef struct CalibDbV2_YnrV24_s {
    // M4_STRING_DESC("Version", M4_SIZE(1,1), M4_RANGE(0, 64), "V24", M4_DYNAMIC(0))
    char* Version;
    // M4_STRUCT_DESC("CalibPara", "normal_ui_style")
    CalibDbV2_YnrV24_Calib_t CalibPara;
    // M4_STRUCT_DESC("TuningPara", "normal_ui_style")
    CalibDbV2_YnrV24_Tuning_t TuningPara;
} CalibDbV2_YnrV24_t;

RKAIQ_END_DECLARE

#endif
