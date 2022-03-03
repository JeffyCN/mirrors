#ifndef __RKAIQ_TYPES_ALGO_ACSM_PRVT_H__
#define __RKAIQ_TYPES_ALGO_ACSM_PRVT_H__

#include "base/xcam_common.h"
#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"
#include "xcam_log.h"
#include "rk_aiq_types_acsm_algo.h"

typedef struct AcsmContext_s {
    CamCalibDbV2Context_t *calibv2;
    rk_aiq_acsm_params_t params;
} AcsmContext_t;

typedef struct _RkAiqAlgoContext {
    AcsmContext_t acsmCtx;
} RkAiqAlgoContext;

#endif//__RKAIQ_TYPES_ALGO_ACSM_PRVT_H__
