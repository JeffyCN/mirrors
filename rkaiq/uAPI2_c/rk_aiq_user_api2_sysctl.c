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

#define RKAIQSYS_CHECK_RET(cond, ret, format, ...) \
    if ((cond)) { \
        LOGE(format, ##__VA_ARGS__); \
        return ret; \
    }

#define RKAIQ_DEFAULT_IQ_PATH "/etc/iqfiles/"

#ifdef ANDROID_OS
#include <cutils/properties.h>
#endif
#include <sys/file.h>
#include <fcntl.h>
#include "rk_aiq_user_api2_sysctl.h"
#include "uAPI2_c/rk_aiq_user_api2_common.h"
#include "uAPI2_c/rk_aiq_api_private_c.h"
#include "RkAiqManager_c.h"
#include "RkAiqCalibDbV2.h"
#include "scene/scene_manager.h"
#include "RkAiqVersion.h"
#include "common/rkaiq_ini.h"
#include "hwi_c/aiq_fake_camhw.h"
#include "uAPI2/rk_aiq_user_api2_ae.h"

#include "uAPI2/rk_aiq_user_api2_ablc_v32.h"
#include "common/rkisp32-config.h"
#include "RkAiqCalibVersion.h"
#include "uAPI2/rk_aiq_user_api2_imgproc.h"

#include "uAPI2/rk_aiq_user_api2_custom2_awb.h"
#include "uAPI2/rk_aiq_user_api2_awb_v3.h"

#include "uAPI2/rk_aiq_user_ae_thread_v25_itf.h"
#include "uAPI2/rk_aiq_user_api2_stats.h"

#if RKAIQ_HAVE_DUMPSYS
#include "rk_aiq_registry.h"
#include "rk_ipcs_service.h"
#endif

#if RKAIQ_HAVE_AIBNR
#include "uAPI2/rk_aiq_user_api2_aibnr.h"
#endif

int g_rkaiq_isp_hw_ver = 0;
static bool g_bypass_uapi = false;

typedef struct dev_buf_cfg_s dev_buf_cfg_t;
struct dev_buf_cfg_s {
    char dev_ent[64];
    int buf_cnt;
    dev_buf_cfg_t* next;
};

typedef struct rk_aiq_sys_preinit_cfg_s rk_aiq_sys_preinit_cfg_t;
struct rk_aiq_sys_preinit_cfg_s {
    char sns_ent_name[64];
    rk_aiq_working_mode_t mode;
    char force_iq_file[256];
    char main_scene[64];
    char sub_scene[64];
    rk_aiq_hwevt_cb hwevt_cb;
    void* hwevt_cb_ctx;
    void* calib_proj;
    rk_aiq_iq_buffer_info_t iq_buffer;
    dev_buf_cfg_t* dev_buf_cnt_map;
    rk_aiq_control_preinit_t ctrl_info;
    bool is_use_as_fake;
    rk_aiq_sys_preinit_cfg_t* next;
};

static rk_aiq_sys_preinit_cfg_t* g_rk_aiq_sys_preinit_cfg_map = NULL;
static void rk_aiq_init_lib(void) /*__attribute__((constructor))*/;
static void rk_aiq_deinit_lib(void) /*__attribute__((destructor))*/;
static bool g_rk_aiq_init_lib = false;

static void _set_fast_aewb_as_init(const rk_aiq_sys_ctx_t* ctx, rk_aiq_working_mode_t mode);

rk_aiq_camgroup_ctx_t* get_binded_group_ctx(const rk_aiq_sys_ctx_t* ctx)
{
#ifdef RKAIQ_ENABLE_CAMGROUP
    if (ctx->_camGroupManager)
        return (rk_aiq_camgroup_ctx_t*)AiqCamGroupManager_getContainerCtx(ctx->_camGroupManager);
    else
#endif
        return NULL;
}

static rk_aiq_sys_preinit_cfg_t* getPreinitCfgs(const char* sns_ent_name)
{
    rk_aiq_sys_preinit_cfg_t* tmp = g_rk_aiq_sys_preinit_cfg_map;
    rk_aiq_sys_preinit_cfg_t* prev = tmp;
    while (tmp) {
        if (strcmp(tmp->sns_ent_name, sns_ent_name) == 0)
            break;
        prev = tmp;
        tmp = tmp->next;
    }

    if (!tmp) {
        tmp = aiq_mallocz(sizeof(rk_aiq_sys_preinit_cfg_t));
        strcpy(tmp->sns_ent_name, sns_ent_name);
        LOGK("%s: create preinitCfg success !", tmp->sns_ent_name);
        if (prev)
            prev->next = tmp;
        else
            g_rk_aiq_sys_preinit_cfg_map = tmp;
    } else {
        //prev->next = tmp;
    }

    return tmp;
}

static rk_aiq_sys_preinit_cfg_t* findPreinitCfgs(const char* sns_ent_name)
{
    rk_aiq_sys_preinit_cfg_t* tmp = g_rk_aiq_sys_preinit_cfg_map;
    while (tmp) {
        if (strcmp(tmp->sns_ent_name, sns_ent_name) == 0)
            break;
        tmp = tmp->next;
    }
    return tmp;
}

XCamReturn
rk_aiq_uapi2_sysctl_preInit(const char* sns_ent_name,
                            rk_aiq_working_mode_t mode,
                            const char* force_iq_file)
{
    rk_aiq_sys_preinit_cfg_t* pPreinitCfg = getPreinitCfgs(sns_ent_name);

    XCAM_ASSERT(pPreinitCfg);

    pPreinitCfg->mode = mode;

    if (force_iq_file)
        strcpy(pPreinitCfg->force_iq_file, force_iq_file);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi2_sysctl_regHwEvtCb(const char* sns_ent_name,
                               rk_aiq_hwevt_cb hwevt_cb,
                               void* cb_ctx)
{
    rk_aiq_sys_preinit_cfg_t* pPreinitCfg = getPreinitCfgs(sns_ent_name);

    XCAM_ASSERT(pPreinitCfg);

    pPreinitCfg->hwevt_cb = hwevt_cb;
    pPreinitCfg->hwevt_cb_ctx = cb_ctx;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi2_sysctl_preInit_scene(const char* sns_ent_name, const char *main_scene,
                                  const char *sub_scene)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (!sns_ent_name || !main_scene || !sub_scene) {
        LOGE("Invalid input parameter");
        return XCAM_RETURN_ERROR_PARAM;
    }

    LOGK("%s: main_scene: %s, sub_scene: %s", __func__, main_scene, sub_scene);
    rk_aiq_sys_preinit_cfg_t* pPreinitCfg = getPreinitCfgs(sns_ent_name);

    XCAM_ASSERT(pPreinitCfg);

    strcpy(pPreinitCfg->main_scene, main_scene);
    strcpy(pPreinitCfg->sub_scene, sub_scene);

    return (ret);
}

XCamReturn
rk_aiq_uapi2_sysctl_preInit_iq_addr(const char* sns_ent_name, void *addr, size_t len)
{
    rk_aiq_sys_preinit_cfg_t* pPreinitCfg = getPreinitCfgs(sns_ent_name);

    XCAM_ASSERT(pPreinitCfg);

    pPreinitCfg->iq_buffer.addr = addr;
    pPreinitCfg->iq_buffer.len = len;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi2_sysctl_preInit_calibproj(const char* sns_ent_name, void *addr)
{
    rk_aiq_sys_preinit_cfg_t* pPreinitCfg = getPreinitCfgs(sns_ent_name);

    XCAM_ASSERT(pPreinitCfg);

    pPreinitCfg->calib_proj = addr;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn rk_aiq_uapi2_sysctl_preInit_devBufCnt(const char* sns_ent_name, const char* dev_ent,
        int buf_cnt) {
    if (!sns_ent_name || !dev_ent) {
        LOGE("Invalid input parameter");
        return XCAM_RETURN_ERROR_PARAM;
    }

    rk_aiq_sys_preinit_cfg_t* pPreinitCfg = getPreinitCfgs(sns_ent_name);

    XCAM_ASSERT(pPreinitCfg);


    dev_buf_cfg_t* tmp = pPreinitCfg->dev_buf_cnt_map;
    dev_buf_cfg_t* prev = tmp;
    while (tmp) {
        if (strcmp(dev_ent, tmp->dev_ent) == 0)
            break;
        prev = tmp;
        tmp = tmp->next;
    }

    if (!tmp) {
        tmp = (dev_buf_cfg_t*)aiq_mallocz(sizeof(dev_buf_cfg_t));
        strcpy(tmp->dev_ent, dev_ent);
        tmp->buf_cnt = buf_cnt;
        if (prev)
            prev->next = tmp;
        else
            pPreinitCfg->dev_buf_cnt_map = tmp;
    } else {
        //prev->next = tmp;
        tmp->buf_cnt = buf_cnt;
    }

    LOGK("%s: dev_ent:%s, buf_cnt:%d", __func__, dev_ent, buf_cnt);

    return XCAM_RETURN_NO_ERROR;
}

static void _print_versions()
{
    printf("\n"
           "************************** VERSION INFOS **************************\n"
           "version release date: %s\n"
           "         AIQ:       %s\n"
           "git logs:\n%s\n"
           "************************ VERSION INFOS END ************************\n"
           , RK_AIQ_RELEASE_DATE
           , RK_AIQ_VERSION
#ifdef GITLOGS
           , GITLOGS
#else
           , "null"
#endif
          );
}

static void rk_aiq_init_lib(void)
{
    xcam_get_log_level();
    ENTER_XCORE_FUNCTION();

    AiqCamHw_initCamHwInfos();
    rk_aiq_static_info_t* s_info = AiqCamHw_getStaticCamHwInfo(NULL, 0);
    if (s_info != NULL) {
        if (s_info->isp_hw_ver == 4)
            g_rkaiq_isp_hw_ver = 20;
        else if (s_info->isp_hw_ver == 5)
            g_rkaiq_isp_hw_ver = 21;
        else if (s_info->isp_hw_ver == 6)
            g_rkaiq_isp_hw_ver = 30;
        else if (s_info->isp_hw_ver == 7)
            g_rkaiq_isp_hw_ver = 32;
        else if (s_info->isp_hw_ver == 8)
            g_rkaiq_isp_hw_ver = 321;
        else if (s_info->isp_hw_ver == 9)
            g_rkaiq_isp_hw_ver = 33;
        else if (s_info->isp_hw_ver == 10)
            g_rkaiq_isp_hw_ver = 39;
        else if (s_info->isp_hw_ver == 11)
            g_rkaiq_isp_hw_ver = 35;
        else
            LOGE("do not support isp hw ver %d now !", s_info->isp_hw_ver);
    }

// // ydb: for test on 1103b
// #if defined(ISP_HW_V35)
//     g_rkaiq_isp_hw_ver = 35;
// #endif
    LOGK("%s, ISP HW ver: %d", __func__, g_rkaiq_isp_hw_ver);

#if defined(ISP_HW_V20)
    assert(g_rkaiq_isp_hw_ver == 20);
#elif defined(ISP_HW_V21)
    assert(g_rkaiq_isp_hw_ver == 21);
#elif defined(ISP_HW_V30)
    assert(g_rkaiq_isp_hw_ver == 30);
#elif defined(ISP_HW_V32)
    assert(g_rkaiq_isp_hw_ver == 32);
#elif defined(ISP_HW_V32_LITE)
    assert(g_rkaiq_isp_hw_ver == 321);
#elif defined(ISP_HW_V39)
    assert(g_rkaiq_isp_hw_ver == 39);
#elif defined(ISP_HW_V33)
    assert(g_rkaiq_isp_hw_ver == 33);
#elif defined(ISP_HW_V35)
    assert(g_rkaiq_isp_hw_ver == 35);
#else
#error "WRONG ISP_HW_VERSION, ONLY SUPPORT V20 AND V21 NOW !"
#endif
    _print_versions();
    EXIT_XCORE_FUNCTION();
}

static void rk_aiq_deinit_lib(void)
{
    ENTER_XCORE_FUNCTION();

    CamCalibDbReleaseCalibDbProj();
    AiqCamHw_clearStaticCamHwInfo();

    EXIT_XCORE_FUNCTION();
}

const char*
rk_aiq_uapi2_sysctl_rawReproc_preInit(const char* isp_driver,
                                      rk_aiq_control_preinit_t ctrl_info)
{
    if (!g_rk_aiq_init_lib) {
        rk_aiq_init_lib();
        g_rk_aiq_init_lib = true;
    }

    if (!isp_driver) {
        LOGE("%s: input isp driver name is NULL, please check input", __FUNCTION__);
        return NULL;
    }

    const char* sns_name = NULL;
    sns_name = AiqCamHw_rawReproc_preInit(isp_driver);
    if (sns_name) {

        if (ctrl_info.mode != RK_AIQ_CONTROL_CIS_ISP_PARAM &&
                ctrl_info.mode != RK_AIQ_CONTROL_ISP_PARAM_ONLY) {
            LOGK("%s limit mode to RK_AIQ_CONTROL_CIS_ISP_PARAM or RK_AIQ_CONTROL_ISP_PARAM_ONLY, "
                 "no support to set mode to %d", __FUNCTION__, ctrl_info.mode);
            return sns_name;
        }

        rk_aiq_sys_preinit_cfg_t* pPreinitCfg = getPreinitCfgs(sns_name);

        XCAM_ASSERT(pPreinitCfg);

        pPreinitCfg->ctrl_info = ctrl_info;
        if (ctrl_info.mode == RK_AIQ_CONTROL_ISP_PARAM_ONLY) {
            rk_aiq_frame_info_t exp_finfo = ctrl_info.isp_param_only_info.first_exp_info;
            LOGD("exp_finfo %d, %d, %f, %f\n",
                 exp_finfo.normal_gain_reg,
                 exp_finfo.normal_exp_reg,
                 exp_finfo.normal_exp,
                 exp_finfo.normal_gain);
            pPreinitCfg->is_use_as_fake = true;
        } else if (ctrl_info.mode == RK_AIQ_CONTROL_ISP_PARAM_AND_RAW) {
            pPreinitCfg->is_use_as_fake = true;
        }
    }

    return sns_name;
}

void rk_aiq_uapi2_sysctl_rawReproc_genIspParams (rk_aiq_sys_ctx_t* sys_ctx,
        uint32_t sequence,
        rk_aiq_frame_info_t *next_frm_info,
        int mode)
{
    AiqCamHw_rawReproc_genIspParams(sys_ctx->_camHw, sequence, next_frm_info, mode);
}

static XCamReturn
rk_aiq_uapi_sysctl_rawReproc_setInitExpInfo(const rk_aiq_sys_ctx_t* ctx,
        rk_aiq_working_mode_t mode)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_sys_preinit_cfg_t* pPreinitCfg = findPreinitCfgs(ctx->_sensor_entity_name);
    if (pPreinitCfg && pPreinitCfg->ctrl_info.mode == RK_AIQ_CONTROL_ISP_PARAM_ONLY) {
        rk_aiq_frame_info_t* _finfo = &pPreinitCfg->ctrl_info.isp_param_only_info.first_exp_info;
        LOGK("_finfo %d, %d, %f, %f\n",
             _finfo->normal_gain_reg,
             _finfo->normal_exp_reg,
             _finfo->normal_exp,
             _finfo->normal_gain);

#ifndef USE_NEWSTRUCT
        if (mode == RK_AIQ_WORKING_MODE_NORMAL) {
            Uapi_LinExpAttrV2_t LinExpAttr;
            ret = rk_aiq_user_api2_ae_getLinExpAttr(ctx, &LinExpAttr);

            LinExpAttr.sync.sync_mode = RK_AIQ_UAPI_MODE_DEFAULT;
            LinExpAttr.sync.done = false;
            LinExpAttr.Params.InitExp.InitTimeValue = (float)_finfo->normal_exp;
            LinExpAttr.Params.InitExp.InitGainValue = (float)_finfo->normal_gain;

            ret = rk_aiq_user_api2_ae_setLinExpAttr(ctx, LinExpAttr);
            if (ret == XCAM_RETURN_NO_ERROR) {
                LOGD("set linear offline exp params success\n");
            }
        }
        else {
            Uapi_HdrExpAttrV2_t HdrExpAttr;
            ret = rk_aiq_user_api2_ae_getHdrExpAttr(ctx, &HdrExpAttr);

            HdrExpAttr.sync.sync_mode = RK_AIQ_UAPI_MODE_DEFAULT;
            HdrExpAttr.sync.done = false;
            HdrExpAttr.Params.InitExp.InitTimeValue[0] = (float)_finfo->hdr_exp_s;
            HdrExpAttr.Params.InitExp.InitGainValue[0] = (float)_finfo->hdr_gain_s;
            HdrExpAttr.Params.InitExp.InitTimeValue[1] = (float)_finfo->hdr_exp_m;
            HdrExpAttr.Params.InitExp.InitGainValue[1] = (float)_finfo->hdr_gain_m;
            HdrExpAttr.Params.InitExp.InitTimeValue[2] = (float)_finfo->hdr_exp_l;
            HdrExpAttr.Params.InitExp.InitGainValue[2] = (float)_finfo->hdr_gain_l;

            ret = rk_aiq_user_api2_ae_setHdrExpAttr(ctx, HdrExpAttr);
            if (ret == XCAM_RETURN_NO_ERROR) {
                LOGD("set HDR offline exp params success\n");
            }
        }
#else
        if (mode == RK_AIQ_WORKING_MODE_NORMAL) {
            ae_api_linExpAttr_t LinExpAttr;
            ret = rk_aiq_user_api2_ae_getLinExpAttr(ctx, &LinExpAttr);

            LinExpAttr.initExp.sw_aeT_initTime_val = (float)_finfo->normal_exp;
            LinExpAttr.initExp.sw_aeT_initGain_val = (float)_finfo->normal_gain;

            ret = rk_aiq_user_api2_ae_setLinExpAttr(ctx, LinExpAttr);
            if (ret == XCAM_RETURN_NO_ERROR) {
                LOGD("set linear offline exp params success\n");
            }
        } else {
            ae_api_hdrExpAttr_t HdrExpAttr;
            ret = rk_aiq_user_api2_ae_getHdrExpAttr(ctx, &HdrExpAttr);

            HdrExpAttr.initExp.sw_aeT_initTime_val[0] = (float)_finfo->hdr_exp_s;
            HdrExpAttr.initExp.sw_aeT_initGain_val[0] = (float)_finfo->hdr_gain_s;
            HdrExpAttr.initExp.sw_aeT_initTime_val[1] = (float)_finfo->hdr_exp_m;
            HdrExpAttr.initExp.sw_aeT_initGain_val[1] = (float)_finfo->hdr_gain_m;
            HdrExpAttr.initExp.sw_aeT_initTime_val[2] = (float)_finfo->hdr_exp_l;
            HdrExpAttr.initExp.sw_aeT_initGain_val[2] = (float)_finfo->hdr_gain_l;

            ret = rk_aiq_user_api2_ae_setHdrExpAttr(ctx, HdrExpAttr);
            if (ret == XCAM_RETURN_NO_ERROR) {
                LOGD("set HDR offline exp params success\n");
            }
        }
#endif

        AiqCamHw_rawReProc_prepare(ctx->_camHw, 0);
    }

    return ret;
}

static int rk_aiq_rawReproc_init(rk_aiq_sys_ctx_t* ctx)
{
    rk_aiq_sys_preinit_cfg_t* pPreinitCfg = findPreinitCfgs(ctx->_sensor_entity_name);
    if (pPreinitCfg) {
        rk_aiq_control_preinit_t* info = &pPreinitCfg->ctrl_info;
        ctx->_ctrlPreinit_info = info;
        if (info->mode == RK_AIQ_CONTROL_ISP_PARAM_ONLY) {
            ctx->_use_fakecam      = true;
        }
    }

    return 0;
}

static const char*
rk_aiq_uapi2_sysctl_getBindedIspDrvNmBySns(const char *sns_ent_name)
{
    if (!g_rk_aiq_init_lib) {
        rk_aiq_init_lib();
        g_rk_aiq_init_lib = true;
    }

    return AiqCamHw_getBindedIspDrvNmBySns(sns_ent_name);
}

XCamReturn
rk_aiq_uapi2_sysctl_preInit_rkrawstream_info(const char* sns_ent_name,
        const rk_aiq_rkrawstream_info_t* info)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (!sns_ent_name || !info) {
        LOGE("Invalid input parameter");
        return XCAM_RETURN_ERROR_PARAM;
    }

    const char* isp_drv = rk_aiq_uapi2_sysctl_getBindedIspDrvNmBySns(sns_ent_name);
    if (!isp_drv) {
        return XCAM_RETURN_ERROR_PARAM;
    }

    rk_aiq_control_preinit_t ctr_info;
    memset(&ctr_info, 0, sizeof(rk_aiq_control_preinit_t));

    switch (info->mode)
    {
    case RK_ISP_RKRAWSTREAM_MODE_INVALID:
        ctr_info.mode = RK_AIQ_CONTROL_DEFAULT;
        break;
    case RK_ISP_RKRAWSTREAM_MODE_HALF_ONLINE:
        ctr_info.mode = RK_AIQ_CONTROL_CIS_ISP_PARAM;
        break;
    case RK_ISP_RKRAWSTREAM_MODE_OFFLINE:
        ctr_info.mode = RK_AIQ_CONTROL_ISP_PARAM_ONLY;
        break;
    default:
        ctr_info.mode = RK_AIQ_CONTROL_DEFAULT;
        break;
    }

    const char* sns_ret = NULL;
    sns_ret = rk_aiq_uapi2_sysctl_rawReproc_preInit(isp_drv, ctr_info);
    if (!sns_ret) {
        return XCAM_RETURN_ERROR_PARAM;
    }

    return (ret);
}

XCamReturn
rk_aiq_uapi2_sysctl_prepareRkRaw(const rk_aiq_sys_ctx_t* ctx, rk_aiq_raw_prop_t prop)
{
    ENTER_XCORE_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    rk_aiq_sys_ctx_array_t ctx_array = rk_aiq_user_api2_common_getSysCtxArray(ctx);
    for (int i = 0; i < ctx_array.num; i++) {
        ret = AiqManager_rawdataPrepare(ctx_array.ctx[i]->_rkAiqManager, prop);
    }
    EXIT_XCORE_FUNCTION();

    return ret;
}

XCamReturn
rk_aiq_uapi2_sysctl_enqueueRkRawBuf(const rk_aiq_sys_ctx_t* ctx, void *rawdata, bool sync)
{
    ENTER_XCORE_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    rk_aiq_sys_ctx_array_t ctx_array = rk_aiq_user_api2_common_getSysCtxArray(ctx);
    for (int i = 0; i < ctx_array.num; i++) {
        ret = AiqManager_enqueueRawBuffer(ctx_array.ctx[i]->_rkAiqManager, rawdata, sync);
    }
    EXIT_XCORE_FUNCTION();

    return ret;
}

XCamReturn
rk_aiq_uapi2_sysctl_enqueueRkRawFile(const rk_aiq_sys_ctx_t* ctx, const char *path)
{
    ENTER_XCORE_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    rk_aiq_sys_ctx_array_t ctx_array = rk_aiq_user_api2_common_getSysCtxArray(ctx);
    for (int i = 0; i < ctx_array.num; i++) {
        ret = AiqManager_enqueueRawFile(ctx_array.ctx[i]->_rkAiqManager, path);
    }
    EXIT_XCORE_FUNCTION();

    return ret;
}

XCamReturn
rk_aiq_uapi2_sysctl_registRkRawCb(const rk_aiq_sys_ctx_t* ctx, void (*callback)(void*))
{
    ENTER_XCORE_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (callback == NULL) return XCAM_RETURN_ERROR_PARAM;

    rk_aiq_sys_ctx_array_t ctx_array = rk_aiq_user_api2_common_getSysCtxArray(ctx);
    for (int i = 0; i < ctx_array.num; i++) {
        ret = AiqManager_registRawdataCb(ctx_array.ctx[i]->_rkAiqManager, callback);
    }
    EXIT_XCORE_FUNCTION();
    return ret;
}

XCamReturn
rk_aiq_uapi2_sysctl_getCamInfos(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_ctx_camInfo_t* camInfo)
{
    ENTER_XCORE_FUNCTION();
    XCAM_ASSERT(sys_ctx != NULL);
    RKAIQ_API_SMART_LOCK(sys_ctx);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (!camInfo) {
        LOGE("null params !");
        return XCAM_RETURN_ERROR_PARAM;
    }

    camInfo->sns_ent_nm = sys_ctx->_sensor_entity_name;
    camInfo->sns_camPhyId = sys_ctx->_camPhyId;

    EXIT_XCORE_FUNCTION();

    return ret;
}

XCamReturn
rk_aiq_uapi2_sysctl_getWorkingMode(const rk_aiq_sys_ctx_t* ctx, rk_aiq_working_mode_t *mode)
{
    ENTER_XCORE_FUNCTION();
    if (!mode || !ctx)
        return XCAM_RETURN_ERROR_PARAM;

    RKAIQ_API_SMART_LOCK(ctx);
    *mode = ctx->_rkAiqManager->mWorkingMode;
    if (*mode < 0)
        return XCAM_RETURN_ERROR_OUTOFRANGE;
    EXIT_XCORE_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

int rk_aiq_uapi2_sysctl_tuning_enable(rk_aiq_sys_ctx_t* sys_ctx, bool enable)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (!sys_ctx) {
        LOGE("%s: sys_ctx is invalied\n", __func__);
        return XCAM_RETURN_ERROR_PARAM;
    }

#ifdef RKAIQ_ENABLE_CAMGROUP
    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
        rk_aiq_camgroup_ctx_t* grp_ctx = (rk_aiq_camgroup_ctx_t*)(sys_ctx);
        if (!grp_ctx)
            return XCAM_RETURN_ERROR_FAILED;

        rk_aiq_sys_ctx_t* cam_ctx = NULL;
        for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
            cam_ctx = grp_ctx->cam_ctxs_array[i];
            if (cam_ctx) {
                if (enable && cam_ctx->_socket) {
                    LOGW("%s: socket server is already enabled!\n", __func__);
                    continue;
                }

                if (!enable && !cam_ctx->_socket) {
                    LOGW("%s: socket server is already disabled!\n", __func__);
                    continue;
                }

                if (enable) {
                    cam_ctx->_socket = (SocketClientCtx_t*)aiq_mallocz(sizeof(SocketClientCtx_t));
                    if (!cam_ctx->_socket) {
                        LOGE("_malloc failure!");
                        return XCAM_RETURN_ERROR_MEM;
                    }
                    socket_client_start(cam_ctx, cam_ctx->_socket, sys_ctx->_camPhyId);
                }
                else {
                    socket_client_exit(cam_ctx->_socket);
                    aiq_free(cam_ctx->_socket);
                    cam_ctx->_socket = NULL;
                }

                LOGD("%s: change socket server status to %d!\n", __func__, enable);
            }
        }
        return XCAM_RETURN_NO_ERROR;
    }
#endif

    if (enable && sys_ctx->_socket) {
        LOGW("%s: socket server is already enabled!\n", __func__);
        return XCAM_RETURN_NO_ERROR;
    }

    if (!enable && !sys_ctx->_socket) {
        LOGW("%s: socket server is already disabled!\n", __func__);
        return XCAM_RETURN_NO_ERROR;
    }

    if (enable) {
        sys_ctx->_socket = (SocketClientCtx_t*)aiq_mallocz(sizeof(SocketClientCtx_t));
        if (!sys_ctx->_socket) {
            LOGE("_malloc failure!");
            return XCAM_RETURN_ERROR_MEM;
        }
        socket_client_start(sys_ctx, sys_ctx->_socket, sys_ctx->_camPhyId);
    }
    else {
        socket_client_exit(sys_ctx->_socket);
        aiq_free(sys_ctx->_socket);
        sys_ctx->_socket = NULL;
    }

    LOGD("%s: change socket server status to %d!\n", __func__, enable);
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi2_sysctl_resetCam(const rk_aiq_sys_ctx_t* sys_ctx, int camId)
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

    ret = AiqCamHw_reset_hardware(sys_ctx->_camHw);
    if (ret) {
        LOGE("failed to reset hardware\n");
    }

    return ret;
}

static void
rk_aiq_uapi2_sysctl_deinit_locked(rk_aiq_sys_ctx_t* ctx)
{
#if RKAIQ_HAVE_DUMPSYS
    aiq_ipcs_exit();
    RKAIQRegistry_deinit();
#endif

    if (ctx->_rkAiqManager) {
        AiqManager_deinit(ctx->_rkAiqManager);
        aiq_free(ctx->_rkAiqManager);
    }

    if (ctx->_camHw) {
        aiq_free(ctx->_camHw);
        ctx->_camHw = NULL;
    }

    if (ctx->_analyzer) {
        aiq_free(ctx->_analyzer);
        ctx->_analyzer = NULL;
    }

    if (ctx->_socket) {
        socket_client_exit(ctx->_socket);
        aiq_free(ctx->_socket);
        ctx->_socket = NULL;
    }

    if (ctx->_calibDbProj) {
        // TODO:public common resource release
    }

    if (ctx->_lock_file) {
        flock(fileno(ctx->_lock_file), LOCK_UN);
        fclose(ctx->_lock_file);
    }

    if (ctx->next_ctx) {
        aiq_free(ctx->next_ctx);
    }

    if (ctx->_sensor_entity_name)
        xcam_free((void*)(ctx->_sensor_entity_name));

    LOGK("cid[%d] %s success.", ctx->_camPhyId, __func__);
#if 0
    // TODO: this will release all sensor's calibs, and should
    // only release the current sensor's calib
    rk_aiq_deinit_lib();
    g_rk_aiq_init_lib = false;
#endif
}

void
rk_aiq_uapi2_sysctl_deinit(rk_aiq_sys_ctx_t* ctx)
{
    ENTER_XCORE_FUNCTION();
    {
        RKAIQ_API_SMART_LOCK(ctx);
        rk_aiq_uapi2_awb_unRegister(ctx);
        rk_aiq_uapi2_ae_unRegister(ctx);
        rk_aiq_uapi2_sysctl_deinit_locked(ctx);
        aiqMutex_deInit(&ctx->_apiMutex);
    }
    aiq_free(ctx);
    EXIT_XCORE_FUNCTION();
}

static int rk_aiq_offline_init(rk_aiq_sys_ctx_t* ctx)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#ifdef ANDROID_OS
    char use_as_fake_cam_env[PROPERTY_VALUE_MAX] = { 0 };
    property_get("persist.vendor.rkisp.use_as_fake_cam", use_as_fake_cam_env, "0");
#else
    char* use_as_fake_cam_env = getenv("USE_AS_FAKE_CAM");
#endif
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

#ifdef ANDROID_OS
    if (!ctx->_use_fakecam) {
        int use_fakecam = atoi(use_as_fake_cam_env);
        if (use_fakecam > 0) {
            ctx->_use_fakecam = true;
        }
        else {
            ctx->_use_fakecam = false;
        }
    }
#else
    if (use_as_fake_cam_env)
        ctx->_use_fakecam = atoi(use_as_fake_cam_env) > 0 ? true : false;
#endif

    LOGI("use fakecam %d", ctx->_use_fakecam);

    EXIT_XCORE_FUNCTION();

    return ret;
}

rk_aiq_sys_ctx_t*
rk_aiq_uapi2_sysctl_init(const char* sns_ent_name,
                         const char* config_file_dir,
                         rk_aiq_error_cb err_cb,
                         rk_aiq_metas_cb metas_cb)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ENTER_XCORE_FUNCTION();
    char config_file[258] = "\0";
    char* main_scene = NULL;
    char* sub_scene = "\0";
    rk_aiq_iq_buffer_info_t iq_buffer = { NULL, 0 };
    char lock_path[255] = "\0";
    int  lock_res = 0;
    void* calib_proj = NULL;
    rk_aiq_rtt_share_info_t* rtt_share = NULL;

    XCAM_ASSERT(sns_ent_name);

    if (!g_rk_aiq_init_lib) {
        rk_aiq_init_lib();
        g_rk_aiq_init_lib = true;
    }

    //check if group mode
    int lens = strlen(sns_ent_name);
    bool is_group_mode = false;
    if (sns_ent_name[lens - 1] == 'g' && sns_ent_name[lens - 2] == '-') {
        is_group_mode = true;
        *(char*)(sns_ent_name + lens - 2) = '\0';
    }

    bool is_ent_name = true;
    if (sns_ent_name[0] != 'm' || sns_ent_name[3] != '_')
        is_ent_name = false;

    if (!is_ent_name) {
        if (config_file_dir && (strlen(config_file_dir) > 0))
            sprintf(config_file, "%s/%s.json", config_file_dir, sns_ent_name);
        else
            sprintf(config_file, "%s/%s.json", RKAIQ_DEFAULT_IQ_PATH, sns_ent_name);
    }

    rk_aiq_sys_ctx_t* ctx = (rk_aiq_sys_ctx_t*)aiq_mallocz(sizeof(rk_aiq_sys_ctx_t));
    aiqMutex_init(&ctx->_apiMutex);
    RKAIQ_API_SMART_LOCK(ctx);

    ctx->_sensor_entity_name = strndup(sns_ent_name, 128);
    ctx->_rkAiqManager = (AiqManager_t*)aiq_mallocz(sizeof(AiqManager_t));

    rk_aiq_static_info_t* s_info = AiqCamHw_getStaticCamHwInfo(sns_ent_name, 0);
    // TODO: for sensor sync check(ensure 1608 sensor is slave mode).
    ctx->_is_1608_sensor = s_info->_is_1608_sensor;
    AiqManager_setCamPhyId(ctx->_rkAiqManager, s_info->sensor_info.phyId);

    ctx->_camPhyId = s_info->sensor_info.phyId;

    rk_aiq_offline_init(ctx);
    rk_aiq_rawReproc_init(ctx);
    ctx->_analyzer = (AiqCore_t*)aiq_mallocz(sizeof(AiqCore_t));
    if (strstr(sns_ent_name, "FakeCamera") || ctx->_use_fakecam) {
        //ctx->_camHw = new FakeCamHwIsp20();
        ctx->_use_fakecam = true;
        ctx->_camHw = (AiqCamHwBase_t*)aiq_mallocz(sizeof(AiqCamHwFake_t));
        ctx->_camHw->mIsFake = true;
#ifdef RKAIQ_ENABLE_FAKECAM
        if (s_info->isp_hw_ver == 4) {
#ifdef ISP_HW_V20
            ctx->_analyzer->mIspHwVer = 0;
#else
            XCAM_ASSERT(0);
#endif
        }
        else if (s_info->isp_hw_ver == 5) {
#ifdef ISP_HW_V21
            ctx->_analyzer->mIspHwVer = 1;
#else
            XCAM_ASSERT(0);
#endif
        }
        else if (s_info->isp_hw_ver == 6) {
#ifdef ISP_HW_V30
            ctx->_analyzer->mIspHwVer = 3;
#else
            XCAM_ASSERT(0);
#endif
        }
        else if (s_info->isp_hw_ver == 7 || s_info->isp_hw_ver == 8) {
#if defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
            ctx->_analyzer->mIspHwVer = 4;
#else
            XCAM_ASSERT(0);
#endif
        }
        else if (s_info->isp_hw_ver == 9) {
#if defined(ISP_HW_V33)
            ctx->_analyzer->mIspHwVer = 5;
#else
            XCAM_ASSERT(0);
#endif
        }
        else if (s_info->isp_hw_ver == 10) {
#if defined(ISP_HW_V39)
            ctx->_analyzer->mIspHwVer = 5;
#else
            XCAM_ASSERT(0);
#endif
        }
        else if (s_info->isp_hw_ver == 11) {
#if defined(ISP_HW_V35)
            ctx->_analyzer->mIspHwVer = 5;
#else
            XCAM_ASSERT(0);
#endif
        }
        else {
            LOGE("do not support this isp hw version %d !", s_info->isp_hw_ver);
            goto error;
        }
#endif
    }
    else {
        ctx->_camHw = (AiqCamHwBase_t*)aiq_mallocz(sizeof(AiqCamHwBase_t));
        if (s_info->isp_hw_ver == 4) {
#ifdef ISP_HW_V20
            ctx->_analyzer->mIspHwVer = 0;
#else
            XCAM_ASSERT(0);
#endif
        }
        else if (s_info->isp_hw_ver == 5) {
#ifdef ISP_HW_V21
            ctx->_analyzer->mIspHwVer = 1;
#else
            XCAM_ASSERT(0);
#endif
        }
        else if (s_info->isp_hw_ver == 6) {
#ifdef ISP_HW_V30
            ctx->_analyzer->mIspHwVer = 3;
#else
            XCAM_ASSERT(0);
#endif
        }
        else if (s_info->isp_hw_ver == 7 || s_info->isp_hw_ver == 8) {
#if defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
            ctx->_analyzer->mIspHwVer = 4;
#else
            XCAM_ASSERT(0);
#endif
        }
        else if (s_info->isp_hw_ver == 9) {
#if defined(ISP_HW_V33) || defined(ISP_HW_V35)
            ctx->_analyzer->mIspHwVer = 5;
#else
            XCAM_ASSERT(0);
#endif
        }
        else if (s_info->isp_hw_ver == 10) {
#if defined(ISP_HW_V39)
            ctx->_analyzer->mIspHwVer = 5;
#else
            XCAM_ASSERT(0);
#endif
        }
        else if (s_info->isp_hw_ver == 11) {
#if defined(ISP_HW_V35)
            ctx->_analyzer->mIspHwVer = 5;
#else
            XCAM_ASSERT(0);
#endif
        }
        else {
            LOGE("do not support this isp hw version %d !", s_info->isp_hw_ver);
            goto error;
        }
    }

    if (ctx->_ctrlPreinit_info) {
        AiqCamHw_setAiqPreCtrlInfo(ctx->_camHw, ctx->_ctrlPreinit_info);
    }

    // use user defined iq file
    {
        rk_aiq_sys_preinit_cfg_t* pPreinitCfg = findPreinitCfgs(ctx->_sensor_entity_name);
        int user_hdr_mode = -1;
        bool user_spec_iq = false;
        if (pPreinitCfg) {
            AiqManager_setHwEvtCb(ctx->_rkAiqManager, pPreinitCfg->hwevt_cb, pPreinitCfg->hwevt_cb_ctx);
            if (pPreinitCfg->dev_buf_cnt_map) {
                dev_buf_cfg_t* tmp = pPreinitCfg->dev_buf_cnt_map;
                int cnts = 0;
                while (tmp) {
                    cnts++;;
                    tmp = tmp->next;
                }
                AiqDevBufCntsInfo_t cfgs[cnts];
                tmp = pPreinitCfg->dev_buf_cnt_map;
                for (int j = 0; j < cnts; j++) {
                    strcpy(cfgs[j]._devName, tmp->dev_ent);
                    cfgs[j]._bufCnts = tmp->buf_cnt;
                    tmp = tmp->next;
                }
                AiqCamHw_setDevBufCnt(ctx->_camHw, cfgs, cnts);
            }

            if (pPreinitCfg->iq_buffer.addr && pPreinitCfg->iq_buffer.len > 0) {
                iq_buffer.addr = pPreinitCfg->iq_buffer.addr;
                iq_buffer.len = pPreinitCfg->iq_buffer.len;
                user_spec_iq = true;
                LOGI("use user sepcified iq addr %p, len: %zu", iq_buffer.addr, iq_buffer.len);
            }
            else if (strlen(pPreinitCfg->force_iq_file)) {
                sprintf(config_file, "%s/%s", config_file_dir, pPreinitCfg->force_iq_file);
                LOGI("use user sepcified iq file %s", config_file);
                user_spec_iq = true;
            }
            else if (pPreinitCfg->calib_proj) {
                calib_proj = pPreinitCfg->calib_proj;
                LOGI("use external CamCalibDbProj: %p", calib_proj);
            }
            else {
                user_hdr_mode = pPreinitCfg->mode;
                LOGI("selected by user sepcified hdr mode %d", user_hdr_mode);
            }

            if (strlen(pPreinitCfg->main_scene))
                main_scene = pPreinitCfg->main_scene;
            if (strlen(pPreinitCfg->sub_scene))
                sub_scene = pPreinitCfg->sub_scene;
        }

        // use auto selected iq file
        if (is_ent_name && !user_spec_iq && !ctx->_use_fakecam) {
            char iq_file[128] = { '\0' };
            AiqCamHw_selectIqFile(sns_ent_name, iq_file);

            char* hdr_mode = getenv("HDR_MODE");
            int start = strlen(iq_file) - strlen(".json");

            if (hdr_mode) {
                iq_file[start] = '\0';
                if (strstr(hdr_mode, "32"))
                    strcat(iq_file, "-hdr3.json");
                else
                    strcat(iq_file, "_normal.json");
            }

            if (config_file_dir) {
                sprintf(config_file, "%s/%s", config_file_dir, iq_file);
            }
            else {
                sprintf(config_file, "%s/%s", RKAIQ_DEFAULT_IQ_PATH, iq_file);
            }

            // use default iq file
            if (hdr_mode && access(config_file, F_OK)) {
                LOGW("%s not exist, will use the default !", config_file);
                if (strstr(hdr_mode, "32"))
                    start = strlen(config_file) - strlen("-hdr3.json");
                else
                    start = strlen(config_file) - strlen("_normal.json");
                config_file[start] = '\0';
                strcat(config_file, ".json");
            }
            LOGI("use iq file %s", config_file);
        }
        else if (ctx->_use_fakecam && !user_spec_iq) {
            if (config_file_dir && (strlen(config_file_dir) > 0))
                sprintf(config_file, "%s/%s", config_file_dir, "FakeCamera0.json");
            else
                sprintf(config_file, "%s/%s", RKAIQ_DEFAULT_IQ_PATH, "FakeCamera0.json");
        }
    }
    AiqCamHw_setCamPhyId(ctx->_camHw, s_info->sensor_info.phyId);
    ctx->_rkAiqManager->mCamHw = ctx->_camHw;

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
    AiqCore_setHwInfos(ctx->_analyzer, &ctx->_hw_info);
    AiqCore_setCamPhyId(ctx->_analyzer, s_info->sensor_info.phyId);

    ctx->_rkAiqManager->mRkAiqAnalyzer = ctx->_analyzer;
    // TODO
    ctx->_socket = (SocketClientCtx_t *)aiq_mallocz(sizeof(SocketClientCtx_t));
    if (!ctx->_socket) {
        LOGE("_malloc failure!");
        goto error;
    }

    if (strstr(config_file, ".xml")) {
        LOGE("Should use json instead of xml");
        strcpy(config_file + strlen(config_file) - strlen(".xml"), ".json");
    }

    CamCalibDbV2Context_t calibdbv2_ctx;
    xcam_mem_clear(calibdbv2_ctx);

    if (calib_proj)
        ctx->_calibDbProj = (CamCalibDbProj_t*)calib_proj;
    else if (iq_buffer.addr && iq_buffer.len > 0)
        ctx->_calibDbProj = CamCalibDbCreateCalibDbProjFromBuf(is_group_mode ? 0 : s_info->sensor_info.phyId, iq_buffer.addr, iq_buffer.len);
    else
        ctx->_calibDbProj = CamCalibDbCreateCalibDbProjFromFile(is_group_mode ? 0 : s_info->sensor_info.phyId, config_file);
    if (!ctx->_calibDbProj)
        goto error;

    if (main_scene && sub_scene)
        calibdbv2_ctx = RkAiqSceneManagerRefToScene(ctx->_calibDbProj,
                        main_scene, sub_scene);

    if (!calibdbv2_ctx.calib_scene) {
        if (main_scene && sub_scene)
            LOGE("Failed to find params of %s:%s scene in json, using default scene",
                 main_scene, sub_scene);
        calibdbv2_ctx = CamCalibDbToDefaultCalibDb(ctx->_calibDbProj);
    }

    AiqManager_setAiqCalibDb(ctx->_rkAiqManager, &calibdbv2_ctx);

#ifndef __ANDROID__
    snprintf(lock_path, 255, "/tmp/aiq%d.lock", ctx->_camPhyId);
    ctx->_lock_file = fopen(lock_path, "w+");
    lock_res = flock(fileno(ctx->_lock_file), LOCK_EX);
    if (!lock_res) {
        LOGI("Locking aiq exclusive");
    }
    else {
        LOGI("Lock aiq exclusive failed with res %d", lock_res);
        fclose(ctx->_lock_file);
        ctx->_lock_file = NULL;
    }
#endif

    ret = AiqManager_init(ctx->_rkAiqManager, ctx->_sensor_entity_name, err_cb, metas_cb);
    if (ret)
        goto error;

    // TODO
    socket_client_start(ctx, ctx->_socket, ctx->_camPhyId);
    GlobalParamsManager_setSocket(&ctx->_rkAiqManager->mGlobalParamsManager, ctx->_socket);

    ctx->ctx_type = CTX_TYPE_USER_MAIN;
    ctx->next_ctx = (rk_aiq_sys_ctx_t*)aiq_mallocz(sizeof(rk_aiq_sys_ctx_t));
    RKAIQSYS_CHECK_RET(!ctx, NULL, "malloc toolserver ctx error !");
    *(ctx->next_ctx) = *ctx;
    ctx->next_ctx->ctx_type = CTX_TYPE_TOOL_SERVER;
    ctx->next_ctx->next_ctx = NULL;
    ctx->cam_type = RK_AIQ_CAM_TYPE_SINGLE;

    {
#ifdef ANDROID_OS
        char property_value[PROPERTY_VALUE_MAX] = { 0 };

        property_get("persist.vendor.rkisp.tuning.bypass_uapi", property_value, "1");
        if (atoi(property_value) != 0) {
            g_bypass_uapi = true;
        }
        else {
            g_bypass_uapi = false;
        }
#else
        const char* bypass_env = getenv("persist_camera_tuning_bypass_uapi");
        if (bypass_env && atoi(bypass_env) != 0) {
            g_bypass_uapi = true;
        }
        else {
            g_bypass_uapi = false;
        }
#endif
    }

    rk_aiq_uapi2_awb_register(ctx, NULL);
    rk_aiq_uapi2_ae_register(ctx, NULL);

#if RKAIQ_HAVE_DUMPSYS
    if (!is_group_mode) {
        aiq_ipcs_init();
        RKAIQRegistry_init();
        RKAIQRegistry_register(ctx);
    }
#endif

    LOGK("cid[%d] %s success. iq:%s", ctx->_camPhyId, __func__, config_file);
    EXIT_XCORE_FUNCTION();

    return ctx;

error:
    LOGE("_rkAiqManager init error!");
    rk_aiq_uapi2_sysctl_deinit_locked(ctx);
    return NULL;
}

XCamReturn
rk_aiq_uapi2_sysctl_prepare(const rk_aiq_sys_ctx_t* ctx,
                            uint32_t  width, uint32_t  height,
                            rk_aiq_working_mode_t mode)
{
    ENTER_XCORE_FUNCTION();
    XCAM_ASSERT(ctx != NULL);

    if (mode != RK_AIQ_WORKING_MODE_NORMAL) {
        if (ctx->_use_aiisp) {
            LOGE_ANALYZER("Aiisp does not supported HDR mode !");
            return XCAM_RETURN_ERROR_FAILED;
        }
#ifdef RKAIQ_HAVE_RGBIR_REMOSAIC
        rk_aiq_global_params_wrap_t rgbir_params;
        rgbir_params.type = RESULT_TYPE_RGBIR_PARAM;
        rgbir_params.man_param_ptr = NULL;
        rgbir_params.aut_param_ptr = NULL;
        GlobalParamsManager_get(&ctx->_rkAiqManager->mGlobalParamsManager, &rgbir_params);
        if (rgbir_params.en) {
            LOGE_ANALYZER("Rgbir does not supported HDR mode !");
            return XCAM_RETURN_ERROR_FAILED;
        }
#endif
    }

    if (ctx->_use_aiisp) {
        rk_aiq_global_params_wrap_t tnr_params;
        tnr_params.type = RESULT_TYPE_TNR_PARAM;
        tnr_params.man_param_ptr = NULL;
        tnr_params.aut_param_ptr = NULL;
        GlobalParamsManager_get(&ctx->_rkAiqManager->mGlobalParamsManager, &tnr_params);
        if (!tnr_params.en) {
            LOGE_ANALYZER("When using aiisp, bayertnr must be on!");
            return XCAM_RETURN_ERROR_FAILED;
        }
    }

    if (ctx->_use_fakecam && ctx->_raw_prop.format &&
            ctx->_raw_prop.frame_width &&
            ctx->_raw_prop.frame_height &&
            ctx->_raw_prop.rawbuf_type) {
        rk_aiq_uapi2_sysctl_prepareRkRaw(ctx, ctx->_raw_prop);
    }

    RKAIQ_API_SMART_LOCK(ctx);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    _set_fast_aewb_as_init(ctx, mode);

    rk_aiq_uapi_sysctl_rawReproc_setInitExpInfo(ctx, mode);

    ret = AiqManager_prepare(ctx->_rkAiqManager, width, height, mode);
    RKAIQSYS_CHECK_RET(ret, ret, "prepare failed !");

#if RKAIQ_HAVE_AIBNR
    rk_aiq_user_api2_aibnr_RecDefFps(ctx);
#endif
    LOGK("cid[%d] %s success. mode:%d ", ctx->_camPhyId, __func__, mode);

    EXIT_XCORE_FUNCTION();

    return ret;
}

XCamReturn
rk_aiq_uapi2_sysctl_start(const rk_aiq_sys_ctx_t* ctx)
{
    ENTER_XCORE_FUNCTION();
    RKAIQ_API_SMART_LOCK(ctx);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = AiqManager_start(ctx->_rkAiqManager);

    LOGK("cid[%d] %s success.", ctx->_camPhyId, __func__);

    EXIT_XCORE_FUNCTION();

    return ret;
}

XCamReturn
rk_aiq_uapi2_sysctl_stop(const rk_aiq_sys_ctx_t* ctx, bool keep_ext_hw_st)
{
    ENTER_XCORE_FUNCTION();
    RKAIQ_API_SMART_LOCK(ctx);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = AiqManager_stop(ctx->_rkAiqManager, keep_ext_hw_st);

    LOGK("cid[%d] %s success.", ctx->_camPhyId, __func__);

    EXIT_XCORE_FUNCTION();

    return ret;
}

XCamReturn
rk_aiq_uapi2_sysctl_updateIq(rk_aiq_sys_ctx_t* sys_ctx, char* iqfile)
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

    CamCalibDbProj_t* calibDbProj = CamCalibDbCreateCalibDbProjFromFile(sys_ctx->_camPhyId, iqfile);
    if (!calibDbProj) {
        LOGE("failed to create CalibDbProj from iqfile\n");
        return XCAM_RETURN_ERROR_PARAM;
    }

    CamCalibDbV2Context_t calibdbv2_ctx =
        CamCalibDbToDefaultCalibDb(calibDbProj);

    GlobalParamsManager_t* paramsManager = &sys_ctx->_rkAiqManager->mGlobalParamsManager;
    GlobalParamsManager_switchCalibDb(paramsManager, &calibdbv2_ctx, true);
    ret = AiqManager_updateCalibDb(sys_ctx->_rkAiqManager, &calibdbv2_ctx);

    if (ret) {
        LOGE("failed to update iqfile\n");
        return ret;
    }

    sys_ctx->_calibDbProj = calibDbProj;

    LOGK("cid[%d] %s: success. new iq:%s ", sys_ctx->_camPhyId, __func__, iqfile);

    return XCAM_RETURN_NO_ERROR;
}

int32_t
rk_aiq_uapi2_sysctl_getModuleCtl(const rk_aiq_sys_ctx_t* ctx,
                                 rk_aiq_module_id_t mId, bool *mod_en)
{
    ENTER_XCORE_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RKAIQ_API_SMART_LOCK(ctx);
    const rk_aiq_sys_ctx_t* sys_ctx = rk_aiq_user_api2_common_getSysCtx(ctx);
    NULL_RETURN_RET(sys_ctx, -1);
    NULL_RETURN_RET(sys_ctx->_rkAiqManager, -1);

    bool en;
    ret = AiqManager_getModuleCtl(sys_ctx->_rkAiqManager, mId, &en);
    *mod_en = en;
    EXIT_XCORE_FUNCTION();

    return ret;
}

XCamReturn
rk_aiq_uapi2_sysctl_enableAxlib(const rk_aiq_sys_ctx_t* ctx,
                                const int algo_type,
                                const int lib_id,
                                bool enable)
{
    RKAIQ_API_SMART_LOCK(ctx);
    if (ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* group_ctx =
            (const rk_aiq_camgroup_ctx_t*)ctx;

        return AiqCamGroupManager_enableAlgo(group_ctx->cam_group_manager, algo_type, lib_id, enable);
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    }
    else {
        return AiqCore_enableAlgo(ctx->_analyzer, algo_type, lib_id, enable);
    }
}

bool
rk_aiq_uapi2_sysctl_getAxlibStatus(const rk_aiq_sys_ctx_t* ctx,
                                   const int algo_type,
                                   const int lib_id)
{
    RKAIQ_API_SMART_LOCK(ctx);
    if (ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* group_ctx =
            (const rk_aiq_camgroup_ctx_t*)ctx;

        return AiqCamGroupManager_getAxlibStatus(group_ctx->cam_group_manager, algo_type, lib_id);
#else
        return false;
#endif
    }
    else
        return AiqCore_getAxlibStatus(ctx->_analyzer, algo_type, lib_id);
}

const RkAiqAlgoContext*
rk_aiq_uapi2_sysctl_getEnabledAxlibCtx(const rk_aiq_sys_ctx_t* ctx, const int algo_type)
{
    RKAIQ_API_SMART_LOCK(ctx);
    if (ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* group_ctx =
            (const rk_aiq_camgroup_ctx_t*)ctx;

        return AiqCamGroupManager_getEnabledAxlibCtx(group_ctx->cam_group_manager, algo_type);
#else
        return NULL;
#endif
    }
    else
        return AiqCore_getEnabledAxlibCtx(ctx->_analyzer, algo_type);
}

RkAiqAlgoContext*
rk_aiq_uapi2_sysctl_getAxlibCtx(const rk_aiq_sys_ctx_t* ctx, const int algo_type, const int lib_id)
{
    RKAIQ_API_SMART_LOCK(ctx);
    if (ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* group_ctx =
            (const rk_aiq_camgroup_ctx_t*)ctx;

        return AiqCamGroupManager_getAxlibCtx(group_ctx->cam_group_manager, algo_type, lib_id);
#else
        return NULL;
#endif
    }
    else
        return AiqCore_getAxlibCtx(ctx->_analyzer, algo_type, lib_id);
}


XCamReturn
rk_aiq_uapi2_sysctl_getStaticMetas(const char* sns_ent_name, rk_aiq_static_info_t* static_info)
{
    if (!sns_ent_name || !static_info)
        return XCAM_RETURN_ERROR_FAILED;

    if (!g_rk_aiq_init_lib) {
        rk_aiq_init_lib();
        g_rk_aiq_init_lib = true;
    }

    rk_aiq_static_info_t* s_info = AiqCamHw_getStaticCamHwInfo(sns_ent_name, 0);
    memcpy(static_info, s_info, sizeof(rk_aiq_static_info_t));

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi2_sysctl_setModuleCtl(const rk_aiq_sys_ctx_t* ctx, rk_aiq_module_id_t mId, bool mod_en)
{
    ENTER_XCORE_FUNCTION();
    CHECK_USER_API_ENABLE2(ctx);
    //CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_MODULES);
    RKAIQ_API_SMART_LOCK(ctx);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (mId > RK_MODULE_INVAL && mId < RK_MODULE_MAX) {
        if (ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
            rk_aiq_sys_ctx_array_t ctx_array = rk_aiq_user_api2_common_getSysCtxArray(ctx);
            for (int i = 0; i < ctx_array.num; i++) {
                ret = AiqManager_setModuleCtl(ctx_array.ctx[i]->_rkAiqManager, mId, mod_en);
            }
        }
    } else {
        ret = XCAM_RETURN_ERROR_FAILED;
    }

    EXIT_XCORE_FUNCTION();

    return ret;
}

XCamReturn
rk_aiq_uapi2_sysctl_getModuleEn(const rk_aiq_sys_ctx_t* ctx,
                                rk_aiq_module_list_t* mod) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    const rk_aiq_sys_ctx_t* sys_ctx = rk_aiq_user_api2_common_getSysCtx(ctx);
    GlobalParamsManager_t* paramsManager = &sys_ctx->_rkAiqManager->mGlobalParamsManager;
    ret = GlobalParamsManager_get_ModuleEn(paramsManager, mod);

    if (ret != XCAM_RETURN_NO_ERROR) {
        LOGE("get_ModuleEn failed!");
    }
    opMode_t mode;
    rk_aiq_uapi2_getExpMode(sys_ctx, &mode);
    mod->module_ctl[RESULT_TYPE_AEC_PARAM].en = mode == OP_AUTO ? 1 : 0;
    mod->module_ctl[RESULT_TYPE_AEC_PARAM].bypass = 0;
    mod->module_ctl[RESULT_TYPE_AEC_PARAM].opMode = mode == OP_AUTO ? RK_AIQ_OP_MODE_AUTO : RK_AIQ_OP_MODE_MANUAL;
    // rk_aiq_uapi2_getWBMode(sys_ctx, &mode);
    awb_gainCtrl_t attr;
    ret = rk_aiq_user_api2_awb_GetWbGainCtrlAttrib(ctx, &attr);
    mod->module_ctl[RESULT_TYPE_AWB_PARAM].en = attr.opMode == RK_AIQ_OP_MODE_AUTO ? 1 : 0;
    mod->module_ctl[RESULT_TYPE_AWB_PARAM].bypass = attr.byPass;
    mod->module_ctl[RESULT_TYPE_AWB_PARAM].opMode = attr.opMode;
#ifdef RKAIQ_HAVE_AF
    rk_aiq_uapi2_getFocusMode(sys_ctx, &mode);
    mod->module_ctl[RESULT_TYPE_AF_PARAM].en = mode == OP_AUTO ? 1 : 0;
    mod->module_ctl[RESULT_TYPE_AF_PARAM].bypass = 0;
    mod->module_ctl[RESULT_TYPE_AF_PARAM].opMode = mode == OP_AUTO ? RK_AIQ_OP_MODE_AUTO : RK_AIQ_OP_MODE_MANUAL;
#endif
#ifdef RKAIQ_HAVE_AIBNR
    aibnr_status_t status;
    rk_aiq_user_api2_aibnr_QueryStatus(sys_ctx, &status);
    mod->module_ctl[RESULT_TYPE_AIBNR_PARAM].en = status.stMan.sta.swOn_cfg.sw_aiBnrT_manualBnrHw_en;
#endif

    return ret;
}

XCamReturn
rk_aiq_uapi2_sysctl_setModuleEn(const rk_aiq_sys_ctx_t* ctx,
                                rk_aiq_module_list_t* mod) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_sys_ctx_array_t ctx_array = rk_aiq_user_api2_common_getSysCtxArray(ctx);
    for (int i = 0; i < ctx_array.num; i++) {
        GlobalParamsManager_t* paramsManager = &ctx_array.ctx[i]->_rkAiqManager->mGlobalParamsManager;
        ret = GlobalParamsManager_set_ModuleEn(paramsManager, mod);
    }

    if (ret != XCAM_RETURN_NO_ERROR) {
        LOGE("set_ModuleEn failed!");
    }

    for (int i = 0; i < RESULT_TYPE_MAX_PARAM; i++) {
        int cur_type = mod->module_ctl[i].type;
        if (cur_type == RESULT_TYPE_AEC_PARAM) {
            if (mod->module_ctl[i].opMode == RK_AIQ_OP_MODE_AUTO) {
                rk_aiq_uapi2_setExpMode(ctx, OP_AUTO);
            }
            else {
                rk_aiq_uapi2_setExpMode(ctx, OP_MANUAL);
            }
        }
        else if (cur_type == RESULT_TYPE_AWB_PARAM) {
            awb_gainCtrl_t attr;
            ret = rk_aiq_user_api2_awb_GetWbGainCtrlAttrib(ctx, &attr);
            attr.opMode = mod->module_ctl[i].opMode;
            attr.byPass = mod->module_ctl[i].bypass;
            ret = rk_aiq_user_api2_awb_SetWbGainCtrlAttrib(ctx, &attr);
        }
#ifdef RKAIQ_HAVE_AF
        else if (cur_type == RESULT_TYPE_AF_PARAM) {
            if (mod->module_ctl[i].opMode == RK_AIQ_OP_MODE_AUTO) {
                rk_aiq_uapi2_setFocusMode(ctx, OP_AUTO);
            }
            else {
                rk_aiq_uapi2_setFocusMode(ctx, OP_MANUAL);
            }
        }
#endif
        if (cur_type == 0)
            break;
    }

    return ret;
}

XCamReturn
rk_aiq_uapi2_sysctl_enumStaticMetasByPhyId(int index, rk_aiq_static_info_t* static_info)
{
    if (!static_info)
        return XCAM_RETURN_ERROR_FAILED;

    if (!g_rk_aiq_init_lib) {
        rk_aiq_init_lib();
        g_rk_aiq_init_lib = true;
    }

    rk_aiq_static_info_t* s_info = AiqCamHw_getStaticCamHwInfoByPhyId(NULL, index);

    if (s_info)
        memcpy(static_info, s_info, sizeof(rk_aiq_static_info_t));
    else
        return XCAM_RETURN_ERROR_OUTOFRANGE;
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi2_sysctl_enumStaticMetas(int index, rk_aiq_static_info_t* static_info)
{
    if (!static_info)
        return XCAM_RETURN_ERROR_FAILED;

    if (!g_rk_aiq_init_lib) {
        rk_aiq_init_lib();
        g_rk_aiq_init_lib = true;
    }

    rk_aiq_static_info_t* s_info = AiqCamHw_getStaticCamHwInfo(NULL, index);

    if (s_info)
        memcpy(static_info, s_info, sizeof(rk_aiq_static_info_t));
    else
        return XCAM_RETURN_ERROR_OUTOFRANGE;
    return XCAM_RETURN_NO_ERROR;
}

const char*
rk_aiq_uapi2_sysctl_getBindedSnsEntNmByVd(const char* vd)
{
    if (!g_rk_aiq_init_lib) {
        rk_aiq_init_lib();
        g_rk_aiq_init_lib = true;
    }

    return AiqCamHw_getBindedSnsEntNmByVd(vd);
}

XCamReturn
rk_aiq_uapi2_sysctl_getCrop(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_rect_t *rect)
{
    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
        LOGE("%s: not support for camgroup\n", __func__);
        return XCAM_RETURN_ERROR_FAILED;
    }

    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ret = AiqCamHw_getSensorCrop(sys_ctx->_camHw, rect);

    return ret;
}

XCamReturn
rk_aiq_uapi2_sysctl_setCrop(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_rect_t rect)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    rk_aiq_sys_ctx_array_t ctx_array = rk_aiq_user_api2_common_getSysCtxArray(sys_ctx);
    for (int i = 0; i < ctx_array.num; i++) {
        ret = AiqCamHw_setSensorCrop(ctx_array.ctx[i]->_camHw, &rect);
    }

    LOGK("cid[%d] %s: %dx%d(%d,%d)", sys_ctx->_camPhyId, __func__, rect.width, rect.height, rect.left, rect.top);
    return ret;
}

void
rk_aiq_uapi2_sysctl_setMulCamConc(const rk_aiq_sys_ctx_t* ctx, bool cc)
{
    ENTER_XCORE_FUNCTION();
    RKAIQ_API_SMART_LOCK(ctx);
#if defined(ISP_HW_V33) || defined(ISP_HW_V35)
    LOGK("cid[%d] %s: ignore !", ctx->_camPhyId, __func__, cc);
#else
    AiqManager_setMulCamConc(ctx->_rkAiqManager, cc);
    LOGK("cid[%d] %s: cc:%d", ctx->_camPhyId, __func__, cc);
#endif
    EXIT_XCORE_FUNCTION();
}

XCamReturn
rk_aiq_uapi2_sysctl_regMemsSensorIntf(const rk_aiq_sys_ctx_t* sys_ctx,
                                      const rk_aiq_mems_sensor_intf_t* intf)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    assert(sys_ctx != NULL);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
        LOGE("%s: not support for camgroup\n", __func__);
        return XCAM_RETURN_ERROR_FAILED;
    }

    ret = AiqCore_setMemsSensorIntf(sys_ctx->_analyzer, intf);
    if (ret) {
        LOGE("failed to update iqfile\n");
        ret = XCAM_RETURN_ERROR_FAILED;
    }

    return ret;
}

int
rk_aiq_uapi2_sysctl_switch_scene(const rk_aiq_sys_ctx_t* sys_ctx,
                                 const char* main_scene, const char* sub_scene)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (!sys_ctx) {
        LOGE("%s: sys_ctx is invalied\n", __func__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    if (!main_scene || !sub_scene) {
        LOGE("%s: request is invalied\n", __func__);
        return XCAM_RETURN_ERROR_PARAM;
    }

#ifdef RKAIQ_ENABLE_CAMGROUP
    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
        rk_aiq_camgroup_ctx_t* grp_ctx = (rk_aiq_camgroup_ctx_t*)(sys_ctx);
        if (!grp_ctx)
            return XCAM_RETURN_ERROR_FAILED;

        rk_aiq_sys_ctx_t* cam_ctx = NULL;
        for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
            cam_ctx = grp_ctx->cam_ctxs_array[i];
            if (cam_ctx) {
                CamCalibDbV2Context_t new_calib = RkAiqSceneManagerRefToScene(cam_ctx->_calibDbProj,
                                                  main_scene, sub_scene);

                if (!new_calib.calib_scene) {
                    LOGE("failed to find scene calib\n");
                    return -1;
                }

                GlobalParamsManager_t* paramsManager = &cam_ctx->_rkAiqManager->mGlobalParamsManager;
                GlobalParamsManager_switchCalibDb(paramsManager, &new_calib, true);
                ret = AiqManager_updateCalibDb(cam_ctx->_rkAiqManager, &new_calib);
                if (ret) {
                    LOGE("failed to switch scene\n");
                    return ret;
                }
            }
        }

        CamCalibDbV2Context_t new_calib = RkAiqSceneManagerRefToScene(grp_ctx->cam_ctxs_array[0]->_calibDbProj,
                                          main_scene, sub_scene);

        if (!new_calib.calib_scene) {
            LOGE("failed to find scene calib\n");
            return -1;
        }

        ret = AiqCamGroupManager_updateCalibDb(grp_ctx->cam_group_manager, &new_calib);
        if (ret) {
            LOGE("failed to switch scene\n");
            return ret;
        }
    }
#endif
    if (sys_ctx->cam_type != RK_AIQ_CAM_TYPE_GROUP) {
        CamCalibDbV2Context_t new_calib = RkAiqSceneManagerRefToScene(sys_ctx->_calibDbProj,
                                          main_scene, sub_scene);

        if (!new_calib.calib_scene) {
            LOGE("failed to find scene calib\n");
            return -1;
        }

        GlobalParamsManager_t* paramsManager = &sys_ctx->_rkAiqManager->mGlobalParamsManager;
        GlobalParamsManager_switchCalibDb(paramsManager, &new_calib, true);
        ret = AiqManager_updateCalibDb(sys_ctx->_rkAiqManager, &new_calib);
        if (ret) {
            LOGE("failed to switch scene\n");
            return ret;
        }
    }

    LOGK("cid[%d] %s: success. main:%s, sub:%s", sys_ctx->_camPhyId, __func__, main_scene, sub_scene);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi2_sysctl_tuning(const rk_aiq_sys_ctx_t* sys_ctx, char* param)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (!sys_ctx) {
        //LOGE("%s: sys_ctx is invalied\n", __func__);
        socket_client_setNote(sys_ctx->_socket, IPC_RET_AIQ_ERROR, "sysctl_tuning: sys_ctx is invalied\n");
        return XCAM_RETURN_ERROR_FAILED;
    }

    // Find json patch
    char* json_start = strchr(param, '[');
    char* json_end = strrchr(param, ']');

    LOGD_IPC("%s: enter, strlen %d", __func__, strlen(param));

    if (!json_start || !json_end ||
            (unsigned long)json_start >= (unsigned long)json_end) {
        //LOGE("%s: patch is invalied\n", __func__);
        socket_client_setNote(sys_ctx->_socket, IPC_RET_JSON_ERROR, "sysctl_tuning: patch is invalied\n");
        return XCAM_RETURN_ERROR_FAILED;
    }

    *(json_end + 1) = '\0';

    CamCalibDbV2Context_t* last_calib = AiqManager_getCurrentCalibDBV2(sys_ctx->_rkAiqManager);

    if (!last_calib) {
        *last_calib = CamCalibDbToDefaultCalibDb(sys_ctx->_calibDbProj);
        if (!last_calib) {
            //LOGE("%s: default calib is invalied\n", __func__);
            socket_client_setNote(sys_ctx->_socket, IPC_RET_AIQ_ERROR, "sysctl_tuning: default calib is invalied\n");
            return XCAM_RETURN_ERROR_FAILED;
        }
    }

    TuningCalib tuning_calib;
    CamCalibDbAnalyzTuningCalibFromStr(last_calib, json_start, &tuning_calib);
    if (!tuning_calib.calib) {
        //LOGE_ANALYZER("invalied tuning param\n");
        socket_client_setNote(sys_ctx->_socket, IPC_RET_AIQ_ERROR, "sysctl_tuning: CamCalibDbAnalyzTuningCalibFromStr failed\n");
        return XCAM_RETURN_ERROR_PARAM;
    }
#ifdef RKAIQ_ENABLE_CAMGROUP
    // api helper call with single instance
    if (get_binded_group_ctx(sys_ctx)) {
        int crt_cam_index = 0;
        rk_aiq_camgroup_ctx_t* grp_ctx = get_binded_group_ctx(sys_ctx);
        if (!grp_ctx)
            return XCAM_RETURN_ERROR_FAILED;

        ret = AiqCamGroupManager_calibTuning(grp_ctx->cam_group_manager, tuning_calib.calib);
        if (ret)
            LOGE("Faile to update the calib of camGroup\n", __func__);

        rk_aiq_sys_ctx_t* camCtx = NULL;
        for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
            camCtx = grp_ctx->cam_ctxs_array[i];
            if (camCtx) {
                // Avoid double free
                if (crt_cam_index == (grp_ctx->cam_ctxs_num - 1)) {
                    AiqManager_unsetTuningCalibDb(camCtx->_rkAiqManager, true);
                }
                else {
                    AiqManager_unsetTuningCalibDb(camCtx->_rkAiqManager, false);
                }
                ret = AiqManager_calibTuning(camCtx->_rkAiqManager, tuning_calib.calib,
                                             &tuning_calib);
                crt_cam_index++;
            }
        }
    } else {
        ret = AiqManager_calibTuning(sys_ctx->_rkAiqManager, tuning_calib.calib,
                                     &tuning_calib);
    }
#else
    ret = AiqManager_calibTuning(sys_ctx->_rkAiqManager, tuning_calib.calib,
                                 &tuning_calib);

#endif

    socket_client_setNote(sys_ctx->_socket, IPC_RET_OK, "sysctl_tuning: write calib sucess\n");
    return ret;
}

char* rk_aiq_uapi2_sysctl_readiq(const rk_aiq_sys_ctx_t* sys_ctx, char* param)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    cJSON* cmd_json = NULL;
    char* ret_str = NULL;

    if (!sys_ctx) {
        //LOGE("%s: sys_ctx is invalied\n", __func__);
        socket_client_setNote(sys_ctx->_socket, IPC_RET_AIQ_ERROR, "readiq: sys_ctx is invalied\n");
        return NULL;
    }

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
        //LOGE("%s: not support for camgroup\n", __func__);
        socket_client_setNote(sys_ctx->_socket, IPC_RET_AIQ_ERROR, "readiq: not support for camgroup\n");
        return NULL;
    }

    // Find json patch
    char* json_start = strchr(param, '[');
    char* json_end = strchr(param, ']');

    LOGI("request is:%s\n", param);

    if (!json_start || !json_end ||
            (unsigned long)json_start >= (unsigned long)json_end) {
        //LOGE("%s: request is invalied\n", __func__);
        socket_client_setNote(sys_ctx->_socket, IPC_RET_JSON_ERROR, "readiq: request is invalied\n");
        return NULL;
    }

    *(json_end + 1) = '\0';

    CamCalibDbV2Context_t* last_calib = AiqManager_getCurrentCalibDBV2(sys_ctx->_rkAiqManager);

    if (!last_calib) {
        *last_calib = CamCalibDbToDefaultCalibDb(sys_ctx->_calibDbProj);
        if (!last_calib) {
            LOGE("%s: default calib is invalied\n", __func__);
            return NULL;
        }
    }

    ret_str = CamCalibDbReadIQNodeStrFromJstr(last_calib, json_start);

    socket_client_setNote(sys_ctx->_socket, IPC_RET_OK, "readiq: read calib success\n");
    return ret_str;
}

#if defined(ISP_HW_V33)
static XCamReturn _get_fast_aewb_from_drv(const char* sensor_name, struct rkisp33_thunderboot_resmem_head* fastAeAwbInfo)
#elif defined(ISP_HW_V32)
static XCamReturn _get_fast_aewb_from_drv(const char* sensor_name, struct rkisp32_thunderboot_resmem_head* fastAeAwbInfo)
#else
static XCamReturn _get_fast_aewb_from_drv(const char* sensor_name, struct rkisp_thunderboot_resmem_head* fastAeAwbInfo)
#endif
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_sensor_full_info_t* s_info = AiqCamHw_getFullSnsInfo(sensor_name);
    if (!s_info) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "can't find sensor %s", sensor_name);
        return XCAM_RETURN_ERROR_SENSOR;
    }

    AiqV4l2SubDevice_t IspCoreDev = {0};

    ret = AiqV4l2SubDevice_init(&IspCoreDev, s_info->isp_info->isp_dev_path);
    if (!ret) {
        IspCoreDev._v4l_base.open(&IspCoreDev._v4l_base, false);
#if defined(ISP_HW_V33)
        if (IspCoreDev._v4l_base.io_control(&IspCoreDev._v4l_base, RKISP_CMD_GET_TB_HEAD_V33, fastAeAwbInfo) < 0)
#elif defined(ISP_HW_V32)
        if (IspCoreDev._v4l_base.io_control(&IspCoreDev._v4l_base, RKISP_CMD_GET_TB_HEAD_V32, fastAeAwbInfo) < 0)
#else
        if (IspCoreDev._v4l_base.io_control(&IspCoreDev._v4l_base, RKISP_CMD_GET_TB_HEAD, fastAeAwbInfo) < 0)
#endif
            ret = XCAM_RETURN_ERROR_FAILED;
        IspCoreDev._v4l_base.close(&IspCoreDev._v4l_base);
        AiqV4l2SubDevice_deinit(&IspCoreDev);
    }

    return ret;
}

static void _set_fast_aewb_as_init(const rk_aiq_sys_ctx_t* ctx, rk_aiq_working_mode_t mode)
{
    struct rkisp_thunderboot_resmem_head *head = NULL;
#if defined(ISP_HW_V33)
    struct rkisp33_thunderboot_resmem_head fastAeAwbInfo;
    head = &fastAeAwbInfo.head;
#elif defined(ISP_HW_V32)
    struct rkisp32_thunderboot_resmem_head fastAeAwbInfo;
    head = &fastAeAwbInfo.head;
#else
    struct rkisp_thunderboot_resmem_head fastAeAwbInfo;
    head = &fastAeAwbInfo;
#endif
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    rk_aiq_tb_info_t info;
    info.is_fastboot = false;
    ret = _get_fast_aewb_from_drv(ctx->_sensor_entity_name, &fastAeAwbInfo);
    if (ret == XCAM_RETURN_NO_ERROR) {
        // // set initial wb
        // rk_aiq_uapiV2_awb_ffwbgain_attr_t attr;

        // attr.sync.sync_mode = RK_AIQ_UAPI_MODE_DEFAULT;
        // attr.sync.done = false;

        // // TODO: check last expmode(Linear/Hdr),use hdr_mode in head

        // if (mode == RK_AIQ_WORKING_MODE_NORMAL) {
        //     float minGain = (8 << 8);
        //     minGain = (minGain < fastAeAwbInfo.cfg.others.awb_gain_cfg.awb1_gain_r) ? minGain : fastAeAwbInfo.cfg.others.awb_gain_cfg.awb1_gain_r;
        //     minGain = (minGain < fastAeAwbInfo.cfg.others.awb_gain_cfg.awb1_gain_gr) ? minGain : fastAeAwbInfo.cfg.others.awb_gain_cfg.awb1_gain_gr;
        //     minGain = (minGain < fastAeAwbInfo.cfg.others.awb_gain_cfg.awb1_gain_gb) ? minGain : fastAeAwbInfo.cfg.others.awb_gain_cfg.awb1_gain_gb;
        //     minGain = (minGain < fastAeAwbInfo.cfg.others.awb_gain_cfg.awb1_gain_b) ? minGain : fastAeAwbInfo.cfg.others.awb_gain_cfg.awb1_gain_b;
        //     attr.wggain.rgain = (float)fastAeAwbInfo.cfg.others.awb_gain_cfg.awb1_gain_r / minGain;
        //     attr.wggain.grgain = (float)fastAeAwbInfo.cfg.others.awb_gain_cfg.awb1_gain_gr / minGain;
        //     attr.wggain.gbgain = (float)fastAeAwbInfo.cfg.others.awb_gain_cfg.awb1_gain_gb / minGain;
        //     attr.wggain.bgain = (float)fastAeAwbInfo.cfg.others.awb_gain_cfg.awb1_gain_b / minGain;
        // }
        // else {
        //     float minGain = (8 << 8);
        //     minGain = (minGain < fastAeAwbInfo.cfg.others.awb_gain_cfg.gain0_red) ? minGain : fastAeAwbInfo.cfg.others.awb_gain_cfg.gain0_red;
        //     minGain = (minGain < fastAeAwbInfo.cfg.others.awb_gain_cfg.gain0_green_r) ? minGain : fastAeAwbInfo.cfg.others.awb_gain_cfg.gain0_green_r;
        //     minGain = (minGain < fastAeAwbInfo.cfg.others.awb_gain_cfg.gain0_green_b) ? minGain : fastAeAwbInfo.cfg.others.awb_gain_cfg.gain0_green_b;
        //     minGain = (minGain < fastAeAwbInfo.cfg.others.awb_gain_cfg.gain0_blue) ? minGain : fastAeAwbInfo.cfg.others.awb_gain_cfg.gain0_blue;
        //     attr.wggain.rgain = (float)fastAeAwbInfo.cfg.others.awb_gain_cfg.gain0_red / minGain;
        //     attr.wggain.grgain = (float)fastAeAwbInfo.cfg.others.awb_gain_cfg.gain0_green_r / minGain;
        //     attr.wggain.gbgain = (float)fastAeAwbInfo.cfg.others.awb_gain_cfg.gain0_green_b / minGain;
        //     attr.wggain.bgain = (float)fastAeAwbInfo.cfg.others.awb_gain_cfg.gain0_blue / minGain;
        // }

        // rk_aiq_user_api2_awb_SetFFWbgainAttrib(ctx, attr);

        // LOGK("cid[%d] %s: ffwb:%f,%f,%f,%f", ctx->_camPhyId, __func__,
        //     attr.wggain.rgain, attr.wggain.grgain, attr.wggain.gbgain, attr.wggain.bgain);

#ifndef USE_NEWSTRUCT
        // set initial exposure
        if (mode == RK_AIQ_WORKING_MODE_NORMAL) {
            Uapi_LinExpAttrV2_t LinExpAttr;
            ret = rk_aiq_user_api2_ae_getLinExpAttr(ctx, &LinExpAttr);

            LinExpAttr.sync.sync_mode = RK_AIQ_UAPI_MODE_DEFAULT;
            LinExpAttr.sync.done = false;
            LinExpAttr.Params.InitExp.InitTimeValue = (float)head->exp_time[0] / (1 << 16);
            LinExpAttr.Params.InitExp.InitGainValue = (float)head->exp_gain[0] / (1 << 16);
            LinExpAttr.Params.InitExp.InitIspDGainValue = (float)head->exp_isp_dgain[0] / (1 << 16);

            ret = rk_aiq_user_api2_ae_setLinExpAttr(ctx, LinExpAttr);

            LOGK("cid[%d] %s: ffexp:%f,%f,%f", ctx->_camPhyId, __func__,
                 LinExpAttr.Params.InitExp.InitTimeValue, LinExpAttr.Params.InitExp.InitGainValue, LinExpAttr.Params.InitExp.InitIspDGainValue);
        }
        else {
            Uapi_HdrExpAttrV2_t HdrExpAttr;
            ret = rk_aiq_user_api2_ae_getHdrExpAttr(ctx, &HdrExpAttr);

            HdrExpAttr.sync.sync_mode = RK_AIQ_UAPI_MODE_DEFAULT;
            HdrExpAttr.sync.done = false;
            HdrExpAttr.Params.InitExp.InitTimeValue[0] = (float)head->exp_time[0] / (1 << 16);
            HdrExpAttr.Params.InitExp.InitGainValue[0] = (float)head->exp_gain[0] / (1 << 16);
            HdrExpAttr.Params.InitExp.InitIspDGainValue[0] = (float)head->exp_isp_dgain[0] / (1 << 16);
            HdrExpAttr.Params.InitExp.InitTimeValue[1] = (float)head->exp_time[1] / (1 << 16);
            HdrExpAttr.Params.InitExp.InitGainValue[1] = (float)head->exp_gain[1] / (1 << 16);
            HdrExpAttr.Params.InitExp.InitIspDGainValue[1] = (float)head->exp_isp_dgain[1] / (1 << 16);
            HdrExpAttr.Params.InitExp.InitTimeValue[2] = (float)head->exp_time[2] / (1 << 16);
            HdrExpAttr.Params.InitExp.InitGainValue[2] = (float)head->exp_gain[2] / (1 << 16);
            HdrExpAttr.Params.InitExp.InitIspDGainValue[2] = (float)head->exp_isp_dgain[2] / (1 << 16);

            ret = rk_aiq_user_api2_ae_setHdrExpAttr(ctx, HdrExpAttr);

            LOGK("cid[%d] %s: hdr ffexp:%f,%f,%f,%f,%f,%f,%f,%f,%f", ctx->_camPhyId, __func__,
                 HdrExpAttr.Params.InitExp.InitTimeValue[0], HdrExpAttr.Params.InitExp.InitGainValue[0], HdrExpAttr.Params.InitExp.InitIspDGainValue[0],
                 HdrExpAttr.Params.InitExp.InitTimeValue[1], HdrExpAttr.Params.InitExp.InitGainValue[1], HdrExpAttr.Params.InitExp.InitIspDGainValue[1],
                 HdrExpAttr.Params.InitExp.InitTimeValue[2], HdrExpAttr.Params.InitExp.InitGainValue[2], HdrExpAttr.Params.InitExp.InitIspDGainValue[2]);
        }
#else
        // set initial exposure
        if (mode == RK_AIQ_WORKING_MODE_NORMAL) {
            ae_api_linExpAttr_t LinExpAttr;
            ret = rk_aiq_user_api2_ae_getLinExpAttr(ctx, &LinExpAttr);

            LinExpAttr.initExp.sw_aeT_initTime_val = (float)head->exp_time[0] / (1 << 16);
            LinExpAttr.initExp.sw_aeT_initGain_val = (float)head->exp_gain[0] / (1 << 16);
            LinExpAttr.initExp.sw_aeT_initIspDGain_val = (float)head->exp_isp_dgain[0] / (1 << 16);

            ret = rk_aiq_user_api2_ae_setLinExpAttr(ctx, LinExpAttr);

            LOGK("cid[%d] %s: ffexp:%f,%f,%f", ctx->_camPhyId, __func__,
                 LinExpAttr.initExp.sw_aeT_initTime_val, LinExpAttr.initExp.sw_aeT_initGain_val, LinExpAttr.initExp.sw_aeT_initIspDGain_val);
        } else {
            ae_api_hdrExpAttr_t HdrExpAttr;
            ret = rk_aiq_user_api2_ae_getHdrExpAttr(ctx, &HdrExpAttr);

            HdrExpAttr.initExp.sw_aeT_initTime_val[0] = (float)head->exp_time[0] / (1 << 16);
            HdrExpAttr.initExp.sw_aeT_initGain_val[0] = (float)head->exp_gain[0] / (1 << 16);
            HdrExpAttr.initExp.sw_aeT_initIspDGain_val[0] = (float)head->exp_isp_dgain[0] / (1 << 16);
            HdrExpAttr.initExp.sw_aeT_initTime_val[1] = (float)head->exp_time[1] / (1 << 16);
            HdrExpAttr.initExp.sw_aeT_initGain_val[1] = (float)head->exp_gain[1] / (1 << 16);
            HdrExpAttr.initExp.sw_aeT_initIspDGain_val[1] = (float)head->exp_isp_dgain[1] / (1 << 16);
            HdrExpAttr.initExp.sw_aeT_initTime_val[2] = (float)head->exp_time[2] / (1 << 16);
            HdrExpAttr.initExp.sw_aeT_initGain_val[2] = (float)head->exp_gain[2] / (1 << 16);
            HdrExpAttr.initExp.sw_aeT_initIspDGain_val[2] = (float)head->exp_isp_dgain[2] / (1 << 16);

            ret = rk_aiq_user_api2_ae_setHdrExpAttr(ctx, HdrExpAttr);

            LOGK("cid[%d] %s: hdr ffexp:%f,%f,%f,%f,%f,%f,%f,%f,%f", ctx->_camPhyId, __func__,
                 HdrExpAttr.initExp.sw_aeT_initTime_val[0], HdrExpAttr.initExp.sw_aeT_initGain_val[0], HdrExpAttr.initExp.sw_aeT_initIspDGain_val[0],
                 HdrExpAttr.initExp.sw_aeT_initTime_val[1], HdrExpAttr.initExp.sw_aeT_initGain_val[1], HdrExpAttr.initExp.sw_aeT_initIspDGain_val[1],
                 HdrExpAttr.initExp.sw_aeT_initTime_val[2], HdrExpAttr.initExp.sw_aeT_initGain_val[2], HdrExpAttr.initExp.sw_aeT_initIspDGain_val[2]);
        }

#endif

        info.is_fastboot = true;
        LOGK("<TB>: get fast aeawb info, run fastboot mode");
    }

    // notify aiq run fastboot mode
    AiqManager_setTbInfo(ctx->_rkAiqManager, &info);
    AiqCamHw_setTbInfo(ctx->_camHw, &info);
    AiqCore_setTbInfo(ctx->_analyzer, &info);
}

void rk_aiq_uapi2_sysctl_setIspParamsDelayCnts(const rk_aiq_sys_ctx_t* sys_ctx, int8_t delay_cnts) {
    rk_aiq_sys_ctx_array_t ctx_array = rk_aiq_user_api2_common_getSysCtxArray(sys_ctx);
    for (int i = 0; i < ctx_array.num; i++) {
        AiqCore_setDelayCnts(ctx_array.ctx[i]->_analyzer, delay_cnts);
    }
}

XCamReturn rk_aiq_uapi2_sysctl_pause(rk_aiq_sys_ctx_t* sys_ctx, bool isSingleMode)
{
    if (!sys_ctx) {
        LOGE("%s: sys_ctx is invalied\n", __func__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    return AiqManager_setVicapStreamMode(sys_ctx->_rkAiqManager, 0, isSingleMode);
}

XCamReturn rk_aiq_uapi2_sysctl_resume(rk_aiq_sys_ctx_t* sys_ctx)
{
    if (!sys_ctx) {
        LOGE("%s: sys_ctx is invalied\n", __func__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    return AiqManager_setVicapStreamMode(sys_ctx->_rkAiqManager, 1, false);
}

#if defined(ISP_HW_V33) || defined(ISP_HW_V35)
XCamReturn
rk_aiq_user_api2_postisp_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, postisp_api_attrib_t* attr);
#endif

XCamReturn
rk_aiq_uapi2_sysctl_getAinrParams(const rk_aiq_sys_ctx_t* sys_ctx, rk_ainr_param* para)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (!sys_ctx) {
        LOGE("%s: sys_ctx is invalied\n", __func__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    Uapi_ExpQueryInfo_t pExpResInfo;
    rk_aiq_working_mode_t mode;
    float dynamicAiBypass = 0;

    ae_api_queryInfo_t queryInfo;
    rk_aiq_user_api2_ae_queryExpResInfo(sys_ctx, &queryInfo);
    rk_aiq_uapi2_sysctl_getWorkingMode(sys_ctx, &mode);

    if (mode == RK_AIQ_WORKING_MODE_NORMAL) {
        para->gain = queryInfo.linExpInfo.expParam.analog_gain * queryInfo.linExpInfo.expParam.isp_dgain;
        para->RawMeanluma = queryInfo.linExpInfo.meanLuma;
    } else if (mode == RK_AIQ_WORKING_MODE_ISP_HDR2) {
        para->gain = queryInfo.hdrExpInfo.expParam[0].analog_gain * queryInfo.hdrExpInfo.expParam[0].isp_dgain;
        para->RawMeanluma = queryInfo.hdrExpInfo.frm0Luma;
    } else {
        para->gain = queryInfo.hdrExpInfo.expParam[1].analog_gain * queryInfo.hdrExpInfo.expParam[1].isp_dgain;
        para->RawMeanluma = queryInfo.hdrExpInfo.frm1Luma;
    }

#if defined(ISP_HW_V33) || defined(ISP_HW_V35)
    postisp_api_attrib_t postisp_attrib = {0};
    ret = rk_aiq_user_api2_postisp_GetAttrib(sys_ctx, &postisp_attrib);
    if (ret != XCAM_RETURN_NO_ERROR)
        return ret;

    postisp_param_auto_t *paut = &postisp_attrib.stAuto;
    postisp_params_static_t *psta = &paut->sta;
    postisp_params_dyn_t *pdyn = paut->dyn;

    para->mode = psta->data.mode;
    para->gain_tab_len = psta->data.gain_tab_len;
    para->gain_max = psta->data.gain_max;
    para->tuning_visual_flag = psta->data.tuning_visual_flag;

    for (int i = 0; i < RK_AINR_LUMA_LEN; i++) {
        para->luma_curve_tab[i] = psta->data.luma_point[i];
    }

    for (int i = 0; i < 13; i++) {
        para->gain_tab[i] = (float)(1 << i);
        para->sigma_tab[i] = pdyn[i].data.sigma;
        para->shade_tab[i] = pdyn[i].data.shade;
        para->sharp_tab[i] = pdyn[i].data.sharp;
        para->min_luma_tab[i] = pdyn[i].data.min_luma;
        para->sat_scale_tab[i] = pdyn[i].data.sat_scale;
        para->dark_contrast_tab[i] = pdyn[i].data.dark_contrast;
        para->ai_ratio_tab[i] = pdyn[i].data.ai_ratio;
        para->mot_thresh_tab[i] = pdyn[i].data.mot_thresh;
        para->static_thresh_tab[i] = pdyn[i].data.static_thresh;
        para->mot_nr_stren_tab [i] = pdyn[i].data.mot_nr_stren;
        for (int j = 0; j < RK_AINR_LUMA_LEN; j++) {
            para->sigma_curve_tab[j][i] = pdyn[i].data.luma_sigma[j];
        }
    }

    if (para->gain > psta->data.dynamicSw[1])
        dynamicAiBypass =  psta->data.dynamicSw[0];
    else if (para->gain <  psta->data.dynamicSw[0])
        dynamicAiBypass =  psta->data.dynamicSw[1];
    else if (sys_ctx->_rkAiqManager->ainr_status)
        dynamicAiBypass =  psta->data.dynamicSw[0];
    else
        dynamicAiBypass =  psta->data.dynamicSw[1];

    if (para->gain > dynamicAiBypass) {
        para->enable = true;
        //printf("AINR on\n");
    } else if (para->gain < dynamicAiBypass) {
        para->enable = false;
        //printf("AINR off\n");
    }

    //printf("getAinrParams test for ainr params set: gain_tab [%f %f %f]\n",
    //        para->gain_tab[0], para->gain_tab[1], para->gain_tab[2]);

    para->enable &= postisp_attrib.en;
    sys_ctx->_rkAiqManager->ainr_status = para->enable;
#endif

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn rk_aiq_uapi2_sysctl_setUserOtpInfo(rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_user_otp_info_t otp_info)
{
    if (!sys_ctx)
        return XCAM_RETURN_ERROR_PARAM;

    if (sys_ctx->cam_type != RK_AIQ_CAM_TYPE_SINGLE) {
        LOGE("cam_type: %d, only supported in single camera mode", sys_ctx->cam_type);
    }

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ret = AiqCore_setUserOtpInfo(sys_ctx->_analyzer, otp_info);
    if (ret != XCAM_RETURN_NO_ERROR) {
        return ret;
    }
    ret = AiqCamHw_setUserOtpInfo(sys_ctx->_camHw, &otp_info);
    return ret;
}

void
rk_aiq_uapi2_sysctl_setListenStrmStatus(rk_aiq_sys_ctx_t* sys_ctx, bool isListen)
{
    rk_aiq_sys_ctx_array_t ctx_array = rk_aiq_user_api2_common_getSysCtxArray(sys_ctx);
    for (int i = 0; i < ctx_array.num; i++) {
        AiqCamHw_setListenStrmEvt(ctx_array.ctx[i]->_camHw, isListen);
    }
}

static XCamReturn rk_aiq_aiisp_defaut_cb(rk_aiq_aiisp_t* aiisp_evt, void* ctx) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    rk_aiq_sys_ctx_t* aiisp_ctx = (rk_aiq_sys_ctx_t *)ctx;
    aiisp_ctx->_wr_linecnt_now += aiisp_evt->height;
    if (true || aiisp_ctx->_wr_linecnt_now >= aiisp_ctx->_camHw->aiisp_param->rawHgt) {
        aiisp_ctx->_wr_linecnt_now = 0;
        aiisp_ctx->_camHw->aiisp_processing(aiisp_ctx->_camHw, aiisp_evt);
        ret = rk_aiq_uapi2_sysctl_ReadAiisp(aiisp_ctx, aiisp_evt);
    }
    return ret;
}

XCamReturn rk_aiq_uapi2_sysctl_initAiisp(rk_aiq_sys_ctx_t* sys_ctx, struct rkisp_aiisp_cfg* aiisp_cfg,
        rk_aiq_aiisp_cb aiisp_cb)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#ifdef ISP_HW_V39
    if (!sys_ctx)
        return XCAM_RETURN_ERROR_PARAM;

    if (aiisp_cfg == NULL) {
        struct rkisp_aiisp_cfg aiisp_cfg_tmp;
        rk_aiq_exposure_sensor_descriptor sensor_des;
        AiqCamHw_getSensorModeData(sys_ctx->_camHw, sys_ctx->_sensor_entity_name, &sensor_des);
        uint32_t height = sensor_des.isp_acq_height;
        aiisp_cfg_tmp.mode = 1;
        aiisp_cfg_tmp.wr_linecnt = height;
        aiisp_cfg_tmp.rd_linecnt = height;
        ret                      = sys_ctx->_camHw->setAiispMode(sys_ctx->_camHw, &aiisp_cfg_tmp);
    }
    else {
        ret = sys_ctx->_camHw->setAiispMode(sys_ctx->_camHw, aiisp_cfg);
    }
    if (ret != XCAM_RETURN_NO_ERROR) {
        LOGE("Set Aiisp mode failed!");
        return ret;
    }

    if (aiisp_cb == NULL) {
        aiisp_cb = rk_aiq_aiisp_defaut_cb;
    }
    rk_aiq_aiispCtx_t aiispCtx;
    aiispCtx.mAiispEvtcb = aiisp_cb;
    aiispCtx.ctx = sys_ctx;

    AiqManager_setAiispCb(sys_ctx->_rkAiqManager, aiispCtx);
    sys_ctx->_use_aiisp = true;

    // RkAiqAblcV32HandleInt* algo_handle =
    //     algoHandle<RkAiqAblcV32HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_ABLC);
    // if (algo_handle)
    //     ret = algo_handle->setAiisp();
    rk_aiq_blc_attrib_V32_t blc_attr;
    memset(&blc_attr, 0x00, sizeof(blc_attr));
    //ret = rk_aiq_user_api2_ablcV32_GetAttrib(sys_ctx, &blc_attr);
    blc_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
    AblcOPMode_V32_t eMode_tmp = blc_attr.eMode;
    blc_attr.eMode = ABLC_V32_OP_MODE_MANUAL;
    blc_attr.stBlcOBManual.enable = 0;
    //ret = rk_aiq_user_api2_ablcV32_SetAttrib(sys_ctx, &blc_attr);

    //ret = rk_aiq_user_api2_ablcV32_GetAttrib(sys_ctx, &blc_attr);
    blc_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
    blc_attr.eMode = ABLC_V32_OP_MODE_AUTO;
    blc_attr.stBlcOBAuto.enable = 0;
    //ret = rk_aiq_user_api2_ablcV32_SetAttrib(sys_ctx, &blc_attr);
    blc_attr.eMode = eMode_tmp;
    //ret = rk_aiq_user_api2_ablcV32_SetAttrib(sys_ctx, &blc_attr);

    LOGK("AIISP on");
#else
    LOGE("The current platform does not support");
#endif
    return ret;
}

XCamReturn rk_aiq_uapi2_sysctl_ReadAiisp(rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_aiisp_t* aiisp_evt)
{
    if (!sys_ctx)
        return XCAM_RETURN_ERROR_PARAM;
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ret            = sys_ctx->_camHw->read_aiisp_result(sys_ctx->_camHw, aiisp_evt);
    LOGD("start to read AIISP result");
    return ret;
}

XCamReturn
rk_aiq_uapi2_sysctl_get3AStatsBlk(const rk_aiq_sys_ctx_t* ctx,
                                  rk_aiq_isp_stats_t **stats, int timeout_ms)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    rk_aiq_isp_stats_t* pstats = (rk_aiq_isp_stats_t*)aiq_mallocz(sizeof(rk_aiq_isp_stats_t));
    rk_aiq_isp_statistics_t sStats;
    if (pstats)
        ret = AiqCore_get3AStats(ctx->_analyzer, &sStats, timeout_ms);

    if (ret == XCAM_RETURN_ERROR_TIMEOUT) {
        aiq_free(pstats);
    } else {
        *stats = pstats;
        pstats->frame_id = sStats.frame_id;
        pstats->aec_stats_v25 = sStats.aec_stats;
        pstats->bValid_aec_stats = sStats.bValid_aec_stats;
        pstats->awb_stats_v39 = sStats.awb_stats;
        pstats->bValid_awb_stats = sStats.bValid_awb_stats;
#if RKAIQ_HAVE_AF_V33 || RKAIQ_ONLY_AF_STATS_V33
        pstats->afStats_stats = sStats.afStats_stats;
        pstats->bValid_af_stats = sStats.bValid_af_stats;
#endif
    }

    return ret;
}

XCamReturn
rk_aiq_uapi2_sysctl_get3AStats(const rk_aiq_sys_ctx_t* ctx,
                               rk_aiq_isp_stats_t *stats)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    rk_aiq_isp_statistics_t sStats;
    ret = AiqCore_get3AStats(ctx->_analyzer, &sStats, 0);
    if (ret == XCAM_RETURN_NO_ERROR) {
        stats->frame_id = sStats.frame_id;
        stats->aec_stats_v25 = sStats.aec_stats;
        stats->bValid_aec_stats = sStats.bValid_aec_stats;
        stats->awb_stats_v39 = sStats.awb_stats;
        stats->bValid_awb_stats = sStats.bValid_awb_stats;
#if RKAIQ_HAVE_AF_V33 || RKAIQ_ONLY_AF_STATS_V33
        stats->afStats_stats = sStats.afStats_stats;
        stats->bValid_af_stats = sStats.bValid_af_stats;
#endif
    }

    return ret;
}

void
rk_aiq_uapi2_sysctl_release3AStatsRef(const rk_aiq_sys_ctx_t* ctx,
                                      rk_aiq_isp_stats_t *stats)
{
    aiq_free(stats);
}

void rk_aiq_uapi2_get_aiqversion_info(const rk_aiq_sys_ctx_t* ctx, rk_aiq_version_info_t* vers)
{
    const char* ver_str = RK_AIQ_VERSION;
    const char* start = ver_str + strlen(RK_AIQ_VERSION_HEAD);
    strcpy(vers->aiq_ver, start);
}

XCamReturn
rk_aiq_uapi2_sysctl_setReadBackMode(rk_aiq_sys_ctx_t* ctx, bool on)
{
    ENTER_XCORE_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ret = AiqCamHw_setNoReadbackMode(ctx->_camHw, on);
    EXIT_XCORE_FUNCTION();

    return ret;
}

XCamReturn
rk_aiq_uapi2_setRawBufNum(rk_aiq_sys_ctx_t* ctx, uint16_t buf_num)
{
    ENTER_XCORE_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ret = AiqCamHw_setRawBufNum(ctx->_camHw, buf_num);
    EXIT_XCORE_FUNCTION();

    return ret;
}

void rk_aiq_uapi2_setImuData(const rk_aiq_sys_ctx_t* ctx, AiqImuData_t *data)
{
    ENTER_XCORE_FUNCTION();
    AiqManager_pushImuData(ctx->_rkAiqManager, data);
    EXIT_XCORE_FUNCTION();
}

XCamReturn rk_aiq_uapi2_setSingleRawBufAsyncMode(const rk_aiq_sys_ctx_t* ctx, bool async)
{
    ENTER_XCORE_FUNCTION();
    if (!ctx && !ctx->_camHw) {
        LOGE("%s: ctx %p or ctx->_camHw %p is invalied\n", __func__, ctx, ctx->_camHw);
        return XCAM_RETURN_ERROR_PARAM;
    }

    return AiqCamHw_setSingleRawBufAsyncMode(ctx->_camHw, async);

    EXIT_XCORE_FUNCTION();
}

bool rk_aiq_uapi2_judgeNeedRawPipeCtrl(const rk_aiq_sys_ctx_t* ctx)
{
    ENTER_XCORE_FUNCTION();
    if (!ctx && !ctx->_camHw) {
        LOGE("%s: ctx %p or ctx->_camHw %p is invalied\n", __func__, ctx, ctx->_camHw);
        return false;
    }

    return AiqCamHw_getAiqReadbackOn(ctx->_camHw);
}

#ifdef USE_IMPLEMENT_C
void rk_aiq_uapi2_get_version_info(rk_aiq_ver_info_t* vers)
{
    uint32_t iq_parser_magic_code;

    xcam_mem_clear(*vers);
    const char* ver_str = RK_AIQ_CALIB_VERSION;
    const char* start = ver_str + strlen(RK_AIQ_CALIB_VERSION_HEAD);
    const char* stop = strstr(ver_str, RK_AIQ_CALIB_VERSION_MAGIC_JOINT);

    // TODO: use strncpy instead of memcpy, but has compile warning now
    memcpy(vers->iq_parser_ver, start, stop - start);

    start = strstr(ver_str, RK_AIQ_CALIB_VERSION_MAGIC_CODE_HEAD) +
            strlen(RK_AIQ_CALIB_VERSION_MAGIC_CODE_HEAD);

    vers->iq_parser_magic_code = atoi(start);

    ver_str = RK_AIQ_VERSION;
    start = ver_str + strlen(RK_AIQ_VERSION_HEAD);
    strcpy(vers->aiq_ver, start);

    strcpy(vers->awb_algo_ver, g_RkIspAlgoDescAwb.common.version);
    strcpy(vers->ae_algo_ver, g_RkIspAlgoDescAe.common.version);
#if RKAIQ_HAVE_AF
    strcpy(vers->af_algo_ver, g_RkIspAlgoDescAf.common.version);
#endif
#if RKAIQ_HAVE_MERGE_V10 || RKAIQ_HAVE_MERGE_V11 || RKAIQ_HAVE_MERGE_V12
#ifndef USE_NEWSTRUCT
    strcpy(vers->ahdr_algo_ver, g_RkIspAlgoDescAmerge.common.version);
#else
    strcpy(vers->ahdr_algo_ver, g_RkIspAlgoDescMerge.common.version);
#endif
#endif
#if RKAIQ_HAVE_TMO_V1
    strcpy(vers->ahdr_algo_ver, g_RkIspAlgoDescAtmo.common.version);
#endif


    LOGI("aiq ver %s, parser ver %s, magic code %d, awb ver %s\n"
         "ae ver %s, af ver %s, ahdr ver %s", vers->aiq_ver,
         vers->iq_parser_ver, vers->iq_parser_magic_code,
         vers->awb_algo_ver, vers->ae_algo_ver,
         vers->af_algo_ver, vers->ahdr_algo_ver);
}

bool is_ctx_need_bypass(const rk_aiq_sys_ctx_t* ctx)
{
    if (!ctx) {
        return true;
    }

    if (!g_bypass_uapi) {
        return false;
    }

    const rk_aiq_sys_ctx_t* sys_ctx = rk_aiq_user_api2_common_getSysCtx(ctx);

    if (sys_ctx->_socket) {
        if (sys_ctx->_socket->connected && sys_ctx->ctx_type != CTX_TYPE_TOOL_SERVER) {
            LOG1("bypass the uapi which isn't called by iqtool while socket is connected");
            return true;
        }
    }

    return false;
}

XCamReturn
rk_aiq_uapi2_sysctl_register3Aalgo(const rk_aiq_sys_ctx_t* ctx,
                                   void* algoDes, void *cbs)
{
    RKAIQ_API_SMART_LOCK(ctx);
    if (ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* group_ctx =
            (const rk_aiq_camgroup_ctx_t*)ctx;

        return AiqCamGroupManager_register3Aalgo(group_ctx->cam_group_manager, algoDes, cbs);
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else
        return AiqCore_register3Aalgo(ctx->_analyzer, algoDes, cbs);
}

XCamReturn
rk_aiq_uapi2_sysctl_unRegister3Aalgo(const rk_aiq_sys_ctx_t* ctx,
                                     const int algo_type)
{
    RKAIQ_API_SMART_LOCK(ctx);
    if (ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* group_ctx =
            (const rk_aiq_camgroup_ctx_t*)ctx;

        return AiqCamGroupManager_unregister3Aalgo(group_ctx->cam_group_manager, algo_type);
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else
        return AiqCore_unregister3Aalgo(ctx->_analyzer, algo_type);
}

XCamReturn
rk_aiq_uapi2_sysctl_setSnsSyncMode(const rk_aiq_sys_ctx_t* ctx, enum rkmodule_sync_mode sync_mode)
{
    if (ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
        LOGE("not support !");
        return XCAM_RETURN_ERROR_FAILED;
    }

    AiqSensorHw_t* sensorHw = ctx->_camHw->_mSensorDev;
    return sensorHw->set_sync_mode(sensorHw, sync_mode);
}

XCamReturn rk_aiq_uapi2_sysctl_getHdrComprCurve(const rk_aiq_sys_ctx_t* ctx,
        RkAiqHdrCompr_t* compr) {
    if (ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
        LOGE("not support !");
        return XCAM_RETURN_ERROR_FAILED;
    }

    return AiqCamHw_getHdrComprCurve(ctx->_camHw, compr);
}

XCamReturn rk_aiq_uapi2_setAiBnrBufCnt(const rk_aiq_sys_ctx_t* ctx,
                                       rk_aiq_aibnr_buffer_count_t aibnr_buf_cnt) {
    if (ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
        LOGE("not support !");
        return XCAM_RETURN_ERROR_FAILED;
    }

    return AiqCamHw_setAiBnrBufCnt(ctx->_camHw, aibnr_buf_cnt);
}

XCamReturn rk_aiq_uapi2_sysctl_setOnetoMultiMode(rk_aiq_sys_ctx_t* ctx, int isp_num,
                                                 __u32 frame_pattern[4], bool bSingleExp)
{
    ENTER_XCORE_FUNCTION();
    RKAIQ_API_SMART_LOCK(ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    int mode       = RKMODULE_ONE_CH_TO_MULTI_ISP;
    int video0_fd;
    struct rkmodule_capture_info capture_info;

    rk_sensor_full_info_t* s_info = AiqCamHw_getFullSnsInfo(ctx->_sensor_entity_name);

    if (s_info && (!s_info->split)) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "don't support one cam split to multi mode!");
        return XCAM_RETURN_ERROR_PARAM;
    }

    if (strlen(s_info->cif_info->mipi_id0) == 0) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "%d: can't find mipi_id0 dev path !", __LINE__);
        return XCAM_RETURN_ERROR_FAILED;
    }

    AiqV4l2Device_t* video0Dev = (AiqV4l2Device_t*)aiq_mallocz(sizeof(AiqV4l2Device_t));
    if (video0Dev == NULL) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "%s:%d: alloc fail !", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_FAILED;
    }
    AiqV4l2Device_init(video0Dev, s_info->cif_info->mipi_id0);
    video0Dev->open(video0Dev, false);
    video0_fd = AiqV4l2Device_getFd(video0Dev);
    if (video0_fd < 0) {
        aiq_free(video0Dev);
        LOGW_CAMHW_SUBM(ISP20HW_SUBM, "get video0 device fd failed! Video0_fd = %d", video0_fd);
        return XCAM_RETURN_ERROR_FAILED;
    }

    capture_info.mode                 = RKMODULE_ONE_CH_TO_MULTI_ISP;
    capture_info.one_to_multi.isp_num = isp_num;
    capture_info.one_to_multi.exp_mode = bSingleExp ? RKMODULE_ONE_TO_MULT_EXP_SINGLE : RKMODULE_ONE_TO_MULT_EXP_SWITCH;
    capture_info.one_to_multi.exp_main_id = 0;
    memcpy(capture_info.one_to_multi.frame_pattern, frame_pattern, sizeof(__u32) * 4);

    if (xcam_device_ioctl(video0_fd, RKMODULE_SET_CAPTURE_MODE, &capture_info) < 0) {
        LOGE_CAMHW_SUBM(SENSOR_SUBM, "failed to set rkmodule_capture_info!\n");
        video0Dev->close(video0Dev);
        return XCAM_RETURN_ERROR_IOCTL;
    }

    video0Dev->close(video0Dev);
    aiq_free(video0Dev);

    EXIT_XCORE_FUNCTION();

    return ret;
}

#include "rk_aiq_user_api2_common.c"
#if defined(ISP_HW_V39)
#include "rk_aiq_user_api2_3dlut.c"
#include "rk_aiq_user_api2_rgbir.c"
#include "rk_aiq_user_api2_dehaze.c"
#include "rk_aiq_user_api2_yme.c"
#include "rk_aiq_user_api2_af.c"
#include "rk_aiq_user_api2_ldc.c"
#endif
#if defined(ISP_HW_V33) || defined(ISP_HW_V35)
#include "rk_aiq_user_api2_enh.c"
#include "rk_aiq_user_api2_hsv.c"
#include "rk_aiq_user_api2_texEst.c"
#include "rk_aiq_user_api2_ldc.c"
#include "rk_aiq_user_api2_postisp.c"
#endif
#if defined(ISP_HW_V35)
#if RKAIQ_HAVE_RGBIR_REMOSAIC
#include "rk_aiq_user_api2_rgbir.c"
#endif
#include "rk_aiq_user_api2_af.c"
#endif
#include "rk_aiq_user_api2_aeMeas.c"
#include "rk_aiq_user_api2_blc.c"
#include "rk_aiq_user_api2_btnr.c"
#include "rk_aiq_user_api2_cac.c"
#include "rk_aiq_user_api2_ccm.c"
#include "rk_aiq_user_api2_cgc.c"
#include "rk_aiq_user_api2_cnr.c"
#include "rk_aiq_user_api2_cp.c"
#include "rk_aiq_user_api2_csm.c"
#include "rk_aiq_user_api2_dm.c"
#include "rk_aiq_user_api2_dpc.c"
#include "rk_aiq_user_api2_drc.c"
#include "rk_aiq_user_api2_gain.c"
#include "rk_aiq_user_api2_gamma.c"
#include "rk_aiq_user_api2_gic.c"
#include "rk_aiq_user_api2_ie.c"
#include "rk_aiq_user_api2_histeq.c"
//#include "rk_aiq_user_api2_ldch.c"
#include "rk_aiq_user_api2_lsc.c"
#include "rk_aiq_user_api2_merge.c"
#include "rk_aiq_user_api2_sharp.c"
#include "rk_aiq_user_api2_trans.c"
#include "rk_aiq_user_api2_ynr.c"
#include "rk_aiq_user_api2_ae.c"
#include "rk_aiq_user_api2_awb_v3.c"
#include "rk_aiq_user_api2_imgproc.c"
#include "rk_aiq_user_api2_stats.c"
#include "rk_aiq_user_api2_camgroup.c"
#include "../uAPI2/rk_aiq_user_ae_thread_v25_itf.c"
#include "../uAPI2/rk_aiq_user_api2_custom2_awb.c"
#include "rk_aiq_user_api2_deprecated.c"
#include "rk_aiq_user_api2_aibnr.c"
#include "rk_aiq_user_api2_amtd.c"
#include "rk_aiq_user_api2_airms.c"
#include "rk_aiq_user_api2_aiynr.c"
#include "rk_aiq_user_api2_fpnSw.c"
#include "rk_aiq_user_api2_asd.c"
#endif
