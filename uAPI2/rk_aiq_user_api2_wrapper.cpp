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

#include "uAPI2/rk_aiq_user_api2_wrapper.h"
#include "RkAiqCalibDbV2.h"
#include "RkAiqUapitypes.h"
#include "cJSON.h"
#include "cJSON_Utils.h"
#include "j2s.h"
#include "rk_aiq_user_api_sysctl.h"
#include "stdlib.h"
#include "string.h"
#include "uAPI/include/rk_aiq_user_api_sysctl.h"
#include "uAPI2/rk_aiq_user_api2_ae.h"
#include "uAPI2/rk_aiq_user_api2_acsm.h"
#include "uAPI2/rk_aiq_user_api2_imgproc.h"
#include "uAPI2/rk_aiq_user_api2_sysctl.h"
#include "rk_aiq_api_private.h"

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

int rk_aiq_user_api2_amerge_GetCtldata(const rk_aiq_sys_ctx_t *sys_ctx,
                                       uapiMergeCurrCtlData_t *ctldata) {
  amerge_attrib_t setdata;

  memset(&setdata, 0, sizeof(amerge_attrib_t));

  rk_aiq_user_api2_amerge_GetAttrib(sys_ctx, &setdata);

  if (CHECK_ISP_HW_V21())
    memcpy(ctldata, &setdata.attrV21.CtlInfo, sizeof(uapiMergeCurrCtlData_t));
  else if (CHECK_ISP_HW_V30())
    memcpy(ctldata, &setdata.attrV30.CtlInfo, sizeof(uapiMergeCurrCtlData_t));

  return 0;
}

int rk_aiq_user_api2_set_scene(const rk_aiq_sys_ctx_t *sys_ctx,
                               aiq_scene_t *scene) {
  return rk_aiq_uapi_sysctl_switch_scene(sys_ctx, scene->main_scene,
                                         scene->sub_scene);
}

int rk_aiq_user_api2_get_scene(const rk_aiq_sys_ctx_t *sys_ctx,
                               aiq_scene_t *scene) {
  (void)sys_ctx;
  scene->main_scene = strdup("normal");
  scene->sub_scene = strdup("day");

  return 0;
}

int rk_aiq_uapi_get_ae_hwstats(const rk_aiq_sys_ctx_t *sys_ctx,
                               uapi_ae_hwstats_t *ae_hwstats) {
  rk_aiq_isp_stats_t isp_stats;

  if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
    LOGE("Can't read 3A stats for group ctx!");
    return XCAM_RETURN_ERROR_PARAM;
  }

  rk_aiq_uapi_sysctl_get3AStats(sys_ctx, &isp_stats);

  memcpy(ae_hwstats, &isp_stats.aec_stats.ae_data, sizeof(uapi_ae_hwstats_t));

  return XCAM_RETURN_NO_ERROR;
}

int rk_aiq_uapi_get_awb_stat(const rk_aiq_sys_ctx_t *sys_ctx,
                             rk_tool_awb_stat_res2_v30_t *awb_stat) {
  rk_aiq_isp_stats_t isp_stats;

  if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
    LOGE("Can't read 3A stats for group ctx!");
    return XCAM_RETURN_ERROR_PARAM;
  }

  rk_aiq_uapi_sysctl_get3AStats(sys_ctx, &isp_stats);

  memcpy(awb_stat, &isp_stats.awb_stats_v3x,
         sizeof(rk_tool_awb_stat_res2_v30_t));

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

XCamReturn rk_aiq_user_api2_adrc_queryinfo(const rk_aiq_sys_ctx_t *sys_ctx,
                                           DrcInfo_t *drc_info) {
  drc_attrib_t drc_attr;

  memset(&drc_attr, 0, sizeof(drc_attrib_t));
  rk_aiq_user_api2_adrc_GetAttrib(sys_ctx, &drc_attr);
  memcpy(&drc_info, &drc_attr.Info, sizeof(DrcInfo_t));

  return XCAM_RETURN_NO_ERROR;
}

XCamReturn rk_aiq_set_adrc_manual_attr(const rk_aiq_sys_ctx_t *sys_ctx,
                                       mdrcAttr_V30_t *manual) {
  drc_attrib_t drc_attr;

  memset(&drc_attr, 0, sizeof(drc_attrib_t));
  rk_aiq_user_api2_adrc_GetAttrib(sys_ctx, &drc_attr);
  memcpy(&drc_attr.stManualV30, manual, sizeof(mdrcAttr_V30_t));
  rk_aiq_user_api2_adrc_SetAttrib(sys_ctx, drc_attr);

  return XCAM_RETURN_NO_ERROR;
}

XCamReturn rk_aiq_get_adrc_manual_attr(const rk_aiq_sys_ctx_t *sys_ctx,
                                       mdrcAttr_V30_t *manual) {
  drc_attrib_t drc_attr;

  memset(&drc_attr, 0, sizeof(drc_attrib_t));
  rk_aiq_user_api2_adrc_GetAttrib(sys_ctx, &drc_attr);
  memcpy(manual, &drc_attr.stManualV30, sizeof(mdrcAttr_V30_t));

  return XCAM_RETURN_NO_ERROR;
}

XCamReturn rk_aiq_set_amerge_manual_attr(const rk_aiq_sys_ctx_t *sys_ctx,
                                         mMergeAttrV30_t *manual) {
  amerge_attrib_t setdata;

  memset(&setdata, 0, sizeof(amerge_attrib_t));
  rk_aiq_user_api2_amerge_GetAttrib(sys_ctx, &setdata);
  memcpy(&setdata.attrV30.stManual, manual, sizeof(mMergeAttrV30_t));

  return rk_aiq_user_api2_amerge_SetAttrib(sys_ctx, setdata);
}

XCamReturn rk_aiq_get_amerge_manual_attr(const rk_aiq_sys_ctx_t *sys_ctx,
                                         mMergeAttrV30_t *manual) {
  amerge_attrib_t setdata;

  memset(&setdata, 0, sizeof(amerge_attrib_t));
  rk_aiq_user_api2_amerge_GetAttrib(sys_ctx, &setdata);
  memcpy(manual, &setdata.attrV30.stManual, sizeof(mMergeAttrV30_t));

  return XCAM_RETURN_NO_ERROR;
}

XCamReturn rk_aiq_set_agamma_manual_attr(const rk_aiq_sys_ctx_t *sys_ctx,
                                         Agamma_api_manualV30_t *manual) {
  rk_aiq_gamma_attrib_V2_t agamma_attr;

  memset(&agamma_attr, 0, sizeof(rk_aiq_gamma_attrib_V2_t));
  rk_aiq_user_api2_agamma_GetAttrib(sys_ctx, &agamma_attr);
  memcpy(&agamma_attr.atrrV30.stManual, manual, sizeof(Agamma_api_manualV30_t));

  return rk_aiq_user_api2_agamma_SetAttrib(sys_ctx, agamma_attr);
}

XCamReturn rk_aiq_get_agamma_manual_attr(const rk_aiq_sys_ctx_t *sys_ctx,
                                         Agamma_api_manualV30_t *manual) {
  rk_aiq_gamma_attrib_V2_t agamma_attr;

  memset(&agamma_attr, 0, sizeof(rk_aiq_gamma_attrib_V2_t));
  rk_aiq_user_api2_agamma_GetAttrib(sys_ctx, &agamma_attr);
  memcpy(manual, &agamma_attr.atrrV30.stManual, sizeof(Agamma_api_manualV30_t));

  return XCAM_RETURN_NO_ERROR;
}

XCamReturn rk_aiq_set_asharp_manual_attr(const rk_aiq_sys_ctx_t *sys_ctx,
                                         RK_SHARP_Params_V4_Select_t *manual) {
  rk_aiq_sharp_attrib_v4_t sharp_attr;

  memset(&sharp_attr, 0, sizeof(rk_aiq_sharp_attrib_v4_t));
  rk_aiq_user_api2_asharpV4_GetAttrib(sys_ctx, &sharp_attr);
  memcpy(&sharp_attr.stManual.stSelect, manual, sizeof(RK_SHARP_Params_V4_Select_t));

  return rk_aiq_user_api2_asharpV4_SetAttrib(sys_ctx, &sharp_attr);
}

XCamReturn rk_aiq_get_asharp_manual_attr(const rk_aiq_sys_ctx_t *sys_ctx,
                                         RK_SHARP_Params_V4_Select_t *manual) {
  rk_aiq_sharp_attrib_v4_t sharp_attr;

  memset(&sharp_attr, 0, sizeof(rk_aiq_sharp_attrib_v4_t));
  rk_aiq_user_api2_asharpV4_GetAttrib(sys_ctx, &sharp_attr);
  memcpy(manual, &sharp_attr.stManual.stSelect, sizeof(RK_SHARP_Params_V4_Select_t));

  return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_set_abayer2dnr_manual_attr(const rk_aiq_sys_ctx_t *sys_ctx,
                                  RK_Bayer2dnr_Params_V2_Select_t *manual) {
  rk_aiq_bayer2dnr_attrib_v2_t abayer2dnr_attr;

  memset(&abayer2dnr_attr, 0, sizeof(rk_aiq_bayer2dnr_attrib_v2_t));
  rk_aiq_user_api2_abayer2dnrV2_GetAttrib(sys_ctx, &abayer2dnr_attr);
  memcpy(&abayer2dnr_attr.stManual.st2DSelect, manual,
         sizeof(RK_Bayer2dnr_Params_V2_Select_t));

  return rk_aiq_user_api2_abayer2dnrV2_SetAttrib(sys_ctx, &abayer2dnr_attr);
}

XCamReturn
rk_aiq_get_abayer2dnr_manual_attr(const rk_aiq_sys_ctx_t *sys_ctx,
                                  RK_Bayer2dnr_Params_V2_Select_t *manual) {
  rk_aiq_bayer2dnr_attrib_v2_t abayer2dnr_attr;

  memset(&abayer2dnr_attr, 0, sizeof(rk_aiq_bayer2dnr_attrib_v2_t));
  rk_aiq_user_api2_abayer2dnrV2_GetAttrib(sys_ctx, &abayer2dnr_attr);
  memcpy(manual, &abayer2dnr_attr.stManual.st2DSelect,
         sizeof(RK_Bayer2dnr_Params_V2_Select_t));

  return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_set_abayertnr_manual_attr(const rk_aiq_sys_ctx_t *sys_ctx,
                                 RK_Bayertnr_Params_V2_Select_t *manual) {
  rk_aiq_bayertnr_attrib_v2_t abayertnr_attr;

  memset(&abayertnr_attr, 0, sizeof(rk_aiq_bayertnr_attrib_v2_t));
  rk_aiq_user_api2_abayertnrV2_GetAttrib(sys_ctx, &abayertnr_attr);
  memcpy(&abayertnr_attr.stManual.st3DSelect, manual,
         sizeof(RK_Bayertnr_Params_V2_Select_t));

  return rk_aiq_user_api2_abayertnrV2_SetAttrib(sys_ctx, &abayertnr_attr);
}

XCamReturn
rk_aiq_get_abayertnr_manual_attr(const rk_aiq_sys_ctx_t *sys_ctx,
                                 RK_Bayertnr_Params_V2_Select_t *manual) {
  rk_aiq_bayertnr_attrib_v2_t abayertnr_attr;

  memset(&abayertnr_attr, 0, sizeof(rk_aiq_bayertnr_attrib_v2_t));
  rk_aiq_user_api2_abayertnrV2_GetAttrib(sys_ctx, &abayertnr_attr);
  memcpy(manual, &abayertnr_attr.stManual.st3DSelect,
         sizeof(RK_Bayertnr_Params_V2_Select_t));

  return XCAM_RETURN_NO_ERROR;
}

XCamReturn rk_aiq_set_aynr_manual_attr(const rk_aiq_sys_ctx_t *sys_ctx,
                                       RK_YNR_Params_V3_Select_t *manual) {
  rk_aiq_ynr_attrib_v3_t aynr_attr;

  memset(&aynr_attr, 0, sizeof(rk_aiq_ynr_attrib_v3_t));
  rk_aiq_user_api2_aynrV3_GetAttrib(sys_ctx, &aynr_attr);
  memcpy(&aynr_attr.stManual.stSelect, manual, sizeof(RK_YNR_Params_V3_Select_t));

  return rk_aiq_user_api2_aynrV3_SetAttrib(sys_ctx, &aynr_attr);
}

XCamReturn rk_aiq_get_aynr_manual_attr(const rk_aiq_sys_ctx_t *sys_ctx,
                                       RK_YNR_Params_V3_Select_t *manual) {
  rk_aiq_ynr_attrib_v3_t aynr_attr;

  memset(&aynr_attr, 0, sizeof(rk_aiq_ynr_attrib_v3_t));
  rk_aiq_user_api2_aynrV3_GetAttrib(sys_ctx, &aynr_attr);
  memcpy(manual, &aynr_attr.stManual.stSelect, sizeof(RK_YNR_Params_V3_Select_t));

  return XCAM_RETURN_NO_ERROR;
}

XCamReturn rk_aiq_set_acnr_manual_attr(const rk_aiq_sys_ctx_t *sys_ctx,
                                       RK_CNR_Params_V2_Select_t *manual) {
  rk_aiq_cnr_attrib_v2_t acnr_attr;

  memset(&acnr_attr, 0, sizeof(rk_aiq_cnr_attrib_v2_t));
  rk_aiq_user_api2_acnrV2_GetAttrib(sys_ctx, &acnr_attr);
  memcpy(&acnr_attr.stManual.stSelect, manual, sizeof(RK_CNR_Params_V2_Select_t));

  return rk_aiq_user_api2_acnrV2_SetAttrib(sys_ctx, &acnr_attr);
}

XCamReturn rk_aiq_get_acnr_manual_attr(const rk_aiq_sys_ctx_t *sys_ctx,
                                       RK_CNR_Params_V2_Select_t *manual) {
  rk_aiq_cnr_attrib_v2_t acnr_attr;

  memset(&acnr_attr, 0, sizeof(rk_aiq_cnr_attrib_v2_t));
  rk_aiq_user_api2_acnrV2_GetAttrib(sys_ctx, &acnr_attr);
  memcpy(manual, &acnr_attr.stManual.stSelect, sizeof(RK_CNR_Params_V2_Select_t));

  return XCAM_RETURN_NO_ERROR;
}

XCamReturn rk_aiq_set_again_manual_attr(const rk_aiq_sys_ctx_t *sys_ctx,
                                        RK_GAIN_Select_V2_t *manual) {
  rk_aiq_gain_attrib_v2_t again_attr;

  memset(&again_attr, 0, sizeof(rk_aiq_gain_attrib_v2_t));
  rk_aiq_user_api2_againV2_GetAttrib(sys_ctx, &again_attr);
  memcpy(&again_attr.stManual.stSelect, manual, sizeof(RK_GAIN_Select_V2_t));

  return rk_aiq_user_api2_againV2_SetAttrib(sys_ctx, &again_attr);
}

XCamReturn rk_aiq_get_again_manual_attr(const rk_aiq_sys_ctx_t *sys_ctx,
                                        RK_GAIN_Select_V2_t *manual) {
  rk_aiq_gain_attrib_v2_t again_attr;

  memset(&again_attr, 0, sizeof(rk_aiq_gain_attrib_v2_t));
  rk_aiq_user_api2_againV2_GetAttrib(sys_ctx, &again_attr);
  memcpy(manual, &again_attr.stManual.stSelect, sizeof(RK_GAIN_Select_V2_t));

  return XCAM_RETURN_NO_ERROR;
}

XCamReturn rk_aiq_set_ablc0_manual_attr(const rk_aiq_sys_ctx_t *sys_ctx,
                                        AblcSelect_t *manual) {
  rk_aiq_blc_attrib_t ablc_attr;

  memset(&ablc_attr, 0, sizeof(rk_aiq_blc_attrib_t));
  rk_aiq_user_api2_ablc_GetAttrib(sys_ctx, &ablc_attr);
  memcpy(&ablc_attr.stBlc0Manual, manual, sizeof(AblcSelect_t));

  return rk_aiq_user_api2_ablc_SetAttrib(sys_ctx, &ablc_attr);
}

XCamReturn rk_aiq_get_ablc0_manual_attr(const rk_aiq_sys_ctx_t *sys_ctx,
                                        AblcSelect_t *manual) {
  rk_aiq_blc_attrib_t ablc_attr;

  memset(&ablc_attr, 0, sizeof(rk_aiq_blc_attrib_t));
  rk_aiq_user_api2_ablc_GetAttrib(sys_ctx, &ablc_attr);
  memcpy(manual, &ablc_attr.stBlc0Manual, sizeof(AblcSelect_t));

  return XCAM_RETURN_NO_ERROR;
}

XCamReturn rk_aiq_set_ablc1_manual_attr(const rk_aiq_sys_ctx_t *sys_ctx,
                                        AblcSelect_t *manual) {
  rk_aiq_blc_attrib_t ablc_attr;

  memset(&ablc_attr, 0, sizeof(rk_aiq_blc_attrib_t));
  rk_aiq_user_api2_ablc_GetAttrib(sys_ctx, &ablc_attr);
  memcpy(&ablc_attr.stBlc0Manual, manual, sizeof(AblcSelect_t));

  return rk_aiq_user_api2_ablc_SetAttrib(sys_ctx, &ablc_attr);
}

XCamReturn rk_aiq_get_ablc1_manual_attr(const rk_aiq_sys_ctx_t *sys_ctx,
                                        AblcSelect_t *manual) {
  rk_aiq_blc_attrib_t ablc_attr;

  memset(&ablc_attr, 0, sizeof(rk_aiq_blc_attrib_t));
  rk_aiq_user_api2_ablc_GetAttrib(sys_ctx, &ablc_attr);
  memcpy(manual, &ablc_attr.stBlc0Manual, sizeof(AblcSelect_t));

  return XCAM_RETURN_NO_ERROR;
}

XCamReturn rk_aiq_set_accm_manual_attr(const rk_aiq_sys_ctx_t *sys_ctx,
                                       rk_aiq_ccm_mccm_attrib_t *manual) {
  rk_aiq_ccm_attrib_t accm_attr;

  memset(&accm_attr, 0, sizeof(rk_aiq_ccm_attrib_t));
  rk_aiq_user_api2_accm_GetAttrib(sys_ctx, &accm_attr);
  memcpy(&accm_attr.stManual, manual, sizeof(rk_aiq_ccm_mccm_attrib_t));

  return rk_aiq_user_api2_accm_SetAttrib(sys_ctx, accm_attr);
}

XCamReturn rk_aiq_get_accm_manual_attr(const rk_aiq_sys_ctx_t *sys_ctx,
                                       rk_aiq_ccm_mccm_attrib_t *manual) {
  rk_aiq_ccm_attrib_t accm_attr;

  memset(&accm_attr, 0, sizeof(rk_aiq_ccm_attrib_t));
  rk_aiq_user_api2_accm_GetAttrib(sys_ctx, &accm_attr);
  memcpy(manual, &accm_attr.stManual, sizeof(rk_aiq_ccm_mccm_attrib_t));

  return XCAM_RETURN_NO_ERROR;
}

XCamReturn rk_aiq_set_a3dlut_manual_attr(const rk_aiq_sys_ctx_t *sys_ctx,
                                         rk_aiq_lut3d_mlut3d_attrib_t *manual) {
  rk_aiq_lut3d_attrib_t a3dlut_attr;

  memset(&a3dlut_attr, 0, sizeof(rk_aiq_lut3d_attrib_t));
  rk_aiq_user_api2_a3dlut_GetAttrib(sys_ctx, &a3dlut_attr);
  memcpy(&a3dlut_attr.stManual, manual, sizeof(rk_aiq_lut3d_mlut3d_attrib_t));

  return rk_aiq_user_api2_a3dlut_SetAttrib(sys_ctx, a3dlut_attr);
}

XCamReturn rk_aiq_get_a3dlut_manual_attr(const rk_aiq_sys_ctx_t *sys_ctx,
                                         rk_aiq_lut3d_mlut3d_attrib_t *manual) {
  rk_aiq_lut3d_attrib_t a3dlut_attr;

  memset(&a3dlut_attr, 0, sizeof(rk_aiq_lut3d_attrib_t));
  rk_aiq_user_api2_a3dlut_GetAttrib(sys_ctx, &a3dlut_attr);
  memcpy(manual, &a3dlut_attr.stManual, sizeof(rk_aiq_lut3d_mlut3d_attrib_t));

  return XCAM_RETURN_NO_ERROR;
}

XCamReturn rk_aiq_set_acsm_manual_attr(const rk_aiq_sys_ctx_t *sys_ctx,
                                       Csm_Param_t *manual) {
  rk_aiq_uapi_acsm_attrib_t acsm_attr;

  memset(&acsm_attr, 0, sizeof(rk_aiq_uapi_acsm_attrib_t));
  rk_aiq_user_api2_acsm_GetAttrib(sys_ctx, &acsm_attr);
  memcpy(&acsm_attr.param, manual, sizeof(Csm_Param_t));

  return rk_aiq_user_api2_acsm_SetAttrib(sys_ctx, acsm_attr);
}

XCamReturn rk_aiq_get_acsm_manual_attr(const rk_aiq_sys_ctx_t *sys_ctx,
                                       Csm_Param_t *manual) {
  rk_aiq_uapi_acsm_attrib_t acsm_attr;

  memset(&acsm_attr, 0, sizeof(rk_aiq_uapi_acsm_attrib_t));
  rk_aiq_user_api2_acsm_GetAttrib(sys_ctx, &acsm_attr);
  memcpy(manual, &acsm_attr.param, sizeof(Csm_Param_t));

  return XCAM_RETURN_NO_ERROR;
}

XCamReturn rk_aiq_set_adehaze_manual_attr(const rk_aiq_sys_ctx_t *sys_ctx,
                                          mDehazeAttr_t *manual) {
  adehaze_sw_V2_t adehaze_attr;

  memset(&adehaze_attr, 0, sizeof(adehaze_sw_V2_t));
  rk_aiq_user_api2_adehaze_getSwAttrib(sys_ctx, &adehaze_attr);
  memcpy(&adehaze_attr.stManual, manual, sizeof(mDehazeAttr_t));

  return rk_aiq_user_api2_adehaze_setSwAttrib(sys_ctx, adehaze_attr);
}

XCamReturn rk_aiq_get_adehaze_manual_attr(const rk_aiq_sys_ctx_t *sys_ctx,
                                          mDehazeAttr_t *manual) {
  adehaze_sw_V2_t adehaze_attr;

  memset(&adehaze_attr, 0, sizeof(adehaze_sw_V2_t));
  rk_aiq_user_api2_adehaze_getSwAttrib(sys_ctx, &adehaze_attr);
  memcpy(manual, &adehaze_attr.stManual, sizeof(mDehazeAttr_t));

  return XCAM_RETURN_NO_ERROR;
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

rk_aiq_sys_ctx_t* rk_aiq_get_last_sysctx(rk_aiq_sys_ctx_t *sys_ctx) {
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

  return NULL;
}
