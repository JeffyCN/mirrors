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

#include "isp/rk_aiq_isp_asd.h"

#include "uAPI2/rk_aiq_user_api2_asd.h"
#include "aiq_core_c/algo_handlers/RkAiqAsdHandler.h"
#include "uAPI2_c/rk_aiq_user_api2_common.h"

RKAIQ_BEGIN_DECLARE

#ifdef RK_SIMULATOR_HW
#define CHECK_USER_API_ENABLE
#endif

#if RKAIQ_HAVE_ASD

XCamReturn
rk_aiq_user_api2_asd_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, asd_api_attrib_t* attr)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_ASD);
    RKAIQ_API_SMART_LOCK(sys_ctx);
    rk_aiq_sys_ctx_array_t ctx_array = rk_aiq_user_api2_common_getSysCtxArray(sys_ctx);

    int type = RESULT_TYPE_ASD_PARAM;
    int man_param_size = sizeof(asd_param_t);
    int aut_param_size = sizeof(asd_param_auto_t);

    for (int i = 0; i < ctx_array.num; i++) {
        if (attr->opMode == RK_AIQ_OP_MODE_MANUAL || attr->opMode == RK_AIQ_OP_MODE_AUTO) {
            ret = rk_aiq_user_api2_common_processParams(ctx_array.ctx[i], true,
                    &attr->opMode, &attr->en, &attr->bypass,
                    type, man_param_size, &attr->stMan, aut_param_size,  &attr->stAuto);
        } else {
            ret = XCAM_RETURN_ERROR_FAILED;
            LOGE_ASD("wrong mode %d !", attr->opMode);
        }
    }

    return ret;
}

XCamReturn
rk_aiq_user_api2_asd_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, asd_api_attrib_t* attr)
{
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_ASD);
    RKAIQ_API_SMART_LOCK(sys_ctx);

    const rk_aiq_sys_ctx_t* ctx = rk_aiq_user_api2_common_getSysCtx(sys_ctx);
    int type = RESULT_TYPE_ASD_PARAM;
    int man_param_size = sizeof(asd_param_t);
    int aut_param_size = sizeof(asd_param_auto_t);

    return rk_aiq_user_api2_common_processParams(ctx, false,
                &attr->opMode, &attr->en, &attr->bypass,
                type, man_param_size, &attr->stMan, aut_param_size,  &attr->stAuto);
}

XCamReturn
rk_aiq_user_api2_asd_QueryStatus(const rk_aiq_sys_ctx_t* sys_ctx, asd_status_t* status)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_ASD);
    RKAIQ_API_SMART_LOCK(sys_ctx);

    asd_api_attrib_t attr;
    rk_aiq_user_api2_asd_GetAttrib(sys_ctx, &attr);
    status->opMode = attr.opMode;
    status->en     = attr.en;
    status->bypass = attr.bypass;
    if (attr.opMode == RK_AIQ_OP_MODE_AUTO) {
        status->stMan.sta = attr.stAuto.sta;
    } else {
        status->stMan = attr.stMan;
    }

    return ret;
}

XCamReturn
rk_aiq_user_api2_asd_setFaceInfo(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_face_info_t* face)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_ASD);
    RKAIQ_API_SMART_LOCK(sys_ctx);

    const rk_aiq_sys_ctx_t* ctx = rk_aiq_user_api2_common_getSysCtx(sys_ctx);

    AiqAlgoHandler_t* algo_handle =
        ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_ASD];

    if (algo_handle) {
        ret = AiqAlgoHandlerAsd_setFaceInfo((AiqAlgoHandlerAsd_t*)algo_handle, face);
    }

    return ret;
}

XCamReturn
rk_aiq_user_api2_asd_getSelFaceInfo(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_face_roi_t* selface)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_ASD);
    RKAIQ_API_SMART_LOCK(sys_ctx);

    const rk_aiq_sys_ctx_t* ctx = rk_aiq_user_api2_common_getSysCtx(sys_ctx);

    AiqAlgoHandler_t* algo_handle =
        ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_ASD];

    if (algo_handle) {
        ret = AiqAlgoHandlerAsd_getSelFaceInfo((AiqAlgoHandlerAsd_t*)algo_handle, selface);
    }

    return ret;
}
#endif

RKAIQ_END_DECLARE
