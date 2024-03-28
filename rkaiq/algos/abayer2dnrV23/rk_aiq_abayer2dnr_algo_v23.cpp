
#include "rk_aiq_abayer2dnr_algo_v23.h"
#include "rk_aiq_abayer2dnr_algo_itf_v23.h"

RKAIQ_BEGIN_DECLARE

Abayer2dnr_result_V23_t Abayer2dnr_Start_V23(Abayer2dnr_Context_V23_t *pAbayernrCtx)
{
    LOGI_ANR( "%s:enter!\n", __FUNCTION__);

    // initial checks
    if (pAbayernrCtx == NULL) {
        return (ABAYER2DNR_V23_RET_NULL_POINTER);
    }

    if ((ABAYER2DNR_V23_STATE_RUNNING == pAbayernrCtx->eState)
            || (ABAYER2DNR_V23_STATE_LOCKED == pAbayernrCtx->eState)) {
        return (ABAYER2DNR_V23_RET_FAILURE);
    }

    pAbayernrCtx->eState = ABAYER2DNR_V23_STATE_RUNNING;

    LOGI_ANR( "%s:exit!\n", __FUNCTION__);
    return (ABAYER2DNR_V23_RET_SUCCESS);
}


Abayer2dnr_result_V23_t Abayer2dnr_Stop_V23(Abayer2dnr_Context_V23_t *pAbayernrCtx)
{
    LOGI_ANR( "%s:enter!\n", __FUNCTION__);

    // initial checks
    if (pAbayernrCtx == NULL) {
        return (ABAYER2DNR_V23_RET_NULL_POINTER);
    }

    if (ABAYER2DNR_V23_STATE_LOCKED == pAbayernrCtx->eState) {
        return (ABAYER2DNR_V23_RET_FAILURE);
    }

    pAbayernrCtx->eState = ABAYER2DNR_V23_STATE_STOPPED;

    LOGI_ANR( "%s:exit!\n", __FUNCTION__);
    return (ABAYER2DNR_V23_RET_SUCCESS);
}


//anr inint
Abayer2dnr_result_V23_t Abayer2dnr_Init_V23(Abayer2dnr_Context_V23_t **ppAbayernrCtx, void *pCalibDb)
{
    Abayer2dnr_Context_V23_t * pAbayernrCtx;

    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    pAbayernrCtx = (Abayer2dnr_Context_V23_t *)malloc(sizeof(Abayer2dnr_Context_V23_t));
    if(pAbayernrCtx == NULL) {
        LOGE_ANR("%s(%d): malloc fail\n", __FUNCTION__, __LINE__);
        return ABAYER2DNR_V23_RET_NULL_POINTER;
    }

    memset(pAbayernrCtx, 0x00, sizeof(Abayer2dnr_Context_V23_t));

    //gain state init
    pAbayernrCtx->stStrength.strength_enable = true;
    pAbayernrCtx->stStrength.percent = 1.0;

    pAbayernrCtx->eState = ABAYER2DNR_V23_STATE_INITIALIZED;
    *ppAbayernrCtx = pAbayernrCtx;

    pAbayernrCtx->eMode = ABAYER2DNR_V23_OP_MODE_AUTO;
    pAbayernrCtx->isIQParaUpdate = false;
    pAbayernrCtx->isGrayMode = false;
    pAbayernrCtx->isReCalculate = 1;

    //read v1 params from xml
#if (ABAYER2DNR_USE_JSON_FILE_V23)
    CalibDbV2_Bayer2dnrV23_t * pcalibdbV23_bayernr_v23 =
        (CalibDbV2_Bayer2dnrV23_t *)(CALIBDBV2_GET_MODULE_PTR((CamCalibDbV2Context_t*)pCalibDb, bayer2dnr_v23));
    pAbayernrCtx->bayernr_v23 = *pcalibdbV23_bayernr_v23;
#endif


#if 1
    pAbayernrCtx->stExpInfo.snr_mode = 1;
    pAbayernrCtx->eParamMode = ABAYER2DNR_V23_PARAM_MODE_NORMAL;
    Abayer2dnr_ConfigSettingParam_V23(pAbayernrCtx, pAbayernrCtx->eParamMode, pAbayernrCtx->stExpInfo.snr_mode);
#endif

    LOGD_ANR("%s(%d):", __FUNCTION__, __LINE__);


    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ABAYER2DNR_V23_RET_SUCCESS;
}

//anr release
Abayer2dnr_result_V23_t Abayer2dnr_Release_V23(Abayer2dnr_Context_V23_t *pAbayernrCtx)
{
    Abayer2dnr_result_V23_t result = ABAYER2DNR_V23_RET_SUCCESS;
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    if(pAbayernrCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYER2DNR_V23_RET_NULL_POINTER;
    }

    result = Abayer2dnr_Stop_V23(pAbayernrCtx);
    if (result != ABAYER2DNR_V23_RET_SUCCESS) {
        LOGE_ANR( "%s: ANRStop() failed!\n", __FUNCTION__);
        return (result);
    }

    // check state
    if ((ABAYER2DNR_V23_STATE_RUNNING == pAbayernrCtx->eState)
            || (ABAYER2DNR_V23_STATE_LOCKED == pAbayernrCtx->eState)) {
        return (ABAYER2DNR_V23_RET_BUSY);
    }

    memset(pAbayernrCtx, 0x00, sizeof(Abayer2dnr_Context_V23_t));
    free(pAbayernrCtx);

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ABAYER2DNR_V23_RET_SUCCESS;
}

//anr config
Abayer2dnr_result_V23_t Abayer2dnr_Prepare_V23(Abayer2dnr_Context_V23_t *pAbayernrCtx, Abayer2dnr_Config_V23_t* pAbayernrConfig)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    if(pAbayernrCtx == NULL || pAbayernrConfig == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYER2DNR_V23_RET_INVALID_PARM;
    }

    if(!!(pAbayernrCtx->prepare_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB)) {
        Abayer2dnr_IQParaUpdate_V23(pAbayernrCtx);
    }

    Abayer2dnr_Start_V23(pAbayernrCtx);

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ABAYER2DNR_V23_RET_SUCCESS;
}

//anr reconfig
Abayer2dnr_result_V23_t Abayer2dnr_ReConfig_V23(Abayer2dnr_Context_V23_t *pAbayernrCtx, Abayer2dnr_Config_V23_t* pAbayernrConfig)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    //need todo what?

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ABAYER2DNR_V23_RET_SUCCESS;
}

//anr reconfig
Abayer2dnr_result_V23_t Abayer2dnr_IQParaUpdate_V23(Abayer2dnr_Context_V23_t *pAbayernrCtx)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    //need todo what?

    if(pAbayernrCtx->isIQParaUpdate) {
        LOGD_ANR("IQ data reconfig\n");
        Abayer2dnr_ConfigSettingParam_V23(pAbayernrCtx, pAbayernrCtx->eParamMode, pAbayernrCtx->stExpInfo.snr_mode);
        pAbayernrCtx->isIQParaUpdate = false;
    }

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ABAYER2DNR_V23_RET_SUCCESS;
}


//anr preprocess
Abayer2dnr_result_V23_t Abayer2dnr_PreProcess_V23(Abayer2dnr_Context_V23_t *pAbayernrCtx)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    //need todo what?

    Abayer2dnr_IQParaUpdate_V23(pAbayernrCtx);

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ABAYER2DNR_V23_RET_SUCCESS;
}

//anr process
Abayer2dnr_result_V23_t Abayer2dnr_Process_V23(Abayer2dnr_Context_V23_t *pAbayernrCtx, Abayer2dnr_ExpInfo_V23_t *pExpInfo)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    Abayer2dnr_ParamMode_V23_t mode = ABAYER2DNR_V23_PARAM_MODE_INVALID;

    if(pAbayernrCtx == NULL || pExpInfo == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYER2DNR_V23_RET_INVALID_PARM;
    }

    if(pAbayernrCtx->eState != ABAYER2DNR_V23_STATE_RUNNING) {
        return ABAYER2DNR_V23_RET_SUCCESS;
    }

    Abayer2dnr_ParamModeProcess_V23(pAbayernrCtx, pExpInfo, &mode);

    if(pAbayernrCtx->eMode == ABAYER2DNR_V23_OP_MODE_AUTO) {

#if ABAYER2DNR_USE_JSON_FILE_V23
        if(pExpInfo->snr_mode != pAbayernrCtx->stExpInfo.snr_mode || pAbayernrCtx->eParamMode != mode) {
            LOGD_ANR("param mode:%d snr_mode:%d\n", mode, pExpInfo->snr_mode);
            pAbayernrCtx->eParamMode = mode;
            Abayer2dnr_ConfigSettingParam_V23(pAbayernrCtx, pAbayernrCtx->eParamMode, pExpInfo->snr_mode);
        }
#endif

        //select param
        bayer2dnr_select_params_by_ISO_V23(&pAbayernrCtx->stAuto.st2DParams, &pAbayernrCtx->stAuto.st2DSelect, pExpInfo);
    } else if(pAbayernrCtx->eMode == ABAYER2DNR_V23_OP_MODE_MANUAL) {
        //TODO
    }

    memcpy(&pAbayernrCtx->stExpInfo, pExpInfo, sizeof(Abayer2dnr_ExpInfo_V23_t));

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ABAYER2DNR_V23_RET_SUCCESS;

}



//anr get result
Abayer2dnr_result_V23_t Abayer2dnr_GetProcResult_V23(Abayer2dnr_Context_V23_t *pAbayernrCtx, Abayer2dnr_ProcResult_V23_t* pAbayernrResult)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    if(pAbayernrCtx == NULL || pAbayernrResult == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYER2DNR_V23_RET_INVALID_PARM;
    }

    RK_Bayer2dnrV23_Params_Select_t* st2DSelect = NULL;
    if(pAbayernrCtx->eMode == ABAYER2DNR_V23_OP_MODE_AUTO) {
        st2DSelect = &pAbayernrCtx->stAuto.st2DSelect;
    } else if(pAbayernrCtx->eMode == ABAYER2DNR_V23_OP_MODE_MANUAL) {
        st2DSelect = &pAbayernrCtx->stManual.st2DSelect;
    }

    //transfer to reg value
    bayer2dnr_fix_transfer_V23(st2DSelect, pAbayernrResult->st2DFix, &pAbayernrCtx->stStrength, &pAbayernrCtx->stExpInfo);

    if(pAbayernrCtx->eMode == ABAYER2DNR_V23_OP_MODE_REG_MANUAL) {
        *pAbayernrResult->st2DFix = pAbayernrCtx->stManual.st2Dfix;
        pAbayernrCtx->stStrength.percent = 1.0;
    }

    LOGD_ANR("%s:%d xml:local:%d mode:%d  reg: local gain:%d  mfnr gain:%d mode:%d\n",
             __FUNCTION__, __LINE__);

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ABAYER2DNR_V23_RET_SUCCESS;
}

Abayer2dnr_result_V23_t Abayer2dnr_ConfigSettingParam_V23(Abayer2dnr_Context_V23_t *pAbayernrCtx, Abayer2dnr_ParamMode_V23_t eParamMode, int snr_mode)
{
    char snr_name[CALIBDB_NR_SHARP_NAME_LENGTH];
    char param_mode_name[CALIBDB_MAX_MODE_NAME_LENGTH];
    memset(param_mode_name, 0x00, sizeof(param_mode_name));
    memset(snr_name, 0x00, sizeof(snr_name));

    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    if(pAbayernrCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYER2DNR_V23_RET_INVALID_PARM;
    }

    //select param mode first
    if(eParamMode == ABAYER2DNR_V23_PARAM_MODE_NORMAL) {
        sprintf(param_mode_name, "%s", "normal");
    } else if(eParamMode == ABAYER2DNR_V23_PARAM_MODE_HDR) {
        sprintf(param_mode_name, "%s", "hdr");
    } else if(eParamMode == ABAYER2DNR_V23_PARAM_MODE_GRAY) {
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

#if (ABAYER2DNR_USE_JSON_FILE_V23)
    bayer2dnr_config_setting_param_json_V23(&pAbayernrCtx->stAuto.st2DParams, &pAbayernrCtx->bayernr_v23, param_mode_name, snr_name);
    // init 2dnr manual params
    pAbayernrCtx->stManual.st2DSelect = pAbayernrCtx->stAuto.st2DParams.Bayer2dnrParamsISO[0];
#endif
    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ABAYER2DNR_V23_RET_SUCCESS;
}

Abayer2dnr_result_V23_t Abayer2dnr_ParamModeProcess_V23(Abayer2dnr_Context_V23_t *pAbayernrCtx, Abayer2dnr_ExpInfo_V23_t *pExpInfo, Abayer2dnr_ParamMode_V23_t *mode) {
    Abayer2dnr_result_V23_t res  = ABAYER2DNR_V23_RET_SUCCESS;

    if(pAbayernrCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYER2DNR_V23_RET_INVALID_PARM;
    }

    *mode = pAbayernrCtx->eParamMode;

    if(pAbayernrCtx->isGrayMode) {
        *mode = ABAYER2DNR_V23_PARAM_MODE_GRAY;
    } else if(pExpInfo->hdr_mode == 0) {
        *mode = ABAYER2DNR_V23_PARAM_MODE_NORMAL;
    } else if(pExpInfo->hdr_mode >= 1) {
        *mode = ABAYER2DNR_V23_PARAM_MODE_HDR;
    } else {
        *mode = ABAYER2DNR_V23_PARAM_MODE_NORMAL;
    }

    return res;
}


RKAIQ_END_DECLARE


