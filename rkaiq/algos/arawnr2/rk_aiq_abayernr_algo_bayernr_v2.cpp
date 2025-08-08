

#include "rk_aiq_abayernr_algo_bayernr_v2.h"

RKAIQ_BEGIN_DECLARE

Abayernr_result_t bayernr_get_mode_by_name_V2(struct list_head *pCalibdbList, char *name, CalibDb_Bayernr_V2_t** ppProfile)
{
    Abayernr_result_t res = ABAYERNR_RET_SUCCESS;

    if(pCalibdbList == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERNR_RET_NULL_POINTER;
    }

    if(name == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERNR_RET_NULL_POINTER;
    }

    if(ppProfile == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERNR_RET_NULL_POINTER;
    }

#if 1
    *ppProfile = NULL;
    struct list_head* p;
    p = pCalibdbList->next;
    while (p != pCalibdbList)
    {
        CalibDb_Bayernr_V2_t* pProfile = container_of(p, CalibDb_Bayernr_V2_t, listItem);
        LOGE_ANR("%s:%d %s  %p \n",
                 __FUNCTION__, __LINE__, pProfile->modeName, p);
        if (!strncmp(pProfile->modeName, name, sizeof(pProfile->modeName))) {
            *ppProfile = pProfile;
            return res;
        }
        p = p->next;
    }

    CalibDb_Bayernr_V2_t* pProfile = container_of(pCalibdbList->next, CalibDb_Bayernr_V2_t, listItem);
    *ppProfile = pProfile;
#else


#endif

    return res;

}

Abayernr_result_t bayernr2D_get_setting_by_name_V2(struct list_head *pSettingList, char *name, Calibdb_Bayernr_2Dparams_V2_t** ppSetting)
{
    Abayernr_result_t res = ABAYERNR_RET_SUCCESS;

    if(pSettingList == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERNR_RET_NULL_POINTER;
    }

    if(name == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERNR_RET_NULL_POINTER;
    }

    if(ppSetting == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERNR_RET_NULL_POINTER;
    }

    *ppSetting = NULL;

    struct list_head* p;
    p = pSettingList->next;
    while (p != pSettingList)
    {
        Calibdb_Bayernr_2Dparams_V2_t* pSetting = container_of(p, Calibdb_Bayernr_2Dparams_V2_t, listItem);
        LOGD_ANR("%s:%d:  %s  %p ",
                 __FUNCTION__, __LINE__, pSetting->snr_mode, p);
        if (!strncmp(pSetting->snr_mode, name, sizeof(pSetting->snr_mode))) {
            *ppSetting = pSetting;
            return res;
        }
        p = p->next;
    }

    Calibdb_Bayernr_2Dparams_V2_t* pSetting = container_of(pSettingList->next, Calibdb_Bayernr_2Dparams_V2_t, listItem);
    *ppSetting = pSetting;
    return res;

}

Abayernr_result_t bayernr3D_get_setting_by_name_V2(struct list_head *pSettingList, char *name, CalibDb_Bayernr_3DParams_V2_t** ppSetting)
{
    Abayernr_result_t res = ABAYERNR_RET_SUCCESS;

    if(pSettingList == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERNR_RET_NULL_POINTER;
    }

    if(name == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERNR_RET_NULL_POINTER;
    }

    if(ppSetting == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERNR_RET_NULL_POINTER;
    }

    *ppSetting = NULL;

    struct list_head* p;
    p = pSettingList->next;
    while (p != pSettingList)
    {
        CalibDb_Bayernr_3DParams_V2_t* pSetting = container_of(p, CalibDb_Bayernr_3DParams_V2_t, listItem);
        LOGD_ANR("%s:%d %s  %p ",
                 __FUNCTION__, __LINE__, pSetting->snr_mode, p);
        if (!strncmp(pSetting->snr_mode, name, sizeof(pSetting->snr_mode))) {
            *ppSetting = pSetting;
            return res;
        }
        p = p->next;
    }

    CalibDb_Bayernr_3DParams_V2_t* pSetting = container_of(pSettingList->next, CalibDb_Bayernr_3DParams_V2_t, listItem);
    *ppSetting = pSetting;

    return res;

}



Abayernr_result_t bayernr2D_config_setting_param_V2(RK_Bayernr_2D_Params_V2_t *pParams, struct list_head *pCalibdbList, char* param_mode, char * snr_name)
{
    Abayernr_result_t res = ABAYERNR_RET_SUCCESS;
    CalibDb_Bayernr_V2_t *pProfile;
    Calibdb_Bayernr_2Dparams_V2_t* p2DParams;

    LOGI_ANR("%s(%d): enter\n", __FUNCTION__, __LINE__);
    if(pParams == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERNR_RET_NULL_POINTER;
    }

    if(pCalibdbList == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERNR_RET_NULL_POINTER;
    }

    if(param_mode == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERNR_RET_NULL_POINTER;
    }

    if(snr_name == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERNR_RET_NULL_POINTER;
    }

    res = bayernr_get_mode_by_name_V2(pCalibdbList, param_mode, &pProfile);
    if(res != ABAYERNR_RET_SUCCESS) {
        LOGW_ANR("%s(%d): error!!!  can't find mode name in iq files, use 0 instead\n", __FUNCTION__, __LINE__);
    }

    res = bayernr2D_get_setting_by_name_V2(&pProfile->st2DParams.listHead, snr_name, &p2DParams);
    if(res != ABAYERNR_RET_SUCCESS) {
        LOGW_ANR("%s(%d): error!!!  can't find setting in iq files, use 0 instead\n", __FUNCTION__, __LINE__);
    }

    res = bayernr2D_init_params_V2(pParams, p2DParams);
    pParams->bayernrv2_2dnr_enable = pProfile->st2DParams.bayernrv2_2dnr_enable;

    LOGI_ANR("%s(%d): exit\n", __FUNCTION__, __LINE__);
    return res;

}

Abayernr_result_t bayernr3D_config_setting_param_V2(RK_Bayernr_3D_Params_V2_t *pParams, struct list_head *pCalibdbList, char* param_mode, char * snr_name)
{
    Abayernr_result_t res = ABAYERNR_RET_SUCCESS;
    CalibDb_Bayernr_V2_t *pProfile;
    CalibDb_Bayernr_3DParams_V2_t* p3DParams;

    if(pParams == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERNR_RET_NULL_POINTER;
    }

    if(pCalibdbList == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERNR_RET_NULL_POINTER;
    }

    if(param_mode == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERNR_RET_NULL_POINTER;
    }

    if(snr_name == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERNR_RET_NULL_POINTER;
    }

    res = bayernr_get_mode_by_name_V2(pCalibdbList, param_mode, &pProfile);
    if(res != ABAYERNR_RET_SUCCESS) {
        LOGW_ANR("%s(%d): error!!!  can't find mode name in iq files, use 0 instead\n", __FUNCTION__, __LINE__);
    }

    res = bayernr3D_get_setting_by_name_V2(&pProfile->st3DParams.listHead, snr_name, &p3DParams);
    if(res != ABAYERNR_RET_SUCCESS) {
        LOGW_ANR("%s(%d): error!!!  can't find setting in iq files, use 0 instead\n", __FUNCTION__, __LINE__);

    }

    res = bayernr3D_init_params_V2(pParams, p3DParams);
    pParams->bayernrv2_tnr_enable = pProfile->st3DParams.bayernrv2_tnr_enable;
    return res;

}


Abayernr_result_t bayernr2D_init_params_V2(RK_Bayernr_2D_Params_V2_t *pParams, Calibdb_Bayernr_2Dparams_V2_t* pCalibdbParams)
{
    Abayernr_result_t res = ABAYERNR_RET_SUCCESS;

    LOGI_ANR("%s:(%d) oyyf bayerner xml config start\n", __FUNCTION__, __LINE__);
    if(pParams == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERNR_RET_NULL_POINTER;
    }

    if(pCalibdbParams == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERNR_RET_NULL_POINTER;
    }

    for(int i = 0; i < RK_BAYERNR_V2_MAX_ISO_NUM; i++) {
        pParams->iso[i] = pCalibdbParams->iso[i];
        pParams->bayernrv2_filter_strength_r[i] = pCalibdbParams->bayernrv2_filter_strength_r[i];
        pParams->bayernrv2_filter_edgesofts_r[i] = pCalibdbParams->bayernrv2_filter_edgesofts_r[i];
        pParams->bayernrv2_filter_out_wgt_r[i] = pCalibdbParams->bayernrv2_filter_out_wgt_r[i];
        pParams->bayernrv2_filter_soft_threshold_ratio_r[i] = pCalibdbParams->bayernrv2_filter_soft_threshold_ratio_r[i];
        pParams->bayernrv2_gauss_guide_r[i] = pCalibdbParams->bayernrv2_gauss_guide_r[i];

        for(int k = 0; k < 8; k++) {
            pParams->bayernrv2_edge_filter_wgt_r[i][k] = pCalibdbParams->bayernrv2_edge_filter_wgt_r[i][k];
        }
        for(int k = 0; k < 16; k++) {
            pParams->bayernrv2_filter_sigma_r[i][k] = pCalibdbParams->bayernrv2_filter_sigma_r[i][k];
        }
    }

    for(int i = 0; i < 8; i++) {
        pParams->bayernrv2_edge_filter_lumapoint_r[i] = pCalibdbParams->bayernrv2_edge_filter_lumapoint_r[i];
    }

    for(int i = 0; i < 16; i++) {
        pParams->bayernrv2_filter_lumapoint_r[i] = pCalibdbParams->bayernrv2_filter_lumapoint_r[i];
    }

    LOGI_ANR("%s:(%d) oyyf bayerner xml config end!   \n", __FUNCTION__, __LINE__);

    return res;
}

Abayernr_result_t bayernr3D_init_params_V2(RK_Bayernr_3D_Params_V2_t *pParams, CalibDb_Bayernr_3DParams_V2_t* pCalibdbParams)
{
    Abayernr_result_t res = ABAYERNR_RET_SUCCESS;
    int i = 0;

    LOGI_ANR("%s:(%d) oyyf bayerner xml config start\n", __FUNCTION__, __LINE__);
    if(pParams == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERNR_RET_NULL_POINTER;
    }

    if(pCalibdbParams == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERNR_RET_NULL_POINTER;
    }

    for(i = 0; i < RK_BAYERNR_V2_MAX_ISO_NUM; i++) {
        pParams->iso[i] = pCalibdbParams->iso[i];
        pParams->bayernrv2_tnr_filter_strength_r[i] = pCalibdbParams->bayernrv2_tnr_filter_strength_r[i];
        pParams->bayernrv2_tnr_lo_clipwgt_r[i] = pCalibdbParams->bayernrv2_tnr_lo_clipwgt_r[i];
        pParams->bayernrv2_tnr_hi_clipwgt_r[i] = pCalibdbParams->bayernrv2_tnr_hi_clipwgt_r[i];
        pParams->bayernrv2_tnr_softwgt_r[i] = pCalibdbParams->bayernrv2_tnr_softwgt_r[i];

        for(int k = 0; k < 16; k++) {
            pParams->bayernrv2_sigma_r[i][k] = pCalibdbParams->bayernrv2_sigma_r[i][k];
        }

    }

    for(i = 0; i < 16; i++) {
        pParams->bayernrv2_lumapoint_r[i] = pCalibdbParams->bayernrv2_lumapoint_r[i];
    }

    LOGI_ANR("%s:(%d) oyyf bayerner xml config end!   \n", __FUNCTION__, __LINE__);

    return res;
}


Abayernr_result_t bayernr2D_select_params_by_ISO_V2(RK_Bayernr_2D_Params_V2_t *pParams, RK_Bayernr_2D_Params_V2_Select_t *pSelect, Abayernr_ExpInfo_t *pExpInfo)
{
    Abayernr_result_t res = ABAYERNR_RET_SUCCESS;
    int iso = 50;

    if(pParams == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERNR_RET_NULL_POINTER;
    }

    if(pSelect == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERNR_RET_NULL_POINTER;
    }

    if(pExpInfo == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERNR_RET_NULL_POINTER;
    }

    iso = pExpInfo->arIso[pExpInfo->hdr_mode];

    LOGD_ANR("%s:%d iso:%d \n", __FUNCTION__, __LINE__, iso);

    int isoGainStd[RK_BAYERNR_V2_MAX_ISO_NUM];
    int isoGain = MAX(int(iso / 50), 1);
    int isoGainLow = 0;
    int isoGainHig = 0;
    int isoLevelLow = 0;
    int isoLevelHig = 0;
    int i;
    float tmpf;

#ifndef RK_SIMULATOR_HW
    for(int i = 0; i < RK_BAYERNR_V2_MAX_ISO_NUM; i++) {
        isoGainStd[i] = pParams->iso[i] / 50;
    }
#else
    for(int i = 0; i < RK_BAYERNR_V2_MAX_ISO_NUM; i++) {
        isoGainStd[i] = 1 * (1 << i);
    }
#endif

    for (i = 0; i < RK_BAYERNR_V2_MAX_ISO_NUM - 1; i++)
    {
        if (isoGain >= isoGainStd[i] && isoGain <= isoGainStd[i + 1])
        {
            isoGainLow = isoGainStd[i];
            isoGainHig = isoGainStd[i + 1];
            isoLevelLow = i;
            isoLevelHig = i + 1;
        }
    }

    LOGD_ANR("%s:%d iso:%d high:%d low:%d\n",
             __FUNCTION__, __LINE__,
             isoGain, isoGainHig, isoGainLow);

    pSelect->bayernrv2_2dnr_enable = pParams->bayernrv2_2dnr_enable;

    for (i = 0; i < 8; i++)
    {
        pSelect->bayernrv2_edge_filter_lumapoint[i] = pParams->bayernrv2_edge_filter_lumapoint_r[i];
        pSelect->bayernrv2_edge_filter_wgt[i] = float(isoGainHig - isoGain) / float(isoGainHig - isoGainLow) * pParams->bayernrv2_edge_filter_wgt_r[isoLevelLow][i]
                                                + float(isoGain - isoGainLow) / float(isoGainHig - isoGainLow) * pParams->bayernrv2_edge_filter_wgt_r[isoLevelHig][i];
    }

    pSelect->bayernrv2_filter_strength = float(isoGainHig - isoGain) / float(isoGainHig - isoGainLow) * pParams->bayernrv2_filter_strength_r[isoLevelLow]
                                         + float(isoGain - isoGainLow) / float(isoGainHig - isoGainLow) * pParams->bayernrv2_filter_strength_r[isoLevelHig];

    tmpf = float(isoGainHig - isoGain) / float(isoGainHig - isoGainLow) * pParams->bayernrv2_gauss_guide_r[isoLevelLow]
           + float(isoGain - isoGainLow) / float(isoGainHig - isoGainLow) * pParams->bayernrv2_gauss_guide_r[isoLevelHig];
    pSelect->bayernrv2_gauss_guide = tmpf != 0;

    for (i = 0; i < 16; i++)
    {
        pSelect->bayernrv2_filter_lumapoint[i] = pParams->bayernrv2_filter_lumapoint_r[i];
        pSelect->bayernrv2_filter_sigma[i] = float(isoGainHig - isoGain) / float(isoGainHig - isoGainLow) * pParams->bayernrv2_filter_sigma_r[isoLevelLow][i]
                                             + float(isoGain - isoGainLow) / float(isoGainHig - isoGainLow) * pParams->bayernrv2_filter_sigma_r[isoLevelHig][i];
    }

    pSelect->bayernrv2_filter_edgesofts = float(isoGainHig - isoGain) / float(isoGainHig - isoGainLow) * pParams->bayernrv2_filter_edgesofts_r[isoLevelLow]
                                          + float(isoGain - isoGainLow) / float(isoGainHig - isoGainLow) * pParams->bayernrv2_filter_edgesofts_r[isoLevelHig];
    pSelect->bayernrv2_filter_soft_threshold_ratio = float(isoGainHig - isoGain) / float(isoGainHig - isoGainLow) * pParams->bayernrv2_filter_soft_threshold_ratio_r[isoLevelLow]
            + float(isoGain - isoGainLow) / float(isoGainHig - isoGainLow) * pParams->bayernrv2_filter_soft_threshold_ratio_r[isoLevelHig];
    pSelect->bayernrv2_filter_out_wgt = float(isoGainHig - isoGain) / float(isoGainHig - isoGainLow) * pParams->bayernrv2_filter_out_wgt_r[isoLevelLow]
                                        + float(isoGain - isoGainLow) / float(isoGainHig - isoGainLow) * pParams->bayernrv2_filter_out_wgt_r[isoLevelHig];


    pSelect->bayernrv2_edge_filter_en = 0;

    return res;
}




Abayernr_result_t bayernr3D_select_params_by_ISO_V2(RK_Bayernr_3D_Params_V2_t *pParams, RK_Bayernr_3D_Params_V2_Select_t *pSelect, Abayernr_ExpInfo_t *pExpInfo)
{
    Abayernr_result_t res = ABAYERNR_RET_SUCCESS;
    int iso = 50;

    if(pParams == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERNR_RET_NULL_POINTER;
    }

    if(pSelect == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERNR_RET_NULL_POINTER;
    }

    if(pExpInfo == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERNR_RET_NULL_POINTER;
    }

    iso = pExpInfo->arIso[pExpInfo->hdr_mode];

    LOGD_ANR("%s:%d iso:%d \n", __FUNCTION__, __LINE__, iso);

    int isoGainStd[RK_BAYERNR_V2_MAX_ISO_NUM];
    int isoGain = MAX(int(iso / 50), 1);
    int isoGainLow = 0;
    int isoGainHig = 0;
    int isoLevelLow = 0;
    int isoLevelHig = 0;
    int i;

#ifndef RK_SIMULATOR_HW
    for(int i = 0; i < RK_BAYERNR_V2_MAX_ISO_NUM; i++) {
        isoGainStd[i] = pParams->iso[i] / 50;
    }
#else
    for(int i = 0; i < RK_BAYERNR_V2_MAX_ISO_NUM; i++) {
        isoGainStd[i] = 1 * (1 << i);
    }
#endif

    for (i = 0; i < RK_BAYERNR_V2_MAX_ISO_NUM - 1; i++) {
        if (isoGain >= isoGainStd[i] && isoGain <= isoGainStd[i + 1]) {
            isoGainLow = isoGainStd[i];
            isoGainHig = isoGainStd[i + 1];
            isoLevelLow = i;
            isoLevelHig = i + 1;
        }
    }

    LOGD_ANR("%s:%d iso:%d high:%d low:%d\n",
             __FUNCTION__, __LINE__,
             isoGain, isoGainHig, isoGainLow);

    pSelect->bayernrv2_tnr_enable = pParams->bayernrv2_tnr_enable;

    pSelect->bayernrv2_tnr_filter_strength = float(isoGainHig - isoGain) / float(isoGainHig - isoGainLow) * pParams->bayernrv2_tnr_filter_strength_r[isoLevelLow]
            + float(isoGain - isoGainLow) / float(isoGainHig - isoGainLow) * pParams->bayernrv2_tnr_filter_strength_r[isoLevelHig];
    pSelect->bayernrv2_tnr_lo_clipwgt = float(isoGainHig - isoGain) / float(isoGainHig - isoGainLow) * pParams->bayernrv2_tnr_lo_clipwgt_r[isoLevelLow]
                                        + float(isoGain - isoGainLow) / float(isoGainHig - isoGainLow) * pParams->bayernrv2_tnr_lo_clipwgt_r[isoLevelHig];
    pSelect->bayernrv2_tnr_hi_clipwgt = float(isoGainHig - isoGain) / float(isoGainHig - isoGainLow) * pParams->bayernrv2_tnr_hi_clipwgt_r[isoLevelLow]
                                        + float(isoGain - isoGainLow) / float(isoGainHig - isoGainLow) * pParams->bayernrv2_tnr_hi_clipwgt_r[isoLevelHig];
    pSelect->bayernrv2_tnr_softwgt    = float(isoGainHig - isoGain) / float(isoGainHig - isoGainLow) * pParams->bayernrv2_tnr_softwgt_r[isoLevelLow]
                                        + float(isoGain - isoGainLow) / float(isoGainHig - isoGainLow) * pParams->bayernrv2_tnr_softwgt_r[isoLevelHig];

    for (i = 0; i < 16; i++) {
        pSelect->bayernrv2_tnr_lumapoint[i] = pParams->bayernrv2_lumapoint_r[i];
        pSelect->bayernrv2_tnr_sigma[i] = float(isoGainHig - isoGain) / float(isoGainHig - isoGainLow) * pParams->bayernrv2_sigma_r[isoLevelLow][i]
                                          + float(isoGain - isoGainLow) / float(isoGainHig - isoGainLow) * pParams->bayernrv2_sigma_r[isoLevelHig][i];
    }

    return res;
}


unsigned short bayernr_get_trans_V2(int tmpfix)
{
    int logtablef[65] = {0, 1465, 2909, 4331, 5731, 7112, 8472, 9813, 11136, 12440,
                         13726, 14995, 16248, 17484, 18704, 19908, 21097, 22272, 23432, 24578, 25710,
                         26829, 27935, 29028, 30109, 31177, 32234, 33278, 34312, 35334, 36345, 37346,
                         38336, 39315, 40285, 41245, 42195, 43136, 44068, 44990, 45904, 46808, 47704,
                         48592, 49472, 50343, 51207, 52062, 52910, 53751, 54584, 55410, 56228, 57040,
                         57844, 58642, 59433, 60218, 60996, 61768, 62534, 63293, 64047, 64794, 65536
                        };
    int logprecision = 6;
    int logfixbit = 16;
    int logtblbit = 16;
    int logscalebit = 12;
    int logfixmul = (1 << logfixbit);
    long long x8, one = 1;
    long long gx, n = 0, ix1, ix2, dp;
    long long lt1, lt2, dx, fx;
    int i, j = 1;

    x8 = tmpfix + (1 << 8);
    // find highest bit
    for (i = 0; i < 32; i++)
    {
        if (x8 & j)
        {
            n = i;
        }
        j = j << 1;
    }

    gx = x8 - (one << n);
    gx = gx * (one << logprecision) * logfixmul;
    gx = gx / (one << n);

    ix1 = gx >> logfixbit;
    dp = gx - ix1 * logfixmul;

    ix2 = ix1 + 1;

    lt1 = logtablef[ix1];
    lt2 = logtablef[ix2];

    dx = lt1 * (logfixmul - dp) + lt2 * dp;

    fx = dx + (n - 8) * (one << (logfixbit + logtblbit));
    fx = fx + (one << (logfixbit + logtblbit - logscalebit - 1));
    fx = fx >> (logfixbit + logtblbit - logscalebit);

    return fx;
}

Abayernr_result_t bayernr2D_fix_transfer_V2(RK_Bayernr_2D_Params_V2_Select_t* pSelect, RK_Bayernr_2D_Fix_V2_t *pFix, float fStrength, Abayernr_ExpInfo_t *pExpInfo)
{
    //--------------------------- v2 params ----------------------------//
    float frameiso[3];
    float frameEt[3];
    float fdGain[3];
    int dGain[3] = {0};
    int i = 0;
    int ypos[8] = {4, 4, 4, 3, 3, 2, 2, 1};
    int xpos[8] = {4, 2, 0, 3, 1, 2, 0, 1};
    float tmp1, tmp2, edgesofts;
    int bayernr_sw_bil_gauss_weight[16];
    int tmp;

    if(pSelect == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERNR_RET_NULL_POINTER;
    }

    if(pFix == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERNR_RET_NULL_POINTER;
    }

    if(pExpInfo == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERNR_RET_NULL_POINTER;
    }

    if(fStrength <= 0.0f) {
        fStrength = 0.000001;
    }

    // hdr gain
    int framenum = pExpInfo->hdr_mode + 1;

    frameiso[0] = pExpInfo->arIso[0];
    frameiso[1] = pExpInfo->arIso[1];
    frameiso[2] = pExpInfo->arIso[2];

    frameEt[0] = pExpInfo->arTime[0];
    frameEt[1] = pExpInfo->arTime[1];
    frameEt[2] = pExpInfo->arTime[2];


    for (i = 0; i < framenum; i++) {
        fdGain[i] = frameiso[i] * frameEt[i];
        LOGD_ANR("bayernrv2 idx[%d] iso:%f time:%f dgain:%f\n",
                 i, frameiso[i], frameEt[i], fdGain[i]);
    }

    for (i = 0; i < framenum; i++) {
        fdGain[i] = fdGain[framenum - 1] / fdGain[i];
        dGain[i] = int(fdGain[i] * (1 << FIXNLMCALC));
        LOGD_ANR("bayernrv2 idx[%d] fdgain:%f dgain:%d \n",
                 i, fdGain[i], dGain[i]);
    }

    //ISP_BAYNR_3A00_CTRL
    pFix->baynr_gauss_en = pSelect->bayernrv2_gauss_guide;
    pFix->baynr_log_bypass = 0;
    pFix->baynr_en = pSelect->bayernrv2_2dnr_enable;

    //pFix->bayernr_gray_mode = 0;


    // ISP_BAYNR_3A00_DGAIN0-2
    for(i = 0; i < framenum; i++) {
        tmp = dGain[i] / ( 1 << (FIXNLMCALC - FIXGAINBIT));
        pFix->baynr_dgain[i] = CLIP(tmp, 0, 0xffff);
    }

    // ISP_BAYNR_3A00_PIXDIFF
    tmp = FIXDIFMAX - 1;
    pFix->baynr_pix_diff = CLIP(tmp, 0, 0x3fff);

    // ISP_BAYNR_3A00_THLD
    tmp = LUTPRECISION_FIX;
    pFix->baynr_diff_thld = CLIP(tmp, 0, 0x3ff);
    tmp = (int)(pSelect->bayernrv2_filter_soft_threshold_ratio / fStrength * (1 << 10));
    pFix->baynr_softthld = CLIP(tmp, 0, 0x3ff);

    // ISP_BAYNR_3A00_W1_STRENG
    tmp = (int)(pSelect->bayernrv2_filter_strength * fStrength * (1 << FIXBILSTRG));
    pFix->bltflt_streng = CLIP(tmp, 0, 0xfff);
    tmp = (int)(pSelect->bayernrv2_filter_out_wgt * fStrength * (1 << 10));
    pFix->baynr_reg_w1 = CLIP(tmp, 0, 0x3ff);

    // ISP_BAYNR_3A00_SIGMAX0-15   ISP_BAYNR_3A00_SIGMAY0-15
    for(i = 0; i < 16; i++) {
        //pFix->sigma_x[i] = bayernr_get_trans_V2(pSelect->bayernrv2_filter_lumapoint[i]);
        tmp = pSelect->bayernrv2_filter_lumapoint[i];
        pFix->sigma_x[i] = CLIP(tmp, 0, 0xffff);
        tmp = pSelect->bayernrv2_filter_sigma[i];
        pFix->sigma_y[i] = CLIP(tmp, 0, 0xffff);
    }

    // ISP_BAYNR_3A00_WRIT_D
#if 0
    pFix->weit_d[0] = 0x178;
    pFix->weit_d[1] = 0x249;
    pFix->weit_d[2] = 0x31d;
#else
    edgesofts = pSelect->bayernrv2_filter_edgesofts;
    for(i = 0; i < 8; i++)
    {
        tmp1 = (float)(ypos[i] * ypos[i] + xpos[i] * xpos[i]);
        tmp1 = tmp1 / (2 * edgesofts * edgesofts);
        tmp2 = expf(-tmp1);
        bayernr_sw_bil_gauss_weight[i] = (int)(tmp1 * (EXP2RECISION_FIX / (1 << 7)));
        bayernr_sw_bil_gauss_weight[i + 8] = (int)(tmp2 * (1 << FIXVSTINV));
    }

    if(0) {
        //gray mode
        tmp = bayernr_sw_bil_gauss_weight[12];
        pFix->weit_d[0] = CLIP(tmp, 0, 0x3ff);
        tmp = bayernr_sw_bil_gauss_weight[10];
        pFix->weit_d[1] = CLIP(tmp, 0, 0x3ff);
        tmp = bayernr_sw_bil_gauss_weight[11];
        pFix->weit_d[2] = CLIP(tmp, 0, 0x3ff);

    } else {
        tmp = bayernr_sw_bil_gauss_weight[13];
        pFix->weit_d[0] = CLIP(tmp, 0, 0x3ff);
        tmp = bayernr_sw_bil_gauss_weight[14];
        pFix->weit_d[1] = CLIP(tmp, 0, 0x3ff);
        tmp = bayernr_sw_bil_gauss_weight[15];
        pFix->weit_d[2] = CLIP(tmp, 0, 0x3ff);
    }

#endif



    bayernr2D_fix_printf_V2(pFix);

    return ABAYERNR_RET_SUCCESS;

}

Abayernr_result_t bayernr3D_fix_transfer_V2(RK_Bayernr_3D_Params_V2_Select_t* pSelect, RK_Bayernr_3D_Fix_V2_t *pFix, float fStrength, Abayernr_ExpInfo_t *pExpInfo)
{
    int i = 0;
    int tmp;

    if(fStrength <= 0.0f) {
        fStrength = 0.000001;
    }

    // BAY3D_BAY3D_CTRL
    pFix->bay3d_exp_sel = 1;
    pFix->bay3d_bypass_en = 0;
    pFix->bay3d_pk_en = 1;  // 1 use local pk, 0 use global pk
    pFix->bay3d_en_i = pSelect->bayernrv2_tnr_enable;

    // BAY3D_BAY3D_KALRATIO
    tmp = (int)(pSelect->bayernrv2_tnr_softwgt / fStrength * (1 << FIXTNRSFT));
    pFix->bay3d_softwgt = CLIP(tmp, 0, 0x3ff);

    tmp = (int)(1.5 * (1 << 10));  //(pSelect->bayertnr_sigratio*(1<<FIXBILSTRG));
    pFix->bay3d_sigratio = CLIP(tmp, 0, 0x3fff);

    // BAY3D_BAY3D_GLBPK2
    tmp = 1024;
    pFix->bay3d_glbpk2 = CLIP(tmp, 0, 0xfffffff);

    // BAY3D_BAY3D_KALSTR
    tmp = (int)(pSelect->bayernrv2_tnr_filter_strength / fStrength * (1 << 10));//  (int)(0.06*(1<<10));    // less, filter strong  // classic is 0.1
    pFix->bay3d_exp_str = CLIP(tmp, 0, 0x3ff);
    tmp = (int)(1.0 * (1 << FIXTNRSTG));
    pFix->bay3d_str = CLIP(tmp, 0, 0x100);

    // BAY3D_BAY3D_WGTLMT
    tmp = (int)(((float)1 - pSelect->bayernrv2_tnr_hi_clipwgt) * fStrength * (1 << FIXTNRWGT));    // 0.0325
    pFix->bay3d_wgtlmt_h = CLIP(tmp, 0, 0x3ff);
    tmp = (int)(((float)1 - pSelect->bayernrv2_tnr_lo_clipwgt) * fStrength  * (1 << FIXTNRWGT));    // 0.0325
    pFix->bay3d_wgtlmt_l = CLIP(tmp, 0, 0x3ff);

    // BAY3D_BAY3D_SIG_X0
    for(i = 0; i < 16; i++)
    {
        //pFix->bay3d_sig_x[i] = bayernr_get_trans_V2(pSelect->bayernrv2_tnr_lumapoint[i]);
        tmp = pSelect->bayernrv2_tnr_lumapoint[i];
        pFix->bay3d_sig_x[i] = CLIP(tmp, 0, 0xffff);

        tmp = pSelect->bayernrv2_tnr_sigma[i];
        pFix->bay3d_sig_y[i] = CLIP(tmp, 0, 0xffff);
    }

    bayernr3D_fix_printf_V2(pFix);

    return ABAYERNR_RET_SUCCESS;
}

Abayernr_result_t bayernr2D_fix_printf_V2(RK_Bayernr_2D_Fix_V2_t * pFix)
{
    //FILE *fp = fopen("bayernr_regsiter.dat", "wb+");
    Abayernr_result_t res = ABAYERNR_RET_SUCCESS;

    if(pFix == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERNR_RET_NULL_POINTER;
    }

    LOGD_ANR("%s:(%d) ############# bayernr2D enter######################## \n", __FUNCTION__, __LINE__);
    //ISP_BAYNR_3A00_CTRL(0x0000)
    LOGD_ANR("(0x0000) gauss_en:0x%x log_bypass:0x%x en:0x%x \n",
             pFix->baynr_gauss_en,
             pFix->baynr_log_bypass,
             pFix->baynr_en);

    // ISP_BAYNR_3A00_DGAIN0-2 (0x0004 - 0x0008)
    for(int i = 0; i < 3; i++) {
        LOGD_ANR("(0x0004 - 0x0008) dgain[%d]:0x%x \n",
                 i, pFix->baynr_dgain[i]);
    }

    // ISP_BAYNR_3A00_PIXDIFF(0x000c)
    LOGD_ANR("(0x000c) pix_diff:0x%x \n",
             pFix->baynr_pix_diff);

    // ISP_BAYNR_3A00_THLD(0x0010)
    LOGD_ANR("(0x000d) diff_thld:0x%x softthld:0x%x \n",
             pFix->baynr_diff_thld,
             pFix->baynr_softthld);

    // ISP_BAYNR_3A00_W1_STRENG(0x0014)
    LOGD_ANR("(0x0014) bltflt_streng:0x%x reg_w1:0x%x \n",
             pFix->bltflt_streng,
             pFix->baynr_reg_w1);

    // ISP_BAYNR_3A00_SIGMAX0-15(0x0018 - 0x0034)
    for(int i = 0; i < 16; i++) {
        LOGD_ANR("(0x0018 - 0x0034) sig_x[%d]:0x%x \n",
                 i, pFix->sigma_x[i]);
    }

    // ISP_BAYNR_3A00_SIGMAY0-15(0x0038 - 0x0054)
    for(int i = 0; i < 16; i++) {
        LOGD_ANR("(0x0038 - 0x0054) sig_y[%d]:0x%x \n",
                 i, pFix->sigma_y[i]);
    }

    // ISP_BAYNR_3A00_WRIT_D(0x0058)
    LOGD_ANR("(0x0058) weit_d[0]:0x%x weit_d[1]:0x%x weit_d[2]:0x%x\n",
             pFix->weit_d[0],
             pFix->weit_d[1],
             pFix->weit_d[2]);

    LOGD_ANR("%s:(%d) ############# bayernr2D exit ######################## \n", __FUNCTION__, __LINE__);
    return res;
}


Abayernr_result_t bayernr3D_fix_printf_V2(RK_Bayernr_3D_Fix_V2_t * pFix)
{
    //FILE *fp = fopen("bayernr_regsiter.dat", "wb+");
    Abayernr_result_t res = ABAYERNR_RET_SUCCESS;

    if(pFix == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERNR_RET_NULL_POINTER;
    }

    LOGD_ANR("%s:(%d) ############# bayernr3D enter######################## \n", __FUNCTION__, __LINE__);

    // BAY3D_BAY3D_CTRL (0x0080)
    LOGD_ANR("(0x0080) exp_sel:0x%x bypass_en:0x%x pk_en:0x%x en_i:0x%x\n",
             pFix->bay3d_exp_sel,
             pFix->bay3d_bypass_en,
             pFix->bay3d_pk_en,
             pFix->bay3d_en_i);

    // BAY3D_BAY3D_KALRATIO (0x0084)
    LOGD_ANR("(0x0084) softwgt:0x%x sigratio:0x%x \n",
             pFix->bay3d_softwgt,
             pFix->bay3d_sigratio);

    // BAY3D_BAY3D_GLBPK2 (0x0088)
    LOGD_ANR("(0x0088) glbpk2:0x%x \n",
             pFix->bay3d_glbpk2);

    // BAY3D_BAY3D_KALSTR (0x008c)
    LOGD_ANR("(0x008c) exp_str:0x%x str:0x%x \n",
             pFix->bay3d_exp_str,
             pFix->bay3d_str);

    // BAY3D_BAY3D_WGTLMT (0x0090)
    LOGD_ANR("(0x0090) wgtlmt_h:0x%x wgtlmt_l:0x%x \n",
             pFix->bay3d_wgtlmt_h,
             pFix->bay3d_wgtlmt_l);

    // BAY3D_BAY3D_SIG_X0-16  (0x0094 - 0x00b0)
    for(int i = 0; i < 16; i++) {
        LOGD_ANR("(0x0094 - 0x00b0) sig_x[%d]:0x%x \n",
                 i, pFix->bay3d_sig_x[i]);
    }

    // BAY3D_BAY3D_SIG_Y0-16 (0x00b4 - 0x00d0)
    for(int i = 0; i < 16; i++) {
        LOGD_ANR("(0x00b4 - 0x00d0) sig_y[%d]:0x%x \n",
                 i, pFix->bay3d_sig_y[i]);
    }


    LOGD_ANR("%s:(%d) ############# bayernr3D exit ######################## \n", __FUNCTION__, __LINE__);
    return res;
}


Abayernr_result_t bayernr2D_get_setting_by_name_json_V2(CalibDbV2_BayerNrV2_t* pCalibdb, char *name, int *calib_idx, int *tuning_idx)
{

    int i = 0;
    Abayernr_result_t res = ABAYERNR_RET_SUCCESS;

    if(pCalibdb == NULL || name == NULL || calib_idx == NULL || tuning_idx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERNR_RET_NULL_POINTER;
    }

    for(i = 0; i < pCalibdb->Bayernr2D.Setting_len; i++) {
        if(strncmp(name, pCalibdb->Bayernr2D.Setting[i].SNR_Mode, strlen(name)*sizeof(char)) == 0) {
            break;
        }
    }

    if(i < pCalibdb->Bayernr2D.Setting_len) {
        *tuning_idx = i;
    } else {
        *tuning_idx = 0;
    }

    for(i = 0; i < pCalibdb->CalibPara.Setting_len; i++) {
        if(strncmp(name, pCalibdb->CalibPara.Setting[i].SNR_Mode, strlen(name)*sizeof(char)) == 0) {
            break;
        }
    }

    if(i < pCalibdb->CalibPara.Setting_len) {
        *calib_idx = i;
    } else {
        *calib_idx = 0;
    }

    LOGD_ANR("%s:%d snr_name:%s  snr_idx:%d i:%d \n", __FUNCTION__, __LINE__, name, *calib_idx, i);
    return res;


}




Abayernr_result_t bayernr2D_init_params_json_V2(RK_Bayernr_2D_Params_V2_t *pParams, CalibDbV2_BayerNrV2_t* pCalibdb, int calib_idx, int tuning_idx)
{
    Abayernr_result_t res = ABAYERNR_RET_SUCCESS;
    CalibDbV2_BayerNrV2_C_ISO_t *pCalibIso = NULL;
    CalibDbV2_BayerNrV2_2d_ISO_t *pTuningISO = NULL;
    int i = 0;

    LOGI_ANR("%s:(%d) oyyf bayerner xml config start\n", __FUNCTION__, __LINE__);
    if(pParams == NULL || pCalibdb == NULL || calib_idx < 0 || tuning_idx < 0) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERNR_RET_NULL_POINTER;
    }

    for(int i = 0; i < pCalibdb->CalibPara.Setting[calib_idx].Calib_ISO_len && i < RK_BAYERNR_V2_MAX_ISO_NUM; i++) {
        pCalibIso = &pCalibdb->CalibPara.Setting[calib_idx].Calib_ISO[i];
        pParams->iso[i] = pCalibIso->iso;
        for(int k = 0; k < 16; k++) {
            pParams->bayernrv2_filter_lumapoint_r[k] = pCalibIso->lumapoint[k];
            pParams->bayernrv2_filter_sigma_r[i][k] = pCalibIso->sigma[k];
        }
    }

    for(int i = 0; i < pCalibdb->Bayernr2D.Setting[tuning_idx].Tuning_ISO_len && i < RK_BAYERNR_V2_MAX_ISO_NUM; i++) {
        pTuningISO = &pCalibdb->Bayernr2D.Setting[tuning_idx].Tuning_ISO[i];
        pParams->iso[i] = pTuningISO->iso;
        pParams->bayernrv2_filter_strength_r[i] = pTuningISO->filter_strength;
        pParams->bayernrv2_filter_edgesofts_r[i] = pTuningISO->edgesofts;
        pParams->bayernrv2_filter_out_wgt_r[i] = pTuningISO->weight;
        pParams->bayernrv2_filter_soft_threshold_ratio_r[i] = pTuningISO->ratio;
        pParams->bayernrv2_gauss_guide_r[i] = pTuningISO->gauss_guide;
    }

    LOGI_ANR("%s:(%d) oyyf bayerner xml config end!   \n", __FUNCTION__, __LINE__);

    return res;
}

Abayernr_result_t bayernr2D_config_setting_param_json_V2(RK_Bayernr_2D_Params_V2_t *pParams, CalibDbV2_BayerNrV2_t* pCalibdbV2, char* param_mode, char * snr_name)
{
    Abayernr_result_t res = ABAYERNR_RET_SUCCESS;
    int calib_idx = 0;
    int tuning_idx = 0;


    if(pParams == NULL || pCalibdbV2 == NULL
            || param_mode == NULL || snr_name == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERNR_RET_NULL_POINTER;
    }

    res = bayernr2D_get_setting_by_name_json_V2(pCalibdbV2, snr_name, &calib_idx, &tuning_idx);
    if(res != ABAYERNR_RET_SUCCESS) {
        LOGW_ANR("%s(%d): error!!!  can't find setting in iq files, use 0 instead\n", __FUNCTION__, __LINE__);
    }

    res = bayernr2D_init_params_json_V2(pParams, pCalibdbV2, calib_idx, tuning_idx);
    pParams->bayernrv2_2dnr_enable = pCalibdbV2->Bayernr2D.enable;

    return res;

}



Abayernr_result_t bayernr3D_get_setting_by_name_json_V2(CalibDbV2_BayerNrV2_t* pCalibdb, char *name, int *calib_idx, int *tuning_idx)
{
    int i = 0;
    Abayernr_result_t res = ABAYERNR_RET_SUCCESS;

    if(pCalibdb == NULL || name == NULL || calib_idx == NULL || tuning_idx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERNR_RET_NULL_POINTER;
    }

    for(i = 0; i < pCalibdb->Bayernr3D.Setting_len; i++) {
        if(strncmp(name, pCalibdb->Bayernr3D.Setting[i].SNR_Mode, strlen(name)*sizeof(char)) == 0) {
            break;
        }
    }

    if(i < pCalibdb->Bayernr3D.Setting_len) {
        *tuning_idx = i;
    } else {
        *tuning_idx = 0;
    }

    for(i = 0; i < pCalibdb->CalibPara.Setting_len; i++) {
        if(strncmp(name, pCalibdb->CalibPara.Setting[i].SNR_Mode, strlen(name)*sizeof(char)) == 0) {
            break;
        }
    }

    if(i < pCalibdb->CalibPara.Setting_len) {
        *calib_idx = i;
    } else {
        *calib_idx = 0;
    }

    LOGD_ANR("%s:%d snr_name:%s  snr_idx:%d i:%d \n", __FUNCTION__, __LINE__, name, *calib_idx, i);
    return res;


}


Abayernr_result_t bayernr3D_init_params_json_V2(RK_Bayernr_3D_Params_V2_t *pParams, CalibDbV2_BayerNrV2_t* pCalibdb, int calib_idx, int tuning_idx)
{
    Abayernr_result_t res = ABAYERNR_RET_SUCCESS;
    CalibDbV2_BayerNrV2_C_ISO_t *pCalibIso = NULL;
    CalibDbV2_BayerNrV2_3d_ISO_t *pTuningIso = NULL;

    LOGI_ANR("%s:(%d) oyyf bayerner xml config start\n", __FUNCTION__, __LINE__);
    if(pParams == NULL || pCalibdb == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERNR_RET_NULL_POINTER;
    }

    for(int i = 0; i < pCalibdb->CalibPara.Setting[calib_idx].Calib_ISO_len && i < RK_BAYERNR_V2_MAX_ISO_NUM; i++) {
        pCalibIso = &pCalibdb->CalibPara.Setting[calib_idx].Calib_ISO[i];
        pParams->iso[i] = pCalibIso->iso;
        for(int k = 0; k < 16; k++) {
            pParams->bayernrv2_lumapoint_r[k] = pCalibIso->lumapoint[k];
            pParams->bayernrv2_sigma_r[i][k] = pCalibIso->sigma[k];
        }
    }

    for(int i = 0; i < pCalibdb->Bayernr3D.Setting[tuning_idx].Tuning_ISO_len && i < RK_BAYERNR_V2_MAX_ISO_NUM; i++) {
        pTuningIso = &pCalibdb->Bayernr3D.Setting[tuning_idx].Tuning_ISO[i];

        pParams->iso[i] = pTuningIso->iso;
        pParams->bayernrv2_tnr_filter_strength_r[i] = pTuningIso->filter_strength;
        pParams->bayernrv2_tnr_lo_clipwgt_r[i] = pTuningIso->lo_clipwgt;
        pParams->bayernrv2_tnr_hi_clipwgt_r[i] = pTuningIso->hi_clipwgt;
        pParams->bayernrv2_tnr_softwgt_r[i] = pTuningIso->softwgt;

    }

    LOGI_ANR("%s:(%d) oyyf bayerner xml config end!   \n", __FUNCTION__, __LINE__);

    return res;
}


Abayernr_result_t bayernr3D_config_setting_param_json_V2(RK_Bayernr_3D_Params_V2_t *pParams, CalibDbV2_BayerNrV2_t* pCalibdbV2, char* param_mode, char * snr_name)
{
    Abayernr_result_t res = ABAYERNR_RET_SUCCESS;
    int calib_idx = 0;
    int tuning_idx = 0;

    if(pParams == NULL || pCalibdbV2 == NULL
            || param_mode == NULL || snr_name == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERNR_RET_NULL_POINTER;
    }

    res = bayernr3D_get_setting_by_name_json_V2(pCalibdbV2, snr_name, &calib_idx, &tuning_idx);
    if(res != ABAYERNR_RET_SUCCESS) {
        LOGW_ANR("%s(%d): error!!!  can't find setting in iq files, use 0 instead\n", __FUNCTION__, __LINE__);

    }

    res = bayernr3D_init_params_json_V2(pParams, pCalibdbV2, calib_idx, tuning_idx);
    pParams->bayernrv2_tnr_enable = pCalibdbV2->Bayernr3D.enable;
    return res;

}

RKAIQ_END_DECLARE

