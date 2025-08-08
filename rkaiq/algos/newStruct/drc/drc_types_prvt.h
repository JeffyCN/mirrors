/*
* demosaic_types_prvt.h

* for rockchip v2.0.0
*
*  Copyright (c) 2024 Rockchip Corporation
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
/* for rockchip v2.0.0*/

#ifndef __RKAIQ_TYPES_Drc_ALGO_PRVT_H__
#define __RKAIQ_TYPES_Drc_ALGO_PRVT_H__

/**
 * @file drc_types_prvt.h
 *
 * @brief
 *
 *****************************************************************************/
/**
 * @page module_name_page Module Name
 * Describe here what this module does.
 *
 * For a detailed list of functions and implementation detail refer to:
 * - @ref module_name
 *
 * @defgroup Drc Auto drc
 * @{
 *
 */
#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"
#include "algo_types_priv.h"
#include "include/drc_algo_api.h"
#include "xcam_log.h"

#define DEFAULT_RECALCULATE_DELTA_ISO (0.01)
#define LONG_FRAME_MODE_RATIO         (1.0f)
#define RATIO_DEFAULT                 (1.0f)
#define RATIO_MAX                     (256.0f)
#define INIT_CALC_PARAMS_NUM          (2)
#define ISP_HDR_BIT_NUM_MAX           (20)
#define ISP_HDR_BIT_NUM_MIN           (12)
#define ISP_PREDGAIN_DEFAULT          (1.0f)
#define GAS_L0_DEFAULT                (24)
#define GAS_L1_DEFAULT                (10)
#define GAS_L2_DEFAULT                (10)
#define GAS_L3_DEFAULT                (5)
#define GAS_T_MAX                     (4)
#define GAS_T_MIN                     (0)
#define GAS_L_MAX                     (64)
#define GAS_L_MIN                     (0)
#define AE_RATIO_MAX                  (256)
#define AE_RATIO_L2M_MAX              (32.0f)

typedef struct drc_auto_curve_iir_s {
    int reCalcNum;
    adrc_drcCurve_mode_t sw_drcT_drcCurve_mode;
    int sw_drcT_iirFrm_maxLimit;
    float hw_drcT_hdr2Sdr_curve[DRC_CURVE_LEN];
    float hw_drcT_drcGain_minLimit;
} drc_auto_curve_iir_t;

typedef struct CurrData_s {
    float MotionCoef;
    DrcAEData_t AEData;
    drc_params_dyn_t dynParams;
    drc_auto_curve_iir_t autoCurveIIRParams;
} CurrData_t;

typedef struct DrcContext_s {
    const RkAiqAlgoCom_prepare_t* prepare_params;
    drc_api_attrib_t* drc_attrib;
    uint32_t *iso_list;
    int iso;
    bool isReCal_;
    FrameNumber_t FrameNumber;
    unsigned char compr_bit;
    bool isCapture;
    bool isDampStable;
    uint32_t FrameID;
    bool blc_ob_enable;
    float isp_ob_predgain;
    CurrData_t CurrData;
    NextData_t NextData;
    adrc_strength_t strg;
    rkisp_adrc_stats_t* drc_stats;
} DrcContext_t;

XCAM_BEGIN_DECLARE

#if RKAIQ_HAVE_DRC_V12
XCamReturn DrcSelectParam(DrcContext_t* pDrcCtx, drc_param_t* out, int iso);
bool DrcDamping(drc_param_t* out, CurrData_t* pCurrData, int FrameID);
void DrcExpoParaProcessing(DrcContext_t* pDrcCtx, drc_param_t* out);
#endif

#if defined(RKAIQ_HAVE_DRC_V20) || defined(RKAIQ_HAVE_DRC_V21)
XCamReturn DrcSelectParam(DrcContext_t* pDrcCtx, drc_param_t* out, trans_params_static_t* pstaTrans,
                          int iso);
static XCamReturn drcApplyStrength(DrcContext_t* pDrcCtx, drc_param_t* out);
static void drcApplyStats(DrcContext_t* pDrcCtx, drc_param_t* out, trans_params_static_t* pstaTrans,
                          int ilow, int ihigh, float ratio);
bool DrcDamping(drc_param_t* out, CurrData_t* pCurrData, int FrameID);
#endif

XCAM_END_DECLARE

#endif  //__RKAIQ_TYPES_Drc_ALGO_PRVT_H__
