/*
 * yuvme_head_v1.h
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

#ifndef __CALIBDBV2_YUVMEV1_UAPI_HEADER_H__
#define __CALIBDBV2_YUVMEV1_UAPI_HEADER_H__

#include "rk_aiq_comm.h"
// clang-format off

RKAIQ_BEGIN_DECLARE

///////////////////////////yuvme v1//////////////////////////////////////


typedef struct RK_YUVME_Params_V1_Select_s
{
    // M4_BOOL_DESC("enable", "1")
    bool enable;

    // M4_BOOL_DESC("hw_yuvme_btnrMap_en", "1")
    bool hw_yuvme_btnrMap_en;

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

} RK_YUVME_Params_V1_Select_t;


typedef struct Ayuvme_ExpInfo_V1_s {
    // M4_NUMBER_DESC("hdr_mode", "u8", M4_RANGE(0, 2), "0", M4_DIGIT(0))
    int hdr_mode;
    // M4_NUMBER_DESC("snr_mode", "s8", M4_RANGE(0, 2), "0", M4_DIGIT(0))
    int   snr_mode;

    // M4_ARRAY_DESC("time", "f32", M4_SIZE(1,3), M4_RANGE(0, 1024), "0.01", M4_DIGIT(6))
    float arTime[3];
    // M4_ARRAY_DESC("again", "f32", M4_SIZE(1,3), M4_RANGE(0, 204800), "1", M4_DIGIT(3))
    float arAGain[3];
    // M4_ARRAY_DESC("dgain", "f32", M4_SIZE(1,3), M4_RANGE(0, 204800), "1", M4_DIGIT(3))
    float arDGain[3];
    // M4_ARRAY_DESC("isp_dgain", "f32", M4_SIZE(1,3), M4_RANGE(0, 204800), "1", M4_DIGIT(3))
    float isp_dgain[3];
    // M4_NUMBER_DESC("blc_ob_predgain", "f32", M4_RANGE(0, 204800), "1", M4_DIGIT(3))
    float blc_ob_predgain;
    // M4_ARRAY_DESC("iso", "u32", M4_SIZE(1,3), M4_RANGE(0, 204800), "1", M4_DIGIT(0))
    int   arIso[3];

    // M4_NUMBER_DESC("isoLevelLow", "u8", M4_RANGE(0, 12), "0", M4_DIGIT(0))
    int isoLevelLow;
    // M4_NUMBER_DESC("isoLevelHig", "u8", M4_RANGE(0, 12), "0", M4_DIGIT(0))
    int isoLevelHig;

    // M4_NUMBER_DESC("rawWidth", "s32", M4_RANGE(0, 65535), "0", M4_DIGIT(0))
    int rawWidth;
    // M4_NUMBER_DESC("rawHeight", "s32", M4_RANGE(0, 65535), "0", M4_DIGIT(0))
    int rawHeight;

    // M4_NUMBER_DESC("bayertnr_en", "s8", M4_RANGE(0, 1), "0", M4_DIGIT(0))
    int bayertnr_en;

} Ayuvme_ExpInfo_V1_t;


typedef struct rk_aiq_yuvme_info_v1_s {
    // M4_ARRAY_TABLE_DESC("sync", "array_table_ui", "none", "1")
    rk_aiq_uapi_sync_t sync;
    // M4_NUMBER_DESC("iso", "u32", M4_RANGE(0, 204800), "50", M4_DIGIT(0), "0", "0")
    int iso;
    // M4_ARRAY_TABLE_DESC("expo_info", "normal_ui_style", "none", "0", "0")
    Ayuvme_ExpInfo_V1_t expo_info;
} rk_aiq_yuvme_info_v1_t;



RKAIQ_END_DECLARE
// clang-format on

#endif
