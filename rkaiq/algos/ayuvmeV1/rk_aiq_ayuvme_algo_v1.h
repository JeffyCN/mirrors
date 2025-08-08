
#ifndef __RKAIQ_AYUVME_V1_H__
#define __RKAIQ_AYUVME_V1_H__

#include "stdio.h"
#include "string.h"
#include "math.h"
#include "stdlib.h"

#include "rk_aiq_ayuvme_algo_yuvme_v1.h"
#include "rk_aiq_types_ayuvme_algo_prvt_v1.h"
#include "RkAiqCalibDbV2Helper.h"

RKAIQ_BEGIN_DECLARE

Ayuvme_result_V1_t Ayuvme_Start_V1(Ayuvme_Context_V1_t *pAyuvmeCtx);

Ayuvme_result_V1_t Ayuvme_Stop_V1(Ayuvme_Context_V1_t *pAyuvmeCtx);

//anr inint
Ayuvme_result_V1_t Ayuvme_Init_V1(Ayuvme_Context_V1_t **ppAyuvmeCtx, CamCalibDbContext_t *pCalibDb);

//anr release
Ayuvme_result_V1_t Ayuvme_Release_V1(Ayuvme_Context_V1_t *pAyuvmeCtx);

//anr config
Ayuvme_result_V1_t Ayuvme_Prepare_V1(Ayuvme_Context_V1_t *pAyuvmeCtx, Ayuvme_Config_V1_t* pANRConfig);

//anr reconfig
Ayuvme_result_V1_t Ayuvme_ReConfig_V1(Ayuvme_Context_V1_t *pAyuvmeCtx, Ayuvme_Config_V1_t* pANRConfig);

Ayuvme_result_V1_t Ayuvme_IQParaUpdate_V1(Ayuvme_Context_V1_t *pAyuvmeCtx);

//anr preprocess
Ayuvme_result_V1_t Ayuvme_PreProcess_V1(Ayuvme_Context_V1_t *pAyuvmeCtx);

//anr process
Ayuvme_result_V1_t Ayuvme_Process_V1(Ayuvme_Context_V1_t *pAyuvmeCtx, Ayuvme_ExpInfo_V1_t *pExpInfo);

//anr get result
Ayuvme_result_V1_t Ayuvme_GetProcResult_V1(Ayuvme_Context_V1_t *pAyuvmeCtx, Ayuvme_ProcResult_V1_t* pANRResult);

Ayuvme_result_V1_t Ayuvme_ConfigSettingParam_V1(Ayuvme_Context_V1_t *pAyuvmeCtx, Ayuvme_ParamMode_V1_t eParamMode, int snr_mode);

Ayuvme_result_V1_t Ayuvme_ParamModeProcess_V1(Ayuvme_Context_V1_t *pAyuvmeCtx, Ayuvme_ExpInfo_V1_t *pExpInfo, Ayuvme_ParamMode_V1_t *mode);


RKAIQ_END_DECLARE

#endif
