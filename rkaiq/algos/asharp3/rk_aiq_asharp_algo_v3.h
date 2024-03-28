
#ifndef __RKAIQ_ASHARP_SHARP_V3_H__
#define __RKAIQ_ASHARP_SHARP_V3_H__

#include "stdio.h"
#include "string.h"
#include "math.h"
#include "stdlib.h"

#include "rk_aiq_asharp_algo_sharp_v3.h"
#include "rk_aiq_types_asharp_algo_prvt_v3.h"

RKAIQ_BEGIN_DECLARE

Asharp3_result_t Asharp_Start_V3(Asharp_Context_V3_t *pAsharpCtx);

Asharp3_result_t Asharp_Stop_V3(Asharp_Context_V3_t *pAsharpCtx);

//anr inint
Asharp3_result_t Asharp_Init_V3(Asharp_Context_V3_t **ppAsharpCtx, void *pCalibDb);

//anr release
Asharp3_result_t Asharp_Release_V3(Asharp_Context_V3_t *pAsharpCtx);

//anr config
Asharp3_result_t Asharp_Prepare_V3(Asharp_Context_V3_t *pAsharpCtx, Asharp_Config_V3_t* pANRConfig);

//anr reconfig
Asharp3_result_t Asharp_ReConfig_V3(Asharp_Context_V3_t *pAsharpCtx, Asharp_Config_V3_t* pANRConfig);

Asharp3_result_t Asharp_IQParaUpdate_V3(Asharp_Context_V3_t *pAsharpCtx);

//anr preprocess
Asharp3_result_t Asharp_PreProcess_V3(Asharp_Context_V3_t *pAsharpCtx);

//anr process
Asharp3_result_t Asharp_Process_V3(Asharp_Context_V3_t *pAsharpCtx, Asharp3_ExpInfo_t *pExpInfo);

//anr get result
Asharp3_result_t Asharp_GetProcResult_V3(Asharp_Context_V3_t *pAsharpCtx, Asharp_ProcResult_V3_t* pANRResult);

Asharp3_result_t Asharp_ConfigSettingParam_V3(Asharp_Context_V3_t *pAsharpCtx, Asharp3_ParamMode_t eParamMode, int snr_mode);

Asharp3_result_t Asharp_ParamModeProcess_V3(Asharp_Context_V3_t *pAsharpCtx, Asharp3_ExpInfo_t *pExpInfo, Asharp3_ParamMode_t *mode);


RKAIQ_END_DECLARE

#endif
