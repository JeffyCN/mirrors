/*
 * ynr_head_v1.h
 *
 *  Copyright (c) 2024 Rockchip Corporation
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

#ifndef __CALIBDBV2_YUVMEV1_HEADER_H__
#define __CALIBDBV2_YUVMEV1_HEADER_H__

#include "rk_aiq_comm.h"

RKAIQ_BEGIN_DECLARE

///////////////////////////yuvme v1//////////////////////////////////////


typedef struct CalibDbV2_YuvmeV1_T_ISO_s {
    // M4_NUMBER_MARK_DESC("iso", "f32", M4_RANGE(50, 204800), "50", M4_DIGIT(1), "index2")
    float iso;

    // M4_NUMBER_DESC("hw_yuvme_searchRange_mode", "u16", M4_RANGE(0, 2), "2", M4_DIGIT(0))
    uint16_t hw_yuvme_searchRange_mode;
    // M4_NUMBER_DESC("hw_yuvme_timeRelevance_offset", "u16", M4_RANGE(0, 15), "6", M4_DIGIT(0))
    uint16_t hw_yuvme_timeRelevance_offset;
    // M4_NUMBER_DESC("hw_yuvme_spaceRelevance_offset", "u16", M4_RANGE(0, 15), "6", M4_DIGIT(0))
    uint16_t hw_yuvme_spaceRelevance_offset;
    // M4_NUMBER_DESC("hw_yuvme_staticDetect_thred", "u16", M4_RANGE(0, 63), "60", M4_DIGIT(0))
    uint16_t hw_yuvme_staticDetect_thred;


    // M4_NUMBER_DESC("sw_yuvme_globalNr_strg", "f32", M4_RANGE(0, 1.0), "1.0", M4_DIGIT(2))
    float sw_yuvme_globalNr_strg;

    // M4_NUMBER_DESC("sw_yuvme_nrDiff_scale", "f32", M4_RANGE(0, 16.0), "3.0", M4_DIGIT(2))
    float sw_yuvme_nrDiff_scale;

    // M4_NUMBER_DESC("sw_yuvme_nrStatic_scale", "f32", M4_RANGE(0, 16.0), "4.0", M4_DIGIT(2))
    float sw_yuvme_nrStatic_scale;
    // M4_NUMBER_DESC("sw_yuvme_nrFusion_limit", "f32", M4_RANGE(0, 1.0), "0.9", M4_DIGIT(2))
    float sw_yuvme_nrFusion_limit;


    // M4_NUMBER_DESC("sw_yuvme_nrMotion_scale", "f32", M4_RANGE(0.0, 2.0), "1.0", M4_DIGIT(2))
    float sw_yuvme_nrMotion_scale;

    // M4_NUMBER_DESC("hw_yuvme_nrFusion_mode", "u8", M4_RANGE(0, 1), "0", M4_DIGIT(0))
    uint8_t hw_yuvme_nrFusion_mode;
    // M4_NUMBER_DESC("sw_yuvme_curWeight_limit", "f32", M4_RANGE(0.0,1.0), "0.0", M4_DIGIT(2))
    float sw_yuvme_curWeight_limit;

    // M4_ARRAY_DESC("hw_yuvme_nrLuma2Sigma_val", "u16", M4_SIZE(1,16), M4_RANGE(0,1023), "256", M4_DIGIT(0), M4_DYNAMIC(0))
    uint16_t hw_yuvme_nrLuma2Sigma_val[16];

} CalibDbV2_YuvmeV1_T_ISO_t;


typedef struct CalibDbV2_YuvmeV1_T_Set_s {
    // M4_STRING_MARK_DESC("SNR_Mode", M4_SIZE(1,1), M4_RANGE(0, 64), "LSNR",M4_DYNAMIC(0), "index1")
    char *SNR_Mode;
    // M4_STRING_DESC("Sensor_Mode", M4_SIZE(1,1), M4_RANGE(0, 64), "lcg", M4_DYNAMIC(0))
    char *Sensor_Mode;
    // M4_STRUCT_LIST_DESC("Tuning_ISO", M4_SIZE_DYNAMIC, "double_index_list")
    CalibDbV2_YuvmeV1_T_ISO_t *Tuning_ISO;
    int Tuning_ISO_len;
} CalibDbV2_YuvmeV1_T_Set_t;

typedef struct CalibDbV2_YuvmeV1_Tuning_s {
    // M4_BOOL_DESC("enable", "1")
    bool enable;
    // M4_BOOL_DESC("hw_yuvme_btnrMap_en", "1")
    bool hw_yuvme_btnrMap_en;
    // M4_BOOL_DESC("hw_yuvme_debug_mode", "0")
    bool hw_yuvme_debug_mode;
    // M4_STRUCT_LIST_DESC("Setting", M4_SIZE_DYNAMIC, "double_index_list")
    CalibDbV2_YuvmeV1_T_Set_t *Setting;
    int Setting_len;
} CalibDbV2_YuvmeV1_Tuning_t;


typedef struct CalibDbV2_YuvmeV1_s {
    // M4_STRING_DESC("Version", M4_SIZE(1,1), M4_RANGE(0, 64), "V1", M4_DYNAMIC(0))
    char *Version;
    // M4_STRUCT_DESC("TuningPara", "normal_ui_style")
    CalibDbV2_YuvmeV1_Tuning_t TuningPara;
} CalibDbV2_YuvmeV1_t;

RKAIQ_END_DECLARE

#endif
