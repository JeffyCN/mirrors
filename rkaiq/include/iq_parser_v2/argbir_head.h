/*
 * argbir_head.h
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

#ifndef __CALIBDBV2_ARGBIR_HEADER_H__
#define __CALIBDBV2_ARGBIR_HEADER_H__

#define ARGBIR_ISO_STEP_MAX        13
#define ARGBIR_LUM2SCALE_TABLE_LEN 17

// rgbir v10
typedef struct Rgbir_lum2scale_table_v10_s {
    // M4_ARRAY_MARK_DESC("hw_rgbir_remosaic_lum2scale_idx", "u16", M4_SIZE(1,17),  M4_RANGE(0, 1024), "[800, 808, 816, 824, 832, 840, 848, 880, 912, 944, 976, 984, 992, 1000, 1008, 1016, 1024]", M4_DIGIT(0), M4_DYNAMIC(0), 0)
    unsigned short hw_rgbir_remosaic_lum2scale_idx[ARGBIR_LUM2SCALE_TABLE_LEN];
    // M4_ARRAY_MARK_DESC("hw_rgbir_remosaic_lum2scale_val", "u16", M4_SIZE(1,17),  M4_RANGE(0, 256), "[256,253,245,232,215,195,173,87,31,8,1,1,1,0,0,0,0]", M4_DIGIT(2), M4_DYNAMIC(0), 0)
    unsigned short hw_rgbir_remosaic_lum2scale_val[ARGBIR_LUM2SCALE_TABLE_LEN];
} Rgbir_lum2scale_table_v10_t;

typedef struct RgbirDataV10_s {
    // M4_NUMBER_MARK_DESC("iso", "f32", M4_RANGE(50,1000000000), "0", M4_DIGIT(2), "index1")
    float iso;
    // M4_NUMBER_DESC("hw_rgbir_remosaic_edge_aware_coef", "u32", M4_RANGE(1,16383), "4", M4_DIGIT(0))
    unsigned short hw_rgbir_remosaic_edge_coef;
    // M4_NUMBER_DESC("hw_rgbir_remosaic_ir_blk_level", "f32", M4_RANGE(0,4095), "0", M4_DIGIT(0))
    float hw_rgbir_remosaic_ir_blk_level;
    // M4_NUMBER_DESC("sw_rgbir_remosaic_r_scale_coef", "f32", M4_RANGE(0,2), "1.7", M4_DIGIT(2))
    float sw_rgbir_remosaic_r_scale_coef;
    // M4_NUMBER_DESC("sw_rgbir_remosaic_gr_scale_coef", "f32", M4_RANGE(0,2), "1.0", M4_DIGIT(2))
    float sw_rgbir_remosaic_gr_scale_coef;
    // M4_NUMBER_DESC("sw_rgbir_remosaic_gb_scale_coef", "f32", M4_RANGE(0,2), "1.0", M4_DIGIT(2))
    float sw_rgbir_remosaic_gb_scale_coef;
    // M4_NUMBER_DESC("sw_rgbir_remosaic_b_scale_coef", "f32", M4_RANGE(0,2), "1.0", M4_DIGIT(2))
    float sw_rgbir_remosaic_b_scale_coef;
    // M4_ARRAY_TABLE_DESC("hw_Rgbir_lum2scale_table", "array_table_ui","none")
    Rgbir_lum2scale_table_v10_t hw_Rgbir_lum2scale_table;
} RgbirDataV10_t;

typedef struct CalibDbV2_rgbir_v10_s {
    // M4_BOOL_DESC("hw_rgbir_remosaic_en", "0")
    bool hw_rgbir_remosaic_en;
    // M4_STRUCT_LIST_DESC("RgbirData", M4_SIZE(1,13), "normal_ui_style")
    RgbirDataV10_t RgbirData[ARGBIR_ISO_STEP_MAX];
} CalibDbV2_rgbir_v10_t;

typedef struct CalibDbV2_argbir_v10_s {
    // M4_STRUCT_DESC("RgbirTuningPara", "normal_ui_style")
    CalibDbV2_rgbir_v10_t RgbirTuningPara;
} CalibDbV2_argbir_v10_t;

#endif
