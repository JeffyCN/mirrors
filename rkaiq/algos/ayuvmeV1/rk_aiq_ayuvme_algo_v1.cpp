
#include "rk_aiq_ayuvme_algo_v1.h"
#include "rk_aiq_ayuvme_algo_itf_v1.h"

RKAIQ_BEGIN_DECLARE

Ayuvme_result_V1_t Ayuvme_Start_V1(Ayuvme_Context_V1_t *pAyuvmeCtx)
{
    LOGI_ANR( "%s:enter!\n", __FUNCTION__);

    // initial checks
    if (pAyuvmeCtx == NULL) {
        return (AYUVMEV1_RET_NULL_POINTER);
    }

    if ((AYUVMEV1_STATE_RUNNING == pAyuvmeCtx->eState)
            || (AYUVMEV1_STATE_LOCKED == pAyuvmeCtx->eState)) {
        return (AYUVMEV1_RET_FAILURE);
    }

    pAyuvmeCtx->eState = AYUVMEV1_STATE_RUNNING;

    LOGI_ANR( "%s:exit!\n", __FUNCTION__);
    return (AYUVMEV1_RET_SUCCESS);
}


Ayuvme_result_V1_t Ayuvme_Stop_V1(Ayuvme_Context_V1_t *pAyuvmeCtx)
{
    LOGI_ANR( "%s:enter!\n", __FUNCTION__);

    // initial checks
    if (pAyuvmeCtx == NULL) {
        return (AYUVMEV1_RET_NULL_POINTER);
    }

    if (AYUVMEV1_STATE_LOCKED == pAyuvmeCtx->eState) {
        return (AYUVMEV1_RET_FAILURE);
    }

    pAyuvmeCtx->eState = AYUVMEV1_STATE_STOPPED;

    LOGI_ANR( "%s:exit!\n", __FUNCTION__);
    return (AYUVMEV1_RET_SUCCESS);
}


//anr inint
Ayuvme_result_V1_t Ayuvme_Init_V1(Ayuvme_Context_V1_t **ppAyuvmeCtx, void *pCalibDb)
{
    Ayuvme_Context_V1_t * pAyuvmeCtx;

    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    pAyuvmeCtx = (Ayuvme_Context_V1_t *)malloc(sizeof(Ayuvme_Context_V1_t));
    if(pAyuvmeCtx == NULL) {
        LOGE_ANR("%s(%d): malloc fail\n", __FUNCTION__, __LINE__);
        return AYUVMEV1_RET_NULL_POINTER;
    }

    memset(pAyuvmeCtx, 0x00, sizeof(Ayuvme_Context_V1_t));

    //gain state init
    pAyuvmeCtx->stStrength.strength_enable = true;
    pAyuvmeCtx->stStrength.percent = 1.0;

    pAyuvmeCtx->eState = AYUVMEV1_STATE_INITIALIZED;
    *ppAyuvmeCtx = pAyuvmeCtx;

    pAyuvmeCtx->eMode = AYUVMEV1_OP_MODE_AUTO;
    pAyuvmeCtx->isIQParaUpdate = false;
    pAyuvmeCtx->isGrayMode = false;
    pAyuvmeCtx->isReCalculate = 1;

#if AYUVME_USE_XML_FILE_V1
    //read v1 params from xml
#if AYUVME_USE_JSON_FILE_V1
    pAyuvmeCtx->yuvme_v1 =
        *(CalibDbV2_YuvmeV1_t *)(CALIBDBV2_GET_MODULE_PTR(pCalibDb, yuvme_v1));
#endif
#endif


#if AYUVME_USE_XML_FILE_V1
    pAyuvmeCtx->stExpInfo.snr_mode = 1;
    pAyuvmeCtx->eParamMode = AYUVMEV1_PARAM_MODE_NORMAL;
    Ayuvme_ConfigSettingParam_V1(pAyuvmeCtx, pAyuvmeCtx->eParamMode, pAyuvmeCtx->stExpInfo.snr_mode);
#endif

    LOGD_ANR("%s(%d):", __FUNCTION__, __LINE__);


    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return AYUVMEV1_RET_SUCCESS;
}

//anr release
Ayuvme_result_V1_t Ayuvme_Release_V1(Ayuvme_Context_V1_t *pAyuvmeCtx)
{
    Ayuvme_result_V1_t result = AYUVMEV1_RET_SUCCESS;
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    if(pAyuvmeCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYUVMEV1_RET_NULL_POINTER;
    }

    result = Ayuvme_Stop_V1(pAyuvmeCtx);
    if (result != AYUVMEV1_RET_SUCCESS) {
        LOGE_ANR( "%s: ANRStop() failed!\n", __FUNCTION__);
        return (result);
    }

    // check state
    if ((AYUVMEV1_STATE_RUNNING == pAyuvmeCtx->eState)
            || (AYUVMEV1_STATE_LOCKED == pAyuvmeCtx->eState)) {
        return (AYUVMEV1_RET_BUSY);
    }

    memset(pAyuvmeCtx, 0x00, sizeof(Ayuvme_Context_V1_t));
    free(pAyuvmeCtx);

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return AYUVMEV1_RET_SUCCESS;
}

//anr config
Ayuvme_result_V1_t Ayuvme_Prepare_V1(Ayuvme_Context_V1_t *pAyuvmeCtx, Ayuvme_Config_V1_t* pAyuvmeConfig)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    if(pAyuvmeCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYUVMEV1_RET_INVALID_PARM;
    }

    if(pAyuvmeConfig == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYUVMEV1_RET_INVALID_PARM;
    }

    if(!!(pAyuvmeCtx->prepare_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB)) {
        Ayuvme_IQParaUpdate_V1(pAyuvmeCtx);
    }

    pAyuvmeCtx->rawWidth = pAyuvmeConfig->rawWidth;
    pAyuvmeCtx->rawHeight = pAyuvmeConfig->rawHeight;
    Ayuvme_Start_V1(pAyuvmeCtx);

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return AYUVMEV1_RET_SUCCESS;
}

//anr reconfig
Ayuvme_result_V1_t Ayuvme_ReConfig_V1(Ayuvme_Context_V1_t *pAyuvmeCtx, Ayuvme_Config_V1_t* pAyuvmeConfig)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    //need todo what?

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return AYUVMEV1_RET_SUCCESS;
}

//anr reconfig
Ayuvme_result_V1_t Ayuvme_IQParaUpdate_V1(Ayuvme_Context_V1_t *pAyuvmeCtx)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    //need todo what?

    if(pAyuvmeCtx->isIQParaUpdate) {
        LOGD_ANR("IQ data reconfig\n");
        Ayuvme_ConfigSettingParam_V1(pAyuvmeCtx, pAyuvmeCtx->eParamMode, pAyuvmeCtx->stExpInfo.snr_mode);
        pAyuvmeCtx->isIQParaUpdate = false;
    }

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return AYUVMEV1_RET_SUCCESS;
}


//anr preprocess
Ayuvme_result_V1_t Ayuvme_PreProcess_V1(Ayuvme_Context_V1_t *pAyuvmeCtx)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    //need todo what?

    Ayuvme_IQParaUpdate_V1(pAyuvmeCtx);

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return AYUVMEV1_RET_SUCCESS;
}

//anr process
Ayuvme_result_V1_t Ayuvme_Process_V1(Ayuvme_Context_V1_t *pAyuvmeCtx, Ayuvme_ExpInfo_V1_t *pExpInfo)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    Ayuvme_ParamMode_V1_t mode = AYUVMEV1_PARAM_MODE_INVALID;

    if(pAyuvmeCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYUVMEV1_RET_INVALID_PARM;
    }

    if(pExpInfo == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYUVMEV1_RET_INVALID_PARM;
    }

    if(pAyuvmeCtx->eState != AYUVMEV1_STATE_RUNNING) {
        return AYUVMEV1_RET_SUCCESS;
    }


    Ayuvme_ParamModeProcess_V1(pAyuvmeCtx, pExpInfo, &mode);

    if(pAyuvmeCtx->eMode == AYUVMEV1_OP_MODE_AUTO) {

        LOGD_ANR("%s(%d): \n", __FUNCTION__, __LINE__);

#if AYUVME_USE_XML_FILE_V1
        if(pExpInfo->snr_mode != pAyuvmeCtx->stExpInfo.snr_mode || pAyuvmeCtx->eParamMode != mode) {
            LOGD_ANR("param mode:%d snr_mode:%d\n", mode, pExpInfo->snr_mode);
            pAyuvmeCtx->eParamMode = mode;
            Ayuvme_ConfigSettingParam_V1(pAyuvmeCtx, pAyuvmeCtx->eParamMode, pExpInfo->snr_mode);
        }
#endif

        //select param
        yuvme_select_params_by_ISO_V1(&pAyuvmeCtx->stAuto.stParams, &pAyuvmeCtx->stAuto.stSelect, pExpInfo);

    } else if(pAyuvmeCtx->eMode == AYUVMEV1_OP_MODE_MANUAL) {
        //TODO
    }

    memcpy(&pAyuvmeCtx->stExpInfo, pExpInfo, sizeof(Ayuvme_ExpInfo_V1_t));
    pAyuvmeCtx->stExpInfo.rawHeight = pAyuvmeCtx->rawHeight;
    pAyuvmeCtx->stExpInfo.rawWidth = pAyuvmeCtx->rawWidth;

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return AYUVMEV1_RET_SUCCESS;

}



//anr get result
Ayuvme_result_V1_t Ayuvme_GetProcResult_V1(Ayuvme_Context_V1_t *pAyuvmeCtx, Ayuvme_ProcResult_V1_t* pAyuvmeResult)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    if(pAyuvmeCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYUVMEV1_RET_INVALID_PARM;
    }

    if(pAyuvmeResult == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYUVMEV1_RET_INVALID_PARM;
    }

    RK_YUVME_Params_V1_Select_t* stSelect = NULL;
    if(pAyuvmeCtx->eMode == AYUVMEV1_OP_MODE_AUTO) {
        stSelect = &pAyuvmeCtx->stAuto.stSelect;
        //transfer to reg value
        yuvme_fix_transfer_V1(stSelect, pAyuvmeResult->stFix, &pAyuvmeCtx->stStrength, &pAyuvmeCtx->stExpInfo);
    } else if(pAyuvmeCtx->eMode == AYUVMEV1_OP_MODE_MANUAL) {
        stSelect = &pAyuvmeCtx->stManual.stSelect;
        //transfer to reg value
        yuvme_fix_transfer_V1(stSelect, pAyuvmeResult->stFix, &pAyuvmeCtx->stStrength, &pAyuvmeCtx->stExpInfo);
    } else if(pAyuvmeCtx->eMode == AYUVMEV1_OP_MODE_REG_MANUAL) {
        *pAyuvmeResult->stFix = pAyuvmeCtx->stManual.stFix;
        pAyuvmeCtx->stStrength.percent = 1.0;
    }

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return AYUVMEV1_RET_SUCCESS;
}

Ayuvme_result_V1_t Ayuvme_ConfigSettingParam_V1(Ayuvme_Context_V1_t *pAyuvmeCtx, Ayuvme_ParamMode_V1_t eParamMode, int snr_mode)
{
    char snr_name[CALIBDB_NR_SHARP_NAME_LENGTH];
    char param_mode_name[CALIBDB_MAX_MODE_NAME_LENGTH];
    memset(param_mode_name, 0x00, sizeof(param_mode_name));
    memset(snr_name, 0x00, sizeof(snr_name));

    if(pAyuvmeCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYUVMEV1_RET_INVALID_PARM;
    }

    //select param mode first
    if(eParamMode == AYUVMEV1_PARAM_MODE_NORMAL) {
        sprintf(param_mode_name, "%s", "normal");
    } else if(eParamMode == AYUVMEV1_PARAM_MODE_HDR) {
        sprintf(param_mode_name, "%s", "hdr");
    } else if(eParamMode == AYUVMEV1_PARAM_MODE_GRAY) {
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

#if AYUVME_USE_JSON_FILE_V1
    yuvme_config_setting_param_json_V1(&pAyuvmeCtx->stAuto.stParams, &pAyuvmeCtx->yuvme_v1, param_mode_name, snr_name);
#endif

    return AYUVMEV1_RET_SUCCESS;
}

Ayuvme_result_V1_t Ayuvme_ParamModeProcess_V1(Ayuvme_Context_V1_t *pAyuvmeCtx, Ayuvme_ExpInfo_V1_t *pExpInfo, Ayuvme_ParamMode_V1_t *mode) {
    Ayuvme_result_V1_t res  = AYUVMEV1_RET_SUCCESS;
    *mode = pAyuvmeCtx->eParamMode;

    if(pAyuvmeCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AYUVMEV1_RET_INVALID_PARM;
    }

    if(pAyuvmeCtx->isGrayMode) {
        *mode = AYUVMEV1_PARAM_MODE_GRAY;
    } else if(pExpInfo->hdr_mode == 0) {
        *mode = AYUVMEV1_PARAM_MODE_NORMAL;
    } else if(pExpInfo->hdr_mode >= 1) {
        *mode = AYUVMEV1_PARAM_MODE_HDR;
    } else {
        *mode = AYUVMEV1_PARAM_MODE_NORMAL;
    }

    return res;
}


RKAIQ_END_DECLARE


