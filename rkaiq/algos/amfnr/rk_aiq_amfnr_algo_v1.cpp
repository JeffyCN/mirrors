
#include "rk_aiq_amfnr_algo_v1.h"
#include "rk_aiq_amfnr_algo_itf_v1.h"

RKAIQ_BEGIN_DECLARE

Amfnr_Result_V1_t Amfnr_Start_V1(Amfnr_Context_V1_t *pAmfnrCtx)
{
    LOGI_ANR( "%s:enter!\n", __FUNCTION__);

    // initial checks
    if (pAmfnrCtx == NULL) {
        return (AMFNR_RET_V1_NULL_POINTER);
    }

    if ((AMFNR_STATE_V1_RUNNING == pAmfnrCtx->eState)
            || (AMFNR_STATE_V1_LOCKED == pAmfnrCtx->eState)) {
        return (AMFNR_RET_V1_FAILURE);
    }

    pAmfnrCtx->eState = AMFNR_STATE_V1_RUNNING;

    LOGI_ANR( "%s:exit!\n", __FUNCTION__);
    return (AMFNR_RET_V1_SUCCESS);
}


Amfnr_Result_V1_t Amfnr_Stop_V1(Amfnr_Context_V1_t *pAmfnrCtx)
{
    LOGI_ANR( "%s:enter!\n", __FUNCTION__);

    // initial checks
    if (pAmfnrCtx == NULL) {
        return (AMFNR_RET_V1_NULL_POINTER);
    }

    if (AMFNR_STATE_V1_LOCKED == pAmfnrCtx->eState) {
        return (AMFNR_RET_V1_FAILURE);
    }

    pAmfnrCtx->eState = AMFNR_STATE_V1_STOPPED;

    LOGI_ANR( "%s:exit!\n", __FUNCTION__);
    return (AMFNR_RET_V1_SUCCESS);
}


//anr inint
Amfnr_Result_V1_t Amfnr_Init_V1(Amfnr_Context_V1_t **ppAmfnrCtx, CamCalibDbContext_t *pCalibDb)
{
    Amfnr_Context_V1_t * pAmfnrCtx;

    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    pAmfnrCtx = (Amfnr_Context_V1_t *)malloc(sizeof(Amfnr_Context_V1_t));
    if(pAmfnrCtx == NULL) {
        LOGE_ANR("%s(%d): malloc fail\n", __FUNCTION__, __LINE__);
        return AMFNR_RET_V1_NULL_POINTER;
    }

    memset(pAmfnrCtx, 0x00, sizeof(Amfnr_Context_V1_t));

    //gain state init
    pAmfnrCtx->stGainState.gain_stat_full_last = -1;
    pAmfnrCtx->stGainState.gainState = -1;
    pAmfnrCtx->stGainState.gainState_last = -1;
    pAmfnrCtx->stGainState.gain_th0[0]    = 2.0;
    pAmfnrCtx->stGainState.gain_th1[0]    = 4.0;
    pAmfnrCtx->stGainState.gain_th0[1]    = 32.0;
    pAmfnrCtx->stGainState.gain_th1[1]    = 64.0;

    pAmfnrCtx->fLuma_TF_Strength = 1.0;
    pAmfnrCtx->fChroma_TF_Strength = 1.0;
    pAmfnrCtx->refYuvBit = 8;

    pAmfnrCtx->eState = AMFNR_STATE_V1_INITIALIZED;
    *ppAmfnrCtx = pAmfnrCtx;

    pAmfnrCtx->eMode = AMFNR_OP_MODE_V1_AUTO;
    pAmfnrCtx->isIQParaUpdate = false;
    pAmfnrCtx->isGrayMode = false;

#if AMFNR_USE_XML_FILE_V1
    //read v1 params from xml
    pAmfnrCtx->stMfnrCalib =
        *(CalibDb_MFNR_2_t*)(CALIBDB_GET_MODULE_PTR((void*)pCalibDb, mfnr));

#endif

#if RK_SIMULATOR_HW
    //just for v1 params from html

#endif

#if AMFNR_USE_XML_FILE_V1
    pAmfnrCtx->stExpInfo.snr_mode = 0;
    pAmfnrCtx->eParamMode = AMFNR_PARAM_MODE_V1_NORMAL;
    Amfnr_ConfigSettingParam_V1(pAmfnrCtx, pAmfnrCtx->eParamMode, pAmfnrCtx->stExpInfo.snr_mode);
#endif

    LOGD_ANR("%s(%d):", __FUNCTION__, __LINE__);


    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return AMFNR_RET_V1_SUCCESS;
}



//anr inint
Amfnr_Result_V1_t Amfnr_Init_Json_V1(Amfnr_Context_V1_t **ppAmfnrCtx, CamCalibDbV2Context_t *pCalibDbV2)
{
    Amfnr_Context_V1_t * pAmfnrCtx;

    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    pAmfnrCtx = (Amfnr_Context_V1_t *)malloc(sizeof(Amfnr_Context_V1_t));
    if(pAmfnrCtx == NULL) {
        LOGE_ANR("%s(%d): malloc fail\n", __FUNCTION__, __LINE__);
        return AMFNR_RET_V1_NULL_POINTER;
    }

    memset(pAmfnrCtx, 0x00, sizeof(Amfnr_Context_V1_t));

    //gain state init
    pAmfnrCtx->stGainState.gain_stat_full_last = -1;
    pAmfnrCtx->stGainState.gainState = -1;
    pAmfnrCtx->stGainState.gainState_last = -1;
    pAmfnrCtx->stGainState.gain_th0[0]    = 2.0;
    pAmfnrCtx->stGainState.gain_th1[0]    = 4.0;
    pAmfnrCtx->stGainState.gain_th0[1]    = 32.0;
    pAmfnrCtx->stGainState.gain_th1[1]    = 64.0;

    pAmfnrCtx->fLuma_TF_Strength = 1.0;
    pAmfnrCtx->fChroma_TF_Strength = 1.0;
    pAmfnrCtx->refYuvBit = 8;

    pAmfnrCtx->eState = AMFNR_STATE_V1_INITIALIZED;
    *ppAmfnrCtx = pAmfnrCtx;

    pAmfnrCtx->eMode = AMFNR_OP_MODE_V1_AUTO;
    pAmfnrCtx->isIQParaUpdate = false;
    pAmfnrCtx->isGrayMode = false;

#if AMFNR_USE_JSON_PARA_V1
    //read v1 params from xml
    CalibDbV2_MFNR_t* calibv2_mfnr_v1 =
        (CalibDbV2_MFNR_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibDbV2, mfnr_v1));
    mfnr_calibdbV2_assign_v1(&pAmfnrCtx->mfnr_v1, calibv2_mfnr_v1);


    LOGI_ANR("%s(%d): mode_3to1:%d mfnr version:%s %p\n",
             __FUNCTION__, __LINE__,
             pAmfnrCtx->mfnr_mode_3to1,
             pAmfnrCtx->mfnr_v1.Version,
             pAmfnrCtx->mfnr_v1.Version);
#endif

#if RK_SIMULATOR_HW
    //just for v1 params from html

#endif

#if AMFNR_USE_JSON_PARA_V1
    pAmfnrCtx->stExpInfo.snr_mode = 0;
    pAmfnrCtx->eParamMode = AMFNR_PARAM_MODE_V1_NORMAL;
    Amfnr_ConfigSettingParam_V1(pAmfnrCtx, pAmfnrCtx->eParamMode, pAmfnrCtx->stExpInfo.snr_mode);
#endif



    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return AMFNR_RET_V1_SUCCESS;
}

//anr release
Amfnr_Result_V1_t Amfnr_Release_V1(Amfnr_Context_V1_t *pAmfnrCtx)
{
    Amfnr_Result_V1_t result = AMFNR_RET_V1_SUCCESS;
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    if(pAmfnrCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AMFNR_RET_V1_NULL_POINTER;
    }

    result = Amfnr_Stop_V1(pAmfnrCtx);
    if (result != AMFNR_RET_V1_SUCCESS) {
        LOGE_ANR( "%s: ANRStop() failed!\n", __FUNCTION__);
        return (result);
    }

    // check state
    if ((AMFNR_STATE_V1_RUNNING == pAmfnrCtx->eState)
            || (AMFNR_STATE_V1_LOCKED == pAmfnrCtx->eState)) {
        return (AMFNR_RET_V1_BUSY);
    }

#if AMFNR_USE_JSON_PARA_V1
    mfnr_calibdbV2_free_v1(&pAmfnrCtx->mfnr_v1);
#endif
    memset(pAmfnrCtx, 0x00, sizeof(Amfnr_Context_V1_t));
    free(pAmfnrCtx);

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return AMFNR_RET_V1_SUCCESS;
}

//anr config
Amfnr_Result_V1_t Amfnr_Prepare_V1(Amfnr_Context_V1_t *pAmfnrCtx, Amfnr_Config_V1_t* pAmfnrConfig)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    if(pAmfnrCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AMFNR_RET_V1_INVALID_PARM;
    }

    if(pAmfnrConfig == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AMFNR_RET_V1_INVALID_PARM;
    }

    if(!!(pAmfnrCtx->prepare_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB)) {
        Amfnr_IQParaUpdate_V1(pAmfnrCtx);
    }

    Amfnr_Start_V1(pAmfnrCtx);

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return AMFNR_RET_V1_SUCCESS;
}

//anr reconfig
Amfnr_Result_V1_t Amfnr_ReConfig_V1(Amfnr_Context_V1_t *pAmfnrCtx, Amfnr_Config_V1_t* pAmfnrConfig)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    //need todo what?

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return AMFNR_RET_V1_SUCCESS;
}

//anr reconfig
Amfnr_Result_V1_t Amfnr_IQParaUpdate_V1(Amfnr_Context_V1_t *pAmfnrCtx)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    //need todo what?

    if(pAmfnrCtx->isIQParaUpdate) {
        LOGD_ANR("IQ data reconfig\n");
        Amfnr_ConfigSettingParam_V1(pAmfnrCtx, pAmfnrCtx->eParamMode, pAmfnrCtx->stExpInfo.snr_mode);
        pAmfnrCtx->isIQParaUpdate = false;
    }

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return AMFNR_RET_V1_SUCCESS;
}


//anr preprocess
Amfnr_Result_V1_t Amfnr_PreProcess_V1(Amfnr_Context_V1_t *pAmfnrCtx)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    //need todo what?

    Amfnr_IQParaUpdate_V1(pAmfnrCtx);

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return AMFNR_RET_V1_SUCCESS;
}


Amfnr_Result_V1_t Amfnr_GainRatioProcess_V1(Amfnr_GainState_t *pGainState, Amfnr_ExpInfo_V1_t *pExpInfo)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    if(pGainState == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AMFNR_RET_V1_INVALID_PARM;
    }

    if(pExpInfo == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AMFNR_RET_V1_INVALID_PARM;
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

    return AMFNR_RET_V1_SUCCESS;
}


//anr process
Amfnr_Result_V1_t Amfnr_Process_V1(Amfnr_Context_V1_t *pAmfnrCtx, Amfnr_ExpInfo_V1_t *pExpInfo)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    Amfnr_ParamMode_V1_t mode = AMFNR_PARAM_MODE_V1_INVALID;

    if(pAmfnrCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AMFNR_RET_V1_INVALID_PARM;
    }

    if(pExpInfo == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AMFNR_RET_V1_INVALID_PARM;
    }

    if(pAmfnrCtx->eState != AMFNR_STATE_V1_RUNNING) {
        return AMFNR_RET_V1_SUCCESS;
    }

    Amfnr_GainRatioProcess_V1(&pAmfnrCtx->stGainState, pExpInfo);
    Amfnr_ParamModeProcess_V1(pAmfnrCtx, pExpInfo, &mode);

    pExpInfo->mfnr_mode_3to1 = pAmfnrCtx->mfnr_mode_3to1;
    if(pExpInfo->mfnr_mode_3to1 ) {
        pExpInfo->snr_mode = pExpInfo->pre_snr_mode;
    } else {
        pExpInfo->snr_mode = pExpInfo->cur_snr_mode;
    }

    if(pAmfnrCtx->eMode == AMFNR_OP_MODE_V1_AUTO) {

        LOGD_ANR("%s(%d): \n", __FUNCTION__, __LINE__);

#if AMFNR_USE_XML_FILE_V1
        if(pExpInfo->snr_mode != pAmfnrCtx->stExpInfo.snr_mode || pAmfnrCtx->eParamMode != mode) {
            LOGD_ANR("param mode:%d snr_mode:%d\n", mode, pExpInfo->snr_mode);
            pAmfnrCtx->eParamMode = mode;
            Amfnr_ConfigSettingParam_V1(pAmfnrCtx, pAmfnrCtx->eParamMode, pExpInfo->snr_mode);
        }
#endif

        //select param
        select_mfnr_params_by_ISO_v1(&pAmfnrCtx->stAuto.stParams, &pAmfnrCtx->stAuto.stSelect, pExpInfo, pAmfnrCtx->refYuvBit);
        mfnr_dynamic_calc_v1(&pAmfnrCtx->stAuto.stMfnr_dynamic, pExpInfo);
    } else if(pAmfnrCtx->eMode == AMFNR_OP_MODE_V1_MANUAL) {
        //TODO
    }

    memcpy(&pAmfnrCtx->stExpInfo, pExpInfo, sizeof(Amfnr_ExpInfo_V1_t));

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return AMFNR_RET_V1_SUCCESS;

}


//anr get result
Amfnr_Result_V1_t Amfnr_GetProcResult_V1(Amfnr_Context_V1_t *pAmfnrCtx, Amfnr_ProcResult_V1_t* pAmfnrResult)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    if(pAmfnrCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AMFNR_RET_V1_INVALID_PARM;
    }

    if(pAmfnrResult == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AMFNR_RET_V1_INVALID_PARM;
    }

    if(pAmfnrCtx->eMode == AMFNR_OP_MODE_V1_AUTO) {
        pAmfnrResult->stSelect = pAmfnrCtx->stAuto.stSelect;
        pAmfnrResult->mfnrEn = pAmfnrCtx->stAuto.mfnrEn;
        if(pAmfnrCtx->stAuto.mfnrEn && pAmfnrCtx->stAuto.stMfnr_dynamic.enable) {
            pAmfnrResult->mfnrEn = pAmfnrCtx->stAuto.stMfnr_dynamic.mfnr_enable_state;
        }
    } else if(pAmfnrCtx->eMode == AMFNR_OP_MODE_V1_MANUAL) {
        //TODO
        pAmfnrResult->stSelect = pAmfnrCtx->stManual.stSelect;
        pAmfnrResult->mfnrEn = pAmfnrCtx->stManual.mfnrEn;
        pAmfnrCtx->fLuma_TF_Strength = 1.0;
        pAmfnrCtx->fChroma_TF_Strength = 1.0;
    }

    if(pAmfnrCtx->isGrayMode) {
        LOGD_ANR("anr: set gray mode!\n");
        for(int i = 0; i < MFNR_MAX_LVL_UV; i++) {
            pAmfnrResult->stSelect.weight_limit_uv[i] = MFNR_MAX_WEIGHT_LIMIT_UV;
        }
    }

    //transfer to reg value
    mfnr_fix_transfer_v1(&pAmfnrResult->stSelect, &pAmfnrResult->stFix, &pAmfnrCtx->stExpInfo, pAmfnrCtx->stGainState.ratio, pAmfnrCtx->fLuma_TF_Strength,  pAmfnrCtx->fChroma_TF_Strength);
    pAmfnrResult->stFix.tnr_en = pAmfnrResult->mfnrEn;

    //set local gain & 3to1 mode
    int local_gain_en = 1;
#if(AMFNR_USE_JSON_PARA_V1)
    pAmfnrResult->stFix.mode = pAmfnrCtx->mfnr_v1.TuningPara.mode_3to1;
    local_gain_en = pAmfnrCtx->mfnr_v1.TuningPara.local_gain_en;
#else
    pAmfnrResult->stFix.mode = pAmfnrCtx->stMfnrCalib.mode_3to1;
    local_gain_en = pAmfnrCtx->stMfnrCalib.local_gain_en;
#endif
    if(local_gain_en) {
        pAmfnrResult->stFix.gain_en = 0;
    } else {
        pAmfnrResult->stFix.gain_en = 1;
    }


    //motion detect
#if AMFNR_USE_JSON_PARA_V1
    pAmfnrResult->stMotion = pAmfnrCtx->stMotion;
#else
    int mode_idx = 0;
    if(pAmfnrCtx->eParamMode == ANR_PARAM_MODE_NORMAL) {
        mfnr_get_mode_cell_idx_by_name(&pAmfnrCtx->stMfnrCalib, "normal", &mode_idx);
    } else if(pAmfnrCtx->eParamMode == ANR_PARAM_MODE_HDR) {
        mfnr_get_mode_cell_idx_by_name(&pAmfnrCtx->stMfnrCalib, "hdr", &mode_idx);
    } else if(pAmfnrCtx->eParamMode == ANR_PARAM_MODE_GRAY) {
        mfnr_get_mode_cell_idx_by_name(&pAmfnrCtx->stMfnrCalib, "gray", &mode_idx);
    } else {
        LOGE_ANR("%s(%d): not support param mode!\n", __FUNCTION__, __LINE__);
    }
    pAmfnrResult->stMotion = pAmfnrCtx->stMfnrCalib.mode_cell[mode_idx].motion;
#endif

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return AMFNR_RET_V1_SUCCESS;
}

Amfnr_Result_V1_t Amfnr_ConfigSettingParam_V1(Amfnr_Context_V1_t *pAmfnrCtx, Amfnr_ParamMode_V1_t eParamMode, int snr_mode)
{
    char snr_name[CALIBDB_NR_SHARP_NAME_LENGTH];
    char param_mode_name[CALIBDB_MAX_MODE_NAME_LENGTH];
    memset(param_mode_name, 0x00, sizeof(param_mode_name));
    memset(snr_name, 0x00, sizeof(snr_name));

    if(pAmfnrCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AMFNR_RET_V1_INVALID_PARM;
    }

    //select param mode first
    if(eParamMode == AMFNR_PARAM_MODE_V1_NORMAL) {
        sprintf(param_mode_name, "%s", "normal");
    } else if(eParamMode == AMFNR_PARAM_MODE_V1_HDR) {
        sprintf(param_mode_name, "%s", "hdr");
    } else if(eParamMode == AMFNR_PARAM_MODE_V1_GRAY) {
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

#if AMFNR_USE_JSON_PARA_V1
    pAmfnrCtx->stAuto.mfnrEn = pAmfnrCtx->mfnr_v1.TuningPara.enable;
    mfnr_config_setting_param_json_v1(&pAmfnrCtx->stAuto.stParams, &pAmfnrCtx->mfnr_v1, param_mode_name, snr_name);
    mfnr_config_dynamic_param_json_v1(&pAmfnrCtx->stAuto.stMfnr_dynamic, &pAmfnrCtx->mfnr_v1, param_mode_name);
    mfnr_config_motion_param_json_v1(&pAmfnrCtx->stMotion, &pAmfnrCtx->mfnr_v1, param_mode_name);
#else
    pAmfnrCtx->stAuto.mfnrEn = pAmfnrCtx->stMfnrCalib.enable;
    mfnr_config_setting_param_v1(&pAmfnrCtx->stAuto.stParams, &pAmfnrCtx->stMfnrCalib, param_mode_name, snr_name);
    mfnr_config_dynamic_param(&pAmfnrCtx->stAuto.stMfnr_dynamic, &pAmfnrCtx->stMfnrCalib, param_mode_name);
#endif

    return AMFNR_RET_V1_SUCCESS;
}

Amfnr_Result_V1_t Amfnr_ParamModeProcess_V1(Amfnr_Context_V1_t *pAmfnrCtx, Amfnr_ExpInfo_V1_t *pExpInfo, Amfnr_ParamMode_V1_t *mode) {
    Amfnr_Result_V1_t res  = AMFNR_RET_V1_SUCCESS;

    if(pAmfnrCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AMFNR_RET_V1_INVALID_PARM;
    }

    *mode = pAmfnrCtx->eParamMode;

    if(pAmfnrCtx->isGrayMode) {
        *mode = AMFNR_PARAM_MODE_V1_GRAY;
    } else if(pExpInfo->hdr_mode == 0) {
        *mode = AMFNR_PARAM_MODE_V1_NORMAL;
    } else if(pExpInfo->hdr_mode >= 1) {
        *mode = AMFNR_PARAM_MODE_V1_HDR;
    } else {
        *mode = AMFNR_PARAM_MODE_V1_NORMAL;
    }

    return res;
}


RKAIQ_END_DECLARE


