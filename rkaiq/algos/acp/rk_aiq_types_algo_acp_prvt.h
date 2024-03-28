#ifndef __RKAIQ_TYPES_ALGO_ACP_PRVT_H__
#define __RKAIQ_TYPES_ALGO_ACP_PRVT_H__

#include "base/xcam_common.h"
#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbTypesV2.h"
#include "xcam_log.h"
#include "rk_aiq_types_acp_algo.h"

typedef struct AcpContext_s {
    CamCalibDbContext_t* calib;
    CamCalibDbV2Context_t *calibv2;
    rk_aiq_acp_params_t params;
    bool isReCal_;
} AcpContext_t;

typedef struct _RkAiqAlgoContext {
    AcpContext_t acpCtx;
} RkAiqAlgoContext;

#endif//__RKAIQ_TYPES_ALGO_ACP_PRVT_H__
