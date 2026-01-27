#include "rk_aiq_acnr_algo_cnr_v31.h"


RKAIQ_BEGIN_DECLARE

#define Math_LOG2(x)    (log((double)x)   / log((double)2))

float interp_cnr_v31(int ISO_low, int ISO_high, float value_low, float value_high, int ISO)
{
    float value = 0.0;

    if (ISO <= ISO_low) {
        value = value_low;
    }
    else if (ISO >= ISO_high) {
        value = value_high;
    }
    else {
        value = float(ISO - ISO_low) / float(ISO_high - ISO_low) * (value_high - value_low) + value_low;
    }

    return value;
}


AcnrV31_result_t cnr_select_params_by_ISO_V31(RK_CNR_Params_V31_t *pParams, RK_CNR_Params_V31_Select_t *pSelect, AcnrV31_ExpInfo_t *pExpInfo)
{
    AcnrV31_result_t res = ACNRV31_RET_SUCCESS;
    int iso = 50;
    if(pParams == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNRV31_RET_NULL_POINTER;
    }

    if(pSelect == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNRV31_RET_NULL_POINTER;
    }

    if(pExpInfo == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNRV31_RET_NULL_POINTER;
    }

    iso = pExpInfo->arIso[pExpInfo->hdr_mode];

    //ȷ��iso�ȼ�
    //rkuvnriso@50 100 200 400 800 1600 3200  6400 12800
    //      isogain: 1  2   4   8   16  32   64    128  256
    //     isoindex: 0  1   2   3   4   5    6     7    8

    int isoIndex = 0;
    int isoGainLow = 0;
    int isoGainHigh = 0;
    int isoIndexLow = 0;
    int isoIndexHigh = 0;
    int iso_div = 50;
    int max_iso_step = RK_CNR_V31_MAX_ISO_NUM;

#ifndef RK_SIMULATOR_HW
    for (int i = 0; i < max_iso_step - 1 ; i++) {
        if (iso >= pParams->iso[i]  &&  iso <= pParams->iso[i + 1]) {
            isoGainLow = pParams->iso[i] ;
            isoGainHigh = pParams->iso[i + 1];
            isoIndexLow = i;
            isoIndexHigh = i + 1;
            isoIndex = isoIndexLow;
        }
    }

    if(iso < pParams->iso[0] ) {
        isoGainLow = pParams->iso[0];
        isoGainHigh = pParams->iso[1];
        isoIndexLow = 0;
        isoIndexHigh = 1;
        isoIndex = 0;
    }

    if(iso > pParams->iso[max_iso_step - 1] ) {
        isoGainLow = pParams->iso[max_iso_step - 2] ;
        isoGainHigh = pParams->iso[max_iso_step - 1];
        isoIndexLow = max_iso_step - 2;
        isoIndexHigh = max_iso_step - 1;
        isoIndex = max_iso_step - 1;
    }
#else
    isoIndex = int(log(float(iso / iso_div)) / log(2.0f));

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
    RK_CNR_Params_V31_Select_t *pLowISO = &pParams->CnrParamsISO[isoIndexLow];
    RK_CNR_Params_V31_Select_t *pHighISO = &pParams->CnrParamsISO[isoIndexHigh];
    RK_CNR_Params_V31_Select_t *pNearISO = NULL;

    if ((isoGainHigh - iso) < (iso - isoGainLow)) {
        pNearISO = pHighISO;
    }
    else {
        pNearISO = pLowISO;
    }

    pSelect->hw_cnrT_exgain_bypass = pNearISO->hw_cnrT_exgain_bypass;
    pSelect->sw_cnrT_ds_scaleX = pNearISO->sw_cnrT_ds_scaleX;
    pSelect->sw_cnrT_ds_scaleY = pNearISO->sw_cnrT_ds_scaleY;
    for(int i = 0; i < 4; i++) {
        pSelect->sw_cnrT_thumbBf_coeff[i] = pNearISO->sw_cnrT_thumbBf_coeff[i];
    }

    for(int i = 0; i < 3; i++) {
        pSelect->hw_cnrT_loFlt_coeff[i] = pNearISO->hw_cnrT_loFlt_coeff[i];
    }

    pSelect->sw_cnrT_loBfFlt_vsigma = interp_cnr_v31(isoGainLow, isoGainHigh, pLowISO->sw_cnrT_loBfFlt_vsigma, pHighISO->sw_cnrT_loBfFlt_vsigma, iso);
    pSelect->sw_cnrT_loBfFlt_alpha = interp_cnr_v31(isoGainLow, isoGainHigh, pLowISO->sw_cnrT_loBfFlt_alpha, pHighISO->sw_cnrT_loBfFlt_alpha, iso);

    pSelect->sw_cnrT_loFlt_vsigma = interp_cnr_v31(isoGainLow, isoGainHigh, pLowISO->sw_cnrT_loFlt_vsigma, pHighISO->sw_cnrT_loFlt_vsigma, iso);
    pSelect->sw_cnrT_loFltWgt_maxLimit = interp_cnr_v31(isoGainLow, isoGainHigh, pLowISO->sw_cnrT_loFltWgt_maxLimit, pHighISO->sw_cnrT_loFltWgt_maxLimit, iso);
    pSelect->sw_cnrT_loFltWgt_minThred = interp_cnr_v31(isoGainLow, isoGainHigh, pLowISO->sw_cnrT_loFltWgt_minThred, pHighISO->sw_cnrT_loFltWgt_minThred, iso);
    pSelect->sw_cnrT_loFltUV_gain = interp_cnr_v31(isoGainLow, isoGainHigh, pLowISO->sw_cnrT_loFltUV_gain, pHighISO->sw_cnrT_loFltUV_gain, iso);
    pSelect->sw_cnrT_loFltWgt_slope = interp_cnr_v31(isoGainLow, isoGainHigh, pLowISO->sw_cnrT_loFltWgt_slope, pHighISO->sw_cnrT_loFltWgt_slope, iso);

    pSelect->hw_cnrT_gausFltSigma_en = pNearISO->hw_cnrT_gausFltSigma_en;
    pSelect->sw_cnrT_gausFlt_sigma = interp_cnr_v31(isoGainLow, isoGainHigh, pLowISO->sw_cnrT_gausFlt_sigma, pHighISO->sw_cnrT_gausFlt_sigma, iso);
    pSelect->sw_cnrT_gausFlt_alpha = interp_cnr_v31(isoGainLow, isoGainHigh, pLowISO->sw_cnrT_gausFlt_alpha, pHighISO->sw_cnrT_gausFlt_alpha, iso);
    for(int i = 0; i < 6; i++) {
        pSelect->sw_cnrT_gausFlt_coeff[i] = pNearISO->sw_cnrT_gausFlt_coeff[i];
    }

    for (int i = 0; i < 8; i++) {
        pSelect->sw_cnrT_hiFltVsigma_idx[i] = pNearISO->sw_cnrT_hiFltVsigma_idx[i];
        pSelect->sw_cnrT_hiFlt_vsigma[i] = interp_cnr_v31(isoGainLow, isoGainHigh, pLowISO->sw_cnrT_hiFlt_vsigma[i], pHighISO->sw_cnrT_hiFlt_vsigma[i], iso);
    }
    pSelect->sw_cnrT_hiFltUV_gain = interp_cnr_v31(isoGainLow, isoGainHigh, pLowISO->sw_cnrT_hiFltUV_gain, pHighISO->sw_cnrT_hiFltUV_gain, iso);
    pSelect->sw_cnrT_hiFltCur_wgt = interp_cnr_v31(isoGainLow, isoGainHigh, pLowISO->sw_cnrT_hiFltCur_wgt, pHighISO->sw_cnrT_hiFltCur_wgt, iso);
    pSelect->sw_cnrT_hiFltWgt_minLimit = interp_cnr_v31(isoGainLow, isoGainHigh, pLowISO->sw_cnrT_hiFltWgt_minLimit, pHighISO->sw_cnrT_hiFltWgt_minLimit, iso);
    pSelect->hw_cnrT_hiFltWgt0_mode = pNearISO->hw_cnrT_hiFltWgt0_mode;
    pSelect->sw_cnrT_hiFlt_alpha = interp_cnr_v31(isoGainLow, isoGainHigh, pLowISO->sw_cnrT_hiFlt_alpha, pHighISO->sw_cnrT_hiFlt_alpha, iso);

    pSelect->hw_cnrT_satAdj_offset = interp_cnr_v31(isoGainLow, isoGainHigh, pLowISO->hw_cnrT_satAdj_offset, pHighISO->hw_cnrT_satAdj_offset, iso);
    pSelect->sw_cnrT_satAdj_scale = interp_cnr_v31(isoGainLow, isoGainHigh, pLowISO->sw_cnrT_satAdj_scale, pHighISO->sw_cnrT_satAdj_scale, iso);

    pSelect->sw_cnrT_global_gain = interp_cnr_v31(isoGainLow, isoGainHigh, pLowISO->sw_cnrT_global_gain, pHighISO->sw_cnrT_global_gain, iso);
    pSelect->sw_cnrT_globalGain_alpha = interp_cnr_v31(isoGainLow, isoGainHigh, pLowISO->sw_cnrT_globalGain_alpha, pHighISO->sw_cnrT_globalGain_alpha, iso);
    pSelect->sw_cnrT_localGain_scale = interp_cnr_v31(isoGainLow, isoGainHigh, pLowISO->sw_cnrT_localGain_scale, pHighISO->sw_cnrT_localGain_scale, iso);
    pSelect->sw_cnrT_loFltGlobalSgm_ratio = interp_cnr_v31(isoGainLow, isoGainHigh, pLowISO->sw_cnrT_loFltGlobalSgm_ratio, pHighISO->sw_cnrT_loFltGlobalSgm_ratio, iso);
    pSelect->sw_cnrT_loFltGlobalSgmRto_alpha = interp_cnr_v31(isoGainLow, isoGainHigh, pLowISO->sw_cnrT_loFltGlobalSgmRto_alpha, pHighISO->sw_cnrT_loFltGlobalSgmRto_alpha, iso);

    for (int i = 0; i < 13; i++)
    {
        pSelect->sw_cnrT_gainAdjHiFltSgm_ratio[i] = interp_cnr_v31(isoGainLow, isoGainHigh, pLowISO->sw_cnrT_gainAdjHiFltSgm_ratio[i], pHighISO->sw_cnrT_gainAdjHiFltSgm_ratio[i], iso);
        pSelect->sw_cnrT_gainAdjHiFltCur_wgt[i] = interp_cnr_v31(isoGainLow, isoGainHigh, pLowISO->sw_cnrT_gainAdjHiFltCur_wgt[i], pHighISO->sw_cnrT_gainAdjHiFltCur_wgt[i], iso);
    }

    return ACNRV31_RET_SUCCESS;

}


AcnrV31_result_t cnr_fix_transfer_V31(RK_CNR_Params_V31_Select_t *pSelect, RK_CNR_Fix_V31_t *pFix, AcnrV31_ExpInfo_t *pExpInfo, rk_aiq_cnr_strength_v31_t* pStrength)
{
    LOGI_ANR("%s:(%d) enter \n", __FUNCTION__, __LINE__);

    int i = 0;
    AcnrV31_result_t res = ACNRV31_RET_SUCCESS;
    int tmp = 0;

    if(pSelect == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNRV31_RET_NULL_POINTER;
    }

    if(pFix == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNRV31_RET_NULL_POINTER;
    }

    if(pExpInfo == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNRV31_RET_NULL_POINTER;
    }

    if(pStrength == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNRV31_RET_NULL_POINTER;
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
    pFix->hiflt_wgt0_mode = pSelect->hw_cnrT_hiFltWgt0_mode;
    if (pSelect->sw_cnrT_ds_scaleX == 4 && pSelect->sw_cnrT_ds_scaleY == 4) {
        pFix->thumb_mode = 1;
    } else if (pSelect->sw_cnrT_ds_scaleX == 8 && pSelect->sw_cnrT_ds_scaleY == 4) {
        pFix->thumb_mode = 3;
    } else {
        pFix->thumb_mode = 1;
    }
    pFix->yuv422_mode = 0;
    pFix->exgain_bypass = pSelect->hw_cnrT_exgain_bypass;
    pFix->cnr_en = pSelect->enable;
	pFix->loflt_coeff = (pSelect->hw_cnrT_loFlt_coeff[0]& 0x01 << 0) 
					| ((pSelect->hw_cnrT_loFlt_coeff[1] & 0x01) << 1)
					| ((pSelect->hw_cnrT_loFlt_coeff[2] & 0x01) << 2)
					| ((pSelect->hw_cnrT_loFlt_coeff[2] & 0x01) << 3)
					| ((pSelect->hw_cnrT_loFlt_coeff[1] & 0x01) << 4)
					| ((pSelect->hw_cnrT_loFlt_coeff[0] & 0x01) << 5);
	
    // CNR_EXGAIN
    tmp = pSelect->sw_cnrT_localGain_scale * (1 << RKCNR_V31_GAIN_ISO_FIX_BITS);
    pFix->local_gain_scale = CLIP(tmp, 0, 0xff);
    tmp = pSelect->sw_cnrT_globalGain_alpha * (1 << RKCNR_V31_G_GAIN_ALPHA_FIX_BITS);
    pFix->global_gain_alpha = CLIP(tmp, 0, 0x8);
    tmp = pSelect->sw_cnrT_global_gain * (1 << RKCNR_V31_G_GAIN_FIX_BITS);
    pFix->global_gain = CLIP(tmp, 0, 0x3ff);


    /* CNR_THUMB1 */
    int yuvBit = 10;
    int scale  = (1 << yuvBit) - 1;
    int log2e = (int)(0.8493f * (1 << (RKCNR_V31_log2e + RKCNR_V31_SIGMA_FIX_BIT)));
    int rkcnr_sw_cnrT_loFltUV_gain = ROUND_F((1 << RKCNR_V31_uvgain) * pSelect->sw_cnrT_loFltUV_gain);
    int thumbBFilterSigma = ROUND_F(pSelect->sw_cnrT_loBfFlt_vsigma * scale * fStrength);
    LOGD_ANR("scale:%d thumbBFilterSigma:%d\n", log2e, thumbBFilterSigma);
    thumbBFilterSigma = (int)(0.8493f * (1 << RKCNR_V31_FIX_BIT_INV_SIGMA) / thumbBFilterSigma);
    tmp = thumbBFilterSigma * ((1 << RKCNR_V31_uvgain) - rkcnr_sw_cnrT_loFltUV_gain * 2);
    LOGD_ANR("thumbBFilterSigma:%d sigmaY:%d\n", thumbBFilterSigma, tmp);
    tmp                 = ROUND_INT(tmp, 6);
    pFix->lobfflt_vsigma_y = CLIP(tmp, 0, 0x3fff);
    tmp                 = thumbBFilterSigma * rkcnr_sw_cnrT_loFltUV_gain;
    tmp = ROUND_INT(tmp, 6); // (diff * sigma) >> tmp = (diff * (sigma >> 6))
    pFix->lobfflt_vsigma_uv = CLIP(tmp, 0, 0x3fff);


    /* CNR_THUMB_BF_RATIO */
    tmp = ROUND_F((1 << RKCNR_V31_FIX_BIT_BF_RATIO) * pSelect->sw_cnrT_loBfFlt_alpha * fStrength);
    pFix->lobfflt_alpha = CLIP(tmp, 0, 0x7ff);

    /* CNR_LBF_WEITD */
    for(int i = 0; i < RKCNR_V31_THUMB_BF_RADIUS + 1; i++) {
        tmp = ROUND_F(pSelect->sw_cnrT_thumbBf_coeff[i] * (1 << RKCNR_V31_exp2_lut_y));
        pFix->thumb_bf_coeff[i] = CLIP(tmp, 0, 0xff);
    }

    /* CNR_IIR_PARA1 */
    tmp = ROUND_F((1 << RKCNR_V31_FIX_BIT_SLOPE) * pSelect->sw_cnrT_loFltWgt_slope);
    pFix->loflt_wgt_slope = CLIP(tmp, 0, 0x3ff);

    tmp = ROUND_F(1.2011 * (1 << RKCNR_V31_FIX_BIT_INV_SIGMA) / (pSelect->sw_cnrT_loFlt_vsigma * scale * fStrength));
    int tmptmp = tmp * pFix->loflt_wgt_slope;
    int shiftBit = Math_LOG2(tmptmp) - RKCNR_V31_FIX_BIT_INT_TO_FLOAT;
    LOGD_ANR("tmp:%d tmptmp:%d shiftBit:%d\n", tmp, tmptmp, shiftBit);
    shiftBit = MAX(shiftBit, 0);
    tmp = RKCNR_V31_FIX_BIT_INV_SIGMA - shiftBit;
    if (tmp < 0) {
        tmp = ABS(tmp) + (1 << 5);
    }
    pFix->exp_x_shift_bit = CLIP(tmp, 0, 0x3f);

    tmp = ROUND_F((float)tmptmp / (1 << shiftBit));
    pFix->loflt_vsigma = CLIP(tmp, 0, 0xff);

    tmp = ROUND_F((1 << RKCNR_V31_uvgain) * pSelect->sw_cnrT_loFltUV_gain);
    pFix->loflt_uv_gain = CLIP(tmp, 0, 0xf);

    /* CNR_IIR_PARA2 */
    tmp = ROUND_F((1 << RKCNR_V31_FIX_BIT_IIR_WGT) * pSelect->sw_cnrT_loFltWgt_minThred);
    pFix->loflt_wgt_min_thred = CLIP(tmp, 0, 0x3f);
    tmp = ROUND_F((1 << 3) * pSelect->sw_cnrT_loFltWgt_maxLimit);
    pFix->loflt_wgt_max_limit = CLIP(tmp, 0, 0x7f);

    /* CNR_GAUS_COE */
    float gaus_table[6];
    uint8_t gaus_reg[6];
    if(pSelect->hw_cnrT_gausFltSigma_en) {
        float dis_table_5x5[6] = { 0.0, 1.0, 2.0, 4.0, 5.0, 8.0 };
        float dis_table_5x3[6] = { 0.0, 1.0, 4.0, 1.0, 2.0, 5.0 };
        float sigma = pSelect->sw_cnrT_gausFlt_sigma;
        double e = 2.71828182845905;
        float sumTable = 0;
        if (0 == !pFix->yuv422_mode) {
            for (int k = 0; k < 6; k++) {
                float tmp = pow(e, -dis_table_5x5[k] / 2.0 / sigma / sigma);
                gaus_table[k] = tmp;
            }
        } else {
            for (int k = 0; k < 6; k++) {
                float tmp = pow(e, -dis_table_5x3[k] / 2.0 / sigma / sigma);
                gaus_table[k] = tmp;
            }
        }
    } else {
        for(int k = 0; k < 6; k++) {
            gaus_table[k] = pSelect->sw_cnrT_gausFlt_coeff[k];
        }
    }

    //check gaus params
    float sumTable = 0;

    if (0 == !pFix->yuv422_mode) {
        sumTable = gaus_table[0]
                   + 4 * gaus_table[1]
                   + 4 * gaus_table[2]
                   + 4 * gaus_table[3]
                   + 8 * gaus_table[4]
                   + 4 * gaus_table[5];
    } else {
        sumTable = gaus_table[0]
                   + 2 * gaus_table[1]
                   + 2 * gaus_table[2]
                   + 2 * gaus_table[3]
                   + 4 * gaus_table[4]
                   + 4 * gaus_table[5];
    }

    for (int k = 0; k < 6; k++) {
        gaus_table[k] = gaus_table[k] / sumTable;
        tmp = ROUND_F(gaus_table[k] * (1 << 8));
        gaus_reg[k] = CLIP(tmp, 0, 0x7f);
    }

    int sum_coeff = 0;
    if (0 == !pFix->yuv422_mode) {
        sum_coeff = gaus_reg[0]
                    + 4 * gaus_reg[1]
                    + 4 * gaus_reg[2]
                    + 4 * gaus_reg[3]
                    + 8 * gaus_reg[4]
                    + 4 * gaus_reg[5];
    } else {
        sum_coeff = gaus_reg[0]
                    + 2 * gaus_reg[1]
                    + 2 * gaus_reg[2]
                    + 2 * gaus_reg[3]
                    + 4 * gaus_reg[4]
                    + 4 * gaus_reg[5];
    }
    int offset = (1 << 8) - sum_coeff;
    gaus_reg[0] = gaus_reg[0] + offset;

    for(int i = 0; i < 6; i++) {
#if 1
        tmp = gaus_reg[5 - i];
#else
        tmp = gaus_reg[i];
#endif
        pFix->gaus_flt_coeff[i] = CLIP(tmp, 0, 0x7f);
    }


    /* CNR_GAUS_RATIO */
    tmp = ROUND_F((1 << RKCNR_V31_FIX_BIT_GAUS_RATIO) * pSelect->sw_cnrT_gausFlt_alpha);
    pFix->gaus_flt_alpha = CLIP(tmp, 0, 0x7ff);
    tmp = ROUND_F((1 << RKCNR_V31_exp2_lut_y) * pSelect->sw_cnrT_hiFltWgt_minLimit);
    pFix->hiflt_wgt_min_limit = CLIP(tmp, 0, 0xff);
    tmp = ROUND_F((1 << RKCNR_V31_FIX_BIT_GLOBAL_ALPHA) * pSelect->sw_cnrT_hiFlt_alpha);
    pFix->hiflt_alpha = CLIP(tmp, 0, 0x7ff);

    /* CNR_BF_PARA1 */
    tmp = ROUND_F((1 << RKCNR_V31_uvgain) * pSelect->sw_cnrT_hiFltUV_gain);
    pFix->hiflt_uv_gain = CLIP(tmp, 0, 0x7f);
    // tmp = (int)(log2e / (pSelect->sw_cnrT_hiFlt_vsigma * scale * fStrength));
    // pFix->hiflt_global_vsigma = CLIP(tmp, 0, 0x3ff);
    tmp = ROUND_F((1 << RKCNR_V31_bfRatio) * pSelect->sw_cnrT_hiFltCur_wgt / fStrength);
    pFix->hiflt_cur_wgt = CLIP(tmp, 0, 0xff);

    /* CNR_BF_PARA2 */
    tmp =  pSelect->hw_cnrT_satAdj_offset;
    pFix->adj_offset = CLIP(tmp, 0, 0x1ff);
    tmp = ROUND_F((1 << RKCNR_V31_FIX_BIT_SATURATION) * pSelect->sw_cnrT_satAdj_scale);
    pFix->adj_scale = CLIP(tmp, 0, 0x7fff);

    /* CNR_SIGMA */
    for(int i = 0; i < 13; i++) {
        tmp = ROUND_F(pSelect->sw_cnrT_gainAdjHiFltSgm_ratio[i] * (1 << RKCNR_V31_sgmRatio));
        pFix->sgm_ratio[i] = CLIP(tmp, 0, 0xff);
    }

    /* CNR_IIR_GLOBAL_GAIN */
    tmp = pSelect->sw_cnrT_loFltGlobalSgmRto_alpha * (1 << RKCNR_V31_G_GAIN_ALPHA_FIX_BITS);
    pFix->loflt_global_sgm_ratio_alpha = CLIP(tmp, 0, 0x8);
    tmp = pSelect->sw_cnrT_loFltGlobalSgm_ratio * (1 << RKCNR_V31_sgmRatio);
    pFix->loflt_global_sgm_ratio = CLIP(tmp, 0, 0xff);

    /* CNR_SIGMA */
    for(int i = 0; i < 13; i++) {
        tmp = ROUND_F(pSelect->sw_cnrT_gainAdjHiFltCur_wgt[i] * (1 << RKCNR_V31_bfRatio));
        pFix->cur_wgt[i] = CLIP(tmp, 0, 0xff);
    }

    /* CNR_SIGMA */
    for(int i = 0; i < 8; i++) {
        tmp = ROUND_F(pSelect->sw_cnrT_hiFltVsigma_idx[i]);
        pFix->hiflt_vsigma_idx[i] = CLIP(tmp, 0, 0x3ff);
        tmp = log2e / (pSelect->sw_cnrT_hiFlt_vsigma[i] * scale);
        pFix->hiflt_vsigma[i] = CLIP(tmp, 0, 0x3fff);
    }
    cnr_fix_printf_V31(pFix);

    LOGI_ANR("%s:(%d) exit \n", __FUNCTION__, __LINE__);

    return ACNRV31_RET_SUCCESS;
}


AcnrV31_result_t cnr_fix_printf_V31(RK_CNR_Fix_V31_t  * pFix)
{
    int i = 0;
    LOGI_ANR("%s:(%d) enter \n", __FUNCTION__, __LINE__);

    AcnrV31_result_t res = ACNRV31_RET_SUCCESS;

    if(pFix == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNRV31_RET_NULL_POINTER;
    }


    //CNR_CTRL (0x0000)
    LOGD_ANR("(0x0000) loflt_coeff:0x%x hiflt_wgt0_mode:0x%x thumb_mode:0x%x yuv422_mode:0x%x exgain_bypass:0x%x cnr_en:0x%x\n",
             pFix->loflt_coeff,
             pFix->hiflt_wgt0_mode,
             pFix->thumb_mode,
             pFix->yuv422_mode,
             pFix->exgain_bypass,
             pFix->cnr_en);

    // CNR_EXGAIN  (0x0004)
    LOGD_ANR("(0x0004) global_gain:0x%x  global_gain_alpha:0x%x  local_gain_scale:0x%x \n",
             pFix->global_gain,
             pFix->global_gain_alpha,
             pFix->local_gain_scale);

    // CNR_THUMB1  (0x0008)
    LOGD_ANR("(0x0008) lobfflt_vsigma_uv:0x%x lobfflt_vsigma_y:0x%x \n",
             pFix->lobfflt_vsigma_uv,
             pFix->lobfflt_vsigma_y);

    // CNR_THUMB_BF_RATIO (0x000c)
    LOGD_ANR("(0x000c) lobfflt_alpha:0x%x \n",
             pFix->lobfflt_alpha);

    // CNR_LBF_WEITD (0x0010)
    LOGD_ANR("(0x0010) thumb_bf_coeff:0x%x  0x%x  0x%x  0x%x \n",
             pFix->thumb_bf_coeff[0],
             pFix->thumb_bf_coeff[1],
             pFix->thumb_bf_coeff[2],
             pFix->thumb_bf_coeff[3]);

    // CNR_IIR_PARA1 (0x0014)
    LOGD_ANR("(0x0014) loflt_uv_gain:0x%x  loflt_vsigma:0x%x  exp_x_shift_bit:0x%x  loflt_wgt_slope:0x%x \n",
             pFix->loflt_uv_gain,
             pFix->loflt_vsigma,
             pFix->exp_x_shift_bit,
             pFix->loflt_wgt_slope);


    // CNR_IIR_PARA2 (0x0018)
    LOGD_ANR("(0x0018) loflt_wgt_min_thred:0x%x loflt_wgt_max_limit:0x%x \n",
             pFix->loflt_wgt_min_thred,
             pFix->loflt_wgt_max_limit);

    // CNR_GAUS_COE (0x001c - 0x0020)
    for(int i = 0; i < 6; i++) {
        LOGD_ANR("(0x001c - 0x0020) gaus_flt_coeff[%d]:0x%x \n",
                 i, pFix->gaus_flt_coeff[i]);
    }

    // CNR_GAUS_RATIO  (0x0024)
    LOGD_ANR("(0x0024) gaus_flt_alpha:0x%x  hiflt_wgt_min_limit:0x%x  hiflt_alpha:0x%x \n",
             pFix->gaus_flt_alpha,
             pFix->hiflt_wgt_min_limit,
             pFix->hiflt_alpha);

    // CNR_BF_PARA1  (0x0028)
    LOGD_ANR("(0x0028) hiflt_uv_gain:0x%x  hiflt_global_vsigma:0x%x  hiflt_cur_wgt:0x%x \n",
             pFix->hiflt_uv_gain,
             pFix->hiflt_global_vsigma,
             pFix->hiflt_cur_wgt);

    // CNR_BF_PARA2 (0x002c)
    LOGD_ANR("(0x002c) adj_offset:0x%x adj_scale:0x%x \n",
             pFix->adj_offset,
             pFix->adj_scale);

    // CNR_SIGMA (0x0030 - 0x003c)
    for(int i = 0; i < 13; i++) {
        LOGD_ANR("(0x0030 - 0x003c) sgm_ratio[%d]:0x%x \n", i, pFix->sgm_ratio[i]);
    }

    // CNR_IIR_GLOBAL_GAIN (0x0040)
    LOGD_ANR("(0x0040) loflt_global_sgm_ratio:0x%x loflt_global_sgm_ratio_alpha:0x%x \n",
             pFix->loflt_global_sgm_ratio,
             pFix->loflt_global_sgm_ratio_alpha);

    // CNR_SIGMA (0x0044 - 0x0050)
    for(int i = 0; i < 13; i++) {
        LOGD_ANR("(0x0044 - 0x0050) cur_wgt[%d]:0x%x \n", i, pFix->cur_wgt[i]);
    }

    // CNR_SIGMA (0x0054 - 0x005c)
    for(int i = 0; i < 8; i++) {
        LOGD_ANR("(0x0054 - 0x005c) hiflt_vsigma_idx[%d]:0x%x \n", i, pFix->hiflt_vsigma_idx[i]);
    }

    // CNR_SIGMA (0x0060 - 0x006c)
    for(int i = 0; i < 8; i++) {
        LOGD_ANR("(0x0060 - 0x006c) hiflt_vsigma[%d]:0x%x \n", i, pFix->hiflt_vsigma[i]);
    }

    return res;
}

AcnrV31_result_t cnr_get_setting_by_name_json_V31(CalibDbV2_CNRV31_t* pCalibdbV2, char* name,
        int* tuning_idx) {
    int i                = 0;
    AcnrV31_result_t res = ACNRV31_RET_SUCCESS;

    if (pCalibdbV2 == NULL || name == NULL || tuning_idx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNRV31_RET_NULL_POINTER;
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

AcnrV31_result_t cnr_init_params_json_V31(RK_CNR_Params_V31_t *pParams, CalibDbV2_CNRV31_t *pCalibdbV2, int tuning_idx)
{
    AcnrV31_result_t res = ACNRV31_RET_SUCCESS;
    CalibDbV2_CNRV31_T_ISO_t *pTuningISO = NULL;

    if(pParams == NULL || pCalibdbV2 == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNRV31_RET_NULL_POINTER;
    }

    pParams->enable = pCalibdbV2->TuningPara.enable;

    for(int i = 0; i < pCalibdbV2->TuningPara.Setting[tuning_idx].Tuning_ISO_len && i < RK_CNR_V31_MAX_ISO_NUM; i++ ) {
        pTuningISO = &pCalibdbV2->TuningPara.Setting[tuning_idx].Tuning_ISO[i];

        pParams->iso[i] = pTuningISO->iso;
        pParams->CnrParamsISO[i].enable = pCalibdbV2->TuningPara.enable;
        pParams->CnrParamsISO[i].hw_cnrT_exgain_bypass = pTuningISO->hw_cnrT_exgain_bypass;
        pParams->CnrParamsISO[i].sw_cnrT_ds_scaleX = pTuningISO->sw_cnrT_ds_scaleX;
        pParams->CnrParamsISO[i].sw_cnrT_ds_scaleY = pTuningISO->sw_cnrT_ds_scaleY;

        pParams->CnrParamsISO[i].sw_cnrT_loBfFlt_vsigma = pTuningISO->sw_cnrT_loBfFlt_vsigma;
        pParams->CnrParamsISO[i].sw_cnrT_loBfFlt_alpha = pTuningISO->sw_cnrT_loBfFlt_alpha;

        for(int j = 0; j < 3; j++) {
            pParams->CnrParamsISO[i].hw_cnrT_loFlt_coeff[j] = pTuningISO->hw_cnrT_loFlt_coeff[j];
        }

        pParams->CnrParamsISO[i].sw_cnrT_loFlt_vsigma = pTuningISO->sw_cnrT_loFlt_vsigma;
        pParams->CnrParamsISO[i].sw_cnrT_loFltWgt_maxLimit = pTuningISO->sw_cnrT_loFltWgt_maxLimit;
        pParams->CnrParamsISO[i].sw_cnrT_loFltWgt_minThred = pTuningISO->sw_cnrT_loFltWgt_minThred;
        pParams->CnrParamsISO[i].sw_cnrT_loFltUV_gain = pTuningISO->sw_cnrT_loFltUV_gain;
        pParams->CnrParamsISO[i].sw_cnrT_loFltWgt_slope = pTuningISO->sw_cnrT_loFltWgt_slope;

        pParams->CnrParamsISO[i].sw_cnrT_gausFlt_alpha = pTuningISO->sw_cnrT_gausFlt_alpha;

        for(int j = 0; j < 8; j++) {
            pParams->CnrParamsISO[i].sw_cnrT_hiFltVsigma_idx[j] = pTuningISO->sw_cnrT_hiFltVsigma_idx[j];
            pParams->CnrParamsISO[i].sw_cnrT_hiFlt_vsigma[j] = pTuningISO->sw_cnrT_hiFlt_vsigma[j];
        }
        pParams->CnrParamsISO[i].sw_cnrT_hiFltUV_gain = pTuningISO->sw_cnrT_hiFltUV_gain;
        pParams->CnrParamsISO[i].sw_cnrT_hiFltCur_wgt = pTuningISO->sw_cnrT_hiFltCur_wgt;
        pParams->CnrParamsISO[i].sw_cnrT_hiFltWgt_minLimit = pTuningISO->sw_cnrT_hiFltWgt_minLimit;
        pParams->CnrParamsISO[i].hw_cnrT_hiFltWgt0_mode = pTuningISO->hw_cnrT_hiFltWgt0_mode;
        pParams->CnrParamsISO[i].sw_cnrT_hiFlt_alpha = pTuningISO->sw_cnrT_hiFlt_alpha;

        pParams->CnrParamsISO[i].hw_cnrT_satAdj_offset = pTuningISO->hw_cnrT_satAdj_offset;
        pParams->CnrParamsISO[i].sw_cnrT_satAdj_scale = pTuningISO->sw_cnrT_satAdj_scale;

        pParams->CnrParamsISO[i].sw_cnrT_global_gain = pTuningISO->sw_cnrT_global_gain;
        pParams->CnrParamsISO[i].sw_cnrT_globalGain_alpha = pTuningISO->sw_cnrT_globalGain_alpha;
        pParams->CnrParamsISO[i].sw_cnrT_localGain_scale = pTuningISO->sw_cnrT_localGain_scale;
        pParams->CnrParamsISO[i].sw_cnrT_loFltGlobalSgm_ratio = pTuningISO->sw_cnrT_loFltGlobalSgm_ratio;
        pParams->CnrParamsISO[i].sw_cnrT_loFltGlobalSgmRto_alpha = pTuningISO->sw_cnrT_loFltGlobalSgmRto_alpha;

        for(int j = 0; j < RKCNR_V31_SGM_ADJ_TABLE_LEN; j++) {
            pParams->CnrParamsISO[i].sw_cnrT_gainAdjHiFltSgm_ratio[j] = pTuningISO->sw_cnrT_gainAdjHiFltSgm_ratio[j];
            pParams->CnrParamsISO[i].sw_cnrT_gainAdjHiFltCur_wgt[j] = pTuningISO->sw_cnrT_gainAdjHiFltCur_wgt[j];
        }

        for(int j = 0; j < 4; j++) {
            pParams->CnrParamsISO[i].sw_cnrT_thumbBf_coeff[j] = pTuningISO->sw_cnrT_thumbBf_coeff[j];
        }

        pParams->CnrParamsISO[i].hw_cnrT_gausFltSigma_en = pTuningISO->hw_cnrT_gausFltSigma_en;
        pParams->CnrParamsISO[i].sw_cnrT_gausFlt_sigma = pTuningISO->sw_cnrT_gausFlt_sigma;
        for(int j = 0; j < 6; j++) {
            pParams->CnrParamsISO[i].sw_cnrT_gausFlt_coeff[j] = pTuningISO->sw_cnrT_gausFlt_coeff[j];
        }
    }

    return ACNRV31_RET_SUCCESS;

}

AcnrV31_result_t cnr_config_setting_param_json_V31(RK_CNR_Params_V31_t *pParams, CalibDbV2_CNRV31_t *pCalibdbV2, char* param_mode, char * snr_name)
{
    AcnrV31_result_t res = ACNRV31_RET_SUCCESS;
    int tuning_idx;

    if(pParams == NULL || pCalibdbV2 == NULL || param_mode == NULL || snr_name == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNRV31_RET_NULL_POINTER;
    }

    res = cnr_get_setting_by_name_json_V31(pCalibdbV2, snr_name, &tuning_idx);
    if(res != ACNRV31_RET_SUCCESS) {
        LOGW_ANR("%s(%d): error!!!  can't find setting in iq files, use 0 instead\n", __FUNCTION__, __LINE__);
    }

    res = cnr_init_params_json_V31(pParams, pCalibdbV2, tuning_idx);
    pParams->enable = pCalibdbV2->TuningPara.enable;
    return res;

}


RKAIQ_END_DECLARE




