
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

#if 0
    CalibDbV2_MFNR_t* pCalibv2_mfnr_v2 =
        (CalibDbV2_MFNR_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibDbV2, mfnr_v2));
    pAgainCtx->mfnr_mode_3to1 = pCalibv2_mfnr_v2->TuningPara.mode_3to1;
    pAgainCtx->mfnr_local_gain_en = pCalibv2_mfnr_v2->TuningPara.local_gain_en;
#endif

#endif


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

        LOGD_ANR("%s(%d): \n", __FUNCTION__, __LINE__);
        //get param from mfnr

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

    if(pAgainCtx->eMode == AGAINV2_OP_MODE_AUTO) {
        pAgainResult->stSelect = pAgainCtx->stAuto.stSelect;
    } else if(pAgainCtx->eMode == AGAINV2_OP_MODE_MANUAL) {
        pAgainResult->stSelect = pAgainCtx->stManual.stSelect;
    }

    //transfer to reg value
    gain_fix_transfer_v2(&pAgainResult->stSelect, &pAgainResult->stFix, &pAgainCtx->stExpInfo, pAgainCtx->stGainState.ratio);
    //pAgainResult->stFix.gain_table_en = //pAgainCtx->mfnr_local_gain_en;

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return AGAINV2_RET_SUCCESS;
}




RKAIQ_END_DECLARE


