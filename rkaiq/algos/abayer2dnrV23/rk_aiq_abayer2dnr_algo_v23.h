
#ifndef __RKAIQ_ABAYER2DNR_V23_H__
#define __RKAIQ_ABAYER2DNR_V23_H__

#include "stdio.h"
#include "string.h"
#include "math.h"
#include "stdlib.h"

#include "rk_aiq_abayer2dnr_algo_bayernr_v23.h"
#include "rk_aiq_types_abayer2dnr_algo_prvt_v23.h"
#include "RkAiqCalibDbV2Helper.h"

RKAIQ_BEGIN_DECLARE

Abayer2dnr_result_V23_t Abayer2dnr_Start_V23(Abayer2dnr_Context_V23_t *pAbayer2dnrCtx);

Abayer2dnr_result_V23_t Abayer2dnr_Stop_V23(Abayer2dnr_Context_V23_t *pAbayer2dnrCtx);

//anr inint
Abayer2dnr_result_V23_t Abayer2dnr_Init_V23(Abayer2dnr_Context_V23_t **ppAbayer2dnrCtx, void *pCalibDb);

//anr release
Abayer2dnr_result_V23_t Abayer2dnr_Release_V23(Abayer2dnr_Context_V23_t *pAbayer2dnrCtx);

//anr config
Abayer2dnr_result_V23_t Abayer2dnr_Prepare_V23(Abayer2dnr_Context_V23_t *pAbayer2dnrCtx, Abayer2dnr_Config_V23_t* pANRConfig);

//anr reconfig
Abayer2dnr_result_V23_t Abayer2dnr_ReConfig_V23(Abayer2dnr_Context_V23_t *pAbayer2dnrCtx, Abayer2dnr_Config_V23_t* pANRConfig);

Abayer2dnr_result_V23_t Abayer2dnr_IQParaUpdate_V23(Abayer2dnr_Context_V23_t *pAbayer2dnrCtx);

//anr preprocess
Abayer2dnr_result_V23_t Abayer2dnr_PreProcess_V23(Abayer2dnr_Context_V23_t *pAbayer2dnrCtx);

//anr process
Abayer2dnr_result_V23_t Abayer2dnr_Process_V23(Abayer2dnr_Context_V23_t *pAbayer2dnrCtx, Abayer2dnr_ExpInfo_V23_t *pExpInfo);

//anr get result
Abayer2dnr_result_V23_t Abayer2dnr_GetProcResult_V23(Abayer2dnr_Context_V23_t *pAbayer2dnrCtx, Abayer2dnr_ProcResult_V23_t* pANRResult);

Abayer2dnr_result_V23_t Abayer2dnr_ConfigSettingParam_V23(Abayer2dnr_Context_V23_t *pAbayer2dnrCtx, Abayer2dnr_ParamMode_V23_t eParamMode, int snr_mode);

Abayer2dnr_result_V23_t Abayer2dnr_ParamModeProcess_V23(Abayer2dnr_Context_V23_t *pAbayer2dnrCtx, Abayer2dnr_ExpInfo_V23_t *pExpInfo, Abayer2dnr_ParamMode_V23_t *mode);


RKAIQ_END_DECLARE

#endif
