#ifndef _GAIN_ALGO_API_H_
#define _GAIN_ALGO_API_H_

#include "rk_aiq_algo_des.h"
#if RKAIQ_HAVE_GAIN_V2
#include "isp/rk_aiq_isp_gain20.h"
#else
#error "wrong gain hw version !"
#endif
#include "algos/rk_aiq_api_types_gain.h"

RKAIQ_BEGIN_DECLARE

#if 0
XCamReturn
algo_gain_SetAttrib
(
    RkAiqAlgoContext *ctx,
    const gain_api_attrib_t *attr,
    bool need_sync
);

XCamReturn
algo_gain_GetAttrib(
    const RkAiqAlgoContext *ctx,
    gain_api_attrib_t *attr
);
#endif

//XCamReturn GainSelectParam(gain_param_auto_t *pAuto, gain_param_t* out, int iso);
XCamReturn Again_processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams, int iso);


extern RkAiqAlgoDescription g_RkIspAlgoDescGain;
RKAIQ_END_DECLARE

#endif
