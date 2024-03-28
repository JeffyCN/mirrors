#ifndef _BLC_ALOG_API_H_
#define _BLC_ALOG_API_H_

#include "rk_aiq_algo_des.h"

#if RKAIQ_HAVE_BLC_V32
#include "isp/rk_aiq_isp_blc30.h"
#else
#error "wrong blc hw version !"
#endif
#include "algos/rk_aiq_api_types_blc.h"

typedef enum BlcHwVersion_e
{
    BLC_HARDWARE_V1 = 0,  // rk1126/1109/356X/3588
    BLC_HARDWARE_V2 = 1,  // rv1106/rk3562/rk3576
    BLC_HARDWARE_MAX,
} BlcHwVersion_t;

XCamReturn
algo_blc_SetAttrib
(
    RkAiqAlgoContext* ctx,
    blc_api_attrib_t *attr
);

XCamReturn
algo_blc_GetAttrib
(
    RkAiqAlgoContext*  ctx,
    blc_api_attrib_t *attr
);

XCAM_BEGIN_DECLARE
extern RkAiqAlgoDescription g_RkIspAlgoDescBlc;
XCAM_END_DECLARE

#endif
