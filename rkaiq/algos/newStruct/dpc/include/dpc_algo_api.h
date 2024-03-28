#ifndef _DPC_ALOG_API_H_
#define _DPC_ALOG_API_H_

#include "rk_aiq_algo_des.h"
#if RKAIQ_HAVE_DPCC_V1
#include "isp/rk_aiq_isp_dpcc20.h"
#elif RKAIQ_HAVE_DPCC_V2
#include "isp/rk_aiq_isp_dpc21.h"
#else
#error "wrong dpcc hw version !"
#endif
#include "algos/rk_aiq_api_types_dpc.h"

typedef enum DpcHwVersion_e
{
    DPC_HARDWARE_V1 = 0,
    DPC_HARDWARE_MAX,
} DpcHwVersion_t;

XCamReturn
algo_dpc_SetAttrib
(
    RkAiqAlgoContext* ctx,
    dpc_api_attrib_t *attr
);

XCamReturn
algo_dpc_GetAttrib
(
    RkAiqAlgoContext*  ctx,
    dpc_api_attrib_t *attr
);

XCAM_BEGIN_DECLARE
extern RkAiqAlgoDescription g_RkIspAlgoDescDpc;
XCAM_END_DECLARE

#endif