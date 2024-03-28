/*
 * Copyright (c) 2019-2022 Rockchip Eletronics Co., Ltd.
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
#include "Isp21Params.h"

#include <cstdint>
#include <type_traits>

#include "common/rk_aiq_types.h"

namespace RkCam {

#define ISP2X_WBGAIN_FIXSCALE_BIT  8//check
#define ISP2X_WBGAIN_INTSCALE_BIT  6//check

#define ISP2X_BLC_BIT_MAX 12

template <typename T>
struct ConvertBlcHelper {
    template < typename U                          = T,
               typename std::enable_if < (std::is_same<U, struct isp2x_isp_params_cfg>::value ||
                                          std::is_same<U, struct isp21_isp_params_cfg>::value ||
                                          std::is_same<U, struct isp3x_isp_params_cfg>::value),
                                         bool >::type = true >
    void FixFromAwbGain(U& isp_cfg, rk_aiq_isp_blc_v21_t& blc) {
        int tmp = 0;
        struct isp21_awb_gain_cfg* awb_cfg = &isp_cfg.others.awb_gain_cfg;
        uint16_t base_wb_gain              = 1 << ISP2X_WBGAIN_FIXSCALE_BIT;

        isp_cfg.others.bls_cfg.bls1_en = blc.v0.blc1_enable;
        tmp                            = blc.v0.blc1_r * awb_cfg->gain0_red / base_wb_gain;
        if (tmp > 0x1fff) tmp = 0x1fff;
        if (tmp < 0) tmp = 0;
        isp_cfg.others.bls_cfg.bls1_val.r = tmp;

        tmp = blc.v0.blc1_gr * awb_cfg->gain0_green_r / base_wb_gain;
        if (tmp > 0x1fff) tmp = 0x1fff;
        if (tmp < 0) tmp = 0;
        isp_cfg.others.bls_cfg.bls1_val.gr = tmp;

        tmp = blc.v0.blc1_gb * awb_cfg->gain0_green_b / base_wb_gain;
        if (tmp > 0x1fff) tmp = 0x1fff;
        if (tmp < 0) tmp = 0;
        isp_cfg.others.bls_cfg.bls1_val.gb = tmp;

        tmp = blc.v0.blc1_b * awb_cfg->gain0_blue / base_wb_gain;
        if (tmp > 0x1fff) tmp = 0x1fff;
        if (tmp < 0) tmp = 0;
        isp_cfg.others.bls_cfg.bls1_val.b = tmp;
    }

    template <typename U                          = T,
              typename std::enable_if<std::is_same<U, struct isp32_isp_params_cfg>::value,
                                      bool>::type = false>
    void FixFromAwbGain(U& isp_cfg, rk_aiq_isp_blc_v21_t& blc) {
        int tmp = 0;
        isp_cfg.others.bls_cfg.bls1_en = blc.v0.blc1_enable;
        tmp                            = blc.v0.blc1_r;
        if (tmp > 0x1fff) tmp = 0x1fff;
        if (tmp < 0) tmp = 0;
        isp_cfg.others.bls_cfg.bls1_val.r = tmp;

        tmp = blc.v0.blc1_gr;
        if (tmp > 0x1fff) tmp = 0x1fff;
        if (tmp < 0) tmp = 0;
        isp_cfg.others.bls_cfg.bls1_val.gr = tmp;

        tmp = blc.v0.blc1_gb;
        if (tmp > 0x1fff) tmp = 0x1fff;
        if (tmp < 0) tmp = 0;
        isp_cfg.others.bls_cfg.bls1_val.gb = tmp;

        tmp = blc.v0.blc1_b;
        if (tmp > 0x1fff) tmp = 0x1fff;
        if (tmp < 0) tmp = 0;
        isp_cfg.others.bls_cfg.bls1_val.b = tmp;
    }
};

#if RKAIQ_HAVE_AWB_V21
template<class T>
void Isp21Params::convertAiqAwbGainToIsp21Params(T& isp_cfg,
        const rk_aiq_wb_gain_t& awb_gain, const rk_aiq_isp_blc_v21_t *blc,
        bool awb_gain_update)
{

    if(awb_gain_update) {
        isp_cfg.module_ens |= 1LL << RK_ISP2X_AWB_GAIN_ID;
        isp_cfg.module_cfg_update |= 1LL << RK_ISP2X_AWB_GAIN_ID;
        isp_cfg.module_en_update |= 1LL << RK_ISP2X_AWB_GAIN_ID;
    } else {
        return;
    }

    struct isp21_awb_gain_cfg *  cfg = &isp_cfg.others.awb_gain_cfg;
    uint16_t max_wb_gain = (1 << (ISP2X_WBGAIN_FIXSCALE_BIT + 3)) - 1;
    rk_aiq_wb_gain_t awb_gain1 = awb_gain;
    if(blc != nullptr && blc->v0.enable) {
        awb_gain1.bgain *= (float)((1 << ISP2X_BLC_BIT_MAX) - 1) / ((1 << ISP2X_BLC_BIT_MAX) - 1 - blc->v0.blc_b);
        awb_gain1.gbgain *= (float)((1 << ISP2X_BLC_BIT_MAX) - 1) / ((1 << ISP2X_BLC_BIT_MAX) - 1 - blc->v0.blc_gb);
        awb_gain1.rgain *= (float)((1 << ISP2X_BLC_BIT_MAX) - 1) / ((1 << ISP2X_BLC_BIT_MAX) - 1 - blc->v0.blc_r);
        awb_gain1.grgain *= (float)((1 << ISP2X_BLC_BIT_MAX) - 1) / ((1 << ISP2X_BLC_BIT_MAX) - 1 - blc->v0.blc_gr);
    }

    uint16_t R = (uint16_t)(0.5 + awb_gain1.rgain * (1 << ISP2X_WBGAIN_FIXSCALE_BIT));
    uint16_t B = (uint16_t)(0.5 + awb_gain1.bgain * (1 << ISP2X_WBGAIN_FIXSCALE_BIT));
    uint16_t Gr = (uint16_t)(0.5 + awb_gain1.grgain * (1 << ISP2X_WBGAIN_FIXSCALE_BIT));
    uint16_t Gb = (uint16_t)(0.5 + awb_gain1.gbgain * (1 << ISP2X_WBGAIN_FIXSCALE_BIT));
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

#if defined(ISP_HW_V30) || defined(ISP_HW_V21)
    mLatestWbGainCfg = *cfg;
#endif
}
#endif

#if RKAIQ_HAVE_BLC_V1
template <class T>
void Isp21Params::convertAiqBlcToIsp21Params(T& isp_cfg, rk_aiq_isp_blc_v21_t& blc) {
    LOGD_ABLC( "%s:(%d) enter enable:%d\n", __FUNCTION__, __LINE__, blc.v0.enable);

    if(blc.v0.enable) {
        isp_cfg.module_ens |= ISP2X_MODULE_BLS;
    }
    isp_cfg.module_en_update |= ISP2X_MODULE_BLS;
    isp_cfg.module_cfg_update |= ISP2X_MODULE_BLS;

    isp_cfg.others.bls_cfg.enable_auto = 0;
    isp_cfg.others.bls_cfg.en_windows = 0;

    isp_cfg.others.bls_cfg.bls_window1.h_offs = 0;
    isp_cfg.others.bls_cfg.bls_window1.v_offs = 0;
    isp_cfg.others.bls_cfg.bls_window1.h_size = 0;
    isp_cfg.others.bls_cfg.bls_window1.v_size = 0;

    isp_cfg.others.bls_cfg.bls_window2.h_offs = 0;
    isp_cfg.others.bls_cfg.bls_window2.v_offs = 0;
    isp_cfg.others.bls_cfg.bls_window2.h_size = 0;
    isp_cfg.others.bls_cfg.bls_window2.v_size = 0;

    isp_cfg.others.bls_cfg.bls_samples = 0;

    isp_cfg.others.bls_cfg.fixed_val.r = blc.v0.blc_r;
    isp_cfg.others.bls_cfg.fixed_val.gr = blc.v0.blc_gr;
    isp_cfg.others.bls_cfg.fixed_val.gb = blc.v0.blc_gb;
    isp_cfg.others.bls_cfg.fixed_val.b = blc.v0.blc_b;

    //TODO bls1 params
    isp_cfg.others.bls_cfg.bls1_en = 0;

    ConvertBlcHelper<T> helper;
    helper.FixFromAwbGain(isp_cfg, blc);

    LOGD_ABLC("%s:(%d) exit \n", __FUNCTION__, __LINE__);
}
#endif

#if RKAIQ_HAVE_DEHAZE_V11
void
Isp21Params::convertAiqAdehazeToIsp21Params(struct isp21_isp_params_cfg& isp_cfg,
        const rk_aiq_isp_dehaze_v21_t& dhaze)
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

    struct isp21_dhaz_cfg *  cfg = &isp_cfg.others.dhaz_cfg;

    cfg->enhance_en      = dhaze.ProcResV11.enhance_en;
    cfg->air_lc_en       = dhaze.ProcResV11.air_lc_en;
    cfg->hpara_en        = dhaze.ProcResV11.hpara_en;
    cfg->hist_en         = dhaze.ProcResV11.hist_en;
    cfg->dc_en           = dhaze.ProcResV11.dc_en;
    cfg->yblk_th         = dhaze.ProcResV11.yblk_th;
    cfg->yhist_th        = dhaze.ProcResV11.yhist_th;
    cfg->dc_max_th       = dhaze.ProcResV11.dc_max_th;
    cfg->dc_min_th       = dhaze.ProcResV11.dc_min_th;
    cfg->wt_max          = dhaze.ProcResV11.wt_max;
    cfg->bright_max      = dhaze.ProcResV11.bright_max;
    cfg->bright_min      = dhaze.ProcResV11.bright_min;
    cfg->tmax_base       = dhaze.ProcResV11.tmax_base;
    cfg->dark_th         = dhaze.ProcResV11.dark_th;
    cfg->air_max         = dhaze.ProcResV11.air_max;
    cfg->air_min         = dhaze.ProcResV11.air_min;
    cfg->tmax_max        = dhaze.ProcResV11.tmax_max;
    cfg->tmax_off        = dhaze.ProcResV11.tmax_off;
    cfg->hist_k          = dhaze.ProcResV11.hist_k;
    cfg->hist_th_off     = dhaze.ProcResV11.hist_th_off;
    cfg->hist_min        = dhaze.ProcResV11.hist_min;
    cfg->hist_gratio     = dhaze.ProcResV11.hist_gratio;
    cfg->hist_scale      = dhaze.ProcResV11.hist_scale;
    cfg->enhance_value   = dhaze.ProcResV11.enhance_value;
    cfg->enhance_chroma  = dhaze.ProcResV11.enhance_chroma;
    cfg->iir_wt_sigma    = dhaze.ProcResV11.iir_wt_sigma;
    cfg->iir_sigma       = dhaze.ProcResV11.iir_sigma;
    cfg->stab_fnum       = dhaze.ProcResV11.stab_fnum;
    cfg->iir_tmax_sigma  = dhaze.ProcResV11.iir_tmax_sigma;
    cfg->iir_air_sigma   = dhaze.ProcResV11.iir_air_sigma;
    cfg->iir_pre_wet     = dhaze.ProcResV11.iir_pre_wet;
    cfg->cfg_wt          = dhaze.ProcResV11.cfg_wt;
    cfg->cfg_air         = dhaze.ProcResV11.cfg_air;
    cfg->cfg_alpha       = dhaze.ProcResV11.cfg_alpha;
    cfg->cfg_gratio      = dhaze.ProcResV11.cfg_gratio;
    cfg->cfg_tmax        = dhaze.ProcResV11.cfg_tmax;
    cfg->range_sima      = dhaze.ProcResV11.range_sima;
    cfg->space_sigma_cur = dhaze.ProcResV11.space_sigma_cur;
    cfg->space_sigma_pre = dhaze.ProcResV11.space_sigma_pre;
    cfg->dc_weitcur      = dhaze.ProcResV11.dc_weitcur;
    cfg->bf_weight       = dhaze.ProcResV11.bf_weight;
    cfg->gaus_h0         = dhaze.ProcResV11.gaus_h0;
    cfg->gaus_h1         = dhaze.ProcResV11.gaus_h1;
    cfg->gaus_h2         = dhaze.ProcResV11.gaus_h2;

    for(int i = 0; i < ISP21_DHAZ_ENH_CURVE_NUM; i++)
        cfg->enh_curve[i] = dhaze.ProcResV11.enh_curve[i];

    LOG1_ADEHAZE("%s: enhance_en:%d air_lc_en:%d hpara_en:%d hist_en:%d dc_en:%d\n", __func__,
                 cfg->enhance_en, cfg->air_lc_en, cfg->hpara_en, cfg->hist_en, cfg->dc_en);
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
    LOG1_ADEHAZE("%s: enh_curve: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
                 __FUNCTION__, cfg->enh_curve[0], cfg->enh_curve[1], cfg->enh_curve[2],
                 cfg->enh_curve[3], cfg->enh_curve[4], cfg->enh_curve[5], cfg->enh_curve[6],
                 cfg->enh_curve[7], cfg->enh_curve[8], cfg->enh_curve[9], cfg->enh_curve[10],
                 cfg->enh_curve[11], cfg->enh_curve[12], cfg->enh_curve[13], cfg->enh_curve[14],
                 cfg->enh_curve[15], cfg->enh_curve[16]);
}
#endif
#if RKAIQ_HAVE_CCM_V1
template<class T>
void Isp21Params::convertAiqCcmToIsp21Params(T& isp_cfg,
        const rk_aiq_ccm_cfg_t& ccm)
{
    if(ccm.ccmEnable) {
        isp_cfg.module_ens |= ISP2X_MODULE_CCM;
    }
    isp_cfg.module_en_update |= ISP2X_MODULE_CCM;
    isp_cfg.module_cfg_update |= ISP2X_MODULE_CCM;

    struct isp21_ccm_cfg *  cfg = &isp_cfg.others.ccm_cfg;
    const float *coeff = ccm.matrix;
    const float *offset = ccm.offs;

    cfg->coeff0_r =  (coeff[0] - 1) > 0 ? (short)((coeff[0] - 1) * 128 + 0.5) : (short)((coeff[0] - 1) * 128 - 0.5); //check -128?
    cfg->coeff1_r =  coeff[1] > 0 ? (short)(coeff[1] * 128 + 0.5) : (short)(coeff[1] * 128 - 0.5);
    cfg->coeff2_r =  coeff[2] > 0 ? (short)(coeff[2] * 128 + 0.5) : (short)(coeff[2] * 128 - 0.5);
    cfg->coeff0_g =  coeff[3] > 0 ? (short)(coeff[3] * 128 + 0.5) : (short)(coeff[3] * 128 - 0.5);
    cfg->coeff1_g =  (coeff[4] - 1) > 0 ? (short)((coeff[4] - 1) * 128 + 0.5) : (short)((coeff[4] - 1) * 128 - 0.5);
    cfg->coeff2_g =  coeff[5] > 0 ? (short)(coeff[5] * 128 + 0.5) : (short)(coeff[5] * 128 - 0.5);
    cfg->coeff0_b =  coeff[6] > 0 ? (short)(coeff[6] * 128 + 0.5) : (short)(coeff[6] * 128 - 0.5);
    cfg->coeff1_b =  coeff[7] > 0 ? (short)(coeff[7] * 128 + 0.5) : (short)(coeff[7] * 128 - 0.5);
    cfg->coeff2_b =  (coeff[8] - 1) > 0 ? (short)((coeff[8] - 1) * 128 + 0.5) : (short)((coeff[8] - 1) * 128 - 0.5);

    cfg->offset_r = offset[0] > 0 ? (short)(offset[0] + 0.5) : (short)(offset[0] - 0.5);// for 12bit
    cfg->offset_g = offset[1] > 0 ? (short)(offset[1] + 0.5) : (int)(offset[1] - 0.5);
    cfg->offset_b = offset[2] > 0 ? (short)(offset[2] + 0.5) : (short)(offset[2] - 0.5);

    cfg->coeff0_y = (u16 )ccm.rgb2y_para[0];
    cfg->coeff1_y = (u16 )ccm.rgb2y_para[1];
    cfg->coeff2_y = (u16 )ccm.rgb2y_para[2];
    cfg->bound_bit = (u8)ccm.bound_bit;//check
    cfg->highy_adjust_dis = 0;
    for( int i = 0; i < 17; i++)
    {
        cfg->alp_y[i] = (u16)(ccm.alp_y[i]);
    }

}
#endif

#if RKAIQ_HAVE_AWB_V21
void
Isp21Params::convertAiqAwbToIsp21Params(struct isp21_isp_params_cfg& isp_cfg,
                                        const rk_aiq_awb_stat_cfg_v201_t& awb_meas,
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
    struct isp21_rawawb_meas_cfg * awb_cfg_v201 = &isp_cfg.meas.rawawb;
    awb_cfg_v201->rawawb_sel                =    awb_meas.frameChoose;
    awb_cfg_v201->sw_rawawb_xy_en0          =  awb_meas.xyDetectionEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201];
    awb_cfg_v201->sw_rawawb_uv_en0          =  awb_meas.uvDetectionEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201];
    awb_cfg_v201->sw_rawawb_3dyuv_en0       =  awb_meas.threeDyuvEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201];
    awb_cfg_v201->sw_rawawb_xy_en1          =  awb_meas.xyDetectionEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201];
    awb_cfg_v201->sw_rawawb_uv_en1          =  awb_meas.uvDetectionEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201];
    awb_cfg_v201->sw_rawawb_3dyuv_en1          =  awb_meas.threeDyuvEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201];
    awb_cfg_v201->sw_rawawb_wp_blk_wei_en0    =  awb_meas.blkWeightEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201];
    awb_cfg_v201->sw_rawawb_wp_blk_wei_en1    =  awb_meas.blkWeightEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201];
    awb_cfg_v201->sw_rawlsc_bypass_en    =  awb_meas.lscBypEnable;//check
    awb_cfg_v201->sw_rawawb_blk_measure_enable    =  awb_meas.blkStatisticsEnable;
    awb_cfg_v201->sw_rawawb_blk_measure_mode     =  awb_meas.blkMeasureMode;
    awb_cfg_v201->sw_rawawb_blk_measure_xytype     =  awb_meas.xyRangeTypeForBlkStatistics;
    awb_cfg_v201->sw_rawawb_blk_measure_illu_idx     =  awb_meas.illIdxForBlkStatistics;
    awb_cfg_v201->sw_rawawb_blk_with_luma_wei_en  =  awb_meas.blkStatisticsWithLumaWeightEn;
    awb_cfg_v201->sw_rawawb_wp_luma_wei_en0   =  awb_meas.wpDiffWeiEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201];
    awb_cfg_v201->sw_rawawb_wp_luma_wei_en1   =  awb_meas.wpDiffWeiEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201];
    awb_cfg_v201->sw_rawawb_wp_hist_xytype    =  awb_meas.xyRangeTypeForWpHist;
    awb_cfg_v201->sw_rawawb_3dyuv_ls_idx0       =  awb_meas.threeDyuvIllu[0];
    awb_cfg_v201->sw_rawawb_3dyuv_ls_idx1       =  awb_meas.threeDyuvIllu[1];
    awb_cfg_v201->sw_rawawb_3dyuv_ls_idx2       =  awb_meas.threeDyuvIllu[2];
    awb_cfg_v201->sw_rawawb_3dyuv_ls_idx3       =  awb_meas.threeDyuvIllu[3];
    awb_cfg_v201->sw_rawawb_light_num      =  awb_meas.lightNum;
    awb_cfg_v201->sw_rawawb_h_offs         =  awb_meas.windowSet[0];
    awb_cfg_v201->sw_rawawb_v_offs         =  awb_meas.windowSet[1];
    awb_cfg_v201->sw_rawawb_h_size         =  awb_meas.windowSet[2];
    awb_cfg_v201->sw_rawawb_v_size         =  awb_meas.windowSet[3];
    awb_cfg_v201->sw_rawawb_wind_size = awb_meas.dsMode;
    awb_cfg_v201->sw_rawawb_r_max          =  awb_meas.maxR;
    awb_cfg_v201->sw_rawawb_g_max          =  awb_meas.maxG;
    awb_cfg_v201->sw_rawawb_b_max          =  awb_meas.maxB;
    awb_cfg_v201->sw_rawawb_y_max          =  awb_meas.maxY;
    awb_cfg_v201->sw_rawawb_r_min          =  awb_meas.minR;
    awb_cfg_v201->sw_rawawb_g_min          =  awb_meas.minG;
    awb_cfg_v201->sw_rawawb_b_min          =  awb_meas.minB;
    awb_cfg_v201->sw_rawawb_y_min          =  awb_meas.minY;
    awb_cfg_v201->sw_rawawb_vertex0_u_0    =  awb_meas.uvRange_param[0].pu_region[0];
    awb_cfg_v201->sw_rawawb_vertex0_v_0    =  awb_meas.uvRange_param[0].pv_region[0];
    awb_cfg_v201->sw_rawawb_vertex1_u_0    =  awb_meas.uvRange_param[0].pu_region[1];
    awb_cfg_v201->sw_rawawb_vertex1_v_0    =  awb_meas.uvRange_param[0].pv_region[1];
    awb_cfg_v201->sw_rawawb_vertex2_u_0    =  awb_meas.uvRange_param[0].pu_region[2];
    awb_cfg_v201->sw_rawawb_vertex2_v_0    =  awb_meas.uvRange_param[0].pv_region[2];
    awb_cfg_v201->sw_rawawb_vertex3_u_0    =  awb_meas.uvRange_param[0].pu_region[3];
    awb_cfg_v201->sw_rawawb_vertex3_v_0    =  awb_meas.uvRange_param[0].pv_region[3];
    awb_cfg_v201->sw_rawawb_islope01_0      =  awb_meas.uvRange_param[0].slope_inv[0];
    awb_cfg_v201->sw_rawawb_islope12_0      =  awb_meas.uvRange_param[0].slope_inv[1];
    awb_cfg_v201->sw_rawawb_islope23_0      =  awb_meas.uvRange_param[0].slope_inv[2];
    awb_cfg_v201->sw_rawawb_islope30_0      =  awb_meas.uvRange_param[0].slope_inv[3];
    awb_cfg_v201->sw_rawawb_vertex0_u_1    =  awb_meas.uvRange_param[1].pu_region[0];
    awb_cfg_v201->sw_rawawb_vertex0_v_1    =  awb_meas.uvRange_param[1].pv_region[0];
    awb_cfg_v201->sw_rawawb_vertex1_u_1    =  awb_meas.uvRange_param[1].pu_region[1];
    awb_cfg_v201->sw_rawawb_vertex1_v_1    =  awb_meas.uvRange_param[1].pv_region[1];
    awb_cfg_v201->sw_rawawb_vertex2_u_1    =  awb_meas.uvRange_param[1].pu_region[2];
    awb_cfg_v201->sw_rawawb_vertex2_v_1    =  awb_meas.uvRange_param[1].pv_region[2];
    awb_cfg_v201->sw_rawawb_vertex3_u_1    =  awb_meas.uvRange_param[1].pu_region[3];
    awb_cfg_v201->sw_rawawb_vertex3_v_1    =  awb_meas.uvRange_param[1].pv_region[3];
    awb_cfg_v201->sw_rawawb_islope01_1      =  awb_meas.uvRange_param[1].slope_inv[0];
    awb_cfg_v201->sw_rawawb_islope12_1      =  awb_meas.uvRange_param[1].slope_inv[1];
    awb_cfg_v201->sw_rawawb_islope23_1      =  awb_meas.uvRange_param[1].slope_inv[2];
    awb_cfg_v201->sw_rawawb_islope30_1      =  awb_meas.uvRange_param[1].slope_inv[3];
    awb_cfg_v201->sw_rawawb_vertex0_u_2    =  awb_meas.uvRange_param[2].pu_region[0];
    awb_cfg_v201->sw_rawawb_vertex0_v_2    =  awb_meas.uvRange_param[2].pv_region[0];
    awb_cfg_v201->sw_rawawb_vertex1_u_2    =  awb_meas.uvRange_param[2].pu_region[1];
    awb_cfg_v201->sw_rawawb_vertex1_v_2    =  awb_meas.uvRange_param[2].pv_region[1];
    awb_cfg_v201->sw_rawawb_vertex2_u_2    =  awb_meas.uvRange_param[2].pu_region[2];
    awb_cfg_v201->sw_rawawb_vertex2_v_2    =  awb_meas.uvRange_param[2].pv_region[2];
    awb_cfg_v201->sw_rawawb_vertex3_u_2    =  awb_meas.uvRange_param[2].pu_region[3];
    awb_cfg_v201->sw_rawawb_vertex3_v_2    =  awb_meas.uvRange_param[2].pv_region[3];
    awb_cfg_v201->sw_rawawb_islope01_2      =  awb_meas.uvRange_param[2].slope_inv[0];
    awb_cfg_v201->sw_rawawb_islope12_2      =  awb_meas.uvRange_param[2].slope_inv[1];
    awb_cfg_v201->sw_rawawb_islope23_2      =  awb_meas.uvRange_param[2].slope_inv[2];
    awb_cfg_v201->sw_rawawb_islope30_2      =  awb_meas.uvRange_param[2].slope_inv[3];
    awb_cfg_v201->sw_rawawb_vertex0_u_3    =  awb_meas.uvRange_param[3].pu_region[0];
    awb_cfg_v201->sw_rawawb_vertex0_v_3    =  awb_meas.uvRange_param[3].pv_region[0];
    awb_cfg_v201->sw_rawawb_vertex1_u_3    =  awb_meas.uvRange_param[3].pu_region[1];
    awb_cfg_v201->sw_rawawb_vertex1_v_3    =  awb_meas.uvRange_param[3].pv_region[1];
    awb_cfg_v201->sw_rawawb_vertex2_u_3    =  awb_meas.uvRange_param[3].pu_region[2];
    awb_cfg_v201->sw_rawawb_vertex2_v_3    =  awb_meas.uvRange_param[3].pv_region[2];
    awb_cfg_v201->sw_rawawb_vertex3_u_3    =  awb_meas.uvRange_param[3].pu_region[3];
    awb_cfg_v201->sw_rawawb_vertex3_v_3    =  awb_meas.uvRange_param[3].pv_region[3];
    awb_cfg_v201->sw_rawawb_islope01_3      =  awb_meas.uvRange_param[3].slope_inv[0];
    awb_cfg_v201->sw_rawawb_islope12_3      =  awb_meas.uvRange_param[3].slope_inv[1];
    awb_cfg_v201->sw_rawawb_islope23_3      =  awb_meas.uvRange_param[3].slope_inv[2];
    awb_cfg_v201->sw_rawawb_islope30_3      =  awb_meas.uvRange_param[3].slope_inv[3];
    awb_cfg_v201->sw_rawawb_vertex0_u_4    =  awb_meas.uvRange_param[4].pu_region[0];
    awb_cfg_v201->sw_rawawb_vertex0_v_4    =  awb_meas.uvRange_param[4].pv_region[0];
    awb_cfg_v201->sw_rawawb_vertex1_u_4    =  awb_meas.uvRange_param[4].pu_region[1];
    awb_cfg_v201->sw_rawawb_vertex1_v_4    =  awb_meas.uvRange_param[4].pv_region[1];
    awb_cfg_v201->sw_rawawb_vertex2_u_4    =  awb_meas.uvRange_param[4].pu_region[2];
    awb_cfg_v201->sw_rawawb_vertex2_v_4    =  awb_meas.uvRange_param[4].pv_region[2];
    awb_cfg_v201->sw_rawawb_vertex3_u_4    =  awb_meas.uvRange_param[4].pu_region[3];
    awb_cfg_v201->sw_rawawb_vertex3_v_4    =  awb_meas.uvRange_param[4].pv_region[3];
    awb_cfg_v201->sw_rawawb_islope01_4      =  awb_meas.uvRange_param[4].slope_inv[0];
    awb_cfg_v201->sw_rawawb_islope12_4      =  awb_meas.uvRange_param[4].slope_inv[1];
    awb_cfg_v201->sw_rawawb_islope23_4      =  awb_meas.uvRange_param[4].slope_inv[2];
    awb_cfg_v201->sw_rawawb_islope30_4      =  awb_meas.uvRange_param[4].slope_inv[3];
    awb_cfg_v201->sw_rawawb_vertex0_u_5    =  awb_meas.uvRange_param[5].pu_region[0];
    awb_cfg_v201->sw_rawawb_vertex0_v_5    =  awb_meas.uvRange_param[5].pv_region[0];
    awb_cfg_v201->sw_rawawb_vertex1_u_5    =  awb_meas.uvRange_param[5].pu_region[1];
    awb_cfg_v201->sw_rawawb_vertex1_v_5    =  awb_meas.uvRange_param[5].pv_region[1];
    awb_cfg_v201->sw_rawawb_vertex2_u_5    =  awb_meas.uvRange_param[5].pu_region[2];
    awb_cfg_v201->sw_rawawb_vertex2_v_5    =  awb_meas.uvRange_param[5].pv_region[2];
    awb_cfg_v201->sw_rawawb_vertex3_u_5    =  awb_meas.uvRange_param[5].pu_region[3];
    awb_cfg_v201->sw_rawawb_vertex3_v_5    =  awb_meas.uvRange_param[5].pv_region[3];
    awb_cfg_v201->sw_rawawb_islope01_5      =  awb_meas.uvRange_param[5].slope_inv[0];
    awb_cfg_v201->sw_rawawb_islope12_5      =  awb_meas.uvRange_param[5].slope_inv[1];
    awb_cfg_v201->sw_rawawb_islope23_5      =  awb_meas.uvRange_param[5].slope_inv[2];
    awb_cfg_v201->sw_rawawb_islope30_5      =  awb_meas.uvRange_param[5].slope_inv[3];
    awb_cfg_v201->sw_rawawb_vertex0_u_6    =  awb_meas.uvRange_param[6].pu_region[0];
    awb_cfg_v201->sw_rawawb_vertex0_v_6    =  awb_meas.uvRange_param[6].pv_region[0];
    awb_cfg_v201->sw_rawawb_vertex1_u_6    =  awb_meas.uvRange_param[6].pu_region[1];
    awb_cfg_v201->sw_rawawb_vertex1_v_6    =  awb_meas.uvRange_param[6].pv_region[1];
    awb_cfg_v201->sw_rawawb_vertex2_u_6    =  awb_meas.uvRange_param[6].pu_region[2];
    awb_cfg_v201->sw_rawawb_vertex2_v_6    =  awb_meas.uvRange_param[6].pv_region[2];
    awb_cfg_v201->sw_rawawb_vertex3_u_6    =  awb_meas.uvRange_param[6].pu_region[3];
    awb_cfg_v201->sw_rawawb_vertex3_v_6    =  awb_meas.uvRange_param[6].pv_region[3];
    awb_cfg_v201->sw_rawawb_islope01_6      =  awb_meas.uvRange_param[6].slope_inv[0];
    awb_cfg_v201->sw_rawawb_islope12_6      =  awb_meas.uvRange_param[6].slope_inv[1];
    awb_cfg_v201->sw_rawawb_islope23_6      =  awb_meas.uvRange_param[6].slope_inv[2];
    awb_cfg_v201->sw_rawawb_islope30_6      =  awb_meas.uvRange_param[6].slope_inv[3];
    awb_cfg_v201->sw_rawawb_rgb2ryuvmat0_u =  awb_meas.icrgb2RYuv_matrix[0];
    awb_cfg_v201->sw_rawawb_rgb2ryuvmat1_u =  awb_meas.icrgb2RYuv_matrix[1];
    awb_cfg_v201->sw_rawawb_rgb2ryuvmat2_u =  awb_meas.icrgb2RYuv_matrix[2];
    awb_cfg_v201->sw_rawawb_rgb2ryuvofs_u =  awb_meas.icrgb2RYuv_matrix[3];
    awb_cfg_v201->sw_rawawb_rgb2ryuvmat0_v =  awb_meas.icrgb2RYuv_matrix[4];
    awb_cfg_v201->sw_rawawb_rgb2ryuvmat1_v =  awb_meas.icrgb2RYuv_matrix[5];
    awb_cfg_v201->sw_rawawb_rgb2ryuvmat2_v =  awb_meas.icrgb2RYuv_matrix[6];
    awb_cfg_v201->sw_rawawb_rgb2ryuvofs_v =  awb_meas.icrgb2RYuv_matrix[7];
    awb_cfg_v201->sw_rawawb_rgb2ryuvmat0_y =  awb_meas.icrgb2RYuv_matrix[8];
    awb_cfg_v201->sw_rawawb_rgb2ryuvmat1_y =  awb_meas.icrgb2RYuv_matrix[9];
    awb_cfg_v201->sw_rawawb_rgb2ryuvmat2_y =  awb_meas.icrgb2RYuv_matrix[10];
    awb_cfg_v201->sw_rawawb_rgb2ryuvofs_y =  awb_meas.icrgb2RYuv_matrix[11];
    awb_cfg_v201->sw_rawawb_rotu0_ls0 =  awb_meas.ic3Dyuv2Range_param[0].thcurve_u[0];
    awb_cfg_v201->sw_rawawb_rotu1_ls0 =  awb_meas.ic3Dyuv2Range_param[0].thcurve_u[1];
    awb_cfg_v201->sw_rawawb_rotu2_ls0 =  awb_meas.ic3Dyuv2Range_param[0].thcurve_u[2];
    awb_cfg_v201->sw_rawawb_rotu3_ls0 =  awb_meas.ic3Dyuv2Range_param[0].thcurve_u[3];
    awb_cfg_v201->sw_rawawb_rotu4_ls0 =  awb_meas.ic3Dyuv2Range_param[0].thcurve_u[4];
    awb_cfg_v201->sw_rawawb_rotu5_ls0 =  awb_meas.ic3Dyuv2Range_param[0].thcurve_u[5];
    awb_cfg_v201->sw_rawawb_th0_ls0 =  awb_meas.ic3Dyuv2Range_param[0].thcure_th[0];
    awb_cfg_v201->sw_rawawb_th1_ls0 =  awb_meas.ic3Dyuv2Range_param[0].thcure_th[1];
    awb_cfg_v201->sw_rawawb_th2_ls0 =  awb_meas.ic3Dyuv2Range_param[0].thcure_th[2];
    awb_cfg_v201->sw_rawawb_th3_ls0 =  awb_meas.ic3Dyuv2Range_param[0].thcure_th[3];
    awb_cfg_v201->sw_rawawb_th4_ls0 =  awb_meas.ic3Dyuv2Range_param[0].thcure_th[4];
    awb_cfg_v201->sw_rawawb_th5_ls0 =  awb_meas.ic3Dyuv2Range_param[0].thcure_th[5];
    awb_cfg_v201->sw_rawawb_coor_x1_ls0_u =  awb_meas.ic3Dyuv2Range_param[0].lineP1[0];
    awb_cfg_v201->sw_rawawb_coor_x1_ls0_v =  awb_meas.ic3Dyuv2Range_param[0].lineP1[1];
    awb_cfg_v201->sw_rawawb_coor_x1_ls0_y =  awb_meas.ic3Dyuv2Range_param[0].lineP1[2];
    awb_cfg_v201->sw_rawawb_vec_x21_ls0_u =  awb_meas.ic3Dyuv2Range_param[0].vP1P2[0];
    awb_cfg_v201->sw_rawawb_vec_x21_ls0_v =  awb_meas.ic3Dyuv2Range_param[0].vP1P2[1];
    awb_cfg_v201->sw_rawawb_vec_x21_ls0_y =  awb_meas.ic3Dyuv2Range_param[0].vP1P2[2];
    awb_cfg_v201->sw_rawawb_dis_x1x2_ls0 =  awb_meas.ic3Dyuv2Range_param[0].disP1P2;
    awb_cfg_v201->sw_rawawb_rotu0_ls1 =  awb_meas.ic3Dyuv2Range_param[1].thcurve_u[0];
    awb_cfg_v201->sw_rawawb_rotu1_ls1 =  awb_meas.ic3Dyuv2Range_param[1].thcurve_u[1];
    awb_cfg_v201->sw_rawawb_rotu2_ls1 =  awb_meas.ic3Dyuv2Range_param[1].thcurve_u[2];
    awb_cfg_v201->sw_rawawb_rotu3_ls1 =  awb_meas.ic3Dyuv2Range_param[1].thcurve_u[3];
    awb_cfg_v201->sw_rawawb_rotu4_ls1 =  awb_meas.ic3Dyuv2Range_param[1].thcurve_u[4];
    awb_cfg_v201->sw_rawawb_rotu5_ls1 =  awb_meas.ic3Dyuv2Range_param[1].thcurve_u[5];
    awb_cfg_v201->sw_rawawb_th0_ls1 =  awb_meas.ic3Dyuv2Range_param[1].thcure_th[0];
    awb_cfg_v201->sw_rawawb_th1_ls1 =  awb_meas.ic3Dyuv2Range_param[1].thcure_th[1];
    awb_cfg_v201->sw_rawawb_th2_ls1 =  awb_meas.ic3Dyuv2Range_param[1].thcure_th[2];
    awb_cfg_v201->sw_rawawb_th3_ls1 =  awb_meas.ic3Dyuv2Range_param[1].thcure_th[3];
    awb_cfg_v201->sw_rawawb_th4_ls1 =  awb_meas.ic3Dyuv2Range_param[1].thcure_th[4];
    awb_cfg_v201->sw_rawawb_th5_ls1 =  awb_meas.ic3Dyuv2Range_param[1].thcure_th[5];
    awb_cfg_v201->sw_rawawb_coor_x1_ls1_u =  awb_meas.ic3Dyuv2Range_param[1].lineP1[0];
    awb_cfg_v201->sw_rawawb_coor_x1_ls1_v =  awb_meas.ic3Dyuv2Range_param[1].lineP1[1];
    awb_cfg_v201->sw_rawawb_coor_x1_ls1_y =  awb_meas.ic3Dyuv2Range_param[1].lineP1[2];
    awb_cfg_v201->sw_rawawb_vec_x21_ls1_u =  awb_meas.ic3Dyuv2Range_param[1].vP1P2[0];
    awb_cfg_v201->sw_rawawb_vec_x21_ls1_v =  awb_meas.ic3Dyuv2Range_param[1].vP1P2[1];
    awb_cfg_v201->sw_rawawb_vec_x21_ls1_y =  awb_meas.ic3Dyuv2Range_param[1].vP1P2[2];
    awb_cfg_v201->sw_rawawb_dis_x1x2_ls1 =  awb_meas.ic3Dyuv2Range_param[1].disP1P2;
    awb_cfg_v201->sw_rawawb_rotu0_ls2 =  awb_meas.ic3Dyuv2Range_param[2].thcurve_u[0];
    awb_cfg_v201->sw_rawawb_rotu1_ls2 =  awb_meas.ic3Dyuv2Range_param[2].thcurve_u[1];
    awb_cfg_v201->sw_rawawb_rotu2_ls2 =  awb_meas.ic3Dyuv2Range_param[2].thcurve_u[2];
    awb_cfg_v201->sw_rawawb_rotu3_ls2 =  awb_meas.ic3Dyuv2Range_param[2].thcurve_u[3];
    awb_cfg_v201->sw_rawawb_rotu4_ls2 =  awb_meas.ic3Dyuv2Range_param[2].thcurve_u[4];
    awb_cfg_v201->sw_rawawb_rotu5_ls2 =  awb_meas.ic3Dyuv2Range_param[2].thcurve_u[5];
    awb_cfg_v201->sw_rawawb_th0_ls2 =  awb_meas.ic3Dyuv2Range_param[2].thcure_th[0];
    awb_cfg_v201->sw_rawawb_th1_ls2 =  awb_meas.ic3Dyuv2Range_param[2].thcure_th[1];
    awb_cfg_v201->sw_rawawb_th2_ls2 =  awb_meas.ic3Dyuv2Range_param[2].thcure_th[2];
    awb_cfg_v201->sw_rawawb_th3_ls2 =  awb_meas.ic3Dyuv2Range_param[2].thcure_th[3];
    awb_cfg_v201->sw_rawawb_th4_ls2 =  awb_meas.ic3Dyuv2Range_param[2].thcure_th[4];
    awb_cfg_v201->sw_rawawb_th5_ls2 =  awb_meas.ic3Dyuv2Range_param[2].thcure_th[5];
    awb_cfg_v201->sw_rawawb_coor_x1_ls2_u =  awb_meas.ic3Dyuv2Range_param[2].lineP1[0];
    awb_cfg_v201->sw_rawawb_coor_x1_ls2_v =  awb_meas.ic3Dyuv2Range_param[2].lineP1[1];
    awb_cfg_v201->sw_rawawb_coor_x1_ls2_y =  awb_meas.ic3Dyuv2Range_param[2].lineP1[2];
    awb_cfg_v201->sw_rawawb_vec_x21_ls2_u =  awb_meas.ic3Dyuv2Range_param[2].vP1P2[0];
    awb_cfg_v201->sw_rawawb_vec_x21_ls2_v =  awb_meas.ic3Dyuv2Range_param[2].vP1P2[1];
    awb_cfg_v201->sw_rawawb_vec_x21_ls2_y =  awb_meas.ic3Dyuv2Range_param[2].vP1P2[2];
    awb_cfg_v201->sw_rawawb_dis_x1x2_ls2 =  awb_meas.ic3Dyuv2Range_param[2].disP1P2;

    awb_cfg_v201->sw_rawawb_rotu0_ls3 =  awb_meas.ic3Dyuv2Range_param[3].thcurve_u[0];
    awb_cfg_v201->sw_rawawb_rotu1_ls3 =  awb_meas.ic3Dyuv2Range_param[3].thcurve_u[1];
    awb_cfg_v201->sw_rawawb_rotu2_ls3 =  awb_meas.ic3Dyuv2Range_param[3].thcurve_u[2];
    awb_cfg_v201->sw_rawawb_rotu3_ls3 =  awb_meas.ic3Dyuv2Range_param[3].thcurve_u[3];
    awb_cfg_v201->sw_rawawb_rotu4_ls3 =  awb_meas.ic3Dyuv2Range_param[3].thcurve_u[4];
    awb_cfg_v201->sw_rawawb_rotu5_ls3 =  awb_meas.ic3Dyuv2Range_param[3].thcurve_u[5];
    awb_cfg_v201->sw_rawawb_th0_ls3 =  awb_meas.ic3Dyuv2Range_param[3].thcure_th[0];
    awb_cfg_v201->sw_rawawb_th1_ls3 =  awb_meas.ic3Dyuv2Range_param[3].thcure_th[1];
    awb_cfg_v201->sw_rawawb_th2_ls3 =  awb_meas.ic3Dyuv2Range_param[3].thcure_th[2];
    awb_cfg_v201->sw_rawawb_th3_ls3 =  awb_meas.ic3Dyuv2Range_param[3].thcure_th[3];
    awb_cfg_v201->sw_rawawb_th4_ls3 =  awb_meas.ic3Dyuv2Range_param[3].thcure_th[4];
    awb_cfg_v201->sw_rawawb_th5_ls3 =  awb_meas.ic3Dyuv2Range_param[3].thcure_th[5];
    awb_cfg_v201->sw_rawawb_coor_x1_ls3_u =  awb_meas.ic3Dyuv2Range_param[3].lineP1[0];
    awb_cfg_v201->sw_rawawb_coor_x1_ls3_v =  awb_meas.ic3Dyuv2Range_param[3].lineP1[1];
    awb_cfg_v201->sw_rawawb_coor_x1_ls3_y =  awb_meas.ic3Dyuv2Range_param[3].lineP1[2];
    awb_cfg_v201->sw_rawawb_vec_x21_ls3_u =  awb_meas.ic3Dyuv2Range_param[3].vP1P2[0];
    awb_cfg_v201->sw_rawawb_vec_x21_ls3_v =  awb_meas.ic3Dyuv2Range_param[3].vP1P2[1];
    awb_cfg_v201->sw_rawawb_vec_x21_ls3_y =  awb_meas.ic3Dyuv2Range_param[3].vP1P2[2];
    awb_cfg_v201->sw_rawawb_dis_x1x2_ls3 =  awb_meas.ic3Dyuv2Range_param[3].disP1P2;
    awb_cfg_v201->sw_rawawb_wt0            =  awb_meas.rgb2xy_param.pseudoLuminanceWeight[0];
    awb_cfg_v201->sw_rawawb_wt1            =  awb_meas.rgb2xy_param.pseudoLuminanceWeight[1];
    awb_cfg_v201->sw_rawawb_wt2            =  awb_meas.rgb2xy_param.pseudoLuminanceWeight[2];
    awb_cfg_v201->sw_rawawb_mat0_x         =  awb_meas.rgb2xy_param.rotationMat[0];
    awb_cfg_v201->sw_rawawb_mat1_x         =  awb_meas.rgb2xy_param.rotationMat[1];
    awb_cfg_v201->sw_rawawb_mat2_x         =  awb_meas.rgb2xy_param.rotationMat[2];
    awb_cfg_v201->sw_rawawb_mat0_y         =  awb_meas.rgb2xy_param.rotationMat[3];
    awb_cfg_v201->sw_rawawb_mat1_y         =  awb_meas.rgb2xy_param.rotationMat[4];
    awb_cfg_v201->sw_rawawb_mat2_y         =  awb_meas.rgb2xy_param.rotationMat[5];
    awb_cfg_v201->sw_rawawb_nor_x0_0       =  awb_meas.xyRange_param[0].NorrangeX[0];
    awb_cfg_v201->sw_rawawb_nor_x1_0       =  awb_meas.xyRange_param[0].NorrangeX[1];
    awb_cfg_v201->sw_rawawb_nor_y0_0       =  awb_meas.xyRange_param[0].NorrangeY[0];
    awb_cfg_v201->sw_rawawb_nor_y1_0       =  awb_meas.xyRange_param[0].NorrangeY[1];
    awb_cfg_v201->sw_rawawb_big_x0_0       =  awb_meas.xyRange_param[0].SperangeX[0];
    awb_cfg_v201->sw_rawawb_big_x1_0       =  awb_meas.xyRange_param[0].SperangeX[1];
    awb_cfg_v201->sw_rawawb_big_y0_0       =  awb_meas.xyRange_param[0].SperangeY[0];
    awb_cfg_v201->sw_rawawb_big_y1_0       =  awb_meas.xyRange_param[0].SperangeY[1];
    awb_cfg_v201->sw_rawawb_nor_x0_1       =  awb_meas.xyRange_param[1].NorrangeX[0];
    awb_cfg_v201->sw_rawawb_nor_x1_1       =  awb_meas.xyRange_param[1].NorrangeX[1];
    awb_cfg_v201->sw_rawawb_nor_y0_1       =  awb_meas.xyRange_param[1].NorrangeY[0];
    awb_cfg_v201->sw_rawawb_nor_y1_1       =  awb_meas.xyRange_param[1].NorrangeY[1];
    awb_cfg_v201->sw_rawawb_big_x0_1       =  awb_meas.xyRange_param[1].SperangeX[0];
    awb_cfg_v201->sw_rawawb_big_x1_1       =  awb_meas.xyRange_param[1].SperangeX[1];
    awb_cfg_v201->sw_rawawb_big_y0_1       =  awb_meas.xyRange_param[1].SperangeY[0];
    awb_cfg_v201->sw_rawawb_big_y1_1       =  awb_meas.xyRange_param[1].SperangeY[1];
    awb_cfg_v201->sw_rawawb_nor_x0_2       =  awb_meas.xyRange_param[2].NorrangeX[0];
    awb_cfg_v201->sw_rawawb_nor_x1_2       =  awb_meas.xyRange_param[2].NorrangeX[1];
    awb_cfg_v201->sw_rawawb_nor_y0_2       =  awb_meas.xyRange_param[2].NorrangeY[0];
    awb_cfg_v201->sw_rawawb_nor_y1_2       =  awb_meas.xyRange_param[2].NorrangeY[1];
    awb_cfg_v201->sw_rawawb_big_x0_2       =  awb_meas.xyRange_param[2].SperangeX[0];
    awb_cfg_v201->sw_rawawb_big_x1_2       =  awb_meas.xyRange_param[2].SperangeX[1];
    awb_cfg_v201->sw_rawawb_big_y0_2       =  awb_meas.xyRange_param[2].SperangeY[0];
    awb_cfg_v201->sw_rawawb_big_y1_2       =  awb_meas.xyRange_param[2].SperangeY[1];
    awb_cfg_v201->sw_rawawb_nor_x0_3       =  awb_meas.xyRange_param[3].NorrangeX[0];
    awb_cfg_v201->sw_rawawb_nor_x1_3       =  awb_meas.xyRange_param[3].NorrangeX[1];
    awb_cfg_v201->sw_rawawb_nor_y0_3       =  awb_meas.xyRange_param[3].NorrangeY[0];
    awb_cfg_v201->sw_rawawb_nor_y1_3       =  awb_meas.xyRange_param[3].NorrangeY[1];
    awb_cfg_v201->sw_rawawb_big_x0_3       =  awb_meas.xyRange_param[3].SperangeX[0];
    awb_cfg_v201->sw_rawawb_big_x1_3       =  awb_meas.xyRange_param[3].SperangeX[1];
    awb_cfg_v201->sw_rawawb_big_y0_3       =  awb_meas.xyRange_param[3].SperangeY[0];
    awb_cfg_v201->sw_rawawb_big_y1_3       =  awb_meas.xyRange_param[3].SperangeY[1];
    awb_cfg_v201->sw_rawawb_nor_x0_4       =  awb_meas.xyRange_param[4].NorrangeX[0];
    awb_cfg_v201->sw_rawawb_nor_x1_4       =  awb_meas.xyRange_param[4].NorrangeX[1];
    awb_cfg_v201->sw_rawawb_nor_y0_4       =  awb_meas.xyRange_param[4].NorrangeY[0];
    awb_cfg_v201->sw_rawawb_nor_y1_4       =  awb_meas.xyRange_param[4].NorrangeY[1];
    awb_cfg_v201->sw_rawawb_big_x0_4       =  awb_meas.xyRange_param[4].SperangeX[0];
    awb_cfg_v201->sw_rawawb_big_x1_4       =  awb_meas.xyRange_param[4].SperangeX[1];
    awb_cfg_v201->sw_rawawb_big_y0_4       =  awb_meas.xyRange_param[4].SperangeY[0];
    awb_cfg_v201->sw_rawawb_big_y1_4       =  awb_meas.xyRange_param[4].SperangeY[1];
    awb_cfg_v201->sw_rawawb_nor_x0_5       =  awb_meas.xyRange_param[5].NorrangeX[0];
    awb_cfg_v201->sw_rawawb_nor_x1_5       =  awb_meas.xyRange_param[5].NorrangeX[1];
    awb_cfg_v201->sw_rawawb_nor_y0_5       =  awb_meas.xyRange_param[5].NorrangeY[0];
    awb_cfg_v201->sw_rawawb_nor_y1_5       =  awb_meas.xyRange_param[5].NorrangeY[1];
    awb_cfg_v201->sw_rawawb_big_x0_5       =  awb_meas.xyRange_param[5].SperangeX[0];
    awb_cfg_v201->sw_rawawb_big_x1_5       =  awb_meas.xyRange_param[5].SperangeX[1];
    awb_cfg_v201->sw_rawawb_big_y0_5       =  awb_meas.xyRange_param[5].SperangeY[0];
    awb_cfg_v201->sw_rawawb_big_y1_5       =  awb_meas.xyRange_param[5].SperangeY[1];
    awb_cfg_v201->sw_rawawb_nor_x0_6       =  awb_meas.xyRange_param[6].NorrangeX[0];
    awb_cfg_v201->sw_rawawb_nor_x1_6       =  awb_meas.xyRange_param[6].NorrangeX[1];
    awb_cfg_v201->sw_rawawb_nor_y0_6       =  awb_meas.xyRange_param[6].NorrangeY[0];
    awb_cfg_v201->sw_rawawb_nor_y1_6       =  awb_meas.xyRange_param[6].NorrangeY[1];
    awb_cfg_v201->sw_rawawb_big_x0_6       =  awb_meas.xyRange_param[6].SperangeX[0];
    awb_cfg_v201->sw_rawawb_big_x1_6       =  awb_meas.xyRange_param[6].SperangeX[1];
    awb_cfg_v201->sw_rawawb_big_y0_6       =  awb_meas.xyRange_param[6].SperangeY[0];
    awb_cfg_v201->sw_rawawb_big_y1_6       =  awb_meas.xyRange_param[6].SperangeY[1];
    awb_cfg_v201->sw_rawawb_pre_wbgain_inv_r       =  awb_meas.pre_wbgain_inv_r;
    awb_cfg_v201->sw_rawawb_pre_wbgain_inv_g       =  awb_meas.pre_wbgain_inv_g;
    awb_cfg_v201->sw_rawawb_pre_wbgain_inv_b       =  awb_meas.pre_wbgain_inv_b;
    awb_cfg_v201->sw_rawawb_exc_wp_region0_excen0     =  awb_meas.excludeWpRange[0].excludeEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201];
    awb_cfg_v201->sw_rawawb_exc_wp_region0_excen1     =  awb_meas.excludeWpRange[0].excludeEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201];
    //awb_cfg_v201->sw_rawawb_exc_wp_region0_measen   =  awb_meas.excludeWpRange[0].measureEnable;
    awb_cfg_v201->sw_rawawb_exc_wp_region0_domain        =  awb_meas.excludeWpRange[0].domain;
    awb_cfg_v201->sw_rawawb_exc_wp_region0_xu0          =  awb_meas.excludeWpRange[0].xu[0];
    awb_cfg_v201->sw_rawawb_exc_wp_region0_xu1          =  awb_meas.excludeWpRange[0].xu[1];
    awb_cfg_v201->sw_rawawb_exc_wp_region0_yv0          =  awb_meas.excludeWpRange[0].yv[0];
    awb_cfg_v201->sw_rawawb_exc_wp_region0_yv1          =  awb_meas.excludeWpRange[0].yv[1];
    awb_cfg_v201->sw_rawawb_exc_wp_region1_excen0     =  awb_meas.excludeWpRange[1].excludeEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201];
    awb_cfg_v201->sw_rawawb_exc_wp_region1_excen1     =  awb_meas.excludeWpRange[1].excludeEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201];
    //awb_cfg_v201->sw_rawawb_exc_wp_region1_measen   =  awb_meas.excludeWpRange[1].measureEnable;
    awb_cfg_v201->sw_rawawb_exc_wp_region1_domain        =  awb_meas.excludeWpRange[1].domain;
    awb_cfg_v201->sw_rawawb_exc_wp_region1_xu0          =  awb_meas.excludeWpRange[1].xu[0];
    awb_cfg_v201->sw_rawawb_exc_wp_region1_xu1          =  awb_meas.excludeWpRange[1].xu[1];
    awb_cfg_v201->sw_rawawb_exc_wp_region1_yv0          =  awb_meas.excludeWpRange[1].yv[0];
    awb_cfg_v201->sw_rawawb_exc_wp_region1_yv1          =  awb_meas.excludeWpRange[1].yv[1];
    awb_cfg_v201->sw_rawawb_exc_wp_region2_excen0     =  awb_meas.excludeWpRange[2].excludeEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201];
    awb_cfg_v201->sw_rawawb_exc_wp_region2_excen1     =  awb_meas.excludeWpRange[2].excludeEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201];
    //awb_cfg_v201->sw_rawawb_exc_wp_region2_measen   =  awb_meas.excludeWpRange[2].measureEnable;
    awb_cfg_v201->sw_rawawb_exc_wp_region2_domain        =  awb_meas.excludeWpRange[2].domain;
    awb_cfg_v201->sw_rawawb_exc_wp_region2_xu0          =  awb_meas.excludeWpRange[2].xu[0];
    awb_cfg_v201->sw_rawawb_exc_wp_region2_xu1          =  awb_meas.excludeWpRange[2].xu[1];
    awb_cfg_v201->sw_rawawb_exc_wp_region2_yv0          =  awb_meas.excludeWpRange[2].yv[0];
    awb_cfg_v201->sw_rawawb_exc_wp_region2_yv1          =  awb_meas.excludeWpRange[2].yv[1];
    awb_cfg_v201->sw_rawawb_exc_wp_region3_excen0     =  awb_meas.excludeWpRange[3].excludeEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201];
    awb_cfg_v201->sw_rawawb_exc_wp_region3_excen1     =  awb_meas.excludeWpRange[3].excludeEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201];
    //awb_cfg_v201->sw_rawawb_exc_wp_region3_measen   =  awb_meas.excludeWpRange[3].measureEnable;
    awb_cfg_v201->sw_rawawb_exc_wp_region3_domain        =  awb_meas.excludeWpRange[3].domain;
    awb_cfg_v201->sw_rawawb_exc_wp_region3_xu0          =  awb_meas.excludeWpRange[3].xu[0];
    awb_cfg_v201->sw_rawawb_exc_wp_region3_xu1          =  awb_meas.excludeWpRange[3].xu[1];
    awb_cfg_v201->sw_rawawb_exc_wp_region3_yv0          =  awb_meas.excludeWpRange[3].yv[0];
    awb_cfg_v201->sw_rawawb_exc_wp_region3_yv1          =  awb_meas.excludeWpRange[3].yv[1];
    awb_cfg_v201->sw_rawawb_exc_wp_region4_excen0     =  awb_meas.excludeWpRange[4].excludeEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201];
    awb_cfg_v201->sw_rawawb_exc_wp_region4_excen1     =  awb_meas.excludeWpRange[4].excludeEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201];
    //awb_cfg_v201->sw_rawawb_exc_wp_region4_measen   =  awb_meas.excludeWpRange[4].measureEnable;
    awb_cfg_v201->sw_rawawb_exc_wp_region4_domain        =  awb_meas.excludeWpRange[4].domain;
    awb_cfg_v201->sw_rawawb_exc_wp_region4_xu0          =  awb_meas.excludeWpRange[4].xu[0];
    awb_cfg_v201->sw_rawawb_exc_wp_region4_xu1          =  awb_meas.excludeWpRange[4].xu[1];
    awb_cfg_v201->sw_rawawb_exc_wp_region4_yv0          =  awb_meas.excludeWpRange[4].yv[0];
    awb_cfg_v201->sw_rawawb_exc_wp_region4_yv1          =  awb_meas.excludeWpRange[4].yv[1];
    awb_cfg_v201->sw_rawawb_exc_wp_region5_excen0     =  awb_meas.excludeWpRange[5].excludeEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201];
    awb_cfg_v201->sw_rawawb_exc_wp_region5_excen1     =  awb_meas.excludeWpRange[5].excludeEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201];
    //awb_cfg_v201->sw_rawawb_exc_wp_region5_measen   =  awb_meas.excludeWpRange[5].measureEnable;
    awb_cfg_v201->sw_rawawb_exc_wp_region5_domain        =  awb_meas.excludeWpRange[5].domain;
    awb_cfg_v201->sw_rawawb_exc_wp_region5_xu0          =  awb_meas.excludeWpRange[5].xu[0];
    awb_cfg_v201->sw_rawawb_exc_wp_region5_xu1          =  awb_meas.excludeWpRange[5].xu[1];
    awb_cfg_v201->sw_rawawb_exc_wp_region5_yv0          =  awb_meas.excludeWpRange[5].yv[0];
    awb_cfg_v201->sw_rawawb_exc_wp_region5_yv1          =  awb_meas.excludeWpRange[5].yv[1];
    awb_cfg_v201->sw_rawawb_exc_wp_region6_excen0     =  awb_meas.excludeWpRange[6].excludeEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201];
    awb_cfg_v201->sw_rawawb_exc_wp_region6_excen1     =  awb_meas.excludeWpRange[6].excludeEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201];
    //awb_cfg_v201->sw_rawawb_exc_wp_region6_measen   =  awb_meas.excludeWpRange[6].measureEnable;
    awb_cfg_v201->sw_rawawb_exc_wp_region6_domain        =  awb_meas.excludeWpRange[6].domain;
    awb_cfg_v201->sw_rawawb_exc_wp_region6_xu0          =  awb_meas.excludeWpRange[6].xu[0];
    awb_cfg_v201->sw_rawawb_exc_wp_region6_xu1          =  awb_meas.excludeWpRange[6].xu[1];
    awb_cfg_v201->sw_rawawb_exc_wp_region6_yv0          =  awb_meas.excludeWpRange[6].yv[0];
    awb_cfg_v201->sw_rawawb_exc_wp_region6_yv1          =  awb_meas.excludeWpRange[6].yv[1];
    awb_cfg_v201->sw_rawawb_wp_luma_weicurve_y0     =   awb_meas.wpDiffwei_y[0];
    awb_cfg_v201->sw_rawawb_wp_luma_weicurve_y1     =   awb_meas.wpDiffwei_y[1];
    awb_cfg_v201->sw_rawawb_wp_luma_weicurve_y2     =   awb_meas.wpDiffwei_y[2];
    awb_cfg_v201->sw_rawawb_wp_luma_weicurve_y3     =   awb_meas.wpDiffwei_y[3];
    awb_cfg_v201->sw_rawawb_wp_luma_weicurve_y4     =   awb_meas.wpDiffwei_y[4];
    awb_cfg_v201->sw_rawawb_wp_luma_weicurve_y5     =   awb_meas.wpDiffwei_y[5];
    awb_cfg_v201->sw_rawawb_wp_luma_weicurve_y6     =   awb_meas.wpDiffwei_y[6];
    awb_cfg_v201->sw_rawawb_wp_luma_weicurve_y7     =   awb_meas.wpDiffwei_y[7];
    awb_cfg_v201->sw_rawawb_wp_luma_weicurve_y8     =   awb_meas.wpDiffwei_y[8];
    awb_cfg_v201->sw_rawawb_wp_luma_weicurve_w0     = awb_meas.wpDiffwei_w[0];
    awb_cfg_v201->sw_rawawb_wp_luma_weicurve_w1     = awb_meas.wpDiffwei_w[1];
    awb_cfg_v201->sw_rawawb_wp_luma_weicurve_w2     = awb_meas.wpDiffwei_w[2];
    awb_cfg_v201->sw_rawawb_wp_luma_weicurve_w3     = awb_meas.wpDiffwei_w[3];
    awb_cfg_v201->sw_rawawb_wp_luma_weicurve_w4     = awb_meas.wpDiffwei_w[4];
    awb_cfg_v201->sw_rawawb_wp_luma_weicurve_w5     = awb_meas.wpDiffwei_w[5];
    awb_cfg_v201->sw_rawawb_wp_luma_weicurve_w6     = awb_meas.wpDiffwei_w[6];
    awb_cfg_v201->sw_rawawb_wp_luma_weicurve_w7     = awb_meas.wpDiffwei_w[7];
    awb_cfg_v201->sw_rawawb_wp_luma_weicurve_w8     = awb_meas.wpDiffwei_w[8];

    for (int i = 0; i < RK_AIQ_AWB_GRID_NUM_TOTAL; i++) {
        awb_cfg_v201->sw_rawawb_wp_blk_wei_w[i]          = awb_meas.blkWeight[i];
    }

    awb_cfg_v201->sw_rawawb_blk_rtdw_measure_en =  awb_meas.blk_rtdw_measure_en;
}
#endif
#if RKAIQ_HAVE_BAYERNR_V2
void
Isp21Params::convertAiqRawnrToIsp21Params(struct isp21_isp_params_cfg& isp_cfg,
        rk_aiq_isp_baynr_v21_t& rawnr)
{

    struct isp21_baynr_cfg * p2DCfg = &isp_cfg.others.baynr_cfg;
    struct isp21_bay3d_cfg * p3DCfg = &isp_cfg.others.bay3d_cfg;


    LOGD_ANR("%s:%d: enter\n", __FUNCTION__, __LINE__);

    //bayernr 2d
    if(rawnr.st2DParam.baynr_en) {
        isp_cfg.module_ens |= ISP2X_MODULE_BAYNR;
    } else {
        isp_cfg.module_ens &= ~ISP2X_MODULE_BAYNR;
    }

    if(rawnr.st3DParam.bay3d_en_i) {
        isp_cfg.module_ens |= ISP2X_MODULE_BAY3D;
        isp_cfg.module_ens |= ISP2X_MODULE_BAYNR;
    } else {
        isp_cfg.module_ens &= ~ISP2X_MODULE_BAY3D;
    }

    isp_cfg.module_en_update |= ISP2X_MODULE_BAYNR;
    isp_cfg.module_cfg_update |= ISP2X_MODULE_BAYNR;
    isp_cfg.module_en_update |= ISP2X_MODULE_BAY3D;
    isp_cfg.module_cfg_update |= ISP2X_MODULE_BAY3D;


    p2DCfg->sw_baynr_gauss_en = rawnr.st2DParam.baynr_gauss_en;
    p2DCfg->sw_baynr_log_bypass = rawnr.st2DParam.baynr_log_bypass;

    p2DCfg->sw_baynr_dgain1 = rawnr.st2DParam.baynr_dgain[1];
    p2DCfg->sw_baynr_dgain0 = rawnr.st2DParam.baynr_dgain[0];
    p2DCfg->sw_baynr_dgain2 = rawnr.st2DParam.baynr_dgain[2];

    p2DCfg->sw_baynr_pix_diff = rawnr.st2DParam.baynr_pix_diff;
    p2DCfg->sw_baynr_diff_thld = rawnr.st2DParam.baynr_diff_thld;
    p2DCfg->sw_baynr_softthld = rawnr.st2DParam.baynr_softthld;

    p2DCfg->sw_bltflt_streng = rawnr.st2DParam.bltflt_streng;
    p2DCfg->sw_baynr_reg_w1 = rawnr.st2DParam.baynr_reg_w1;

    for(int i = 0; i < ISP21_BAYNR_XY_NUM; i++) {
        p2DCfg->sw_sigma_x[i] = rawnr.st2DParam.sigma_x[i];
        p2DCfg->sw_sigma_y[i] = rawnr.st2DParam.sigma_y[i];
    }

    p2DCfg->weit_d2 = rawnr.st2DParam.weit_d[2];
    p2DCfg->weit_d1 = rawnr.st2DParam.weit_d[1];
    p2DCfg->weit_d0 = rawnr.st2DParam.weit_d[0];


    //bayernr 3d

    p3DCfg->sw_bay3d_exp_sel = rawnr.st3DParam.bay3d_exp_sel;
    p3DCfg->sw_bay3d_bypass_en = rawnr.st3DParam.bay3d_bypass_en;
    p3DCfg->sw_bay3d_pk_en = rawnr.st3DParam.bay3d_pk_en;

    p3DCfg->sw_bay3d_softwgt = rawnr.st3DParam.bay3d_softwgt;
    p3DCfg->sw_bay3d_sigratio = rawnr.st3DParam.bay3d_sigratio;

    p3DCfg->sw_bay3d_glbpk2 = rawnr.st3DParam.bay3d_glbpk2;

    p3DCfg->sw_bay3d_exp_str = rawnr.st3DParam.bay3d_exp_str;
    p3DCfg->sw_bay3d_str = rawnr.st3DParam.bay3d_str;
    p3DCfg->sw_bay3d_wgtlmt_h = rawnr.st3DParam.bay3d_wgtlmt_h;
    p3DCfg->sw_bay3d_wgtlmt_l = rawnr.st3DParam.bay3d_wgtlmt_l;

    for(int i = 0; i < ISP21_BAY3D_XY_NUM; i++) {
        p3DCfg->sw_bay3d_sig_x[i] = rawnr.st3DParam.bay3d_sig_x[i];
        p3DCfg->sw_bay3d_sig_y[i] = rawnr.st3DParam.bay3d_sig_y[i];
    }

}
#endif
#if RKAIQ_HAVE_CNR_V1
void
Isp21Params::convertAiqUvnrToIsp21Params(struct isp21_isp_params_cfg& isp_cfg,
        rk_aiq_isp_cnr_v21_t& uvnr)
{
    struct isp21_cnr_cfg * pCfg = &isp_cfg.others.cnr_cfg;

    LOGD_ANR("%s:%d: enter\n", __FUNCTION__, __LINE__);

    isp_cfg.module_ens |= ISP2X_MODULE_CNR;
    isp_cfg.module_en_update |= ISP2X_MODULE_CNR;
    isp_cfg.module_cfg_update |= ISP2X_MODULE_CNR;

    pCfg->sw_cnr_thumb_mix_cur_en = uvnr.cnr_thumb_mix_cur_en;
    pCfg->sw_cnr_lq_bila_bypass = uvnr.cnr_lq_bila_bypass;
    pCfg->sw_cnr_hq_bila_bypass = uvnr.cnr_hq_bila_bypass;
    pCfg->sw_cnr_exgain_bypass = uvnr.cnr_exgain_bypass;

    if(uvnr.cnr_en_i == 0) {
        pCfg->sw_cnr_lq_bila_bypass = 0x01;
        pCfg->sw_cnr_hq_bila_bypass = 0x01;
        pCfg->sw_cnr_exgain_bypass = 0x01;
    }

    pCfg->sw_cnr_exgain_mux = uvnr.cnr_exgain_mux;
    pCfg->sw_cnr_gain_iso = uvnr.cnr_gain_iso;

    pCfg->sw_cnr_gain_offset = uvnr.cnr_gain_offset;
    pCfg->sw_cnr_gain_1sigma = uvnr.cnr_gain_1sigma;
    pCfg->sw_cnr_gain_uvgain1 = uvnr.cnr_gain_uvgain1;
    pCfg->sw_cnr_gain_uvgain0 = uvnr.cnr_gain_uvgain0;
    pCfg->sw_cnr_lmed3_alpha = uvnr.cnr_lmed3_alpha;
    pCfg->sw_cnr_lbf5_gain_y = uvnr.cnr_lbf5_gain_y;
    pCfg->sw_cnr_lbf5_gain_c = uvnr.cnr_lbf5_gain_c;

    pCfg->sw_cnr_lbf5_weit_d3 = uvnr.cnr_lbf5_weit_d[3];
    pCfg->sw_cnr_lbf5_weit_d2 = uvnr.cnr_lbf5_weit_d[2];
    pCfg->sw_cnr_lbf5_weit_d1 = uvnr.cnr_lbf5_weit_d[1];
    pCfg->sw_cnr_lbf5_weit_d0 = uvnr.cnr_lbf5_weit_d[0];
    pCfg->sw_cnr_lbf5_weit_d4 = uvnr.cnr_lbf5_weit_d[4];

    pCfg->sw_cnr_hmed3_alpha = uvnr.cnr_hmed3_alpha;
    pCfg->sw_cnr_hbf5_weit_src = uvnr.cnr_hbf5_weit_src;
    pCfg->sw_cnr_hbf5_min_wgt = uvnr.cnr_hbf5_min_wgt;
    pCfg->sw_cnr_hbf5_sigma = uvnr.cnr_hbf5_sigma;
    pCfg->sw_cnr_lbf5_weit_src = uvnr.cnr_lbf5_weit_src;
    pCfg->sw_cnr_lbf3_sigma = uvnr.cnr_lbf3_sigma;

}
#endif
#if RKAIQ_HAVE_YNR_V2
void
Isp21Params::convertAiqYnrToIsp21Params(struct isp21_isp_params_cfg& isp_cfg,
                                        rk_aiq_isp_ynr_v21_t& ynr)
{
    struct isp21_ynr_cfg * pCfg = &isp_cfg.others.ynr_cfg;

    LOGD_ANR("%s:%d: enter\n", __FUNCTION__, __LINE__);

    isp_cfg.module_ens |= ISP2X_MODULE_YNR;
    isp_cfg.module_en_update |= ISP2X_MODULE_YNR;
    isp_cfg.module_cfg_update |= ISP2X_MODULE_YNR;

    pCfg->sw_ynr_thumb_mix_cur_en = ynr.ynr_thumb_mix_cur_en;
    pCfg->sw_ynr_global_gain_alpha = ynr.ynr_global_gain_alpha;
    pCfg->sw_ynr_global_gain = ynr.ynr_global_gain;
    pCfg->sw_ynr_flt1x1_bypass_sel = ynr.ynr_flt1x1_bypass_sel;

    pCfg->sw_ynr_sft5x5_bypass = ynr.ynr_sft5x5_bypass;
    pCfg->sw_ynr_flt1x1_bypass = ynr.ynr_flt1x1_bypass;
    pCfg->sw_ynr_lgft3x3_bypass = ynr.ynr_lgft3x3_bypass;
    pCfg->sw_ynr_lbft5x5_bypass = ynr.ynr_lbft5x5_bypass;
    pCfg->sw_ynr_bft3x3_bypass = ynr.ynr_bft3x3_bypass;
    if(ynr.ynr_en == 0) {
        pCfg->sw_ynr_sft5x5_bypass = 0x01;
        pCfg->sw_ynr_flt1x1_bypass = 0x01;
        pCfg->sw_ynr_lgft3x3_bypass = 0x01;
        pCfg->sw_ynr_lbft5x5_bypass = 0x01;
        pCfg->sw_ynr_bft3x3_bypass = 0x01;
    }

    pCfg->sw_ynr_rnr_max_r = ynr.ynr_rnr_max_r;
    pCfg->sw_ynr_low_bf_inv1 = ynr.ynr_low_bf_inv[1];
    pCfg->sw_ynr_low_bf_inv0 = ynr.ynr_low_bf_inv[0];
    pCfg->sw_ynr_low_peak_supress = ynr.ynr_low_peak_supress;
    pCfg->sw_ynr_low_thred_adj = ynr.ynr_low_thred_adj;
    pCfg->sw_ynr_low_dist_adj = ynr.ynr_low_dist_adj;

    pCfg->sw_ynr_low_edge_adj_thresh = ynr.ynr_low_edge_adj_thresh;
    pCfg->sw_ynr_low_bi_weight = ynr.ynr_low_bi_weight;
    pCfg->sw_ynr_low_weight = ynr.ynr_low_weight;
    pCfg->sw_ynr_low_center_weight = ynr.ynr_low_center_weight;
    pCfg->sw_ynr_hi_min_adj = ynr.ynr_hi_min_adj;
    pCfg->sw_ynr_high_thred_adj = ynr.ynr_high_thred_adj;

    pCfg->sw_ynr_high_retain_weight = ynr.ynr_high_retain_weight;
    pCfg->sw_ynr_hi_edge_thed = ynr.ynr_hi_edge_thed;

    pCfg->sw_ynr_base_filter_weight2 = ynr.ynr_base_filter_weight[2];
    pCfg->sw_ynr_base_filter_weight1 = ynr.ynr_base_filter_weight[1];
    pCfg->sw_ynr_base_filter_weight0 = ynr.ynr_base_filter_weight[0];

    pCfg->sw_ynr_low_gauss1_coeff2 = ynr.ynr_low_gauss1_coeff[2];
    pCfg->sw_ynr_low_gauss1_coeff1 = ynr.ynr_low_gauss1_coeff[1];
    pCfg->sw_ynr_low_gauss1_coeff0 = ynr.ynr_low_gauss1_coeff[0];

    pCfg->sw_ynr_low_gauss2_coeff2 = ynr.ynr_low_gauss2_coeff[2];
    pCfg->sw_ynr_low_gauss2_coeff1 = ynr.ynr_low_gauss2_coeff[1];
    pCfg->sw_ynr_low_gauss2_coeff0 = ynr.ynr_low_gauss2_coeff[0];

    pCfg->sw_ynr_direction_weight3 = ynr.ynr_direction_weight[3];
    pCfg->sw_ynr_direction_weight2 = ynr.ynr_direction_weight[2];
    pCfg->sw_ynr_direction_weight1 = ynr.ynr_direction_weight[1];
    pCfg->sw_ynr_direction_weight0 = ynr.ynr_direction_weight[0];

    pCfg->sw_ynr_direction_weight7 = ynr.ynr_direction_weight[7];
    pCfg->sw_ynr_direction_weight6 = ynr.ynr_direction_weight[6];
    pCfg->sw_ynr_direction_weight5 = ynr.ynr_direction_weight[5];
    pCfg->sw_ynr_direction_weight4 = ynr.ynr_direction_weight[4];

    for(int i = 0; i < ISP21_YNR_XY_NUM; i++) {
        pCfg->sw_ynr_luma_points_x[i] = ynr.ynr_luma_points_x[i];
        pCfg->sw_ynr_lsgm_y[i] = ynr.ynr_lsgm_y[i];
        pCfg->sw_ynr_hsgm_y[i] = ynr.ynr_hsgm_y[i];
        pCfg->sw_ynr_rnr_strength3[i] = ynr.ynr_rnr_strength[i];
    }

    LOGD_ANR("%s:%d: exit\n", __FUNCTION__, __LINE__);

}
#endif
#if RKAIQ_HAVE_SHARP_V3
void
Isp21Params::convertAiqSharpenToIsp21Params(struct isp21_isp_params_cfg& isp_cfg,
        rk_aiq_isp_sharp_v21_t& sharp)
{
    struct isp21_sharp_cfg * pCfg = &isp_cfg.others.sharp_cfg;

    LOGD_ASHARP("%s:%d: enter\n", __FUNCTION__, __LINE__);

    isp_cfg.module_ens |= ISP2X_MODULE_SHARP;
    isp_cfg.module_en_update |= ISP2X_MODULE_SHARP;
    isp_cfg.module_cfg_update |= ISP2X_MODULE_SHARP;

    pCfg->sw_sharp_bypass = sharp.sharp_bypass;
    if(sharp.sharp_en == 0) {
        pCfg->sw_sharp_bypass = 0x01;
    }

    pCfg->sw_sharp_sharp_ratio = sharp.sharp_sharp_ratio;
    pCfg->sw_sharp_bf_ratio = sharp.sharp_bf_ratio;
    pCfg->sw_sharp_gaus_ratio = sharp.sharp_gaus_ratio;
    pCfg->sw_sharp_pbf_ratio = sharp.sharp_pbf_ratio;

    for(int i = 0; i < ISP21_SHARP_X_NUM; i++) {
        pCfg->sw_sharp_luma_dx[i] = sharp.sharp_luma_dx[i];
    }

    for(int i = 0; i < ISP21_SHARP_Y_NUM; i++) {
        pCfg->sw_sharp_pbf_sigma_inv[i] = sharp.sharp_pbf_sigma_inv[i];
    }

    for(int i = 0; i < ISP21_SHARP_Y_NUM; i++) {
        pCfg->sw_sharp_bf_sigma_inv[i] = sharp.sharp_bf_sigma_inv[i];
    }

    pCfg->sw_sharp_bf_sigma_shift = sharp.sharp_bf_sigma_shift;
    pCfg->sw_sharp_pbf_sigma_shift = sharp.sharp_pbf_sigma_shift;

    for(int i = 0; i < ISP21_SHARP_Y_NUM; i++) {
        pCfg->sw_sharp_ehf_th[i] = sharp.sharp_ehf_th[i];
    }

    for(int i = 0; i < ISP21_SHARP_Y_NUM; i++) {
        pCfg->sw_sharp_clip_hf[i] = sharp.sharp_clip_hf[i];
    }

    pCfg->sw_sharp_pbf_coef_2 = sharp.sharp_pbf_coef[2];
    pCfg->sw_sharp_pbf_coef_1 = sharp.sharp_pbf_coef[1];
    pCfg->sw_sharp_pbf_coef_0 = sharp.sharp_pbf_coef[0];

    pCfg->sw_sharp_bf_coef_2 = sharp.sharp_bf_coef[2];
    pCfg->sw_sharp_bf_coef_1 = sharp.sharp_bf_coef[1];
    pCfg->sw_sharp_bf_coef_0 = sharp.sharp_bf_coef[0];

    pCfg->sw_sharp_gaus_coef_2 = sharp.sharp_gaus_coef[2];
    pCfg->sw_sharp_gaus_coef_1 = sharp.sharp_gaus_coef[1];
    pCfg->sw_sharp_gaus_coef_0 = sharp.sharp_gaus_coef[0];

    LOGD_ASHARP("%s:%d: exit\n", __FUNCTION__, __LINE__);

}
#endif
#if RKAIQ_HAVE_DRC_V10
void
Isp21Params::convertAiqDrcToIsp21Params(struct isp21_isp_params_cfg& isp_cfg,
                                        rk_aiq_isp_drc_v21_t& adrc_data)
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

    isp_cfg.others.drc_cfg.sw_drc_offset_pow2   = adrc_data.DrcProcRes.Drc_v10.sw_drc_offset_pow2;
    isp_cfg.others.drc_cfg.sw_drc_compres_scl   = adrc_data.DrcProcRes.Drc_v10.sw_drc_compres_scl;
    isp_cfg.others.drc_cfg.sw_drc_position      = adrc_data.DrcProcRes.Drc_v10.sw_drc_position;
    isp_cfg.others.drc_cfg.sw_drc_delta_scalein = adrc_data.DrcProcRes.Drc_v10.sw_drc_delta_scalein;
    isp_cfg.others.drc_cfg.sw_drc_hpdetail_ratio =
        adrc_data.DrcProcRes.Drc_v10.sw_drc_hpdetail_ratio;
    isp_cfg.others.drc_cfg.sw_drc_lpdetail_ratio =
        adrc_data.DrcProcRes.Drc_v10.sw_drc_lpdetail_ratio;
    isp_cfg.others.drc_cfg.sw_drc_weicur_pix   = adrc_data.DrcProcRes.Drc_v10.sw_drc_weicur_pix;
    isp_cfg.others.drc_cfg.sw_drc_weipre_frame = adrc_data.DrcProcRes.Drc_v10.sw_drc_weipre_frame;
    isp_cfg.others.drc_cfg.sw_drc_force_sgm_inv0 =
        adrc_data.DrcProcRes.Drc_v10.sw_drc_force_sgm_inv0;
    isp_cfg.others.drc_cfg.sw_drc_motion_scl = adrc_data.DrcProcRes.Drc_v10.sw_drc_motion_scl;
    isp_cfg.others.drc_cfg.sw_drc_edge_scl   = adrc_data.DrcProcRes.Drc_v10.sw_drc_edge_scl;
    isp_cfg.others.drc_cfg.sw_drc_space_sgm_inv1 =
        adrc_data.DrcProcRes.Drc_v10.sw_drc_space_sgm_inv1;
    isp_cfg.others.drc_cfg.sw_drc_space_sgm_inv0 =
        adrc_data.DrcProcRes.Drc_v10.sw_drc_space_sgm_inv0;
    isp_cfg.others.drc_cfg.sw_drc_range_sgm_inv1 =
        adrc_data.DrcProcRes.Drc_v10.sw_drc_range_sgm_inv1;
    isp_cfg.others.drc_cfg.sw_drc_range_sgm_inv0 =
        adrc_data.DrcProcRes.Drc_v10.sw_drc_range_sgm_inv0;
    isp_cfg.others.drc_cfg.sw_drc_weig_maxl  = adrc_data.DrcProcRes.Drc_v10.sw_drc_weig_maxl;
    isp_cfg.others.drc_cfg.sw_drc_weig_bilat = adrc_data.DrcProcRes.Drc_v10.sw_drc_weig_bilat;
    isp_cfg.others.drc_cfg.sw_drc_iir_weight = adrc_data.DrcProcRes.Drc_v10.sw_drc_iir_weight;
    isp_cfg.others.drc_cfg.sw_drc_min_ogain  = adrc_data.DrcProcRes.Drc_v10.sw_drc_min_ogain;

    for(int i = 0; i < 17; i++) {
        isp_cfg.others.drc_cfg.sw_drc_gain_y[i] = adrc_data.DrcProcRes.Drc_v10.sw_drc_gain_y[i];
        isp_cfg.others.drc_cfg.sw_drc_compres_y[i] =
            adrc_data.DrcProcRes.Drc_v10.sw_drc_compres_y[i];
        isp_cfg.others.drc_cfg.sw_drc_scale_y[i] = adrc_data.DrcProcRes.Drc_v10.sw_drc_scale_y[i];
    }

#if 0
    LOGE_CAMHW_SUBM(ISP20PARAM_SUBM, "%d: sw_drc_offset_pow2 %d", __LINE__, isp_cfg.others.drc_cfg.sw_drc_offset_pow2);
    LOGE_CAMHW_SUBM(ISP20PARAM_SUBM, "sw_drc_offset_pow2 %d", isp_cfg.others.drc_cfg.sw_drc_offset_pow2);
    LOGE_CAMHW_SUBM(ISP20PARAM_SUBM, "sw_drc_compres_scl %d", isp_cfg.others.drc_cfg.sw_drc_compres_scl);
    LOGE_CAMHW_SUBM(ISP20PARAM_SUBM, "sw_drc_position %d", isp_cfg.others.drc_cfg.sw_drc_position);
    LOGE_CAMHW_SUBM(ISP20PARAM_SUBM, "sw_drc_delta_scalein %d", isp_cfg.others.drc_cfg.sw_drc_delta_scalein);
    LOGE_CAMHW_SUBM(ISP20PARAM_SUBM, "sw_drc_hpdetail_ratio %d", isp_cfg.others.drc_cfg.sw_drc_hpdetail_ratio);
    LOGE_CAMHW_SUBM(ISP20PARAM_SUBM, "sw_drc_lpdetail_ratio %d", isp_cfg.others.drc_cfg.sw_drc_lpdetail_ratio);
    LOGE_CAMHW_SUBM(ISP20PARAM_SUBM, "sw_drc_weicur_pix %d", isp_cfg.others.drc_cfg.sw_drc_weicur_pix);

#endif
}
#endif

template<class T>
void Isp21Params::convertAiqAgicToIsp21Params(T& isp_cfg,
        const rk_aiq_isp_gic_v21_t& agic)
{
    bool enable = agic.gic_en;
    if (enable) {
        isp_cfg.module_en_update |= 1LL << RK_ISP2X_GIC_ID;
        isp_cfg.module_ens |= 1LL << RK_ISP2X_GIC_ID;
        isp_cfg.module_cfg_update |= 1LL << RK_ISP2X_GIC_ID;
    } else {
        isp_cfg.module_en_update |= 1LL << RK_ISP2X_GIC_ID;
        isp_cfg.module_ens &= ~(1LL << RK_ISP2X_GIC_ID);
        isp_cfg.module_cfg_update &= ~(1LL << RK_ISP2X_GIC_ID);
    }

    isp_cfg.others.gic_cfg.regmingradthrdark2 = agic.ProcResV21.regmingradthrdark2;
    isp_cfg.others.gic_cfg.regmingradthrdark1 = agic.ProcResV21.regmingradthrdark1;
    isp_cfg.others.gic_cfg.regminbusythre     = agic.ProcResV21.regminbusythre;
    isp_cfg.others.gic_cfg.regdarkthre        = agic.ProcResV21.regdarkthre;

    isp_cfg.others.gic_cfg.regmaxcorvboth        = agic.ProcResV21.regmaxcorvboth;
    isp_cfg.others.gic_cfg.regdarktthrehi        = agic.ProcResV21.regdarktthrehi;
    isp_cfg.others.gic_cfg.regkgrad2dark         = agic.ProcResV21.regkgrad2dark;
    isp_cfg.others.gic_cfg.regkgrad1dark         = agic.ProcResV21.regkgrad1dark;
    isp_cfg.others.gic_cfg.regstrengthglobal_fix = agic.ProcResV21.regstrengthglobal_fix;
    isp_cfg.others.gic_cfg.regdarkthrestep       = agic.ProcResV21.regdarkthrestep;
    isp_cfg.others.gic_cfg.regkgrad2             = agic.ProcResV21.regkgrad2;
    isp_cfg.others.gic_cfg.regkgrad1             = agic.ProcResV21.regkgrad1;
    isp_cfg.others.gic_cfg.reggbthre             = agic.ProcResV21.reggbthre;

    isp_cfg.others.gic_cfg.regmaxcorv     = agic.ProcResV21.regmaxcorv;
    isp_cfg.others.gic_cfg.regmingradthr2 = agic.ProcResV21.regmingradthr2;
    isp_cfg.others.gic_cfg.regmingradthr1 = agic.ProcResV21.regmingradthr1;
    isp_cfg.others.gic_cfg.gr_ratio       = agic.ProcResV21.gr_ratio;
    isp_cfg.others.gic_cfg.noise_scale    = agic.ProcResV21.noise_scale;
    isp_cfg.others.gic_cfg.noise_base     = agic.ProcResV21.noise_base;
    isp_cfg.others.gic_cfg.diff_clip      = agic.ProcResV21.diff_clip;
    for (int i = 0; i < 15; i++) isp_cfg.others.gic_cfg.sigma_y[i] = agic.ProcResV21.sigma_y[i];
}

template<class T>
void
Isp21Params::convertAiqCsmToIsp21Params(T& isp_cfg,
                                        const rk_aiq_acsm_params_t& csm_param)
{
    struct isp21_csm_cfg* csm_cfg = &isp_cfg.others.csm_cfg;
    if (csm_param.op_mode == RK_AIQ_OP_MODE_MANUAL ||
        csm_param.op_mode == RK_AIQ_OP_MODE_AUTO) {
        isp_cfg.module_ens |= ISP2X_MODULE_CSM;
        isp_cfg.module_en_update |= ISP2X_MODULE_CSM;
        isp_cfg.module_cfg_update |= ISP2X_MODULE_CSM;
        csm_cfg->csm_full_range = csm_param.full_range ? 1 : 0;
        csm_cfg->csm_y_offset = csm_param.y_offset;
        csm_cfg->csm_c_offset = csm_param.c_offset;
        for (int i = 0; i < RK_AIQ_CSM_COEFF_NUM; i++) {
            csm_cfg->csm_coeff[i] = csm_param.coeff[i] > 0
                                    ? (short)(csm_param.coeff[i] * 128 + 0.5)
                                    : (short)(csm_param.coeff[i] * 128 - 0.5);
        }
    } else {
        isp_cfg.module_ens &= ~ISP2X_MODULE_CSM;
        isp_cfg.module_en_update |= ISP2X_MODULE_CSM;
        isp_cfg.module_cfg_update &= ~(ISP2X_MODULE_CSM);
    }
}
#if RKAIQ_HAVE_CGC_V1
template <class T>
void Isp21Params::convertAiqCgcToIsp21Params(T& isp_cfg,
        const rk_aiq_acgc_params_t& cgc_param) {
    struct isp21_cgc_cfg* cgc_cfg = &isp_cfg.others.cgc_cfg;
    if (cgc_param.op_mode == RK_AIQ_OP_MODE_MANUAL ||
            cgc_param.op_mode == RK_AIQ_OP_MODE_AUTO) {
        isp_cfg.module_ens |= ISP2X_MODULE_CGC;
        isp_cfg.module_en_update |= ISP2X_MODULE_CGC;
        isp_cfg.module_cfg_update |= ISP2X_MODULE_CGC;
        cgc_cfg->ratio_en = cgc_param.cgc_ratio_en;
        cgc_cfg->yuv_limit = cgc_param.cgc_yuv_limit;
    } else {
        isp_cfg.module_ens &= ~ISP2X_MODULE_CGC;
        isp_cfg.module_en_update |= ISP2X_MODULE_CGC;
        isp_cfg.module_cfg_update &= ~(ISP2X_MODULE_CGC);
    }
}
#endif

#define ISP3X_WBGAIN_INTSCALE_BIT  8

void Isp21Params::convertAiqExpIspDgainToIspParams(void* isp_cfg_, RKAiqAecExpInfo_t ae_exp)
{
#if defined(ISP_HW_V30) || defined(ISP_HW_V21)
#if defined(ISP_HW_V30)
    struct isp3x_isp_params_cfg* isp_cfg = (struct isp3x_isp_params_cfg*)isp_cfg_;
    uint16_t max_wb_gain = (1 << (ISP2X_WBGAIN_FIXSCALE_BIT + ISP3X_WBGAIN_INTSCALE_BIT)) - 1;
#elif defined(ISP_HW_V21)
    struct isp21_isp_params_cfg* isp_cfg = (struct isp21_isp_params_cfg*)isp_cfg_;
    uint16_t max_wb_gain = (1 << (ISP2X_WBGAIN_FIXSCALE_BIT + ISP2X_WBGAIN_INTSCALE_BIT)) - 1;
#else
   LOGE("not suppport !");
#endif
    struct isp21_awb_gain_cfg *  dest_cfg = &isp_cfg->others.awb_gain_cfg;
    struct isp21_awb_gain_cfg *  cfg = &mLatestWbGainCfg;

    if(_working_mode == RK_AIQ_WORKING_MODE_NORMAL) {

        float isp_dgain = ae_exp.LinearExp.exp_real_params.isp_dgain;
        isp_dgain = isp_dgain >= 1.0f ?  isp_dgain : 1.0f;
        LOGD_CAMHW_SUBM(ISP20PARAM_SUBM,"cid: %d, isp_dgain:%0.3f\n", _CamPhyId, isp_dgain);
        bool isRecalc = true;
        if (isp_dgain < 1.0000001f)
            isRecalc = false;

        if (isRecalc) {
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
            isp_cfg->module_cfg_update |= 1LL << RK_ISP2X_AWB_GAIN_ID;
        }

        if (mBlcResult) {
            uint16_t base_wb_gain = 1 << ISP2X_WBGAIN_FIXSCALE_BIT;
            int32_t tmp = 0;
            RkAiqIspBlcParamsProxy* blc_proxy = dynamic_cast<RkAiqIspBlcParamsProxy*>(mBlcResult);
            rk_aiq_isp_blc_v21_t& blc = blc_proxy->data()->result;

            if (isRecalc) {
                tmp = (uint16_t)(blc.v0.blc1_r * cfg->gain0_red / base_wb_gain + 0.5);
                if(tmp > 0x1fff)
                    tmp = 0x1fff;
                if(tmp < 0)
                    tmp = 0;

                isp_cfg->others.bls_cfg.bls1_val.r = tmp;

                tmp = (uint16_t)(blc.v0.blc1_gr * cfg->gain0_green_r / base_wb_gain + 0.5);
                if(tmp > 0x1fff)
                    tmp = 0x1fff;
                if(tmp < 0)
                    tmp = 0;
                isp_cfg->others.bls_cfg.bls1_val.gr = tmp;

                tmp = (uint16_t)(blc.v0.blc1_gb * cfg->gain0_green_b / base_wb_gain + 0.5);
                if(tmp > 0x1fff)
                    tmp = 0x1fff;
                if(tmp < 0)
                    tmp = 0;
                isp_cfg->others.bls_cfg.bls1_val.gb = tmp;

                tmp = (uint16_t)(blc.v0.blc1_b * cfg->gain0_blue / base_wb_gain + 0.5);
                if(tmp > 0x1fff)
                    tmp = 0x1fff;
                if(tmp < 0)
                    tmp = 0;
                isp_cfg->others.bls_cfg.bls1_val.b = tmp;
                isp_cfg->module_cfg_update |= 1LL << RK_ISP2X_BLS_ID;
            }
            #define MAX_LSC_VALUE 8191
#if defined (RKAIQ_HAVE_LSC_V2) || defined (RKAIQ_HAVE_LSC_V3)
            struct isp3x_lsc_cfg *  cfg = &isp_cfg->others.lsc_cfg;
            cfg->sector_16x16 = true;
#else
            struct isp2x_lsc_cfg *  cfg = &isp_cfg->others.lsc_cfg;
#endif
            struct isp21_bls_cfg &bls_cfg = isp_cfg->others.bls_cfg;
            if(bls_cfg.bls1_en && bls_cfg.bls1_val.b >0 && bls_cfg.bls1_val.r>0
                && bls_cfg.bls1_val.gb >0 && bls_cfg.bls1_val.gr>0 ){
                if(_lsc_en){
                    for(int i=0;i<ISP3X_LSC_DATA_TBL_SIZE;i++){
                        cfg->b_data_tbl[i] = cfg->b_data_tbl[i]*((1 << ISP2X_BLC_BIT_MAX) - 1) / ((1 << ISP2X_BLC_BIT_MAX) - 1 - bls_cfg.bls1_val.b);
                        cfg->b_data_tbl[i] = MIN(cfg->b_data_tbl[i],MAX_LSC_VALUE);
                        cfg->gb_data_tbl[i] = cfg->gb_data_tbl[i]*((1 << ISP2X_BLC_BIT_MAX) - 1) / ((1 << ISP2X_BLC_BIT_MAX) - 1 - bls_cfg.bls1_val.gb);
                        cfg->gb_data_tbl[i] = MIN(cfg->gb_data_tbl[i],MAX_LSC_VALUE);
                        cfg->r_data_tbl[i] = cfg->r_data_tbl[i]*((1 << ISP2X_BLC_BIT_MAX) - 1) / ((1 << ISP2X_BLC_BIT_MAX) - 1 - bls_cfg.bls1_val.r);
                        cfg->r_data_tbl[i] = MIN(cfg->r_data_tbl[i],MAX_LSC_VALUE);
                        cfg->gr_data_tbl[i] = cfg->gr_data_tbl[i]*((1 << ISP2X_BLC_BIT_MAX) - 1) / ((1 << ISP2X_BLC_BIT_MAX) - 1 - bls_cfg.bls1_val.gr);
                        cfg->gr_data_tbl[i] = MIN(cfg->gr_data_tbl[i],MAX_LSC_VALUE);
                    }
                    isp_cfg->module_cfg_update |= ISP2X_MODULE_LSC;
                }else{
                    isp_cfg->module_ens |= ISP2X_MODULE_LSC; //force open lsc
                    isp_cfg->module_en_update |= ISP2X_MODULE_LSC;
                    isp_cfg->module_cfg_update |= ISP2X_MODULE_LSC;
                    for(int i=0;i<ISP3X_LSC_DATA_TBL_SIZE;i++){
                        cfg->b_data_tbl[i] = 1024*((1 << ISP2X_BLC_BIT_MAX) - 1) / ((1 << ISP2X_BLC_BIT_MAX) - 1 - bls_cfg.bls1_val.b);
                        cfg->b_data_tbl[i] = MIN(cfg->b_data_tbl[i],MAX_LSC_VALUE);
                        cfg->gb_data_tbl[i] = 1024*((1 << ISP2X_BLC_BIT_MAX) - 1) / ((1 << ISP2X_BLC_BIT_MAX) - 1 - bls_cfg.bls1_val.gb);
                        cfg->gb_data_tbl[i] = MIN(cfg->gb_data_tbl[i],MAX_LSC_VALUE);
                        cfg->r_data_tbl[i] = 1024*((1 << ISP2X_BLC_BIT_MAX) - 1) / ((1 << ISP2X_BLC_BIT_MAX) - 1 - bls_cfg.bls1_val.r);
                        cfg->r_data_tbl[i] = MIN(cfg->r_data_tbl[i],MAX_LSC_VALUE);
                        cfg->gr_data_tbl[i] = 1024*((1 << ISP2X_BLC_BIT_MAX) - 1) / ((1 << ISP2X_BLC_BIT_MAX) - 1 - bls_cfg.bls1_val.gr);
                        cfg->gr_data_tbl[i] = MIN(cfg->gr_data_tbl[i],MAX_LSC_VALUE);
                    }
                }
            }
        }

    } else {

        float isp_dgain0 = ae_exp.HdrExp[0].exp_real_params.isp_dgain;
        float isp_dgain1 = ae_exp.HdrExp[1].exp_real_params.isp_dgain;
        float isp_dgain2 = ae_exp.HdrExp[2].exp_real_params.isp_dgain;

        isp_dgain0 = isp_dgain0 >= 1.0f ?  isp_dgain0 : 1.0f;
        isp_dgain1 = isp_dgain1 >= 1.0f ?  isp_dgain1 : 1.0f;
        isp_dgain2 = isp_dgain2 >= 1.0f ?  isp_dgain2 : 1.0f;

        LOGD_CAMHW_SUBM(ISP20PARAM_SUBM,"cid: %d, isp_dgain:%0.3f,%0.3f,%0.3f\n",
                        _CamPhyId, isp_dgain0, isp_dgain1, isp_dgain2);
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
        isp_cfg->module_cfg_update |= 1LL << RK_ISP2X_AWB_GAIN_ID;

    }
#else
    // not support, do nothing
#endif
}

bool Isp21Params::convert3aResultsToIspCfg(SmartPtr<cam3aResult> &result,
        void* isp_cfg_p, bool is_multi_isp)
{
    struct isp21_isp_params_cfg& isp_cfg = *(struct isp21_isp_params_cfg*)isp_cfg_p;

    if (result.ptr() == NULL) {
        LOGE_CAMHW_SUBM(ISP20PARAM_SUBM, "3A result empty");
        return false;
    }

    int32_t type = result->getType();
    // LOGE_CAMHW_SUBM(ISP20PARAM_SUBM, "%s, module (0x%x) convert params!\n", __FUNCTION__, type);
    switch (type)
    {
    // followings are specific for isp21
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
            convertAiqAwbToIsp21Params(isp_cfg, params->data()->result, true);
#endif
    }
    break;
    case RESULT_TYPE_CCM_PARAM:
    {
        RkAiqIspCcmParamsProxy* params = result.get_cast_ptr<RkAiqIspCcmParamsProxy>();
#if RKAIQ_HAVE_CCM_V1
        if (params)
            convertAiqCcmToIsp21Params(isp_cfg, params->data()->result);
#endif
    }
    break;
    case RESULT_TYPE_DRC_PARAM:
    {
#if RKAIQ_HAVE_DRC_V10
        RkAiqIspDrcParamsProxy* params = result.get_cast_ptr<RkAiqIspDrcParamsProxy>();
        if (params)
            convertAiqDrcToIsp21Params(isp_cfg, params->data()->result);
#endif
    }
    break;
    case RESULT_TYPE_BLC_PARAM:
    {
#if RKAIQ_HAVE_BLC_V1
        RkAiqIspBlcParamsProxy* params = result.get_cast_ptr<RkAiqIspBlcParamsProxy>();
        if (params)
            convertAiqBlcToIsp21Params(isp_cfg, params->data()->result);
#endif
    }
    break;
    case RESULT_TYPE_RAWNR_PARAM:
    {
#if RKAIQ_HAVE_BAYERNR_V2
        RkAiqIspBaynrParamsProxy* params = result.get_cast_ptr<RkAiqIspBaynrParamsProxy>();
        if (params)
            convertAiqRawnrToIsp21Params(isp_cfg, params->data()->result);
#endif
    }
    break;
    case RESULT_TYPE_YNR_PARAM:
    {
#if RKAIQ_HAVE_YNR_V2
        RkAiqIspYnrParamsProxy* params = result.get_cast_ptr<RkAiqIspYnrParamsProxy>();
        if (params)
            convertAiqYnrToIsp21Params(isp_cfg, params->data()->result);
#endif
    }
    break;
    case RESULT_TYPE_UVNR_PARAM:
    {
#if RKAIQ_HAVE_CNR_V1
        RkAiqIspCnrParamsProxy* params = result.get_cast_ptr<RkAiqIspCnrParamsProxy>();
        if (params)
            convertAiqUvnrToIsp21Params(isp_cfg, params->data()->result);
#endif
    }
    break;
    case RESULT_TYPE_SHARPEN_PARAM:
    {
#if RKAIQ_HAVE_SHARP_V3
        RkAiqIspSharpenParamsProxy* params = result.get_cast_ptr<RkAiqIspSharpenParamsProxy>();
        if (params)
            convertAiqSharpenToIsp21Params(isp_cfg, params->data()->result);
#endif
    }
    break;
    case RESULT_TYPE_DEHAZE_PARAM:
    {
#if RKAIQ_HAVE_DEHAZE_V11
        RkAiqIspDehazeParamsProxy* params = result.get_cast_ptr<RkAiqIspDehazeParamsProxy>();
        if (params)
            convertAiqAdehazeToIsp21Params(isp_cfg, params->data()->result);
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
    // followings are the same as isp20
    case RESULT_TYPE_AEC_PARAM:
    {
#ifdef ISP_HW_V21
        RkAiqIspAecParamsProxy* params = result.get_cast_ptr<RkAiqIspAecParamsProxy>();
        if (params) {
            convertAiqAeToIsp20Params(isp_cfg, params->data()->result);
        }
#endif
    }
    break;
    case RESULT_TYPE_HIST_PARAM:
    {
#ifdef ISP_HW_V21
        RkAiqIspHistParamsProxy* params = result.get_cast_ptr<RkAiqIspHistParamsProxy>();
        if (params)
            convertAiqHistToIsp20Params(isp_cfg, params->data()->result);
#endif
    }
    break;
    case RESULT_TYPE_AF_PARAM:
    {
#if RKAIQ_HAVE_AF_V20 || RKAIQ_ONLY_AF_STATS_V20
        RkAiqIspAfParamsProxy* params = result.get_cast_ptr<RkAiqIspAfParamsProxy>();
        if (params)
            convertAiqAfToIsp20Params(isp_cfg, params->data()->result, true);
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
    case RESULT_TYPE_MERGE_PARAM:
    {
#if RKAIQ_HAVE_MERGE_V10
        RkAiqIspMergeParamsProxy* params = result.get_cast_ptr<RkAiqIspMergeParamsProxy>();
        if (params) {
            convertAiqMergeToIsp20Params(isp_cfg, params->data()->result);
        }
#endif
    }
    break;
    case RESULT_TYPE_LSC_PARAM:
    {
#ifdef RKAIQ_HAVE_LSC_V1
        RkAiqIspLscParamsProxy* params = result.get_cast_ptr<RkAiqIspLscParamsProxy>();
        if (params)
            convertAiqLscToIsp20Params(isp_cfg, params->data()->result);
#endif
    }
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
    {
#if RKAIQ_HAVE_LDCH_V10
        RkAiqIspLdchParamsProxy* params = result.get_cast_ptr<RkAiqIspLdchParamsProxy>();
        if (params)
            convertAiqAldchToIsp20Params(isp_cfg, params->data()->result);
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
    case RESULT_TYPE_AGAMMA_PARAM:
    {
#if RKAIQ_HAVE_GAMMA_V10
        RkAiqIspAgammaParamsProxy* params = result.get_cast_ptr<RkAiqIspAgammaParamsProxy>();
        if (params)
            convertAiqAgammaToIsp20Params(isp_cfg, params->data()->result);
#endif
    }
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
    case RESULT_TYPE_CGC_PARAM: {
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
    default:
        LOGE("unknown param type: 0x%x!", type);
        return false;
    }

    return true;
}

} //namspace RkCam
