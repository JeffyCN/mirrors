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
#include "rk_aiq_user_api2_adebayer.h"

#include "algo_handlers/RkAiqAdebayerHandle.h"
#include "uAPI/include/rk_aiq_user_api_adebayer.h"

RKAIQ_BEGIN_DECLARE

#ifdef RK_SIMULATOR_HW
#define CHECK_USER_API_ENABLE
#endif

XCamReturn
rk_aiq_user_api2_adebayer_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, adebayer_attrib_t attr)
{
    return rk_aiq_user_api_adebayer_SetAttrib(sys_ctx, attr);
}

XCamReturn
rk_aiq_user_api2_adebayer_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, adebayer_attrib_t *attr)
{
    return rk_aiq_user_api_adebayer_GetAttrib(sys_ctx, attr);
}


XCamReturn
rk_aiq_user_api2_adebayer_v2_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, adebayer_v2_attrib_t attr)
{
    return rk_aiq_user_api_adebayer_v2_SetAttrib(sys_ctx, attr);
}

XCamReturn
rk_aiq_user_api2_adebayer_v2_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, adebayer_v2_attrib_t *attr)
{
    return rk_aiq_user_api_adebayer_v2_GetAttrib(sys_ctx, attr);
}
XCamReturn
rk_aiq_user_api2_adebayer_v2_lite_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, adebayer_v2lite_attrib_t attr)
{
    return rk_aiq_user_api_adebayer_v2_lite_SetAttrib(sys_ctx, attr);
}

XCamReturn
rk_aiq_user_api2_adebayer_v2_lite_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, adebayer_v2lite_attrib_t *attr)
{
    return rk_aiq_user_api_adebayer_v2_lite_GetAttrib(sys_ctx, attr);
}
XCamReturn
rk_aiq_user_api2_adebayer_v3_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, adebayer_v3_attrib_t attr)
{
    return rk_aiq_user_api_adebayer_v3_SetAttrib(sys_ctx, attr);
}

XCamReturn
rk_aiq_user_api2_adebayer_v3_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, adebayer_v3_attrib_t *attr)
{
    return rk_aiq_user_api_adebayer_v3_GetAttrib(sys_ctx, attr);
}


RKAIQ_END_DECLARE
