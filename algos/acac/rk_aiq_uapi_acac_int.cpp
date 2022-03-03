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
#include "acac/rk_aiq_uapi_acac_int.h"

#include "acac/rk_aiq_types_acac_algo_prvt.h"
#include "xcam_log.h"

XCamReturn rk_aiq_uapi_acac_SetAttrib(RkAiqAlgoContext* ctx, rk_aiq_cac_attrib_t attr,
                                      bool need_sync) {
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn rk_aiq_uapi_acac_GetAttrib(const RkAiqAlgoContext* ctx, rk_aiq_cac_attrib_t* attr) {
    return XCAM_RETURN_NO_ERROR;
}
