#ifndef _CAC_ALOG_API_H_
#define _CAC_ALOG_API_H_

#include "rk_aiq_algo_des.h"

#if RKAIQ_HAVE_CAC_V11
#include "isp/rk_aiq_isp_cac21.h"
#elif RKAIQ_HAVE_CAC_V30
#include "isp/rk_aiq_isp_cac30.h"
#else
#error "wrong cac hw version !"
#endif

#include "algos/rk_aiq_api_types_cac.h"

XCAM_BEGIN_DECLARE

#if 0
XCamReturn
algo_cac_SetAttrib
(
    RkAiqAlgoContext* ctx,
    cac_api_attrib_t *attr
);

XCamReturn
algo_cac_GetAttrib
(
    RkAiqAlgoContext*  ctx,
    cac_api_attrib_t *attr
);
#endif
extern RkAiqAlgoDescription g_RkIspAlgoDescCac;
XCAM_END_DECLARE

#endif
