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

#ifndef ___RK_AIQ_CALIB_DB_V2_TYPES_ISP33_H__
#define ___RK_AIQ_CALIB_DB_V2_TYPES_ISP33_H__

//#include "awb_head.h"
#include "cproc_head.h"
#include "cpsl_head.h"
#include "af_head.h"

#include "isp/rk_aiq_isp_ae25.h"
#include "algos/rk_aiq_api_types_ae.h"
#include "isp/rk_aiq_isp_dm24.h"
#include "algos/rk_aiq_api_types_dm.h"
#include "isp/rk_aiq_isp_btnr41.h"
#include "algos/rk_aiq_api_types_btnr41.h"
#include "isp/rk_aiq_isp_gamma21.h"
#include "algos/rk_aiq_api_types_gamma.h"
#include "isp/rk_aiq_isp_texEst40.h"
#include "algos/rk_aiq_api_types_texEst.h"
#include "isp/rk_aiq_isp_ynr40.h"
#include "algos/rk_aiq_api_types_ynr.h"
#include "isp/rk_aiq_isp_sharp40.h"
#include "algos/rk_aiq_api_types_sharp.h"
#include "isp/rk_aiq_isp_cnr35.h"
#include "algos/rk_aiq_api_types_cnr.h"
#include "isp/rk_aiq_isp_drc40.h"
#include "algos/rk_aiq_api_types_drc.h"
#include "isp/rk_aiq_isp_enh30.h"
#include "algos/rk_aiq_api_types_enh.h"
#include "isp/rk_aiq_isp_hsv10.h"
#include "algos/rk_aiq_api_types_hsv.h"
#include "isp/rk_aiq_isp_blc30.h"
#include "algos/rk_aiq_api_types_blc.h"
#include "isp/rk_aiq_isp_trans10.h"
#include "algos/rk_aiq_api_types_trans.h"
#include "isp/rk_aiq_isp_dpc21.h"
#include "algos/rk_aiq_api_types_dpc.h"
#include "isp/rk_aiq_isp_gic30.h"
#include "algos/rk_aiq_api_types_gic.h"
#include "isp/rk_aiq_isp_histeq30.h"
#include "algos/rk_aiq_api_types_histeq.h"
#include "isp/rk_aiq_isp_cac30.h"
#include "algos/rk_aiq_api_types_cac.h"
#include "isp/rk_aiq_isp_csm21.h"
#include "algos/rk_aiq_api_types_csm.h"
#include "isp/rk_aiq_isp_merge22.h"
#include "algos/rk_aiq_api_types_merge.h"
#include "isp/rk_aiq_isp_lsc21.h"
#include "algos/rk_aiq_api_types_lsc.h"
#include "isp/rk_aiq_isp_cgc10.h"
#include "algos/rk_aiq_api_types_cgc.h"
#include "isp/rk_aiq_isp_cp10.h"
#include "algos/rk_aiq_api_types_cp.h"
#include "isp/rk_aiq_isp_ie10.h"
#include "algos/rk_aiq_api_types_ie.h"
#include "isp/rk_aiq_isp_gain20.h"
#include "algos/rk_aiq_api_types_gain.h"
#include "isp/rk_aiq_isp_ccm22.h"
#include "algos/rk_aiq_api_types_ccm.h"
#include "isp/awb_head2.h"
#include "algos/rk_aiq_api_types_awb.h"
#include "isp/rk_aiq_isp_ldc22.h"
#include "algos/rk_aiq_api_types_ldc.h"
#include "rkpostisp_head_v2.h"

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

typedef struct {
    // M4_STRUCT_DESC("ae_calib", "normal_ui_style")
    ae_param_t ae_calib;
    // M4_STRUCT_DESC("wb", "normal_ui_style")
    awb_api_attrib_t wb;
    // M4_STRUCT_DESC("blc", "normal_ui_style")
    blc_api_attrib_t blc;
    // M4_STRUCT_DESC("ccm", "normal_ui_style")
    ccm_calib_attrib_t ccm;
    // M4_STRUCT_DESC("gic", "normal_ui_style")
    gic_api_attrib_t gic;
    // M4_STRUCT_DESC("demosaic", "normal_ui_style")
    dm_api_attrib_t demosaic;
    // M4_STRUCT_DESC("mge", "normal_ui_style")
    mge_api_attrib_t mge;
    // M4_STRUCT_DESC("gamma", "normal_ui_style")
    gamma_api_attrib_t gamma;
    // M4_STRUCT_DESC("drc", "normal_ui_style")
    drc_api_attrib_t drc;
    // M4_STRUCT_DESC("trans", "normal_ui_style")
    trans_api_attrib_t trans;
    // M4_STRUCT_DESC("enh", "normal_ui_style")
    enh_api_attrib_t enh;
    // M4_STRUCT_DESC("hsv", "normal_ui_style")
    hsv_calib_attrib_t hsv;
    // M4_STRUCT_DESC("histEQ", "normal_ui_style")
    histeq_api_attrib_t histEQ;
    // M4_STRUCT_DESC("dpc", "normal_ui_style")
    dpc_api_attrib_t dpc;
    // M4_STRUCT_DESC("cp", "normal_ui_style")
    cp_api_attrib_t cp;
    // M4_STRUCT_DESC("ie", "normal_ui_style")
    ie_api_attrib_t ie;
    // M4_STRUCT_DESC("colorAsGrey", "normal_ui_style")
    CalibDbV2_ColorAsGrey_t colorAsGrey;
    // M4_STRUCT_DESC("lsc", "normal_ui_style")
    lsc_calib_attrib_t lsc;
    // M4_STRUCT_DESC("bayertnr", "normal_ui_style")
    btnr_api_attrib_t bayertnr;
    // M4_STRUCT_DESC("texEst", "normal_ui_style")
    texEst_api_attrib_t texEst;
    // M4_STRUCT_DESC("ynr", "normal_ui_style")
    ynr_api_attrib_t ynr;
    // M4_STRUCT_DESC("cnr", "normal_ui_style")
    cnr_api_attrib_t cnr;
    // M4_STRUCT_DESC("sharp", "normal_ui_style")
    sharp_api_attrib_t sharp;
    // M4_STRUCT_DESC("cac", "normal_ui_style")
    cac_api_attrib_t cac;
    // M4_STRUCT_DESC("csm", "normal_ui_style")
    csm_api_attrib_t csm;
    // M4_STRUCT_DESC("cgc", "normal_ui_style")
    cgc_api_attrib_t cgc;
    // M4_STRUCT_DESC("gain", "normal_ui_style")
    gain_api_attrib_t gain;
    // M4_STRUCT_DESC("ldc", "normal_ui_style")
    ldc_api_attrib_t ldc;
    // M4_STRUCT_DESC("postisp", "normal_ui_style")
    postisp_api_attrib_t postisp;
} CamCalibDbV2ContextIsp33_t;

RKAIQ_END_DECLARE

#endif
