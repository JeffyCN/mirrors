/*
 *rk_aiq_types_alsc_algo_int.h
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

#ifndef _RK_AIQ_TYPE_ABAYERTNRV30_ALGO_INT_H_
#define _RK_AIQ_TYPE_ABAYERTNRV30_ALGO_INT_H_

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "rk_aiq_comm.h"
#include "RkAiqCalibDbTypes.h"
#include "abayertnrV30/rk_aiq_types_abayertnr_algo_v30.h"
#include "bayertnr_head_v30.h"
#include "bayertnr_uapi_head_v30.h"


//RKAIQ_BEGIN_DECLARE

#define ABAYERTNRV30_RECALCULATE_DELTA_ISO (10)
#define RK_BAYERNR_V30_MAX_ISO_NUM (CALIBDB_MAX_ISO_LEVEL)

typedef enum Abayertnr_result_V30_e {
    ABAYERTNRV30_RET_SUCCESS             = 0,   // this has to be 0, if clauses rely on it
    ABAYERTNRV30_RET_FAILURE             = 1,   // process failure
    ABAYERTNRV30_RET_INVALID_PARM        = 2,   // invalid parameter
    ABAYERTNRV30_RET_WRONG_CONFIG        = 3,   // feature not supported
    ABAYERTNRV30_RET_BUSY                = 4,   // there's already something going on...
    ABAYERTNRV30_RET_CANCELED            = 5,   // operation canceled
    ABAYERTNRV30_RET_OUTOFMEM            = 6,   // out of memory
    ABAYERTNRV30_RET_OUTOFRANGE          = 7,   // parameter/value out of range
    ABAYERTNRV30_RET_NULL_POINTER        = 8,   // the/one/all parameter(s) is a(are) NULL pointer(s)
    ABAYERTNRV30_RET_DIVISION_BY_ZERO    = 9,   // a divisor equals ZERO
    ABAYERTNRV30_RET_NO_INPUTIMAGE       = 10   // no input image
} Abayertnr_result_V30_t;

typedef enum Abayertnr_State_V30_e {
    ABAYERTNRV30_STATE_INVALID           = 0,                   /**< initialization value */
    ABAYERTNRV30_STATE_INITIALIZED       = 1,                   /**< instance is created, but not initialized */
    ABAYERTNRV30_STATE_STOPPED           = 2,                   /**< instance is confiured (ready to start) or stopped */
    ABAYERTNRV30_STATE_RUNNING           = 3,                   /**< instance is running (processes frames) */
    ABAYERTNRV30_STATE_LOCKED            = 4,                   /**< instance is locked (for taking snapshots) */
    ABAYERTNRV30_STATE_MAX                                      /**< max */
} Abayertnr_State_V30_t;

typedef enum Abayertnr_OPMode_V30_e {
    ABAYERTNRV30_OP_MODE_INVALID           = 0,                   /**< initialization value */
    ABAYERTNRV30_OP_MODE_AUTO              = 1,                   /**< instance is created, but not initialized */
    ABAYERTNRV30_OP_MODE_MANUAL            = 2,                   /**< instance is confiured (ready to start) or stopped */
    ABAYERTNRV30_OP_MODE_REG_MANUAL        = 3,
    ABAYERTNRV30_OP_MODE_MAX                                      /**< max */
} Abayertnr_OPMode_V30_t;

typedef enum Abayertnr_ParamMode_V30_e {
    ABAYERTNRV30_PARAM_MODE_INVALID           = 0,
    ABAYERTNRV30_PARAM_MODE_NORMAL          = 1,                   /**< initialization value */
    ABAYERTNRV30_PARAM_MODE_HDR              = 2,                   /**< instance is created, but not initialized */
    ABAYERTNRV30_PARAM_MODE_GRAY            = 3,                   /**< instance is confiured (ready to start) or stopped */
    ABAYERTNRV30_PARAM_MODE_MAX                                      /**< max */
} Abayertnr_ParamMode_V30_t;


typedef struct RK_Bayertnr_Params_V30_s
{
    bool enable;
    float iso[RK_BAYERNR_V30_MAX_ISO_NUM];
    RK_Bayertnr_Params_V30_Select_t bayertnrParamISO[RK_BAYERNR_V30_MAX_ISO_NUM];
} RK_Bayertnr_Params_V30_t;


typedef struct Abayertnr_Manual_Attr_V30_s
{
    RK_Bayertnr_Params_V30_Select_t st3DSelect;

    RK_Bayertnr_Fix_V30_t st3DFix;
} Abayertnr_Manual_Attr_V30_t;



typedef struct Abayertnr_Auto_Attr_V30_s
{
    RK_Bayertnr_Params_V30_t st3DParams;
    RK_Bayertnr_Params_V30_Select_t st3DSelect;

} Abayertnr_Auto_Attr_V30_t;



typedef struct Abayertnr_ProcResult_V30_s {
    int bayernr3DEn;

    //for sw simultaion
    //RK_Bayertnr_Params_V30_Select_t st3DSelect;


    //for hw register
    RK_Bayertnr_Fix_V30_t* st3DFix;

} Abayertnr_ProcResult_V30_t;


typedef struct Abayertnr_Config_V30_s {
    Abayertnr_State_V30_t eState;
    Abayertnr_OPMode_V30_t eMode;
    int rawWidth;
    int rawHeight;
} Abayertnr_Config_V30_t;


typedef struct rk_aiq_bayertnr_attrib_v30_s {
    rk_aiq_uapi_sync_t sync;
    Abayertnr_OPMode_V30_t eMode;
    Abayertnr_Auto_Attr_V30_t stAuto;
    Abayertnr_Manual_Attr_V30_t stManual;
} rk_aiq_bayertnr_attrib_v30_t;

typedef struct rk_aiq_bayertnr_strength_v30_s {
    rk_aiq_uapi_sync_t sync;
    float percent;
    bool strength_enable;
} rk_aiq_bayertnr_strength_v30_t;



//RKAIQ_END_DECLARE

#endif

