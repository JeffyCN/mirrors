#ifndef _BAYERTNR_ALGO_API_H_
#define _BAYERTNR_ALGO_API_H_

#include "rk_aiq_algo_des.h"
#if RKAIQ_HAVE_BAYERTNR_V23
#include "isp/rk_aiq_isp_btnr32.h"
#include "algos/rk_aiq_api_types_btnr32.h"
#elif RKAIQ_HAVE_BAYERTNR_V30
#include "isp/rk_aiq_isp_btnr40.h"
#include "algos/rk_aiq_api_types_btnr40.h"
#elif RKAIQ_HAVE_BAYERTNR_V41
#include "isp/rk_aiq_isp_btnr41.h"
#include "algos/rk_aiq_api_types_btnr41.h"
#elif RKAIQ_HAVE_BAYERTNR_V42
#include "isp/rk_aiq_isp_btnr42.h"
#include "algos/rk_aiq_api_types_btnr42.h"
#else
#error "wrong btnr hw version !"
#endif

RKAIQ_BEGIN_DECLARE

#if 0 
XCamReturn
algo_bayertnr_SetAttrib
(
    RkAiqAlgoContext *ctx,
    const btnr_api_attrib_t *attr,
    bool need_sync
);

XCamReturn
algo_bayertnr_GetAttrib(
    const RkAiqAlgoContext *ctx,
    btnr_api_attrib_t *attr
);
#endif
XCamReturn
algo_bayertnr_SetStrength(RkAiqAlgoContext *ctx, float strg, bool strg_en);

XCamReturn
algo_bayertnr_GetStrength(RkAiqAlgoContext *ctx, float *strg, bool *strg_en);

XCamReturn Abtnr_processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams, int iso);

extern RkAiqAlgoDescription g_RkIspAlgoDescBayertnr;
extern RkAiqAlgoDescription g_RkIspAlgoDescBayertnr2;
RKAIQ_END_DECLARE

#endif
