/*
 *  Copyright (c) 2019 Rockchip Corporation
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
#include "rk_aiq_user_api_sysctl.h"
#include "rk_aiq_api_private.h"
#include "RkAiqManager.h"
#include "socket_server.h"
#include "RkAiqCalibDbV2.h"
#include "scene/scene_manager.h"
#include "uAPI/rk_aiq_user_api_sysctl.cpp"

XCamReturn
rk_aiq_uapi2_sysctl_preInit(const char* sns_ent_name,
                           rk_aiq_working_mode_t mode,
                           const char* force_iq_file)
{
    return rk_aiq_uapi_sysctl_preInit(sns_ent_name, mode, force_iq_file);
}

XCamReturn
rk_aiq_uapi2_sysctl_regHwEvtCb(const char* sns_ent_name,
                               rk_aiq_hwevt_cb hwevt_cb,
                               void* cb_ctx)
{
    g_rk_aiq_sys_preinit_cfg_map[sns_ent_name].hwevt_cb = hwevt_cb;
    g_rk_aiq_sys_preinit_cfg_map[sns_ent_name].hwevt_cb_ctx = cb_ctx;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi2_sysctl_preInit_scene(const char* sns_ent_name, const char *main_scene,
                              const char *sub_scene)
{
    return rk_aiq_uapi_sysctl_preInit_scene(sns_ent_name, main_scene, sub_scene);
}

XCamReturn
rk_aiq_uapi2_sysctl_preInit_iq_addr(const char* sns_ent_name, void *addr, size_t len)
{
    g_rk_aiq_sys_preinit_cfg_map[sns_ent_name].iq_buffer.addr = addr;
    g_rk_aiq_sys_preinit_cfg_map[sns_ent_name].iq_buffer.len = len;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi2_sysctl_preInit_calibproj(const char* sns_ent_name, void *addr)
{
    g_rk_aiq_sys_preinit_cfg_map[sns_ent_name].calib_proj = addr;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn rk_aiq_uapi2_sysctl_preInit_devBufCnt(const char* sns_ent_name, const char* dev_ent,
                                                 int buf_cnt) {

    return rk_aiq_uapi_sysctl_preInit_devBufCnt(sns_ent_name, dev_ent, buf_cnt);
}

rk_aiq_sys_ctx_t*
rk_aiq_uapi2_sysctl_init(const char* sns_ent_name,
                        const char* config_file_dir,
                        rk_aiq_error_cb err_cb,
                        rk_aiq_metas_cb metas_cb)
{
    return rk_aiq_uapi_sysctl_init(sns_ent_name, config_file_dir, err_cb, metas_cb);
}

void rk_aiq_uapi2_sysctl_deinit(rk_aiq_sys_ctx_t* ctx)
{
    rk_aiq_uapi_sysctl_deinit(ctx);
}

XCamReturn
rk_aiq_uapi2_sysctl_prepare(const rk_aiq_sys_ctx_t* ctx,
                           uint32_t  width, uint32_t  height,
                           rk_aiq_working_mode_t mode)
{
    return rk_aiq_uapi_sysctl_prepare(ctx, width, height, mode);
}

XCamReturn
rk_aiq_uapi2_sysctl_start(const rk_aiq_sys_ctx_t* ctx)
{
    return rk_aiq_uapi_sysctl_start(ctx);
}

XCamReturn
rk_aiq_uapi2_sysctl_stop(const rk_aiq_sys_ctx_t* ctx, bool keep_ext_hw_st)
{
    return rk_aiq_uapi_sysctl_stop(ctx, keep_ext_hw_st);
}

void rk_aiq_uapi2_get_version_info(rk_aiq_ver_info_t* vers)
{
    rk_aiq_uapi_get_version_info(vers);
}

XCamReturn
rk_aiq_uapi2_sysctl_updateIq(rk_aiq_sys_ctx_t* sys_ctx, char* iqfile)
{
    return rk_aiq_uapi_sysctl_updateIq(sys_ctx, iqfile);
}

int32_t
rk_aiq_uapi2_sysctl_getModuleCtl(const rk_aiq_sys_ctx_t* ctx,
                                rk_aiq_module_id_t mId, bool *mod_en)
{
    return rk_aiq_uapi_sysctl_getModuleCtl(ctx, mId, mod_en);
}

XCamReturn
rk_aiq_uapi2_sysctl_setModuleCtl(const rk_aiq_sys_ctx_t* ctx, rk_aiq_module_id_t mId, bool mod_en)
{
    return rk_aiq_uapi_sysctl_setModuleCtl(ctx, mId, mod_en);
}

XCamReturn
rk_aiq_uapi2_sysctl_enableAxlib(const rk_aiq_sys_ctx_t* ctx,
                               const int algo_type,
                               const int lib_id,
                               bool enable)
{
    return rk_aiq_uapi_sysctl_enableAxlib(ctx, algo_type, lib_id, enable);
}

bool
rk_aiq_uapi2_sysctl_getAxlibStatus(const rk_aiq_sys_ctx_t* ctx,
                                  const int algo_type,
                                  const int lib_id)
{
    return rk_aiq_uapi_sysctl_getAxlibStatus(ctx, algo_type, lib_id);
}

const RkAiqAlgoContext*
rk_aiq_uapi2_sysctl_getEnabledAxlibCtx(const rk_aiq_sys_ctx_t* ctx, const int algo_type)
{
    return rk_aiq_uapi_sysctl_getEnabledAxlibCtx(ctx, algo_type);
}

XCamReturn
rk_aiq_uapi2_sysctl_getStaticMetas(const char* sns_ent_name, rk_aiq_static_info_t* static_info)
{
    return rk_aiq_uapi_sysctl_getStaticMetas(sns_ent_name, static_info);
}


XCamReturn
rk_aiq_uapi2_sysctl_enumStaticMetasByPhyId(int index, rk_aiq_static_info_t* static_info)
{
    return rk_aiq_uapi_sysctl_enumStaticMetasByPhyId(index, static_info);
}

XCamReturn
rk_aiq_uapi2_sysctl_enumStaticMetas(int index, rk_aiq_static_info_t* static_info)
{
    return rk_aiq_uapi_sysctl_enumStaticMetas(index, static_info);
}

const char*
rk_aiq_uapi2_sysctl_getBindedSnsEntNmByVd(const char* vd)
{
    return rk_aiq_uapi_sysctl_getBindedSnsEntNmByVd(vd);
}

XCamReturn
rk_aiq_uapi2_sysctl_getCrop(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_rect_t *rect)
{
    return rk_aiq_uapi_sysctl_getCrop(sys_ctx, rect);
}

#if 1
XCamReturn
rk_aiq_uapi2_sysctl_setCrop(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_rect_t rect)
{
    return rk_aiq_uapi_sysctl_setCrop(sys_ctx, rect);
}
#endif

XCamReturn
rk_aiq_uapi2_sysctl_setCpsLtCfg(const rk_aiq_sys_ctx_t* ctx, rk_aiq_cpsl_cfg_t* cfg)
{
    return rk_aiq_uapi_sysctl_setCpsLtCfg(ctx, cfg);
}

XCamReturn
rk_aiq_uapi2_sysctl_getCpsLtInfo(const rk_aiq_sys_ctx_t* ctx, rk_aiq_cpsl_info_t* info)
{
    return rk_aiq_uapi_sysctl_getCpsLtInfo(ctx, info);
}

XCamReturn
rk_aiq_uapi2_sysctl_queryCpsLtCap(const rk_aiq_sys_ctx_t* ctx, rk_aiq_cpsl_cap_t* cap)
{
    return rk_aiq_uapi_sysctl_queryCpsLtCap(ctx, cap);
}

XCamReturn
rk_aiq_uapi2_sysctl_setSharpFbcRotation(const rk_aiq_sys_ctx_t* ctx, rk_aiq_rotation_t rot)
{
    return rk_aiq_uapi_sysctl_setSharpFbcRotation(ctx, rot);
}

void
rk_aiq_uapi2_sysctl_setMulCamConc(const rk_aiq_sys_ctx_t* ctx, bool cc)
{
    rk_aiq_uapi_sysctl_setMulCamConc(ctx, cc);
}

XCamReturn
rk_aiq_uapi2_sysctl_regMemsSensorIntf(const rk_aiq_sys_ctx_t* sys_ctx,
                                     const rk_aiq_mems_sensor_intf_t* intf)
{
    return rk_aiq_uapi_sysctl_regMemsSensorIntf(sys_ctx, intf);
}

int
rk_aiq_uapi2_sysctl_switch_scene(const rk_aiq_sys_ctx_t* sys_ctx,
                                const char* main_scene, const char* sub_scene)
{
    return rk_aiq_uapi_sysctl_switch_scene(sys_ctx, main_scene, sub_scene);
}

XCamReturn
rk_aiq_uapi2_sysctl_tuning(const rk_aiq_sys_ctx_t* sys_ctx, char* param)
{
    return rk_aiq_uapi_sysctl_tuning(sys_ctx, param);
}

char* rk_aiq_uapi2_sysctl_readiq(const rk_aiq_sys_ctx_t* sys_ctx, char* param)
{
    return rk_aiq_uapi_sysctl_readiq(sys_ctx, param);
}

XCamReturn
rk_aiq_uapi2_sysctl_prepareRkRaw(const rk_aiq_sys_ctx_t* ctx, rk_aiq_raw_prop_t prop)
{
    return rk_aiq_uapi_sysctl_prepareRkRaw(ctx, prop);
}

XCamReturn
rk_aiq_uapi2_sysctl_enqueueRkRawBuf(const rk_aiq_sys_ctx_t* ctx, void *rawdata, bool sync)
{
    return rk_aiq_uapi_sysctl_enqueueRkRawBuf(ctx, rawdata, sync);
}

XCamReturn
rk_aiq_uapi2_sysctl_enqueueRkRawFile(const rk_aiq_sys_ctx_t* ctx, const char *path)
{
    return rk_aiq_uapi_sysctl_enqueueRkRawFile(ctx, path);
}

XCamReturn
rk_aiq_uapi2_sysctl_registRkRawCb(const rk_aiq_sys_ctx_t* ctx, void (*callback)(void*))
{
    return rk_aiq_uapi_sysctl_registRkRawCb(ctx, callback);
}

XCamReturn
rk_aiq_uapi2_sysctl_getCamInfos(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_ctx_camInfo_t* camInfo)
{
    ENTER_XCORE_FUNCTION();
    XCAM_ASSERT(sys_ctx != nullptr);
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
rk_aiq_uapi2_sysctl_get3AStats(const rk_aiq_sys_ctx_t* ctx,
                              rk_aiq_isp_stats_t *stats)
{
    return rk_aiq_uapi_sysctl_get3AStats(ctx, stats);
}

XCamReturn
rk_aiq_uapi2_sysctl_get3AStatsBlk(const rk_aiq_sys_ctx_t* ctx,
                              rk_aiq_isp_stats_t **stats, int timeout_ms)
{
    return rk_aiq_uapi_sysctl_get3AStatsBlk(ctx, stats, timeout_ms);
}

void
rk_aiq_uapi2_sysctl_release3AStatsRef(const rk_aiq_sys_ctx_t* ctx,
                                     rk_aiq_isp_stats_t *stats)
{
    return rk_aiq_uapi_sysctl_release3AStatsRef(ctx, stats);
}

XCamReturn
rk_aiq_uapi2_sysctl_getWorkingMode(const rk_aiq_sys_ctx_t* ctx, rk_aiq_working_mode_t *mode)
{
    ENTER_XCORE_FUNCTION();
    if (!mode || !ctx)
        return XCAM_RETURN_ERROR_PARAM;

    RKAIQ_API_SMART_LOCK(ctx);
    *mode = ctx->_rkAiqManager->getWorkingMode();
    if (*mode < 0)
        return XCAM_RETURN_ERROR_OUTOFRANGE;
    EXIT_XCORE_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

int rk_aiq_uapi2_sysctl_tuning_enable(rk_aiq_sys_ctx_t* sys_ctx, bool enable)
{
    return rk_aiq_uapi_sysctl_tuning_enable(sys_ctx, enable);
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

    ret = sys_ctx->_camHw->reset_hardware();
    if (ret) {
        LOGE("failed to reset hardware\n");
    }

    return ret;
}

void rk_aiq_uapi2_sysctl_rawReproc_genIspParams (rk_aiq_sys_ctx_t* sys_ctx,
                                                 uint32_t sequence,
                                                 rk_aiq_frame_info_t *next_frm_info,
                                                 int mode)
{
    sys_ctx->_camHw->rawReproc_genIspParams(sequence, next_frm_info, mode);
}

const char*
rk_aiq_uapi2_sysctl_rawReproc_preInit(const char* isp_driver,
                                           const char* offline_sns_name,
                                           rk_aiq_frame_info_t two_frm_exp_info[2])
{
    if (!g_rk_aiq_init_lib) {
        rk_aiq_init_lib();
        g_rk_aiq_init_lib = true;
    }
    const char* sns_name = NULL;
    sns_name = CamHwIsp20::rawReproc_preInit(isp_driver, offline_sns_name);
    if (sns_name) {
        rk_aiq_frame_info_t exp_finfo = two_frm_exp_info[0];
        LOGD("exp_finfo %d, %d, %f, %f\n",
               exp_finfo.normal_gain_reg,
               exp_finfo.normal_exp_reg,
               exp_finfo.normal_exp,
               exp_finfo.normal_gain);
        std::string sns_ent_name = std::string(sns_name);
        rk_aiq_sys_preinit_cfg_t cfg;
        memcpy(cfg.frame_exp_info, two_frm_exp_info, sizeof(cfg.frame_exp_info));
        g_rk_aiq_offline_raw_exp_map[sns_ent_name] = cfg;
    }

    return sns_name;
}

void rk_aiq_uapi2_sysctl_setIspParamsDelayCnts(const rk_aiq_sys_ctx_t* sys_ctx, int8_t delay_cnts) {
#ifdef RKAIQ_ENABLE_CAMGROUP
    // TODO
#endif
    sys_ctx->_analyzer->setDelayCnts(delay_cnts);
}

XCamReturn
rk_aiq_uapi2_sysctl_preInit_rkrawstream_info(const char* sns_ent_name,
                           const rk_aiq_rkrawstream_info_t* info)
{
    return rk_aiq_uapi_sysctl_preInit_rkrawtream_info(sns_ent_name, info);
}

XCamReturn rk_aiq_uapi2_sysctl_pause(rk_aiq_sys_ctx_t* sys_ctx, bool isSingleMode)
{
    if (!sys_ctx) {
        LOGE("%s: sys_ctx is invalied\n", __func__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    return sys_ctx->_rkAiqManager->setVicapStreamMode(0, isSingleMode);
}

XCamReturn rk_aiq_uapi2_sysctl_resume(rk_aiq_sys_ctx_t* sys_ctx)
{
    if (!sys_ctx) {
        LOGE("%s: sys_ctx is invalied\n", __func__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    return sys_ctx->_rkAiqManager->setVicapStreamMode(1, false);
}

XCamReturn
rk_aiq_uapi2_sysctl_getAinrParams(const rk_aiq_sys_ctx_t* sys_ctx, rk_ainr_param* para)
{

    if (!sys_ctx) {
        LOGE("%s: sys_ctx is invalied\n", __func__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    Uapi_ExpQueryInfo_t pExpResInfo;
    rk_aiq_working_mode_t mode;
    float dynamicAiBypass = 0;

    rk_aiq_user_api2_ae_queryExpResInfo(sys_ctx, &pExpResInfo);
    rk_aiq_uapi2_sysctl_getWorkingMode(sys_ctx, &mode);

    if (mode == RK_AIQ_WORKING_MODE_NORMAL) {
        para->gain = pExpResInfo.LinAeInfo.LinearExp.analog_gain *
                     pExpResInfo.LinAeInfo.LinearExp.isp_dgain;
        para->RawMeanluma = pExpResInfo.LinAeInfo.MeanLuma;
    } else if (mode == RK_AIQ_WORKING_MODE_ISP_HDR2) {
        para->gain = pExpResInfo.HdrAeInfo.HdrExp[0].analog_gain *
                     pExpResInfo.HdrAeInfo.HdrExp[0].isp_dgain;
        para->RawMeanluma = pExpResInfo.HdrAeInfo.Frm0Luma;
    } else {
        para->gain = pExpResInfo.HdrAeInfo.HdrExp[1].analog_gain *
                     pExpResInfo.HdrAeInfo.HdrExp[1].isp_dgain;
        para->RawMeanluma = pExpResInfo.HdrAeInfo.Frm1Luma;
    }

    CamCalibDbV2Context_t* aiqCalib;
    aiqCalib = sys_ctx->_rkAiqManager->getCurrentCalibDBV2();
    CalibDbV2_PostIspV1_t *ainr = (CalibDbV2_PostIspV1_t*)(CALIBDBV2_GET_MODULE_PTR((void*)aiqCalib, ainr_v1));

    if (!ainr) {
        LOGE("%s: could not get ainr calib \n", __func__);
        return XCAM_RETURN_ERROR_FAILED;
    }

    para->gain_tab_len = ainr->TuningPara.gain_tab_len;
    para->gain_max = ainr->TuningPara.gain_max;
    para->tuning_visual_flag = ainr->TuningPara.tuning_visual_flag;

    for (int i = 0; i < RK_AINR_LUMA_LEN; i++) {
        para->luma_curve_tab[i] = ainr->TuningPara.luma_point[i];
    }

    for (int i = 0; i < ainr->TuningPara.Tuning_ISO_len; i++) {
        para->gain_tab[i] = ainr->TuningPara.Tuning_ISO[i].gain;
        para->sigma_tab[i] = ainr->TuningPara.Tuning_ISO[i].sigma;
        para->shade_tab[i] = ainr->TuningPara.Tuning_ISO[i].shade;
        para->sharp_tab[i] = ainr->TuningPara.Tuning_ISO[i].sharp;
        para->min_luma_tab[i] = ainr->TuningPara.Tuning_ISO[i].min_luma;
        para->sat_scale_tab[i] = ainr->TuningPara.Tuning_ISO[i].sat_scale;
        para->dark_contrast_tab[i] = ainr->TuningPara.Tuning_ISO[i].dark_contrast;
        para->ai_ratio_tab[i] = ainr->TuningPara.Tuning_ISO[i].ai_ratio;
        para->mot_thresh_tab[i] = ainr->TuningPara.Tuning_ISO[i].mot_thresh;
        para->static_thresh_tab[i] = ainr->TuningPara.Tuning_ISO[i].static_thresh;
        para->mot_nr_stren_tab [i] = ainr->TuningPara.Tuning_ISO[i].mot_nr_stren;
        for (int j = 0; j < RK_AINR_LUMA_LEN; j++) {
            para->sigma_curve_tab[j][i] = ainr->TuningPara.Tuning_ISO[i].luma_sigma[j];
        }
    }

    LOGD("getAinrParams test for ainr params set: en=%d tuning_visual_flag: %d"
             "gain_tab_len:%d, gain_max:%d cur_gain:%f raw mean:%f, yuv mean:%f \n",
              para->enable, para->tuning_visual_flag, para->gain_tab_len,
              para->gain_max, para->gain, para->RawMeanluma, para->YuvMeanluma);

    for (int i =0; i < ainr->TuningPara.Tuning_ISO_len; i++) {
        LOGD("gain_tab[%d]: %f, sigam[%d]: %f, shade[%d]: %f, sharp[%d]: %f, min_luma[%d]:%f, sat_scale[%d]:%f"
             ", dark_contrast[%d]:%f, ai_ratio[%d]:%f, mot_thresh[%d]: %f, static_thresh[%d]: %f, mot_nr_stren[%d]: %f \n",
                i, para->gain_tab[i], i, para->sigma_tab[i], i, para->shade_tab[i], i, para->sharp_tab[i], i, para->min_luma_tab[i],
                i, para->sat_scale_tab[i], i, para->dark_contrast_tab[i], i, para->ai_ratio_tab[i], i, para->mot_thresh_tab[i],
                i, para->static_thresh_tab[i], i, para->mot_nr_stren_tab [i]);
    }

    if (para->gain > ainr->TuningPara.dynamicSw[1])
        dynamicAiBypass =  ainr->TuningPara.dynamicSw[0];
    else if (para->gain <  ainr->TuningPara.dynamicSw[0])
        dynamicAiBypass =  ainr->TuningPara.dynamicSw[1];
    else if (sys_ctx->_rkAiqManager->ainr_status)
        dynamicAiBypass =  ainr->TuningPara.dynamicSw[0];
    else
        dynamicAiBypass =  ainr->TuningPara.dynamicSw[1];

    LOGD("ainr bypass switch %f", dynamicAiBypass);

    if (para->gain > dynamicAiBypass){
        para->enable = true;
        LOGD("AINR on\n");
    } else if (para->gain < dynamicAiBypass){
        para->enable = false;
        LOGD("AINR off\n");
    }

    para->enable &= ainr->TuningPara.enable;
    sys_ctx->_rkAiqManager->ainr_status = para->enable;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn rk_aiq_uapi2_sysctl_setUserOtpInfo(rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_user_otp_info_t otp_info)
{
    if (!sys_ctx)
        return XCAM_RETURN_ERROR_PARAM;

    if (sys_ctx->cam_type != RK_AIQ_CAM_TYPE_SINGLE) {
        LOGE("cam_type: %d, only supported in single camera mode", sys_ctx->cam_type);
    }

    return sys_ctx->_analyzer->setUserOtpInfo(otp_info);
}

void
rk_aiq_uapi2_sysctl_setListenStrmStatus(rk_aiq_sys_ctx_t* sys_ctx, bool isListen)
{
    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
        for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
            if (!camCtx)
                continue;

            dynamic_cast<CamHwIsp20*>(camCtx->_camHw.ptr())->setListenStrmEvt(isListen);
        }
#endif
    } else {
        dynamic_cast<CamHwIsp20*>(sys_ctx->_camHw.ptr())->setListenStrmEvt(isListen);
    }
}

static XCamReturn rk_aiq_aiisp_defaut_cb(rk_aiq_aiisp_t* aiisp_evt, void* ctx) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    rk_aiq_sys_ctx_t* aiisp_ctx= (rk_aiq_sys_ctx_t *)ctx;
    aiisp_ctx->_wr_linecnt_now += aiisp_evt->wr_linecnt;
    if (aiisp_ctx->_wr_linecnt_now == aiisp_evt->rd_linecnt) {
        aiisp_ctx->_wr_linecnt_now = 0;
        aiisp_ctx->_camHw->aiisp_processing(aiisp_evt);
        ret = rk_aiq_uapi2_sysctl_ReadAiisp(aiisp_ctx);
    }
    return ret;
}

XCamReturn rk_aiq_uapi2_sysctl_initAiisp(rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_aiisp_cfg_t* aiisp_cfg,
                                         rk_aiq_aiisp_cb aiisp_cb)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#ifdef ISP_HW_V39
    if (!sys_ctx)
        return XCAM_RETURN_ERROR_PARAM;

    if (aiisp_cfg == NULL) {
        rk_aiq_aiisp_cfg_t aiisp_cfg_tmp;
        rk_aiq_exposure_sensor_descriptor sensor_des;
        sys_ctx->_camHw->getSensorModeData(sys_ctx->_sensor_entity_name, sensor_des);
        uint32_t height = sensor_des.sensor_output_height;
        aiisp_cfg_tmp.wr_mode = 1;
        aiisp_cfg_tmp.rd_mode = 0;
        aiisp_cfg_tmp.wr_linecnt = height / 2;
        aiisp_cfg_tmp.rd_linecnt = height;
        ret = sys_ctx->_camHw->setAiispMode(&aiisp_cfg_tmp);
    }
    else {
        ret = sys_ctx->_camHw->setAiispMode(aiisp_cfg);
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
    sys_ctx->_rkAiqManager->setAiispCb(aiispCtx);
    sys_ctx->_use_aiisp = true;

    // RkAiqAblcV32HandleInt* algo_handle =
    //     algoHandle<RkAiqAblcV32HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_ABLC);
    // if (algo_handle)
    //     ret = algo_handle->setAiisp();
    rk_aiq_blc_attrib_V32_t blc_attr;
    memset(&blc_attr, 0x00, sizeof(blc_attr));
    ret = rk_aiq_user_api2_ablcV32_GetAttrib(sys_ctx, &blc_attr);
    blc_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
    AblcOPMode_V32_t eMode_tmp = blc_attr.eMode;
    blc_attr.eMode = ABLC_V32_OP_MODE_MANUAL;
    blc_attr.stBlcOBManual.enable = 0;
    ret = rk_aiq_user_api2_ablcV32_SetAttrib(sys_ctx, &blc_attr);

    ret = rk_aiq_user_api2_ablcV32_GetAttrib(sys_ctx, &blc_attr);
    blc_attr.sync.sync_mode = RK_AIQ_UAPI_MODE_ASYNC;
    blc_attr.eMode = ABLC_V32_OP_MODE_AUTO;
    blc_attr.stBlcOBAuto.enable = 0;
    ret = rk_aiq_user_api2_ablcV32_SetAttrib(sys_ctx, &blc_attr);
    blc_attr.eMode = eMode_tmp;
    ret = rk_aiq_user_api2_ablcV32_SetAttrib(sys_ctx, &blc_attr);

    LOGK("AIISP on");
#else
    LOGE("The current platform does not support");
#endif
    return ret;
}


XCamReturn rk_aiq_uapi2_sysctl_ReadAiisp(rk_aiq_sys_ctx_t* sys_ctx)
{
    if (!sys_ctx)
        return XCAM_RETURN_ERROR_PARAM;
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ret = sys_ctx->_camHw->read_aiisp_result();
    LOGD("start to read AIISP result");
    return ret;
}
