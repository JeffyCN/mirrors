/*
 *   Copyright (c) 2024 Rockchip Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "aiq_CamHwBaseInfo.h"

#include "dumpcam_server/info/include/rk_info_utils.h"
#include "dumpcam_server/info/include/st_string.h"
#include "hwi_c/aiq_rawStreamCapUnit.h"
#include "hwi_c/aiq_rawStreamProcUnit.h"

extern SnsFullInfoWraps_t* g_mSensorHwInfos;

void hwi_base_dump_mod_param(AiqCamHwBase_t* self, st_string* result) {
    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "base module param");

    snprintf(buffer, MAX_LINE_LENGTH, "%-12s%-9s%-8s%-10s%-11s%-11s", "dev", "phy_chn", "mode",
             "readback", "multi_isp", "use_aiisp");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");
    memset(buffer, 0, MAX_LINE_LENGTH);

    const char* mode = "linear";
    if (RK_AIQ_HDR_GET_WORKING_MODE(self->mWorkingMode) == RK_AIQ_WORKING_MODE_ISP_HDR2)
        mode = "HDR2";
    else if (RK_AIQ_HDR_GET_WORKING_MODE(self->mWorkingMode) == RK_AIQ_WORKING_MODE_ISP_HDR3)
        mode = "HDR3";

    SnsFullInfoWraps_t* pItem = g_mSensorHwInfos;
    while (pItem) {
        if (strcmp(self->sns_name, pItem->key) == 0) break;
        pItem = pItem->next;
    }

    const char* driver = "unlinked";
    if (pItem) {
        rk_sensor_full_info_t* s_info = &pItem->data;
        if (s_info) driver = s_info->isp_info->driver;
    }

    snprintf(buffer, MAX_LINE_LENGTH, "%-12s%-9d%-8s%-10s%-11s%-11s", driver, self->mCamPhyId, mode,
             self->mNoReadBack ? "N" : "Y", g_mIsMultiIspMode ? "Y" : "N",
             self->use_aiisp ? "Y" : "N");

    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");
}

void hwi_base_dump_chn_status(AiqCamHwBase_t* self, st_string* result) {
    char buffer[MAX_LINE_LENGTH] = {0};
    aiq_info_dump_title(result, "base phychn status 1");

    snprintf(buffer, MAX_LINE_LENGTH, "%-9s%-10s%-9s%-14s%-10s%-10s%-12s%-14s%-13s%-8s", "phy_chn",
             "fs_id", "fs_drop", "fs_time(s)", "fs_delay", "stats_id", "stats_drop",
             "stats_time(s)", "stats_delay", "rate");

    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    aiq_memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-9d%-10d%-9d%-14.3f%-10d%-10d%-12d%-14.3f%-13d%-8d",
             self->mCamPhyId, self->prev_fs.id, self->prev_fs.frameloss,
             self->prev_fs.timestamp / 1000000.0f, self->prev_fs.delay, self->stats.id,
             self->stats.frameloss, self->stats.timestamp / 1000000.0f, self->stats.delay,
             self->stats.interval);

    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");
}

void hwi_base_dump_chn_status1(AiqCamHwBase_t* self, st_string* result) {
    if (self->mNoReadBack) return;

    char buffer[MAX_LINE_LENGTH] = {0};
    aiq_info_dump_title(result, "base phychn status 2");

    snprintf(buffer, MAX_LINE_LENGTH,
             "%-9s%-10s%-9s%-11s%-10s%-10s%-11s%-11s%-12s%-10s%-9s%-11s%-10s", "phy_chn", "tx_id",
             "tx_drop", "tx_time", "tx_delay", "trig_id", "trig_drop", "trig_time", "trig_delay",
             "rx_id", "rx_drop", "rx_time", "rx_delay");

    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    aiq_memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH,
             "%-9d%-10d%-9d%-11.3f%-10d%-10d%-11d%-11.3f%-12d%-10d%-9d%-11.3f%-10d",
             self->mCamPhyId, self->mRawCapUnit->fe.id, self->mRawCapUnit->fe.frameloss,
             self->mRawCapUnit->fe.timestamp / 1000000.0f, self->mRawCapUnit->fe.delay,
             self->mRawProcUnit->trig.id, self->mRawProcUnit->trig.frameloss,
             self->mRawProcUnit->trig.timestamp / 1000000.0f, self->mRawProcUnit->trig.delay,
             self->mRawProcUnit->fe.id, self->mRawProcUnit->fe.frameloss,
             self->mRawProcUnit->fe.timestamp / 1000000.0f, self->mRawProcUnit->fe.delay);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");
}

void hwi_base_dump_stats_videobuf_status(AiqCamHwBase_t* self, st_string* result) {
    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "isp stats dev video buf status");

    snprintf(buffer, MAX_LINE_LENGTH, "%-14s%-10s%-9s%-7s%-8s%-11s%-6s%-10s%-10s", "dev_node",
             "seq", "buf_idx", "fd", "queued", "sizeimage", "refs", "mem_type", "buf_type");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);

    int32_t cnt = AiqV4l2Device_getBufCnt(self->mIspStatsDev);
    for (int32_t i = 0; i < cnt; i++) {
        AiqV4l2Buffer_t* buf = AiqV4l2Device_getBufByIndex(self->mIspStatsDev, i);
        snprintf(buffer, MAX_LINE_LENGTH, "%-14s%-10d%-9d%-7d%-8d%-11d%-6d%-10d%-10d",
                 AiqV4l2Device_getDevName(self->mIspStatsDev), AiqV4l2Buffer_getSequence(buf),
                 AiqV4l2Buffer_getV4lBufIndex(buf), AiqV4l2Buffer_getExpbufFd(buf),
                 AiqV4l2Buffer_getQueued(buf), AiqV4l2Buffer_getV4lBufLength(buf), buf->_ref_cnts,
                 AiqV4l2Device_getMemType(self->mIspStatsDev),
                 AiqV4l2Device_getBufType(self->mIspStatsDev));
        aiq_string_printf(result, buffer);
        aiq_string_printf(result, "\n");
    }

    aiq_string_printf(result, "\n");
}

void hwi_base_dump_params_videobuf_status(AiqCamHwBase_t* self, st_string* result) {
    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "isp params dev video buf status");

    snprintf(buffer, MAX_LINE_LENGTH, "%-14s%-10s%-9s%-7s%-8s%-11s%-6s%-10s%-10s", "dev_node",
             "seq", "buf_idx", "fd", "queued", "sizeimage", "refs", "mem_type", "buf_type");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);

    int32_t cnt = AiqV4l2Device_getBufCnt(self->mIspParamsDev);
    for (int32_t i = 0; i < cnt; i++) {
        AiqV4l2Buffer_t* buf = AiqV4l2Device_getBufByIndex(self->mIspParamsDev, i);
        snprintf(buffer, MAX_LINE_LENGTH, "%-14s%-10d%-9d%-7d%-8d%-11d%-6d%-10d%-10d",
                 AiqV4l2Device_getDevName(self->mIspParamsDev), AiqV4l2Buffer_getSequence(buf),
                 AiqV4l2Buffer_getV4lBufIndex(buf), AiqV4l2Buffer_getExpbufFd(buf),
                 AiqV4l2Buffer_getQueued(buf), AiqV4l2Buffer_getV4lBufLength(buf), buf->_ref_cnts,
                 AiqV4l2Device_getMemType(self->mIspParamsDev),
                 AiqV4l2Device_getBufType(self->mIspParamsDev));
        aiq_string_printf(result, buffer);
        aiq_string_printf(result, "\n");
    }

    aiq_string_printf(result, "\n");
}

void hwi_base_dump_params_configure_status(AiqCamHwBase_t* self, st_string* result) {
    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "configure isp params status");

    snprintf(buffer, MAX_LINE_LENGTH, "%-14s%-10s%-9s%-7s%-8s%-11s%-6s%-10s%-10s", "dev_node",
             "seq", "buf_idx", "fd", "queued", "sizeimage", "refs", "mem_type", "buf_type");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);

    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");
}
