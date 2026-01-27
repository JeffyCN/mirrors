#include "rk_aiq_acnr_algo_cnr_v1.h"


RKAIQ_BEGIN_DECLARE

Acnr_result_t cnr_get_mode_by_name_V1(struct list_head* pCalibdbList, char *name, Calibdb_Cnr_V1_t** ppProfile)
{
    Acnr_result_t res = ACNR_RET_SUCCESS;

    if(pCalibdbList == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNR_RET_NULL_POINTER;
    }

    if(name == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNR_RET_NULL_POINTER;
    }

    if(ppProfile == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNR_RET_NULL_POINTER;
    }

#if 1
    *ppProfile = NULL;
    struct list_head* p;
    p = pCalibdbList->next;
    while (p != pCalibdbList)
    {
        Calibdb_Cnr_V1_t* pProfile = container_of(p, Calibdb_Cnr_V1_t, listItem);
        LOGD_ANR("%s:%d %s  %p \n",
                 __FUNCTION__, __LINE__, pProfile->modeName, p);
        if (!strncmp(pProfile->modeName, name, sizeof(pProfile->modeName))) {
            *ppProfile = pProfile;
            return res;
        }
        p = p->next;
    }

    Calibdb_Cnr_V1_t* pProfile = container_of(pCalibdbList->next, Calibdb_Cnr_V1_t, listItem);
    *ppProfile = pProfile;
#else


#endif

    return res;

}



Acnr_result_t cnr_get_setting_by_name_V1(struct list_head *pSettingList, char *name, Calibdb_Cnr_params_V1_t** ppSetting)
{
    Acnr_result_t res = ACNR_RET_SUCCESS;

    if(pSettingList == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNR_RET_NULL_POINTER;
    }

    if(name == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNR_RET_NULL_POINTER;
    }

    if(ppSetting == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNR_RET_NULL_POINTER;
    }

    *ppSetting = NULL;

    struct list_head* p;
    p = pSettingList->next;
    while (p != pSettingList)
    {
        Calibdb_Cnr_params_V1_t* pSetting = container_of(p, Calibdb_Cnr_params_V1_t, listItem);
        LOGD_ANR("%s:%d:  %s  %p \n",
                 __FUNCTION__, __LINE__, pSetting->snr_mode, p);
        if (!strncmp(pSetting->snr_mode, name, sizeof(pSetting->snr_mode))) {
            *ppSetting = pSetting;
            return res;
        }
        p = p->next;
    }

    Calibdb_Cnr_params_V1_t* pSetting = container_of(pSettingList->next, Calibdb_Cnr_params_V1_t, listItem);
    *ppSetting = pSetting;
    return res;
}


Acnr_result_t cnr_config_setting_param_V1(RK_CNR_Params_V1_t *pParams, struct list_head *pCalibdbList, char* param_mode, char * snr_name)
{
    Acnr_result_t res = ACNR_RET_SUCCESS;
    Calibdb_Cnr_V1_t *pProfile;
    Calibdb_Cnr_params_V1_t *pCalibParms;

    if(pParams == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNR_RET_NULL_POINTER;
    }

    if(pCalibdbList == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNR_RET_NULL_POINTER;
    }

    if(param_mode == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNR_RET_NULL_POINTER;
    }

    if(snr_name == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNR_RET_NULL_POINTER;
    }

    res = cnr_get_mode_by_name_V1(pCalibdbList, param_mode, &pProfile);
    if(res != ACNR_RET_SUCCESS) {
        LOGW_ANR("%s(%d): error!!!  can't find mode name in iq files, use 0 instead\n", __FUNCTION__, __LINE__);
    }

    res = cnr_get_setting_by_name_V1(&pProfile->listHead, snr_name, &pCalibParms);
    if(res != ACNR_RET_SUCCESS) {
        LOGW_ANR("%s(%d): error!!!  can't find setting in iq files, use 0 instead\n", __FUNCTION__, __LINE__);
    }

    res = cnr_init_params_V1(pParams, pCalibParms);
    pParams->enable = pProfile->enable;
    return res;

}


Acnr_result_t cnr_init_params_V1(RK_CNR_Params_V1_t *pParams, Calibdb_Cnr_params_V1_t* pCalibParms)
{
    Acnr_result_t res = ACNR_RET_SUCCESS;
    int i = 0;

    if(pParams == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNR_RET_NULL_POINTER;
    }

    if(pCalibParms == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNR_RET_NULL_POINTER;
    }

    pParams->enable = pCalibParms->enable;

    for(i = 0; i < RK_CNR_V1_MAX_ISO_NUM; i++ ) {
        pParams->iso[i] = pCalibParms->iso[i];
        pParams->rkcnr_hq_bila_bypass[i] = pCalibParms->rkcnr_hq_bila_bypass[i];
        pParams->rkcnr_lq_bila_bypass[i] = pCalibParms->rkcnr_lq_bila_bypass[i];
        pParams->rkcnr_exgain[i] = pCalibParms->rkcnr_exgain[i];
        pParams->rkcnr_g_gain[i] = pCalibParms->rkcnr_g_gain[i];
        pParams->ratio[i] = pCalibParms->ratio[i];
        pParams->offset[i] = pCalibParms->offset[i];
        pParams->medRatio1[i] = pCalibParms->medRatio1[i];
        pParams->sigmaR1[i] = pCalibParms->sigmaR1[i];
        pParams->uvgain1[i] = pCalibParms->uvgain1[i];
        pParams->bfRatio1[i] = pCalibParms->bfRatio1[i];
        pParams->hbf_wgt_clip[i] = pCalibParms->hbf_wgt_clip[i];
        pParams->medRatio2[i] = pCalibParms->medRatio2[i];
        pParams->sigmaR2[i] = pCalibParms->sigmaR2[i];
        pParams->uvgain2[i] = pCalibParms->uvgain2[i];
        pParams->sigmaR3[i] = pCalibParms->sigmaR3[i];
        pParams->uvgain3[i] = pCalibParms->uvgain3[i];
        pParams->bfRatio3[i] = pCalibParms->bfRatio3[i];
    }


    memcpy(pParams->kernel_5x5_table, pCalibParms->kernel_5x5_table, sizeof(float) * 5);

    return ACNR_RET_SUCCESS;

}


float interp_cnr_v1(int ISO_low, int ISO_high, float value_low, float value_high, int ISO, float value)
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


Acnr_result_t cnr_select_params_by_ISO_V1(RK_CNR_Params_V1_t *pParams, RK_CNR_Params_V1_Select_t *pSelect, Acnr_ExpInfo_t *pExpInfo)
{
    Acnr_result_t res = ACNR_RET_SUCCESS;
    int iso = 50;
    if(pParams == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNR_RET_NULL_POINTER;
    }

    if(pSelect == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNR_RET_NULL_POINTER;
    }

    if(pExpInfo == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNR_RET_NULL_POINTER;
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
    int max_iso_step = RK_CNR_V1_MAX_ISO_NUM;

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

    LOGD_ANR("%s:%d iso:%d high:%d low:%d \n",
             __FUNCTION__, __LINE__,
             iso, isoGainHigh, isoGainLow);

    pSelect->enable = pParams->enable;
    // bypass
    pSelect->rkcnr_hq_bila_bypass = pParams->rkcnr_hq_bila_bypass[isoIndex];
    pSelect->rkcnr_lq_bila_bypass = pParams->rkcnr_lq_bila_bypass[isoIndex];

    // gain
    pSelect->rkcnr_exgain   = interp_cnr_v1(isoGainLow, isoGainHigh, pParams->rkcnr_exgain[isoIndexLow], pParams->rkcnr_exgain[isoIndexHigh], iso, pSelect->rkcnr_exgain);
    pSelect->rkcnr_g_gain   = interp_cnr_v1(isoGainLow, isoGainHigh, pParams->rkcnr_g_gain[isoIndexLow], pParams->rkcnr_g_gain[isoIndexHigh], iso, pSelect->rkcnr_g_gain);


    //
    pSelect->ratio = interp_cnr_v1(isoGainLow, isoGainHigh, pParams->ratio[isoIndexLow], pParams->ratio[isoIndexHigh], iso, pSelect->ratio);
    pSelect->offset = interp_cnr_v1(isoGainLow, isoGainHigh, pParams->offset[isoIndexLow], pParams->offset[isoIndexHigh], iso, pSelect->offset);

    // step1
    // median filter
    pSelect->medRatio1 = interp_cnr_v1(isoGainLow, isoGainHigh, pParams->medRatio1[isoIndexLow], pParams->medRatio1[isoIndexHigh], iso, pSelect->medRatio1);

    // bilateral filter
    pSelect->sigmaR1 = interp_cnr_v1(isoGainLow, isoGainHigh, pParams->sigmaR1[isoIndexLow], pParams->sigmaR1[isoIndexHigh], iso, pSelect->sigmaR1);
    pSelect->uvgain1 = interp_cnr_v1(isoGainLow, isoGainHigh, pParams->uvgain1[isoIndexLow], pParams->uvgain1[isoIndexHigh], iso, pSelect->uvgain1);
    pSelect->bfRatio1 = interp_cnr_v1(isoGainLow, isoGainHigh, pParams->bfRatio1[isoIndexLow], pParams->bfRatio1[isoIndexHigh], iso, pSelect->bfRatio1);
    pSelect->hbf_wgt_clip = pParams->hbf_wgt_clip[isoIndex];

    // step2
    // median filter
    pSelect->medRatio2 = interp_cnr_v1(isoGainLow, isoGainHigh, pParams->medRatio2[isoIndexLow], pParams->medRatio2[isoIndexHigh], iso, pSelect->medRatio2);

    // bilateral filter
    pSelect->sigmaR2 = interp_cnr_v1(isoGainLow, isoGainHigh, pParams->sigmaR2[isoIndexLow], pParams->sigmaR2[isoIndexHigh], iso, pSelect->sigmaR2);
    pSelect->uvgain2 = interp_cnr_v1(isoGainLow, isoGainHigh, pParams->uvgain2[isoIndexLow], pParams->uvgain2[isoIndexHigh], iso, pSelect->uvgain2);

    // step3
    // bilateral filter
    pSelect->sigmaR3 = interp_cnr_v1(isoGainLow, isoGainHigh, pParams->sigmaR3[isoIndexLow], pParams->sigmaR3[isoIndexHigh], iso, pSelect->sigmaR3);
    pSelect->uvgain3 = interp_cnr_v1(isoGainLow, isoGainHigh, pParams->uvgain3[isoIndexLow], pParams->uvgain3[isoIndexHigh], iso, pSelect->uvgain3);
    pSelect->bfRatio3 = interp_cnr_v1(isoGainLow, isoGainHigh, pParams->bfRatio3[isoIndexLow], pParams->bfRatio3[isoIndexHigh], iso, pSelect->bfRatio3);

    // bilateral filter kernels
    memcpy(pSelect->kernel_5x5_table, pParams->kernel_5x5_table, sizeof(float) * 5);

    return ACNR_RET_SUCCESS;

}


Acnr_result_t cnr_fix_transfer_V1(RK_CNR_Params_V1_Select_t *pSelect, RK_CNR_Fix_V1_t *pFix, Acnr_ExpInfo_t *pExpInfo, float fStrength)
{
    LOGI_ANR("%s:(%d) enter \n", __FUNCTION__, __LINE__);

    int i = 0;
    Acnr_result_t res = ACNR_RET_SUCCESS;
    int tmp = 0;

    if(pSelect == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNR_RET_NULL_POINTER;
    }

    if(pFix == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNR_RET_NULL_POINTER;
    }

    if(pExpInfo == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNR_RET_NULL_POINTER;
    }


    if(fStrength <= 0.0) {
        fStrength = 0.000001;
    }

    // fix bit : RK_CNR_V1_log2e + RK_CNR_V1_SIGMA_FIX_BIT, msigma = (1 / sigma) * (1 << RK_CNR_V1_SIGMA_FIX_BIT) * log2e * (1 << RK_CNR_V1_log2e);
    int log2e = (int)(0.8493f * (1 << (RK_CNR_V1_log2e + RK_CNR_V1_SIGMA_FIX_BIT)));

    //ISP_CNR_2800_CTRL
    pFix->cnr_thumb_mix_cur_en = 0;
    pFix->cnr_exgain_bypass = 1;
    pFix->cnr_hq_bila_bypass = pSelect->rkcnr_hq_bila_bypass;
    pFix->cnr_lq_bila_bypass = pSelect->rkcnr_lq_bila_bypass;
    pFix->cnr_en_i = pSelect->enable;

    // ISP_CNR_2800_EXGAIN
    tmp = pSelect->rkcnr_exgain * (1 << RK_CNR_V1_sgmGain);
    pFix->cnr_exgain_mux = CLIP(tmp, 0, 255);

    // ISP_CNR_2800_GAIN_PARA
    tmp = pSelect->rkcnr_g_gain * (1 << RK_CNR_V1_gainRatio);
    pFix->cnr_gain_iso = CLIP(tmp, 0, 128);
    tmp = (int)(pSelect->offset * (1 << RK_CNR_V1_offset));
    pFix->cnr_gain_offset = CLIP(tmp, 0, 16);
    tmp = (int)(pSelect->ratio * (1 << RK_CNR_V1_ratio));
    pFix->cnr_gain_1sigma = CLIP(tmp, 0, 255);

    // ISP_CNR_2800_GAIN_UV_PARA
    tmp = (int)(pSelect->uvgain1 / fStrength * (1 << RK_CNR_V1_uvgain));
    pFix->cnr_gain_uvgain0 = CLIP(tmp, 0, (1 << 7) - 1);
    tmp = (int)(pSelect->uvgain3 / fStrength * (1 << RK_CNR_V1_uvgain));
    pFix->cnr_gain_uvgain1 = CLIP(tmp, 0, (1 << 7) - 1);

    // ISP_CNR_2800_LMED3
    tmp = (int)(pSelect->medRatio2 * (1 << RK_CNR_V1_medRatio));
    pFix->cnr_lmed3_alpha = CLIP(tmp, 0, 16);

    // ISP_CNR_2800_LBF5_GAIN
    int rkcnr_sigmaR2   = (int)(log2e / pSelect->sigmaR2 / fStrength);
    int rkcnr_uvgain2   = (int)(pSelect->uvgain2 / fStrength * (1 << RK_CNR_V1_uvgain));
    int tmpBit = 10 + RK_CNR_V1_log2e + RK_CNR_V1_uvgain - 6;
    int sgmRatio = 1 << RK_CNR_V1_sgmRatio;
    int ky = sgmRatio * (1 << RK_CNR_V1_uvgain) >> RK_CNR_V1_sgmRatio;
    int kuv = sgmRatio * rkcnr_uvgain2 >> RK_CNR_V1_sgmRatio;
    ky = MIN2(ky, (1 << (4 + RK_CNR_V1_uvgain)) - 1);
    kuv = MIN2(kuv, (1 << (4 + RK_CNR_V1_uvgain)) - 1);
    tmp = ((rkcnr_sigmaR2 * ky + (1 << (tmpBit - 1))) >> tmpBit);
    pFix->cnr_lbf5_gain_y = CLIP(tmp, 0, 15);
    tmp = ((rkcnr_sigmaR2 * kuv + (1 << (tmpBit - 1))) >> tmpBit);
    pFix->cnr_lbf5_gain_c = CLIP(tmp, 0, 63);

    // ISP_CNR_2800_LBF5_WEITD0_3
    // bilateral filter kernels
    for (i = 0; i < 5; i++) {
        tmp = (int)(pSelect->kernel_5x5_table[i] * (1 << RK_CNR_V1_kernels));
        pFix->cnr_lbf5_weit_d[i] = CLIP(tmp, 0, 128);
    }

    // ISP_CNR_2800_HMED3
    tmp = (int)(pSelect->medRatio1 * (1 << RK_CNR_V1_medRatio));
    pFix->cnr_hmed3_alpha = CLIP(tmp, 0, 16);

    // ISP_CNR_2800_HBF5
    tmp = (int)(log2e / pSelect->sigmaR1 / fStrength);
    pFix->cnr_hbf5_sigma    = CLIP(tmp, 0, (1 << 13) - 1);
    tmp = (int)(pSelect->bfRatio1 * (1 << RK_CNR_V1_bfRatio));
    pFix->cnr_hbf5_weit_src = CLIP(tmp, 0, 128);
    tmp = pSelect->hbf_wgt_clip;
    pFix->cnr_hbf5_min_wgt = CLIP(tmp, 0, (1 << 8) - 1);

    // ISP_CNR_2800_LBF3
    // step3
    // bilateral filter
    tmp = (int)(log2e / pSelect->sigmaR3 / fStrength);
    pFix->cnr_lbf3_sigma = CLIP(tmp, 0, (1 << 13) - 1);
    tmp = (int)(pSelect->bfRatio3 * (1 << RK_CNR_V1_bfRatio));
    pFix->cnr_lbf5_weit_src = CLIP(tmp, 0, 128);

    cnr_fix_printf_V1(pFix);

    LOGI_ANR("%s:(%d) exit \n", __FUNCTION__, __LINE__);

    return ACNR_RET_SUCCESS;
}


Acnr_result_t cnr_fix_printf_V1(RK_CNR_Fix_V1_t  * pFix)
{
    int i = 0;
    LOGI_ANR("%s:(%d) enter \n", __FUNCTION__, __LINE__);

    Acnr_result_t res = ACNR_RET_SUCCESS;

    if(pFix == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNR_RET_NULL_POINTER;
    }

    //ISP_CNR_2800_CTRL (0x0000)
    LOGD_ANR("(0x0000) thumb_mix_cur_en:0x%x lq_bila_bypass:0x%x hq_bila_bypass:0x%x exgain_bypass:0x%x cnr_en_i:0x%x\n",
             pFix->cnr_thumb_mix_cur_en,
             pFix->cnr_lq_bila_bypass,
             pFix->cnr_hq_bila_bypass,
             pFix->cnr_exgain_bypass,
             pFix->cnr_en_i);

    // ISP_CNR_2800_EXGAIN  (0x0004)
    LOGD_ANR("(0x0004) cnr_exgain_mux:0x%x \n",
             pFix->cnr_exgain_mux);

    // ISP_CNR_2800_GAIN_PARA  (0x0008)
    LOGD_ANR("(0x0008) gain_iso:0x%x gain_offset:0x%x gain_1sigma:0x%x \n",
             pFix->cnr_gain_iso,
             pFix->cnr_gain_offset,
             pFix->cnr_gain_1sigma);

    // ISP_CNR_2800_GAIN_UV_PARA (0x000c)
    LOGD_ANR("(0x000c) gain_uvgain1:0x%x gain_uvgain0:0x%x \n",
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
    for(i = 0; i < 5; i++) {
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

    LOGD_ANR("%s:(%d) exit \n", __FUNCTION__, __LINE__);

    return ACNR_RET_SUCCESS;
}




Acnr_result_t cnr_get_setting_by_name_json_V1(CalibDbV2_CNR_t *pCalibdbV2, char *name, int *tuning_idx)
{
    int i = 0;
    Acnr_result_t res = ACNR_RET_SUCCESS;

    if(pCalibdbV2 == NULL || name == NULL || tuning_idx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNR_RET_NULL_POINTER;
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



Acnr_result_t cnr_init_params_json_V1(RK_CNR_Params_V1_t *pParams, CalibDbV2_CNR_t *pCalibdbV2, int tuning_idx)
{
    Acnr_result_t res = ACNR_RET_SUCCESS;
    CalibDbV2_CNR_T_ISO_t *pTuningISO = NULL;

    if(pParams == NULL || pCalibdbV2 == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNR_RET_NULL_POINTER;
    }

    pParams->enable = pCalibdbV2->TuningPara.enable;

    for(int i = 0; i < pCalibdbV2->TuningPara.Setting[tuning_idx].Tuning_ISO_len && i < RK_CNR_V1_MAX_ISO_NUM; i++ ) {
        pTuningISO = &pCalibdbV2->TuningPara.Setting[tuning_idx].Tuning_ISO[i];

        pParams->iso[i] = pTuningISO->iso;
        pParams->rkcnr_hq_bila_bypass[i] = pTuningISO->hf_bypass;
        pParams->rkcnr_lq_bila_bypass[i] = pTuningISO->lf_bypass;
        pParams->rkcnr_exgain[i] = pTuningISO->cnr_exgain;
        pParams->rkcnr_g_gain[i] = pTuningISO->cnr_g_gain;
        pParams->ratio[i] = pTuningISO->color_sat_adj;
        pParams->offset[i] = pTuningISO->color_sat_adj_alpha;
        pParams->medRatio1[i] = pTuningISO->hf_spikes_reducion_strength;
        pParams->sigmaR1[i] = pTuningISO->hf_denoise_strength;
        pParams->uvgain1[i] = pTuningISO->hf_color_sat;
        pParams->bfRatio1[i] = pTuningISO->hf_denoise_alpha;
        pParams->hbf_wgt_clip[i] = pTuningISO->hf_bf_wgt_clip;
        pParams->medRatio2[i] = pTuningISO->thumb_spikes_reducion_strength;
        pParams->sigmaR2[i] = pTuningISO->thumb_denoise_strength;
        pParams->uvgain2[i] = pTuningISO->thumb_color_sat;
        pParams->sigmaR3[i] = pTuningISO->lf_denoise_strength;
        pParams->uvgain3[i] = pTuningISO->lf_color_sat;
        pParams->bfRatio3[i] = pTuningISO->lf_denoise_alpha;
    }


    memcpy(pParams->kernel_5x5_table, pCalibdbV2->TuningPara.Kernel_Coeff.kernel_5x5, sizeof(float) * 5);

    return ACNR_RET_SUCCESS;

}

Acnr_result_t cnr_config_setting_param_json_V1(RK_CNR_Params_V1_t *pParams, CalibDbV2_CNR_t *pCalibdbV2, char* param_mode, char * snr_name)
{
    Acnr_result_t res = ACNR_RET_SUCCESS;
    int tuning_idx;

    if(pParams == NULL || pCalibdbV2 == NULL || param_mode == NULL || snr_name == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNR_RET_NULL_POINTER;
    }

    res = cnr_get_setting_by_name_json_V1(pCalibdbV2, snr_name, &tuning_idx);
    if(res != ACNR_RET_SUCCESS) {
        LOGW_ANR("%s(%d): error!!!  can't find setting in iq files, use 0 instead\n", __FUNCTION__, __LINE__);
    }

    res = cnr_init_params_json_V1(pParams, pCalibdbV2, tuning_idx);
    pParams->enable = pCalibdbV2->TuningPara.enable;
    return res;

}


RKAIQ_END_DECLARE




