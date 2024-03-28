/*
 *  Copyright (c) 2019 Rockchip Corporation
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

#ifndef _RK_AIQ_MANAGER_H_
#define _RK_AIQ_MANAGER_H_

#include "ICamHw.h"
#include "RkAiqCore.h"
#include "RkAiqCalibDb.h"
#include "RkAiqCalibDbV2.h"
#include "RkLumaCore.h"
#include "rk_aiq.h"
#include "RkAiqGlobalParamsManager.h"
#include <memory>
#include <list>

namespace RkCam {

class RkAiqManager;

class RkAiqMngCmdThread
    : public Thread {
public:
    RkAiqMngCmdThread(RkAiqManager* aiqMng)
        : Thread("RkAiqMngCmdThread")
        , mAiqMng(aiqMng) {};
    ~RkAiqMngCmdThread() {};

    void triger_stop() {
        mAiqCmdQueue.pause_pop ();
    };

    void triger_start() {
        mAiqCmdQueue.resume_pop ();
    };

    enum MSG_CMD {
        MSG_CMD_SW_WORKING_MODE
    };

    typedef struct message_s {
        int cmd;
        union {
            struct {
                rk_aiq_working_mode_t mode;
            } sw_wk_mode;
        } data;
        bool sync;
        SmartPtr<Mutex>             mutex;
        SmartPtr<XCam::Cond>        cond;
    } msg_t;

    bool send_cmd(SmartPtr<msg_t> msg) {
        bool ret = true;
        if (msg->sync) {
            msg->mutex = new Mutex();
            msg->cond = new XCam::Cond();
            SmartLock lock (*msg->mutex.ptr());
            ret = mAiqCmdQueue.push(msg);
            msg->cond->wait(*msg->mutex.ptr());
        } else {
            ret = mAiqCmdQueue.push(msg);
        }

        return ret;
    };

protected:
    virtual void stopped () {
        mAiqCmdQueue.clear ();
    };

    virtual bool loop ();

private:
    RkAiqManager* mAiqMng;
    SafeList<msg_t>  mAiqCmdQueue;
};

class RkAiqRstApplyThread
    : public Thread {
public:
    RkAiqRstApplyThread(RkAiqManager* aiqMng)
        : Thread("RkAiqRstApplyThread")
        , mAiqMng(aiqMng) {};
    ~RkAiqRstApplyThread() {};

    void triger_stop() {
        mAiqRstQueue.pause_pop ();
    };

    void triger_start() {
        mAiqRstQueue.clear ();
        mAiqRstQueue.resume_pop ();
    };

    bool push_results (SmartPtr<RkAiqFullParamsProxy> aiqRst) {
        return mAiqRstQueue.push(aiqRst);
    };

protected:
    virtual void stopped () {
        mAiqRstQueue.clear ();
    };

    virtual bool loop ();

private:
    RkAiqManager* mAiqMng;
    SafeList<RkAiqFullParamsProxy>  mAiqRstQueue;
};

class RkAiqManager
    :/* public IspStatsListener
    , public IsppStatsListener
    , public IspLumaListener
    , public IspEvtsListener
    ,*/ public HwResListener
    , public RkAiqAnalyzerCb
    , public RkLumaAnalyzerCb {
    friend RkAiqRstApplyThread;
    friend RkAiqMngCmdThread;
#ifdef RKAIQ_ENABLE_CAMGROUP
    friend class RkAiqCamGroupManager;
#endif
public:
    explicit RkAiqManager(const char* sns_ent_name,
                          rk_aiq_error_cb err_cb,
                          rk_aiq_metas_cb metas_cb);
    virtual ~RkAiqManager();
    void setHwEvtCb(rk_aiq_hwevt_cb hwevt_cb, void* evt_cb_ctx) {
        mHwEvtCbCtx = evt_cb_ctx;
        mHwEvtCb = hwevt_cb;
    };
    void setAiispCb(rk_aiq_aiispCtx_t aiispCtx) {
        mAiispCtx = aiispCtx;
    };
    void setTbInfo(RkAiqTbInfo_t& info) {
        mTbInfo = info;
    }
    void setCamHw(SmartPtr<ICamHw>& camhw);
    void setCamPhyId(int phyId) {mCamPhyId = phyId;}
    int getCamPhyId() { return mCamPhyId;}
    void setAnalyzer(SmartPtr<RkAiqCore> analyzer);
#ifdef RKAIQ_ENABLE_PARSER_V1
    void setAiqCalibDb(const CamCalibDbContext_t* calibDb);
#endif
    void setAiqCalibDb(const CamCalibDbV2Context_t* calibDb);
    void unsetTuningCalibDb(bool isNeedFreeCalib);
#if defined(ISP_HW_V20)
    void setLumaAnalyzer(SmartPtr<RkLumaCore> analyzer);
#endif
    XCamReturn init();
    XCamReturn prepare(uint32_t width, uint32_t height, rk_aiq_working_mode_t mode);
    XCamReturn start();
    XCamReturn stop(bool keep_ext_hw_st = false);
    XCamReturn deInit();
    XCamReturn updateCalibDb(const CamCalibDbV2Context_t* newCalibDb);
    #if 0
    // from IsppStatsListener
    XCamReturn isppStatsCb(SmartPtr<VideoBuffer>& isppStats);
    // from IspLumaListener
    XCamReturn ispLumaCb(SmartPtr<VideoBuffer>& ispLuma);
    // from IspStatsListener
    XCamReturn ispStatsCb(SmartPtr<VideoBuffer>& ispStats);
    // from IspEvtsListener
    XCamReturn ispEvtsCb(ispHwEvt_t* evt);
    #endif
    XCamReturn hwResCb(SmartPtr<VideoBuffer>& hwres);
    XCamReturn syncSofEvt(SmartPtr<VideoBuffer>& hwres);
    // from RkAiqAnalyzerCb
    void rkAiqCalcDone(SmartPtr<RkAiqFullParamsProxy>& results);
    void rkAiqCalcFailed(const char* msg);
    // from RkLumaAnalyzerCb
    void rkLumaCalcDone(rk_aiq_luma_params_t luma_params);
    void rkLumaCalcFailed(const char* msg);
    XCamReturn setModuleCtl(rk_aiq_module_id_t mId, bool mod_en);
    XCamReturn getModuleCtl(rk_aiq_module_id_t mId, bool& mod_en);
    XCamReturn enqueueRawBuffer(void *rawdata, bool sync);
    XCamReturn enqueueRawFile(const char *path);
    XCamReturn registRawdataCb(void (*callback)(void *));
    XCamReturn rawdataPrepare(rk_aiq_raw_prop_t prop);
    XCamReturn setSharpFbcRotation(rk_aiq_rotation_t rot);
    XCamReturn setMirrorFlip(bool mirror, bool flip, int skip_frm_cnt);
    XCamReturn getMirrorFlip(bool& mirror, bool& flip);
    void setDefMirrorFlip();
    XCamReturn swWorkingModeDyn_msg(rk_aiq_working_mode_t mode);
    void setMulCamConc(bool cc);
    CamCalibDbV2Context_t* getCurrentCalibDBV2(void);
    XCamReturn calibTuning(CamCalibDbV2Context_t* aiqCalib,
                           ModuleNameList& change_list);
    XCamReturn setVicapStreamMode(int on, bool isSingleMode);
#ifdef RKAIQ_ENABLE_CAMGROUP
    void setCamGroupManager(RkAiqCamGroupManager* cam_group_manager, bool isMain) {
        mCamGroupCoreManager = cam_group_manager;
        mIsMain = isMain;
    }
#endif
    rk_aiq_working_mode_t getWorkingMode() {
        return mWorkingMode;
    }

    GlobalParamsManager* getGlobalParamsManager() {
        return mGlobalParamsManager.ptr();
    }
    uint32_t sensor_output_width;
    uint32_t sensor_output_height;
    // post aiisp status
    bool ainr_status;
protected:
    XCamReturn applyAnalyzerResult(SmartPtr<RkAiqFullParamsProxy>& results, bool ignoreIsUpdate = false);
    XCamReturn swWorkingModeDyn(rk_aiq_working_mode_t mode);
private:
    enum aiq_state_e {
        AIQ_STATE_INVALID,
        AIQ_STATE_INITED,
        AIQ_STATE_PREPARED,
        AIQ_STATE_STARTED,
        AIQ_STATE_STOPED,
    };
    XCAM_DEAD_COPY (RkAiqManager);
private:
    SmartPtr<ICamHw> mCamHw;
    SmartPtr<RkAiqCore> mRkAiqAnalyzer;
    SmartPtr<RkAiqRstApplyThread> mAiqRstAppTh;
    SmartPtr<RkAiqMngCmdThread> mAiqMngCmdTh;
#if defined(ISP_HW_V20)
    SmartPtr<RkLumaCore> mRkLumaAnalyzer;
#endif
    rk_aiq_error_cb mErrCb;
    rk_aiq_metas_cb mMetasCb;
    rk_aiq_hwevt_cb mHwEvtCb;
    rk_aiq_aiispCtx_t mAiispCtx;

    void* mHwEvtCbCtx;
    const char* mSnsEntName;
    RkAiqTbInfo_t mTbInfo;
#ifdef RKAIQ_ENABLE_PARSER_V1
    const CamCalibDbContext_t* mCalibDb;
#endif
    CamCalibDbV2Context_t* mCalibDbV2;
    CamCalibDbV2Context_t* tuningCalib;
    bool mNeedFreeCalib;
    rk_aiq_working_mode_t mWorkingMode;
    rk_aiq_working_mode_t mOldWkModeForGray;
    bool mWkSwitching;
    uint32_t mWidth;
    uint32_t mHeight;
    int _state;
    bool mCurMirror;
    bool mCurFlip;
    SmartPtr<RkAiqCpslParamsProxy> mDleayCpslParams;
    int mDelayCpslApplyFrmNum;
    int mCamPhyId;
#ifdef RKAIQ_ENABLE_CAMGROUP
    RkAiqCamGroupManager* mCamGroupCoreManager;
#endif
    bool mIsMain;
    int mTBStatsCnt {0};
    uint32_t mLastAweekId{(uint32_t)-1};
    SmartPtr<GlobalParamsManager> mGlobalParamsManager;
};

} //namespace RkCam

#endif //_RK_AIQ_MANAGER_H_
