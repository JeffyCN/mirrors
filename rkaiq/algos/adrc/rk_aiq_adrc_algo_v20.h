/******************************************************************************
 *
 * Copyright 2024, Fuzhou Rockchip Electronics Co.Ltd . All rights reserved.
 * No part of this work may be reproduced, modified, distributed, transmitted,
 * transcribed, or translated into any language or computer format, in any form
 * or by any means without written permission of:
 * Fuzhou Rockchip Electronics Co.Ltd .
 *
 *
 *****************************************************************************/
#ifndef __RK_AIQ_ADRC_ALGO_V20_H__
#define __RK_AIQ_ADRC_ALGO_V20_H__

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "ae/rk_aiq_types_ae_algo_int.h"
#include "rk_aiq_types_adrc_algo_prvt.h"

XCamReturn AdrcStart(AdrcContext_t* pAdrcCtx);
XCamReturn AdrcStop(AdrcContext_t* pAdrcCtx);
bool DrcEnableSetting(AdrcContext_t* pAdrcCtx, RkAiqAdrcProcResult_t* pAdrcProcRes);
void AdrcTuningParaProcessing(AdrcContext_t* pAdrcCtx, RkAiqAdrcProcResult_t* pAdrcProcRes);
void AdrcExpoParaProcessing(AdrcContext_t* pAdrcCtx, RkAiqAdrcProcResult_t* pAdrcProcRes);
bool AdrcByPassTuningProcessing(AdrcContext_t* pAdrcCtx);
void AdrcV20Params2Api(AdrcContext_t* pAdrcCtx, DrcInfoV20_t* pDrcInfo);
void AdrcV20ClipStAutoParams(AdrcContext_t* pAdrcCtx);
XCamReturn AdrcInit(AdrcContext_t** ppAdrcCtx, CamCalibDbV2Context_t* pCalibDb);
XCamReturn AdrcRelease(AdrcContext_t* pAdrcCtx);

#endif
