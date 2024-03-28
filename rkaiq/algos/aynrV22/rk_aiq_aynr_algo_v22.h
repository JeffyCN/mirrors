
#ifndef __RKAIQ_AYNR_V22_H__
#define __RKAIQ_AYNR_V22_H__

#include "stdio.h"
#include "string.h"
#include "math.h"
#include "stdlib.h"

#include "rk_aiq_aynr_algo_ynr_v22.h"
#include "rk_aiq_types_aynr_algo_prvt_v22.h"
#include "RkAiqCalibDbV2Helper.h"

RKAIQ_BEGIN_DECLARE

Aynr_result_V22_t Aynr_Start_V22(Aynr_Context_V22_t *pAynrCtx);

Aynr_result_V22_t Aynr_Stop_V22(Aynr_Context_V22_t *pAynrCtx);

//anr inint
Aynr_result_V22_t Aynr_Init_V22(Aynr_Context_V22_t **ppAynrCtx, CamCalibDbContext_t *pCalibDb);

//anr release
Aynr_result_V22_t Aynr_Release_V22(Aynr_Context_V22_t *pAynrCtx);

//anr config
Aynr_result_V22_t Aynr_Prepare_V22(Aynr_Context_V22_t *pAynrCtx, Aynr_Config_V22_t* pANRConfig);

//anr reconfig
Aynr_result_V22_t Aynr_ReConfig_V22(Aynr_Context_V22_t *pAynrCtx, Aynr_Config_V22_t* pANRConfig);

Aynr_result_V22_t Aynr_IQParaUpdate_V22(Aynr_Context_V22_t *pAynrCtx);

//anr preprocess
Aynr_result_V22_t Aynr_PreProcess_V22(Aynr_Context_V22_t *pAynrCtx);

//anr process
Aynr_result_V22_t Aynr_Process_V22(Aynr_Context_V22_t *pAynrCtx, Aynr_ExpInfo_V22_t *pExpInfo);

//anr get result
Aynr_result_V22_t Aynr_GetProcResult_V22(Aynr_Context_V22_t *pAynrCtx, Aynr_ProcResult_V22_t* pANRResult);

Aynr_result_V22_t Aynr_ConfigSettingParam_V22(Aynr_Context_V22_t *pAynrCtx, Aynr_ParamMode_V22_t eParamMode, int snr_mode);

Aynr_result_V22_t Aynr_ParamModeProcess_V22(Aynr_Context_V22_t *pAynrCtx, Aynr_ExpInfo_V22_t *pExpInfo, Aynr_ParamMode_V22_t *mode);


RKAIQ_END_DECLARE

#endif
