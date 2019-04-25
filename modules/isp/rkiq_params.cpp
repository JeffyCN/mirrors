/*
 * Copyright 2017 Rockchip Electronics Co., Ltd
 *     Author: Jacob Chen <jacob2.chen@rock-chips.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include "rkiq_params.h"

struct AiqResults mLastAiqResults = {0};
struct rkisp_parameters mLastRkiqResults = {0};
static int g_isp_acq_out_width = -1;
static int g_isp_acq_out_height = -1;

static int dpcc_param_check(struct cifisp_dpcc_config* arg, int isp_ver)
{
    unsigned int i = 0;
    struct cifisp_dpcc_methods_config* method;
    unsigned int mode_max = arg->mode & ~CIF_ISP_DPCC_MODE_MEM_UPDATE_V12;
    // ISP v12 dpcc uses some more bits which are not used in isp v10, and
    // ISP V12 supports two dpcc modes, including a new one, and the old one
    // which is the same as V10. Because the dpcc params defined in tuning xml
    // use the V12's format for the reason compatible with V10, so we just check
    // the parpams with V12 here.

    if (mode_max > CIF_ISP_DPCC_MODE_MAX ||
        arg->output_mode > CIF_ISP_DPCC_OUTPUTMODE_MAX_V12 ||
        arg->set_use > CIF_ISP_DPCC_SETUSE_MAX) {
        LOGE("%s:%d check error !", __FUNCTION__, __LINE__);
        goto err;
    }
    if (arg->ro_limits & CIF_ISP_DPCC_RO_LIMIT_RESERVED || arg->rnd_offs & CIF_ISP_DPCC_RND_OFFS_RESERVED) {
        LOGE("%s:%d check error !", __FUNCTION__, __LINE__);
        goto err;
    }
    method = &arg->methods[i];
    for (i = 0; i < CIFISP_DPCC_METHODS_MAX; i++) {
        if ((method->method & CIF_ISP_DPCC_METHODS_SET_RESERVED_V12) ||
            (method->line_thresh & CIF_ISP_DPCC_LINE_THRESH_RESERVED_V12) ||
            (method->line_mad_fac & CIF_ISP_DPCC_LINE_MAD_FAC_RESERVED)) {
            LOGE("%s:%d check error !", __FUNCTION__, __LINE__);
            goto err;
        }
        if ((method->pg_fac & CIF_ISP_DPCC_PG_FAC_RESERVED) ||
            (method->rnd_thresh & CIF_ISP_DPCC_RND_THRESH_RESERVED) ||
            (method->rg_fac & CIF_ISP_DPCC_RG_FAC_RESERVED)) {
            LOGE("%s:%d check error !", __FUNCTION__, __LINE__);
            goto err;
        }
        method++;
    }

    return 0;
err:
    return -1;
}

static int bls_param_check(struct cifisp_bls_config* arg)
{
    if (!BLS_START_H_MAX_IS_VALID(arg->bls_window1.h_offs) || !BLS_STOP_H_MAX_IS_VALID(arg->bls_window1.h_size) || !BLS_START_V_MAX_IS_VALID(arg->bls_window1.v_offs) || !BLS_STOP_V_MAX_IS_VALID(arg->bls_window1.v_size)) {
        LOGE("%s:%d check error !", __FUNCTION__, __LINE__);
        goto err;
    }
    if (!BLS_START_H_MAX_IS_VALID(arg->bls_window2.h_offs) || !BLS_STOP_H_MAX_IS_VALID(arg->bls_window2.h_size) || !BLS_START_V_MAX_IS_VALID(arg->bls_window2.v_offs) || !BLS_STOP_V_MAX_IS_VALID(arg->bls_window2.v_size)) {
        LOGE("%s:%d check error !", __FUNCTION__, __LINE__);
        goto err;
    }
    if (!BLS_SAMPLE_MAX_IS_VALID(arg->bls_samples)) {
        LOGE("%s:%d check error !", __FUNCTION__, __LINE__);
        goto err;
    }
    if (!BLS_FIX_SUB_IS_VALID(arg->fixed_val.r) || !BLS_FIX_SUB_IS_VALID(arg->fixed_val.gr) || !BLS_FIX_SUB_IS_VALID(arg->fixed_val.gb) || !BLS_FIX_SUB_IS_VALID(arg->fixed_val.b)) {
        LOGE("%s:%d check error !", __FUNCTION__, __LINE__);
        goto err;
    }
    return 0;
err:
    return -1;
}

static int sdg_param_check(struct cifisp_sdg_config* arg)
{
    int i;
    if (arg->xa_pnts.gamma_dx0 & CIFISP_DEGAMMA_X_RESERVED || arg->xa_pnts.gamma_dx1 & CIFISP_DEGAMMA_X_RESERVED) {
        LOGE("%s:%d check error !", __FUNCTION__, __LINE__);
        return -1;
    }
    for (i = 0; i < CIFISP_DEGAMMA_CURVE_SIZE; i++) {
        if ((arg->curve_b.gamma_y[i] & CIFISP_DEGAMMA_Y_RESERVED) || (arg->curve_r.gamma_y[i] & CIFISP_DEGAMMA_Y_RESERVED) || (arg->curve_g.gamma_y[i] & CIFISP_DEGAMMA_Y_RESERVED)) {
            LOGE("%s:%d check error !", __FUNCTION__, __LINE__);
            return -1;
        }
    }
    return 0;
}

static int lsc_param_check(struct cifisp_lsc_config* arg, int isp_ver)
{
    int i;
    for (i = 0; i < CIFISP_LSC_SIZE_TBL_SIZE; i++) {
        if ((arg->x_size_tbl[i] & CIF_ISP_LSC_SECT_SIZE_RESERVED) || (arg->y_size_tbl[i] & CIF_ISP_LSC_SECT_SIZE_RESERVED)) {
            LOGE("%s:%d check error !", __FUNCTION__, __LINE__);
            return -1;
        }
    }
    for (i = 0; i < CIFISP_LSC_GRAD_TBL_SIZE; i++) {
        if (isp_ver == 0) {
            if ((arg->x_grad_tbl[i] & CIF_ISP_LSC_GRAD_RESERVED_V10) || (arg->y_grad_tbl[i] & CIF_ISP_LSC_GRAD_RESERVED_V10)) {
                LOGE("%s:%d check error !", __FUNCTION__, __LINE__);
                return -1;
            }
        } else {
            if ((arg->x_grad_tbl[i] & CIF_ISP_LSC_GRAD_RESERVED_V12) || (arg->y_grad_tbl[i] & CIF_ISP_LSC_GRAD_RESERVED_V12)) {
                LOGE("%s:%d check error !", __FUNCTION__, __LINE__);
                return -1;
            }
        }
    }
    for (i = 0; i < CIFISP_LSC_DATA_TBL_SIZE; i++) {
        if (isp_ver == 0) {
            if ((arg->r_data_tbl[i] & CIF_ISP_LSC_SAMPLE_RESERVED_V10) || (arg->gr_data_tbl[i] & CIF_ISP_LSC_SAMPLE_RESERVED_V10) ||
                (arg->gb_data_tbl[i] & CIF_ISP_LSC_SAMPLE_RESERVED_V10) || (arg->b_data_tbl[i] & CIF_ISP_LSC_SAMPLE_RESERVED_V10)) {
                LOGE("%s:%d check error !", __FUNCTION__, __LINE__);
                return -1;
            }
        } else {
            if ((arg->r_data_tbl[i] & CIF_ISP_LSC_SAMPLE_RESERVED_V12) || (arg->gr_data_tbl[i] & CIF_ISP_LSC_SAMPLE_RESERVED_V12) ||
                (arg->gb_data_tbl[i] & CIF_ISP_LSC_SAMPLE_RESERVED_V12) || (arg->b_data_tbl[i] & CIF_ISP_LSC_SAMPLE_RESERVED_V12)) {
                LOGE("%s:%d check error !", __FUNCTION__, __LINE__);
                return -1;
            }
        }
    }
    return 0;
}

static int awb_gain_param_check(struct cifisp_awb_gain_config* arg)
{
    if (arg->gain_red > CIF_ISP_AWB_GAINS_MAX_VAL || arg->gain_green_r > CIF_ISP_AWB_GAINS_MAX_VAL || arg->gain_green_b > CIF_ISP_AWB_GAINS_MAX_VAL || arg->gain_blue > CIF_ISP_AWB_GAINS_MAX_VAL) {
        LOGE("%s:%d check error !", __FUNCTION__, __LINE__);
        return -1;
    }

    if (arg->gain_red == 0 || arg->gain_green_r == 0 || arg->gain_green_b == 0 || arg->gain_blue == 0) {
        LOGE("awb gain %d,%d,%d,%d",
             arg->gain_red,arg->gain_green_r, arg->gain_green_b,arg->gain_blue);
        LOGE("%s:%d check error !", __FUNCTION__, __LINE__);
        return -1;
    }

    return 0;
}

static int flt_param_check(struct cifisp_flt_config* arg)
{
    if (arg->mode > CIF_ISP_FLT_MODE_MAX || arg->grn_stage1 > CIF_ISP_FLT_GREEN_STAGE1_MAX || arg->chr_v_mode > CIF_ISP_FLT_CHROMA_MODE_MAX || arg->chr_h_mode > CIF_ISP_FLT_CHROMA_MODE_MAX || arg->thresh_sh0 & CIF_ISP_FLT_THREAD_RESERVED || arg->thresh_sh1 & CIF_ISP_FLT_THREAD_RESERVED || arg->thresh_bl0 & CIF_ISP_FLT_THREAD_RESERVED || arg->thresh_bl1 & CIF_ISP_FLT_THREAD_RESERVED || arg->fac_bl0 & CIF_ISP_FLT_FAC_RESERVED || arg->fac_bl1 & CIF_ISP_FLT_FAC_RESERVED || arg->fac_sh0 & CIF_ISP_FLT_FAC_RESERVED || arg->fac_sh1 & CIF_ISP_FLT_FAC_RESERVED || arg->fac_mid & CIF_ISP_FLT_FAC_RESERVED || arg->lum_weight & CIF_ISP_FLT_LUM_WEIGHT_RESERVED) {
        LOGE("%s:%d check error !", __FUNCTION__, __LINE__);
        return -1;
    }
    return 0;
}

static int ctk_param_check(struct cifisp_ctk_config* arg)
{
    if (arg->coeff0 & CIF_ISP_CTK_COEFF_RESERVED || arg->coeff1 & CIF_ISP_CTK_COEFF_RESERVED || arg->coeff2 & CIF_ISP_CTK_COEFF_RESERVED || arg->coeff3 & CIF_ISP_CTK_COEFF_RESERVED || arg->coeff4 & CIF_ISP_CTK_COEFF_RESERVED || arg->coeff5 & CIF_ISP_CTK_COEFF_RESERVED || arg->coeff6 & CIF_ISP_CTK_COEFF_RESERVED || arg->coeff7 & CIF_ISP_CTK_COEFF_RESERVED || arg->coeff8 & CIF_ISP_CTK_COEFF_RESERVED || arg->ct_offset_r & CIF_ISP_XTALK_OFFSET_RESERVED || arg->ct_offset_g & CIF_ISP_XTALK_OFFSET_RESERVED || arg->ct_offset_b & CIF_ISP_XTALK_OFFSET_RESERVED) {
        LOGE("%s:%d check error !", __FUNCTION__, __LINE__);
        return -1;
    }
    return 0;
}

static int goc_param_check(struct cifisp_goc_config* arg)
{
    if (arg->mode > CIF_ISP_GOC_MODE_MAX) {
        LOGE("%s:%d check error !", __FUNCTION__, __LINE__);
        return -1;
    }
    return 0;
}

static int cproc_param_check(struct cifisp_cproc_config* arg)
{
    if (arg->c_out_range & CIF_C_PROC_CTRL_RESERVED || arg->y_out_range & CIF_C_PROC_CTRL_RESERVED || arg->y_in_range & CIF_C_PROC_CTRL_RESERVED || arg->contrast & CIF_C_PROC_CONTRAST_RESERVED || arg->brightness & CIF_C_PROC_BRIGHTNESS_RESERVED || arg->sat & CIF_C_PROC_SATURATION_RESERVED || arg->hue & CIF_C_PROC_HUE_RESERVED) {
        LOGE("%s:%d check error !", __FUNCTION__, __LINE__);
        return -1;
    }
    return 0;
}

static int ie_param_check(struct cifisp_ie_config* arg)
{
    switch (arg->effect) {
    case V4L2_COLORFX_NONE:
    case V4L2_COLORFX_BW:
    case V4L2_COLORFX_SEPIA:
    case V4L2_COLORFX_NEGATIVE:
    case V4L2_COLORFX_EMBOSS:
    case V4L2_COLORFX_SKETCH:
    case V4L2_COLORFX_AQUA:
    case V4L2_COLORFX_SET_CBCR:
        return 0;
    default:
        LOGE("%s:%d check error !", __FUNCTION__, __LINE__);
        return -1;
    }
}

static int dpf_param_check(struct cifisp_dpf_config* arg)
{
    int retval = 0;
    int i;
    /* Parameter check */
    if (arg->gain.mode >= CIFISP_DPF_GAIN_USAGE_MAX || arg->gain.mode < CIFISP_DPF_GAIN_USAGE_DISABLED || arg->gain.nf_b_gain & CIF_ISP_DPF_NF_GAIN_RESERVED || arg->gain.nf_r_gain & CIF_ISP_DPF_NF_GAIN_RESERVED || arg->gain.nf_gr_gain & CIF_ISP_DPF_NF_GAIN_RESERVED || arg->gain.nf_gb_gain & CIF_ISP_DPF_NF_GAIN_RESERVED) {
        LOGE("%s:%d check error !", __FUNCTION__, __LINE__);
        retval = -1;
        goto end;
    }
    for (i = 0; i < CIFISP_DPF_MAX_SPATIAL_COEFFS; i++) {
        if (arg->g_flt.spatial_coeff[i] > CIF_ISP_DPF_SPATIAL_COEFF_MAX) {
            LOGE("%s:%d check error !", __FUNCTION__, __LINE__);
            retval = -1;
            goto end;
        }
        if (arg->rb_flt.spatial_coeff[i] > CIF_ISP_DPF_SPATIAL_COEFF_MAX) {
            LOGE("%s:%d check error !", __FUNCTION__, __LINE__);
            retval = -1;
            goto end;
        }
    }
    if (arg->rb_flt.fltsize != CIFISP_DPF_RB_FILTERSIZE_9x9 && arg->rb_flt.fltsize != CIFISP_DPF_RB_FILTERSIZE_13x9) {
        LOGE("%s:%d check error !", __FUNCTION__, __LINE__);
        retval = -1;
        goto end;
    }
    for (i = 0; i < CIFISP_DPF_MAX_NLF_COEFFS; i++) {
        if (arg->nll.coeff[i] > CIF_ISP_DPF_NLL_COEFF_N_MAX) {
            LOGE("%s:%d check error !", __FUNCTION__, __LINE__);
            retval = -1;
            goto end;
        }
    }
    if (arg->nll.scale_mode != CIFISP_NLL_SCALE_LINEAR && arg->nll.scale_mode != CIFISP_NLL_SCALE_LOGARITHMIC) {
        LOGE("%s:%d check error !", __FUNCTION__, __LINE__);
        retval = -1;
        goto end;
    }
end:
    return retval;
}

static int awb_meas_param_check(struct cifisp_awb_meas_config* arg)
{
    if (arg->awb_mode > CIFISP_AWB_MODE_YCBCR || arg->awb_wnd.h_offs > CIF_ISP_AWB_WINDOW_OFFSET_MAX || arg->awb_wnd.v_offs > CIF_ISP_AWB_WINDOW_OFFSET_MAX || arg->awb_wnd.h_size > CIF_ISP_AWB_WINDOW_MAX_SIZE || arg->awb_wnd.v_size > CIF_ISP_AWB_WINDOW_MAX_SIZE || arg->frames > CIFISP_AWB_MAX_FRAMES) {
        LOGE("%s:%d check error !", __FUNCTION__, __LINE__);
        return -1;
    }
    return 0;
}

static int afm_param_check(struct cifisp_afc_config* arg)
{
    int i;
    if (arg->num_afm_win > CIFISP_AFM_MAX_WINDOWS || arg->thres & CIF_ISP_AFM_THRES_RESERVED || arg->var_shift & CIF_ISP_AFM_VAR_SHIFT_RESERVED) {
        LOGE("%s:%d check error !", __FUNCTION__, __LINE__);
        return -1;
    }
    for (i = 0; i < arg->num_afm_win; i++) {
        if (arg->afm_win[i].h_offs & CIF_ISP_AFM_WINDOW_X_RESERVED ||
            arg->afm_win[i].h_offs < CIF_ISP_AFM_WINDOW_X_MIN ||
            arg->afm_win[i].v_offs & CIF_ISP_AFM_WINDOW_Y_RESERVED ||
            arg->afm_win[i].v_offs < CIF_ISP_AFM_WINDOW_Y_MIN ||
            arg->afm_win[i].h_size & CIF_ISP_AFM_WINDOW_X_RESERVED ||
            arg->afm_win[i].v_size & CIF_ISP_AFM_WINDOW_Y_RESERVED ||
            (arg->afm_win[i].h_size + arg->afm_win[i].h_offs > g_isp_acq_out_width) ||
            (arg->afm_win[i].v_size + arg->afm_win[i].v_offs > g_isp_acq_out_height - 3)) {
            LOGW("%s:%d check error, fit to limits !", __FUNCTION__, __LINE__);
            arg->afm_win[i].h_offs = arg->afm_win[i].h_offs < CIF_ISP_AFM_WINDOW_X_MIN ?
                CIF_ISP_AFM_WINDOW_X_MIN : arg->afm_win[i].h_offs;
            arg->afm_win[i].v_offs = arg->afm_win[i].v_offs < CIF_ISP_AFM_WINDOW_Y_MIN ?
                CIF_ISP_AFM_WINDOW_Y_MIN : arg->afm_win[i].v_offs;

            if (arg->afm_win[i].h_size + arg->afm_win[i].h_offs > g_isp_acq_out_width)
                arg->afm_win[i].h_size = g_isp_acq_out_width - arg->afm_win[i].h_offs;
            if (arg->afm_win[i].v_size + arg->afm_win[i].v_offs > g_isp_acq_out_height - 3)
                arg->afm_win[i].v_size = g_isp_acq_out_height - 3 - arg->afm_win[i].v_offs;

            return 0;
        }
    }
    return 0;
}

static int hst_param_check(struct cifisp_hst_config* arg, int isp_ver)
{
    int i;
    if (isp_ver == 0) {
        if (arg->mode > CIFISP_HISTOGRAM_MODE_Y_HISTOGRAM ||
            arg->histogram_predivider > CIF_ISP_MAX_HIST_PREDIVIDER_V10 ||
            arg->meas_window.v_offs & CIF_ISP_HIST_WINDOW_OFFSET_RESERVED_V10 ||
            arg->meas_window.h_offs & CIF_ISP_HIST_WINDOW_OFFSET_RESERVED_V10 ||
            (arg->meas_window.v_size / (CIF_ISP_HIST_ROW_NUM_V10 - 1)) & CIF_ISP_HIST_WINDOW_SIZE_RESERVED_V10 ||
            (arg->meas_window.h_size / (CIF_ISP_HIST_COLUMN_NUM_V10 - 1)) & CIF_ISP_HIST_WINDOW_SIZE_RESERVED_V10 ||
            arg->meas_window.v_offs + arg->meas_window.v_size > g_isp_acq_out_height ||
            arg->meas_window.h_offs + arg->meas_window.h_size > g_isp_acq_out_width) {
            LOGW("%s:%d check error, fit to limits !", __FUNCTION__, __LINE__);
            if (arg->meas_window.v_offs + arg->meas_window.v_size > g_isp_acq_out_height)
                arg->meas_window.v_size = g_isp_acq_out_height - arg->meas_window.v_offs;
            if (arg->meas_window.h_offs + arg->meas_window.h_size > g_isp_acq_out_width)
                arg->meas_window.h_size = g_isp_acq_out_width - arg->meas_window.h_offs;
            return 0;
        }
        for (i = 0; i < 25; i++) {
            if (arg->hist_weight[i] & CIF_ISP_HIST_WEIGHT_RESERVED_V10) {
                LOGE("%s:%d check error !", __FUNCTION__, __LINE__);
                return -1;
            }
        }
    } else {
        if (arg->mode > CIFISP_HISTOGRAM_MODE_Y_HISTOGRAM ||
            arg->histogram_predivider > CIF_ISP_MAX_HIST_PREDIVIDER_V10 ||
            arg->meas_window.v_offs & CIF_ISP_HIST_WINDOW_OFFSET_RESERVED_V10 ||
            arg->meas_window.h_offs & CIF_ISP_HIST_WINDOW_OFFSET_RESERVED_V10 ||
            (arg->meas_window.v_size / (CIF_ISP_HIST_ROW_NUM_V12 - 1)) & CIF_ISP_HIST_WINDOW_SIZE_RESERVED_V10 ||
            (arg->meas_window.h_size / (CIF_ISP_HIST_COLUMN_NUM_V12 - 1)) & CIF_ISP_HIST_WINDOW_SIZE_RESERVED_V10 ||
            arg->meas_window.v_offs + arg->meas_window.v_size > g_isp_acq_out_height ||
            arg->meas_window.h_offs + arg->meas_window.h_size > g_isp_acq_out_width) {
            LOGW("%s:%d check error, fit to limits !", __FUNCTION__, __LINE__);
            if (arg->meas_window.v_offs + arg->meas_window.v_size > g_isp_acq_out_height)
                arg->meas_window.v_size = g_isp_acq_out_height - arg->meas_window.v_offs;
            if (arg->meas_window.h_offs + arg->meas_window.h_size > g_isp_acq_out_width)
                arg->meas_window.h_size = g_isp_acq_out_width - arg->meas_window.h_offs;
            return -1;
        }
        for (i = 0; i < 81; i++) {
            if (arg->hist_weight[i] & CIF_ISP_HIST_WEIGHT_RESERVED_V12) {
                LOGE("%s:%d check error !", __FUNCTION__, __LINE__);
                return -1;
            }
        }
    }
    return 0;
}

static int aec_param_check(struct cifisp_aec_config* arg, int isp_ver)
{
    if (isp_ver == 0) {
        if (arg->meas_window.h_offs > CIF_ISP_EXP_MAX_HOFFS_V10 ||
            arg->meas_window.h_size > CIF_ISP_EXP_MAX_HSIZE_V10 ||
            arg->meas_window.h_size < CIF_ISP_EXP_MIN_HSIZE_V10 ||
            arg->meas_window.v_offs > CIF_ISP_EXP_MAX_VOFFS_V10 ||
            arg->meas_window.v_size > CIF_ISP_EXP_MAX_VSIZE_V10 ||
            arg->meas_window.v_size < CIF_ISP_EXP_MIN_VSIZE_V10 ||
            arg->meas_window.h_size + arg->meas_window.h_offs > g_isp_acq_out_width - 1 ||
            arg->meas_window.v_size + arg->meas_window.v_offs > g_isp_acq_out_height - 3 ||
            arg->mode > CIFISP_EXP_MEASURING_MODE_1) {
            LOGW("aec meas win %dx%d(%dx%d)",
                 arg->meas_window.h_size, arg->meas_window.v_size,
                 arg->meas_window.h_offs, arg->meas_window.v_offs
                 );
            LOGW("%s:%d check error, use limit value !", __FUNCTION__, __LINE__);
            // AE regions when zoom may not meet the limitation,
            // so just set the regions to the limitation
            arg->meas_window.h_size = arg->meas_window.h_size >
                CIF_ISP_EXP_MIN_HSIZE_V10 ? arg->meas_window.h_size : CIF_ISP_EXP_MIN_HSIZE_V10;
            arg->meas_window.v_size = arg->meas_window.v_size >
                CIF_ISP_EXP_MIN_VSIZE_V10 ? arg->meas_window.v_size : CIF_ISP_EXP_MIN_VSIZE_V10;

            arg->meas_window.h_size = arg->meas_window.h_size <
                CIF_ISP_EXP_MAX_HSIZE_V10 ? arg->meas_window.h_size : CIF_ISP_EXP_MAX_HSIZE_V10;
            arg->meas_window.v_size = arg->meas_window.v_size <
                CIF_ISP_EXP_MAX_VSIZE_V10 ? arg->meas_window.v_size : CIF_ISP_EXP_MAX_VSIZE_V10;
            if (arg->meas_window.h_size + arg->meas_window.h_offs > g_isp_acq_out_width - 1)
                arg->meas_window.h_size = g_isp_acq_out_width - 1 - arg->meas_window.h_offs;
            if (arg->meas_window.v_size + arg->meas_window.v_offs > g_isp_acq_out_height - 3)
                arg->meas_window.v_size = g_isp_acq_out_height - 3 - arg->meas_window.v_offs;
        }
    } else {
        if (arg->meas_window.h_offs > CIF_ISP_EXP_MAX_HOFFS_V12 ||
            arg->meas_window.h_size > CIF_ISP_EXP_MAX_HSIZE_V12 ||
            arg->meas_window.h_size < CIF_ISP_EXP_MIN_HSIZE_V12 ||
            arg->meas_window.v_offs > CIF_ISP_EXP_MAX_VOFFS_V12 ||
            arg->meas_window.v_size > CIF_ISP_EXP_MAX_VSIZE_V12 ||
            arg->meas_window.v_size < CIF_ISP_EXP_MIN_VSIZE_V12 ||
            arg->meas_window.h_size + arg->meas_window.h_offs > g_isp_acq_out_width - 1 ||
            arg->meas_window.v_size + arg->meas_window.v_offs > g_isp_acq_out_height - 3 ||
            arg->mode > CIFISP_EXP_MEASURING_MODE_1) {
            LOGW("aec meas win %dx%d(%dx%d)",
                 arg->meas_window.h_size, arg->meas_window.v_size,
                 arg->meas_window.h_offs, arg->meas_window.v_offs
                 );
            LOGW("%s:%d check error, use limit value !", __FUNCTION__, __LINE__);
            // AE regions when zoom may not meet the limitation,
            // so just set the regions to the limitation
            arg->meas_window.h_size = arg->meas_window.h_size >
                CIF_ISP_EXP_MIN_HSIZE_V12 ? arg->meas_window.h_size : CIF_ISP_EXP_MIN_HSIZE_V12;
            arg->meas_window.v_size = arg->meas_window.v_size >
                CIF_ISP_EXP_MIN_VSIZE_V12 ? arg->meas_window.v_size : CIF_ISP_EXP_MIN_VSIZE_V12;

            arg->meas_window.h_size = arg->meas_window.h_size <
                CIF_ISP_EXP_MAX_HSIZE_V12 ? arg->meas_window.h_size : CIF_ISP_EXP_MAX_HSIZE_V12;
            arg->meas_window.v_size = arg->meas_window.v_size <
                CIF_ISP_EXP_MAX_VSIZE_V12 ? arg->meas_window.v_size : CIF_ISP_EXP_MAX_VSIZE_V12;
            if (arg->meas_window.h_size + arg->meas_window.h_offs > g_isp_acq_out_width - 1)
                arg->meas_window.h_size = g_isp_acq_out_width - 1 - arg->meas_window.h_offs;
            if (arg->meas_window.v_size + arg->meas_window.v_offs > g_isp_acq_out_height - 3)
                arg->meas_window.v_size = g_isp_acq_out_height - 3 - arg->meas_window.v_offs;
        }
    }
    return 0;
}

static int demosaiclp_param_check(struct cifisp_demosaiclp_config* arg, int isp_ver)
{
    // TODO
    return 0;
}

static int wdr_param_check(struct cifisp_wdr_config* arg, int isp_ver)
{
    // TODO
    return 0;
}

static int rkiesharp_param_check(struct cifisp_rkiesharp_config* arg, int isp_ver)
{
    // TODO
    return 0;
}


XCamReturn rkisp1_check_params(struct rkisp1_isp_params_cfg* configs,
                               int isp_acq_out_width, int isp_acq_out_height,
                               int isp_ver)
{
    int ret = 0;

    g_isp_acq_out_width = isp_acq_out_width;
    g_isp_acq_out_height = isp_acq_out_height;

    if (configs->module_cfg_update & CIFISP_MODULE_DPCC) {
        ret = dpcc_param_check(&configs->others.dpcc_config, isp_ver);
        if (ret < 0)
            configs->module_cfg_update &= ~CIFISP_MODULE_DPCC;
    }
    if (configs->module_cfg_update & CIFISP_MODULE_BLS) {
        ret = bls_param_check(&configs->others.bls_config);
        if (ret < 0)
            configs->module_cfg_update &= ~CIFISP_MODULE_BLS;
    }
    if (configs->module_cfg_update & CIFISP_MODULE_SDG) {
        ret = sdg_param_check(&configs->others.sdg_config);
        if (ret < 0)
            configs->module_cfg_update &= ~CIFISP_MODULE_SDG;
    }
    if (configs->module_cfg_update & CIFISP_MODULE_LSC) {
        ret = lsc_param_check(&configs->others.lsc_config, isp_ver);
        if (ret < 0)
            configs->module_cfg_update &= ~CIFISP_MODULE_LSC;
    }
    if (configs->module_cfg_update & CIFISP_MODULE_AWB_GAIN) {
        ret = awb_gain_param_check(&configs->others.awb_gain_config);
        if (ret < 0)
            configs->module_cfg_update &= ~CIFISP_MODULE_AWB_GAIN;
    }
    if (configs->module_cfg_update & CIFISP_MODULE_FLT) {
        ret = flt_param_check(&configs->others.flt_config);
        if (ret < 0)
            configs->module_cfg_update &= ~CIFISP_MODULE_FLT;
    }
    if (configs->module_cfg_update & CIFISP_MODULE_CTK) {
        ret = ctk_param_check(&configs->others.ctk_config);
        if (ret < 0)
            configs->module_cfg_update &= ~CIFISP_MODULE_CTK;
    }
    if (configs->module_cfg_update & CIFISP_MODULE_GOC) {
        ret = goc_param_check(&configs->others.goc_config);
        if (ret < 0)
            configs->module_cfg_update &= ~CIFISP_MODULE_GOC;
    }
    if (configs->module_cfg_update & CIFISP_MODULE_CPROC) {
        ret = cproc_param_check(&configs->others.cproc_config);
        if (ret < 0)
            configs->module_cfg_update &= ~CIFISP_MODULE_CPROC;
    }
    if (configs->module_cfg_update & CIFISP_MODULE_IE) {
        ret = ie_param_check(&configs->others.ie_config);
        if (ret < 0)
            configs->module_cfg_update &= ~CIFISP_MODULE_IE;
    }
    if (configs->module_cfg_update & CIFISP_MODULE_DPF) {
        ret = dpf_param_check(&configs->others.dpf_config);
        if (ret < 0)
            configs->module_cfg_update &= ~CIFISP_MODULE_DPF;
    }
    if (configs->module_cfg_update & CIFISP_MODULE_AWB) {
        ret = awb_meas_param_check(&configs->meas.awb_meas_config);
        if (ret < 0)
            configs->module_cfg_update &= ~CIFISP_MODULE_AWB;
    }
    if (configs->module_cfg_update & CIFISP_MODULE_AFC) {
        ret = afm_param_check(&configs->meas.afc_config);
        if (ret < 0)
            configs->module_cfg_update &= ~CIFISP_MODULE_AFC;
    }
    if (configs->module_cfg_update & CIFISP_MODULE_HST) {
        ret = hst_param_check(&configs->meas.hst_config, isp_ver);
        if (ret < 0)
            configs->module_cfg_update &= ~CIFISP_MODULE_HST;
    }
    if (configs->module_cfg_update & CIFISP_MODULE_AEC) {
        ret = aec_param_check(&configs->meas.aec_config, isp_ver);
        if (ret < 0)
            configs->module_cfg_update &= ~CIFISP_MODULE_AEC;
    }

    if (configs->module_cfg_update & CIFISP_MODULE_DEMOSAICLP) {
        ret = demosaiclp_param_check(&configs->others.demosaiclp_config, isp_ver);
        if (ret < 0)
            configs->module_cfg_update &= ~CIFISP_MODULE_DEMOSAICLP;
    }

    if (configs->module_cfg_update & CIFISP_MODULE_RK_IESHARP) {
        ret = rkiesharp_param_check(&configs->others.rkiesharp_config, isp_ver);
        if (ret < 0)
            configs->module_cfg_update &= ~CIFISP_MODULE_RK_IESHARP;
    }

    if (configs->module_cfg_update & CIFISP_MODULE_WDR) {
        ret = wdr_param_check(&configs->others.wdr_config, isp_ver);
        if (ret < 0)
            configs->module_cfg_update &= ~CIFISP_MODULE_WDR;
    }

    if (ret < 0) {
        return XCAM_RETURN_ERROR_AIQ;
    }

    return XCAM_RETURN_NO_ERROR;
}

static void rkisp1_params_convertDPCC(struct rkisp1_isp_params_cfg* configs,
    struct cifisp_dpcc_config* dpcc_config, rk_aiq_dpcc_config* aiq_dpcc_config)
{
    if (aiq_dpcc_config->enabled != mLastAiqResults.miscIspResults.dpcc_config.enabled)
        configs->module_en_update |= HAL_ISP_BPC_MASK;
    configs->module_cfg_update |= HAL_ISP_BPC_MASK;
    configs->module_ens |= (aiq_dpcc_config->enabled ? HAL_ISP_BPC_MASK : 0);

    dpcc_config->mode = aiq_dpcc_config->isp_dpcc_mode;
    dpcc_config->output_mode = aiq_dpcc_config->isp_dpcc_output_mode;
    dpcc_config->set_use = aiq_dpcc_config->isp_dpcc_set_use;
    dpcc_config->ro_limits = aiq_dpcc_config->isp_dpcc_ro_limits;
    dpcc_config->rnd_offs = aiq_dpcc_config->isp_dpcc_rnd_offs;
    dpcc_config->methods[0].method = aiq_dpcc_config->isp_dpcc_methods_set_1;
    dpcc_config->methods[0].line_thresh = aiq_dpcc_config->isp_dpcc_line_thresh_1;
    dpcc_config->methods[0].line_mad_fac = aiq_dpcc_config->isp_dpcc_line_mad_fac_1;
    dpcc_config->methods[0].pg_fac = aiq_dpcc_config->isp_dpcc_pg_fac_1;
    dpcc_config->methods[0].rnd_thresh = aiq_dpcc_config->isp_dpcc_rnd_thresh_1;
    dpcc_config->methods[0].rg_fac = aiq_dpcc_config->isp_dpcc_rg_fac_1;

    dpcc_config->methods[1].method = aiq_dpcc_config->isp_dpcc_methods_set_2;
    dpcc_config->methods[1].line_thresh = aiq_dpcc_config->isp_dpcc_line_thresh_2;
    dpcc_config->methods[1].line_mad_fac = aiq_dpcc_config->isp_dpcc_line_mad_fac_2;
    dpcc_config->methods[1].pg_fac = aiq_dpcc_config->isp_dpcc_pg_fac_2;
    dpcc_config->methods[1].rnd_thresh = aiq_dpcc_config->isp_dpcc_rnd_thresh_2;
    dpcc_config->methods[1].rg_fac = aiq_dpcc_config->isp_dpcc_rg_fac_2;

    dpcc_config->methods[2].method = aiq_dpcc_config->isp_dpcc_methods_set_3;
    dpcc_config->methods[2].line_thresh = aiq_dpcc_config->isp_dpcc_line_thresh_3;
    dpcc_config->methods[2].line_mad_fac = aiq_dpcc_config->isp_dpcc_line_mad_fac_3;
    dpcc_config->methods[2].pg_fac = aiq_dpcc_config->isp_dpcc_pg_fac_3;
    dpcc_config->methods[2].rnd_thresh = aiq_dpcc_config->isp_dpcc_rnd_thresh_3;
    dpcc_config->methods[2].rg_fac = aiq_dpcc_config->isp_dpcc_rg_fac_3;
}

static void rkisp1_params_convertBLS(struct rkisp1_isp_params_cfg* configs,
    struct cifisp_bls_config* bls_config, rk_aiq_bls_config* aiq_bls_config)
{
    if (aiq_bls_config->enabled != mLastAiqResults.miscIspResults.bls_config.enabled)
        configs->module_en_update |= HAL_ISP_BLS_MASK;
    configs->module_cfg_update |= HAL_ISP_BLS_MASK;
    configs->module_ens |= (aiq_bls_config->enabled ? HAL_ISP_BLS_MASK : 0);

    bls_config->enable_auto = false; //fx
    bls_config->en_windows = aiq_bls_config->num_win; //fx
    bls_config->bls_window1.h_offs = aiq_bls_config->window1.h_offset;
    bls_config->bls_window1.v_offs = aiq_bls_config->window1.v_offset;
    bls_config->bls_window1.h_size = aiq_bls_config->window1.width;
    bls_config->bls_window1.v_size = aiq_bls_config->window1.height;
    bls_config->bls_window2.h_offs = aiq_bls_config->window2.h_offset;
    bls_config->bls_window2.v_offs = aiq_bls_config->window2.v_offset;
    bls_config->bls_window2.h_size = aiq_bls_config->window2.width;
    bls_config->bls_window2.v_size = aiq_bls_config->window2.height;
    bls_config->bls_samples = 0; //fx

    //fx
    bls_config->fixed_val.r = aiq_bls_config->isp_bls_a_fixed;
    bls_config->fixed_val.gr = aiq_bls_config->isp_bls_b_fixed;
    bls_config->fixed_val.gb = aiq_bls_config->isp_bls_c_fixed;
    bls_config->fixed_val.b = aiq_bls_config->isp_bls_d_fixed;
}

// static void rkisp1_params_convertSDG(struct rkisp1_isp_params_cfg* configs,
//     struct cifisp_sdg_config* sdg_config, rk_aiq_sdg_config* aiq_sdg_config)
// {
//     int i = 0;

//     if (aiq_sdg_config->enabled != mLastAiqResults.miscIspResults.sdg_config.enabled)
//         configs->module_en_update |= HAL_ISP_SDG_MASK;
//     configs->module_cfg_update |= HAL_ISP_SDG_MASK;
//     confgis->module_ens |= (aiq_sdg_config->enabled ? HAL_ISP_SDG_MASK : 0);

//     for (i = 0; i < CIFISP_DEGAMMA_CURVE_SIZE; i++) {
//         sdg_config->curve_r.gamma_y[i] = aiq_sdg_config->red[i];
//     }

//     for (i = 0; i < CIFISP_DEGAMMA_CURVE_SIZE; i++) {
//         sdg_config->curve_g.gamma_y[i] = aiq_sdg_config->green[i];
//     }

//     for (i = 0; i < CIFISP_DEGAMMA_CURVE_SIZE; i++) {
//         sdg_config->curve_b.gamma_y[i] = aiq_sdg_config->blue[i];
//     }

//     sdg_config->xa_pnts.gamma_dx0 = 0;
//     sdg_config->xa_pnts.gamma_dx1 = 0;

//     for (i = 0; i < (CIFISP_DEGAMMA_CURVE_SIZE - 1); i++) {
//         if (i < (CIFISP_DEGAMMA_CURVE_SIZE - 1) / 2)
//             sdg_config->xa_pnts.gamma_dx0 |= (uint32_t)(aiq_sdg_config->segment[i]) << (i * 4);
//         else {
//             int index = i - (CIFISP_DEGAMMA_CURVE_SIZE - 1) / 2;
//             sdg_config->xa_pnts.gamma_dx1 |= (uint32_t)(aiq_sdg_config->segment[i]) << (index * 4);
//         }
//     }
// }

static void rkisp1_params_convertHST(struct rkisp1_isp_params_cfg* configs,
    struct cifisp_hst_config* hst_config, rk_aiq_hist_config* aiq_hst_config)
{
    int i, weight_size;

    if (aiq_hst_config->enabled != mLastAiqResults.aeResults.hist_config_result.enabled)
        configs->module_en_update |= HAL_ISP_HST_MASK;
    configs->module_cfg_update |= HAL_ISP_HST_MASK;
    configs->module_ens |= (aiq_hst_config->enabled ? HAL_ISP_HST_MASK : 0);

    hst_config->histogram_predivider = (unsigned short)aiq_hst_config->stepSize;
    hst_config->meas_window.h_offs = aiq_hst_config->window.h_offset;
    hst_config->meas_window.v_offs = aiq_hst_config->window.v_offset;
    hst_config->meas_window.h_size = aiq_hst_config->window.width;
    hst_config->meas_window.v_size = aiq_hst_config->window.height;

    weight_size = aiq_hst_config->weights_cnt;
    if (weight_size > CIFISP_HISTOGRAM_WEIGHT_GRIDS_SIZE)
        weight_size = CIFISP_HISTOGRAM_WEIGHT_GRIDS_SIZE;
    for (i = 0; i < weight_size; i++) {
        hst_config->hist_weight[i] = aiq_hst_config->weights[i];
    }

    switch (aiq_hst_config->mode) {
    case RK_ISP_HIST_MODE_RGB_COMBINED:
        hst_config->mode = CIFISP_HISTOGRAM_MODE_RGB_COMBINED;
        break;
    case RK_ISP_HIST_MODE_R:
        hst_config->mode = CIFISP_HISTOGRAM_MODE_R_HISTOGRAM;
        break;
    case RK_ISP_HIST_MODE_G:
        hst_config->mode = CIFISP_HISTOGRAM_MODE_G_HISTOGRAM;
        break;
    case RK_ISP_HIST_MODE_B:
        hst_config->mode = CIFISP_HISTOGRAM_MODE_B_HISTOGRAM;
        break;
    case RK_ISP_HIST_MODE_Y:
        hst_config->mode = CIFISP_HISTOGRAM_MODE_Y_HISTOGRAM;
        break;
    case RK_ISP_HIST_MODE_INVALID:
        hst_config->mode = CIFISP_HISTOGRAM_MODE_DISABLE;
        break;
    case RK_ISP_HIST_MODE_MAX:
    default:
        break;
    }
}

static void rkisp1_params_convertLSC(struct rkisp1_isp_params_cfg* configs,
    struct cifisp_lsc_config* lsc_config, rk_aiq_lsc_config* aiq_lsc_config)
{
    int data_tbl_size, grad_tbl_size, size_tbl_size;

    if (aiq_lsc_config->enabled != mLastAiqResults.awbResults.lsc_cfg.enabled)
        configs->module_en_update |= HAL_ISP_LSC_MASK;
    configs->module_cfg_update |= HAL_ISP_LSC_MASK;
    configs->module_ens |= (aiq_lsc_config->enabled ? HAL_ISP_LSC_MASK : 0);

    data_tbl_size = aiq_lsc_config->lsc_config.lsc_data_tbl_cnt;
    if (data_tbl_size > CIFISP_LSC_DATA_TBL_SIZE)
        data_tbl_size = CIFISP_LSC_DATA_TBL_SIZE;
    for (int i = 0; i < data_tbl_size; i++) {
        lsc_config->r_data_tbl[i] = aiq_lsc_config->lsc_config.lsc_r_data_tbl[i];
        lsc_config->gr_data_tbl[i] = aiq_lsc_config->lsc_config.lsc_gr_data_tbl[i];
        lsc_config->gb_data_tbl[i] = aiq_lsc_config->lsc_config.lsc_gb_data_tbl[i];
        lsc_config->b_data_tbl[i] = aiq_lsc_config->lsc_config.lsc_b_data_tbl[i];
    }

    grad_tbl_size = aiq_lsc_config->lsc_config.lsc_grad_tbl_cnt;
    if (grad_tbl_size > CIFISP_LSC_GRAD_TBL_SIZE)
        grad_tbl_size = CIFISP_LSC_GRAD_TBL_SIZE;
    for (int i = 0; i < grad_tbl_size; i++) {
        lsc_config->x_grad_tbl[i] = aiq_lsc_config->lsc_config.lsc_x_grad_tbl[i];
        lsc_config->y_grad_tbl[i] = aiq_lsc_config->lsc_config.lsc_y_grad_tbl[i];
    }

    size_tbl_size = aiq_lsc_config->lsc_config.lsc_size_tbl_cnt;
    if (size_tbl_size > CIFISP_LSC_SIZE_TBL_SIZE)
        size_tbl_size = CIFISP_LSC_SIZE_TBL_SIZE;
    for (int i = 0; i < size_tbl_size; i++) {
        lsc_config->x_size_tbl[i] = aiq_lsc_config->lsc_config.lsc_x_size_tbl[i];
        lsc_config->y_size_tbl[i] = aiq_lsc_config->lsc_config.lsc_y_size_tbl[i];
    }

    lsc_config->config_width = aiq_lsc_config->config_width;
    lsc_config->config_height = aiq_lsc_config->config_height;
}

static void rkisp1_params_convertAWBGain(struct rkisp1_isp_params_cfg* configs,
    struct cifisp_awb_gain_config* awbGain_config, rk_aiq_awb_gain_config* aiq_awbGain_config)
{
    if (aiq_awbGain_config->enabled != mLastAiqResults.awbResults.awb_gain_cfg.enabled)
        configs->module_en_update |= HAL_ISP_AWB_GAIN_MASK;
    configs->module_cfg_update |= HAL_ISP_AWB_GAIN_MASK;
    configs->module_ens |= (aiq_awbGain_config->enabled ? HAL_ISP_AWB_GAIN_MASK : 0);

    awbGain_config->gain_red = aiq_awbGain_config->awb_gains.red_gain;
    awbGain_config->gain_green_r = aiq_awbGain_config->awb_gains.green_r_gain;
    awbGain_config->gain_blue = aiq_awbGain_config->awb_gains.blue_gain;
    awbGain_config->gain_green_b = aiq_awbGain_config->awb_gains.green_b_gain;
}

static void rkisp1_params_convertFLT(struct rkisp1_isp_params_cfg* configs,
    struct cifisp_flt_config* flt_config, rk_aiq_flt_config* aiq_flt_config)
{
    if (aiq_flt_config->enabled != mLastAiqResults.miscIspResults.flt_config.enabled)
        configs->module_en_update |= HAL_ISP_FLT_MASK;
    configs->module_cfg_update |= HAL_ISP_FLT_MASK;
    configs->module_ens |= (aiq_flt_config->enabled ? HAL_ISP_FLT_MASK : 0);

    switch (aiq_flt_config->mode) {
    case RK_ISP_FLT_STATIC_MODE:
        flt_config->mode = CIFISP_FLT_STATIC_MODE;
        break;
    case RK_ISP_FLT_DYNAMIC_MODE:
        flt_config->mode = CIFISP_FLT_DYNAMIC_MODE;
        break;
    default:
        break;
    }

    flt_config->grn_stage1 = aiq_flt_config->grn_stage1;
    flt_config->chr_h_mode = aiq_flt_config->chr_h_mode;
    flt_config->chr_v_mode = aiq_flt_config->chr_v_mode;
    flt_config->thresh_bl0 = aiq_flt_config->thresh_bl0;
    flt_config->thresh_bl1 = aiq_flt_config->thresh_bl1;
    flt_config->thresh_sh0 = aiq_flt_config->thresh_sh0;
    flt_config->thresh_sh1 = aiq_flt_config->thresh_sh1;
    flt_config->lum_weight = aiq_flt_config->lum_weight;
    flt_config->fac_sh1 = aiq_flt_config->fac_sh1;
    flt_config->fac_sh0 = aiq_flt_config->fac_sh0;
    flt_config->fac_mid = aiq_flt_config->fac_mid;
    flt_config->fac_bl0 = aiq_flt_config->fac_bl0;
    flt_config->fac_bl1 = aiq_flt_config->fac_bl1;
}

static void rkisp1_params_convertBDM(struct rkisp1_isp_params_cfg* configs,
    struct cifisp_bdm_config* bdm_config, rk_aiq_bdm_config* aiq_bdm_config)
{
    if (aiq_bdm_config->enabled != mLastAiqResults.miscIspResults.bdm_config.enabled)
        configs->module_en_update |= HAL_ISP_BDM_MASK;
    configs->module_cfg_update |= HAL_ISP_BDM_MASK;
    configs->module_ens |= (aiq_bdm_config->enabled ? HAL_ISP_BDM_MASK : 0);

    bdm_config->demosaic_th = aiq_bdm_config->demosaic_th;
}

static void rkisp1_params_convertCTK(struct rkisp1_isp_params_cfg* configs,
    struct cifisp_ctk_config* ctk_config, rk_aiq_ctk_config* aiq_ctk_config)
{
    if (aiq_ctk_config->enabled != mLastAiqResults.awbResults.ctk_config.enabled)
        configs->module_en_update |= HAL_ISP_CTK_MASK;
    configs->module_cfg_update |= HAL_ISP_CTK_MASK;
    configs->module_ens |= (aiq_ctk_config->enabled ? HAL_ISP_CTK_MASK : 0);

    ctk_config->coeff0 = aiq_ctk_config->ctk_matrix.coeff[0];
    ctk_config->coeff1 = aiq_ctk_config->ctk_matrix.coeff[1];
    ctk_config->coeff2 = aiq_ctk_config->ctk_matrix.coeff[2];
    ctk_config->coeff3 = aiq_ctk_config->ctk_matrix.coeff[3];
    ctk_config->coeff4 = aiq_ctk_config->ctk_matrix.coeff[4];
    ctk_config->coeff5 = aiq_ctk_config->ctk_matrix.coeff[5];
    ctk_config->coeff6 = aiq_ctk_config->ctk_matrix.coeff[6];
    ctk_config->coeff7 = aiq_ctk_config->ctk_matrix.coeff[7];
    ctk_config->coeff8 = aiq_ctk_config->ctk_matrix.coeff[8];

    ctk_config->ct_offset_r = aiq_ctk_config->cc_offset.red;
    ctk_config->ct_offset_g = aiq_ctk_config->cc_offset.green;
    ctk_config->ct_offset_b = aiq_ctk_config->cc_offset.blue;
}

static void rkisp1_params_convertGOC(struct rkisp1_isp_params_cfg* configs,
    struct cifisp_goc_config* goc_config, rk_aiq_goc_config* aiq_goc_config)
{
    int curve_size;

    if (aiq_goc_config->enabled != mLastAiqResults.miscIspResults.gbce_config.goc_config.enabled)
        configs->module_en_update |= HAL_ISP_GOC_MASK;
    configs->module_cfg_update |= HAL_ISP_GOC_MASK;
    configs->module_ens |= (aiq_goc_config->enabled ? HAL_ISP_GOC_MASK : 0);

    switch (aiq_goc_config->mode) {
    case RK_ISP_SEGMENTATION_MODE_LOGARITHMIC:
        goc_config->mode = CIFISP_GOC_MODE_LOGARITHMIC;
        break;
    case RK_ISP_SEGMENTATION_MODE_EQUIDISTANT:
        goc_config->mode = CIFISP_GOC_MODE_EQUIDISTANT;
        break;
    case RK_ISP_SEGMENTATION_MODE_INVALID:
    case RK_ISP_SEGMENTATION_MODE_MAX:
    default:
        break;
    }

    curve_size = aiq_goc_config->gamma_y.gamma_y_cnt;
    if (curve_size > CIFISP_GAMMA_OUT_MAX_SAMPLES)
        curve_size = CIFISP_GAMMA_OUT_MAX_SAMPLES;
    for (int i = 0; i < curve_size; i++) {
        goc_config->gamma_y[i] = aiq_goc_config->gamma_y.gamma_y[i];
    }
}

static void rkisp1_params_convertCPROC(struct rkisp1_isp_params_cfg* configs,
    struct cifisp_cproc_config* cproc_config, rk_aiq_cproc_config* aiq_cproc_config)
{
    if (aiq_cproc_config->enabled != mLastAiqResults.miscIspResults.gbce_config.cproc_config.enabled)
        configs->module_en_update |= HAL_ISP_CPROC_MASK;
    configs->module_cfg_update |= HAL_ISP_CPROC_MASK;
    configs->module_ens |= (aiq_cproc_config->enabled ? HAL_ISP_CPROC_MASK : 0);

    cproc_config->c_out_range = aiq_cproc_config->ChromaOut;
    cproc_config->y_in_range = aiq_cproc_config->LumaIn;
    cproc_config->y_out_range = aiq_cproc_config->LumaOut;
    cproc_config->contrast = aiq_cproc_config->contrast;
    cproc_config->brightness = aiq_cproc_config->brightness;
    cproc_config->sat = aiq_cproc_config->saturation;
    cproc_config->hue = aiq_cproc_config->hue;
}

static void rkisp1_params_convertAWB(struct rkisp1_isp_params_cfg* configs,
    struct cifisp_awb_meas_config* awb_config, rk_aiq_awb_measure_config* aiq_awb_config)
{
    if (aiq_awb_config->enabled != mLastAiqResults.awbResults.awb_meas_cfg.enabled)
        configs->module_en_update |= HAL_ISP_AWB_MEAS_MASK;
    configs->module_cfg_update |= HAL_ISP_AWB_MEAS_MASK;
    configs->module_ens |= (aiq_awb_config->enabled ? HAL_ISP_AWB_MEAS_MASK : 0);

    awb_config->max_y = aiq_awb_config->awb_meas_cfg.max_y;
    awb_config->min_y = aiq_awb_config->awb_meas_cfg.min_y_max_g;
    awb_config->max_csum = aiq_awb_config->awb_meas_cfg.max_c_sum;
    awb_config->min_c = aiq_awb_config->awb_meas_cfg.min_c;
    awb_config->frames = 0; //fx
    awb_config->awb_ref_cr = aiq_awb_config->awb_meas_cfg.ref_cr_max_r;
    awb_config->awb_ref_cb = aiq_awb_config->awb_meas_cfg.ref_cb_max_b;
    awb_config->enable_ymax_cmp = false; //fx

    awb_config->awb_wnd.h_offs = aiq_awb_config->awb_win.h_offset;
    awb_config->awb_wnd.v_offs = aiq_awb_config->awb_win.v_offset;
    awb_config->awb_wnd.h_size = aiq_awb_config->awb_win.width;
    awb_config->awb_wnd.v_size = aiq_awb_config->awb_win.height;

    switch (aiq_awb_config->awb_meas_mode) {
    case RK_ISP_AWB_MEASURING_MODE_RGB:
        awb_config->awb_mode = CIFISP_AWB_MODE_RGB;
        break;
    case RK_ISP_AWB_MEASURING_MODE_YCBCR:
        awb_config->awb_mode = CIFISP_AWB_MODE_YCBCR;
        break;
    case RK_ISP_AWB_MEASURING_MODE_INVALID:
        awb_config->awb_mode = CIFISP_AWB_MODE_MANUAL;
        break;
    case RK_ISP_AWB_MEASURING_MODE_MAX:
    default:
        break;
    }
}

static void rkisp1_params_convertIE(struct rkisp1_isp_params_cfg* configs,
    struct cifisp_ie_config* ie_config, rk_aiq_ie_config* aiq_ie_config)
{
    if (aiq_ie_config->enabled != mLastAiqResults.miscIspResults.gbce_config.ie_config.enabled)
        configs->module_en_update |= HAL_ISP_IE_MASK;
    configs->module_cfg_update |= HAL_ISP_IE_MASK;
    configs->module_ens |= (aiq_ie_config->enabled ? HAL_ISP_IE_MASK : 0);

    switch (aiq_ie_config->mode) {
    case RK_ISP_IE_MODE_GRAYSCALE:
        ie_config->effect = V4L2_COLORFX_BW;
        break;
    case RK_ISP_IE_MODE_NEGATIVE:
        ie_config->effect = V4L2_COLORFX_NEGATIVE;
        break;
    case RK_ISP_IE_MODE_SEPIA: {
        ie_config->effect = V4L2_COLORFX_SEPIA;
    } break;
    case RK_ISP_IE_MODE_EMBOSS: {
        ie_config->effect = V4L2_COLORFX_EMBOSS;
        ie_config->eff_mat_1 = (uint16_t)(aiq_ie_config->mode_config.emboss.coeff[0])
            | ((uint16_t)(aiq_ie_config->mode_config.emboss.coeff[1]) << 0x4)
            | ((uint16_t)(aiq_ie_config->mode_config.emboss.coeff[2]) << 0x8)
            | ((uint16_t)(aiq_ie_config->mode_config.emboss.coeff[3]) << 0xc);
        ie_config->eff_mat_2 = (uint16_t)(aiq_ie_config->mode_config.emboss.coeff[4])
            | ((uint16_t)(aiq_ie_config->mode_config.emboss.coeff[5]) << 0x4)
            | ((uint16_t)(aiq_ie_config->mode_config.emboss.coeff[6]) << 0x8)
            | ((uint16_t)(aiq_ie_config->mode_config.emboss.coeff[7]) << 0xc);
        ie_config->eff_mat_3 = (aiq_ie_config->mode_config.emboss.coeff[8]);
        /*not used for this effect*/
        ie_config->eff_mat_4 = 0;
        ie_config->eff_mat_5 = 0;
        ie_config->color_sel = 0;
        ie_config->eff_tint = 0;
    } break;
    case RK_ISP_IE_MODE_SKETCH: {
        ie_config->effect = V4L2_COLORFX_SKETCH;
        ie_config->eff_mat_3 = ((uint16_t)(aiq_ie_config->mode_config.sketch.coeff[0]) << 0x4)
            | ((uint16_t)(aiq_ie_config->mode_config.sketch.coeff[1]) << 0x8)
            | ((uint16_t)(aiq_ie_config->mode_config.sketch.coeff[2]) << 0xc);
        /*not used for this effect*/
        ie_config->eff_mat_4 = (uint16_t)(aiq_ie_config->mode_config.sketch.coeff[3])
            | ((uint16_t)(aiq_ie_config->mode_config.sketch.coeff[4]) << 0x4)
            | ((uint16_t)(aiq_ie_config->mode_config.sketch.coeff[5]) << 0x8)
            | ((uint16_t)(aiq_ie_config->mode_config.sketch.coeff[6]) << 0xc);
        ie_config->eff_mat_5 = (uint16_t)(aiq_ie_config->mode_config.sketch.coeff[7])
            | ((uint16_t)(aiq_ie_config->mode_config.sketch.coeff[8]) << 0x4);

        /*not used for this effect*/
        ie_config->eff_mat_1 = 0;
        ie_config->eff_mat_2 = 0;
        ie_config->color_sel = 0;
        ie_config->eff_tint = 0;
    } break;
    case RK_ISP_IE_MODE_SHARPEN: {
        /* TODO: can't find related mode in v4l2_colorfx*/
        //ie_config->effect =
        //  V4L2_COLORFX_EMBOSS;
        ie_config->eff_mat_1 = (uint16_t)(aiq_ie_config->mode_config.sharpen.coeff[0])
            | ((uint16_t)(aiq_ie_config->mode_config.sharpen.coeff[1]) << 0x4)
            | ((uint16_t)(aiq_ie_config->mode_config.sharpen.coeff[2]) << 0x8)
            | ((uint16_t)(aiq_ie_config->mode_config.sharpen.coeff[3]) << 0xc);
        ie_config->eff_mat_2 = (uint16_t)(aiq_ie_config->mode_config.sharpen.coeff[4])
            | ((uint16_t)(aiq_ie_config->mode_config.sharpen.coeff[5]) << 0x4)
            | ((uint16_t)(aiq_ie_config->mode_config.sharpen.coeff[6]) << 0x8)
            | ((uint16_t)(aiq_ie_config->mode_config.sharpen.coeff[7]) << 0xc);
        ie_config->eff_mat_3 = (aiq_ie_config->mode_config.sharpen.coeff[8]);
        /*not used for this effect*/
        ie_config->eff_mat_4 = 0;
        ie_config->eff_mat_5 = 0;
        ie_config->color_sel = 0;
        ie_config->eff_tint = 0;
    } break;
    default:
        break;
    }
}

static void rkisp1_params_convertAEC(struct rkisp1_isp_params_cfg* configs,
    struct cifisp_aec_config* aec_config, rk_aiq_aec_config* aiq_aec_config)
{
    if (aiq_aec_config->enabled != mLastAiqResults.aeResults.aec_config_result.enabled)
        configs->module_en_update |= HAL_ISP_AEC_MASK;
    configs->module_cfg_update |= HAL_ISP_AEC_MASK;
    configs->module_ens |= (aiq_aec_config->enabled ? HAL_ISP_AEC_MASK : 0);

    aec_config->autostop = 0;

    aec_config->meas_window.h_offs = aiq_aec_config->win.h_offset;
    aec_config->meas_window.v_offs = aiq_aec_config->win.v_offset;
    aec_config->meas_window.h_size = aiq_aec_config->win.width;
    aec_config->meas_window.v_size = aiq_aec_config->win.height;

    switch (aiq_aec_config->mode) {
    case RK_ISP_EXP_MEASURING_MODE_0:
        aec_config->mode = CIFISP_EXP_MEASURING_MODE_0;
        break;
    case RK_ISP_EXP_MEASURING_MODE_1:
        aec_config->mode = CIFISP_EXP_MEASURING_MODE_1;
        break;
    case RK_ISP_EXP_MEASURING_MODE_INVALID:
    case RK_ISP_EXP_MEASURING_MODE_MAX:
    default:
        break;
    }
}

static void rkisp1_params_convertDPF(struct rkisp1_isp_params_cfg* configs,
    struct cifisp_dpf_config* dpf_config, rk_aiq_dpf_config* aiq_dpf_config)
{
    int spatial_size, nll_size, i;

    if (aiq_dpf_config->enabled != mLastAiqResults.miscIspResults.dpf_config.enabled)
        configs->module_en_update |= HAL_ISP_DPF_MASK;
    configs->module_cfg_update |= HAL_ISP_DPF_MASK;
    configs->module_ens |= (aiq_dpf_config->enabled ? HAL_ISP_DPF_MASK : 0);

    switch (aiq_dpf_config->gain_usage) {
    case RK_AIQ_ISP_DPF_GAIN_USAGE_DISABLED:
        dpf_config->gain.mode = CIFISP_DPF_GAIN_USAGE_DISABLED;
        break;
    case RK_AIQ_ISP_DPF_GAIN_USAGE_NF_GAINS:
        dpf_config->gain.mode = CIFISP_DPF_GAIN_USAGE_NF_GAINS;
        break;
    case RK_AIQ_ISP_DPF_GAIN_USAGE_LSC_GAINS:
        dpf_config->gain.mode = CIFISP_DPF_GAIN_USAGE_LSC_GAINS;
        break;
    case RK_AIQ_ISP_DPF_GAIN_USAGE_NF_LSC_GAINS:
        dpf_config->gain.mode = CIFISP_DPF_GAIN_USAGE_NF_LSC_GAINS;
        break;
    case RK_AIQ_ISP_DPF_GAIN_USAGE_AWB_GAINS:
        dpf_config->gain.mode = CIFISP_DPF_GAIN_USAGE_AWB_GAINS;
        break;
    case RK_AIQ_ISP_DPF_GAIN_USAGE_AWB_LSC_GAINS:
        dpf_config->gain.mode = CIFISP_DPF_GAIN_USAGE_AWB_LSC_GAINS;
        break;
    case RK_AIQ_ISP_DPF_GAIN_USAGE_MAX:
        dpf_config->gain.mode = CIFISP_DPF_GAIN_USAGE_MAX;
        break;
    case RK_AIQ_ISP_DPF_GAIN_USAGE_INVALID:
    default:
        break;
    }
    dpf_config->gain.nf_r_gain = aiq_dpf_config->nf_ains.red_gain;
    dpf_config->gain.nf_gr_gain = aiq_dpf_config->nf_ains.green_r_gain;
    dpf_config->gain.nf_gb_gain = aiq_dpf_config->nf_ains.green_b_gain;
    dpf_config->gain.nf_b_gain = aiq_dpf_config->nf_ains.blue_gain;

    dpf_config->g_flt.gr_enable = aiq_dpf_config->process_gr_pixel;
    dpf_config->g_flt.gb_enable = aiq_dpf_config->process_gb_pixel;
    spatial_size = aiq_dpf_config->spatial_g.weight_coeff_size;
    if (spatial_size > CIFISP_DPF_MAX_SPATIAL_COEFFS)
        spatial_size = CIFISP_DPF_MAX_SPATIAL_COEFFS;
    for (int i = 0; i < spatial_size; i++) {
        dpf_config->g_flt.spatial_coeff[i] = aiq_dpf_config->spatial_g.weight_coeff[i];
    }

    dpf_config->rb_flt.r_enable = aiq_dpf_config->process_red_pixel;
    dpf_config->rb_flt.b_enable = aiq_dpf_config->process_blue_pixel;
    spatial_size = aiq_dpf_config->spatial_rb.weight_coeff_size;
    if (spatial_size > CIFISP_DPF_MAX_SPATIAL_COEFFS)
        spatial_size = CIFISP_DPF_MAX_SPATIAL_COEFFS;
    for (i = 0; i < spatial_size; i++) {
        dpf_config->rb_flt.spatial_coeff[i] = aiq_dpf_config->spatial_rb.weight_coeff[i];
    }

    switch (aiq_dpf_config->rb_flt_size) {
    case RK_AIQ_ISP_DPF_RB_FILTERSIZE_13x9:
        dpf_config->rb_flt.fltsize = CIFISP_DPF_RB_FILTERSIZE_13x9;
        break;
    case RK_AIQ_ISP_DPF_RB_FILTERSIZE_9x9:
        dpf_config->rb_flt.fltsize = CIFISP_DPF_RB_FILTERSIZE_9x9;
        break;
    case RK_AIQ_ISP_DPF_RB_FILTERSIZE_INVALID:
    case RK_AIQ_ISP_DPF_RB_FILTERSIZE_MAX:
    default:
        break;
    }

    nll_size = aiq_dpf_config->lookup.nll_coeff_size;
    if (nll_size > CIFISP_DPF_MAX_NLF_COEFFS)
        nll_size = CIFISP_DPF_MAX_NLF_COEFFS;
    for (int i = 0; i < nll_size; i++) {
        dpf_config->nll.coeff[i] = aiq_dpf_config->lookup.nll_coeff[i];
    }
    switch (aiq_dpf_config->lookup.x_scale) {
    case RK_AIQ_ISP_DPF_NLL_SCALE_LINEAR:
        dpf_config->nll.scale_mode = CIFISP_NLL_SCALE_LINEAR;
        break;
    case RK_AIQ_ISP_DPF_NLL_SCALE_LOGARITHMIC:
        dpf_config->nll.scale_mode = CIFISP_NLL_SCALE_LOGARITHMIC;
        break;
    case RK_AIQ_ISP_DPF_NLL_SCALE_INVALID:
    case RK_AIQ_ISP_DPF_NLL_SCALE_MAX:
    default:
        break;
    }
}

static void rkisp1_params_convertDPFStrength(struct rkisp1_isp_params_cfg* configs,
    struct cifisp_dpf_strength_config* dpfStrength_config, rk_aiq_dpf_strength_config* aiq_dpfStrength_config)
{
    if (aiq_dpfStrength_config->enabled != mLastAiqResults.miscIspResults.strength_config.enabled)
        configs->module_en_update |= HAL_ISP_DPF_STRENGTH_MASK;
    configs->module_cfg_update |= HAL_ISP_DPF_STRENGTH_MASK;
    configs->module_ens |= (aiq_dpfStrength_config->enabled ? HAL_ISP_DPF_STRENGTH_MASK : 0);

    dpfStrength_config->r = aiq_dpfStrength_config->r;
    dpfStrength_config->g = aiq_dpfStrength_config->g;
    dpfStrength_config->b = aiq_dpfStrength_config->b;
}

XCamReturn rkisp1_convert_params(struct rkisp1_isp_params_cfg* isp_cfg, struct AiqResults* aiqResults)
{
    if (memcmp(&aiqResults->awbResults.awb_meas_cfg, &mLastAiqResults.awbResults.awb_meas_cfg, sizeof(rk_aiq_awb_measure_config)) != 0)
        rkisp1_params_convertAWB(isp_cfg, &isp_cfg->meas.awb_meas_config, &aiqResults->awbResults.awb_meas_cfg);
    if (memcmp(&aiqResults->awbResults.awb_gain_cfg, &mLastAiqResults.awbResults.awb_gain_cfg, sizeof(rk_aiq_awb_gain_config)) != 0)
        rkisp1_params_convertAWBGain(isp_cfg, &isp_cfg->others.awb_gain_config, &aiqResults->awbResults.awb_gain_cfg);
    if (memcmp(&aiqResults->awbResults.ctk_config, &mLastAiqResults.awbResults.ctk_config, sizeof(rk_aiq_ctk_config)) != 0)
        rkisp1_params_convertCTK(isp_cfg, &isp_cfg->others.ctk_config, &aiqResults->awbResults.ctk_config);
    if (memcmp(&aiqResults->awbResults.lsc_cfg, &mLastAiqResults.awbResults.lsc_cfg, sizeof(rk_aiq_lsc_config)) != 0)
        rkisp1_params_convertLSC(isp_cfg, &isp_cfg->others.lsc_config, &aiqResults->awbResults.lsc_cfg);

    if (memcmp(&aiqResults->aeResults.aec_config_result, &mLastAiqResults.aeResults.aec_config_result, sizeof(rk_aiq_aec_config)) != 0)
        rkisp1_params_convertAEC(isp_cfg, &isp_cfg->meas.aec_config, &aiqResults->aeResults.aec_config_result);
    if (memcmp(&aiqResults->aeResults.hist_config_result, &mLastAiqResults.aeResults.hist_config_result, sizeof(rk_aiq_hist_config)) != 0)
        rkisp1_params_convertHST(isp_cfg, &isp_cfg->meas.hst_config, &aiqResults->aeResults.hist_config_result);

    if (memcmp(&aiqResults->miscIspResults.bls_config, &mLastAiqResults.miscIspResults.bls_config, sizeof(rk_aiq_bls_config)) != 0)
        rkisp1_params_convertBLS(isp_cfg, &isp_cfg->others.bls_config, &aiqResults->miscIspResults.bls_config);
    if (memcmp(&aiqResults->miscIspResults.dpcc_config, &mLastAiqResults.miscIspResults.dpcc_config, sizeof(rk_aiq_dpcc_config)) != 0)
        rkisp1_params_convertDPCC(isp_cfg, &isp_cfg->others.dpcc_config, &aiqResults->miscIspResults.dpcc_config);
    if (memcmp(&aiqResults->miscIspResults.flt_config, &mLastAiqResults.miscIspResults.flt_config, sizeof(rk_aiq_flt_config)) != 0)
        rkisp1_params_convertFLT(isp_cfg, &isp_cfg->others.flt_config, &aiqResults->miscIspResults.flt_config);
    if (memcmp(&aiqResults->miscIspResults.dpf_config, &mLastAiqResults.miscIspResults.dpf_config, sizeof(rk_aiq_dpf_config)) != 0)
        rkisp1_params_convertDPF(isp_cfg, &isp_cfg->others.dpf_config, &aiqResults->miscIspResults.dpf_config);
    if (memcmp(&aiqResults->miscIspResults.strength_config, &mLastAiqResults.miscIspResults.strength_config, sizeof(rk_aiq_dpf_strength_config)) != 0)
        rkisp1_params_convertDPFStrength(isp_cfg, &isp_cfg->others.dpf_strength_config, &aiqResults->miscIspResults.strength_config);
    if (memcmp(&aiqResults->miscIspResults.gbce_config.ie_config, &mLastAiqResults.miscIspResults.gbce_config.ie_config, sizeof(rk_aiq_ie_config)) != 0)
        rkisp1_params_convertIE(isp_cfg, &isp_cfg->others.ie_config, &aiqResults->miscIspResults.gbce_config.ie_config);
    if (memcmp(&aiqResults->miscIspResults.bdm_config, &mLastAiqResults.miscIspResults.bdm_config, sizeof(rk_aiq_bdm_config)) != 0)
        rkisp1_params_convertBDM(isp_cfg, &isp_cfg->others.bdm_config, &aiqResults->miscIspResults.bdm_config);
    if (memcmp(&aiqResults->miscIspResults.gbce_config.goc_config, &mLastAiqResults.miscIspResults.gbce_config.goc_config, sizeof(rk_aiq_goc_config)) != 0)
        rkisp1_params_convertGOC(isp_cfg, &isp_cfg->others.goc_config, &aiqResults->miscIspResults.gbce_config.goc_config);
    if (memcmp(&aiqResults->miscIspResults.gbce_config.cproc_config, &mLastAiqResults.miscIspResults.gbce_config.cproc_config, sizeof(rk_aiq_cproc_config)) != 0)
        rkisp1_params_convertCPROC(isp_cfg, &isp_cfg->others.cproc_config, &aiqResults->miscIspResults.gbce_config.cproc_config);

    mLastAiqResults = *aiqResults;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn rkisp1_convert_results(
    struct rkisp1_isp_params_cfg* isp_cfg,
    struct rkisp_parameters *aiq_results,
    struct rkisp_parameters &last_aiq_results)
{
#define CONVERT_RET(id, mask, cfg, ret, last) \
    XCAM_LOG_DEBUG("convert ret id: %d, en: %d, lasten: %d", id, aiq_results->enabled[id], last_aiq_results.enabled[id]); \
    if ((aiq_results->active_configs & mask) && (memcmp(&ret, &last, sizeof(ret)) != 0)) { \
        if (aiq_results->enabled[id] != last_aiq_results.enabled[id]) \
            isp_cfg->module_en_update |= mask; \
        isp_cfg->module_cfg_update |= mask; \
        isp_cfg->module_ens |= (aiq_results->enabled[id] ? mask : 0); \
        cfg = ret; \
    }

    //disable ie cproc bdm
    /* aiq_results->enabled[HAL_ISP_IE_ID] = 0; */
    /*
     * TODO: BDM cause the image black-white
     * BDM enable means demosaic bypassed, so it should
     * be disabled normally, but now the value is 1 here,
     * we just force it disabled here for workaround.
     */
    aiq_results->enabled[HAL_ISP_BDM_ID] = 0;
    /* aiq_results->enabled[HAL_ISP_CPROC_ID] = 0; */

    //aiq_results->enabled[HAL_ISP_AWB_MEAS_ID] = 0;
    //aiq_results->enabled[HAL_ISP_AWB_GAIN_ID] = 0;
    /* aiq_results->enabled[HAL_ISP_CTK_ID] = 0; */
    //aiq_results->enabled[HAL_ISP_LSC_ID] = 0;
    //aiq_results->enabled[HAL_ISP_BLS_ID] = 0;
    //aiq_results->enabled[HAL_ISP_BPC_ID] = 0;
    //aiq_results->enabled[HAL_ISP_FLT_ID] = 0;
    //aiq_results->enabled[HAL_ISP_DPF_ID] = 0;
    //aiq_results->enabled[HAL_ISP_DPF_STRENGTH_ID] = 0;
    //aiq_results->enabled[HAL_ISP_GOC_ID] = 0;

#if 0
    struct cifisp_aec_config aec_config;
    aec_config.autostop = 0;
    aec_config.meas_window = aiq_results->aec_config.meas_window;
    aec_config.mode = aiq_results->aec_config.mode;

    struct cifisp_aec_config last_aec_config;
    last_aec_config.autostop = 0;
    last_aec_config.meas_window = last_aiq_results.aec_config.meas_window;
    last_aec_config.mode = last_aiq_results.aec_config.mode;
    CONVERT_RET(HAL_ISP_AEC_ID, HAL_ISP_AEC_MASK,
        isp_cfg->meas.aec_config, aec_config, last_aec_config);
#else
    CONVERT_RET(HAL_ISP_AEC_ID, HAL_ISP_AEC_MASK,
        isp_cfg->meas.aec_config, aiq_results->aec_config, last_aiq_results.aec_config);
#endif
    CONVERT_RET(HAL_ISP_AWB_MEAS_ID, HAL_ISP_AWB_MEAS_MASK,
        isp_cfg->meas.awb_meas_config, aiq_results->awb_meas_config, last_aiq_results.awb_meas_config);
    CONVERT_RET(HAL_ISP_HST_ID, HAL_ISP_HST_MASK,
        isp_cfg->meas.hst_config, aiq_results->hst_config, last_aiq_results.hst_config);

    CONVERT_RET(HAL_ISP_AWB_GAIN_ID, HAL_ISP_AWB_GAIN_MASK,
        isp_cfg->others.awb_gain_config, aiq_results->awb_gain_config, last_aiq_results.awb_gain_config);
    CONVERT_RET(HAL_ISP_CTK_ID, HAL_ISP_CTK_MASK,
        isp_cfg->others.ctk_config, aiq_results->ctk_config, last_aiq_results.ctk_config);
    CONVERT_RET(HAL_ISP_LSC_ID, HAL_ISP_LSC_MASK,
        isp_cfg->others.lsc_config, aiq_results->lsc_config, last_aiq_results.lsc_config);
    CONVERT_RET(HAL_ISP_BLS_ID, HAL_ISP_BLS_MASK,
        isp_cfg->others.bls_config, aiq_results->bls_config, last_aiq_results.bls_config);
    CONVERT_RET(HAL_ISP_BPC_ID, HAL_ISP_BPC_MASK,
        isp_cfg->others.dpcc_config, aiq_results->dpcc_config, last_aiq_results.dpcc_config);
    CONVERT_RET(HAL_ISP_FLT_ID, HAL_ISP_FLT_MASK,
        isp_cfg->others.flt_config, aiq_results->flt_config, last_aiq_results.flt_config);
    CONVERT_RET(HAL_ISP_DPF_ID, HAL_ISP_DPF_MASK,
        isp_cfg->others.dpf_config, aiq_results->dpf_config, last_aiq_results.dpf_config);
    CONVERT_RET(HAL_ISP_DPF_STRENGTH_ID, HAL_ISP_DPF_STRENGTH_MASK,
        isp_cfg->others.dpf_strength_config, aiq_results->dpf_strength_config, last_aiq_results.dpf_strength_config);
    CONVERT_RET(HAL_ISP_IE_ID, HAL_ISP_IE_MASK,
        isp_cfg->others.ie_config, aiq_results->ie_config, last_aiq_results.ie_config);
    CONVERT_RET(HAL_ISP_BDM_ID, HAL_ISP_BDM_MASK,
        isp_cfg->others.bdm_config, aiq_results->bdm_config, last_aiq_results.bdm_config);
    CONVERT_RET(HAL_ISP_GOC_ID, HAL_ISP_GOC_MASK,
        isp_cfg->others.goc_config, aiq_results->goc_config, last_aiq_results.goc_config);
    CONVERT_RET(HAL_ISP_CPROC_ID, HAL_ISP_CPROC_MASK,
        isp_cfg->others.cproc_config, aiq_results->cproc_config, last_aiq_results.cproc_config);

    CONVERT_RET(HAL_ISP_AFC_ID,HAL_ISP_AFC_MASK,
        isp_cfg->meas.afc_config, aiq_results->afc_config, last_aiq_results.afc_config);

    CONVERT_RET(HAL_ISP_DEMOSAICLP_ID,HAL_ISP_DEMOSAICLP_MASK,
        isp_cfg->others.demosaiclp_config, aiq_results->demosaiclp_config, last_aiq_results.demosaiclp_config);
    CONVERT_RET(HAL_ISP_RKIESHARP_ID,HAL_ISP_RK_IESHARP_MASK,
        isp_cfg->others.rkiesharp_config, aiq_results->rkiesharp_config, last_aiq_results.rkiesharp_config);
    CONVERT_RET(HAL_ISP_WDR_ID,HAL_ISP_WDR_MASK,
        isp_cfg->others.wdr_config, aiq_results->wdr_config, last_aiq_results.wdr_config);

    last_aiq_results = *aiq_results;

    XCAM_LOG_DEBUG("isp_cfg module_ens 0x%x, en_update 0x%x, cfg_update 0x%x",
                 isp_cfg->module_ens,
                 isp_cfg->module_en_update,
                 isp_cfg->module_cfg_update);
    return XCAM_RETURN_NO_ERROR;
}

