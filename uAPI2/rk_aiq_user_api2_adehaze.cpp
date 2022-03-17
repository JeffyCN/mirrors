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

#include "include/uAPI2/rk_aiq_user_api2_adehaze.h"
#include "RkAiqHandleInt.h"

RKAIQ_BEGIN_DECLARE

#ifdef RK_SIMULATOR_HW
#define CHECK_USER_API_ENABLE
#endif

XCamReturn  rk_aiq_user_api2_adehaze_setSwAttrib(const rk_aiq_sys_ctx_t* sys_ctx, adehaze_sw_V2_t attr)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_ADHAZ);
    RKAIQ_API_SMART_LOCK(sys_ctx);

    RkAiqAdhazHandleInt* algo_handle =
        algoHandle<RkAiqAdhazHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_ADHAZ);

    if (algo_handle) {
        return algo_handle->setSwAttrib(attr);
    }

    return (ret);
}

XCamReturn  rk_aiq_user_api2_adehaze_getSwAttrib(const rk_aiq_sys_ctx_t* sys_ctx, adehaze_sw_V2_t *attr)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAdhazHandleInt* algo_handle =
        algoHandle<RkAiqAdhazHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_ADHAZ);

    if (algo_handle) {
        return algo_handle->getSwAttrib(attr);
    }

    return (ret);

}


RKAIQ_END_DECLARE

