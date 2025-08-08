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

#include "aiq_sensorHwInfo.h"

#include <string.h>

#include "c_base/aiq_map.h"
#include "dumpcam_server/info/include/rk_info_utils.h"
#include "dumpcam_server/info/include/st_string.h"

void sensor_dump_mod_param(AiqSensorHw_t* self, st_string* result) {
    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "sensor module param");

    snprintf(buffer, MAX_LINE_LENGTH, "%-10s%-9s%-9s", "dev", "phy_chn", "i2c_exp");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    // sensor entity name format SHOULD be like this:
    // m00_b_ov13850 1-0010
    char name[32]       = {0};
    char* real_name_end = strchr(self->_sns_entity_name, ' ');
    int size            = (int)(real_name_end - self->_sns_entity_name) - 6;
    if (real_name_end) strncpy(name, self->_sns_entity_name + 6, size);

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-10s%-9d%-9s", name, self->mCamPhyId,
             self->_is_i2c_exp ? "Y" : "N");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");
}

void sensor_dump_dev_attr1(AiqSensorHw_t* self, st_string* result) {
    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "sensor dev attr 1");

    snprintf(buffer, MAX_LINE_LENGTH, "%-9s%-8s%-14s%-7s%-8s%-8s%-6s%-5s", "phy_chn", "mode",
             "pixel_format", "width", "height", "mirror", "flip", "dcg");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    const char* mode = "linear";
    if (RK_AIQ_HDR_GET_WORKING_MODE(self->_working_mode) == RK_AIQ_WORKING_MODE_ISP_HDR2) {
        mode = "HDR2";
    } else if (RK_AIQ_HDR_GET_WORKING_MODE(self->_working_mode) == RK_AIQ_WORKING_MODE_ISP_HDR3) {
        mode = "HDR3";
    }

    char fmt_str[5];
    fmt_str[0] = self->desc.sensor_pixelformat & 0xFF;
    fmt_str[1] = (self->desc.sensor_pixelformat >> 8) & 0xFF;
    fmt_str[2] = (self->desc.sensor_pixelformat >> 16) & 0xFF;
    fmt_str[3] = (self->desc.sensor_pixelformat >> 24) & 0xFF;
    fmt_str[4] = '\0';

    const char* dcg = "N";
    if (self->dcg_mode == GAIN_MODE_HCG)
        dcg = "HCG";
    else if (self->dcg_mode == GAIN_MODE_LCG)
        dcg = "LCG";

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-9d%-8s%-14s%-7d%-8d%-8s%-6s%-5s", self->mCamPhyId, mode,
             fmt_str, self->desc.sensor_output_width, self->desc.sensor_output_height,
             self->_mirror ? "Y" : "N", self->_flip ? "Y" : "N", dcg);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");
}

void sensor_dump_dev_attr2(AiqSensorHw_t* self, st_string* result) {
    struct v4l2_control ctrl;

    int hts = 0, h_blank = 0;
    memset(&ctrl, 0, sizeof(ctrl));
    ctrl.id = V4L2_CID_HBLANK;
    if (!AiqV4l2SubDevice_ioctl(self->mSd, VIDIOC_G_CTRL, &ctrl)) {
        h_blank = ctrl.value;
        hts     = self->desc.sensor_output_width + h_blank;
    }

    int vts = 0, v_blank = 0;
    memset(&ctrl, 0, sizeof(ctrl));
    ctrl.id = V4L2_CID_VBLANK;
    if (!AiqV4l2SubDevice_ioctl(self->mSd, VIDIOC_G_CTRL, &ctrl)) {
        v_blank = ctrl.value;
        vts     = self->desc.sensor_output_height + v_blank;
    }

    struct v4l2_subdev_frame_interval finterval;

    memset(&finterval, 0, sizeof(finterval));
    finterval.pad = 0;

    float fps = 0.0f;
    if (!AiqV4l2SubDevice_ioctl(self->mSd, VIDIOC_SUBDEV_G_FRAME_INTERVAL, &finterval)) {
        fps = (float)(finterval.interval.denominator) / finterval.interval.numerator;
    }

    double pclk = vts * hts * fps;

    double line_time = (double)hts / pclk;
    double vb_time   = line_time * v_blank;

    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "sensor dev attr 2");

    snprintf(buffer, MAX_LINE_LENGTH, "%-9s%-16s%-9s%-6s%-9s%-6s%-9s%-11s%-9s", "phy_chn",
             "pixel_clk(mHz)", "fps", "hts", "h-blank", "vts", "v-blank", "line_time", "vb_time");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-9d%-16.3f%-9.3f%-6d%-9d%-6d%-9d%-11.3f%-9.0f",
             self->mCamPhyId, self->desc.pixel_clock_freq_mhz, fps, hts, h_blank, vts, v_blank,
             line_time * 1000000 * 1000 / 1000, vb_time * 1000000);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");
}

void sensor_dump_reg_effect_delay(AiqSensorHw_t* self, st_string* result) {
    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "register activation delay");

    snprintf(buffer, MAX_LINE_LENGTH, "%-12s%-12s%-12s", "time_delay", "gain_delay", "dcg_delay");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-12d%-12d%-12d", self->_time_delay, self->_gain_delay,
             MAX(self->_dcg_gain_mode_delay, 0));
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");
}

void sensor_dump_exp_list_size(AiqSensorHw_t* self, st_string* result) {
    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "debug info for exposure config");

    snprintf(buffer, MAX_LINE_LENGTH, "%-10s%-10s%-16s%-15s%-14s", "seq", "exp_list",
             "effect_exp_map", "gain_del_list", "dcg_del_list");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    aiqMutex_lock(&self->_mutex);
    snprintf(buffer, MAX_LINE_LENGTH, "%-10d%-10d%-16d%-15d%-14d", self->_frame_sequence,
             aiqList_size(self->_exp_list), aiqMap_size(self->_effecting_exp_map),
             aiqList_size(self->_delayed_gain_list),
             aiqList_size(self->_delayed_dcg_gain_mode_list));
    aiqMutex_unlock(&self->_mutex);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");
}

void __dump_configured_linear_exp(AiqSensorHw_t* self, st_string* result) {
    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "exposure configured to drv");

    AiqMapItem_t* pItem = NULL;
    bool rm             = false;
    snprintf(buffer, MAX_LINE_LENGTH, "%-10s%-6s%-6s%-10s%-10s%-10s%-12s", "seq", "time", "again",
             "time_f", "again_f", "dgain_f", "ispdgain_f");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    AIQ_MAP_FOREACH(self->_effecting_exp_map, pItem, rm) {
        if (!pItem) goto out;

        memset(buffer, 0, MAX_LINE_LENGTH);
        AiqSensorExpInfo_t* pExp = *((AiqSensorExpInfo_t**)(pItem->_pData));
        snprintf(buffer, MAX_LINE_LENGTH, "%-10d%-6d%-6d%-10.6f%-10.2f%-10.2f%-12.2f",
                 pExp->_base.frame_id,
                 pExp->aecExpInfo.LinearExp.exp_sensor_params.coarse_integration_time,
                 pExp->aecExpInfo.LinearExp.exp_sensor_params.analog_gain_code_global,
                 pExp->aecExpInfo.LinearExp.exp_real_params.integration_time,
                 pExp->aecExpInfo.LinearExp.exp_real_params.analog_gain,
                 pExp->aecExpInfo.LinearExp.exp_real_params.digital_gain,
                 pExp->aecExpInfo.LinearExp.exp_real_params.isp_dgain);
        aiq_string_printf(result, buffer);
        aiq_string_printf(result, "\n");
    }

out:
    aiq_string_printf(result, "\n");
}

void __dump_configured_hdr_short_exp(AiqSensorHw_t* self, st_string* result) {
    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "HDR short-frame exposure configured to drv");

    AiqMapItem_t* pItem = NULL;
    bool rm             = false;
    snprintf(buffer, MAX_LINE_LENGTH, "%-10s%-6s%-6s%-10s%-10s%-10s%-12s", "id", "time", "again",
             "time_f", "again_f", "dgain_f", "ispdgain_f");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    AIQ_MAP_FOREACH(self->_effecting_exp_map, pItem, rm) {
        if (!pItem) goto out;

        memset(buffer, 0, MAX_LINE_LENGTH);

        AiqSensorExpInfo_t* pExp = *((AiqSensorExpInfo_t**)(pItem->_pData));
        snprintf(buffer, MAX_LINE_LENGTH, "%-10d%-6d%-6d%-10.6f%-10.2f%-10.2f%-12.2f",
                 pExp->_base.frame_id,
                 pExp->aecExpInfo.HdrExp[0].exp_sensor_params.coarse_integration_time,
                 pExp->aecExpInfo.HdrExp[0].exp_sensor_params.analog_gain_code_global,
                 pExp->aecExpInfo.HdrExp[0].exp_real_params.integration_time,
                 pExp->aecExpInfo.HdrExp[0].exp_real_params.analog_gain,
                 pExp->aecExpInfo.HdrExp[0].exp_real_params.digital_gain,
                 pExp->aecExpInfo.HdrExp[0].exp_real_params.isp_dgain);
        aiq_string_printf(result, buffer);
        aiq_string_printf(result, "\n");
    }

out:
    aiq_string_printf(result, "\n");
}

void __dump_configured_hdr_middle_exp(AiqSensorHw_t* self, st_string* result) {
    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "HDR mid-frame exposure configured to drv");

    AiqMapItem_t* pItem = NULL;
    bool rm             = false;
    snprintf(buffer, MAX_LINE_LENGTH, "%-10s%-6s%-6s%-10s%-10s%-10s%-12s", "id", "time", "again",
             "time_f", "again_f", "dgain_f", "ispdgain_f");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    AIQ_MAP_FOREACH(self->_effecting_exp_map, pItem, rm) {
        if (!pItem) goto out;

        memset(buffer, 0, MAX_LINE_LENGTH);

        AiqSensorExpInfo_t* pExp = *((AiqSensorExpInfo_t**)(pItem->_pData));
        snprintf(buffer, MAX_LINE_LENGTH, "%-10d%-6d%-6d%-10.6f%-10.2f%-10.2f%-12.2f",
                 pExp->_base.frame_id,
                 pExp->aecExpInfo.HdrExp[1].exp_sensor_params.coarse_integration_time,
                 pExp->aecExpInfo.HdrExp[1].exp_sensor_params.analog_gain_code_global,
                 pExp->aecExpInfo.HdrExp[1].exp_real_params.integration_time,
                 pExp->aecExpInfo.HdrExp[1].exp_real_params.analog_gain,
                 pExp->aecExpInfo.HdrExp[1].exp_real_params.digital_gain,
                 pExp->aecExpInfo.HdrExp[1].exp_real_params.isp_dgain);
        aiq_string_printf(result, buffer);
        aiq_string_printf(result, "\n");
    }

out:
    aiq_string_printf(result, "\n");
}

void __dump_configured_hdr_long_exp(AiqSensorHw_t* self, st_string* result) {
    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "HDR long-frame exposure configured to drv");

    AiqMapItem_t* pItem = NULL;
    bool rm             = false;
    snprintf(buffer, MAX_LINE_LENGTH, "%-10s%-6s%-6s%-10s%-10s%-10s%-12s", "id", "time", "again",
             "time_f", "again_f", "dgain_f", "ispdgain_f");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    AIQ_MAP_FOREACH(self->_effecting_exp_map, pItem, rm) {
        if (!pItem) goto out;

        memset(buffer, 0, MAX_LINE_LENGTH);

        AiqSensorExpInfo_t* pExp = *((AiqSensorExpInfo_t**)(pItem->_pData));
        snprintf(buffer, MAX_LINE_LENGTH, "%-10d%-6d%-6d%-10.6f%-10.2f%-10.2f%-12.2f",
                 pExp->_base.frame_id,
                 pExp->aecExpInfo.HdrExp[2].exp_sensor_params.coarse_integration_time,
                 pExp->aecExpInfo.HdrExp[2].exp_sensor_params.analog_gain_code_global,
                 pExp->aecExpInfo.HdrExp[2].exp_real_params.integration_time,
                 pExp->aecExpInfo.HdrExp[2].exp_real_params.analog_gain,
                 pExp->aecExpInfo.HdrExp[2].exp_real_params.digital_gain,
                 pExp->aecExpInfo.HdrExp[2].exp_real_params.isp_dgain);
        aiq_string_printf(result, buffer);
        aiq_string_printf(result, "\n");
    }

out:
    aiq_string_printf(result, "\n");
}

void sensor_dump_configured_exp(AiqSensorHw_t* self, st_string* result) {
    if (self->_working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
        __dump_configured_linear_exp(self, result);
    } else {
        __dump_configured_hdr_short_exp(self, result);
        __dump_configured_hdr_middle_exp(self, result);
        if (RK_AIQ_HDR_GET_WORKING_MODE(self->_working_mode) == RK_AIQ_WORKING_MODE_ISP_HDR3)
            __dump_configured_hdr_long_exp(self, result);
    }
}
