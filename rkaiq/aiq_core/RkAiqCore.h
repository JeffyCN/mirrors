/*
 * Copyright (c) 2019-2021 Rockchip Eletronics Co., Ltd.
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
#ifndef _RK_AIQ_CORE_H_
#define _RK_AIQ_CORE_H_

#include <bitset>
#include <memory>

#include "ICamHw.h"
#include "MessageBus.h"
#include "RkAiqCalibDb.h"
#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbV2.h"
#include "RkAiqCalibDbV2Helper.h"
#include "RkAiqCamProfiles.h"
#include "RkAiqHandle.h"
#include "RkAiqResourceTranslator.h"
#include "common/panorama_stitchingApp.h"
#include "rk_aiq.h"
#include "rk_aiq_algo_types.h"
#include "rk_aiq_pool.h"
#include "rk_aiq_types.h"
#include "safe_list.h"
#include "smartptr.h"
#include "video_buffer.h"
#include "xcam_log.h"
#include "xcam_thread.h"

using namespace XCam;
namespace RkCam {

#define RKAIQCORE_CHECK_RET(ret, format, ...) \
    do { \
        if (ret < 0) { \
            LOGE_ANALYZER(format, ##__VA_ARGS__); \
            return ret; \
        } else if (ret == XCAM_RETURN_BYPASS) { \
            LOGW_ANALYZER(format, ##__VA_ARGS__); \
            return ret; \
        } \
    } while (0)

#define RKAIQCORE_CHECK_RET_NULL(ret, format, ...) \
    do { \
        if ((ret) < 0) { \
            LOGE_ANALYZER(format, ##__VA_ARGS__); \
            return NULL; \
        } else if ((ret) == XCAM_RETURN_BYPASS) { \
            LOGW_ANALYZER("bypass !", __FUNCTION__, __LINE__); \
        } \
    } while (0)

#define RKAIQCORE_CHECK_BYPASS(ret, format, ...) \
    do { \
        if ((ret) < 0) { \
            LOGE_ANALYZER(format, ##__VA_ARGS__); \
            return ret; \
        } else if ((ret) == XCAM_RETURN_BYPASS) { \
            LOGW_ANALYZER("bypass !", __FUNCTION__, __LINE__); \
        } \
    } while (0)

template <typename StandardMsg>
class RkAiqCoreMsg
    : public XCamMessage
{
public:
    RkAiqCoreMsg(XCamMessageType msg_id, uint32_t id, const SmartPtr<StandardMsg> &message)
        :  XCamMessage(msg_id, id)
        , msg(message)
    {};
    RkAiqCoreMsg() {};
    virtual ~RkAiqCoreMsg() {};

    SmartPtr<StandardMsg> msg;

private:
    //XCAM_DEAD_COPY (RkAiqCoreMsg);
};

typedef RkAiqCoreMsg<VideoBuffer>                       RkAiqCoreVdBufMsg;

class RkAiqCore;
class MessageThread;
class RkAiqAnalyzerGroup;
class RkAiqAnalyzeGroupManager;
class GlobalParamsManager;

class RkAiqAnalyzerCb {
public:
    explicit RkAiqAnalyzerCb() {};
    virtual ~RkAiqAnalyzerCb() {};
    virtual void rkAiqCalcDone(SmartPtr<RkAiqFullParamsProxy> &results) = 0;
    virtual void rkAiqCalcFailed(const char* msg) = 0;
private:
    XCAM_DEAD_COPY (RkAiqAnalyzerCb);
};

class RkAiqCoreThread
    : public Thread {
public:
    RkAiqCoreThread(RkAiqCore* rkAiqCore)
        : Thread("RkAiqCoreThread")
        , mRkAiqCore(rkAiqCore) {};
    ~RkAiqCoreThread() {
        mStatsQueue.clear ();
    };

    void triger_stop() {
        mStatsQueue.pause_pop ();
    };

    void triger_start() {
        mStatsQueue.clear ();
        mStatsQueue.resume_pop ();
    };

    bool push_stats (const SmartPtr<VideoBuffer> &buffer) {
        mStatsQueue.push (buffer);
        return true;
    };

protected:
    //virtual bool started ();
    virtual void stopped () {
        mStatsQueue.clear ();
    };
    virtual bool loop ();
private:
    RkAiqCore* mRkAiqCore;
    SafeList<VideoBuffer> mStatsQueue;
};

class RkAiqCoreEvtsThread
    : public Thread {
public:
    RkAiqCoreEvtsThread(RkAiqCore* rkAiqCore)
        : Thread("RkAiqCoreEvtsThread")
        , mRkAiqCore(rkAiqCore) {};
    ~RkAiqCoreEvtsThread() {
        mEvtsQueue.clear ();
    };

    void triger_stop() {
        mEvtsQueue.pause_pop ();
    };

    void triger_start() {
        mEvtsQueue.clear ();
        mEvtsQueue.resume_pop ();
    };

    bool push_evts (const SmartPtr<ispHwEvt_t> &buffer) {
        mEvtsQueue.push (buffer);
        return true;
    };

protected:
    //virtual bool started ();
    virtual void stopped () {
        mEvtsQueue.clear ();
    };
    virtual bool loop ();
private:
    RkAiqCore* mRkAiqCore;
    SafeList<ispHwEvt_t> mEvtsQueue;
};

struct RkAiqHwInfo {
    bool fl_supported;   // led flash
    bool fl_strth_adj;   // led streng_adjust
    bool irc_supported;  // ir flash & ir cutter
    bool fl_ir_strth_adj;   // ir streng_adjust
    bool lens_supported;
    bool is_multi_isp_mode;
    uint16_t multi_isp_extended_pixel;
    enum RK_PS_SrcOverlapPosition module_rotation;
};

#ifdef RKAIQ_ENABLE_CAMGROUP
class RkAiqCamGroupManager;
#endif

class RkAiqCore
    : public rk_aiq_share_ptr_ops_t
{
    friend class RkAiqCoreThread;
    friend class RkAiqCoreEvtsThread;
    friend class MessageThread;
    friend class RkAiqAnalyzeGroup;
    friend class RkAiqAnalyzeGroupManager;

#ifdef RKAIQ_ENABLE_CAMGROUP
    friend class RkAiqCamGroupManager;
#endif

public:
    RkAiqCore() = delete;
    explicit RkAiqCore(int isp_hw_ver);
    virtual ~RkAiqCore();

    bool setAnalyzeResultCb(RkAiqAnalyzerCb* callback) {
        mCb = callback;
        return true;
    }

#ifdef RKAIQ_ENABLE_CAMGROUP
    void setCamGroupManager(RkAiqCamGroupManager* cam_group_manager);
    const RkAiqCamGroupManager* GetCamGroupManager() { return mCamGroupCoreManager; }
#endif
    // called only once
    XCamReturn init(const char* sns_ent_name, const CamCalibDbContext_t* aiqCalib,
                    const CamCalibDbV2Context_t* aiqCalibv2 = nullptr);
    // called only once
    XCamReturn deInit();
    // start analyze thread
    XCamReturn start();
    // stop analyze thread
    XCamReturn stop();
    // called before start(), get initial settings
    virtual XCamReturn prepare(const rk_aiq_exposure_sensor_descriptor* sensor_des,
                               int mode);
    // should called befor prepare
    void notifyIspStreamMode(rk_isp_stream_mode_t mode) {
        if (mode == RK_ISP_STREAM_MODE_ONLNIE) {
            mIspOnline = true;
        } else if (mode == RK_ISP_STREAM_MODE_OFFLNIE) {
            mIspOnline = false;
        } else {
            mIspOnline = false;
        }
    }
    // could be called after prepare() or start()
    // if called between prepare() start(), can get
    // initial settings
    SmartPtr<RkAiqFullParamsProxy>& getAiqFullParams() {
        return mAiqCurParams;
    };
    XCamReturn pushStats(SmartPtr<VideoBuffer> &buffer);
    XCamReturn pushEvts(SmartPtr<ispHwEvt_t> &evts);
    XCamReturn addAlgo(RkAiqAlgoDesComm& algo);
    XCamReturn enableAlgo(int algoType, int id, bool enable);
    XCamReturn rmAlgo(int algoType, int id);
    bool getAxlibStatus(int algoType, int id);
    RkAiqAlgoContext* getEnabledAxlibCtx(const int algo_type);
    RkAiqAlgoContext* getAxlibCtx(const int algo_type, const int lib_id);
    RkAiqHandle* getAiqAlgoHandle(const int algo_type);
    XCamReturn get3AStatsFromCachedList(rk_aiq_isp_stats_t &stats);
    XCamReturn get3AStatsFromCachedList(rk_aiq_isp_stats_t **stats, int timeout_ms);
    void release3AStatsRef(rk_aiq_isp_stats_t *stats);
#if RKAIQ_HAVE_ASD_V10
    XCamReturn setCpsLtCfg(rk_aiq_cpsl_cfg_t &cfg);
    XCamReturn getCpsLtInfo(rk_aiq_cpsl_info_t &info);
    XCamReturn queryCpsLtCap(rk_aiq_cpsl_cap_t &cap);
#endif
    XCamReturn setHwInfos(struct RkAiqHwInfo &hw_info);
    XCamReturn setGrayMode(rk_aiq_gray_mode_t mode);
    rk_aiq_gray_mode_t getGrayMode();
    void setSensorFlip(bool mirror, bool flip);
    void setResrcPath(const char* rp) {
        if (mAlogsComSharedParams.resourcePath) {
            xcam_free((void*)(mAlogsComSharedParams.resourcePath));
            mAlogsComSharedParams.resourcePath = NULL;
        }
        if (rp)
            mAlogsComSharedParams.resourcePath = strdup(rp);
    };
    bool isRunningState() {
        return mState == RK_AIQ_CORE_STATE_RUNNING;
    }
    void setShareMemOps(isp_drv_share_mem_ops_t *mem_ops) {
        mShareMemOps = mem_ops;
    }
#ifdef RKAIQ_ENABLE_PARSER_V1
    XCamReturn setCalib(const CamCalibDbContext_t* aiqCalib);
#endif
    XCamReturn setCalib(const CamCalibDbV2Context_t* aiqCalib);
    XCamReturn events_analyze(const SmartPtr<ispHwEvt_t> &evts);
    XCamReturn calibTuning(const CamCalibDbV2Context_t* aiqCalib,
                           ModuleNameList& change_list);
    XCamReturn setMemsSensorIntf(const rk_aiq_mems_sensor_intf_t* intf);
    const rk_aiq_mems_sensor_intf_t* getMemsSensorIntf();
    XCamReturn set_sp_resolution(int &width, int &height, int &aligned_w, int &aligned_h);

    void setMulCamConc(bool cc) {
        mAlogsComSharedParams.is_multi_sensor = cc;
    };
    void setCamPhyId(int phyId) {
        mAlogsComSharedParams.mCamPhyId = phyId;
        mTranslator->setCamPhyId(phyId);
    }
    int getCamPhyId() {
        return mAlogsComSharedParams.mCamPhyId;
    }

#if RKAIQ_HAVE_PDAF
    XCamReturn set_pdaf_support(bool support);
    bool get_pdaf_support();
#endif
    void setGlobalParamsManager(GlobalParamsManager* globalParamsManger) {
       mGlobalParamsManger = globalParamsManger;
    }
    XCamReturn setAOVForAE(bool en);

    GlobalParamsManager* getGlobalParamsManager() {
       return mGlobalParamsManger;
    }
public:
    // following vars shared by all algo handlers
    typedef struct RkAiqAlgosComShared_s {
#ifdef RKAIQ_ENABLE_PARSER_V1
        const CamCalibDbContext_t* calib;
#endif
        const CamCalibDbV2Context_t* calibv2;
        rk_aiq_exposure_sensor_descriptor snsDes;
        int64_t sof;
        int working_mode;
        uint16_t multi_isp_extended_pixels;
        bool is_multi_isp_mode;
        bool fill_light_on;
        bool gray_mode;
        bool init;
        bool reConfig;
        bool is_bw_sensor;
        bool is_multi_sensor;
        uint32_t hardware_version;
        int iso;
        AlgoCtxInstanceCfg ctxCfigs[RK_AIQ_ALGO_TYPE_MAX];
        rk_aiq_cpsl_cfg_t cpslCfg;
        int conf_type;
        const char* resourcePath;
        bool sns_mirror;
        bool sns_flip;
        int spWidth;
        int spHeight;
        int spAlignedWidth;
        int spAlignedHeight;
        int mCamPhyId;
        uint8_t hdr_mode;

        void reset() {
            xcam_mem_clear(ctxCfigs);
            xcam_mem_clear(cpslCfg);
            xcam_mem_clear(snsDes);
#ifdef RKAIQ_ENABLE_PARSER_V1
            calib = NULL;
#endif
            calibv2 = NULL;
            sof = -1;
            working_mode = 0;
            is_multi_sensor = 0;
            init = false;
            reConfig = false;
            hardware_version = 0;
            iso = 0;
            fill_light_on = false;
            gray_mode = false;
            is_bw_sensor = false;
            resourcePath = NULL;
            sns_mirror = false;
            sns_flip = false;
            conf_type = RK_AIQ_ALGO_CONFTYPE_INIT;
            mCamPhyId = -1;
            multi_isp_extended_pixels = 0;
            is_multi_isp_mode = false;
            snsDes.otp_lsc = nullptr;
        }
    } RkAiqAlgosComShared_t;

    // following vars shared among itself by algo handlers
    typedef struct RkAiqAlgosGroupShared_s {
        int32_t groupId;
        uint32_t frameId;
        int64_t sof;
        int iso;
        XCamVideoBuffer* ispStats;
        RKAiqAecExpInfo_t preExp;
        RKAiqAecExpInfo_t curExp;
        RKAiqAecExpInfo_t nxtExp;
        rk_aiq_amd_params_t amdResParams;
        RkAiqAecStats* aecStatsBuf;
        RkAiqAwbStats* awbStatsBuf;
        RkAiqAfStats* afStatsBuf;
        RkAiqAdehazeStats* adehazeStatsBuf;
        RkAiqAgainStats* againStatsBuf;
        XCamVideoBuffer* sp;
        XCamVideoBuffer* ispGain;
        XCamVideoBuffer* kgGain;
        XCamVideoBuffer* wrGain;
        XCamVideoBuffer* tx;
        XCamVideoBuffer* orbStats;
        XCamVideoBuffer* nrImg;
        RkAiqPdafStats* pdafStatsBuf;
        RkAiqResComb res_comb;
        rk_aiq_scale_raw_info_t scaleRawInfo;
        RkAiqFullParams* fullParams;
        Mutex bay3dStatListMutex{false};
        std::list<SmartPtr<RkAiqBay3dStat>> bay3dStatList;
        void reset() {
            frameId = 0;
            sof     = 0;
            iso     = 0;
            xcam_mem_clear(res_comb);
            xcam_mem_clear(amdResParams);
            xcam_mem_clear(preExp);
            xcam_mem_clear(curExp);
            xcam_mem_clear(nxtExp);
            xcam_mem_clear(scaleRawInfo);
            bay3dStatList.clear();
            ispStats = nullptr;
            sp = nullptr;
            ispGain = nullptr;
            kgGain = nullptr;
            wrGain = nullptr;
            tx = nullptr;
            aecStatsBuf = nullptr;
            awbStatsBuf = nullptr;
            afStatsBuf = nullptr;
            adehazeStatsBuf = nullptr;
            orbStats = nullptr;
            nrImg       = nullptr;
            pdafStatsBuf = nullptr;
            fullParams = nullptr;
            againStatsBuf = nullptr;
        }
    } RkAiqAlgosGroupShared_t;
    RkAiqAlgosComShared_t mAlogsComSharedParams;
    // key: group type
    std::map<int32_t, RkAiqAlgosGroupShared_t *> mAlogsGroupSharedParamsMap;

    // key: algo type
    std::map<int32_t, uint64_t> mAlgoTypeToGrpMaskMap;

    // for handler access
    SmartPtr<RkAiqFullParamsProxy> mAiqCurParams;

    isp_drv_share_mem_ops_t *mShareMemOps;

    void post_message (RkAiqCoreVdBufMsg& msg);
    int32_t getGroupId(RkAiqAlgoType_t type);
    XCamReturn getGroupSharedParams(uint64_t groupId, RkAiqAlgosGroupShared_t* &shared);
    uint64_t getInitDisAlgosMask() {
        return mInitDisAlgosMask;
    }
    // TODO(Cody): Just AF use it, should it be public ?
    SmartPtr<RkAiqHandle>* getCurAlgoTypeHandle(int algo_type);
#if RKAIQ_HAVE_ASD_V10
    virtual XCamReturn genCpslResult(RkAiqFullParams* params, RkAiqAlgoPreResAsd* asd_pre_rk);
#endif

    XCamReturn updateCalib(enum rk_aiq_core_analyze_type_e type);
    XCamReturn updateCalibDbBrutal(CamCalibDbV2Context_t* aiqCalib);
    void setDelayCnts(int8_t delayCnts);
    void setVicapScaleFlag(bool mode);
    void setTbInfo(RkAiqTbInfo_t& info) {
        mTbInfo = info;
    }

    RkAiqTbInfo_t* getTbInfo(void) {
        return &mTbInfo;
    }

    void syncVicapScaleMode();
    IRkAiqResourceTranslator* getTranslator() {
        return mTranslator.ptr();
    }

    void awakenClean(uint32_t sequeence);
    XCamReturn setUserOtpInfo(rk_aiq_user_otp_info_t otp_info);

    bool isGroupAlgo(int algoType);
protected:
    // in analyzer thread
    XCamReturn analyze(const SmartPtr<VideoBuffer> &buffer);
    SmartPtr<RkAiqFullParamsProxy> analyzeInternal(enum rk_aiq_core_analyze_type_e type);
    XCamReturn freeSharebuf(uint64_t grpId);
    XCamReturn prepare(enum rk_aiq_core_analyze_type_e type);
    XCamReturn preProcess(enum rk_aiq_core_analyze_type_e type);
    XCamReturn processing(enum rk_aiq_core_analyze_type_e type);
    XCamReturn postProcess(enum rk_aiq_core_analyze_type_e type);
    std::map<int, SmartPtr<RkAiqHandle>>* getAlgoTypeHandleMap(int algo_type);
    void addDefaultAlgos(const struct RkAiqAlgoDesCommExt* algoDes);
    virtual SmartPtr<RkAiqHandle> newAlgoHandle(RkAiqAlgoDesComm* algo, int hw_ver, int handle_ver);
    virtual void copyIspStats(SmartPtr<RkAiqAecStatsProxy>& aecStat,
                              SmartPtr<RkAiqAwbStatsProxy>& awbStat,
                              SmartPtr<RkAiqAfStatsProxy>& afStat,
                              rk_aiq_isp_stats_t* to);
    void cacheIspStatsToList(SmartPtr<RkAiqAecStatsProxy>& aecStat,
                             SmartPtr<RkAiqAwbStatsProxy>& awbStat,
                             SmartPtr<RkAiqAfStatsProxy>& afStat);
#if RKAIQ_HAVE_ASD_V10
    void initCpsl();
#endif

protected:
    enum rk_aiq_core_state_e {
        RK_AIQ_CORE_STATE_INVALID,
        RK_AIQ_CORE_STATE_INITED,
        RK_AIQ_CORE_STATE_PREPARED,
        RK_AIQ_CORE_STATE_STARTED,
        RK_AIQ_CORE_STATE_RUNNING,
        RK_AIQ_CORE_STATE_STOPED,
    };
    SmartPtr<RkAiqCoreThread> mRkAiqCoreTh;
#if defined(ISP_HW_V20)
    SmartPtr<RkAiqCoreThread> mRkAiqCorePpTh;
#endif
    SmartPtr<RkAiqCoreEvtsThread> mRkAiqCoreEvtsTh;
    int mState;
    RkAiqAnalyzerCb* mCb;
    bool mHasPp;
    bool mIspOnline;
    bool mIsSingleThread;
    // key1: algo type
    // key2: algo id
    // contains default handlers and custom handlers
    std::map<int, map<int, SmartPtr<RkAiqHandle>>> mAlgoHandleMaps;
    // key: algo type
    // mCurAlgoHandleMaps and mCurIspAlgoHandleList only contain default handlers(id == 0),
    // default handlers will be treated as root handler, and custom handlers as children.
    // Custom handlers located in mAlgoHandleMaps and nexthdl of default handlers.
    std::map<int, SmartPtr<RkAiqHandle>> mCurAlgoHandleMaps;
    // ordered algo list
    std::list<SmartPtr<RkAiqHandle>> mCurIspAlgoHandleList;

    SmartPtr<RkAiqFullParamsPool> mAiqParamsPool;
    //SmartPtr<RkAiqFullParamsProxy> mAiqCurParams;
    SmartPtr<RkAiqExpParamsPool> mAiqExpParamsPool;
    SmartPtr<RkAiqIrisParamsPool> mAiqIrisParamsPool;
    SmartPtr<RkAiqFocusParamsPool> mAiqFocusParamsPool;
    SmartPtr<RkAiqCpslParamsPool> mAiqCpslParamsPool;
#if 1
    SmartPtr<RkAiqIspAecParamsPool>         mAiqIspAecParamsPool;
    SmartPtr<RkAiqIspHistParamsPool>        mAiqIspHistParamsPool;
    SmartPtr<RkAiqIspAwbParamsPool>         mAiqIspAwbParamsPool;
    SmartPtr<RkAiqIspAwbGainParamsPool>     mAiqIspAwbGainParamsPool;
    SmartPtr<RkAiqIspAfParamsPool>          mAiqIspAfParamsPool;
    SmartPtr<RkAiqIspDpccParamsPool>        mAiqIspDpccParamsPool;
    // SmartPtr<RkAiqIspHdrParamsPool>         mAiqIspHdrParamsPool;
    SmartPtr<RkAiqIspMergeParamsPool>       mAiqIspMergeParamsPool;
    SmartPtr<RkAiqIspTmoParamsPool>         mAiqIspTmoParamsPool;
    SmartPtr<RkAiqIspCcmParamsPool>         mAiqIspCcmParamsPool;
    SmartPtr<RkAiqIspLscParamsPool>         mAiqIspLscParamsPool;
    SmartPtr<RkAiqIspBlcParamsPool>         mAiqIspBlcParamsPool;
    SmartPtr<RkAiqIspRawnrParamsPool>       mAiqIspRawnrParamsPool;
    SmartPtr<RkAiqIspGicParamsPool>         mAiqIspGicParamsPool;
    SmartPtr<RkAiqIspDebayerParamsPool>     mAiqIspDebayerParamsPool;
    SmartPtr<RkAiqIspLdchParamsPool>        mAiqIspLdchParamsPool;
    SmartPtr<RkAiqIspLut3dParamsPool>       mAiqIspLut3dParamsPool;
    SmartPtr<RkAiqIspDehazeParamsPool>      mAiqIspDehazeParamsPool;
    SmartPtr<RkAiqIspAgammaParamsPool>      mAiqIspAgammaParamsPool;
    SmartPtr<RkAiqIspAdegammaParamsPool>    mAiqIspAdegammaParamsPool;
    SmartPtr<RkAiqIspWdrParamsPool>         mAiqIspWdrParamsPool;
    SmartPtr<RkAiqIspCsmParamsPool>         mAiqIspCsmParamsPool;
    SmartPtr<RkAiqIspCgcParamsPool>         mAiqIspCgcParamsPool;
    SmartPtr<RkAiqIspConv422ParamsPool>     mAiqIspConv422ParamsPool;
    SmartPtr<RkAiqIspYuvconvParamsPool>     mAiqIspYuvconvParamsPool;
    SmartPtr<RkAiqIspGainParamsPool>        mAiqIspGainParamsPool;
    SmartPtr<RkAiqIspCpParamsPool>          mAiqIspCpParamsPool;
    SmartPtr<RkAiqIspIeParamsPool>          mAiqIspIeParamsPool;
    SmartPtr<RkAiqIspMotionParamsPool>      mAiqIspMotionParamsPool;
    SmartPtr<RkAiqIspMdParamsPool>          mAiqIspMdParamsPool;

    SmartPtr<RkAiqIspTnrParamsPool>         mAiqIspTnrParamsPool;
    SmartPtr<RkAiqIspYnrParamsPool>         mAiqIspYnrParamsPool;
    SmartPtr<RkAiqIspUvnrParamsPool>        mAiqIspUvnrParamsPool;
    SmartPtr<RkAiqIspSharpenParamsPool>     mAiqIspSharpenParamsPool;
    SmartPtr<RkAiqIspEdgefltParamsPool>     mAiqIspEdgefltParamsPool;
    SmartPtr<RkAiqIspFecParamsPool>         mAiqIspFecParamsPool;
    SmartPtr<RkAiqIspOrbParamsPool>         mAiqIspOrbParamsPool;

    // TODO: change full params to list
    // V21 differential modules
    SmartPtr<RkAiqIspDrcParamsPool>         mAiqIspDrcParamsPool;
    SmartPtr<RkAiqIspBaynrParamsPool>       mAiqIspBaynrParamsPool;
    SmartPtr<RkAiqIspBa3dParamsPool>        mAiqIspBa3dParamsPool;
    SmartPtr<RkAiqIspCnrParamsPool>         mAiqIspCnrParamsPool;

    // V30
#if RKAIQ_HAVE_CAC
    SmartPtr<RkAiqIspCacParamsPool>         mAiqIspCacParamsPool;
#endif
    // V32
    SmartPtr<RkAiqIspAfdParamsPool>         mAiqIspAfdParamsPool;

    // V39
#if RKAIQ_HAVE_YUVME
    SmartPtr<RkAiqIspYuvmeParamsPool>    mAiqIspYuvmeParamsPool;
#endif
#if RKAIQ_HAVE_RGBIR_REMOSAIC
    SmartPtr<RkAiqIspRgbirParamsPool> mAiqIspRgbirParamsPool;
#endif
    // for new struct
#if USE_NEWSTRUCT
    SmartPtr<RkAiqIspDmParamsPool>  mAiqIspDmParamsPool;
    SmartPtr<RkAiqIspBtnrParamsPool>  mAiqIspBtnrParamsPool;
    SmartPtr<RkAiqIspGammaParamsPool>  mAiqIspGammaParamsPool;
    SmartPtr<RkAiqIspSharpParamsPool>  mAiqIspSharpParamsPool;
#endif

#endif
    static uint16_t DEFAULT_POOL_SIZE;
    XCam::Cond mIspStatsCond;
    Mutex ispStatsListMutex;
    struct RkAiqHwInfo mHwInfo;
    rk_aiq_cpsl_cap_t mCpslCap;
    bool mCurCpslOn;
    float mStrthLed;
    float mStrthIr;
    rk_aiq_gray_mode_t mGrayMode;
    bool firstStatsReceived;
    typedef SharedItemPool<rk_aiq_isp_stats_wrapper_t> RkAiqStatsPool;
    typedef SharedItemProxy<rk_aiq_isp_stats_wrapper_t> RkAiqStatsProxy;
    SmartPtr<RkAiqStatsPool> mAiqStatsPool;
    std::list<SmartPtr<RkAiqStatsProxy>> mAiqStatsCachedList;
    std::map<rk_aiq_isp_stats_t*, SmartPtr<RkAiqStatsProxy>> mAiqStatsOutMap;

    /* isp/ispp resource pool */
    SmartPtr<RkAiqSofInfoWrapperPool>           mAiqSofInfoWrapperPool;
    SmartPtr<RkAiqIspStatsIntPool>              mAiqIspStatsIntPool;
    SmartPtr<RkAiqAecStatsPool>                 mAiqAecStatsPool;
    SmartPtr<RkAiqAwbStatsPool>                 mAiqAwbStatsPool;
    SmartPtr<RkAiqAtmoStatsPool>                mAiqAtmoStatsPool;
    SmartPtr<RkAiqAdehazeStatsPool>             mAiqAdehazeStatsPool;
    SmartPtr<RkAiqAgainStatsPool>               mAiqAgainStatsPool;
    SmartPtr<RkAiqAfStatsPool>                  mAiqAfStatsPool;
    SmartPtr<RkAiqOrbStatsPool>                 mAiqOrbStatsIntPool;
#if RKAIQ_HAVE_PDAF
    SmartPtr<RkAiqPdafStatsPool>                mAiqPdafStatsPool;
#endif

    const struct RkAiqAlgoDesCommExt* mAlgosDesArray;
    Mutex mApiMutex;
    XCam::Cond mApiMutexCond;
    bool mSafeEnableAlgo;
    int mIspHwVer;
    const rk_aiq_mems_sensor_intf_t *mMemsSensorIntf;

    // messageBus: receive and distribute all kinds of messagess
    virtual XCamReturn handle_message (const SmartPtr<XCamMessage> &msg);

    // the group manager handle the messages from MessageBus
    XCamReturn newAiqGroupAnayzer();
    SmartPtr<RkAiqAnalyzeGroupManager> mRkAiqCoreGroupManager;

    XCamReturn thumbnailsGroupAnalyze(rkaiq_image_source_t &thumbnailsSrc);
    XCamReturn groupAnalyze(uint64_t grpId, const RkAiqAlgosGroupShared_t* shared);

    virtual void newAiqParamsPool();
#if RKAIQ_HAVE_PDAF
    void newPdafStatsPool();
    void delPdafStatsPool();
#endif
    XCamReturn getAiqParamsBuffer(RkAiqFullParams* aiqParams, int type, uint32_t frame_id);
    //void getThumbCfgForStreams();
    //void setThumbCfgFromStreams();
    void onThumbnailsResult(const rkaiq_thumbnails_t& thumbnail);

    XCamReturn handleIspStats(const SmartPtr<VideoBuffer>& buffer,
                              const SmartPtr<RkAiqAecStatsProxy>& aecStat,
                              const SmartPtr<RkAiqAwbStatsProxy>& awbStat,
                              const SmartPtr<RkAiqAfStatsProxy>& afStat,
                              const SmartPtr<RkAiqAtmoStatsProxy>& tmoStat,
                              const SmartPtr<RkAiqAdehazeStatsProxy>& dehazeStat);
    XCamReturn handleAecStats(const SmartPtr<VideoBuffer> &buffer, SmartPtr<RkAiqAecStatsProxy>& aecStat);
    XCamReturn handleAwbStats(const SmartPtr<VideoBuffer> &buffer, SmartPtr<RkAiqAwbStatsProxy>& awbStat);
    XCamReturn handleAfStats(const SmartPtr<VideoBuffer> &buffer, SmartPtr<RkAiqAfStatsProxy>& afStat);
    XCamReturn handleAtmoStats(const SmartPtr<VideoBuffer> &buffer, SmartPtr<RkAiqAtmoStatsProxy>& tmoStat);
    XCamReturn handleAdehazeStats(const SmartPtr<VideoBuffer> &buffer, SmartPtr<RkAiqAdehazeStatsProxy>& dehazeStat);
#if RK_GAIN_V2_ENABLE_GAIN2DDR
    XCamReturn handleAgainStats(const SmartPtr<VideoBuffer> &buffer, SmartPtr<RkAiqAgainStatsProxy>& gainStat);
#endif
    XCamReturn handleOrbStats(const SmartPtr<VideoBuffer> &buffer);
    XCamReturn handlePdafStats(const SmartPtr<VideoBuffer> &buffer);
    XCamReturn handleBay3dStats(const SmartPtr<VideoBuffer> &buffer);
    inline uint64_t grpId2GrpMask(uint32_t grpId) {
        return grpId == RK_AIQ_CORE_ANALYZE_ALL ? (uint64_t)grpId : (1ULL << grpId);
    }
    XCamReturn handleVicapScaleBufs(const SmartPtr<VideoBuffer> &buffer);

    std::bitset<RK_AIQ_ALGO_TYPE_MAX> getReqAlgoResMask(int algoType);
    void setReqAlgoResMask(int algoType, bool req);

    void getDummyAlgoRes(int type, uint32_t frame_id);

    SmartPtr<IRkAiqResourceTranslator> mTranslator;
    uint32_t mLastAnalyzedId;
#ifdef RKAIQ_ENABLE_CAMGROUP
    RkAiqCamGroupManager* mCamGroupCoreManager;
#endif
    std::bitset<RK_AIQ_ALGO_TYPE_MAX> mAllReqAlgoResMask {0};

    XCamReturn fixAiqParamsIsp(RkAiqFullParams* aiqParams);
    void ClearBay3dStatsList();

 private:
#if defined(RKAIQ_HAVE_THUMBNAILS)
    SmartPtr<ThumbnailsService> mThumbnailsService;
#endif
    int mSpWidth;
    int mSpHeight;
    int mSpAlignedWidth;
    int mSpAlignedHeight;
    uint64_t mInitDisAlgosMask;
    // update calib for each group
    XCam::Mutex _update_mutex;
    XCam::Cond _update_done_cond;
    XCamReturn notifyUpdate(uint64_t mask);
    XCamReturn waitUpdateDone();
    uint64_t groupUpdateMask;
    RkAiqTbInfo_t mTbInfo;

    bool mPdafSupport{false};
    int64_t mFrmInterval = 30000LL;
    int64_t mSofTime = 0LL;
    int64_t mAfStatsTime[2] {0};
    int64_t mPdafStatsTime[2] {0};
    uint32_t mAfStatsFrmId[2] {(uint32_t) -1, (uint32_t) -1};
    uint32_t mPdafStatsFrmId[2] {(uint32_t) -1, (uint32_t) -1};
    SmartPtr<RkAiqAfStatsProxy> mAfStats[2];
    SmartPtr<RkAiqPdafStatsProxy> mPdafStats[2];
    CamProfiles mProfiles;
    SmartPtr<RkAiqVicapRawBuf_t> mVicapBufs;
    bool mIsEnableVicap{false};
    int mScaleRatio{32};
    typedef struct {
        SmartPtr<RkAiqFullParamsProxy> proxy;
        uint64_t groupMasks;
        bool ready;
    } pending_params_t;
    // key: frame_id
    std::map<uint32_t, pending_params_t> mFullParamsPendingMap;
    uint64_t mFullParamReqGroupsMasks{0};
    XCam::Mutex _mFullParam_mutex;
    uint32_t mLatestParamsDoneId {0};
    uint32_t mLatestEvtsId {0};
    uint32_t mLatestStatsId {0};
    std::list<RkAiqAlgoType_t> mUpdateCalibAlgosList;
    void mapModStrListToEnum(ModuleNameList& change_name_list);
    rk_aiq_user_otp_info_t mUserOtpInfo;
    GlobalParamsManager* mGlobalParamsManger {NULL};
};

}

#endif //_RK_AIQ_CORE_H_
