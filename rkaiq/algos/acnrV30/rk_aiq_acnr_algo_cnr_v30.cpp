#include "rk_aiq_acnr_algo_cnr_v30.h"


RKAIQ_BEGIN_DECLARE

#define Math_LOG2(x)    (log((double)x)   / log((double)2))
#define interp_cnr_v30(value_low, value_high, ratio) (ratio * ((value_high) - (value_low)) + value_low)



AcnrV30_result_t cnr_select_params_by_ISO_V30(RK_CNR_Params_V30_t *pParams, RK_CNR_Params_V30_Select_t *pSelect, AcnrV30_ExpInfo_t *pExpInfo)
{
    AcnrV30_result_t res = ACNRV30_RET_SUCCESS;
    int iso = 50;
    if(pParams == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNRV30_RET_NULL_POINTER;
    }

    if(pSelect == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNRV30_RET_NULL_POINTER;
    }

    if(pExpInfo == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNRV30_RET_NULL_POINTER;
    }

    iso = pExpInfo->arIso[pExpInfo->hdr_mode];

    //ȷ��iso�ȼ�
    //rkuvnriso@50 100 200 400 800 1600 3200  6400 12800
    //      isogain: 1  2   4   8   16  32   64    128  256
    //     isoindex: 0  1   2   3   4   5    6     7    8

    int isoGainLow = 0;
    int isoGainHigh = 0;
    int isoIndexLow = 0;
    int isoIndexHigh = 0;
    int iso_div = 50;
    int max_iso_step = RK_CNR_V30_MAX_ISO_NUM;

#ifndef RK_SIMULATOR_HW
    for (int i = 0; i < max_iso_step - 1 ; i++) {
        if (iso >= pParams->iso[i]  &&  iso <= pParams->iso[i + 1]) {
            isoGainLow = pParams->iso[i] ;
            isoGainHigh = pParams->iso[i + 1];
            isoIndexLow = i;
            isoIndexHigh = i + 1;
        }
    }

    if(iso < pParams->iso[0] ) {
        isoGainLow = pParams->iso[0];
        isoGainHigh = pParams->iso[1];
        isoIndexLow = 0;
        isoIndexHigh = 1;
    }

    if(iso > pParams->iso[max_iso_step - 1] ) {
        isoGainLow = pParams->iso[max_iso_step - 2] ;
        isoGainHigh = pParams->iso[max_iso_step - 1];
        isoIndexLow = max_iso_step - 2;
        isoIndexHigh = max_iso_step - 1;
    }
#else

    for (int i = max_iso_step - 1; i >= 0; i--) {
        if (iso < iso_div * (2 << i)) {
            isoGainLow = iso_div * (2 << (i)) / 2;
            isoGainHigh = iso_div * (2 << i);
        }
    }

    isoGainLow = MIN(isoGainLow, iso_div * (2 << max_iso_step));
    isoGainHigh = MIN(isoGainHigh, iso_div * (2 << max_iso_step));

    isoIndexHigh = (int)(log((float)isoGainHigh / iso_div) / log((float)2));
    isoIndexLow = (int)(log((float)isoGainLow / iso_div) / log((float)2));

    isoIndexLow = MIN(MAX(isoIndexLow, 0), max_iso_step - 1);
    isoIndexHigh = MIN(MAX(isoIndexHigh, 0), max_iso_step - 1);
#endif

    pSelect->enable = pParams->enable;

    LOGD_ANR("%s:%d iso:%d high:%d low:%d \n",
             __FUNCTION__, __LINE__,
             iso, isoGainHigh, isoGainLow);

    pExpInfo->isoLevelLow = isoIndexLow;
    pExpInfo->isoLevelHig = isoIndexHigh;
    RK_CNR_Params_V30_Select_t *pLowISO = &pParams->CnrParamsISO[isoIndexLow];
    RK_CNR_Params_V30_Select_t *pHighISO = &pParams->CnrParamsISO[isoIndexHigh];

    if ((isoGainHigh - iso) < (iso - isoGainLow))   {
        pSelect->down_scale_x = pHighISO->down_scale_x;
        pSelect->down_scale_y = pHighISO->down_scale_y;
        pSelect->bf_wgt0_sel = pHighISO->bf_wgt0_sel;

        for(int i = 0; i < 4; i++) {
            pSelect->thumb_filter_wgt_coeff[i] = pHighISO->thumb_filter_wgt_coeff[i];
        }

        for(int i = 0; i < 6; i++) {
            pSelect->gaus_coeff[i] = pHighISO->gaus_coeff[i];
        }
    }
    else {
        pSelect->down_scale_x = pLowISO->down_scale_x;
        pSelect->down_scale_y = pLowISO->down_scale_y;
        pSelect->bf_wgt0_sel  = pLowISO->bf_wgt0_sel;

        for(int i = 0; i < 4; i++) {
            pSelect->thumb_filter_wgt_coeff[i] = pLowISO->thumb_filter_wgt_coeff[i];
        }

        for(int i = 0; i < 6; i++) {
            pSelect->gaus_coeff[i] = pLowISO->gaus_coeff[i];
        }
    }

    float ratio = float(iso - isoGainLow) / float(isoGainHigh - isoGainLow);

    pSelect->thumb_sigma = interp_cnr_v30( pLowISO->thumb_sigma, pHighISO->thumb_sigma, ratio);
    pSelect->thumb_bf_ratio = interp_cnr_v30(pLowISO->thumb_bf_ratio, pHighISO->thumb_bf_ratio, ratio);

    pSelect->chroma_filter_strength = interp_cnr_v30(pLowISO->chroma_filter_strength, pHighISO->chroma_filter_strength, ratio);
    pSelect->chroma_filter_wgt_clip = interp_cnr_v30(pLowISO->chroma_filter_wgt_clip, pHighISO->chroma_filter_wgt_clip, ratio);
    pSelect->anti_chroma_ghost = interp_cnr_v30(pLowISO->anti_chroma_ghost, pHighISO->anti_chroma_ghost, ratio);
    pSelect->chroma_filter_uv_gain = interp_cnr_v30(pLowISO->chroma_filter_uv_gain, pHighISO->chroma_filter_uv_gain, ratio);
    pSelect->wgt_slope = interp_cnr_v30(pLowISO->wgt_slope, pHighISO->wgt_slope, ratio);

    pSelect->gaus_ratio = interp_cnr_v30(pLowISO->gaus_ratio, pHighISO->gaus_ratio, ratio);

    pSelect->bf_sigmaR = interp_cnr_v30(pLowISO->bf_sigmaR, pHighISO->bf_sigmaR, ratio);
    pSelect->bf_uvgain = interp_cnr_v30(pLowISO->bf_uvgain, pHighISO->bf_uvgain, ratio);
    pSelect->bf_ratio = interp_cnr_v30(pLowISO->bf_ratio, pHighISO->bf_ratio, ratio);
    pSelect->hbf_wgt_clip = interp_cnr_v30(pLowISO->hbf_wgt_clip, pHighISO->hbf_wgt_clip, ratio);
    pSelect->global_alpha = interp_cnr_v30(pLowISO->global_alpha, pHighISO->global_alpha, ratio);

    pSelect->saturation_adj_offset = interp_cnr_v30(pLowISO->saturation_adj_offset, pHighISO->saturation_adj_offset, ratio);
    pSelect->saturation_adj_ratio = interp_cnr_v30(pLowISO->saturation_adj_ratio, pHighISO->saturation_adj_ratio, ratio);

    pSelect->global_gain = interp_cnr_v30(pLowISO->global_gain, pHighISO->global_gain, ratio);
    pSelect->global_gain_alpha = interp_cnr_v30(pLowISO->global_gain_alpha, pHighISO->global_gain_alpha, ratio);
    pSelect->local_gain_scale = interp_cnr_v30(pLowISO->local_gain_scale, pHighISO->local_gain_scale, ratio);
    pSelect->global_gain_thumb = interp_cnr_v30(pLowISO->global_gain_thumb, pHighISO->global_gain_thumb, ratio);
    pSelect->global_gain_alpha_thumb = interp_cnr_v30(pLowISO->global_gain_alpha_thumb, pHighISO->global_gain_alpha_thumb, ratio);

    for (int i = 0; i < 13; i++)
    {
        pSelect->gain_adj_strength_ratio[i] = interp_cnr_v30(pLowISO->gain_adj_strength_ratio[i], pHighISO->gain_adj_strength_ratio[i], ratio);
    }

    return ACNRV30_RET_SUCCESS;

}


AcnrV30_result_t cnr_fix_transfer_V30(RK_CNR_Params_V30_Select_t *pSelect, RK_CNR_Fix_V30_t *pFix, AcnrV30_ExpInfo_t *pExpInfo, rk_aiq_cnr_strength_v30_t* pStrength)
{
    LOGI_ANR("%s:(%d) enter \n", __FUNCTION__, __LINE__);

    int i = 0;
    AcnrV30_result_t res = ACNRV30_RET_SUCCESS;
    int tmp = 0;

    if(pSelect == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNRV30_RET_NULL_POINTER;
    }

    if(pFix == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNRV30_RET_NULL_POINTER;
    }

    if(pExpInfo == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNRV30_RET_NULL_POINTER;
    }

    if(pStrength == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNRV30_RET_NULL_POINTER;
    }

    float fStrength = 1.0;

    if(pStrength->strength_enable) {
        fStrength = pStrength->percent;
    }
    if(fStrength <= 0.0) {
        fStrength = 0.000001;
    }

    LOGD_ANR("strength_enable:%d fStrength: %f \n", pStrength->strength_enable, fStrength);

    // CNR_CTRL
    pFix->bf3x3_wgt0_sel = pSelect->bf_wgt0_sel;
#if RKAIQ_HAVE_CNR_V30
    if(pSelect->down_scale_x == 2 && pSelect->down_scale_y == 2) {
        pFix->thumb_mode = 0;
    } else if(pSelect->down_scale_x == 4 && pSelect->down_scale_y == 4) {
        pFix->thumb_mode = 1;
    } else if(pSelect->down_scale_x == 8 && pSelect->down_scale_y == 6) {
        pFix->thumb_mode = 2;
    } else {
        pFix->thumb_mode = 2;
    }
#else
    if (pSelect->down_scale_x == 4 && pSelect->down_scale_y == 4) {
        pFix->thumb_mode = 1;
    } else if (pSelect->down_scale_x == 8 && pSelect->down_scale_y == 4) {
        pFix->thumb_mode = 3;
    } else {
        pFix->thumb_mode = 1;
    }
#endif
    pFix->yuv422_mode = 0;
    pFix->exgain_bypass = 0;
    pFix->cnr_en = pSelect->enable;


    // CNR_EXGAIN
    tmp = pSelect->local_gain_scale * (1 << RKCNR_V30_GAIN_ISO_FIX_BITS);
    pFix->gain_iso = CLIP(tmp, 0, 0xff);
    tmp = pSelect->global_gain_alpha * (1 << RKCNR_V30_G_GAIN_ALPHA_FIX_BITS);
    pFix->global_gain_alpha = CLIP(tmp, 0, 0x8);
    tmp = pSelect->global_gain * (1 << RKCNR_V30_G_GAIN_FIX_BITS);
    pFix->global_gain = CLIP(tmp, 0, 0x3ff);


    /* CNR_THUMB1 */
    int yuvBit = 10;
    int scale  = (1 << yuvBit) - 1;
    int log2e = (int)(0.8493f * (1 << (RKCNR_V30_log2e + RKCNR_V30_SIGMA_FIX_BIT)));
    int rkcnr_chroma_filter_uv_gain =
        ROUND_F((1 << RKCNR_V30_uvgain) * pSelect->chroma_filter_uv_gain);
    int thumbBFilterSigma = ROUND_F(pSelect->thumb_sigma * scale * fStrength);
    LOGD_ANR("scale:%d thumbBFilterSigma:%d\n", log2e, thumbBFilterSigma);
    thumbBFilterSigma = (int)(0.8493f * (1 << RKCNR_V30_FIX_BIT_INV_SIGMA) / thumbBFilterSigma);
    tmp = thumbBFilterSigma * ((1 << RKCNR_V30_uvgain) - rkcnr_chroma_filter_uv_gain * 2);
    LOGD_ANR("thumbBFilterSigma:%d sigmaY:%d\n", thumbBFilterSigma, tmp);
    tmp                 = ROUND_INT(tmp, 6);
    pFix->thumb_sigma_y = CLIP(tmp, 0, 0x3fff);
    tmp                 = thumbBFilterSigma * rkcnr_chroma_filter_uv_gain;
    tmp = ROUND_INT(tmp, 6); // (diff * sigma) >> tmp = (diff * (sigma >> 6))
    pFix->thumb_sigma_c = CLIP(tmp, 0, 0x3fff);


    /* CNR_THUMB_BF_RATIO */
    tmp = ROUND_F((1 << RKCNR_V30_FIX_BIT_BF_RATIO) * pSelect->thumb_bf_ratio * fStrength);
    pFix->thumb_bf_ratio = CLIP(tmp, 0, 0x7ff);

    /* CNR_LBF_WEITD */
    for(i = 0; i < RKCNR_V30_THUMB_BF_RADIUS + 1; i++) {
        tmp = ROUND_F(pSelect->thumb_filter_wgt_coeff[i] * (1 << RKCNR_V30_exp2_lut_y));
        pFix->lbf1x7_weit_d[i] = CLIP(tmp, 0, 0xff);
    }

    /* CNR_IIR_PARA1 */
    tmp = ROUND_F((1 << RKCNR_V30_FIX_BIT_SLOPE) * pSelect->wgt_slope);
    pFix->wgt_slope = CLIP(tmp, 0, 0x3ff);

    tmp = ROUND_F(1.2011 * (1 << RKCNR_V30_FIX_BIT_INV_SIGMA) / (pSelect->chroma_filter_strength * scale * fStrength));
    int tmptmp = tmp * pFix->wgt_slope;
    int shiftBit = Math_LOG2(tmptmp) - RKCNR_V30_FIX_BIT_INT_TO_FLOAT;
    LOGD_ANR("tmp:%d tmptmp:%d shiftBit:%d\n", tmp, tmptmp, shiftBit);
    shiftBit = MAX(shiftBit, 0);

    tmp = RKCNR_V30_FIX_BIT_INV_SIGMA - shiftBit;
    if (tmp < 0) {
        tmp = ABS(tmp) + (1 << 5);
    }
    pFix->exp_shift = CLIP(tmp, 0, 0x3f);

    tmp = ROUND_F((float)tmptmp / (1 << shiftBit));
    pFix->iir_strength = CLIP(tmp, 0, 0xff);

    tmp = ROUND_F((1 << RKCNR_V30_uvgain) * pSelect->chroma_filter_uv_gain);
    pFix->iir_uvgain = CLIP(tmp, 0, 0xf);

    /* CNR_IIR_PARA2 */
    tmp = ROUND_F((1 << RKCNR_V30_FIX_BIT_IIR_WGT) * pSelect->anti_chroma_ghost);
    pFix->chroma_ghost = CLIP(tmp, 0, 0x3f);
    tmp = ROUND_F((1 << 3) * pSelect->chroma_filter_wgt_clip);
    pFix->iir_uv_clip = CLIP(tmp, 0, 0x7f);

    /* CNR_GAUS_COE */
    for(i = 0; i < 6; i++) {
        tmp = pSelect->gaus_coeff[5 - i];
        pFix->gaus_coe[i] = CLIP(tmp, 0, 0x7f);
    }

    /* CNR_GAUS_RATIO */
    tmp = ROUND_F((1 << RKCNR_V30_FIX_BIT_GAUS_RATIO) * pSelect->gaus_ratio);
    pFix->gaus_ratio = CLIP(tmp, 0, 0x7ff);
    tmp = ROUND_F((1 << RKCNR_V30_exp2_lut_y) * pSelect->hbf_wgt_clip);
    pFix->bf_wgt_clip = CLIP(tmp, 0, 0xff);
    tmp = ROUND_F((1 << RKCNR_V30_FIX_BIT_GLOBAL_ALPHA) * pSelect->global_alpha);
    pFix->global_alpha = CLIP(tmp, 0, 0x7ff);

    /* CNR_BF_PARA1 */
    tmp = ROUND_F((1 << RKCNR_V30_uvgain) * pSelect->bf_uvgain);
    pFix->uv_gain = CLIP(tmp, 0, 0x7f);
    tmp = (int)(log2e / (pSelect->bf_sigmaR * scale * fStrength));
    pFix->sigma_r = CLIP(tmp, 0, 0x3ff);
    tmp = ROUND_F((1 << RKCNR_V30_bfRatio) * pSelect->bf_ratio / fStrength);
    pFix->bf_ratio = CLIP(tmp, 0, 0xff);

    /* CNR_BF_PARA2 */
    tmp =  pSelect->saturation_adj_offset;
    pFix->adj_offset = CLIP(tmp, 0, 0x1ff);
    tmp = ROUND_F((1 << RKCNR_V30_FIX_BIT_SATURATION) * pSelect->saturation_adj_ratio);
    pFix->adj_ratio = CLIP(tmp, 0, 0x7fff);

    /* CNR_SIGMA */
    for(i = 0; i < 13; i++) {
        tmp = ROUND_F(pSelect->gain_adj_strength_ratio[i] * (1 << RKCNR_V30_sgmRatio));
        pFix->sigma_y[i] = CLIP(tmp, 0, 0xff);
    }

    /* CNR_IIR_GLOBAL_GAIN */
    //tmp = pSelect->global_gain_alpha_thumb * (1 << RKCNR_V30_G_GAIN_ALPHA_FIX_BITS);
    pFix->iir_gain_alpha = 8;    //ic suggest must be 8                                // CLIP(tmp, 0, 0xf);
    tmp = pSelect->global_gain_thumb * (1 << RKCNR_V30_sgmRatio);//(1 << RKCNR_V30_G_GAIN_FIX_BITS);
    pFix->iir_global_gain = CLIP(tmp, 0, 0xff);

    cnr_fix_printf_V30(pFix);

    LOGI_ANR("%s:(%d) exit \n", __FUNCTION__, __LINE__);

    return ACNRV30_RET_SUCCESS;
}


AcnrV30_result_t cnr_fix_printf_V30(RK_CNR_Fix_V30_t  * pFix)
{
    int i = 0;
    LOGI_ANR("%s:(%d) enter \n", __FUNCTION__, __LINE__);

    AcnrV30_result_t res = ACNRV30_RET_SUCCESS;

    if(pFix == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNRV30_RET_NULL_POINTER;
    }


    //CNR_CTRL (0x0000)
    LOGD_ANR("(0x0000) bf3x3_wgt0_sel:0x%x thumb_mode:0x%x yuv422_mode:0x%x exgain_bypass:0x%x cnr_en:0x%x\n",
             pFix->bf3x3_wgt0_sel,
             pFix->thumb_mode,
             pFix->yuv422_mode,
             pFix->exgain_bypass,
             pFix->cnr_en);

    // CNR_EXGAIN  (0x0004)
    LOGD_ANR("(0x0004) global_gain:0x%x  global_gain_alpha:0x%x  gain_iso:0x%x \n",
             pFix->global_gain,
             pFix->global_gain_alpha,
             pFix->gain_iso);

    // CNR_THUMB1  (0x0008)
    LOGD_ANR("(0x0008) thumb_sigma_c:0x%x thumb_sigma_y:0x%x \n",
             pFix->thumb_sigma_c,
             pFix->thumb_sigma_y);

    // CNR_THUMB_BF_RATIO (0x000c)
    LOGD_ANR("(0x000c) thumb_bf_ratio:0x%x \n",
             pFix->thumb_bf_ratio);

    // CNR_LBF_WEITD (0x0010)
    LOGD_ANR("(0x0010) lbf1x7_weit_d:0x%x  0x%x  0x%x  0x%x \n",
             pFix->lbf1x7_weit_d[0],
             pFix->lbf1x7_weit_d[1],
             pFix->lbf1x7_weit_d[2],
             pFix->lbf1x7_weit_d[3]);

    // CNR_IIR_PARA1 (0x0014)
    LOGD_ANR("(0x0014) iir_uvgain:0x%x  iir_strength:0x%x  exp_shift:0x%x  wgt_slope:0x%x \n",
             pFix->iir_uvgain,
             pFix->iir_strength,
             pFix->exp_shift,
             pFix->wgt_slope);


    // CNR_IIR_PARA2 (0x0018)
    LOGD_ANR("(0x0018) chroma_ghost:0x%x iir_uv_clip:0x%x \n",
             pFix->chroma_ghost,
             pFix->iir_uv_clip);

    // CNR_GAUS_COE (0x001c - 0x0020)
    for(i = 0; i < 6; i++) {
        LOGD_ANR("(0x001c - 0x0020) gaus_coe[%d]:0x%x \n",
                 i, pFix->gaus_coe[i]);
    }

    // CNR_GAUS_RATIO  (0x0024)
    LOGD_ANR("(0x0024) gaus_ratio:0x%x  bf_wgt_clip:0x%x  global_alpha:0x%x \n",
             pFix->gaus_ratio,
             pFix->bf_wgt_clip,
             pFix->global_alpha);

    // CNR_BF_PARA1  (0x0028)
    LOGD_ANR("(0x0028) uv_gain:0x%x  sigma_r:0x%x  bf_ratio:0x%x \n",
             pFix->uv_gain,
             pFix->sigma_r,
             pFix->bf_ratio);

    // CNR_BF_PARA2 (0x002c)
    LOGD_ANR("(0x002c) adj_offset:0x%x adj_ratio:0x%x \n",
             pFix->adj_offset,
             pFix->adj_ratio);

    // CNR_SIGMA (0x0030 - 0x003c)
    for(i = 0; i < 13; i++) {
        LOGD_ANR("(0x0030 - 0x003c) sigma_y[%d]:0x%x \n", i, pFix->sigma_y[i]);
    }

    // CNR_IIR_GLOBAL_GAIN (0x0040)
    LOGD_ANR("(0x0040) iir_gain_alpha:0x%x iir_global_gain:0x%x \n",
             pFix->iir_gain_alpha,
             pFix->iir_global_gain);


    return res;
}

AcnrV30_result_t cnr_get_setting_by_name_json_V30(CalibDbV2_CNRV30_t* pCalibdbV2, char* name,
        int* tuning_idx) {
    int i                = 0;
    AcnrV30_result_t res = ACNRV30_RET_SUCCESS;

    if (pCalibdbV2 == NULL || name == NULL || tuning_idx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNRV30_RET_NULL_POINTER;
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

    LOGD_ANR("%s:%d snr_name:%s  snr_idx:%d i:%d \n", __FUNCTION__, __LINE__, name, *tuning_idx, i);

    return res;
}

AcnrV30_result_t cnr_init_params_json_V30(RK_CNR_Params_V30_t *pParams, CalibDbV2_CNRV30_t *pCalibdbV2, int tuning_idx)
{
    AcnrV30_result_t res = ACNRV30_RET_SUCCESS;
    CalibDbV2_CNRV30_T_ISO_t *pTuningISO = NULL;

    if(pParams == NULL || pCalibdbV2 == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNRV30_RET_NULL_POINTER;
    }

    pParams->enable = pCalibdbV2->TuningPara.enable;

    for(int i = 0; i < pCalibdbV2->TuningPara.Setting[tuning_idx].Tuning_ISO_len && i < RK_CNR_V30_MAX_ISO_NUM; i++ ) {
        pTuningISO = &pCalibdbV2->TuningPara.Setting[tuning_idx].Tuning_ISO[i];

        pParams->iso[i] = pTuningISO->iso;
        pParams->CnrParamsISO[i].enable = pCalibdbV2->TuningPara.enable;
        pParams->CnrParamsISO[i].down_scale_x = pTuningISO->down_scale_x;
        pParams->CnrParamsISO[i].down_scale_y = pTuningISO->down_scale_y;

        pParams->CnrParamsISO[i].thumb_sigma = pTuningISO->thumb_sigma;
        pParams->CnrParamsISO[i].thumb_bf_ratio = pTuningISO->thumb_bf_ratio;

        pParams->CnrParamsISO[i].chroma_filter_strength = pTuningISO->chroma_filter_strength;
        pParams->CnrParamsISO[i].chroma_filter_wgt_clip = pTuningISO->chroma_filter_wgt_clip;
        pParams->CnrParamsISO[i].anti_chroma_ghost = pTuningISO->anti_chroma_ghost;
        pParams->CnrParamsISO[i].chroma_filter_uv_gain = pTuningISO->chroma_filter_uv_gain;
        pParams->CnrParamsISO[i].wgt_slope = pTuningISO->wgt_slope;

        pParams->CnrParamsISO[i].gaus_ratio = pTuningISO->gaus_ratio;

        pParams->CnrParamsISO[i].bf_sigmaR = pTuningISO->bf_sigmaR;
        pParams->CnrParamsISO[i].bf_uvgain = pTuningISO->bf_uvgain;
        pParams->CnrParamsISO[i].bf_ratio = pTuningISO->bf_ratio;
        pParams->CnrParamsISO[i].hbf_wgt_clip = pTuningISO->hbf_wgt_clip;
        pParams->CnrParamsISO[i].bf_wgt0_sel = pTuningISO->bf_wgt0_sel;
        pParams->CnrParamsISO[i].global_alpha = pTuningISO->global_alpha;

        pParams->CnrParamsISO[i].saturation_adj_offset = pTuningISO->saturation_adj_offset;
        pParams->CnrParamsISO[i].saturation_adj_ratio = pTuningISO->saturation_adj_ratio;

        pParams->CnrParamsISO[i].global_gain = pTuningISO->global_gain;
        pParams->CnrParamsISO[i].global_gain_alpha = pTuningISO->global_gain_alpha;
        pParams->CnrParamsISO[i].local_gain_scale = pTuningISO->local_gain_scale;
        pParams->CnrParamsISO[i].global_gain_thumb = pTuningISO->global_gain_thumb;
        pParams->CnrParamsISO[i].global_gain_alpha_thumb = pTuningISO->global_gain_alpha_thumb;

        for(int j = 0; j < RKCNR_V30_SGM_ADJ_TABLE_LEN; j++) {
            pParams->CnrParamsISO[i].gain_adj_strength_ratio[j] = pTuningISO->gain_adj_strength_ratio[j];
        }

        for(int j = 0; j < 4; j++) {
            pParams->CnrParamsISO[i].thumb_filter_wgt_coeff[j] = pTuningISO->thumb_filter_wgt_coeff[j];
        }

        for(int j = 0; j < 6; j++) {
            pParams->CnrParamsISO[i].gaus_coeff[j] = pTuningISO->gaus_coeff[j];
        }
    }

    return ACNRV30_RET_SUCCESS;

}

AcnrV30_result_t cnr_config_setting_param_json_V30(RK_CNR_Params_V30_t *pParams, CalibDbV2_CNRV30_t *pCalibdbV2, char* param_mode, char * snr_name)
{
    AcnrV30_result_t res = ACNRV30_RET_SUCCESS;
    int tuning_idx;

    if(pParams == NULL || pCalibdbV2 == NULL || param_mode == NULL || snr_name == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNRV30_RET_NULL_POINTER;
    }

    res = cnr_get_setting_by_name_json_V30(pCalibdbV2, snr_name, &tuning_idx);
    if(res != ACNRV30_RET_SUCCESS) {
        LOGW_ANR("%s(%d): error!!!  can't find setting in iq files, use 0 instead\n", __FUNCTION__, __LINE__);
    }

    res = cnr_init_params_json_V30(pParams, pCalibdbV2, tuning_idx);
    pParams->enable = pCalibdbV2->TuningPara.enable;
    return res;

}


RKAIQ_END_DECLARE




