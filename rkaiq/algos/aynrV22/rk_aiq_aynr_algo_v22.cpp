
#include "rk_aiq_aynr_algo_v22.h"
#include "rk_aiq_aynr_algo_itf_v22.h"

RKAIQ_BEGIN_DECLARE

Aynr_result_V22_t Aynr_Start_V22(Aynr_Context_V22_t *pAynrCtx)
{
    LOGI_ANR( "%s:enter!\n", __FUNCTION__);

    // initial checks
    if (pAynrCtx == NULL) {
        return (AYNRV22_RET_NULL_POINTER);
    }

    if ((AYNRV22_STATE_RUNNING == pAynrCtx->eState)
            || (AYNRV22_STATE_LOCKED == pAynrCtx->eState)) {
        return (AYNRV22_RET_FAILURE);
    }

    pAynrCtx->eState = AYNRV22_STATE_RUNNING;

    LOGI_ANR( "%s:exit!\n", __FUNCTION__);
    return (AYNRV22_RET_SUCCESS);
}


Aynr_result_V22_t Aynr_Stop_V22(Aynr_Context_V22_t *pAynrCtx)
{
    LOGI_ANR( "%s:enter!\n", __FUNCTION__);

    // initial checks
    if (pAynrCtx == NULL) {
        return (AYNRV22_RET_NULL_POINTER);
    }

    if (AYNRV22_STATE_LOCKED == pAynrCtx->eState) {
        return (AYNRV22_RET_FAILURE);
    }

    pAynrCtx->eState = AYNRV22_STATE_STOPPED;

    LOGI_ANR( "%s:exit!\n", __FUNCTION__);
    return (AYNRV22_RET_SUCCESS);
}


//anr inint
Aynr_result_V22_t Aynr_Init_V22(Aynr_Context_V22_t **ppAynrCtx, void *pCalibDb)
{
    Aynr_Context_V22_t * pAynrCtx;

    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    pAynrCtx = (Aynr_Context_V22_t *)malloc(sizeof(Aynr_Context_V22_t));
    if(pAynrCtx == NULL) {
        LOGE_ANR("%s(%d): malloc fail\n", __FUNCTION__, __LINE__);
        return AYNRV22_RET_NULL_POINTER;
    }

    memset(pAynrCtx, 0x00, sizeof(Aynr_Context_V22_t));

    //gain state init
    pAynrCtx->stStrength.strength_enable = true;
    pAynrCtx->stStrength.percent = 1.0;

    pAynrCtx->eState = AYNRV22_STATE_INITIALIZED;
    *ppAynrCtx = pAynrCtx;

    pAynrCtx->eMode = AYNRV22_OP_MODE_AUTO;
    pAynrCtx->isIQParaUpdate = false;
    pAynrCtx->isGrayMode = false;
    pAynrCtx->isReCalculate = 1;

#if AYNR_USE_XML_FILE_V22
    //read v1 params from xml
#if AYNR_USE_JSON_FILE_V22
    pAynrCtx->ynr_v22 =
        *(CalibDbV2_YnrV22_t *)(CALIBDBV2_GET_MODULE_PTR(pCalibDb, ynr_v22));
#endif
#endif


#if AYNR_USE_XML_FILE_V22
    pAynrCtx->stExpInfo.snr_mode = 1;
    pAynrCtx->eParamMode = AYNRV22_PARAM_MODE_NORMAL;
    Aynr_ConfigSettingParam_V22(pAynrCtx, pAynrCtx->eParamMode, pAynrCtx->stExpInfo.snr_mode);
    // init manual params
    pAynrCtx->stManual.stSelect = pAynrCtx->stAuto.stParams.arYnrParamsISO[0];
#endif

    LOGD_ANR("%s(%d):", __FUNCTION__, __LINE__);


    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return AYNRV22_RET_SUCCESS;
}

//anr release
Aynr_result_V22_t Aynr_Release_V22(Aynr_Context_V22_t *pAynrCtx)
{
    Aynr_result_V22_t result = AYNRV22_RET_SUCCESS;
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    if(pAynrCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNRV22_RET_NULL_POINTER;
    }

    result = Aynr_Stop_V22(pAynrCtx);
    if (result != AYNRV22_RET_SUCCESS) {
        LOGE_ANR( "%s: ANRStop() failed!\n", __FUNCTION__);
        return (result);
    }

    // check state
    if ((AYNRV22_STATE_RUNNING == pAynrCtx->eState)
            || (AYNRV22_STATE_LOCKED == pAynrCtx->eState)) {
        return (AYNRV22_RET_BUSY);
    }

    memset(pAynrCtx, 0x00, sizeof(Aynr_Context_V22_t));
    free(pAynrCtx);

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return AYNRV22_RET_SUCCESS;
}

//anr config
Aynr_result_V22_t Aynr_Prepare_V22(Aynr_Context_V22_t *pAynrCtx, Aynr_Config_V22_t* pAynrConfig)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    if(pAynrCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNRV22_RET_INVALID_PARM;
    }

    if(pAynrConfig == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNRV22_RET_INVALID_PARM;
    }

    if(!!(pAynrCtx->prepare_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB)) {
        Aynr_IQParaUpdate_V22(pAynrCtx);
    }

    pAynrCtx->rawWidth = pAynrConfig->rawWidth;
    pAynrCtx->rawHeight = pAynrConfig->rawHeight;
    Aynr_Start_V22(pAynrCtx);

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return AYNRV22_RET_SUCCESS;
}

//anr reconfig
Aynr_result_V22_t Aynr_ReConfig_V22(Aynr_Context_V22_t *pAynrCtx, Aynr_Config_V22_t* pAynrConfig)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    //need todo what?

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return AYNRV22_RET_SUCCESS;
}

//anr reconfig
Aynr_result_V22_t Aynr_IQParaUpdate_V22(Aynr_Context_V22_t *pAynrCtx)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    //need todo what?

    if(pAynrCtx->isIQParaUpdate) {
        LOGD_ANR("IQ data reconfig\n");
        Aynr_ConfigSettingParam_V22(pAynrCtx, pAynrCtx->eParamMode, pAynrCtx->stExpInfo.snr_mode);
        pAynrCtx->isIQParaUpdate = false;
    }

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return AYNRV22_RET_SUCCESS;
}


//anr preprocess
Aynr_result_V22_t Aynr_PreProcess_V22(Aynr_Context_V22_t *pAynrCtx)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    //need todo what?

    Aynr_IQParaUpdate_V22(pAynrCtx);

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return AYNRV22_RET_SUCCESS;
}

//anr process
Aynr_result_V22_t Aynr_Process_V22(Aynr_Context_V22_t *pAynrCtx, Aynr_ExpInfo_V22_t *pExpInfo)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    Aynr_ParamMode_V22_t mode = AYNRV22_PARAM_MODE_INVALID;

    if(pAynrCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNRV22_RET_INVALID_PARM;
    }

    if(pExpInfo == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNRV22_RET_INVALID_PARM;
    }

    if(pAynrCtx->eState != AYNRV22_STATE_RUNNING) {
        return AYNRV22_RET_SUCCESS;
    }


    Aynr_ParamModeProcess_V22(pAynrCtx, pExpInfo, &mode);

    if(pAynrCtx->eMode == AYNRV22_OP_MODE_AUTO) {

        LOGD_ANR("%s(%d): \n", __FUNCTION__, __LINE__);

#if AYNR_USE_XML_FILE_V22
        if(pExpInfo->snr_mode != pAynrCtx->stExpInfo.snr_mode || pAynrCtx->eParamMode != mode) {
            LOGD_ANR("param mode:%d snr_mode:%d\n", mode, pExpInfo->snr_mode);
            pAynrCtx->eParamMode = mode;
            Aynr_ConfigSettingParam_V22(pAynrCtx, pAynrCtx->eParamMode, pExpInfo->snr_mode);
        }
#endif

        //select param
        ynr_select_params_by_ISO_V22(&pAynrCtx->stAuto.stParams, &pAynrCtx->stAuto.stSelect, pExpInfo);

    } else if(pAynrCtx->eMode == AYNRV22_OP_MODE_MANUAL) {
        //TODO
    }

    memcpy(&pAynrCtx->stExpInfo, pExpInfo, sizeof(Aynr_ExpInfo_V22_t));
    pAynrCtx->stExpInfo.rawHeight = pAynrCtx->rawHeight;
    pAynrCtx->stExpInfo.rawWidth = pAynrCtx->rawWidth;

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return AYNRV22_RET_SUCCESS;

}



//anr get result
Aynr_result_V22_t Aynr_GetProcResult_V22(Aynr_Context_V22_t *pAynrCtx, Aynr_ProcResult_V22_t* pAynrResult)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    if(pAynrCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNRV22_RET_INVALID_PARM;
    }

    if(pAynrResult == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNRV22_RET_INVALID_PARM;
    }

    if(pAynrCtx->eMode == AYNRV22_OP_MODE_AUTO) {
        pAynrResult->stSelect = &pAynrCtx->stAuto.stSelect;
    } else if(pAynrCtx->eMode == AYNRV22_OP_MODE_MANUAL) {
        pAynrResult->stSelect = &pAynrCtx->stManual.stSelect;
    }

    //transfer to reg value
    ynr_fix_transfer_V22(pAynrResult->stSelect, pAynrResult->stFix, &pAynrCtx->stStrength, &pAynrCtx->stExpInfo);

    if(pAynrCtx->eMode == AYNRV22_OP_MODE_REG_MANUAL) {
        *pAynrResult->stFix = pAynrCtx->stManual.stFix;
        pAynrCtx->stStrength.percent = 1.0;
    }

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return AYNRV22_RET_SUCCESS;
}

Aynr_result_V22_t Aynr_ConfigSettingParam_V22(Aynr_Context_V22_t *pAynrCtx, Aynr_ParamMode_V22_t eParamMode, int snr_mode)
{
    char snr_name[CALIBDB_NR_SHARP_NAME_LENGTH];
    char param_mode_name[CALIBDB_MAX_MODE_NAME_LENGTH];
    memset(param_mode_name, 0x00, sizeof(param_mode_name));
    memset(snr_name, 0x00, sizeof(snr_name));

    if(pAynrCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNRV22_RET_INVALID_PARM;
    }

    //select param mode first
    if(eParamMode == AYNRV22_PARAM_MODE_NORMAL) {
        sprintf(param_mode_name, "%s", "normal");
    } else if(eParamMode == AYNRV22_PARAM_MODE_HDR) {
        sprintf(param_mode_name, "%s", "hdr");
    } else if(eParamMode == AYNRV22_PARAM_MODE_GRAY) {
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

#if AYNR_USE_JSON_FILE_V22
    ynr_config_setting_param_json_V22(&pAynrCtx->stAuto.stParams, &pAynrCtx->ynr_v22, param_mode_name, snr_name);
#endif

    return AYNRV22_RET_SUCCESS;
}

Aynr_result_V22_t Aynr_ParamModeProcess_V22(Aynr_Context_V22_t *pAynrCtx, Aynr_ExpInfo_V22_t *pExpInfo, Aynr_ParamMode_V22_t *mode) {

    if(pAynrCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYNRV22_RET_INVALID_PARM;
    }

    Aynr_result_V22_t res  = AYNRV22_RET_SUCCESS;
    *mode = pAynrCtx->eParamMode;

    if(pAynrCtx->isGrayMode) {
        *mode = AYNRV22_PARAM_MODE_GRAY;
    } else if(pExpInfo->hdr_mode == 0) {
        *mode = AYNRV22_PARAM_MODE_NORMAL;
    } else if(pExpInfo->hdr_mode >= 1) {
        *mode = AYNRV22_PARAM_MODE_HDR;
    } else {
        *mode = AYNRV22_PARAM_MODE_NORMAL;
    }

    return res;
}


RKAIQ_END_DECLARE


