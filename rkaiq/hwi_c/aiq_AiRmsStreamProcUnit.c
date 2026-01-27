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
#include <stddef.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/dma-buf.h>
#include <fcntl.h>
//#define __USE_GNU
#include <sched.h>
#include <pthread.h>

#ifdef __ARM_NEON
#define NEON_OPT
#endif
#ifdef NEON_OPT
#include <arm_neon.h>
#endif

#include "hwi_c/aiq_AiRmsStreamProcUnit.h"
#include "hwi_c/aiq_CamHwBase.h"

#define CEIL_DOWN(x, y)                              (((x) + ((y) - 1)) / (y))
#define FLOOR_BY(v, r)                               (((v) / (r)) * (r))
#define CEIL_BY(v, r)                                FLOOR_BY(((v) + (r) - 1), (r))

static int64_t get_systime_us() {
    struct timespec times = {0, 0};
    int64_t time;

    clock_gettime(CLOCK_MONOTONIC, &times);
    time = times.tv_sec * 1000000LL + times.tv_nsec / 1000LL;

    return time;
}

static XCamReturn dump_buffer(char *filename, uint32_t frame_id, char *virt_addr, u32 size)
{
    FILE *fp;
    char name[256];

    sprintf(name, "%s/%d_%s", AIRMS_DUMPBUF_DIR, frame_id, filename);
    fp = fopen(name, "wb");
    if (fp) {
        LOGK_AIRMS("dump buffer to filename %s", name);
        fwrite(virt_addr, 1, size, fp);
        fclose(fp);
    } else {
        LOGE_AIRMS("can not open file %s", name);
    }

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn quad2rggb_nd_binning(
    uint16_t* pSrc,
    uint16_t* pDst,
    uint8_t*  pDstDwn,
    int srcHgt,
    int srcWid,
    int dstHgt,
    int dstWid)
{
    int dwnHgt = dstHgt / 2;
    int dwnWid = dstWid / 2;


    LOGD_AIRMS("%s: start: pSrc %p, pDst %p, pDstDwn %p, srcHgt %d, srcWid %d, dstHgt %d, dstWid %d",
        __func__, pSrc, pDst, pDstDwn, srcHgt, srcWid, dstHgt, dstWid);

    for (int i = 0; i < MIN(srcHgt, dstHgt) / 2 * 2; i += 2)
    {
        for (int j = 0; j < MIN(srcWid, dstWid) / 16 * 16; j += 4 * 4)
        {
            /*
                R0      R1      Ga0     Ga1             R0      Ga0     R1      Ga1
                R2      R3      Ga2     Ga3             Gb0     B0      Gb1     B1
                                                -- >
                Gb0     Gb1     B0      B1              R2      Ga2     R3      Ga3
                Gb2     Gb3     B2      B3              Gb2     B2      Gb3     B3
            */
            uint16x4x4_t vR0_R1_Ga0_Ga1_u16 = vld4_u16(pSrc + (i + 0) * srcWid + j);   //  [0, 1023]
            uint16x4x4_t vR2_R3_Ga2_Ga3_u16 = vld4_u16(pSrc + (i + 1) * srcWid + j);

            //  Binning DownSample
            uint16x4x2_t vR_Ga_u16;
            vR_Ga_u16.val[0] = vadd_u16(vadd_u16(vR0_R1_Ga0_Ga1_u16.val[0], vR0_R1_Ga0_Ga1_u16.val[1]), vadd_u16(vR2_R3_Ga2_Ga3_u16.val[0], vR2_R3_Ga2_Ga3_u16.val[1]));
            vR_Ga_u16.val[1] = vadd_u16(vadd_u16(vR0_R1_Ga0_Ga1_u16.val[2], vR0_R1_Ga0_Ga1_u16.val[3]), vadd_u16(vR2_R3_Ga2_Ga3_u16.val[2], vR2_R3_Ga2_Ga3_u16.val[3]));

            vR_Ga_u16.val[0] = vrshr_n_u16(vR_Ga_u16.val[0], 2 + 2);  //  mean [0, 1023] -> [0, 255]
            vR_Ga_u16.val[1] = vrshr_n_u16(vR_Ga_u16.val[1], 2 + 2);  //  mean [0, 1023] -> [0, 255]

            //  R0      R1      R2      R3      -->     R0      Ga0     R1      Ga1
            //  Ga0     Ga1     Ga2     Ga4             R2      Ga2     R3      Ga3
            vR_Ga_u16 = vzip_u16(vR_Ga_u16.val[0], vR_Ga_u16.val[1]);

            vst1_u8(pDstDwn + (i / 2) * dwnWid + (j / 2), vqmovn_u16(vcombine_u16(vR_Ga_u16.val[0], vR_Ga_u16.val[1])));

            //  4 Cell Pattern to RGGB Pattern
            int offset0_i = ((i + 0) % 4 == 0 || (i + 0) % 4 == 3) ? 0 : (((i + 0) % 4 == 1) ? 1 : -1);
            int offset1_i = ((i + 1) % 4 == 0 || (i + 1) % 4 == 3) ? 0 : (((i + 1) % 4 == 1) ? 1 : -1);

            uint16x4x4_t vR0_Ga0_R1_Ga1_u16;
            vR0_Ga0_R1_Ga1_u16.val[0] = vshl_n_u16(vR0_R1_Ga0_Ga1_u16.val[0], 6);  //  [0, 1023] -> [0, 65535]
            vR0_Ga0_R1_Ga1_u16.val[1] = vshl_n_u16(vR0_R1_Ga0_Ga1_u16.val[2], 6);
            vR0_Ga0_R1_Ga1_u16.val[2] = vshl_n_u16(vR0_R1_Ga0_Ga1_u16.val[1], 6);
            vR0_Ga0_R1_Ga1_u16.val[3] = vshl_n_u16(vR0_R1_Ga0_Ga1_u16.val[3], 6);
            vst4_u16(pDst + ((i + 0) + offset0_i) * dstWid + j, vR0_Ga0_R1_Ga1_u16);

            uint16x4x4_t vR2_Ga2_R3_Ga3_u16;
            vR2_Ga2_R3_Ga3_u16.val[0] = vshl_n_u16(vR2_R3_Ga2_Ga3_u16.val[0], 6);  //  [0, 1023] -> [0, 65535]
            vR2_Ga2_R3_Ga3_u16.val[1] = vshl_n_u16(vR2_R3_Ga2_Ga3_u16.val[2], 6);
            vR2_Ga2_R3_Ga3_u16.val[2] = vshl_n_u16(vR2_R3_Ga2_Ga3_u16.val[1], 6);
            vR2_Ga2_R3_Ga3_u16.val[3] = vshl_n_u16(vR2_R3_Ga2_Ga3_u16.val[3], 6);
            vst4_u16(pDst + ((i + 1) + offset1_i) * dstWid + j, vR2_Ga2_R3_Ga3_u16);
        }

        if (MIN(srcWid, dstWid) / 32 * 32 != MIN(srcWid, dstWid))
            for (int j = MIN(srcWid, dstWid) / 16 * 16; j < MIN(srcWid, dstWid); j += 2)
            {
                int offset0_i = ((i + 0) % 4 == 0 || (i + 0) % 4 == 3) ? 0 : (((i + 0) % 4 == 1) ? 1 : -1);
                int offset1_i = ((i + 1) % 4 == 0 || (i + 1) % 4 == 3) ? 0 : (((i + 1) % 4 == 1) ? 1 : -1);

                int src_i0 = i + 0;
                int src_i1 = i + 1;
                int dst_i0 = src_i0 + offset0_i;
                int dst_i1 = src_i1 + offset1_i;

                int src_j0 = -1;
                int src_j1 = -1;
                int dst_j0 = j + 0;
                int dst_j1 = j + 1;

                src_j0 = (dst_j0 % 4 == 0 || dst_j0 % 4 == 3) ? dst_j0 : (dst_j0 % 4 == 1 ? MIN(dst_j0 + 1, srcWid - 1) : dst_j0 - 1);
                src_j1 = (dst_j1 % 4 == 0 || dst_j1 % 4 == 3) ? dst_j1 : (dst_j1 % 4 == 1 ? MIN(dst_j1 + 1, srcWid - 1) : dst_j1 - 1);

                uint16_t vacc = pSrc[src_i0 * srcWid + src_j0] + pSrc[src_i0 * srcWid + src_j1] + pSrc[src_i1 * srcWid + src_j0] + pSrc[src_i1 * srcWid + src_j1];
                pDstDwn[(i / 2) * dwnWid + (j / 2)] = vacc >> (2 + 2);  //  mean [0, 1023] -> [0, 255]

                pDst[dst_i0 * dstWid + dst_j0] = pSrc[src_i0 * srcWid + src_j0] << 6;  //  [0, 1023] -> [0, 65535]
                pDst[dst_i0 * dstWid + dst_j1] = pSrc[src_i0 * srcWid + src_j1] << 6;
                pDst[dst_i1 * dstWid + dst_j0] = pSrc[src_i1 * srcWid + src_j0] << 6;
                pDst[dst_i1 * dstWid + dst_j1] = pSrc[src_i1 * srcWid + src_j1] << 6;
            }
    }

    LOGD_AIRMS("%s: end: pSrc %p, pDst %p, pDstDwn %p, srcHgt %d, srcWid %d, dstHgt %d, dstWid %d",
        __func__, pSrc, pDst, pDstDwn, srcHgt, srcWid, dstHgt, dstWid);

    return XCAM_RETURN_NO_ERROR;
}

static void AiRmsStream_quard2rggb(const AirmsStreamParam* attrib)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ENTER_ANALYZER_FUNCTION();

    struct timeval start_time, end_time;
    unsigned long thread_id = (unsigned long)pthread_self();

    gettimeofday(&start_time, NULL);

    AiqAiRmsStreamProcUnit_t* pProcUnit = attrib->pProcUnit;
    uint16_t* pSrc = attrib->pSrc;
    AiRmsAiispBuf *aiispInBuf = attrib->aiispInBuf;
    int hdlIdx = attrib->hdlIdx;
    int proc_height = pProcUnit->isp_acq_height + (32 - pProcUnit->isp_acq_height % 32) % 32;
    int proc_width = pProcUnit->isp_acq_width + (32 - pProcUnit->isp_acq_width % 32) % 32;
    int srcHgt = pProcUnit->isp_acq_height;
    int srcWid = pProcUnit->isp_acq_width;
    int dstHgt = proc_height;
    int dstWid = CEIL_BY(proc_width, 16);
    uint16_t* pDst = (uint16_t*)aiispInBuf->virt_addr;
    uint8_t* pDstDwn;
    u32 bin_width, bin_height;

    if (!pSrc || !pDst) {
        LOGE_AIRMS("%s: input buffer is null(%p, %p)", __func__, pSrc, pDst);
        return;
    }

    pDstDwn = (uint8_t*)(pDst + dstHgt * dstWid);
    srcHgt = srcHgt / AIRMS_HELP_THREAD_POOL_NUM;
    dstHgt = srcHgt;
    pSrc += srcWid * srcHgt * hdlIdx;
    pDst += dstWid * dstHgt * hdlIdx;
    bin_width  = CEIL_BY(CEIL_DOWN(dstWid, 2), 2);
    bin_height = CEIL_BY(CEIL_DOWN(dstHgt, 2), 2);
    pDstDwn += bin_height * bin_width * hdlIdx;
    if (hdlIdx == AIRMS_HELP_THREAD_POOL_NUM - 1) {
        srcHgt += (pProcUnit->isp_acq_height - srcHgt * AIRMS_HELP_THREAD_POOL_NUM);
        dstHgt += (proc_height - dstHgt * AIRMS_HELP_THREAD_POOL_NUM);
    }

    quad2rggb_nd_binning(pSrc, pDst, pDstDwn, srcHgt, srcWid, dstHgt, dstWid);

    gettimeofday(&end_time, NULL);

    long seconds      = end_time.tv_sec - start_time.tv_sec;
    long micros       = ((seconds * 1000000) + end_time.tv_usec) - start_time.tv_usec;
    double elapsed_ms = micros / 1000.0;

    LOGD_AIRMS("TID: %lu, srcWid %d, srcHgt %d, dstWid %d, dstHgt %d, elapsed_ms %.2f ms",
        thread_id, srcWid, srcHgt, dstWid, dstHgt, elapsed_ms);

    EXIT_ANALYZER_FUNCTION();

    return;
}

static uint32_t airms_V4l2Buffer_unref(XCamVideoBuffer* buf) {
    AiqV4l2Buffer_t* pAiqV4l2Buf = (AiqV4l2Buffer_t*)buf;
    AiqAiRmsStreamProcUnit_t* pProcUnit = container_of(pAiqV4l2Buf->_device, AiqAiRmsStreamProcUnit_t, _device);
    uint32_t ref_cnts = 0;
    AiRmsAiispBuf *aiispOutBuf = NULL;

    aiqMutex_lock(&pAiqV4l2Buf->_device->_buf_mutex);
    if (pAiqV4l2Buf->_ref_cnts > 0) pAiqV4l2Buf->_ref_cnts--;
    ref_cnts = pAiqV4l2Buf->_ref_cnts;
    if (ref_cnts == 0) {
        for (int i = 0; i < AIRMS_OUTBUF_NUM; i++) {
            if (pProcUnit->outbuf_tbl[i].dma_fd == pAiqV4l2Buf->_expbuf_fd) {
                pProcUnit->outbuf_tbl[i].in_used = false;
                aiispOutBuf = &pProcUnit->outbuf_tbl[i];
                break;
            }
        }
        if (!aiispOutBuf) {
            LOGE_AIRMS("%s:%d can not find aiispOutBuf", __FUNCTION__, __LINE__);
        }

        LOGI_AIRMS("%s: seq: %d, isp unref buffer: outbuf_idx %d, dma_fd %d, size %d",
            __func__, AiqV4l2Buffer_getSequence(&aiispOutBuf->aiqV4l2Buf),
            aiispOutBuf->buf_idx, aiispOutBuf->dma_fd, aiispOutBuf->buf_size);
    }
    aiqMutex_unlock(&pAiqV4l2Buf->_device->_buf_mutex);

    return ref_cnts;
}

static bool AiRmsQuardConvertThd_push_attr(AirmsQuardConvertThd_t* pConvertThd,
                                           AirmsQuardConvertParam* params) {
    if (!pConvertThd->_base->_started) return false;

    aiqMutex_lock(&pConvertThd->_mutex);
    int ret = aiqList_push(pConvertThd->mMsgsQueue, params);
    if (ret) {
        LOGE_AIRMS("push AirmsQuardConvertParam failed", __func__);
        aiqMutex_unlock(&pConvertThd->_mutex);
        return false;
    }

    aiqCond_broadcast(&pConvertThd->_cond);
    aiqMutex_unlock(&pConvertThd->_mutex);

    return true;
}

static bool AiRmsQuardConvertThd_loop(void* user_data) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ENTER_ANALYZER_FUNCTION();

    AirmsQuardConvertThd_t* pCvtTh = (AirmsQuardConvertThd_t*)user_data;
    AiqAiRmsStreamProcUnit_t* pProcUnit = pCvtTh->mAiRmsStreamProc;

    aiqMutex_lock(&pCvtTh->_mutex);
    while (!pCvtTh->bQuit && aiqList_size(pCvtTh->mMsgsQueue) <= 0) {
        aiqCond_wait(&pCvtTh->_cond, &pCvtTh->_mutex);
    }
    if (pCvtTh->bQuit) {
        aiqMutex_unlock(&pCvtTh->_mutex);
        LOGE_AIRMS("quit AiRmsQuardConvertThd!");
        return false;
    }
    aiqMutex_unlock(&pCvtTh->_mutex);

    AiqListItem_t* pItem = aiqList_get_item(pCvtTh->mMsgsQueue, NULL);
    if (!pItem) {
        // ignore error
        return true;
    }

    AirmsQuardConvertParam* attrib = (AirmsQuardConvertParam*)pItem->_pData;
    AiqV4l2Buffer_t *vicapbuf = attrib->vicapbuf;
    struct rkaiisp_airms_st airms_st;
    AiRmsAiispBuf *aiispInBuf  = NULL;
    AiRmsAiispBuf *aiispOutBuf = NULL;

    LOGI_AIRMS("%s: seq: %d, QuardConvert in buffer: dma_fd %d length %d",
        __func__, AiqV4l2Buffer_getSequence(vicapbuf),
        AiqV4l2Buffer_getExpbufFd(vicapbuf), AiqV4l2Buffer_getLength(vicapbuf));

    int64_t start_time = get_systime_us();

    // get in_fd
    aiqMutex_lock(&pProcUnit->inbuf_mutex);
    for (int i = 0; i < AIRMS_INBUF_NUM; i++) {
        if (!pProcUnit->inbuf_tbl[i].in_used) {
            pProcUnit->inbuf_tbl[i].in_used = true;
            aiispInBuf = &pProcUnit->inbuf_tbl[i];
            break;
        }
    }
    aiqMutex_unlock(&pProcUnit->inbuf_mutex);

    if (!aiispInBuf) {
        LOGE_AIRMS("%s:%d no free aiispInBuf", __FUNCTION__, __LINE__);
        AiqVideoBuffer_unref(vicapbuf);
        aiqList_erase_item(pCvtTh->mMsgsQueue, pItem);
        return true;
    }

    // change quard to bayer
    struct dma_buf_sync sync = { 0 };
    AirmsStreamParam convert_params;
    bool dump_flg = false;

    sync.flags = DMA_BUF_SYNC_WRITE | DMA_BUF_SYNC_START;
    ioctl(aiispInBuf->dma_fd, DMA_BUF_IOCTL_SYNC, &sync);

    aiqMutex_lock(&pProcUnit->mStreamMutex);
    convert_params.pSrc = (uint16_t *)AiqV4l2Buffer_getExpbufUsrptr(vicapbuf);
    convert_params.aiispInBuf = aiispInBuf;
    convert_params.pProcUnit = pProcUnit;

    for (int n = 0; n < AIRMS_HELP_THREAD_POOL_NUM; n++) {
        convert_params.hdlIdx = n;
        thpool_add_work_ex(pProcUnit->mHelpThPool, (void*)AiRmsStream_quard2rggb, &convert_params, sizeof(convert_params));
    }

    thpool_wait(pProcUnit->mHelpThPool);

    aiqMutex_unlock(&pProcUnit->mStreamMutex);

    sync.flags = DMA_BUF_SYNC_WRITE | DMA_BUF_SYNC_END;
    ioctl(aiispInBuf->dma_fd, DMA_BUF_IOCTL_SYNC, &sync);

    int64_t end_time = get_systime_us();

    LOGD_AIRMS("quard to bayer convert time %lld", (end_time - start_time) / 1000);

    //if (end_time - start_time > 40 * 1000) {
    //    LOGE_AIRMS("======== convert time is too long, start_time %lld, end_time %lld, %lld ==========",
    //        start_time, end_time, (end_time - start_time) / 1000);
    //}

    if (pProcUnit->dump_raw_num >= -1 && pProcUnit->dump_raw_num != 0) {
        dump_flg = true;
        pProcUnit->dump_raw_num--;
    }

    if (dump_flg) {
        uint32_t frame_id = AiqV4l2Buffer_getSequence(vicapbuf);
        dump_buffer("vicapin.raw", frame_id, (char *)AiqV4l2Buffer_getExpbufUsrptr(vicapbuf),
                    AiqV4l2Buffer_getLength(vicapbuf));
        dump_buffer("quard2rggb.raw", frame_id, (char *)AiqV4l2Buffer_getExpbufUsrptr(&aiispInBuf->aiqV4l2Buf),
                    AiqV4l2Buffer_getLength(&aiispInBuf->aiqV4l2Buf));
    }

    aiqMutex_lock(&pProcUnit->_device._buf_mutex);
    for (int i = 0; i < AIRMS_OUTBUF_NUM; i++) {
        if (!pProcUnit->outbuf_tbl[i].in_used) {
            pProcUnit->outbuf_tbl[i].in_used = true;
            aiispOutBuf = &pProcUnit->outbuf_tbl[i];
            break;
        }
    }
    aiqMutex_unlock(&pProcUnit->_device._buf_mutex);

    if (!aiispOutBuf) {
        LOGE_AIRMS("%s:%d no free aiispOutBuf", __FUNCTION__, __LINE__);
        aiqMutex_lock(&pProcUnit->inbuf_mutex);
        aiispInBuf->in_used = false;
        AiqVideoBuffer_unref(vicapbuf);
        aiqList_erase_item(pCvtTh->mMsgsQueue, pItem);
        aiqMutex_unlock(&pProcUnit->inbuf_mutex);
        return true;
    }

    aiispOutBuf->dump_flg = dump_flg;
    AiqV4l2Buffer_init(&aiispOutBuf->aiqV4l2Buf, &vicapbuf->_buf, &vicapbuf->_format, &pProcUnit->_device);
    aiispOutBuf->aiqV4l2Buf._expbuf_fd     = aiispOutBuf->dma_fd;
    aiispOutBuf->aiqV4l2Buf._length        = aiispOutBuf->buf_size;
    aiispOutBuf->aiqV4l2Buf._expbuf_usrptr = (uintptr_t)aiispOutBuf->virt_addr;
    aiispOutBuf->aiqV4l2Buf._device        = &pProcUnit->_device;
    aiispOutBuf->aiqV4l2Buf._buf.memory    = V4L2_MEMORY_DMABUF;
    aiispOutBuf->aiqV4l2Buf._vb._xcam_vb.unref = airms_V4l2Buffer_unref;
    aiispOutBuf->aiqV4l2Buf._buf.m.planes              = &aiispOutBuf->planes;
    aiispOutBuf->aiqV4l2Buf._buf.m.planes[0].length    = aiispOutBuf->buf_size;
    aiispOutBuf->aiqV4l2Buf._buf.m.planes[0].bytesused = aiispOutBuf->buf_size;

    // send bayer to aiisp driver(sigmamap/narmap is must all zero)
    airms_st.sequence   = vicapbuf->_buf.sequence;
    airms_st.inbuf_idx  = aiispInBuf->buf_idx;
    airms_st.outbuf_idx = aiispOutBuf->buf_idx;
    LOGI_AIRMS("%s: seq: %d, send buffer to aiisp: inbuf_idx %d, outbuf_idx %d",
        __func__, airms_st.sequence, airms_st.inbuf_idx, airms_st.outbuf_idx);
    if (pProcUnit->mAiIspDev->io_control(pProcUnit->mAiIspDev,
                                      RKAIISP_CMD_QUEUE_BUF, &airms_st) != 0) {
        LOGE_AIRMS("%s:%d can't queue buffer to aiisp", __FUNCTION__, __LINE__);
    }

    AiqVideoBuffer_unref(vicapbuf);
    aiqList_erase_item(pCvtTh->mMsgsQueue, pItem);

    if (ret == XCAM_RETURN_NO_ERROR) return true;

    LOGE_AIRMS("AiRmsQuardConvertThd failed to run command!");

    EXIT_ANALYZER_FUNCTION();

    return false;
}

XCamReturn AiRmsQuardConvertThd_init(AirmsQuardConvertThd_t* pConvertThd,
                                    AiqAiRmsStreamProcUnit_t* pProcUnit) {
    pConvertThd->mAiRmsStreamProc = pProcUnit;

    aiqMutex_init(&pConvertThd->_mutex);
    aiqCond_init(&pConvertThd->_cond);

    AiqListConfig_t msgqCfg;
    msgqCfg._name        = "AiRmsConvertMsgQ";
    msgqCfg._item_nums   = AIRMS_CONVERT_THREAD_MSG_MAX;
    msgqCfg._item_size   = sizeof(AirmsQuardConvertParam);
    pConvertThd->mMsgsQueue = aiqList_init(&msgqCfg);
    if (!pConvertThd->mMsgsQueue) {
        LOGE_AIRMS("init %s error", msgqCfg._name);
        goto err_list;
    }

    pConvertThd->_base = aiqThread_init("AiRmsQuardConvertThd", AiRmsQuardConvertThd_loop, pConvertThd);
    if (!pConvertThd->_base) goto err_thread;

    return XCAM_RETURN_NO_ERROR;
err_thread:
    if (pConvertThd->mMsgsQueue) aiqList_deinit(pConvertThd->mMsgsQueue);
err_list:
    aiqMutex_deInit(&pConvertThd->_mutex);
    aiqCond_deInit(&pConvertThd->_cond);
    return XCAM_RETURN_ERROR_FAILED;
}

void AiRmsQuardConvertThd_deinit(AirmsQuardConvertThd_t* pConvertThd) {
    ENTER_ANALYZER_FUNCTION();
    if (pConvertThd->mMsgsQueue) aiqList_deinit(pConvertThd->mMsgsQueue);
    if (pConvertThd->_base) aiqThread_deinit(pConvertThd->_base);

    aiqMutex_deInit(&pConvertThd->_mutex);
    aiqCond_deInit(&pConvertThd->_cond);
    EXIT_ANALYZER_FUNCTION();
}

XCamReturn AiRmsQuardConvertThd_start(AirmsQuardConvertThd_t* pConvertThd) {
    ENTER_ANALYZER_FUNCTION();
    if (!aiqThread_start(pConvertThd->_base)) {
        LOGE_AIRMS("%s failed", __func__);
        return XCAM_RETURN_ERROR_FAILED;
    }
    pConvertThd->bQuit = false;
    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiRmsQuardConvertThd_stop(AirmsQuardConvertThd_t* pConvertThd) {
    ENTER_ANALYZER_FUNCTION();
    aiqMutex_lock(&pConvertThd->_mutex);
    pConvertThd->bQuit = true;
    aiqCond_broadcast(&pConvertThd->_cond);
    aiqMutex_unlock(&pConvertThd->_mutex);
    aiqThread_stop(pConvertThd->_base);
    AiqListItem_t* pItem = NULL;
    bool rm              = false;
    while (aiqList_size(pConvertThd->mMsgsQueue) > 0) {
        AiqListItem_t* pItem = aiqList_get_item(pConvertThd->mMsgsQueue, NULL);
        if (pItem) {
            AirmsQuardConvertParam* attrib = (AirmsQuardConvertParam*)pItem->_pData;
            AiqV4l2Buffer_t *vicapbuf = attrib->vicapbuf;

            AiqVideoBuffer_unref(vicapbuf);
            aiqList_erase_item(pConvertThd->mMsgsQueue, pItem);
        }
    }
    AIQ_LIST_FOREACH(pConvertThd->mMsgsQueue, pItem, rm) {
        rm    = true;
        pItem = aiqList_erase_item_locked(pConvertThd->mMsgsQueue, pItem);
    }

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

static float airms_nonLinear(float x, int flag)
{
    float r = 0.5;
    x = CLIP(x, 0, 1);

    float R2 = (1 + r) * (1 + r) + r * r;
    float y = 0;
    if (flag == 0)
    {
        float X2 = (ABS(x) - (1 + r)) * (ABS(x) - (1 + r));
        y = sqrt(R2 - X2) - r;
    }
    else
    {
        float X2 = (ABS(x) + r) * (ABS(x) + r);
        y = (r + 1) - sqrt(R2 - X2);
    }
    return y;
}

static int airms_getKwtMaxSize(char *model_buf, uint32_t *kwt_size)
{
    struct rkaiisp_model_info *model_info = (struct rkaiisp_model_info *)model_buf;
    int ret = -1;

    *kwt_size = 0;
    if (model_info) {
        struct rkaiisp_kwt_cfg *kwt_cfg;
        uint32_t runcnt;

        runcnt = model_info->model_runcnt;
        kwt_cfg = &model_info->kwt_cfg;
        *kwt_size = kwt_cfg->kwt_offet[runcnt-1] + kwt_cfg->kwt_pad_size[runcnt-1];
        ret = 0;
    }

    return ret;
}

static int airms_getParamInfo(AiqAiRmsStreamProcUnit_t* pProcUnit, struct rkaiisp_param_info *param_info)
{
    uint32_t kwt_size, max_size;

    max_size = 0;
    if (airms_getKwtMaxSize(pProcUnit->mModelBuf, &kwt_size) != 0) {
        LOGE_AIRMS("%s: get param info failed!", __func__);
    }

    if (kwt_size > max_size)
        max_size = kwt_size;

    param_info->exealgo    = AIRMS;
    param_info->exemode    = BOTHEVENT_TO_AIQ;
    param_info->max_runcnt = pProcUnit->model_max_runcnt;
    param_info->para_size  = max_size + sizeof(struct rkaiisp_params);
    return 0;
}

static void airms_params_print(struct rkaiisp_params *aiisp_cfg)
{
    struct rkaiisp_other_cfg *other_cfg = &aiisp_cfg->other_cfg;
    struct rkaiisp_kwt_cfg *kwt_cfg = &aiisp_cfg->kwt_cfg;
    struct rkaiisp_model_cfg *model_cfg;
    u32 i, j;

    LOGD_AIBNR("aiisp: frame_id %d, module_update = 0x%x", aiisp_cfg->frame_id, aiisp_cfg->module_update);
    if (aiisp_cfg->module_update & RKAIISP_OTHER_UPDATE) {
        LOGD_AIRMS("sw_neg_noiselimit = 0x%x", other_cfg->sw_neg_noiselimit);
        LOGD_AIRMS("sw_pos_noiselimit = 0x%x", other_cfg->sw_pos_noiselimit);
        LOGD_AIRMS("sw_prev_blacklvl = 0x%x", other_cfg->sw_prev_blacklvl);
        LOGD_AIRMS("sw_post_blacklvl = 0x%x", other_cfg->sw_post_blacklvl);

        for (i = 0; i < 33; i++) {
            LOGD_AIRMS("sw_in_comp_y[%d] = 0x%x", i, other_cfg->sw_in_comp_y[i]);
        }
        for (i = 0; i < 33; i++) {
            LOGD_AIRMS("sw_out_decomp_y[%d] = 0x%x", i, other_cfg->sw_out_decomp_y[i]);
        }
    }

    if (aiisp_cfg->module_update & RKAIISP_MODEL_UPDATE) {
        for (i = 0; i < aiisp_cfg->model_runcnt; i++) {
            model_cfg = (struct rkaiisp_model_cfg *)&aiisp_cfg->model_cfg[i];

            LOGD_AIRMS("model_cfg %d:", i);
            for (j = 0; j < 7; j++) {
                LOGD_AIRMS("mi_chn_src[%d] = 0x%x", j, model_cfg->mi_chn_src[j]);
            }
            LOGD_AIRMS("sw_aiisp_mode = %d", model_cfg->sw_aiisp_mode);
            LOGD_AIRMS("sw_aiisp_level_num = %d", model_cfg->sw_aiisp_level_num);
            LOGD_AIRMS("sw_aiisp_l1_level_num = %d", model_cfg->sw_aiisp_l1_level_num);
            LOGD_AIRMS("sw_aiisp_op_mode = %d", model_cfg->sw_aiisp_op_mode);
            LOGD_AIRMS("sw_aiisp_drop_en = %d", model_cfg->sw_aiisp_drop_en);
            for (j = 0; j < 16; j++) {
                LOGD_AIRMS("sw_aiisp_lv_active[%d] = 0x%x", j, model_cfg->sw_aiisp_lv_active[j]);
            }
            for (j = 0; j < 16; j++) {
                LOGD_AIRMS("sw_aiisp_lv_mode[%d] = 0x%x", j, model_cfg->sw_aiisp_lv_mode[j]);
            }
            for (j = 0; j < 7; j++) {
                LOGD_AIRMS("sw_mi_chn_en[%d] = 0x%x", j, model_cfg->sw_mi_chn_en[j]);
            }
            for (j = 0; j < 7; j++) {
                LOGD_AIRMS("sw_mi_chn_mode[%d] = 0x%x", j, model_cfg->sw_mi_chn_mode[j]);
            }
            for (j = 0; j < 7; j++) {
                LOGD_AIRMS("sw_mi_chn_num[%d] = 0x%x", j, model_cfg->sw_mi_chn_num[j]);
            }
            for (j = 0; j < 7; j++) {
                LOGD_AIRMS("sw_mi_chn_data_mode[%d] = 0x%x", j, model_cfg->sw_mi_chn_data_mode[j]);
            }
            LOGD_AIRMS("sw_mi_chn1_sel = %d", model_cfg->sw_mi_chn1_sel);
            LOGD_AIRMS("sw_mi_chn3_sel = %d", model_cfg->sw_mi_chn3_sel);
            LOGD_AIRMS("sw_out_d2s_en = %d", model_cfg->sw_out_d2s_en);
            LOGD_AIRMS("sw_out_mode = %d", model_cfg->sw_out_mode);
            LOGD_AIRMS("sw_lastlvlm1_clip8bit = %d", model_cfg->sw_lastlvlm1_clip8bit);
        }

        for (i = 0; i < RKAIISP_MAX_RUNCNT; i++) {
            LOGD_AIRMS("kwt_offet[%d] = 0x%x", i, kwt_cfg->kwt_offet[i]);
        }
        for (i = 0; i < RKAIISP_MAX_RUNCNT; i++) {
            LOGD_AIRMS("kwt_size[%d] = 0x%x", i, kwt_cfg->kwt_size[i]);
        }
        for (i = 0; i < RKAIISP_MAX_RUNCNT; i++) {
            LOGD_AIRMS("kwt_pad_size[%d] = 0x%x", i, kwt_cfg->kwt_pad_size[i]);
        }
    }
}

static int airms_checkModelSum(char *model_file, char *model_buf, size_t file_len)
{
    uint32_t* pTmp = (unsigned int*)model_buf;
    uint32_t i, checksum, realsum;

    realsum = 0;
    checksum = *pTmp++;
    for (i = 1; i < file_len / 4; i++) {
        realsum += *pTmp++;
    }

    if (checksum != realsum) {
        LOGE_AIRMS("model(%s) sum check error 0x%x, 0x%x", model_file, checksum, realsum);
        return -1;
    }

    return 0;
}

static int airms_checkModelCfg(struct rkaiisp_model_cfg *model_cfg)
{
    uint32_t i;

    if (model_cfg->sw_aiisp_mode > 1) {
        LOGE_AIRMS("%s: sw_aiisp_mode %d", __func__, model_cfg->sw_aiisp_mode);
        return -1;
    }
    if (model_cfg->sw_aiisp_op_mode > 1) {
        LOGE_AIRMS("%s: sw_aiisp_op_mode %d", __func__, model_cfg->sw_aiisp_op_mode);
        return -1;
    }
    if (model_cfg->sw_out_mode > 2) {
        LOGE_AIRMS("%s: sw_out_mode %d", __func__, model_cfg->sw_out_mode);
        return -1;
    }
    if (model_cfg->sw_aiisp_level_num > 16) {
        LOGE_AIRMS("%s: sw_aiisp_level_num %d", __func__, model_cfg->sw_aiisp_level_num);
        return -1;
    }

    for (i = 0; i < 7; i++) {
        if (model_cfg->sw_mi_chn_mode[i] > 2) {
            LOGE_AIRMS("%s: sw_mi_chn_mode %d", __func__, model_cfg->sw_mi_chn_mode[i]);
            return -1;
        }
        if (model_cfg->sw_mi_chn_data_mode[i] > 2) {
            LOGE_AIRMS("%s: sw_mi_chn_data_mode %d", __func__, model_cfg->sw_mi_chn_data_mode[i]);
            return -1;
        }
    }

    for (i = 0; i < 16; i++) {
        if (model_cfg->sw_aiisp_lv_mode[i] > 4) {
            LOGE_AIRMS("%s: sw_aiisp_lv_mode %d", __func__, model_cfg->sw_aiisp_lv_mode[i]);
            return -1;
        }
    }

    return 0;
}

static XCamReturn airms_readModel(AiqAiRmsStreamProcUnit_t* pProcUnit, char *model_file, char **model_buf)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    uint32_t model_max_runcnt = 0;

    struct rkaiisp_model_info model_info;
    size_t file_size, correct_size, runcnt;
    struct rkaiisp_kwt_cfg *kwt_cfg;
    FILE *fp;

    *model_buf = NULL;
    fp = fopen(model_file, "rb");
    if (fp) {
        if (fread(&model_info, sizeof(model_info), 1, fp) != 1) {
            LOGE_AIRMS("%s(%d): %s: file head is error\n",
                __FUNCTION__, __LINE__, model_file);
            return XCAM_RETURN_ERROR_FILE;
        }
        fseek(fp, 0, 2);
        file_size = ftell(fp);

        runcnt = model_info.model_runcnt;
        kwt_cfg = &model_info.kwt_cfg;

        if (runcnt > model_max_runcnt)
            model_max_runcnt = runcnt;

        correct_size = kwt_cfg->kwt_offet[runcnt-1] + kwt_cfg->kwt_pad_size[runcnt-1];
        if (file_size != correct_size) {
            LOGE_AIRMS("%s(%d): %s: file size(%d, %d) is error\n",
                __FUNCTION__, __LINE__, model_file, file_size, correct_size);
            return XCAM_RETURN_ERROR_FILE;
        }

        for (uint32_t j = 0; j < model_info.model_runcnt; j++) {
            if (airms_checkModelCfg(&model_info.model_cfg[j]) != 0) {
                LOGE_AIRMS("%s(%d): %s: model info check failure\n",
                    __FUNCTION__, __LINE__, model_file);
                return XCAM_RETURN_ERROR_FILE;
            }
        }

        pProcUnit->mModelMode = model_info.model_mode;
        *model_buf = aiq_malloc(file_size);
        if (*model_buf) {
            fseek(fp, 0, 0);
            fread(*model_buf, 1, file_size, fp);
            fclose(fp);
            if (airms_checkModelSum(model_file, *model_buf, file_size) != 0) {
                aiq_free(*model_buf);
                *model_buf = NULL;
                ret = XCAM_RETURN_ERROR_FAILED;
            }
        }
    } else {
        LOGE_AIRMS("read aiisp model file(%s) failed !", model_file);
        ret = XCAM_RETURN_ERROR_FAILED;
    }

    pProcUnit->model_max_runcnt = model_max_runcnt;

    return ret;
}

static XCamReturn airms_getParamsBuf(AiqV4l2Device_t* pAiIspDev, AiqV4l2Buffer_t** pV4l2Buf)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    uint32_t frameId          = -1;

    *pV4l2Buf = NULL;
    if (pAiIspDev) {
        *pV4l2Buf = AiqV4l2Device_getBuf(pAiIspDev, -1);
        if (!*pV4l2Buf) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "Can not get aiisp params buffer, queued cnts:%d \n",
                            AiqV4l2Device_getQueuedBufCnt(pAiIspDev));
            ret = XCAM_RETURN_ERROR_PARAM;
        }
    } else {
        ret = XCAM_RETURN_ERROR_FAILED;
    }

    return ret;
}

static XCamReturn airms_updateParams(AiqV4l2Device_t* pAiIspDev, AiqV4l2Buffer_t* pV4l2Buf)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (!pAiIspDev || !pV4l2Buf) {
        return XCAM_RETURN_BYPASS;
    }

    if (AiqV4l2Device_qbuf(pAiIspDev, pV4l2Buf, true) != 0) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM,
                        "RKISP1: failed to ioctl VIDIOC_QBUF %d %s.\n",
                        errno, strerror(errno));
        AiqV4l2Device_returnBufToPool(pAiIspDev, pV4l2Buf);
        return XCAM_RETURN_ERROR_IOCTL;
    }

    int timeout = 1;
    uint32_t buf_counts = AiqV4l2Device_getBufCnt(pAiIspDev);
    uint32_t try_time   = 3;
    while (AiqV4l2Device_getQueuedBufCnt(pAiIspDev) > 2) {
        if (pAiIspDev->poll_event(pAiIspDev, timeout, -1) <= 0) {
            LOGW_CAMHW_SUBM(ISP20HW_SUBM, "poll params error, queue cnts: %d !",
                            AiqV4l2Device_getQueuedBufCnt(pAiIspDev));
            if (AiqV4l2Device_getQueuedBufCnt(pAiIspDev) == buf_counts &&
                try_time > 0) {
                timeout = 30;
                try_time--;
                continue;
            } else
                break;
        }
        AiqV4l2Buffer_t* dqbuf = pAiIspDev->_dequeue_buffer(pAiIspDev);
        if (!dqbuf) {
            XCAM_LOG_WARNING("dequeue buffer failed");
            // return ret;
        } else {
            AiqV4l2Device_returnBufToPool(pAiIspDev, dqbuf);
        }
    }
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn airms_config_param(AiqAiRmsStreamProcUnit_t* pProcUnit)
{
    AiqV4l2Buffer_t* pV4l2Buf = NULL;

    airms_getParamsBuf(pProcUnit->mAiIspDev, &pV4l2Buf);
    if (pV4l2Buf) {
        struct rkaiisp_params* aiisp_cfg = (struct rkaiisp_params*)(AiqV4l2Buffer_getBuf(pV4l2Buf)->m.userptr);
        struct rkaiisp_other_cfg *other_cfg = &aiisp_cfg->other_cfg;
        struct rkaiisp_model_info *model_info = (struct rkaiisp_model_info *)pProcUnit->mModelBuf;
        int i;

        aiisp_cfg->frame_id = 0;
        aiisp_cfg->module_update = RKAIISP_OTHER_UPDATE;
        other_cfg->sw_prev_blacklvl  = 0;
        other_cfg->sw_post_blacklvl  = 0;
        other_cfg->sw_pos_noiselimit = 32767;
        other_cfg->sw_neg_noiselimit = (0xFFFF - other_cfg->sw_pos_noiselimit + 1) | (1 << 15);
        for (int i = 0; i < 33; i++) {
            float x = (float)i / (33 - 1);

            x = airms_nonLinear(x, 0);
            other_cfg->sw_in_comp_y[i] = round(x * 1023);
        }
        for (int i = 0; i < 33; i++) {
            float x = (float)i / (33 - 1);

            x = airms_nonLinear(x, 1);
            other_cfg->sw_out_decomp_y[i] = round(x * 65535);
        }

        aiisp_cfg->model_mode = model_info->model_mode;
        aiisp_cfg->model_runcnt = model_info->model_runcnt;
        memcpy(&aiisp_cfg->model_cfg[0], &model_info->model_cfg[0], sizeof(model_info->model_cfg));
        memcpy(&aiisp_cfg->kwt_cfg, &model_info->kwt_cfg, sizeof(model_info->kwt_cfg));
        for (uint32_t i = 0; i < model_info->model_runcnt; i++) {
            uint32_t offset = model_info->kwt_cfg.kwt_offet[i] - sizeof(struct rkaiisp_model_info) + sizeof(struct rkaiisp_params);
            char *pKwtBuffer;

            pKwtBuffer = (char *)aiisp_cfg + offset;
            aiisp_cfg->kwt_cfg.kwt_offet[i] = offset;
            memcpy(pKwtBuffer, (char *)model_info + model_info->kwt_cfg.kwt_offet[i], model_info->kwt_cfg.kwt_pad_size[i]);
        }

        aiisp_cfg->module_update |= RKAIISP_MODEL_UPDATE;

        //airms_params_print(aiisp_cfg);
        airms_updateParams(pProcUnit->mAiIspDev, pV4l2Buf);
        return XCAM_RETURN_NO_ERROR;
    } else {
        LOGE_AIRMS("%s: Can not get param buffer", __func__);
        return XCAM_RETURN_ERROR_FAILED;
    }
}

static void AiRmsStream_mergeAiispOut(const AirmsMgeParam* attrib)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ENTER_ANALYZER_FUNCTION();

    struct timeval start_time, end_time;
    unsigned long thread_id = (unsigned long)pthread_self();

    gettimeofday(&start_time, NULL);

    AiqAiRmsStreamProcUnit_t* pProcUnit = attrib->pProcUnit;
    AiRmsAiispBuf *aiispOutBuf = attrib->aiispOutBuf;
    int hdlIdx = attrib->hdlIdx;
    int height = pProcUnit->isp_acq_height / AIRMS_HELP_THREAD_POOL_NUM;
    int width  = pProcUnit->isp_acq_width;
    int stride = CEIL_BY(pProcUnit->isp_acq_width, 2) + 2 * AIRMS_EXTEND_PIXEL;
    uint16_t* pSrc = (uint16_t*)aiispOutBuf->virt_addr;
    uint16_t* pDst = (uint16_t*)aiispOutBuf->virt_addr;

    pSrc += hdlIdx * stride * height;
    pSrc += CEIL_BY(pProcUnit->isp_acq_width, 2) / 2 + 2 * AIRMS_EXTEND_PIXEL;
    pDst += hdlIdx * stride * height;
    pDst += CEIL_BY(pProcUnit->isp_acq_width, 2) / 2;
    if (hdlIdx == AIRMS_MGE_THREAD_POOL_NUM - 1) {
        height += (pProcUnit->isp_acq_height - height * AIRMS_HELP_THREAD_POOL_NUM);
    }

    for (int i = 0; i < height; i++) {
        memcpy(pDst, pSrc, pProcUnit->isp_acq_width / 2 * 2);
        pSrc += stride;
        pDst += stride;
    }

    gettimeofday(&end_time, NULL);

    long seconds      = end_time.tv_sec - start_time.tv_sec;
    long micros       = ((seconds * 1000000) + end_time.tv_usec) - start_time.tv_usec;
    double elapsed_ms = micros / 1000.0;

    LOGD_AIRMS("TID: %lu, width %d, height %d, stride %d, hdlIdx %d, elapsed_ms %.2f ms",
        thread_id, width, height, stride, hdlIdx, elapsed_ms);

    EXIT_ANALYZER_FUNCTION();

    return;
}

static XCamReturn AiqAiRmsStreamProcUnit_poll_buffer_ready(void* ctx, AiqHwEvt_t* evt,
                                                          int dev_index) {
    XCAM_FAIL_RETURN(ERROR, ctx, XCAM_RETURN_ERROR_PARAM, "pAiRmsStreamProc is NULL!");
    XCamReturn ret                     = XCAM_RETURN_NO_ERROR;
    AiqAiRmsStreamProcUnit_t* pProcUnit = (AiqAiRmsStreamProcUnit_t*)ctx;
    AiqHwAinnEvt_t* nnevt = (AiqHwAinnEvt_t *)evt;
    struct rkaiisp_airms_st *airms_st = &nnevt->queue_buf.airms_st;
    AiRmsAiispBuf *aiispInBuf  = NULL;
    AiRmsAiispBuf *aiispOutBuf = NULL;
    int i;

    LOGI_AIRMS("%s: seq: %d, aiisp done: inbuf_idx %d, outbuf_idx %d",
        __func__, airms_st->sequence, airms_st->inbuf_idx, airms_st->outbuf_idx);

    // return in_fd to free list
    aiqMutex_lock(&pProcUnit->inbuf_mutex);
    for (int i = 0; i < AIRMS_INBUF_NUM; i++) {
        if (pProcUnit->inbuf_tbl[i].buf_idx == airms_st->inbuf_idx) {
            aiispInBuf = &pProcUnit->inbuf_tbl[i];
            aiispInBuf->in_used = false;
            break;
        }
    }
    aiqMutex_unlock(&pProcUnit->inbuf_mutex);

    if (!aiispInBuf) {
        LOGE_AIRMS("%s:%d can not find aiispInBuf", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_FAILED;
    }

    aiqMutex_lock(&pProcUnit->_device._buf_mutex);
    for (int i = 0; i < AIRMS_OUTBUF_NUM; i++) {
        if (pProcUnit->outbuf_tbl[i].buf_idx == airms_st->outbuf_idx) {
            aiispOutBuf = &pProcUnit->outbuf_tbl[i];
            break;
        }
    }
    if (!aiispOutBuf) {
        LOGE_AIRMS("%s:%d can not find aiispOutBuf", __FUNCTION__, __LINE__);
        aiqMutex_unlock(&pProcUnit->_device._buf_mutex);
        return XCAM_RETURN_ERROR_FAILED;
    }
    aiqMutex_unlock(&pProcUnit->_device._buf_mutex);

    if (pProcUnit->is_parthdl) {
        struct dma_buf_sync sync = { 0 };
        AirmsMgeParam mge_params;
        bool dump_flg = false;

        int64_t start_time = get_systime_us();

        sync.flags = DMA_BUF_SYNC_WRITE | DMA_BUF_SYNC_START;
        ioctl(aiispOutBuf->dma_fd, DMA_BUF_IOCTL_SYNC, &sync);

        //aiqMutex_lock(&pProcUnit->mStreamMutex);
        mge_params.aiispOutBuf = aiispOutBuf;
        mge_params.pProcUnit = pProcUnit;

        for (int n = 0; n < AIRMS_MGE_THREAD_POOL_NUM; n++) {
            mge_params.hdlIdx = n;
            thpool_add_work_ex(pProcUnit->mMgeThPool, (void*)AiRmsStream_mergeAiispOut, &mge_params, sizeof(mge_params));
        }

        thpool_wait(pProcUnit->mMgeThPool);

        //aiqMutex_unlock(&pProcUnit->mStreamMutex);

        sync.flags = DMA_BUF_SYNC_WRITE | DMA_BUF_SYNC_END;
        ioctl(aiispOutBuf->dma_fd, DMA_BUF_IOCTL_SYNC, &sync);

        int64_t end_time = get_systime_us();

        LOGD_AIRMS("aiisp out buffer merge time %lld", (end_time - start_time) / 1000);
    }

    if (aiispOutBuf->dump_flg) {
        uint32_t frame_id = AiqV4l2Buffer_getSequence(&aiispOutBuf->aiqV4l2Buf);

        dump_buffer("aiispout.raw", frame_id, (char *)AiqV4l2Buffer_getExpbufUsrptr(&aiispOutBuf->aiqV4l2Buf),
            AiqV4l2Buffer_getLength(&aiispOutBuf->aiqV4l2Buf));
    }

    // rkaiisp done, send out0 buf to isp
    aiispOutBuf->aiqV4l2Buf._buf.index     = aiispOutBuf->buf_idx;
    aiispOutBuf->aiqV4l2Buf._buf.type      = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    aiispOutBuf->aiqV4l2Buf._buf.bytesused = aiispOutBuf->buf_size;
    aiispOutBuf->aiqV4l2Buf._buf.memory    = V4L2_MEMORY_DMABUF;
    aiispOutBuf->aiqV4l2Buf._buf.m.fd      = aiispOutBuf->dma_fd;
    aiispOutBuf->aiqV4l2Buf._buf.sequence  = airms_st->sequence;
    aiispOutBuf->aiqV4l2Buf._buf.m.planes              = &aiispOutBuf->planes;
    aiispOutBuf->aiqV4l2Buf._buf.m.planes[0].length    = aiispOutBuf->buf_size;
    aiispOutBuf->aiqV4l2Buf._buf.m.planes[0].bytesused = aiispOutBuf->buf_size;
    aiispOutBuf->aiqV4l2Buf._expbuf_fd     = aiispOutBuf->dma_fd;
    aiispOutBuf->aiqV4l2Buf._length        = aiispOutBuf->buf_size;
    aiispOutBuf->aiqV4l2Buf._expbuf_usrptr = (uintptr_t)aiispOutBuf->virt_addr;
    aiispOutBuf->aiqV4l2Buf._device        = &pProcUnit->_device;
    aiispOutBuf->aiqV4l2Buf._buf.memory    = V4L2_MEMORY_DMABUF;
    aiispOutBuf->aiqV4l2Buf._vb._xcam_vb.unref = airms_V4l2Buffer_unref;
    LOGI_AIRMS("%s: seq %d, send buffer to isp: outbuf_idx %d, dma_fd %d, size %d",
        __func__, airms_st->sequence, aiispOutBuf->buf_idx, aiispOutBuf->dma_fd, aiispOutBuf->buf_size);
    AiqRawStreamProcUnit_send_sync_buf(pProcUnit->_proc_stream, &aiispOutBuf->aiqV4l2Buf, NULL, NULL);

    return ret;
}

XCamReturn AiqAiRmsStreamProcUnit_init(AiqCamHwBase_t* pCamHw, AiqAiRmsStreamProcUnit_t* pProcUnit, rk_aiq_aiisp_info_t* aiisp_info) {
    XCAM_FAIL_RETURN(ERROR, pProcUnit, XCAM_RETURN_ERROR_PARAM, "pAiRmsStreamProc is NULL!");

    pProcUnit->pCamHw           = pCamHw;
    pProcUnit->aiisp_info       = *aiisp_info;
    pProcUnit->mAiIspDev        = NULL;
    pProcUnit->mAiIspSubDev     = NULL;
    pProcUnit->_pcb             = NULL;
    pProcUnit->mStartFlag       = false;
    pProcUnit->mStartStreamFlag = false;
    strcpy(pProcUnit->mModelFile, "/etc/iqfiles/rkrms_model_info.bin");

    pProcUnit->_device._memory_type = V4L2_MEMORY_DMABUF;
    aiqMutex_init(&pProcUnit->mStreamMutex);
    aiqMutex_init(&pProcUnit->inbuf_mutex);
    aiqMutex_init(&pProcUnit->_device._buf_mutex);
    for (int i = 0; i < 33; i++) {
        float x = (float)i / (33 - 1);

        x = airms_nonLinear(x, 0);
        pProcUnit->mCompY[i] = round(x * 1023);
    }

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn airms_freeReource(AiqAiRmsStreamProcUnit_t* pProcUnit) {
    if (!pProcUnit) return XCAM_RETURN_ERROR_PARAM;

    if (pProcUnit->mAiIspDev && pProcUnit->mBufPoolInit) {
        int dummy = 0;

        if (pProcUnit->mAiIspDev->io_control(pProcUnit->mAiIspDev,
                                          RKAIISP_CMD_FREE_BUFPOOL, &dummy) != 0) {
            LOGE_AIRMS("%s:%d can't free bufpool", __FUNCTION__, __LINE__);
        }
        pProcUnit->mBufPoolInit = false;
    }
    for (int i = 0; i < AIRMS_INBUF_NUM; i++) {
        if (pProcUnit->inbuf_tbl[i].virt_addr != NULL) {
            munmap(pProcUnit->inbuf_tbl[i].virt_addr, pProcUnit->inbuf_tbl[i].buf_size);
            pProcUnit->inbuf_tbl[i].virt_addr = NULL;
            close(pProcUnit->inbuf_tbl[i].dma_fd);
            memset(&pProcUnit->inbuf_tbl[i], 0, sizeof(pProcUnit->inbuf_tbl[i]));
        }
    }
    for (int i = 0; i < AIRMS_OUTBUF_NUM; i++) {
        if (pProcUnit->outbuf_tbl[i].virt_addr != NULL) {
            munmap(pProcUnit->outbuf_tbl[i].virt_addr, pProcUnit->outbuf_tbl[i].buf_size);
            pProcUnit->outbuf_tbl[i].virt_addr = NULL;
            close(pProcUnit->outbuf_tbl[i].dma_fd);
            memset(&pProcUnit->outbuf_tbl[i], 0, sizeof(pProcUnit->outbuf_tbl[i]));
        }
    }

    if (pProcUnit->mModelBuf != NULL) {
        aiq_free(pProcUnit->mModelBuf);
        pProcUnit->mModelBuf = NULL;
    }
    if (pProcUnit->_pcb) {
        aiq_free(pProcUnit->_pcb);
        pProcUnit->_pcb = NULL;
    }
    if (pProcUnit->_AiRmsStream) {
        AiqStream_deinit(pProcUnit->_AiRmsStream);
        aiq_free(pProcUnit->_AiRmsStream);
        pProcUnit->_AiRmsStream = NULL;
    }
    if (pProcUnit->mAiIspSubDev) {
        AiqV4l2SubDevice_deinit(pProcUnit->mAiIspSubDev);
        aiq_free(pProcUnit->mAiIspSubDev);
        pProcUnit->mAiIspSubDev = NULL;
    }
    if (pProcUnit->mAiIspDev) {
        AiqV4l2Device_deinit(pProcUnit->mAiIspDev);
        aiq_free(pProcUnit->mAiIspDev);
        pProcUnit->mAiIspDev = NULL;
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqAiRmsStreamProcUnit_deinit(AiqAiRmsStreamProcUnit_t* pProcUnit) {
    if (!pProcUnit) return XCAM_RETURN_ERROR_PARAM;

    airms_freeReource(pProcUnit);
    aiqMutex_deInit(&pProcUnit->mStreamMutex);
    aiqMutex_deInit(&pProcUnit->inbuf_mutex);
    aiqMutex_deInit(&pProcUnit->_device._buf_mutex);
    pProcUnit->pCamHw = NULL;
    return XCAM_RETURN_NO_ERROR;
}

void AiqAiRmsStreamProcUnit_set_devices(AiqAiRmsStreamProcUnit_t* pProcUnit, AiqRawStreamProcUnit_t* proc) {
    if (!pProcUnit) return;

    pProcUnit->_proc_stream = proc;
}

static XCamReturn AiqAiRmsStreamProcUnit_setParamInf(AiqAiRmsStreamProcUnit_t* pProcUnit, struct rkaiisp_param_info *param_info)
{
    if (pProcUnit->mAiIspDev) {
        pProcUnit->mParamInfo = *param_info;
        if (pProcUnit->mAiIspDev->io_control(pProcUnit->mAiIspDev,
                                          RKAIISP_CMD_SET_PARAM_INFO, param_info) != 0) {
            LOGE_AIRMS("%s:%d can't set param info", __FUNCTION__, __LINE__);
            return XCAM_RETURN_ERROR_FAILED;
        }
    }
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn AiqAiRmsStreamProcUnit_setRmsBufInf(AiqAiRmsStreamProcUnit_t* pProcUnit, struct rkaiisp_rmsbuf_info *rmsbuf_info)
{
    if (pProcUnit->mAiIspDev) {
        if (pProcUnit->mBufPoolInit) {
            int dummy = 0;

            if (pProcUnit->mAiIspDev->io_control(pProcUnit->mAiIspDev,
                                              RKAIISP_CMD_FREE_BUFPOOL, &dummy) != 0) {
                LOGE_AIRMS("%s:%d can't free bufpool", __FUNCTION__, __LINE__);
                return XCAM_RETURN_ERROR_FAILED;
            }
            pProcUnit->mBufPoolInit = false;
        }

        if (pProcUnit->mAiIspDev->io_control(pProcUnit->mAiIspDev,
                                          RKAIISP_CMD_INIT_AIRMS_BUFPOOL, rmsbuf_info) != 0) {
            LOGE_AIRMS("%s:%d can't int bufpool", __FUNCTION__, __LINE__);
            return XCAM_RETURN_ERROR_FAILED;
        } else {
            pProcUnit->mBufPoolInit = true;
        }
    }
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn airms_prepareBuffer(AiqAiRmsStreamProcUnit_t* pProcUnit)
{
    uint32_t isp_acq_width  = pProcUnit->isp_acq_width;
    uint32_t isp_acq_height = pProcUnit->isp_acq_height;
    struct rkaiisp_param_info param_info;
    struct rkaiisp_rmsbuf_info rmsbuf_info;
    u32 bin_width, bin_height;
    char *virt_addr;

    // send paraminfo
    airms_getParamInfo(pProcUnit, &param_info);
    if (AiqAiRmsStreamProcUnit_setParamInf(pProcUnit, &param_info) != XCAM_RETURN_NO_ERROR) {
        LOGE_AIRMS("%s:%d setParamInf failed", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_FAILED;
    }

    // get input/output buffer fd from aiisp driver
    int proc_height = isp_acq_height + (32 - isp_acq_height % 32) % 32;
    int proc_width = isp_acq_width + (32 - isp_acq_width % 32) % 32;
    int gain_width_com  = (proc_width + 7) / 8;
    int gain_height_com = proc_height / 2;

    rmsbuf_info.image_width   = CEIL_BY(proc_width, 16);
    rmsbuf_info.image_height  = proc_height;
    rmsbuf_info.sigma_width   = rmsbuf_info.image_width / 2;
    rmsbuf_info.sigma_height  = rmsbuf_info.image_height / 2;
    rmsbuf_info.narmap_width  = gain_width_com * 2;
    rmsbuf_info.narmap_height = gain_height_com;
    rmsbuf_info.inbuf_num     = AIRMS_INBUF_NUM;
    rmsbuf_info.outbuf_num    = AIRMS_OUTBUF_NUM;
    if (AiqAiRmsStreamProcUnit_setRmsBufInf(pProcUnit, &rmsbuf_info) != XCAM_RETURN_NO_ERROR) {
        LOGE_AIRMS("%s:%d setRmsBufInf failed", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_FAILED;
    }

    pProcUnit->mRmsbufInfo = rmsbuf_info;

    bin_width  = CEIL_BY(CEIL_DOWN(rmsbuf_info.image_width, 2), 2);
    bin_height = CEIL_BY(CEIL_DOWN(rmsbuf_info.image_height, 2), 2);

    // push input/output buffer fd to list
    for (int i = 0; i < AIRMS_INBUF_NUM; i++) {
        pProcUnit->inbuf_tbl[i].in_used  = false;
        pProcUnit->inbuf_tbl[i].buf_idx  = i;
        pProcUnit->inbuf_tbl[i].dma_fd   = rmsbuf_info.inbuf_fd[i];
        pProcUnit->inbuf_tbl[i].buf_size = rmsbuf_info.image_width * rmsbuf_info.image_height * 2 + bin_width * bin_height;
        virt_addr = (char*)mmap(NULL, pProcUnit->inbuf_tbl[i].buf_size, PROT_READ | PROT_WRITE, MAP_SHARED, pProcUnit->inbuf_tbl[i].dma_fd, 0);
        if (MAP_FAILED == virt_addr) {
            LOGE_AIRMS("inbuf fd %d, size %d mmap failed", pProcUnit->inbuf_tbl[i].dma_fd, pProcUnit->inbuf_tbl[i].buf_size);
            pProcUnit->inbuf_tbl[i].virt_addr = NULL;
        } else {
            pProcUnit->inbuf_tbl[i].virt_addr = virt_addr;
        }
        AiqV4l2Buffer_init(&pProcUnit->inbuf_tbl[i].aiqV4l2Buf, &pProcUnit->v4l2_buf, &pProcUnit->v4l2_format, &pProcUnit->_device);
        pProcUnit->inbuf_tbl[i].aiqV4l2Buf._expbuf_fd     = pProcUnit->inbuf_tbl[i].dma_fd;
        pProcUnit->inbuf_tbl[i].aiqV4l2Buf._length        = pProcUnit->inbuf_tbl[i].buf_size;
        pProcUnit->inbuf_tbl[i].aiqV4l2Buf._expbuf_usrptr = (uintptr_t)pProcUnit->inbuf_tbl[i].virt_addr;
        pProcUnit->inbuf_tbl[i].aiqV4l2Buf._device        = &pProcUnit->_device;
        pProcUnit->inbuf_tbl[i].aiqV4l2Buf._buf.memory    = V4L2_MEMORY_DMABUF;
        pProcUnit->inbuf_tbl[i].aiqV4l2Buf._vb._xcam_vb.unref = airms_V4l2Buffer_unref;
        pProcUnit->inbuf_tbl[i].aiqV4l2Buf._buf.m.planes              = &pProcUnit->inbuf_tbl[i].planes;
        pProcUnit->inbuf_tbl[i].aiqV4l2Buf._buf.m.planes[0].length    = pProcUnit->inbuf_tbl[i].buf_size;
        pProcUnit->inbuf_tbl[i].aiqV4l2Buf._buf.m.planes[0].bytesused = pProcUnit->inbuf_tbl[i].buf_size;
    }
    for (int i = 0; i < AIRMS_OUTBUF_NUM; i++) {
        pProcUnit->outbuf_tbl[i].in_used = false;
        pProcUnit->outbuf_tbl[i].buf_idx = i;
        pProcUnit->outbuf_tbl[i].dma_fd = rmsbuf_info.outbuf_fd[i];
        pProcUnit->outbuf_tbl[i].buf_size = rmsbuf_info.image_width * rmsbuf_info.image_height * 2;
        if (pProcUnit->is_parthdl)
            pProcUnit->outbuf_tbl[i].buf_size += 2 * AIRMS_EXTEND_PIXEL * rmsbuf_info.image_height * 2;
        virt_addr = (char*)mmap(NULL, pProcUnit->outbuf_tbl[i].buf_size, PROT_READ | PROT_WRITE, MAP_SHARED, pProcUnit->outbuf_tbl[i].dma_fd, 0);
        if (MAP_FAILED == virt_addr) {
            LOGE_AIRMS("outbuf fd %d, size %d mmap failed", pProcUnit->outbuf_tbl[i].dma_fd, pProcUnit->outbuf_tbl[i].buf_size);
            pProcUnit->outbuf_tbl[i].virt_addr = NULL;
        } else {
            pProcUnit->outbuf_tbl[i].virt_addr = virt_addr;
        }
        AiqV4l2Buffer_init(&pProcUnit->outbuf_tbl[i].aiqV4l2Buf, &pProcUnit->v4l2_buf, &pProcUnit->v4l2_format, &pProcUnit->_device);
        pProcUnit->outbuf_tbl[i].aiqV4l2Buf._expbuf_fd     = pProcUnit->outbuf_tbl[i].dma_fd;
        pProcUnit->outbuf_tbl[i].aiqV4l2Buf._length        = pProcUnit->outbuf_tbl[i].buf_size;
        pProcUnit->outbuf_tbl[i].aiqV4l2Buf._expbuf_usrptr = (uintptr_t)pProcUnit->outbuf_tbl[i].virt_addr;
        pProcUnit->outbuf_tbl[i].aiqV4l2Buf._device        = &pProcUnit->_device;
        pProcUnit->outbuf_tbl[i].aiqV4l2Buf._buf.memory    = V4L2_MEMORY_DMABUF;
        pProcUnit->outbuf_tbl[i].aiqV4l2Buf._vb._xcam_vb.unref = airms_V4l2Buffer_unref;
        pProcUnit->outbuf_tbl[i].aiqV4l2Buf._buf.m.planes              = &pProcUnit->outbuf_tbl[i].planes;
        pProcUnit->outbuf_tbl[i].aiqV4l2Buf._buf.m.planes[0].length    = pProcUnit->outbuf_tbl[i].buf_size;
        pProcUnit->outbuf_tbl[i].aiqV4l2Buf._buf.m.planes[0].bytesused = pProcUnit->outbuf_tbl[i].buf_size;
    }
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqAiRmsStreamProcUnit_prepare(AiqAiRmsStreamProcUnit_t* pProcUnit,
                                         uint32_t isp_acq_width, uint32_t isp_acq_height,
                                         char *model_file) {
    XCAM_FAIL_RETURN(ERROR, pProcUnit, XCAM_RETURN_ERROR_PARAM, "pAiRmsStreamProc is NULL!");

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    char *model_buf;

    pProcUnit->mPrepareOK = false;
    AiqAiRmsStreamProcUnit_stop(pProcUnit);

    aiqMutex_lock(&pProcUnit->mStreamMutex);
    airms_freeReource(pProcUnit);

    pProcUnit->mAiIspDev = (AiqV4l2Device_t*)aiq_mallocz(sizeof(AiqV4l2Device_t));
    if (!pProcUnit->mAiIspDev) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "%d: alloc fail !", __LINE__);
        goto fail;
    }
    AiqV4l2Device_init(pProcUnit->mAiIspDev, pProcUnit->aiisp_info.video_path);
    pProcUnit->mAiIspDev->open(pProcUnit->mAiIspDev, false);

    pProcUnit->mAiIspSubDev = (AiqV4l2SubDevice_t*)aiq_mallocz(sizeof(AiqV4l2SubDevice_t));
    if (!pProcUnit->mAiIspSubDev) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "%d: alloc fail !", __LINE__);
        goto fail;
    }
    AiqV4l2SubDevice_init(pProcUnit->mAiIspSubDev, pProcUnit->aiisp_info.subdev_path);
    pProcUnit->mAiIspSubDev->_v4l_base.open(&pProcUnit->mAiIspSubDev->_v4l_base, false);
    pProcUnit->_AiRmsStream = (AiqStream_t*)aiq_mallocz(sizeof(AiqStream_t));
    AiqStream_init(pProcUnit->_AiRmsStream, (AiqV4l2Device_t*)pProcUnit->mAiIspSubDev, ISP_POLL_AIRMS_DONE);
    if (!pProcUnit->_pcb) {
        AiqPollCallback_t* pcb = (AiqPollCallback_t*)aiq_mallocz(sizeof(AiqPollCallback_t));
        pcb->_pCtx             = pProcUnit;
        pcb->poll_buffer_ready = AiqAiRmsStreamProcUnit_poll_buffer_ready;
        pcb->poll_event_ready  = NULL;
        pProcUnit->_pcb        = pcb;
        AiqPollThread_setPollCallback(pProcUnit->_AiRmsStream->_poll_thread, pcb);
    }

    strcpy(pProcUnit->mModelFile, model_file);
    if (airms_readModel(pProcUnit, pProcUnit->mModelFile, &model_buf) != XCAM_RETURN_NO_ERROR) {
        LOGE_AIRMS("%s: read model file failed", __func__);
        goto fail;
    }
    if (model_buf != NULL)
        pProcUnit->mModelBuf = model_buf;

    pProcUnit->isp_acq_width   = isp_acq_width;
    pProcUnit->isp_acq_height  = isp_acq_height;

    pProcUnit->is_parthdl = false;
    if (isp_acq_width > AIRMS_MAX_WIDTH)
        pProcUnit->is_parthdl = true;

    LOGI_AIRMS("%s: model_file %s, isp_acq_width %d, isp_acq_height %d", __func__, model_file, isp_acq_width, isp_acq_height);
    airms_prepareBuffer(pProcUnit);
    pProcUnit->mPrepareOK = true;

    aiqMutex_unlock(&pProcUnit->mStreamMutex);

    return ret;
fail:
    aiqMutex_unlock(&pProcUnit->mStreamMutex);
    return XCAM_RETURN_ERROR_FAILED;
}

void AiqAiRmsStreamProcUnit_start(AiqAiRmsStreamProcUnit_t* pProcUnit) {
    int cpu_cores[AIRMS_HELP_THREAD_POOL_NUM] = {0};

    if (!pProcUnit || !pProcUnit->mModelBuf || !pProcUnit->mPrepareOK) return;

    aiqMutex_lock(&pProcUnit->mStreamMutex);
    pProcUnit->mStartOK = false;

    for (int i = 0; i < AIRMS_INBUF_NUM; i++) {
        pProcUnit->inbuf_tbl[i].in_used  = false;
    }
    for (int i = 0; i < AIRMS_OUTBUF_NUM; i++) {
        pProcUnit->outbuf_tbl[i].in_used = false;
    }

    pProcUnit->dump_raw_num = 0;
    if (pProcUnit->_AiRmsStream && !pProcUnit->mStartFlag) {
        pProcUnit->_AiRmsStream->start(pProcUnit->_AiRmsStream);
        AiqV4l2Device_subscribeEvt(pProcUnit->_AiRmsStream->_dev, RKAIISP_V4L2_EVENT_AIISP_DONE);
        pProcUnit->mStartFlag = true;
        LOGD_AIRMS("start airms stream device");
    }
    if (pProcUnit->mAiIspDev) {
        AiqV4l2Device_start(pProcUnit->mAiIspDev, false);
    }

    for (int i = 0; i < AIRMS_HELP_THREAD_POOL_NUM; i++) {
        cpu_cores[i] = i;
    }

    pProcUnit->mHelpThPool = thpool_init_ex(AIRMS_HELP_THREAD_POOL_NUM,
                AIRMS_HELP_THREAD_SCHED_POLICY,
                AIRMS_HELP_THREAD_SCHED_PRIORITY,
                cpu_cores, AIRMS_HELP_THREAD_POOL_NUM);
    AiRmsQuardConvertThd_init(&pProcUnit->mQuardConvertThd, pProcUnit);
    AiRmsQuardConvertThd_start(&pProcUnit->mQuardConvertThd);

    for (int i = 0; i < AIRMS_MGE_THREAD_POOL_NUM; i++) {
        cpu_cores[i] = i + AIRMS_MGE_THREAD_START_COREID;
    }

    pProcUnit->mMgeThPool = thpool_init_ex(AIRMS_MGE_THREAD_POOL_NUM,
                AIRMS_HELP_THREAD_SCHED_POLICY,
                AIRMS_HELP_THREAD_SCHED_PRIORITY,
                cpu_cores, AIRMS_MGE_THREAD_POOL_NUM);

    // send first param to aiisp
    airms_config_param(pProcUnit);
    pProcUnit->mStartOK = true;
    aiqMutex_unlock(&pProcUnit->mStreamMutex);
}

void AiqAiRmsStreamProcUnit_stop(AiqAiRmsStreamProcUnit_t* pProcUnit) {
    if (!pProcUnit || !pProcUnit->mPrepareOK || !pProcUnit->mStartFlag) return;

    aiqMutex_lock(&pProcUnit->mStreamMutex);
    pProcUnit->mStartOK = false;
    AiRmsQuardConvertThd_stop(&pProcUnit->mQuardConvertThd);
    AiRmsQuardConvertThd_deinit(&pProcUnit->mQuardConvertThd);
    thpool_destroy(pProcUnit->mHelpThPool);
    thpool_destroy(pProcUnit->mMgeThPool);
    if (pProcUnit->_AiRmsStream && pProcUnit->mStartFlag) {
        pProcUnit->_AiRmsStream->stopThreadOnly(pProcUnit->_AiRmsStream);
        AiqV4l2Device_unsubscribeEvt(pProcUnit->_AiRmsStream->_dev, RKAIISP_V4L2_EVENT_AIISP_DONE);
        pProcUnit->_AiRmsStream->stopDeviceOnly(pProcUnit->_AiRmsStream);
        pProcUnit->mStartFlag = false;
    }
    if (pProcUnit->mAiIspDev)
        AiqV4l2Device_stop(pProcUnit->mAiIspDev);
    aiqMutex_unlock(&pProcUnit->mStreamMutex);
}

void AiqAiRmsStreamProcUnit_setVicapBuf(AiqAiRmsStreamProcUnit_t* pProcUnit, AiqV4l2Buffer_t *vicapbuf) {
    if (!pProcUnit || !pProcUnit->mAiIspDev) return;

    aiqMutex_lock(&pProcUnit->mStreamMutex);
    if (!pProcUnit->mStartOK) {
        LOGE_AIRMS("%s: AiRmsStream is not start", __func__);
        aiqMutex_unlock(&pProcUnit->mStreamMutex);
        return;
    }

    LOGI_AIRMS("%s: seq: %d, get vicap buffer: dma_fd %d length %d",
        __func__, AiqV4l2Buffer_getSequence(vicapbuf), AiqV4l2Buffer_getExpbufFd(vicapbuf), AiqV4l2Buffer_getLength(vicapbuf));

    AirmsQuardConvertParam params;

    params.vicapbuf = vicapbuf;
    AiqVideoBuffer_ref(vicapbuf);
    AiRmsQuardConvertThd_push_attr(&pProcUnit->mQuardConvertThd, &params);
    aiqMutex_unlock(&pProcUnit->mStreamMutex);
}

int AiqAiRmsStreamProcUnit_dumpRaw(AiqAiRmsStreamProcUnit_t* pProcUnit, int dump_raw_num)
{
    int ret = -1;

    if (pProcUnit) {
        ret = 0;
        pProcUnit->dump_raw_num = dump_raw_num;
    }

    return ret;
}

void AiqAiRmsStreamProcUnit_getBytesPerline(AiqAiRmsStreamProcUnit_t* pProcUnit, uint32_t width, uint32_t* bytes_perline)
{
    *bytes_perline = 0;
    if (pProcUnit) {
        if (width > AIRMS_MAX_WIDTH) {
            *bytes_perline = 2 * (width + 2 * AIRMS_EXTEND_PIXEL);
        }
    }
    LOGK_AIRMS("%s: bytes_perline %d", __func__, *bytes_perline);
}


