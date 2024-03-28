
#include "rk_aiq_aynr_algo_ynr_v22.h"

RKAIQ_BEGIN_DECLARE

Aynr_result_V22_t ynr_select_params_by_ISO_V22(RK_YNR_Params_V22_t *pParams, RK_YNR_Params_V22_Select_t *pSelect, Aynr_ExpInfo_V22_t *pExpInfo)
{
    float ratio = 0.0f;
    int iso = 50;
    RK_YNR_Params_V22_Select_t *pParamHi = NULL;
    RK_YNR_Params_V22_Select_t *pParamLo = NULL;
    RK_YNR_Params_V22_Select_t* pParamTmp = NULL;


    Aynr_result_V22_t res = AYNRV22_RET_SUCCESS;

    if(pParams == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNRV22_RET_NULL_POINTER;
    }

    if(pSelect == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNRV22_RET_NULL_POINTER;
    }

    if(pExpInfo == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNRV22_RET_NULL_POINTER;
    }

    iso = pExpInfo->arIso[pExpInfo->hdr_mode];

    // choose integer type data
    int cur_iso_idx = 0;
    int idx = 0;
    for (idx = 0; idx < RK_YNR_V22_MAX_ISO_NUM; idx++) {
        if (iso < pParams->iso[idx]) {
            if (idx == 0) {
                cur_iso_idx = 0;
                break;
            } else {
                int dist1 = iso - pParams->iso[idx - 1];
                int dist2 = pParams->iso[idx] - iso;
                cur_iso_idx = (dist1 > dist2) ? (idx) : (idx - 1);
                break;
            }
        }
    }
    if (idx == RK_YNR_V22_MAX_ISO_NUM)
        cur_iso_idx = RK_YNR_V22_MAX_ISO_NUM - 1;


    pParamTmp = &pParams->arYnrParamsISO[cur_iso_idx];

    pSelect->enable = pParams->enable;
    pSelect->ynr_bft3x3_bypass = pParamTmp->ynr_bft3x3_bypass;
    pSelect->ynr_lbft5x5_bypass = pParamTmp->ynr_lbft5x5_bypass;
    pSelect->ynr_lgft3x3_bypass = pParamTmp->ynr_lgft3x3_bypass;
    pSelect->ynr_flt1x1_bypass = pParamTmp->ynr_flt1x1_bypass;
    pSelect->ynr_nlm11x11_bypass = pParamTmp->ynr_nlm11x11_bypass;
    pSelect->ynr_thumb_mix_cur_en = pParamTmp->ynr_thumb_mix_cur_en;

    pSelect->hi_filter_coeff1_1 = pParamTmp->hi_filter_coeff1_1;
    pSelect->hi_filter_coeff1_2 = pParamTmp->hi_filter_coeff1_2;
    pSelect->hi_filter_coeff1_3 = pParamTmp->hi_filter_coeff1_3;
    pSelect->hi_filter_coeff2_1 = pParamTmp->hi_filter_coeff2_1;
    pSelect->hi_filter_coeff2_2 = pParamTmp->hi_filter_coeff2_2;
    pSelect->hi_filter_coeff2_3 = pParamTmp->hi_filter_coeff2_3;

    int iso_div = 50;
    int lowIso = 50;
    int highIso = 50;
    int minIso = 50;
    int maxIso = 50;
    int isoLevelLow = 0;
    int isoLevelHig = 0;

    for(int i = 0; i < RK_YNR_V22_MAX_ISO_NUM - 1; i++) {
#ifndef RK_SIMULATOR_HW
        lowIso = pParams->iso[i];
        highIso = pParams->iso[i + 1];
#else
        lowIso = iso_div * (1 << i);
        highIso = iso_div * (1 << (i + 1));
#endif
        if(iso >= lowIso && iso <= highIso) {
            ratio = (iso - lowIso ) / (float)(highIso - lowIso);
            pParamLo = &pParams->arYnrParamsISO[i];
            pParamHi = &pParams->arYnrParamsISO[i + 1];
            isoLevelLow = i;
            isoLevelHig = i + 1;
            break;
        }
    }

#ifndef RK_SIMULATOR_HW
    minIso = pParams->iso[0];
    maxIso = pParams->iso[RK_YNR_V22_MAX_ISO_NUM - 1];
#else
    minIso = iso_div * (1 << 0);
    maxIso = iso_div * (1 << (RK_YNR_V22_MAX_ISO_NUM - 1));
#endif

    if(iso < minIso) {
        ratio = 0;
        pParamLo = &pParams->arYnrParamsISO[0];
        pParamHi = &pParams->arYnrParamsISO[1];
        isoLevelLow = 0;
        isoLevelHig = 1;
    }

    if(iso > maxIso) {
        ratio = 1;
        pParamLo = &pParams->arYnrParamsISO[RK_YNR_V22_MAX_ISO_NUM - 2];
        pParamHi = &pParams->arYnrParamsISO[RK_YNR_V22_MAX_ISO_NUM - 1];
        isoLevelLow = RK_YNR_V22_MAX_ISO_NUM - 2;
        isoLevelHig = RK_YNR_V22_MAX_ISO_NUM - 1;
    }


    LOGD_ANR("oyyf %s:%d  iso:%d low:%d hight:%d ratio:%f iso_index:%d \n", __FUNCTION__, __LINE__,
             iso, lowIso, highIso, ratio, cur_iso_idx);

    pExpInfo->isoLevelLow = isoLevelLow;
    pExpInfo->isoLevelHig = isoLevelHig;
    //global gain local gain cfg
    pSelect->ynr_global_gain_alpha = ratio * (pParamHi->ynr_global_gain_alpha - pParamLo->ynr_global_gain_alpha) + pParamLo->ynr_global_gain_alpha;
    pSelect->ynr_global_gain       = ratio * (pParamHi->ynr_global_gain - pParamLo->ynr_global_gain) + pParamLo->ynr_global_gain;
    pSelect->ynr_adjust_thresh = ratio * (pParamHi->ynr_adjust_thresh - pParamLo->ynr_adjust_thresh) + pParamLo->ynr_adjust_thresh;
    pSelect->ynr_adjust_scale = ratio * (pParamHi->ynr_adjust_scale - pParamLo->ynr_adjust_scale) + pParamLo->ynr_adjust_scale;

    // get rnr parameters
    for (int i = 0; i < 17; i++)
    {
        pSelect->rnr_strength[i] = ratio * (pParamHi->rnr_strength[i] - pParamLo->rnr_strength[i]) + pParamLo->rnr_strength[i];
    }

    // get the parameters for current ISO
    // ci
    pSelect->lci = ratio * (pParamHi->lci - pParamLo->lci) + pParamLo->lci;
    pSelect->hci = ratio * (pParamHi->hci - pParamLo->hci) + pParamLo->hci;

    // noise curve
    for (int i = 0; i < YNR_V22_ISO_CURVE_POINT_NUM; i++)
    {
        pSelect->sigma[i] = ratio * (pParamHi->sigma[i] - pParamLo->sigma[i]) + pParamLo->sigma[i];
        pSelect->lumaPoint[i] = (short)(ratio * (pParamHi->lumaPoint[i] - pParamLo->lumaPoint[i]) + pParamLo->lumaPoint[i]);
    }

    for (int i = 0; i < 6; i++)
    {
        pSelect->lo_lumaPoint[i] = ratio * (pParamHi->lo_lumaPoint[i] - pParamLo->lo_lumaPoint[i]) + pParamLo->lo_lumaPoint[i];
        pSelect->lo_ratio[i] = ratio * (pParamHi->lo_ratio[i] - pParamLo->lo_ratio[i]) + pParamLo->lo_ratio[i];

        pSelect->lo_lumaPoint[i] *= 4; // curve point 8 bits -> 10 bits
    }

    // lo bf
    pSelect->low_bf1 = ratio * (pParamHi->low_bf1 - pParamLo->low_bf1) + pParamLo->low_bf1;
    pSelect->low_bf2 = ratio * (pParamHi->low_bf2 - pParamLo->low_bf2) + pParamLo->low_bf2;


    pSelect->low_thred_adj = ratio * (pParamHi->low_thred_adj - pParamLo->low_thred_adj) + pParamLo->low_thred_adj;
    pSelect->low_peak_supress = ratio * (pParamHi->low_peak_supress - pParamLo->low_peak_supress) + pParamLo->low_peak_supress;
    pSelect->low_edge_adj_thresh = ratio * (pParamHi->low_edge_adj_thresh - pParamLo->low_edge_adj_thresh) + pParamLo->low_edge_adj_thresh;
    pSelect->low_lbf_weight_thresh = ratio * (pParamHi->low_lbf_weight_thresh - pParamLo->low_lbf_weight_thresh) + pParamLo->low_lbf_weight_thresh;
    pSelect->low_center_weight = ratio * (pParamHi->low_center_weight - pParamLo->low_center_weight) + pParamLo->low_center_weight;
    pSelect->low_dist_adj = ratio * (pParamHi->low_dist_adj - pParamLo->low_dist_adj) + pParamLo->low_dist_adj;
    pSelect->low_weight = ratio * (pParamHi->low_weight - pParamLo->low_weight) + pParamLo->low_weight;

    pSelect->low_filt1_strength = ratio * (pParamHi->low_filt1_strength - pParamLo->low_filt1_strength) + pParamLo->low_filt1_strength;
    pSelect->low_filt2_strength = ratio * (pParamHi->low_filt2_strength - pParamLo->low_filt2_strength) + pParamLo->low_filt2_strength;

    pSelect->low_bi_weight = ratio * (pParamHi->low_bi_weight - pParamLo->low_bi_weight) + pParamLo->low_bi_weight;

    // High Freq
    pSelect->hi_weight_offset = ratio * (pParamHi->hi_weight_offset - pParamLo->hi_weight_offset) + pParamLo->hi_weight_offset;
    pSelect->hi_center_weight = ratio * (pParamHi->hi_center_weight - pParamLo->hi_center_weight) + pParamLo->hi_center_weight;
    pSelect->hi_bf_scale = ratio * (pParamHi->hi_bf_scale - pParamLo->hi_bf_scale) + pParamLo->hi_bf_scale;
    pSelect->hi_min_sigma = ratio * (pParamHi->hi_min_sigma - pParamLo->hi_min_sigma) + pParamLo->hi_min_sigma;
    pSelect->hi_nr_weight = ratio * (pParamHi->hi_nr_weight - pParamLo->hi_nr_weight) + pParamLo->hi_nr_weight;
    pSelect->hi_gain_alpha = ratio * (pParamHi->hi_gain_alpha - pParamLo->hi_gain_alpha) + pParamLo->hi_gain_alpha;


    for (int i = 0; i < 6; i++)
    {
        pSelect->hi_lumaPoint[i] = ratio * (pParamHi->hi_lumaPoint[i] - pParamLo->hi_lumaPoint[i]) + pParamLo->hi_lumaPoint[i];
        pSelect->hi_ratio[i] = ratio * (pParamHi->hi_ratio[i] - pParamLo->hi_ratio[i]) + pParamLo->hi_ratio[i];

        pSelect->hi_lumaPoint[i] *= 4; // curve point 8 bits -> 10 bits
    }

    return res;
}

Aynr_result_V22_t ynr_fix_transfer_V22(RK_YNR_Params_V22_Select_t* pSelect, RK_YNR_Fix_V22_t *pFix, rk_aiq_ynr_strength_v22_t* pStrength, Aynr_ExpInfo_V22_t *pExpInfo)
{
    LOGI_ANR("%s:(%d) enter \n", __FUNCTION__, __LINE__);

    Aynr_result_V22_t res = AYNRV22_RET_SUCCESS;
    int w0, w1, w2;
    int tmp;

    if(pSelect == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNRV22_RET_NULL_POINTER;
    }

    if(pFix == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNRV22_RET_NULL_POINTER;
    }

    if(pExpInfo == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNRV22_RET_NULL_POINTER;
    }

    if(pStrength == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNRV22_RET_NULL_POINTER;
    }

    float fStrength = 1.0;

    if(pStrength->strength_enable) {
        fStrength = pStrength->percent;
    }
    if(fStrength <= 0.0) {
        fStrength = 0.000001;
    }

    LOGD_ANR("strength_enable:%d fStrength: %f \n", pStrength->strength_enable, fStrength);

    LOGD_ANR("%s:%d strength:%f raw:width:%d height:%d\n",
             __FUNCTION__, __LINE__,
             fStrength, pExpInfo->rawHeight, pExpInfo->rawWidth);

    // YNR_2700_GLOBAL_CTRL (0x0000)
    pFix->rnr_en = 1;
    pFix->gate_dis = 0;
    pFix->thumb_mix_cur_en = pSelect->ynr_thumb_mix_cur_en;
    tmp = (int)(pSelect->ynr_global_gain_alpha * (1 << 3));
    pFix->global_gain_alpha = CLIP(tmp, 0, 8);
    tmp = (int)(pSelect->ynr_global_gain * (1 << 4));
    pFix->global_gain  = CLIP(tmp, 0, 1023);
    pFix->flt1x1_bypass_sel = 0;
    pFix->nlm11x11_bypass = pSelect->ynr_nlm11x11_bypass;
    pFix->flt1x1_bypass = pSelect->ynr_flt1x1_bypass;
    pFix->lgft3x3_bypass = pSelect->ynr_lgft3x3_bypass;
    pFix->lbft5x5_bypass = pSelect->ynr_lbft5x5_bypass;
    pFix->bft3x3_bypass = pSelect->ynr_bft3x3_bypass;
    pFix->ynr_en = pSelect->enable;

    // YNR_2700_RNR_MAX_R  (0x0004)
    int rows = pExpInfo->rawHeight; //raw height
    int cols = pExpInfo->rawWidth; //raw  width
    float r_sq_inv = 16.0f / (cols * cols + rows * rows); // divide 2
    int* number_ptr = (int*)(&r_sq_inv);
    int EE = ((*number_ptr) >> 23) & (0x0ff);
    EE = -(EE - 127 + 1);
    int MM = (*number_ptr) & 0x7fffff;
    float tmp2 = ((MM / float(1 << 23)) + 1) / 2;
    MM = int(256 * tmp2 + 0.5);
    tmp = (MM << 5) + EE;
    pFix->rnr_max_r = CLIP(tmp, 0, 0x3fff);
    //local gain scale
    //tmp = ( sqrt(double(50) / pExpInfo->arIso[pExpInfo->hdr_mode])) * (1 << 7);  //old
    tmp = (1.0) * (1 << 7);
    pFix->local_gainscale = CLIP(tmp, 0, 0x80);

    //// YNR_2700_CENTRE_COOR (0x0008)
    pFix->rnr_center_coorv = rows / 2;
    pFix->rnr_center_coorh = cols / 2;

    //// YNR_2700_CENTRE_COOR (0x000c)
    tmp = pSelect->ynr_adjust_scale * (1 << 4);
    pFix->localgain_adj = CLIP(tmp, 0, 0xff);
    //tmp = pSelect->ynr_adjust_thresh * 16;  //new
    tmp = pSelect->ynr_adjust_thresh * 16;
    pFix->localgain_adj_thresh = CLIP(tmp, 0, 0x3ff);;

    // YNR_2700_LOWNR_CTRL0 (0x0010)
    tmp = (int)(1.0f / pSelect->low_bf2 / fStrength * (1 << 9));
    pFix->low_bf_inv1 = CLIP(tmp, 0, 0x3fff);
    tmp = (int)(1.0f / pSelect->low_bf1 / fStrength * (1 << 9));
    pFix->low_bf_inv0 = CLIP(tmp, 0, 0x3fff);


    // YNR_2700_LOWNR_CTRL1  (0x0014)
    tmp = (int)(pSelect->low_peak_supress / fStrength * (1 << 7));
    pFix->low_peak_supress = CLIP(tmp, 0, 0x80);
    tmp = (int)(pSelect->low_thred_adj * fStrength * (1 << 6));
    pFix->low_thred_adj = CLIP(tmp, 0, 0x7ff);

    // YNR_2700_LOWNR_CTRL2 (0x0018)
    tmp = (int)(pSelect->low_dist_adj * (1 << 2));
    pFix->low_dist_adj = CLIP(tmp, 0, 0x1ff);
    tmp = (int)(pSelect->low_edge_adj_thresh);
    pFix->low_edge_adj_thresh = CLIP(tmp, 0, 0x3ff);


    // YNR_2700_LOWNR_CTRL3 (0x001c)
    tmp = (int)(pSelect->low_bi_weight * fStrength * (1 << 7));
    pFix->low_bi_weight = CLIP(tmp, 0, 0x80);
    tmp = (int)(pSelect->low_weight *  fStrength * (1 << 7));
    pFix->low_weight = CLIP(tmp, 0, 0x80);
    tmp = (int)(pSelect->low_center_weight / fStrength * (1 << 10));
    pFix->low_center_weight = CLIP(tmp, 0, 0x400);

    // YNR_2700_LOWNR_CTRL4 (0x002c)
    tmp = (int)(0x0000);
    pFix->frame_full_size = CLIP(tmp, 0, 0x3fff);
    tmp = (int)(pSelect->low_lbf_weight_thresh * 1023);
    pFix->lbf_weight_thres = CLIP(tmp, 0, 0x3ff);


    // YNR_2700_GAUSS1_COEFF (0x0030)
    float filter1_sigma = pSelect->low_filt1_strength;
    float filt1_coeff1 = exp(-1 / (2 * filter1_sigma * filter1_sigma));
    float filt1_coeff0 = filt1_coeff1 * filt1_coeff1;
    float coeff1_sum = 1 + 4 * filt1_coeff1 + 4 * filt1_coeff0;
    w0 = int(filt1_coeff0 / coeff1_sum * 256 + 0.5);
    w1 = int(filt1_coeff1 / coeff1_sum * 256 + 0.5);
    w2 = 256 - w0 * 4 - w1 * 4;
    tmp = (int)(w2);
    pFix->low_gauss1_coeff2 = CLIP(tmp, 0, 0x1ff);
    tmp = (int)(w1);
    pFix->low_gauss1_coeff1 = CLIP(tmp, 0, 0x3f);
    tmp = (int)(w0);
    pFix->low_gauss1_coeff0 = CLIP(tmp, 0, 0x3f);

    // YNR_2700_GAUSS2_COEFF (0x0034)
    float filter2_sigma = pSelect->low_filt2_strength;
    float filt2_coeff1 = exp(-1 / (2 * filter2_sigma * filter2_sigma));
    float filt2_coeff0 = filt2_coeff1 * filt2_coeff1;
    float coeff2_sum = 1 + 4 * filt2_coeff1 + 4 * filt2_coeff0;
    w0 = int(filt2_coeff0 / coeff2_sum * 256 + 0.5);
    w1 = int(filt2_coeff1 / coeff2_sum * 256 + 0.5);
    w2 = 256 - w0 * 4 - w1 * 4;
    tmp = (int)(w2);
    pFix->low_gauss2_coeff2 = CLIP(tmp, 0, 0x1ff);
    tmp = (int)(w1);
    pFix->low_gauss2_coeff1 = CLIP(tmp, 0, 0x3f);
    tmp = (int)(w0);
    pFix->low_gauss2_coeff0 = CLIP(tmp, 0, 0x3f);


    // YNR_2700_SGM_DX_0_1 (0x0040 - 0x0060)
    // YNR_2700_LSGM_Y_0_1 (0x0070- 0x0090)
    // YNR_2700_HSGM_Y_0_1 (0x00a0- 0x00c0)
    for (int i = 0; i < YNR_V22_ISO_CURVE_POINT_NUM; i++) {
        tmp = pSelect->lumaPoint[i];
        pFix->luma_points_x[i] = CLIP(tmp, 0, 0x400);
        tmp = (int)(pSelect->sigma[i] * pSelect->lci * (1 << YNR_V22_NOISE_SIGMA_FIX_BIT));
        pFix->lsgm_y[i] = CLIP(tmp, 0, 0xfff);
        //tmp = (int)(pSelect->sigma[i] * pSelect->hci * (1 << YNR_V22_NOISE_SIGMA_FIX_BIT));
        //pFix->hsgm_y[i] = CLIP(tmp, 0, 0xfff);
    }

    float lo_lumaPoint[6];
    float lo_ratio[6];
    float hi_lumaPoint[6];
    float hi_ratio[6];
    for (int i = 0; i < 6; i++) {
        lo_lumaPoint[i] = pSelect->lo_lumaPoint[i];
        lo_ratio[i] = pSelect->lo_ratio[i];
        hi_lumaPoint[i] = pSelect->hi_lumaPoint[i];
        hi_ratio[i] = pSelect->hi_ratio[i];
    }

    //update lo noise curve;
    for (int i = 0; i < YNR_V22_ISO_CURVE_POINT_NUM; i++) {
        float rate;
        int j = 0;
        for (j = 0; j < 6; j++) {
            if (pFix->luma_points_x[i] <= lo_lumaPoint[j])
                break;
        }

        if (j <= 0)
            rate = lo_ratio[0];
        else if (j >= 6)
            rate = lo_ratio[5];
        else {
            rate = ((float)pFix->luma_points_x[i] - lo_lumaPoint[j - 1]) / (lo_lumaPoint[j] - lo_lumaPoint[j - 1]);
            rate = lo_ratio[j - 1] + rate * (lo_ratio[j] - lo_ratio[j - 1]);
        }
        tmp = (int)(rate * pFix->lsgm_y[i]);
        pFix->lsgm_y[i] = CLIP(tmp, 0, 0xfff);
    }

#if 0
    //update hi noise curve;
    for (int i = 0; i < YNR_V22_ISO_CURVE_POINT_NUM; i++) {
        float rate;
        int j;
        for (j = 0; j < 6; j++) {
            if (pFix->ynr_luma_points_x[i] <= hi_lumaPoint[j])
                break;
        }

        if (j <= 0)
            rate = hi_ratio[0];
        else if (j >= 6)
            rate = hi_ratio[5];
        else {
            rate = ((float)pFix->ynr_luma_points_x[i] - hi_lumaPoint[j - 1]) / (hi_lumaPoint[j] - hi_lumaPoint[j - 1]);
            rate = hi_ratio[j - 1] + rate * (hi_ratio[j] - hi_ratio[j - 1]);
        }
        tmp = (int)(rate * pFix->ynr_hsgm_y[i]);
        pFix->ynr_hsgm_y[i] = CLIP(tmp, 0, 0xfff);
    }
#endif

    // YNR_2700_RNR_STRENGTH03 (0x00d0- 0x00e0)
    for (int i = 0; i < 17; i++) {
        tmp = int(pSelect->rnr_strength[i] * 16);
        pFix->rnr_strength[i] = CLIP(tmp, 0, 0xff);
    }

    // YNR_NLM_SIGMA_GAIN  (0x00f0)
    tmp = (int)(pSelect->hi_min_sigma * (1 << 11));
    pFix->nlm_min_sigma = CLIP(tmp, 0, 0x7ff);
    tmp = (int)(pSelect->hi_gain_alpha * (1 << 4));
    pFix->nlm_hi_gain_alpha = CLIP(tmp, 0, 0x1f);
    tmp = (int)(pSelect->hi_bf_scale * fStrength * (1 << 6));
    pFix->nlm_hi_bf_scale = CLIP(tmp, 0, 0x3ff);

    // YNR_NLM_COE  (0x00f4)
    tmp = (int)(pSelect->hi_filter_coeff1_1);
    pFix->nlm_coe_0 = CLIP(tmp, 0, 0xf);
    tmp = (int)(pSelect->hi_filter_coeff1_2);
    pFix->nlm_coe_1 = CLIP(tmp, 0, 0xf);
    tmp = (int)(pSelect->hi_filter_coeff1_3);
    pFix->nlm_coe_2 = CLIP(tmp, 0, 0xf);
    tmp = (int)(pSelect->hi_filter_coeff2_1);
    pFix->nlm_coe_3 = CLIP(tmp, 0, 0xf);
    tmp = (int)(pSelect->hi_filter_coeff2_2);
    pFix->nlm_coe_4 = CLIP(tmp, 0, 0xf);
    tmp = (int)(pSelect->hi_filter_coeff2_3);
    pFix->nlm_coe_5 = CLIP(tmp, 0, 0xf);

    //YNR_NLM_WEIGHT (0x00f8)
    tmp = (int)(pSelect->hi_center_weight / fStrength * (1 << 10));
    pFix->nlm_center_weight = CLIP(tmp, 0, 0x3ffff);
    tmp = (int)(pSelect->hi_weight_offset * (1 << 10));
    pFix->nlm_weight_offset = CLIP(tmp, 0, 0x3ff);

    // YNR_NLM_NR_WEIGHT (0x00fc)
    tmp = (int)(pSelect->hi_nr_weight * fStrength * (1 << 10));
    pFix->nlm_nr_weight = CLIP(tmp, 0, 0x400);

    ynr_fix_printf_V22(pFix);
    return res;
}

Aynr_result_V22_t ynr_fix_printf_V22(RK_YNR_Fix_V22_t * pFix)
{
    LOGD_ANR("%s:(%d) enter \n", __FUNCTION__, __LINE__);

    Aynr_result_V22_t res = AYNRV22_RET_SUCCESS;

    if(pFix == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNRV22_RET_NULL_POINTER;
    }

    // YNR_2700_GLOBAL_CTRL (0x0000)
    LOGD_ANR("(0x0000) sw_ynr_thumb_mix_cur_en:0x%x sw_ynr_gate_dis:0x%x sw_ynr_rnr_en:0x%x\n",
             pFix->thumb_mix_cur_en,
             pFix->gate_dis,
             pFix->rnr_en);

    // YNR_2700_GLOBAL_CTRL (0x0000)
    LOGD_ANR("(0x0000) ynr_global_gain_alpha:0x%x ynr_global_gain:0x%x \n ynr_flt1x1_bypass_sel:0x%x  ynr_nlm11x11_bypass:0x%x \n ynr_flt1x1_bypass:0x%x  ynr_lgft3x3_bypass:0x%x \n ynr_lbft5x5_bypass:0x%x  ynr_bft3x3_bypass:0x%x \n ynr_en:0x%x\n",
             pFix->global_gain_alpha,
             pFix->global_gain,
             pFix->flt1x1_bypass_sel,
             pFix->nlm11x11_bypass,
             pFix->flt1x1_bypass,
             pFix->lgft3x3_bypass,
             pFix->lbft5x5_bypass,
             pFix->bft3x3_bypass,
             pFix->ynr_en);


    // YNR_2700_RNR_MAX_R  (0x0004)
    LOGD_ANR("(0x0004) ynr_rnr_max_r:0x%x  ynr_local_gainscale:0x%x\n",
             pFix->rnr_max_r, pFix->local_gainscale);

    // YNR_2700_RNR_MAX_R  (0x0008)
    LOGD_ANR("(0x0008) ynr_rnr_center_coorv:0x%x  ynr_rnr_center_coorh:0x%x\n",
             pFix->rnr_center_coorv, pFix->rnr_center_coorh);

    // YNR_2700_RNR_MAX_R  (0x000c)
    LOGD_ANR("(0x000c) ynr_localgain_adj:0x%x  ynr_localgain_adj_thresh:0x%x\n",
             pFix->localgain_adj, pFix->localgain_adj_thresh);

    // YNR_2700_LOWNR_CTRL0 (0x0010)
    LOGD_ANR("(0x0010) ynr_low_bf_inv_0:0x%x ynr_low_bf_inv_1:0x%x\n",
             pFix->low_bf_inv0, pFix->low_bf_inv1);


    // YNR_2700_LOWNR_CTRL1  (0x0014)
    LOGD_ANR("(0x0014) ynr_low_peak_supress:0x%x ynr_low_thred_adj:0x%x \n",
             pFix->low_peak_supress,
             pFix->low_thred_adj);

    // YNR_2700_LOWNR_CTRL2 (0x0018)
    LOGD_ANR("(0x0018) ynr_low_dist_adj:0x%x ynr_low_edge_adj_thresh:0x%x \n",
             pFix->low_dist_adj,
             pFix->low_edge_adj_thresh);

    // YNR_2700_LOWNR_CTRL3 (0x001c)
    LOGD_ANR("(0x001c) ynr_low_bi_weight:0x%x ynr_low_weight:0x%x  ynr_low_center_weight:0x%x \n",
             pFix->low_bi_weight,
             pFix->low_weight,
             pFix->low_center_weight);

    // YNR_2700_HIGHNR_CTRL1  (0x002c)
    LOGD_ANR("(0x002c) ynr_frame_full_size:0x%x ynr_lbf_weight_thres:0x%x \n",
             pFix->frame_full_size,
             pFix->lbf_weight_thres);

    // YNR_2700_GAUSS1_COEFF  (0x0030)
    LOGD_ANR("(0x0030) ynr_low_gauss1_coeff:0x%x 0x%x 0x%x\n",
             pFix->low_gauss1_coeff2,
             pFix->low_gauss1_coeff1,
             pFix->low_gauss1_coeff0);


    // YNR_2700_GAUSS2_COEFF  (0x0034)
    LOGD_ANR("(0x0034) ynr_low_gauss2_coeff:0x%x 0x%x 0x%x\n",
             pFix->low_gauss2_coeff2,
             pFix->low_gauss2_coeff1,
             pFix->low_gauss2_coeff0);

    // YNR_2700_SGM_DX_0_1 (0x0040 - 0x0060)
    for(int i = 0; i < 17; i++) {
        LOGD_ANR("(0x0040- 0x0060) ynr_luma_points_x[%d]:0x%x \n",
                 i, pFix->luma_points_x[i]);
    }

    // YNR_2700_LSGM_Y_0_1 (0x0070- 0x0090)
    for(int i = 0; i < 17; i++) {
        LOGD_ANR("(0x0070- 0x0090) ynr_lsgm_y[%d]:0x%x \n",
                 i, pFix->lsgm_y[i]);
    }

#if 0
    // YNR_2700_HSGM_Y_0_1 (0x00a0- 0x00c0)
    for(int i = 0; i < 17; i++) {
        LOGD_ANR("(0x00a0- 0x00c0) ynr_hsgm_y[%d]:0x%x \n",
                 i, pFix->hsgm_y[i]);
    }
#endif

    // YNR_2700_RNR_STRENGTH03 (0x00d0- 0x00e0)
    for(int i = 0; i < 17; i++) {
        LOGD_ANR("(0x00d0- 0x00e0) ynr_rnr_strength[%d]:0x%x \n",
                 i, pFix->rnr_strength[i]);
    }

    // YNR_NLM_SIGMA_GAIN (0x00f0)
    LOGD_ANR("(0x00f0) ynr_nlm_min_sigma:0x%x ynr_nlm_hi_gain_alpha:0x%x  ynr_nlm_hi_bf_scale:0x%x \n",
             pFix->nlm_min_sigma,
             pFix->nlm_hi_gain_alpha,
             pFix->nlm_hi_bf_scale);

    // YNR_NLM_COE  (0x00f4)
    LOGD_ANR("(0x00f4) ynr_nlm_coe:0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",
             pFix->nlm_coe_0,
             pFix->nlm_coe_1,
             pFix->nlm_coe_2,
             pFix->nlm_coe_3,
             pFix->nlm_coe_4,
             pFix->nlm_coe_5);

    // YNR_NLM_WEIGHT  (0x00f8)
    LOGD_ANR("(0x00f8) ynr_nlm_center_weight:0x%x ynr_nlm_weight_offset:0x%x \n",
             pFix->nlm_center_weight,
             pFix->nlm_weight_offset);

    // YNR_NLM_NR_WEIGHT  (0x00fc)
    LOGD_ANR("(0x00fc) ynr_nlm_nr_weight:0x%x \n",
             pFix->nlm_nr_weight);


    LOGD_ANR("%s:(%d) exit \n", __FUNCTION__, __LINE__);

    return res;
}



Aynr_result_V22_t ynr_get_setting_by_name_json_V22(CalibDbV2_YnrV22_t *pCalibdbV2, char *name, int* calib_idx, int* tuning_idx)
{
    int i = 0;
    Aynr_result_V22_t res = AYNRV22_RET_SUCCESS;

    if(pCalibdbV2 == NULL || name == NULL
            || calib_idx == NULL || tuning_idx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNRV22_RET_NULL_POINTER;
    }

    for(i = 0; i < pCalibdbV2->TuningPara.Setting_len; i++) {
        if(strncmp(name, pCalibdbV2->TuningPara.Setting[i].SNR_Mode, strlen(name)*sizeof(char)) == 0) {
            break;
        }
    }

    if(i < pCalibdbV2->TuningPara.Setting_len) {
        *tuning_idx = i;
    } else {
        *tuning_idx = 0;
    }

    for(i = 0; i < pCalibdbV2->CalibPara.Setting_len; i++) {
        if(strncmp(name, pCalibdbV2->CalibPara.Setting[i].SNR_Mode, strlen(name)*sizeof(char)) == 0) {
            break;
        }
    }

    if(i < pCalibdbV2->CalibPara.Setting_len) {
        *calib_idx = i;
    } else {
        *calib_idx = 0;
    }

    LOGD_ANR("%s:%d snr_name:%s  snr_idx:%d i:%d \n",
             __FUNCTION__, __LINE__,
             name, *calib_idx, i);

    return res;
}

Aynr_result_V22_t ynr_init_params_json_V22(RK_YNR_Params_V22_t *pYnrParams, CalibDbV2_YnrV22_t *pCalibdbV2, int calib_idx, int tuning_idx)
{
    Aynr_result_V22_t res = AYNRV22_RET_SUCCESS;
    int i = 0;
    int j = 0;
    short isoCurveSectValue;
    short isoCurveSectValue1;
    float ave1, ave2, ave3, ave4;
    int bit_calib = 12;
    int bit_proc;
    int bit_shift;

    CalibDbV2_YnrV22_T_ISO_t *pISO = NULL;
    CalibDbV2_YnrV22_C_ISO_t *pCalibISO = NULL;

    LOGD_ANR("%s(%d): enter\n", __FUNCTION__, __LINE__);

    if(pYnrParams == NULL || pCalibdbV2 == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNRV22_RET_NULL_POINTER;
    }

    bit_proc = YNR_V22_SIGMA_BITS; // for V22, YNR_SIGMA_BITS = 10
    bit_shift = bit_calib - bit_proc;

    isoCurveSectValue = (1 << (bit_calib - YNR_V22_ISO_CURVE_POINT_BIT));
    isoCurveSectValue1 = (1 << bit_calib);


    for(j = 0; j < pCalibdbV2->CalibPara.Setting[tuning_idx].Calib_ISO_len && j < RK_YNR_V22_MAX_ISO_NUM ; j++) {
        pCalibISO = &pCalibdbV2->CalibPara.Setting[tuning_idx].Calib_ISO[j];
        pYnrParams->iso[j] = pCalibISO->iso;

        // get noise sigma sample data at [0, 64, 128, ... , 1024]
        for (i = 0; i < YNR_V22_ISO_CURVE_POINT_NUM; i++) {
            if (i == (YNR_V22_ISO_CURVE_POINT_NUM - 1)) {
                ave1 = (float)isoCurveSectValue1;
            } else {
                ave1 = (float)(i * isoCurveSectValue);
            }
            pYnrParams->arYnrParamsISO[j].lumaPoint[i] = (short)ave1;
            ave2 = ave1 * ave1;
            ave3 = ave2 * ave1;
            ave4 = ave3 * ave1;
            pYnrParams->arYnrParamsISO[j].sigma[i] = pCalibISO->sigma_curve[0] * ave4
                    + pCalibISO->sigma_curve[1] * ave3
                    + pCalibISO->sigma_curve[2] * ave2
                    + pCalibISO->sigma_curve[3] * ave1
                    + pCalibISO->sigma_curve[4];

            if (pYnrParams->arYnrParamsISO[j].sigma[i] < 0) {
                pYnrParams->arYnrParamsISO[j].sigma[i] = 0;
            }

            if (bit_shift > 0) {
                pYnrParams->arYnrParamsISO[j].lumaPoint[i] >>= bit_shift;
            } else {
                pYnrParams->arYnrParamsISO[j].lumaPoint[i] <<= ABS(bit_shift);
            }
        }

        pYnrParams->arYnrParamsISO[j].lci = pCalibISO->ynr_lci;
        pYnrParams->arYnrParamsISO[j].hci = pCalibISO->ynr_hci;
    }

    LOGD_ANR(" iso len:%d calib_max:%d\n", pCalibdbV2->TuningPara.Setting[tuning_idx].Tuning_ISO_len, RK_YNR_V22_MAX_ISO_NUM);

    for(j = 0; j < pCalibdbV2->TuningPara.Setting[tuning_idx].Tuning_ISO_len && j < RK_YNR_V22_MAX_ISO_NUM; j++) {
        pISO = &pCalibdbV2->TuningPara.Setting[tuning_idx].Tuning_ISO[j];

        for(i = 0; i < 17; i++) {
            pYnrParams->arYnrParamsISO[j].rnr_strength[i] = pISO->rnr_strength[i];
        }

        //luma param
        for(int k = 0; k < 6; k++) {
            pYnrParams->arYnrParamsISO[j].lo_lumaPoint[k] = pISO->lumaPara.lo_lumaPoint[k];
            pYnrParams->arYnrParamsISO[j].lo_ratio[k] = pISO->lumaPara.lo_ratio[k];
            pYnrParams->arYnrParamsISO[j].hi_lumaPoint[k] = pISO->lumaPara.hi_lumaPoint[k];
            pYnrParams->arYnrParamsISO[j].hi_ratio[k] = pISO->lumaPara.hi_ratio[k];
        }

        pYnrParams->arYnrParamsISO[j].ynr_bft3x3_bypass = pISO->ynr_bft3x3_bypass;
        pYnrParams->arYnrParamsISO[j].ynr_lbft5x5_bypass = pISO->ynr_lbft5x5_bypass;
        pYnrParams->arYnrParamsISO[j].ynr_lgft3x3_bypass = pISO->ynr_lgft3x3_bypass;
        pYnrParams->arYnrParamsISO[j].ynr_flt1x1_bypass = pISO->ynr_flt1x1_bypass;
        pYnrParams->arYnrParamsISO[j].ynr_nlm11x11_bypass = pISO->ynr_nlm11x11_bypass;
        pYnrParams->arYnrParamsISO[j].ynr_thumb_mix_cur_en = pISO->ynr_thumb_mix_cur_en;

        //low freq
        pYnrParams->arYnrParamsISO[j].low_bf1 = pISO->low_bf1;
        pYnrParams->arYnrParamsISO[j].low_bf2 = pISO->low_bf2;
        pYnrParams->arYnrParamsISO[j].low_thred_adj = pISO->low_thred_adj;
        pYnrParams->arYnrParamsISO[j].low_peak_supress = pISO->low_peak_supress;
        pYnrParams->arYnrParamsISO[j].low_edge_adj_thresh = pISO->low_edge_adj_thresh;
        pYnrParams->arYnrParamsISO[j].low_lbf_weight_thresh = pISO->low_lbf_weight_thresh;
        pYnrParams->arYnrParamsISO[j].low_center_weight = pISO->low_center_weight;
        pYnrParams->arYnrParamsISO[j].low_dist_adj = pISO->low_dist_adj;
        pYnrParams->arYnrParamsISO[j].low_weight = pISO->low_weight;
        pYnrParams->arYnrParamsISO[j].low_filt1_strength = pISO->low_filt1_strength;
        pYnrParams->arYnrParamsISO[j].low_filt2_strength = pISO->low_filt2_strength;
        pYnrParams->arYnrParamsISO[j].low_bi_weight = pISO->low_bi_weight;

        //hi freq
        pYnrParams->arYnrParamsISO[j].hi_weight_offset = pISO->hi_weight_offset;
        pYnrParams->arYnrParamsISO[j].hi_center_weight = pISO->hi_center_weight;
        pYnrParams->arYnrParamsISO[j].hi_bf_scale = pISO->hi_bf_scale;
        pYnrParams->arYnrParamsISO[j].hi_min_sigma = pISO->hi_min_sigma;
        pYnrParams->arYnrParamsISO[j].hi_nr_weight = pISO->hi_nr_weight;
        pYnrParams->arYnrParamsISO[j].hi_gain_alpha = pISO->hi_gain_alpha;
        pYnrParams->arYnrParamsISO[j].hi_filter_coeff1_1 = pISO->hi_filter_coeff1_1;
        pYnrParams->arYnrParamsISO[j].hi_filter_coeff1_2 = pISO->hi_filter_coeff1_2;
        pYnrParams->arYnrParamsISO[j].hi_filter_coeff1_3 = pISO->hi_filter_coeff1_3;
        pYnrParams->arYnrParamsISO[j].hi_filter_coeff2_1 = pISO->hi_filter_coeff2_1;
        pYnrParams->arYnrParamsISO[j].hi_filter_coeff2_2 = pISO->hi_filter_coeff2_2;
        pYnrParams->arYnrParamsISO[j].hi_filter_coeff2_3 = pISO->hi_filter_coeff2_3;

        //global gain
        pYnrParams->arYnrParamsISO[j].ynr_global_gain_alpha = pISO->ynr_global_gain_alpha;
        pYnrParams->arYnrParamsISO[j].ynr_global_gain = pISO->ynr_global_gain;
        pYnrParams->arYnrParamsISO[j].ynr_adjust_thresh = pISO->ynr_adjust_thresh;
        pYnrParams->arYnrParamsISO[j].ynr_adjust_scale = pISO->ynr_adjust_scale;
    }

    LOGD_ANR("%s(%d): exit\n", __FUNCTION__, __LINE__);

    return res;
}

Aynr_result_V22_t ynr_config_setting_param_json_V22(RK_YNR_Params_V22_t *pParams, CalibDbV2_YnrV22_t *pCalibdbV2, char* param_mode, char * snr_name)
{
    Aynr_result_V22_t res = AYNRV22_RET_SUCCESS;
    int calib_idx = 0;
    int tuning_idx = 0;

    if(pParams == NULL || pCalibdbV2 == NULL
            || param_mode == NULL || snr_name == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNRV22_RET_NULL_POINTER;
    }

    res = ynr_get_setting_by_name_json_V22(pCalibdbV2, snr_name, &calib_idx, &tuning_idx);
    if(res != AYNRV22_RET_SUCCESS) {
        LOGW_ANR("%s(%d): error!!!  can't find setting in iq files, use 0 instead\n", __FUNCTION__, __LINE__);
    }

    res = ynr_init_params_json_V22(pParams, pCalibdbV2, calib_idx, tuning_idx);
    pParams->enable = pCalibdbV2->TuningPara.enable;
    return res;

}

RKAIQ_END_DECLARE

