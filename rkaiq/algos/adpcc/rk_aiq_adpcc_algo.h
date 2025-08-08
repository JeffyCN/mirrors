#ifndef _RK_AIQ_ADPCC_H_
#define _RK_AIQ_ADPCC_H_

#include "stdio.h"
#include "string.h"
#include "math.h"
#include "base/xcam_log.h"
#include "rk_aiq_comm.h"
#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"
#include "adpcc/rk_aiq_types_adpcc_algo_prvt.h"


AdpccResult_t AdpccReloadPara(AdpccContext_t *pAdpccCtx, CamCalibDbV2Context_t *pCalibDb);
AdpccResult_t AdpccInit(AdpccContext_t **ppAdpccCtx, CamCalibDbV2Context_t *pCalibDb);

AdpccResult_t AdpccRelease(AdpccContext_t *pAdpccCtx);

AdpccResult_t AdpccConfig(AdpccContext_t *pAdpccCtx, AdpccConfig_t* pAdpccConfig);

AdpccResult_t AdpccReConfig(AdpccContext_t *pAdpccCtx, AdpccConfig_t* pAdpccConfig);

AdpccResult_t AdpccPreProcess(AdpccContext_t *pAdpccCtx);

AdpccResult_t AdpccProcess(AdpccContext_t *pAdpccCtx, AdpccExpInfo_t *pExpInfo, AdpccProcResult_t* pAdpccResult);

#endif
