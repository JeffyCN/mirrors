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

#ifndef _AIQ_MANUAL_PARAMS_MANAGER_C_H_
#define _AIQ_MANUAL_PARAMS_MANAGER_C_H_

#include "rk_aiq_comm.h"
#include "aiq_mutex.h"
#include "common/rk_aiq_types_priv_c.h"
#include "uAPI2/rk_aiq_user_api_common.h"
#include "ipc/socket_client.h"

RKAIQ_BEGIN_DECLARE

// paramsManager
// Mode: Full Manual, Non Full Manual
// Full Manual: allocate and init module params (only part of manual, same as
//              result buf)
// Non Full:    module params buf are from calib ctx (contains maunual and auto
//              params buf)
// ops: get/set/query by app; get/set/clear by handler

// opMode/en/bypass update
// auto/manual params update

// aec,awb,af
// hw: stats_cfg(path,win,weight), sw: algos

// global struct for part of xx_api_attrib, manage everythings
// except for params Auto
typedef struct rk_aiq_global_params_wrap_s {
    // input & output params
    rk_aiq_op_mode_t opMode;
    bool en;
    bool bypass;
    // input params
    int type;
    int man_param_size;
    void* man_param_ptr;
    int aut_param_size;
    void* aut_param_ptr;
} rk_aiq_global_params_wrap_t;

typedef struct rk_aiq_global_params_ptr_wrap_s {
    rk_aiq_op_mode_t* opMode;
    bool* en;
    bool* bypass;
    void* man_param_ptr;
    void* aut_param_ptr;
} rk_aiq_global_params_ptr_wrap_t;

typedef struct AiqManager_s AiqManager_t;

typedef struct GlobalParamsManager_s {
    // current
    rk_aiq_global_params_ptr_wrap_t mGlobalParams[RESULT_TYPE_MAX_PARAM];
    // storage for full manual params
    AiqFullParams_t* mFullManualParamsProxy;
    // for fast access
    aiq_params_base_t* mFullManualParamsProxyArray[RESULT_TYPE_MAX_PARAM];
    // mutex between set attrib and algo processing
    AiqMutex_t mAlgoMutex[RESULT_TYPE_MAX_PARAM];

    AiqMutex_t mMutex;
    bool mIsHold;
    bool mFullManualMode;
    CamCalibDbV2Context_t* mCalibDb;
    uint64_t mIsGlobalModulesUpdateBits;
    AiqManager_t* rkAiqManager;
    SocketClientCtx_t* _socket;
    btnr_pixDomain_mode_t mBtnrPixDomainMode;
#if RKAIQ_HAVE_YUVME
    bool yme_init_enable;
#endif
#if defined(ISP_HW_V33) || defined(ISP_HW_V35)
    bool btnr_init_enable;
#endif
} GlobalParamsManager_t;

XCamReturn GlobalParamsManager_init(GlobalParamsManager_t* pMan, bool isFullManMode, CamCalibDbV2Context_t* calibDb);
void GlobalParamsManager_deinit(GlobalParamsManager_t* pMan);
XCamReturn GlobalParamsManager_switchCalibDb(GlobalParamsManager_t* pMan, CamCalibDbV2Context_t* calibDb, bool need_check);
XCamReturn GlobalParamsManager_set(GlobalParamsManager_t* pMan, rk_aiq_global_params_wrap_t* params);
XCamReturn GlobalParamsManager_get(GlobalParamsManager_t* pMan, rk_aiq_global_params_wrap_t* params);
XCamReturn GlobalParamsManager_getAndClearPending(GlobalParamsManager_t* pMan, rk_aiq_global_params_wrap_t* params);
XCamReturn GlobalParamsManager_getAndClearPendingLocked(GlobalParamsManager_t* pMan, rk_aiq_global_params_wrap_t* params);
aiq_params_base_t* GlobalParamsManager_getAndClearPending2(GlobalParamsManager_t* pMan, int type);
bool GlobalParamsManager_isManual(GlobalParamsManager_t* pMan, int type);
bool GlobalParamsManager_isManualLocked(GlobalParamsManager_t* pMan, int type);
void GlobalParamsManager_hold(GlobalParamsManager_t* pMan, bool hold);
bool GlobalParamsManager_isFullManualMode(GlobalParamsManager_t* pMan);
AiqFullParams_t* GlobalParamsManager_getFullManParamsProxy(GlobalParamsManager_t* pMan);
void GlobalParamsManager_lockAlgoParam(GlobalParamsManager_t* pMan, int type);
void GlobalParamsManager_unlockAlgoParam(GlobalParamsManager_t* pMan, int type);
bool GlobalParamsManager_getAndClearAlgoParamUpdateFlagLocked(GlobalParamsManager_t* pMan, int type);
XCamReturn GlobalParamsManager_checkAlgoEnableBypass(GlobalParamsManager_t* pMan, int type, bool* en, bool* bypass);
#if ISP_HW_V35
XCamReturn GlobalParamsManager_checkStatsrc(GlobalParamsManager_t* pMan, void* calib, int type);
#endif
#define GlobalParamsManager_setManager(pMan, pRkAiqManager) \
        (pMan)->rkAiqManager = pRkAiqManager
#define GlobalParamsManager_setSocket(pMan, socket) \
        (pMan)->_socket = socket

XCamReturn GlobalParamsManager_get_ModuleEn(GlobalParamsManager_t* pMan, rk_aiq_module_list_t* mod);
XCamReturn GlobalParamsManager_set_ModuleEn(GlobalParamsManager_t* pMan, rk_aiq_module_list_t* mod);
bool GlobalParamsManager_get_SingleModuleEn(GlobalParamsManager_t* pMan, int type);

RKAIQ_END_DECLARE
#endif

