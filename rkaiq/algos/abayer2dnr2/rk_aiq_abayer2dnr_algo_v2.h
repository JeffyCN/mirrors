
#ifndef __RKAIQ_ABAYER2DNR_V2_H__
#define __RKAIQ_ABAYER2DNR_V2_H__

#include "stdio.h"
#include "string.h"
#include "math.h"
#include "stdlib.h"

#include "rk_aiq_abayer2dnr_algo_bayernr_v2.h"
#include "rk_aiq_types_abayer2dnr_algo_prvt_v2.h"
#include "RkAiqCalibDbV2Helper.h"

RKAIQ_BEGIN_DECLARE

Abayer2dnr_result_V2_t Abayer2dnr_Start_V2(Abayer2dnr_Context_V2_t *pAbayer2dnrCtx);

Abayer2dnr_result_V2_t Abayer2dnr_Stop_V2(Abayer2dnr_Context_V2_t *pAbayer2dnrCtx);

//anr inint
Abayer2dnr_result_V2_t Abayer2dnr_Init_V2(Abayer2dnr_Context_V2_t **ppAbayer2dnrCtx, void *pCalibDb);

//anr release
Abayer2dnr_result_V2_t Abayer2dnr_Release_V2(Abayer2dnr_Context_V2_t *pAbayer2dnrCtx);

//anr config
Abayer2dnr_result_V2_t Abayer2dnr_Prepare_V2(Abayer2dnr_Context_V2_t *pAbayer2dnrCtx, Abayer2dnr_Config_V2_t* pANRConfig);

//anr reconfig
Abayer2dnr_result_V2_t Abayer2dnr_ReConfig_V2(Abayer2dnr_Context_V2_t *pAbayer2dnrCtx, Abayer2dnr_Config_V2_t* pANRConfig);

Abayer2dnr_result_V2_t Abayer2dnr_IQParaUpdate_V2(Abayer2dnr_Context_V2_t *pAbayer2dnrCtx);

//anr preprocess
Abayer2dnr_result_V2_t Abayer2dnr_PreProcess_V2(Abayer2dnr_Context_V2_t *pAbayer2dnrCtx);

//anr process
Abayer2dnr_result_V2_t Abayer2dnr_Process_V2(Abayer2dnr_Context_V2_t *pAbayer2dnrCtx, Abayer2dnr_ExpInfo_V2_t *pExpInfo);

//anr get result
Abayer2dnr_result_V2_t Abayer2dnr_GetProcResult_V2(Abayer2dnr_Context_V2_t *pAbayer2dnrCtx, Abayer2dnr_ProcResult_V2_t* pANRResult);

Abayer2dnr_result_V2_t Abayer2dnr_ConfigSettingParam_V2(Abayer2dnr_Context_V2_t *pAbayer2dnrCtx, Abayer2dnr_ParamMode_V2_t eParamMode, int snr_mode);

Abayer2dnr_result_V2_t Abayer2dnr_ParamModeProcess_V2(Abayer2dnr_Context_V2_t *pAbayer2dnrCtx, Abayer2dnr_ExpInfo_V2_t *pExpInfo, Abayer2dnr_ParamMode_V2_t *mode);


RKAIQ_END_DECLARE

#endif
