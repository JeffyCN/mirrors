#ifndef _SHARP_ALGO_API_H_
#define _SHARP_ALGO_API_H_

#include "rk_aiq_algo_des.h"
#if RKAIQ_HAVE_SHARP_V33
#include "isp/rk_aiq_isp_sharp32.h"
#elif RKAIQ_HAVE_SHARP_V34
#include "isp/rk_aiq_isp_sharp34.h"
#elif RKAIQ_HAVE_SHARP_V40
#include "isp/rk_aiq_isp_sharp40.h"
#include "isp/rk_aiq_isp_texEst40.h"
#elif RKAIQ_HAVE_SHARP_V41
#include "isp/rk_aiq_isp_sharp41.h"
#include "isp/rk_aiq_isp_texEst40.h"
#else
#error "wrong sharp hw version !"
#endif
#include "algos/rk_aiq_api_types_sharp.h"

RKAIQ_BEGIN_DECLARE

#if 0
XCamReturn
algo_sharp_SetAttrib
(
    RkAiqAlgoContext *ctx,
    const sharp_api_attrib_t *attr
);

XCamReturn
algo_sharp_GetAttrib(
    const RkAiqAlgoContext *ctx,
    sharp_api_attrib_t *attr
);
#endif

XCamReturn
algo_sharp_SetStrength(RkAiqAlgoContext *ctx, float strg, bool strg_en);

XCamReturn
algo_sharp_GetStrength(RkAiqAlgoContext *ctx, float *strg, bool *strg_en);

XCamReturn Asharp_processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams, int iso);
XCamReturn AtexEst_processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams, int iso);

extern RkAiqAlgoDescription g_RkIspAlgoDescSharp;
RKAIQ_END_DECLARE

#endif
