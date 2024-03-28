#ifndef _CNR_ALGO_API_H_
#define _CNR_ALGO_API_H_

#include "rk_aiq_algo_des.h"
#if RKAIQ_HAVE_CNR_V30
#include "isp/rk_aiq_isp_cnr32.h"
#elif RKAIQ_HAVE_CNR_V31
#include "isp/rk_aiq_isp_cnr34.h"
#else
#error "wrong cnr hw version !"
#endif
#include "algos/rk_aiq_api_types_cnr.h"

XCamReturn
algo_cnr_GetAttrib(
    const RkAiqAlgoContext *ctx,
    cnr_api_attrib_t *attr
);

XCamReturn CnrSelectParam(cnr_param_auto_t *pAuto, cnr_param_t* out, int iso);

RKAIQ_BEGIN_DECLARE
extern RkAiqAlgoDescription g_RkIspAlgoDescCnr;
RKAIQ_END_DECLARE

#endif
