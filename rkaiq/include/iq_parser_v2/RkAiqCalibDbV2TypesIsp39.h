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

#ifndef ___RK_AIQ_CALIB_DB_V2_TYPES_ISP39_H__
#define ___RK_AIQ_CALIB_DB_V2_TYPES_ISP39_H__

#include "ablc_head_V32.h"
#include "adegamma_head.h"
#include "adehaze_head.h"
#include "adpcc_head.h"
#include "adrc_head.h"
#include "aec_head.h"
#include "af_head.h"
#include "agamma_head.h"
#include "agic_head.h"
#include "aie_head.h"
#include "alsc_head.h"
#include "amerge_head.h"
#include "argbir_head.h"
#include "atmo_head.h"
//#include "awb_head.h"
#include "bayertnr_head_v30.h"
#include "cac_head.h"
#include "ccm_head.h"
#include "cgc_head.h"
#include "cnr_head_v31.h"
#include "cproc_head.h"
#include "cpsl_head.h"
#include "csm_head.h"
#include "debayer_head.h"
#include "fec_head.h"
#include "gain_head_v2.h"
#include "ldc_head.h"
#include "ldch_head.h"
#include "lut3d_head.h"
#include "sharp_head_v34.h"
#include "thumbnails_head.h"
#include "ynr_head_v24.h"
#include "yuvme_head_v1.h"

#include "isp/rk_aiq_isp_ae25.h"
#include "algos/rk_aiq_api_types_ae.h"
#include "isp/rk_aiq_isp_dm23.h"
#include "algos/rk_aiq_api_types_dm.h"
#include "isp/rk_aiq_isp_btnr40.h"
#include "algos/rk_aiq_api_types_btnr40.h"
#include "isp/rk_aiq_isp_gamma21.h"
#include "algos/rk_aiq_api_types_gamma.h"
#include "isp/rk_aiq_isp_ynr34.h"
#include "algos/rk_aiq_api_types_ynr.h"
#include "isp/rk_aiq_isp_sharp34.h"
#include "algos/rk_aiq_api_types_sharp.h"
#include "isp/rk_aiq_isp_cnr34.h"
#include "algos/rk_aiq_api_types_cnr.h"
#include "isp/rk_aiq_isp_drc40.h"
#include "algos/rk_aiq_api_types_drc.h"
#include "isp/rk_aiq_isp_dehaze23.h"
#include "algos/rk_aiq_api_types_dehaze.h"
#include "isp/rk_aiq_isp_blc30.h"
#include "algos/rk_aiq_api_types_blc.h"
#include "isp/rk_aiq_isp_trans10.h"
#include "algos/rk_aiq_api_types_trans.h"
#include "isp/rk_aiq_isp_dpc21.h"
#include "algos/rk_aiq_api_types_dpc.h"
#include "isp/rk_aiq_isp_gic21.h"
#include "algos/rk_aiq_api_types_gic.h"
#include "isp/rk_aiq_isp_yme10.h"
#include "algos/rk_aiq_api_types_yme.h"
#include "isp/rk_aiq_isp_histeq23.h"
#include "algos/rk_aiq_api_types_histeq.h"
#include "isp/rk_aiq_isp_cac21.h"
#include "algos/rk_aiq_api_types_cac.h"
#include "isp/rk_aiq_isp_csm21.h"
#include "algos/rk_aiq_api_types_csm.h"
#include "isp/rk_aiq_isp_merge22.h"
#include "algos/rk_aiq_api_types_merge.h"
#include "isp/rk_aiq_isp_lsc21.h"
#include "algos/rk_aiq_api_types_lsc.h"
#include "isp/rk_aiq_isp_rgbir10.h"
#include "algos/rk_aiq_api_types_rgbir.h"
#include "isp/rk_aiq_isp_cgc10.h"
#include "algos/rk_aiq_api_types_cgc.h"
#include "isp/rk_aiq_isp_cp10.h"
#include "algos/rk_aiq_api_types_cp.h"
#include "isp/rk_aiq_isp_ie10.h"
#include "algos/rk_aiq_api_types_ie.h"
#include "isp/rk_aiq_isp_gain20.h"
#include "algos/rk_aiq_api_types_gain.h"
#include "isp/rk_aiq_isp_3dlut20.h"
#include "algos/rk_aiq_api_types_3dlut.h"
#include "isp/rk_aiq_isp_ccm22.h"
#include "algos/rk_aiq_api_types_ccm.h"
#include "isp/awb_head2.h"
#include "algos/rk_aiq_api_types_awb.h"
#include "isp/rk_aiq_isp_ldc22.h"
#include "algos/rk_aiq_api_types_ldc.h"

RKAIQ_BEGIN_DECLARE

typedef struct CalibDbV2_ColorAsGrey_Param_s {
    // M4_BOOL_DESC("enable", "0")
    bool enable;
    // M4_NUMBER_DESC("skip_frame", "s8", M4_RANGE(0,255), "10", M4_DIGIT(0))
    int8_t skip_frame;
} CalibDbV2_ColorAsGrey_Param_t;

typedef struct CalibDbV2_ColorAsGrey_s {
    // M4_STRUCT_DESC("param", "normal_ui_style")
    CalibDbV2_ColorAsGrey_Param_t param;
} CalibDbV2_ColorAsGrey_t;

typedef struct CalibDbV2_LUMA_DETECT_s {
    unsigned char luma_detect_en;
    int fixed_times;
    float mutation_threshold;
    float mutation_threshold_level2;
} CalibDbV2_LUMA_DETECT_t;

typedef struct CamCalibDbV2ContextIsp39_s {
#if USE_NEWSTRUCT
    // M4_STRUCT_DESC("ae_calib", "normal_ui_style")
    ae_param_t ae_calib;
#else
    // M4_STRUCT_DESC("ae_calib", "normal_ui_style")
    CalibDb_Aec_ParaV2_t ae_calib;
#endif
    // M4_STRUCT_DESC("wb_v32", "normal_ui_style")
    //CalibDbV2_Wb_Para_V32_t wb_v32;
    // M4_STRUCT_DESC("wb", "normal_ui_style")
    awb_api_attrib_t wb;
#if USE_NEWSTRUCT
    // M4_STRUCT_DESC("blc", "normal_ui_style")
    blc_api_attrib_t blc;
#else
    // M4_STRUCT_DESC("ablcV32_calib", "normal_ui_style")
    CalibDbV2_Blc_V32_t ablcV32_calib;
#endif
#if USE_NEWSTRUCT
    // M4_STRUCT_DESC("ccm", "normal_ui_style")
    ccm_calib_attrib_t ccm;
#else
    // M4_STRUCT_DESC("ccm_calib", "normal_ui_style")
    CalibDbV2_Ccm_Para_V39_t ccm_calib_v2;
#endif
#if USE_NEWSTRUCT
    // M4_STRUCT_DESC("lut3d", "normal_ui_style")
    lut3d_calib_attrib_t lut3d;
#else
    // M4_STRUCT_DESC("lut3d_calib", "normal_ui_style")
    CalibDbV2_Lut3D_Para_V2_t lut3d_calib;
#endif
    // M4_STRUCT_DESC("degamma", "curve_ui_type_A")
    CalibDbV2_Adegmma_t adegamma_calib;
#if defined(ISP_HW_V39)
    #ifdef USE_NEWSTRUCT
    // M4_STRUCT_DESC("gic", "normal_ui_style")
    gic_api_attrib_t gic;
    #else
    // M4_STRUCT_DESC("agic_calib_v21", "normal_ui_style")
    CalibDbV2_Gic_V21_t agic_calib_v21;
    #endif
#endif

#if defined(ISP_HW_V39)
    #ifdef USE_NEWSTRUCT
    // M4_STRUCT_DESC("demosaic", "normal_ui_style")
    dm_api_attrib_t demosaic;
    #else
    // M4_STRUCT_DESC("debayer_calib", "normal_ui_style")
    CalibDbV2_Debayer_v3_t debayer_v3;
    #endif
#endif
#ifdef USE_NEWSTRUCT
    // M4_STRUCT_DESC("mge", "normal_ui_style")
    mge_api_attrib_t mge;
#else
    // M4_STRUCT_DESC("Amerge_calib", "normal_ui_style")
    CalibDbV2_merge_v12_t amerge_calib_v12;
#endif
#if defined(ISP_HW_V39)
    #ifdef USE_NEWSTRUCT
    // M4_STRUCT_DESC("gamma", "normal_ui_style")
    gamma_api_attrib_t gamma;
    // M4_STRUCT_DESC("drc", "normal_ui_style")
    drc_api_attrib_t drc;
    // M4_STRUCT_DESC("trans", "normal_ui_style")
    trans_api_attrib_t trans;
    // M4_STRUCT_DESC("dhzEhz", "normal_ui_style")
    dehaze_api_attrib_t dhzEhz;
#if 0 // disable for some reason now
    // M4_STRUCT_DESC("yme", "normal_ui_style")
    yme_api_attrib_t yme;
#endif
    // M4_STRUCT_DESC("histEQ", "normal_ui_style")
    histeq_api_attrib_t histEQ;
    #else
    // M4_STRUCT_DESC("Adrc_calib", "normal_ui_style")
    CalibDbV2_drc_V20_t adrc_calib_v20;
    // M4_STRUCT_DESC("agamma_calib", "normal_ui_style")
    CalibDbV2_gamma_v11_t agamma_calib_v11;
    // M4_STRUCT_DESC("adehaze_calib", "normal_ui_style")
    CalibDbV2_dehaze_v14_t adehaze_calib_v14;
    // M4_STRUCT_DESC("yuvme_v1", "normal_ui_style")
    CalibDbV2_YuvmeV1_t yuvme_v1;
    #endif
#endif

    #ifdef USE_NEWSTRUCT
    // M4_STRUCT_DESC("rgbir", "normal_ui_style")
    rgbir_api_attrib_t rgbir;
    // M4_STRUCT_DESC("dpc", "normal_ui_style")
    dpc_api_attrib_t dpc;
    // M4_STRUCT_DESC("cp", "normal_ui_style")
    cp_api_attrib_t cp;
    // M4_STRUCT_DESC("ie", "normal_ui_style")
    ie_api_attrib_t ie;
    #else
    // M4_STRUCT_DESC("argbir_calib_v10", "normal_ui_style")
    CalibDbV2_argbir_v10_t argbir_calib_v10;
    // M4_STRUCT_DESC("adpcc_calib", "normal_ui_style")
    CalibDbV2_Dpcc_t adpcc_calib;
    // M4_STRUCT_DESC("aldch", "normal_ui_style")
    CalibDbV2_LDCH_t aldch;
    // M4_STRUCT_DESC("cproc_calib", "normal_ui_style")
    CalibDbV2_Cproc_t cproc;
    // M4_STRUCT_DESC("aie_calib", "normal_ui_style")
    CalibDbV2_IE_t ie;
    #endif
    // M4_STRUCT_DESC("colorAsGrey", "normal_ui_style")
    CalibDbV2_ColorAsGrey_t colorAsGrey;
    #ifdef USE_NEWSTRUCT
    // M4_STRUCT_DESC("lsc", "normal_ui_style")
    lsc_calib_attrib_t lsc;
    #else
    // M4_STRUCT_DESC("lsc_v2", "normal_ui_style")
    CalibDbV2_LSC_t lsc_v2;
    #endif
#if defined(ISP_HW_V39)
#ifdef USE_NEWSTRUCT
    // M4_STRUCT_DESC("bayertnr", "normal_ui_style")
    btnr_api_attrib_t bayertnr;
#else
    // M4_STRUCT_DESC("bayertnr_v30", "normal_ui_style")
    CalibDbV2_BayerTnrV30_t bayertnr_v30;
#endif
#endif
#ifdef USE_NEWSTRUCT
    // M4_STRUCT_DESC("ynr", "normal_ui_style")
    ynr_api_attrib_t ynr;
#else
    // M4_STRUCT_DESC("ynr_v24", "normal_ui_style")
    CalibDbV2_YnrV24_t ynr_v24;
#endif
#if defined(ISP_HW_V39)
#ifdef USE_NEWSTRUCT
    // M4_STRUCT_DESC("cnr", "normal_ui_style")
    cnr_api_attrib_t cnr;
#else
    // M4_STRUCT_DESC("cnr_v31", "normal_ui_style")
    CalibDbV2_CNRV31_t cnr_v31;
#endif
#endif
#ifdef USE_NEWSTRUCT
    // M4_STRUCT_DESC("sharp", "normal_ui_style")
    sharp_api_attrib_t sharp;
#else
    // M4_STRUCT_DESC("sharp_v34", "normal_ui_style")
    CalibDbV2_SharpV34_t sharp_v34;
#endif
#if defined(ISP_HW_V39)
    #ifdef USE_NEWSTRUCT
    // M4_STRUCT_DESC("cac", "normal_ui_style")
    cac_api_attrib_t cac;
    #else
    // M4_STRUCT_DESC("cac_v11", "normal_ui_style")
    CalibDbV2_Cac_V11_t cac_v11;
    #endif
#endif

#if defined(ISP_HW_V39)
    // M4_STRUCT_DESC("af_calib", "normal_ui_style")
    CalibDbV2_AFV33_t af_v33;
#endif

#ifdef USE_NEWSTRUCT
    // M4_STRUCT_DESC("csm", "normal_ui_style")
    csm_api_attrib_t csm;
    // M4_STRUCT_DESC("cgc", "normal_ui_style")
    cgc_api_attrib_t cgc;
    // M4_STRUCT_DESC("gain", "normal_ui_style")
    gain_api_attrib_t gain;
#else
    // M4_STRUCT_DESC("csm", "normal_ui_style")
    CalibDbV2_csm_t csm;
    // M4_STRUCT_DESC("cgc", "normal_ui_style")
    CalibDbV2_cgc_t cgc;
    // M4_STRUCT_DESC("gain_v2", "normal_ui_style")
    CalibDbV2_GainV2_t gain_v2;
#endif
#ifdef USE_NEWSTRUCT
    // M4_STRUCT_DESC("ldc", "normal_ui_style")
    ldc_api_attrib_t ldc;
#endif
} CamCalibDbV2ContextIsp39_t;

RKAIQ_END_DECLARE

#endif
