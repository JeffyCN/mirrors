/*
 *  Copyright (c) 2019 Rockchip Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#include "uAPI2/rk_aiq_user_api2_aldc.h"

#include "Isp20_module_dbg.h"
#include "aiq_core/algo_handlers/RkAiqAldcHandle.h"
#include "common/rk_aiq.h"
#include "uAPI/rk_aiq_api_private.h"
#include "xcore/base/xcam_common.h"

RKAIQ_BEGIN_DECLARE

#ifdef RK_SIMULATOR_HW
#define CHECK_USER_API_ENABLE
#endif

#if RKAIQ_HAVE_LDC
XCamReturn rk_aiq_user_api2_aldc_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx,
                                           const rk_aiq_ldc_attrib_t* attr) {
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_ALDC);
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
        return XCAM_RETURN_ERROR_FAILED;
    } else {
        RkAiqAldcHandleInt* algo_handle =
            algoHandle<RkAiqAldcHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_ALDC);

        if (algo_handle) {
            return algo_handle->setAttrib(attr);
        }
    }

    return XCAM_RETURN_ERROR_FAILED;
}

XCamReturn rk_aiq_user_api2_aldc_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx,
                                           rk_aiq_ldc_attrib_t* attr) {
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
        return XCAM_RETURN_ERROR_FAILED;
    } else {
        RkAiqAldcHandleInt* algo_handle =
            algoHandle<RkAiqAldcHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_ALDC);

        if (algo_handle) {
            return algo_handle->getAttrib(attr);
        }
    }

    return XCAM_RETURN_ERROR_FAILED;
}
#else
XCamReturn rk_aiq_user_api2_aldc_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx,
                                           const rk_aiq_ldc_attrib_t* attr) {

    return XCAM_RETURN_ERROR_FAILED;
}

XCamReturn rk_aiq_user_api2_aldc_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx,
                                           rk_aiq_ldc_attrib_t* attr) {
    return XCAM_RETURN_ERROR_FAILED;
}
#endif

RKAIQ_END_DECLARE
