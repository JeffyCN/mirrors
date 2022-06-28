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
#include "rk_aiq_user_api_af.h"

#include "algo_handlers/RkAiqAfHandle.h"
#include "rk_aiq_user_api_sysctl.h"

RKAIQ_BEGIN_DECLARE

#ifdef RK_SIMULATOR_HW
#define CHECK_USER_API_ENABLE
#endif

XCamReturn
rk_aiq_user_api_af_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_af_attrib_t *attr)
{
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AF);
    RKAIQ_API_SMART_LOCK(sys_ctx);
    RkAiqAfHandleInt* algo_handle =
        algoHandle<RkAiqAfHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AF);

    if (algo_handle) {
        return algo_handle->setAttrib(attr);
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api_af_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_af_attrib_t *attr)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    RkAiqAfHandleInt* algo_handle =
        algoHandle<RkAiqAfHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AF);

    if (algo_handle) {
        return algo_handle->getAttrib(attr);
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api_af_Lock(const rk_aiq_sys_ctx_t* sys_ctx)
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
rk_aiq_user_api_af_Unlock(const rk_aiq_sys_ctx_t* sys_ctx)
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
rk_aiq_user_api_af_Oneshot(const rk_aiq_sys_ctx_t* sys_ctx)
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
rk_aiq_user_api_af_ManualTriger(const rk_aiq_sys_ctx_t* sys_ctx)
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
rk_aiq_user_api_af_Tracking(const rk_aiq_sys_ctx_t* sys_ctx)
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
rk_aiq_user_api_af_SetZoomIndex(const rk_aiq_sys_ctx_t* sys_ctx, int index)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    CamCalibDbV2Context_t calibdbv2_ctx =
        RkAiqCalibDbV2::toDefaultCalibDb(sys_ctx->_calibDbProj);
    RkAiqAfHandleInt* algo_handle =
        algoHandle<RkAiqAfHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AF);
    CalibDbV2_Af_ZoomFocusTbl_t *zoomfocus_tbl;
    int focal_length_len;

    if (CHECK_ISP_HW_V30()) {
        CalibDbV2_AFV30_t *af_v30 =
            (CalibDbV2_AFV30_t*)(CALIBDBV2_GET_MODULE_PTR((&calibdbv2_ctx), af_v30));
        zoomfocus_tbl = (CalibDbV2_Af_ZoomFocusTbl_t*)(&af_v30->TuningPara.zoomfocus_tbl);
    } else {
        CalibDbV2_AF_t *af =
            (CalibDbV2_AF_t*)CALIBDBV2_GET_MODULE_PTR((&calibdbv2_ctx), af);
        zoomfocus_tbl = (CalibDbV2_Af_ZoomFocusTbl_t*)(&af->TuningPara.zoomfocus_tbl);
    }
    focal_length_len = zoomfocus_tbl->focal_length_len;

    if (index < 0)
        index = 0;
    else if (index >= focal_length_len)
        index = focal_length_len - 1;

    if (algo_handle) {
        return algo_handle->setZoomIndex(index);
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api_af_EndZoomChg(const rk_aiq_sys_ctx_t* sys_ctx)
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
rk_aiq_user_api_af_GetZoomIndex(const rk_aiq_sys_ctx_t* sys_ctx, int *index)
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
rk_aiq_user_api_af_StartZoomCalib(const rk_aiq_sys_ctx_t* sys_ctx)
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
rk_aiq_user_api_af_resetZoom(const rk_aiq_sys_ctx_t* sys_ctx)
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
rk_aiq_user_api_af_SetVcmCfg(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_lens_vcmcfg* cfg)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ret = sys_ctx->_camHw->setLensVcmCfg(*cfg);

    return ret;
}

XCamReturn
rk_aiq_user_api_af_GetVcmCfg(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_lens_vcmcfg* cfg)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ret = sys_ctx->_camHw->getLensVcmCfg(*cfg);

    return ret;
}

XCamReturn
rk_aiq_user_api_af_GetSearchPath(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_af_sec_path_t* path)
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
rk_aiq_user_api_af_GetSearchResult(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_af_result_t* result)
{
    RkAiqAfHandleInt* algo_handle =
        algoHandle<RkAiqAfHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AF);

    if (algo_handle) {
        return algo_handle->GetSearchResult(result);
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api_af_GetZoomRange(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_af_zoomrange* range)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    CamCalibDbV2Context_t calibdbv2_ctx =
        RkAiqCalibDbV2::toDefaultCalibDb(sys_ctx->_calibDbProj);
    CalibDbV2_Af_ZoomFocusTbl_t *zoomfocus_tbl;
    int focal_length_len;

    if (CHECK_ISP_HW_V30()) {
        CalibDbV2_AFV30_t *af_v30 =
            (CalibDbV2_AFV30_t*)(CALIBDBV2_GET_MODULE_PTR((&calibdbv2_ctx), af_v30));
        zoomfocus_tbl = (CalibDbV2_Af_ZoomFocusTbl_t*)(&af_v30->TuningPara.zoomfocus_tbl);
    } else {
        CalibDbV2_AF_t *af =
            (CalibDbV2_AF_t*)CALIBDBV2_GET_MODULE_PTR((&calibdbv2_ctx), af);
        zoomfocus_tbl = (CalibDbV2_Af_ZoomFocusTbl_t*)(&af->TuningPara.zoomfocus_tbl);
    }
    focal_length_len = zoomfocus_tbl->focal_length_len;

    if (focal_length_len > 1) {
        range->min_fl = zoomfocus_tbl->focal_length[0];
        range->max_fl = zoomfocus_tbl->focal_length[focal_length_len - 1];
        range->min_pos = 0;
        range->max_pos = focal_length_len - 1;
    } else {
        ret = XCAM_RETURN_ERROR_FAILED;
    }

    return ret;
}

XCamReturn
rk_aiq_user_api_af_GetFocusRange(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_af_focusrange* range)
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
rk_aiq_user_api_af_FocusCorrection(const rk_aiq_sys_ctx_t* sys_ctx)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ret = sys_ctx->_camHw->FocusCorrection();

    return ret;
}

XCamReturn
rk_aiq_user_api_af_ZoomCorrection(const rk_aiq_sys_ctx_t* sys_ctx)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ret = sys_ctx->_camHw->ZoomCorrection();

    return ret;
}

XCamReturn
rk_aiq_user_api_af_setAngleZ(const rk_aiq_sys_ctx_t* sys_ctx, float angleZ)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ret = sys_ctx->_camHw->setAngleZ(angleZ);

    return ret;
}

RKAIQ_END_DECLARE
