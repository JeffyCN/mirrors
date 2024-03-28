#ifndef _YNR_ALGO_API_H_
#define _YNR_ALGO_API_H_

#include "rk_aiq_algo_des.h"
#if RKAIQ_HAVE_YNR_V22
#include "isp/rk_aiq_isp_ynr32.h"
#elif RKAIQ_HAVE_YNR_V24
#include "isp/rk_aiq_isp_ynr34.h"
#else
#error "wrong ynr hw version !"
#endif
#include "algos/rk_aiq_api_types_ynr.h"

XCamReturn
algo_ynr_SetAttrib
(
    RkAiqAlgoContext *ctx,
    const ynr_api_attrib_t *attr,
    bool need_sync
);

XCamReturn
algo_ynr_GetAttrib(
    const RkAiqAlgoContext *ctx,
    ynr_api_attrib_t *attr
);

XCamReturn YnrSelectParam(ynr_param_auto_t *pAuto, ynr_param_t* out, int iso);

RKAIQ_BEGIN_DECLARE
extern RkAiqAlgoDescription g_RkIspAlgoDescYnr;
RKAIQ_END_DECLARE

#endif
