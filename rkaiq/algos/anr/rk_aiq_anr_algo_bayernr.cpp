

#include "rk_aiq_anr_algo_bayernr.h"

RKAIQ_BEGIN_DECLARE

ANRresult_t bayernr_get_mode_cell_idx_by_name(CalibDb_BayerNr_2_t *pCalibdb, char *name, int *mode_idx)
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
        LOGE_ANR("%s(%d): bayerne mode cell is zero\n", __FUNCTION__, __LINE__);
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

ANRresult_t bayernr_get_setting_idx_by_name(CalibDb_BayerNr_2_t *pCalibdb, char *name, int mode_idx, int *setting_idx)
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

    LOGD_ANR("%s:%d snr_name:%s  snr_idx:%d i:%d \n", __FUNCTION__, __LINE__, name, *setting_idx, i);
    return res;

}

ANRresult_t bayernr_config_setting_param(RKAnr_Bayernr_Params_t *pParams, CalibDb_BayerNr_2_t *pCalibdb, char* param_mode, char * snr_name)
{
    ANRresult_t res = ANR_RET_SUCCESS;
    int mode_idx = 0;
    int setting_idx = 0;

    res = bayernr_get_mode_cell_idx_by_name(pCalibdb, param_mode, &mode_idx);
    if(res != ANR_RET_SUCCESS) {
        LOGW_ANR("%s(%d): error!!!  can't find mode name in iq files, use 0 instead\n", __FUNCTION__, __LINE__);
    }

    res = bayernr_get_setting_idx_by_name(pCalibdb, snr_name, mode_idx, &setting_idx);
    if(res != ANR_RET_SUCCESS) {
        LOGW_ANR("%s(%d): error!!!  can't find setting in iq files, use 0 instead\n", __FUNCTION__, __LINE__);
    }

    res = init_bayernr_params(pParams, pCalibdb, mode_idx, setting_idx);

    return res;

}

ANRresult_t init_bayernr_params(RKAnr_Bayernr_Params_t *pParams, CalibDb_BayerNr_2_t *pCalibdb, int mode_idx, int setting_idx)
{
    ANRresult_t res = ANR_RET_SUCCESS;
    int i = 0;
    int j = 0;

    LOGI_ANR("%s:(%d) oyyf bayerner xml config start\n", __FUNCTION__, __LINE__);
    if(pParams == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ANR_RET_NULL_POINTER;
    }

    if(pCalibdb == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ANR_RET_NULL_POINTER;
    }

    //RKAnr_Bayernr_Params_t *pParams = &pANRCtx->stAuto.stBayernrParams;
    //CalibDb_BayerNr_t *pCalibdb = &pANRConfig->stBayernrCalib;

    CalibDb_BayerNR_Params_t *pSetting = &pCalibdb->mode_cell[mode_idx].setting[setting_idx];

    for(i = 0; i < MAX_ISO_STEP; i++) {
#ifndef RK_SIMULATOR_HW
        pParams->iso[i] = pSetting->iso[i];
#endif
        pParams->a[i] = pSetting->iso[i];
        pParams->b[i] = pSetting->iso[i];
        pParams->filtpar[i] = pSetting->filtPara[i];
        LOGI_ANR("a[%d]:%f filtpar[%d]:%f\n",
                 i, pParams->a[i],
                 i, pParams->filtpar[i]);
    }

    pParams->halfpatch = 1;
    pParams->halfblock = 1;

    for(i = 0; i < 7; i++) {
        pParams->ctrPit[i] = 1.0;
    }

    for(i = 0; i < 8; i++) {
        pParams->luLevel[i] = pSetting->luLevelVal[i];
        LOGI_ANR("luLevel[%d]:%f \n",
                 i, pParams->luLevel[i]);
    }

    for(i = 0; i < MAX_ISO_STEP; i++) {
        for(j = 0; j < 8; j++) {
            pParams->luRatio[i][j] = pSetting->luRatio[j][i];
        }
    }

    for(i = 0; i < MAX_ISO_STEP; i++) {
        for(j = 0; j < 4; j++) {
            pParams->w[i][j] = pSetting->fixW[j][i];
        }
    }

    pParams->peaknoisesigma = pSetting->lamda;
    pParams->sw_rawnr_gauss_en = pSetting->gauss_en;
    pParams->rgain_offs = pSetting->RGainOff;
    pParams->rgain_filp = pSetting->RGainFilp;
    pParams->bgain_offs = pSetting->BGainOff;
    pParams->bgain_filp = pSetting->BGainFilp;
    pParams->bayernr_edgesoftness = pSetting->edgeSoftness;
    pParams->bayernr_gauss_weight0 = 0;
    pParams->bayernr_gauss_weight1 = 0;

    memcpy(pParams->bayernr_ver_char,  pCalibdb->version, sizeof(pParams->bayernr_ver_char));

    LOGI_ANR("%s:(%d) oyyf bayerner xml config end!  ver:%s \n", __FUNCTION__, __LINE__, pParams->bayernr_ver_char);

    return res;
}



ANRresult_t bayernr_get_setting_idx_by_name_json(CalibDbV2_BayerNrV1_t *pCalibdb, char *name,  int *calib_idx,  int * tuning_idx)
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

    if(calib_idx == NULL) {
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


ANRresult_t init_bayernr_params_json(RKAnr_Bayernr_Params_t *pParams, CalibDbV2_BayerNrV1_t *pCalibdb, int calib_idx, int tuning_idx)
{
    ANRresult_t res = ANR_RET_SUCCESS;
    int i = 0;
    int j = 0;

    LOGI_ANR("%s:(%d) oyyf bayerner xml config start\n", __FUNCTION__, __LINE__);
    if(pParams == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ANR_RET_NULL_POINTER;
    }

    if(pCalibdb == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ANR_RET_NULL_POINTER;
    }

    CalibDbV2_BayerNrV1_C_Set_t *pCalibSetting =  &pCalibdb->CalibPara.Setting[calib_idx];
    CalibDbV2_BayerNrV1_T_Set_t *pTuningSetting = &pCalibdb->TuningPara.Setting[tuning_idx];

    CalibDbV2_BayerNrV1_C_ISO_t *pCalib_ISO = NULL;
    CalibDbV2_BayerNrV1_T_ISO_t *pTuning_ISO = NULL;


    for(i = 0; i < pTuningSetting->Tuning_ISO_len; i++) {
        pTuning_ISO = &pTuningSetting->Tuning_ISO[i];

#ifndef RK_SIMULATOR_HW
        pParams->iso[i] = pTuning_ISO->iso;
#endif
        pParams->a[i] = pTuning_ISO->iso;
        pParams->b[i] = pTuning_ISO->iso;
        pParams->filtpar[i] = pTuning_ISO->filtPara;
        LOGI_ANR("a[%d]:%f filtpar[%d]:%f\n",
                 i, pParams->a[i],
                 i, pParams->filtpar[i]);
    }

    pParams->halfpatch = 1;
    pParams->halfblock = 1;

    for(i = 0; i < 7; i++) {
        pParams->ctrPit[i] = 1.0;
    }

    for(i = 0; i < 8; i++) {
        pParams->luLevel[i] = pCalibSetting->Calib_ISO[0].luLevelVal[i];
        LOGI_ANR("luLevel[%d]:%f \n",
                 i, pParams->luLevel[i]);
    }

    for(i = 0; i < pCalibSetting->Calib_ISO_len; i++) {
        for(j = 0; j < 8; j++) {
            pParams->luRatio[i][j] = pCalibSetting->Calib_ISO[i].luRatio[j];
        }
    }

    for(i = 0; i < pTuningSetting->Tuning_ISO_len; i++) {
        pParams->w[i][0] = pTuningSetting->Tuning_ISO[i].fixW0;
        pParams->w[i][1] = pTuningSetting->Tuning_ISO[i].fixW1;
        pParams->w[i][2] = pTuningSetting->Tuning_ISO[i].fixW2;
        pParams->w[i][3] = pTuningSetting->Tuning_ISO[i].fixW3;
    }

    pParams->peaknoisesigma = pTuningSetting->Tuning_ISO[0].lamda;
    pParams->sw_rawnr_gauss_en = pTuningSetting->Tuning_ISO[0].gauss_en;
    pParams->rgain_offs = pTuningSetting->Tuning_ISO[0].RGainOff;
    pParams->rgain_filp = pTuningSetting->Tuning_ISO[0].RGainFilp;
    pParams->bgain_offs = pTuningSetting->Tuning_ISO[0].BGainOff;
    pParams->bgain_filp = pTuningSetting->Tuning_ISO[0].BGainFilp;

    pParams->bayernr_edgesoftness = 0;
    pParams->bayernr_gauss_weight0 = 0;
    pParams->bayernr_gauss_weight1 = 0;

    strncpy(pParams->bayernr_ver_char,  pCalibdb->Version, sizeof(pParams->bayernr_ver_char)-1);

    LOGI_ANR("%s:(%d) oyyf bayerner xml config end!  ver:%s \n", __FUNCTION__, __LINE__, pParams->bayernr_ver_char);

    bayernr_algo_param_printf(pParams);

    return res;
}

ANRresult_t bayernr_algo_param_printf(RKAnr_Bayernr_Params_t *pParams)
{
    int i, j;

    if(pParams == NULL) {
        LOGE_ANR("NULL pointer\n");
        return ANR_RET_NULL_POINTER;
    }

    for(i = 0; i < MAX_ISO_STEP; i++) {
#ifndef RK_SIMULATOR_HW
        LOGD_ANR("bayernr: iso:%f\n",
                 pParams->iso[i]);
#endif

        LOGD_ANR("a[%d]:%f filtpar[%d]:%f\n",
                 i, pParams->a[i],
                 i, pParams->filtpar[i]);
    }

    for(i = 0; i < 8; i++) {
        LOGD_ANR("luLevel[%d]:%f \n",
                 i, pParams->luLevel[i]);
    }

    for(i = 0; i < MAX_ISO_STEP; i++) {
        for(j = 0; j < 8; j++) {
            LOGD_ANR("luLevel[%d][%d]:%f \n",
                     i, j, pParams->luRatio[i][j]);
        }

        LOGD_ANR("fixw[%d]:%f %f %f %f \n",
                 i, pParams->w[i][0], pParams->w[i][1], pParams->w[i][2], pParams->w[i][3]);
    }

    LOGD_ANR(" lamda:%f gauss_en:%d\n",
             pParams->peaknoisesigma, pParams->sw_rawnr_gauss_en);

    return ANR_RET_SUCCESS;
}

ANRresult_t bayernr_config_setting_param_json(RKAnr_Bayernr_Params_t *pParams, CalibDbV2_BayerNrV1_t *pCalibdb, char* param_mode, char * snr_name)
{
    ANRresult_t res = ANR_RET_SUCCESS;
    int calib_idx = 0;
    int tuning_idx = 0;

    res = bayernr_get_setting_idx_by_name_json(pCalibdb, snr_name,  &calib_idx, &tuning_idx);
    if(res != ANR_RET_SUCCESS) {
        LOGW_ANR("%s(%d): error!!!  can't find setting in iq files, use 0 instead\n", __FUNCTION__, __LINE__);
    }

    res = init_bayernr_params_json(pParams, pCalibdb, calib_idx, tuning_idx);

    return res;

}

ANRresult_t selsec_hdr_parmas_by_ISO(RKAnr_Bayernr_Params_t *stBayerNrParams, RKAnr_Bayernr_Params_Select_t *stBayerNrParamsSelected, ANRExpInfo_t *pExpInfo)
{
    float frameiso[3];
    float frameEt[3];
    float fdgain[3];
    int i = 0;

    if(stBayerNrParams == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ANR_RET_NULL_POINTER;
    }

    if(stBayerNrParamsSelected == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ANR_RET_NULL_POINTER;
    }

    if(pExpInfo == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ANR_RET_NULL_POINTER;
    }

    int framenum = pExpInfo->hdr_mode + 1;

    frameiso[0] = pExpInfo->arAGain[0] * pExpInfo->arDGain[0];
    frameiso[1] = pExpInfo->arAGain[1] * pExpInfo->arDGain[1];
    frameiso[2] = pExpInfo->arAGain[2] * pExpInfo->arDGain[2];

    frameEt[0] = pExpInfo->arTime[0];
    frameEt[1] = pExpInfo->arTime[1];
    frameEt[2] = pExpInfo->arTime[2];

    for(int j = 0; j < framenum; j++)
    {
        ////降噪参数获取
        //确定iso等级
        //共有7个iso等级：50 100 200 400 800 1600 3200  6400 12800
        //       isogain: 1   2   4   8   16   32  64  128  256
        //      isolevel: 0   1   2   3   4    5   6   7    8
        int isoGainStd[MAX_ISO_STEP];
        int isoGain = int(frameiso[j]);
        int isoGainLow = 0; //向下一个isoGain,用做参数插值：y=float(isoGainHig-isoGain)/float(isoGainHig-isoGainLow)*y[isoLevelLow]
        //                                  +float(isoGain-isoGainLow)/float(isoGainHig-isoGainLow)*y[isoLevelHig];
        int isoGainHig = 0; //向上一个isoGain
        int isoGainCorrect = 1; //选择最近的一档iso的配置

        int isoLevelLow = 0;
        int isoLevelHig = 0;
        int isoLevelCorrect = 0;

#ifndef RK_SIMULATOR_HW
        for(int i = 0; i < MAX_ISO_STEP; i++) {
            isoGainStd[i] = stBayerNrParams->iso[i] / 50;
        }
#else
        for(int i = 0; i < MAX_ISO_STEP; i++) {
            isoGainStd[i] = 1 * (1 << i);
        }
#endif

        for (i = 0; i < MAX_ISO_STEP - 1; i++)
        {
            if (isoGain >= isoGainStd[i] && isoGain <= isoGainStd[i + 1])
            {
                isoGainLow = isoGainStd[i];
                isoGainHig = isoGainStd[i + 1];
                isoLevelLow = i;
                isoLevelHig = i + 1;
                isoGainCorrect = ((isoGain - isoGainStd[i]) <= (isoGainStd[i + 1] - isoGain)) ? isoGainStd[i] : isoGainStd[i + 1];
                isoLevelCorrect = ((isoGain - isoGainStd[i]) <= (isoGainStd[i + 1] - isoGain)) ? i : (i + 1);
            }
        }

        //VST变换参数, bilinear
        stBayerNrParamsSelected->a[j] = float(isoGainHig - isoGain) / float(isoGainHig - isoGainLow) * stBayerNrParams->a[isoLevelLow]
                                        + float(isoGain - isoGainLow) / float(isoGainHig - isoGainLow) * stBayerNrParams->a[isoLevelHig];

        stBayerNrParamsSelected->b[j] = float(isoGainHig - isoGain) / float(isoGainHig - isoGainLow) * stBayerNrParams->b[isoLevelLow]
                                        + float(isoGain - isoGainLow) / float(isoGainHig - isoGainLow) * stBayerNrParams->b[isoLevelHig];

        stBayerNrParamsSelected->b[j] = 0;
        stBayerNrParamsSelected->t0[j] = 0;

        stBayerNrParamsSelected->filtPar[j] = float(isoGainHig - isoGain) / float(isoGainHig - isoGainLow) * stBayerNrParams->filtpar[isoLevelLow]
                                              + float(isoGain - isoGainLow) / float(isoGainHig - isoGainLow) * stBayerNrParams->filtpar[isoLevelHig];

    }

    for (i = 0; i < framenum; i++) {
        frameiso[i] = frameiso[i] * 50;
        fdgain[i] = frameiso[i] * frameEt[i];
    }

    for (i = 0; i < framenum; i++) {
        fdgain[i] = fdgain[framenum - 1] / fdgain[i];
#if 0
        stBayerNrParamsSelected->sw_dgain[i] = fdgain[i];
#else
        stBayerNrParamsSelected->sw_dgain[i] = sqrt(fdgain[i]);
#endif
    }

    for (i = 0; i < framenum; i++)
    {
        float gainsqrt = sqrt(fdgain[i]);
#if 0
        float filtParDiscount = (float)0.1;
        float par = (stBayerNrParamsSelected->filtPar[i] * filtParDiscount);

        LOGD_ANR("gainsqrt:%f filtpar:%f, total:%f\n",
                 gainsqrt, stBayerNrParamsSelected->filtPar[i], par * gainsqrt);
        stBayerNrParamsSelected->filtPar[i] = par * gainsqrt;
#else
        stBayerNrParamsSelected->filtPar[i] = stBayerNrParamsSelected->filtPar[i] * gainsqrt;
#endif
    }

    if(framenum <= 1 ) {
        stBayerNrParamsSelected->gausskparsq = int((1 * 1) * float(1 << (FIXNLMCALC)));// * (1 << 7);
    } else {
        stBayerNrParamsSelected->gausskparsq = int((1 * 1) * float(1 << (FIXNLMCALC)));
    }
    stBayerNrParamsSelected->sigmaPar = 0 * (1 << FIXNLMCALC);
    stBayerNrParamsSelected->thld_diff = (int(LUTMAXM1_FIX * LUTPRECISION_FIX));
    stBayerNrParamsSelected->thld_chanelw = int(0.1 * float(1 << FIXNLMCALC));
    stBayerNrParamsSelected->pix_diff = FIXDIFMAX - 1;
    stBayerNrParamsSelected->log_bypass = 0;    //0 is none, 1 is G and RB all en,  2 only en G, 3 only RB;

    if(framenum <= 1 ) {
        stBayerNrParamsSelected->filtPar[1] = stBayerNrParamsSelected->filtPar[0];
        stBayerNrParamsSelected->filtPar[2] = stBayerNrParamsSelected->filtPar[0];
        stBayerNrParamsSelected->sw_dgain[1] = stBayerNrParamsSelected->sw_dgain[0];
        stBayerNrParamsSelected->sw_dgain[2] = stBayerNrParamsSelected->sw_dgain[0];
    }
    return ANR_RET_SUCCESS;
}

ANRresult_t select_bayernr_params_by_ISO(RKAnr_Bayernr_Params_t *stBayerNrParams, RKAnr_Bayernr_Params_Select_t *stBayerNrParamsSelected, ANRExpInfo_t *pExpInfo)
{
    ANRresult_t res = ANR_RET_SUCCESS;
    int iso = 50;

    if(stBayerNrParams == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ANR_RET_NULL_POINTER;
    }

    if(stBayerNrParamsSelected == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ANR_RET_NULL_POINTER;
    }

    if(pExpInfo == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ANR_RET_NULL_POINTER;
    }

    iso = pExpInfo->arIso[pExpInfo->hdr_mode];

    LOGD_ANR("%s:%d iso:%d \n", __FUNCTION__, __LINE__, iso);

    int isoGainStd[MAX_ISO_STEP];
    int isoGain = MAX(int(iso / 50), 1);
    int isoGainLow = 0;
    int isoGainHig = 0;
    int isoLevelLow = 0;
    int isoLevelHig = 0;
    int i;

#ifndef RK_SIMULATOR_HW
    for(int i = 0; i < MAX_ISO_STEP; i++) {
        isoGainStd[i] = stBayerNrParams->iso[i] / 50;
    }
#else
    for(int i = 0; i < MAX_ISO_STEP; i++) {
        isoGainStd[i] = 1 * (1 << i);
    }
#endif


    for (i = 0; i < MAX_ISO_STEP - 1; i++)
    {
        if (isoGain >= isoGainStd[i] && isoGain <= isoGainStd[i + 1])
        {
            isoGainLow = isoGainStd[i];
            isoGainHig = isoGainStd[i + 1];
            isoLevelLow = i;
            isoLevelHig = i + 1;
            break;
        }
    }

    if(i == MAX_ISO_STEP - 1) {
        if(isoGain < isoGainStd[0]) {
            isoGainLow = isoGainStd[0];
            isoGainHig = isoGainStd[1];
            isoLevelLow = 0;
            isoLevelHig = 1;
        }

        if(isoGain > isoGainStd[MAX_ISO_STEP - 1]) {
            isoGainLow = isoGainStd[MAX_ISO_STEP - 2];
            isoGainHig = isoGainStd[MAX_ISO_STEP - 1];
            isoLevelLow = MAX_ISO_STEP - 2;
            isoLevelHig = MAX_ISO_STEP - 1;
        }
    }

    LOGD_ANR("%s:%d iso:%d high:%d low:%d\n",
             __FUNCTION__, __LINE__,
             isoGain, isoGainHig, isoGainLow);

    //VST变换参数, bilinear
    stBayerNrParamsSelected->a[0] = float(isoGainHig - isoGain) / float(isoGainHig - isoGainLow) * stBayerNrParams->a[isoLevelLow]
                                    + float(isoGain - isoGainLow) / float(isoGainHig - isoGainLow) * stBayerNrParams->a[isoLevelHig];

    stBayerNrParamsSelected->b[0] = float(isoGainHig - isoGain) / float(isoGainHig - isoGainLow) * stBayerNrParams->b[isoLevelLow]
                                    + float(isoGain - isoGainLow) / float(isoGainHig - isoGainLow) * stBayerNrParams->b[isoLevelHig];

    stBayerNrParamsSelected->b[0] = 0;
    stBayerNrParamsSelected->t0[0] = 0;

    //领域halfBlock、搜索halfBlock、降噪系数filtPar,其中halfPatch和halfBlock都是对单通道而言
    stBayerNrParamsSelected->halfPatch = stBayerNrParams->halfpatch;
    stBayerNrParamsSelected->halfBlock = stBayerNrParams->halfblock;

    stBayerNrParamsSelected->filtPar[0] = float(isoGainHig - isoGain) / float(isoGainHig - isoGainLow) * stBayerNrParams->filtpar[isoLevelLow]
                                          + float(isoGain - isoGainLow) / float(isoGainHig - isoGainLow) * stBayerNrParams->filtpar[isoLevelHig];

#ifdef BAYER_NR_DEBUG
    LOGD_ANR("Patch=%d*%d\n", stBayerNrParamsSelected->halfPatch * 2 + 1, stBayerNrParamsSelected->halfPatch * 2 + 1);
    LOGD_ANR("Block=%d*%d\n", stBayerNrParamsSelected->halfBlock * 2 + 1, stBayerNrParamsSelected->halfBlock * 2 + 1);
    LOGD_ANR("filPar=%f\n", stBayerNrParamsSelected->filtPar);
#endif

    for (i = 0; i < 7; i++)
    {
        stBayerNrParamsSelected->ctrPit[i] = stBayerNrParams->ctrPit[i];
    }

    for (i = 0; i < 8; i++)
    {
        stBayerNrParamsSelected->luLevel[i] = stBayerNrParams->luLevel[i];
        stBayerNrParamsSelected->luRatio[i] = float(isoGainHig - isoGain) / float(isoGainHig - isoGainLow) * stBayerNrParams->luRatio[isoLevelLow][i]
                                              + float(isoGain - isoGainLow) / float(isoGainHig - isoGainLow) * stBayerNrParams->luRatio[isoLevelHig][i];
    }

    stBayerNrParamsSelected->peaknoisesigma = stBayerNrParams->peaknoisesigma;
    stBayerNrParamsSelected->sw_rawnr_gauss_en = stBayerNrParams->sw_rawnr_gauss_en;
    for (i = 0; i < 4; i++)
    {
        stBayerNrParamsSelected->w[i] = float(isoGainHig - isoGain) / float(isoGainHig - isoGainLow) * stBayerNrParams->w[isoLevelLow][i]
                                        + float(isoGain - isoGainLow) / float(isoGainHig - isoGainLow) * stBayerNrParams->w[isoLevelHig][i];
    }
    stBayerNrParamsSelected->bayernr_edgesoftness = stBayerNrParams->bayernr_edgesoftness;

    stBayerNrParamsSelected->sw_bayernr_edge_filter_en = stBayerNrParams->sw_bayernr_edge_filter_en;
    for (i = 0; i < 8; i++)
    {
        stBayerNrParamsSelected->sw_bayernr_edge_filter_lumapoint[i] = stBayerNrParams->sw_bayernr_edge_filter_lumapoint[i];
        stBayerNrParamsSelected->sw_bayernr_edge_filter_wgt[i] = float(isoGainHig - isoGain) / float(isoGainHig - isoGainLow) * stBayerNrParams->sw_bayernr_edge_filter_wgt[isoLevelLow][i]
                + float(isoGain - isoGainLow) / float(isoGainHig - isoGainLow) * stBayerNrParams->sw_bayernr_edge_filter_wgt[isoLevelHig][i];
    }
    stBayerNrParamsSelected->sw_bayernr_filter_strength = float(isoGainHig - isoGain) / float(isoGainHig - isoGainLow) * stBayerNrParams->sw_bayernr_filter_strength[isoLevelLow]
            + float(isoGain - isoGainLow) / float(isoGainHig - isoGainLow) * stBayerNrParams->sw_bayernr_filter_strength[isoLevelHig];
    for (i = 0; i < 16; i++)
    {
        stBayerNrParamsSelected->sw_bayernr_filter_lumapoint[i] = stBayerNrParams->sw_bayernr_filter_lumapoint[i];
        stBayerNrParamsSelected->sw_bayernr_filter_sigma[i] = float(isoGainHig - isoGain) / float(isoGainHig - isoGainLow) * stBayerNrParams->sw_bayernr_filter_sigma[isoLevelLow][i]
                + float(isoGain - isoGainLow) / float(isoGainHig - isoGainLow) * stBayerNrParams->sw_bayernr_filter_sigma[isoLevelHig][i];
    }
    stBayerNrParamsSelected->sw_bayernr_filter_edgesofts = float(isoGainHig - isoGain) / float(isoGainHig - isoGainLow) * stBayerNrParams->sw_bayernr_filter_edgesofts[isoLevelLow]
            + float(isoGain - isoGainLow) / float(isoGainHig - isoGainLow) * stBayerNrParams->sw_bayernr_filter_edgesofts[isoLevelHig];
    stBayerNrParamsSelected->sw_bayernr_filter_soft_threshold_ratio = float(isoGainHig - isoGain) / float(isoGainHig - isoGainLow) * stBayerNrParams->sw_bayernr_filter_soft_threshold_ratio[isoLevelLow]
            + float(isoGain - isoGainLow) / float(isoGainHig - isoGainLow) * stBayerNrParams->sw_bayernr_filter_soft_threshold_ratio[isoLevelHig];
    stBayerNrParamsSelected->sw_bayernr_filter_out_wgt = float(isoGainHig - isoGain) / float(isoGainHig - isoGainLow) * stBayerNrParams->sw_bayernr_filter_out_wgt[isoLevelLow]
            + float(isoGain - isoGainLow) / float(isoGainHig - isoGainLow) * stBayerNrParams->sw_bayernr_filter_out_wgt[isoLevelHig];

    //oyyf: add some fix params select
    memcpy(stBayerNrParamsSelected->bayernr_ver_char, stBayerNrParams->bayernr_ver_char, sizeof(stBayerNrParams->bayernr_ver_char));
    stBayerNrParamsSelected->rgain_offs = stBayerNrParams->rgain_offs;
    stBayerNrParamsSelected->rgain_filp = stBayerNrParams->rgain_filp;
    stBayerNrParamsSelected->bgain_offs = stBayerNrParams->bgain_offs;
    stBayerNrParamsSelected->bgain_filp = stBayerNrParams->bgain_filp;

    stBayerNrParamsSelected->bayernr_gauss_weight0 = stBayerNrParams->bayernr_gauss_weight0;
    stBayerNrParamsSelected->bayernr_gauss_weight1 = stBayerNrParams->bayernr_gauss_weight1;

    stBayerNrParamsSelected->gausskparsq = int((1.15 * 1.15) * float(1 << (FIXNLMCALC)));
    stBayerNrParamsSelected->sigmaPar = 0 * (1 << FIXNLMCALC);
    stBayerNrParamsSelected->thld_diff = (int(LUTMAXM1_FIX * LUTPRECISION_FIX));
    stBayerNrParamsSelected->thld_chanelw = int(0.1 * float(1 << FIXNLMCALC));
    stBayerNrParamsSelected->pix_diff = FIXDIFMAX - 1;
    stBayerNrParamsSelected->log_bypass = 0;    //0 is none, 1 is G and RB all en,  2 only en G, 3 only RB;

    //oyyf: if hdr open
    selsec_hdr_parmas_by_ISO(stBayerNrParams, stBayerNrParamsSelected, pExpInfo);

    return res;
}


unsigned short bayernr_get_trans(int tmpfix)
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

ANRresult_t bayernr_fix_tranfer(RKAnr_Bayernr_Params_Select_t* rawnr, RKAnr_Bayernr_Fix_t *pRawnrCfg, float fStrength)
{
    ANRresult_t res = ANR_RET_SUCCESS;
    int rawbit = 12;//rawBit;
    float tmp;

    LOGI_ANR("%s:(%d) enter \n", __FUNCTION__, __LINE__);

    if(rawnr == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ANR_RET_NULL_POINTER;
    }

    if(pRawnrCfg == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ANR_RET_NULL_POINTER;
    }

    if(fStrength <= 0.0f) {
        fStrength = 0.000001;
    }

    LOGD_ANR("%s(%d): strength:%f \n", __FUNCTION__, __LINE__, fStrength);

    //(0x0004)
    pRawnrCfg->gauss_en = rawnr->sw_rawnr_gauss_en;
    pRawnrCfg->log_bypass = rawnr->log_bypass;

    //(0x0008 - 0x00010)
    pRawnrCfg->filtpar0 = (unsigned short)(rawnr->filtPar[0] * fStrength * (1 << FIXNLMCALC));
    pRawnrCfg->filtpar1 = (unsigned short)(rawnr->filtPar[1] * fStrength * (1 << FIXNLMCALC));
    pRawnrCfg->filtpar2 = (unsigned short)(rawnr->filtPar[2] * fStrength * (1 << FIXNLMCALC));
    if(pRawnrCfg->filtpar0 > 0x3fff) {
        pRawnrCfg->filtpar0 =  0x3fff;
    }
    if(pRawnrCfg->filtpar1 > 0x3fff) {
        pRawnrCfg->filtpar1 =  0x3fff;
    }
    if(pRawnrCfg->filtpar2 > 0x3fff) {
        pRawnrCfg->filtpar2 =  0x3fff;
    }

    //(0x0014 - 0x0001c)
    pRawnrCfg->dgain0 = (unsigned int)(rawnr->sw_dgain[0] * (1 << FIXNLMCALC));
    pRawnrCfg->dgain1 = (unsigned int)(rawnr->sw_dgain[1] * (1 << FIXNLMCALC));
    pRawnrCfg->dgain2 = (unsigned int)(rawnr->sw_dgain[2] * (1 << FIXNLMCALC));
    if(pRawnrCfg->dgain0 > 0x3ffff) {
        pRawnrCfg->dgain0 =  0x3ffff;
    }
    if(pRawnrCfg->dgain1 > 0x3ffff) {
        pRawnrCfg->dgain1 =  0x3ffff;
    }
    if(pRawnrCfg->dgain2 > 0x3ffff) {
        pRawnrCfg->dgain2 =  0x3ffff;
    }


    //(0x0020 - 0x0002c)
    for(int i = 0; i < 8; i++) {
        pRawnrCfg->luration[i] = (unsigned short)(rawnr->luRatio[i] * (1 << FIXNLMCALC));
    }

    //(0x0030 - 0x0003c)
    for(int i = 0; i < 8; i++) {
        tmp = rawnr->luLevel[i] * ( 1 << (rawbit - 8) );
        pRawnrCfg->lulevel[i] = bayernr_get_trans((int)tmp);
    }
    tmp = (float)((1 << rawbit) - 1);
    pRawnrCfg->lulevel[8 - 1] = bayernr_get_trans((int)tmp);

    //(0x0040)
    pRawnrCfg->gauss = rawnr->gausskparsq;

    //(0x0044)
    pRawnrCfg->sigma = rawnr->sigmaPar;


    //(0x0048)
    pRawnrCfg->pix_diff = rawnr->pix_diff;


    //(0x004c)
    pRawnrCfg->thld_diff = rawnr->thld_diff;


    //(0x0050)
    pRawnrCfg->gas_weig_scl1 = rawnr->bayernr_gauss_weight0 * (1 << 8);
    pRawnrCfg->gas_weig_scl2 = rawnr->bayernr_gauss_weight1 * (1 << 8);
    pRawnrCfg->thld_chanelw = rawnr->thld_chanelw;



    //(0x0054)
    pRawnrCfg->lamda = rawnr->peaknoisesigma;


    //(0x0058 - 0x0005c)
    tmp = (rawnr->w[0] / fStrength * (1 << FIXNLMCALC));
    if(tmp > 0x3ff) {
        tmp = 0x3ff;
    }
    pRawnrCfg->fixw0 = (unsigned short)tmp;
    tmp = (rawnr->w[1] / fStrength * (1 << FIXNLMCALC));
    if(tmp > 0x3ff) {
        tmp = 0x3ff;
    }
    pRawnrCfg->fixw1 = (unsigned short)tmp;;
    tmp = (rawnr->w[2] / fStrength * (1 << FIXNLMCALC));
    if(tmp > 0x3ff) {
        tmp = 0x3ff;
    }
    pRawnrCfg->fixw2 = (unsigned short)tmp;
    tmp = (rawnr->w[3] / fStrength * (1 << FIXNLMCALC));
    if(tmp > 0x3ff) {
        tmp = 0x3ff;
    }
    pRawnrCfg->fixw3 = (unsigned short)tmp;


    //(0x0060 - 0x00068)
    pRawnrCfg->wlamda0 = (pRawnrCfg->fixw0 * pRawnrCfg->lamda) >> FIXNLMCALC;
    pRawnrCfg->wlamda1 = (pRawnrCfg->fixw1 * pRawnrCfg->lamda) >> FIXNLMCALC;
    pRawnrCfg->wlamda2 = (pRawnrCfg->fixw2 * pRawnrCfg->lamda) >> FIXNLMCALC;


    //(0x006c)
    pRawnrCfg->rgain_filp = rawnr->rgain_filp;
    pRawnrCfg->bgain_filp = rawnr->bgain_filp;

#if BAYERNR_FIX_VALUE_PRINTF
    bayernr_fix_printf(pRawnrCfg);
#endif

    LOGI_ANR("%s:(%d) exit \n", __FUNCTION__, __LINE__);

    return res;

}

ANRresult_t bayernr_fix_printf(RKAnr_Bayernr_Fix_t * pRawnrCfg)
{
    //FILE *fp = fopen("bayernr_regsiter.dat", "wb+");
    ANRresult_t res = ANR_RET_SUCCESS;

    if(pRawnrCfg == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ANR_RET_NULL_POINTER;
    }

    LOGD_ANR("%s:(%d) ############# rawnr enter######################## \n", __FUNCTION__, __LINE__);

    //(0x0004)
    LOGD_ANR("gauss_en:%d log_bypass:%d \n",
             pRawnrCfg->gauss_en,
             pRawnrCfg->log_bypass);

    //(0x0008 - 0x00010)
    LOGD_ANR("filtpar0-2:%d %d %d \n",
             pRawnrCfg->filtpar0,
             pRawnrCfg->filtpar1,
             pRawnrCfg->filtpar2);

    //(0x0014 - 0x0001c)
    LOGD_ANR("bayernr (0x0014 - 0x0001c)dgain0-2:%d %d %d \n",
             pRawnrCfg->dgain0,
             pRawnrCfg->dgain1,
             pRawnrCfg->dgain2);

    //(0x0020 - 0x0002c)
    for(int i = 0; i < 8; i++) {
        LOGD_ANR("luration[%d]:%d \n", i, pRawnrCfg->luration[i]);
    }

    //(0x0030 - 0x0003c)
    for(int i = 0; i < 8; i++) {
        LOGD_ANR("lulevel[%d]:%d \n", i, pRawnrCfg->lulevel[i]);
    }

    //(0x0040)
    LOGD_ANR("gauss:%d \n", pRawnrCfg->gauss);

    //(0x0044)
    LOGD_ANR("sigma:%d \n", pRawnrCfg->sigma);

    //(0x0048)
    LOGD_ANR("pix_diff:%d \n", pRawnrCfg->pix_diff);

    //(0x004c)
    LOGD_ANR("thld_diff:%d \n", pRawnrCfg->thld_diff);

    //(0x0050)
    LOGD_ANR("gas_weig_scl1:%d gas_weig_scl2:%d thld_chanelw:%d \n",
             pRawnrCfg->gas_weig_scl1,
             pRawnrCfg->gas_weig_scl2,
             pRawnrCfg->thld_chanelw);

    //(0x0054)
    LOGD_ANR("lamda:%d \n", pRawnrCfg->lamda);

    //(0x0058 - 0x0005c)
    LOGD_ANR("fixw0-3:%d %d %d %d\n",
             pRawnrCfg->fixw0,
             pRawnrCfg->fixw1,
             pRawnrCfg->fixw2,
             pRawnrCfg->fixw3);

    //(0x0060 - 0x00068)
    LOGD_ANR("wlamda0-2:%d %d %d \n",
             pRawnrCfg->wlamda0,
             pRawnrCfg->wlamda1,
             pRawnrCfg->wlamda2);

    //(0x006c)
    LOGD_ANR("rgain_filp:%d bgain_filp:%d \n",
             pRawnrCfg->rgain_filp,
             pRawnrCfg->bgain_filp);

    LOGD_ANR("%s:(%d) ############# rawnr exit ######################## \n", __FUNCTION__, __LINE__);
    LOGD_ANR("%s:(%d) exit \n", __FUNCTION__, __LINE__);

    return res;
}

ANRresult_t bayernr_calibdbV2_assign(CalibDbV2_BayerNrV1_t *pDst, CalibDbV2_BayerNrV1_t *pSrc)
{
    ANRresult_t res = ANR_RET_SUCCESS;
    CalibDbV2_BayerNrV1_Calib_t *pSrcCalibParaV2 = NULL;
    CalibDbV2_BayerNrV1_Tuning_t *pSrcTuningParaV2 = NULL;
    CalibDbV2_BayerNrV1_Calib_t *pDstCalibParaV2 = NULL;
    CalibDbV2_BayerNrV1_Tuning_t *pDstTuningParaV2 = NULL;
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

    bayernr_calibdbV2_free(pDst);

    pSrcCalibParaV2 = &pSrc->CalibPara;
    pSrcTuningParaV2 = &pSrc->TuningPara;
    pDstCalibParaV2 = &pDst->CalibPara;
    pDstTuningParaV2 = &pDst->TuningPara;

    //assign the value
    pDst->Version = strdup(pSrc->Version);
    pDstTuningParaV2->enable = pSrcTuningParaV2->enable;


    //malloc iso size
    setting_len = pSrcCalibParaV2->Setting_len;
    pDstCalibParaV2->Setting = (CalibDbV2_BayerNrV1_C_Set_t *)malloc(setting_len * sizeof(CalibDbV2_BayerNrV1_C_Set_t));
    memset(pDstCalibParaV2->Setting,  0x00, setting_len * sizeof(CalibDbV2_BayerNrV1_C_Set_t));
    pDstCalibParaV2->Setting_len = setting_len;

    for(int i = 0; i < setting_len; i++) {
        iso_len = pSrcCalibParaV2->Setting[i].Calib_ISO_len;
        pDstCalibParaV2->Setting[i].Calib_ISO =  (CalibDbV2_BayerNrV1_C_ISO_t *)malloc(iso_len * sizeof(CalibDbV2_BayerNrV1_C_ISO_t));
        memset(pDstCalibParaV2->Setting[i].Calib_ISO, 0x00, iso_len * sizeof(CalibDbV2_BayerNrV1_C_ISO_t));
        pDstCalibParaV2->Setting[i].Calib_ISO_len = iso_len;
    }

    for(int i = 0; i < setting_len; i++) {
        iso_len = pSrcCalibParaV2->Setting[i].Calib_ISO_len;
        pDstCalibParaV2->Setting[i].SNR_Mode = strdup(pSrcTuningParaV2->Setting[i].SNR_Mode);
        pDstCalibParaV2->Setting[i].Sensor_Mode = strdup(pSrcTuningParaV2->Setting[i].Sensor_Mode);

        for(int j = 0; j < iso_len; j++) {
            pDstCalibParaV2->Setting[i].Calib_ISO[j] = pSrcCalibParaV2->Setting[i].Calib_ISO[j];
        }
    }

    setting_len = pSrcTuningParaV2->Setting_len;
    pDstTuningParaV2->Setting = (CalibDbV2_BayerNrV1_T_Set_t *)malloc(setting_len * sizeof(CalibDbV2_BayerNrV1_T_Set_t));
    memset(pDstTuningParaV2->Setting, 0x00, setting_len * sizeof(CalibDbV2_BayerNrV1_T_Set_t));
    pDstTuningParaV2->Setting_len = setting_len;

    for(int i = 0; i < setting_len; i++) {
        iso_len = pSrcTuningParaV2->Setting[i].Tuning_ISO_len;
        pDstTuningParaV2->Setting[i].Tuning_ISO = (CalibDbV2_BayerNrV1_T_ISO_t *)malloc(iso_len * sizeof(CalibDbV2_BayerNrV1_T_ISO_t));
        memset(pDstTuningParaV2->Setting[i].Tuning_ISO, 0x00, iso_len * sizeof(CalibDbV2_BayerNrV1_T_ISO_t));
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

    return res;
}



void bayernr_calibdbV2_free(CalibDbV2_BayerNrV1_t *pCalibdbV2)
{
    if(pCalibdbV2 != NULL) {
        if(pCalibdbV2->CalibPara.Setting != NULL) {
            for(int i = 0; i < pCalibdbV2->CalibPara.Setting_len; i++) {
                if(pCalibdbV2->CalibPara.Setting[i].Calib_ISO != NULL) {
                    free(pCalibdbV2->CalibPara.Setting[i].Calib_ISO );
                }
                if(pCalibdbV2->CalibPara.Setting[i].Sensor_Mode != NULL) {
                    free(pCalibdbV2->CalibPara.Setting[i].Sensor_Mode);
                }
                if(pCalibdbV2->CalibPara.Setting[i].SNR_Mode != NULL) {
                    free(pCalibdbV2->CalibPara.Setting[i].SNR_Mode);
                }
            }
            free(pCalibdbV2->CalibPara.Setting);
        }

        if(pCalibdbV2->TuningPara.Setting != NULL) {
            for(int i = 0; i < pCalibdbV2->TuningPara.Setting_len; i++) {
                if(pCalibdbV2->TuningPara.Setting[i].Tuning_ISO != NULL) {
                    free(pCalibdbV2->TuningPara.Setting[i].Tuning_ISO );
                }
                if(pCalibdbV2->TuningPara.Setting[i].Sensor_Mode != NULL) {
                    free(pCalibdbV2->TuningPara.Setting[i].Sensor_Mode);
                }
                if(pCalibdbV2->TuningPara.Setting[i].SNR_Mode != NULL) {
                    free(pCalibdbV2->TuningPara.Setting[i].SNR_Mode);
                }
            }
            free(pCalibdbV2->TuningPara.Setting);
        }

        if(pCalibdbV2->Version != NULL) {
            free(pCalibdbV2->Version);
        }

    }

}


RKAIQ_END_DECLARE

