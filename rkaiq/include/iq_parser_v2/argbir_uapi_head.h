/*
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

#ifndef __ARGBIR_UAPI_HEAD_H__
#define __ARGBIR_UAPI_HEAD_H__

#include "argbir_head.h"

// rgbir v10
typedef struct mRgbirDataV10_s {
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
} mRgbirDataV10_t;

typedef struct mRgbirAttr_v10_s {
    // M4_BOOL_DESC("hw_rgbir_remosaic_en", "0")
    bool hw_rgbir_remosaic_en;
    // M4_STRUCT_DESC("RgbirData", "normal_ui_style")
    mRgbirDataV10_t RgbirData;
} mRgbirAttr_v10_t;

typedef struct RgbirInfoV10_s {
    // M4_NUMBER_DESC("iso", "f32", M4_RANGE(50,1000000000), "0", M4_DIGIT(4))
    float iso;
    // M4_STRUCT_DESC("ValidParams", "normal_ui_style")
    mRgbirAttr_v10_t ValidParams;
} RgbirInfoV10_t;

#endif /*__ARGBIR_UAPI_HEAD_H__*/
