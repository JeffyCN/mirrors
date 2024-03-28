#include "rk_aiq_anr_algo_uvnr.h"

RKAIQ_BEGIN_DECLARE

ANRresult_t uvnr_get_mode_cell_idx_by_name(CalibDb_UVNR_2_t *pCalibdb, char *name, int *mode_idx)
{
    int i = 0;
    ANRresult_t res = ANR_RET_SUCCESS;

    if(pCalibdb == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ANR_RET_NULL_POINTER;
    }

    if(name == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ANR_RET_NULL_POINTER;
    }

    if(mode_idx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ANR_RET_NULL_POINTER;
    }

    if(pCalibdb->mode_num < 1) {
        LOGE_ANR("%s(%d): uvnr mode cell num is zero\n", __FUNCTION__, __LINE__);
        return ANR_RET_NULL_POINTER;
    }

    for(i = 0; i < pCalibdb->mode_num; i++) {
        if(strncmp(name, pCalibdb->mode_cell[i].name, sizeof(pCalibdb->mode_cell[i].name)) == 0) {
            break;
        }
    }

    if(i < pCalibdb->mode_num) {
        *mode_idx = i;
        res = ANR_RET_SUCCESS;
    } else {
        *mode_idx = 0;
        res = ANR_RET_FAILURE;
    }

    LOGD_ANR("%s:%d mode_name:%s  mode_idx:%d i:%d \n", __FUNCTION__, __LINE__, name, *mode_idx, i);
    return res;

}

ANRresult_t uvnr_get_setting_idx_by_name(CalibDb_UVNR_2_t *pCalibdb, char *name, int mode_idx, int *setting_idx)
{
    int i = 0;
    ANRresult_t res = ANR_RET_SUCCESS;

    if(pCalibdb == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ANR_RET_NULL_POINTER;
    }

    if(name == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ANR_RET_NULL_POINTER;
    }

    if(setting_idx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ANR_RET_NULL_POINTER;
    }

    for(i = 0; i < CALIBDB_NR_SHARP_SETTING_LEVEL; i++) {
        if(strncmp(name, pCalibdb->mode_cell[mode_idx].setting[i].snr_mode, sizeof(pCalibdb->mode_cell[mode_idx].setting[i].snr_mode)) == 0) {
            break;
        }
    }

    if(i < CALIBDB_NR_SHARP_SETTING_LEVEL) {
        *setting_idx = i;
        res = ANR_RET_SUCCESS;
    } else {
        *setting_idx = 0;
        res = ANR_RET_FAILURE;
    }

    LOGD_ANR("%s:%d snr_name:%s  snr_idx:%d i:%d \n", __FUNCTION__, __LINE__, name, * setting_idx, i);
    return res;

}

ANRresult_t uvnr_config_setting_param(RKAnr_Uvnr_Params_t *pParams, CalibDb_UVNR_2_t *pCalibdb, char* param_mode, char * snr_name)
{
    ANRresult_t res = ANR_RET_SUCCESS;
    int mode_idx = 0;
    int setting_idx = 0;

    if(pParams == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ANR_RET_NULL_POINTER;
    }

    if(pCalibdb == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ANR_RET_NULL_POINTER;
    }

    res = uvnr_get_mode_cell_idx_by_name(pCalibdb, param_mode, &mode_idx);
    if(res != ANR_RET_SUCCESS) {
        LOGW_ANR("%s(%d): error!!!  can't find mode cell in iq files, use 0 instead\n", __FUNCTION__, __LINE__);
    }

    res = uvnr_get_setting_idx_by_name(pCalibdb, snr_name, mode_idx, &setting_idx);
    if(res != ANR_RET_SUCCESS) {
        LOGW_ANR("%s(%d): error!!!  can't find setting in iq files, use 0 instead\n", __FUNCTION__, __LINE__);
    }

    res = init_uvnr_params(pParams, pCalibdb, mode_idx, setting_idx);

    return res;

}

ANRresult_t init_uvnr_params(RKAnr_Uvnr_Params_t *pParams, CalibDb_UVNR_2_t *pCalibdb, int mode_idx, int setting_idx)
{
    ANRresult_t res = ANR_RET_SUCCESS;
    int i = 0;

    if(pParams == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ANR_RET_NULL_POINTER;
    }

    if(pCalibdb == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ANR_RET_NULL_POINTER;
    }

    CalibDb_UVNR_Params_t *pSetting = &pCalibdb->mode_cell[mode_idx].setting[setting_idx];
    for(i = 0; i < MAX_ISO_STEP; i++) {
#ifndef RK_SIMULATOR_HW
        pParams->iso[i] = pSetting->ISO[i];
#endif
        pParams->ratio[i] = pSetting->step0_uvgrad_ratio[i];
        pParams->offset[i] = pSetting->step0_uvgrad_offset[i];

        pParams->wStep1[i] = pSetting->step1_downSample_w[i];
        pParams->hStep1[i] = pSetting->step1_downSample_h[i];
        pParams->meanSize1[i] = pSetting->step1_downSample_meansize[i];

        pParams->medSize1[i] = pSetting->step1_median_size[i];
        pParams->medRatio1[i] = pSetting->step1_median_ratio[i];
        pParams->isMedIIR1[i] = pSetting->step1_median_IIR[i];

        pParams->bfSize1[i] = pSetting->step1_bf_size[i];
        pParams->sigmaR1[i] = pSetting->step1_bf_sigmaR[i];
        pParams->sigmaD1[i] = pSetting->step1_bf_sigmaD[i];
        pParams->uvgain1[i] = pSetting->step1_bf_uvgain[i];
        pParams->bfRatio1[i] = pSetting->step1_bf_ratio[i];
        pParams->isRowIIR1[i] = pSetting->step1_bf_isRowIIR[i];
        pParams->isYcopy1[i] = pSetting->step1_bf_isYcopy[i];

        pParams->wStep2[i] = pSetting->step2_downSample_w[i];
        pParams->hStep2[i] = pSetting->step2_downSample_h[i];
        pParams->meanSize2[i] = pSetting->step2_downSample_meansize[i];

        pParams->medSize2[i] = pSetting->step2_median_size[i];
        pParams->medRatio2[i] = pSetting->step2_median_ratio[i];
        pParams->isMedIIR2[i] = pSetting->step2_median_IIR[i];

        pParams->bfSize3[i] = pSetting->step2_bf_size[i];
        pParams->sigmaR2[i] = pSetting->step2_bf_sigmaR[i];
        pParams->sigmaD2[i] = pSetting->step2_bf_sigmaD[i];
        pParams->uvgain2[i] = pSetting->step2_bf_uvgain[i];
        pParams->bfRatio2[i] = pSetting->step2_bf_ratio[i];
        pParams->isRowIIR2[i] = pSetting->step2_bf_isRowIIR[i];
        pParams->isYcopy2[i] = pSetting->step2_bf_isYcopy[i];

        pParams->bfSize3[i] = pSetting->step3_bf_size[i];
        pParams->sigmaR3[i] = pSetting->step3_bf_sigmaR[i];
        pParams->sigmaD3[i] = pSetting->step3_bf_sigmaD[i];
        pParams->uvgain3[i] = pSetting->step3_bf_uvgain[i];
        pParams->bfRatio3[i] = pSetting->step3_bf_ratio[i];
        pParams->isRowIIR3[i] = pSetting->step3_bf_isRowIIR[i];
        pParams->isYcopy3[i] = pSetting->step3_bf_isYcopy[i];

    }

    for(i = 0; i < 4; i++) {
        pParams->nonMed1[i] = pSetting->step1_nonMed1[i];
        pParams->nonBf1[i] = pSetting->step1_nonBf1[i];
        pParams->block2_ext[i] = pSetting->step2_nonExt_block[i];
        pParams->nonMed2[i] = pSetting->step2_nonMed[i];
        pParams->nonBf2[i] = pSetting->step2_nonBf[i];
        pParams->nonBf3[i] = pSetting->step3_nonBf3[i];
    }

    for(i = 0; i < 3; i++) {
        pParams->kernel_3x3_table[i] = pSetting->kernel_3x3[i];
    }

    for(i = 0; i < 5; i++) {
        pParams->kernel_5x5_talbe[i] = pSetting->kernel_5x5[i];
    }

    for(i = 0; i < 8; i++) {
        pParams->kernel_9x9_table[i] = pSetting->kernel_9x9[i];
    }

    pParams->kernel_9x9_num = pSetting->kernel_9x9_num;

    for(i = 0; i < 9; i++) {
        pParams->sigmaAdj_x[i] = pSetting->sigma_adj_luma[i];
        pParams->sigamAdj_y[i] = pSetting->sigma_adj_ratio[i];

        pParams->threAdj_x[i] = pSetting->threshold_adj_luma[i];
        pParams->threAjd_y[i] = pSetting->threshold_adj_thre[i];
    }

    return ANR_RET_SUCCESS;

}

ANRresult_t uvnr_get_setting_idx_by_name_json(CalibDbV2_UVNR_t *pCalibdb, char *name, int *tuning_idx)
{
    int i = 0;
    ANRresult_t res = ANR_RET_SUCCESS;

    if(pCalibdb == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ANR_RET_NULL_POINTER;
    }

    if(name == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ANR_RET_NULL_POINTER;
    }

    if(tuning_idx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ANR_RET_NULL_POINTER;
    }

    for(i = 0; i < pCalibdb->TuningPara.Setting_len; i++) {
        if(strncmp(name, pCalibdb->TuningPara.Setting[i].SNR_Mode, strlen(name)*sizeof(char)) == 0) {
            break;
        }
    }

    if(i < pCalibdb->TuningPara.Setting_len) {
        *tuning_idx = i;
        res = ANR_RET_SUCCESS;
    } else {
        *tuning_idx = 0;
        res = ANR_RET_FAILURE;
    }

    LOGD_ANR("%s:%d snr_name:%s  snr_idx:%d i:%d \n", __FUNCTION__, __LINE__, name, *tuning_idx, i);
    return res;

}


ANRresult_t init_uvnr_params_json(RKAnr_Uvnr_Params_t *pParams, CalibDbV2_UVNR_t *pCalibdb,  int setting_idx)
{
    ANRresult_t res = ANR_RET_SUCCESS;
    int i = 0;
    float luma[9] = {0.0000, 32.0000, 64.0000, 96.0000, 128.0000, 160.0000, 192.0000, 224.0000, 256.0000 };

    if(pParams == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ANR_RET_NULL_POINTER;
    }

    if(pCalibdb == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ANR_RET_NULL_POINTER;
    }

    CalibDbV2_UVNR_T_Set_t *pSetting = &pCalibdb->TuningPara.Setting[setting_idx];
    CalibDbV2_UVNR_T_ISO_t *pTuning_ISO = NULL;
    for(i = 0; i < pSetting->Tuning_ISO_len; i++) {
        pTuning_ISO = &pSetting->Tuning_ISO[i];
#ifndef RK_SIMULATOR_HW
        pParams->iso[i] = pTuning_ISO->iso;
#endif
        pParams->ratio[i] = pTuning_ISO->step0_uvgrad_ratio;
        pParams->offset[i] = pTuning_ISO->step0_uvgrad_offset;

        pParams->wStep1[i] = 4;
        pParams->hStep1[i] = 4;
        pParams->meanSize1[i] = 4;

        pParams->medSize1[i] = 3;
        pParams->medRatio1[i] = pTuning_ISO->step1_median_ratio;
        pParams->isMedIIR1[i] = 0;

        pParams->bfSize1[i] = 5;
        pParams->sigmaR1[i] = pTuning_ISO->step1_bf_sigmaR;
        pParams->sigmaD1[i] = 16;
        pParams->uvgain1[i] = pTuning_ISO->step1_bf_uvgain;
        pParams->bfRatio1[i] = pTuning_ISO->step1_bf_ratio;
        pParams->isRowIIR1[i] = 0;
        pParams->isYcopy1[i] = 1;

        pParams->wStep2[i] = 32;
        pParams->hStep2[i] = 32;
        pParams->meanSize2[i] = 8;

        pParams->medSize2[i] = 3;
        pParams->medRatio2[i] = pTuning_ISO->step2_median_ratio;
        pParams->isMedIIR2[i] = 0;

        pParams->bfSize3[i] = 5;
        pParams->sigmaR2[i] = pTuning_ISO->step2_bf_sigmaR;
        pParams->sigmaD2[i] = 128;
        pParams->uvgain2[i] = pTuning_ISO->step2_bf_uvgain;
        pParams->bfRatio2[i] = pTuning_ISO->step2_bf_ratio;
        pParams->isRowIIR2[i] = 0;
        pParams->isYcopy2[i] = 1;

        pParams->bfSize3[i] = 3;
        pParams->sigmaR3[i] = pTuning_ISO->step3_bf_sigmaR;
        pParams->sigmaD3[i] = 8;
        pParams->uvgain3[i] = pTuning_ISO->step3_bf_uvgain;
        pParams->bfRatio3[i] = pTuning_ISO->step3_bf_ratio;
        pParams->isRowIIR3[i] = 0;
        pParams->isYcopy3[i] = 1;

    }

    for(i = 0; i < 4; i++) {
        pParams->nonMed1[i] = 3;
        pParams->nonBf1[i] = 31;
        pParams->block2_ext[i] = 7;
        pParams->nonMed2[i] = 1;
        pParams->nonBf2[i] = 3;
        pParams->nonBf3[i] = 32;
    }

    for(i = 0; i < 3; i++) {
        pParams->kernel_3x3_table[i] = pCalibdb->TuningPara.Kernel_Coeff.kernel_3x3[i];
    }

    for(i = 0; i < 5; i++) {
        pParams->kernel_5x5_talbe[i] = pCalibdb->TuningPara.Kernel_Coeff.kernel_5x5[i];
    }

    for(i = 0; i < 8; i++) {
        pParams->kernel_9x9_table[i] = pCalibdb->TuningPara.Kernel_Coeff.kernel_9x9[i];
    }

    pParams->kernel_9x9_num = pCalibdb->TuningPara.Kernel_Coeff.kernel_9x9_num;

    for(i = 0; i < 9; i++) {
        pParams->sigmaAdj_x[i] = luma[i];
        pParams->sigamAdj_y[i] = 1;

        pParams->threAdj_x[i] = luma[i];
        pParams->threAjd_y[i] = 10;
    }

    uvnr_algo_param_printf(pParams);
    return ANR_RET_SUCCESS;

}

ANRresult_t uvnr_algo_param_printf(RKAnr_Uvnr_Params_t *pParams)
{
    int i;

    if(pParams != NULL) {
        for(i = 0; i < MAX_ISO_STEP; i++) {
#ifndef RK_SIMULATOR_HW
            LOGD_ANR("uvnr iso:%f\n",
                     pParams->iso[i]);
#endif
            LOGD_ANR(" ratio:%f, offset:%f medRatio1\n",
                     pParams->ratio[i],
                     pParams->offset[i]);

            LOGD_ANR("step1_median_ratio:%f  step1_bf_sigmaR:%f step1_bf_uvgain:%f step1_bf_ratio:%f\n",
                     pParams->medRatio1[i],
                     pParams->sigmaR1[i],
                     pParams->uvgain1[i],
                     pParams->bfRatio1[i]);

            LOGD_ANR("step2_median_ratio:%f  step2_bf_sigmaR:%f step2_bf_uvgain:%f step2_bf_ratio:%f\n",
                     pParams->medRatio2[i],
                     pParams->sigmaR2[i],
                     pParams->uvgain2[i],
                     pParams->bfRatio2[i]);

            LOGD_ANR(" step3_bf_sigmaR:%f step3_bf_uvgain:%f step3_bf_ratio:%f\n",
                     pParams->sigmaR3[i],
                     pParams->uvgain3[i],
                     pParams->bfRatio3[i]);

            LOGD_ANR(" kernel3x3: %f %f %f\n",
                     pParams->kernel_3x3_table[0],
                     pParams->kernel_3x3_table[1],
                     pParams->kernel_3x3_table[2]);

            LOGD_ANR(" kernel5x5: %f %f %f %f %f\n",
                     pParams->kernel_5x5_talbe[0],
                     pParams->kernel_5x5_talbe[1],
                     pParams->kernel_5x5_talbe[2],
                     pParams->kernel_5x5_talbe[3],
                     pParams->kernel_5x5_talbe[4]);

            LOGD_ANR(" kernel9x9: %f %f %f %f %f %f %f %f\n",
                     pParams->kernel_9x9_table[0],
                     pParams->kernel_9x9_table[1],
                     pParams->kernel_9x9_table[2],
                     pParams->kernel_9x9_table[3],
                     pParams->kernel_9x9_table[4],
                     pParams->kernel_9x9_table[5],
                     pParams->kernel_9x9_table[6],
                     pParams->kernel_9x9_table[7]);

            LOGD_ANR("kernel_num:%d\n\n", pParams->kernel_9x9_num);


        }
    }

    return ANR_RET_SUCCESS;
}


ANRresult_t uvnr_config_setting_param_json(RKAnr_Uvnr_Params_t *pParams, CalibDbV2_UVNR_t *pCalibdb, char* param_mode, char * snr_name)
{
    ANRresult_t res = ANR_RET_SUCCESS;
    int tuning_idx = 0;

    if(pParams == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ANR_RET_NULL_POINTER;
    }

    if(pCalibdb == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ANR_RET_NULL_POINTER;
    }

    res = uvnr_get_setting_idx_by_name_json(pCalibdb, snr_name,  &tuning_idx);
    if(res != ANR_RET_SUCCESS) {
        LOGW_ANR("%s(%d): error!!!  can't find setting in iq files, use 0 instead\n", __FUNCTION__, __LINE__);
    }

    res = init_uvnr_params_json(pParams, pCalibdb, tuning_idx);

    return res;

}

float interpISO(int ISO_low, int ISO_high, float value_low, float value_high, int ISO, float value)
{
    if (ISO <= ISO_low)
    {
        value = value_low;
    }
    else if (ISO >= ISO_high)
    {
        value = value_high;
    }
    else
    {
        value = float(ISO - ISO_low) / float(ISO_high - ISO_low) * (value_high - value_low) + value_low;
    }
    return value;
}


ANRresult_t select_uvnr_params_by_ISO(RKAnr_Uvnr_Params_t *stRKUVNrParams, RKAnr_Uvnr_Params_Select_t *stRKUVNrParamsSelected, ANRExpInfo_t *pExpInfo)
{
    ANRresult_t res = ANR_RET_SUCCESS;
    int iso = 50;
    if(stRKUVNrParams == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ANR_RET_NULL_POINTER;
    }

    if(stRKUVNrParamsSelected == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ANR_RET_NULL_POINTER;
    }

    if(pExpInfo == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ANR_RET_NULL_POINTER;
    }

    if(pExpInfo->mfnr_mode_3to1) {
        iso = pExpInfo->preIso[pExpInfo->hdr_mode];
    } else {
        iso = pExpInfo->arIso[pExpInfo->hdr_mode];
    }
    //ȷ��iso�ȼ�
    //rkuvnriso@50 100 200 400 800 1600 3200  6400 12800
    //      isogain: 1  2   4   8   16  32   64    128  256
    //     isoindex: 0  1   2   3   4   5    6     7    8

    int isoIndex = 0;
    int isoGainLow = 0;
    int isoGainHigh = 0;
    int isoIndexLow = 0;
    int isoIndexHigh = 0;
    int iso_div         = 50;
    int max_iso_step  = MAX_ISO_STEP;
    int i = 0;

#ifndef RK_SIMULATOR_HW
    for ( i = 0; i < max_iso_step - 1 ; i++)
    {
        if (iso >=  stRKUVNrParams->iso[i]  &&  iso <= stRKUVNrParams->iso[i + 1])
        {
            isoGainLow =  stRKUVNrParams->iso[i] ;
            isoGainHigh = stRKUVNrParams->iso[i + 1];
            isoIndexLow = i;
            isoIndexHigh = i + 1;
            isoIndex = isoIndexLow;
            break;
        }
    }

    if(i == max_iso_step - 1) {
        if(iso < stRKUVNrParams->iso[0] ) {
            isoGainLow =  stRKUVNrParams->iso[0];
            isoGainHigh = stRKUVNrParams->iso[1];
            isoIndexLow = 0;
            isoIndexHigh = 1;
            isoIndex = 0;
        }

        if(iso >  stRKUVNrParams->iso[max_iso_step - 1] ) {
            isoGainLow =  stRKUVNrParams->iso[max_iso_step - 2] ;
            isoGainHigh = stRKUVNrParams->iso[max_iso_step - 1];
            isoIndexLow = max_iso_step - 2;
            isoIndexHigh = max_iso_step - 1;
            isoIndex = max_iso_step - 1;
        }
    }
#else
    isoIndex = int(log(float(iso / iso_div)) / log(2.0f));

    for (i = max_iso_step - 1; i >= 0; i--)
    {
        if (iso < iso_div * (2 << i))
        {
            isoGainLow = iso_div * (2 << (i)) / 2;
            isoGainHigh = iso_div * (2 << i);
        }
    }

    isoGainLow      = MIN(isoGainLow, iso_div * (2 << max_iso_step));
    isoGainHigh     = MIN(isoGainHigh, iso_div * (2 << max_iso_step));

    isoIndexHigh    = (int)(log((float)isoGainHigh / iso_div) / log((float)2));
    isoIndexLow     = (int)(log((float)isoGainLow / iso_div) / log((float)2));

    isoIndexLow     = MIN(MAX(isoIndexLow, 0), max_iso_step - 1);
    isoIndexHigh    = MIN(MAX(isoIndexHigh, 0), max_iso_step - 1);
#endif

    LOGD_ANR("%s:%d iso:%d high:%d low:%d \n",
             __FUNCTION__, __LINE__,
             iso, isoGainHigh, isoGainLow);

    //ȡ��
    memcpy(stRKUVNrParamsSelected->select_iso, stRKUVNrParams->rkuvnrISO, sizeof(char) * 256);
    //step0:uvgainԤ����
    stRKUVNrParamsSelected->ratio = interpISO(isoGainLow, isoGainHigh, stRKUVNrParams->ratio[isoIndexLow],
                                    stRKUVNrParams->ratio[isoIndexHigh], iso, stRKUVNrParamsSelected->ratio);
    stRKUVNrParamsSelected->offset = interpISO(isoGainLow, isoGainHigh, stRKUVNrParams->offset[isoIndexLow],
                                     stRKUVNrParams->offset[isoIndexHigh], iso, stRKUVNrParamsSelected->offset);
    //step1-�²���1
    //��ֵ1
    stRKUVNrParamsSelected->wStep1 = stRKUVNrParams->wStep1[isoIndex];
    stRKUVNrParamsSelected->hStep1 = stRKUVNrParams->hStep1[isoIndex];
    stRKUVNrParamsSelected->meanSize1 = stRKUVNrParams->meanSize1[isoIndex];
    //��ֵ1
    memcpy(stRKUVNrParamsSelected->nonMed1, stRKUVNrParams->nonMed1, sizeof(int) * 4);
    stRKUVNrParamsSelected->medSize1 = stRKUVNrParams->medSize1[isoIndex];
    stRKUVNrParamsSelected->medRatio1 = interpISO(isoGainLow, isoGainHigh, stRKUVNrParams->medRatio1[isoIndexLow],
                                        stRKUVNrParams->medRatio1[isoIndexHigh], iso, stRKUVNrParamsSelected->medRatio1);
    stRKUVNrParamsSelected->isMedIIR1 = stRKUVNrParams->isMedIIR1[isoIndex];
    //˫��1
    memcpy(stRKUVNrParamsSelected->nonBf1, stRKUVNrParams->nonBf1, sizeof(int) * 4);
    stRKUVNrParamsSelected->bfSize1 = stRKUVNrParams->bfSize1[isoIndex];
    stRKUVNrParamsSelected->sigmaR1 = interpISO(isoGainLow, isoGainHigh, stRKUVNrParams->sigmaR1[isoIndexLow],
                                      stRKUVNrParams->sigmaR1[isoIndexHigh], iso, stRKUVNrParamsSelected->sigmaR1);
    stRKUVNrParamsSelected->sigmaD1 = interpISO(isoGainLow, isoGainHigh, stRKUVNrParams->sigmaD1[isoIndexLow],
                                      stRKUVNrParams->sigmaD1[isoIndexHigh], iso, stRKUVNrParamsSelected->sigmaD1);
    stRKUVNrParamsSelected->uvgain1 = interpISO(isoGainLow, isoGainHigh, stRKUVNrParams->uvgain1[isoIndexLow],
                                      stRKUVNrParams->uvgain1[isoIndexHigh], iso, stRKUVNrParamsSelected->uvgain1);
    stRKUVNrParamsSelected->bfRatio1 = interpISO(isoGainLow, isoGainHigh, stRKUVNrParams->bfRatio1[isoIndexLow],
                                       stRKUVNrParams->bfRatio1[isoIndexHigh], iso, stRKUVNrParamsSelected->bfRatio1);
    stRKUVNrParamsSelected->isRowIIR1 = stRKUVNrParams->isRowIIR1[isoIndex];
    stRKUVNrParamsSelected->isYcopy1 = stRKUVNrParams->isYcopy1[isoIndex];

    //step2-�²���2
    memcpy(stRKUVNrParamsSelected->block2_ext, stRKUVNrParams->block2_ext, sizeof(int) * 4);
    //��ֵ2
    stRKUVNrParamsSelected->wStep2 = stRKUVNrParams->wStep2[isoIndex];
    stRKUVNrParamsSelected->hStep2 = stRKUVNrParams->hStep2[isoIndex];
    stRKUVNrParamsSelected->meanSize2 = stRKUVNrParams->meanSize2[isoIndex];
    //��ֵ2
    memcpy(stRKUVNrParamsSelected->nonMed2, stRKUVNrParams->nonMed2, sizeof(int) * 4);
    stRKUVNrParamsSelected->medSize2 = stRKUVNrParams->medSize2[isoIndex];
    stRKUVNrParamsSelected->medRatio2 = interpISO(isoGainLow, isoGainHigh, stRKUVNrParams->medRatio2[isoIndexLow],
                                        stRKUVNrParams->medRatio2[isoIndexHigh], iso, stRKUVNrParamsSelected->medRatio2);
    stRKUVNrParamsSelected->isMedIIR2 = stRKUVNrParams->isMedIIR2[isoIndex];
    //˫��2
    memcpy(stRKUVNrParamsSelected->nonBf2, stRKUVNrParams->nonBf2, sizeof(int) * 4);
    stRKUVNrParamsSelected->bfSize2 = stRKUVNrParams->bfSize2[isoIndex];
    stRKUVNrParamsSelected->sigmaR2 = interpISO(isoGainLow, isoGainHigh, stRKUVNrParams->sigmaR2[isoIndexLow],
                                      stRKUVNrParams->sigmaR2[isoIndexHigh], iso, stRKUVNrParamsSelected->sigmaR2);
    stRKUVNrParamsSelected->sigmaD2 = interpISO(isoGainLow, isoGainHigh, stRKUVNrParams->sigmaD2[isoIndexLow],
                                      stRKUVNrParams->sigmaD2[isoIndexHigh], iso, stRKUVNrParamsSelected->sigmaD2);
    stRKUVNrParamsSelected->uvgain2 = interpISO(isoGainLow, isoGainHigh, stRKUVNrParams->uvgain2[isoIndexLow],
                                      stRKUVNrParams->uvgain2[isoIndexHigh], iso, stRKUVNrParamsSelected->uvgain2);
    stRKUVNrParamsSelected->bfRatio2 = interpISO(isoGainLow, isoGainHigh, stRKUVNrParams->bfRatio2[isoIndexLow],
                                       stRKUVNrParams->bfRatio2[isoIndexHigh], iso, stRKUVNrParamsSelected->bfRatio2);
    stRKUVNrParamsSelected->isRowIIR2 = stRKUVNrParams->isRowIIR2[isoIndex];
    stRKUVNrParamsSelected->isYcopy2 = stRKUVNrParams->isYcopy2[isoIndex];

    //step3
    memcpy(stRKUVNrParamsSelected->nonBf3, stRKUVNrParams->nonBf3, sizeof(int) * 4);
    //˫��3
    stRKUVNrParamsSelected->bfSize3 = stRKUVNrParams->bfSize3[isoIndex];
    stRKUVNrParamsSelected->sigmaR3 = interpISO(isoGainLow, isoGainHigh, stRKUVNrParams->sigmaR3[isoIndexLow],
                                      stRKUVNrParams->sigmaR3[isoIndexHigh], iso, stRKUVNrParamsSelected->sigmaR3);
    stRKUVNrParamsSelected->sigmaD3 = interpISO(isoGainLow, isoGainHigh, stRKUVNrParams->sigmaD3[isoIndexLow],
                                      stRKUVNrParams->sigmaD3[isoIndexHigh], iso, stRKUVNrParamsSelected->sigmaD3);
    stRKUVNrParamsSelected->uvgain3 = interpISO(isoGainLow, isoGainHigh, stRKUVNrParams->uvgain3[isoIndexLow],
                                      stRKUVNrParams->uvgain3[isoIndexHigh], iso, stRKUVNrParamsSelected->uvgain3);
    stRKUVNrParamsSelected->bfRatio3 = interpISO(isoGainLow, isoGainHigh, stRKUVNrParams->bfRatio3[isoIndexLow],
                                       stRKUVNrParams->bfRatio3[isoIndexHigh], iso, stRKUVNrParamsSelected->bfRatio3);
    stRKUVNrParamsSelected->isRowIIR3 = stRKUVNrParams->isRowIIR3[isoIndex];
    stRKUVNrParamsSelected->isYcopy3 = stRKUVNrParams->isYcopy3[isoIndex];

    //kernels
    memcpy(stRKUVNrParamsSelected->kernel_3x3_table, stRKUVNrParams->kernel_3x3_table, sizeof(float) * 3);
    memcpy(stRKUVNrParamsSelected->kernel_5x5_table, stRKUVNrParams->kernel_5x5_talbe, sizeof(float) * 5);
    memcpy(stRKUVNrParamsSelected->kernel_9x9_table, stRKUVNrParams->kernel_9x9_table, sizeof(float) * 8);
    stRKUVNrParamsSelected->kernel_9x9_num = stRKUVNrParams->kernel_9x9_num;

    //curves
    memcpy(stRKUVNrParamsSelected->sigmaAdj_x, stRKUVNrParams->sigmaAdj_x, sizeof(int) * 9);
    memcpy(stRKUVNrParamsSelected->sigmaAdj_y, stRKUVNrParams->sigamAdj_y, sizeof(float) * 9);
    memcpy(stRKUVNrParamsSelected->threAdj_x, stRKUVNrParams->threAdj_x, sizeof(int) * 9);
    memcpy(stRKUVNrParamsSelected->threAdj_y, stRKUVNrParams->threAjd_y, sizeof(int) * 9);

    return ANR_RET_SUCCESS;

}


ANRresult_t uvnr_fix_transfer(RKAnr_Uvnr_Params_Select_t *uvnr, RKAnr_Uvnr_Fix_t *pNrCfg, ANRExpInfo_t *pExpInfo, float gain_ratio, float fStrength)
{
    LOGI_ANR("%s:(%d) enter \n", __FUNCTION__, __LINE__);

    int i = 0;
    ANRresult_t res = ANR_RET_SUCCESS;

    if(uvnr == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ANR_RET_NULL_POINTER;
    }

    if(pNrCfg == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ANR_RET_NULL_POINTER;
    }

    if(pExpInfo == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ANR_RET_NULL_POINTER;
    }

    int iso = pExpInfo->arIso[pExpInfo->hdr_mode] * gain_ratio;

    int log2e                       = (int)(0.8493f * (1 << RKUVNR_log2e));
    log2e                           = log2e * (1 << RKUVNR_imgBit_set);

    //0x0080
    pNrCfg->uvnr_step1_en = 1;
    pNrCfg->uvnr_step2_en = 1;
    pNrCfg->nr_gain_en = 1;
    pNrCfg->uvnr_nobig_en = 0;
    pNrCfg->uvnr_big_en = 0;


    //0x0084
    pNrCfg->uvnr_gain_1sigma = (unsigned char)(uvnr->ratio * (1 << RKUVNR_ratio));

    //0x0088
    pNrCfg->uvnr_gain_offset = (unsigned char)(uvnr->offset * (1 << RKUVNR_offset));

    //0x008c
    pNrCfg->uvnr_gain_uvgain[0] = (unsigned char)(uvnr->uvgain1 * fStrength * (1 << RKUVNR_uvgain));
    if( pNrCfg->uvnr_gain_uvgain[0]  > 0x7f) {
        pNrCfg->uvnr_gain_uvgain[0] = 0x7f;
    }
    pNrCfg->uvnr_gain_uvgain[1] = (unsigned char)(uvnr->uvgain3 * fStrength * (1 << RKUVNR_uvgain));
    if( pNrCfg->uvnr_gain_uvgain[1]  > 0x7f) {
        pNrCfg->uvnr_gain_uvgain[1] = 0x7f;
    }
    pNrCfg->uvnr_gain_t2gen = (unsigned char)(uvnr->uvgain2 *  fStrength * (1 << RKUVNR_uvgain));
    if( pNrCfg->uvnr_gain_t2gen  > 0x7f) {
        pNrCfg->uvnr_gain_t2gen = 0x7f;
    }
    pNrCfg->uvnr_gain_iso = (int)(sqrt(50.0 / (float)(iso)) * (1 << RKUVNR_gainRatio));
    if(pNrCfg->uvnr_gain_iso > 0x80) {
        pNrCfg->uvnr_gain_iso = 0x80;
    }

    if(pNrCfg->uvnr_gain_iso < 0x8) {
        pNrCfg->uvnr_gain_iso = 0x8;
    }

    //0x0090
    pNrCfg->uvnr_t1gen_m3alpha = (uvnr->medRatio1 * (1 << RKUVNR_medRatio));

    //0x0094
    pNrCfg->uvnr_t1flt_mode = uvnr->kernel_9x9_num;

    //0x0098
    pNrCfg->uvnr_t1flt_msigma = (unsigned short)(log2e / uvnr->sigmaR1);

    pNrCfg->uvnr_t1flt_msigma = MIN(pNrCfg->uvnr_t1flt_msigma, 8191);


    //0x009c
    pNrCfg->uvnr_t1flt_wtp = (unsigned char)(uvnr->bfRatio1 * (1 << RKUVNR_bfRatio));

    //0x00a0-0x00a4
    for(i = 0; i < 8; i++) {
        pNrCfg->uvnr_t1flt_wtq[i] = (unsigned char)(uvnr->kernel_9x9_table[i] * (1 << RKUVNR_kernels));
    }

    //0x00a8
    pNrCfg->uvnr_t2gen_m3alpha = (unsigned char)(uvnr->medRatio2 * (1 << RKUVNR_medRatio));

    //0x00ac
    pNrCfg->uvnr_t2gen_msigma = (unsigned short)(log2e / uvnr->sigmaR2);
    pNrCfg->uvnr_t2gen_msigma = MIN(pNrCfg->uvnr_t2gen_msigma, 8191);


    //0x00b0
    pNrCfg->uvnr_t2gen_wtp = (unsigned char)(uvnr->kernel_5x5_table[0] * (1 << RKUVNR_kernels));

    //0x00b4
    for(i = 0; i < 4; i++) {
        pNrCfg->uvnr_t2gen_wtq[i] = (unsigned char)(uvnr->kernel_5x5_table[i + 1] * (1 << RKUVNR_kernels));
    }

    //0x00b8
    pNrCfg->uvnr_t2flt_msigma = (unsigned short)(log2e / uvnr->sigmaR3);
    pNrCfg->uvnr_t2flt_msigma = MIN(pNrCfg->uvnr_t2flt_msigma, 8191);

    //0x00bc
    pNrCfg->uvnr_t2flt_wtp = (unsigned char)(uvnr->bfRatio3 * (1 << RKUVNR_bfRatio));
    for(i = 0; i < 3; i++) {
        pNrCfg->uvnr_t2flt_wt[i] = (unsigned char)(uvnr->kernel_3x3_table[i] * (1 << RKUVNR_kernels));
    }

#if UVNR_FIX_VALUE_PRINTF
    uvnr_fix_Printf(pNrCfg);
#endif

    LOGI_ANR("%s:(%d) exit \n", __FUNCTION__, __LINE__);

    return ANR_RET_SUCCESS;
}


ANRresult_t uvnr_fix_Printf(RKAnr_Uvnr_Fix_t  * pNrCfg)
{
    int i = 0;
    LOGI_ANR("%s:(%d) enter \n", __FUNCTION__, __LINE__);

    ANRresult_t res = ANR_RET_SUCCESS;

    if(pNrCfg == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ANR_RET_NULL_POINTER;
    }

    //0x0080
    LOGD_ANR("(0x0080) uvnr_step1_en:%d uvnr_step2_en:%d nr_gain_en:%d uvnr_nobig_en:%d uvnr_big_en:%d\n",
             pNrCfg->uvnr_step1_en,
             pNrCfg->uvnr_step2_en,
             pNrCfg->nr_gain_en,
             pNrCfg->uvnr_nobig_en,
             pNrCfg->uvnr_big_en);

    //0x0084
    LOGD_ANR("(0x0084) uvnr_gain_1sigma:%d \n",
             pNrCfg->uvnr_gain_1sigma);

    //0x0088
    LOGD_ANR("(0x0088) uvnr_gain_offset:%d \n",
             pNrCfg->uvnr_gain_offset);

    //0x008c
    LOGD_ANR("uvnr: (0x008c) uvnr_gain_uvgain:%d uvnr_step2_en:%d uvnr_gain_t2gen:%d uvnr_gain_iso:%d\n",
             pNrCfg->uvnr_gain_uvgain[0],
             pNrCfg->uvnr_gain_uvgain[1],
             pNrCfg->uvnr_gain_t2gen,
             pNrCfg->uvnr_gain_iso);


    //0x0090
    LOGD_ANR("(0x0090) uvnr_t1gen_m3alpha:%d \n",
             pNrCfg->uvnr_t1gen_m3alpha);

    //0x0094
    LOGD_ANR("(0x0094) uvnr_t1flt_mode:%d \n",
             pNrCfg->uvnr_t1flt_mode);

    //0x0098
    LOGD_ANR("(0x0098) uvnr_t1flt_msigma:%d \n",
             pNrCfg->uvnr_t1flt_msigma);

    //0x009c
    LOGD_ANR("(0x009c) uvnr_t1flt_wtp:%d \n",
             pNrCfg->uvnr_t1flt_wtp);

    //0x00a0-0x00a4
    for(i = 0; i < 8; i++) {
        LOGD_ANR("(0x00a0-0x00a4) uvnr_t1flt_wtq[%d]:%d \n",
                 i, pNrCfg->uvnr_t1flt_wtq[i]);
    }

    //0x00a8
    LOGD_ANR("(0x00a8) uvnr_t2gen_m3alpha:%d \n",
             pNrCfg->uvnr_t2gen_m3alpha);

    //0x00ac
    LOGD_ANR("(0x00ac) uvnr_t2gen_msigma:%d \n",
             pNrCfg->uvnr_t2gen_msigma);

    //0x00b0
    LOGD_ANR("(0x00b0) uvnr_t2gen_wtp:%d \n",
             pNrCfg->uvnr_t2gen_wtp);

    //0x00b4
    for(i = 0; i < 4; i++) {
        LOGD_ANR("(0x00b4) uvnr_t2gen_wtq[%d]:%d \n",
                 i, pNrCfg->uvnr_t2gen_wtq[i]);
    }

    //0x00b8
    LOGD_ANR("(0x00b8) uvnr_t2flt_msigma:%d \n",
             pNrCfg->uvnr_t2flt_msigma);

    //0x00bc
    LOGD_ANR("(0x00bc) uvnr_t2flt_wtp:%d \n",
             pNrCfg->uvnr_t2flt_wtp);
    for(i = 0; i < 3; i++) {
        LOGD_ANR("(0x00bc) uvnr_t2flt_wt[%d]:%d \n",
                 i, pNrCfg->uvnr_t2flt_wt[i]);
    }


    LOGD_ANR("%s:(%d) exit \n", __FUNCTION__, __LINE__);

    return ANR_RET_SUCCESS;
}


ANRresult_t uvnr_calibdbV2_assign(CalibDbV2_UVNR_t *pDst, CalibDbV2_UVNR_t *pSrc)
{
    ANRresult_t res = ANR_RET_SUCCESS;
    CalibDbV2_UVNR_Tuning_t *pSrcTuningParaV2 = NULL;
    CalibDbV2_UVNR_Tuning_t *pDstTuningParaV2 = NULL;
    int setting_len = 0;
    int iso_len = 0;


    if(pDst == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ANR_RET_NULL_POINTER;
    }

    if(pSrc == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ANR_RET_NULL_POINTER;
    }

    uvnr_calibdbV2_free(pDst);

    pSrcTuningParaV2 = &pSrc->TuningPara;
    pDstTuningParaV2 = &pDst->TuningPara;

    //assign the value
    pDst->Version = strdup(pSrc->Version);
    pDstTuningParaV2->enable = pSrcTuningParaV2->enable;

    //malloc iso size
    setting_len = pSrcTuningParaV2->Setting_len;
    pDstTuningParaV2->Setting = (CalibDbV2_UVNR_T_Set_t *)malloc(setting_len * sizeof(CalibDbV2_UVNR_T_Set_t));
    memset(pDstTuningParaV2->Setting, 0x00, setting_len * sizeof(CalibDbV2_UVNR_T_Set_t));
    pDstTuningParaV2->Setting_len = setting_len;

    for(int i = 0; i < setting_len; i++) {
        iso_len = pSrcTuningParaV2->Setting[i].Tuning_ISO_len;
        pDstTuningParaV2->Setting[i].Tuning_ISO = (CalibDbV2_UVNR_T_ISO_t *)malloc(iso_len * sizeof(CalibDbV2_UVNR_T_ISO_t));
        memset(pDstTuningParaV2->Setting[i].Tuning_ISO, 0x00, iso_len * sizeof(CalibDbV2_UVNR_T_ISO_t));
        pDstTuningParaV2->Setting[i].Tuning_ISO_len = iso_len;
    }

    for(int i = 0; i < setting_len; i++) {
        iso_len = pSrcTuningParaV2->Setting[i].Tuning_ISO_len;
        pDstTuningParaV2->Setting[i].SNR_Mode = strdup(pSrcTuningParaV2->Setting[i].SNR_Mode);
        pDstTuningParaV2->Setting[i].Sensor_Mode = strdup(pSrcTuningParaV2->Setting[i].Sensor_Mode);

        for(int j = 0; j < iso_len; j++) {
            pDstTuningParaV2->Setting[i].Tuning_ISO[j] = pSrcTuningParaV2->Setting[i].Tuning_ISO[j];
        }
    }

    pDstTuningParaV2->Kernel_Coeff = pSrcTuningParaV2->Kernel_Coeff;

    return res;

}



void uvnr_calibdbV2_free(CalibDbV2_UVNR_t *pCalibdbV2)
{
    if(pCalibdbV2) {
        if(pCalibdbV2->Version) {
            free(pCalibdbV2->Version);
        }

        if(pCalibdbV2->TuningPara.Setting) {
            for(int i = 0; i < pCalibdbV2->TuningPara.Setting_len; i++) {
                if(pCalibdbV2->TuningPara.Setting[i].SNR_Mode) {
                    free(pCalibdbV2->TuningPara.Setting[i].SNR_Mode);
                }
                if(pCalibdbV2->TuningPara.Setting[i].Sensor_Mode) {
                    free(pCalibdbV2->TuningPara.Setting[i].Sensor_Mode);
                }
                if(pCalibdbV2->TuningPara.Setting[i].Tuning_ISO) {
                    free(pCalibdbV2->TuningPara.Setting[i].Tuning_ISO);
                }
            }
            free(pCalibdbV2->TuningPara.Setting);
        }

    }
}


RKAIQ_END_DECLARE




