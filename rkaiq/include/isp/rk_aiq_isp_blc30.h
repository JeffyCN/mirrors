/*
 * rk_aiq_param_blc30.h
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
#ifndef _RK_AIQ_PARAM_BLC30_H_
#define _RK_AIQ_PARAM_BLC30_H_

/*AIQ6.0 does not support manualOBC mode*/
// #define supportManualOBC

typedef struct blc_obcPreTnr_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_blcC_obR_val),
        M4_TYPE(s16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(-4096,4095),
        M4_DEFAULT(0),
        M4_DIGIT_EX(4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The optical black correction of bayer R pixel.\n
        It is is recommended to be calibrated and generated\n
        Freq of use: high))  */
    //reg: sw_bls_b_fixed
    int16_t hw_blcC_obR_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_blcC_obGr_val),
        M4_TYPE(s16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(-4096,4095),
        M4_DEFAULT(0),
        M4_DIGIT_EX(4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The optical black correction of bayer Gr pixel.\n
        It is is recommended to be calibrated and generated\n
        Freq of use: high))  */
    //reg: sw_bls_a_fixed
    int16_t hw_blcC_obGr_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_blcC_obGb_val),
        M4_TYPE(s16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(-4096,4095),
        M4_DEFAULT(0),
        M4_DIGIT_EX(4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The optical black correction of bayer Gb pixel.\n
        It is is recommended to be calibrated and generated\n
        Freq of use: high))  */
    //reg: sw_bls_d_fixed
    int16_t hw_blcC_obGb_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_blcC_obB_val),
        M4_TYPE(s16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(-4096,4095),
        M4_DEFAULT(0),
        M4_DIGIT_EX(4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(The optical black correction of bayer B pixel.\n
        It is is recommended to be calibrated and generated\n
        Freq of use: high))  */
    //reg: sw_bls_c_fixed
    int16_t hw_blcC_obB_val;
} blc_obcPreTnr_dyn_t;

#ifdef supportManualOBC
typedef enum blc_obcPostTnr_mode_e {
    /*
    (sw_isp_ob_offset > 0) || (sw_isp_ob_predgain > 0)
    @note: It is only recommended for use in low SNR situations in linear mode.
    */
    blc_autoOBCPostTnr_mode = 0,
    /*
    (sw_isp_ob_offset == 0) && (sw_isp_ob_predgain == 0) && (sw_bls1_en == 1)
    */
    blc_manualOBCPostTnr_mode = 1
} blc_obcPostTnr_mode_t;
#endif

#if defined(ISP_HW_V33) || defined(ISP_HW_V35)
typedef struct blc_autoBlc_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_blcT_darkArea_thred),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,4095),
        M4_DEFAULT(200),
        M4_DIGIT_EX(4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Images below the threshold are considered as dark areas for detection.\n
        Freq of use: high))  */
    uint16_t sw_blcT_darkArea_thred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_blcT_lumaR_wgt),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1024),
        M4_DEFAULT(256),
        M4_DIGIT_EX(4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Calculate the weight of average brightness.\n
        lumaR_wgt + lumaG_wgt + lumaB_wgt = 1024\n
        Freq of use: high))  */
    uint16_t sw_blcT_lumaR_wgt;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_blcT_lumaB_wgt),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1024),
        M4_DEFAULT(512),
        M4_DIGIT_EX(4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Calculate the weight of average brightness.\n
        lumaR_wgt + lumaG_wgt + lumaB_wgt = 1024\n
        Freq of use: high))  */
    uint16_t sw_blcT_lumaG_wgt;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_blcT_lumaG_wgt),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1024),
        M4_DEFAULT(256),
        M4_DIGIT_EX(4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Calculate the weight of average brightness.\n
        lumaR_wgt + lumaG_wgt + lumaB_wgt = 1024\n
        Freq of use: high))  */
    uint16_t sw_blcT_lumaB_wgt;
} blc_autoBlc_dyn_t;
#endif

typedef struct blc_obcPostTnr_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_blcT_obcPostTnr_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP_CTRL(obcPostTnr_en_group),
        M4_NOTES(The enable bit of the obc after TNR.\n
        Freq of use: low))  */
    // reg: (sw_isp_ob_offset == 0) && (sw_isp_ob_predgain == 0) && (sw_bls1_en == 0)
    bool sw_blcT_obcPostTnr_en;
#ifdef supportManualOBC
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_blcT_obcPostTnr_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(blc_obcPostTnr_mode_t),
        M4_DEFAULT(blc_autoOBCPostTnr_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(obcPostTnr_en_group),
        M4_GROUP_CTRL(obcPostTnr_mode_group),
        M4_NOTES( The mode of obc that post btnr\n
        Freq of use: high))  */
    blc_obcPostTnr_mode_t sw_blcT_obcPostTnr_mode;
#endif
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_blcT_autoOB_offset),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,511),
        M4_DEFAULT(0),
        M4_DIGIT_EX(4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(obcPostTnr_en_group),
        M4_NOTES(In automatic mode, the OB offset value is added before tnr.\n
        It is only recommended for use in low SNR situations in linear mode.\n
        Freq of use: high))  */
    uint16_t sw_blcT_autoOB_offset;
#if defined(ISP_HW_V33) || defined(ISP_HW_V35)
    /* M4_GENERIC_DESC(
        M4_ALIAS(autoBlc),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_GROUP(obcPostTnr_en_group;autoblc_en_group),
        M4_NOTES(TODO))  */
    blc_autoBlc_dyn_t autoBlc;
#endif
#ifdef supportManualOBC
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_blcT_manualOBR_val),
        M4_TYPE(s16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(-4096,4095),
        M4_DEFAULT(0),
        M4_DIGIT_EX(4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(obcPostTnr_en_group; obcPostTnr_mode_group:blc_manualOBCPostTnr_mode),
        M4_NOTES(In manual mode, the optical black correction of bayer R pixel that deducted after tnr\n
        Freq of use: low))  */
    //reg: sw_bls1_b_fixed
    int16_t hw_blcT_manualOBR_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_blcT_manualOBGr_val),
        M4_TYPE(s16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(-4096,4095),
        M4_DEFAULT(0),
        M4_DIGIT_EX(4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_GROUP(obcPostTnr_en_group; obcPostTnr_mode_group:blc_manualOBCPostTnr_mode),
        M4_NOTES(In manual mode, the optical black correction of bayer Gr pixel that deducted after tnr\n
        Freq of use: low))  */
    //reg: sw_bls1_a_fixed
    int16_t hw_blcT_manualOBGr_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_blcT_manualOBGb_val),
        M4_TYPE(s16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(-4096,4095),
        M4_DEFAULT(0),
        M4_DIGIT_EX(4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_GROUP(obcPostTnr_en_group; obcPostTnr_mode_group:blc_manualOBCPostTnr_mode),
        M4_NOTES(In manual mode, the optical black correction of bayer Gb pixel that deducted after tnr\n
        Freq of use: low))  */
    //reg: sw_bls1_d_fixed
    int16_t hw_blcT_manualOBGb_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_blcT_manualOBB_val),
        M4_TYPE(s16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(-4096,4095),
        M4_DEFAULT(0),
        M4_DIGIT_EX(4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_GROUP(obcPostTnr_en_group; obcPostTnr_mode_group:blc_manualOBCPostTnr_mode),
        M4_NOTES(In manual mode, the optical black correction of bayer B pixel that deducted after tnr\n
        Freq of use: low))  */
    //reg: sw_bls1_c_fixed
    int16_t hw_blcT_manualOBB_val;
#endif
} blc_obcPostTnr_dyn_t;

typedef struct blc_params_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(obcPreTnr),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The dynamic params of optical black correction before TNR.))  */
    blc_obcPreTnr_dyn_t obcPreTnr;
    /* M4_GENERIC_DESC(
        M4_ALIAS(obcPostTnr),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_MARK(LinearOnly),
        M4_NOTES(The dynamic params of optical black correction after TNR.))  */
    blc_obcPostTnr_dyn_t obcPostTnr;
} blc_params_dyn_t;

#if defined(ISP_HW_V33) || defined(ISP_HW_V35)
typedef struct blc_autoBlc_sta_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_blcT_autoBlc_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP_CTRL(autoblc_en_group),
        M4_NOTES(The enable bit of the autoblc after TNR.\n
        Only support when sw_blcT_obcPostTnr_en == 1.\n
        Freq of use: low))  */
    bool sw_blcT_autoBlc_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_blcT_autoBlcEn_thred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,4096.0),
        M4_DEFAULT(128.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(autoblc_en_group),
        M4_NOTES(enable autoblc when the total gain exceeds the threshold))  */
    float sw_blcT_autoBlcEn_thred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_blcT_damping_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0.2),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(autoblc_en_group),
        M4_NOTES(control the convergence speed of parameters produced by autoblc))  */
    float sw_blcT_damping_val;
} blc_autoBlc_sta_t;

#if defined(ISP_HW_V33)
typedef struct blc_obcPostTnr_sta_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_blcT_autoOBWkArd_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.\n
        Freq of use: low))  */
    bool sw_blcT_autoOBWkArd_en;
} blc_obcPostTnr_sta_t;

typedef struct blc_debug_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(obcPostTnr),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO))  */
    blc_obcPostTnr_sta_t obcPostTnr;
} blc_debug_t;
#endif

typedef struct blc_params_static_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(autoBlc),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO))  */
    blc_autoBlc_sta_t autoBlc;
#if defined(ISP_HW_V33)
    /* M4_GENERIC_DESC(
        M4_ALIAS(debug),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    blc_debug_t debug;
#endif
} blc_params_static_t;
#endif

typedef struct blc_param_s {
#if defined(ISP_HW_V33) || defined(ISP_HW_V35)
    /* M4_GENERIC_DESC(
        M4_ALIAS(static_param),
        M4_TYPE(struct),
        M4_UI_MODULE(static_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The static params of demosaic module))  */
    blc_params_static_t sta;
#endif
    /* M4_GENERIC_DESC(
        M4_ALIAS(dyn),
        M4_TYPE(struct),
        M4_UI_MODULE(dynamic_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO))  */
    blc_params_dyn_t dyn;
} blc_param_t;

#endif
