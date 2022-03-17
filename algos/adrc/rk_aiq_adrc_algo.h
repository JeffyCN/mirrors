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
#ifndef __RK_AIQ_ADRC_ALGO_H__
#define __RK_AIQ_ADRC_ALGO_H__

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "ae/rk_aiq_types_ae_algo_int.h"
#include "af/rk_aiq_af_hw_v200.h"
//#include "rk_aiq_uapi_adrc_int.h"
#include "rk_aiq_types_adrc_algo_prvt.h"


XCamReturn AdrcStart(AdrcHandle_t pAdrcCtx);
XCamReturn AdrcStop(AdrcHandle_t pAdrcCtx);
void DrcNewMalloc(AdrcConfig_t* pConfig, CalibDbV2_drc_t* pCalibDb);
void AdrcApiOffConfig(AdrcHandle_t pAdrcCtx);
void AdrcIQUpdate(AdrcHandle_t pAdrcCtx, AecPreResult_t AecHdrPreResult, af_preprocess_result_t AfPreResult);
void AdrcGetSensorInfo( AdrcHandle_t pAdrcCtx, AecProcResult_t AecHdrProcResult);
void AdrcUpdateConfig(AdrcHandle_t pAdrcCtx, CalibDbV2_drc_t* pCalibDb);
void AdrcProcessing(AdrcHandle_t pAdrcCtx);
bool AdrcByPassProcessing(AdrcHandle_t pAdrcCtx, AecPreResult_t AecHdrPreResult);
bool DrcSetGlobalTMO(AdrcHandle_t pAdrcCtx);
XCamReturn AdrcInit(AdrcInstanceConfig_t* pInstConfig, CamCalibDbV2Context_t* calibv2) ;
XCamReturn AdrcRelease(AdrcHandle_t pAdrcCtx) ;


#endif
