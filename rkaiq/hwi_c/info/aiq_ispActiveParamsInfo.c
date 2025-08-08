/*
 *   Copyright (c) 2024 Rockchip CorporatiY
 *
 * Licensed under the Apache License, VersiY 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed Y an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CYDITIYS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissiYs and
 * limitatiYs under the License.
 *
 */

#define _GNU_SOURCE

#include "aiq_ispActiveParamsInfo.h"

#include <string.h>

#include "hwi_c/aiq_CamHwBase.h"
#include "rk_info_utils.h"

void active_isp_params_dump_mod_param(void* self, int cam_id, st_string* result) {
    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "ISP active params");

    snprintf(buffer, MAX_LINE_LENGTH, "%-8s", "phychn");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);

    snprintf(buffer, MAX_LINE_LENGTH, "%-8d", cam_id);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");
}

#if defined(ISP_HW_V39)
void active_isp_params_dump_rgbir_status(void* self, st_string* result) {
    struct isp39_isp_params_cfg* params = (struct isp39_isp_params_cfg*)self;
    struct isp39_rgbir_cfg* rgbir_cfg   = &params->others.rgbir_cfg;

    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "rgbir mod status");

    snprintf(buffer, MAX_LINE_LENGTH, "%-5s", "en");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    bool en = params->module_ens & ISP39_MODULE_RGBIR;

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-5s", en ? "Y" : "N");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");
}
#endif

void active_isp_params_dump_bls_status(void* self, st_string* result) {
#if defined(ISP_HW_V39)
    struct isp39_isp_params_cfg* params = (struct isp39_isp_params_cfg*)self;
#elif defined(ISP_HW_V33)
    struct isp33_isp_params_cfg* params = (struct isp33_isp_params_cfg*)self;
#elif defined(ISP_HW_V35)
    struct isp35_isp_params_cfg* params = (struct isp35_isp_params_cfg*)self;
#else
    return;
#endif

    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "bls mod status");

    snprintf(buffer, MAX_LINE_LENGTH, "%-5s%-9s%-11s%-12s%-12s%-12s%-12s%-13s%-8s", "en", "bls1_en",
             "bls1_val.r", "bls1_val.gr", "bls1_val.gb", "bls1_val.b", "ob_offset",
             "ob_predgain", "ob_max");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    bool en                       = params->module_ens & ISP39_MODULE_BLS;
    
#if ISP_HW_V35
    struct isp35_bls_cfg* bls_cfg = &params->others.bls_cfg;
#else
    struct isp32_bls_cfg* bls_cfg = &params->others.bls_cfg;
#endif

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-5s%-9s%-11d%-12d%-12d%-12d%-12d%-13d%-8d", en ? "Y" : "N",
             bls_cfg->bls1_en ? "Y" : "N", bls_cfg->bls1_val.r, bls_cfg->bls1_val.gr,
             bls_cfg->bls1_val.gb, bls_cfg->bls1_val.b, bls_cfg->isp_ob_offset,
             bls_cfg->isp_ob_predgain, bls_cfg->isp_ob_max);

    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");
}

static void __dump_awb1_gain(void* self, char* buffer, st_string* result) {
#if defined(ISP_HW_V39)
    struct isp39_isp_params_cfg* params = (struct isp39_isp_params_cfg*)self;
#elif defined(ISP_HW_V33)
    struct isp33_isp_params_cfg* params = (struct isp33_isp_params_cfg*)self;
#elif defined(ISP_HW_V35)
    struct isp35_isp_params_cfg* params = (struct isp35_isp_params_cfg*)self;
#else
    return;
#endif

    struct isp32_awb_gain_cfg* wb_cfg = &params->others.awb_gain_cfg;

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-14s%-14s%-14s%-14s", "wb1_gain0_r", "wb1_gain0_gr",
             "wb1_gain0_gb", "wb1_gain0_b");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    snprintf(buffer, MAX_LINE_LENGTH, "%-14d%-14d%-14d%-14d", wb_cfg->awb1_gain_r,
             wb_cfg->awb1_gain_gr, wb_cfg->awb1_gain_gb, wb_cfg->awb1_gain_b);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");
}

static void __dump_awb0_gain(void* self, char* buffer, st_string* result) {
#if defined(ISP_HW_V39)
    struct isp39_isp_params_cfg* params = (struct isp39_isp_params_cfg*)self;
#elif defined(ISP_HW_V33)
    struct isp33_isp_params_cfg* params = (struct isp33_isp_params_cfg*)self;
#elif defined(ISP_HW_V35)
    struct isp35_isp_params_cfg* params = (struct isp35_isp_params_cfg*)self;
#else
    return;
#endif

    struct isp32_awb_gain_cfg* wb_cfg = &params->others.awb_gain_cfg;

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-14s%-14s%-14s%-14s", "wb0_gain0_r", "wb0_gain0_gr",
             "wb0_gain0_gb", "wb0_gain0_b");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    snprintf(buffer, MAX_LINE_LENGTH, "%-14d%-14d%-14d%-14d", wb_cfg->gain0_red,
             wb_cfg->gain0_green_r, wb_cfg->gain0_green_b, wb_cfg->gain0_blue);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-14s%-14s%-14s%-14s", "wb0_gain1_r", "wb0_gain1_gr",
             "wb0_gain1_gb", "wb0_gain1_b");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    snprintf(buffer, MAX_LINE_LENGTH, "%-14d%-14d%-14d%-14d", wb_cfg->gain1_red,
             wb_cfg->gain1_green_r, wb_cfg->gain1_green_b, wb_cfg->gain1_blue);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-14s%-14s%-14s%-14s", "wb0_gain2_r", "wb0_gain2_gr",
             "wb0_gain2_gb", "wb0_gain2_b");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    snprintf(buffer, MAX_LINE_LENGTH, "%-14d%-14d%-14d%-14d", wb_cfg->gain2_red,
             wb_cfg->gain2_green_r, wb_cfg->gain2_green_b, wb_cfg->gain2_blue);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");
}

void active_isp_params_dump_wbgain_status(void* self, st_string* result) {
#if defined(ISP_HW_V39)
    struct isp39_isp_params_cfg* params = (struct isp39_isp_params_cfg*)self;
#elif defined(ISP_HW_V33)
    struct isp33_isp_params_cfg* params = (struct isp33_isp_params_cfg*)self;
#elif defined(ISP_HW_V35)
    struct isp35_isp_params_cfg* params = (struct isp35_isp_params_cfg*)self;
#else
    return;
#endif

    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "awb gain mod status");

    snprintf(buffer, MAX_LINE_LENGTH, "%-5s", "en");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    bool en = params->module_ens & ISP39_MODULE_AWB_GAIN;

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-5s", en ? "Y" : "N");

    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");

    __dump_awb1_gain(self, buffer, result);
    __dump_awb0_gain(self, buffer, result);
}

void active_isp_params_dump_rawawb_status(void* self, st_string* result) {
#if defined(ISP_HW_V39)
    struct isp39_isp_params_cfg* params =  (struct isp39_isp_params_cfg*)self;
    struct isp39_rawawb_meas_cfg* awb_cfg = &params->meas.rawawb;
#elif defined(ISP_HW_V33)
    struct isp33_isp_params_cfg* params = (struct isp33_isp_params_cfg*)self;
    struct isp33_rawawb_meas_cfg* awb_cfg = &params->meas.rawawb;
#elif defined(ISP_HW_V35)
    struct isp35_isp_params_cfg* params = (struct isp35_isp_params_cfg*)self;
    struct isp35_rawawb_meas_cfg* awb_cfg = &params->meas.rawawb;
#else
    return;
#endif

    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "rawawb mod status");

    snprintf(buffer, MAX_LINE_LENGTH, "%-5s", "en");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    bool en = params->module_ens & ISP39_MODULE_RAWAWB;

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-5s", en ? "Y" : "N");

    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");
#if defined(ISP_HW_V33) || defined(ISP_HW_V35)
    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-14s%-14s%-14s%-14s%-14s", "uvDct_en", "xyDct_en",
             "drcOut_mod_en", "bnrOut_mod_en", "light_num");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");
    snprintf(buffer, MAX_LINE_LENGTH, "%-14d%-14d%-14d%-14d%-14d", awb_cfg->uv_en0,
             awb_cfg->xy_en0, awb_cfg->drc2awb_sel, awb_cfg->bnr2awb_sel, awb_cfg->light_num);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");
#else
    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-14s%-14s%-14s%-14s%-14s%-14s", "uvDct_en", "xyDct_en",
             "yuvDct_en", "drcOut_mod_en", "bnrOut_mod_en", "light_num");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");
    snprintf(buffer, MAX_LINE_LENGTH, "%-14d%-14d%-14d%-14d%-14d%-14d", awb_cfg->uv_en0,
             awb_cfg->xy_en0, awb_cfg->yuv3d_en0, awb_cfg->drc2awb_sel, awb_cfg->bnr2awb_sel, awb_cfg->light_num);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");
#endif
    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-14s%-14s%-14s%-14s", "zoneStat_en", "zoneStat_mod",
             "zoneStat_type", "zoneStat_illu");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");
    snprintf(buffer, MAX_LINE_LENGTH, "%-14d%-14d%-14d%-14d", awb_cfg->blk_measure_enable,
             awb_cfg->blk_measure_mode, awb_cfg->blk_measure_xytype, awb_cfg->blk_measure_illu_idx);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-14s%-14s%-14s%-14s", "is12bit", "ds16x8_mode",
             "ds8x8_mode", "overexpo_th");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");
    snprintf(buffer, MAX_LINE_LENGTH, "%-14d%-14d%-14d%-14d", awb_cfg->low12bit_val,
             awb_cfg->ds16x8_mode_en, awb_cfg->wind_size, awb_cfg->in_overexposure_threshold);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-14s%-14s%-14s%-14s%-14s", "mainWin_x", "mainWin_y",
             "mainWin_width", "mainWin_hegt", "nonROI_en");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");
    snprintf(buffer, MAX_LINE_LENGTH, "%-14d%-14d%-14d%-14d%-14d", awb_cfg->h_offs,
             awb_cfg->v_offs, awb_cfg->h_size, awb_cfg->v_size, awb_cfg->multiwindow_en);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-14s%-14s%-14s%-14s", "limit_maxR", "limit_maxG",
             "limit_maxB", "limit_minY");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");
    snprintf(buffer, MAX_LINE_LENGTH, "%-14d%-14d%-14d%-14d", awb_cfg->r_max,
             awb_cfg->g_max, awb_cfg->b_max, awb_cfg->y_max);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-14s%-6s%-6s%-6s%-6s%-6s%-6s%-6s%-6s%-6s", "luma2WpWgt_en", "w0",
             "w1", "w2", "w3", "w4", "w5", "w6", "w7", "w8");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");
    snprintf(buffer, MAX_LINE_LENGTH, "%-14d%-6d%-6d%-6d%-6d%-6d%-6d%-6d%-6d%-6d", awb_cfg->wp_luma_wei_en0,
             awb_cfg->wp_luma_weicurve_w0, awb_cfg->wp_luma_weicurve_w1, awb_cfg->wp_luma_weicurve_w2,
             awb_cfg->wp_luma_weicurve_w3, awb_cfg->wp_luma_weicurve_w4, awb_cfg->wp_luma_weicurve_w5,
             awb_cfg->wp_luma_weicurve_w6, awb_cfg->wp_luma_weicurve_w7, awb_cfg->wp_luma_weicurve_w8);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");
#if defined(ISP_HW_V33) || defined(ISP_HW_V35)
    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-6s%-6s%-6s%-6s%-6s%-6s%-6s%-6s%-6s", "ccm0r", "ccm1r",
             "ccm2r", "ccm0g", "ccm1g", "ccm1g", "ccm0b", "ccm1b", "ccm1b");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");
    snprintf(buffer, MAX_LINE_LENGTH, "%-6d%-6d%-6d%-6d%-6d%-6d%-6d%-6d%-6d", c2trval(15, awb_cfg->ccm_coeff0_r),
             c2trval(15, awb_cfg->ccm_coeff1_r), c2trval(15, awb_cfg->ccm_coeff2_r), c2trval(15, awb_cfg->ccm_coeff0_g),
             c2trval(15, awb_cfg->ccm_coeff1_g), c2trval(15, awb_cfg->ccm_coeff2_g), c2trval(15, awb_cfg->ccm_coeff0_b),
             c2trval(15, awb_cfg->ccm_coeff1_b), c2trval(15, awb_cfg->ccm_coeff2_b));
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");
#endif
    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-14s%-14s%-14s", "pre_wbgain_r", "pre_wbgain_g",
             "pre_wbgain_b");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");
    snprintf(buffer, MAX_LINE_LENGTH, "%-14d%-14d%-14d", awb_cfg->pre_wbgain_inv_r,
             awb_cfg->pre_wbgain_inv_g, awb_cfg->pre_wbgain_inv_b);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");


}

void active_isp_params_dump_dpc_status(void* self, st_string* result) {
#if defined(ISP_HW_V39)
    struct isp39_isp_params_cfg* params = (struct isp39_isp_params_cfg*)self;
#elif defined(ISP_HW_V33)
    struct isp33_isp_params_cfg* params = (struct isp33_isp_params_cfg*)self;
#elif defined(ISP_HW_V35)
    struct isp35_isp_params_cfg* params = (struct isp35_isp_params_cfg*)self;
#else
    return;
#endif

    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "dpc mod status");

    snprintf(buffer, MAX_LINE_LENGTH, "%-5s%-14s%-16s%-14s%-14s%-14s", "en", "rk_out_sel",
             "dpcc_output_sel", "use_set_1", "use_set_2", "use_set_3");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    bool en = params->module_ens & ISP39_MODULE_DPCC;
    struct isp39_dpcc_cfg* dpc_cfg = &params->others.dpcc_cfg;

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-5s%-14d%-16d%-14d%-14d%-14d", en ? "Y" : "N",
             dpc_cfg->sw_rk_out_sel, dpc_cfg->sw_dpcc_output_sel, dpc_cfg->stage1_use_set_1,
             dpc_cfg->stage1_use_set_2, dpc_cfg->stage1_use_set_3);

    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");
}

void active_isp_params_dump_lsc_status(void* self, st_string* result) {
#if defined(ISP_HW_V39)
    struct isp39_isp_params_cfg* params = (struct isp39_isp_params_cfg*)self;
#elif defined(ISP_HW_V33)
    struct isp33_isp_params_cfg* params = (struct isp33_isp_params_cfg*)self;
#elif defined(ISP_HW_V35)
    struct isp35_isp_params_cfg* params = (struct isp35_isp_params_cfg*)self;
#else
    return;
#endif

    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "lsc mod status");

    snprintf(buffer, MAX_LINE_LENGTH, "%-5s%-8s%-8s%-8s%-8s%-8s%-9s%-9s%-8s", "en", "x_size",
             "y_size", "x_grad", "y_grad", "r_data", "gr_data", "gb_data", "b_data");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    bool en                       = params->module_ens & ISP39_MODULE_LSC;
    struct isp3x_lsc_cfg* lsc_cfg = &params->others.lsc_cfg;

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-5s%-8d%-8d%-8d%-8d%-8d%-9d%-9d%-8d", en ? "Y" : "N",
             lsc_cfg->x_size_tbl[0], lsc_cfg->y_size_tbl[0], lsc_cfg->x_grad_tbl[0],
             lsc_cfg->y_grad_tbl[0], lsc_cfg->r_data_tbl[0], lsc_cfg->gr_data_tbl[0],
             lsc_cfg->gb_data_tbl[0], lsc_cfg->b_data_tbl[0]);

    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");
}

void active_isp_params_dump_gic_status(void* self, st_string* result) {
#if defined(ISP_HW_V39)
    struct isp39_isp_params_cfg* params = (struct isp39_isp_params_cfg*)self;
    bool en                             = params->module_ens & ISP39_MODULE_GIC;
    struct isp39_gic_cfg* gic_cfg       = &params->others.gic_cfg;
#elif defined(ISP_HW_V33)
    struct isp33_isp_params_cfg* params = (struct isp33_isp_params_cfg*)self;
    bool en                             = params->module_ens & ISP33_MODULE_GIC;
    struct isp33_gic_cfg* gic_cfg       = &params->others.gic_cfg;
#elif defined(ISP_HW_V35)
    struct isp35_isp_params_cfg* params = (struct isp35_isp_params_cfg*)self;
    bool en                             = params->module_ens & ISP33_MODULE_GIC;
    struct isp33_gic_cfg* gic_cfg       = &params->others.gic_cfg;
#else
    return;
#endif

    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "gic mod status");

    snprintf(buffer, MAX_LINE_LENGTH, "%-5s%-8s", "en", "bypass");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-5s%-8s", en ? "Y" : "N", gic_cfg->bypass_en ? "Y" : "N");

    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");
}

void active_isp_params_dump_debayer_status(void* self, st_string* result) {
#if defined(ISP_HW_V39)
    struct isp39_isp_params_cfg* params = (struct isp39_isp_params_cfg*)self;
    struct isp39_debayer_cfg* dm_cfg    = &params->others.debayer_cfg;
#elif defined(ISP_HW_V33)
    struct isp33_isp_params_cfg* params = (struct isp33_isp_params_cfg*)self;
    struct isp33_debayer_cfg* dm_cfg    = &params->others.debayer_cfg;
#elif defined(ISP_HW_V35)
    struct isp35_isp_params_cfg* params = (struct isp35_isp_params_cfg*)self;
    struct isp35_debayer_cfg* dm_cfg    = &params->others.debayer_cfg;
#else
    return;
#endif

    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "debayer mod status");

#if defined(ISP_HW_V39)
    snprintf(buffer, MAX_LINE_LENGTH, "%-5s%-10s%-10s", "en", "flt_g_en", "flt_c_en");
#elif defined(ISP_HW_V33)
    snprintf(buffer, MAX_LINE_LENGTH, "%-5s%-8s%-10s", "en", "bypass", "g_flt_en");
#elif defined(ISP_HW_V35)
    snprintf(buffer, MAX_LINE_LENGTH, "%-5s%-10s%-10s", "en", "flt_g_en", "flt_c_en");
#endif
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    bool en = params->module_ens & ISP39_MODULE_DEBAYER;

    memset(buffer, 0, MAX_LINE_LENGTH);
#if defined(ISP_HW_V39)
    snprintf(buffer, MAX_LINE_LENGTH, "%-5s%-10s%-10s", en ? "Y" : "N",
             dm_cfg->filter_g_en ? "Y" : "N", dm_cfg->filter_c_en ? "Y" : "N");
#elif defined(ISP_HW_V33)
    snprintf(buffer, MAX_LINE_LENGTH, "%-5s%-10s%-10s", en ? "Y" : "N", dm_cfg->bypass ? "Y" : "N",
             dm_cfg->g_out_flt_en ? "Y" : "N");
#elif defined(ISP_HW_V35)
    snprintf(buffer, MAX_LINE_LENGTH, "%-5s%-10s%-10s", en ? "Y" : "N", dm_cfg->bypass ? "Y" : "N",
             dm_cfg->g_out_flt_en ? "Y" : "N");
#endif

    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");
}

static void __dump_ccm_coeff(void* self, char* buffer, st_string* result) {
#if defined(ISP_HW_V39)
    struct isp39_isp_params_cfg* params = (struct isp39_isp_params_cfg*)self;
    struct isp39_ccm_cfg* ccm_cfg       = &params->others.ccm_cfg;
#elif defined(ISP_HW_V33)
    struct isp33_isp_params_cfg* params = (struct isp33_isp_params_cfg*)self;
    struct isp33_ccm_cfg* ccm_cfg       = &params->others.ccm_cfg;
#elif defined(ISP_HW_V35)
    struct isp35_isp_params_cfg* params = (struct isp35_isp_params_cfg*)self;
    struct isp33_ccm_cfg* ccm_cfg       = &params->others.ccm_cfg;
#else
    return;
#endif

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s", "coeff0_r",
             "coeff1_r", "coeff2_r", "offset_r", "coeff0_g", "coeff1_g", "coeff2_g", "offset_g");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    snprintf(buffer, MAX_LINE_LENGTH, "%-10d%-10d%-10d%-10d%-10d%-10d%-10d%-10d", ccm_cfg->coeff0_r,
             ccm_cfg->coeff1_r, ccm_cfg->coeff2_r, ccm_cfg->offset_r, ccm_cfg->coeff0_g,
             ccm_cfg->coeff1_g, ccm_cfg->coeff2_g, ccm_cfg->offset_g);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-10s%-10s%-10s%-10s%-10s%-10s%-10s", "coeff0_b", "coeff1_b",
             "coeff2_b", "offset_b", "coeff0_y", "coeff1_y", "coeff2_y");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    snprintf(buffer, MAX_LINE_LENGTH, "%-10d%-10d%-10d%-10d%-10d%-10d%-10d", ccm_cfg->coeff0_b,
             ccm_cfg->coeff1_b, ccm_cfg->coeff2_b, ccm_cfg->offset_b, ccm_cfg->coeff0_y,
             ccm_cfg->coeff1_y, ccm_cfg->coeff2_y);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");
}

static void __dump_ccm_alp(void* self, char* buffer, st_string* result) {
#if defined(ISP_HW_V39)
    struct isp39_isp_params_cfg* params = (struct isp39_isp_params_cfg*)self;
    struct isp39_ccm_cfg* ccm_cfg       = &params->others.ccm_cfg;
#elif defined(ISP_HW_V33)
    struct isp33_isp_params_cfg* params = (struct isp33_isp_params_cfg*)self;
    struct isp33_ccm_cfg* ccm_cfg       = &params->others.ccm_cfg;
#elif defined(ISP_HW_V35)
    struct isp35_isp_params_cfg* params = (struct isp35_isp_params_cfg*)self;
    struct isp33_ccm_cfg* ccm_cfg       = &params->others.ccm_cfg;
#else
    return;
#endif

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-8s%-8s%-8s%-8s%-8s%-8s%-8s%-8s%-8s", "alp_y0", "alp_y1",
             "alp_y2", "alp_y3", "alp_y4", "alp_y5", "alp_y6", "alp_y7", "alp_y8");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    snprintf(buffer, MAX_LINE_LENGTH, "%-8d%-8d%-8d%-8d%-8d%-8d%-8d%-8d%-8d", ccm_cfg->alp_y[0],
             ccm_cfg->alp_y[1], ccm_cfg->alp_y[2], ccm_cfg->alp_y[3], ccm_cfg->alp_y[4],
             ccm_cfg->alp_y[5], ccm_cfg->alp_y[6], ccm_cfg->alp_y[7], ccm_cfg->alp_y[8]);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-8s%-9s%-9s%-9s%-9s%-9s%-9s%-9s%-9s", "alp_y9", "alp_y10",
             "alp_y11", "alp_y12", "alp_y13", "alp_y14", "alp_y15", "alp_y16", "alp_y17");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    snprintf(buffer, MAX_LINE_LENGTH, "%-8d%-9d%-9d%-9d%-9d%-9d%-9d%-9d%-9d", ccm_cfg->alp_y[9],
             ccm_cfg->alp_y[10], ccm_cfg->alp_y[11], ccm_cfg->alp_y[12], ccm_cfg->alp_y[13],
             ccm_cfg->alp_y[14], ccm_cfg->alp_y[15], ccm_cfg->alp_y[16], ccm_cfg->alp_y[17]);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");
}

void active_isp_params_dump_ccm_status(void* self, st_string* result) {
#if defined(ISP_HW_V39)
    struct isp39_isp_params_cfg* params = (struct isp39_isp_params_cfg*)self;
    struct isp39_ccm_cfg* ccm_cfg       = &params->others.ccm_cfg;
#elif defined(ISP_HW_V33)
    struct isp33_isp_params_cfg* params = (struct isp33_isp_params_cfg*)self;
    struct isp33_ccm_cfg* ccm_cfg       = &params->others.ccm_cfg;
#elif defined(ISP_HW_V35)
    struct isp35_isp_params_cfg* params = (struct isp35_isp_params_cfg*)self;
    struct isp33_ccm_cfg* ccm_cfg       = &params->others.ccm_cfg;
#else
    return;
#endif

    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "ccm mod status");

    snprintf(buffer, MAX_LINE_LENGTH, "%-5s%-11s%-12s%-12s%-12s", "en", "hi_adj_en", "enh_adj_en",
             "asym_adj_en", "sat_decay_en");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    bool en = params->module_ens & ISP39_MODULE_CCM;

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-5s%-11d%-12s%-12s%-12s", en ? "Y" : "N",
             ccm_cfg->highy_adjust_dis, ccm_cfg->enh_adj_en ? "Y" : "N",
             ccm_cfg->asym_adj_en ? "Y" : "N", ccm_cfg->sat_decay_en ? "Y" : "N");

    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");

    __dump_ccm_coeff(self, buffer, result);
    __dump_ccm_alp(self, buffer, result);
}

void active_isp_params_dump_gammaout_status(void* self, st_string* result) {
#if defined(ISP_HW_V39)
    struct isp39_isp_params_cfg* params = (struct isp39_isp_params_cfg*)self;
#elif defined(ISP_HW_V33)
    struct isp33_isp_params_cfg* params = (struct isp33_isp_params_cfg*)self;
#elif defined(ISP_HW_V35)
    struct isp35_isp_params_cfg* params = (struct isp35_isp_params_cfg*)self;
#else
    return;
#endif

    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "gammaout mod status");

    snprintf(buffer, MAX_LINE_LENGTH, "%-5s%-10s%-12s%-10s", "en", "equ_segm", "fx4_den_en", "y0");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    bool en                                 = params->module_ens & ISP39_MODULE_GOC;
    struct isp3x_gammaout_cfg* gammaout_cfg = &params->others.gammaout_cfg;

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-5s%-10d%-12s%-10d", en ? "Y" : "N", gammaout_cfg->equ_segm,
             gammaout_cfg->finalx4_dense_en ? "Y" : "N", gammaout_cfg->gamma_y[0]);

    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");
}

#if defined(ISP_HW_V39)
void active_isp_params_dump_cp_status(void* self, st_string* result) {
    struct isp39_isp_params_cfg* params = (struct isp39_isp_params_cfg*)self;

    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "cproc mod status");

    snprintf(buffer, MAX_LINE_LENGTH, "%-5s%-11s%-11s%-11s%-6s%-8s%-5s%-5s", "en", "c_o_range",
             "y_i_range", "y_o_range", "cont", "bright", "sat", "hue");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    bool en                        = params->module_ens & ISP39_MODULE_CPROC;
    struct isp2x_cproc_cfg* cp_cfg = &params->others.cproc_cfg;

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-5s%-11d%-11d%-11d%-6d%-8d%-5d%-5d", en ? "Y" : "N",
             cp_cfg->c_out_range, cp_cfg->y_in_range, cp_cfg->y_out_range, cp_cfg->contrast,
             cp_cfg->brightness, cp_cfg->sat, cp_cfg->hue);

    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");
}
#endif

void active_isp_params_dump_sdg_status(void* self, st_string* result) {
#if defined(ISP_HW_V39)
    struct isp39_isp_params_cfg* params = (struct isp39_isp_params_cfg*)self;
#elif defined(ISP_HW_V33)
    struct isp33_isp_params_cfg* params = (struct isp33_isp_params_cfg*)self;
#elif defined(ISP_HW_V35)
    struct isp35_isp_params_cfg* params = (struct isp35_isp_params_cfg*)self;
#else
    return;
#endif

    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "SDG mod status");

    snprintf(buffer, MAX_LINE_LENGTH, "%-5s", "en");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    bool en = params->module_ens & ISP39_MODULE_SDG;

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-5s", en ? "Y" : "N");

    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");
}

void active_isp_params_dump_drc_status(void* self, st_string* result) {
#if defined(ISP_HW_V39)
    struct isp39_isp_params_cfg* params = (struct isp39_isp_params_cfg*)self;
    struct isp39_drc_cfg* drc_cfg       = &params->others.drc_cfg;
#elif defined(ISP_HW_V33)
    struct isp33_isp_params_cfg* params = (struct isp33_isp_params_cfg*)self;
    struct isp33_drc_cfg* drc_cfg       = &params->others.drc_cfg;
#elif defined(ISP_HW_V35)
    struct isp35_isp_params_cfg* params = (struct isp35_isp_params_cfg*)self;
    struct isp35_drc_cfg* drc_cfg       = &params->others.drc_cfg;
#else
    return;
#endif

    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "drc mod status");

    snprintf(buffer, MAX_LINE_LENGTH, "%-5s%-8s%-10s%-9s", "en", "bypass", "cmps_byp", "gx32_en");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    bool en = params->module_ens & ISP39_MODULE_DRC;

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-5s%-8s%-10s%-9s", en ? "Y" : "N",
             drc_cfg->bypass_en ? "Y" : "N", drc_cfg->cmps_byp_en ? "Y" : "N",
             drc_cfg->gainx32_en ? "Y" : "N");

    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");
}

void active_isp_params_dump_mge_status(void* self, st_string* result) {
#if defined(ISP_HW_V39)
    struct isp39_isp_params_cfg* params = (struct isp39_isp_params_cfg*)self;
    struct isp32_hdrmge_cfg* mge_cfg = &params->others.hdrmge_cfg;
#elif defined(ISP_HW_V33)
    struct isp33_isp_params_cfg* params = (struct isp33_isp_params_cfg*)self;
    struct isp32_hdrmge_cfg* mge_cfg = &params->others.hdrmge_cfg;
#elif defined(ISP_HW_V35)
    struct isp35_isp_params_cfg* params = (struct isp35_isp_params_cfg*)self;
    struct isp35_hdrmge_cfg* mge_cfg = &params->others.hdrmge_cfg;
#else
    return;
#endif

    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "hdr merge mod status");

    snprintf(buffer, MAX_LINE_LENGTH, "%-5s%-8s%-6s", "en", "s_base", "mode");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    bool en                          = params->module_ens & ISP39_MODULE_HDRMGE;
#if defined(ISP_HW_V39) || defined(ISP_HW_V33)
    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-5s%-8d%-6d", en ? "Y" : "N", mge_cfg->s_base,
             mge_cfg->mode);
#elif defined(ISP_HW_V35)
    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-5s%-8d%-6d", en ? "Y" : "N", mge_cfg->short_base_en,
            mge_cfg->frame_mode);
#endif
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");
}

#if defined(ISP_HW_V39)
void active_isp_params_dump_dhaz_status(void* self, st_string* result) {
    struct isp39_isp_params_cfg* params = (struct isp39_isp_params_cfg*)self;
    struct isp39_dhaz_cfg* dhaz_cfg     = &params->others.dhaz_cfg;

    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "dehaze mod status");

    snprintf(buffer, MAX_LINE_LENGTH, "%-5s%-7s%-9s%-10s%-11s%-8s%-10s%-12s%-13s", "en", "dc_en",
             "hist_en", "map_mode", "air_lc_en", "enh_en", "round_en", "color_devi", "enh_luma_en");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    bool en = params->module_ens & ISP39_MODULE_DHAZ;

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-5s%-7s%-9s%-10d%-11s%-8s%-10s%-12s%-13s", en ? "Y" : "N",
             dhaz_cfg->dc_en ? "Y" : "N", dhaz_cfg->hist_en ? "Y" : "N", dhaz_cfg->map_mode,
             dhaz_cfg->air_lc_en ? "Y" : "N", dhaz_cfg->enhance_en ? "Y" : "N",
             dhaz_cfg->round_en ? "Y" : "N", dhaz_cfg->color_deviate_en ? "Y" : "N",
             dhaz_cfg->enh_luma_en ? "Y" : "N");

    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");
}
#endif

#if defined(ISP_HW_V39)
void active_isp_params_dump_3dlut_status(void* self, st_string* result) {
    struct isp39_isp_params_cfg* params = (struct isp39_isp_params_cfg*)self;
    struct isp2x_3dlut_cfg* lut3d_cfg   = &params->others.isp3dlut_cfg;

    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "3dlut mod status");

    snprintf(buffer, MAX_LINE_LENGTH, "%-5s%-8s%-6s%-8s%-8s%-8s", "en", "bypass", "size", "lut_r0",
             "lut_g0", "lut_b0");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    bool en = params->module_ens & ISP39_MODULE_3DLUT;

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-5s%-8s%-6d%-8d%-8d%-8d", en ? "Y" : "N",
             lut3d_cfg->bypass_en ? "Y" : "N", lut3d_cfg->actual_size, lut3d_cfg->lut_r[0],
             lut3d_cfg->lut_g[0], lut3d_cfg->lut_r[0]);

    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");
}
#endif

void active_isp_params_dump_ldch_status(void* self, st_string* result) {
#if defined(ISP_HW_V39)
    struct isp39_isp_params_cfg* params = (struct isp39_isp_params_cfg*)self;
    struct isp39_ldch_cfg* ldch_cfg     = &params->others.ldch_cfg;
#elif defined(ISP_HW_V33)
    struct isp33_isp_params_cfg* params = (struct isp33_isp_params_cfg*)self;
    struct isp32_ldch_cfg* ldch_cfg     = &params->others.ldch_cfg;
#elif defined(ISP_HW_V35)
    struct isp35_isp_params_cfg* params = (struct isp35_isp_params_cfg*)self;
    struct isp32_ldch_cfg* ldch_cfg     = &params->others.ldch_cfg;
#else
    return;
#endif

    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "ldch mod status");

    snprintf(buffer, MAX_LINE_LENGTH, "%-5s%-9s%-12s%-10s%-11s%-9s", "en", "fe_dis", "sample_avr",
             "bic_mode", "force_map", "map13p3");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    bool en = params->module_ens & ISP39_MODULE_LDCH;

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-5s%-9s%-12s%-10s%-11s%-9s", en ? "Y" : "N",
             ldch_cfg->frm_end_dis ? "Y" : "N", ldch_cfg->sample_avr_en ? "Y" : "N",
             ldch_cfg->bic_mode_en ? "Y" : "N", ldch_cfg->force_map_en ? "Y" : "N",
             ldch_cfg->map13p3_en ? "Y" : "N");

    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-8s%-8s%-8s", "mesh_x", "mesh_y", "buf_fd");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-8d%-8d%-8d", ldch_cfg->hsize, ldch_cfg->vsize,
             ldch_cfg->buf_fd);

    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");
}

#if defined(ISP_HW_V39)
void active_isp_params_dump_ldcv_status(void* self, st_string* result) {
    struct isp39_isp_params_cfg* params = (struct isp39_isp_params_cfg*)self;
    struct isp39_ldcv_cfg* ldcv_cfg     = &params->others.ldcv_cfg;

    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "ldcv mod status");

    snprintf(buffer, MAX_LINE_LENGTH, "%-5s%-7s%-12s%-11s%-9s", "en", "thumb", "dth_bypass",
             "force_map", "map13p3");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    bool en = params->module_ens & ISP39_MODULE_LDCV;

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-5s%-7d%-12s%-11s%-9s", en ? "Y" : "N",
             ldcv_cfg->thumb_mode, ldcv_cfg->dth_bypass ? "Y" : "N",
             ldcv_cfg->force_map_en ? "Y" : "N", ldcv_cfg->map13p3_en ? "Y" : "N");

    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-6s%-8s%-8s%-8s", "out_v", "mesh_x", "mesh_y", "buf_fd");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-6d%-8d%-8d%-8d", ldcv_cfg->out_vsize, ldcv_cfg->hsize,
             ldcv_cfg->vsize, ldcv_cfg->buf_fd);

    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");
}
#endif

void active_isp_params_dump_ldc_status(void* self, st_string* result) {
#if defined(ISP_HW_V39)
    active_isp_params_dump_ldch_status(self, result);
    active_isp_params_dump_ldcv_status(self, result);
#elif defined(ISP_HW_V33)
    active_isp_params_dump_ldch_status(self, result);
#elif defined(ISP_HW_V35)
    active_isp_params_dump_ldch_status(self, result);
#endif
}

void active_isp_params_dump_bay3d_status(void* self, st_string* result) {
#if defined(ISP_HW_V39)
    struct isp39_isp_params_cfg* params = (struct isp39_isp_params_cfg*)self;
    struct isp39_bay3d_cfg* bay3d_cfg   = &params->others.bay3d_cfg;
#elif defined(ISP_HW_V33)
    struct isp33_isp_params_cfg* params = (struct isp33_isp_params_cfg*)self;
    struct isp33_bay3d_cfg* bay3d_cfg   = &params->others.bay3d_cfg;
#elif defined(ISP_HW_V35)
    struct isp35_isp_params_cfg* params = (struct isp35_isp_params_cfg*)self;
    struct isp35_bay3d_cfg* bay3d_cfg   = &params->others.bay3d_cfg;
#else
    return;
#endif

    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "bay3d mod status");

    snprintf(buffer, MAX_LINE_LENGTH, "%-5s%-8s%-14s", "en", "bypass", "iirsparse_en");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    bool en = params->module_ens & ISP39_MODULE_BAY3D;
#if defined(ISP_HW_V39) || defined(ISP_HW_V33)
    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-5s%-8s%-14s", en ? "Y" : "N",
             bay3d_cfg->bypass_en ? "Y" : "N", bay3d_cfg->iirsparse_en ? "Y" : "N");
#elif defined(ISP_HW_V35)
    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-5s%-8s%-14s", en ? "Y" : "N",
             bay3d_cfg->bypass_en ? "Y" : "N", bay3d_cfg->iir_wr_src ? "Y" : "N");
#endif
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");
}

void active_isp_params_dump_ynr_status(void* self, st_string* result) {
#if defined(ISP_HW_V39)
    struct isp39_isp_params_cfg* params = (struct isp39_isp_params_cfg*)self;
    struct isp39_ynr_cfg* ynr_cfg       = &params->others.ynr_cfg;
#elif defined(ISP_HW_V33)
    struct isp33_isp_params_cfg* params = (struct isp33_isp_params_cfg*)self;
    struct isp33_ynr_cfg* ynr_cfg       = &params->others.ynr_cfg;
#elif defined(ISP_HW_V35)
    struct isp35_isp_params_cfg* params = (struct isp35_isp_params_cfg*)self;
    struct isp35_ynr_cfg* ynr_cfg       = &params->others.ynr_cfg;
#else
    return;
#endif

    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "ynr mod status");

#if defined(ISP_HW_V39)
    snprintf(buffer, MAX_LINE_LENGTH, "%-5s%-12s%-12s%-12s%-10s%-9s%-5s", "en", "lospnr_byp",
             "hispnr_byp", "exgain_byp", "glo_gain", "gain_alp", "rnr");
#elif defined(ISP_HW_V33)
    snprintf(buffer, MAX_LINE_LENGTH, "%-5s%-12s%-12s%-12s%-10s%-9s%-5s", "en", "lospnr_byp",
             "hispnr_byp", "mispnr_byp", "glo_gain", "gain_alp", "rnr");
#elif defined(ISP_HW_V35)
    snprintf(buffer, MAX_LINE_LENGTH, "%-5s%-12s%-12s%-12s%-10s%-9s%-5s", "en", "lospnr_byp",
             "hispnr_byp", "mispnr_byp", "glo_gain", "gain_alp", "rnr");
#endif
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    bool en      = params->module_ens & ISP39_MODULE_YNR;
    bool en_upd  = params->module_en_update & ISP39_MODULE_YNR;
    bool cfg_upd = params->module_cfg_update & ISP39_MODULE_YNR;

    memset(buffer, 0, MAX_LINE_LENGTH);
#if defined(ISP_HW_V39)
    snprintf(buffer, MAX_LINE_LENGTH, "%-5s%-12s%-12s%-12s%-10d%-9d%-5s", en ? "Y" : "N",
             ynr_cfg->lospnr_bypass ? "Y" : "N", ynr_cfg->hispnr_bypass ? "Y" : "N",
             ynr_cfg->exgain_bypass ? "Y" : "N", ynr_cfg->global_set_gain,
             ynr_cfg->gain_merge_alpha, ynr_cfg->rnr_en ? "Y" : "N");
#elif defined(ISP_HW_V33)
    snprintf(buffer, MAX_LINE_LENGTH, "%-5s%-12s%-12s%-12s%-10d%-9d%-5s", en ? "Y" : "N",
             ynr_cfg->lo_spnr_bypass ? "Y" : "N", ynr_cfg->hi_spnr_bypass ? "Y" : "N",
             ynr_cfg->mi_spnr_bypass ? "Y" : "N", ynr_cfg->global_set_gain,
             ynr_cfg->gain_merge_alpha, ynr_cfg->rnr_en ? "Y" : "N");
#elif defined(ISP_HW_V35)
    snprintf(buffer, MAX_LINE_LENGTH, "%-5s%-12s%-12s%-12s%-10d%-9d%-5s", en ? "Y" : "N",
             ynr_cfg->lo_spnr_bypass ? "Y" : "N", ynr_cfg->hi_spnr_bypass ? "Y" : "N",
             ynr_cfg->mi_spnr_bypass ? "Y" : "N", ynr_cfg->global_set_gain,
             ynr_cfg->gain_merge_alpha, ynr_cfg->rnr_en ? "Y" : "N");
#endif

    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");
}

void active_isp_params_dump_cnr_status(void* self, st_string* result) {
#if defined(ISP_HW_V39)
    struct isp39_isp_params_cfg* params = (struct isp39_isp_params_cfg*)self;
    struct isp39_cnr_cfg* cnr_cfg       = &params->others.cnr_cfg;
#elif defined(ISP_HW_V33)
    struct isp33_isp_params_cfg* params = (struct isp33_isp_params_cfg*)self;
    struct isp33_cnr_cfg* cnr_cfg       = &params->others.cnr_cfg;
#elif defined(ISP_HW_V35)
    struct isp35_isp_params_cfg* params = (struct isp35_isp_params_cfg*)self;
    struct isp35_cnr_cfg* cnr_cfg       = &params->others.cnr_cfg;
#else
    return;
#endif

    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "cnr mod status");

    snprintf(buffer, MAX_LINE_LENGTH, "%-5s%-11s%-13s%-12s%-12s%-12s", "en", "exgain_byp",
             "yuv422_mode", "thumb_mode", "hiflt_wgt0", "loflt_coeff");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    bool en = params->module_ens & ISP39_MODULE_CNR;

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-5s%-11s%-13d%-12d%-12d%-12d", en ? "Y" : "N",
             cnr_cfg->exgain_bypass ? "Y" : "N", cnr_cfg->yuv422_mode, cnr_cfg->thumb_mode,
             cnr_cfg->hiflt_wgt0_mode, cnr_cfg->loflt_coeff);

    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");
}

void active_isp_params_dump_sharp_status(void* self, st_string* result) {
#if defined(ISP_HW_V39)
    struct isp39_isp_params_cfg* params = (struct isp39_isp_params_cfg*)self;
    struct isp39_sharp_cfg* shp_cfg     = &params->others.sharp_cfg;
#elif defined(ISP_HW_V33)
    struct isp33_isp_params_cfg* params = (struct isp33_isp_params_cfg*)self;
    struct isp33_sharp_cfg* shp_cfg     = &params->others.sharp_cfg;
#elif defined(ISP_HW_V35)
    struct isp35_isp_params_cfg* params = (struct isp35_isp_params_cfg*)self;
    struct isp35_sharp_cfg* shp_cfg     = &params->others.sharp_cfg;
#else
    return;
#endif

    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "sharp mod status");

#if defined(ISP_HW_V39)
    snprintf(buffer, MAX_LINE_LENGTH, "%-5s%-8s%-13s%-13s%-13s%-12s%-9s%-9s%-11s%-9s", "en",
             "bypass", "center_mode", "lo_gain_byp", "radius_step", "noise_clip", "clipldx",
             "baselmg", "noise_flt", "tex2wgt");
#elif defined(ISP_HW_V33)
    snprintf(buffer, MAX_LINE_LENGTH, "%-5s%-8s%-13s%-10s%-10s%-12s%-11s%-13s%-12s%-10s%-7s", "en",
             "bypass", "lo_gain_byp", "tex_est", "max_flt", "fusion_wgt", "noise_calc",
             "radius_step", "curve_clip", "gain_wgt", "lp_en");
#elif defined(ISP_HW_V35)
    snprintf(buffer, MAX_LINE_LENGTH, "%-5s%-8s%-13s%-10s%-10s%-12s%-11s%-13s%-12s%-10s%-7s", "en",
             "bypass", "lo_gain_byp", "tex_est", "max_flt", "fusion_wgt", "noise_calc",
             "radius_step", "curve_clip", "gain_wgt", "lp_en");
#endif
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    bool en = params->module_ens & ISP39_MODULE_SHARP;

    memset(buffer, 0, MAX_LINE_LENGTH);
#if defined(ISP_HW_V39)
    snprintf(buffer, MAX_LINE_LENGTH, "%-5s%-8s%-13d%-13d%-13d%-12d%-9d%-9d%-11d%-9s",
             en ? "Y" : "N", shp_cfg->bypass ? "Y" : "N", shp_cfg->center_mode,
             shp_cfg->local_gain_bypass, shp_cfg->radius_step_mode, shp_cfg->noise_clip_mode,
             shp_cfg->clipldx_sel, shp_cfg->baselmg_sel, shp_cfg->noise_filt_sel,
             shp_cfg->tex2wgt_en ? "Y" : "N");
#elif defined(ISP_HW_V33)
    snprintf(buffer, MAX_LINE_LENGTH, "%-5s%-8s%-13d%-10d%-10d%-12d%-11d%-13d%-12d%-10d%-7s",
             en ? "Y" : "N", shp_cfg->bypass ? "Y" : "N", shp_cfg->local_gain_bypass,
             shp_cfg->tex_est_mode, shp_cfg->max_min_flt_mode, shp_cfg->detail_fusion_wgt_mode,
             shp_cfg->noise_calc_mode, shp_cfg->radius_step_mode, shp_cfg->noise_curve_mode,
             shp_cfg->gain_wgt_mode, shp_cfg->detail_lp_en ? "Y" : "N");
#elif defined(ISP_HW_V35)
    snprintf(buffer, MAX_LINE_LENGTH, "%-5s%-8s%-13d%-10d%-10d%-12d%-11d%-13d%-12d%-10d%-7s",
             en ? "Y" : "N", shp_cfg->bypass ? "Y" : "N", shp_cfg->local_gain_bypass,
             shp_cfg->tex_est_mode, shp_cfg->max_min_flt_mode, shp_cfg->detail_fusion_wgt_mode,
             shp_cfg->noise_calc_mode, shp_cfg->radius_step_mode, shp_cfg->noise_curve_mode,
             shp_cfg->gain_wgt_mode, shp_cfg->detail_lp_en ? "Y" : "N");
#endif

    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");
}

void active_isp_params_dump_cac_status(void* self, st_string* result) {
#if defined(ISP_HW_V39)
    struct isp39_isp_params_cfg* params = (struct isp39_isp_params_cfg*)self;
    struct isp32_cac_cfg* cac_cfg       = &params->others.cac_cfg;
#elif defined(ISP_HW_V33)
    struct isp33_isp_params_cfg* params = (struct isp33_isp_params_cfg*)self;
    struct isp33_cac_cfg* cac_cfg       = &params->others.cac_cfg;
#elif defined(ISP_HW_V35)
    struct isp35_isp_params_cfg* params = (struct isp35_isp_params_cfg*)self;
    struct isp33_cac_cfg* cac_cfg       = &params->others.cac_cfg;
#else
    return;
#endif

    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "cac mod status");

#if defined(ISP_HW_V39)
    snprintf(buffer, MAX_LINE_LENGTH, "%-5s%-8s%-11s%-13s%-13s%-11s", "en", "bypass", "center_en",
             "clip_g_mode", "edge_detect", "neg_clip0");
#elif defined(ISP_HW_V33)
    snprintf(buffer, MAX_LINE_LENGTH, "%-5s%-8s%-16s%-14s%-14s", "en", "bypass", "edge_detect_en",
             "neg_clip0_en", "wgt_color_en");
#elif defined(ISP_HW_V35)
    snprintf(buffer, MAX_LINE_LENGTH, "%-5s%-8s%-16s%-14s%-14s", "en", "bypass", "edge_detect_en",
             "neg_clip0_en", "wgt_color_en");
#endif
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    bool en = params->module_ens & ISP39_MODULE_CAC;

    memset(buffer, 0, MAX_LINE_LENGTH);
#if defined(ISP_HW_V39)
    snprintf(buffer, MAX_LINE_LENGTH, "%-5s%-8s%-11s%-13d%-13s%-11s", en ? "Y" : "N",
             cac_cfg->bypass_en ? "Y" : "N", cac_cfg->center_en ? "Y" : "N", cac_cfg->clip_g_mode,
             cac_cfg->edge_detect_en ? "Y" : "N", cac_cfg->neg_clip0_en ? "Y" : "N");
#elif defined(ISP_HW_V33)
    snprintf(buffer, MAX_LINE_LENGTH, "%-5s%-8s%-16s%-14s%-14s", en ? "Y" : "N",
             cac_cfg->bypass_en ? "Y" : "N", cac_cfg->edge_detect_en ? "Y" : "N",
             cac_cfg->neg_clip0_en ? "Y" : "N", cac_cfg->wgt_color_en ? "Y" : "N");
#elif defined(ISP_HW_V35)
    snprintf(buffer, MAX_LINE_LENGTH, "%-5s%-8s%-16s%-14s%-14s", en ? "Y" : "N",
             cac_cfg->bypass_en ? "Y" : "N", cac_cfg->edge_detect_en ? "Y" : "N",
             cac_cfg->neg_clip0_en ? "Y" : "N", cac_cfg->wgt_color_en ? "Y" : "N");
#endif

    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");
}

void active_isp_params_dump_gain_status(void* self, st_string* result) {
#if defined(ISP_HW_V39)
    struct isp39_isp_params_cfg* params = (struct isp39_isp_params_cfg*)self;
#elif defined(ISP_HW_V33)
    struct isp33_isp_params_cfg* params = (struct isp33_isp_params_cfg*)self;
#elif defined(ISP_HW_V35)
    struct isp35_isp_params_cfg* params = (struct isp35_isp_params_cfg*)self;
#else
    return;
#endif

    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "gain mod status");

    snprintf(buffer, MAX_LINE_LENGTH, "%-5s%-8s%-8s%-8s", "en", "gain0", "gain1", "gain2");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    bool en                         = params->module_ens & ISP39_MODULE_GAIN;
    struct isp3x_gain_cfg* gain_cfg = &params->others.gain_cfg;

    snprintf(buffer, MAX_LINE_LENGTH, "%-5s%-8d%-8d%-8d", en ? "Y" : "N", gain_cfg->g0,
             gain_cfg->g1, gain_cfg->g2);

    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");
}

void active_isp_params_dump_csm_status(void* self, st_string* result) {
#if defined(ISP_HW_V39)
    struct isp39_isp_params_cfg* params = (struct isp39_isp_params_cfg*)self;
#elif defined(ISP_HW_V33)
    struct isp33_isp_params_cfg* params = (struct isp33_isp_params_cfg*)self;
#elif defined(ISP_HW_V35)
    struct isp35_isp_params_cfg* params = (struct isp35_isp_params_cfg*)self;
#else
    return;
#endif

    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "csm mod status");

    snprintf(buffer, MAX_LINE_LENGTH, "%-5s%-12s%-10s%-10s", "en", "full_range", "y_offset",
             "c_offset");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    bool en                       = params->module_ens & ISP39_MODULE_CSM;
    struct isp21_csm_cfg* csm_cfg = &params->others.csm_cfg;

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-5s%-12d%-10d%-10d", en ? "Y" : "N",
             csm_cfg->csm_full_range, csm_cfg->csm_y_offset, csm_cfg->csm_c_offset);

    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");
}

void active_isp_params_dump_cgc_status(void* self, st_string* result) {
#if defined(ISP_HW_V39)
    struct isp39_isp_params_cfg* params = (struct isp39_isp_params_cfg*)self;
#elif defined(ISP_HW_V33)
    struct isp33_isp_params_cfg* params = (struct isp33_isp_params_cfg*)self;
#elif defined(ISP_HW_V35)
    struct isp35_isp_params_cfg* params = (struct isp35_isp_params_cfg*)self;
#else
    return;
#endif

    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "cgc mod status");

    snprintf(buffer, MAX_LINE_LENGTH, "%-5s%-11s%-10s", "en", "yuv_limit", "ratio_en");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    bool en                       = params->module_ens & ISP39_MODULE_CGC;
    struct isp21_cgc_cfg* cgc_cfg = &params->others.cgc_cfg;

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-5s%-11d%-10s", en ? "Y" : "N", cgc_cfg->yuv_limit,
             cgc_cfg->ratio_en ? "Y" : "N");

    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");
}

#if defined(ISP_HW_V39)
void active_isp_params_dump_yme_status(void* self, st_string* result) {
    struct isp39_isp_params_cfg* params = (struct isp39_isp_params_cfg*)self;
    struct isp39_yuvme_cfg* yme_cfg     = &params->others.yuvme_cfg;

    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "yme mod status");

    snprintf(buffer, MAX_LINE_LENGTH, "%-5s%-8s%-13s", "en", "bypass", "tnr_wgt0_en");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    uint64_t seq = params->frame_id;
    bool en      = params->module_ens & ISP39_MODULE_YUVME;

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-5s%-8s%-13s", en ? "Y" : "N", yme_cfg->bypass ? "Y" : "N",
             yme_cfg->tnr_wgt0_en ? "Y" : "N");

    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");
}
#endif

void active_isp_params_dump_aec_status(void* self, st_string* result) {

    char buffer[MAX_LINE_LENGTH] = {0};

#if defined(ISP_HW_V39)
    struct isp39_isp_params_cfg* params = (struct isp39_isp_params_cfg*)self;
    struct isp39_isp_meas_cfg* meas_cfg     = &params->meas;
#elif defined(ISP_HW_V33)
    struct isp33_isp_params_cfg* params = (struct isp33_isp_params_cfg*)self;
    struct isp33_isp_meas_cfg* meas_cfg     = &params->meas;
#elif defined(ISP_HW_V35)
    struct isp35_isp_params_cfg* params = (struct isp35_isp_params_cfg*)self;
    struct isp35_isp_meas_cfg* meas_cfg     = &params->meas;
#else
    return;
#endif

#ifndef ISP_HW_V35  //TODO
    aiq_info_dump_title(result, "aec hwi params");

    snprintf(buffer, MAX_LINE_LENGTH, "%-5s%-8s%-8s", "en", "aeswap", "aesel");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    u64 module_en = ISP39_MODULE_RAWAE0 | ISP39_MODULE_RAWAE3 | ISP39_MODULE_RAWHIST0 | ISP39_MODULE_RAWHIST3;
    bool en = params->module_ens & module_en;

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-5s%-8d%-8d", en == 1 ? "Y" : "N", meas_cfg->rawae0.rawae_sel, meas_cfg->rawae3.rawae_sel);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-8s%-13s%-13s%-13s%-13s", "rawae0:", "win_off_h", "win_off_v", "win_size_h", "win_size_v");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-8s%-13d%-13d%-13d%-13d", "",
             meas_cfg->rawae0.win.h_offs, meas_cfg->rawae0.win.v_offs,
             meas_cfg->rawae0.win.h_size, meas_cfg->rawae0.win.v_size);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-8s%-13s%-13s%-13s%-13s", "rawae3:", "win_off_h", "win_off_v", "win_size_h", "win_size_v");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-8s%-13d%-13d%-13d%-13d", "",
             meas_cfg->rawae3.win.h_offs, meas_cfg->rawae3.win.v_offs,
             meas_cfg->rawae3.win.h_size, meas_cfg->rawae3.win.v_size);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-10s%-13s%-13s%-13s%-13s", "rawhist0:", "win_off_h", "win_off_v", "win_size_h", "win_size_v");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-10s%-13d%-13d%-13d%-13d", "",
             meas_cfg->rawhist0.win.h_offs, meas_cfg->rawhist0.win.v_offs,
             meas_cfg->rawhist0.win.h_size, meas_cfg->rawhist0.win.v_size);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-10s%-13s%-13s%-13s%-13s", "rawhist3:", "win_off_h", "win_off_v", "win_size_h", "win_size_v");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-10s%-13d%-13d%-13d%-13d", "",
             meas_cfg->rawhist3.win.h_offs, meas_cfg->rawhist3.win.v_offs,
             meas_cfg->rawhist3.win.h_size, meas_cfg->rawhist3.win.v_size);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");
#endif
}


#define DUMP_INFO(_type, _func) \
    [_type] = {                 \
        .type = _type,          \
        .dump = _func,          \
    }

struct params_dump_info {
    int32_t type;
    const char* name;
    void (*dump)(void* self, st_string* result);
};

static const struct params_dump_info params_dump[] = {
    DUMP_INFO(RESULT_TYPE_DEBAYER_PARAM, active_isp_params_dump_debayer_status),
    DUMP_INFO(RESULT_TYPE_AESTATS_PARAM, active_isp_params_dump_aec_status),
    DUMP_INFO(RESULT_TYPE_AWB_PARAM, active_isp_params_dump_rawawb_status),
    DUMP_INFO(RESULT_TYPE_AWBGAIN_PARAM, active_isp_params_dump_wbgain_status),
    DUMP_INFO(RESULT_TYPE_CCM_PARAM, active_isp_params_dump_ccm_status),
    DUMP_INFO(RESULT_TYPE_AF_PARAM, NULL),
    DUMP_INFO(RESULT_TYPE_TNR_PARAM, active_isp_params_dump_bay3d_status),
    DUMP_INFO(RESULT_TYPE_YNR_PARAM, active_isp_params_dump_ynr_status),
    DUMP_INFO(RESULT_TYPE_UVNR_PARAM, active_isp_params_dump_cnr_status),
    DUMP_INFO(RESULT_TYPE_MERGE_PARAM, active_isp_params_dump_mge_status),
    DUMP_INFO(RESULT_TYPE_HISTEQ_PARAM, NULL),
    DUMP_INFO(RESULT_TYPE_SHARPEN_PARAM, active_isp_params_dump_sharp_status),
    DUMP_INFO(RESULT_TYPE_BLC_PARAM, active_isp_params_dump_bls_status),
    DUMP_INFO(RESULT_TYPE_CSM_PARAM, active_isp_params_dump_csm_status),
    DUMP_INFO(RESULT_TYPE_MOTION_PARAM, NULL),
    DUMP_INFO(RESULT_TYPE_DPCC_PARAM, active_isp_params_dump_dpc_status),
    DUMP_INFO(RESULT_TYPE_AGAMMA_PARAM, active_isp_params_dump_gammaout_status),
    DUMP_INFO(RESULT_TYPE_LSC_PARAM, active_isp_params_dump_lsc_status),
    DUMP_INFO(RESULT_TYPE_DRC_PARAM, active_isp_params_dump_drc_status),
    DUMP_INFO(RESULT_TYPE_GIC_PARAM, active_isp_params_dump_gic_status),
    DUMP_INFO(RESULT_TYPE_CGC_PARAM, active_isp_params_dump_cgc_status),
    DUMP_INFO(RESULT_TYPE_IE_PARAM, NULL),
    DUMP_INFO(RESULT_TYPE_GAIN_PARAM, active_isp_params_dump_gain_status),
    DUMP_INFO(RESULT_TYPE_CAC_PARAM, active_isp_params_dump_cac_status),
#if defined(ISP_HW_V39)
    DUMP_INFO(RESULT_TYPE_DEHAZE_PARAM, active_isp_params_dump_dhaz_status),
    DUMP_INFO(RESULT_TYPE_RGBIR_PARAM, active_isp_params_dump_rgbir_status),
    DUMP_INFO(RESULT_TYPE_CP_PARAM, active_isp_params_dump_cp_status),
    DUMP_INFO(RESULT_TYPE_LUT3D_PARAM, active_isp_params_dump_3dlut_status),
#endif
    DUMP_INFO(RESULT_TYPE_LDC_PARAM, active_isp_params_dump_ldc_status),
};

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif
#define DUMP_MODS ARRAY_SIZE(params_dump)

void active_isp_params_dump_by_type(void* dumper, int type, st_string* result) {
#if defined(ISP_HW_V39)
    struct isp39_isp_params_cfg isp_params;
    unsigned long cmd = RKISP_CMD_GET_PARAMS_V39;
#elif defined(ISP_HW_V33)
    struct isp33_isp_params_cfg isp_params;
    unsigned long cmd = RKISP_CMD_GET_PARAMS_V33;
#elif defined(ISP_HW_V35)
    struct isp35_isp_params_cfg isp_params;
    unsigned long cmd = RKISP_CMD_GET_PARAMS_V35;
#else
    return -1;
#endif

    AiqCamHwBase_t* pCamHw = (AiqCamHwBase_t*)dumper;

    int ret = pCamHw->mIspParamsDev->io_control(pCamHw->mIspParamsDev, cmd, &isp_params);
    if (ret < 0) {
        LOGW("Not supported by ISP driver.");
        return;
    }

    for (size_t pos = 0; pos < DUMP_MODS; pos++) {
        if ((params_dump[pos].type == type) && params_dump[pos].dump) {
            const char* name = NULL;
            name = aiq_notifier_notify_subscriber_name(&((AiqCamHwBase_t*)dumper)->notifier,
                                                       Cam3aResult2NotifierType[type]);
            aiq_info_dump_submod_name(result, name);
            active_isp_params_dump_mod_param(&isp_params, pCamHw->mCamPhyId, result);

            params_dump[pos].dump(&isp_params, result);
        }
    }
}
