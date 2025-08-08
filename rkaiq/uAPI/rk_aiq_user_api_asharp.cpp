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
#include "rk_aiq_user_api_asharp.h"

#include "algo_handlers/RkAiqAsharpHandle.h"

RKAIQ_BEGIN_DECLARE

#ifdef RK_SIMULATOR_HW
#define CHECK_USER_API_ENABLE
#endif

#if RKAIQ_HAVE_SHARP_V1

XCamReturn
rk_aiq_user_api_asharp_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_sharp_attrib_t *attr)
{
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_ASHARP);
    RKAIQ_API_SMART_LOCK(sys_ctx);
    RkAiqAsharpHandleInt* algo_handle =
        algoHandle<RkAiqAsharpHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_ASHARP);

    if (algo_handle) {
        return algo_handle->setAttrib(attr);
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api_asharp_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_sharp_attrib_t *attr)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    RkAiqAsharpHandleInt* algo_handle =
        algoHandle<RkAiqAsharpHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_ASHARP);

    if (algo_handle) {
        return algo_handle->getAttrib(attr);
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api_asharp_SetIQPara(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_sharp_IQpara_t *para)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    RkAiqAsharpHandleInt* algo_handle =
        algoHandle<RkAiqAsharpHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_ASHARP);

    if (algo_handle) {
        return algo_handle->setIQPara(para);
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api_asharp_GetIQPara(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_sharp_IQpara_t *para)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    RkAiqAsharpHandleInt* algo_handle =
        algoHandle<RkAiqAsharpHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_ASHARP);

    if (algo_handle) {
        return algo_handle->getIQPara(para);
    }

    return XCAM_RETURN_NO_ERROR;
}


XCamReturn
rk_aiq_user_api_asharp_SetStrength(const rk_aiq_sys_ctx_t* sys_ctx, float fPercent)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    RkAiqAsharpHandleInt* algo_handle =
        algoHandle<RkAiqAsharpHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_ASHARP);

    if (algo_handle) {
        return algo_handle->setStrength(fPercent);
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api_asharp_GetStrength(const rk_aiq_sys_ctx_t* sys_ctx, float *pPercent)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    RkAiqAsharpHandleInt* algo_handle =
        algoHandle<RkAiqAsharpHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_ASHARP);

    if (algo_handle) {
        return algo_handle->getStrength(pPercent);
    }

    return XCAM_RETURN_NO_ERROR;
}
#else
XCamReturn
rk_aiq_user_api_asharp_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_sharp_attrib_t *attr)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn
rk_aiq_user_api_asharp_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_sharp_attrib_t *attr)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn
rk_aiq_user_api_asharp_SetIQPara(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_sharp_IQpara_t *para)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn
rk_aiq_user_api_asharp_GetIQPara(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_sharp_IQpara_t *para)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn
rk_aiq_user_api_asharp_SetStrength(const rk_aiq_sys_ctx_t* sys_ctx, float fPercent)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn
rk_aiq_user_api_asharp_GetStrength(const rk_aiq_sys_ctx_t* sys_ctx, float *pPercent)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}


#endif
RKAIQ_END_DECLARE
