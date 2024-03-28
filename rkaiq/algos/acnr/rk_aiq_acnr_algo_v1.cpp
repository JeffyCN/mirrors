
#include "rk_aiq_acnr_algo_v1.h"
#include "rk_aiq_acnr_algo_itf_v1.h"
#include "RkAiqCalibDbV2Helper.h"

RKAIQ_BEGIN_DECLARE

Acnr_result_t Acnr_Start_V1(Acnr_Context_V1_t *pAcnrCtx)
{
    LOGI_ANR( "%s:enter!\n", __FUNCTION__);

    // initial checks
    if (pAcnrCtx == NULL) {
        return (ACNR_RET_NULL_POINTER);
    }

    if ((ACNR_STATE_RUNNING == pAcnrCtx->eState)
            || (ACNR_STATE_LOCKED == pAcnrCtx->eState)) {
        return (ACNR_RET_FAILURE);
    }

    pAcnrCtx->eState = ACNR_STATE_RUNNING;

    LOGI_ANR( "%s:exit!\n", __FUNCTION__);
    return (ACNR_RET_SUCCESS);
}


Acnr_result_t Acnr_Stop_V1(Acnr_Context_V1_t *pAcnrCtx)
{
    LOGI_ANR( "%s:enter!\n", __FUNCTION__);

    // initial checks
    if (pAcnrCtx == NULL) {
        return (ACNR_RET_NULL_POINTER);
    }

    if (ACNR_STATE_LOCKED == pAcnrCtx->eState) {
        return (ACNR_RET_FAILURE);
    }

    pAcnrCtx->eState = ACNR_STATE_STOPPED;

    LOGI_ANR( "%s:exit!\n", __FUNCTION__);
    return (ACNR_RET_SUCCESS);
}


//anr inint
Acnr_result_t Acnr_Init_V1(Acnr_Context_V1_t **ppAcnrCtx, void *pCalibDb)
{
    Acnr_Context_V1_t * pAcnrCtx;

    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    pAcnrCtx = (Acnr_Context_V1_t *)malloc(sizeof(Acnr_Context_V1_t));
    if(pAcnrCtx == NULL) {
        LOGE_ANR("%s(%d): malloc fail\n", __FUNCTION__, __LINE__);
        return ACNR_RET_NULL_POINTER;
    }

    memset(pAcnrCtx, 0x00, sizeof(Acnr_Context_V1_t));

    //gain state init
    pAcnrCtx->fCnr_SF_Strength = 1.0;

    pAcnrCtx->eState = ACNR_STATE_INITIALIZED;
    *ppAcnrCtx = pAcnrCtx;

    pAcnrCtx->eMode = ACNR_OP_MODE_AUTO;
    pAcnrCtx->isIQParaUpdate = false;
    pAcnrCtx->isGrayMode = false;
    pAcnrCtx->isReCalculate = 1;

#if ACNR_USE_XML_FILE_V1
    //read v1 params from xml
#if ACNR_USE_JSON_FILE_V1
    pAcnrCtx->cnr_v1 =
        *(CalibDbV2_CNR_t *)(CALIBDBV2_GET_MODULE_PTR(pCalibDb, cnr_v1));
#else
    pAcnrCtx->list_cnr_v1 =
        (struct list_head*)(CALIBDB_GET_MODULE_PTR(pCalibDb, list_cnr_v1));
    printf("%s(%d): cnrV1 list:%p\n", __FUNCTION__, __LINE__, pAcnrCtx->list_cnr_v1);
#endif
#endif

#if RK_SIMULATOR_HW
    //just for v2 params from html

#endif

#if ACNR_USE_XML_FILE_V1
    pAcnrCtx->stExpInfo.snr_mode = 1;
    pAcnrCtx->eParamMode = ACNR_PARAM_MODE_NORMAL;
    Acnr_ConfigSettingParam_V1(pAcnrCtx, pAcnrCtx->eParamMode, pAcnrCtx->stExpInfo.snr_mode);
#endif

    LOGD_ANR("%s(%d):", __FUNCTION__, __LINE__);


    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ACNR_RET_SUCCESS;
}

//anr release
Acnr_result_t Acnr_Release_V1(Acnr_Context_V1_t *pAcnrCtx)
{
    Acnr_result_t result = ACNR_RET_SUCCESS;
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    if(pAcnrCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNR_RET_NULL_POINTER;
    }

    result = Acnr_Stop_V1(pAcnrCtx);
    if (result != ACNR_RET_SUCCESS) {
        LOGE_ANR( "%s: ANRStop() failed!\n", __FUNCTION__);
        return (result);
    }

    // check state
    if ((ACNR_STATE_RUNNING == pAcnrCtx->eState)
            || (ACNR_STATE_LOCKED == pAcnrCtx->eState)) {
        return (ACNR_RET_BUSY);
    }

    memset(pAcnrCtx, 0x00, sizeof(Acnr_Context_V1_t));
    free(pAcnrCtx);

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ACNR_RET_SUCCESS;
}

//anr config
Acnr_result_t Acnr_Prepare_V1(Acnr_Context_V1_t *pAcnrCtx, Acnr_Config_V1_t* pAcnrConfig)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    if(pAcnrCtx == NULL || pAcnrConfig == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNR_RET_INVALID_PARM;
    }

    if(!!(pAcnrCtx->prepare_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB)) {
        Acnr_IQParaUpdate_V1(pAcnrCtx);
    }

    pAcnrCtx->rawWidth = pAcnrConfig->rawWidth;
    pAcnrCtx->rawHeight = pAcnrConfig->rawHeight;
    Acnr_Start_V1(pAcnrCtx);

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ACNR_RET_SUCCESS;
}

//anr reconfig
Acnr_result_t Acnr_ReConfig_V1(Acnr_Context_V1_t *pAcnrCtx, Acnr_Config_V1_t* pAcnrConfig)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    //need todo what?

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ACNR_RET_SUCCESS;
}

//anr reconfig
Acnr_result_t Acnr_IQParaUpdate_V1(Acnr_Context_V1_t *pAcnrCtx)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    //need todo what?

    if(pAcnrCtx->isIQParaUpdate) {
        LOGD_ANR("IQ data reconfig\n");
        Acnr_ConfigSettingParam_V1(pAcnrCtx, pAcnrCtx->eParamMode, pAcnrCtx->stExpInfo.snr_mode);
        pAcnrCtx->isIQParaUpdate = false;
    }

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ACNR_RET_SUCCESS;
}


//anr preprocess
Acnr_result_t Acnr_PreProcess_V1(Acnr_Context_V1_t *pAcnrCtx)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    //need todo what?

    Acnr_IQParaUpdate_V1(pAcnrCtx);

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ACNR_RET_SUCCESS;
}

//anr process
Acnr_result_t Acnr_Process_V1(Acnr_Context_V1_t *pAcnrCtx, Acnr_ExpInfo_t *pExpInfo)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    Acnr_ParamMode_t mode = ACNR_PARAM_MODE_INVALID;

    if(pAcnrCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNR_RET_INVALID_PARM;
    }

    if(pExpInfo == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNR_RET_INVALID_PARM;
    }

    if(pAcnrCtx->eState != ACNR_STATE_RUNNING) {
        return ACNR_RET_SUCCESS;
    }


    Acnr_ParamModeProcess_V1(pAcnrCtx, pExpInfo, &mode);

    if(pAcnrCtx->eMode == ACNR_OP_MODE_AUTO) {

        LOGD_ANR("%s(%d): \n", __FUNCTION__, __LINE__);

#if ACNR_USE_XML_FILE_V1
        if(pExpInfo->snr_mode != pAcnrCtx->stExpInfo.snr_mode || pAcnrCtx->eParamMode != mode) {
            LOGD_ANR("param mode:%d snr_mode:%d\n", mode, pExpInfo->snr_mode);
            pAcnrCtx->eParamMode = mode;
            Acnr_ConfigSettingParam_V1(pAcnrCtx, pAcnrCtx->eParamMode, pExpInfo->snr_mode);
        }
#endif

        //select param
        cnr_select_params_by_ISO_V1(&pAcnrCtx->stAuto.stParams, &pAcnrCtx->stAuto.stSelect, pExpInfo);

    } else if(pAcnrCtx->eMode == ACNR_OP_MODE_MANUAL) {
        //TODO
    }

    memcpy(&pAcnrCtx->stExpInfo, pExpInfo, sizeof(Acnr_ExpInfo_t));
    pAcnrCtx->stExpInfo.rawHeight = pAcnrCtx->rawHeight;
    pAcnrCtx->stExpInfo.rawWidth = pAcnrCtx->rawWidth;

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ACNR_RET_SUCCESS;

}



//anr get result
Acnr_result_t Acnr_GetProcResult_V1(Acnr_Context_V1_t *pAcnrCtx, Acnr_ProcResult_V1_t* pAcnrResult)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    if(pAcnrCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNR_RET_INVALID_PARM;
    }

    if(pAcnrResult == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNR_RET_INVALID_PARM;
    }

    RK_CNR_Params_V1_Select_t* stSelect = NULL;
    if(pAcnrCtx->eMode == ACNR_OP_MODE_AUTO) {
        stSelect = &pAcnrCtx->stAuto.stSelect;
    } else if(pAcnrCtx->eMode == ACNR_OP_MODE_MANUAL) {
        //TODO
        stSelect = &pAcnrCtx->stManual.stSelect;
        pAcnrCtx->fCnr_SF_Strength = 1.0;
    }

    //transfer to reg value
    cnr_fix_transfer_V1(stSelect, pAcnrResult->stFix,  &pAcnrCtx->stExpInfo, pAcnrCtx->fCnr_SF_Strength);

    LOGD_ANR("%s:%d xml:local:%d mode:%d  reg: local gain:%d  mfnr gain:%d mode:%d\n",
             __FUNCTION__, __LINE__);

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ACNR_RET_SUCCESS;
}

Acnr_result_t Acnr_ConfigSettingParam_V1(Acnr_Context_V1_t *pAcnrCtx, Acnr_ParamMode_t eParamMode, int snr_mode)
{
    char snr_name[CALIBDB_NR_SHARP_NAME_LENGTH];
    char param_mode_name[CALIBDB_MAX_MODE_NAME_LENGTH];
    memset(param_mode_name, 0x00, sizeof(param_mode_name));
    memset(snr_name, 0x00, sizeof(snr_name));

    if(pAcnrCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNR_RET_INVALID_PARM;
    }

    //select param mode first
    if(eParamMode == ACNR_PARAM_MODE_NORMAL) {
        sprintf(param_mode_name, "%s", "normal");
    } else if(eParamMode == ACNR_PARAM_MODE_HDR) {
        sprintf(param_mode_name, "%s", "hdr");
    } else if(eParamMode == ACNR_PARAM_MODE_GRAY) {
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

#if ACNR_USE_JSON_FILE_V1
    cnr_config_setting_param_json_V1(&pAcnrCtx->stAuto.stParams, &pAcnrCtx->cnr_v1, param_mode_name, snr_name);
#else
    cnr_config_setting_param_V1(&pAcnrCtx->stAuto.stParams, pAcnrCtx->list_cnr_v1, param_mode_name, snr_name);
#endif

    return ACNR_RET_SUCCESS;
}

Acnr_result_t Acnr_ParamModeProcess_V1(Acnr_Context_V1_t *pAcnrCtx, Acnr_ExpInfo_t *pExpInfo, Acnr_ParamMode_t *mode) {
    Acnr_result_t res  = ACNR_RET_SUCCESS;

    if(pAcnrCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ACNR_RET_INVALID_PARM;
    }

    *mode = pAcnrCtx->eParamMode;

    if(pAcnrCtx->isGrayMode) {
        *mode = ACNR_PARAM_MODE_GRAY;
    } else if(pExpInfo->hdr_mode == 0) {
        *mode = ACNR_PARAM_MODE_NORMAL;
    } else if(pExpInfo->hdr_mode >= 1) {
        *mode = ACNR_PARAM_MODE_HDR;
    } else {
        *mode = ACNR_PARAM_MODE_NORMAL;
    }

    return res;
}


RKAIQ_END_DECLARE


