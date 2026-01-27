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

#ifndef _RK_AIQ_API_PRIVATE_C_H_
#define _RK_AIQ_API_PRIVATE_C_H_

#include "common/rk_aiq_comm.h"
#include "common/rk_aiq.h"
#include "common/panorama_stitchingApp.h"
#include "scene/scene_manager.h"
#include "RkAiqCalibDbV2.h"
#include "aiq_core_c/aiq_core.h"
#include "RkAiqManager_c.h"
#include "uAPI2/rk_aiq_user_api2_camgroup.h"
#include "socket_client.h"
#include "RkAiqCamGroupManager_c.h"
#include "common/panorama_stitchingApp.h"

RKAIQ_BEGIN_DECLARE

typedef enum {
    CTX_TYPE_USER_MAIN      = 0,
    CTX_TYPE_TOOL_SERVER    = 1,
    CTX_TYPE_NULL           = -255,
} rk_aiq_ctx_type_e;

typedef struct rk_aiq_sys_ctx_s {
    rk_aiq_cam_type_t cam_type;
    RkAiqAlgoDescription* ae_desc;
    bool _isCustomAe;
    RkAiqAlgoDescription* awb_desc;
    const char* _sensor_entity_name;
    AiqManager_t* _rkAiqManager;
    AiqCamHwBase_t* _camHw;
    AiqCore_t* _analyzer;
    int _isp_hw_ver;
    bool _is_1608_sensor;  /*< for sensor sync mode verify. */

    SocketClientCtx_t* _socket;
    AiqMutex_t _apiMutex;
    CamCalibDbProj_t* _calibDbProj;

    rk_aiq_ctx_type_e ctx_type;
    rk_aiq_sys_ctx_t* next_ctx;
#ifdef RKAIQ_ENABLE_CAMGROUP
    AiqCamGroupManager_t* _camGroupManager;
#endif
    int _camPhyId;
    AiqHwInfo_t _hw_info;
    int _use_fakecam;
    rk_aiq_raw_prop_t _raw_prop;
    bool _use_rkrawstream;
    rk_aiq_control_preinit_t *_ctrlPreinit_info;
    FILE* _lock_file;
    bool _use_aiisp;
    uint16_t _wr_linecnt_now;
} rk_aiq_sys_ctx_t;

/**
 * gcc-4.4.7 disallow typedef redefinition
 * error: redefinition of typedef 'RKAiqAecExpInfo_t' with include/uAPI2/rk_aiq_user_api2_xxxx.h
 */
#ifndef RK_AIQ_SYS_CTX_T
#define RK_AIQ_SYS_CTX_T
typedef struct rk_aiq_sys_ctx_s rk_aiq_sys_ctx_t;
#endif

typedef struct rk_aiq_camgroup_ctx_s {
#ifdef RKAIQ_ENABLE_CAMGROUP
    rk_aiq_cam_type_t cam_type;
    RkAiqAlgoDescription* ae_desc;
    bool _isCustomAe;
    RkAiqAlgoDescription* awb_desc;
    rk_aiq_sys_ctx_t* cam_ctxs_array[RK_AIQ_CAM_GROUP_MAX_CAMS];
    int sns_ids_array[RK_AIQ_CAM_GROUP_MAX_CAMS];
    int cam_ctxs_num;
    int cam_1608_num;
    AiqCamGroupManager_t* cam_group_manager;
    AiqMutex_t _apiMutex;
    CamCalibDbCamgroup_t* _camgroup_calib;
    struct RK_PS_SrcOverlapMap* _srcOverlapMap_s;
#else
    void* place_holder;
#endif
} rk_aiq_camgroup_ctx_t;

rk_aiq_sys_ctx_t* get_next_ctx(const rk_aiq_sys_ctx_t* ctx);
rk_aiq_camgroup_ctx_t* get_binded_group_ctx(const rk_aiq_sys_ctx_t* ctx);

bool is_ctx_need_bypass(const rk_aiq_sys_ctx_t* ctx);
void rk_aiq_ctx_set_tool_mode(const rk_aiq_sys_ctx_t* ctx, bool status);

#define CHECK_USER_API_ENABLE2(ctx) \
    if (is_ctx_need_bypass(ctx)) { return XCAM_RETURN_NO_ERROR; }

#define RKAIQ_API_SMART_LOCK(ctx)

#define CHECK_USER_API_ENABLE(mask)

RKAIQ_END_DECLARE

#endif
