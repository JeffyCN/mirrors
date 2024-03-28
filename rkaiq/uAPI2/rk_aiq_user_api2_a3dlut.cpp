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
#include "uAPI2/rk_aiq_user_api2_a3dlut.h"

#include "RkAiqCamGroupHandleInt.h"
#include "algo_handlers/RkAiqA3dlutHandle.h"

#ifdef RK_SIMULATOR_HW
#define CHECK_USER_API_ENABLE
#endif
#if RKAIQ_HAVE_3DLUT_V1
XCamReturn rk_aiq_user_api2_a3dlut_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx,
                                             const rk_aiq_lut3d_attrib_t* attr)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_A3DLUT);
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
    #ifdef RKAIQ_ENABLE_CAMGROUP
        RkAiqCamGroupA3dlutHandleInt* algo_handle =
            camgroupAlgoHandle<RkAiqCamGroupA3dlutHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_A3DLUT);

        if (algo_handle) {
            return algo_handle->setAttrib(attr);
        } else {
            const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
            for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
                if (!camCtx)
                    continue;

                RkAiqA3dlutHandleInt* singleCam_algo_handle =
                    algoHandle<RkAiqA3dlutHandleInt>(camCtx, RK_AIQ_ALGO_TYPE_A3DLUT);
                if (singleCam_algo_handle)
                    ret = singleCam_algo_handle->setAttrib(attr);
            }
        }
    #else
        return XCAM_RETURN_ERROR_FAILED;
    #endif
    } else {
        RkAiqA3dlutHandleInt* algo_handle =
            algoHandle<RkAiqA3dlutHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_A3DLUT);

        if (algo_handle) {
            return algo_handle->setAttrib(attr);
        }
    }

    return (ret);
}

XCamReturn rk_aiq_user_api2_a3dlut_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_lut3d_attrib_t *attr)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
    #ifdef RKAIQ_ENABLE_CAMGROUP
        RkAiqCamGroupA3dlutHandleInt* algo_handle =
            camgroupAlgoHandle<RkAiqCamGroupA3dlutHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_A3DLUT);

        if (algo_handle) {
            return algo_handle->getAttrib(attr);
        } else {
            const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
            for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
                if (!camCtx)
                    continue;

                RkAiqA3dlutHandleInt* singleCam_algo_handle =
                    algoHandle<RkAiqA3dlutHandleInt>(camCtx, RK_AIQ_ALGO_TYPE_A3DLUT);
                if (singleCam_algo_handle)
                    ret = singleCam_algo_handle->getAttrib(attr);
            }
        }
    #else
        return XCAM_RETURN_ERROR_FAILED;
    #endif
    } else {
        RkAiqA3dlutHandleInt* algo_handle =
            algoHandle<RkAiqA3dlutHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_A3DLUT);

        if (algo_handle) {
            return algo_handle->getAttrib(attr);
        }
    }

    return (ret);

}

XCamReturn
rk_aiq_user_api2_a3dlut_Query3dlutInfo(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_lut3d_querry_info_t *lut3d_querry_info )
{
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
    #ifdef RKAIQ_ENABLE_CAMGROUP
        RkAiqCamGroupA3dlutHandleInt* algo_handle =
            camgroupAlgoHandle<RkAiqCamGroupA3dlutHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_A3DLUT);

        if (algo_handle) {
            return algo_handle->query3dlutInfo(lut3d_querry_info);
        } else {
            const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
            for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
                if (!camCtx)
                    continue;

                RkAiqA3dlutHandleInt* singleCam_algo_handle =
                    algoHandle<RkAiqA3dlutHandleInt>(camCtx, RK_AIQ_ALGO_TYPE_A3DLUT);
                if (singleCam_algo_handle)
                    return singleCam_algo_handle->query3dlutInfo(lut3d_querry_info);
            }
        }
    #else
        return XCAM_RETURN_ERROR_FAILED;
    #endif
    } else {
        RkAiqA3dlutHandleInt* algo_handle =
            algoHandle<RkAiqA3dlutHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_A3DLUT);

        if (algo_handle) {
            return algo_handle->query3dlutInfo(lut3d_querry_info);
        }
    }

    return XCAM_RETURN_NO_ERROR;
}
XCamReturn rk_aiq_user_api2_a3dlut_SetAcolorSwInfo(const rk_aiq_sys_ctx_t* sys_ctx,
                                              rk_aiq_color_info_t aColor_sw_info)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {

    } else {
        RkAiqA3dlutHandleInt* algo_handle =
            algoHandle<RkAiqA3dlutHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_A3DLUT);

        if (algo_handle) {
            return algo_handle->setAcolorSwInfo(aColor_sw_info);
        }
    }

    return XCAM_RETURN_NO_ERROR;
}

#else

XCamReturn rk_aiq_user_api2_a3dlut_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, const rk_aiq_lut3d_attrib_t* attr)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn rk_aiq_user_api2_a3dlut_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_lut3d_attrib_t *attr)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn rk_aiq_user_api2_a3dlut_Query3dlutInfo(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_lut3d_querry_info_t *lut3d_querry_info)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}

#endif

