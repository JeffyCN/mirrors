/*
 *rk_aiq_types_alsc_algo_prvt.h
 *
 *  Copyright (c) 2019 Rockchip Corporation
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

#ifndef _RK_AIQ_TYPES_ABAYERTNR_ALGO_PRVT_V23_H_
#define _RK_AIQ_TYPES_ABAYERTNR_ALGO_PRVT_V23_H_

#include "abayertnrV23/rk_aiq_types_abayertnr_algo_int_v23.h"
#include "RkAiqCalibDbTypes.h"
#include "xcam_log.h"
#include "xcam_common.h"

//RKAIQ_BEGIN_DECLARE

// fix define
#define FIXVSTINV       10      // VST fix
#define FIXSQRTBIT      30      // SQRT fix bits
#define FIXSQRTDIV      1       // SQRT fix
#define FIXNLMCALC      10      // NLM calc fix
#define FIXGAINBIT      8       // NLM calc fix
#define FIXBILSTRG      8       // BIL flt strength calc fix
#define FIXNLMSQRT      6       // NLM SQRT fix, FIXNLMSQRT*2 nee less FIXNLMCALC
#define FIXGAUSBIT      8       // gauss weight fix bits;
#define FIXTNRWGT       10
#define FIXTNRDIF       10
#define FIXTNRSFT       10
#define FIXTNRSTG       8
#define FIXTNREXP       20      //20        // TNR EXP fix bits
#define FIXTNREXPMOD    14      //20        // TNR EXP fix bits
#define FIXTNRSQRT      30
#define FIXGAINOUT      8

#define FIXEXPBIT      20       // EXP fix bits
#define FIXEXP2BIT     10       // EXP fix bits
#define FIXEXPDIV      1        // EXP fix

#define FIXMERGEWG      10      // HDR merge weight
#define FIXTMOWG        12      // HDR TMO weight

#define FIXDIFMAX       ((long long)1<<(14))     // DIFF USE 12bits
#define DIVSUBBIT       1                        // div sub bits

#define FIXCURVE        8           // noise curve interp
#define FIXDIVBIT       20          // DIV MAX BITS
#define FIXDAOBIT       49          // DAO MAX BITS
#define FIXSHTBIT       2           // div result shift bits.
#define FIXMINBIT       8           // div min bits.

#define LUTMAX_FIX      30.0
#define LUTMAXM1_FIX    300.0 //29.0
#define LUTPRECISION_FIX (1<<FIXNLMCALC)
#define EXPPRECISION_FIX (1<<(FIXNLMCALC+5))
#define EXP2RECISION_FIX (1<<(FIXNLMCALC+5))

//anr context
typedef struct Abayertnr_Context_V23_s {
    Abayertnr_ExpInfo_V23_t stExpInfo;
    Abayertnr_State_V23_t eState;
    Abayertnr_OPMode_V23_t eMode;

#if (RKAIQ_HAVE_BAYERTNR_V23)
    Abayertnr_Auto_Attr_V23_t stAuto;
    Abayertnr_Manual_Attr_V23_t stManual;
#else
    Abayertnr_Auto_Attr_V23L_t stAuto;
    Abayertnr_Manual_Attr_V23L_t stManual;
#endif

    // struct list_head* list_bayernr_v23;

    rk_aiq_bayertnr_strength_v23_t stStrength;

    bool isIQParaUpdate;
    bool isGrayMode;
    Abayertnr_ParamMode_V23_t eParamMode;
    int prepare_type;

#if (RKAIQ_HAVE_BAYERTNR_V23)
    CalibDbV2_BayerTnrV23_t bayertnr_v23;
#else
    CalibDbV2_BayerTnrV23Lite_t bayertnr_v23;
#endif

    int isReCalculate;
} Abayertnr_Context_V23_t;






//RKAIQ_END_DECLARE

#endif


