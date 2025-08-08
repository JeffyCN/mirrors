
#include "rk_aiq_aynr_algo_v1.h"
#include "rk_aiq_aynr_algo_itf_v1.h"

RKAIQ_BEGIN_DECLARE

Aynr_Result_V1_t Aynr_Start_V1(Aynr_Context_V1_t *pAynrCtx)
{
    LOGI_ANR( "%s:enter!\n", __FUNCTION__);

    // initial checks
    if (pAynrCtx == NULL) {
        return (AYNR_RET_V1_NULL_POINTER);
    }

    if ((AYNR_STATE_V1_RUNNING == pAynrCtx->eState)
            || (AYNR_STATE_V1_LOCKED == pAynrCtx->eState)) {
        return (AYNR_RET_V1_FAILURE);
    }

    pAynrCtx->eState = AYNR_STATE_V1_RUNNING;

    LOGI_ANR( "%s:exit!\n", __FUNCTION__);
    return (AYNR_RET_V1_SUCCESS);
}


Aynr_Result_V1_t Aynr_Stop_V1(Aynr_Context_V1_t *pAynrCtx)
{
    LOGI_ANR( "%s:enter!\n", __FUNCTION__);

    // initial checks
    if (pAynrCtx == NULL) {
        return (AYNR_RET_V1_NULL_POINTER);
    }

    if (AYNR_STATE_V1_LOCKED == pAynrCtx->eState) {
        return (AYNR_RET_V1_FAILURE);
    }

    pAynrCtx->eState = AYNR_STATE_V1_STOPPED;

    LOGI_ANR( "%s:exit!\n", __FUNCTION__);
    return (AYNR_RET_V1_SUCCESS);
}


//anr inint
Aynr_Result_V1_t Aynr_Init_V1(Aynr_Context_V1_t **ppAynrCtx, CamCalibDbContext_t *pCalibDb)
{
    Aynr_Context_V1_t * pAynrCtx;

    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    pAynrCtx = (Aynr_Context_V1_t *)malloc(sizeof(Aynr_Context_V1_t));
    if(pAynrCtx == NULL) {
        LOGE_ANR("%s(%d): malloc fail\n", __FUNCTION__, __LINE__);
        return AYNR_RET_V1_NULL_POINTER;
    }

    memset(pAynrCtx, 0x00, sizeof(Aynr_Context_V1_t));

    //gain state init
    pAynrCtx->stGainState.gain_stat_full_last = -1;
    pAynrCtx->stGainState.gainState = -1;
    pAynrCtx->stGainState.gainState_last = -1;
    pAynrCtx->stGainState.gain_th0[0]    = 2.0;
    pAynrCtx->stGainState.gain_th1[0]    = 4.0;
    pAynrCtx->stGainState.gain_th0[1]    = 32.0;
    pAynrCtx->stGainState.gain_th1[1]    = 64.0;

    pAynrCtx->fLuma_SF_Strength = 1.0;
    pAynrCtx->refYuvBit = 8;

    pAynrCtx->eState = AYNR_STATE_V1_INITIALIZED;
    *ppAynrCtx = pAynrCtx;

    pAynrCtx->eMode = AYNR_OP_MODE_V1_AUTO;
    pAynrCtx->isIQParaUpdate = false;
    pAynrCtx->isGrayMode = false;

#if AYNR_USE_XML_FILE_V1
    //read v1 params from xml
    pAynrCtx->stYnrCalib =
        *(CalibDb_YNR_2_t*)(CALIBDB_GET_MODULE_PTR((void*)pCalibDb, ynr));

    CalibDb_MFNR_2_t *pMfnrCalib =
        (CalibDb_MFNR_2_t*)(CALIBDB_GET_MODULE_PTR((void*)pCalibDb, mfnr));
    pAynrCtx->mfnr_mode_3to1 = pMfnrCalib->mode_3to1;
#endif

#if RK_SIMULATOR_HW
    //just for v1 params from html

#endif

#if AYNR_USE_XML_FILE_V1
    pAynrCtx->stExpInfo.snr_mode = 0;
    pAynrCtx->eParamMode = AYNR_PARAM_MODE_V1_NORMAL;
    Aynr_ConfigSettingParam_V1(pAynrCtx, pAynrCtx->eParamMode, pAynrCtx->stExpInfo.snr_mode);
#endif

    LOGD_ANR("%s(%d):", __FUNCTION__, __LINE__);


    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return AYNR_RET_V1_SUCCESS;
}



//anr inint
Aynr_Result_V1_t Aynr_Init_Json_V1(Aynr_Context_V1_t **ppAynrCtx, CamCalibDbV2Context_t *pCalibDbV2)
{
    Aynr_Context_V1_t * pAynrCtx;

    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    pAynrCtx = (Aynr_Context_V1_t *)malloc(sizeof(Aynr_Context_V1_t));
    if(pAynrCtx == NULL) {
        LOGE_ANR("%s(%d): malloc fail\n", __FUNCTION__, __LINE__);
        return AYNR_RET_V1_NULL_POINTER;
    }

    memset(pAynrCtx, 0x00, sizeof(Aynr_Context_V1_t));

    //gain state init
    pAynrCtx->stGainState.gain_stat_full_last = -1;
    pAynrCtx->stGainState.gainState = -1;
    pAynrCtx->stGainState.gainState_last = -1;
    pAynrCtx->stGainState.gain_th0[0]    = 2.0;
    pAynrCtx->stGainState.gain_th1[0]    = 4.0;
    pAynrCtx->stGainState.gain_th0[1]    = 32.0;
    pAynrCtx->stGainState.gain_th1[1]    = 64.0;

    pAynrCtx->fLuma_SF_Strength = 1.0;
    pAynrCtx->refYuvBit = 8;

    pAynrCtx->eState = AYNR_STATE_V1_INITIALIZED;
    *ppAynrCtx = pAynrCtx;

    pAynrCtx->eMode = AYNR_OP_MODE_V1_AUTO;
    pAynrCtx->isIQParaUpdate = false;
    pAynrCtx->isGrayMode = false;

#if AYNR_USE_JSON_PARA_V1
    //read v1 params from xml
    CalibDbV2_YnrV1_t* calibv2_ynr_v1 =
        (CalibDbV2_YnrV1_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibDbV2, ynr_v1));
    ynr_calibdbV2_assign_v1(&pAynrCtx->ynr_v1, calibv2_ynr_v1);

    CalibDbV2_MFNR_t *calibv2_mfnr_v1 =
        (CalibDbV2_MFNR_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCalibDbV2, mfnr_v1));
    pAynrCtx->mfnr_mode_3to1 = calibv2_mfnr_v1->TuningPara.mode_3to1;


    LOGI_ANR("%s(%d): mode_3to1:%d ynr version:%s %p\n",
             __FUNCTION__, __LINE__,
             pAynrCtx->mfnr_mode_3to1,
             pAynrCtx->ynr_v1.Version,
             pAynrCtx->ynr_v1.Version);
#endif

#if RK_SIMULATOR_HW
    //just for v1 params from html

#endif

#if AYNR_USE_JSON_PARA_V1
    pAynrCtx->stExpInfo.snr_mode = 0;
    pAynrCtx->eParamMode = AYNR_PARAM_MODE_V1_NORMAL;
    Aynr_ConfigSettingParam_V1(pAynrCtx, pAynrCtx->eParamMode, pAynrCtx->stExpInfo.snr_mode);
#endif



    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return AYNR_RET_V1_SUCCESS;
}

//anr release
Aynr_Result_V1_t Aynr_Release_V1(Aynr_Context_V1_t *pAynrCtx)
{
    Aynr_Result_V1_t result = AYNR_RET_V1_SUCCESS;
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    if(pAynrCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNR_RET_V1_NULL_POINTER;
    }

    result = Aynr_Stop_V1(pAynrCtx);
    if (result != AYNR_RET_V1_SUCCESS) {
        LOGE_ANR( "%s: ANRStop() failed!\n", __FUNCTION__);
        return (result);
    }

    // check state
    if ((AYNR_STATE_V1_RUNNING == pAynrCtx->eState)
            || (AYNR_STATE_V1_LOCKED == pAynrCtx->eState)) {
        return (AYNR_RET_V1_BUSY);
    }

#if AYNR_USE_JSON_PARA_V1
    ynr_calibdbV2_free_v1(&pAynrCtx->ynr_v1);
#endif
    memset(pAynrCtx, 0x00, sizeof(Aynr_Context_V1_t));
    free(pAynrCtx);

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return AYNR_RET_V1_SUCCESS;
}

//anr config
Aynr_Result_V1_t Aynr_Prepare_V1(Aynr_Context_V1_t *pAynrCtx, Aynr_Config_V1_t* pAynrConfig)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    if(pAynrCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNR_RET_V1_INVALID_PARM;
    }

    if(pAynrConfig == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNR_RET_V1_INVALID_PARM;
    }

    if(!!(pAynrCtx->prepare_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB)) {
        Aynr_IQParaUpdate_V1(pAynrCtx);
    }

    Aynr_Start_V1(pAynrCtx);

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return AYNR_RET_V1_SUCCESS;
}

//anr reconfig
Aynr_Result_V1_t Aynr_ReConfig_V1(Aynr_Context_V1_t *pAynrCtx, Aynr_Config_V1_t* pAynrConfig)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    //need todo what?

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return AYNR_RET_V1_SUCCESS;
}

//anr reconfig
Aynr_Result_V1_t Aynr_IQParaUpdate_V1(Aynr_Context_V1_t *pAynrCtx)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    //need todo what?

    if(pAynrCtx->isIQParaUpdate) {
        LOGD_ANR("IQ data reconfig\n");
        Aynr_ConfigSettingParam_V1(pAynrCtx, pAynrCtx->eParamMode, pAynrCtx->stExpInfo.snr_mode);
        pAynrCtx->isIQParaUpdate = false;
    }

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return AYNR_RET_V1_SUCCESS;
}


//anr preprocess
Aynr_Result_V1_t Aynr_PreProcess_V1(Aynr_Context_V1_t *pAynrCtx)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    //need todo what?

    Aynr_IQParaUpdate_V1(pAynrCtx);

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return AYNR_RET_V1_SUCCESS;
}


Aynr_Result_V1_t Aynr_GainRatioProcess_V1(Aynr_GainState_t *pGainState, Aynr_ExpInfo_V1_t *pExpInfo)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    if(pGainState == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNR_RET_V1_INVALID_PARM;
    }

    if(pExpInfo == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNR_RET_V1_INVALID_PARM;
    }

    float gain_cur = pExpInfo->arAGain[pExpInfo->hdr_mode] * pExpInfo->arDGain[pExpInfo->hdr_mode];
    float th[2];
    float gain_th0[2];
    float gain_th1[2];
    for(int i = 0; i < 2; i++) {
        gain_th0[i]     = pGainState->gain_th0[i];
        gain_th1[i]     = pGainState->gain_th1[i];
        th[i]           = pow(2.0, (log2(gain_th0[i]) + log2(gain_th1[i])) / 2);
    }

    pGainState->gain_cur = gain_cur;

    int gain_stat_full = -1;
    int gain_stat_full_last = pGainState->gain_stat_full_last;
    int gain_stat_last = pGainState->gainState_last;
    int gain_stat_cur  = -1;
    int gain_stat = -1;

    if(gain_cur <= gain_th0[0])
    {
        gain_stat_full = 0;
        gain_stat_cur = 0;
    }
    else if(gain_cur <= gain_th1[0] && gain_cur >= gain_th0[0])
    {
        gain_stat_full = 1;
    }
    else if(gain_cur <= gain_th0[1] && gain_cur >= gain_th1[0])
    {
        gain_stat_full = 2;
        gain_stat_cur = 1;
    }
    else if(gain_cur <= gain_th1[1] && gain_cur >= gain_th0[1])
    {
        gain_stat_full = 3;
    }
    else if(gain_cur >= gain_th1[1])
    {
        gain_stat_full = 4;
        gain_stat_cur = 2;
    }
    if(gain_stat_last == -1 || (abs(gain_stat_full - gain_stat_full_last) >= 2 && gain_stat_cur == -1)) {
        if(gain_cur <= th[0])
            gain_stat_cur = 0;
        else if(gain_cur <= th[1])
            gain_stat_cur = 1;
        else
            gain_stat_cur = 2;
    }
    if (gain_stat_cur != -1) {
        gain_stat_last      = gain_stat_cur;
        gain_stat_full_last = gain_stat_full;
        gain_stat       = gain_stat_cur;
    } else {
        gain_stat       = gain_stat_last;
    }
    if (gain_stat == 0)
        pGainState->ratio = 16;
    else if (gain_stat == 1)
        pGainState->ratio = 1;
    else
        pGainState->ratio = 1.0 / 16.0;

    pGainState->gain_stat_full_last     = gain_stat_full_last;
    pGainState->gainState       = gain_stat;
    pGainState->gainState_last  = gain_stat_last;

    LOGD_ANR("%s:%d gain_cur:%f gain th %f %fd %f %f ratio:%f gain_state:%d %d full    %d %d\n",
             __FUNCTION__, __LINE__,
             gain_cur,
             gain_th0[0], gain_th0[1],
             gain_th1[0], gain_th1[1],
             pGainState->ratio,
             pGainState->gainState_last,
             pGainState->gainState,
             pGainState->gain_stat_full_last,
             gain_stat_full);


    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);

    return AYNR_RET_V1_SUCCESS;
}


//anr process
Aynr_Result_V1_t Aynr_Process_V1(Aynr_Context_V1_t *pAynrCtx, Aynr_ExpInfo_V1_t *pExpInfo)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    Aynr_ParamMode_V1_t mode = AYNR_PARAM_MODE_V1_INVALID;

    if(pAynrCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNR_RET_V1_INVALID_PARM;
    }

    if(pExpInfo == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNR_RET_V1_INVALID_PARM;
    }

    if(pAynrCtx->eState != AYNR_STATE_V1_RUNNING) {
        return AYNR_RET_V1_SUCCESS;
    }

    Aynr_GainRatioProcess_V1(&pAynrCtx->stGainState, pExpInfo);
    Aynr_ParamModeProcess_V1(pAynrCtx, pExpInfo, &mode);

    pExpInfo->mfnr_mode_3to1 = pAynrCtx->mfnr_mode_3to1;
    if(pExpInfo->mfnr_mode_3to1 ) {
        pExpInfo->snr_mode = pExpInfo->pre_snr_mode;
    } else {
        pExpInfo->snr_mode = pExpInfo->cur_snr_mode;
    }

    if(pAynrCtx->eMode == AYNR_OP_MODE_V1_AUTO) {

        LOGD_ANR("%s(%d): \n", __FUNCTION__, __LINE__);

#if AYNR_USE_XML_FILE_V1
        if(pExpInfo->snr_mode != pAynrCtx->stExpInfo.snr_mode || pAynrCtx->eParamMode != mode) {
            LOGD_ANR("param mode:%d snr_mode:%d\n", mode, pExpInfo->snr_mode);
            pAynrCtx->eParamMode = mode;
            Aynr_ConfigSettingParam_V1(pAynrCtx, pAynrCtx->eParamMode, pExpInfo->snr_mode);
        }
#endif

        //select param
        select_ynr_params_by_ISO_v1(&pAynrCtx->stAuto.stParams, &pAynrCtx->stAuto.stSelect, pExpInfo, pAynrCtx->refYuvBit);

    } else if(pAynrCtx->eMode == AYNR_OP_MODE_V1_MANUAL) {
        //TODO
    }

    memcpy(&pAynrCtx->stExpInfo, pExpInfo, sizeof(Aynr_ExpInfo_V1_t));

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return AYNR_RET_V1_SUCCESS;

}



//anr get result
Aynr_Result_V1_t Aynr_GetProcResult_V1(Aynr_Context_V1_t *pAynrCtx, Aynr_ProcResult_V1_t* pAynrResult)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    if(pAynrCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNR_RET_V1_INVALID_PARM;
    }

    if(pAynrResult == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNR_RET_V1_INVALID_PARM;
    }

    if(pAynrCtx->eMode == AYNR_OP_MODE_V1_AUTO) {
        pAynrResult->stSelect = pAynrCtx->stAuto.stSelect;
        pAynrResult->ynrEn = pAynrCtx->stAuto.ynrEn;

    } else if(pAynrCtx->eMode == AYNR_OP_MODE_V1_MANUAL) {
        //TODO
        pAynrResult->stSelect = pAynrCtx->stManual.stSelect;
        pAynrResult->ynrEn = pAynrCtx->stManual.ynrEn;
        pAynrCtx->fLuma_SF_Strength = 1.0;
    }

    //transfer to reg value
    ynr_fix_transfer_v1(&pAynrResult->stSelect, &pAynrResult->stFix, pAynrCtx->stGainState.ratio, pAynrCtx->fLuma_SF_Strength);
    pAynrResult->stFix.ynr_en = pAynrResult->ynrEn;

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return AYNR_RET_V1_SUCCESS;
}

Aynr_Result_V1_t Aynr_ConfigSettingParam_V1(Aynr_Context_V1_t *pAynrCtx, Aynr_ParamMode_V1_t eParamMode, int snr_mode)
{
    char snr_name[CALIBDB_NR_SHARP_NAME_LENGTH];
    char param_mode_name[CALIBDB_MAX_MODE_NAME_LENGTH];
    memset(param_mode_name, 0x00, sizeof(param_mode_name));
    memset(snr_name, 0x00, sizeof(snr_name));

    if(pAynrCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNR_RET_V1_INVALID_PARM;
    }

    //select param mode first
    if(eParamMode == AYNR_PARAM_MODE_V1_NORMAL) {
        sprintf(param_mode_name, "%s", "normal");
    } else if(eParamMode == AYNR_PARAM_MODE_V1_HDR) {
        sprintf(param_mode_name, "%s", "hdr");
    } else if(eParamMode == AYNR_PARAM_MODE_V1_GRAY) {
        sprintf(param_mode_name, "%s", "gray");
    } else {
        LOGE_ANR("%s(%d): not support param mode!\n", __FUNCTION__, __LINE__);
        sprintf(param_mode_name, "%s", "normal");
    }


    //then select snr mode next
    if(snr_mode == 1) {
        sprintf(snr_name, "%s", "HSNR");
    } else if(snr_mode == 0) {
        sprintf(snr_name, "%s", "LSNR");
    } else {
        LOGE_ANR("%s(%d): not support snr mode!\n", __FUNCTION__, __LINE__);
        sprintf(snr_name, "%s", "LSNR");
    }

#if AYNR_USE_JSON_PARA_V1
    pAynrCtx->stAuto.ynrEn = pAynrCtx->ynr_v1.TuningPara.enable;
    ynr_config_setting_param_json_v1(&pAynrCtx->stAuto.stParams, &pAynrCtx->ynr_v1, param_mode_name, snr_name);
#else
    pAynrCtx->stAuto.ynrEn = pAynrCtx->stYnrCalib.enable;
    ynr_config_setting_param_v1(&pAynrCtx->stAuto.stParams, &pAynrCtx->stYnrCalib, param_mode_name, snr_name);
#endif

    return AYNR_RET_V1_SUCCESS;
}

Aynr_Result_V1_t Aynr_ParamModeProcess_V1(Aynr_Context_V1_t *pAynrCtx, Aynr_ExpInfo_V1_t *pExpInfo, Aynr_ParamMode_V1_t *mode) {

    if(pAynrCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNR_RET_V1_INVALID_PARM;
    }

    Aynr_Result_V1_t res  = AYNR_RET_V1_SUCCESS;
    *mode = pAynrCtx->eParamMode;

    if(pAynrCtx->isGrayMode) {
        *mode = AYNR_PARAM_MODE_V1_GRAY;
    } else if(pExpInfo->hdr_mode == 0) {
        *mode = AYNR_PARAM_MODE_V1_NORMAL;
    } else if(pExpInfo->hdr_mode >= 1) {
        *mode = AYNR_PARAM_MODE_V1_HDR;
    } else {
        *mode = AYNR_PARAM_MODE_V1_NORMAL;
    }

    return res;
}


RKAIQ_END_DECLARE


