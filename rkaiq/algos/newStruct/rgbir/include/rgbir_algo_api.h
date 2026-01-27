#ifndef _RGBIR_ALGO_API_H_
#define _RGBIR_ALGO_API_H_

#include "rk_aiq_algo_des.h"
#if RKAIQ_HAVE_RGBIR_REMOSAIC
#include "isp/rk_aiq_isp_rgbir10.h"
#else
#error "wrong rgbir hw version !"
#endif
#include "algos/rk_aiq_api_types_rgbir.h"

RKAIQ_BEGIN_DECLARE
#if 0
XCamReturn
algo_rgbir_SetAttrib
(
    RkAiqAlgoContext *ctx,
    const rgbir_api_attrib_t *attr,
    bool need_sync
);

XCamReturn
algo_rgbir_GetAttrib(
    const RkAiqAlgoContext *ctx,
    rgbir_api_attrib_t *attr
);
#endif
//XCamReturn RgbirSelectParam(rgbir_param_auto_t *pAuto, rgbir_param_t* out, int iso);
XCamReturn Argbir_processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams, int iso);

extern RkAiqAlgoDescription g_RkIspAlgoDescRgbir;
RKAIQ_END_DECLARE

#endif
