#ifndef _DRC_ALOG_API_H_
#define _DRC_ALOG_API_H_

#include "rk_aiq_algo_des.h"

#if RKAIQ_HAVE_DRC_V12
#include "isp/rk_aiq_isp_drc32.h"
#elif RKAIQ_HAVE_DRC_V20
#include "isp/rk_aiq_isp_drc40.h"
#elif RKAIQ_HAVE_DRC_V21
#include "isp/rk_aiq_isp_drc41.h"
#else
#error "wrong drc hw version !"
#endif
#include "algos/rk_aiq_api_types_drc.h"
#include "isp/rk_aiq_isp_trans10.h"
#include "algos/rk_aiq_api_types_trans.h"

XCAM_BEGIN_DECLARE

typedef enum FrameNumber_e {
    LINEAR_NUM = 1,
    HDR_2X_NUM = 2,
    HDR_3X_NUM = 3,
    HDR_NUM_MAX
} FrameNumber_t;

typedef struct DrcAEData_s {
    bool LongFrmMode;
    float L2M_Ratio;
    float M2S_Ratio;
    float L2S_Ratio;
    float LExpo;  // invaild in Curr
    float MExpo;  // invaild in Curr
    float SExpo;  // invaild in Curr
} DrcAEData_t;

typedef struct NextData_s {
    bool bDrcEn;
    float MotionCoef;
    DrcAEData_t AEData;
} NextData_t;

#if 0
XCamReturn
algo_drc_SetAttrib
(
    RkAiqAlgoContext* ctx,
    drc_api_attrib_t *attr
);

XCamReturn
algo_drc_GetAttrib
(
    RkAiqAlgoContext*  ctx,
    drc_api_attrib_t *attr
);
#endif

XCamReturn algo_drc_SetStrength(RkAiqAlgoContext* ctx, adrc_strength_t* ctrl);

XCamReturn algo_drc_GetStrength(RkAiqAlgoContext* ctx, adrc_strength_t* ctrl);

extern RkAiqAlgoDescription g_RkIspAlgoDescDrc;
XCAM_END_DECLARE

#endif
