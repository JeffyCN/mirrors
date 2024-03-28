/*
 * Copyright (c) 2024 Rockchip Eletronics Co., Ltd.
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
 */
#include "Isp39Params.h"
#include "rk_aiq_isp39_modules.h"

namespace RkCam {

#define ISP2X_WBGAIN_FIXSCALE_BIT  8
#define ISP3X_WBGAIN_INTSCALE_BIT  8


void Isp39Params::convertAiqAwbGainToIsp39Params(struct isp39_isp_params_cfg& isp_cfg,
        const rk_aiq_wb_gain_v32_t& awb_gain, bool awb_gain_update)
{

#if RKAIQ_HAVE_AWB_V32

    if(awb_gain_update) {
        isp_cfg.module_ens |= 1LL << RK_ISP2X_AWB_GAIN_ID;
        isp_cfg.module_cfg_update |= 1LL << RK_ISP2X_AWB_GAIN_ID;
        isp_cfg.module_en_update |= 1LL << RK_ISP2X_AWB_GAIN_ID;
    } else {
        return;
    }

    struct isp32_awb_gain_cfg *  cfg = &isp_cfg.others.awb_gain_cfg;
    uint16_t max_wb_gain = (1 << (ISP2X_WBGAIN_FIXSCALE_BIT + 3)) - 1;
    rk_aiq_wb_gain_v32_t awb_gain1 = awb_gain;
#if 0 //to do
    if(blc != nullptr && blc->v0.enable) {
        awb_gain1.bgain *= (float)((1 << ISP2X_BLC_BIT_MAX) - 1) / ((1 << ISP2X_BLC_BIT_MAX) - 1 - blc->v0.blc_b);
        awb_gain1.gbgain *= (float)((1 << ISP2X_BLC_BIT_MAX) - 1) / ((1 << ISP2X_BLC_BIT_MAX) - 1 - blc->v0.blc_gb);
        awb_gain1.rgain *= (float)((1 << ISP2X_BLC_BIT_MAX) - 1) / ((1 << ISP2X_BLC_BIT_MAX) - 1 - blc->v0.blc_r);
        awb_gain1.grgain *= (float)((1 << ISP2X_BLC_BIT_MAX) - 1) / ((1 << ISP2X_BLC_BIT_MAX) - 1 - blc->v0.blc_gr);
    }
#endif
    uint16_t R = (uint16_t)(0.5 + awb_gain1.rgain * (1 << ISP2X_WBGAIN_FIXSCALE_BIT));
    uint16_t B = (uint16_t)(0.5 + awb_gain1.bgain * (1 << ISP2X_WBGAIN_FIXSCALE_BIT));
    uint16_t Gr = (uint16_t)(0.5 + awb_gain1.grgain * (1 << ISP2X_WBGAIN_FIXSCALE_BIT));
    uint16_t Gb = (uint16_t)(0.5 + awb_gain1.gbgain * (1 << ISP2X_WBGAIN_FIXSCALE_BIT));
    if(awb_gain.applyPosition == IN_AWBGAIN0) {
        cfg->gain0_red       = R > max_wb_gain ? max_wb_gain : R;
        cfg->gain0_blue      = B > max_wb_gain ? max_wb_gain : B;
        cfg->gain0_green_r   = Gr > max_wb_gain ? max_wb_gain : Gr ;
        cfg->gain0_green_b   = Gb > max_wb_gain ? max_wb_gain : Gb;
        cfg->gain1_red       = R > max_wb_gain ? max_wb_gain : R;
        cfg->gain1_blue      = B > max_wb_gain ? max_wb_gain : B;
        cfg->gain1_green_r   = Gr > max_wb_gain ? max_wb_gain : Gr ;
        cfg->gain1_green_b   = Gb > max_wb_gain ? max_wb_gain : Gb;
        cfg->gain2_red       = R > max_wb_gain ? max_wb_gain : R;
        cfg->gain2_blue      = B > max_wb_gain ? max_wb_gain : B;
        cfg->gain2_green_r   = Gr > max_wb_gain ? max_wb_gain : Gr ;
        cfg->gain2_green_b   = Gb > max_wb_gain ? max_wb_gain : Gb;
        cfg->awb1_gain_r = (1 << ISP2X_WBGAIN_FIXSCALE_BIT);
        cfg->awb1_gain_b = (1 << ISP2X_WBGAIN_FIXSCALE_BIT);
        cfg->awb1_gain_gr = (1 << ISP2X_WBGAIN_FIXSCALE_BIT);
        cfg->awb1_gain_gb = (1 << ISP2X_WBGAIN_FIXSCALE_BIT);
    } else {
        cfg->gain0_red       = (1 << ISP2X_WBGAIN_FIXSCALE_BIT);
        cfg->gain0_blue      = (1 << ISP2X_WBGAIN_FIXSCALE_BIT);
        cfg->gain0_green_r   = (1 << ISP2X_WBGAIN_FIXSCALE_BIT);
        cfg->gain0_green_b   = (1 << ISP2X_WBGAIN_FIXSCALE_BIT);
        cfg->gain1_red       = (1 << ISP2X_WBGAIN_FIXSCALE_BIT);
        cfg->gain1_blue      = (1 << ISP2X_WBGAIN_FIXSCALE_BIT);
        cfg->gain1_green_r   = (1 << ISP2X_WBGAIN_FIXSCALE_BIT);
        cfg->gain1_green_b   = (1 << ISP2X_WBGAIN_FIXSCALE_BIT);
        cfg->gain2_red       = (1 << ISP2X_WBGAIN_FIXSCALE_BIT);
        cfg->gain2_blue      = (1 << ISP2X_WBGAIN_FIXSCALE_BIT);
        cfg->gain2_green_r   = (1 << ISP2X_WBGAIN_FIXSCALE_BIT);
        cfg->gain2_green_b   = (1 << ISP2X_WBGAIN_FIXSCALE_BIT);
        cfg->awb1_gain_r = R > max_wb_gain ? max_wb_gain : R;
        cfg->awb1_gain_b = B > max_wb_gain ? max_wb_gain : B;
        cfg->awb1_gain_gr = Gr > max_wb_gain ? max_wb_gain : Gr;
        cfg->awb1_gain_gb = Gb > max_wb_gain ? max_wb_gain : Gb;
    }
#endif

#if RKAIQ_HAVE_AWB_V32
    mLatestWbGainCfg = *cfg;
#endif
}

#if RKAIQ_HAVE_AWB_V32
static void WriteAwbReg(struct isp39_rawawb_meas_cfg* awb_cfg_v32)
{
#if 0
    char fName[100];
    sprintf(fName, "./tmp/awb_reg.txt");
    LOGE_AWB( "%s", fName);

    FILE* fp = fopen(fName, "wb");
    if(fp == NULL) {
        return;
    }
    //fprintf(fp, "\t\tsw_rawawb_en = 0x%0x (%d)\n",                              awb_cfg_v32->en                            ,awb_cfg_v32->en);
    fprintf(fp, "\t\tsw_rawawb_uv_en0 = 0x%0x (%d)\n",                            awb_cfg_v32->uv_en0, awb_cfg_v32->uv_en0);
    fprintf(fp, "\t\tsw_rawawb_xy_en0 = 0x%0x (%d)\n",                            awb_cfg_v32->xy_en0, awb_cfg_v32->xy_en0);
    fprintf(fp, "\t\tsw_rawawb_yuv3d_en0 = 0x%0x (%d)\n",                         awb_cfg_v32->yuv3d_en0, awb_cfg_v32->yuv3d_en0);
    fprintf(fp, "\t\tsw_rawawb_yuv3d_ls_idx0 = 0x%0x (%d)\n",                     awb_cfg_v32->yuv3d_ls_idx0, awb_cfg_v32->yuv3d_ls_idx0);
    fprintf(fp, "\t\tsw_rawawb_yuv3d_ls_idx1 = 0x%0x (%d)\n",                     awb_cfg_v32->yuv3d_ls_idx1, awb_cfg_v32->yuv3d_ls_idx1);
    fprintf(fp, "\t\tsw_rawawb_yuv3d_ls_idx2 = 0x%0x (%d)\n",                     awb_cfg_v32->yuv3d_ls_idx2, awb_cfg_v32->yuv3d_ls_idx2);
    fprintf(fp, "\t\tsw_rawawb_yuv3d_ls_idx3 = 0x%0x (%d)\n",                     awb_cfg_v32->yuv3d_ls_idx3, awb_cfg_v32->yuv3d_ls_idx3);
    fprintf(fp, "\t\tsw_rawawb_in_rshift_to_12bit_en = 0x%0x (%d)\n",             awb_cfg_v32->in_rshift_to_12bit_en, awb_cfg_v32->in_rshift_to_12bit_en);
    fprintf(fp, "\t\tsw_rawawb_in_overexposure_check_en = 0x%0x (%d)\n",          awb_cfg_v32->in_overexposure_check_en, awb_cfg_v32->in_overexposure_check_en);
    fprintf(fp, "\t\tsw_rawawb_wind_size = 0x%0x (%d)\n",                         awb_cfg_v32->wind_size, awb_cfg_v32->wind_size);
    fprintf(fp, "\t\tsw_rawlsc_bypass_en = 0x%0x (%d)\n",                         awb_cfg_v32->rawlsc_bypass_en, awb_cfg_v32->rawlsc_bypass_en);
    fprintf(fp, "\t\tsw_rawawb_light_num = 0x%0x (%d)\n",                         awb_cfg_v32->light_num, awb_cfg_v32->light_num);
    //fprintf(fp, "\t\tsw_rawawb_2ddr_path_en = 0x%0x (%d)\n",                      awb_cfg_v32->2ddr_path_en                  ,awb_cfg_v32->2ddr_path_en);
    fprintf(fp, "\t\tsw_rawawb_uv_en1 = 0x%0x (%d)\n",                            awb_cfg_v32->uv_en1, awb_cfg_v32->uv_en1);
    fprintf(fp, "\t\tsw_rawawb_xy_en1 = 0x%0x (%d)\n",                            awb_cfg_v32->xy_en1, awb_cfg_v32->xy_en1);
    fprintf(fp, "\t\tsw_rawawb_yuv3d_en1 = 0x%0x (%d)\n",                         awb_cfg_v32->yuv3d_en1, awb_cfg_v32->yuv3d_en1);
    //fprintf(fp, "\t\tsw_rawawb_2ddr_path_sel = 0x%0x (%d)\n",                     awb_cfg_v32->2ddr_path_sel                 ,awb_cfg_v32->2ddr_path_sel);
    fprintf(fp, "\t\tsw_rawawbin_low12bit_val = 0x%0x (%d)\n",                    awb_cfg_v32->low12bit_val, awb_cfg_v32->low12bit_val);
    fprintf(fp, "\t\tsw_rawawb_blk_measure_en = 0x%0x (%d)\n",                    awb_cfg_v32->blk_measure_enable, awb_cfg_v32->blk_measure_enable);
    fprintf(fp, "\t\tsw_rawawb_blk_measure_mode = 0x%0x (%d)\n",                  awb_cfg_v32->blk_measure_mode, awb_cfg_v32->blk_measure_mode);
    fprintf(fp, "\t\tsw_rawawb_blk_measure_xytype = 0x%0x (%d)\n",                awb_cfg_v32->blk_measure_xytype, awb_cfg_v32->blk_measure_xytype);
    fprintf(fp, "\t\tsw_rawawb_blk_rtdw_measure_en = 0x%0x (%d)\n",               awb_cfg_v32->blk_rtdw_measure_en, awb_cfg_v32->blk_rtdw_measure_en);
    fprintf(fp, "\t\tsw_rawawb_blk_measure_illu_idx = 0x%0x (%d)\n",              awb_cfg_v32->blk_measure_illu_idx, awb_cfg_v32->blk_measure_illu_idx);
    fprintf(fp, "\t\tsw_rawawb_ds16x8_mode_en = 0x%0x (%d)\n",                    awb_cfg_v32->ds16x8_mode_en, awb_cfg_v32->ds16x8_mode_en);
    fprintf(fp, "\t\tsw_rawawb_blk_with_luma_wei_en = 0x%0x (%d)\n",              awb_cfg_v32->blk_with_luma_wei_en, awb_cfg_v32->blk_with_luma_wei_en);
    fprintf(fp, "\t\tsw_rawawb_in_overexposure_threshold = 0x%0x (%d)\n",         awb_cfg_v32->in_overexposure_threshold, awb_cfg_v32->in_overexposure_threshold);
    fprintf(fp, "\t\tsw_rawawb_h_offs = 0x%0x (%d)\n",                            awb_cfg_v32->h_offs, awb_cfg_v32->h_offs);
    fprintf(fp, "\t\tsw_rawawb_v_offs = 0x%0x (%d)\n",                            awb_cfg_v32->v_offs, awb_cfg_v32->v_offs);
    fprintf(fp, "\t\tsw_rawawb_h_size = 0x%0x (%d)\n",                            awb_cfg_v32->h_size, awb_cfg_v32->h_size);
    fprintf(fp, "\t\tsw_rawawb_v_size = 0x%0x (%d)\n",                            awb_cfg_v32->v_size, awb_cfg_v32->v_size);
    fprintf(fp, "\t\tsw_rawawb_r_max = 0x%0x (%d)\n",                             awb_cfg_v32->r_max, awb_cfg_v32->r_max);
    fprintf(fp, "\t\tsw_rawawb_g_max = 0x%0x (%d)\n",                             awb_cfg_v32->g_max, awb_cfg_v32->g_max);
    fprintf(fp, "\t\tsw_rawawb_b_max = 0x%0x (%d)\n",                             awb_cfg_v32->b_max, awb_cfg_v32->b_max);
    fprintf(fp, "\t\tsw_rawawb_y_max = 0x%0x (%d)\n",                             awb_cfg_v32->y_max, awb_cfg_v32->y_max);
    fprintf(fp, "\t\tsw_rawawb_r_min = 0x%0x (%d)\n",                             awb_cfg_v32->r_min, awb_cfg_v32->r_min);
    fprintf(fp, "\t\tsw_rawawb_g_min = 0x%0x (%d)\n",                             awb_cfg_v32->g_min, awb_cfg_v32->g_min);
    fprintf(fp, "\t\tsw_rawawb_b_min = 0x%0x (%d)\n",                             awb_cfg_v32->b_min, awb_cfg_v32->b_min);
    fprintf(fp, "\t\tsw_rawawb_y_min = 0x%0x (%d)\n",                             awb_cfg_v32->y_min, awb_cfg_v32->y_min);
    fprintf(fp, "\t\tsw_rawawb_wp_luma_wei_en0 = 0x%0x (%d)\n",                   awb_cfg_v32->wp_luma_wei_en0, awb_cfg_v32->wp_luma_wei_en0);
    fprintf(fp, "\t\tsw_rawawb_wp_luma_wei_en1 = 0x%0x (%d)\n",                   awb_cfg_v32->wp_luma_wei_en1, awb_cfg_v32->wp_luma_wei_en1);
    fprintf(fp, "\t\tsw_rawawb_wp_blk_wei_en0 = 0x%0x (%d)\n",                    awb_cfg_v32->wp_blk_wei_en0, awb_cfg_v32->wp_blk_wei_en0);
    fprintf(fp, "\t\tsw_rawawb_wp_blk_wei_en1 = 0x%0x (%d)\n",                    awb_cfg_v32->wp_blk_wei_en1, awb_cfg_v32->wp_blk_wei_en1);
    fprintf(fp, "\t\tsw_rawawb_wp_hist_xytype = 0x%0x (%d)\n",                    awb_cfg_v32->wp_hist_xytype, awb_cfg_v32->wp_hist_xytype);
    fprintf(fp, "\t\tsw_rawawb_wp_luma_weicurve_y0 = 0x%0x (%d)\n",               awb_cfg_v32->wp_luma_weicurve_y0, awb_cfg_v32->wp_luma_weicurve_y0);
    fprintf(fp, "\t\tsw_rawawb_wp_luma_weicurve_y1 = 0x%0x (%d)\n",               awb_cfg_v32->wp_luma_weicurve_y1, awb_cfg_v32->wp_luma_weicurve_y1);
    fprintf(fp, "\t\tsw_rawawb_wp_luma_weicurve_y2 = 0x%0x (%d)\n",               awb_cfg_v32->wp_luma_weicurve_y2, awb_cfg_v32->wp_luma_weicurve_y2);
    fprintf(fp, "\t\tsw_rawawb_wp_luma_weicurve_y3 = 0x%0x (%d)\n",               awb_cfg_v32->wp_luma_weicurve_y3, awb_cfg_v32->wp_luma_weicurve_y3);
    fprintf(fp, "\t\tsw_rawawb_wp_luma_weicurve_y4 = 0x%0x (%d)\n",               awb_cfg_v32->wp_luma_weicurve_y4, awb_cfg_v32->wp_luma_weicurve_y4);
    fprintf(fp, "\t\tsw_rawawb_wp_luma_weicurve_y5 = 0x%0x (%d)\n",               awb_cfg_v32->wp_luma_weicurve_y5, awb_cfg_v32->wp_luma_weicurve_y5);
    fprintf(fp, "\t\tsw_rawawb_wp_luma_weicurve_y6 = 0x%0x (%d)\n",               awb_cfg_v32->wp_luma_weicurve_y6, awb_cfg_v32->wp_luma_weicurve_y6);
    fprintf(fp, "\t\tsw_rawawb_wp_luma_weicurve_y7 = 0x%0x (%d)\n",               awb_cfg_v32->wp_luma_weicurve_y7, awb_cfg_v32->wp_luma_weicurve_y7);
    fprintf(fp, "\t\tsw_rawawb_wp_luma_weicurve_y8 = 0x%0x (%d)\n",               awb_cfg_v32->wp_luma_weicurve_y8, awb_cfg_v32->wp_luma_weicurve_y8);
    fprintf(fp, "\t\tsw_rawawb_wp_luma_weicurve_w0 = 0x%0x (%d)\n",               awb_cfg_v32->wp_luma_weicurve_w0, awb_cfg_v32->wp_luma_weicurve_w0);
    fprintf(fp, "\t\tsw_rawawb_wp_luma_weicurve_w1 = 0x%0x (%d)\n",               awb_cfg_v32->wp_luma_weicurve_w1, awb_cfg_v32->wp_luma_weicurve_w1);
    fprintf(fp, "\t\tsw_rawawb_wp_luma_weicurve_w2 = 0x%0x (%d)\n",               awb_cfg_v32->wp_luma_weicurve_w2, awb_cfg_v32->wp_luma_weicurve_w2);
    fprintf(fp, "\t\tsw_rawawb_wp_luma_weicurve_w3 = 0x%0x (%d)\n",               awb_cfg_v32->wp_luma_weicurve_w3, awb_cfg_v32->wp_luma_weicurve_w3);
    fprintf(fp, "\t\tsw_rawawb_wp_luma_weicurve_w4 = 0x%0x (%d)\n",               awb_cfg_v32->wp_luma_weicurve_w4, awb_cfg_v32->wp_luma_weicurve_w4);
    fprintf(fp, "\t\tsw_rawawb_wp_luma_weicurve_w5 = 0x%0x (%d)\n",               awb_cfg_v32->wp_luma_weicurve_w5, awb_cfg_v32->wp_luma_weicurve_w5);
    fprintf(fp, "\t\tsw_rawawb_wp_luma_weicurve_w6 = 0x%0x (%d)\n",               awb_cfg_v32->wp_luma_weicurve_w6, awb_cfg_v32->wp_luma_weicurve_w6);
    fprintf(fp, "\t\tsw_rawawb_wp_luma_weicurve_w7 = 0x%0x (%d)\n",               awb_cfg_v32->wp_luma_weicurve_w7, awb_cfg_v32->wp_luma_weicurve_w7);
    fprintf(fp, "\t\tsw_rawawb_wp_luma_weicurve_w8 = 0x%0x (%d)\n",               awb_cfg_v32->wp_luma_weicurve_w8, awb_cfg_v32->wp_luma_weicurve_w8);
    fprintf(fp, "\t\tsw_rawawb_pre_wbgain_inv_r = 0x%0x (%d)\n",                  awb_cfg_v32->pre_wbgain_inv_r, awb_cfg_v32->pre_wbgain_inv_r);
    fprintf(fp, "\t\tsw_rawawb_pre_wbgain_inv_g = 0x%0x (%d)\n",                  awb_cfg_v32->pre_wbgain_inv_g, awb_cfg_v32->pre_wbgain_inv_g);
    fprintf(fp, "\t\tsw_rawawb_pre_wbgain_inv_b = 0x%0x (%d)\n",                  awb_cfg_v32->pre_wbgain_inv_b, awb_cfg_v32->pre_wbgain_inv_b);
    fprintf(fp, "\t\tsw_rawawb_vertex0_u_0 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex0_u_0, awb_cfg_v32->vertex0_u_0);
    fprintf(fp, "\t\tsw_rawawb_vertex0_v_0 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex0_v_0, awb_cfg_v32->vertex0_v_0);
    fprintf(fp, "\t\tsw_rawawb_vertex1_u_0 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex1_u_0, awb_cfg_v32->vertex1_u_0);
    fprintf(fp, "\t\tsw_rawawb_vertex1_v_0 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex1_v_0, awb_cfg_v32->vertex1_v_0);
    fprintf(fp, "\t\tsw_rawawb_vertex2_u_0 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex2_u_0, awb_cfg_v32->vertex2_u_0);
    fprintf(fp, "\t\tsw_rawawb_vertex2_v_0 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex2_v_0, awb_cfg_v32->vertex2_v_0);
    fprintf(fp, "\t\tsw_rawawb_vertex3_u_0 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex3_u_0, awb_cfg_v32->vertex3_u_0);
    fprintf(fp, "\t\tsw_rawawb_vertex3_v_0 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex3_v_0, awb_cfg_v32->vertex3_v_0);
    fprintf(fp, "\t\tsw_rawawb_islope01_0 = 0x%0x (%d)\n",                        awb_cfg_v32->islope01_0, awb_cfg_v32->islope01_0);
    fprintf(fp, "\t\tsw_rawawb_islope12_0 = 0x%0x (%d)\n",                        awb_cfg_v32->islope12_0, awb_cfg_v32->islope12_0);
    fprintf(fp, "\t\tsw_rawawb_islope23_0 = 0x%0x (%d)\n",                        awb_cfg_v32->islope23_0, awb_cfg_v32->islope23_0);
    fprintf(fp, "\t\tsw_rawawb_islope30_0 = 0x%0x (%d)\n",                        awb_cfg_v32->islope30_0, awb_cfg_v32->islope30_0);
    fprintf(fp, "\t\tsw_rawawb_vertex0_u_1 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex0_u_1, awb_cfg_v32->vertex0_u_1);
    fprintf(fp, "\t\tsw_rawawb_vertex0_v_1 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex0_v_1, awb_cfg_v32->vertex0_v_1);
    fprintf(fp, "\t\tsw_rawawb_vertex1_u_1 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex1_u_1, awb_cfg_v32->vertex1_u_1);
    fprintf(fp, "\t\tsw_rawawb_vertex1_v_1 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex1_v_1, awb_cfg_v32->vertex1_v_1);
    fprintf(fp, "\t\tsw_rawawb_vertex2_u_1 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex2_u_1, awb_cfg_v32->vertex2_u_1);
    fprintf(fp, "\t\tsw_rawawb_vertex2_v_1 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex2_v_1, awb_cfg_v32->vertex2_v_1);
    fprintf(fp, "\t\tsw_rawawb_vertex3_u_1 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex3_u_1, awb_cfg_v32->vertex3_u_1);
    fprintf(fp, "\t\tsw_rawawb_vertex3_v_1 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex3_v_1, awb_cfg_v32->vertex3_v_1);
    fprintf(fp, "\t\tsw_rawawb_islope01_1 = 0x%0x (%d)\n",                        awb_cfg_v32->islope01_1, awb_cfg_v32->islope01_1);
    fprintf(fp, "\t\tsw_rawawb_islope12_1 = 0x%0x (%d)\n",                        awb_cfg_v32->islope12_1, awb_cfg_v32->islope12_1);
    fprintf(fp, "\t\tsw_rawawb_islope23_1 = 0x%0x (%d)\n",                        awb_cfg_v32->islope23_1, awb_cfg_v32->islope23_1);
    fprintf(fp, "\t\tsw_rawawb_islope30_1 = 0x%0x (%d)\n",                        awb_cfg_v32->islope30_1, awb_cfg_v32->islope30_1);
    fprintf(fp, "\t\tsw_rawawb_vertex0_u_2 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex0_u_2, awb_cfg_v32->vertex0_u_2);
    fprintf(fp, "\t\tsw_rawawb_vertex0_v_2 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex0_v_2, awb_cfg_v32->vertex0_v_2);
    fprintf(fp, "\t\tsw_rawawb_vertex1_u_2 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex1_u_2, awb_cfg_v32->vertex1_u_2);
    fprintf(fp, "\t\tsw_rawawb_vertex1_v_2 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex1_v_2, awb_cfg_v32->vertex1_v_2);
    fprintf(fp, "\t\tsw_rawawb_vertex2_u_2 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex2_u_2, awb_cfg_v32->vertex2_u_2);
    fprintf(fp, "\t\tsw_rawawb_vertex2_v_2 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex2_v_2, awb_cfg_v32->vertex2_v_2);
    fprintf(fp, "\t\tsw_rawawb_vertex3_u_2 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex3_u_2, awb_cfg_v32->vertex3_u_2);
    fprintf(fp, "\t\tsw_rawawb_vertex3_v_2 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex3_v_2, awb_cfg_v32->vertex3_v_2);
    fprintf(fp, "\t\tsw_rawawb_islope01_2 = 0x%0x (%d)\n",                        awb_cfg_v32->islope01_2, awb_cfg_v32->islope01_2);
    fprintf(fp, "\t\tsw_rawawb_islope12_2 = 0x%0x (%d)\n",                        awb_cfg_v32->islope12_2, awb_cfg_v32->islope12_2);
    fprintf(fp, "\t\tsw_rawawb_islope23_2 = 0x%0x (%d)\n",                        awb_cfg_v32->islope23_2, awb_cfg_v32->islope23_2);
    fprintf(fp, "\t\tsw_rawawb_islope30_2 = 0x%0x (%d)\n",                        awb_cfg_v32->islope30_2, awb_cfg_v32->islope30_2);
    fprintf(fp, "\t\tsw_rawawb_vertex0_u_3 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex0_u_3, awb_cfg_v32->vertex0_u_3);
    fprintf(fp, "\t\tsw_rawawb_vertex0_v_3 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex0_v_3, awb_cfg_v32->vertex0_v_3);
    fprintf(fp, "\t\tsw_rawawb_vertex1_u_3 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex1_u_3, awb_cfg_v32->vertex1_u_3);
    fprintf(fp, "\t\tsw_rawawb_vertex1_v_3 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex1_v_3, awb_cfg_v32->vertex1_v_3);
    fprintf(fp, "\t\tsw_rawawb_vertex2_u_3 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex2_u_3, awb_cfg_v32->vertex2_u_3);
    fprintf(fp, "\t\tsw_rawawb_vertex2_v_3 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex2_v_3, awb_cfg_v32->vertex2_v_3);
    fprintf(fp, "\t\tsw_rawawb_vertex3_u_3 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex3_u_3, awb_cfg_v32->vertex3_u_3);
    fprintf(fp, "\t\tsw_rawawb_vertex3_v_3 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex3_v_3, awb_cfg_v32->vertex3_v_3);
    fprintf(fp, "\t\tsw_rawawb_islope01_3 = 0x%0x (%d)\n",                        awb_cfg_v32->islope01_3, awb_cfg_v32->islope01_3);
    fprintf(fp, "\t\tsw_rawawb_islope12_3 = 0x%0x (%d)\n",                        awb_cfg_v32->islope12_3, awb_cfg_v32->islope12_3);
    fprintf(fp, "\t\tsw_rawawb_islope23_3 = 0x%0x (%d)\n",                        awb_cfg_v32->islope23_3, awb_cfg_v32->islope23_3);
    fprintf(fp, "\t\tsw_rawawb_islope30_3 = 0x%0x (%d)\n",                        awb_cfg_v32->islope30_3, awb_cfg_v32->islope30_3);
    fprintf(fp, "\t\tsw_rawawb_rgb2ryuvmat0_y = 0x%0x (%d)\n",                    awb_cfg_v32->rgb2ryuvmat0_y, awb_cfg_v32->rgb2ryuvmat0_y);
    fprintf(fp, "\t\tsw_rawawb_rgb2ryuvmat1_y = 0x%0x (%d)\n",                    awb_cfg_v32->rgb2ryuvmat1_y, awb_cfg_v32->rgb2ryuvmat1_y);
    fprintf(fp, "\t\tsw_rawawb_rgb2ryuvmat2_y = 0x%0x (%d)\n",                    awb_cfg_v32->rgb2ryuvmat2_y, awb_cfg_v32->rgb2ryuvmat2_y);
    fprintf(fp, "\t\tsw_rawawb_rgb2ryuvofs_y = 0x%0x (%d)\n",                     awb_cfg_v32->rgb2ryuvofs_y, awb_cfg_v32->rgb2ryuvofs_y);
    fprintf(fp, "\t\tsw_rawawb_rgb2ryuvmat0_u = 0x%0x (%d)\n",                    awb_cfg_v32->rgb2ryuvmat0_u, awb_cfg_v32->rgb2ryuvmat0_u);
    fprintf(fp, "\t\tsw_rawawb_rgb2ryuvmat1_u = 0x%0x (%d)\n",                    awb_cfg_v32->rgb2ryuvmat1_u, awb_cfg_v32->rgb2ryuvmat1_u);
    fprintf(fp, "\t\tsw_rawawb_rgb2ryuvmat2_u = 0x%0x (%d)\n",                    awb_cfg_v32->rgb2ryuvmat2_u, awb_cfg_v32->rgb2ryuvmat2_u);
    fprintf(fp, "\t\tsw_rawawb_rgb2ryuvofs_u = 0x%0x (%d)\n",                     awb_cfg_v32->rgb2ryuvofs_u, awb_cfg_v32->rgb2ryuvofs_u);
    fprintf(fp, "\t\tsw_rawawb_rgb2ryuvmat0_v = 0x%0x (%d)\n",                    awb_cfg_v32->rgb2ryuvmat0_v, awb_cfg_v32->rgb2ryuvmat0_v);
    fprintf(fp, "\t\tsw_rawawb_rgb2ryuvmat1_v = 0x%0x (%d)\n",                    awb_cfg_v32->rgb2ryuvmat1_v, awb_cfg_v32->rgb2ryuvmat1_v);
    fprintf(fp, "\t\tsw_rawawb_rgb2ryuvmat2_v = 0x%0x (%d)\n",                    awb_cfg_v32->rgb2ryuvmat2_v, awb_cfg_v32->rgb2ryuvmat2_v);
    fprintf(fp, "\t\tsw_rawawb_rgb2ryuvofs_v = 0x%0x (%d)\n",                     awb_cfg_v32->rgb2ryuvofs_v, awb_cfg_v32->rgb2ryuvofs_v);
    fprintf(fp, "\t\tsw_rawawb_coor_x1_ls0_y = 0x%0x (%d)\n",                     awb_cfg_v32->coor_x1_ls0_y, awb_cfg_v32->coor_x1_ls0_y);
    fprintf(fp, "\t\tsw_rawawb_vec_x21_ls0_y = 0x%0x (%d)\n",                     awb_cfg_v32->vec_x21_ls0_y, awb_cfg_v32->vec_x21_ls0_y);
    fprintf(fp, "\t\tsw_rawawb_coor_x1_ls0_u = 0x%0x (%d)\n",                     awb_cfg_v32->coor_x1_ls0_u, awb_cfg_v32->coor_x1_ls0_u);
    fprintf(fp, "\t\tsw_rawawb_vec_x21_ls0_u = 0x%0x (%d)\n",                     awb_cfg_v32->vec_x21_ls0_u, awb_cfg_v32->vec_x21_ls0_u);
    fprintf(fp, "\t\tsw_rawawb_coor_x1_ls0_v = 0x%0x (%d)\n",                     awb_cfg_v32->coor_x1_ls0_v, awb_cfg_v32->coor_x1_ls0_v);
    fprintf(fp, "\t\tsw_rawawb_vec_x21_ls0_v = 0x%0x (%d)\n",                     awb_cfg_v32->vec_x21_ls0_v, awb_cfg_v32->vec_x21_ls0_v);
    fprintf(fp, "\t\tsw_rawawb_dis_x1x2_ls0 = 0x%0x (%d)\n",                      awb_cfg_v32->dis_x1x2_ls0, awb_cfg_v32->dis_x1x2_ls0);
    fprintf(fp, "\t\tsw_rawawb_rotu0_ls0 = 0x%0x (%d)\n",                         awb_cfg_v32->rotu0_ls0, awb_cfg_v32->rotu0_ls0);
    fprintf(fp, "\t\tsw_rawawb_rotu1_ls0 = 0x%0x (%d)\n",                         awb_cfg_v32->rotu1_ls0, awb_cfg_v32->rotu1_ls0);
    fprintf(fp, "\t\tsw_rawawb_rotu2_ls0 = 0x%0x (%d)\n",                         awb_cfg_v32->rotu2_ls0, awb_cfg_v32->rotu2_ls0);
    fprintf(fp, "\t\tsw_rawawb_rotu3_ls0 = 0x%0x (%d)\n",                         awb_cfg_v32->rotu3_ls0, awb_cfg_v32->rotu3_ls0);
    fprintf(fp, "\t\tsw_rawawb_rotu4_ls0 = 0x%0x (%d)\n",                         awb_cfg_v32->rotu4_ls0, awb_cfg_v32->rotu4_ls0);
    fprintf(fp, "\t\tsw_rawawb_rotu5_ls0 = 0x%0x (%d)\n",                         awb_cfg_v32->rotu5_ls0, awb_cfg_v32->rotu5_ls0);
    fprintf(fp, "\t\tsw_rawawb_th0_ls0 = 0x%0x (%d)\n",                           awb_cfg_v32->th0_ls0, awb_cfg_v32->th0_ls0);
    fprintf(fp, "\t\tsw_rawawb_th1_ls0 = 0x%0x (%d)\n",                           awb_cfg_v32->th1_ls0, awb_cfg_v32->th1_ls0);
    fprintf(fp, "\t\tsw_rawawb_th2_ls0 = 0x%0x (%d)\n",                           awb_cfg_v32->th2_ls0, awb_cfg_v32->th2_ls0);
    fprintf(fp, "\t\tsw_rawawb_th3_ls0 = 0x%0x (%d)\n",                           awb_cfg_v32->th3_ls0, awb_cfg_v32->th3_ls0);
    fprintf(fp, "\t\tsw_rawawb_th4_ls0 = 0x%0x (%d)\n",                           awb_cfg_v32->th4_ls0, awb_cfg_v32->th4_ls0);
    fprintf(fp, "\t\tsw_rawawb_th5_ls0 = 0x%0x (%d)\n",                           awb_cfg_v32->th5_ls0, awb_cfg_v32->th5_ls0);
    fprintf(fp, "\t\tsw_rawawb_coor_x1_ls1_y = 0x%0x (%d)\n",                     awb_cfg_v32->coor_x1_ls1_y, awb_cfg_v32->coor_x1_ls1_y);
    fprintf(fp, "\t\tsw_rawawb_vec_x21_ls1_y = 0x%0x (%d)\n",                     awb_cfg_v32->vec_x21_ls1_y, awb_cfg_v32->vec_x21_ls1_y);
    fprintf(fp, "\t\tsw_rawawb_coor_x1_ls1_u = 0x%0x (%d)\n",                     awb_cfg_v32->coor_x1_ls1_u, awb_cfg_v32->coor_x1_ls1_u);
    fprintf(fp, "\t\tsw_rawawb_vec_x21_ls1_u = 0x%0x (%d)\n",                     awb_cfg_v32->vec_x21_ls1_u, awb_cfg_v32->vec_x21_ls1_u);
    fprintf(fp, "\t\tsw_rawawb_coor_x1_ls1_v = 0x%0x (%d)\n",                     awb_cfg_v32->coor_x1_ls1_v, awb_cfg_v32->coor_x1_ls1_v);
    fprintf(fp, "\t\tsw_rawawb_vec_x21_ls1_v = 0x%0x (%d)\n",                     awb_cfg_v32->vec_x21_ls1_v, awb_cfg_v32->vec_x21_ls1_v);
    fprintf(fp, "\t\tsw_rawawb_dis_x1x2_ls1 = 0x%0x (%d)\n",                      awb_cfg_v32->dis_x1x2_ls1, awb_cfg_v32->dis_x1x2_ls1);
    fprintf(fp, "\t\tsw_rawawb_rotu0_ls1 = 0x%0x (%d)\n",                         awb_cfg_v32->rotu0_ls1, awb_cfg_v32->rotu0_ls1);
    fprintf(fp, "\t\tsw_rawawb_rotu1_ls1 = 0x%0x (%d)\n",                         awb_cfg_v32->rotu1_ls1, awb_cfg_v32->rotu1_ls1);
    fprintf(fp, "\t\tsw_rawawb_rotu2_ls1 = 0x%0x (%d)\n",                         awb_cfg_v32->rotu2_ls1, awb_cfg_v32->rotu2_ls1);
    fprintf(fp, "\t\tsw_rawawb_rotu3_ls1 = 0x%0x (%d)\n",                         awb_cfg_v32->rotu3_ls1, awb_cfg_v32->rotu3_ls1);
    fprintf(fp, "\t\tsw_rawawb_rotu4_ls1 = 0x%0x (%d)\n",                         awb_cfg_v32->rotu4_ls1, awb_cfg_v32->rotu4_ls1);
    fprintf(fp, "\t\tsw_rawawb_rotu5_ls1 = 0x%0x (%d)\n",                         awb_cfg_v32->rotu5_ls1, awb_cfg_v32->rotu5_ls1);
    fprintf(fp, "\t\tsw_rawawb_th0_ls1 = 0x%0x (%d)\n",                           awb_cfg_v32->th0_ls1, awb_cfg_v32->th0_ls1);
    fprintf(fp, "\t\tsw_rawawb_th1_ls1 = 0x%0x (%d)\n",                           awb_cfg_v32->th1_ls1, awb_cfg_v32->th1_ls1);
    fprintf(fp, "\t\tsw_rawawb_th2_ls1 = 0x%0x (%d)\n",                           awb_cfg_v32->th2_ls1, awb_cfg_v32->th2_ls1);
    fprintf(fp, "\t\tsw_rawawb_th3_ls1 = 0x%0x (%d)\n",                           awb_cfg_v32->th3_ls1, awb_cfg_v32->th3_ls1);
    fprintf(fp, "\t\tsw_rawawb_th4_ls1 = 0x%0x (%d)\n",                           awb_cfg_v32->th4_ls1, awb_cfg_v32->th4_ls1);
    fprintf(fp, "\t\tsw_rawawb_th5_ls1 = 0x%0x (%d)\n",                           awb_cfg_v32->th5_ls1, awb_cfg_v32->th5_ls1);
    fprintf(fp, "\t\tsw_rawawb_coor_x1_ls2_y = 0x%0x (%d)\n",                     awb_cfg_v32->coor_x1_ls2_y, awb_cfg_v32->coor_x1_ls2_y);
    fprintf(fp, "\t\tsw_rawawb_vec_x21_ls2_y = 0x%0x (%d)\n",                     awb_cfg_v32->vec_x21_ls2_y, awb_cfg_v32->vec_x21_ls2_y);
    fprintf(fp, "\t\tsw_rawawb_coor_x1_ls2_u = 0x%0x (%d)\n",                     awb_cfg_v32->coor_x1_ls2_u, awb_cfg_v32->coor_x1_ls2_u);
    fprintf(fp, "\t\tsw_rawawb_vec_x21_ls2_u = 0x%0x (%d)\n",                     awb_cfg_v32->vec_x21_ls2_u, awb_cfg_v32->vec_x21_ls2_u);
    fprintf(fp, "\t\tsw_rawawb_coor_x1_ls2_v = 0x%0x (%d)\n",                     awb_cfg_v32->coor_x1_ls2_v, awb_cfg_v32->coor_x1_ls2_v);
    fprintf(fp, "\t\tsw_rawawb_vec_x21_ls2_v = 0x%0x (%d)\n",                     awb_cfg_v32->vec_x21_ls2_v, awb_cfg_v32->vec_x21_ls2_v);
    fprintf(fp, "\t\tsw_rawawb_dis_x1x2_ls2 = 0x%0x (%d)\n",                      awb_cfg_v32->dis_x1x2_ls2, awb_cfg_v32->dis_x1x2_ls2);
    fprintf(fp, "\t\tsw_rawawb_rotu0_ls2 = 0x%0x (%d)\n",                         awb_cfg_v32->rotu0_ls2, awb_cfg_v32->rotu0_ls2);
    fprintf(fp, "\t\tsw_rawawb_rotu1_ls2 = 0x%0x (%d)\n",                         awb_cfg_v32->rotu1_ls2, awb_cfg_v32->rotu1_ls2);
    fprintf(fp, "\t\tsw_rawawb_rotu2_ls2 = 0x%0x (%d)\n",                         awb_cfg_v32->rotu2_ls2, awb_cfg_v32->rotu2_ls2);
    fprintf(fp, "\t\tsw_rawawb_rotu3_ls2 = 0x%0x (%d)\n",                         awb_cfg_v32->rotu3_ls2, awb_cfg_v32->rotu3_ls2);
    fprintf(fp, "\t\tsw_rawawb_rotu4_ls2 = 0x%0x (%d)\n",                         awb_cfg_v32->rotu4_ls2, awb_cfg_v32->rotu4_ls2);
    fprintf(fp, "\t\tsw_rawawb_rotu5_ls2 = 0x%0x (%d)\n",                         awb_cfg_v32->rotu5_ls2, awb_cfg_v32->rotu5_ls2);
    fprintf(fp, "\t\tsw_rawawb_th0_ls2 = 0x%0x (%d)\n",                           awb_cfg_v32->th0_ls2, awb_cfg_v32->th0_ls2);
    fprintf(fp, "\t\tsw_rawawb_th1_ls2 = 0x%0x (%d)\n",                           awb_cfg_v32->th1_ls2, awb_cfg_v32->th1_ls2);
    fprintf(fp, "\t\tsw_rawawb_th2_ls2 = 0x%0x (%d)\n",                           awb_cfg_v32->th2_ls2, awb_cfg_v32->th2_ls2);
    fprintf(fp, "\t\tsw_rawawb_th3_ls2 = 0x%0x (%d)\n",                           awb_cfg_v32->th3_ls2, awb_cfg_v32->th3_ls2);
    fprintf(fp, "\t\tsw_rawawb_th4_ls2 = 0x%0x (%d)\n",                           awb_cfg_v32->th4_ls2, awb_cfg_v32->th4_ls2);
    fprintf(fp, "\t\tsw_rawawb_th5_ls2 = 0x%0x (%d)\n",                           awb_cfg_v32->th5_ls2, awb_cfg_v32->th5_ls2);
    fprintf(fp, "\t\tsw_rawawb_coor_x1_ls3_y = 0x%0x (%d)\n",                     awb_cfg_v32->coor_x1_ls3_y, awb_cfg_v32->coor_x1_ls3_y);
    fprintf(fp, "\t\tsw_rawawb_vec_x21_ls3_y = 0x%0x (%d)\n",                     awb_cfg_v32->vec_x21_ls3_y, awb_cfg_v32->vec_x21_ls3_y);
    fprintf(fp, "\t\tsw_rawawb_coor_x1_ls3_u = 0x%0x (%d)\n",                     awb_cfg_v32->coor_x1_ls3_u, awb_cfg_v32->coor_x1_ls3_u);
    fprintf(fp, "\t\tsw_rawawb_vec_x21_ls3_u = 0x%0x (%d)\n",                     awb_cfg_v32->vec_x21_ls3_u, awb_cfg_v32->vec_x21_ls3_u);
    fprintf(fp, "\t\tsw_rawawb_coor_x1_ls3_v = 0x%0x (%d)\n",                     awb_cfg_v32->coor_x1_ls3_v, awb_cfg_v32->coor_x1_ls3_v);
    fprintf(fp, "\t\tsw_rawawb_vec_x21_ls3_v = 0x%0x (%d)\n",                     awb_cfg_v32->vec_x21_ls3_v, awb_cfg_v32->vec_x21_ls3_v);
    fprintf(fp, "\t\tsw_rawawb_dis_x1x2_ls3 = 0x%0x (%d)\n",                      awb_cfg_v32->dis_x1x2_ls3, awb_cfg_v32->dis_x1x2_ls3);
    fprintf(fp, "\t\tsw_rawawb_rotu0_ls3 = 0x%0x (%d)\n",                         awb_cfg_v32->rotu0_ls3, awb_cfg_v32->rotu0_ls3);
    fprintf(fp, "\t\tsw_rawawb_rotu1_ls3 = 0x%0x (%d)\n",                         awb_cfg_v32->rotu1_ls3, awb_cfg_v32->rotu1_ls3);
    fprintf(fp, "\t\tsw_rawawb_rotu2_ls3 = 0x%0x (%d)\n",                         awb_cfg_v32->rotu2_ls3, awb_cfg_v32->rotu2_ls3);
    fprintf(fp, "\t\tsw_rawawb_rotu3_ls3 = 0x%0x (%d)\n",                         awb_cfg_v32->rotu3_ls3, awb_cfg_v32->rotu3_ls3);
    fprintf(fp, "\t\tsw_rawawb_rotu4_ls3 = 0x%0x (%d)\n",                         awb_cfg_v32->rotu4_ls3, awb_cfg_v32->rotu4_ls3);
    fprintf(fp, "\t\tsw_rawawb_rotu5_ls3 = 0x%0x (%d)\n",                         awb_cfg_v32->rotu5_ls3, awb_cfg_v32->rotu5_ls3);
    fprintf(fp, "\t\tsw_rawawb_th0_ls3 = 0x%0x (%d)\n",                           awb_cfg_v32->th0_ls3, awb_cfg_v32->th0_ls3);
    fprintf(fp, "\t\tsw_rawawb_th1_ls3 = 0x%0x (%d)\n",                           awb_cfg_v32->th1_ls3, awb_cfg_v32->th1_ls3);
    fprintf(fp, "\t\tsw_rawawb_th2_ls3 = 0x%0x (%d)\n",                           awb_cfg_v32->th2_ls3, awb_cfg_v32->th2_ls3);
    fprintf(fp, "\t\tsw_rawawb_th3_ls3 = 0x%0x (%d)\n",                           awb_cfg_v32->th3_ls3, awb_cfg_v32->th3_ls3);
    fprintf(fp, "\t\tsw_rawawb_th4_ls3 = 0x%0x (%d)\n",                           awb_cfg_v32->th4_ls3, awb_cfg_v32->th4_ls3);
    fprintf(fp, "\t\tsw_rawawb_th5_ls3 = 0x%0x (%d)\n",                           awb_cfg_v32->th5_ls3, awb_cfg_v32->th5_ls3);
    fprintf(fp, "\t\tsw_rawawb_wt0 = 0x%0x (%d)\n",                               awb_cfg_v32->wt0, awb_cfg_v32->wt0);
    fprintf(fp, "\t\tsw_rawawb_wt1 = 0x%0x (%d)\n",                               awb_cfg_v32->wt1, awb_cfg_v32->wt1);
    fprintf(fp, "\t\tsw_rawawb_wt2 = 0x%0x (%d)\n",                               awb_cfg_v32->wt2, awb_cfg_v32->wt2);
    fprintf(fp, "\t\tsw_rawawb_mat0_x = 0x%0x (%d)\n",                            awb_cfg_v32->mat0_x, awb_cfg_v32->mat0_x);
    fprintf(fp, "\t\tsw_rawawb_mat0_y = 0x%0x (%d)\n",                            awb_cfg_v32->mat0_y, awb_cfg_v32->mat0_y);
    fprintf(fp, "\t\tsw_rawawb_mat1_x = 0x%0x (%d)\n",                            awb_cfg_v32->mat1_x, awb_cfg_v32->mat1_x);
    fprintf(fp, "\t\tsw_rawawb_mat1_y = 0x%0x (%d)\n",                            awb_cfg_v32->mat1_y, awb_cfg_v32->mat1_y);
    fprintf(fp, "\t\tsw_rawawb_mat2_x = 0x%0x (%d)\n",                            awb_cfg_v32->mat2_x, awb_cfg_v32->mat2_x);
    fprintf(fp, "\t\tsw_rawawb_mat2_y = 0x%0x (%d)\n",                            awb_cfg_v32->mat2_y, awb_cfg_v32->mat2_y);
    fprintf(fp, "\t\tsw_rawawb_nor_x0_0 = 0x%0x (%d)\n",                          awb_cfg_v32->nor_x0_0, awb_cfg_v32->nor_x0_0);
    fprintf(fp, "\t\tsw_rawawb_nor_x1_0 = 0x%0x (%d)\n",                          awb_cfg_v32->nor_x1_0, awb_cfg_v32->nor_x1_0);
    fprintf(fp, "\t\tsw_rawawb_nor_y0_0 = 0x%0x (%d)\n",                          awb_cfg_v32->nor_y0_0, awb_cfg_v32->nor_y0_0);
    fprintf(fp, "\t\tsw_rawawb_nor_y1_0 = 0x%0x (%d)\n",                          awb_cfg_v32->nor_y1_0, awb_cfg_v32->nor_y1_0);
    fprintf(fp, "\t\tsw_rawawb_big_x0_0 = 0x%0x (%d)\n",                          awb_cfg_v32->big_x0_0, awb_cfg_v32->big_x0_0);
    fprintf(fp, "\t\tsw_rawawb_big_x1_0 = 0x%0x (%d)\n",                          awb_cfg_v32->big_x1_0, awb_cfg_v32->big_x1_0);
    fprintf(fp, "\t\tsw_rawawb_big_y0_0 = 0x%0x (%d)\n",                          awb_cfg_v32->big_y0_0, awb_cfg_v32->big_y0_0);
    fprintf(fp, "\t\tsw_rawawb_big_y1_0 = 0x%0x (%d)\n",                          awb_cfg_v32->big_y1_0, awb_cfg_v32->big_y1_0);
    fprintf(fp, "\t\tsw_rawawb_nor_x0_1 = 0x%0x (%d)\n",                          awb_cfg_v32->nor_x0_1, awb_cfg_v32->nor_x0_1);
    fprintf(fp, "\t\tsw_rawawb_nor_x1_1 = 0x%0x (%d)\n",                          awb_cfg_v32->nor_x1_1, awb_cfg_v32->nor_x1_1);
    fprintf(fp, "\t\tsw_rawawb_nor_y0_1 = 0x%0x (%d)\n",                          awb_cfg_v32->nor_y0_1, awb_cfg_v32->nor_y0_1);
    fprintf(fp, "\t\tsw_rawawb_nor_y1_1 = 0x%0x (%d)\n",                          awb_cfg_v32->nor_y1_1, awb_cfg_v32->nor_y1_1);
    fprintf(fp, "\t\tsw_rawawb_big_x0_1 = 0x%0x (%d)\n",                          awb_cfg_v32->big_x0_1, awb_cfg_v32->big_x0_1);
    fprintf(fp, "\t\tsw_rawawb_big_x1_1 = 0x%0x (%d)\n",                          awb_cfg_v32->big_x1_1, awb_cfg_v32->big_x1_1);
    fprintf(fp, "\t\tsw_rawawb_big_y0_1 = 0x%0x (%d)\n",                          awb_cfg_v32->big_y0_1, awb_cfg_v32->big_y0_1);
    fprintf(fp, "\t\tsw_rawawb_big_y1_1 = 0x%0x (%d)\n",                          awb_cfg_v32->big_y1_1, awb_cfg_v32->big_y1_1);
    fprintf(fp, "\t\tsw_rawawb_nor_x0_2 = 0x%0x (%d)\n",                          awb_cfg_v32->nor_x0_2, awb_cfg_v32->nor_x0_2);
    fprintf(fp, "\t\tsw_rawawb_nor_x1_2 = 0x%0x (%d)\n",                          awb_cfg_v32->nor_x1_2, awb_cfg_v32->nor_x1_2);
    fprintf(fp, "\t\tsw_rawawb_nor_y0_2 = 0x%0x (%d)\n",                          awb_cfg_v32->nor_y0_2, awb_cfg_v32->nor_y0_2);
    fprintf(fp, "\t\tsw_rawawb_nor_y1_2 = 0x%0x (%d)\n",                          awb_cfg_v32->nor_y1_2, awb_cfg_v32->nor_y1_2);
    fprintf(fp, "\t\tsw_rawawb_big_x0_2 = 0x%0x (%d)\n",                          awb_cfg_v32->big_x0_2, awb_cfg_v32->big_x0_2);
    fprintf(fp, "\t\tsw_rawawb_big_x1_2 = 0x%0x (%d)\n",                          awb_cfg_v32->big_x1_2, awb_cfg_v32->big_x1_2);
    fprintf(fp, "\t\tsw_rawawb_big_y0_2 = 0x%0x (%d)\n",                          awb_cfg_v32->big_y0_2, awb_cfg_v32->big_y0_2);
    fprintf(fp, "\t\tsw_rawawb_big_y1_2 = 0x%0x (%d)\n",                          awb_cfg_v32->big_y1_2, awb_cfg_v32->big_y1_2);
    fprintf(fp, "\t\tsw_rawawb_nor_x0_3 = 0x%0x (%d)\n",                          awb_cfg_v32->nor_x0_3, awb_cfg_v32->nor_x0_3);
    fprintf(fp, "\t\tsw_rawawb_nor_x1_3 = 0x%0x (%d)\n",                          awb_cfg_v32->nor_x1_3, awb_cfg_v32->nor_x1_3);
    fprintf(fp, "\t\tsw_rawawb_nor_y0_3 = 0x%0x (%d)\n",                          awb_cfg_v32->nor_y0_3, awb_cfg_v32->nor_y0_3);
    fprintf(fp, "\t\tsw_rawawb_nor_y1_3 = 0x%0x (%d)\n",                          awb_cfg_v32->nor_y1_3, awb_cfg_v32->nor_y1_3);
    fprintf(fp, "\t\tsw_rawawb_big_x0_3 = 0x%0x (%d)\n",                          awb_cfg_v32->big_x0_3, awb_cfg_v32->big_x0_3);
    fprintf(fp, "\t\tsw_rawawb_big_x1_3 = 0x%0x (%d)\n",                          awb_cfg_v32->big_x1_3, awb_cfg_v32->big_x1_3);
    fprintf(fp, "\t\tsw_rawawb_big_y0_3 = 0x%0x (%d)\n",                          awb_cfg_v32->big_y0_3, awb_cfg_v32->big_y0_3);
    fprintf(fp, "\t\tsw_rawawb_big_y1_3 = 0x%0x (%d)\n",                          awb_cfg_v32->big_y1_3, awb_cfg_v32->big_y1_3);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region0_excen = 0x%0x (%d)\n",              awb_cfg_v32->exc_wp_region0_excen, awb_cfg_v32->exc_wp_region0_excen);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region0_measen = 0x%0x (%d)\n",             awb_cfg_v32->exc_wp_region0_measen, awb_cfg_v32->exc_wp_region0_measen);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region0_domain = 0x%0x (%d)\n",             awb_cfg_v32->exc_wp_region0_domain, awb_cfg_v32->exc_wp_region0_domain);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region1_excen = 0x%0x (%d)\n",              awb_cfg_v32->exc_wp_region1_excen, awb_cfg_v32->exc_wp_region1_excen);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region1_measen = 0x%0x (%d)\n",             awb_cfg_v32->exc_wp_region1_measen, awb_cfg_v32->exc_wp_region1_measen);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region1_domain = 0x%0x (%d)\n",             awb_cfg_v32->exc_wp_region1_domain, awb_cfg_v32->exc_wp_region1_domain);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region2_excen = 0x%0x (%d)\n",              awb_cfg_v32->exc_wp_region2_excen, awb_cfg_v32->exc_wp_region2_excen);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region2_measen = 0x%0x (%d)\n",             awb_cfg_v32->exc_wp_region2_measen, awb_cfg_v32->exc_wp_region2_measen);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region2_domain = 0x%0x (%d)\n",             awb_cfg_v32->exc_wp_region2_domain, awb_cfg_v32->exc_wp_region2_domain);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region3_excen = 0x%0x (%d)\n",              awb_cfg_v32->exc_wp_region3_excen, awb_cfg_v32->exc_wp_region3_excen);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region3_measen = 0x%0x (%d)\n",             awb_cfg_v32->exc_wp_region3_measen, awb_cfg_v32->exc_wp_region3_measen);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region3_domain = 0x%0x (%d)\n",             awb_cfg_v32->exc_wp_region3_domain, awb_cfg_v32->exc_wp_region3_domain);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region4_excen = 0x%0x (%d)\n",              awb_cfg_v32->exc_wp_region4_excen, awb_cfg_v32->exc_wp_region4_excen);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region4_domain = 0x%0x (%d)\n",             awb_cfg_v32->exc_wp_region4_domain, awb_cfg_v32->exc_wp_region4_domain);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region5_excen = 0x%0x (%d)\n",              awb_cfg_v32->exc_wp_region5_excen, awb_cfg_v32->exc_wp_region5_excen);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region5_domain = 0x%0x (%d)\n",             awb_cfg_v32->exc_wp_region5_domain, awb_cfg_v32->exc_wp_region5_domain);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region6_excen = 0x%0x (%d)\n",              awb_cfg_v32->exc_wp_region6_excen, awb_cfg_v32->exc_wp_region6_excen);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region6_domain = 0x%0x (%d)\n",             awb_cfg_v32->exc_wp_region6_domain, awb_cfg_v32->exc_wp_region6_domain);
    fprintf(fp, "\t\tsw_rawawb_multiwindow_en = 0x%0x (%d)\n",                    awb_cfg_v32->multiwindow_en, awb_cfg_v32->multiwindow_en);
    fprintf(fp, "\t\tsw_rawawb_multiwindow0_h_offs = 0x%0x (%d)\n",               awb_cfg_v32->multiwindow0_h_offs, awb_cfg_v32->multiwindow0_h_offs);
    fprintf(fp, "\t\tsw_rawawb_multiwindow0_v_offs = 0x%0x (%d)\n",               awb_cfg_v32->multiwindow0_v_offs, awb_cfg_v32->multiwindow0_v_offs);
    fprintf(fp, "\t\tsw_rawawb_multiwindow0_h_size = 0x%0x (%d)\n",               awb_cfg_v32->multiwindow0_h_size, awb_cfg_v32->multiwindow0_h_size);
    fprintf(fp, "\t\tsw_rawawb_multiwindow1_v_size = 0x%0x (%d)\n",               awb_cfg_v32->multiwindow1_v_size, awb_cfg_v32->multiwindow1_v_size);
    fprintf(fp, "\t\tsw_rawawb_multiwindow1_h_offs = 0x%0x (%d)\n",               awb_cfg_v32->multiwindow1_h_offs, awb_cfg_v32->multiwindow1_h_offs);
    fprintf(fp, "\t\tsw_rawawb_multiwindow1_v_offs = 0x%0x (%d)\n",               awb_cfg_v32->multiwindow1_v_offs, awb_cfg_v32->multiwindow1_v_offs);
    fprintf(fp, "\t\tsw_rawawb_multiwindow1_h_size = 0x%0x (%d)\n",               awb_cfg_v32->multiwindow1_h_size, awb_cfg_v32->multiwindow1_h_size);
    fprintf(fp, "\t\tsw_rawawb_multiwindow1_v_size = 0x%0x (%d)\n",               awb_cfg_v32->multiwindow1_v_size, awb_cfg_v32->multiwindow1_v_size);
    fprintf(fp, "\t\tsw_rawawb_multiwindow2_h_offs = 0x%0x (%d)\n",               awb_cfg_v32->multiwindow2_h_offs, awb_cfg_v32->multiwindow2_h_offs);
    fprintf(fp, "\t\tsw_rawawb_multiwindow2_v_offs = 0x%0x (%d)\n",               awb_cfg_v32->multiwindow2_v_offs, awb_cfg_v32->multiwindow2_v_offs);
    fprintf(fp, "\t\tsw_rawawb_multiwindow2_h_size = 0x%0x (%d)\n",               awb_cfg_v32->multiwindow2_h_size, awb_cfg_v32->multiwindow2_h_size);
    fprintf(fp, "\t\tsw_rawawb_multiwindow2_v_size = 0x%0x (%d)\n",               awb_cfg_v32->multiwindow2_v_size, awb_cfg_v32->multiwindow2_v_size);
    fprintf(fp, "\t\tsw_rawawb_multiwindow3_h_offs = 0x%0x (%d)\n",               awb_cfg_v32->multiwindow3_h_offs, awb_cfg_v32->multiwindow3_h_offs);
    fprintf(fp, "\t\tsw_rawawb_multiwindow3_v_offs = 0x%0x (%d)\n",               awb_cfg_v32->multiwindow3_v_offs, awb_cfg_v32->multiwindow3_v_offs);
    fprintf(fp, "\t\tsw_rawawb_multiwindow3_h_size = 0x%0x (%d)\n",               awb_cfg_v32->multiwindow3_h_size, awb_cfg_v32->multiwindow3_h_size);
    fprintf(fp, "\t\tsw_rawawb_multiwindow3_v_size = 0x%0x (%d)\n",               awb_cfg_v32->multiwindow3_v_size, awb_cfg_v32->multiwindow3_v_size);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region0_xu0 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region0_xu0, awb_cfg_v32->exc_wp_region0_xu0);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region0_xu1 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region0_xu1, awb_cfg_v32->exc_wp_region0_xu1);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region0_yv0 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region0_yv0, awb_cfg_v32->exc_wp_region0_yv0);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region0_yv1 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region0_yv1, awb_cfg_v32->exc_wp_region0_yv1);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region1_xu0 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region1_xu0, awb_cfg_v32->exc_wp_region1_xu0);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region1_xu1 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region1_xu1, awb_cfg_v32->exc_wp_region1_xu1);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region1_yv0 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region1_yv0, awb_cfg_v32->exc_wp_region1_yv0);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region1_yv1 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region1_yv1, awb_cfg_v32->exc_wp_region1_yv1);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region2_xu0 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region2_xu0, awb_cfg_v32->exc_wp_region2_xu0);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region2_xu1 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region2_xu1, awb_cfg_v32->exc_wp_region2_xu1);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region2_yv0 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region2_yv0, awb_cfg_v32->exc_wp_region2_yv0);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region2_yv1 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region2_yv1, awb_cfg_v32->exc_wp_region2_yv1);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region3_xu0 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region3_xu0, awb_cfg_v32->exc_wp_region3_xu0);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region3_xu1 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region3_xu1, awb_cfg_v32->exc_wp_region3_xu1);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region3_yv0 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region3_yv0, awb_cfg_v32->exc_wp_region3_yv0);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region3_yv1 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region3_yv1, awb_cfg_v32->exc_wp_region3_yv1);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region4_xu0 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region4_xu0, awb_cfg_v32->exc_wp_region4_xu0);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region4_xu1 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region4_xu1, awb_cfg_v32->exc_wp_region4_xu1);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region4_yv0 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region4_yv0, awb_cfg_v32->exc_wp_region4_yv0);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region4_yv1 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region4_yv1, awb_cfg_v32->exc_wp_region4_yv1);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region5_xu0 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region5_xu0, awb_cfg_v32->exc_wp_region5_xu0);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region5_xu1 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region5_xu1, awb_cfg_v32->exc_wp_region5_xu1);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region5_yv0 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region5_yv0, awb_cfg_v32->exc_wp_region5_yv0);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region5_yv1 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region5_yv1, awb_cfg_v32->exc_wp_region5_yv1);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region6_xu0 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region6_xu0, awb_cfg_v32->exc_wp_region6_xu0);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region6_xu1 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region6_xu1, awb_cfg_v32->exc_wp_region6_xu1);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region6_yv0 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region6_yv0, awb_cfg_v32->exc_wp_region6_yv0);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region6_yv1 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region6_yv1, awb_cfg_v32->exc_wp_region6_yv1);


    fclose(fp);
#endif
}
void Isp39Params::convertAiqAwbToIsp39Params(struct isp39_isp_params_cfg& isp_cfg,
        const rk_aiq_isp_awb_meas_cfg_v32_t& awb_meas,
        bool awb_cfg_udpate) {
    if (awb_cfg_udpate) {
        if (awb_meas.awbEnable) {
            isp_cfg.module_ens |= ISP2X_MODULE_RAWAWB;
            isp_cfg.module_cfg_update |= ISP2X_MODULE_RAWAWB;
            isp_cfg.module_en_update |= ISP2X_MODULE_RAWAWB;
        }
    } else {
        return;
    }
    //to do :
    //config blc2 important
    //isp_cfg.module_ens |= ISP2X_MODULE_BLS;
    //isp_cfg.module_cfg_update |= ISP2X_MODULE_BLS;
    //isp_cfg.module_en_update |= ISP2X_MODULE_BLS;

    struct isp39_rawawb_meas_cfg* awb_cfg_v32 = &isp_cfg.meas.rawawb;
    awb_cfg_v32->bls2_en = awb_meas.blc.enable;
    awb_cfg_v32->bls2_val.r = awb_meas.blc.blc[0];
    awb_cfg_v32->bls2_val.gr = awb_meas.blc.blc[1];
    awb_cfg_v32->bls2_val.gb = awb_meas.blc.blc[2];
    awb_cfg_v32->bls2_val.b = awb_meas.blc.blc[3];
    LOGV_AWB("blc2_cfg %d %d,%d,%d,%d", awb_cfg_v32->bls2_en, awb_cfg_v32->bls2_val.r, awb_cfg_v32->bls2_val.gr,
             awb_cfg_v32->bls2_val.gb, awb_cfg_v32->bls2_val.b);

    awb_cfg_v32->rawawb_sel = 0;
    if (awb_meas.frameChoose == CALIB_AWB_INPUT_DRC) {
        awb_cfg_v32->drc2awb_sel = 1;
        //awb_cfg_v32->rawawb_sel need check
    } else {
        awb_cfg_v32->drc2awb_sel = 0;
        if (awb_meas.frameChoose == CALIB_AWB_INPUT_BAYERNR) {
            awb_cfg_v32->bnr2awb_sel = 1;
            //awb_cfg_v32->rawawb_sel need check
        }
        else {
            awb_cfg_v32->bnr2awb_sel = 0;
            awb_cfg_v32->rawawb_sel = awb_meas.frameChoose;
        }
    }
    awb_cfg_v32->low12bit_val = awb_meas.inputBitIs12Bit;
    //awb_cfg_v32->ddr_path_en = awb_meas.write2ddrEnable;
    //awb_cfg_v32->ddr_path_sel = awb_meas.write2ddrSelc;
    awb_cfg_v32->in_rshift_to_12bit_en = awb_meas.inputShiftEnable;
    awb_cfg_v32->in_overexposure_check_en = true;
    awb_cfg_v32->in_overexposure_threshold = awb_meas.overexposure_value;
    if(awb_cfg_v32->in_overexposure_threshold == 0) {
        awb_cfg_v32->in_overexposure_check_en = false;
    }
    awb_cfg_v32->xy_en0    = awb_meas.xyDetectionEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201];
    awb_cfg_v32->uv_en0    = awb_meas.uvDetectionEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201];
    awb_cfg_v32->yuv3d_en0 = awb_meas.threeDyuvEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201];
    awb_cfg_v32->xy_en1    = awb_meas.xyDetectionEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201];
    awb_cfg_v32->uv_en1    = awb_meas.uvDetectionEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201];
    awb_cfg_v32->yuv3d_en1 = awb_meas.threeDyuvEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201];
    awb_cfg_v32->wp_blk_wei_en0 =
        awb_meas.blkWeightEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201];
    awb_cfg_v32->wp_blk_wei_en1 = awb_meas.blkWeightEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201];
    awb_cfg_v32->rawlsc_bypass_en      = awb_meas.lscBypEnable;
    awb_cfg_v32->blk_measure_enable   = awb_meas.blkStatisticsEnable;
    awb_cfg_v32->blk_measure_mode     = awb_meas.blkMeasureMode;
    awb_cfg_v32->blk_measure_xytype   = awb_meas.xyRangeTypeForBlkStatistics;
    awb_cfg_v32->blk_measure_illu_idx = awb_meas.illIdxForBlkStatistics;
    awb_cfg_v32->blk_with_luma_wei_en = awb_meas.blkStatisticsWithLumaWeightEn;
    awb_cfg_v32->wp_luma_wei_en0 =
        awb_meas.wpDiffWeiEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201];
    awb_cfg_v32->wp_luma_wei_en1 = awb_meas.wpDiffWeiEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201];
    awb_cfg_v32->wp_hist_xytype  = awb_meas.xyRangeTypeForWpHist;
    awb_cfg_v32->yuv3d_ls_idx0   = awb_meas.threeDyuvIllu[0];
    awb_cfg_v32->yuv3d_ls_idx1   = awb_meas.threeDyuvIllu[1];
    awb_cfg_v32->yuv3d_ls_idx2   = awb_meas.threeDyuvIllu[2];
    awb_cfg_v32->yuv3d_ls_idx3   = awb_meas.threeDyuvIllu[3];
    awb_cfg_v32->light_num       = awb_meas.lightNum;
    awb_cfg_v32->h_offs          = awb_meas.windowSet[0];
    awb_cfg_v32->v_offs          = awb_meas.windowSet[1];
    awb_cfg_v32->h_size          = awb_meas.windowSet[2];
    awb_cfg_v32->v_size          = awb_meas.windowSet[3];
#if RKAIQ_HAVE_AWB_V32LT||ISP_HW_V39
    if(awb_meas.dsMode == RK_AIQ_AWB_DS_4X4 || awb_meas.dsMode == RK_AIQ_AWB_DS_8X8) {
        awb_cfg_v32->wind_size       = awb_meas.dsMode;
        awb_cfg_v32->ds16x8_mode_en = 0;
    } else if(awb_meas.dsMode == RK_AIQ_AWB_DS_16X8) {
        awb_cfg_v32->ds16x8_mode_en = 1;
        awb_cfg_v32->wind_size = RK_AIQ_AWB_DS_8X8;
    }
#else
    awb_cfg_v32->wind_size       = awb_meas.dsMode;
#endif
    awb_cfg_v32->r_max           = awb_meas.maxR;
    awb_cfg_v32->g_max           = awb_meas.maxG;
    awb_cfg_v32->b_max           = awb_meas.maxB;
    awb_cfg_v32->y_max           = awb_meas.maxY;
    awb_cfg_v32->r_min           = awb_meas.minR;
    awb_cfg_v32->g_min           = awb_meas.minG;
    awb_cfg_v32->b_min           = awb_meas.minB;
    awb_cfg_v32->y_min           = awb_meas.minY;
    awb_cfg_v32->vertex0_u_0     = awb_meas.uvRange_param[0].pu_region[0];
    awb_cfg_v32->vertex0_v_0     = awb_meas.uvRange_param[0].pv_region[0];
    awb_cfg_v32->vertex1_u_0     = awb_meas.uvRange_param[0].pu_region[1];
    awb_cfg_v32->vertex1_v_0     = awb_meas.uvRange_param[0].pv_region[1];
    awb_cfg_v32->vertex2_u_0     = awb_meas.uvRange_param[0].pu_region[2];
    awb_cfg_v32->vertex2_v_0     = awb_meas.uvRange_param[0].pv_region[2];
    awb_cfg_v32->vertex3_u_0     = awb_meas.uvRange_param[0].pu_region[3];
    awb_cfg_v32->vertex3_v_0     = awb_meas.uvRange_param[0].pv_region[3];
    awb_cfg_v32->islope01_0      = awb_meas.uvRange_param[0].slope_inv[0];
    awb_cfg_v32->islope12_0      = awb_meas.uvRange_param[0].slope_inv[1];
    awb_cfg_v32->islope23_0      = awb_meas.uvRange_param[0].slope_inv[2];
    awb_cfg_v32->islope30_0      = awb_meas.uvRange_param[0].slope_inv[3];
    awb_cfg_v32->vertex0_u_1     = awb_meas.uvRange_param[1].pu_region[0];
    awb_cfg_v32->vertex0_v_1     = awb_meas.uvRange_param[1].pv_region[0];
    awb_cfg_v32->vertex1_u_1     = awb_meas.uvRange_param[1].pu_region[1];
    awb_cfg_v32->vertex1_v_1     = awb_meas.uvRange_param[1].pv_region[1];
    awb_cfg_v32->vertex2_u_1     = awb_meas.uvRange_param[1].pu_region[2];
    awb_cfg_v32->vertex2_v_1     = awb_meas.uvRange_param[1].pv_region[2];
    awb_cfg_v32->vertex3_u_1     = awb_meas.uvRange_param[1].pu_region[3];
    awb_cfg_v32->vertex3_v_1     = awb_meas.uvRange_param[1].pv_region[3];
    awb_cfg_v32->islope01_1      = awb_meas.uvRange_param[1].slope_inv[0];
    awb_cfg_v32->islope12_1      = awb_meas.uvRange_param[1].slope_inv[1];
    awb_cfg_v32->islope23_1      = awb_meas.uvRange_param[1].slope_inv[2];
    awb_cfg_v32->islope30_1      = awb_meas.uvRange_param[1].slope_inv[3];
    awb_cfg_v32->vertex0_u_2     = awb_meas.uvRange_param[2].pu_region[0];
    awb_cfg_v32->vertex0_v_2     = awb_meas.uvRange_param[2].pv_region[0];
    awb_cfg_v32->vertex1_u_2     = awb_meas.uvRange_param[2].pu_region[1];
    awb_cfg_v32->vertex1_v_2     = awb_meas.uvRange_param[2].pv_region[1];
    awb_cfg_v32->vertex2_u_2     = awb_meas.uvRange_param[2].pu_region[2];
    awb_cfg_v32->vertex2_v_2     = awb_meas.uvRange_param[2].pv_region[2];
    awb_cfg_v32->vertex3_u_2     = awb_meas.uvRange_param[2].pu_region[3];
    awb_cfg_v32->vertex3_v_2     = awb_meas.uvRange_param[2].pv_region[3];
    awb_cfg_v32->islope01_2      = awb_meas.uvRange_param[2].slope_inv[0];
    awb_cfg_v32->islope12_2      = awb_meas.uvRange_param[2].slope_inv[1];
    awb_cfg_v32->islope23_2      = awb_meas.uvRange_param[2].slope_inv[2];
    awb_cfg_v32->islope30_2      = awb_meas.uvRange_param[2].slope_inv[3];
    awb_cfg_v32->vertex0_u_3     = awb_meas.uvRange_param[3].pu_region[0];
    awb_cfg_v32->vertex0_v_3     = awb_meas.uvRange_param[3].pv_region[0];
    awb_cfg_v32->vertex1_u_3     = awb_meas.uvRange_param[3].pu_region[1];
    awb_cfg_v32->vertex1_v_3     = awb_meas.uvRange_param[3].pv_region[1];
    awb_cfg_v32->vertex2_u_3     = awb_meas.uvRange_param[3].pu_region[2];
    awb_cfg_v32->vertex2_v_3     = awb_meas.uvRange_param[3].pv_region[2];
    awb_cfg_v32->vertex3_u_3     = awb_meas.uvRange_param[3].pu_region[3];
    awb_cfg_v32->vertex3_v_3     = awb_meas.uvRange_param[3].pv_region[3];
    awb_cfg_v32->islope01_3      = awb_meas.uvRange_param[3].slope_inv[0];
    awb_cfg_v32->islope12_3      = awb_meas.uvRange_param[3].slope_inv[1];
    awb_cfg_v32->islope23_3      = awb_meas.uvRange_param[3].slope_inv[2];
    awb_cfg_v32->islope30_3      = awb_meas.uvRange_param[3].slope_inv[3];
    awb_cfg_v32->rgb2ryuvmat0_u  = awb_meas.icrgb2RYuv_matrix[0];
    awb_cfg_v32->rgb2ryuvmat1_u  = awb_meas.icrgb2RYuv_matrix[1];
    awb_cfg_v32->rgb2ryuvmat2_u  = awb_meas.icrgb2RYuv_matrix[2];
    awb_cfg_v32->rgb2ryuvofs_u   = awb_meas.icrgb2RYuv_matrix[3];
    awb_cfg_v32->rgb2ryuvmat0_v  = awb_meas.icrgb2RYuv_matrix[4];
    awb_cfg_v32->rgb2ryuvmat1_v  = awb_meas.icrgb2RYuv_matrix[5];
    awb_cfg_v32->rgb2ryuvmat2_v  = awb_meas.icrgb2RYuv_matrix[6];
    awb_cfg_v32->rgb2ryuvofs_v   = awb_meas.icrgb2RYuv_matrix[7];
    awb_cfg_v32->rgb2ryuvmat0_y  = awb_meas.icrgb2RYuv_matrix[8];
    awb_cfg_v32->rgb2ryuvmat1_y  = awb_meas.icrgb2RYuv_matrix[9];
    awb_cfg_v32->rgb2ryuvmat2_y  = awb_meas.icrgb2RYuv_matrix[10];
    awb_cfg_v32->rgb2ryuvofs_y   = awb_meas.icrgb2RYuv_matrix[11];
    awb_cfg_v32->rotu0_ls0       = awb_meas.ic3Dyuv2Range_param[0].thcurve_u[0];
    awb_cfg_v32->rotu1_ls0       = awb_meas.ic3Dyuv2Range_param[0].thcurve_u[1];
    awb_cfg_v32->rotu2_ls0       = awb_meas.ic3Dyuv2Range_param[0].thcurve_u[2];
    awb_cfg_v32->rotu3_ls0       = awb_meas.ic3Dyuv2Range_param[0].thcurve_u[3];
    awb_cfg_v32->rotu4_ls0       = awb_meas.ic3Dyuv2Range_param[0].thcurve_u[4];
    awb_cfg_v32->rotu5_ls0       = awb_meas.ic3Dyuv2Range_param[0].thcurve_u[5];
    awb_cfg_v32->th0_ls0         = awb_meas.ic3Dyuv2Range_param[0].thcure_th[0];
    awb_cfg_v32->th1_ls0         = awb_meas.ic3Dyuv2Range_param[0].thcure_th[1];
    awb_cfg_v32->th2_ls0         = awb_meas.ic3Dyuv2Range_param[0].thcure_th[2];
    awb_cfg_v32->th3_ls0         = awb_meas.ic3Dyuv2Range_param[0].thcure_th[3];
    awb_cfg_v32->th4_ls0         = awb_meas.ic3Dyuv2Range_param[0].thcure_th[4];
    awb_cfg_v32->th5_ls0         = awb_meas.ic3Dyuv2Range_param[0].thcure_th[5];
    awb_cfg_v32->coor_x1_ls0_u   = awb_meas.ic3Dyuv2Range_param[0].lineP1[0];
    awb_cfg_v32->coor_x1_ls0_v   = awb_meas.ic3Dyuv2Range_param[0].lineP1[1];
    awb_cfg_v32->coor_x1_ls0_y   = awb_meas.ic3Dyuv2Range_param[0].lineP1[2];
    awb_cfg_v32->vec_x21_ls0_u   = awb_meas.ic3Dyuv2Range_param[0].vP1P2[0];
    awb_cfg_v32->vec_x21_ls0_v   = awb_meas.ic3Dyuv2Range_param[0].vP1P2[1];
    awb_cfg_v32->vec_x21_ls0_y   = awb_meas.ic3Dyuv2Range_param[0].vP1P2[2];
    awb_cfg_v32->dis_x1x2_ls0    = awb_meas.ic3Dyuv2Range_param[0].disP1P2;
    awb_cfg_v32->rotu0_ls1       = awb_meas.ic3Dyuv2Range_param[1].thcurve_u[0];
    awb_cfg_v32->rotu1_ls1       = awb_meas.ic3Dyuv2Range_param[1].thcurve_u[1];
    awb_cfg_v32->rotu2_ls1       = awb_meas.ic3Dyuv2Range_param[1].thcurve_u[2];
    awb_cfg_v32->rotu3_ls1       = awb_meas.ic3Dyuv2Range_param[1].thcurve_u[3];
    awb_cfg_v32->rotu4_ls1       = awb_meas.ic3Dyuv2Range_param[1].thcurve_u[4];
    awb_cfg_v32->rotu5_ls1       = awb_meas.ic3Dyuv2Range_param[1].thcurve_u[5];
    awb_cfg_v32->th0_ls1         = awb_meas.ic3Dyuv2Range_param[1].thcure_th[0];
    awb_cfg_v32->th1_ls1         = awb_meas.ic3Dyuv2Range_param[1].thcure_th[1];
    awb_cfg_v32->th2_ls1         = awb_meas.ic3Dyuv2Range_param[1].thcure_th[2];
    awb_cfg_v32->th3_ls1         = awb_meas.ic3Dyuv2Range_param[1].thcure_th[3];
    awb_cfg_v32->th4_ls1         = awb_meas.ic3Dyuv2Range_param[1].thcure_th[4];
    awb_cfg_v32->th5_ls1         = awb_meas.ic3Dyuv2Range_param[1].thcure_th[5];
    awb_cfg_v32->coor_x1_ls1_u   = awb_meas.ic3Dyuv2Range_param[1].lineP1[0];
    awb_cfg_v32->coor_x1_ls1_v   = awb_meas.ic3Dyuv2Range_param[1].lineP1[1];
    awb_cfg_v32->coor_x1_ls1_y   = awb_meas.ic3Dyuv2Range_param[1].lineP1[2];
    awb_cfg_v32->vec_x21_ls1_u   = awb_meas.ic3Dyuv2Range_param[1].vP1P2[0];
    awb_cfg_v32->vec_x21_ls1_v   = awb_meas.ic3Dyuv2Range_param[1].vP1P2[1];
    awb_cfg_v32->vec_x21_ls1_y   = awb_meas.ic3Dyuv2Range_param[1].vP1P2[2];
    awb_cfg_v32->dis_x1x2_ls1    = awb_meas.ic3Dyuv2Range_param[1].disP1P2;
    awb_cfg_v32->rotu0_ls2       = awb_meas.ic3Dyuv2Range_param[2].thcurve_u[0];
    awb_cfg_v32->rotu1_ls2       = awb_meas.ic3Dyuv2Range_param[2].thcurve_u[1];
    awb_cfg_v32->rotu2_ls2       = awb_meas.ic3Dyuv2Range_param[2].thcurve_u[2];
    awb_cfg_v32->rotu3_ls2       = awb_meas.ic3Dyuv2Range_param[2].thcurve_u[3];
    awb_cfg_v32->rotu4_ls2       = awb_meas.ic3Dyuv2Range_param[2].thcurve_u[4];
    awb_cfg_v32->rotu5_ls2       = awb_meas.ic3Dyuv2Range_param[2].thcurve_u[5];
    awb_cfg_v32->th0_ls2         = awb_meas.ic3Dyuv2Range_param[2].thcure_th[0];
    awb_cfg_v32->th1_ls2         = awb_meas.ic3Dyuv2Range_param[2].thcure_th[1];
    awb_cfg_v32->th2_ls2         = awb_meas.ic3Dyuv2Range_param[2].thcure_th[2];
    awb_cfg_v32->th3_ls2         = awb_meas.ic3Dyuv2Range_param[2].thcure_th[3];
    awb_cfg_v32->th4_ls2         = awb_meas.ic3Dyuv2Range_param[2].thcure_th[4];
    awb_cfg_v32->th5_ls2         = awb_meas.ic3Dyuv2Range_param[2].thcure_th[5];
    awb_cfg_v32->coor_x1_ls2_u   = awb_meas.ic3Dyuv2Range_param[2].lineP1[0];
    awb_cfg_v32->coor_x1_ls2_v   = awb_meas.ic3Dyuv2Range_param[2].lineP1[1];
    awb_cfg_v32->coor_x1_ls2_y   = awb_meas.ic3Dyuv2Range_param[2].lineP1[2];
    awb_cfg_v32->vec_x21_ls2_u   = awb_meas.ic3Dyuv2Range_param[2].vP1P2[0];
    awb_cfg_v32->vec_x21_ls2_v   = awb_meas.ic3Dyuv2Range_param[2].vP1P2[1];
    awb_cfg_v32->vec_x21_ls2_y   = awb_meas.ic3Dyuv2Range_param[2].vP1P2[2];
    awb_cfg_v32->dis_x1x2_ls2    = awb_meas.ic3Dyuv2Range_param[2].disP1P2;

    awb_cfg_v32->rotu0_ls3        = awb_meas.ic3Dyuv2Range_param[3].thcurve_u[0];
    awb_cfg_v32->rotu1_ls3        = awb_meas.ic3Dyuv2Range_param[3].thcurve_u[1];
    awb_cfg_v32->rotu2_ls3        = awb_meas.ic3Dyuv2Range_param[3].thcurve_u[2];
    awb_cfg_v32->rotu3_ls3        = awb_meas.ic3Dyuv2Range_param[3].thcurve_u[3];
    awb_cfg_v32->rotu4_ls3        = awb_meas.ic3Dyuv2Range_param[3].thcurve_u[4];
    awb_cfg_v32->rotu5_ls3        = awb_meas.ic3Dyuv2Range_param[3].thcurve_u[5];
    awb_cfg_v32->th0_ls3          = awb_meas.ic3Dyuv2Range_param[3].thcure_th[0];
    awb_cfg_v32->th1_ls3          = awb_meas.ic3Dyuv2Range_param[3].thcure_th[1];
    awb_cfg_v32->th2_ls3          = awb_meas.ic3Dyuv2Range_param[3].thcure_th[2];
    awb_cfg_v32->th3_ls3          = awb_meas.ic3Dyuv2Range_param[3].thcure_th[3];
    awb_cfg_v32->th4_ls3          = awb_meas.ic3Dyuv2Range_param[3].thcure_th[4];
    awb_cfg_v32->th5_ls3          = awb_meas.ic3Dyuv2Range_param[3].thcure_th[5];
    awb_cfg_v32->coor_x1_ls3_u    = awb_meas.ic3Dyuv2Range_param[3].lineP1[0];
    awb_cfg_v32->coor_x1_ls3_v    = awb_meas.ic3Dyuv2Range_param[3].lineP1[1];
    awb_cfg_v32->coor_x1_ls3_y    = awb_meas.ic3Dyuv2Range_param[3].lineP1[2];
    awb_cfg_v32->vec_x21_ls3_u    = awb_meas.ic3Dyuv2Range_param[3].vP1P2[0];
    awb_cfg_v32->vec_x21_ls3_v    = awb_meas.ic3Dyuv2Range_param[3].vP1P2[1];
    awb_cfg_v32->vec_x21_ls3_y    = awb_meas.ic3Dyuv2Range_param[3].vP1P2[2];
    awb_cfg_v32->dis_x1x2_ls3     = awb_meas.ic3Dyuv2Range_param[3].disP1P2;
    awb_cfg_v32->wt0              = awb_meas.rgb2xy_param.pseudoLuminanceWeight[0];
    awb_cfg_v32->wt1              = awb_meas.rgb2xy_param.pseudoLuminanceWeight[1];
    awb_cfg_v32->wt2              = awb_meas.rgb2xy_param.pseudoLuminanceWeight[2];
    awb_cfg_v32->mat0_x           = awb_meas.rgb2xy_param.rotationMat[0];
    awb_cfg_v32->mat1_x           = awb_meas.rgb2xy_param.rotationMat[1];
    awb_cfg_v32->mat2_x           = awb_meas.rgb2xy_param.rotationMat[2];
    awb_cfg_v32->mat0_y           = awb_meas.rgb2xy_param.rotationMat[3];
    awb_cfg_v32->mat1_y           = awb_meas.rgb2xy_param.rotationMat[4];
    awb_cfg_v32->mat2_y           = awb_meas.rgb2xy_param.rotationMat[5];
    awb_cfg_v32->nor_x0_0         = awb_meas.xyRange_param[0].NorrangeX[0];
    awb_cfg_v32->nor_x1_0         = awb_meas.xyRange_param[0].NorrangeX[1];
    awb_cfg_v32->nor_y0_0         = awb_meas.xyRange_param[0].NorrangeY[0];
    awb_cfg_v32->nor_y1_0         = awb_meas.xyRange_param[0].NorrangeY[1];
    awb_cfg_v32->big_x0_0         = awb_meas.xyRange_param[0].SperangeX[0];
    awb_cfg_v32->big_x1_0         = awb_meas.xyRange_param[0].SperangeX[1];
    awb_cfg_v32->big_y0_0         = awb_meas.xyRange_param[0].SperangeY[0];
    awb_cfg_v32->big_y1_0         = awb_meas.xyRange_param[0].SperangeY[1];
    awb_cfg_v32->nor_x0_1         = awb_meas.xyRange_param[1].NorrangeX[0];
    awb_cfg_v32->nor_x1_1         = awb_meas.xyRange_param[1].NorrangeX[1];
    awb_cfg_v32->nor_y0_1         = awb_meas.xyRange_param[1].NorrangeY[0];
    awb_cfg_v32->nor_y1_1         = awb_meas.xyRange_param[1].NorrangeY[1];
    awb_cfg_v32->big_x0_1         = awb_meas.xyRange_param[1].SperangeX[0];
    awb_cfg_v32->big_x1_1         = awb_meas.xyRange_param[1].SperangeX[1];
    awb_cfg_v32->big_y0_1         = awb_meas.xyRange_param[1].SperangeY[0];
    awb_cfg_v32->big_y1_1         = awb_meas.xyRange_param[1].SperangeY[1];
    awb_cfg_v32->nor_x0_2         = awb_meas.xyRange_param[2].NorrangeX[0];
    awb_cfg_v32->nor_x1_2         = awb_meas.xyRange_param[2].NorrangeX[1];
    awb_cfg_v32->nor_y0_2         = awb_meas.xyRange_param[2].NorrangeY[0];
    awb_cfg_v32->nor_y1_2         = awb_meas.xyRange_param[2].NorrangeY[1];
    awb_cfg_v32->big_x0_2         = awb_meas.xyRange_param[2].SperangeX[0];
    awb_cfg_v32->big_x1_2         = awb_meas.xyRange_param[2].SperangeX[1];
    awb_cfg_v32->big_y0_2         = awb_meas.xyRange_param[2].SperangeY[0];
    awb_cfg_v32->big_y1_2         = awb_meas.xyRange_param[2].SperangeY[1];
    awb_cfg_v32->nor_x0_3         = awb_meas.xyRange_param[3].NorrangeX[0];
    awb_cfg_v32->nor_x1_3         = awb_meas.xyRange_param[3].NorrangeX[1];
    awb_cfg_v32->nor_y0_3         = awb_meas.xyRange_param[3].NorrangeY[0];
    awb_cfg_v32->nor_y1_3         = awb_meas.xyRange_param[3].NorrangeY[1];
    awb_cfg_v32->big_x0_3         = awb_meas.xyRange_param[3].SperangeX[0];
    awb_cfg_v32->big_x1_3         = awb_meas.xyRange_param[3].SperangeX[1];
    awb_cfg_v32->big_y0_3         = awb_meas.xyRange_param[3].SperangeY[0];
    awb_cfg_v32->big_y1_3         = awb_meas.xyRange_param[3].SperangeY[1];
    awb_cfg_v32->pre_wbgain_inv_r = awb_meas.pre_wbgain_inv_r;
    awb_cfg_v32->pre_wbgain_inv_g = awb_meas.pre_wbgain_inv_g;
    awb_cfg_v32->pre_wbgain_inv_b = awb_meas.pre_wbgain_inv_b;
    awb_cfg_v32->multiwindow_en      = awb_meas.multiwindow_en;
    awb_cfg_v32->multiwindow0_h_offs = awb_meas.multiwindow[0][0];
    awb_cfg_v32->multiwindow0_v_offs = awb_meas.multiwindow[0][1];
    awb_cfg_v32->multiwindow0_h_size =
        awb_meas.multiwindow[0][0] + awb_meas.multiwindow[0][2];
    awb_cfg_v32->multiwindow0_v_size =
        awb_meas.multiwindow[0][1] + awb_meas.multiwindow[0][3];
    awb_cfg_v32->multiwindow1_h_offs = awb_meas.multiwindow[1][0];
    awb_cfg_v32->multiwindow1_v_offs = awb_meas.multiwindow[1][1];
    awb_cfg_v32->multiwindow1_h_size =
        awb_meas.multiwindow[1][0] + awb_meas.multiwindow[1][2];
    awb_cfg_v32->multiwindow1_v_size =
        awb_meas.multiwindow[1][1] + awb_meas.multiwindow[1][3];
    awb_cfg_v32->multiwindow2_h_offs = awb_meas.multiwindow[2][0];
    awb_cfg_v32->multiwindow2_v_offs = awb_meas.multiwindow[2][1];
    awb_cfg_v32->multiwindow2_h_size =
        awb_meas.multiwindow[2][0] + awb_meas.multiwindow[2][2];
    awb_cfg_v32->multiwindow2_v_size =
        awb_meas.multiwindow[2][1] + awb_meas.multiwindow[2][3];
    awb_cfg_v32->multiwindow3_h_offs = awb_meas.multiwindow[3][0];
    awb_cfg_v32->multiwindow3_v_offs = awb_meas.multiwindow[3][1];
    awb_cfg_v32->multiwindow3_h_size =
        awb_meas.multiwindow[3][0] + awb_meas.multiwindow[3][2];
    awb_cfg_v32->multiwindow3_v_size =
        awb_meas.multiwindow[3][1] + awb_meas.multiwindow[3][3];
    int exc_wp_region0_excen0 =
        awb_meas.excludeWpRange[0].excludeEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201];
    int exc_wp_region0_excen1 =
        awb_meas.excludeWpRange[0].excludeEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201];
    awb_cfg_v32->exc_wp_region0_excen = ((exc_wp_region0_excen1 << 1) + exc_wp_region0_excen0) & 0x3;
    awb_cfg_v32->exc_wp_region0_measen = awb_meas.excludeWpRange[0].measureEnable;
    awb_cfg_v32->exc_wp_region0_domain = awb_meas.excludeWpRange[0].domain;
    awb_cfg_v32->exc_wp_region0_xu0    = awb_meas.excludeWpRange[0].xu[0];
    awb_cfg_v32->exc_wp_region0_xu1    = awb_meas.excludeWpRange[0].xu[1];
    awb_cfg_v32->exc_wp_region0_yv0    = awb_meas.excludeWpRange[0].yv[0];
    awb_cfg_v32->exc_wp_region0_yv1    = awb_meas.excludeWpRange[0].yv[1];
    awb_cfg_v32->exc_wp_region0_weight = awb_meas.excludeWpRange[0].weightInculde;
    int exc_wp_region1_excen0 =
        awb_meas.excludeWpRange[1].excludeEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201];
    int exc_wp_region1_excen1 =
        awb_meas.excludeWpRange[1].excludeEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201];
    awb_cfg_v32->exc_wp_region1_excen =  ((exc_wp_region1_excen1 << 1) + exc_wp_region1_excen0) & 0x3;
    awb_cfg_v32->exc_wp_region1_measen = awb_meas.excludeWpRange[1].measureEnable;
    awb_cfg_v32->exc_wp_region1_domain = awb_meas.excludeWpRange[1].domain;
    awb_cfg_v32->exc_wp_region1_xu0    = awb_meas.excludeWpRange[1].xu[0];
    awb_cfg_v32->exc_wp_region1_xu1    = awb_meas.excludeWpRange[1].xu[1];
    awb_cfg_v32->exc_wp_region1_yv0    = awb_meas.excludeWpRange[1].yv[0];
    awb_cfg_v32->exc_wp_region1_yv1    = awb_meas.excludeWpRange[1].yv[1];
    awb_cfg_v32->exc_wp_region1_weight = awb_meas.excludeWpRange[1].weightInculde;
    int exc_wp_region2_excen0 =
        awb_meas.excludeWpRange[2].excludeEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201];
    int exc_wp_region2_excen1 =
        awb_meas.excludeWpRange[2].excludeEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201];
    awb_cfg_v32->exc_wp_region2_excen = ((exc_wp_region2_excen1 << 1) + exc_wp_region2_excen0) & 0x3;
    awb_cfg_v32->exc_wp_region2_measen = awb_meas.excludeWpRange[2].measureEnable;
    awb_cfg_v32->exc_wp_region2_domain = awb_meas.excludeWpRange[2].domain;
    awb_cfg_v32->exc_wp_region2_xu0    = awb_meas.excludeWpRange[2].xu[0];
    awb_cfg_v32->exc_wp_region2_xu1    = awb_meas.excludeWpRange[2].xu[1];
    awb_cfg_v32->exc_wp_region2_yv0    = awb_meas.excludeWpRange[2].yv[0];
    awb_cfg_v32->exc_wp_region2_yv1    = awb_meas.excludeWpRange[2].yv[1];
    awb_cfg_v32->exc_wp_region2_weight = awb_meas.excludeWpRange[2].weightInculde;
    int exc_wp_region3_excen0 =
        awb_meas.excludeWpRange[3].excludeEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201];
    int exc_wp_region3_excen1 =
        awb_meas.excludeWpRange[3].excludeEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201];
    awb_cfg_v32->exc_wp_region3_excen = ((exc_wp_region3_excen1 << 1) + exc_wp_region3_excen0) & 0x3;
    awb_cfg_v32->exc_wp_region3_measen = awb_meas.excludeWpRange[3].measureEnable;
    awb_cfg_v32->exc_wp_region3_domain = awb_meas.excludeWpRange[3].domain;
    awb_cfg_v32->exc_wp_region3_xu0    = awb_meas.excludeWpRange[3].xu[0];
    awb_cfg_v32->exc_wp_region3_xu1    = awb_meas.excludeWpRange[3].xu[1];
    awb_cfg_v32->exc_wp_region3_yv0    = awb_meas.excludeWpRange[3].yv[0];
    awb_cfg_v32->exc_wp_region3_yv1    = awb_meas.excludeWpRange[3].yv[1];
    awb_cfg_v32->exc_wp_region3_weight = awb_meas.excludeWpRange[3].weightInculde;
    int exc_wp_region4_excen0 =
        awb_meas.excludeWpRange[4].excludeEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201];
    int exc_wp_region4_excen1 =
        awb_meas.excludeWpRange[4].excludeEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201];
    awb_cfg_v32->exc_wp_region4_excen = ((exc_wp_region4_excen1 << 1) + exc_wp_region4_excen0) & 0x3;
    awb_cfg_v32->exc_wp_region4_domain = awb_meas.excludeWpRange[4].domain;
    awb_cfg_v32->exc_wp_region4_xu0    = awb_meas.excludeWpRange[4].xu[0];
    awb_cfg_v32->exc_wp_region4_xu1    = awb_meas.excludeWpRange[4].xu[1];
    awb_cfg_v32->exc_wp_region4_yv0    = awb_meas.excludeWpRange[4].yv[0];
    awb_cfg_v32->exc_wp_region4_yv1    = awb_meas.excludeWpRange[4].yv[1];
    awb_cfg_v32->exc_wp_region4_weight = awb_meas.excludeWpRange[4].weightInculde;
    int exc_wp_region5_excen0 =
        awb_meas.excludeWpRange[5].excludeEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201];
    int exc_wp_region5_excen1 =
        awb_meas.excludeWpRange[5].excludeEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201];
    awb_cfg_v32->exc_wp_region5_excen = ((exc_wp_region5_excen1 << 1) + exc_wp_region5_excen0) & 0x3;
    awb_cfg_v32->exc_wp_region5_domain = awb_meas.excludeWpRange[5].domain;
    awb_cfg_v32->exc_wp_region5_xu0    = awb_meas.excludeWpRange[5].xu[0];
    awb_cfg_v32->exc_wp_region5_xu1    = awb_meas.excludeWpRange[5].xu[1];
    awb_cfg_v32->exc_wp_region5_yv0    = awb_meas.excludeWpRange[5].yv[0];
    awb_cfg_v32->exc_wp_region5_yv1    = awb_meas.excludeWpRange[5].yv[1];
    awb_cfg_v32->exc_wp_region5_weight = awb_meas.excludeWpRange[5].weightInculde;
    int exc_wp_region6_excen0 =
        awb_meas.excludeWpRange[6].excludeEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201];
    int exc_wp_region6_excen1 =
        awb_meas.excludeWpRange[6].excludeEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201];
    awb_cfg_v32->exc_wp_region6_excen = ((exc_wp_region6_excen1 << 1) + exc_wp_region6_excen0) & 0x3;
    awb_cfg_v32->exc_wp_region6_domain = awb_meas.excludeWpRange[6].domain;
    awb_cfg_v32->exc_wp_region6_xu0    = awb_meas.excludeWpRange[6].xu[0];
    awb_cfg_v32->exc_wp_region6_xu1    = awb_meas.excludeWpRange[6].xu[1];
    awb_cfg_v32->exc_wp_region6_yv0    = awb_meas.excludeWpRange[6].yv[0];
    awb_cfg_v32->exc_wp_region6_yv1    = awb_meas.excludeWpRange[6].yv[1];
    awb_cfg_v32->exc_wp_region6_weight = awb_meas.excludeWpRange[6].weightInculde;
    awb_cfg_v32->wp_luma_weicurve_y0   = awb_meas.wpDiffwei_y[0];
    awb_cfg_v32->wp_luma_weicurve_y1   = awb_meas.wpDiffwei_y[1];
    awb_cfg_v32->wp_luma_weicurve_y2   = awb_meas.wpDiffwei_y[2];
    awb_cfg_v32->wp_luma_weicurve_y3   = awb_meas.wpDiffwei_y[3];
    awb_cfg_v32->wp_luma_weicurve_y4   = awb_meas.wpDiffwei_y[4];
    awb_cfg_v32->wp_luma_weicurve_y5   = awb_meas.wpDiffwei_y[5];
    awb_cfg_v32->wp_luma_weicurve_y6   = awb_meas.wpDiffwei_y[6];
    awb_cfg_v32->wp_luma_weicurve_y7   = awb_meas.wpDiffwei_y[7];
    awb_cfg_v32->wp_luma_weicurve_y8   = awb_meas.wpDiffwei_y[8];
    awb_cfg_v32->wp_luma_weicurve_w0   = awb_meas.wpDiffwei_w[0];
    awb_cfg_v32->wp_luma_weicurve_w1   = awb_meas.wpDiffwei_w[1];
    awb_cfg_v32->wp_luma_weicurve_w2   = awb_meas.wpDiffwei_w[2];
    awb_cfg_v32->wp_luma_weicurve_w3   = awb_meas.wpDiffwei_w[3];
    awb_cfg_v32->wp_luma_weicurve_w4   = awb_meas.wpDiffwei_w[4];
    awb_cfg_v32->wp_luma_weicurve_w5   = awb_meas.wpDiffwei_w[5];
    awb_cfg_v32->wp_luma_weicurve_w6   = awb_meas.wpDiffwei_w[6];
    awb_cfg_v32->wp_luma_weicurve_w7   = awb_meas.wpDiffwei_w[7];
    awb_cfg_v32->wp_luma_weicurve_w8   = awb_meas.wpDiffwei_w[8];
#if RKAIQ_HAVE_AWB_V32LT
    for (int i = 0; i < ISP32L_RAWAWB_WEIGHT_NUM; i++) {
        awb_cfg_v32->win_weight[i] = awb_meas.blkWeight[i * 5] |
                                     (awb_meas.blkWeight[i * 5 + 1] << 6) |
                                     (awb_meas.blkWeight[i * 5 + 2] << 12) |
                                     (awb_meas.blkWeight[i * 5 + 3] << 18) |
                                     (awb_meas.blkWeight[i * 5 + 4] << 24);
    }
#else
    for (int i = 0; i < RK_AIQ_AWB_GRID_NUM_TOTAL; i++) {
        awb_cfg_v32->wp_blk_wei_w[i] = awb_meas.blkWeight[i];
    }
#endif
    awb_cfg_v32->blk_rtdw_measure_en = awb_meas.blk_rtdw_measure_en;

    WriteAwbReg(awb_cfg_v32);

}
#endif

#if RKAIQ_HAVE_AF_V33 || RKAIQ_ONLY_AF_STATS_V33
#ifdef USE_NEWSTRUCT
void Isp39Params::convertAiqAfToIsp39Params(struct isp39_isp_params_cfg& isp_cfg,
        const afStats_cfg_t& af_data, bool af_cfg_udpate)
{
    const afStats_cfg_t *afStats_cfg = &af_data;
    struct isp39_rawaf_meas_cfg *rawaf = &isp_cfg.meas.rawaf;
    int i;

    if (!af_cfg_udpate) return;

    if (af_data.hw_afCfg_stats_en) isp_cfg.module_ens |= ISP2X_MODULE_RAWAF;
    isp_cfg.module_en_update |= ISP2X_MODULE_RAWAF;
    isp_cfg.module_cfg_update |= ISP2X_MODULE_RAWAF;

    switch (afStats_cfg->hw_afCfg_statsSrc_mode) {
        case afStats_vicapOut_mode:
            rawaf->bnr2af_sel = 0;
            rawaf->from_ynr = 1;
            rawaf->rawaf_sel = 0;
            break;
        case afStats_ynrOut_mode:
            rawaf->bnr2af_sel = 0;
            rawaf->from_ynr = 1;
            rawaf->rawaf_sel = 0;
            break;
        case afStats_btnrOut_mode:
            rawaf->bnr2af_sel = 1;
            rawaf->from_ynr = 0;
            rawaf->rawaf_sel = 0;
            break;
        case afStats_dmIn_mode:
            rawaf->bnr2af_sel = 0;
            rawaf->from_ynr = 0;
            rawaf->rawaf_sel = 3;
            break;
        case afStats_chl2Wb0Out_mode:
            rawaf->bnr2af_sel = 0;
            rawaf->from_ynr = 0;
            rawaf->rawaf_sel = 2;
            break;
        case afStats_chl1Wb0Out_mode:
            rawaf->bnr2af_sel = 0;
            rawaf->from_ynr = 0;
            rawaf->rawaf_sel = 1;
            break;
        default:
            rawaf->bnr2af_sel = 0;
            rawaf->from_ynr = 0;
            rawaf->rawaf_sel = 0;
            break;
    }

    rawaf->tnrin_shift = afStats_cfg->hw_afCfg_statsBtnrOut_shift;
    rawaf->win[0].h_offs = afStats_cfg->mainWin.hw_afCfg_win_x;
    rawaf->win[0].v_offs = afStats_cfg->mainWin.hw_afCfg_win_y;
    rawaf->win[0].h_size = afStats_cfg->mainWin.hw_afCfg_win_width;
    rawaf->win[0].v_size = afStats_cfg->mainWin.hw_afCfg_win_height;
    rawaf->win[1].h_offs = afStats_cfg->subWin.hw_afCfg_win_x;
    rawaf->win[1].v_offs = afStats_cfg->subWin.hw_afCfg_win_y;
    rawaf->win[1].h_size = afStats_cfg->subWin.hw_afCfg_win_width;
    rawaf->win[1].v_size = afStats_cfg->subWin.hw_afCfg_win_height;
    rawaf->bls_en = afStats_cfg->blc.hw_afCfg_blc_en;
    rawaf->bls_offset = afStats_cfg->blc.hw_afCfg_ob_val;
    rawaf->aehgl_en = afStats_cfg->mainWin.hw_afCfg_lumaStats_en;
    rawaf->ae_mode = 0;
    rawaf->ae_config_use = 1;
    rawaf->vldg_sel = 0;
    rawaf->y_mode = 0;
    rawaf->num_afm_win = 2;

    switch (rawaf->avg_ds_mode) {
        case afStats_ds_2x2_mode:
            rawaf->avg_ds_en = 0;
            rawaf->v_dnscl_mode = 2;
            break;
        case afStats_ds_4x4_mode:
            rawaf->avg_ds_en = 1;
            rawaf->avg_ds_mode = 0;
            break;
        case afStats_ds_8x8_mode:
            rawaf->avg_ds_en = 1;
            rawaf->avg_ds_mode = 1;
            break;
        default:
            rawaf->avg_ds_en = 0;
            rawaf->v_dnscl_mode = 1;
            break;
    }

    rawaf->gamma_en = afStats_cfg->gamma.hw_afCfg_gamma_en;
    for (i = 0; i < 17; i++)
        rawaf->gamma_y[i] = afStats_cfg->gamma.hw_afCfg_gamma_val[i] * 0x3FF;
    rawaf->gaus_en = afStats_cfg->preFilt.hw_afCfg_gaus_en;
    for (i = 0; i < 9; i++)
        rawaf->gaus_coe[i] = afStats_cfg->preFilt.hw_afCfg_gaus_coeff[i] * 128;
    rawaf->highlit_thresh = afStats_cfg->hw_afCfg_hLumaCnt_thred * 0x3FF;
    rawaf->ldg_en = afStats_cfg->hw_afCfg_ldg_en;

    rawaf->hiir_en = afStats_cfg->hw_afCfg_hFilt_en;
    rawaf->hiir_left_border_mode = afStats_cfg->hw_afCfg_hFiltLnBnd_mode;
    for (i = 0; i < 6; i++)
        rawaf->h1iir1_coe[i] = afStats_cfg->hFilt1.hw_afCfg_iirStep1_coeff[i];
    for (i = 0; i < 6; i++)
        rawaf->h1iir2_coe[i] = afStats_cfg->hFilt1.hw_afCfg_iirStep2_coeff[i];
    switch (afStats_cfg->hFilt1.hw_afCfg_fvFmt_mode) {
        case afStats_outPeak_sumLinear_mode:
            rawaf->h1_fv_mode = 1;
            rawaf->h1_acc_mode = 1;
            break;
        case afStats_outNorm_sumLinear_mode:
            rawaf->h1_fv_mode = 1;
            rawaf->h1_acc_mode = 0;
            break;
        case afStats_outPeak_sumSqu_mode:
            rawaf->h1_fv_mode = 0;
            rawaf->h1_acc_mode = 1;
            break;
        default:
            rawaf->h1_fv_mode = 0;
            rawaf->h1_acc_mode = 0;
            break;
    }
    rawaf->h1iir_shift_wina = afStats_cfg->hFilt1.hw_afCfg_accMainWin_shift;
    rawaf->h1iir_shift_winb = afStats_cfg->hFilt1.hw_afCfg_accSubWin_shift;

    for (i = 0; i < 6; i++)
        rawaf->h2iir1_coe[i] = afStats_cfg->hFilt2.hw_afCfg_iirStep1_coeff[i];
    for (i = 0; i < 6; i++)
        rawaf->h2iir2_coe[i] = afStats_cfg->hFilt2.hw_afCfg_iirStep2_coeff[i];
    switch (afStats_cfg->hFilt2.hw_afCfg_fvFmt_mode) {
        case afStats_outPeak_sumLinear_mode:
            rawaf->h2_fv_mode = 1;
            rawaf->h2_acc_mode = 1;
            break;
        case afStats_outNorm_sumLinear_mode:
            rawaf->h2_fv_mode = 1;
            rawaf->h2_acc_mode = 0;
            break;
        case afStats_outPeak_sumSqu_mode:
            rawaf->h2_fv_mode = 0;
            rawaf->h2_acc_mode = 1;
            break;
        default:
            rawaf->h2_fv_mode = 0;
            rawaf->h2_acc_mode = 0;
            break;
    }
    rawaf->h2iir_shift_wina = afStats_cfg->hFilt2.hw_afCfg_accMainWin_shift;
    rawaf->h2iir_shift_winb = afStats_cfg->hFilt2.hw_afCfg_accSubWin_shift;

    rawaf->h_fv_thresh = afStats_cfg->hFilt_coring.hw_afCfg_coring_thred * 0xFFF;
    rawaf->h_fv_slope  = afStats_cfg->hFilt_coring.hw_afCfg_slope_val * 0x100;
    rawaf->h_fv_limit = afStats_cfg->hFilt_coring.hw_afCfg_coring_maxLimit * 0x3FF;
    rawaf->curve_h[0].ldg_lumth = afStats_cfg->hFilt_ldg.hw_afCfg_lumaL_thred * 0xFF;
    rawaf->curve_h[0].ldg_gain  = afStats_cfg->hFilt_ldg.hw_afCfg_gainL_val * 0xFF;
    rawaf->curve_h[0].ldg_gslp  = afStats_cfg->hFilt_ldg.hw_afCfg_slopeL_val * 0x100;
    rawaf->curve_h[1].ldg_lumth = afStats_cfg->hFilt_ldg.hw_afCfg_lumaR_thred * 0xFF;
    rawaf->curve_h[1].ldg_gain  = afStats_cfg->hFilt_ldg.hw_afCfg_gainR_val * 0xFF;
    rawaf->curve_h[1].ldg_gslp  = afStats_cfg->hFilt_ldg.hw_afCfg_slopeR_val * 0x100;
    rawaf->hldg_dilate_num = afStats_cfg->hFilt_ldg.hw_afCfg_maxOptBndL_len;

    rawaf->viir_en = afStats_cfg->hw_afCfg_vFilt_en;
    for (i = 0; i < 3; i++)
        rawaf->v1iir_coe[i] = afStats_cfg->vFilt1.hw_afCfg_iirStep1_coeff[i];
    for (i = 0; i < 3; i++)
        rawaf->v1fir_coe[i] = afStats_cfg->vFilt1.hw_afCfg_firStep2_coeff[i];
    switch (afStats_cfg->vFilt1.hw_afCfg_fvFmt_mode) {
        case afStats_outPeak_sumLinear_mode:
            rawaf->v1_fv_mode = 1;
            rawaf->v1_acc_mode = 1;
            break;
        case afStats_outNorm_sumLinear_mode:
            rawaf->v1_fv_mode = 1;
            rawaf->v1_acc_mode = 0;
            break;
        case afStats_outPeak_sumSqu_mode:
            rawaf->v1_fv_mode = 0;
            rawaf->v1_acc_mode = 1;
            break;
        default:
            rawaf->v1_fv_mode = 0;
            rawaf->v1_acc_mode = 0;
            break;
    }
    rawaf->v1iir_shift_wina = afStats_cfg->vFilt1.hw_afCfg_accMainWin_shift;
    rawaf->v1iir_shift_winb = afStats_cfg->vFilt1.hw_afCfg_accSubWin_shift;

    for (i = 0; i < 3; i++)
        rawaf->v2iir_coe[i] = afStats_cfg->vFilt2.hw_afCfg_iirStep1_coeff[i];
    for (i = 0; i < 3; i++)
        rawaf->v2fir_coe[i] = afStats_cfg->vFilt2.hw_afCfg_firStep2_coeff[i];
    switch (afStats_cfg->vFilt2.hw_afCfg_fvFmt_mode) {
        case afStats_outPeak_sumLinear_mode:
            rawaf->v2_fv_mode = 1;
            rawaf->v2_acc_mode = 1;
            break;
        case afStats_outNorm_sumLinear_mode:
            rawaf->v2_fv_mode = 1;
            rawaf->v2_acc_mode = 0;
            break;
        case afStats_outPeak_sumSqu_mode:
            rawaf->v2_fv_mode = 0;
            rawaf->v2_acc_mode = 1;
            break;
        default:
            rawaf->v2_fv_mode = 0;
            rawaf->v2_acc_mode = 0;
            break;
    }
    rawaf->v2iir_shift_wina = afStats_cfg->vFilt2.hw_afCfg_accMainWin_shift;
    rawaf->v2iir_shift_winb = afStats_cfg->vFilt2.hw_afCfg_accSubWin_shift;

    rawaf->v_fv_thresh = afStats_cfg->vFilt_coring.hw_afCfg_coring_thred * 0xFFF;
    rawaf->v_fv_slope  = afStats_cfg->vFilt_coring.hw_afCfg_slope_val * 0x100;
    rawaf->v_fv_limit = afStats_cfg->vFilt_coring.hw_afCfg_coring_maxLimit * 0x3FF;
    rawaf->curve_v[0].ldg_lumth = afStats_cfg->vFilt_ldg.hw_afCfg_lumaL_thred * 0xFF;
    rawaf->curve_v[0].ldg_gain  = afStats_cfg->vFilt_ldg.hw_afCfg_gainL_val * 0xFF;
    rawaf->curve_v[0].ldg_gslp  = afStats_cfg->vFilt_ldg.hw_afCfg_slopeL_val * 0x100;
    rawaf->curve_v[1].ldg_lumth = afStats_cfg->vFilt_ldg.hw_afCfg_lumaR_thred * 0xFF;
    rawaf->curve_v[1].ldg_gain  = afStats_cfg->vFilt_ldg.hw_afCfg_gainR_val * 0xFF;
    rawaf->curve_v[1].ldg_gslp  = afStats_cfg->vFilt_ldg.hw_afCfg_slopeR_val * 0x100;
}
#else
void Isp39Params::convertAiqAfToIsp39Params(struct isp39_isp_params_cfg& isp_cfg,
        const rk_aiq_isp_af_v33_t& af_data,
        bool af_cfg_udpate)
{
    int i;

    if (!af_cfg_udpate) return;

    if (af_data.af_en) isp_cfg.module_ens |= ISP2X_MODULE_RAWAF;
    isp_cfg.module_en_update |= ISP2X_MODULE_RAWAF;
    isp_cfg.module_cfg_update |= ISP2X_MODULE_RAWAF;

    isp_cfg.meas.rawaf.rawaf_sel      = af_data.rawaf_sel;
    isp_cfg.meas.rawaf.gamma_en       = af_data.gamma_en;
    isp_cfg.meas.rawaf.gaus_en        = af_data.gaus_en;
    isp_cfg.meas.rawaf.hiir_en        = af_data.hiir_en;
    isp_cfg.meas.rawaf.viir_en        = af_data.viir_en;
    isp_cfg.meas.rawaf.v1_fv_mode     = af_data.v1_fv_outmode;
    isp_cfg.meas.rawaf.v2_fv_mode     = af_data.v2_fv_outmode;
    isp_cfg.meas.rawaf.h1_fv_mode     = af_data.h1_fv_outmode;
    isp_cfg.meas.rawaf.h2_fv_mode     = af_data.h2_fv_outmode;
    isp_cfg.meas.rawaf.ldg_en         = af_data.ldg_en;
    isp_cfg.meas.rawaf.y_mode         = af_data.y_mode;
    isp_cfg.meas.rawaf.vldg_sel       = af_data.vldg_sel;
    isp_cfg.meas.rawaf.v_dnscl_mode   = af_data.v_dnscl_mode;
    isp_cfg.meas.rawaf.from_ynr       = af_data.from_ynr;
    isp_cfg.meas.rawaf.ae_mode        = 0;
    isp_cfg.meas.rawaf.ae_config_use  = 1;
    isp_cfg.meas.rawaf.aehgl_en       = af_data.aehgl_en;
    isp_cfg.meas.rawaf.avg_ds_en      = af_data.avg_ds_en;
    isp_cfg.meas.rawaf.avg_ds_mode    = af_data.avg_ds_mode;
    isp_cfg.meas.rawaf.h1_acc_mode    = af_data.h1_acc_mode;
    isp_cfg.meas.rawaf.h2_acc_mode    = af_data.h2_acc_mode;
    isp_cfg.meas.rawaf.v1_acc_mode    = af_data.v1_acc_mode;
    isp_cfg.meas.rawaf.v2_acc_mode    = af_data.v2_acc_mode;

    isp_cfg.meas.rawaf.bnr2af_sel     = af_data.from_bnr;
    isp_cfg.meas.rawaf.tnrin_shift    = af_data.bnrin_shift;

    isp_cfg.meas.rawaf.num_afm_win   = af_data.window_num;
    isp_cfg.meas.rawaf.win[0].h_offs = af_data.wina_h_offs;
    isp_cfg.meas.rawaf.win[0].v_offs = af_data.wina_v_offs;
    isp_cfg.meas.rawaf.win[0].h_size = af_data.wina_h_size;
    isp_cfg.meas.rawaf.win[0].v_size = af_data.wina_v_size;
    isp_cfg.meas.rawaf.win[1].h_offs = af_data.winb_h_offs;
    isp_cfg.meas.rawaf.win[1].v_offs = af_data.winb_v_offs;
    isp_cfg.meas.rawaf.win[1].h_size = af_data.winb_h_size;
    isp_cfg.meas.rawaf.win[1].v_size = af_data.winb_v_size;

    isp_cfg.meas.rawaf.bls_en          = af_data.bls_en;
    isp_cfg.meas.rawaf.bls_offset      = af_data.bls_offset;
    isp_cfg.meas.rawaf.hldg_dilate_num = af_data.hldg_dilate_num;
    isp_cfg.meas.rawaf.hiir_left_border_mode = af_data.hiir_left_border_mode;

    isp_cfg.meas.rawaf.h1iir_shift_wina = af_data.wina_h1_shift;
    isp_cfg.meas.rawaf.h2iir_shift_wina = af_data.wina_h2_shift;
    isp_cfg.meas.rawaf.v1iir_shift_wina = af_data.wina_v1_shift;
    isp_cfg.meas.rawaf.v2iir_shift_wina = af_data.wina_v2_shift;
    isp_cfg.meas.rawaf.h1iir_shift_winb = af_data.winb_h1_shift;
    isp_cfg.meas.rawaf.h2iir_shift_winb = af_data.winb_h2_shift;
    isp_cfg.meas.rawaf.v1iir_shift_winb = af_data.winb_v1_shift;
    isp_cfg.meas.rawaf.v2iir_shift_winb = af_data.winb_v2_shift;

    for (i = 0; i < ISP32_RAWAF_GAUS_COE_NUM; i++) {
        isp_cfg.meas.rawaf.gaus_coe[i] = af_data.gaus_coe[i];
    }

    memcpy(isp_cfg.meas.rawaf.gamma_y, af_data.gamma_y,
           ISP32_RAWAF_GAMMA_NUM * sizeof(unsigned short));

    for (i = 0; i < ISP32_RAWAF_CURVE_NUM; i++) {
        isp_cfg.meas.rawaf.curve_h[i].ldg_lumth = af_data.h_ldg_lumth[i];
        isp_cfg.meas.rawaf.curve_h[i].ldg_gain  = af_data.h_ldg_gain[i];
        isp_cfg.meas.rawaf.curve_h[i].ldg_gslp  = af_data.h_ldg_gslp[i];
        isp_cfg.meas.rawaf.curve_v[i].ldg_lumth = af_data.v_ldg_lumth[i];
        isp_cfg.meas.rawaf.curve_v[i].ldg_gain  = af_data.v_ldg_gain[i];
        isp_cfg.meas.rawaf.curve_v[i].ldg_gslp  = af_data.v_ldg_gslp[i];
    }

    for (i = 0; i < ISP32_RAWAF_HIIR_COE_NUM; i++) {
        isp_cfg.meas.rawaf.h1iir1_coe[i] = af_data.h1_iir1_coe[i];
        isp_cfg.meas.rawaf.h1iir2_coe[i] = af_data.h1_iir2_coe[i];
        isp_cfg.meas.rawaf.h2iir1_coe[i] = af_data.h2_iir1_coe[i];
        isp_cfg.meas.rawaf.h2iir2_coe[i] = af_data.h2_iir2_coe[i];
    }
    for (i = 0; i < ISP32_RAWAF_VIIR_COE_NUM; i++) {
        isp_cfg.meas.rawaf.v1iir_coe[i] = af_data.v1_iir_coe[i];
        isp_cfg.meas.rawaf.v2iir_coe[i] = af_data.v2_iir_coe[i];
    }
    for (i = 0; i < ISP32_RAWAF_VFIR_COE_NUM; i++) {
        isp_cfg.meas.rawaf.v1fir_coe[i] = af_data.v1_fir_coe[i];
        isp_cfg.meas.rawaf.v2fir_coe[i] = af_data.v2_fir_coe[i];
    }
    isp_cfg.meas.rawaf.highlit_thresh = af_data.highlit_thresh;

    isp_cfg.meas.rawaf.h_fv_thresh = af_data.h_fv_thresh;
    isp_cfg.meas.rawaf.h_fv_limit  = af_data.h_fv_limit;
    isp_cfg.meas.rawaf.h_fv_slope  = af_data.h_fv_slope;
    isp_cfg.meas.rawaf.v_fv_thresh = af_data.v_fv_thresh;
    isp_cfg.meas.rawaf.v_fv_limit  = af_data.v_fv_limit;
    isp_cfg.meas.rawaf.v_fv_slope  = af_data.v_fv_slope;
    mLatestMeasCfg.rawaf = isp_cfg.meas.rawaf;
}
#endif
#endif

#if RKAIQ_HAVE_CAC_V11
void Isp39Params::convertAiqCacToIsp39Params(struct isp39_isp_params_cfg& isp_cfg,
        const rk_aiq_isp_cac_v32_t& cac_cfg) {

    LOGD_ACAC("convert CAC params enable %d", cac_cfg.enable);

    if (cac_cfg.enable) {
        isp_cfg.module_en_update |= ISP3X_MODULE_CAC;
        isp_cfg.module_ens |= ISP3X_MODULE_CAC;
        isp_cfg.module_cfg_update |= ISP3X_MODULE_CAC;
    } else {
        isp_cfg.module_en_update |= (ISP3X_MODULE_CAC);
        isp_cfg.module_ens &= ~(ISP3X_MODULE_CAC);
        isp_cfg.module_cfg_update &= ~(ISP3X_MODULE_CAC);
    }

    struct isp32_cac_cfg* cfg = &isp_cfg.others.cac_cfg;
    memcpy(cfg, &cac_cfg.cfg, sizeof(*cfg));

#if 1
    LOGD_ACAC("Dump CAC config: ");
    LOGD_ACAC("current enable: %d",    cac_cfg.enable);
    LOGD_ACAC("by en: %d",             cfg->bypass_en);
    LOGD_ACAC("center en: %d",         cfg->center_en);
    LOGD_ACAC("center x: %d",          cfg->center_width);
    LOGD_ACAC("center y: %d",          cfg->center_height);
    LOGD_ACAC("psf shift bits: %d",    cfg->psf_sft_bit);
    LOGD_ACAC("psf cfg num: %d",       cfg->cfg_num);
    LOGD_ACAC("psf buf fd: %d",        cfg->buf_fd);
    LOGD_ACAC("psf hwsize: %d",        cfg->hsize);
    LOGD_ACAC("psf vsize: %d",          cfg->vsize);
    for (int i = 0; i < RKCAC_STRENGTH_TABLE_LEN; i++) {
        LOGD_ACAC("strength %d: %d", i, cfg->strength[i]);
    }
    LOGD_ACAC("clip_g_mode : %d",     cfg->clip_g_mode);
    LOGD_ACAC("edge_detect_en : %d",  cfg->edge_detect_en);
    LOGD_ACAC("neg_clip0_en : %d",    cfg->neg_clip0_en);
    LOGD_ACAC("flat_thed_b : %d",     cfg->flat_thed_b);
    LOGD_ACAC("flat_thed_r : %d",     cfg->flat_thed_r);
    LOGD_ACAC("offset_b : %d",        cfg->offset_b);
    LOGD_ACAC("offset_r : %d",        cfg->offset_r);
    LOGD_ACAC("expo_thed_b : %d",     cfg->expo_thed_b);
    LOGD_ACAC("expo_thed_r : %d",     cfg->expo_thed_r);
    LOGD_ACAC("expo_adj_b : %d",      cfg->expo_adj_b);
    LOGD_ACAC("expo_adj_r : %d",      cfg->expo_adj_r);
#endif
}
#endif

#if (RKAIQ_HAVE_BAYERTNR_V30)
#if USE_NEWSTRUCT
void Isp39Params::convertAiqBtnrToIsp39Params(struct isp39_isp_params_cfg& isp_cfg,
        rk_aiq_isp_btnr_params_t* btnr_attr)
{
    if (btnr_attr->en) {
        // bayer3dnr enable  bayer2dnr must enable at the same time
        isp_cfg.module_ens |= ISP3X_MODULE_BAY3D;
        isp_cfg.module_ens |= ISP3X_MODULE_BAYNR;
        isp_cfg.module_en_update |= ISP3X_MODULE_BAY3D;
        isp_cfg.module_cfg_update |= ISP3X_MODULE_BAY3D;
        isp_cfg.module_en_update |= ISP3X_MODULE_BAYNR;
    } else {
        // tnr can't open/close in runtime.
        isp_cfg.module_ens |= ISP3X_MODULE_BAY3D;
        return;
    }

    mBtnrInfo.bypass = btnr_attr->bypass;
    rk_aiq_btnr40_params_cvt(&btnr_attr->result, &isp_cfg, &mCommonCvtInfo, &mBtnrInfo);
}
#else
void Isp39Params::convertAiqTnrToIsp39Params(struct isp39_isp_params_cfg& isp_cfg,
        rk_aiq_isp_tnr_v39_t& tnr) {
    LOGD_ANR("%s:%d enter! enable:%d \n", __FUNCTION__, __LINE__, tnr.bypass_en);

    if (tnr.bypass_en) {
        isp_cfg.module_ens |= ISP3X_MODULE_BAY3D;
    } else {
        isp_cfg.module_ens |= ISP3X_MODULE_BAY3D;
    }

    isp_cfg.module_en_update |= ISP3X_MODULE_BAY3D;
    isp_cfg.module_cfg_update |= ISP3X_MODULE_BAY3D;

    struct isp39_bay3d_cfg* pBtnr = &isp_cfg.others.bay3d_cfg;

    pBtnr->bypass_en = tnr.bypass_en;
    pBtnr->iirsparse_en = tnr.iirsparse_en;

    pBtnr->transf_bypass_en = tnr.transf_bypass_en;
    pBtnr->sigma_curve_double_en = tnr.sigma_curve_double_en;
    pBtnr->cur_spnr_bypass_en = tnr.cur_spnr_bypass_en;
    pBtnr->cur_spnr_sigma_idxfilt_bypass_en = tnr.cur_spnr_sigma_idxfilt_bypass_en;
    pBtnr->cur_spnr_sigma_curve_double_en = tnr.cur_spnr_sigma_curve_double_en;
    pBtnr->pre_spnr_bypass_en = tnr.pre_spnr_bypass_en;
    pBtnr->pre_spnr_sigma_idxfilt_bypass_en = tnr.pre_spnr_sigma_idxfilt_bypass_en;
    pBtnr->pre_spnr_sigma_curve_double_en = tnr.pre_spnr_sigma_curve_double_en;
    pBtnr->lpf_hi_bypass_en = tnr.lpf_hi_bypass_en;
    pBtnr->lo_diff_vfilt_bypass_en = tnr.lo_diff_vfilt_bypass_en;
    pBtnr->lpf_lo_bypass_en = tnr.lpf_lo_bypass_en;
    pBtnr->lo_wgt_hfilt_en = tnr.lo_wgt_hfilt_en;
    pBtnr->lo_diff_hfilt_en = tnr.lo_diff_hfilt_en;
    pBtnr->sig_hfilt_en = tnr.sig_hfilt_en;
    //pBtnr->gkalman_en = tnr.gkalman_en;
    pBtnr->spnr_pre_sigma_use_en = tnr.spnr_pre_sigma_use_en;
    pBtnr->lo_detection_mode = tnr.lo_detection_mode;
    pBtnr->md_wgt_out_en = tnr.md_wgt_out_en;
    pBtnr->md_wgt_out_en = tnr.md_wgt_out_en;
    pBtnr->curdbg_out_en = tnr.curdbg_out_en;
    pBtnr->noisebal_mode = tnr.noisebal_mode;
    pBtnr->md_bypass_en = tnr.md_bypass_en;
    pBtnr->iirspnr_out_en = tnr.iirspnr_out_en;
    pBtnr->lomdwgt_dbg_en = tnr.lomdwgt_dbg_en;


    pBtnr->transf_mode = tnr.transf_mode;
    pBtnr->wgt_cal_mode = tnr.wgt_cal_mode;
    pBtnr->mge_wgt_ds_mode = tnr.mge_wgt_ds_mode;
    pBtnr->kalman_wgt_ds_mode = tnr.kalman_wgt_ds_mode;
    pBtnr->mge_wgt_hdr_sht_thred = tnr.mge_wgt_hdr_sht_thred;
    pBtnr->sigma_calc_mge_wgt_hdr_sht_thred = tnr.sigma_calc_mge_wgt_hdr_sht_thred;

    pBtnr->transf_mode_offset = tnr.transf_mode_offset;
    pBtnr->transf_mode_scale = tnr.transf_mode_scale;
    pBtnr->itransf_mode_offset = tnr.itransf_mode_offset;

    pBtnr->transf_data_max_limit = tnr.transf_data_max_limit;

    pBtnr->cur_spnr_sigma_hdr_sht_scale = tnr.cur_spnr_sigma_hdr_sht_scale;
    pBtnr->cur_spnr_sigma_scale = tnr.cur_spnr_sigma_scale;

    for (int i = 0; i < 16; i++) {
        pBtnr->cur_spnr_luma_sigma_x[i] = tnr.cur_spnr_luma_sigma_x[i];
        pBtnr->cur_spnr_luma_sigma_y[i] = tnr.cur_spnr_luma_sigma_y[i];

        pBtnr->pre_spnr_luma_sigma_x[i] = tnr.pre_spnr_luma_sigma_x[i];
        pBtnr->pre_spnr_luma_sigma_y[i] = tnr.pre_spnr_luma_sigma_y[i];
    }

    pBtnr->cur_spnr_sigma_rgain_offset = tnr.cur_spnr_sigma_rgain_offset;
    pBtnr->cur_spnr_sigma_bgain_offset = tnr.cur_spnr_sigma_bgain_offset;

    pBtnr->cur_spnr_sigma_hdr_sht_offset = tnr.cur_spnr_sigma_hdr_sht_offset;
    pBtnr->cur_spnr_sigma_offset = tnr.cur_spnr_sigma_offset;

    pBtnr->cur_spnr_pix_diff_max_limit = tnr.cur_spnr_pix_diff_max_limit;
    pBtnr->cur_spnr_wgt_cal_offset = tnr.cur_spnr_wgt_cal_offset;

    pBtnr->cur_spnr_wgt = tnr.cur_spnr_wgt;
    pBtnr->pre_spnr_wgt = tnr.pre_spnr_wgt;


    for (int i = 0; i < 9; i++) {
        pBtnr->cur_spnr_space_rb_wgt[i] = tnr.cur_spnr_space_rb_wgt[i];
        pBtnr->cur_spnr_space_gg_wgt[i] = tnr.cur_spnr_space_gg_wgt[i];

        pBtnr->pre_spnr_space_rb_wgt[i] = tnr.pre_spnr_space_rb_wgt[i];
        pBtnr->pre_spnr_space_gg_wgt[i] = tnr.pre_spnr_space_gg_wgt[i];

        pBtnr->tnr_lpf_hi_coeff[i] = tnr.tnr_lpf_hi_coeff[i];
        pBtnr->tnr_lpf_lo_coeff[i] = tnr.tnr_lpf_lo_coeff[i];
    }

    pBtnr->pre_spnr_sigma_hdr_sht_scale = tnr.pre_spnr_sigma_hdr_sht_scale;
    pBtnr->pre_spnr_sigma_scale = tnr.pre_spnr_sigma_scale;

    pBtnr->pre_spnr_sigma_rgain_offset = tnr.pre_spnr_sigma_rgain_offset;
    pBtnr->pre_spnr_sigma_bgain_offset = tnr.pre_spnr_sigma_bgain_offset;

    pBtnr->pre_spnr_sigma_hdr_sht_offset = tnr.pre_spnr_sigma_hdr_sht_offset;
    pBtnr->pre_spnr_sigma_offset = tnr.pre_spnr_sigma_offset;

    pBtnr->pre_spnr_pix_diff_max_limit = tnr.pre_spnr_pix_diff_max_limit;
    pBtnr->pre_spnr_wgt_cal_offset = tnr.pre_spnr_wgt_cal_offset;

    pBtnr->cur_spnr_wgt_cal_scale = tnr.cur_spnr_wgt_cal_scale;
    pBtnr->pre_spnr_wgt_cal_scale = tnr.pre_spnr_wgt_cal_scale;

    for (int i = 0; i < 20; i++) {
        pBtnr->tnr_luma_sigma_x[i] = tnr.tnr_luma_sigma_x[i];
        pBtnr->tnr_luma_sigma_y[i] = tnr.tnr_luma_sigma_y[i];
    }

    pBtnr->tnr_wgt_filt_coeff0 = tnr.tnr_wgt_filt_coeff0;
    pBtnr->tnr_wgt_filt_coeff1 = tnr.tnr_wgt_filt_coeff1;
    pBtnr->tnr_wgt_filt_coeff2 = tnr.tnr_wgt_filt_coeff2;

    pBtnr->tnr_sigma_scale = tnr.tnr_sigma_scale;
    pBtnr->tnr_sigma_hdr_sht_scale = tnr.tnr_sigma_hdr_sht_scale;

    pBtnr->tnr_sig_vfilt_wgt = tnr.tnr_sig_vfilt_wgt;
    pBtnr->tnr_lo_diff_vfilt_wgt = tnr.tnr_lo_diff_vfilt_wgt;
    pBtnr->tnr_lo_wgt_vfilt_wgt = tnr.tnr_lo_wgt_vfilt_wgt;
    pBtnr->tnr_sig_first_line_scale = tnr.tnr_sig_first_line_scale;
    pBtnr->tnr_lo_diff_first_line_scale = tnr.tnr_lo_diff_first_line_scale;

    pBtnr->tnr_lo_wgt_cal_offset = tnr.tnr_lo_wgt_cal_offset;
    pBtnr->tnr_lo_wgt_cal_scale = tnr.tnr_lo_wgt_cal_scale;

    pBtnr->tnr_low_wgt_cal_max_limit = tnr.tnr_low_wgt_cal_max_limit;
    pBtnr->tnr_mode0_base_ratio = tnr.tnr_mode0_base_ratio;


    pBtnr->tnr_lo_diff_wgt_cal_offset = tnr.tnr_lo_diff_wgt_cal_offset;
    pBtnr->tnr_lo_diff_wgt_cal_scale = tnr.tnr_lo_diff_wgt_cal_scale;

    pBtnr->tnr_lo_mge_pre_wgt_offset = tnr.tnr_lo_mge_pre_wgt_offset;
    pBtnr->tnr_lo_mge_pre_wgt_scale = tnr.tnr_lo_mge_pre_wgt_scale;

    pBtnr->tnr_mode0_lo_wgt_scale = tnr.tnr_mode0_lo_wgt_scale;
    pBtnr->tnr_mode0_lo_wgt_hdr_sht_scale = tnr.tnr_mode0_lo_wgt_hdr_sht_scale;


    pBtnr->tnr_mode1_lo_wgt_scale = tnr.tnr_mode1_lo_wgt_scale;
    pBtnr->tnr_mode1_lo_wgt_hdr_sht_scale = tnr.tnr_mode1_lo_wgt_hdr_sht_scale;

    pBtnr->tnr_mode1_wgt_scale = tnr.tnr_mode1_wgt_scale;
    pBtnr->tnr_mode1_wgt_hdr_sht_scale = tnr.tnr_mode1_wgt_hdr_sht_scale;

    pBtnr->tnr_mode1_lo_wgt_offset = tnr.tnr_mode1_lo_wgt_offset;
    pBtnr->tnr_mode1_lo_wgt_hdr_sht_offset = tnr.tnr_mode1_lo_wgt_hdr_sht_offset;

    pBtnr->tnr_auto_sigma_count_wgt_thred = tnr.tnr_auto_sigma_count_wgt_thred;
    pBtnr->tnr_mode1_wgt_min_limit = tnr.tnr_mode1_wgt_min_limit;
    pBtnr->tnr_mode1_wgt_offset = tnr.tnr_mode1_wgt_offset;

    pBtnr->tnr_out_sigma_sq = tnr.tnr_out_sigma_sq;

    pBtnr->tnr_lo_wgt_clip_min_limit = tnr.tnr_lo_wgt_clip_min_limit;
    pBtnr->tnr_lo_wgt_clip_hdr_sht_min_limit = tnr.tnr_lo_wgt_clip_hdr_sht_min_limit;

    pBtnr->tnr_lo_wgt_clip_max_limit = tnr.tnr_lo_wgt_clip_max_limit;
    pBtnr->tnr_lo_wgt_clip_hdr_sht_max_limit = tnr.tnr_lo_wgt_clip_hdr_sht_max_limit;

    pBtnr->tnr_hi_wgt_clip_min_limit = tnr.tnr_hi_wgt_clip_min_limit;
    pBtnr->tnr_hi_wgt_clip_hdr_sht_min_limit = tnr.tnr_hi_wgt_clip_hdr_sht_min_limit;


    pBtnr->tnr_hi_wgt_clip_max_limit = tnr.tnr_hi_wgt_clip_max_limit;
    pBtnr->tnr_hi_wgt_clip_hdr_sht_max_limit = tnr.tnr_hi_wgt_clip_hdr_sht_max_limit;

    pBtnr->tnr_cur_spnr_hi_wgt_min_limit = tnr.tnr_cur_spnr_hi_wgt_min_limit;
    pBtnr->tnr_pre_spnr_hi_wgt_min_limit = tnr.tnr_pre_spnr_hi_wgt_min_limit;

    pBtnr->tnr_pix_max = tnr.tnr_pix_max;
    pBtnr->tnr_auto_sigma_count_th = tnr.tnr_auto_sigma_count_th;


    pBtnr->lowgt_ctrl = tnr.hw_bay3d_lowgt_ctrl;
    pBtnr->lowgt_offint = tnr.hw_bay3d_lowgt_offinit;

    pBtnr->tnr_motion_nr_strg = tnr.tnr_motion_nr_strg;
    pBtnr->tnr_gain_max = tnr.tnr_gain_max;

    LOGD_ANR("%s:%d exit!\n", __FUNCTION__, __LINE__);
}
#endif
#endif

#if RKAIQ_HAVE_YNR_V24
#if USE_NEWSTRUCT
void Isp39Params::convertAiqYnrToIsp39Params(struct isp39_isp_params_cfg& isp_cfg,
        rk_aiq_isp_ynr_params_t *ynr_attr) {

    isp_cfg.module_ens |= ISP3X_MODULE_YNR;
    isp_cfg.module_en_update |= ISP3X_MODULE_YNR;
    isp_cfg.module_cfg_update |= ISP3X_MODULE_YNR;

    rk_aiq_ynr34_params_cvt(&ynr_attr->result, &isp_cfg, &mCommonCvtInfo);
}
#else
void Isp39Params::convertAiqYnrToIsp39Params(struct isp39_isp_params_cfg& isp_cfg,
        rk_aiq_isp_ynr_v39_t& ynr) {
    isp_cfg.module_en_update |= ISP3X_MODULE_YNR;
    isp_cfg.module_ens |= ISP3X_MODULE_YNR;
    isp_cfg.module_cfg_update |= ISP3X_MODULE_YNR;

    struct isp39_ynr_cfg* pYnr = &isp_cfg.others.ynr_cfg;

    pYnr->lospnr_bypass    = ynr.lospnr_bypass;
    pYnr->hispnr_bypass    = ynr.hispnr_bypass;
    pYnr->exgain_bypass    = ynr.exgain_bypass;
    pYnr->global_set_gain  = ynr.global_set_gain;
    pYnr->gain_merge_alpha = ynr.gain_merge_alpha;
    pYnr->rnr_en           = ynr.rnr_en;

    pYnr->rnr_max_radius   = ynr.rnr_max_radius;
    pYnr->local_gain_scale = ynr.local_gain_scale;

    pYnr->rnr_center_coorh = ynr.rnr_center_coorh;
    pYnr->rnr_center_coorv = ynr.rnr_center_coorv;

    pYnr->ds_filt_soft_thred_scale = ynr.ds_filt_soft_thred_scale;
    pYnr->ds_img_edge_scale        = ynr.ds_img_edge_scale;
    pYnr->ds_filt_wgt_thred_scale  = ynr.ds_filt_wgt_thred_scale;

    pYnr->ds_filt_local_gain_alpha = ynr.ds_filt_local_gain_alpha;
    pYnr->ds_iir_init_wgt_scale    = ynr.ds_iir_init_wgt_scale;
    pYnr->ds_filt_center_wgt       = ynr.ds_filt_center_wgt;

    pYnr->ds_filt_inv_strg = ynr.ds_filt_inv_strg;
    pYnr->lospnr_wgt       = ynr.lospnr_wgt;

    pYnr->lospnr_center_wgt = ynr.lospnr_center_wgt;
    pYnr->lospnr_strg       = ynr.lospnr_strg;

    pYnr->lospnr_dist_vstrg_scale = ynr.lospnr_dist_vstrg_scale;
    pYnr->lospnr_dist_hstrg_scale = ynr.lospnr_dist_hstrg_scale;

    pYnr->pre_filt_coeff0 = ynr.pre_filt_coeff0;
    pYnr->pre_filt_coeff1 = ynr.pre_filt_coeff1;
    pYnr->pre_filt_coeff2 = ynr.pre_filt_coeff2;

    for (int i = 0; i < ISP39_YNR_LOW_GAIN_ADJ_NUM; i++)
        pYnr->lospnr_gain2strg_val[i] = ynr.lospnr_gain2strg_val[i];

    for (int i = 0; i < ISP39_YNR_XY_NUM; i++) pYnr->luma2sima_idx[i] = ynr.luma2sima_idx[i];

    for (int i = 0; i < ISP39_YNR_XY_NUM; i++) pYnr->luma2sima_val[i] = ynr.luma2sima_val[i];

    for (int i = 0; i < ISP39_YNR_XY_NUM; i++) pYnr->radius2strg_val[i] = ynr.radius2strg_val[i];

    pYnr->hispnr_strong_edge = ynr.hispnr_strong_edge;

    pYnr->hispnr_sigma_min_limit  = ynr.hispnr_sigma_min_limit;
    pYnr->hispnr_local_gain_alpha = ynr.hispnr_local_gain_alpha;
    pYnr->hispnr_strg             = ynr.hispnr_strg;

    for (int i = 0; i < ISP39_YNR_NLM_COE_NUM; i++)
        pYnr->hispnr_filt_coeff[i] = ynr.hispnr_filt_coeff[i];

    pYnr->hispnr_filt_wgt_offset = ynr.hispnr_filt_wgt_offset;
    pYnr->hispnr_filt_center_wgt = ynr.hispnr_filt_center_wgt;

    pYnr->hispnr_filt_wgt   = ynr.hispnr_filt_wgt;
    pYnr->hispnr_gain_thred = ynr.hispnr_gain_thred;

    LOGD_ANR("%s:%d exit!\n", __FUNCTION__, __LINE__);
}
#endif
#endif
#if (RKAIQ_HAVE_CNR_V31)
#if USE_NEWSTRUCT
void Isp39Params::convertAiqCnrToIsp39Params(struct isp39_isp_params_cfg& isp_cfg,
        rk_aiq_isp_cnr_params_t *attr) {

    isp_cfg.module_ens |= ISP3X_MODULE_CNR;
    isp_cfg.module_en_update |= ISP3X_MODULE_CNR;
    isp_cfg.module_cfg_update |= ISP3X_MODULE_CNR;

    //struct isp39_cnr_cfg* pCnr = &isp_cfg.others.cnr_cfg;
    rk_aiq_cnr34_params_cvt(&attr->result, &isp_cfg, &mCommonCvtInfo);
}
#else
void Isp39Params::convertAiqUvnrToIsp39Params(struct isp39_isp_params_cfg& isp_cfg,
        rk_aiq_isp_cnr_v39_t& uvnr) {
    LOGD_ANR("%s:%d enter!  enable:%d\n", __FUNCTION__, __LINE__, uvnr.cnr_en);

    if (uvnr.cnr_en) {
        isp_cfg.module_ens |= ISP3X_MODULE_CNR;
    } else {
        isp_cfg.module_ens |= ISP3X_MODULE_CNR;
    }

    isp_cfg.module_en_update |= ISP3X_MODULE_CNR;
    isp_cfg.module_cfg_update |= ISP3X_MODULE_CNR;

// ynr cnr sharp should open together for test
#if 0
    isp_cfg.module_ens |= ISP3X_MODULE_YNR;
    isp_cfg.module_ens |= ISP3X_MODULE_SHARP;
    isp_cfg.module_en_update |= ISP3X_MODULE_YNR;
    isp_cfg.module_cfg_update |= ISP3X_MODULE_YNR;
    isp_cfg.module_en_update |= ISP3X_MODULE_SHARP;
    isp_cfg.module_cfg_update |= ISP3X_MODULE_SHARP;
    struct isp39_ynr_cfg* pYnr = &isp_cfg.others.ynr_cfg;
    pYnr->lospnr_bypass            = 1;
    pYnr->hispnr_bypass            = 1;
    struct isp39_sharp_cfg* pSharp = &isp_cfg.others.sharp_cfg;
    pSharp->bypass = 1;
#endif

    struct isp39_cnr_cfg* pCnr = &isp_cfg.others.cnr_cfg;

    pCnr->exgain_bypass = uvnr.exgain_bypass;
    pCnr->yuv422_mode = uvnr.yuv422_mode;
    pCnr->thumb_mode = uvnr.thumb_mode ;
    pCnr->hiflt_wgt0_mode = uvnr.hiflt_wgt0_mode;
    pCnr->loflt_coeff = uvnr.loflt_coeff;

    pCnr->global_gain = uvnr.global_gain;
    pCnr->global_gain_alpha = uvnr.global_gain_alpha;
    pCnr->local_gain_scale = uvnr.local_gain_scale;

    pCnr->lobfflt_vsigma_uv = uvnr.lobfflt_vsigma_uv;
    pCnr->lobfflt_vsigma_y = uvnr.lobfflt_vsigma_y;

    pCnr->lobfflt_alpha = uvnr.lobfflt_alpha;

    pCnr->thumb_bf_coeff0 = uvnr.thumb_bf_coeff[0];
    pCnr->thumb_bf_coeff1 = uvnr.thumb_bf_coeff[1];
    pCnr->thumb_bf_coeff2 = uvnr.thumb_bf_coeff[2];
    pCnr->thumb_bf_coeff3 = uvnr.thumb_bf_coeff[3];

    pCnr->loflt_uv_gain = uvnr.loflt_uv_gain;
    pCnr->loflt_vsigma = uvnr.loflt_vsigma;
    pCnr->exp_x_shift_bit = uvnr.exp_x_shift_bit;
    pCnr->loflt_wgt_slope = uvnr.loflt_wgt_slope;

    pCnr->loflt_wgt_min_thred = uvnr.loflt_wgt_min_thred;
    pCnr->loflt_wgt_max_limit = uvnr.loflt_wgt_max_limit;

    for (int i = 0; i < 6; i++) {
        pCnr->gaus_flt_coeff[i] = uvnr.gaus_flt_coeff[i];
    }

    pCnr->gaus_flt_alpha = uvnr.gaus_flt_alpha;
    pCnr->hiflt_wgt_min_limit = uvnr.hiflt_wgt_min_limit;
    pCnr->hiflt_alpha = uvnr.hiflt_alpha;
    if (!uvnr.cnr_en) {
        pCnr->hiflt_alpha = 0x000;
        LOGD_ANR("0x0024 cnr bypss, global_alpha:0x%x 0x%x\n", uvnr.hiflt_alpha, pCnr->hiflt_alpha);
    }

    pCnr->hiflt_uv_gain = uvnr.hiflt_uv_gain;
    pCnr->hiflt_global_vsigma = uvnr.hiflt_global_vsigma;
    pCnr->hiflt_cur_wgt = uvnr.hiflt_cur_wgt;

    pCnr->adj_offset = uvnr.adj_offset;
    pCnr->adj_scale = uvnr.adj_scale;


    for (int i = 0; i < 13; i++) {
        pCnr->sgm_ratio[i] = uvnr.sgm_ratio[i];
    }

    pCnr->loflt_global_sgm_ratio = uvnr.loflt_global_sgm_ratio;
    pCnr->loflt_global_sgm_ratio_alpha = uvnr.loflt_global_sgm_ratio_alpha;


    for (int i = 0; i < 13; i++) {
        pCnr->cur_wgt[i] = uvnr.cur_wgt[i];
    }


    for (int i = 0; i < 8; i++) {
        pCnr->hiflt_vsigma_idx[i] = uvnr.hiflt_vsigma_idx[i];
        pCnr->hiflt_vsigma[i] = uvnr.hiflt_vsigma[i];
    }
    LOGD_ANR("%s:%d exit!\n", __FUNCTION__, __LINE__);
}
#endif
#endif

#if RKAIQ_HAVE_DEBAYER_V3
void Isp39Params::convertAiqAdebayerToIsp39Params(struct isp39_isp_params_cfg& isp_cfg,
        rk_aiq_isp_debayer_v39_t & debayer)
{
    if (debayer.updatecfg) {
        if (debayer.enable) {
            isp_cfg.module_ens |= ISP2X_MODULE_DEBAYER;
            isp_cfg.module_en_update |= ISP2X_MODULE_DEBAYER;
            isp_cfg.module_cfg_update |= ISP2X_MODULE_DEBAYER;
        } else {
            isp_cfg.module_ens &= ~ISP2X_MODULE_DEBAYER;
            isp_cfg.module_en_update |= ISP2X_MODULE_DEBAYER;
        }
    } else {
        return;
    }
    /* CONTROL */
    isp_cfg.others.debayer_cfg.filter_c_en = debayer.filter_c_en;
    isp_cfg.others.debayer_cfg.filter_g_en = debayer.filter_g_en;

    /* LUMA_DX */
    isp_cfg.others.debayer_cfg.luma_dx[0] = debayer.luma_dx[0];
    isp_cfg.others.debayer_cfg.luma_dx[1] = debayer.luma_dx[1];
    isp_cfg.others.debayer_cfg.luma_dx[2] = debayer.luma_dx[2];
    isp_cfg.others.debayer_cfg.luma_dx[3] = debayer.luma_dx[3];
    isp_cfg.others.debayer_cfg.luma_dx[4] = debayer.luma_dx[4];
    isp_cfg.others.debayer_cfg.luma_dx[5] = debayer.luma_dx[5];
    isp_cfg.others.debayer_cfg.luma_dx[6] = debayer.luma_dx[6];

    /* G_INTERP */
    isp_cfg.others.debayer_cfg.clip_en = debayer.clip_en;
    isp_cfg.others.debayer_cfg.dist_scale = debayer.dist_scale;
    isp_cfg.others.debayer_cfg.thed0 = debayer.thed0;
    isp_cfg.others.debayer_cfg.thed1 = debayer.thed1;
    isp_cfg.others.debayer_cfg.select_thed = debayer.select_thed;
    isp_cfg.others.debayer_cfg.max_ratio = debayer.max_ratio;

    /* G_INTERP_FILTER1 */
    isp_cfg.others.debayer_cfg.filter1_coe1 = debayer.filter1_coe1;
    isp_cfg.others.debayer_cfg.filter1_coe2 = debayer.filter1_coe2;
    isp_cfg.others.debayer_cfg.filter1_coe3 = debayer.filter1_coe3;
    isp_cfg.others.debayer_cfg.filter1_coe4 = debayer.filter1_coe4;

    /* G_INTERP_FILTER2 */
    isp_cfg.others.debayer_cfg.filter2_coe1 = debayer.filter2_coe1;
    isp_cfg.others.debayer_cfg.filter2_coe2 = debayer.filter2_coe2;
    isp_cfg.others.debayer_cfg.filter2_coe3 = debayer.filter2_coe3;
    isp_cfg.others.debayer_cfg.filter2_coe4 = debayer.filter2_coe4;

    /* G_INTERP_OFFSET_ALPHA */
    isp_cfg.others.debayer_cfg.gain_offset = debayer.gain_offset;
    isp_cfg.others.debayer_cfg.gradloflt_alpha = debayer.gradloflt_alpha;
    isp_cfg.others.debayer_cfg.wgt_alpha = debayer.wgt_alpha;

    /* G_INTERP_DRCT_OFFSET */
    for (int i = 0; i < RK_DEBAYER_V31_LUMA_POINT_NUM; i++) {
        isp_cfg.others.debayer_cfg.drct_offset[i] = debayer.drct_offset[i];
    }

    /* G_FILTER_MODE_OFFSET */
    isp_cfg.others.debayer_cfg.gfilter_mode = debayer.gfilter_mode;
    isp_cfg.others.debayer_cfg.bf_ratio = debayer.bf_ratio;
    isp_cfg.others.debayer_cfg.offset = debayer.offset;

    /* G_FILTER_FILTER */
    isp_cfg.others.debayer_cfg.filter_coe0 = debayer.filter_coe0;
    isp_cfg.others.debayer_cfg.filter_coe1 = debayer.filter_coe1;
    isp_cfg.others.debayer_cfg.filter_coe2 = debayer.filter_coe2;

    /* G_FILTER_VSIGMA */
    for(int i = 0; i < RK_DEBAYER_V31_LUMA_POINT_NUM; i++)
    {
        isp_cfg.others.debayer_cfg.vsigma[i] = debayer.vsigma[i];
    }

    /* C_FILTER_GUIDE_GAUS */
    isp_cfg.others.debayer_cfg.guid_gaus_coe0 = debayer.guid_gaus_coe0;
    isp_cfg.others.debayer_cfg.guid_gaus_coe1 = debayer.guid_gaus_coe1;
    isp_cfg.others.debayer_cfg.guid_gaus_coe2 = debayer.guid_gaus_coe2;

    /* C_FILTER_CE_GAUS */
    isp_cfg.others.debayer_cfg.ce_gaus_coe0 = debayer.ce_gaus_coe0;
    isp_cfg.others.debayer_cfg.ce_gaus_coe1 = debayer.ce_gaus_coe1;
    isp_cfg.others.debayer_cfg.ce_gaus_coe2 = debayer.ce_gaus_coe2;

    /* C_FILTER_ALPHA_GAUS */
    isp_cfg.others.debayer_cfg.alpha_gaus_coe0 = debayer.alpha_gaus_coe0;
    isp_cfg.others.debayer_cfg.alpha_gaus_coe1 = debayer.alpha_gaus_coe1;
    isp_cfg.others.debayer_cfg.alpha_gaus_coe2 = debayer.alpha_gaus_coe2;

    /* C_FILTER_IIR */
    isp_cfg.others.debayer_cfg.ce_sgm = debayer.ce_sgm;
    isp_cfg.others.debayer_cfg.exp_shift = debayer.exp_shift;
    isp_cfg.others.debayer_cfg.wgtslope = debayer.wgtslope;
    isp_cfg.others.debayer_cfg.wet_clip = debayer.wet_clip;
    isp_cfg.others.debayer_cfg.wet_ghost = debayer.wet_ghost;

    /* C_FILTER_BF */
    isp_cfg.others.debayer_cfg.bf_clip = debayer.bf_clip;
    isp_cfg.others.debayer_cfg.bf_curwgt = debayer.bf_curwgt;
    isp_cfg.others.debayer_cfg.bf_sgm = debayer.bf_sgm;

    /* C_FILTER_LOG_OFFSET */
    isp_cfg.others.debayer_cfg.log_en       = debayer.log_en;
    isp_cfg.others.debayer_cfg.loghf_offset = debayer.loghf_offset;
    isp_cfg.others.debayer_cfg.loggd_offset = debayer.loggd_offset;

    /* C_FILTER_ALPHA */
    isp_cfg.others.debayer_cfg.alpha_offset = debayer.alpha_offset;
    isp_cfg.others.debayer_cfg.alpha_scale = debayer.alpha_scale;

    /* C_FILTER_EDGE */
    isp_cfg.others.debayer_cfg.edge_offset = debayer.edge_offset;
    isp_cfg.others.debayer_cfg.edge_scale = debayer.edge_scale;

}
#endif

#if RKAIQ_HAVE_MERGE_V12
void Isp39Params::convertAiqMergeToIsp39Params(struct isp39_isp_params_cfg& isp_cfg,
        const rk_aiq_isp_merge_v32_t& amerge_data) {
    isp_cfg.module_en_update |= 1LL << RK_ISP2X_HDRMGE_ID;
    isp_cfg.module_ens |= 1LL << RK_ISP2X_HDRMGE_ID;
    isp_cfg.module_cfg_update |= 1LL << RK_ISP2X_HDRMGE_ID;

    isp_cfg.others.hdrmge_cfg.mode         = amerge_data.Merge_v12.mode;
    isp_cfg.others.hdrmge_cfg.gain0_inv    = amerge_data.Merge_v12.gain0_inv;
    isp_cfg.others.hdrmge_cfg.gain0        = amerge_data.Merge_v12.gain0;
    isp_cfg.others.hdrmge_cfg.gain1_inv    = amerge_data.Merge_v12.gain1_inv;
    isp_cfg.others.hdrmge_cfg.gain1        = amerge_data.Merge_v12.gain1;
    isp_cfg.others.hdrmge_cfg.gain2        = amerge_data.Merge_v12.gain2;
    isp_cfg.others.hdrmge_cfg.lm_dif_0p15  = amerge_data.Merge_v12.lm_dif_0p15;
    isp_cfg.others.hdrmge_cfg.lm_dif_0p9   = amerge_data.Merge_v12.lm_dif_0p9;
    isp_cfg.others.hdrmge_cfg.ms_diff_0p15 = amerge_data.Merge_v12.ms_dif_0p15;
    isp_cfg.others.hdrmge_cfg.ms_dif_0p8   = amerge_data.Merge_v12.ms_dif_0p8;
    for (int i = 0; i < ISP3X_HDRMGE_L_CURVE_NUM; i++) {
        isp_cfg.others.hdrmge_cfg.curve.curve_0[i] = amerge_data.Merge_v12.l0_y[i];
        isp_cfg.others.hdrmge_cfg.curve.curve_1[i] = amerge_data.Merge_v12.l1_y[i];
    }
    for (int i = 0; i < ISP3X_HDRMGE_E_CURVE_NUM; i++)
        isp_cfg.others.hdrmge_cfg.e_y[i] = amerge_data.Merge_v12.e_y[i];

    // merge v11 add
    isp_cfg.others.hdrmge_cfg.s_base  = amerge_data.Merge_v12.s_base;
    isp_cfg.others.hdrmge_cfg.ms_thd0 = amerge_data.Merge_v12.ms_thd0;
    isp_cfg.others.hdrmge_cfg.ms_thd1 = amerge_data.Merge_v12.ms_thd1;
    isp_cfg.others.hdrmge_cfg.ms_scl  = amerge_data.Merge_v12.ms_scl;
    isp_cfg.others.hdrmge_cfg.lm_thd0 = amerge_data.Merge_v12.lm_thd0;
    isp_cfg.others.hdrmge_cfg.lm_thd1 = amerge_data.Merge_v12.lm_thd1;
    isp_cfg.others.hdrmge_cfg.lm_scl  = amerge_data.Merge_v12.lm_scl;

    // merge v12 add
    isp_cfg.others.hdrmge_cfg.each_raw_en    = amerge_data.Merge_v12.each_raw_en;
    isp_cfg.others.hdrmge_cfg.each_raw_gain0 = amerge_data.Merge_v12.each_raw_gain0;
    isp_cfg.others.hdrmge_cfg.each_raw_gain1 = amerge_data.Merge_v12.each_raw_gain1;
    for (int i = 0; i < ISP3X_HDRMGE_E_CURVE_NUM; i++) {
        isp_cfg.others.hdrmge_cfg.l_raw0[i] = amerge_data.Merge_v12.l_raw0[i];
        isp_cfg.others.hdrmge_cfg.l_raw1[i] = amerge_data.Merge_v12.l_raw1[i];
    }

#if 0
    LOGE_CAMHW_SUBM(ISP20PARAM_SUBM, "%d: gain0_inv %d", __LINE__, isp_cfg.others.hdrmge_cfg.gain0_inv);
    LOGE_CAMHW_SUBM(ISP20PARAM_SUBM, "%d: gain0 %d", __LINE__, isp_cfg.others.hdrmge_cfg.gain0);
    LOGE_CAMHW_SUBM(ISP20PARAM_SUBM, "%d: gain1_inv %d", __LINE__, isp_cfg.others.hdrmge_cfg.gain1_inv);
    LOGE_CAMHW_SUBM(ISP20PARAM_SUBM, "%d: gain1 %d", __LINE__, isp_cfg.others.hdrmge_cfg.gain1);
    LOGE_CAMHW_SUBM(ISP20PARAM_SUBM, "%d: gain2 %d", __LINE__, isp_cfg.others.hdrmge_cfg.gain2);
    LOGE_CAMHW_SUBM(ISP20PARAM_SUBM, "%d: lm_dif_0p15 %d", __LINE__, isp_cfg.others.hdrmge_cfg.lm_dif_0p15);
    LOGE_CAMHW_SUBM(ISP20PARAM_SUBM, "%d: lm_dif_0p9 %d", __LINE__, isp_cfg.others.hdrmge_cfg.lm_dif_0p9);
    LOGE_CAMHW_SUBM(ISP20PARAM_SUBM, "%d: ms_diff_0p15 %d", __LINE__, isp_cfg.others.hdrmge_cfg.ms_diff_0p15);
    LOGE_CAMHW_SUBM(ISP20PARAM_SUBM, "%d: ms_dif_0p8 %d", __LINE__, isp_cfg.others.hdrmge_cfg.ms_dif_0p8);
    LOGE_CAMHW_SUBM(ISP20PARAM_SUBM, "%d: s_base %d", __LINE__, isp_cfg.others.hdrmge_cfg.s_base);
    LOGE_CAMHW_SUBM(ISP20PARAM_SUBM, "%d: sw_hdrmge_ms_thd0 %d", __LINE__, isp_cfg.others.hdrmge_cfg.ms_thd0);
    LOGE_CAMHW_SUBM(ISP20PARAM_SUBM, "%d: ms_thd0 %d", __LINE__, isp_cfg.others.hdrmge_cfg.ms_thd0);
    LOGE_CAMHW_SUBM(ISP20PARAM_SUBM, "%d: ms_thd1 %d", __LINE__, isp_cfg.others.hdrmge_cfg.ms_thd1);
    LOGE_CAMHW_SUBM(ISP20PARAM_SUBM, "%d: ms_scl %d", __LINE__, isp_cfg.others.hdrmge_cfg.ms_scl);
    LOGE_CAMHW_SUBM(ISP20PARAM_SUBM, "%d: lm_thd0 %d", __LINE__, isp_cfg.others.hdrmge_cfg.lm_thd0);
    LOGE_CAMHW_SUBM(ISP20PARAM_SUBM, "%d: lm_thd1 %d", __LINE__, isp_cfg.others.hdrmge_cfg.lm_thd1);
    LOGE_CAMHW_SUBM(ISP20PARAM_SUBM, "%d: lm_scl %d", __LINE__, isp_cfg.others.hdrmge_cfg.lm_scl);
    LOGE_CAMHW_SUBM(ISP20PARAM_SUBM, "%d: each_raw_en 0x%x each_raw_gain0 0x%x each_raw_gain1 0x%x\n", __LINE__, isp_cfg.others.hdrmge_cfg.each_raw_en,
                    isp_cfg.others.hdrmge_cfg.each_raw_gain0, isp_cfg.others.hdrmge_cfg.each_raw_gain1);
    for(int i = 0 ; i < 17; i++)
    {
        LOGE_CAMHW_SUBM(ISP20PARAM_SUBM, "%d: curve_0[%d] %d", __LINE__, i, isp_cfg.others.hdrmge_cfg.curve.curve_0[i]);
        LOGE_CAMHW_SUBM(ISP20PARAM_SUBM, "%d: curve_1[%d] %d", __LINE__, i, isp_cfg.others.hdrmge_cfg.curve.curve_1[i]);
        LOGE_CAMHW_SUBM(ISP20PARAM_SUBM, "%d: e_y[%d] %d", __LINE__, i, isp_cfg.others.hdrmge_cfg.e_y[i]);
    }
#endif
}
#endif

#if RKAIQ_HAVE_DEHAZE_V14
#ifdef USE_NEWSTRUCT
void Isp39Params::convertAiqDehazeToIsp39Params(struct isp39_isp_params_cfg& isp_cfg,
        rk_aiq_isp_dehaze_params_t* dehaze_attr) {

    //hwi_api_dehaze23_params_check(&dehaze_attr->result);

    if (dehaze_attr->en) {
        isp_cfg.module_en_update |= ISP2X_MODULE_DHAZ;
        isp_cfg.module_ens |= ISP2X_MODULE_DHAZ;
        isp_cfg.module_cfg_update |= ISP2X_MODULE_DHAZ;
    } else {
        isp_cfg.module_en_update |= ISP2X_MODULE_DHAZ;
        isp_cfg.module_ens &= ~(ISP2X_MODULE_DHAZ);
        isp_cfg.module_cfg_update &= ~(ISP2X_MODULE_DHAZ);
        return;
    }

    //hwi_api_dehaze23_params_cvt(&dehaze_attr->result, &isp_cfg);
    //hwi_api_dehaze23_params_dump(&dehaze_attr->result, &isp_cfg.others.dhaz_cfg);
}
#else
void Isp39Params::convertAiqAdehazeToIsp39Params(struct isp39_isp_params_cfg& isp_cfg,
        const rk_aiq_isp_dehaze_v39_t& dhaze) {
    if (dhaze.enable) {
        isp_cfg.module_en_update |= ISP2X_MODULE_DHAZ;
        isp_cfg.module_ens |= ISP2X_MODULE_DHAZ;
        isp_cfg.module_cfg_update |= ISP2X_MODULE_DHAZ;
    } else {
        isp_cfg.module_en_update |= ISP2X_MODULE_DHAZ;
        isp_cfg.module_ens &= ~(ISP2X_MODULE_DHAZ);
        isp_cfg.module_cfg_update &= ~(ISP2X_MODULE_DHAZ);
        return;
    }

    struct isp39_dhaz_cfg* cfg = &isp_cfg.others.dhaz_cfg;

    cfg->dc_en            = dhaze.ProcResV14.dc_en;
    cfg->hist_en          = dhaze.ProcResV14.hist_en;
    cfg->map_mode         = dhaze.ProcResV14.map_mode;
    cfg->mem_mode         = dhaze.ProcResV14.mem_mode;
    cfg->mem_force        = dhaze.ProcResV14.mem_force;
    cfg->air_lc_en        = dhaze.ProcResV14.air_lc_en;
    cfg->enhance_en       = dhaze.ProcResV14.enhance_en;
    cfg->soft_wr_en       = dhaze.ProcResV14.soft_wr_en;
    cfg->round_en         = dhaze.ProcResV14.round_en;
    cfg->color_deviate_en = dhaze.ProcResV14.color_deviate_en;
    cfg->enh_luma_en      = dhaze.ProcResV14.enh_luma_en;
    cfg->dc_min_th        = dhaze.ProcResV14.dc_min_th;
    cfg->dc_max_th        = dhaze.ProcResV14.dc_max_th;
    cfg->yhist_th         = dhaze.ProcResV14.yhist_th;
    cfg->yblk_th          = dhaze.ProcResV14.yblk_th;
    cfg->bright_min       = dhaze.ProcResV14.bright_min;
    cfg->bright_max       = dhaze.ProcResV14.bright_max;
    cfg->wt_max           = dhaze.ProcResV14.wt_max;
    cfg->air_min          = dhaze.ProcResV14.air_min;
    cfg->air_max          = dhaze.ProcResV14.air_max;
    cfg->dark_th          = dhaze.ProcResV14.dark_th;
    cfg->tmax_base        = dhaze.ProcResV14.tmax_base;
    cfg->tmax_off         = dhaze.ProcResV14.tmax_off;
    cfg->tmax_max         = dhaze.ProcResV14.tmax_max;
    cfg->enhance_chroma   = dhaze.ProcResV14.enhance_chroma;
    cfg->enhance_value    = dhaze.ProcResV14.enhance_value;
    cfg->stab_fnum        = dhaze.ProcResV14.stab_fnum;
    cfg->iir_sigma        = dhaze.ProcResV14.iir_sigma;
    cfg->iir_wt_sigma     = dhaze.ProcResV14.iir_wt_sigma;
    cfg->iir_air_sigma    = dhaze.ProcResV14.iir_air_sigma;
    cfg->iir_tmax_sigma   = dhaze.ProcResV14.iir_tmax_sigma;
    cfg->iir_pre_wet      = dhaze.ProcResV14.iir_pre_wet;
    cfg->cfg_alpha        = dhaze.ProcResV14.cfg_alpha;
    cfg->cfg_air          = dhaze.ProcResV14.cfg_air;
    cfg->cfg_wt           = dhaze.ProcResV14.cfg_wt;
    cfg->cfg_tmax         = dhaze.ProcResV14.cfg_tmax;
    cfg->space_sigma_cur  = dhaze.ProcResV14.space_sigma_cur;
    cfg->space_sigma_pre  = dhaze.ProcResV14.space_sigma_pre;
    cfg->range_sima       = dhaze.ProcResV14.range_sima;
    cfg->bf_weight        = dhaze.ProcResV14.bf_weight;
    cfg->dc_weitcur       = dhaze.ProcResV14.dc_weitcur;
    cfg->gaus_h0          = dhaze.ProcResV14.gaus_h0;
    cfg->gaus_h1          = dhaze.ProcResV14.gaus_h1;
    cfg->gaus_h2          = dhaze.ProcResV14.gaus_h2;
    cfg->adp_wt_wr        = dhaze.ProcResV14.adp_wt_wr;
    cfg->adp_air_wr       = dhaze.ProcResV14.adp_air_wr;
    cfg->adp_tmax_wr      = dhaze.ProcResV14.adp_tmax_wr;
    cfg->hist_k           = dhaze.ProcResV14.hist_k;
    cfg->hist_th_off      = dhaze.ProcResV14.hist_th_off;
    cfg->hist_min         = dhaze.ProcResV14.hist_min;
    cfg->blk_het          = dhaze.ProcResV14.blk_het;
    cfg->blk_wid          = dhaze.ProcResV14.blk_wid;
    cfg->thumb_row        = dhaze.ProcResV14.thumb_row;
    cfg->thumb_col        = dhaze.ProcResV14.thumb_col;
    cfg->cfg_k_alpha      = dhaze.ProcResV14.cfg_k_alpha;
    cfg->cfg_k            = dhaze.ProcResV14.cfg_k;
    cfg->k_gain           = dhaze.ProcResV14.k_gain;
    cfg->gain_fuse_alpha  = dhaze.ProcResV14.gain_fuse_alpha;

    for (int i = 0; i < ISP39_DHAZ_SIGMA_IDX_NUM; i++)
        cfg->sigma_idx[i] = dhaze.ProcResV14.sigma_idx[i];

    for (int i = 0; i < ISP39_DHAZ_SIGMA_LUT_NUM; i++)
        cfg->sigma_lut[i] = dhaze.ProcResV14.sigma_lut[i];

    for (int i = 0; i < ISP39_DHAZ_ENH_CURVE_NUM; i++)
        cfg->enh_curve[i] = dhaze.ProcResV14.enh_curve[i];

    for (int i = 0; i < ISP39_DHAZ_ENH_LUMA_NUM; i++)
        cfg->enh_luma[i] = dhaze.ProcResV14.enh_luma[i];

    for (int i = 0; i < ISP39_DHAZ_BLEND_WET_NUM; i++)
        cfg->blend_wet[i] = dhaze.ProcResV14.blend_wet[i];

    for (int i = 0; i < ISP39_DHAZ_HIST_IIR_BLK_MAX; i++)
        for (int j = 0; j < ISP39_DHAZ_HIST_IIR_NUM; j++)
            cfg->hist_iir[i][j] = dhaze.ProcResV14.hist_iir[i][j];

    // set hist_iir_wr 0x0 for now, do  not need for now
    cfg->hist_iir_wr = 0x0;

#if 0
    LOGE_ADEHAZE("%s(%d) dehaze local gain IDX(0~5): 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n", __func__, __LINE__, cfg->sigma_idx[0], cfg->sigma_idx[1],
                 cfg->sigma_idx[2], cfg->sigma_idx[3], cfg->sigma_idx[4], cfg->sigma_idx[5]);
    LOGE_ADEHAZE("%s(%d) dehaze local gain LUT(0~5): 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n", __func__, __LINE__, cfg->sigma_lut[0], cfg->sigma_lut[1],
                 cfg->sigma_lut[2], cfg->sigma_lut[3], cfg->sigma_lut[4], cfg->sigma_lut[5]);
    LOGE_ADEHAZE("%s(%d) enh_luma(0~5): 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n", __func__, __LINE__, cfg->enh_luma[0], cfg->enh_luma[1],
                 cfg->enh_luma[2], cfg->enh_luma[3], cfg->enh_luma[4], cfg->enh_luma[5]);
#endif
}
#endif
#endif

#if RKAIQ_HAVE_DRC_V20
#ifdef USE_NEWSTRUCT
void Isp39Params::convertAiqDrcToIsp39Params(struct isp39_isp_params_cfg& isp_cfg,
        rk_aiq_isp_drc_params_t* drc_attr) {
    //hwi_api_drc40_params_check(&drc_attr->result);

    if (drc_attr->en) {
        isp_cfg.module_en_update |= 1LL << Rk_ISP21_DRC_ID;
        isp_cfg.module_ens |= 1LL << Rk_ISP21_DRC_ID;
        isp_cfg.module_cfg_update |= 1LL << Rk_ISP21_DRC_ID;
    } else {
        isp_cfg.module_en_update |= 1LL << Rk_ISP21_DRC_ID;
        isp_cfg.module_ens &= ~(1LL << Rk_ISP21_DRC_ID);
        // need to update cmps_byp_en when drc en off
        isp_cfg.module_cfg_update |= (1LL << Rk_ISP21_DRC_ID);
    }

    rk_aiq_drc40_params_cvt(&drc_attr->result, &isp_cfg, &mCommonCvtInfo);
    //hwi_api_drc40_params_cvt(&drc_attr->result, &isp_cfg);
    //hwi_api_drc40_params_dump(&drc_attr->result, &isp_cfg.others.drc_cfg);
}
#else
void Isp39Params::convertAiqDrcToIsp39Params(struct isp39_isp_params_cfg& isp_cfg,
        rk_aiq_isp_drc_v39_t& adrc_data) {
    // cmps_byp_en is not related to drc en
    isp_cfg.others.drc_cfg.cmps_byp_en = adrc_data.DrcProcRes.Drc_v20.cmps_byp_en;
    if (adrc_data.bDrcEn) {
        isp_cfg.module_en_update |= 1LL << Rk_ISP21_DRC_ID;
        isp_cfg.module_ens |= 1LL << Rk_ISP21_DRC_ID;
        isp_cfg.module_cfg_update |= 1LL << Rk_ISP21_DRC_ID;
    } else {
        isp_cfg.module_en_update |= 1LL << Rk_ISP21_DRC_ID;
        isp_cfg.module_ens &= ~(1LL << Rk_ISP21_DRC_ID);
        // need to update cmps_byp_en when drc en off
        isp_cfg.module_cfg_update |= 1LL << Rk_ISP21_DRC_ID;
        return;
    }

    isp_cfg.others.drc_cfg.bypass_en            = adrc_data.DrcProcRes.Drc_v20.bypass_en;
    isp_cfg.others.drc_cfg.gainx32_en           = adrc_data.DrcProcRes.Drc_v20.gainx32_en;
    isp_cfg.others.drc_cfg.raw_dly_dis          = adrc_data.DrcProcRes.Drc_v20.raw_dly_dis;
    isp_cfg.others.drc_cfg.position             = adrc_data.DrcProcRes.Drc_v20.position;
    isp_cfg.others.drc_cfg.compres_scl          = adrc_data.DrcProcRes.Drc_v20.compres_scl;
    isp_cfg.others.drc_cfg.offset_pow2          = adrc_data.DrcProcRes.Drc_v20.offset_pow2;
    isp_cfg.others.drc_cfg.lpdetail_ratio       = adrc_data.DrcProcRes.Drc_v20.lpdetail_ratio;
    isp_cfg.others.drc_cfg.hpdetail_ratio       = adrc_data.DrcProcRes.Drc_v20.hpdetail_ratio;
    isp_cfg.others.drc_cfg.delta_scalein        = adrc_data.DrcProcRes.Drc_v20.delta_scalein;
    isp_cfg.others.drc_cfg.bilat_wt_off         = adrc_data.DrcProcRes.Drc_v20.bilat_wt_off;
    isp_cfg.others.drc_cfg.thumb_thd_neg        = adrc_data.DrcProcRes.Drc_v20.thumb_thd_neg;
    isp_cfg.others.drc_cfg.thumb_thd_enable     = adrc_data.DrcProcRes.Drc_v20.thumb_thd_enable;
    isp_cfg.others.drc_cfg.weicur_pix           = adrc_data.DrcProcRes.Drc_v20.weicur_pix;
    isp_cfg.others.drc_cfg.cmps_offset_bits_int = adrc_data.DrcProcRes.Drc_v20.cmps_offset_bits_int;
    isp_cfg.others.drc_cfg.cmps_fixbit_mode     = adrc_data.DrcProcRes.Drc_v20.cmps_fixbit_mode;
    isp_cfg.others.drc_cfg.drc_gas_t            = adrc_data.DrcProcRes.Drc_v20.drc_gas_t;
    isp_cfg.others.drc_cfg.thumb_clip           = adrc_data.DrcProcRes.Drc_v20.thumb_clip;
    isp_cfg.others.drc_cfg.thumb_scale          = adrc_data.DrcProcRes.Drc_v20.thumb_scale;
    isp_cfg.others.drc_cfg.range_sgm_inv0       = adrc_data.DrcProcRes.Drc_v20.range_sgm_inv0;
    isp_cfg.others.drc_cfg.range_sgm_inv1       = adrc_data.DrcProcRes.Drc_v20.range_sgm_inv1;
    isp_cfg.others.drc_cfg.weig_bilat           = adrc_data.DrcProcRes.Drc_v20.weig_bilat;
    isp_cfg.others.drc_cfg.weight_8x8thumb      = adrc_data.DrcProcRes.Drc_v20.weight_8x8thumb;
    isp_cfg.others.drc_cfg.bilat_soft_thd       = adrc_data.DrcProcRes.Drc_v20.bilat_soft_thd;
    isp_cfg.others.drc_cfg.enable_soft_thd      = adrc_data.DrcProcRes.Drc_v20.enable_soft_thd;
    isp_cfg.others.drc_cfg.min_ogain            = adrc_data.DrcProcRes.Drc_v20.min_ogain;
    for (int i = 0; i < ISP39_DRC_Y_NUM; i++) {
        isp_cfg.others.drc_cfg.gain_y[i]    = adrc_data.DrcProcRes.Drc_v20.gain_y[i];
        isp_cfg.others.drc_cfg.compres_y[i] = adrc_data.DrcProcRes.Drc_v20.compres_y[i];
        isp_cfg.others.drc_cfg.scale_y[i]   = adrc_data.DrcProcRes.Drc_v20.scale_y[i];
        isp_cfg.others.drc_cfg.sfthd_y[i]   = adrc_data.DrcProcRes.Drc_v20.sfthd_y[i];
    }

    LOG1_ATMO(
        "%s: bypass_en:%d gainx32_en:%d raw_dly_dis:%d sw_drc_position:%d sw_drc_compres_scl:%d "
        "sw_drc_offset_pow2:%d\n",
        __FUNCTION__, isp_cfg.others.drc_cfg.bypass_en, isp_cfg.others.drc_cfg.gainx32_en,
        isp_cfg.others.drc_cfg.raw_dly_dis, isp_cfg.others.drc_cfg.position,
        isp_cfg.others.drc_cfg.compres_scl, isp_cfg.others.drc_cfg.offset_pow2);
    LOG1_ATMO("%s: sw_drc_lpdetail_ratio:%d sw_drc_hpdetail_ratio:%d sw_drc_delta_scalein:%d\n",
              __FUNCTION__, isp_cfg.others.drc_cfg.lpdetail_ratio,
              isp_cfg.others.drc_cfg.hpdetail_ratio, isp_cfg.others.drc_cfg.delta_scalein);
    LOG1_ATMO(
        "%s: sw_drc_bilat_wt_off:%d thumb_thd_neg:%d thumb_thd_enable:%d sw_drc_weicur_pix:%d\n",
        __FUNCTION__, isp_cfg.others.drc_cfg.bilat_wt_off, isp_cfg.others.drc_cfg.thumb_thd_neg,
        isp_cfg.others.drc_cfg.thumb_thd_enable, isp_cfg.others.drc_cfg.weicur_pix);
    LOG1_ATMO(
        "%s: cmps_byp_en:%d cmps_offset_bits_int:%d cmps_fixbit_mode:%d drc_gas_t:%d thumb_clip:%d "
        "thumb_scale:%d "
        "\n",
        __FUNCTION__, isp_cfg.others.drc_cfg.cmps_byp_en,
        isp_cfg.others.drc_cfg.cmps_offset_bits_int, isp_cfg.others.drc_cfg.cmps_fixbit_mode,
        isp_cfg.others.drc_cfg.drc_gas_t, isp_cfg.others.drc_cfg.thumb_clip,
        isp_cfg.others.drc_cfg.thumb_scale);
    LOG1_ATMO(
        "%s: sw_drc_range_sgm_inv0:%d sw_drc_range_sgm_inv1:%d weig_bilat:%d weight_8x8thumb:%d\n",
        __FUNCTION__, isp_cfg.others.drc_cfg.range_sgm_inv0, isp_cfg.others.drc_cfg.range_sgm_inv1,
        isp_cfg.others.drc_cfg.weig_bilat, isp_cfg.others.drc_cfg.weight_8x8thumb);
    LOG1_ATMO(
        "%s: sw_drc_bilat_soft_thd:%d "
        "sw_drc_enable_soft_thd:%d sw_drc_min_ogain:%d\n",
        __FUNCTION__, isp_cfg.others.drc_cfg.bilat_soft_thd, isp_cfg.others.drc_cfg.enable_soft_thd,
        isp_cfg.others.drc_cfg.min_ogain);
    LOG1_ATMO("%s: sw_drc_gain_y: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
              __FUNCTION__, isp_cfg.others.drc_cfg.gain_y[0], isp_cfg.others.drc_cfg.gain_y[1],
              isp_cfg.others.drc_cfg.gain_y[2], isp_cfg.others.drc_cfg.gain_y[3],
              isp_cfg.others.drc_cfg.gain_y[4], isp_cfg.others.drc_cfg.gain_y[5],
              isp_cfg.others.drc_cfg.gain_y[6], isp_cfg.others.drc_cfg.gain_y[7],
              isp_cfg.others.drc_cfg.gain_y[8], isp_cfg.others.drc_cfg.gain_y[9],
              isp_cfg.others.drc_cfg.gain_y[10], isp_cfg.others.drc_cfg.gain_y[11],
              isp_cfg.others.drc_cfg.gain_y[12], isp_cfg.others.drc_cfg.gain_y[13],
              isp_cfg.others.drc_cfg.gain_y[14], isp_cfg.others.drc_cfg.gain_y[15],
              isp_cfg.others.drc_cfg.gain_y[16]);
    LOG1_ATMO("%s: sw_drc_scale_y: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
              __FUNCTION__, isp_cfg.others.drc_cfg.scale_y[0], isp_cfg.others.drc_cfg.scale_y[1],
              isp_cfg.others.drc_cfg.scale_y[2], isp_cfg.others.drc_cfg.scale_y[3],
              isp_cfg.others.drc_cfg.scale_y[4], isp_cfg.others.drc_cfg.scale_y[5],
              isp_cfg.others.drc_cfg.scale_y[6], isp_cfg.others.drc_cfg.scale_y[7],
              isp_cfg.others.drc_cfg.scale_y[8], isp_cfg.others.drc_cfg.scale_y[9],
              isp_cfg.others.drc_cfg.scale_y[10], isp_cfg.others.drc_cfg.scale_y[11],
              isp_cfg.others.drc_cfg.scale_y[12], isp_cfg.others.drc_cfg.scale_y[13],
              isp_cfg.others.drc_cfg.scale_y[14], isp_cfg.others.drc_cfg.scale_y[15],
              isp_cfg.others.drc_cfg.scale_y[16]);
    LOG1_ATMO(
        "%s: sw_drc_compres_y: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d "
        "%d\n",
        __FUNCTION__, isp_cfg.others.drc_cfg.compres_y[0], isp_cfg.others.drc_cfg.compres_y[1],
        isp_cfg.others.drc_cfg.compres_y[2], isp_cfg.others.drc_cfg.compres_y[3],
        isp_cfg.others.drc_cfg.compres_y[4], isp_cfg.others.drc_cfg.compres_y[5],
        isp_cfg.others.drc_cfg.compres_y[6], isp_cfg.others.drc_cfg.compres_y[7],
        isp_cfg.others.drc_cfg.compres_y[8], isp_cfg.others.drc_cfg.compres_y[9],
        isp_cfg.others.drc_cfg.compres_y[10], isp_cfg.others.drc_cfg.compres_y[11],
        isp_cfg.others.drc_cfg.compres_y[12], isp_cfg.others.drc_cfg.compres_y[13],
        isp_cfg.others.drc_cfg.compres_y[14], isp_cfg.others.drc_cfg.compres_y[15],
        isp_cfg.others.drc_cfg.compres_y[16]);
    LOG1_ATMO(
        "%s: sfthd_y: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d "
        "%d\n",
        __FUNCTION__, isp_cfg.others.drc_cfg.sfthd_y[0], isp_cfg.others.drc_cfg.sfthd_y[1],
        isp_cfg.others.drc_cfg.sfthd_y[2], isp_cfg.others.drc_cfg.sfthd_y[3],
        isp_cfg.others.drc_cfg.sfthd_y[4], isp_cfg.others.drc_cfg.sfthd_y[5],
        isp_cfg.others.drc_cfg.sfthd_y[6], isp_cfg.others.drc_cfg.sfthd_y[7],
        isp_cfg.others.drc_cfg.sfthd_y[8], isp_cfg.others.drc_cfg.sfthd_y[9],
        isp_cfg.others.drc_cfg.sfthd_y[10], isp_cfg.others.drc_cfg.sfthd_y[11],
        isp_cfg.others.drc_cfg.sfthd_y[12], isp_cfg.others.drc_cfg.sfthd_y[13],
        isp_cfg.others.drc_cfg.sfthd_y[14], isp_cfg.others.drc_cfg.sfthd_y[15],
        isp_cfg.others.drc_cfg.sfthd_y[16]);
}
#endif
#endif

#if RKAIQ_HAVE_RGBIR_REMOSAIC_V10
void Isp39Params::convertAiqRgbirToIsp39Params(struct isp39_isp_params_cfg& isp_cfg,
        rk_aiq_isp_rgbir_v39_t& argbir_data) {
    if (argbir_data.bRgbirEn) {
        isp_cfg.module_ens |= ISP39_MODULE_RGBIR;
        isp_cfg.module_en_update |= ISP39_MODULE_RGBIR;
        isp_cfg.module_cfg_update |= ISP39_MODULE_RGBIR;
    } else {
        isp_cfg.module_ens &= ~ISP39_MODULE_RGBIR;
        isp_cfg.module_en_update |= ISP39_MODULE_RGBIR;
        isp_cfg.module_cfg_update |= ISP39_MODULE_RGBIR;
        return;
    }

    isp_cfg.others.rgbir_cfg.coe_delta = argbir_data.RgbirProcRes.rgbir_v10.coe_delta;
    isp_cfg.others.rgbir_cfg.coe_theta = argbir_data.RgbirProcRes.rgbir_v10.coe_theta;
    for (int i = 0; i < ISP39_RGBIR_SCALE_NUM; i++)
        isp_cfg.others.rgbir_cfg.scale[i] = argbir_data.RgbirProcRes.rgbir_v10.scale[i];
    for (int i = 0; i < ISP39_RGBIR_LUMA_POINT_NUM; i++) {
        isp_cfg.others.rgbir_cfg.luma_point[i] = argbir_data.RgbirProcRes.rgbir_v10.luma_point[i];
        isp_cfg.others.rgbir_cfg.scale_map[i]  = argbir_data.RgbirProcRes.rgbir_v10.scale_map[i];
    }

    LOGV_ARGBIR(
        "%s: edge_aware_coef:%d ir_black_level:%d b_scale_coef:%d gb_scale_coef:%d "
        "gr_scale_coef:%d r_scale_coef:%d\n",
        __FUNCTION__, isp_cfg.others.rgbir_cfg.coe_delta, isp_cfg.others.rgbir_cfg.coe_theta,
        isp_cfg.others.rgbir_cfg.scale[3], isp_cfg.others.rgbir_cfg.scale[2],
        isp_cfg.others.rgbir_cfg.scale[1], isp_cfg.others.rgbir_cfg.scale[0]);
    LOGV_ARGBIR("%s: luma_point: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
                __FUNCTION__, isp_cfg.others.rgbir_cfg.luma_point[0],
                isp_cfg.others.rgbir_cfg.luma_point[1], isp_cfg.others.rgbir_cfg.luma_point[2],
                isp_cfg.others.rgbir_cfg.luma_point[3], isp_cfg.others.rgbir_cfg.luma_point[4],
                isp_cfg.others.rgbir_cfg.luma_point[5], isp_cfg.others.rgbir_cfg.luma_point[6],
                isp_cfg.others.rgbir_cfg.luma_point[7], isp_cfg.others.rgbir_cfg.luma_point[8],
                isp_cfg.others.rgbir_cfg.luma_point[9], isp_cfg.others.rgbir_cfg.luma_point[10],
                isp_cfg.others.rgbir_cfg.luma_point[11], isp_cfg.others.rgbir_cfg.luma_point[12],
                isp_cfg.others.rgbir_cfg.luma_point[13], isp_cfg.others.rgbir_cfg.luma_point[14],
                isp_cfg.others.rgbir_cfg.luma_point[15], isp_cfg.others.rgbir_cfg.luma_point[16]);
    LOGV_ARGBIR("%s: scale_map: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n", __FUNCTION__,
                isp_cfg.others.rgbir_cfg.scale_map[0], isp_cfg.others.rgbir_cfg.scale_map[1],
                isp_cfg.others.rgbir_cfg.scale_map[2], isp_cfg.others.rgbir_cfg.scale_map[3],
                isp_cfg.others.rgbir_cfg.scale_map[4], isp_cfg.others.rgbir_cfg.scale_map[5],
                isp_cfg.others.rgbir_cfg.scale_map[6], isp_cfg.others.rgbir_cfg.scale_map[7],
                isp_cfg.others.rgbir_cfg.scale_map[8], isp_cfg.others.rgbir_cfg.scale_map[9],
                isp_cfg.others.rgbir_cfg.scale_map[10], isp_cfg.others.rgbir_cfg.scale_map[11],
                isp_cfg.others.rgbir_cfg.scale_map[12], isp_cfg.others.rgbir_cfg.scale_map[13],
                isp_cfg.others.rgbir_cfg.scale_map[14], isp_cfg.others.rgbir_cfg.scale_map[15],
                isp_cfg.others.rgbir_cfg.scale_map[16]);
}
#endif

#if RKAIQ_HAVE_CCM_V3
void Isp39Params::convertAiqCcmToIsp39Params(struct isp39_isp_params_cfg& isp_cfg,
        const rk_aiq_ccm_cfg_v2_t& ccm) {
    if (ccm.ccmEnable) {
        isp_cfg.module_ens |= ISP3X_MODULE_CCM;
    }
    isp_cfg.module_en_update |= ISP3X_MODULE_CCM;
    isp_cfg.module_cfg_update |= ISP3X_MODULE_CCM;

    struct isp39_ccm_cfg* cfg = &isp_cfg.others.ccm_cfg;

    const float* coeff        = ccm.matrix;
    const float* offset       = ccm.offs;

    cfg->coeff0_r = (coeff[0] - 1) > 0 ? (short)((coeff[0] - 1) * 128 + 0.5)
                    : (short)((coeff[0] - 1) * 128 - 0.5);  // check -128?
    cfg->coeff1_r = coeff[1] > 0 ? (short)(coeff[1] * 128 + 0.5) : (short)(coeff[1] * 128 - 0.5);
    cfg->coeff2_r = coeff[2] > 0 ? (short)(coeff[2] * 128 + 0.5) : (short)(coeff[2] * 128 - 0.5);
    cfg->coeff0_g = coeff[3] > 0 ? (short)(coeff[3] * 128 + 0.5) : (short)(coeff[3] * 128 - 0.5);
    cfg->coeff1_g = (coeff[4] - 1) > 0 ? (short)((coeff[4] - 1) * 128 + 0.5)
                    : (short)((coeff[4] - 1) * 128 - 0.5);
    cfg->coeff2_g = coeff[5] > 0 ? (short)(coeff[5] * 128 + 0.5) : (short)(coeff[5] * 128 - 0.5);
    cfg->coeff0_b = coeff[6] > 0 ? (short)(coeff[6] * 128 + 0.5) : (short)(coeff[6] * 128 - 0.5);
    cfg->coeff1_b = coeff[7] > 0 ? (short)(coeff[7] * 128 + 0.5) : (short)(coeff[7] * 128 - 0.5);
    cfg->coeff2_b = (coeff[8] - 1) > 0 ? (short)((coeff[8] - 1) * 128 + 0.5)
                    : (short)((coeff[8] - 1) * 128 - 0.5);

    cfg->offset_r =
        offset[0] > 0 ? (short)(offset[0] + 0.5) : (short)(offset[0] - 0.5);  // for 12bit
    cfg->offset_g = offset[1] > 0 ? (short)(offset[1] + 0.5) : (int)(offset[1] - 0.5);
    cfg->offset_b = offset[2] > 0 ? (short)(offset[2] + 0.5) : (short)(offset[2] - 0.5);

    cfg->coeff0_y         = (u16)ccm.rgb2y_para[0];
    cfg->coeff1_y         = (u16)ccm.rgb2y_para[1];
    cfg->coeff2_y         = (u16)ccm.rgb2y_para[2];
    cfg->bound_bit        = (u8)ccm.bound_bit;  // check
    cfg->right_bit        = (u8)ccm.right_bit;
    cfg->highy_adjust_dis = (u8)(ccm.highy_adj_en ? 0 : 1);
    for (int i = 0; i < ISP32_CCM_CURVE_NUM; i++) {
        cfg->alp_y[i] = (u16)(ccm.alp_y[i]);
    }
    cfg->enh_adj_en  = (u8)(ccm.enh_adj_en);
    cfg->asym_adj_en = (u8)(ccm.asym_adj_en ? 1 : 0);

    cfg->color_coef0_r2y   = (u16)ccm.enh_rgb2y_para[0];
    cfg->color_coef1_g2y   = (u16)ccm.enh_rgb2y_para[1];
    cfg->color_coef2_b2y   = (u16)ccm.enh_rgb2y_para[2];
    cfg->color_enh_rat_max = (u16)(ccm.enh_rat_max * 1024);

    cfg->sat_decay_en = ccm.sat_decay_en;
    cfg->hf_low       = ccm.hf_low;
    cfg->hf_up        = ccm.hf_up;
    cfg->hf_scale     = ccm.hf_scale;
    for (int i = 0; i < ISP39_CCM_HF_FACTOR_NUM; i++) {
        cfg->hf_factor[i] = ccm.hf_factor[i];
    }
}
#endif

#if (RKAIQ_HAVE_SHARP_V34)
#if USE_NEWSTRUCT
void Isp39Params::convertAiqSharpToIsp39Params(struct isp39_isp_params_cfg& isp_cfg,
        rk_aiq_isp_sharp_params_t *attr) {

    isp_cfg.module_ens |= ISP3X_MODULE_SHARP;
    isp_cfg.module_en_update |= ISP3X_MODULE_SHARP;
    isp_cfg.module_cfg_update |= ISP3X_MODULE_SHARP;

    struct isp39_sharp_cfg* pSharp = &isp_cfg.others.sharp_cfg;

    rk_aiq_sharp34_params_cvt(&attr->result, &isp_cfg, &mCommonCvtInfo);
}
#else
void Isp39Params::convertAiqSharpenToIsp39Params(struct isp39_isp_params_cfg& isp_cfg,
        rk_aiq_isp_sharp_v39_t& sharp) {
    bool enable = !sharp.bypass;

    isp_cfg.module_en_update |= ISP3X_MODULE_SHARP;
    isp_cfg.module_ens |= ISP3X_MODULE_SHARP;
    isp_cfg.module_cfg_update |= ISP3X_MODULE_SHARP;

    struct isp39_sharp_cfg* pSharp = &isp_cfg.others.sharp_cfg;

    pSharp->bypass            = sharp.bypass;
    pSharp->center_mode       = sharp.center_mode;
    pSharp->local_gain_bypass = sharp.local_gain_bypass;
    pSharp->radius_step_mode  = sharp.radius_step_mode;
    pSharp->noise_clip_mode   = sharp.noise_clip_mode;
    pSharp->clipldx_sel       = sharp.clipldx_sel;
    pSharp->baselmg_sel       = sharp.baselmg_sel;
    pSharp->noise_filt_sel    = sharp.noise_filt_sel;
    pSharp->tex2wgt_en        = sharp.tex2wgt_en;
    LOGV_ASHARP(
        "%s:%d  bypass:0x%x center_mode:0x%x local_gain_bypass:0x%x radius_ds_mode:0x%x "
        "noiseclip_mode:0x%x clip_hf_mode:0x%x add_mode:0x%x\n",
        __FUNCTION__, __LINE__, pSharp->bypass, pSharp->center_mode, pSharp->local_gain_bypass,
        pSharp->radius_step_mode, pSharp->noise_clip_mode, pSharp->clipldx_sel,
        pSharp->baselmg_sel);

    pSharp->pre_bifilt_alpha    = sharp.pre_bifilt_alpha;
    pSharp->guide_filt_alpha    = sharp.guide_filt_alpha;
    pSharp->detail_bifilt_alpha = sharp.detail_bifilt_alpha;
    pSharp->global_sharp_strg   = sharp.global_sharp_strg;
    LOGV_ASHARP(
        "%s:%d  tex_mean_mode:0x%x tex_lut_en:0x%x pbf_ratio:0x%x gaus_ratio:0x%x bf_ratio:0x%x\n",
        __FUNCTION__, __LINE__, pSharp->noise_filt_sel, pSharp->tex2wgt_en,
        pSharp->pre_bifilt_alpha, pSharp->guide_filt_alpha, pSharp->detail_bifilt_alpha,
        pSharp->global_sharp_strg);

    for (int i = 0; i < ISP39_SHARP_X_NUM; i++) pSharp->luma2table_idx[i] = sharp.luma2table_idx[i];
    LOGV_ASHARP("%s:%d  luma2table_idx[0~6]:0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x \n", __FUNCTION__,
                __LINE__, pSharp->luma2table_idx[0], pSharp->luma2table_idx[1],
                pSharp->luma2table_idx[2], pSharp->luma2table_idx[3], pSharp->luma2table_idx[4],
                pSharp->luma2table_idx[5], pSharp->luma2table_idx[6]);

    for (int i = 0; i < ISP39_SHARP_Y_NUM; i++) pSharp->pbf_sigma_inv[i] = sharp.pbf_sigma_inv[i];
    LOGV_ASHARP("%s:%d  pbf_sigma_inv[0~7]:0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x \n",
                __FUNCTION__, __LINE__, pSharp->pbf_sigma_inv[0], pSharp->pbf_sigma_inv[1],
                pSharp->pbf_sigma_inv[2], pSharp->pbf_sigma_inv[3], pSharp->pbf_sigma_inv[4],
                pSharp->pbf_sigma_inv[5], pSharp->pbf_sigma_inv[6], pSharp->pbf_sigma_inv[7]);

    for (int i = 0; i < ISP39_SHARP_Y_NUM; i++) pSharp->bf_sigma_inv[i] = sharp.bf_sigma_inv[i];
    LOGV_ASHARP("%s:%d  bf_sigma_inv[0~7]:0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x \n", __FUNCTION__,
                __LINE__, pSharp->bf_sigma_inv[0], pSharp->bf_sigma_inv[1], pSharp->bf_sigma_inv[2],
                pSharp->bf_sigma_inv[3], pSharp->bf_sigma_inv[4], pSharp->bf_sigma_inv[5],
                pSharp->bf_sigma_inv[6], pSharp->bf_sigma_inv[7]);

    pSharp->bf_sigma_shift  = sharp.bf_sigma_shift;
    pSharp->pbf_sigma_shift = sharp.pbf_sigma_shift;
    LOGV_ASHARP("%s:%d  bf_sigma_shift:0x%x pbf_sigma_shift:0x%x\n", __FUNCTION__, __LINE__,
                pSharp->bf_sigma_shift, pSharp->pbf_sigma_shift);

    for (int i = 0; i < ISP39_SHARP_Y_NUM; i++) pSharp->luma2strg_val[i] = sharp.luma2strg_val[i];
    LOGV_ASHARP("%s:%d  luma2strg_val[0~7]:0x%x  0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x \n",
                __FUNCTION__, __LINE__, pSharp->luma2strg_val[0], pSharp->luma2strg_val[1],
                pSharp->luma2strg_val[2], pSharp->luma2strg_val[3], pSharp->luma2strg_val[4],
                pSharp->luma2strg_val[5], pSharp->luma2strg_val[6], pSharp->luma2strg_val[7]);

    for (int i = 0; i < ISP39_SHARP_Y_NUM; i++)
        pSharp->luma2posclip_val[i] = sharp.luma2posclip_val[i];
    LOGV_ASHARP("%s:%d  clip_hf[0~7]:0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x \n", __FUNCTION__,
                __LINE__, pSharp->luma2posclip_val[0], pSharp->luma2posclip_val[1],
                pSharp->luma2posclip_val[2], pSharp->luma2posclip_val[3],
                pSharp->luma2posclip_val[4], pSharp->luma2posclip_val[5],
                pSharp->luma2posclip_val[6], pSharp->luma2posclip_val[7]);

    pSharp->pbf_coef2 = sharp.pbf_coef2;
    pSharp->pbf_coef1 = sharp.pbf_coef1;
    pSharp->pbf_coef0 = sharp.pbf_coef0;

    pSharp->bf_coef2 = sharp.bf_coef2;
    pSharp->bf_coef1 = sharp.bf_coef1;
    pSharp->bf_coef0 = sharp.bf_coef0;
    LOGV_ASHARP("%s:%d  pbf_coef0~2:0x%x 0x%x 0x%x bf_coef0~2:0x%x 0x%x 0x%x\n", __FUNCTION__,
                __LINE__, pSharp->pbf_coef0, pSharp->pbf_coef1, pSharp->pbf_coef2, pSharp->bf_coef0,
                pSharp->bf_coef1, pSharp->bf_coef2);

    for (int i = 0; i < ISP39_SHARP_GAUS_COEF_NUM; i++)
        pSharp->img_lpf_coeff[i] = sharp.img_lpf_coeff[i];
    LOGV_ASHARP("%s:%d  img_lpf_coeff[0~5]:0x%x 0x%x 0x%x 0x%x 0x%x 0x%x \n", __FUNCTION__,
                __LINE__, pSharp->img_lpf_coeff[0], pSharp->img_lpf_coeff[1],
                pSharp->img_lpf_coeff[2], pSharp->img_lpf_coeff[3], pSharp->img_lpf_coeff[4],
                pSharp->img_lpf_coeff[5]);

    pSharp->global_gain      = sharp.global_gain;
    pSharp->gain_merge_alpha = sharp.gain_merge_alpha;
    pSharp->local_gain_scale = sharp.local_gain_scale;
    LOGV_ASHARP("%s:%d  global_gain:0x%x gain_merge_alpha:0x%x local_gain_scale:0x%x\n",
                __FUNCTION__, __LINE__, pSharp->global_gain, pSharp->gain_merge_alpha,
                pSharp->local_gain_scale);

    for (int i = 0; i < ISP39_SHARP_GAIN_ADJ_NUM; i++)
        pSharp->gain2strg_val[i] = sharp.gain2strg_val[i];
    LOGV_ASHARP(
        "%s:%d  gain2strg_val[0~13]:0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x "
        "0x%x "
        "0x%x\n",
        __FUNCTION__, __LINE__, pSharp->gain2strg_val[0], pSharp->gain2strg_val[1],
        pSharp->gain2strg_val[2], pSharp->gain2strg_val[3], pSharp->gain2strg_val[4],
        pSharp->gain2strg_val[5], pSharp->gain2strg_val[6], pSharp->gain2strg_val[7],
        pSharp->gain2strg_val[8], pSharp->gain2strg_val[9], pSharp->gain2strg_val[10],
        pSharp->gain2strg_val[11], pSharp->gain2strg_val[12], pSharp->gain2strg_val[13]);

    pSharp->center_x = sharp.center_x;
    pSharp->center_y = sharp.center_y;
    LOGV_ASHARP("%s:%d  center_x:0x%x center_y:0x%x\n", __FUNCTION__, __LINE__, pSharp->center_x,
                pSharp->center_y);

    for (int i = 0; i < ISP39_SHARP_STRENGTH_NUM; i++)
        pSharp->distance2strg_val[i] = sharp.distance2strg_val[i];
    LOGV_ASHARP(
        "%s:%d  distance2strg_val[0~21]:0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x "
        "0x%x 0x%x "
        "0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",
        __FUNCTION__, __LINE__, pSharp->distance2strg_val[0], pSharp->distance2strg_val[1],
        pSharp->distance2strg_val[2], pSharp->distance2strg_val[3], pSharp->distance2strg_val[4],
        pSharp->distance2strg_val[5], pSharp->distance2strg_val[6], pSharp->distance2strg_val[7],
        pSharp->distance2strg_val[8], pSharp->distance2strg_val[9], pSharp->distance2strg_val[10],
        pSharp->distance2strg_val[11], pSharp->distance2strg_val[12], pSharp->distance2strg_val[13],
        pSharp->distance2strg_val[14], pSharp->distance2strg_val[15], pSharp->distance2strg_val[16],
        pSharp->distance2strg_val[17], pSharp->distance2strg_val[18], pSharp->distance2strg_val[19],
        pSharp->distance2strg_val[20], pSharp->distance2strg_val[21]);

    for (int i = 0; i < ISP39_SHARP_Y_NUM; i++)
        pSharp->luma2neg_clip_val[i] = sharp.luma2neg_clip_val[i];
    LOGV_ASHARP("%s:%d  luma2neg_clip_val[0~7]:0x%x  0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x \n",
                __FUNCTION__, __LINE__, pSharp->luma2neg_clip_val[0], pSharp->luma2neg_clip_val[1],
                pSharp->luma2neg_clip_val[2], pSharp->luma2neg_clip_val[3],
                pSharp->luma2neg_clip_val[4], pSharp->luma2neg_clip_val[5],
                pSharp->luma2neg_clip_val[6], pSharp->luma2neg_clip_val[7]);

    pSharp->noise_max_limit   = sharp.noise_max_limit;
    pSharp->tex_reserve_level = sharp.tex_reserve_level;
    pSharp->tex_wet_scale     = sharp.tex_wet_scale;
    pSharp->noise_norm_bit    = sharp.noise_norm_bit;
    pSharp->tex_wgt_mode      = sharp.tex_wgt_mode;
    pSharp->noise_strg        = sharp.noise_strg;
    LOGV_ASHARP("%s:%d  noise_max_limit:0x%x tex_reserve_level:0x%x noise_strg:0x%x\n",
                __FUNCTION__, __LINE__, pSharp->noise_max_limit, pSharp->tex_reserve_level,
                pSharp->noise_strg);
    LOGV_ASHARP("%s:%d  tex_wet_scale:0x%x noise_norm_bit:0x%x tex_wgt_mode:0x%x\n", __FUNCTION__,
                __LINE__, pSharp->tex_wet_scale, pSharp->noise_norm_bit, pSharp->tex_wgt_mode);

    for (int i = 0; i < ISP39_SHARP_TEX_WET_LUT_NUM; i++)
        pSharp->tex2wgt_val[i] = sharp.tex2wgt_val[i];
    LOGV_ASHARP(
        "%s:%d  tex2wgt_val[0~16]:0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x "
        "0x%x 0x%x 0x%x 0x%x\n",
        __FUNCTION__, __LINE__, pSharp->tex2wgt_val[0], pSharp->tex2wgt_val[1],
        pSharp->tex2wgt_val[2], pSharp->tex2wgt_val[3], pSharp->tex2wgt_val[4],
        pSharp->tex2wgt_val[5], pSharp->tex2wgt_val[6], pSharp->tex2wgt_val[7],
        pSharp->tex2wgt_val[8], pSharp->tex2wgt_val[9], pSharp->tex2wgt_val[10],
        pSharp->tex2wgt_val[11], pSharp->tex2wgt_val[12], pSharp->tex2wgt_val[13],
        pSharp->tex2wgt_val[14], pSharp->tex2wgt_val[15], pSharp->tex2wgt_val[16]);

    for (int i = 0; i < ISP39_SHARP_DETAIL_STRG_NUM; i++)
        pSharp->detail2strg_val[i] = sharp.detail2strg_val[i];
    LOGV_ASHARP(
        "%s:%d  detail2strg_val[0~16]:0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x "
        "0x%x "
        "0x%x 0x%x 0x%x 0x%x\n",
        __FUNCTION__, __LINE__, pSharp->detail2strg_val[0], pSharp->detail2strg_val[1],
        pSharp->detail2strg_val[2], pSharp->detail2strg_val[3], pSharp->detail2strg_val[4],
        pSharp->detail2strg_val[5], pSharp->detail2strg_val[6], pSharp->detail2strg_val[7],
        pSharp->detail2strg_val[8], pSharp->detail2strg_val[9], pSharp->detail2strg_val[10],
        pSharp->detail2strg_val[11], pSharp->detail2strg_val[12], pSharp->detail2strg_val[13],
        pSharp->detail2strg_val[14], pSharp->detail2strg_val[15], pSharp->detail2strg_val[16]);

    LOG1_ASHARP("%s:%d exit!\n", __FUNCTION__, __LINE__);
}
#endif
#endif
#if USE_NEWSTRUCT
void Isp39Params::convertAiqBlcToIsp39Params(struct isp39_isp_params_cfg& isp_cfg,
        rk_aiq_isp_blc_params_t* blc_attr) {

    if (blc_attr->en) {
        isp_cfg.module_ens |= ISP2X_MODULE_BLS;
        isp_cfg.module_en_update |= ISP2X_MODULE_BLS;
        isp_cfg.module_cfg_update |= ISP2X_MODULE_BLS;
    } else {
        isp_cfg.module_ens &= ~ISP2X_MODULE_BLS;
        isp_cfg.module_en_update |= ISP2X_MODULE_BLS;
        return;
    }

    rk_aiq_blc30_params_cvt(&blc_attr->result, &isp_cfg, &mCommonCvtInfo);

}
#else
void Isp39Params::convertAiqBlcToIsp39Params(struct isp39_isp_params_cfg& isp_cfg,
        rk_aiq_isp_blc_v32_t& blc) {
    LOGD_ABLC("%s:(%d) enter enable:%d\n", __FUNCTION__, __LINE__, blc.enable);

    if (blc.enable) {
        isp_cfg.module_ens |= ISP3X_MODULE_BLS;
    } else {
        isp_cfg.module_ens &= ~ISP3X_MODULE_BLS;
    }
    isp_cfg.module_en_update |= ISP3X_MODULE_BLS;
    isp_cfg.module_cfg_update |= ISP3X_MODULE_BLS;

#if 1
    isp_cfg.others.bls_cfg.enable_auto = 0;
    isp_cfg.others.bls_cfg.en_windows  = 0;

    isp_cfg.others.bls_cfg.bls_window1.h_offs = 0;
    isp_cfg.others.bls_cfg.bls_window1.v_offs = 0;
    isp_cfg.others.bls_cfg.bls_window1.h_size = 0;
    isp_cfg.others.bls_cfg.bls_window1.v_size = 0;

    isp_cfg.others.bls_cfg.bls_window2.h_offs = 0;
    isp_cfg.others.bls_cfg.bls_window2.v_offs = 0;
    isp_cfg.others.bls_cfg.bls_window2.h_size = 0;
    isp_cfg.others.bls_cfg.bls_window2.v_size = 0;

    isp_cfg.others.bls_cfg.bls_samples = 0;

    // blc0
    isp_cfg.others.bls_cfg.fixed_val.r  = blc.blc_r;
    isp_cfg.others.bls_cfg.fixed_val.gr = blc.blc_gr;
    isp_cfg.others.bls_cfg.fixed_val.gb = blc.blc_gb;
    isp_cfg.others.bls_cfg.fixed_val.b  = blc.blc_b;

    if (blc.isp_ob_predgain != 0 ) {
        isp_cfg.others.bls_cfg.bls1_val.r  = CLIP((int)(blc.blc1_r * blc.isp_ob_predgain), 0, 32767);
        isp_cfg.others.bls_cfg.bls1_val.gr = CLIP((int)(blc.blc1_gr * blc.isp_ob_predgain), 0, 32767);
        isp_cfg.others.bls_cfg.bls1_val.gb = CLIP((int)(blc.blc1_gb * blc.isp_ob_predgain), 0, 32767);
        isp_cfg.others.bls_cfg.bls1_val.b  = CLIP((int)(blc.blc1_b * blc.isp_ob_predgain), 0, 32767);
    } else {
        isp_cfg.others.bls_cfg.bls1_val.r  = (int)blc.blc1_r;
        isp_cfg.others.bls_cfg.bls1_val.gr = (int)blc.blc1_gr;
        isp_cfg.others.bls_cfg.bls1_val.gb = (int)blc.blc1_gb;
        isp_cfg.others.bls_cfg.bls1_val.b  = (int)blc.blc1_b;
    }

    // TODO bls1 params
    isp_cfg.others.bls_cfg.bls1_en = blc.blc1_enable;

    // blc_ob

    isp_cfg.others.bls_cfg.isp_ob_offset   = CLIP((int)blc.isp_ob_offset, 0, 511);
    isp_cfg.others.bls_cfg.isp_ob_predgain = CLIP((int)(blc.isp_ob_predgain * (1 << 8)), 0, 65535);
    isp_cfg.others.bls_cfg.isp_ob_max      = CLIP(blc.isp_ob_max, 0, 1048575);

#if defined(ISP_HW_V39)
    mLatestBlsCfg = isp_cfg.others.bls_cfg;
#endif

    LOGD_ABLC("isp_ob_offset = 0x%x ,isp_ob_predgain = 0x%x, isp_ob_max = %x \n",
              isp_cfg.others.bls_cfg.isp_ob_offset, isp_cfg.others.bls_cfg.isp_ob_predgain,
              isp_cfg.others.bls_cfg.isp_ob_max);

#endif
    LOGD_ABLC("%s:(%d) exit \n", __FUNCTION__, __LINE__);
}
#endif

void Isp39Params::convertAiqAldchToIsp39Params(struct isp39_isp_params_cfg& isp_cfg,
        const rk_aiq_isp_ldch_v21_t& ldch_cfg)
{
    struct isp39_ldch_cfg *pLdchCfg = &isp_cfg.others.ldch_cfg;

    // TODO: add update flag for ldch
    if (ldch_cfg.base.sw_ldch_en) {
        isp_cfg.module_ens |= ISP39_MODULE_LDCH;
        isp_cfg.module_en_update |= ISP39_MODULE_LDCH;
        isp_cfg.module_cfg_update |= ISP39_MODULE_LDCH;

        pLdchCfg->hsize = ldch_cfg.base.lut_h_size;
        pLdchCfg->vsize = ldch_cfg.base.lut_v_size;
        pLdchCfg->buf_fd = ldch_cfg.base.lut_mapxy_buf_fd[0];

        pLdchCfg->frm_end_dis = ldch_cfg.frm_end_dis;
        pLdchCfg->sample_avr_en = ldch_cfg.sample_avr_en;
        pLdchCfg->bic_mode_en = ldch_cfg.bic_mode_en;
        memcpy(pLdchCfg->bicubic, ldch_cfg.bicubic, sizeof(ldch_cfg.bicubic));

        LOGV_CAMHW_SUBM(ISP20PARAM_SUBM, "enable ldch h/v size: %dx%d, buf_fd: %d",
                        pLdchCfg->hsize, pLdchCfg->vsize, pLdchCfg->buf_fd);

    } else {
        isp_cfg.module_ens &= ~ISP39_MODULE_LDCH;
        isp_cfg.module_en_update |= ISP39_MODULE_LDCH;
    }
}

void Isp39Params::convertAiqExpIspDgainToIsp39Params(struct isp39_isp_params_cfg& isp_cfg, RKAiqAecExpInfo_t ae_exp)
{
    // TODO
    struct isp32_awb_gain_cfg *  cfg = &mLatestWbGainCfg;
    struct isp32_awb_gain_cfg *  dest_cfg = &isp_cfg.others.awb_gain_cfg;
    uint16_t max_wb_gain = (1 << (ISP2X_WBGAIN_FIXSCALE_BIT + ISP3X_WBGAIN_INTSCALE_BIT)) - 1;

    if(_working_mode == RK_AIQ_WORKING_MODE_NORMAL) {

        float isp_dgain = MAX(1.0f, ae_exp.LinearExp.exp_real_params.isp_dgain);
        if (isp_dgain < 1.0000001f)
            return;

        dest_cfg->gain0_red = MIN(cfg->gain0_red * isp_dgain + 0.5, max_wb_gain);
        dest_cfg->gain0_green_r = MIN(cfg->gain0_green_r * isp_dgain + 0.5, max_wb_gain);
        dest_cfg->gain0_green_b = MIN(cfg->gain0_green_b * isp_dgain + 0.5, max_wb_gain);
        dest_cfg->gain0_blue = MIN(cfg->gain0_blue * isp_dgain + 0.5, max_wb_gain);

        dest_cfg->gain1_red = MIN(cfg->gain1_red * isp_dgain + 0.5, max_wb_gain);
        dest_cfg->gain1_green_r = MIN(cfg->gain1_green_r * isp_dgain + 0.5, max_wb_gain);
        dest_cfg->gain1_green_b = MIN(cfg->gain1_green_b * isp_dgain + 0.5, max_wb_gain);
        dest_cfg->gain1_blue = MIN(cfg->gain1_blue * isp_dgain + 0.5, max_wb_gain);

        dest_cfg->gain2_red = MIN(cfg->gain2_red * isp_dgain + 0.5, max_wb_gain);
        dest_cfg->gain2_green_r = MIN(cfg->gain2_green_r * isp_dgain + 0.5, max_wb_gain);
        dest_cfg->gain2_green_b = MIN(cfg->gain2_green_b * isp_dgain + 0.5, max_wb_gain);
        dest_cfg->gain2_blue = MIN(cfg->gain2_blue * isp_dgain + 0.5, max_wb_gain);
        isp_cfg.module_cfg_update |= 1LL << RK_ISP2X_AWB_GAIN_ID;

    } else {

        float isp_dgain0 = MAX(1.0f, ae_exp.HdrExp[0].exp_real_params.isp_dgain);
        float isp_dgain1 = MAX(1.0f, ae_exp.HdrExp[1].exp_real_params.isp_dgain);
        float isp_dgain2 = MAX(1.0f, ae_exp.HdrExp[2].exp_real_params.isp_dgain);
        if (isp_dgain0 < 1.0000001f &&
                isp_dgain1 < 1.0000001f &&
                isp_dgain2 < 1.0000001f )
            return;

        dest_cfg->gain0_red = MIN(cfg->gain0_red * isp_dgain0 + 0.5, max_wb_gain);
        dest_cfg->gain0_green_r = MIN(cfg->gain0_green_r * isp_dgain0 + 0.5, max_wb_gain);
        dest_cfg->gain0_green_b = MIN(cfg->gain0_green_b * isp_dgain0 + 0.5, max_wb_gain);
        dest_cfg->gain0_blue = MIN(cfg->gain0_blue * isp_dgain0 + 0.5, max_wb_gain);

        dest_cfg->gain1_red = MIN(cfg->gain1_red * isp_dgain1 + 0.5, max_wb_gain);
        dest_cfg->gain1_green_r = MIN(cfg->gain1_green_r * isp_dgain1 + 0.5, max_wb_gain);
        dest_cfg->gain1_green_b = MIN(cfg->gain1_green_b * isp_dgain1 + 0.5, max_wb_gain);
        dest_cfg->gain1_blue = MIN(cfg->gain1_blue * isp_dgain1 + 0.5, max_wb_gain);

        dest_cfg->gain2_red = MIN(cfg->gain2_red * isp_dgain2 + 0.5, max_wb_gain);
        dest_cfg->gain2_green_r = MIN(cfg->gain2_green_r * isp_dgain2 + 0.5, max_wb_gain);
        dest_cfg->gain2_green_b = MIN(cfg->gain2_green_b * isp_dgain2 + 0.5, max_wb_gain);
        dest_cfg->gain2_blue = MIN(cfg->gain2_blue * isp_dgain2 + 0.5, max_wb_gain);
        isp_cfg.module_cfg_update |= 1LL << RK_ISP2X_AWB_GAIN_ID;

    }

}

#if (RKAIQ_HAVE_YUVME_V1)
void Isp39Params::convertAiqYuvmeToIsp39Params(struct isp39_isp_params_cfg& isp_cfg,
        rk_aiq_isp_yuvme_v39_t& yuvme) {

    LOGD_ANR("%s:%d enter! enable:%d tnr_wgt0_en:%d \n",
             __FUNCTION__, __LINE__, !yuvme.bypass, yuvme.tnr_wgt0_en);

    if (yuvme.bypass) {
        isp_cfg.module_ens |= ISP39_MODULE_YUVME;
    } else {
        isp_cfg.module_ens |= ISP39_MODULE_YUVME;
    }

    isp_cfg.module_en_update |= ISP39_MODULE_YUVME;
    isp_cfg.module_cfg_update |= ISP39_MODULE_YUVME;

    struct isp39_yuvme_cfg* pYuvme = &isp_cfg.others.yuvme_cfg;

    pYuvme->bypass = yuvme.bypass;
    pYuvme->tnr_wgt0_en = yuvme.tnr_wgt0_en;

    pYuvme->global_nr_strg = yuvme.global_nr_strg;
    pYuvme->wgt_fact3 = yuvme.wgt_fact3;
    pYuvme->search_range_mode = yuvme.search_range_mode;
    pYuvme->static_detect_thred = yuvme.static_detect_thred;

    pYuvme->time_relevance_offset = yuvme.time_relevance_offset;
    pYuvme->space_relevance_offset = yuvme.space_relevance_offset;
    pYuvme->nr_diff_scale = yuvme.nr_diff_scale;
    pYuvme->nr_fusion_limit = yuvme.nr_fusion_limit;

    pYuvme->nr_static_scale = yuvme.nr_static_scale;
    pYuvme->nr_motion_scale = yuvme.nr_motion_scale;

    pYuvme->nr_fusion_mode = yuvme.nr_fusion_mode;
    pYuvme->cur_weight_limit = yuvme.cur_weight_limit;

    for(int i = 0; i < ISP39_YUVME_SIGMA_NUM; i++) {
        pYuvme->nr_luma2sigma_val[i] = yuvme.nr_luma2sigma_val[i];
    }


}
#endif

#if RKAIQ_HAVE_DPCC_V2
#ifdef USE_NEWSTRUCT
void Isp39Params::convertAiqDpccToIsp39Params(struct isp39_isp_params_cfg& isp_cfg,
                                              rk_aiq_isp_dpcc_params_t* dpcc_attr){
    // hwi_api_dpcc20_params_check(&dpcc_attr->result);

    if (dpcc_attr->en) {
        isp_cfg.module_ens |= ISP2X_MODULE_DPCC;
        isp_cfg.module_en_update |= ISP2X_MODULE_DPCC;
        isp_cfg.module_cfg_update |= ISP2X_MODULE_DPCC;
    } else {
        isp_cfg.module_ens &= ~ISP2X_MODULE_DPCC;
        isp_cfg.module_en_update |= ISP2X_MODULE_DPCC;
        return;
    }

    rk_aiq_dpcc21_params_cvt(&dpcc_attr->result, &isp_cfg);
    // hwi_api_dpcc20_params_cvt(&dpcc_attr->result, &isp_cfg);
    // hwi_api_dpcc20_params_dump(&dpcc_attr->result, &isp_cfg.others.dpcc_cfg);

}
#endif
#endif

bool Isp39Params::convert3aResultsToIspCfg(SmartPtr<cam3aResult>& result, void* isp_cfg_p, bool is_multi_isp) {
    struct isp39_isp_params_cfg& isp_cfg       = *(struct isp39_isp_params_cfg*)isp_cfg_p;

    if (result.ptr() == NULL) {
        LOGE_CAMHW_SUBM(ISP20PARAM_SUBM, "3A result empty");
        return false;
    }
    int32_t type = result->getType();
    //LOGE_CAMHW_SUBM(ISP20PARAM_SUBM, "%s, module (0x%x) convert params!\n", __FUNCTION__, type);
    switch (type) {
    case RESULT_TYPE_EXPOSURE_PARAM:
    {
        RkAiqSensorExpParamsProxy* expParams =
            result.get_cast_ptr<RkAiqSensorExpParamsProxy>();
        if (expParams)
            convertAiqExpIspDgainToIsp39Params(isp_cfg,
                                               expParams->data()->aecExpInfo);
    }
    break;
    case RESULT_TYPE_AWBGAIN_PARAM: {
        RkAiqIspAwbGainParamsProxy* awb_gain =
            result.get_cast_ptr<RkAiqIspAwbGainParamsProxy>();
        if (awb_gain) {
            convertAiqAwbGainToIsp39Params(isp_cfg, awb_gain->data()->result, true);
        }
    }
    break;
    case RESULT_TYPE_AWB_PARAM: {
#if RKAIQ_HAVE_AWB_V32
        mAwbParams = result.ptr();
        RkAiqIspAwbParamsProxy* params =
            result.get_cast_ptr<RkAiqIspAwbParamsProxy>();

        if (params) convertAiqAwbToIsp39Params(isp_cfg, params->data()->result, true);
#endif
    }
    break;
    case RESULT_TYPE_GIC_PARAM: {
#if RKAIQ_HAVE_GIC_V2
        RkAiqIspGicParamsProxy* params =
            result.get_cast_ptr<RkAiqIspGicParamsProxy>();
        if (params) convertAiqAgicToIsp21Params(isp_cfg, params->data()->result);
#endif
    }
    break;
    case RESULT_TYPE_LSC_PARAM: {
#if RKAIQ_HAVE_LSC_V3
        RkAiqIspLscParamsProxy* params =
            result.get_cast_ptr<RkAiqIspLscParamsProxy>();
        if (params) convertAiqLscToIsp20Params(isp_cfg, params->data()->result);
#endif
    }
    break;
    case RESULT_TYPE_AF_PARAM: {
#if RKAIQ_HAVE_AF_V33 || RKAIQ_ONLY_AF_STATS_V33
        RkAiqIspAfParamsProxy* params =
            result.get_cast_ptr<RkAiqIspAfParamsProxy>();
        if (params) convertAiqAfToIsp39Params(isp_cfg, params->data()->result, true);
#endif
    }
    break;
    case RESULT_TYPE_CCM_PARAM: {
#if RKAIQ_HAVE_CCM_V3
        RkAiqIspCcmParamsProxy* params =
            result.get_cast_ptr<RkAiqIspCcmParamsProxy>();
        if (params) convertAiqCcmToIsp39Params(isp_cfg, params->data()->result);
#endif
    }
    break;
    case RESULT_TYPE_CAC_PARAM: {
#if RKAIQ_HAVE_CAC_V11
        RkAiqIspCacParamsProxy* params =
            result.get_cast_ptr<RkAiqIspCacParamsProxy>();
        if (params)
            convertAiqCacToIsp39Params(isp_cfg, params->data()->result);
#endif
    }
    break;
    case RESULT_TYPE_DEBAYER_PARAM: {
#if RKAIQ_HAVE_DEBAYER_V3
        RkAiqIspDebayerParamsProxy* params =
            result.get_cast_ptr<RkAiqIspDebayerParamsProxy>();
        if (params) {
            convertAiqAdebayerToIsp39Params(isp_cfg, params->data()->result);
        }
#endif
    }
    break;
    case RESULT_TYPE_AEC_PARAM: {
#ifdef ISP_HW_V39
        RkAiqIspAecParamsProxy* params =
            result.get_cast_ptr<RkAiqIspAecParamsProxy>();
        if (params) {
            convertAiqAeToIsp20Params(isp_cfg, params->data()->result);
        }
#endif
    }
    break;
    case RESULT_TYPE_HIST_PARAM: {
#ifdef ISP_HW_V39
        RkAiqIspHistParamsProxy* params =
            result.get_cast_ptr<RkAiqIspHistParamsProxy>();
        if (params) convertAiqHistToIsp20Params(isp_cfg, params->data()->result);
#endif
    }
    break;
    case RESULT_TYPE_AGAMMA_PARAM: {
#if RKAIQ_HAVE_GAMMA_V11
        RkAiqIspAgammaParamsProxy* params =
            result.get_cast_ptr<RkAiqIspAgammaParamsProxy>();
        if (params) convertAiqAgammaToIsp3xParams(isp_cfg, params->data()->result);
#endif
    }
    break;
    case RESULT_TYPE_MERGE_PARAM: {
#if RKAIQ_HAVE_MERGE_V12
        RkAiqIspMergeParamsProxy* params =
            result.get_cast_ptr<RkAiqIspMergeParamsProxy>();
        if (params) convertAiqMergeToIsp39Params(isp_cfg, params->data()->result);
#endif
    }
    break;
    case RESULT_TYPE_DEHAZE_PARAM: {
#if RKAIQ_HAVE_DEHAZE_V14
#ifdef USE_NEWSTRUCT
        RkAiqIspDehazeParamsProxy* params =
            result.get_cast_ptr<RkAiqIspDehazeParamsProxy>();
        if (params) convertAiqDehazeToIsp39Params(isp_cfg, params->data().ptr());
#else
        RkAiqIspDehazeParamsProxy* params =
            result.get_cast_ptr<RkAiqIspDehazeParamsProxy>();
        if (params) convertAiqAdehazeToIsp39Params(isp_cfg, params->data()->result);
#endif
#endif
    }
    break;
    case RESULT_TYPE_DRC_PARAM: {
#if RKAIQ_HAVE_DRC_V20
#ifdef USE_NEWSTRUCT
        RkAiqIspDrcParamsProxy* params = result.get_cast_ptr<RkAiqIspDrcParamsProxy>();
        if (params) convertAiqDrcToIsp39Params(isp_cfg, params->data().ptr());
#else
        RkAiqIspDrcParamsProxy* params = result.get_cast_ptr<RkAiqIspDrcParamsProxy>();
        if (params) convertAiqDrcToIsp39Params(isp_cfg, params->data()->result);
#endif
#endif
    }
    break;
    case RESULT_TYPE_LUT3D_PARAM: {
#if RKAIQ_HAVE_3DLUT_V1
        RkAiqIspLut3dParamsProxy* params =
            result.get_cast_ptr<RkAiqIspLut3dParamsProxy>();
        if (params) convertAiqA3dlutToIsp20Params(isp_cfg, params->data()->result);
#endif
    }
    break;
#if RKAIQ_HAVE_DPCC_V2
    case RESULT_TYPE_DPCC_PARAM: {
#ifdef USE_NEWSTRUCT
        RkAiqIspDpccParamsProxy* params = result.get_cast_ptr<RkAiqIspDpccParamsProxy>();
        if (params) convertAiqDpccToIsp39Params(isp_cfg, params->data().ptr());
#else
        RkAiqIspDpccParamsProxy* params =
            result.get_cast_ptr<RkAiqIspDpccParamsProxy>();
        if (params) convertAiqDpccToIsp20Params(isp_cfg, params->data()->result);
#endif
    }
    break;
#endif
    case RESULT_TYPE_CSM_PARAM: {
#ifdef USE_NEWSTRUCT
#else
        RkAiqIspCsmParamsProxy* params =
            result.get_cast_ptr<RkAiqIspCsmParamsProxy>();
        if (params) convertAiqCsmToIsp21Params(isp_cfg, params->data()->result);
#endif
    }
    break;
    case RESULT_TYPE_RAWNR_PARAM: {
#if RKAIQ_HAVE_BAYER2DNR_V23
        RkAiqIspBaynrParamsProxy* params =
            result.get_cast_ptr<RkAiqIspBaynrParamsProxy>();
        if (params) convertAiqRawnrToIsp39Params(isp_cfg, params->data()->result);
#endif
    }
    break;
    case RESULT_TYPE_TNR_PARAM: {
#if (RKAIQ_HAVE_BAYERTNR_V30)
#ifdef USE_NEWSTRUCT
        RkAiqIspBtnrParamsProxy* params =
            result.get_cast_ptr<RkAiqIspBtnrParamsProxy>();
        if (params) convertAiqBtnrToIsp39Params(isp_cfg, params->data().ptr());
#else
        RkAiqIspTnrParamsProxy* params =
            result.get_cast_ptr<RkAiqIspTnrParamsProxy>();
        if (params) convertAiqTnrToIsp39Params(isp_cfg, params->data()->result);
#endif
#endif
    }
    break;
    case RESULT_TYPE_YNR_PARAM: {
#if RKAIQ_HAVE_YNR_V24
#ifdef USE_NEWSTRUCT
        RkAiqIspYnrParamsProxy* params =
            result.get_cast_ptr<RkAiqIspYnrParamsProxy>();
        if (params) convertAiqYnrToIsp39Params(isp_cfg, params->data().ptr());
#else
        RkAiqIspYnrParamsProxy* params =
            result.get_cast_ptr<RkAiqIspYnrParamsProxy>();
        if (params) convertAiqYnrToIsp39Params(isp_cfg, params->data()->result);
#endif
#endif
    }
    break;
    case RESULT_TYPE_UVNR_PARAM: {
#if (RKAIQ_HAVE_CNR_V31)
#ifdef USE_NEWSTRUCT
        RkAiqIspCnrParamsProxy* params =
            result.get_cast_ptr<RkAiqIspCnrParamsProxy>();
        if (params) convertAiqCnrToIsp39Params(isp_cfg, params->data().ptr());
#else
        RkAiqIspCnrParamsProxy* params =
            result.get_cast_ptr<RkAiqIspCnrParamsProxy>();
        if (params) convertAiqUvnrToIsp39Params(isp_cfg, params->data()->result);
#endif
#endif
    }
    break;
    case RESULT_TYPE_BLC_PARAM:
    {
#if USE_NEWSTRUCT
        RkAiqIspBlcParamsProxy* params =
            result.get_cast_ptr<RkAiqIspBlcParamsProxy>();
        if (params) convertAiqBlcToIsp39Params(isp_cfg, params->data().ptr());
#else
        RkAiqIspBlcParamsProxy* params =
            result.get_cast_ptr<RkAiqIspBlcParamsProxy>();
        if (params) convertAiqBlcToIsp39Params(isp_cfg, params->data()->result);
#endif
    }
    break;
    case RESULT_TYPE_GAIN_PARAM: {
        RkAiqIspGainParamsProxy* params =
            result.get_cast_ptr<RkAiqIspGainParamsProxy>();
        if (params) convertAiqGainToIsp3xParams(isp_cfg, params->data()->result);
    }
    break;
    case RESULT_TYPE_SHARPEN_PARAM: {
#if (RKAIQ_HAVE_SHARP_V34)
#ifdef USE_NEWSTRUCT
        RkAiqIspSharpParamsProxy* params =
            result.get_cast_ptr<RkAiqIspSharpParamsProxy>();
        if (params) convertAiqSharpToIsp39Params(isp_cfg, params->data().ptr());
#else
        RkAiqIspSharpenParamsProxy* params =
            result.get_cast_ptr<RkAiqIspSharpenParamsProxy>();
        if (params) convertAiqSharpenToIsp39Params(isp_cfg, params->data()->result);
#endif
#endif
    }
    break;
    case RESULT_TYPE_CGC_PARAM: {
#if RKAIQ_HAVE_CGC_V1
        RkAiqIspCgcParamsProxy* params =
            result.get_cast_ptr<RkAiqIspCgcParamsProxy>();
        if (params) convertAiqCgcToIsp21Params(isp_cfg, params->data()->result);
#endif
    }
    break;
    case RESULT_TYPE_CP_PARAM: {
#if RKAIQ_HAVE_ACP_V10
        RkAiqIspCpParamsProxy* params =
            result.get_cast_ptr<RkAiqIspCpParamsProxy>();
        if (params) convertAiqCpToIsp20Params(isp_cfg, params->data()->result);
#endif
    }
    break;
    case RESULT_TYPE_LDCH_PARAM:
    {
        RkAiqIspLdchParamsProxy* params = result.get_cast_ptr<RkAiqIspLdchParamsProxy>();
        if (params)
            convertAiqAldchToIsp39Params(isp_cfg, params->data()->result);
    }
    break;
    case RESULT_TYPE_MOTION_PARAM: {
#if RKAIQ_HAVE_YUVME_V1
        RkAiqIspYuvmeParamsProxy* params = result.get_cast_ptr<RkAiqIspYuvmeParamsProxy>();
        if (params) convertAiqYuvmeToIsp39Params(isp_cfg, params->data()->result);
#endif
    }
    break;
    case RESULT_TYPE_RGBIR_PARAM: {
#if RKAIQ_HAVE_RGBIR_REMOSAIC_V10
        RkAiqIspRgbirParamsProxy* params = result.get_cast_ptr<RkAiqIspRgbirParamsProxy>();
        if (params) convertAiqRgbirToIsp39Params(isp_cfg, params->data()->result);
#endif
    }
    break;
    default:
        LOGE("unknown param type: 0x%x!", type);
        return false;
    }

    return true;
}

}  // namespace RkCam
