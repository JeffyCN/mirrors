/*
 * rk_aiq_param_dpc21.h
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

#ifndef _RK_AIQ_PARAM_DPC21_H_
#define _RK_AIQ_PARAM_DPC21_H_

#define DPC_PDAF_POINT_NUM            (16)

/*
DP: defect pixel
NP: normal pixel
WP: white pixel
*/

typedef struct dpc_dpDct_norDist2DpTh_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_rod_green0_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Enable ROD method check for green.\n
        Freq of use: low))  */
    //reg: sw_rod_green0_en ~ sw_rod_green2_en
    bool hw_dpcT_dctG_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_rod_red_blue0_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Enable ROD method check for red blue.\n
        Freq of use: low))  */
    //reg: sw_rod_red_blue0_en ~ sw_rod_red_blue2_en
    bool hw_dpcT_dctRB_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_dpcc_od0_range_g),
        M4_TYPE(u8),
        M4_RANGE_EX(0,3),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO\n
        Freq of use: low))  */
    //reg: sw_dpcc_od0_range_g ~ sw_dpcc_od2_range_g
    unsigned char hw_dpcT_ordDpThG_idx;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_dpcc_od0_range_rb),
        M4_TYPE(u8),
        M4_RANGE_EX(0,3),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO\n
        Freq of use: low))  */
    //reg: sw_dpcc_od0_range_rb ~ sw_dpcc_od2_range_rb
    unsigned char hw_dpcT_ordDpThRB_idx;
    /* M4_GENERIC_DESC(
        M4_ALIAS(od_green0_enable),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Enable order detect for green.\n
        Freq of use: low))  */
    //reg: od_green0_enable ~ od_green2_enable
    bool hw_dpcT_dctByDpIdxThG_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(od_red_blue0_enable),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Enable order detect check for green.\n
        Freq of use: low))  */
    //reg: od_red_blue0_enable ~ od_red_blue2_enable
    bool hw_dpcT_dctByDpIdxThRB_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_dpcc_rod_mindis0_g),
        M4_TYPE(u8),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Min distance for green.
        The smaller the value, the more likely it is to be judged as a defective pixel.\n
        Freq of use: low))  */
    //reg: sw_dpcc_rod_mindis0_g ~ sw_dpcc_rod_mindis2_g
    unsigned char hw_dpcT_norDistG_minLimit;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_dpcc_rod_mindis0_rb),
        M4_TYPE(u8),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Min distance for red/blue.
        The smaller the value, the more likely it is to be judged as a defective pixel.\n
        Freq of use: low))  */
    //reg: sw_dpcc_rod_mindis0_rb ~ sw_dpcc_rod_mindis2_rb
    unsigned char hw_dpcT_norDistRB_minLimit;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_dpcc_rod0_dis_max),
        M4_TYPE(u8),
        M4_RANGE_EX(0,63),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Distance scale for max channel.
        The smaller the value, the more likely it is to be judged as a defective pixel.\n
        Freq of use: low))  */
    //reg: sw_dpcc_rod0_dis_max ~ sw_dpcc_rod2_dis_max
    unsigned char hw_dpcT_dist2LightDpTh_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_dpcc_rod0_dis_min),
        M4_TYPE(u8),
        M4_RANGE_EX(0,63),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Distance scale for min channel.
        The smaller the value, the more likely it is to be judged as a defective pixel.\n
        Freq of use: low))  */
    //reg: sw_dpcc_rod0_dis_min ~ sw_dpcc_rod2_dis_min
    unsigned char hw_dpcT_dist2DarkDpTh_scale;
} dpc_dpDct_norDist2DpTh_t;

typedef struct dpcc_dpDct_dpThCfg_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(rdd_green0_enable),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Enable rank differernce detect check for green.\n
        Freq of use: low))  */
    //reg: rdd_green0_enable ~ rdd_green2_enable
    bool hw_dpcT_dctG_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(rdd_red_blue0_enable),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Enable rank differernce detect check for green.\n
        Freq of use: low))  */
    //reg: rdd_red_blue0_enable ~ rdd_red_blue2_enable
    bool hw_dpcT_dctRB_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_dpcc_rdd0_thr_g),
        M4_TYPE(u8),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Rank neighbor difference threshold for green.
        The smaller the value, the more likely it is to be judged as a defective pixel.\n
        Freq of use: low))  */
    //reg: sw_dpcc_rdd0_thr_g ~ sw_dpcc_rdd2_thr_g
    unsigned char hw_dpcT_dpLumaG_thread;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_dpcc_rdd0_thr_rb),
        M4_TYPE(u8),
        M4_RANGE_EX(0,64),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Rank neighbor difference threshold for red/blue.
        The smaller the value, the more likely it is to be judged as a defective pixel.\n
        Freq of use: low))  */
    //reg: sw_dpcc_rdd0_thr_rb ~ sw_dpcc_rdd2_thr_rb
    unsigned char hw_dpcT_dpLumaRB_thread;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_dpcc_rdd0_range_g),
        M4_TYPE(u8),
        M4_RANGE_EX(0,3),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Rank offset to neighbor for green.
        The smaller the value, the more likely it is to be judged as a defective pixel.\n
        Freq of use: low))  */
    //reg: sw_dpcc_rdd0_range_g ~ sw_dpcc_rdd2_range_g
    unsigned char hw_dpcT_ordDpThG_idx;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_dpcc_rdd0_range_rb),
        M4_TYPE(u8),
        M4_RANGE_EX(0,3),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Rank offset to neighbor for red/blue.
        The smaller the value, the more likely it is to be judged as a defective pixel.\n
        Freq of use: low))  */
    //reg: sw_dpcc_rdd0_range_rb ~ sw_dpcc_rdd2_range_rb
    unsigned char hw_dpcT_ordDpThRB_idx;
} dpc_dpDct_dpThCfg_t;

typedef struct dpc_dpDct_grad_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(rd_green0_enable),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Enable rank detect check for green.\n
        Freq of use: low))  */
    //reg: rd_green0_enable ~ rd_green2_enable
    bool hw_dpcT_dctG_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(rd_red_blue0_enable),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Enable rank detect check for red/blue.\n
        Freq of use: low))  */
    //reg: rd_red_blue0_enable ~ rd_red_blue2_enable
    bool hw_dpcT_dctRB_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_dpcc_rd0_fac_g),
        M4_TYPE(u8),
        M4_RANGE_EX(0,64),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Rank detect factor for green.
        The smaller the value, the more likely it is to be judged as a defective pixel.\n
        Freq of use: low))  */
    //reg: sw_dpcc_rd0_fac_g ~ sw_dpcc_rd2_fac_g
    unsigned char hw_dpcT_dpGradThG_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_dpcc_rd0_fac_rb),
        M4_TYPE(u8),
        M4_RANGE_EX(0,64),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Rank detect factor for red/blue.
        The smaller the value, the more likely it is to be judged as a defective pixel.\n
        Freq of use: low))  */
    //reg: sw_dpcc_rd0_fac_rb ~ sw_dpcc_rd2_fac_rb
    unsigned char hw_dpcT_dpGradThRB_scale;
} dpc_dpDct_grad_t;

typedef struct dpc_dpDct_edg_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(epd_green0_enable),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Enable edge preservation detect for green.\n
        Freq of use: low))  */
    //reg: epd_green0_enable ~ epd_green2_enable
    bool hw_dpcT_dctG_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(epd_red_blue0_enable),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Enable edge preservation detect for red_blue.\n
        Freq of use: low))  */
    //reg: epd_red_blue0_enable ~ epd_red_blue2_enable
    bool hw_dpcT_dctRB_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_dpcc_epd0_fac_g),
        M4_TYPE(u8),
        M4_RANGE_EX(0,64),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Line MAD factor for green.
        The smaller the value, the more likely it is to be judged as a defective pixel.\n
        Freq of use: low))  */
    //reg: sw_dpcc_epd0_fac_g ~ sw_dpcc_epd2_fac_g
    unsigned char hw_dpcT_dpEdgThG_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_dpcc_epd0_fac_rb),
        M4_TYPE(u8),
        M4_RANGE_EX(0,64),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Line MAD factor for red/blue.
        The smaller the value, the more likely it is to be judged as a defective pixel.\n
        Freq of use: low))  */
    //reg: sw_dpcc_epd0_fac_rb ~ sw_dpcc_epd2_fac_rb
    unsigned char hw_dpcT_dpEdgThRB_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_dpcc_epd0_thr_g),
        M4_TYPE(u8),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Line threshold for green.
        The smaller the value, the more likely it is to be judged as a defective pixel.\n
        Freq of use: low))  */
    //reg: sw_dpcc_epd0_thr_g ~ sw_dpcc_epd2_thr_g
    unsigned char hw_dpcT_dpEdgThG_offset;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_dpcc_epd0_thr_rb),
        M4_TYPE(u8),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Line threshold for red/blue.
        The smaller the value, the more likely it is to be judged as a defective pixel.\n
        Freq of use: low))  */
    //reg: sw_dpcc_epd0_thr_rb ~ sw_dpcc_epd2_thr_rb
    unsigned char hw_dpcT_dpEdgThRB_offset;
} dpc_dpDct_edg_t;

typedef struct dpc_dpDct_peak_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(pd_green0_enable),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Enable peak gradient check for green.\n
        Freq of use: low))  */
    //reg: pd_green0_enable ~ pd_green2_enable
    bool hw_dpcT_dctG_en; 
    /* M4_GENERIC_DESC(
        M4_ALIAS(pd_red_blue0_enable),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Enable peak gradient check for red/blue.\n
        Freq of use: low))  */
    //reg: pd_red_blue0_enable ~ pd_red_blue2_enable
    bool hw_dpcT_dctRB_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_dpcc_pd0_fac_rb),
        M4_TYPE(u8),
        M4_RANGE_EX(0,64),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Peak gradient factor for green.
        The smaller the value, the more likely it is to be judged as a defective pixel.\n
        Freq of use: low))  */
    //reg: sw_dpcc_pd0_fac_rb ~ sw_dpcc_pd2_fac_rb
    unsigned char hw_dpcT_dpPeakThG_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_dpcc_pd0_fac_rb),
        M4_TYPE(u8),
        M4_RANGE_EX(0,64),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Peak gradient factor for red/blue.
        The smaller the value, the more likely it is to be judged as a defective pixel.\n
        Freq of use: low))  */
    //reg: sw_dpcc_pd0_fac_rb ~ sw_dpcc_pd2_fac_rb
    unsigned char hw_dpcT_dpPeakThRb_scale;
} dpc_dpDct_peak_t;

typedef enum dpc_dpDctEngine_mode_e {
    dpc_usrCfg_mode = 0,     // User defined mode, corresponding to the original expert mode
    dpc_dpSingleDefault_mode = 1,  
    dpc_dpSmallClusDefault_mode = 2,
    dpc_dpBigClusDefault_mode = 3
} dpc_dpDctEngine_mode_t;

typedef struct dpc_dpDct_cfgEngine_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_dpcc_engine_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO\n
        Freq of use: low))  */
    //reg: sw_dpcc_engine_en
    bool hw_dpcT_engine_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_dpcT_engine_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(dpc_dpDctEngine_mode_t),
        M4_DEFAULT(dpc_usrCfg_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.\n
        Reference enum types.\n
        Freq of use: low))  */
    dpc_dpDctEngine_mode_t sw_dpcT_engine_mode;
        /* M4_GENERIC_DESC(
        M4_ALIAS(sw_dpcT_FastMode_strg),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,10),
        M4_DEFAULT(1),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Directly configure the correction strength of
        the defect point detection algorithm when sw_dpcct_engine_mode != dpcc_usrCfg_mode.\n
        Freq of use: low))  */
    unsigned char sw_dpcT_defaultMode_strg;
        /* M4_GENERIC_DESC(
        M4_ALIAS(peakDct),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))   */
    dpc_dpDct_peak_t dpDct_peak;
    /* M4_GENERIC_DESC(
        M4_ALIAS(edgDct),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))   */
    dpc_dpDct_edg_t dpDct_edg;
    /* M4_GENERIC_DESC(
        M4_ALIAS(rankOrdDct),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))   */
    dpc_dpDct_norDist2DpTh_t dpDct_norDist2DpTh;
    /* M4_GENERIC_DESC(
        M4_ALIAS(rankDiffDct),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))   */
    dpc_dpDct_dpThCfg_t dpDct_dpThCfg;
    /* M4_GENERIC_DESC(
        M4_ALIAS(rankDct),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))   */
    dpc_dpDct_grad_t dpDct_grad;
} dpc_dpDct_cfgEngine_t;

typedef struct dpc_dpDct_fixEngine_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_dpcc_engine_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO\n
        Freq of use: low))  */
    //reg: sw_dpcc_engine_en
    bool hw_dpcT_engine_en;
} dpc_dpDct_fixEngine_t;

typedef enum dpc_dpcByMux_mode_e {
    // @reg: sw_dpcc_output_sel == 0
    dpc_dpcByMedFilt_mode = 0,
    // @reg: sw_dpcc_output_sel==1 && sw_dpcc_roc_out_sel == 0
    dpc_dpcByDpIdxThEngine0_mode = 1,
    // @reg: sw_dpcc_output_sel==1 && sw_dpcc_roc_out_sel == 1
    dpc_dpcByDpIdxThEngine1_mode = 2,
    // @reg: sw_dpcc_output_sel==1 && sw_dpcc_roc_out_sel == 2
    dpc_dpcByDpIdxThEngine2_mode = 3
} dpc_dpcByMux_mode_t;

typedef enum dpc_medFilt_mode_e {
    // @reg: sw_dpcc_median_mode_sel == 0
    dpc_medEntireKernel_mode = 0,
    // @reg: (sw_dpcc_median_mode_sel == 1) && (sw_dpcc_incl_center_en == 1)
    dpc_medNrstNhoodDpInc_mode = 1,
    // @reg: (sw_dpcc_median_mode_sel == 1) && (sw_dpcc_incl_center_en == 0)
    dpc_medNrstNhoodDpExc_mode = 2    
} dpc_medFilt_mode_t;

typedef struct dpc_dpcProc_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_dpcc_roc_out_sel),
        M4_TYPE(enum),
        M4_ENUM_DEF(dpc_dpcByMux_mode_t),
        M4_DEFAULT(dpc_dpcByMedFilt_mode),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Select ROD method.\n
        Freq of use: low))  */
    //reg: sw_dpcc_roc_out_sel
    dpc_dpcByMux_mode_t hw_dpcT_dpcByMux_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_dpcT_medFiltG_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(dpc_medFilt_mode_t),
        M4_DEFAULT(dpc_medEntireKernel_mode),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The mode of the median filter for the G channel.When the dpcByMux_mode == dpcc_dpcByMedFilt_mode, the defect pixels are replaced by the median filter result.\n
        Freq of use: low))  */
    //reg: sw_dpcc_median_mode_sel, sw_dpcc_incl_center_en
    dpc_medFilt_mode_t hw_dpcT_medFiltG_mode;    
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_dpcT_medFiltRB_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(dpc_medFilt_mode_t),
        M4_DEFAULT(dpc_medEntireKernel_mode),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The mode of the median filter for the R/B channel.When the dpcByMux_mode == dpcc_dpcByMedFilt_mode, the defect pixels are replaced by the median filter result.\n
        Freq of use: low))  */
    //reg: sw_dpcc_median_mode_sel, sw_dpcc_incl_center_en
    dpc_medFilt_mode_t hw_dpcT_medFiltRB_mode;   
} dpc_dpcProc_t;

/*
*    |<-----------                                         dpcc input width                         --------->|
*     +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++   
*    +                                                                                             
*    +                                                                                                     
*    +                      |<-----------                     zonesRow_num                          --------->|      
*    +    (spcWin_x, spcWin_y) #+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++    
*    +                      +                                        +                                        +      
*    +                      +   *(spInZoneCoord_x,spInZoneCoord_y)   +   *           zone                     + 
*    +                      +                                        +                                        + 
*    +                      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*    +                      +                                        +                                        +  
*    +                      +   *           zone                     +   *           zone                     + 
*    +                      +                                        +                                        + 
*    +                      +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*    +                                                                                               
*    +                                                                                               
*    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/

typedef enum dpc_spc_mode_e {
    /*
    sw_pdaf_forward_med = 0
    */
    dpc_spcByMedBackward_mode = 0,
    /*
    sw_pdaf_forward_med = 1
    */
    dpc_spcByMedforward_mode = 1
} dpc_spc_mode_t;
 /*spc: shield pixel correct */
typedef struct dpc_spc_s {
	/* M4_GENERIC_DESC(
        M4_ALIAS(sw_pdaf_en),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Enable PDAF point replacement.
        Turn on by setting this bit to 1.\n
        Freq of use: high))  */
    // reg: sw_pdaf_en;
    bool hw_dpcCfg_spc_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_pdaf_offsetx),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,8191),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The beginning x coordinate of pdaf pattern.\n
		Freq of use: low))  */
    //para:pdaf_offsetx   reg: sw_pdaf_offsetx
    uint16_t hw_dpcCfg_win_x;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_pdaf_offsety),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,8191),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The beginning y coordinate of pdaf pattern.\n
		Freq of use: low))  */
    //para:pdaf_offsety   reg: sw_pdaf_offsety
    uint16_t hw_dpcCfg_win_y;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_pdaf_wrapx),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1023),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The width of pdaf pixel pattern.\n
        Freq of use: low))  */
    //para:pdaf_offsetx   reg: sw_pdaf_wrapx
    uint16_t hw_dpcCfg_zone_width;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_pdaf_wrapy),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1023),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The height of pdaf pixel pattern.\n
        Freq of use: low))  */
    //para:pdaf_wrapy     reg: sw_pdaf_wrapy
    uint16_t hw_dpcCfg_zone_height;    
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_pdaf_wrapx_num),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1023),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The num of wrapy.\n
        Freq of use: low))  */
    //para:pdaf_wrapx_num     reg: sw_pdaf_wrapx_num
    uint16_t hw_dpcCfg_zonesRow_num;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_pdaf_wrapy_num),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1023),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The num of wrapx.\n
        Freq of use: low))  */
    //para:pdaf_wrapy_num     reg: sw_pdaf_wrapy_num
    uint16_t hw_dpcCfg_zonesCol_num;
	/* M4_GENERIC_DESC(
        M4_ALIAS(sw_pdaf_point0_en),
        M4_TYPE(bool),
        M4_SIZE_EX(1,16),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Point replace enable bit.\n
        Freq of use: high))  */
    //para:pdaf_point_en   reg: sw_pdaf_point0_en ~ sw_pdaf_point15_en
    uint16_t hw_dpcCfg_spcEnInZone_bit[DPC_PDAF_POINT_NUM];    
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_pdaf_point0_x),
        M4_TYPE(u8),
        M4_SIZE_EX(1,16),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Point y coordinate.\n
        Freq of use: low))  */
    //para:point_x   reg: sw_pdaf_point0_x ~ sw_pdaf_point15_x
    unsigned char hw_dpcCfg_spInZoneCoord_x[DPC_PDAF_POINT_NUM];
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_pdaf_point0_y),
        M4_TYPE(u8),
        M4_SIZE_EX(1,16),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Point x coordinate.\n
        Freq of use: low))  */
    //para:point_y   reg: sw_pdaf_point0_y ~ sw_pdaf_point15_y
    unsigned char hw_dpcCfg_spInZoneCoord_y[DPC_PDAF_POINT_NUM];
    /* M4_GENERIC_DESC(
        M4_ALIAS(pdaf_forward_med),
        M4_TYPE(enum),
        M4_ENUM_DEF(dpc_spc_mode_t),
        M4_DEFAULT(dpc_spcByMedBackward_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Enable forward median value to replace pdaf point.\n
        Freq of use: low))  */
    //para:pdaf_forward_med     reg: sw_pdaf_forward_med
    dpc_spc_mode_t hw_dpc_Cfg_spc_mode;
} dpc_spc_t;

typedef enum dpc_srcFmt_e {
    /*
    sw_dpcc_grayscale_mode = 0
    */
    dpc_srcFmt_bayer = 0,
    /*
    sw_dpcc_grayscale_mode = 1
    */
    dpc_srcFmt_bw = 1
} dpc_srcFmt_t;

typedef enum dpc_dpcROI_mode_e {
    dpc_imgBoundaryIncl_mode = 0,
    dpc_imgBoundaryExcl_mode = 1,
} dpc_dpcROI_mode_t;

typedef struct dpc_params_static_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_dpcc_grayscale_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(dpc_srcFmt_t),
        M4_DEFAULT(dpc_srcFmt_bayer),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.\n
		Freq of use: low))  */
    // reg: sw_dpcc_grayscale_mode;
    dpc_srcFmt_t hw_dpcCfg_src_fmt;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_dpcc_border_bypss_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(dpc_dpcROI_mode_t),
        M4_DEFAULT(dpc_imgBoundaryIncl_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.\n
		Freq of use: low))  */
    // reg: sw_dpcc_border_bypss_mode;
    dpc_dpcROI_mode_t hw_dpcCfg_dpcROI_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(spc),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))   */
    dpc_spc_t spc;
} dpc_params_static_t;

typedef struct dpc_params_dyn_t {
    /* M4_GENERIC_DESC(
        M4_ALIAS(dpDctEngine0),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))   */
    dpc_dpDct_cfgEngine_t dpDct_cfgEngine[3];
    /* M4_GENERIC_DESC(
        M4_ALIAS(dpDctEngine2),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))   */
    dpc_dpDct_fixEngine_t dpDct_fixEngine;    
    /* M4_GENERIC_DESC(
        M4_ALIAS(dpcProc),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))   */
    dpc_dpcProc_t dpcProc;
} dpc_params_dyn_t;

typedef struct dpc_param_s{
    /* M4_GENERIC_DESC(
        M4_ALIAS(static),
        M4_TYPE(struct),
        M4_UI_MODULE(dynamic_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    dpc_params_static_t sta;
    /* M4_GENERIC_DESC(
        M4_ALIAS(dynamic_param),
        M4_TYPE(struct),
        M4_UI_MODULE(dynamic_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(All dynamic params array corresponded with iso array))  */
    dpc_params_dyn_t dyn;
} dpc_param_t;

#endif
