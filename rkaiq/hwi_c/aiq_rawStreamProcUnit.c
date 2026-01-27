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

#include "aiq_rawStreamProcUnit.h"

#if RKAIQ_HAVE_DUMPSYS
#include "aiq_streamProcInfo.h"
#endif
#include "c_base/aiq_list.h"
#include "c_base/aiq_map.h"
#include "c_base/aiq_thread.h"
#include "hwi_c/aiq_CamHwBase.h"
#include "hwi_c/aiq_camHw.h"
#include "hwi_c/aiq_lensHw.h"
#include "hwi_c/aiq_rawStreamProcUnit.h"
#include "hwi_c/aiq_stream.h"
#include "include/common/rkisp2-config.h"
#include "xcore_c/aiq_v4l2_buffer.h"
#include "xcore_c/aiq_v4l2_device.h"

#ifdef ANDROID_OS
#include <cutils/properties.h>
#ifndef PROPERTY_VALUE_MAX
#define PROPERTY_VALUE_MAX 32
#endif
#endif

static XCamReturn DumpRkRawInFrameEnd(AiqRawStreamProcUnit_t* pRawStrProcUnit,
                                      AiqV4l2Buffer_t* rx_buf) {
    uint32_t sequence = 0;

    if (rx_buf) sequence = AiqV4l2Buffer_getSequence(rx_buf);

    AiqSensorHw_t* mSensorSubdev = pRawStrProcUnit->_camHw->_mSensorDev;
    AiqSensorExpInfo_t* expParam = NULL;
    if (mSensorSubdev) {
        expParam = mSensorSubdev->getEffectiveExpParams(mSensorSubdev, sequence);
        if (!expParam)
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "frame_id(%d), get exposure failed!!!\n", sequence);
    }

    aiq_isp_effect_params_t* ispParams = NULL;
    AiqCamHw_getEffectiveIspParams(pRawStrProcUnit->_camHw, &ispParams, sequence);

#ifndef UseCaptureRawData
    XCamReturn ret =
        aiq_dumpRkRaw_isDumpInFrameEnd(&pRawStrProcUnit->_rawCap, sequence, ispParams, expParam);
    if (ret == XCAM_RETURN_NO_ERROR) {
        AiqAfInfoWrapper_t* afParams = NULL;
        if (pRawStrProcUnit->_camHw->mLensDev)
            AiqLensHw_getAfInfoParams(pRawStrProcUnit->_camHw->mLensDev, sequence, afParams);

        pRawStrProcUnit->_rawCap.dumpMetadataBlock(&pRawStrProcUnit->_rawCap, sequence, ispParams,
                                                   expParam, afParams);
#ifdef ISP_REGS_BASE
        if (pRawStrProcUnit->_rawCap.dumpIspRegBlock)
            pRawStrProcUnit->_rawCap.dumpIspRegBlock(&pRawStrProcUnit->_rawCap, ISP_REGS_BASE, 0,
                                                     ISP_REGS_SIZE, sequence);
#endif
        if (pRawStrProcUnit->_rawCap.dumpPlatformInfoBlock)
            pRawStrProcUnit->_rawCap.dumpPlatformInfoBlock(&pRawStrProcUnit->_rawCap);
        aiq_dumpRkRaw_updateDumpStatus(&pRawStrProcUnit->_rawCap);
        aiq_dumpRkRaw_notifyDumpRaw(&pRawStrProcUnit->_rawCap);
    }
#endif

    if (expParam) AIQ_REF_BASE_UNREF(&(expParam)->_base._ref_base);
    if (ispParams) AIQ_REF_BASE_UNREF(&(ispParams)->_ref_base);

    return XCAM_RETURN_NO_ERROR;
}

static int8_t getDumpRkRawType(AiqRawStreamProcUnit_t* pRawStrProcUnit) {
#ifdef ANDROID_OS
    char property_value[PROPERTY_VALUE_MAX] = {0};

    property_get("persist.vendor.rkisp.rkraw.type", property_value, "1");
    pRawStrProcUnit->dumpRkRawType = strtoull(property_value, NULL, 16);
#else
    char* valueStr = getenv("rkisp_dump_rkraw_type");
    if (valueStr) pRawStrProcUnit->dumpRkRawType = strtoull(valueStr, NULL, 16);
#endif

    LOGD_CAMHW_SUBM(ISP20HW_SUBM, "dumpRkRawType: %d", pRawStrProcUnit->dumpRkRawType);

    return pRawStrProcUnit->dumpRkRawType;
}

void trigger_isp_readback(AiqRawStreamProcUnit_t* pRawStrProcUnit) {
    AiqMapItem_t* mapItem = NULL;
    uint32_t sequence     = -1;
    AiqV4l2Buffer_t* v4l2_buf[3];
    AiqListItem_t* buf_proxy = NULL;

    aiqMutex_lock(&pRawStrProcUnit->_buf_mutex);

    if (!aiqMap_size(pRawStrProcUnit->_isp_hdr_fid2ready_map)) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "%s buf not ready !", __func__);
        aiqMutex_unlock(&pRawStrProcUnit->_buf_mutex);
        return;
    }

RESTART:
    mapItem  = aiqMap_begin(pRawStrProcUnit->_isp_hdr_fid2ready_map);
    sequence = (intptr_t)mapItem->_key;

    aiqMap_erase(pRawStrProcUnit->_isp_hdr_fid2ready_map, (void*)(intptr_t)sequence);

    if (pRawStrProcUnit->_camHw) {
        {
            int ret = XCAM_RETURN_NO_ERROR;

            // whether to start capturing raw files
            aiq_dumpRkRaw_detectDumpStatus(&pRawStrProcUnit->_rawCap, sequence);

            // CaptureRawData::getInstance().detect_capture_raw_status(sequence, _first_trigger);
            //_camHw->setIsppConfig(sequence);

            AiqV4l2Buffer_t* v4l2_tx_buf = NULL;
            for (int i = 0; i < pRawStrProcUnit->_mipi_dev_max; i++) {
                buf_proxy = aiqList_get_item(pRawStrProcUnit->cache_list[i], NULL);
                v4l2_tx_buf                  = *(AiqV4l2Buffer_t**)(buf_proxy->_pData);
                v4l2_buf[i]                  = AiqV4l2Device_getBuf(pRawStrProcUnit->_dev[i],
                                                   AiqV4l2Buffer_getV4lBufIndex(v4l2_tx_buf));
                if (!v4l2_buf[i]) {
                    LOGE_CAMHW_SUBM(ISP20HW_SUBM, "Rx[%d] can not get buffer\n", i);
                    goto out;
                } else {
                    aiqList_erase_item(pRawStrProcUnit->cache_list[i], buf_proxy);
                    ret = aiqList_push(pRawStrProcUnit->buf_list[i], &v4l2_tx_buf);
                    if (ret) LOGE_CAMHW_SUBM(ISP20HW_SUBM, "buf_list:%d push error \n", i);

                    enum v4l2_memory mem_type = AiqV4l2Device_getMemType(pRawStrProcUnit->_dev[i]);
                    if (mem_type == V4L2_MEMORY_USERPTR)
                        AiqV4l2Buffer_setExpbufUsrptr(v4l2_buf[i], v4l2_tx_buf->_expbuf_usrptr);
                    else if (mem_type == V4L2_MEMORY_DMABUF) {
                        AiqV4l2Buffer_setExpbufFd(v4l2_buf[i], v4l2_tx_buf->_expbuf_fd);
                    } else if (mem_type == V4L2_MEMORY_MMAP) {
                        if (pRawStrProcUnit->_dev[i]->_use_type == 1) {
                            memcpy((void*)v4l2_buf[i]->_expbuf_usrptr,
                                   (void*)v4l2_tx_buf->_expbuf_usrptr,
                                   AiqV4l2Buffer_getV4lBufPlanarLength(v4l2_buf[i], 0));
                            AiqV4l2Buffer_setReserved(v4l2_buf[i],
                                                      (uint8_t*)v4l2_tx_buf->_expbuf_usrptr);
                        }
                    }

                    if (pRawStrProcUnit->_rawCap.dumpRkRawBlock) {
                        uintptr_t expbuf_usrptr =
                            AiqV4l2Buffer_getExpbufUsrptr(*(AiqV4l2Buffer_t**)(buf_proxy->_pData));
                        pRawStrProcUnit->_rawCap.dumpRkRawBlock(
                            &pRawStrProcUnit->_rawCap, i, sequence, expbuf_usrptr,
                            v4l2_buf[i]->_buf.m.planes[0].bytesused);
                    }
                    // CaptureRawData::getInstance().dynamic_capture_raw(i, sequence, buf_proxy,
                    // v4l2buf[i],_mipi_dev_max,_working_mode,_dev[0]);
                }
            }

            for (int i = 0; i < pRawStrProcUnit->_mipi_dev_max; i++) {
                ret = AiqV4l2Device_qbuf(pRawStrProcUnit->_dev[i], v4l2_buf[i], true);
                if (ret != XCAM_RETURN_NO_ERROR) {
                    aiqList_erase(pRawStrProcUnit->buf_list[i], &v4l2_tx_buf);
                    AiqV4l2Buffer_unref(v4l2_tx_buf);
                    LOGE_CAMHW_SUBM(ISP20HW_SUBM, "Rx[%d] queue buffer failed\n", i);
                    break;
                }
            }

            int32_t dev_max             = pRawStrProcUnit->_mipi_dev_max;
            struct isp2x_csi_trigger tg = {
                .sof_timestamp   = 0,
                .frame_timestamp = 0,
                .frame_id        = sequence,
                .times           = 0,
                .mode = dev_max == 1 ? T_START_X1 : dev_max == 2 ? T_START_X2 : T_START_X3,
            };

            tg.times           = 1;
            tg.frame_timestamp = AiqVideoBuffer_getTimestamp(&v4l2_tx_buf->_vb) * 1000;
            tg.sof_timestamp   = tg.frame_timestamp;
            // tg.times = 1;//fixed to three times readback
            LOGD_CAMHW_SUBM(ISP20HW_SUBM,
                            "camId: %d, frameId: %d: sof_ts %" PRId64 "ms, frame_ts %" PRId64
                            "ms, trigger readback times: %d\n",
                            pRawStrProcUnit->mCamPhyId, sequence, tg.sof_timestamp / 1000 / 1000,
                            tg.frame_timestamp / 1000 / 1000, tg.times);

            if (ret == XCAM_RETURN_NO_ERROR) {
                pRawStrProcUnit->_isp_core_dev->_v4l_base.io_control(
                    &pRawStrProcUnit->_isp_core_dev->_v4l_base, RKISP_CMD_TRIGGER_READ_BACK, &tg);
#if RKAIQ_HAVE_DUMPSYS
                {
                    struct timespec time;
                    clock_gettime(CLOCK_MONOTONIC, &time);

                    pRawStrProcUnit->trig.frameloss +=
                        tg.frame_id ? tg.frame_id - pRawStrProcUnit->trig.id - 1 : 0;
                    pRawStrProcUnit->trig.id        = tg.frame_id;
                    pRawStrProcUnit->trig.timestamp = tg.frame_timestamp / 1000;
                    pRawStrProcUnit->trig.delay =
                        XCAM_TIMESPEC_2_USEC(time) - tg.frame_timestamp / 1000;
                }

                pRawStrProcUnit->trig_times = tg.times;
#endif
            } else {
                LOGE_CAMHW_SUBM(ISP20HW_SUBM, "%s frame[%d] queue  failed, don't read back!\n",
                                __func__, sequence);
            }

            aiqMutex_unlock(&pRawStrProcUnit->_buf_mutex);
            aiq_dumpRkRaw_waitDumpRawDone(&pRawStrProcUnit->_rawCap);
            aiq_dumpRkRaw_waitThirdPartyDumpDone(&pRawStrProcUnit->_rawCap,
                                                 pRawStrProcUnit->_first_trigger);
            aiqMutex_lock(&pRawStrProcUnit->_buf_mutex);
            // CaptureRawData::getInstance().update_capture_raw_status(_first_trigger);
        }
    }

    if (aiqMap_size(pRawStrProcUnit->_isp_hdr_fid2ready_map) > 0) {
        goto RESTART;
    }
out:
    aiqMutex_unlock(&pRawStrProcUnit->_buf_mutex);
    return;
}

static bool raw_buffer_proc(void* args) {
    AiqRawStreamProcUnit_t* pRawStrProcUnit = (AiqRawStreamProcUnit_t*)args;

    aiqMutex_lock(&pRawStrProcUnit->_buf_mutex);
    if (pRawStrProcUnit->_isRawProcThQuit) {
        LOGK_CAMHW_SUBM(ISP20HW_SUBM, "RawProcTh quit !");
        aiqMutex_unlock(&pRawStrProcUnit->_buf_mutex);
        return false;
    }

    int ret = 0;
    if (aiqMap_size(pRawStrProcUnit->_isp_hdr_fid2ready_map) == 0)
        ret = aiqCond_timedWait(&pRawStrProcUnit->_buf_cond, &pRawStrProcUnit->_buf_mutex, -1);
    aiqMutex_unlock(&pRawStrProcUnit->_buf_mutex);

    if (ret) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "RawProcTh quit by erro %s !", ret, strerror(ret));
        return false;
    }

    trigger_isp_readback(pRawStrProcUnit);

    return true;
}

XCamReturn RawStreamProcUnit_poll_buffer_ready(void* ctx, AiqHwEvt_t* evt, int dev_index) {
    AiqV4l2Buffer_t* rx_buf                 = NULL;
    AiqRawStreamProcUnit_t* pRawStrProcUnit = (AiqRawStreamProcUnit_t*)ctx;

#if RKAIQ_HAVE_DUMPSYS
    // dump fe info
    {
        struct timespec time;
        clock_gettime(CLOCK_MONOTONIC, &time);

        if (evt->frame_id > pRawStrProcUnit->fe.id)
            pRawStrProcUnit->fe.frameloss +=
                evt->frame_id ? evt->frame_id - pRawStrProcUnit->fe.id - 1 : 0;
        pRawStrProcUnit->fe.id        = evt->frame_id;
        pRawStrProcUnit->fe.timestamp = evt->mTimestamp;
        // pRawStrProcUnit->fe.delay     = XCAM_TIMESPEC_2_USEC(time) - evt->mTimestamp;
        pRawStrProcUnit->fe.delay = 0;
    }
#endif

    aiqMutex_lock(&pRawStrProcUnit->_buf_mutex);
    if (aiqList_size(pRawStrProcUnit->buf_list[dev_index]) > 0) {
        aiqList_get(pRawStrProcUnit->buf_list[dev_index], &rx_buf);
        if (dev_index == 0) DumpRkRawInFrameEnd(pRawStrProcUnit, rx_buf);
        LOGV_CAMHW_SUBM(ISP20HW_SUBM, "%s dev_index:%d index:%d fd:%d, seq:%d\n", __func__,
                        dev_index, AiqV4l2Buffer_getV4lBufIndex(rx_buf),
                        AiqV4l2Buffer_getExpbufFd(rx_buf), AiqV4l2Buffer_getSequence(rx_buf));
    }
    if (pRawStrProcUnit->_PollCallback)
        pRawStrProcUnit->_PollCallback->poll_buffer_ready(pRawStrProcUnit->_PollCallback->_pCtx,
                                                          evt, dev_index);

    AiqV4l2Buffer_unref(rx_buf);
    aiqMutex_unlock(&pRawStrProcUnit->_buf_mutex);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqRawStreamProcUnit_init(AiqRawStreamProcUnit_t* pRawStrProcUnit,
                                     const rk_sensor_full_info_t* s_info, bool linked_to_isp,
                                     int tx_buf_cnt) {
    pRawStrProcUnit->_first_trigger = true;
    pRawStrProcUnit->mCamPhyId      = -1;
    pRawStrProcUnit->dumpRkRawType  = DUMP_RKRAW_DEFAULT;
    pRawStrProcUnit->_isExtDev      = false;
    pRawStrProcUnit->_isRawProcThQuit = true;
#if RKAIQ_HAVE_DUMPSYS
    pRawStrProcUnit->data_mode = 0;
#endif

    pRawStrProcUnit->_raw_proc_thread = aiqThread_init("RawProc", raw_buffer_proc, pRawStrProcUnit);
    aiqThread_setPolicy(pRawStrProcUnit->_raw_proc_thread, SCHED_RR);
    aiqThread_setPriority(pRawStrProcUnit->_raw_proc_thread, 20);

    aiqMutex_init(&pRawStrProcUnit->_buf_mutex);
    aiqMutex_init(&pRawStrProcUnit->_mipi_trigger_mutex);
    aiqCond_init(&pRawStrProcUnit->_buf_cond);

    // short frame
    if (strlen(s_info->isp_info->rawrd2_s_path)) {
        pRawStrProcUnit->_dev[0] = (AiqV4l2Device_t*)aiq_mallocz(sizeof(AiqV4l2Device_t));
        if (!pRawStrProcUnit->_dev[0]) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "%d: alloc fail !", __LINE__);
            goto fail;
        }
        AiqV4l2Device_init(pRawStrProcUnit->_dev[0], s_info->isp_info->rawrd2_s_path);
        pRawStrProcUnit->_dev[0]->open(pRawStrProcUnit->_dev[0], false);
        AiqV4l2Device_setMemType(pRawStrProcUnit->_dev[0], V4L2_MEMORY_DMABUF);
    }
    // mid frame
    if (strlen(s_info->isp_info->rawrd0_m_path)) {
        pRawStrProcUnit->_dev[1] = (AiqV4l2Device_t*)aiq_mallocz(sizeof(AiqV4l2Device_t));
        if (!pRawStrProcUnit->_dev[1]) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "%d: alloc fail !", __LINE__);
            goto fail;
        }
        AiqV4l2Device_init(pRawStrProcUnit->_dev[1], s_info->isp_info->rawrd0_m_path);
        pRawStrProcUnit->_dev[1]->open(pRawStrProcUnit->_dev[1], false);
        AiqV4l2Device_setMemType(pRawStrProcUnit->_dev[1], V4L2_MEMORY_DMABUF);
    }
    // long frame
    if (strlen(s_info->isp_info->rawrd1_l_path)) {
        pRawStrProcUnit->_dev[2] = (AiqV4l2Device_t*)aiq_mallocz(sizeof(AiqV4l2Device_t));
        if (!pRawStrProcUnit->_dev[2]) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "%d: alloc fail !", __LINE__);
            goto fail;
        }
        AiqV4l2Device_init(pRawStrProcUnit->_dev[2], s_info->isp_info->rawrd1_l_path);
        pRawStrProcUnit->_dev[2]->open(pRawStrProcUnit->_dev[2], false);
        AiqV4l2Device_setMemType(pRawStrProcUnit->_dev[2], V4L2_MEMORY_DMABUF);
    }

    int buf_cnt = 0;
    if (tx_buf_cnt > 0) {
        buf_cnt = tx_buf_cnt;
    } else if (linked_to_isp) {
        buf_cnt = ISP_TX_BUF_NUM;
    } else if (s_info->linked_to_1608) {
        buf_cnt = VIPCAP_TX_BUF_NUM_1608;
    } else {
        buf_cnt = VIPCAP_TX_BUF_NUM;
    }
    for (int i = 0; i < 3; i++) {
        if (pRawStrProcUnit->_dev[i]) {
            AiqV4l2Device_setBufCnt(pRawStrProcUnit->_dev[i], buf_cnt);
            AiqV4l2Device_setBufSync(pRawStrProcUnit->_dev[i], true);

            pRawStrProcUnit->_dev_index[i] = i;
            pRawStrProcUnit->_stream[i]    = (AiqRawStream_t*)aiq_mallocz(sizeof(AiqRawStream_t));
            if (!pRawStrProcUnit->_stream[i]) {
                LOGE_CAMHW_SUBM(ISP20HW_SUBM, "%d: alloc fail !", __LINE__);
                goto fail;
            }
            AiqRawStream_init(pRawStrProcUnit->_stream[i], pRawStrProcUnit->_dev[i], ISP_POLL_RX,
                              i);
            AiqPollCallback_t* pcb   = (AiqPollCallback_t*)aiq_mallocz(sizeof(AiqPollCallback_t));
            pcb->_pCtx               = pRawStrProcUnit;
            pcb->poll_buffer_ready   = RawStreamProcUnit_poll_buffer_ready;
            pcb->poll_event_ready    = NULL;
            pRawStrProcUnit->_pcb[i] = pcb;
            AiqPollThread_setPollCallback(pRawStrProcUnit->_stream[i]->_base._poll_thread, pcb);
        }
    }

    pRawStrProcUnit->_is_split = s_info->split;

    return XCAM_RETURN_NO_ERROR;
fail:
    AiqRawStreamProcUnit_deinit(pRawStrProcUnit);
    return XCAM_RETURN_ERROR_FAILED;
}

void AiqRawStreamProcUnit_deinit(AiqRawStreamProcUnit_t* pRawStrProcUnit) {
    if (pRawStrProcUnit->_raw_proc_thread) aiqThread_deinit(pRawStrProcUnit->_raw_proc_thread);

    for (int i = 0; i < 3; i++) {
        if (pRawStrProcUnit->_pcb[i]) {
            aiq_free(pRawStrProcUnit->_pcb[i]);
            pRawStrProcUnit->_pcb[i] = NULL;
        }

        if (pRawStrProcUnit->_dev[i]) {
            pRawStrProcUnit->_dev[i]->close(pRawStrProcUnit->_dev[i]);
            if (pRawStrProcUnit->_stream[i]) {
                AiqRawStream_deinit(pRawStrProcUnit->_stream[i]);
                aiq_free(pRawStrProcUnit->_stream[i]);
                pRawStrProcUnit->_stream[i] = NULL;
            }
            AiqV4l2Device_deinit(pRawStrProcUnit->_dev[i]);
            aiq_free(pRawStrProcUnit->_dev[i]);
            pRawStrProcUnit->_dev[i] = NULL;
        }

        if (pRawStrProcUnit->buf_list[i]) {
            aiqList_deinit(pRawStrProcUnit->buf_list[i]);
        }

        if (pRawStrProcUnit->cache_list[i]) {
            aiqList_deinit(pRawStrProcUnit->cache_list[i]);
        }
    }

    if (pRawStrProcUnit->_isp_hdr_fid2ready_map)
        aiqMap_deinit(pRawStrProcUnit->_isp_hdr_fid2ready_map);

    aiqMutex_deInit(&pRawStrProcUnit->_buf_mutex);
    aiqMutex_deInit(&pRawStrProcUnit->_mipi_trigger_mutex);
    aiqCond_deInit(&pRawStrProcUnit->_buf_cond);
}

static XCamReturn RawStreamProcUnit_setIspInfoToDump(AiqRawStreamProcUnit_t* pRawStrProcUnit) {
    if (!pRawStrProcUnit) return XCAM_RETURN_ERROR_FAILED;
    if (!pRawStrProcUnit->_dev[0] || !pRawStrProcUnit->_camHw) return XCAM_RETURN_ERROR_FAILED;

    struct ispInfo_s ispInfo;
    struct v4l2_format format;
    memset(&format, 0, sizeof(format));

    if (pRawStrProcUnit->_dev[0]->get_format(pRawStrProcUnit->_dev[0], &format) < 0)
        return XCAM_RETURN_ERROR_FAILED;

    ispInfo.sns_width     = format.fmt.pix.width;
    ispInfo.sns_height    = format.fmt.pix.height;
    ispInfo.pixelformat   = format.fmt.pix.pixelformat;
    ispInfo.working_mode  = pRawStrProcUnit->_working_mode;
    ispInfo.mCamPhyId     = pRawStrProcUnit->mCamPhyId;
    ispInfo.stridePerLine = format.fmt.pix_mp.plane_fmt[0].bytesperline;
    if (strlen(pRawStrProcUnit->_camHw->sns_name) > 0)
        sscanf(&pRawStrProcUnit->_camHw->sns_name[6], "%s", ispInfo.sns_name);

#ifndef UseCaptureRawData
    aiq_dumpRkRaw_setIspInfo(&pRawStrProcUnit->_rawCap, &ispInfo);
#endif

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqRawStreamProcUnit_start(AiqRawStreamProcUnit_t* pRawStrProcUnit, int mode) {
    if (getDumpRkRawType(pRawStrProcUnit) == DUMP_RKRAW1)
        aiq_dumpRkRaw1_init(&pRawStrProcUnit->_rawCap, pRawStrProcUnit->mCamPhyId);
    else
        aiq_dumpRkRaw2_init(&pRawStrProcUnit->_rawCap, pRawStrProcUnit->mCamPhyId);

    RawStreamProcUnit_setIspInfoToDump(pRawStrProcUnit);

    aiqMutex_lock(&pRawStrProcUnit->_buf_mutex);
    // TODO: [baron] for rk1608, vi will capture data before start
    for (int32_t i = 0; i < pRawStrProcUnit->_mipi_dev_max; i++) {
        if (!pRawStrProcUnit->buf_list[i]) {
            AiqListConfig_t bufListCfg;
            bufListCfg._name             = "RawProcBufList";
            bufListCfg._item_nums        = 8;
            bufListCfg._item_size        = sizeof(AiqV4l2Buffer_t*);
            pRawStrProcUnit->buf_list[i] = aiqList_init(&bufListCfg);
            if (!pRawStrProcUnit->buf_list[i])
                LOGE_CAMHW_SUBM(ISP20HW_SUBM, "cId:%d init %s error", pRawStrProcUnit->mCamPhyId,
                                bufListCfg._name);
        } else {
            AiqListItem_t* pItem = NULL;
            bool rm              = false;
            AIQ_LIST_FOREACH(pRawStrProcUnit->buf_list[i], pItem, rm) {
                AiqV4l2Buffer_unref(*(AiqV4l2Buffer_t**)(pItem->_pData));
                pItem = aiqList_erase_item_locked(pRawStrProcUnit->buf_list[i], pItem);
                rm    = true;
            }
            aiqList_reset(pRawStrProcUnit->buf_list[i]);
        }

        if (!pRawStrProcUnit->cache_list[i]) {
            AiqListConfig_t bufListCfg;
            bufListCfg._name             = "RawProcBufCacheList";
            bufListCfg._item_nums        = 8;
            bufListCfg._item_size        = sizeof(AiqV4l2Buffer_t*);
            pRawStrProcUnit->cache_list[i] = aiqList_init(&bufListCfg);
            if (!pRawStrProcUnit->cache_list[i])
                LOGE_CAMHW_SUBM(ISP20HW_SUBM, "cId:%d init %s error", pRawStrProcUnit->mCamPhyId,
                                bufListCfg._name);
        } else {
            AiqListItem_t* pItem = NULL;
            bool rm              = false;
            AIQ_LIST_FOREACH(pRawStrProcUnit->cache_list[i], pItem, rm) {
                AiqV4l2Buffer_unref(*(AiqV4l2Buffer_t**)(pItem->_pData));
                pItem = aiqList_erase_item_locked(pRawStrProcUnit->cache_list[i], pItem);
                rm    = true;
            }
            aiqList_reset(pRawStrProcUnit->cache_list[i]);
        }
        pRawStrProcUnit->_stream[i]->_base.setCamPhyId(&pRawStrProcUnit->_stream[i]->_base,
                                                       pRawStrProcUnit->mCamPhyId);
        pRawStrProcUnit->_stream[i]->_base.start(&pRawStrProcUnit->_stream[i]->_base);
    }

    if (!pRawStrProcUnit->_isp_hdr_fid2ready_map) {
        AiqMapConfig_t fid2readyMapCfg;
        fid2readyMapCfg._name                   = "f2r";
        fid2readyMapCfg._item_nums              = 16;
        fid2readyMapCfg._item_size              = sizeof(bool);
        fid2readyMapCfg._key_type               = AIQ_MAP_KEY_TYPE_UINT32;
        pRawStrProcUnit->_isp_hdr_fid2ready_map = aiqMap_init(&fid2readyMapCfg);
        if (!pRawStrProcUnit->_isp_hdr_fid2ready_map)
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "cId:%d init %s error", pRawStrProcUnit->mCamPhyId,
                            fid2readyMapCfg._name);
    }

    aiqMap_reset(pRawStrProcUnit->_isp_hdr_fid2ready_map);

    pRawStrProcUnit->_isRawProcThQuit = false;

    aiqMutex_unlock(&pRawStrProcUnit->_buf_mutex);

    aiqThread_start(pRawStrProcUnit->_raw_proc_thread);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqRawStreamProcUnit_stop(AiqRawStreamProcUnit_t* pRawStrProcUnit) {
    aiqMutex_lock(&pRawStrProcUnit->_buf_mutex);
    pRawStrProcUnit->_isRawProcThQuit = true;
    aiqMutex_unlock(&pRawStrProcUnit->_buf_mutex);
    aiqCond_broadcast(&pRawStrProcUnit->_buf_cond);
    aiqThread_stop(pRawStrProcUnit->_raw_proc_thread);

    for (int i = 0; i < pRawStrProcUnit->_mipi_dev_max; i++) {
        pRawStrProcUnit->_stream[i]->_base.stopThreadOnly(&pRawStrProcUnit->_stream[i]->_base);

        AiqListItem_t* pItem = NULL;
        bool rm              = false;
        if (pRawStrProcUnit->buf_list[i]) {
            AIQ_LIST_FOREACH(pRawStrProcUnit->buf_list[i], pItem, rm) {
                AiqV4l2Buffer_unref(*(AiqV4l2Buffer_t**)(pItem->_pData));
                pItem = aiqList_erase_item_locked(pRawStrProcUnit->buf_list[i], pItem);
                rm    = true;
            }
            aiqList_reset(pRawStrProcUnit->buf_list[i]);
        }

        if (pRawStrProcUnit->cache_list[i]) {
            AIQ_LIST_FOREACH(pRawStrProcUnit->cache_list[i], pItem, rm) {
                AiqV4l2Buffer_unref(*(AiqV4l2Buffer_t**)(pItem->_pData));
                pItem = aiqList_erase_item_locked(pRawStrProcUnit->cache_list[i], pItem);
                rm    = true;
            }
            aiqList_reset(pRawStrProcUnit->cache_list[i]);
        }
    }

    if (pRawStrProcUnit->_isp_hdr_fid2ready_map)
        aiqMap_reset(pRawStrProcUnit->_isp_hdr_fid2ready_map);

    for (int i = 0; i < pRawStrProcUnit->_mipi_dev_max; i++) {
        pRawStrProcUnit->_stream[i]->_base.stopDeviceOnly(&pRawStrProcUnit->_stream[i]->_base);
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqRawStreamProcUnit_prepare(AiqRawStreamProcUnit_t* pRawStrProcUnit, int idx) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    // mipi rx/tx format should match to sensor.
    for (int i = 0; i < 3; i++) {
        if (!(idx & (1 << i))) continue;

        ret = AiqV4l2Device_prepare(pRawStrProcUnit->_dev[i]);
        if (ret < 0) LOGE_CAMHW_SUBM(ISP20HW_SUBM, "mipi tx:%d prepare err: %d\n", ret);

        pRawStrProcUnit->_stream[i]->_base.set_device_prepared(&pRawStrProcUnit->_stream[i]->_base,
                                                               true);
    }
    return ret;
}

void AiqRawStreamProcUnit_set_working_mode(AiqRawStreamProcUnit_t* pRawStrProcUnit, int mode) {
    pRawStrProcUnit->_working_mode = mode;

    switch (pRawStrProcUnit->_working_mode) {
        case RK_AIQ_ISP_HDR_MODE_3_FRAME_HDR:
        case RK_AIQ_ISP_HDR_MODE_3_LINE_HDR:
            pRawStrProcUnit->_mipi_dev_max = 3;
            break;
        case RK_AIQ_ISP_HDR_MODE_2_FRAME_HDR:
        case RK_AIQ_ISP_HDR_MODE_2_LINE_HDR:
            pRawStrProcUnit->_mipi_dev_max = 2;
            break;
        default:
            pRawStrProcUnit->_mipi_dev_max = 1;
    }

    LOGD_CAMHW_SUBM(ISP20HW_SUBM, "working_mode:0x%x, _mipi_dev_max=%d\n",
                    pRawStrProcUnit->_working_mode, pRawStrProcUnit->_mipi_dev_max);
}

void AiqRawStreamProcUnit_set_rx_devices(AiqRawStreamProcUnit_t* pRawStrProcUnit,
                                         AiqV4l2Device_t* mipi_rx_devs[3]) {
    int i = 0;
    for (i = 0; i < 3; i++) {
        if (pRawStrProcUnit->_dev[i]) {
            if (!pRawStrProcUnit->_isExtDev) {
                pRawStrProcUnit->_dev[i]->close(pRawStrProcUnit->_dev[i]);
            }
            AiqRawStream_deinit(pRawStrProcUnit->_stream[i]);
            if (!pRawStrProcUnit->_isExtDev) {
                aiq_free(pRawStrProcUnit->_dev[i]);
            }
            pRawStrProcUnit->_isExtDev = true;
            pRawStrProcUnit->_dev[i]   = mipi_rx_devs[i];
            AiqRawStream_init(pRawStrProcUnit->_stream[i], pRawStrProcUnit->_dev[i], ISP_POLL_RX,
                              i);
            AiqPollCallback_t* pcb = pRawStrProcUnit->_pcb[i];
            if (!pcb) {
                pcb                    = (AiqPollCallback_t*)aiq_mallocz(sizeof(AiqPollCallback_t));
                pcb->_pCtx             = pRawStrProcUnit;
                pcb->poll_buffer_ready = RawStreamProcUnit_poll_buffer_ready;
                pcb->poll_event_ready  = NULL;

                pRawStrProcUnit->_pcb[i] = pcb;
            }
            AiqPollThread_setPollCallback(pRawStrProcUnit->_stream[i]->_base._poll_thread, pcb);
        }
    }
}

AiqV4l2Device_t* AiqRawStreamProcUnit_get_rx_device(AiqRawStreamProcUnit_t* pRawStrProcUnit,
                                                    int index) {
    if (index > pRawStrProcUnit->_mipi_dev_max)
        return NULL;
    else
        return pRawStrProcUnit->_dev[index];
}

XCamReturn AiqRawStreamProcUnit_set_rx_format(AiqRawStreamProcUnit_t* pRawStrProcUnit,
                                              const struct v4l2_subdev_format* sns_sd_fmt,
                                              uint32_t sns_v4l_pix_fmt) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    uint32_t bytes_perline = 0;
    int i          = 0;

    for (i = 0; i < 3; i++) {
        if (!pRawStrProcUnit->_dev[i]) {
            ret = XCAM_RETURN_ERROR_PARAM;
            break;
        }

        if (pRawStrProcUnit->_camHw->_airms_en) {
            int mem_mode = CSI_MEM_WORD_BIG_ALIGN;
            int ret1 = pRawStrProcUnit->_dev[i]->io_control(
                       pRawStrProcUnit->_dev[i], RKISP_CMD_SET_CSI_MEMORY_MODE, &mem_mode);
#if RKAIQ_HAVE_AIRMS
            AiqAiRmsStreamProcUnit_getBytesPerline(pRawStrProcUnit->_camHw->mAiRmsProcUnit, sns_sd_fmt->format.width, &bytes_perline);
#endif
        }

        ret = AiqV4l2Device_setFmt(pRawStrProcUnit->_dev[i], sns_sd_fmt->format.width,
                                   sns_sd_fmt->format.height, sns_v4l_pix_fmt, V4L2_FIELD_NONE, bytes_perline);
        if (ret < 0) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "device(%s) set format failed",
                            AiqV4l2Device_getDevName(pRawStrProcUnit->_dev[i]));
        } else {
            LOGD_CAMHW_SUBM(ISP20HW_SUBM, "device(%s) set fmt info: fmt 0x%x, %dx%d, bytes_perline %d !",
                            AiqV4l2Device_getDevName(pRawStrProcUnit->_dev[i]), sns_v4l_pix_fmt,
                            sns_sd_fmt->format.width, sns_sd_fmt->format.height, bytes_perline);
        }
    }

    return ret;
}

XCamReturn AiqRawStreamProcUnit_set_rx_format2(AiqRawStreamProcUnit_t* pRawStrProcUnit,
                                               const struct v4l2_subdev_selection* sns_sd_sel,
                                               uint32_t sns_v4l_pix_fmt) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    uint32_t bytes_perline = 0;

    int i = 0;
    for (i = 0; i < 3; i++) {
        if (!pRawStrProcUnit->_dev[i]) {
            ret = XCAM_RETURN_ERROR_PARAM;
            break;
        }

        int mem_mode = CSI_MEM_COMPACT;
        if (pRawStrProcUnit->_camHw->_airms_en) {
            int mem_mode = CSI_MEM_WORD_BIG_ALIGN;
            int ret1     = pRawStrProcUnit->_dev[i]->io_control(
                           pRawStrProcUnit->_dev[i], RKISP_CMD_SET_CSI_MEMORY_MODE, &mem_mode);
#if RKAIQ_HAVE_AIRMS
            AiqAiRmsStreamProcUnit_getBytesPerline(pRawStrProcUnit->_camHw->mAiRmsProcUnit, sns_sd_sel->r.width, &bytes_perline);
#endif
        }

        ret = AiqV4l2Device_setFmt(pRawStrProcUnit->_dev[i], sns_sd_sel->r.width,
                                   sns_sd_sel->r.height, sns_v4l_pix_fmt, V4L2_FIELD_NONE, bytes_perline);
        if (ret < 0) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "device(%s) set format failed",
                            AiqV4l2Device_getDevName(pRawStrProcUnit->_dev[i]));
        } else {
            LOGD_CAMHW_SUBM(ISP20HW_SUBM, "device(%s) set fmt info: fmt 0x%x, %dx%d, bytes_perline %d !",
                            AiqV4l2Device_getDevName(pRawStrProcUnit->_dev[i]), sns_v4l_pix_fmt,
                            sns_sd_sel->r.width, sns_sd_sel->r.height, bytes_perline);
        }
    }

    return ret;
}

void AiqRawStreamProcUnit_set_devices(AiqRawStreamProcUnit_t* pRawStrProcUnit,
                                      AiqV4l2SubDevice_t* ispdev, AiqCamHwBase_t* handle) {
    pRawStrProcUnit->_isp_core_dev = ispdev;
    pRawStrProcUnit->_camHw        = handle;
}

void AiqRawStreamProcUnit_send_sync_buf(AiqRawStreamProcUnit_t* pRawStrProcUnit,
                                        AiqV4l2Buffer_t* buf_s, AiqV4l2Buffer_t* buf_m,
                                        AiqV4l2Buffer_t* buf_l) {
    int ret = 0;
    aiqMutex_lock(&pRawStrProcUnit->_buf_mutex);
    if (pRawStrProcUnit->_isRawProcThQuit) {
        LOGW_CAMHW_SUBM(ISP20HW_SUBM, "quit already, ignore !");
        aiqMutex_unlock(&pRawStrProcUnit->_buf_mutex);
        return;
    }
    for (int i = 0; i < pRawStrProcUnit->_mipi_dev_max; i++) {
        if (i == ISP_MIPI_HDR_S) {
            ret = aiqList_push(pRawStrProcUnit->cache_list[ISP_MIPI_HDR_S], &buf_s);
            if (!ret) AiqV4l2Buffer_ref(buf_s);
        } else if (i == ISP_MIPI_HDR_M) {
            ret = aiqList_push(pRawStrProcUnit->cache_list[ISP_MIPI_HDR_M], &buf_m);
            if (!ret) AiqV4l2Buffer_ref(buf_m);
        } else if (i == ISP_MIPI_HDR_L) {
            ret = aiqList_push(pRawStrProcUnit->cache_list[ISP_MIPI_HDR_L], &buf_l);
            if (!ret) AiqV4l2Buffer_ref(buf_l);
        }

        if (ret) {
            aiqMutex_unlock(&pRawStrProcUnit->_buf_mutex);
            return;
        };
    }

    bool val     = true;
    uint32_t key = AiqV4l2Buffer_getSequence(buf_s);
    if (!aiqMap_insert(pRawStrProcUnit->_isp_hdr_fid2ready_map, (void*)(intptr_t)key, &val)) {
        LOGW_CAMHW_SUBM(ISP20HW_SUBM, "Failed to insert %d to map for RawStrProcUnit", key);
    }
    aiqMutex_unlock(&pRawStrProcUnit->_buf_mutex);
    aiqCond_broadcast(&pRawStrProcUnit->_buf_cond);
}

XCamReturn AiqRawStreamProcUnit_set_csi_mem_word_align_mode(AiqRawStreamProcUnit_t* pRawStrProcUnit, int mode) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    for (int i = 0; i < 3; i++) {
        if (!pRawStrProcUnit->_dev[i]) {
            ret = XCAM_RETURN_BYPASS;
            break;
        }

        int mem_mode = mode;
        int ret1     = pRawStrProcUnit->_dev[i]->io_control(
            pRawStrProcUnit->_dev[i], RKISP_CMD_SET_CSI_MEMORY_MODE, &mem_mode);
        if (ret1) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "set CSI_MEM_WORD_BIG_ALIGN failed !\n");
            ret = XCAM_RETURN_ERROR_IOCTL;
        } else {
            LOGD_CAMHW_SUBM(ISP20HW_SUBM, "set the memory mode of isp rx to big align");
#if RKAIQ_HAVE_DUMPSYS
            pRawStrProcUnit->data_mode = mem_mode;
#endif
        }
    }

    return ret;
}

void AiqRawStreamProcUnit_setSensorCategory(AiqRawStreamProcUnit_t* pRawStrProcUnit,
                                            bool sensorState) {
    pRawStrProcUnit->_is_1608_sensor = sensorState;
}

void AiqRawStreamProcUnit_setMulCamConc(AiqRawStreamProcUnit_t* pRawStrProcUnit, bool cc) {
    pRawStrProcUnit->_is_multi_cam_conc = cc;
}

void AiqRawStreamProcUnit_setCamPhyId(AiqRawStreamProcUnit_t* pRawStrProcUnit, int phyId) {
    pRawStrProcUnit->mCamPhyId = phyId;
}

void AiqRawStreamProcUnit_setPollCallback(AiqRawStreamProcUnit_t* pRawStrProcUnit,
                                          AiqPollCallback_t* cb) {
    pRawStrProcUnit->_PollCallback = cb;
}

XCamReturn AiqRawStreamProcUnit_capture_raw_ctl(AiqRawStreamProcUnit_t* pRawStrProcUnit,
                                                capture_raw_t type, int count,
                                                const char* capture_dir, char* output_dir) {
#ifdef UseCaptureRawData
    return aiq_dumpRkRaw_dumpControl(&pRawStrProcUnit->_rawCap, type, count, capture_dir,
                                     output_dir);
#else
    return aiq_dumpRkRaw_dumpControl(&pRawStrProcUnit->_rawCap, type, count, capture_dir,
                                     output_dir);
#endif
}

XCamReturn AiqRawStreamProcUnit_notify_capture_raw(AiqRawStreamProcUnit_t* pRawStrProcUnit) {
#ifdef UseCaptureRawData
    return aiq_dumpRkRaw_notifyDumpRaw(&pRawStrProcUnit->_rawCap);
#else
    return aiq_dumpRkRaw_notifyDumpRaw(&pRawStrProcUnit->_rawCap);
#endif
}

void AiqRawStreamProcUnit_setRxBufferCnt(AiqRawStreamProcUnit_t* pRawStrProcUnit, uint16_t buf_num) {
    for (int i = 0; i < 3; i++) {
        if (pRawStrProcUnit->_dev[i]) {
            AiqV4l2Device_setBufCnt(pRawStrProcUnit->_dev[i], buf_num);
        }
    }
}

bool AiqRawStreamProcUnit_getStreamOnStatus(AiqRawStreamProcUnit_t* pRawStrProcUnit) {
    bool stream_on = true;
    for (int i = 0; i < pRawStrProcUnit->_mipi_dev_max; i++) {
        if (pRawStrProcUnit->_dev[i]) {
            stream_on &= AiqV4l2Device_isActivated(pRawStrProcUnit->_dev[i]);
        }
    }
    return stream_on;
}

#if RKAIQ_HAVE_DUMPSYS
int AiqRawStreamProcUnit_dump(void* dumper, st_string* result, int argc, void* argv[]) {
    AiqRawStreamProcUnit_t* pRawStrProcUnit = (AiqRawStreamProcUnit_t*)dumper;
    if (pRawStrProcUnit->_isRawProcThQuit) return 0;

    stream_proc_dump_mod_param(pRawStrProcUnit, result);
    stream_proc_dump_dev_attr(pRawStrProcUnit, result);
    stream_proc_dump_chn_attr1(pRawStrProcUnit, result);
    stream_proc_dump_chn_status1(pRawStrProcUnit, result);
    stream_proc_dump_chn_status2(pRawStrProcUnit, result);
    stream_proc_dump_videobuf_status(pRawStrProcUnit, result);
    return 0;
}
#endif
