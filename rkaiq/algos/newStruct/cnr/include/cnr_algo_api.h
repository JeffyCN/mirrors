#ifndef _CNR_ALGO_API_H_
#define _CNR_ALGO_API_H_

#include "rk_aiq_algo_des.h"
#if RKAIQ_HAVE_CNR_V30
#include "isp/rk_aiq_isp_cnr32.h"
#elif RKAIQ_HAVE_CNR_V31
#include "isp/rk_aiq_isp_cnr34.h"
#elif RKAIQ_HAVE_CNR_V35
#include "isp/rk_aiq_isp_cnr35.h"
#elif RKAIQ_HAVE_CNR_V36
#include "isp/rk_aiq_isp_cnr36.h"
#else
#error "wrong cnr hw version !"
#endif
#include "algos/rk_aiq_api_types_cnr.h"

RKAIQ_BEGIN_DECLARE

#if 0
XCamReturn
algo_cnr_GetAttrib(
    const RkAiqAlgoContext *ctx,
    cnr_api_attrib_t *attr
);

XCamReturn
algo_cnr_SetAttrib(RkAiqAlgoContext *ctx,
                                    const cnr_api_attrib_t *attr,
                                    bool need_sync);

#endif
XCamReturn
algo_cnr_SetStrength(RkAiqAlgoContext *ctx, float strg, bool strg_en);

XCamReturn
algo_cnr_GetStrength(RkAiqAlgoContext *ctx, float *strg, bool *strg_en);

XCamReturn Acnr_processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams, int iso);

extern RkAiqAlgoDescription g_RkIspAlgoDescCnr;
RKAIQ_END_DECLARE

#endif
