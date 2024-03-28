#ifndef _BAYERTNR_ALGO_API_H_
#define _BAYERTNR_ALGO_API_H_

#include "rk_aiq_algo_des.h"
#if RKAIQ_HAVE_BAYERTNR_V23
#include "isp/rk_aiq_isp_btnr32.h"
#include "algos/rk_aiq_api_types_btnr32.h"
#elif RKAIQ_HAVE_BAYERTNR_V30
#include "isp/rk_aiq_isp_btnr40.h"
#include "algos/rk_aiq_api_types_btnr40.h"
#else
#error "wrong btnr hw version !"
#endif

XCamReturn
algo_bayertnr_SetAttrib
(
    RkAiqAlgoContext *ctx,
    const btnr_api_attrib_t *attr,
    bool need_sync
);

XCamReturn
algo_bayertnr_GetAttrib(
    const RkAiqAlgoContext *ctx,
    btnr_api_attrib_t *attr
);

XCamReturn BtnrSelectParam(btnr_param_auto_t *pAuto, btnr_param_t* out, int iso);

RKAIQ_BEGIN_DECLARE
extern RkAiqAlgoDescription g_RkIspAlgoDescBayertnr;
RKAIQ_END_DECLARE

#endif
