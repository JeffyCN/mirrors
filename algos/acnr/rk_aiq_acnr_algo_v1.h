
#ifndef __RKAIQ_ACNR_V1_H__
#define __RKAIQ_ACNR_V1_H__

#include "stdio.h"
#include "string.h"
#include "math.h"
#include "stdlib.h"

#include "rk_aiq_acnr_algo_cnr_v1.h"
#include "rk_aiq_types_acnr_algo_prvt_v1.h"

RKAIQ_BEGIN_DECLARE

Acnr_result_t Acnr_Start_V1(Acnr_Context_V1_t *pAcnrCtx);

Acnr_result_t Acnr_Stop_V1(Acnr_Context_V1_t *pAcnrCtx);

//anr inint
Acnr_result_t Acnr_Init_V1(Acnr_Context_V1_t **ppAcnrCtx, void *pCalibDb);

//anr release
Acnr_result_t Acnr_Release_V1(Acnr_Context_V1_t *pAcnrCtx);

//anr config
Acnr_result_t Acnr_Prepare_V1(Acnr_Context_V1_t *pAcnrCtx, Acnr_Config_V1_t* pANRConfig);

//anr reconfig
Acnr_result_t Acnr_ReConfig_V1(Acnr_Context_V1_t *pAcnrCtx, Acnr_Config_V1_t* pANRConfig);

Acnr_result_t Acnr_IQParaUpdate_V1(Acnr_Context_V1_t *pAcnrCtx);

//anr preprocess
Acnr_result_t Acnr_PreProcess_V1(Acnr_Context_V1_t *pAcnrCtx);

//anr process
Acnr_result_t Acnr_Process_V1(Acnr_Context_V1_t *pAcnrCtx, Acnr_ExpInfo_t *pExpInfo);

//anr get result
Acnr_result_t Acnr_GetProcResult_V1(Acnr_Context_V1_t *pAcnrCtx, Acnr_ProcResult_V1_t* pANRResult);

Acnr_result_t Acnr_ConfigSettingParam_V1(Acnr_Context_V1_t *pAcnrCtx, Acnr_ParamMode_t eParamMode, int snr_mode);

Acnr_result_t Acnr_ParamModeProcess_V1(Acnr_Context_V1_t *pAcnrCtx, Acnr_ExpInfo_t *pExpInfo, Acnr_ParamMode_t *mode);


RKAIQ_END_DECLARE

#endif
