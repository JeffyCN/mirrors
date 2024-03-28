/*
 *rk_aiq_types_alsc_hw.h
 *
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

#ifndef _RK_AIQ_TYPE_ABAYERTNR_HW_V30_H_
#define _RK_AIQ_TYPE_ABAYERTNR_HW_V30_H_
#include "rk_aiq_comm.h"




typedef struct RK_Bayertnr_Fix_V30_s {
    /* BAY3D_CTRL */
    uint8_t bypass_en;
    uint8_t iirsparse_en;
    /* BAY3D_CTRL1 */
    uint8_t transf_bypass_en;
    uint8_t sigma_curve_double_en;
    uint8_t cur_spnr_bypass_en;
    uint8_t cur_spnr_sigma_idxfilt_bypass_en;
    uint8_t cur_spnr_sigma_curve_double_en;
    uint8_t pre_spnr_bypass_en;
    uint8_t pre_spnr_sigma_idxfilt_bypass_en;
    uint8_t pre_spnr_sigma_curve_double_en;
    uint8_t lpf_hi_bypass_en;
    uint8_t lo_diff_vfilt_bypass_en;
    uint8_t lpf_lo_bypass_en;
    uint8_t lo_wgt_hfilt_en;
    uint8_t lo_diff_hfilt_en;
    uint8_t sig_hfilt_en;
    uint8_t gkalman_en;
    uint8_t spnr_pre_sigma_use_en;
    uint8_t lo_detection_mode;
    uint8_t md_wgt_out_en;
    uint8_t md_bypass_en;
    uint8_t iirspnr_out_en;
    uint8_t lomdwgt_dbg_en;
    uint8_t curdbg_out_en;
    uint8_t noisebal_mode;
    /* BAY3D_CTRL2 */
    uint8_t transf_mode;
    uint8_t wgt_cal_mode;
    uint8_t mge_wgt_ds_mode;
    uint8_t kalman_wgt_ds_mode;
    uint8_t mge_wgt_hdr_sht_thred;
    uint8_t sigma_calc_mge_wgt_hdr_sht_thred;
    /* BAY3D_TRANS0 */
    uint16_t transf_mode_offset;
    uint8_t transf_mode_scale;
    uint16_t itransf_mode_offset;
    /* BAY3D_TRANS1 */
    uint32_t transf_data_max_limit;
    /* BAY3D_CURDGAIN */
    uint16_t cur_spnr_sigma_hdr_sht_scale;
    uint16_t cur_spnr_sigma_scale;
    /* BAY3D_CURSIG_X */
    uint16_t cur_spnr_luma_sigma_x[16];
    /* BAY3D_CURSIG_Y */
    uint16_t cur_spnr_luma_sigma_y[16];
    /* BAY3D_CURGAIN_OFF */
    uint16_t cur_spnr_sigma_rgain_offset;
    uint16_t cur_spnr_sigma_bgain_offset;
    /* BAY3D_CURSIG_OFF */
    uint16_t cur_spnr_sigma_hdr_sht_offset;
    uint16_t cur_spnr_sigma_offset;
    /* BAY3D_CURWTH */
    uint16_t cur_spnr_pix_diff_max_limit;
    uint16_t cur_spnr_wgt_cal_offset;
    /* BAY3D_CURBFALP */
    uint16_t cur_spnr_wgt;
    uint16_t pre_spnr_wgt;
    /* BAY3D_CURWDC */
    uint16_t cur_spnr_space_rb_wgt[9];
    /* BAY3D_CURWDY */
    uint16_t cur_spnr_space_gg_wgt[9];
    /* BAY3D_IIRDGAIN */
    uint16_t pre_spnr_sigma_hdr_sht_scale;
    uint16_t pre_spnr_sigma_scale;
    /* BAY3D_IIRSIG_X */
    uint16_t pre_spnr_luma_sigma_x[16];
    /* BAY3D_IIRSIG_Y */
    uint16_t pre_spnr_luma_sigma_y[16];
    /* BAY3D_IIRGAIN_OFF */
    uint16_t pre_spnr_sigma_rgain_offset;
    uint16_t pre_spnr_sigma_bgain_offset;
    /* BAY3D_IIRSIG_OFF */
    uint16_t pre_spnr_sigma_hdr_sht_offset;
    uint16_t pre_spnr_sigma_offset;
    /* BAY3D_IIRWTH */
    uint16_t pre_spnr_pix_diff_max_limit;
    uint16_t pre_spnr_wgt_cal_offset;
    /* BAY3D_IIRWDC */
    uint16_t pre_spnr_space_rb_wgt[9];
    /* BAY3D_IIRWDY */
    uint16_t pre_spnr_space_gg_wgt[9];
    /* BAY3D_BFCOEF */
    uint16_t cur_spnr_wgt_cal_scale;
    uint16_t pre_spnr_wgt_cal_scale;

    /* BAY3D_TNRSIG_X */
    uint16_t tnr_luma_sigma_x[20];
    /* BAY3D_TNRSIG_Y */
    uint16_t tnr_luma_sigma_y[20];
    /* BAY3D_TNRHIW */
    uint16_t tnr_lpf_hi_coeff[9];
    /* BAY3D_TNRLOW*/
    uint16_t tnr_lpf_lo_coeff[9];
    /* BAY3D_TNRGF3 */
    uint16_t tnr_wgt_filt_coeff0;
    uint16_t tnr_wgt_filt_coeff1;
    uint16_t tnr_wgt_filt_coeff2;
    /* BAY3D_TNRSIGSCL */
    uint16_t tnr_sigma_scale;
    uint16_t tnr_sigma_hdr_sht_scale;
    /* BAY3D_TNRVIIR */
    uint8_t tnr_sig_vfilt_wgt;
    uint8_t tnr_lo_diff_vfilt_wgt;
    uint8_t tnr_lo_wgt_vfilt_wgt;
    uint8_t tnr_sig_first_line_scale;
    uint8_t tnr_lo_diff_first_line_scale;
    /* BAY3D_TNRLFSCL */
    uint16_t tnr_lo_wgt_cal_offset;
    uint16_t tnr_lo_wgt_cal_scale;
    /* BAY3D_TNRLFSCLTH */
    uint16_t tnr_low_wgt_cal_max_limit;
    uint16_t tnr_mode0_base_ratio;
    /* BAY3D_TNRDSWGTSCL */
    uint16_t tnr_lo_diff_wgt_cal_offset;
    uint16_t tnr_lo_diff_wgt_cal_scale;
    /* BAY3D_TNRWLSTSCL */
    uint16_t tnr_lo_mge_pre_wgt_offset;
    uint16_t tnr_lo_mge_pre_wgt_scale;
    /* BAY3D_TNRWGT0SCL0 */
    uint16_t tnr_mode0_lo_wgt_scale;
    uint16_t tnr_mode0_lo_wgt_hdr_sht_scale;
    /* BAY3D_TNRWGT1SCL1 */
    uint16_t tnr_mode1_lo_wgt_scale;
    uint16_t tnr_mode1_lo_wgt_hdr_sht_scale;
    /* BAY3D_TNRWGT1SCL2 */
    uint16_t tnr_mode1_wgt_scale;
    uint16_t tnr_mode1_wgt_hdr_sht_scale;
    /* BAY3D_TNRWGTOFF */
    uint16_t tnr_mode1_lo_wgt_offset;
    uint16_t tnr_mode1_lo_wgt_hdr_sht_offset;
    /* BAY3D_TNRWGT1OFF */
    uint16_t tnr_auto_sigma_count_wgt_thred;
    uint16_t tnr_mode1_wgt_min_limit;
    uint16_t tnr_mode1_wgt_offset;
    /* BAY3D_TNRSIGORG */
    uint32_t tnr_out_sigma_sq;
    /* BAY3D_TNRWLO_THL */
    uint16_t tnr_lo_wgt_clip_min_limit;
    uint16_t tnr_lo_wgt_clip_hdr_sht_min_limit;
    /* BAY3D_TNRWLO_THH */
    uint16_t tnr_lo_wgt_clip_max_limit;
    uint16_t tnr_lo_wgt_clip_hdr_sht_max_limit;
    /* BAY3D_TNRWHI_THL */
    uint16_t tnr_hi_wgt_clip_min_limit;
    uint16_t tnr_hi_wgt_clip_hdr_sht_min_limit;
    /* BAY3D_TNRWHI_THH */
    uint16_t tnr_hi_wgt_clip_max_limit;
    uint16_t tnr_hi_wgt_clip_hdr_sht_max_limit;
    /* BAY3D_TNRKEEP */
    uint8_t tnr_cur_spnr_hi_wgt_min_limit;
    uint8_t tnr_pre_spnr_hi_wgt_min_limit;
    /* BAY3D_PIXMAX */
    uint16_t tnr_pix_max;
    uint8_t hw_bay3d_lowgt_ctrl;
    uint16_t hw_bay3d_lowgt_offinit;

    /* BAY3D_SIGNUMTH */
    uint32_t tnr_auto_sigma_count_th;
    /* BAY3D_SIGSUM */
    uint32_t tnr_Auto_Sigma_Count;
    /* BAY3D_TNRSIGYO */
    uint16_t tnr_auto_sigma_calc[20];

    /* BAY3D_TNRMO_STR */
    uint16_t tnr_motion_nr_strg;
    uint8_t tnr_gain_max;
} RK_Bayertnr_Fix_V30_t;



#endif



