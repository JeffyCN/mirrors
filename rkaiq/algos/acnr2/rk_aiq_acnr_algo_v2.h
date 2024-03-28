
#ifndef __RKAIQ_ACNR_V2_H__
#define __RKAIQ_ACNR_V2_H__

#include "stdio.h"
#include "string.h"
#include "math.h"
#include "stdlib.h"

#include "rk_aiq_acnr_algo_cnr_v2.h"
#include "rk_aiq_types_acnr_algo_prvt_v2.h"
#include "RkAiqCalibDbV2Helper.h"

RKAIQ_BEGIN_DECLARE

AcnrV2_result_t Acnr_Start_V2(Acnr_Context_V2_t *pAcnrCtx);

AcnrV2_result_t Acnr_Stop_V2(Acnr_Context_V2_t *pAcnrCtx);

//anr inint
AcnrV2_result_t Acnr_Init_V2(Acnr_Context_V2_t **ppAcnrCtx, void *pCalibDb);

//anr release
AcnrV2_result_t Acnr_Release_V2(Acnr_Context_V2_t *pAcnrCtx);

//anr config
AcnrV2_result_t Acnr_Prepare_V2(Acnr_Context_V2_t *pAcnrCtx, Acnr_Config_V2_t* pANRConfig);

//anr reconfig
AcnrV2_result_t Acnr_ReConfig_V2(Acnr_Context_V2_t *pAcnrCtx, Acnr_Config_V2_t* pANRConfig);

AcnrV2_result_t Acnr_IQParaUpdate_V2(Acnr_Context_V2_t *pAcnrCtx);

//anr preprocess
AcnrV2_result_t Acnr_PreProcess_V2(Acnr_Context_V2_t *pAcnrCtx);

//anr process
AcnrV2_result_t Acnr_Process_V2(Acnr_Context_V2_t *pAcnrCtx, AcnrV2_ExpInfo_t *pExpInfo);

//anr get result
AcnrV2_result_t Acnr_GetProcResult_V2(Acnr_Context_V2_t *pAcnrCtx, Acnr_ProcResult_V2_t* pANRResult);

AcnrV2_result_t Acnr_ConfigSettingParam_V2(Acnr_Context_V2_t *pAcnrCtx, AcnrV2_ParamMode_t eParamMode, int snr_mode);

AcnrV2_result_t Acnr_ParamModeProcess_V2(Acnr_Context_V2_t *pAcnrCtx, AcnrV2_ExpInfo_t *pExpInfo, AcnrV2_ParamMode_t *mode);


RKAIQ_END_DECLARE

#endif
