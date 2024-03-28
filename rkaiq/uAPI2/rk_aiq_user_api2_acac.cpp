/*
 *  Copyright (c) 2022 Rockchip Electronics Co., Ltd.
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
#include "rk_aiq_user_api2_acac.h"

#include "Isp20_module_dbg.h"
#include "aiq_core/algo_camgroup_handlers/RkAiqCamGroupAcacHandle.h"
#include "aiq_core/algo_handlers/RkAiqAcacV11Handle.h"
#include "algos/acac/rk_aiq_types_acac_algo_int.h"
#include "common/rk_aiq.h"
#include "uAPI/rk_aiq_api_private.h"
#include "xcore/base/xcam_common.h"

RKAIQ_BEGIN_DECLARE

using namespace RkCam;

typedef struct rk_aiq_sys_ctx_s rk_aiq_sys_ctx_t;

#ifdef RK_SIMULATOR_HW
#define CHECK_USER_API_ENABLE
#endif

#if RKAIQ_HAVE_CAC_V03
XCamReturn rk_aiq_user_api2_acac_v03_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx,
                                           const rkaiq_cac_v03_api_attr_t* attr) {
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_ACAC);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    XCAM_ASSERT(attr != nullptr);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        RkAiqCamGroupAcacHandleInt* algo_handle =
            camgroupAlgoHandle<RkAiqCamGroupAcacHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_ACAC);

        if (algo_handle) {
            return algo_handle->setAttribV03(attr);
        } else {
            const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t*)sys_ctx;
            for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
                if (!camCtx) continue;

                RkAiqAcacV3HandleInt* singleCam_algo_handle =
                    algoHandle<RkAiqAcacV3HandleInt>(camCtx, RK_AIQ_ALGO_TYPE_ACAC);
                if (singleCam_algo_handle) ret = singleCam_algo_handle->setAttrib(attr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        RkAiqAcacV3HandleInt* algo_handle =
            algoHandle<RkAiqAcacV3HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_ACAC);

        if (algo_handle) {
            return algo_handle->setAttrib(attr);
        }
    }

    return ret;

}

XCamReturn rk_aiq_user_api2_acac_v03_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx,
                                           rkaiq_cac_v03_api_attr_t* attr) {
    RKAIQ_API_SMART_LOCK(sys_ctx);

    XCAM_ASSERT(attr != nullptr);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        RkAiqCamGroupAcacHandleInt* algo_handle =
            camgroupAlgoHandle<RkAiqCamGroupAcacHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_ACAC);

        if (algo_handle) {
            return algo_handle->getAttribV03(attr);
        } else {
            const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t*)sys_ctx;
            for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
                if (!camCtx) continue;

                RkAiqAcacV3HandleInt* singleCam_algo_handle =
                    algoHandle<RkAiqAcacV3HandleInt>(camCtx, RK_AIQ_ALGO_TYPE_ACAC);
                if (singleCam_algo_handle) return singleCam_algo_handle->getAttrib(attr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        RkAiqAcacV3HandleInt* algo_handle =
            algoHandle<RkAiqAcacV3HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_ACAC);

        if (algo_handle) {
            return algo_handle->getAttrib(attr);
        }
    }

    return XCAM_RETURN_NO_ERROR;
}
#else
XCamReturn rk_aiq_user_api2_acac_v03_SetAttrib(const rk_aiq_sys_ctx_t* /* sys_ctx */,
                                           const rkaiq_cac_v03_api_attr_t* /* attr */) {
    return XCAM_RETURN_ERROR_FAILED;
}
XCamReturn rk_aiq_user_api2_acac_v03_GetAttrib(const rk_aiq_sys_ctx_t* /* sys_ctx */,
                                           rkaiq_cac_v03_api_attr_t* /* attr */) {
    return XCAM_RETURN_ERROR_FAILED;
}
#endif

#if RKAIQ_HAVE_CAC_V10
XCamReturn rk_aiq_user_api2_acac_v10_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx,
                                           const rkaiq_cac_v10_api_attr_t* attr) {
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_ACAC);

    XCAM_ASSERT(attr != nullptr);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        RkAiqCamGroupAcacHandleInt* algo_handle =
            camgroupAlgoHandle<RkAiqCamGroupAcacHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_ACAC);

        if (algo_handle) {
            return algo_handle->setAttribV10(attr);
        } else {
            const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t*)sys_ctx;
            for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
                if (!camCtx) continue;

                RkAiqAcacHandleInt* singleCam_algo_handle =
                    algoHandle<RkAiqAcacHandleInt>(camCtx, RK_AIQ_ALGO_TYPE_ACAC);
                if (singleCam_algo_handle) ret = singleCam_algo_handle->setAttrib(attr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        RkAiqAcacHandleInt* algo_handle =
            algoHandle<RkAiqAcacHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_ACAC);

        if (algo_handle) {
            return algo_handle->setAttrib(attr);
        }
    }

    return ret;
}

XCamReturn rk_aiq_user_api2_acac_v10_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx,
                                           rkaiq_cac_v10_api_attr_t* attr) {
    RKAIQ_API_SMART_LOCK(sys_ctx);

    XCAM_ASSERT(attr != nullptr);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        RkAiqCamGroupAcacHandleInt* algo_handle =
            camgroupAlgoHandle<RkAiqCamGroupAcacHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_ACAC);

        if (algo_handle) {
            return algo_handle->getAttribV10(attr);
        } else {
            const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t*)sys_ctx;
            for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
                if (!camCtx) continue;

                RkAiqAcacHandleInt* singleCam_algo_handle =
                    algoHandle<RkAiqAcacHandleInt>(camCtx, RK_AIQ_ALGO_TYPE_ACAC);
                if (singleCam_algo_handle) return singleCam_algo_handle->getAttrib(attr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        RkAiqAcacHandleInt* algo_handle =
            algoHandle<RkAiqAcacHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_ACAC);

        if (algo_handle) {
            return algo_handle->getAttrib(attr);
        }
    }

    return XCAM_RETURN_NO_ERROR;
}
#else
XCamReturn rk_aiq_user_api2_acac_v10_SetAttrib(const rk_aiq_sys_ctx_t* /* sys_ctx */,
                                           const rkaiq_cac_v10_api_attr_t* /* attr */) {
    return XCAM_RETURN_ERROR_FAILED;
}
XCamReturn rk_aiq_user_api2_acac_v10_GetAttrib(const rk_aiq_sys_ctx_t* /* sys_ctx */,
                                           rkaiq_cac_v10_api_attr_t* /* attr */) {
    return XCAM_RETURN_ERROR_FAILED;
}
#endif

#if RKAIQ_HAVE_CAC_V11
XCamReturn rk_aiq_user_api2_acac_v11_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx,
                                           const rkaiq_cac_v11_api_attr_t* attr) {
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_ACAC);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    XCAM_ASSERT(attr != nullptr);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        RkAiqCamGroupAcacHandleInt* algo_handle =
            camgroupAlgoHandle<RkAiqCamGroupAcacHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_ACAC);

        if (algo_handle) {
            return algo_handle->setAttribV11(attr);
        } else {
            const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t*)sys_ctx;
            for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
                if (!camCtx) continue;

                RkAiqAcacV11HandleInt* singleCam_algo_handle =
                    algoHandle<RkAiqAcacV11HandleInt>(camCtx, RK_AIQ_ALGO_TYPE_ACAC);
                if (singleCam_algo_handle) ret = singleCam_algo_handle->setAttrib(attr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        RkAiqAcacV11HandleInt* algo_handle =
            algoHandle<RkAiqAcacV11HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_ACAC);

        if (algo_handle) {
            return algo_handle->setAttrib(attr);
        }
    }

    return ret;
}

XCamReturn rk_aiq_user_api2_acac_v11_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx,
                                           rkaiq_cac_v11_api_attr_t* attr) {
    RKAIQ_API_SMART_LOCK(sys_ctx);

    XCAM_ASSERT(attr != nullptr);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        RkAiqCamGroupAcacHandleInt* algo_handle =
            camgroupAlgoHandle<RkAiqCamGroupAcacHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_ACAC);

        if (algo_handle) {
            return algo_handle->getAttribV11(attr);
        } else {
            const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t*)sys_ctx;
            for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
                if (!camCtx) continue;

                RkAiqAcacV11HandleInt* singleCam_algo_handle =
                    algoHandle<RkAiqAcacV11HandleInt>(camCtx, RK_AIQ_ALGO_TYPE_ACAC);
                if (singleCam_algo_handle) return singleCam_algo_handle->getAttrib(attr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        RkAiqAcacV11HandleInt* algo_handle =
            algoHandle<RkAiqAcacV11HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_ACAC);

        if (algo_handle) {
            return algo_handle->getAttrib(attr);
        }
    }

    return XCAM_RETURN_NO_ERROR;
}
#else
XCamReturn rk_aiq_user_api2_acac_v11_SetAttrib(const rk_aiq_sys_ctx_t* /* sys_ctx */,
                                           const rkaiq_cac_v11_api_attr_t* /* attr */) {
    return XCAM_RETURN_ERROR_FAILED;
}
XCamReturn rk_aiq_user_api2_acac_v11_GetAttrib(const rk_aiq_sys_ctx_t* /* sys_ctx */,
                                           rkaiq_cac_v11_api_attr_t* /* attr */) {
    return XCAM_RETURN_ERROR_FAILED;
}
#endif

RKAIQ_END_DECLARE
