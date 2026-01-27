#ifndef _IE_ALGO_API_H_
#define _IE_ALGO_API_H_

#include "rk_aiq_algo_des.h"
#if RKAIQ_HAVE_AIE_V10
#include "isp/rk_aiq_isp_ie10.h"
#else
#error "wrong ie hw version !"
#endif
#include "algos/rk_aiq_api_types_ie.h"

RKAIQ_BEGIN_DECLARE
#if 0
XCamReturn
algo_ie_SetAttrib
(
    RkAiqAlgoContext *ctx,
    const ie_api_attrib_t *attr,
    bool need_sync
);

XCamReturn
algo_ie_GetAttrib(
    const RkAiqAlgoContext *ctx,
    ie_api_attrib_t *attr
);
#endif
XCamReturn IeSelectParam(ie_param_auto_t *pAuto, ie_param_t* out);

extern RkAiqAlgoDescription g_RkIspAlgoDescIe;
RKAIQ_END_DECLARE

#endif
