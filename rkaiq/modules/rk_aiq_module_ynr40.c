#include "rk_aiq_isp33_modules.h"

#define BIT_MIN               (0)
#define BIT_3_MAX             (7)
#define BIT_4_MAX             (15)
#define BIT_8_MAX             (255)
#define BIT_10_MAX            (1023)
#define BIT_14_MAX            (16383)
#define BIT_17_MAX            (131071)
#define LIMIT_VALUE(value,max_value,min_value)      (value > max_value? max_value : value < min_value ? min_value : value)

// for noise sigma
#define NOISE_SIGMA_FIX_BIT 3
#define GAIN_YNR_FIX_BITS_DECI 4
#define NOISE_CURVE_POINT_NUM 17

int find_top_one_pos_v3(int data)
{
    int i, j = 1;
    int pos = 0;
    for(i = 0; i < 32; i++)
    {
        if(data & j)
        {
            pos = i + 1;
        }
        j = j << 1;
    }
    return pos;
}
void rk_aiq_ynr40_params_cvt(void* attr, isp_params_t* isp_params, common_cvt_info_t *cvtinfo)
{
    struct isp33_ynr_cfg *pCfg = &isp_params->isp_cfg->others.ynr_cfg;
    ynr_param_t *ynr_param = (ynr_param_t *) attr;
    ynr_params_static_t* psta = &ynr_param->sta;
    ynr_params_dyn_t* pdyn = &ynr_param->dyn;
    int rows = cvtinfo->rawHeight;
    int cols = cvtinfo->rawWidth;
    int i, tmp = 0;
    float gain_base_inv_f = 1.0f;

    // REG: GLOBAL_CTRL
    pCfg->hi_spnr_bypass = !pdyn->hiNr.hw_ynrT_hiNr_en;
    pCfg->mi_spnr_bypass = !pdyn->midLoNr.mf.hw_ynrT_midNr_en;
    pCfg->lo_spnr_bypass = !pdyn->midLoNr.lf.hw_ynrT_loNr_en;
    pCfg->rnr_en = pdyn->locYnrStrg.radiDist.sw_ynrT_radiDist_en;
    pCfg->tex2lo_strg_en = pdyn->midLoNr.lf.locYnrStrg_texRegion.hw_ynrT_tex2NrStrg_en;

    // REG: GAIN_CTRL
    tmp = (pdyn->locYnrStrg.locSgmStrg.hw_ynrT_glbSgmStrg_val) * (1 << 4);
    pCfg->global_set_gain = CLIP(tmp, 0, 0x3ff);
    tmp = (pdyn->locYnrStrg.locSgmStrg.hw_ynrT_glbSgmStrg_alpha) * (1 << 3);
    pCfg->gain_merge_alpha = CLIP(tmp, 0, 0xf);
    tmp = gain_base_inv_f * (1 << 7);  // {1i, 7f}, sqrt(1/4095/4)*128 = 1;
    pCfg->local_gain_scale = CLIP(tmp, 0, 0xff);

    {
        int lo_spnr_gain2strg[9];
        for (i = 0; i < 9; i++) {
            lo_spnr_gain2strg[i] = (int)(pdyn->midLoNr.locNrStrg.hw_ynrT_locSgmStrg2NrStrg_val[i] * (1 << 4));
        }
        // REG: GAIN_ADJ_0_2
        pCfg->lo_spnr_gain2strg[0] = CLIP(lo_spnr_gain2strg[0], 0, 0x1ff);
        pCfg->lo_spnr_gain2strg[1] = CLIP(lo_spnr_gain2strg[1], 0, 0x1ff);
        pCfg->lo_spnr_gain2strg[2] = CLIP(lo_spnr_gain2strg[2], 0, 0x1ff);
        // REG: GAIN_ADJ_3_5
        pCfg->lo_spnr_gain2strg[3] = CLIP(lo_spnr_gain2strg[3], 0, 0x1ff);
        pCfg->lo_spnr_gain2strg[4] = CLIP(lo_spnr_gain2strg[4], 0, 0x1ff);
        pCfg->lo_spnr_gain2strg[5] = CLIP(lo_spnr_gain2strg[5], 0, 0x1ff);
        // REG: GAIN_ADJ_6_8
        pCfg->lo_spnr_gain2strg[6] = CLIP(lo_spnr_gain2strg[6], 0, 0x1ff);
        pCfg->lo_spnr_gain2strg[7] = CLIP(lo_spnr_gain2strg[7], 0, 0x1ff);
        pCfg->lo_spnr_gain2strg[8] = CLIP(lo_spnr_gain2strg[8], 0, 0x1ff);
    }

    {
        float r_sq_inv = 16.0f / (cols * cols + rows * rows); // divide 2
        int* number_ptr = (int*)(&r_sq_inv);
        int EE = ((*number_ptr) >> 23) & (0x0ff);
        EE = -(EE - 127 + 1);
        int MM = (*number_ptr) & 0x7fffff;
        float tmpf = ((MM / (float)(1 << 23)) + 1) / 2;
        MM = (int)(256 * tmpf + 0.5);
        tmp = (MM << 5) + EE;
        // REG: RNR_MAX_R
        pCfg->rnr_max_radius = CLIP(tmp, 0, 0x3fff);
    }

    {
        int center_x = convert_coordinate(psta->locYnrStrg_radiDist.hw_ynrCfg_opticCenter_x, cols);
        int center_y = convert_coordinate(psta->locYnrStrg_radiDist.hw_ynrCfg_opticCenter_y, rows);
        // REG: RNR_CENTER_COOR
        pCfg->rnr_center_h = CLIP(center_x, 0, 0x1fff);
        pCfg->rnr_center_v = CLIP(center_y, 0, 0x1fff);
    }

    {
        int radius2strg[17];
        for (i = 0; i < 17; i++) {
            radius2strg[i] = (int)(pdyn->locYnrStrg.radiDist.hw_ynrT_radiDist2YnrStrg_val[i] * 16);
        }
        // REG: RNR_STRENGTH03
        pCfg->radius2strg[0] = CLIP(radius2strg[0], 0, 0xff);
        pCfg->radius2strg[1] = CLIP(radius2strg[1], 0, 0xff);
        pCfg->radius2strg[2] = CLIP(radius2strg[2], 0, 0xff);
        pCfg->radius2strg[3] = CLIP(radius2strg[3], 0, 0xff);
        // REG: RNR_STRENGTH47
        pCfg->radius2strg[4] = CLIP(radius2strg[4], 0, 0xff);
        pCfg->radius2strg[5] = CLIP(radius2strg[5], 0, 0xff);
        pCfg->radius2strg[6] = CLIP(radius2strg[6], 0, 0xff);
        pCfg->radius2strg[7] = CLIP(radius2strg[7], 0, 0xff);
        // REG: RNR_STRENGTH8b
        pCfg->radius2strg[8] = CLIP(radius2strg[8], 0, 0xff);
        pCfg->radius2strg[9] = CLIP(radius2strg[9], 0, 0xff);
        pCfg->radius2strg[10] = CLIP(radius2strg[10], 0, 0xff);
        pCfg->radius2strg[11] = CLIP(radius2strg[11], 0, 0xff);
        // REG: RNR_STRENGTHcf
        pCfg->radius2strg[12] = CLIP(radius2strg[12], 0, 0xff);
        pCfg->radius2strg[13] = CLIP(radius2strg[13], 0, 0xff);
        pCfg->radius2strg[14] = CLIP(radius2strg[14], 0, 0xff);
        pCfg->radius2strg[15] = CLIP(radius2strg[15], 0, 0xff);
        // REG: RNR_STRENGTH16
        pCfg->radius2strg[16] = CLIP(radius2strg[16], 0, 0xff);
    }

    for (i = 0; i < ISO_CURVE_POINT_NUM; i++) {
        tmp = pdyn->sigmaEnv.hw_ynrC_luma2Sigma_curve.idx[i];
        pCfg->luma2sima_x[i] = CLIP(tmp, 0, 0x7ff);;
        tmp = pdyn->sigmaEnv.hw_ynrC_luma2Sigma_curve.val[i] * (1 << NOISE_SIGMA_FIX_BIT);
        pCfg->luma2sima_y[i] = CLIP(tmp, 0, 0xfff);
    }

    float loFreqLumaNrCurvePoint[6] = {0, 32, 64, 128, 192, 256};
    float loFreqLumaNrCurveRatio[6];
    for (i = 0; i < 6; i++) {
        loFreqLumaNrCurveRatio[i] = pdyn->midLoNr.locNrStrg.hw_ynrT_luma2RgeSgm_scale[i];
        loFreqLumaNrCurvePoint[i] *= 4;
    }

    //update lo noise curve;
    for (i = 0; i < ISO_CURVE_POINT_NUM; i++) {
        float rate;

        int j = 0;
        for (j = 0; j < 6; j++) {
            if (pCfg->luma2sima_x[i] <= loFreqLumaNrCurvePoint[j])
                break;
        }

        if (j <= 0)
            rate = loFreqLumaNrCurveRatio[0];
        else if (j >= 6)
            rate = loFreqLumaNrCurveRatio[5];
        else
        {
            rate = ((float)pCfg->luma2sima_x[i] - loFreqLumaNrCurvePoint[j - 1]) / (loFreqLumaNrCurvePoint[j] - loFreqLumaNrCurvePoint[j - 1]);
            rate = loFreqLumaNrCurveRatio[j - 1] + rate * (loFreqLumaNrCurveRatio[j] - loFreqLumaNrCurveRatio[j - 1]);
        }
        pCfg->luma2sima_y[i] = MIN((int)(rate * pCfg->luma2sima_y[i]), 4095);
    }

    // REG: HI_SIGMA_GAIN
    tmp = (pdyn->hiNr.epf.hw_ynrT_rgeSgm_minLimit) * (1 << 11);
    pCfg->hi_spnr_sigma_min_limit = CLIP(tmp, 0, 0x7ff);
    tmp = (pdyn->hiNr.epf.hw_ynrT_locYnrStrg_alpha) * (1 << 4);
    pCfg->hi_spnr_local_gain_alpha = CLIP(tmp, 0, 0x1f);
    tmp = (pdyn->hiNr.epf.hw_ynrT_rgeSgm_scale) * (1 << 6);
    pCfg->hi_spnr_strg = CLIP(tmp, 0, 0x3ff);
    pCfg->hi_lp_en = psta->lowPowerCfg.hiNrLP.epfLP.hw_ynrT_lp_en;

    {
        int coeff[5];
        if (pdyn->hiNr.epf.sw_ynrT_filtCfg_mode == ynr_cfgByFiltStrg_mode) {
            float ynr_hi_gauss_sigma = pdyn->hiNr.epf.sw_ynrT_filtSpatial_strg;
            float w01_f = exp(-(0 + 1) / (2.0 * ynr_hi_gauss_sigma * ynr_hi_gauss_sigma));
            float w02_f = exp(-(0 + 4) / (2.0 * ynr_hi_gauss_sigma * ynr_hi_gauss_sigma));
            float w11_f = exp(-(1 + 1) / (2.0 * ynr_hi_gauss_sigma * ynr_hi_gauss_sigma));
            float w12_f = exp(-(1 + 4) / (2.0 * ynr_hi_gauss_sigma * ynr_hi_gauss_sigma));
            float w22_f = exp(-(4 + 4) / (2.0 * ynr_hi_gauss_sigma * ynr_hi_gauss_sigma));
            float w_sum_f = 1.0f + w01_f * 4 + w02_f * 4 + w11_f * 4 + w12_f * 8 + w22_f * 4;
            coeff[0] = (int)(w01_f / w_sum_f * 256 + 0.5f); // [0, 63]
            coeff[1] = (int)(w02_f / w_sum_f * 256 + 0.5f); // [0, 31]
            coeff[2] = (int)(w11_f / w_sum_f * 256 + 0.5f); // [0, 31]
            coeff[3] = (int)(w12_f / w_sum_f * 256 + 0.5f); // [0, 15]
            coeff[4] = (int)(w22_f / w_sum_f * 256 + 0.5f); // [0, 15]
        } else {
            for (i = 0; i < 5; i++)
                coeff[i] = pdyn->hiNr.epf.hw_ynrT_filtSpatial_wgt[i] * (1 << 8);
        }

        if (pCfg->hi_lp_en == 1) {
            coeff[3] = 0;
            coeff[4] = 0;
        }

        // REG: HI_GAUS_COE
        pCfg->hi_spnr_filt_coeff[0] = CLIP(coeff[0], 0, 0x3f);
        pCfg->hi_spnr_filt_coeff[1] = CLIP(coeff[1], 0, 0x3f);
        pCfg->hi_spnr_filt_coeff[2] = CLIP(coeff[2], 0, 0x3f);
        pCfg->hi_spnr_filt_coeff[3] = CLIP(coeff[3], 0, 0x3f);
        pCfg->hi_spnr_filt_coeff[4] = CLIP(coeff[4], 0, 0x3f);
    }

    // REG: HI_WEIGHT
    tmp = (pdyn->hiNr.epf.hw_ynrT_rgeWgt_negOff) * (1 << 10);
    pCfg->hi_spnr_filt_wgt_offset = CLIP(tmp, 0, 0x3ff);
    tmp = (pdyn->hiNr.epf.hw_ynrT_centerPix_wgt) * (1 << 12);
    pCfg->hi_spnr_filt_center_wgt = CLIP(tmp, 1, 0x1fff);

    {
        int coeff[6];
        if (pdyn->hiNr.sf.sw_ynrT_filtCfg_mode == ynr_cfgByFiltStrg_mode) {
            float ynr_hiFilt1_gauss_sigma = pdyn->hiNr.sf.sw_ynrT_filtSpatial_strg;
            float filt1_w01_f = exp(-(0 + 1) / (2.0 * ynr_hiFilt1_gauss_sigma * ynr_hiFilt1_gauss_sigma));
            float filt1_w02_f = exp(-(0 + 4) / (2.0 * ynr_hiFilt1_gauss_sigma * ynr_hiFilt1_gauss_sigma));
            float filt1_w11_f = exp(-(1 + 1) / (2.0 * ynr_hiFilt1_gauss_sigma * ynr_hiFilt1_gauss_sigma));
            float filt1_w12_f = exp(-(1 + 4) / (2.0 * ynr_hiFilt1_gauss_sigma * ynr_hiFilt1_gauss_sigma));
            float filt1_w22_f = exp(-(4 + 4) / (2.0 * ynr_hiFilt1_gauss_sigma * ynr_hiFilt1_gauss_sigma));
            float filt1_w_sum_f = 1.0f + filt1_w01_f * 4 + filt1_w02_f * 4 + filt1_w11_f * 4 + filt1_w12_f * 8 + filt1_w22_f * 4;
            int g_w01 = (int)(filt1_w01_f / filt1_w_sum_f * 256); // [0, 63]
            int g_w02 = (int)(filt1_w02_f / filt1_w_sum_f * 256); // [0, 31]
            int g_w11 = (int)(filt1_w11_f / filt1_w_sum_f * 256); // [0, 31]
            int g_w12 = (int)(filt1_w12_f / filt1_w_sum_f * 256); // [0, 15]
            int g_w22 = (int)(filt1_w22_f / filt1_w_sum_f * 256); // [0, 15]
            int g_w00 = 256 - 4 * g_w01 - 4 * g_w02 - 4 * g_w11 - 8 * g_w12 - 4 * g_w22;
            coeff[0] = g_w00;
            coeff[1] = g_w01;
            coeff[2] = g_w02;
            coeff[3] = g_w11;
            coeff[4] = g_w12;
            coeff[5] = g_w22;
        } else {
            for (i = 0; i < 6; i++)
                coeff[i] = pdyn->hiNr.sf.hw_ynrT_filtSpatial_wgt[i] * (1 << 8);
        }
        // REG: HI_GAUS1_COE_0_2
        pCfg->hi_spnr_filt1_coeff[0] = CLIP(coeff[0], 0, 0x1ff);
        pCfg->hi_spnr_filt1_coeff[1] = CLIP(coeff[1], 0, 0x1ff);
        pCfg->hi_spnr_filt1_coeff[2] = CLIP(coeff[2], 0, 0x1ff);
        // REG: HI_GAUS1_COE_3_5
        pCfg->hi_spnr_filt1_coeff[3] = CLIP(coeff[3], 0, 0x1ff);
        pCfg->hi_spnr_filt1_coeff[4] = CLIP(coeff[4], 0, 0x1ff);
        pCfg->hi_spnr_filt1_coeff[5] = CLIP(coeff[5], 0, 0x1ff);
    }
    // REG: HI_TEXT
    tmp = (pdyn->hiNr.sfAlphaEpf_baseTex.hw_ynrT_maxAlphaTex_maxThred) * (1 << 8);
    pCfg->hi_spnr_filt1_tex_thred = CLIP(tmp, 0, 0x7ff);
    tmp = (pdyn->hiNr.sfAlphaEpf_baseTex.sw_ynr_texIdx_scale) * (1 << 3);
    pCfg->hi_spnr_filt1_tex_scale = CLIP(tmp, 0, 0x3ff);
    tmp = (pdyn->hiNr.sfAlphaEpf_baseTex.sw_ynr_sfAlpha_scale) * (1 << 8);
    pCfg->hi_spnr_filt1_wgt_alpha = CLIP(tmp, 0, 0x1ff);

    {
        int coeff[3];
        if (pdyn->midLoNr.mf.sw_ynrT_filtCfg_mode == ynr_cfgByFiltStrg_mode) {
            float ynr_mi_gauss_sigma = pdyn->midLoNr.mf.sw_ynr_filtSpatial_strg;
            float m_w00_f = 1.0f;
            float m_w01_f = exp(-(0 + 4) / (1.0f * ynr_mi_gauss_sigma * ynr_mi_gauss_sigma));
            float m_w11_f = exp(-(4 + 4) / (1.0f * ynr_mi_gauss_sigma * ynr_mi_gauss_sigma));
            float m_w_sum_f = m_w00_f + m_w01_f * 4 +  m_w11_f * 4;
            int m_w01 = (int)(m_w01_f / m_w_sum_f * 256);
            int m_w11 = (int)(m_w11_f / m_w_sum_f * 256);
            coeff[0] = 256 - m_w01 * 4 - m_w11 * 4;
            coeff[1] = m_w01;
            coeff[2] = m_w11;
        } else {
            for (i = 0; i < 3; i++)
                coeff[i] = pdyn->midLoNr.mf.sw_ynr_filtSpatial_wgt[i] * (1 << 8);
        }
        // REG: MI_GAUS_COE
        pCfg->mi_spnr_filt_coeff0 = CLIP(coeff[0], 0, 0x1ff);
        pCfg->mi_spnr_filt_coeff1 = CLIP(coeff[1], 0, 0x1ff);
        pCfg->mi_spnr_filt_coeff2 = CLIP(coeff[2], 0, 0x1ff);
    }
    // REG: MI_STRG_DETAIL
    tmp = (pdyn->midLoNr.mf.sw_ynr_rgeSgm_scale) * (1 << 6);
    pCfg->mi_spnr_strg = CLIP(tmp, 0, 0x3ff);
    tmp = (pdyn->midLoNr.mf.hw_ynrT_softThd_scale) * (1 << 10);
    pCfg->mi_spnr_soft_thred_scale = CLIP(tmp, 0, 0xfff);
    // REG: MI_WEIGHT
    tmp = (pdyn->midLoNr.mf.hw_ynrT_midNrOut_alpha) * (1 << 7);
    pCfg->mi_spnr_wgt = CLIP(tmp, 0, 0xff);
    tmp = (pdyn->midLoNr.mf.sw_ynr_centerPix_wgt) * (1 << 10);
    pCfg->mi_spnr_filt_center_wgt = CLIP(tmp, 0, 0x7ff);
    if (pdyn->midLoNr.mf.hw_ynrT_alphaMfTex_scale == 1.0f) {
        pCfg->mi_ehance_scale_en = 0;
    } else {
        pCfg->mi_ehance_scale_en = 1;
    }
    tmp = (pdyn->midLoNr.mf.hw_ynrT_alphaMfTex_scale) * 8;
    pCfg->mi_ehance_scale = CLIP(tmp, 0, 255);

    // REG: LO_STRG_DETAIL
    tmp = (pdyn->midLoNr.lf.epf.hw_ynrT_rgeSgm_scale) * (1 << 6);
    pCfg->lo_spnr_strg = CLIP(tmp, 0, 0x3ff);
    tmp = (pdyn->midLoNr.lf.epf.hw_ynrT_softThd_scale) * (1 << 10);
    pCfg->lo_spnr_soft_thred_scale = CLIP(tmp, 0, 0xfff);
    // REG: LO_LIMIT_SCALE
    tmp = (pdyn->midLoNr.lf.epf.hw_ynrT_guideSoftThd_scale) * (1 << 6);
    pCfg->lo_spnr_thumb_thred_scale = CLIP(tmp, 0, 0x3ff);
    // REG: LO_WEIGHT
    tmp = (pdyn->midLoNr.lf.epf.hw_ynrT_loNrOut_alpha) * (1 << 7);
    pCfg->lo_spnr_wgt = CLIP(tmp, 0, 0x80);
    tmp = (pdyn->midLoNr.lf.epf.hw_ynrT_centerPix_wgt) * (1 << 8) * 6;
    pCfg->lo_spnr_filt_center_wgt = CLIP(tmp, 1, 0x1fff);

    // REG: LO_TEXT_THRED
    tmp = (pdyn->midLoNr.lf.locYnrStrg_texRegion.hw_ynrT_flatRegion_maxThred) * (1023);
    pCfg->tex2lo_strg_lower_thred = CLIP(tmp, 0, 1024);
    tmp = (pdyn->midLoNr.lf.locYnrStrg_texRegion.hw_ynrT_edgeRegion_minThred) * (1023);
    pCfg->tex2lo_strg_upper_thred = CLIP(tmp, 0, 1024);
    int tex2strg_step = (1 << 20) / MAX(pCfg->tex2lo_strg_upper_thred - pCfg->tex2lo_strg_lower_thred, 1);
    int ynr_tex2loStrg_minLimit   = (int)(pdyn->midLoNr.lf.locYnrStrg_texRegion.sw_ynrT_edgeRegionNr_strg * 1024);     // range : [0, 1024]; 11 bits
    int tex_lo_strg_slop = 1024 - ynr_tex2loStrg_minLimit; // [0, 1024]
    int pos_idx = find_top_one_pos_v3(tex2strg_step);
    int tex2strg_step_exponent = MAX(pos_idx - 11, 0);                            // [0, 10]
    int tex2strg_step_mantissa = tex2strg_step >> tex2strg_step_exponent;         // [1,1025]
    tex2strg_step_exponent = 20 - tex2strg_step_exponent;  // [10, 20]

    pCfg->tex2lo_strg_mantissa = (tex_lo_strg_slop * tex2strg_step_mantissa) >> 10;  //  [0, 1024]x[1024,2047]->[0, 2047]
    pCfg->tex2lo_strg_exponent = tex2strg_step_exponent - 10;  // [0, 10]

    for (i = 0; i < 9; i++) {
        tmp = pdyn->midLoNr.locNrStrg.hw_ynrT_locSgmStrg2NrOut_alpha[i] * (1 << 7);
        pCfg->lo_gain2wgt[i] = CLIP(tmp, 0, 128);
    }

    return;
}
