
#include "rk_aiq_acnr_algo_v31.h"
#include "rk_aiq_acnr_algo_itf_v31.h"

RKAIQ_BEGIN_DECLARE

AcnrV31_result_t Acnr_Start_V31(Acnr_Context_V31_t *pAcnrCtx)
{
    LOGI_ANR( "%s:enter!\n", __FUNCTION__);

    // initial checks
    if (pAcnrCtx == NULL) {
        return (ACNRV31_RET_NULL_POINTER);
    }

    if ((ACNRV31_STATE_RUNNING == pAcnrCtx->eState)
            || (ACNRV31_STATE_LOCKED == pAcnrCtx->eState)) {
        return (ACNRV31_RET_FAILURE);
    }

    pAcnrCtx->eState = ACNRV31_STATE_RUNNING;

    LOGI_ANR( "%s:exit!\n", __FUNCTION__);
    return (ACNRV31_RET_SUCCESS);
}


AcnrV31_result_t Acnr_Stop_V31(Acnr_Context_V31_t *pAcnrCtx)
{
    LOGI_ANR( "%s:enter!\n", __FUNCTION__);

    // initial checks
    if (pAcnrCtx == NULL) {
        return (ACNRV31_RET_NULL_POINTER);
    }

    if (ACNRV31_STATE_LOCKED == pAcnrCtx->eState) {
        return (ACNRV31_RET_FAILURE);
    }

    pAcnrCtx->eState = ACNRV31_STATE_STOPPED;

    LOGI_ANR( "%s:exit!\n", __FUNCTION__);
    return (ACNRV31_RET_SUCCESS);
}


//anr inint
AcnrV31_result_t Acnr_Init_V31(Acnr_Context_V31_t **ppAcnrCtx, void *pCalibDb)
{
    Acnr_Context_V31_t * pAcnrCtx;

    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    pAcnrCtx = (Acnr_Context_V31_t *)malloc(sizeof(Acnr_Context_V31_t));
    if(pAcnrCtx == NULL) {
        LOGE_ANR("%s(%d): malloc fail\n", __FUNCTION__, __LINE__);
        return ACNRV31_RET_NULL_POINTER;
    }

    memset(pAcnrCtx, 0x00, sizeof(Acnr_Context_V31_t));

    //gain state init
    pAcnrCtx->stStrength.strength_enable = true;
    pAcnrCtx->stStrength.percent = 1.0;

    pAcnrCtx->eState = ACNRV31_STATE_INITIALIZED;
    *ppAcnrCtx = pAcnrCtx;

    pAcnrCtx->eMode = ACNRV31_OP_MODE_AUTO;
    pAcnrCtx->isIQParaUpdate = false;
    pAcnrCtx->isGrayMode = false;
    pAcnrCtx->isReCalculate = 1;

#if ACNR_USE_XML_FILE_V31
    //read v31 params from xml
#if ACNR_USE_JSON_FILE_V31
    pAcnrCtx->cnr_v31 =
        *(CalibDbV2_CNRV31_t *)(CALIBDBV2_GET_MODULE_PTR(pCalibDb, cnr_v31));
#endif

#endif

#if RK_SIMULATOR_HW
    //just for v31 params from html

#endif

#if ACNR_USE_XML_FILE_V31
    pAcnrCtx->stExpInfo.snr_mode = 1;
    pAcnrCtx->eParamMode = ACNRV31_PARAM_MODE_NORMAL;
    Acnr_ConfigSettingParam_V31(pAcnrCtx, pAcnrCtx->eParamMode, pAcnrCtx->stExpInfo.snr_mode);
#endif

    LOGD_ANR("%s(%d):", __FUNCTION__, __LINE__);


    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ACNRV31_RET_SUCCESS;
}

//anr release
AcnrV31_result_t Acnr_Release_V31(Acnr_Context_V31_t *pAcnrCtx)
{
    AcnrV31_result_t result = ACNRV31_RET_SUCCESS;
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    if(pAcnrCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNRV31_RET_NULL_POINTER;
    }

    result = Acnr_Stop_V31(pAcnrCtx);
    if (result != ACNRV31_RET_SUCCESS) {
        LOGE_ANR( "%s: ANRStop() failed!\n", __FUNCTION__);
        return (result);
    }

    // check state
    if ((ACNRV31_STATE_RUNNING == pAcnrCtx->eState)
            || (ACNRV31_STATE_LOCKED == pAcnrCtx->eState)) {
        return (ACNRV31_RET_BUSY);
    }

    memset(pAcnrCtx, 0x00, sizeof(Acnr_Context_V31_t));
    free(pAcnrCtx);

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ACNRV31_RET_SUCCESS;
}

//anr config
AcnrV31_result_t Acnr_Prepare_V31(Acnr_Context_V31_t *pAcnrCtx, Acnr_Config_V31_t* pAcnrConfig)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    if(pAcnrCtx == NULL || pAcnrConfig == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNRV31_RET_INVALID_PARM;
    }

    if(!!(pAcnrCtx->prepare_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB)) {
        Acnr_IQParaUpdate_V31(pAcnrCtx);
    }

    pAcnrCtx->rawWidth = pAcnrConfig->rawWidth;
    pAcnrCtx->rawHeight = pAcnrConfig->rawHeight;
    Acnr_Start_V31(pAcnrCtx);

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ACNRV31_RET_SUCCESS;
}

//anr reconfig
AcnrV31_result_t Acnr_ReConfig_V31(Acnr_Context_V31_t *pAcnrCtx, Acnr_Config_V31_t* pAcnrConfig)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    //need todo what?

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ACNRV31_RET_SUCCESS;
}

//anr reconfig
AcnrV31_result_t Acnr_IQParaUpdate_V31(Acnr_Context_V31_t *pAcnrCtx)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    //need todo what?

    if(pAcnrCtx->isIQParaUpdate) {
        LOGD_ANR("IQ data reconfig\n");
        Acnr_ConfigSettingParam_V31(pAcnrCtx, pAcnrCtx->eParamMode, pAcnrCtx->stExpInfo.snr_mode);
        pAcnrCtx->isIQParaUpdate = false;
    }

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ACNRV31_RET_SUCCESS;
}


//anr preprocess
AcnrV31_result_t Acnr_PreProcess_V31(Acnr_Context_V31_t *pAcnrCtx)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    //need todo what?

    Acnr_IQParaUpdate_V31(pAcnrCtx);

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ACNRV31_RET_SUCCESS;
}

//anr process
AcnrV31_result_t Acnr_Process_V31(Acnr_Context_V31_t *pAcnrCtx, AcnrV31_ExpInfo_t *pExpInfo)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    AcnrV31_ParamMode_t mode = ACNRV31_PARAM_MODE_INVALID;

    if(pAcnrCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNRV31_RET_INVALID_PARM;
    }

    if(pExpInfo == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNRV31_RET_INVALID_PARM;
    }

    if(pAcnrCtx->eState != ACNRV31_STATE_RUNNING) {
        return ACNRV31_RET_SUCCESS;
    }


    Acnr_ParamModeProcess_V31(pAcnrCtx, pExpInfo, &mode);

    if(pAcnrCtx->eMode == ACNRV31_OP_MODE_AUTO) {

        LOGD_ANR("%s(%d): \n", __FUNCTION__, __LINE__);

#if ACNR_USE_XML_FILE_V31
        if(pExpInfo->snr_mode != pAcnrCtx->stExpInfo.snr_mode || pAcnrCtx->eParamMode != mode) {
            LOGD_ANR("param mode:%d snr_mode:%d\n", mode, pExpInfo->snr_mode);
            pAcnrCtx->eParamMode = mode;
            Acnr_ConfigSettingParam_V31(pAcnrCtx, pAcnrCtx->eParamMode, pExpInfo->snr_mode);
        }
#endif

        //select param
        cnr_select_params_by_ISO_V31(&pAcnrCtx->stAuto.stParams, &pAcnrCtx->stAuto.stSelect, pExpInfo);

    } else if(pAcnrCtx->eMode == ACNRV31_OP_MODE_MANUAL) {
        //TODO
    }

    memcpy(&pAcnrCtx->stExpInfo, pExpInfo, sizeof(AcnrV31_ExpInfo_t));
    pAcnrCtx->stExpInfo.rawHeight = pAcnrCtx->rawHeight;
    pAcnrCtx->stExpInfo.rawWidth = pAcnrCtx->rawWidth;

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ACNRV31_RET_SUCCESS;

}



//anr get result
AcnrV31_result_t Acnr_GetProcResult_V31(Acnr_Context_V31_t *pAcnrCtx, Acnr_ProcResult_V31_t* pAcnrResult)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    if(pAcnrCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNRV31_RET_INVALID_PARM;
    }

    if(pAcnrResult == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNRV31_RET_INVALID_PARM;
    }

    RK_CNR_Params_V31_Select_t* stSelect = NULL;
    if(pAcnrCtx->eMode == ACNRV31_OP_MODE_AUTO) {
        stSelect = &pAcnrCtx->stAuto.stSelect;
        //transfer to reg value
        cnr_fix_transfer_V31(stSelect, pAcnrResult->stFix,  &pAcnrCtx->stExpInfo, &pAcnrCtx->stStrength);
    } else if(pAcnrCtx->eMode == ACNRV31_OP_MODE_MANUAL) {
        stSelect = &pAcnrCtx->stManual.stSelect;
        //transfer to reg value
        cnr_fix_transfer_V31(stSelect, pAcnrResult->stFix,  &pAcnrCtx->stExpInfo, &pAcnrCtx->stStrength);
    }
    else if(pAcnrCtx->eMode == ACNRV31_OP_MODE_REG_MANUAL) {
        *pAcnrResult->stFix = pAcnrCtx->stManual.stFix;
        pAcnrCtx->stStrength.percent = 1.0;
    }

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ACNRV31_RET_SUCCESS;
}

AcnrV31_result_t Acnr_ConfigSettingParam_V31(Acnr_Context_V31_t *pAcnrCtx, AcnrV31_ParamMode_t eParamMode, int snr_mode)
{
    char snr_name[CALIBDB_NR_SHARP_NAME_LENGTH];
    char param_mode_name[CALIBDB_MAX_MODE_NAME_LENGTH];
    memset(param_mode_name, 0x00, sizeof(param_mode_name));
    memset(snr_name, 0x00, sizeof(snr_name));

    if(pAcnrCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNRV31_RET_INVALID_PARM;
    }

    //select param mode first
    if(eParamMode == ACNRV31_PARAM_MODE_NORMAL) {
        sprintf(param_mode_name, "%s", "normal");
    } else if(eParamMode == ACNRV31_PARAM_MODE_HDR) {
        sprintf(param_mode_name, "%s", "hdr");
    } else if(eParamMode == ACNRV31_PARAM_MODE_GRAY) {
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

#if ACNR_USE_JSON_FILE_V31
    cnr_config_setting_param_json_V31(&pAcnrCtx->stAuto.stParams, &pAcnrCtx->cnr_v31, param_mode_name, snr_name);
#endif

    return ACNRV31_RET_SUCCESS;
}

AcnrV31_result_t Acnr_ParamModeProcess_V31(Acnr_Context_V31_t *pAcnrCtx, AcnrV31_ExpInfo_t *pExpInfo, AcnrV31_ParamMode_t *mode) {
    AcnrV31_result_t res  = ACNRV31_RET_SUCCESS;
    *mode = pAcnrCtx->eParamMode;

    if(pAcnrCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNRV31_RET_INVALID_PARM;
    }

    if(pAcnrCtx->isGrayMode) {
        *mode = ACNRV31_PARAM_MODE_GRAY;
    } else if(pExpInfo->hdr_mode == 0) {
        *mode = ACNRV31_PARAM_MODE_NORMAL;
    } else if(pExpInfo->hdr_mode >= 1) {
        *mode = ACNRV31_PARAM_MODE_HDR;
    } else {
        *mode = ACNRV31_PARAM_MODE_NORMAL;
    }

    return res;
}


RKAIQ_END_DECLARE


