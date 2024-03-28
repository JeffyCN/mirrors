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
 *
 */
#include "uAPI2/rk_aiq_user_api2_af.h"

#include "algo_handlers/RkAiqAfHandle.h"
#include "base/xcam_common.h"

RKAIQ_BEGIN_DECLARE

#ifdef RK_SIMULATOR_HW
#define CHECK_USER_API_ENABLE
#endif

#if RKAIQ_HAVE_AF

XCamReturn
rk_aiq_user_api2_af_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_af_attrib_t *attr)
{
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AF);
    RKAIQ_API_SMART_LOCK(sys_ctx);
    RkAiqAfHandleInt* algo_handle =
        algoHandle<RkAiqAfHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AF);

    if (algo_handle) {
        LOGD_AF("%s: AfMode %d, Win: %d, %d, %d, %d",
            __func__, attr->AfMode, attr->h_offs, attr->v_offs, attr->h_size, attr->v_size);
        return algo_handle->setAttrib(attr);
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api2_af_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_af_attrib_t *attr)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    RkAiqAfHandleInt* algo_handle =
        algoHandle<RkAiqAfHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AF);

    if (algo_handle) {
        return algo_handle->getAttrib(attr);
    }

    return XCAM_RETURN_NO_ERROR;
}

#else

XCamReturn
rk_aiq_user_api2_af_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_af_attrib_t *attr)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn
rk_aiq_user_api2_af_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_af_attrib_t *attr)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}

#endif

#if RKAIQ_HAVE_AF_V20 || RKAIQ_HAVE_AF_V30 || RKAIQ_HAVE_AF_V31 || RKAIQ_HAVE_AF_V32_LITE || RKAIQ_HAVE_AF_V33

XCamReturn
rk_aiq_user_api2_af_SetCalib(const rk_aiq_sys_ctx_t* sys_ctx, void *calib)
{
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AF);
    RKAIQ_API_SMART_LOCK(sys_ctx);
    RkAiqAfHandleInt* algo_handle =
        algoHandle<RkAiqAfHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AF);

    if (algo_handle) {
        return algo_handle->setCalib(calib);
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api2_af_GetCalib(const rk_aiq_sys_ctx_t* sys_ctx, void *calib)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    RkAiqAfHandleInt* algo_handle =
        algoHandle<RkAiqAfHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AF);

    if (algo_handle) {
        return algo_handle->getCalib(calib);
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api2_af_Lock(const rk_aiq_sys_ctx_t* sys_ctx)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    RkAiqAfHandleInt* algo_handle =
        algoHandle<RkAiqAfHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AF);

    if (algo_handle) {
        return algo_handle->lock();
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api2_af_Unlock(const rk_aiq_sys_ctx_t* sys_ctx)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    RkAiqAfHandleInt* algo_handle =
        algoHandle<RkAiqAfHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AF);

    if (algo_handle) {
        return algo_handle->unlock();
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api2_af_Oneshot(const rk_aiq_sys_ctx_t* sys_ctx)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    RkAiqAfHandleInt* algo_handle =
        algoHandle<RkAiqAfHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AF);

    if (algo_handle) {
        return algo_handle->Oneshot();
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api2_af_ManualTriger(const rk_aiq_sys_ctx_t* sys_ctx)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    RkAiqAfHandleInt* algo_handle =
        algoHandle<RkAiqAfHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AF);

    if (algo_handle) {
        return algo_handle->ManualTriger();
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api2_af_Tracking(const rk_aiq_sys_ctx_t* sys_ctx)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    RkAiqAfHandleInt* algo_handle =
        algoHandle<RkAiqAfHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AF);

    if (algo_handle) {
        return algo_handle->Tracking();
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api2_af_SetZoomIndex(const rk_aiq_sys_ctx_t* sys_ctx, int index)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    RkAiqAfHandleInt* algo_handle =
        algoHandle<RkAiqAfHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AF);

    if (algo_handle) {
        return algo_handle->setZoomIndex(index);
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api2_af_EndZoomChg(const rk_aiq_sys_ctx_t* sys_ctx)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    RkAiqAfHandleInt* algo_handle =
        algoHandle<RkAiqAfHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AF);

    if (algo_handle) {
        return algo_handle->endZoomChg();
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api2_af_GetZoomIndex(const rk_aiq_sys_ctx_t* sys_ctx, int *index)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    RkAiqAfHandleInt* algo_handle =
        algoHandle<RkAiqAfHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AF);

    if (algo_handle) {
        return algo_handle->getZoomIndex(index);
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api2_af_StartZoomCalib(const rk_aiq_sys_ctx_t* sys_ctx)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    RkAiqAfHandleInt* algo_handle =
        algoHandle<RkAiqAfHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AF);

    if (algo_handle) {
        return algo_handle->startZoomCalib();
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api2_af_resetZoom(const rk_aiq_sys_ctx_t* sys_ctx)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    RkAiqAfHandleInt* algo_handle =
        algoHandle<RkAiqAfHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AF);

    if (algo_handle) {
        return algo_handle->resetZoom();
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api2_af_SetVcmCfg(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_lens_vcmcfg* cfg)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ret = sys_ctx->_camHw->setLensVcmCfg(*cfg);

    return ret;
}

XCamReturn
rk_aiq_user_api2_af_GetVcmCfg(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_lens_vcmcfg* cfg)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ret = sys_ctx->_camHw->getLensVcmCfg(*cfg);

    return ret;
}

XCamReturn
rk_aiq_user_api2_af_GetSearchPath(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_af_sec_path_t* path)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    RkAiqAfHandleInt* algo_handle =
        algoHandle<RkAiqAfHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AF);

    if (algo_handle) {
        return algo_handle->GetSearchPath(path);
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api2_af_GetSearchResult(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_af_result_t* result)
{
    RkAiqAfHandleInt* algo_handle =
        algoHandle<RkAiqAfHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AF);

    if (algo_handle) {
        return algo_handle->GetSearchResult(result);
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api2_af_GetZoomRange(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_af_zoomrange* range)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqAfHandleInt* algo_handle =
        algoHandle<RkAiqAfHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AF);

    if (algo_handle) {
        return algo_handle->GetZoomRange(range);
    }

    return ret;
}

XCamReturn
rk_aiq_user_api2_af_GetFocusRange(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_af_focusrange* range)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    RkAiqAfHandleInt* algo_handle =
        algoHandle<RkAiqAfHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AF);

    if (algo_handle) {
        return algo_handle->GetFocusRange(range);
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api2_af_FocusCorrection(const rk_aiq_sys_ctx_t* sys_ctx)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ret = sys_ctx->_camHw->FocusCorrection();

    return ret;
}

XCamReturn
rk_aiq_user_api2_af_ZoomCorrection(const rk_aiq_sys_ctx_t* sys_ctx)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ret = sys_ctx->_camHw->ZoomCorrection();

    return ret;
}

XCamReturn
rk_aiq_user_api2_af_setAngleZ(const rk_aiq_sys_ctx_t* sys_ctx, float angleZ)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ret = sys_ctx->_camHw->setAngleZ(angleZ);

    return ret;
}

XCamReturn
rk_aiq_user_api2_af_GetFocusPos(const rk_aiq_sys_ctx_t* sys_ctx, int* pos)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = sys_ctx->_camHw->getFocusPosition(*pos);

    return ret;
}

#else

XCamReturn
rk_aiq_user_api2_af_SetCalib(const rk_aiq_sys_ctx_t* sys_ctx, void *calib)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn
rk_aiq_user_api2_af_GetCalib(const rk_aiq_sys_ctx_t* sys_ctx, void *calib)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn
rk_aiq_user_api2_af_Lock(const rk_aiq_sys_ctx_t* sys_ctx)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn
rk_aiq_user_api2_af_Unlock(const rk_aiq_sys_ctx_t* sys_ctx)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn
rk_aiq_user_api2_af_Oneshot(const rk_aiq_sys_ctx_t* sys_ctx)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn
rk_aiq_user_api2_af_ManualTriger(const rk_aiq_sys_ctx_t* sys_ctx)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn
rk_aiq_user_api2_af_Tracking(const rk_aiq_sys_ctx_t* sys_ctx)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn
rk_aiq_user_api2_af_SetZoomIndex(const rk_aiq_sys_ctx_t* sys_ctx, int index)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn
rk_aiq_user_api2_af_EndZoomChg(const rk_aiq_sys_ctx_t* sys_ctx)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn
rk_aiq_user_api2_af_GetZoomIndex(const rk_aiq_sys_ctx_t* sys_ctx, int *index)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn
rk_aiq_user_api2_af_StartZoomCalib(const rk_aiq_sys_ctx_t* sys_ctx)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn
rk_aiq_user_api2_af_resetZoom(const rk_aiq_sys_ctx_t* sys_ctx)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn
rk_aiq_user_api2_af_SetVcmCfg(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_lens_vcmcfg* cfg)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn
rk_aiq_user_api2_af_GetVcmCfg(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_lens_vcmcfg* cfg)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn
rk_aiq_user_api2_af_GetSearchPath(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_af_sec_path_t* path)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn
rk_aiq_user_api2_af_GetSearchResult(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_af_result_t* result)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn
rk_aiq_user_api2_af_GetZoomRange(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_af_zoomrange* range)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn
rk_aiq_user_api2_af_GetFocusRange(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_af_focusrange* range)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn
rk_aiq_user_api2_af_FocusCorrection(const rk_aiq_sys_ctx_t* sys_ctx)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn
rk_aiq_user_api2_af_ZoomCorrection(const rk_aiq_sys_ctx_t* sys_ctx)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn
rk_aiq_user_api2_af_setAngleZ(const rk_aiq_sys_ctx_t* sys_ctx, float angleZ)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn
rk_aiq_user_api2_af_GetFocusPos(const rk_aiq_sys_ctx_t* sys_ctx, int* pos)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}

#endif

RKAIQ_END_DECLARE
