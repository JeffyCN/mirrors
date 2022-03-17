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
#ifndef __RK_AIQ_AMERGE_ALGO_H__
#define __RK_AIQ_AMERGE_ALGO_H__

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "ae/rk_aiq_types_ae_algo_int.h"
#include "rk_aiq_uapi_amerge_int.h"
#include "rk_aiq_types_amerge_algo_prvt.h"

RESULT AmergeStart(AmergeHandle_t pAmergeCtx);
RESULT AmergeStop(AmergeHandle_t pAmergeCtx);
void AmergeIQUpdate(AmergeHandle_t pAmergeCtx);
void AmergeGetSensorInfo( AmergeHandle_t pAmergeCtx, AecProcResult_t AecHdrProcResult);
void MergeNewMalloc(AmergeConfig_t* pAmergeConfig, CalibDbV2_merge_t* pCalibDb);
void AmergeUpdateConfig(AmergeHandle_t pAmergeCtx, CalibDbV2_merge_t* pCalibDb);
void AmergeProcessing(AmergeHandle_t pAmergeCtx);
bool AmergeByPassProcessing(AmergeHandle_t pAmergeCtx, AecPreResult_t AecHdrPreResult);
RESULT AmergeInit(AmergeInstanceConfig_t* pInstConfig, CamCalibDbV2Context_t* pCalibV2) ;
RESULT AmergeRelease(AmergeHandle_t pAmergeCtx) ;


#endif
