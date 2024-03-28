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

#include "uAPI2/rk_aiq_user_api2_helper.h"
#include "uAPI/rk_aiq_api_private.h"
#include "RkAiqCalibDbV2.h"
#include "RkAiqUapitypes.h"
#include "cJSON.h"
#include "cJSON_Utils.h"
#include "j2s.h"
#include "stdlib.h"
#include "string.h"
#include "rk_aiq_user_api_sysctl.h"
#include "uAPI2/rk_aiq_user_api2_ae.h"
#include "uAPI2/rk_aiq_user_api2_imgproc.h"
#include "uAPI2/rk_aiq_user_api2_wrapper.h"
#include "uAPI2/rk_aiq_user_api2_agic.h"
#include "uAPI2/rk_aiq_user_api2_wrapper.h"

#ifdef ISP_HW_V39
#include "rk_aiq_user_api2_rk3576.h"
#elif  defined(ISP_HW_V32)
#include "rk_aiq_user_api2_rv1106.h"
#endif

#include "rk_aiq_tool_api.h"

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif

RKAIQ_BEGIN_DECLARE

// json cmd example {
//    "op":"replace",
//    "path":"/uapi/0/system/crop"
//    "value":"{
//      "x":0,
//      "y":0,
//      "w":1280,
//      "h":720
//    }"
// }
//
// json cmd example {
//    "op":"replace",
//    "path":"/uapi/0/ae_uapi/opmode"
//    "value":"{
//      1
//    }"
// }
//

/*****************Add UAPI wrapper here if necessary*****************/
__RKAIQUAPI_SET_WRAPPER(rk_aiq_user_api2_ae_setExpSwAttr, Uapi_ExpSwAttrV2_t);
__RKAIQUAPI_SET_WRAPPER(rk_aiq_user_api2_aie_SetAttrib, aie_attrib_t*);
__RKAIQUAPI_SET_WRAPPER(rk_aiq_user_api2_acp_SetAttrib, acp_attrib_t*);

/********************** Add Attr caller here ************************/
__RKAIQUAPI_CALLER(uapi_expsw_attr_t);
__RKAIQUAPI_CALLER(uapi_expinfo_t);
__RKAIQUAPI_CALLER(aiq_scene_t);
__RKAIQUAPI_CALLER(work_mode_t);
__RKAIQUAPI_CALLER(uapi_wb_gain_t);
__RKAIQUAPI_CALLER(uapi_wb_mode_t);
__RKAIQUAPI_CALLER(uapiMergeCurrCtlData_t);
#if RKAIQ_HAVE_MERGE_V10
__RKAIQUAPI_CALLER(mMergeAttrV10_t);
#endif
#if RKAIQ_HAVE_MERGE_V11
__RKAIQUAPI_CALLER(mMergeAttrV11_t);
#endif
#if RKAIQ_HAVE_MERGE_V12
__RKAIQUAPI_CALLER(mMergeAttrV12_t);
#endif
#if RKAIQ_HAVE_DRC_V10
__RKAIQUAPI_CALLER(DrcInfoV10_t);
__RKAIQUAPI_CALLER(mdrcAttr_V10_t);
#endif
#if RKAIQ_HAVE_DRC_V11
__RKAIQUAPI_CALLER(DrcInfoV11_t);
__RKAIQUAPI_CALLER(mdrcAttr_V11_t);
#endif
#if RKAIQ_HAVE_DRC_V12
__RKAIQUAPI_CALLER(DrcInfoV12_t);
__RKAIQUAPI_CALLER(mdrcAttr_V12_t);
#endif
#if RKAIQ_HAVE_DRC_V12_LITE
__RKAIQUAPI_CALLER(DrcInfoV12Lite_t);
__RKAIQUAPI_CALLER(mdrcAttr_v12_lite_t);
#endif
#if RKAIQ_HAVE_DRC_V20
__RKAIQUAPI_CALLER(DrcInfoV20_t);
__RKAIQUAPI_CALLER(mdrcAttr_V20_t);
#endif
#if RKAIQ_HAVE_GAMMA_V10
__RKAIQUAPI_CALLER(AgammaApiManualV10_t);
#endif
#if RKAIQ_HAVE_GAMMA_V11
__RKAIQUAPI_CALLER(AgammaApiManualV11_t);
#endif
__RKAIQUAPI_CALLER(mDehazeAttrInfoV11_t);
#if RKAIQ_HAVE_DEHAZE_V10
#endif
#if RKAIQ_HAVE_DEHAZE_V11
__RKAIQUAPI_CALLER(mDehazeAttrV11_t);
#endif
#if RKAIQ_HAVE_DEHAZE_V12
__RKAIQUAPI_CALLER(mDehazeAttrV12_t);
#endif
#if RKAIQ_HAVE_DEHAZE_V14
__RKAIQUAPI_CALLER(mDehazeAttrV14_t);
#endif
__RKAIQUAPI_CALLER(uapi_ae_hwstats_t);
#if defined(ISP_HW_V21)
__RKAIQUAPI_CALLER(rk_aiq_awb_stat_res2_v201_t);
#elif defined(ISP_HW_V30)
__RKAIQUAPI_CALLER(rk_tool_awb_stat_res2_v30_t);
#elif defined(ISP_HW_V39) || defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
__RKAIQUAPI_CALLER(rk_tool_isp_awb_stats_v32_t);
#endif
__RKAIQUAPI_CALLER(rk_tool_awb_stat_res_full_t);
__RKAIQUAPI_CALLER(rk_tool_awb_strategy_result_t);
__RKAIQUAPI_CALLER(rk_aiq_ccm_querry_info_t);
#if defined(ISP_HW_V21) || defined(ISP_HW_V30)
__RKAIQUAPI_CALLER(rk_aiq_ccm_mccm_attrib_t);
#elif defined(ISP_HW_V39) || defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
__RKAIQUAPI_CALLER(rk_aiq_ccm_mccm_attrib_v2_t);
#endif
__RKAIQUAPI_CALLER(Cgc_Param_t);
__RKAIQUAPI_CALLER(Csm_Param_t);
__RKAIQUAPI_CALLER(Adpcc_Manual_Attr_t);

#if defined(ISP_HW_V39) || defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE) || defined(ISP_HW_V30)
__RKAIQUAPI_CALLER(abayer2dnr_uapi_manual_t);
__RKAIQUAPI_CALLER(again_uapi_manual_t);
__RKAIQUAPI_CALLER(abayer2dnr_uapi_info_t);
__RKAIQUAPI_CALLER(again_uapi_info_t);
#endif
#if defined(ISP_HW_V39)
__RKAIQUAPI_CALLER(ayuvme_uapi_manual_t);
__RKAIQUAPI_CALLER(ayuvme_uapi_info_t);

#endif


__RKAIQUAPI_CALLER(rk_aiq_lut3d_mlut3d_attrib_t);
__RKAIQUAPI_CALLER(rkaiq_gic_v2_api_attr_t);
__RKAIQUAPI_CALLER(aie_attrib_t);
__RKAIQUAPI_CALLER(acp_attrib_t);
__RKAIQUAPI_CALLER(rk_aiq_lsc_table_t);
__RKAIQUAPI_CALLER(camgroup_uapi_t);
#if USE_NEWSTRUCT
__RKAIQUAPI_CALLER(dm_api_attrib_t);
__RKAIQUAPI_CALLER(dm_status_t);
__RKAIQUAPI_CALLER(btnr_api_attrib_t);
__RKAIQUAPI_CALLER(btnr_status_t);
__RKAIQUAPI_CALLER(gamma_api_attrib_t);
__RKAIQUAPI_CALLER(gamma_status_t);
__RKAIQUAPI_CALLER(drc_api_attrib_t);
__RKAIQUAPI_CALLER(drc_status_t);
__RKAIQUAPI_CALLER(dehaze_api_attrib_t);
__RKAIQUAPI_CALLER(dehaze_status_t);
__RKAIQUAPI_CALLER(ynr_api_attrib_t);
__RKAIQUAPI_CALLER(ynr_status_t);
__RKAIQUAPI_CALLER(cnr_api_attrib_t);
__RKAIQUAPI_CALLER(cnr_status_t);
__RKAIQUAPI_CALLER(sharp_api_attrib_t);
__RKAIQUAPI_CALLER(sharp_status_t);
__RKAIQUAPI_CALLER(blc_api_attrib_t);
__RKAIQUAPI_CALLER(blc_status_t);
__RKAIQUAPI_CALLER(trans_api_attrib_t);
__RKAIQUAPI_CALLER(trans_status_t);
__RKAIQUAPI_CALLER(dpc_api_attrib_t);
__RKAIQUAPI_CALLER(dpc_status_t);
#else
__RKAIQUAPI_CALLER(abayertnr_uapi_manual_t);
__RKAIQUAPI_CALLER(aynr_uapi_manual_t);
__RKAIQUAPI_CALLER(acnr_uapi_manual_t);
__RKAIQUAPI_CALLER(ablc_uapi_manual_t);
__RKAIQUAPI_CALLER(asharp_uapi_manual_t);
__RKAIQUAPI_CALLER(aynr_uapi_info_t);
__RKAIQUAPI_CALLER(acnr_uapi_info_t);
__RKAIQUAPI_CALLER(asharp_uapi_info_t);
__RKAIQUAPI_CALLER(adebayer_attrib_t);
__RKAIQUAPI_CALLER(ablc_uapi_info_t);
__RKAIQUAPI_CALLER(abayertnr_uapi_info_t);
#endif

RkAiqUapiDesc_t rkaiq_uapidesc_list[] = {
    __RKAIQUAPI_DESC_DEF("/uapi/0/ae_uapi/expsw_attr", uapi_expsw_attr_t,
                         __RKAIQUAPI_SET_WRAPPER_NAME(rk_aiq_user_api2_ae_setExpSwAttr),
                         rk_aiq_user_api2_ae_getExpSwAttr),
    __RKAIQUAPI_DESC_DEF("/uapi/0/ae_uapi/expinfo", uapi_expinfo_t, NULL,
                         rk_aiq_user_api2_ae_queryExpResInfo),
    __RKAIQUAPI_DESC_DEF("/uapi/0/awb_uapi/wbgain", uapi_wb_gain_t, rk_aiq_uapi2_setMWBGain,
                         rk_aiq_uapi2_getWBGain),
    __RKAIQUAPI_DESC_DEF("/uapi/0/awb_uapi/mode", uapi_wb_mode_t, rk_aiq_uapi2_setWBMode2,
                         rk_aiq_uapi2_getWBMode2),

#if RKAIQ_HAVE_MERGE_V10
    __RKAIQUAPI_DESC_DEF("/uapi/0/amerge_uapi/Info", uapiMergeCurrCtlData_t, NULL,
                         rk_aiq_user_api2_amerge_v10_queryinfo),
    __RKAIQUAPI_DESC_DEF("/uapi/0/amerge_uapi/stManual", mMergeAttrV10_t,
                         rk_aiq_set_amerge_v10_manual_attr, rk_aiq_get_amerge_v10_manual_attr),
#endif
#if RKAIQ_HAVE_MERGE_V11
    __RKAIQUAPI_DESC_DEF("/uapi/0/amerge_uapi/Info", uapiMergeCurrCtlData_t, NULL,
                         rk_aiq_user_api2_amerge_v11_queryinfo),
    __RKAIQUAPI_DESC_DEF("/uapi/0/amerge_uapi/stManual", mMergeAttrV11_t,
                         rk_aiq_set_amerge_v11_manual_attr, rk_aiq_get_amerge_v11_manual_attr),
#endif
#if RKAIQ_HAVE_MERGE_V12
    __RKAIQUAPI_DESC_DEF("/uapi/0/amerge_uapi/Info", uapiMergeCurrCtlData_t, NULL,
                         rk_aiq_user_api2_amerge_v12_queryinfo),
    __RKAIQUAPI_DESC_DEF("/uapi/0/amerge_uapi/stManual", mMergeAttrV12_t,
                         rk_aiq_set_amerge_v12_manual_attr, rk_aiq_get_amerge_v12_manual_attr),
#endif
#if RKAIQ_HAVE_DRC_V10
    __RKAIQUAPI_DESC_DEF("/uapi/0/adrc_uapi/Info", DrcInfoV10_t, NULL,
                         rk_aiq_user_api2_adrc_v10_queryinfo),
    __RKAIQUAPI_DESC_DEF("/uapi/0/adrc_uapi/stManual", mdrcAttr_V10_t,
                         rk_aiq_set_adrc_v10_manual_attr, rk_aiq_get_adrc_v10_manual_attr),
#endif
#if RKAIQ_HAVE_DRC_V11
    __RKAIQUAPI_DESC_DEF("/uapi/0/adrc_uapi/Info", DrcInfoV11_t, NULL,
                         rk_aiq_user_api2_adrc_v11_queryinfo),
    __RKAIQUAPI_DESC_DEF("/uapi/0/adrc_uapi/stManual", mdrcAttr_V11_t,
                         rk_aiq_set_adrc_v11_manual_attr, rk_aiq_get_adrc_v11_manual_attr),
#endif
#if RKAIQ_HAVE_DRC_V12  && (USE_NEWSTRUCT == 0)
    __RKAIQUAPI_DESC_DEF("/uapi/0/adrc_uapi/Info", DrcInfoV12_t, NULL,
                         rk_aiq_user_api2_adrc_v12_queryinfo),
    __RKAIQUAPI_DESC_DEF("/uapi/0/adrc_uapi/stManual", mdrcAttr_V12_t,
                         rk_aiq_set_adrc_v12_manual_attr, rk_aiq_get_adrc_v12_manual_attr),
#endif
#if RKAIQ_HAVE_DRC_V12_LITE
    __RKAIQUAPI_DESC_DEF("/uapi/0/adrc_uapi/Info", DrcInfoV12Lite_t, NULL,
                         rk_aiq_user_api2_adrc_v12_lite_queryinfo),
    __RKAIQUAPI_DESC_DEF("/uapi/0/adrc_uapi/stManual", mdrcAttr_v12_lite_t,
                         rk_aiq_set_adrc_v12_lite_manual_attr,
                         rk_aiq_get_adrc_v12_lite_manual_attr),
#endif
#if RKAIQ_HAVE_DRC_V20  && (USE_NEWSTRUCT == 0)
    __RKAIQUAPI_DESC_DEF("/uapi/0/adrc_uapi/Info", DrcInfoV20_t, NULL,
                         rk_aiq_user_api2_adrc_v20_queryinfo),
    __RKAIQUAPI_DESC_DEF("/uapi/0/adrc_uapi/stManual", mdrcAttr_V20_t,
                         rk_aiq_set_adrc_v20_manual_attr, rk_aiq_get_adrc_v20_manual_attr),
#endif
#if RKAIQ_HAVE_GAMMA_V10
    __RKAIQUAPI_DESC_DEF("/uapi/0/agamma_uapi/stManual", AgammaApiManualV10_t,
                         rk_aiq_set_agamma_v10_manual_attr, rk_aiq_get_agamma_v10_manual_attr),
#endif
#if RKAIQ_HAVE_GAMMA_V11 && (USE_NEWSTRUCT == 0)
    __RKAIQUAPI_DESC_DEF("/uapi/0/agamma_uapi/stManual", AgammaApiManualV11_t,
                         rk_aiq_set_agamma_v11_manual_attr, rk_aiq_get_agamma_v11_manual_attr),
#endif
#if RKAIQ_HAVE_DEHAZE_V10
#endif
#if RKAIQ_HAVE_DEHAZE_V11
    __RKAIQUAPI_DESC_DEF("/uapi/0/adehaze_uapi/Info", mDehazeAttrInfoV11_t, NULL,
                         rk_aiq_user_api2_adehaze_v11_queryinfo),
    __RKAIQUAPI_DESC_DEF("/uapi/0/adehaze_uapi/stManual", mDehazeAttrV11_t,
                         rk_aiq_set_adehaze_v11_manual_attr, rk_aiq_get_adehaze_v11_manual_attr),
#endif
#if RKAIQ_HAVE_DEHAZE_V12  && (USE_NEWSTRUCT == 0)
    __RKAIQUAPI_DESC_DEF("/uapi/0/adehaze_uapi/Info", mDehazeAttrInfoV11_t, NULL,
                         rk_aiq_user_api2_adehaze_v12_queryinfo),
    __RKAIQUAPI_DESC_DEF("/uapi/0/adehaze_uapi/stManual", mDehazeAttrV12_t,
                         rk_aiq_set_adehaze_v12_manual_attr, rk_aiq_get_adehaze_v12_manual_attr),
#endif
#if RKAIQ_HAVE_DEHAZE_V14  && (USE_NEWSTRUCT == 0)
    __RKAIQUAPI_DESC_DEF("/uapi/0/adehaze_uapi/Info", mDehazeAttrInfoV11_t, NULL,
                         rk_aiq_user_api2_adehaze_v14_queryinfo),
    __RKAIQUAPI_DESC_DEF("/uapi/0/adehaze_uapi/stManual", mDehazeAttrV14_t,
                         rk_aiq_set_adehaze_v14_manual_attr, rk_aiq_get_adehaze_v14_manual_attr),
#endif

    __RKAIQUAPI_DESC_DEF("/uapi/0/system/work_mode", work_mode_t,
                         rk_aiq_uapi_sysctl_swWorkingModeDyn2,
                         rk_aiq_uapi_sysctl_getWorkingModeDyn),
    __RKAIQUAPI_DESC_DEF("/uapi/0/system/scene", aiq_scene_t, rk_aiq_user_api2_set_scene,
                         rk_aiq_user_api2_get_scene),
    __RKAIQUAPI_DESC_DEF("/uapi/0/measure_info/ae_hwstats", uapi_ae_hwstats_t, NULL,
                         rk_aiq_uapi_get_ae_hwstats),
#if defined(ISP_HW_V21)
    __RKAIQUAPI_DESC_DEF("/uapi/0/measure_info/wb_log/info/awb_stat", rk_aiq_awb_stat_res2_v201_t,
                         NULL, rk_aiq_uapi_get_awbV21_stat),
#elif defined(ISP_HW_V30)
    __RKAIQUAPI_DESC_DEF("/uapi/0/measure_info/wb_log/info/awb_stat", rk_tool_awb_stat_res2_v30_t,
                         NULL, rk_aiq_uapi_get_awb_stat),
#elif defined(ISP_HW_V39) || defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
    __RKAIQUAPI_DESC_DEF("/uapi/0/measure_info/wb_log/info/awb_stat", rk_tool_isp_awb_stats_v32_t,
                         NULL, rk_aiq_uapi_get_awbV32_stat),
#endif
    __RKAIQUAPI_DESC_DEF("/uapi/0/measure_info/wb_log/info/awb_stat_algo",
                         rk_tool_awb_stat_res_full_t, NULL, rk_aiq_user_api2_awb_getAlgoSta),
    __RKAIQUAPI_DESC_DEF("/uapi/0/measure_info/wb_log/info/awb_strategy_result",
                         rk_tool_awb_strategy_result_t, NULL,
                         rk_aiq_user_api2_awb_getStrategyResult),
    __RKAIQUAPI_DESC_DEF("/uapi/0/accm_uapi/mode", uapi_wb_mode_t, rk_aiq_set_tool_accm_mode,
                         rk_aiq_get_accm_mode),
#if defined(ISP_HW_V21) || defined(ISP_HW_V30)
    __RKAIQUAPI_DESC_DEF("/uapi/0/accm_uapi/Info", rk_aiq_ccm_querry_info_t, NULL,
                         rk_aiq_user_api2_accm_QueryCcmInfo),
    __RKAIQUAPI_DESC_DEF("/uapi/0/accm_uapi/stManual", rk_aiq_ccm_mccm_attrib_t,
                         rk_aiq_set_accm_v1_manual_attr, rk_aiq_get_accm_v1_manual_attr),
#elif defined(ISP_HW_V39) || defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
    __RKAIQUAPI_DESC_DEF("/uapi/0/accm_uapi/Info", rk_aiq_ccm_querry_info_t, NULL,
                         rk_aiq_user_api2_accm_QueryCcmInfo),
    __RKAIQUAPI_DESC_DEF("/uapi/0/accm_uapi/stManual", rk_aiq_ccm_mccm_attrib_v2_t,
                         rk_aiq_set_accm_v2_manual_attr, rk_aiq_get_accm_v2_manual_attr),
#endif
    __RKAIQUAPI_DESC_DEF("/uapi/0/acgc_uapi/manual", Cgc_Param_t, rk_aiq_set_acgc_manual_attr,
                         rk_aiq_get_acgc_manual_attr),
    __RKAIQUAPI_DESC_DEF("/uapi/0/acsm_uapi/manual", Csm_Param_t, rk_aiq_set_acsm_manual_attr,
                         rk_aiq_get_acsm_manual_attr),
    __RKAIQUAPI_DESC_DEF("/uapi/0/a3dlut_uapi/mode", uapi_wb_mode_t, rk_aiq_set_tool_3dlut_mode,
                         rk_aiq_get_a3dlut_mode),
    __RKAIQUAPI_DESC_DEF("/uapi/0/a3dlut_uapi/manual", rk_aiq_lut3d_mlut3d_attrib_t,
                         rk_aiq_set_a3dlut_manual_attr, rk_aiq_get_a3dlut_manual_attr),

#if defined(ISP_HW_V39) || defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
    __RKAIQUAPI_DESC_DEF("/uapi/0/abayer2dnr_uapi/manual", abayer2dnr_uapi_manual_t,
                         rk_aiq_set_abayer2dnr_manual_attr, rk_aiq_get_abayer2dnr_manual_attr),
    __RKAIQUAPI_DESC_DEF("/uapi/0/again_uapi/manual", again_uapi_manual_t,
                         rk_aiq_set_again_manual_attr, rk_aiq_get_again_manual_attr),

#if (USE_NEWSTRUCT == 0)
    __RKAIQUAPI_DESC_DEF("/uapi/0/asharp_uapi/manual", asharp_uapi_manual_t,
                         rk_aiq_set_asharp_manual_attr, rk_aiq_get_asharp_manual_attr),
    __RKAIQUAPI_DESC_DEF("/uapi/0/ablc_uapi/manual", ablc_uapi_manual_t,
                         rk_aiq_set_ablc_manual_attr, rk_aiq_get_ablc_manual_attr),
    __RKAIQUAPI_DESC_DEF("/uapi/0/ablc_uapi/info", ablc_uapi_info_t, NULL, rk_aiq_get_ablc_info),
    __RKAIQUAPI_DESC_DEF("/uapi/0/abayertnr_uapi/manual", abayertnr_uapi_manual_t,
                         rk_aiq_set_abayertnr_manual_attr, rk_aiq_get_abayertnr_manual_attr),
    __RKAIQUAPI_DESC_DEF("/uapi/0/aynr_uapi/manual", aynr_uapi_manual_t,
                         rk_aiq_set_aynr_manual_attr, rk_aiq_get_aynr_manual_attr),
    __RKAIQUAPI_DESC_DEF("/uapi/0/acnr_uapi/manual", acnr_uapi_manual_t,
                         rk_aiq_set_acnr_manual_attr, rk_aiq_get_acnr_manual_attr),
    __RKAIQUAPI_DESC_DEF("/uapi/0/abayertnr_uapi/info", abayertnr_uapi_info_t, NULL,
                         rk_aiq_get_abayertnr_info),
    __RKAIQUAPI_DESC_DEF("/uapi/0/aynr_uapi/info", aynr_uapi_info_t, NULL, rk_aiq_get_aynr_info),

    __RKAIQUAPI_DESC_DEF("/uapi/0/acnr_uapi/info", acnr_uapi_info_t, NULL, rk_aiq_get_acnr_info),

    __RKAIQUAPI_DESC_DEF("/uapi/0/asharp_uapi/info", asharp_uapi_info_t, NULL,
                         rk_aiq_get_asharp_info),
#endif

    __RKAIQUAPI_DESC_DEF("/uapi/0/abayer2dnr_uapi/info", abayer2dnr_uapi_info_t, NULL,
                         rk_aiq_get_abayer2dnr_info),


    __RKAIQUAPI_DESC_DEF("/uapi/0/again_uapi/info", again_uapi_info_t, NULL, rk_aiq_get_again_info),

#endif

#if defined(ISP_HW_V39)
    __RKAIQUAPI_DESC_DEF("/uapi/0/ayuvme_uapi/manual", ayuvme_uapi_manual_t,
                         rk_aiq_set_ayuvme_manual_attr, rk_aiq_get_ayuvme_manual_attr),
    __RKAIQUAPI_DESC_DEF("/uapi/0/ayuvme_uapi/info", ayuvme_uapi_info_t, NULL, rk_aiq_get_ayuvme_info),
#endif


#if USE_NEWSTRUCT
    __RKAIQUAPI_DESC_DEF("/uapi/0/dm_uapi/attr", dm_api_attrib_t, rk_aiq_user_api2_dm_SetAttrib, rk_aiq_user_api2_dm_GetAttrib),
    __RKAIQUAPI_DESC_DEF("/uapi/0/dm_uapi/info", dm_status_t, NULL, rk_aiq_user_api2_dm_QueryStatus),
    __RKAIQUAPI_DESC_DEF("/uapi/0/btnr_uapi/attr", btnr_api_attrib_t, rk_aiq_user_api2_btnr_SetAttrib, rk_aiq_user_api2_btnr_GetAttrib),
    __RKAIQUAPI_DESC_DEF("/uapi/0/btnr_uapi/info", btnr_status_t, NULL, rk_aiq_user_api2_btnr_QueryStatus),
    __RKAIQUAPI_DESC_DEF("/uapi/0/gamma_uapi/attr", gamma_api_attrib_t, rk_aiq_user_api2_gamma_SetAttrib, rk_aiq_user_api2_gamma_GetAttrib),
    __RKAIQUAPI_DESC_DEF("/uapi/0/gamma_uapi/info", gamma_status_t, NULL, rk_aiq_user_api2_gamma_QueryStatus),
    __RKAIQUAPI_DESC_DEF("/uapi/0/drc_uapi/attr", drc_api_attrib_t, rk_aiq_user_api2_drc_SetAttrib, rk_aiq_user_api2_drc_GetAttrib),
    __RKAIQUAPI_DESC_DEF("/uapi/0/drc_uapi/info", drc_status_t, NULL, rk_aiq_user_api2_drc_QueryStatus),
    __RKAIQUAPI_DESC_DEF("/uapi/0/dehaze_uapi/attr", dehaze_api_attrib_t, rk_aiq_user_api2_dehaze_SetAttrib, rk_aiq_user_api2_dehaze_GetAttrib),
    __RKAIQUAPI_DESC_DEF("/uapi/0/dehaze_uapi/info", dehaze_status_t, NULL, rk_aiq_user_api2_dehaze_QueryStatus),
    __RKAIQUAPI_DESC_DEF("/uapi/0/ynr_uapi/attr", ynr_api_attrib_t, rk_aiq_user_api2_ynr_SetAttrib, rk_aiq_user_api2_ynr_GetAttrib),
    __RKAIQUAPI_DESC_DEF("/uapi/0/ynr_uapi/info", ynr_status_t, NULL, rk_aiq_user_api2_ynr_QueryStatus),
    __RKAIQUAPI_DESC_DEF("/uapi/0/sharp_uapi/attr", sharp_api_attrib_t, rk_aiq_user_api2_sharp_SetAttrib, rk_aiq_user_api2_sharp_GetAttrib),
    __RKAIQUAPI_DESC_DEF("/uapi/0/sharp_uapi/info", sharp_status_t, NULL, rk_aiq_user_api2_sharp_QueryStatus),
    __RKAIQUAPI_DESC_DEF("/uapi/0/cnr_uapi/attr", cnr_api_attrib_t, rk_aiq_user_api2_cnr_SetAttrib, rk_aiq_user_api2_cnr_GetAttrib),
    __RKAIQUAPI_DESC_DEF("/uapi/0/cnr_uapi/info", cnr_status_t, NULL, rk_aiq_user_api2_cnr_QueryStatus),
    __RKAIQUAPI_DESC_DEF("/uapi/0/blc_uapi/attr", blc_api_attrib_t, rk_aiq_user_api2_blc_SetAttrib, rk_aiq_user_api2_blc_GetAttrib),
    __RKAIQUAPI_DESC_DEF("/uapi/0/blc_uapi/info", blc_status_t, NULL, rk_aiq_user_api2_blc_QueryStatus),
    __RKAIQUAPI_DESC_DEF("/uapi/0/trans_uapi/attr", trans_api_attrib_t, rk_aiq_user_api2_trans_SetAttrib, rk_aiq_user_api2_trans_GetAttrib),
    __RKAIQUAPI_DESC_DEF("/uapi/0/trans_uapi/info", trans_status_t, NULL, rk_aiq_user_api2_trans_QueryStatus),
    __RKAIQUAPI_DESC_DEF("/uapi/0/dpc_uapi/attr", dpc_api_attrib_t, rk_aiq_user_api2_dpc_SetAttrib, rk_aiq_user_api2_dpc_GetAttrib),
    __RKAIQUAPI_DESC_DEF("/uapi/0/dpc_uapi/info", dpc_status_t, NULL, rk_aiq_user_api2_dpc_QueryStatus),
#endif
};
/***********************END OF CUSTOM AREA**************************/

char* rkaiq_uapi_rpc_response(const char* cmd_path, cJSON* root_js,
    const char* sub_node) {
    char* ret_str = NULL;
    cJSON* ret_json = NULL;
    cJSON* node_json = NULL;
    cJSON* node_json_clone = NULL;
    ret_json = cJSON_CreateArray();

    if (!root_js || !sub_node) {
        XCAM_LOG_ERROR("invalid json argument for sysctl!");
        return NULL;
    }

    if (0 == strcmp(sub_node, "/")) {
        node_json = root_js;
    }
    else {
        node_json = cJSONUtils_GetPointer(root_js, sub_node);
    }

    node_json_clone = cJSON_Duplicate(node_json, 1);

    if (root_js) {
        cJSON* ret_item = cJSON_CreateObject();
        cJSON_AddStringToObject(ret_item, JSON_PATCH_PATH, cmd_path);
        cJSON_AddItemToObject(ret_item, JSON_PATCH_VALUE, node_json_clone);
        cJSON_AddItemToArray(ret_json, ret_item);
    }

    ret_str = cJSON_Print(ret_json);

    if (ret_json)
        cJSON_Delete(ret_json);

    return ret_str;
}

int rkaiq_uapi_best_match(const char* cmd_path_str) {
    int i = 0;
    int list_len = -1;
    int beset_match = -1;
    int max_length = -1;

    list_len = sizeof(rkaiq_uapidesc_list) / sizeof(RkAiqUapiDesc_t);
    if (list_len <= 0) {
        return -1;
    }

    // Find most match uapi
    for (i = 0; i < list_len; i++) {
        RkAiqUapiDesc_t* temp_uapi_desc = &rkaiq_uapidesc_list[i];
        if (strstr(cmd_path_str, temp_uapi_desc->arg_path)) {
            int path_length = std::string(temp_uapi_desc->arg_path).length();
            if (path_length < max_length) {
                continue;
            }
            max_length = path_length;
            beset_match = i;
        }
    }

    return beset_match;
}

int rkaiq_uapi_unified_ctl(rk_aiq_sys_ctx_t* sys_ctx, const char* js_str,
    char** ret_str, int op_mode) {
    RkAiqUapiDesc_t* uapi_desc = NULL;
    std::string cmd_path_str;
    std::string final_path = "/";
    cJSON* cmd_js = NULL;
    cJSON* ret_js = NULL;
    cJSON* arr_item = NULL;
    int list_len = -1;
    int change_sum = -1;
    int i = 0;
    rk_aiq_sys_ctx_t* msys_ctx = NULL;
    *ret_str = NULL;

    list_len = sizeof(rkaiq_uapidesc_list) / sizeof(RkAiqUapiDesc_t);
    if (list_len <= 0) {
        return -1;
    }

    cmd_js = cJSON_Parse(js_str);
    change_sum = cJSON_GetArraySize(cmd_js);

    if (change_sum <= 0) {
        XCAM_LOG_ERROR("can't find json patch operation\n");
        return -1;
    }

    arr_item = cmd_js->child;

    for (i = 0; i <= (change_sum - 1); ++i) {
        if (arr_item) {
            if (cJSON_GetObjectItem(arr_item, JSON_PATCH_PATH)->valuestring) {
                cmd_path_str = std::string(
                    cJSON_GetObjectItem(arr_item, JSON_PATCH_PATH)->valuestring);
                int desc_i = rkaiq_uapi_best_match(cmd_path_str.c_str());
                if (desc_i >= 0) {
                    uapi_desc = &rkaiq_uapidesc_list[desc_i];
                    if (0 == std::string(uapi_desc->arg_path).compare(cmd_path_str)) {
                        /* FIX bug: root path is null in cJSON_Utils.cpp:apply_patch, not the "/" */
                        //final_path = "/";
                        final_path = "";
                    }
                    else {
                        final_path = cmd_path_str.substr(
                            std::string(uapi_desc->arg_path).length());
                    }
                    cJSON_ReplaceItemInObject(arr_item, JSON_PATCH_PATH,
                        cJSON_CreateString(final_path.c_str()));
                }
            }
        }
        arr_item = arr_item->next;
    }

    if (!uapi_desc) {
        XCAM_LOG_ERROR("can't find uapi for %s\n", cmd_path_str.c_str());
        return -1;
    }

#if RKAIQ_ENABLE_CAMGROUP
    msys_ctx = rk_aiq_get_last_sysctx(sys_ctx);
    if (!msys_ctx) {
        XCAM_LOG_ERROR("can't find valid ctx\n");
        return -1;
    }
#else
    msys_ctx = sys_ctx;
#endif

    uapi_desc->uapi_caller(uapi_desc, msys_ctx, cmd_js,
        (void**)&ret_js, op_mode);

    if (op_mode == RKAIQUAPI_OPMODE_SET) {
        *ret_str = NULL;
    }
    else if (op_mode == RKAIQUAPI_OPMODE_GET) {
        if (ret_js) {
            *ret_str = rkaiq_uapi_rpc_response(cmd_path_str.c_str(), ret_js,
                final_path.c_str());
            cJSON_Delete(ret_js);
        }
    }

    cJSON_Delete(cmd_js);

    return 0;
    }

RKAIQ_END_DECLARE

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif
