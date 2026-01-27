
#ifndef __RKAIQ_AYNR_V24_H__
#define __RKAIQ_AYNR_V24_H__

#include "RkAiqCalibDbV2Helper.h"
#include "math.h"
#include "rk_aiq_aynr_algo_ynr_v24.h"
#include "rk_aiq_types_aynr_algo_prvt_v24.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

RKAIQ_BEGIN_DECLARE

Aynr_result_V24_t Aynr_Start_V24(Aynr_Context_V24_t* pAynrCtx);

Aynr_result_V24_t Aynr_Stop_V24(Aynr_Context_V24_t* pAynrCtx);

// anr inint
Aynr_result_V24_t Aynr_Init_V24(Aynr_Context_V24_t** ppAynrCtx, CamCalibDbContext_t* pCalibDb);

// anr release
Aynr_result_V24_t Aynr_Release_V24(Aynr_Context_V24_t* pAynrCtx);

// anr config
Aynr_result_V24_t Aynr_Prepare_V24(Aynr_Context_V24_t* pAynrCtx, Aynr_Config_V24_t* pANRConfig);

// anr reconfig
Aynr_result_V24_t Aynr_IQParaUpdate_V24(Aynr_Context_V24_t* pAynrCtx);

// anr preprocess
Aynr_result_V24_t Aynr_PreProcess_V24(Aynr_Context_V24_t* pAynrCtx);

// anr process
Aynr_result_V24_t Aynr_Process_V24(Aynr_Context_V24_t* pAynrCtx, Aynr_ExpInfo_V24_t* pExpInfo);

// anr get result
Aynr_result_V24_t Aynr_GetProcResult_V24(Aynr_Context_V24_t* pAynrCtx,
                                         Aynr_ProcResult_V24_t* pANRResult);

Aynr_result_V24_t Aynr_ConfigSettingParam_V24(Aynr_Context_V24_t* pAynrCtx,
                                              Aynr_ParamMode_V24_t eParamMode, int snr_mode);

Aynr_result_V24_t Aynr_ParamModeProcess_V24(Aynr_Context_V24_t* pAynrCtx,
                                            Aynr_ExpInfo_V24_t* pExpInfo,
                                            Aynr_ParamMode_V24_t* mode);

RKAIQ_END_DECLARE

#endif
