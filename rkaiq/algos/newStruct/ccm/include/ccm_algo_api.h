#ifndef _CCM_ALOG_API_H_
#define _CCM_ALOG_API_H_

#include "rk_aiq_algo_des.h"

#if RKAIQ_HAVE_CCM_V3
#include "isp/rk_aiq_isp_ccm22.h"
#else
#error "wrong ccm hw version !"
#endif
#include "algos/rk_aiq_api_types_ccm.h"

typedef struct {
    float sensorGain;
    float awbGain[2];
    float awbIIRDampCoef;
    float varianceLuma;
    bool grayMode;
    bool awbConverged;
} ccm_sw_info_t;

XCAM_BEGIN_DECLARE
XCamReturn algo_ccm_queryaccmStatus(RkAiqAlgoContext* ctx, accm_status_t* status);
XCamReturn algo_ccm_SetCalib(RkAiqAlgoContext* ctx, accm_ccmCalib_t* calib);
XCamReturn algo_ccm_GetCalib(RkAiqAlgoContext* ctx, accm_ccmCalib_t* calib);
#if 0
XCamReturn
algo_ccm_SetAttrib
(
    RkAiqAlgoContext* ctx,
    ccm_api_attrib_t *attr
);

XCamReturn
algo_ccm_GetAttrib
(
    RkAiqAlgoContext*  ctx,
    ccm_api_attrib_t *attr
);
#endif
extern RkAiqAlgoDescription g_RkIspAlgoDescCcm;
XCAM_END_DECLARE

#endif
