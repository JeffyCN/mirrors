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

#include "isp/rk_aiq_isp_gamma21.h"
#include "uAPI2/rk_aiq_user_api2_gamma.h"
#include "algo_handlers/newStruct/RkAiqGammaHandler.h"
#include "RkAiqGlobalParamsManager.h"

RKAIQ_BEGIN_DECLARE

#ifdef RK_SIMULATOR_HW
#define CHECK_USER_API_ENABLE
#endif

static XCamReturn
_gamma_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, gamma_api_attrib_t* attr)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    /*
    if (!rk_aiq_gamma_check_attrib(attr)) {
        LOGE_ANR("gamma wrong attrib!");
        return ret;
    }
    */

    rk_aiq_global_params_wrap_t params;
    params.opMode = attr->opMode;
    params.bypass = attr->bypass;
    params.en = attr->en;
    params.type = RESULT_TYPE_AGAMMA_PARAM;
    params.man_param_size = sizeof(gamma_param_t);
    params.man_param_ptr = &attr->stMan;
    params.aut_param_size = sizeof(gamma_param_auto_t);
    params.aut_param_ptr = &attr->stAuto;
    ret = sys_ctx->_rkAiqManager->getGlobalParamsManager()->set(&params);

    return ret;
}

XCamReturn
rk_aiq_user_api2_gamma_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, gamma_api_attrib_t* attr)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if USE_NEWSTRUCT
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AGAMMA);
    RKAIQ_API_SMART_LOCK(sys_ctx);
    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
        for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
            if (!camCtx)
                continue;
            ret = _gamma_SetAttrib(camCtx, attr);
        }
        return ret;
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        return _gamma_SetAttrib(sys_ctx, attr);
    }
#else
    return XCAM_RETURN_ERROR_UNKNOWN;
#endif
}

static XCamReturn
_gamma_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, gamma_api_attrib_t* attr)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqGammaHandleInt* algo_handle =
        algoHandle<RkAiqGammaHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AGAMMA);

    if (algo_handle) {
        algo_handle->getAttrib(attr);
    }

    // get cur manual params
    rk_aiq_global_params_wrap_t params;
    params.type = RESULT_TYPE_AGAMMA_PARAM;
    params.man_param_size = sizeof(gamma_param_t);
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
rk_aiq_user_api2_gamma_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, gamma_api_attrib_t* attr)
{
#if USE_NEWSTRUCT
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AGAMMA);
    RKAIQ_API_SMART_LOCK(sys_ctx);
    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
        for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
            if (!camCtx)
                continue;
            // return the first cam's attrib
            return _gamma_GetAttrib(camCtx , attr);
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        return _gamma_GetAttrib(sys_ctx, attr);
    }
#else
    return XCAM_RETURN_ERROR_UNKNOWN;
#endif
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
_gamma_QueryStatus(const rk_aiq_sys_ctx_t* sys_ctx, gamma_status_t* status)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (sys_ctx->_rkAiqManager->getGlobalParamsManager()->isManual(RESULT_TYPE_AGAMMA_PARAM)) {
        // get cur manual params
        rk_aiq_global_params_wrap_t params;
        params.type = RESULT_TYPE_AGAMMA_PARAM;
        params.man_param_size = sizeof(gamma_param_t);
        params.man_param_ptr = &status->stMan;
        ret = sys_ctx->_rkAiqManager->getGlobalParamsManager()->get(&params);
        if (ret == XCAM_RETURN_NO_ERROR) {
            status->en = params.en;
            status->bypass = params.bypass;
            status->opMode = (RKAiqOPMode_t)params.opMode;
        }
    } else {
        RkAiqGammaHandleInt* algo_handle =
            algoHandle<RkAiqGammaHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AGAMMA);

        if (algo_handle) {
            return algo_handle->queryStatus(status);
        }
    }

    return ret;
}

XCamReturn
rk_aiq_user_api2_gamma_QueryStatus(const rk_aiq_sys_ctx_t* sys_ctx, gamma_status_t* status)
{
#if USE_NEWSTRUCT
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AGAMMA);
    RKAIQ_API_SMART_LOCK(sys_ctx);
    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
        for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
            if (!camCtx)
                continue;
            // return the first cam's attrib
            return _gamma_QueryStatus(camCtx , status);
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        return _gamma_QueryStatus(sys_ctx, status);
    }
#else
    return XCAM_RETURN_ERROR_UNKNOWN;
#endif
   return XCAM_RETURN_NO_ERROR;
}

RKAIQ_END_DECLARE
