#ifndef _CP_ALGO_API_H_
#define _CP_ALGO_API_H_

#include "rk_aiq_algo_des.h"
#if RKAIQ_HAVE_ACP_V10
#include "isp/rk_aiq_isp_cp10.h"
#else
#error "wrong cp hw version !"
#endif
#include "algos/rk_aiq_api_types_cp.h"

RKAIQ_BEGIN_DECLARE
XCamReturn
algo_cp_SetAttrib
(
    RkAiqAlgoContext *ctx,
    const cp_api_attrib_t *attr,
    bool need_sync
);

XCamReturn
algo_cp_GetAttrib(
    const RkAiqAlgoContext *ctx,
    cp_api_attrib_t *attr
);

XCamReturn CpSelectParam(cp_param_auto_t *pAuto, cp_param_t* out, int iso);

extern RkAiqAlgoDescription g_RkIspAlgoDescCp;
RKAIQ_END_DECLARE

#endif
