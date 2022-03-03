/*
 * Copyright (c) 2019-2021 Rockchip Eletronics Co., Ltd.
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
 */
#include "rk_aiq_user_api_sysctl.h"
#include "rk_aiq_api_private.h"
#include "RkAiqManager.h"
#include "socket_server.h"
#include "RkAiqCalibDbV2.h"
#include "scene/scene_manager.h"
#include "rkaiq_ini.h"
#ifdef RK_SIMULATOR_HW
#include "simulator/CamHwSimulator.h"
#else
#include "isp20/CamHwIsp20.h"
#include "fakecamera/FakeCamHwIsp20.h"
#include "isp20/Isp20_module_dbg.h"
#include "isp21/CamHwIsp21.h"
#include "isp3x/CamHwIsp3x.h"
#endif

using namespace RkCam;
using namespace XCam;

#define RKAIQSYS_CHECK_RET(cond, ret, format, ...) \
    if ((cond)) { \
        LOGE(format, ##__VA_ARGS__); \
        return ret; \
    }

#define RKAIQ_DEFAULT_IQ_PATH "/etc/iqfiles/"

RKAIQ_BEGIN_DECLARE

int g_rkaiq_isp_hw_ver = 0;

rk_aiq_sys_ctx_t* get_next_ctx(const rk_aiq_sys_ctx_t* ctx)
{
    if (ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP)
        return nullptr;
    else
        return ctx->next_ctx;
}

rk_aiq_camgroup_ctx_t* get_binded_group_ctx(const rk_aiq_sys_ctx_t* ctx)
{
#ifdef RKAIQ_ENABLE_CAMGROUP
    if (ctx->_camGroupManager)
        return (rk_aiq_camgroup_ctx_t*)ctx->_camGroupManager->getContainerCtx();
    else
#endif
    return NULL;
}

void rk_aiq_ctx_set_tool_mode(const rk_aiq_sys_ctx_t* ctx, bool status)
{
    if (!ctx)
        return;

    if (ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)ctx;
        for (auto camCtx : camgroup_ctx->cam_ctxs_array)
            if(camCtx && camCtx->_socket)
                camCtx->_socket->tool_mode_set(status);
#endif
    } else if(ctx->_socket) {
        ctx->_socket->tool_mode_set(status);
    }
}

bool is_ctx_need_bypass(const rk_aiq_sys_ctx_t* ctx)
{
    if (!ctx)
        return true;

    /* TODO: remove the uapi enable check for the tool that configure the uapi in real time */
    return false;

    if (ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)ctx;
        for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
            if(camCtx && camCtx->_socket) {
                if (camCtx->_socket->is_connected() && \
                        camCtx->ctx_type != CTX_TYPE_TOOL_SERVER) {
                    return true;
                }
            }
        }
    } else {
        if(ctx->_socket) {
            if (ctx->_socket->is_connected() && \
                    ctx->ctx_type != CTX_TYPE_TOOL_SERVER) {
                return true;
            }
        }
    }

    return false;
}


typedef struct rk_aiq_sys_preinit_cfg_s {
    rk_aiq_working_mode_t mode;
    std::string force_iq_file;
    std::string main_scene;
    std::string sub_scene;
    rk_aiq_hwevt_cb hwevt_cb;
    void* hwevt_cb_ctx;
    rk_aiq_sys_preinit_cfg_s() {
        hwevt_cb = NULL;
        hwevt_cb_ctx = NULL;
    };
} rk_aiq_sys_preinit_cfg_t;

static std::map<std::string, rk_aiq_sys_preinit_cfg_t> g_rk_aiq_sys_preinit_cfg_map;

XCamReturn
rk_aiq_uapi_sysctl_preInit(const char* sns_ent_name,
                           rk_aiq_working_mode_t mode,
                           const char* force_iq_file)
{
    std::string sns_ent_name_str(sns_ent_name);
    rk_aiq_sys_preinit_cfg_t cfg;

    cfg.mode = mode;
    if (force_iq_file)
        cfg.force_iq_file = force_iq_file;
    g_rk_aiq_sys_preinit_cfg_map[sns_ent_name_str] = cfg;

    return XCAM_RETURN_NO_ERROR;
}


XCamReturn
rk_aiq_uapi_sysctl_preInit_scene(const char* sns_ent_name, const char *main_scene,
                                 const char *sub_scene)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (!sns_ent_name || !main_scene || !sub_scene) {
        LOGE("Invalid input parameter");
        return XCAM_RETURN_ERROR_PARAM;
    }

    std::string sns_ent_name_str(sns_ent_name);

    LOGI("main_scene: %s, sub_scene: %s", main_scene, sub_scene);
    g_rk_aiq_sys_preinit_cfg_map[sns_ent_name_str].main_scene   = main_scene;
    g_rk_aiq_sys_preinit_cfg_map[sns_ent_name_str].sub_scene    = sub_scene;

    return (ret);
}

static int rk_aiq_offline_init(rk_aiq_sys_ctx_t* ctx)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    char* use_as_fake_cam_env = getenv("USE_AS_FAKE_CAM");
    ini_t* aiq_ini = rkaiq_ini_load(OFFLINE_INI_FILE);

    ENTER_XCORE_FUNCTION();

    if (aiq_ini) {
        const char* raw_offline_str = rkaiq_ini_get(aiq_ini, "rkaiq", "offline");
        const char* raw_w_str = rkaiq_ini_get(aiq_ini, "rkaiq", "width");
        const char* raw_h_str = rkaiq_ini_get(aiq_ini, "rkaiq", "height");
        const char* raw_fmt_str = rkaiq_ini_get(aiq_ini, "rkaiq", "format");

        bool offline = atoi(raw_offline_str) > 0 ? true : false;
        int raw_w = atoi(raw_w_str);
        int raw_h = atoi(raw_h_str);

        // valid offline mode
        if (offline && raw_w && raw_h && raw_fmt_str) {
            ctx->_raw_prop.frame_width = raw_w;
            ctx->_raw_prop.frame_height = raw_h;
            ctx->_raw_prop.rawbuf_type = RK_AIQ_RAW_FILE;
            ctx->_use_fakecam = true;

            if (strcmp(raw_fmt_str, "BG10") == 0)
                ctx->_raw_prop.format = RK_PIX_FMT_SBGGR10;
            else if (strcmp(raw_fmt_str, "GB10") == 0)
                ctx->_raw_prop.format = RK_PIX_FMT_SGBRG10;
            else if (strcmp(raw_fmt_str, "RG10") == 0)
                ctx->_raw_prop.format = RK_PIX_FMT_SRGGB10;
            else if (strcmp(raw_fmt_str, "BA10") == 0)
                ctx->_raw_prop.format = RK_PIX_FMT_SGRBG10;
            else if (strcmp(raw_fmt_str, "BG12") == 0)
                ctx->_raw_prop.format = RK_PIX_FMT_SBGGR12;
            else if (strcmp(raw_fmt_str, "GB12") == 0)
                ctx->_raw_prop.format = RK_PIX_FMT_SGBRG12;
            else if (strcmp(raw_fmt_str, "RG12") == 0)
                ctx->_raw_prop.format = RK_PIX_FMT_SRGGB12;
            else if (strcmp(raw_fmt_str, "BA12") == 0)
                ctx->_raw_prop.format = RK_PIX_FMT_SGRBG12;
            else if (strcmp(raw_fmt_str, "BG14") == 0)
                ctx->_raw_prop.format = RK_PIX_FMT_SBGGR14;
            else if (strcmp(raw_fmt_str, "GB14") == 0)
                ctx->_raw_prop.format = RK_PIX_FMT_SGBRG14;
            else if (strcmp(raw_fmt_str, "RG14") == 0)
                ctx->_raw_prop.format = RK_PIX_FMT_SRGGB14;
            else if (strcmp(raw_fmt_str, "BA14") == 0)
                ctx->_raw_prop.format = RK_PIX_FMT_SGRBG14;
            else
                ctx->_raw_prop.format = RK_PIX_FMT_SBGGR10;
        }

        rkaiq_ini_free(aiq_ini);
    }

    if (use_as_fake_cam_env)
        ctx->_use_fakecam = atoi(use_as_fake_cam_env) > 0 ? true : false;

    EXIT_XCORE_FUNCTION();

    return ret;
}

static void
rk_aiq_uapi_sysctl_deinit_locked(rk_aiq_sys_ctx_t* ctx);

rk_aiq_sys_ctx_t*
rk_aiq_uapi_sysctl_init(const char* sns_ent_name,
                        const char* config_file_dir,
                        rk_aiq_error_cb err_cb,
                        rk_aiq_metas_cb metas_cb)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ENTER_XCORE_FUNCTION();
    char config_file[256];
    std::string main_scene;
    std::string sub_scene;

    XCAM_ASSERT(sns_ent_name);

    bool is_ent_name = true;
    if (sns_ent_name[0] != 'm' || sns_ent_name[3] != '_')
        is_ent_name = false;

    if (!is_ent_name) {
        if (config_file_dir && (strlen(config_file_dir) > 0))
            sprintf(config_file, "%s/%s.xml", config_file_dir, sns_ent_name);
        else
            sprintf(config_file, "%s/%s.xml", RKAIQ_DEFAULT_IQ_PATH, sns_ent_name);
    }
    rk_aiq_sys_ctx_t* ctx = new rk_aiq_sys_ctx_t();
    RKAIQSYS_CHECK_RET(!ctx, NULL, "malloc main ctx error !");

    ctx->_apiMutex = new Mutex();
    RKAIQ_API_SMART_LOCK(ctx);

    ctx->_sensor_entity_name = strndup(sns_ent_name, 128);
    RKAIQSYS_CHECK_RET(!ctx->_sensor_entity_name, NULL, "dup sensor name error !");

    ctx->_rkAiqManager = new RkAiqManager(ctx->_sensor_entity_name,
                                          err_cb,
                                          metas_cb);
    std::map<std::string, rk_aiq_sys_preinit_cfg_t>::iterator it =
        g_rk_aiq_sys_preinit_cfg_map.find(std::string(ctx->_sensor_entity_name));
    if (it != g_rk_aiq_sys_preinit_cfg_map.end())
        ctx->_rkAiqManager->setHwEvtCb(it->second.hwevt_cb, it->second.hwevt_cb_ctx);

    rk_aiq_static_info_t* s_info = CamHwIsp20::getStaticCamHwInfo(sns_ent_name);
    ctx->_rkAiqManager->setCamPhyId(s_info->sensor_info.phyId);

    ctx->_camPhyId = s_info->sensor_info.phyId;

#ifdef RK_SIMULATOR_HW
    ctx->_camHw = new CamHwSimulator();
#else
    rk_aiq_offline_init(ctx);
    if (strstr(sns_ent_name, "FakeCamera") || ctx->_use_fakecam) {
        //ctx->_camHw = new FakeCamHwIsp20();
        if (s_info->isp_hw_ver == 4)
            ctx->_camHw = new FakeCamHwIsp20 ();
        else if (s_info->isp_hw_ver == 5)
            ctx->_camHw = new FakeCamHwIsp21 ();
        else if (s_info->isp_hw_ver == 6)
            ctx->_camHw = new FakeCamHwIsp3x ();
        else {
            LOGE("do not support this isp hw version %d !", s_info->isp_hw_ver);
            goto error;
        }
    } else {
        if (s_info->isp_hw_ver == 4)
            ctx->_camHw = new CamHwIsp20();
        else if (s_info->isp_hw_ver == 5)
            ctx->_camHw = new CamHwIsp21();
        else if (s_info->isp_hw_ver == 6)
            ctx->_camHw = new CamHwIsp3x();
        else {
            LOGE("do not support this isp hw version %d !", s_info->isp_hw_ver);
            goto error;
        }
    }
    // use user defined iq file
    {
        std::map<std::string, rk_aiq_sys_preinit_cfg_t>::iterator it =
            g_rk_aiq_sys_preinit_cfg_map.find(std::string(ctx->_sensor_entity_name));
        int user_hdr_mode = -1;
        bool user_spec_iq = false;
        if (it != g_rk_aiq_sys_preinit_cfg_map.end()) {
            if (!it->second.force_iq_file.empty()) {
                sprintf(config_file, "%s/%s", config_file_dir, it->second.force_iq_file.c_str());
                LOGI("use user sepcified iq file %s", config_file);
                user_spec_iq = true;
            } else {
                user_hdr_mode = it->second.mode;
                LOGI("selected by user sepcified hdr mode %d", user_hdr_mode);
            }

            if (!it->second.main_scene.empty())
                main_scene = it->second.main_scene;
            if (!it->second.sub_scene.empty())
                sub_scene = it->second.sub_scene;
        }

        // use auto selected iq file
        if (is_ent_name && !user_spec_iq) {
            char iq_file[128] = {'\0'};
            CamHwIsp20::selectIqFile(sns_ent_name, iq_file);

            char* hdr_mode = getenv("HDR_MODE");
            int start = strlen(iq_file) - strlen(".xml");

            if (hdr_mode) {
                iq_file[start] = '\0';
                if (strstr(hdr_mode, "32"))
                    strcat(iq_file, "-hdr3.xml");
                else
                    strcat(iq_file, "_normal.xml");
            }

            if (config_file_dir) {
                sprintf(config_file, "%s/%s", config_file_dir, iq_file);
            } else {
                sprintf(config_file, "%s/%s", RKAIQ_DEFAULT_IQ_PATH, iq_file);
            }

            // use default iq file
            if (hdr_mode && access(config_file, F_OK)) {
                LOGW("%s not exist, will use the default !", config_file);
                if (strstr(hdr_mode, "32"))
                    start = strlen(config_file) - strlen("-hdr3.xml");
                else
                    start = strlen(config_file) - strlen("_normal.xml");
                config_file[start] = '\0';
                strcat(config_file, ".xml");
            }
            LOGI("use iq file %s", config_file);
        }
    }
#endif
    ctx->_camHw->setCamPhyId(s_info->sensor_info.phyId);
    ctx->_rkAiqManager->setCamHw(ctx->_camHw);
    if (s_info->isp_hw_ver == 4)
        ctx->_analyzer = new RkAiqCore(0);
    else if (s_info->isp_hw_ver == 5)
        ctx->_analyzer = new RkAiqCore(1);
    else if (s_info->isp_hw_ver == 6)
        ctx->_analyzer = new RkAiqCore(3);

#ifndef RK_SIMULATOR_HW
    ctx->_hw_info.fl_supported = s_info->has_fl;
    ctx->_hw_info.irc_supported = s_info->has_irc;
    ctx->_hw_info.lens_supported = s_info->has_lens_vcm;
    ctx->_hw_info.fl_strth_adj = s_info->fl_strth_adj_sup;
    ctx->_hw_info.fl_ir_strth_adj = s_info->fl_ir_strth_adj_sup;
    ctx->_hw_info.is_multi_isp_mode = s_info->is_multi_isp_mode;
    ctx->_hw_info.multi_isp_extended_pixel = s_info->multi_isp_extended_pixel;
    ctx->_hw_info.module_rotation = RK_PS_SrcOverlapPosition_0;
#endif
    ctx->_analyzer->setHwInfos(ctx->_hw_info);

    ctx->_analyzer->setCamPhyId(s_info->sensor_info.phyId);

    if (is_ent_name && config_file_dir) {
        ctx->_analyzer->setResrcPath(config_file_dir);
    } else
        ctx->_analyzer->setResrcPath(RKAIQ_DEFAULT_IQ_PATH);
    ctx->_rkAiqManager->setAnalyzer(ctx->_analyzer);
    ctx->_lumaAnalyzer = new RkLumaCore();
    ctx->_rkAiqManager->setLumaAnalyzer(ctx->_lumaAnalyzer);
    //ctx->_calibDb = RkAiqCalibDb::createCalibDb(config_file);
    ctx->_socket  = new SocketServer();
    //if (!ctx->_calibDb)
    //    goto error;
    //ctx->_rkAiqManager->setAiqCalibDb(ctx->_calibDb);

    if (strstr(config_file, ".xml")) {
        LOGE("Should use json instead of xml");
        strcpy(config_file + strlen(config_file) - strlen(".xml"), ".json");
    }

    CamCalibDbV2Context_t calibdbv2_ctx;
    xcam_mem_clear (calibdbv2_ctx);

    ctx->_calibDbProj = RkAiqCalibDbV2::createCalibDbProj(config_file);
    if (!ctx->_calibDbProj)
        goto error;

    if (!main_scene.empty() && !sub_scene.empty())
        calibdbv2_ctx = RkAiqSceneManager::refToScene(ctx->_calibDbProj,
                        main_scene.c_str(), sub_scene.c_str());

    if (!calibdbv2_ctx.calib_scene) {
        LOGE("Failed to find params of %s:%s scene in json, using default scene",
             main_scene.c_str(), sub_scene.c_str());
        calibdbv2_ctx = RkAiqCalibDbV2::toDefaultCalibDb(ctx->_calibDbProj);
    }
    ctx->_rkAiqManager->setAiqCalibDb(&calibdbv2_ctx);

    ret = ctx->_rkAiqManager->init();
    ctx->_socket->Process(ctx);
    if (ret)
        goto error;

    ctx->ctx_type = CTX_TYPE_USER_MAIN;
    ctx->next_ctx = new rk_aiq_sys_ctx_t();
    RKAIQSYS_CHECK_RET(!ctx, NULL, "malloc toolserver ctx error !");
    *(ctx->next_ctx) = *ctx;
    ctx->next_ctx->ctx_type = CTX_TYPE_TOOL_SERVER;
    ctx->next_ctx->next_ctx = NULL;
    ctx->cam_type = RK_AIQ_CAM_TYPE_SINGLE;

    EXIT_XCORE_FUNCTION();

    return ctx;

error:
    LOGE("_rkAiqManager init error!");
    rk_aiq_uapi_sysctl_deinit_locked(ctx);
    return NULL;
}

void
rk_aiq_uapi_sysctl_deinit_locked(rk_aiq_sys_ctx_t* ctx)
{
    std::map<std::string, rk_aiq_sys_preinit_cfg_t>::iterator it =
        g_rk_aiq_sys_preinit_cfg_map.find(std::string(ctx->_sensor_entity_name));
    if (it != g_rk_aiq_sys_preinit_cfg_map.end()) {
        g_rk_aiq_sys_preinit_cfg_map.erase(it);
        LOGI("unset user specific iq file.");
    }

    if (ctx->_rkAiqManager.ptr())
        ctx->_rkAiqManager->deInit();

    ctx->_socket->Deinit();
    delete(ctx->_socket);
    ctx->_analyzer.release();
    ctx->_lumaAnalyzer.release();
    ctx->_rkAiqManager.release();
    ctx->_camHw.release();
    if (ctx->_calibDbProj) {
        // TODO:public common resource release
    }

    if (ctx->next_ctx) {
        delete ctx->next_ctx;
    }

    if (ctx->_sensor_entity_name)
        xcam_free((void*)(ctx->_sensor_entity_name));
}

void
rk_aiq_uapi_sysctl_deinit(rk_aiq_sys_ctx_t* ctx)
{
    ENTER_XCORE_FUNCTION();
    {
        RKAIQ_API_SMART_LOCK(ctx);
        rk_aiq_uapi_sysctl_deinit_locked(ctx);
    }
    delete ctx;
    EXIT_XCORE_FUNCTION();
}

XCamReturn
rk_aiq_uapi_sysctl_prepare(const rk_aiq_sys_ctx_t* ctx,
                           uint32_t  width, uint32_t  height,
                           rk_aiq_working_mode_t mode)
{
    ENTER_XCORE_FUNCTION();
    XCAM_ASSERT(ctx != nullptr);

    if (ctx->_use_fakecam && ctx->_raw_prop.format &&
            ctx->_raw_prop.frame_width &&
            ctx->_raw_prop.frame_height &&
            ctx->_raw_prop.rawbuf_type) {
        rk_aiq_uapi_sysctl_prepareRkRaw(ctx, ctx->_raw_prop);
    }

    RKAIQ_API_SMART_LOCK(ctx);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = ctx->_rkAiqManager->prepare(width, height, mode);
    RKAIQSYS_CHECK_RET(ret, ret, "prepare failed !");

    EXIT_XCORE_FUNCTION();

    return ret;
}

XCamReturn
rk_aiq_uapi_sysctl_start(const rk_aiq_sys_ctx_t* ctx)
{
    ENTER_XCORE_FUNCTION();
    RKAIQ_API_SMART_LOCK(ctx);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = ctx->_rkAiqManager->start();

    EXIT_XCORE_FUNCTION();

    return ret;
}

XCamReturn
rk_aiq_uapi_sysctl_stop(const rk_aiq_sys_ctx_t* ctx, bool keep_ext_hw_st)
{
    ENTER_XCORE_FUNCTION();
    RKAIQ_API_SMART_LOCK(ctx);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = ctx->_rkAiqManager->stop(keep_ext_hw_st);

    EXIT_XCORE_FUNCTION();

    return ret;
}

XCamReturn
rk_aiq_uapi_sysctl_getStaticMetas(const char* sns_ent_name, rk_aiq_static_info_t* static_info)
{
    if (!sns_ent_name || !static_info)
        return XCAM_RETURN_ERROR_FAILED;
#ifdef RK_SIMULATOR_HW
    /* nothing to do now*/
    static_info = NULL;
#else
    memcpy(static_info, CamHwIsp20::getStaticCamHwInfo(sns_ent_name), sizeof(rk_aiq_static_info_t));
#endif
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi_sysctl_enumStaticMetas(int index, rk_aiq_static_info_t* static_info)
{
    if (!static_info)
        return XCAM_RETURN_ERROR_FAILED;
#ifdef RK_SIMULATOR_HW
    /* nothing to do now*/
    static_info = NULL;
#else
    rk_aiq_static_info_t* tmp =  CamHwIsp20::getStaticCamHwInfo(NULL, index);
    if (tmp)
        memcpy(static_info, tmp, sizeof(rk_aiq_static_info_t));
    else
        return XCAM_RETURN_ERROR_OUTOFRANGE;
#endif
    return XCAM_RETURN_NO_ERROR;
}

const char*
rk_aiq_uapi_sysctl_getBindedSnsEntNmByVd(const char* vd)
{
#ifndef RK_SIMULATOR_HW
    return CamHwIsp20::getBindedSnsEntNmByVd(vd);
#endif
    return NULL;
}

XCamReturn
rk_aiq_uapi_sysctl_getMetaData(const rk_aiq_sys_ctx_t* ctx, uint32_t frame_id, rk_aiq_metas_t* metas)
{
    // TODO
    return XCAM_RETURN_ERROR_FAILED;
}

XCamReturn
rk_aiq_uapi_sysctl_regLib(const rk_aiq_sys_ctx_t* ctx,
                          RkAiqAlgoDesComm* algo_lib_des)
{
    RKAIQ_API_SMART_LOCK(ctx);
    if (ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* group_ctx =
            (const rk_aiq_camgroup_ctx_t*)ctx;

        return group_ctx->cam_group_manager->addAlgo(*algo_lib_des);
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else
        return ctx->_analyzer->addAlgo(*algo_lib_des);
}

XCamReturn
rk_aiq_uapi_sysctl_unRegLib(const rk_aiq_sys_ctx_t* ctx,
                            const int algo_type,
                            const int lib_id)
{
    RKAIQ_API_SMART_LOCK(ctx);
    if (ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* group_ctx =
            (const rk_aiq_camgroup_ctx_t*)ctx;

        return group_ctx->cam_group_manager->rmAlgo(algo_type, lib_id);
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else
        return ctx->_analyzer->rmAlgo(algo_type, lib_id);
}

XCamReturn
rk_aiq_uapi_sysctl_enableAxlib(const rk_aiq_sys_ctx_t* ctx,
                               const int algo_type,
                               const int lib_id,
                               bool enable)
{
    RKAIQ_API_SMART_LOCK(ctx);
    if (ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* group_ctx =
            (const rk_aiq_camgroup_ctx_t*)ctx;

        return group_ctx->cam_group_manager->enableAlgo(algo_type, lib_id, enable);
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        return ctx->_analyzer->enableAlgo(algo_type, lib_id, enable);
    }
}

bool
rk_aiq_uapi_sysctl_getAxlibStatus(const rk_aiq_sys_ctx_t* ctx,
                                  const int algo_type,
                                  const int lib_id)
{
    RKAIQ_API_SMART_LOCK(ctx);
    if (ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* group_ctx =
            (const rk_aiq_camgroup_ctx_t*)ctx;

        return group_ctx->cam_group_manager->getAxlibStatus(algo_type, lib_id);
#else
        return false;
#endif
    } else
        return ctx->_analyzer->getAxlibStatus(algo_type, lib_id);
}

RkAiqAlgoContext*
rk_aiq_uapi_sysctl_getEnabledAxlibCtx(const rk_aiq_sys_ctx_t* ctx, const int algo_type)
{
    RKAIQ_API_SMART_LOCK(ctx);
    if (ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* group_ctx =
            (const rk_aiq_camgroup_ctx_t*)ctx;

        return group_ctx->cam_group_manager->getEnabledAxlibCtx(algo_type);
#else
        return nullptr;
#endif
    } else
        return ctx->_analyzer->getEnabledAxlibCtx(algo_type);
}

RkAiqAlgoContext*
rk_aiq_uapi_sysctl_getAxlibCtx(const rk_aiq_sys_ctx_t* ctx, const int algo_type, const int lib_id)
{
    RKAIQ_API_SMART_LOCK(ctx);
    if (ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* group_ctx =
            (const rk_aiq_camgroup_ctx_t*)ctx;

        return group_ctx->cam_group_manager->getAxlibCtx(algo_type, lib_id);
#else
        return nullptr;
#endif
    } else
        return ctx->_analyzer->getAxlibCtx(algo_type, lib_id);
}

XCamReturn
rk_aiq_uapi_sysctl_get3AStats(const rk_aiq_sys_ctx_t* ctx,
                              rk_aiq_isp_stats_t *stats)
{
    RKAIQ_API_SMART_LOCK(ctx);
    return ctx->_analyzer->get3AStatsFromCachedList(*stats);
}

XCamReturn
rk_aiq_uapi_sysctl_get3AStatsBlk(const rk_aiq_sys_ctx_t* ctx,
                                 rk_aiq_isp_stats_t **stats, int timeout_ms)
{
    // blocked API, add lock ?
    //RKAIQ_API_SMART_LOCK(ctx);
    return ctx->_analyzer->get3AStatsFromCachedList(stats, timeout_ms);
}

void
rk_aiq_uapi_sysctl_release3AStatsRef(const rk_aiq_sys_ctx_t* ctx,
                                     rk_aiq_isp_stats_t *stats)
{
    RKAIQ_API_SMART_LOCK(ctx);
    ctx->_analyzer->release3AStatsRef(stats);
}

RKAIQ_END_DECLARE

template<typename T> static T*
algoHandle(const rk_aiq_sys_ctx_t* ctx, const int algo_type)
{
    T* algo_handle = NULL;

    RkCam::RkAiqHandle* handle =
        const_cast<RkCam::RkAiqHandle*>(ctx->_analyzer->getAiqAlgoHandle(algo_type));

    XCAM_ASSERT(handle);

    int algo_id = handle->getAlgoId();

    if (algo_id == 0)
        algo_handle = dynamic_cast<T*>(handle);

    return algo_handle;
}

#ifdef RKAIQ_ENABLE_CAMGROUP
template<typename T> static T*
camgroupAlgoHandle(const rk_aiq_sys_ctx_t* ctx, const int algo_type)
{
    T* algo_handle = NULL;

    const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)ctx;
    RkCam::RkAiqCamgroupHandle* handle =
        const_cast<RkCam::RkAiqCamgroupHandle*>(camgroup_ctx->cam_group_manager->getAiqCamgroupHandle(algo_type, 0));

    if (!handle)
        return NULL;

    int algo_id = handle->getAlgoId();

    if (algo_id == 0)
        algo_handle = dynamic_cast<T*>(handle);

    return algo_handle;
}
#endif

#include "RkAiqVersion.h"
#include "RkAiqCalibVersion.h"
#include "rk_aiq_user_api_awb.cpp"
#include "rk_aiq_user_api_adebayer.cpp"
#include "rk_aiq_user_api2_adebayer.cpp"
#include "rk_aiq_user_api_ahdr.cpp"
#include "uAPI2/rk_aiq_user_api2_amerge.cpp"
#include "uAPI2/rk_aiq_user_api2_atmo.cpp"
#include "uAPI2/rk_aiq_user_api2_adrc.cpp"
#include "rk_aiq_user_api_alsc.cpp"
#include "rk_aiq_user_api_accm.cpp"
#include "rk_aiq_user_api_a3dlut.cpp"
#include "rk_aiq_user_api_adehaze.cpp"
#include "uAPI2/rk_aiq_user_api2_adehaze.cpp"
#include "uAPI2/rk_aiq_user_api2_adpcc.cpp"
#include "rk_aiq_user_api_agamma.cpp"
#include "uAPI2/rk_aiq_user_api2_agamma.cpp"
#include "uAPI2/rk_aiq_user_api2_adegamma.cpp"
#include "uAPI2/rk_aiq_user_api2_ablc.cpp"
#include "rk_aiq_user_api_adpcc.cpp"
#include "rk_aiq_user_api_ae.cpp"
#include "uAPI2/rk_aiq_user_api2_ae.cpp"
#include "rk_aiq_user_api_anr.cpp"
#include "rk_aiq_user_api_asharp.cpp"
#include "rk_aiq_user_api_imgproc.cpp"
#include "uAPI2/rk_aiq_user_api2_imgproc.cpp"
#include "rk_aiq_user_api_afec.cpp"
#include "rk_aiq_user_api_af.cpp"
#include "uAPI2/rk_aiq_user_api2_af.cpp"
#include "rk_aiq_user_api_asd.cpp"
#include "rk_aiq_user_api_aldch.cpp"
#include "rk_aiq_user_api2_aldch.cpp"
#include "rk_aiq_user_api_acp.cpp"
#include "rk_aiq_user_api2_acp.cpp"
#include "rk_aiq_user_api_aie.cpp"
#include "rk_aiq_user_api2_aie.cpp"
#include "rk_aiq_user_api_aeis.cpp"
#include "rk_aiq_user_api_abayernr_v2.cpp"
#include "rk_aiq_user_api_aynr_v2.cpp"
#include "rk_aiq_user_api_acnr_v1.cpp"
#include "rk_aiq_user_api_asharp_v3.cpp"
#include "uAPI2/rk_aiq_user_api2_abayernr_v2.cpp"
#include "uAPI2/rk_aiq_user_api2_aynr_v2.cpp"
#include "uAPI2/rk_aiq_user_api2_acnr_v1.cpp"
#include "uAPI2/rk_aiq_user_api2_asharp_v3.cpp"
#include "uAPI2/rk_aiq_user_api2_anr.cpp"
#include "uAPI2/rk_aiq_user_api2_awb.cpp"
#include "uAPI2/rk_aiq_user_api2_alsc.cpp"
#include "uAPI2/rk_aiq_user_api2_accm.cpp"
#include "uAPI2/rk_aiq_user_api2_a3dlut.cpp"
#include "rk_aiq_user_api2_afec.cpp"
#include "uAPI/rk_aiq_user_api_agic.cpp"
#include "uAPI2/rk_aiq_user_api2_camgroup.cpp"
#include "uAPI2/rk_aiq_user_api2_agic.cpp"
#include "rk_aiq_user_api2_custom_ae.cpp"
#include "rk_aiq_user_api2_custom_awb.cpp"
#include "uAPI2/rk_aiq_user_api2_aynr_v3.cpp"
#include "rk_aiq_user_api_aynr_v3.cpp"
#include "uAPI2/rk_aiq_user_api2_acnr_v2.cpp"
#include "rk_aiq_user_api_acnr_v2.cpp"
#include "uAPI2/rk_aiq_user_api2_asharp_v4.cpp"
#include "rk_aiq_user_api_asharp_v4.cpp"
#include "uAPI2/rk_aiq_user_api2_abayer2dnr_v2.cpp"
#include "rk_aiq_user_api_abayer2dnr_v2.cpp"
#include "uAPI2/rk_aiq_user_api2_abayertnr_v2.cpp"
#include "rk_aiq_user_api_abayertnr_v2.cpp"
#include "uAPI2/rk_aiq_user_api2_acsm.cpp"
#include "uAPI2/rk_aiq_user_api2_again_v2.cpp"
#include "rk_aiq_user_api_again_v2.cpp"


#define RK_AIQ_ALGO_TYPE_MODULES (RK_AIQ_ALGO_TYPE_MAX + 1)

XCamReturn
rk_aiq_uapi_sysctl_setModuleCtl(const rk_aiq_sys_ctx_t* ctx, rk_aiq_module_id_t mId, bool mod_en)
{
    ENTER_XCORE_FUNCTION();
    CHECK_USER_API_ENABLE2(ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_MODULES);
    RKAIQ_API_SMART_LOCK(ctx);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (mId > RK_MODULE_INVAL && mId < RK_MODULE_MAX) {
        if (ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
            const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)ctx;
            for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
                if (!camCtx)
                    continue;

                ret = camCtx->_rkAiqManager->setModuleCtl(mId, mod_en);
            }
#else
            return XCAM_RETURN_ERROR_FAILED;
#endif
        } else {
            ret = ctx->_rkAiqManager->setModuleCtl(mId, mod_en);
        }
    }

    EXIT_XCORE_FUNCTION();

    return ret;
}

int32_t
rk_aiq_uapi_sysctl_getModuleCtl(const rk_aiq_sys_ctx_t* ctx, rk_aiq_module_id_t mId, bool *mod_en)
{
    ENTER_XCORE_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RKAIQ_API_SMART_LOCK(ctx);
    if (ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)ctx;
        for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
            if (!camCtx)
                continue;

            bool en;
            ret = ctx->_rkAiqManager->getModuleCtl(mId, en);
            *mod_en = en;

            return ret;
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        NULL_RETURN_RET(ctx, -1);
        NULL_RETURN_RET(ctx->_rkAiqManager.ptr(), -1);

        bool en;
        ret = ctx->_rkAiqManager->getModuleCtl(mId, en);
        *mod_en = en;
    }
    EXIT_XCORE_FUNCTION();

    return ret;
}

XCamReturn
rk_aiq_uapi_sysctl_setCpsLtCfg(const rk_aiq_sys_ctx_t* ctx,
                               rk_aiq_cpsl_cfg_t* cfg)
{
    RKAIQ_API_SMART_LOCK(ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)ctx;
        for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
            if (!camCtx)
                continue;

            ret = camCtx->_analyzer->setCpsLtCfg(*cfg);
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        ret = ctx->_analyzer->setCpsLtCfg(*cfg);
    }

    return ret;
}

XCamReturn
rk_aiq_uapi_sysctl_getCpsLtInfo(const rk_aiq_sys_ctx_t* ctx,
                                rk_aiq_cpsl_info_t* info)
{
    RKAIQ_API_SMART_LOCK(ctx);
    if (ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)ctx;
        for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
            if (!camCtx)
                continue;

            return camCtx->_analyzer->getCpsLtInfo(*info);
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        return ctx->_analyzer->getCpsLtInfo(*info);
    }

    return XCAM_RETURN_ERROR_FAILED;
}

XCamReturn
rk_aiq_uapi_sysctl_queryCpsLtCap(const rk_aiq_sys_ctx_t* ctx,
                                 rk_aiq_cpsl_cap_t* cap)
{
    RKAIQ_API_SMART_LOCK(ctx);
    if (ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)ctx;
        for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
            if (!camCtx)
                continue;

            return camCtx->_analyzer->queryCpsLtCap(*cap);
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        return ctx->_analyzer->queryCpsLtCap(*cap);
    }

    return XCAM_RETURN_ERROR_FAILED;
}

extern RkAiqAlgoDescription g_RkIspAlgoDescAe;
extern RkAiqAlgoDescription g_RkIspAlgoDescAwb;
extern RkAiqAlgoDescription g_RkIspAlgoDescAf;
extern RkAiqAlgoDescription g_RkIspAlgoDescAmerge;
extern RkAiqAlgoDescription g_RkIspAlgoDescAtmo;

static void _print_versions()
{
    LOGI("\n"
         "************************** VERSION INFOS **************************\n"
         "version release date: %s\n"
         "         AIQ:       %s\n"
         "   IQ PARSER:       %s\n"
         "************************ VERSION INFOS END ************************\n"
         , RK_AIQ_RELEASE_DATE
         , RK_AIQ_VERSION
         , RK_AIQ_CALIB_VERSION
        );
}

void rk_aiq_uapi_get_version_info(rk_aiq_ver_info_t* vers)
{
    uint32_t iq_parser_magic_code;

    xcam_mem_clear (*vers);
    const char* ver_str = RK_AIQ_CALIB_VERSION;
    const char* start = ver_str +  strlen(RK_AIQ_CALIB_VERSION_HEAD);
    const char* stop = strstr(ver_str, RK_AIQ_CALIB_VERSION_MAGIC_JOINT);

    // TODO: use strncpy instead of memcpy, but has compile warning now
    memcpy(vers->iq_parser_ver, start, stop - start);

    start = strstr(ver_str, RK_AIQ_CALIB_VERSION_MAGIC_CODE_HEAD) +
            strlen(RK_AIQ_CALIB_VERSION_MAGIC_CODE_HEAD);

    vers->iq_parser_magic_code = atoi(start);

    ver_str = RK_AIQ_VERSION;
    start = ver_str +  strlen(RK_AIQ_VERSION_HEAD);
    strcpy(vers->aiq_ver, start);

    strcpy(vers->awb_algo_ver, g_RkIspAlgoDescAwb.common.version);
    strcpy(vers->ae_algo_ver, g_RkIspAlgoDescAe.common.version);
    strcpy(vers->af_algo_ver, g_RkIspAlgoDescAf.common.version);
    strcpy(vers->ahdr_algo_ver, g_RkIspAlgoDescAmerge.common.version);
    strcpy(vers->ahdr_algo_ver, g_RkIspAlgoDescAtmo.common.version);

    LOGI("aiq ver %s, parser ver %s, magic code %d, awb ver %s\n"
         "ae ver %s, af ver %s, ahdr ver %s", vers->aiq_ver,
         vers->iq_parser_ver, vers->iq_parser_magic_code,
         vers->awb_algo_ver, vers->ae_algo_ver,
         vers->af_algo_ver, vers->ahdr_algo_ver);
}

static void rk_aiq_init_lib(void) __attribute__((constructor));
static void rk_aiq_init_lib(void)
{
    xcam_get_log_level();
    ENTER_XCORE_FUNCTION();
#ifdef RK_SIMULATOR_HW
    /* nothing to do now */
#else
    CamHwIsp20::initCamHwInfos();
    rk_aiq_static_info_t* s_info = CamHwIsp20::getStaticCamHwInfo(NULL, 0);

    if (s_info != nullptr) {
        if (s_info->isp_hw_ver == 4)
            g_rkaiq_isp_hw_ver = 20;
        else if (s_info->isp_hw_ver == 5)
            g_rkaiq_isp_hw_ver = 21;
        else if (s_info->isp_hw_ver == 6)
            g_rkaiq_isp_hw_ver = 30;
        else
            LOGE("do not support isp hw ver %d now !", s_info->isp_hw_ver);
    }
#endif
#if defined(ISP_HW_V20)
    assert(g_rkaiq_isp_hw_ver == 20);
#elif defined(ISP_HW_V21)
    assert(g_rkaiq_isp_hw_ver == 21);
#elif defined(ISP_HW_V30)
    assert(g_rkaiq_isp_hw_ver == 30);
#else
#error "WRONG ISP_HW_VERSION, ONLY SUPPORT V20 AND V21 NOW !"
#endif
    _print_versions();
    EXIT_XCORE_FUNCTION();

}
static void rk_aiq_deinit_lib(void) __attribute__((destructor));
static void rk_aiq_deinit_lib(void)
{
    ENTER_XCORE_FUNCTION();
#ifdef RK_SIMULATOR_HW
    /* nothing to do now */
#else
    RkAiqCalibDbV2::releaseCalibDbProj();
#ifdef RKAIQ_ENABLE_PARSER_V1
    RkAiqCalibDb::releaseCalibDb();
#endif
    CamHwIsp20::clearStaticCamHwInfo();
#endif
    EXIT_XCORE_FUNCTION();
}

XCamReturn
rk_aiq_uapi_sysctl_enqueueRkRawBuf(const rk_aiq_sys_ctx_t* ctx, void *rawdata, bool sync)
{
    ENTER_XCORE_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)ctx;
        for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
            if (!camCtx)
                continue;

            ret = camCtx->_rkAiqManager->enqueueRawBuffer(rawdata, sync);
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        ret = ctx->_rkAiqManager->enqueueRawBuffer(rawdata, sync);
    }
    EXIT_XCORE_FUNCTION();

    return ret;
}

XCamReturn
rk_aiq_uapi_sysctl_enqueueRkRawFile(const rk_aiq_sys_ctx_t* ctx, const char *path)
{
    ENTER_XCORE_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)ctx;
        for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
            if (!camCtx)
                continue;

            ret = camCtx->_rkAiqManager->enqueueRawFile(path);
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        ret = ctx->_rkAiqManager->enqueueRawFile(path);
    }
    EXIT_XCORE_FUNCTION();

    return ret;
}

XCamReturn
rk_aiq_uapi_sysctl_registRkRawCb(const rk_aiq_sys_ctx_t* ctx, void (*callback)(void*))
{
    ENTER_XCORE_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (callback == NULL)
        return XCAM_RETURN_ERROR_PARAM;

    if (ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)ctx;
        for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
            if (!camCtx)
                continue;

            ret = camCtx->_rkAiqManager->registRawdataCb(callback);
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        ret = ctx->_rkAiqManager->registRawdataCb(callback);
    }
    EXIT_XCORE_FUNCTION();
    return ret;
}

XCamReturn
rk_aiq_uapi_sysctl_prepareRkRaw(const rk_aiq_sys_ctx_t* ctx, rk_aiq_raw_prop_t prop)
{
    ENTER_XCORE_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)ctx;
        for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
            if (!camCtx)
                continue;

            ret = camCtx->_rkAiqManager->rawdataPrepare(prop);
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        ret = ctx->_rkAiqManager->rawdataPrepare(prop);
    }
    EXIT_XCORE_FUNCTION();

    return ret;
}

XCamReturn
rk_aiq_uapi_sysctl_setSharpFbcRotation(const rk_aiq_sys_ctx_t* ctx, rk_aiq_rotation_t rot)
{
    ENTER_XCORE_FUNCTION();
    RKAIQ_API_SMART_LOCK(ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)ctx;
        for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
            if (!camCtx)
                continue;

            ret = camCtx->_rkAiqManager->setSharpFbcRotation(rot);
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        ret = ctx->_rkAiqManager->setSharpFbcRotation(rot);
    }
    EXIT_XCORE_FUNCTION();
    return ret;
}

/*!
 * \brief switch working mode dynamically
 * this aims to switch the isp pipeline working mode fast, and can be called on
 * streaming status. On non streaming status, should call rk_aiq_uapi_sysctl_prepare
 * instead of this to set working mode.
 */
XCamReturn
rk_aiq_uapi_sysctl_swWorkingModeDyn(const rk_aiq_sys_ctx_t* ctx, rk_aiq_working_mode_t mode)
{
    ENTER_XCORE_FUNCTION();
    if (ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP)
        return XCAM_RETURN_ERROR_FAILED;
    RKAIQ_API_SMART_LOCK(ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    /* ret = ctx->_rkAiqManager->swWorkingModeDyn(mode); */
    ret = ctx->_rkAiqManager->swWorkingModeDyn_msg(mode);
    EXIT_XCORE_FUNCTION();
    return ret;
}

void
rk_aiq_uapi_sysctl_setMulCamConc(const rk_aiq_sys_ctx_t* ctx, bool cc)
{
    ENTER_XCORE_FUNCTION();
    RKAIQ_API_SMART_LOCK(ctx);
    ctx->_rkAiqManager->setMulCamConc(cc);
    EXIT_XCORE_FUNCTION();
}

XCamReturn
rk_aiq_uapi_sysctl_setCrop(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_rect_t rect)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
        for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
            if (!camCtx)
                continue;

            ret = camCtx->_camHw->setSensorCrop(rect);
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        ret = sys_ctx->_camHw->setSensorCrop(rect);
    }
    return ret;
}

XCamReturn
rk_aiq_uapi_sysctl_getCrop(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_rect_t* rect)
{
    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
        LOGE("%s: not support for camgroup\n", __func__);
        return XCAM_RETURN_ERROR_FAILED;
    }

    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ret = sys_ctx->_camHw->getSensorCrop(*rect);

    return ret;
}

XCamReturn
rk_aiq_uapi_sysctl_updateIq(rk_aiq_sys_ctx_t* sys_ctx, char* iqfile)
{
    if (!sys_ctx) {
        LOGE("%s: sys_ctx is invalied\n", __func__);
        return XCAM_RETURN_ERROR_FAILED;
    }

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
        LOGE("%s: not support for camgroup\n", __func__);
        return XCAM_RETURN_ERROR_FAILED;
    }

    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    LOGI("applying new iq file:%s\n", iqfile);

    auto calibDbProj = RkAiqCalibDbV2::createCalibDbProj(iqfile);
    if (!calibDbProj) {
        LOGE("failed to create CalibDbProj from iqfile\n");
        return XCAM_RETURN_ERROR_PARAM;
    }

    CamCalibDbV2Context_t calibdbv2_ctx =
        RkAiqCalibDbV2::toDefaultCalibDb(calibDbProj);
    ret = sys_ctx->_rkAiqManager->updateCalibDb(&calibdbv2_ctx);

    if (ret) {
        LOGE("failed to update iqfile\n");
        return ret;
    }

    sys_ctx->_calibDbProj = calibDbProj;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi_sysctl_tuning(const rk_aiq_sys_ctx_t* sys_ctx, char* param)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (!sys_ctx) {
        LOGE("%s: sys_ctx is invalied\n", __func__);
        return XCAM_RETURN_ERROR_FAILED;
    }

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
        LOGE("%s: not support for camgroup\n", __func__);
        return XCAM_RETURN_ERROR_FAILED;
    }

    // Find json patch
    std::string patch_str(param);
    size_t json_start = patch_str.find_first_of("[");
    size_t json_end = patch_str.find_last_of("]");

    LOGI("patch is:%s\n", patch_str.c_str());

    if (json_start >= patch_str.size() || json_end > patch_str.size() ||
            json_start >= json_end) {
        LOGE("%s: patch is invalied\n", __func__);
        return XCAM_RETURN_ERROR_FAILED;
    }
    std::string json_str = patch_str.substr(json_start, json_end + 1);

    if (json_str.empty()) {
        LOGE("%s: patch is empty\n", __func__);
        return XCAM_RETURN_ERROR_FAILED;
    }

    auto last_calib = sys_ctx->_rkAiqManager->getCurrentCalibDBV2();

    if (!last_calib) {
        *last_calib = RkAiqCalibDbV2::toDefaultCalibDb(sys_ctx->_calibDbProj);
        if (!last_calib) {
            LOGE("%s: default calib is invalied\n", __func__);
            return XCAM_RETURN_ERROR_FAILED;
        }
    }

    auto tuning_calib = RkCam::RkAiqCalibDbV2::analyzTuningCalib(
                            last_calib, json_str.c_str());

    ret = sys_ctx->_rkAiqManager->calibTuning(tuning_calib.calib,
            tuning_calib.ModuleNames);

    return ret;
}

char* rk_aiq_uapi_sysctl_readiq(const rk_aiq_sys_ctx_t* sys_ctx, char* param)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    cJSON* cmd_json = NULL;
    char* ret_str = NULL;

    if (!sys_ctx) {
        LOGE("%s: sys_ctx is invalied\n", __func__);
        return NULL;
    }

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
        LOGE("%s: not support for camgroup\n", __func__);
        return NULL;
    }

    // Find json patch
    std::string patch_str(param);
    size_t json_start = patch_str.find_first_of("[");
    size_t json_end = patch_str.find_first_of("]");

    LOGI("request is:%s\n", patch_str.c_str());

    if (json_start >= patch_str.size() || json_end > patch_str.size() ||
            json_start >= json_end) {
        LOGE("%s: request is invalied\n", __func__);
        return NULL;
    }
    std::string json_str = patch_str.substr(json_start, json_end + 1);

    if (json_str.empty()) {
        LOGE("%s: request is empty\n", __func__);
        return NULL;
    }

    auto last_calib = sys_ctx->_rkAiqManager->getCurrentCalibDBV2();

    if (!last_calib) {
        *last_calib = RkAiqCalibDbV2::toDefaultCalibDb(sys_ctx->_calibDbProj);
        if (!last_calib) {
            LOGE("%s: default calib is invalied\n", __func__);
            return NULL;
        }
    }

    ret_str = RkCam::RkAiqCalibDbV2::readIQNodeStrFromJstr(last_calib,
              json_str.c_str());

    return ret_str;
}

XCamReturn rk_aiq_uapi_sysctl_regMemsSensorIntf(const rk_aiq_sys_ctx_t* sys_ctx,
        const rk_aiq_mems_sensor_intf_t* intf) {
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    assert(sys_ctx != nullptr);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
        LOGE("%s: not support for camgroup\n", __func__);
        return XCAM_RETURN_ERROR_FAILED;
    }

    ret = sys_ctx->_analyzer->setMemsSensorIntf(intf);
    if (ret) {
        LOGE("failed to update iqfile\n");
        ret = XCAM_RETURN_ERROR_FAILED;
    }

    return ret;
}

int rk_aiq_uapi_sysctl_switch_scene(const rk_aiq_sys_ctx_t* sys_ctx,
                                    const char* main_scene,
                                    const char* sub_scene)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (!sys_ctx) {
        LOGE("%s: sys_ctx is invalied\n", __func__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
        LOGE("%s: not support for camgroup\n", __func__);
        return XCAM_RETURN_ERROR_FAILED;
    }

    if (!main_scene || !sub_scene) {
        LOGE("%s: request is invalied\n", __func__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    auto new_calib = RkAiqSceneManager::refToScene(sys_ctx->_calibDbProj,
                     main_scene, sub_scene);

    ret = sys_ctx->_rkAiqManager->updateCalibDb(&new_calib);
    if (ret) {
        LOGE("failed to switch scene\n");
        return ret;
    }

    return XCAM_RETURN_NO_ERROR;
}
