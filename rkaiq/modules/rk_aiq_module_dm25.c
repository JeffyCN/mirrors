#include "rk_aiq_isp39_modules.h"

void rk_aiq_dm25_params_cvt(void* attr, isp_params_t* isp_params, common_cvt_info_t *cvtinfo)
{
    struct isp35_debayer_cfg *phwcfg = &isp_params->isp_cfg->others.debayer_cfg;
    dm_param_t *dm_param = (dm_param_t *) attr;
    dm_params_dyn_t *pdyn = &dm_param->dyn;
    dm_params_static_t *psta = &dm_param->sta;

    /* CONTROL */
    phwcfg->g_out_flt_en = pdyn->hw_dmT_gOutlsFlt_en;
    phwcfg->cnt_flt_en = pdyn->cFlt.hw_dmT_cnrFlt_en;

    /* LUMA_DX */
    phwcfg->luma_dx[0] = 8;
    phwcfg->luma_dx[1] = 8;
    phwcfg->luma_dx[2] = 9;
    phwcfg->luma_dx[3] = 9;
    phwcfg->luma_dx[4] = 10;
    phwcfg->luma_dx[5] = 10;
    phwcfg->luma_dx[6] = 9;

    /* G_INTERP */
    phwcfg->g_interp_clip_en = pdyn->gInterp.hw_dmT_gInterpClip_en;
    phwcfg->hi_texture_thred  = pdyn->gDrctAlpha.hw_dmT_hiTexture_thred;
    phwcfg->hi_drct_thred       = pdyn->gDrctAlpha.hw_dmT_hiDrct_thred;
    phwcfg->lo_drct_thred       = pdyn->gDrctAlpha.hw_dmT_loDrct_thred;
    phwcfg->drct_method_thred = pdyn->gDrctAlpha.hw_dmT_drctMethod_thred;
    phwcfg->g_interp_sharp_strg_max_limit   = pdyn->gInterp.hw_dmT_gInterpSharpStrg_maxLim;

    /* G_INTERP_FILTER1 */
    phwcfg->lo_drct_flt_coeff1 = psta->gInterp.hw_dmT_loDrctFlt_coeff[0];
    phwcfg->lo_drct_flt_coeff2 = psta->gInterp.hw_dmT_loDrctFlt_coeff[1];
    phwcfg->lo_drct_flt_coeff3 = psta->gInterp.hw_dmT_loDrctFlt_coeff[2];
    phwcfg->lo_drct_flt_coeff4 = psta->gInterp.hw_dmT_loDrctFlt_coeff[3];

    /* G_INTERP_FILTER2 */
    phwcfg->hi_drct_flt_coeff1 = psta->gInterp.hw_dmT_hiDrctFlt_coeff[0];
    phwcfg->hi_drct_flt_coeff2 = psta->gInterp.hw_dmT_hiDrctFlt_coeff[1];
    phwcfg->hi_drct_flt_coeff3 = psta->gInterp.hw_dmT_hiDrctFlt_coeff[2];
    phwcfg->hi_drct_flt_coeff4 = psta->gInterp.hw_dmT_hiDrctFlt_coeff[3];

    /* G_INTERP_OFFSET_ALPHA */
    phwcfg->g_interp_sharp_strg_offset = pdyn->gInterp.hw_dmT_gInterpSharpStrg_offset;
    phwcfg->grad_lo_flt_alpha = (pdyn->gDrctAlpha.hw_dmT_gradLoFlt_alpha * (1 << RK_DM23_FIX_BIT_GRAD_FLT_ALPHA));

    /* G_INTERP_DRCT_OFFSET */
    //@reg:hw_dmT_drct_offset
    phwcfg->drct_offset[0] = MAX(pdyn->gDrctAlpha.hw_dmT_luma2Drct_offset[0], 1);
    phwcfg->drct_offset[1] = MAX(pdyn->gDrctAlpha.hw_dmT_luma2Drct_offset[1], 1);
    phwcfg->drct_offset[2] = MAX(pdyn->gDrctAlpha.hw_dmT_luma2Drct_offset[2], 1);
    phwcfg->drct_offset[3] = MAX(pdyn->gDrctAlpha.hw_dmT_luma2Drct_offset[3], 1);
    phwcfg->drct_offset[4] = MAX(pdyn->gDrctAlpha.hw_dmT_luma2Drct_offset[4], 1);
    phwcfg->drct_offset[5] = MAX(pdyn->gDrctAlpha.hw_dmT_luma2Drct_offset[5], 1);
    phwcfg->drct_offset[6] = MAX(pdyn->gDrctAlpha.hw_dmT_luma2Drct_offset[6], 1);
    phwcfg->drct_offset[7] = MAX(pdyn->gDrctAlpha.hw_dmT_luma2Drct_offset[7], 1);

    /* G_FILTER_MODE_OFFSET */
    phwcfg->gflt_mode = pdyn->hw_dmT_gOutlsFlt_mode;
    phwcfg->gflt_ratio = ROUND_F(pdyn->gOutlsFlt_bifilt.hw_dmT_bifilt_alpha * (1 << RK_DM23_FIX_BIT_BF_RATIO));
    phwcfg->gflt_offset = pdyn->gOutlsFlt_maxMin.hw_dmT_gOutlsFltRange_offset;

    /* G_FILTER_FILTER */
    int filter_coe[3];
    if (pdyn->gOutlsFlt_bifilt.sw_dmT_filtCfg_mode == dm_cfgByFiltStrg_mode) {
        float dis_table_3x3[3] = { 0.0, 1.0, 2.0};
        float gaus_table[3];

        float sigma = pdyn->gOutlsFlt_bifilt.sw_dmT_filtSpatial_strg;
        double e = 2.71828182845905;
        for (int k = 0; k < 3; k++)
        {
            float tmp = pow(e, -dis_table_3x3[k] / 2.0 / sigma / sigma);
            gaus_table[k] = tmp;
        }

        float sumTable = 0;
        sumTable = gaus_table[0]
                   + 4 * gaus_table[1]
                   + 4 * gaus_table[2];

        for (int k = 0; k < 3; k++) {
            gaus_table[k] = gaus_table[k] / sumTable;
            filter_coe[k] = ROUND_F(gaus_table[k] * (1 << RK_DM23_FIX_BIT_BF_WGT));
        }
    } else {
        filter_coe[0] = ROUND_F(pdyn->gOutlsFlt_bifilt.hw_dmT_filtSpatial_wgt[0] * (1 << RK_DM23_FIX_BIT_BF_WGT));
        filter_coe[1] = ROUND_F(pdyn->gOutlsFlt_bifilt.hw_dmT_filtSpatial_wgt[1] * (1 << RK_DM23_FIX_BIT_BF_WGT));
        filter_coe[2] = ROUND_F(pdyn->gOutlsFlt_bifilt.hw_dmT_filtSpatial_wgt[2] * (1 << RK_DM23_FIX_BIT_BF_WGT));
    }

    // check filter coeff
    int sum_coeff, offset;
    sum_coeff = filter_coe[0] + 4 * filter_coe[1] + 4 * filter_coe[2];
    offset = (1 << RK_DM23_FIX_BIT_BF_WGT) - sum_coeff;
    filter_coe[0] = filter_coe[0] + offset;

    phwcfg->gflt_coe0 = filter_coe[0];
    phwcfg->gflt_coe1 = filter_coe[1];
    phwcfg->gflt_coe2 = filter_coe[2];

    /* G_FILTER_VSIGMA */
    phwcfg->gflt_vsigma[0] = ROUND_F((1 << RK_DM23_FIX_BIT_INV_BF_SIGMA) / pdyn->gOutlsFlt_bifilt.hw_dmT_luma2RgeSgm_val[0]);
    phwcfg->gflt_vsigma[1] = ROUND_F((1 << RK_DM23_FIX_BIT_INV_BF_SIGMA) / pdyn->gOutlsFlt_bifilt.hw_dmT_luma2RgeSgm_val[1]);
    phwcfg->gflt_vsigma[2] = ROUND_F((1 << RK_DM23_FIX_BIT_INV_BF_SIGMA) / pdyn->gOutlsFlt_bifilt.hw_dmT_luma2RgeSgm_val[2]);
    phwcfg->gflt_vsigma[3] = ROUND_F((1 << RK_DM23_FIX_BIT_INV_BF_SIGMA) / pdyn->gOutlsFlt_bifilt.hw_dmT_luma2RgeSgm_val[3]);
    phwcfg->gflt_vsigma[4] = ROUND_F((1 << RK_DM23_FIX_BIT_INV_BF_SIGMA) / pdyn->gOutlsFlt_bifilt.hw_dmT_luma2RgeSgm_val[4]);
    phwcfg->gflt_vsigma[5] = ROUND_F((1 << RK_DM23_FIX_BIT_INV_BF_SIGMA) / pdyn->gOutlsFlt_bifilt.hw_dmT_luma2RgeSgm_val[5]);
    phwcfg->gflt_vsigma[6] = ROUND_F((1 << RK_DM23_FIX_BIT_INV_BF_SIGMA) / pdyn->gOutlsFlt_bifilt.hw_dmT_luma2RgeSgm_val[6]);
    phwcfg->gflt_vsigma[7] = ROUND_F((1 << RK_DM23_FIX_BIT_INV_BF_SIGMA) / pdyn->gOutlsFlt_bifilt.hw_dmT_luma2RgeSgm_val[7]);

    /* C_FILTER_GUIDE_GAUS */
    phwcfg->cnr_lo_guide_lpf_coe0 = psta->cFlt.hw_dmT_cnrLoGuideLpf_coeff[0];
    phwcfg->cnr_lo_guide_lpf_coe1 = psta->cFlt.hw_dmT_cnrLoGuideLpf_coeff[1];
    phwcfg->cnr_lo_guide_lpf_coe2 = psta->cFlt.hw_dmT_cnrLoGuideLpf_coeff[2];

    /* C_FILTER_CE_GAUS */
    phwcfg->cnr_pre_flt_coe0 = psta->cFlt.hw_dmT_cnrPreFlt_coeff[0];
    phwcfg->cnr_pre_flt_coe1 = psta->cFlt.hw_dmT_cnrPreFlt_coeff[1];
    phwcfg->cnr_pre_flt_coe2 = psta->cFlt.hw_dmT_cnrPreFlt_coeff[2];

    /* C_FILTER_ALPHA_GAUS */
    phwcfg->cnr_alpha_lpf_coe0 = psta->cFlt.hw_dmT_cnrAlphaLpf_coeff[0];
    phwcfg->cnr_alpha_lpf_coe1 = psta->cFlt.hw_dmT_cnrAlphaLpf_coeff[1];
    phwcfg->cnr_alpha_lpf_coe2 = psta->cFlt.hw_dmT_cnrAlphaLpf_coeff[2];

    /* C_FILTER_IIR_0 */
    // log fix bit : 10, 1 / sigma, (1 << RK_DM23_FIX_BIT_INV_SIGMA) / (sigma)
    float sqrtLog2e                         = 1.2011;
    int hw_dmT_cnrLoFltVsigma_inv           = ROUND_F((1 << RK_DM23_FIX_BIT_INV_SIGMA) * sqrtLog2e / ((1 << RK_DM23_FIX_BIT_LOG2) * pdyn->cFlt.sw_dmT_cnrLoFlt_rgeSgm));
    //@reg: hw_dmT_cnrLoFltWgt_slope
    phwcfg->cnr_lo_flt_wgt_slope           = ROUND_F(pdyn->cFlt.hw_dmT_cnrLoFltWgt_slope * (1 << RK_DM23_FIX_BIT_SLOPE));

    // chromaFilterStrength * cnr_lo_flt_wgt_slope, int to float
    int tmptmp                              = hw_dmT_cnrLoFltVsigma_inv * phwcfg->cnr_lo_flt_wgt_slope;
    int shiftBit                            = LOG2(tmptmp) - RK_DM23_FIX_BIT_INT_TO_FLOAT;
    shiftBit                                = MAX(shiftBit, 0);
    //@reg: hw_dmT_cnrLoFltStrg_inv
    phwcfg->cnr_lo_flt_strg_inv             = ROUND_F((float)tmptmp / (1 << shiftBit));
    //@reg: hw_dmT_cnrLoFltStrg_shift
    phwcfg->cnr_lo_flt_strg_shift          = RK_DM23_FIX_BIT_INV_SIGMA - shiftBit;

    /* C_FILTER_IIR_1 */

    //@reg: hw_dmT_cnrLoFltWgt_maxLimit
    phwcfg->cnr_lo_flt_wgt_max_limit           = ROUND_F(pdyn->cFlt.hw_dmT_cnrLoFltWgt_maxLimit * (1 << RK_DM23_FIX_BIT_WGT_RATIO));
    //@reg: hw_dmT_cnrLoFltWgt_minThred
    phwcfg->cnr_lo_flt_wgt_min_thred          = ROUND_F(pdyn->cFlt.hw_dmT_cnrLoFltWgt_minThred * (1 << RK_DM23_FIX_BIT_IIR_WGT));

    /* C_FILTER_BF */

    //@reg: hw_dmT_cnrHiFltWgt_minLimit
    phwcfg->cnr_hi_flt_wgt_min_limit = pdyn->cFlt.hw_dmT_cnrHiFltWgt_minLimit;

    //@reg: hw_dmT_cnrHiFltCur_wgt
    phwcfg->cnr_hi_flt_cur_wgt = pdyn->cFlt.hw_dmT_cnrHiFltCur_wgt;
    int scale                   = (1 << 12) - 1; //rawbit: 12
    float log2e                 = 0.8493;

    //@reg: hw_dmT_cnrHiFlt_vsigma
    phwcfg->cnr_hi_flt_vsigma = (int)((1 << RK_DM23_FIX_BIT_INV_BF_SIGMA) * log2e / (pdyn->cFlt.sw_dmT_cnrHiFlt_rgeSgm * scale));

    /* C_FILTER_LOG_OFFSET */
    phwcfg->cnr_trans_en = cvtinfo->cmps_on ? 0 : 1;
    phwcfg->cnr_log_grad_offset = pdyn->cFlt.hw_dmT_cnrLogGrad_offset;
    phwcfg->cnr_log_guide_offset = pdyn->cFlt.hw_dmT_cnrLogGuide_offset;

    /* C_FILTER_ALPHA */
    phwcfg->cnr_moire_alpha_offset = pdyn->cFlt.hw_dmT_cnrMoireAlpha_offset;
    phwcfg->cnr_moire_alpha_scale = ROUND_F(pdyn->cFlt.hw_dmT_cnrMoireAlpha_scale * (1 << RK_DM23_FIX_BIT_ALPHA_SCALE));

    /* C_FILTER_EDGE */
    phwcfg->cnr_edge_alpha_offset = pdyn->cFlt.hw_dmT_cnrEdgeAlpha_offset;
    phwcfg->cnr_edge_alpha_scale = ROUND_F(pdyn->cFlt.hw_dmT_cnrEdgeAlpha_scale * (1 << RK_DM23_FIX_BIT_EDGE_SCALE));

    return;
}
