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

#ifndef _RK_AIQ_PARAM_HISTEQ30_H_
#define _RK_AIQ_PARAM_HISTEQ30_H_

#define HIST_SIGMA_LUT_NUM     17

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_histCfg_zonesHorz_num),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(4,10),
        M4_DEFAULT(4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Even number only. The horizontal num of the block for histogram statistics.\n
        Freq of use: low))  */
    uint8_t hw_histCfg_zonesHorz_num;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_histCfg_zonesVert_num),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(4,8),
        M4_DEFAULT(4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Even number only. The vertical num of the block for histogram statistics.\n
        Freq of use: low))  */
    uint8_t hw_histCfg_zonesVert_num;
} histeq_stats_params_static_t;

typedef struct {
    /* M4_GENERIC_DESC(
    M4_ALIAS(statsHist),
    M4_TYPE(struct),
    M4_UI_MODULE(normal_ui_style),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(2),
    M4_NOTES(All histeq stats params in static.\n Freq of use: low))  */
    histeq_stats_params_static_t statsHist;
} histeq_params_static_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_histCfg_noiseCount_offset),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(64),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The offset of noise curve.\n Freq of use: low))  */
    uint8_t hw_histCfg_noiseCount_offset;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_histCfg_noiseCount_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,7),
        M4_DEFAULT(2),
        M4_DIGIT_EX(2f4b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The scale of noise curve.\n Freq of use: low))  */
    float sw_histCfg_noiseCount_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_histCfg_countWgt_minLimit),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0.015),
        M4_DIGIT_EX(4f8b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The min limit of histogram statistics.\n Freq of use: low))  */
    float sw_histCfg_countWgt_minLimit;
} histeq_stats_params_t;

typedef struct {
    /* M4_GENERIC_DESC(
    M4_ALIAS(sw_histT_mapUsrCfg_strg),
    M4_TYPE(f32),
    M4_SIZE_EX(1,1),
    M4_RANGE_EX(0,1),
    M4_DEFAULT(0.2),
    M4_DIGIT_EX(2f6b),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(0),
    M4_NOTES(The user-config scale of histeq.\n Freq of use: high))  */
    float sw_histT_mapUsrCfg_strg;
    /* M4_GENERIC_DESC(
    M4_ALIAS(sw_histT_mapStats_scale),
    M4_TYPE(f32),
    M4_SIZE_EX(1,1),
    M4_RANGE_EX(0,1),
    M4_DEFAULT(0.09),
    M4_DIGIT_EX(2f8b),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(0),
    M4_NOTES(The auto scale of histeq.\n Freq of use: high))  */
    float sw_histT_mapStats_scale;
    /* M4_GENERIC_DESC(
    M4_ALIAS(sw_histT_mapUsrCfgStrg_alpha),
    M4_TYPE(f32),
    M4_SIZE_EX(1,1),
    M4_RANGE_EX(0,1),
    M4_DEFAULT(0),
    M4_DIGIT_EX(2f8b),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(0),
    M4_NOTES(In the fusion operation of sw_hist_mapCount_scale and sw_hist_userMapCount_val, the
    fusion weight value of sw_hist_userMapCount_val.\n Freq of use: high))  */
    float sw_histT_mapUsrCfgStrg_alpha;
} histeq_mapHist_params_t;

typedef struct {
    /* M4_GENERIC_DESC(
    M4_ALIAS(sw_histT_iirSgm_scale),
    M4_TYPE(f32),
    M4_SIZE_EX(1,1),
    M4_RANGE_EX(1,255),
    M4_DEFAULT(6),
    M4_DIGIT_EX(0f8b),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(0),
    M4_NOTES(The iir sigma coefficient of histeq mapping curve.\n
        Freq of use: high))  */
    uint8_t sw_histT_iirSgm_scale;
    /* M4_GENERIC_DESC(
    M4_ALIAS(hw_histT_iirFrm_maxLimit),
    M4_TYPE(f32),
    M4_SIZE_EX(1,1),
    M4_RANGE_EX(1,15),
    M4_DEFAULT(8),
    M4_DIGIT_EX(0f8b),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(0),
    M4_NOTES(The frame num for iir stable.\n Freq of use: low))  */
    float hw_histT_iirFrm_maxLimit;
} histeq_mapHistIIR_params_t;

typedef enum {
    histeq_enhShpOutMix_mode = 0,
} histeq_idxSrc_mode_t;

typedef struct {
    /* M4_GENERIC_DESC(
    M4_ALIAS(hw_histT_idxSrc_mode),
    M4_TYPE(enum),
    M4_ENUM_DEF(histeq_idxSrc_mode_t),
    M4_DEFAULT(histeq_enhShpOutMix_mode),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(0),
    M4_NOTES(The mode of index source.\n Freq of use: low))	*/
    histeq_idxSrc_mode_t hw_histT_idxSrc_mode;
    /* M4_GENERIC_DESC(
    M4_ALIAS(hw_histT_shpOut_alpha),
    M4_TYPE(f32),
    M4_SIZE_EX(1,1),
    M4_RANGE_EX(0,1),
    M4_DEFAULT(0.5),
    M4_DIGIT_EX(2f8b),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(0),
    M4_NOTES(In the fusion operation of using enhance-filter result and using sharp output result,
    the fusion weight value of using sharp output result.\n Freq of use: high))  */
    float hw_histT_shpOut_alpha;
} histeq_mapHistIdx_params_t;

typedef enum {
    histeq_fusion_glbWgt_mode   = 0,
    histeq_fusion_luma2Wgt_mode = 1
} histeq_fusion_mode_t;

typedef struct {
    /* M4_GENERIC_DESC(
    M4_ALIAS(hw_histT_glbWgt_mode),
    M4_TYPE(enum),
    M4_ENUM_DEF(histeq_fusion_mode_t),
    M4_DEFAULT(histeq_fusion_glbWgt_mode),
    M4_GROUP_CTRL(glbWgt_mode_group),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(0),
    M4_NOTES(The mode of using fusion wgt.\n Freq of use: high))	*/
    histeq_fusion_mode_t hw_histT_glbWgt_mode;
    /* M4_GENERIC_DESC(
    M4_ALIAS(sw_histT_glbWgt_posVal),
    M4_TYPE(f32),
    M4_GROUP(glbWgt_mode_group:histeq_fusion_glbWgt_mode),
    M4_SIZE_EX(1,1),
    M4_RANGE_EX(0,4),
    M4_DEFAULT(0.5),
    M4_DIGIT_EX(2f8b),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(0),
    M4_NOTES(When gain is over 1, the global fusion weight value of histeq-out image in the fusion
    operation of histeq-out image and orginal image.\n Freq of use: high))  */
    float sw_histT_glbWgt_posVal;
    /* M4_GENERIC_DESC(
    M4_ALIAS(sw_histT_glbWgt_negVal),
    M4_TYPE(f32),
    M4_GROUP(glbWgt_mode_group:histeq_fusion_glbWgt_mode),
    M4_SIZE_EX(1,1),
    M4_RANGE_EX(0,4),
    M4_DEFAULT(0.5),
    M4_DIGIT_EX(2f8b),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(0),
    M4_NOTES(When gain is under 1, the global fusion weight value of histeq-out image in the fusion
    operation of histeq-out image and orginal image.\n Freq of use: high))  */
    float sw_histT_glbWgt_negVal;
    /* M4_GENERIC_DESC(
    M4_ALIAS(sw_histT_luma2Wgt_posVal),
    M4_GROUP(glbWgt_mode_group:histeq_fusion_luma2Wgt_mode),
    M4_TYPE(f32),
    M4_UI_MODULE(drc_curve),
    M4_SIZE_EX(1,17),
    M4_RANGE_EX(0.0,4.0),
    M4_DEFAULT([1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0]),
    M4_DATAX([0, 64, 128, 192, 256, 320, 384, 448, 512, 576, 640, 704, 768, 832, 896, 960, 1024]),
    M4_DIGIT_EX(2f6b),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(0),
    M4_NOTES(When gain is over 1, the local fusion weight value of histeq-out image in the fusion
    operation of histeq-out image and orginal image.\n Freq of use: high))  */
    float sw_histT_luma2Wgt_posVal[17];
    /* M4_GENERIC_DESC(
    M4_ALIAS(sw_histT_luma2Wgt_negVal),
    M4_GROUP(glbWgt_mode_group:histeq_fusion_luma2Wgt_mode),
    M4_TYPE(f32),
    M4_UI_MODULE(drc_curve),
    M4_SIZE_EX(1,17),
    M4_RANGE_EX(0.0,4.0),
    M4_DEFAULT([1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0]),
    M4_DATAX([0, 64, 128, 192, 256, 320, 384, 448, 512, 576, 640, 704, 768, 832, 896, 960, 1024]),
    M4_DIGIT_EX(2f6b),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(0),
    M4_NOTES(When gain is under 1, the local fusion weight value of histeq-out image in the fusion
    operation of histeq-out image and orginal image.\n Freq of use: high))  */
    float sw_histT_luma2Wgt_negVal[17];
} histeq_fusion_params_t;

typedef struct {
    /* M4_GENERIC_DESC(
    M4_ALIAS(sw_histT_saturate_scale),
    M4_TYPE(f32),
    M4_SIZE_EX(1,1),
    M4_RANGE_EX(0,8),
    M4_DEFAULT(1),
    M4_DIGIT_EX(2f5b),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(0),
    M4_NOTES(The saturate strength of histeq.\n Freq of use: high))  */
    float sw_histT_saturate_scale;
} histeq_chroma_params_t;

typedef struct {
    /* M4_GENERIC_DESC(
    M4_ALIAS(statsHist),
    M4_TYPE(struct),
    M4_UI_MODULE(normal_ui_style),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(2),
    M4_NOTES(All statsHist params corresponded with iso array.\n Freq of use: low))  */
    histeq_stats_params_t statsHist;
    /* M4_GENERIC_DESC(
    M4_ALIAS(mapHist),
    M4_TYPE(struct),
    M4_UI_MODULE(normal_ui_style),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(2),
    M4_NOTES(All mapHist params corresponded with iso array.\n Freq of use: high))  */
    histeq_mapHist_params_t mapHist;
    /* M4_GENERIC_DESC(
    M4_ALIAS(mapHistIIR),
    M4_TYPE(struct),
    M4_UI_MODULE(normal_ui_style),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(2),
    M4_NOTES(All mapHistIIR params corresponded with iso array.\n Freq of use: high))  */
    histeq_mapHistIIR_params_t mapHistIIR;
    /* M4_GENERIC_DESC(
    M4_ALIAS(mapHist_idx),
    M4_TYPE(struct),
    M4_UI_MODULE(normal_ui_style),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(2),
    M4_NOTES(All mapHist_idx params corresponded with iso array.\n Freq of use: high))  */
    histeq_mapHistIdx_params_t mapHist_idx;
    /* M4_GENERIC_DESC(
    M4_ALIAS(fusion),
    M4_TYPE(struct),
    M4_UI_MODULE(normal_ui_style),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(2),
    M4_NOTES(All fusion params corresponded with iso array.\n Freq of use: high))  */
    histeq_fusion_params_t fusion;
    /* M4_GENERIC_DESC(
    M4_ALIAS(chroma),
    M4_TYPE(struct),
    M4_UI_MODULE(normal_ui_style),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(2),
    M4_NOTES(All chroma params corresponded with iso array.\n Freq of use: high))  */
    histeq_chroma_params_t chroma;
} histeq_params_dyn_t;

typedef struct {
    /* M4_GENERIC_DESC(
    M4_ALIAS(sta),
    M4_TYPE(struct),
    M4_UI_MODULE(static_ui),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(1),
    M4_NOTES(The static params of histeq module))  */
    histeq_params_static_t sta;
    /* M4_GENERIC_DESC(
    M4_ALIAS(dyn),
    M4_TYPE(struct),
    M4_UI_MODULE(dynamic_ui),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(2),
    M4_NOTES(All dynamic params array corresponded with iso array))  */
    histeq_params_dyn_t dyn;
} histeq_param_t;

#endif
