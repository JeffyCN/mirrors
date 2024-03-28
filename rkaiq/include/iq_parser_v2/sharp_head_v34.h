/*
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
#ifndef _SHARP_HEAD_V34_H_
#define _SHARP_HEAD_V34_H_

#include "rk_aiq_comm.h"

RKAIQ_BEGIN_DECLARE

///////////////////////////sharp//////////////////////////////////////
// clang-format off

typedef struct CalibDbV2_SharpV34_Luma_t {
    // M4_ARRAY_DESC("hw_sharp_luma2Table_idx", "u16", M4_SIZE(1,8), M4_RANGE(0,1024), "[0, 64, 128, 256, 384, 640, 896, 1024]", M4_DIGIT(1), M4_DYNAMIC(0))
    uint16_t hw_sharp_luma2Table_idx[8];
    // M4_ARRAY_DESC("sw_sharp_luma2vsigma_val", "u16", M4_SIZE(1,8), M4_RANGE(0,1023), "[8, 12, 16, 16, 24, 20, 16, 16]", M4_DIGIT(0), M4_DYNAMIC(0))
    uint16_t sw_sharp_luma2vsigma_val[8];
    // M4_ARRAY_DESC("hw_sharp_luma2posClip_val", "u16", M4_SIZE(1,8), M4_RANGE(0,1023), "[256, 256, 256, 256, 350, 512, 512, 512]", M4_DIGIT(0), M4_DYNAMIC(0))
    uint16_t hw_sharp_luma2posClip_val[8];
	// M4_ARRAY_DESC("hw_sharp_luma2negClip_val", "u16", M4_SIZE(1,8), M4_RANGE(0,1023), "[256, 256, 256, 256, 350, 512, 512, 512]", M4_DIGIT(0), M4_DYNAMIC(0))
    uint16_t hw_sharp_luma2negClip_val[8];
	// M4_ARRAY_DESC("hw_sharp_luma2strg_val", "f32", M4_SIZE(1,8), M4_RANGE(0,1023), "[256, 256, 256, 256, 350, 512, 512, 512]", M4_DIGIT(1), M4_DYNAMIC(0))
    float hw_sharp_luma2strg_val[8];
} CalibDbV2_SharpV34_Luma_t;

typedef struct CalibDbV2_SharpV34_kernel_s {
    // M4_ARRAY_DESC("sw_sharp_preBifilt_coeff", "f32", M4_SIZE(1,3), M4_RANGE(0,1), "[0.2042,0.1238,0.0751]", M4_DIGIT(4), M4_DYNAMIC(0))
    float sw_sharp_preBifilt_coeff[3];
    // M4_ARRAY_DESC("sw_sharp_imgLpf0_coeff", "f32", M4_SIZE(1,6), M4_RANGE(0.0,1.0), "[0.2042,0.1238,0.0751,0,0,0]", M4_DIGIT(4), M4_DYNAMIC(0))
    float sw_sharp_imgLpf0_coeff[6];
	// M4_ARRAY_DESC("sw_sharp_imgLpf1_coeff", "f32", M4_SIZE(1,6), M4_RANGE(0.0,1.0), "[0.2042,0.1238,0.0751,0,0,0]", M4_DIGIT(4), M4_DYNAMIC(0))
    float sw_sharp_imgLpf1_coeff[6];
    // M4_ARRAY_DESC("sw_sharp_detailBifilt_coeff", "f32", M4_SIZE(1,3), M4_RANGE(0.0,1.0), "[0.2042,0.1238,0.0751]", M4_DIGIT(4), M4_DYNAMIC(0))
    float sw_sharp_detailBifilt_coeff[3];
} CalibDbV2_SharpV34_kernel_t;

typedef struct CalibDbV2_SharpV34_Ksigma_s {
    // M4_NUMBER_MARK_DESC("sw_sharp_preBifilt_rsigma", "f32", M4_RANGE(0.0, 100.0), "1.0", M4_DIGIT(2))
    float sw_sharp_preBifilt_rsigma;
    // M4_NUMBER_MARK_DESC("sw_sharp_detailBifilt_rsigma", "f32", M4_RANGE(0.0, 100.0), "1.0", M4_DIGIT(2))
    float sw_sharp_detailBifilt_rsigma;
    // M4_NUMBER_MARK_DESC("sw_sharp_imgLpf0_rsigma", "f32", M4_RANGE(0.0, 100.0), "0.85", M4_DIGIT(2))
    float sw_sharp_imgLpf0_rsigma;
    // M4_NUMBER_MARK_DESC("sw_sharp_imgLpf0_radius", "u8", M4_RANGE(1, 2), "2", M4_DIGIT(0))
    uint8_t sw_sharp_imgLpf0_radius;
	// M4_NUMBER_MARK_DESC("sw_sharp_imgLpf1_rsigma", "f32", M4_RANGE(0.0, 100.0), "0.85", M4_DIGIT(2))
    float sw_sharp_imgLpf1_rsigma;
    // M4_NUMBER_MARK_DESC("sw_sharp_imgLpf1_radius", "u8", M4_RANGE(1, 2), "2", M4_DIGIT(0))
    uint8_t sw_sharp_imgLpf1_radius;
} CalibDbV2_SharpV34_Ksigma_t;

typedef struct CalibDbV2_SharpV34_T_ISO_s {
	// M4_NUMBER_MARK_DESC("iso", "f32", M4_RANGE(50, 204800), "50", M4_DIGIT(1), "index2")
	float iso;

	// M4_BOOL_DESC("hw_sharp_localGain_bypass", "0")
	bool hw_sharp_localGain_bypass;
	
	// M4_NUMBER_DESC("sw_sharp_preBifilt_scale", "f32", M4_RANGE(0.0, 4.0), "0.5", M4_DIGIT(2))
	float sw_sharp_preBifilt_scale;
	// M4_NUMBER_DESC("sw_sharp_preBifilt_offset", "f32", M4_RANGE(0.0, 1023.0), "0.0", M4_DIGIT(2))
	float sw_sharp_preBifilt_offset;
	// M4_NUMBER_DESC("sw_sharp_preBifilt_alpha", "f32", M4_RANGE(0.0, 31.275), "8", M4_DIGIT(2))
	float sw_sharp_preBifilt_alpha;
	
	// M4_NUMBER_DESC("sw_sharp_guideFilt_alpha", "f32", M4_RANGE(0.0, 31.275), "8", M4_DIGIT(2))
	float sw_sharp_guideFilt_alpha;
	// M4_NUMBER_DESC("sw_sharp_imgLpf0_strg", "f32", M4_RANGE(0.0, 31.275), "3", M4_DIGIT(2))
	float sw_sharp_imgLpf0_strg;
	// M4_NUMBER_DESC("sw_sharp_imgLpf1_strg", "f32", M4_RANGE(0.0, 31.275), "3", M4_DIGIT(2))
	float sw_sharp_imgLpf1_strg;
	// M4_NUMBER_DESC("hw_sharp_baseImg_sel", "u8", M4_RANGE(0, 1), "1", M4_DIGIT(0))
	uint8_t hw_sharp_baseImg_sel;
	// M4_NUMBER_DESC("hw_sharp_clipIdx_sel", "u8", M4_RANGE(0, 1), "1", M4_DIGIT(0))
	uint8_t hw_sharp_clipIdx_sel;
	
	// M4_NUMBER_DESC("sw_sharp_detailBifilt_scale", "f32", M4_RANGE(0.0, 4.0), "0.5", M4_DIGIT(2))
	float sw_sharp_detailBifilt_scale;
	// M4_NUMBER_DESC("sw_sharp_detailBifilt_offset", "f32", M4_RANGE(0.0, 1023.0), "0.0", M4_DIGIT(2))
	float sw_sharp_detailBifilt_offset;
	// M4_NUMBER_DESC("sw_sharp_detailBifilt_alpha", "f32", M4_RANGE(0.0, 1.0), "0.0", M4_DIGIT(2))
	float sw_sharp_detailBifilt_alpha;

	// M4_NUMBER_DESC("sw_sharp_global_gain", "f32", M4_RANGE(0.0, 63.0), "1.0", M4_DIGIT(2))
	float sw_sharp_global_gain;
	// M4_NUMBER_DESC("sw_sharp_gainMerge_alpha", "f32", M4_RANGE(0.0, 1.0), "0.0", M4_DIGIT(2))
	float sw_sharp_gainMerge_alpha;
	// M4_NUMBER_DESC("sw_sharp_localGain_scale", "f32", M4_RANGE(0.0, 1.0), "0.0", M4_DIGIT(2))
	float sw_sharp_localGain_scale;
	
	// M4_ARRAY_DESC("sw_sharp_gain2strg_val", "f32", M4_SIZE(1,14), M4_RANGE(0.0,31.275), "[1.0]", M4_DIGIT(3), M4_DYNAMIC(0))
	float sw_sharp_gain2strg_val[14];
	// M4_ARRAY_DESC("sw_sharp_distance2strg_val", "f32", M4_SIZE(1,22), M4_RANGE(0.0,1.0), "[1.0]", M4_DIGIT(3), M4_DYNAMIC(0))
	float sw_sharp_distance2strg_val[22];
	
	// M4_NUMBER_DESC("hw_sharp_noiseFilt_sel", "u8", M4_RANGE(0, 1), "0", M4_DIGIT(0))
	float hw_sharp_noiseFilt_sel;
	// M4_NUMBER_DESC("hw_sharp_noiseNorm_bit", "u8", M4_RANGE(0, 10), "1", M4_DIGIT(0))
	uint8_t hw_sharp_noiseNorm_bit;
	// M4_NUMBER_DESC("hw_sharp_noise_maxLimit", "u16", M4_RANGE(0, 1023), "1023", M4_DIGIT(0))
	uint16_t hw_sharp_noise_maxLimit;
	// M4_NUMBER_DESC("hw_sharp_noiseClip_sel", "u8", M4_RANGE(0, 1), "8", M4_DIGIT(0))
	float hw_sharp_noiseClip_sel;
	// M4_NUMBER_DESC("sw_sharp_noise_strg", "f32", M4_RANGE(0, 10.0), "1", M4_DIGIT(1))
	float sw_sharp_noise_strg;
	// M4_NUMBER_DESC("sw_sharp_texReserve_strg", "f32", M4_RANGE(0.0, 31), "8", M4_DIGIT(2))
	float sw_sharp_texReserve_strg;
	
	// M4_NUMBER_DESC("hw_sharp_tex2wgt_en", "u8", M4_RANGE(0, 1), "0", M4_DIGIT(0))
	float hw_sharp_tex2wgt_en;
	// M4_ARRAY_DESC("hw_sharp_tex2wgt_val", "u16", M4_SIZE(1,17), M4_RANGE(0,1023), "[1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023]", M4_DIGIT(0), M4_DYNAMIC(0))
	uint16_t hw_sharp_tex2wgt_val[17];

	// M4_ARRAY_DESC("hw_sharp_detail2strg_val", "u16", M4_SIZE(1,17), M4_RANGE(0,1023), "[0, 3, 30, 104, 224, 372, 528, 672, 791, 882, 945, 985, 1008, 1019, 1023, 1024, 1024]", M4_DIGIT(0), M4_DYNAMIC(0))
	uint16_t hw_sharp_detail2strg_val[17];
	
	// M4_ARRAY_TABLE_DESC("lumaPara", "array_table_ui", "none")
	CalibDbV2_SharpV34_Luma_t luma_para;
	// M4_ARRAY_TABLE_DESC("kernel_para", "array_table_ui", "none")
	CalibDbV2_SharpV34_kernel_t kernel_para;
	// M4_ARRAY_TABLE_DESC("kernel_sigma", "array_table_ui", "none")
	CalibDbV2_SharpV34_Ksigma_t kernel_sigma;
} CalibDbV2_SharpV34_T_ISO_t;

typedef struct CalibDbV2_SharpV34_T_Set_s {
    // M4_STRING_MARK_DESC("SNR_Mode", M4_SIZE(1,1), M4_RANGE(0, 64), "LSNR",M4_DYNAMIC(0), "index1")
    char* SNR_Mode;
    // M4_STRING_DESC("Sensor_Mode", M4_SIZE(1,1), M4_RANGE(0, 64), "lcg", M4_DYNAMIC(0))
    char* Sensor_Mode;
    // M4_STRUCT_LIST_DESC("Tuning_ISO", M4_SIZE_DYNAMIC, "double_indx_list")
    CalibDbV2_SharpV34_T_ISO_t* Tuning_ISO;
    int Tuning_ISO_len;
} CalibDbV2_SharpV34_T_Set_t;

typedef struct CalibDbV2_SharpV34_Tuning_s {
    // M4_BOOL_DESC("enable", "1")
    bool enable;
    // M4_BOOL_DESC("sw_sharp_filtCoeff_mode", "0")
    bool sw_sharp_filtCoeff_mode;
    // M4_BOOL_DESC("hw_sharp_centerPosition_mode", "0")
    bool hw_sharp_centerPosition_mode;
	// M4_NUMBER_DESC("hw_sharp_texWgt_mode", "u8", M4_RANGE(0, 3), "0", M4_DIGIT(0))
	uint8_t hw_sharp_texWgt_mode;
    // M4_STRUCT_LIST_DESC("Setting", M4_SIZE_DYNAMIC, "double_index_list")
    CalibDbV2_SharpV34_T_Set_t* Setting;
    int Setting_len;
} CalibDbV2_SharpV34_Tuning_t;

typedef struct CalibDbV2_SharpV34_s {
    // M4_STRING_DESC("Version", M4_SIZE(1,1), M4_RANGE(0, 64), "V34", M4_DYNAMIC(0))
    char* Version;
    // M4_STRUCT_DESC("TuningPara", "normal_ui_style")
    CalibDbV2_SharpV34_Tuning_t TuningPara;
} CalibDbV2_SharpV34_t;

// clang-format on
RKAIQ_END_DECLARE

#endif
