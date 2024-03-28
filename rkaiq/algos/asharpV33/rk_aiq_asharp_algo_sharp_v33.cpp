/*
 * rk_aiq_asharp_algo_sharp_v33.cpp
 *
 *  Copyright (c) 2022 Rockchip Corporation
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

#include "rk_aiq_asharp_algo_sharp_v33.h"

RKAIQ_BEGIN_DECLARE

Asharp_result_V33_t sharp_select_params_by_ISO_V33(void* pParams_v, void* pSelect_v,
        Asharp_ExpInfo_V33_t* pExpInfo) {
    Asharp_result_V33_t res = ASHARP_V33_RET_SUCCESS;
#if RKAIQ_HAVE_SHARP_V33
    RK_SHARP_Params_V33_t* pParams        = (RK_SHARP_Params_V33_t*)pParams_v;
    RK_SHARP_Params_V33_Select_t* pSelect = (RK_SHARP_Params_V33_Select_t*)pSelect_v;
#else
    RK_SHARP_Params_V33LT_t* pParams        = (RK_SHARP_Params_V33LT_t*)pParams_v;
    RK_SHARP_Params_V33LT_Select_t* pSelect = (RK_SHARP_Params_V33LT_Select_t*)pSelect_v;
#endif

    int i;
    int iso     = 50;
    int iso_div = 50;
    float ratio = 0.0f;
    int iso_low = iso, iso_high = iso;
    int gain_high = 0, gain_low = 0;
    int max_iso_step = RK_SHARP_V33_MAX_ISO_NUM;

    LOGI_ASHARP("%s(%d): enter\n", __FUNCTION__, __LINE__);

    if (pParams == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP_V33_RET_NULL_POINTER;
    }

    if (pSelect == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP_V33_RET_NULL_POINTER;
    }

    if (pExpInfo == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP_V33_RET_NULL_POINTER;
    }

    iso = pExpInfo->arIso[pExpInfo->hdr_mode];

#ifndef RK_SIMULATOR_HW
    for (i = 0; i < max_iso_step - 1; i++) {
        if (iso >= pParams->iso[i] && iso <= pParams->iso[i + 1]) {
            iso_low   = pParams->iso[i];
            iso_high  = pParams->iso[i + 1];
            gain_low  = i;
            gain_high = i + 1;
            ratio     = (float)(iso - iso_low) / (iso_high - iso_low);
            break;
        }
    }

    if (iso < pParams->iso[0]) {
        iso_low   = pParams->iso[0];
        iso_high  = pParams->iso[1];
        gain_low  = 0;
        gain_high = 1;
        ratio     = 0;
    }

    if (iso > pParams->iso[max_iso_step - 1]) {
        iso_low   = pParams->iso[max_iso_step - 2];
        iso_high  = pParams->iso[max_iso_step - 1];
        gain_low  = max_iso_step - 2;
        gain_high = max_iso_step - 1;
        ratio     = 1;
    }
#else
    for (i = max_iso_step - 1; i >= 0; i--) {
        if (iso < iso_div * (2 << i)) {
            iso_low  = iso_div * (2 << (i)) / 2;
            iso_high = iso_div * (2 << i);
        }
    }

    ratio = (float)(iso - iso_low) / (iso_high - iso_low);
    if (iso_low == iso) {
        iso_high = iso;
        ratio    = 0;
    }
    if (iso_high == iso) {
        iso_low = iso;
        ratio   = 1;
    }
    gain_high = (int)(log((float)iso_high / 50) / log((float)2));
    gain_low  = (int)(log((float)iso_low / 50) / log((float)2));

    gain_low  = MIN(MAX(gain_low, 0), max_iso_step - 1);
    gain_high = MIN(MAX(gain_high, 0), max_iso_step - 1);
#endif

    LOGD_ASHARP("%s:%d iso:%d gainlow:%d gian_high:%d\n", __FUNCTION__, __LINE__, iso, gain_high,
                gain_high);

    pExpInfo->isoLevelLow = gain_low;
    pExpInfo->isoLevelHig = gain_high;
    pSelect->enable = pParams->enable;

    pSelect->Center_Mode    = pParams->Center_Mode;
    pSelect->kernel_sigma_enable = pParams->kernel_sigma_enable;
    pSelect->sharp_ratio_seperate_en = pParams->sharp_ratio_seperate_en;
    for (int i = 0; i < RK_SHARP_V33_LUMA_POINT_NUM; i++) {
        pSelect->luma_point[i] =
            pParams->sharpParamsISO[gain_low].luma_point[i];
        pSelect->luma_sigma[i] =
            INTERP_V4(pParams->sharpParamsISO[gain_low].luma_sigma[i],
                      pParams->sharpParamsISO[gain_high].luma_sigma[i], ratio);
        pSelect->hf_clip[i] =
            (uint16_t)ROUND_F(INTERP_V4(pParams->sharpParamsISO[gain_low].hf_clip[i],
                                        pParams->sharpParamsISO[gain_high].hf_clip[i], ratio));
#if RKAIQ_HAVE_SHARP_V33_LITE
        pSelect->hf_clip_neg[i] =
            (uint16_t)ROUND_F(INTERP_V4(pParams->sharpParamsISO[gain_low].hf_clip_neg[i],
                                        pParams->sharpParamsISO[gain_high].hf_clip_neg[i], ratio));
        pSelect->local_sharp_strength[i] =
            INTERP_V4(pParams->sharpParamsISO[gain_low].local_sharp_strength[i],
                      pParams->sharpParamsISO[gain_high].local_sharp_strength[i], ratio);
#endif
    }

    if (iso - iso_low <= iso_high - iso) {
        pSelect->GaussianFilter_radius_0 = pParams->sharpParamsISO[gain_low].GaussianFilter_radius_0;
        pSelect->GaussianFilter_radius_1 = pParams->sharpParamsISO[gain_low].GaussianFilter_radius_1;
        pSelect->GaussianFilter_radius = pParams->sharpParamsISO[gain_low].GaussianFilter_radius;
#if RKAIQ_HAVE_SHARP_V33
        pSelect->global_hf_clip_pos = pParams->sharpParamsISO[gain_low].global_hf_clip_pos;
        pSelect->noiseclip_mode = pParams->sharpParamsISO[gain_low].noiseclip_mode;
#endif
#if RKAIQ_HAVE_SHARP_V33_LITE
        pSelect->clip_hf_mode = pParams->sharpParamsISO[gain_low].clip_hf_mode;
        pSelect->add_mode     = pParams->sharpParamsISO[gain_low].add_mode;
#endif
    } else {
        pSelect->GaussianFilter_radius_0 = pParams->sharpParamsISO[gain_high].GaussianFilter_radius_0;
        pSelect->GaussianFilter_radius_1 = pParams->sharpParamsISO[gain_high].GaussianFilter_radius_1;
        pSelect->GaussianFilter_radius = pParams->sharpParamsISO[gain_high].GaussianFilter_radius;
#if RKAIQ_HAVE_SHARP_V33
        pSelect->global_hf_clip_pos = pParams->sharpParamsISO[gain_high].global_hf_clip_pos;
        pSelect->noiseclip_mode = pParams->sharpParamsISO[gain_high].noiseclip_mode;
#endif
#if RKAIQ_HAVE_SHARP_V33_LITE
        pSelect->clip_hf_mode = pParams->sharpParamsISO[gain_high].clip_hf_mode;
        pSelect->add_mode     = pParams->sharpParamsISO[gain_high].add_mode;
#endif
    }

    pSelect->prefilter_sigma =
        INTERP_V4(pParams->sharpParamsISO[gain_low].prefilter_sigma,
                  pParams->sharpParamsISO[gain_high].prefilter_sigma, ratio);
    pSelect->hfBilateralFilter_sigma =
        INTERP_V4(pParams->sharpParamsISO[gain_low].hfBilateralFilter_sigma,
                  pParams->sharpParamsISO[gain_high].hfBilateralFilter_sigma, ratio);
    pSelect->GaussianFilter_sigma_0 =
        INTERP_V4(pParams->sharpParamsISO[gain_low].GaussianFilter_sigma_0,
                  pParams->sharpParamsISO[gain_high].GaussianFilter_sigma_0, ratio);
    pSelect->GaussianFilter_sigma_1 =
        INTERP_V4(pParams->sharpParamsISO[gain_low].GaussianFilter_sigma_1,
                  pParams->sharpParamsISO[gain_high].GaussianFilter_sigma_1, ratio);
    pSelect->GaussianFilter_sigma =
        INTERP_V4(pParams->sharpParamsISO[gain_low].GaussianFilter_sigma,
                  pParams->sharpParamsISO[gain_high].GaussianFilter_sigma, ratio);

    pSelect->pbf_gain =
        INTERP_V4(pParams->sharpParamsISO[gain_low].pbf_gain,
                  pParams->sharpParamsISO[gain_high].pbf_gain, ratio);
    pSelect->pbf_add =
        INTERP_V4(pParams->sharpParamsISO[gain_low].pbf_add,
                  pParams->sharpParamsISO[gain_high].pbf_add, ratio);
    pSelect->pbf_ratio =
        INTERP_V4(pParams->sharpParamsISO[gain_low].pbf_ratio,
                  pParams->sharpParamsISO[gain_high].pbf_ratio, ratio);

    pSelect->gaus_ratio =
        INTERP_V4(pParams->sharpParamsISO[gain_low].gaus_ratio,
                  pParams->sharpParamsISO[gain_high].gaus_ratio, ratio);

    pSelect->sharp_ratio_0 =
        INTERP_V4(pParams->sharpParamsISO[gain_low].sharp_ratio_0,
                  pParams->sharpParamsISO[gain_high].sharp_ratio_0, ratio);
    pSelect->sharp_ratio_1 =
        INTERP_V4(pParams->sharpParamsISO[gain_low].sharp_ratio_1,
                  pParams->sharpParamsISO[gain_high].sharp_ratio_1, ratio);
    pSelect->sharp_ratio =
        INTERP_V4(pParams->sharpParamsISO[gain_low].sharp_ratio,
                  pParams->sharpParamsISO[gain_high].sharp_ratio, ratio);


    pSelect->bf_gain =
        INTERP_V4(pParams->sharpParamsISO[gain_low].bf_gain,
                  pParams->sharpParamsISO[gain_high].bf_gain, ratio);
    pSelect->bf_add = INTERP_V4(
                          pParams->sharpParamsISO[gain_low].bf_add, pParams->sharpParamsISO[gain_high].bf_add, ratio);
    pSelect->bf_ratio =
        INTERP_V4(pParams->sharpParamsISO[gain_low].bf_ratio,
                  pParams->sharpParamsISO[gain_high].bf_ratio, ratio);

    for (int i = 0; i < 3; i++) {
        pSelect->prefilter_coeff[i] =
            INTERP_V4(pParams->sharpParamsISO[gain_low].prefilter_coeff[i],
                      pParams->sharpParamsISO[gain_high].prefilter_coeff[i], ratio);
    }

    for (int i = 0; i < 6; i++) {
        pSelect->GaussianFilter_coeff_0[i] =
            INTERP_V4(pParams->sharpParamsISO[gain_low].GaussianFilter_coeff_0[i],
                      pParams->sharpParamsISO[gain_high].GaussianFilter_coeff_0[i], ratio);
        pSelect->GaussianFilter_coeff_1[i] =
            INTERP_V4(pParams->sharpParamsISO[gain_low].GaussianFilter_coeff_1[i],
                      pParams->sharpParamsISO[gain_high].GaussianFilter_coeff_1[i], ratio);
        pSelect->GaussianFilter_coeff[i] =
            INTERP_V4(pParams->sharpParamsISO[gain_low].GaussianFilter_coeff[i],
                      pParams->sharpParamsISO[gain_high].GaussianFilter_coeff[i], ratio);

    }

    for (int i = 0; i < 3; i++) {
        pSelect->hfBilateralFilter_coeff[i] =
            INTERP_V4(pParams->sharpParamsISO[gain_low].hfBilateralFilter_coeff[i],
                      pParams->sharpParamsISO[gain_high].hfBilateralFilter_coeff[i], ratio);
    }

    pSelect->global_gain =
        INTERP_V4(pParams->sharpParamsISO[gain_low].global_gain,
                  pParams->sharpParamsISO[gain_high].global_gain, ratio);
    pSelect->global_gain_alpha =
        INTERP_V4(pParams->sharpParamsISO[gain_low].global_gain_alpha,
                  pParams->sharpParamsISO[gain_high].global_gain_alpha, ratio);
    pSelect->local_gainscale =
        INTERP_V4(pParams->sharpParamsISO[gain_low].local_gainscale,
                  pParams->sharpParamsISO[gain_high].local_gainscale, ratio);

    for (int i = 0; i < RK_SHARP_V33_SHARP_ADJ_GAIN_TABLE_LEN; i++) {
        pSelect->gain_adj_sharp_strength[i] =
            INTERP_V4(pParams->sharpParamsISO[gain_low].gain_adj_sharp_strength[i],
                      pParams->sharpParamsISO[gain_high].gain_adj_sharp_strength[i], ratio);
    }
    pSelect->exgain_bypass =
        INTERP_V4(pParams->sharpParamsISO[gain_low].exgain_bypass,
                  pParams->sharpParamsISO[gain_high].exgain_bypass, ratio);

    for (int i = 0; i < RK_SHARP_V33_STRENGTH_TABLE_LEN; i++) {
        pSelect->dis_adj_sharp_strength[i] =
            INTERP_V4(pParams->sharpParamsISO[gain_low].dis_adj_sharp_strength[i],
                      pParams->sharpParamsISO[gain_high].dis_adj_sharp_strength[i], ratio);
    }

#if RKAIQ_HAVE_SHARP_V33
    pSelect->noiseclip_strength =
        INTERP_V4(pParams->sharpParamsISO[gain_low].noiseclip_strength,
                  pParams->sharpParamsISO[gain_high].noiseclip_strength, ratio);
    pSelect->enhance_bit =
        INTERP_V4(pParams->sharpParamsISO[gain_low].enhance_bit,
                  pParams->sharpParamsISO[gain_high].enhance_bit, ratio);
    pSelect->noise_sigma_clip =
        INTERP_V4(pParams->sharpParamsISO[gain_low].noise_sigma_clip,
                  pParams->sharpParamsISO[gain_high].noise_sigma_clip, ratio);
#endif

    LOGI_ASHARP("%s(%d): exit\n", __FUNCTION__, __LINE__);
    return res;
}

Asharp_result_V33_t sharp_fix_transfer_V33(void* pSelect_v, RK_SHARP_Fix_V33_t* pFix,
        rk_aiq_sharp_strength_v33_t* pStrength,
        Asharp_ExpInfo_V33_t* pExpInfo) {
    int sum_coeff, offset;
    int pbf_sigma_shift     = 0;
    int bf_sigma_shift      = 0;
    Asharp_result_V33_t res = ASHARP_V33_RET_SUCCESS;
    int tmp                 = 0;

#if RKAIQ_HAVE_SHARP_V33
    RK_SHARP_Params_V33_Select_t* pSelect = (RK_SHARP_Params_V33_Select_t*)pSelect_v;
#else
    RK_SHARP_Params_V33LT_Select_t* pSelect = (RK_SHARP_Params_V33LT_Select_t*)pSelect_v;
#endif

    LOGI_ASHARP("%s(%d): enter\n", __FUNCTION__, __LINE__);

    if (pSelect == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP_V33_RET_NULL_POINTER;
    }

    if (pFix == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP_V33_RET_NULL_POINTER;
    }

    if (pStrength == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP_V33_RET_NULL_POINTER;
    }

    if(pExpInfo == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP_V33_RET_NULL_POINTER;
    }

    float fPercent = 1.0f;

    if(pStrength->strength_enable) {
        fPercent = pStrength->percent;
    }
    if(fPercent <= 0.0) {
        fPercent = 0.000001;
    }

    LOGD_ASHARP("strength_enable:%d fPercent: %f \n", pStrength->strength_enable, fPercent);

    LOGD_ASHARP("%s:%d strength:%f raw:width:%d height:%d\n",
                __FUNCTION__, __LINE__,
                fPercent, pExpInfo->rawHeight, pExpInfo->rawWidth);

    int rows = pExpInfo->rawHeight; //raw height
    int cols = pExpInfo->rawWidth; //raw  width

    // SHARP_EN (0x0000)
    pFix->sharp_exgain_bypass = pSelect->exgain_bypass;
    pFix->sharp_center_mode = 0;
#if RKAIQ_HAVE_SHARP_V33
    pFix->sharp_noiseclip_mode = pSelect->noiseclip_mode;
#else
    pFix->sharp_noiseclip_mode              = 0;
#endif
    pFix->sharp_bypass        = !pSelect->enable;
    pFix->sharp_en            = pSelect->enable;

#if RKAIQ_HAVE_SHARP_V33_LITE
    pFix->sharp_clip_hf_mode = pSelect->clip_hf_mode;
    pFix->sharp_add_mode     = pSelect->add_mode;
#else
    pFix->sharp_clip_hf_mode                = 0;
    pFix->sharp_add_mode                    = 0;
#endif

    if (cols > 3072 && rows > 1728) {
        pFix->sharp_radius_ds_mode = 1;
    } else {
        pFix->sharp_radius_ds_mode = 0;
    }
#if RKAIQ_HAVE_SHARP_V33
    LOGD_ASHARP("sharp_noiseclip_mode:%d \n", pFix->sharp_noiseclip_mode);
#endif

    // CENTER
    pFix->sharp_center_mode   = pSelect->Center_Mode;
    tmp = cols / 2;
    pFix->sharp_center_wid = CLIP(tmp, 0, 8191);
    tmp = rows / 2;
    pFix->sharp_center_het = CLIP(tmp, 0, 8191);

    // SHARP_RATIO  (0x0004)
    tmp                     = (int)ROUND_F(pSelect->pbf_ratio / fPercent *
                                           (1 << RK_SHARP_V33_BF_RATIO_FIX_BITS));
    pFix->sharp_pbf_ratio   = CLIP(tmp, 0, 0x80);
    tmp                     = (int)ROUND_F(pSelect->gaus_ratio / fPercent *
                                           (1 << RK_SHARP_V33_GAUS_RATIO_FIX_BITS));
    pFix->sharp_gaus_ratio  = CLIP(tmp, 0, 0x80);
    if(pSelect->sharp_ratio_seperate_en) {
        tmp = (int)ROUND_F( (pSelect->sharp_ratio_0 + pSelect->sharp_ratio_1) * fPercent *
                            (1 << RK_SHARP_V33_SHARP_RATIO_FIX_BITS));
    } else {
        tmp = (int)ROUND_F(pSelect->sharp_ratio * fPercent *
                           (1 << RK_SHARP_V33_SHARP_RATIO_FIX_BITS));
    }

    pFix->sharp_sharp_ratio = CLIP(tmp, 0, 127);
    tmp                     = (int)ROUND_F(pSelect->bf_ratio / fPercent *
                                           (1 << RK_SHARP_V33_BF_RATIO_FIX_BITS));
    pFix->sharp_bf_ratio    = CLIP(tmp, 0, 0x80);

    // SHARP_LUMA_DX (0x0008)
    for (int i = 0; i < RK_SHARP_V33_LUMA_POINT_NUM - 1; i++) {
        tmp                    = (int16_t)LOG2(pSelect->luma_point[i + 1] -
                                               pSelect->luma_point[i]);
        pFix->sharp_luma_dx[i] = CLIP(tmp, 0, 15);
    }

    // SHARP_PBF_SIGMA_INV_0 (0x000c - 0x0014)
    // pre bf sigma inv
    int sigma_deci_bits = 9;
    int sigma_inte_bits = 1;
    int max_val         = 0;
    int min_val         = 65536;
    short sigma_bits[3];
    for (int i = 0; i < RK_SHARP_V33_LUMA_POINT_NUM; i++) {
        int cur_sigma = FLOOR((pSelect->luma_sigma[i] * pSelect->pbf_gain +
                               pSelect->pbf_add) / fPercent);
        if (max_val < cur_sigma) max_val = cur_sigma;
        if (min_val > cur_sigma) min_val = cur_sigma;
    }
    sigma_bits[0]   = FLOOR(log((float)min_val) / log((float)2));
    sigma_bits[1]   = MAX(sigma_inte_bits - sigma_bits[0], 0);
    sigma_bits[2]   = sigma_deci_bits + sigma_bits[0];
    pbf_sigma_shift = sigma_bits[2] - 5;
    for (int i = 0; i < RK_SHARP_V33_LUMA_POINT_NUM; i++) {
        tmp = (int16_t)ROUND_F((float)1 / (pSelect->luma_sigma[i] * pSelect->pbf_gain +
                                           pSelect->pbf_add) * fPercent * (1 << sigma_bits[2]));
        pFix->sharp_pbf_sigma_inv[i] = CLIP(tmp, 0, 4095);
    }

    // SHARP_BF_SIGMA_INV_0 (0x0018 -  0x0020)
    // bf sigma inv
    sigma_deci_bits = 9;
    sigma_inte_bits = 1;
    max_val         = 0;
    min_val         = 65536;
    for (int i = 0; i < RK_SHARP_V33_LUMA_POINT_NUM; i++) {
        int cur_sigma = FLOOR((pSelect->luma_sigma[i] * pSelect->bf_gain
                               + pSelect->bf_add) / fPercent);
        if (max_val < cur_sigma) max_val = cur_sigma;
        if (min_val > cur_sigma) min_val = cur_sigma;
    }
    sigma_bits[0]  = FLOOR(log((float)min_val) / log((float)2));
    sigma_bits[1]  = MAX(sigma_inte_bits - sigma_bits[0], 0);
    sigma_bits[2]  = sigma_deci_bits + sigma_bits[0];
    bf_sigma_shift = sigma_bits[2] - 5;
    for (int i = 0; i < RK_SHARP_V33_LUMA_POINT_NUM; i++) {
        tmp = (int16_t)ROUND_F((float)1 / (pSelect->luma_sigma[i] * pSelect->bf_gain +
                                           pSelect->bf_add) * fPercent * (1 << sigma_bits[2]));
        pFix->sharp_bf_sigma_inv[i] = CLIP(tmp, 0, 4095);
    }

    // SHARP_SIGMA_SHIFT (0x00024)
    pFix->sharp_pbf_sigma_shift = CLIP(pbf_sigma_shift, 0, 15);
    pFix->sharp_bf_sigma_shift  = CLIP(bf_sigma_shift, 0, 15);

    // SHARP_SHARP_CLIP_HF_0 (0x0034 -  0x003c)
    for (int i = 0; i < RK_SHARP_V33_LUMA_POINT_NUM; i++) {
        tmp                    = (int)(pSelect->hf_clip[i] * fPercent);
        pFix->sharp_clip_hf[i] = CLIP(tmp, 0, 1023);
#if RKAIQ_HAVE_SHARP_V33_LITE
        tmp                     = (int)(pSelect->hf_clip_neg[i] * fPercent);
        pFix->sharp_clip_neg[i] = CLIP(tmp, 0, 1023);
#else
        pFix->sharp_clip_neg[i] = 0;
#endif
    }

    // SHARP_SHARP_PBF_COEF (0x00040)
    // filter coeff
    // bf coeff
    // rk_sharp_V33_pbfCoeff : [4], [1], [0]
    float pre_bila_filter[3];
    if (pSelect->kernel_sigma_enable) {
        float dis_table_3x3[3] = {0.0, 1.0, 2.0};
        double e               = 2.71828182845905;
        float sigma            = pSelect->prefilter_sigma;
        float sum_gauss_coeff  = 0.0;
        for (int i = 0; i < 3; i++) {
            float tmp          = pow(e, -dis_table_3x3[i] / 2.0 / sigma / sigma);
            pre_bila_filter[i] = tmp;
        }
        sum_gauss_coeff = pre_bila_filter[0] + 4 * pre_bila_filter[1] + 4 * pre_bila_filter[2];
        for (int i = 0; i < 3; i++) {
            pre_bila_filter[i] = pre_bila_filter[i] / sum_gauss_coeff;
            LOGD_ASHARP("kernel_sigma_enable:%d pre_bila_filter[%d]:%f\n",
                        pSelect->kernel_sigma_enable, i, pre_bila_filter[i]);
        }
    } else {
        for (int i = 0; i < 3; i++) {
            pre_bila_filter[i] = pSelect->prefilter_coeff[i];
            LOGD_ASHARP("kernel_sigma_enable:%d pre_bila_filter[%d]:%f\n",
                        pSelect->kernel_sigma_enable, i, pre_bila_filter[i]);
        }
    }
    tmp = (int)ROUND_F(pre_bila_filter[0] * (1 << RK_SHARP_V33_PBFCOEFF_FIX_BITS));
    pFix->sharp_pbf_coef[0] = CLIP(tmp, 0, 127);
    tmp = (int)ROUND_F(pre_bila_filter[1] * (1 << RK_SHARP_V33_PBFCOEFF_FIX_BITS));
    pFix->sharp_pbf_coef[1] = CLIP(tmp, 0, 127);
    tmp = (int)ROUND_F(pre_bila_filter[2] * (1 << RK_SHARP_V33_PBFCOEFF_FIX_BITS));
    pFix->sharp_pbf_coef[2] = CLIP(tmp, 0, 127);

    sum_coeff = pFix->sharp_pbf_coef[0] + 4 * pFix->sharp_pbf_coef[1] + 4 * pFix->sharp_pbf_coef[2];
    offset    = (1 << RK_SHARP_V33_PBFCOEFF_FIX_BITS) - sum_coeff;
    tmp       = (int)(pFix->sharp_pbf_coef[0] + offset);
    pFix->sharp_pbf_coef[0] = CLIP(tmp, 0, 127);

    // SHARP_SHARP_GAUS_COEF (0x00048)
    // rk_sharp_V33_rfCoeff :  [4], [1], [0]
    float bila_filter[3];
    if (pSelect->kernel_sigma_enable) {
        float dis_table_3x3[3] = {0.0, 1.0, 2.0};
        double e               = 2.71828182845905;
        float sigma            = pSelect->hfBilateralFilter_sigma;
        float sum_gauss_coeff  = 0.0;
        for (int i = 0; i < 3; i++) {
            float tmp      = pow(e, -dis_table_3x3[i] / 2.0 / sigma / sigma);
            bila_filter[i] = tmp;
        }
        sum_gauss_coeff = bila_filter[0] + 4 * bila_filter[1] + 4 * bila_filter[2];
        for (int i = 0; i < 3; i++) {
            bila_filter[i] = bila_filter[i] / sum_gauss_coeff;
            LOGD_ASHARP("kernel_sigma_enable:%d bila_filter[%d]:%f\n", pSelect->kernel_sigma_enable,
                        i, bila_filter[i]);
        }
    } else {
        for (int i = 0; i < 3; i++) {
            bila_filter[i] = pSelect->hfBilateralFilter_coeff[i];
            LOGD_ASHARP("kernel_sigma_enable:%d bila_filter[%d]:%f\n", pSelect->kernel_sigma_enable,
                        i, bila_filter[i]);
        }
    }
    tmp                    = (int)ROUND_F(bila_filter[0] * (1 << RK_SHARP_V33_RFCOEFF_FIX_BITS));
    pFix->sharp_bf_coef[0] = CLIP(tmp, 0, 127);
    tmp                    = (int)ROUND_F(bila_filter[1] * (1 << RK_SHARP_V33_RFCOEFF_FIX_BITS));
    pFix->sharp_bf_coef[1] = CLIP(tmp, 0, 127);
    tmp                    = (int)ROUND_F(bila_filter[2] * (1 << RK_SHARP_V33_RFCOEFF_FIX_BITS));
    pFix->sharp_bf_coef[2] = CLIP(tmp, 0, 127);

    sum_coeff = pFix->sharp_bf_coef[0] + 4 * pFix->sharp_bf_coef[1] + 4 * pFix->sharp_bf_coef[2];
    offset    = (1 << RK_SHARP_V33_PBFCOEFF_FIX_BITS) - sum_coeff;
    tmp       = (int)(pFix->sharp_bf_coef[0] + offset);
    pFix->sharp_bf_coef[0] = CLIP(tmp, 0, 127);

    // SHARP_SHARP_BF_COEF (0x00044)
    // bf coeff
    // rk_sharp_V33_rfCoeff : [4], [1], [0]
    float range_filter[6];
    float kernel0_ratio = 0, kernel1_ratio = 0;
    if(pSelect->sharp_ratio_seperate_en) {
        kernel0_ratio = pSelect->sharp_ratio_0 / (pSelect->sharp_ratio_0 + pSelect->sharp_ratio_1);
        kernel1_ratio = pSelect->sharp_ratio_1 / (pSelect->sharp_ratio_0 + pSelect->sharp_ratio_1);
        LOGD_ASHARP("GaussianFilter_sigma = %f %f ,GaussianFilter_radius %f %f \n",
                    pSelect->GaussianFilter_sigma_0, pSelect->GaussianFilter_sigma_1,
                    pSelect->GaussianFilter_radius_0, pSelect->GaussianFilter_radius_1);
    } else {
        LOGD_ASHARP("GaussianFilter_sigma = %f ,GaussianFilter_radius %f \n",
                    pSelect->GaussianFilter_sigma,
                    pSelect->GaussianFilter_radius);
    }
    if (pSelect->kernel_sigma_enable) {
        float gauss_dis_table_5x5[6] = {0.0, 1.0, 2.0, 4.0, 5.0, 8.0};
        float gauss_dis_table_3x3[6] = {0.0, 1.0, 2.0, 1000, 1000, 1000};
        double e                     = 2.71828182845905;
        if(pSelect->sharp_ratio_seperate_en) {
            float sigma                 = pSelect->GaussianFilter_sigma_0;
            float sigma1                  = pSelect->GaussianFilter_sigma_1;
            float range_filter_1[6];
            float sum_gauss_coeff        = 0.0;
            float sum_gauss_coeff_1        = 0.0;
            if (pSelect->GaussianFilter_radius_0 == 2) {
                for (int i = 0; i < 6; i++) {
                    float tmp       = pow(e, -gauss_dis_table_5x5[i] / 2.0 / sigma / sigma);
                    range_filter[i] = tmp;
                }
            } else {
                for (int i = 0; i < 6; i++) {
                    float tmp       = pow(e, -gauss_dis_table_3x3[i] / 2.0 / sigma / sigma);
                    range_filter[i] = tmp;
                }
            }

            if (pSelect->GaussianFilter_radius_1 == 2) {
                for (int i = 0; i < 6; i++) {
                    float tmp       = pow(e, -gauss_dis_table_5x5[i] / 2.0 / sigma1 / sigma1);
                    range_filter_1[i] = tmp;
                }
            } else {
                for (int i = 0; i < 6; i++) {
                    float tmp       = pow(e, -gauss_dis_table_3x3[i] / 2.0 / sigma1 / sigma1);
                    range_filter_1[i] = tmp;
                }
            }

            sum_gauss_coeff = range_filter[0] + 4 * range_filter[1] + 4 * range_filter[2] +
                              4 * range_filter[3] + 8 * range_filter[4] + 4 * range_filter[5];
            for (int i = 0; i < 6; i++) {
                range_filter[i] = range_filter[i] / sum_gauss_coeff;
            }

            sum_gauss_coeff_1 = range_filter_1[0] + 4 * range_filter_1[1] + 4 * range_filter_1[2] +
                                4 * range_filter_1[3] + 8 * range_filter_1[4] + 4 * range_filter_1[5];
            for (int i = 0; i < 6; i++) {
                range_filter_1[i] = range_filter_1[i] / sum_gauss_coeff_1;
            }


            for (int i = 0; i < 6; i++) {
                range_filter[i] = kernel0_ratio * range_filter[i] + kernel1_ratio * range_filter_1[i];
                LOGD_ASHARP("kernel_sigma_enable:%d range_filter_combine[%d]:%f\n",
                            pSelect->kernel_sigma_enable, i, range_filter[i]);
            }
        } else {
            float sigma                  = pSelect->GaussianFilter_sigma;
            float sum_gauss_coeff        = 0.0;
            if (pSelect->GaussianFilter_radius == 2) {
                for (int i = 0; i < 6; i++) {
                    float tmp       = pow(e, -gauss_dis_table_5x5[i] / 2.0 / sigma / sigma);
                    range_filter[i] = tmp;
                }
            } else {
                for (int i = 0; i < 6; i++) {
                    float tmp       = pow(e, -gauss_dis_table_3x3[i] / 2.0 / sigma / sigma);
                    range_filter[i] = tmp;
                }
            }

            sum_gauss_coeff = range_filter[0] + 4 * range_filter[1] + 4 * range_filter[2] +
                              4 * range_filter[3] + 8 * range_filter[4] + 4 * range_filter[5];
            for (int i = 0; i < 6; i++) {
                range_filter[i] = range_filter[i] / sum_gauss_coeff;
                LOGD_ASHARP("kernel_sigma_enable:%d range_filter[%d]:%f\n",
                            pSelect->kernel_sigma_enable, i, range_filter[i]);
            }
        }
    } else {
        for (int i = 0; i < 6; i++) {
            if(pSelect->sharp_ratio_seperate_en) {
                range_filter[i] = kernel0_ratio * pSelect->GaussianFilter_coeff_0[i]
                                  + kernel1_ratio * pSelect->GaussianFilter_coeff_1[i];
            } else {
                range_filter[i] = pSelect->GaussianFilter_coeff[i];
            }
            LOGD_ASHARP("kernel_sigma_enable:%d range_filter[%d]:%f\n",
                        pSelect->kernel_sigma_enable, i, range_filter[i]);
        }
    }
    tmp = (int)ROUND_F(range_filter[0] * (1 << RK_SHARP_V33_HBFCOEFF_FIX_BITS));
    pFix->sharp_gaus_coef[0] = CLIP(tmp, 0, 127);
    tmp = (int)ROUND_F(range_filter[1] * (1 << RK_SHARP_V33_HBFCOEFF_FIX_BITS));
    pFix->sharp_gaus_coef[1] = CLIP(tmp, 0, 127);
    tmp = (int)ROUND_F(range_filter[2] * (1 << RK_SHARP_V33_HBFCOEFF_FIX_BITS));
    pFix->sharp_gaus_coef[2] = CLIP(tmp, 0, 127);
    tmp = (int)ROUND_F(range_filter[3] * (1 << RK_SHARP_V33_HBFCOEFF_FIX_BITS));
    pFix->sharp_gaus_coef[3] = CLIP(tmp, 0, 127);
    tmp = (int)ROUND_F(range_filter[4] * (1 << RK_SHARP_V33_HBFCOEFF_FIX_BITS));
    pFix->sharp_gaus_coef[4] = CLIP(tmp, 0, 127);
    tmp = (int)ROUND_F(range_filter[5] * (1 << RK_SHARP_V33_HBFCOEFF_FIX_BITS));
    pFix->sharp_gaus_coef[5] = CLIP(tmp, 0, 127);

    sum_coeff = pFix->sharp_gaus_coef[0] +
                4 * pFix->sharp_gaus_coef[1] +
                4 * pFix->sharp_gaus_coef[2] +
                4 * pFix->sharp_gaus_coef[3] +
                8 * pFix->sharp_gaus_coef[4] +
                4 * pFix->sharp_gaus_coef[5];
    offset                   = (1 << RK_SHARP_V33_RFCOEFF_FIX_BITS) - sum_coeff;
    tmp                      = (int)(pFix->sharp_gaus_coef[0] + offset);
    pFix->sharp_gaus_coef[0] = CLIP(tmp, 0, 127);

    // gain
    tmp = pSelect->global_gain * (1 << RK_SHARP_V33_GLOBAL_GAIN_FIX_BITS);
    pFix->sharp_global_gain = CLIP(tmp, 0, 1023);
    tmp                     = pSelect->global_gain_alpha *
                              (1 << RK_SHARP_V33_GLOBAL_GAIN_ALPHA_FIX_BITS);
    pFix->sharp_global_gain_alpha = CLIP(tmp, 0, 8);
    tmp                           = pSelect->local_gainscale *
                                    (1 << RK_SHARP_V33_LOCAL_GAIN_SACLE_FIX_BITS);
    pFix->sharp_local_gainscale = CLIP(tmp, 0, 128);

    // gain adjust strength
    for (int i = 0; i < RK_SHARP_V33_SHARP_ADJ_GAIN_TABLE_LEN; i++) {
        tmp                     = ROUND_F(pSelect->gain_adj_sharp_strength[i] *
                                          (1 << RK_SHARP_V33_ADJ_GAIN_FIX_BITS));
        pFix->sharp_gain_adj[i] = CLIP(tmp, 0, 16384);
    }

    // gain dis strength
    for (int i = 0; i < RK_SHARP_V33_STRENGTH_TABLE_LEN; i++) {
        tmp                     = ROUND_F(pSelect->dis_adj_sharp_strength[i] *
                                          (1 << RK_SHARP_V33_STRENGTH_TABLE_FIX_BITS));
        pFix->sharp_strength[i] = CLIP(tmp, 0, 128);
    }

    // isp32 only: use sw_sharp_strength21 bit [7:6] to clip over shoot.
    // 2'b00: No clip
    // 2'b10: Use 512 to clip
    // 2'b01: Use 256 to clip
    // 2'b11: Reserved
#if RKAIQ_HAVE_SHARP_V33
    if (pSelect->global_hf_clip_pos == 1) {
        pFix->sharp_strength[RK_SHARP_V33_STRENGTH_TABLE_LEN - 1] = 0x40;
    } else if (pSelect->global_hf_clip_pos == 2) {
        pFix->sharp_strength[RK_SHARP_V33_STRENGTH_TABLE_LEN - 1] = 0x80;
    } else {
        pFix->sharp_strength[RK_SHARP_V33_STRENGTH_TABLE_LEN - 1] = 0;
    }
#endif

    // texture: sharp enhence strength
#if RKAIQ_HAVE_SHARP_V33
    tmp                        = ROUND_F(pSelect->noiseclip_strength *
                                         (1 << RK_SHARP_V33_ADJ_GAIN_FIX_BITS));
    pFix->sharp_noise_strength = CLIP(tmp, 0, 16383);
    tmp                        = ROUND_F(pSelect->enhance_bit);
    pFix->sharp_enhance_bit    = CLIP(tmp, 0, 9);
    tmp                        = ROUND_F(pSelect->noise_sigma_clip);
    pFix->sharp_noise_sigma    = CLIP(tmp, 0, 1023);
#else
    pFix->sharp_noise_strength = 0;
    pFix->sharp_enhance_bit    = 0;
    pFix->sharp_noise_sigma    = 0;
#endif

#if RKAIQ_HAVE_SHARP_V33_LITE
    for (int i = 0; i < RK_SHARP_V33_LUMA_POINT_NUM; i++) {
        tmp                   = ROUND_F(pSelect->local_sharp_strength[i]);
        pFix->sharp_ehf_th[i] = CLIP(tmp, 0, 1023);
    }
#else
    for (int i = 0; i < RK_SHARP_V33_LUMA_POINT_NUM; i++) {
        pFix->sharp_ehf_th[i] = 0;
    }
#endif

#if 1
    sharp_fix_printf_V33(pFix);
#endif

    LOGI_ASHARP("%s(%d): exit\n", __FUNCTION__, __LINE__);
    return res;
}

Asharp_result_V33_t sharp_fix_printf_V33(RK_SHARP_Fix_V33_t* pFix) {

    Asharp_result_V33_t res = ASHARP_V33_RET_SUCCESS;

    LOGD_ASHARP("%s:(%d) enter \n", __FUNCTION__, __LINE__);

    if (pFix == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP_V33_RET_NULL_POINTER;
    }

    // SHARP_SHARP_EN (0X0000)
    LOGD_ASHARP("(0x0000) sahrp_Center_Mode:0x%x sharp_bypass:0x%x sharp_en:0x%x \n",
                pFix->sharp_center_mode, pFix->sharp_bypass, pFix->sharp_en);

#if RKAIQ_HAVE_SHARP_V33_LITE
    // SHARP_SHARP_EN (0X0000)
    LOGD_ASHARP("(0x0000) sharp_clip_hf_mode:0x%x sharp_add_mode:0x%x  \n",
                pFix->sharp_clip_hf_mode, pFix->sharp_add_mode);
#endif

    // SHARP_SHARP_RATIO (0x0004)
    LOGD_ASHARP(
        "(0x0004) sharp_sharp_ratio:0x%x sharp_bf_ratio:0x%x sharp_gaus_ratio:0x%x "
        "sharp_pbf_ratio:0x%x \n",
        pFix->sharp_sharp_ratio, pFix->sharp_bf_ratio, pFix->sharp_gaus_ratio,
        pFix->sharp_pbf_ratio);

    // SHARP_SHARP_LUMA_DX (0x0008)
    for (int i = 0; i < 7; i++) {
        LOGD_ASHARP("(0x0008) sharp_luma_dx[%d]:0x%x \n", i, pFix->sharp_luma_dx[i]);
    }

    // SHARP_SHARP_PBF_SIGMA_INV_0 (0x000c - 0x0014)
    for (int i = 0; i < 8; i++) {
        LOGD_ASHARP("(0x000c - 0x0014) sharp_pbf_sigma_inv[%d]:0x%x \n", i,
                    pFix->sharp_pbf_sigma_inv[i]);
    }

    // SHARP_SHARP_BF_SIGMA_INV_0 (0x0018 -  0x0020)
    for (int i = 0; i < 8; i++) {
        LOGD_ASHARP("(0x0018 - 0x0020) sharp_bf_sigma_inv[%d]:0x%x \n", i,
                    pFix->sharp_bf_sigma_inv[i]);
    }

    // SHARP_SHARP_SIGMA_SHIFT (0x00024)
    LOGD_ASHARP("(0x00024) sharp_bf_sigma_shift:0x%x sharp_pbf_sigma_shift:0x%x \n",
                pFix->sharp_bf_sigma_shift, pFix->sharp_pbf_sigma_shift);

    // SHARP_SHARP_CLIP_HF_0 (0x0034 -  0x003c)
    for (int i = 0; i < 8; i++) {
        LOGD_ASHARP("(0x0034 - 0x003c) sharp_clip_hf[%d]:0x%x \n", i, pFix->sharp_clip_hf[i]);
    }

    // SHARP_SHARP_PBF_COEF (0x00040)
    for (int i = 0; i < 3; i++) {
        LOGD_ASHARP("(0x00040) sharp_pbf_coef[%d]:0x%x \n", i, pFix->sharp_pbf_coef[i]);
    }

    // SHARP_SHARP_BF_COEF (0x00044)
    for (int i = 0; i < 3; i++) {
        LOGD_ASHARP("(0x00044) sharp_bf_coef[%d]:0x%x \n", i, pFix->sharp_bf_coef[i]);
    }

    // SHARP_SHARP_GAUS_COEF (0x00048)
    for (int i = 0; i < 6; i++) {
        LOGD_ASHARP("(0x00048) sharp_gaus_coef[%d]:0x%x \n", i, pFix->sharp_gaus_coef[i]);
    }

    // SHARP_GAIN (0x0050)
    LOGD_ASHARP(
        "(0x0050) sharp_global_gain:0x%x sharp_global_gain_alpha:0x%x sharp_local_gaincale:0x%x \n",
        pFix->sharp_global_gain, pFix->sharp_global_gain_alpha, pFix->sharp_local_gainscale);

    // SHARP_GAIN_ADJUST (0x54)
    for (int i = 0; i < 14; i++) {
        LOGD_ASHARP("(0x00048) sharp_gain_adj[%d]:0x%x \n", i, pFix->sharp_gain_adj[i]);
    }

    // SHARP_CENTER (0x70)
    LOGD_ASHARP("(0x0070) sharp_center_wid:0x%x shrap_center_het:%x \n", pFix->sharp_center_wid,
                pFix->sharp_center_het);

    // SHARP_GAIN_DIS_STRENGTH (0x74)
    for (int i = 0; i < 22; i++) {
        LOGD_ASHARP("(0x00048) sharp_strength[%d]:0x%x \n", i, pFix->sharp_strength[i]);
    }

#if RKAIQ_HAVE_SHARP_V33
    // SHARP_TEXTURE (0x8c)
    LOGD_ASHARP("(0x0070) sharp_noise_sigma:0x%x sharp_enhance_bit:%x sharp_noise_strength:%x \n",
                pFix->sharp_noise_sigma, pFix->sharp_enhance_bit, pFix->sharp_noise_strength);
#endif

#if RKAIQ_HAVE_SHARP_V33_LITE
    // SHARP_TEXTURE (0x28)
    for (int i = 0; i < 8; i++) {
        LOGD_ASHARP("(0x00048) sharp_ehf_th[%d]:0x%x \n", i, pFix->sharp_ehf_th[i]);
    }

    // SHARP_TEXTURE (0x90)
    for (int i = 0; i < 8; i++) {
        LOGD_ASHARP("(0x00048) sharp_clip_neg[%d]:0x%x \n", i, pFix->sharp_clip_neg[i]);
    }
#endif

    return res;
}

Asharp_result_V33_t sharp_get_setting_by_name_json_V33(void* pCalibdbV2_v, char* name,
        int* tuning_idx) {
    int i = 0;
    Asharp_result_V33_t res = ASHARP_V33_RET_SUCCESS;

    LOGI_ASHARP("%s(%d): enter  \n", __FUNCTION__, __LINE__);

#if RKAIQ_HAVE_SHARP_V33
    CalibDbV2_SharpV33_t* pCalibdbV2 = (CalibDbV2_SharpV33_t*)pCalibdbV2_v;
#else
    CalibDbV2_SharpV33Lite_t* pCalibdbV2  = (CalibDbV2_SharpV33Lite_t*)pCalibdbV2_v;
#endif

    if(pCalibdbV2 == NULL || name == NULL || tuning_idx == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP_V33_RET_NULL_POINTER;
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

    LOGI_ASHARP("%s:%d snr_name:%s  snr_idx:%d i:%d \n",
                __FUNCTION__, __LINE__,
                name, *tuning_idx, i);
    return res;
}

Asharp_result_V33_t sharp_init_params_json_V33(void* pSharpParams_v, void* pCalibdbV2_v,
        int tuning_idx) {
    Asharp_result_V33_t res = ASHARP_V33_RET_SUCCESS;
    int i                   = 0;
    int j                   = 0;
#if RKAIQ_HAVE_SHARP_V33
    CalibDbV2_SharpV33_t* pCalibdbV2    = (CalibDbV2_SharpV33_t*)pCalibdbV2_v;
    RK_SHARP_Params_V33_t* pSharpParams = (RK_SHARP_Params_V33_t*)pSharpParams_v;
    CalibDbV2_SharpV33_T_ISO_t* pTuningISO;
#else
    CalibDbV2_SharpV33Lite_t* pCalibdbV2  = (CalibDbV2_SharpV33Lite_t*)pCalibdbV2_v;
    RK_SHARP_Params_V33LT_t* pSharpParams = (RK_SHARP_Params_V33LT_t*)pSharpParams_v;
    CalibDbV2_SharpV33LT_T_ISO_t* pTuningISO;
#endif

    LOGI_ASHARP("%s(%d): enter\n", __FUNCTION__, __LINE__);

    if (pSharpParams == NULL || pCalibdbV2 == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP_V33_RET_NULL_POINTER;
    }

    pSharpParams->enable              = pCalibdbV2->TuningPara.enable;
    pSharpParams->kernel_sigma_enable = pCalibdbV2->TuningPara.kernel_sigma_enable;
    pSharpParams->sharp_ratio_seperate_en = pCalibdbV2->TuningPara.sharp_ratio_seperate_en;
    pSharpParams->Center_Mode         = pCalibdbV2->TuningPara.Center_Mode;
    for (i = 0; i < pCalibdbV2->TuningPara.Setting[tuning_idx].Tuning_ISO_len &&
            i < RK_SHARP_V33_MAX_ISO_NUM;
            i++) {
        pTuningISO           = &pCalibdbV2->TuningPara.Setting[tuning_idx].Tuning_ISO[i];
        pSharpParams->iso[i] = pTuningISO->iso;
#if RKAIQ_HAVE_SHARP_V33_LITE
        pSharpParams->sharpParamsISO[i].clip_hf_mode = pTuningISO->clip_hf_mode;
        pSharpParams->sharpParamsISO[i].add_mode     = pTuningISO->add_mode;
#endif

        for (j = 0; j < RK_SHARP_V33_LUMA_POINT_NUM; j++) {
            pSharpParams->sharpParamsISO[i].luma_point[j] = pTuningISO->luma_para.luma_point[j];
            pSharpParams->sharpParamsISO[i].luma_sigma[j] = pTuningISO->luma_para.luma_sigma[j];
            pSharpParams->sharpParamsISO[i].hf_clip[j] = pTuningISO->luma_para.hf_clip[j];
#if RKAIQ_HAVE_SHARP_V33_LITE
            pSharpParams->sharpParamsISO[i].hf_clip_neg[j] = pTuningISO->luma_para.hf_clip_neg[j];
            pSharpParams->sharpParamsISO[i].local_sharp_strength[j] =
                pTuningISO->luma_para.local_sharp_strength[j];
#endif
        }

        pSharpParams->sharpParamsISO[i].pbf_gain        = pTuningISO->pbf_gain;
        pSharpParams->sharpParamsISO[i].pbf_add         = pTuningISO->pbf_add;
        pSharpParams->sharpParamsISO[i].pbf_ratio       = pTuningISO->pbf_ratio;
        pSharpParams->sharpParamsISO[i].gaus_ratio      = pTuningISO->gaus_ratio;
        pSharpParams->sharpParamsISO[i].sharp_ratio_0     = pTuningISO->sharp_ratio_0;
        pSharpParams->sharpParamsISO[i].sharp_ratio_1     = pTuningISO->sharp_ratio_1;
        pSharpParams->sharpParamsISO[i].sharp_ratio     = pTuningISO->sharp_ratio;

        pSharpParams->sharpParamsISO[i].bf_gain         = pTuningISO->bf_gain;
        pSharpParams->sharpParamsISO[i].bf_add          = pTuningISO->bf_add;
        pSharpParams->sharpParamsISO[i].bf_ratio        = pTuningISO->bf_ratio;
#if RKAIQ_HAVE_SHARP_V33
        pSharpParams->sharpParamsISO[i].global_hf_clip_pos = pTuningISO->global_hf_clip_pos;
#endif

        for (j = 0; j < 3; j++) {
            pSharpParams->sharpParamsISO[i].prefilter_coeff[j] =
                pTuningISO->kernel_para.prefilter_coeff[j];
            pSharpParams->sharpParamsISO[i].hfBilateralFilter_coeff[j] =
                pTuningISO->kernel_para.hfBilateralFilter_coeff[j];
            LOGD_ASHARP("kernel_pbf: index[%d][%d] = %f\n", i, j,
                        pSharpParams->sharpParamsISO[i].prefilter_coeff[j]);
            LOGD_ASHARP("kernel_bf: index[%d][%d] = %f\n", i, j,
                        pSharpParams->sharpParamsISO[i].hfBilateralFilter_coeff[j]);
        }

        for (j = 0; j < 6; j++) {
            pSharpParams->sharpParamsISO[i].GaussianFilter_coeff_0[j] =
                pTuningISO->kernel_para.GaussianFilter_coeff_0[j];
            pSharpParams->sharpParamsISO[i].GaussianFilter_coeff_1[j] =
                pTuningISO->kernel_para.GaussianFilter_coeff_1[j];
            pSharpParams->sharpParamsISO[i].GaussianFilter_coeff[j] =
                pTuningISO->kernel_para.GaussianFilter_coeff[j];
            LOGD_ASHARP("kernel: index[%d][%d] = %f\n", i, j,
                        pSharpParams->sharpParamsISO[i].GaussianFilter_coeff[j]);

        }

        pSharpParams->sharpParamsISO[i].global_gain       = pTuningISO->global_gain;
        pSharpParams->sharpParamsISO[i].global_gain_alpha = pTuningISO->global_gain_alpha;
        pSharpParams->sharpParamsISO[i].local_gainscale   = pTuningISO->local_gainscale;

        for (int j = 0; j < 14; j++) {
            pSharpParams->sharpParamsISO[i].gain_adj_sharp_strength[j] =
                pTuningISO->gain_adj_sharp_strength[j];
        }

        pSharpParams->sharpParamsISO[i].exgain_bypass = pTuningISO->exgain_bypass;

        for (int j = 0; j < 22; j++) {
            pSharpParams->sharpParamsISO[i].dis_adj_sharp_strength[j] =
                pTuningISO->dis_adj_sharp_strength[j];
        }

#if RKAIQ_HAVE_SHARP_V33
        pSharpParams->sharpParamsISO[i].noiseclip_strength = pTuningISO->noiseclip_strength;
        pSharpParams->sharpParamsISO[i].enhance_bit        = pTuningISO->enhance_bit;
        pSharpParams->sharpParamsISO[i].noiseclip_mode     = pTuningISO->noiseclip_mode;
        pSharpParams->sharpParamsISO[i].noise_sigma_clip   = pTuningISO->noise_sigma_clip;
#endif

        pSharpParams->sharpParamsISO[i].prefilter_sigma = pTuningISO->kernel_sigma.prefilter_sigma;
        pSharpParams->sharpParamsISO[i].hfBilateralFilter_sigma = pTuningISO->kernel_sigma.hfBilateralFilter_sigma;

        pSharpParams->sharpParamsISO[i].GaussianFilter_sigma_0 = pTuningISO->kernel_sigma.GaussianFilter_sigma_0;
        pSharpParams->sharpParamsISO[i].GaussianFilter_radius_0 = pTuningISO->kernel_sigma.GaussianFilter_radius_0;
        pSharpParams->sharpParamsISO[i].GaussianFilter_sigma_1 = pTuningISO->kernel_sigma.GaussianFilter_sigma_1;
        pSharpParams->sharpParamsISO[i].GaussianFilter_radius_1 = pTuningISO->kernel_sigma.GaussianFilter_radius_1;
        pSharpParams->sharpParamsISO[i].GaussianFilter_sigma = pTuningISO->kernel_sigma.GaussianFilter_sigma;
        pSharpParams->sharpParamsISO[i].GaussianFilter_radius = pTuningISO->kernel_sigma.GaussianFilter_radius;

    }

    LOGI_ASHARP("%s(%d): exit\n", __FUNCTION__, __LINE__);
    return res;
}

Asharp_result_V33_t sharp_config_setting_param_json_V33(void* pParams_v, void* pCalibdbV2_v,
        char* param_mode, char* snr_name) {
    Asharp_result_V33_t res = ASHARP_V33_RET_SUCCESS;
    int tuning_idx          = 0;
#if RKAIQ_HAVE_SHARP_V33
    CalibDbV2_SharpV33_t* pCalibdbV2 = (CalibDbV2_SharpV33_t*)pCalibdbV2_v;
    RK_SHARP_Params_V33_t* pParams   = (RK_SHARP_Params_V33_t*)pParams_v;
#else
    CalibDbV2_SharpV33Lite_t* pCalibdbV2 = (CalibDbV2_SharpV33Lite_t*)pCalibdbV2_v;
    RK_SHARP_Params_V33LT_t* pParams     = (RK_SHARP_Params_V33LT_t*)pParams_v;
#endif

    LOGI_ASHARP("%s(%d): enter\n", __FUNCTION__, __LINE__);

    if (pParams == NULL || pCalibdbV2 == NULL || param_mode == NULL || snr_name == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        if (pParams)
            pParams->enable = false;
        return ASHARP_V33_RET_NULL_POINTER;
    }

    res = sharp_get_setting_by_name_json_V33(pCalibdbV2, snr_name, &tuning_idx);
    if(res != ASHARP_V33_RET_SUCCESS) {
        LOGW_ASHARP("%s(%d): error!!!  can't find setting in iq files, use 0 instead\n", __FUNCTION__, __LINE__);
    }

    res             = sharp_init_params_json_V33(pParams, pCalibdbV2, tuning_idx);
    pParams->enable = pCalibdbV2->TuningPara.enable;

    LOGI_ASHARP("%s(%d): exit\n", __FUNCTION__, __LINE__);

    return res;
}

RKAIQ_END_DECLARE
