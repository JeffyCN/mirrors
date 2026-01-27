/*
 * rk_aiq_uapi_aldc_int.cpp
 *
 *  Copyright (c) 2023 Rockchip Corporation
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

#include "algos/aldc/rk_aiq_uapi_aldc_int.h"

#include "algos/aldc/rk_aiq_types_aldc_algo_prvt.h"
#include "xcore/base/xcam_common.h"

using RkCam::LdcAlgoAdaptor;

#define DISABLE_HANDLE_ATTRIB

XCamReturn rk_aiq_uapi_aldc_SetAttrib(RkAiqAlgoContext* ctx, const rk_aiq_ldc_attrib_t* attr,
                                      bool /* need_sync */) {
    if (ctx == nullptr) {
        LOGE_ALDC("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    auto* ldc = static_cast<LdcAlgoAdaptor*>(ctx->handle);
    ldc->SetApiAttr(attr);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn rk_aiq_uapi_aldc_GetAttrib(const RkAiqAlgoContext* ctx, rk_aiq_ldc_attrib_t* attr) {
    if (ctx == nullptr) {
        LOGE_ACAC("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    auto* ldc = static_cast<LdcAlgoAdaptor*>(ctx->handle);
    ldc->GetApiAttr(attr);

    return XCAM_RETURN_NO_ERROR;
}
