#ifndef _CGC_ALGO_API_H_
#define _CGC_ALGO_API_H_

#include "rk_aiq_algo_des.h"
#if RKAIQ_HAVE_CGC_V1
#include "isp/rk_aiq_isp_cgc10.h"
#else
#error "wrong cgc hw version !"
#endif
#include "algos/rk_aiq_api_types_cgc.h"

RKAIQ_BEGIN_DECLARE
#if 0
XCamReturn
algo_cgc_SetAttrib
(
    RkAiqAlgoContext *ctx,
    const cgc_api_attrib_t *attr,
    bool need_sync
);

XCamReturn
algo_cgc_GetAttrib(
    const RkAiqAlgoContext *ctx,
    cgc_api_attrib_t *attr
);
#endif
extern RkAiqAlgoDescription g_RkIspAlgoDescCgc;
RKAIQ_END_DECLARE

#endif
