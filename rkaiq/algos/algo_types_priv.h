/*
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

#ifndef _ALGO_TYPES_PRIV_H_
#define _ALGO_TYPES_PRIV_H_

#include "rk_aiq_algo_des.h"
#if USE_NEWSTRUCT
#include "newStruct/demosaic/include/demosaic_algo_api.h"
#include "newStruct/bayertnr/include/bayertnr_algo_api.h"
#include "newStruct/gamma/include/gamma_algo_api.h"
#include "newStruct/ynr/include/ynr_algo_api.h"
#include "newStruct/sharp/include/sharp_algo_api.h"
#include "newStruct/cnr/include/cnr_algo_api.h"
#include "newStruct/drc/include/drc_algo_api.h"
#include "newStruct/dehaze/include/dehaze_algo_api.h"
#include "newStruct/blc/include/blc_algo_api.h"
#include "newStruct/dpc/include/dpc_algo_api.h"
#endif

typedef struct {
    RkAiqAlgoResCom res_com;
#if USE_NEWSTRUCT
    dm_param_t* dmRes;
#endif
} RkAiqAlgoProcResDm;

typedef struct {
    RkAiqAlgoResCom res_com;
#if USE_NEWSTRUCT
    btnr_param_t* btnrRes;
#endif
} RkAiqAlgoProcResBtnr;

typedef struct {
    RkAiqAlgoCom com;
    float blc_ob_predgain;
} RkAiqAlgoProcBtnr;

typedef struct {
    RkAiqAlgoResCom res_com;
#if USE_NEWSTRUCT
    gamma_param_t* gammaRes;
#endif
} RkAiqAlgoProcResGamma;

typedef struct {
    RkAiqAlgoResCom res_com;
#if USE_NEWSTRUCT
    drc_param_t* drcRes;
#endif
} RkAiqAlgoProcResDrc;

typedef struct {
    RkAiqAlgoCom com;
    bool LongFrmMode;
    bool blc_ob_enable;
    float isp_ob_predgain;
#if USE_NEWSTRUCT
    trans_mode_t hw_transCfg_trans_mode;
    uint16_t hw_transCfg_transOfDrc_offset;
#endif
} RkAiqAlgoProcDrc;

typedef struct {
    RkAiqAlgoCom com;
    int working_mode;
    unsigned char compr_bit;
} RkAiqAlgoConfigDrc;

typedef struct {
    RkAiqAlgoResCom res_com;
#if USE_NEWSTRUCT
    dehaze_param_t* dehazeRes;
#endif
} RkAiqAlgoProcResDehaze;

typedef struct {
    RkAiqAlgoCom com;
#if USE_NEWSTRUCT
    union {
        dehaze_stats_v12_t* dehaze_stats_v12;
        dehaze_stats_v14_t* dehaze_stats_v14;
    };
    bool stats_true;
    float sigma[YNR_ISO_CURVE_POINT_NUM];
    bool blc_ob_enable;
    float isp_ob_predgain;
#endif
} RkAiqAlgoProcDehaze;

typedef struct {
    RkAiqAlgoResCom res_com;
#if USE_NEWSTRUCT
    ynr_param_t* ynrRes;
#endif
} RkAiqAlgoProcResYnr;

typedef struct {
    RkAiqAlgoCom com;
    float blc_ob_predgain;
} RkAiqAlgoProcYnr;

typedef struct {
    RkAiqAlgoResCom res_com;
#if USE_NEWSTRUCT
    sharp_param_t* sharpRes;
#endif
} RkAiqAlgoProcResSharp;

typedef struct {
    RkAiqAlgoCom com;
    float blc_ob_predgain;
} RkAiqAlgoProcSharp;

typedef struct {
    RkAiqAlgoResCom res_com;
#if USE_NEWSTRUCT
    cnr_param_t* cnrRes;
#endif
} RkAiqAlgoProcResCnr;

typedef struct {
    RkAiqAlgoCom com;
    float blc_ob_predgain;
} RkAiqAlgoProcCnr;

typedef struct {
    RkAiqAlgoResCom res_com;
#if USE_NEWSTRUCT
    blc_param_t* blcRes;
#endif
} RkAiqAlgoProcResBlc;

typedef struct _RkAiqAlgoProcResDpcc {
    RkAiqAlgoResCom res_com;
#if USE_NEWSTRUCT
    dpc_param_t* dpcRes;
#endif
} RkAiqAlgoProcResDpcc;

#endif
