#ifndef _GAMMA_ALOG_API_H_
#define _GAMMA_ALOG_API_H_

#include "rk_aiq_algo_des.h"

#include "isp/rk_aiq_isp_gamma21.h"
#include "algos/rk_aiq_api_types_gamma.h"

typedef enum GammaHwVersion_e
{
    Gamma_HARDWARE_V10 = 0,  // rk1126
    Gamma_HARDWARE_V11 = 1,  //rk3588
    Gamma_HARDWARE_MAX,
} GammaHwVersion_t;

XCamReturn
algo_gamma_SetAttrib
(
    RkAiqAlgoContext* ctx,
    gamma_api_attrib_t *attr
);

XCamReturn
algo_gamma_GetAttrib
(
    RkAiqAlgoContext*  ctx,
    gamma_api_attrib_t *attr
);

XCamReturn GammaSelectParam(gamma_param_auto_t *pAuto, gamma_param_t* out);

XCAM_BEGIN_DECLARE
extern RkAiqAlgoDescription g_RkIspAlgoDescGamma;
XCAM_END_DECLARE

#endif
