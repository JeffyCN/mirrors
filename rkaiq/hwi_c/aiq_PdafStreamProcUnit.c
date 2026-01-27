/*
 *  Copyright (c) 2024 Rockchip Electronics Co., Ltd
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

#include "hwi_c/aiq_pdafStreamProcUnit.h"

#include "c_base/aiq_list.h"
#include "hwi_c/aiq_CamHwBase.h"
#include "rkcif-config.h"

#define PDAF_HELP_THREAD_MSG_MAX (3)

static int64_t get_systime_us() {
    struct timespec times = {0, 0};
    int64_t time;

    clock_gettime(CLOCK_MONOTONIC, &times);
    time = times.tv_sec * 1000000LL + times.tv_nsec / 1000LL;

    return time;
}

static XCamReturn _start_stream(AiqPdafStreamProcUnit_t* pProcUnit, bool block) {
    XCAM_FAIL_RETURN(ERROR, pProcUnit, XCAM_RETURN_ERROR_PARAM, "pPdafStreamProc is NULL!");

    bool do_start = false;

    if (block) aiqMutex_lock(&pProcUnit->mStreamMutex);

    if (pProcUnit->_pdafStream && !pProcUnit->mStartStreamFlag) {
        pProcUnit->mStartStreamFlag = true;
        do_start = true;
        LOGD_AF("start pdaf stream device");
    }
    if (block) aiqMutex_unlock(&pProcUnit->mStreamMutex);

    if (do_start) pProcUnit->_pdafStream->start(pProcUnit->_pdafStream);

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _stop_stream(AiqPdafStreamProcUnit_t* pProcUnit, bool block) {
    XCAM_FAIL_RETURN(ERROR, pProcUnit, XCAM_RETURN_ERROR_PARAM, "pPdafStreamProc is NULL!");

    bool do_stop = false;

    if (block) aiqMutex_lock(&pProcUnit->mStreamMutex);
    if (pProcUnit->_pdafStream && pProcUnit->mStartStreamFlag) {
        pProcUnit->mStartStreamFlag = false;
        do_stop = true;
        LOGD_AF("stop pdaf stream device");
    }
    if (block) aiqMutex_unlock(&pProcUnit->mStreamMutex);

    if (do_stop) pProcUnit->_pdafStream->stop(pProcUnit->_pdafStream);

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn AiqPdafStreamProcUnit_poll_buffer_ready(void* ctx, AiqHwEvt_t* evt,
                                                          int dev_index) {
    XCAM_FAIL_RETURN(ERROR, ctx, XCAM_RETURN_ERROR_PARAM, "pPdafStreamProc is NULL!");
    XCamReturn ret                     = XCAM_RETURN_NO_ERROR;
    AiqPdafStreamProcUnit_t* pProcUnit = (AiqPdafStreamProcUnit_t*)ctx;
    if (pProcUnit->_camHw && pProcUnit->_camHw->_hwResListener.hwResCb) {
        AiqHwPdafEvt_t *pdaf_evt = (AiqHwPdafEvt_t *)evt;

        pdaf_evt->pdaf_meas = pProcUnit->mPdafMeas;
        //LOGD_AF("%s: PDAF_STATS seq: %d, driver_time : %lld, aiq_time: %lld", __func__,
        //        evt->frame_id, evt->mTimestamp, get_systime_us());
        // vicap/pdaf driver set timestamp using fs, 3a stats use fe on old platform and fs on new platform(3576/1126b).
        return pProcUnit->_camHw->_hwResListener.hwResCb(pProcUnit->_camHw->_hwResListener._pCtx,
                                                         evt);
    } else {
        LOGD_AF("%s: poll buffer failed!", __func__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    return ret;
}

static bool _PdafStreamHelperThd_push_attr(PdafStreamHelperThd_t* pHelpThd,
                                           AiqPdafStreamParam* params) {
    if (!pHelpThd->_base->_started) return false;

    int ret = aiqList_push(pHelpThd->mMsgsQueue, params);
    if (ret) {
        LOGE_AF("push focus params failed", __func__);
        return false;
    }

    aiqMutex_lock(&pHelpThd->_mutex);
    aiqCond_broadcast(&pHelpThd->_cond);
    aiqMutex_unlock(&pHelpThd->_mutex);

    return true;
}

static bool PdafStreamHelperThd_loop(void* user_data) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ENTER_ANALYZER_FUNCTION();

    PdafStreamHelperThd_t* pHdlTh = (PdafStreamHelperThd_t*)user_data;

    aiqMutex_lock(&pHdlTh->_mutex);
    while (!pHdlTh->bQuit && aiqList_size(pHdlTh->mMsgsQueue) <= 0) {
        aiqCond_wait(&pHdlTh->_cond, &pHdlTh->_mutex);
    }
    if (pHdlTh->bQuit) {
        aiqMutex_unlock(&pHdlTh->_mutex);
        LOGE_AF("quit PdafStreamHelperThd!");
        return false;
    }
    aiqMutex_unlock(&pHdlTh->_mutex);

    AiqListItem_t* pItem = aiqList_get_item(pHdlTh->mMsgsQueue, NULL);
    if (!pItem) {
        // ignore error
        return true;
    }
    AiqPdafStreamParam* attrib = (AiqPdafStreamParam*)pItem->_pData;

    if (attrib->valid) {
        if (attrib->stream_flag) {
            ret = _start_stream(pHdlTh->mPdafStreamProc, true);
        } else {
            ret = _stop_stream(pHdlTh->mPdafStreamProc, true);
        }
    }

    aiqList_erase_item(pHdlTh->mMsgsQueue, pItem);

    if (ret == XCAM_RETURN_NO_ERROR) return true;

    LOGE_AF("PdafStreamHelperThd failed to run command!");

    EXIT_ANALYZER_FUNCTION();

    return false;
}

XCamReturn PdafStreamHelperThd_init(PdafStreamHelperThd_t* pHelpThd,
                                    AiqPdafStreamProcUnit_t* pProcUnit) {
    pHelpThd->mPdafStreamProc = pProcUnit;

    aiqMutex_init(&pHelpThd->_mutex);
    aiqCond_init(&pHelpThd->_cond);

    AiqListConfig_t msgqCfg;
    msgqCfg._name        = "PdafHMsgQ";
    msgqCfg._item_nums   = PDAF_HELP_THREAD_MSG_MAX;
    msgqCfg._item_size   = sizeof(AiqPdafStreamParam);
    pHelpThd->mMsgsQueue = aiqList_init(&msgqCfg);
    if (!pHelpThd->mMsgsQueue) {
        LOGE_AF("init %s error", msgqCfg._name);
        goto err_list;
    }

    pHelpThd->_base = aiqThread_init("AfHelpTh", PdafStreamHelperThd_loop, pHelpThd);
    if (!pHelpThd->_base) goto err_thread;

    return XCAM_RETURN_NO_ERROR;
err_thread:
    if (pHelpThd->mMsgsQueue) aiqList_deinit(pHelpThd->mMsgsQueue);
err_list:
    aiqMutex_deInit(&pHelpThd->_mutex);
    aiqCond_deInit(&pHelpThd->_cond);
    return XCAM_RETURN_ERROR_FAILED;
}

void PdafStreamHelperThd_deinit(PdafStreamHelperThd_t* pHelpThd) {
    ENTER_ANALYZER_FUNCTION();
    if (pHelpThd->mMsgsQueue) aiqList_deinit(pHelpThd->mMsgsQueue);
    if (pHelpThd->_base) aiqThread_deinit(pHelpThd->_base);

    aiqMutex_deInit(&pHelpThd->_mutex);
    aiqCond_deInit(&pHelpThd->_cond);
    EXIT_ANALYZER_FUNCTION();
}

XCamReturn PdafStreamHelperThd_start(PdafStreamHelperThd_t* pHelpThd) {
    ENTER_ANALYZER_FUNCTION();
    if (!aiqThread_start(pHelpThd->_base)) {
        LOGE_AF("%s failed", __func__);
        return XCAM_RETURN_ERROR_FAILED;
    }
    pHelpThd->bQuit = false;
    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn PdafStreamHelperThd_stop(PdafStreamHelperThd_t* pHelpThd) {
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

XCamReturn AiqPdafStreamProcUnit_init(AiqPdafStreamProcUnit_t* pProcUnit, int type) {
    XCAM_FAIL_RETURN(ERROR, pProcUnit, XCAM_RETURN_ERROR_PARAM, "pPdafStreamProc is NULL!");

    pProcUnit->_camHw           = NULL;
    pProcUnit->mPdafDev         = NULL;
    pProcUnit->_pcb             = NULL;
    pProcUnit->mStartFlag       = false;
    pProcUnit->mStartStreamFlag = false;
    pProcUnit->mBufType         = type;

    aiqMutex_init(&pProcUnit->mStreamMutex);
    PdafStreamHelperThd_init(&pProcUnit->mHelperThd, pProcUnit);
    PdafStreamHelperThd_start(&pProcUnit->mHelperThd);
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqPdafStreamProcUnit_deinit(AiqPdafStreamProcUnit_t* pProcUnit) {
    if (!pProcUnit) return XCAM_RETURN_ERROR_PARAM;

    PdafStreamHelperThd_stop(&pProcUnit->mHelperThd);
    PdafStreamHelperThd_deinit(&pProcUnit->mHelperThd);

    if (pProcUnit->_pcb) {
        aiq_free(pProcUnit->_pcb);
        pProcUnit->_pcb = NULL;
    }
    if (pProcUnit->mPdafDev) {
        pProcUnit->mPdafDev->close(pProcUnit->mPdafDev);
        aiq_free(pProcUnit->mPdafDev);
        pProcUnit->mPdafDev = NULL;
        if (pProcUnit->_pdafStream) {
            aiq_free(pProcUnit->_pdafStream);
            pProcUnit->_pdafStream = NULL;
        }
    }

    aiqMutex_deInit(&pProcUnit->mStreamMutex);
    return XCAM_RETURN_NO_ERROR;
}

void AiqPdafStreamProcUnit_set_devices(AiqPdafStreamProcUnit_t* pProcUnit, AiqCamHwBase_t* camHw) {
    if (!pProcUnit) return;

    pProcUnit->_camHw = camHw;
}

XCamReturn AiqPdafStreamProcUnit_preapre(AiqPdafStreamProcUnit_t* pProcUnit,
                                         rk_sensor_pdaf_info_t* pdaf_inf) {
    XCAM_FAIL_RETURN(ERROR, pProcUnit, XCAM_RETURN_ERROR_PARAM, "pPdafStreamProc is NULL!");
    XCAM_FAIL_RETURN(ERROR, pdaf_inf, XCAM_RETURN_ERROR_PARAM, " pdaf_inf is NULL!");

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    AiqPdafStreamProcUnit_stop(pProcUnit);
    pProcUnit->mPdafInf = *pdaf_inf;

    if (pProcUnit->mPdafDev) {
        pProcUnit->mPdafDev->close(pProcUnit->mPdafDev);
        aiq_free(pProcUnit->mPdafDev);
        pProcUnit->mPdafDev = NULL;
        if (pProcUnit->_pdafStream) {
            aiq_free(pProcUnit->_pdafStream);
            pProcUnit->_pdafStream = NULL;
        }
    }

    pProcUnit->mPdafDev = (AiqV4l2Device_t*)aiq_mallocz(sizeof(AiqV4l2Device_t));
    if (!pProcUnit->mPdafDev) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "%d: alloc fail !", __LINE__);
        goto fail;
    }
    AiqV4l2Device_init(pProcUnit->mPdafDev, pProcUnit->mPdafInf.pdaf_vdev);
    pProcUnit->mPdafDev->open(pProcUnit->mPdafDev, false);
    pProcUnit->_pdafStream = (AiqStream_t*)aiq_mallocz(sizeof(AiqStream_t));
    AiqPdafStream_init(pProcUnit->_pdafStream, pProcUnit->mPdafDev, ISP_POLL_PDAF_STATS);
    if (!pProcUnit->_pcb) {
        AiqPollCallback_t* pcb = (AiqPollCallback_t*)aiq_mallocz(sizeof(AiqPollCallback_t));
        pcb->_pCtx             = pProcUnit;
        pcb->poll_buffer_ready = AiqPdafStreamProcUnit_poll_buffer_ready;
        pcb->poll_event_ready  = NULL;
        pProcUnit->_pcb        = pcb;
        AiqPollThread_setPollCallback(pProcUnit->_pdafStream->_poll_thread, pcb);
    }

    strcpy(pProcUnit->mPdafMeas.snsName, pProcUnit->mPdafInf.sns_name);
    pProcUnit->mPdafMeas.pdafSensorType = pProcUnit->mPdafInf.pdaf_type;
    pProcUnit->mPdafMeas.pdLRInDiffLine = pProcUnit->mPdafInf.pdaf_lrdiffline;
    pProcUnit->mPdafMeas.pdWidth = pProcUnit->mPdafInf.pdaf_width;
    pProcUnit->mPdafMeas.pdHeight = pProcUnit->mPdafInf.pdaf_height;
    LOGD_AF("pd inf: sns_name %s, pdaf_vdev %s, pdafSensorType %d, pdLRInDiffLine %d, pdWidth %d, pdHeight %d",
        pProcUnit->mPdafInf.sns_name, pProcUnit->mPdafInf.pdaf_vdev, pProcUnit->mPdafMeas.pdafSensorType, pProcUnit->mPdafMeas.pdLRInDiffLine,
        pProcUnit->mPdafMeas.pdWidth, pProcUnit->mPdafMeas.pdHeight);

    ret = AiqV4l2Device_setFmt(pProcUnit->mPdafDev, pProcUnit->mPdafInf.pdaf_width, pProcUnit->mPdafInf.pdaf_height,
                               pProcUnit->mPdafInf.pdaf_pixelformat, V4L2_FIELD_NONE, 0);

    struct v4l2_format fmt;
    ret |= AiqV4l2Device_getV4lFmt(pProcUnit->mPdafDev, &fmt);
    pProcUnit->mPdafMeas.bytesperline = fmt.fmt.pix_mp.plane_fmt[0].bytesperline;
    return ret;
fail:
    return XCAM_RETURN_ERROR_FAILED;
}

void AiqPdafStreamProcUnit_start(AiqPdafStreamProcUnit_t* pProcUnit) {
    if (!pProcUnit) return;

    aiqMutex_lock(&pProcUnit->mStreamMutex);
    if (pProcUnit->_pdafStream && !pProcUnit->mStartFlag) {
        if (pProcUnit->mPdafInf.pdaf_type == PDAF_SENSOR_TYPE3) {
            _start_stream(pProcUnit, false);
        }
        pProcUnit->mStartFlag = true;
    }
    aiqMutex_unlock(&pProcUnit->mStreamMutex);
}

void AiqPdafStreamProcUnit_stop(AiqPdafStreamProcUnit_t* pProcUnit) {
    if (!pProcUnit) return;

    aiqMutex_lock(&pProcUnit->mStreamMutex);
    if (pProcUnit->_pdafStream && pProcUnit->mStartFlag) {
        if (pProcUnit->mPdafInf.pdaf_type != PDAF_SENSOR_TYPE3) {
            AiqPdafStreamParam attr;

            memset(&attr, 0, sizeof(attr));
            AiqPdafStreamParam* attrPtr = &attr;
            attrPtr->valid              = true;
            attrPtr->stream_flag        = false;
            aiqList_reset(pProcUnit->mHelperThd.mMsgsQueue);
            _PdafStreamHelperThd_push_attr(&pProcUnit->mHelperThd, attrPtr);
        } else {
            _stop_stream(pProcUnit, false);
        }

        pProcUnit->mStartFlag = false;
    }
    aiqMutex_unlock(&pProcUnit->mStreamMutex);
}

void AiqPdafStreamProcUnit_notify_sof(AiqPdafStreamProcUnit_t* pProcUnit) {
    if (!pProcUnit) return;

    int32_t mem_mode = 0;

    aiqMutex_lock(&pProcUnit->mStreamMutex);
    if (pProcUnit->_pdafStream && pProcUnit->mStartFlag && !pProcUnit->mStartStreamFlag) {
        if (pProcUnit->mPdafInf.pdaf_type != PDAF_SENSOR_TYPE3) {
            pProcUnit->mPdafDev->io_control(pProcUnit->mPdafDev, RKCIF_CMD_GET_CSI_MEMORY_MODE,
                                            &mem_mode);
            if (mem_mode != CSI_LVDS_MEM_WORD_LOW_ALIGN) {
                mem_mode = CSI_LVDS_MEM_WORD_LOW_ALIGN;
                pProcUnit->mPdafDev->io_control(pProcUnit->mPdafDev, RKCIF_CMD_SET_CSI_MEMORY_MODE,
                                                &mem_mode);
                LOGI_AF("memory mode of pdaf video need low align, mem_mode %d", mem_mode);
            }

            AiqPdafStreamParam attr;

            memset(&attr, 0, sizeof(attr));
            AiqPdafStreamParam* attrPtr = &attr;
            attrPtr->valid       = true;
            attrPtr->stream_flag = true;
            aiqList_reset(pProcUnit->mHelperThd.mMsgsQueue);
            _PdafStreamHelperThd_push_attr(&pProcUnit->mHelperThd, attrPtr);
        }
    }
    aiqMutex_unlock(&pProcUnit->mStreamMutex);
}

