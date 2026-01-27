
#include "rk_aiq_again_algo.h"
#include "rk_aiq_again_algo_itf.h"

RKAIQ_BEGIN_DECLARE

Again_result_t Again_Start_V1(Again_Context_V1_t *pAgainCtx) 
{
    LOGI_ANR( "%s:enter!\n", __FUNCTION__);

    // initial checks
    if (pAgainCtx == NULL) {
        return (AGAIN_RET_NULL_POINTER);
    }

    if ((AGAIN_STATE_RUNNING == pAgainCtx->eState)
            || (AGAIN_STATE_LOCKED == pAgainCtx->eState)) {
        return (AGAIN_RET_FAILURE);
    }

    pAgainCtx->eState = AGAIN_STATE_RUNNING;

    LOGI_ANR( "%s:exit!\n", __FUNCTION__);
    return (AGAIN_RET_SUCCESS);
}


Again_result_t Again_Stop_V1(Again_Context_V1_t *pAgainCtx) 
{
    LOGI_ANR( "%s:enter!\n", __FUNCTION__);

    // initial checks
    if (pAgainCtx == NULL) {
        return (AGAIN_RET_NULL_POINTER);
    }

    if (AGAIN_STATE_LOCKED == pAgainCtx->eState) {
        return (AGAIN_RET_FAILURE);
    }

    pAgainCtx->eState = AGAIN_STATE_STOPPED;

    LOGI_ANR( "%s:exit!\n", __FUNCTION__);
    return (AGAIN_RET_SUCCESS);
}


//anr inint
Again_result_t Again_Init_V1(Again_Context_V1_t **ppAgainCtx, CamCalibDbContext_t *pCalibDb)
{
    Again_Context_V1_t * pAgainCtx;

    LOGE_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    pAgainCtx = (Again_Context_V1_t *)malloc(sizeof(Again_Context_V1_t));
    if(pAgainCtx == NULL) {
        LOGE_ANR("%s(%d): malloc fail\n", __FUNCTION__, __LINE__);
        return AGAIN_RET_NULL_POINTER;
    }

    memset(pAgainCtx, 0x00, sizeof(Again_Context_V1_t));
	
    //gain state init
    pAgainCtx->stGainState.gain_stat_full_last = -1;
    pAgainCtx->stGainState.gainState = -1;
    pAgainCtx->stGainState.gainState_last = -1;
    pAgainCtx->stGainState.gain_th0[0]    = 2.0;
    pAgainCtx->stGainState.gain_th1[0]    = 4.0;
    pAgainCtx->stGainState.gain_th0[1]    = 32.0;
    pAgainCtx->stGainState.gain_th1[1]    = 64.0;
	
    pAgainCtx->eState = AGAIN_STATE_INITIALIZED;
    *ppAgainCtx = pAgainCtx;

    pAgainCtx->eMode = AGAIN_OP_MODE_AUTO;
	pAgainCtx->isIQParaUpdate = false;
	pAgainCtx->isGrayMode = false;


#if AGAIN_USE_XML_FILE_V1
	CalibDb_MFNR_2_t *pMfnrCalib=
        (CalibDb_MFNR_2_t*)(CALIBDB_GET_MODULE_PTR((void*)pCalibDb, mfnr));
	pAgainCtx->mfnr_mode_3to1 = pMfnrCalib->mode_3to1;
	pAgainCtx->mfnr_local_gain_en = pMfnrCalib->local_gain_en;
#endif


    LOGE_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return AGAIN_RET_SUCCESS;
}



//anr inint
Again_result_t Again_Init_Json_V1(Again_Context_V1_t **ppAgainCtx, CamCalibDbV2Context_t *pCalibDbV2)
{
    Again_Context_V1_t * pAgainCtx;

    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    pAgainCtx = (Again_Context_V1_t *)malloc(sizeof(Again_Context_V1_t));
    if(pAgainCtx == NULL) {
        LOGE_ANR("%s(%d): malloc fail\n", __FUNCTION__, __LINE__);
        return AGAIN_RET_NULL_POINTER;
    }

    memset(pAgainCtx, 0x00, sizeof(Again_Context_V1_t));
	
    //gain state init
    pAgainCtx->stGainState.gain_stat_full_last = -1;
    pAgainCtx->stGainState.gainState = -1;
    pAgainCtx->stGainState.gainState_last = -1;
    pAgainCtx->stGainState.gain_th0[0]    = 2.0;
    pAgainCtx->stGainState.gain_th1[0]    = 4.0;
    pAgainCtx->stGainState.gain_th0[1]    = 32.0;
    pAgainCtx->stGainState.gain_th1[1]    = 64.0;


    pAgainCtx->eState = AGAIN_STATE_INITIALIZED;
    *ppAgainCtx = pAgainCtx;

    pAgainCtx->eMode = AGAIN_OP_MODE_AUTO;
	pAgainCtx->isIQParaUpdate = false;
	pAgainCtx->isGrayMode = false;

#if AGAIN_USE_JSON_FILE_V1
	
    CalibDbV2_MFNR_t* pCalibv2_mfnr_v1 =
            (CalibDbV2_MFNR_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibDbV2, mfnr_v1));
	pAgainCtx->mfnr_mode_3to1 = pCalibv2_mfnr_v1->TuningPara.mode_3to1;
	pAgainCtx->mfnr_local_gain_en = pCalibv2_mfnr_v1->TuningPara.local_gain_en;

#endif


    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return AGAIN_RET_SUCCESS;
}


//anr release
Again_result_t Again_Release_V1(Again_Context_V1_t *pAgainCtx)
{
	Again_result_t result = AGAIN_RET_SUCCESS;
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    if(pAgainCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AGAIN_RET_NULL_POINTER;
    }

	result = Again_Stop_V1(pAgainCtx);
    if (result != AGAIN_RET_SUCCESS) {
        LOGE_ANR( "%s: ANRStop() failed!\n", __FUNCTION__);
        return (result);
    }

    // check state
    if ((AGAIN_STATE_RUNNING == pAgainCtx->eState)
            || (AGAIN_STATE_LOCKED == pAgainCtx->eState)) {
        return (AGAIN_RET_BUSY);
    }
	
    memset(pAgainCtx, 0x00, sizeof(Again_Context_V1_t));
    free(pAgainCtx);

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return AGAIN_RET_SUCCESS;
}

//anr config
Again_result_t Again_Prepare_V1(Again_Context_V1_t *pAgainCtx, Again_Config_V1_t* pAgainConfig)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    if(pAgainCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AGAIN_RET_INVALID_PARM;
    }

    if(pAgainConfig == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AGAIN_RET_INVALID_PARM;
    }

    Again_Start_V1(pAgainCtx);

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return AGAIN_RET_SUCCESS;
}

//anr reconfig
Again_result_t Again_ReConfig_V1(Again_Context_V1_t *pAgainCtx, Again_Config_V1_t* pAgainConfig)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    //need todo what?

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return AGAIN_RET_SUCCESS;
}


//anr preprocess
Again_result_t Again_PreProcess_V1(Again_Context_V1_t *pAgainCtx)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    //need todo what?
	
    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return AGAIN_RET_SUCCESS;
}

Again_result_t Again_GainRatioProcess_V1(Again_GainState_t *pGainState, Again_ExpInfo_t *pExpInfo)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    if(pGainState == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AGAIN_RET_INVALID_PARM;
    }

    if(pExpInfo == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AGAIN_RET_INVALID_PARM;
    }

    float gain_cur = pExpInfo->arAGain[pExpInfo->hdr_mode] * pExpInfo->arDGain[pExpInfo->hdr_mode];
    float th[2];
    float gain_th0[2];
    float gain_th1[2];
    for(int i = 0; i < 2; i++){
        gain_th0[i]     = pGainState->gain_th0[i];
        gain_th1[i]     = pGainState->gain_th1[i];
        th[i]           = pow(2.0, (log2(gain_th0[i])+log2(gain_th1[i])) / 2);
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
    if (gain_stat ==0)
        pGainState->ratio = 16;
    else if (gain_stat == 1)
        pGainState->ratio = 1;
    else
        pGainState->ratio = 1.0/16.0;

	pGainState->gain_stat_full_last 	= gain_stat_full_last;
	pGainState->gainState 		= gain_stat;
	pGainState->gainState_last 	= gain_stat_last;

    LOGD_ANR("%s:%d gain_cur:%f gain th %f %fd %f %f ratio:%f gain_state:%d %d full    %d %d\n",
             __FUNCTION__, __LINE__,
             gain_cur, 
             gain_th0[0],gain_th0[1],
             gain_th1[0],gain_th1[1],
             pGainState->ratio,
             pGainState->gainState_last,
             pGainState->gainState,
             pGainState->gain_stat_full_last,
             gain_stat_full);
    

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);

    return AGAIN_RET_SUCCESS;
}


//anr process
Again_result_t Again_Process_V1(Again_Context_V1_t *pAgainCtx, Again_ExpInfo_t *pExpInfo)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
	Again_ParamMode_t mode = AGAIN_PARAM_MODE_INVALID;
	
    if(pAgainCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AGAIN_RET_INVALID_PARM;
    }

    if(pExpInfo == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AGAIN_RET_INVALID_PARM;
    }

	if(pAgainCtx->eState != AGAIN_STATE_RUNNING){
		return AGAIN_RET_SUCCESS;
	}
	
	Again_GainRatioProcess_V1(&pAgainCtx->stGainState, pExpInfo);
	
    if(pAgainCtx->eMode == AGAIN_OP_MODE_AUTO) {

        LOGD_ANR("%s(%d): \n", __FUNCTION__, __LINE__);        
        //get param from mfnr

    } else if(pAgainCtx->eMode == AGAIN_OP_MODE_MANUAL) {
        //TODO
    }

    memcpy(&pAgainCtx->stExpInfo, pExpInfo, sizeof(Again_ExpInfo_t));

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return AGAIN_RET_SUCCESS;

}


//anr get result
Again_result_t Again_GetProcResult_V1(Again_Context_V1_t *pAgainCtx, Again_ProcResult_V1_t* pAgainResult)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    if(pAgainCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AGAIN_RET_INVALID_PARM;
    }

    if(pAgainResult == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AGAIN_RET_INVALID_PARM;
    }

    RK_GAIN_Params_V1_Select_t* stSelect = NULL;
    if(pAgainCtx->eMode == AGAIN_OP_MODE_AUTO) {
        stSelect = &pAgainCtx->stAuto.stSelect;
    } else if(pAgainCtx->eMode == AGAIN_OP_MODE_MANUAL) {
        stSelect= &pAgainCtx->stManual.stSelect;
    }
		
    //transfer to reg value
	gain_fix_transfer_v1(stSelect, pAgainResult->stFix, &pAgainCtx->stExpInfo, pAgainCtx->stGainState.ratio);
	pAgainResult->stFix->gain_table_en = pAgainCtx->mfnr_local_gain_en;
	
    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return AGAIN_RET_SUCCESS;
}




RKAIQ_END_DECLARE


