#ifndef _CSM_ALOG_API_H_
#define _CSM_ALOG_API_H_

#include "rk_aiq_algo_des.h"

#if RKAIQ_HAVE_CSM_V1
#include "isp/rk_aiq_isp_csm21.h"
#else
#error "wrong csm hw version !"
#endif
#include "algos/rk_aiq_api_types_csm.h"

typedef enum CsmHwVersion_e
{
    CSM_HARDWARE_V20 = 0,  // rk1126/1109/
    CSM_HARDWARE_V21 = 1,  // 356X/3588/rv1106/rk3562/rk3576
    CSM_HARDWARE_MAX,
} CsmHwVersion_t;

XCAM_BEGIN_DECLARE

#if 0
XCamReturn
algo_csm_SetAttrib
(
    RkAiqAlgoContext* ctx,
    csm_api_attrib_t *attr
);

XCamReturn
algo_csm_GetAttrib
(
    RkAiqAlgoContext*  ctx,
    csm_api_attrib_t *attr
);
#endif
extern RkAiqAlgoDescription g_RkIspAlgoDescCsm;

XCAM_END_DECLARE

#endif
