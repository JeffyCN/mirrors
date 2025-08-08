/*
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

#ifndef _RK_AIQ_PARAM_RGBIR10_H_
#define _RK_AIQ_PARAM_RGBIR10_H_

#define RGBIR_LUM2SCALE_TABLE_LEN 17

typedef struct {
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
    // M4_ARRAY_MARK_DESC("hw_rgbir_remosaic_lum2scale_idx", "u16", M4_SIZE(1,17),  M4_RANGE(0, 1024), "[800, 808, 816, 824, 832, 840, 848, 880, 912, 944, 976, 984, 992, 1000, 1008, 1016, 1024]", M4_DIGIT(0), M4_DYNAMIC(0), 0)
    unsigned short hw_rgbir_remosaic_lum2scale_idx[RGBIR_LUM2SCALE_TABLE_LEN];
    // M4_ARRAY_MARK_DESC("hw_rgbir_remosaic_lum2scale_val", "u16", M4_SIZE(1,17),  M4_RANGE(0, 256), "[256,253,245,232,215,195,173,87,31,8,1,1,1,0,0,0,0]", M4_DIGIT(2), M4_DYNAMIC(0), 0)
    unsigned short hw_rgbir_remosaic_lum2scale_val[RGBIR_LUM2SCALE_TABLE_LEN];
} rgbir_params_dyn_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(dyn),
        M4_TYPE(struct),
        M4_UI_MODULE(dynamic_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    rgbir_params_dyn_t dyn;
} rgbir_param_t;

#endif
