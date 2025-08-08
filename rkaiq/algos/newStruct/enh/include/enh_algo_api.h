#ifndef _ENHANCE_ALOG_API_H_
#define _ENHANCE_ALOG_API_H_

#include "rk_aiq_algo_des.h"

#if RKAIQ_HAVE_ENHANCE_V10
#include "isp/rk_aiq_isp_enh30.h"
#else
#error "wrong enh hw version !"
#endif

XCAM_BEGIN_DECLARE
#if 0
XCamReturn
algo_enh_GetAttrib
(
    RkAiqAlgoContext* ctx,
    enh_api_attrib_t* attr
);

XCamReturn
algo_enh_SetAttrib
(
    RkAiqAlgoContext* ctx,
    enh_api_attrib_t *attr
);
#endif
XCamReturn Aenh_processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams, int iso);
XCamReturn algo_enh_SetStrength(RkAiqAlgoContext* ctx, aenh_strength_t *strg);
XCamReturn algo_enh_GetStrength(RkAiqAlgoContext* ctx, aenh_strength_t *strg);

extern RkAiqAlgoDescription g_RkIspAlgoDescEnh;
XCAM_END_DECLARE

#endif
