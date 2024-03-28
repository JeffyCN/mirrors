/*
 *rk_aiq_types_alsc_algo_prvt.h
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

#ifndef _RK_AIQ_TYPES_ABAYERTNR_ALGO_PRVT_V30_H_
#define _RK_AIQ_TYPES_ABAYERTNR_ALGO_PRVT_V30_H_

#include "abayertnrV30/rk_aiq_types_abayertnr_algo_int_v30.h"
#include "RkAiqCalibDbTypes.h"
#include "xcam_log.h"
#include "xcam_common.h"
#include "rk_aiq_types_priv.h"

//RKAIQ_BEGIN_DECLARE

// fix define
#define FIXTNRSQRT      30
#define FIXTNRWGT       10
#define FIXTNRKAL       8
#define FIXTNRWWW       12
#define FIXGAINOUT      8
#define FIXTNRSFT       10
#define FIXMERGEWG      10      // HDR merge weight
#define MERGEWGBIT      6       // HDR merge weight cut bits
#define FIXGAINBIT      8
#define FIXTNRCALC      10
#define FIXBILCALC      10
#define FIXDIFMAX       ((long long)1<<(14))     // DIFF USE 12bits
#define FIXBITIIRSPR    8
#define FIXBITWFWGT     8
#define FIXBITDGAIN     8


typedef struct Abayertnr_trans_params_v30_s {
    int bayertnr_logprecision;
    int bayertnr_logfixbit;
    int bayertnr_logtblbit;
    int bayertnr_logscalebit;
    int bayertnr_logfixmul;
    int bayertnr_logtblmul;
    int bayertnr_logtablef[64];
    int bayertnr_logtablei[64];
    int bayertnr_tnr_sigma_curve_double_pos;

    float bayertnr_wgt_stat;
    float bayertnr_pk_stat;

    bool bayertnr_auto_sig_count_en;
    int  bayertnr_auto_sig_count_filt_wgt;
    int bayertnr_auto_sig_count_valid;
    uint32_t bayertnr_auto_sig_count_max;


    int rawWidth;
    int rawHeight;

    uint16_t tnr_luma_sigma_y[20];
    int bayertnr_iso_pre;
} Abayertnr_trans_params_v30_t;

//anr context
typedef struct Abayertnr_Context_V30_s {
    Abayertnr_ExpInfo_V30_t stExpInfo;
    Abayertnr_State_V30_t eState;
    Abayertnr_OPMode_V30_t eMode;

    Abayertnr_Auto_Attr_V30_t stAuto;
    Abayertnr_Manual_Attr_V30_t stManual;

    rk_aiq_bayertnr_strength_v30_t stStrength;

    bool isIQParaUpdate;
    bool isGrayMode;
    Abayertnr_ParamMode_V30_t eParamMode;
    int prepare_type;

    CalibDbV2_BayerTnrV30_t bayertnr_v30;

    int isReCalculate;

    bool isFirstFrame;
    Abayertnr_trans_params_v30_t stTransPrarms;
    Abayertnr_Stats_V30_t stStats;
} Abayertnr_Context_V30_t;






//RKAIQ_END_DECLARE

#endif


