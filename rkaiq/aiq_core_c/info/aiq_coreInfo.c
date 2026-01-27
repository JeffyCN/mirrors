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

#include "aiq_coreInfo.h"

#include "rk_info_utils.h"

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif
static const char* State2Str[16] = {
    [RK_AIQ_CORE_STATE_INVALID] = "invalid",   [RK_AIQ_CORE_STATE_INITED] = "inited",
    [RK_AIQ_CORE_STATE_PREPARED] = "prepared", [RK_AIQ_CORE_STATE_STARTED] = "started",
    [RK_AIQ_CORE_STATE_RUNNING] = "running",   [RK_AIQ_CORE_STATE_STOPED] = "stoped",
};
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif

void core_dump_mod_param(AiqCore_t* self, st_string* result) {
    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "module param");

    snprintf(buffer, MAX_LINE_LENGTH, "%-9s%-14s%-16s%-15s", "phy_chn", "status", "force_done_cnt",
             "force_done_id");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-9d%-14s%-16d%-15d", self->mAlogsComSharedParams.mCamPhyId,
             State2Str[self->mState], self->mForceDoneCnt, self->mForceDoneId);

    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");
}

void core_dump_mod_attr(AiqCore_t* self, st_string* result) {
    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "core attr");

    snprintf(buffer, MAX_LINE_LENGTH, "%-9s%-8s%-11s%-11s%-11s%-10s", "phy_chn", "mode",
             "multi_isp", "cam_group", "gray_mode", "fastboot");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    const char* mode = "linear";
    if (RK_AIQ_HDR_GET_WORKING_MODE(self->mAlogsComSharedParams.working_mode) ==
        RK_AIQ_WORKING_MODE_ISP_HDR2)
        mode = "HDR2";
    else if (RK_AIQ_HDR_GET_WORKING_MODE(self->mAlogsComSharedParams.working_mode) ==
             RK_AIQ_WORKING_MODE_ISP_HDR3)
        mode = "HDR3";

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-9d%-8s%-11s%-11s%-11s%-10s",
             self->mAlogsComSharedParams.mCamPhyId, mode,
             self->mAlogsComSharedParams.is_multi_isp_mode ? "Y" : "N",
             self->mCamGroupCoreManager ? "Y" : "N",
             self->mGrayMode == RK_AIQ_GRAY_MODE_ON ? "Y" : "N",
             self->mTbInfo.is_fastboot ? "Y" : "N");

    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");
}

void core_dump_mod_status(AiqCore_t* self, st_string* result) {
    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "core status");

    snprintf(buffer, MAX_LINE_LENGTH, "%-9s%-12s%-10s%-12s%-10s", "phy_chn", "fs_id", "fs_drop",
             "stats_id", "stats_drop");

    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    aiq_memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-9d%-12d%-10d%-12d%-10d",
             self->mAlogsComSharedParams.mCamPhyId, self->mLatestEvtsId, self->mSofLoss,
             self->mLatestStatsId, self->mStatsLoss);

    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");
}

void core_dump_mod_3a_stats(AiqCore_t* self, st_string* result) {
    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "core 3a stats");

    snprintf(buffer, MAX_LINE_LENGTH, "%-9s%-12s%-4s%-5s%-4s%-6s%-8s%-6s", "phy_chn", "seq", "ae",
             "awb", "af", "btnr", "dehaze", "gain");

    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    aiq_memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-9d%-12d%-4s%-5s%-4s%-6s%-8s%-6s",
             self->mAlogsComSharedParams.mCamPhyId, self->mLatestStatsId,
             self->mIsStatsAvail.ae ? "Y" : "N", self->mIsStatsAvail.awb ? "Y" : "N",
             self->mIsStatsAvail.af ? "Y" : "N", self->mIsStatsAvail.btnr ? "Y" : "N",
             self->mIsStatsAvail.dhaze ? "Y" : "N", self->mIsStatsAvail.gain ? "Y" : "N");

    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");
}
