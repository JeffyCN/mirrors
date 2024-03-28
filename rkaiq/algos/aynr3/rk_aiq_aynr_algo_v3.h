
#ifndef __RKAIQ_AYNR_V3_H__
#define __RKAIQ_AYNR_V3_H__

#include "stdio.h"
#include "string.h"
#include "math.h"
#include "stdlib.h"

#include "rk_aiq_aynr_algo_ynr_v3.h"
#include "rk_aiq_types_aynr_algo_prvt_v3.h"
#include "RkAiqCalibDbV2Helper.h"

RKAIQ_BEGIN_DECLARE

Aynr_result_V3_t Aynr_Start_V3(Aynr_Context_V3_t *pAynrCtx);

Aynr_result_V3_t Aynr_Stop_V3(Aynr_Context_V3_t *pAynrCtx);

//anr inint
Aynr_result_V3_t Aynr_Init_V3(Aynr_Context_V3_t **ppAynrCtx, CamCalibDbContext_t *pCalibDb);

//anr release
Aynr_result_V3_t Aynr_Release_V3(Aynr_Context_V3_t *pAynrCtx);

//anr config
Aynr_result_V3_t Aynr_Prepare_V3(Aynr_Context_V3_t *pAynrCtx, Aynr_Config_V3_t* pANRConfig);

//anr reconfig
Aynr_result_V3_t Aynr_ReConfig_V3(Aynr_Context_V3_t *pAynrCtx, Aynr_Config_V3_t* pANRConfig);

Aynr_result_V3_t Aynr_IQParaUpdate_V3(Aynr_Context_V3_t *pAynrCtx);

//anr preprocess
Aynr_result_V3_t Aynr_PreProcess_V3(Aynr_Context_V3_t *pAynrCtx);

//anr process
Aynr_result_V3_t Aynr_Process_V3(Aynr_Context_V3_t *pAynrCtx, Aynr_ExpInfo_V3_t *pExpInfo);

//anr get result
Aynr_result_V3_t Aynr_GetProcResult_V3(Aynr_Context_V3_t *pAynrCtx, Aynr_ProcResult_V3_t* pANRResult);

Aynr_result_V3_t Aynr_ConfigSettingParam_V3(Aynr_Context_V3_t *pAynrCtx, Aynr_ParamMode_V3_t eParamMode, int snr_mode);

Aynr_result_V3_t Aynr_ParamModeProcess_V3(Aynr_Context_V3_t *pAynrCtx, Aynr_ExpInfo_V3_t *pExpInfo, Aynr_ParamMode_V3_t *mode);


RKAIQ_END_DECLARE

#endif
