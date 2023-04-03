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
#ifndef __RK_AIQ_ADRC_ALGO_V11_H__
#define __RK_AIQ_ADRC_ALGO_V11_H__

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "ae/rk_aiq_types_ae_algo_int.h"
#include "af/rk_aiq_af_hw_v200.h"
#include "rk_aiq_types_adrc_algo_prvt.h"

XCamReturn AdrcStart(AdrcContext_t* pAdrcCtx);
XCamReturn AdrcStop(AdrcContext_t* pAdrcCtx);
bool DrcEnableSetting(AdrcContext_t* pAdrcCtx);
void AdrcApiOffConfig(AdrcContext_t* pAdrcCtx);
void AdrcIQUpdate(AdrcContext_t* pAdrcCtx, AecPreResult_t AecHdrPreResult,
                  af_preprocess_result_t AfPreResult);
void AdrcTuningParaProcessing(AdrcContext_t* pAdrcCtx);
void AdrcExpoParaProcessing(AdrcContext_t* pAdrcCtx);
bool AdrcByPassTuningProcessing(AdrcContext_t* pAdrcCtx, AecPreResult_t AecHdrPreResult);
bool DrcSetGlobalTMO(AdrcContext_t* pAdrcCtx);
XCamReturn AdrcInit(AdrcContext_t** ppAdrcCtx, CamCalibDbV2Context_t* pCalibDb);
XCamReturn AdrcRelease(AdrcContext_t* pAdrcCtx);

#endif
