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

#include "aiq_groupAnalyzerInfo.h"

#include "aiq_algogroups_manager.h"
#include "rk_info_utils.h"

void group_analyzer_dump_mod_param(AiqAnalyzeGroupManager_t* self, st_string* result) {
    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "module param");

    snprintf(buffer, MAX_LINE_LENGTH, "%-9s%-15s", "phy_chn", "single_thread");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-9d%-15s", self->mAiqCore->mAlogsComSharedParams.mCamPhyId,
             self->mSingleThreadMode ? "Y" : "N");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");
}

void group_analyzer_dump_attr(AiqAnalyzeGroupManager_t* self, st_string* result) {
    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "group analyzer attr");

    bool isFirst             = true;
    AiqAnalyzerGroup_t* pGrp = NULL;
    for (int i = 0; i < (int)RK_AIQ_CORE_ANALYZE_MAX; i++) {
        pGrp = self->mGroupMap[i];
        if (!pGrp) continue;

        if (isFirst) {
            memset(buffer, 0, MAX_LINE_LENGTH);
            snprintf(buffer, MAX_LINE_LENGTH, "%-12s%-12s%-13s%-11s", "name", "st_vi_scale",
                     "user_delay", "awaken_id");
            aiq_string_printf(result, buffer);
            aiq_string_printf(result, "\n");
            isFirst = false;
        }

        memset(buffer, 0, MAX_LINE_LENGTH);
        char lower_case[32] = {0};
        xcam_to_lowercase(AnalyzerGroupType2Str[pGrp->mGroupType], lower_case);

        if (pGrp->mUserSetDelayCnts == INT8_MAX && pGrp->mAwakenId == (uint32_t)-1)
            snprintf(buffer, MAX_LINE_LENGTH, "%-12s%-12s%-13s%-11s", lower_case,
                     pGrp->mVicapScaleStart ? "Y" : "N", "\\", "\\");
        else if (pGrp->mUserSetDelayCnts == INT8_MAX)
            snprintf(buffer, MAX_LINE_LENGTH, "%-12s%-12s%-13s%-11d", lower_case,
                     pGrp->mVicapScaleStart ? "Y" : "N", "\\", pGrp->mAwakenId);
        else if (pGrp->mAwakenId == (uint32_t)-1)
            snprintf(buffer, MAX_LINE_LENGTH, "%-12s%-12s%-13d%-11s", lower_case,
                     pGrp->mVicapScaleStart ? "Y" : "N", pGrp->mUserSetDelayCnts, "\\");

        aiq_string_printf(result, buffer);
        aiq_string_printf(result, "\n");
    }

    aiq_string_printf(result, "\n");
}

void group_analyzer_dump_msg_hdl_status(AiqAnalyzeGroupManager_t* self, st_string* result) {
    if (!self->mSingleThreadMode) return;

    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "message handler status");

    snprintf(buffer, MAX_LINE_LENGTH, "%-6s%-10s%-11s", "name", "item_num", "using_num");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-6s%-10d%-11d", "msgQ",
             aiqList_num(self->mMsgThrd->mMsgsQueue), aiqList_size(self->mMsgThrd->mMsgsQueue));
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");
}

void group_analyzer_dump_msg_map_status1(AiqAnalyzeGroupManager_t* self, st_string* result) {
    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "message map status 1");

    bool isFirst             = true;
    AiqAnalyzerGroup_t* pGrp = NULL;
    for (int i = 0; i < (int)RK_AIQ_CORE_ANALYZE_MAX; i++) {
        pGrp = self->mGroupMap[i];
        if (!pGrp) continue;

        if (isFirst) {
            snprintf(buffer, MAX_LINE_LENGTH, "%-14s%-10s%-11s%-13s%-11s%-12s", "name", "item_num",
                     "item_size", "overflow_th", "using_num", "reduce_cnt");
            aiq_string_printf(result, buffer);
            aiq_string_printf(result, "\n");
            isFirst = false;
        }

        char lower_case[32] = {0};
        xcam_to_lowercase(AnalyzerGroupType2Str[pGrp->mGroupType], lower_case);

        memset(buffer, 0, MAX_LINE_LENGTH);
        snprintf(buffer, MAX_LINE_LENGTH, "%-14s%-10d%-11d%-13d%-11d%-12d", lower_case,
                 aiqMap_itemNums(pGrp->mGroupMsgMap), aiqMap_itemSize(pGrp->mGroupMsgMap),
                 pGrp->mGrpMsgOverflowCnt, aiqMap_size(pGrp->mGroupMsgMap), pGrp->mMsgReduceCnt);
        aiq_string_printf(result, buffer);
        aiq_string_printf(result, "\n");
    }

    aiq_string_printf(result, "\n");
}

void group_analyzer_dump_msg_map_status2(AiqAnalyzeGroupManager_t* self, st_string* result) {
    char buffer[MAX_LINE_LENGTH] = {0};

    AiqAnalyzerGroup_t* pGrp = NULL;
    bool isFirst             = true;
    for (int i = 0; i < (int)RK_AIQ_CORE_ANALYZE_MAX; i++) {
        pGrp = self->mGroupMap[i];
        if (!pGrp || aiqMap_size(pGrp->mGroupMsgMap) <= 0) continue;

        if (isFirst) {
            aiq_info_dump_title(result, "message map status 2");
            isFirst = false;
        }

        snprintf(buffer, MAX_LINE_LENGTH, "%-12s%-12s", "name", "frame_id");

        RkAiqGrpConditions_t* grp_conds = &pGrp->mGrpConds;
        char lower_case[32]             = {0};
        for (uint32_t j = 0; j < grp_conds->size; j++) {
            memset(lower_case, 0, sizeof(lower_case));
            xcam_to_lowercase(MessageType2Str[grp_conds->conds[j].cond], lower_case);
            snprintf(buffer + strlen(buffer), MAX_LINE_LENGTH, "%-16s", lower_case);
        }

        aiq_string_printf(result, buffer);
        aiq_string_printf(result, "\n");

        AiqMapItem_t* pItem = NULL;
        bool rm             = false;
        memset(lower_case, 0, sizeof(lower_case));
        xcam_to_lowercase(AnalyzerGroupType2Str[pGrp->mGroupType], lower_case);

        AIQ_MAP_FOREACH(pGrp->mGroupMsgMap, pItem, rm) {
            memset(buffer, 0, MAX_LINE_LENGTH);

            snprintf(buffer + strlen(buffer), MAX_LINE_LENGTH, "%-12s%-12d", lower_case,
                     (uint32_t)(intptr_t)pItem->_key);

            GroupMessage_t* pMsg = (GroupMessage_t*)(pItem->_pData);
            for (uint32_t j = 0; j < grp_conds->size; j++) {
                if (pMsg->msg_flags & BIT_ULL(grp_conds->conds[j].cond))
                    snprintf(buffer + strlen(buffer), MAX_LINE_LENGTH, "%-16s", "Y");
                else
                    snprintf(buffer + strlen(buffer), MAX_LINE_LENGTH, "%-16s", "N");
            }
            aiq_string_printf(result, buffer);
            aiq_string_printf(result, "\n");
        }

        aiq_string_printf(result, "\n");
    }
}
