
#ifndef __RKAIQ_ACNR_V30_H__
#define __RKAIQ_ACNR_V30_H__

#include "stdio.h"
#include "string.h"
#include "math.h"
#include "stdlib.h"

#include "rk_aiq_acnr_algo_cnr_v30.h"
#include "rk_aiq_types_acnr_algo_prvt_v30.h"
#include "RkAiqCalibDbV2Helper.h"

RKAIQ_BEGIN_DECLARE

AcnrV30_result_t Acnr_Start_V30(Acnr_Context_V30_t *pAcnrCtx);

AcnrV30_result_t Acnr_Stop_V30(Acnr_Context_V30_t *pAcnrCtx);

//anr inint
AcnrV30_result_t Acnr_Init_V30(Acnr_Context_V30_t **ppAcnrCtx, void *pCalibDb);

//anr release
AcnrV30_result_t Acnr_Release_V30(Acnr_Context_V30_t *pAcnrCtx);

//anr config
AcnrV30_result_t Acnr_Prepare_V30(Acnr_Context_V30_t *pAcnrCtx, Acnr_Config_V30_t* pANRConfig);

//anr reconfig
AcnrV30_result_t Acnr_ReConfig_V30(Acnr_Context_V30_t *pAcnrCtx, Acnr_Config_V30_t* pANRConfig);

AcnrV30_result_t Acnr_IQParaUpdate_V30(Acnr_Context_V30_t *pAcnrCtx);

//anr preprocess
AcnrV30_result_t Acnr_PreProcess_V30(Acnr_Context_V30_t *pAcnrCtx);

//anr process
AcnrV30_result_t Acnr_Process_V30(Acnr_Context_V30_t *pAcnrCtx, AcnrV30_ExpInfo_t *pExpInfo);

//anr get result
AcnrV30_result_t Acnr_GetProcResult_V30(Acnr_Context_V30_t *pAcnrCtx, Acnr_ProcResult_V30_t* pANRResult);

AcnrV30_result_t Acnr_ConfigSettingParam_V30(Acnr_Context_V30_t *pAcnrCtx, AcnrV30_ParamMode_t eParamMode, int snr_mode);

AcnrV30_result_t Acnr_ParamModeProcess_V30(Acnr_Context_V30_t *pAcnrCtx, AcnrV30_ExpInfo_t *pExpInfo, AcnrV30_ParamMode_t *mode);


RKAIQ_END_DECLARE

#endif
