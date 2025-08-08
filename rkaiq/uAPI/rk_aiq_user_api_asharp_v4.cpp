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
#include "rk_aiq_user_api_asharp_v4.h"

#include "algo_handlers/RkAiqAsharpV4Handle.h"

RKAIQ_BEGIN_DECLARE

#ifdef RK_SIMULATOR_HW
#define CHECK_USER_API_ENABLE
#endif

#if RKAIQ_HAVE_SHARP_V4

XCamReturn
rk_aiq_user_api_asharpV4_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_sharp_attrib_v4_t* attr)
{
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_ASHARP);
    RkAiqAsharpV4HandleInt* algo_handle =
        algoHandle<RkAiqAsharpV4HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_ASHARP);

    if (algo_handle) {
        return algo_handle->setAttrib(attr);
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api_asharpV4_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_sharp_attrib_v4_t* attr)
{
    RkAiqAsharpV4HandleInt* algo_handle =
        algoHandle<RkAiqAsharpV4HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_ASHARP);

    if (algo_handle) {
        return algo_handle->getAttrib(attr);
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api_asharpV4_SetStrength(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_sharp_strength_v4_t *pStrength)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RKAIQ_API_SMART_LOCK(sys_ctx);


    RkAiqAsharpV4HandleInt* algo_handle =
        algoHandle<RkAiqAsharpV4HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_ASHARP);
    if (algo_handle) {
        ret = algo_handle->setStrength(pStrength);
    }


    return ret;
}


XCamReturn
rk_aiq_user_api_asharpV4_GetStrength(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_sharp_strength_v4_t *pStrength)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RKAIQ_API_SMART_LOCK(sys_ctx);


    RkAiqAsharpV4HandleInt* algo_handle =
        algoHandle<RkAiqAsharpV4HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_ASHARP);
    if (algo_handle) {
        ret = algo_handle->getStrength(pStrength);
    }

    return ret;
}

XCamReturn
rk_aiq_user_api_asharpV4_GetInfo(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_sharp_info_v4_t *pInfo)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RKAIQ_API_SMART_LOCK(sys_ctx);


    RkAiqAsharpV4HandleInt* algo_handle =
        algoHandle<RkAiqAsharpV4HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_ASHARP);
    if (algo_handle) {
        ret = algo_handle->getInfo(pInfo);
    }

    return ret;
}

#else
XCamReturn
rk_aiq_user_api_asharpV4_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_sharp_attrib_v4_t* attr)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn
rk_aiq_user_api_asharpV4_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_sharp_attrib_v4_t* attr)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn
rk_aiq_user_api_asharpV4_SetStrength(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_sharp_strength_v4_t *pStrength)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn
rk_aiq_user_api_asharpV4_GetStrength(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_sharp_strength_v4_t *pStrength)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn
rk_aiq_user_api_asharpV4_GetInfo(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_sharp_info_v4_t *pInfo)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}

#endif


RKAIQ_END_DECLARE
