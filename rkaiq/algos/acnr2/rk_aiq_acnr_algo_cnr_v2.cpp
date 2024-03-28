#include "rk_aiq_acnr_algo_cnr_v2.h"


RKAIQ_BEGIN_DECLARE


float interp_cnr_v2(int ISO_low, int ISO_high, float value_low, float value_high, int ISO, float value)
{
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


AcnrV2_result_t cnr_select_params_by_ISO_V2(RK_CNR_Params_V2_t *pParams, RK_CNR_Params_V2_Select_t *pSelect, AcnrV2_ExpInfo_t *pExpInfo)
{
    AcnrV2_result_t res = ACNRV2_RET_SUCCESS;
    int iso = 50;
    if(pParams == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNRV2_RET_NULL_POINTER;
    }

    if(pSelect == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNRV2_RET_NULL_POINTER;
    }

    if(pExpInfo == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNRV2_RET_NULL_POINTER;
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
    int max_iso_step = RK_CNR_V2_MAX_ISO_NUM;

#ifndef RK_SIMULATOR_HW
    for (int i = 0; i < max_iso_step - 1 ; i++) {
        if (iso >= pParams->iso[i]  &&  iso <= pParams->iso[i + 1]) {
            isoGainLow = pParams->iso[i] ;
            isoGainHigh = pParams->iso[i + 1];
            isoIndexLow = i;
            isoIndexHigh = i + 1;
            //isoIndex = isoIndexLow;
            isoIndex = (iso - pParams->iso[i]) < (pParams->iso[i + 1] - iso) ? isoIndexLow : isoIndexHigh;
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

    pExpInfo->isoHigh = pParams->iso[isoIndexHigh];
    pExpInfo->isoLow = pParams->iso[isoIndexLow];

    LOGD_ANR("%s:%d iso:%d high:%d low:%d \n",
             __FUNCTION__, __LINE__,
             iso, isoGainHigh, isoGainLow);

    pSelect->enable = pParams->enable;
    // bypass
    pSelect->hf_bypass = pParams->hf_bypass[isoIndex];
    pSelect->lf_bypass = pParams->lf_bypass[isoIndex];

    // gain
    pSelect->global_gain = interp_cnr_v2(isoGainLow, isoGainHigh, pParams->global_gain[isoIndexLow], pParams->global_gain[isoIndexHigh], iso, pSelect->global_gain);
    pSelect->global_gain_alpha = interp_cnr_v2(isoGainLow, isoGainHigh, pParams->global_gain_alpha[isoIndexLow], pParams->global_gain_alpha[isoIndexHigh], iso, pSelect->global_gain_alpha);
    pSelect->local_gain_scale = interp_cnr_v2(isoGainLow, isoGainHigh, pParams->local_gain_scale[isoIndexLow], pParams->local_gain_scale[isoIndexHigh], iso, pSelect->local_gain_scale);

    for (int i = 0; i < 13; i++)
    {
        pSelect->gain_adj_strength_ratio[i] = interp_cnr_v2(isoGainLow, isoGainHigh, pParams->gain_adj_strength_ratio[isoIndexLow][i], pParams->gain_adj_strength_ratio[isoIndexHigh][i], iso, pSelect->gain_adj_strength_ratio[i]);
    }
    //
    pSelect->color_sat_adj = interp_cnr_v2(isoGainLow, isoGainHigh, pParams->color_sat_adj[isoIndexLow], pParams->color_sat_adj[isoIndexHigh], iso, pSelect->color_sat_adj);
    pSelect->color_sat_adj_alpha = interp_cnr_v2(isoGainLow, isoGainHigh, pParams->color_sat_adj_alpha[isoIndexLow], pParams->color_sat_adj_alpha[isoIndexHigh], iso, pSelect->color_sat_adj_alpha);

    // step1
    // median filter
    pSelect->hf_spikes_reducion_strength = interp_cnr_v2(isoGainLow, isoGainHigh, pParams->hf_spikes_reducion_strength[isoIndexLow], pParams->hf_spikes_reducion_strength[isoIndexHigh], iso, pSelect->hf_spikes_reducion_strength);

    // bilateral filter
    pSelect->hf_denoise_strength = interp_cnr_v2(isoGainLow, isoGainHigh, pParams->hf_denoise_strength[isoIndexLow], pParams->hf_denoise_strength[isoIndexHigh], iso, pSelect->hf_denoise_strength);
    pSelect->hf_color_sat = interp_cnr_v2(isoGainLow, isoGainHigh, pParams->hf_color_sat[isoIndexLow], pParams->hf_color_sat[isoIndexHigh], iso, pSelect->hf_color_sat);
    pSelect->hf_denoise_alpha = interp_cnr_v2(isoGainLow, isoGainHigh, pParams->hf_denoise_alpha[isoIndexLow], pParams->hf_denoise_alpha[isoIndexHigh], iso, pSelect->hf_denoise_alpha);
    pSelect->hf_bf_wgt_clip = pParams->hf_bf_wgt_clip[isoIndex];

    // step2
    // median filter
    pSelect->thumb_spikes_reducion_strength = interp_cnr_v2(isoGainLow, isoGainHigh, pParams->thumb_spikes_reducion_strength[isoIndexLow], pParams->thumb_spikes_reducion_strength[isoIndexHigh], iso, pSelect->thumb_spikes_reducion_strength);

    // bilateral filter
    pSelect->thumb_denoise_strength = interp_cnr_v2(isoGainLow, isoGainHigh, pParams->thumb_denoise_strength[isoIndexLow], pParams->thumb_denoise_strength[isoIndexHigh], iso, pSelect->thumb_denoise_strength);
    pSelect->thumb_color_sat = interp_cnr_v2(isoGainLow, isoGainHigh, pParams->thumb_color_sat[isoIndexLow], pParams->thumb_color_sat[isoIndexHigh], iso, pSelect->thumb_color_sat);

    // step3
    // bilateral filter
    pSelect->lf_denoise_strength = interp_cnr_v2(isoGainLow, isoGainHigh, pParams->lf_denoise_strength[isoIndexLow], pParams->lf_denoise_strength[isoIndexHigh], iso, pSelect->lf_denoise_strength);
    pSelect->lf_color_sat = interp_cnr_v2(isoGainLow, isoGainHigh, pParams->lf_color_sat[isoIndexLow], pParams->lf_color_sat[isoIndexHigh], iso, pSelect->lf_color_sat);
    pSelect->lf_denoise_alpha = interp_cnr_v2(isoGainLow, isoGainHigh, pParams->lf_denoise_alpha[isoIndexLow], pParams->lf_denoise_alpha[isoIndexHigh], iso, pSelect->lf_denoise_alpha);

    // bilateral filter kernels
    memcpy(pSelect->kernel_5x5, pParams->kernel_5x5, sizeof(float) * 5);

    return ACNRV2_RET_SUCCESS;

}


AcnrV2_result_t cnr_fix_transfer_V2(RK_CNR_Params_V2_Select_t *pSelect, RK_CNR_Fix_V2_t *pFix, AcnrV2_ExpInfo_t *pExpInfo, rk_aiq_cnr_strength_v2_t *pStrength)
{
    LOGI_ANR("%s:(%d) enter \n", __FUNCTION__, __LINE__);

    int i = 0;
    AcnrV2_result_t res = ACNRV2_RET_SUCCESS;
    int tmp = 0;

    if(pSelect == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNRV2_RET_NULL_POINTER;
    }

    if(pFix == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNRV2_RET_NULL_POINTER;
    }

    if(pExpInfo == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNRV2_RET_NULL_POINTER;
    }

    if(pStrength == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNRV2_RET_NULL_POINTER;
    }

    float fStrength = 1.0;

    if(pStrength->strength_enable) {
        fStrength = pStrength->percent;
    }

    if(fStrength <= 0.0) {
        fStrength = 0.000001;
    }

    LOGD_ANR("strength_enable:%d percent:%f fStrength:%f\n",
             pStrength->strength_enable,
             pStrength->percent,
             fStrength);
    // fix bit : RK_CNR_V2_log2e + RK_CNR_V2_SIGMA_FIX_BIT, msigma = (1 / sigma) * (1 << RK_CNR_V2_SIGMA_FIX_BIT) * log2e * (1 << RK_CNR_V2_log2e);
    int log2e = (int)(0.8493f * (1 << (RK_CNR_V2_log2e + RK_CNR_V2_SIGMA_FIX_BIT)));

    //ISP_CNR_2800_CTRL
    pFix->cnr_thumb_mix_cur_en = 0;
    pFix->cnr_exgain_bypass = 0;
    pFix->cnr_hq_bila_bypass = pSelect->hf_bypass;
    pFix->cnr_lq_bila_bypass = pSelect->lf_bypass;
    pFix->cnr_en_i = pSelect->enable;

    // ISP_CNR_2800_EXGAIN
    tmp = pSelect->global_gain * (1 << RKCNR_V2_G_GAIN_FIX_BITS);
    pFix->cnr_global_gain = CLIP(tmp, 0, 1023);
    tmp = pSelect->global_gain_alpha * (1 << RKCNR_V2_G_GAIN_ALPHA_FIX_BITS);
    if (pFix->cnr_exgain_bypass == 1)
    {
        tmp = pSelect->global_gain_alpha  = 1.0 * (1 << RKCNR_V2_G_GAIN_ALPHA_FIX_BITS);
    }
    pFix->cnr_global_gain_alpha = CLIP(tmp, 0, 8);


    // ISP_CNR_2800_GAIN_PARA
    tmp = pSelect->local_gain_scale * (1 << RKCNR_V2_GAIN_ISO_FIX_BITS);
    pFix->cnr_gain_iso = CLIP(tmp, 0, 128);
    tmp = (int)(pSelect->color_sat_adj_alpha * (1 << RK_CNR_V2_offset));
    pFix->cnr_gain_offset = CLIP(tmp, 0, 16);
    tmp = (int)(pSelect->color_sat_adj * (1 << RK_CNR_V2_ratio));
    pFix->cnr_gain_1sigma = CLIP(tmp, 0, 255);

    // ISP_CNR_2800_GAIN_UV_PARA
    tmp = (int)(pSelect->hf_color_sat / fStrength * (1 << RK_CNR_V2_uvgain));
    pFix->cnr_gain_uvgain0 = CLIP(tmp, 0, (1 << 7) - 1);
    tmp = (int)(pSelect->lf_color_sat / fStrength * (1 << RK_CNR_V2_uvgain));
    pFix->cnr_gain_uvgain1 = CLIP(tmp, 0, (1 << 7) - 1);

    // ISP_CNR_2800_LMED3
    tmp = (int)(pSelect->thumb_spikes_reducion_strength * (1 << RK_CNR_V2_medRatio));
    pFix->cnr_lmed3_alpha = CLIP(tmp, 0, 16);

    // ISP_CNR_2800_LBF5_GAIN
    int rkcnr_thumb_denoise_strength   = (int)(log2e / pSelect->thumb_denoise_strength / fStrength);
    int rkcnr_thumb_color_sat   = (int)(pSelect->thumb_color_sat / fStrength * (1 << RK_CNR_V2_uvgain));
    int tmpBit = 10 + RK_CNR_V2_log2e + RK_CNR_V2_uvgain - 6;
    int sgmRatio = 1 << RK_CNR_V2_sgmRatio;
    int ky = sgmRatio * (1 << RK_CNR_V2_uvgain) >> RK_CNR_V2_sgmRatio;
    int kuv = sgmRatio * rkcnr_thumb_color_sat >> RK_CNR_V2_sgmRatio;
    ky = MIN2(ky, (1 << (4 + RK_CNR_V2_uvgain)) - 1);
    kuv = MIN2(kuv, (1 << (4 + RK_CNR_V2_uvgain)) - 1);
    tmp = ((rkcnr_thumb_denoise_strength * ky + (1 << (tmpBit - 1))) >> tmpBit);
    pFix->cnr_lbf5_gain_y = CLIP(tmp, 0, 15);
    tmp = ((rkcnr_thumb_denoise_strength * kuv + (1 << (tmpBit - 1))) >> tmpBit);
    pFix->cnr_lbf5_gain_c = CLIP(tmp, 0, 63);

    // ISP_CNR_2800_LBF5_WEITD0_3
    // bilateral filter kernels
    for (i = 0; i < 5; i++) {
        tmp = (int)(pSelect->kernel_5x5[i] * (1 << RK_CNR_V2_kernels));
        pFix->cnr_lbf5_weit_d[i] = CLIP(tmp, 0, 128);
    }

    // ISP_CNR_2800_HMED3
    tmp = (int)(pSelect->hf_spikes_reducion_strength * (1 << RK_CNR_V2_medRatio));
    pFix->cnr_hmed3_alpha = CLIP(tmp, 0, 16);

    // ISP_CNR_2800_HBF5
    tmp = (int)(log2e / pSelect->hf_denoise_strength / fStrength);
    pFix->cnr_hbf5_sigma    = CLIP(tmp, 0, (1 << 13) - 1);
    tmp = pSelect->hf_bf_wgt_clip;
    pFix->cnr_hbf5_min_wgt = CLIP(tmp, 0, (1 << 8) - 1);
    tmp = (int)(pSelect->hf_denoise_alpha / fStrength  * (1 << RK_CNR_V2_bfRatio));
    pFix->cnr_hbf5_weit_src = CLIP(tmp, 0, 128);

    // ISP_CNR_2800_LBF3
    // step3
    // bilateral filter
    tmp = (int)(log2e / pSelect->lf_denoise_strength / fStrength);
    pFix->cnr_lbf3_sigma = CLIP(tmp, 0, (1 << 13) - 1);
    tmp = (int)(pSelect->lf_denoise_alpha / fStrength * (1 << RK_CNR_V2_bfRatio));
    pFix->cnr_lbf5_weit_src = CLIP(tmp, 0, 128);


    //sigma
    for(i = 0; i < 13; i++) {
        tmp = pSelect->gain_adj_strength_ratio[i];
        pFix->cnr_sigma_y[i] = CLIP(tmp, 0, (1 << 8) - 1);
    }

    cnr_fix_printf_V2(pFix);

    LOGI_ANR("%s:(%d) exit \n", __FUNCTION__, __LINE__);

    return ACNRV2_RET_SUCCESS;
}


AcnrV2_result_t cnr_fix_printf_V2(RK_CNR_Fix_V2_t  * pFix)
{
    int i = 0;
    LOGI_ANR("%s:(%d) enter \n", __FUNCTION__, __LINE__);

    AcnrV2_result_t res = ACNRV2_RET_SUCCESS;

    if(pFix == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNRV2_RET_NULL_POINTER;
    }

    //ISP_CNR_2800_CTRL (0x0000)
    LOGD_ANR("(0x0000) thumb_mix_cur_en:0x%x lq_bila_bypass:0x%x hq_bila_bypass:0x%x exgain_bypass:0x%x cnr_en_i:0x%x\n",
             pFix->cnr_thumb_mix_cur_en,
             pFix->cnr_lq_bila_bypass,
             pFix->cnr_hq_bila_bypass,
             pFix->cnr_exgain_bypass,
             pFix->cnr_en_i);

    // ISP_CNR_2800_EXGAIN  (0x0004)
    LOGD_ANR("(0x0004) cnr_global_gain_alpha:0x%x  cnr_global_gain:0x%x \n",
             pFix->cnr_global_gain_alpha,
             pFix->cnr_global_gain);

    // ISP_CNR_2800_GAIN_PARA  (0x0008)
    LOGD_ANR("(0x0008) gain_iso:0x%x cnr_gain_offset:0x%x gain_1sigma:0x%x \n",
             pFix->cnr_gain_iso,
             pFix->cnr_gain_offset,
             pFix->cnr_gain_1sigma);

    // ISP_CNR_2800_GAIN_UV_PARA (0x000c)
    LOGD_ANR("(0x000c) cnr_gain_uvgain1:0x%x gain_uvgain0:0x%x \n",
             pFix->cnr_gain_uvgain1,
             pFix->cnr_gain_uvgain0);

    // ISP_CNR_2800_LMED3 (0x0010)
    LOGD_ANR("(0x0010) lmed3_alpha:0x%x \n",
             pFix->cnr_lmed3_alpha);

    // ISP_CNR_2800_LBF5_GAIN (0x0014)
    LOGD_ANR("(0x0014) lbf5_gain_y:0x%x lbf5_gain_c:0x%x \n",
             pFix->cnr_lbf5_gain_y,
             pFix->cnr_lbf5_gain_c);

    // ISP_CNR_2800_LBF5_WEITD0_4 (0x0018 - 0x001c)
    for(int i = 0; i < 5; i++) {
        LOGD_ANR("(0x0018 - 0x001c) lbf5_weit_d[%d]:0x%x \n",
                 i, pFix->cnr_lbf5_weit_d[i]);
    }

    // ISP_CNR_2800_HMED3 (0x0020)
    LOGD_ANR("(0x0020) hmed3_alpha:0x%x \n",
             pFix->cnr_hmed3_alpha);

    // ISP_CNR_2800_HBF5  (0x0024)
    LOGD_ANR("(0x0024) hbf5_weit_src:0x%x hbf5_min_wgt:0x%x hbf5_sigma:0x%x \n",
             pFix->cnr_hbf5_weit_src,
             pFix->cnr_hbf5_min_wgt,
             pFix->cnr_hbf5_sigma);

    // ISP_CNR_2800_LBF3  (0x0028)
    LOGD_ANR("(0x0028) lbf5_weit_src:0x%x lbf3_sigma:0x%x \n",
             pFix->cnr_lbf5_weit_src,
             pFix->cnr_lbf3_sigma);

    // ISP_CNR_2800_LBF5_WEITD0_4 (0x002c - 0x0038)
    for(i = 0; i < 13; i++) {
        LOGD_ANR("(0x002c - 0x0038) cnr_sigma_y[%d]:0x%x \n",
                 i, pFix->cnr_sigma_y[i]);
    }

    LOGD_ANR("%s:(%d) exit \n", __FUNCTION__, __LINE__);

    return ACNRV2_RET_SUCCESS;
}




AcnrV2_result_t cnr_get_setting_by_name_json_V2(CalibDbV2_CNRV2_t *pCalibdbV2, char *name, int *tuning_idx)
{
    int i = 0;
    AcnrV2_result_t res = ACNRV2_RET_SUCCESS;

    if(pCalibdbV2 == NULL || name == NULL || tuning_idx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNRV2_RET_NULL_POINTER;
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

    LOGD_ANR("%s:%d snr_name:%s  snr_idx:%d i:%d \n",
             __FUNCTION__, __LINE__,
             name, *tuning_idx, i);

    return res;
}



AcnrV2_result_t cnr_init_params_json_V2(RK_CNR_Params_V2_t *pParams, CalibDbV2_CNRV2_t *pCalibdbV2, int tuning_idx)
{
    AcnrV2_result_t res = ACNRV2_RET_SUCCESS;
    CalibDbV2_CNRV2_T_Set_ISO_t *pTuningISO = NULL;

    if(pParams == NULL || pCalibdbV2 == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNRV2_RET_NULL_POINTER;
    }

    pParams->enable = pCalibdbV2->TuningPara.enable;

    for(int i = 0; i < pCalibdbV2->TuningPara.Setting[tuning_idx].Tuning_ISO_len && i < RK_CNR_V2_MAX_ISO_NUM; i++ ) {
        pTuningISO = &pCalibdbV2->TuningPara.Setting[tuning_idx].Tuning_ISO[i];

        pParams->iso[i] = pTuningISO->iso;
        pParams->hf_bypass[i] = pTuningISO->hf_bypass;
        pParams->lf_bypass[i] = pTuningISO->lf_bypass;
        pParams->global_gain[i] = pTuningISO->global_gain;
        pParams->global_gain_alpha[i] = pTuningISO->global_gain_alpha;
        pParams->local_gain_scale[i] = pTuningISO->local_gain_scale;
        pParams->color_sat_adj[i] = pTuningISO->color_sat_adj;
        for(int j = 0; j < RKCNR_V2_SGM_ADJ_TABLE_LEN; j++) {
            pParams->gain_adj_strength_ratio[i][j] = pTuningISO->gain_adj_strength_ratio[j];
        }
        pParams->color_sat_adj_alpha[i] = pTuningISO->color_sat_adj_alpha;
        pParams->hf_spikes_reducion_strength[i] = pTuningISO->hf_spikes_reducion_strength;
        pParams->hf_denoise_strength[i] = pTuningISO->hf_denoise_strength;
        pParams->hf_color_sat[i] = pTuningISO->hf_color_sat;
        pParams->hf_denoise_alpha[i] = pTuningISO->hf_denoise_alpha;
        pParams->hf_bf_wgt_clip[i] = pTuningISO->hf_bf_wgt_clip;
        pParams->thumb_spikes_reducion_strength[i] = pTuningISO->thumb_spikes_reducion_strength;
        pParams->thumb_denoise_strength[i] = pTuningISO->thumb_denoise_strength;
        pParams->thumb_color_sat[i] = pTuningISO->thumb_color_sat;
        pParams->lf_denoise_strength[i] = pTuningISO->lf_denoise_strength;
        pParams->lf_color_sat[i] = pTuningISO->lf_color_sat;
        pParams->lf_denoise_alpha[i] = pTuningISO->lf_denoise_alpha;
    }


    memcpy(pParams->kernel_5x5, pCalibdbV2->TuningPara.Kernel_Coeff.kernel_5x5, sizeof(float) * 5);

    return ACNRV2_RET_SUCCESS;

}

AcnrV2_result_t cnr_config_setting_param_json_V2(RK_CNR_Params_V2_t *pParams, CalibDbV2_CNRV2_t *pCalibdbV2, char* param_mode, char * snr_name)
{
    AcnrV2_result_t res = ACNRV2_RET_SUCCESS;
    int tuning_idx;

    if(pParams == NULL || pCalibdbV2 == NULL || param_mode == NULL || snr_name == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNRV2_RET_NULL_POINTER;
    }

    res = cnr_get_setting_by_name_json_V2(pCalibdbV2, snr_name, &tuning_idx);
    if(res != ACNRV2_RET_SUCCESS) {
        LOGW_ANR("%s(%d): error!!!  can't find setting in iq files, use 0 instead\n", __FUNCTION__, __LINE__);
    }

    res = cnr_init_params_json_V2(pParams, pCalibdbV2, tuning_idx);
    pParams->enable = pCalibdbV2->TuningPara.enable;
    return res;

}


RKAIQ_END_DECLARE




