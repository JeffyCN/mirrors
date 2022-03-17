
#ifndef __RKAIQ_AYNR_V2_H__
#define __RKAIQ_AYNR_V2_H__

#include "stdio.h"
#include "string.h"
#include "math.h"
#include "stdlib.h"

#include "rk_aiq_aynr_algo_ynr_v2.h"
#include "rk_aiq_types_aynr_algo_prvt_v2.h"

RKAIQ_BEGIN_DECLARE

Aynr_result_t Aynr_Start_V2(Aynr_Context_V2_t *pAynrCtx);

Aynr_result_t Aynr_Stop_V2(Aynr_Context_V2_t *pAynrCtx); 

//anr inint
Aynr_result_t Aynr_Init_V2(Aynr_Context_V2_t **ppAynrCtx, CamCalibDbContext_t *pCalibDb);

//anr release
Aynr_result_t Aynr_Release_V2(Aynr_Context_V2_t *pAynrCtx);

//anr config
Aynr_result_t Aynr_Prepare_V2(Aynr_Context_V2_t *pAynrCtx, Aynr_Config_V2_t* pANRConfig);

//anr reconfig
Aynr_result_t Aynr_ReConfig_V2(Aynr_Context_V2_t *pAynrCtx, Aynr_Config_V2_t* pANRConfig);

Aynr_result_t Aynr_IQParaUpdate_V2(Aynr_Context_V2_t *pAynrCtx);

//anr preprocess
Aynr_result_t Aynr_PreProcess_V2(Aynr_Context_V2_t *pAynrCtx);

//anr process
Aynr_result_t Aynr_Process_V2(Aynr_Context_V2_t *pAynrCtx, Aynr_ExpInfo_t *pExpInfo);

//anr get result
Aynr_result_t Aynr_GetProcResult_V2(Aynr_Context_V2_t *pAynrCtx, Aynr_ProcResult_V2_t* pANRResult);

Aynr_result_t Aynr_ConfigSettingParam_V2(Aynr_Context_V2_t *pAynrCtx, Aynr_ParamMode_t eParamMode, int snr_mode);

Aynr_result_t Aynr_ParamModeProcess_V2(Aynr_Context_V2_t *pAynrCtx, Aynr_ExpInfo_t *pExpInfo, Aynr_ParamMode_t *mode);


RKAIQ_END_DECLARE

#endif
