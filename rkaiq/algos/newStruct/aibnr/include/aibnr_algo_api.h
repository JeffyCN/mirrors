#ifndef _AIBNR_ALGO_API_H_
#define _AIBNR_ALGO_API_H_

#include "rk_aiq_algo_des.h"
#include "isp/rk_aiq_isp_aibnr.h"
#include "algos/rk_aiq_api_types_aibnr.h"

RKAIQ_BEGIN_DECLARE

XCamReturn Aibnr_processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams, int iso);

extern RkAiqAlgoDescription g_RkIspAlgoDescAibnr;
RKAIQ_END_DECLARE

#endif
