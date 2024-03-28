/*
 *  Copyright (c) 2021 Rockchip Corporation
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

#include "RkAiqCalibDbV2.h"
#include "RkAiqUapitypes.h"
#include "cJSON.h"
#include "cJSON_Utils.h"
#include "j2s.h"
#include "stdlib.h"
#include "string.h"
#include "uAPI2/rk_aiq_user_api2_wrapper.h"
#include "uAPI2/rk_aiq_user_api2_sysctl.h"
#include "rk_aiq_user_api_sysctl.h"
#include "uAPI2/rk_aiq_user_api2_ae.h"
#include "uAPI2/rk_aiq_user_api2_imgproc.h"
#include "uAPI/include/rk_aiq_user_api_sysctl.h"
#include "uAPI2/rk_aiq_user_api2_acsm.h"
#include "uAPI2/rk_aiq_user_api2_acgc.h"
#include "uAPI2/rk_aiq_user_api2_ablc_v32.h"
#include "uAPI2/rk_aiq_user_api2_a3dlut.h"
#include "uAPI/rk_aiq_api_private.h"

static camgroup_uapi_t last_camindex;

int rk_aiq_uapi_sysctl_swWorkingModeDyn2(const rk_aiq_sys_ctx_t *ctx,
        work_mode_t *mode) {
    return rk_aiq_uapi_sysctl_swWorkingModeDyn(ctx, mode->mode);
}

int rk_aiq_uapi_sysctl_getWorkingModeDyn(const rk_aiq_sys_ctx_t *ctx,
        work_mode_t *mode) {
    return rk_aiq_uapi2_sysctl_getWorkingMode(ctx, &mode->mode);
}

int rk_aiq_uapi2_setWBMode2(rk_aiq_sys_ctx_t *ctx, uapi_wb_mode_t *mode) {
    return rk_aiq_uapi2_setWBMode(ctx, mode->mode);
}

int rk_aiq_uapi2_getWBMode2(rk_aiq_sys_ctx_t *ctx, uapi_wb_mode_t *mode) {
    return rk_aiq_uapi2_getWBMode(ctx, &mode->mode);
}

int rk_aiq_user_api2_set_scene(const rk_aiq_sys_ctx_t* sys_ctx, aiq_scene_t* scene)
{
    return rk_aiq_uapi_sysctl_switch_scene(sys_ctx, scene->main_scene, scene->sub_scene);
}

int rk_aiq_user_api2_get_scene(const rk_aiq_sys_ctx_t* sys_ctx, aiq_scene_t* scene)
{
    (void)sys_ctx;
    scene->main_scene = strdup("normal");
    scene->sub_scene = strdup("day");

    return 0;
}

int rk_aiq_uapi_get_ae_hwstats(const rk_aiq_sys_ctx_t* sys_ctx, uapi_ae_hwstats_t* ae_hwstats)
{
    rk_aiq_isp_stats_t isp_stats;

#if RKAIQ_ENABLE_CAMGROUP
    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
        LOGE("Can't read 3A stats for group ctx!");
        return XCAM_RETURN_ERROR_PARAM;
    }
#endif

    rk_aiq_uapi_sysctl_get3AStats(sys_ctx, &isp_stats);

    memcpy(ae_hwstats, &isp_stats.aec_stats.ae_data, sizeof(uapi_ae_hwstats_t));

    return 0;
}

int rk_aiq_uapi_get_awb_stat(const rk_aiq_sys_ctx_t* sys_ctx, rk_tool_awb_stat_res2_v30_t* awb_stat)
{
    rk_aiq_isp_stats_t isp_stats;

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
        LOGE("Can't read 3A stats for group ctx!");
        return XCAM_RETURN_ERROR_PARAM;
    }

    rk_aiq_uapi_sysctl_get3AStats(sys_ctx, &isp_stats);
    memcpy(awb_stat, &isp_stats.awb_stats_v3x, sizeof(rk_aiq_isp_awb_stats2_v3x_t));

    return 0;
}
int rk_aiq_uapi_get_awbV21_stat(const rk_aiq_sys_ctx_t* sys_ctx, rk_tool_awb_stat_res2_v201_t* awb_stat)
{
    rk_aiq_isp_stats_t isp_stats;

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
        LOGE("Can't read 3A stats for group ctx!");
        return XCAM_RETURN_ERROR_PARAM;
    }

    rk_aiq_uapi_sysctl_get3AStats(sys_ctx, &isp_stats);

    memcpy(awb_stat, &isp_stats.awb_stats_v21, sizeof(rk_aiq_awb_stat_res2_v201_t));

    return 0;
}
int rk_aiq_uapi_get_awbV32_stat(const rk_aiq_sys_ctx_t* sys_ctx, rk_tool_isp_awb_stats_v32_t* awb_stat)
{
    rk_aiq_isp_stats_t isp_stats;

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
        LOGE("Can't read 3A stats for group ctx!");
        return XCAM_RETURN_ERROR_PARAM;
    }

    rk_aiq_uapi_sysctl_get3AStats(sys_ctx, &isp_stats);
    memcpy(awb_stat->light, isp_stats.awb_stats_v32.light, sizeof(awb_stat->light));
    memcpy(awb_stat->blockResult, isp_stats.awb_stats_v32.blockResult, sizeof(isp_stats.awb_stats_v32.blockResult));
    memcpy(awb_stat->WpNo2, isp_stats.awb_stats_v32.WpNo2, sizeof(awb_stat->WpNo2));
    memcpy(awb_stat->WpNoHist, isp_stats.awb_stats_v32.WpNoHist, sizeof(awb_stat->WpNoHist));
    memcpy(awb_stat->excWpRangeResult, isp_stats.awb_stats_v32.excWpRangeResult, sizeof(awb_stat->excWpRangeResult));

    return 0;
}

XCamReturn rk_aiq_get_adpcc_manual_attr(const rk_aiq_sys_ctx_t *sys_ctx,
                                        Adpcc_Manual_Attr_t *manual) {
    rk_aiq_dpcc_attrib_V20_t adpcc_attr;

    memset(&adpcc_attr, 0, sizeof(rk_aiq_dpcc_attrib_V20_t));
    rk_aiq_user_api2_adpcc_GetAttrib(sys_ctx, &adpcc_attr);
    memcpy(manual, &adpcc_attr.stManual, sizeof(Adpcc_Manual_Attr_t));

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn rk_aiq_set_adpcc_manual_attr(const rk_aiq_sys_ctx_t *sys_ctx,
                                        Adpcc_Manual_Attr_t *manual) {
    rk_aiq_dpcc_attrib_V20_t adpcc_attr;

    memset(&adpcc_attr, 0, sizeof(rk_aiq_dpcc_attrib_V20_t));
    rk_aiq_user_api2_adpcc_GetAttrib(sys_ctx, &adpcc_attr);
    memcpy(&adpcc_attr.stManual, manual, sizeof(Adpcc_Manual_Attr_t));
    rk_aiq_user_api2_adpcc_SetAttrib(sys_ctx, &adpcc_attr);

    return XCAM_RETURN_NO_ERROR;
}

#if RKAIQ_HAVE_MERGE_V10
XCamReturn rk_aiq_user_api2_amerge_v10_queryinfo(const rk_aiq_sys_ctx_t* sys_ctx,
        uapiMergeCurrCtlData_t* merge_info) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mergeAttrV10_t merge_attr_v10;
    ret         = rk_aiq_user_api2_amerge_v10_GetAttrib(sys_ctx, &merge_attr_v10);
    *merge_info = merge_attr_v10.Info;

    return ret;
}

XCamReturn rk_aiq_set_amerge_v10_manual_attr(const rk_aiq_sys_ctx_t* sys_ctx,
        mMergeAttrV10_t* manual) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mergeAttrV10_t merge_attr_v10;
    ret                     = rk_aiq_user_api2_amerge_v10_GetAttrib(sys_ctx, &merge_attr_v10);
    merge_attr_v10.stManual = *manual;
    ret                     = rk_aiq_user_api2_amerge_v10_SetAttrib(sys_ctx, &merge_attr_v10);

    return ret;
}

XCamReturn rk_aiq_get_amerge_v10_manual_attr(const rk_aiq_sys_ctx_t* sys_ctx,
        mMergeAttrV10_t* manual) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mergeAttrV10_t merge_attr_v10;
    ret     = rk_aiq_user_api2_amerge_v10_GetAttrib(sys_ctx, &merge_attr_v10);
    *manual = merge_attr_v10.stManual;

    return ret;
}
#endif
#if RKAIQ_HAVE_MERGE_V11
XCamReturn rk_aiq_user_api2_amerge_v11_queryinfo(const rk_aiq_sys_ctx_t* sys_ctx,
        uapiMergeCurrCtlData_t* merge_info) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mergeAttrV11_t merge_attr_v11;
    ret         = rk_aiq_user_api2_amerge_v11_GetAttrib(sys_ctx, &merge_attr_v11);
    *merge_info = merge_attr_v11.Info;

    return ret;
}

XCamReturn rk_aiq_set_amerge_v11_manual_attr(const rk_aiq_sys_ctx_t* sys_ctx,
        mMergeAttrV11_t* manual) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mergeAttrV11_t merge_attr_v11;
    ret                     = rk_aiq_user_api2_amerge_v11_GetAttrib(sys_ctx, &merge_attr_v11);
    merge_attr_v11.stManual = *manual;
    ret                     = rk_aiq_user_api2_amerge_v11_SetAttrib(sys_ctx, &merge_attr_v11);

    return ret;
}

XCamReturn rk_aiq_get_amerge_v11_manual_attr(const rk_aiq_sys_ctx_t* sys_ctx,
        mMergeAttrV11_t* manual) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mergeAttrV11_t merge_attr_v11;
    ret     = rk_aiq_user_api2_amerge_v11_GetAttrib(sys_ctx, &merge_attr_v11);
    *manual = merge_attr_v11.stManual;

    return ret;
}
#endif
#if RKAIQ_HAVE_MERGE_V12
XCamReturn rk_aiq_user_api2_amerge_v12_queryinfo(const rk_aiq_sys_ctx_t* sys_ctx,
        uapiMergeCurrCtlData_t* merge_info) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mergeAttrV12_t merge_attr_v12;
    ret         = rk_aiq_user_api2_amerge_v12_GetAttrib(sys_ctx, &merge_attr_v12);
    *merge_info = merge_attr_v12.Info;

    return ret;
}

XCamReturn rk_aiq_set_amerge_v12_manual_attr(const rk_aiq_sys_ctx_t* sys_ctx,
        mMergeAttrV12_t* manual) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mergeAttrV12_t merge_attr_v12;
    ret                     = rk_aiq_user_api2_amerge_v12_GetAttrib(sys_ctx, &merge_attr_v12);
    merge_attr_v12.stManual = *manual;
    ret                     = rk_aiq_user_api2_amerge_v12_SetAttrib(sys_ctx, &merge_attr_v12);

    return ret;
}

XCamReturn rk_aiq_get_amerge_v12_manual_attr(const rk_aiq_sys_ctx_t* sys_ctx,
        mMergeAttrV12_t* manual) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mergeAttrV12_t merge_attr_v12;
    ret     = rk_aiq_user_api2_amerge_v12_GetAttrib(sys_ctx, &merge_attr_v12);
    *manual = merge_attr_v12.stManual;

    return ret;
}
#endif

#if RKAIQ_HAVE_DRC_V10
XCamReturn rk_aiq_user_api2_adrc_v10_queryinfo(const rk_aiq_sys_ctx_t* sys_ctx,
        DrcInfoV10_t* drc_info) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    drcAttrV10_t drc_attr_v10;
    ret = rk_aiq_user_api2_adrc_v10_GetAttrib(sys_ctx, &drc_attr_v10);
    *drc_info = drc_attr_v10.Info;

    return ret;
}

XCamReturn rk_aiq_set_adrc_v10_manual_attr(const rk_aiq_sys_ctx_t* sys_ctx,
        mdrcAttr_V10_t* manual) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    drcAttrV10_t drc_attr_v10;
    ret = rk_aiq_user_api2_adrc_v10_GetAttrib(sys_ctx, &drc_attr_v10);
    drc_attr_v10.stManual = *manual;
    ret = rk_aiq_user_api2_adrc_v10_SetAttrib(sys_ctx, &drc_attr_v10);

    return ret;
}
XCamReturn rk_aiq_get_adrc_v10_manual_attr(const rk_aiq_sys_ctx_t* sys_ctx,
        mdrcAttr_V10_t* manual) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    drcAttrV10_t drc_attr_v10;
    ret = rk_aiq_user_api2_adrc_v10_GetAttrib(sys_ctx, &drc_attr_v10);
    *manual = drc_attr_v10.stManual;

    return ret;
}
#endif
#if RKAIQ_HAVE_DRC_V11
XCamReturn rk_aiq_user_api2_adrc_v11_queryinfo(const rk_aiq_sys_ctx_t* sys_ctx,
        DrcInfoV11_t* drc_info) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    drcAttrV11_t drc_attr_v11;
    ret = rk_aiq_user_api2_adrc_v11_GetAttrib(sys_ctx, &drc_attr_v11);
    *drc_info = drc_attr_v11.Info;

    return ret;
}

XCamReturn rk_aiq_set_adrc_v11_manual_attr(const rk_aiq_sys_ctx_t* sys_ctx,
        mdrcAttr_V11_t* manual) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    drcAttrV11_t drc_attr_v11;
    ret = rk_aiq_user_api2_adrc_v11_GetAttrib(sys_ctx, &drc_attr_v11);
    drc_attr_v11.stManual = *manual;
    ret = rk_aiq_user_api2_adrc_v11_SetAttrib(sys_ctx, &drc_attr_v11);

    return ret;
}
XCamReturn rk_aiq_get_adrc_v11_manual_attr(const rk_aiq_sys_ctx_t* sys_ctx,
        mdrcAttr_V11_t* manual) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    drcAttrV11_t drc_attr_v11;
    ret = rk_aiq_user_api2_adrc_v11_GetAttrib(sys_ctx, &drc_attr_v11);
    *manual = drc_attr_v11.stManual;

    return ret;
}
#endif
#if RKAIQ_HAVE_DRC_V12
XCamReturn rk_aiq_user_api2_adrc_v12_queryinfo(const rk_aiq_sys_ctx_t* sys_ctx,
        DrcInfoV12_t* drc_info) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    drcAttrV12_t drc_attr_v12;
    ret = rk_aiq_user_api2_adrc_v12_GetAttrib(sys_ctx, &drc_attr_v12);
    *drc_info = drc_attr_v12.Info;

    return ret;
}

XCamReturn rk_aiq_set_adrc_v12_manual_attr(const rk_aiq_sys_ctx_t* sys_ctx,
        mdrcAttr_V12_t* manual) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    drcAttrV12_t drc_attr_v12;
    ret = rk_aiq_user_api2_adrc_v12_GetAttrib(sys_ctx, &drc_attr_v12);
    drc_attr_v12.stManual = *manual;
    ret = rk_aiq_user_api2_adrc_v12_SetAttrib(sys_ctx, &drc_attr_v12);

    return ret;
}
XCamReturn rk_aiq_get_adrc_v12_manual_attr(const rk_aiq_sys_ctx_t* sys_ctx,
        mdrcAttr_V12_t* manual) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    drcAttrV12_t drc_attr_v12;
    ret     = rk_aiq_user_api2_adrc_v12_GetAttrib(sys_ctx, &drc_attr_v12);
    *manual = drc_attr_v12.stManual;

    return ret;
}
#endif
#if RKAIQ_HAVE_DRC_V12_LITE
XCamReturn rk_aiq_user_api2_adrc_v12_lite_queryinfo(const rk_aiq_sys_ctx_t* sys_ctx,
        DrcInfoV12Lite_t* drc_info) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    drcAttrV12Lite_t drc_attr_v12_lite;
    ret       = rk_aiq_user_api2_adrc_v12_lite_GetAttrib(sys_ctx, &drc_attr_v12_lite);
    *drc_info = drc_attr_v12_lite.Info;

    return ret;
}

XCamReturn rk_aiq_set_adrc_v12_lite_manual_attr(const rk_aiq_sys_ctx_t* sys_ctx,
        mdrcAttr_v12_lite_t* manual) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    drcAttrV12Lite_t drc_attr_v12_lite;
    ret = rk_aiq_user_api2_adrc_v12_lite_GetAttrib(sys_ctx, &drc_attr_v12_lite);
    drc_attr_v12_lite.stManual = *manual;
    ret = rk_aiq_user_api2_adrc_v12_lite_SetAttrib(sys_ctx, &drc_attr_v12_lite);

    return ret;
}
XCamReturn rk_aiq_get_adrc_v12_lite_manual_attr(const rk_aiq_sys_ctx_t* sys_ctx,
        mdrcAttr_v12_lite_t* manual) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    drcAttrV12Lite_t drc_attr_v12_lite;
    ret     = rk_aiq_user_api2_adrc_v12_lite_GetAttrib(sys_ctx, &drc_attr_v12_lite);
    *manual = drc_attr_v12_lite.stManual;

    return ret;
}
#endif
#if RKAIQ_HAVE_DRC_V20
XCamReturn rk_aiq_user_api2_adrc_v20_queryinfo(const rk_aiq_sys_ctx_t* sys_ctx,
        DrcInfoV20_t* drc_info) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    drcAttrV20_t drc_attr_v20;
    ret       = rk_aiq_user_api2_adrc_v20_GetAttrib(sys_ctx, &drc_attr_v20);
    *drc_info = drc_attr_v20.Info;

    return ret;
}

XCamReturn rk_aiq_set_adrc_v20_manual_attr(const rk_aiq_sys_ctx_t* sys_ctx,
        mdrcAttr_V20_t* manual) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    drcAttrV20_t drc_attr_v20;
    ret                   = rk_aiq_user_api2_adrc_v20_GetAttrib(sys_ctx, &drc_attr_v20);
    drc_attr_v20.stManual = *manual;
    ret                   = rk_aiq_user_api2_adrc_v20_SetAttrib(sys_ctx, &drc_attr_v20);

    return ret;
}
XCamReturn rk_aiq_get_adrc_v20_manual_attr(const rk_aiq_sys_ctx_t* sys_ctx,
        mdrcAttr_V20_t* manual) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    drcAttrV20_t drc_attr_v20;
    ret     = rk_aiq_user_api2_adrc_v20_GetAttrib(sys_ctx, &drc_attr_v20);
    *manual = drc_attr_v20.stManual;

    return ret;
}
#endif

#if RKAIQ_HAVE_GAMMA_V10
XCamReturn rk_aiq_set_agamma_v10_manual_attr(const rk_aiq_sys_ctx_t* sys_ctx,
        AgammaApiManualV10_t* manual) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_gamma_v10_attr_t gamma_attr_v10;
    ret                     = rk_aiq_user_api2_agamma_v10_GetAttrib(sys_ctx, &gamma_attr_v10);
    gamma_attr_v10.stManual = *manual;
    ret                     = rk_aiq_user_api2_agamma_v10_SetAttrib(sys_ctx, &gamma_attr_v10);

    return ret;
}
XCamReturn rk_aiq_get_agamma_v10_manual_attr(const rk_aiq_sys_ctx_t* sys_ctx,
        AgammaApiManualV10_t* manual) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_gamma_v10_attr_t gamma_attr_v10;
    ret     = rk_aiq_user_api2_agamma_v10_GetAttrib(sys_ctx, &gamma_attr_v10);
    *manual = gamma_attr_v10.stManual;

    return ret;
}
#endif
#if RKAIQ_HAVE_GAMMA_V11
XCamReturn rk_aiq_set_agamma_v11_manual_attr(const rk_aiq_sys_ctx_t* sys_ctx,
        AgammaApiManualV11_t* manual) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_gamma_v11_attr_t gamma_attr_v11;
    ret                     = rk_aiq_user_api2_agamma_v11_GetAttrib(sys_ctx, &gamma_attr_v11);
    gamma_attr_v11.stManual = *manual;
    ret                     = rk_aiq_user_api2_agamma_v11_SetAttrib(sys_ctx, &gamma_attr_v11);

    return ret;
}
XCamReturn rk_aiq_get_agamma_v11_manual_attr(const rk_aiq_sys_ctx_t* sys_ctx,
        AgammaApiManualV11_t* manual) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_gamma_v11_attr_t gamma_attr_v11;
    ret     = rk_aiq_user_api2_agamma_v11_GetAttrib(sys_ctx, &gamma_attr_v11);
    *manual = gamma_attr_v11.stManual;

    return ret;
}
#endif

#if RKAIQ_HAVE_DEHAZE_V11
XCamReturn rk_aiq_user_api2_adehaze_v11_queryinfo(const rk_aiq_sys_ctx_t* sys_ctx,
        mDehazeAttrInfoV11_t* dehaze_info) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    adehaze_sw_v11_t dehaze_attr_v11;
    ret          = rk_aiq_user_api2_adehaze_v11_getSwAttrib(sys_ctx, &dehaze_attr_v11);
    *dehaze_info = dehaze_attr_v11.Info;

    return ret;
}

XCamReturn rk_aiq_set_adehaze_v11_manual_attr(const rk_aiq_sys_ctx_t* sys_ctx,
        mDehazeAttrV11_t* manual) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    adehaze_sw_v11_t dehaze_attr_v11;
    ret                      = rk_aiq_user_api2_adehaze_v11_getSwAttrib(sys_ctx, &dehaze_attr_v11);
    dehaze_attr_v11.stManual = *manual;
    ret                      = rk_aiq_user_api2_adehaze_v11_setSwAttrib(sys_ctx, &dehaze_attr_v11);

    return ret;
}
XCamReturn rk_aiq_get_adehaze_v11_manual_attr(const rk_aiq_sys_ctx_t* sys_ctx,
        mDehazeAttrV11_t* manual) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    adehaze_sw_v11_t dehaze_attr_v11;
    ret     = rk_aiq_user_api2_adehaze_v11_getSwAttrib(sys_ctx, &dehaze_attr_v11);
    *manual = dehaze_attr_v11.stManual;

    return ret;
}
#endif
#if RKAIQ_HAVE_DEHAZE_V12
XCamReturn rk_aiq_user_api2_adehaze_v12_queryinfo(const rk_aiq_sys_ctx_t* sys_ctx,
        mDehazeAttrInfoV11_t* dehaze_info) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    adehaze_sw_v12_t dehaze_attr_v12;
    ret          = rk_aiq_user_api2_adehaze_v12_getSwAttrib(sys_ctx, &dehaze_attr_v12);
    *dehaze_info = dehaze_attr_v12.Info;

    return ret;
}

XCamReturn rk_aiq_set_adehaze_v12_manual_attr(const rk_aiq_sys_ctx_t* sys_ctx,
        mDehazeAttrV12_t* manual) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    adehaze_sw_v12_t dehaze_attr_v12;
    ret                      = rk_aiq_user_api2_adehaze_v12_getSwAttrib(sys_ctx, &dehaze_attr_v12);
    dehaze_attr_v12.stManual = *manual;
    ret                      = rk_aiq_user_api2_adehaze_v12_setSwAttrib(sys_ctx, &dehaze_attr_v12);

    return ret;
}
XCamReturn rk_aiq_get_adehaze_v12_manual_attr(const rk_aiq_sys_ctx_t* sys_ctx,
        mDehazeAttrV12_t* manual) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    adehaze_sw_v12_t dehaze_attr_v12;
    ret     = rk_aiq_user_api2_adehaze_v12_getSwAttrib(sys_ctx, &dehaze_attr_v12);
    *manual = dehaze_attr_v12.stManual;

    return ret;
}
#endif
#if RKAIQ_HAVE_DEHAZE_V14
XCamReturn rk_aiq_user_api2_adehaze_v14_queryinfo(const rk_aiq_sys_ctx_t* sys_ctx,
        mDehazeAttrInfoV11_t* dehaze_info) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    adehaze_sw_v14_t dehaze_attr_v14;
    ret          = rk_aiq_user_api2_adehaze_v14_getSwAttrib(sys_ctx, &dehaze_attr_v14);
    *dehaze_info = dehaze_attr_v14.Info;

    return ret;
}

XCamReturn rk_aiq_set_adehaze_v14_manual_attr(const rk_aiq_sys_ctx_t* sys_ctx,
        mDehazeAttrV14_t* manual) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    adehaze_sw_v14_t dehaze_attr_v14;
    ret                      = rk_aiq_user_api2_adehaze_v14_getSwAttrib(sys_ctx, &dehaze_attr_v14);
    dehaze_attr_v14.stManual = *manual;
    ret                      = rk_aiq_user_api2_adehaze_v14_setSwAttrib(sys_ctx, &dehaze_attr_v14);

    return ret;
}
XCamReturn rk_aiq_get_adehaze_v14_manual_attr(const rk_aiq_sys_ctx_t* sys_ctx,
        mDehazeAttrV14_t* manual) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    adehaze_sw_v14_t dehaze_attr_v14;
    ret     = rk_aiq_user_api2_adehaze_v14_getSwAttrib(sys_ctx, &dehaze_attr_v14);
    *manual = dehaze_attr_v14.stManual;

    return ret;
}
#endif

XCamReturn rk_aiq_set_tool_accm_mode(rk_aiq_sys_ctx_t *ctx, uapi_wb_mode_t *mode) {
    return rk_aiq_uapi2_setCCMMode(ctx, mode->mode);
}

XCamReturn rk_aiq_get_accm_mode(rk_aiq_sys_ctx_t *ctx, uapi_wb_mode_t *mode) {
    return rk_aiq_uapi2_getCCMMode(ctx, &mode->mode);
}

XCamReturn rk_aiq_set_accm_v1_manual_attr(const rk_aiq_sys_ctx_t* sys_ctx,
        rk_aiq_ccm_mccm_attrib_t* manual) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_ccm_attrib_t ccm_attr_v1;
    ret                      = rk_aiq_user_api2_accm_GetAttrib(sys_ctx, &ccm_attr_v1);
    ccm_attr_v1.stManual = *manual;
    ret                      = rk_aiq_user_api2_accm_SetAttrib(sys_ctx, &ccm_attr_v1);

    return ret;
}
XCamReturn rk_aiq_get_accm_v1_manual_attr(const rk_aiq_sys_ctx_t* sys_ctx,
        rk_aiq_ccm_mccm_attrib_t* manual) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_ccm_attrib_t ccm_attr_v1;
    ret     = rk_aiq_user_api2_accm_GetAttrib(sys_ctx, &ccm_attr_v1);
    *manual = ccm_attr_v1.stManual;

    return ret;
}

XCamReturn rk_aiq_set_accm_v2_manual_attr(const rk_aiq_sys_ctx_t* sys_ctx,
        rk_aiq_ccm_mccm_attrib_v2_t* manual) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_ccm_v2_attrib_t ccm_attr_v2;
    ret                      = rk_aiq_user_api2_accm_v2_GetAttrib(sys_ctx, &ccm_attr_v2);
    ccm_attr_v2.stManual = *manual;
    ret                      = rk_aiq_user_api2_accm_v2_SetAttrib(sys_ctx, &ccm_attr_v2);

    return ret;
}
XCamReturn rk_aiq_get_accm_v2_manual_attr(const rk_aiq_sys_ctx_t* sys_ctx,
        rk_aiq_ccm_mccm_attrib_v2_t* manual) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_ccm_v2_attrib_t ccm_attr_v2;
    ret     = rk_aiq_user_api2_accm_v2_GetAttrib(sys_ctx, &ccm_attr_v2);
    *manual = ccm_attr_v2.stManual;

    return ret;
}

XCamReturn rk_aiq_set_tool_3dlut_mode(rk_aiq_sys_ctx_t *ctx, uapi_wb_mode_t *mode) {
    return rk_aiq_uapi2_setLut3dMode(ctx, mode->mode);
}

XCamReturn rk_aiq_get_a3dlut_mode(rk_aiq_sys_ctx_t *ctx, uapi_wb_mode_t *mode) {
    return rk_aiq_uapi2_getLut3dMode(ctx, &mode->mode);
}

XCamReturn rk_aiq_set_a3dlut_manual_attr(const rk_aiq_sys_ctx_t *sys_ctx,
        rk_aiq_lut3d_mlut3d_attrib_t *manual) {
    rk_aiq_lut3d_attrib_t a3dlut_attr;

    memset(&a3dlut_attr, 0, sizeof(rk_aiq_lut3d_attrib_t));
    rk_aiq_user_api2_a3dlut_GetAttrib(sys_ctx, &a3dlut_attr);
    memcpy(&a3dlut_attr.stManual, manual, sizeof(rk_aiq_lut3d_mlut3d_attrib_t));

    return rk_aiq_user_api2_a3dlut_SetAttrib(sys_ctx, &a3dlut_attr);
}

XCamReturn rk_aiq_get_a3dlut_manual_attr(const rk_aiq_sys_ctx_t *sys_ctx,
        rk_aiq_lut3d_mlut3d_attrib_t *manual) {
    rk_aiq_lut3d_attrib_t a3dlut_attr;

    memset(&a3dlut_attr, 0, sizeof(rk_aiq_lut3d_attrib_t));
    rk_aiq_user_api2_a3dlut_GetAttrib(sys_ctx, &a3dlut_attr);
    memcpy(manual, &a3dlut_attr.stManual, sizeof(rk_aiq_lut3d_mlut3d_attrib_t));

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn rk_aiq_set_acgc_manual_attr(const rk_aiq_sys_ctx_t* sys_ctx,
                                       Cgc_Param_t* manual) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    LOGE_ACGC("READY TO SET ATTR\n");

    rk_aiq_uapi_acgc_attrib_t cgc_attr;
    ret = rk_aiq_user_api2_acgc_GetAttrib(sys_ctx, &cgc_attr);
    cgc_attr.param = *manual;
    ret = rk_aiq_user_api2_acgc_SetAttrib(sys_ctx, &cgc_attr);

    return ret;
}
XCamReturn rk_aiq_get_acgc_manual_attr(const rk_aiq_sys_ctx_t* sys_ctx,
                                       Cgc_Param_t* manual) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_acgc_attrib_t cgc_attr;
    ret = rk_aiq_user_api2_acgc_GetAttrib(sys_ctx, &cgc_attr);
    *manual = cgc_attr.param;

    return ret;
}

XCamReturn rk_aiq_set_acsm_manual_attr(const rk_aiq_sys_ctx_t* sys_ctx,
                                       Csm_Param_t* manual) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_acsm_attrib_t csm_attr;
    ret = rk_aiq_user_api2_acsm_GetAttrib(sys_ctx, &csm_attr);
    csm_attr.param = *manual;
    ret = rk_aiq_user_api2_acsm_SetAttrib(sys_ctx, &csm_attr);

    return ret;
}
XCamReturn rk_aiq_get_acsm_manual_attr(const rk_aiq_sys_ctx_t* sys_ctx,
                                       Csm_Param_t* manual) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_acsm_attrib_t csm_attr;
    ret = rk_aiq_user_api2_acsm_GetAttrib(sys_ctx, &csm_attr);
    *manual = csm_attr.param;

    return ret;
}
XCamReturn rk_aiq_set_adebayer_attr(const rk_aiq_sys_ctx_t *sys_ctx,
                                    adebayer_attrib_t *adebayer_attr) {
    adebayer_attrib_t attr;

    memset(&attr, 0, sizeof(attr));
    rk_aiq_user_api2_adebayer_GetAttrib(sys_ctx, &attr);
    memcpy(&attr, adebayer_attr, sizeof(attr));

    return rk_aiq_user_api2_adebayer_SetAttrib(sys_ctx, attr);
}

XCamReturn rk_aiq_get_adebayer_attr(const rk_aiq_sys_ctx_t *sys_ctx,
                                    adebayer_attrib_t *adebayer_attr) {
    adebayer_attrib_t attr;

    memset(&attr, 0, sizeof(attr));
    rk_aiq_user_api2_adebayer_GetAttrib(sys_ctx, &attr);
    memcpy(adebayer_attr, &attr, sizeof(attr));

    return XCAM_RETURN_NO_ERROR;
}


XCamReturn rk_aiq_set_alsc_manual_attr(const rk_aiq_sys_ctx_t *sys_ctx,
                                       rk_aiq_lsc_table_t *manual) {
    rk_aiq_lsc_attrib_t alsc_attr;

    memset(&alsc_attr, 0, sizeof(rk_aiq_lsc_attrib_t));
    rk_aiq_user_api2_alsc_GetAttrib(sys_ctx, &alsc_attr);
    memcpy(&alsc_attr.stManual, manual, sizeof(rk_aiq_lsc_table_t));

    return rk_aiq_user_api2_alsc_SetAttrib(sys_ctx, alsc_attr);
}

XCamReturn rk_aiq_get_alsc_manual_attr(const rk_aiq_sys_ctx_t *sys_ctx,
                                       rk_aiq_lsc_table_t *manual) {
    rk_aiq_lsc_attrib_t alsc_attr;

    memset(&alsc_attr, 0, sizeof(rk_aiq_lsc_attrib_t));
    rk_aiq_user_api2_alsc_GetAttrib(sys_ctx, &alsc_attr);
    memcpy(manual, &alsc_attr.stManual, sizeof(rk_aiq_lsc_table_t));

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn rk_aiq_set_current_camindex(const rk_aiq_sys_ctx_t *sys_ctx,
                                       camgroup_uapi_t *arg) {
    (void)(sys_ctx);
    last_camindex = *arg;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn rk_aiq_get_current_camindex(const rk_aiq_sys_ctx_t *sys_ctx,
                                       camgroup_uapi_t *arg) {
    (void)(sys_ctx);

    *arg = last_camindex;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn rk_aiq_set_ablc_manual_attr(const rk_aiq_sys_ctx_t *sys_ctx, ablc_uapi_manual_t *manual) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#if defined(ISP_HW_V39) || defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
    rk_aiq_blc_attrib_V32_t blc_attr_v32;
    memset(&blc_attr_v32, 0, sizeof(rk_aiq_blc_attrib_V32_t));
    if (manual->AblcOPMode == RK_AIQ_OP_MODE_AUTO)
        blc_attr_v32.eMode = ABLC_V32_OP_MODE_AUTO;
    else if (manual->AblcOPMode == RK_AIQ_OP_MODE_INVALID)
        blc_attr_v32.eMode = ABLC_V32_OP_MODE_OFF;
    else if (manual->AblcOPMode == RK_AIQ_OP_MODE_MANUAL)
        blc_attr_v32.eMode = ABLC_V32_OP_MODE_MANUAL;
    else if (manual->AblcOPMode == RK_AIQ_OP_MODE_MAX)
        blc_attr_v32.eMode = ABLC_V32_OP_MODE_MAX;
    else
        blc_attr_v32.eMode = ABLC_V32_OP_MODE_AUTO;

    memcpy(&blc_attr_v32.stBlc0Manual, &manual->blc0_para, sizeof(blc_attr_v32.stBlc0Manual));
    memcpy(&blc_attr_v32.stBlc1Manual, &manual->blc1_para, sizeof(blc_attr_v32.stBlc1Manual));
    memcpy(&blc_attr_v32.stBlcOBManual, &manual->blc_ob_para, sizeof(blc_attr_v32.stBlcOBManual));

    ret = rk_aiq_user_api2_ablcV32_SetAttrib(sys_ctx, &blc_attr_v32);
#elif defined(ISP_HW_V30)
    rk_aiq_blc_attrib_t ablc_attr;
    memset(&ablc_attr, 0, sizeof(ablc_attr));
    rk_aiq_user_api2_ablc_GetAttrib(sys_ctx, &ablc_attr);
    memcpy(&ablc_attr.stBlc0Manual, &manual->bls0, sizeof(manual->bls0));
    memcpy(&ablc_attr.stBlc1Manual, &manual->bls1, sizeof(manual->bls1));

    if (manual->AblcOPMode == RK_AIQ_OP_MODE_AUTO)
        ablc_attr.eMode = ABLC_OP_MODE_AUTO;
    else if(manual->AblcOPMode == RK_AIQ_OP_MODE_MANUAL)
        ablc_attr.eMode = ABLC_OP_MODE_MANUAL;
    else if(manual->AblcOPMode == RK_AIQ_OP_MODE_INVALID)
        ablc_attr.eMode = ABLC_OP_MODE_OFF;
    else if(manual->AblcOPMode == RK_AIQ_OP_MODE_MAX)
        ablc_attr.eMode = ABLC_OP_MODE_MAX;
    else
        ablc_attr.eMode = ABLC_OP_MODE_AUTO;

    ret = rk_aiq_user_api2_ablc_SetAttrib(sys_ctx, &ablc_attr);
#endif

    return ret;
}

XCamReturn rk_aiq_get_ablc_manual_attr(const rk_aiq_sys_ctx_t *sys_ctx, ablc_uapi_manual_t* manual) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if defined(ISP_HW_V39) || defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
    rk_aiq_blc_attrib_V32_t blc_attr_v32;
    memset(&blc_attr_v32, 0, sizeof(rk_aiq_blc_attrib_V32_t));
    ret = rk_aiq_user_api2_ablcV32_GetAttrib(sys_ctx, &blc_attr_v32);

    if (blc_attr_v32.eMode == ABLC_V32_OP_MODE_AUTO)
        manual->AblcOPMode = RK_AIQ_OP_MODE_AUTO;
    else if (blc_attr_v32.eMode == ABLC_V32_OP_MODE_OFF)
        manual->AblcOPMode = RK_AIQ_OP_MODE_MANUAL;
    else if (blc_attr_v32.eMode == ABLC_V32_OP_MODE_MANUAL)
        manual->AblcOPMode = RK_AIQ_OP_MODE_MANUAL;
    else if (blc_attr_v32.eMode == ABLC_V32_OP_MODE_MAX)
        manual->AblcOPMode = RK_AIQ_OP_MODE_MAX;
    else
        manual->AblcOPMode = RK_AIQ_OP_MODE_AUTO;

    memcpy(&manual->blc0_para, &blc_attr_v32.stBlc0Manual, sizeof(manual->blc0_para));
    memcpy(&manual->blc1_para, &blc_attr_v32.stBlc1Manual, sizeof(manual->blc1_para));
    memcpy(&manual->blc_ob_para, &blc_attr_v32.stBlcOBManual, sizeof(manual->blc_ob_para));
#elif defined(ISP_HW_V30)
    rk_aiq_blc_attrib_t ablc_attr;

    memset(&ablc_attr, 0, sizeof(ablc_attr));
    rk_aiq_user_api2_ablc_GetAttrib(sys_ctx, &ablc_attr);
    memcpy(&manual->bls0, &ablc_attr.stBlc0Manual, sizeof(manual->bls0));
    memcpy(&manual->bls1, &ablc_attr.stBlc1Manual, sizeof(manual->bls1));

    if (ablc_attr.eMode == ABLC_OP_MODE_AUTO)
        manual->AblcOPMode = RK_AIQ_OP_MODE_AUTO;
    else if(ablc_attr.eMode == ABLC_OP_MODE_MANUAL)
        manual->AblcOPMode = RK_AIQ_OP_MODE_MANUAL;
    else if(ablc_attr.eMode == ABLC_OP_MODE_OFF )
        manual->AblcOPMode = RK_AIQ_OP_MODE_INVALID;
    else if(ablc_attr.eMode == ABLC_OP_MODE_MAX)
        manual->AblcOPMode = RK_AIQ_OP_MODE_MAX;
    else
        manual->AblcOPMode = RK_AIQ_OP_MODE_AUTO;
#endif

    return ret;
}

XCamReturn rk_aiq_set_asharp_manual_attr(const rk_aiq_sys_ctx_t *sys_ctx,
        asharp_uapi_manual_t *manual) {
    XCamReturn res = XCAM_RETURN_NO_ERROR;

#if defined(ISP_HW_V39)
    rk_aiq_sharp_attrib_v34_t sharp_attr_v34;
    memset(&sharp_attr_v34, 0, sizeof(rk_aiq_sharp_attrib_v34_t));
    rk_aiq_user_api2_asharpV34_GetAttrib(sys_ctx, &sharp_attr_v34);
    memcpy(&sharp_attr_v34.stManual.stSelect, &manual->manual_v34, sizeof(RK_SHARP_Params_V34_Select_t));

    if (manual->AsharpOpMode == RK_AIQ_OP_MODE_AUTO)
        sharp_attr_v34.eMode = ASHARP_V34_OP_MODE_AUTO;
    else if(manual->AsharpOpMode == RK_AIQ_OP_MODE_MANUAL)
        sharp_attr_v34.eMode = ASHARP_V34_OP_MODE_MANUAL;
    else if(manual->AsharpOpMode == RK_AIQ_OP_MODE_INVALID)
        sharp_attr_v34.eMode = ASHARP_V34_OP_MODE_INVALID;
    else if(manual->AsharpOpMode == RK_AIQ_OP_MODE_MAX)
        sharp_attr_v34.eMode = ASHARP_V34_OP_MODE_MAX;
    else
        sharp_attr_v34.eMode = ASHARP_V34_OP_MODE_AUTO;

    res = rk_aiq_user_api2_asharpV34_SetAttrib(sys_ctx, &sharp_attr_v34);

#elif defined(ISP_HW_V32)
    rk_aiq_sharp_attrib_v33_t sharp_attr_v33;
    memset(&sharp_attr_v33, 0, sizeof(rk_aiq_sharp_attrib_v33_t));
    rk_aiq_user_api2_asharpV33_GetAttrib(sys_ctx, &sharp_attr_v33);
    memcpy(&sharp_attr_v33.stManual.stSelect, &manual->manual_v33, sizeof(RK_SHARP_Params_V33_Select_t));

    if (manual->AsharpOpMode == RK_AIQ_OP_MODE_AUTO)
        sharp_attr_v33.eMode = ASHARP_V33_OP_MODE_AUTO;
    else if(manual->AsharpOpMode == RK_AIQ_OP_MODE_MANUAL)
        sharp_attr_v33.eMode = ASHARP_V33_OP_MODE_MANUAL;
    else if(manual->AsharpOpMode == RK_AIQ_OP_MODE_INVALID)
        sharp_attr_v33.eMode = ASHARP_V33_OP_MODE_INVALID;
    else if(manual->AsharpOpMode == RK_AIQ_OP_MODE_MAX)
        sharp_attr_v33.eMode = ASHARP_V33_OP_MODE_MAX;
    else
        sharp_attr_v33.eMode = ASHARP_V33_OP_MODE_AUTO;

    res = rk_aiq_user_api2_asharpV33_SetAttrib(sys_ctx, &sharp_attr_v33);
#elif defined(ISP_HW_V32_LITE)
    rk_aiq_sharp_attrib_v33LT_t sharp_attr_v33_lt;
    memset(&sharp_attr_v33_lt, 0, sizeof(rk_aiq_sharp_attrib_v33LT_t));
    rk_aiq_user_api2_asharpV33Lite_GetAttrib(sys_ctx, &sharp_attr_v33_lt);
    memcpy(&sharp_attr_v33_lt.stManual.stSelect, &manual->manual_v33Lite,
           sizeof(RK_SHARP_Params_V33LT_Select_t));

    if (manual->AsharpOpMode == RK_AIQ_OP_MODE_AUTO)
        sharp_attr_v33_lt.eMode = ASHARP_V33_OP_MODE_AUTO;
    else if (manual->AsharpOpMode == RK_AIQ_OP_MODE_MANUAL)
        sharp_attr_v33_lt.eMode = ASHARP_V33_OP_MODE_MANUAL;
    else if (manual->AsharpOpMode == RK_AIQ_OP_MODE_INVALID)
        sharp_attr_v33_lt.eMode = ASHARP_V33_OP_MODE_INVALID;
    else if (manual->AsharpOpMode == RK_AIQ_OP_MODE_MAX)
        sharp_attr_v33_lt.eMode = ASHARP_V33_OP_MODE_MAX;
    else
        sharp_attr_v33_lt.eMode = ASHARP_V33_OP_MODE_AUTO;

    res = rk_aiq_user_api2_asharpV33Lite_SetAttrib(sys_ctx, &sharp_attr_v33_lt);
#elif defined(ISP_HW_V30)
    rk_aiq_sharp_attrib_v4_t sharp_attr_v4;
    memset(&sharp_attr_v4, 0, sizeof(rk_aiq_sharp_attrib_v4_t));
    rk_aiq_user_api2_asharpV4_GetAttrib(sys_ctx, &sharp_attr_v4);
    memcpy(&sharp_attr_v4.stManual.stSelect, &manual->manual_v4, sizeof(RK_SHARP_Params_V4_Select_t));

    if (manual->AsharpOpMode == RK_AIQ_OP_MODE_AUTO)
        sharp_attr_v4.eMode = ASHARP4_OP_MODE_AUTO;
    else if(manual->AsharpOpMode == RK_AIQ_OP_MODE_MANUAL)
        sharp_attr_v4.eMode = ASHARP4_OP_MODE_MANUAL;
    else if(manual->AsharpOpMode == RK_AIQ_OP_MODE_INVALID)
        sharp_attr_v4.eMode = ASHARP4_OP_MODE_INVALID;
    else if(manual->AsharpOpMode == RK_AIQ_OP_MODE_MAX)
        sharp_attr_v4.eMode = ASHARP4_OP_MODE_MAX;
    else
        sharp_attr_v4.eMode = ASHARP4_OP_MODE_AUTO;

    res = rk_aiq_user_api2_asharpV4_SetAttrib(sys_ctx, &sharp_attr_v4);
#endif

    return res;
}

XCamReturn rk_aiq_get_asharp_manual_attr(const rk_aiq_sys_ctx_t *sys_ctx,
        asharp_uapi_manual_t *manual) {
#if defined(ISP_HW_V39)
    rk_aiq_sharp_attrib_v34_t sharp_attr_v34;
    memset(&sharp_attr_v34, 0, sizeof(rk_aiq_sharp_attrib_v34_t));
    rk_aiq_user_api2_asharpV34_GetAttrib(sys_ctx, &sharp_attr_v34);
    memcpy(&manual->manual_v34, &sharp_attr_v34.stManual.stSelect, sizeof(RK_SHARP_Params_V34_Select_t));

    if (sharp_attr_v34.eMode == ASHARP_V34_OP_MODE_AUTO)
        manual->AsharpOpMode = RK_AIQ_OP_MODE_AUTO;
    else if(sharp_attr_v34.eMode == ASHARP_V34_OP_MODE_MANUAL)
        manual->AsharpOpMode = RK_AIQ_OP_MODE_MANUAL;
    else if(sharp_attr_v34.eMode == ASHARP_V34_OP_MODE_INVALID )
        manual->AsharpOpMode = RK_AIQ_OP_MODE_INVALID;
    else if(sharp_attr_v34.eMode == ASHARP_V34_OP_MODE_MAX)
        manual->AsharpOpMode = RK_AIQ_OP_MODE_MAX;
    else
        manual->AsharpOpMode = RK_AIQ_OP_MODE_AUTO;

#elif defined(ISP_HW_V32)
    rk_aiq_sharp_attrib_v33_t sharp_attr_v33;
    memset(&sharp_attr_v33, 0, sizeof(rk_aiq_sharp_attrib_v33_t));
    rk_aiq_user_api2_asharpV33_GetAttrib(sys_ctx, &sharp_attr_v33);
    memcpy(&manual->manual_v33, &sharp_attr_v33.stManual.stSelect, sizeof(RK_SHARP_Params_V33_Select_t));

    if (sharp_attr_v33.eMode == ASHARP_V33_OP_MODE_AUTO)
        manual->AsharpOpMode = RK_AIQ_OP_MODE_AUTO;
    else if(sharp_attr_v33.eMode == ASHARP_V33_OP_MODE_MANUAL)
        manual->AsharpOpMode = RK_AIQ_OP_MODE_MANUAL;
    else if(sharp_attr_v33.eMode == ASHARP_V33_OP_MODE_INVALID )
        manual->AsharpOpMode = RK_AIQ_OP_MODE_INVALID;
    else if(sharp_attr_v33.eMode == ASHARP_V33_OP_MODE_MAX)
        manual->AsharpOpMode = RK_AIQ_OP_MODE_MAX;
    else
        manual->AsharpOpMode = RK_AIQ_OP_MODE_AUTO;
#elif defined(ISP_HW_V32_LITE)
    rk_aiq_sharp_attrib_v33LT_t sharp_attr_v33LT;
    memset(&sharp_attr_v33LT, 0, sizeof(rk_aiq_sharp_attrib_v33LT_t));
    rk_aiq_user_api2_asharpV33Lite_GetAttrib(sys_ctx, &sharp_attr_v33LT);
    memcpy(&manual->manual_v33Lite, &sharp_attr_v33LT.stManual.stSelect,
           sizeof(RK_SHARP_Params_V33LT_Select_t));

    if (sharp_attr_v33LT.eMode == ASHARP_V33_OP_MODE_AUTO)
        manual->AsharpOpMode = RK_AIQ_OP_MODE_AUTO;
    else if (sharp_attr_v33LT.eMode == ASHARP_V33_OP_MODE_MANUAL)
        manual->AsharpOpMode = RK_AIQ_OP_MODE_MANUAL;
    else if (sharp_attr_v33LT.eMode == ASHARP_V33_OP_MODE_INVALID)
        manual->AsharpOpMode = RK_AIQ_OP_MODE_INVALID;
    else if (sharp_attr_v33LT.eMode == ASHARP_V33_OP_MODE_MAX)
        manual->AsharpOpMode = RK_AIQ_OP_MODE_MAX;
    else
        manual->AsharpOpMode = RK_AIQ_OP_MODE_AUTO;
#elif defined(ISP_HW_V30)
    rk_aiq_sharp_attrib_v4_t sharp_attr_v4;
    memset(&sharp_attr_v4, 0, sizeof(rk_aiq_sharp_attrib_v4_t));
    rk_aiq_user_api2_asharpV4_GetAttrib(sys_ctx, &sharp_attr_v4);
    memcpy(&manual->manual_v4, &sharp_attr_v4.stManual.stSelect, sizeof(RK_SHARP_Params_V4_Select_t));

    if (sharp_attr_v4.eMode == ASHARP4_OP_MODE_AUTO)
        manual->AsharpOpMode = RK_AIQ_OP_MODE_AUTO;
    else if(sharp_attr_v4.eMode == ASHARP4_OP_MODE_MANUAL)
        manual->AsharpOpMode = RK_AIQ_OP_MODE_MANUAL;
    else if(sharp_attr_v4.eMode == ASHARP4_OP_MODE_INVALID )
        manual->AsharpOpMode = RK_AIQ_OP_MODE_INVALID;
    else if(sharp_attr_v4.eMode == ASHARP4_OP_MODE_MAX)
        manual->AsharpOpMode = RK_AIQ_OP_MODE_MAX;
    else
        manual->AsharpOpMode = RK_AIQ_OP_MODE_AUTO;

#endif
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_set_abayer2dnr_manual_attr(const rk_aiq_sys_ctx_t *sys_ctx,
                                  abayer2dnr_uapi_manual_t *manual) {
    XCamReturn res = XCAM_RETURN_NO_ERROR;

#if  defined(ISP_HW_V32)
    rk_aiq_bayer2dnr_attrib_v23_t abayer2dnr_attr_v23;

    memset(&abayer2dnr_attr_v23, 0, sizeof(rk_aiq_bayer2dnr_attrib_v23_t));
    rk_aiq_user_api2_abayer2dnrV23_GetAttrib(sys_ctx, &abayer2dnr_attr_v23);
    memcpy(&abayer2dnr_attr_v23.stManual.st2DSelect, &manual->manual_v23,
           sizeof(RK_Bayer2dnrV23_Params_Select_t));

    if (manual->Abayer2dnrOpMode == RK_AIQ_OP_MODE_AUTO)
        abayer2dnr_attr_v23.eMode = ABAYER2DNR_V23_OP_MODE_AUTO;
    else if(manual->Abayer2dnrOpMode == RK_AIQ_OP_MODE_MANUAL)
        abayer2dnr_attr_v23.eMode = ABAYER2DNR_V23_OP_MODE_MANUAL;
    else if(manual->Abayer2dnrOpMode == RK_AIQ_OP_MODE_INVALID)
        abayer2dnr_attr_v23.eMode = ABAYER2DNR_V23_OP_MODE_INVALID;
    else if(manual->Abayer2dnrOpMode == RK_AIQ_OP_MODE_MAX)
        abayer2dnr_attr_v23.eMode = ABAYER2DNR_V23_OP_MODE_MAX;
    else
        abayer2dnr_attr_v23.eMode = ABAYER2DNR_V23_OP_MODE_AUTO;


    res = rk_aiq_user_api2_abayer2dnrV23_SetAttrib(sys_ctx, &abayer2dnr_attr_v23);
#elif defined(ISP_HW_V30)
    rk_aiq_bayer2dnr_attrib_v2_t abayer2dnr_attr_v2;

    memset(&abayer2dnr_attr_v2, 0, sizeof(rk_aiq_bayer2dnr_attrib_v2_t));
    rk_aiq_user_api2_abayer2dnrV2_GetAttrib(sys_ctx, &abayer2dnr_attr_v2);
    memcpy(&abayer2dnr_attr_v2.stManual.st2DSelect, &manual->manual_v2,
           sizeof(RK_Bayer2dnr_Params_V2_Select_t));

    if (manual->Abayer2dnrOpMode == RK_AIQ_OP_MODE_AUTO)
        abayer2dnr_attr_v2.eMode = ABAYER2DNR_OP_MODE_AUTO;
    else if(manual->Abayer2dnrOpMode == RK_AIQ_OP_MODE_MANUAL)
        abayer2dnr_attr_v2.eMode = ABAYER2DNR_OP_MODE_MANUAL;
    else if(manual->Abayer2dnrOpMode == RK_AIQ_OP_MODE_INVALID)
        abayer2dnr_attr_v2.eMode = ABAYER2DNR_OP_MODE_INVALID;
    else if(manual->Abayer2dnrOpMode == RK_AIQ_OP_MODE_MAX)
        abayer2dnr_attr_v2.eMode = ABAYER2DNR_OP_MODE_MAX;
    else
        abayer2dnr_attr_v2.eMode = ABAYER2DNR_OP_MODE_AUTO;


    res = rk_aiq_user_api2_abayer2dnrV2_SetAttrib(sys_ctx, &abayer2dnr_attr_v2);
#endif

    return res;
}

XCamReturn
rk_aiq_get_abayer2dnr_manual_attr(const rk_aiq_sys_ctx_t *sys_ctx,
                                  abayer2dnr_uapi_manual_t *manual) {
#if defined(ISP_HW_V32)
    rk_aiq_bayer2dnr_attrib_v23_t abayer2dnr_attr_v23;
    memset(&abayer2dnr_attr_v23, 0, sizeof(rk_aiq_bayer2dnr_attrib_v23_t));
    rk_aiq_user_api2_abayer2dnrV23_GetAttrib(sys_ctx, &abayer2dnr_attr_v23);
    memcpy(&manual->manual_v23, &abayer2dnr_attr_v23.stManual.st2DSelect,
           sizeof(RK_Bayer2dnrV23_Params_Select_t));

    if (abayer2dnr_attr_v23.eMode == ABAYER2DNR_V23_OP_MODE_AUTO)
        manual->Abayer2dnrOpMode = RK_AIQ_OP_MODE_AUTO;
    else if(abayer2dnr_attr_v23.eMode == ABAYER2DNR_V23_OP_MODE_MANUAL)
        manual->Abayer2dnrOpMode = RK_AIQ_OP_MODE_MANUAL;
    else if(abayer2dnr_attr_v23.eMode == ABAYER2DNR_V23_OP_MODE_INVALID )
        manual->Abayer2dnrOpMode = RK_AIQ_OP_MODE_INVALID;
    else if(abayer2dnr_attr_v23.eMode == ABAYER2DNR_V23_OP_MODE_MAX)
        manual->Abayer2dnrOpMode = RK_AIQ_OP_MODE_MAX;
    else
        manual->Abayer2dnrOpMode = RK_AIQ_OP_MODE_AUTO;
#elif defined(ISP_HW_V30)
    rk_aiq_bayer2dnr_attrib_v2_t abayer2dnr_attr_v2;
    memset(&abayer2dnr_attr_v2, 0, sizeof(rk_aiq_bayer2dnr_attrib_v2_t));
    rk_aiq_user_api2_abayer2dnrV2_GetAttrib(sys_ctx, &abayer2dnr_attr_v2);
    memcpy(&manual->manual_v2, &abayer2dnr_attr_v2.stManual.st2DSelect,
           sizeof(RK_Bayer2dnr_Params_V2_Select_t));

    if (abayer2dnr_attr_v2.eMode == ABAYER2DNR_OP_MODE_AUTO)
        manual->Abayer2dnrOpMode = RK_AIQ_OP_MODE_AUTO;
    else if(abayer2dnr_attr_v2.eMode == ABAYER2DNR_OP_MODE_MANUAL)
        manual->Abayer2dnrOpMode = RK_AIQ_OP_MODE_MANUAL;
    else if(abayer2dnr_attr_v2.eMode == ABAYER2DNR_OP_MODE_INVALID )
        manual->Abayer2dnrOpMode = RK_AIQ_OP_MODE_INVALID;
    else if(abayer2dnr_attr_v2.eMode == ABAYER2DNR_OP_MODE_MAX)
        manual->Abayer2dnrOpMode = RK_AIQ_OP_MODE_MAX;
    else
        manual->Abayer2dnrOpMode = RK_AIQ_OP_MODE_AUTO;
#endif
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_set_abayertnr_manual_attr(const rk_aiq_sys_ctx_t *sys_ctx,
                                 abayertnr_uapi_manual_t *manual) {
    XCamReturn res = XCAM_RETURN_NO_ERROR;

#if defined(ISP_HW_V39)
    rk_aiq_bayertnr_attrib_v30_t abayertnr_attr_v30;
    memset(&abayertnr_attr_v30, 0, sizeof(rk_aiq_bayertnr_attrib_v30_t));
    rk_aiq_user_api2_abayertnrV30_GetAttrib(sys_ctx, &abayertnr_attr_v30);
    memcpy(&abayertnr_attr_v30.stManual.st3DSelect, &manual->manual_v30,
           sizeof(RK_Bayertnr_Params_V30_Select_t));

    if (manual->AbayertnrOpMode == RK_AIQ_OP_MODE_AUTO)
        abayertnr_attr_v30.eMode = ABAYERTNRV30_OP_MODE_AUTO;
    else if(manual->AbayertnrOpMode == RK_AIQ_OP_MODE_MANUAL)
        abayertnr_attr_v30.eMode = ABAYERTNRV30_OP_MODE_MANUAL;
    else if(manual->AbayertnrOpMode == RK_AIQ_OP_MODE_INVALID)
        abayertnr_attr_v30.eMode = ABAYERTNRV30_OP_MODE_INVALID;
    else if(manual->AbayertnrOpMode == RK_AIQ_OP_MODE_MAX)
        abayertnr_attr_v30.eMode = ABAYERTNRV30_OP_MODE_MAX;
    else
        abayertnr_attr_v30.eMode = ABAYERTNRV30_OP_MODE_AUTO;

    res = rk_aiq_user_api2_abayertnrV30_SetAttrib(sys_ctx, &abayertnr_attr_v30);

#endif

#if defined(ISP_HW_V32)
    rk_aiq_bayertnr_attrib_v23_t abayertnr_attr_v23;
    memset(&abayertnr_attr_v23, 0, sizeof(rk_aiq_bayertnr_attrib_v23_t));
    rk_aiq_user_api2_abayertnrV23_GetAttrib(sys_ctx, &abayertnr_attr_v23);
    memcpy(&abayertnr_attr_v23.stManual.st3DSelect, &manual->manual_v23,
           sizeof(RK_Bayertnr_Params_V23_Select_t));

    if (manual->AbayertnrOpMode == RK_AIQ_OP_MODE_AUTO)
        abayertnr_attr_v23.eMode = ABAYERTNRV23_OP_MODE_AUTO;
    else if(manual->AbayertnrOpMode == RK_AIQ_OP_MODE_MANUAL)
        abayertnr_attr_v23.eMode = ABAYERTNRV23_OP_MODE_MANUAL;
    else if(manual->AbayertnrOpMode == RK_AIQ_OP_MODE_INVALID)
        abayertnr_attr_v23.eMode = ABAYERTNRV23_OP_MODE_INVALID;
    else if(manual->AbayertnrOpMode == RK_AIQ_OP_MODE_MAX)
        abayertnr_attr_v23.eMode = ABAYERTNRV23_OP_MODE_MAX;
    else
        abayertnr_attr_v23.eMode = ABAYERTNRV23_OP_MODE_AUTO;

    res = rk_aiq_user_api2_abayertnrV23_SetAttrib(sys_ctx, &abayertnr_attr_v23);
#endif

#if defined(ISP_HW_V32_LITE)
    rk_aiq_bayertnr_attrib_v23L_t abayertnr_attr_v23L;
    memset(&abayertnr_attr_v23L, 0, sizeof(rk_aiq_bayertnr_attrib_v23L_t));
    rk_aiq_user_api2_abayertnrV23Lite_GetAttrib(sys_ctx, &abayertnr_attr_v23L);
    memcpy(&abayertnr_attr_v23L.stManual.st3DSelect, &manual->manual_v23L,
           sizeof(RK_Bayertnr_Param_V23L_Select_t));

    if (manual->AbayertnrOpMode == RK_AIQ_OP_MODE_AUTO)
        abayertnr_attr_v23L.eMode = ABAYERTNRV23_OP_MODE_AUTO;
    else if (manual->AbayertnrOpMode == RK_AIQ_OP_MODE_MANUAL)
        abayertnr_attr_v23L.eMode = ABAYERTNRV23_OP_MODE_MANUAL;
    else if (manual->AbayertnrOpMode == RK_AIQ_OP_MODE_INVALID)
        abayertnr_attr_v23L.eMode = ABAYERTNRV23_OP_MODE_INVALID;
    else if (manual->AbayertnrOpMode == RK_AIQ_OP_MODE_MAX)
        abayertnr_attr_v23L.eMode = ABAYERTNRV23_OP_MODE_MAX;
    else
        abayertnr_attr_v23L.eMode = ABAYERTNRV23_OP_MODE_AUTO;

    res = rk_aiq_user_api2_abayertnrV23Lite_SetAttrib(sys_ctx, &abayertnr_attr_v23L);

#endif

#if defined(ISP_HW_V30)
    rk_aiq_bayertnr_attrib_v2_t abayertnr_attr_v2;
    memset(&abayertnr_attr_v2, 0, sizeof(abayertnr_attr_v2));
    rk_aiq_user_api2_abayertnrV2_GetAttrib(sys_ctx, &abayertnr_attr_v2);
    memcpy(&abayertnr_attr_v2.stManual.st3DSelect, &manual->manual_v2,
           sizeof(manual->manual_v2));

    if (manual->AbayertnrOpMode == RK_AIQ_OP_MODE_AUTO)
        abayertnr_attr_v2.eMode = ABAYERTNRV2_OP_MODE_AUTO;
    else if(manual->AbayertnrOpMode == RK_AIQ_OP_MODE_MANUAL)
        abayertnr_attr_v2.eMode = ABAYERTNRV2_OP_MODE_MANUAL;
    else if(manual->AbayertnrOpMode == RK_AIQ_OP_MODE_INVALID)
        abayertnr_attr_v2.eMode = ABAYERTNRV2_OP_MODE_INVALID;
    else if(manual->AbayertnrOpMode == RK_AIQ_OP_MODE_MAX)
        abayertnr_attr_v2.eMode = ABAYERTNRV2_OP_MODE_MAX;
    else
        abayertnr_attr_v2.eMode = ABAYERTNRV2_OP_MODE_AUTO;

    res = rk_aiq_user_api2_abayertnrV2_SetAttrib(sys_ctx, &abayertnr_attr_v2);
#endif

    return res;
}

XCamReturn
rk_aiq_get_abayertnr_manual_attr(const rk_aiq_sys_ctx_t *sys_ctx,
                                 abayertnr_uapi_manual_t *manual) {

#if defined(ISP_HW_V39)
    rk_aiq_bayertnr_attrib_v30_t abayertnr_attr;
    memset(&abayertnr_attr, 0, sizeof(rk_aiq_bayertnr_attrib_v30_t));
    rk_aiq_user_api2_abayertnrV30_GetAttrib(sys_ctx, &abayertnr_attr);
    memcpy(&manual->manual_v30, &abayertnr_attr.stManual.st3DSelect,
           sizeof(RK_Bayertnr_Params_V30_Select_t));

    if (abayertnr_attr.eMode == ABAYERTNRV30_OP_MODE_AUTO)
        manual->AbayertnrOpMode = RK_AIQ_OP_MODE_AUTO;
    else if(abayertnr_attr.eMode == ABAYERTNRV30_OP_MODE_MANUAL)
        manual->AbayertnrOpMode = RK_AIQ_OP_MODE_MANUAL;
    else if(abayertnr_attr.eMode == ABAYERTNRV30_OP_MODE_INVALID )
        manual->AbayertnrOpMode = RK_AIQ_OP_MODE_INVALID;
    else if(abayertnr_attr.eMode == ABAYERTNRV30_OP_MODE_MAX)
        manual->AbayertnrOpMode = RK_AIQ_OP_MODE_MAX;
    else
        manual->AbayertnrOpMode = RK_AIQ_OP_MODE_AUTO;


#elif defined(ISP_HW_V32)
    rk_aiq_bayertnr_attrib_v23_t abayertnr_attr;
    memset(&abayertnr_attr, 0, sizeof(rk_aiq_bayertnr_attrib_v23_t));
    rk_aiq_user_api2_abayertnrV23_GetAttrib(sys_ctx, &abayertnr_attr);
    memcpy(&manual->manual_v23, &abayertnr_attr.stManual.st3DSelect,
           sizeof(RK_Bayertnr_Params_V23_Select_t));

    if (abayertnr_attr.eMode == ABAYERTNRV23_OP_MODE_AUTO)
        manual->AbayertnrOpMode = RK_AIQ_OP_MODE_AUTO;
    else if(abayertnr_attr.eMode == ABAYERTNRV23_OP_MODE_MANUAL)
        manual->AbayertnrOpMode = RK_AIQ_OP_MODE_MANUAL;
    else if(abayertnr_attr.eMode == ABAYERTNRV23_OP_MODE_INVALID )
        manual->AbayertnrOpMode = RK_AIQ_OP_MODE_INVALID;
    else if(abayertnr_attr.eMode == ABAYERTNRV23_OP_MODE_MAX)
        manual->AbayertnrOpMode = RK_AIQ_OP_MODE_MAX;
    else
        manual->AbayertnrOpMode = RK_AIQ_OP_MODE_AUTO;

#elif defined(ISP_HW_V32_LITE)
    rk_aiq_bayertnr_attrib_v23L_t abayertnr_attr_v23L;
    memset(&abayertnr_attr_v23L, 0, sizeof(rk_aiq_bayertnr_attrib_v23L_t));
    rk_aiq_user_api2_abayertnrV23Lite_GetAttrib(sys_ctx, &abayertnr_attr_v23L);
    memcpy(&manual->manual_v23L, &abayertnr_attr_v23L.stManual.st3DSelect,
           sizeof(RK_Bayertnr_Param_V23L_Select_t));

    if (abayertnr_attr_v23L.eMode == ABAYERTNRV23_OP_MODE_AUTO)
        manual->AbayertnrOpMode = RK_AIQ_OP_MODE_AUTO;
    else if (abayertnr_attr_v23L.eMode == ABAYERTNRV23_OP_MODE_MANUAL)
        manual->AbayertnrOpMode = RK_AIQ_OP_MODE_MANUAL;
    else if (abayertnr_attr_v23L.eMode == ABAYERTNRV23_OP_MODE_INVALID)
        manual->AbayertnrOpMode = RK_AIQ_OP_MODE_INVALID;
    else if (abayertnr_attr_v23L.eMode == ABAYERTNRV23_OP_MODE_MAX)
        manual->AbayertnrOpMode = RK_AIQ_OP_MODE_MAX;
    else
        manual->AbayertnrOpMode = RK_AIQ_OP_MODE_AUTO;

#elif defined(ISP_HW_V30)
    rk_aiq_bayertnr_attrib_v2_t abayertnr_attr_v2;
    memset(&abayertnr_attr_v2, 0, sizeof(abayertnr_attr_v2));
    rk_aiq_user_api2_abayertnrV2_GetAttrib(sys_ctx, &abayertnr_attr_v2);
    memcpy(&manual->manual_v2, &abayertnr_attr_v2.stManual.st3DSelect,
           sizeof(manual->manual_v2));

    if (abayertnr_attr_v2.eMode == ABAYERTNRV2_OP_MODE_AUTO)
        manual->AbayertnrOpMode = RK_AIQ_OP_MODE_AUTO;
    else if(abayertnr_attr_v2.eMode == ABAYERTNRV2_OP_MODE_MANUAL)
        manual->AbayertnrOpMode = RK_AIQ_OP_MODE_MANUAL;
    else if(abayertnr_attr_v2.eMode == ABAYERTNRV2_OP_MODE_INVALID )
        manual->AbayertnrOpMode = RK_AIQ_OP_MODE_INVALID;
    else if(abayertnr_attr_v2.eMode == ABAYERTNRV2_OP_MODE_MAX)
        manual->AbayertnrOpMode = RK_AIQ_OP_MODE_MAX;
    else
        manual->AbayertnrOpMode = RK_AIQ_OP_MODE_AUTO;
#endif
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn rk_aiq_set_aynr_manual_attr(const rk_aiq_sys_ctx_t *sys_ctx,
                                       aynr_uapi_manual_t *manual) {
    XCamReturn res = XCAM_RETURN_NO_ERROR;

#if defined(ISP_HW_V39)
    rk_aiq_ynr_attrib_v24_t aynr_attr_v24;
    memset(&aynr_attr_v24, 0, sizeof(rk_aiq_ynr_attrib_v24_t));
    rk_aiq_user_api2_aynrV24_GetAttrib(sys_ctx, &aynr_attr_v24);
    memcpy(&aynr_attr_v24.stManual.stSelect, &manual->manual_v24, sizeof(RK_YNR_Params_V24_Select_t));

    if (manual->AynrOpMode == RK_AIQ_OP_MODE_AUTO)
        aynr_attr_v24.eMode = AYNRV24_OP_MODE_AUTO;
    else if(manual->AynrOpMode == RK_AIQ_OP_MODE_MANUAL)
        aynr_attr_v24.eMode = AYNRV24_OP_MODE_MANUAL;
    else if(manual->AynrOpMode == RK_AIQ_OP_MODE_INVALID)
        aynr_attr_v24.eMode = AYNRV24_OP_MODE_INVALID;
    else if(manual->AynrOpMode == RK_AIQ_OP_MODE_MAX)
        aynr_attr_v24.eMode = AYNRV24_OP_MODE_MAX;

    res = rk_aiq_user_api2_aynrV24_SetAttrib(sys_ctx, &aynr_attr_v24);
#elif defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
    rk_aiq_ynr_attrib_v22_t aynr_attr_v22;
    memset(&aynr_attr_v22, 0, sizeof(rk_aiq_ynr_attrib_v22_t));
    rk_aiq_user_api2_aynrV22_GetAttrib(sys_ctx, &aynr_attr_v22);
    memcpy(&aynr_attr_v22.stManual.stSelect, &manual->manual_v22, sizeof(RK_YNR_Params_V22_Select_t));

    if (manual->AynrOpMode == RK_AIQ_OP_MODE_AUTO)
        aynr_attr_v22.eMode = AYNRV22_OP_MODE_AUTO;
    else if(manual->AynrOpMode == RK_AIQ_OP_MODE_MANUAL)
        aynr_attr_v22.eMode = AYNRV22_OP_MODE_MANUAL;
    else if(manual->AynrOpMode == RK_AIQ_OP_MODE_INVALID)
        aynr_attr_v22.eMode = AYNRV22_OP_MODE_INVALID;
    else if(manual->AynrOpMode == RK_AIQ_OP_MODE_MAX)
        aynr_attr_v22.eMode = AYNRV22_OP_MODE_MAX;

    res = rk_aiq_user_api2_aynrV22_SetAttrib(sys_ctx, &aynr_attr_v22);
#elif defined(ISP_HW_V30)
    rk_aiq_ynr_attrib_v3_t aynr_attr_v3;
    memset(&aynr_attr_v3, 0, sizeof(aynr_attr_v3));
    rk_aiq_user_api2_aynrV3_GetAttrib(sys_ctx, &aynr_attr_v3);
    memcpy(&aynr_attr_v3.stManual.stSelect, &manual->manual_v3, sizeof(manual->manual_v3));

    if (manual->AynrOpMode == RK_AIQ_OP_MODE_AUTO)
        aynr_attr_v3.eMode = AYNRV3_OP_MODE_AUTO;
    else if(manual->AynrOpMode == RK_AIQ_OP_MODE_MANUAL)
        aynr_attr_v3.eMode = AYNRV3_OP_MODE_MANUAL;
    else if(manual->AynrOpMode == RK_AIQ_OP_MODE_INVALID)
        aynr_attr_v3.eMode = AYNRV3_OP_MODE_INVALID;
    else if(manual->AynrOpMode == RK_AIQ_OP_MODE_MAX)
        aynr_attr_v3.eMode = AYNRV3_OP_MODE_MAX;

    res = rk_aiq_user_api2_aynrV3_SetAttrib(sys_ctx, &aynr_attr_v3);
#endif

    return res;
}

XCamReturn rk_aiq_get_aynr_manual_attr(const rk_aiq_sys_ctx_t *sys_ctx,
                                       aynr_uapi_manual_t *manual) {
#if defined(ISP_HW_V39)
    rk_aiq_ynr_attrib_v24_t aynr_attr_v24;
    memset(&aynr_attr_v24, 0, sizeof(rk_aiq_ynr_attrib_v24_t));
    rk_aiq_user_api2_aynrV24_GetAttrib(sys_ctx, &aynr_attr_v24);
    memcpy(&manual->manual_v24, &aynr_attr_v24.stManual.stSelect, sizeof(RK_YNR_Params_V24_Select_t));

    if (aynr_attr_v24.eMode == AYNRV24_OP_MODE_AUTO)
        manual->AynrOpMode = RK_AIQ_OP_MODE_AUTO;
    else if(aynr_attr_v24.eMode == AYNRV24_OP_MODE_MANUAL)
        manual->AynrOpMode = RK_AIQ_OP_MODE_MANUAL;
    else if(aynr_attr_v24.eMode == AYNRV24_OP_MODE_INVALID )
        manual->AynrOpMode = RK_AIQ_OP_MODE_INVALID;
    else if(aynr_attr_v24.eMode == AYNRV24_OP_MODE_MAX)
        manual->AynrOpMode = RK_AIQ_OP_MODE_MAX;
    else
        manual->AynrOpMode = RK_AIQ_OP_MODE_AUTO;

#elif defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
    rk_aiq_ynr_attrib_v22_t aynr_attr_v22;

    memset(&aynr_attr_v22, 0, sizeof(rk_aiq_ynr_attrib_v22_t));
    rk_aiq_user_api2_aynrV22_GetAttrib(sys_ctx, &aynr_attr_v22);
    memcpy(&manual->manual_v22, &aynr_attr_v22.stManual.stSelect, sizeof(RK_YNR_Params_V22_Select_t));

    if (aynr_attr_v22.eMode == AYNRV22_OP_MODE_AUTO)
        manual->AynrOpMode = RK_AIQ_OP_MODE_AUTO;
    else if(aynr_attr_v22.eMode == AYNRV22_OP_MODE_MANUAL)
        manual->AynrOpMode = RK_AIQ_OP_MODE_MANUAL;
    else if(aynr_attr_v22.eMode == AYNRV22_OP_MODE_INVALID )
        manual->AynrOpMode = RK_AIQ_OP_MODE_INVALID;
    else if(aynr_attr_v22.eMode == AYNRV22_OP_MODE_MAX)
        manual->AynrOpMode = RK_AIQ_OP_MODE_MAX;
    else
        manual->AynrOpMode = RK_AIQ_OP_MODE_AUTO;
#elif defined(ISP_HW_V30)
    rk_aiq_ynr_attrib_v3_t aynr_attr_v3;

    memset(&aynr_attr_v3, 0, sizeof(aynr_attr_v3));
    rk_aiq_user_api2_aynrV3_GetAttrib(sys_ctx, &aynr_attr_v3);
    memcpy(&manual->manual_v3, &aynr_attr_v3.stManual.stSelect, sizeof(manual->manual_v3));

    if (aynr_attr_v3.eMode == AYNRV3_OP_MODE_AUTO)
        manual->AynrOpMode = RK_AIQ_OP_MODE_AUTO;
    else if(aynr_attr_v3.eMode == AYNRV3_OP_MODE_MANUAL)
        manual->AynrOpMode = RK_AIQ_OP_MODE_MANUAL;
    else if(aynr_attr_v3.eMode == AYNRV3_OP_MODE_INVALID )
        manual->AynrOpMode = RK_AIQ_OP_MODE_INVALID;
    else if(aynr_attr_v3.eMode == AYNRV3_OP_MODE_MAX)
        manual->AynrOpMode = RK_AIQ_OP_MODE_MAX;
    else
        manual->AynrOpMode = RK_AIQ_OP_MODE_AUTO;
#endif

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn rk_aiq_set_acnr_manual_attr(const rk_aiq_sys_ctx_t *sys_ctx,
                                       acnr_uapi_manual_t *manual) {
    XCamReturn res = XCAM_RETURN_NO_ERROR;

#if defined(ISP_HW_V39)
    rk_aiq_cnr_attrib_v31_t acnr_attr_v31;

    memset(&acnr_attr_v31, 0, sizeof(rk_aiq_cnr_attrib_v31_t));
    rk_aiq_user_api2_acnrV31_GetAttrib(sys_ctx, &acnr_attr_v31);
    memcpy(&acnr_attr_v31.stManual.stSelect, &manual->manual_v31, sizeof(RK_CNR_Params_V31_Select_t));

    if (manual->AcnrOpMode == RK_AIQ_OP_MODE_AUTO)
        acnr_attr_v31.eMode = ACNRV31_OP_MODE_AUTO;
    else if(manual->AcnrOpMode == RK_AIQ_OP_MODE_MANUAL)
        acnr_attr_v31.eMode = ACNRV31_OP_MODE_MANUAL;
    else if(manual->AcnrOpMode == RK_AIQ_OP_MODE_INVALID)
        acnr_attr_v31.eMode = ACNRV31_OP_MODE_INVALID;
    else if(manual->AcnrOpMode == RK_AIQ_OP_MODE_MAX)
        acnr_attr_v31.eMode = ACNRV31_OP_MODE_MAX;
    else
        acnr_attr_v31.eMode = ACNRV31_OP_MODE_AUTO;

    res = rk_aiq_user_api2_acnrV31_SetAttrib(sys_ctx, &acnr_attr_v31);

#elif defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
    rk_aiq_cnr_attrib_v30_t acnr_attr_v30;

    memset(&acnr_attr_v30, 0, sizeof(rk_aiq_cnr_attrib_v30_t));
    rk_aiq_user_api2_acnrV30_GetAttrib(sys_ctx, &acnr_attr_v30);
    memcpy(&acnr_attr_v30.stManual.stSelect, &manual->manual_v30, sizeof(RK_CNR_Params_V30_Select_t));

    if (manual->AcnrOpMode == RK_AIQ_OP_MODE_AUTO)
        acnr_attr_v30.eMode = ACNRV30_OP_MODE_AUTO;
    else if(manual->AcnrOpMode == RK_AIQ_OP_MODE_MANUAL)
        acnr_attr_v30.eMode = ACNRV30_OP_MODE_MANUAL;
    else if(manual->AcnrOpMode == RK_AIQ_OP_MODE_INVALID)
        acnr_attr_v30.eMode = ACNRV30_OP_MODE_INVALID;
    else if(manual->AcnrOpMode == RK_AIQ_OP_MODE_MAX)
        acnr_attr_v30.eMode = ACNRV30_OP_MODE_MAX;
    else
        acnr_attr_v30.eMode = ACNRV30_OP_MODE_AUTO;

    res = rk_aiq_user_api2_acnrV30_SetAttrib(sys_ctx, &acnr_attr_v30);
#elif defined(ISP_HW_V30)
    rk_aiq_cnr_attrib_v2_t acnr_attr_v2;

    memset(&acnr_attr_v2, 0, sizeof(acnr_attr_v2));
    rk_aiq_user_api2_acnrV2_GetAttrib(sys_ctx, &acnr_attr_v2);
    memcpy(&acnr_attr_v2.stManual.stSelect, &manual->manual_v2, sizeof(manual->manual_v2));

    if (manual->AcnrOpMode == RK_AIQ_OP_MODE_AUTO)
        acnr_attr_v2.eMode = ACNRV2_OP_MODE_AUTO;
    else if(manual->AcnrOpMode == RK_AIQ_OP_MODE_MANUAL)
        acnr_attr_v2.eMode = ACNRV2_OP_MODE_MANUAL;
    else if(manual->AcnrOpMode == RK_AIQ_OP_MODE_INVALID)
        acnr_attr_v2.eMode = ACNRV2_OP_MODE_INVALID;
    else if(manual->AcnrOpMode == RK_AIQ_OP_MODE_MAX)
        acnr_attr_v2.eMode = ACNRV2_OP_MODE_MAX;
    else
        acnr_attr_v2.eMode = ACNRV2_OP_MODE_AUTO;

    res = rk_aiq_user_api2_acnrV2_SetAttrib(sys_ctx, &acnr_attr_v2);
#endif

    return res;
}

XCamReturn rk_aiq_get_acnr_manual_attr(const rk_aiq_sys_ctx_t *sys_ctx,
                                       acnr_uapi_manual_t *manual) {
#if defined(ISP_HW_V39)
    rk_aiq_cnr_attrib_v31_t acnr_attr_v31;
    memset(&acnr_attr_v31, 0, sizeof(rk_aiq_cnr_attrib_v31_t));
    rk_aiq_user_api2_acnrV31_GetAttrib(sys_ctx, &acnr_attr_v31);
    memcpy(&manual->manual_v31, &acnr_attr_v31.stManual.stSelect, sizeof(RK_CNR_Params_V31_Select_t));

    if (acnr_attr_v31.eMode == ACNRV31_OP_MODE_AUTO)
        manual->AcnrOpMode = RK_AIQ_OP_MODE_AUTO;
    else if(acnr_attr_v31.eMode == ACNRV31_OP_MODE_MANUAL)
        manual->AcnrOpMode = RK_AIQ_OP_MODE_MANUAL;
    else if(acnr_attr_v31.eMode == ACNRV31_OP_MODE_INVALID )
        manual->AcnrOpMode = RK_AIQ_OP_MODE_INVALID;
    else if(acnr_attr_v31.eMode == ACNRV31_OP_MODE_MAX)
        manual->AcnrOpMode = RK_AIQ_OP_MODE_MAX;
    else
        manual->AcnrOpMode = RK_AIQ_OP_MODE_AUTO;

#elif defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
    rk_aiq_cnr_attrib_v30_t acnr_attr_v30;
    memset(&acnr_attr_v30, 0, sizeof(rk_aiq_cnr_attrib_v30_t));
    rk_aiq_user_api2_acnrV30_GetAttrib(sys_ctx, &acnr_attr_v30);
    memcpy(&manual->manual_v30, &acnr_attr_v30.stManual.stSelect, sizeof(RK_CNR_Params_V30_Select_t));

    if (acnr_attr_v30.eMode == ACNRV30_OP_MODE_AUTO)
        manual->AcnrOpMode = RK_AIQ_OP_MODE_AUTO;
    else if(acnr_attr_v30.eMode == ACNRV30_OP_MODE_MANUAL)
        manual->AcnrOpMode = RK_AIQ_OP_MODE_MANUAL;
    else if(acnr_attr_v30.eMode == ACNRV30_OP_MODE_INVALID )
        manual->AcnrOpMode = RK_AIQ_OP_MODE_INVALID;
    else if(acnr_attr_v30.eMode == ACNRV30_OP_MODE_MAX)
        manual->AcnrOpMode = RK_AIQ_OP_MODE_MAX;
    else
        manual->AcnrOpMode = RK_AIQ_OP_MODE_AUTO;
#elif defined(ISP_HW_V30)
    rk_aiq_cnr_attrib_v2_t acnr_attr_v2;
    memset(&acnr_attr_v2, 0, sizeof(acnr_attr_v2));
    rk_aiq_user_api2_acnrV2_GetAttrib(sys_ctx, &acnr_attr_v2);
    memcpy(&manual->manual_v2, &acnr_attr_v2.stManual.stSelect, sizeof(manual->manual_v2));

    if (acnr_attr_v2.eMode == ACNRV2_OP_MODE_AUTO)
        manual->AcnrOpMode = RK_AIQ_OP_MODE_AUTO;
    else if(acnr_attr_v2.eMode == ACNRV2_OP_MODE_MANUAL)
        manual->AcnrOpMode = RK_AIQ_OP_MODE_MANUAL;
    else if(acnr_attr_v2.eMode == ACNRV2_OP_MODE_INVALID )
        manual->AcnrOpMode = RK_AIQ_OP_MODE_INVALID;
    else if(acnr_attr_v2.eMode == ACNRV2_OP_MODE_MAX)
        manual->AcnrOpMode = RK_AIQ_OP_MODE_MAX;
    else
        manual->AcnrOpMode = RK_AIQ_OP_MODE_AUTO;
#endif

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn rk_aiq_set_again_manual_attr(const rk_aiq_sys_ctx_t *sys_ctx,
                                        again_uapi_manual_t *manual) {
    XCamReturn res = XCAM_RETURN_NO_ERROR;

#if defined(ISP_HW_V39) || defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE) || defined(ISP_HW_V30)
    rk_aiq_gain_attrib_v2_t again_attr_v2;
    memset(&again_attr_v2, 0, sizeof(rk_aiq_gain_attrib_v2_t));
    rk_aiq_user_api2_againV2_GetAttrib(sys_ctx, &again_attr_v2);
    memcpy(&again_attr_v2.stManual.stSelect, &manual->manual_v2, sizeof(RK_GAIN_Select_V2_t));

    if (manual->AgainOpMode == RK_AIQ_OP_MODE_AUTO)
        again_attr_v2.eMode = AGAINV2_OP_MODE_AUTO;
    else if(manual->AgainOpMode == RK_AIQ_OP_MODE_MANUAL)
        again_attr_v2.eMode = AGAINV2_OP_MODE_MANUAL;
    else if(manual->AgainOpMode == RK_AIQ_OP_MODE_INVALID)
        again_attr_v2.eMode = AGAINV2_OP_MODE_INVALID;
    else if(manual->AgainOpMode == RK_AIQ_OP_MODE_MAX)
        again_attr_v2.eMode = AGAINV2_OP_MODE_MAX;
    else
        again_attr_v2.eMode = AGAINV2_OP_MODE_AUTO;

    res = rk_aiq_user_api2_againV2_SetAttrib(sys_ctx, &again_attr_v2);
#endif

    return res;
}

XCamReturn rk_aiq_get_again_manual_attr(const rk_aiq_sys_ctx_t *sys_ctx,
                                        again_uapi_manual_t *manual) {
#if defined(ISP_HW_V39) || defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE) || defined(ISP_HW_V30)
    rk_aiq_gain_attrib_v2_t again_attr_v2;

    memset(&again_attr_v2, 0, sizeof(rk_aiq_gain_attrib_v2_t));
    rk_aiq_user_api2_againV2_GetAttrib(sys_ctx, &again_attr_v2);
    memcpy(&manual->manual_v2, &again_attr_v2.stManual.stSelect, sizeof(RK_GAIN_Select_V2_t));

    if (again_attr_v2.eMode == AGAINV2_OP_MODE_AUTO)
        manual->AgainOpMode = RK_AIQ_OP_MODE_AUTO;
    else if(again_attr_v2.eMode == AGAINV2_OP_MODE_MANUAL)
        manual->AgainOpMode = RK_AIQ_OP_MODE_MANUAL;
    else if(again_attr_v2.eMode == AGAINV2_OP_MODE_INVALID )
        manual->AgainOpMode = RK_AIQ_OP_MODE_INVALID;
    else if(again_attr_v2.eMode == AGAINV2_OP_MODE_MAX)
        manual->AgainOpMode = RK_AIQ_OP_MODE_MAX;
    else
        manual->AgainOpMode = RK_AIQ_OP_MODE_AUTO;
#endif
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn rk_aiq_set_ayuvme_manual_attr(const rk_aiq_sys_ctx_t *sys_ctx,
        ayuvme_uapi_manual_t *manual) {
    XCamReturn res = XCAM_RETURN_NO_ERROR;

#if defined(ISP_HW_V39)
    rk_aiq_yuvme_attrib_v1_t ayuvme_attr_v1;
    memset(&ayuvme_attr_v1, 0, sizeof(rk_aiq_yuvme_attrib_v1_t));
    rk_aiq_user_api2_ayuvmeV1_GetAttrib(sys_ctx, &ayuvme_attr_v1);
    memcpy(&ayuvme_attr_v1.stManual.stSelect, &manual->manual_v1, sizeof(RK_YUVME_Params_V1_Select_t));

    if (manual->AyuvmeOpMode == RK_AIQ_OP_MODE_AUTO)
        ayuvme_attr_v1.eMode = AYUVMEV1_OP_MODE_AUTO;
    else if(manual->AyuvmeOpMode == RK_AIQ_OP_MODE_MANUAL)
        ayuvme_attr_v1.eMode = AYUVMEV1_OP_MODE_MANUAL;
    else if(manual->AyuvmeOpMode == RK_AIQ_OP_MODE_INVALID)
        ayuvme_attr_v1.eMode = AYUVMEV1_OP_MODE_INVALID;
    else if(manual->AyuvmeOpMode == RK_AIQ_OP_MODE_MAX)
        ayuvme_attr_v1.eMode = AYUVMEV1_OP_MODE_MAX;
    else
        ayuvme_attr_v1.eMode = AYUVMEV1_OP_MODE_AUTO;

    res = rk_aiq_user_api2_ayuvmeV1_SetAttrib(sys_ctx, &ayuvme_attr_v1);
#endif

    return res;
}

XCamReturn rk_aiq_get_ayuvme_manual_attr(const rk_aiq_sys_ctx_t *sys_ctx,
        ayuvme_uapi_manual_t *manual) {
#if defined(ISP_HW_V39)
    rk_aiq_yuvme_attrib_v1_t ayuvme_attr_v1;

    memset(&ayuvme_attr_v1, 0, sizeof(rk_aiq_yuvme_attrib_v1_t));
    rk_aiq_user_api2_ayuvmeV1_GetAttrib(sys_ctx, &ayuvme_attr_v1);
    memcpy(&manual->manual_v1, &ayuvme_attr_v1.stManual.stSelect, sizeof(RK_YUVME_Params_V1_Select_t));

    if (ayuvme_attr_v1.eMode == AYUVMEV1_OP_MODE_AUTO)
        manual->AyuvmeOpMode = RK_AIQ_OP_MODE_AUTO;
    else if(ayuvme_attr_v1.eMode == AYUVMEV1_OP_MODE_MANUAL)
        manual->AyuvmeOpMode = RK_AIQ_OP_MODE_MANUAL;
    else if(ayuvme_attr_v1.eMode == AYUVMEV1_OP_MODE_INVALID )
        manual->AyuvmeOpMode = RK_AIQ_OP_MODE_INVALID;
    else if(ayuvme_attr_v1.eMode == AYUVMEV1_OP_MODE_MAX)
        manual->AyuvmeOpMode = RK_AIQ_OP_MODE_MAX;
    else
        manual->AyuvmeOpMode = RK_AIQ_OP_MODE_AUTO;
#endif
    return XCAM_RETURN_NO_ERROR;
}


XCamReturn
rk_aiq_get_ablc_info(const rk_aiq_sys_ctx_t *sys_ctx,
                     ablc_uapi_info_t* pInfo) {

#if defined(ISP_HW_V39) || defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
    rk_aiq_blc_info_v32_t ablc_info_v23;
    rk_aiq_user_api2_ablcV32_GetInfo(sys_ctx, &ablc_info_v23);
    pInfo->iso = ablc_info_v23.iso;
    pInfo->expo_info = ablc_info_v23.expo_info;
#elif defined(ISP_HW_V30)
    rk_aiq_ablc_info_t ablc_info;
    rk_aiq_user_api2_ablc_GetInfo(sys_ctx, &ablc_info);
    pInfo->iso = ablc_info.iso;
    pInfo->expo_info = ablc_info.expo_info;
#endif
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_get_abayertnr_info(const rk_aiq_sys_ctx_t *sys_ctx,
                          abayertnr_uapi_info_t  *info) {

#if defined(ISP_HW_V39)
    rk_aiq_bayertnr_info_v30_t abayertnr_info_v30;
    rk_aiq_user_api2_abayertnrV30_GetInfo(sys_ctx, &abayertnr_info_v30);
    info->iso = abayertnr_info_v30.iso;
    info->expo_info = abayertnr_info_v30.expo_info;
#elif defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
    rk_aiq_bayertnr_info_v23_t abayertnr_info_v23;
    rk_aiq_user_api2_abayertnrV23_GetInfo(sys_ctx, &abayertnr_info_v23);
    info->iso = abayertnr_info_v23.iso;
    info->expo_info = abayertnr_info_v23.expo_info;
#elif defined(ISP_HW_V30)
    rk_aiq_bayertnr_info_v2_t abayertnr_info_v2;
    rk_aiq_user_api2_abayertnrV2_GetInfo(sys_ctx, &abayertnr_info_v2);
    info->iso = abayertnr_info_v2.iso;
    info->expo_info = abayertnr_info_v2.expo_info;
#endif
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_get_abayer2dnr_info(const rk_aiq_sys_ctx_t *sys_ctx,
                           abayer2dnr_uapi_info_t *info) {
#if  defined(ISP_HW_V32)
    rk_aiq_bayer2dnr_info_v23_t bayer2dnr_info_v23;
    rk_aiq_user_api2_abayer2dnrV23_GetInfo(sys_ctx, &bayer2dnr_info_v23);
    info->iso = bayer2dnr_info_v23.iso;
    info->expo_info = bayer2dnr_info_v23.expo_info;
#elif defined(ISP_HW_V30)
    rk_aiq_bayer2dnr_info_v2_t bayer2dnr_info_v2;
    rk_aiq_user_api2_abayer2dnrV2_GetInfo(sys_ctx, &bayer2dnr_info_v2);
    info->iso = bayer2dnr_info_v2.iso;
    info->expo_info = bayer2dnr_info_v2.expo_info;
#endif
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_get_aynr_info(const rk_aiq_sys_ctx_t *sys_ctx,
                     aynr_uapi_info_t  *info) {
#if defined(ISP_HW_V39)
    rk_aiq_ynr_info_v24_t ynr_info_v24;
    rk_aiq_user_api2_aynrV24_GetInfo(sys_ctx, &ynr_info_v24);
    info->iso = ynr_info_v24.iso;
    info->expo_info = ynr_info_v24.expo_info;
#elif defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
    rk_aiq_ynr_info_v22_t ynr_info_v22;
    rk_aiq_user_api2_aynrV22_GetInfo(sys_ctx, &ynr_info_v22);
    info->iso = ynr_info_v22.iso;
    info->expo_info = ynr_info_v22.expo_info;
#elif defined(ISP_HW_V30)
    rk_aiq_ynr_info_v3_t ynr_info_v3;
    rk_aiq_user_api2_aynrV3_GetInfo(sys_ctx, &ynr_info_v3);
    info->iso = ynr_info_v3.iso;
    info->expo_info = ynr_info_v3.expo_info;
#endif
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_get_acnr_info(const rk_aiq_sys_ctx_t *sys_ctx,
                     acnr_uapi_info_t  *info) {
#if defined(ISP_HW_V39)
    rk_aiq_cnr_info_v31_t cnr_info_v31;
    rk_aiq_user_api2_acnrV31_GetInfo(sys_ctx, &cnr_info_v31);
    info->iso = cnr_info_v31.iso;
    info->expo_info = cnr_info_v31.expo_info;
#elif defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
    rk_aiq_cnr_info_v30_t cnr_info_v30;
    rk_aiq_user_api2_acnrV30_GetInfo(sys_ctx, &cnr_info_v30);
    info->iso = cnr_info_v30.iso;
    info->expo_info = cnr_info_v30.expo_info;
#elif defined(ISP_HW_V30)
    rk_aiq_cnr_info_v2_t cnr_info_v2;
    rk_aiq_user_api2_acnrV2_GetInfo(sys_ctx, &cnr_info_v2);
    info->iso = cnr_info_v2.iso;
    info->expo_info = cnr_info_v2.expo_info;
#endif

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_get_asharp_info(const rk_aiq_sys_ctx_t *sys_ctx,
                       asharp_uapi_info_t *info) {

#if defined(ISP_HW_V39)
    rk_aiq_sharp_info_v34_t sharp_info_v34;
    rk_aiq_user_api2_asharpV34_GetInfo(sys_ctx, &sharp_info_v34);
    info->iso = sharp_info_v34.iso;
    info->expo_info = sharp_info_v34.expo_info;
#elif defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
    rk_aiq_sharp_info_v33_t sharp_info_v33;
    rk_aiq_user_api2_asharpV33_GetInfo(sys_ctx, &sharp_info_v33);
    info->iso = sharp_info_v33.iso;
    info->expo_info = sharp_info_v33.expo_info;
#elif defined(ISP_HW_V30)
    rk_aiq_sharp_info_v4_t sharp_info_v4;
    rk_aiq_user_api2_asharpV4_GetInfo(sys_ctx, &sharp_info_v4);
    info->iso = sharp_info_v4.iso;
    info->expo_info = sharp_info_v4.expo_info;
#endif

    return XCAM_RETURN_NO_ERROR;

}


XCamReturn
rk_aiq_get_again_info(const rk_aiq_sys_ctx_t *sys_ctx,
                      again_uapi_info_t *info) {

#if defined(ISP_HW_V39) || defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE) || defined(ISP_HW_V30)
    rk_aiq_gain_info_v2_t gain_info_v2;
    rk_aiq_user_api2_againV2_GetInfo(sys_ctx, &gain_info_v2);
    info->iso = gain_info_v2.iso;
    info->expo_info = gain_info_v2.expo_info;
#endif

    return XCAM_RETURN_NO_ERROR;
}



XCamReturn
rk_aiq_get_ayuvme_info(const rk_aiq_sys_ctx_t *sys_ctx,
                       ayuvme_uapi_info_t *info) {

#if defined(ISP_HW_V39)
    rk_aiq_yuvme_info_v1_t yuvme_info_v1;
    rk_aiq_user_api2_ayuvmeV1_GetInfo(sys_ctx, &yuvme_info_v1);
    info->iso = yuvme_info_v1.iso;
    info->expo_info = yuvme_info_v1.expo_info;
#endif

    return XCAM_RETURN_NO_ERROR;
}


rk_aiq_sys_ctx_t* rk_aiq_get_last_sysctx(rk_aiq_sys_ctx_t *sys_ctx) {
#if RKAIQ_ENABLE_CAMGROUP
    camgroup_uapi_t last_arg;
    rk_aiq_camgroup_ctx_t* grp_ctx = NULL;

    // Single camera instance
    grp_ctx = get_binded_group_ctx(sys_ctx);
    if (!grp_ctx) {
        return sys_ctx->next_ctx ? sys_ctx->next_ctx : sys_ctx;
    }

    // Group camera instance
    // Check if an special index of camera required
    memset(&last_arg, 0, sizeof(camgroup_uapi_t));
    rk_aiq_get_current_camindex(sys_ctx, &last_arg);
    if (last_arg.current_index == 0) {
        return (rk_aiq_sys_ctx_t*)grp_ctx;
    }

    for (auto cam_ctx : grp_ctx->cam_ctxs_array) {
        if (!cam_ctx || cam_ctx->_camPhyId != (last_arg.current_index - 1))
            continue;
        return cam_ctx;
    }

#endif
    return NULL;
}
