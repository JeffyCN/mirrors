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

#include "hwi_c/aiq_lensHw.h"

#define LENS_HELP_THREAD_MSG_MAX (3)

static XCamReturn _AiqLensHw_start(AiqLensHw_t* pLensHw, bool prepared);
static XCamReturn _AiqLensHw_stop(AiqLensHw_t* pLensHw);
static XCamReturn _AiqLensHw_open(AiqLensHw_t* pLensHw);
static XCamReturn _AiqLensHw_close(AiqLensHw_t* pLensHw);

static bool LensHwHelperThd_loop(void* user_data) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ENTER_ANALYZER_FUNCTION();

    LensHwHelperThd_t* pHdlTh = (LensHwHelperThd_t*)user_data;
    int32_t wait_handle_cnt = 0;

    aiqMutex_lock(&pHdlTh->_mutex);
    while (!pHdlTh->bQuit && aiqList_size(pHdlTh->mMsgsQueue) <= 0) {
        aiqCond_wait(&pHdlTh->_cond, &pHdlTh->_mutex);
    }
    if (pHdlTh->bQuit) {
        aiqMutex_unlock(&pHdlTh->_mutex);
        LOGE_CAMHW_SUBM(LENS_SUBM, "quit LensHelperTh !");
        return false;
    }
    aiqMutex_unlock(&pHdlTh->_mutex);

    AiqListItem_t* pItem = aiqList_get_item(pHdlTh->mMsgsQueue, NULL);
    if (!pItem) {
        // ignore error
        return true;
    }
    // XCAM_STATIC_FPS_CALCULATION(GROUPMSGTH, 100);
    rk_aiq_focus_params_t* attrib = (rk_aiq_focus_params_t*)pItem->_pData;

    if (attrib->zoomfocus_modifypos) {
        AiqLensHw_ZoomFocusModifyPositionSync(pHdlTh->mLensHw, attrib);
    } else if (attrib->focus_correction) {
        wait_handle_cnt = 0;
        AiqLensHw_FocusCorrectionSync(pHdlTh->mLensHw);
        while (AiqLensHw_IsZoomInCorrectionSync(pHdlTh->mLensHw)) {
            LOGD_AF("wait zoom correction");
            usleep(10000);
            if (wait_handle_cnt++ > 1000) {
                LOGE_AF("wait zoom correction timeout");
                break;
            }
        }
    } else if (attrib->zoom_correction) {
        wait_handle_cnt = 0;
        AiqLensHw_ZoomCorrectionSync(pHdlTh->mLensHw);
        while (AiqLensHw_IsFocusInCorrectionSync(pHdlTh->mLensHw)) {
            LOGD_AF("wait focus correction");
            usleep(10000);
            if (wait_handle_cnt++ > 1000) {
                LOGE_AF("wait focus correction timeout");
                break;
            }
        }
    } else if (attrib->lens_pos_valid == 1 && attrib->zoom_pos_valid == 0) {
        if (attrib->end_zoom_chg) {
            ret = AiqLensHw_endZoomChgSync(pHdlTh->mLensHw, attrib, true);
        }
        ret = AiqLensHw_setFocusParamsSync(pHdlTh->mLensHw, attrib->next_lens_pos[0], true, attrib->focus_noreback);
    } else {
        if (attrib->send_zoom_reback == 1 || attrib->send_zoom1_reback == 1 || attrib->send_focus_reback == 1) {
            AiqLensHw_setZoomFocusRebackSync(pHdlTh->mLensHw, attrib, false);
        }
        if (attrib->end_zoom_chg) {
            if (attrib->IsNeedCkRebackAtStart)
                ret = AiqLensHw_startZoomChgSync(pHdlTh->mLensHw, attrib, false);
            ret = AiqLensHw_setZoomFocusParamsSync(pHdlTh->mLensHw, attrib, false);
            ret = AiqLensHw_endZoomChgSync(pHdlTh->mLensHw, attrib, true);
        } else {
            if (attrib->IsNeedCkRebackAtStart)
                ret = AiqLensHw_startZoomChgSync(pHdlTh->mLensHw, attrib, false);
            ret = AiqLensHw_setZoomFocusParamsSync(pHdlTh->mLensHw, attrib, true);
        }
    }

    aiqList_erase_item(pHdlTh->mMsgsQueue, pItem);

    if (ret == XCAM_RETURN_NO_ERROR)
        return true;

    LOGE_CAMHW_SUBM(LENS_SUBM, "LensHwHelperThd failed to run command!");

    EXIT_ANALYZER_FUNCTION();

    return false;
}

static bool _LensHwHelperThd_push_attr(LensHwHelperThd_t* pHelpThd,
                                          rk_aiq_focus_params_t* params) {
    if (!pHelpThd->_base->_started) return false;

    int ret = aiqList_push(pHelpThd->mMsgsQueue, params);
    if (ret) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "push focus params failed", __func__);
        return false;
    }

    aiqCond_broadcast(&pHelpThd->_cond);

    return true;
}


XCamReturn LensHwHelperThd_init(LensHwHelperThd_t* pHelpThd, AiqLensHw_t* lenshw, int id)
{
    pHelpThd->mId = id;
    pHelpThd->mLensHw = lenshw;

    aiqMutex_init(&pHelpThd->_mutex);
    aiqCond_init(&pHelpThd->_cond);

    AiqListConfig_t msgqCfg;
    msgqCfg._name      = "AfHMsgQ";
    msgqCfg._item_nums = LENS_HELP_THREAD_MSG_MAX;
    msgqCfg._item_size = sizeof(rk_aiq_focus_params_t);
    pHelpThd->mMsgsQueue = aiqList_init(&msgqCfg);
    if (!pHelpThd->mMsgsQueue) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "init %s error", msgqCfg._name);
        goto fail;
    }

    pHelpThd->_base = aiqThread_init("AfHelpTh", LensHwHelperThd_loop, pHelpThd);
    if (!pHelpThd->_base) goto fail;

    return XCAM_RETURN_NO_ERROR;

fail:
    return XCAM_RETURN_ERROR_FAILED;
}

void LensHwHelperThd_deinit(LensHwHelperThd_t* pHelpThd)
{
    ENTER_ANALYZER_FUNCTION();
    if (pHelpThd->mMsgsQueue) aiqList_deinit(pHelpThd->mMsgsQueue);
    if (pHelpThd->_base) aiqThread_deinit(pHelpThd->_base);

    aiqMutex_deInit(&pHelpThd->_mutex);
    aiqCond_deInit(&pHelpThd->_cond);
    EXIT_ANALYZER_FUNCTION();
}

XCamReturn LensHwHelperThd_start(LensHwHelperThd_t* pHelpThd)
{
    ENTER_ANALYZER_FUNCTION();
    pHelpThd->bQuit = false;
    if (!aiqThread_start(pHelpThd->_base)) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "%s failed", __func__);
        return XCAM_RETURN_ERROR_FAILED;
    }
    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn LensHwHelperThd_stop(LensHwHelperThd_t* pHelpThd)
{
    ENTER_ANALYZER_FUNCTION();
    aiqMutex_lock(&pHelpThd->_mutex);
    pHelpThd->bQuit = true;
    aiqMutex_unlock(&pHelpThd->_mutex);
    aiqCond_broadcast(&pHelpThd->_cond);
    aiqThread_stop(pHelpThd->_base);
    AiqListItem_t* pItem = NULL;
    bool rm              = false;
    AIQ_LIST_FOREACH(pHelpThd->mMsgsQueue, pItem, rm) {
        rm    = true;
        pItem = aiqList_erase_item_locked(pHelpThd->mMsgsQueue, pItem);
    }
    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqLensHw_init(AiqLensHw_t* pLensHw, const char* name)
{
    AiqV4l2SubDevice_init(&pLensHw->mSd, name);
    pLensHw->_piris_step = -1;
    pLensHw->_max_logical_pos = 64;
    aiqMutex_init(&pLensHw->_mutex);
    LensHwHelperThd_init(&pLensHw->_lenshw_thd, pLensHw, 0);
    LensHwHelperThd_init(&pLensHw->_lenshw_thd1, pLensHw, 1);
    pLensHw->open = _AiqLensHw_open;
    pLensHw->start = _AiqLensHw_start;
    pLensHw->stop = _AiqLensHw_stop;
    pLensHw->close = _AiqLensHw_close;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqLensHw_deinit(AiqLensHw_t* pLensHw)
{
    LensHwHelperThd_deinit(&pLensHw->_lenshw_thd);
    LensHwHelperThd_deinit(&pLensHw->_lenshw_thd1);
    aiqMutex_deInit(&pLensHw->_mutex);
    AiqV4l2SubDevice_deinit(&pLensHw->mSd);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqLensHw_getOTPData(AiqLensHw_t* pLensHw)
{
    ENTER_CAMHW_FUNCTION();

    pLensHw->_otp_valid = false;
    pLensHw->_startCurrent = -1;
    pLensHw->_endCurrent = -1;
    pLensHw->_posture = -1;
    pLensHw->_hysteresis = -1;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqLensHw_queryLensSupport(AiqLensHw_t* pLensHw)
{
    ENTER_CAMHW_FUNCTION();

    pLensHw->_iris_enable = false;
    pLensHw->_focus_enable = false;
    pLensHw->_zoom_enable = false;
    pLensHw->_zoom1_enable = false;

    if (!pLensHw->mSd._v4l_base._name)
        return XCAM_RETURN_NO_ERROR;

    memset(&pLensHw->_iris_query, 0, sizeof(pLensHw->_iris_query));
    pLensHw->_iris_query.id = V4L2_CID_IRIS_ABSOLUTE;
    if (pLensHw->mSd._v4l_base.io_control(&pLensHw->mSd._v4l_base, VIDIOC_QUERYCTRL, &pLensHw->_iris_query) < 0) {
        LOGI_CAMHW_SUBM(LENS_SUBM, "query iris ctrl failed");
        pLensHw->_iris_enable = false;
    } else {
        pLensHw->_iris_enable = true;
    }

    memset(&pLensHw->_focus_query, 0, sizeof(pLensHw->_focus_query));
    pLensHw->_focus_query.id = V4L2_CID_FOCUS_ABSOLUTE;
    if (pLensHw->mSd._v4l_base.io_control(&pLensHw->mSd._v4l_base, VIDIOC_QUERYCTRL, &pLensHw->_focus_query) < 0) {
        LOGI_CAMHW_SUBM(LENS_SUBM, "query focus ctrl failed");
        pLensHw->_focus_enable = false;
    } else {
        pLensHw->_focus_enable = true;
    }

    memset(&pLensHw->_zoom_query, 0, sizeof(pLensHw->_zoom_query));
    pLensHw->_zoom_query.id = V4L2_CID_ZOOM_ABSOLUTE;
    if (pLensHw->mSd._v4l_base.io_control(&pLensHw->mSd._v4l_base, VIDIOC_QUERYCTRL, &pLensHw->_zoom_query) < 0) {
        LOGI_CAMHW_SUBM(LENS_SUBM, "query zoom ctrl failed");
        pLensHw->_zoom_enable = false;
    } else {
        pLensHw->_zoom_enable = true;
    }

    memset(&pLensHw->_zoom1_query, 0, sizeof(pLensHw->_zoom1_query));
    pLensHw->_zoom1_query.id = V4L2_CID_ZOOM_CONTINUOUS;
    if (pLensHw->mSd._v4l_base.io_control(&pLensHw->mSd._v4l_base, VIDIOC_QUERYCTRL, &pLensHw->_zoom1_query) < 0) {
        LOGI_CAMHW_SUBM(LENS_SUBM, "query zoom1 ctrl failed");
        pLensHw->_zoom1_enable = false;
    } else {
        pLensHw->_zoom1_enable = true;
    }

    AiqLensHw_getOTPData(pLensHw);

    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _start_internal(AiqLensHw_t* pLensHw)
{
    ENTER_CAMHW_FUNCTION();

    if (pLensHw->mSd._v4l_base._active)
        return XCAM_RETURN_NO_ERROR;

    pLensHw->_rec_sof_idx = 0;
    pLensHw->_rec_lowfv_idx = 0;
    pLensHw->_piris_step = 0;
    pLensHw->_last_piris_step = 0;
    pLensHw->_dciris_pwmduty = 0;
    pLensHw->_last_dciris_pwmduty = 0;
    pLensHw->_focus_pos = -1;
    pLensHw->_zoom_pos = -1;
    pLensHw->_zoom1_pos = -1;
    pLensHw->_angleZ = 0;
    pLensHw->_zoom_correction = false;
    pLensHw->_focus_correction = false;
    memset(&pLensHw->_focus_tim, 0, sizeof(pLensHw->_focus_tim));
    memset(&pLensHw->_zoom_tim, 0, sizeof(pLensHw->_zoom_tim));
    memset(&pLensHw->_zoom1_tim, 0, sizeof(pLensHw->_zoom1_tim));
    memset(pLensHw->_frame_time, 0, sizeof(pLensHw->_frame_time));
    memset(pLensHw->_frame_sequence, 0, sizeof(pLensHw->_frame_sequence));
    memset(pLensHw->_lowfv_fv4_4, 0, sizeof(pLensHw->_lowfv_fv4_4));
    memset(pLensHw->_lowfv_fv8_8, 0, sizeof(pLensHw->_lowfv_fv8_8));
    memset(pLensHw->_lowfv_highlht, 0, sizeof(pLensHw->_lowfv_highlht));
    memset(pLensHw->_lowfv_seq, 0, sizeof(pLensHw->_lowfv_seq));
    AiqLensHw_queryLensSupport(pLensHw);
    if (pLensHw->_zoom_enable) {
        LensHwHelperThd_start(&pLensHw->_lenshw_thd);
        LensHwHelperThd_start(&pLensHw->_lenshw_thd1);
    }

    pLensHw->mSd._v4l_base._active = true;
    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}
static XCamReturn _AiqLensHw_open(AiqLensHw_t* pLensHw)
{
    return AiqV4l2SubDevice_open(&pLensHw->mSd, false);
}

static XCamReturn _AiqLensHw_close(AiqLensHw_t* pLensHw)
{
    return AiqV4l2SubDevice_close(&pLensHw->mSd);
}

static XCamReturn _AiqLensHw_start(AiqLensHw_t* pLensHw, bool prepared)
{
    (void)prepared;
    ENTER_CAMHW_FUNCTION();
    aiqMutex_lock(&pLensHw->_mutex);
    if (!pLensHw->mSd._v4l_base._name) {
        aiqMutex_unlock(&pLensHw->_mutex);
        return XCAM_RETURN_NO_ERROR;
    }

    _start_internal(pLensHw);

    EXIT_CAMHW_FUNCTION();

    aiqMutex_unlock(&pLensHw->_mutex);

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _AiqLensHw_stop(AiqLensHw_t* pLensHw)
{
    ENTER_CAMHW_FUNCTION();

    if (!pLensHw->mSd._v4l_base._name) {
        return XCAM_RETURN_NO_ERROR;
    }

    if (pLensHw->_zoom_enable) {
        LensHwHelperThd_stop(&pLensHw->_lenshw_thd);
        LensHwHelperThd_stop(&pLensHw->_lenshw_thd1);
    }

    aiqMutex_lock(&pLensHw->_mutex);
    pLensHw->mSd._v4l_base._active = false;
    aiqMutex_unlock(&pLensHw->_mutex);
    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqLensHw_getLensModeData(AiqLensHw_t* pLensHw, rk_aiq_lens_descriptor* lens_des)
{
    ENTER_CAMHW_FUNCTION();

    aiqMutex_lock(&pLensHw->_mutex);
    if (!pLensHw->mSd._v4l_base._name) {
        aiqMutex_unlock(&pLensHw->_mutex);
        return XCAM_RETURN_NO_ERROR;
    }
    if (!pLensHw->mSd._v4l_base._active)
        AiqLensHw_queryLensSupport(pLensHw);
    lens_des->focus_support = pLensHw->_focus_enable;
    lens_des->iris_support = pLensHw->_iris_enable;
    lens_des->zoom_support = pLensHw->_zoom_enable;
    lens_des->focus_minimum = pLensHw->_focus_query.minimum;
    lens_des->focus_maximum = pLensHw->_focus_query.maximum;
    lens_des->zoom_minimum = pLensHw->_zoom_query.minimum;
    lens_des->zoom_maximum = pLensHw->_zoom_query.maximum;
    lens_des->zoom1_minimum = pLensHw->_zoom1_query.minimum;
    lens_des->zoom1_maximum = pLensHw->_zoom1_query.maximum;
    lens_des->otp_valid = pLensHw->_otp_valid;
    lens_des->posture = pLensHw->_posture;
    lens_des->hysteresis = pLensHw->_hysteresis;
    lens_des->startCurrent = pLensHw->_startCurrent;
    lens_des->endCurrent = pLensHw->_endCurrent;
    EXIT_CAMHW_FUNCTION();

    aiqMutex_unlock(&pLensHw->_mutex);
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqLensHw_getLensVcmCfg(AiqLensHw_t* pLensHw, rk_aiq_lens_vcmcfg* lens_cfg)
{
    ENTER_CAMHW_FUNCTION();
    struct rk_cam_vcm_cfg cfg;

    if (!pLensHw->mSd._v4l_base._name)
        return XCAM_RETURN_NO_ERROR;

    if (pLensHw->mSd._v4l_base.io_control(&pLensHw->mSd._v4l_base, RK_VIDIOC_GET_VCM_CFG, &cfg) < 0) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "get vcm cfg failed");
        return XCAM_RETURN_ERROR_IOCTL;
    }
    lens_cfg->start_ma = cfg.start_ma;
    lens_cfg->rated_ma = cfg.rated_ma;
    lens_cfg->step_mode = cfg.step_mode;

    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqLensHw_setLensVcmCfg(AiqLensHw_t* pLensHw, rk_aiq_lens_vcmcfg* lens_cfg)
{
    ENTER_CAMHW_FUNCTION();
    struct rk_cam_vcm_cfg cfg;

    if (!pLensHw->mSd._v4l_base._name)
        return XCAM_RETURN_NO_ERROR;

    cfg.start_ma = lens_cfg->start_ma;
    cfg.rated_ma = lens_cfg->rated_ma;
    cfg.step_mode = lens_cfg->step_mode;
    if (pLensHw->mSd._v4l_base.io_control(&pLensHw->mSd._v4l_base, RK_VIDIOC_SET_VCM_CFG, &cfg) < 0) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "set vcm cfg failed");
        return XCAM_RETURN_ERROR_IOCTL;
    }

    LOGD_AF("%s: start_ma %d, rated_ma %d, step_mode %d",
            __func__, cfg.start_ma, cfg.rated_ma, cfg.step_mode);

    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqLensHw_getLensVcmMaxlogpos(AiqLensHw_t* pLensHw, int* max_log_pos)
{
    ENTER_CAMHW_FUNCTION();

    if (!pLensHw->mSd._v4l_base._name)
        return XCAM_RETURN_NO_ERROR;

    *max_log_pos = pLensHw->_max_logical_pos;

    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;

}

XCamReturn AiqLensHw_setLensVcmMaxlogpos(AiqLensHw_t* pLensHw, int* max_log_pos)
{
    ENTER_CAMHW_FUNCTION();

    struct v4l2_queryctrl focus_query;

    pLensHw->_max_logical_pos = *max_log_pos;
    if (!pLensHw->mSd._v4l_base._name)
        return XCAM_RETURN_NO_ERROR;

    if (pLensHw->mSd._v4l_base.io_control(&pLensHw->mSd._v4l_base, RK_VIDIOC_SET_VCM_MAX_LOGICALPOS, max_log_pos) < 0) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "set vcm cfg failed");
        return XCAM_RETURN_ERROR_IOCTL;
    }

    memset(&focus_query, 0, sizeof(focus_query));
    focus_query.id = V4L2_CID_FOCUS_ABSOLUTE;
    if (pLensHw->mSd._v4l_base.io_control(&pLensHw->mSd._v4l_base, VIDIOC_QUERYCTRL, &focus_query) < 0) {
        LOGI_CAMHW_SUBM(LENS_SUBM, "query focus ctrl failed");
        return XCAM_RETURN_ERROR_IOCTL;
    } else {
        pLensHw->_focus_query = focus_query;
    }

    LOGD_AF("%s: max_log_pos %d", __func__, *max_log_pos);

    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqLensHw_setFocusParamsSync(AiqLensHw_t* pLensHw, int position, bool is_update_time,
                                        bool focus_noreback)
{
    ENTER_CAMHW_FUNCTION();
    struct rk_cam_set_focus set_focus;
    unsigned long end_time;

#ifdef DISABLE_ZOOM_FOCUS
    return XCAM_RETURN_NO_ERROR;
#endif

    if (!pLensHw->_focus_enable) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "focus is not supported");
        return XCAM_RETURN_NO_ERROR;
    }

    if (position < pLensHw->_focus_query.minimum)
        position = pLensHw->_focus_query.minimum;
    if (position > pLensHw->_focus_query.maximum)
        position = pLensHw->_focus_query.maximum;

    xcam_mem_clear (set_focus);
    set_focus.focus_pos = position;
    if ((position < pLensHw->_focus_pos) && !focus_noreback)
        set_focus.is_need_reback = true;

    if (pLensHw->mSd._v4l_base.io_control(&pLensHw->mSd._v4l_base, RK_VIDIOC_FOCUS_SET_POSITION, &set_focus) < 0) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "set focus result failed to device");
        return XCAM_RETURN_ERROR_IOCTL;
    }
    pLensHw->_focus_pos = position;

    if (!focus_noreback)
        pLensHw->_last_zoomchg_focus = position;

    struct rk_cam_vcm_tim tim;
    if (pLensHw->mSd._v4l_base.io_control(&pLensHw->mSd._v4l_base, RK_VIDIOC_VCM_TIMEINFO, &tim) < 0) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "get focus timeinfo failed");
        aiqMutex_lock(&pLensHw->_mutex);
        if (is_update_time)
            pLensHw->_focus_tim.vcm_end_t.tv_sec += 2;
        aiqMutex_unlock(&pLensHw->_mutex);
    } else {
        aiqMutex_lock(&pLensHw->_mutex);
        if (is_update_time)
            pLensHw->_focus_tim = tim;
        aiqMutex_unlock(&pLensHw->_mutex);

        end_time = pLensHw->_focus_tim.vcm_end_t.tv_sec * 1000 + pLensHw->_focus_tim.vcm_end_t.tv_usec / 1000;
        LOGD_CAMHW_SUBM(LENS_SUBM, "|||set focus result: %d, focus_pos %d, _last_zoomchg_focus %d, end time %ld, is_update_time %d, is_need_reback %d",
                        position, set_focus.focus_pos, pLensHw->_last_zoomchg_focus, end_time, is_update_time, set_focus.is_need_reback);
    }

    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;

}

XCamReturn AiqLensHw_setFocusParams(AiqLensHw_t* pLensHw, rk_aiq_focus_params_t* focus_params)
{
    ENTER_CAMHW_FUNCTION();
    aiqMutex_lock(&pLensHw->_mutex);
    rk_aiq_focus_params_t* p_focus = focus_params;

    if (!pLensHw->_focus_enable) {
        aiqMutex_unlock(&pLensHw->_mutex);
        LOGE_CAMHW_SUBM(LENS_SUBM, "focus is not supported");
        return XCAM_RETURN_NO_ERROR;
    }

    if (!pLensHw->mSd._v4l_base._active)
        _start_internal(pLensHw);

#ifdef DISABLE_ZOOM_FOCUS
    aiqMutex_unlock(&pLensHw->_mutex);
    return XCAM_RETURN_NO_ERROR;
#endif

    if (pLensHw->_zoom_enable) {
        rk_aiq_focus_params_t attrPtr;

        attrPtr.zoomfocus_modifypos = false;
        attrPtr.focus_correction = false;
        attrPtr.zoom_correction = false;
        attrPtr.lens_pos_valid = true;
        attrPtr.zoom_pos_valid = false;
        attrPtr.IsNeedCkRebackAtStart = false;
        attrPtr.send_zoom_reback = p_focus->send_zoom_reback;
        attrPtr.send_zoom1_reback = p_focus->send_zoom1_reback;
        attrPtr.send_focus_reback = p_focus->send_focus_reback;
        attrPtr.end_zoom_chg = p_focus->end_zoom_chg;
        attrPtr.focus_noreback = p_focus->focus_noreback;
        attrPtr.next_pos_num = 1;
        attrPtr.next_lens_pos[0] = p_focus->next_lens_pos[0];

        LOGI_AF("set focus position: %d", attrPtr.next_lens_pos[0]);
        _LensHwHelperThd_push_attr(&pLensHw->_lenshw_thd, &attrPtr);
    } else {
        struct v4l2_control control;
        unsigned long start_time, end_time;
        int algo_pos, driver_pos;

        algo_pos = p_focus->next_lens_pos[0];
        driver_pos = algo_pos * (pLensHw->_focus_query.maximum - pLensHw->_focus_query.minimum) / pLensHw->_max_logical_pos + pLensHw->_focus_query.minimum;
        if (driver_pos < pLensHw->_focus_query.minimum)
            driver_pos = pLensHw->_focus_query.minimum;
        if (driver_pos > pLensHw->_focus_query.maximum)
            driver_pos = pLensHw->_focus_query.maximum;

        xcam_mem_clear (control);
        control.id = V4L2_CID_FOCUS_ABSOLUTE;
        control.value = driver_pos;

        if (pLensHw->mSd._v4l_base.io_control(&pLensHw->mSd._v4l_base, VIDIOC_S_CTRL, &control) < 0) {
            aiqMutex_unlock(&pLensHw->_mutex);
            LOGE_CAMHW_SUBM(LENS_SUBM, "set focus result failed to device");
            return XCAM_RETURN_ERROR_IOCTL;
        }
        pLensHw->_focus_pos = algo_pos;

        struct rk_cam_vcm_tim tim;
        if (pLensHw->mSd._v4l_base.io_control(&pLensHw->mSd._v4l_base, RK_VIDIOC_VCM_TIMEINFO, &tim) < 0) {
            aiqMutex_unlock(&pLensHw->_mutex);
            LOGE_CAMHW_SUBM(LENS_SUBM, "get focus timeinfo failed");
            return XCAM_RETURN_ERROR_IOCTL;
        }
        pLensHw->_focus_tim = tim;

        start_time = pLensHw->_focus_tim.vcm_start_t.tv_sec * 1000 + pLensHw->_focus_tim.vcm_start_t.tv_usec / 1000;
        end_time = pLensHw->_focus_tim.vcm_end_t.tv_sec * 1000 + pLensHw->_focus_tim.vcm_end_t.tv_usec / 1000;
        LOGI_AF("|||set focus: algo_pos %d, driver_pos %d, end time %ld, need time %d",
                algo_pos, driver_pos, end_time, end_time - start_time);
    }

    EXIT_CAMHW_FUNCTION();
    aiqMutex_unlock(&pLensHw->_mutex);
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqLensHw_setPIrisParams(AiqLensHw_t* pLensHw, int step)
{
    ENTER_CAMHW_FUNCTION();
    aiqMutex_lock(&pLensHw->_mutex);
    struct v4l2_control control;

    if (!pLensHw->_iris_enable) {
        aiqMutex_unlock(&pLensHw->_mutex);
        LOGE_CAMHW_SUBM(LENS_SUBM, "iris is not supported");
        return XCAM_RETURN_NO_ERROR;
    }

    if (!pLensHw->mSd._v4l_base._active)
        _start_internal(pLensHw);

    if (pLensHw->_piris_step == step) {
        aiqMutex_unlock(&pLensHw->_mutex);
        return XCAM_RETURN_NO_ERROR;
    }
    //get old Piris-step
    pLensHw->_last_piris_step = pLensHw->_piris_step;

    xcam_mem_clear (control);
    control.id = V4L2_CID_IRIS_ABSOLUTE;
    control.value = step;

    LOGD_CAMHW_SUBM(LENS_SUBM, "|||set iris result: %d, control.value %d", step, control.value);
    if (pLensHw->mSd._v4l_base.io_control(&pLensHw->mSd._v4l_base, VIDIOC_S_CTRL, &control) < 0) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "set iris result failed to device");
        aiqMutex_unlock(&pLensHw->_mutex);
        return XCAM_RETURN_ERROR_IOCTL;
    }
    pLensHw->_piris_step = step;

    struct rk_cam_motor_tim tim;
    if (pLensHw->mSd._v4l_base.io_control(&pLensHw->mSd._v4l_base, RK_VIDIOC_IRIS_TIMEINFO, &tim) < 0) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "get iris timeinfo failed");
        aiqMutex_unlock(&pLensHw->_mutex);
        return XCAM_RETURN_ERROR_IOCTL;
    }
    pLensHw->_piris_tim = tim;
    aiqMutex_unlock(&pLensHw->_mutex);

    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;

}

XCamReturn AiqLensHw_setDCIrisParams(AiqLensHw_t* pLensHw, int pwmDuty)
{
    ENTER_CAMHW_FUNCTION();
    aiqMutex_lock(&pLensHw->_mutex);
    struct v4l2_control control;

    if (!pLensHw->_iris_enable) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "iris is not supported");
        aiqMutex_unlock(&pLensHw->_mutex);
        return XCAM_RETURN_NO_ERROR;
    }

    if (!pLensHw->mSd._v4l_base._active)
        _start_internal(pLensHw);

    if (pLensHw->_dciris_pwmduty == pwmDuty) {
        aiqMutex_unlock(&pLensHw->_mutex);
        return XCAM_RETURN_NO_ERROR;
    }
    //get old DCiris-step
    pLensHw->_last_dciris_pwmduty = pwmDuty;

    xcam_mem_clear (control);
    control.id = V4L2_CID_IRIS_ABSOLUTE;
    control.value = pwmDuty;

    LOGD_CAMHW_SUBM(LENS_SUBM, "|||set dc-iris result: %d, control.value %d", pwmDuty, control.value);
    if (pLensHw->mSd._v4l_base.io_control(&pLensHw->mSd._v4l_base, VIDIOC_S_CTRL, &control) < 0) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "set dc-iris result failed to device");
        aiqMutex_unlock(&pLensHw->_mutex);
        return XCAM_RETURN_ERROR_IOCTL;
    }
    pLensHw->_dciris_pwmduty = pwmDuty;
    aiqMutex_unlock(&pLensHw->_mutex);

    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqLensHw_setHDCIrisParams(AiqLensHw_t* pLensHw, int target)
{
    ENTER_CAMHW_FUNCTION();
    aiqMutex_lock(&pLensHw->_mutex);
    struct v4l2_control control;

    if (!pLensHw->_iris_enable) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "iris is not supported");
        aiqMutex_unlock(&pLensHw->_mutex);
        return XCAM_RETURN_NO_ERROR;
    }

    if (!pLensHw->mSd._v4l_base._active)
        _start_internal(pLensHw);

    if (pLensHw->_hdciris_target == target) {
        aiqMutex_unlock(&pLensHw->_mutex);
        return XCAM_RETURN_NO_ERROR;
    }
    //get old HDCiris-step
    pLensHw->_last_hdciris_target = pLensHw->_hdciris_target;

    xcam_mem_clear (control);
    control.id = V4L2_CID_IRIS_ABSOLUTE;
    control.value = target;

    LOGD_CAMHW_SUBM(LENS_SUBM, "|||set hdc-iris result: %d, control.value %d", target, control.value);
    if (pLensHw->mSd._v4l_base.io_control(&pLensHw->mSd._v4l_base, VIDIOC_S_CTRL, &control) < 0) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "set hdc-iris result failed to device");
        aiqMutex_unlock(&pLensHw->_mutex);
        return XCAM_RETURN_ERROR_IOCTL;
    }
    pLensHw->_hdciris_target = target;

    EXIT_CAMHW_FUNCTION();
    aiqMutex_unlock(&pLensHw->_mutex);
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqLensHw_getHDCIrisParams(AiqLensHw_t* pLensHw, int* adc)
{
    ENTER_CAMHW_FUNCTION();
    aiqMutex_lock(&pLensHw->_mutex);

    if (!pLensHw->_iris_enable) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "iris is not supported");
        aiqMutex_unlock(&pLensHw->_mutex);
        return XCAM_RETURN_ERROR_FAILED;
    }

    int iris_adc = 0;
    // No work for now
    /*
    if (pLensHw->mSd._v4l_base.io_control(&pLensHw->mSd._v4l_base, RK_VIDIOC_GET_DCIRIS_HALL_ADC, &iris_adc) < 0) {
        LOGD_CAMHW_SUBM(LENS_SUBM, "failed to get iris adc");
        *adc = 0;
        aiqMutex_unlock(&pLensHw->_mutex);
        return XCAM_RETURN_ERROR_IOCTL;
    }
    */
    *adc = iris_adc;
    LOGD("|||get dciris adc value=%d\n", iris_adc);

    EXIT_CAMHW_FUNCTION();
    aiqMutex_unlock(&pLensHw->_mutex);
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqLensHw_getPIrisParams(AiqLensHw_t* pLensHw, int* step)
{
    ENTER_CAMHW_FUNCTION();
    aiqMutex_lock(&pLensHw->_mutex);
    struct v4l2_control control;

    if (!pLensHw->_iris_enable) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "iris is not supported");
        aiqMutex_unlock(&pLensHw->_mutex);
        return XCAM_RETURN_ERROR_FAILED;
    }

    xcam_mem_clear (control);
    control.id = V4L2_CID_IRIS_ABSOLUTE;

    if (pLensHw->mSd._v4l_base.io_control(&pLensHw->mSd._v4l_base, VIDIOC_G_CTRL, &control) < 0) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "get iris result failed");
        aiqMutex_unlock(&pLensHw->_mutex);
        return XCAM_RETURN_ERROR_IOCTL;
    }
    *step = control.value;
    LOGD_CAMHW_SUBM(LENS_SUBM, "|||get iris result: %d, control.value %d", *step, control.value);

    EXIT_CAMHW_FUNCTION();
    aiqMutex_unlock(&pLensHw->_mutex);
    return XCAM_RETURN_NO_ERROR;
}


XCamReturn AiqLensHw_setZoomParams(AiqLensHw_t* pLensHw, int position)
{
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqLensHw_setZoomFocusRebackSync(AiqLensHw_t* pLensHw, rk_aiq_focus_params_t* attrPtr,
                                            bool is_update_time)
{
    ENTER_CAMHW_FUNCTION();
    struct rk_cam_vcm_tim zoomtim, zoom1tim, focustim;
    struct rk_cam_set_zoom set_zoom;
    struct v4l2_control control;
    unsigned long time0, time1, time2;
    int zoom_pos = 0, zoom1_pos = 0, focus_pos = 0;

#ifdef DISABLE_ZOOM_FOCUS
    return XCAM_RETURN_NO_ERROR;
#endif

    if (!pLensHw->_zoom_enable || !pLensHw->_focus_enable) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "zoom or focus is not supported");
        return XCAM_RETURN_NO_ERROR;
    }

    xcam_mem_clear (set_zoom);
    set_zoom.setzoom_cnt = 1;
    if (attrPtr->send_zoom_reback) set_zoom.is_need_zoom_reback = true;
    if (attrPtr->send_zoom1_reback) set_zoom.is_need_zoom1_reback = true;
    if (attrPtr->send_focus_reback) set_zoom.is_need_focus_reback = true;
    aiqMutex_lock(&pLensHw->_mutex);
    zoom_pos = pLensHw->_zoom_pos;
    zoom1_pos = pLensHw->_zoom1_pos;
    focus_pos = pLensHw->_focus_pos;
    aiqMutex_unlock(&pLensHw->_mutex);
    set_zoom.zoom_pos[0].zoom_pos = zoom_pos;
    set_zoom.zoom_pos[0].zoom1_pos = zoom1_pos;
    set_zoom.zoom_pos[0].focus_pos = focus_pos;

    if (pLensHw->mSd._v4l_base.io_control(&pLensHw->mSd._v4l_base, RK_VIDIOC_ZOOM_SET_POSITION, &set_zoom) < 0) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "set zoom position failed");
        return XCAM_RETURN_ERROR_IOCTL;
    }

    if (pLensHw->mSd._v4l_base.io_control(&pLensHw->mSd._v4l_base, RK_VIDIOC_ZOOM_TIMEINFO, &zoomtim) < 0) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "get zoom timeinfo failed");
        aiqMutex_lock(&pLensHw->_mutex);
        zoomtim = pLensHw->_zoom_tim;
        zoomtim.vcm_end_t.tv_sec += 1;
        aiqMutex_unlock(&pLensHw->_mutex);
    }

    if (pLensHw->_zoom1_enable) {
        if (pLensHw->mSd._v4l_base.io_control(&pLensHw->mSd._v4l_base, RK_VIDIOC_ZOOM1_TIMEINFO, &zoom1tim) < 0) {
            LOGE_CAMHW_SUBM(LENS_SUBM, "get zoom1 timeinfo failed");
            aiqMutex_lock(&pLensHw->_mutex);
            zoom1tim = pLensHw->_zoom1_tim;
            zoom1tim.vcm_end_t.tv_sec += 1;
            aiqMutex_unlock(&pLensHw->_mutex);
        }
    }

    if (pLensHw->mSd._v4l_base.io_control(&pLensHw->mSd._v4l_base, RK_VIDIOC_VCM_TIMEINFO, &focustim) < 0) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "get focus timeinfo failed");
        aiqMutex_lock(&pLensHw->_mutex);
        focustim = pLensHw->_focus_tim;
        focustim.vcm_end_t.tv_sec += 1;
        aiqMutex_unlock(&pLensHw->_mutex);
    }

    time0 = zoomtim.vcm_end_t.tv_sec * 1000 + zoomtim.vcm_end_t.tv_usec / 1000;
    time1 = focustim.vcm_end_t.tv_sec * 1000 + focustim.vcm_end_t.tv_usec / 1000;
    time2 = zoom1tim.vcm_end_t.tv_sec * 1000 + zoom1tim.vcm_end_t.tv_usec / 1000;
    if (time1 > time0) {
        zoomtim = focustim;
        time0 = time1;
    }
    if ((time2 > time0) && pLensHw->_zoom1_enable)
        zoomtim = zoom1tim;

    aiqMutex_lock(&pLensHw->_mutex);
    if (is_update_time) {
        pLensHw->_zoom_tim = zoomtim;
        pLensHw->_zoom1_tim = zoom1tim;
    }
    pLensHw->_zoom_pos = zoom_pos;
    pLensHw->_zoom1_pos = zoom1_pos;
    pLensHw->_focus_pos = focus_pos;
    aiqMutex_unlock(&pLensHw->_mutex);

    time0 = pLensHw->_zoom_tim.vcm_end_t.tv_sec * 1000 + pLensHw->_zoom_tim.vcm_end_t.tv_usec / 1000;
    LOGD_CAMHW_SUBM(LENS_SUBM, "zoom_pos %d, zoom1_pos %d, focus_pos %d, is_need_zoom_reback %d, is_need_focus_reback %d, end time %ld, is_update_time %d",
                    zoom_pos, zoom1_pos, focus_pos, set_zoom.is_need_zoom_reback, set_zoom.is_need_focus_reback, time0, is_update_time);

    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqLensHw_endZoomChgSync(AiqLensHw_t* pLensHw, rk_aiq_focus_params_t* attrPtr,
                                    bool is_update_time)
{
    ENTER_CAMHW_FUNCTION();
    struct rk_cam_vcm_tim zoomtim, zoom1tim, focustim;
    struct rk_cam_set_zoom set_zoom;
    struct v4l2_control control;
    unsigned long time0, time1, time2;
    int zoom_pos = 0, zoom1_pos = 0, focus_pos = 0;

#ifdef DISABLE_ZOOM_FOCUS
    return XCAM_RETURN_NO_ERROR;
#endif

    if (!pLensHw->_zoom_enable || !pLensHw->_focus_enable) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "zoom or focus is not supported");
        return XCAM_RETURN_NO_ERROR;
    }

    xcam_mem_clear (set_zoom);
    if (attrPtr->end_zoom_chg) {
        set_zoom.setzoom_cnt = 1;
        aiqMutex_lock(&pLensHw->_mutex);
        zoom_pos = pLensHw->_zoom_pos;
        zoom1_pos = pLensHw->_zoom1_pos;
        focus_pos = pLensHw->_focus_pos;
        aiqMutex_unlock(&pLensHw->_mutex);
        set_zoom.zoom_pos[0].zoom_pos = zoom_pos;
        set_zoom.zoom_pos[0].zoom1_pos = zoom1_pos;
        set_zoom.zoom_pos[0].focus_pos = focus_pos;

        LOGD_CAMHW_SUBM(LENS_SUBM, "zoom_pos %d, focus_pos %d, _last_zoomchg_zoom %d, _last_zoomchg_focus %d",
                        zoom_pos, focus_pos, pLensHw->_last_zoomchg_zoom, pLensHw->_last_zoomchg_focus);
        if (zoom_pos < pLensHw->_last_zoomchg_zoom)
            set_zoom.is_need_zoom_reback = true;
        else
            set_zoom.is_need_zoom_reback = false;

        if (pLensHw->_zoom1_enable) {
            if (zoom1_pos < pLensHw->_last_zoomchg_zoom1)
                set_zoom.is_need_zoom1_reback = true;
            else
                set_zoom.is_need_zoom1_reback = false;
        }

        if (focus_pos < pLensHw->_last_zoomchg_focus)
            set_zoom.is_need_focus_reback = true;
        else
            set_zoom.is_need_focus_reback = false;

        pLensHw->_last_zoomchg_zoom = zoom_pos;
        pLensHw->_last_zoomchg_zoom1 = zoom1_pos;
        pLensHw->_last_zoomchg_focus = focus_pos;

        if (pLensHw->mSd._v4l_base.io_control(&pLensHw->mSd._v4l_base, RK_VIDIOC_ZOOM_SET_POSITION, &set_zoom) < 0) {
            LOGE_CAMHW_SUBM(LENS_SUBM, "set zoom position failed");
            return XCAM_RETURN_ERROR_IOCTL;
        }

        if (pLensHw->mSd._v4l_base.io_control(&pLensHw->mSd._v4l_base, RK_VIDIOC_ZOOM_TIMEINFO, &zoomtim) < 0) {
            LOGE_CAMHW_SUBM(LENS_SUBM, "get zoom timeinfo failed");
            aiqMutex_lock(&pLensHw->_mutex);
            zoomtim = pLensHw->_zoom_tim;
            zoomtim.vcm_end_t.tv_sec += 1;
            aiqMutex_unlock(&pLensHw->_mutex);
        }

        if (pLensHw->_zoom1_enable) {
            if (pLensHw->mSd._v4l_base.io_control(&pLensHw->mSd._v4l_base, RK_VIDIOC_ZOOM1_TIMEINFO, &zoom1tim) < 0) {
                LOGE_CAMHW_SUBM(LENS_SUBM, "get zoom1 timeinfo failed");
                aiqMutex_lock(&pLensHw->_mutex);
                zoom1tim = pLensHw->_zoom1_tim;
                zoom1tim.vcm_end_t.tv_sec += 1;
                aiqMutex_unlock(&pLensHw->_mutex);
            }
        }

        if (pLensHw->mSd._v4l_base.io_control(&pLensHw->mSd._v4l_base, RK_VIDIOC_VCM_TIMEINFO, &focustim) < 0) {
            LOGE_CAMHW_SUBM(LENS_SUBM, "get focus timeinfo failed");
            aiqMutex_lock(&pLensHw->_mutex);
            focustim = pLensHw->_focus_tim;
            focustim.vcm_end_t.tv_sec += 1;
            aiqMutex_unlock(&pLensHw->_mutex);
        }

        time0 = zoomtim.vcm_end_t.tv_sec * 1000 + zoomtim.vcm_end_t.tv_usec / 1000;
        time1 = focustim.vcm_end_t.tv_sec * 1000 + focustim.vcm_end_t.tv_usec / 1000;
        time2 = zoom1tim.vcm_end_t.tv_sec * 1000 + zoom1tim.vcm_end_t.tv_usec / 1000;
        if (time1 > time0) {
            zoomtim = focustim;
            time0 = time1;
        }
        if ((time2 > time0) && pLensHw->_zoom1_enable)
            zoomtim = zoom1tim;

        aiqMutex_lock(&pLensHw->_mutex);
        if (is_update_time) {
            pLensHw->_zoom_tim = zoomtim;
            pLensHw->_zoom1_tim = zoom1tim;
        }
        pLensHw->_zoom_pos = zoom_pos;
        pLensHw->_zoom1_pos = zoom1_pos;
        pLensHw->_focus_pos = focus_pos;
        aiqMutex_unlock(&pLensHw->_mutex);

        time0 = pLensHw->_zoom_tim.vcm_end_t.tv_sec * 1000 + pLensHw->_zoom_tim.vcm_end_t.tv_usec / 1000;
        LOGD_CAMHW_SUBM(LENS_SUBM, "zoom_pos %d, zoom1_pos %d, focus_pos %d, is_need_zoom_reback %d, is_need_focus_reback %d, end time %ld, is_update_time %d",
                        zoom_pos, zoom1_pos, focus_pos, set_zoom.is_need_zoom_reback, set_zoom.is_need_focus_reback, time0, is_update_time);
    }

    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;

}

XCamReturn AiqLensHw_startZoomChgSync(AiqLensHw_t* pLensHw, rk_aiq_focus_params_t* attrPtr,
                                    bool is_update_time)
{
    ENTER_CAMHW_FUNCTION();
    struct rk_cam_vcm_tim zoomtim, zoom1tim, focustim;
    struct rk_cam_set_zoom set_zoom;
    struct v4l2_control control;
    unsigned long time0, time1, time2;
    int zoom_pos = 0, zoom1_pos = 0, focus_pos = 0;

#ifdef DISABLE_ZOOM_FOCUS
    return XCAM_RETURN_NO_ERROR;
#endif

    if (!pLensHw->_zoom_enable || !pLensHw->_focus_enable) {
        LOGE_AF("zoom or focus is not supported");
        return XCAM_RETURN_NO_ERROR;
    }

    xcam_mem_clear(set_zoom);
    if (attrPtr->IsNeedCkRebackAtStart) {
        set_zoom.setzoom_cnt = 1;
        aiqMutex_lock(&pLensHw->_mutex);
        zoom_pos = pLensHw->_zoom_pos;
        zoom1_pos = pLensHw->_zoom1_pos;
        focus_pos =pLensHw->_focus_pos;
        aiqMutex_unlock(&pLensHw->_mutex);
        set_zoom.zoom_pos[0].zoom_pos = zoom_pos;
        set_zoom.zoom_pos[0].zoom1_pos = zoom1_pos;
        set_zoom.zoom_pos[0].focus_pos = focus_pos;

        LOGD_CAMHW_SUBM(LENS_SUBM, "zoom_pos %d, focus_pos %d, _last_zoomchg_zoom %d, _last_zoomchg_focus %d",
                 zoom_pos, focus_pos, pLensHw->_last_zoomchg_zoom, pLensHw->_last_zoomchg_focus);
        if (zoom_pos < pLensHw->_last_zoomchg_zoom)
            set_zoom.is_need_zoom_reback = true;
        else
            set_zoom.is_need_zoom_reback = false;

        if (pLensHw->_zoom1_enable) {
            if (zoom1_pos < pLensHw->_last_zoomchg_zoom1)
                set_zoom.is_need_zoom1_reback = true;
            else
                set_zoom.is_need_zoom1_reback = false;
        }

        if (focus_pos < pLensHw->_last_zoomchg_focus)
            set_zoom.is_need_focus_reback = true;
        else
            set_zoom.is_need_focus_reback = false;

        if (!set_zoom.is_need_zoom_reback && !set_zoom.is_need_zoom1_reback && !set_zoom.is_need_focus_reback) {
            LOGI_AF("no need handle reback, return.\n");
            return XCAM_RETURN_NO_ERROR;
        }

        pLensHw->_last_zoomchg_zoom = zoom_pos;
        pLensHw->_last_zoomchg_zoom1 = zoom1_pos;
        pLensHw->_last_zoomchg_focus = focus_pos;

        if (pLensHw->mSd._v4l_base.io_control(&pLensHw->mSd._v4l_base, RK_VIDIOC_ZOOM_SET_POSITION, &set_zoom) < 0) {
            LOGE_AF("set zoom position failed");
            return XCAM_RETURN_ERROR_IOCTL;
        }

        if (pLensHw->mSd._v4l_base.io_control(&pLensHw->mSd._v4l_base, RK_VIDIOC_ZOOM_TIMEINFO, &zoomtim) < 0) {
            LOGE_AF("get zoom timeinfo failed");
            aiqMutex_lock(&pLensHw->_mutex);
            zoomtim = pLensHw->_zoom_tim;
            zoomtim.vcm_end_t.tv_sec += 1;
            aiqMutex_unlock(&pLensHw->_mutex);
        }

        if (pLensHw->_zoom1_enable) {
            if (pLensHw->mSd._v4l_base.io_control(&pLensHw->mSd._v4l_base, RK_VIDIOC_ZOOM1_TIMEINFO, &zoom1tim) < 0) {
                LOGE_CAMHW_SUBM(LENS_SUBM, "get zoom1 timeinfo failed");
                aiqMutex_lock(&pLensHw->_mutex);
                zoom1tim = pLensHw->_zoom1_tim;
                zoom1tim.vcm_end_t.tv_sec += 1;
                aiqMutex_unlock(&pLensHw->_mutex);
            }
        }

        if (pLensHw->mSd._v4l_base.io_control(&pLensHw->mSd._v4l_base, RK_VIDIOC_VCM_TIMEINFO, &focustim) < 0) {
            LOGE_AF("get focus timeinfo failed");
            aiqMutex_lock(&pLensHw->_mutex);
            focustim = pLensHw->_focus_tim;
            focustim.vcm_end_t.tv_sec += 1;
            aiqMutex_unlock(&pLensHw->_mutex);
        }

        time0 = zoomtim.vcm_end_t.tv_sec * 1000 + zoomtim.vcm_end_t.tv_usec / 1000;
        time1 = focustim.vcm_end_t.tv_sec * 1000 + focustim.vcm_end_t.tv_usec / 1000;
        time2 = zoom1tim.vcm_end_t.tv_sec * 1000 + zoom1tim.vcm_end_t.tv_usec / 1000;
        if (time1 > time0) {
            zoomtim = focustim;
            time0 = time1;
        }
        if ((time2 > time0) && pLensHw->_zoom1_enable)
            zoomtim = zoom1tim;

        aiqMutex_lock(&pLensHw->_mutex);
        if (is_update_time) {
            pLensHw->_zoom_tim = zoomtim;
            pLensHw->_zoom1_tim = zoom1tim;
        }
        pLensHw->_zoom_pos = zoom_pos;
        pLensHw->_zoom1_pos = zoom1_pos;
        pLensHw->_focus_pos = focus_pos;
        aiqMutex_unlock(&pLensHw->_mutex);

        time0 = pLensHw->_zoom_tim.vcm_end_t.tv_sec * 1000 + pLensHw->_zoom_tim.vcm_end_t.tv_usec / 1000;
        LOGD_CAMHW_SUBM(LENS_SUBM, "start sync: zoom_pos %d, zoom1_pos %d, focus_pos %d, zoom focus move end time %ld, is_need_zoom_reback %d, is_need_focus_reback %d",
                zoom_pos, zoom1_pos, focus_pos, time0, set_zoom.is_need_zoom_reback, set_zoom.is_need_focus_reback);
    }

    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqLensHw_setZoomFocusParamsSync(AiqLensHw_t* pLensHw, rk_aiq_focus_params_t* attrPtr,
                                            bool is_update_time)
{
    ENTER_CAMHW_FUNCTION();
    struct rk_cam_vcm_tim zoomtim, zoom1tim, focustim;
    struct rk_cam_set_zoom set_zoom;
    struct v4l2_control control;
    unsigned long time0, time1, time2;
    int zoom_pos = 0, zoom1_pos = 0, focus_pos = 0;

#ifdef DISABLE_ZOOM_FOCUS
    return XCAM_RETURN_NO_ERROR;
#endif

    if (!pLensHw->_zoom_enable || !pLensHw->_focus_enable) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "zoom or focus is not supported");
        return XCAM_RETURN_NO_ERROR;
    }

    if (!attrPtr->lens_pos_valid && !attrPtr->zoom_pos_valid) {
        return XCAM_RETURN_NO_ERROR;
    }

    xcam_mem_clear (set_zoom);
    if (attrPtr->lens_pos_valid || attrPtr->zoom_pos_valid) {
        set_zoom.setzoom_cnt = attrPtr->next_pos_num;
        set_zoom.is_need_zoom_reback = false;
        set_zoom.is_need_zoom1_reback = false;
        set_zoom.is_need_focus_reback = false;
        for (unsigned int i = 0; i < set_zoom.setzoom_cnt; i++) {
            zoom_pos = attrPtr->next_zoom_pos[i];
            zoom1_pos = attrPtr->next_zoom1_pos[i];
            focus_pos = attrPtr->next_lens_pos[i];

            if (zoom_pos < pLensHw->_zoom_query.minimum)
                zoom_pos = pLensHw->_zoom_query.minimum;
            if (zoom_pos > pLensHw->_zoom_query.maximum)
                zoom_pos = pLensHw->_zoom_query.maximum;

            if (pLensHw->_zoom1_enable) {
                if (zoom1_pos < pLensHw->_zoom1_query.minimum)
                    zoom1_pos = pLensHw->_zoom1_query.minimum;
                if (zoom1_pos > pLensHw->_zoom1_query.maximum)
                    zoom1_pos = pLensHw->_zoom1_query.maximum;
            }

            if (focus_pos < pLensHw->_focus_query.minimum)
                focus_pos = pLensHw->_focus_query.minimum;
            if (focus_pos > pLensHw->_focus_query.maximum)
                focus_pos = pLensHw->_focus_query.maximum;

            set_zoom.zoom_pos[i].zoom_pos = zoom_pos;
            set_zoom.zoom_pos[i].zoom1_pos = zoom1_pos;
            set_zoom.zoom_pos[i].focus_pos = focus_pos;
            LOGD_CAMHW_SUBM(LENS_SUBM, "i %d, zoom_pos %d, zoom1_pos %d, focus_pos %d\n", i, zoom_pos, zoom1_pos, focus_pos);
        }

        if (pLensHw->mSd._v4l_base.io_control(&pLensHw->mSd._v4l_base, RK_VIDIOC_ZOOM_SET_POSITION, &set_zoom) < 0) {
            LOGE_CAMHW_SUBM(LENS_SUBM, "set zoom position failed");
            return XCAM_RETURN_ERROR_IOCTL;
        }

        if (pLensHw->mSd._v4l_base.io_control(&pLensHw->mSd._v4l_base, RK_VIDIOC_ZOOM_TIMEINFO, &zoomtim) < 0) {
            LOGE_CAMHW_SUBM(LENS_SUBM, "get zoom timeinfo failed");
            aiqMutex_lock(&pLensHw->_mutex);
            zoomtim = pLensHw->_zoom_tim;
            zoomtim.vcm_end_t.tv_sec += 1;
            aiqMutex_unlock(&pLensHw->_mutex);
        }

        if (pLensHw->_zoom1_enable) {
            if (pLensHw->mSd._v4l_base.io_control(&pLensHw->mSd._v4l_base, RK_VIDIOC_ZOOM1_TIMEINFO, &zoom1tim) < 0) {
                LOGE_CAMHW_SUBM(LENS_SUBM, "get zoom1 timeinfo failed");
                aiqMutex_lock(&pLensHw->_mutex);
                zoom1tim = pLensHw->_zoom1_tim;
                zoom1tim.vcm_end_t.tv_sec += 1;
                aiqMutex_unlock(&pLensHw->_mutex);
            }
        }

        if (pLensHw->mSd._v4l_base.io_control(&pLensHw->mSd._v4l_base, RK_VIDIOC_VCM_TIMEINFO, &focustim) < 0) {
            LOGE_CAMHW_SUBM(LENS_SUBM, "get focus timeinfo failed");
            aiqMutex_lock(&pLensHw->_mutex);
            focustim = pLensHw->_focus_tim;
            focustim.vcm_end_t.tv_sec += 1;
            aiqMutex_unlock(&pLensHw->_mutex);
        }

        time0 = zoomtim.vcm_end_t.tv_sec * 1000 + zoomtim.vcm_end_t.tv_usec / 1000;
        time1 = focustim.vcm_end_t.tv_sec * 1000 + focustim.vcm_end_t.tv_usec / 1000;
        time2 = zoom1tim.vcm_end_t.tv_sec * 1000 + zoom1tim.vcm_end_t.tv_usec / 1000;
        if (time1 > time0) {
            zoomtim = focustim;
            time0 = time1;
        }
        if ((time2 > time0) && pLensHw->_zoom1_enable)
            zoomtim = zoom1tim;

        aiqMutex_lock(&pLensHw->_mutex);
        if (is_update_time) {
            pLensHw->_zoom_tim = zoomtim;
            pLensHw->_zoom1_tim = zoom1tim;
        }
        pLensHw->_zoom_pos = zoom_pos;
        pLensHw->_zoom1_pos = zoom1_pos;
        pLensHw->_focus_pos = focus_pos;
        aiqMutex_unlock(&pLensHw->_mutex);

        time0 = pLensHw->_zoom_tim.vcm_end_t.tv_sec * 1000 + pLensHw->_zoom_tim.vcm_end_t.tv_usec / 1000;
        LOGD_CAMHW_SUBM(LENS_SUBM, "zoom_pos %d, zoom1_pos %d, focus_pos %d, zoom focus move end time %ld, is_need_zoom_reback %d, is_need_zoom1_reback %d, is_need_focus_reback %d, is_update_time %d",
                        zoom_pos, zoom1_pos, focus_pos, time0, set_zoom.is_need_zoom_reback, set_zoom.is_need_zoom1_reback, set_zoom.is_need_focus_reback, is_update_time);
    }

    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;

}

XCamReturn AiqLensHw_setZoomFocusParams(AiqLensHw_t* pLensHw, rk_aiq_focus_params_t* focus_params)
{
    ENTER_CAMHW_FUNCTION();
    aiqMutex_lock(&pLensHw->_mutex);
    rk_aiq_focus_params_t* p_focus = focus_params;
    int zoom_pos = 0, zoom1_pos = 0, focus_pos = 0;

    if (!pLensHw->_zoom_enable || !pLensHw->_focus_enable) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "zoom or focus is not supported");
        aiqMutex_unlock(&pLensHw->_mutex);
        return XCAM_RETURN_NO_ERROR;
    }

    if (!pLensHw->mSd._v4l_base._active)
        _start_internal(pLensHw);

#ifdef DISABLE_ZOOM_FOCUS
    aiqMutex_unlock(&pLensHw->_mutex);
    return XCAM_RETURN_NO_ERROR;
#endif

    rk_aiq_focus_params_t attrPtrParams;

    memset(&attrPtrParams, 0, sizeof(rk_aiq_focus_params_t));
    rk_aiq_focus_params_t* attrPtr = &attrPtrParams;
    attrPtr->zoomfocus_modifypos = false;
    attrPtr->focus_correction = false;
    attrPtr->zoom_correction = false;
    attrPtr->lens_pos_valid = p_focus->lens_pos_valid;
    attrPtr->zoom_pos_valid = p_focus->zoom_pos_valid;
    attrPtr->send_zoom_reback = p_focus->send_zoom_reback;
    attrPtr->send_zoom1_reback = p_focus->send_zoom1_reback;
    attrPtr->send_focus_reback = p_focus->send_focus_reback;
    attrPtr->end_zoom_chg = p_focus->end_zoom_chg;
    attrPtr->IsNeedCkRebackAtStart = p_focus->IsNeedCkRebackAtStart;
    attrPtr->focus_noreback = false;
    if (attrPtr->lens_pos_valid || attrPtr->zoom_pos_valid) {
        attrPtr->next_pos_num = p_focus->next_pos_num;
        for (int i = 0; i < attrPtr->next_pos_num; i++) {
            zoom_pos = p_focus->next_zoom_pos[i];
            zoom1_pos = p_focus->next_zoom1_pos[i];
            focus_pos = p_focus->next_lens_pos[i];

            if (zoom_pos < pLensHw->_zoom_query.minimum)
                zoom_pos = pLensHw->_zoom_query.minimum;
            if (zoom_pos > pLensHw->_zoom_query.maximum)
                zoom_pos = pLensHw->_zoom_query.maximum;

            if (pLensHw->_zoom1_enable) {
                if (zoom1_pos < pLensHw->_zoom1_query.minimum)
                    zoom1_pos = pLensHw->_zoom1_query.minimum;
                if (zoom1_pos > pLensHw->_zoom1_query.maximum)
                    zoom1_pos = pLensHw->_zoom1_query.maximum;
            }

            if (focus_pos < pLensHw->_focus_query.minimum)
                focus_pos = pLensHw->_focus_query.minimum;
            if (focus_pos > pLensHw->_focus_query.maximum)
                focus_pos = pLensHw->_focus_query.maximum;

            attrPtr->next_zoom_pos[i] = zoom_pos;
            attrPtr->next_zoom1_pos[i] = zoom1_pos;
            attrPtr->next_lens_pos[i] = focus_pos;
        }

        LOGD_CAMHW_SUBM(LENS_SUBM, "zoom_pos %d, zoom1_pos %d, focus_pos %d", zoom_pos, zoom1_pos, focus_pos);
        _LensHwHelperThd_push_attr(&pLensHw->_lenshw_thd, attrPtr);
    } else if (attrPtr->send_zoom_reback || attrPtr->send_zoom1_reback || attrPtr->send_focus_reback) {
        LOGD_CAMHW_SUBM(LENS_SUBM, "send reback zoom_pos %d, zoom1_pos %d, focus_pos %d", pLensHw->_zoom_pos, pLensHw->_zoom1_pos, pLensHw->_focus_pos);
        _LensHwHelperThd_push_attr(&pLensHw->_lenshw_thd, attrPtr);
    } else if (attrPtr->end_zoom_chg) {
        LOGD_CAMHW_SUBM(LENS_SUBM, "end_zoom_chg zoom_pos %d, zoom1_pos %d, focus_pos %d, next_pos_num %d",
                        pLensHw->_zoom_pos, pLensHw->_zoom1_pos, pLensHw->_focus_pos, attrPtr->next_pos_num);
        _LensHwHelperThd_push_attr(&pLensHw->_lenshw_thd, attrPtr);
    } else if (attrPtr->IsNeedCkRebackAtStart) {
        LOGD_CAMHW_SUBM(LENS_SUBM, "IsNeedCkRebackAtStart zoom_pos %d, zoom1_pos %d, focus_pos %d, next_pos_num %d",
                        pLensHw->_zoom_pos, pLensHw->_zoom1_pos, pLensHw->_focus_pos, attrPtr->next_pos_num);
        _LensHwHelperThd_push_attr(&pLensHw->_lenshw_thd, attrPtr);
    }

    EXIT_CAMHW_FUNCTION();
    aiqMutex_unlock(&pLensHw->_mutex);
    return XCAM_RETURN_NO_ERROR;

}

XCamReturn AiqLensHw_getFocusParams(AiqLensHw_t* pLensHw, int* position)
{
    ENTER_CAMHW_FUNCTION();
    aiqMutex_lock(&pLensHw->_mutex);
    struct v4l2_control control;

    if (!pLensHw->_focus_enable) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "focus is not supported");
        aiqMutex_unlock(&pLensHw->_mutex);
        return XCAM_RETURN_ERROR_FAILED;
    }

    xcam_mem_clear (control);
    control.id = V4L2_CID_FOCUS_ABSOLUTE;

    if (pLensHw->mSd._v4l_base.io_control(&pLensHw->mSd._v4l_base, VIDIOC_G_CTRL, &control) < 0) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "get focus result failed");
        aiqMutex_unlock(&pLensHw->_mutex);
        return XCAM_RETURN_ERROR_IOCTL;
    }
    *position = control.value;
    LOGD_CAMHW_SUBM(LENS_SUBM, "|||get focus result: %d, control.value %d", *position, control.value);

    pLensHw->_focus_pos = *position;

    EXIT_CAMHW_FUNCTION();
    aiqMutex_unlock(&pLensHw->_mutex);
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqLensHw_getZoomParams(AiqLensHw_t* pLensHw, int* position)
{
    ENTER_CAMHW_FUNCTION();
    aiqMutex_lock(&pLensHw->_mutex);
    //struct v4l2_control control;

    if (!pLensHw->_zoom_enable) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "zoom is not supported");
        aiqMutex_unlock(&pLensHw->_mutex);
        return XCAM_RETURN_ERROR_FAILED;
    }
#if 0
    xcam_mem_clear (control);
    control.id = V4L2_CID_ZOOM_ABSOLUTE;

    if (pLensHw->mSd._v4l_base.io_control(&pLensHw->mSd._v4l_base, VIDIOC_G_CTRL, &control) < 0) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "get zoom result failed");
        aiqMutex_unlock(&pLensHw->_mutex);
        return XCAM_RETURN_ERROR_IOCTL;
    }
    *position = control.value;
    LOGD_CAMHW_SUBM(LENS_SUBM, "|||get zoom result: %d, control.value %d", *position, control.value);

    pLensHw->_zoom_pos = *position;
#else
    *position = pLensHw->_zoom_pos;
#endif
    LOGD_CAMHW_SUBM(LENS_SUBM, "*position %d", *position);

    EXIT_CAMHW_FUNCTION();
    aiqMutex_unlock(&pLensHw->_mutex);
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqLensHw_FocusCorrectionSync(AiqLensHw_t* pLensHw)
{
    ENTER_CAMHW_FUNCTION();
    int correction = 0;

    if (!pLensHw->_focus_enable) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "focus is not supported");
        return XCAM_RETURN_ERROR_FAILED;
    }

    LOGD_CAMHW_SUBM(LENS_SUBM, "focus_correction start");
    if (pLensHw->mSd._v4l_base.io_control(&pLensHw->mSd._v4l_base, RK_VIDIOC_FOCUS_CORRECTION, &correction) < 0) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "focus correction failed");
        return XCAM_RETURN_ERROR_IOCTL;
    }

    aiqMutex_lock(&pLensHw->_mutex);
    pLensHw->_focus_pos = 0;
    pLensHw->_last_zoomchg_focus = 0;
    pLensHw->_focus_correction = false;
    aiqMutex_unlock(&pLensHw->_mutex);
    LOGD_CAMHW_SUBM(LENS_SUBM, "focus_correction end");

    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

bool AiqLensHw_IsFocusInCorrectionSync(AiqLensHw_t* pLensHw)
{
    ENTER_CAMHW_FUNCTION();
    bool in_correction;

    if (!pLensHw->_zoom_enable) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "zoom is not supported");
        return false;
    }

    aiqMutex_lock(&pLensHw->_mutex);
    in_correction = pLensHw->_focus_correction;
    aiqMutex_unlock(&pLensHw->_mutex);

    EXIT_CAMHW_FUNCTION();
    return in_correction;
}

XCamReturn AiqLensHw_FocusCorrection(AiqLensHw_t* pLensHw)
{
    ENTER_CAMHW_FUNCTION();
    aiqMutex_lock(&pLensHw->_mutex);

    if (!pLensHw->_focus_enable) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "focus is not supported");
        return XCAM_RETURN_NO_ERROR;
    }

    if (!pLensHw->mSd._v4l_base._active)
        _start_internal(pLensHw);

#ifdef DISABLE_ZOOM_FOCUS
    aiqMutex_unlock(&pLensHw->_mutex);
    return XCAM_RETURN_NO_ERROR;
#endif

    rk_aiq_focus_params_t attrPtr;

    attrPtr.zoomfocus_modifypos = false;
    attrPtr.focus_correction = true;
    attrPtr.zoom_correction = false;

    LOGD_CAMHW_SUBM(LENS_SUBM, "focus_correction");
    pLensHw->_focus_correction = true;
    _LensHwHelperThd_push_attr(&pLensHw->_lenshw_thd, &attrPtr);

    EXIT_CAMHW_FUNCTION();
    aiqMutex_unlock(&pLensHw->_mutex);
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqLensHw_ZoomCorrectionSync(AiqLensHw_t* pLensHw)
{
    ENTER_CAMHW_FUNCTION();
    int correction = 0;

    if (!pLensHw->_zoom_enable) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "zoom is not supported");
        return XCAM_RETURN_ERROR_FAILED;
    }

    LOGD_CAMHW_SUBM(LENS_SUBM, "zoom_correction start");
    if (pLensHw->mSd._v4l_base.io_control(&pLensHw->mSd._v4l_base, RK_VIDIOC_ZOOM_CORRECTION, &correction) < 0) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "zoom correction failed");
        return XCAM_RETURN_ERROR_IOCTL;
    }

    if (pLensHw->_zoom1_enable) {
        if (pLensHw->mSd._v4l_base.io_control(&pLensHw->mSd._v4l_base, RK_VIDIOC_ZOOM1_CORRECTION, &correction) < 0) {
            LOGE_CAMHW_SUBM(LENS_SUBM, "zoom correction failed");
            return XCAM_RETURN_ERROR_IOCTL;
        }
    }

    aiqMutex_lock(&pLensHw->_mutex);
    pLensHw->_zoom_pos = 0;
    pLensHw->_zoom1_pos = 0;
    pLensHw->_last_zoomchg_zoom = 0;
    pLensHw->_last_zoomchg_zoom1 = 0;
    pLensHw->_zoom_correction = false;
    aiqMutex_unlock(&pLensHw->_mutex);
    LOGD_CAMHW_SUBM(LENS_SUBM, "zoom_correction end");

    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

bool AiqLensHw_IsZoomInCorrectionSync(AiqLensHw_t* pLensHw)
{
    ENTER_CAMHW_FUNCTION();
    bool in_correction;

    if (!pLensHw->_zoom_enable) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "zoom is not supported");
        return false;
    }

    aiqMutex_lock(&pLensHw->_mutex);
    in_correction = pLensHw->_zoom_correction;
    aiqMutex_unlock(&pLensHw->_mutex);

    EXIT_CAMHW_FUNCTION();
    return in_correction;
}

XCamReturn AiqLensHw_ZoomCorrection(AiqLensHw_t* pLensHw)
{
    ENTER_CAMHW_FUNCTION();
    aiqMutex_lock(&pLensHw->_mutex);

    if (!pLensHw->_zoom_enable) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "focus is not supported");
        aiqMutex_unlock(&pLensHw->_mutex);
        return XCAM_RETURN_NO_ERROR;
    }

    if (!pLensHw->mSd._v4l_base._active)
        _start_internal(pLensHw);

#ifdef DISABLE_ZOOM_FOCUS
    aiqMutex_unlock(&pLensHw->_mutex);
    return XCAM_RETURN_NO_ERROR;
#endif

    rk_aiq_focus_params_t attrPtr;

    attrPtr.zoomfocus_modifypos = false;
    attrPtr.zoom_correction = true;
    attrPtr.focus_correction = false;

    LOGD_CAMHW_SUBM(LENS_SUBM, "zoom_correction");
    pLensHw->_zoom_correction = true;
    _LensHwHelperThd_push_attr(&pLensHw->_lenshw_thd1, &attrPtr);

    EXIT_CAMHW_FUNCTION();
    aiqMutex_unlock(&pLensHw->_mutex);
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqLensHw_ZoomFocusModifyPositionSync(AiqLensHw_t* pLensHw,
                                                 rk_aiq_focus_params_t* attrPtr)
{
    ENTER_CAMHW_FUNCTION();
    struct rk_cam_modify_pos modify_pos;

    if (!pLensHw->_zoom_enable) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "zoom is not supported");
        return XCAM_RETURN_ERROR_FAILED;
    }

    if (!attrPtr->use_manual) {
        modify_pos.zoom_pos = attrPtr->auto_zoompos;
        modify_pos.zoom1_pos = attrPtr->auto_zoom1pos;
        modify_pos.focus_pos = attrPtr->auto_focpos;
    } else {
        modify_pos.zoom_pos = attrPtr->manual_zoompos;
        modify_pos.zoom1_pos = attrPtr->manual_zoom1pos;
        modify_pos.focus_pos = attrPtr->manual_focpos;
    }
    if (pLensHw->mSd._v4l_base.io_control(&pLensHw->mSd._v4l_base, RK_VIDIOC_MODIFY_POSITION, &modify_pos) < 0) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "zoom focus modify position failed");
        return XCAM_RETURN_ERROR_IOCTL;
    }

    pLensHw->_zoom_pos = modify_pos.zoom_pos;
    pLensHw->_zoom1_pos = modify_pos.zoom1_pos;
    pLensHw->_focus_pos = modify_pos.focus_pos;
    pLensHw->_last_zoomchg_zoom = attrPtr->auto_zoompos;
    pLensHw->_last_zoomchg_zoom1 = attrPtr->auto_zoom1pos;
    pLensHw->_last_zoomchg_focus = attrPtr->auto_focpos;

    LOGD_CAMHW_SUBM(LENS_SUBM, "zoom focus modify position, use_manual %d, zoom_pos %d, zoom1_pos %d, focus_pos %d",
                    attrPtr->use_manual, modify_pos.zoom_pos, modify_pos.zoom1_pos, modify_pos.focus_pos);

    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqLensHw_ZoomFocusModifyPosition(AiqLensHw_t* pLensHw,
                                             rk_aiq_focus_params_t* focus_params)
{
    ENTER_CAMHW_FUNCTION();
    aiqMutex_lock(&pLensHw->_mutex);
    rk_aiq_focus_params_t* p_focus = focus_params;

    if (!pLensHw->_zoom_enable) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "focus is not supported");
        aiqMutex_unlock(&pLensHw->_mutex);
        return XCAM_RETURN_NO_ERROR;
    }

    if (!pLensHw->mSd._v4l_base._active)
        _start_internal(pLensHw);

#ifdef DISABLE_ZOOM_FOCUS
    aiqMutex_unlock(&pLensHw->_mutex);
    return XCAM_RETURN_NO_ERROR;
#endif

    rk_aiq_focus_params_t attrPtr;

    attrPtr.zoomfocus_modifypos = true;
    attrPtr.zoom_correction = false;
    attrPtr.focus_correction = false;
    attrPtr.use_manual = p_focus->use_manual;
    attrPtr.auto_focpos = p_focus->auto_focpos;
    attrPtr.auto_zoompos = p_focus->auto_zoompos;
    attrPtr.auto_zoom1pos = p_focus->auto_zoom1pos;
    attrPtr.manual_focpos = p_focus->manual_focpos;
    attrPtr.manual_zoompos = p_focus->manual_zoompos;
    attrPtr.manual_zoom1pos = p_focus->manual_zoom1pos;

    _LensHwHelperThd_push_attr(&pLensHw->_lenshw_thd, &attrPtr);
    aiqMutex_unlock(&pLensHw->_mutex);

    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqLensHw_handle_sof(AiqLensHw_t* pLensHw, int64_t time, uint32_t frameid)
{
    ENTER_CAMHW_FUNCTION();
    //SmartLock locker (_mutex);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    int idx;

    idx = (pLensHw->_rec_sof_idx + 1) % LENSHW_RECORD_SOF_NUM;
    pLensHw->_frame_sequence[idx] = frameid;
    pLensHw->_frame_time[idx] = time;
    pLensHw->_rec_sof_idx = idx;

    LOGD_CAMHW_SUBM(LENS_SUBM, "%s: frm_id %u, time %lld\n", __func__, frameid, time);

    EXIT_CAMHW_FUNCTION();
    return ret;
}

XCamReturn AiqLensHw_setLowPassFv(AiqLensHw_t* pLensHw,
                                  uint32_t sub_shp4_4[RKAIQ_RAWAF_SUMDATA_NUM],
                                  uint32_t sub_shp8_8[RKAIQ_RAWAF_SUMDATA_NUM],
                                  uint32_t high_light[RKAIQ_RAWAF_SUMDATA_NUM],
                                  uint32_t high_light2[RKAIQ_RAWAF_SUMDATA_NUM], uint32_t frameid)
{
    ENTER_CAMHW_FUNCTION();
    //SmartLock locker (_mutex);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    int idx;
    int64_t lowPassFv4_4, lowPassFv8_8, lowPassLight;

    if (!pLensHw->mSd._v4l_base._active)
        _start_internal(pLensHw);

    idx = (pLensHw->_rec_lowfv_idx + 1) % LENSHW_RECORD_LOWPASSFV_NUM;
    pLensHw->_lowfv_seq[idx] = frameid;
    memcpy(&pLensHw->_lowfv_fv4_4[idx], sub_shp4_4, sizeof(pLensHw->_lowfv_fv4_4[idx]));
    memcpy(&pLensHw->_lowfv_fv8_8[idx], sub_shp8_8, sizeof(pLensHw->_lowfv_fv8_8[idx]));
    memcpy(&pLensHw->_lowfv_highlht[idx], high_light, sizeof(pLensHw->_lowfv_highlht[idx]));
    memcpy(&pLensHw->_lowfv_highlht2[idx], high_light2, sizeof(pLensHw->_lowfv_highlht2[idx]));
    pLensHw->_rec_lowfv_idx = idx;

    lowPassFv4_4 = 0;
    lowPassFv8_8 = 0;
    lowPassLight = 0;
    for (int i = 0; i < RKAIQ_RAWAF_SUMDATA_NUM; i++) {
        lowPassFv4_4 += sub_shp4_4[i];
        lowPassFv8_8 += sub_shp8_8[i];
        lowPassLight += high_light[i];
    }

    LOGD_CAMHW_SUBM(LENS_SUBM, "%s: frm_id %d, lowPassFv4_4 %lld, lowPassFv8_8 %lld, lowPassLight %lld\n",
                    __func__, frameid, lowPassFv4_4, lowPassFv8_8, lowPassLight);

    EXIT_CAMHW_FUNCTION();
    return ret;
}

XCamReturn AiqLensHw_getIrisInfoParams(AiqLensHw_t* pLensHw, uint32_t frame_id, AiqIrisInfoWrapper_t* pIrisInfo)
{
    ENTER_CAMHW_FUNCTION();
    //SmartLock locker (_mutex);

    int i;

    for (i = 0; i < LENSHW_RECORD_SOF_NUM; i++) {
        if (frame_id == pLensHw->_frame_sequence[i])
            break;
    }

    //Piris
    pIrisInfo->PIris.StartTim = pLensHw->_piris_tim.motor_start_t;
    pIrisInfo->PIris.EndTim = pLensHw->_piris_tim.motor_end_t;
    pIrisInfo->PIris.laststep = pLensHw->_last_piris_step;
    pIrisInfo->PIris.step = pLensHw->_piris_step;

    //DCiris

    if (i < LENSHW_RECORD_SOF_NUM) {
        pIrisInfo->sofTime = pLensHw->_frame_time[i];
    } else {
        LOGE_CAMHW_SUBM(LENS_SUBM, "%s: frame_id %u, can not find sof time!\n", __FUNCTION__, frame_id);
        return  XCAM_RETURN_ERROR_PARAM;
    }

    LOGD_CAMHW_SUBM(LENS_SUBM, "%s: frm_id %u, time %lld\n", __func__, frame_id, pIrisInfo->sofTime);

    EXIT_CAMHW_FUNCTION();

    return XCAM_RETURN_NO_ERROR;

}

XCamReturn AiqLensHw_getAfInfoParams(AiqLensHw_t* pLensHw, uint32_t frame_id, AiqAfInfoWrapper_t* pAfInfo)
{
    ENTER_CAMHW_FUNCTION();
    aiqMutex_lock(&pLensHw->_mutex);

    int i;

    for (i = 0; i < LENSHW_RECORD_SOF_NUM; i++) {
        if (frame_id == pLensHw->_frame_sequence[i])
            break;
    }

    pAfInfo->focusStartTim = pLensHw->_focus_tim.vcm_start_t;
    pAfInfo->focusEndTim = pLensHw->_focus_tim.vcm_end_t;
    pAfInfo->zoomStartTim = pLensHw->_zoom_tim.vcm_start_t;
    pAfInfo->zoomEndTim = pLensHw->_zoom_tim.vcm_end_t;
    pAfInfo->focusCode = pLensHw->_focus_pos;
    pAfInfo->zoomCode = pLensHw->_zoom_pos;
    pAfInfo->angleZ = pLensHw->_angleZ;
    pAfInfo->zoomCorrection = pLensHw->_zoom_correction;
    pAfInfo->focusCorrection = pLensHw->_focus_correction;
    if (i < LENSHW_RECORD_SOF_NUM) {
        pAfInfo->sofTime = pLensHw->_frame_time[i];
    } else {
        LOGE_CAMHW_SUBM(LENS_SUBM, "%s: frame_id %d, can not find sof time!\n", __FUNCTION__, frame_id);
        aiqMutex_unlock(&pLensHw->_mutex);
        return  XCAM_RETURN_ERROR_PARAM;
    }

    for (i = 0; i < LENSHW_RECORD_LOWPASSFV_NUM; i++) {
        if (frame_id == pLensHw->_lowfv_seq[i] + 1)
            break;
    }

    if (i < LENSHW_RECORD_SOF_NUM) {
        pAfInfo->lowPassId = pLensHw->_lowfv_seq[i];
        memcpy(pAfInfo->lowPassFv4_4,
               pLensHw->_lowfv_fv4_4[i], RKAIQ_RAWAF_SUMDATA_NUM * sizeof(int32_t));
        memcpy(pAfInfo->lowPassFv8_8,
               pLensHw->_lowfv_fv8_8[i], RKAIQ_RAWAF_SUMDATA_NUM * sizeof(int32_t));
        memcpy(pAfInfo->lowPassHighLht,
               pLensHw->_lowfv_highlht[i], RKAIQ_RAWAF_SUMDATA_NUM * sizeof(int32_t));
        memcpy(pAfInfo->lowPassHighLht2,
               pLensHw->_lowfv_highlht2[i], RKAIQ_RAWAF_SUMDATA_NUM * sizeof(int32_t));
    } else {
        pAfInfo->lowPassId = 0;
        memset(pAfInfo->lowPassFv4_4, 0, RKAIQ_RAWAF_SUMDATA_NUM * sizeof(int32_t));
        memset(pAfInfo->lowPassFv8_8, 0, RKAIQ_RAWAF_SUMDATA_NUM * sizeof(int32_t));
        memset(pAfInfo->lowPassHighLht, 0, RKAIQ_RAWAF_SUMDATA_NUM * sizeof(int32_t));
        memset(pAfInfo->lowPassHighLht2, 0, RKAIQ_RAWAF_SUMDATA_NUM * sizeof(int32_t));
    }

    LOGD_CAMHW_SUBM(LENS_SUBM, "%s: frm_id %u, time %lld, lowPassFv4_4[0] %d, lowPassId %d\n",
                    __func__, frame_id, pAfInfo->sofTime, pAfInfo->lowPassFv4_4[0], pAfInfo->lowPassId);


    LOGD_CAMHW_SUBM(LENS_SUBM, "%s: frm_id %u, time %lld\n", __func__, frame_id, pAfInfo->sofTime);

    EXIT_CAMHW_FUNCTION();
    aiqMutex_unlock(&pLensHw->_mutex);

    return XCAM_RETURN_NO_ERROR;

}
XCamReturn AiqLensHw_setAngleZ(AiqLensHw_t* pLensHw, float angleZ)
{
    ENTER_CAMHW_FUNCTION();
    aiqMutex_lock(&pLensHw->_mutex);

    pLensHw->_angleZ = angleZ;
    EXIT_CAMHW_FUNCTION();
    aiqMutex_unlock(&pLensHw->_mutex);

    return XCAM_RETURN_NO_ERROR;
}
