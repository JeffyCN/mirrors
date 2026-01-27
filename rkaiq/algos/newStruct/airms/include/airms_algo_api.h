#ifndef _AIRMS_ALGO_API_H_
#define _AIRMS_ALGO_API_H_

#include "rk_aiq_algo_des.h"
#include "isp/rk_aiq_isp_airms.h"
#include "algos/rk_aiq_api_types_airms.h"

RKAIQ_BEGIN_DECLARE

XCamReturn Airms_processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams, int iso);

extern RkAiqAlgoDescription g_RkIspAlgoDescAirms;
RKAIQ_END_DECLARE

#endif
