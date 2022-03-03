
#ifndef __RKAIQ_ASHARP_SHARP_V4_H__
#define __RKAIQ_ASHARP_SHARP_V4_H__

#include "stdio.h"
#include "string.h"
#include "math.h"
#include "stdlib.h"

#include "rk_aiq_asharp_algo_sharp_v4.h"
#include "rk_aiq_types_asharp_algo_prvt_v4.h"
#include "RkAiqCalibDbV2Helper.h"

RKAIQ_BEGIN_DECLARE

Asharp4_result_t Asharp_Start_V4(Asharp_Context_V4_t *pAsharpCtx);

Asharp4_result_t Asharp_Stop_V4(Asharp_Context_V4_t *pAsharpCtx);

//anr inint
Asharp4_result_t Asharp_Init_V4(Asharp_Context_V4_t **ppAsharpCtx, void *pCalibDb);

//anr release
Asharp4_result_t Asharp_Release_V4(Asharp_Context_V4_t *pAsharpCtx);

//anr config
Asharp4_result_t Asharp_Prepare_V4(Asharp_Context_V4_t *pAsharpCtx, Asharp_Config_V4_t* pANRConfig);

//anr reconfig
Asharp4_result_t Asharp_ReConfig_V4(Asharp_Context_V4_t *pAsharpCtx, Asharp_Config_V4_t* pANRConfig);

Asharp4_result_t Asharp_IQParaUpdate_V4(Asharp_Context_V4_t *pAsharpCtx);

//anr preprocess
Asharp4_result_t Asharp_PreProcess_V4(Asharp_Context_V4_t *pAsharpCtx);

//anr process
Asharp4_result_t Asharp_Process_V4(Asharp_Context_V4_t *pAsharpCtx, Asharp4_ExpInfo_t *pExpInfo);

//anr get result
Asharp4_result_t Asharp_GetProcResult_V4(Asharp_Context_V4_t *pAsharpCtx, Asharp_ProcResult_V4_t* pANRResult);

Asharp4_result_t Asharp_ConfigSettingParam_V4(Asharp_Context_V4_t *pAsharpCtx, Asharp4_ParamMode_t eParamMode, int snr_mode);

Asharp4_result_t Asharp_ParamModeProcess_V4(Asharp_Context_V4_t *pAsharpCtx, Asharp4_ExpInfo_t *pExpInfo, Asharp4_ParamMode_t *mode);


RKAIQ_END_DECLARE

#endif
