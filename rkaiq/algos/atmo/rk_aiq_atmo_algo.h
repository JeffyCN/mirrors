/******************************************************************************
 *
 * Copyright 2019, Fuzhou Rockchip Electronics Co.Ltd . All rights reserved.
 * No part of this work may be reproduced, modified, distributed, transmitted,
 * transcribed, or translated into any language or computer format, in any form
 * or by any means without written permission of:
 * Fuzhou Rockchip Electronics Co.Ltd .
 *
 *
 *****************************************************************************/
#ifndef __RK_AIQ_ATMO_ALGO_H__
#define __RK_AIQ_ATMO_ALGO_H__

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "ae/rk_aiq_types_ae_algo_int.h"
#include "af/rk_aiq_af_hw_v200.h"
#include "rk_aiq_uapi_atmo_int.h"
#include "rk_aiq_types_atmo_algo_prvt.h"


RESULT AtmoStart(AtmoHandle_t pAtmoCtx);
RESULT AtmoStop(AtmoHandle_t pAtmoCtx);
void AtmoGetStats(AtmoHandle_t pAtmoCtx, rkisp_atmo_stats_t* ROData) ;
void ApiOffProcess(AtmoHandle_t pAtmoCtx);
void AtmoGetSensorInfo( AtmoHandle_t pAtmoCtx, AecProcResult_t AecHdrProcResult);
void newMalloc(AtmoConfig_t* pAtmoConfig, CalibDbV2_tmo_t* pCalibDb);
void AtmoUpdateConfig(AtmoHandle_t pAtmoCtx, CalibDbV2_tmo_t* pCalibDb);
void AtmoProcessing(AtmoHandle_t pAtmoCtx, AecPreResult_t AecHdrPreResult, af_preprocess_result_t AfPreResult);
bool SetGlobalTMO(AtmoHandle_t pAtmoCtx);
RESULT AtmoInit(AtmoInstanceConfig_t* pInstConfig, CamCalibDbV2Context_t* pCalibV2) ;
RESULT AtmoRelease(AtmoHandle_t pAtmoCtx) ;

#endif
