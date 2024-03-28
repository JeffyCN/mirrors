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
#include "uAPI2/rk_aiq_user_api2_amerge.h"

#include "RkAiqCamGroupHandleInt.h"
#include "algo_handlers/RkAiqAmergeHandle.h"

RKAIQ_BEGIN_DECLARE

#ifdef RK_SIMULATOR_HW
#define CHECK_USER_API_ENABLE
#endif

XCamReturn rk_aiq_user_api2_amerge_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx,
                                             amerge_attrib_t attr) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if RKAIQ_HAVE_MERGE_V10
    mergeAttrV10_t attr_v10;
    memset(&attr_v10, 0x0, sizeof(mergeAttrV10_t));

    attr_v10.sync.sync_mode = attr.sync.sync_mode;
    attr_v10.sync.done      = attr.sync.done;
    if (attr.attrV21.opMode == MERGE_OPMODE_API_OFF) attr_v10.opMode = MERGE_OPMODE_AUTO;
    if (attr.attrV21.opMode == MERGE_OPMODE_MANU) attr_v10.opMode = MERGE_OPMODE_MANUAL;
    attr_v10.stManual.OECurve = attr.attrV21.stManual.OECurve;
    attr_v10.stManual.MDCurve = attr.attrV21.stManual.MDCurve;
    ret                       = rk_aiq_user_api2_amerge_v10_SetAttrib(sys_ctx, &attr_v10);
#endif
#if RKAIQ_HAVE_MERGE_V11
    mergeAttrV11_t attr_v11;
    memset(&attr_v11, 0x0, sizeof(mergeAttrV11_t));

    attr_v11.sync.sync_mode = attr.sync.sync_mode;
    attr_v11.sync.done      = attr.sync.done;
    if (attr.attrV30.opMode == MERGE_OPMODE_API_OFF) attr_v11.opMode = MERGE_OPMODE_AUTO;
    if (attr.attrV30.opMode == MERGE_OPMODE_MANU) attr_v11.opMode = MERGE_OPMODE_MANUAL;
    attr_v11.stManual.BaseFrm                  = attr.attrV30.stManual.BaseFrm;
    attr_v11.stManual.LongFrmModeData.OECurve  = attr.attrV30.stManual.LongFrmModeData.OECurve;
    attr_v11.stManual.LongFrmModeData.MDCurve  = attr.attrV30.stManual.LongFrmModeData.MDCurve;
    attr_v11.stManual.ShortFrmModeData.OECurve = attr.attrV30.stManual.ShortFrmModeData.OECurve;
    attr_v11.stManual.ShortFrmModeData.MDCurve = attr.attrV30.stManual.ShortFrmModeData.MDCurve;
    ret = rk_aiq_user_api2_amerge_v11_SetAttrib(sys_ctx, &attr_v11);
#endif
    return ret;
}
XCamReturn rk_aiq_user_api2_amerge_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx,
                                             amerge_attrib_t* attr) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if RKAIQ_HAVE_MERGE_V10
    mergeAttrV10_t attr_v10;
    memset(&attr_v10, 0x0, sizeof(mergeAttrV10_t));
    ret = rk_aiq_user_api2_amerge_v10_GetAttrib(sys_ctx, &attr_v10);

    attr->sync.sync_mode = attr_v10.sync.sync_mode;
    attr->sync.done      = attr_v10.sync.done;
    if (attr_v10.opMode == MERGE_OPMODE_AUTO) attr->attrV21.opMode = MERGE_OPMODE_API_OFF;
    if (attr_v10.opMode == MERGE_OPMODE_MANUAL) attr->attrV21.opMode = MERGE_OPMODE_MANU;
    attr->attrV21.stManual.OECurve = attr_v10.stManual.OECurve;
    attr->attrV21.stManual.MDCurve = attr_v10.stManual.MDCurve;
    attr->attrV21.CtlInfo          = attr_v10.Info;
#endif
#if RKAIQ_HAVE_MERGE_V11
    mergeAttrV11_t attr_v11;
    memset(&attr_v11, 0x0, sizeof(mergeAttrV11_t));
    ret = rk_aiq_user_api2_amerge_v11_GetAttrib(sys_ctx, &attr_v11);

    attr->sync.sync_mode = attr_v11.sync.sync_mode;
    attr->sync.done      = attr_v11.sync.done;
    if (attr_v11.opMode == MERGE_OPMODE_AUTO) attr->attrV30.opMode = MERGE_OPMODE_API_OFF;
    if (attr_v11.opMode == MERGE_OPMODE_MANUAL) attr->attrV30.opMode = MERGE_OPMODE_MANU;
    attr->attrV30.stManual.BaseFrm                  = attr_v11.stManual.BaseFrm;
    attr->attrV30.stManual.LongFrmModeData.OECurve  = attr_v11.stManual.LongFrmModeData.OECurve;
    attr->attrV30.stManual.LongFrmModeData.MDCurve  = attr_v11.stManual.LongFrmModeData.MDCurve;
    attr->attrV30.stManual.ShortFrmModeData.OECurve = attr_v11.stManual.ShortFrmModeData.OECurve;
    attr->attrV30.stManual.ShortFrmModeData.MDCurve = attr_v11.stManual.ShortFrmModeData.MDCurve;
    attr->attrV30.CtlInfo                           = attr_v11.Info;
#endif
    return ret;
}

#if RKAIQ_HAVE_MERGE_V10
XCamReturn rk_aiq_user_api2_amerge_v10_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx,
                                                 const mergeAttrV10_t* attr) {
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AMERGE);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP

        RkAiqCamGroupAmergeHandleInt* algo_handle =
            camgroupAlgoHandle<RkAiqCamGroupAmergeHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AMERGE);

        if (algo_handle) {
            return algo_handle->setAttribV10(attr);
        } else {
            XCamReturn ret                            = XCAM_RETURN_NO_ERROR;
            const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
            for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
                if (!camCtx)
                    continue;

                RkAiqAmergeHandleInt* singleCam_algo_handle =
                    algoHandle<RkAiqAmergeHandleInt>(camCtx, RK_AIQ_ALGO_TYPE_AMERGE);
                if (singleCam_algo_handle) {
                    ret = singleCam_algo_handle->setAttribV10(attr);
                    if (ret != XCAM_RETURN_NO_ERROR) LOGE("%s returned: %d", __FUNCTION__, ret);
                }
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        RkAiqAmergeHandleInt* algo_handle =
            algoHandle<RkAiqAmergeHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AMERGE);

        if (algo_handle) {
            return algo_handle->setAttribV10(attr);
        }
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn rk_aiq_user_api2_amerge_v10_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx,
                                                 mergeAttrV10_t* attr) {
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        RkAiqCamGroupAmergeHandleInt* algo_handle =
            camgroupAlgoHandle<RkAiqCamGroupAmergeHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AMERGE);

        if (algo_handle) {
            return algo_handle->getAttribV10(attr);
        } else {
            const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
            for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
                if (!camCtx)
                    continue;

                RkAiqAmergeHandleInt* singleCam_algo_handle =
                    algoHandle<RkAiqAmergeHandleInt>(camCtx, RK_AIQ_ALGO_TYPE_AMERGE);
                if (singleCam_algo_handle) return singleCam_algo_handle->getAttribV10(attr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        RkAiqAmergeHandleInt* algo_handle =
            algoHandle<RkAiqAmergeHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AMERGE);

        if (algo_handle) {
            return algo_handle->getAttribV10(attr);
        }
    }

    return XCAM_RETURN_NO_ERROR;
}
#else
XCamReturn rk_aiq_user_api2_amerge_v10_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx,
                                                 const mergeAttrV10_t* attr) {
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn rk_aiq_user_api2_amerge_v10_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx,
                                                 mergeAttrV10_t* attr) {
    return XCAM_RETURN_ERROR_UNKNOWN;
}

#endif

#if RKAIQ_HAVE_MERGE_V11
XCamReturn rk_aiq_user_api2_amerge_v11_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx,
                                                 const mergeAttrV11_t* attr) {
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AMERGE);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        RkAiqCamGroupAmergeHandleInt* algo_handle =
            camgroupAlgoHandle<RkAiqCamGroupAmergeHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AMERGE);

        if (algo_handle) {
            return algo_handle->setAttribV11(attr);
        } else {
            const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t*)sys_ctx;
            for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
                if (!camCtx) continue;

                RkAiqAmergeHandleInt* singleCam_algo_handle =
                    algoHandle<RkAiqAmergeHandleInt>(camCtx, RK_AIQ_ALGO_TYPE_AMERGE);
                if (singleCam_algo_handle) ret = singleCam_algo_handle->setAttribV11(attr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        RkAiqAmergeHandleInt* algo_handle =
            algoHandle<RkAiqAmergeHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AMERGE);

        if (algo_handle) {
            return algo_handle->setAttribV11(attr);
        }
    }

    return ret;
}

XCamReturn rk_aiq_user_api2_amerge_v11_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx,
                                                 mergeAttrV11_t* attr) {
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        RkAiqCamGroupAmergeHandleInt* algo_handle =
            camgroupAlgoHandle<RkAiqCamGroupAmergeHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AMERGE);

        if (algo_handle) {
            return algo_handle->getAttribV11(attr);
        } else {
            const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t*)sys_ctx;
            for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
                if (!camCtx) continue;

                RkAiqAmergeHandleInt* singleCam_algo_handle =
                    algoHandle<RkAiqAmergeHandleInt>(camCtx, RK_AIQ_ALGO_TYPE_AMERGE);
                if (singleCam_algo_handle) return singleCam_algo_handle->getAttribV11(attr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        RkAiqAmergeHandleInt* algo_handle =
            algoHandle<RkAiqAmergeHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AMERGE);

        if (algo_handle) {
            return algo_handle->getAttribV11(attr);
        }
    }

    return XCAM_RETURN_NO_ERROR;
}
#else
XCamReturn rk_aiq_user_api2_amerge_v11_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx,
                                                 const mergeAttrV11_t* attr) {
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn rk_aiq_user_api2_amerge_v11_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx,
                                                 mergeAttrV11_t* attr) {
    return XCAM_RETURN_ERROR_UNKNOWN;
}

#endif

#if RKAIQ_HAVE_MERGE_V12
XCamReturn rk_aiq_user_api2_amerge_v12_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx,
                                                 const mergeAttrV12_t* attr) {
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AMERGE);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        RkAiqCamGroupAmergeHandleInt* algo_handle =
            camgroupAlgoHandle<RkAiqCamGroupAmergeHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AMERGE);

        if (algo_handle) {
            return algo_handle->setAttribV12(attr);
        } else {
            const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t*)sys_ctx;
            for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
                if (!camCtx) continue;

                RkAiqAmergeHandleInt* singleCam_algo_handle =
                    algoHandle<RkAiqAmergeHandleInt>(camCtx, RK_AIQ_ALGO_TYPE_AMERGE);
                if (singleCam_algo_handle) ret = singleCam_algo_handle->setAttribV12(attr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        RkAiqAmergeHandleInt* algo_handle =
            algoHandle<RkAiqAmergeHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AMERGE);

        if (algo_handle) {
            return algo_handle->setAttribV12(attr);
        }
    }

    return ret;
}

XCamReturn rk_aiq_user_api2_amerge_v12_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx,
                                                 mergeAttrV12_t* attr) {
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        RkAiqCamGroupAmergeHandleInt* algo_handle =
            camgroupAlgoHandle<RkAiqCamGroupAmergeHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AMERGE);

        if (algo_handle) {
            return algo_handle->getAttribV12(attr);
        } else {
            const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t*)sys_ctx;
            for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
                if (!camCtx) continue;

                RkAiqAmergeHandleInt* singleCam_algo_handle =
                    algoHandle<RkAiqAmergeHandleInt>(camCtx, RK_AIQ_ALGO_TYPE_AMERGE);
                if (singleCam_algo_handle) return singleCam_algo_handle->getAttribV12(attr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        RkAiqAmergeHandleInt* algo_handle =
            algoHandle<RkAiqAmergeHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AMERGE);

        if (algo_handle) {
            return algo_handle->getAttribV12(attr);
        }
    }

    return XCAM_RETURN_NO_ERROR;
}
#else
XCamReturn rk_aiq_user_api2_amerge_v12_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx,
                                                 const mergeAttrV12_t* attr) {
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn rk_aiq_user_api2_amerge_v12_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx,
                                                 mergeAttrV12_t* attr) {
    return XCAM_RETURN_ERROR_UNKNOWN;
}

#endif

RKAIQ_END_DECLARE
