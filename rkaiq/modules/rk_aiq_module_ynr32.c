#include "rk_aiq_isp32_modules.h"

#define YNR_V22_ISO_CURVE_POINT_BIT          4
#define YNR_V22_ISO_CURVE_POINT_NUM          ((1 << YNR_V22_ISO_CURVE_POINT_BIT)+1)
#define YNR_V22_NOISE_SIGMA_FIX_BIT              3

bool rk_aiq_ynr32_check_attrib(ynr_api_attrib_t *attr)
{
    return true;
}

bool rk_aiq_ynr32_check_param(ynr_param_t *param)
{
    return true;
}

void rk_aiq_ynr32_params_cvt(void* attr, struct isp32_isp_params_cfg* isp_cfg,common_cvt_info_t *cvtinfo)
{
    int i;
    int tmp;
    int w0, w1, w2;
    struct isp32_ynr_cfg *phwcfg = &isp_cfg->others.ynr_cfg;
    ynr_param_t *ynr_param = (ynr_param_t *) attr;
    ynr_params_static_t* psta = &ynr_param->sta;
    ynr_params_dyn_t* pdyn = &ynr_param->dyn;
    int rows = cvtinfo->rawHeight;
    int cols = cvtinfo->rawWidth;

    // YNR_2700_GLOBAL_CTRL (0x0000)
    phwcfg->rnr_en            = 1;
    phwcfg->thumb_mix_cur_en  = pdyn->loNrBifilt5.hw_ynrT_guideImg_mode == 1;

    tmp = (int)(pdyn->inPixSgm.hw_ynrT_glbPixSgm_alpha * (1 << 3));
    phwcfg->global_gain_alpha = CLIP(tmp, 0, 8);
    tmp = (int)(pdyn->inPixSgm.hw_ynrT_glbPixSgm_val * (1 << 4));
    phwcfg->global_gain       = CLIP(tmp, 0, 1023);

    switch (pdyn->loNrPost.hw_ynrT_loNROutSel_mode) {
    case loNrGausFilt_mode:
        phwcfg->flt1x1_bypass_sel = 0;
        phwcfg->flt1x1_bypass = 1;
        break;
    case loNrBifilt3_mode:
        phwcfg->flt1x1_bypass_sel = 1;
        phwcfg->flt1x1_bypass = 1;
        break;
    case loNrBifilt3GuideImg_mode:
        phwcfg->flt1x1_bypass_sel = 2;
        phwcfg->flt1x1_bypass = 1;
        break;
    case loNrEnd_mode:
        phwcfg->flt1x1_bypass = 0;
        break;
    }
    phwcfg->nlm11x11_bypass   = !pdyn->hiNr.hw_ynrT_nlmFilt_en;
    phwcfg->lgft3x3_bypass    = !pdyn->loNrBifilt3.hw_ynrT_bifilt3_en;
    phwcfg->lbft5x5_bypass    = !pdyn->loNrBifilt5.hw_ynrT_bifilt5_en;
    phwcfg->bft3x3_bypass     = !pdyn->loNrGaus.hw_ynrT_gausFilt_en;

    // YNR_2700_RNR_MAX_R  (0x0004)
    float r_sq_inv = 16.0f / (cols * cols + rows * rows); // divide 2
    int* number_ptr = (int*)(&r_sq_inv);
    int EE = ((*number_ptr) >> 23) & (0x0ff);
    EE = -(EE - 127 + 1);
    int MM = (*number_ptr) & 0x7fffff;
    float tmp2 = ((MM / (float)(1 << 23)) + 1) / 2;
    MM = (int)(256 * tmp2 + 0.5);
    tmp = (MM << 5) + EE;
    phwcfg->rnr_max_r = CLIP(tmp, 0, 0x3fff);
    tmp = (1.0) * (1 << 7);
    phwcfg->local_gainscale = CLIP(tmp, 0, 0x80);

    // YNR_2700_CENTRE_COOR (0x0008)
    phwcfg->rnr_center_coorv = rows / 2;
    phwcfg->rnr_center_coorh = cols / 2;

    // YNR_2700_CENTRE_COOR (0x000c)
    tmp = pdyn->inPixSgm.hw_ynrT_localPixSgm_scale2 * (1 << 4);
    phwcfg->localgain_adj = CLIP(tmp, 0, 0xff);
    //tmp = pSelect->ynr_adjust_thresh * 16;  //new
    tmp = pdyn->inPixSgm.hw_ynrT_localPixSgmScl2_thred * 16;
    phwcfg->loclagain_adj_thresh  = CLIP(tmp, 0, 0x3ff);;

    // YNR_2700_LOWNR_CTRL0 (0x0010)
    tmp = (int)(1.0f / pdyn->loNrBifilt5.hw_ynrT_rgeSgm_scale * (1 << 9));
    phwcfg->low_bf_inv1 = CLIP(tmp, 0, 0x3fff);
    tmp = (int)(1.0f / pdyn->loNrBifilt3.hw_ynrT_rgeSgm_div * (1 << 9));
    phwcfg->low_bf_inv0 = CLIP(tmp, 0, 0x3fff);

    // YNR_2700_LOWNR_CTRL1  (0x0014)
    tmp = (int)(pdyn->loNrBifilt3.hw_ynrT_guideImgLpf_alpha * (1 << 7));
    phwcfg->low_peak_supress = CLIP(tmp, 0, 0x80);
    tmp = (int)(pdyn->loNrPost.hw_ynrT_softThd_val * (1 << 6));
    phwcfg->low_thred_adj = CLIP(tmp, 0, 0x7ff);

    // YNR_2700_LOWNR_CTRL2 (0x0018)
    tmp = (int)(pdyn->loNrBifilt5.hw_ynrT_spatialSgm_scale * (1 << 2));
    phwcfg->low_dist_adj = CLIP(tmp, 0, 0x1ff);
    tmp = (int)(pdyn->loNrBifilt5.hw_ynrT_guideImgText_maxLimit);
    phwcfg->low_edge_adj_thresh = CLIP(tmp, 0, 0x3ff);

    // YNR_2700_LOWNR_CTRL3 (0x001c)
    tmp = (int)(pdyn->loNrBifilt3.hw_ynrT_bifiltOut_alpha * (1 << 7));
    phwcfg->low_bi_weight = CLIP(tmp, 0, 0x80);
    tmp = (int)(pdyn->loNrPost.hw_ynrT_loNr_alpha * (1 << 7));
    phwcfg->low_weight = CLIP(tmp, 0, 0x80);
    tmp = (int)(pdyn->loNrBifilt5.hw_ynrT_centerPix_wgt * (1 << 10));
    phwcfg->low_center_weight = CLIP(tmp, 0, 0x400);

    // YNR_2700_LOWNR_CTRL4 (0x002c)
    tmp = (int)(0x0000);
    phwcfg->frame_full_size = CLIP(tmp, 0, 0x3fff);
    tmp = (int)(pdyn->loNrBifilt5.hw_ynrT_nhoodPixWgt_thred * 1023);
    phwcfg->lbf_weight_thres = CLIP(tmp, 0, 0x3ff);

    // YNR_2700_GAUSS1_COEFF (0x0030)
    float filter1_sigma = pdyn->loNrBifilt3.sw_ynrT_guideImgLpf_strg;
    float filt1_coeff1 = exp(-1 / (2 * filter1_sigma * filter1_sigma));
    float filt1_coeff0 = filt1_coeff1 * filt1_coeff1;
    float coeff1_sum = 1 + 4 * filt1_coeff1 + 4 * filt1_coeff0;
    w0 = (int)(filt1_coeff0 / coeff1_sum * 256 + 0.5);
    w1 = (int)(filt1_coeff1 / coeff1_sum * 256 + 0.5);
    w2 = 256 - w0 * 4 - w1 * 4;
    tmp = (int)(w2);
    phwcfg->low_gauss1_coeff2 = CLIP(tmp, 0, 0x1ff);
    tmp = (int)(w1);
    phwcfg->low_gauss1_coeff1 = CLIP(tmp, 0, 0x3f);
    tmp = (int)(w0);
    phwcfg->low_gauss1_coeff0 = CLIP(tmp, 0, 0x3f);

    // YNR_2700_GAUSS2_COEFF (0x0034)
    float filter2_sigma = pdyn->loNrGaus.sw_ynrT_gausFilt_strg;
    float filt2_coeff1 = exp(-1 / (2 * filter2_sigma * filter2_sigma));
    float filt2_coeff0 = filt2_coeff1 * filt2_coeff1;
    float coeff2_sum = 1 + 4 * filt2_coeff1 + 4 * filt2_coeff0;
    w0 = (int)(filt2_coeff0 / coeff2_sum * 256 + 0.5);
    w1 = (int)(filt2_coeff1 / coeff2_sum * 256 + 0.5);
    w2 = 256 - w0 * 4 - w1 * 4;
    tmp = (int)(w2);
    phwcfg->low_gauss2_coeff2 = CLIP(tmp, 0, 0x1ff);
    tmp = (int)(w1);
    phwcfg->low_gauss2_coeff1 = CLIP(tmp, 0, 0x3f);
    tmp = (int)(w0);
    phwcfg->low_gauss2_coeff0 = CLIP(tmp, 0, 0x3f);

    // YNR_2700_SGM_DX_0_1 (0x0040 - 0x0060)
    // YNR_2700_LSGM_Y_0_1 (0x0070- 0x0090)
    // YNR_2700_HSGM_Y_0_1 (0x00a0- 0x00c0)
    for (int i = 0; i < YNR_V22_ISO_CURVE_POINT_NUM; i++) {
        tmp = pdyn->loNrPost.hw_ynrC_luma2LoSgm_curve.idx[i];
        phwcfg->luma_points_x[i] = CLIP(tmp, 0, 0x400);
        tmp = (int)(pdyn->loNrPost.hw_ynrC_luma2LoSgm_curve.val[i] * (1 << YNR_V22_NOISE_SIGMA_FIX_BIT));
        phwcfg->lsgm_y[i] = CLIP(tmp, 0, 0xfff);
    }
    //TODO: soft params.

    // YNR_2700_RNR_STRENGTH03 (0x00d0- 0x00e0)
    for (int i = 0; i < 17; i++) {
        tmp = (int)(pdyn->hiNr.hw_ynrT_radiDist2Wgt_val[i] * 16);
        phwcfg->rnr_strength3[i] = CLIP(tmp, 0, 0xff);
    }

    // YNR_NLM_SIGMA_GAIN  (0x00f0)
    tmp = (int)(pdyn->hiNr.hw_ynrT_nlmSgm_minLimit * (1 << 11));
    phwcfg->nlm_min_sigma = CLIP(tmp, 0, 0x7ff);
    tmp = (int)(pdyn->hiNr.hw_ynrT_pixSgmRadiDistWgt_alpha * (1 << 4));
    phwcfg->nlm_hi_gain_alpha = CLIP(tmp, 0, 0x1f);
    tmp = (int)(pdyn->hiNr.hw_ynrT_nlmSgm_scale  * (1 << 6));
    phwcfg->nlm_hi_bf_scale = CLIP(tmp, 0, 0x3ff);

    // YNR_NLM_COE  (0x00f4)
    tmp = (int)(pdyn->hiNr.hw_ynrT_nlmSpatial_wgt[0]);
    phwcfg->nlm_coe[0] = CLIP(tmp, 0, 0xf);
    tmp = (int)(pdyn->hiNr.hw_ynrT_nlmSpatial_wgt[1]);
    phwcfg->nlm_coe[1] = CLIP(tmp, 0, 0xf);
    tmp = (int)(pdyn->hiNr.hw_ynrT_nlmSpatial_wgt[2]);
    phwcfg->nlm_coe[2] = CLIP(tmp, 0, 0xf);
    tmp = (int)(pdyn->hiNr.hw_ynrT_nlmSpatial_wgt[3]);
    phwcfg->nlm_coe[3] = CLIP(tmp, 0, 0xf);
    tmp = (int)(pdyn->hiNr.hw_ynrT_nlmSpatial_wgt[4]);
    phwcfg->nlm_coe[4] = CLIP(tmp, 0, 0xf);
    tmp = (int)(pdyn->hiNr.hw_ynrT_nlmSpatial_wgt[5]);
    phwcfg->nlm_coe[5] = CLIP(tmp, 0, 0xf);

    //YNR_NLM_WEIGHT (0x00f8)
    tmp = (int)(pdyn->hiNr.hw_ynrT_nlmSpatial_wgt[6] * (1 << 10));
    phwcfg->nlm_center_weight = CLIP(tmp, 0, 0x3ffff);
    tmp = (int)(pdyn->hiNr.hw_ynrT_nlmRgeWgt_negOffset * (1 << 10));
    phwcfg->nlm_weight_offset = CLIP(tmp, 0, 0x3ff);

    // YNR_NLM_NR_WEIGHT (0x00fc)
    tmp = (int)(pdyn->hiNr.hw_ynrT_nlmOut_alpha * (1 << 10));
    phwcfg->nlm_nr_weight = CLIP(tmp, 0, 0x400);
    return;
}
