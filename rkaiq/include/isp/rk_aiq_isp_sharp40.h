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

#ifndef _RK_AIQ_PARAM_SHARP40_H_
#define _RK_AIQ_PARAM_SHARP40_H_

#define SHARP_ISO_STEP_MAX                  13
#define SHARP_FILTPIXGAINCURVE_SEGMENT_MAX     14
#define SHARP_RADIDISTCURVE_SEGMENT_MAX        22
#define SHARP_TXETCURVE_SEGMENT_MAX        17

typedef enum shp_radiusStep_mode_e {
    // @reg: hw_shp_radiusStep_mode == 0
    shp_step_128_mode = 0,
    // @reg: hw_shp_radiusStep_mode == 1
    shp_step_256_mode = 1,
} shp_radiusStep_mode_t;

typedef struct shp_radiDist_static_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_shp_center_x),
        M4_TYPE(s16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(-1000,1000),
        M4_DEFAULT(0),
        M4_DIGIT_EX(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES( The x-coordinates of the optical center in the image\n
        (0,0) is the img center. (-1000,-1000) is the img left top corner. (1000,1000) is the img right bottom corner\n
        Freq of use: high))  */
    // @reg: hw_shp_center_x
    // @para: Center_Mode
    int16_t hw_shpCfg_opticCenter_x;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_shp_center_y),
        M4_TYPE(s16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(-1000,1000),
        M4_DEFAULT(0),
        M4_DIGIT_EX(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES( The y-coordinates of the optical center in the image\n
        (0,0) is the img center. (-1000,-1000) is the img left top corner. (1000,1000) is the img right bottom corner\n
        Freq of use: high))  */
    // @reg: hw_shp_center_y
    // @para: Center_Mode
    int16_t hw_shpCfg_opticCenter_y;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_shpT_radiusStep_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(shp_radiusStep_mode_t),
        M4_DEFAULT(shp_step_128_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES( Reference enum types.\n
        Freq of use: high))  */
    shp_radiusStep_mode_t hw_shpT_radiusStep_mode;
} shp_radiDist_static_t;

typedef enum shp_dbgOutMux_mode_e {
    // @reg: hw_shp_debug_mode == 1
    shp_texEst_mode = 1,
    // @reg: hw_shp_debug_mode == 2
    shp_detailLocShpStrg_mode = 2,
    // @reg: hw_shp_debug_mode == 3
    shp_edgeShpStrg_mode = 3,
    // @reg: hw_shp_debug_mode == 4
    shp_contrastDetailPosStrg_mode = 4,
    // @reg: hw_shp_debug_mode == 5
    shp_detailPosLimit_mode = 5,
} shp_dbgOutMux_mode_t;

typedef struct shp_debug_static_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_shpT_dbgOut_en),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_GROUP_CTRL(dbgOut_en_group),
        M4_NOTES(The enable bit for debugging data to replace pixel data output\n
        Freq of use: high))  */
    // reg: hw_shp_debug_mode == 0  which means disable debug mode
    bool sw_shpT_dbgOut_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_shpT_dbgOut_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(shp_dbgOutMux_mode_t),
        M4_DEFAULT(shp_texEst_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(dbgOut_en_group),
        M4_NOTES( Reference enum types.\n
        Freq of use: high))  */
    shp_dbgOutMux_mode_t hw_shpT_dbgOut_mode;
} shp_debug_static_t;

typedef struct shp_detailLP_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(detail_lp_en),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_GROUP_CTRL(shpCfg_lp_en_group),
        M4_NOTES(IIR frame low freq channel averge low power mode enable.\n
        Freq of use: low))  */
    // reg: sw_detail_lp_en
    bool hw_shpCfg_lp_en;
} shp_detailLP_t;

typedef struct shp_cfgLP_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(preSpnrLP),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.\n
        Freq of use: low))  */
    shp_detailLP_t detailShpLP;
} shp_cfgLP_t;

typedef struct shp_lumaLutIdx_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(luma_dx),
        M4_TYPE(f32),
        M4_SIZE_EX(1,8),
        M4_RANGE_EX(0,1024),
        M4_DIGIT_EX(0),
        M4_FP_EX(0,1,7),
        M4_DEFAULT([0, 64, 128, 256, 384, 640, 896, 1024]),
        M4_HIDE_EX(0),
        M4_UI_MODULE(curve),
        M4_DATAX([0, 1, 2, 3, 4, 5, 6, 7]),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The brightness weight0 mapped by lo_nr, the x-axis is hw_shp_luma2Table_idx. \n
        Freq of use: low))  */
    // @reg: hw_shp_luma_dx0~7
    float hw_shpT_lumaLutIdx_val[8];
} shp_lumaLutIdx_t;

typedef struct sharp_params_static_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(shpScl_radiDist),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    shp_radiDist_static_t locShpStrg_radiDist;
    /* M4_GENERIC_DESC(
        M4_ALIAS(shpScl_radiDist),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    shp_lumaLutIdx_t lumaLutCfg;
    /* M4_GENERIC_DESC(
        M4_ALIAS(shpScl_radiDist),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    shp_debug_static_t debug;
    /* M4_GENERIC_DESC(
        M4_ALIAS(lowPower),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    shp_cfgLP_t lowPowerCfg;
} sharp_params_static_t;

typedef struct shp_texRegionClsf_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(flat_maxLimit),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1023),
        M4_DEFAULT(100),
        M4_DIGIT_EX(3),
        M4_FP_EX(0,10,0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(the upper limit value of flat area.\n
        Freq of use: high))  */
    // @reg: hw_shp_flat_maxLimit
    uint16_t hw_shpT_flatRegion_maxThred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(edge_minLimit),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1023),
        M4_DEFAULT(300),
        M4_DIGIT_EX(3),
        M4_FP_EX(0,10,0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(the lower limit value of edge area.\n
        The hw_shp_edge_minLimit must be bigger than hw_shp_flat_maxLimit.\n
        Freq of use: high))  */
    // @reg: hw_shp_edge_minLimit
    uint16_t hw_shpT_edgeRegion_minThred;
} shp_texRegionClsf_t;

typedef enum shp_locSgmStrg_mode_e {
    // @reg: hw_shp_localGain_bypass == 0
    // @note: "Mixed mode of local and global input pix sigma"
    shp_locGlbSgmStrgMix_mode = 0,
    // @reg: hw_shp_localGain_bypass == 1
    // @note: "Only global input pix sigma"
    shp_glbSgmStrgOnly_mode = 1
} shp_locSgmStrg_mode_t;

typedef enum shp_locSgmStrg2Mot_mode_e {
    // @reg: hw_shp_gainWgt_mode == 0
    // @note: "use noise balance curve"
    // @para: MotionWgt_mode = 0
    shp_toMotionStrg1_mode = 0,
    // @reg: hw_shp_gainWgt_mode == 1
    // @note: "use manually configured curve"
    // @para: MotionWgt_mode = 1
    shp_toMotionStrg2_mode = 1
} shp_locSgmStrg2Mot_mode_t;

typedef struct shp_motionStrg_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_shp_localGain_bypass),
        M4_TYPE(enum),
        M4_ENUM_DEF(shp_locSgmStrg_mode_t),
        M4_DEFAULT(shp_locGlbSgmStrgMix_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP_CTRL(localSgmStrg_mode_group),
        M4_NOTES(The mode of sharp input pix sigma. Reference enum types.\n
        Freq of use: low))  */
    // @reg: hw_shp_localGain_bypass
    // @para: exgain_bypass
    shp_locSgmStrg_mode_t hw_shpT_locSgmStrg_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_shp_local_gainscale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(2),
        M4_FP_EX(0,1,7),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_GROUP(localSgmStrg_mode_group:shp_locGlbSgmStrgMix_mode),
        M4_NOTES(The scaling factor of the local input pix sigma.\n
        Higher the value, the higher the local input pix sigma value.\n
        Freq of use: low))  */
    // @reg:  hw_sharp_localGain_scale
    // @para: local_gainscale
    float hw_shpT_localSgmStrg_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_shp_global_gain),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,64.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(2),
        M4_FP_EX(0,6,4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The value of the global input pix sigma.\n
        Higher the value, the higher the global input pix sigma value.\n
        Freq of use: low))  */
    // @reg: hw_sharp_global_gain
    // @para: global_gain
    float hw_shpT_glbSgmStrg_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_cnr_global_gain_alpha),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1.0),
        M4_DEFAULT(0.0),
        M4_DIGIT_EX(2),
        M4_FP_EX(0,1,3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_GROUP(localSgmStrg_mode_group:shp_locGlbSgmStrgMix_mode),
        M4_NOTES(The wgt of the global input pix sigma is used in the fusion operation with the local input pix sigma.\n
        The higher the value, the wgt of bifilted pixel is higher.\n
        Freq of use: low))  */
    // @reg: hw_sharp_gainMerge_alpha
    // @para: global_gain_alpha
    float hw_shpT_glbSgmStrg_alpha;
    /* M4_GENERIC_DESC(
        M4_ALIAS(gainWgt_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(shp_locSgmStrg2Mot_mode_t),
        M4_DEFAULT(shp_toMotionStrg2_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP_CTRL(locSgmStrg2Mot_mode_group),
        M4_NOTES(The mode of gain weight generation method. Reference enum types.\n
        Freq of use: low))  */
    // @reg: hw_shp_gainWgt_mode
    shp_locSgmStrg2Mot_mode_t hw_shpT_locSgmStrg2Mot_mode;
} shp_motionStrg_dyn_t;

typedef struct shp_lumaShpStrg_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(luma_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_GROUP_CTRL(luma_en_group),
        M4_NOTES(Enable the miNr filter for lo freq noise.Turn on by setting this bit to 1.
        Freq of use: high))  */
    bool sw_shpT_luma_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(luma2strg_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,8),
        M4_RANGE_EX(0.0,1.0),
        M4_DIGIT_EX(3),
        M4_FP_EX(0,1,7),
        M4_DEFAULT([1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0]),
        M4_HIDE_EX(0),
        M4_UI_MODULE(curve),
        M4_DATAX([0, 64, 128, 256, 384, 640, 896, 1024]),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(luma_en_group),
        M4_NOTES(The brightness weight0 mapped by lo_nr, the x-axis is hw_shp_luma2Table_idx. \n
        Freq of use: high))  */
    // @reg: hw_shp_luma2strg_val0~7
    float hw_shpT_luma2ShpStrg_val[8];
} shp_lumaShpStrg_dyn_t;

typedef enum shp_lumaShpStrgEn_mode_e {
    shp_lumaShpStrgEn_mode = 0
} shp_lumaShpStrgEn_mode_t;

typedef struct shp_lumaShpStrgEn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(locShpStrg_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(shp_lumaShpStrgEn_mode_t),
        M4_DEFAULT(shp_lumaShpStrgEn_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP_CTRL(motionWgt_mode_group),
        M4_NOTES(The mode of gain weight generation method. Reference enum types.\n
        Freq of use: low))  */
    shp_lumaShpStrgEn_mode_t sw_shpT_locShpStrg_mode;
} shp_lumaShpStrgEn_t;

typedef struct shp_radiDistShpWgt_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(radiDist_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_GROUP_CTRL(radiDist_en_group),
        M4_NOTES(Enable the miNr filter for lo freq noise.Turn on by setting this bit to 1.
        Freq of use: high))  */
    bool sw_shpT_radiDist_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(distance2strg),
        M4_TYPE(f32),
        M4_SIZE_EX(1,11),
        M4_RANGE_EX(0.0,1.0),
        M4_DIGIT_EX(3),
        M4_FP_EX(0,1,7),
        M4_DEFAULT([1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0]),
        M4_HIDE_EX(0),
        M4_UI_MODULE(curve),
        M4_DATAX([0, 362, 627, 958, 1201, 1402, 1736, 2016, 2261, 2482, 2685]),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(radiDist_en_group),
        M4_NOTES(Radial distance weight based on the distance from the center point. \n
        step=128,radia = [0, 362, 627, 958, 1201, 1402, 1736, 2016, 2261, 2482, 2685].\n
        step=256,radial = [0, 724, 1254, 1916, 2401, 2804, 3473, 4031, 4522, 4964, 5370].\n
        Freq of use: high))  */
    // @reg: hw_shp_distance2strg_val0~10
    float hw_shpT_radiDist2ShpStrg_val[11];
} shp_radiDistShpStrg_dyn_t;

typedef enum shp_radiDistShpStrgEn_mode_s {
    shp_radiDistShpStrgEn_mode = 0
} shp_radiDistShpStrgEn_mode_t;

typedef struct shp_radiDistShpStrgEn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(locShpStrg_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(shp_radiDistShpStrgEn_mode_t),
        M4_DEFAULT(shp_radiDistShpStrgEn_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP_CTRL(motionWgt_mode_group),
        M4_NOTES(The mode of gain weight generation method. Reference enum types.\n
        Freq of use: low))  */
    shp_radiDistShpStrgEn_mode_t sw_shpT_locShpStrg_mode;
} shp_radiDistShpStrgEn_t;

typedef struct shp_hueShpStrg_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hue_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_GROUP_CTRL(hue_en_group),
        M4_NOTES(Enable the miNr filter for lo freq noise.Turn on by setting this bit to 1.
        Freq of use: high))  */
    bool sw_shpT_hue_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(luma2strg_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,9),
        M4_RANGE_EX(0.0,2.0),
        M4_DIGIT_EX(3),
        M4_FP_EX(0,1,9),
        M4_DEFAULT([1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0]),
        M4_HIDE_EX(0),
        M4_UI_MODULE(curve),
        M4_GROUP(hue_en_group),
        M4_DATAX([0, 128, 256, 384, 512, 640, 768, 896, 1024]),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The brightness weight0 mapped by lo_nr, the x-axis is hw_shp_luma2Table_idx. \n
        Freq of use: high))  */
    // @reg: hw_shp_hue2strg_val0~8
    float hw_shpT_hue2ShpStrg_val[9];
} shp_hueShpStrg_dyn_t;

typedef enum shp_hueShpStrgEn_mode_s {
    shp_hueShpStrgEn_mode = 0
} shp_hueShpStrgEn_mode_t;

typedef struct shp_hueShpStrgEn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(locShpStrg_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(shp_hueShpStrgEn_mode_t),
        M4_DEFAULT(shp_hueShpStrgEn_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP_CTRL(motionWgt_mode_group),
        M4_NOTES(The mode of gain weight generation method. Reference enum types.\n
        Freq of use: low))  */
    shp_hueShpStrgEn_mode_t sw_shpT_locShpStrg_mode;
} shp_hueShpStrgEn_t;

typedef struct shp_locShpStrg_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(texDct_dyn),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    shp_texRegionClsf_t texRegion_clsfBaseTex;
    /* M4_GENERIC_DESC(
       M4_ALIAS(localGain),
       M4_TYPE(struct),
       M4_UI_MODULE(normal_ui_style),
       M4_HIDE_EX(0),
       M4_RO(0),
       M4_ORDER(2),
       M4_NOTES(TODO))  */
    shp_motionStrg_dyn_t motionStrg;
    /* M4_GENERIC_DESC(
        M4_ALIAS(luma),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    shp_lumaShpStrg_dyn_t luma;
    /* M4_GENERIC_DESC(
        M4_ALIAS(radiDist),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    shp_radiDistShpStrg_dyn_t radiDist;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hue),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    shp_hueShpStrg_dyn_t hue;
} shp_locShpStrg_dyn_t;

typedef struct shp_texRegionShpStrg_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(tex2detailStrg_val0),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1.0),
        M4_DEFAULT(0.4),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The local weight of the detail layer generated based on texture weight,\n
        with an x-axis of [0, hw_shp_flat_maxLimit, hw_shp_edge_minLimit, 1024]. \n
        Freq of use: high))  */
    // @reg: hw_shp_tex2detailStrg_val0
    float hw_shpT_flatRegionL_strg;
    /* M4_GENERIC_DESC(
        M4_ALIAS(tex2detailStrg_val1),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1.0),
        M4_DEFAULT(0.6),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The local weight of the detail layer generated based on texture weight,\n
        with an x-axis of [0, hw_shp_flat_maxLimit, hw_shp_edge_minLimit, 1024]. \n
        Freq of use: high))  */
    // @reg: hw_shp_tex2detailStrg_val1
    float hw_shpT_flatRegionR_strg;
    /* M4_GENERIC_DESC(
        M4_ALIAS(tex2detailStrg_val2),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1.0),
        M4_DEFAULT(0.8),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The local weight of the detail layer generated based on texture weight,\n
        with an x-axis of [0, hw_shp_flat_maxLimit, hw_shp_edge_minLimit, 1024]. \n
        Freq of use: high))  */
    // @reg: hw_shp_tex2detailStrg_val2
    float hw_shpT_edgeRegionL_strg;
    /* M4_GENERIC_DESC(
        M4_ALIAS(tex2detailStrg_val3),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The local weight of the detail layer generated based on texture weight,\n
        with an x-axis of [0, hw_shp_flat_maxLimit, hw_shp_edge_minLimit, 1024]. \n
        Freq of use: high))  */
    // @reg: hw_shp_tex2detailStrg_val3
    float hw_shpT_edgeRegionR_strg;
} shp_texRegionShpStrg_t;

typedef struct shp_motionStrg1_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_shp_detailStaticRegion_thred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,64.0),
        M4_DEFAULT(0.2),
        M4_DIGIT_EX(2),
        M4_FP_EX(0,6,4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The max static region thred of the detail gain.\n
        Freq of use: low))  */
    float sw_shpT_locSgmStrgStat_maxThred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_shp_detailMotionRegion_thred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,64.0),
        M4_DEFAULT(0.5),
        M4_DIGIT_EX(2),
        M4_FP_EX(0,6,4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The min motion region thred of the detail gain.\n
        Freq of use: low))  */
    float sw_shpT_locSgmStrgMot_minThred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(detailGain_maxLimit),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,2.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(2),
        M4_FP_EX(0,1,7),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(the upper limit of detail gain weight.\n
        The hw_shp_detailGain_maxLimit must be bigger than hw_shp_detailGain_minLimit.\n
        Freq of use: low))  */
    // para: detailMotionWgt_maxLimit, edgeMotionWgt_maxLimit, hifreqMotionWgt_maxLimit
    float hw_shpT_statRegionShp_strg;
    /* M4_GENERIC_DESC(
        M4_ALIAS(detailGain_minLimit),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,2.0),
        M4_DEFAULT(0.0),
        M4_DIGIT_EX(2),
        M4_FP_EX(0,1,7),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(the uppper limit of detail gain weight.\n
        The hw_shp_detailGain_maxLimit must be bigger than hw_shp_detailGain_minLimit.\n
        Freq of use: low))  */
    // para: detailMotionWgt_minLimit, edgeMotionWgt_minLimit, hifreqMotionWgt_minLimit
    float hw_shpT_motRegionShp_strg;
} shp_motionStrg1_t;

typedef struct shp_motionStrg2_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(detailGain_sigma),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1.0),
        M4_DEFAULT(0.25),
        M4_DIGIT_EX(2),
        M4_FP_EX(0,1,7),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(the sigma of detail gain weight.\n
        Freq of use: high))  */
    //reg: hw_sharp_edgeGain_sigma
    float hw_shpT_motionStrg_sigma;
} shp_motionStrg2_t;

typedef enum shp_filtCfg_mode_e {
    // @note: The filter coefficients for configuring filters are generated by inputting  filter strength into the formula
    // @para: kernel_sigma_enable == 1
    shp_cfgByFiltStrg_mode = 0,
    // @note: The filter coefficients for configuring filters are directly input.
    // @para: kernel_sigma_enable == 0
    shp_cfgByFiltCoeff_mode = 1
} shp_filtCfg_mode_t;

typedef struct shp_sigmaCurve_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(val),
        M4_TYPE(u16),
        M4_UI_PARAM(data_y),
        M4_SIZE_EX(1,8),
        M4_RANGE_EX(1,1023),
        M4_DEFAULT([32, 40, 48, 56, 64, 56, 48, 40]),
        M4_HIDE_EX(0),
        M4_UI_MODULE(curve),
        M4_DATAX([0, 64, 128, 256, 384, 640, 896, 1024]),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO.\n
        Freq of use: low))  */
    uint16_t val[8];
} shp_sigmaCurve_t;

typedef enum shp_edge_filtRadius_mode_e {
    // @note: radius = 1
    shp_edgeFilt3x3_mode = 1,
    // @note: radius = 2
    shp_edgeFilt5x5_mode = 2,
    // @note: radius = 3
    shp_edgeFilt7x7_mode = 3
} shp_edge_filtRadius_mode_t;

typedef struct shp_edge_glbShpStrg_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(posDetail_strg),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,16.0),
        M4_DEFAULT(8),
        M4_DIGIT_EX(2),
        M4_FP_EX(0,4,2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(positive detail global strength.\n
        Freq of use: high))  */
    // @reg: hw_shp_posEdge_strg
    // @para: posEdge_strg
    float hw_shpT_edgePos_strg;
    /* M4_GENERIC_DESC(
        M4_ALIAS(posDetail_strg),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,16.0),
        M4_DEFAULT(8),
        M4_DIGIT_EX(2),
        M4_FP_EX(0,4,2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(negative detail global strength.\n
        Freq of use: high))  */
    // @reg: hw_shp_negEdge_strg
    // @para: negEdge_strg
    float hw_shpT_edgeNeg_strg;
} shp_edge_glbShpStrg_t;

typedef struct shp_edgeExtra_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(edgeLpf_radius),
        M4_TYPE(enum),
        M4_ENUM_DEF(shp_edge_filtRadius_mode_t),
        M4_DEFAULT(shp_edgeFilt7x7_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The radius of edge low pass filter kernel.\n
        Reference enum types.\n
        Freq of use: high))  */
    shp_edge_filtRadius_mode_t sw_shpT_filtRadius_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(filtCfg_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(shp_filtCfg_mode_t),
        M4_DEFAULT(shp_cfgByFiltStrg_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP_CTRL(edgeLpf_filtCfg_mode_group),
        M4_NOTES(The config mode of lpf used for hi-freq pre filtering processing of input pixels of the sharp module.\n
        Reference enum types.\n
        Freq of use: high))  */
    shp_filtCfg_mode_t sw_shpT_filtCfg_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(edgeLpf_rsigma),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,256.0),
        M4_DEFAULT(2.0),
        M4_DIGIT_EX(2),
        M4_FP_EX(0,7,8),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_GROUP(edgeLpf_filtCfg_mode_group:shp_cfgByFiltStrg_mode),
        M4_NOTES(The spatial wgt of pre bifilter is operator from the strength value. Only valid on shp_cfgByFiltStrg_mode.\n
        Higher the value, the higher spatial denoise strength.\n
        Freq of use: high))  */
    // @reg: hw_shp_edgeLpf_coeff0~hw_shp_edgeLpf_coeff9
    float sw_shpT_filtSpatial_strg;
    /* M4_GENERIC_DESC(
        M4_ALIAS(edgeLpf_coeff),
        M4_TYPE(f32),
        M4_SIZE_EX(1,10),
        M4_RANGE_EX(0,1.0),
        M4_DEFAULT([0.0467, 0.0412, 0.0364, 0.0283, 0.0250, 0.0172, 0.0152, 0.0134, 0.0092, 0.0049]),
        M4_DIGIT_EX(3),
        M4_FP_EX(0,1,7),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_GROUP(edgeLpf_filtCfg_mode_group:shp_cfgByFiltCoeff_mode),
        M4_NOTES(The spatial filter kernel of bifilter . Only valid on shp_cfgByFiltCoeff_mode.\n
        coeff[0] + 4*coeff[1] + 4*coeff[2] == 1.0 .\n
        Freq of use: high))  */
    // @reg: hw_shp_edgeLpf_coeff0~hw_shp_edgeLpf_coeff9
    float hw_shpT_filtSpatial_wgt[10];
} shp_edgeExtra_t;

typedef enum shp_curveCfg_mode_e {
    shp_cfgCurveDirect_mode    = 0,
    shp_cfgCurveCtrlCoeff_mode = 1
} shp_curveCfg_mode_t;

typedef enum shp_maxMinFlt_mode_e {
    // 5x5 window of hi nr filter
    // reg: hw_shp_maxminFlt_mode == 0
    shp_maxMinFiltRadius5_mode = 0,
    // 7x7 window of hi nr filter
    // reg: hw_shp_maxminFlt_mode == 1
    shp_maxMinFiltRadius7_mode = 1
} shp_maxMinFlt_mode_t;

typedef struct shp_edgeStrgCurveCtrl_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(edgeWgtCurve_power),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,15.0),
        M4_DEFAULT(6.0),
        M4_DIGIT_EX(2),
        M4_FP_EX(0,10,2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The edgewgtCurve power value. Only valid on shp_formuleCurve_mode.\n
        Formule: val = MIN(1023, edgeWgt_minLimit + ROUND_F(1024 * (1 - pow(1 - pow(i / 16 , power), power)))).\n
        Freq of use: high))  */
    //reg: hw_shp_edgeWgt_val0~16
    //para: edgeWgtCurve_power
    float sw_shpT_curvePower_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(edgeWgt_minLimit),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1023.0),
        M4_DEFAULT(0.0),
        M4_DIGIT_EX(2),
        M4_FP_EX(0,10,0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(the lower limit for edge weight. Only valid on shp_formuleCurve_mode\n
        Formule: val = MIN(1023, edgeWgt_minLimit + ROUND_F(1024 * (1 - pow(1 - pow(i / 16 , power), power)))).\n
        Freq of use: high))  */
    //reg: hw_shp_edgeWgt_val0~16
    //para: edgeWgtCurve_power
    float sw_shpT_edgeStrg_minLimit;
} shp_edgeStrgCurveCtrl_t;

typedef struct shp_edgeShpStrg_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(filtCfg_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(shp_curveCfg_mode_t),
        M4_DEFAULT(shp_cfgCurveCtrlCoeff_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP_CTRL(edgeWgtCurve_mode_group),
        M4_NOTES(The config mode of lpf used for hi-freq pre filtering processing of input pixels of the sharp module.\n
        Reference enum types.\n
        Freq of use: low))  */
    shp_curveCfg_mode_t sw_shpT_edgeStrgCurve_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(posEdgeWgt_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1023.0),
        M4_DEFAULT(10.0),
        M4_DIGIT_EX(2),
        M4_FP_EX(0,10,2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The scaling coefficient for the estimated edge probability, as the x-value for the positive edge signal weight.\n
        Freq of use: high))  */
    // @reg: hw_shp_posEdgeWgt_scale
    float hw_shpT_edgePosIdx_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(negEdgeWgt_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1023.0),
        M4_DEFAULT(12.0),
        M4_DIGIT_EX(2),
        M4_FP_EX(0,10,2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The scaling coefficient for the estimated edge probability, as the x-value for the negative edge signal weight.\n
        Freq of use: high))  */
    // @reg: hw_shp_negEdgeWgt_scale
    float hw_shpT_edgeNegIdx_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(edgeWgt_val_curve),
        M4_TYPE(struct),
        M4_UI_MODULE(sharp_edgeStrg_curve),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_DATAX([0, 64, 128, 192, 256, 320, 384, 448, 512, 576, 640, 704, 768, 832, 896, 960, 1024]),
        M4_GROUP(edgeWgtCurve_mode_group:shp_cfgCurveCtrlCoeff_mode),
        M4_NOTES(TODO))  */
    shp_edgeStrgCurveCtrl_t edgeStrgCurveCtrl;
    /* M4_GENERIC_DESC(
        M4_ALIAS(edgeWgt_val),
        M4_TYPE(u16),
        M4_SIZE_EX(1,17),
        M4_RANGE_EX(0,1023),
        M4_DEFAULT([2,4,6,8,10,12,17,42,92,180,316,500,708,890,995,1023,1023]),
        M4_DIGIT_EX(0),
        M4_FP_EX(0,10,0),
        M4_HIDE_EX(0),
        M4_UI_MODULE(curve),
        M4_DATAX([0, 64, 128, 192, 256, 320, 384, 448, 512, 576, 640, 704, 768, 832, 896, 960, 1024]),
        M4_RO(0),
        M4_ORDER(1),
        M4_GROUP(edgeWgtCurve_mode_group:shp_cfgCurveDirect_mode),
        M4_NOTES(The manual edgeWgt curve value . Only valid on shp_manualCurve_mode.\n
        Freq of use: low))  */
    // @reg: hw_shp_edgeWgt_val0~16
    uint16_t hw_shpT_edge2ShpStrg_val[17];
} shp_edgeShpStrg_t;

typedef struct shp_edgeShoot_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(filtCfg_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(shp_maxMinFlt_mode_t),
        M4_DEFAULT(shp_maxMinFiltRadius7_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(SHARP maxmin filter mode.\n
        Reference enum types.\n
        Freq of use: low))  */
    //@reg: hw_sharp_maxminFlt_mode
    //@para: maxminFlt_mode
    shp_maxMinFlt_mode_t sw_shpT_maxMinFlt_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(edgeWgt_minLimit),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(3),
        M4_FP_EX(0,1,5),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(linear scaling ratio of the difference between edge overshoot signal and local extremum.\n
        Freq of use: high))  */
    //reg: hw_shp_overshoot_alpha
    float sw_shpT_overShoot_alpha;
    /* M4_GENERIC_DESC(
        M4_ALIAS(edgeWgt_minLimit),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(3),
        M4_FP_EX(0,1,5),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(linear scaling ratio of the difference between edge undershoot signal and local extremum.\n
        Freq of use: high))  */
    //reg: hw_shp_undershoot_alpha
    float sw_shpT_underShoot_alpha;
} shp_edgeShoot_t;

typedef struct shp_edge_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(edgeLpf),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    shp_edgeExtra_t edgeExtra;
    /* M4_GENERIC_DESC(
        M4_ALIAS(glbShpStrg),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    shp_edge_glbShpStrg_t glbShpStrg;
    /* M4_GENERIC_DESC(
        M4_ALIAS(edgeShpStrg),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    shp_edgeShpStrg_t locShpStrg_edge;
    /* M4_GENERIC_DESC(
        M4_ALIAS(edgeShoot),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    shp_edgeShoot_t shootReduction;
    /* M4_GENERIC_DESC(
        M4_ALIAS(edgeMotionWgt1),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_GROUP(locSgmStrg2Mot_mode_group:shp_toMotionStrg1_mode),
        M4_NOTES(TODO))  */
    //@para: edgeMotionWgt_sel
    // @para: edgeStaticRegion_thred
    // @para: edgeMotionRegion_thred
    // @para: edgeMotionWgt_minLimit
    // @para: edgeMotionWgt_maxLimit
    shp_motionStrg1_t locShpStrg_motionStrg1;
    /* M4_GENERIC_DESC(
        M4_ALIAS(edgeMotionWgt2),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_GROUP(locSgmStrg2Mot_mode_group:shp_toMotionStrg2_mode),
        M4_NOTES(TODO))  */
    //para: edgeMotionWgt_sigma
    shp_motionStrg2_t locShpStrg_motionStrg2;
    /* M4_GENERIC_DESC(
        M4_ALIAS(motionWgt),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    shp_lumaShpStrgEn_t locShpStrg_luma;
    /* M4_GENERIC_DESC(
        M4_ALIAS(motionWgt),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    shp_radiDistShpStrgEn_t locShpStrg_radiDist;
    /* M4_GENERIC_DESC(
        M4_ALIAS(motionWgt),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    shp_hueShpStrgEn_t locShpStrg_hue;
} shp_edge_dyn_t;


typedef struct shp_detailSigmaEnv_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_shp_detailBifilt_vsigma),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The range sigma curve of bifilter.\n
        Freq of use: low))  */
    // @reg: hw_shp_preBifilt_vsigma_inv0~7, hw_shp_detailBifilt_vsigma_inv0~7
    shp_sigmaCurve_t sw_shpC_luma2Sigma_curve;
} shp_detailSigmaEnv_dyn_t;



typedef struct shp_detailPreBifilt_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(kernel_sigma_enable),
        M4_TYPE(enum),
        M4_ENUM_DEF(shp_filtCfg_mode_t),
        M4_DEFAULT(shp_cfgByFiltStrg_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP_CTRL(detailPbft_filtCfg_mode_group),
        M4_NOTES(The config mode of pre bifilter used for hi-freq pre filtering processing of input pixels of the sharp module.\n
        Reference enum types.\n
        Freq of use: low))  */
    // @para: kernel_sigma_enable  (kernel_sigma_enable == true) ->  shp_cfgByFiltStrg_mode
    shp_filtCfg_mode_t sw_shpT_filtCfg_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_shp_preBifilt_strg),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,256.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(2),
        M4_FP_EX(0,7,8),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_GROUP(detailPbft_filtCfg_mode_group:shp_cfgByFiltStrg_mode),
        M4_NOTES(The spatial wgt of pre bifilter is operator from the strength value. Only valid on shp_cfgByFiltStrg_mode.\n
        Higher the value, the higher spatial denoise strength.\n
        Freq of use: high))  */
    // @reg: sw_shp_preBifilt_coeff0~sw_shp_preBifilt_coeff2
    // @para: preBifilt_rsigma
    float sw_shpT_filtSpatial_strg;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_shp_preBifilt_coeff),
        M4_TYPE(f32),
        M4_SIZE_EX(1,3),
        M4_RANGE_EX(0,1.0),
        M4_DEFAULT([0.2042,0.1238,0.0751]),
        M4_DIGIT_EX(3),
        M4_FP_EX(0,1,7),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_GROUP(detailPbft_filtCfg_mode_group:shp_cfgByFiltCoeff_mode),
        M4_NOTES(The spatial filter kernel of bifilter . Only valid on shp_cfgByFiltCoeff_mode.\n
        coeff[0] + 4*coeff[1] + 4*coeff[2] == 1.0 .\n
        Freq of use: low))  */
    // @reg: sw_shp_preBifilt_coeff0~sw_shp_preBifilt_coeff2
    // @para: prefilter_coeff
    float hw_shpT_filtSpatial_wgt[3];
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_shp_preBifilt_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,4.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(2),
        M4_FP_EX(0,1,3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The scaling factor of the range sigma of pre bifilter.\n
        Higher the value, the stronger denoise strength of pre bifilter.
        Freq of use: high))  */
    // @reg: hw_shp_preBifilt_vsigma_inv0~7
    // @para: sw_shp_preBifiltVsigma_scale
    float sw_shpT_rgeSgm_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_shp_preBifilt_offset),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1023),
        M4_DEFAULT(1),
        M4_DIGIT_EX(2),
        M4_FP_EX(0,1,3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The offset of the range sigma of pre bifilter.\n
        Higher the value, the stronger denoise strength of pre bifilter.
        Freq of use: high))  */
    // @reg: hw_shp_preBifilt_vsigma_inv0~7
    // @para: sw_shp_preBifiltVsigma_offset
    uint8_t sw_shpT_rgeSgm_offset;
    /* M4_GENERIC_DESC(
        M4_ALIAS(preBifilt_slope),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,4.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(3),
        M4_FP_EX(0,3,8),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The slope of the range weight curve.\n
        Freq of use: high))  */
    // @reg: hw_shp_preBifilt_slope_fix
    float hw_shpT_rgeWgt_slope;

} shp_detailPreBifilt_dyn_t;

typedef struct shp_detail_lpfSrc_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_sharp_preBifilt_alpha),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1.0),
        M4_DEFAULT(0.3),
        M4_DIGIT_EX(3),
        M4_FP_EX(0,1,7),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The wgt of pre bifilted pixel is used in the fusion operation between the pre bifilted pixel and the original pixel.\n
        Higher the value, the stronger denoise strength of pre bilateral filter.
        Freq of use: high))  */
    // @reg: hw_sharp_preBifilt_alpha
    // @para: sw_shp_preBifilt_alpha
    float hw_shpT_detailSrcHf_alpha;

    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_sharp_detailLpfData_alpha),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1.0),
        M4_DEFAULT(0.0),
        M4_DIGIT_EX(3),
        M4_FP_EX(0,1,7),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The fusion alpha of pre_y and hi_nr.\n
        The higher value, the bigger weight of hi_nr.
        Freq of use: high))  */
    // @reg: hw_sharp_detailIn_alpha
    // @para: detailLpfData_alpha
    float hw_shpT_detailSrcMf_alpha;
} shp_detail_lpfSrc_t;

typedef enum shp_filtRadius_mode_e {
    // @note: radius = 1
    shp_filtRadius3_mode = 1,
    // @note: radius = 2
    shp_filtRadius5_mode = 2
} shp_filtRadius_mode_t;

typedef struct shp_hiDetailFilt_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(filtCfg_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(shp_filtCfg_mode_t),
        M4_DEFAULT(shp_cfgByFiltStrg_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP_CTRL(hiDetail_filtCfg_mode_group),
        M4_NOTES(The config mode of gaus filter is lpf for input pixels of the sharp module.\n
        Reference enum types.\n
        Freq of use: high))  */
    shp_filtCfg_mode_t sw_shpT_filtCfg_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hiDetailLpf_radius),
        M4_TYPE(enum),
        M4_ENUM_DEF(shp_filtRadius_mode_t),
        M4_DEFAULT(shp_filtRadius3_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The radius of hi detail gaus filter .\n
        Reference enum types.\n
        Freq of use: high))  */
    shp_filtRadius_mode_t sw_shpT_filtRadius_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hiDetailLpf_rsigma),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,256.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(2),
        M4_FP_EX(0,7,8),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_GROUP(hiDetail_filtCfg_mode_group:shp_cfgByFiltStrg_mode),
        M4_NOTES(The spatial wgt of gaus filter is operator from the strength value. Only valid on shp_cfgBy2SwLpfStrg_mode.\n
        Higher the value, the higher spatial denoise strength.\n
        Freq of use: high))  */
    // @reg: hw_shp_hiDetailLpf_coeff0~5
    float sw_shpT_filtSpatial_strg;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hiDetailLpf_coeff),
        M4_TYPE(f32),
        M4_SIZE_EX(1,6),
        M4_RANGE_EX(0,1.0),
        M4_DEFAULT([0.0632, 0.0558, 0.0492, 0.0383, 0.0338,0.0232]),
        M4_DIGIT_EX(4),
        M4_FP_EX(0,0,7),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_GROUP(hiDetail_filtCfg_mode_group:shp_cfgByFiltCoeff_mode),
        M4_NOTES(The spatial filter kernel of gaus filter . Only valid on shp_cfgByHwLpfCoeff_mode.\n
        coeff[0] + 4*coeff[1] + 4*coeff[2] + 4*coeff[3] + 8*coeff[4] + 4*coeff[5] == 1.\n
        Freq of use: high))  */
    // @reg: hw_shp_hiDetailLpf_coeff0~5
    float hw_shpT_filtSpatial_wgt[6];
} shp_hiDetailFilt_dyn_t;


typedef struct shp_midDetailFilt_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(filtCfg_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(shp_filtCfg_mode_t),
        M4_DEFAULT(shp_cfgByFiltStrg_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP_CTRL(miDetail_filtCfg_mode_group),
        M4_NOTES(The config mode of gaus filter is lpf for input pixels of the sharp module.\n
        Reference enum types.\n
        Freq of use: high))  */
    shp_filtCfg_mode_t sw_shpT_filtCfg_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hiDetailLpf_radius),
        M4_TYPE(enum),
        M4_ENUM_DEF(shp_filtRadius_mode_t),
        M4_DEFAULT(shp_filtRadius5_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The radius of hi detail gaus filter .\n
        Reference enum types.\n
        Freq of use: high))  */
    shp_filtRadius_mode_t sw_shpT_filtRadius_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hiDetailLpf_rsigma),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,256.0),
        M4_DEFAULT(2.0),
        M4_DIGIT_EX(2),
        M4_FP_EX(0,7,8),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_GROUP(miDetail_filtCfg_mode_group:shp_cfgByFiltStrg_mode),
        M4_NOTES(The spatial wgt of gaus filter is operator from the strength value. Only valid on shp_cfgBy2SwLpfStrg_mode.\n
        Higher the value, the higher spatial denoise strength.\n
        Freq of use: high))  */
    // @reg: hw_shp_miDetailLpf_coeff0~5
    float sw_shpT_filt_strg;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hiDetailLpf_coeff),
        M4_TYPE(f32),
        M4_SIZE_EX(1,6),
        M4_RANGE_EX(0,1.0),
        M4_DEFAULT([0.0632, 0.0558, 0.0492, 0.0383, 0.0338,0.0232]),
        M4_DIGIT_EX(4),
        M4_FP_EX(0,0,7),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_GROUP(miDetail_filtCfg_mode_group:shp_cfgByFiltCoeff_mode),
        M4_NOTES(The spatial filter kernel of gaus filter . Only valid on shp_cfgByHwLpfCoeff_mode.\n
        coeff[0] + 4*coeff[1] + 4*coeff[2] + 4*coeff[3] + 8*coeff[4] + 4*coeff[5] == 1.\n
        Freq of use: high))  */
    // @reg: hw_shp_miDetailLpf_coeff0~5
    float hw_shpT_filtSpatial_wgt[6];
} shp_midDetailFilt_dyn_t;


typedef enum shp_tex2DetailAlpha_mode_e {
    // hw_shp_detailFusionWgt_mode == 0,  The larger the texture weight, the bigger the weight of using hi_hf
    shp_tex2AlphaPosCorr_mode = 0,
    // hw_shp_detailFusionWgt_mode == 1,  The larger the texture weight, the bigger the weight of using mi_hf
    shp_tex2AlphaNegCorr_mode = 1
} shp_tex2DetailAlpha_mode_t;

typedef struct shp_detailAlpha_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(freqBlending_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(shp_tex2DetailAlpha_mode_t),
        M4_DEFAULT(shp_tex2AlphaPosCorr_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The mode of the noise curve. \n
        Reference enum types.\n
        Freq of use: high))  */
    // @para: freqBlending_mode
    shp_tex2DetailAlpha_mode_t hw_shpT_detailAlpha_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(fusionWgt_minLimit),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_FP_EX(0,1,0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(the lower limit of detail fusion weight.\n
        Fusion weight is equal to 1024 when this bit set 1.\n
        Freq of use: high))  */
    // @reg: hw_shp_fusionWgt_minLimit
    uint8_t hw_shpT_hiDetailAlpha_minLimit;
    /* M4_GENERIC_DESC(
        M4_ALIAS(fusionWgt_maxLimit),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(1),
        M4_DIGIT_EX(0),
        M4_FP_EX(0,1,0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(the upper limit of detail fusion weight.\n
        Fusion weight is equal to 0 when this bit set 0.\n
        Freq of use: high))  */
    // @reg: hw_shp_fusionWgt_maxLimit
    uint8_t hw_shpT_hiDetailAlpha_maxLimit;
} shp_detailAlpha_t;



typedef struct shp_detailContrast_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(contrast2posStrg_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,9),
        M4_RANGE_EX(0,1.0),
        M4_DIGIT_EX(2),
        M4_DEFAULT([1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0]),
        M4_HIDE_EX(0),
        M4_UI_MODULE(curve),
        M4_DATAX([0, 128, 256, 384, 512, 650, 768, 896, 1024]),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(the scale of detail positive signal based on local dynamic range. \n
        Freq of use: high))  */
    // @reg: hw_shp_contrast2posStrg_val0~8
    float hw_shpT_contrast2posStrg_val[9];
    /* M4_GENERIC_DESC(
        M4_ALIAS(contrast2negStrg_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,9),
        M4_DIGIT_EX(2),
        M4_RANGE_EX(0,1.0),
        M4_DEFAULT([1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0]),
        M4_HIDE_EX(0),
        M4_UI_MODULE(curve),
        M4_DATAX([0, 128, 256, 384, 512, 650, 768, 896, 1024]),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(the scale0 of detail negative signal based on local dynamic range. \n
        Freq of use: high))  */
    // @reg: hw_shp_contrast2negStrg_val0~8
    float hw_shpT_contrast2negStrg_val[9];
} shp_detailContrast_t;

typedef struct shp_detailShootReduction_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(tex2detailPosClip_val),
        M4_TYPE(u16),
        M4_SIZE_EX(1,9),
        M4_RANGE_EX(0,1023),
        M4_DEFAULT([1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023]),
        M4_HIDE_EX(0),
        M4_UI_MODULE(curve),
        M4_DATAX([0, 128, 256, 384, 512, 650, 768, 896, 1024]),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(the clip value of positive detail signals based on texture weight. \n
        Freq of use: high))  */
    // @reg: hw_shp_tex2detailPosClip_val0~8
    uint16_t hw_shpT_tex2DetailPosClip_val[9];
    /* M4_GENERIC_DESC(
        M4_ALIAS(tex2detailNegClip_val),
        M4_TYPE(u16),
        M4_SIZE_EX(1,9),
        M4_RANGE_EX(0,1023),
        M4_DEFAULT([1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023]),
        M4_HIDE_EX(0),
        M4_UI_MODULE(curve),
        M4_DATAX([0, 128, 256, 384, 512, 650, 768, 896, 1024]),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(the clip value of negative  detail signals based on texture weight. \n
        Freq of use: high))  */
    // @reg: hw_shp_tex2detailNegClip_val0~8
    uint16_t hw_shpT_tex2DetailNegClip_val[9];
    /* M4_GENERIC_DESC(
        M4_ALIAS(tex2detailPosClip_val),
        M4_TYPE(u16),
        M4_SIZE_EX(1,8),
        M4_RANGE_EX(0,1023),
        M4_DEFAULT([64, 100, 160, 256, 256, 160, 96, 64]),
        M4_HIDE_EX(0),
        M4_UI_MODULE(curve),
        M4_DATAX([0, 64, 128, 256, 384, 640, 896, 1024]),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(the clip value of positive detail signals based on lo_nr. \n
        Freq of use: high))  */
    // @reg: hw_shp_luma2detailPosClip_val0~7
    uint16_t hw_shpT_luma2DetailPosClip_val[8];
    /* M4_GENERIC_DESC(
        M4_ALIAS(luma2detailNegClip_val),
        M4_TYPE(u16),
        M4_SIZE_EX(1,8),
        M4_RANGE_EX(0,1023),
        M4_DEFAULT([256, 256, 256, 256, 256, 256, 256, 256]),
        M4_HIDE_EX(0),
        M4_UI_MODULE(curve),
        M4_DATAX([0, 64, 128, 256, 384, 640, 896, 1024]),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(the clip value of negative detail signals based on lo_nr. \n
        Freq of use: high))  */
    // @reg: hw_shp_luma2detailNegClip_val0~7
    uint16_t hw_shpT_luma2DetailNegClip_val[8];
} shp_detailShootReduction_t;

typedef struct shp_detail_glbShpStrg_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(posDetail_strg),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,16.0),
        M4_DEFAULT(8),
        M4_DIGIT_EX(2),
        M4_FP_EX(0,4,2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(positive detail global strength.\n
        Freq of use: high))  */
    // @reg: hw_shp_posDetail_strg
    float hw_shpT_detailPos_strg;
    /* M4_GENERIC_DESC(
        M4_ALIAS(posDetail_strg),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,16.0),
        M4_DEFAULT(8),
        M4_DIGIT_EX(2),
        M4_FP_EX(0,4,2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(negative detail global strength.\n
        Freq of use: high))  */
    // @reg: hw_shp_negDetail_strg
    float hw_shpT_detailNeg_strg;
} shp_detail_glbShpStrg_t;

typedef struct shp_detail_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sgmEnv),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    shp_detailSigmaEnv_dyn_t sgmEnv;
    /* M4_GENERIC_DESC(
        M4_ALIAS(preBifilt),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    shp_detailPreBifilt_dyn_t detailExtra_preBifilt;
    /* M4_GENERIC_DESC(
        M4_ALIAS(detailExtra_lpfSrc),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    shp_detail_lpfSrc_t detailExtra_lpfSrc;
    /* M4_GENERIC_DESC(
        M4_ALIAS(lpf),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    shp_hiDetailFilt_dyn_t hiDetailExtra_lpf;
    /* M4_GENERIC_DESC(
        M4_ALIAS(lpf),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    shp_midDetailFilt_dyn_t midDetailExtra_lpf;
    /* M4_GENERIC_DESC(
        M4_ALIAS(fusionWgt),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    shp_detailAlpha_t detailFusion;
    /* M4_GENERIC_DESC(
        M4_ALIAS(fusionWgt),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    shp_detail_glbShpStrg_t glbShpStrg;
    /* M4_GENERIC_DESC(
        M4_ALIAS(tex2DetailStrg),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    shp_texRegionShpStrg_t locShpStrg_texRegion;
    /* M4_GENERIC_DESC(
        M4_ALIAS(shootScale),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    shp_detailContrast_t locShpStrg_contrast;
    /* M4_GENERIC_DESC(
        M4_ALIAS(shootReduce),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    shp_detailShootReduction_t shootReduction;
    /* M4_GENERIC_DESC(
        M4_ALIAS(motionWgt),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(locSgmStrg2Mot_mode_group:shp_toMotionStrg1_mode),
        M4_NOTES(TODO))  */
    //@para: detailMotionWgt_sel
    // @para: detailStaticRegion_thred
    // @para: detailMotionRegion_thred
    // @para: detailMotionWgt_minLimit
    // @para: detailMotionWgt_maxLimit
    shp_motionStrg1_t locShpStrg_motionStrg1;
    /* M4_GENERIC_DESC(
        M4_ALIAS(motionWgt),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(locSgmStrg2Mot_mode_group:shp_toMotionStrg2_mode),
        M4_NOTES(TODO))  */
    //para: detailMotionWgt_sigma
    shp_motionStrg2_t locShpStrg_motionStrg2;
    /* M4_GENERIC_DESC(
        M4_ALIAS(motionWgt),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    shp_lumaShpStrgEn_t locShpStrg_luma;
    /* M4_GENERIC_DESC(
        M4_ALIAS(motionWgt),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    shp_radiDistShpStrgEn_t locShpStrg_radiDist;
    /* M4_GENERIC_DESC(
        M4_ALIAS(motionWgt),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    shp_hueShpStrgEn_t locShpStrg_hue;
} shp_detail_dyn_t;

typedef struct shp_dHiDetail_glbShpStrg_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(posDetail_strg),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,16.0),
        M4_DEFAULT(8),
        M4_DIGIT_EX(2),
        M4_FP_EX(0,4,2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(positive detail global strength.\n
        Freq of use: high))  */
    // @reg: hw_shp_lossTexInHinr_strg
    float hw_shpT_dHiDetail_strg;
} shp_dHiDetail_glbShpStrg_t;

typedef struct shp_deepHfDetailExtra_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(noiseSigma_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,16.0),
        M4_DEFAULT(8),
        M4_DIGIT_EX(2),
        M4_FP_EX(0,4,2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(noise threshold for lossTexInHinr. The scale of ynr_lo_noise_sigma curve.\n
        Freq of use: high))  */
    // @reg: hw_shp_hi_tex_threshold0-8
    float hw_shp_noiseThred_scale;
} shp_deepHfDetailExtra_t;

typedef enum shp_deepHfDetailMotStrg2_mode_e {
    shp_deepHfDetailSyncDetail_mode = 0
} shp_deepHfDetailMotStrg2_mode_t;

typedef struct shp_deepHfDetail_motionStrg2_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(motionStrg_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(shp_deepHfDetailMotStrg2_mode_t),
        M4_DEFAULT(shp_deepHfDetailSyncDetail_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The mode of the noise curve. \n
        Reference enum types.\n
        Freq of use: low))  */
    shp_deepHfDetailMotStrg2_mode_t sw_shpT_motionStrg_mode;
} shp_deepHfDetail_motionStrg2_t;

typedef struct shp_deepHfDetail_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(detailExtra),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    shp_deepHfDetailExtra_t detailExtra;
    /* M4_GENERIC_DESC(
        M4_ALIAS(glbShpStrg),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    shp_dHiDetail_glbShpStrg_t glbShpStrg;
    /* M4_GENERIC_DESC(
        M4_ALIAS(fusionWgt),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    shp_texRegionShpStrg_t locShpStrg_texRegion;
    /* M4_GENERIC_DESC(
        M4_ALIAS(motionWgt1),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(locSgmStrg2Mot_mode_group:shp_toMotionStrg1_mode),
        M4_NOTES(TODO))  */
    //@para: hiFreqMotionWgt_sel
    // @para: hiFreqStaticRegion_thred
    // @para: hiFreqMotionRegion_thred
    // @para: hiFreqMotionWgt_minLimit
    // @para: hiFreqMotionWgt_maxLimit
    shp_motionStrg1_t locShpStrg_motionStrg1;
    /* M4_GENERIC_DESC(
        M4_ALIAS(motionWgt2),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(locSgmStrg2Mot_mode_group:shp_toMotionStrg2_mode),
        M4_NOTES(TODO))  */
    shp_deepHfDetail_motionStrg2_t locShpStrg_motionStrg2;
    /* M4_GENERIC_DESC(
        M4_ALIAS(motionWgt),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    shp_lumaShpStrgEn_t locShpStrg_luma;
    /* M4_GENERIC_DESC(
        M4_ALIAS(motionWgt),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    shp_radiDistShpStrgEn_t locShpStrg_radiDist;
    /* M4_GENERIC_DESC(
        M4_ALIAS(motionWgt),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    shp_hueShpStrgEn_t locShpStrg_hue;
} shp_deepHfDetail_dyn_t;

typedef struct shp_extHfDetail_glbShpStrg_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(posDetail_strg),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,16.0),
        M4_DEFAULT(8),
        M4_DIGIT_EX(2),
        M4_FP_EX(0,4,2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(positive detail global strength.\n
        Freq of use: high))  */
    // @reg: hw_shp_grain_strg
    float hw_shpT_eHfDetail_strg;
} shp_extHfDetail_glbShpStrg_t;

typedef struct shp_extHfDetailExtra_Hpf_s {
    /* M4_GENERIC_DESC(
       M4_ALIAS(grainHpf_filtCfg_mode),
       M4_TYPE(enum),
       M4_ENUM_DEF(shp_filtCfg_mode_t),
       M4_DEFAULT(shp_cfgByFiltCoeff_mode),
       M4_HIDE_EX(0),
       M4_RO(0),
       M4_ORDER(0),
       M4_GROUP_CTRL(extHfDetailExtra_filtCfg_mode_group),
       M4_NOTES(The config mode of gaus filter is lpf for input pixels of the sharp module.\n
       Reference enum types.\n
       Freq of use: low))  */
    shp_filtCfg_mode_t sw_shpT_filtCfg_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(imgHpf_rsigma),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,256.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(2),
        M4_FP_EX(0,7,8),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_GROUP(extHfDetailExtra_filtCfg_mode_group:shp_cfgByFiltStrg_mode),
        M4_NOTES(The spatial wgt of high-pass filter is operator from the strength value. Only valid on shp_cfgBy2SwLpfStrg_mode.\n
        Higher the value, the higher spatial denoise strength.\n
        Freq of use: low))  */
    // @reg: hw_shp_hiDetailLpf_coeff0~5
    float sw_shpT_filtSpatial_strg;
    /* M4_GENERIC_DESC(
        M4_ALIAS(imgHpf_coeff0),
        M4_TYPE(f32),
        M4_SIZE_EX(1,6),
        M4_RANGE_EX(-1.0,1.0),
        M4_DEFAULT([0.6838, -0.1759, -0.0943, 0.0339, 0.0264, 0.0125]),
        M4_DIGIT_EX(4),
        M4_FP_EX(0,0,7),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_GROUP(extHfDetailExtra_filtCfg_mode_group:shp_cfgByFiltCoeff_mode),
        M4_NOTES(5*5 high pass filter coefficient . Only valid on shp_cfgByHwLpfCoeff_mode.\n
        coeff[0] + 4*coeff[1] + 4*coeff[2] + 4*coeff[3] + 8*coeff[4] + 4*coeff[5] == 0.\n
        Freq of use: high))  */
    // @reg: hw_shp_imgHpf_coeff0~5
    float hw_shpT_filtSpatial_wgt[6];
} shp_extHfDetailExtra_t;

typedef struct shp_extHfDetail_shoot_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(tex2detailPosClip_val),
        M4_TYPE(u16),
        M4_SIZE_EX(1,9),
        M4_RANGE_EX(0,1023),
        M4_DEFAULT([153, 153, 153, 153, 153, 153, 153, 153, 153]),
        M4_HIDE_EX(0),
        M4_UI_MODULE(curve),
        M4_DATAX([0, 128, 256, 384, 512, 650, 768, 896, 1024]),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The clip value of positive grain signals based on texture weight. \n
        Freq of use: high))  */
    // @reg: hw_shp_tex2GrainPosClip_val0~8
    uint16_t hw_shpT_tex2DetailPosClip_val[9];
    /* M4_GENERIC_DESC(
        M4_ALIAS(tex2detailNegClip_val),
        M4_TYPE(u16),
        M4_SIZE_EX(1,9),
        M4_RANGE_EX(0,1023),
        M4_DEFAULT([153, 153, 153, 153, 153, 153, 153, 153, 153]),
        M4_HIDE_EX(0),
        M4_UI_MODULE(curve),
        M4_DATAX([0, 128, 256, 384, 512, 650, 768, 896, 1024]),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The clip value of negative grain signals based on texture weight. \n
        Freq of use: high))  */
    // @reg: hw_shp_tex2GrainNegClip_val0~8
    uint16_t hw_shpT_tex2DetailNegClip_val[9];
} shp_extHfDetail_shoot_t;

typedef struct shp_extHfDetail_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(fusionWgt),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    shp_extHfDetailExtra_t detailExtra_hpf;
    /* M4_GENERIC_DESC(
        M4_ALIAS(fusionWgt),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    shp_extHfDetail_glbShpStrg_t glbShpStrg;
    /* M4_GENERIC_DESC(
        M4_ALIAS(fusionWgt),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    shp_extHfDetail_shoot_t shootReduction;
    /* M4_GENERIC_DESC(
        M4_ALIAS(locShpStrg_luma),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    shp_lumaShpStrgEn_t locShpStrg_luma;
} shp_extHfDetail_dyn_t;

typedef struct shp_dyn_s {
    /* M4_GENERIC_DESC(
       M4_ALIAS(localShpStrg),
       M4_TYPE(struct),
       M4_UI_MODULE(normal_ui_style),
       M4_HIDE_EX(0),
       M4_RO(0),
       M4_ORDER(2),
       M4_NOTES(TODO))  */
    shp_locShpStrg_dyn_t locShpStrg;
    /* M4_GENERIC_DESC(
        M4_ALIAS(grain_dyn),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    shp_extHfDetail_dyn_t eHfDetailShp;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hiFreq_dyn),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    shp_deepHfDetail_dyn_t dHfDetailShp;
    /* M4_GENERIC_DESC(
        M4_ALIAS(detail_dyn),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_GROUP(!shpCfg_lp_en_group),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    shp_detail_dyn_t detailShp;
    /* M4_GENERIC_DESC(
        M4_ALIAS(edge_dyn),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    shp_edge_dyn_t edgeShp;
} sharp_params_dyn_t;

typedef struct shp_param_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sta),
        M4_TYPE(struct),
        M4_UI_MODULE(static_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    sharp_params_static_t sta;
    /* M4_GENERIC_DESC(
        M4_ALIAS(dyn),
        M4_TYPE(struct),
        M4_UI_MODULE(dynamic_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    sharp_params_dyn_t dyn;
} sharp_param_t;

#endif


