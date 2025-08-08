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

#include "aiq_rawStreamCapUnit.h"

#if RKAIQ_HAVE_DUMPSYS
#include "aiq_streamCapInfo.h"
#endif
#include "c_base/aiq_list.h"
#include "hwi_c/aiq_CamHwBase.h"
#include "hwi_c/aiq_rawStreamProcUnit.h"
#include "include/common/rkcif-config.h"

extern sensor_info_share_t g_rk1608_share_inf;

bool check_skip_frame(AiqRawStreamCapUnit_t* pRawStrCapUnit, int32_t buf_seq) {
    aiqMutex_lock(&pRawStrCapUnit->_mipi_mutex);
#if 0  // ts
    if (_skip_num > 0) {
        int64_t skip_ts_ms = _skip_start_ts / 1000 / 1000;
        int64_t buf_ts_ms = buf_ts / 1000;
        LOGD_CAMHW_SUBM(ISP20HW_SUBM, "skip num  %d, start from %" PRId64 " ms,  buf ts %" PRId64 " ms",
                        _skip_num,
                        skip_ts_ms,
                        buf_ts_ms);
        if (buf_ts_ms  > skip_ts_ms) {
            _skip_num--;
            _mipi_mutex.unlock();
            return true;
        }
    }
#else

    if ((pRawStrCapUnit->_skip_num > 0) && (buf_seq < pRawStrCapUnit->_skip_to_seq)) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "skip num  %d, skip seq %d, dest seq %d",
                        pRawStrCapUnit->_skip_num, buf_seq, pRawStrCapUnit->_skip_to_seq);
        pRawStrCapUnit->_skip_num--;
        aiqMutex_unlock(&pRawStrCapUnit->_mipi_mutex);
        return true;
    }
#endif
    aiqMutex_unlock(&pRawStrCapUnit->_mipi_mutex);
    return false;
}

XCamReturn sync_raw_buf(AiqRawStreamCapUnit_t* pRawStrCapUnit, AiqV4l2Buffer_t** buf_s,
                        AiqV4l2Buffer_t** buf_m, AiqV4l2Buffer_t** buf_l) {
    uint32_t sequence_s = -1, sequence_m = -1, sequence_l = -1;
    AiqListItem_t *pItem_s = NULL, *pItem_m = NULL, *pItem_l = NULL;

    for (int i = 0; i < pRawStrCapUnit->_mipi_dev_max; i++) {
        if (!aiqList_size(pRawStrCapUnit->buf_list[i])) {
            return XCAM_RETURN_ERROR_FAILED;
        }
    }

    if (pRawStrCapUnit->buf_list[ISP_MIPI_HDR_L])
        pItem_l = aiqList_get_item(pRawStrCapUnit->buf_list[ISP_MIPI_HDR_L], NULL);

    if (pItem_l) {
        *buf_l     = *(AiqV4l2Buffer_t**)(pItem_l->_pData);
        sequence_l = AiqV4l2Buffer_getSequence(*buf_l);
    }

    if (pRawStrCapUnit->buf_list[ISP_MIPI_HDR_M])
        pItem_m = aiqList_get_item(pRawStrCapUnit->buf_list[ISP_MIPI_HDR_M], NULL);

    if (pItem_m) {
        *buf_m     = *(AiqV4l2Buffer_t**)(pItem_m->_pData);
        sequence_m = AiqV4l2Buffer_getSequence(*buf_m);
    }

    if (pRawStrCapUnit->buf_list[ISP_MIPI_HDR_S])
        pItem_s = aiqList_get_item(pRawStrCapUnit->buf_list[ISP_MIPI_HDR_S], NULL);

    if (pItem_s) {
        *buf_s     = *(AiqV4l2Buffer_t**)(pItem_s->_pData);
        sequence_s = AiqV4l2Buffer_getSequence(*buf_s);
    }

    if (*buf_s) {
        if ((pRawStrCapUnit->_working_mode == RK_AIQ_ISP_HDR_MODE_3_FRAME_HDR ||
             pRawStrCapUnit->_working_mode == RK_AIQ_ISP_HDR_MODE_3_LINE_HDR) &&
            *buf_m && *buf_l && sequence_l == sequence_s && sequence_m == sequence_s) {
            aiqList_erase_item(pRawStrCapUnit->buf_list[ISP_MIPI_HDR_S], pItem_s);
            aiqList_erase_item(pRawStrCapUnit->buf_list[ISP_MIPI_HDR_M], pItem_m);
            aiqList_erase_item(pRawStrCapUnit->buf_list[ISP_MIPI_HDR_L], pItem_l);
            if (check_skip_frame(pRawStrCapUnit, sequence_s)) {
                AiqV4l2Buffer_unref(*buf_s);
                AiqV4l2Buffer_unref(*buf_m);
                AiqV4l2Buffer_unref(*buf_l);
                LOGW_CAMHW_SUBM(ISP20HW_SUBM, "skip frame %d", sequence_s);
                goto end;
            }
        } else if ((pRawStrCapUnit->_working_mode == RK_AIQ_ISP_HDR_MODE_2_FRAME_HDR ||
                    pRawStrCapUnit->_working_mode == RK_AIQ_ISP_HDR_MODE_2_LINE_HDR) &&
                   *buf_m && sequence_m == sequence_s) {
            aiqList_erase_item(pRawStrCapUnit->buf_list[ISP_MIPI_HDR_S], pItem_s);
            aiqList_erase_item(pRawStrCapUnit->buf_list[ISP_MIPI_HDR_M], pItem_m);
            if (check_skip_frame(pRawStrCapUnit, sequence_s)) {
                LOGE_CAMHW_SUBM(ISP20HW_SUBM, "skip frame %d", sequence_s);
                goto end;
            }
        } else if (pRawStrCapUnit->_working_mode == RK_AIQ_WORKING_MODE_NORMAL ||
                RK_AIQ_HDR_IS_SENSOR_BUILTIN(pRawStrCapUnit->_working_mode)) {
            aiqList_erase_item(pRawStrCapUnit->buf_list[ISP_MIPI_HDR_S], pItem_s);
            if (check_skip_frame(pRawStrCapUnit, sequence_s)) {
                LOGW_CAMHW_SUBM(ISP20HW_SUBM, "skip frame %d", sequence_s);
                goto end;
            }
        } else {
            LOGW_CAMHW_SUBM(ISP20HW_SUBM, "do nothing, sequence not match l: %d, s: %d, m: %d !!!",
                            sequence_l, sequence_s, sequence_m);
        }
        return XCAM_RETURN_NO_ERROR;
    }
end:
    return XCAM_RETURN_ERROR_FAILED;
}

XCamReturn RawStreamCapUnit_poll_buffer_ready(void* ctx, AiqHwEvt_t* evt, int dev_index) {
    XCamReturn ret                        = XCAM_RETURN_NO_ERROR;
    AiqRawStreamCapUnit_t* pRawStrCapUnit = (AiqRawStreamCapUnit_t*)ctx;
    AiqV4l2Buffer_t *buf_s = NULL, *buf_m = NULL, *buf_l = NULL;

#if RKAIQ_HAVE_DUMPSYS
    // dump fe info
    if (dev_index == ISP_MIPI_HDR_S) {
        struct timespec time;
        clock_gettime(CLOCK_MONOTONIC, &time);

        pRawStrCapUnit->fe.frameloss +=
            evt->frame_id ? evt->frame_id - pRawStrCapUnit->fe.id - 1 : 0;
        pRawStrCapUnit->fe.id        = evt->frame_id;
        pRawStrCapUnit->fe.timestamp = evt->mTimestamp;
        pRawStrCapUnit->fe.delay     = XCAM_TIMESPEC_2_USEC(time) - evt->mTimestamp;
    }
#endif

    aiqMutex_lock(&pRawStrCapUnit->_buf_mutex);
    AiqVideoBuffer_ref(evt->vb);
    aiqList_push(pRawStrCapUnit->buf_list[dev_index], &evt->vb);
    ret = sync_raw_buf(pRawStrCapUnit, &buf_s, &buf_m, &buf_l);
    aiqMutex_unlock(&pRawStrCapUnit->_buf_mutex);

    if (ret == XCAM_RETURN_NO_ERROR) {
        // FIXME: rawCap only has one valid pointer(just malloc once buf), so it needs to do
        // multiple syncs
        if (!pRawStrCapUnit->_is_1608_stream) {
            // normal
#if RKAIQ_HAVE_AIRMS
            if (pRawStrCapUnit->_pAirmsStream) {
                AiqAiRmsStreamProcUnit_setVicapBuf(pRawStrCapUnit->_pAirmsStream, buf_s);
            }
            else
#endif
            if (pRawStrCapUnit->_send_sync_buf_func && pRawStrCapUnit->_sw_stream_ctx) {
                pRawStrCapUnit->_send_sync_buf_func(pRawStrCapUnit->_sw_stream_ctx, buf_s, buf_m, buf_l);
            } else if (pRawStrCapUnit->_proc_stream) {
                AiqRawStreamProcUnit_send_sync_buf(pRawStrCapUnit->_proc_stream, buf_s, buf_m, buf_l);
            }
        } else {
            // 1608 mode.
            for (int idx = 0; idx < CAM_INDEX_FOR_1608; idx++) {
                if (g_rk1608_share_inf.raw_proc_unit[idx]) {
                    AiqRawStreamProcUnit_send_sync_buf(g_rk1608_share_inf.raw_proc_unit[idx], buf_s,
                                                       buf_m, buf_l);
                }
            }
        }

        if (pRawStrCapUnit->_camHw->_hwResListener.hwResCb) {
            pRawStrCapUnit->_camHw->_hwResListener.hwResCb(
                pRawStrCapUnit->_camHw->_hwResListener._pCtx, evt);
        }
    }

    if (buf_s) AiqV4l2Buffer_unref(buf_s);
    if (buf_m) AiqV4l2Buffer_unref(buf_m);
    if (buf_l) AiqV4l2Buffer_unref(buf_l);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqRawStreamCapUnit_init(AiqRawStreamCapUnit_t* pRawStrCapUnit,
                                    const rk_sensor_full_info_t* s_info, bool linked_to_isp,
                                    int tx_buf_cnt) {
    pRawStrCapUnit->_is_1608_stream = false;
    pRawStrCapUnit->_skip_num       = 0;
    pRawStrCapUnit->_mipi_dev_max   = 1;
    pRawStrCapUnit->_state          = RAW_CAP_STATE_INVALID;
    pRawStrCapUnit->_isExtDev       = false;
#if RKAIQ_HAVE_DUMPSYS
    pRawStrCapUnit->data_mode = 0;
#endif
    aiqMutex_init(&pRawStrCapUnit->_buf_mutex);
    aiqMutex_init(&pRawStrCapUnit->_mipi_mutex);
    /*
     * for _mipi_tx_devs, index 0 refer to short frame always, inedex 1 refer
     * to middle frame always, index 2 refert to long frame always.
     * for CIF usecase, because mipi_id0 refert to long frame always, so we
     * should know the HDR mode firstly befor building the relationship between
     * _mipi_tx_devs array and mipi_idx. here we just set the mipi_idx to
     * _mipi_tx_devs, we will build the real relation in start.
     * for CIF usecase, rawwr2_path is always connected to _mipi_tx_devs[0],
     * rawwr0_path is always connected to _mipi_tx_devs[1], and rawwr1_path is always
     * connected to _mipi_tx_devs[0]
     */
    // FIXME: flag(start_en) is ensure 1608 sensor already open.
    bool start_en = true;
    if (s_info->linked_to_1608) {
        // Traverse the corresponding [1~4] nodes
        if (g_rk1608_share_inf.us_open_cnt > 0) {
            // 1st open
            start_en = false;
        }

        // state update.
        g_rk1608_share_inf.us_open_cnt++;
        if (g_rk1608_share_inf.us_open_cnt > 10) g_rk1608_share_inf.us_open_cnt = 10;
    }

    const char* dev_str = NULL;
    // short frame
    if (strlen(s_info->isp_info->rawrd2_s_path)) {
        if (linked_to_isp) {
            dev_str = s_info->isp_info->rawwr2_path;
        } else {
            if (s_info->dvp_itf) {
                if (strlen(s_info->cif_info->stream_cif_path)) {
                    dev_str = s_info->cif_info->stream_cif_path;
                } else {
                    dev_str = s_info->cif_info->dvp_id0;
                }
            } else {
                if (!s_info->linked_to_1608) {
                    // normal mode
                    dev_str = s_info->cif_info->mipi_id0;
                } else {
                    if (start_en) {
                        // 1608 sensor mode.
                        dev_str = s_info->cif_info->mipi_id0;
                    }
                }
            }
        }
        if (dev_str) {
            pRawStrCapUnit->_dev[0] = (AiqV4l2Device_t*)aiq_mallocz(sizeof(AiqV4l2Device_t));
            if (!pRawStrCapUnit->_dev[0]) {
                LOGE_CAMHW_SUBM(ISP20HW_SUBM, "%d: alloc fail !", __LINE__);
                goto fail;
            }
            AiqV4l2Device_init(pRawStrCapUnit->_dev[0], dev_str);
            pRawStrCapUnit->_dev[0]->open(pRawStrCapUnit->_dev[0], false);
        }
    }
    // mid frame
    dev_str = NULL;
    if (strlen(s_info->isp_info->rawrd0_m_path)) {
        if (linked_to_isp)
            dev_str = s_info->isp_info->rawwr0_path;  // rkisp_rawwr0
        else {
            if (!s_info->dvp_itf) {
                if (!s_info->linked_to_1608) {
                    // normal mode.
                    dev_str = s_info->cif_info->mipi_id1;
                } else {
                    if (start_en) {
                        // 1608 sensor mode.
                        dev_str = s_info->cif_info->mipi_id1;
                    }
                }
            }
        }

        if (dev_str) {
            pRawStrCapUnit->_dev[1] = (AiqV4l2Device_t*)aiq_mallocz(sizeof(AiqV4l2Device_t));
            if (!pRawStrCapUnit->_dev[1]) {
                LOGE_CAMHW_SUBM(ISP20HW_SUBM, "%d: alloc fail !", __LINE__);
                goto fail;
            }
            AiqV4l2Device_init(pRawStrCapUnit->_dev[1], dev_str);
            pRawStrCapUnit->_dev[1]->open(pRawStrCapUnit->_dev[1], false);
        }
    }
    dev_str = NULL;
    // long frame
    if (strlen(s_info->isp_info->rawrd1_l_path)) {
        if (linked_to_isp)
            dev_str = s_info->isp_info->rawwr1_path;  // rkisp_rawwr1
        else {
            if (!s_info->dvp_itf) {
                if (!s_info->linked_to_1608) {
                    // normal mode.
                    dev_str = s_info->cif_info->mipi_id2;  // rkisp_rawwr1
                } else {
                    if (start_en) {
                        // 1608 sensor mode.
                        dev_str = s_info->cif_info->mipi_id2;  // rkisp_rawwr1
                    }
                }
            }
        }
        if (dev_str) {
            pRawStrCapUnit->_dev[2] = (AiqV4l2Device_t*)aiq_mallocz(sizeof(AiqV4l2Device_t));
            if (!pRawStrCapUnit->_dev[2]) {
                LOGE_CAMHW_SUBM(ISP20HW_SUBM, "%d: alloc fail !", __LINE__);
                goto fail;
            }
            AiqV4l2Device_init(pRawStrCapUnit->_dev[2], dev_str);
            pRawStrCapUnit->_dev[2]->open(pRawStrCapUnit->_dev[2], false);
        }
    }
    int buf_cnt = tx_buf_cnt;
    if (tx_buf_cnt == 0) {
        if (linked_to_isp) {
            buf_cnt = ISP_TX_BUF_NUM;
        } else if (s_info->linked_to_1608) {
            buf_cnt = VIPCAP_TX_BUF_NUM_1608;
        } else {
            buf_cnt = VIPCAP_TX_BUF_NUM;
        }
    }

    int i = 0;
    for (i = 0; i < 3; i++) {
        if (pRawStrCapUnit->_dev[i]) {
            AiqV4l2Device_setBufCnt(pRawStrCapUnit->_dev[i], buf_cnt);
            AiqV4l2Device_setBufSync(pRawStrCapUnit->_dev[i], true);
            pRawStrCapUnit->_dev_bakup[i] = pRawStrCapUnit->_dev[i];
            pRawStrCapUnit->_dev_index[i] = i;
            pRawStrCapUnit->_stream[i]    = (AiqRawStream_t*)aiq_mallocz(sizeof(AiqRawStream_t));
            AiqRawStream_init(pRawStrCapUnit->_stream[i], pRawStrCapUnit->_dev[i], ISP_POLL_TX, i);
            AiqPollCallback_t* pcb  = (AiqPollCallback_t*)aiq_mallocz(sizeof(AiqPollCallback_t));
            pcb->_pCtx              = pRawStrCapUnit;
            pcb->poll_buffer_ready  = RawStreamCapUnit_poll_buffer_ready;
            pcb->poll_event_ready   = NULL;
            pRawStrCapUnit->_pcb[i] = pcb;
            AiqPollThread_setPollCallback(pRawStrCapUnit->_stream[i]->_base._poll_thread, pcb);
        } else {
            pRawStrCapUnit->_dev_bakup[i] = NULL;
            pRawStrCapUnit->_dev_index[i] = i;
        }
    }

    pRawStrCapUnit->_state = RAW_CAP_STATE_INITED;

    return XCAM_RETURN_NO_ERROR;
fail:
    AiqRawStreamCapUnit_deinit(pRawStrCapUnit);
    return XCAM_RETURN_ERROR_FAILED;
}

void AiqRawStreamCapUnit_deinit(AiqRawStreamCapUnit_t* pRawStrCapUnit) {
    pRawStrCapUnit->_state = RAW_CAP_STATE_INVALID;
    int i                  = 0;
    for (i = 0; i < 3; i++) {
        if (pRawStrCapUnit->_pcb[i]) {
            aiq_free(pRawStrCapUnit->_pcb[i]);
            pRawStrCapUnit->_pcb[i] = NULL;
        }

        if (pRawStrCapUnit->_dev[i]) {
            pRawStrCapUnit->_dev[i]->close(pRawStrCapUnit->_dev[i]);
            if (pRawStrCapUnit->_stream[i]) {
                AiqRawStream_deinit(pRawStrCapUnit->_stream[i]);
                aiq_free(pRawStrCapUnit->_stream[i]);
                pRawStrCapUnit->_stream[i] = NULL;
            }
            AiqV4l2Device_deinit(pRawStrCapUnit->_dev[i]);
            aiq_free(pRawStrCapUnit->_dev[i]);
            pRawStrCapUnit->_dev[i] = NULL;
        }

        if (pRawStrCapUnit->buf_list[i]) {
            aiqList_deinit(pRawStrCapUnit->buf_list[i]);
        }
    }

    aiqMutex_deInit(&pRawStrCapUnit->_buf_mutex);
    aiqMutex_deInit(&pRawStrCapUnit->_mipi_mutex);
}

XCamReturn AiqRawStreamCapUnit_start(AiqRawStreamCapUnit_t* pRawStrCapUnit, int mode) {
    LOGD_CAMHW_SUBM(ISP20HW_SUBM, "%s enter", __FUNCTION__);
    int i = 0;
    for (i = 0; i < pRawStrCapUnit->_mipi_dev_max; i++) {
        if (!pRawStrCapUnit->buf_list[i]) {
            AiqListConfig_t bufListCfg;
            bufListCfg._name             = "RawCapBufList";
            bufListCfg._item_nums        = 8;
            bufListCfg._item_size        = sizeof(AiqV4l2Buffer_t*);
            pRawStrCapUnit->buf_list[i]  = aiqList_init(&bufListCfg);
            if (!pRawStrCapUnit->buf_list[i])
                LOGE_CAMHW_SUBM(SENSOR_SUBM, "cId:%d init %s error", pRawStrCapUnit->mCamPhyId,
                                bufListCfg._name);
        }
        // FIXME: add mCamPhyId
        pRawStrCapUnit->_stream[i]->_base.setCamPhyId(&pRawStrCapUnit->_stream[i]->_base, 0);
        pRawStrCapUnit->_stream[i]->_base.start(&pRawStrCapUnit->_stream[i]->_base);
    }

    pRawStrCapUnit->_state = RAW_CAP_STATE_STARTED;
    LOGD_CAMHW_SUBM(ISP20HW_SUBM, "%s exit", __FUNCTION__);
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqRawStreamCapUnit_stop(AiqRawStreamCapUnit_t* pRawStrCapUnit) {
    LOGD_CAMHW_SUBM(ISP20HW_SUBM, "%s enter", __FUNCTION__);
    int i = 0;
    for (i = 0; i < pRawStrCapUnit->_mipi_dev_max; i++) {
        pRawStrCapUnit->_stream[i]->_base.stopThreadOnly(&pRawStrCapUnit->_stream[i]->_base);
    }
    aiqMutex_lock(&pRawStrCapUnit->_buf_mutex);
    for (i = 0; i < pRawStrCapUnit->_mipi_dev_max; i++) {
        AiqListItem_t* pItem = NULL;
        bool rm              = false;
        if (pRawStrCapUnit->buf_list[i]) {
            AIQ_LIST_FOREACH(pRawStrCapUnit->buf_list[i], pItem, rm) {
                AiqV4l2Buffer_unref(*(AiqV4l2Buffer_t**)(pItem->_pData));
                pItem = aiqList_erase_item_locked(pRawStrCapUnit->buf_list[i], pItem);
                rm    = true;
            }
            aiqList_reset(pRawStrCapUnit->buf_list[i]);
        }
    }
    aiqMutex_unlock(&pRawStrCapUnit->_buf_mutex);
    for (i = 0; i < pRawStrCapUnit->_mipi_dev_max; i++) {
        pRawStrCapUnit->_stream[i]->_base.stopDeviceOnly(&pRawStrCapUnit->_stream[i]->_base);
    }
    pRawStrCapUnit->_state = RAW_CAP_STATE_STOPPED;
    LOGD_CAMHW_SUBM(ISP20HW_SUBM, "%s exit", __FUNCTION__);
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqRawStreamCapUnit_prepare(AiqRawStreamCapUnit_t* pRawStrCapUnit, int idx) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    LOGD_CAMHW_SUBM(ISP20HW_SUBM, "%s enter", __FUNCTION__);
    // mipi rx/tx format should match to sensor.
    int i = 0;
    for (i = 0; i < 3; i++) {
        if (!(idx & (1 << i))) continue;
        ret = pRawStrCapUnit->_dev[i]->prepare(pRawStrCapUnit->_dev[i]);
        if (ret < 0) LOGE_CAMHW_SUBM(ISP20HW_SUBM, "mipi tx:%d prepare err: %d\n", ret);

        pRawStrCapUnit->_stream[i]->_base.set_device_prepared(&pRawStrCapUnit->_stream[i]->_base,
                                                              true);
    }
    pRawStrCapUnit->_state = RAW_CAP_STATE_PREPARED;
    LOGD_CAMHW_SUBM(ISP20HW_SUBM, "%s exit", __FUNCTION__);
    return ret;
}

void AiqRawStreamCapUnit_prepare_cif_mipi(AiqRawStreamCapUnit_t* pRawStrCapUnit) {
    LOGD_CAMHW_SUBM(ISP20HW_SUBM, "%s enter,working_mode=0x%x", __FUNCTION__,
                    pRawStrCapUnit->_working_mode);

    const char* dev_name = AiqV4l2Device_getDevName(pRawStrCapUnit->_dev[0]);

    if (strstr(dev_name, "zero")) {
        LOGD_CAMHW_SUBM(ISP20HW_SUBM, "ignore fake tx");
        return;
    }

    AiqV4l2Device_t* tx_devs_tmp[3] = {
        pRawStrCapUnit->_dev_bakup[0],
        pRawStrCapUnit->_dev_bakup[1],
        pRawStrCapUnit->_dev_bakup[2],
    };

    // _mipi_tx_devs
    if (pRawStrCapUnit->_working_mode == RK_AIQ_WORKING_MODE_NORMAL ||
        RK_AIQ_HDR_IS_SENSOR_BUILTIN(pRawStrCapUnit->_working_mode)) {
        // use _mipi_tx_devs[0] only
        // id0 as normal
        // do nothing
        LOGD_CAMHW_SUBM(ISP20HW_SUBM, "CIF tx: %s -> normal",
                        AiqV4l2Device_getDevName(pRawStrCapUnit->_dev[0]));
    } else if (RK_AIQ_HDR_GET_WORKING_MODE(pRawStrCapUnit->_working_mode) ==
               RK_AIQ_WORKING_MODE_ISP_HDR2) {
        // use _mipi_tx_devs[0] and _mipi_tx_devs[1]
        // id0 as l, id1 as s
        AiqV4l2Device_t* tmp = tx_devs_tmp[1];
        tx_devs_tmp[1]       = tx_devs_tmp[0];
        tx_devs_tmp[0]       = tmp;
        LOGD_CAMHW_SUBM(ISP20HW_SUBM, "CIF tx: %s -> long",
                        AiqV4l2Device_getDevName(pRawStrCapUnit->_dev[1]));
        LOGD_CAMHW_SUBM(ISP20HW_SUBM, "CIF tx: %s -> short",
                        AiqV4l2Device_getDevName(pRawStrCapUnit->_dev[0]));
    } else if (RK_AIQ_HDR_GET_WORKING_MODE(pRawStrCapUnit->_working_mode) ==
               RK_AIQ_WORKING_MODE_ISP_HDR3) {
        // use _mipi_tx_devs[0] and _mipi_tx_devs[1]
        // id0 as l, id1 as m, id2 as s
        AiqV4l2Device_t* tmp = tx_devs_tmp[2];
        tx_devs_tmp[2]       = tx_devs_tmp[0];
        tx_devs_tmp[0]       = tmp;
        LOGD_CAMHW_SUBM(ISP20HW_SUBM, "CIF tx: %s -> long",
                        AiqV4l2Device_getDevName(pRawStrCapUnit->_dev[2]));
        LOGD_CAMHW_SUBM(ISP20HW_SUBM, "CIF tx: %s -> middle",
                        AiqV4l2Device_getDevName(pRawStrCapUnit->_dev[1]));
        LOGD_CAMHW_SUBM(ISP20HW_SUBM, "CIF tx: %s -> short",
                        AiqV4l2Device_getDevName(pRawStrCapUnit->_dev[0]));
    } else {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "wrong hdr mode: %d\n", pRawStrCapUnit->_working_mode);
    }

    int i = 0;
    for (i = 0; i < 3; i++) {
        if (tx_devs_tmp[i]) {
            pRawStrCapUnit->_dev[i]       = tx_devs_tmp[i];
            pRawStrCapUnit->_dev_index[i] = i;
            AiqRawStream_deinit(pRawStrCapUnit->_stream[i]);
            AiqRawStream_init(pRawStrCapUnit->_stream[i], pRawStrCapUnit->_dev[i], ISP_POLL_TX, i);
            AiqPollCallback_t* pcb = pRawStrCapUnit->_pcb[i];
            if (!pcb) {
                pcb                    = (AiqPollCallback_t*)aiq_mallocz(sizeof(AiqPollCallback_t));
                pcb->_pCtx             = pRawStrCapUnit;
                pcb->poll_buffer_ready = RawStreamCapUnit_poll_buffer_ready;
                pcb->poll_event_ready  = NULL;
                pRawStrCapUnit->_pcb[i] = pcb;
            }
            AiqPollThread_setPollCallback(pRawStrCapUnit->_stream[i]->_base._poll_thread, pcb);
        }
    }
    LOGD_CAMHW_SUBM(ISP20HW_SUBM, "%s exit", __FUNCTION__);
}

void AiqRawStreamCapUnit_set_working_mode(AiqRawStreamCapUnit_t* pRawStrCapUnit, int mode) {
    LOGD_CAMHW_SUBM(ISP20HW_SUBM, "%s enter,mode=0x%x", __FUNCTION__, mode);
    pRawStrCapUnit->_working_mode = mode;

    switch (pRawStrCapUnit->_working_mode) {
        case RK_AIQ_ISP_HDR_MODE_3_FRAME_HDR:
        case RK_AIQ_ISP_HDR_MODE_3_LINE_HDR:
            pRawStrCapUnit->_mipi_dev_max = 3;
            break;
        case RK_AIQ_ISP_HDR_MODE_2_FRAME_HDR:
        case RK_AIQ_ISP_HDR_MODE_2_LINE_HDR:
            pRawStrCapUnit->_mipi_dev_max = 2;
            break;
        default:
            pRawStrCapUnit->_mipi_dev_max = 1;
    }
    LOGD_CAMHW_SUBM(ISP20HW_SUBM, "%s exit", __FUNCTION__);
}

void AiqRawStreamCapUnit_set_tx_devices(AiqRawStreamCapUnit_t* pRawStrCapUnit,
                                        AiqV4l2Device_t* mipi_tx_devs[3]) {
    int i = 0;
    for (i = 0; i < 3; i++) {
        if (pRawStrCapUnit->_dev[i]) {
            if (!pRawStrCapUnit->_isExtDev) {
                pRawStrCapUnit->_dev[i]->close(pRawStrCapUnit->_dev[i]);
            }
            AiqRawStream_deinit(pRawStrCapUnit->_stream[i]);
            if (!pRawStrCapUnit->_isExtDev) {
                aiq_free(pRawStrCapUnit->_dev[i]);
            }
            pRawStrCapUnit->_isExtDev = true;
            pRawStrCapUnit->_dev[i]   = mipi_tx_devs[i];
            AiqRawStream_init(pRawStrCapUnit->_stream[i], pRawStrCapUnit->_dev[i], ISP_POLL_TX, i);
            AiqPollCallback_t* pcb = pRawStrCapUnit->_pcb[i];
            if (!pcb) {
                pcb                    = (AiqPollCallback_t*)aiq_mallocz(sizeof(AiqPollCallback_t));
                pcb->_pCtx             = pRawStrCapUnit;
                pcb->poll_buffer_ready = RawStreamCapUnit_poll_buffer_ready;
                pcb->poll_event_ready  = NULL;
                pRawStrCapUnit->_pcb[i] = pcb;
            }
            AiqPollThread_setPollCallback(pRawStrCapUnit->_stream[i]->_base._poll_thread, pcb);
        }
    }
}

AiqV4l2Device_t* AiqRawStreamCapUnit_get_tx_device(AiqRawStreamCapUnit_t* pRawStrCapUnit,
                                                   int index) {
    if (index > pRawStrCapUnit->_mipi_dev_max)
        return NULL;
    else
        return pRawStrCapUnit->_dev[index];
}

XCamReturn AiqRawStreamCapUnit_set_tx_format(AiqRawStreamCapUnit_t* pRawStrCapUnit,
                                             const struct v4l2_subdev_format* sns_sd_fmt,
                                             uint32_t sns_v4l_pix_fmt) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    int i          = 0;
    for (i = 0; i < 3; i++) {
        if (!pRawStrCapUnit->_dev[i]) {
            ret = XCAM_RETURN_ERROR_PARAM;
            break;
        }

        if (pRawStrCapUnit->_camHw->_airms_en) {
            int mem_mode = CSI_LVDS_MEM_WORD_LOW_ALIGN;
            int ret1     = pRawStrCapUnit->_dev[i]->io_control(
            pRawStrCapUnit->_dev[i], RKCIF_CMD_SET_CSI_MEMORY_MODE, &mem_mode);
        }

        ret = AiqV4l2Device_setFmt(pRawStrCapUnit->_dev[i], sns_sd_fmt->format.width,
                                   sns_sd_fmt->format.height, sns_v4l_pix_fmt, V4L2_FIELD_NONE, 0);
        if (ret < 0) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "device(%s) set format failed",
                            AiqV4l2Device_getDevName(pRawStrCapUnit->_dev[i]));
        } else {
            LOGD_CAMHW_SUBM(ISP20HW_SUBM, "device(%s) set fmt info: fmt 0x%x, %dx%d !",
                            AiqV4l2Device_getDevName(pRawStrCapUnit->_dev[i]), sns_v4l_pix_fmt,
                            sns_sd_fmt->format.width, sns_sd_fmt->format.height);
        }
    }

    return ret;
}

XCamReturn AiqRawStreamCapUnit_set_tx_format2(AiqRawStreamCapUnit_t* pRawStrCapUnit,
                                              const struct v4l2_subdev_selection* sns_sd_sel,
                                              uint32_t sns_v4l_pix_fmt) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    int i = 0;
    for (i = 0; i < 3; i++) {
        if (!pRawStrCapUnit->_dev[i]) {
            ret = XCAM_RETURN_ERROR_PARAM;
            break;
        }

        if (pRawStrCapUnit->_camHw->_airms_en) {
            int mem_mode = CSI_LVDS_MEM_WORD_LOW_ALIGN;
            int ret1     = pRawStrCapUnit->_dev[i]->io_control(
            pRawStrCapUnit->_dev[i], RKCIF_CMD_SET_CSI_MEMORY_MODE, &mem_mode);
        }

        ret = AiqV4l2Device_setFmt(pRawStrCapUnit->_dev[i], sns_sd_sel->r.width,
                                   sns_sd_sel->r.height, sns_v4l_pix_fmt, V4L2_FIELD_NONE, 0);
        if (ret < 0) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "device(%s) set format failed",
                            AiqV4l2Device_getDevName(pRawStrCapUnit->_dev[i]));
        } else {
            LOGD_CAMHW_SUBM(ISP20HW_SUBM, "device(%s) set fmt info: fmt 0x%x, %dx%d !",
                            AiqV4l2Device_getDevName(pRawStrCapUnit->_dev[i]), sns_v4l_pix_fmt,
                            sns_sd_sel->r.width, sns_sd_sel->r.height);
        }
    }

    return ret;
}

void AiqRawStreamCapUnit_set_devices(AiqRawStreamCapUnit_t* pRawStrCapUnit,
                                     AiqV4l2SubDevice_t* ispdev, AiqCamHwBase_t* handle,
                                     AiqRawStreamProcUnit_t* proc, AiqAiRmsStreamProcUnit_t* pAirmsStream) {
    pRawStrCapUnit->_isp_core_dev = ispdev;
    pRawStrCapUnit->_camHw        = handle;
    pRawStrCapUnit->_proc_stream  = proc;
    pRawStrCapUnit->_pAirmsStream  = pAirmsStream;
}

void AiqRawStreamCapUnit_skip_frames(AiqRawStreamCapUnit_t* pRawStrCapUnit, int skip_num,
                                     int32_t skip_seq) {
    aiqMutex_lock(&pRawStrCapUnit->_mipi_mutex);
    pRawStrCapUnit->_skip_num    = skip_num;
    pRawStrCapUnit->_skip_to_seq = skip_seq + skip_num;
    aiqMutex_unlock(&pRawStrCapUnit->_mipi_mutex);
}

void AiqRawStreamCapUnit_stop_vicap_stream_only(AiqRawStreamCapUnit_t* pRawStrCapUnit) {

    int skip_frm = 0;
    if (pRawStrCapUnit->_dev[0]) {
        pRawStrCapUnit->_dev[0]->io_control(pRawStrCapUnit->_dev[0], RKCIF_CMD_SET_SENSOR_FLIP_START,
                                            &skip_frm);
    }
}

void AiqRawStreamCapUnit_skip_frame_and_restart_vicap_stream(AiqRawStreamCapUnit_t* pRawStrCapUnit, int skip_frm_cnt) {

    if (skip_frm_cnt < 2) {
        skip_frm_cnt = 2;
    }

    if (pRawStrCapUnit->_dev[0]) {
        pRawStrCapUnit->_dev[0]->io_control(pRawStrCapUnit->_dev[0], RKCIF_CMD_SET_SENSOR_FLIP_END,
                                            &skip_frm_cnt);
    }
}

XCamReturn AiqRawStreamCapUnit_reset_hardware(AiqRawStreamCapUnit_t* pRawStrCapUnit) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (pRawStrCapUnit->_dev[0]) {
        int32_t reset = 1;
        int32_t b_ret = 0;

        errno = 0;
        b_ret = pRawStrCapUnit->_dev[0]->io_control(pRawStrCapUnit->_dev[0], RKCIF_CMD_SET_RESET,
                                                    &reset);
        if (b_ret < 0) {
            LOGE_CAMHW_SUBM(SENSOR_SUBM, "device(%s) reset failed: %d (%s)!",
                            AiqV4l2Device_getDevName(pRawStrCapUnit->_dev[0]), b_ret,
                            strerror(errno));
            return XCAM_RETURN_ERROR_IOCTL;
        }

        LOGD_CAMHW_SUBM(SENSOR_SUBM, "device(%s) reset",
                        AiqV4l2Device_getDevName(pRawStrCapUnit->_dev[0]));
    }

    return ret;
}

XCamReturn AiqRawStreamCapUnit_set_csi_mem_word_big_align(AiqRawStreamCapUnit_t* pRawStrCapUnit,
                                                          uint32_t width, uint32_t height,
                                                          uint32_t sns_v4l_pix_fmt,
                                                          int8_t sns_bpp) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    LOGD_CAMHW_SUBM(ISP20HW_SUBM, "sensor fmt 0x%x, %dx%d, sns_bpp: %d", sns_v4l_pix_fmt, width,
                    height, sns_bpp);

    int i = 0;
    for (i = 0; i < 3; i++) {
        if (!pRawStrCapUnit->_dev[i]) {
            ret = XCAM_RETURN_BYPASS;
            break;
        }

        if (((width / 2 - RKMOUDLE_UNITE_EXTEND_PIXEL) * sns_bpp / 8) & 0xf) {
            int mem_mode = CSI_LVDS_MEM_WORD_HIGH_ALIGN;
            int ret1     = pRawStrCapUnit->_dev[i]->io_control(
                pRawStrCapUnit->_dev[i], RKCIF_CMD_SET_CSI_MEMORY_MODE, &mem_mode);
            if (ret1) {
                LOGE_CAMHW_SUBM(ISP20HW_SUBM, "set CSI_MEM_WORD_BIG_ALIGN failed!\n");
                ret = XCAM_RETURN_ERROR_IOCTL;
            } else {
                LOGD_CAMHW_SUBM(ISP20HW_SUBM, "set the memory mode of vicap to big align");
#if RKAIQ_HAVE_DUMPSYS
                pRawStrCapUnit->data_mode = mem_mode;
#endif
            }
        }
    }

    return ret;
}

XCamReturn AiqRawStreamCapUnit_setVicapStreamMode(AiqRawStreamCapUnit_t* pRawStrCapUnit, int mode,
                                                  uint32_t* frameId, bool isSingleMode) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    struct rkcif_quick_stream_param info;

    if (!pRawStrCapUnit->_dev[0]) {
        LOGE_CAMHW("dev[0] is null pointer");
        return XCAM_RETURN_ERROR_FAILED;
    }

    info.on = mode;
    if (isSingleMode)
        info.resume_mode = RKISP_RTT_MODE_ONE_FRAME;
    else
        info.resume_mode = RKISP_RTT_MODE_MULTI_FRAME;

    if (pRawStrCapUnit->_dev[0]->io_control(pRawStrCapUnit->_dev[0], RKCIF_CMD_SET_QUICK_STREAM,
                                            &info) < 0) {
        LOGE_CAMHW("dev(%s) ioctl faile, set vicap %s faile",
                   AiqV4l2Device_getDevName(pRawStrCapUnit->_dev[0]), mode ? "resume" : "pause");
        ret = XCAM_RETURN_ERROR_IOCTL;
    }
    if (frameId) *frameId = info.frame_num;
    return ret;
}

void AiqRawStreamCapUnit_setSensorCategory(AiqRawStreamCapUnit_t* pRawStrCapUnit,
                                           bool sensorState) {
    pRawStrCapUnit->_is_1608_stream = sensorState;
}

void AiqRawStreamCapUnit_setCamPhyId(AiqRawStreamCapUnit_t* pRawStrCapUnit, int phyId) {
    pRawStrCapUnit->mCamPhyId = phyId;
}

void AiqRawStreamCapUnit_setTxBufferCnt(AiqRawStreamCapUnit_t* pRawStrCapUnit, uint16_t buf_num) {
    for (int i = 0; i < 3; i++) {
        if (pRawStrCapUnit->_dev[i]) {
            AiqV4l2Device_setBufCnt(pRawStrCapUnit->_dev[i], buf_num);
        }
    }
}

XCamReturn AiqRawStreamCapUnit_setSwStreamInfo(AiqRawStreamCapUnit_t* pRawStrCapUnit, void* sw_stream_ctx, rawStream_send_sync_buf_func send_sync_buf_func)
{
    pRawStrCapUnit->_sw_stream_ctx = sw_stream_ctx;
    pRawStrCapUnit->_send_sync_buf_func = send_sync_buf_func;

    return XCAM_RETURN_NO_ERROR;
}

#if RKAIQ_HAVE_DUMPSYS
int AiqRawStreamCapUnit_dump(void* dumper, st_string* result, int argc, void* argv[]) {
    AiqRawStreamCapUnit_t* pRawStrCapUnit = (AiqRawStreamCapUnit_t*)dumper;
    if (pRawStrCapUnit->_state != RAW_CAP_STATE_STARTED) return 0;

    stream_cap_dump_mod_param(pRawStrCapUnit, result);
    stream_cap_dump_dev_attr(pRawStrCapUnit, result);
    stream_cap_dump_chn_attr(pRawStrCapUnit, result);
    stream_cap_dump_chn_status(pRawStrCapUnit, result);
    stream_cap_dump_videobuf_status(pRawStrCapUnit, result);

    return 0;
}
#endif
