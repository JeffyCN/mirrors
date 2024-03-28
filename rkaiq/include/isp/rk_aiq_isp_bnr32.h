/*
 * rk_aiq_param_bnr32.h
 *
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

#ifndef _RK_AIQ_PARAM_BNR32_H_
#define _RK_AIQ_PARAM_BNR32_H_

#define BNR_ISO_STEP_MAX                        13
#define BNR_NRPIXGAINCURVE_SEGMENT_MAX          16
#define BNR_SIGMACURVE_SEGMENT_MAX              16

typedef enum bnr_inputPixSgm_mode_e {
    // @reg: sw_bayer3d_gain_en == 1
    bnr_inPixSgm_local_mode = 0,
    // @reg: sw_bayer3d_gain_en == 0
    bnr_inPixSgm_glb_mode = 1
} bnr_inPixSgm_mode_t;

typedef enum bnr_logTrans_mode_e {
    // @reg: sw_baynr_lg2_mode == 0
    // @note: "BNR must be config logTrans_fpI4F8_mode when work on hdr mode"
    logTrans_fpI4F8_mode = 0,
    // @reg: sw_baynr_lg2_mode == 1
    // @note: "BNR can be config logTrans_fpI4F8_mode or logTrans_fpI3F9_mode when work on linear mode"
    logTrans_fpI3F9_mode = 1
#if 0
    // @note: "Don't config bnr into LogTrans_fpI2F10_mode or LogTrans_fpIF11_mode"
    // @reg: sw_baynr_lg2_mode == 2
    LogTrans_fpI2F10_mode = 2,

    // @note: "Don't config bnr into LogTrans_fpI2F10_mode or LogTrans_fpI1F11_Mode"
    // @reg: sw_baynr_lg2_mode == 3
    LogTrans_fpI1F11_mode = 3,
#endif
} bnr_logTrans_mode_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(idx),
        M4_TYPE(u16),
        M4_UI_PARAM(data_x),
        M4_SIZE_EX(1,16),
        M4_RANGE_EX(0,65535),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    // @reg: sw_gain_x0 ~ sw_gain_x15
    uint16_t idx[BNR_SIGMACURVE_SEGMENT_MAX];
    /* M4_GENERIC_DESC(
        M4_ALIAS(val),
        M4_TYPE(u16),
        M4_UI_PARAM(data_y),
        M4_SIZE_EX(1,16),
        M4_RANGE_EX(0,65535),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO))  */
    // @reg: sw_gain_y0 ~ sw_gain_y15
    uint16_t val[BNR_SIGMACURVE_SEGMENT_MAX];
} bnr_sigmaCurve_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(idx),
        M4_TYPE(u16),
        M4_UI_PARAM(data_x),
        M4_SIZE_EX(1,16),
        M4_RANGE_EX(0,65535),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    // @reg: sw_gain_x0 ~ sw_gain_x15
    uint16_t idx[BNR_NRPIXGAINCURVE_SEGMENT_MAX];
    /* M4_GENERIC_DESC(
        M4_ALIAS(val),
        M4_TYPE(u16),
        M4_UI_PARAM(data_y),
        M4_SIZE_EX(1,16),
        M4_RANGE_EX(0,65535),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO))  */
    // @reg: sw_gain_y0 ~ sw_gain_y15
    uint16_t val[BNR_NRPIXGAINCURVE_SEGMENT_MAX];
} bnr_pixSgmAdjCurve_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_bnrT_inPixSgm_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(bnr_inPixSgm_mode_t),
        M4_DEFAULT(bnr_inPixSgm_local_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The mode of sigma of input pixel for bnr. Reference enum types.\nFreq of use: low))  */
    bnr_inPixSgm_mode_t hw_bnrT_inPixSgm_mode;
} bnr_inPixSgm_static_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_bnrT_gain2Gain_curve),
        M4_TYPE(struct),
        M4_UI_MODULE(curve_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    bnr_pixSgmAdjCurve_t hw_bnrT_pixSgm2SgmWgt_curve;
} bnr_inPixSgm_dyn_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_bnrCfg_logTrans_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Enable log transform of bnr input pixel.\nTurn on by setting this bit to 1. Must be turn on  in HDR.\nFreq of use: low))  */
    // @reg: sw_baynr_log_bypass
    bool hw_bnrCfg_logTrans_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_bnrCfg_logTrans_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(bnr_logTrans_mode_t),
        M4_DEFAULT(logTrans_fpI4F8_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(It is configed according to the log trans precision and input data range.\nHDR:  LogPrecs_Int4Float8_Mode\nLinear raw10/12: LogPrecs_Int4Float8_Mode or LogPrecs_Int3Float9_Mode\nFreq of use: low))  */
    // @reg: sw_baynr_lg2_mode
    bnr_logTrans_mode_t hw_bnrCfg_logTrans_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_bnrCfg_logTrans_offset),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,8191),
        M4_DEFAULT(256),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The offset of log trans input pixel data.\nFreq of use: low))  */
    // @reg: sw_baynr_lg2_off
    uint16_t hw_bnrCfg_logTrans_offset;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_bnrCfg_invLogTrans_offset),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,65535),
        M4_DEFAULT(32768),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(The offset of log inverse trans input pixel data.\nFreq of use: low))  */
    // @reg: sw_baynr_lg2_lgoff
    uint16_t hw_bnrCfg_invLogTrans_offset;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_bnrCfg_logTrans_maxLimit),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1048575),
        M4_DEFAULT(1048575),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(4),
        M4_NOTES(The max of log inverse trans input pixel data.\nHDR: 0xfffff\nLinear raw10: 0x3ff\nLinear raw12: 0xfff\nFreq of use: low))  */
    // @reg: sw_baynr_dat_max
    uint32_t hw_bnrCfg_logTrans_maxLimit;
} bnr_logTrans_static_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_bnrC_obRef_r),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,4095),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The ob value of bnr input r pixel.\nFreq of use: low))  */
    uint16_t sw_bnrC_obRef_r;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_bnrC_obRef_gr),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,4095),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The ob value of bnr input gr pixel.\nFreq of use: low))  */
    uint16_t sw_bnrC_obRef_gr;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_bnrC_obRef_gb),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,4095),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The ob value of bnr input gb pixel.\nFreq of use: low))  */
    uint16_t sw_bnrC_obRef_gb;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_bnrC_obRef_b),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,4095),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(The ob value of bnr input b pixel.\nFreq of use: low))  */
    uint16_t sw_bnrC_obRef_b;
} bnr_obRef_dyn_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_bnrT_guideImgLpf_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Enable the pre filter the guide img that be guide bilateral filter.\nTurn on by setting this bit to 1.\nFreq of use: high))  */
    // @reg: sw_baynr_gauss_en
    bool hw_bnrT_guideImgLpf_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_bnrT_hdrSFrmGain_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,8.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The scaling factor of hdr short frm dgain as input of bnr. This dgain is only affect bnr sigma.\nHigher the value, higher the dgain.\nFreq of use: low))  */
    // @reg: sw_baynr_dgain0
    // @para: hdr_dgain_scale_s
    // @note: sw_baynr_dgain0 = real_sfrm_dgain * scale real_sfrm_dgain is real short frame dgain from AE.
    float sw_bnrT_hdrSFrmGain_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_bnrT_hdrMFrmGain_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,8.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The scaling factor of hdr middle frm dgain as input of bnr. This dgain is only affect bnr sigma.Higher the value, higher the dgain.Freq of use: low))  */
    // @reg: sw_baynr_dgain1
    // @para: hdr_dgain_scale_m
    // @note: sw_baynr_dgain1 = real_mfrm_dgain * scale real_mfrm_dgain is real middle frame dgain from AE.
    float sw_bnrT_hdrMFrmGain_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_bnrT_nhoodPixDiff_maxLimit),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,4095),
        M4_DEFAULT(1.0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(The max limit of pix range distance that affects the range wgt of bifilt. It is in the linear domain.The higher the value, the mini of the range wgt of bifilt is lower.Freq of use: high))  */
    // @reg: sw_baynr_pix_diff
    // @para: pix_diff
    /* @note:
           hw_bnrCfg_logTrans_en== 1:  It must be trans to log by AIQ before set to sw_baynr_pix_diff.
           hw_bnrCfg_logTrans_en== 0:  It is set to sw_baynr_pix_diff directly.
    */
    uint16_t hw_bnrT_nhoodPixDiff_maxLimit;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_bnrT_nhoodPixWgt_thred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,0.999),
        M4_DEFAULT(0.999),
        M4_DIGIT_EX(10),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(4),
        M4_NOTES(The pix which range wgt is less than threshold is not considered as neighborhood pixels. The higher the value, need stronger texture to be unaffected by bifilt.Freq of use: high))  */
    // @reg: sw_baynr_pix_diff_thld
    // @para: diff_thld
    float hw_bnrT_nhoodPixWgt_thred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_bnrT_filtSpatial_strg),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,16.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(5),
        M4_NOTES(The spatial wgt of bifilt is operator from the strength value.The higher the value, the spatial wgt is higher.Freq of use: high))  */
    // @reg: sw_baynr_weit_d0 ~ sw_baynr_weit_d2
    // @para: edgesofts
    float sw_bnrT_filtSpatial_strg;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_bnrT_bifiltOut_alpha),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(6),
        M4_NOTES(The wgt of bifilted pixel is used in the fusion operation between the bifilted pixel and the original pixel.The higher the value, the wgt of bifilted pixel is higher.Freq of use: high))  */
    // @reg: sw_baynr_reg_w1
    // @para: weight
    float hw_bnrT_bifiltOut_alpha;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_bnrT_softThred_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1.0),
        M4_DEFAULT(0.2),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(7),
        M4_NOTES(The scaling factor of sigma that is used in soft threshold operationThe higher the value, the higher original noise be retained .Freq of use: high))  */
    // @reg: sw_baynr_softthld
    // @para: ratio
    float hw_bnrT_softThd_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_bnrT_bifilt_strg),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,16.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(8),
        M4_NOTES(The spatial wgt of bifilt is operator from the strength value.The higher the value, the spatial wgt is higher.Freq of use: high))  */
    // @para: filter_strength
    float sw_bnrT_bifilt_strg;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_bnrC_luma2Sigma_curve),
        M4_TYPE(struct),
        M4_UI_MODULE(curve_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    // @reg: sw_baynr_sigma_x0 ~ sw_baynr_sigma_x15, sw_baynr_sigma_y0 ~ sw_baynr_sigma_y15
    /* @note: The hw_bnrC_luma2RSigma_curve data is related to the configuration of logTrans and obRef.
        So it is recommended to obtain these three parameters together from the calibration tool.
    */
    bnr_sigmaCurve_t hw_bnrC_luma2Sigma_curve;
} bnr_bifilt_dyn_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(logTrans),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    /* @note:
       The data in the following table is recommended to be generated from the calibration tool:
         logTrans
         obRef
         hw_bnrC_luma2Sigma_curve
    */
    bnr_logTrans_static_t logTrans;
    /* M4_GENERIC_DESC(
        M4_ALIAS(inputPixSgm),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(TODO))  */
    bnr_inPixSgm_static_t inPixSgm;
} bnr_params_static_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(inPixSgm),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    bnr_inPixSgm_dyn_t inPixSgm;
    /* M4_GENERIC_DESC(
        M4_ALIAS(bifilt),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO))  */
    bnr_bifilt_dyn_t bifilt;
    /* M4_GENERIC_DESC(
        M4_ALIAS(obRef),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(1),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    /* @note: The hw_bnrC_luma2RSigma_curve data is related to the configuration of logTrans and obRef.
        So it is recommended to obtain these three parameters together from the calibration tool.
    */
    bnr_obRef_dyn_t obRef;
} bnr_params_dyn_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(en),
        M4_TYPE(bool),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    bool en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sta),
        M4_TYPE(struct),
        M4_UI_MODULE(static_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO))  */
    bnr_params_static_t sta;
    /* M4_GENERIC_DESC(
        M4_ALIAS(dyn),
        M4_TYPE(struct),
        M4_UI_MODULE(dynamic_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    bnr_params_dyn_t dyn;
} bnr_param_t;

#endif
