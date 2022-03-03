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

#include "rk_aiq_types.h"
#include "rk_aiq_algo_types.h"
#include "RkAiqCalibDb.h"
#include "RkAiqCalibDbV2.h"
#include "RkAiqCalibDbV2Helper.h"
#include "RkAiqCalibDbTypes.h"
#include "RkAiqHandle.h"
#include "xcam_thread.h"
#include "smartptr.h"
#include "safe_list.h"
#include "xcam_log.h"
#include "video_buffer.h"
#include "rk_aiq_pool.h"
#include "ICamHw.h"
#include <memory>
#include "RkAiqResourceTranslator.h"
#include "MessageBus.h"
#include "common/panorama_stitchingApp.h"

using namespace XCam;
namespace RkCam {

#define RKAIQCORE_CHECK_RET(ret, format, ...) \
    do { \
        if (ret < 0) { \
            LOGE_ANALYZER(format, ##__VA_ARGS__); \
            return ret; \
        } else if (ret == XCAM_RETURN_BYPASS) { \
            LOGW_ANALYZER("bypass !", __FUNCTION__, __LINE__); \
            return ret; \
        } \
    } while (0)

#define RKAIQCORE_CHECK_RET_NULL(ret, format, ...) \
    do { \
        if (ret < 0) { \
            LOGE_ANALYZER(format, ##__VA_ARGS__); \
            return NULL; \
        } else if (ret == XCAM_RETURN_BYPASS) { \
            LOGW_ANALYZER("bypass !", __FUNCTION__, __LINE__); \
            return NULL; \
        } \
    } while (0)

#define RKAIQCORE_CHECK_BYPASS(ret, format, ...) \
    do { \
        if (ret < 0) { \
            LOGE_ANALYZER(format, ##__VA_ARGS__); \
            return ret; \
        } else if (ret == XCAM_RETURN_BYPASS) { \
            LOGW_ANALYZER("bypass !", __FUNCTION__, __LINE__); \
            ret = XCAM_RETURN_NO_ERROR; \
        } \
    } while (0)

#define grp_array_size(x) (sizeof(x)/sizeof(x[0]))
#define grp_conds_array_info(x) \
    x, \
    grp_array_size(x)

template <typename StandardMsg>
class RkAiqCoreMsg
    : public XCamMessage
{
public:
    RkAiqCoreMsg(XCamMessageType msg_id, uint32_t id, const SmartPtr<StandardMsg> &message)
        :  XCamMessage(msg_id, id)
           , msg(message)
    {};
    virtual ~RkAiqCoreMsg() {};

    SmartPtr<StandardMsg> msg;

private:
    XCAM_DEAD_COPY (RkAiqCoreMsg);
};

typedef RkAiqCoreMsg<VideoBuffer>                       RkAiqCoreVdBufMsg;

class RkAiqCore;
class MessageThread;
class RkAiqAnalyzerGroup;
class RkAiqAnalyzeGroupManager;

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

typedef struct RkAiqGrpCondition_s {
    enum XCamMessageType    cond;
    int8_t                  delay;
} RkAiqGrpCondition_t;

typedef struct RkAiqGrpConditions_s {
    RkAiqGrpCondition_t*    conds;
    u32                     size;
} RkAiqGrpConditions_t;

struct RkAiqAlgoDesCommExt {
    RkAiqAlgoDesComm* des;
    enum rk_aiq_core_analyze_type_e group;
    uint8_t algo_ver;
    uint8_t module_hw_ver;
    uint8_t handle_ver;
    RkAiqGrpConditions_t grpConds;
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
    void setCamGroupManager(RkAiqCamGroupManager* cam_group_manager) {
        mCamGroupCoreManager = cam_group_manager;
        if (mTranslator.ptr() && cam_group_manager)
            mTranslator->setGroupMode(true);
    }
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
    XCamReturn setCpsLtCfg(rk_aiq_cpsl_cfg_t &cfg);
    XCamReturn getCpsLtInfo(rk_aiq_cpsl_info_t &info);
    XCamReturn queryCpsLtCap(rk_aiq_cpsl_cap_t &cap);
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

    void setMulCamConc(bool cc) { mAlogsComSharedParams.is_multi_sensor = cc; };
    void setCamPhyId(int phyId) {
        mAlogsComSharedParams.mCamPhyId = phyId;
        mTranslator->setCamPhyId(phyId);
    }
    int getCamPhyId() { return mAlogsComSharedParams.mCamPhyId;}

    XCamReturn set_pdaf_support(bool support);
    bool get_pdaf_support();

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
        }
    } RkAiqAlgosComShared_t;

    // following vars shared among itself by algo handlers
    typedef struct RkAiqAlgosGroupShared_s {
        int32_t groupId;
        uint32_t frameId;
        int64_t sof;
        XCamVideoBuffer* ispStats;
        RKAiqAecExpInfo_t preExp;
        RKAiqAecExpInfo_t curExp;
        RKAiqAecExpInfo_t nxtExp;
        rk_aiq_amd_params_t amdResParams;
        XCamVideoBuffer* aecStatsBuf;
        XCamVideoBuffer* awbStatsBuf;
        XCamVideoBuffer* afStatsBuf;
        XCamVideoBuffer* sp;
        XCamVideoBuffer* ispGain;
        XCamVideoBuffer* kgGain;
        XCamVideoBuffer* wrGain;
        XCamVideoBuffer* tx;
        XCamVideoBuffer* orbStats;
        XCamVideoBuffer* nrImg;
        XCamVideoBuffer* pdafStatsBuf;
        RkAiqResComb res_comb;
        void reset() {
            frameId = -1;
            sof     = 0;
            xcam_mem_clear(res_comb);
            xcam_mem_clear(amdResParams);
            xcam_mem_clear(preExp);
            xcam_mem_clear(curExp);
            xcam_mem_clear(nxtExp);
            ispStats = nullptr;
            sp = nullptr;
            ispGain = nullptr;
            kgGain = nullptr;
            wrGain = nullptr;
            tx = nullptr;
            aecStatsBuf = nullptr;
            awbStatsBuf = nullptr;
            afStatsBuf = nullptr;
            orbStats = nullptr;
            nrImg       = nullptr;
            pdafStatsBuf = nullptr;
        }
    } RkAiqAlgosGroupShared_t;
    RkAiqAlgosComShared_t mAlogsComSharedParams;
    // key: group type
    std::map<int32_t, RkAiqAlgosGroupShared_t *> mAlogsGroupSharedParamsMap;

    // key: algo type
    std::map<int32_t, uint64_t> mAlgoTypeToGrpMaskMap;
    std::list<uint32_t> mAlogsGroupList;
    // std::map<int32_t, uint64_t> mGrpType2GrpMaskMap;

    isp_drv_share_mem_ops_t *mShareMemOps;

    void post_message (SmartPtr<XCamMessage> &msg);
    int32_t getGroupId(RkAiqAlgoType_t type);
    XCamReturn getGroupSharedParams(int32_t groupId, RkAiqAlgosGroupShared_t* &shared);
    uint64_t getCustomEnAlgosMask() {
        return mCustomEnAlgosMask;
    }
    // TODO(Cody): Just AF use it, should it be public ?
    SmartPtr<RkAiqHandle>* getCurAlgoTypeHandle(int algo_type);
    virtual XCamReturn genCpslResult(RkAiqFullParams* params, RkAiqAlgoPreResAsd* asd_pre_rk);

protected:
    // in analyzer thread
    XCamReturn analyze(const SmartPtr<VideoBuffer> &buffer);
    SmartPtr<RkAiqFullParamsProxy> analyzeInternal(enum rk_aiq_core_analyze_type_e type);
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
	virtual void setResultExpectedEffId(uint32_t& eff_id, enum RkAiqAlgoType_e type);
    void cacheIspStatsToList(SmartPtr<RkAiqAecStatsProxy>& aecStat,
                             SmartPtr<RkAiqAwbStatsProxy>& awbStat,
                             SmartPtr<RkAiqAfStatsProxy>& afStat);
    void initCpsl();

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
    SmartPtr<RkAiqCoreThread> mRkAiqCorePpTh;
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
    SmartPtr<RkAiqFullParamsProxy> mAiqCurParams;
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
    SmartPtr<RkAiqIspAwbParamsPoolV21>     mAiqIspAwbV21ParamsPool;
    SmartPtr<RkAiqIspDrcParamsPool>        mAiqIspDrcParamsPool;
    SmartPtr<RkAiqIspBlcParamsPoolV21>     mAiqIspBlcV21ParamsPool;
    SmartPtr<RkAiqIspBaynrParamsPoolV21>   mAiqIspBaynrV21ParamsPool;
    SmartPtr<RkAiqIspBa3dParamsPoolV21>    mAiqIspBa3dV21ParamsPool;
    SmartPtr<RkAiqIspYnrParamsPoolV21>     mAiqIspYnrV21ParamsPool;
    SmartPtr<RkAiqIspCnrParamsPoolV21>     mAiqIspCnrV21ParamsPool;
    SmartPtr<RkAiqIspSharpenParamsPoolV21> mAiqIspSharpenV21ParamsPool;

    // V30
    SmartPtr<RkAiqIspAwbParamsPoolV3x>         mAiqIspAwbV3xParamsPool;
    SmartPtr<RkAiqIspAfParamsPoolV3x>          mAiqIspAfV3xParamsPool;
    SmartPtr<RkAiqIspCacParamsPoolV3x>         mAiqIspCacV3xParamsPool;
    SmartPtr<RkAiqIspGainParamsPoolV3x>        mAiqIspGainV3xParamsPool;
    SmartPtr<RkAiqIspBaynrParamsPoolV3x>       mAiqIspBaynrV3xParamsPool;
    SmartPtr<RkAiqIspBa3dParamsPoolV3x>        mAiqIspBa3dV3xParamsPool;
    SmartPtr<RkAiqIspYnrParamsPoolV3x>         mAiqIspYnrV3xParamsPool;
    SmartPtr<RkAiqIspCnrParamsPoolV3x>         mAiqIspCnrV3xParamsPool;
    SmartPtr<RkAiqIspSharpenParamsPoolV3x>     mAiqIspSharpenV3xParamsPool;
    SmartPtr<RkAiqIspTnrParamsPoolV3x>         mAiqIspTnrV3xParamsPool;

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
    SmartPtr<RkAiqAfStatsPool>                  mAiqAfStatsPool;
    SmartPtr<RkAiqOrbStatsPool>                 mAiqOrbStatsIntPool;
    SmartPtr<RkAiqPdafStatsPool>                mAiqPdafStatsPool;

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
    void newPdafStatsPool();
    void delPdafStatsPool();
    virtual XCamReturn getAiqParamsBuffer(RkAiqFullParams* aiqParams, enum rk_aiq_core_analyze_type_e type);
    virtual XCamReturn genIspParamsResult(RkAiqFullParams *aiqParams, enum rk_aiq_core_analyze_type_e type);
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
    XCamReturn handleOrbStats(const SmartPtr<VideoBuffer> &buffer);
    XCamReturn handlePdafStats(const SmartPtr<VideoBuffer> &buffer);
    inline uint64_t grpId2GrpMask(uint32_t grpId) {
        return grpId == RK_AIQ_CORE_ANALYZE_ALL ? (uint64_t)grpId : (1ULL << grpId);
    }

    virtual void setReqAlgoResMask(int algoType, bool req);

    SmartPtr<IRkAiqResourceTranslator> mTranslator;
    uint32_t mLastAnalyzedId;
#ifdef RKAIQ_ENABLE_CAMGROUP
    RkAiqCamGroupManager* mCamGroupCoreManager;
#endif
    uint64_t mAllReqAlgoResMask;

private:
    SmartPtr<ThumbnailsService> mThumbnailsService;
    int mSpWidth;
    int mSpHeight;
    int mSpAlignedWidth;
    int mSpAlignedHeight;
    uint64_t mCustomEnAlgosMask;
    bool mPdafSupport;
};

};

#endif //_RK_AIQ_CORE_H_
