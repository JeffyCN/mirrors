/*
 * rk_aiq_user_api2_camgroup.cpp
 *
 *  Copyright (c) 2021 Rockchip Corporation
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

#include "rk_aiq_user_api2_sysctl.h"
#include "rk_aiq_user_api2_camgroup.h"
#include "RkAiqCamGroupManager.h"
#include "common/panorama_stitchingApp.h"

using namespace RkCam;
using namespace XCam;

static XCamReturn
_cam_group_bind(rk_aiq_camgroup_ctx_t* camgroup_ctx, rk_aiq_sys_ctx_t* aiq_ctx)
{
#ifdef RKAIQ_ENABLE_CAMGROUP
    ENTER_XCORE_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    // bind aiq to group
    ret = camgroup_ctx->cam_group_manager->bind(aiq_ctx->_rkAiqManager.ptr());
    if (ret) {
        LOGE("bind sensor %s aiq ctx %p failed !", aiq_ctx->_sensor_entity_name, aiq_ctx);
        return ret;
    }

    camgroup_ctx->cam_group_manager->setContainerCtx(camgroup_ctx);

    // bind group to aiq
    aiq_ctx->_camGroupManager = camgroup_ctx->cam_group_manager.ptr();
    aiq_ctx->_analyzer->setCamGroupManager(aiq_ctx->_camGroupManager);

#if 0
    // set first one as main cam
    aiq_ctx->_rkAiqManager->setCamGroupManager(aiq_ctx->_camGroupManager,
                                               camgroup_ctx->cam_ctxs_num == 0 ? true : false);
#else
    if (aiq_ctx->_is_1608_sensor) {
        // 1608 sensor.
        aiq_ctx->_rkAiqManager->setCamGroupManager(aiq_ctx->_camGroupManager, false);
        camgroup_ctx->cam_1608_num++;
        LOGD("  >>>>====<<<< sensor name: %s, 1608-sensor(%d), sync mode(isMain): %d. \n", aiq_ctx->_sensor_entity_name,
                                                                                           aiq_ctx->_is_1608_sensor,
                                                                                           false);
    } else {
        // normal sensor.
        aiq_ctx->_rkAiqManager->setCamGroupManager(aiq_ctx->_camGroupManager,
                                                   camgroup_ctx->cam_ctxs_num == camgroup_ctx->cam_1608_num ? true : false);
        LOGD("  >>>>====<<<< sensor name: %s, 1608-sensor(%d), sync mode(isMain): %d. \n", aiq_ctx->_sensor_entity_name,
                                                                                           aiq_ctx->_is_1608_sensor,
                                                                                           camgroup_ctx->cam_ctxs_num == \
                                                                                           camgroup_ctx->cam_1608_num ? true : false);
    }
#endif

    camgroup_ctx->cam_ctxs_num++;
    camgroup_ctx->cam_ctxs_array[aiq_ctx->_camPhyId] = aiq_ctx;

    LOGD("%s: bind sensor %s aiq ctx success !", aiq_ctx->_sensor_entity_name, aiq_ctx);

    EXIT_XCORE_FUNCTION();
#endif

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
_cam_group_unbind(rk_aiq_camgroup_ctx_t* camgroup_ctx, rk_aiq_sys_ctx_t* aiq_ctx)
{
#ifdef RKAIQ_ENABLE_CAMGROUP
    ENTER_XCORE_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = camgroup_ctx->cam_group_manager->unbind(aiq_ctx->_camPhyId);
    if (ret) {
        LOGE("unbind sensor %s aiq ctx 0x%x failed !", aiq_ctx->_sensor_entity_name, aiq_ctx);
        return ret;
    }
    aiq_ctx->_camGroupManager = NULL;
    aiq_ctx->_analyzer->setCamGroupManager(NULL);
    aiq_ctx->_rkAiqManager->setCamGroupManager(NULL, false);
    camgroup_ctx->cam_ctxs_array[aiq_ctx->_camPhyId] = NULL;
    camgroup_ctx->cam_ctxs_num--;

    LOGD("%s: unbind sensor %s aiq ctx success !", aiq_ctx->_sensor_entity_name, aiq_ctx);

    EXIT_XCORE_FUNCTION();
#endif

    return XCAM_RETURN_NO_ERROR;
}

rk_aiq_camgroup_ctx_t*
rk_aiq_uapi2_camgroup_create(rk_aiq_camgroup_instance_cfg_t* cfg)
{
#ifdef RKAIQ_ENABLE_CAMGROUP
    ENTER_XCORE_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_camgroup_ctx_t* camgroup_ctx = NULL;
    rk_aiq_sys_ctx_t* aiq_ctx = NULL;
    std::string single_iq_file;
    std::string camgroup_iq_file;
    std::string overlap_map_file;

    camgroup_ctx = new rk_aiq_camgroup_ctx_t();

    if (!camgroup_ctx)
        goto error;

    camgroup_ctx->cam_group_manager = new RkAiqCamGroupManager();
    if (!camgroup_ctx->cam_group_manager.ptr())
        goto error;

    camgroup_ctx->_apiMutex = new Mutex();
    //RKAIQ_API_SMART_LOCK(camgroup_ctx);

    if (cfg->sns_num > RK_AIQ_CAM_GROUP_MAX_CAMS) {
        LOGE("nums %s is over the max cams %d !", cfg->sns_num, RK_AIQ_CAM_GROUP_MAX_CAMS);
        return NULL;
    }

    camgroup_ctx->cam_type = RK_AIQ_CAM_TYPE_GROUP;
    camgroup_ctx->cam_ctxs_num = 0;
    camgroup_ctx->cam_1608_num = 0;
    memset(camgroup_ctx->cam_ctxs_array, 0, sizeof(camgroup_ctx->cam_ctxs_array));
    camgroup_ctx->_srcOverlapMap_s = NULL;
    camgroup_ctx->_camgroup_calib = NULL;

    if (cfg->config_file_dir) {
        if (cfg->single_iq_file) {
            //single_iq_file += cfg->config_file_dir;
            //single_iq_file += "/";
            single_iq_file += cfg->single_iq_file;
        }
        if (cfg->group_iq_file) {
            camgroup_iq_file += cfg->config_file_dir;
            camgroup_iq_file += "/";
            camgroup_iq_file += cfg->group_iq_file;
        }
        if (cfg->overlap_map_file) {
            overlap_map_file += cfg->config_file_dir;
            overlap_map_file += "/";
            overlap_map_file += cfg->overlap_map_file;
        }
    }

    if (overlap_map_file.length()) {
        camgroup_ctx->_srcOverlapMap_s = new RK_PS_SrcOverlapMap();
        ret = rk_aiq_uapi2_camgroup_getOverlapMap_from_file(overlap_map_file.c_str(),
                                                           &camgroup_ctx->_srcOverlapMap_s);
        if (ret) {
            delete camgroup_ctx->_srcOverlapMap_s;
            camgroup_ctx->_srcOverlapMap_s = NULL;
        }
    }

    for (int i = 0; i < cfg->sns_num; i++) {
        // TODO: fix 1608 psy-sensor, "m0x_b_RK1608-dphy RK1608-dphy0"
        if (strstr(cfg->sns_ent_nm_array[i], "1608")) {
            // LOGE("  >>>>====<<<<  sensor_name: %s.", cfg->sns_ent_nm_array[i]);
            continue;
        }

        if (single_iq_file.length())
            rk_aiq_uapi_sysctl_preInit(cfg->sns_ent_nm_array[i],
                                       RK_AIQ_WORKING_MODE_NORMAL, /* nonsense */
                                       single_iq_file.c_str());

        if (cfg->pHwEvt_cb)
            rk_aiq_uapi2_sysctl_regHwEvtCb(cfg->sns_ent_nm_array[i],
                                           cfg->pHwEvt_cb,
                                           cfg->pHwEvtCbCtx);

        aiq_ctx = rk_aiq_uapi_sysctl_init(cfg->sns_ent_nm_array[i],
                                          cfg->config_file_dir, NULL, NULL);
        if (!aiq_ctx) {
           LOGE("init aiq ctx %d for %s failed !", i, cfg->sns_ent_nm_array[i]);
           goto error;
        }
        // notify single cam work as multiple mode
        rk_aiq_uapi_sysctl_setMulCamConc(aiq_ctx, true);

        if (camgroup_ctx->_srcOverlapMap_s) {
            aiq_ctx->_hw_info.module_rotation =
                camgroup_ctx->_srcOverlapMap_s->srcOverlapPositon[i];

            // update module rotation info
            aiq_ctx->_analyzer->setHwInfos(aiq_ctx->_hw_info);
        }

        ret = _cam_group_bind(camgroup_ctx, aiq_ctx);
        if (ret) {
            LOGE("%s: bind sensor %s aiq ctx 0x%x failed !",
                 __func__, aiq_ctx->_sensor_entity_name, aiq_ctx);
            goto error;
        }
    }
    if (camgroup_ctx->cam_1608_num == camgroup_ctx->cam_ctxs_num) {
        LOGE("%s: >>>>====<<<<  ERROR, Only slave(master-slave)-sensor work is not supported in group mode!", __func__);
        goto error;
    }

    if (camgroup_iq_file.length())
        camgroup_ctx->_camgroup_calib = RkAiqCalibDbV2::createCalibDbCamgroup(camgroup_iq_file.c_str());

    ret = camgroup_ctx->cam_group_manager->setCamgroupCalib(camgroup_ctx->_camgroup_calib);
    if (ret) {
        LOGE("%s: set camgroup calib failed !", __func__);
        goto error;
    }
    ret = camgroup_ctx->cam_group_manager->init();
    if (ret) {
        LOGE("%s: init failed !", __func__);
        goto error;
    }

    LOGD("%s: create camgroup 0x%x success !", __func__, camgroup_ctx);

    EXIT_XCORE_FUNCTION();

    return camgroup_ctx;

error:
    LOGE("%s failed", __func__);
    if (camgroup_ctx)
        rk_aiq_uapi2_camgroup_destroy(camgroup_ctx);
#endif
    return NULL;
}

struct RK_PS_SrcOverlapMap*
rk_aiq_uapi2_camgroup_getOverlapMap(rk_aiq_camgroup_ctx_t* camgroup_ctx)
{
#ifdef RKAIQ_ENABLE_CAMGROUP
    if (camgroup_ctx->_srcOverlapMap_s)
        return camgroup_ctx->_srcOverlapMap_s;
    else
        return NULL;
#else
    return NULL;
#endif
}

XCamReturn
rk_aiq_uapi2_camgroup_getOverlapMap_from_file(const char* map_file, struct RK_PS_SrcOverlapMap** overlapMap)
{
#ifdef RKAIQ_ENABLE_CAMGROUP
    FILE *fp2 = fopen(map_file, "rb");

    if (fp2) {
        fread(*overlapMap, sizeof(RK_PS_SrcOverlapMap), 1, fp2);
        fclose(fp2);
        return XCAM_RETURN_NO_ERROR;
    } else {
        LOGW("get overlap data from %s error!", map_file);
        return XCAM_RETURN_ERROR_FAILED;
    }
#else
    return XCAM_RETURN_ERROR_FAILED;
#endif
}

rk_aiq_sys_ctx_t*
rk_aiq_uapi2_camgroup_getAiqCtxBySnsNm(rk_aiq_camgroup_ctx_t* camgroup_ctx, const char* sns_entity_name)
{
#ifdef RKAIQ_ENABLE_CAMGROUP
    ENTER_XCORE_FUNCTION();

    RKAIQ_API_SMART_LOCK(camgroup_ctx);

    for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
        if (camgroup_ctx->cam_ctxs_array[i]) {
            if (strcmp(camgroup_ctx->cam_ctxs_array[i]->_sensor_entity_name, sns_entity_name) == 0) {
                LOGD("%s: get sensor %s aiq ctx 0x%x success !",
                     __func__, sns_entity_name, camgroup_ctx->cam_ctxs_array[i]);
                return camgroup_ctx->cam_ctxs_array[i];
            }
        }
    }

    LOGD("%s: get sensor %s aiq ctx failed !", __func__, sns_entity_name);

    EXIT_XCORE_FUNCTION();
#endif

    return NULL;

}

XCamReturn
rk_aiq_uapi2_camgroup_bind(rk_aiq_camgroup_ctx_t* camgroup_ctx, rk_aiq_sys_ctx_t** ctx, int num)
{
#ifdef RKAIQ_ENABLE_CAMGROUP
    ENTER_XCORE_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RKAIQ_API_SMART_LOCK(camgroup_ctx);

    if (camgroup_ctx->cam_ctxs_num + num > RK_AIQ_CAM_GROUP_MAX_CAMS) {
        LOGE("binded num %d + num %d > max %d !", camgroup_ctx->cam_ctxs_num, num, RK_AIQ_CAM_GROUP_MAX_CAMS);
        return XCAM_RETURN_ERROR_OUTOFRANGE;
    }

    for (int j = 0; j < num; j++) {
        // query bind status firstly
        bool need_bind = true;
        for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
            if (camgroup_ctx->cam_ctxs_array[i] == ctx[j]) {
                LOGI("already binded for ctx 0x%x", ctx[j]);
                need_bind = false;
                break;
            }
        }
        // find a empty slot
        if (need_bind) {
            for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
                if (camgroup_ctx->cam_ctxs_array[i] == NULL) {
                    ret = _cam_group_bind(camgroup_ctx, ctx[j]);
                    if (ret) {
                        LOGE("%s: bind sensor %s aiq ctx 0x%x failed !",
                             __func__, ctx[j]->_sensor_entity_name, ctx[j]);
                        break;
                    }
                }
            }
        }
    }

    LOGD("%s: bind sensor aiq ctxs success !", __func__);

    EXIT_XCORE_FUNCTION();
#endif

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi2_camgroup_unbind(rk_aiq_camgroup_ctx_t* camgroup_ctx, rk_aiq_sys_ctx_t** ctx, int num)
{
#ifdef RKAIQ_ENABLE_CAMGROUP
    ENTER_XCORE_FUNCTION();

    RKAIQ_API_SMART_LOCK(camgroup_ctx);

    for (int j = 0; j < num; j++) {
        for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
            if (camgroup_ctx->cam_ctxs_array[i] == ctx[j]) {
                if (_cam_group_unbind(camgroup_ctx, ctx[j])) {
                    LOGE("%s: unbind sensor %s aiq ctx 0x%x failed !",
                         __func__, ctx[j]->_sensor_entity_name, ctx[j]);
                    break;
                }
            }
        }
    }

    LOGD("%s: unbind sensor aiq ctxs success !", __func__);

    EXIT_XCORE_FUNCTION();
#endif

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi2_camgroup_prepare(rk_aiq_camgroup_ctx_t* camgroup_ctx, rk_aiq_working_mode_t mode)
{
#ifdef RKAIQ_ENABLE_CAMGROUP
    ENTER_XCORE_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RKAIQ_API_SMART_LOCK(camgroup_ctx);

    for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
        rk_aiq_sys_ctx_t* aiq_ctx = camgroup_ctx->cam_ctxs_array[i];
        if (aiq_ctx) {
            ret = rk_aiq_uapi_sysctl_prepare(aiq_ctx, 0, 0, mode);
            if (ret) {
                LOGE("%s: prepare failed for aiq ctx 0x%x !", __func__, aiq_ctx);
                continue;
            }
        }
    }

    ret = camgroup_ctx->cam_group_manager->prepare();
    if (ret) {
        LOGE("%s: prepare failed !", __func__);
        return ret;
    }

    LOGD("%s: prepare camgroup success !", __func__);

    EXIT_XCORE_FUNCTION();
#endif

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi2_camgroup_start(rk_aiq_camgroup_ctx_t* camgroup_ctx)
{
#ifdef RKAIQ_ENABLE_CAMGROUP
    ENTER_XCORE_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RKAIQ_API_SMART_LOCK(camgroup_ctx);

    ret = camgroup_ctx->cam_group_manager->start();
    if (ret) {
        LOGE("%s: start failed !", __func__);
        return ret;
    }

    for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
        rk_aiq_sys_ctx_t* aiq_ctx = camgroup_ctx->cam_ctxs_array[i];
        if (aiq_ctx) {
            ret = rk_aiq_uapi_sysctl_start(aiq_ctx);
            if (ret) {
                LOGE("%s: start failed for aiq ctx 0x%x !", __func__, aiq_ctx);
                continue;
            }
        }
    }

    LOGD("%s: start camgroup success !", __func__);

    EXIT_XCORE_FUNCTION();
#endif

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi2_camgroup_stop(rk_aiq_camgroup_ctx_t* camgroup_ctx)
{
#ifdef RKAIQ_ENABLE_CAMGROUP
    ENTER_XCORE_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RKAIQ_API_SMART_LOCK(camgroup_ctx);

    for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
        rk_aiq_sys_ctx_t* aiq_ctx = camgroup_ctx->cam_ctxs_array[i];
        if (aiq_ctx) {
            ret = rk_aiq_uapi_sysctl_stop(aiq_ctx, false);
            if (ret) {
                LOGE("%s: stop failed for aiq ctx 0x%x !", __func__, aiq_ctx);
                continue;
            }
        }
    }

    ret = camgroup_ctx->cam_group_manager->stop();
    if (ret) {
        LOGE("%s: stop failed !", __func__);
        return ret;
    }

    LOGD("%s: stop camgroup success !", __func__);

    EXIT_XCORE_FUNCTION();
#endif

    return XCAM_RETURN_NO_ERROR;

}

XCamReturn
rk_aiq_uapi2_camgroup_destroy(rk_aiq_camgroup_ctx_t* camgroup_ctx)
{
#ifdef RKAIQ_ENABLE_CAMGROUP
    ENTER_XCORE_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    {
        RKAIQ_API_SMART_LOCK(camgroup_ctx);
        for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
            rk_aiq_sys_ctx_t* aiq_ctx = camgroup_ctx->cam_ctxs_array[i];
            if (aiq_ctx) {
                ret = _cam_group_unbind(camgroup_ctx, aiq_ctx);
                if (ret) {
                    LOGE("%s: unbind sensor %s aiq ctx 0x%x failed !",
                         __func__, aiq_ctx->_sensor_entity_name, aiq_ctx);
                    continue;
                }
                rk_aiq_uapi_sysctl_deinit(aiq_ctx);
            }
        }

        if (camgroup_ctx->cam_ctxs_num > 0) {
            LOGE("impossible case, some aiq ctx may not deinit !");
        }
        ret = camgroup_ctx->cam_group_manager->deInit();
        if (ret) {
            LOGE("%s: deinit failed !", __func__);
            return ret;
        }
        camgroup_ctx->cam_group_manager.release();
    }
    if (camgroup_ctx->_camgroup_calib) {
        RkAiqCalibDbV2::CamCalibDbCamgroupFree(camgroup_ctx->_camgroup_calib);
    }
    if (camgroup_ctx->_srcOverlapMap_s)
        delete camgroup_ctx->_srcOverlapMap_s;
    camgroup_ctx->_apiMutex.release();
    delete camgroup_ctx;

    LOGD("%s: destroy camgroup success !", __func__);

    EXIT_XCORE_FUNCTION();
#endif

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi2_camgroup_getCamInfos(rk_aiq_camgroup_ctx_t* camgroup_ctx, rk_aiq_camgroup_camInfos_t* camInfos)
{
#ifdef RKAIQ_ENABLE_CAMGROUP
    ENTER_XCORE_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    {
        RKAIQ_API_SMART_LOCK(camgroup_ctx);
        if (!camInfos) {
            LOGE("null params !");
            return XCAM_RETURN_ERROR_PARAM;
        }

        camInfos->valid_sns_num = camgroup_ctx->cam_ctxs_num;

        for (int i = 0, j = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
            rk_aiq_sys_ctx_t* aiq_ctx = camgroup_ctx->cam_ctxs_array[i];

            if (aiq_ctx) {
                camInfos->sns_ent_nm[j] = aiq_ctx->_sensor_entity_name;
                camInfos->sns_camPhyId[j] = aiq_ctx->_camPhyId;
                j++;
            }
        }
    }
    EXIT_XCORE_FUNCTION();
#endif

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi2_camgroup_resetCam(rk_aiq_camgroup_ctx_t* camgroup_ctx, int camId)
{
#ifdef RKAIQ_ENABLE_CAMGROUP
    ENTER_XCORE_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (!camgroup_ctx)
        return XCAM_RETURN_ERROR_PARAM;

    for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
        if (!camCtx)
            continue;

        if (camCtx->_camPhyId == camId) {
            LOGD("cam: %d, reset...", camId);
            ret = camCtx->_camHw->reset_hardware();
            if (ret) {
                LOGE("failed to reset hardware\n");
                return XCAM_RETURN_ERROR_IOCTL;
            }
        }
    }

    EXIT_XCORE_FUNCTION();
#endif

    return XCAM_RETURN_NO_ERROR;
}

