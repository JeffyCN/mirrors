
#include "rk_aiq_abayernr_algo_v1.h"
#include "rk_aiq_abayernr_algo_itf_v1.h"

RKAIQ_BEGIN_DECLARE

Abayernr_result_v1_t Abayernr_Start_V1(Abayernr_Context_V1_t *pAbayernrCtx)
{
    LOGI_ANR( "%s:enter!\n", __FUNCTION__);

    // initial checks
    if (pAbayernrCtx == NULL) {
        return (ABAYERNR_RET_V1_NULL_POINTER);
    }

    if ((ABAYERNR_STATE_V1_RUNNING == pAbayernrCtx->eState)
            || (ABAYERNR_STATE_V1_LOCKED == pAbayernrCtx->eState)) {
        return (ABAYERNR_RET_V1_FAILURE);
    }

    pAbayernrCtx->eState = ABAYERNR_STATE_V1_RUNNING;

    LOGI_ANR( "%s:exit!\n", __FUNCTION__);
    return (ABAYERNR_RET_V1_SUCCESS);
}


Abayernr_result_v1_t Abayernr_Stop_V1(Abayernr_Context_V1_t *pAbayernrCtx)
{
    LOGI_ANR( "%s:enter!\n", __FUNCTION__);

    // initial checks
    if (pAbayernrCtx == NULL) {
        return (ABAYERNR_RET_V1_NULL_POINTER);
    }

    if (ABAYERNR_STATE_V1_LOCKED == pAbayernrCtx->eState) {
        return (ABAYERNR_RET_V1_FAILURE);
    }

    pAbayernrCtx->eState = ABAYERNR_STATE_V1_STOPPED;

    LOGI_ANR( "%s:exit!\n", __FUNCTION__);
    return (ABAYERNR_RET_V1_SUCCESS);
}

#if ABAYERNR_USE_JSON_FILE_V1

//anr inint
Abayernr_result_v1_t Abayernr_Init_Json_V1(Abayernr_Context_V1_t **ppAbayernrCtx, CamCalibDbV2Context_t *pCalibDbV2)
{
    Abayernr_Context_V1_t * pAbayernrCtx;

    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    pAbayernrCtx = (Abayernr_Context_V1_t *)malloc(sizeof(Abayernr_Context_V1_t));
    if(pAbayernrCtx == NULL) {
        LOGE_ANR("%s(%d): malloc fail\n", __FUNCTION__, __LINE__);
        return ABAYERNR_RET_V1_NULL_POINTER;
    }

    memset(pAbayernrCtx, 0x00, sizeof(Abayernr_Context_V1_t));

    //gain state init
    pAbayernrCtx->fRawnr_SF_Strength = 1.0;

    pAbayernrCtx->eState = ABAYERNR_STATE_V1_INITIALIZED;
    *ppAbayernrCtx = pAbayernrCtx;

    pAbayernrCtx->eMode = ABAYERNR_OP_MODE_V1_AUTO;
    pAbayernrCtx->isIQParaUpdate = false;
    pAbayernrCtx->isGrayMode = false;

#if ABAYERNR_USE_JSON_FILE_V1
    //read v1 params from xml
    CalibDbV2_BayerNrV1_t* calibv2_bayernr_v1 =
        (CalibDbV2_BayerNrV1_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibDbV2, bayernr_v1));
    bayernrV1_calibdbV2_assign(&pAbayernrCtx->bayernr_v1, calibv2_bayernr_v1);

#endif

#if RK_SIMULATOR_HW
    //just for v2 params from html

#endif

#if ABAYERNR_USE_JSON_FILE_V1
    pAbayernrCtx->stExpInfo.snr_mode = 1;
    pAbayernrCtx->eParamMode = ABAYERNR_PARAM_MODE_V1_NORMAL;
    Abayernr_ConfigSettingParam_V1(pAbayernrCtx, pAbayernrCtx->eParamMode, pAbayernrCtx->stExpInfo.snr_mode);
#endif

    LOGD_ANR("%s(%d): bayernr %f %f %f %d %d %f", __FUNCTION__, __LINE__);


    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ABAYERNR_RET_V1_SUCCESS;
}

#else

//anr inint
Abayernr_result_v1_t Abayernr_Init_V1(Abayernr_Context_V1_t **ppAbayernrCtx, CamCalibDbContext_t *pCalibDb)
{
    Abayernr_Context_V1_t * pAbayernrCtx;

    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    pAbayernrCtx = (Abayernr_Context_V1_t *)malloc(sizeof(Abayernr_Context_V1_t));
    if(pAbayernrCtx == NULL) {
        LOGE_ANR("%s(%d): malloc fail\n", __FUNCTION__, __LINE__);
        return ABAYERNR_RET_V1_NULL_POINTER;
    }

    memset(pAbayernrCtx, 0x00, sizeof(Abayernr_Context_V1_t));

    //gain state init
    pAbayernrCtx->fRawnr_SF_Strength = 1.0;

    pAbayernrCtx->eState = ABAYERNR_STATE_V1_INITIALIZED;
    *ppAbayernrCtx = pAbayernrCtx;

    pAbayernrCtx->eMode = ABAYERNR_OP_MODE_V1_AUTO;
    pAbayernrCtx->isIQParaUpdate = false;
    pAbayernrCtx->isGrayMode = false;

#if ABAYERNR_USE_XML_FILE_V1
    //read v1 params from xml
    pAbayernrCtx->stBayernrCalib = *(CalibDb_BayerNr_2_t*)(CALIBDB_GET_MODULE_PTR((void*)pCalibDb, bayerNr));
#endif

#if RK_SIMULATOR_HW
    //just for v2 params from html

#endif

#if ABAYERNR_USE_XML_FILE_V1
    pAbayernrCtx->stExpInfo.snr_mode = 1;
    pAbayernrCtx->eParamMode = ABAYERNR_PARAM_MODE_V1_NORMAL;
    Abayernr_ParamModeProcess_V1(pAbayernrCtx, &pAbayernrCtx->stExpInfo, &pAbayernrCtx->eParamMode);
#endif

    LOGD_ANR("%s(%d): bayernr %f %f %f %d %d %f", __FUNCTION__, __LINE__);


    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ABAYERNR_RET_V1_SUCCESS;
}
#endif

//anr release
Abayernr_result_v1_t Abayernr_Release_V1(Abayernr_Context_V1_t *pAbayernrCtx)
{
    Abayernr_result_v1_t result = ABAYERNR_RET_V1_SUCCESS;
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    if(pAbayernrCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERNR_RET_V1_NULL_POINTER;
    }

    result = Abayernr_Stop_V1(pAbayernrCtx);
    if (result != ABAYERNR_RET_V1_SUCCESS) {
        LOGE_ANR( "%s: ANRStop() failed!\n", __FUNCTION__);
        return (result);
    }

    // check state
    if ((ABAYERNR_STATE_V1_RUNNING == pAbayernrCtx->eState)
            || (ABAYERNR_STATE_V1_LOCKED == pAbayernrCtx->eState)) {
        return (ABAYERNR_RET_V1_BUSY);
    }

#if ABAYERNR_USE_JSON_FILE_V1
    bayernrV1_calibdbV2_release(&pAbayernrCtx->bayernr_v1);
#endif

    memset(pAbayernrCtx, 0x00, sizeof(Abayernr_Context_V1_t));
    free(pAbayernrCtx);

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ABAYERNR_RET_V1_SUCCESS;
}

//anr config
Abayernr_result_v1_t Abayernr_Prepare_V1(Abayernr_Context_V1_t *pAbayernrCtx, Abayernr_Config_V1_t* pAbayernrConfig)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    if(pAbayernrCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERNR_RET_V1_INVALID_PARM;
    }

    if(pAbayernrConfig == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERNR_RET_V1_INVALID_PARM;
    }

    if(!!(pAbayernrCtx->prepare_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB)) {
        Abayernr_IQParaUpdate_V1(pAbayernrCtx);
    }

    Abayernr_Start_V1(pAbayernrCtx);

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ABAYERNR_RET_V1_SUCCESS;
}

//anr reconfig
Abayernr_result_v1_t Abayernr_ReConfig_V1(Abayernr_Context_V1_t *pAbayernrCtx, Abayernr_Config_V1_t* pAbayernrConfig)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    //need todo what?

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ABAYERNR_RET_V1_SUCCESS;
}

//anr reconfig
Abayernr_result_v1_t Abayernr_IQParaUpdate_V1(Abayernr_Context_V1_t *pAbayernrCtx)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    //need todo what?

    if(pAbayernrCtx->isIQParaUpdate) {
        LOGD_ANR("IQ data reconfig\n");
        Abayernr_ConfigSettingParam_V1(pAbayernrCtx, pAbayernrCtx->eParamMode, pAbayernrCtx->stExpInfo.snr_mode);
        pAbayernrCtx->isIQParaUpdate = false;
    }

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ABAYERNR_RET_V1_SUCCESS;
}


//anr preprocess
Abayernr_result_v1_t Abayernr_PreProcess_V1(Abayernr_Context_V1_t *pAbayernrCtx)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    //need todo what?

    Abayernr_IQParaUpdate_V1(pAbayernrCtx);

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ABAYERNR_RET_V1_SUCCESS;
}

//anr process
Abayernr_result_v1_t Abayernr_Process_V1(Abayernr_Context_V1_t *pAbayernrCtx, Abayernr_ExpInfo_V1_t *pExpInfo)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    Abayernr_ParamMode_V1_t mode = ABAYERNR_PARAM_MODE_V1_INVALID;

    if(pAbayernrCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERNR_RET_V1_INVALID_PARM;
    }

    if(pExpInfo == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERNR_RET_V1_INVALID_PARM;
    }

    if(pAbayernrCtx->eState != ABAYERNR_STATE_V1_RUNNING) {
        return ABAYERNR_RET_V1_SUCCESS;
    }


    Abayernr_ParamModeProcess_V1(pAbayernrCtx, pExpInfo, &mode);

    if(pAbayernrCtx->eMode == ABAYERNR_OP_MODE_V1_AUTO) {

        LOGD_ANR("%s(%d): \n", __FUNCTION__, __LINE__);

#if ABAYERNR_USE_XML_FILE_V1
        if(pExpInfo->snr_mode != pAbayernrCtx->stExpInfo.snr_mode || pAbayernrCtx->eParamMode != mode) {
            LOGD_ANR("param mode:%d snr_mode:%d\n", mode, pExpInfo->snr_mode);
            pAbayernrCtx->eParamMode = mode;
            Abayernr_ConfigSettingParam_V1(pAbayernrCtx, pAbayernrCtx->eParamMode, pExpInfo->snr_mode);
        }
#endif

        //select param
        select_bayernrV1_params_by_ISO(&pAbayernrCtx->stAuto.stParams, &pAbayernrCtx->stAuto.stSelect, pExpInfo);

    } else if(pAbayernrCtx->eMode == ABAYERNR_OP_MODE_V1_MANUAL) {
        //TODO
    }

    memcpy(&pAbayernrCtx->stExpInfo, pExpInfo, sizeof(Abayernr_ExpInfo_V1_t));

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ABAYERNR_RET_V1_SUCCESS;

}



//anr get result
Abayernr_result_v1_t Abayernr_GetProcResult_V1(Abayernr_Context_V1_t *pAbayernrCtx, Abayernr_ProcResult_V1_t* pAbayernrResult)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    if(pAbayernrCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERNR_RET_V1_INVALID_PARM;
    }

    if(pAbayernrResult == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERNR_RET_V1_INVALID_PARM;
    }

    if(pAbayernrCtx->eMode == ABAYERNR_OP_MODE_V1_AUTO) {
        pAbayernrResult->stSelect = pAbayernrCtx->stAuto.stSelect;
        pAbayernrResult->bayernrEn = pAbayernrCtx->stAuto.bayernrEn;

    } else if(pAbayernrCtx->eMode == ABAYERNR_OP_MODE_V1_MANUAL) {
        //TODO
        pAbayernrResult->stSelect = pAbayernrCtx->stManual.stSelect;
        pAbayernrResult->bayernrEn = pAbayernrCtx->stManual.bayernrEn;
        pAbayernrCtx->fRawnr_SF_Strength = 1.0;
    }

    //transfer to reg value
    bayernrV1_fix_tranfer(&pAbayernrResult->stSelect, &pAbayernrResult->stFix, pAbayernrCtx->fRawnr_SF_Strength);
    pAbayernrResult->stFix.rawnr_en = pAbayernrResult->bayernrEn;

    LOGD_ANR("%s:%d xml:local:%d mode:%d  reg: local gain:%d  mfnr gain:%d mode:%d\n",
             __FUNCTION__, __LINE__);

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ABAYERNR_RET_V1_SUCCESS;
}

Abayernr_result_v1_t Abayernr_ConfigSettingParam_V1(Abayernr_Context_V1_t *pAbayernrCtx, Abayernr_ParamMode_V1_t eParamMode, int snr_mode)
{
    char snr_name[CALIBDB_NR_SHARP_NAME_LENGTH];
    char param_mode_name[CALIBDB_MAX_MODE_NAME_LENGTH];
    memset(param_mode_name, 0x00, sizeof(param_mode_name));
    memset(snr_name, 0x00, sizeof(snr_name));

    if(pAbayernrCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERNR_RET_V1_INVALID_PARM;
    }

    //select param mode first
    if(eParamMode == ABAYERNR_PARAM_MODE_V1_NORMAL) {
        sprintf(param_mode_name, "%s", "normal");
    } else if(eParamMode == ABAYERNR_PARAM_MODE_V1_HDR) {
        sprintf(param_mode_name, "%s", "hdr");
    } else if(eParamMode == ABAYERNR_PARAM_MODE_V1_GRAY) {
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
        sprintf(snr_name, "%s", "HSNR");
    }

#if ABAYERNR_USE_JSON_FILE_V1
    pAbayernrCtx->stAuto.bayernrEn = pAbayernrCtx->bayernr_v1.TuningPara.enable;
    bayernrV1_config_setting_param_json(&pAbayernrCtx->stAuto.stParams, &pAbayernrCtx->bayernr_v1, param_mode_name, snr_name);
#else
    pAbayernrCtx->stAuto.bayernrEn = pAbayernrCtx->stBayernrCalib.enable;
    bayernrV1_config_setting_param(&pAbayernrCtx->stAuto.stParams, &pAbayernrCtx->stBayernrCalib, param_mode_name, snr_name);
#endif

    return ABAYERNR_RET_V1_SUCCESS;
}

Abayernr_result_v1_t Abayernr_ParamModeProcess_V1(Abayernr_Context_V1_t *pAbayernrCtx, Abayernr_ExpInfo_V1_t *pExpInfo, Abayernr_ParamMode_V1_t *mode) {
    Abayernr_result_v1_t res  = ABAYERNR_RET_V1_SUCCESS;

    if(pAbayernrCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERNR_RET_V1_INVALID_PARM;
    }

    *mode = pAbayernrCtx->eParamMode;

    if(pAbayernrCtx->isGrayMode) {
        *mode = ABAYERNR_PARAM_MODE_V1_GRAY;
    } else if(pExpInfo->hdr_mode == 0) {
        *mode = ABAYERNR_PARAM_MODE_V1_NORMAL;
    } else if(pExpInfo->hdr_mode >= 1) {
        *mode = ABAYERNR_PARAM_MODE_V1_HDR;
    } else {
        *mode = ABAYERNR_PARAM_MODE_V1_NORMAL;
    }

    return res;
}


RKAIQ_END_DECLARE


