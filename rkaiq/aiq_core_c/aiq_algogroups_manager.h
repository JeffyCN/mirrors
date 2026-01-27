/*
 * Copyright (c) 2024 Rockchip Eletronics Co., Ltd.
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
 */

#ifndef _AIQ_ALGO_GROUPS_MANAGER_H_
#define _AIQ_ALGO_GROUPS_MANAGER_H_

#include "common/rk_aiq_comm.h"
#include "common/rk_aiq_types_priv_c.h"
#include "aiq_core_c/AiqCoreConfig.h"
#include "c_base/aiq_map.h"
#include "c_base/aiq_thread.h"
#include "c_base/aiq_list.h"
#if RKAIQ_HAVE_DUMPSYS
#include "st_string.h"
#endif

RKAIQ_BEGIN_DECLARE

typedef struct AiqCore_s AiqCore_t;
typedef struct GroupMessage_s GroupMessage_t;
typedef struct AiqCoreMsg_s AiqCoreMsg_t;
typedef struct AiqAlgoHandler_s AiqAlgoHandler_t;
typedef struct AiqAnalyzeGroupMsgHdlThread_s AiqAnalyzeGroupMsgHdlThread_t;
typedef XCamReturn (*MessageHandleWrapper)(AiqCore_t* aiqCore,
                                           AiqCoreMsg_t*,
                                           int, uint32_t,
                                           uint64_t);

#define MAX_MESSAGES 5
#define GROUP_MSG_OVERFLOW_TH 2

typedef struct AiqAnalyzerGroup_s {
    AiqCore_t* mAiqCore;
    enum rk_aiq_core_analyze_type_e mGroupType;
    uint64_t mDepsFlag;
    RkAiqGrpConditions_t mGrpConds;
    AiqAnalyzeGroupMsgHdlThread_t* mRkAiqGroupMsgHdlTh;
    // key: uint32_t, val: GroupMessage_t
    AiqMap_t* mGroupMsgMap;
    MessageHandleWrapper mHandler;
    int8_t mUserSetDelayCnts;
    bool mVicapScaleStart;
    uint32_t mAwakenId;
    uint8_t mGrpMsgOverflowCnt;
#if RKAIQ_HAVE_DUMPSYS
    uint32_t mMsgReduceCnt;
#endif
} AiqAnalyzerGroup_t;

XCamReturn AiqAnalyzerGroup_init(AiqAnalyzerGroup_t* pGroup, AiqCore_t* aiqCore,
                                 enum rk_aiq_core_analyze_type_e type, const uint64_t flag,
                                 const RkAiqGrpConditions_t* grpConds, const bool singleThrd);
void AiqAnalyzerGroup_deinit(AiqAnalyzerGroup_t* pGroup);

XCamReturn AiqAnalyzerGroup_start(AiqAnalyzerGroup_t* pGroup);
bool AiqAnalyzerGroup_pushMsg(AiqAnalyzerGroup_t* pGroup, AiqCoreMsg_t* msg);
bool AiqAnalyzerGroup_msgHandle(AiqAnalyzerGroup_t* pGroup, AiqCoreMsg_t* msg);
XCamReturn AiqAnalyzerGroup_stop(AiqAnalyzerGroup_t* pGroup);

enum rk_aiq_core_analyze_type_e AiqAnalyzerGroup_getType(AiqAnalyzerGroup_t* pGroup);
uint64_t AiqAnalyzerGroup_getDepsFlag(AiqAnalyzerGroup_t* pGroup);
void AiqAnalyzerGroup_setDepsFlag(AiqAnalyzerGroup_t* pGroup, uint64_t new_deps);
void AiqAnalyzerGroup_setDepsFlagAndClearMap(AiqAnalyzerGroup_t* pGroup, uint64_t new_deps);
AiqCore_t* AiqAnalyzerGroup_getAiqCore(AiqAnalyzerGroup_t* pGroup);
void AiqAnalyzerGroup_setDelayCnts(AiqAnalyzerGroup_t* pGroup, int8_t delayCnts);
void AiqAnalyzerGroup_setVicapScaleFlag(AiqAnalyzerGroup_t* pGroup, bool mode);

#define AiqAnalyzerGroup_setWakenId(pGroup, sequence) \
    pGroup->mAwakenId = sequence

struct AiqAnalyzeGroupMsgHdlThread_s {
    AiqThread_t* _base;
    // AiqAnalyzerGroup_t*
    AiqList_t* mHandlerGroups;
    // AiqCoreMsg_t
    AiqList_t* mMsgsQueue;
    AiqMutex_t _mutex;
    AiqCond_t _cond;
    bool bQuit;
};

XCamReturn AiqAnalyzeGroupMsgHdlThread_init(AiqAnalyzeGroupMsgHdlThread_t* pHdlTh, char* name,
                                            AiqAnalyzerGroup_t* group);

void AiqAnalyzeGroupMsgHdlThread_deinit(AiqAnalyzeGroupMsgHdlThread_t* pHdlTh);
void AiqAnalyzeGroupMsgHdlThread_add_group(AiqAnalyzeGroupMsgHdlThread_t* pHdlTh,
                                           AiqAnalyzerGroup_t* group);
void AiqAnalyzeGroupMsgHdlThread_start(AiqAnalyzeGroupMsgHdlThread_t* pHdlTh);
void AiqAnalyzeGroupMsgHdlThread_stop(AiqAnalyzeGroupMsgHdlThread_t* pHdlTh);
bool AiqAnalyzeGroupMsgHdlThread_push_msg(AiqAnalyzeGroupMsgHdlThread_t* pHdlTh,
                                          AiqCoreMsg_t* buffer);

typedef struct AiqAnalyzeGroupManager_s {
    AiqCore_t* mAiqCore;
    bool mSingleThreadMode;
    // RK_AIQ_CORE_ANALYZE_MAX means RK_AIQ_CORE_ANALYZE_ALL
    AiqAlgoHandler_t* mGroupAlgoListMap[RK_AIQ_CORE_ANALYZE_MAX + 1][RK_AIQ_ALGO_TYPE_MAX];
    int mGroupAlgoListCnts[RK_AIQ_CORE_ANALYZE_MAX + 1];
    AiqAnalyzerGroup_t* mGroupMap[RK_AIQ_CORE_ANALYZE_MAX];
    AiqAnalyzeGroupMsgHdlThread_t* mMsgThrd;
    int mDefaultDelayCnt;
} AiqAnalyzeGroupManager_t;

XCamReturn AiqAnalyzeGroupManager_init(AiqAnalyzeGroupManager_t* pGroupMan, AiqCore_t* aiqCore,
                                       bool single_thread);
void AiqAnalyzeGroupManager_deinit(AiqAnalyzeGroupManager_t* pGroupMan);
void AiqAnalyzeGroupManager_parseAlgoGroup(AiqAnalyzeGroupManager_t* pGroupMan,
                                           const struct RkAiqAlgoDesCommExt* algoDes);

#define AiqAnalyzeGroupManager_getGrpDeps(pGroupMan, group) \
       (pGroupMan)->mGroupMap[group] ? (pGroupMan)->mGroupMap[group]->mDepsFlag : 0

#define AiqAnalyzeGroupManager_setGrpDeps(pGroupMan, group, new_deps) \
       (pGroupMan)->mGroupMap[group] && ((pGroupMan)->mGroupMap[group]->mDepsFlag = new_deps)

XCamReturn AiqAnalyzeGroupManager_start(AiqAnalyzeGroupManager_t* pGroupMan);
XCamReturn AiqAnalyzeGroupManager_stop(AiqAnalyzeGroupManager_t* pGroupMan);
void AiqAnalyzeGroupManager_clean(AiqAnalyzeGroupManager_t* pGroupMan);
void AiqAnalyzeGroupManager_setDelayCnts(AiqAnalyzeGroupManager_t* pGroupMan, int delayCnts);

XCamReturn AiqAnalyzeGroupManager_firstAnalyze(AiqAnalyzeGroupManager_t* pGroupMan);
XCamReturn AiqAnalyzeGroupManager_handleMessage(AiqAnalyzeGroupManager_t* pGroupMan,
                                                AiqCoreMsg_t* msg);
AiqAlgoHandler_t** AiqAnalyzeGroupManager_getGroupAlgoList(AiqAnalyzeGroupManager_t* pGroupMan,
                                                      enum rk_aiq_core_analyze_type_e group, int* lens);
AiqAnalyzerGroup_t** AiqAnalyzeGroupManager_getGroups(AiqAnalyzeGroupManager_t* pGroupMan);
void AiqAnalyzeGroupManager_rmAlgoHandle(AiqAnalyzeGroupManager_t* pGroupMan, int algoType);
void AiqAnalyzeGroupManager_awakenClean(AiqAnalyzeGroupManager_t* pGroupMan, uint32_t sequence);
XCamReturn AiqAnalyzeGroupManager_resetDelayCnt(AiqAnalyzeGroupManager_t* pGroupMan, int delayCnt);

#if RKAIQ_HAVE_DUMPSYS
int AiqAnalyzerGroup_dump(void* self, st_string* result, int argc, void* argv[]);
#endif

RKAIQ_END_DECLARE

#endif
