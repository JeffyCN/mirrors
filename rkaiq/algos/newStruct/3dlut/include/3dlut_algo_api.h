#ifndef _LUT3D_ALOG_API_H_
#define _LUT3D_ALOG_API_H_

#include "rk_aiq_algo_des.h"

#if RKAIQ_HAVE_3DLUT_V1
#include "isp/rk_aiq_isp_3dlut20.h"
#else
#error "wrong 3dlut hw version !"
#endif
#include "algos/rk_aiq_api_types_3dlut.h"

typedef struct {
    float sensorGain;
    float awbGain[2];
    float awbIIRDampCoef;
    float varianceLuma;
    bool grayMode;
    bool awbConverged;
} lut3d_sw_info_t;

XCAM_BEGIN_DECLARE
XCamReturn algo_lut3d_queryalut3dStatus(RkAiqAlgoContext* ctx, alut3d_status_t* status);
XCamReturn algo_lut3d_SetCalib(RkAiqAlgoContext* ctx, alut3d_lut3dCalib_t* calib);
XCamReturn algo_lut3d_GetCalib(RkAiqAlgoContext* ctx, alut3d_lut3dCalib_t* calib);
#if 0
XCamReturn
algo_lut3d_SetAttrib
(
    RkAiqAlgoContext* ctx,
    lut3d_api_attrib_t *attr
);

XCamReturn
algo_lut3d_GetAttrib
(
    RkAiqAlgoContext*  ctx,
    lut3d_api_attrib_t *attr
);
#endif
extern RkAiqAlgoDescription g_RkIspAlgoDescLut3d;
XCAM_END_DECLARE

#endif
