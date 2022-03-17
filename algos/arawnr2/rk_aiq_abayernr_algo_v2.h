
#ifndef __RKAIQ_ABAYERNR_V2_H__
#define __RKAIQ_ABAYERNR_V2_H__

#include "stdio.h"
#include "string.h"
#include "math.h"
#include "stdlib.h"

#include "rk_aiq_abayernr_algo_bayernr_v2.h"
#include "rk_aiq_types_abayernr_algo_prvt_v2.h"

RKAIQ_BEGIN_DECLARE

Abayernr_result_t Abayernr_Start_V2(Abayernr_Context_V2_t *pAbayernrCtx);

Abayernr_result_t Abayernr_Stop_V2(Abayernr_Context_V2_t *pAbayernrCtx);

//anr inint
Abayernr_result_t Abayernr_Init_V2(Abayernr_Context_V2_t **ppAbayernrCtx, void *pCalibDb);

//anr release
Abayernr_result_t Abayernr_Release_V2(Abayernr_Context_V2_t *pAbayernrCtx);

//anr config
Abayernr_result_t Abayernr_Prepare_V2(Abayernr_Context_V2_t *pAbayernrCtx, Abayernr_Config_V2_t* pANRConfig);

//anr reconfig
Abayernr_result_t Abayernr_ReConfig_V2(Abayernr_Context_V2_t *pAbayernrCtx, Abayernr_Config_V2_t* pANRConfig);

Abayernr_result_t Abayernr_IQParaUpdate_V2(Abayernr_Context_V2_t *pAbayernrCtx);

//anr preprocess
Abayernr_result_t Abayernr_PreProcess_V2(Abayernr_Context_V2_t *pAbayernrCtx);

//anr process
Abayernr_result_t Abayernr_Process_V2(Abayernr_Context_V2_t *pAbayernrCtx, Abayernr_ExpInfo_t *pExpInfo);

//anr get result
Abayernr_result_t Abayernr_GetProcResult_V2(Abayernr_Context_V2_t *pAbayernrCtx, Abayernr_ProcResult_V2_t* pANRResult);

Abayernr_result_t Abayernr_ConfigSettingParam_V2(Abayernr_Context_V2_t *pAbayernrCtx, Abayernr_ParamMode_t eParamMode, int snr_mode);

Abayernr_result_t Abayernr_ParamModeProcess_V2(Abayernr_Context_V2_t *pAbayernrCtx, Abayernr_ExpInfo_t *pExpInfo, Abayernr_ParamMode_t *mode);


RKAIQ_END_DECLARE

#endif
