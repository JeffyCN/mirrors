#ifndef _GIC_ALGO_API_H_
#define _GIC_ALGO_API_H_

#include "rk_aiq_algo_des.h"
#if RKAIQ_HAVE_GIC_V2
#include "isp/rk_aiq_isp_gic21.h"
#elif RKAIQ_HAVE_GIC_V3
#include "isp/rk_aiq_isp_gic30.h"
#else
#error "wrong gic hw version !"
#endif

#include "algos/rk_aiq_api_types_gic.h"

RKAIQ_BEGIN_DECLARE

#if 0
XCamReturn
algo_gic_GetAttrib(
    const RkAiqAlgoContext *ctx,
    gic_api_attrib_t *attr
);
#endif
//XCamReturn GicSelectParam(gic_param_auto_t *pAuto, gic_param_t* out, int iso);
XCamReturn Agic_processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams, int iso);

extern RkAiqAlgoDescription g_RkIspAlgoDescGic;
RKAIQ_END_DECLARE

#endif
