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

#if RKAIQ_HAVE_3DLUT_V1
#include "isp/rk_aiq_isp_3dlut20.h"
#else
#error "wrong lut3d hw version !"
#endif
#include "uAPI2/rk_aiq_user_api2_3dlut.h"
#include "algo_handlers/newStruct/RkAiq3dlutHandler.h"
#include "RkAiqGlobalParamsManager.h"

RKAIQ_BEGIN_DECLARE

#ifdef RK_SIMULATOR_HW
#define CHECK_USER_API_ENABLE
#endif

#ifndef USE_IMPLEMENT_C
static XCamReturn
_lut3d_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, lut3d_api_attrib_t* attr)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (attr->opMode == RK_AIQ_OP_MODE_MANUAL || attr->opMode == RK_AIQ_OP_MODE_AUTO) {
        rk_aiq_global_params_wrap_t params;
        params.opMode = attr->opMode;
        params.bypass = attr->bypass;
        params.en = attr->en;
        params.type = RESULT_TYPE_LUT3D_PARAM;
        params.man_param_size = sizeof(lut3d_param_t);
        params.man_param_ptr = &attr->stMan;
        params.aut_param_size = sizeof(lut3d_param_auto_t);
        params.aut_param_ptr = &attr->stAuto;
        ret = sys_ctx->_rkAiqManager->getGlobalParamsManager()->set(&params);
    } else {
        LOGE_A3DLUT("wrong mode %d !", attr->opMode);
    }

    return ret;
} 

XCamReturn
rk_aiq_user_api2_3dlut_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, lut3d_api_attrib_t* attr)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if USE_NEWSTRUCT
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_A3DLUT);
    RKAIQ_API_SMART_LOCK(sys_ctx);
    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
        for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
            if (!camCtx)
                continue;
            ret = _lut3d_SetAttrib(camCtx, attr);
        }
        return ret;
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        return _lut3d_SetAttrib(sys_ctx, attr);
    }
#else
    return XCAM_RETURN_ERROR_UNKNOWN;
#endif
}

static XCamReturn
_lut3d_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, lut3d_api_attrib_t* attr)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiq3dlutHandleInt* algo_handle =
        algoHandle<RkAiq3dlutHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_A3DLUT);

    if (algo_handle) {
        algo_handle->getAttrib(attr);
    }

    // get cur manual params
    rk_aiq_global_params_wrap_t params;
    params.type = RESULT_TYPE_LUT3D_PARAM;
    params.man_param_size = sizeof(lut3d_param_t);
    params.man_param_ptr = &attr->stMan;
    ret = sys_ctx->_rkAiqManager->getGlobalParamsManager()->get(&params);
    if (ret == XCAM_RETURN_NO_ERROR) {
        attr->en = params.en;
        attr->bypass = params.bypass;
        attr->opMode = (RKAiqOPMode_t)params.opMode;
    }

    return ret;
}

XCamReturn
rk_aiq_user_api2_3dlut_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, lut3d_api_attrib_t* attr)
{
#if USE_NEWSTRUCT
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_A3DLUT);
    RKAIQ_API_SMART_LOCK(sys_ctx);
    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
        for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
            if (!camCtx)
                continue;
            // return the first cam's attrib
            return _lut3d_GetAttrib(camCtx , attr);
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        return _lut3d_GetAttrib(sys_ctx, attr);
    }
#else
    return XCAM_RETURN_ERROR_UNKNOWN;
#endif
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
_lut3d_QueryStatus(const rk_aiq_sys_ctx_t* sys_ctx, lut3d_status_t* status)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (sys_ctx->_rkAiqManager->getGlobalParamsManager()->isManual(RESULT_TYPE_LUT3D_PARAM)) {
        // get cur manual params
        rk_aiq_global_params_wrap_t params;
        params.type = RESULT_TYPE_LUT3D_PARAM;
        params.man_param_size = sizeof(lut3d_param_t);
        params.man_param_ptr = &status->stMan;
        ret = sys_ctx->_rkAiqManager->getGlobalParamsManager()->get(&params);
        if (ret == XCAM_RETURN_NO_ERROR) {
            status->en = params.en;
            status->bypass = params.bypass;
            status->opMode = (RKAiqOPMode_t)params.opMode;
        }
    } else {
        RkAiq3dlutHandleInt* algo_handle =
            algoHandle<RkAiq3dlutHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_A3DLUT);

        if (algo_handle) {
            return algo_handle->queryStatus(status);
        }
    }

    return ret;
}

XCamReturn
rk_aiq_user_api2_3dlut_QueryStatus(const rk_aiq_sys_ctx_t* sys_ctx, lut3d_status_t* status)
{
#if USE_NEWSTRUCT
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_A3DLUT);
    RKAIQ_API_SMART_LOCK(sys_ctx);
    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
        for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
            if (!camCtx)
                continue;
            // return the first cam's attrib
            return _lut3d_QueryStatus(camCtx , status);
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        return _lut3d_QueryStatus(sys_ctx, status);
    }
#else
    return XCAM_RETURN_ERROR_UNKNOWN;
#endif
   return XCAM_RETURN_NO_ERROR;
}
#endif

RKAIQ_END_DECLARE
