/*
 *  Copyright (c) 2019-2022 Rockchip Electronics Co., Ltd
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
#include "uAPI2/rk_aiq_user_api2_agic.h"

#include "uAPI/include/rk_aiq_user_api_agic.h"
#include "uAPI/rk_aiq_api_private.h"

RKAIQ_BEGIN_DECLARE

#if RKAIQ_HAVE_GIC_V1
XCamReturn rk_aiq_user_api2_agic_v1_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx,
                                              const rkaiq_gic_v1_api_attr_t* attr) {
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AGIC);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        RkAiqCamGroupAgicHandleInt* algo_handle =
            camgroupAlgoHandle<RkAiqCamGroupAgicHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AGIC);

        if (algo_handle) {
            return algo_handle->setAttribV1(attr);
        } else {
            XCamReturn ret                            = XCAM_RETURN_NO_ERROR;
            const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t*)sys_ctx;
            for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
                if (!camCtx) continue;

                RkAiqAgicHandleInt* singleCam_algo_handle =
                    algoHandle<RkAiqAgicHandleInt>(camCtx, RK_AIQ_ALGO_TYPE_AGIC);
                if (singleCam_algo_handle) {
                    ret = singleCam_algo_handle->setAttribV1(attr);
                    if (ret != XCAM_RETURN_NO_ERROR) LOGE("%s returned: %d", __FUNCTION__, ret);
                }
            }
            return ret;
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        RkAiqAgicHandleInt* algo_handle =
            algoHandle<RkAiqAgicHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AGIC);

        if (algo_handle) {
            return algo_handle->setAttribV1(attr);
        }
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn rk_aiq_user_api2_agic_v1_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx,
                                              rkaiq_gic_v1_api_attr_t* attr) {
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        RkAiqCamGroupAgicHandleInt* algo_handle =
            camgroupAlgoHandle<RkAiqCamGroupAgicHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AGIC);

        if (algo_handle) {
            return algo_handle->getAttribV1(attr);
        } else {
            const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t*)sys_ctx;
            for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
                if (!camCtx) continue;

                RkAiqAgicHandleInt* singleCam_algo_handle =
                    algoHandle<RkAiqAgicHandleInt>(camCtx, RK_AIQ_ALGO_TYPE_AGIC);
                if (singleCam_algo_handle) return singleCam_algo_handle->getAttribV1(attr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        RkAiqAgicHandleInt* algo_handle =
            algoHandle<RkAiqAgicHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AGIC);

        if (algo_handle) {
            return algo_handle->getAttribV1(attr);
        }
    }

    return XCAM_RETURN_NO_ERROR;
}
#else

XCamReturn rk_aiq_user_api2_agic_v1_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx,
                                              const rkaiq_gic_v1_api_attr_t* attr) {
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn rk_aiq_user_api2_agic_v1_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx,
                                              rkaiq_gic_v1_api_attr_t* attr) {
    return XCAM_RETURN_ERROR_UNKNOWN;
}

#endif

#if RKAIQ_HAVE_GIC_V2
XCamReturn rk_aiq_user_api2_agic_v2_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx,
                                              const rkaiq_gic_v2_api_attr_t* attr) {
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AGIC);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        RkAiqCamGroupAgicHandleInt* algo_handle =
            camgroupAlgoHandle<RkAiqCamGroupAgicHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AGIC);

        if (algo_handle) {
            return algo_handle->setAttribV2(attr);
        } else {
            const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t*)sys_ctx;
            for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
                if (!camCtx) continue;

                RkAiqAgicHandleInt* singleCam_algo_handle =
                    algoHandle<RkAiqAgicHandleInt>(camCtx, RK_AIQ_ALGO_TYPE_AGIC);
                if (singleCam_algo_handle) ret = singleCam_algo_handle->setAttribV2(attr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        RkAiqAgicHandleInt* algo_handle =
            algoHandle<RkAiqAgicHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AGIC);

        if (algo_handle) {
            return algo_handle->setAttribV2(attr);
        }
    }

    return ret;
}

XCamReturn rk_aiq_user_api2_agic_v2_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx,
                                              rkaiq_gic_v2_api_attr_t* attr) {
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        RkAiqCamGroupAgicHandleInt* algo_handle =
            camgroupAlgoHandle<RkAiqCamGroupAgicHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AGIC);

        if (algo_handle) {
            return algo_handle->getAttribV2(attr);
        } else {
            const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t*)sys_ctx;
            for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
                if (!camCtx) continue;

                RkAiqAgicHandleInt* singleCam_algo_handle =
                    algoHandle<RkAiqAgicHandleInt>(camCtx, RK_AIQ_ALGO_TYPE_AGIC);
                if (singleCam_algo_handle) return singleCam_algo_handle->getAttribV2(attr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        RkAiqAgicHandleInt* algo_handle =
            algoHandle<RkAiqAgicHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AGIC);

        if (algo_handle) {
            return algo_handle->getAttribV2(attr);
        }
    }

    return XCAM_RETURN_NO_ERROR;
}
#else

XCamReturn rk_aiq_user_api2_agic_v2_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx,
                                              const rkaiq_gic_v2_api_attr_t* attr) {
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn rk_aiq_user_api2_agic_v2_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx,
                                              rkaiq_gic_v2_api_attr_t* attr) {
    return XCAM_RETURN_ERROR_UNKNOWN;
}

#endif

RKAIQ_END_DECLARE

