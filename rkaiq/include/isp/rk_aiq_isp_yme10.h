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

#ifndef _RK_AIQ_PARAM_YME10_H_
#define _RK_AIQ_PARAM_YME10_H_


typedef struct {
    // M4_BOOL_DESC("hw_yuvme_btnrMap_en", "1")
    bool hw_yuvme_btnrMap_en;
    // M4_BOOL_DESC("hw_yuvme_debug_mode", "0")
    bool hw_yuvme_debug_mode;
} yme_params_static_t;

typedef struct {
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
} yme_params_dyn_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sta),
        M4_TYPE(struct),
        M4_UI_MODULE(static_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO))  */
    yme_params_static_t sta;
    /* M4_GENERIC_DESC(
        M4_ALIAS(dyn),
        M4_TYPE(struct),
        M4_UI_MODULE(dynamic_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    yme_params_dyn_t dyn;
} yme_param_t;

#endif
