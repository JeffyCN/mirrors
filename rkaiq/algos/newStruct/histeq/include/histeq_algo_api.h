#ifndef _HISTEQ_ALOG_API_H_
#define _HISTEQ_ALOG_API_H_

#include "rk_aiq_algo_des.h"

#if RKAIQ_HAVE_DEHAZE_V14
#include "isp/rk_aiq_isp_histeq23.h"
#elif RKAIQ_HAVE_HISTEQ_V10
#include "isp/rk_aiq_isp_histeq30.h"
#else
#error "wrong histeq hw version !"
#endif

#if RKAIQ_HAVE_DEHAZE || RKAIQ_HAVE_HISTEQ
#include "algos/rk_aiq_api_types_histeq.h"
#endif

XCAM_BEGIN_DECLARE

#if 0
XCamReturn
algo_histeq_GetAttrib
(
    RkAiqAlgoContext* ctx,
    histeq_api_attrib_t* attr
);

XCamReturn
algo_histeq_SetAttrib
(
    RkAiqAlgoContext* ctx,
    histeq_api_attrib_t *attr
);
#endif

XCamReturn Ahisteq_processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams, int iso);

extern RkAiqAlgoDescription g_RkIspAlgoDescHisteq;
XCAM_END_DECLARE

#endif
