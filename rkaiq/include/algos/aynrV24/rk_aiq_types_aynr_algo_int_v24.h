/*
 *rk_aiq_types_aynr_algo_int_v24.h
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

#ifndef _RK_AIQ_TYPE_AYNR_ALGO_INT_V24_H_
#define _RK_AIQ_TYPE_AYNR_ALGO_INT_V24_H_

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "RkAiqCalibDbTypes.h"
#include "aynrV24/rk_aiq_types_aynr_algo_v24.h"
#include "rk_aiq_comm.h"
#include "ynr_head_v24.h"
#include "ynr_uapi_head_v24.h"

// RKAIQ_BEGIN_DECLARE
#define RK_YNR_V24_MAX_ISO_NUM (CALIBDB_MAX_ISO_LEVEL)

#define AYNRV24_RECALCULATE_DELTA_ISO (10)
#define YNR_V24_SIGMA_BITS            10
#define YNR_V24_NOISE_SIGMA_FIX_BIT   3
#define LOG2(x)                       (log((double)x) / log((double)2))

typedef enum Aynr_result_V24_e {
    AYNRV24_RET_SUCCESS          = 0,  // this has to be 0, if clauses rely on it
    AYNRV24_RET_FAILURE          = 1,  // process failure
    AYNRV24_RET_INVALID_PARM     = 2,  // invalid parameter
    AYNRV24_RET_WRONG_CONFIG     = 3,  // feature not supported
    AYNRV24_RET_BUSY             = 4,  // there's already something going on...
    AYNRV24_RET_CANCELED         = 5,  // operation canceled
    AYNRV24_RET_OUTOFMEM         = 6,  // out of memory
    AYNRV24_RET_OUTOFRANGE       = 7,  // parameter/value out of range
    AYNRV24_RET_NULL_POINTER     = 8,  // the/one/all parameter(s) is a(are) NULL pointer(s)
    AYNRV24_RET_DIVISION_BY_ZERO = 9,  // a divisor equals ZERO
    AYNRV24_RET_NO_INPUTIMAGE    = 10  // no input image
} Aynr_result_V24_t;

typedef enum Aynr_State_V24_e {
    AYNRV24_STATE_INVALID     = 0, /**< initialization value */
    AYNRV24_STATE_INITIALIZED = 1, /**< instance is created, but not initialized */
    AYNRV24_STATE_STOPPED     = 2, /**< instance is confiured (ready to start) or stopped */
    AYNRV24_STATE_RUNNING     = 3, /**< instance is running (processes frames) */
    AYNRV24_STATE_LOCKED      = 4, /**< instance is locked (for taking snapshots) */
    AYNRV24_STATE_MAX              /**< max */
} Aynr_State_V24_t;

typedef enum Aynr_OPMode_V24_e {
    AYNRV24_OP_MODE_INVALID    = 0, /**< initialization value */
    AYNRV24_OP_MODE_AUTO       = 1, /**< instance is created, but not initialized */
    AYNRV24_OP_MODE_MANUAL     = 2, /**< instance is confiured (ready to start) or stopped */
    AYNRV24_OP_MODE_REG_MANUAL = 3,
    AYNRV24_OP_MODE_MAX /**< max */
} Aynr_OPMode_V24_t;

typedef enum Aynr_ParamMode_V24_e {
    AYNRV24_PARAM_MODE_INVALID = 0,
    AYNRV24_PARAM_MODE_NORMAL  = 1, /**< initialization value */
    AYNRV24_PARAM_MODE_HDR     = 2, /**< instance is created, but not initialized */
    AYNRV24_PARAM_MODE_GRAY    = 3, /**< instance is confiured (ready to start) or stopped */
    AYNRV24_PARAM_MODE_MAX          /**< max */
} Aynr_ParamMode_V24_t;

typedef struct RK_YNR_Params_V24_s {
    int enable;
    char version[64];
    float iso[RK_YNR_V24_MAX_ISO_NUM];
    RK_YNR_Params_V24_Select_t arYnrParamsISO[RK_YNR_V24_MAX_ISO_NUM];
} RK_YNR_Params_V24_t;

typedef struct Aynr_Manual_Attr_V24_s {
    RK_YNR_Params_V24_Select_t stSelect;

    RK_YNR_Fix_V24_t stFix;

} Aynr_Manual_Attr_V24_t;

typedef struct Aynr_Auto_Attr_V24_s {
    // all ISO params and select param

    RK_YNR_Params_V24_t stParams;
    RK_YNR_Params_V24_Select_t stSelect;

} Aynr_Auto_Attr_V24_t;

typedef struct Aynr_ProcResult_V24_s {
    // for sw simultaion
    RK_YNR_Params_V24_Select_t* stSelect;

    // for hw register
    RK_YNR_Fix_V24_t* stFix;

    //bool isNeedUpdate;

} Aynr_ProcResult_V24_t;

typedef struct Aynr_Config_V24_s {
    Aynr_State_V24_t eState;
    Aynr_OPMode_V24_t eMode;
    int rawHeight;
    int rawWidth;
} Aynr_Config_V24_t;

typedef struct rk_aiq_ynr_attrib_v24_s {
    /*
     * @sync (param in): flags for param update mode,
     *     true for sync, false for async.
     * @done (parsm out): flags for param update status,
     *     true indicate param has been updated, false
     *     indicate param has not been updated.
     */
    rk_aiq_uapi_sync_t sync;

    Aynr_OPMode_V24_t eMode;
    Aynr_Auto_Attr_V24_t stAuto;
    Aynr_Manual_Attr_V24_t stManual;
} rk_aiq_ynr_attrib_v24_t;

typedef struct rk_aiq_ynr_strength_v24_s {
    /*
     * @sync (param in): flags for param update mode,
     *     true for sync, false for async.
     * @done (parsm out): flags for param update status,
     *     true indicate param has been updated, false
     *     indicate param has not been updated.
     */
    rk_aiq_uapi_sync_t sync;

    float percent;
    bool strength_enable;
} rk_aiq_ynr_strength_v24_t;

// calibdb

// RKAIQ_END_DECLARE

#endif
