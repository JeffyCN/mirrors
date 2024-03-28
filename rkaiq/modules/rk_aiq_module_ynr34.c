#include "rk_aiq_isp39_modules.h"

#define BIT_MIN               (0)
#define BIT_3_MAX             (7)
#define BIT_4_MAX             (15)
#define BIT_8_MAX             (255)
#define BIT_10_MAX            (1023)
#define BIT_14_MAX            (16383)
#define BIT_17_MAX            (131071)
#define LIMIT_VALUE(value,max_value,min_value)      (value > max_value? max_value : value < min_value ? min_value : value)

#define YNR_V24_ISO_CURVE_POINT_BIT          4
#define YNR_V24_ISO_CURVE_POINT_NUM          ((1 << YNR_V24_ISO_CURVE_POINT_BIT)+1)
#define YNR_V24_NOISE_SIGMA_FIX_BIT   3

int ynrClipFloatValueV24(float posx, int BitInt, int BitFloat) {
    int yOutInt    = 0;
    int yOutIntMax = (int)(pow(2, (BitFloat + BitInt)) - 1);
    int yOutIntMin = 0;

    yOutInt = LIMIT_VALUE((int)(posx * pow(2, BitFloat)), yOutIntMax, yOutIntMin);

    return yOutInt;
}

void rk_aiq_ynr34_params_cvt(void* attr, struct isp39_isp_params_cfg* isp_cfg,common_cvt_info_t *cvtinfo)
{
    int i;
    int tmp;
    int w0, w1, w2;
    struct isp39_ynr_cfg *pFix = &isp_cfg->others.ynr_cfg;
    ynr_param_t *ynr_param = (ynr_param_t *) attr;
    ynr_params_static_t* psta = &ynr_param->sta;
    ynr_params_dyn_t* pdyn = &ynr_param->dyn;
    int rows = cvtinfo->rawHeight;
    int cols = cvtinfo->rawWidth;

    // YNR_2700_GLOBAL_CTRL (0x0000)
    pFix->lospnr_bypass    = !pdyn->hw_ynrT_loNr_en;
    pFix->hispnr_bypass    = !pdyn->hiNr_filtProc.hw_ynrT_nlmFilt_en;
    pFix->exgain_bypass    = 0;
    pFix->global_set_gain  = ynrClipFloatValueV24(pdyn->ynrScl_locSgmStrg.hw_ynrT_glbSgmStrg_val, 6, 4);
    pFix->gain_merge_alpha = LIMIT_VALUE(pdyn->ynrScl_locSgmStrg.hw_ynrT_glbSgmStrg_alpha * 8.0, BIT_3_MAX + 1, BIT_MIN);
    pFix->rnr_en           = 1;

    // YNR_2700_RNR_MAX_R  (0x0004)
    float r_sq_inv       = 16.0f / (cols * cols + rows * rows);  // divide 2
    int* number_ptr      = (int*)(&r_sq_inv);
    int EE               = ((*number_ptr) >> 23) & (0x0ff);
    EE                   = -(EE - 127 + 1);
    int MM               = (*number_ptr) & 0x7fffff;
    float tmp2           = ((MM / (float)(1 << 23)) + 1) / 2;
    MM                   = (int)(256 * tmp2 + 0.5);
    tmp                  = (MM << 5) + EE;
    pFix->rnr_max_radius = CLIP(tmp, 0, 0x3fff);

    // local gain scale
    tmp                    = (1.0) * (1 << 7);
    pFix->local_gain_scale = CLIP(tmp, 0, 0x80);

    // YNR_2700_CENTRE_COOR (0x0008)
    pFix->rnr_center_coorv = rows / 2;
    pFix->rnr_center_coorh = cols / 2;

    // YNR_2700_LOCAL_GAIN_CTRL (0x000c) register deleted

    // YNR_2700_LOWNR_CTRL0 (0x0010)
    pFix->ds_filt_soft_thred_scale =
        ynrClipFloatValueV24(pdyn->loNr_iirGuide.hw_ynrT_softThd_scale, 4, 5);
    pFix->ds_img_edge_scale = ynrClipFloatValueV24(pdyn->loNr_preProc.sw_ynrT_edgeDctConf_scale, 5, 0);
    pFix->ds_filt_wgt_thred_scale =
        ynrClipFloatValueV24(pdyn->loNr_iirGuide.hw_ynrT_pixDiffEge_thred, 3, 6);

    // YNR_2700_LOWNR_CTRL1 (0x0014)
    pFix->ds_filt_local_gain_alpha =
        LIMIT_VALUE(pdyn->loNr_iirGuide.hw_ynrT_localYnrScl_alpha * 16.0f, BIT_4_MAX + 1, BIT_MIN);
    pFix->ds_iir_init_wgt_scale = ynrClipFloatValueV24(pdyn->loNr_iirGuide.hw_ynrT_iirInitWgt_scale, 0, 6);
    pFix->ds_filt_center_wgt    = ynrClipFloatValueV24(pdyn->loNr_iirGuide.hw_ynrT_centerPix_wgt, 1, 10);

    // YNR_2700_LOWNR_CTRL2 (0x0018)
    if (pdyn->loNr_iirGuide.hw_ynrT_iiFilt_strg == 0.0f)
        pFix->ds_filt_inv_strg = 0x3fff;
    else
        pFix->ds_filt_inv_strg = ynrClipFloatValueV24(1.0f / pdyn->loNr_iirGuide.hw_ynrT_iiFilt_strg, 5, 9);
    pFix->lospnr_wgt = LIMIT_VALUE(pdyn->loNr_bifilt.hw_ynrT_bifiltOut_alpha * 1024.0f, BIT_10_MAX + 1, BIT_MIN);

    // YNR_2700_LOWNR_CTRL3 (0x001c)
    pFix->lospnr_center_wgt = ynrClipFloatValueV24(pdyn->loNr_bifilt.hw_ynrT_centerPix_wgt, 2, 10);
    pFix->lospnr_strg       = ynrClipFloatValueV24(pdyn->loNr_bifilt.hw_ynrT_rgeSgm_scale, 5, 7);

    // YNR_2700_LOWNR_CTRL4 (0x002c)
    pFix->lospnr_dist_vstrg_scale =
        ynrClipFloatValueV24(pdyn->loNr_bifilt.hw_ynrT_filtSpatialV_strg, 3, 6);
    pFix->lospnr_dist_hstrg_scale =
        ynrClipFloatValueV24(pdyn->loNr_bifilt.hw_ynrT_filtSpatialH_strg, 3, 6);

    // YNR_2700_GAUSS_COEFF (0x0030)
    if (pdyn->loNr_preProc.sw_ynrT_preLpfCfg_mode == ynr_cfgByFiltStrg_mode) {
        float filter1_sigma   = pdyn->loNr_preProc.sw_ynrT_preLpf_strg;
        float filt1_coeff1    = exp(-1 / (2 * filter1_sigma * filter1_sigma));
        float filt1_coeff0    = filt1_coeff1 * filt1_coeff1;
        float coeff1_sum      = 1 + 4 * filt1_coeff1 + 4 * filt1_coeff0;
        int w2                = (int)(filt1_coeff0 / coeff1_sum * 128 + 0.5);
        int w1                = (int)(filt1_coeff1 / coeff1_sum * 128 + 0.5);
        int w0                = 128 - w1 * 4 - w2 * 4;
        pFix->pre_filt_coeff0 = w0;
        pFix->pre_filt_coeff1 = w1;
        pFix->pre_filt_coeff2 = w2;
    } else {
        pFix->pre_filt_coeff0 = pdyn->loNr_preProc.hw_ynrT_preLpfSpatial_wgt[0];
        pFix->pre_filt_coeff1 = pdyn->loNr_preProc.hw_ynrT_preLpfSpatial_wgt[1];
        pFix->pre_filt_coeff2 = pdyn->loNr_preProc.hw_ynrT_preLpfSpatial_wgt[2];
    }

    // YNR_2700_LOW_GAIN_ADJ (0x0034 ~ 0x003c)
    for (int i = 0; i < 9; i++) {
        pFix->lospnr_gain2strg_val[i] =
            ynrClipFloatValueV24(pdyn->ynrScl_locSgmStrg.hw_ynrT_locSgmStrg2YnrScl_val[i], 4, 4);
    }

    // YNR_2700_SGM_DX (0x0040 ~ 0x0060)
    // YNR_2700_luma2sima_val (0x0070 ~ 0x0060)

    if (psta->sw_ynrCfg_sgmCurve_mode == ynr_cfgByCoeff2Curve_mode) {
        for (int i = 0; i < YNR_V24_ISO_CURVE_POINT_NUM; i++) {
            tmp                    = pdyn->hw_ynrC_luma2Sigma_curve.idx[i];
            pFix->luma2sima_idx[i] = CLIP(tmp, 0, 0x400);
            tmp                    = (int)(pdyn->hw_ynrC_luma2Sigma_curve.val[i]  * (1 << YNR_V24_NOISE_SIGMA_FIX_BIT)) * pdyn->coeff2SgmCurve.lowFreqCoeff;
            pFix->luma2sima_val[i] = CLIP(tmp, 0, 0xfff);
        }
    } else {
        for (int i = 0; i < YNR_V24_ISO_CURVE_POINT_NUM; i++) {
            tmp                    = pdyn->hw_ynrC_luma2Sigma_curve.idx[i];
            pFix->luma2sima_idx[i] = CLIP(tmp, 0, 0x400);
            tmp                    = (int)(pdyn->hw_ynrC_luma2Sigma_curve.val[i]  * (1 << YNR_V24_NOISE_SIGMA_FIX_BIT));
            pFix->luma2sima_val[i] = CLIP(tmp, 0, 0xfff);
        }
    }

    // YNR_2700_RNR_STRENGTH03 (0x00d0- 0x00e0)
    for (int i = 0; i < 17; i++) {
        tmp                      = (int)(pdyn->ynrScl_radi.hw_ynrT_radiDist2YnrScl_val[i] * 16);
        pFix->radius2strg_val[i] = CLIP(tmp, 0, 0xff);
    }

    // YNR_2700_NLM_STRONG_EDGE (0x00ec)
    pFix->hispnr_strong_edge = ynrClipFloatValueV24(pdyn->hiNr_alphaProc.hw_ynrT_edgAlphaUp_thred, 5, 3);

    // YNR_2700_NLM_SIGMA_GAIN (0x00f0)
    pFix->hispnr_sigma_min_limit =
        ynrClipFloatValueV24(pdyn->hiNr_filtProc.hw_ynrT_nlmSgm_minLimit, 0, 11);
    pFix->hispnr_local_gain_alpha =
        LIMIT_VALUE(pdyn->hiNr_filtProc.hw_ynrT_localYnrScl_alpha * 16.0f, BIT_4_MAX + 1, BIT_MIN);
    pFix->hispnr_strg = ynrClipFloatValueV24(pdyn->hiNr_filtProc.hw_ynrT_nlmSgm_scale, 4, 6);

    // YNR_2700_NLM_COE (0x00f4)
    pFix->hispnr_filt_coeff[0] = ynrClipFloatValueV24(pdyn->hiNr_filtProc.hw_ynrT_nlmSpatial_wgt[0], 4, 0);
    pFix->hispnr_filt_coeff[1] = ynrClipFloatValueV24(pdyn->hiNr_filtProc.hw_ynrT_nlmSpatial_wgt[1], 4, 0);
    pFix->hispnr_filt_coeff[2] = ynrClipFloatValueV24(pdyn->hiNr_filtProc.hw_ynrT_nlmSpatial_wgt[2], 4, 0);
    pFix->hispnr_filt_coeff[3] = ynrClipFloatValueV24(pdyn->hiNr_filtProc.hw_ynrT_nlmSpatial_wgt[3], 4, 0);
    pFix->hispnr_filt_coeff[4] = ynrClipFloatValueV24(pdyn->hiNr_filtProc.hw_ynrT_nlmSpatial_wgt[4], 4, 0);
    pFix->hispnr_filt_coeff[5] = ynrClipFloatValueV24(pdyn->hiNr_filtProc.hw_ynrT_nlmSpatial_wgt[5], 4, 0);

    // YNR_2700_NLM_WEIGHT (0x00f8)
    pFix->hispnr_filt_wgt_offset =
        ynrClipFloatValueV24(pdyn->hiNr_filtProc.hw_ynrT_nlmRgeWgt_negOff, 0, 10);
    pFix->hispnr_filt_center_wgt =
        LIMIT_VALUE(pdyn->hiNr_filtProc.hw_ynrT_centerPix_wgt * 1024.0f, BIT_17_MAX + 1, BIT_MIN);

    // YNR_2700_NLM_NR_WEIGHT (0x00fc)
    pFix->hispnr_filt_wgt =
        LIMIT_VALUE(pdyn->hiNr_alphaProc.hw_ynrT_nlmOut_alpha * 1024.0f, BIT_10_MAX + 1, BIT_MIN);
    pFix->hispnr_gain_thred = LIMIT_VALUE(pdyn->hiNr_alphaProc.hw_ynrT_locSgmStrgAlphaUp_thred * 16.0, BIT_10_MAX, 1);

    return;
}
