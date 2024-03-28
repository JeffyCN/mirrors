
#ifndef __RKAIQ_ABAYERNR_V23_H__
#define __RKAIQ_ABAYERNR_V23_H__

#include "stdio.h"
#include "string.h"
#include "math.h"
#include "stdlib.h"

#include "rk_aiq_abayertnr_algo_bayertnr_v23.h"
#include "rk_aiq_types_abayertnr_algo_prvt_v23.h"
#include "RkAiqCalibDbV2Helper.h"

RKAIQ_BEGIN_DECLARE

Abayertnr_result_V23_t Abayertnr_Start_V23(Abayertnr_Context_V23_t *pAbayertnrCtx);

Abayertnr_result_V23_t Abayertnr_Stop_V23(Abayertnr_Context_V23_t *pAbayertnrCtx);

//anr inint
Abayertnr_result_V23_t Abayertnr_Init_V23(Abayertnr_Context_V23_t **ppAbayertnrCtx, void *pCalibDb);

//anr release
Abayertnr_result_V23_t Abayertnr_Release_V23(Abayertnr_Context_V23_t *pAbayertnrCtx);

//anr config
Abayertnr_result_V23_t Abayertnr_Prepare_V23(Abayertnr_Context_V23_t *pAbayertnrCtx, Abayertnr_Config_V23_t* pANRConfig);

//anr reconfig
Abayertnr_result_V23_t Abayertnr_ReConfig_V23(Abayertnr_Context_V23_t *pAbayertnrCtx, Abayertnr_Config_V23_t* pANRConfig);

Abayertnr_result_V23_t Abayertnr_IQParaUpdate_V23(Abayertnr_Context_V23_t *pAbayertnrCtx);

//anr preprocess
Abayertnr_result_V23_t Abayertnr_PreProcess_V23(Abayertnr_Context_V23_t *pAbayertnrCtx);

//anr process
Abayertnr_result_V23_t Abayertnr_Process_V23(Abayertnr_Context_V23_t *pAbayertnrCtx, Abayertnr_ExpInfo_V23_t *pExpInfo);

//anr get result
Abayertnr_result_V23_t Abayertnr_GetProcResult_V23(Abayertnr_Context_V23_t *pAbayertnrCtx, Abayertnr_ProcResult_V23_t* pANRResult);

Abayertnr_result_V23_t Abayertnr_ConfigSettingParam_V23(Abayertnr_Context_V23_t *pAbayertnrCtx, Abayertnr_ParamMode_V23_t eParamMode, int snr_mode);

Abayertnr_result_V23_t Abayertnr_ParamModeProcess_V23(Abayertnr_Context_V23_t *pAbayertnrCtx, Abayertnr_ExpInfo_V23_t *pExpInfo, Abayertnr_ParamMode_V23_t *mode);


RKAIQ_END_DECLARE

#endif
