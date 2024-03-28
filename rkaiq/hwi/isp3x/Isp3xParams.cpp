/*
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

#include "Isp3xParams.h"

namespace RkCam {

void Isp3xParams::fixedAwbOveflowToIsp3xParams(void* isp_cfg_p, bool is_dual_isp)
{
    //in overfloce case :
    // 1 sw_rawawb_wp_luma_wei_en is force to be true  and the weight is 31
    // 2 sw_rawawb_blk_with_luma_wei_en is force to be true

    struct isp3x_isp_params_cfg& isp_cfg = *(struct isp3x_isp_params_cfg*)isp_cfg_p;
    if(is_dual_isp == false) {
        struct isp3x_rawawb_meas_cfg * awb_cfg_v30 = &isp_cfg.meas.rawawb;
        int w, h;
        w = awb_cfg_v30->sw_rawawb_h_size;
        h = awb_cfg_v30->sw_rawawb_v_size;
        if(w * h > RK_AIQ_AWB_STAT_MAX_AREA) {
            LOGD_AWB("%s wp_luma_wei is force to be enable and the weight is %d", __FUNCTION__, (1 << RK_AIQ_AWB_WP_WEIGHT_BIS_V201) - 1);
            awb_cfg_v30->sw_rawawb_wp_luma_wei_en0 = true;
            awb_cfg_v30->sw_rawawb_wp_luma_wei_en1 = true;
            awb_cfg_v30->sw_rawawb_wp_luma_weicurve_w0 = (1 << RK_AIQ_AWB_WP_WEIGHT_BIS_V201) - 1;
            awb_cfg_v30->sw_rawawb_wp_luma_weicurve_w1 = (1 << RK_AIQ_AWB_WP_WEIGHT_BIS_V201) - 1;
            awb_cfg_v30->sw_rawawb_wp_luma_weicurve_w2 = (1 << RK_AIQ_AWB_WP_WEIGHT_BIS_V201) - 1;
            awb_cfg_v30->sw_rawawb_wp_luma_weicurve_w3 = (1 << RK_AIQ_AWB_WP_WEIGHT_BIS_V201) - 1;
            awb_cfg_v30->sw_rawawb_wp_luma_weicurve_w4 = (1 << RK_AIQ_AWB_WP_WEIGHT_BIS_V201) - 1;
            awb_cfg_v30->sw_rawawb_wp_luma_weicurve_w5 = (1 << RK_AIQ_AWB_WP_WEIGHT_BIS_V201) - 1;
            awb_cfg_v30->sw_rawawb_wp_luma_weicurve_w6 = (1 << RK_AIQ_AWB_WP_WEIGHT_BIS_V201) - 1;
            awb_cfg_v30->sw_rawawb_wp_luma_weicurve_w7 = (1 << RK_AIQ_AWB_WP_WEIGHT_BIS_V201) - 1;
            awb_cfg_v30->sw_rawawb_wp_luma_weicurve_w8 = (1 << RK_AIQ_AWB_WP_WEIGHT_BIS_V201) - 1;
            awb_cfg_v30->sw_rawawb_blk_with_luma_wei_en = true;
        }
    } else {
        struct isp3x_isp_params_cfg* left_isp_params  = &isp_cfg;
        struct isp3x_isp_params_cfg* right_isp_params = &isp_cfg + 1;
        struct isp3x_rawawb_meas_cfg * left_awb_cfg_v30 = &left_isp_params->meas.rawawb;
        struct isp3x_rawawb_meas_cfg * right_awb_cfg_v30 = &right_isp_params->meas.rawawb;
        int w, h, w2, h2;
        w = left_awb_cfg_v30->sw_rawawb_h_size;
        h = left_awb_cfg_v30->sw_rawawb_v_size;
        w2 = right_awb_cfg_v30->sw_rawawb_h_size;
        h2 = right_awb_cfg_v30->sw_rawawb_v_size;
        if(w * h > RK_AIQ_AWB_STAT_MAX_AREA || w2 * h2 > RK_AIQ_AWB_STAT_MAX_AREA ) {
            LOGD_AWB("wp_luma_wei is for to be enable and the weight is %d", (1 << RK_AIQ_AWB_WP_WEIGHT_BIS_V201) - 1);
            left_awb_cfg_v30->sw_rawawb_wp_luma_wei_en0 = true;
            left_awb_cfg_v30->sw_rawawb_wp_luma_wei_en1 = true;
            left_awb_cfg_v30->sw_rawawb_wp_luma_weicurve_w0 = (1 << RK_AIQ_AWB_WP_WEIGHT_BIS_V201) - 1;
            left_awb_cfg_v30->sw_rawawb_wp_luma_weicurve_w1 = (1 << RK_AIQ_AWB_WP_WEIGHT_BIS_V201) - 1;
            left_awb_cfg_v30->sw_rawawb_wp_luma_weicurve_w2 = (1 << RK_AIQ_AWB_WP_WEIGHT_BIS_V201) - 1;
            left_awb_cfg_v30->sw_rawawb_wp_luma_weicurve_w3 = (1 << RK_AIQ_AWB_WP_WEIGHT_BIS_V201) - 1;
            left_awb_cfg_v30->sw_rawawb_wp_luma_weicurve_w4 = (1 << RK_AIQ_AWB_WP_WEIGHT_BIS_V201) - 1;
            left_awb_cfg_v30->sw_rawawb_wp_luma_weicurve_w5 = (1 << RK_AIQ_AWB_WP_WEIGHT_BIS_V201) - 1;
            left_awb_cfg_v30->sw_rawawb_wp_luma_weicurve_w6 = (1 << RK_AIQ_AWB_WP_WEIGHT_BIS_V201) - 1;
            left_awb_cfg_v30->sw_rawawb_wp_luma_weicurve_w7 = (1 << RK_AIQ_AWB_WP_WEIGHT_BIS_V201) - 1;
            left_awb_cfg_v30->sw_rawawb_wp_luma_weicurve_w8 = (1 << RK_AIQ_AWB_WP_WEIGHT_BIS_V201) - 1;
            right_awb_cfg_v30->sw_rawawb_wp_luma_wei_en0 = true;
            right_awb_cfg_v30->sw_rawawb_wp_luma_wei_en1 = true;
            right_awb_cfg_v30->sw_rawawb_wp_luma_weicurve_w0 = (1 << RK_AIQ_AWB_WP_WEIGHT_BIS_V201) - 1;
            right_awb_cfg_v30->sw_rawawb_wp_luma_weicurve_w1 = (1 << RK_AIQ_AWB_WP_WEIGHT_BIS_V201) - 1;
            right_awb_cfg_v30->sw_rawawb_wp_luma_weicurve_w2 = (1 << RK_AIQ_AWB_WP_WEIGHT_BIS_V201) - 1;
            right_awb_cfg_v30->sw_rawawb_wp_luma_weicurve_w3 = (1 << RK_AIQ_AWB_WP_WEIGHT_BIS_V201) - 1;
            right_awb_cfg_v30->sw_rawawb_wp_luma_weicurve_w4 = (1 << RK_AIQ_AWB_WP_WEIGHT_BIS_V201) - 1;
            right_awb_cfg_v30->sw_rawawb_wp_luma_weicurve_w5 = (1 << RK_AIQ_AWB_WP_WEIGHT_BIS_V201) - 1;
            right_awb_cfg_v30->sw_rawawb_wp_luma_weicurve_w6 = (1 << RK_AIQ_AWB_WP_WEIGHT_BIS_V201) - 1;
            right_awb_cfg_v30->sw_rawawb_wp_luma_weicurve_w7 = (1 << RK_AIQ_AWB_WP_WEIGHT_BIS_V201) - 1;
            right_awb_cfg_v30->sw_rawawb_wp_luma_weicurve_w8 = (1 << RK_AIQ_AWB_WP_WEIGHT_BIS_V201) - 1;
            left_awb_cfg_v30->sw_rawawb_blk_with_luma_wei_en = true;
            right_awb_cfg_v30->sw_rawawb_blk_with_luma_wei_en = true;
        }
    }
}

#if RKAIQ_HAVE_AWB_V21
void Isp3xParams::convertAiqAwbToIsp3xParams(struct isp3x_isp_params_cfg& isp_cfg,
        const rk_aiq_isp_awb_meas_cfg_v3x_t& awb_meas,
        bool awb_cfg_udpate)
{
    if(awb_cfg_udpate) {
        if(awb_meas.awbEnable) {
            isp_cfg.module_ens |= ISP2X_MODULE_RAWAWB;
            isp_cfg.module_cfg_update |= ISP2X_MODULE_RAWAWB;
            isp_cfg.module_en_update |= ISP2X_MODULE_RAWAWB;
        }
    } else {
        return;
    }
    struct isp3x_rawawb_meas_cfg * awb_cfg_v30 = &isp_cfg.meas.rawawb;
    awb_cfg_v30->rawawb_sel                =    awb_meas.frameChoose;
    awb_cfg_v30->sw_rawawb_xy_en0          =  awb_meas.xyDetectionEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201];
    awb_cfg_v30->sw_rawawb_uv_en0          =  awb_meas.uvDetectionEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201];
    awb_cfg_v30->sw_rawawb_3dyuv_en0       =  awb_meas.threeDyuvEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201];
    awb_cfg_v30->sw_rawawb_xy_en1          =  awb_meas.xyDetectionEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201];
    awb_cfg_v30->sw_rawawb_uv_en1          =  awb_meas.uvDetectionEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201];
    awb_cfg_v30->sw_rawawb_3dyuv_en1          =  awb_meas.threeDyuvEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201];
    awb_cfg_v30->sw_rawawb_wp_blk_wei_en0    =  awb_meas.blkWeightEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201];
    awb_cfg_v30->sw_rawawb_wp_blk_wei_en1    =  awb_meas.blkWeightEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201];
    awb_cfg_v30->sw_rawlsc_bypass_en    =  awb_meas.lscBypEnable;
    awb_cfg_v30->sw_rawawb_blk_measure_enable    =  awb_meas.blkStatisticsEnable;
    awb_cfg_v30->sw_rawawb_blk_measure_mode     =  awb_meas.blkMeasureMode;
    awb_cfg_v30->sw_rawawb_blk_measure_xytype     =  awb_meas.xyRangeTypeForBlkStatistics;
    awb_cfg_v30->sw_rawawb_blk_measure_illu_idx     =  awb_meas.illIdxForBlkStatistics;
    awb_cfg_v30->sw_rawawb_blk_with_luma_wei_en  =  awb_meas.blkStatisticsWithLumaWeightEn;
    awb_cfg_v30->sw_rawawb_wp_luma_wei_en0   =  awb_meas.wpDiffWeiEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201];
    awb_cfg_v30->sw_rawawb_wp_luma_wei_en1   =  awb_meas.wpDiffWeiEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201];
    awb_cfg_v30->sw_rawawb_wp_hist_xytype    =  awb_meas.xyRangeTypeForWpHist;
    awb_cfg_v30->sw_rawawb_3dyuv_ls_idx0       =  awb_meas.threeDyuvIllu[0];
    awb_cfg_v30->sw_rawawb_3dyuv_ls_idx1       =  awb_meas.threeDyuvIllu[1];
    awb_cfg_v30->sw_rawawb_3dyuv_ls_idx2       =  awb_meas.threeDyuvIllu[2];
    awb_cfg_v30->sw_rawawb_3dyuv_ls_idx3       =  awb_meas.threeDyuvIllu[3];
    awb_cfg_v30->sw_rawawb_light_num      =  awb_meas.lightNum;
    awb_cfg_v30->sw_rawawb_h_offs         =  awb_meas.windowSet[0];
    awb_cfg_v30->sw_rawawb_v_offs         =  awb_meas.windowSet[1];
    awb_cfg_v30->sw_rawawb_h_size         =  awb_meas.windowSet[2];
    awb_cfg_v30->sw_rawawb_v_size         =  awb_meas.windowSet[3];
    awb_cfg_v30->sw_rawawb_wind_size = awb_meas.dsMode;
    awb_cfg_v30->sw_rawawb_r_max          =  awb_meas.maxR;
    awb_cfg_v30->sw_rawawb_g_max          =  awb_meas.maxG;
    awb_cfg_v30->sw_rawawb_b_max          =  awb_meas.maxB;
    awb_cfg_v30->sw_rawawb_y_max          =  awb_meas.maxY;
    awb_cfg_v30->sw_rawawb_r_min          =  awb_meas.minR;
    awb_cfg_v30->sw_rawawb_g_min          =  awb_meas.minG;
    awb_cfg_v30->sw_rawawb_b_min          =  awb_meas.minB;
    awb_cfg_v30->sw_rawawb_y_min          =  awb_meas.minY;
    awb_cfg_v30->sw_rawawb_vertex0_u_0    =  awb_meas.uvRange_param[0].pu_region[0];
    awb_cfg_v30->sw_rawawb_vertex0_v_0    =  awb_meas.uvRange_param[0].pv_region[0];
    awb_cfg_v30->sw_rawawb_vertex1_u_0    =  awb_meas.uvRange_param[0].pu_region[1];
    awb_cfg_v30->sw_rawawb_vertex1_v_0    =  awb_meas.uvRange_param[0].pv_region[1];
    awb_cfg_v30->sw_rawawb_vertex2_u_0    =  awb_meas.uvRange_param[0].pu_region[2];
    awb_cfg_v30->sw_rawawb_vertex2_v_0    =  awb_meas.uvRange_param[0].pv_region[2];
    awb_cfg_v30->sw_rawawb_vertex3_u_0    =  awb_meas.uvRange_param[0].pu_region[3];
    awb_cfg_v30->sw_rawawb_vertex3_v_0    =  awb_meas.uvRange_param[0].pv_region[3];
    awb_cfg_v30->sw_rawawb_islope01_0      =  awb_meas.uvRange_param[0].slope_inv[0];
    awb_cfg_v30->sw_rawawb_islope12_0      =  awb_meas.uvRange_param[0].slope_inv[1];
    awb_cfg_v30->sw_rawawb_islope23_0      =  awb_meas.uvRange_param[0].slope_inv[2];
    awb_cfg_v30->sw_rawawb_islope30_0      =  awb_meas.uvRange_param[0].slope_inv[3];
    awb_cfg_v30->sw_rawawb_vertex0_u_1    =  awb_meas.uvRange_param[1].pu_region[0];
    awb_cfg_v30->sw_rawawb_vertex0_v_1    =  awb_meas.uvRange_param[1].pv_region[0];
    awb_cfg_v30->sw_rawawb_vertex1_u_1    =  awb_meas.uvRange_param[1].pu_region[1];
    awb_cfg_v30->sw_rawawb_vertex1_v_1    =  awb_meas.uvRange_param[1].pv_region[1];
    awb_cfg_v30->sw_rawawb_vertex2_u_1    =  awb_meas.uvRange_param[1].pu_region[2];
    awb_cfg_v30->sw_rawawb_vertex2_v_1    =  awb_meas.uvRange_param[1].pv_region[2];
    awb_cfg_v30->sw_rawawb_vertex3_u_1    =  awb_meas.uvRange_param[1].pu_region[3];
    awb_cfg_v30->sw_rawawb_vertex3_v_1    =  awb_meas.uvRange_param[1].pv_region[3];
    awb_cfg_v30->sw_rawawb_islope01_1      =  awb_meas.uvRange_param[1].slope_inv[0];
    awb_cfg_v30->sw_rawawb_islope12_1      =  awb_meas.uvRange_param[1].slope_inv[1];
    awb_cfg_v30->sw_rawawb_islope23_1      =  awb_meas.uvRange_param[1].slope_inv[2];
    awb_cfg_v30->sw_rawawb_islope30_1      =  awb_meas.uvRange_param[1].slope_inv[3];
    awb_cfg_v30->sw_rawawb_vertex0_u_2    =  awb_meas.uvRange_param[2].pu_region[0];
    awb_cfg_v30->sw_rawawb_vertex0_v_2    =  awb_meas.uvRange_param[2].pv_region[0];
    awb_cfg_v30->sw_rawawb_vertex1_u_2    =  awb_meas.uvRange_param[2].pu_region[1];
    awb_cfg_v30->sw_rawawb_vertex1_v_2    =  awb_meas.uvRange_param[2].pv_region[1];
    awb_cfg_v30->sw_rawawb_vertex2_u_2    =  awb_meas.uvRange_param[2].pu_region[2];
    awb_cfg_v30->sw_rawawb_vertex2_v_2    =  awb_meas.uvRange_param[2].pv_region[2];
    awb_cfg_v30->sw_rawawb_vertex3_u_2    =  awb_meas.uvRange_param[2].pu_region[3];
    awb_cfg_v30->sw_rawawb_vertex3_v_2    =  awb_meas.uvRange_param[2].pv_region[3];
    awb_cfg_v30->sw_rawawb_islope01_2      =  awb_meas.uvRange_param[2].slope_inv[0];
    awb_cfg_v30->sw_rawawb_islope12_2      =  awb_meas.uvRange_param[2].slope_inv[1];
    awb_cfg_v30->sw_rawawb_islope23_2      =  awb_meas.uvRange_param[2].slope_inv[2];
    awb_cfg_v30->sw_rawawb_islope30_2      =  awb_meas.uvRange_param[2].slope_inv[3];
    awb_cfg_v30->sw_rawawb_vertex0_u_3    =  awb_meas.uvRange_param[3].pu_region[0];
    awb_cfg_v30->sw_rawawb_vertex0_v_3    =  awb_meas.uvRange_param[3].pv_region[0];
    awb_cfg_v30->sw_rawawb_vertex1_u_3    =  awb_meas.uvRange_param[3].pu_region[1];
    awb_cfg_v30->sw_rawawb_vertex1_v_3    =  awb_meas.uvRange_param[3].pv_region[1];
    awb_cfg_v30->sw_rawawb_vertex2_u_3    =  awb_meas.uvRange_param[3].pu_region[2];
    awb_cfg_v30->sw_rawawb_vertex2_v_3    =  awb_meas.uvRange_param[3].pv_region[2];
    awb_cfg_v30->sw_rawawb_vertex3_u_3    =  awb_meas.uvRange_param[3].pu_region[3];
    awb_cfg_v30->sw_rawawb_vertex3_v_3    =  awb_meas.uvRange_param[3].pv_region[3];
    awb_cfg_v30->sw_rawawb_islope01_3      =  awb_meas.uvRange_param[3].slope_inv[0];
    awb_cfg_v30->sw_rawawb_islope12_3      =  awb_meas.uvRange_param[3].slope_inv[1];
    awb_cfg_v30->sw_rawawb_islope23_3      =  awb_meas.uvRange_param[3].slope_inv[2];
    awb_cfg_v30->sw_rawawb_islope30_3      =  awb_meas.uvRange_param[3].slope_inv[3];
    awb_cfg_v30->sw_rawawb_vertex0_u_4    =  awb_meas.uvRange_param[4].pu_region[0];
    awb_cfg_v30->sw_rawawb_vertex0_v_4    =  awb_meas.uvRange_param[4].pv_region[0];
    awb_cfg_v30->sw_rawawb_vertex1_u_4    =  awb_meas.uvRange_param[4].pu_region[1];
    awb_cfg_v30->sw_rawawb_vertex1_v_4    =  awb_meas.uvRange_param[4].pv_region[1];
    awb_cfg_v30->sw_rawawb_vertex2_u_4    =  awb_meas.uvRange_param[4].pu_region[2];
    awb_cfg_v30->sw_rawawb_vertex2_v_4    =  awb_meas.uvRange_param[4].pv_region[2];
    awb_cfg_v30->sw_rawawb_vertex3_u_4    =  awb_meas.uvRange_param[4].pu_region[3];
    awb_cfg_v30->sw_rawawb_vertex3_v_4    =  awb_meas.uvRange_param[4].pv_region[3];
    awb_cfg_v30->sw_rawawb_islope01_4      =  awb_meas.uvRange_param[4].slope_inv[0];
    awb_cfg_v30->sw_rawawb_islope12_4      =  awb_meas.uvRange_param[4].slope_inv[1];
    awb_cfg_v30->sw_rawawb_islope23_4      =  awb_meas.uvRange_param[4].slope_inv[2];
    awb_cfg_v30->sw_rawawb_islope30_4      =  awb_meas.uvRange_param[4].slope_inv[3];
    awb_cfg_v30->sw_rawawb_vertex0_u_5    =  awb_meas.uvRange_param[5].pu_region[0];
    awb_cfg_v30->sw_rawawb_vertex0_v_5    =  awb_meas.uvRange_param[5].pv_region[0];
    awb_cfg_v30->sw_rawawb_vertex1_u_5    =  awb_meas.uvRange_param[5].pu_region[1];
    awb_cfg_v30->sw_rawawb_vertex1_v_5    =  awb_meas.uvRange_param[5].pv_region[1];
    awb_cfg_v30->sw_rawawb_vertex2_u_5    =  awb_meas.uvRange_param[5].pu_region[2];
    awb_cfg_v30->sw_rawawb_vertex2_v_5    =  awb_meas.uvRange_param[5].pv_region[2];
    awb_cfg_v30->sw_rawawb_vertex3_u_5    =  awb_meas.uvRange_param[5].pu_region[3];
    awb_cfg_v30->sw_rawawb_vertex3_v_5    =  awb_meas.uvRange_param[5].pv_region[3];
    awb_cfg_v30->sw_rawawb_islope01_5      =  awb_meas.uvRange_param[5].slope_inv[0];
    awb_cfg_v30->sw_rawawb_islope12_5      =  awb_meas.uvRange_param[5].slope_inv[1];
    awb_cfg_v30->sw_rawawb_islope23_5      =  awb_meas.uvRange_param[5].slope_inv[2];
    awb_cfg_v30->sw_rawawb_islope30_5      =  awb_meas.uvRange_param[5].slope_inv[3];
    awb_cfg_v30->sw_rawawb_vertex0_u_6    =  awb_meas.uvRange_param[6].pu_region[0];
    awb_cfg_v30->sw_rawawb_vertex0_v_6    =  awb_meas.uvRange_param[6].pv_region[0];
    awb_cfg_v30->sw_rawawb_vertex1_u_6    =  awb_meas.uvRange_param[6].pu_region[1];
    awb_cfg_v30->sw_rawawb_vertex1_v_6    =  awb_meas.uvRange_param[6].pv_region[1];
    awb_cfg_v30->sw_rawawb_vertex2_u_6    =  awb_meas.uvRange_param[6].pu_region[2];
    awb_cfg_v30->sw_rawawb_vertex2_v_6    =  awb_meas.uvRange_param[6].pv_region[2];
    awb_cfg_v30->sw_rawawb_vertex3_u_6    =  awb_meas.uvRange_param[6].pu_region[3];
    awb_cfg_v30->sw_rawawb_vertex3_v_6    =  awb_meas.uvRange_param[6].pv_region[3];
    awb_cfg_v30->sw_rawawb_islope01_6      =  awb_meas.uvRange_param[6].slope_inv[0];
    awb_cfg_v30->sw_rawawb_islope12_6      =  awb_meas.uvRange_param[6].slope_inv[1];
    awb_cfg_v30->sw_rawawb_islope23_6      =  awb_meas.uvRange_param[6].slope_inv[2];
    awb_cfg_v30->sw_rawawb_islope30_6      =  awb_meas.uvRange_param[6].slope_inv[3];
    awb_cfg_v30->sw_rawawb_rgb2ryuvmat0_u =  awb_meas.icrgb2RYuv_matrix[0];
    awb_cfg_v30->sw_rawawb_rgb2ryuvmat1_u =  awb_meas.icrgb2RYuv_matrix[1];
    awb_cfg_v30->sw_rawawb_rgb2ryuvmat2_u =  awb_meas.icrgb2RYuv_matrix[2];
    awb_cfg_v30->sw_rawawb_rgb2ryuvofs_u =  awb_meas.icrgb2RYuv_matrix[3];
    awb_cfg_v30->sw_rawawb_rgb2ryuvmat0_v =  awb_meas.icrgb2RYuv_matrix[4];
    awb_cfg_v30->sw_rawawb_rgb2ryuvmat1_v =  awb_meas.icrgb2RYuv_matrix[5];
    awb_cfg_v30->sw_rawawb_rgb2ryuvmat2_v =  awb_meas.icrgb2RYuv_matrix[6];
    awb_cfg_v30->sw_rawawb_rgb2ryuvofs_v =  awb_meas.icrgb2RYuv_matrix[7];
    awb_cfg_v30->sw_rawawb_rgb2ryuvmat0_y =  awb_meas.icrgb2RYuv_matrix[8];
    awb_cfg_v30->sw_rawawb_rgb2ryuvmat1_y =  awb_meas.icrgb2RYuv_matrix[9];
    awb_cfg_v30->sw_rawawb_rgb2ryuvmat2_y =  awb_meas.icrgb2RYuv_matrix[10];
    awb_cfg_v30->sw_rawawb_rgb2ryuvofs_y =  awb_meas.icrgb2RYuv_matrix[11];
    awb_cfg_v30->sw_rawawb_rotu0_ls0 =  awb_meas.ic3Dyuv2Range_param[0].thcurve_u[0];
    awb_cfg_v30->sw_rawawb_rotu1_ls0 =  awb_meas.ic3Dyuv2Range_param[0].thcurve_u[1];
    awb_cfg_v30->sw_rawawb_rotu2_ls0 =  awb_meas.ic3Dyuv2Range_param[0].thcurve_u[2];
    awb_cfg_v30->sw_rawawb_rotu3_ls0 =  awb_meas.ic3Dyuv2Range_param[0].thcurve_u[3];
    awb_cfg_v30->sw_rawawb_rotu4_ls0 =  awb_meas.ic3Dyuv2Range_param[0].thcurve_u[4];
    awb_cfg_v30->sw_rawawb_rotu5_ls0 =  awb_meas.ic3Dyuv2Range_param[0].thcurve_u[5];
    awb_cfg_v30->sw_rawawb_th0_ls0 =  awb_meas.ic3Dyuv2Range_param[0].thcure_th[0];
    awb_cfg_v30->sw_rawawb_th1_ls0 =  awb_meas.ic3Dyuv2Range_param[0].thcure_th[1];
    awb_cfg_v30->sw_rawawb_th2_ls0 =  awb_meas.ic3Dyuv2Range_param[0].thcure_th[2];
    awb_cfg_v30->sw_rawawb_th3_ls0 =  awb_meas.ic3Dyuv2Range_param[0].thcure_th[3];
    awb_cfg_v30->sw_rawawb_th4_ls0 =  awb_meas.ic3Dyuv2Range_param[0].thcure_th[4];
    awb_cfg_v30->sw_rawawb_th5_ls0 =  awb_meas.ic3Dyuv2Range_param[0].thcure_th[5];
    awb_cfg_v30->sw_rawawb_coor_x1_ls0_u =  awb_meas.ic3Dyuv2Range_param[0].lineP1[0];
    awb_cfg_v30->sw_rawawb_coor_x1_ls0_v =  awb_meas.ic3Dyuv2Range_param[0].lineP1[1];
    awb_cfg_v30->sw_rawawb_coor_x1_ls0_y =  awb_meas.ic3Dyuv2Range_param[0].lineP1[2];
    awb_cfg_v30->sw_rawawb_vec_x21_ls0_u =  awb_meas.ic3Dyuv2Range_param[0].vP1P2[0];
    awb_cfg_v30->sw_rawawb_vec_x21_ls0_v =  awb_meas.ic3Dyuv2Range_param[0].vP1P2[1];
    awb_cfg_v30->sw_rawawb_vec_x21_ls0_y =  awb_meas.ic3Dyuv2Range_param[0].vP1P2[2];
    awb_cfg_v30->sw_rawawb_dis_x1x2_ls0 =  awb_meas.ic3Dyuv2Range_param[0].disP1P2;
    awb_cfg_v30->sw_rawawb_rotu0_ls1 =  awb_meas.ic3Dyuv2Range_param[1].thcurve_u[0];
    awb_cfg_v30->sw_rawawb_rotu1_ls1 =  awb_meas.ic3Dyuv2Range_param[1].thcurve_u[1];
    awb_cfg_v30->sw_rawawb_rotu2_ls1 =  awb_meas.ic3Dyuv2Range_param[1].thcurve_u[2];
    awb_cfg_v30->sw_rawawb_rotu3_ls1 =  awb_meas.ic3Dyuv2Range_param[1].thcurve_u[3];
    awb_cfg_v30->sw_rawawb_rotu4_ls1 =  awb_meas.ic3Dyuv2Range_param[1].thcurve_u[4];
    awb_cfg_v30->sw_rawawb_rotu5_ls1 =  awb_meas.ic3Dyuv2Range_param[1].thcurve_u[5];
    awb_cfg_v30->sw_rawawb_th0_ls1 =  awb_meas.ic3Dyuv2Range_param[1].thcure_th[0];
    awb_cfg_v30->sw_rawawb_th1_ls1 =  awb_meas.ic3Dyuv2Range_param[1].thcure_th[1];
    awb_cfg_v30->sw_rawawb_th2_ls1 =  awb_meas.ic3Dyuv2Range_param[1].thcure_th[2];
    awb_cfg_v30->sw_rawawb_th3_ls1 =  awb_meas.ic3Dyuv2Range_param[1].thcure_th[3];
    awb_cfg_v30->sw_rawawb_th4_ls1 =  awb_meas.ic3Dyuv2Range_param[1].thcure_th[4];
    awb_cfg_v30->sw_rawawb_th5_ls1 =  awb_meas.ic3Dyuv2Range_param[1].thcure_th[5];
    awb_cfg_v30->sw_rawawb_coor_x1_ls1_u =  awb_meas.ic3Dyuv2Range_param[1].lineP1[0];
    awb_cfg_v30->sw_rawawb_coor_x1_ls1_v =  awb_meas.ic3Dyuv2Range_param[1].lineP1[1];
    awb_cfg_v30->sw_rawawb_coor_x1_ls1_y =  awb_meas.ic3Dyuv2Range_param[1].lineP1[2];
    awb_cfg_v30->sw_rawawb_vec_x21_ls1_u =  awb_meas.ic3Dyuv2Range_param[1].vP1P2[0];
    awb_cfg_v30->sw_rawawb_vec_x21_ls1_v =  awb_meas.ic3Dyuv2Range_param[1].vP1P2[1];
    awb_cfg_v30->sw_rawawb_vec_x21_ls1_y =  awb_meas.ic3Dyuv2Range_param[1].vP1P2[2];
    awb_cfg_v30->sw_rawawb_dis_x1x2_ls1 =  awb_meas.ic3Dyuv2Range_param[1].disP1P2;
    awb_cfg_v30->sw_rawawb_rotu0_ls2 =  awb_meas.ic3Dyuv2Range_param[2].thcurve_u[0];
    awb_cfg_v30->sw_rawawb_rotu1_ls2 =  awb_meas.ic3Dyuv2Range_param[2].thcurve_u[1];
    awb_cfg_v30->sw_rawawb_rotu2_ls2 =  awb_meas.ic3Dyuv2Range_param[2].thcurve_u[2];
    awb_cfg_v30->sw_rawawb_rotu3_ls2 =  awb_meas.ic3Dyuv2Range_param[2].thcurve_u[3];
    awb_cfg_v30->sw_rawawb_rotu4_ls2 =  awb_meas.ic3Dyuv2Range_param[2].thcurve_u[4];
    awb_cfg_v30->sw_rawawb_rotu5_ls2 =  awb_meas.ic3Dyuv2Range_param[2].thcurve_u[5];
    awb_cfg_v30->sw_rawawb_th0_ls2 =  awb_meas.ic3Dyuv2Range_param[2].thcure_th[0];
    awb_cfg_v30->sw_rawawb_th1_ls2 =  awb_meas.ic3Dyuv2Range_param[2].thcure_th[1];
    awb_cfg_v30->sw_rawawb_th2_ls2 =  awb_meas.ic3Dyuv2Range_param[2].thcure_th[2];
    awb_cfg_v30->sw_rawawb_th3_ls2 =  awb_meas.ic3Dyuv2Range_param[2].thcure_th[3];
    awb_cfg_v30->sw_rawawb_th4_ls2 =  awb_meas.ic3Dyuv2Range_param[2].thcure_th[4];
    awb_cfg_v30->sw_rawawb_th5_ls2 =  awb_meas.ic3Dyuv2Range_param[2].thcure_th[5];
    awb_cfg_v30->sw_rawawb_coor_x1_ls2_u =  awb_meas.ic3Dyuv2Range_param[2].lineP1[0];
    awb_cfg_v30->sw_rawawb_coor_x1_ls2_v =  awb_meas.ic3Dyuv2Range_param[2].lineP1[1];
    awb_cfg_v30->sw_rawawb_coor_x1_ls2_y =  awb_meas.ic3Dyuv2Range_param[2].lineP1[2];
    awb_cfg_v30->sw_rawawb_vec_x21_ls2_u =  awb_meas.ic3Dyuv2Range_param[2].vP1P2[0];
    awb_cfg_v30->sw_rawawb_vec_x21_ls2_v =  awb_meas.ic3Dyuv2Range_param[2].vP1P2[1];
    awb_cfg_v30->sw_rawawb_vec_x21_ls2_y =  awb_meas.ic3Dyuv2Range_param[2].vP1P2[2];
    awb_cfg_v30->sw_rawawb_dis_x1x2_ls2 =  awb_meas.ic3Dyuv2Range_param[2].disP1P2;

    awb_cfg_v30->sw_rawawb_rotu0_ls3 =  awb_meas.ic3Dyuv2Range_param[3].thcurve_u[0];
    awb_cfg_v30->sw_rawawb_rotu1_ls3 =  awb_meas.ic3Dyuv2Range_param[3].thcurve_u[1];
    awb_cfg_v30->sw_rawawb_rotu2_ls3 =  awb_meas.ic3Dyuv2Range_param[3].thcurve_u[2];
    awb_cfg_v30->sw_rawawb_rotu3_ls3 =  awb_meas.ic3Dyuv2Range_param[3].thcurve_u[3];
    awb_cfg_v30->sw_rawawb_rotu4_ls3 =  awb_meas.ic3Dyuv2Range_param[3].thcurve_u[4];
    awb_cfg_v30->sw_rawawb_rotu5_ls3 =  awb_meas.ic3Dyuv2Range_param[3].thcurve_u[5];
    awb_cfg_v30->sw_rawawb_th0_ls3 =  awb_meas.ic3Dyuv2Range_param[3].thcure_th[0];
    awb_cfg_v30->sw_rawawb_th1_ls3 =  awb_meas.ic3Dyuv2Range_param[3].thcure_th[1];
    awb_cfg_v30->sw_rawawb_th2_ls3 =  awb_meas.ic3Dyuv2Range_param[3].thcure_th[2];
    awb_cfg_v30->sw_rawawb_th3_ls3 =  awb_meas.ic3Dyuv2Range_param[3].thcure_th[3];
    awb_cfg_v30->sw_rawawb_th4_ls3 =  awb_meas.ic3Dyuv2Range_param[3].thcure_th[4];
    awb_cfg_v30->sw_rawawb_th5_ls3 =  awb_meas.ic3Dyuv2Range_param[3].thcure_th[5];
    awb_cfg_v30->sw_rawawb_coor_x1_ls3_u =  awb_meas.ic3Dyuv2Range_param[3].lineP1[0];
    awb_cfg_v30->sw_rawawb_coor_x1_ls3_v =  awb_meas.ic3Dyuv2Range_param[3].lineP1[1];
    awb_cfg_v30->sw_rawawb_coor_x1_ls3_y =  awb_meas.ic3Dyuv2Range_param[3].lineP1[2];
    awb_cfg_v30->sw_rawawb_vec_x21_ls3_u =  awb_meas.ic3Dyuv2Range_param[3].vP1P2[0];
    awb_cfg_v30->sw_rawawb_vec_x21_ls3_v =  awb_meas.ic3Dyuv2Range_param[3].vP1P2[1];
    awb_cfg_v30->sw_rawawb_vec_x21_ls3_y =  awb_meas.ic3Dyuv2Range_param[3].vP1P2[2];
    awb_cfg_v30->sw_rawawb_dis_x1x2_ls3 =  awb_meas.ic3Dyuv2Range_param[3].disP1P2;
    awb_cfg_v30->sw_rawawb_wt0            =  awb_meas.rgb2xy_param.pseudoLuminanceWeight[0];
    awb_cfg_v30->sw_rawawb_wt1            =  awb_meas.rgb2xy_param.pseudoLuminanceWeight[1];
    awb_cfg_v30->sw_rawawb_wt2            =  awb_meas.rgb2xy_param.pseudoLuminanceWeight[2];
    awb_cfg_v30->sw_rawawb_mat0_x         =  awb_meas.rgb2xy_param.rotationMat[0];
    awb_cfg_v30->sw_rawawb_mat1_x         =  awb_meas.rgb2xy_param.rotationMat[1];
    awb_cfg_v30->sw_rawawb_mat2_x         =  awb_meas.rgb2xy_param.rotationMat[2];
    awb_cfg_v30->sw_rawawb_mat0_y         =  awb_meas.rgb2xy_param.rotationMat[3];
    awb_cfg_v30->sw_rawawb_mat1_y         =  awb_meas.rgb2xy_param.rotationMat[4];
    awb_cfg_v30->sw_rawawb_mat2_y         =  awb_meas.rgb2xy_param.rotationMat[5];
    awb_cfg_v30->sw_rawawb_nor_x0_0       =  awb_meas.xyRange_param[0].NorrangeX[0];
    awb_cfg_v30->sw_rawawb_nor_x1_0       =  awb_meas.xyRange_param[0].NorrangeX[1];
    awb_cfg_v30->sw_rawawb_nor_y0_0       =  awb_meas.xyRange_param[0].NorrangeY[0];
    awb_cfg_v30->sw_rawawb_nor_y1_0       =  awb_meas.xyRange_param[0].NorrangeY[1];
    awb_cfg_v30->sw_rawawb_big_x0_0       =  awb_meas.xyRange_param[0].SperangeX[0];
    awb_cfg_v30->sw_rawawb_big_x1_0       =  awb_meas.xyRange_param[0].SperangeX[1];
    awb_cfg_v30->sw_rawawb_big_y0_0       =  awb_meas.xyRange_param[0].SperangeY[0];
    awb_cfg_v30->sw_rawawb_big_y1_0       =  awb_meas.xyRange_param[0].SperangeY[1];
    awb_cfg_v30->sw_rawawb_nor_x0_1       =  awb_meas.xyRange_param[1].NorrangeX[0];
    awb_cfg_v30->sw_rawawb_nor_x1_1       =  awb_meas.xyRange_param[1].NorrangeX[1];
    awb_cfg_v30->sw_rawawb_nor_y0_1       =  awb_meas.xyRange_param[1].NorrangeY[0];
    awb_cfg_v30->sw_rawawb_nor_y1_1       =  awb_meas.xyRange_param[1].NorrangeY[1];
    awb_cfg_v30->sw_rawawb_big_x0_1       =  awb_meas.xyRange_param[1].SperangeX[0];
    awb_cfg_v30->sw_rawawb_big_x1_1       =  awb_meas.xyRange_param[1].SperangeX[1];
    awb_cfg_v30->sw_rawawb_big_y0_1       =  awb_meas.xyRange_param[1].SperangeY[0];
    awb_cfg_v30->sw_rawawb_big_y1_1       =  awb_meas.xyRange_param[1].SperangeY[1];
    awb_cfg_v30->sw_rawawb_nor_x0_2       =  awb_meas.xyRange_param[2].NorrangeX[0];
    awb_cfg_v30->sw_rawawb_nor_x1_2       =  awb_meas.xyRange_param[2].NorrangeX[1];
    awb_cfg_v30->sw_rawawb_nor_y0_2       =  awb_meas.xyRange_param[2].NorrangeY[0];
    awb_cfg_v30->sw_rawawb_nor_y1_2       =  awb_meas.xyRange_param[2].NorrangeY[1];
    awb_cfg_v30->sw_rawawb_big_x0_2       =  awb_meas.xyRange_param[2].SperangeX[0];
    awb_cfg_v30->sw_rawawb_big_x1_2       =  awb_meas.xyRange_param[2].SperangeX[1];
    awb_cfg_v30->sw_rawawb_big_y0_2       =  awb_meas.xyRange_param[2].SperangeY[0];
    awb_cfg_v30->sw_rawawb_big_y1_2       =  awb_meas.xyRange_param[2].SperangeY[1];
    awb_cfg_v30->sw_rawawb_nor_x0_3       =  awb_meas.xyRange_param[3].NorrangeX[0];
    awb_cfg_v30->sw_rawawb_nor_x1_3       =  awb_meas.xyRange_param[3].NorrangeX[1];
    awb_cfg_v30->sw_rawawb_nor_y0_3       =  awb_meas.xyRange_param[3].NorrangeY[0];
    awb_cfg_v30->sw_rawawb_nor_y1_3       =  awb_meas.xyRange_param[3].NorrangeY[1];
    awb_cfg_v30->sw_rawawb_big_x0_3       =  awb_meas.xyRange_param[3].SperangeX[0];
    awb_cfg_v30->sw_rawawb_big_x1_3       =  awb_meas.xyRange_param[3].SperangeX[1];
    awb_cfg_v30->sw_rawawb_big_y0_3       =  awb_meas.xyRange_param[3].SperangeY[0];
    awb_cfg_v30->sw_rawawb_big_y1_3       =  awb_meas.xyRange_param[3].SperangeY[1];
    awb_cfg_v30->sw_rawawb_nor_x0_4       =  awb_meas.xyRange_param[4].NorrangeX[0];
    awb_cfg_v30->sw_rawawb_nor_x1_4       =  awb_meas.xyRange_param[4].NorrangeX[1];
    awb_cfg_v30->sw_rawawb_nor_y0_4       =  awb_meas.xyRange_param[4].NorrangeY[0];
    awb_cfg_v30->sw_rawawb_nor_y1_4       =  awb_meas.xyRange_param[4].NorrangeY[1];
    awb_cfg_v30->sw_rawawb_big_x0_4       =  awb_meas.xyRange_param[4].SperangeX[0];
    awb_cfg_v30->sw_rawawb_big_x1_4       =  awb_meas.xyRange_param[4].SperangeX[1];
    awb_cfg_v30->sw_rawawb_big_y0_4       =  awb_meas.xyRange_param[4].SperangeY[0];
    awb_cfg_v30->sw_rawawb_big_y1_4       =  awb_meas.xyRange_param[4].SperangeY[1];
    awb_cfg_v30->sw_rawawb_nor_x0_5       =  awb_meas.xyRange_param[5].NorrangeX[0];
    awb_cfg_v30->sw_rawawb_nor_x1_5       =  awb_meas.xyRange_param[5].NorrangeX[1];
    awb_cfg_v30->sw_rawawb_nor_y0_5       =  awb_meas.xyRange_param[5].NorrangeY[0];
    awb_cfg_v30->sw_rawawb_nor_y1_5       =  awb_meas.xyRange_param[5].NorrangeY[1];
    awb_cfg_v30->sw_rawawb_big_x0_5       =  awb_meas.xyRange_param[5].SperangeX[0];
    awb_cfg_v30->sw_rawawb_big_x1_5       =  awb_meas.xyRange_param[5].SperangeX[1];
    awb_cfg_v30->sw_rawawb_big_y0_5       =  awb_meas.xyRange_param[5].SperangeY[0];
    awb_cfg_v30->sw_rawawb_big_y1_5       =  awb_meas.xyRange_param[5].SperangeY[1];
    awb_cfg_v30->sw_rawawb_nor_x0_6       =  awb_meas.xyRange_param[6].NorrangeX[0];
    awb_cfg_v30->sw_rawawb_nor_x1_6       =  awb_meas.xyRange_param[6].NorrangeX[1];
    awb_cfg_v30->sw_rawawb_nor_y0_6       =  awb_meas.xyRange_param[6].NorrangeY[0];
    awb_cfg_v30->sw_rawawb_nor_y1_6       =  awb_meas.xyRange_param[6].NorrangeY[1];
    awb_cfg_v30->sw_rawawb_big_x0_6       =  awb_meas.xyRange_param[6].SperangeX[0];
    awb_cfg_v30->sw_rawawb_big_x1_6       =  awb_meas.xyRange_param[6].SperangeX[1];
    awb_cfg_v30->sw_rawawb_big_y0_6       =  awb_meas.xyRange_param[6].SperangeY[0];
    awb_cfg_v30->sw_rawawb_big_y1_6       =  awb_meas.xyRange_param[6].SperangeY[1];
    awb_cfg_v30->sw_rawawb_pre_wbgain_inv_r       =  awb_meas.pre_wbgain_inv_r;
    awb_cfg_v30->sw_rawawb_pre_wbgain_inv_g       =  awb_meas.pre_wbgain_inv_g;
    awb_cfg_v30->sw_rawawb_pre_wbgain_inv_b       =  awb_meas.pre_wbgain_inv_b;
    //multiwindow
#if defined(ISP_HW_V30)
    awb_cfg_v30->sw_rawawb_multiwindow_en          =      awb_meas.multiwindow_en;
    awb_cfg_v30->sw_rawawb_multiwindow0_h_offs     =      awb_meas.multiwindow[0][0];
    awb_cfg_v30->sw_rawawb_multiwindow0_v_offs     =      awb_meas.multiwindow[0][1];
    awb_cfg_v30->sw_rawawb_multiwindow0_h_size     =      awb_meas.multiwindow[0][0] + awb_meas.multiwindow[0][2];
    awb_cfg_v30->sw_rawawb_multiwindow0_v_size     =      awb_meas.multiwindow[0][1] + awb_meas.multiwindow[0][3];
    awb_cfg_v30->sw_rawawb_multiwindow1_h_offs     =      awb_meas.multiwindow[1][0];
    awb_cfg_v30->sw_rawawb_multiwindow1_v_offs     =      awb_meas.multiwindow[1][1];
    awb_cfg_v30->sw_rawawb_multiwindow1_h_size     =      awb_meas.multiwindow[1][0] + awb_meas.multiwindow[1][2];
    awb_cfg_v30->sw_rawawb_multiwindow1_v_size     =      awb_meas.multiwindow[1][1] + awb_meas.multiwindow[1][3];
    awb_cfg_v30->sw_rawawb_multiwindow2_h_offs     =      awb_meas.multiwindow[2][0];
    awb_cfg_v30->sw_rawawb_multiwindow2_v_offs     =      awb_meas.multiwindow[2][1];
    awb_cfg_v30->sw_rawawb_multiwindow2_h_size     =      awb_meas.multiwindow[2][0] + awb_meas.multiwindow[2][2];
    awb_cfg_v30->sw_rawawb_multiwindow2_v_size     =      awb_meas.multiwindow[2][1] + awb_meas.multiwindow[2][3];
    awb_cfg_v30->sw_rawawb_multiwindow3_h_offs     =      awb_meas.multiwindow[3][0];
    awb_cfg_v30->sw_rawawb_multiwindow3_v_offs     =      awb_meas.multiwindow[3][1];
    awb_cfg_v30->sw_rawawb_multiwindow3_h_size     =      awb_meas.multiwindow[3][0] + awb_meas.multiwindow[3][2];
    awb_cfg_v30->sw_rawawb_multiwindow3_v_size     =      awb_meas.multiwindow[3][1] + awb_meas.multiwindow[3][3];

#endif
    awb_cfg_v30->sw_rawawb_exc_wp_region0_excen0     =  awb_meas.excludeWpRange[0].excludeEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201];
    awb_cfg_v30->sw_rawawb_exc_wp_region0_excen1     =  awb_meas.excludeWpRange[0].excludeEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201];
    awb_cfg_v30->sw_rawawb_exc_wp_region0_measen   =  awb_meas.excludeWpRange[0].measureEnable;
    awb_cfg_v30->sw_rawawb_exc_wp_region0_domain        =  awb_meas.excludeWpRange[0].domain;
    awb_cfg_v30->sw_rawawb_exc_wp_region0_xu0          =  awb_meas.excludeWpRange[0].xu[0];
    awb_cfg_v30->sw_rawawb_exc_wp_region0_xu1          =  awb_meas.excludeWpRange[0].xu[1];
    awb_cfg_v30->sw_rawawb_exc_wp_region0_yv0          =  awb_meas.excludeWpRange[0].yv[0];
    awb_cfg_v30->sw_rawawb_exc_wp_region0_yv1          =  awb_meas.excludeWpRange[0].yv[1];
    awb_cfg_v30->sw_rawawb_exc_wp_region1_excen0     =  awb_meas.excludeWpRange[1].excludeEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201];
    awb_cfg_v30->sw_rawawb_exc_wp_region1_excen1     =  awb_meas.excludeWpRange[1].excludeEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201];
    awb_cfg_v30->sw_rawawb_exc_wp_region1_measen   =  awb_meas.excludeWpRange[1].measureEnable;
    awb_cfg_v30->sw_rawawb_exc_wp_region1_domain        =  awb_meas.excludeWpRange[1].domain;
    awb_cfg_v30->sw_rawawb_exc_wp_region1_xu0          =  awb_meas.excludeWpRange[1].xu[0];
    awb_cfg_v30->sw_rawawb_exc_wp_region1_xu1          =  awb_meas.excludeWpRange[1].xu[1];
    awb_cfg_v30->sw_rawawb_exc_wp_region1_yv0          =  awb_meas.excludeWpRange[1].yv[0];
    awb_cfg_v30->sw_rawawb_exc_wp_region1_yv1          =  awb_meas.excludeWpRange[1].yv[1];
    awb_cfg_v30->sw_rawawb_exc_wp_region2_excen0     =  awb_meas.excludeWpRange[2].excludeEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201];
    awb_cfg_v30->sw_rawawb_exc_wp_region2_excen1     =  awb_meas.excludeWpRange[2].excludeEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201];
    awb_cfg_v30->sw_rawawb_exc_wp_region2_measen   =  awb_meas.excludeWpRange[2].measureEnable;
    awb_cfg_v30->sw_rawawb_exc_wp_region2_domain        =  awb_meas.excludeWpRange[2].domain;
    awb_cfg_v30->sw_rawawb_exc_wp_region2_xu0          =  awb_meas.excludeWpRange[2].xu[0];
    awb_cfg_v30->sw_rawawb_exc_wp_region2_xu1          =  awb_meas.excludeWpRange[2].xu[1];
    awb_cfg_v30->sw_rawawb_exc_wp_region2_yv0          =  awb_meas.excludeWpRange[2].yv[0];
    awb_cfg_v30->sw_rawawb_exc_wp_region2_yv1          =  awb_meas.excludeWpRange[2].yv[1];
    awb_cfg_v30->sw_rawawb_exc_wp_region3_excen0     =  awb_meas.excludeWpRange[3].excludeEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201];
    awb_cfg_v30->sw_rawawb_exc_wp_region3_excen1     =  awb_meas.excludeWpRange[3].excludeEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201];
    awb_cfg_v30->sw_rawawb_exc_wp_region3_measen   =  awb_meas.excludeWpRange[3].measureEnable;
    awb_cfg_v30->sw_rawawb_exc_wp_region3_domain        =  awb_meas.excludeWpRange[3].domain;
    awb_cfg_v30->sw_rawawb_exc_wp_region3_xu0          =  awb_meas.excludeWpRange[3].xu[0];
    awb_cfg_v30->sw_rawawb_exc_wp_region3_xu1          =  awb_meas.excludeWpRange[3].xu[1];
    awb_cfg_v30->sw_rawawb_exc_wp_region3_yv0          =  awb_meas.excludeWpRange[3].yv[0];
    awb_cfg_v30->sw_rawawb_exc_wp_region3_yv1          =  awb_meas.excludeWpRange[3].yv[1];
    awb_cfg_v30->sw_rawawb_exc_wp_region4_excen0     =  awb_meas.excludeWpRange[4].excludeEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201];
    awb_cfg_v30->sw_rawawb_exc_wp_region4_excen1     =  awb_meas.excludeWpRange[4].excludeEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201];
    awb_cfg_v30->sw_rawawb_exc_wp_region4_domain        =  awb_meas.excludeWpRange[4].domain;
    awb_cfg_v30->sw_rawawb_exc_wp_region4_xu0          =  awb_meas.excludeWpRange[4].xu[0];
    awb_cfg_v30->sw_rawawb_exc_wp_region4_xu1          =  awb_meas.excludeWpRange[4].xu[1];
    awb_cfg_v30->sw_rawawb_exc_wp_region4_yv0          =  awb_meas.excludeWpRange[4].yv[0];
    awb_cfg_v30->sw_rawawb_exc_wp_region4_yv1          =  awb_meas.excludeWpRange[4].yv[1];
    awb_cfg_v30->sw_rawawb_exc_wp_region5_excen0     =  awb_meas.excludeWpRange[5].excludeEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201];
    awb_cfg_v30->sw_rawawb_exc_wp_region5_excen1     =  awb_meas.excludeWpRange[5].excludeEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201];
    awb_cfg_v30->sw_rawawb_exc_wp_region5_domain        =  awb_meas.excludeWpRange[5].domain;
    awb_cfg_v30->sw_rawawb_exc_wp_region5_xu0          =  awb_meas.excludeWpRange[5].xu[0];
    awb_cfg_v30->sw_rawawb_exc_wp_region5_xu1          =  awb_meas.excludeWpRange[5].xu[1];
    awb_cfg_v30->sw_rawawb_exc_wp_region5_yv0          =  awb_meas.excludeWpRange[5].yv[0];
    awb_cfg_v30->sw_rawawb_exc_wp_region5_yv1          =  awb_meas.excludeWpRange[5].yv[1];
    awb_cfg_v30->sw_rawawb_exc_wp_region6_excen0     =  awb_meas.excludeWpRange[6].excludeEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201];
    awb_cfg_v30->sw_rawawb_exc_wp_region6_excen1     =  awb_meas.excludeWpRange[6].excludeEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201];
    awb_cfg_v30->sw_rawawb_exc_wp_region6_domain        =  awb_meas.excludeWpRange[6].domain;
    awb_cfg_v30->sw_rawawb_exc_wp_region6_xu0          =  awb_meas.excludeWpRange[6].xu[0];
    awb_cfg_v30->sw_rawawb_exc_wp_region6_xu1          =  awb_meas.excludeWpRange[6].xu[1];
    awb_cfg_v30->sw_rawawb_exc_wp_region6_yv0          =  awb_meas.excludeWpRange[6].yv[0];
    awb_cfg_v30->sw_rawawb_exc_wp_region6_yv1          =  awb_meas.excludeWpRange[6].yv[1];
    awb_cfg_v30->sw_rawawb_wp_luma_weicurve_y0     =   awb_meas.wpDiffwei_y[0];
    awb_cfg_v30->sw_rawawb_wp_luma_weicurve_y1     =   awb_meas.wpDiffwei_y[1];
    awb_cfg_v30->sw_rawawb_wp_luma_weicurve_y2     =   awb_meas.wpDiffwei_y[2];
    awb_cfg_v30->sw_rawawb_wp_luma_weicurve_y3     =   awb_meas.wpDiffwei_y[3];
    awb_cfg_v30->sw_rawawb_wp_luma_weicurve_y4     =   awb_meas.wpDiffwei_y[4];
    awb_cfg_v30->sw_rawawb_wp_luma_weicurve_y5     =   awb_meas.wpDiffwei_y[5];
    awb_cfg_v30->sw_rawawb_wp_luma_weicurve_y6     =   awb_meas.wpDiffwei_y[6];
    awb_cfg_v30->sw_rawawb_wp_luma_weicurve_y7     =   awb_meas.wpDiffwei_y[7];
    awb_cfg_v30->sw_rawawb_wp_luma_weicurve_y8     =   awb_meas.wpDiffwei_y[8];
    awb_cfg_v30->sw_rawawb_wp_luma_weicurve_w0     = awb_meas.wpDiffwei_w[0];
    awb_cfg_v30->sw_rawawb_wp_luma_weicurve_w1     = awb_meas.wpDiffwei_w[1];
    awb_cfg_v30->sw_rawawb_wp_luma_weicurve_w2     = awb_meas.wpDiffwei_w[2];
    awb_cfg_v30->sw_rawawb_wp_luma_weicurve_w3     = awb_meas.wpDiffwei_w[3];
    awb_cfg_v30->sw_rawawb_wp_luma_weicurve_w4     = awb_meas.wpDiffwei_w[4];
    awb_cfg_v30->sw_rawawb_wp_luma_weicurve_w5     = awb_meas.wpDiffwei_w[5];
    awb_cfg_v30->sw_rawawb_wp_luma_weicurve_w6     = awb_meas.wpDiffwei_w[6];
    awb_cfg_v30->sw_rawawb_wp_luma_weicurve_w7     = awb_meas.wpDiffwei_w[7];
    awb_cfg_v30->sw_rawawb_wp_luma_weicurve_w8     = awb_meas.wpDiffwei_w[8];

    for (int i = 0; i < RK_AIQ_AWB_GRID_NUM_TOTAL; i++) {
        awb_cfg_v30->sw_rawawb_wp_blk_wei_w[i]          = awb_meas.blkWeight[i];
    }

    awb_cfg_v30->sw_rawawb_blk_rtdw_measure_en =  awb_meas.blk_rtdw_measure_en;

}
#endif
#if RKAIQ_HAVE_BAYER2DNR_V2
void Isp3xParams::convertAiqRawnrToIsp3xParams(struct isp3x_isp_params_cfg& isp_cfg,
        rk_aiq_isp_baynr_v3x_t& rawnr)
{

    LOGD_ANR("%s:%d enter! enable:%d \n", __FUNCTION__, __LINE__, rawnr.baynr_en);
    bool enable = rawnr.baynr_en;

    // use weight=0 to bypss bayer2dnr effect, then  always  keep 2dnr en bit = 1.
#if 0
   if(enable) {
       isp_cfg.module_ens |= ISP3X_MODULE_BAYNR;
   } else {
       isp_cfg.module_ens &= ~(ISP3X_MODULE_BAYNR);
   }
#else
    isp_cfg.module_ens |= ISP3X_MODULE_BAYNR;
#endif
    isp_cfg.module_en_update |= ISP3X_MODULE_BAYNR;
    isp_cfg.module_cfg_update |= ISP3X_MODULE_BAYNR;

    struct isp3x_baynr_cfg *pBayernr = &isp_cfg.others.baynr_cfg;
    pBayernr->lg2_mode = rawnr.baynr_lg2_mode;
    pBayernr->gauss_en = rawnr.baynr_gauss_en;
    pBayernr->log_bypass = rawnr.baynr_log_bypass;

    pBayernr->dgain0 = rawnr.baynr_dgain[0];
    pBayernr->dgain1 = rawnr.baynr_dgain[1];
    pBayernr->dgain2 = rawnr.baynr_dgain[2];

    pBayernr->pix_diff = rawnr.baynr_pix_diff;

    pBayernr->diff_thld = rawnr.baynr_diff_thld;
    pBayernr->softthld = rawnr.baynr_softthld;

    pBayernr->bltflt_streng = rawnr.bltflt_streng;
#if 0
    pBayernr->reg_w1 = rawnr.baynr_reg_w1;
#else
    if (enable) {
        pBayernr->reg_w1 = rawnr.baynr_reg_w1;
    } else {
        pBayernr->reg_w1 = 0;
    }
#endif

    for(int i = 0; i < ISP3X_BAYNR_XY_NUM; i++) {
        pBayernr->sigma_x[i] = rawnr.sigma_x[i];
        pBayernr->sigma_y[i] = rawnr.sigma_y[i];
    }

    pBayernr->weit_d2 = rawnr.weit_d[2];
    pBayernr->weit_d1 = rawnr.weit_d[1];
    pBayernr->weit_d0 = rawnr.weit_d[0];

    pBayernr->lg2_lgoff = rawnr.lg2_lgoff;
    pBayernr->lg2_off = rawnr.lg2_off;

    pBayernr->dat_max = rawnr.dat_max;

    LOGD_ANR("%s:%d exit!\n", __FUNCTION__, __LINE__);
}
#endif
#if RKAIQ_HAVE_BAYERTNR_V2
void Isp3xParams::convertAiqTnrToIsp3xParams(struct isp3x_isp_params_cfg& isp_cfg,
        rk_aiq_isp_tnr_v3x_t& tnr)
{
    LOGD_ANR("%s:%d enter! enable:%d\n", __FUNCTION__, __LINE__, tnr.bay3d_en_i);
    bool enable = tnr.bay3d_en_i;
    if(enable) {
        isp_cfg.module_ens |= ISP3X_MODULE_BAY3D;
        // use 2dnr weight to bypss 2dnr effect, not need 3dnr state at all.
        // just keep 2dnr and 3dnr en bit always 1
#if 0
       //bayer3dnr enable  bayer2dnr must enable at the same time
       isp_cfg.module_ens |= ISP3X_MODULE_BAYNR;
       isp_cfg.module_en_update |= ISP3X_MODULE_BAYNR;
#endif
    } else {
        //isp_cfg.module_ens &= ~(ISP3X_MODULE_BAY3D);
        isp_cfg.module_ens |= ISP3X_MODULE_BAY3D;
    }

    isp_cfg.module_cfg_update |= ISP3X_MODULE_BAY3D;
    isp_cfg.module_en_update |= ISP3X_MODULE_BAY3D;


    struct isp3x_bay3d_cfg *pBayertnr = &isp_cfg.others.bay3d_cfg;

    pBayertnr->bypass_en = tnr.bay3d_bypass_en;
    pBayertnr->hibypass_en = tnr.bay3d_hibypass_en;
    pBayertnr->lobypass_en = tnr.bay3d_lobypass_en;
    pBayertnr->himed_bypass_en = tnr.bay3d_himed_bypass_en;
    pBayertnr->higaus_bypass_en = tnr.bay3d_higaus_bypass_en;
    pBayertnr->hiabs_possel = tnr.bay3d_hiabs_pssel;
    pBayertnr->hichnsplit_en = tnr.bay3d_hichnsplit_en;
    pBayertnr->lomed_bypass_en = tnr.bay3d_lomed_bypass_en;
    pBayertnr->logaus5_bypass_en = tnr.bay3d_logaus5_bypass_en;
    pBayertnr->logaus3_bypass_en = tnr.bay3d_logaus3_bypass_en;
    pBayertnr->glbpk_en = tnr.bay3d_glbpk_en;
    pBayertnr->loswitch_protect = tnr.bay3d_loswitch_protect;

    pBayertnr->softwgt = tnr.bay3d_softwgt;
    pBayertnr->hidif_th = tnr.bay3d_hidif_th;

    pBayertnr->glbpk2 = tnr.bay3d_glbpk2;

    pBayertnr->wgtlmt = tnr.bay3d_wgtlmt;
    pBayertnr->wgtratio = tnr.bay3d_wgtratio;

    for(int i = 0; i < ISP3X_BAY3D_XY_NUM; i++) {
        pBayertnr->sig0_x[i] = tnr.bay3d_sig0_x[i];
        pBayertnr->sig0_y[i] = tnr.bay3d_sig0_y[i];

        pBayertnr->sig1_x[i] = tnr.bay3d_sig1_x[i];
        pBayertnr->sig1_y[i] = tnr.bay3d_sig1_y[i];

        //pBayertnr->sig2_x[i] = tnr.bay3d_sig1_x[i];
        pBayertnr->sig2_y[i] = tnr.bay3d_sig2_y[i];
    }
    LOGD_ANR("%s:%d exit!\n", __FUNCTION__, __LINE__);
}
#endif
#if RKAIQ_HAVE_CNR_V2
void Isp3xParams::convertAiqUvnrToIsp3xParams(struct isp3x_isp_params_cfg& isp_cfg,
        rk_aiq_isp_cnr_v3x_t& uvnr)
{
    LOGD_ANR("%s:%d enter! enable:%d \n", __FUNCTION__, __LINE__, uvnr.cnr_en_i);

    bool enable = uvnr.cnr_en_i;

    isp_cfg.module_en_update |= ISP3X_MODULE_CNR;
    isp_cfg.module_ens |= ISP3X_MODULE_CNR;
    isp_cfg.module_cfg_update |= ISP3X_MODULE_CNR;


    struct isp3x_cnr_cfg *pCnr = &isp_cfg.others.cnr_cfg;

    pCnr->thumb_mix_cur_en = uvnr.cnr_thumb_mix_cur_en;
    pCnr->lq_bila_bypass = uvnr.cnr_lq_bila_bypass;
    pCnr->hq_bila_bypass = uvnr.cnr_hq_bila_bypass;
    pCnr->exgain_bypass = uvnr.cnr_exgain_bypass;
    if(!enable) {
        pCnr->lq_bila_bypass = 0x01;
        pCnr->hq_bila_bypass = 0x01;
        pCnr->exgain_bypass = 0x01;
    }

    pCnr->global_gain_alpha = uvnr.cnr_global_gain_alpha;
    pCnr->global_gain = uvnr.cnr_global_gain;

    pCnr->gain_iso = uvnr.cnr_gain_iso;
    pCnr->gain_offset = uvnr.cnr_gain_offset;
    pCnr->gain_1sigma = uvnr.cnr_gain_1sigma;

    pCnr->gain_uvgain1 = uvnr.cnr_gain_uvgain1;
    pCnr->gain_uvgain0 = uvnr.cnr_gain_uvgain0;

    pCnr->lmed3_alpha = uvnr.cnr_lmed3_alpha;

    pCnr->lbf5_gain_y = uvnr.cnr_lbf5_gain_y;
    pCnr->lbf5_gain_c = uvnr.cnr_lbf5_gain_c;

    pCnr->lbf5_weit_d3 = uvnr.cnr_lbf5_weit_d[3];
    pCnr->lbf5_weit_d2 = uvnr.cnr_lbf5_weit_d[2];
    pCnr->lbf5_weit_d1 = uvnr.cnr_lbf5_weit_d[1];
    pCnr->lbf5_weit_d0 = uvnr.cnr_lbf5_weit_d[0];

    pCnr->lbf5_weit_d4 = uvnr.cnr_lbf5_weit_d[4];

    pCnr->hmed3_alpha = uvnr.cnr_hmed3_alpha;

    pCnr->hbf5_weit_src = uvnr.cnr_hbf5_weit_src;
    pCnr->hbf5_min_wgt = uvnr.cnr_hbf5_min_wgt;
    pCnr->hbf5_sigma = uvnr.cnr_hbf5_sigma;

    pCnr->lbf5_weit_src = uvnr.cnr_lbf5_weit_src;
    pCnr->lbf3_sigma = uvnr.cnr_lbf3_sigma;

    for(int i = 0; i < ISP3X_CNR_SIGMA_Y_NUM; i++) {
        pCnr->sigma_y[i] = uvnr.cnr_sigma_y[i];
    }
    LOGD_ANR("%s:%d exit!\n", __FUNCTION__, __LINE__);
}
#endif
#if RKAIQ_HAVE_YNR_V3
void Isp3xParams::convertAiqYnrToIsp3xParams(struct isp3x_isp_params_cfg& isp_cfg,
        rk_aiq_isp_ynr_v3x_t& ynr)
{
    LOGD_ANR("%s:%d enter! enable:%d \n", __FUNCTION__, __LINE__, ynr.ynr_en);

    bool enable = ynr.ynr_en;

    isp_cfg.module_en_update |= ISP3X_MODULE_YNR;
    isp_cfg.module_ens |= ISP3X_MODULE_YNR;
    isp_cfg.module_cfg_update |= ISP3X_MODULE_YNR;

    struct isp3x_ynr_cfg *pYnr = &isp_cfg.others.ynr_cfg;

    pYnr->rnr_en = ynr.ynr_rnr_en;
    pYnr->thumb_mix_cur_en = ynr.ynr_thumb_mix_cur_en;
    pYnr->global_gain_alpha = ynr.ynr_global_gain_alpha;
    pYnr->global_gain = ynr.ynr_global_gain;
    pYnr->flt1x1_bypass_sel = ynr.ynr_flt1x1_bypass_sel;
    pYnr->sft5x5_bypass = ynr.ynr_sft5x5_bypass;
    pYnr->flt1x1_bypass_sel = ynr.ynr_flt1x1_bypass_sel;
    pYnr->flt1x1_bypass = ynr.ynr_flt1x1_bypass;
    pYnr->lgft3x3_bypass = ynr.ynr_lgft3x3_bypass;
    pYnr->lbft5x5_bypass = ynr.ynr_lbft5x5_bypass;
    pYnr->bft3x3_bypass = ynr.ynr_bft3x3_bypass;
    if(!enable) {
        pYnr->sft5x5_bypass = 0x01;
        pYnr->flt1x1_bypass = 0x01;
        pYnr->lgft3x3_bypass = 0x01;
        pYnr->lbft5x5_bypass = 0x01;
        pYnr->bft3x3_bypass = 0x01;
    }

    pYnr->rnr_max_r = ynr.ynr_rnr_max_r;
    pYnr->local_gainscale = ynr.ynr_local_gainscale;

    pYnr->rnr_center_coorh = ynr.ynr_rnr_center_coorh;
    pYnr->rnr_center_coorv = ynr.ynr_rnr_center_coorv;

    pYnr->loclagain_adj_thresh = ynr.ynr_localgain_adj_thresh;
    pYnr->localgain_adj = ynr.ynr_localgain_adj;

    pYnr->low_bf_inv1 = ynr.ynr_low_bf_inv[1];
    pYnr->low_bf_inv0 = ynr.ynr_low_bf_inv[0];

    pYnr->low_peak_supress = ynr.ynr_low_peak_supress;
    pYnr->low_thred_adj = ynr.ynr_low_thred_adj;

    pYnr->low_dist_adj = ynr.ynr_low_dist_adj;
    pYnr->low_edge_adj_thresh = ynr.ynr_low_edge_adj_thresh;

    pYnr->low_bi_weight = ynr.ynr_low_bi_weight;
    pYnr->low_weight = ynr.ynr_low_weight;
    pYnr->low_center_weight = ynr.ynr_low_center_weight;

    pYnr->hi_min_adj = ynr.ynr_hi_min_adj;
    pYnr->high_thred_adj = ynr.ynr_high_thred_adj;
    pYnr->high_retain_weight = ynr.ynr_high_retain_weight;
    pYnr->hi_edge_thed = ynr.ynr_hi_edge_thed;

    pYnr->base_filter_weight2 = ynr.ynr_base_filter_weight[2];
    pYnr->base_filter_weight1 = ynr.ynr_base_filter_weight[1];
    pYnr->base_filter_weight0 = ynr.ynr_base_filter_weight[0];

    pYnr->frame_full_size = ynr.ynr_frame_full_size;
    pYnr->lbf_weight_thres = ynr.ynr_lbf_weight_thres;

    pYnr->low_gauss1_coeff2 = ynr.ynr_low_gauss1_coeff[2];
    pYnr->low_gauss1_coeff1 = ynr.ynr_low_gauss1_coeff[1];
    pYnr->low_gauss1_coeff0 = ynr.ynr_low_gauss1_coeff[0];

    pYnr->low_gauss2_coeff2 = ynr.ynr_low_gauss2_coeff[2];
    pYnr->low_gauss2_coeff1 = ynr.ynr_low_gauss2_coeff[1];
    pYnr->low_gauss2_coeff0 = ynr.ynr_low_gauss2_coeff[0];

    pYnr->direction_weight3 = ynr.ynr_direction_weight[3];
    pYnr->direction_weight2 = ynr.ynr_direction_weight[2];
    pYnr->direction_weight1 = ynr.ynr_direction_weight[1];
    pYnr->direction_weight0 = ynr.ynr_direction_weight[0];

    pYnr->direction_weight7 = ynr.ynr_direction_weight[7];
    pYnr->direction_weight6 = ynr.ynr_direction_weight[6];
    pYnr->direction_weight5 = ynr.ynr_direction_weight[5];
    pYnr->direction_weight4 = ynr.ynr_direction_weight[4];

    for(int i = 0; i < ISP3X_YNR_XY_NUM; i++) {
        pYnr->luma_points_x[i] = ynr.ynr_luma_points_x[i];
        pYnr->lsgm_y[i] = ynr.ynr_lsgm_y[i];
        pYnr->hsgm_y[i] = ynr.ynr_hsgm_y[i];
        pYnr->rnr_strength3[i] = ynr.ynr_rnr_strength[i];
    }
    LOGD_ANR("%s:%d exit!\n", __FUNCTION__, __LINE__);
}
#endif
#if RKAIQ_HAVE_SHARP_V4
void Isp3xParams::convertAiqSharpenToIsp3xParams(struct isp3x_isp_params_cfg& isp_cfg,
        rk_aiq_isp_sharp_v3x_t& sharp)
{
    LOGD_ANR("%s:%d enter! enable:%d\n", __FUNCTION__, __LINE__, sharp.sharp_en);
    bool enable = sharp.sharp_en;

    isp_cfg.module_en_update |= ISP3X_MODULE_SHARP;
    isp_cfg.module_ens |= ISP3X_MODULE_SHARP;
    isp_cfg.module_cfg_update |= ISP3X_MODULE_SHARP;


    struct isp3x_sharp_cfg *pSharp = &isp_cfg.others.sharp_cfg;

    pSharp->exgain_bypass = sharp.sharp_exgain_bypass;
    pSharp->center_mode = sharp.sharp_center_mode;
    pSharp->bypass = sharp.sharp_bypass;
    if(!enable) {
        pSharp->bypass = 0x01;
    }

    pSharp->sharp_ratio = sharp.sharp_sharp_ratio;
    pSharp->bf_ratio = sharp.sharp_bf_ratio;
    pSharp->gaus_ratio = sharp.sharp_gaus_ratio;
    pSharp->pbf_ratio = sharp.sharp_pbf_ratio;

    for(int i = 0; i < ISP3X_SHARP_X_NUM; i++) {
        pSharp->luma_dx[i] = sharp.sharp_luma_dx[i];
    }

    for(int i = 0; i < ISP3X_SHARP_Y_NUM; i++) {
        pSharp->pbf_sigma_inv[i] = sharp.sharp_pbf_sigma_inv[i];
        pSharp->bf_sigma_inv[i] = sharp.sharp_bf_sigma_inv[i];
    }

    pSharp->bf_sigma_shift = sharp.sharp_bf_sigma_shift;
    pSharp->pbf_sigma_shift = sharp.sharp_pbf_sigma_shift;

    for(int i = 0; i < ISP3X_SHARP_Y_NUM; i++) {
        pSharp->ehf_th[i] = sharp.sharp_ehf_th[i];
        pSharp->clip_hf[i] = sharp.sharp_clip_hf[i];
    }

    pSharp->pbf_coef2 = sharp.sharp_pbf_coef[2];
    pSharp->pbf_coef1 = sharp.sharp_pbf_coef[1];
    pSharp->pbf_coef0 = sharp.sharp_pbf_coef[0];

    pSharp->bf_coef2 = sharp.sharp_bf_coef[2];
    pSharp->bf_coef1 = sharp.sharp_bf_coef[1];
    pSharp->bf_coef0 = sharp.sharp_bf_coef[0];

    for(int i = 0; i < ISP3X_SHARP_GAUS_COEF_NUM; i++) {
        pSharp->gaus_coef[i] = sharp.sharp_gaus_coef[i];
    }
    LOGD_ANR("%s:%d exit!\n", __FUNCTION__, __LINE__);
}
#endif

#if RKAIQ_HAVE_GAIN_V2
template <class T>
void Isp3xParams::convertAiqGainToIsp3xParams(T& isp_cfg, rk_aiq_isp_gain_v3x_t& gain) {
    LOGD_ANR("%s:%d enter! enable:%d gain:0x%x 0x%x 0x%x\n", __FUNCTION__, __LINE__,
             gain.sw_gain_en,
             gain.sw_gain[0],
             gain.sw_gain[1],
             gain.sw_gain[2]);

    bool enable = gain.sw_gain_en;
    if(enable) {
        isp_cfg.module_ens |= ISP3X_MODULE_GAIN;
    } else {
        isp_cfg.module_ens &= ~(ISP3X_MODULE_GAIN);
    }

    isp_cfg.module_en_update |= ISP3X_MODULE_GAIN;
    isp_cfg.module_cfg_update |= (ISP3X_MODULE_GAIN);

    struct isp3x_gain_cfg *pGain = &isp_cfg.others.gain_cfg;

    pGain->g0 = gain.sw_gain[0];
    pGain->g1 = gain.sw_gain[1];
    pGain->g2 = gain.sw_gain[2];

    LOGD_ANR("%s:%d exit!\n", __FUNCTION__, __LINE__);
}
#endif

#if RKAIQ_HAVE_DRC_V11
void Isp3xParams::convertAiqDrcToIsp3xParams(struct isp3x_isp_params_cfg& isp_cfg,
        rk_aiq_isp_drc_v3x_t& adrc_data)
{
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

    isp_cfg.others.drc_cfg.bypass_en       = adrc_data.DrcProcRes.Drc_v11.bypass_en;
    isp_cfg.others.drc_cfg.offset_pow2     = adrc_data.DrcProcRes.Drc_v11.offset_pow2;
    isp_cfg.others.drc_cfg.compres_scl     = adrc_data.DrcProcRes.Drc_v11.compres_scl;
    isp_cfg.others.drc_cfg.position        = adrc_data.DrcProcRes.Drc_v11.position;
    isp_cfg.others.drc_cfg.delta_scalein   = adrc_data.DrcProcRes.Drc_v11.delta_scalein;
    isp_cfg.others.drc_cfg.hpdetail_ratio  = adrc_data.DrcProcRes.Drc_v11.hpdetail_ratio;
    isp_cfg.others.drc_cfg.lpdetail_ratio  = adrc_data.DrcProcRes.Drc_v11.lpdetail_ratio;
    isp_cfg.others.drc_cfg.weicur_pix      = adrc_data.DrcProcRes.Drc_v11.weicur_pix;
    isp_cfg.others.drc_cfg.weipre_frame    = adrc_data.DrcProcRes.Drc_v11.weipre_frame;
    isp_cfg.others.drc_cfg.bilat_wt_off    = adrc_data.DrcProcRes.Drc_v11.bilat_wt_off;
    isp_cfg.others.drc_cfg.force_sgm_inv0  = adrc_data.DrcProcRes.Drc_v11.force_sgm_inv0;
    isp_cfg.others.drc_cfg.motion_scl      = adrc_data.DrcProcRes.Drc_v11.motion_scl;
    isp_cfg.others.drc_cfg.edge_scl        = adrc_data.DrcProcRes.Drc_v11.edge_scl;
    isp_cfg.others.drc_cfg.space_sgm_inv1  = adrc_data.DrcProcRes.Drc_v11.space_sgm_inv1;
    isp_cfg.others.drc_cfg.space_sgm_inv0  = adrc_data.DrcProcRes.Drc_v11.space_sgm_inv0;
    isp_cfg.others.drc_cfg.range_sgm_inv1  = adrc_data.DrcProcRes.Drc_v11.range_sgm_inv1;
    isp_cfg.others.drc_cfg.range_sgm_inv0  = adrc_data.DrcProcRes.Drc_v11.range_sgm_inv0;
    isp_cfg.others.drc_cfg.weig_maxl       = adrc_data.DrcProcRes.Drc_v11.weig_maxl;
    isp_cfg.others.drc_cfg.weig_bilat      = adrc_data.DrcProcRes.Drc_v11.weig_bilat;
    isp_cfg.others.drc_cfg.enable_soft_thd = adrc_data.DrcProcRes.Drc_v11.enable_soft_thd;
    isp_cfg.others.drc_cfg.bilat_soft_thd  = adrc_data.DrcProcRes.Drc_v11.bilat_soft_thd;
    isp_cfg.others.drc_cfg.iir_weight      = adrc_data.DrcProcRes.Drc_v11.iir_weight;
    isp_cfg.others.drc_cfg.min_ogain       = adrc_data.DrcProcRes.Drc_v11.min_ogain;
    isp_cfg.others.drc_cfg.wr_cycle        = adrc_data.DrcProcRes.Drc_v11.wr_cycle;

    for(int i = 0; i < 17; i++) {
        isp_cfg.others.drc_cfg.gain_y[i]    = adrc_data.DrcProcRes.Drc_v11.gain_y[i];
        isp_cfg.others.drc_cfg.compres_y[i] = adrc_data.DrcProcRes.Drc_v11.compres_y[i];
        isp_cfg.others.drc_cfg.scale_y[i]   = adrc_data.DrcProcRes.Drc_v11.scale_y[i];
    }

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

#if RKAIQ_HAVE_AF_V30 || RKAIQ_ONLY_AF_STATS_V30
void Isp3xParams::convertAiqAfToIsp3xParams(struct isp3x_isp_params_cfg& isp_cfg,
        const rk_aiq_isp_af_meas_v3x_t& af_data, bool af_cfg_udpate)
{
    int i;

    if (!af_cfg_udpate)
        return;

    if (af_data.af_en)
        isp_cfg.module_ens |= ISP2X_MODULE_RAWAF;
    isp_cfg.module_en_update |= ISP2X_MODULE_RAWAF;
    isp_cfg.module_cfg_update |= ISP2X_MODULE_RAWAF;

    isp_cfg.meas.rawaf.rawaf_sel = af_data.rawaf_sel;
    isp_cfg.meas.rawaf.gamma_en = af_data.gamma_en;
    isp_cfg.meas.rawaf.gaus_en = af_data.gaus_en;
    isp_cfg.meas.rawaf.v1_fir_sel = af_data.v1_fir_sel;
    isp_cfg.meas.rawaf.hiir_en = af_data.hiir_en;
    isp_cfg.meas.rawaf.viir_en = af_data.viir_en;
    isp_cfg.meas.rawaf.v1_fv_mode = af_data.v1_fv_outmode;
    isp_cfg.meas.rawaf.v2_fv_mode = af_data.v2_fv_outmode;
    isp_cfg.meas.rawaf.h1_fv_mode = af_data.h1_fv_outmode;
    isp_cfg.meas.rawaf.h2_fv_mode = af_data.h2_fv_outmode;
    isp_cfg.meas.rawaf.ldg_en = af_data.ldg_en;
    isp_cfg.meas.rawaf.accu_8bit_mode = af_data.accu_8bit_mode;
    if (af_data.af_en)
        isp_cfg.meas.rawaf.ae_mode = af_data.ae_mode;
    else
        isp_cfg.meas.rawaf.ae_mode = 0;
    isp_cfg.meas.rawaf.y_mode = af_data.y_mode;

    memcpy(isp_cfg.meas.rawaf.line_en,
           af_data.line_en, ISP2X_RAWAF_LINE_NUM * sizeof(unsigned char));
    memcpy(isp_cfg.meas.rawaf.line_num,
           af_data.line_num, ISP2X_RAWAF_LINE_NUM * sizeof(unsigned char));

    isp_cfg.meas.rawaf.num_afm_win = af_data.window_num;
    isp_cfg.meas.rawaf.win[0].h_offs = af_data.wina_h_offs;
    isp_cfg.meas.rawaf.win[0].v_offs = af_data.wina_v_offs;
    isp_cfg.meas.rawaf.win[0].h_size = af_data.wina_h_size;
    isp_cfg.meas.rawaf.win[0].v_size = af_data.wina_v_size;
    isp_cfg.meas.rawaf.win[1].h_offs = af_data.winb_h_offs;
    isp_cfg.meas.rawaf.win[1].v_offs = af_data.winb_v_offs;
    isp_cfg.meas.rawaf.win[1].h_size = af_data.winb_h_size;
    isp_cfg.meas.rawaf.win[1].v_size = af_data.winb_v_size;

    memcpy(isp_cfg.meas.rawaf.gamma_y,
           af_data.gamma_y, ISP3X_RAWAF_GAMMA_NUM * sizeof(unsigned short));

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

    for (i = 0; i < ISP3X_RAWAF_CURVE_NUM; i++) {
        isp_cfg.meas.rawaf.curve_h[i].ldg_lumth = af_data.h_ldg_lumth[i];
        isp_cfg.meas.rawaf.curve_h[i].ldg_gain = af_data.h_ldg_gain[i];
        isp_cfg.meas.rawaf.curve_h[i].ldg_gslp = af_data.h_ldg_gslp[i];
        isp_cfg.meas.rawaf.curve_v[i].ldg_lumth = af_data.v_ldg_lumth[i];
        isp_cfg.meas.rawaf.curve_v[i].ldg_gain = af_data.v_ldg_gain[i];
        isp_cfg.meas.rawaf.curve_v[i].ldg_gslp = af_data.v_ldg_gslp[i];
    }

    for (i = 0; i < ISP3X_RAWAF_HIIR_COE_NUM; i++) {
        isp_cfg.meas.rawaf.h1iir1_coe[i] = af_data.h1_iir1_coe[i];
        isp_cfg.meas.rawaf.h1iir2_coe[i] = af_data.h1_iir2_coe[i];
        isp_cfg.meas.rawaf.h2iir1_coe[i] = af_data.h2_iir1_coe[i];
        isp_cfg.meas.rawaf.h2iir2_coe[i] = af_data.h2_iir2_coe[i];
    }
    for (i = 0; i < ISP3X_RAWAF_V1IIR_COE_NUM; i++) {
        isp_cfg.meas.rawaf.v1iir_coe[i] = af_data.v1_iir_coe[i];
    }
    for (i = 0; i < ISP3X_RAWAF_V2IIR_COE_NUM; i++) {
        isp_cfg.meas.rawaf.v2iir_coe[i] = af_data.v2_iir_coe[i];
    }
    for (i = 0; i < ISP3X_RAWAF_VFIR_COE_NUM; i++) {
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
}
#endif
#if RKAIQ_HAVE_MERGE_V11
void Isp3xParams::convertAiqMergeToIsp3xParams(struct isp3x_isp_params_cfg& isp_cfg,
        const rk_aiq_isp_merge_v3x_t& amerge_data)
{
    isp_cfg.module_en_update |= 1LL << RK_ISP2X_HDRMGE_ID;
    isp_cfg.module_ens |= 1LL << RK_ISP2X_HDRMGE_ID;
    isp_cfg.module_cfg_update |= 1LL << RK_ISP2X_HDRMGE_ID;

    isp_cfg.others.hdrmge_cfg.mode         = amerge_data.Merge_v11.sw_hdrmge_mode;
    isp_cfg.others.hdrmge_cfg.gain0_inv    = amerge_data.Merge_v11.sw_hdrmge_gain0_inv;
    isp_cfg.others.hdrmge_cfg.gain0        = amerge_data.Merge_v11.sw_hdrmge_gain0;
    isp_cfg.others.hdrmge_cfg.gain1_inv    = amerge_data.Merge_v11.sw_hdrmge_gain1_inv;
    isp_cfg.others.hdrmge_cfg.gain1        = amerge_data.Merge_v11.sw_hdrmge_gain1;
    isp_cfg.others.hdrmge_cfg.gain2        = amerge_data.Merge_v11.sw_hdrmge_gain2;
    isp_cfg.others.hdrmge_cfg.lm_dif_0p15  = amerge_data.Merge_v11.sw_hdrmge_lm_dif_0p15;
    isp_cfg.others.hdrmge_cfg.lm_dif_0p9   = amerge_data.Merge_v11.sw_hdrmge_lm_dif_0p9;
    isp_cfg.others.hdrmge_cfg.ms_diff_0p15 = amerge_data.Merge_v11.sw_hdrmge_ms_dif_0p15;
    isp_cfg.others.hdrmge_cfg.ms_dif_0p8   = amerge_data.Merge_v11.sw_hdrmge_ms_dif_0p8;
    for(int i = 0; i < ISP3X_HDRMGE_L_CURVE_NUM; i++)
    {
        isp_cfg.others.hdrmge_cfg.curve.curve_0[i] = amerge_data.Merge_v11.sw_hdrmge_l0_y[i];
        isp_cfg.others.hdrmge_cfg.curve.curve_1[i] = amerge_data.Merge_v11.sw_hdrmge_l1_y[i];
    }
    for(int i = 0; i < ISP3X_HDRMGE_E_CURVE_NUM; i++)
        isp_cfg.others.hdrmge_cfg.e_y[i] = amerge_data.Merge_v11.sw_hdrmge_e_y[i];

    //isp30 add
    isp_cfg.others.hdrmge_cfg.s_base  = amerge_data.Merge_v11.sw_hdrmge_s_base;
    isp_cfg.others.hdrmge_cfg.ms_thd0 = amerge_data.Merge_v11.sw_hdrmge_ms_thd0;
    isp_cfg.others.hdrmge_cfg.ms_thd1 = amerge_data.Merge_v11.sw_hdrmge_ms_thd1;
    isp_cfg.others.hdrmge_cfg.ms_scl  = amerge_data.Merge_v11.sw_hdrmge_ms_scl;
    isp_cfg.others.hdrmge_cfg.lm_thd0 = amerge_data.Merge_v11.sw_hdrmge_lm_thd0;
    isp_cfg.others.hdrmge_cfg.lm_thd1 = amerge_data.Merge_v11.sw_hdrmge_lm_thd1;
    isp_cfg.others.hdrmge_cfg.lm_scl  = amerge_data.Merge_v11.sw_hdrmge_lm_scl;

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
    for(int i = 0 ; i < 17; i++)
    {
        LOGE_CAMHW_SUBM(ISP20PARAM_SUBM, "%d: curve_0[%d] %d", __LINE__, i, isp_cfg.others.hdrmge_cfg.curve.curve_0[i]);
        LOGE_CAMHW_SUBM(ISP20PARAM_SUBM, "%d: curve_1[%d] %d", __LINE__, i, isp_cfg.others.hdrmge_cfg.curve.curve_1[i]);
        LOGE_CAMHW_SUBM(ISP20PARAM_SUBM, "%d: e_y[%d] %d", __LINE__, i, isp_cfg.others.hdrmge_cfg.e_y[i]);
    }

#endif
}
#endif

#if RKAIQ_HAVE_GAMMA_V11
template <class T>
void Isp3xParams::convertAiqAgammaToIsp3xParams(T& isp_cfg,
        const rk_aiq_isp_goc_v3x_t& gamma_out_cfg) {
    if (gamma_out_cfg.Gamma_v11.gamma_en) {
        isp_cfg.module_ens |= ISP2X_MODULE_GOC;
        isp_cfg.module_en_update |= ISP2X_MODULE_GOC;
        isp_cfg.module_cfg_update |= ISP2X_MODULE_GOC;
    } else {
        isp_cfg.module_ens &= ~ISP2X_MODULE_GOC;
        isp_cfg.module_en_update |= ISP2X_MODULE_GOC;
        return;
    }

    struct isp3x_gammaout_cfg* cfg = &isp_cfg.others.gammaout_cfg;
    cfg->offset                    = gamma_out_cfg.Gamma_v11.offset;
    cfg->finalx4_dense_en          = gamma_out_cfg.Gamma_v11.EnableDot49 ? 1 : 0;
    cfg->equ_segm                  = gamma_out_cfg.Gamma_v11.equ_segm;
    for (int i = 0; i < 49; i++) cfg->gamma_y[i] = gamma_out_cfg.Gamma_v11.gamma_y[i];
#if 0
    printf("cfg->offset %d\n", cfg->offset);
    for (int i = 0; i < 6; i++)
        printf("cfg->gamma_y[%d] %d\n", i, cfg->gamma_y[i]);
#endif
#if 0
    LOGE_CAMHW_SUBM(ISP20PARAM_SUBM, "%s:(%d) gamma en:%d, finalx4_dense_en:%d offset:%d, equ_segm:%d\n", __FUNCTION__, __LINE__, gamma_out_cfg.Gamma_v11.gamma_en,
                    cfg->finalx4_dense_en, cfg->offset, cfg->offset);
    LOGE_CAMHW_SUBM(ISP20PARAM_SUBM, "Gamma_Y:%d %d %d %d %d %d %d %d\n", cfg->gamma_y[0], cfg->gamma_y[1],
                    cfg->gamma_y[2], cfg->gamma_y[3], cfg->gamma_y[4], cfg->gamma_y[5], cfg->gamma_y[6], cfg->gamma_y[7]);
#endif
}
#endif

#if RKAIQ_HAVE_CAC_V03 || RKAIQ_HAVE_CAC_V10
void Isp3xParams::convertAiqCacToIsp3xParams(struct isp3x_isp_params_cfg& isp_cfg,
        struct isp3x_isp_params_cfg& isp_cfg_right,
        const rk_aiq_isp_cac_v3x_t& cac_cfg, bool is_multi_isp)
{
    LOGD_ACAC("convert CAC params enable %d", cac_cfg.enable);
    if(cac_cfg.enable) {
        isp_cfg.module_en_update |= ISP3X_MODULE_CAC;
        isp_cfg.module_ens |= ISP3X_MODULE_CAC;
        isp_cfg.module_cfg_update |= ISP3X_MODULE_CAC;
    }
    else {
        isp_cfg.module_en_update |= ISP3X_MODULE_CAC;
        isp_cfg.module_ens &= ~(ISP3X_MODULE_CAC);
        isp_cfg.module_cfg_update &= ~(ISP3X_MODULE_CAC);
    }

    struct isp3x_cac_cfg* cfg = &isp_cfg.others.cac_cfg;
    struct isp3x_cac_cfg* cfg_right = nullptr;
    memcpy(cfg, &cac_cfg.cfg[0], sizeof(*cfg));
    if (is_multi_isp) {
        cfg_right = &isp_cfg_right.others.cac_cfg;
        memcpy(cfg_right, &cac_cfg.cfg[1], sizeof(*cfg));
    }

#if 1
    LOGD_ACAC("driver current enable: %d", cac_cfg.enable);
    LOGD_ACAC("driver center en: %d", cfg->center_en);
    LOGD_ACAC("driver center x: %u", cfg->center_width);
    LOGD_ACAC("driver center y: %u", cfg->center_height);
    LOGD_ACAC("driver psf shift bits: %u", cfg->psf_sft_bit);
    LOGD_ACAC("driver psf cfg num: %u", cfg->cfg_num);
    LOGD_ACAC("driver psf buf fd: %d", cfg->buf_fd);
    if (is_multi_isp) {
        LOGD_ACAC("driver psf buf fd right : %d", cfg_right->buf_fd);
    }
    LOGD_ACAC("driver psf hwsize: %u", cfg->hsize);
    LOGD_ACAC("driver psf size: %u", cfg->vsize);
    for (int i = 0; i < RKCAC_STRENGTH_TABLE_LEN; i++) {
        LOGD_ACAC("driver strength %d: %u", i, cfg->strength[i]);
    }
#endif
}
#endif

#if defined(ISP_HW_V30)
void Isp3xParams::convertAiqLdchToIsp3xParams(struct isp3x_isp_params_cfg& isp_cfg,
        struct isp3x_isp_params_cfg& isp_cfg_right,
        const rk_aiq_isp_ldch_t& ldch_cfg, bool is_multi_isp)
{
    struct isp2x_ldch_cfg  *pLdchCfg = &isp_cfg.others.ldch_cfg;

    // TODO: add update flag for ldch
    if (ldch_cfg.sw_ldch_en) {
        isp_cfg.module_ens |= ISP2X_MODULE_LDCH;
        isp_cfg.module_en_update |= ISP2X_MODULE_LDCH;
        isp_cfg.module_cfg_update |= ISP2X_MODULE_LDCH;

        pLdchCfg->hsize = ldch_cfg.lut_h_size;
        pLdchCfg->vsize = ldch_cfg.lut_v_size;
        pLdchCfg->buf_fd = ldch_cfg.lut_mapxy_buf_fd[0];

        LOGD_CAMHW_SUBM(ISP20PARAM_SUBM, "enable ldch h/v size: %dx%d, buf_fd: %d",
                        pLdchCfg->hsize, pLdchCfg->vsize, pLdchCfg->buf_fd);

        if (is_multi_isp) {
            struct isp2x_ldch_cfg *cfg_right = &isp_cfg.others.ldch_cfg;
            cfg_right = &isp_cfg_right.others.ldch_cfg;
            memcpy(cfg_right, pLdchCfg, sizeof(*cfg_right));
            cfg_right->buf_fd = ldch_cfg.lut_mapxy_buf_fd[1];

            LOGD_CAMHW_SUBM(ISP20PARAM_SUBM, "multi isp: ldch right isp buf fd: %d", cfg_right->buf_fd);
        }
    } else {
        isp_cfg.module_ens &= ~ISP2X_MODULE_LDCH;
        isp_cfg.module_en_update |= ISP2X_MODULE_LDCH;
    }
}
#endif

#if RKAIQ_HAVE_DEHAZE_V11_DUO
void Isp3xParams::convertAiqAdehazeToIsp3xParams(struct isp3x_isp_params_cfg& isp_cfg,
        const rk_aiq_isp_dehaze_v3x_t& dhaze)
{
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

    struct isp3x_dhaz_cfg *  cfg = &isp_cfg.others.dhaz_cfg;

    cfg->round_en        = dhaze.ProcResV11duo.round_en;
    cfg->soft_wr_en      = dhaze.ProcResV11duo.soft_wr_en;
    cfg->enhance_en      = dhaze.ProcResV11duo.enhance_en;
    cfg->air_lc_en       = dhaze.ProcResV11duo.air_lc_en;
    cfg->hpara_en        = dhaze.ProcResV11duo.hpara_en;
    cfg->hist_en         = dhaze.ProcResV11duo.hist_en;
    cfg->dc_en           = dhaze.ProcResV11duo.dc_en;
    cfg->yblk_th         = dhaze.ProcResV11duo.yblk_th;
    cfg->yhist_th        = dhaze.ProcResV11duo.yhist_th;
    cfg->dc_max_th       = dhaze.ProcResV11duo.dc_max_th;
    cfg->dc_min_th       = dhaze.ProcResV11duo.dc_min_th;
    cfg->wt_max          = dhaze.ProcResV11duo.wt_max;
    cfg->bright_max      = dhaze.ProcResV11duo.bright_max;
    cfg->bright_min      = dhaze.ProcResV11duo.bright_min;
    cfg->tmax_base       = dhaze.ProcResV11duo.tmax_base;
    cfg->dark_th         = dhaze.ProcResV11duo.dark_th;
    cfg->air_max         = dhaze.ProcResV11duo.air_max;
    cfg->air_min         = dhaze.ProcResV11duo.air_min;
    cfg->tmax_max        = dhaze.ProcResV11duo.tmax_max;
    cfg->tmax_off        = dhaze.ProcResV11duo.tmax_off;
    cfg->hist_k          = dhaze.ProcResV11duo.hist_k;
    cfg->hist_th_off     = dhaze.ProcResV11duo.hist_th_off;
    cfg->hist_min        = dhaze.ProcResV11duo.hist_min;
    cfg->hist_gratio     = dhaze.ProcResV11duo.hist_gratio;
    cfg->hist_scale      = dhaze.ProcResV11duo.hist_scale;
    cfg->enhance_value   = dhaze.ProcResV11duo.enhance_value;
    cfg->enhance_chroma  = dhaze.ProcResV11duo.enhance_chroma;
    cfg->iir_wt_sigma    = dhaze.ProcResV11duo.iir_wt_sigma;
    cfg->iir_sigma       = dhaze.ProcResV11duo.iir_sigma;
    cfg->stab_fnum       = dhaze.ProcResV11duo.stab_fnum;
    cfg->iir_tmax_sigma  = dhaze.ProcResV11duo.iir_tmax_sigma;
    cfg->iir_air_sigma   = dhaze.ProcResV11duo.iir_air_sigma;
    cfg->iir_pre_wet     = dhaze.ProcResV11duo.iir_pre_wet;
    cfg->cfg_wt          = dhaze.ProcResV11duo.cfg_wt;
    cfg->cfg_air         = dhaze.ProcResV11duo.cfg_air;
    cfg->cfg_alpha       = dhaze.ProcResV11duo.cfg_alpha;
    cfg->cfg_gratio      = dhaze.ProcResV11duo.cfg_gratio;
    cfg->cfg_tmax        = dhaze.ProcResV11duo.cfg_tmax;
    cfg->range_sima      = dhaze.ProcResV11duo.range_sima;
    cfg->space_sigma_cur = dhaze.ProcResV11duo.space_sigma_cur;
    cfg->space_sigma_pre = dhaze.ProcResV11duo.space_sigma_pre;
    cfg->dc_weitcur      = dhaze.ProcResV11duo.dc_weitcur;
    cfg->bf_weight       = dhaze.ProcResV11duo.bf_weight;
    cfg->gaus_h0         = dhaze.ProcResV11duo.gaus_h0;
    cfg->gaus_h1         = dhaze.ProcResV11duo.gaus_h1;
    cfg->gaus_h2         = dhaze.ProcResV11duo.gaus_h2;
    cfg->adp_wt_wr       = dhaze.ProcResV11duo.adp_wt_wr;
    cfg->adp_air_wr      = dhaze.ProcResV11duo.adp_air_wr;
    cfg->adp_tmax_wr     = dhaze.ProcResV11duo.adp_tmax_wr;
    cfg->adp_gratio_wr   = dhaze.ProcResV11duo.adp_gratio_wr;

    for(int i = 0; i < ISP3X_DHAZ_SIGMA_IDX_NUM; i++)
        cfg->sigma_idx[i] = dhaze.ProcResV11duo.sigma_idx[i];

    for(int i = 0; i < ISP3X_DHAZ_ENH_CURVE_NUM; i++)
        cfg->enh_curve[i] = dhaze.ProcResV11duo.enh_curve[i];

    for(int i = 0; i < ISP3X_DHAZ_SIGMA_LUT_NUM; i++)
        cfg->sigma_lut[i] = dhaze.ProcResV11duo.sigma_lut[i];

    for(int i = 0; i < ISP3X_DHAZ_HIST_WR_NUM; i++)
        cfg->hist_wr[i] = dhaze.ProcResV11duo.hist_wr[i];

    LOG1_ADEHAZE(
        "%s: round_en:%d soft_wr_en:%d enhance_en:%d air_lc_en:%d hpara_en:%d "
        "hist_en:%d "
        "dc_en:%d\n",
        __func__, cfg->round_en, cfg->soft_wr_en, cfg->enhance_en, cfg->air_lc_en, cfg->hpara_en,
        cfg->hist_en, cfg->dc_en);
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
}
#endif

bool Isp3xParams::convert3aResultsToIspCfg(SmartPtr<cam3aResult> &result,
        void* isp_cfg_p, bool is_multi_isp)
{
    struct isp3x_isp_params_cfg& isp_cfg       = *(struct isp3x_isp_params_cfg*)isp_cfg_p;
    struct isp3x_isp_params_cfg& isp_cfg_right = *((struct isp3x_isp_params_cfg*)isp_cfg_p + 1);

    if (result.ptr() == NULL) {
        LOGE_CAMHW_SUBM(ISP20PARAM_SUBM, "3A result empty");
        return false;
    }

    int32_t type = result->getType();
    // LOGE_CAMHW_SUBM(ISP20PARAM_SUBM, "%s, module (0x%x) convert params!\n", __FUNCTION__, type);
    switch (type)
    {
    // followings are specific for isp3x
    case RESULT_TYPE_EXPOSURE_PARAM:
    {
        RkAiqSensorExpParamsProxy* expParams =
            result.get_cast_ptr<RkAiqSensorExpParamsProxy>();
        if (expParams)
            convertAiqExpIspDgainToIspParams(isp_cfg_p,
                                             expParams->data()->aecExpInfo);
    }
    break;
    case RESULT_TYPE_AWBGAIN_PARAM:
    {
#if RKAIQ_HAVE_AWB_V21
        RkAiqIspAwbGainParamsProxy* awb_gain = result.get_cast_ptr<RkAiqIspAwbGainParamsProxy>();
        if (awb_gain) {
            if(mBlcResult) {
                RkAiqIspBlcParamsProxy* blc = dynamic_cast<RkAiqIspBlcParamsProxy*>(mBlcResult);
                convertAiqAwbGainToIsp21Params(isp_cfg,
                                               awb_gain->data()->result, &blc->data()->result, true);
            } else {
                convertAiqAwbGainToIsp21Params(isp_cfg,
                                               awb_gain->data()->result, nullptr, true);
            }

        } else
            LOGE("don't get awb_gain params, convert awbgain params failed!");
#endif
    }
    break;
    case RESULT_TYPE_AWB_PARAM:
    {
#if RKAIQ_HAVE_AWB_V21
        mAwbParams = result.ptr();
        RkAiqIspAwbParamsProxy* params = result.get_cast_ptr<RkAiqIspAwbParamsProxy>();
        if (params)
            convertAiqAwbToIsp3xParams(isp_cfg, params->data()->result, true);
#endif
    }
    break;
    case RESULT_TYPE_DRC_PARAM:
    {
#if RKAIQ_HAVE_DRC_V11
        RkAiqIspDrcParamsProxy* params = result.get_cast_ptr<RkAiqIspDrcParamsProxy>();
        if (params)
            convertAiqDrcToIsp3xParams(isp_cfg, params->data()->result);
#endif
    }
    break;
    case RESULT_TYPE_TNR_PARAM:
    {
#if RKAIQ_HAVE_BAYERTNR_V2
        RkAiqIspTnrParamsProxy* params = result.get_cast_ptr<RkAiqIspTnrParamsProxy>();
        if (params)
            convertAiqTnrToIsp3xParams(isp_cfg, params->data()->result);
#endif
    }
    break;
    case RESULT_TYPE_RAWNR_PARAM:
    {
#if RKAIQ_HAVE_BAYER2DNR_V2
        RkAiqIspBaynrParamsProxy* params = result.get_cast_ptr<RkAiqIspBaynrParamsProxy>();
        if (params)
            convertAiqRawnrToIsp3xParams(isp_cfg, params->data()->result);
#endif
    }
    break;
    case RESULT_TYPE_YNR_PARAM:
    {
#if RKAIQ_HAVE_YNR_V3
        RkAiqIspYnrParamsProxy* params = result.get_cast_ptr<RkAiqIspYnrParamsProxy>();
        if (params)
            convertAiqYnrToIsp3xParams(isp_cfg, params->data()->result);
#endif
    }
    break;
    case RESULT_TYPE_UVNR_PARAM:
    {
#if RKAIQ_HAVE_CNR_V2
        RkAiqIspCnrParamsProxy* params = result.get_cast_ptr<RkAiqIspCnrParamsProxy>();
        if (params)
            convertAiqUvnrToIsp3xParams(isp_cfg, params->data()->result);
#endif
    }
    break;
    case RESULT_TYPE_SHARPEN_PARAM:
    {
#if RKAIQ_HAVE_SHARP_V4
        RkAiqIspSharpenParamsProxy* params = result.get_cast_ptr<RkAiqIspSharpenParamsProxy>();
        if (params)
            convertAiqSharpenToIsp3xParams(isp_cfg, params->data()->result);
#endif
    }
    break;
    case RESULT_TYPE_GAIN_PARAM:
    {
#if RKAIQ_HAVE_GAIN_V2
        RkAiqIspGainParamsProxy* params = result.get_cast_ptr<RkAiqIspGainParamsProxy>();
        if (params)
            convertAiqGainToIsp3xParams(isp_cfg, params->data()->result);
#endif
    }
    break;
    case RESULT_TYPE_DEHAZE_PARAM:
    {
#if RKAIQ_HAVE_DEHAZE_V11_DUO
        RkAiqIspDehazeParamsProxy* params = result.get_cast_ptr<RkAiqIspDehazeParamsProxy>();
        if (params)
            convertAiqAdehazeToIsp3xParams(isp_cfg, params->data()->result);
#endif
    }
    break;
    case RESULT_TYPE_GIC_PARAM:
    {
        RkAiqIspGicParamsProxy* params = result.get_cast_ptr<RkAiqIspGicParamsProxy>();
        if (params)
            convertAiqAgicToIsp21Params(isp_cfg, params->data()->result);
    }
    break;
    case RESULT_TYPE_AF_PARAM:
    {
#if RKAIQ_HAVE_AF_V30 || RKAIQ_ONLY_AF_STATS_V30
        RkAiqIspAfParamsProxy* params = result.get_cast_ptr<RkAiqIspAfParamsProxy>();
        if (params)
            convertAiqAfToIsp3xParams(isp_cfg, params->data()->result, true);
#endif
    }
    break;
    case RESULT_TYPE_MERGE_PARAM:
    {
#if RKAIQ_HAVE_MERGE_V11
        RkAiqIspMergeParamsProxy* params = result.get_cast_ptr<RkAiqIspMergeParamsProxy>();
        if (params) {
            convertAiqMergeToIsp3xParams(isp_cfg, params->data()->result);
        }
#endif
    }
    break;
    case RESULT_TYPE_AGAMMA_PARAM:
    {
#if RKAIQ_HAVE_GAMMA_V11
#ifndef USE_NEWSTRUCT
        RkAiqIspAgammaParamsProxy* params = result.get_cast_ptr<RkAiqIspAgammaParamsProxy>();
        if (params)
            convertAiqAgammaToIsp3xParams(isp_cfg, params->data()->result);
#endif
#endif
    }
    break;
    case RESULT_TYPE_CAC_PARAM:
    {
#if RKAIQ_HAVE_CAC_V03 || RKAIQ_HAVE_CAC_V10
        RkAiqIspCacParamsProxy* params = result.get_cast_ptr<RkAiqIspCacParamsProxy>();
        if (params) {
            convertAiqCacToIsp3xParams(isp_cfg, isp_cfg_right, params->data()->result, is_multi_isp);
        }
#endif
    }
    break;
    case RESULT_TYPE_LSC_PARAM:
    {
#if RKAIQ_HAVE_LSC_V2
        RkAiqIspLscParamsProxy* params = result.get_cast_ptr<RkAiqIspLscParamsProxy>();
        if (params)
            convertAiqLscToIsp20Params(isp_cfg, params->data()->result);
#endif
    }
    break;
    // followings are the same as isp21
    case RESULT_TYPE_BLC_PARAM:
    {
#if RKAIQ_HAVE_BLC_V1
        RkAiqIspBlcParamsProxy* params = result.get_cast_ptr<RkAiqIspBlcParamsProxy>();
        if (params)
            convertAiqBlcToIsp21Params(isp_cfg, params->data()->result);
#endif
    }
    break;
    case RESULT_TYPE_CCM_PARAM:
    {
#if RKAIQ_HAVE_CCM_V1
        RkAiqIspCcmParamsProxy* params = result.get_cast_ptr<RkAiqIspCcmParamsProxy>();
        if (params)
            convertAiqCcmToIsp21Params(isp_cfg, params->data()->result);
#endif
    }
    break;
    // followings are the same as isp20
    case RESULT_TYPE_AEC_PARAM:
    {
#if defined(ISP_HW_V30)
        RkAiqIspAecParamsProxy* params = result.get_cast_ptr<RkAiqIspAecParamsProxy>();
        if (params) {
            convertAiqAeToIsp20Params(isp_cfg, params->data()->result);
        }
#endif
    }
    break;
    case RESULT_TYPE_HIST_PARAM:
    {
#if defined(ISP_HW_V30)
        RkAiqIspHistParamsProxy* params = result.get_cast_ptr<RkAiqIspHistParamsProxy>();
        if (params)
            convertAiqHistToIsp20Params(isp_cfg, params->data()->result);
#endif
    }
    break;
    case RESULT_TYPE_LUT3D_PARAM:
    {
#if RKAIQ_HAVE_3DLUT_V1
        RkAiqIspLut3dParamsProxy* params = result.get_cast_ptr<RkAiqIspLut3dParamsProxy>();
        if (params)
            convertAiqA3dlutToIsp20Params(isp_cfg, params->data()->result);
#endif
    }
    break;
    case RESULT_TYPE_DPCC_PARAM:
#if RKAIQ_HAVE_DPCC_V1
#ifndef USE_NEWSTRUCT
    {
        RkAiqIspDpccParamsProxy* params = result.get_cast_ptr<RkAiqIspDpccParamsProxy>();
        if (params)
            convertAiqDpccToIsp20Params(isp_cfg, params->data()->result);
    }
#endif
#endif
    break;
    case RESULT_TYPE_DEBAYER_PARAM:
    {
#if RKAIQ_HAVE_DEBAYER_V1
        RkAiqIspDebayerParamsProxy* params = result.get_cast_ptr<RkAiqIspDebayerParamsProxy>();
        if (params)
            convertAiqAdemosaicToIsp20Params(isp_cfg, params->data()->result);
#endif
    }
    break;
    case RESULT_TYPE_LDCH_PARAM:
#if defined(ISP_HW_V30)
    {
        RkAiqIspLdchParamsProxy* params = result.get_cast_ptr<RkAiqIspLdchParamsProxy>();
        if (params && params->data()->is_update) {
            convertAiqLdchToIsp3xParams(isp_cfg, isp_cfg_right, params->data()->result, is_multi_isp);
        }
    }
#endif
    break;
    case RESULT_TYPE_ADEGAMMA_PARAM:
    {
        RkAiqIspAdegammaParamsProxy* params = result.get_cast_ptr<RkAiqIspAdegammaParamsProxy>();
        if (params)
            convertAiqAdegammaToIsp20Params(isp_cfg, params->data()->result);
    }
    break;
    case RESULT_TYPE_WDR_PARAM:
#if 0
    {
        SmartPtr<RkAiqIspWdrParamsProxy> params = result.dynamic_cast_ptr<RkAiqIspWdrParamsProxy>();
        if (params.ptr())
            convertAiqWdrToIsp20Params(isp_cfg, params->data()->result);
    }
#endif
    break;
    case RESULT_TYPE_CSM_PARAM:
    {
        RkAiqIspCsmParamsProxy* params = result.get_cast_ptr<RkAiqIspCsmParamsProxy>();
        if (params)
            convertAiqCsmToIsp21Params(isp_cfg, params->data()->result);
    }
    break;
    case RESULT_TYPE_CGC_PARAM:
    {
#if RKAIQ_HAVE_CGC_V1
        RkAiqIspCgcParamsProxy* params = result.get_cast_ptr<RkAiqIspCgcParamsProxy>();
        if (params)
            convertAiqCgcToIsp21Params(isp_cfg, params->data()->result);
#endif
    }
    break;
    case RESULT_TYPE_CONV422_PARAM:
        break;
    case RESULT_TYPE_YUVCONV_PARAM:
        break;
    case RESULT_TYPE_CP_PARAM:
    {
#if RKAIQ_HAVE_ACP_V10
        RkAiqIspCpParamsProxy* params = result.get_cast_ptr<RkAiqIspCpParamsProxy>();
        if (params)
            convertAiqCpToIsp20Params(isp_cfg, params->data()->result);
#endif
    }
    break;
    case RESULT_TYPE_IE_PARAM:
    {
#if RKAIQ_HAVE_AIE_V10
        RkAiqIspIeParamsProxy* params = result.get_cast_ptr<RkAiqIspIeParamsProxy>();
        if (params)
            convertAiqIeToIsp20Params(isp_cfg, params->data()->result);
#endif
    }
    break;
    case RESULT_TYPE_FEC_PARAM:
        break;
    default:
        LOGE("unknown param type: 0x%x!", type);
        return false;
    }

    return true;
}

}; //namspace RkCam
