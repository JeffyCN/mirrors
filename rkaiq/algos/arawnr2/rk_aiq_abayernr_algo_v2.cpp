
#include "rk_aiq_abayernr_algo_v2.h"
#include "rk_aiq_abayernr_algo_itf_v2.h"
#include "RkAiqCalibDbV2Helper.h"

RKAIQ_BEGIN_DECLARE

Abayernr_result_t Abayernr_Start_V2(Abayernr_Context_V2_t *pAbayernrCtx)
{
    LOGI_ANR( "%s:enter!\n", __FUNCTION__);

    // initial checks
    if (pAbayernrCtx == NULL) {
        return (ABAYERNR_RET_NULL_POINTER);
    }

    if ((ABAYERNR_STATE_RUNNING == pAbayernrCtx->eState)
            || (ABAYERNR_STATE_LOCKED == pAbayernrCtx->eState)) {
        return (ABAYERNR_RET_FAILURE);
    }

    pAbayernrCtx->eState = ABAYERNR_STATE_RUNNING;

    LOGI_ANR( "%s:exit!\n", __FUNCTION__);
    return (ABAYERNR_RET_SUCCESS);
}


Abayernr_result_t Abayernr_Stop_V2(Abayernr_Context_V2_t *pAbayernrCtx)
{
    LOGI_ANR( "%s:enter!\n", __FUNCTION__);

    // initial checks
    if (pAbayernrCtx == NULL) {
        return (ABAYERNR_RET_NULL_POINTER);
    }

    if (ABAYERNR_STATE_LOCKED == pAbayernrCtx->eState) {
        return (ABAYERNR_RET_FAILURE);
    }

    pAbayernrCtx->eState = ABAYERNR_STATE_STOPPED;

    LOGI_ANR( "%s:exit!\n", __FUNCTION__);
    return (ABAYERNR_RET_SUCCESS);
}


//anr inint
Abayernr_result_t Abayernr_Init_V2(Abayernr_Context_V2_t **ppAbayernrCtx, void *pCalibDb)
{
    Abayernr_Context_V2_t * pAbayernrCtx;

    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    pAbayernrCtx = (Abayernr_Context_V2_t *)malloc(sizeof(Abayernr_Context_V2_t));
    if(pAbayernrCtx == NULL) {
        LOGE_ANR("%s(%d): malloc fail\n", __FUNCTION__, __LINE__);
        return ABAYERNR_RET_NULL_POINTER;
    }

    memset(pAbayernrCtx, 0x00, sizeof(Abayernr_Context_V2_t));

    //gain state init
    pAbayernrCtx->fRawnr_SF_Strength = 1.0;
    pAbayernrCtx->fRawnr_TF_Strength = 1.0;

    pAbayernrCtx->eState = ABAYERNR_STATE_INITIALIZED;
    *ppAbayernrCtx = pAbayernrCtx;

    pAbayernrCtx->eMode = ABAYERNR_OP_MODE_AUTO;
    pAbayernrCtx->isIQParaUpdate = false;
    pAbayernrCtx->isGrayMode = false;
    pAbayernrCtx->isReCalculate = 1;

#if 1
    //read v1 params from xml
#if (ABAYERNR_USE_JSON_FILE_V2)
    CalibDbV2_BayerNrV2_t * pcalibdbV2_bayernr_v2 =
        (CalibDbV2_BayerNrV2_t *)(CALIBDBV2_GET_MODULE_PTR((CamCalibDbV2Context_t*)pCalibDb, bayernr_v2));
    pAbayernrCtx->bayernr_v2 = *pcalibdbV2_bayernr_v2;
#else
    pAbayernrCtx->list_bayernr_v2 =
        (struct list_head*)(CALIBDB_GET_MODULE_PTR((CamCalibDbContext_t *)pCalibDb, list_bayernr_v2));
    printf("%s(%d): bayernr list:%p\n", __FUNCTION__, __LINE__, pAbayernrCtx->list_bayernr_v2);
#endif
#endif

#if RK_SIMULATOR_HW
    //just for v2 params from html

#endif

#if 1
    pAbayernrCtx->stExpInfo.snr_mode = 1;
    pAbayernrCtx->eParamMode = ABAYERNR_PARAM_MODE_NORMAL;
    Abayernr_ConfigSettingParam_V2(pAbayernrCtx, pAbayernrCtx->eParamMode, pAbayernrCtx->stExpInfo.snr_mode);
#endif

    LOGD_ANR("%s(%d):", __FUNCTION__, __LINE__);


    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ABAYERNR_RET_SUCCESS;
}

//anr release
Abayernr_result_t Abayernr_Release_V2(Abayernr_Context_V2_t *pAbayernrCtx)
{
    Abayernr_result_t result = ABAYERNR_RET_SUCCESS;
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    if(pAbayernrCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERNR_RET_NULL_POINTER;
    }

    result = Abayernr_Stop_V2(pAbayernrCtx);
    if (result != ABAYERNR_RET_SUCCESS) {
        LOGE_ANR( "%s: ANRStop() failed!\n", __FUNCTION__);
        return (result);
    }

    // check state
    if ((ABAYERNR_STATE_RUNNING == pAbayernrCtx->eState)
            || (ABAYERNR_STATE_LOCKED == pAbayernrCtx->eState)) {
        return (ABAYERNR_RET_BUSY);
    }

    memset(pAbayernrCtx, 0x00, sizeof(Abayernr_Context_V2_t));
    free(pAbayernrCtx);

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ABAYERNR_RET_SUCCESS;
}

//anr config
Abayernr_result_t Abayernr_Prepare_V2(Abayernr_Context_V2_t *pAbayernrCtx, Abayernr_Config_V2_t* pAbayernrConfig)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    if(pAbayernrCtx == NULL || pAbayernrConfig == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERNR_RET_INVALID_PARM;
    }

    if(!!(pAbayernrCtx->prepare_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB)) {
        Abayernr_IQParaUpdate_V2(pAbayernrCtx);
    }

    Abayernr_Start_V2(pAbayernrCtx);

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ABAYERNR_RET_SUCCESS;
}

//anr reconfig
Abayernr_result_t Abayernr_ReConfig_V2(Abayernr_Context_V2_t *pAbayernrCtx, Abayernr_Config_V2_t* pAbayernrConfig)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    //need todo what?

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ABAYERNR_RET_SUCCESS;
}

//anr reconfig
Abayernr_result_t Abayernr_IQParaUpdate_V2(Abayernr_Context_V2_t *pAbayernrCtx)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    //need todo what?

    if(pAbayernrCtx->isIQParaUpdate) {
        LOGD_ANR("IQ data reconfig\n");
        Abayernr_ConfigSettingParam_V2(pAbayernrCtx, pAbayernrCtx->eParamMode, pAbayernrCtx->stExpInfo.snr_mode);
        pAbayernrCtx->isIQParaUpdate = false;
    }

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ABAYERNR_RET_SUCCESS;
}


//anr preprocess
Abayernr_result_t Abayernr_PreProcess_V2(Abayernr_Context_V2_t *pAbayernrCtx)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    //need todo what?

    Abayernr_IQParaUpdate_V2(pAbayernrCtx);

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ABAYERNR_RET_SUCCESS;
}

//anr process
Abayernr_result_t Abayernr_Process_V2(Abayernr_Context_V2_t *pAbayernrCtx, Abayernr_ExpInfo_t *pExpInfo)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    Abayernr_ParamMode_t mode = ABAYERNR_PARAM_MODE_INVALID;

    if(pAbayernrCtx == NULL || pExpInfo == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERNR_RET_INVALID_PARM;
    }

    if(pAbayernrCtx->eState != ABAYERNR_STATE_RUNNING) {
        return ABAYERNR_RET_SUCCESS;
    }

    Abayernr_ParamModeProcess_V2(pAbayernrCtx, pExpInfo, &mode);

    if(pAbayernrCtx->eMode == ABAYERNR_OP_MODE_AUTO) {

        LOGD_ANR("%s(%d): \n", __FUNCTION__, __LINE__);

#if ABAYERNR_USE_XML_FILE_V2
        if(pExpInfo->snr_mode != pAbayernrCtx->stExpInfo.snr_mode || pAbayernrCtx->eParamMode != mode) {
            LOGD_ANR("param mode:%d snr_mode:%d\n", mode, pExpInfo->snr_mode);
            pAbayernrCtx->eParamMode = mode;
            Abayernr_ConfigSettingParam_V2(pAbayernrCtx, pAbayernrCtx->eParamMode, pExpInfo->snr_mode);
        }
#endif

        //select param
        bayernr2D_select_params_by_ISO_V2(&pAbayernrCtx->stAuto.st2DParams, &pAbayernrCtx->stAuto.st2DSelect, pExpInfo);
        bayernr3D_select_params_by_ISO_V2(&pAbayernrCtx->stAuto.st3DParams, &pAbayernrCtx->stAuto.st3DSelect, pExpInfo);

    } else if(pAbayernrCtx->eMode == ABAYERNR_OP_MODE_MANUAL) {
        //TODO
    }

    memcpy(&pAbayernrCtx->stExpInfo, pExpInfo, sizeof(Abayernr_ExpInfo_t));

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ABAYERNR_RET_SUCCESS;

}



//anr get result
Abayernr_result_t Abayernr_GetProcResult_V2(Abayernr_Context_V2_t *pAbayernrCtx, Abayernr_ProcResult_V2_t* pAbayernrResult)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    if(pAbayernrCtx == NULL || pAbayernrResult == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERNR_RET_INVALID_PARM;
    }

    RK_Bayernr_2D_Params_V2_Select_t* st2DSelect = NULL;
    RK_Bayernr_3D_Params_V2_Select_t* st3DSelect = NULL;
    if(pAbayernrCtx->eMode == ABAYERNR_OP_MODE_AUTO) {
        st2DSelect = &pAbayernrCtx->stAuto.st2DSelect;
        st3DSelect = &pAbayernrCtx->stAuto.st3DSelect;
    } else if(pAbayernrCtx->eMode == ABAYERNR_OP_MODE_MANUAL) {
        //TODO
        st2DSelect = &pAbayernrCtx->stManual.st2DSelect;
        st3DSelect = &pAbayernrCtx->stManual.st3DSelect;
        pAbayernrCtx->fRawnr_SF_Strength = 1.0;
        pAbayernrCtx->fRawnr_TF_Strength = 1.0;
    }

    //transfer to reg value
    bayernr2D_fix_transfer_V2(st2DSelect, pAbayernrResult->st2DFix, pAbayernrCtx->fRawnr_SF_Strength, &pAbayernrCtx->stExpInfo);
    bayernr3D_fix_transfer_V2(st3DSelect, pAbayernrResult->st3DFix, pAbayernrCtx->fRawnr_TF_Strength, &pAbayernrCtx->stExpInfo);

    LOGD_ANR("%s:%d xml:local:%d mode:%d  reg: local gain:%d  mfnr gain:%d mode:%d\n",
             __FUNCTION__, __LINE__);

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ABAYERNR_RET_SUCCESS;
}

Abayernr_result_t Abayernr_ConfigSettingParam_V2(Abayernr_Context_V2_t *pAbayernrCtx, Abayernr_ParamMode_t eParamMode, int snr_mode)
{
    char snr_name[CALIBDB_NR_SHARP_NAME_LENGTH];
    char param_mode_name[CALIBDB_MAX_MODE_NAME_LENGTH];
    memset(param_mode_name, 0x00, sizeof(param_mode_name));
    memset(snr_name, 0x00, sizeof(snr_name));

    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    if(pAbayernrCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERNR_RET_INVALID_PARM;
    }

    //select param mode first
    if(eParamMode == ABAYERNR_PARAM_MODE_NORMAL) {
        sprintf(param_mode_name, "%s", "normal");
    } else if(eParamMode == ABAYERNR_PARAM_MODE_HDR) {
        sprintf(param_mode_name, "%s", "hdr");
    } else if(eParamMode == ABAYERNR_PARAM_MODE_GRAY) {
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

#if (ABAYERNR_USE_JSON_FILE_V2)
    bayernr2D_config_setting_param_json_V2(&pAbayernrCtx->stAuto.st2DParams, &pAbayernrCtx->bayernr_v2, param_mode_name, snr_name);
    bayernr3D_config_setting_param_json_V2(&pAbayernrCtx->stAuto.st3DParams, &pAbayernrCtx->bayernr_v2, param_mode_name, snr_name);
#else
    bayernr2D_config_setting_param_V2(&pAbayernrCtx->stAuto.st2DParams, pAbayernrCtx->list_bayernr_v2, param_mode_name, snr_name);
    bayernr3D_config_setting_param_V2(&pAbayernrCtx->stAuto.st3DParams, pAbayernrCtx->list_bayernr_v2, param_mode_name, snr_name);
#endif
    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ABAYERNR_RET_SUCCESS;
}

Abayernr_result_t Abayernr_ParamModeProcess_V2(Abayernr_Context_V2_t *pAbayernrCtx, Abayernr_ExpInfo_t *pExpInfo, Abayernr_ParamMode_t *mode) {
    Abayernr_result_t res  = ABAYERNR_RET_SUCCESS;

    if(pAbayernrCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERNR_RET_INVALID_PARM;
    }

    *mode = pAbayernrCtx->eParamMode;

    if(pAbayernrCtx->isGrayMode) {
        *mode = ABAYERNR_PARAM_MODE_GRAY;
    } else if(pExpInfo->hdr_mode == 0) {
        *mode = ABAYERNR_PARAM_MODE_NORMAL;
    } else if(pExpInfo->hdr_mode >= 1) {
        *mode = ABAYERNR_PARAM_MODE_HDR;
    } else {
        *mode = ABAYERNR_PARAM_MODE_NORMAL;
    }

    return res;
}


RKAIQ_END_DECLARE


