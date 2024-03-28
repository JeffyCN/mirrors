/*
 * RkAiqCamGroupManager.h
 *
 *  Copyright (c) 2021 Rockchip Corporation
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

#ifndef _RK_AIQ_CAM_GROUP_MANAGER_H_
#define _RK_AIQ_CAM_GROUP_MANAGER_H_

#include "rk_aiq_algo_camgroup_types.h"
#include "xcore/smartptr.h"
#include "xcore/safe_list.h"
#include "common/rk_aiq_pool.h"
#include "aiq_core/MessageBus.h"
#include "aiq_core/RkAiqCamgroupHandle.h"
#include "aiq_core/RkAiqCore.h"
#include "rk_aiq.h"

using namespace XCam;
namespace RkCam {

class RkAiqManager;
class RkAiqCore;

typedef struct rk_aiq_singlecam_result_s {
    rk_aiq_singlecam_3a_result_t _3aResults;
    SmartPtr<RkAiqFullParamsProxy> _fullIspParam;

    rk_aiq_singlecam_result_s () {
        memset(&_3aResults, 0, sizeof(_3aResults));
    }
    void reset() {
        XCamVideoBuffer* stats_buf = _3aResults.awb._awbStats;
        if (stats_buf)
            stats_buf->unref(stats_buf);

        stats_buf = _3aResults.aec._aecStats;
        if (stats_buf)
            stats_buf->unref(stats_buf);

        stats_buf = _3aResults.aec._aePreRes;
        if (stats_buf)
            stats_buf->unref(stats_buf);

#if (USE_NEWSTRUCT == 0)
        stats_buf = _3aResults.abayertnr._tnr_stats;
        if (stats_buf) stats_buf->unref(stats_buf);
#endif
        memset(&_3aResults, 0, sizeof(_3aResults));
        _fullIspParam = NULL;
    }
} rk_aiq_singlecam_result_t;

typedef struct rk_aiq_singlecam_result_status_s {
    rk_aiq_singlecam_result_t _singleCamResults;
    uint64_t _validAlgoResBits;
    uint64_t _validCoreMsgsBits;
    bool _ready;
    void reset() {
        _validAlgoResBits = 0;
        _validCoreMsgsBits = 0;
        _ready = false;
        _singleCamResults.reset();
    }
} rk_aiq_singlecam_result_status_t;

typedef struct rk_aiq_groupcam_result_s {
    rk_aiq_singlecam_result_status_t _singleCamResultsStatus[RK_AIQ_CAM_GROUP_MAX_CAMS];
    uint8_t _validCamResBits;
    uint32_t _frameId;
    bool _ready;
    uint32_t _refCnt;
    void reset() {
        _validCamResBits = 0;
        _ready = false;
        _frameId = -1;
        _refCnt = 0;
        for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++)
            _singleCamResultsStatus[i].reset();
    }
} rk_aiq_groupcam_result_t;

typedef struct rk_aiq_groupcam_sofsync_s {
    SmartPtr<VideoBuffer> _singleCamSofEvt[RK_AIQ_CAM_GROUP_MAX_CAMS];
    uint8_t _validCamSofSyncBits;
    rk_aiq_groupcam_sofsync_s() {
        _validCamSofSyncBits = 0;
        _refCnt              = 0;
    }
    uint32_t _refCnt;
    void reset() {
        _validCamSofSyncBits = 0;
        _refCnt = 0;
        for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++)
            _singleCamSofEvt[i] = NULL;
    }
} rk_aiq_groupcam_sofsync_t;

class RkAiqCamGroupManager;
class RkAiqCamGroupReprocTh
    : public Thread {
public:
    RkAiqCamGroupReprocTh(RkAiqCamGroupManager* camGroupManager)
        : Thread("RkAiqCamGroupReprocTh")
        , mCamGroupManager(camGroupManager) {};
    ~RkAiqCamGroupReprocTh() {
    };

    void triger_stop() {
        mMsgQueue.pause_pop ();
    };

    void triger_start() {
        mMsgQueue.clear ();
        mMsgQueue.resume_pop ();
    };
    typedef struct rk_aiq_groupcam_result_wrapper_s {
        rk_aiq_groupcam_result_wrapper_s(rk_aiq_groupcam_result_t* gc_result)
            :_gc_result(gc_result){};
        rk_aiq_groupcam_result_t* _gc_result;
    } rk_aiq_groupcam_result_wrapper_t;
    bool sendFrame(rk_aiq_groupcam_result_t* gc_result);

protected:
    //virtual bool started ();
    virtual void stopped () {
        mMsgQueue.clear ();
    };
    virtual bool loop ();
private:
    RkAiqCamGroupManager* mCamGroupManager;
    SafeList<rk_aiq_groupcam_result_wrapper_t>  mMsgQueue;
};

typedef std::shared_ptr<std::list<std::string>> ModuleNameList;
class RkAiqCamgroupHandle;
class RkAiqCamGroupManager
{
    friend class RkAiqCamGroupReprocTh;
    friend class RkAiqCamgroupHandle;
    /* normal processing */
    // add cam's AIQ ctx to cam group
    // receive group cam's awb,ae stats
    // receive group cam's awb,ae pre/proc results
    // receive group cam's genIspxxx results
    // running group algos, and replace the params
    // send all cam's params to hwi
    /* special for init params */
    // called only once
public:
    RkAiqCamGroupManager();
    virtual ~RkAiqCamGroupManager();

    // folowing called by single RkAiq
    void processAiqCoreMsgs(RkAiqCore* src, RkAiqCoreVdBufMsg& msg);
    void RelayAiqCoreResults(RkAiqCore* src, SmartPtr<RkAiqFullParamsProxy> &results);
    XCamReturn sofSync(RkAiqManager* aiqManager, SmartPtr<VideoBuffer>& sof_evt);

    XCamReturn setCamgroupCalib(CamCalibDbCamgroup_t* camgroup_calib);
    // rk_aiq_camgroup_ctx_t
    void setContainerCtx(void* group_ctx) {
        mGroupCtx = group_ctx;
    };
    void* getContainerCtx() {
        return mGroupCtx;
    };
    // called after single cam aiq init
    XCamReturn init();
    // called only once
    XCamReturn deInit();
    // start analyze thread
    XCamReturn start();
    // stop analyze thread
    XCamReturn stop();
    // called before start(), get initial settings
    XCamReturn prepare();
    // if called, prepare should be re-called
    XCamReturn bind(RkAiqManager* ctx);
    XCamReturn unbind(int camId);
    bool isRunningState() {
        return mState == CAMGROUP_MANAGER_STARTED;
    }
    XCamReturn addAlgo(RkAiqAlgoDesComm& algo);
    XCamReturn enableAlgo(int algoType, int id, bool enable);
    XCamReturn rmAlgo(int algoType, int id);
    bool getAxlibStatus(int algoType, int id);
    RkAiqAlgoContext* getEnabledAxlibCtx(const int algo_type);
    RkAiqAlgoContext* getAxlibCtx(const int algo_type, const int lib_id);
    RkAiqCamgroupHandle* getAiqCamgroupHandle(const int algo_type, const int lib_id);
    XCamReturn calibTuning(const CamCalibDbV2Context_t* aiqCalib, ModuleNameList& change_name_list);
    XCamReturn updateCalibDb(const CamCalibDbV2Context_t* newCalibDb);
    XCamReturn rePrepare();

    void setVicapReady(rk_aiq_hwevt_t* hwevt);
    bool isAllVicapReady();
protected:
    const struct RkAiqAlgoDesCommExt* mGroupAlgosDesArray;
    /* key: camId*/
    std::map<uint8_t, RkAiqManager*> mBindAiqsMap;
    /* key: frameId */
    std::map<uint32_t, rk_aiq_groupcam_result_t*> mCamGroupResMap;
    /* key: frameId */
    std::map<uint32_t, rk_aiq_groupcam_sofsync_t*> mCamGroupSofsyncMap;
    Mutex mCamGroupResMutex;
    SmartPtr<RkAiqCamGroupReprocTh> mCamGroupReprocTh;
    /* */
    Mutex mCamGroupApiSyncMutex;
    Mutex mSofMutex;
    uint64_t mRequiredMsgsMask;
    uint64_t mRequiredAlgoResMask;
    uint8_t mRequiredCamsResMask;
    uint8_t mVicapReadyMask;
    AlgoCtxInstanceCfgCamGroup mGroupAlgoCtxCfg;
    // mDefAlgoHandleList and mDefAlgoHandleMap only contain default handlers(id == 0),
    // default handlers will be treated as root handler, and custom handlers as children.
    // Custom handlers located in mAlgoHandleMaps and nexthdl of default handlers.
    // ordered algo list
    std::list<SmartPtr<RkAiqCamgroupHandle>> mDefAlgoHandleList;
    // key: algo type
    // for fast access
    std::map<int, SmartPtr<RkAiqCamgroupHandle>> mDefAlgoHandleMap;

    // key1: algo type
    // key2: algo id
    std::map<int, std::map<int, SmartPtr<RkAiqCamgroupHandle>>> mAlgoHandleMaps;
    // status transition
    /*   Typical transitions:
     *        CURRENT_STATE              NEXT_STATE                  OPERATION
     *   CAMGROUP_MANAGER_INVALID -> CAMGROUP_MANAGER_BINDED          bind
     *   CAMGROUP_MANAGER_BINDED-> CAMGROUP_MANAGER_INITED            init
     *   CAMGROUP_MANAGER_INITED -> CAMGROUP_MANAGER_PREPARED         prepare
     *   CAMGROUP_MANAGER_PREPARED -> CAMGROUP_MANAGER_STARTED        start
     *   CAMGROUP_MANAGER_STARTED -> CAMGROUP_MANAGER_PREPARED        stop
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
    };
    int mState;
    bool mInit;
    CamCalibDbCamgroup_t* mCamgroupCalib;
    uint32_t mClearedSofId;
    uint32_t mClearedResultId;

protected:
    XCamReturn reProcess(rk_aiq_groupcam_result_t* gc_res);
    rk_aiq_groupcam_result_t* getGroupCamResult(uint32_t frameId, bool query_ready = true);
    rk_aiq_groupcam_sofsync_t* getGroupCamSofsync(uint32_t frameId, bool query_ready = true);
    void setSingleCamStatusReady(rk_aiq_singlecam_result_status_t* status, rk_aiq_groupcam_result_t* gc_result);
    void relayToHwi(rk_aiq_groupcam_result_t* gc_res);
    void clearGroupCamResult(uint32_t frameId);
    void clearGroupCamResult_Locked(uint32_t frameId);
    void putGroupCamResult(rk_aiq_groupcam_result_t* gc_res);
    void clearGroupCamSofsync(uint32_t frameId);
    void clearGroupCamSofsync_Locked(uint32_t frameId);
    void putGroupCamSofsync(rk_aiq_groupcam_sofsync_t* syncSof);
    void addDefaultAlgos(const struct RkAiqAlgoDesCommExt* algoDes);
    virtual SmartPtr<RkAiqCamgroupHandle> newAlgoHandle(RkAiqAlgoDesComm* algo, int hw_ver);
    SmartPtr<RkAiqCamgroupHandle> getDefAlgoTypeHandle(int algo_type);
    XCamReturn syncSingleCamResultWithMaster(rk_aiq_groupcam_result_t* gc_res);
    std::map<int, SmartPtr<RkAiqCamgroupHandle>>* getAlgoTypeHandleMap(int algo_type);
    void calcHdrIso(RKAiqAecExpInfo_t* pCurExp, rk_aiq_singlecam_3a_result_t *singleCam3aRes);
    void* mGroupCtx;
private:
    CamCalibDbV2Context_t mCalibv2;
    bool needReprepare;
    XCam::Mutex _update_mutex;
    XCam::Cond _update_done_cond;
    std::atomic<bool> _sync_sof_running;
};

} //namespace

#endif
