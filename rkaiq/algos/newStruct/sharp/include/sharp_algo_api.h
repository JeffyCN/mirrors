#ifndef _SHARP_ALGO_API_H_
#define _SHARP_ALGO_API_H_

#include "rk_aiq_algo_des.h"
#if RKAIQ_HAVE_SHARP_V33
#include "isp/rk_aiq_isp_sharp32.h"
#elif RKAIQ_HAVE_SHARP_V34
#include "isp/rk_aiq_isp_sharp34.h"
#else
#error "wrong sharp hw version !"
#endif
#include "algos/rk_aiq_api_types_sharp.h"

XCamReturn
algo_sharp_SetAttrib
(
    RkAiqAlgoContext *ctx,
    const sharp_api_attrib_t *attr,
    bool need_sync
);

XCamReturn
algo_sharp_GetAttrib(
    const RkAiqAlgoContext *ctx,
    sharp_api_attrib_t *attr
);

XCamReturn SharpSelectParam(sharp_param_auto_t *pAuto, sharp_param_t* out, int iso);

RKAIQ_BEGIN_DECLARE
extern RkAiqAlgoDescription g_RkIspAlgoDescSharp;
RKAIQ_END_DECLARE

#endif
