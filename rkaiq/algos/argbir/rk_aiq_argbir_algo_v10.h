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
#ifndef __RK_AIQ_ARGBIR_ALGO_V10_H__
#define __RK_AIQ_ARGBIR_ALGO_V10_H__

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "ae/rk_aiq_types_ae_algo_int.h"
#include "rk_aiq_types_argbir_algo_prvt.h"

XCamReturn aRgbirStart(aRgbirContext_t* pARgbirCtx);
XCamReturn aRgbirStop(aRgbirContext_t* pARgbirCtx);
bool RgbirEnableSetting(aRgbirContext_t* pARgbirCtx, RkAiqArgbirProcResult_t* pArgbirProcRes);
void ArgbirTuningParaProcessing(aRgbirContext_t* pARgbirCtx,
                                RkAiqArgbirProcResult_t* pArgbirProcRes);
bool ArgbirByPassTuningProcessing(aRgbirContext_t* pARgbirCtx);
XCamReturn aRgbirInit(aRgbirContext_t** ppARgbirCtx, CamCalibDbV2Context_t* pCalibDb);
XCamReturn aRgbirRelease(aRgbirContext_t* pARgbirCtx);

#endif
