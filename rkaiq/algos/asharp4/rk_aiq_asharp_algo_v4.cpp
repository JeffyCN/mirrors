
#include "rk_aiq_asharp_algo_v4.h"
#include "rk_aiq_asharp_algo_itf_v4.h"

RKAIQ_BEGIN_DECLARE

Asharp4_result_t Asharp_Start_V4(Asharp_Context_V4_t *pAsharpCtx)
{
    LOGI_ASHARP( "%s:enter!\n", __FUNCTION__);

    // initial checks
    if (pAsharpCtx == NULL) {
        return (ASHARP4_RET_NULL_POINTER);
    }

    if ((ASHARP4_STATE_RUNNING == pAsharpCtx->eState)
            || (ASHARP4_STATE_LOCKED == pAsharpCtx->eState)) {
        return (ASHARP4_RET_FAILURE);
    }

    pAsharpCtx->eState = ASHARP4_STATE_RUNNING;

    LOGI_ASHARP( "%s:exit!\n", __FUNCTION__);
    return (ASHARP4_RET_SUCCESS);
}


Asharp4_result_t Asharp_Stop_V4(Asharp_Context_V4_t *pAsharpCtx)
{
    LOGI_ASHARP( "%s:enter!\n", __FUNCTION__);

    // initial checks
    if (pAsharpCtx == NULL) {
        return (ASHARP4_RET_NULL_POINTER);
    }

    if (ASHARP4_STATE_LOCKED == pAsharpCtx->eState) {
        return (ASHARP4_RET_FAILURE);
    }

    pAsharpCtx->eState = ASHARP4_STATE_STOPPED;

    LOGI_ASHARP( "%s:exit!\n", __FUNCTION__);
    return (ASHARP4_RET_SUCCESS);
}


//anr inint
Asharp4_result_t Asharp_Init_V4(Asharp_Context_V4_t **ppAsharpCtx, void *pCalibDb)
{
    Asharp_Context_V4_t * pAsharpCtx;

    LOGI_ASHARP("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    pAsharpCtx = (Asharp_Context_V4_t *)malloc(sizeof(Asharp_Context_V4_t));
    if(pAsharpCtx == NULL) {
        LOGE_ASHARP("%s(%d): malloc fail\n", __FUNCTION__, __LINE__);
        return ASHARP4_RET_NULL_POINTER;
    }

    memset(pAsharpCtx, 0x00, sizeof(Asharp_Context_V4_t));

    //gain state init
    pAsharpCtx->stStrength.strength_enable = false;
    pAsharpCtx->stStrength.percent = 1.0;

    pAsharpCtx->eState = ASHARP4_STATE_INITIALIZED;
    *ppAsharpCtx = pAsharpCtx;

    pAsharpCtx->eMode = ASHARP4_OP_MODE_AUTO;
    pAsharpCtx->isIQParaUpdate = false;
    pAsharpCtx->isGrayMode = false;
    pAsharpCtx->isReCalculate = 1;

#if ASHARP_USE_XML_FILE_V4
    //read v1 params from xml
#if ASHARP_USE_JSON_FILE_V4
    pAsharpCtx->sharp_v4 =
        *(CalibDbV2_SharpV4_t *)(CALIBDBV2_GET_MODULE_PTR(pCalibDb, sharp_v4));
#endif
#endif

#if RK_SIMULATOR_HW
    //just for v4 params from html

#endif

#if ASHARP_USE_XML_FILE_V4
    pAsharpCtx->stExpInfo.snr_mode = 0;
    pAsharpCtx->eParamMode = ASHARP4_PARAM_MODE_NORMAL;
    Asharp_ConfigSettingParam_V4(pAsharpCtx, pAsharpCtx->eParamMode, pAsharpCtx->stExpInfo.snr_mode);
#endif

    LOGD_ASHARP("%s(%d):", __FUNCTION__, __LINE__);


    LOGI_ASHARP("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ASHARP4_RET_SUCCESS;
}

//anr release
Asharp4_result_t Asharp_Release_V4(Asharp_Context_V4_t *pAsharpCtx)
{
    Asharp4_result_t result = ASHARP4_RET_SUCCESS;
    LOGI_ASHARP("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    if(pAsharpCtx == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP4_RET_NULL_POINTER;
    }

    result = Asharp_Stop_V4(pAsharpCtx);
    if (result != ASHARP4_RET_SUCCESS) {
        LOGE_ASHARP( "%s: ANRStop() failed!\n", __FUNCTION__);
        return (result);
    }

    // check state
    if ((ASHARP4_STATE_RUNNING == pAsharpCtx->eState)
            || (ASHARP4_STATE_LOCKED == pAsharpCtx->eState)) {
        return (ASHARP4_RET_BUSY);
    }

    memset(pAsharpCtx, 0x00, sizeof(Asharp_Context_V4_t));
    free(pAsharpCtx);

    LOGI_ASHARP("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ASHARP4_RET_SUCCESS;
}

//anr config
Asharp4_result_t Asharp_Prepare_V4(Asharp_Context_V4_t *pAsharpCtx, Asharp_Config_V4_t* pAsharpConfig)
{
    LOGI_ASHARP("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    if(pAsharpCtx == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP4_RET_INVALID_PARM;
    }

    if(pAsharpConfig == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP4_RET_INVALID_PARM;
    }

    if(!!(pAsharpCtx->prepare_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB)) {
        Asharp_IQParaUpdate_V4(pAsharpCtx);
    }

    pAsharpCtx->rawWidth = pAsharpConfig->rawWidth;
    pAsharpCtx->rawHeight = pAsharpConfig->rawHeight;
    Asharp_Start_V4(pAsharpCtx);

    LOGI_ASHARP("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ASHARP4_RET_SUCCESS;
}

//anr reconfig
Asharp4_result_t Asharp_ReConfig_V4(Asharp_Context_V4_t *pAsharpCtx, Asharp_Config_V4_t* pAsharpConfig)
{
    LOGI_ASHARP("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    //need todo what?

    LOGI_ASHARP("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ASHARP4_RET_SUCCESS;
}

//anr reconfig
Asharp4_result_t Asharp_IQParaUpdate_V4(Asharp_Context_V4_t *pAsharpCtx)
{
    LOGI_ASHARP("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    //need todo what?

    if(pAsharpCtx->isIQParaUpdate) {
        LOGD_ASHARP("IQ data reconfig\n");
        Asharp_ConfigSettingParam_V4(pAsharpCtx, pAsharpCtx->eParamMode, pAsharpCtx->stExpInfo.snr_mode);
        pAsharpCtx->isIQParaUpdate = false;
    }

    LOGI_ASHARP("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ASHARP4_RET_SUCCESS;
}


//anr preprocess
Asharp4_result_t Asharp_PreProcess_V4(Asharp_Context_V4_t *pAsharpCtx)
{
    LOGI_ASHARP("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    //need todo what?

    Asharp_IQParaUpdate_V4(pAsharpCtx);

    LOGI_ASHARP("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ASHARP4_RET_SUCCESS;
}

//anr process
Asharp4_result_t Asharp_Process_V4(Asharp_Context_V4_t *pAsharpCtx, Asharp4_ExpInfo_t *pExpInfo)
{
    LOGI_ASHARP("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    Asharp4_ParamMode_t mode = ASHARP4_PARAM_MODE_INVALID;

    if(pAsharpCtx == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP4_RET_INVALID_PARM;
    }

    if(pExpInfo == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP4_RET_INVALID_PARM;
    }

    if(pAsharpCtx->eState != ASHARP4_STATE_RUNNING) {
        return ASHARP4_RET_SUCCESS;
    }


    Asharp_ParamModeProcess_V4(pAsharpCtx, pExpInfo, &mode);

    if(pAsharpCtx->eMode == ASHARP4_OP_MODE_AUTO) {

        LOGD_ASHARP("%s(%d): \n", __FUNCTION__, __LINE__);

#if ASHARP_USE_XML_FILE_V4
        if(pExpInfo->snr_mode != pAsharpCtx->stExpInfo.snr_mode || pAsharpCtx->eParamMode != mode) {
            LOGD_ASHARP("param mode:%d snr_mode:%d\n", mode, pExpInfo->snr_mode);
            pAsharpCtx->eParamMode = mode;
            Asharp_ConfigSettingParam_V4(pAsharpCtx, pAsharpCtx->eParamMode, pExpInfo->snr_mode);
        }
#endif

        //select param
        sharp_select_params_by_ISO_V4(&pAsharpCtx->stAuto.stParams, &pAsharpCtx->stAuto.stSelect, pExpInfo);

    } else if(pAsharpCtx->eMode == ASHARP4_OP_MODE_MANUAL) {
        //TODO
    }

    memcpy(&pAsharpCtx->stExpInfo, pExpInfo, sizeof(Asharp4_ExpInfo_t));
    pAsharpCtx->stExpInfo.rawHeight = pAsharpCtx->rawHeight;
    pAsharpCtx->stExpInfo.rawWidth = pAsharpCtx->rawWidth;

    LOGI_ASHARP("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ASHARP4_RET_SUCCESS;

}



//anr get result
Asharp4_result_t Asharp_GetProcResult_V4(Asharp_Context_V4_t *pAsharpCtx, Asharp_ProcResult_V4_t* pAsharpResult)
{
    LOGI_ASHARP("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    if(pAsharpCtx == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP4_RET_INVALID_PARM;
    }

    if(pAsharpResult == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP4_RET_INVALID_PARM;
    }

    RK_SHARP_Params_V4_Select_t* stSelect = NULL;
    if(pAsharpCtx->eMode == ASHARP4_OP_MODE_AUTO) {
        stSelect = &pAsharpCtx->stAuto.stSelect;
    } else if(pAsharpCtx->eMode == ASHARP4_OP_MODE_MANUAL) {
        //TODO
        stSelect = &pAsharpCtx->stManual.stSelect;
    }

    //transfer to reg value
    sharp_fix_transfer_V4(stSelect, pAsharpResult->stFix, &pAsharpCtx->stStrength);

    if(pAsharpCtx->eMode == ASHARP4_OP_MODE_REG_MANUAL) {
        *pAsharpResult->stFix = pAsharpCtx->stManual.stFix;
        pAsharpCtx->stStrength.strength_enable = false;
        pAsharpCtx->stStrength.percent = 1.0;
    }

    LOGD_ASHARP("%s:%d xml:local:%d mode:%d  reg: local gain:%d  mfnr gain:%d mode:%d\n",
                __FUNCTION__, __LINE__);

    LOGI_ASHARP("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ASHARP4_RET_SUCCESS;
}

Asharp4_result_t Asharp_ConfigSettingParam_V4(Asharp_Context_V4_t *pAsharpCtx, Asharp4_ParamMode_t eParamMode, int snr_mode)
{
    char snr_name[CALIBDB_NR_SHARP_NAME_LENGTH];
    char param_mode_name[CALIBDB_MAX_MODE_NAME_LENGTH];
    memset(param_mode_name, 0x00, sizeof(param_mode_name));
    memset(snr_name, 0x00, sizeof(snr_name));

    if(pAsharpCtx == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP4_RET_INVALID_PARM;
    }

    //select param mode first
    if(eParamMode == ASHARP4_PARAM_MODE_NORMAL) {
        sprintf(param_mode_name, "%s", "normal");
    } else if(eParamMode == ASHARP4_PARAM_MODE_HDR) {
        sprintf(param_mode_name, "%s", "hdr");
    } else if(eParamMode == ASHARP4_PARAM_MODE_GRAY) {
        sprintf(param_mode_name, "%s", "gray");
    } else {
        LOGE_ASHARP("%s(%d): not support param mode!\n", __FUNCTION__, __LINE__);
        sprintf(param_mode_name, "%s", "normal");
    }


    //then select snr mode next
    if(snr_mode == 1) {
        sprintf(snr_name, "%s", "HSNR");
    } else if(snr_mode == 0) {
        sprintf(snr_name, "%s", "LSNR");
    } else {
        LOGE_ASHARP("%s(%d): not support snr mode:%d!\n", __FUNCTION__, __LINE__, snr_mode);
        sprintf(snr_name, "%s", "LSNR");
    }

#if ASHARP_USE_JSON_FILE_V4
    sharp_config_setting_param_json_V4(&pAsharpCtx->stAuto.stParams, &pAsharpCtx->sharp_v4, param_mode_name, snr_name);
#endif

    return ASHARP4_RET_SUCCESS;
}

Asharp4_result_t Asharp_ParamModeProcess_V4(Asharp_Context_V4_t *pAsharpCtx, Asharp4_ExpInfo_t *pExpInfo, Asharp4_ParamMode_t *mode) {
    Asharp4_result_t res  = ASHARP4_RET_SUCCESS;

    if(pAsharpCtx == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP4_RET_INVALID_PARM;
    }

    *mode = pAsharpCtx->eParamMode;

    if(pAsharpCtx->isGrayMode) {
        *mode = ASHARP4_PARAM_MODE_GRAY;
    } else if(pExpInfo->hdr_mode == 0) {
        *mode = ASHARP4_PARAM_MODE_NORMAL;
    } else if(pExpInfo->hdr_mode >= 1) {
        *mode = ASHARP4_PARAM_MODE_HDR;
    } else {
        *mode = ASHARP4_PARAM_MODE_NORMAL;
    }

    return res;
}


RKAIQ_END_DECLARE


