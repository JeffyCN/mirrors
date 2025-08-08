#ifndef _GAMMA_ALOG_API_H_
#define _GAMMA_ALOG_API_H_

#include "algos/rk_aiq_api_types_gamma.h"
#include "isp/rk_aiq_isp_gamma21.h"
#include "rk_aiq_algo_des.h"

typedef enum GammaHwVersion_e {
    Gamma_HARDWARE_V10 = 0,  // rk1126
    Gamma_HARDWARE_V11 = 1,  // rk3588
    Gamma_HARDWARE_MAX,
} GammaHwVersion_t;

XCAM_BEGIN_DECLARE

#if 0
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
#endif

//XCamReturn GammaSelectParam(agamma_param_auto_t* pAuto, gamma_param_t* out, int iso);
XCamReturn Agamma_processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams, int iso);
XCamReturn algo_gamma_SetStrength(RkAiqAlgoContext* ctx, int strg);

extern RkAiqAlgoDescription g_RkIspAlgoDescGamma;
XCAM_END_DECLARE

#endif
