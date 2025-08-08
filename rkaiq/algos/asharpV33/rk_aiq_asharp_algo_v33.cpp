/*
 * rk_aiq_asharp_algo_v33.cpp
 *
 *  Copyright (c) 2022 Rockchip Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "rk_aiq_asharp_algo_v33.h"

#include "rk_aiq_asharp_algo_itf_v33.h"

RKAIQ_BEGIN_DECLARE

Asharp_result_V33_t Asharp_Start_V33(Asharp_Context_V33_t* pAsharpCtx) {
    LOGI_ASHARP("%s:enter!\n", __FUNCTION__);

    // initial checks
    if (pAsharpCtx == NULL) {
        return (ASHARP_V33_RET_NULL_POINTER);
    }

    if ((ASHARP_V33_STATE_RUNNING == pAsharpCtx->eState) ||
            (ASHARP_V33_STATE_LOCKED == pAsharpCtx->eState)) {
        return (ASHARP_V33_RET_FAILURE);
    }

    pAsharpCtx->eState = ASHARP_V33_STATE_RUNNING;

    LOGI_ASHARP("%s:exit!\n", __FUNCTION__);
    return (ASHARP_V33_RET_SUCCESS);
}

Asharp_result_V33_t Asharp_Stop_V33(Asharp_Context_V33_t* pAsharpCtx) {
    LOGI_ASHARP("%s:enter!\n", __FUNCTION__);

    // initial checks
    if (pAsharpCtx == NULL) {
        return (ASHARP_V33_RET_NULL_POINTER);
    }

    if (ASHARP_V33_STATE_LOCKED == pAsharpCtx->eState) {
        return (ASHARP_V33_RET_FAILURE);
    }

    pAsharpCtx->eState = ASHARP_V33_STATE_STOPPED;

    LOGI_ASHARP("%s:exit!\n", __FUNCTION__);
    return (ASHARP_V33_RET_SUCCESS);
}

// sharp inint
Asharp_result_V33_t Asharp_Init_V33(Asharp_Context_V33_t** ppAsharpCtx, void* pCalibDb) {
    Asharp_Context_V33_t* pAsharpCtx;

    LOGI_ASHARP("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    pAsharpCtx = (Asharp_Context_V33_t*)malloc(sizeof(Asharp_Context_V33_t));
    if (pAsharpCtx == NULL) {
        LOGE_ASHARP("%s(%d): malloc fail\n", __FUNCTION__, __LINE__);
        return ASHARP_V33_RET_NULL_POINTER;
    }

    memset(pAsharpCtx, 0x00, sizeof(Asharp_Context_V33_t));

    // gain state init
    pAsharpCtx->stStrength.strength_enable = true;
    pAsharpCtx->stStrength.percent = 1.0;

    pAsharpCtx->eState = ASHARP_V33_STATE_INITIALIZED;
    *ppAsharpCtx       = pAsharpCtx;

    pAsharpCtx->eMode          = ASHARP_V33_OP_MODE_AUTO;
    pAsharpCtx->isIQParaUpdate = false;
    pAsharpCtx->isGrayMode     = false;
    pAsharpCtx->isReCalculate  = 1;

#if ASHARP_USE_XML_FILE_V33
    // read v1 params from xml
#if ASHARP_USE_JSON_FILE_V33
#if RKAIQ_HAVE_SHARP_V33
    pAsharpCtx->sharp_v33 = *(CalibDbV2_SharpV33_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibDb, sharp_v33));
#else
    pAsharpCtx->sharp_v33 =
        *(CalibDbV2_SharpV33Lite_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibDb, sharp_v33));
#endif
#endif
#endif

#if RK_SIMULATOR_HW
    // just for V33 params from html

#endif

#if ASHARP_USE_XML_FILE_V33
    pAsharpCtx->stExpInfo.snr_mode = 0;
    pAsharpCtx->eParamMode         = ASHARP_V33_PARAM_MODE_NORMAL;
    Asharp_ConfigSettingParam_V33(pAsharpCtx, pAsharpCtx->eParamMode,
                                  pAsharpCtx->stExpInfo.snr_mode);
    // init manual params
    pAsharpCtx->stManual.stSelect = pAsharpCtx->stAuto.stParams.sharpParamsISO[0];
#endif

    LOGD_ASHARP("%s(%d):", __FUNCTION__, __LINE__);

    LOGI_ASHARP("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ASHARP_V33_RET_SUCCESS;
}

// anr release
Asharp_result_V33_t Asharp_Release_V33(Asharp_Context_V33_t* pAsharpCtx) {
    Asharp_result_V33_t result = ASHARP_V33_RET_SUCCESS;
    LOGI_ASHARP("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    if (pAsharpCtx == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP_V33_RET_NULL_POINTER;
    }

    result = Asharp_Stop_V33(pAsharpCtx);
    if (result != ASHARP_V33_RET_SUCCESS) {
        LOGE_ASHARP("%s: ANRStop() failed!\n", __FUNCTION__);
        return (result);
    }

    // check state
    if ((ASHARP_V33_STATE_RUNNING == pAsharpCtx->eState) ||
            (ASHARP_V33_STATE_LOCKED == pAsharpCtx->eState)) {
        return (ASHARP_V33_RET_BUSY);
    }

    memset(pAsharpCtx, 0x00, sizeof(Asharp_Context_V33_t));
    free(pAsharpCtx);

    LOGI_ASHARP("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ASHARP_V33_RET_SUCCESS;
}

// anr config
Asharp_result_V33_t Asharp_Prepare_V33(Asharp_Context_V33_t* pAsharpCtx,
                                       Asharp_Config_V33_t* pAsharpConfig) {
    LOGI_ASHARP("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    if (pAsharpCtx == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP_V33_RET_INVALID_PARM;
    }

    if (pAsharpConfig == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP_V33_RET_INVALID_PARM;
    }

    if (!!(pAsharpCtx->prepare_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB)) {
        Asharp_IQParaUpdate_V33(pAsharpCtx);
    }

    pAsharpCtx->rawWidth  = pAsharpConfig->rawWidth;
    pAsharpCtx->rawHeight = pAsharpConfig->rawHeight;
    Asharp_Start_V33(pAsharpCtx);

    LOGI_ASHARP("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ASHARP_V33_RET_SUCCESS;
}

// anr reconfig
Asharp_result_V33_t Asharp_ReConfig_V33(Asharp_Context_V33_t* pAsharpCtx,
                                        Asharp_Config_V33_t* pAsharpConfig) {
    LOGI_ASHARP("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    // need todo what?

    LOGI_ASHARP("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ASHARP_V33_RET_SUCCESS;
}

// anr reconfig
Asharp_result_V33_t Asharp_IQParaUpdate_V33(Asharp_Context_V33_t* pAsharpCtx) {
    LOGI_ASHARP("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    // need todo what?

    if (pAsharpCtx->isIQParaUpdate) {
        LOGD_ASHARP("IQ data reconfig\n");
        Asharp_ConfigSettingParam_V33(pAsharpCtx, pAsharpCtx->eParamMode,
                                      pAsharpCtx->stExpInfo.snr_mode);
        pAsharpCtx->isIQParaUpdate = false;
    }

    LOGI_ASHARP("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ASHARP_V33_RET_SUCCESS;
}

// anr preprocess
Asharp_result_V33_t Asharp_PreProcess_V33(Asharp_Context_V33_t* pAsharpCtx) {
    LOGI_ASHARP("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    // need todo what?

    Asharp_IQParaUpdate_V33(pAsharpCtx);

    LOGI_ASHARP("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ASHARP_V33_RET_SUCCESS;
}

// anr process
Asharp_result_V33_t Asharp_Process_V33(Asharp_Context_V33_t* pAsharpCtx,
                                       Asharp_ExpInfo_V33_t* pExpInfo) {
    LOGI_ASHARP("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    Asharp_ParamMode_V33_t mode = ASHARP_V33_PARAM_MODE_INVALID;

    if (pAsharpCtx == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP_V33_RET_INVALID_PARM;
    }

    if (pExpInfo == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP_V33_RET_INVALID_PARM;
    }

    if (pAsharpCtx->eState != ASHARP_V33_STATE_RUNNING) {
        return ASHARP_V33_RET_SUCCESS;
    }

    Asharp_ParamModeProcess_V33(pAsharpCtx, pExpInfo, &mode);

    if (pAsharpCtx->eMode == ASHARP_V33_OP_MODE_AUTO) {
        LOGD_ASHARP("%s(%d): \n", __FUNCTION__, __LINE__);

#if ASHARP_USE_XML_FILE_V33
        if (pExpInfo->snr_mode != pAsharpCtx->stExpInfo.snr_mode ||
                pAsharpCtx->eParamMode != mode) {
            LOGD_ASHARP("param mode:%d snr_mode:%d\n", mode, pExpInfo->snr_mode);
            pAsharpCtx->eParamMode = mode;
            Asharp_ConfigSettingParam_V33(pAsharpCtx, pAsharpCtx->eParamMode, pExpInfo->snr_mode);
        }
#endif

        // select param
        sharp_select_params_by_ISO_V33(&pAsharpCtx->stAuto.stParams, &pAsharpCtx->stAuto.stSelect,
                                       pExpInfo);

    } else if (pAsharpCtx->eMode == ASHARP_V33_OP_MODE_MANUAL) {
        // TODO
    }

    memcpy(&pAsharpCtx->stExpInfo, pExpInfo, sizeof(Asharp_ExpInfo_V33_t));
    pAsharpCtx->stExpInfo.rawHeight = pAsharpCtx->rawHeight;
    pAsharpCtx->stExpInfo.rawWidth  = pAsharpCtx->rawWidth;

    LOGI_ASHARP("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ASHARP_V33_RET_SUCCESS;
}

// anr get result
Asharp_result_V33_t Asharp_GetProcResult_V33(Asharp_Context_V33_t* pAsharpCtx,
        Asharp_ProcResult_V33_t* pAsharpResult) {
    LOGI_ASHARP("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    if (pAsharpCtx == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP_V33_RET_INVALID_PARM;
    }

    if (pAsharpResult == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP_V33_RET_INVALID_PARM;
    }

#if RKAIQ_HAVE_SHARP_V33
    RK_SHARP_Params_V33_Select_t* stSelect = NULL;
#else
    RK_SHARP_Params_V33LT_Select_t* stSelect = NULL;
#endif
    if (pAsharpCtx->eMode == ASHARP_V33_OP_MODE_AUTO) {
        stSelect = &pAsharpCtx->stAuto.stSelect;
    } else if (pAsharpCtx->eMode == ASHARP_V33_OP_MODE_MANUAL) {
        stSelect     = &pAsharpCtx->stManual.stSelect;
    }

    // transfer to reg value
    sharp_fix_transfer_V33(stSelect, pAsharpResult->stFix, &pAsharpCtx->stStrength, &pAsharpCtx->stExpInfo);
    if (pAsharpCtx->eMode == ASHARP_V33_OP_MODE_REG_MANUAL) {
        *pAsharpResult->stFix        = pAsharpCtx->stManual.stFix;
        pAsharpCtx->stStrength.percent = 1.0;
    }

    LOGI_ASHARP("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ASHARP_V33_RET_SUCCESS;
}

Asharp_result_V33_t Asharp_ConfigSettingParam_V33(Asharp_Context_V33_t* pAsharpCtx,
        Asharp_ParamMode_V33_t eParamMode, int snr_mode) {
    char snr_name[CALIBDB_NR_SHARP_NAME_LENGTH];
    char param_mode_name[CALIBDB_MAX_MODE_NAME_LENGTH];
    memset(param_mode_name, 0x00, sizeof(param_mode_name));
    memset(snr_name, 0x00, sizeof(snr_name));

    if (pAsharpCtx == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP_V33_RET_INVALID_PARM;
    }

    // select param mode first
    if (eParamMode == ASHARP_V33_PARAM_MODE_NORMAL) {
        sprintf(param_mode_name, "%s", "normal");
    } else if (eParamMode == ASHARP_V33_PARAM_MODE_HDR) {
        sprintf(param_mode_name, "%s", "hdr");
    } else if (eParamMode == ASHARP_V33_PARAM_MODE_GRAY) {
        sprintf(param_mode_name, "%s", "gray");
    } else {
        LOGE_ASHARP("%s(%d): not support param mode!\n", __FUNCTION__, __LINE__);
        sprintf(param_mode_name, "%s", "normal");
    }

    // then select snr mode next
    if (snr_mode == 1) {
        sprintf(snr_name, "%s", "HSNR");
    } else if (snr_mode == 0) {
        sprintf(snr_name, "%s", "LSNR");
    } else {
        LOGE_ASHARP("%s(%d): not support snr mode:%d!\n", __FUNCTION__, __LINE__, snr_mode);
        sprintf(snr_name, "%s", "LSNR");
    }

#if ASHARP_USE_JSON_FILE_V33
    sharp_config_setting_param_json_V33(&pAsharpCtx->stAuto.stParams, &pAsharpCtx->sharp_v33,
                                        param_mode_name, snr_name);
#endif

    return ASHARP_V33_RET_SUCCESS;
}

Asharp_result_V33_t Asharp_ParamModeProcess_V33(Asharp_Context_V33_t* pAsharpCtx,
        Asharp_ExpInfo_V33_t* pExpInfo,
        Asharp_ParamMode_V33_t* mode) {
    Asharp_result_V33_t res = ASHARP_V33_RET_SUCCESS;

    if (pAsharpCtx == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP_V33_RET_INVALID_PARM;
    }

    *mode = pAsharpCtx->eParamMode;

    if (pAsharpCtx->isGrayMode) {
        *mode = ASHARP_V33_PARAM_MODE_GRAY;
    } else if (pExpInfo->hdr_mode == 0) {
        *mode = ASHARP_V33_PARAM_MODE_NORMAL;
    } else if (pExpInfo->hdr_mode >= 1) {
        *mode = ASHARP_V33_PARAM_MODE_HDR;
    } else {
        *mode = ASHARP_V33_PARAM_MODE_NORMAL;
    }

    return res;
}

RKAIQ_END_DECLARE
