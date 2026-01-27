#ifndef _AIYNR_ALGO_API_H_
#define _AIYNR_ALGO_API_H_

#include "rk_aiq_algo_des.h"
#include "isp/rk_aiq_isp_aiynr.h"
#include "algos/rk_aiq_api_types_aiynr.h"

RKAIQ_BEGIN_DECLARE

XCamReturn Aiynr_processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams, int iso);

extern RkAiqAlgoDescription g_RkIspAlgoDescAiynr;
RKAIQ_END_DECLARE

#endif
