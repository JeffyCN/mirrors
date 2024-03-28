/*
 * rk_aiq_asharp_algo_sharp_v34.cpp
 *
 *  Copyright (c) 2024 Rockchip Corporation
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

#include "rk_aiq_asharp_algo_sharp_v34.h"

RKAIQ_BEGIN_DECLARE

Asharp_result_V34_t sharp_select_printf_V34(RK_SHARP_Params_V34_Select_t* pSelect) {
    Asharp_result_V34_t res = ASHARP_V34_RET_SUCCESS;

    LOG1_ASHARP("%s:(%d) enter \n", __FUNCTION__, __LINE__);

    if (pSelect == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP_V34_RET_NULL_POINTER;
    }

    LOGD_ASHARP(
        "%s:%d  enable:%d hw_sharp_centerPosition_mode:%d hw_sharp_localGain_bypass:%d "
        "hw_sharp_noiseClip_sel:%d hw_sharp_clipIdx_sel:%d hw_sharp_baseImg_sel:%d\n",
        __FUNCTION__, __LINE__, pSelect->enable, pSelect->hw_sharp_centerPosition_mode,
        pSelect->hw_sharp_localGain_bypass, pSelect->hw_sharp_noiseClip_sel,
        pSelect->hw_sharp_clipIdx_sel, pSelect->hw_sharp_baseImg_sel);

    LOGD_ASHARP(
        "%s:%d  hw_sharp_noiseFilt_sel:%d hw_sharp_tex2wgt_en:%d sw_sharp_imgLpf0_strg:%f "
        "sw_sharp_imgLpf1_strg:%f\n",
        __FUNCTION__, __LINE__, pSelect->hw_sharp_noiseFilt_sel, pSelect->hw_sharp_tex2wgt_en,
        pSelect->sw_sharp_imgLpf0_strg, pSelect->sw_sharp_imgLpf1_strg);

    LOGD_ASHARP("%s:%d  sw_sharp_luma2vsigma_val[0~7]:%d %d %d %d %d %d %d %d \n", __FUNCTION__,
                __LINE__, pSelect->sw_sharp_luma2vsigma_val[0],
                pSelect->sw_sharp_luma2vsigma_val[1], pSelect->sw_sharp_luma2vsigma_val[2],
                pSelect->sw_sharp_luma2vsigma_val[3], pSelect->sw_sharp_luma2vsigma_val[4],
                pSelect->sw_sharp_luma2vsigma_val[5], pSelect->sw_sharp_luma2vsigma_val[6],
                pSelect->sw_sharp_luma2vsigma_val[7]);

    LOGD_ASHARP(
        "%s:%d  sw_sharp_preBifilt_offset:%f sw_sharp_preBifilt_scale:%f "
        "sw_sharp_preBifilt_alpha:%f sw_sharp_preBifilt_rsigma:%f\n",
        __FUNCTION__, __LINE__, pSelect->sw_sharp_preBifilt_offset,
        pSelect->sw_sharp_preBifilt_scale, pSelect->sw_sharp_preBifilt_alpha,
        pSelect->sw_sharp_preBifilt_rsigma);

    LOGD_ASHARP(
        "%s:%d  sw_sharp_detailBifilt_offset:%f sw_sharp_detailBifilt_scale:%f "
        "sw_sharp_detailBifilt_alpha:%f sw_sharp_detailBifilt_rsigma:%f\n",
        __FUNCTION__, __LINE__, pSelect->sw_sharp_detailBifilt_offset,
        pSelect->sw_sharp_detailBifilt_scale, pSelect->sw_sharp_detailBifilt_alpha,
        pSelect->sw_sharp_detailBifilt_rsigma);

    LOGD_ASHARP("%s:%d  hw_sharp_luma2strg_val[0~7]:%d %d %d %d %d %d %d %d \n", __FUNCTION__,
                __LINE__, pSelect->hw_sharp_luma2strg_val[0], pSelect->hw_sharp_luma2strg_val[1],
                pSelect->hw_sharp_luma2strg_val[2], pSelect->hw_sharp_luma2strg_val[3],
                pSelect->hw_sharp_luma2strg_val[4], pSelect->hw_sharp_luma2strg_val[5],
                pSelect->hw_sharp_luma2strg_val[6], pSelect->hw_sharp_luma2strg_val[7]);

    LOGD_ASHARP("%s:%d  hw_sharp_luma2posClip_val[0~7]:%d %d %d %d %d %d %d %d \n", __FUNCTION__,
                __LINE__, pSelect->hw_sharp_luma2posClip_val[0],
                pSelect->hw_sharp_luma2posClip_val[1], pSelect->hw_sharp_luma2posClip_val[2],
                pSelect->hw_sharp_luma2posClip_val[3], pSelect->hw_sharp_luma2posClip_val[4],
                pSelect->hw_sharp_luma2posClip_val[5], pSelect->hw_sharp_luma2posClip_val[6],
                pSelect->hw_sharp_luma2posClip_val[7]);

    LOGD_ASHARP("%s:%d  sw_sharp_preBifilt_coeff:%f %f %f sw_sharp_detailBifilt_coeff:%f %f %f\n",
                __FUNCTION__, __LINE__, pSelect->sw_sharp_preBifilt_coeff[0],
                pSelect->sw_sharp_preBifilt_coeff[1], pSelect->sw_sharp_preBifilt_coeff[2],
                pSelect->sw_sharp_detailBifilt_coeff[0], pSelect->sw_sharp_detailBifilt_coeff[1],
                pSelect->sw_sharp_detailBifilt_coeff[2]);

    LOGD_ASHARP("%s:%d  sw_sharp_imgLpf0_coeff[0~5]:%f %f %f %f %f %f \n", __FUNCTION__, __LINE__,
                pSelect->sw_sharp_imgLpf0_coeff[0], pSelect->sw_sharp_imgLpf0_coeff[1],
                pSelect->sw_sharp_imgLpf0_coeff[2], pSelect->sw_sharp_imgLpf0_coeff[3],
                pSelect->sw_sharp_imgLpf0_coeff[4], pSelect->sw_sharp_imgLpf0_coeff[5]);

    LOGD_ASHARP("%s:%d  sw_sharp_imgLpf1_coeff[0~5]:%f %f %f %f %f %f \n", __FUNCTION__, __LINE__,
                pSelect->sw_sharp_imgLpf1_coeff[0], pSelect->sw_sharp_imgLpf1_coeff[1],
                pSelect->sw_sharp_imgLpf1_coeff[2], pSelect->sw_sharp_imgLpf1_coeff[3],
                pSelect->sw_sharp_imgLpf1_coeff[4], pSelect->sw_sharp_imgLpf1_coeff[5]);

    LOGD_ASHARP(
        "%s:%d  sw_sharp_global_gain:%f sw_sharp_gainMerge_alpha:%f sw_sharp_localGain_scale:%f\n",
        __FUNCTION__, __LINE__, pSelect->sw_sharp_global_gain, pSelect->sw_sharp_gainMerge_alpha,
        pSelect->sw_sharp_localGain_scale);

    LOGD_ASHARP(
        "%s:%d  sw_sharp_gain2strg_val[0~13]:%f %f %f %f %f %f %f %f %f %f %f %f %f "
        "%f\n",
        __FUNCTION__, __LINE__, pSelect->sw_sharp_gain2strg_val[0],
        pSelect->sw_sharp_gain2strg_val[1], pSelect->sw_sharp_gain2strg_val[2],
        pSelect->sw_sharp_gain2strg_val[3], pSelect->sw_sharp_gain2strg_val[4],
        pSelect->sw_sharp_gain2strg_val[5], pSelect->sw_sharp_gain2strg_val[6],
        pSelect->sw_sharp_gain2strg_val[7], pSelect->sw_sharp_gain2strg_val[8],
        pSelect->sw_sharp_gain2strg_val[9], pSelect->sw_sharp_gain2strg_val[10],
        pSelect->sw_sharp_gain2strg_val[11], pSelect->sw_sharp_gain2strg_val[12],
        pSelect->sw_sharp_gain2strg_val[13]);

    LOGD_ASHARP(
        "%s:%d  sw_sharp_distance2strg_val[0~21]:%f %f %f %f %f %f %f %f %f %f %f %f %f "
        "%f %f %f %f %f %f %f %f %f\n",
        __FUNCTION__, __LINE__, pSelect->sw_sharp_distance2strg_val[0],
        pSelect->sw_sharp_distance2strg_val[1], pSelect->sw_sharp_distance2strg_val[2],
        pSelect->sw_sharp_distance2strg_val[3], pSelect->sw_sharp_distance2strg_val[4],
        pSelect->sw_sharp_distance2strg_val[5], pSelect->sw_sharp_distance2strg_val[6],
        pSelect->sw_sharp_distance2strg_val[7], pSelect->sw_sharp_distance2strg_val[8],
        pSelect->sw_sharp_distance2strg_val[9], pSelect->sw_sharp_distance2strg_val[10],
        pSelect->sw_sharp_distance2strg_val[11], pSelect->sw_sharp_distance2strg_val[12],
        pSelect->sw_sharp_distance2strg_val[13], pSelect->sw_sharp_distance2strg_val[14],
        pSelect->sw_sharp_distance2strg_val[15], pSelect->sw_sharp_distance2strg_val[16],
        pSelect->sw_sharp_distance2strg_val[17], pSelect->sw_sharp_distance2strg_val[18],
        pSelect->sw_sharp_distance2strg_val[19], pSelect->sw_sharp_distance2strg_val[20],
        pSelect->sw_sharp_distance2strg_val[21]);

    /* CLIP_NEG */
    LOGD_ASHARP("%s:%d  hw_sharp_luma2negClip_val[0~7]:%d  %d %d %d %d %d %d %d \n", __FUNCTION__,
                __LINE__, pSelect->hw_sharp_luma2negClip_val[0],
                pSelect->hw_sharp_luma2negClip_val[1], pSelect->hw_sharp_luma2negClip_val[2],
                pSelect->hw_sharp_luma2negClip_val[3], pSelect->hw_sharp_luma2negClip_val[4],
                pSelect->hw_sharp_luma2negClip_val[5], pSelect->hw_sharp_luma2negClip_val[6],
                pSelect->hw_sharp_luma2negClip_val[7]);

    // SHARP_TEXTURE (0x8c)
    LOGD_ASHARP(
        "%s:%d  hw_sharp_noise_maxLimit:%d sw_sharp_texReserve_strg:%f sw_sharp_noise_strg:%f\n",
        __FUNCTION__, __LINE__, pSelect->hw_sharp_noise_maxLimit, pSelect->sw_sharp_texReserve_strg,
        pSelect->sw_sharp_noise_strg);
    LOGD_ASHARP("%s:%d hw_sharp_noiseNorm_bit:%d hw_sharp_texWgt_mode:%d\n", __FUNCTION__, __LINE__,
                pSelect->hw_sharp_noiseNorm_bit, pSelect->hw_sharp_texWgt_mode);

    // SHARP_TEXTURE (0x90)
    LOGD_ASHARP(
        "%s:%d  hw_sharp_tex2wgt_val[0~16]:%d %d %d %d %d %d %d %d %d %d %d 0x%d %d "
        "%d %d %d %d\n",
        __FUNCTION__, __LINE__, pSelect->hw_sharp_tex2wgt_val[0], pSelect->hw_sharp_tex2wgt_val[1],
        pSelect->hw_sharp_tex2wgt_val[2], pSelect->hw_sharp_tex2wgt_val[3],
        pSelect->hw_sharp_tex2wgt_val[4], pSelect->hw_sharp_tex2wgt_val[5],
        pSelect->hw_sharp_tex2wgt_val[6], pSelect->hw_sharp_tex2wgt_val[7],
        pSelect->hw_sharp_tex2wgt_val[8], pSelect->hw_sharp_tex2wgt_val[9],
        pSelect->hw_sharp_tex2wgt_val[10], pSelect->hw_sharp_tex2wgt_val[11],
        pSelect->hw_sharp_tex2wgt_val[12], pSelect->hw_sharp_tex2wgt_val[13],
        pSelect->hw_sharp_tex2wgt_val[14], pSelect->hw_sharp_tex2wgt_val[15],
        pSelect->hw_sharp_tex2wgt_val[16]);

    LOGD_ASHARP(
        "%s:%d  hw_sharp_detail2strg_val[0~16]:%d %d %d %d %d %d %d %d %d %d %d 0x%d %d "
        "%d %d %d %d\n",
        __FUNCTION__, __LINE__, pSelect->hw_sharp_detail2strg_val[0],
        pSelect->hw_sharp_detail2strg_val[1], pSelect->hw_sharp_detail2strg_val[2],
        pSelect->hw_sharp_detail2strg_val[3], pSelect->hw_sharp_detail2strg_val[4],
        pSelect->hw_sharp_detail2strg_val[5], pSelect->hw_sharp_detail2strg_val[6],
        pSelect->hw_sharp_detail2strg_val[7], pSelect->hw_sharp_detail2strg_val[8],
        pSelect->hw_sharp_detail2strg_val[9], pSelect->hw_sharp_detail2strg_val[10],
        pSelect->hw_sharp_detail2strg_val[11], pSelect->hw_sharp_detail2strg_val[12],
        pSelect->hw_sharp_detail2strg_val[13], pSelect->hw_sharp_detail2strg_val[14],
        pSelect->hw_sharp_detail2strg_val[15], pSelect->hw_sharp_detail2strg_val[16]);

    return res;
}

Asharp_result_V34_t sharp_select_params_by_ISO_V34(void* pParams_v, void* pSelect_v,
                                                   Asharp_ExpInfo_V34_t* pExpInfo) {
    Asharp_result_V34_t res               = ASHARP_V34_RET_SUCCESS;
    RK_SHARP_Params_V34_t* pParams        = (RK_SHARP_Params_V34_t*)pParams_v;
    RK_SHARP_Params_V34_Select_t* pSelect = (RK_SHARP_Params_V34_Select_t*)pSelect_v;

    int i;
    int iso     = 50;
    int iso_div = 50;
    float ratio = 0.0f;
    int iso_low = iso, iso_high = iso;
    int gain_high = 0, gain_low = 0, gain_int = iso;
    int max_iso_step = RK_SHARP_V34_MAX_ISO_NUM;
    int sum_coeff, offset;

    LOG1_ASHARP("%s(%d): enter\n", __FUNCTION__, __LINE__);

    if (pParams == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP_V34_RET_NULL_POINTER;
    }

    if (pSelect == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP_V34_RET_NULL_POINTER;
    }

    if (pExpInfo == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP_V34_RET_NULL_POINTER;
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
    if (iso - iso_low <= iso_high - iso)
        gain_int = gain_low;
    else
        gain_int = gain_high;

    pSelect->enable = pParams->enable;
    pSelect->hw_sharp_centerPosition_mode = pParams->hw_sharp_centerPosition_mode;
    pSelect->sw_sharp_filtCoeff_mode      = pParams->sw_sharp_filtCoeff_mode;
    pSelect->hw_sharp_texWgt_mode         = pParams->hw_sharp_texWgt_mode;
    for (int i = 0; i < RK_SHARP_V34_LUMA_POINT_NUM; i++) {
        pSelect->hw_sharp_luma2Table_idx[i] =
            pParams->sharpParamsISO[gain_low].hw_sharp_luma2Table_idx[i];
        pSelect->sw_sharp_luma2vsigma_val[i] =
            INTERP_V4(pParams->sharpParamsISO[gain_low].sw_sharp_luma2vsigma_val[i],
                      pParams->sharpParamsISO[gain_high].sw_sharp_luma2vsigma_val[i], ratio);
    }
    pSelect->hw_sharp_localGain_bypass =
        pParams->sharpParamsISO[gain_int].hw_sharp_localGain_bypass;

    pSelect->sw_sharp_preBifilt_scale =
        INTERP_V4(pParams->sharpParamsISO[gain_low].sw_sharp_preBifilt_scale,
                  pParams->sharpParamsISO[gain_high].sw_sharp_preBifilt_scale, ratio);
    pSelect->sw_sharp_preBifilt_offset =
        INTERP_V4(pParams->sharpParamsISO[gain_low].sw_sharp_preBifilt_offset,
                  pParams->sharpParamsISO[gain_high].sw_sharp_preBifilt_offset, ratio);
    pSelect->sw_sharp_preBifilt_alpha =
        INTERP_V4(pParams->sharpParamsISO[gain_low].sw_sharp_preBifilt_alpha,
                  pParams->sharpParamsISO[gain_high].sw_sharp_preBifilt_alpha, ratio);
    pSelect->sw_sharp_preBifilt_rsigma =
        INTERP_V4(pParams->sharpParamsISO[gain_low].sw_sharp_preBifilt_rsigma,
                  pParams->sharpParamsISO[gain_high].sw_sharp_preBifilt_rsigma, ratio);
    for (int i = 0; i < 3; i++) {
        pSelect->sw_sharp_preBifilt_coeff[i] =
            INTERP_V4(pParams->sharpParamsISO[gain_low].sw_sharp_preBifilt_coeff[i],
                      pParams->sharpParamsISO[gain_high].sw_sharp_preBifilt_coeff[i], ratio);
    }

    pSelect->sw_sharp_guideFilt_alpha =
        INTERP_V4(pParams->sharpParamsISO[gain_low].sw_sharp_guideFilt_alpha,
                  pParams->sharpParamsISO[gain_high].sw_sharp_guideFilt_alpha, ratio);
    pSelect->sw_sharp_imgLpf0_strg =
        INTERP_V4(pParams->sharpParamsISO[gain_low].sw_sharp_imgLpf0_strg,
                  pParams->sharpParamsISO[gain_high].sw_sharp_imgLpf0_strg, ratio);
    pSelect->sw_sharp_imgLpf1_strg =
        INTERP_V4(pParams->sharpParamsISO[gain_low].sw_sharp_imgLpf1_strg,
                  pParams->sharpParamsISO[gain_high].sw_sharp_imgLpf1_strg, ratio);
    pSelect->hw_sharp_baseImg_sel = pParams->sharpParamsISO[gain_int].hw_sharp_baseImg_sel;
    pSelect->hw_sharp_clipIdx_sel = pParams->sharpParamsISO[gain_int].hw_sharp_clipIdx_sel;
    for (int i = 0; i < 8; i++) {
        pSelect->hw_sharp_luma2posClip_val[i] =
            INTERP_V4(pParams->sharpParamsISO[gain_low].hw_sharp_luma2posClip_val[i],
                      pParams->sharpParamsISO[gain_high].hw_sharp_luma2posClip_val[i], ratio);
        pSelect->hw_sharp_luma2negClip_val[i] =
            INTERP_V4(pParams->sharpParamsISO[gain_low].hw_sharp_luma2negClip_val[i],
                      pParams->sharpParamsISO[gain_high].hw_sharp_luma2negClip_val[i], ratio);
    }

    pSelect->sw_sharp_detailBifilt_scale =
        INTERP_V4(pParams->sharpParamsISO[gain_low].sw_sharp_detailBifilt_scale,
                  pParams->sharpParamsISO[gain_high].sw_sharp_detailBifilt_scale, ratio);
    pSelect->sw_sharp_detailBifilt_offset =
        INTERP_V4(pParams->sharpParamsISO[gain_low].sw_sharp_detailBifilt_offset,
                  pParams->sharpParamsISO[gain_high].sw_sharp_detailBifilt_offset, ratio);
    pSelect->sw_sharp_detailBifilt_alpha =
        INTERP_V4(pParams->sharpParamsISO[gain_low].sw_sharp_detailBifilt_alpha,
                  pParams->sharpParamsISO[gain_high].sw_sharp_detailBifilt_alpha, ratio);
    pSelect->sw_sharp_detailBifilt_rsigma =
        INTERP_V4(pParams->sharpParamsISO[gain_low].sw_sharp_detailBifilt_rsigma,
                  pParams->sharpParamsISO[gain_high].sw_sharp_detailBifilt_rsigma, ratio);
    for (int i = 0; i < 3; i++) {
        pSelect->sw_sharp_detailBifilt_coeff[i] =
            INTERP_V4(pParams->sharpParamsISO[gain_low].sw_sharp_detailBifilt_coeff[i],
                      pParams->sharpParamsISO[gain_high].sw_sharp_detailBifilt_coeff[i], ratio);
    }

    for (int i = 0; i < 8; i++) {
        pSelect->hw_sharp_luma2strg_val[i] =
            INTERP_V4(pParams->sharpParamsISO[gain_low].hw_sharp_luma2strg_val[i],
                      pParams->sharpParamsISO[gain_high].hw_sharp_luma2strg_val[i], ratio);
    }

    pSelect->sw_sharp_imgLpf0_radius = pParams->sharpParamsISO[gain_int].sw_sharp_imgLpf0_radius;
    pSelect->sw_sharp_imgLpf0_rsigma =
        INTERP_V4(pParams->sharpParamsISO[gain_low].sw_sharp_imgLpf0_rsigma,
                  pParams->sharpParamsISO[gain_high].sw_sharp_imgLpf0_rsigma, ratio);
    pSelect->sw_sharp_imgLpf1_radius = pParams->sharpParamsISO[gain_int].sw_sharp_imgLpf1_radius;
    pSelect->sw_sharp_imgLpf1_rsigma =
        INTERP_V4(pParams->sharpParamsISO[gain_low].sw_sharp_imgLpf1_rsigma,
                  pParams->sharpParamsISO[gain_high].sw_sharp_imgLpf1_rsigma, ratio);
    for (int i = 0; i < 6; i++) {
        pSelect->sw_sharp_imgLpf0_coeff[i] =
            INTERP_V4(pParams->sharpParamsISO[gain_low].sw_sharp_imgLpf0_coeff[i],
                      pParams->sharpParamsISO[gain_high].sw_sharp_imgLpf0_coeff[i], ratio);
        pSelect->sw_sharp_imgLpf1_coeff[i] =
            INTERP_V4(pParams->sharpParamsISO[gain_low].sw_sharp_imgLpf1_coeff[i],
                      pParams->sharpParamsISO[gain_high].sw_sharp_imgLpf1_coeff[i], ratio);
    }

    pSelect->sw_sharp_global_gain =
        INTERP_V4(pParams->sharpParamsISO[gain_low].sw_sharp_global_gain,
                  pParams->sharpParamsISO[gain_high].sw_sharp_global_gain, ratio);
    pSelect->sw_sharp_gainMerge_alpha =
        INTERP_V4(pParams->sharpParamsISO[gain_low].sw_sharp_gainMerge_alpha,
                  pParams->sharpParamsISO[gain_high].sw_sharp_gainMerge_alpha, ratio);
    pSelect->sw_sharp_localGain_scale =
        INTERP_V4(pParams->sharpParamsISO[gain_low].sw_sharp_localGain_scale,
                  pParams->sharpParamsISO[gain_high].sw_sharp_localGain_scale, ratio);

    for (int i = 0; i < 14; i++) {
        pSelect->sw_sharp_gain2strg_val[i] =
            INTERP_V4(pParams->sharpParamsISO[gain_low].sw_sharp_gain2strg_val[i],
                      pParams->sharpParamsISO[gain_high].sw_sharp_gain2strg_val[i], ratio);
    }

    for (int i = 0; i < 22; i++) {
        pSelect->sw_sharp_distance2strg_val[i] =
            INTERP_V4(pParams->sharpParamsISO[gain_low].sw_sharp_distance2strg_val[i],
                      pParams->sharpParamsISO[gain_high].sw_sharp_distance2strg_val[i], ratio);
    }

    pSelect->hw_sharp_noiseFilt_sel = pParams->sharpParamsISO[gain_int].hw_sharp_noiseFilt_sel;
    pSelect->hw_sharp_noiseNorm_bit = pParams->sharpParamsISO[gain_int].hw_sharp_noiseNorm_bit;
    pSelect->hw_sharp_noise_maxLimit =
        INTERP_V4(pParams->sharpParamsISO[gain_low].hw_sharp_noise_maxLimit,
                  pParams->sharpParamsISO[gain_high].hw_sharp_noise_maxLimit, ratio);
    pSelect->hw_sharp_noiseClip_sel = pParams->sharpParamsISO[gain_int].hw_sharp_noiseClip_sel;
    pSelect->sw_sharp_noise_strg =
        INTERP_V4(pParams->sharpParamsISO[gain_low].sw_sharp_noise_strg,
                  pParams->sharpParamsISO[gain_high].sw_sharp_noise_strg, ratio);
    pSelect->sw_sharp_texReserve_strg =
        INTERP_V4(pParams->sharpParamsISO[gain_low].sw_sharp_texReserve_strg,
                  pParams->sharpParamsISO[gain_high].sw_sharp_texReserve_strg, ratio);
    pSelect->hw_sharp_tex2wgt_en = pParams->sharpParamsISO[gain_int].hw_sharp_tex2wgt_en;

    for (int i = 0; i < 17; i++) {
        pSelect->hw_sharp_tex2wgt_val[i] =
            INTERP_V4(pParams->sharpParamsISO[gain_low].hw_sharp_tex2wgt_val[i],
                      pParams->sharpParamsISO[gain_high].hw_sharp_tex2wgt_val[i], ratio);
    }

    for (int i = 0; i < 17; i++) {
        pSelect->hw_sharp_detail2strg_val[i] =
            INTERP_V4(pParams->sharpParamsISO[gain_low].hw_sharp_detail2strg_val[i],
                      pParams->sharpParamsISO[gain_high].hw_sharp_detail2strg_val[i], ratio);
    }

#if 1
    sharp_select_printf_V34(pSelect);
#endif

    LOG1_ASHARP("%s(%d): exit\n", __FUNCTION__, __LINE__);
    return res;
}

Asharp_result_V34_t sharp_fix_transfer_V34(void* pSelect_v, RK_SHARP_Fix_V34_t* pFix,
                                           rk_aiq_sharp_strength_v34_t* pStrength,
                                           Asharp_ExpInfo_V34_t* pExpInfo) {
    int sum_coeff, offset;
    int pbf_sigma_shift     = 0;
    int bf_sigma_shift      = 0;
    Asharp_result_V34_t res = ASHARP_V34_RET_SUCCESS;
    int tmp                 = 0;

    RK_SHARP_Params_V34_Select_t* pSelect = (RK_SHARP_Params_V34_Select_t*)pSelect_v;

    LOG1_ASHARP("%s(%d): enter\n", __FUNCTION__, __LINE__);

    if (pSelect == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP_V34_RET_NULL_POINTER;
    }

    if (pFix == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP_V34_RET_NULL_POINTER;
    }

    if (pStrength == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP_V34_RET_NULL_POINTER;
    }

    if (pExpInfo == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP_V34_RET_NULL_POINTER;
    }

    float fPercent = 1.0f;

    if (pStrength->strength_enable) {
        fPercent = pStrength->percent;
    }
    if (fPercent <= 0.0) {
        fPercent = 0.000001;
    }

    LOGD_ASHARP("strength_enable:%d fPercent: %f \n", pStrength->strength_enable, fPercent);

    LOGD_ASHARP("%s:%d strength:%f raw:width:%d height:%d\n", __FUNCTION__, __LINE__, fPercent,
                pExpInfo->rawHeight, pExpInfo->rawWidth);

    int rows = pExpInfo->rawHeight;  // raw height
    int cols = pExpInfo->rawWidth;   // raw  width

    // SHARP_EN (0x0000)
    pFix->bypass            = !pSelect->enable;
    pFix->center_mode       = pSelect->hw_sharp_centerPosition_mode;
    pFix->local_gain_bypass = pSelect->hw_sharp_localGain_bypass;
    if (cols > 3072 && rows > 1728) {
        pFix->radius_step_mode = 1;
    } else {
        pFix->radius_step_mode = 0;
    }
    pFix->noise_clip_mode = pSelect->hw_sharp_noiseClip_sel;
    pFix->clipldx_sel     = pSelect->hw_sharp_clipIdx_sel;
    pFix->baselmg_sel     = pSelect->hw_sharp_baseImg_sel;
    pFix->noise_filt_sel  = pSelect->hw_sharp_noiseFilt_sel;
    pFix->tex2wgt_en      = pSelect->hw_sharp_tex2wgt_en;

    // SHARP_RATIO  (0x0004)
    tmp                       = (int)ROUND_F(pSelect->sw_sharp_preBifilt_alpha / fPercent *
                       (1 << RK_SHARP_V34_BF_RATIO_FIX_BITS));
    pFix->pre_bifilt_alpha    = CLIP(tmp, 0, 0x80);
    tmp                       = (int)ROUND_F(pSelect->sw_sharp_guideFilt_alpha / fPercent *
                       (1 << RK_SHARP_V34_GAUS_RATIO_FIX_BITS));
    pFix->guide_filt_alpha    = CLIP(tmp, 0, 0x80);
    tmp                       = (int)ROUND_F(pSelect->sw_sharp_detailBifilt_alpha / fPercent *
                       (1 << RK_SHARP_V34_BF_RATIO_FIX_BITS));
    pFix->detail_bifilt_alpha = CLIP(tmp, 0, 0x80);
    tmp = (int)ROUND_F((pSelect->sw_sharp_imgLpf0_strg + pSelect->sw_sharp_imgLpf1_strg) *
                       fPercent * (1 << RK_SHARP_V34_SHARP_STRG_FIX_BITS));
    pFix->global_sharp_strg = CLIP(tmp, 0, 127);

    // SHARP_LUMA_DX (0x0008)
    for (int i = 0; i < RK_SHARP_V34_LUMA_POINT_NUM - 1; i++) {
        tmp                     = (int16_t)LOG2(pSelect->hw_sharp_luma2Table_idx[i + 1] -
                            pSelect->hw_sharp_luma2Table_idx[i]);
        pFix->luma2table_idx[i] = CLIP(tmp, 0, 15);
    }

    // SHARP_PBF_SIGMA_INV_0 (0x000c - 0x0014)
    // pre bf sigma inv
    int sigma_deci_bits = 9;
    int sigma_inte_bits = 1;
    int max_val         = 0;
    int min_val         = 65536;
    int shf_bits        = 0;
    short sigma_bits[3];
    for (int i = 0; i < RK_SHARP_V34_LUMA_POINT_NUM; i++) {
        int cur_sigma =
            FLOOR((pSelect->sw_sharp_luma2vsigma_val[i] * pSelect->sw_sharp_preBifilt_scale +
                   pSelect->sw_sharp_preBifilt_offset) /
                  fPercent);
        if (max_val < cur_sigma) max_val = cur_sigma;
        if (min_val > cur_sigma) min_val = cur_sigma;
    }
    sigma_bits[0]   = FLOOR(log((float)min_val) / log((float)2));
    sigma_bits[1]   = MAX(sigma_inte_bits - sigma_bits[0], 0);
    sigma_bits[2]   = sigma_deci_bits + sigma_bits[0];
    pbf_sigma_shift = sigma_bits[2] - 5;
    for (int i = 0; i < RK_SHARP_V34_LUMA_POINT_NUM; i++) {
        tmp = (int16_t)ROUND_F(
            1.0f /
            (pSelect->sw_sharp_luma2vsigma_val[i] * pSelect->sw_sharp_preBifilt_scale +
             pSelect->sw_sharp_preBifilt_offset) *
            fPercent * (1 << sigma_bits[2]));
        pFix->pbf_sigma_inv[i] = CLIP(tmp, 0, 4095);
    }

    // SHARP_BF_SIGMA_INV_0 (0x0018 -  0x0020)
    // bf sigma inv
    sigma_deci_bits = 9;
    sigma_inte_bits = 1;
    max_val         = 0;
    min_val         = 65536;
    shf_bits        = 0;
    for (int i = 0; i < RK_SHARP_V34_LUMA_POINT_NUM; i++) {
        int cur_sigma =
            FLOOR((pSelect->sw_sharp_luma2vsigma_val[i] * pSelect->sw_sharp_detailBifilt_scale +
                   pSelect->sw_sharp_detailBifilt_offset) /
                  fPercent);
        if (max_val < cur_sigma) max_val = cur_sigma;
        if (min_val > cur_sigma) min_val = cur_sigma;
    }
    sigma_bits[0]  = FLOOR(log((float)min_val) / log((float)2.0f));
    sigma_bits[1]  = MAX(sigma_inte_bits - sigma_bits[0], 0);
    sigma_bits[2]  = sigma_deci_bits + sigma_bits[0];
    bf_sigma_shift = sigma_bits[2] - 5;
    for (int i = 0; i < RK_SHARP_V34_LUMA_POINT_NUM; i++) {
        tmp = (int16_t)ROUND_F(
            1.0f /
            (pSelect->sw_sharp_luma2vsigma_val[i] * pSelect->sw_sharp_detailBifilt_scale +
             pSelect->sw_sharp_detailBifilt_offset) *
            fPercent * (1 << sigma_bits[2]));
        pFix->bf_sigma_inv[i] = CLIP(tmp, 0, 4095);
    }

    // SHARP_SIGMA_SHIFT (0x00024)
    pFix->pbf_sigma_shift = CLIP(pbf_sigma_shift, 0, 15);
    pFix->bf_sigma_shift  = CLIP(bf_sigma_shift, 0, 15);

    // EHF_TH (0x0028 -  0x0030)
    for (int i = 0; i < 8; i++)
        pFix->luma2strg_val[i] = CLIP(pSelect->hw_sharp_luma2strg_val[i], 0, 1023);

    // SHARP_SHARP_CLIP_HF (0x0034 -  0x003c)
    for (int i = 0; i < RK_SHARP_V34_LUMA_POINT_NUM; i++) {
        tmp                       = (int)(pSelect->hw_sharp_luma2posClip_val[i] * fPercent);
        pFix->luma2posclip_val[i] = CLIP(tmp, 0, 1023);
    }

    // SHARP_SHARP_PBF_COEF (0x00040)
    // filter coeff
    // bf coeff
    // rk_sharp_V34_pbfCoeff : [4], [1], [0]
    float pre_bila_filter[3];
    if (pSelect->sw_sharp_filtCoeff_mode) {
        float dis_table_3x3[3] = {0.0, 1.0, 2.0};
        double e               = 2.71828182845905;
        float sigma            = pSelect->sw_sharp_preBifilt_rsigma;
        float sum_gauss_coeff  = 0.0;
        for (int i = 0; i < 3; i++) {
            float tmp          = pow(e, -dis_table_3x3[i] / 2.0 / sigma / sigma);
            pre_bila_filter[i] = tmp;
        }
        sum_gauss_coeff = pre_bila_filter[0] + 4 * pre_bila_filter[1] + 4 * pre_bila_filter[2];
        for (int i = 0; i < 3; i++) {
            pre_bila_filter[i] = pre_bila_filter[i] / sum_gauss_coeff;
            LOGD_ASHARP("sw_sharp_filtCoeff_mode:%d pre_bila_filter[%d]:%f\n",
                        pSelect->sw_sharp_filtCoeff_mode, i, pre_bila_filter[i]);
        }
    } else {
        for (int i = 0; i < 3; i++) {
            pre_bila_filter[i] = pSelect->sw_sharp_preBifilt_coeff[i];
            LOGD_ASHARP("sw_sharp_filtCoeff_mode:%d pre_bila_filter[%d]:%f\n",
                        pSelect->sw_sharp_filtCoeff_mode, i, pre_bila_filter[i]);
        }
    }
    tmp             = (int)ROUND_F(pre_bila_filter[0] * (1 << RK_SHARP_V34_PBFCOEFF_FIX_BITS));
    pFix->pbf_coef0 = CLIP(tmp, 0, 127);
    tmp             = (int)ROUND_F(pre_bila_filter[1] * (1 << RK_SHARP_V34_PBFCOEFF_FIX_BITS));
    pFix->pbf_coef1 = CLIP(tmp, 0, 127);
    tmp             = (int)ROUND_F(pre_bila_filter[2] * (1 << RK_SHARP_V34_PBFCOEFF_FIX_BITS));
    pFix->pbf_coef2 = CLIP(tmp, 0, 127);

    sum_coeff       = pFix->pbf_coef0 + 4 * pFix->pbf_coef1 + 4 * pFix->pbf_coef2;
    offset          = (1 << RK_SHARP_V34_PBFCOEFF_FIX_BITS) - sum_coeff;
    tmp             = (int)(pFix->pbf_coef0 + offset);
    pFix->pbf_coef0 = CLIP(tmp, 0, 127);

    // SHARP_SHARP_GAUS_COEF (0x00048)
    // rk_sharp_V34_rfCoeff :  [4], [1], [0]
    float bila_filter[3];
    if (pSelect->sw_sharp_filtCoeff_mode) {
        float dis_table_3x3[3] = {0.0, 1.0, 2.0};
        double e               = 2.71828182845905;
        float sigma            = pSelect->sw_sharp_detailBifilt_rsigma;
        float sum_gauss_coeff  = 0.0;
        for (int i = 0; i < 3; i++) {
            float tmp      = pow(e, -dis_table_3x3[i] / 2.0 / sigma / sigma);
            bila_filter[i] = tmp;
        }
        sum_gauss_coeff = bila_filter[0] + 4 * bila_filter[1] + 4 * bila_filter[2];
        for (int i = 0; i < 3; i++) {
            bila_filter[i] = bila_filter[i] / sum_gauss_coeff;
            LOGD_ASHARP("sw_sharp_filtCoeff_mode:%d bila_filter[%d]:%f\n",
                        pSelect->sw_sharp_filtCoeff_mode, i, bila_filter[i]);
        }
    } else {
        for (int i = 0; i < 3; i++) {
            bila_filter[i] = pSelect->sw_sharp_detailBifilt_coeff[i];
            LOGD_ASHARP("sw_sharp_filtCoeff_mode:%d bila_filter[%d]:%f\n",
                        pSelect->sw_sharp_filtCoeff_mode, i, bila_filter[i]);
        }
    }
    tmp            = (int)ROUND_F(bila_filter[0] * (1 << RK_SHARP_V34_RFCOEFF_FIX_BITS));
    pFix->bf_coef0 = CLIP(tmp, 0, 127);
    tmp            = (int)ROUND_F(bila_filter[1] * (1 << RK_SHARP_V34_RFCOEFF_FIX_BITS));
    pFix->bf_coef1 = CLIP(tmp, 0, 127);
    tmp            = (int)ROUND_F(bila_filter[2] * (1 << RK_SHARP_V34_RFCOEFF_FIX_BITS));
    pFix->bf_coef2 = CLIP(tmp, 0, 127);

    sum_coeff      = pFix->bf_coef0 + 4 * pFix->bf_coef1 + 4 * pFix->bf_coef2;
    offset         = (1 << RK_SHARP_V34_PBFCOEFF_FIX_BITS) - sum_coeff;
    tmp            = (int)(pFix->bf_coef0 + offset);
    pFix->bf_coef0 = CLIP(tmp, 0, 127);

    // SHARP_SHARP_BF_COEF (0x00044)
    // rk_sharp_V34_rfCoeff : [4], [1], [0]
    float kernel0_ratio = pSelect->sw_sharp_imgLpf0_strg /
                          (pSelect->sw_sharp_imgLpf0_strg + pSelect->sw_sharp_imgLpf1_strg);
    float kernel1_ratio = pSelect->sw_sharp_imgLpf1_strg /
                          (pSelect->sw_sharp_imgLpf0_strg + pSelect->sw_sharp_imgLpf1_strg);
    if (pSelect->sw_sharp_filtCoeff_mode) {
        float dis_table_5x5[6] = {0.0f, 1.0f, 2.0f, 4.0f, 5.0f, 8.0f};
        float dis_table_3x3[6] = {0.0f, 1.0f, 2.0f, 1000.0f, 1000.0f, 1000.0f};
        float gaus_table[6];
        float gaus_table1[6];
        float gaus_table_combine[6];

        float sigma  = pSelect->sw_sharp_imgLpf0_rsigma;
        float sigma1 = pSelect->sw_sharp_imgLpf1_rsigma;
        double e     = 2.71828182845905;
        if (2 == pSelect->sw_sharp_imgLpf0_radius) {
            for (int k = 0; k < 6; k++) {
                float tmp0    = pow(e, -dis_table_5x5[k] / 2.0 / sigma / sigma);
                gaus_table[k] = tmp0;
            }
        } else {
            for (int k = 0; k < 6; k++) {
                float tmp0    = pow(e, -dis_table_3x3[k] / 2.0 / sigma / sigma);
                gaus_table[k] = tmp0;
            }
        }
        if (2 == pSelect->sw_sharp_imgLpf1_radius) {
            for (int k = 0; k < 6; k++) {
                float tmp1     = pow(e, -dis_table_5x5[k] / 2.0 / sigma1 / sigma1);
                gaus_table1[k] = tmp1;
            }
        } else {
            for (int k = 0; k < 6; k++) {
                float tmp1     = pow(e, -dis_table_3x3[k] / 2.0 / sigma1 / sigma1);
                gaus_table1[k] = tmp1;
            }
        }
        float sumTable = 0;
        sumTable       = gaus_table[0] + 4 * gaus_table[1] + 4 * gaus_table[2] + 4 * gaus_table[3] +
                   8 * gaus_table[4] + 4 * gaus_table[5];

        float sumTable1 = 0;
        sumTable1 = gaus_table1[0] + 4 * gaus_table1[1] + 4 * gaus_table1[2] + 4 * gaus_table1[3] +
                    8 * gaus_table1[4] + 4 * gaus_table1[5];
        for (int k = 0; k < 6; k++) {
            // gaus_table[k] = gaus_table[k] / sumTable;
            gaus_table_combine[k] = kernel0_ratio * gaus_table[k] / sumTable +
                                    kernel1_ratio * gaus_table1[k] / sumTable1;
            pFix->img_lpf_coeff[k] =
                ROUND_F(gaus_table_combine[k] * (1 << RK_SHARP_V34_HBFCOEFF_FIX_BITS));
        }
    } else {
        for (int k = 0; k < 6; k++) {
            float range_coeff0        = pSelect->sw_sharp_imgLpf0_coeff[k];
            float range_coeff1        = pSelect->sw_sharp_imgLpf1_coeff[k];
            float range_coeff_combine = kernel0_ratio * range_coeff0 + kernel1_ratio * range_coeff1;
            pFix->img_lpf_coeff[k] =
                ROUND_F(range_coeff_combine * (1 << RK_SHARP_V34_HBFCOEFF_FIX_BITS));
        }
    }
    sum_coeff = pFix->img_lpf_coeff[0] + 4 * pFix->img_lpf_coeff[1] + 4 * pFix->img_lpf_coeff[2] +
                4 * pFix->img_lpf_coeff[3] + 8 * pFix->img_lpf_coeff[4] +
                4 * pFix->img_lpf_coeff[5];
    offset                 = (1 << RK_SHARP_V34_RFCOEFF_FIX_BITS) - sum_coeff;
    tmp                    = (int)(pFix->img_lpf_coeff[0] + offset);
    pFix->img_lpf_coeff[0] = CLIP(tmp, 0, 127);

    // gain
    tmp               = pSelect->sw_sharp_global_gain * (1 << RK_SHARP_V34_GLOBAL_GAIN_FIX_BITS);
    pFix->global_gain = CLIP(tmp, 0, 1023);
    tmp = pSelect->sw_sharp_gainMerge_alpha * (1 << RK_SHARP_V34_GLOBAL_GAIN_ALPHA_FIX_BITS);
    pFix->gain_merge_alpha = CLIP(tmp, 0, 8);
    tmp = pSelect->sw_sharp_localGain_scale * (1 << RK_SHARP_V34_LOCAL_GAIN_SACLE_FIX_BITS);
    pFix->local_gain_scale = CLIP(tmp, 0, 128);

    // gain adjust strength
    for (int i = 0; i < RK_SHARP_V34_SHARP_ADJ_GAIN_TABLE_LEN; i++) {
        tmp = ROUND_F(pSelect->sw_sharp_gain2strg_val[i] * (1 << RK_SHARP_V34_ADJ_GAIN_FIX_BITS));
        pFix->gain2strg_val[i] = CLIP(tmp, 0, 16384);
    }

    // CENTER
    tmp            = cols / 2;
    pFix->center_x = CLIP(tmp, 0, 8191);
    tmp            = rows / 2;
    pFix->center_y = CLIP(tmp, 0, 8191);

    // gain dis strength
    for (int i = 0; i < RK_SHARP_V34_STRENGTH_TABLE_LEN; i++) {
        tmp                        = ROUND_F(pSelect->sw_sharp_distance2strg_val[i] *
                      (1 << RK_SHARP_V34_STRENGTH_TABLE_FIX_BITS));
        pFix->distance2strg_val[i] = CLIP(tmp, 0, 128);
    }

    // SHARP_SHARP_CLIP_NEG (0x008c -  0x0094)
    for (int i = 0; i < RK_SHARP_V34_LUMA_POINT_NUM; i++) {
        tmp                        = (int)(pSelect->hw_sharp_luma2negClip_val[i] * fPercent);
        pFix->luma2neg_clip_val[i] = CLIP(tmp, 0, 1023);
    }

    // TEXTURE0
    tmp                     = ROUND_F(pSelect->hw_sharp_noise_maxLimit);
    pFix->noise_max_limit   = CLIP(tmp, 0, 1023);

    // TEXTURE1
    pFix->noise_norm_bit = pSelect->hw_sharp_noiseNorm_bit;
    pFix->tex_wgt_mode   = pSelect->hw_sharp_texWgt_mode;

    float tex_coef        = pSelect->sw_sharp_texReserve_strg;
    int tex_reserve_level = 0;
    if (tex_coef >= 16 && tex_coef <= 31)
        tex_reserve_level = 0;
    else if (tex_coef >= 8 && tex_coef < 16)
        tex_reserve_level = 1;
    else if (tex_coef >= 4 && tex_coef < 8)
        tex_reserve_level = 2;
    else if (tex_coef >= 2 && tex_coef < 4)
        tex_reserve_level = 3;
    else if (tex_coef >= 1 && tex_coef < 2)
        tex_reserve_level = 4;
    else if (tex_coef >= 0.5 && tex_coef < 1)
        tex_reserve_level = 5;
    else if (tex_coef >= 0.25 && tex_coef < 0.5)
        tex_reserve_level = 6;
    else if (tex_coef >= 0.125 && tex_coef < 0.25)
        tex_reserve_level = 7;
    else if (tex_coef >= 0.0625 && tex_coef < 0.125)
        tex_reserve_level = 8;
    else if (tex_coef >= 0.03125 && tex_coef < 0.0625)
        tex_reserve_level = 9;
    else if (tex_coef < 0.03125)
        tex_reserve_level = 10;

    pFix->tex_reserve_level = tex_reserve_level;
    tmp = ROUND_F(tex_coef * (1 << (tex_reserve_level + RK_SHARP_V34_TEX_FIX_BITS)));
    pFix->tex_wet_scale = CLIP(tmp, 0, 31 * (1 << RK_SHARP_V34_TEX_FIX_BITS));

    // TEXTURE_LUT
    for (int i = 0; i < 17; i++) pFix->tex2wgt_val[i] = pSelect->hw_sharp_tex2wgt_val[i];

    // TEXTURE2
    tmp = ROUND_F(pSelect->sw_sharp_noise_strg * (1 << RK_SHARP_V34_ADJ_GAIN_FIX_BITS));
    pFix->noise_strg = CLIP(tmp, 0, 16383);

    for (int i = 0; i < 17; i++) pFix->detail2strg_val[i] = pSelect->hw_sharp_detail2strg_val[i];

#if 0
    sharp_fix_printf_V34(pFix);
#endif

    LOG1_ASHARP("%s(%d): exit\n", __FUNCTION__, __LINE__);
    return res;
}

Asharp_result_V34_t sharp_fix_printf_V34(RK_SHARP_Fix_V34_t* pFix) {
    int i                   = 0;
    Asharp_result_V34_t res = ASHARP_V34_RET_SUCCESS;

    LOG1_ASHARP("%s:(%d) enter \n", __FUNCTION__, __LINE__);

    if (pFix == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP_V34_RET_NULL_POINTER;
    }

    // SHARP_SHARP_EN (0X0000)
    LOGD_ASHARP("(0x0000) sahrp_Center_Mode:0x%x sharp_bypass:0x%x\n", pFix->center_mode,
                pFix->bypass);

    // SHARP_SHARP_RATIO (0x0004)
    LOGD_ASHARP(
        "(0x0004) global_sharp_strg:0x%x detail_bifilt_alpha:0x%x guide_filt_alpha:0x%x "
        "pre_bifilt_alpha:0x%x \n",
        pFix->global_sharp_strg, pFix->detail_bifilt_alpha, pFix->guide_filt_alpha,
        pFix->pre_bifilt_alpha);

    // SHARP_SHARP_LUMA_DX (0x0008)
    for (int i = 0; i < 7; i++) {
        LOGD_ASHARP("(0x0008) sharp_luma2table_idx[%d]:0x%x \n", i, pFix->luma2table_idx[i]);
    }

    // SHARP_SHARP_PBF_SIGMA_INV_0 (0x000c - 0x0014)
    for (int i = 0; i < 8; i++) {
        LOGD_ASHARP("(0x000c - 0x0014) sharp_pbf_sigma_inv[%d]:0x%x \n", i, pFix->pbf_sigma_inv[i]);
    }

    // SHARP_SHARP_BF_SIGMA_INV_0 (0x0018 -  0x0020)
    for (int i = 0; i < 8; i++) {
        LOGD_ASHARP("(0x0018 - 0x0020) sharp_bf_sigma_inv[%d]:0x%x \n", i, pFix->bf_sigma_inv[i]);
    }

    // SHARP_SHARP_SIGMA_SHIFT (0x00024)
    LOGD_ASHARP("(0x00024) sharp_bf_sigma_shift:0x%x sharp_pbf_sigma_shift:0x%x \n",
                pFix->bf_sigma_shift, pFix->pbf_sigma_shift);

    // SHARP_SHARP_CLIP_HF_0 (0x0034 -  0x003c)
    for (int i = 0; i < 8; i++) {
        LOGD_ASHARP("(0x0034 - 0x003c) sharp_luma2posclip_val[%d]:0x%x \n", i,
                    pFix->luma2posclip_val[i]);
    }

    // SHARP_SHARP_PBF_COEF (0x00040)
    for (int i = 0; i < 3; i++) {
        LOGD_ASHARP("(0x00040) sharp_pbf_coef[0~2]:0x%x 0x%x 0x%x\n", pFix->pbf_coef0,
                    pFix->pbf_coef1, pFix->pbf_coef2);
    }

    // SHARP_SHARP_BF_COEF (0x00044)
    LOGD_ASHARP("(0x00044) sharp_bf_coef[0~1]:0x%x 0x%x 0x%x\n", pFix->bf_coef0, pFix->bf_coef1,
                pFix->bf_coef2);

    // SHARP_SHARP_GAUS_COEF (0x00048)
    for (int i = 0; i < 6; i++) {
        LOGD_ASHARP("(0x00048) img_lpf_coeff[%d]:0x%x \n", i, pFix->img_lpf_coeff[i]);
    }

    // SHARP_GAIN (0x0050)
    LOGD_ASHARP("(0x0050) sharp_global_gain:0x%x gain_merge_alpha:0x%x local_gain_scale:0x%x \n",
                pFix->global_gain, pFix->gain_merge_alpha, pFix->local_gain_scale);

    // SHARP_GAIN_ADJUST (0x54)
    for (int i = 0; i < 14; i++) {
        LOGD_ASHARP("(0x00048) sharp_gain2strg_val[%d]:0x%x \n", i, pFix->gain2strg_val[i]);
    }

    // SHARP_CENTER (0x70)
    LOGD_ASHARP("(0x0070) sharp_center_x:0x%x shrap_center_y:%x \n", pFix->center_x,
                pFix->center_y);

    // SHARP_GAIN_DIS_STRENGTH (0x74)
    for (int i = 0; i < 22; i++) {
        LOGD_ASHARP("(0x00048) sharp_distance2strg_val[%d]:0x%x \n", i, pFix->distance2strg_val[i]);
    }

    // SHARP_TEXTURE (0x8c)
    LOGD_ASHARP(
        "(0x0070) sharp_noise_max_limit:0x%x sharp_tex_reserve_level:%x sharp_noise_strg:%x \n",
        pFix->noise_max_limit, pFix->tex_reserve_level, pFix->noise_strg);

    // SHARP_TEXTURE (0x28)
    for (int i = 0; i < 8; i++) {
        LOGD_ASHARP("(0x00048) sharp_luma2strg_val[%d]:0x%x \n", i, pFix->luma2strg_val[i]);
    }

    // SHARP_TEXTURE (0x90)
    for (int i = 0; i < 8; i++) {
        LOGD_ASHARP("(0x00048) sharp_luma2neg_clip_val[%d]:0x%x \n", i, pFix->luma2neg_clip_val[i]);
    }

    return res;
}

Asharp_result_V34_t sharp_get_setting_by_name_json_V34(void* pCalibdbV2_v, char* name,
                                                       int* tuning_idx) {
    int i                   = 0;
    Asharp_result_V34_t res = ASHARP_V34_RET_SUCCESS;

    LOG1_ASHARP("%s(%d): enter  \n", __FUNCTION__, __LINE__);

    CalibDbV2_SharpV34_t* pCalibdbV2 = (CalibDbV2_SharpV34_t*)pCalibdbV2_v;

    if (pCalibdbV2 == NULL || name == NULL || tuning_idx == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP_V34_RET_NULL_POINTER;
    }

    for (i = 0; i < pCalibdbV2->TuningPara.Setting_len; i++) {
        if (strncmp(name, pCalibdbV2->TuningPara.Setting[i].SNR_Mode,
                    strlen(name) * sizeof(char)) == 0) {
            break;
        }
    }

    if (i < pCalibdbV2->TuningPara.Setting_len) {
        *tuning_idx = i;
    } else {
        *tuning_idx = 0;
    }

    LOG1_ASHARP("%s:%d snr_name:%s  snr_idx:%d i:%d \n", __FUNCTION__, __LINE__, name, *tuning_idx,
                i);
    return res;
}

Asharp_result_V34_t sharp_init_params_json_V34(void* pSharpParams_v, void* pCalibdbV2_v,
                                               int tuning_idx) {
    Asharp_result_V34_t res = ASHARP_V34_RET_SUCCESS;
    int i                   = 0;
    int j                   = 0;
    short isoCurveSectValue;
    short isoCurveSectValue1;
    float ave1, ave2, ave3, ave4;
    int bit_calib = 12;
    int bit_proc;
    int bit_shift;
    CalibDbV2_SharpV34_t* pCalibdbV2    = (CalibDbV2_SharpV34_t*)pCalibdbV2_v;
    RK_SHARP_Params_V34_t* pSharpParams = (RK_SHARP_Params_V34_t*)pSharpParams_v;
    CalibDbV2_SharpV34_T_ISO_t* pTuningISO;

    LOG1_ASHARP("%s(%d): enter\n", __FUNCTION__, __LINE__);

    if (pSharpParams == NULL || pCalibdbV2 == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP_V34_RET_NULL_POINTER;
    }

    pSharpParams->enable                  = pCalibdbV2->TuningPara.enable;
    pSharpParams->sw_sharp_filtCoeff_mode = pCalibdbV2->TuningPara.sw_sharp_filtCoeff_mode;
    pSharpParams->hw_sharp_centerPosition_mode =
        pCalibdbV2->TuningPara.hw_sharp_centerPosition_mode;
    pSharpParams->hw_sharp_texWgt_mode = pCalibdbV2->TuningPara.hw_sharp_texWgt_mode;
    for (i = 0; i < pCalibdbV2->TuningPara.Setting[tuning_idx].Tuning_ISO_len &&
                i < RK_SHARP_V34_MAX_ISO_NUM;
         i++) {
        pTuningISO           = &pCalibdbV2->TuningPara.Setting[tuning_idx].Tuning_ISO[i];
        pSharpParams->iso[i] = pTuningISO->iso;
        pSharpParams->sharpParamsISO[i].hw_sharp_localGain_bypass =
            pTuningISO->hw_sharp_localGain_bypass;

        pSharpParams->sharpParamsISO[i].sw_sharp_preBifilt_scale =
            pTuningISO->sw_sharp_preBifilt_scale;
        pSharpParams->sharpParamsISO[i].sw_sharp_preBifilt_offset =
            pTuningISO->sw_sharp_preBifilt_offset;
        pSharpParams->sharpParamsISO[i].sw_sharp_preBifilt_alpha =
            pTuningISO->sw_sharp_preBifilt_alpha;

        pSharpParams->sharpParamsISO[i].sw_sharp_guideFilt_alpha =
            pTuningISO->sw_sharp_guideFilt_alpha;
        pSharpParams->sharpParamsISO[i].sw_sharp_imgLpf0_strg = pTuningISO->sw_sharp_imgLpf0_strg;
        pSharpParams->sharpParamsISO[i].sw_sharp_imgLpf1_strg = pTuningISO->sw_sharp_imgLpf1_strg;
        pSharpParams->sharpParamsISO[i].hw_sharp_baseImg_sel  = pTuningISO->hw_sharp_baseImg_sel;
        pSharpParams->sharpParamsISO[i].hw_sharp_clipIdx_sel  = pTuningISO->hw_sharp_clipIdx_sel;

        pSharpParams->sharpParamsISO[i].sw_sharp_detailBifilt_scale =
            pTuningISO->sw_sharp_detailBifilt_scale;
        pSharpParams->sharpParamsISO[i].sw_sharp_detailBifilt_offset =
            pTuningISO->sw_sharp_detailBifilt_offset;
        pSharpParams->sharpParamsISO[i].sw_sharp_detailBifilt_alpha =
            pTuningISO->sw_sharp_detailBifilt_alpha;

        pSharpParams->sharpParamsISO[i].sw_sharp_global_gain = pTuningISO->sw_sharp_global_gain;
        pSharpParams->sharpParamsISO[i].sw_sharp_gainMerge_alpha =
            pTuningISO->sw_sharp_gainMerge_alpha;
        pSharpParams->sharpParamsISO[i].sw_sharp_localGain_scale =
            pTuningISO->sw_sharp_localGain_scale;

        for (j = 0; j < 14; j++)
            pSharpParams->sharpParamsISO[i].sw_sharp_gain2strg_val[j] =
                pTuningISO->sw_sharp_gain2strg_val[j];

        for (j = 0; j < 22; j++)
            pSharpParams->sharpParamsISO[i].sw_sharp_distance2strg_val[j] =
                pTuningISO->sw_sharp_distance2strg_val[j];

        pSharpParams->sharpParamsISO[i].hw_sharp_noiseFilt_sel = pTuningISO->hw_sharp_noiseFilt_sel;
        pSharpParams->sharpParamsISO[i].hw_sharp_noiseNorm_bit = pTuningISO->hw_sharp_noiseNorm_bit;
        pSharpParams->sharpParamsISO[i].hw_sharp_noise_maxLimit =
            pTuningISO->hw_sharp_noise_maxLimit;
        pSharpParams->sharpParamsISO[i].hw_sharp_noiseClip_sel = pTuningISO->hw_sharp_noiseClip_sel;
        pSharpParams->sharpParamsISO[i].sw_sharp_noise_strg    = pTuningISO->sw_sharp_noise_strg;
        pSharpParams->sharpParamsISO[i].sw_sharp_texReserve_strg =
            pTuningISO->sw_sharp_texReserve_strg;
        pSharpParams->sharpParamsISO[i].hw_sharp_tex2wgt_en  = pTuningISO->hw_sharp_tex2wgt_en;

        for (j = 0; j < 17; j++)
            pSharpParams->sharpParamsISO[i].hw_sharp_tex2wgt_val[j] =
                pTuningISO->hw_sharp_tex2wgt_val[j];

        for (j = 0; j < 17; j++)
            pSharpParams->sharpParamsISO[i].hw_sharp_detail2strg_val[j] =
                pTuningISO->hw_sharp_detail2strg_val[j];

        for (j = 0; j < RK_SHARP_V34_LUMA_POINT_NUM; j++) {
            pSharpParams->sharpParamsISO[i].hw_sharp_luma2Table_idx[j] =
                pTuningISO->luma_para.hw_sharp_luma2Table_idx[j];
            pSharpParams->sharpParamsISO[i].sw_sharp_luma2vsigma_val[j] =
                pTuningISO->luma_para.sw_sharp_luma2vsigma_val[j];
            pSharpParams->sharpParamsISO[i].hw_sharp_luma2posClip_val[j] =
                pTuningISO->luma_para.hw_sharp_luma2posClip_val[j];
            pSharpParams->sharpParamsISO[i].hw_sharp_luma2negClip_val[j] =
                pTuningISO->luma_para.hw_sharp_luma2negClip_val[j];
            pSharpParams->sharpParamsISO[i].hw_sharp_luma2strg_val[j] =
                pTuningISO->luma_para.hw_sharp_luma2strg_val[j];
        }

        for (j = 0; j < 3; j++) {
            pSharpParams->sharpParamsISO[i].sw_sharp_preBifilt_coeff[j] =
                pTuningISO->kernel_para.sw_sharp_preBifilt_coeff[j];
            pSharpParams->sharpParamsISO[i].sw_sharp_detailBifilt_coeff[j] =
                pTuningISO->kernel_para.sw_sharp_detailBifilt_coeff[j];
        }

        for (j = 0; j < 6; j++) {
            pSharpParams->sharpParamsISO[i].sw_sharp_imgLpf0_coeff[j] =
                pTuningISO->kernel_para.sw_sharp_imgLpf0_coeff[j];
            pSharpParams->sharpParamsISO[i].sw_sharp_imgLpf1_coeff[j] =
                pTuningISO->kernel_para.sw_sharp_imgLpf1_coeff[j];
        }

        pSharpParams->sharpParamsISO[i].sw_sharp_preBifilt_rsigma =
            pTuningISO->kernel_sigma.sw_sharp_preBifilt_rsigma;
        pSharpParams->sharpParamsISO[i].sw_sharp_detailBifilt_rsigma =
            pTuningISO->kernel_sigma.sw_sharp_detailBifilt_rsigma;
        pSharpParams->sharpParamsISO[i].sw_sharp_imgLpf0_rsigma =
            pTuningISO->kernel_sigma.sw_sharp_imgLpf0_rsigma;
        pSharpParams->sharpParamsISO[i].sw_sharp_imgLpf0_radius =
            pTuningISO->kernel_sigma.sw_sharp_imgLpf0_radius;
        pSharpParams->sharpParamsISO[i].sw_sharp_imgLpf1_rsigma =
            pTuningISO->kernel_sigma.sw_sharp_imgLpf1_rsigma;
        pSharpParams->sharpParamsISO[i].sw_sharp_imgLpf1_radius =
            pTuningISO->kernel_sigma.sw_sharp_imgLpf1_radius;
    }

    LOG1_ASHARP("%s(%d): exit\n", __FUNCTION__, __LINE__);
    return res;
}

Asharp_result_V34_t sharp_config_setting_param_json_V34(void* pParams_v, void* pCalibdbV2_v,
                                                        char* param_mode, char* snr_name) {
    Asharp_result_V34_t res          = ASHARP_V34_RET_SUCCESS;
    int tuning_idx                   = 0;
    CalibDbV2_SharpV34_t* pCalibdbV2 = (CalibDbV2_SharpV34_t*)pCalibdbV2_v;
    RK_SHARP_Params_V34_t* pParams   = (RK_SHARP_Params_V34_t*)pParams_v;

    LOG1_ASHARP("%s(%d): enter\n", __FUNCTION__, __LINE__);

    if (pParams == NULL || pCalibdbV2 == NULL || param_mode == NULL || snr_name == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        pParams->enable = false;
        return ASHARP_V34_RET_NULL_POINTER;
    }

    res = sharp_get_setting_by_name_json_V34(pCalibdbV2, snr_name, &tuning_idx);
    if (res != ASHARP_V34_RET_SUCCESS) {
        LOGW_ASHARP("%s(%d): error!!!  can't find setting in iq files, use 0 instead\n",
                    __FUNCTION__, __LINE__);
    }

    res             = sharp_init_params_json_V34(pParams, pCalibdbV2, tuning_idx);
    pParams->enable = pCalibdbV2->TuningPara.enable;

    LOG1_ASHARP("%s(%d): exit\n", __FUNCTION__, __LINE__);

    return res;
}

RKAIQ_END_DECLARE
