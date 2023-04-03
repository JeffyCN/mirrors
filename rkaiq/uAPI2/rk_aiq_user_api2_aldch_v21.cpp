/*
 *  Copyright (c) 2019 Rockchip Corporation
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

#include "rk_aiq_user_api2_aldch_v21.h"

RKAIQ_BEGIN_DECLARE

#ifdef RK_SIMULATOR_HW
#define CHECK_USER_API_ENABLE
#endif

#if RKAIQ_HAVE_LDCH_V21
XCamReturn
rk_aiq_user_api2_aldch_v21_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, const rk_aiq_ldch_v21_attrib_t* attr)
{
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_ALDCH);
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
    #ifdef RKAIQ_ENABLE_CAMGROUP
        RkAiqCamGroupAldchHandleInt* algo_handle =
            camgroupAlgoHandle<RkAiqCamGroupAldchHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_ALDCH);

        if (algo_handle) {
            return algo_handle->setAttrib(attr);
        } else {
            XCamReturn ret = XCAM_RETURN_ERROR_FAILED;
            const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
            for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
                if (!camCtx)
                    continue;

                RkAiqAldchHandleInt* singleCam_algo_handle =
                    algoHandle<RkAiqAldchHandleInt>(camCtx, RK_AIQ_ALGO_TYPE_ALDCH);
                if (singleCam_algo_handle)
                    ret = singleCam_algo_handle->setAttrib(attr);
            }
            return ret;
        }
    #else
        return XCAM_RETURN_ERROR_FAILED;
    #endif
    } else {
        if (attr->sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC && \
            attr->lut.update_flag && \
            attr->update_lut_mode == RK_AIQ_LDCH_UPDATE_LUT_FROM_EXTERNAL_BUFFER) {
            LOGE_ALDCH("Unsupported update_lut_mode %d in async mode.\n", attr->update_lut_mode);
            return XCAM_RETURN_ERROR_PARAM;
        }

        RkAiqAldchHandleInt* algo_handle =
            algoHandle<RkAiqAldchHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_ALDCH);

        if (algo_handle) {
            return algo_handle->setAttrib(attr);
        }
    }

    return XCAM_RETURN_ERROR_FAILED;
}

XCamReturn
rk_aiq_user_api2_aldch_v21_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_ldch_v21_attrib_t *attr)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
    #ifdef RKAIQ_ENABLE_CAMGROUP
        RkAiqCamGroupAldchHandleInt* algo_handle =
            camgroupAlgoHandle<RkAiqCamGroupAldchHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_ALDCH);

        if (algo_handle) {
            return algo_handle->getAttrib(attr);
        } else {
            XCamReturn ret = XCAM_RETURN_ERROR_FAILED;
            const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
            for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
                if (!camCtx)
                    continue;

                RkAiqAldchHandleInt* singleCam_algo_handle =
                    algoHandle<RkAiqAldchHandleInt>(camCtx, RK_AIQ_ALGO_TYPE_ALDCH);
                if (singleCam_algo_handle)
                    ret = singleCam_algo_handle->getAttrib(attr);
            }
            return ret;
        }
    #else
        return XCAM_RETURN_ERROR_FAILED;
    #endif
    } else {
        RkAiqAldchHandleInt* algo_handle =
            algoHandle<RkAiqAldchHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_ALDCH);

        if (algo_handle) {
            return algo_handle->getAttrib(attr);
        }
    }

    return XCAM_RETURN_ERROR_FAILED;
}
#else
XCamReturn
rk_aiq_user_api2_aldch_v21_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, const rk_aiq_ldch_v21_attrib_t* attr)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn
rk_aiq_user_api2_aldch_v21_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_ldch_v21_attrib_t *attr)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}
#endif

RKAIQ_END_DECLARE
