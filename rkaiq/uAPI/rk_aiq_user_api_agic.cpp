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
#include "rk_aiq_user_api_agic.h"

#include "algo_handlers/RkAiqAgicHandle.h"

#ifdef RK_SIMULATOR_HW
#define CHECK_USER_API_ENABLE
#endif

#if RKAIQ_HAVE_GIC_V1
XCamReturn rk_aiq_user_api_agic_v1_SetAttrib(rk_aiq_sys_ctx_t* sys_ctx,
                                             const rkaiq_gic_v1_api_attr_t* attr) {
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AGIC);
    RKAIQ_API_SMART_LOCK(sys_ctx);
    RkAiqAgicHandleInt* algo_handle =
        algoHandle<RkAiqAgicHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AGIC);

    if (algo_handle) {
        return algo_handle->setAttribV1(attr);
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn rk_aiq_user_api_agic_v1_GetAttrib(rk_aiq_sys_ctx_t* sys_ctx,
                                             rkaiq_gic_v1_api_attr_t* attr) {
    RKAIQ_API_SMART_LOCK(sys_ctx);
    RkAiqAgicHandleInt* algo_handle =
        algoHandle<RkAiqAgicHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AGIC);

    if (algo_handle) {
        return algo_handle->getAttribV1(attr);
    }

    return XCAM_RETURN_NO_ERROR;
}
#else

XCamReturn rk_aiq_user_api_agic_v1_SetAttrib(rk_aiq_sys_ctx_t* sys_ctx,
                                             rkaiq_gic_v1_api_attr_t attr) {
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn rk_aiq_user_api_agic_v1_GetAttrib(rk_aiq_sys_ctx_t* sys_ctx,
                                             rkaiq_gic_v1_api_attr_t* attr) {
    return XCAM_RETURN_ERROR_UNKNOWN;
}

#endif

#if RKAIQ_HAVE_GIC_V2
XCamReturn rk_aiq_user_api_agic_v2_SetAttrib(rk_aiq_sys_ctx_t* sys_ctx,
                                             const rkaiq_gic_v2_api_attr_t* attr) {
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AGIC);
    RKAIQ_API_SMART_LOCK(sys_ctx);
    RkAiqAgicHandleInt* algo_handle =
        algoHandle<RkAiqAgicHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AGIC);

    if (algo_handle) {
        return algo_handle->setAttribV2(attr);
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn rk_aiq_user_api_agic_v2_GetAttrib(rk_aiq_sys_ctx_t* sys_ctx,
                                             rkaiq_gic_v2_api_attr_t* attr) {
    RKAIQ_API_SMART_LOCK(sys_ctx);
    RkAiqAgicHandleInt* algo_handle =
        algoHandle<RkAiqAgicHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AGIC);

    if (algo_handle) {
        return algo_handle->getAttribV2(attr);
    }

    return XCAM_RETURN_NO_ERROR;
}
#else

XCamReturn rk_aiq_user_api_agic_v2_SetAttrib(rk_aiq_sys_ctx_t* sys_ctx,
                                             rkaiq_gic_v2_api_attr_t attr) {
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn rk_aiq_user_api_agic_v2_GetAttrib(rk_aiq_sys_ctx_t* sys_ctx,
                                             rkaiq_gic_v2_api_attr_t* attr) {
    return XCAM_RETURN_ERROR_UNKNOWN;
}

#endif

