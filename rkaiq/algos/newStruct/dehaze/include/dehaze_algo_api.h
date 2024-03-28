#ifndef _DEHAZE_ALOG_API_H_
#define _DEHAZE_ALOG_API_H_

#include "rk_aiq_algo_des.h"

#if RKAIQ_HAVE_DEHAZE_V12
#include "isp/rk_aiq_isp_dehaze22.h"
#elif RKAIQ_HAVE_DEHAZE_V14
#include "isp/rk_aiq_isp_dehaze23.h"
#else
#error "wrong dehaze hw version !"
#endif
#include "algos/rk_aiq_api_types_dehaze.h"

#define YNR_ISO_CURVE_POINT_BIT          4
#define YNR_ISO_CURVE_POINT_NUM          ((1 << YNR_ISO_CURVE_POINT_BIT)+1)

XCamReturn
algo_dehaze_SetAttrib
(
    RkAiqAlgoContext* ctx,
    dehaze_api_attrib_t *attr
);

XCamReturn
algo_dehaze_GetAttrib
(
    RkAiqAlgoContext*  ctx,
    dehaze_api_attrib_t *attr
);


XCAM_BEGIN_DECLARE
extern RkAiqAlgoDescription g_RkIspAlgoDescDehaze;
XCAM_END_DECLARE

#endif
