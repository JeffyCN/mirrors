/*
 * debayer_head.h
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

#ifndef __CALIBDBV2_DEBAYER_HEAD_H__
#define __CALIBDBV2_DEBAYER_HEAD_H__

#include <rk_aiq_comm.h>

RKAIQ_BEGIN_DECLARE

typedef struct __debayer_array {
    // M4_ARRAY_DESC("debayer_filter1", "u32", M4_SIZE(1,9),  M4_RANGE(08, 65536), "[50,100,200,400,800,1600,3200,6400,12800]", M4_DIGIT(0), M4_DYNAMIC(0), 0)
    int ISO[9];
    // M4_ARRAY_DESC("sharp_strength", "u8", M4_SIZE(1,9),  M4_RANGE(0, 255), "[4,4,4,4,4,4,4,4,4]", M4_DIGIT(0), M4_DYNAMIC(0), 0)
    unsigned char sharp_strength[9];
    // M4_ARRAY_DESC("debayer_hf_offset", "u16", M4_SIZE(1,9),  M4_RANGE(0, 255), "[1,1,1,1,1,1,1,1,1]", M4_DIGIT(0), M4_DYNAMIC(0), 0)
    unsigned short debayer_hf_offset[9];
}Debayer_Array_t;

typedef struct __Debayer_Param {
    // M4_BOOL_DESC("debayer_en", "1", 0, M4_HIDE(1))
    bool debayer_en;
    // M4_ARRAY_DESC("debayer_filter1", "s8", M4_SIZE(1,5),  M4_RANGE(-128, 128), "[2,-6,0,6,-2]", M4_DIGIT(0), M4_DYNAMIC(0), 0)
    int debayer_filter1[5];
    // M4_ARRAY_DESC("debayer_filter2", "s8", M4_SIZE(1,5),  M4_RANGE(-128, 128), "[2,-4,4,-4,2]", M4_DIGIT(0), M4_DYNAMIC(0), 0)
    int debayer_filter2[5];
    // M4_NUMBER_DESC("debayer_gain_offset", "u8", M4_RANGE(0,128), "4", M4_DIGIT(0), 0)
    unsigned char debayer_gain_offset;
    // M4_NUMBER_DESC("debayer_offset", "u8", M4_RANGE(0,128), "1", M4_DIGIT(0), 0)
    unsigned char debayer_offset;
    // M4_BOOL_DESC("debayer_clip_en", "1", 0)
    bool debayer_clip_en;
    // M4_BOOL_DESC("debayer_filter_g_en", "1", 0)
    bool debayer_filter_g_en;
    // M4_BOOL_DESC("debayer_filter_c_en", "1", 0)
    bool debayer_filter_c_en;
    // M4_NUMBER_DESC("debayer_thed0", "u8", M4_RANGE(0,128), "3", M4_DIGIT(0), 0)
    unsigned char debayer_thed0;
    // M4_NUMBER_DESC("debayer_thed1", "u8", M4_RANGE(0,128), "6", M4_DIGIT(0), 0)
    unsigned char debayer_thed1;
    // M4_NUMBER_DESC("debayer_dist_scale", "u8", M4_RANGE(0,128), "8", M4_DIGIT(0), 0)
    unsigned char debayer_dist_scale;
    // M4_NUMBER_DESC("debayer_cnr_strength", "u8", M4_RANGE(0,128), "5", M4_DIGIT(0), 0)
    unsigned char debayer_cnr_strength;
    // M4_NUMBER_DESC("debayer_shift_num", "u8", M4_RANGE(0,128), "2", M4_DIGIT(0), 0)
    unsigned char debayer_shift_num;
    // M4_ARRAY_TABLE_DESC("ISO_Params", "array_table_ui", "default")
    Debayer_Array_t array;
}Debayer_Param_t;

typedef struct __debayer {
    // M4_STRUCT_DESC("DebayerTuningPara", "normal_ui_style")
    Debayer_Param_t param;
} CalibDbV2_Debayer_t;

RKAIQ_END_DECLARE

#endif
