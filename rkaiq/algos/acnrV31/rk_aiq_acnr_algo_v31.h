
#ifndef __RKAIQ_ACNR_V31_H__
#define __RKAIQ_ACNR_V31_H__

#include "stdio.h"
#include "string.h"
#include "math.h"
#include "stdlib.h"

#include "rk_aiq_acnr_algo_cnr_v31.h"
#include "rk_aiq_types_acnr_algo_prvt_v31.h"
#include "RkAiqCalibDbV2Helper.h"

RKAIQ_BEGIN_DECLARE

AcnrV31_result_t Acnr_Start_V31(Acnr_Context_V31_t *pAcnrCtx);

AcnrV31_result_t Acnr_Stop_V31(Acnr_Context_V31_t *pAcnrCtx);

//anr inint
AcnrV31_result_t Acnr_Init_V31(Acnr_Context_V31_t **ppAcnrCtx, void *pCalibDb);

//anr release
AcnrV31_result_t Acnr_Release_V31(Acnr_Context_V31_t *pAcnrCtx);

//anr config
AcnrV31_result_t Acnr_Prepare_V31(Acnr_Context_V31_t *pAcnrCtx, Acnr_Config_V31_t* pANRConfig);

//anr reconfig
AcnrV31_result_t Acnr_ReConfig_V31(Acnr_Context_V31_t *pAcnrCtx, Acnr_Config_V31_t* pANRConfig);

AcnrV31_result_t Acnr_IQParaUpdate_V31(Acnr_Context_V31_t *pAcnrCtx);

//anr preprocess
AcnrV31_result_t Acnr_PreProcess_V31(Acnr_Context_V31_t *pAcnrCtx);

//anr process
AcnrV31_result_t Acnr_Process_V31(Acnr_Context_V31_t *pAcnrCtx, AcnrV31_ExpInfo_t *pExpInfo);

//anr get result
AcnrV31_result_t Acnr_GetProcResult_V31(Acnr_Context_V31_t *pAcnrCtx, Acnr_ProcResult_V31_t* pANRResult);

AcnrV31_result_t Acnr_ConfigSettingParam_V31(Acnr_Context_V31_t *pAcnrCtx, AcnrV31_ParamMode_t eParamMode, int snr_mode);

AcnrV31_result_t Acnr_ParamModeProcess_V31(Acnr_Context_V31_t *pAcnrCtx, AcnrV31_ExpInfo_t *pExpInfo, AcnrV31_ParamMode_t *mode);


RKAIQ_END_DECLARE

#endif
