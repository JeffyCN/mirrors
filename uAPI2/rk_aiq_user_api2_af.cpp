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

XCamReturn
rk_aiq_user_api2_af_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_af_attrib_t *attr)
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
    CamCalibDbV2Context_t calibdbv2_ctx =
        RkAiqCalibDbV2::toDefaultCalibDb(sys_ctx->_calibDbProj);
    CalibDbV2_Af_ZoomFocusTbl_t *zoomfocus_tbl;
    int focal_length_len;
    RkAiqAfHandleInt* algo_handle =
        algoHandle<RkAiqAfHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AF);

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
rk_aiq_user_api2_af_getCustomAfRes(const rk_aiq_sys_ctx_t* sys_ctx, rk_tool_customAf_res_t *attr)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    RkAiqAfHandleInt* algo_handle =
        algoHandle<RkAiqAfHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AF);

    if (algo_handle) {
        return algo_handle->GetCustomAfRes(attr);
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api2_af_setCustomAfRes(const rk_aiq_sys_ctx_t* sys_ctx, rk_tool_customAf_res_t *attr)
{
    rk_aiq_af_attrib_t af_attr;

    if (CHECK_ISP_HW_V30()) {
        rk_aiq_user_api2_af_GetAttrib(sys_ctx, &af_attr);
        memset(&af_attr.manual_meascfg_v30, 0, sizeof(af_attr.manual_meascfg_v30));
        af_attr.manual_meascfg_v30.af_en = attr->af_en;
        af_attr.manual_meascfg_v30.rawaf_sel = attr->rawaf_sel;
        af_attr.manual_meascfg_v30.accu_8bit_mode = attr->accu_8bit_mode;
        af_attr.manual_meascfg_v30.ae_mode = attr->ae_mode;

        af_attr.manual_meascfg_v30.window_num = attr->window_num;
        af_attr.manual_meascfg_v30.wina_h_offs = attr->wina_h_offs;
        af_attr.manual_meascfg_v30.wina_v_offs = attr->wina_v_offs;
        af_attr.manual_meascfg_v30.wina_h_size = attr->wina_h_size;
        af_attr.manual_meascfg_v30.wina_v_size = attr->wina_v_size;

        af_attr.manual_meascfg_v30.winb_h_offs = attr->winb_h_offs;
        af_attr.manual_meascfg_v30.winb_v_offs = attr->winb_v_offs;
        af_attr.manual_meascfg_v30.winb_h_size = attr->winb_h_size;
        af_attr.manual_meascfg_v30.winb_v_size = attr->winb_v_size;

        af_attr.manual_meascfg_v30.gamma_en = attr->gamma_en;
        memcpy(af_attr.manual_meascfg_v30.gamma_y, attr->gamma_y, RKAIQ_RAWAF_GAMMA_NUM * sizeof(uint16_t));

        // param for winb
        af_attr.manual_meascfg_v30.thres = attr->af_en;
        af_attr.manual_meascfg_v30.shift_sum_a = attr->af_en;
        af_attr.manual_meascfg_v30.shift_y_a = attr->af_en;
        af_attr.manual_meascfg_v30.shift_sum_b = attr->af_en;
        af_attr.manual_meascfg_v30.shift_y_b = attr->af_en;

        af_attr.manual_meascfg_v30.gaus_en = attr->gaus_en;
        af_attr.manual_meascfg_v30.v1_fir_sel = attr->v1_fir_sel;
        af_attr.manual_meascfg_v30.viir_en = attr->viir_en;
        af_attr.manual_meascfg_v30.v1_fv_outmode = attr->v1_fv_outmode;
        af_attr.manual_meascfg_v30.v2_fv_outmode = attr->v2_fv_outmode;
        af_attr.manual_meascfg_v30.v1_fv_shift = attr->v1_fv_shift;
        af_attr.manual_meascfg_v30.v2_fv_shift = attr->v2_fv_shift;
        af_attr.manual_meascfg_v30.v_fv_thresh = attr->v_fv_thresh;
        for (int i = 0; i < 9; i++) {
            af_attr.manual_meascfg_v30.v1_iir_coe[i] = attr->v1_iir_coe[i];
        }
        for (int i = 0; i < 3; i++) {
            af_attr.manual_meascfg_v30.v1_fir_coe[i] = attr->v1_fir_coe[i];
            af_attr.manual_meascfg_v30.v2_iir_coe[i] = attr->v2_iir_coe[i];
            af_attr.manual_meascfg_v30.v2_fir_coe[i] = attr->v2_fir_coe[i];
        }

        af_attr.manual_meascfg_v30.hiir_en = attr->hiir_en;
        af_attr.manual_meascfg_v30.h1_fv_outmode = attr->h1_fv_outmode;
        af_attr.manual_meascfg_v30.h2_fv_outmode = attr->h2_fv_outmode;
        af_attr.manual_meascfg_v30.h1_fv_shift = attr->h1_fv_shift;
        af_attr.manual_meascfg_v30.h2_fv_shift = attr->h2_fv_shift;
        af_attr.manual_meascfg_v30.h_fv_thresh = attr->h_fv_thresh;
        for (int i = 0; i < 6; i++) {
            af_attr.manual_meascfg_v30.h1_iir1_coe[i] = attr->h1_iir1_coe[i];
            af_attr.manual_meascfg_v30.h1_iir2_coe[i] = attr->h1_iir2_coe[i];
            af_attr.manual_meascfg_v30.h2_iir1_coe[i] = attr->h2_iir1_coe[i];
            af_attr.manual_meascfg_v30.h2_iir2_coe[i] = attr->h2_iir2_coe[i];
        }

        // level depended gain
        af_attr.manual_meascfg_v30.ldg_en = attr->af_en;
        af_attr.manual_meascfg_v30.h_ldg_lumth[0] = attr->h_ldg_lumth[0];
        af_attr.manual_meascfg_v30.h_ldg_gain[0]  = attr->h_ldg_gain[0];
        af_attr.manual_meascfg_v30.h_ldg_gslp[0]  = attr->h_ldg_gslp[0];
        af_attr.manual_meascfg_v30.h_ldg_lumth[1] = attr->h_ldg_lumth[1];
        af_attr.manual_meascfg_v30.h_ldg_gain[1]  = attr->h_ldg_gain[1];
        af_attr.manual_meascfg_v30.h_ldg_gslp[1]  = attr->h_ldg_gslp[1];
        af_attr.manual_meascfg_v30.v_ldg_lumth[0] = attr->v_ldg_lumth[0];
        af_attr.manual_meascfg_v30.v_ldg_gain[0]  = attr->v_ldg_gain[0];
        af_attr.manual_meascfg_v30.v_ldg_gslp[0]  = attr->v_ldg_gslp[0];
        af_attr.manual_meascfg_v30.v_ldg_lumth[1] = attr->v_ldg_lumth[1];
        af_attr.manual_meascfg_v30.v_ldg_gain[1]  = attr->v_ldg_gain[1];
        af_attr.manual_meascfg_v30.v_ldg_gslp[1]  = attr->v_ldg_gslp[1];

        // High light
        af_attr.manual_meascfg_v30.highlit_thresh = attr->highlit_thresh;
        af_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_SYNC;
        rk_aiq_user_api2_af_SetAttrib(sys_ctx, &af_attr);
        return XCAM_RETURN_NO_ERROR;
    } else {
        return XCAM_RETURN_ERROR_FAILED;
    }
}

RKAIQ_END_DECLARE
