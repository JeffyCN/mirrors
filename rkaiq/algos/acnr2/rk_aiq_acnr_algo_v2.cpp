
#include "rk_aiq_acnr_algo_v2.h"
#include "rk_aiq_acnr_algo_itf_v2.h"

RKAIQ_BEGIN_DECLARE

AcnrV2_result_t Acnr_Start_V2(Acnr_Context_V2_t *pAcnrCtx)
{
    LOGI_ANR( "%s:enter!\n", __FUNCTION__);

    // initial checks
    if (pAcnrCtx == NULL) {
        return (ACNRV2_RET_NULL_POINTER);
    }

    if ((ACNRV2_STATE_RUNNING == pAcnrCtx->eState)
            || (ACNRV2_STATE_LOCKED == pAcnrCtx->eState)) {
        return (ACNRV2_RET_FAILURE);
    }

    pAcnrCtx->eState = ACNRV2_STATE_RUNNING;

    LOGI_ANR( "%s:exit!\n", __FUNCTION__);
    return (ACNRV2_RET_SUCCESS);
}


AcnrV2_result_t Acnr_Stop_V2(Acnr_Context_V2_t *pAcnrCtx)
{
    LOGI_ANR( "%s:enter!\n", __FUNCTION__);

    // initial checks
    if (pAcnrCtx == NULL) {
        return (ACNRV2_RET_NULL_POINTER);
    }

    if (ACNRV2_STATE_LOCKED == pAcnrCtx->eState) {
        return (ACNRV2_RET_FAILURE);
    }

    pAcnrCtx->eState = ACNRV2_STATE_STOPPED;

    LOGI_ANR( "%s:exit!\n", __FUNCTION__);
    return (ACNRV2_RET_SUCCESS);
}


//anr inint
AcnrV2_result_t Acnr_Init_V2(Acnr_Context_V2_t **ppAcnrCtx, void *pCalibDb)
{
    Acnr_Context_V2_t * pAcnrCtx;

    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    pAcnrCtx = (Acnr_Context_V2_t *)malloc(sizeof(Acnr_Context_V2_t));
    if(pAcnrCtx == NULL) {
        LOGE_ANR("%s(%d): malloc fail\n", __FUNCTION__, __LINE__);
        return ACNRV2_RET_NULL_POINTER;
    }

    memset(pAcnrCtx, 0x00, sizeof(Acnr_Context_V2_t));

    //gain state init
    pAcnrCtx->stStrength.strength_enable = false;
    pAcnrCtx->stStrength.percent = 1.0;

    pAcnrCtx->eState = ACNRV2_STATE_INITIALIZED;
    *ppAcnrCtx = pAcnrCtx;

    pAcnrCtx->eMode = ACNRV2_OP_MODE_AUTO;
    pAcnrCtx->isIQParaUpdate = false;
    pAcnrCtx->isGrayMode = false;
    pAcnrCtx->isReCalculate = 1;

#if ACNR_USE_XML_FILE_V2
    //read v2 params from xml
#if ACNR_USE_JSON_FILE_V2
    pAcnrCtx->cnr_v2 =
        *(CalibDbV2_CNRV2_t *)(CALIBDBV2_GET_MODULE_PTR(pCalibDb, cnr_v2));
#endif

#endif

#if RK_SIMULATOR_HW
    //just for v2 params from html

#endif

#if ACNR_USE_XML_FILE_V2
    pAcnrCtx->stExpInfo.snr_mode = 1;
    pAcnrCtx->eParamMode = ACNRV2_PARAM_MODE_NORMAL;
    Acnr_ConfigSettingParam_V2(pAcnrCtx, pAcnrCtx->eParamMode, pAcnrCtx->stExpInfo.snr_mode);
#endif

    LOGD_ANR("%s(%d):", __FUNCTION__, __LINE__);


    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ACNRV2_RET_SUCCESS;
}

//anr release
AcnrV2_result_t Acnr_Release_V2(Acnr_Context_V2_t *pAcnrCtx)
{
    AcnrV2_result_t result = ACNRV2_RET_SUCCESS;
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    if(pAcnrCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNRV2_RET_NULL_POINTER;
    }

    result = Acnr_Stop_V2(pAcnrCtx);
    if (result != ACNRV2_RET_SUCCESS) {
        LOGE_ANR( "%s: ANRStop() failed!\n", __FUNCTION__);
        return (result);
    }

    // check state
    if ((ACNRV2_STATE_RUNNING == pAcnrCtx->eState)
            || (ACNRV2_STATE_LOCKED == pAcnrCtx->eState)) {
        return (ACNRV2_RET_BUSY);
    }

    memset(pAcnrCtx, 0x00, sizeof(Acnr_Context_V2_t));
    free(pAcnrCtx);

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ACNRV2_RET_SUCCESS;
}

//anr config
AcnrV2_result_t Acnr_Prepare_V2(Acnr_Context_V2_t *pAcnrCtx, Acnr_Config_V2_t* pAcnrConfig)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    if(pAcnrCtx == NULL || pAcnrConfig == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNRV2_RET_INVALID_PARM;
    }

    if(!!(pAcnrCtx->prepare_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB)) {
        Acnr_IQParaUpdate_V2(pAcnrCtx);
    }

    pAcnrCtx->rawWidth = pAcnrConfig->rawWidth;
    pAcnrCtx->rawHeight = pAcnrConfig->rawHeight;
    Acnr_Start_V2(pAcnrCtx);

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ACNRV2_RET_SUCCESS;
}

//anr reconfig
AcnrV2_result_t Acnr_ReConfig_V2(Acnr_Context_V2_t *pAcnrCtx, Acnr_Config_V2_t* pAcnrConfig)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    //need todo what?

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ACNRV2_RET_SUCCESS;
}

//anr reconfig
AcnrV2_result_t Acnr_IQParaUpdate_V2(Acnr_Context_V2_t *pAcnrCtx)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    //need todo what?

    if(pAcnrCtx->isIQParaUpdate) {
        LOGD_ANR("IQ data reconfig\n");
        Acnr_ConfigSettingParam_V2(pAcnrCtx, pAcnrCtx->eParamMode, pAcnrCtx->stExpInfo.snr_mode);
        pAcnrCtx->isIQParaUpdate = false;
    }

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ACNRV2_RET_SUCCESS;
}


//anr preprocess
AcnrV2_result_t Acnr_PreProcess_V2(Acnr_Context_V2_t *pAcnrCtx)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    //need todo what?

    Acnr_IQParaUpdate_V2(pAcnrCtx);

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ACNRV2_RET_SUCCESS;
}

//anr process
AcnrV2_result_t Acnr_Process_V2(Acnr_Context_V2_t *pAcnrCtx, AcnrV2_ExpInfo_t *pExpInfo)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    AcnrV2_ParamMode_t mode = ACNRV2_PARAM_MODE_INVALID;

    if(pAcnrCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNRV2_RET_INVALID_PARM;
    }

    if(pExpInfo == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNRV2_RET_INVALID_PARM;
    }

    if(pAcnrCtx->eState != ACNRV2_STATE_RUNNING) {
        return ACNRV2_RET_SUCCESS;
    }


    Acnr_ParamModeProcess_V2(pAcnrCtx, pExpInfo, &mode);

    if(pAcnrCtx->eMode == ACNRV2_OP_MODE_AUTO) {

        LOGD_ANR("%s(%d): \n", __FUNCTION__, __LINE__);

#if ACNR_USE_XML_FILE_V2
        if(pExpInfo->snr_mode != pAcnrCtx->stExpInfo.snr_mode || pAcnrCtx->eParamMode != mode) {
            LOGD_ANR("param mode:%d snr_mode:%d\n", mode, pExpInfo->snr_mode);
            pAcnrCtx->eParamMode = mode;
            Acnr_ConfigSettingParam_V2(pAcnrCtx, pAcnrCtx->eParamMode, pExpInfo->snr_mode);
        }
#endif

        //select param
        cnr_select_params_by_ISO_V2(&pAcnrCtx->stAuto.stParams, &pAcnrCtx->stAuto.stSelect, pExpInfo);

    } else if(pAcnrCtx->eMode == ACNRV2_OP_MODE_MANUAL) {
        //TODO
    }

    memcpy(&pAcnrCtx->stExpInfo, pExpInfo, sizeof(AcnrV2_ExpInfo_t));
    pAcnrCtx->stExpInfo.rawHeight = pAcnrCtx->rawHeight;
    pAcnrCtx->stExpInfo.rawWidth = pAcnrCtx->rawWidth;

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ACNRV2_RET_SUCCESS;

}



//anr get result
AcnrV2_result_t Acnr_GetProcResult_V2(Acnr_Context_V2_t *pAcnrCtx, Acnr_ProcResult_V2_t* pAcnrResult)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    if(pAcnrCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNRV2_RET_INVALID_PARM;
    }

    if(pAcnrResult == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNRV2_RET_INVALID_PARM;
    }

    RK_CNR_Params_V2_Select_t* stSelect = NULL;
    if(pAcnrCtx->eMode == ACNRV2_OP_MODE_AUTO) {
        stSelect = &pAcnrCtx->stAuto.stSelect;
    } else if(pAcnrCtx->eMode == ACNRV2_OP_MODE_MANUAL) {
        stSelect = &pAcnrCtx->stManual.stSelect;
    }

    //transfer to reg value
    cnr_fix_transfer_V2(stSelect, pAcnrResult->stFix,  &pAcnrCtx->stExpInfo, &pAcnrCtx->stStrength);

    if(pAcnrCtx->eMode == ACNRV2_OP_MODE_REG_MANUAL) {
        *pAcnrResult->stFix = pAcnrCtx->stManual.stFix;
        pAcnrCtx->stStrength.strength_enable = false;
        pAcnrCtx->stStrength.percent = 1.0;
    }


    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ACNRV2_RET_SUCCESS;
}

AcnrV2_result_t Acnr_ConfigSettingParam_V2(Acnr_Context_V2_t *pAcnrCtx, AcnrV2_ParamMode_t eParamMode, int snr_mode)
{
    char snr_name[CALIBDB_NR_SHARP_NAME_LENGTH];
    char param_mode_name[CALIBDB_MAX_MODE_NAME_LENGTH];
    memset(param_mode_name, 0x00, sizeof(param_mode_name));
    memset(snr_name, 0x00, sizeof(snr_name));

    if(pAcnrCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNRV2_RET_INVALID_PARM;
    }

    //select param mode first
    if(eParamMode == ACNRV2_PARAM_MODE_NORMAL) {
        sprintf(param_mode_name, "%s", "normal");
    } else if(eParamMode == ACNRV2_PARAM_MODE_HDR) {
        sprintf(param_mode_name, "%s", "hdr");
    } else if(eParamMode == ACNRV2_PARAM_MODE_GRAY) {
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

#if ACNR_USE_JSON_FILE_V2
    cnr_config_setting_param_json_V2(&pAcnrCtx->stAuto.stParams, &pAcnrCtx->cnr_v2, param_mode_name, snr_name);
#endif

    return ACNRV2_RET_SUCCESS;
}

AcnrV2_result_t Acnr_ParamModeProcess_V2(Acnr_Context_V2_t *pAcnrCtx, AcnrV2_ExpInfo_t *pExpInfo, AcnrV2_ParamMode_t *mode) {
    AcnrV2_result_t res  = ACNRV2_RET_SUCCESS;

    if(pAcnrCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNRV2_RET_INVALID_PARM;
    }

    *mode = pAcnrCtx->eParamMode;

    if(pAcnrCtx->isGrayMode) {
        *mode = ACNRV2_PARAM_MODE_GRAY;
    } else if(pExpInfo->hdr_mode == 0) {
        *mode = ACNRV2_PARAM_MODE_NORMAL;
    } else if(pExpInfo->hdr_mode >= 1) {
        *mode = ACNRV2_PARAM_MODE_HDR;
    } else {
        *mode = ACNRV2_PARAM_MODE_NORMAL;
    }

    return res;
}


RKAIQ_END_DECLARE


