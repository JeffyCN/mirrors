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

#include "aiq_coreBufMgrInfo.h"

#include <ctype.h>

#include "aiq_algo_handler.h"
#include "rk_info_utils.h"

void core_buf_mgr_dump_mod_param(AiqCore_t* self, st_string* result) {
    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "module param");

    snprintf(buffer, MAX_LINE_LENGTH, "%-9s", "phy_chn");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-9d", self->mAlogsComSharedParams.mCamPhyId);

    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");
}

static void __snprintf_strings(const char* key_word, AiqPool_t* pool, uint32_t no_free_cnt,
                               char* buffer, st_string* result) {
    if (!pool) return;

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-14s%-10d%-11d%-10d%-13d", key_word,
             aiqPool_getItemNums(pool), aiqPool_getItemSize(pool), aiqPool_freeNums(pool),
             no_free_cnt);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");
}

void core_buf_mgr_dump_mem_pool_info(AiqCore_t* self, st_string* result) {
    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "mem pool info");

    snprintf(buffer, MAX_LINE_LENGTH, "%-14s%-10s%-11s%-10s%-13s", "pool_name", "item_num",
             "item_size", "free_num", "no_free_cnt");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    __snprintf_strings("sof_info", self->mAiqSofInfoWrapperPool, self->mNoFreeBufCnt.sofInfo,
                       buffer, result);

    __snprintf_strings("fullParam", self->mAiqFullParamsPool, self->mNoFreeBufCnt.fullParams,
                       buffer, result);
    for (int i = 0; i < RESULT_TYPE_MAX_PARAM; i++) {
        if (!self->mAiqParamsPoolArray[i] || aiqPool_getItemNums(self->mAiqParamsPoolArray[i]) <= 0)
            continue;

        __snprintf_strings(Cam3aResultType2Str[i], self->mAiqParamsPoolArray[i],
                           self->mNoFreeBufCnt.algosParams[i], buffer, result);
    }

    __snprintf_strings("ae_stats", self->mAiqAecStatsPool, self->mNoFreeBufCnt.aeStats, buffer,
                       result);
    __snprintf_strings("awb_stats", self->mAiqAwbStatsPool, self->mNoFreeBufCnt.awbStats, buffer,
                       result);
    __snprintf_strings("dhaz_stats", self->mAiqAdehazeStatsPool, self->mNoFreeBufCnt.dhazStats,
                       buffer, result);
    __snprintf_strings("gain_stats", self->mAiqAgainStatsPool, self->mNoFreeBufCnt.gainStats,
                       buffer, result);
    __snprintf_strings("af_stats", self->mAiqAfStatsPool, 0, buffer, result);
#if RKAIQ_HAVE_PDAF
    __snprintf_strings("pdaf_stats", self->mAiqPdafStatsPool, 0, buffer, result);
#endif
    __snprintf_strings("bay3d_stats", self->mbay3dStatPool, self->mNoFreeBufCnt.btnrStats, buffer,
                       result);

    __snprintf_strings("ae_pre_res", self->mPreResAeSharedPool, self->mNoFreeBufCnt.aePreRes,
                       buffer, result);
    __snprintf_strings("awb_proc_res", self->mProcResAwbSharedPool, self->mNoFreeBufCnt.awbProcRes,
                       buffer, result);
    __snprintf_strings("blc_proc_res", self->mProcResBlcSharedPool, self->mNoFreeBufCnt.blcProcRes,
                       buffer, result);
    __snprintf_strings("ynr_proc_res", self->mProcResYnrSharedPool, self->mNoFreeBufCnt.ynrProcRes,
                       buffer, result);

    aiq_string_printf(result, "\n");
}

void core_buf_mgr_dump_pending_param_map_info(AiqCore_t* self, st_string* result) {
    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "pending param map info");

    snprintf(buffer, MAX_LINE_LENGTH, "%-14s%-10s%-11s%-18s%-13s", "name", "item_num", "item_size",
             "max_pending_size", "pending_num");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-14s%-10d%-11d%-18d%-13d", "pendingParam",
             aiqMap_itemNums(self->mFullParamsPendingMap),
             aiqMap_itemSize(self->mFullParamsPendingMap), self->mDefaultDelayCnt,
             aiqMap_size(self->mFullParamsPendingMap));
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");
}

void core_buf_mgr_dump_pending_param_map_status(AiqCore_t* self, st_string* result) {
    if (aiqMap_size(self->mFullParamsPendingMap) <= 0) return;

    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "pending param map status");

    memset(buffer, 0, MAX_LINE_LENGTH);
    aiqMutex_lock(&self->_mFullParam_mutex);
    bool rm             = false;
    AiqMapItem_t* pItem = NULL;
    bool isFirst        = true;
    AIQ_MAP_FOREACH(self->mFullParamsPendingMap, pItem, rm) {
        if (isFirst) {
            memset(buffer, 0, MAX_LINE_LENGTH);
            snprintf(buffer, MAX_LINE_LENGTH, "%-12s%-7s", "seq", "ready");
            for (int i = 0; i < (int)RK_AIQ_CORE_ANALYZE_MAX; i++) {
                if (self->mFullParamReqGroupsMasks & BIT_ULL(i)) {
                    char grp_type[32] = {0};
                    xcam_to_lowercase(AnalyzerGroupType2Str[i], grp_type);
                    snprintf(buffer + strlen(buffer), MAX_LINE_LENGTH, "%-11s", grp_type);
                }
            }
            aiq_string_printf(result, buffer);
            aiq_string_printf(result, "\n");
            isFirst = false;
        }

        memset(buffer, 0, MAX_LINE_LENGTH);
        pending_params_t* pPendingParams = (pending_params_t*)pItem->_pData;
        snprintf(buffer, MAX_LINE_LENGTH, "%-12d%-7s", (int32_t)(intptr_t)pItem->_key,
                 pPendingParams->ready ? "Y" : "N");
        for (int i = 0; i < (int)RK_AIQ_CORE_ANALYZE_MAX; i++) {
            if (self->mFullParamReqGroupsMasks & BIT_ULL(i)) {
                if (pPendingParams->groupMasks & BIT_ULL(i))
                    snprintf(buffer + strlen(buffer), MAX_LINE_LENGTH, "%-11s", "Y");
                else
                    snprintf(buffer + strlen(buffer), MAX_LINE_LENGTH, "%-11s", "N");
            }
        }

        aiq_string_printf(result, buffer);
        aiq_string_printf(result, "\n");
    }
    aiqMutex_unlock(&self->_mFullParam_mutex);

    aiq_string_printf(result, "\n");
}

int core_buf_mgr_dump(void* self, st_string* result, int argc, void* argv[]) {
    core_buf_mgr_dump_mod_param((AiqCore_t*)self, result);
    core_buf_mgr_dump_mem_pool_info((AiqCore_t*)self, result);
    core_buf_mgr_dump_pending_param_map_info((AiqCore_t*)self, result);
    core_buf_mgr_dump_pending_param_map_status((AiqCore_t*)self, result);

    return 0;
}
