#ifndef _ASD_ALOG_API_H_
#define _ASD_ALOG_API_H_

#include "rk_aiq_algo_des.h"

#include "isp/rk_aiq_isp_asd.h"
#include "algos/rk_aiq_api_types_asd.h"

XCAM_BEGIN_DECLARE

extern RkAiqAlgoDescription g_RkIspAlgoDescAsd;
XCamReturn algo_asd_setFaceInfo(RkAiqAlgoContext *ctx, rk_aiq_face_info_t *face);
XCAM_END_DECLARE

#endif