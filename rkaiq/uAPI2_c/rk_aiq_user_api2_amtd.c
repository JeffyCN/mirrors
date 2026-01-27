/******************************************************************************
 *
 * Copyright 2019, Fuzhou Rockchip Electronics Co.Ltd . All rights reserved.
 * No part of this work may be reproduced, modified, distributed, transmitted,
 * transcribed, or translated into any language or computer format, in any form
 * or by any means without written permission of:
 * Fuzhou Rockchip Electronics Co.Ltd .
 *
 *
 *****************************************************************************/
#include "uAPI2/rk_aiq_user_api2_amtd.h"
//#include "RkAiqCamGroupHandleInt.h"
#include "aiq_core_c/algo_handlers/RkAiqAmtdHandler.h"
#include "base/xcam_common.h"
#include "uAPI2_c/rk_aiq_user_api2_common.h"
#include "uAPI2/rk_aiq_user_api2_sysctl.h"

RKAIQ_BEGIN_DECLARE

XCamReturn
rk_aiq_user_api2_amtd_setImuData(const rk_aiq_sys_ctx_t* sys_ctx, AiqImuData_t* data)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi2_setImuData(sys_ctx, data);

    AiqAmtdHandler_t* algo_handle =
        (AiqAmtdHandler_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AMTD];

    if (algo_handle) {
        return AiqAlgoHandlerAmtd_setImuData(algo_handle, data);
    }

    return ret;
}

RKAIQ_END_DECLARE

