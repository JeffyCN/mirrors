
#include "rk_aiq_again_algo_v2.h"
#include "rk_aiq_again_algo_itf_v2.h"

RKAIQ_BEGIN_DECLARE

Again_result_V2_t Again_Start_V2(Again_Context_V2_t *pAgainCtx)
{
    LOGI_ANR( "%s:enter!\n", __FUNCTION__);

    // initial checks
    if (pAgainCtx == NULL) {
        return (AGAINV2_RET_NULL_POINTER);
    }

    if ((AGAINV2_STATE_RUNNING == pAgainCtx->eState)
            || (AGAINV2_STATE_LOCKED == pAgainCtx->eState)) {
        return (AGAINV2_RET_FAILURE);
    }

    pAgainCtx->eState = AGAINV2_STATE_RUNNING;

    LOGI_ANR( "%s:exit!\n", __FUNCTION__);
    return (AGAINV2_RET_SUCCESS);
}


Again_result_V2_t Again_Stop_V2(Again_Context_V2_t *pAgainCtx)
{
    LOGI_ANR( "%s:enter!\n", __FUNCTION__);

    // initial checks
    if (pAgainCtx == NULL) {
        return (AGAINV2_RET_NULL_POINTER);
    }

    if (AGAINV2_STATE_LOCKED == pAgainCtx->eState) {
        return (AGAINV2_RET_FAILURE);
    }

    pAgainCtx->eState = AGAINV2_STATE_STOPPED;

    LOGI_ANR( "%s:exit!\n", __FUNCTION__);
    return (AGAINV2_RET_SUCCESS);
}

//anr inint
Again_result_V2_t Again_Init_V2(Again_Context_V2_t **ppAgainCtx, CamCalibDbV2Context_t *pCalibDbV2)
{
    Again_Context_V2_t * pAgainCtx;

    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    pAgainCtx = (Again_Context_V2_t *)malloc(sizeof(Again_Context_V2_t));
    if(pAgainCtx == NULL) {
        LOGE_ANR("%s(%d): malloc fail\n", __FUNCTION__, __LINE__);
        return AGAINV2_RET_NULL_POINTER;
    }

    memset(pAgainCtx, 0x00, sizeof(Again_Context_V2_t));

    //gain state init
    pAgainCtx->stGainState.gain_stat_full_last = -1;
    pAgainCtx->stGainState.gainState = -1;
    pAgainCtx->stGainState.gainState_last = -1;
    pAgainCtx->stGainState.gain_th0[0]    = 2.0;
    pAgainCtx->stGainState.gain_th1[0]    = 4.0;
    pAgainCtx->stGainState.gain_th0[1]    = 32.0;
    pAgainCtx->stGainState.gain_th1[1]    = 64.0;


    pAgainCtx->eState = AGAINV2_STATE_INITIALIZED;
    *ppAgainCtx = pAgainCtx;

    pAgainCtx->eMode = AGAINV2_OP_MODE_AUTO;
    pAgainCtx->isIQParaUpdate = false;
    pAgainCtx->isGrayMode = false;
    pAgainCtx->isReCalculate = 1;
#if AGAIN_USE_JSON_FILE_V2

    CalibDbV2_GainV2_t * pcalibdbV2_gain_v2 =
        (CalibDbV2_GainV2_t *)(CALIBDBV2_GET_MODULE_PTR((CamCalibDbV2Context_t*)pCalibDbV2, gain_v2));

    pAgainCtx->gain_v2 = *pcalibdbV2_gain_v2;


    pAgainCtx->stExpInfo.snr_mode = 1;
    pAgainCtx->eParamMode = AGAINV2_PARAM_MODE_NORMAL;
    Again_ConfigSettingParam_V2(pAgainCtx, pAgainCtx->eParamMode, pAgainCtx->stExpInfo.snr_mode);
    // init manual params
    pAgainCtx->stManual.stSelect = pAgainCtx->stAuto.stParams.iso_params[0];
#endif

    pAgainCtx->wrt2ddr.need2wrt = false;
    pAgainCtx->wrt2ddr.buf_cnt = 1;
    pAgainCtx->wrt2ddr.again2ddr_mode = 0;

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return AGAINV2_RET_SUCCESS;
}


//anr release
Again_result_V2_t Again_Release_V2(Again_Context_V2_t *pAgainCtx)
{
    Again_result_V2_t result = AGAINV2_RET_SUCCESS;
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    if(pAgainCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AGAINV2_RET_NULL_POINTER;
    }

    result = Again_Stop_V2(pAgainCtx);
    if (result != AGAINV2_RET_SUCCESS) {
        LOGE_ANR( "%s: ANRStop() failed!\n", __FUNCTION__);
        return (result);
    }

    // check state
    if ((AGAINV2_STATE_RUNNING == pAgainCtx->eState)
            || (AGAINV2_STATE_LOCKED == pAgainCtx->eState)) {
        return (AGAINV2_RET_BUSY);
    }

    memset(pAgainCtx, 0x00, sizeof(Again_Context_V2_t));
    free(pAgainCtx);

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return AGAINV2_RET_SUCCESS;
}

//anr config
Again_result_V2_t Again_Prepare_V2(Again_Context_V2_t *pAgainCtx, Again_Config_V2_t* pAgainConfig)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    if(pAgainCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AGAINV2_RET_INVALID_PARM;
    }

    if(pAgainConfig == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AGAINV2_RET_INVALID_PARM;
    }

    Again_Start_V2(pAgainCtx);

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return AGAINV2_RET_SUCCESS;
}

//anr reconfig
Again_result_V2_t Again_ReConfig_V2(Again_Context_V2_t *pAgainCtx, Again_Config_V2_t* pAgainConfig)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    //need todo what?

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return AGAINV2_RET_SUCCESS;
}


//anr preprocess
Again_result_V2_t Again_PreProcess_V2(Again_Context_V2_t *pAgainCtx)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    //need todo what?

    if(pAgainCtx->isIQParaUpdate) {
        Again_ConfigSettingParam_V2(pAgainCtx, pAgainCtx->eParamMode, pAgainCtx->stExpInfo.snr_mode);
        pAgainCtx->isIQParaUpdate = false;
    }

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return AGAINV2_RET_SUCCESS;
}

Again_result_V2_t Again_GainRatioProcess_V2(Again_GainState_V2_t *pGainState, Again_ExpInfo_V2_t *pExpInfo)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    if(pGainState == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AGAINV2_RET_INVALID_PARM;
    }

    if(pExpInfo == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AGAINV2_RET_INVALID_PARM;
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

    return AGAINV2_RET_SUCCESS;
}


//anr process
Again_result_V2_t Again_Process_V2(Again_Context_V2_t *pAgainCtx, Again_ExpInfo_V2_t *pExpInfo)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    Again_ParamMode_V2_t mode = AGAINV2_PARAM_MODE_INVALID;

    if(pAgainCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AGAINV2_RET_INVALID_PARM;
    }

    if(pExpInfo == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AGAINV2_RET_INVALID_PARM;
    }

    if(pAgainCtx->eState != AGAINV2_STATE_RUNNING) {
        return AGAINV2_RET_SUCCESS;
    }

    //Again_GainRatioProcess_V2(&pAgainCtx->stGainState, pExpInfo);

    if(pAgainCtx->eMode == AGAINV2_OP_MODE_AUTO) {


#if AGAIN_USE_JSON_FILE_V2
        if(pExpInfo->snr_mode != pAgainCtx->stExpInfo.snr_mode || pAgainCtx->eParamMode != mode) {
            LOGD_ANR("param mode:%d snr_mode:%d\n", mode, pExpInfo->snr_mode);
            if (pAgainCtx->isGrayMode) {
                mode = AGAINV2_PARAM_MODE_GRAY;
            } else if (pExpInfo->hdr_mode == 0) {
                mode = AGAINV2_PARAM_MODE_NORMAL;
            } else if (pExpInfo->hdr_mode >= 1) {
                mode = AGAINV2_PARAM_MODE_HDR;
            } else {
                mode = AGAINV2_PARAM_MODE_NORMAL;
            }
            pAgainCtx->eParamMode = mode;
            Again_ConfigSettingParam_V2(pAgainCtx, pAgainCtx->eParamMode, pExpInfo->snr_mode);
        }
#endif

        //select param
        gain_select_params_by_ISO_V2(&pAgainCtx->stAuto.stParams, &pAgainCtx->stAuto.stSelect, pExpInfo);

    } else if(pAgainCtx->eMode == AGAINV2_OP_MODE_MANUAL) {
        //TODO
    }

    memcpy(&pAgainCtx->stExpInfo, pExpInfo, sizeof(Again_ExpInfo_V2_t));

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return AGAINV2_RET_SUCCESS;

}


//anr get result
Again_result_V2_t Again_GetProcResult_V2(Again_Context_V2_t *pAgainCtx, Again_ProcResult_V2_t* pAgainResult)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    if(pAgainCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AGAINV2_RET_INVALID_PARM;
    }

    if(pAgainResult == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AGAINV2_RET_INVALID_PARM;
    }

    RK_GAIN_Select_V2_t* stSelect = NULL;
    if(pAgainCtx->eMode == AGAINV2_OP_MODE_AUTO) {
        stSelect = &pAgainCtx->stAuto.stSelect;
    } else if(pAgainCtx->eMode == AGAINV2_OP_MODE_MANUAL) {
        stSelect = &pAgainCtx->stManual.stSelect;
    }

    //transfer to reg value
    gain_fix_transfer_v2(stSelect, pAgainResult->stFix, &pAgainCtx->stExpInfo, pAgainCtx->stGainState.ratio);

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return AGAINV2_RET_SUCCESS;
}


Again_result_V2_t Again_ConfigSettingParam_V2(Again_Context_V2_t *pAgainCtx, Again_ParamMode_V2_t eParamMode, int snr_mode)
{
    char snr_name[CALIBDB_NR_SHARP_NAME_LENGTH];
    char param_mode_name[CALIBDB_MAX_MODE_NAME_LENGTH];
    memset(param_mode_name, 0x00, sizeof(param_mode_name));
    memset(snr_name, 0x00, sizeof(snr_name));

    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    if(pAgainCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AGAINV2_RET_INVALID_PARM;
    }

    //select param mode first
    if(eParamMode == AGAINV2_PARAM_MODE_NORMAL) {
        sprintf(param_mode_name, "%s", "normal");
    } else if(eParamMode == AGAINV2_PARAM_MODE_HDR) {
        sprintf(param_mode_name, "%s", "hdr");
    } else if(eParamMode == AGAINV2_PARAM_MODE_GRAY) {
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
        LOGE_ANR("%s(%d): not support snr mode:%d!\n", __FUNCTION__, __LINE__, snr_mode);
        sprintf(snr_name, "%s", "LSNR");
    }

#if (AGAIN_USE_JSON_FILE_V2)
    gain_config_setting_param_json_V2(&pAgainCtx->stAuto.stParams, &pAgainCtx->gain_v2, param_mode_name, snr_name);
#endif
    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return AGAINV2_RET_SUCCESS;
}


Again_result_V2_t Again_ParamModeProcess_V2(Again_Context_V2_t *pAgainCtx, Again_ExpInfo_V2_t *pExpInfo, Again_ParamMode_V2_t *mode)
{
    Again_result_V2_t res  = AGAINV2_RET_SUCCESS;

    if(pAgainCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AGAINV2_RET_INVALID_PARM;
    }

    *mode = pAgainCtx->eParamMode;

    if(pAgainCtx->isGrayMode) {
        *mode = AGAINV2_PARAM_MODE_GRAY;
    } else if(pExpInfo->hdr_mode == 0) {
        *mode = AGAINV2_PARAM_MODE_NORMAL;
    } else if(pExpInfo->hdr_mode >= 1) {
        *mode = AGAINV2_PARAM_MODE_HDR;
    } else {
        *mode = AGAINV2_PARAM_MODE_NORMAL;
    }

    return res;
}

RKAIQ_END_DECLARE


