#ifndef _HSV_ALOG_API_H_
#define _HSV_ALOG_API_H_

#include "rk_aiq_algo_des.h"

#if RKAIQ_HAVE_HSV_V10
#include "isp/rk_aiq_isp_hsv10.h"
#elif RKAIQ_HAVE_HSV_V11
#include "isp/rk_aiq_isp_hsv11.h"
#else
#error "wrong hsv hw version !"
#endif
#include "algos/rk_aiq_api_types_hsv.h"

XCAM_BEGIN_DECLARE
XCamReturn algo_hsv_queryahsvStatus(RkAiqAlgoContext* ctx, ahsv_status_t* status);
XCamReturn
algo_hsv_SetCalib
(
    RkAiqAlgoContext* ctx,
    ahsv_hsvCalib_t* calib
);
XCamReturn
algo_hsv_GetCalib
(
    RkAiqAlgoContext* ctx,
    ahsv_hsvCalib_t* calib
);

XCamReturn
algo_hsv_SetSatStrth
(
    RkAiqAlgoContext* ctx,
    ahsv_satStrg_t* strg
);

XCamReturn
algo_hsv_GetSatStrth
(
    RkAiqAlgoContext* ctx,
    ahsv_satStrg_t* strg
);

XCamReturn
algo_hsv_SetHueOffset
(
    RkAiqAlgoContext* ctx,
    ahsv_hueOffset_t* offset
);

XCamReturn
algo_hsv_getHueOffset
(
    RkAiqAlgoContext* ctx,
    ahsv_hueOffset_t* offset
);

XCamReturn
algo_hsv_SetValOffset
(
    RkAiqAlgoContext* ctx,
    ahsv_valOffset_t* offset
);

XCamReturn
algo_hsv_getValOffset
(
    RkAiqAlgoContext* ctx,
    ahsv_valOffset_t* offset
);

extern RkAiqAlgoDescription g_RkIspAlgoDescHsv;
XCAM_END_DECLARE

#endif
