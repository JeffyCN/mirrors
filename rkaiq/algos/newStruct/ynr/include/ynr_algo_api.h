#ifndef _YNR_ALGO_API_H_
#define _YNR_ALGO_API_H_

#include "rk_aiq_algo_des.h"
#if RKAIQ_HAVE_YNR_V22
#include "isp/rk_aiq_isp_ynr32.h"
#elif RKAIQ_HAVE_YNR_V24
#include "isp/rk_aiq_isp_ynr34.h"
#elif RKAIQ_HAVE_YNR_V40
#include "isp/rk_aiq_isp_ynr40.h"
#elif RKAIQ_HAVE_YNR_V41
#include "isp/rk_aiq_isp_ynr41.h"
#else
#error "wrong ynr hw version !"
#endif
#include "algos/rk_aiq_api_types_ynr.h"

RKAIQ_BEGIN_DECLARE
#if 0
XCamReturn
algo_ynr_SetAttrib
(
    RkAiqAlgoContext *ctx,
    const ynr_api_attrib_t *attr
);

XCamReturn
algo_ynr_GetAttrib(
    const RkAiqAlgoContext *ctx,
    ynr_api_attrib_t *attr
);
#endif


XCamReturn
algo_ynr_SetStrength(RkAiqAlgoContext *ctx, float strg, bool strg_en);

XCamReturn
algo_ynr_GetStrength(RkAiqAlgoContext *ctx, float *strg, bool *strg_en);

XCamReturn Aynr_processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams, int iso);

extern RkAiqAlgoDescription g_RkIspAlgoDescYnr;
RKAIQ_END_DECLARE

#endif
