/*
 * Copyright (c) 2019-2022 Rockchip Eletronics Co., Ltd.
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
 */
#include "uAPI2/rk_aiq_user_api2_atmo.h"

#include "algo_handlers/RkAiqAtmoHandle.h"

RKAIQ_BEGIN_DECLARE

#ifdef RK_SIMULATOR_HW
#define CHECK_USER_API_ENABLE
#endif

#if RKAIQ_HAVE_TMO_V1
XCamReturn
rk_aiq_user_api2_atmo_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, atmo_attrib_t attr)
{
    if(CHECK_ISP_HW_V20()) {
        CHECK_USER_API_ENABLE2(sys_ctx);
        CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_ATMO);
        RkAiqAtmoHandleInt* algo_handle =
            algoHandle<RkAiqAtmoHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_ATMO);

        if (algo_handle) {
            return algo_handle->setAttrib(attr);
        }
    }
    else if(CHECK_ISP_HW_V21())
        LOGE_ATMO("%s:ISP2.1 do not support atmo api!\n", __FUNCTION__);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api2_atmo_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, atmo_attrib_t* attr)
{
    if(CHECK_ISP_HW_V20()) {
        RkAiqAtmoHandleInt* algo_handle =
            algoHandle<RkAiqAtmoHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_ATMO);

        if (algo_handle) {
            return algo_handle->getAttrib(attr);
        }
    }
    else if(CHECK_ISP_HW_V21())
        LOGE_ATMO("%s:ISP2.1 do not support atmo api!\n", __FUNCTION__);

    return XCAM_RETURN_NO_ERROR;
}
#else
XCamReturn
rk_aiq_user_api2_atmo_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, atmo_attrib_t attr) {
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn
rk_aiq_user_api2_atmo_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, atmo_attrib_t* attr) {
    return XCAM_RETURN_ERROR_UNKNOWN;
}
#endif


RKAIQ_END_DECLARE
