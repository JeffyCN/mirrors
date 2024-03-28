#ifndef _DRC_ALOG_API_H_
#define _DRC_ALOG_API_H_

#include "rk_aiq_algo_des.h"

#if RKAIQ_HAVE_DRC_V12
#include "isp/rk_aiq_isp_drc32.h"
#elif RKAIQ_HAVE_DRC_V20
#include "isp/rk_aiq_isp_drc40.h"
#else
#error "wrong drc hw version !"
#endif
#include "algos/rk_aiq_api_types_drc.h"
#include "isp/rk_aiq_isp_trans10.h"
#include "algos/rk_aiq_api_types_trans.h"

XCamReturn
algo_drc_SetAttrib
(
    RkAiqAlgoContext* ctx,
    drc_api_attrib_t *attr
);

XCamReturn
algo_drc_GetAttrib
(
    RkAiqAlgoContext*  ctx,
    drc_api_attrib_t *attr
);


XCAM_BEGIN_DECLARE
extern RkAiqAlgoDescription g_RkIspAlgoDescDrc;
XCAM_END_DECLARE

#endif
