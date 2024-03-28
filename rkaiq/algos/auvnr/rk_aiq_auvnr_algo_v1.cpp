
#include "rk_aiq_auvnr_algo_v1.h"
#include "rk_aiq_auvnr_algo_itf_v1.h"

RKAIQ_BEGIN_DECLARE

Auvnr_result_t Auvnr_Start_V1(Auvnr_Context_V1_t *pAuvnrCtx)
{
    LOGI_ANR( "%s:enter!\n", __FUNCTION__);

    // initial checks
    if (pAuvnrCtx == NULL) {
        return (AUVNR_RET_NULL_POINTER);
    }

    if ((AUVNR_STATE_RUNNING == pAuvnrCtx->eState)
            || (AUVNR_STATE_LOCKED == pAuvnrCtx->eState)) {
        return (AUVNR_RET_FAILURE);
    }

    pAuvnrCtx->eState = AUVNR_STATE_RUNNING;

    LOGI_ANR( "%s:exit!\n", __FUNCTION__);
    return (AUVNR_RET_SUCCESS);
}


Auvnr_result_t Auvnr_Stop_V1(Auvnr_Context_V1_t *pAuvnrCtx)
{
    LOGI_ANR( "%s:enter!\n", __FUNCTION__);

    // initial checks
    if (pAuvnrCtx == NULL) {
        return (AUVNR_RET_NULL_POINTER);
    }

    if (AUVNR_STATE_LOCKED == pAuvnrCtx->eState) {
        return (AUVNR_RET_FAILURE);
    }

    pAuvnrCtx->eState = AUVNR_STATE_STOPPED;

    LOGI_ANR( "%s:exit!\n", __FUNCTION__);
    return (AUVNR_RET_SUCCESS);
}


//anr inint
Auvnr_result_t Auvnr_Init_V1(Auvnr_Context_V1_t **ppAuvnrCtx, CamCalibDbContext_t *pCalibDb)
{
    Auvnr_Context_V1_t * pAuvnrCtx;

    LOGE_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    pAuvnrCtx = (Auvnr_Context_V1_t *)malloc(sizeof(Auvnr_Context_V1_t));
    if(pAuvnrCtx == NULL) {
        LOGE_ANR("%s(%d): malloc fail\n", __FUNCTION__, __LINE__);
        return AUVNR_RET_NULL_POINTER;
    }

    memset(pAuvnrCtx, 0x00, sizeof(Auvnr_Context_V1_t));

    //gain state init
    pAuvnrCtx->stGainState.gain_stat_full_last = -1;
    pAuvnrCtx->stGainState.gainState = -1;
    pAuvnrCtx->stGainState.gainState_last = -1;
    pAuvnrCtx->stGainState.gain_th0[0]    = 2.0;
    pAuvnrCtx->stGainState.gain_th1[0]    = 4.0;
    pAuvnrCtx->stGainState.gain_th0[1]    = 32.0;
    pAuvnrCtx->stGainState.gain_th1[1]    = 64.0;

    pAuvnrCtx->fChrom_SF_Strength = 1.0;

    pAuvnrCtx->eState = AUVNR_STATE_INITIALIZED;
    *ppAuvnrCtx = pAuvnrCtx;

    pAuvnrCtx->eMode = AUVNR_OP_MODE_AUTO;
    pAuvnrCtx->isIQParaUpdate = false;
    pAuvnrCtx->isGrayMode = false;


#if AUVNR_USE_XML_FILE_V1
    //read v1 params from xml
    pAuvnrCtx->stUvnrCalib =
        *(CalibDb_UVNR_2_t*)(CALIBDB_GET_MODULE_PTR((void*)pCalibDb, uvnr));


    CalibDb_MFNR_2_t *pMfnrCalib =
        (CalibDb_MFNR_2_t*)(CALIBDB_GET_MODULE_PTR((void*)pCalibDb, mfnr));
    pAuvnrCtx->mfnr_mode_3to1 = pMfnrCalib->mode_3to1;
    pAuvnrCtx->mfnr_local_gain_en = pMfnrCalib->local_gain_en;


    LOGE_ANR("%s(%d): uvnr version:%s mode_3to1:%d\n",
             __FUNCTION__, __LINE__,
             pAuvnrCtx->stUvnrCalib.version,
             pAuvnrCtx->mfnr_mode_3to1);
#endif

#if RK_SIMULATOR_HW
    //just for v2 params from html

#endif

#if AUVNR_USE_XML_FILE_V1
    pAuvnrCtx->stExpInfo.snr_mode = 1;
    pAuvnrCtx->eParamMode = AUVNR_PARAM_MODE_NORMAL;
    Auvnr_ConfigSettingParam_V1(pAuvnrCtx, pAuvnrCtx->eParamMode, pAuvnrCtx->stExpInfo.snr_mode);
#endif

    LOGD_ANR("%s(%d):", __FUNCTION__, __LINE__);


    LOGE_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return AUVNR_RET_SUCCESS;
}



//anr inint
Auvnr_result_t Auvnr_Init_Json_V1(Auvnr_Context_V1_t **ppAuvnrCtx, CamCalibDbV2Context_t *pCalibDbV2)
{
    Auvnr_Context_V1_t * pAuvnrCtx;

    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    pAuvnrCtx = (Auvnr_Context_V1_t *)malloc(sizeof(Auvnr_Context_V1_t));
    if(pAuvnrCtx == NULL) {
        LOGE_ANR("%s(%d): malloc fail\n", __FUNCTION__, __LINE__);
        return AUVNR_RET_NULL_POINTER;
    }

    memset(pAuvnrCtx, 0x00, sizeof(Auvnr_Context_V1_t));

    //gain state init
    pAuvnrCtx->stGainState.gain_stat_full_last = -1;
    pAuvnrCtx->stGainState.gainState = -1;
    pAuvnrCtx->stGainState.gainState_last = -1;
    pAuvnrCtx->stGainState.gain_th0[0]    = 2.0;
    pAuvnrCtx->stGainState.gain_th1[0]    = 4.0;
    pAuvnrCtx->stGainState.gain_th0[1]    = 32.0;
    pAuvnrCtx->stGainState.gain_th1[1]    = 64.0;

    pAuvnrCtx->fChrom_SF_Strength = 1.0;

    pAuvnrCtx->eState = AUVNR_STATE_INITIALIZED;
    *ppAuvnrCtx = pAuvnrCtx;

    pAuvnrCtx->eMode = AUVNR_OP_MODE_AUTO;
    pAuvnrCtx->isIQParaUpdate = false;
    pAuvnrCtx->isGrayMode = false;

#if AUVNR_USE_JSON_FILE_V1
    //read v1 params from xml
    CalibDbV2_UVNR_t* calibv2_uvnr_v1 =
        (CalibDbV2_UVNR_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibDbV2, uvnr_v1));
    uvnr_calibdbV2_assign_v1(&pAuvnrCtx->uvnr_v1, calibv2_uvnr_v1);

    CalibDbV2_MFNR_t* pCalibv2_mfnr_v1 =
        (CalibDbV2_MFNR_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibDbV2, mfnr_v1));
    pAuvnrCtx->mfnr_mode_3to1 = pCalibv2_mfnr_v1->TuningPara.mode_3to1;
    pAuvnrCtx->mfnr_local_gain_en = pCalibv2_mfnr_v1->TuningPara.local_gain_en;

    LOGD_ANR("%s(%d): uvnr version:%s mode_3to1:%d\n",
             __FUNCTION__, __LINE__,
             pAuvnrCtx->stUvnrCalib.version,
             pAuvnrCtx->mfnr_mode_3to1);
#endif

#if RK_SIMULATOR_HW
    //just for v2 params from html

#endif

#if AUVNR_USE_JSON_FILE_V1
    pAuvnrCtx->stExpInfo.snr_mode = 1;
    pAuvnrCtx->eParamMode = AUVNR_PARAM_MODE_NORMAL;
    Auvnr_ConfigSettingParam_V1(pAuvnrCtx, pAuvnrCtx->eParamMode, pAuvnrCtx->stExpInfo.snr_mode);
#endif


    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return AUVNR_RET_SUCCESS;
}


//anr release
Auvnr_result_t Auvnr_Release_V1(Auvnr_Context_V1_t *pAuvnrCtx)
{
    Auvnr_result_t result = AUVNR_RET_SUCCESS;
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    if(pAuvnrCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AUVNR_RET_NULL_POINTER;
    }

    result = Auvnr_Stop_V1(pAuvnrCtx);
    if (result != AUVNR_RET_SUCCESS) {
        LOGE_ANR( "%s: ANRStop() failed!\n", __FUNCTION__);
        return (result);
    }

    // check state
    if ((AUVNR_STATE_RUNNING == pAuvnrCtx->eState)
            || (AUVNR_STATE_LOCKED == pAuvnrCtx->eState)) {
        return (AUVNR_RET_BUSY);
    }

#if AUVNR_USE_JSON_FILE_V1
    uvnr_calibdbV2_free_v1(&pAuvnrCtx->uvnr_v1);
#endif

    memset(pAuvnrCtx, 0x00, sizeof(Auvnr_Context_V1_t));
    free(pAuvnrCtx);

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return AUVNR_RET_SUCCESS;
}

//anr config
Auvnr_result_t Auvnr_Prepare_V1(Auvnr_Context_V1_t *pAuvnrCtx, Auvnr_Config_V1_t* pAuvnrConfig)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    if(pAuvnrCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AUVNR_RET_INVALID_PARM;
    }

    if(pAuvnrConfig == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AUVNR_RET_INVALID_PARM;
    }

    if(!!(pAuvnrCtx->prepare_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB)) {
        Auvnr_IQParaUpdate_V1(pAuvnrCtx);
    }

    pAuvnrCtx->rawWidth = pAuvnrConfig->rawWidth;
    pAuvnrCtx->rawHeight = pAuvnrConfig->rawHeight;
    Auvnr_Start_V1(pAuvnrCtx);

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return AUVNR_RET_SUCCESS;
}

//anr reconfig
Auvnr_result_t Auvnr_ReConfig_V1(Auvnr_Context_V1_t *pAuvnrCtx, Auvnr_Config_V1_t* pAuvnrConfig)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    //need todo what?

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return AUVNR_RET_SUCCESS;
}

//anr reconfig
Auvnr_result_t Auvnr_IQParaUpdate_V1(Auvnr_Context_V1_t *pAuvnrCtx)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    //need todo what?

    if(pAuvnrCtx->isIQParaUpdate) {
        LOGD_ANR("IQ data reconfig\n");
        Auvnr_ConfigSettingParam_V1(pAuvnrCtx, pAuvnrCtx->eParamMode, pAuvnrCtx->stExpInfo.snr_mode);
        pAuvnrCtx->isIQParaUpdate = false;
    }

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return AUVNR_RET_SUCCESS;
}


//anr preprocess
Auvnr_result_t Auvnr_PreProcess_V1(Auvnr_Context_V1_t *pAuvnrCtx)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    //need todo what?

    Auvnr_IQParaUpdate_V1(pAuvnrCtx);

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return AUVNR_RET_SUCCESS;
}

Auvnr_result_t Auvnr_GainRatioProcess_V1(Auvnr_GainState_t *pGainState, Auvnr_ExpInfo_t *pExpInfo)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    if(pGainState == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AUVNR_RET_INVALID_PARM;
    }

    if(pExpInfo == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AUVNR_RET_INVALID_PARM;
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

    return AUVNR_RET_SUCCESS;
}


//anr process
Auvnr_result_t Auvnr_Process_V1(Auvnr_Context_V1_t *pAuvnrCtx, Auvnr_ExpInfo_t *pExpInfo)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    Auvnr_ParamMode_t mode = AUVNR_PARAM_MODE_INVALID;

    if(pAuvnrCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AUVNR_RET_INVALID_PARM;
    }

    if(pExpInfo == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AUVNR_RET_INVALID_PARM;
    }

    if(pAuvnrCtx->eState != AUVNR_STATE_RUNNING) {
        return AUVNR_RET_SUCCESS;
    }


    Auvnr_ParamModeProcess_V1(pAuvnrCtx, pExpInfo, &mode);
    Auvnr_GainRatioProcess_V1(&pAuvnrCtx->stGainState, pExpInfo);
    pExpInfo->mfnr_mode_3to1 = pAuvnrCtx->mfnr_mode_3to1;
    if(pExpInfo->mfnr_mode_3to1 ) {
        pExpInfo->snr_mode = pExpInfo->pre_snr_mode;
    } else {
        pExpInfo->snr_mode = pExpInfo->cur_snr_mode;
    }

    if(pAuvnrCtx->eMode == AUVNR_OP_MODE_AUTO) {

        LOGD_ANR("%s(%d): \n", __FUNCTION__, __LINE__);

#if AUVNR_USE_XML_FILE_V1
        if(pExpInfo->snr_mode != pAuvnrCtx->stExpInfo.snr_mode || pAuvnrCtx->eParamMode != mode) {
            LOGD_ANR("param mode:%d snr_mode:%d\n", mode, pExpInfo->snr_mode);
            pAuvnrCtx->eParamMode = mode;
            Auvnr_ConfigSettingParam_V1(pAuvnrCtx, pAuvnrCtx->eParamMode, pExpInfo->snr_mode);
        }
#endif

        //select param
        select_uvnr_params_by_ISO_v1(&pAuvnrCtx->stAuto.stParams, &pAuvnrCtx->stAuto.stSelect, pExpInfo);

    } else if(pAuvnrCtx->eMode == AUVNR_OP_MODE_MANUAL) {
        //TODO
    }

    memcpy(&pAuvnrCtx->stExpInfo, pExpInfo, sizeof(Auvnr_ExpInfo_t));

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return AUVNR_RET_SUCCESS;

}


//anr get result
Auvnr_result_t Auvnr_GetProcResult_V1(Auvnr_Context_V1_t *pAuvnrCtx, Auvnr_ProcResult_V1_t* pAuvnrResult)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    if(pAuvnrCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AUVNR_RET_INVALID_PARM;
    }

    if(pAuvnrResult == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AUVNR_RET_INVALID_PARM;
    }

    if(pAuvnrCtx->eMode == AUVNR_OP_MODE_AUTO) {
        pAuvnrResult->stSelect = pAuvnrCtx->stAuto.stSelect;
        pAuvnrResult->uvnrEn = pAuvnrCtx->stAuto.uvnrEn;
    } else if(pAuvnrCtx->eMode == AUVNR_OP_MODE_MANUAL) {
        //TODO
        pAuvnrResult->stSelect = pAuvnrCtx->stManual.stSelect;
        pAuvnrResult->uvnrEn = pAuvnrCtx->stManual.uvnrEn;
        pAuvnrCtx->fChrom_SF_Strength = 1.0;
    }

    //transfer to reg value
    uvnr_fix_transfer_v1(&pAuvnrResult->stSelect, &pAuvnrResult->stFix,  &pAuvnrCtx->stExpInfo, pAuvnrCtx->stGainState.ratio, pAuvnrCtx->fChrom_SF_Strength);
    pAuvnrResult->stFix.uvnr_en = pAuvnrResult->uvnrEn;

    if(pAuvnrCtx->isGrayMode) {
        //uvnr disable
        pAuvnrResult->stFix.uvnr_step1_en = 0;
        pAuvnrResult->stFix.uvnr_step2_en = 0;
    }

    if(pAuvnrCtx->mfnr_local_gain_en) {
        pAuvnrResult->stFix.nr_gain_en = 0;
    } else {
        pAuvnrResult->stFix.nr_gain_en = 1;
    }

    LOGD_ANR("%s:%d xml:local:%d mode:%d  reg: local gain:%d  mfnr gain:%d mode:%d\n",
             __FUNCTION__, __LINE__);

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return AUVNR_RET_SUCCESS;
}

Auvnr_result_t Auvnr_ConfigSettingParam_V1(Auvnr_Context_V1_t *pAuvnrCtx, Auvnr_ParamMode_t eParamMode, int snr_mode)
{
    char snr_name[CALIBDB_NR_SHARP_NAME_LENGTH];
    char param_mode_name[CALIBDB_MAX_MODE_NAME_LENGTH];
    memset(param_mode_name, 0x00, sizeof(param_mode_name));
    memset(snr_name, 0x00, sizeof(snr_name));

    if(pAuvnrCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AUVNR_RET_INVALID_PARM;
    }

    //select param mode first
    if(eParamMode == AUVNR_PARAM_MODE_NORMAL) {
        sprintf(param_mode_name, "%s", "normal");
    } else if(eParamMode == AUVNR_PARAM_MODE_HDR) {
        sprintf(param_mode_name, "%s", "hdr");
    } else if(eParamMode == AUVNR_PARAM_MODE_GRAY) {
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

#if (AUVNR_USE_JSON_FILE_V1)
    pAuvnrCtx->stAuto.uvnrEn = pAuvnrCtx->uvnr_v1.TuningPara.enable;
    uvnr_config_setting_param_json_v1(&pAuvnrCtx->stAuto.stParams, &pAuvnrCtx->uvnr_v1, param_mode_name, snr_name);
#else
    pAuvnrCtx->stAuto.uvnrEn = pAuvnrCtx->stUvnrCalib.enable;
    uvnr_config_setting_param_v1(&pAuvnrCtx->stAuto.stParams, &pAuvnrCtx->stUvnrCalib, param_mode_name, snr_name);
#endif

    return AUVNR_RET_SUCCESS;
}

Auvnr_result_t Auvnr_ParamModeProcess_V1(Auvnr_Context_V1_t *pAuvnrCtx, Auvnr_ExpInfo_t *pExpInfo, Auvnr_ParamMode_t *mode) {

    if(pAuvnrCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AUVNR_RET_INVALID_PARM;
    }

    Auvnr_result_t res  = AUVNR_RET_SUCCESS;
    *mode = pAuvnrCtx->eParamMode;

    if(pAuvnrCtx->isGrayMode) {
        *mode = AUVNR_PARAM_MODE_GRAY;
    } else if(pExpInfo->hdr_mode == 0) {
        *mode = AUVNR_PARAM_MODE_NORMAL;
    } else if(pExpInfo->hdr_mode >= 1) {
        *mode = AUVNR_PARAM_MODE_HDR;
    } else {
        *mode = AUVNR_PARAM_MODE_NORMAL;
    }

    return res;
}


RKAIQ_END_DECLARE


