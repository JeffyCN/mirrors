/*
 * RkAiqCamGroupManager.h
 *
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
 *
 */

#ifndef _RK_AIQ_CAM_GROUP_MANAGER_C_H_
#define _RK_AIQ_CAM_GROUP_MANAGER_C_H_

#include "rk_aiq_algo_camgroup_types.h"
#include "aiq_core_c/aiq_algo_camgroup_handler.h"
#include "aiq_core_c/aiq_core.h"
#include "rk_aiq.h"

typedef struct AiqManager_s AiqManager_t;
typedef struct AiqCore_s AiqCore_t;

RKAIQ_BEGIN_DECLARE

typedef struct rk_aiq_singlecam_result_s {
    rk_aiq_singlecam_3a_result_t _3aResults;
    AiqFullParams_t* _fullIspParam;
    // aiq_awb_stats_wrapper_t
    aiq_stats_base_t* _mAwbStats;
    // aiq_ae_stats_wrapper_t
    aiq_stats_base_t* _mAeStats;
} rk_aiq_singlecam_result_t;

typedef struct rk_aiq_singlecam_result_status_s {
    rk_aiq_singlecam_result_t _singleCamResults;
    uint64_t _validAlgoResBits;
    uint64_t _validCoreMsgsBits;
    bool _ready;
} rk_aiq_singlecam_result_status_t;

typedef struct rk_aiq_groupcam_result_s {
    rk_aiq_singlecam_result_status_t _singleCamResultsStatus[RK_AIQ_CAM_GROUP_MAX_CAMS];
    uint8_t _validCamResBits;
    uint32_t _frameId;
    bool _ready;
    uint32_t _refCnt;
} rk_aiq_groupcam_result_t;

typedef struct AiqCamGroupManager_s AiqCamGroupManager_t;
typedef struct AiqCamGroupReprocTh_s {
    AiqThread_t* _base;
    AiqMutex_t _mutex;
    AiqCond_t _cond;
    bool bQuit;
    AiqCamGroupManager_t* pCamGrpMan;
    // rk_aiq_groupcam_result_t*
    AiqList_t* mMsgsQueue;
} AiqCamGroupReprocTh_t;

XCamReturn AiqCamGroupReprocTh_init(AiqCamGroupReprocTh_t* pReprocTh, char* name,
                                            AiqCamGroupManager_t* pCamGroupManager);
void AiqCamGroupReprocTh_deinit(AiqCamGroupReprocTh_t* pReprocTh);
void AiqCamGroupReprocTh_start(AiqCamGroupReprocTh_t* pReprocTh);
void AiqCamGroupReprocTh_stop(AiqCamGroupReprocTh_t* pReprocTh);
bool AiqCamGroupReprocTh_sendFrame(AiqCamGroupReprocTh_t* pReprocTh, rk_aiq_groupcam_result_t* gc_result);

// status transition
/*   Typical transitions:
 *        CURRENT_STATE              NEXT_STATE                  OPERATION
 *   CAMGROUP_MANAGER_INVALID -> CAMGROUP_MANAGER_BINDED          bind
 *   CAMGROUP_MANAGER_BINDED-> CAMGROUP_MANAGER_INITED            init
 *   CAMGROUP_MANAGER_INITED -> CAMGROUP_MANAGER_PREPARED         prepare
 *   CAMGROUP_MANAGER_PREPARED -> CAMGROUP_MANAGER_STARTED        start
 *   CAMGROUP_MANAGER_STARTED -> CAMGROUP_MANAGER_STOPED          stop
 *   CAMGROUP_MANAGER_PREPARED-> CAMGROUP_MANAGER_UNBINDED        unbind
 *   CAMGROUP_MANAGER_UNBINDED -> CAMGROUP_MANAGER_INVALID        deinit
 *
 *   Others:
 *   CAMGROUP_MANAGER_BINDED-> CAMGROUP_MANAGER_UNBINDED          unbind
 *   CAMGROUP_MANAGER_INITED -> CAMGROUP_MANAGER_UNBINDED         unbind
 *   CAMGROUP_MANAGER_PREPARED -> CAMGROUP_MANAGER_UNBINDED       unbind
 */
enum camgroup_manager_state_e {
    CAMGROUP_MANAGER_INVALID,
    CAMGROUP_MANAGER_BINDED,
    CAMGROUP_MANAGER_UNBINDED,
    CAMGROUP_MANAGER_INITED,
    CAMGROUP_MANAGER_PREPARED,
    CAMGROUP_MANAGER_STARTED,
    CAMGROUP_MANAGER_STOPED,
};

typedef struct AiqAlgoCamGroupHandler_s AiqAlgoCamGroupHandler_t;
struct AiqCamGroupManager_s
{
    /* normal processing */
    // add cam's AIQ ctx to cam group
    // receive group cam's awb,ae stats
    // receive group cam's awb,ae pre/proc results
    // receive group cam's genIspxxx results
    // running group algos, and replace the params
    // send all cam's params to hwi
    /* special for init params */
    // called only once
    const struct RkAiqAlgoDesCommExt* mGroupAlgosDesArray;
    /* key: camId, val: AiqManager_t* */
    AiqMap_t* mBindAiqsMap;
    /* key: frameId val: rk_aiq_groupcam_result_t* */
    AiqMap_t* mCamGroupResMap;
    AiqMutex_t mCamGroupResMutex;
    AiqCamGroupReprocTh_t mCamGroupReprocTh;
    /* */
    AiqMutex_t mCamGroupApiSyncMutex;
    uint64_t mRequiredMsgsMask;
    uint64_t mRequiredAlgoResMask;
    uint8_t mRequiredCamsResMask;
    uint8_t mVicapReadyMask;
    AlgoCtxInstanceCfgCamGroup mGroupAlgoCtxCfg;

    AiqAlgoCamGroupHandler_t* mDefAlgoHandleMap[RK_AIQ_ALGO_TYPE_MAX];
    AiqList_t* mDefAlgoHandleList;

    int mState;
    bool mInit;
    CamCalibDbCamgroup_t* mCamgroupCalib;
    uint32_t mClearedResultId;

    void* mGroupCtx;
    CamCalibDbV2Context_t mCalibv2;
    bool needReprepare;
    AiqMutex_t _update_mutex;
    AiqCond_t _update_done_cond;
    AiqFullParams_t _pLatestFullParams;
};

void AiqCamGroupManager_processAiqCoreMsgs(AiqCamGroupManager_t* pCamGrpMan, AiqCore_t* src, AiqCoreMsg_t* msg);
void AiqCamGroupManager_RelayAiqCoreResults(AiqCamGroupManager_t* pCamGrpMan, AiqCore_t* src, AiqFullParams_t* results);
XCamReturn AiqCamGroupManager_setCamgroupCalib(AiqCamGroupManager_t* pCamGrpMan, CamCalibDbCamgroup_t* camgroup_calib);

#define AiqCamGroupManager_setContainerCtx(pCamGrpMan, group_ctx) \
        (pCamGrpMan)->mGroupCtx = group_ctx

#define AiqCamGroupManager_getContainerCtx(pCamGrpMan) \
        (pCamGrpMan)->mGroupCtx

XCamReturn AiqCamGroupManager_construct(AiqCamGroupManager_t* pCamGrpMan);
// called after single cam aiq init
XCamReturn AiqCamGroupManager_init(AiqCamGroupManager_t* pCamGrpMan);
// called only once
XCamReturn AiqCamGroupManager_deInit(AiqCamGroupManager_t* pCamGrpMan);
// start analyze thread
XCamReturn AiqCamGroupManager_start(AiqCamGroupManager_t* pCamGrpMan);
// stop analyze thread
XCamReturn AiqCamGroupManager_stop(AiqCamGroupManager_t* pCamGrpMan);
// called before start(), get initial settings
XCamReturn AiqCamGroupManager_prepare(AiqCamGroupManager_t* pCamGrpMan);
// if called, prepare should be re-called
XCamReturn AiqCamGroupManager_bind(AiqCamGroupManager_t* pCamGrpMan, AiqManager_t* ctx);
XCamReturn AiqCamGroupManager_unbind(AiqCamGroupManager_t* pCamGrpMan, int camId);

#define AiqCamGroupManager_isRunningState(pCamGrpMan) \
        ((pCamGrpMan)->mState == CAMGROUP_MANAGER_STARTED)

XCamReturn AiqCamGroupManager_addAlgo(AiqCamGroupManager_t* pCamGrpMan, RkAiqAlgoDesComm* algo);
XCamReturn AiqCamGroupManager_enableAlgo(AiqCamGroupManager_t* pCamGrpMan, int algoType, int id, bool enable);
XCamReturn AiqCamGroupManager_rmAlgo(AiqCamGroupManager_t* pCamGrpMan, int algoType, int id);
bool AiqCamGroupManager_getAxlibStatus(AiqCamGroupManager_t* pCamGrpMan, int algoType, int id);
RkAiqAlgoContext* AiqCamGroupManager_getEnabledAxlibCtx(AiqCamGroupManager_t* pCamGrpMan, const int algo_type);
RkAiqAlgoContext* AiqCamGroupManager_getAxlibCtx(AiqCamGroupManager_t* pCamGrpMan, const int algo_type, const int lib_id);
AiqAlgoCamGroupHandler_t* AiqCamGroupManager_getAiqCamgroupHandle(AiqCamGroupManager_t* pCamGrpMan, const int algo_type, const int lib_id);
XCamReturn AiqCamGroupManager_calibTuning(AiqCamGroupManager_t* pCamGrpMan, const CamCalibDbV2Context_t* aiqCalib);
XCamReturn AiqCamGroupManager_updateCalibDb(AiqCamGroupManager_t* pCamGrpMan, const CamCalibDbV2Context_t* newCalibDb);
XCamReturn AiqCamGroupManager_rePrepare(AiqCamGroupManager_t* pCamGrpMan);

void AiqCamGroupManager_setVicapReady(AiqCamGroupManager_t* pCamGrpMan, rk_aiq_hwevt_t* hwevt);
bool AiqCamGroupManager_isAllVicapReady(AiqCamGroupManager_t* pCamGrpMan);
XCamReturn AiqCamGroupManager_register3Aalgo(AiqCamGroupManager_t* pCamGrpMan, void* algoDes, void *cbs);
XCamReturn AiqCamGroupManager_unregister3Aalgo(AiqCamGroupManager_t* pCamGrpMan, int algoType);

RKAIQ_END_DECLARE

#endif
