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

#ifndef _RK_AIQ_PARAM_HISTEQ23_H_
#define _RK_AIQ_PARAM_HISTEQ23_H_

#define HIST_SIGMA_IDX_NUM     15
#define HIST_SIGMA_LUT_NUM     17

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(MapTflt_invSigma),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,255),
        M4_DEFAULT(6),
        M4_DIGIT_EX(0f8b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The iir coefficient of histeq mapping curve.\n
        Freq of use: low))  */
    float sw_hist_MapTflt_invSigma;
    /* M4_GENERIC_DESC(
        M4_ALIAS(imgMap_mode),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The histeq mode, use Y gain when hw_hist_imgMap_mode == 0. Use RGB gain when
       hw_hist_imgMap_mode == 1.\n Freq of use: low))  */
    uint8_t hw_hist_imgMap_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(blocks_cols),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(4,10),
        M4_DEFAULT(4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Even number only. The horizontal num of the block for histogram statistics.\n
        Freq of use: low))  */
    uint8_t hw_histc_blocks_cols;
    /* M4_GENERIC_DESC(
        M4_ALIAS(blocks_rows),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(4,8),
        M4_DEFAULT(4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Even number only. The vertical num of the block for histogram statistics.\n
        Freq of use: low))  */
    uint8_t hw_histc_blocks_rows;
} histeq_params_static_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(mapUserSet),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0.18),
        M4_DIGIT_EX(2f6b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The user-config scale of histeq.\n Freq of use: high))  */
    float sw_hist_mapUserSet;
    /* M4_GENERIC_DESC(
        M4_ALIAS(noiseCount_offset),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(64),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The offset of noise curve.\n Freq of use: low))  */
    uint8_t hw_histc_noiseCount_offset;
    /* M4_GENERIC_DESC(
        M4_ALIAS(noiseCount_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,8),
        M4_DEFAULT(2),
        M4_DIGIT_EX(2f4b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The scale of noise curve.\n Freq of use: low))  */
    float sw_histc_noiseCount_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(countWgt_minLimit),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0.015),
        M4_DIGIT_EX(4f8b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The min limit of histogram statistics.\n Freq of use: low))  */
    float sw_histc_countWgt_minLimit;
    /* M4_GENERIC_DESC(
        M4_ALIAS(mapCount_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,4),
        M4_DEFAULT(0.09),
        M4_DIGIT_EX(2f8b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The auto scale of histeq.\n Freq of use: high))  */
    float sw_hist_mapCount_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(mapMerge_alpha),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0),
        M4_DIGIT_EX(2f8b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(In the fusion operation of sw_hist_mapCount_scale and sw_hist_mapUserSet, the
       fusion weight value of sw_hist_mapUserSet.\n Freq of use: high))  */
    float sw_hist_mapMerge_alpha;
    /* M4_GENERIC_DESC(
        M4_ALIAS(outputMerge_alpha),
        M4_TYPE(f32),
        M4_UI_MODULE(drc_curve),
        M4_SIZE_EX(1,17),
        M4_RANGE_EX(0.0,1.0),
        M4_DEFAULT([1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0]),
        M4_DATAX([0, 64, 128, 192, 256, 320, 384, 448, 512, 576, 640, 704, 768, 832, 896, 960, 1024]),
        M4_DIGIT_EX(2f7b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(In the fusion operation of the original image and the after-proccessing image, the
       fusion weight value of the after-proccessing image\n Freq of use: low))  */
    float sw_dehaze_outputMerge_alpha[17];
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
