#ifndef _DEMOSAIC_ALOG_API_H_
#define _DEMOSAIC_ALOG_API_H_

#include "rk_aiq_algo_des.h"

#if RKAIQ_HAVE_DEBAYER_V2
#include "isp/rk_aiq_isp_dm21.h"
#elif RKAIQ_HAVE_DEBAYER_V3
#include "isp/rk_aiq_isp_dm23.h"
#else
#error "wrong demosaic hw version !"
#endif
#include "algos/rk_aiq_api_types_dm.h"

typedef enum DmHwVersion_e
{
    DM_HARDWARE_V1 = 0,  // rk1126/1109/356X/3588
    DM_HARDWARE_V2 = 1,  //rk1106
    DM_HARDWARE_V2_LITE = 2,  //rk3562
    DM_HARDWARE_MAX,
} DmHwVersion_t;

XCamReturn
algo_demosaic_SetAttrib
(
    RkAiqAlgoContext* ctx,
    dm_api_attrib_t *attr
);

XCamReturn
algo_demosaic_GetAttrib
(
    RkAiqAlgoContext*  ctx,
    dm_api_attrib_t *attr
);

XCAM_BEGIN_DECLARE
extern RkAiqAlgoDescription g_RkIspAlgoDescDemosaic;
XCAM_END_DECLARE

#endif
