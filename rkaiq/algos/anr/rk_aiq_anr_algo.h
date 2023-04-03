
#ifndef __RKAIQ_ANR_H__
#define __RKAIQ_ANR_H__

#include "stdio.h"
#include "string.h"
#include "math.h"

#include "rk_aiq_anr_algo_uvnr.h"
#include "rk_aiq_anr_algo_ynr.h"
#include "rk_aiq_anr_algo_mfnr.h"
#include "rk_aiq_anr_algo_bayernr.h"
#include "rk_aiq_types_anr_algo_prvt.h"
#include "rk_aiq_anr_algo_gain.h"

RKAIQ_BEGIN_DECLARE

ANRresult_t ANRStart(ANRContext_t *pANRCtx);

ANRresult_t ANRStop(ANRContext_t *pANRCtx); 

//anr inint
ANRresult_t ANRInit(ANRContext_t **ppANRCtx, CamCalibDbContext_t *pCalibDb);

//anr release
ANRresult_t ANRRelease(ANRContext_t *pANRCtx);

//anr config
ANRresult_t ANRPrepare(ANRContext_t *pANRCtx, ANRConfig_t* pANRConfig);

//anr reconfig
ANRresult_t ANRReConfig(ANRContext_t *pANRCtx, ANRConfig_t* pANRConfig);

ANRresult_t ANRIQParaUpdate(ANRContext_t *pANRCtx);

//anr preprocess
ANRresult_t ANRPreProcess(ANRContext_t *pANRCtx);

//anr process
ANRresult_t ANRProcess(ANRContext_t *pANRCtx, ANRExpInfo_t *pExpInfo);

//anr get result
ANRresult_t ANRGetProcResult(ANRContext_t *pANRCtx, ANRProcResult_t* pANRResult);

ANRresult_t ANRGainRatioProcess(ANRGainState_t *pGainState, ANRExpInfo_t *pExpInfo);

ANRresult_t ANRConfigSettingParam(ANRContext_t *pANRCtx, ANRParamMode_t eParamMode, int snr_mode);

ANRresult_t ANRParamModeProcess(ANRContext_t *pANRCtx, ANRExpInfo_t *pExpInfo, ANRParamMode_t *mode);


ANRresult_t ANRInit_json(ANRContext_t **ppANRCtx, CamCalibDbV2Context_t *pCalibDbV2);

ANRresult_t ANRConfigParamJson(ANRContext_t *pANRCtx, ANRParamMode_t eParamMode, int snr_mode);


RKAIQ_END_DECLARE

#endif
