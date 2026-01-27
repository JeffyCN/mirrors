#ifndef _MERGE_ALOG_API_H_
#define _MERGE_ALOG_API_H_

#include "rk_aiq_algo_des.h"

#if RKAIQ_HAVE_MERGE_V12
#include "isp/rk_aiq_isp_merge22.h"
#elif RKAIQ_HAVE_MERGE_V13
#include "isp/rk_aiq_isp_merge23.h"
#else
#error "wrong mge hw version !"
#endif

#include "algos/rk_aiq_api_types_merge.h"

typedef enum MergeHwVersion_e
{
    MERGE_HARDWARE_V10 = 0,  //rk1126/1109/356X/
    MERGE_HARDWARE_V11 = 1,  //rk3588
    MERGE_HARDWARE_V12 = 2,  //rk3562 rk1106
    MERGE_HARDWARE_MAX,
} MergeHwVersion_t;

XCAM_BEGIN_DECLARE

#if 0
XCamReturn
algo_merge_SetAttrib
(
    RkAiqAlgoContext* ctx,
    mge_api_attrib_t *attr
);

XCamReturn
algo_merge_GetAttrib
(
    RkAiqAlgoContext*  ctx,
    mge_api_attrib_t *attr
);
#endif
extern RkAiqAlgoDescription g_RkIspAlgoDescMerge;

XCAM_END_DECLARE

#endif
