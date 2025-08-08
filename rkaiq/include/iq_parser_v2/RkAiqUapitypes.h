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

#ifndef ___RK_AIQ_UAPITYPES_H__
#define ___RK_AIQ_UAPITYPES_H__

#include "iq_parser_v2/adehaze_uapi_head.h"
#include "iq_parser_v2/adrc_uapi_head.h"
#include "iq_parser_v2/aec_uapi_head.h"
#include "iq_parser_v2/agamma_uapi_head.h"
#include "iq_parser_v2/amerge_uapi_head.h"
#include "iq_parser_v2/atmo_uapi_head.h"
#include "iq_parser_v2/awb_uapi_head.h"
#include "uAPI2/rk_aiq_user_api_common.h"
#include "algos/adpcc/rk_aiq_types_adpcc_ext.h"

#include "algos/a3dlut/rk_aiq_types_a3dlut_ext.h"
#if defined(ISP_HW_V32_LITE) ||  defined(ISP_HW_V39) || defined(ISP_HW_V32) || defined(ISP_HW_V30)
#include "iq_parser_v2/gain_uapi_head_v2.h"
#endif
#include "iq_parser_v2/aie_head.h"
#include "iq_parser_v2/ccm_head.h"
#include "iq_parser_v2/ccm_uapi_head.h"
#include "iq_parser_v2/csm_head.h"
#include "iq_parser_v2/cgc_head.h"
#include "iq_parser_v2/agic_uapi_head.h"
#include "iq_parser_v2/aie_uapi_head.h"
#include "iq_parser_v2/acp_uapi_head.h"
#include "iq_parser_v2/alsc_uapi_head.h"

#if defined(ISP_HW_V39) || defined(ISP_HW_V32) || defined(ISP_HW_V30)
#include "iq_parser_v2/gain_uapi_head_v2.h"
#endif

#if defined(ISP_HW_V39)
#include "iq_parser_v2/bayertnr_uapi_head_v30.h"
#include "iq_parser_v2/ynr_uapi_head_v24.h"
#include "iq_parser_v2/cnr_uapi_head_v31.h"
#include "iq_parser_v2/sharp_uapi_head_v34.h"
#include "iq_parser_v2/gain_uapi_head_v2.h"
#include "iq_parser_v2/ablc_uapi_head_v32.h"
#include "iq_parser_v2/yuvme_uapi_head_v1.h"
#endif

#if defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
#include "iq_parser_v2/bayer2dnr_uapi_head_v23.h"
#include "iq_parser_v2/bayertnr_uapi_head_v23.h"
#include "iq_parser_v2/ynr_uapi_head_v22.h"
#include "iq_parser_v2/cnr_uapi_head_v30.h"
#include "iq_parser_v2/sharp_uapi_head_v33.h"
#include "iq_parser_v2/gain_uapi_head_v2.h"
#include "iq_parser_v2/ablc_uapi_head_v32.h"
#endif

#if defined(ISP_HW_V30)
#include "iq_parser_v2/bayer2dnr_uapi_head_v2.h"
#include "iq_parser_v2/bayertnr_uapi_head_v2.h"
#include "iq_parser_v2/ynr_uapi_head_v3.h"
#include "iq_parser_v2/cnr_uapi_head_v2.h"
#include "iq_parser_v2/sharp_uapi_head_v4.h"
#include "iq_parser_v2/ablc_uapi_head.h"
#endif

#if defined(ISP_HW_V33) || defined(ISP_HW_V35)
#include "iq_parser_v2/rkpostisp_head_v2.h"
#endif

#ifndef ISP_HW_V33
#include "iq_parser_v2/af_uapi_head.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if USE_NEWSTRUCT
typedef struct __ae_uapi {
    /* M4_GENERIC_DESC(
       M4_ALIAS(attr),
       M4_TYPE(struct),
       M4_UI_MODULE(tuning_param),
       M4_REF(/ae_calib)) */
    ae_param_t attr;
    // M4_STRUCT_DESC("expswAttr", "normal_ui_style")
    ae_api_expSwAttr_t expSwAttr;
    // M4_STRUCT_DESC("queryExpInfo", "normal_ui_style")
    ae_queryInfo_t queryExpInfo;
} ae_uapi_t;

typedef struct {
    /* M4_GENERIC_DESC(
       M4_ALIAS(attr),
       M4_TYPE(struct),
       M4_UI_MODULE(tuning_param),
       M4_REF(/demosaic)) */
    dm_api_attrib_t attr;
    // M4_STRUCT_DESC("info", "tuning_status")
    dm_status_t info;
} dm_uapi_t;

typedef struct {
    /* M4_GENERIC_DESC(
       M4_ALIAS(attr),
       M4_TYPE(struct),
       M4_UI_MODULE(tuning_param),
       M4_REF(/bayertnr)) */
    btnr_api_attrib_t attr;
    // M4_STRUCT_DESC("info", "tuning_status")
    btnr_status_t info;
} btnr_uapi_t;

typedef struct {
    /* M4_GENERIC_DESC(
       M4_ALIAS(attr),
       M4_TYPE(struct),
       M4_UI_MODULE(tuning_param),
       M4_REF(/bayertnr2)) */
    btnr_api_attrib_t attr;
    // M4_STRUCT_DESC("info", "tuning_status")
    btnr_status_t info;
} btnr2_uapi_t;

typedef struct {
    /* M4_GENERIC_DESC(
       M4_ALIAS(attr),
       M4_TYPE(struct),
       M4_UI_MODULE(tuning_param),
       M4_REF(/gamma)) */
    gamma_api_attrib_t attr;
    // M4_STRUCT_DESC("info", "tuning_status")
    gamma_status_t info;
} gamma_uapi_t;

typedef struct {
    /* M4_GENERIC_DESC(
       M4_ALIAS(attr),
       M4_TYPE(struct),
       M4_UI_MODULE(tuning_param),
       M4_REF(/drc)) */
    drc_api_attrib_t attr;
    // M4_STRUCT_DESC("info", "tuning_status")
    drc_status_t info;
} drc_uapi_t;

typedef struct {
    /* M4_GENERIC_DESC(
       M4_ALIAS(attr),
       M4_TYPE(struct),
       M4_UI_MODULE(tuning_param),
       M4_REF(/trans)) */
    trans_api_attrib_t attr;
    // M4_STRUCT_DESC("info", "tuning_status")
    trans_status_t info;
} trans_uapi_t;

#if defined(ISP_HW_V39)
typedef struct {
    /* M4_GENERIC_DESC(
       M4_ALIAS(attr),
       M4_TYPE(struct),
       M4_UI_MODULE(tuning_param),
       M4_REF(/dhzEhz)) */
    dehaze_api_attrib_t attr;
    // M4_STRUCT_DESC("info", "tuning_status")
    dehaze_status_t info;
} dehaze_uapi_t;
#endif

typedef struct {
    /* M4_GENERIC_DESC(
       M4_ALIAS(attr),
       M4_TYPE(struct),
       M4_UI_MODULE(tuning_param),
       M4_REF(/histEQ)) */
    histeq_api_attrib_t attr;
    // M4_STRUCT_DESC("info", "tuning_status")
    histeq_status_t info;
} histeq_uapi_t;

#if defined(ISP_HW_V33) || defined(ISP_HW_V35)
typedef struct {
    /* M4_GENERIC_DESC(
       M4_ALIAS(attr),
       M4_TYPE(struct),
       M4_UI_MODULE(tuning_param),
       M4_REF(/enh)) */
    enh_api_attrib_t attr;
    // M4_STRUCT_DESC("info", "tuning_status")
    enh_status_t info;
} enh_uapi_t;
#endif

#if defined(ISP_HW_V33) || defined(ISP_HW_V35)
typedef struct {
    /* M4_GENERIC_DESC(
       M4_ALIAS(attr),
       M4_TYPE(struct),
       M4_UI_MODULE(tuning_param),
       M4_REF(/texEst)) */
    texEst_api_attrib_t attr;
    // M4_STRUCT_DESC("info", "tuning_status")
    texEst_status_t info;
} texEst_uapi_t;
#endif

#if defined(ISP_HW_V33) || defined(ISP_HW_V35)
typedef struct {
    /* M4_GENERIC_DESC(
       M4_ALIAS(attr),
       M4_TYPE(struct),
       M4_UI_MODULE(tuning_param),
       M4_REF(/postisp)) */
    postisp_api_attrib_t attr;
    // M4_STRUCT_DESC("info", "tuning_status")
    postisp_status_t info;
} postisp_uapi_t;
#endif

typedef struct {
    /* M4_GENERIC_DESC(
       M4_ALIAS(attr),
       M4_TYPE(struct),
       M4_UI_MODULE(tuning_param),
       M4_REF(/ynr)) */
    ynr_api_attrib_t attr;
    // M4_STRUCT_DESC("info", "tuning_status")
    ynr_status_t info;
} ynr_uapi_t;

typedef struct {
    /* M4_GENERIC_DESC(
       M4_ALIAS(attr),
       M4_TYPE(struct),
       M4_UI_MODULE(tuning_param),
       M4_REF(/cnr)) */
    cnr_api_attrib_t attr;
    // M4_STRUCT_DESC("info", "tuning_status")
    cnr_status_t info;
} cnr_uapi_t;

typedef struct {
    /* M4_GENERIC_DESC(
       M4_ALIAS(attr),
       M4_TYPE(struct),
       M4_UI_MODULE(tuning_param),
       M4_REF(/sharp)) */
    sharp_api_attrib_t attr;
    // M4_STRUCT_DESC("info", "tuning_status")
    sharp_status_t info;
} sharp_uapi_t;

typedef struct {
    /* M4_GENERIC_DESC(
       M4_ALIAS(attr),
       M4_TYPE(struct),
       M4_UI_MODULE(tuning_param),
       M4_REF(/blc)) */
    blc_api_attrib_t attr;
    // M4_STRUCT_DESC("info", "tuning_status")
    blc_status_t info;
} blc_uapi_t;

typedef struct {
    /* M4_GENERIC_DESC(
       M4_ALIAS(attr),
       M4_TYPE(struct),
       M4_UI_MODULE(tuning_param),
       M4_REF(/dpc)) */
    dpc_api_attrib_t attr;
    // M4_STRUCT_DESC("info", "tuning_status")
    dpc_status_t info;
} dpc_uapi_t;

typedef struct {
    /* M4_GENERIC_DESC(
       M4_ALIAS(attr),
       M4_TYPE(struct),
       M4_UI_MODULE(tuning_param),
       M4_REF(/gic)) */
    gic_api_attrib_t attr;
    // M4_STRUCT_DESC("info", "tuning_status")
    gic_status_t info;
} gic_uapi_t;

typedef struct {
    /* M4_GENERIC_DESC(
       M4_ALIAS(attr),
       M4_TYPE(struct),
       M4_UI_MODULE(tuning_param),
       M4_REF(/cac)) */
    cac_api_attrib_t attr;
    // M4_STRUCT_DESC("info", "tuning_status")
    cac_status_t info;
} cac_uapi_t;

typedef struct {
    /* M4_GENERIC_DESC(
       M4_ALIAS(attr),
       M4_TYPE(struct),
       M4_UI_MODULE(tuning_param),
       M4_REF(/ldc)) */
    ldc_api_attrib_t attr;
    // M4_STRUCT_DESC("info", "tuning_status")
    ldc_status_t info;
} ldc_uapi_t;

typedef struct {
    /* M4_GENERIC_DESC(
       M4_ALIAS(attr),
       M4_TYPE(struct),
       M4_UI_MODULE(tuning_param),
       M4_REF(/csm)) */
    csm_api_attrib_t attr;
    // M4_STRUCT_DESC("info", "tuning_status")
    csm_status_t info;
} csm_uapi_t;

typedef struct {
    /* M4_GENERIC_DESC(
       M4_ALIAS(attr),
       M4_TYPE(struct),
       M4_UI_MODULE(tuning_param),
       M4_REF(/mge)) */
    mge_api_attrib_t attr;
    // M4_STRUCT_DESC("info", "tuning_status")
    mge_status_t info;
} merge_uapi_t;

typedef struct {
    // M4_STRUCT_DESC("info", "normal_ui_style")
    rk_aiq_version_info_t ver_info;
} ver_uapi_t;

typedef struct {
    // M4_STRUCT_DESC("module_ctl", "normal_ui_style")
    rk_aiq_module_list_t module_ctl;
} module_ctl_uapi_t;

#if defined(ISP_HW_V39) || defined(ISP_HW_V35)
typedef struct {
    /* M4_GENERIC_DESC(
       M4_ALIAS(attr),
       M4_TYPE(struct),
       M4_UI_MODULE(tuning_param),
       M4_REF(/rgbir)) */
    rgbir_api_attrib_t attr;
    // M4_STRUCT_DESC("info", "tuning_status")
    rgbir_status_t info;
} rgbir_uapi_t;
#endif

typedef struct {
    /* M4_GENERIC_DESC(
       M4_ALIAS(attr),
       M4_TYPE(struct),
       M4_UI_MODULE(tuning_param),
       M4_REF(/cgc)) */
    cgc_api_attrib_t attr;
    // M4_STRUCT_DESC("info", "tuning_status")
    cgc_status_t info;
} cgc_uapi_t;

typedef struct {
    /* M4_GENERIC_DESC(
       M4_ALIAS(attr),
       M4_TYPE(struct),
       M4_UI_MODULE(tuning_param),
       M4_REF(/cp)) */
    cp_api_attrib_t attr;
    // M4_STRUCT_DESC("info", "tuning_status")
    cp_status_t info;
} cp_uapi_t;

typedef struct {
    /* M4_GENERIC_DESC(
       M4_ALIAS(attr),
       M4_TYPE(struct),
       M4_UI_MODULE(tuning_param),
       M4_REF(/ie)) */
    ie_api_attrib_t attr;
    // M4_STRUCT_DESC("info", "tuning_status")
    ie_status_t info;
} ie_uapi_t;

typedef struct {
    /* M4_GENERIC_DESC(
       M4_ALIAS(attr),
       M4_TYPE(struct),
       M4_UI_MODULE(tuning_param),
       M4_REF(/gain)) */
    gain_api_attrib_t attr;
    // M4_STRUCT_DESC("info", "tuning_status")
    gain_status_t info;
} gain_uapi_t;

#if defined(ISP_HW_V39)
typedef struct {
    /* M4_GENERIC_DESC(
       M4_ALIAS(attr),
       M4_TYPE(struct),
       M4_UI_MODULE(tuning_param),
       M4_REF(/lut3d/tunning)) */
    lut3d_api_attrib_t attr;
    /* M4_GENERIC_DESC(
       M4_ALIAS(calibdb),
       M4_TYPE(struct),
       M4_UI_MODULE(calib_param),
       M4_REF(/lut3d/calibdb)) */
    alut3d_lut3dCalib_t calibdb;
    // M4_STRUCT_DESC("info", "tuning_status")
    lut3d_status_t info;
} lut3d_uapi_t;
#endif

#if defined(ISP_HW_V33) || defined(ISP_HW_V35)
typedef struct {
    /* M4_GENERIC_DESC(
       M4_ALIAS(attr),
       M4_TYPE(struct),
       M4_UI_MODULE(tuning_param),
       M4_REF(/hsv/tunning)) */
    hsv_api_attrib_t attr;
    /* M4_GENERIC_DESC(
       M4_ALIAS(calibdb),
       M4_TYPE(struct),
       M4_UI_MODULE(calib_param),
       M4_REF(/hsv/calibdb)) */
    ahsv_hsvCalib_t calibdb;
    // M4_STRUCT_DESC("info", "tuning_status")
    hsv_status_t info;
} hsv_uapi_t;
#endif

typedef struct {
    /* M4_GENERIC_DESC(
       M4_ALIAS(attr),
       M4_TYPE(struct),
       M4_UI_MODULE(tuning_param),
       M4_REF(/lsc/tunning)) */
    lsc_api_attrib_t attr;
    /* M4_GENERIC_DESC(
       M4_ALIAS(calibdb),
       M4_TYPE(struct),
       M4_UI_MODULE(calib_param),
       M4_REF(/lsc/calibdb)) */
    alsc_lscCalib_t calibdb;
    // M4_STRUCT_DESC("info", "tuning_status")
    lsc_status_t info;
} lsc_uapi_t;

typedef struct {
    /* M4_GENERIC_DESC(
       M4_ALIAS(attr),
       M4_TYPE(struct),
       M4_UI_MODULE(tuning_param),
       M4_REF(/ccm/tunning)) */
    ccm_api_attrib_t attr;
    /* M4_GENERIC_DESC(
       M4_ALIAS(calibdb),
       M4_TYPE(struct),
       M4_UI_MODULE(calib_param),
       M4_REF(/ccm/calibdb)) */
    accm_ccmCalib_t calibdb;
    // M4_STRUCT_DESC("info", "tuning_status")
    ccm_status_t info;
} ccm_uapi_t;
#endif

#ifndef USE_NEWSTRUCT
typedef struct __ae_uapi {
    // M4_STRUCT_DESC("expsw_attr", "normal_ui_style")
    uapi_expsw_attr_t expsw_attr;
    // M4_STRUCT_DESC("QueryExpInfo", "normal_ui_style")
    uapi_expinfo_t expinfo;
} ae_uapi_t;
#endif

typedef struct __awb_uapi {
#ifdef USE_NEWSTRUCT
    /* M4_GENERIC_DESC(
       M4_ALIAS(attr),
       M4_TYPE(struct),
       M4_UI_MODULE(tuning_param),
       M4_REF(/wb)) */
    awb_api_attrib_t attr;
    // M4_STRUCT_DESC("mode", "normal_ui_style")
    uapi_wb_mode_t mode;
    // M4_STRUCT_DESC("wbgain", "normal_ui_style")
    uapi_wb_gain_t wbgain;
#endif
} awb_uapi_t;

typedef struct __amerge_uapi {
    // M4_STRUCT_DESC("Info", "normal_ui_style")
    uapiMergeCurrCtlData_t Info;
#if defined(ISP_HW_V21)
    // M4_STRUCT_DESC("stManual", "normal_ui_style")
    mMergeAttrV10_t stManual;
#endif
#if defined(ISP_HW_V30)
    // M4_STRUCT_DESC("stManual", "normal_ui_style")
    mMergeAttrV11_t stManual;
#endif
#if defined(ISP_HW_V39) || defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
    // M4_STRUCT_DESC("stManual", "normal_ui_style")
    mMergeAttrV12_t stManual;
#endif
} amerge_uapi_t;

typedef struct __atmo_uapi {
    // M4_STRUCT_DESC("ctldata", "normal_ui_style")
    uapiTmoCurrCtlData_t ctldata;
} atmo_uapi_t;

typedef struct __adrc_uapi {
#if defined(ISP_HW_V21)
    // M4_STRUCT_DESC("Info", "normal_ui_style")
    DrcInfoV10_t Info;
    // M4_STRUCT_DESC("stManual", "normal_ui_style")
    mdrcAttr_V10_t stManual;
#endif
#if defined(ISP_HW_V30)
    // M4_STRUCT_DESC("Info", "normal_ui_style")
    DrcInfoV11_t Info;
    // M4_STRUCT_DESC("stManual", "normal_ui_style")
    mdrcAttr_V11_t stManual;
#endif
#if defined(ISP_HW_V39) || defined(ISP_HW_V32)
    // M4_STRUCT_DESC("Info", "normal_ui_style")
    DrcInfoV12_t Info;
    // M4_STRUCT_DESC("stManual", "normal_ui_style")
    mdrcAttr_V12_t stManual;
#endif
#if defined(ISP_HW_V32_LITE)
    // M4_STRUCT_DESC("Info", "normal_ui_style")
    DrcInfoV12Lite_t Info;
    // M4_STRUCT_DESC("stManual", "normal_ui_style")
    mdrcAttr_v12_lite_t stManual;
#endif
} adrc_uapi_t;

typedef struct __agamma_uapi {
#if defined(ISP_HW_V20) || defined(ISP_HW_V21)
    // M4_STRUCT_DESC("stManual", "normal_ui_style")
    AgammaApiManualV10_t stManual;
#else
    // M4_STRUCT_DESC("stManual", "normal_ui_style")
    AgammaApiManualV11_t stManual;
#endif
} agamma_uapi_t;

typedef struct __adehaze_uapi {
    // M4_STRUCT_DESC("Info", "normal_ui_style")
    mDehazeAttrInfoV11_t Info;
#if defined(ISP_HW_V21) || defined(ISP_HW_V30)
    // M4_STRUCT_DESC("stManual", "normal_ui_style")
    mDehazeAttrV11_t stManual;
#endif
#if defined(ISP_HW_V39) || defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
    // M4_STRUCT_DESC("stManual", "normal_ui_style")
    mDehazeAttrV12_t stManual;
#endif
} adehaze_uapi_t;

#if defined(ISP_HW_V35)
typedef struct {
    /* M4_GENERIC_DESC(
       M4_ALIAS(attr),
       M4_TYPE(struct),
       M4_UI_MODULE(tuning_param),
       M4_REF(/aibnr)) */
    aibnr_api_attrib_t attr;
    // M4_STRUCT_DESC("info", "tuning_status")
    aibnr_status_t info;
} aibnr_uapi_t;

typedef struct {
    /* M4_GENERIC_DESC(
       M4_ALIAS(attr),
       M4_TYPE(struct),
       M4_UI_MODULE(tuning_param),
       M4_REF(/airms)) */
    airms_api_attrib_t attr;
    // M4_STRUCT_DESC("info", "tuning_status")
    airms_status_t info;
} airms_uapi_t;

typedef struct {
    /* M4_GENERIC_DESC(
       M4_ALIAS(attr),
       M4_TYPE(struct),
       M4_UI_MODULE(tuning_param),
       M4_REF(/aiynr)) */
    aiynr_api_attrib_t attr;
    // M4_STRUCT_DESC("info", "tuning_status")
    aiynr_status_t info;
} aiynr_uapi_t;

typedef struct {
    /* M4_GENERIC_DESC(
       M4_ALIAS(attr),
       M4_TYPE(struct),
       M4_UI_MODULE(tuning_param),
       M4_REF(/fpnSw)) */
    fpnSw_api_attrib_t attr;
    // M4_STRUCT_DESC("info", "tuning_status")
    fpnSw_status_t info;
} fpnSw_uapi_t;
#endif

typedef struct __aiq_scene {
    // M4_STRING_DESC("main_scene", M4_SIZE(1,1), M4_RANGE(0, 32), "normal", M4_DYNAMIC(0))
    char* main_scene;
    // M4_STRING_DESC("sub_scene", M4_SIZE(1,1), M4_RANGE(0, 32), "day", M4_DYNAMIC(0))
    char* sub_scene;
} aiq_scene_t;

typedef struct __work_mode {
    // M4_ENUM_DESC("mode", "rk_aiq_working_mode_t", "RK_AIQ_WORKING_MODE_NORMAL");
    rk_aiq_working_mode_t mode;
} work_mode_t;

typedef struct __aiq_sysctl_desc {
    // M4_STRUCT_DESC("scene", "normal_ui_style")
    aiq_scene_t scene;
    // M4_STRUCT_DESC("work_mode", "normal_ui_style")
    work_mode_t work_mode;
} RkaiqSysCtl_t;

#if ISP_HW_V21
typedef struct __aiq_measure_info {
    // M4_STRUCT_DESC("ae_hwstats", "normal_ui_style")
    uapi_ae_hwstats_t ae_hwstats;
    // M4_STRUCT_DESC("wb_log", "normal_ui_style")
    uapi_wbV21_log_t wb_log;
    // M4_STRUCT_DESC("af_hwstats", "normal_ui_style")
    uapi_af_v20stats_t af_hwstats;
} aiq_measure_info_t;
#elif ISP_HW_V30
typedef struct __aiq_measure_info {
    // M4_STRUCT_DESC("ae_hwstats", "normal_ui_style")
    uapi_ae_hwstats_t ae_hwstats;
    // M4_STRUCT_DESC("wb_log", "normal_ui_style")
    uapi_wbV30_log_t wb_log;
    // M4_STRUCT_DESC("af_hwstats", "normal_ui_style")
    uapi_af_v30stats_t af_hwstats;
} aiq_measure_info_t;

#elif  defined(ISP_HW_V32) || defined(ISP_HW_V30) || defined(ISP_HW_V32_LITE)
typedef struct __aiq_measure_info {
    // M4_STRUCT_DESC("ae_hwstats", "normal_ui_style")
    uapi_ae_hwstats_t ae_hwstats;
    // M4_STRUCT_DESC("wb_log", "normal_ui_style")
    uapi_wbV32_log_t wb_log;
#if ISP_HW_V32
    // M4_STRUCT_DESC("af_hwstats", "normal_ui_style")
    uapi_af_v30stats_t af_hwstats;
#elif ISP_HW_V32_LITE
    // M4_STRUCT_DESC("af_hwstats", "normal_ui_style")
    uapi_af_v32litestats_t af_hwstats;
#endif
} aiq_measure_info_t;
#elif defined(ISP_HW_V39) || defined(ISP_HW_V33) || defined(ISP_HW_V35)
typedef struct __aiq_measure_info {
    // M4_STRUCT_DESC("ae_hwstats", "normal_ui_style")
    uapi_ae_v39_hwstats_t ae_hwstats;
    // M4_STRUCT_DESC("wb_log", "normal_ui_style")
    uapi_wbV32_log_t wb_log;
#if ISP_HW_V39 || ISP_HW_V35
    // M4_STRUCT_DESC("af_hwstats", "normal_ui_style")
    afStats_stats_t af_hwstats;
#endif
} aiq_measure_info_t;
#endif

typedef struct __ablc_uapi_manual {
    // M4_ENUM_DESC("OPMOde", "RKAiqOPMode_uapi_t","RK_AIQ_OP_MODE_AUTO")
    RKAiqOPMode_t AblcOPMode;

#if defined(ISP_HW_V39) || defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
    // M4_STRUCT_DESC("blc0_para", "normal_ui_style")
    AblcSelect_V32_t blc0_para;

    // M4_STRUCT_DESC("blc1_para", "normal_ui_style")
    AblcSelect_V32_t blc1_para;

    // M4_STRUCT_DESC("blc_ob_para", "normal_ui_style")
    AblcOBSelect_V32_t blc_ob_para;
#elif defined(ISP_HW_V30)
    // M4_STRUCT_DESC("bls0", "normal_ui_style")
    AblcSelect_t bls0;
    // M4_STRUCT_DESC("bls1", "normal_ui_style")
    AblcSelect_t bls1;
#endif
} ablc_uapi_manual_t;

typedef struct __ablc_uapi_info {
    // M4_NUMBER_DESC("iso", "u32", M4_RANGE(0, 204800), "50", M4_DIGIT(0), "0", "0")
    int iso;
#if defined(ISP_HW_V39) || defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
    // M4_ARRAY_TABLE_DESC("expo_info", "normal_ui_style", "none", "0", "0")
    AblcExpInfo_V32_t expo_info;
#elif defined(ISP_HW_V30)
    // M4_ARRAY_TABLE_DESC("expo_info", "normal_ui_style", "none", "0", "0")
    AblcExpInfo_t expo_info;
#endif
} ablc_uapi_info_t;

typedef struct __ablc_uapi {
#if defined(ISP_HW_V39) || defined(ISP_HW_V32) || defined(ISP_HW_V30) || defined(ISP_HW_V32_LITE)
    // M4_STRUCT_DESC("manual", "normal_ui_style")
    ablc_uapi_manual_t manual;
    // M4_STRUCT_DESC("info", "normal_ui_style")
    ablc_uapi_info_t info;
#endif
} ablc_uapi_t;

typedef struct __abayertnr_uapi_manual {
    // M4_ENUM_DESC("OPMode", "RKAiqOPMode_uapi_t","RK_AIQ_OP_MODE_AUTO")
    RKAiqOPMode_t AbayertnrOpMode;
#if defined(ISP_HW_V39)
    // M4_STRUCT_DESC("manual_v30", "normal_ui_style")
    RK_Bayertnr_Params_V30_Select_t manual_v30;
#elif defined(ISP_HW_V32_LITE)
    // M4_STRUCT_DESC("manual_v23L", "normal_ui_style")
    RK_Bayertnr_Param_V23L_Select_t manual_v23L;
#elif  defined(ISP_HW_V32)
    // M4_STRUCT_DESC("manual_v23", "normal_ui_style")
    RK_Bayertnr_Params_V23_Select_t manual_v23;
#elif defined(ISP_HW_V30)
    // M4_STRUCT_DESC("manual_v2", "normal_ui_style")
    RK_Bayertnr_Params_V2_Select_t manual_v2;
#endif
} abayertnr_uapi_manual_t;

typedef struct __abayertnr_uapi_info {
    // M4_NUMBER_DESC("iso", "u32", M4_RANGE(0, 204800), "50", M4_DIGIT(0), "0", "0")
    int iso;
#if defined(ISP_HW_V39)
    // M4_ARRAY_TABLE_DESC("expo_info", "normal_ui_style", "none", "0", "0")
    Abayertnr_ExpInfo_V30_t expo_info;
#elif defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
    // M4_ARRAY_TABLE_DESC("expo_info", "normal_ui_style", "none", "0", "0")
    Abayertnr_ExpInfo_V23_t expo_info;
#elif defined(ISP_HW_V30)
    // M4_ARRAY_TABLE_DESC("expo_info", "normal_ui_style", "none", "0", "0")
    Abayertnr_ExpInfo_V2_t expo_info;
#endif
} abayertnr_uapi_info_t;

typedef struct __abayertnr_uapi {
#if defined(ISP_HW_V39) || defined(ISP_HW_V32) || defined(ISP_HW_V30) || defined(ISP_HW_V32_LITE)
    // M4_STRUCT_DESC("manual", "normal_ui_style")
    abayertnr_uapi_manual_t manual;
    // M4_STRUCT_DESC("info", "normal_ui_style")
    abayertnr_uapi_info_t info;
#endif
} abayertnr_uapi_t;

typedef struct __abayer2dnr_uapi_manual {
    // M4_ENUM_DESC("OPMode", "RKAiqOPMode_uapi_t","RK_AIQ_OP_MODE_AUTO")
    RKAiqOPMode_t Abayer2dnrOpMode;

#if defined(ISP_HW_V32)
    // M4_STRUCT_DESC("manual_v23", "normal_ui_style")
    RK_Bayer2dnrV23_Params_Select_t manual_v23;
#elif defined(ISP_HW_V30)
    // M4_STRUCT_DESC("manual_v2", "normal_ui_style")
    RK_Bayer2dnr_Params_V2_Select_t manual_v2;
#endif
} abayer2dnr_uapi_manual_t;

typedef struct __abayer2dnr_uapi_info {
    // M4_NUMBER_DESC("iso", "u32", M4_RANGE(0, 204800), "50", M4_DIGIT(0), "0", "0")
    int iso;
#if defined(ISP_HW_V32)
    // M4_ARRAY_TABLE_DESC("expo_info", "normal_ui_style", "none", "0", "0")
    Abayer2dnr_ExpInfo_V23_t expo_info;
#elif defined(ISP_HW_V30)
    // M4_ARRAY_TABLE_DESC("expo_info", "normal_ui_style", "none", "0", "0")
    Abayer2dnr_ExpInfo_V2_t expo_info;
#endif
} abayer2dnr_uapi_info_t;


typedef struct __abayer2dnr_uapi {
#if defined(ISP_HW_V32) || defined(ISP_HW_V30)
    // M4_STRUCT_DESC("manual", "normal_ui_style")
    abayer2dnr_uapi_manual_t manual;
    // M4_STRUCT_DESC("info", "normal_ui_style")
    abayer2dnr_uapi_info_t info;

#endif
} abayer2dnr_uapi_t;

typedef struct __aynr_uapi_manual {
    // M4_ENUM_DESC("OPMode", "RKAiqOPMode_uapi_t","RK_AIQ_OP_MODE_AUTO")
    RKAiqOPMode_t AynrOpMode;

#if defined(ISP_HW_V39)
    // M4_STRUCT_DESC("manual_v24", "normal_ui_style")
    RK_YNR_Params_V24_Select_t manual_v24;
#elif defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
    // M4_STRUCT_DESC("manual_v22", "normal_ui_style")
    RK_YNR_Params_V22_Select_t manual_v22;
#elif defined(ISP_HW_V30)
    // M4_STRUCT_DESC("manual_v3", "normal_ui_style")
    RK_YNR_Params_V3_Select_t manual_v3;
#endif
} aynr_uapi_manual_t;

typedef struct __aynr_uapi_info {
    // M4_NUMBER_DESC("iso", "u32", M4_RANGE(0, 204800), "50", M4_DIGIT(0), "0", "0")
    int iso;
#if defined(ISP_HW_V39)
    // M4_ARRAY_TABLE_DESC("expo_info", "normal_ui_style", "none", "0", "0")
    Aynr_ExpInfo_V24_t expo_info;
#elif defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
    // M4_ARRAY_TABLE_DESC("expo_info", "normal_ui_style", "none", "0", "0")
    Aynr_ExpInfo_V22_t expo_info;
#elif defined(ISP_HW_V30)
    // M4_ARRAY_TABLE_DESC("expo_info", "normal_ui_style", "none", "0", "0")
    Aynr_ExpInfo_V3_t expo_info;
#endif
} aynr_uapi_info_t;

typedef struct __aynr_uapi {
#if defined(ISP_HW_V39) || defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)|| defined(ISP_HW_V30)
    // M4_STRUCT_DESC("manual", "normal_ui_style")
    aynr_uapi_manual_t manual;
    // M4_STRUCT_DESC("info", "normal_ui_style")
    aynr_uapi_info_t info;

#endif
} aynr_uapi_t;

typedef struct __acnr_uapi_manual {
    // M4_ENUM_DESC("OPMode", "RKAiqOPMode_uapi_t","RK_AIQ_OP_MODE_AUTO")
    RKAiqOPMode_t AcnrOpMode;

#if defined(ISP_HW_V39)
    // M4_STRUCT_DESC("manual_v31", "normal_ui_style")
    RK_CNR_Params_V31_Select_t manual_v31;
#elif defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
    // M4_STRUCT_DESC("manual_v30", "normal_ui_style")
    RK_CNR_Params_V30_Select_t manual_v30;
#elif defined(ISP_HW_V30)
    // M4_STRUCT_DESC("manual_v2", "normal_ui_style")
    RK_CNR_Params_V2_Select_t manual_v2;
#endif
} acnr_uapi_manual_t;

typedef struct __acnr_uapi_info {
    // M4_NUMBER_DESC("iso", "u32", M4_RANGE(0, 204800), "50", M4_DIGIT(0), "0", "0")
    int iso;
#if defined(ISP_HW_V39)
    // M4_ARRAY_TABLE_DESC("expo_info", "normal_ui_style", "none", "0", "0")
    AcnrV31_ExpInfo_t expo_info;
#elif defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
    // M4_ARRAY_TABLE_DESC("expo_info", "normal_ui_style", "none", "0", "0")
    AcnrV30_ExpInfo_t expo_info;
#elif defined(ISP_HW_V30)
    // M4_ARRAY_TABLE_DESC("expo_info", "normal_ui_style", "none", "0", "0")
    AcnrV2_ExpInfo_t expo_info;
#endif
} acnr_uapi_info_t;


typedef struct __acnr_uapi {
#if defined(ISP_HW_V39) || defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE) || defined(ISP_HW_V30)
    // M4_STRUCT_DESC("manual", "normal_ui_style")
    acnr_uapi_manual_t manual;
    // M4_STRUCT_DESC("info", "normal_ui_style")
    acnr_uapi_info_t info;

#endif
} acnr_uapi_t;

typedef struct __asharp_uapi_manual {
    // M4_ENUM_DESC("OPMode", "RKAiqOPMode_uapi_t","RK_AIQ_OP_MODE_AUTO")
    RKAiqOPMode_t AsharpOpMode;

#if defined(ISP_HW_V39)
    // M4_STRUCT_DESC("manual_v34", "normal_ui_style")
    RK_SHARP_Params_V34_Select_t manual_v34;
#elif defined(ISP_HW_V32)
    // M4_STRUCT_DESC("manual_v33", "normal_ui_style")
    RK_SHARP_Params_V33_Select_t manual_v33;
#elif defined(ISP_HW_V32_LITE)
    // M4_STRUCT_DESC("manual_v33Lite", "normal_ui_style")
    RK_SHARP_Params_V33LT_Select_t manual_v33Lite;
#elif defined(ISP_HW_V30)
    // M4_STRUCT_DESC("manual_v4", "normal_ui_style")
    RK_SHARP_Params_V4_Select_t manual_v4;
#endif
} asharp_uapi_manual_t;

typedef struct __asharp_uapi_info {
    // M4_NUMBER_DESC("iso", "u32", M4_RANGE(0, 204800), "50", M4_DIGIT(0), "0", "0")
    int iso;
#if defined(ISP_HW_V39)
    // M4_ARRAY_TABLE_DESC("expo_info", "normal_ui_style", "none", "0", "0")
    Asharp_ExpInfo_V34_t expo_info;
#elif defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
    // M4_ARRAY_TABLE_DESC("expo_info", "normal_ui_style", "none", "0", "0")
    Asharp_ExpInfo_V33_t expo_info;
#elif defined(ISP_HW_V30)
    // M4_ARRAY_TABLE_DESC("expo_info", "normal_ui_style", "none", "0", "0")
    Asharp4_ExpInfo_t expo_info;
#endif
} asharp_uapi_info_t;


typedef struct __asharp_uapi {
#if defined(ISP_HW_V32_LITE) || defined(ISP_HW_V39) || defined(ISP_HW_V32) || defined(ISP_HW_V30)
    // M4_STRUCT_DESC("manual", "normal_ui_style")
    asharp_uapi_manual_t manual;
    // M4_STRUCT_DESC("info", "normal_ui_style")
    asharp_uapi_info_t info;
#endif
} asharp_uapi_t;

typedef struct __ayuvme_uapi_manual {
    // M4_ENUM_DESC("OPMode", "RKAiqOPMode_uapi_t","RK_AIQ_OP_MODE_AUTO")
    RKAiqOPMode_t AyuvmeOpMode;

#if defined(ISP_HW_V39)
    // M4_STRUCT_DESC("manual_v1", "normal_ui_style")
    RK_YUVME_Params_V1_Select_t manual_v1;
#endif
} ayuvme_uapi_manual_t;

typedef struct __ayuvme_uapi_info {
    // M4_NUMBER_DESC("iso", "u32", M4_RANGE(0, 204800), "50", M4_DIGIT(0), "0", "0")
    int iso;
#if defined(ISP_HW_V39)
    // M4_ARRAY_TABLE_DESC("expo_info", "normal_ui_style", "none", "0", "0")
    Ayuvme_ExpInfo_V1_t expo_info;
#endif
} ayuvme_uapi_info_t;

typedef struct __ayuvme_uapi {
#if defined(ISP_HW_V39)
    // M4_STRUCT_DESC("manual", "normal_ui_style")
    ayuvme_uapi_manual_t manual;
    // M4_STRUCT_DESC("info", "normal_ui_style")
    ayuvme_uapi_info_t info;
#endif
} ayuvme_uapi_t;


typedef struct __again_uapi_manual {
    // M4_ENUM_DESC("OPMode", "RKAiqOPMode_uapi_t","RK_AIQ_OP_MODE_AUTO")
    RKAiqOPMode_t AgainOpMode;

#if defined(ISP_HW_V32_LITE) || defined(ISP_HW_V39) || defined(ISP_HW_V32) || defined(ISP_HW_V30)
    // M4_STRUCT_DESC("manual_v2", "normal_ui_style")
    RK_GAIN_Select_V2_t manual_v2;
#endif
} again_uapi_manual_t;

typedef struct __again_uapi_info {
    // M4_NUMBER_DESC("iso", "u32", M4_RANGE(0, 204800), "50", M4_DIGIT(0), "0", "0")
    int iso;
#if defined(ISP_HW_V32_LITE) || defined(ISP_HW_V39) || defined(ISP_HW_V32) || defined(ISP_HW_V30)
    // M4_ARRAY_TABLE_DESC("expo_info", "normal_ui_style", "none", "0", "0")
    Again_ExpInfo_V2_t expo_info;
#endif
} again_uapi_info_t;


typedef struct __again_uapi {
#if defined(ISP_HW_V39) || defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE) || defined(ISP_HW_V30)
    // M4_STRUCT_DESC("manual", "normal_ui_style")
    again_uapi_manual_t manual;
    // M4_STRUCT_DESC("info", "normal_ui_style")
    again_uapi_info_t info;
#endif
} again_uapi_t;

typedef struct __accm_uapi {
    // M4_STRUCT_DESC("mode", "normal_ui_style")
    uapi_wb_mode_t mode;
    // M4_STRUCT_DESC("Info", "normal_ui_style", "0", "1")
    rk_aiq_ccm_querry_info_t Info;
#if defined(ISP_HW_V20) || defined(ISP_HW_V21) || defined(ISP_HW_V30)
    // M4_STRUCT_DESC("manual", "normal_ui_style")
    rk_aiq_ccm_mccm_attrib_t stManual;
#endif
#if defined(ISP_HW_V39) || defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
    // M4_STRUCT_DESC("manual", "normal_ui_style")
    rk_aiq_ccm_mccm_attrib_v2_t stManual;
#endif
} accm_uapi_t;

typedef struct __acsm_uapi {
    // M4_STRUCT_DESC("manual", "normal_ui_style")
    Csm_Param_t manual;
} acsm_uapi_t;

typedef struct __acgc_uapi {
    // M4_STRUCT_DESC("manual", "normal_ui_style")
    Cgc_Param_t manual;
} acgc_uapi_t;

typedef struct __a3dlut_uapi {
    // M4_STRUCT_DESC("mode", "normal_ui_style")
    uapi_wb_mode_t mode;
    // M4_STRUCT_DESC("manual", "normal_ui_style")
    rk_aiq_lut3d_mlut3d_attrib_t manual;
} a3dlut_uapi_t;

typedef struct __agic_uapi {
    // M4_STRUCT_DESC("attr", "normal_ui_style")
#if RKAIQ_HAVE_GIC_V1
    rkaiq_gic_v1_api_attr_t attr;
#endif
#if RKAIQ_HAVE_GIC_V2
    // M4_STRUCT_DESC("attr", "normal_ui_style")
    rkaiq_gic_v2_api_attr_t attr;
#endif
} agic_uapi_t;

typedef struct __aie_uapi {
    // M4_STRUCT_DESC("attr", "normal_ui_style")
    aie_attrib_t attr;
} aie_uapi_t;

typedef struct __acp_uapi {
    // M4_STRUCT_DESC("attr", "normal_ui_style")
    acp_attrib_t attr;
} acp_uapi_t;

#if (USE_NEWSTRUCT == 0)
typedef struct __alsc_uapi {
    // M4_STRUCT_DESC("manual", "normal_ui_style")
    rk_aiq_lsc_table_t manual;
} alsc_uapi_t;
#endif

typedef struct __camgroup_uapi {
    // M4_NUMBER_DESC("current_index", "u8", M4_RANGE(0, 8), "0", M4_DIGIT(0), M4_HIDE(1))
    uint8_t current_index;
} camgroup_uapi_t;

#if defined(ISP_HW_V20) || defined(ISP_HW_V21) || defined(ISP_HW_V30) || defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE) || defined(ISP_HW_V39) || defined(ISP_HW_V35)
typedef struct __af_uapi {
#if defined(ISP_HW_V20) || defined(ISP_HW_V21)
    // M4_STRUCT_DESC("calib", "normal_ui_style")
    CalibDbV2_AF_Tuning_Para_t attr;
#endif
#if defined(ISP_HW_V30)
    // M4_STRUCT_DESC("calib", "normal_ui_style")
    CalibDbV2_AFV30_Tuning_Para_t attr;
#endif
#if defined(ISP_HW_V32)
    // M4_STRUCT_DESC("calib", "normal_ui_style")
    CalibDbV2_AFV31_Tuning_Para_t attr;
#endif
#if defined(ISP_HW_V32_LITE)
    // M4_STRUCT_DESC("calib", "normal_ui_style")
    CalibDbV2_AFV32_Tuning_Para_t attr;
#endif
#if defined(ISP_HW_V39)
    /* M4_GENERIC_DESC(
       M4_ALIAS(attr),
       M4_TYPE(struct),
       M4_UI_MODULE(tuning_param),
       M4_REF(/af_v33)) */
    CalibDbV2_AFV33_t attr;
#endif
#if defined(ISP_HW_V35)
    /* M4_GENERIC_DESC(
       M4_ALIAS(attr),
       M4_TYPE(struct),
       M4_UI_MODULE(tuning_param),
       M4_REF(/af_calib)) */
    af_param_t attr;
#endif
} af_uapi_t;
#endif

typedef struct __aiq_uapi_t {
#if (USE_NEWSTRUCT == 0)
    // M4_STRUCT_DESC("ae_uapi", "normal_ui_style")
    ae_uapi_t ae_uapi;
#endif
#if defined(ISP_HW_V20)
    // M4_STRUCT_DESC("atmo_uapi", "normal_ui_style")
    atmo_uapi_t atmo_uapi;
#else

#endif
#if (USE_NEWSTRUCT == 0)
    // M4_STRUCT_DESC("amerge_uapi", "normal_ui_style")
    amerge_uapi_t amerge_uapi;
    // M4_STRUCT_DESC("agamma_uapi", "normal_ui_style")
    agamma_uapi_t agamma_uapi;
    // M4_STRUCT_DESC("adrc_uapi", "normal_ui_style")
    adrc_uapi_t adrc_uapi;
    // M4_STRUCT_DESC("adehaze_uapi", "normal_ui_style")
    adehaze_uapi_t adehaze_uapi;
#endif
    // M4_STRUCT_DESC("SystemCtl", "normal_ui_style")
    RkaiqSysCtl_t system;
#if (USE_NEWSTRUCT == 0)
    // M4_STRUCT_DESC("adpcc_manual", "normal_ui_style")
    Adpcc_Manual_Attr_t adpcc_manual;
    // M4_STRUCT_DESC("a3dlut_uapi", "normal_ui_style")
    a3dlut_uapi_t a3dlut_uapi;
#endif
#if !defined(ISP_HW_V32_LITE)
#if (USE_NEWSTRUCT == 0)
    // M4_STRUCT_DESC("agic_uapi", "normal_ui_style")
    agic_uapi_t agic_uapi;
#endif
#endif
#if (USE_NEWSTRUCT == 0)
    // M4_STRUCT_DESC("aie_uapi", "normal_ui_style")
    aie_uapi_t aie_uapi;
    // M4_STRUCT_DESC("acp_uapi", "normal_ui_style")
    acp_uapi_t acp_uapi;
    // M4_STRUCT_DESC("alsc_uapi", "normal_ui_style")
    alsc_uapi_t alsc_uapi;
#endif
#if defined(ISP_HW_V30)
    // M4_STRUCT_DESC("camgroup_uapi", "normal_ui_style", M4_HIDE(1))
    camgroup_uapi_t camgroup_uapi;
#endif
#if (USE_NEWSTRUCT == 0)
    // M4_STRUCT_DESC("ablc_uapi_t", "normal_ui_style")
    ablc_uapi_t ablc_uapi;
    // M4_STRUCT_DESC("abayertnr_uapi", "normal_ui_style")
    abayertnr_uapi_t abayertnr_uapi;
#endif
#if !defined(ISP_HW_V32_LITE) || !defined(ISP_HW_V39)
    // M4_STRUCT_DESC("abayer2dnr_uapi", "normal_ui_style")
    abayer2dnr_uapi_t abayer2dnr_uapi;
#endif
#if (USE_NEWSTRUCT == 0)
    // M4_STRUCT_DESC("aynr_uapi", "normal_ui_style")
    aynr_uapi_t aynr_uapi;
    // M4_STRUCT_DESC("acnr_uapi", "normal_ui_style")
    acnr_uapi_t acnr_uapi;
    // M4_STRUCT_DESC("asharp_uapi", "normal_ui_style")
    asharp_uapi_t asharp_uapi;
    // M4_STRUCT_DESC("again_uapi", "normal_ui_style")
    again_uapi_t again_uapi;
#if defined (ISP_HW_V39)
    // M4_STRUCT_DESC("ayuvme_uapi", "normal_ui_style")
    ayuvme_uapi_t ayuvme_uapi;
#endif
#endif
#if !defined (ISP_HW_V20)
    // M4_STRUCT_DESC("measure_info", "normal_ui_style",M4_HIDE(1))
    aiq_measure_info_t measure_info;
#endif
#if (USE_NEWSTRUCT == 0)
    // M4_STRUCT_DESC("accm_uapi", "normal_ui_style")
    accm_uapi_t accm_uapi;
    // M4_STRUCT_DESC("acsm_uapi", "normal_ui_style")
    acsm_uapi_t acsm_uapi;
#if !defined(ISP_HW_V20)
    // M4_STRUCT_DESC("acgc_uapi", "normal_ui_style")
    acgc_uapi_t acgc_uapi;
#endif
#endif

#if USE_NEWSTRUCT
    // M4_STRUCT_DESC("ae_uapi", "normal_ui_style")
    ae_uapi_t ae_uapi;
    // M4_STRUCT_DESC("awb_uapi", "normal_ui_style")
    awb_uapi_t awb_uapi;
    // M4_STRUCT_DESC("dm_uapi", "iso_list_template")
    dm_uapi_t dm_uapi;
    // M4_STRUCT_DESC("btnr_uapi", "iso_list_template")
    btnr_uapi_t btnr_uapi;
    // M4_STRUCT_V2_DESC("btnr2_uapi", "iso_list_template", "0", "0", "/uapi/0/btnr_uapi")
    btnr2_uapi_t btnr2_uapi;
    // M4_STRUCT_DESC("gamma_uapi", "iso_list_template")
    gamma_uapi_t gamma_uapi;
    // M4_STRUCT_DESC("drc_uapi", "iso_list_template")
    drc_uapi_t drc_uapi;
    // M4_STRUCT_DESC("trans_uapi", "iso_list_template")
    trans_uapi_t trans_uapi;
#if defined(ISP_HW_V39)
    // M4_STRUCT_DESC("dhzEhz_uapi", "iso_list_template")
    dehaze_uapi_t dhzEhz_uapi;
#endif
    // M4_STRUCT_DESC("histeq_uapi", "iso_list_template")
    histeq_uapi_t histeq_uapi;
#if defined(ISP_HW_V33) || defined(ISP_HW_V35)
    // M4_STRUCT_DESC("enh_uapi", "iso_list_template")
    enh_uapi_t enh_uapi;
    // M4_STRUCT_DESC("texEst_uapi", "iso_list_template")
    texEst_uapi_t texEst_uapi;
#endif
    // M4_STRUCT_DESC("ynr_uapi", "iso_list_template")
    ynr_uapi_t ynr_uapi;
    // M4_STRUCT_DESC("sharp_uapi", "iso_list_template")
    sharp_uapi_t sharp_uapi;
    // M4_STRUCT_DESC("cnr_uapi", "iso_list_template")
    cnr_uapi_t cnr_uapi;
    // M4_STRUCT_DESC("blc_uapi", "iso_list_template")
    blc_uapi_t blc_uapi;
    // M4_STRUCT_DESC("dpc_uapi", "iso_list_template")
    dpc_uapi_t dpc_uapi;
    // M4_STRUCT_DESC("gic_uapi", "iso_list_template")
    gic_uapi_t gic_uapi;
    // M4_STRUCT_DESC("cac_uapi", "iso_list_template")
    cac_uapi_t cac_uapi;
    // M4_STRUCT_DESC("csm_uapi", "iso_list_template")
    csm_uapi_t csm_uapi;
    // M4_STRUCT_DESC("mge_uapi", "iso_list_template")
    merge_uapi_t mge_uapi;
#if defined(ISP_HW_V39) || defined(ISP_HW_V35)
    // M4_STRUCT_DESC("rgbir_uapi", "iso_list_template")
    rgbir_uapi_t rgbir_uapi;
#endif
    // M4_STRUCT_DESC("cgc_uapi", "iso_list_template")
    cgc_uapi_t cgc_uapi;
    // M4_STRUCT_DESC("cp_uapi", "iso_list_template")
    cp_uapi_t cp_uapi;
    // M4_STRUCT_DESC("ie_uapi", "iso_list_template")
    ie_uapi_t ie_uapi;
    // M4_STRUCT_DESC("gain_uapi", "iso_list_template")
    gain_uapi_t gain_uapi;
#if defined(ISP_HW_V39)
    // M4_STRUCT_DESC("lut3d_uapi", "double_list_template")
    lut3d_uapi_t lut3d_uapi;
#endif
    // M4_STRUCT_DESC("ldc_uapi", "normal_ui_style")
    ldc_uapi_t ldc_uapi;
#if defined(ISP_HW_V33) || defined(ISP_HW_V35)
    // M4_STRUCT_DESC("hsv_uapi", "double_list_template")
    hsv_uapi_t hsv_uapi;
#endif
    // M4_STRUCT_DESC("ver_uapi", "normal_ui_style")
    ver_uapi_t ver_uapi;
    // M4_STRUCT_DESC("module_ctl_uapi", "normal_ui_style")
    module_ctl_uapi_t module_ctl_uapi;
    // M4_STRUCT_DESC("lsc_uapi", "double_list_template")
    lsc_uapi_t lsc_uapi;
    // M4_STRUCT_DESC("ccm_uapi", "double_list_template")
    ccm_uapi_t ccm_uapi;
#if defined(ISP_HW_V33) || defined(ISP_HW_V35)
    // M4_STRUCT_DESC("postisp_uapi", "iso_list_template")
    postisp_uapi_t postisp_uapi;
#endif
#endif
#if defined(ISP_HW_V20) || defined(ISP_HW_V21) || defined(ISP_HW_V30) || defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE) || defined(ISP_HW_V39) || defined(ISP_HW_V35)
    // M4_STRUCT_DESC("af_uapi", "normal_ui_style")
    af_uapi_t af_uapi;
#endif
#if defined(ISP_HW_V35)
    // M4_STRUCT_DESC("aibnr_uapi", "iso_list_template")
    aibnr_uapi_t aibnr_uapi;

    // M4_STRUCT_DESC("airms_uapi", "iso_list_template")
    airms_uapi_t airms_uapi;

    // M4_STRUCT_DESC("aiynr_uapi", "iso_list_template")
    aiynr_uapi_t aiynr_uapi;
#endif
#if defined(ISP_HW_V35)
    // M4_STRUCT_DESC("interpIso_uapi", "iso_list_template")
    algo_interp_iso_list_t interpIso_uapi;
    // M4_STRUCT_DESC("cp_uapi", "iso_list_template")
    fpnSw_uapi_t fpnSw_uapi;
#endif
} RkaiqUapi_t;

#ifdef __cplusplus
}
#endif

#endif  /*___RK_AIQ_UAPITYPES_H__*/
