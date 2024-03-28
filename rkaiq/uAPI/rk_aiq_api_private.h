
#ifndef _RK_AIQ_API_PRIVATE
#define _RK_AIQ_API_PRIVATE

#include "xcore/smartptr.h"
#include "rk_aiq_user_api_sysctl.h"
#include "RkAiqManager.h"
#include "socket_server.h"
#include "RkAiqCalibDbV2.h"
#include "scene/scene_manager.h"
#include "ICamHw.h"
#include "RkAiqCamGroupManager.h"
#include "common/panorama_stitchingApp.h"

using namespace RkCam;
using namespace XCam;

extern "C" {

typedef enum {
    CTX_TYPE_USER_MAIN      = 0,
    CTX_TYPE_TOOL_SERVER    = 1,
    CTX_TYPE_NULL           = -255,
} rk_aiq_ctx_type_e;

typedef struct rk_aiq_sys_ctx_s {
    rk_aiq_cam_type_t cam_type;
    const char* _sensor_entity_name;
    SmartPtr<RkAiqManager> _rkAiqManager;
    SmartPtr<ICamHw> _camHw;
    SmartPtr<RkAiqCore> _analyzer;
#ifdef ISP_HW_V20
    SmartPtr<RkLumaCore> _lumaAnalyzer;
#endif
#ifdef RKAIQ_ENABLE_PARSER_V1
    CamCalibDbContext_t *_calibDb;
#endif
    int _isp_hw_ver;
    bool _is_1608_sensor;  /*< for sensor sync mode verify. */

    SocketServer *  _socket;
    SmartPtr<Mutex> _apiMutex;
    CamCalibDbProj_t* _calibDbProj;

    rk_aiq_ctx_type_e ctx_type;
    rk_aiq_sys_ctx_t* next_ctx;
#ifdef RKAIQ_ENABLE_CAMGROUP
    RkAiqCamGroupManager* _camGroupManager;
#endif
    int _camPhyId;
    struct RkAiqHwInfo _hw_info;
    int _use_fakecam;
    rk_aiq_raw_prop_t _raw_prop;
    bool _use_rkrawstream;
    rk_aiq_rkrawstream_info_t *_rawstream_info;
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
    rk_aiq_sys_ctx_t* cam_ctxs_array[RK_AIQ_CAM_GROUP_MAX_CAMS];
    int sns_ids_array[RK_AIQ_CAM_GROUP_MAX_CAMS];
    int cam_ctxs_num;
    int cam_1608_num;
    SmartPtr<RkAiqCamGroupManager> cam_group_manager;
    SmartPtr<Mutex> _apiMutex;
    CamCalibDbCamgroup_t* _camgroup_calib;
    RK_PS_SrcOverlapMap* _srcOverlapMap_s;
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

#define RKAIQ_NO_API_LOCK
#ifndef RKAIQ_NO_API_LOCK
#ifdef RKAIQ_ENABLE_CAMGROUP
#define RKAIQ_API_SMART_LOCK(ctx) \
    const rk_aiq_camgroup_ctx_t* lock_group_ctx = NULL; \
    if (ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) \
        lock_group_ctx = (const rk_aiq_camgroup_ctx_t*)ctx; \
    SmartLock lock (lock_group_ctx ? *lock_group_ctx->_apiMutex.ptr() : *ctx->_apiMutex.ptr());
#else
#define RKAIQ_API_SMART_LOCK(ctx) \
    SmartLock lock (*ctx->_apiMutex.ptr());
#endif
#else
#define RKAIQ_API_SMART_LOCK(ctx)
#endif

}

#endif
