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

#ifndef _RK_AIQ_TYPE_AYUVME_ALGO_INT_V1_H_
#define _RK_AIQ_TYPE_AYUVME_ALGO_INT_V1_H_

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "rk_aiq_comm.h"
#include "RkAiqCalibDbTypes.h"
#include "ayuvmeV1/rk_aiq_types_ayuvme_algo_v1.h"
#include "yuvme_head_v1.h"
#include "yuvme_uapi_head_v1.h"


//RKAIQ_BEGIN_DECLARE
#define RK_YUVME_V1_MAX_ISO_NUM (CALIBDB_MAX_ISO_LEVEL)


#define AYUVMEV1_RECALCULATE_DELTA_ISO       (10)
#define YUVME_V1_SIGMA_BITS                  10
#define YUVME_V1_NOISE_SIGMA_FIX_BIT              3
#define LOG2(x)                             (log((double)x)                 / log((double)2))


typedef enum Ayuvme_result_V1_e {
    AYUVMEV1_RET_SUCCESS             = 0,   // this has to be 0, if clauses rely on it
    AYUVMEV1_RET_FAILURE             = 1,   // process failure
    AYUVMEV1_RET_INVALID_PARM        = 2,   // invalid parameter
    AYUVMEV1_RET_WRONG_CONFIG        = 3,   // feature not supported
    AYUVMEV1_RET_BUSY                = 4,   // there's already something going on...
    AYUVMEV1_RET_CANCELED            = 5,   // operation canceled
    AYUVMEV1_RET_OUTOFMEM            = 6,   // out of memory
    AYUVMEV1_RET_OUTOFRANGE          = 7,   // parameter/value out of range
    AYUVMEV1_RET_NULL_POINTER        = 8,   // the/one/all parameter(s) is a(are) NULL pointer(s)
    AYUVMEV1_RET_DIVISION_BY_ZERO    = 9,   // a divisor equals ZERO
    AYUVMEV1_RET_NO_INPUTIMAGE       = 10   // no input image
} Ayuvme_result_V1_t;

typedef enum Ayuvme_State_V1_e {
    AYUVMEV1_STATE_INVALID           = 0,                   /**< initialization value */
    AYUVMEV1_STATE_INITIALIZED       = 1,                   /**< instance is created, but not initialized */
    AYUVMEV1_STATE_STOPPED           = 2,                   /**< instance is confiured (ready to start) or stopped */
    AYUVMEV1_STATE_RUNNING           = 3,                   /**< instance is running (processes frames) */
    AYUVMEV1_STATE_LOCKED            = 4,                   /**< instance is locked (for taking snapshots) */
    AYUVMEV1_STATE_MAX                                      /**< max */
} Ayuvme_State_V1_t;

typedef enum Ayuvme_OPMode_V1_e {
    AYUVMEV1_OP_MODE_INVALID           = 0,                   /**< initialization value */
    AYUVMEV1_OP_MODE_AUTO              = 1,                   /**< instance is created, but not initialized */
    AYUVMEV1_OP_MODE_MANUAL            = 2,                   /**< instance is confiured (ready to start) or stopped */
    AYUVMEV1_OP_MODE_REG_MANUAL        = 3,
    AYUVMEV1_OP_MODE_MAX                                      /**< max */
} Ayuvme_OPMode_V1_t;

typedef enum Ayuvme_ParamMode_V1_e {
    AYUVMEV1_PARAM_MODE_INVALID           = 0,
    AYUVMEV1_PARAM_MODE_NORMAL          = 1,                   /**< initialization value */
    AYUVMEV1_PARAM_MODE_HDR              = 2,                   /**< instance is created, but not initialized */
    AYUVMEV1_PARAM_MODE_GRAY            = 3,                   /**< instance is confiured (ready to start) or stopped */
    AYUVMEV1_PARAM_MODE_MAX                                      /**< max */
} Ayuvme_ParamMode_V1_t;



typedef struct RK_YUVME_Params_V1_s
{
    int enable;
    char version[64];
    float iso[RK_YUVME_V1_MAX_ISO_NUM];
    RK_YUVME_Params_V1_Select_t arYuvmeParamsISO[RK_YUVME_V1_MAX_ISO_NUM];

} RK_YUVME_Params_V1_t;


typedef struct Ayuvme_Manual_Attr_V1_s
{
    RK_YUVME_Params_V1_Select_t stSelect;

    RK_YUVME_Fix_V1_t stFix;

} Ayuvme_Manual_Attr_V1_t;

typedef struct Ayuvme_Auto_Attr_V1_s
{
    //all ISO params and select param

    RK_YUVME_Params_V1_t stParams;
    RK_YUVME_Params_V1_Select_t stSelect;

} Ayuvme_Auto_Attr_V1_t;

typedef struct Ayuvme_ProcResult_V1_s {

    //for sw simultaion
    //RK_YUVME_Params_V1_Select_t stSelect;

    //for hw register
    RK_YUVME_Fix_V1_t* stFix;
} Ayuvme_ProcResult_V1_t;


typedef struct Ayuvme_Config_V1_s {
    Ayuvme_State_V1_t eState;
    Ayuvme_OPMode_V1_t eMode;
    int rawHeight;
    int rawWidth;
} Ayuvme_Config_V1_t;


typedef struct rk_aiq_yuvme_attrib_v1_s {
    /*
     * @sync (param in): flags for param update mode,
     *     true for sync, false for async.
     * @done (parsm out): flags for param update status,
     *     true indicate param has been updated, false
     *     indicate param has not been updated.
     */
    rk_aiq_uapi_sync_t sync;

    Ayuvme_OPMode_V1_t eMode;
    Ayuvme_Auto_Attr_V1_t stAuto;
    Ayuvme_Manual_Attr_V1_t stManual;
} rk_aiq_yuvme_attrib_v1_t;


typedef struct rk_aiq_yuvme_strength_v1_s {
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
} rk_aiq_yuvme_strength_v1_t;





//calibdb


//RKAIQ_END_DECLARE

#endif

