/*
 * Copyright (c) 2021-2022 Rockchip Eletronics Co., Ltd.
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
#include "Isp32Params.h"

namespace RkCam {

#define ISP2X_WBGAIN_FIXSCALE_BIT  8
#define ISP3X_WBGAIN_INTSCALE_BIT  8

#if RKAIQ_HAVE_AWB_V32 || RKAIQ_HAVE_AWB_V32LT
void Isp32Params::convertAiqAwbGainToIsp32Params(struct isp32_isp_params_cfg& isp_cfg,
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
#endif

#if RKAIQ_HAVE_AWB_V32
static void WriteAwbReg(struct isp32_rawawb_meas_cfg* awb_cfg_v32)
{
#if 0
    char fName[100];
    sprintf(fName, "./tmp/awb_reg.txt");
    LOGE_AWB( "%s", fName);

    FILE* fp = fopen(fName, "wb");
    if(fp == NULL){
        return;
    }
    //fprintf(fp, "\t\tsw_rawawb_en = 0x%0x (%d)\n",                              awb_cfg_v32->en                            ,awb_cfg_v32->en);
    fprintf(fp, "\t\tsw_rawawb_uv_en0 = 0x%0x (%d)\n",                            awb_cfg_v32->uv_en0                        ,awb_cfg_v32->uv_en0);
    fprintf(fp, "\t\tsw_rawawb_xy_en0 = 0x%0x (%d)\n",                            awb_cfg_v32->xy_en0                        ,awb_cfg_v32->xy_en0);
    fprintf(fp, "\t\tsw_rawawb_yuv3d_en0 = 0x%0x (%d)\n",                         awb_cfg_v32->yuv3d_en0                     ,awb_cfg_v32->yuv3d_en0);
    fprintf(fp, "\t\tsw_rawawb_yuv3d_ls_idx0 = 0x%0x (%d)\n",                     awb_cfg_v32->yuv3d_ls_idx0                 ,awb_cfg_v32->yuv3d_ls_idx0);
    fprintf(fp, "\t\tsw_rawawb_yuv3d_ls_idx1 = 0x%0x (%d)\n",                     awb_cfg_v32->yuv3d_ls_idx1                 ,awb_cfg_v32->yuv3d_ls_idx1);
    fprintf(fp, "\t\tsw_rawawb_yuv3d_ls_idx2 = 0x%0x (%d)\n",                     awb_cfg_v32->yuv3d_ls_idx2                 ,awb_cfg_v32->yuv3d_ls_idx2);
    fprintf(fp, "\t\tsw_rawawb_yuv3d_ls_idx3 = 0x%0x (%d)\n",                     awb_cfg_v32->yuv3d_ls_idx3                 ,awb_cfg_v32->yuv3d_ls_idx3);
    fprintf(fp, "\t\tsw_rawawb_in_rshift_to_12bit_en = 0x%0x (%d)\n",             awb_cfg_v32->in_rshift_to_12bit_en         ,awb_cfg_v32->in_rshift_to_12bit_en);
    fprintf(fp, "\t\tsw_rawawb_in_overexposure_check_en = 0x%0x (%d)\n",          awb_cfg_v32->in_overexposure_check_en      ,awb_cfg_v32->in_overexposure_check_en);
    fprintf(fp, "\t\tsw_rawawb_wind_size = 0x%0x (%d)\n",                         awb_cfg_v32->wind_size                     ,awb_cfg_v32->wind_size);
    fprintf(fp, "\t\tsw_rawlsc_bypass_en = 0x%0x (%d)\n",                         awb_cfg_v32->rawlsc_bypass_en                     ,awb_cfg_v32->rawlsc_bypass_en);
    fprintf(fp, "\t\tsw_rawawb_light_num = 0x%0x (%d)\n",                         awb_cfg_v32->light_num                     ,awb_cfg_v32->light_num);
    //fprintf(fp, "\t\tsw_rawawb_2ddr_path_en = 0x%0x (%d)\n",                      awb_cfg_v32->2ddr_path_en                  ,awb_cfg_v32->2ddr_path_en);
    fprintf(fp, "\t\tsw_rawawb_uv_en1 = 0x%0x (%d)\n",                            awb_cfg_v32->uv_en1                        ,awb_cfg_v32->uv_en1);
    fprintf(fp, "\t\tsw_rawawb_xy_en1 = 0x%0x (%d)\n",                            awb_cfg_v32->xy_en1                        ,awb_cfg_v32->xy_en1);
    fprintf(fp, "\t\tsw_rawawb_yuv3d_en1 = 0x%0x (%d)\n",                         awb_cfg_v32->yuv3d_en1                     ,awb_cfg_v32->yuv3d_en1);
    //fprintf(fp, "\t\tsw_rawawb_2ddr_path_sel = 0x%0x (%d)\n",                     awb_cfg_v32->2ddr_path_sel                 ,awb_cfg_v32->2ddr_path_sel);
    fprintf(fp, "\t\tsw_rawawbin_low12bit_val = 0x%0x (%d)\n",                    awb_cfg_v32->low12bit_val                ,awb_cfg_v32->low12bit_val);
    fprintf(fp, "\t\tsw_rawawb_blk_measure_en = 0x%0x (%d)\n",                    awb_cfg_v32->blk_measure_enable                ,awb_cfg_v32->blk_measure_enable);
    fprintf(fp, "\t\tsw_rawawb_blk_measure_mode = 0x%0x (%d)\n",                  awb_cfg_v32->blk_measure_mode              ,awb_cfg_v32->blk_measure_mode);
    fprintf(fp, "\t\tsw_rawawb_blk_measure_xytype = 0x%0x (%d)\n",                awb_cfg_v32->blk_measure_xytype            ,awb_cfg_v32->blk_measure_xytype);
    fprintf(fp, "\t\tsw_rawawb_blk_rtdw_measure_en = 0x%0x (%d)\n",               awb_cfg_v32->blk_rtdw_measure_en           ,awb_cfg_v32->blk_rtdw_measure_en);
    fprintf(fp, "\t\tsw_rawawb_blk_measure_illu_idx = 0x%0x (%d)\n",              awb_cfg_v32->blk_measure_illu_idx          ,awb_cfg_v32->blk_measure_illu_idx);
    fprintf(fp, "\t\tsw_rawawb_ds16x8_mode_en = 0x%0x (%d)\n",                    awb_cfg_v32->ds16x8_mode_en                ,awb_cfg_v32->ds16x8_mode_en);
    fprintf(fp, "\t\tsw_rawawb_blk_with_luma_wei_en = 0x%0x (%d)\n",              awb_cfg_v32->blk_with_luma_wei_en          ,awb_cfg_v32->blk_with_luma_wei_en);
    fprintf(fp, "\t\tsw_rawawb_in_overexposure_threshold = 0x%0x (%d)\n",         awb_cfg_v32->in_overexposure_threshold     ,awb_cfg_v32->in_overexposure_threshold);
    fprintf(fp, "\t\tsw_rawawb_h_offs = 0x%0x (%d)\n",                            awb_cfg_v32->h_offs                        ,awb_cfg_v32->h_offs);
    fprintf(fp, "\t\tsw_rawawb_v_offs = 0x%0x (%d)\n",                            awb_cfg_v32->v_offs                        ,awb_cfg_v32->v_offs);
    fprintf(fp, "\t\tsw_rawawb_h_size = 0x%0x (%d)\n",                            awb_cfg_v32->h_size                        ,awb_cfg_v32->h_size);
    fprintf(fp, "\t\tsw_rawawb_v_size = 0x%0x (%d)\n",                            awb_cfg_v32->v_size                        ,awb_cfg_v32->v_size);
    fprintf(fp, "\t\tsw_rawawb_r_max = 0x%0x (%d)\n",                             awb_cfg_v32->r_max                         ,awb_cfg_v32->r_max);
    fprintf(fp, "\t\tsw_rawawb_g_max = 0x%0x (%d)\n",                             awb_cfg_v32->g_max                         ,awb_cfg_v32->g_max);
    fprintf(fp, "\t\tsw_rawawb_b_max = 0x%0x (%d)\n",                             awb_cfg_v32->b_max                         ,awb_cfg_v32->b_max);
    fprintf(fp, "\t\tsw_rawawb_y_max = 0x%0x (%d)\n",                             awb_cfg_v32->y_max                         ,awb_cfg_v32->y_max);
    fprintf(fp, "\t\tsw_rawawb_r_min = 0x%0x (%d)\n",                             awb_cfg_v32->r_min                         ,awb_cfg_v32->r_min);
    fprintf(fp, "\t\tsw_rawawb_g_min = 0x%0x (%d)\n",                             awb_cfg_v32->g_min                         ,awb_cfg_v32->g_min);
    fprintf(fp, "\t\tsw_rawawb_b_min = 0x%0x (%d)\n",                             awb_cfg_v32->b_min                         ,awb_cfg_v32->b_min);
    fprintf(fp, "\t\tsw_rawawb_y_min = 0x%0x (%d)\n",                             awb_cfg_v32->y_min                         ,awb_cfg_v32->y_min);
    fprintf(fp, "\t\tsw_rawawb_wp_luma_wei_en0 = 0x%0x (%d)\n",                   awb_cfg_v32->wp_luma_wei_en0               ,awb_cfg_v32->wp_luma_wei_en0);
    fprintf(fp, "\t\tsw_rawawb_wp_luma_wei_en1 = 0x%0x (%d)\n",                   awb_cfg_v32->wp_luma_wei_en1               ,awb_cfg_v32->wp_luma_wei_en1);
    fprintf(fp, "\t\tsw_rawawb_wp_blk_wei_en0 = 0x%0x (%d)\n",                    awb_cfg_v32->wp_blk_wei_en0                ,awb_cfg_v32->wp_blk_wei_en0);
    fprintf(fp, "\t\tsw_rawawb_wp_blk_wei_en1 = 0x%0x (%d)\n",                    awb_cfg_v32->wp_blk_wei_en1                ,awb_cfg_v32->wp_blk_wei_en1);
    fprintf(fp, "\t\tsw_rawawb_wp_hist_xytype = 0x%0x (%d)\n",                    awb_cfg_v32->wp_hist_xytype                ,awb_cfg_v32->wp_hist_xytype);
    fprintf(fp, "\t\tsw_rawawb_wp_luma_weicurve_y0 = 0x%0x (%d)\n",               awb_cfg_v32->wp_luma_weicurve_y0           ,awb_cfg_v32->wp_luma_weicurve_y0);
    fprintf(fp, "\t\tsw_rawawb_wp_luma_weicurve_y1 = 0x%0x (%d)\n",               awb_cfg_v32->wp_luma_weicurve_y1           ,awb_cfg_v32->wp_luma_weicurve_y1);
    fprintf(fp, "\t\tsw_rawawb_wp_luma_weicurve_y2 = 0x%0x (%d)\n",               awb_cfg_v32->wp_luma_weicurve_y2           ,awb_cfg_v32->wp_luma_weicurve_y2);
    fprintf(fp, "\t\tsw_rawawb_wp_luma_weicurve_y3 = 0x%0x (%d)\n",               awb_cfg_v32->wp_luma_weicurve_y3           ,awb_cfg_v32->wp_luma_weicurve_y3);
    fprintf(fp, "\t\tsw_rawawb_wp_luma_weicurve_y4 = 0x%0x (%d)\n",               awb_cfg_v32->wp_luma_weicurve_y4           ,awb_cfg_v32->wp_luma_weicurve_y4);
    fprintf(fp, "\t\tsw_rawawb_wp_luma_weicurve_y5 = 0x%0x (%d)\n",               awb_cfg_v32->wp_luma_weicurve_y5           ,awb_cfg_v32->wp_luma_weicurve_y5);
    fprintf(fp, "\t\tsw_rawawb_wp_luma_weicurve_y6 = 0x%0x (%d)\n",               awb_cfg_v32->wp_luma_weicurve_y6           ,awb_cfg_v32->wp_luma_weicurve_y6);
    fprintf(fp, "\t\tsw_rawawb_wp_luma_weicurve_y7 = 0x%0x (%d)\n",               awb_cfg_v32->wp_luma_weicurve_y7           ,awb_cfg_v32->wp_luma_weicurve_y7);
    fprintf(fp, "\t\tsw_rawawb_wp_luma_weicurve_y8 = 0x%0x (%d)\n",               awb_cfg_v32->wp_luma_weicurve_y8           ,awb_cfg_v32->wp_luma_weicurve_y8);
    fprintf(fp, "\t\tsw_rawawb_wp_luma_weicurve_w0 = 0x%0x (%d)\n",               awb_cfg_v32->wp_luma_weicurve_w0           ,awb_cfg_v32->wp_luma_weicurve_w0);
    fprintf(fp, "\t\tsw_rawawb_wp_luma_weicurve_w1 = 0x%0x (%d)\n",               awb_cfg_v32->wp_luma_weicurve_w1           ,awb_cfg_v32->wp_luma_weicurve_w1);
    fprintf(fp, "\t\tsw_rawawb_wp_luma_weicurve_w2 = 0x%0x (%d)\n",               awb_cfg_v32->wp_luma_weicurve_w2           ,awb_cfg_v32->wp_luma_weicurve_w2);
    fprintf(fp, "\t\tsw_rawawb_wp_luma_weicurve_w3 = 0x%0x (%d)\n",               awb_cfg_v32->wp_luma_weicurve_w3           ,awb_cfg_v32->wp_luma_weicurve_w3);
    fprintf(fp, "\t\tsw_rawawb_wp_luma_weicurve_w4 = 0x%0x (%d)\n",               awb_cfg_v32->wp_luma_weicurve_w4           ,awb_cfg_v32->wp_luma_weicurve_w4);
    fprintf(fp, "\t\tsw_rawawb_wp_luma_weicurve_w5 = 0x%0x (%d)\n",               awb_cfg_v32->wp_luma_weicurve_w5           ,awb_cfg_v32->wp_luma_weicurve_w5);
    fprintf(fp, "\t\tsw_rawawb_wp_luma_weicurve_w6 = 0x%0x (%d)\n",               awb_cfg_v32->wp_luma_weicurve_w6           ,awb_cfg_v32->wp_luma_weicurve_w6);
    fprintf(fp, "\t\tsw_rawawb_wp_luma_weicurve_w7 = 0x%0x (%d)\n",               awb_cfg_v32->wp_luma_weicurve_w7           ,awb_cfg_v32->wp_luma_weicurve_w7);
    fprintf(fp, "\t\tsw_rawawb_wp_luma_weicurve_w8 = 0x%0x (%d)\n",               awb_cfg_v32->wp_luma_weicurve_w8           ,awb_cfg_v32->wp_luma_weicurve_w8);
    fprintf(fp, "\t\tsw_rawawb_pre_wbgain_inv_r = 0x%0x (%d)\n",                  awb_cfg_v32->pre_wbgain_inv_r              ,awb_cfg_v32->pre_wbgain_inv_r);
    fprintf(fp, "\t\tsw_rawawb_pre_wbgain_inv_g = 0x%0x (%d)\n",                  awb_cfg_v32->pre_wbgain_inv_g              ,awb_cfg_v32->pre_wbgain_inv_g);
    fprintf(fp, "\t\tsw_rawawb_pre_wbgain_inv_b = 0x%0x (%d)\n",                  awb_cfg_v32->pre_wbgain_inv_b              ,awb_cfg_v32->pre_wbgain_inv_b);
    fprintf(fp, "\t\tsw_rawawb_vertex0_u_0 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex0_u_0                   ,awb_cfg_v32->vertex0_u_0);
    fprintf(fp, "\t\tsw_rawawb_vertex0_v_0 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex0_v_0                   ,awb_cfg_v32->vertex0_v_0);
    fprintf(fp, "\t\tsw_rawawb_vertex1_u_0 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex1_u_0                   ,awb_cfg_v32->vertex1_u_0);
    fprintf(fp, "\t\tsw_rawawb_vertex1_v_0 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex1_v_0                   ,awb_cfg_v32->vertex1_v_0);
    fprintf(fp, "\t\tsw_rawawb_vertex2_u_0 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex2_u_0                   ,awb_cfg_v32->vertex2_u_0);
    fprintf(fp, "\t\tsw_rawawb_vertex2_v_0 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex2_v_0                   ,awb_cfg_v32->vertex2_v_0);
    fprintf(fp, "\t\tsw_rawawb_vertex3_u_0 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex3_u_0                   ,awb_cfg_v32->vertex3_u_0);
    fprintf(fp, "\t\tsw_rawawb_vertex3_v_0 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex3_v_0                   ,awb_cfg_v32->vertex3_v_0);
    fprintf(fp, "\t\tsw_rawawb_islope01_0 = 0x%0x (%d)\n",                        awb_cfg_v32->islope01_0                    ,awb_cfg_v32->islope01_0);
    fprintf(fp, "\t\tsw_rawawb_islope12_0 = 0x%0x (%d)\n",                        awb_cfg_v32->islope12_0                    ,awb_cfg_v32->islope12_0);
    fprintf(fp, "\t\tsw_rawawb_islope23_0 = 0x%0x (%d)\n",                        awb_cfg_v32->islope23_0                    ,awb_cfg_v32->islope23_0);
    fprintf(fp, "\t\tsw_rawawb_islope30_0 = 0x%0x (%d)\n",                        awb_cfg_v32->islope30_0                    ,awb_cfg_v32->islope30_0);
    fprintf(fp, "\t\tsw_rawawb_vertex0_u_1 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex0_u_1                   ,awb_cfg_v32->vertex0_u_1);
    fprintf(fp, "\t\tsw_rawawb_vertex0_v_1 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex0_v_1                   ,awb_cfg_v32->vertex0_v_1);
    fprintf(fp, "\t\tsw_rawawb_vertex1_u_1 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex1_u_1                   ,awb_cfg_v32->vertex1_u_1);
    fprintf(fp, "\t\tsw_rawawb_vertex1_v_1 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex1_v_1                   ,awb_cfg_v32->vertex1_v_1);
    fprintf(fp, "\t\tsw_rawawb_vertex2_u_1 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex2_u_1                   ,awb_cfg_v32->vertex2_u_1);
    fprintf(fp, "\t\tsw_rawawb_vertex2_v_1 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex2_v_1                   ,awb_cfg_v32->vertex2_v_1);
    fprintf(fp, "\t\tsw_rawawb_vertex3_u_1 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex3_u_1                   ,awb_cfg_v32->vertex3_u_1);
    fprintf(fp, "\t\tsw_rawawb_vertex3_v_1 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex3_v_1                   ,awb_cfg_v32->vertex3_v_1);
    fprintf(fp, "\t\tsw_rawawb_islope01_1 = 0x%0x (%d)\n",                        awb_cfg_v32->islope01_1                    ,awb_cfg_v32->islope01_1);
    fprintf(fp, "\t\tsw_rawawb_islope12_1 = 0x%0x (%d)\n",                        awb_cfg_v32->islope12_1                    ,awb_cfg_v32->islope12_1);
    fprintf(fp, "\t\tsw_rawawb_islope23_1 = 0x%0x (%d)\n",                        awb_cfg_v32->islope23_1                    ,awb_cfg_v32->islope23_1);
    fprintf(fp, "\t\tsw_rawawb_islope30_1 = 0x%0x (%d)\n",                        awb_cfg_v32->islope30_1                    ,awb_cfg_v32->islope30_1);
    fprintf(fp, "\t\tsw_rawawb_vertex0_u_2 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex0_u_2                   ,awb_cfg_v32->vertex0_u_2);
    fprintf(fp, "\t\tsw_rawawb_vertex0_v_2 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex0_v_2                   ,awb_cfg_v32->vertex0_v_2);
    fprintf(fp, "\t\tsw_rawawb_vertex1_u_2 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex1_u_2                   ,awb_cfg_v32->vertex1_u_2);
    fprintf(fp, "\t\tsw_rawawb_vertex1_v_2 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex1_v_2                   ,awb_cfg_v32->vertex1_v_2);
    fprintf(fp, "\t\tsw_rawawb_vertex2_u_2 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex2_u_2                   ,awb_cfg_v32->vertex2_u_2);
    fprintf(fp, "\t\tsw_rawawb_vertex2_v_2 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex2_v_2                   ,awb_cfg_v32->vertex2_v_2);
    fprintf(fp, "\t\tsw_rawawb_vertex3_u_2 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex3_u_2                   ,awb_cfg_v32->vertex3_u_2);
    fprintf(fp, "\t\tsw_rawawb_vertex3_v_2 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex3_v_2                   ,awb_cfg_v32->vertex3_v_2);
    fprintf(fp, "\t\tsw_rawawb_islope01_2 = 0x%0x (%d)\n",                        awb_cfg_v32->islope01_2                    ,awb_cfg_v32->islope01_2);
    fprintf(fp, "\t\tsw_rawawb_islope12_2 = 0x%0x (%d)\n",                        awb_cfg_v32->islope12_2                    ,awb_cfg_v32->islope12_2);
    fprintf(fp, "\t\tsw_rawawb_islope23_2 = 0x%0x (%d)\n",                        awb_cfg_v32->islope23_2                    ,awb_cfg_v32->islope23_2);
    fprintf(fp, "\t\tsw_rawawb_islope30_2 = 0x%0x (%d)\n",                        awb_cfg_v32->islope30_2                    ,awb_cfg_v32->islope30_2);
    fprintf(fp, "\t\tsw_rawawb_vertex0_u_3 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex0_u_3                   ,awb_cfg_v32->vertex0_u_3);
    fprintf(fp, "\t\tsw_rawawb_vertex0_v_3 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex0_v_3                   ,awb_cfg_v32->vertex0_v_3);
    fprintf(fp, "\t\tsw_rawawb_vertex1_u_3 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex1_u_3                   ,awb_cfg_v32->vertex1_u_3);
    fprintf(fp, "\t\tsw_rawawb_vertex1_v_3 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex1_v_3                   ,awb_cfg_v32->vertex1_v_3);
    fprintf(fp, "\t\tsw_rawawb_vertex2_u_3 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex2_u_3                   ,awb_cfg_v32->vertex2_u_3);
    fprintf(fp, "\t\tsw_rawawb_vertex2_v_3 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex2_v_3                   ,awb_cfg_v32->vertex2_v_3);
    fprintf(fp, "\t\tsw_rawawb_vertex3_u_3 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex3_u_3                   ,awb_cfg_v32->vertex3_u_3);
    fprintf(fp, "\t\tsw_rawawb_vertex3_v_3 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex3_v_3                   ,awb_cfg_v32->vertex3_v_3);
    fprintf(fp, "\t\tsw_rawawb_islope01_3 = 0x%0x (%d)\n",                        awb_cfg_v32->islope01_3                    ,awb_cfg_v32->islope01_3);
    fprintf(fp, "\t\tsw_rawawb_islope12_3 = 0x%0x (%d)\n",                        awb_cfg_v32->islope12_3                    ,awb_cfg_v32->islope12_3);
    fprintf(fp, "\t\tsw_rawawb_islope23_3 = 0x%0x (%d)\n",                        awb_cfg_v32->islope23_3                    ,awb_cfg_v32->islope23_3);
    fprintf(fp, "\t\tsw_rawawb_islope30_3 = 0x%0x (%d)\n",                        awb_cfg_v32->islope30_3                    ,awb_cfg_v32->islope30_3);
    fprintf(fp, "\t\tsw_rawawb_rgb2ryuvmat0_y = 0x%0x (%d)\n",                    awb_cfg_v32->rgb2ryuvmat0_y                ,awb_cfg_v32->rgb2ryuvmat0_y);
    fprintf(fp, "\t\tsw_rawawb_rgb2ryuvmat1_y = 0x%0x (%d)\n",                    awb_cfg_v32->rgb2ryuvmat1_y                ,awb_cfg_v32->rgb2ryuvmat1_y);
    fprintf(fp, "\t\tsw_rawawb_rgb2ryuvmat2_y = 0x%0x (%d)\n",                    awb_cfg_v32->rgb2ryuvmat2_y                ,awb_cfg_v32->rgb2ryuvmat2_y);
    fprintf(fp, "\t\tsw_rawawb_rgb2ryuvofs_y = 0x%0x (%d)\n",                     awb_cfg_v32->rgb2ryuvofs_y                 ,awb_cfg_v32->rgb2ryuvofs_y);
    fprintf(fp, "\t\tsw_rawawb_rgb2ryuvmat0_u = 0x%0x (%d)\n",                    awb_cfg_v32->rgb2ryuvmat0_u                ,awb_cfg_v32->rgb2ryuvmat0_u);
    fprintf(fp, "\t\tsw_rawawb_rgb2ryuvmat1_u = 0x%0x (%d)\n",                    awb_cfg_v32->rgb2ryuvmat1_u                ,awb_cfg_v32->rgb2ryuvmat1_u);
    fprintf(fp, "\t\tsw_rawawb_rgb2ryuvmat2_u = 0x%0x (%d)\n",                    awb_cfg_v32->rgb2ryuvmat2_u                ,awb_cfg_v32->rgb2ryuvmat2_u);
    fprintf(fp, "\t\tsw_rawawb_rgb2ryuvofs_u = 0x%0x (%d)\n",                     awb_cfg_v32->rgb2ryuvofs_u                 ,awb_cfg_v32->rgb2ryuvofs_u);
    fprintf(fp, "\t\tsw_rawawb_rgb2ryuvmat0_v = 0x%0x (%d)\n",                    awb_cfg_v32->rgb2ryuvmat0_v                ,awb_cfg_v32->rgb2ryuvmat0_v);
    fprintf(fp, "\t\tsw_rawawb_rgb2ryuvmat1_v = 0x%0x (%d)\n",                    awb_cfg_v32->rgb2ryuvmat1_v                ,awb_cfg_v32->rgb2ryuvmat1_v);
    fprintf(fp, "\t\tsw_rawawb_rgb2ryuvmat2_v = 0x%0x (%d)\n",                    awb_cfg_v32->rgb2ryuvmat2_v                ,awb_cfg_v32->rgb2ryuvmat2_v);
    fprintf(fp, "\t\tsw_rawawb_rgb2ryuvofs_v = 0x%0x (%d)\n",                     awb_cfg_v32->rgb2ryuvofs_v                 ,awb_cfg_v32->rgb2ryuvofs_v);
    fprintf(fp, "\t\tsw_rawawb_coor_x1_ls0_y = 0x%0x (%d)\n",                     awb_cfg_v32->coor_x1_ls0_y                 ,awb_cfg_v32->coor_x1_ls0_y);
    fprintf(fp, "\t\tsw_rawawb_vec_x21_ls0_y = 0x%0x (%d)\n",                     awb_cfg_v32->vec_x21_ls0_y                 ,awb_cfg_v32->vec_x21_ls0_y);
    fprintf(fp, "\t\tsw_rawawb_coor_x1_ls0_u = 0x%0x (%d)\n",                     awb_cfg_v32->coor_x1_ls0_u                 ,awb_cfg_v32->coor_x1_ls0_u);
    fprintf(fp, "\t\tsw_rawawb_vec_x21_ls0_u = 0x%0x (%d)\n",                     awb_cfg_v32->vec_x21_ls0_u                 ,awb_cfg_v32->vec_x21_ls0_u);
    fprintf(fp, "\t\tsw_rawawb_coor_x1_ls0_v = 0x%0x (%d)\n",                     awb_cfg_v32->coor_x1_ls0_v                 ,awb_cfg_v32->coor_x1_ls0_v);
    fprintf(fp, "\t\tsw_rawawb_vec_x21_ls0_v = 0x%0x (%d)\n",                     awb_cfg_v32->vec_x21_ls0_v                 ,awb_cfg_v32->vec_x21_ls0_v);
    fprintf(fp, "\t\tsw_rawawb_dis_x1x2_ls0 = 0x%0x (%d)\n",                      awb_cfg_v32->dis_x1x2_ls0                  ,awb_cfg_v32->dis_x1x2_ls0);
    fprintf(fp, "\t\tsw_rawawb_rotu0_ls0 = 0x%0x (%d)\n",                         awb_cfg_v32->rotu0_ls0                     ,awb_cfg_v32->rotu0_ls0);
    fprintf(fp, "\t\tsw_rawawb_rotu1_ls0 = 0x%0x (%d)\n",                         awb_cfg_v32->rotu1_ls0                     ,awb_cfg_v32->rotu1_ls0);
    fprintf(fp, "\t\tsw_rawawb_rotu2_ls0 = 0x%0x (%d)\n",                         awb_cfg_v32->rotu2_ls0                     ,awb_cfg_v32->rotu2_ls0);
    fprintf(fp, "\t\tsw_rawawb_rotu3_ls0 = 0x%0x (%d)\n",                         awb_cfg_v32->rotu3_ls0                     ,awb_cfg_v32->rotu3_ls0);
    fprintf(fp, "\t\tsw_rawawb_rotu4_ls0 = 0x%0x (%d)\n",                         awb_cfg_v32->rotu4_ls0                     ,awb_cfg_v32->rotu4_ls0);
    fprintf(fp, "\t\tsw_rawawb_rotu5_ls0 = 0x%0x (%d)\n",                         awb_cfg_v32->rotu5_ls0                     ,awb_cfg_v32->rotu5_ls0);
    fprintf(fp, "\t\tsw_rawawb_th0_ls0 = 0x%0x (%d)\n",                           awb_cfg_v32->th0_ls0                       ,awb_cfg_v32->th0_ls0);
    fprintf(fp, "\t\tsw_rawawb_th1_ls0 = 0x%0x (%d)\n",                           awb_cfg_v32->th1_ls0                       ,awb_cfg_v32->th1_ls0);
    fprintf(fp, "\t\tsw_rawawb_th2_ls0 = 0x%0x (%d)\n",                           awb_cfg_v32->th2_ls0                       ,awb_cfg_v32->th2_ls0);
    fprintf(fp, "\t\tsw_rawawb_th3_ls0 = 0x%0x (%d)\n",                           awb_cfg_v32->th3_ls0                       ,awb_cfg_v32->th3_ls0);
    fprintf(fp, "\t\tsw_rawawb_th4_ls0 = 0x%0x (%d)\n",                           awb_cfg_v32->th4_ls0                       ,awb_cfg_v32->th4_ls0);
    fprintf(fp, "\t\tsw_rawawb_th5_ls0 = 0x%0x (%d)\n",                           awb_cfg_v32->th5_ls0                       ,awb_cfg_v32->th5_ls0);
    fprintf(fp, "\t\tsw_rawawb_coor_x1_ls1_y = 0x%0x (%d)\n",                     awb_cfg_v32->coor_x1_ls1_y                 ,awb_cfg_v32->coor_x1_ls1_y);
    fprintf(fp, "\t\tsw_rawawb_vec_x21_ls1_y = 0x%0x (%d)\n",                     awb_cfg_v32->vec_x21_ls1_y                 ,awb_cfg_v32->vec_x21_ls1_y);
    fprintf(fp, "\t\tsw_rawawb_coor_x1_ls1_u = 0x%0x (%d)\n",                     awb_cfg_v32->coor_x1_ls1_u                 ,awb_cfg_v32->coor_x1_ls1_u);
    fprintf(fp, "\t\tsw_rawawb_vec_x21_ls1_u = 0x%0x (%d)\n",                     awb_cfg_v32->vec_x21_ls1_u                 ,awb_cfg_v32->vec_x21_ls1_u);
    fprintf(fp, "\t\tsw_rawawb_coor_x1_ls1_v = 0x%0x (%d)\n",                     awb_cfg_v32->coor_x1_ls1_v                 ,awb_cfg_v32->coor_x1_ls1_v);
    fprintf(fp, "\t\tsw_rawawb_vec_x21_ls1_v = 0x%0x (%d)\n",                     awb_cfg_v32->vec_x21_ls1_v                 ,awb_cfg_v32->vec_x21_ls1_v);
    fprintf(fp, "\t\tsw_rawawb_dis_x1x2_ls1 = 0x%0x (%d)\n",                      awb_cfg_v32->dis_x1x2_ls1                  ,awb_cfg_v32->dis_x1x2_ls1);
    fprintf(fp, "\t\tsw_rawawb_rotu0_ls1 = 0x%0x (%d)\n",                         awb_cfg_v32->rotu0_ls1                     ,awb_cfg_v32->rotu0_ls1);
    fprintf(fp, "\t\tsw_rawawb_rotu1_ls1 = 0x%0x (%d)\n",                         awb_cfg_v32->rotu1_ls1                     ,awb_cfg_v32->rotu1_ls1);
    fprintf(fp, "\t\tsw_rawawb_rotu2_ls1 = 0x%0x (%d)\n",                         awb_cfg_v32->rotu2_ls1                     ,awb_cfg_v32->rotu2_ls1);
    fprintf(fp, "\t\tsw_rawawb_rotu3_ls1 = 0x%0x (%d)\n",                         awb_cfg_v32->rotu3_ls1                     ,awb_cfg_v32->rotu3_ls1);
    fprintf(fp, "\t\tsw_rawawb_rotu4_ls1 = 0x%0x (%d)\n",                         awb_cfg_v32->rotu4_ls1                     ,awb_cfg_v32->rotu4_ls1);
    fprintf(fp, "\t\tsw_rawawb_rotu5_ls1 = 0x%0x (%d)\n",                         awb_cfg_v32->rotu5_ls1                     ,awb_cfg_v32->rotu5_ls1);
    fprintf(fp, "\t\tsw_rawawb_th0_ls1 = 0x%0x (%d)\n",                           awb_cfg_v32->th0_ls1                       ,awb_cfg_v32->th0_ls1);
    fprintf(fp, "\t\tsw_rawawb_th1_ls1 = 0x%0x (%d)\n",                           awb_cfg_v32->th1_ls1                       ,awb_cfg_v32->th1_ls1);
    fprintf(fp, "\t\tsw_rawawb_th2_ls1 = 0x%0x (%d)\n",                           awb_cfg_v32->th2_ls1                       ,awb_cfg_v32->th2_ls1);
    fprintf(fp, "\t\tsw_rawawb_th3_ls1 = 0x%0x (%d)\n",                           awb_cfg_v32->th3_ls1                       ,awb_cfg_v32->th3_ls1);
    fprintf(fp, "\t\tsw_rawawb_th4_ls1 = 0x%0x (%d)\n",                           awb_cfg_v32->th4_ls1                       ,awb_cfg_v32->th4_ls1);
    fprintf(fp, "\t\tsw_rawawb_th5_ls1 = 0x%0x (%d)\n",                           awb_cfg_v32->th5_ls1                       ,awb_cfg_v32->th5_ls1);
    fprintf(fp, "\t\tsw_rawawb_coor_x1_ls2_y = 0x%0x (%d)\n",                     awb_cfg_v32->coor_x1_ls2_y                 ,awb_cfg_v32->coor_x1_ls2_y);
    fprintf(fp, "\t\tsw_rawawb_vec_x21_ls2_y = 0x%0x (%d)\n",                     awb_cfg_v32->vec_x21_ls2_y                 ,awb_cfg_v32->vec_x21_ls2_y);
    fprintf(fp, "\t\tsw_rawawb_coor_x1_ls2_u = 0x%0x (%d)\n",                     awb_cfg_v32->coor_x1_ls2_u                 ,awb_cfg_v32->coor_x1_ls2_u);
    fprintf(fp, "\t\tsw_rawawb_vec_x21_ls2_u = 0x%0x (%d)\n",                     awb_cfg_v32->vec_x21_ls2_u                 ,awb_cfg_v32->vec_x21_ls2_u);
    fprintf(fp, "\t\tsw_rawawb_coor_x1_ls2_v = 0x%0x (%d)\n",                     awb_cfg_v32->coor_x1_ls2_v                 ,awb_cfg_v32->coor_x1_ls2_v);
    fprintf(fp, "\t\tsw_rawawb_vec_x21_ls2_v = 0x%0x (%d)\n",                     awb_cfg_v32->vec_x21_ls2_v                 ,awb_cfg_v32->vec_x21_ls2_v);
    fprintf(fp, "\t\tsw_rawawb_dis_x1x2_ls2 = 0x%0x (%d)\n",                      awb_cfg_v32->dis_x1x2_ls2                  ,awb_cfg_v32->dis_x1x2_ls2);
    fprintf(fp, "\t\tsw_rawawb_rotu0_ls2 = 0x%0x (%d)\n",                         awb_cfg_v32->rotu0_ls2                     ,awb_cfg_v32->rotu0_ls2);
    fprintf(fp, "\t\tsw_rawawb_rotu1_ls2 = 0x%0x (%d)\n",                         awb_cfg_v32->rotu1_ls2                     ,awb_cfg_v32->rotu1_ls2);
    fprintf(fp, "\t\tsw_rawawb_rotu2_ls2 = 0x%0x (%d)\n",                         awb_cfg_v32->rotu2_ls2                     ,awb_cfg_v32->rotu2_ls2);
    fprintf(fp, "\t\tsw_rawawb_rotu3_ls2 = 0x%0x (%d)\n",                         awb_cfg_v32->rotu3_ls2                     ,awb_cfg_v32->rotu3_ls2);
    fprintf(fp, "\t\tsw_rawawb_rotu4_ls2 = 0x%0x (%d)\n",                         awb_cfg_v32->rotu4_ls2                     ,awb_cfg_v32->rotu4_ls2);
    fprintf(fp, "\t\tsw_rawawb_rotu5_ls2 = 0x%0x (%d)\n",                         awb_cfg_v32->rotu5_ls2                     ,awb_cfg_v32->rotu5_ls2);
    fprintf(fp, "\t\tsw_rawawb_th0_ls2 = 0x%0x (%d)\n",                           awb_cfg_v32->th0_ls2                       ,awb_cfg_v32->th0_ls2);
    fprintf(fp, "\t\tsw_rawawb_th1_ls2 = 0x%0x (%d)\n",                           awb_cfg_v32->th1_ls2                       ,awb_cfg_v32->th1_ls2);
    fprintf(fp, "\t\tsw_rawawb_th2_ls2 = 0x%0x (%d)\n",                           awb_cfg_v32->th2_ls2                       ,awb_cfg_v32->th2_ls2);
    fprintf(fp, "\t\tsw_rawawb_th3_ls2 = 0x%0x (%d)\n",                           awb_cfg_v32->th3_ls2                       ,awb_cfg_v32->th3_ls2);
    fprintf(fp, "\t\tsw_rawawb_th4_ls2 = 0x%0x (%d)\n",                           awb_cfg_v32->th4_ls2                       ,awb_cfg_v32->th4_ls2);
    fprintf(fp, "\t\tsw_rawawb_th5_ls2 = 0x%0x (%d)\n",                           awb_cfg_v32->th5_ls2                       ,awb_cfg_v32->th5_ls2);
    fprintf(fp, "\t\tsw_rawawb_coor_x1_ls3_y = 0x%0x (%d)\n",                     awb_cfg_v32->coor_x1_ls3_y                 ,awb_cfg_v32->coor_x1_ls3_y);
    fprintf(fp, "\t\tsw_rawawb_vec_x21_ls3_y = 0x%0x (%d)\n",                     awb_cfg_v32->vec_x21_ls3_y                 ,awb_cfg_v32->vec_x21_ls3_y);
    fprintf(fp, "\t\tsw_rawawb_coor_x1_ls3_u = 0x%0x (%d)\n",                     awb_cfg_v32->coor_x1_ls3_u                 ,awb_cfg_v32->coor_x1_ls3_u);
    fprintf(fp, "\t\tsw_rawawb_vec_x21_ls3_u = 0x%0x (%d)\n",                     awb_cfg_v32->vec_x21_ls3_u                 ,awb_cfg_v32->vec_x21_ls3_u);
    fprintf(fp, "\t\tsw_rawawb_coor_x1_ls3_v = 0x%0x (%d)\n",                     awb_cfg_v32->coor_x1_ls3_v                 ,awb_cfg_v32->coor_x1_ls3_v);
    fprintf(fp, "\t\tsw_rawawb_vec_x21_ls3_v = 0x%0x (%d)\n",                     awb_cfg_v32->vec_x21_ls3_v                 ,awb_cfg_v32->vec_x21_ls3_v);
    fprintf(fp, "\t\tsw_rawawb_dis_x1x2_ls3 = 0x%0x (%d)\n",                      awb_cfg_v32->dis_x1x2_ls3                  ,awb_cfg_v32->dis_x1x2_ls3);
    fprintf(fp, "\t\tsw_rawawb_rotu0_ls3 = 0x%0x (%d)\n",                         awb_cfg_v32->rotu0_ls3                     ,awb_cfg_v32->rotu0_ls3);
    fprintf(fp, "\t\tsw_rawawb_rotu1_ls3 = 0x%0x (%d)\n",                         awb_cfg_v32->rotu1_ls3                     ,awb_cfg_v32->rotu1_ls3);
    fprintf(fp, "\t\tsw_rawawb_rotu2_ls3 = 0x%0x (%d)\n",                         awb_cfg_v32->rotu2_ls3                     ,awb_cfg_v32->rotu2_ls3);
    fprintf(fp, "\t\tsw_rawawb_rotu3_ls3 = 0x%0x (%d)\n",                         awb_cfg_v32->rotu3_ls3                     ,awb_cfg_v32->rotu3_ls3);
    fprintf(fp, "\t\tsw_rawawb_rotu4_ls3 = 0x%0x (%d)\n",                         awb_cfg_v32->rotu4_ls3                     ,awb_cfg_v32->rotu4_ls3);
    fprintf(fp, "\t\tsw_rawawb_rotu5_ls3 = 0x%0x (%d)\n",                         awb_cfg_v32->rotu5_ls3                     ,awb_cfg_v32->rotu5_ls3);
    fprintf(fp, "\t\tsw_rawawb_th0_ls3 = 0x%0x (%d)\n",                           awb_cfg_v32->th0_ls3                       ,awb_cfg_v32->th0_ls3);
    fprintf(fp, "\t\tsw_rawawb_th1_ls3 = 0x%0x (%d)\n",                           awb_cfg_v32->th1_ls3                       ,awb_cfg_v32->th1_ls3);
    fprintf(fp, "\t\tsw_rawawb_th2_ls3 = 0x%0x (%d)\n",                           awb_cfg_v32->th2_ls3                       ,awb_cfg_v32->th2_ls3);
    fprintf(fp, "\t\tsw_rawawb_th3_ls3 = 0x%0x (%d)\n",                           awb_cfg_v32->th3_ls3                       ,awb_cfg_v32->th3_ls3);
    fprintf(fp, "\t\tsw_rawawb_th4_ls3 = 0x%0x (%d)\n",                           awb_cfg_v32->th4_ls3                       ,awb_cfg_v32->th4_ls3);
    fprintf(fp, "\t\tsw_rawawb_th5_ls3 = 0x%0x (%d)\n",                           awb_cfg_v32->th5_ls3                       ,awb_cfg_v32->th5_ls3);
    fprintf(fp, "\t\tsw_rawawb_wt0 = 0x%0x (%d)\n",                               awb_cfg_v32->wt0                           ,awb_cfg_v32->wt0);
    fprintf(fp, "\t\tsw_rawawb_wt1 = 0x%0x (%d)\n",                               awb_cfg_v32->wt1                           ,awb_cfg_v32->wt1);
    fprintf(fp, "\t\tsw_rawawb_wt2 = 0x%0x (%d)\n",                               awb_cfg_v32->wt2                           ,awb_cfg_v32->wt2);
    fprintf(fp, "\t\tsw_rawawb_mat0_x = 0x%0x (%d)\n",                            awb_cfg_v32->mat0_x                        ,awb_cfg_v32->mat0_x);
    fprintf(fp, "\t\tsw_rawawb_mat0_y = 0x%0x (%d)\n",                            awb_cfg_v32->mat0_y                        ,awb_cfg_v32->mat0_y);
    fprintf(fp, "\t\tsw_rawawb_mat1_x = 0x%0x (%d)\n",                            awb_cfg_v32->mat1_x                        ,awb_cfg_v32->mat1_x);
    fprintf(fp, "\t\tsw_rawawb_mat1_y = 0x%0x (%d)\n",                            awb_cfg_v32->mat1_y                        ,awb_cfg_v32->mat1_y);
    fprintf(fp, "\t\tsw_rawawb_mat2_x = 0x%0x (%d)\n",                            awb_cfg_v32->mat2_x                        ,awb_cfg_v32->mat2_x);
    fprintf(fp, "\t\tsw_rawawb_mat2_y = 0x%0x (%d)\n",                            awb_cfg_v32->mat2_y                        ,awb_cfg_v32->mat2_y);
    fprintf(fp, "\t\tsw_rawawb_nor_x0_0 = 0x%0x (%d)\n",                          awb_cfg_v32->nor_x0_0                      ,awb_cfg_v32->nor_x0_0);
    fprintf(fp, "\t\tsw_rawawb_nor_x1_0 = 0x%0x (%d)\n",                          awb_cfg_v32->nor_x1_0                      ,awb_cfg_v32->nor_x1_0);
    fprintf(fp, "\t\tsw_rawawb_nor_y0_0 = 0x%0x (%d)\n",                          awb_cfg_v32->nor_y0_0                      ,awb_cfg_v32->nor_y0_0);
    fprintf(fp, "\t\tsw_rawawb_nor_y1_0 = 0x%0x (%d)\n",                          awb_cfg_v32->nor_y1_0                      ,awb_cfg_v32->nor_y1_0);
    fprintf(fp, "\t\tsw_rawawb_big_x0_0 = 0x%0x (%d)\n",                          awb_cfg_v32->big_x0_0                      ,awb_cfg_v32->big_x0_0);
    fprintf(fp, "\t\tsw_rawawb_big_x1_0 = 0x%0x (%d)\n",                          awb_cfg_v32->big_x1_0                      ,awb_cfg_v32->big_x1_0);
    fprintf(fp, "\t\tsw_rawawb_big_y0_0 = 0x%0x (%d)\n",                          awb_cfg_v32->big_y0_0                      ,awb_cfg_v32->big_y0_0);
    fprintf(fp, "\t\tsw_rawawb_big_y1_0 = 0x%0x (%d)\n",                          awb_cfg_v32->big_y1_0                      ,awb_cfg_v32->big_y1_0);
    fprintf(fp, "\t\tsw_rawawb_nor_x0_1 = 0x%0x (%d)\n",                          awb_cfg_v32->nor_x0_1                      ,awb_cfg_v32->nor_x0_1);
    fprintf(fp, "\t\tsw_rawawb_nor_x1_1 = 0x%0x (%d)\n",                          awb_cfg_v32->nor_x1_1                      ,awb_cfg_v32->nor_x1_1);
    fprintf(fp, "\t\tsw_rawawb_nor_y0_1 = 0x%0x (%d)\n",                          awb_cfg_v32->nor_y0_1                      ,awb_cfg_v32->nor_y0_1);
    fprintf(fp, "\t\tsw_rawawb_nor_y1_1 = 0x%0x (%d)\n",                          awb_cfg_v32->nor_y1_1                      ,awb_cfg_v32->nor_y1_1);
    fprintf(fp, "\t\tsw_rawawb_big_x0_1 = 0x%0x (%d)\n",                          awb_cfg_v32->big_x0_1                      ,awb_cfg_v32->big_x0_1);
    fprintf(fp, "\t\tsw_rawawb_big_x1_1 = 0x%0x (%d)\n",                          awb_cfg_v32->big_x1_1                      ,awb_cfg_v32->big_x1_1);
    fprintf(fp, "\t\tsw_rawawb_big_y0_1 = 0x%0x (%d)\n",                          awb_cfg_v32->big_y0_1                      ,awb_cfg_v32->big_y0_1);
    fprintf(fp, "\t\tsw_rawawb_big_y1_1 = 0x%0x (%d)\n",                          awb_cfg_v32->big_y1_1                      ,awb_cfg_v32->big_y1_1);
    fprintf(fp, "\t\tsw_rawawb_nor_x0_2 = 0x%0x (%d)\n",                          awb_cfg_v32->nor_x0_2                      ,awb_cfg_v32->nor_x0_2);
    fprintf(fp, "\t\tsw_rawawb_nor_x1_2 = 0x%0x (%d)\n",                          awb_cfg_v32->nor_x1_2                      ,awb_cfg_v32->nor_x1_2);
    fprintf(fp, "\t\tsw_rawawb_nor_y0_2 = 0x%0x (%d)\n",                          awb_cfg_v32->nor_y0_2                      ,awb_cfg_v32->nor_y0_2);
    fprintf(fp, "\t\tsw_rawawb_nor_y1_2 = 0x%0x (%d)\n",                          awb_cfg_v32->nor_y1_2                      ,awb_cfg_v32->nor_y1_2);
    fprintf(fp, "\t\tsw_rawawb_big_x0_2 = 0x%0x (%d)\n",                          awb_cfg_v32->big_x0_2                      ,awb_cfg_v32->big_x0_2);
    fprintf(fp, "\t\tsw_rawawb_big_x1_2 = 0x%0x (%d)\n",                          awb_cfg_v32->big_x1_2                      ,awb_cfg_v32->big_x1_2);
    fprintf(fp, "\t\tsw_rawawb_big_y0_2 = 0x%0x (%d)\n",                          awb_cfg_v32->big_y0_2                      ,awb_cfg_v32->big_y0_2);
    fprintf(fp, "\t\tsw_rawawb_big_y1_2 = 0x%0x (%d)\n",                          awb_cfg_v32->big_y1_2                      ,awb_cfg_v32->big_y1_2);
    fprintf(fp, "\t\tsw_rawawb_nor_x0_3 = 0x%0x (%d)\n",                          awb_cfg_v32->nor_x0_3                      ,awb_cfg_v32->nor_x0_3);
    fprintf(fp, "\t\tsw_rawawb_nor_x1_3 = 0x%0x (%d)\n",                          awb_cfg_v32->nor_x1_3                      ,awb_cfg_v32->nor_x1_3);
    fprintf(fp, "\t\tsw_rawawb_nor_y0_3 = 0x%0x (%d)\n",                          awb_cfg_v32->nor_y0_3                      ,awb_cfg_v32->nor_y0_3);
    fprintf(fp, "\t\tsw_rawawb_nor_y1_3 = 0x%0x (%d)\n",                          awb_cfg_v32->nor_y1_3                      ,awb_cfg_v32->nor_y1_3);
    fprintf(fp, "\t\tsw_rawawb_big_x0_3 = 0x%0x (%d)\n",                          awb_cfg_v32->big_x0_3                      ,awb_cfg_v32->big_x0_3);
    fprintf(fp, "\t\tsw_rawawb_big_x1_3 = 0x%0x (%d)\n",                          awb_cfg_v32->big_x1_3                      ,awb_cfg_v32->big_x1_3);
    fprintf(fp, "\t\tsw_rawawb_big_y0_3 = 0x%0x (%d)\n",                          awb_cfg_v32->big_y0_3                      ,awb_cfg_v32->big_y0_3);
    fprintf(fp, "\t\tsw_rawawb_big_y1_3 = 0x%0x (%d)\n",                          awb_cfg_v32->big_y1_3                      ,awb_cfg_v32->big_y1_3);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region0_excen = 0x%0x (%d)\n",              awb_cfg_v32->exc_wp_region0_excen          ,awb_cfg_v32->exc_wp_region0_excen);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region0_measen = 0x%0x (%d)\n",             awb_cfg_v32->exc_wp_region0_measen         ,awb_cfg_v32->exc_wp_region0_measen);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region0_domain = 0x%0x (%d)\n",             awb_cfg_v32->exc_wp_region0_domain         ,awb_cfg_v32->exc_wp_region0_domain);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region1_excen = 0x%0x (%d)\n",              awb_cfg_v32->exc_wp_region1_excen          ,awb_cfg_v32->exc_wp_region1_excen);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region1_measen = 0x%0x (%d)\n",             awb_cfg_v32->exc_wp_region1_measen         ,awb_cfg_v32->exc_wp_region1_measen);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region1_domain = 0x%0x (%d)\n",             awb_cfg_v32->exc_wp_region1_domain         ,awb_cfg_v32->exc_wp_region1_domain);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region2_excen = 0x%0x (%d)\n",              awb_cfg_v32->exc_wp_region2_excen          ,awb_cfg_v32->exc_wp_region2_excen);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region2_measen = 0x%0x (%d)\n",             awb_cfg_v32->exc_wp_region2_measen         ,awb_cfg_v32->exc_wp_region2_measen);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region2_domain = 0x%0x (%d)\n",             awb_cfg_v32->exc_wp_region2_domain         ,awb_cfg_v32->exc_wp_region2_domain);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region3_excen = 0x%0x (%d)\n",              awb_cfg_v32->exc_wp_region3_excen          ,awb_cfg_v32->exc_wp_region3_excen);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region3_measen = 0x%0x (%d)\n",             awb_cfg_v32->exc_wp_region3_measen         ,awb_cfg_v32->exc_wp_region3_measen);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region3_domain = 0x%0x (%d)\n",             awb_cfg_v32->exc_wp_region3_domain         ,awb_cfg_v32->exc_wp_region3_domain);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region4_excen = 0x%0x (%d)\n",              awb_cfg_v32->exc_wp_region4_excen          ,awb_cfg_v32->exc_wp_region4_excen);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region4_domain = 0x%0x (%d)\n",             awb_cfg_v32->exc_wp_region4_domain         ,awb_cfg_v32->exc_wp_region4_domain);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region5_excen = 0x%0x (%d)\n",              awb_cfg_v32->exc_wp_region5_excen          ,awb_cfg_v32->exc_wp_region5_excen);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region5_domain = 0x%0x (%d)\n",             awb_cfg_v32->exc_wp_region5_domain         ,awb_cfg_v32->exc_wp_region5_domain);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region6_excen = 0x%0x (%d)\n",              awb_cfg_v32->exc_wp_region6_excen          ,awb_cfg_v32->exc_wp_region6_excen);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region6_domain = 0x%0x (%d)\n",             awb_cfg_v32->exc_wp_region6_domain         ,awb_cfg_v32->exc_wp_region6_domain);
    fprintf(fp, "\t\tsw_rawawb_multiwindow_en = 0x%0x (%d)\n",                    awb_cfg_v32->multiwindow_en                ,awb_cfg_v32->multiwindow_en);
    fprintf(fp, "\t\tsw_rawawb_multiwindow0_h_offs = 0x%0x (%d)\n",               awb_cfg_v32->multiwindow0_h_offs           ,awb_cfg_v32->multiwindow0_h_offs);
    fprintf(fp, "\t\tsw_rawawb_multiwindow0_v_offs = 0x%0x (%d)\n",               awb_cfg_v32->multiwindow0_v_offs           ,awb_cfg_v32->multiwindow0_v_offs);
    fprintf(fp, "\t\tsw_rawawb_multiwindow0_h_size = 0x%0x (%d)\n",               awb_cfg_v32->multiwindow0_h_size           ,awb_cfg_v32->multiwindow0_h_size);
    fprintf(fp, "\t\tsw_rawawb_multiwindow1_v_size = 0x%0x (%d)\n",               awb_cfg_v32->multiwindow1_v_size           ,awb_cfg_v32->multiwindow1_v_size);
    fprintf(fp, "\t\tsw_rawawb_multiwindow1_h_offs = 0x%0x (%d)\n",               awb_cfg_v32->multiwindow1_h_offs           ,awb_cfg_v32->multiwindow1_h_offs);
    fprintf(fp, "\t\tsw_rawawb_multiwindow1_v_offs = 0x%0x (%d)\n",               awb_cfg_v32->multiwindow1_v_offs           ,awb_cfg_v32->multiwindow1_v_offs);
    fprintf(fp, "\t\tsw_rawawb_multiwindow1_h_size = 0x%0x (%d)\n",               awb_cfg_v32->multiwindow1_h_size           ,awb_cfg_v32->multiwindow1_h_size);
    fprintf(fp, "\t\tsw_rawawb_multiwindow1_v_size = 0x%0x (%d)\n",               awb_cfg_v32->multiwindow1_v_size           ,awb_cfg_v32->multiwindow1_v_size);
    fprintf(fp, "\t\tsw_rawawb_multiwindow2_h_offs = 0x%0x (%d)\n",               awb_cfg_v32->multiwindow2_h_offs           ,awb_cfg_v32->multiwindow2_h_offs);
    fprintf(fp, "\t\tsw_rawawb_multiwindow2_v_offs = 0x%0x (%d)\n",               awb_cfg_v32->multiwindow2_v_offs           ,awb_cfg_v32->multiwindow2_v_offs);
    fprintf(fp, "\t\tsw_rawawb_multiwindow2_h_size = 0x%0x (%d)\n",               awb_cfg_v32->multiwindow2_h_size           ,awb_cfg_v32->multiwindow2_h_size);
    fprintf(fp, "\t\tsw_rawawb_multiwindow2_v_size = 0x%0x (%d)\n",               awb_cfg_v32->multiwindow2_v_size           ,awb_cfg_v32->multiwindow2_v_size);
    fprintf(fp, "\t\tsw_rawawb_multiwindow3_h_offs = 0x%0x (%d)\n",               awb_cfg_v32->multiwindow3_h_offs           ,awb_cfg_v32->multiwindow3_h_offs);
    fprintf(fp, "\t\tsw_rawawb_multiwindow3_v_offs = 0x%0x (%d)\n",               awb_cfg_v32->multiwindow3_v_offs           ,awb_cfg_v32->multiwindow3_v_offs);
    fprintf(fp, "\t\tsw_rawawb_multiwindow3_h_size = 0x%0x (%d)\n",               awb_cfg_v32->multiwindow3_h_size           ,awb_cfg_v32->multiwindow3_h_size);
    fprintf(fp, "\t\tsw_rawawb_multiwindow3_v_size = 0x%0x (%d)\n",               awb_cfg_v32->multiwindow3_v_size           ,awb_cfg_v32->multiwindow3_v_size);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region0_xu0 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region0_xu0            ,awb_cfg_v32->exc_wp_region0_xu0);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region0_xu1 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region0_xu1            ,awb_cfg_v32->exc_wp_region0_xu1);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region0_yv0 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region0_yv0            ,awb_cfg_v32->exc_wp_region0_yv0);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region0_yv1 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region0_yv1            ,awb_cfg_v32->exc_wp_region0_yv1);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region1_xu0 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region1_xu0            ,awb_cfg_v32->exc_wp_region1_xu0);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region1_xu1 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region1_xu1            ,awb_cfg_v32->exc_wp_region1_xu1);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region1_yv0 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region1_yv0            ,awb_cfg_v32->exc_wp_region1_yv0);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region1_yv1 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region1_yv1            ,awb_cfg_v32->exc_wp_region1_yv1);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region2_xu0 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region2_xu0            ,awb_cfg_v32->exc_wp_region2_xu0);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region2_xu1 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region2_xu1            ,awb_cfg_v32->exc_wp_region2_xu1);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region2_yv0 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region2_yv0            ,awb_cfg_v32->exc_wp_region2_yv0);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region2_yv1 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region2_yv1            ,awb_cfg_v32->exc_wp_region2_yv1);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region3_xu0 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region3_xu0            ,awb_cfg_v32->exc_wp_region3_xu0);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region3_xu1 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region3_xu1            ,awb_cfg_v32->exc_wp_region3_xu1);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region3_yv0 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region3_yv0            ,awb_cfg_v32->exc_wp_region3_yv0);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region3_yv1 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region3_yv1            ,awb_cfg_v32->exc_wp_region3_yv1);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region4_xu0 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region4_xu0            ,awb_cfg_v32->exc_wp_region4_xu0);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region4_xu1 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region4_xu1            ,awb_cfg_v32->exc_wp_region4_xu1);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region4_yv0 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region4_yv0            ,awb_cfg_v32->exc_wp_region4_yv0);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region4_yv1 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region4_yv1            ,awb_cfg_v32->exc_wp_region4_yv1);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region5_xu0 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region5_xu0            ,awb_cfg_v32->exc_wp_region5_xu0);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region5_xu1 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region5_xu1            ,awb_cfg_v32->exc_wp_region5_xu1);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region5_yv0 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region5_yv0            ,awb_cfg_v32->exc_wp_region5_yv0);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region5_yv1 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region5_yv1            ,awb_cfg_v32->exc_wp_region5_yv1);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region6_xu0 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region6_xu0            ,awb_cfg_v32->exc_wp_region6_xu0);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region6_xu1 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region6_xu1            ,awb_cfg_v32->exc_wp_region6_xu1);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region6_yv0 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region6_yv0            ,awb_cfg_v32->exc_wp_region6_yv0);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region6_yv1 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region6_yv1            ,awb_cfg_v32->exc_wp_region6_yv1);


    fclose(fp);
#endif
}
void Isp32Params::convertAiqAwbToIsp32Params(struct isp32_isp_params_cfg& isp_cfg,
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

    struct isp32_rawawb_meas_cfg* awb_cfg_v32 = &isp_cfg.meas.rawawb;
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
 #if RKAIQ_HAVE_AWB_V32LT
    if(awb_meas.dsMode ==RK_AIQ_AWB_DS_4X4 || awb_meas.dsMode ==RK_AIQ_AWB_DS_8X8){
        awb_cfg_v32->wind_size       = awb_meas.dsMode;
        awb_cfg_v32->ds16x8_mode_en = 0;
    }else if(awb_meas.dsMode ==RK_AIQ_AWB_DS_16X8){
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
        awb_cfg_v32->win_weight[i] = awb_meas.blkWeight[i*5]|
            (awb_meas.blkWeight[i*5+1]<<6)|
            (awb_meas.blkWeight[i*5+2]<<12)|
            (awb_meas.blkWeight[i*5+3]<<18)|
            (awb_meas.blkWeight[i*5+4]<<24);
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
#if RKAIQ_HAVE_AF_V31 || RKAIQ_ONLY_AF_STATS_V31
void Isp32Params::convertAiqAfToIsp32Params(struct isp32_isp_params_cfg& isp_cfg,
        const rk_aiq_isp_af_v31_t& af_data,
        bool af_cfg_udpate) {
    int i;

    if (!af_cfg_udpate) return;

    if (af_data.af_en) isp_cfg.module_ens |= ISP2X_MODULE_RAWAF;
    isp_cfg.module_en_update |= ISP2X_MODULE_RAWAF;
    isp_cfg.module_cfg_update |= ISP2X_MODULE_RAWAF;

    isp_cfg.meas.rawaf.rawaf_sel      = af_data.rawaf_sel;
    isp_cfg.meas.rawaf.gamma_en       = af_data.gamma_en;
    isp_cfg.meas.rawaf.gaus_en        = af_data.gaus_en;
    isp_cfg.meas.rawaf.v1_fir_sel     = af_data.v1_fir_sel;
    isp_cfg.meas.rawaf.hiir_en        = af_data.hiir_en;
    isp_cfg.meas.rawaf.viir_en        = af_data.viir_en;
    isp_cfg.meas.rawaf.v1_fv_mode     = af_data.v1_fv_outmode;
    isp_cfg.meas.rawaf.v2_fv_mode     = af_data.v2_fv_outmode;
    isp_cfg.meas.rawaf.h1_fv_mode     = af_data.h1_fv_outmode;
    isp_cfg.meas.rawaf.h2_fv_mode     = af_data.h2_fv_outmode;
    isp_cfg.meas.rawaf.ldg_en         = af_data.ldg_en;
    isp_cfg.meas.rawaf.accu_8bit_mode = af_data.accu_8bit_mode;
    isp_cfg.meas.rawaf.y_mode         = af_data.y_mode;
    isp_cfg.meas.rawaf.vldg_sel       = af_data.vldg_sel;
    isp_cfg.meas.rawaf.sobel_sel      = af_data.sobel_sel;
    isp_cfg.meas.rawaf.v_dnscl_mode   = af_data.v_dnscl_mode;
    isp_cfg.meas.rawaf.from_awb       = af_data.from_awb;
    isp_cfg.meas.rawaf.from_ynr       = af_data.from_ynr;
    if (af_data.af_en) {
        isp_cfg.meas.rawaf.ae_mode = af_data.ae_mode;
        isp_cfg.meas.rawaf.ae_config_use = af_data.ae_config_use;
    } else {
        isp_cfg.meas.rawaf.ae_mode = 0;
        isp_cfg.meas.rawaf.ae_config_use = 1;
    }

    memcpy(isp_cfg.meas.rawaf.line_en, af_data.line_en,
           ISP32_RAWAF_LINE_NUM * sizeof(unsigned char));
    memcpy(isp_cfg.meas.rawaf.line_num, af_data.line_num,
           ISP32_RAWAF_LINE_NUM * sizeof(unsigned char));

    isp_cfg.meas.rawaf.num_afm_win   = af_data.window_num;
    isp_cfg.meas.rawaf.win[0].h_offs = af_data.wina_h_offs;
    isp_cfg.meas.rawaf.win[0].v_offs = af_data.wina_v_offs;
    isp_cfg.meas.rawaf.win[0].h_size = af_data.wina_h_size;
    isp_cfg.meas.rawaf.win[0].v_size = af_data.wina_v_size;
    isp_cfg.meas.rawaf.win[1].h_offs = af_data.winb_h_offs;
    isp_cfg.meas.rawaf.win[1].v_offs = af_data.winb_v_offs;
    isp_cfg.meas.rawaf.win[1].h_size = af_data.winb_h_size;
    isp_cfg.meas.rawaf.win[1].v_size = af_data.winb_v_size;

    memcpy(isp_cfg.meas.rawaf.gamma_y, af_data.gamma_y,
           ISP32_RAWAF_GAMMA_NUM * sizeof(unsigned short));

    /* THRES */
    isp_cfg.meas.rawaf.afm_thres = af_data.thres;
    /* VAR_SHIFT */
    isp_cfg.meas.rawaf.afm_var_shift[0] = af_data.shift_sum_a;
    isp_cfg.meas.rawaf.lum_var_shift[0] = af_data.shift_y_a;
    isp_cfg.meas.rawaf.afm_var_shift[1] = af_data.shift_sum_b;
    isp_cfg.meas.rawaf.lum_var_shift[1] = af_data.shift_y_b;
    /* HVIIR_VAR_SHIFT */
    isp_cfg.meas.rawaf.h1iir_var_shift = af_data.h1_fv_shift;
    isp_cfg.meas.rawaf.h2iir_var_shift = af_data.h2_fv_shift;
    isp_cfg.meas.rawaf.v1iir_var_shift = af_data.v1_fv_shift;
    isp_cfg.meas.rawaf.v2iir_var_shift = af_data.v2_fv_shift;

    /* HIIR_THRESH */
    isp_cfg.meas.rawaf.h_fv_thresh = af_data.h_fv_thresh;
    isp_cfg.meas.rawaf.v_fv_thresh = af_data.v_fv_thresh;

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
    for (i = 0; i < ISP32_RAWAF_GAUS_COE_NUM; i++) {
        isp_cfg.meas.rawaf.gaus_coe[i] = af_data.gaus_coe[i];
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

    // rawae3 is used by af now!!!
    if (af_data.af_en && af_data.ae_mode) {
        isp_cfg.module_ens |= ISP2X_MODULE_RAWAE3;
        isp_cfg.module_en_update |= ISP2X_MODULE_RAWAE3;
        isp_cfg.module_cfg_update |= ISP2X_MODULE_RAWAE3;

        isp_cfg.meas.rawae3.win.h_offs = af_data.wina_h_offs;
        isp_cfg.meas.rawae3.win.v_offs = af_data.wina_v_offs;
        isp_cfg.meas.rawae3.win.h_size = af_data.wina_h_size;
        isp_cfg.meas.rawae3.win.v_size = af_data.wina_v_size;
    }
    mLatestMeasCfg.rawaf = isp_cfg.meas.rawaf;
    mLatestMeasCfg.rawae3 = isp_cfg.meas.rawae3;
}
#endif
#if RKAIQ_HAVE_AF_V32_LITE || RKAIQ_ONLY_AF_STATS_V32_LITE
void Isp32Params::convertAiqAfLiteToIsp32Params(struct isp32_isp_params_cfg& isp_cfg,
                                            const rk_aiq_isp_af_v32_t& af_data,
                                            bool af_cfg_udpate) {
    int i;

    if (!af_cfg_udpate) return;

    if (af_data.af_en) isp_cfg.module_ens |= ISP2X_MODULE_RAWAF;
    isp_cfg.module_en_update |= ISP2X_MODULE_RAWAF;
    isp_cfg.module_cfg_update |= ISP2X_MODULE_RAWAF;

    isp_cfg.meas.rawaf.rawaf_sel      = af_data.rawaf_sel;
    isp_cfg.meas.rawaf.gamma_en       = af_data.gamma_en;
    isp_cfg.meas.rawaf.gaus_en        = af_data.gaus_en;
    isp_cfg.meas.rawaf.v1_fir_sel     = af_data.v1_fir_sel;
    isp_cfg.meas.rawaf.hiir_en        = af_data.hiir_en;
    isp_cfg.meas.rawaf.viir_en        = af_data.viir_en;
    isp_cfg.meas.rawaf.v1_fv_mode     = af_data.v1_fv_outmode;
    isp_cfg.meas.rawaf.v2_fv_mode     = af_data.v2_fv_outmode;
    isp_cfg.meas.rawaf.h1_fv_mode     = af_data.h1_fv_outmode;
    isp_cfg.meas.rawaf.h2_fv_mode     = af_data.h2_fv_outmode;
    isp_cfg.meas.rawaf.ldg_en         = af_data.ldg_en;
    isp_cfg.meas.rawaf.accu_8bit_mode = af_data.accu_8bit_mode;
    isp_cfg.meas.rawaf.y_mode         = af_data.y_mode;
    isp_cfg.meas.rawaf.vldg_sel       = af_data.vldg_sel;
    isp_cfg.meas.rawaf.sobel_sel      = af_data.sobel_sel;
    isp_cfg.meas.rawaf.v_dnscl_mode   = af_data.v_dnscl_mode;
    isp_cfg.meas.rawaf.from_awb       = af_data.from_awb;
    isp_cfg.meas.rawaf.from_ynr       = af_data.from_ynr;
    if (af_data.af_en) {
        isp_cfg.meas.rawaf.ae_mode = af_data.ae_mode;
        isp_cfg.meas.rawaf.ae_config_use = af_data.ae_config_use;
        isp_cfg.meas.rawaf.ae_sel = af_data.ae_sel;
    } else {
        isp_cfg.meas.rawaf.ae_mode = 0;
        isp_cfg.meas.rawaf.ae_config_use = 1;
        isp_cfg.meas.rawaf.ae_sel = 1;
    }

    memcpy(isp_cfg.meas.rawaf.line_en, af_data.line_en,
           ISP32_RAWAF_LINE_NUM * sizeof(unsigned char));
    memcpy(isp_cfg.meas.rawaf.line_num, af_data.line_num,
           ISP32_RAWAF_LINE_NUM * sizeof(unsigned char));

    isp_cfg.meas.rawaf.num_afm_win   = af_data.window_num;
    isp_cfg.meas.rawaf.win[0].h_offs = af_data.wina_h_offs;
    isp_cfg.meas.rawaf.win[0].v_offs = af_data.wina_v_offs;
    isp_cfg.meas.rawaf.win[0].h_size = af_data.wina_h_size;
    isp_cfg.meas.rawaf.win[0].v_size = af_data.wina_v_size;
    isp_cfg.meas.rawaf.win[1].h_offs = af_data.winb_h_offs;
    isp_cfg.meas.rawaf.win[1].v_offs = af_data.winb_v_offs;
    isp_cfg.meas.rawaf.win[1].h_size = af_data.winb_h_size;
    isp_cfg.meas.rawaf.win[1].v_size = af_data.winb_v_size;

    memcpy(isp_cfg.meas.rawaf.gamma_y, af_data.gamma_y,
           ISP32_RAWAF_GAMMA_NUM * sizeof(unsigned short));

    /* THRES */
    isp_cfg.meas.rawaf.afm_thres = af_data.thres;
    /* VAR_SHIFT */
    isp_cfg.meas.rawaf.afm_var_shift[0] = af_data.shift_sum_a;
    isp_cfg.meas.rawaf.lum_var_shift[0] = af_data.shift_y_a;
    isp_cfg.meas.rawaf.afm_var_shift[1] = af_data.shift_sum_b;
    isp_cfg.meas.rawaf.lum_var_shift[1] = af_data.shift_y_b;
    /* HVIIR_VAR_SHIFT */
    isp_cfg.meas.rawaf.h1iir_var_shift = af_data.h1_fv_shift;
    isp_cfg.meas.rawaf.h2iir_var_shift = af_data.h2_fv_shift;
    isp_cfg.meas.rawaf.v1iir_var_shift = af_data.v1_fv_shift;
    isp_cfg.meas.rawaf.v2iir_var_shift = af_data.v2_fv_shift;

    /* HIIR_THRESH */
    isp_cfg.meas.rawaf.h_fv_thresh = af_data.h_fv_thresh;
    isp_cfg.meas.rawaf.h_fv_limit = af_data.h_fv_limit;
    isp_cfg.meas.rawaf.h_fv_slope = af_data.h_fv_slope;
    isp_cfg.meas.rawaf.v_fv_thresh = af_data.v_fv_thresh;
    isp_cfg.meas.rawaf.v_fv_limit = af_data.v_fv_limit;
    isp_cfg.meas.rawaf.v_fv_slope = af_data.v_fv_slope;

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
    for (i = 0; i < ISP32_RAWAF_GAUS_COE_NUM; i++) {
        isp_cfg.meas.rawaf.gaus_coe[i] = af_data.gaus_coe[i];
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

    /* af32 add */
    isp_cfg.meas.rawaf.bnr2af_sel = af_data.from_bnr;
    isp_cfg.meas.rawaf.tnrin_shift = af_data.bnrin_shift;
    isp_cfg.meas.rawaf.hldg_dilate_num = af_data.hldg_dilate_num;
    isp_cfg.meas.rawaf.hiir_left_border_mode = af_data.hiir_left_border_mode;
    isp_cfg.meas.rawaf.avg_ds_en = af_data.avg_ds_en;
    isp_cfg.meas.rawaf.avg_ds_mode = af_data.avg_ds_mode;
    isp_cfg.meas.rawaf.h1_acc_mode = af_data.h1_acc_mode;
    isp_cfg.meas.rawaf.h2_acc_mode = af_data.h2_acc_mode;
    isp_cfg.meas.rawaf.v1_acc_mode = af_data.v1_acc_mode;
    isp_cfg.meas.rawaf.v2_acc_mode = af_data.v2_acc_mode;
    isp_cfg.meas.rawaf.h_fv_limit = af_data.h_fv_limit;
    isp_cfg.meas.rawaf.h_fv_slope = af_data.h_fv_slope;
    isp_cfg.meas.rawaf.v_fv_limit = af_data.v_fv_limit;
    isp_cfg.meas.rawaf.v_fv_slope = af_data.v_fv_slope;
    isp_cfg.meas.rawaf.bls_en = af_data.bls_en;
    isp_cfg.meas.rawaf.bls_offset = af_data.bls_offset;

    // rawae0 is used by af now!!!
    if (af_data.af_en && af_data.ae_mode) {
        isp_cfg.module_ens |= ISP2X_MODULE_RAWAE0;
        isp_cfg.module_en_update |= ISP2X_MODULE_RAWAE0;
        isp_cfg.module_cfg_update |= ISP2X_MODULE_RAWAE0;

        isp_cfg.meas.rawae0.win.h_offs = af_data.wina_h_offs;
        isp_cfg.meas.rawae0.win.v_offs = af_data.wina_v_offs;
        isp_cfg.meas.rawae0.win.h_size = af_data.wina_h_size;
        isp_cfg.meas.rawae0.win.v_size = af_data.wina_v_size;
    }
    mLatestMeasCfg.rawaf = isp_cfg.meas.rawaf;
    mLatestMeasCfg.rawae3 = isp_cfg.meas.rawae3;
}
#endif
#if RKAIQ_HAVE_CAC_V11
void Isp32Params::convertAiqCacToIsp32Params(struct isp32_isp_params_cfg& isp_cfg,
        struct isp32_isp_params_cfg& isp_cfg_right,
        const rk_aiq_isp_cac_v32_t& cac_cfg, bool is_multi_isp) {

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
    memcpy(cfg, &cac_cfg.cfg[0], sizeof(*cfg));
    struct isp32_cac_cfg* cfg_right = nullptr;
    if (is_multi_isp) {
        cfg_right = &isp_cfg_right.others.cac_cfg;
        memcpy(cfg_right, &cac_cfg.cfg[1], sizeof(*cfg));
    }
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
    if (is_multi_isp) {
        LOGD_ACAC("driver psf buf fd right : %d", cfg_right->buf_fd);
    }
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

#if RKAIQ_HAVE_BAYER2DNR_V23
void Isp32Params::convertAiqRawnrToIsp32Params(struct isp32_isp_params_cfg& isp_cfg,
        rk_aiq_isp_baynr_v32_t& rawnr) {
    LOGD_ANR("%s:%d enter! enable:%d \n", __FUNCTION__, __LINE__, rawnr.bayer_en);
    bool enable = rawnr.bayer_en;
    if (enable) {
        isp_cfg.module_ens |= ISP3X_MODULE_BAYNR;
    } else {
        isp_cfg.module_ens &= ~(ISP3X_MODULE_BAYNR);
    }

    isp_cfg.module_en_update |= ISP3X_MODULE_BAYNR;

#if 1
    isp_cfg.module_cfg_update |= ISP3X_MODULE_BAYNR;

    struct isp32_baynr_cfg* pBayernr = &isp_cfg.others.baynr_cfg;
    pBayernr->bay3d_gain_en          = rawnr.bay3d_gain_en;
    pBayernr->lg2_mode               = rawnr.lg2_mode;
    pBayernr->gauss_en               = rawnr.gauss_en;
    pBayernr->log_bypass             = rawnr.log_bypass;

    pBayernr->dgain0 = rawnr.dgain[0];
    pBayernr->dgain1 = rawnr.dgain[1];
    pBayernr->dgain2 = rawnr.dgain[2];

    pBayernr->pix_diff = rawnr.pix_diff;

    pBayernr->diff_thld = rawnr.diff_thld;
    pBayernr->softthld  = rawnr.softthld;

    pBayernr->bltflt_streng = rawnr.bltflt_streng;
    pBayernr->reg_w1        = rawnr.reg_w1;

    for (int i = 0; i < ISP3X_BAYNR_XY_NUM; i++) {
        pBayernr->sigma_x[i] = rawnr.sigma_x[i];
        pBayernr->sigma_y[i] = rawnr.sigma_y[i];
    }

    pBayernr->weit_d2 = rawnr.weit_d[2];
    pBayernr->weit_d1 = rawnr.weit_d[1];
    pBayernr->weit_d0 = rawnr.weit_d[0];

    pBayernr->lg2_lgoff = rawnr.lg2_lgoff;
    pBayernr->lg2_off   = rawnr.lg2_off;

    pBayernr->dat_max = rawnr.dat_max;

    pBayernr->rgain_off = rawnr.rgain_off;
    pBayernr->bgain_off = rawnr.bgain_off;

    for (int i = 0; i < ISP32_BAYNR_GAIN_NUM; i++) {
        pBayernr->gain_x[i] = rawnr.gain_x[i];
        pBayernr->gain_y[i] = rawnr.gain_y[i];
    }
#endif
    LOGD_ANR("%s:%d exit!\n", __FUNCTION__, __LINE__);
}
#endif
#if (RKAIQ_HAVE_BAYERTNR_V23 || RKAIQ_HAVE_BAYERTNR_V23_LITE)
#if USE_NEWSTRUCT
void Isp32Params::convertAiqBtnrToIsp32Params(struct isp32_isp_params_cfg& isp_cfg,
        rk_aiq_isp_btnr_params_t* btnr_attr)
{
    if (btnr_attr->en) {
        isp_cfg.module_ens |= ISP3X_MODULE_BAY3D;
        // bayer3dnr enable  bayer2dnr must enable at the same time
        isp_cfg.module_ens |= ISP3X_MODULE_BAYNR;
        isp_cfg.module_en_update |= ISP3X_MODULE_BAYNR;
    } else {
        // tnr can't open/close in runtime.
        isp_cfg.module_ens |= ISP3X_MODULE_BAY3D;
        return;
    }

    rk_aiq_btnr32_params_cvt(&btnr_attr->result, &isp_cfg, btnr_attr->bypass);
}
#else
void Isp32Params::convertAiqTnrToIsp32Params(struct isp32_isp_params_cfg& isp_cfg,
        rk_aiq_isp_tnr_v32_t& tnr) {
    LOGD_ANR("%s:%d enter! enable:%d\n", __FUNCTION__, __LINE__, tnr.bay3d_en);
    bool enable = tnr.bay3d_en;
    if (enable) {
        isp_cfg.module_ens |= ISP3X_MODULE_BAY3D;
        // bayer3dnr enable  bayer2dnr must enable at the same time
        isp_cfg.module_ens |= ISP3X_MODULE_BAYNR;
        isp_cfg.module_en_update |= ISP3X_MODULE_BAYNR;
    } else {
        // isp_cfg.module_ens &= ~(ISP3X_MODULE_BAY3D);
        isp_cfg.module_ens |= ISP3X_MODULE_BAY3D;
    }

    isp_cfg.module_en_update |= ISP3X_MODULE_BAY3D;

#if 1
    isp_cfg.module_cfg_update |= ISP3X_MODULE_BAY3D;

    struct isp32_bay3d_cfg* pBayertnr = &isp_cfg.others.bay3d_cfg;

    pBayertnr->bwsaving_en       = tnr.bwsaving_en;
    pBayertnr->bypass_en         = tnr.bypass_en;
    pBayertnr->hibypass_en       = tnr.hibypass_en;
    pBayertnr->lobypass_en       = tnr.lobypass_en;
    pBayertnr->himed_bypass_en   = tnr.himed_bypass_en;
    pBayertnr->higaus_bypass_en  = tnr.higaus_bypass_en;
    pBayertnr->hiabs_possel      = tnr.hiabs_possel;
    pBayertnr->hichnsplit_en     = tnr.hichnsplit_en;
    pBayertnr->lomed_bypass_en   = tnr.lomed_bypass_en;
    pBayertnr->logaus5_bypass_en = tnr.logaus5_bypass_en;
    pBayertnr->logaus3_bypass_en = tnr.logaus3_bypass_en;
    pBayertnr->glbpk_en          = tnr.glbpk_en;
    pBayertnr->loswitch_protect  = tnr.loswitch_protect;

    pBayertnr->softwgt  = tnr.softwgt;
    pBayertnr->hidif_th = tnr.hidif_th;

    pBayertnr->glbpk2 = tnr.glbpk2;

    /* BAY3D_CTRL1 */
    pBayertnr->hiwgt_opt_en = tnr.hiwgt_opt_en;
    pBayertnr->hichncor_en = tnr.hichncor_en;
    pBayertnr->bwopt_gain_dis = tnr.bwopt_gain_dis;
    pBayertnr->lo4x8_en = tnr.lo4x8_en;
    pBayertnr->lo4x4_en = tnr.lo4x4_en;
    pBayertnr->hisig_ind_sel = tnr.hisig_ind_sel;
    pBayertnr->pksig_ind_sel = tnr.pksig_ind_sel;
    pBayertnr->iirwr_rnd_en = tnr.iirwr_rnd_en;
    pBayertnr->curds_high_en = tnr.curds_high_en;
    pBayertnr->higaus3_mode = tnr.higaus3_mode;
    pBayertnr->higaus5x5_en = tnr.higaus5x5_en;
    pBayertnr->wgtmix_opt_en = tnr.wgtmix_opt_en;

    pBayertnr->wgtmm_opt_en = tnr.wgtmm_opt_en;
    pBayertnr->wgtmm_sel_en = tnr.wgtmm_sel_en;

    pBayertnr->wgtlmt   = tnr.wgtlmt;
    pBayertnr->wgtratio = tnr.wgtratio;

    for (int i = 0; i < ISP3X_BAY3D_XY_NUM; i++) {
        pBayertnr->sig0_x[i] = tnr.sig0_x[i];
        pBayertnr->sig0_y[i] = tnr.sig0_y[i];

        pBayertnr->sig1_x[i] = tnr.sig1_x[i];
        pBayertnr->sig1_y[i] = tnr.sig1_y[i];

        // pBayertnr->sig2_x[i] = tnr.bay3d_sig1_x[i];
        pBayertnr->sig2_y[i] = tnr.sig2_y[i];
    }

    pBayertnr->wgtmin = tnr.wgtmin;

    pBayertnr->hisigrat0 = tnr.hisigrat0;
    pBayertnr->hisigrat1 = tnr.hisigrat1;

    pBayertnr->hisigoff0 = tnr.hisigoff0;
    pBayertnr->hisigoff1 = tnr.hisigoff1;

    pBayertnr->losigoff = tnr.losigoff;
    pBayertnr->losigrat = tnr.losigrat;

    pBayertnr->rgain_off = tnr.rgain_off;
    pBayertnr->bgain_off = tnr.bgain_off;

    pBayertnr->siggaus0 = tnr.siggaus0;
    pBayertnr->siggaus1 = tnr.siggaus1;
    pBayertnr->siggaus2 = tnr.siggaus2;
    pBayertnr->siggaus3 = tnr.siggaus3;
#endif
    LOGD_ANR("%s:%d exit!\n", __FUNCTION__, __LINE__);
}
#endif
#endif

#if RKAIQ_HAVE_YNR_V22
#if USE_NEWSTRUCT
void Isp32Params::convertAiqYnrToIsp32Params(struct isp32_isp_params_cfg& isp_cfg,
        rk_aiq_isp_ynr_params_t *ynr_attr) {

    if (ynr_attr->en) {
        isp_cfg.module_ens |= ISP3X_MODULE_YNR;
        isp_cfg.module_en_update |= ISP3X_MODULE_YNR;
        isp_cfg.module_cfg_update |= ISP3X_MODULE_YNR;
    } else {
        isp_cfg.module_ens &=~ ISP3X_MODULE_YNR;
        return;
    }

    rk_aiq_ynr32_params_cvt(&ynr_attr->result, &isp_cfg, &mCommonCvtInfo);
}
#else
void Isp32Params::convertAiqYnrToIsp32Params(struct isp32_isp_params_cfg& isp_cfg,
        rk_aiq_isp_ynr_v32_t& ynr) {
    LOGD_ANR("%s:%d enter! enable:%d \n", __FUNCTION__, __LINE__, ynr.ynr_en);

    bool enable = ynr.ynr_en;

    isp_cfg.module_en_update |= ISP3X_MODULE_YNR;
    isp_cfg.module_ens |= ISP3X_MODULE_YNR;

#if 1

    isp_cfg.module_cfg_update |= ISP3X_MODULE_YNR;

    struct isp32_ynr_cfg* pYnr = &isp_cfg.others.ynr_cfg;

    pYnr->rnr_en            = ynr.rnr_en;
    pYnr->thumb_mix_cur_en  = ynr.thumb_mix_cur_en;
    pYnr->global_gain_alpha = ynr.global_gain_alpha;
    pYnr->global_gain       = ynr.global_gain;
    pYnr->flt1x1_bypass_sel = ynr.flt1x1_bypass_sel;
    pYnr->nlm11x11_bypass   = ynr.nlm11x11_bypass;
    pYnr->flt1x1_bypass_sel = ynr.flt1x1_bypass_sel;
    pYnr->flt1x1_bypass     = ynr.flt1x1_bypass;
    pYnr->lgft3x3_bypass    = ynr.lgft3x3_bypass;
    pYnr->lbft5x5_bypass    = ynr.lbft5x5_bypass;
    pYnr->bft3x3_bypass     = ynr.bft3x3_bypass;
    if (!enable) {
        pYnr->nlm11x11_bypass  = 0x01;
        pYnr->flt1x1_bypass  = 0x01;
        pYnr->lgft3x3_bypass = 0x01;
        pYnr->lbft5x5_bypass = 0x01;
        pYnr->bft3x3_bypass  = 0x01;
    }

    pYnr->rnr_max_r       = ynr.rnr_max_r;
    pYnr->local_gainscale = ynr.local_gainscale;

    pYnr->rnr_center_coorh = ynr.rnr_center_coorh;
    pYnr->rnr_center_coorv = ynr.rnr_center_coorv;

    pYnr->loclagain_adj_thresh = ynr.localgain_adj_thresh;
    pYnr->localgain_adj        = ynr.localgain_adj;

    pYnr->low_bf_inv1 = ynr.low_bf_inv1;
    pYnr->low_bf_inv0 = ynr.low_bf_inv0;

    pYnr->low_peak_supress = ynr.low_peak_supress;
    pYnr->low_thred_adj    = ynr.low_thred_adj;

    pYnr->low_dist_adj        = ynr.low_dist_adj;
    pYnr->low_edge_adj_thresh = ynr.low_edge_adj_thresh;

    pYnr->low_bi_weight     = ynr.low_bi_weight;
    pYnr->low_weight        = ynr.low_weight;
    pYnr->low_center_weight = ynr.low_center_weight;

    pYnr->frame_full_size  = ynr.frame_full_size;
    pYnr->lbf_weight_thres = ynr.lbf_weight_thres;

    pYnr->low_gauss1_coeff2 = ynr.low_gauss1_coeff2;
    pYnr->low_gauss1_coeff1 = ynr.low_gauss1_coeff1;
    pYnr->low_gauss1_coeff0 = ynr.low_gauss1_coeff0;

    pYnr->low_gauss2_coeff2 = ynr.low_gauss2_coeff2;
    pYnr->low_gauss2_coeff1 = ynr.low_gauss2_coeff1;
    pYnr->low_gauss2_coeff0 = ynr.low_gauss2_coeff0;

    for (int i = 0; i < ISP3X_YNR_XY_NUM; i++) {
        pYnr->luma_points_x[i] = ynr.luma_points_x[i];
        pYnr->lsgm_y[i]        = ynr.lsgm_y[i];
        //pYnr->hsgm_y[i]        = ynr.ynr_hsgm_y[i];
        pYnr->rnr_strength3[i] = ynr.rnr_strength[i];
    }

    pYnr->nlm_min_sigma      = ynr.nlm_min_sigma;
    pYnr->nlm_hi_gain_alpha  = ynr.nlm_hi_gain_alpha;
    pYnr->nlm_hi_bf_scale    = ynr.nlm_hi_bf_scale;

    pYnr->nlm_coe[0]  = ynr.nlm_coe_0;
    pYnr->nlm_coe[1]  = ynr.nlm_coe_1;
    pYnr->nlm_coe[2]  = ynr.nlm_coe_2;
    pYnr->nlm_coe[3]  = ynr.nlm_coe_3;
    pYnr->nlm_coe[4]  = ynr.nlm_coe_4;
    pYnr->nlm_coe[5]  = ynr.nlm_coe_5;

    pYnr->nlm_center_weight  = ynr.nlm_center_weight;
    pYnr->nlm_weight_offset  = ynr.nlm_weight_offset;

    pYnr->nlm_nr_weight      = ynr.nlm_nr_weight;
#endif
    LOGD_ANR("%s:%d exit!\n", __FUNCTION__, __LINE__);
}
#endif
#endif
#if (RKAIQ_HAVE_CNR_V30 || RKAIQ_HAVE_CNR_V30_LITE)
#if USE_NEWSTRUCT
void Isp32Params::convertAiqCnrToIsp32Params(struct isp32_isp_params_cfg& isp_cfg,
        rk_aiq_isp_cnr_params_t *attr) {

    isp_cfg.module_ens |= ISP3X_MODULE_CNR;
    isp_cfg.module_en_update |= ISP3X_MODULE_CNR;
    isp_cfg.module_cfg_update |= ISP3X_MODULE_CNR;

}
#else
void Isp32Params::convertAiqUvnrToIsp32Params(struct isp32_isp_params_cfg& isp_cfg,
        rk_aiq_isp_cnr_v32_t& uvnr) {
    LOGD_ANR("%s:%d enter! enable:%d \n", __FUNCTION__, __LINE__, uvnr.cnr_en);

    bool enable = uvnr.cnr_en;

    isp_cfg.module_en_update |= ISP3X_MODULE_CNR;
    isp_cfg.module_ens &=~ ISP3X_MODULE_CNR;


#if 0

    isp_cfg.module_cfg_update |= ISP3X_MODULE_CNR;
    struct isp32_cnr_cfg* pCnr = &isp_cfg.others.cnr_cfg;

    pCnr->exgain_bypass = uvnr.exgain_bypass;
    pCnr->yuv422_mode   = uvnr.yuv422_mode;
    pCnr->thumb_mode    = uvnr.thumb_mode;
    pCnr->bf3x3_wgt0_sel = uvnr.bf3x3_wgt0_sel;

    pCnr->gain_iso          = uvnr.gain_iso;
    pCnr->global_gain_alpha = uvnr.global_gain_alpha;
    pCnr->global_gain       = uvnr.global_gain;

    pCnr->thumb_sigma_c    = uvnr.thumb_sigma_c;
    pCnr->thumb_sigma_y = uvnr.thumb_sigma_y;

    pCnr->thumb_bf_ratio = uvnr.thumb_bf_ratio;

    pCnr->lbf1x7_weit_d0 = uvnr.lbf1x7_weit_d[0];
    pCnr->lbf1x7_weit_d1 = uvnr.lbf1x7_weit_d[1];
    pCnr->lbf1x7_weit_d2 = uvnr.lbf1x7_weit_d[2];
    pCnr->lbf1x7_weit_d3 = uvnr.lbf1x7_weit_d[3];

    pCnr->iir_uvgain = uvnr.iir_uvgain;
    pCnr->iir_strength = uvnr.iir_strength;
    pCnr->exp_shift = uvnr.exp_shift;
    pCnr->wgt_slope = uvnr.wgt_slope;

    pCnr->chroma_ghost = uvnr.chroma_ghost;
    pCnr->iir_uv_clip = uvnr.iir_uv_clip;

    for (int i = 0; i < 6; i++) {
        pCnr->gaus_coe[i] = uvnr.gaus_coe[i];
    }

    pCnr->gaus_ratio  = uvnr.gaus_ratio;
    pCnr->bf_wgt_clip    = uvnr.bf_wgt_clip;
    pCnr->global_alpha = uvnr.global_alpha;
    if (!enable) {
        pCnr->global_alpha = 0x000;
        LOGD_ANR("0x0024 global_alpha:0x%x 0x%x\n", uvnr.global_alpha, pCnr->global_alpha);
    }

    pCnr->uv_gain    = uvnr.uv_gain;
    pCnr->sigma_r  = uvnr.sigma_r;
    pCnr->bf_ratio    = uvnr.bf_ratio;

    pCnr->adj_offset = uvnr.adj_offset;
    pCnr->adj_ratio = uvnr.adj_ratio;

    for (int i = 0; i < ISP32_CNR_SIGMA_Y_NUM; i++) {
        pCnr->sigma_y[i] = uvnr.sigma_y[i];
    }

    pCnr->iir_gain_alpha = uvnr.iir_gain_alpha;
    pCnr->iir_global_gain = uvnr.iir_global_gain;
#endif
    LOGD_ANR("%s:%d exit!\n", __FUNCTION__, __LINE__);
}
#endif
#endif

#if RKAIQ_HAVE_DEBAYER_V2 || RKAIQ_HAVE_DEBAYER_V2_LITE
#if USE_NEWSTRUCT
void Isp32Params::convertAiqDmToIsp32Params(struct isp32_isp_params_cfg& isp_cfg,
        rk_aiq_isp_dm_params_t* dm_attr)
{
    if (dm_attr->en) {
        isp_cfg.module_ens |= ISP2X_MODULE_DEBAYER;
        isp_cfg.module_en_update |= ISP2X_MODULE_DEBAYER;
        isp_cfg.module_cfg_update |= ISP2X_MODULE_DEBAYER;
    } else {
        isp_cfg.module_ens &= ~ISP2X_MODULE_DEBAYER;
        isp_cfg.module_en_update |= ISP2X_MODULE_DEBAYER;
        return;
    }

    rk_aiq_dm21_params_cvt(&dm_attr->result, &isp_cfg);
}

#else
void Isp32Params::convertAiqAdebayerToIsp32Params(struct isp32_isp_params_cfg& isp_cfg,
        rk_aiq_isp_debayer_v32_t & debayer)
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

    /* G_INTERP */
    isp_cfg.others.debayer_cfg.clip_en = debayer.clip_en;
    isp_cfg.others.debayer_cfg.dist_scale = debayer.dist_scale;
    isp_cfg.others.debayer_cfg.thed0 = debayer.thed0;
    isp_cfg.others.debayer_cfg.thed1 = debayer.thed1;
    isp_cfg.others.debayer_cfg.select_thed = debayer.select_thed;
    isp_cfg.others.debayer_cfg.max_ratio = debayer.max_ratio;

    /* G_INTERP_FILTER1 */
    isp_cfg.others.debayer_cfg.filter1_coe1 = debayer.filter1_coe[0];
    isp_cfg.others.debayer_cfg.filter1_coe2 = debayer.filter1_coe[1];
    isp_cfg.others.debayer_cfg.filter1_coe3 = debayer.filter1_coe[2];
    isp_cfg.others.debayer_cfg.filter1_coe4 = debayer.filter1_coe[3];

    /* G_INTERP_FILTER2 */
    isp_cfg.others.debayer_cfg.filter2_coe1 = debayer.filter2_coe[0];
    isp_cfg.others.debayer_cfg.filter2_coe2 = debayer.filter2_coe[1];
    isp_cfg.others.debayer_cfg.filter2_coe3 = debayer.filter2_coe[2];
    isp_cfg.others.debayer_cfg.filter2_coe4 = debayer.filter2_coe[3];

    /* G_INTERP_OFFSET */
    isp_cfg.others.debayer_cfg.hf_offset = debayer.hf_offset;
    isp_cfg.others.debayer_cfg.gain_offset = debayer.gain_offset;

    /* G_FILTER_OFFSET */
    isp_cfg.others.debayer_cfg.offset = debayer.offset;

#if RKAIQ_HAVE_DEBAYER_V2
    /* C_FILTER_GUIDE_GAUS */
    isp_cfg.others.debayer_cfg.guid_gaus_coe0 = debayer.guid_gaus_coe[0];
    isp_cfg.others.debayer_cfg.guid_gaus_coe1 = debayer.guid_gaus_coe[1];
    isp_cfg.others.debayer_cfg.guid_gaus_coe2 = debayer.guid_gaus_coe[2];

    /* C_FILTER_CE_GAUS */
    isp_cfg.others.debayer_cfg.ce_gaus_coe0 = debayer.ce_gaus_coe[0];
    isp_cfg.others.debayer_cfg.ce_gaus_coe1 = debayer.ce_gaus_coe[1];
    isp_cfg.others.debayer_cfg.ce_gaus_coe2 = debayer.ce_gaus_coe[2];

    /* C_FILTER_ALPHA_GAUS */
    isp_cfg.others.debayer_cfg.alpha_gaus_coe0 = debayer.alpha_gaus_coe[0];
    isp_cfg.others.debayer_cfg.alpha_gaus_coe1 = debayer.alpha_gaus_coe[1];
    isp_cfg.others.debayer_cfg.alpha_gaus_coe2 = debayer.alpha_gaus_coe[2];

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
    isp_cfg.others.debayer_cfg.loghf_offset = debayer.loghf_offset;
    isp_cfg.others.debayer_cfg.loggd_offset = debayer.loggd_offset;

    /* C_FILTER_ALPHA */
    isp_cfg.others.debayer_cfg.alpha_offset = debayer.alpha_offset;
    isp_cfg.others.debayer_cfg.alpha_scale = debayer.alpha_scale;

    /* C_FILTER_EDGE */
    isp_cfg.others.debayer_cfg.edge_offset = debayer.edge_offset;
    isp_cfg.others.debayer_cfg.edge_scale = debayer.edge_scale;
#endif
}
#endif //USE_NEWSTRUCT
#endif

#if RKAIQ_HAVE_MERGE_V12
void Isp32Params::convertAiqMergeToIsp32Params(struct isp32_isp_params_cfg& isp_cfg,
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

#if RKAIQ_HAVE_DEHAZE_V12
#ifdef USE_NEWSTRUCT
void Isp32Params::convertAiqDehazeToIsp32Params(struct isp32_isp_params_cfg& isp_cfg,
        rk_aiq_isp_dehaze_params_t* dehaze_attr) {

    //hwi_api_dehaze22_params_check(&dehaze_attr->result);

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

    //rk_aiq_dehaze22_params_cvt(&dehaze_attr->result, &isp_cfg);
    //hwi_api_dehaze22_params_dump(&dehaze_attr->result, &isp_cfg.others.dhaz_cfg);
}
#else
void Isp32Params::convertAiqAdehazeToIsp32Params(struct isp32_isp_params_cfg& isp_cfg,
        const rk_aiq_isp_dehaze_v32_t& dhaze) {
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

    struct isp32_dhaz_cfg* cfg = &isp_cfg.others.dhaz_cfg;

    cfg->round_en        = dhaze.ProcResV12.round_en;
    cfg->soft_wr_en      = dhaze.ProcResV12.soft_wr_en;
    cfg->enhance_en      = dhaze.ProcResV12.enhance_en;
    cfg->air_lc_en       = dhaze.ProcResV12.air_lc_en;
    cfg->hpara_en        = dhaze.ProcResV12.hpara_en;
    cfg->hist_en         = dhaze.ProcResV12.hist_en;
    cfg->dc_en           = dhaze.ProcResV12.dc_en;
    cfg->yblk_th         = dhaze.ProcResV12.yblk_th;
    cfg->yhist_th        = dhaze.ProcResV12.yhist_th;
    cfg->dc_max_th       = dhaze.ProcResV12.dc_max_th;
    cfg->dc_min_th       = dhaze.ProcResV12.dc_min_th;
    cfg->wt_max          = dhaze.ProcResV12.wt_max;
    cfg->bright_max      = dhaze.ProcResV12.bright_max;
    cfg->bright_min      = dhaze.ProcResV12.bright_min;
    cfg->tmax_base       = dhaze.ProcResV12.tmax_base;
    cfg->dark_th         = dhaze.ProcResV12.dark_th;
    cfg->air_max         = dhaze.ProcResV12.air_max;
    cfg->air_min         = dhaze.ProcResV12.air_min;
    cfg->tmax_max        = dhaze.ProcResV12.tmax_max;
    cfg->tmax_off        = dhaze.ProcResV12.tmax_off;
    cfg->hist_k          = dhaze.ProcResV12.hist_k;
    cfg->hist_th_off     = dhaze.ProcResV12.hist_th_off;
    cfg->hist_min        = dhaze.ProcResV12.hist_min;
    cfg->hist_gratio     = dhaze.ProcResV12.hist_gratio;
    cfg->hist_scale      = dhaze.ProcResV12.hist_scale;
    cfg->enhance_value   = dhaze.ProcResV12.enhance_value;
    cfg->enhance_chroma  = dhaze.ProcResV12.enhance_chroma;
    cfg->iir_wt_sigma    = dhaze.ProcResV12.iir_wt_sigma;
    cfg->iir_sigma       = dhaze.ProcResV12.iir_sigma;
    cfg->stab_fnum       = dhaze.ProcResV12.stab_fnum;
    cfg->iir_tmax_sigma  = dhaze.ProcResV12.iir_tmax_sigma;
    cfg->iir_air_sigma   = dhaze.ProcResV12.iir_air_sigma;
    cfg->iir_pre_wet     = dhaze.ProcResV12.iir_pre_wet;
    cfg->cfg_wt          = dhaze.ProcResV12.cfg_wt;
    cfg->cfg_air         = dhaze.ProcResV12.cfg_air;
    cfg->cfg_alpha       = dhaze.ProcResV12.cfg_alpha;
    cfg->cfg_gratio      = dhaze.ProcResV12.cfg_gratio;
    cfg->cfg_tmax        = dhaze.ProcResV12.cfg_tmax;
    cfg->range_sima      = dhaze.ProcResV12.range_sima;
    cfg->space_sigma_cur = dhaze.ProcResV12.space_sigma_cur;
    cfg->space_sigma_pre = dhaze.ProcResV12.space_sigma_pre;
    cfg->dc_weitcur      = dhaze.ProcResV12.dc_weitcur;
    cfg->bf_weight       = dhaze.ProcResV12.bf_weight;
    cfg->gaus_h0         = dhaze.ProcResV12.gaus_h0;
    cfg->gaus_h1         = dhaze.ProcResV12.gaus_h1;
    cfg->gaus_h2         = dhaze.ProcResV12.gaus_h2;

    for (int i = 0; i < ISP32_DHAZ_SIGMA_IDX_NUM; i++)
        cfg->sigma_idx[i] = dhaze.ProcResV12.sigma_idx[i];

    for (int i = 0; i < ISP32_DHAZ_ENH_CURVE_NUM; i++)
        cfg->enh_curve[i] = dhaze.ProcResV12.enh_curve[i];

    for (int i = 0; i < ISP32_DHAZ_SIGMA_LUT_NUM; i++)
        cfg->sigma_lut[i] = dhaze.ProcResV12.sigma_lut[i];

    for (int i = 0; i < ISP32_DHAZ_HIST_WR_NUM; i++) cfg->hist_wr[i] = dhaze.ProcResV12.hist_wr[i];

    // dehaze v12 add
    cfg->enh_luma_en      = dhaze.ProcResV12.enh_luma_en;
    cfg->color_deviate_en = dhaze.ProcResV12.color_deviate_en;
    for (int i = 0; i < ISP32_DHAZ_ENH_LUMA_NUM; i++)
        cfg->enh_luma[i] = dhaze.ProcResV12.enh_luma[i];

    LOG1_ADEHAZE(
        "%s: color_deviate_en:%d round_en:%d soft_wr_en:%d enhance_en:%d air_lc_en:%d hpara_en:%d "
        "hist_en:%d "
        "dc_en:%d\n",
        __func__, cfg->color_deviate_en, cfg->round_en, cfg->soft_wr_en, cfg->enhance_en,
        cfg->air_lc_en, cfg->hpara_en, cfg->hist_en, cfg->dc_en);
    LOG1_ADEHAZE(
        "%s: yblk_th:%d yhist_th:%d dc_max_th:%d dc_min_th:%d wt_max:%d bright_max:%d "
        "bright_min:%d \n",
        __func__, cfg->yblk_th, cfg->yhist_th, cfg->dc_max_th, cfg->dc_min_th, cfg->wt_max,
        cfg->bright_max, cfg->bright_min);
    LOG1_ADEHAZE("%s: tmax_base:%d dark_th:%d air_max:%d air_min:%d tmax_max:%d tmax_off:%d \n",
                 __func__, cfg->tmax_base, cfg->dark_th, cfg->air_max, cfg->air_min, cfg->tmax_max,
                 cfg->tmax_off);
    LOG1_ADEHAZE("%s: hist_k:%d hist_th_off:%d hist_min:%d hist_gratio:%d hist_scale:%d \n",
                 __func__, cfg->hist_k, cfg->hist_th_off, cfg->hist_min, cfg->hist_gratio,
                 cfg->hist_scale);
    LOG1_ADEHAZE("%s: gaus_h0:%d gaus_h1:%d gaus_h2:%d enhance_value:%d enhance_chroma:%d \n",
                 __func__, cfg->gaus_h0, cfg->gaus_h1, cfg->gaus_h2, cfg->enhance_value,
                 cfg->enhance_chroma);
    LOG1_ADEHAZE(
        "%s: iir_wt_sigma:%d iir_sigma:%d stab_fnum:%d iir_tmax_sigma:%d iir_air_sigma:%d "
        "iir_pre_wet:%d \n",
        __func__, cfg->iir_wt_sigma, cfg->iir_sigma, cfg->stab_fnum, cfg->iir_tmax_sigma,
        cfg->iir_air_sigma, cfg->iir_pre_wet);
    LOG1_ADEHAZE("%s: cfg_alpha:%d cfg_wt:%d cfg_air:%d cfg_gratio:%d cfg_tmax:%d \n", __func__,
                 cfg->cfg_alpha, cfg->cfg_wt, cfg->cfg_air, cfg->cfg_gratio, cfg->cfg_tmax);
    LOG1_ADEHAZE(
        "%s: range_sima:%d space_sigma_cur:%d space_sigma_pre:%d dc_weitcur:%d bf_weight:%d \n",
        __func__, cfg->range_sima, cfg->space_sigma_cur, cfg->space_sigma_pre, cfg->dc_weitcur,
        cfg->bf_weight);
    LOG1_ADEHAZE("%s: sigma_idx: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n", __FUNCTION__,
                 cfg->sigma_idx[0], cfg->sigma_idx[1], cfg->sigma_idx[2], cfg->sigma_idx[3],
                 cfg->sigma_idx[4], cfg->sigma_idx[5], cfg->sigma_idx[6], cfg->sigma_idx[7],
                 cfg->sigma_idx[8], cfg->sigma_idx[9], cfg->sigma_idx[10], cfg->sigma_idx[11],
                 cfg->sigma_idx[12], cfg->sigma_idx[13], cfg->sigma_idx[14]);
    LOG1_ADEHAZE("%s: sigma_lut: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
                 __FUNCTION__, cfg->sigma_lut[0], cfg->sigma_lut[1], cfg->sigma_lut[2],
                 cfg->sigma_lut[3], cfg->sigma_lut[4], cfg->sigma_lut[5], cfg->sigma_lut[6],
                 cfg->sigma_lut[7], cfg->sigma_lut[8], cfg->sigma_lut[9], cfg->sigma_lut[10],
                 cfg->sigma_lut[11], cfg->sigma_lut[12], cfg->sigma_lut[13], cfg->sigma_lut[14],
                 cfg->sigma_lut[15], cfg->sigma_lut[16]);
    LOG1_ADEHAZE("%s: enh_curve: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
                 __FUNCTION__, cfg->enh_curve[0], cfg->enh_curve[1], cfg->enh_curve[2],
                 cfg->enh_curve[3], cfg->enh_curve[4], cfg->enh_curve[5], cfg->enh_curve[6],
                 cfg->enh_curve[7], cfg->enh_curve[8], cfg->enh_curve[9], cfg->enh_curve[10],
                 cfg->enh_curve[11], cfg->enh_curve[12], cfg->enh_curve[13], cfg->enh_curve[14],
                 cfg->enh_curve[15], cfg->enh_curve[16]);
    LOG1_ADEHAZE(
        "%s: enh_luma_en:%d enh_luma: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
        __FUNCTION__, cfg->enh_luma_en, cfg->enh_luma[0], cfg->enh_luma[1], cfg->enh_luma[2],
        cfg->enh_luma[3], cfg->enh_luma[4], cfg->enh_luma[5], cfg->enh_luma[6], cfg->enh_luma[7],
        cfg->enh_luma[8], cfg->enh_luma[9], cfg->enh_luma[10], cfg->enh_luma[11], cfg->enh_luma[12],
        cfg->enh_luma[13], cfg->enh_luma[14], cfg->enh_luma[15], cfg->enh_luma[16]);
}
#endif
#endif

#if RKAIQ_HAVE_DRC_V12 || RKAIQ_HAVE_DRC_V12_LITE
#ifdef USE_NEWSTRUCT
void Isp32Params::convertAiqDrcToIsp32Params(struct isp32_isp_params_cfg& isp_cfg,
        rk_aiq_isp_drc_params_t* drc_attr) {
    //hwi_api_drc32_params_check(&drc_attr->result);

    if (drc_attr->en) {
        isp_cfg.module_en_update |= 1LL << Rk_ISP21_DRC_ID;
        isp_cfg.module_ens |= 1LL << Rk_ISP21_DRC_ID;
        isp_cfg.module_cfg_update |= 1LL << Rk_ISP21_DRC_ID;
    } else {
        isp_cfg.module_en_update |= 1LL << Rk_ISP21_DRC_ID;
        isp_cfg.module_ens &= ~(1LL << Rk_ISP21_DRC_ID);
        isp_cfg.module_cfg_update &= ~(1LL << Rk_ISP21_DRC_ID);
        return;
    }

    //hwi_api_drc32_params_cvt(&drc_attr->result, &isp_cfg);
    //hwi_api_drc32_params_dump(&drc_attr->result, &isp_cfg.others.drc_cfg);
}
#else
void Isp32Params::convertAiqDrcToIsp32Params(struct isp32_isp_params_cfg& isp_cfg,
        rk_aiq_isp_drc_v32_t& adrc_data) {
    if (adrc_data.bDrcEn) {
        isp_cfg.module_en_update |= 1LL << Rk_ISP21_DRC_ID;
        isp_cfg.module_ens |= 1LL << Rk_ISP21_DRC_ID;
        isp_cfg.module_cfg_update |= 1LL << Rk_ISP21_DRC_ID;
    } else {
        isp_cfg.module_en_update |= 1LL << Rk_ISP21_DRC_ID;
        isp_cfg.module_ens &= ~(1LL << Rk_ISP21_DRC_ID);
        isp_cfg.module_cfg_update &= ~(1LL << Rk_ISP21_DRC_ID);
        return;
    }

    isp_cfg.others.drc_cfg.bypass_en       = adrc_data.DrcProcRes.Drc_v12.bypass_en;
    isp_cfg.others.drc_cfg.offset_pow2     = adrc_data.DrcProcRes.Drc_v12.offset_pow2;
    isp_cfg.others.drc_cfg.compres_scl     = adrc_data.DrcProcRes.Drc_v12.compres_scl;
    isp_cfg.others.drc_cfg.position        = adrc_data.DrcProcRes.Drc_v12.position;
    isp_cfg.others.drc_cfg.delta_scalein   = adrc_data.DrcProcRes.Drc_v12.delta_scalein;
    isp_cfg.others.drc_cfg.hpdetail_ratio  = adrc_data.DrcProcRes.Drc_v12.hpdetail_ratio;
    isp_cfg.others.drc_cfg.lpdetail_ratio  = adrc_data.DrcProcRes.Drc_v12.lpdetail_ratio;
    isp_cfg.others.drc_cfg.weicur_pix      = adrc_data.DrcProcRes.Drc_v12.weicur_pix;
    isp_cfg.others.drc_cfg.weipre_frame    = adrc_data.DrcProcRes.Drc_v12.weipre_frame;
    isp_cfg.others.drc_cfg.bilat_wt_off    = adrc_data.DrcProcRes.Drc_v12.bilat_wt_off;
    isp_cfg.others.drc_cfg.force_sgm_inv0  = adrc_data.DrcProcRes.Drc_v12.force_sgm_inv0;
    isp_cfg.others.drc_cfg.motion_scl      = adrc_data.DrcProcRes.Drc_v12.motion_scl;
    isp_cfg.others.drc_cfg.edge_scl        = adrc_data.DrcProcRes.Drc_v12.edge_scl;
    isp_cfg.others.drc_cfg.space_sgm_inv1  = adrc_data.DrcProcRes.Drc_v12.space_sgm_inv1;
    isp_cfg.others.drc_cfg.space_sgm_inv0  = adrc_data.DrcProcRes.Drc_v12.space_sgm_inv0;
    isp_cfg.others.drc_cfg.range_sgm_inv1  = adrc_data.DrcProcRes.Drc_v12.range_sgm_inv1;
    isp_cfg.others.drc_cfg.range_sgm_inv0  = adrc_data.DrcProcRes.Drc_v12.range_sgm_inv0;
    isp_cfg.others.drc_cfg.weig_maxl       = adrc_data.DrcProcRes.Drc_v12.weig_maxl;
    isp_cfg.others.drc_cfg.weig_bilat      = adrc_data.DrcProcRes.Drc_v12.weig_bilat;
    isp_cfg.others.drc_cfg.enable_soft_thd = adrc_data.DrcProcRes.Drc_v12.enable_soft_thd;
    isp_cfg.others.drc_cfg.bilat_soft_thd  = adrc_data.DrcProcRes.Drc_v12.bilat_soft_thd;
    isp_cfg.others.drc_cfg.iir_weight      = adrc_data.DrcProcRes.Drc_v12.iir_weight;
    isp_cfg.others.drc_cfg.min_ogain       = adrc_data.DrcProcRes.Drc_v12.min_ogain;

    for (int i = 0; i < ISP32_DRC_Y_NUM; i++) {
        isp_cfg.others.drc_cfg.gain_y[i]    = adrc_data.DrcProcRes.Drc_v12.gain_y[i];
        isp_cfg.others.drc_cfg.compres_y[i] = adrc_data.DrcProcRes.Drc_v12.compres_y[i];
        isp_cfg.others.drc_cfg.scale_y[i]   = adrc_data.DrcProcRes.Drc_v12.scale_y[i];
    }

    // drc v12 add
    isp_cfg.others.drc_cfg.gas_t           = adrc_data.DrcProcRes.Drc_v12.gas_t;
    isp_cfg.others.drc_cfg.gas_l0          = adrc_data.DrcProcRes.Drc_v12.gas_l0;
    isp_cfg.others.drc_cfg.gas_l1          = adrc_data.DrcProcRes.Drc_v12.gas_l1;
    isp_cfg.others.drc_cfg.gas_l2          = adrc_data.DrcProcRes.Drc_v12.gas_l2;
    isp_cfg.others.drc_cfg.gas_l3          = adrc_data.DrcProcRes.Drc_v12.gas_l3;

#if 0
    LOGE_CAMHW_SUBM(ISP20PARAM_SUBM, "%d: sw_drc_offset_pow2 %d", __LINE__, isp_cfg.others.drc_cfg.offset_pow2);
    LOGE_CAMHW_SUBM(ISP20PARAM_SUBM, "sw_drc_offset_pow2 %d", isp_cfg.others.drc_cfg.offset_pow2);
    LOGE_CAMHW_SUBM(ISP20PARAM_SUBM, "sw_drc_compres_scl %d", isp_cfg.others.drc_cfg.compres_scl);
    LOGE_CAMHW_SUBM(ISP20PARAM_SUBM, "sw_drc_position %d", isp_cfg.others.drc_cfg.position);
    LOGE_CAMHW_SUBM(ISP20PARAM_SUBM, "sw_drc_delta_scalein %d", isp_cfg.others.drc_cfg.delta_scalein);
    LOGE_CAMHW_SUBM(ISP20PARAM_SUBM, "sw_drc_hpdetail_ratio %d", isp_cfg.others.drc_cfg.hpdetail_ratio);
    LOGE_CAMHW_SUBM(ISP20PARAM_SUBM, "sw_drc_lpdetail_ratio %d", isp_cfg.others.drc_cfg.lpdetail_ratio);
    LOGE_CAMHW_SUBM(ISP20PARAM_SUBM, "sw_drc_weicur_pix %d", isp_cfg.others.drc_cfg.weicur_pix);
#endif
}
#endif
#endif

#if RKAIQ_HAVE_CCM_V2
void Isp32Params::convertAiqCcmToIsp32Params(struct isp32_isp_params_cfg& isp_cfg,
        const rk_aiq_ccm_cfg_v2_t& ccm) {
    if (ccm.ccmEnable) {
        isp_cfg.module_ens |= ISP2X_MODULE_CCM;
    }
    isp_cfg.module_en_update |= ISP2X_MODULE_CCM;
    isp_cfg.module_cfg_update |= ISP2X_MODULE_CCM;

    struct isp32_ccm_cfg* cfg = &isp_cfg.others.ccm_cfg;
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
}
#endif

#if (RKAIQ_HAVE_SHARP_V33 || RKAIQ_HAVE_SHARP_V33_LITE)
#if USE_NEWSTRUCT
void Isp32Params::convertAiqSharpToIsp32Params(struct isp32_isp_params_cfg& isp_cfg,
        rk_aiq_isp_sharp_params_t *sharp_attr) {

    if (sharp_attr->en) {
        isp_cfg.module_ens |= ISP3X_MODULE_SHARP;
        isp_cfg.module_en_update |= ISP3X_MODULE_SHARP;
        isp_cfg.module_cfg_update |= ISP3X_MODULE_SHARP;
    } else {
        isp_cfg.module_ens &=~ ISP3X_MODULE_SHARP;
        return;
    }

    struct isp32_sharp_cfg* pSharp = &isp_cfg.others.sharp_cfg;
    pSharp->bypass = sharp_attr->bypass;
    rk_aiq_sharp32_params_cvt(&sharp_attr->result, &isp_cfg, &mCommonCvtInfo);
}
#else
void Isp32Params::convertAiqSharpenToIsp32Params(struct isp32_isp_params_cfg& isp_cfg,
        rk_aiq_isp_sharp_v32_t& sharp) {
    LOGD_ASHARP("%s:%d enter! enable:%d\n", __FUNCTION__, __LINE__, sharp.sharp_en);
    bool enable = sharp.sharp_en;

    isp_cfg.module_en_update |= ISP3X_MODULE_SHARP;
    isp_cfg.module_ens &=~ ISP3X_MODULE_SHARP;

#if 0

    isp_cfg.module_cfg_update |= ISP3X_MODULE_SHARP;
    struct isp32_sharp_cfg* pSharp = &isp_cfg.others.sharp_cfg;

    pSharp->radius_ds_mode = sharp.sharp_radius_ds_mode;
    pSharp->noiseclip_mode = sharp.sharp_noiseclip_mode;
    pSharp->exgain_bypass  = sharp.sharp_exgain_bypass;
    pSharp->center_mode    = sharp.sharp_center_mode;
    pSharp->bypass         = sharp.sharp_bypass;
    if (!enable) {
        pSharp->bypass = 0x01;
    }

    pSharp->clip_hf_mode = sharp.sharp_clip_hf_mode;
    pSharp->add_mode     = sharp.sharp_add_mode;

    pSharp->sharp_ratio = sharp.sharp_sharp_ratio;
    pSharp->bf_ratio    = sharp.sharp_bf_ratio;
    pSharp->gaus_ratio  = sharp.sharp_gaus_ratio;
    pSharp->pbf_ratio   = sharp.sharp_pbf_ratio;

    for (int i = 0; i < ISP3X_SHARP_X_NUM; i++) {
        pSharp->luma_dx[i] = sharp.sharp_luma_dx[i];
        LOGD_ASHARP("%s:%d  luma_dx[%d]:0x%x \n", __FUNCTION__, __LINE__, i, pSharp->luma_dx[i]);
    }

    for (int i = 0; i < ISP3X_SHARP_Y_NUM; i++) {
        pSharp->pbf_sigma_inv[i] = sharp.sharp_pbf_sigma_inv[i];
        pSharp->bf_sigma_inv[i]  = sharp.sharp_bf_sigma_inv[i];
        LOGD_ASHARP("%s:%d  pbf_sigma_inv[%d]:0x%x \n", __FUNCTION__, __LINE__, i,
                    pSharp->pbf_sigma_inv[i]);
        LOGD_ASHARP("%s:%d  bf_sigma_inv[%d]:0x%x \n", __FUNCTION__, __LINE__, i,
                    pSharp->bf_sigma_inv[i]);
    }

    pSharp->bf_sigma_shift  = sharp.sharp_bf_sigma_shift;
    pSharp->pbf_sigma_shift = sharp.sharp_pbf_sigma_shift;
    LOGD_ASHARP("%s:%d  bf_sigma_shift:0x%x \n", __FUNCTION__, __LINE__, pSharp->bf_sigma_shift);
    LOGD_ASHARP("%s:%d  pbf_sigma_shift:0x%x \n", __FUNCTION__, __LINE__, pSharp->pbf_sigma_shift);
    for (int i = 0; i < ISP3X_SHARP_Y_NUM; i++) {
        pSharp->clip_hf[i] = sharp.sharp_clip_hf[i];
        LOGD_ASHARP("%s:%d  clip_hf[%d]:0x%x \n", __FUNCTION__, __LINE__, i, pSharp->clip_hf[i]);
    }

    pSharp->pbf_coef2 = sharp.sharp_pbf_coef[2];
    pSharp->pbf_coef1 = sharp.sharp_pbf_coef[1];
    pSharp->pbf_coef0 = sharp.sharp_pbf_coef[0];
    LOGD_ASHARP("%s:%d  pbf_coef2:0x%x \n", __FUNCTION__, __LINE__, pSharp->pbf_coef2);
    LOGD_ASHARP("%s:%d  pbf_coef1:0x%x \n", __FUNCTION__, __LINE__, pSharp->pbf_coef1);
    LOGD_ASHARP("%s:%d  pbf_coef0:0x%x \n", __FUNCTION__, __LINE__, pSharp->pbf_coef0);

    pSharp->bf_coef2 = sharp.sharp_bf_coef[2];
    pSharp->bf_coef1 = sharp.sharp_bf_coef[1];
    pSharp->bf_coef0 = sharp.sharp_bf_coef[0];
    LOGD_ASHARP("%s:%d bf_coef2:0x%x \n", __FUNCTION__, __LINE__, pSharp->bf_coef2);
    LOGD_ASHARP("%s:%d bf_coef1:0x%x \n", __FUNCTION__, __LINE__, pSharp->bf_coef1);
    LOGD_ASHARP("%s:%d bf_coef0:0x%x \n", __FUNCTION__, __LINE__, pSharp->bf_coef0);
    for (int i = 0; i < ISP3X_SHARP_GAUS_COEF_NUM; i++) {
        pSharp->gaus_coef[i] = sharp.sharp_gaus_coef[i];
        LOGD_ASHARP("%s:%d  gaus_coef[%d]:0x%x \n", __FUNCTION__, __LINE__, i,
                    pSharp->gaus_coef[i]);
    }

    pSharp->global_gain       = sharp.sharp_global_gain;
    pSharp->global_gain_alpha = sharp.sharp_global_gain_alpha;
    pSharp->local_gainscale   = sharp.sharp_local_gainscale;
    LOGD_ASHARP("%s:%d  gloable_gain:0x%x \n", __FUNCTION__, __LINE__, pSharp->global_gain);
    LOGD_ASHARP("%s:%d  gloable_gain_alpha:0x%x \n", __FUNCTION__, __LINE__,
                pSharp->global_gain_alpha);
    LOGD_ASHARP("%s:%d  local_gainscale:0x%x \n", __FUNCTION__, __LINE__, pSharp->local_gainscale);

    for (int i = 0; i < 14; i++) {
        pSharp->gain_adj[i] = sharp.sharp_gain_adj[i];
        LOGD_ASHARP("%s:%d  sharp_gain_adj:0x%x \n", __FUNCTION__, __LINE__, pSharp->gain_adj[i]);
    }
    pSharp->center_wid = sharp.sharp_center_wid;
    pSharp->center_het = sharp.sharp_center_het;
    LOGD_ASHARP("%s:%d  center_wid:0x%x \n", __FUNCTION__, __LINE__, pSharp->center_wid);
    LOGD_ASHARP("%s:%d  center_het:0x%x \n", __FUNCTION__, __LINE__, pSharp->center_het);

    for (int i = 0; i < 22; i++) {
        pSharp->strength[i] = sharp.sharp_strength[i];
        LOGD_ASHARP("%s:%d  strength[%d]:0x%x \n", __FUNCTION__, __LINE__, i, pSharp->strength[i]);
    }
    pSharp->noise_sigma    = sharp.sharp_noise_sigma;
    pSharp->enhance_bit    = sharp.sharp_enhance_bit;
    pSharp->noise_strength = sharp.sharp_noise_strength;
    LOGD_ASHARP("%s:%d  noise_sigma:0x%x \n", __FUNCTION__, __LINE__, pSharp->noise_sigma);
    LOGD_ASHARP("%s:%d  enhance_bit:0x%x \n", __FUNCTION__, __LINE__, pSharp->enhance_bit);
    LOGD_ASHARP("%s:%d  noise_strength:0x%x \n", __FUNCTION__, __LINE__, pSharp->noise_strength);

    for (int i = 0; i < 8; i++) {
        pSharp->ehf_th[i]   = sharp.sharp_ehf_th[i];
        pSharp->clip_neg[i] = sharp.sharp_clip_neg[i];
        LOGD_ASHARP("%s:%d  ehf_th[%d]:0x%x  clip_neg[%d]:0x%x\n", __FUNCTION__, __LINE__, i,
                    pSharp->ehf_th[i], i, pSharp->clip_neg[i]);
    }
#endif
    LOGD_ASHARP("%s:%d exit!\n", __FUNCTION__, __LINE__);
}
#endif
#endif
#if RKAIQ_HAVE_BLC_V32
void Isp32Params::convertAiqBlcToIsp32Params(struct isp32_isp_params_cfg& isp_cfg,
        rk_aiq_isp_blc_v32_t& blc) {
    LOGD_ABLC("%s:(%d) enter enable:%d\n", __FUNCTION__, __LINE__, blc.enable);

    if (blc.enable) {
        isp_cfg.module_ens |= ISP3X_MODULE_BLS;
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

#if defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
    mLatestBlsCfg = isp_cfg.others.bls_cfg;
#endif

    LOGD_ABLC("isp_ob_offset = 0x%x ,isp_ob_predgain = 0x%x, isp_ob_max = %x \n",
              isp_cfg.others.bls_cfg.isp_ob_offset, isp_cfg.others.bls_cfg.isp_ob_predgain,
              isp_cfg.others.bls_cfg.isp_ob_max);

#endif
    LOGD_ABLC("%s:(%d) exit \n", __FUNCTION__, __LINE__);
}
#endif

#if RKAIQ_HAVE_LDCH_V21
void Isp32Params::convertAiqAldchToIsp32Params(struct isp32_isp_params_cfg& isp_cfg,
        struct isp32_isp_params_cfg& isp_cfg_right,
        const rk_aiq_isp_ldch_v21_t& ldch_cfg,
        bool is_multi_isp)
{
    struct isp32_ldch_cfg *pLdchCfg = &isp_cfg.others.ldch_cfg;

    if (ldch_cfg.base.sw_ldch_en) {
        isp_cfg.module_ens |= ISP32_MODULE_LDCH;
        isp_cfg.module_en_update |= ISP32_MODULE_LDCH;
        isp_cfg.module_cfg_update |= ISP32_MODULE_LDCH;

        pLdchCfg->hsize = ldch_cfg.base.lut_h_size;
        pLdchCfg->vsize = ldch_cfg.base.lut_v_size;
        pLdchCfg->buf_fd = ldch_cfg.base.lut_mapxy_buf_fd[0];

        pLdchCfg->frm_end_dis = ldch_cfg.frm_end_dis;
        pLdchCfg->zero_interp_en = ldch_cfg.zero_interp_en;
        pLdchCfg->sample_avr_en = ldch_cfg.sample_avr_en;
        pLdchCfg->bic_mode_en = ldch_cfg.bic_mode_en;
        pLdchCfg->force_map_en = ldch_cfg.force_map_en;
        pLdchCfg->map13p3_en = ldch_cfg.map13p3_en;
        memcpy(pLdchCfg->bicubic, ldch_cfg.bicubic, sizeof(ldch_cfg.bicubic));

        LOGD_CAMHW_SUBM(ISP20PARAM_SUBM, "enable ldch h/v size: %dx%d, buf_fd: %d",
                        pLdchCfg->hsize, pLdchCfg->vsize, pLdchCfg->buf_fd);

        if (is_multi_isp) {
            struct isp32_ldch_cfg *cfg_right = &isp_cfg.others.ldch_cfg;
            cfg_right = &isp_cfg_right.others.ldch_cfg;
            memcpy(cfg_right, pLdchCfg, sizeof(*cfg_right));
            cfg_right->buf_fd = ldch_cfg.base.lut_mapxy_buf_fd[1];

            LOGD_CAMHW_SUBM(ISP20PARAM_SUBM, "multi isp: ldch right isp buf fd: %d", cfg_right->buf_fd);
        }
    } else {
        isp_cfg.module_ens &= ~ISP32_MODULE_LDCH;
        isp_cfg.module_en_update |= ISP32_MODULE_LDCH;
    }
}
#endif

void Isp32Params::convertAiqExpIspDgainToIsp32Params(struct isp32_isp_params_cfg& isp_cfg, RKAiqAecExpInfo_t ae_exp)
{
#if defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
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
#endif
}

#if RKAIQ_HAVE_DPCC_V1
#ifdef USE_NEWSTRUCT
void Isp32Params::convertAiqDpccToIsp32Params(struct isp32_isp_params_cfg& isp_cfg,
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

    rk_aiq_dpcc20_params_cvt(&dpcc_attr->result, &isp_cfg);
    // hwi_api_dpcc20_params_cvt(&dpcc_attr->result, &isp_cfg);
    // hwi_api_dpcc20_params_dump(&dpcc_attr->result, &isp_cfg.others.dpcc_cfg);

}
#endif
#endif

#if RKAIQ_HAVE_GAMMA_V11
#if USE_NEWSTRUCT
void Isp32Params::convertAiqGammaToIsp32Params(struct isp32_isp_params_cfg& isp_cfg,
        rk_aiq_isp_gamma_params_t* gamma_attr) {

    if (gamma_attr->en) {
        isp_cfg.module_ens |= ISP2X_MODULE_GOC;
        isp_cfg.module_en_update |= ISP2X_MODULE_GOC;
        isp_cfg.module_cfg_update |= ISP2X_MODULE_GOC;
    } else {
        isp_cfg.module_ens &= ~ISP2X_MODULE_GOC;
        isp_cfg.module_en_update |= ISP2X_MODULE_GOC;
        return;
    }

    rk_aiq_gamma21_params_cvt(&gamma_attr->result, &isp_cfg);
}
#endif
#endif

bool Isp32Params::convert3aResultsToIspCfg(SmartPtr<cam3aResult>& result, void* isp_cfg_p, bool is_multi_isp) {
    struct isp32_isp_params_cfg& isp_cfg       = *(struct isp32_isp_params_cfg*)isp_cfg_p;
    struct isp32_isp_params_cfg& isp_cfg_right = *((struct isp32_isp_params_cfg*)isp_cfg_p + 1);
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
            convertAiqExpIspDgainToIsp32Params(isp_cfg,
                                               expParams->data()->aecExpInfo);
    }
    break;
    case RESULT_TYPE_AWBGAIN_PARAM: {
#if RKAIQ_HAVE_AWB_V32 || RKAIQ_HAVE_AWB_V32LT
        RkAiqIspAwbGainParamsProxy* awb_gain =
            result.get_cast_ptr<RkAiqIspAwbGainParamsProxy>();
        if (awb_gain) {
            convertAiqAwbGainToIsp32Params(isp_cfg, awb_gain->data()->result, true);
        }
#endif
    }
    break;
    case RESULT_TYPE_AWB_PARAM: {
#if RKAIQ_HAVE_AWB_V32
        mAwbParams = result.ptr();
        RkAiqIspAwbParamsProxy* params =
            result.get_cast_ptr<RkAiqIspAwbParamsProxy>();

        if (params) convertAiqAwbToIsp32Params(isp_cfg, params->data()->result, true);
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
#if RKAIQ_HAVE_AF_V31 || RKAIQ_ONLY_AF_STATS_V31
        RkAiqIspAfParamsProxy* params =
            result.get_cast_ptr<RkAiqIspAfParamsProxy>();
        if (params) convertAiqAfToIsp32Params(isp_cfg, params->data()->result, true);
#endif
#if RKAIQ_HAVE_AF_V32_LITE || RKAIQ_ONLY_AF_STATS_V32_LITE
        RkAiqIspAfParamsProxy* params =
            result.get_cast_ptr<RkAiqIspAfParamsProxy>();
        if (params) convertAiqAfLiteToIsp32Params(isp_cfg, params->data()->result, true);
#endif
    }
    break;
    case RESULT_TYPE_CCM_PARAM: {
#if RKAIQ_HAVE_CCM_V2
        RkAiqIspCcmParamsProxy* params =
            result.get_cast_ptr<RkAiqIspCcmParamsProxy>();
        if (params) convertAiqCcmToIsp32Params(isp_cfg, params->data()->result);
#endif
    }
    break;
    case RESULT_TYPE_CAC_PARAM: {
#if RKAIQ_HAVE_CAC_V11
        RkAiqIspCacParamsProxy* params =
            result.get_cast_ptr<RkAiqIspCacParamsProxy>();
        if (params) {
            convertAiqCacToIsp32Params(isp_cfg, isp_cfg_right, params->data()->result, is_multi_isp);
        }
#endif
    }
    break;
    case RESULT_TYPE_DEBAYER_PARAM: {
#if RKAIQ_HAVE_DEBAYER_V2 || RKAIQ_HAVE_DEBAYER_V2_LITE
#if USE_NEWSTRUCT
        RkAiqIspDmParamsProxy* params =
            result.get_cast_ptr<RkAiqIspDmParamsProxy>();
        if (params) {
            convertAiqDmToIsp32Params(isp_cfg, params->data().ptr());
        }
#else
        RkAiqIspDebayerParamsProxy* params =
            result.get_cast_ptr<RkAiqIspDebayerParamsProxy>();
        if (params) {
            convertAiqAdebayerToIsp32Params(isp_cfg, params->data()->result);
        }
#endif
#endif
    }
    break;
    case RESULT_TYPE_AEC_PARAM: {
#if defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
        RkAiqIspAecParamsProxy* params =
            result.get_cast_ptr<RkAiqIspAecParamsProxy>();
        if (params) {
            convertAiqAeToIsp20Params(isp_cfg, params->data()->result);
        }
#endif
    }
    break;
    case RESULT_TYPE_HIST_PARAM: {
#if defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
        RkAiqIspHistParamsProxy* params =
            result.get_cast_ptr<RkAiqIspHistParamsProxy>();
        if (params) convertAiqHistToIsp20Params(isp_cfg, params->data()->result);
#endif
    }
    break;
    case RESULT_TYPE_AGAMMA_PARAM: {
#if RKAIQ_HAVE_GAMMA_V11
#if USE_NEWSTRUCT
        RkAiqIspGammaParamsProxy* params =
            result.get_cast_ptr<RkAiqIspGammaParamsProxy>();
        if (params) convertAiqGammaToIsp32Params(isp_cfg, params->data().ptr());
#else
        RkAiqIspAgammaParamsProxy* params =
            result.get_cast_ptr<RkAiqIspAgammaParamsProxy>();
        if (params) convertAiqAgammaToIsp3xParams(isp_cfg, params->data()->result);
#endif
#endif
    }
    break;
    case RESULT_TYPE_ADEGAMMA_PARAM: {
#if RKAIQ_HAVE_DEGAMMA_V1
        RkAiqIspAdegammaParamsProxy* params =
            result.get_cast_ptr<RkAiqIspAdegammaParamsProxy>();
        if (params) convertAiqAdegammaToIsp20Params(isp_cfg, params->data()->result);
#endif
    }
    break;
    case RESULT_TYPE_MERGE_PARAM: {
#if RKAIQ_HAVE_MERGE_V12
        RkAiqIspMergeParamsProxy* params =
            result.get_cast_ptr<RkAiqIspMergeParamsProxy>();
        if (params) convertAiqMergeToIsp32Params(isp_cfg, params->data()->result);
#endif
    }
    break;
    case RESULT_TYPE_DEHAZE_PARAM: {
#if RKAIQ_HAVE_DEHAZE_V12
#ifdef USE_NEWSTRUCT
        RkAiqIspDehazeParamsProxy* params =
            result.get_cast_ptr<RkAiqIspDehazeParamsProxy>();
        if (params) convertAiqDehazeToIsp32Params(isp_cfg, params->data().ptr());
#else
        RkAiqIspDehazeParamsProxy* params =
            result.get_cast_ptr<RkAiqIspDehazeParamsProxy>();
        if (params) convertAiqAdehazeToIsp32Params(isp_cfg, params->data()->result);
#endif
#endif
    }
    break;
    case RESULT_TYPE_DRC_PARAM: {
#if RKAIQ_HAVE_DRC_V12 || RKAIQ_HAVE_DRC_V12_LITE
#ifdef USE_NEWSTRUCT
        RkAiqIspDrcParamsProxy* params =
            result.get_cast_ptr<RkAiqIspDrcParamsProxy>();
        if (params) convertAiqDrcToIsp32Params(isp_cfg, params->data().ptr());
#else
        RkAiqIspDrcParamsProxy* params = result.get_cast_ptr<RkAiqIspDrcParamsProxy>();
        if (params) convertAiqDrcToIsp32Params(isp_cfg, params->data()->result);
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
    case RESULT_TYPE_DPCC_PARAM: {
#if RKAIQ_HAVE_DPCC_V1
#ifdef USE_NEWSTRUCT
        RkAiqIspDpccParamsProxy* params =
            result.get_cast_ptr<RkAiqIspDpccParamsProxy>();
        if (params) {
            convertAiqDpccToIsp32Params(isp_cfg, params->data().ptr());
        }
#else
        RkAiqIspDpccParamsProxy* params =
            result.get_cast_ptr<RkAiqIspDpccParamsProxy>();
        if (params) convertAiqDpccToIsp20Params(isp_cfg, params->data()->result);
#endif
#endif
    }
    break;

    case RESULT_TYPE_CSM_PARAM: {
        RkAiqIspCsmParamsProxy* params =
            result.get_cast_ptr<RkAiqIspCsmParamsProxy>();
        if (params) convertAiqCsmToIsp21Params(isp_cfg, params->data()->result);
    }
    break;
    case RESULT_TYPE_RAWNR_PARAM: {
#if RKAIQ_HAVE_BAYER2DNR_V23
        RkAiqIspBaynrParamsProxy* params =
            result.get_cast_ptr<RkAiqIspBaynrParamsProxy>();
        if (params) convertAiqRawnrToIsp32Params(isp_cfg, params->data()->result);
#endif
    }
    break;
    case RESULT_TYPE_TNR_PARAM: {
#if (RKAIQ_HAVE_BAYERTNR_V23 || RKAIQ_HAVE_BAYERTNR_V23_LITE)
#if USE_NEWSTRUCT
        RkAiqIspBtnrParamsProxy* params =
            result.get_cast_ptr<RkAiqIspBtnrParamsProxy>();
        if (params) {
            convertAiqBtnrToIsp32Params(isp_cfg, params->data().ptr());
        }
#else
        RkAiqIspTnrParamsProxy* params =
            result.get_cast_ptr<RkAiqIspTnrParamsProxy>();
        if (params) convertAiqTnrToIsp32Params(isp_cfg, params->data()->result);
#endif
#endif
    }
    break;
    case RESULT_TYPE_YNR_PARAM: {
#if RKAIQ_HAVE_YNR_V22
#ifdef USE_NEWSTRUCT
        RkAiqIspYnrParamsProxy* params =
            result.get_cast_ptr<RkAiqIspYnrParamsProxy>();
        if (params) {
            convertAiqYnrToIsp32Params(isp_cfg, params->data().ptr());
        }
#else
        RkAiqIspYnrParamsProxy* params =
            result.get_cast_ptr<RkAiqIspYnrParamsProxy>();
        if (params) convertAiqYnrToIsp32Params(isp_cfg, params->data()->result);
#endif
#endif
    }
    break;
    case RESULT_TYPE_UVNR_PARAM: {
#if (RKAIQ_HAVE_CNR_V30 || RKAIQ_HAVE_CNR_V30_LITE)
#ifdef USE_NEWSTRUCT
        RkAiqIspCnrParamsProxy* params =
            result.get_cast_ptr<RkAiqIspCnrParamsProxy>();
        if (params) convertAiqCnrToIsp32Params(isp_cfg, params->data().ptr());
#else
        RkAiqIspCnrParamsProxy* params =
            result.get_cast_ptr<RkAiqIspCnrParamsProxy>();
        if (params) convertAiqUvnrToIsp32Params(isp_cfg, params->data()->result);
#endif
#endif
    }
    break;
    case RESULT_TYPE_BLC_PARAM:
    {
#if RKAIQ_HAVE_BLC_V32 && !USE_NEWSTRUCT
        RkAiqIspBlcParamsProxy* params =
            result.get_cast_ptr<RkAiqIspBlcParamsProxy>();
        if (params) convertAiqBlcToIsp32Params(isp_cfg, params->data()->result);
#endif
    }
    break;
    case RESULT_TYPE_GAIN_PARAM: {
#if RKAIQ_HAVE_GAIN_V2
        RkAiqIspGainParamsProxy* params =
            result.get_cast_ptr<RkAiqIspGainParamsProxy>();
        if (params) convertAiqGainToIsp3xParams(isp_cfg, params->data()->result);
#endif
    }
    break;
    case RESULT_TYPE_SHARPEN_PARAM: {
#if (RKAIQ_HAVE_SHARP_V33 || RKAIQ_HAVE_SHARP_V33_LITE)
#ifdef USE_NEWSTRUCT
        RkAiqIspSharpParamsProxy* params =
            result.get_cast_ptr<RkAiqIspSharpParamsProxy>();
        if (params) convertAiqSharpToIsp32Params(isp_cfg, params->data().ptr());
#else
        RkAiqIspSharpenParamsProxy* params =
            result.get_cast_ptr<RkAiqIspSharpenParamsProxy>();
        if (params) convertAiqSharpenToIsp32Params(isp_cfg, params->data()->result);
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
    case RESULT_TYPE_IE_PARAM: {
#if RKAIQ_HAVE_AIE_V10
        RkAiqIspIeParamsProxy* params =
            result.get_cast_ptr<RkAiqIspIeParamsProxy>();
        if (params) convertAiqIeToIsp20Params(isp_cfg, params->data()->result);
#endif
    }
    break;
    case RESULT_TYPE_LDCH_PARAM:
    {
#if RKAIQ_HAVE_LDCH_V21
        RkAiqIspLdchParamsProxy* params = result.get_cast_ptr<RkAiqIspLdchParamsProxy>();
        if (params)
            convertAiqAldchToIsp32Params(isp_cfg, isp_cfg_right, params->data()->result, is_multi_isp);
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
