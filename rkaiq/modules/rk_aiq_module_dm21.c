#include "rk_aiq_isp32_modules.h"

static bool rk_aiq_dm21_check_sta(dm_params_static_t *sta) {
    CHECK_NUM_IN_RANGE(sta->gInterp.hw_dmT_loDrctFlt_coeff[0], -16, 15);
    CHECK_NUM_IN_RANGE(sta->gInterp.hw_dmT_loDrctFlt_coeff[1], -16, 15);
    CHECK_NUM_IN_RANGE(sta->gInterp.hw_dmT_loDrctFlt_coeff[2], -16, 15);
    CHECK_NUM_IN_RANGE(sta->gInterp.hw_dmT_loDrctFlt_coeff[3], -16, 15);
    CHECK_NUM_IN_RANGE(sta->gInterp.hw_dmT_hiDrctFlt_coeff[0], -16, 15);
    CHECK_NUM_IN_RANGE(sta->gInterp.hw_dmT_hiDrctFlt_coeff[1], -16, 15);
    CHECK_NUM_IN_RANGE(sta->gInterp.hw_dmT_hiDrctFlt_coeff[2], -16, 15);
    CHECK_NUM_IN_RANGE(sta->gInterp.hw_dmT_hiDrctFlt_coeff[3], -16, 15);
    CHECK_NUM_IN_RANGE(sta->cFlt.hw_dmT_cnrLoGuideLpf_coeff[0], 0, 127);
    CHECK_NUM_IN_RANGE(sta->cFlt.hw_dmT_cnrLoGuideLpf_coeff[1], 0, 127);
    CHECK_NUM_IN_RANGE(sta->cFlt.hw_dmT_cnrLoGuideLpf_coeff[2], 0, 127);
    CHECK_NUM_IN_RANGE(sta->cFlt.hw_dmT_cnrAlphaLpf_coeff[0], 0, 127);
    CHECK_NUM_IN_RANGE(sta->cFlt.hw_dmT_cnrAlphaLpf_coeff[1], 0, 127);
    CHECK_NUM_IN_RANGE(sta->cFlt.hw_dmT_cnrAlphaLpf_coeff[2], 0, 127);
    CHECK_NUM_IN_RANGE(sta->cFlt.hw_dmT_cnrPreFlt_coeff[0], 0, 127);
    CHECK_NUM_IN_RANGE(sta->cFlt.hw_dmT_cnrPreFlt_coeff[1], 0, 127);
    CHECK_NUM_IN_RANGE(sta->cFlt.hw_dmT_cnrPreFlt_coeff[2], 0, 127);
    return true;
}

static bool rk_aiq_dm21_check_dyn(dm_params_dyn_t *dyn) {
    CHECK_NUM_IN_RANGE(dyn->gInterp.hw_dmT_gInterpSharpStrg_offset, 0, 4095);
    CHECK_NUM_IN_RANGE(dyn->gInterp.hw_dmT_gInterpSharpStrg_maxLim, 0, 63);
    CHECK_NUM_IN_RANGE(dyn->gDrctAlpha.hw_dmT_hiDrct_thred, 0, 15);
    CHECK_NUM_IN_RANGE(dyn->gDrctAlpha.hw_dmT_loDrct_thred, 0, 15);
    CHECK_NUM_IN_RANGE(dyn->gDrctAlpha.hw_dmT_hiTexture_thred, 0, 15);
    CHECK_NUM_IN_RANGE(dyn->gOutlsFlt.hw_dmT_gOutlsFltRange_offset, 0, 2047);
    CHECK_NUM_IN_RANGE(dyn->cFlt.hw_dmT_cnrLogGuide_offset, 0, 4095);
    CHECK_NUM_IN_RANGE(dyn->cFlt.sw_dmT_cnrLoFlt_vsigma, 0.0, 1.0);
    CHECK_NUM_IN_RANGE(dyn->cFlt.sw_dmT_cnrLoFltWgt_maxLimit, 0.0, 15.875);
    CHECK_NUM_IN_RANGE(dyn->cFlt.sw_dmT_cnrLoFltWgt_minThred, 0.0, 0.98);
    CHECK_NUM_IN_RANGE(dyn->cFlt.sw_dmT_cnrLoFltWgt_slope, 0.0, 31.992);
    CHECK_NUM_IN_RANGE(dyn->cFlt.sw_dmT_cnrHiFlt_vsigma, 0.0, 1.0);
    CHECK_NUM_IN_RANGE(dyn->cFlt.hw_dmT_cnrHiFltWgt_minLimit, 0, 127);
    CHECK_NUM_IN_RANGE(dyn->cFlt.hw_dmT_cnrHiFltCur_wgt, 0, 127);
    CHECK_NUM_IN_RANGE(dyn->cFlt.hw_dmT_cnrLogGrad_offset, 0, 8191);
    CHECK_NUM_IN_RANGE(dyn->cFlt.hw_dmT_cnrMoireAlpha_offset, 0, 4095);
    CHECK_NUM_IN_RANGE(dyn->cFlt.sw_dmT_cnrMoireAlpha_scale, 0.0, 1023.999);
    CHECK_NUM_IN_RANGE(dyn->cFlt.hw_dmT_cnrEdgeAlpha_offset, 0, 4095);
    CHECK_NUM_IN_RANGE(dyn->cFlt.sw_dmT_cnrEdgeAlpha_scale, 0.0, 1023.999);
    return true;
}

bool rk_aiq_dm21_check_param(dm_param_t *param)
{
    if (!rk_aiq_dm21_check_sta(&param->sta))
        return false;

    if (!rk_aiq_dm21_check_dyn(&param->dyn))
        return false;
    return true;
}

bool rk_aiq_dm21_check_attrib(dm_api_attrib_t *attr)
{
    if (attr->opMode == RK_AIQ_OP_MODE_AUTO) {
        dm_param_auto_t *pAuto = &attr->stAuto;

        if (!rk_aiq_dm21_check_sta(&pAuto->sta))
            return false;

        for (int i=0; i<13; i++) {
            if (!rk_aiq_dm21_check_dyn(&pAuto->dyn[i]))
                return false;
        }

    } else if (attr->opMode == RK_AIQ_OP_MODE_MANUAL) {
        dm_param_t *pMan= &attr->stMan;

        return rk_aiq_dm21_check_param(pMan);
    } else {
        return false;
    }
    return true;
}

void rk_aiq_dm21_params_cvt(void* attr, struct isp32_isp_params_cfg* isp_cfg)
{
    struct isp32_debayer_cfg *phwcfg = &isp_cfg->others.debayer_cfg;
    dm_param_t *dm_param = (dm_param_t *) attr;
    dm_params_dyn_t *pdyn = &dm_param->dyn;
    dm_params_static_t *psta = &dm_param->sta;

    phwcfg->filter_g_en = pdyn->gOutlsFlt.hw_dmT_gOutlsFlt_en;
    phwcfg->filter_c_en = pdyn->cFlt.hw_dmT_cnrFlt_en;

    //printf("DemosaicCurParam2HwCfg filter_g_en %d, filter_c_en %d!\n", phwcfg->filter_g_en, phwcfg->filter_c_en);
    /* G_INTERP */
    phwcfg->clip_en     = pdyn->gInterp.hw_dmT_gInterpClip_en;
    phwcfg->dist_scale  = pdyn->gDrctAlpha.hw_dmT_hiTexture_thred;
    phwcfg->thed0       = pdyn->gDrctAlpha.hw_dmT_hiDrct_thred;
    phwcfg->thed1       = pdyn->gDrctAlpha.hw_dmT_loDrct_thred;
    phwcfg->select_thed = pdyn->gDrctAlpha.hw_dmT_drctMethod_thred;
    phwcfg->max_ratio   = pdyn->gInterp.hw_dmT_gInterpSharpStrg_maxLim;

    /* G_INTERP_FILTER */
    phwcfg->filter1_coe1 = psta->gInterp.hw_dmT_loDrctFlt_coeff[0];
    phwcfg->filter1_coe2 = psta->gInterp.hw_dmT_loDrctFlt_coeff[1];
    phwcfg->filter1_coe3 = psta->gInterp.hw_dmT_loDrctFlt_coeff[2];
    phwcfg->filter1_coe4 = psta->gInterp.hw_dmT_loDrctFlt_coeff[3];

    phwcfg->filter2_coe1 = psta->gInterp.hw_dmT_hiDrctFlt_coeff[0];
    phwcfg->filter2_coe2 = psta->gInterp.hw_dmT_hiDrctFlt_coeff[1];
    phwcfg->filter2_coe3 = psta->gInterp.hw_dmT_hiDrctFlt_coeff[2];
    phwcfg->filter2_coe4 = psta->gInterp.hw_dmT_hiDrctFlt_coeff[3];

    /* C_FILTER_GUIDE_GAUS */
    phwcfg->guid_gaus_coe0 = psta->cFlt.hw_dmT_cnrLoGuideLpf_coeff[0];
    phwcfg->guid_gaus_coe1 = psta->cFlt.hw_dmT_cnrLoGuideLpf_coeff[1];
    phwcfg->guid_gaus_coe2 = psta->cFlt.hw_dmT_cnrLoGuideLpf_coeff[2];

    /* C_FILTER_CE_GAUS */
    phwcfg->ce_gaus_coe0 = psta->cFlt.hw_dmT_cnrPreFlt_coeff[0];
    phwcfg->ce_gaus_coe1 = psta->cFlt.hw_dmT_cnrPreFlt_coeff[1];
    phwcfg->ce_gaus_coe2 = psta->cFlt.hw_dmT_cnrPreFlt_coeff[2];

    /* C_FILTER_ALPHA_GAUS */
    phwcfg->alpha_gaus_coe0 = psta->cFlt.hw_dmT_cnrAlphaLpf_coeff[0];
    phwcfg->alpha_gaus_coe1 = psta->cFlt.hw_dmT_cnrAlphaLpf_coeff[1];
    phwcfg->alpha_gaus_coe2 = psta->cFlt.hw_dmT_cnrAlphaLpf_coeff[2];

    /* G_INTERP_OFFSET */
    phwcfg->hf_offset         = pdyn->gDrctAlpha.hw_dmT_drct_offset;
    phwcfg->gain_offset       = pdyn->gInterp.hw_dmT_gInterpSharpStrg_offset;

    /* G_FILTER_OFFSET */
    phwcfg->offset            = pdyn->gOutlsFlt.hw_dmT_gOutlsFltRange_offset;

    /* C_FILTER_LOG_OFFSET */
    phwcfg->loghf_offset      = pdyn->cFlt.hw_dmT_cnrLogGrad_offset;
    phwcfg->loggd_offset      = pdyn->cFlt.hw_dmT_cnrLogGuide_offset;

    /* C_FILTER_ALPHA */
    phwcfg->alpha_offset      = pdyn->cFlt.hw_dmT_cnrMoireAlpha_offset;
    phwcfg->alpha_scale       = ROUND_F(pdyn->cFlt.sw_dmT_cnrMoireAlpha_scale * (1 << RK_DM21_FIX_BIT_ALPHA_SCALE));

    /* C_FILTER_EDGE */
    phwcfg->edge_offset       = pdyn->cFlt.hw_dmT_cnrEdgeAlpha_offset;
    phwcfg->edge_scale        = ROUND_F(pdyn->cFlt.sw_dmT_cnrEdgeAlpha_scale * (1 << RK_DM21_FIX_BIT_EDGE_SCALE));

    // log fix bit : 10, 1 / sigma, (1 << RK_DM_V30_FIX_BIT_INV_SIGMA) / (sigma)
    float sqrtLog2e           = 1.2011;
    uint32_t ce_sgm           = ROUND_F((1 << RK_DM21_FIX_BIT_INV_SIGMA) * sqrtLog2e / ((1 << RK_DM21_FIX_BIT_LOG2) * pdyn->cFlt.sw_dmT_cnrLoFlt_vsigma));
    phwcfg->wgtslope          = ROUND_F(pdyn->cFlt.sw_dmT_cnrLoFltWgt_slope * (1 << RK_DM21_FIX_BIT_WGT_SLOPE));

    /* C_FILTER_IIR_1 */
    phwcfg->wet_clip          = ROUND_F(pdyn->cFlt.sw_dmT_cnrLoFltWgt_maxLimit * (1 << RK_DM21_FIX_BIT_IIR_WGT_CLIP));
    phwcfg->wet_ghost         = ROUND_F(pdyn->cFlt.sw_dmT_cnrLoFltWgt_minThred * (1 << RK_DM21_FIX_BIT_IIR_GHOST));

    // chromaFilterStrength * wgtSlope, int to float
    int tmptmp               = ce_sgm * phwcfg->wgtslope;
    int shiftBit             = LOG2(tmptmp) - RK_DM21_FIX_BIT_INT_TO_FLOAT;
    shiftBit                 = MAX(shiftBit, 0);
    phwcfg->ce_sgm     = ROUND_F((float)tmptmp / (1 << shiftBit));
    phwcfg->exp_shift        = RK_DM21_FIX_BIT_INV_SIGMA - shiftBit;

    /* C_FILTER_BF */
    phwcfg->bf_clip          = pdyn->cFlt.hw_dmT_cnrHiFltWgt_minLimit;
    phwcfg->bf_curwgt        = pdyn->cFlt.hw_dmT_cnrHiFltCur_wgt;

    int scale                = (1 << 12) - 1; //rawbit: 12
    float log2e              = 0.8493;
    phwcfg->bf_sgm           = (int)((1 << RK_DM21_FIX_BIT_INV_BF_SIGMA) * log2e / (pdyn->cFlt.sw_dmT_cnrHiFlt_vsigma * scale));

    return;
}
