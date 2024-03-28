
#include "rk_aiq_aynr_algo_ynr_v3.h"

RKAIQ_BEGIN_DECLARE

Aynr_result_V3_t ynr_select_params_by_ISO_V3(RK_YNR_Params_V3_t *pParams, RK_YNR_Params_V3_Select_t *pSelect, Aynr_ExpInfo_V3_t *pExpInfo)
{
    float ratio = 0.0f;
    int iso = 50;
    RK_YNR_Params_V3_Select_t *pParamHi = NULL;
    RK_YNR_Params_V3_Select_t *pParamLo = NULL;
    RK_YNR_Params_V3_Select_t* pParamTmp = NULL;


    Aynr_result_V3_t res = AYNRV3_RET_SUCCESS;

    if(pParams == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNRV3_RET_NULL_POINTER;
    }

    if(pSelect == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNRV3_RET_NULL_POINTER;
    }

    if(pExpInfo == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNRV3_RET_NULL_POINTER;
    }

    iso = pExpInfo->arIso[pExpInfo->hdr_mode];

    // choose integer type data
    int cur_iso_idx = 0;
    int idx = 0;
    for (idx = 0; idx < RK_YNR_V3_MAX_ISO_NUM; idx++) {
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
    if (idx == RK_YNR_V3_MAX_ISO_NUM)
        cur_iso_idx = RK_YNR_V3_MAX_ISO_NUM - 1;


    pParamTmp = &pParams->arYnrParamsISO[cur_iso_idx];

    pSelect->enable = pParams->enable;
    pSelect->ynr_bft3x3_bypass = pParamTmp->ynr_bft3x3_bypass;
    pSelect->ynr_lbft5x5_bypass = pParamTmp->ynr_lbft5x5_bypass;
    pSelect->ynr_lgft3x3_bypass = pParamTmp->ynr_lgft3x3_bypass;
    pSelect->ynr_flt1x1_bypass = pParamTmp->ynr_flt1x1_bypass;
    pSelect->ynr_sft5x5_bypass = pParamTmp->ynr_sft5x5_bypass;


    int iso_div = 50;
    int lowIso = 50;
    int highIso = 50;
    int minIso = 50;
    int maxIso = 50;

    for(int i = 0; i < RK_YNR_V3_MAX_ISO_NUM - 1; i++) {
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

            break;
        }
    }

#ifndef RK_SIMULATOR_HW
    minIso = pParams->iso[0];
    maxIso = pParams->iso[RK_YNR_V3_MAX_ISO_NUM - 1];
#else
    minIso = iso_div * (1 << 0);
    maxIso = iso_div * (1 << (RK_YNR_V3_MAX_ISO_NUM - 1));
#endif

    if(iso < minIso) {
        ratio = 0;
        pParamLo = &pParams->arYnrParamsISO[0];
        pParamHi = &pParams->arYnrParamsISO[1];
#ifndef RK_SIMULATOR_HW
        lowIso = pParams->iso[0];
        highIso = pParams->iso[1];
#else
        lowIso = iso_div * (1 << 0);
        highIso = iso_div * (1 << 1);
#endif
    }

    if(iso > maxIso) {
        ratio = 1;
        pParamLo = &pParams->arYnrParamsISO[RK_YNR_V3_MAX_ISO_NUM - 2];
        pParamHi = &pParams->arYnrParamsISO[RK_YNR_V3_MAX_ISO_NUM - 1];
#ifndef RK_SIMULATOR_HW
        lowIso = pParams->iso[RK_YNR_V3_MAX_ISO_NUM - 2];
        highIso = pParams->iso[RK_YNR_V3_MAX_ISO_NUM - 1];
#else
        lowIso = iso_div * (1 << (RK_YNR_V3_MAX_ISO_NUM - 2));
        highIso = iso_div * (1 << (RK_YNR_V3_MAX_ISO_NUM - 1));
#endif
    }

    pExpInfo->isoHigh = highIso;
    pExpInfo->isoLow = lowIso;

    LOGD_ANR("oyyf %s:%d  iso:%d low:%d hight:%d ratio:%f iso_index:%d \n", __FUNCTION__, __LINE__,
             iso, lowIso, highIso, ratio, cur_iso_idx);
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
    for (int i = 0; i < YNR_V3_ISO_CURVE_POINT_NUM; i++)
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
    pSelect->base_filter_weight1 = ratio * (pParamHi->base_filter_weight1 - pParamLo->base_filter_weight1) + pParamLo->base_filter_weight1;
    pSelect->base_filter_weight2 = ratio * (pParamHi->base_filter_weight2 - pParamLo->base_filter_weight2) + pParamLo->base_filter_weight2;
    pSelect->base_filter_weight3 = ratio * (pParamHi->base_filter_weight3 - pParamLo->base_filter_weight3) + pParamLo->base_filter_weight3;


    pSelect->high_thred_adj = ratio * (pParamHi->high_thred_adj - pParamLo->high_thred_adj) + pParamLo->high_thred_adj;
    pSelect->high_weight = ratio * (pParamHi->high_weight - pParamLo->high_weight) + pParamLo->high_weight;

    for (int i = 0; i < 8; i++)
    {
        pSelect->high_direction_weight[i] = ratio * (pParamHi->high_direction_weight[i] - pParamLo->high_direction_weight[i]) + pParamLo->high_direction_weight[i];
    }
    pSelect->hi_min_adj = ratio * (pParamHi->hi_min_adj - pParamLo->hi_min_adj) + pParamLo->hi_min_adj;
    pSelect->hi_edge_thed = ratio * (pParamHi->hi_edge_thed - pParamLo->hi_edge_thed) + pParamLo->hi_edge_thed;

    for (int i = 0; i < 6; i++)
    {
        pSelect->hi_lumaPoint[i] = ratio * (pParamHi->hi_lumaPoint[i] - pParamLo->hi_lumaPoint[i]) + pParamLo->hi_lumaPoint[i];
        pSelect->hi_ratio[i] = ratio * (pParamHi->hi_ratio[i] - pParamLo->hi_ratio[i]) + pParamLo->hi_ratio[i];

        pSelect->hi_lumaPoint[i] *= 4; // curve point 8 bits -> 10 bits
    }

    return res;
}

Aynr_result_V3_t ynr_fix_transfer_V3(RK_YNR_Params_V3_Select_t* pSelect, RK_YNR_Fix_V3_t *pFix, rk_aiq_ynr_strength_v3_t *pStrength, Aynr_ExpInfo_V3_t *pExpInfo)
{
    LOGI_ANR("%s:(%d) enter \n", __FUNCTION__, __LINE__);

    Aynr_result_V3_t res = AYNRV3_RET_SUCCESS;
    int w0, w1, w2;
    int tmp;

    if(pSelect == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNRV3_RET_NULL_POINTER;
    }

    if(pFix == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNRV3_RET_NULL_POINTER;
    }

    if(pStrength == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNRV3_RET_NULL_POINTER;
    }

    if(pExpInfo == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNRV3_RET_NULL_POINTER;
    }

    float fStrength = 1.0;

    if(pStrength->strength_enable) {
        fStrength = pStrength->percent;
    }

    if(fStrength <= 0.0) {
        fStrength = 0.000001;
    }

    LOGD_ANR("%s:%d strength_enable:%d fStrength:%f raw:width:%d height:%d\n",
             __FUNCTION__, __LINE__,
             pStrength->strength_enable,
             fStrength,
             pExpInfo->rawHeight,
             pExpInfo->rawWidth);

    // YNR_2700_GLOBAL_CTRL (0x0000)
    pFix->ynr_rnr_en = 1;
    pFix->ynr_gate_dis = 0;
    pFix->ynr_thumb_mix_cur_en = 0;
    tmp = (int)(pSelect->ynr_global_gain_alpha * (1 << 3));
    pFix->ynr_global_gain_alpha = CLIP(tmp, 0, 8);
    tmp = (int)(pSelect->ynr_global_gain * (1 << 4));
    pFix->ynr_global_gain  = CLIP(tmp, 0, 1023);
    pFix->ynr_flt1x1_bypass_sel = 0;
    pFix->ynr_sft5x5_bypass = pSelect->ynr_sft5x5_bypass;
    pFix->ynr_flt1x1_bypass = pSelect->ynr_flt1x1_bypass;
    pFix->ynr_lgft3x3_bypass = pSelect->ynr_lgft3x3_bypass;
    pFix->ynr_lbft5x5_bypass = pSelect->ynr_lbft5x5_bypass;
    pFix->ynr_bft3x3_bypass = pSelect->ynr_bft3x3_bypass;
    pFix->ynr_en = pSelect->enable;

    // YNR_2700_RNR_MAX_R  (0x0004)
    // pFix->ynr_local_gainscale =
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
    pFix->ynr_rnr_max_r = CLIP(tmp, 0, 0x3fff);
    //local gain scale
    //tmp = ( sqrt(double(50) / pExpInfo->arIso[pExpInfo->hdr_mode])) * (1 << 7);  //old
    tmp = (1.0) * (1 << 7);
    pFix->ynr_local_gainscale = CLIP(tmp, 0, 0x80);

    //// YNR_2700_CENTRE_COOR (0x0008)
    pFix->ynr_rnr_center_coorv = rows / 2;
    pFix->ynr_rnr_center_coorh = cols / 2;

    //// YNR_2700_CENTRE_COOR (0x000c)
    tmp = pSelect->ynr_adjust_scale * (1 << 4);
    pFix->ynr_localgain_adj = CLIP(tmp, 0, 0xff);
    //tmp = pSelect->ynr_adjust_thresh * 1023;  //old
    tmp = pSelect->ynr_adjust_thresh * 16;
    pFix->ynr_localgain_adj_thresh = CLIP(tmp, 0, 0x3ff);;

    // YNR_2700_LOWNR_CTRL0 (0x0010)
    tmp = (int)(1.0f / pSelect->low_bf2 / fStrength * (1 << 9));
    pFix->ynr_low_bf_inv[1] = CLIP(tmp, 0, 0x3fff);
    tmp = (int)(1.0f / pSelect->low_bf1 / fStrength * (1 << 9));
    pFix->ynr_low_bf_inv[0] = CLIP(tmp, 0, 0x3fff);


    // YNR_2700_LOWNR_CTRL1  (0x0014)
    tmp = (int)(pSelect->low_peak_supress * (1 << 7));
    pFix->ynr_low_peak_supress = CLIP(tmp, 0, 0x80);
    tmp = (int)(pSelect->low_thred_adj * fStrength * (1 << 6));
    pFix->ynr_low_thred_adj = CLIP(tmp, 0, 0x7ff);

    // YNR_2700_LOWNR_CTRL2 (0x0018)
    tmp = (int)(pSelect->low_dist_adj * (1 << 2));
    pFix->ynr_low_dist_adj = CLIP(tmp, 0, 0x1ff);
    tmp = (int)(pSelect->low_edge_adj_thresh);
    pFix->ynr_low_edge_adj_thresh = CLIP(tmp, 0, 0x3ff);


    // YNR_2700_LOWNR_CTRL3 (0x001c)
    tmp = (int)(pSelect->low_bi_weight * (1 << 7));
    pFix->ynr_low_bi_weight = CLIP(tmp, 0, 0x80);
    tmp = (int)(pSelect->low_weight * (1 << 7));
    pFix->ynr_low_weight = CLIP(tmp, 0, 0x80);
    tmp = (int)(pSelect->low_center_weight * (1 << 10));
    pFix->ynr_low_center_weight = CLIP(tmp, 0, 0x400);

    // YNR_2700_HIGHNR_CTRL0 (0x0020)
    tmp = (int)(pSelect->hi_min_adj * (1 << 6));
    pFix->ynr_hi_min_adj = CLIP(tmp, 0, 0x3f);
    tmp = (int)(pSelect->high_thred_adj * fStrength * (1 << 6));
    pFix->ynr_high_thred_adj = CLIP(tmp, 0, 0x7ff);

    // YNR_2700_HIGHNR_CTRL1  (0x0024)
    tmp = (1 << 7) - (int)(pSelect->high_weight * (1 << 7));
    pFix->ynr_high_retain_weight = CLIP(tmp, 0, 0x80);
    tmp = (int)(pSelect->hi_edge_thed);
    pFix->ynr_hi_edge_thed = CLIP(tmp, 0, 0xff);

    // YNR_2700_HIGHNR_BASE_FILTER_WEIGHT  (0x0028)
    w2 = int(pSelect->base_filter_weight3 * 64 / 2 + 0.5);
    w1 = int(pSelect->base_filter_weight2 * 64 / 2 + 0.5);
    w0 = 64 - w1 * 2 - w2 * 2;
    pFix->ynr_base_filter_weight[0] = CLIP(w0, 0, 0x40);
    pFix->ynr_base_filter_weight[1] = CLIP(w1, 0, 0x1f);
    pFix->ynr_base_filter_weight[2] = CLIP(w2, 0, 0xf);

    // YNR_2700_HIGHNR_BASE_FILTER_WEIGHT  (0x002c)
    pFix->ynr_frame_full_size = 0x0000;
    tmp = pSelect->low_lbf_weight_thresh * 1023;
    pFix->ynr_lbf_weight_thres = CLIP(tmp, 0, 0x3ff);

    // YNR_2700_GAUSS1_COEFF  (0x0030)
    float filter1_sigma = pSelect->low_filt1_strength;
    float filt1_coeff1 = exp(-1 / (2 * filter1_sigma * filter1_sigma));
    float filt1_coeff0 = filt1_coeff1 * filt1_coeff1;
    float coeff1_sum = 1 + 4 * filt1_coeff1 + 4 * filt1_coeff0;
    w0 = int(filt1_coeff0 / coeff1_sum * 256 + 0.5);
    w1 = int(filt1_coeff1 / coeff1_sum * 256 + 0.5);
    w2 = 256 - w0 * 4 - w1 * 4;
    pFix->ynr_low_gauss1_coeff[0] = CLIP(w0, 0, 0x3f);
    pFix->ynr_low_gauss1_coeff[1] = CLIP(w1, 0, 0x3f);
    pFix->ynr_low_gauss1_coeff[2] = CLIP(w2, 0, 0x100);

    // YNR_2700_GAUSS2_COEFF  (0x0034)
    float filter2_sigma = pSelect->low_filt2_strength;
    float filt2_coeff1 = exp(-1 / (2 * filter2_sigma * filter2_sigma));
    float filt2_coeff0 = filt2_coeff1 * filt2_coeff1;
    float coeff2_sum = 1 + 4 * filt2_coeff1 + 4 * filt2_coeff0;
    w0 = int(filt2_coeff0 / coeff2_sum * 256 + 0.5);
    w1 = int(filt2_coeff1 / coeff2_sum * 256 + 0.5);
    w2 = 256 - w0 * 4 - w1 * 4;
    pFix->ynr_low_gauss2_coeff[0] = CLIP(w0, 0, 0x3f);
    pFix->ynr_low_gauss2_coeff[1] = CLIP(w1, 0, 0x3f);
    pFix->ynr_low_gauss2_coeff[2] = CLIP(w2, 0, 0x100);


    // YNR_2700_DIRECTION_W_0_3  (0x0038 - 0x003c)
    for (int i = 0; i < 8; i++) {
        tmp = (int)(pSelect->high_direction_weight[i] * (1 << 4));
        pFix->ynr_direction_weight[i] = CLIP(tmp, 0, 0x10);
    }

    // YNR_2700_SGM_DX_0_1 (0x0040 - 0x0060)
    // YNR_2700_LSGM_Y_0_1 (0x0070- 0x0090)
    // YNR_2700_HSGM_Y_0_1 (0x00a0- 0x00c0)
    for (int i = 0; i < YNR_V3_ISO_CURVE_POINT_NUM; i++) {
        tmp = pSelect->lumaPoint[i];
        pFix->ynr_luma_points_x[i] = CLIP(tmp, 0, 0x400);
        tmp = (int)(pSelect->sigma[i] * pSelect->lci * (1 << YNR_V3_NOISE_SIGMA_FIX_BIT));
        pFix->ynr_lsgm_y[i] = CLIP(tmp, 0, 0xfff);
        tmp = (int)(pSelect->sigma[i] * pSelect->hci * (1 << YNR_V3_NOISE_SIGMA_FIX_BIT));
        pFix->ynr_hsgm_y[i] = CLIP(tmp, 0, 0xfff);
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
    for (int i = 0; i < YNR_V3_ISO_CURVE_POINT_NUM; i++) {
        float rate;
        int j = 0;
        for (j = 0; j < 6; j++) {
            if (pFix->ynr_luma_points_x[i] <= lo_lumaPoint[j])
                break;
        }

        if (j <= 0)
            rate = lo_ratio[0];
        else if (j >= 6)
            rate = lo_ratio[5];
        else {
            rate = ((float)pFix->ynr_luma_points_x[i] - lo_lumaPoint[j - 1]) / (lo_lumaPoint[j] - lo_lumaPoint[j - 1]);
            rate = lo_ratio[j - 1] + rate * (lo_ratio[j] - lo_ratio[j - 1]);
        }
        tmp = (int)(rate * pFix->ynr_lsgm_y[i]);
        pFix->ynr_lsgm_y[i] = CLIP(tmp, 0, 0xfff);
    }

    //update hi noise curve;
    for (int i = 0; i < YNR_V3_ISO_CURVE_POINT_NUM; i++) {
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

    // YNR_2700_RNR_STRENGTH03 (0x00d0- 0x00e0)
    for (int i = 0; i < 17; i++) {
        tmp = int(pSelect->rnr_strength[i] * 16);
        pFix->ynr_rnr_strength[i] = CLIP(tmp, 0, 0xff);
    }

    ynr_fix_printf_V3(pFix);
    return res;
}

Aynr_result_V3_t ynr_fix_printf_V3(RK_YNR_Fix_V3_t * pFix)
{
    LOGD_ANR("%s:(%d) enter \n", __FUNCTION__, __LINE__);

    Aynr_result_V3_t res = AYNRV3_RET_SUCCESS;

    if(pFix == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNRV3_RET_NULL_POINTER;
    }

    // YNR_2700_GLOBAL_CTRL (0x0000)
    LOGD_ANR("(0x0000) sw_ynr_thumb_mix_cur_en:0x%x sw_ynr_gate_dis:0x%x sw_ynr_rnr_en:0x%x\n",
             pFix->ynr_thumb_mix_cur_en,
             pFix->ynr_gate_dis,
             pFix->ynr_rnr_en);

    // YNR_2700_GLOBAL_CTRL (0x0000)
    LOGD_ANR("(0x0000) ynr_global_gain_alpha:0x%x ynr_global_gain:0x%x \n ynr_flt1x1_bypass_sel:0x%x  ynr_sft5x5_bypass:0x%x \n ynr_flt1x1_bypass:0x%x  ynr_lgft3x3_bypass:0x%x \n ynr_lbft5x5_bypass:0x%x  ynr_bft3x3_bypass:0x%x \n ynr_en:0x%x\n",
             pFix->ynr_global_gain_alpha,
             pFix->ynr_global_gain,
             pFix->ynr_flt1x1_bypass_sel,
             pFix->ynr_sft5x5_bypass,
             pFix->ynr_flt1x1_bypass,
             pFix->ynr_lgft3x3_bypass,
             pFix->ynr_lbft5x5_bypass,
             pFix->ynr_bft3x3_bypass,
             pFix->ynr_en);


    // YNR_2700_RNR_MAX_R  (0x0004)
    LOGD_ANR("(0x0004) ynr_rnr_max_r:0x%x  ynr_local_gainscale:0x%x\n",
             pFix->ynr_rnr_max_r, pFix->ynr_local_gainscale);

    // YNR_2700_RNR_MAX_R  (0x0008)
    LOGD_ANR("(0x0008) ynr_rnr_center_coorv:0x%x  ynr_rnr_center_coorh:0x%x\n",
             pFix->ynr_rnr_center_coorv, pFix->ynr_rnr_center_coorh);

    // YNR_2700_RNR_MAX_R  (0x00010)
    LOGD_ANR("(0x0010) ynr_localgain_adj:0x%x  ynr_localgain_adj_thresh:0x%x\n",
             pFix->ynr_localgain_adj, pFix->ynr_localgain_adj_thresh);

    // YNR_2700_LOWNR_CTRL0 (0x0010)
    for(int i = 0; i < 2; i++) {
        LOGD_ANR("(0x0010) ynr_low_bf_inv[%d]:0x%x \n",
                 i, pFix->ynr_low_bf_inv[i]);
    }

    // YNR_2700_LOWNR_CTRL1  (0x0014)
    LOGD_ANR("(0x0014) ynr_low_peak_supress:0x%x ynr_low_thred_adj:0x%x \n",
             pFix->ynr_low_peak_supress,
             pFix->ynr_low_thred_adj);

    // YNR_2700_LOWNR_CTRL2 (0x0018)
    LOGD_ANR("(0x0018) ynr_low_dist_adj:0x%x ynr_low_edge_adj_thresh:0x%x \n",
             pFix->ynr_low_dist_adj,
             pFix->ynr_low_edge_adj_thresh);

    // YNR_2700_LOWNR_CTRL3 (0x001c)
    LOGD_ANR("(0x001c) ynr_low_bi_weight:0x%x ynr_low_weight:0x%x  ynr_low_center_weight:0x%x \n",
             pFix->ynr_low_bi_weight,
             pFix->ynr_low_weight,
             pFix->ynr_low_center_weight);

    // YNR_2700_HIGHNR_CTRL0 (0x0020)
    LOGD_ANR("(0x0020) ynr_hi_min_adj:0x%x ynr_high_thred_adj:0x%x \n",
             pFix->ynr_hi_min_adj,
             pFix->ynr_high_thred_adj);

    // YNR_2700_HIGHNR_CTRL1  (0x0024)
    LOGD_ANR("(0x0024) ynr_high_retain_weight:0x%x ynr_hi_edge_thed:0x%x \n",
             pFix->ynr_high_retain_weight,
             pFix->ynr_hi_edge_thed);

    // YNR_2700_HIGHNR_BASE_FILTER_WEIGHT  (0x0028)
    for(int i = 0; i < 3; i++) {
        LOGD_ANR("(0x0028) ynr_base_filter_weight[%d]:0x%x \n",
                 i, pFix->ynr_base_filter_weight[i]);
    }

    // YNR_2700_HIGHNR_CTRL1  (0x002c)
    LOGD_ANR("(0x002c) ynr_frame_full_size:0x%x ynr_lbf_weight_thres:0x%x \n",
             pFix->ynr_frame_full_size,
             pFix->ynr_lbf_weight_thres);

    // YNR_2700_GAUSS1_COEFF  (0x0030)
    for(int i = 0; i < 3; i++) {
        LOGD_ANR("(0x0030) ynr_low_gauss1_coeff[%d]:0x%x \n",
                 i, pFix->ynr_low_gauss1_coeff[i]);
    }

    // YNR_2700_GAUSS2_COEFF  (0x0034)
    for(int i = 0; i < 3; i++) {
        LOGD_ANR("(0x0034) ynr_low_gauss2_coeff[%d]:0x%x \n",
                 i, pFix->ynr_low_gauss2_coeff[i]);
    }

    // YNR_2700_DIRECTION_W_0_3  (0x0038 - 0x003c)
    for(int i = 0; i < 8; i++) {
        LOGD_ANR("(0x0038- 0x003c) ynr_direction_weight[%d]:0x%x \n",
                 i, pFix->ynr_direction_weight[i]);
    }

    // YNR_2700_SGM_DX_0_1 (0x0040 - 0x0060)
    for(int i = 0; i < 17; i++) {
        LOGD_ANR("(0x0040- 0x0060) ynr_luma_points_x[%d]:0x%x \n",
                 i, pFix->ynr_luma_points_x[i]);
    }

    // YNR_2700_LSGM_Y_0_1 (0x0070- 0x0090)
    for(int i = 0; i < 17; i++) {
        LOGD_ANR("(0x0070- 0x0090) ynr_lsgm_y[%d]:0x%x \n",
                 i, pFix->ynr_lsgm_y[i]);
    }

    // YNR_2700_HSGM_Y_0_1 (0x00a0- 0x00c0)
    for(int i = 0; i < 17; i++) {
        LOGD_ANR("(0x00a0- 0x00c0) ynr_hsgm_y[%d]:0x%x \n",
                 i, pFix->ynr_hsgm_y[i]);
    }

    // YNR_2700_RNR_STRENGTH03 (0x00d0- 0x00e0)
    for(int i = 0; i < 17; i++) {
        LOGD_ANR("(0x00d0- 0x00e0) ynr_rnr_strength[%d]:0x%x \n",
                 i, pFix->ynr_rnr_strength[i]);
    }


    LOGD_ANR("%s:(%d) exit \n", __FUNCTION__, __LINE__);

    return res;
}



Aynr_result_V3_t ynr_get_setting_by_name_json_V3(CalibDbV2_YnrV3_t *pCalibdbV2, char *name, int* calib_idx, int* tuning_idx)
{
    int i = 0;
    Aynr_result_V3_t res = AYNRV3_RET_SUCCESS;

    if(pCalibdbV2 == NULL || name == NULL
            || calib_idx == NULL || tuning_idx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNRV3_RET_NULL_POINTER;
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

Aynr_result_V3_t ynr_init_params_json_V3(RK_YNR_Params_V3_t *pYnrParams, CalibDbV2_YnrV3_t *pCalibdbV2, int calib_idx, int tuning_idx)
{
    Aynr_result_V3_t res = AYNRV3_RET_SUCCESS;
    int i = 0;
    int j = 0;
    short isoCurveSectValue;
    short isoCurveSectValue1;
    float ave1, ave2, ave3, ave4;
    int bit_calib = 12;
    int bit_proc;
    int bit_shift;

    CalibDbV2_YnrV3_T_Set_ISO_t *pISO = NULL;
    CalibDbV2_YnrV3_C_Set_ISO_t *pCalibISO = NULL;

    LOGD_ANR("%s(%d): enter\n", __FUNCTION__, __LINE__);

    if(pYnrParams == NULL || pCalibdbV2 == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNRV3_RET_NULL_POINTER;
    }

    bit_proc = YNR_V3_SIGMA_BITS; // for V3, YNR_SIGMA_BITS = 10
    bit_shift = bit_calib - bit_proc;

    isoCurveSectValue = (1 << (bit_calib - YNR_V3_ISO_CURVE_POINT_BIT));
    isoCurveSectValue1 = (1 << bit_calib);



    for(j = 0; j < pCalibdbV2->CalibPara.Setting[tuning_idx].Calib_ISO_len && j < RK_YNR_V3_MAX_ISO_NUM ; j++) {
        pCalibISO = &pCalibdbV2->CalibPara.Setting[tuning_idx].Calib_ISO[j];
        pYnrParams->iso[j] = pCalibISO->iso;
        pYnrParams->sigma_use_point = pCalibdbV2->CalibPara.sigma_use_point;
        if(pCalibdbV2->CalibPara.sigma_use_point) {
            LOGD_ANR("oyyf ynr use point\n");
            for (i = 0; i < YNR_V3_ISO_CURVE_POINT_NUM; i++) {
                pYnrParams->arYnrParamsISO[j].sigma[i] = pCalibISO->sigma[i];
                pYnrParams->arYnrParamsISO[j].lumaPoint[i] = pCalibISO->lumaPoint[i];
            }
        } else {
            LOGD_ANR("oyyf ynr use formula\n");
            // get noise sigma sample data at [0, 64, 128, ... , 1024]
            for (i = 0; i < YNR_V3_ISO_CURVE_POINT_NUM; i++) {
                if (i == (YNR_V3_ISO_CURVE_POINT_NUM - 1)) {
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
        }

        pYnrParams->arYnrParamsISO[j].lci = pCalibISO->ynr_lci;
        pYnrParams->arYnrParamsISO[j].hci = pCalibISO->ynr_hci;
    }

    LOGD_ANR(" iso len:%d calib_max:%d\n", pCalibdbV2->TuningPara.Setting[tuning_idx].Tuning_ISO_len, RK_YNR_V3_MAX_ISO_NUM);

    for(j = 0; j < pCalibdbV2->TuningPara.Setting[tuning_idx].Tuning_ISO_len && j < RK_YNR_V3_MAX_ISO_NUM; j++) {
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
        pYnrParams->arYnrParamsISO[j].ynr_sft5x5_bypass = pISO->ynr_sft5x5_bypass;

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

        pYnrParams->arYnrParamsISO[j].base_filter_weight1 = pISO->base_filter_weight1;
        pYnrParams->arYnrParamsISO[j].base_filter_weight2 = pISO->base_filter_weight2;
        pYnrParams->arYnrParamsISO[j].base_filter_weight3 = pISO->base_filter_weight3;
        pYnrParams->arYnrParamsISO[j].high_thred_adj = pISO->high_thred_adj;
        pYnrParams->arYnrParamsISO[j].high_weight = pISO->high_weight;
        for(i = 0; i < 8; i++) {
            pYnrParams->arYnrParamsISO[j].high_direction_weight[i] = pISO->high_direction_weight[i];
        }
        pYnrParams->arYnrParamsISO[j].hi_min_adj = pISO->hi_min_adj;
        pYnrParams->arYnrParamsISO[j].hi_edge_thed = pISO->hi_edge_thed;

        //global gain
        pYnrParams->arYnrParamsISO[j].ynr_global_gain_alpha = pISO->ynr_global_gain_alpha;
        pYnrParams->arYnrParamsISO[j].ynr_global_gain = pISO->ynr_global_gain;
        pYnrParams->arYnrParamsISO[j].ynr_adjust_thresh = pISO->ynr_adjust_thresh;
        pYnrParams->arYnrParamsISO[j].ynr_adjust_scale = pISO->ynr_adjust_scale;
    }

    LOGD_ANR("%s(%d): exit\n", __FUNCTION__, __LINE__);

    return res;
}

Aynr_result_V3_t ynr_config_setting_param_json_V3(RK_YNR_Params_V3_t *pParams, CalibDbV2_YnrV3_t *pCalibdbV2, char* param_mode, char * snr_name)
{
    Aynr_result_V3_t res = AYNRV3_RET_SUCCESS;
    int calib_idx = 0;
    int tuning_idx = 0;

    if(pParams == NULL || pCalibdbV2 == NULL
            || param_mode == NULL || snr_name == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNRV3_RET_NULL_POINTER;
    }

    res = ynr_get_setting_by_name_json_V3(pCalibdbV2, snr_name, &calib_idx, &tuning_idx);
    if(res != AYNRV3_RET_SUCCESS) {
        LOGW_ANR("%s(%d): error!!!  can't find setting in iq files, use 0 instead\n", __FUNCTION__, __LINE__);
    }

    res = ynr_init_params_json_V3(pParams, pCalibdbV2, calib_idx, tuning_idx);
    pParams->enable = pCalibdbV2->TuningPara.enable;
    return res;

}

RKAIQ_END_DECLARE

