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

#include "aiq_streamProcInfo.h"

#include "c_base/aiq_list.h"
#include "c_base/aiq_map.h"
#include "dumpcam_server/info/include/rk_info_utils.h"
#include "dumpcam_server/info/include/st_string.h"
#include "xcore_c/aiq_v4l2_device.h"

void stream_proc_dump_mod_param(AiqRawStreamProcUnit_t* self, st_string* result) {
    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "stream proc module param");
    snprintf(buffer, MAX_LINE_LENGTH, "%-12s", "drop_frame");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-12d", self->fe.frameloss);

    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");
}

void stream_proc_dump_dev_attr(AiqRawStreamProcUnit_t* self, st_string* result) {
    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "stream proc dev attr 1");

    snprintf(buffer, MAX_LINE_LENGTH, "%-15s%-9s%-8s%-9s%-11s%-11s", "dev_node", "phy_chn", "mode",
             "ext_dev", "multi_cam", "1608_strm");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");
    memset(buffer, 0, MAX_LINE_LENGTH);

    const char* mode = "linear";
    if (RK_AIQ_HDR_GET_WORKING_MODE(self->_working_mode) == RK_AIQ_WORKING_MODE_ISP_HDR2)
        mode = "HDR2";
    else if (RK_AIQ_HDR_GET_WORKING_MODE(self->_working_mode) == RK_AIQ_WORKING_MODE_ISP_HDR3)
        mode = "HDR3";

    snprintf(buffer, MAX_LINE_LENGTH, "%-15s%-9d%-8s%-9s%-11s%-11s",
             AiqV4l2Device_getDevName(self->_dev[0]), self->mCamPhyId, mode,
             self->_isExtDev ? "Y" : "N", self->_is_multi_cam_conc ? "Y" : "N",
             self->_is_1608_sensor ? "Y" : "N");

    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");
}

void stream_proc_dump_chn_attr1(AiqRawStreamProcUnit_t* self, st_string* result) {
    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "stream proc phychn attr 1");

    snprintf(buffer, MAX_LINE_LENGTH, "%-9s%-14s%-12s%-7s%-7s", "phy_chn", "pixel_format",
             "data_mode", "dst_w", "dst_h");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    char fmt_str[5];
    fmt_str[0] = AiqV4l2Device_getPixelFmt(self->_dev[0]) & 0xFF;
    fmt_str[1] = (AiqV4l2Device_getPixelFmt(self->_dev[0]) >> 8) & 0xFF;
    fmt_str[2] = (AiqV4l2Device_getPixelFmt(self->_dev[0]) >> 16) & 0xFF;
    fmt_str[3] = (AiqV4l2Device_getPixelFmt(self->_dev[0]) >> 24) & 0xFF;
    fmt_str[4] = '\0';

    const char* data_mode = "compact";
    if (self->data_mode == CSI_MEM_WORD_BIG_ALIGN) data_mode = "big_align";
    if (self->data_mode == CSI_MEM_WORD_LITTLE_ALIGN) data_mode = "little_align";

    uint32_t width = 0, height = 0;
    AiqV4l2Device_getSize(self->_dev[0], &width, &height);

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-9d%-14s%-12s%-7d%-7d", self->mCamPhyId, fmt_str, data_mode,
             width, height);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");
}

void stream_proc_dump_chn_status1(AiqRawStreamProcUnit_t* self, st_string* result) {
    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "stream proc phychn status 1");

    snprintf(buffer, MAX_LINE_LENGTH, "%-9s%-10s%-11s%-14s%-12s%-10s%-9s%-14s%-10s", "phy_chn",
             "trig_id", "trig_drop", "trig_time(s)", "trig_delay", "rx_id", "rx_drop", "rx_time(s)",
             "rx_delay");

    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    aiq_memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-9d%-10d%-11d%-14.3f%-12d%-10d%-9d%-14.3f%-10d",
             self->mCamPhyId, self->trig.id, self->trig.frameloss,
             self->trig.timestamp / 1000000.0f, self->trig.delay, self->fe.id, self->fe.frameloss,
             self->fe.timestamp / 1000000.0f, self->fe.delay);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");
}

void stream_proc_dump_chn_status2(AiqRawStreamProcUnit_t* self, st_string* result) {
    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "stream proc phychn status 2");

    snprintf(buffer, MAX_LINE_LENGTH, "%-9s%-10s%-9s%-10s%-11s%-11s", "phy_chn", "tg_seq",
             "tg_time", "buf_list", "cache_list", "ready_map");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    aiqMutex_lock(&self->_buf_mutex);
    snprintf(buffer, MAX_LINE_LENGTH, "%-9d%-10d%-9d%-10d%-11d%-11d", self->mCamPhyId,
             self->trig.id, self->trig_times, aiqList_size(self->buf_list[0]),
             aiqList_size(self->cache_list[0]), aiqMap_size(self->_isp_hdr_fid2ready_map));
    aiqMutex_unlock(&self->_buf_mutex);

    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");
}

void stream_proc_dump_videobuf_status(AiqRawStreamProcUnit_t* self, st_string* result) {
    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "stream proc video buf status");

    snprintf(buffer, MAX_LINE_LENGTH, "%-14s%-10s%-9s%-7s%-8s%-11s%-6s%-10s%-10s", "dev_node",
             "seq", "buf_idx", "fd", "queued", "sizeimage", "refs", "mem_type", "buf_type");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);

    int32_t cnt = AiqV4l2Device_getBufCnt(self->_dev[0]);
    for (int32_t i = 0; i < cnt; i++) {
        AiqV4l2Buffer_t* buf = AiqV4l2Device_getBufByIndex(self->_dev[0], i);
        snprintf(buffer, MAX_LINE_LENGTH, "%-14s%-10d%-9d%-7d%-8d%-11d%-6d%-10d%-10d",
                 AiqV4l2Device_getDevName(self->_dev[0]), self->fe.id,
                 AiqV4l2Buffer_getV4lBufIndex(buf), AiqV4l2Buffer_getExpbufFd(buf),
                 AiqV4l2Buffer_getQueued(buf), AiqV4l2Buffer_getV4lBufPlanarLength(buf, 0),
                 buf->_ref_cnts, AiqV4l2Device_getMemType(self->_dev[0]),
                 AiqV4l2Device_getBufType(self->_dev[0]));
        aiq_string_printf(result, buffer);
        aiq_string_printf(result, "\n");
    }

    aiq_string_printf(result, "\n");
}
