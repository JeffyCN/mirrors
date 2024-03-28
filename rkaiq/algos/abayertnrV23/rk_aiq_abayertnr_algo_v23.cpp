
#include "rk_aiq_abayertnr_algo_v23.h"
#include "rk_aiq_abayertnr_algo_itf_v23.h"

RKAIQ_BEGIN_DECLARE

Abayertnr_result_V23_t Abayertnr_Start_V23(Abayertnr_Context_V23_t *pAbayertnrCtx)
{
    LOGI_ANR( "%s:enter!\n", __FUNCTION__);

    // initial checks
    if (pAbayertnrCtx == NULL) {
        return (ABAYERTNRV23_RET_NULL_POINTER);
    }

    if ((ABAYERTNRV23_STATE_RUNNING == pAbayertnrCtx->eState)
            || (ABAYERTNRV23_STATE_LOCKED == pAbayertnrCtx->eState)) {
        return (ABAYERTNRV23_RET_FAILURE);
    }

    pAbayertnrCtx->eState = ABAYERTNRV23_STATE_RUNNING;

    LOGI_ANR( "%s:exit!\n", __FUNCTION__);
    return (ABAYERTNRV23_RET_SUCCESS);
}


Abayertnr_result_V23_t Abayertnr_Stop_V23(Abayertnr_Context_V23_t *pAbayertnrCtx)
{
    LOGI_ANR( "%s:enter!\n", __FUNCTION__);

    // initial checks
    if (pAbayertnrCtx == NULL) {
        return (ABAYERTNRV23_RET_NULL_POINTER);
    }

    if (ABAYERTNRV23_STATE_LOCKED == pAbayertnrCtx->eState) {
        return (ABAYERTNRV23_RET_FAILURE);
    }

    pAbayertnrCtx->eState = ABAYERTNRV23_STATE_STOPPED;

    LOGI_ANR( "%s:exit!\n", __FUNCTION__);
    return (ABAYERTNRV23_RET_SUCCESS);
}


//anr inint
Abayertnr_result_V23_t Abayertnr_Init_V23(Abayertnr_Context_V23_t **ppAbayertnrCtx, void *pCalibDb)
{
    Abayertnr_Context_V23_t * pAbayertnrCtx;

    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    pAbayertnrCtx = (Abayertnr_Context_V23_t *)malloc(sizeof(Abayertnr_Context_V23_t));
    if(pAbayertnrCtx == NULL) {
        LOGE_ANR("%s(%d): malloc fail\n", __FUNCTION__, __LINE__);
        return ABAYERTNRV23_RET_NULL_POINTER;
    }

    memset(pAbayertnrCtx, 0x00, sizeof(Abayertnr_Context_V23_t));

    //gain state init
    pAbayertnrCtx->stStrength.strength_enable = true;
    pAbayertnrCtx->stStrength.percent = 1.0;

    pAbayertnrCtx->eState = ABAYERTNRV23_STATE_INITIALIZED;
    *ppAbayertnrCtx = pAbayertnrCtx;

    pAbayertnrCtx->eMode = ABAYERTNRV23_OP_MODE_AUTO;
    pAbayertnrCtx->isIQParaUpdate = false;
    pAbayertnrCtx->isGrayMode = false;
    pAbayertnrCtx->isReCalculate = 1;

    //read v1 params from xml
#if (ABAYERTNR_USE_JSON_FILE_V23)
#if (RKAIQ_HAVE_BAYERTNR_V23)
    CalibDbV2_BayerTnrV23_t * pcalibdbV23_bayertnr_v23 =
        (CalibDbV2_BayerTnrV23_t *)(CALIBDBV2_GET_MODULE_PTR((CamCalibDbV2Context_t*)pCalibDb, bayertnr_v23));
#else
    CalibDbV2_BayerTnrV23Lite_t* pcalibdbV23_bayertnr_v23 =
        (CalibDbV2_BayerTnrV23Lite_t*)(CALIBDBV2_GET_MODULE_PTR((CamCalibDbV2Context_t*)pCalibDb,
                                                                bayertnr_v23));
#endif
    pAbayertnrCtx->bayertnr_v23 = *pcalibdbV23_bayertnr_v23;
#endif


#if RK_SIMULATOR_HW
    //just for v23 params from html

#endif

#if 1
    pAbayertnrCtx->stExpInfo.snr_mode = 1;
    pAbayertnrCtx->eParamMode = ABAYERTNRV23_PARAM_MODE_NORMAL;
    Abayertnr_ConfigSettingParam_V23(pAbayertnrCtx, pAbayertnrCtx->eParamMode, pAbayertnrCtx->stExpInfo.snr_mode);
#endif

    LOGD_ANR("%s(%d):", __FUNCTION__, __LINE__);


    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ABAYERTNRV23_RET_SUCCESS;
}

//anr release
Abayertnr_result_V23_t Abayertnr_Release_V23(Abayertnr_Context_V23_t *pAbayertnrCtx)
{
    Abayertnr_result_V23_t result = ABAYERTNRV23_RET_SUCCESS;
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    if(pAbayertnrCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERTNRV23_RET_NULL_POINTER;
    }

    result = Abayertnr_Stop_V23(pAbayertnrCtx);
    if (result != ABAYERTNRV23_RET_SUCCESS) {
        LOGE_ANR( "%s: ANRStop() failed!\n", __FUNCTION__);
        return (result);
    }

    // check state
    if ((ABAYERTNRV23_STATE_RUNNING == pAbayertnrCtx->eState)
            || (ABAYERTNRV23_STATE_LOCKED == pAbayertnrCtx->eState)) {
        return (ABAYERTNRV23_RET_BUSY);
    }

    memset(pAbayertnrCtx, 0x00, sizeof(Abayertnr_Context_V23_t));
    free(pAbayertnrCtx);

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ABAYERTNRV23_RET_SUCCESS;
}

//anr config
Abayertnr_result_V23_t Abayertnr_Prepare_V23(Abayertnr_Context_V23_t *pAbayertnrCtx, Abayertnr_Config_V23_t* pAbayertnrConfig)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    if(pAbayertnrCtx == NULL || pAbayertnrConfig == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERTNRV23_RET_INVALID_PARM;
    }

    if(!!(pAbayertnrCtx->prepare_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB)) {
        Abayertnr_IQParaUpdate_V23(pAbayertnrCtx);
    }

    Abayertnr_Start_V23(pAbayertnrCtx);

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ABAYERTNRV23_RET_SUCCESS;
}

//anr reconfig
Abayertnr_result_V23_t Abayertnr_ReConfig_V23(Abayertnr_Context_V23_t *pAbayertnrCtx, Abayertnr_Config_V23_t* pAbayertnrConfig)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    //need todo what?

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ABAYERTNRV23_RET_SUCCESS;
}

//anr reconfig
Abayertnr_result_V23_t Abayertnr_IQParaUpdate_V23(Abayertnr_Context_V23_t *pAbayertnrCtx)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    //need todo what?

    if(pAbayertnrCtx->isIQParaUpdate) {
        LOGD_ANR("IQ data reconfig\n");
        Abayertnr_ConfigSettingParam_V23(pAbayertnrCtx, pAbayertnrCtx->eParamMode, pAbayertnrCtx->stExpInfo.snr_mode);
        pAbayertnrCtx->isIQParaUpdate = false;
    }

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ABAYERTNRV23_RET_SUCCESS;
}


//anr preprocess
Abayertnr_result_V23_t Abayertnr_PreProcess_V23(Abayertnr_Context_V23_t *pAbayertnrCtx)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    //need todo what?

    Abayertnr_IQParaUpdate_V23(pAbayertnrCtx);

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ABAYERTNRV23_RET_SUCCESS;
}

//anr process
Abayertnr_result_V23_t Abayertnr_Process_V23(Abayertnr_Context_V23_t *pAbayertnrCtx, Abayertnr_ExpInfo_V23_t *pExpInfo)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    Abayertnr_ParamMode_V23_t mode = ABAYERTNRV23_PARAM_MODE_INVALID;

    if(pAbayertnrCtx == NULL || pExpInfo == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERTNRV23_RET_INVALID_PARM;
    }

    if(pAbayertnrCtx->eState != ABAYERTNRV23_STATE_RUNNING) {
        return ABAYERTNRV23_RET_SUCCESS;
    }

    Abayertnr_ParamModeProcess_V23(pAbayertnrCtx, pExpInfo, &mode);

    if(pAbayertnrCtx->eMode == ABAYERTNRV23_OP_MODE_AUTO) {

        LOGD_ANR("%s(%d): \n", __FUNCTION__, __LINE__);

#if ABAYERTNR_USE_XML_FILE_V23
        if(pExpInfo->snr_mode != pAbayertnrCtx->stExpInfo.snr_mode || pAbayertnrCtx->eParamMode != mode) {
            LOGD_ANR("param mode:%d snr_mode:%d\n", mode, pExpInfo->snr_mode);
            pAbayertnrCtx->eParamMode = mode;
            Abayertnr_ConfigSettingParam_V23(pAbayertnrCtx, pAbayertnrCtx->eParamMode, pExpInfo->snr_mode);
        }
#endif

        //select param
        bayertnr_select_params_by_ISO_V23(&pAbayertnrCtx->stAuto.st3DParams, &pAbayertnrCtx->stAuto.st3DSelect, pExpInfo);

    } else if(pAbayertnrCtx->eMode == ABAYERTNRV23_OP_MODE_MANUAL) {
        //TODO
    }

    memcpy(&pAbayertnrCtx->stExpInfo, pExpInfo, sizeof(Abayertnr_ExpInfo_V23_t));

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ABAYERTNRV23_RET_SUCCESS;

}



//anr get result
Abayertnr_result_V23_t Abayertnr_GetProcResult_V23(Abayertnr_Context_V23_t *pAbayertnrCtx, Abayertnr_ProcResult_V23_t* pAbayertnrResult)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    if(pAbayertnrCtx == NULL || pAbayertnrResult == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERTNRV23_RET_INVALID_PARM;
    }


#if (RKAIQ_HAVE_BAYERTNR_V23)
    RK_Bayertnr_Params_V23_Select_t* st3DSelect = NULL;
#else
    RK_Bayertnr_Param_V23L_Select_t* st3DSelect = NULL;
#endif
    if(pAbayertnrCtx->eMode == ABAYERTNRV23_OP_MODE_AUTO) {
        st3DSelect = &pAbayertnrCtx->stAuto.st3DSelect;

    } else if(pAbayertnrCtx->eMode == ABAYERTNRV23_OP_MODE_MANUAL) {
        //TODO
        st3DSelect = &pAbayertnrCtx->stManual.st3DSelect;
    }

    //transfer to reg value
    bayertnr_fix_transfer_V23(st3DSelect, pAbayertnrResult->st3DFix, &pAbayertnrCtx->stStrength, &pAbayertnrCtx->stExpInfo);

    if(pAbayertnrCtx->eMode == ABAYERTNRV23_OP_MODE_REG_MANUAL) {
        *pAbayertnrResult->st3DFix = pAbayertnrCtx->stManual.st3DFix;
        pAbayertnrCtx->stStrength.percent = 1.0;
    }

    LOGD_ANR("%s:%d abayertnr eMode:%d bypass:%d iso:%d fstrength:%f\n",
             __FUNCTION__, __LINE__,
             pAbayertnrCtx->eMode,
             pAbayertnrResult->st3DFix->bypass_en,
             pAbayertnrCtx->stExpInfo.arIso[pAbayertnrCtx->stExpInfo.hdr_mode],
             pAbayertnrCtx->stStrength.percent);

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ABAYERTNRV23_RET_SUCCESS;
}

Abayertnr_result_V23_t Abayertnr_ConfigSettingParam_V23(Abayertnr_Context_V23_t *pAbayertnrCtx, Abayertnr_ParamMode_V23_t eParamMode, int snr_mode)
{
    char snr_name[CALIBDB_NR_SHARP_NAME_LENGTH];
    char param_mode_name[CALIBDB_MAX_MODE_NAME_LENGTH];
    memset(param_mode_name, 0x00, sizeof(param_mode_name));
    memset(snr_name, 0x00, sizeof(snr_name));

    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    if(pAbayertnrCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERTNRV23_RET_INVALID_PARM;
    }

    //select param mode first
    if(eParamMode == ABAYERTNRV23_PARAM_MODE_NORMAL) {
        sprintf(param_mode_name, "%s", "normal");
    } else if(eParamMode == ABAYERTNRV23_PARAM_MODE_HDR) {
        sprintf(param_mode_name, "%s", "hdr");
    } else if(eParamMode == ABAYERTNRV23_PARAM_MODE_GRAY) {
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

#if (ABAYERTNR_USE_JSON_FILE_V23)
    bayertnr_config_setting_param_json_V23(&pAbayertnrCtx->stAuto.st3DParams, &pAbayertnrCtx->bayertnr_v23, param_mode_name, snr_name);
    // init tnr manual params
    pAbayertnrCtx->stManual.st3DSelect = pAbayertnrCtx->stAuto.st3DParams.bayertnrParamISO[0];
#endif
    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ABAYERTNRV23_RET_SUCCESS;
}

Abayertnr_result_V23_t Abayertnr_ParamModeProcess_V23(Abayertnr_Context_V23_t *pAbayertnrCtx, Abayertnr_ExpInfo_V23_t *pExpInfo, Abayertnr_ParamMode_V23_t *mode) {
    Abayertnr_result_V23_t res  = ABAYERTNRV23_RET_SUCCESS;

    if(pAbayertnrCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERTNRV23_RET_INVALID_PARM;
    }

    *mode = pAbayertnrCtx->eParamMode;

    if(pAbayertnrCtx->isGrayMode) {
        *mode = ABAYERTNRV23_PARAM_MODE_GRAY;
    } else if(pExpInfo->hdr_mode == 0) {
        *mode = ABAYERTNRV23_PARAM_MODE_NORMAL;
    } else if(pExpInfo->hdr_mode >= 1) {
        *mode = ABAYERTNRV23_PARAM_MODE_HDR;
    } else {
        *mode = ABAYERTNRV23_PARAM_MODE_NORMAL;
    }

    return res;
}


RKAIQ_END_DECLARE


