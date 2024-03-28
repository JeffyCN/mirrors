
#include "rk_aiq_aynr_algo_ynr_v2.h"

RKAIQ_BEGIN_DECLARE


Aynr_result_t ynr_get_mode_by_name_V2(struct list_head* pCalibdbList, char *name, Calibdb_Ynr_V2_t** ppProfile)
{
    Aynr_result_t res = AYNR_RET_SUCCESS;

    if(pCalibdbList == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNR_RET_NULL_POINTER;
    }

    if(name == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNR_RET_NULL_POINTER;
    }

    if(ppProfile == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNR_RET_NULL_POINTER;
    }

#if 1
    *ppProfile = NULL;
    struct list_head* p;
    p = pCalibdbList->next;
    while (p != pCalibdbList)
    {
        Calibdb_Ynr_V2_t* pProfile = container_of(p, Calibdb_Ynr_V2_t, listItem);
        LOGD_ANR("%s:%d %s  %p \n",
                 __FUNCTION__, __LINE__, pProfile->modeName, p);
        if (!strncmp(pProfile->modeName, name, sizeof(pProfile->modeName))) {
            *ppProfile = pProfile;
            return res;
        }
        p = p->next;
    }

    Calibdb_Ynr_V2_t* pProfile = container_of(pCalibdbList->next, Calibdb_Ynr_V2_t, listItem);
    *ppProfile = pProfile;
#else


#endif

    return res;

}


Aynr_result_t ynr_get_setting_by_name_V2(struct list_head *pSettingList, char *name, Calibdb_Ynr_params_V2_t** ppSetting)
{
    Aynr_result_t res = AYNR_RET_SUCCESS;

    if(pSettingList == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNR_RET_NULL_POINTER;
    }

    if(name == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNR_RET_NULL_POINTER;
    }

    if(ppSetting == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNR_RET_NULL_POINTER;
    }

    *ppSetting = NULL;

    struct list_head* p;
    p = pSettingList->next;
    while (p != pSettingList)
    {
        Calibdb_Ynr_params_V2_t* pSetting = container_of(p, Calibdb_Ynr_params_V2_t, listItem);
        LOGD_ANR("%s:%d:  %s  %p ",
                 __FUNCTION__, __LINE__, pSetting->snr_mode, p);
        if (!strncmp(pSetting->snr_mode, name, sizeof(pSetting->snr_mode))) {
            *ppSetting = pSetting;
            return res;
        }
        p = p->next;
    }

    Calibdb_Ynr_params_V2_t* pSetting = container_of(pSettingList->next, Calibdb_Ynr_params_V2_t, listItem);
    *ppSetting = pSetting;
    return res;
}

Aynr_result_t ynr_config_setting_param_V2(RK_YNR_Params_V2_t *pParams, struct list_head *pCalibdbList, char* param_mode, char * snr_name)
{
    Aynr_result_t res = AYNR_RET_SUCCESS;
    Calibdb_Ynr_V2_t *pProfile;
    Calibdb_Ynr_params_V2_t *pCalibParms;

    if(pParams == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNR_RET_NULL_POINTER;
    }

    if(pCalibdbList == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNR_RET_NULL_POINTER;
    }

    if(param_mode == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNR_RET_NULL_POINTER;
    }

    if(snr_name == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNR_RET_NULL_POINTER;
    }

    res = ynr_get_mode_by_name_V2(pCalibdbList, param_mode, &pProfile);
    if(res != AYNR_RET_SUCCESS) {
        LOGW_ANR("%s(%d): error!!!  can't find mode name in iq files, use 0 instead\n", __FUNCTION__, __LINE__);
    }

    res = ynr_get_setting_by_name_V2(&pProfile->listHead, snr_name, &pCalibParms);
    if(res != AYNR_RET_SUCCESS) {
        LOGW_ANR("%s(%d): error!!!  can't find setting in iq files, use 0 instead\n", __FUNCTION__, __LINE__);
    }

    res = ynr_init_params_V2(pParams, pCalibParms);
    pParams->enable = pProfile->enable;
    return res;

}
Aynr_result_t ynr_init_params_V2(RK_YNR_Params_V2_t *pYnrParams, Calibdb_Ynr_params_V2_t* pCalibParms)
{
    Aynr_result_t res = AYNR_RET_SUCCESS;
    int i = 0;
    int j = 0;
    short isoCurveSectValue;
    short isoCurveSectValue1;
    float ave1, ave2, ave3, ave4;
    int bit_calib = 12;
    int bit_proc;
    int bit_shift;

    if(pYnrParams == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNR_RET_NULL_POINTER;
    }

    if(pCalibParms == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNR_RET_NULL_POINTER;
    }

    bit_proc = YNR_V2_SIGMA_BITS; // for V3, YNR_SIGMA_BITS = 10
    bit_shift = bit_calib - bit_proc;

    isoCurveSectValue = (1 << (bit_calib - YNR_V2_ISO_CURVE_POINT_BIT));
    isoCurveSectValue1 = (1 << bit_calib);

    for(j = 0; j < RK_YNR_V2_MAX_ISO_NUM; j++) {
        pYnrParams->iso[j] = pCalibParms->iso[j];
        pYnrParams->arYnrParamsISO[j].ciISO_V2[0] = pCalibParms->ciISO_V2[0][j];
        pYnrParams->arYnrParamsISO[j].ciISO_V2[1] = pCalibParms->ciISO_V2[1][j];

        // get noise sigma sample data at [0, 64, 128, ... , 1024]
        for (i = 0; i < YNR_V2_ISO_CURVE_POINT_NUM; i++) {
            if (i == (YNR_V2_ISO_CURVE_POINT_NUM - 1)) {
                ave1 = (float)isoCurveSectValue1;
            } else {
                ave1 = (float)(i * isoCurveSectValue);
            }
            pYnrParams->arYnrParamsISO[j].lumaPoints_V2[i] = (short)ave1;
            ave2 = ave1 * ave1;
            ave3 = ave2 * ave1;
            ave4 = ave3 * ave1;
            pYnrParams->arYnrParamsISO[j].noiseSigma_V2[i] = pCalibParms->sigmaCurve[j][0] * ave4
                    + pCalibParms->sigmaCurve[j][1] * ave3
                    + pCalibParms->sigmaCurve[j][2] * ave2
                    + pCalibParms->sigmaCurve[j][3] * ave1
                    + pCalibParms->sigmaCurve[j][4];

            if (pYnrParams->arYnrParamsISO[j].noiseSigma_V2[i] < 0) {
                pYnrParams->arYnrParamsISO[j].noiseSigma_V2[i] = 0;
            }

            if (bit_shift > 0) {
                pYnrParams->arYnrParamsISO[j].lumaPoints_V2[i] >>= bit_shift;
            } else {
                pYnrParams->arYnrParamsISO[j].lumaPoints_V2[i] <<= ABS(bit_shift);
            }
        }

        for(i = 0; i < 17; i++) {
            pYnrParams->arYnrParamsISO[j].ynr_rnr_strength_V2[i] = pCalibParms->ynr_rnr_strength_V2[j][i];
        }

        pYnrParams->arYnrParamsISO[j].ynr_bft3x3_bypass_V2 = pCalibParms->ynr_bft3x3_bypass_V2[j];
        pYnrParams->arYnrParamsISO[j].ynr_lbft5x5_bypass_V2 = pCalibParms->ynr_lbft5x5_bypass_V2[j];
        pYnrParams->arYnrParamsISO[j].ynr_lgft3x3_bypass_V2 = pCalibParms->ynr_lgft3x3_bypass_V2[j];
        pYnrParams->arYnrParamsISO[j].ynr_flt1x1_bypass_V2 = pCalibParms->ynr_flt1x1_bypass_V2[j];
        pYnrParams->arYnrParamsISO[j].ynr_sft5x5_bypass_V2 = pCalibParms->ynr_sft5x5_bypass_V2[j];

        pYnrParams->arYnrParamsISO[j].ynr_low_bf_V2[0] = pCalibParms->ynr_low_bf_V2[0][j];
        pYnrParams->arYnrParamsISO[j].ynr_low_bf_V2[1] = pCalibParms->ynr_low_bf_V2[1][j];
        pYnrParams->arYnrParamsISO[j].ynr_low_thred_adj_V2 = pCalibParms->ynr_low_thred_adj_V2[j];
        pYnrParams->arYnrParamsISO[j].ynr_low_peak_supress_V2 = pCalibParms->ynr_low_peak_supress_V2[j];
        pYnrParams->arYnrParamsISO[j].ynr_low_edge_adj_thresh_V2 = pCalibParms->ynr_low_edge_adj_thresh_V2[j];
        pYnrParams->arYnrParamsISO[j].ynr_low_center_weight_V2 = pCalibParms->ynr_low_center_weight_V2[j];
        pYnrParams->arYnrParamsISO[j].ynr_low_dist_adj_V2 = pCalibParms->ynr_low_dist_adj_V2[j];
        pYnrParams->arYnrParamsISO[j].ynr_low_weight_V2 = pCalibParms->ynr_low_weight_V2[j];
        pYnrParams->arYnrParamsISO[j].ynr_low_filt1_strength_V2 = pCalibParms->ynr_low_filt_strength_V2[0][j];
        pYnrParams->arYnrParamsISO[j].ynr_low_filt2_strength_V2 = pCalibParms->ynr_low_filt_strength_V2[1][j];
        pYnrParams->arYnrParamsISO[j].ynr_low_bi_weight_V2 = pCalibParms->ynr_low_bi_weight_V2[j];

        pYnrParams->arYnrParamsISO[j].ynr_base_filter_weight1_V2 = pCalibParms->ynr_base_filter_weight_V2[0][j];
        pYnrParams->arYnrParamsISO[j].ynr_base_filter_weight2_V2 = pCalibParms->ynr_base_filter_weight_V2[1][j];
        pYnrParams->arYnrParamsISO[j].ynr_base_filter_weight3_V2 = pCalibParms->ynr_base_filter_weight_V2[2][j];
        pYnrParams->arYnrParamsISO[j].ynr_high_thred_adj_V2 = pCalibParms->ynr_high_thred_adj_V2[j];
        pYnrParams->arYnrParamsISO[j].ynr_high_weight_V2 = pCalibParms->ynr_high_weight_V2[j];
        for(i = 0; i < 8; i++) {
            pYnrParams->arYnrParamsISO[j].ynr_direction_weight_V2[i] = pCalibParms->ynr_direction_weight_V2[j][i];
        }
        pYnrParams->arYnrParamsISO[j].ynr_hi_min_adj_V2 = pCalibParms->ynr_hi_min_adj_V2[j];
        pYnrParams->arYnrParamsISO[j].ynr_hi_edge_thed_V2 = pCalibParms->ynr_hi_edge_thed_V2[j];

    }


    return res;
}


Aynr_result_t ynr_select_params_by_ISO_V2(RK_YNR_Params_V2_t *pParams, RK_YNR_Params_V2_Select_t *pSelect, Aynr_ExpInfo_t *pExpInfo)
{
    float ratio = 0.0f;
    int iso = 50;
    RK_YNR_Params_V2_Select_t *pParamHi = NULL;
    RK_YNR_Params_V2_Select_t *pParamLo = NULL;
    RK_YNR_Params_V2_Select_t* pParamTmp = NULL;


    Aynr_result_t res = AYNR_RET_SUCCESS;

    if(pParams == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNR_RET_NULL_POINTER;
    }

    if(pSelect == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNR_RET_NULL_POINTER;
    }

    if(pExpInfo == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNR_RET_NULL_POINTER;
    }

    iso = pExpInfo->arIso[pExpInfo->hdr_mode];

    // choose integer type data
    int cur_iso_idx = 0;
    int idx = 0;
    for (idx = 0; idx < RK_YNR_V2_MAX_ISO_NUM; idx++) {
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
    if (idx == RK_YNR_V2_MAX_ISO_NUM)
        cur_iso_idx = RK_YNR_V2_MAX_ISO_NUM - 1;


    pParamTmp = &pParams->arYnrParamsISO[cur_iso_idx];

    pSelect->enable = pParams->enable;
    pSelect->ynr_bft3x3_bypass_V2 = pParamTmp->ynr_bft3x3_bypass_V2;
    pSelect->ynr_lbft5x5_bypass_V2 = pParamTmp->ynr_lbft5x5_bypass_V2;
    pSelect->ynr_lgft3x3_bypass_V2 = pParamTmp->ynr_lgft3x3_bypass_V2;
    pSelect->ynr_flt1x1_bypass_V2 = pParamTmp->ynr_flt1x1_bypass_V2;
    pSelect->ynr_sft5x5_bypass_V2 = pParamTmp->ynr_sft5x5_bypass_V2;


    int iso_div = 50;
    int lowIso = 50;
    int highIso = 50;
    int minIso = 50;
    int maxIso = 50;

    for(int i = 0; i < RK_YNR_V2_MAX_ISO_NUM - 1; i++) {
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
    maxIso = pParams->iso[RK_YNR_V2_MAX_ISO_NUM - 1];
#else
    minIso = iso_div * (1 << 0);
    maxIso = iso_div * (1 << (RK_YNR_V2_MAX_ISO_NUM - 1));
#endif

    if(iso < minIso) {
        ratio = 0;
        pParamLo = &pParams->arYnrParamsISO[0];
        pParamHi = &pParams->arYnrParamsISO[1];
    }

    if(iso > maxIso) {
        ratio = 1;
        pParamLo = &pParams->arYnrParamsISO[RK_YNR_V2_MAX_ISO_NUM - 1];
        pParamHi = &pParams->arYnrParamsISO[RK_YNR_V2_MAX_ISO_NUM];
    }


    LOGD_ANR("oyyf %s:%d  iso:%d low:%d hight:%d ratio:%f iso_index:%d \n", __FUNCTION__, __LINE__,
             iso, lowIso, highIso, ratio, cur_iso_idx);
    pSelect->ynr_global_gain_V2 = 16;
    // get rnr parameters
    for (int i = 0; i < 17; i++)
    {
        pSelect->ynr_rnr_strength_V2[i] = ratio * (pParamHi->ynr_rnr_strength_V2[i] - pParamLo->ynr_rnr_strength_V2[i]) + pParamLo->ynr_rnr_strength_V2[i];
    }

    // get the parameters for current ISO
    // ci
    for (int i = 0; i < 2; i++)
    {
        pSelect->ciISO_V2[i] = ratio * (pParamHi->ciISO_V2[i] - pParamLo->ciISO_V2[i]) + pParamLo->ciISO_V2[i];
    }

    // noise curve
    for (int i = 0; i < YNR_V2_ISO_CURVE_POINT_NUM; i++)
    {
        pSelect->noiseSigma_V2[i] = ratio * (pParamHi->noiseSigma_V2[i] - pParamLo->noiseSigma_V2[i]) + pParamLo->noiseSigma_V2[i];
        pSelect->lumaPoints_V2[i] = (short)(ratio * (pParamHi->lumaPoints_V2[i] - pParamLo->lumaPoints_V2[i]) + pParamLo->lumaPoints_V2[i]);
    }

    // lo bf
    for (int i = 0; i < 2; i++)
    {
        pSelect->ynr_low_bf_V2[i] = ratio * (pParamHi->ynr_low_bf_V2[i] - pParamLo->ynr_low_bf_V2[i]) + pParamLo->ynr_low_bf_V2[i];
    }

    pSelect->ynr_low_thred_adj_V2 = ratio * (pParamHi->ynr_low_thred_adj_V2 - pParamLo->ynr_low_thred_adj_V2) + pParamLo->ynr_low_thred_adj_V2;
    pSelect->ynr_low_peak_supress_V2 = ratio * (pParamHi->ynr_low_peak_supress_V2 - pParamLo->ynr_low_peak_supress_V2) + pParamLo->ynr_low_peak_supress_V2;
    pSelect->ynr_low_edge_adj_thresh_V2 = ratio * (pParamHi->ynr_low_edge_adj_thresh_V2 - pParamLo->ynr_low_edge_adj_thresh_V2) + pParamLo->ynr_low_edge_adj_thresh_V2;
    pSelect->ynr_low_center_weight_V2 = ratio * (pParamHi->ynr_low_center_weight_V2 - pParamLo->ynr_low_center_weight_V2) + pParamLo->ynr_low_center_weight_V2;
    pSelect->ynr_low_dist_adj_V2 = ratio * (pParamHi->ynr_low_dist_adj_V2 - pParamLo->ynr_low_dist_adj_V2) + pParamLo->ynr_low_dist_adj_V2;
    pSelect->ynr_low_weight_V2 = ratio * (pParamHi->ynr_low_weight_V2 - pParamLo->ynr_low_weight_V2) + pParamLo->ynr_low_weight_V2;

    pSelect->ynr_low_filt1_strength_V2 = ratio * (pParamHi->ynr_low_filt1_strength_V2 - pParamLo->ynr_low_filt1_strength_V2) + pParamLo->ynr_low_filt1_strength_V2;
    pSelect->ynr_low_filt2_strength_V2 = ratio * (pParamHi->ynr_low_filt2_strength_V2 - pParamLo->ynr_low_filt2_strength_V2) + pParamLo->ynr_low_filt2_strength_V2;

    pSelect->ynr_low_bi_weight_V2 = ratio * (pParamHi->ynr_low_bi_weight_V2 - pParamLo->ynr_low_bi_weight_V2) + pParamLo->ynr_low_bi_weight_V2;

    // High Freq
    pSelect->ynr_base_filter_weight1_V2 = ratio * (pParamHi->ynr_base_filter_weight1_V2 - pParamLo->ynr_base_filter_weight1_V2) + pParamLo->ynr_base_filter_weight1_V2;
    pSelect->ynr_base_filter_weight2_V2 = ratio * (pParamHi->ynr_base_filter_weight2_V2 - pParamLo->ynr_base_filter_weight2_V2) + pParamLo->ynr_base_filter_weight2_V2;
    pSelect->ynr_base_filter_weight3_V2 = ratio * (pParamHi->ynr_base_filter_weight3_V2 - pParamLo->ynr_base_filter_weight3_V2) + pParamLo->ynr_base_filter_weight3_V2;


    pSelect->ynr_high_thred_adj_V2 = ratio * (pParamHi->ynr_high_thred_adj_V2 - pParamLo->ynr_high_thred_adj_V2) + pParamLo->ynr_high_thred_adj_V2;
    pSelect->ynr_high_weight_V2 = ratio * (pParamHi->ynr_high_weight_V2 - pParamLo->ynr_high_weight_V2) + pParamLo->ynr_high_weight_V2;

    for (int i = 0; i < 8; i++)
    {
        pSelect->ynr_direction_weight_V2[i] = ratio * (pParamHi->ynr_direction_weight_V2[i] - pParamLo->ynr_direction_weight_V2[i]) + pParamLo->ynr_direction_weight_V2[i];
    }
    pSelect->ynr_hi_min_adj_V2 = ratio * (pParamHi->ynr_hi_min_adj_V2 - pParamLo->ynr_hi_min_adj_V2) + pParamLo->ynr_hi_min_adj_V2;
    pSelect->ynr_hi_edge_thed_V2 = ratio * (pParamHi->ynr_hi_edge_thed_V2 - pParamLo->ynr_hi_edge_thed_V2) + pParamLo->ynr_hi_edge_thed_V2;

    return res;
}

Aynr_result_t ynr_fix_transfer_V2(RK_YNR_Params_V2_Select_t* pSelect, RK_YNR_Fix_V2_t *pFix, float fStrength, Aynr_ExpInfo_t *pExpInfo)
{
    LOGI_ANR("%s:(%d) enter \n", __FUNCTION__, __LINE__);

    Aynr_result_t res = AYNR_RET_SUCCESS;
    int w0, w1, w2;
    int tmp;

    if(pSelect == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNR_RET_NULL_POINTER;
    }

    if(pFix == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNR_RET_NULL_POINTER;
    }

    if(pExpInfo == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNR_RET_NULL_POINTER;
    }

    LOGD_ANR("%s:%d strength:%f raw:width:%d height:%d\n",
             __FUNCTION__, __LINE__,
             fStrength, pExpInfo->rawHeight, pExpInfo->rawWidth);

    if(fStrength <= 0.0) {
        fStrength = 0.000001;
    }

    // YNR_2700_GLOBAL_CTRL (0x0000)
    pFix->ynr_debug_en = 0;
    pFix->ynr_gate_dis = 0;
    pFix->ynr_thumb_mix_cur_en = 0;
    pFix->ynr_global_gain_alpha = 8;
    pFix->ynr_global_gain = pSelect->ynr_global_gain_V2;
    pFix->ynr_flt1x1_bypass_sel = 0;
    pFix->ynr_sft5x5_bypass = pSelect->ynr_sft5x5_bypass_V2;
    pFix->ynr_flt1x1_bypass = pSelect->ynr_flt1x1_bypass_V2;
    pFix->ynr_lgft3x3_bypass = pSelect->ynr_lgft3x3_bypass_V2;
    pFix->ynr_lbft5x5_bypass = pSelect->ynr_lbft5x5_bypass_V2;
    pFix->ynr_bft3x3_bypass = pSelect->ynr_bft3x3_bypass_V2;
    pFix->ynr_en = pSelect->enable;

    // YNR_2700_RNR_MAX_R  (0x0004)
    int rows = pExpInfo->rawHeight; //raw height
    int cols = pExpInfo->rawWidth; //raw  width
    float r_sq_inv = 16.0f / (cols * cols + rows * rows); // divide 2
    float r_sq_inv_log = LOG2(r_sq_inv);
    int E = abs(ceil(r_sq_inv_log));
    int M = ROUND_F((pow(2, r_sq_inv_log + E)) * 256);
    tmp = (M << 5) + E; // check: (M/256)/(1<<E) = r_sq_inv
    pFix->ynr_rnr_max_r = CLIP(tmp, 0, 0x3fff);

    // YNR_2700_LOWNR_CTRL0 (0x0010)
    tmp = (int)(1.0f / pSelect->ynr_low_bf_V2[1] / fStrength * (1 << 9));
    pFix->ynr_low_bf_inv[1] = CLIP(tmp, 0, 0x3fff);
    tmp = (int)(1.0f / pSelect->ynr_low_bf_V2[0] / fStrength * (1 << 9));
    pFix->ynr_low_bf_inv[0] = CLIP(tmp, 0, 0x3fff);


    // YNR_2700_LOWNR_CTRL1  (0x0014)
    tmp = (int)(pSelect->ynr_low_peak_supress_V2 / fStrength * (1 << 7));
    pFix->ynr_low_peak_supress = CLIP(tmp, 0, 0x80);
    tmp = (int)(pSelect->ynr_low_thred_adj_V2  * (1 << 6));
    pFix->ynr_low_thred_adj = CLIP(tmp, 0, 0x7ff);

    // YNR_2700_LOWNR_CTRL2 (0x0018)
    tmp = (int)(pSelect->ynr_low_dist_adj_V2 * (1 << 2));
    pFix->ynr_low_dist_adj = CLIP(tmp, 0, 0x1ff);
    tmp = (int)(pSelect->ynr_low_edge_adj_thresh_V2);
    pFix->ynr_low_edge_adj_thresh = CLIP(tmp, 0, 0x3ff);


    // YNR_2700_LOWNR_CTRL3 (0x001c)
    tmp = (int)(pSelect->ynr_low_bi_weight_V2 * fStrength * (1 << 7));
    pFix->ynr_low_bi_weight = CLIP(tmp, 0, 0x80);
    tmp = (int)(pSelect->ynr_low_weight_V2 *  fStrength * (1 << 7));
    pFix->ynr_low_weight = CLIP(tmp, 0, 0x80);
    tmp = (int)(pSelect->ynr_low_center_weight_V2 / fStrength * (1 << 10));
    pFix->ynr_low_center_weight = CLIP(tmp, 0, 0x400);

    // YNR_2700_HIGHNR_CTRL0 (0x0020)
    tmp = (int)(pSelect->ynr_hi_min_adj_V2 / fStrength * (1 << 6));
    pFix->ynr_hi_min_adj = CLIP(tmp, 0, 0x3f);
    tmp = (int)(pSelect->ynr_high_thred_adj_V2 * fStrength * (1 << 6));
    pFix->ynr_high_thred_adj = CLIP(tmp, 0, 0x7ff);

    // YNR_2700_HIGHNR_CTRL1  (0x0024)
    tmp = (1 << 7) - (int)(pSelect->ynr_high_weight_V2 * fStrength * (1 << 7));
    pFix->ynr_high_retain_weight = CLIP(tmp, 0, 0x80);
    tmp = (int)(pSelect->ynr_hi_edge_thed_V2 / fStrength);
    pFix->ynr_hi_edge_thed = CLIP(tmp, 0, 0xff);

    // YNR_2700_HIGHNR_BASE_FILTER_WEIGHT  (0x0028)
    w2 = int(pSelect->ynr_base_filter_weight3_V2 * 64 / 2 + 0.5);
    w1 = int(pSelect->ynr_base_filter_weight2_V2 * 64 / 2 + 0.5);
    w0 = 64 - w1 * 2 - w2 * 2;
    pFix->ynr_base_filter_weight[0] = CLIP(w0, 0, 0x40);
    pFix->ynr_base_filter_weight[1] = CLIP(w1, 0, 0x1f);
    pFix->ynr_base_filter_weight[2] = CLIP(w2, 0, 0xf);

    // YNR_2700_GAUSS1_COEFF  (0x0030)
    float filter1_sigma = pSelect->ynr_low_filt1_strength_V2;
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
    float filter2_sigma = pSelect->ynr_low_filt2_strength_V2;
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
        tmp = (int)(pSelect->ynr_direction_weight_V2[i] * (1 << 4));
        pFix->ynr_direction_weight[i] = CLIP(tmp, 0, 0x10);
    }

    // YNR_2700_SGM_DX_0_1 (0x0040 - 0x0060)
    // YNR_2700_LSGM_Y_0_1 (0x0070- 0x0090)
    // YNR_2700_HSGM_Y_0_1 (0x00a0- 0x00c0)
    for (int i = 0; i < YNR_V2_ISO_CURVE_POINT_NUM; i++) {
        tmp = pSelect->lumaPoints_V2[i];
        pFix->ynr_luma_points_x[i] = CLIP(tmp, 0, 0x400);
        tmp = (int)(pSelect->noiseSigma_V2[i] * pSelect->ciISO_V2[0] * (1 << YNR_V2_NOISE_SIGMA_FIX_BIT));
        pFix->ynr_lsgm_y[i] = CLIP(tmp, 0, 0xfff);
        tmp = (int)(pSelect->noiseSigma_V2[i] * pSelect->ciISO_V2[1] * (1 << YNR_V2_NOISE_SIGMA_FIX_BIT));
        pFix->ynr_hsgm_y[i] = CLIP(tmp, 0, 0xfff);
    }

    // YNR_2700_RNR_STRENGTH03 (0x00d0- 0x00e0)
    for (int i = 0; i < 17; i++) {
        tmp = int(pSelect->ynr_rnr_strength_V2[i] * 16);
        pFix->ynr_rnr_strength[i] = CLIP(tmp, 0, 0xff);
    }

    ynr_fix_printf_V2(pFix);
    return res;
}

Aynr_result_t ynr_fix_printf_V2(RK_YNR_Fix_V2_t * pFix)
{
    LOGD_ANR("%s:(%d) enter \n", __FUNCTION__, __LINE__);

    Aynr_result_t res = AYNR_RET_SUCCESS;

    if(pFix == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNR_RET_NULL_POINTER;
    }


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
    LOGD_ANR("(0x0004) ynr_rnr_max_r:0x%x \n",
             pFix->ynr_rnr_max_r);

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



Aynr_result_t ynr_get_setting_by_name_json_V2(CalibDbV2_YnrV2_t *pCalibdbV2, char *name, int* calib_idx, int* tuning_idx)
{
    int i = 0;
    Aynr_result_t res = AYNR_RET_SUCCESS;

    if(pCalibdbV2 == NULL || name == NULL
            || calib_idx == NULL || tuning_idx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNR_RET_NULL_POINTER;
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

Aynr_result_t ynr_init_params_json_V2(RK_YNR_Params_V2_t *pYnrParams, CalibDbV2_YnrV2_t *pCalibdbV2, int calib_idx, int tuning_idx)
{
    Aynr_result_t res = AYNR_RET_SUCCESS;
    int i = 0;
    int j = 0;
    short isoCurveSectValue;
    short isoCurveSectValue1;
    float ave1, ave2, ave3, ave4;
    int bit_calib = 12;
    int bit_proc;
    int bit_shift;

    CalibDbV2_YnrV2_T_ISO_t *pISO = NULL;
    CalibDbV2_YnrV2_C_ISO_t *pCalibISO = NULL;

    LOGD_ANR("%s(%d): enter\n", __FUNCTION__, __LINE__);

    if(pYnrParams == NULL || pCalibdbV2 == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNR_RET_NULL_POINTER;
    }

    bit_proc = YNR_V2_SIGMA_BITS; // for V3, YNR_SIGMA_BITS = 10
    bit_shift = bit_calib - bit_proc;

    isoCurveSectValue = (1 << (bit_calib - YNR_V2_ISO_CURVE_POINT_BIT));
    isoCurveSectValue1 = (1 << bit_calib);


    for(j = 0; j < pCalibdbV2->CalibPara.Setting[tuning_idx].Calib_ISO_len && j < RK_YNR_V2_MAX_ISO_NUM ; j++) {
        pCalibISO = &pCalibdbV2->CalibPara.Setting[tuning_idx].Calib_ISO[j];
        pYnrParams->iso[j] = pCalibISO->iso;

        // get noise sigma sample data at [0, 64, 128, ... , 1024]
        for (i = 0; i < YNR_V2_ISO_CURVE_POINT_NUM; i++) {
            if (i == (YNR_V2_ISO_CURVE_POINT_NUM - 1)) {
                ave1 = (float)isoCurveSectValue1;
            } else {
                ave1 = (float)(i * isoCurveSectValue);
            }
            pYnrParams->arYnrParamsISO[j].lumaPoints_V2[i] = (short)ave1;
            ave2 = ave1 * ave1;
            ave3 = ave2 * ave1;
            ave4 = ave3 * ave1;
            pYnrParams->arYnrParamsISO[j].noiseSigma_V2[i] = pCalibISO->sigma_curve[0] * ave4
                    + pCalibISO->sigma_curve[1] * ave3
                    + pCalibISO->sigma_curve[2] * ave2
                    + pCalibISO->sigma_curve[3] * ave1
                    + pCalibISO->sigma_curve[4];

            if (pYnrParams->arYnrParamsISO[j].noiseSigma_V2[i] < 0) {
                pYnrParams->arYnrParamsISO[j].noiseSigma_V2[i] = 0;
            }

            if (bit_shift > 0) {
                pYnrParams->arYnrParamsISO[j].lumaPoints_V2[i] >>= bit_shift;
            } else {
                pYnrParams->arYnrParamsISO[j].lumaPoints_V2[i] <<= ABS(bit_shift);
            }
        }

        pYnrParams->arYnrParamsISO[j].ciISO_V2[0] = pCalibISO->ynr_ci_l;
        pYnrParams->arYnrParamsISO[j].ciISO_V2[1] = pCalibISO->ynr_ci_h;
    }

    LOGD_ANR(" iso len:%d calib_max:%d\n", pCalibdbV2->TuningPara.Setting[tuning_idx].Tuning_ISO_len, RK_YNR_V2_MAX_ISO_NUM);

    for(j = 0; j < pCalibdbV2->TuningPara.Setting[tuning_idx].Tuning_ISO_len && j < RK_YNR_V2_MAX_ISO_NUM; j++) {
        pISO = &pCalibdbV2->TuningPara.Setting[tuning_idx].Tuning_ISO[j];

        for(i = 0; i < 17; i++) {
            pYnrParams->arYnrParamsISO[j].ynr_rnr_strength_V2[i] = pISO->rnr_strength[i];
        }

        pYnrParams->arYnrParamsISO[j].ynr_bft3x3_bypass_V2 = pISO->ynr_bft3x3_bypass;
        pYnrParams->arYnrParamsISO[j].ynr_lbft5x5_bypass_V2 = pISO->ynr_lbft5x5_bypass;
        pYnrParams->arYnrParamsISO[j].ynr_lgft3x3_bypass_V2 = pISO->ynr_lgft3x3_bypass;
        pYnrParams->arYnrParamsISO[j].ynr_flt1x1_bypass_V2 = pISO->ynr_flt1x1_bypass;
        pYnrParams->arYnrParamsISO[j].ynr_sft5x5_bypass_V2 = pISO->ynr_sft5x5_bypass;

        pYnrParams->arYnrParamsISO[j].ynr_low_bf_V2[0] = pISO->low_bf_0;
        pYnrParams->arYnrParamsISO[j].ynr_low_bf_V2[1] = pISO->low_bf_1;
        pYnrParams->arYnrParamsISO[j].ynr_low_thred_adj_V2 = pISO->low_thred_adj;
        pYnrParams->arYnrParamsISO[j].ynr_low_peak_supress_V2 = pISO->low_peak_supress;
        pYnrParams->arYnrParamsISO[j].ynr_low_edge_adj_thresh_V2 = pISO->low_edge_adj_thresh;;
        pYnrParams->arYnrParamsISO[j].ynr_low_center_weight_V2 = pISO->low_center_weight;
        pYnrParams->arYnrParamsISO[j].ynr_low_dist_adj_V2 = pISO->low_dist_adj;
        pYnrParams->arYnrParamsISO[j].ynr_low_weight_V2 = pISO->low_weight;
        pYnrParams->arYnrParamsISO[j].ynr_low_filt1_strength_V2 = pISO->low_filt_strength_0;
        pYnrParams->arYnrParamsISO[j].ynr_low_filt2_strength_V2 = pISO->low_filt_strength_1;
        pYnrParams->arYnrParamsISO[j].ynr_low_bi_weight_V2 = pISO->low_bi_weight;

        pYnrParams->arYnrParamsISO[j].ynr_base_filter_weight1_V2 = pISO->base_filter_weight_0;
        pYnrParams->arYnrParamsISO[j].ynr_base_filter_weight2_V2 = pISO->base_filter_weight_1;
        pYnrParams->arYnrParamsISO[j].ynr_base_filter_weight3_V2 = pISO->base_filter_weight_2;
        pYnrParams->arYnrParamsISO[j].ynr_high_thred_adj_V2 = pISO->high_thred_adj;
        pYnrParams->arYnrParamsISO[j].ynr_high_weight_V2 = pISO->high_weight;
        for(i = 0; i < 8; i++) {
            pYnrParams->arYnrParamsISO[j].ynr_direction_weight_V2[i] = pISO->high_direction_weight[i];
        }
        pYnrParams->arYnrParamsISO[j].ynr_hi_min_adj_V2 = pISO->hi_min_adj;
        pYnrParams->arYnrParamsISO[j].ynr_hi_edge_thed_V2 = pISO->hi_edge_thed;

    }

    LOGD_ANR("%s(%d): exit\n", __FUNCTION__, __LINE__);

    return res;
}

Aynr_result_t ynr_config_setting_param_json_V2(RK_YNR_Params_V2_t *pParams, CalibDbV2_YnrV2_t *pCalibdbV2, char* param_mode, char * snr_name)
{
    Aynr_result_t res = AYNR_RET_SUCCESS;
    int calib_idx = 0;
    int tuning_idx = 0;

    if(pParams == NULL || pCalibdbV2 == NULL
            || param_mode == NULL || snr_name == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNR_RET_NULL_POINTER;
    }

    res = ynr_get_setting_by_name_json_V2(pCalibdbV2, snr_name, &calib_idx, &tuning_idx);
    if(res != AYNR_RET_SUCCESS) {
        LOGW_ANR("%s(%d): error!!!  can't find setting in iq files, use 0 instead\n", __FUNCTION__, __LINE__);
    }

    res = ynr_init_params_json_V2(pParams, pCalibdbV2, calib_idx, tuning_idx);
    pParams->enable = pCalibdbV2->TuningPara.enable;
    return res;

}

RKAIQ_END_DECLARE

