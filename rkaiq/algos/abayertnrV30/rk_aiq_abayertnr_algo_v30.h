
#ifndef __RKAIQ_ABAYERNR_V30_H__
#define __RKAIQ_ABAYERNR_V30_H__

#include "stdio.h"
#include "string.h"
#include "math.h"
#include "stdlib.h"

#include "rk_aiq_abayertnr_algo_bayertnr_v30.h"
#include "rk_aiq_types_abayertnr_algo_prvt_v30.h"
#include "RkAiqCalibDbV2Helper.h"

RKAIQ_BEGIN_DECLARE

Abayertnr_result_V30_t Abayertnr_Start_V30(Abayertnr_Context_V30_t *pAbayertnrCtx);

Abayertnr_result_V30_t Abayertnr_Stop_V30(Abayertnr_Context_V30_t *pAbayertnrCtx);

//anr inint
Abayertnr_result_V30_t Abayertnr_Init_V30(Abayertnr_Context_V30_t **ppAbayertnrCtx, void *pCalibDb);

//anr release
Abayertnr_result_V30_t Abayertnr_Release_V30(Abayertnr_Context_V30_t *pAbayertnrCtx);

//anr config
Abayertnr_result_V30_t Abayertnr_Prepare_V30(Abayertnr_Context_V30_t *pAbayertnrCtx, Abayertnr_Config_V30_t* pANRConfig);

//anr reconfig
Abayertnr_result_V30_t Abayertnr_ReConfig_V30(Abayertnr_Context_V30_t *pAbayertnrCtx, Abayertnr_Config_V30_t* pANRConfig);

Abayertnr_result_V30_t Abayertnr_IQParaUpdate_V30(Abayertnr_Context_V30_t *pAbayertnrCtx);

//anr preprocess
Abayertnr_result_V30_t Abayertnr_PreProcess_V30(Abayertnr_Context_V30_t *pAbayertnrCtx);

//anr process
Abayertnr_result_V30_t Abayertnr_Process_V30(Abayertnr_Context_V30_t *pAbayertnrCtx, Abayertnr_ExpInfo_V30_t *pExpInfo);

//anr get result
Abayertnr_result_V30_t Abayertnr_GetProcResult_V30(Abayertnr_Context_V30_t *pAbayertnrCtx, Abayertnr_ProcResult_V30_t* pANRResult);

Abayertnr_result_V30_t Abayertnr_ConfigSettingParam_V30(Abayertnr_Context_V30_t *pAbayertnrCtx, Abayertnr_ParamMode_V30_t eParamMode, int snr_mode);

Abayertnr_result_V30_t Abayertnr_ParamModeProcess_V30(Abayertnr_Context_V30_t *pAbayertnrCtx, Abayertnr_ExpInfo_V30_t *pExpInfo, Abayertnr_ParamMode_V30_t *mode);


RKAIQ_END_DECLARE

#endif
