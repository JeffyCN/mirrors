#ifndef __RKAIQ_TYPES_ALGO_AIE_PRVT_H__
#define __RKAIQ_TYPES_ALGO_AIE_PRVT_H__

#include "base/xcam_common.h"
#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbTypesV2.h"
#include "rk_aiq_types_aie_algo_int.h"
#include "xcam_log.h"

typedef struct _RkAiqAlgoContext {
    int skip_frame;
    CamCalibDbContext_t* calib;
    CamCalibDbV2Context_t *calibv2;
    rk_aiq_aie_params_t params;
    rk_aiq_aie_last_params_t last_params;
    rk_aiq_aie_params_int_t sharp_params;
    rk_aiq_aie_params_int_t emboss_params;
    rk_aiq_aie_params_int_t sketch_params;
    bool isReCal_;
} RkAiqAlgoContext;

#endif//__RKAIQ_TYPES_ALGO_AIE_PRVT_H__
