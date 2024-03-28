/*
 * rk_aiq_uapi_acac_int.cpp
 *
 *  Copyright (c) 2021 Rockchip Corporation
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
#include "algos/acac/rk_aiq_uapi_acac_int.h"

#include "algos/acac/rk_aiq_types_acac_algo_prvt.h"
#include "xcore/base/xcam_common.h"

using RkCam::CacAlgoAdaptor;

#if RKAIQ_HAVE_CAC_V03
XCamReturn rk_aiq_uapi_acac_v03_SetAttrib(RkAiqAlgoContext* ctx,
                                          const rkaiq_cac_v03_api_attr_t* attr, bool need_sync) {
    if (ctx == nullptr) {
        LOGE_ACAC("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    auto* cac = static_cast<CacAlgoAdaptor*>(ctx->handle);
    cac->SetApiAttr(attr);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn rk_aiq_uapi_acac_v03_GetAttrib(const RkAiqAlgoContext* ctx,
                                          rkaiq_cac_v03_api_attr_t* attr) {
    if (ctx == nullptr) {
        LOGE_ACAC("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    auto* cac = static_cast<CacAlgoAdaptor*>(ctx->handle);
    cac->GetApiAttr(attr);

    return XCAM_RETURN_NO_ERROR;
}
#else
XCamReturn rk_aiq_uapi_acac_v03_SetAttrib(RkAiqAlgoContext* /* ctx */,
                                          const rkaiq_cac_v03_api_attr_t* /* attr */,
                                          bool /* need_sync */) {
    return XCAM_RETURN_NO_ERROR;
}
XCamReturn rk_aiq_uapi_acac_v03_GetAttrib(const RkAiqAlgoContext* /* ctx */,
                                          rkaiq_cac_v03_api_attr_t* /* attr */) {
    return XCAM_RETURN_NO_ERROR;
}
#endif

#if RKAIQ_HAVE_CAC_V10
XCamReturn rk_aiq_uapi_acac_v10_SetAttrib(RkAiqAlgoContext* ctx,
                                          const rkaiq_cac_v10_api_attr_t* attr, bool need_sync) {
    if (ctx == nullptr) {
        LOGE_ACAC("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    auto* cac = static_cast<CacAlgoAdaptor*>(ctx->handle);
    cac->SetApiAttr(attr);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn rk_aiq_uapi_acac_v10_GetAttrib(const RkAiqAlgoContext* ctx,
                                          rkaiq_cac_v10_api_attr_t* attr) {
    if (ctx == nullptr) {
        LOGE_ACAC("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    auto* cac = static_cast<CacAlgoAdaptor*>(ctx->handle);
    cac->GetApiAttr(attr);

    return XCAM_RETURN_NO_ERROR;
}
#else
XCamReturn rk_aiq_uapi_acac_v10_SetAttrib(RkAiqAlgoContext* /* ctx */,
                                          const rkaiq_cac_v10_api_attr_t* /* attr */,
                                          bool /* need_sync */) {
    return XCAM_RETURN_NO_ERROR;
}
XCamReturn rk_aiq_uapi_acac_v10_GetAttrib(const RkAiqAlgoContext* /* ctx */,
                                          rkaiq_cac_v10_api_attr_t* /* attr */) {
    return XCAM_RETURN_NO_ERROR;
}
#endif

#if RKAIQ_HAVE_CAC_V11
XCamReturn rk_aiq_uapi_acac_v11_SetAttrib(RkAiqAlgoContext* ctx,
                                          const rkaiq_cac_v11_api_attr_t* attr,
                                          bool /* need_sync */) {
    if (ctx == nullptr) {
        LOGE_ACAC("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    auto* cac = static_cast<CacAlgoAdaptor*>(ctx->handle);
    cac->SetApiAttr(attr);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn rk_aiq_uapi_acac_v11_GetAttrib(const RkAiqAlgoContext* ctx,
                                          rkaiq_cac_v11_api_attr_t* attr) {
    if (ctx == nullptr) {
        LOGE_ACAC("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    auto* cac = static_cast<CacAlgoAdaptor*>(ctx->handle);
    cac->GetApiAttr(attr);

    return XCAM_RETURN_NO_ERROR;
}
#else
XCamReturn rk_aiq_uapi_acac_v11_SetAttrib(RkAiqAlgoContext* /* ctx */,
                                          const rkaiq_cac_v11_api_attr_t* /* attr */,
                                          bool /* need_sync */) {
    return XCAM_RETURN_NO_ERROR;
}
XCamReturn rk_aiq_uapi_acac_v11_GetAttrib(const RkAiqAlgoContext* /* ctx */,
                                          rkaiq_cac_v11_api_attr_t* /* attr */) {
    return XCAM_RETURN_NO_ERROR;
}
#endif
