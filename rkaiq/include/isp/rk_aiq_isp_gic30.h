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

#ifndef _RK_AIQ_PARAM_GIC30_H_
#define _RK_AIQ_PARAM_GIC30_H_

#define GIC_SIGMACURVE_SEGMENT_MAX         15

typedef enum gic_gicPorc_mode_e {
	// @note: "中值滤波+保边滤波"
	// @reg: hw_gic_pro_mode == 0
    gic_medAndEpf_mode = 0,
	// @note: "低通+导向保边滤波"
    // @reg: hw_gic_pro_mode == 1
    gic_guideEpf_mode = 1,
} gic_gicProc_mode_t;

typedef enum gic_locSgmStrg_mode_e {
    // @reg: hw_gic_gain_bypass  == 0
    // @note: "Mixed mode of local and global input pix sigma"
    gic_locGlbSgmStrgMix_mode = 0,
    // @reg: hw_gic_gain_bypass  == 1
    // @note: "Only global input pix sigma"
    gic_glbSgmStrgOnly_mode = 1
} gic_locSgmStrg_mode_t;

typedef struct gic_lumaLutIdx_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(luma_dx),
        M4_TYPE(u16),
        M4_SIZE_EX(1,8),
        M4_RANGE_EX(0, 1024),
        M4_DEFAULT([0, 64, 128, 256, 384, 640, 896, 1024]),
        M4_HIDE_EX(0),
        M4_UI_MODULE(curve),
        M4_DATAX([0, 64, 128, 256, 384, 640, 896, 1024]),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The x-axis of gic softhred curve. \n
        Freq of use: low))  */
    // @reg: hw_gic_luma_dx0~7
    uint16_t hw_gicT_lumaLutIdx_val[8];
} gic_lumaLutIdx_t;

typedef struct gic_locSgmStrg_dyn_t {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_gic_gain_bypass),
        M4_TYPE(enum),
        M4_ENUM_DEF(gic_locSgmStrg_mode_t),
        M4_DEFAULT(gic_locGlbSgmStrgMix_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP_CTRL(locSgmStrg_mode_group),
        M4_NOTES(The mode of gic input pix sigma. Reference enum types.\n
        Freq of use: low))  */
    // @reg: hw_gic_gain_bypass
    // @para: hw_gic_gain_bypass
    gic_locSgmStrg_mode_t hw_gicT_locSgmStrg_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_gic_localGain_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(2),
        M4_FP_EX(0,1,7),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_GROUP(locSgmStrg_mode_group:gic_locGlbSgmStrgMix_mode),
        M4_NOTES(The scaling factor of the local input pix sigma.\n
        Higher the value, the higher the local input pix sigma value.\n
        Freq of use: high))  */
    // @reg: sw_gic_localGain_scale
    // @para: sw_gic_localGain_scale
    float hw_gicT_locSgmStrg_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_gic_global_gain),
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
    // @reg: sw_gic_global_gain
    // @para: global_gain
    float hw_gicT_glbSgmStrg_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_gic_globalGain_alpha),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1.0),
        M4_DEFAULT(0.0),
        M4_DIGIT_EX(2),
        M4_FP_EX(0,1,3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_GROUP(locSgmStrg_mode_group:gic_locGlbSgmStrgMix_mode),
        M4_NOTES(The wgt of the global input pix sigma is used in the fusion operation with the local input pix sigma.\n
        The higher the value, the wgt of bifilted pixel is higher.\n
        Freq of use: low))  */
    // @reg: sw_gic_globalGain_alpha
    // @para: sw_gic_globalGain_alpha
    float hw_gicT_glbSgmStrg_alpha;
} gic_locSgmStrg_dyn_t;

typedef struct gic_locSgmStrgGicStrg_dyn_s {
	/* M4_GENERIC_DESC(
        M4_ALIAS(gain_minThred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,64.0),
        M4_DEFAULT(0.2),
        M4_DIGIT_EX(2),
        M4_FP_EX(0,6,4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The max static region thred of  gain.\n
        Freq of use: low))  */
    // @para: gain_minThred
    float sw_gicT_locSgmStrgStat_maxThred;
	/* M4_GENERIC_DESC(
        M4_ALIAS(gain_maxThred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,64.0),
        M4_DEFAULT(0.5),
        M4_DIGIT_EX(2),
        M4_FP_EX(0,6,4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The min motion region thred of gain.\n
        Freq of use: low))  */
    // @para: gain_maxThred
    float sw_gicT_locSgmStrgMot_minThred;
	/* M4_GENERIC_DESC(
        M4_ALIAS(gainAdjBfFltStrg_minThred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,8.0),
        M4_DEFAULT(0.0),
        M4_DIGIT_EX(2),
        M4_FP_EX(0,1,7),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(the lower limit of detail gain weight.\n
		The hw_gicT_motionStrg_maxLimit must be bigger than hw_gicT_motionStrg_minLimit.\n
        Freq of use: low))  */
    //@para: gainAdjBfFltStrg_minThred
    float hw_shpT_statRegionGic_strg;
	/* M4_GENERIC_DESC(
        M4_ALIAS(gainAdjBfFltStrg_maxThred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,8.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(2),
        M4_FP_EX(0,1,7),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(the uppper limit of gain weight.\n
		The hw_gicT_motionStrg_maxLimit must be bigger than hw_gicT_motionStrg_minLimit.\n
        Freq of use: low))  */
    //@para: gainAdjBfFltStrg_maxThred
    float hw_shpT_motRegionGic_strg;
} gic_locSgmStrgGicStrg_dyn_t;

typedef struct gic_locGicStrg_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(locSgmStrg),
        M4_TYPE(struct),
        M4_UI_MODULE(dynamic_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    gic_locSgmStrg_dyn_t locSgmStrg;
    /* M4_GENERIC_DESC(
        M4_ALIAS(locSgmStrg2GicStrg),
        M4_TYPE(struct),
        M4_UI_MODULE(dynamic_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    gic_locSgmStrgGicStrg_dyn_t locSgmStrg2GicStrg;
} gic_locGicStrg_t;

typedef struct gic_medFilt_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(medFilt_minThred),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,7),
        M4_DEFAULT(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO .\n
        Freq of use: low))  */
    //para: medFilt_minThred
    uint8_t hw_gicT_yFiltClipMin_idx;
    /* M4_GENERIC_DESC(
        M4_ALIAS(medFilt_maxThred),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,7),
        M4_DEFAULT(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO .\n
        Freq of use: low))  */
    //para: medFilt_maxThred
    uint8_t hw_gicT_yFiltClipMax_idx;
    /* M4_GENERIC_DESC(
        M4_ALIAS(medFilt_ratio),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(3),
        M4_FP_EX(0,4,4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO .\n
        Freq of use: low))  */
    //para: medFilt_ratio
    float hw_gicT_yFiltOut_alpha;
    /* M4_GENERIC_DESC(
        M4_ALIAS(medFilt_minThred),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,7),
        M4_DEFAULT(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO .\n
        Freq of use: low))  */
    //para: medFilt_minThred
    uint8_t hw_gicT_uvFiltClipMin_idx;
    /* M4_GENERIC_DESC(
        M4_ALIAS(medFilt_maxThred),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,7),
        M4_DEFAULT(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO .\n
        Freq of use: low))  */
    //para: medFiltUV_maxThred
    uint8_t hw_gicT_uvFiltClipMax_idx;
    /* M4_GENERIC_DESC(
        M4_ALIAS(medFiltUV_ratio),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(3),
        M4_FP_EX(0,4,4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO .\n
        Freq of use: low))  */
    // para: medFiltUV_ratio
    float hw_gicT_uvFiltOut_alpha;
} gic_medFilt_dyn_t;

typedef struct gic_gicPre_medAndEpf_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(gicPost_guideEpf),
        M4_TYPE(struct),
        M4_UI_MODULE(dynamic_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    gic_medFilt_dyn_t medFilt;
} gic_gicPre_medAndEpf_t;

typedef struct gic_diffSgmRat2RgeWgt_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(bfFiltWgt_maxThred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,8.0),
        M4_DEFAULT(2.0),
        M4_DIGIT_EX(3),
        M4_FP_EX(0,4,4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The scaling ratio of noise statistics in the previous frame, used as a texture threshold.\n
        Lower the value, the lower the noise into noise statistics .\n
        Freq of use: low))  */
    // @reg: sw_gic_bfFiltWgt_maxThred
    float sw_gicT_rat2MaxWgt_maxThred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(bfFiltWgt_minThred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,4.0),
        M4_DEFAULT(2.0),
        M4_DIGIT_EX(3),
        M4_FP_EX(0,4,4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The scaling ratio of noise statistics in the previous frame, used as a texture threshold.\n
        Lower the value, the lower the noise into noise statistics .\n
        Freq of use: low))  */
    // @reg: sw_gicT_bfFiltWgt_maxThred
    float sw_gicT_rat2MinWgt_minThred;
} gic_diffSgmRat2RgeWgt_t;


typedef struct gic_epf_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(bfFilt_vsigma),
        M4_TYPE(u16),
        M4_SIZE_EX(1,17),
        M4_RANGE_EX(0, 1024),
        M4_DEFAULT([2,4,6,8,10,12,17,42,92,180,316,500,708,890,995,1023,1023]),
        M4_HIDE_EX(0),
        M4_UI_MODULE(curve),
        M4_DATAX([0, 64, 128, 192, 256, 320, 384, 448, 512, 576, 640, 704, 768, 832, 896, 960, 1024]),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(rgeSgm_mode_group:gic_manualSigma_mode),
        M4_NOTES(The x-axis of gic softhred curve. \n
        Freq of use: low))  */
    // @para: bfFilt_vsigma0~7
    uint16_t hw_gicT_luma2Manual_rgeSgm[17];
    /* M4_GENERIC_DESC(
        M4_ALIAS(noiseCurve_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,4.0),
        M4_DEFAULT(2.0),
        M4_DIGIT_EX(3),
        M4_FP_EX(0,4,4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO .\n
        Freq of use: low))  */
    // @para: noiseCurve_scale
	float sw_gicT_rgeSgm_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(diffSgm2RgeWgt),
        M4_TYPE(struct),
        M4_UI_MODULE(dynamic_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    gic_diffSgmRat2RgeWgt_t diffSgmRat2RgeWgt;
    /* M4_GENERIC_DESC(
        M4_ALIAS(bfFlt_rsigma),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,4.0),
        M4_DEFAULT(2.0),
        M4_DIGIT_EX(3),
        M4_FP_EX(0,4,4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO .\n
        Freq of use: low))  */
    // @reg: sw_gicT_bfFlt_rsigma;
	float sw_gicT_filtSpatial_strg;
    /* M4_GENERIC_DESC(
        M4_ALIAS(bfFlt_ratio),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(3),
        M4_FP_EX(0,4,4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO .\n
        Freq of use: low))  */
    //float sw_gicT_bfFlt_ratio;
	float sw_gicT_filtOut_alpha;
} gic_epf_dyn_t;

typedef struct gic_gicSoftThd_manual_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(noise_thred),
        M4_TYPE(u16),
        M4_SIZE_EX(1,8),
        M4_RANGE_EX(0, 512),
        M4_DEFAULT([0, 64, 128, 256, 384, 640, 896, 1024]),
        M4_HIDE_EX(0),
        M4_UI_MODULE(curve),
        M4_DATAX([0, 64, 128, 256, 384, 640, 896, 1024]),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The x-axis of gic softhred curve. \n
        Freq of use: low))  */
    // @para: noise_thred0~7
    uint16_t hw_gicT_luma2SofThd_thred[8];
} gic_gicSoftThd_manual_t;

typedef struct gic_gicSoftThd_auto_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(autoNoiseThred_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 8.0),
        M4_DEFAULT(2.0),
        M4_DIGIT_EX(3),
        M4_FP_EX(0,4,4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO .\n
        Freq of use: low))  */
    // @para: autoNoiseThred_scale
	float hw_gicT_softThd_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(noise_thred),
        M4_TYPE(u16),
        M4_SIZE_EX(1,8),
        M4_RANGE_EX(0, 512),
        M4_DEFAULT([0, 64, 128, 256, 384, 640, 896, 1024]),
        M4_HIDE_EX(0),
        M4_UI_MODULE(curve),
        M4_DATAX([0, 64, 128, 256, 384, 640, 896, 1024]),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The x-axis of gic softhred curve. \n
        Freq of use: low))  */
    // @para: noise_thred0~7
    uint16_t hw_gicT_luma2Thred_maxLimit[8];
    /* M4_GENERIC_DESC(
        M4_ALIAS(noise_minThred),
        M4_TYPE(u16),
        M4_SIZE_EX(1,8),
        M4_RANGE_EX(0, 512),
        M4_DEFAULT([0, 64, 128, 256, 384, 640, 896, 1024]),
        M4_HIDE_EX(0),
        M4_UI_MODULE(curve),
        M4_DATAX([0, 64, 128, 256, 384, 640, 896, 1024]),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The x-axis of gic softhred curve. \n
        Freq of use: low))  */
    // @para: noise_minThred0~7
    uint16_t hw_gicT_luma2Thred_minLimit[8];
    /* M4_GENERIC_DESC(
        M4_ALIAS(loFltThed_coeff),
        M4_TYPE(u16),
        M4_SIZE_EX(1,2),
        M4_RANGE_EX(0, 32),
        M4_DEFAULT([1,2]),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The x-axis of gic softhred curve. \n
        Freq of use: low))  */
    // @para: loFltThed_coeff0~2
    uint16_t hw_gicT_thredFiltSpatial_wgt[2];
} gic_gicSoftThd_auto_t;

typedef enum gic_medEpfSoftThd_mode_e {
    gic_manualSoftThd_mode = 0
} gic_medEpfSoftThd_mode_t;

typedef struct gic_gicPost_medAndEpf_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(softThd_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(gic_medEpfSoftThd_mode_t),
        M4_DEFAULT(gic_manualSoftThd_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.\n
        Freq of use: low))  */
    gic_medEpfSoftThd_mode_t sw_gicT_softThd_mode;
} gic_gicPost_medAndEpf_t;

typedef enum gic_guideEpfSoftThd_mode_e {
    //hw_gic_manualNoiseThred_en = 1
    gic_softThdManual_mode = 0,
    //hw_gic_manualNoiseThred_en = 0
    gic_softThdAuto_mode = 1
} gic_guideEpfSoftThd_mode_t;

typedef struct gic_guideEpf_lpf_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(loFltGr_coeff),
        M4_TYPE(u16),
        M4_SIZE_EX(1,4),
        M4_RANGE_EX(0, 32),
        M4_DEFAULT([4, 4, 4, 4]),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The x-axis of gic softhred curve. \n
        Freq of use: low))  */
    // @para: loFltGr_coeff0~3
    uint16_t hw_gicT_grFiltSpatial_wgt[4];
    /* M4_GENERIC_DESC(
        M4_ALIAS(loFltGb_coeff),
        M4_TYPE(u16),
        M4_SIZE_EX(1,2),
        M4_RANGE_EX(0, 32),
        M4_DEFAULT([6, 6]),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The x-axis of gic softhred curve. \n
        Freq of use: low))  */
    // @para: loFltGb_coeff0~1
    uint16_t hw_gicT_gbFiltSpatial_wgt[2];
} gic_guideEpf_lpf_t;

typedef struct gic_gicPost_guideEpf_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(loFltGrGb),
        M4_TYPE(struct),
        M4_UI_MODULE(dynamic_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    gic_guideEpf_lpf_t lpf;
    /* M4_GENERIC_DESC(
        M4_ALIAS(autoSoftThd),
        M4_TYPE(struct),
        M4_UI_MODULE(dynamic_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_GROUP(softThd_mode_group:gic_softThdAuto_mode),
        M4_NOTES(TODO))  */
    gic_gicSoftThd_auto_t autoSoftThd;
} gic_gicPost_guideEpf_t;

typedef enum gic_sigma_mode_e {
    // @note: "自适应计算sigma曲线"
    // @reg: hw_gic_manualNoiseCurve_en == 0
    gic_autoSigma_mode = 0,
    // @note: "保边滤波sigma曲线手动配置"
    // @reg: hw_gic_manualNoiseCurve_en == 1
    gic_manualSigma_mode = 1
} gic_sigma_mode_t;

typedef struct gic_epf_static_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(manualNoiseCurve_en),
        M4_TYPE(enum),
        M4_ENUM_DEF(gic_sigma_mode_t),
        M4_DEFAULT(gic_autoSigma_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP_CTRL(rgeSgm_mode_group),
        M4_NOTES(TODO))  */
    gic_sigma_mode_t sw_gicCfg_rgeSgm_mode;
} gic_epf_static_t;

typedef struct gic_gicPost_guideEpf_static_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(softThd_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(gic_guideEpfSoftThd_mode_t),
        M4_DEFAULT(gic_softThdAuto_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP_CTRL(softThd_mode_group),
        M4_NOTES(The mode of gic input pix sigma. Reference enum types.\n
        Freq of use: low))  */
    gic_guideEpfSoftThd_mode_t sw_gicCfg_softThd_mode;
} gic_gicPost_guideEpf_static_t;

typedef struct gic_params_static_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(gic_pro_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(gic_gicProc_mode_t),
        M4_DEFAULT(gic_guideEpf_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP_CTRL(gic_mode_group),
        M4_NOTES(The mode of gic input pix sigma. Reference enum types.\n
        Freq of use: low))  */
    //para: pro_mode
	gic_gicProc_mode_t hw_gicT_gic_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(epf),
        M4_TYPE(struct),
        M4_UI_MODULE(dynamic_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    gic_epf_static_t epf;
    /* M4_GENERIC_DESC(
        M4_ALIAS(gicPost_medAndEpf),
        M4_TYPE(struct),
        M4_UI_MODULE(dynamic_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    gic_gicPost_medAndEpf_t gicPost_medAndEpf;
    /* M4_GENERIC_DESC(
        M4_ALIAS(gicPost_medAndEpf),
        M4_TYPE(struct),
        M4_UI_MODULE(dynamic_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    gic_gicPost_guideEpf_static_t gicPost_guideEpf;
} gic_params_static_t;

typedef struct gic_params_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(locSgmStrg2GicStrg),
        M4_TYPE(struct),
        M4_UI_MODULE(dynamic_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    gic_locGicStrg_t locGicStrg;
    /* M4_GENERIC_DESC(
        M4_ALIAS(gicPre_medAndEpf),
        M4_TYPE(struct),
        M4_UI_MODULE(dynamic_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_GROUP(gic_mode_group:gic_medAndEpf_mode),
        M4_NOTES(TODO))  */
    gic_gicPre_medAndEpf_t gicPre_medAndEpf;
    /* M4_GENERIC_DESC(
        M4_ALIAS(locSgmStrg2GicStrg),
        M4_TYPE(struct),
        M4_UI_MODULE(dynamic_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    gic_lumaLutIdx_t lumaLutCfg;
    /* M4_GENERIC_DESC(
        M4_ALIAS(epf),
        M4_TYPE(struct),
        M4_UI_MODULE(dynamic_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    gic_epf_dyn_t epf;
    /* M4_GENERIC_DESC(
        M4_ALIAS(manualSoftThd),
        M4_TYPE(struct),
        M4_UI_MODULE(dynamic_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_GROUP((gic_mode_group:gic_medAndEpf_mode)||(softThd_mode_group:gic_softThdManual_mode)),
        M4_NOTES(TODO))  */
    gic_gicSoftThd_manual_t manualSoftThd;
    /* M4_GENERIC_DESC(
        M4_ALIAS(gicPost_guideEpf),
        M4_TYPE(struct),
        M4_UI_MODULE(dynamic_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_GROUP(gic_mode_group:gic_guideEpf_mode),
        M4_NOTES(TODO))  */
    gic_gicPost_guideEpf_t gicPost_guideEpf;
} gic_params_dyn_t;

typedef struct gic_param_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sta),
        M4_TYPE(struct),
        M4_UI_MODULE(static_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    gic_params_static_t sta;
    /* M4_GENERIC_DESC(
        M4_ALIAS(dyn),
        M4_TYPE(struct),
        M4_UI_MODULE(dynamic_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    gic_params_dyn_t dyn;
} gic_param_t;

#endif

