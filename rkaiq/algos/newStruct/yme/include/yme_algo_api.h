#ifndef _YME_ALGO_API_H_
#define _YME_ALGO_API_H_

#include "rk_aiq_algo_des.h"
#if RKAIQ_HAVE_YUVME_V1
#include "isp/rk_aiq_isp_yme10.h"
#else
#error "wrong yme hw version !"
#endif
#include "algos/rk_aiq_api_types_yme.h"

RKAIQ_BEGIN_DECLARE
#if 0
XCamReturn
algo_yme_SetAttrib
(
    RkAiqAlgoContext *ctx,
    const yme_api_attrib_t *attr,
    bool need_sync
);

XCamReturn
algo_yme_GetAttrib(
    const RkAiqAlgoContext *ctx,
    yme_api_attrib_t *attr
);
#endif
XCamReturn YmeSelectParam(yme_param_auto_t *pAuto, yme_param_t* out, int iso);

extern RkAiqAlgoDescription g_RkIspAlgoDescYme;
RKAIQ_END_DECLARE

#endif
