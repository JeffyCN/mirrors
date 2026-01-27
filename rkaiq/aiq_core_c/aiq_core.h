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

#ifndef _AIQ_CORE_C_H_
#define _AIQ_CORE_C_H_

#include "common/rk_aiq_comm.h"
#include "common/panorama_stitchingApp.h"
#include "common/rk_aiq_types_priv_c.h"
#if RKAIQ_HAVE_DUMPSYS
#include "common/aiq_notifier.h"
#endif
#include "algos/rk_aiq_algo_des.h"
#include "c_base/aiq_list.h"
#include "c_base/aiq_pool.h"
#include "aiq_core_c/aiq_stats_translator.h"
#include "aiq_algogroups_manager.h"
#include "RkAiqGlobalParamsManager_c.h"
#include "hwi_c/aiq_camHw.h"
#include "aiq_core_c/AiqCoreConfig.h"
#include "aiq_core_c/aiq_coreMsg.h"
#include "algos/algo_types_priv.h"
#include "iq_parser_v2/RkAiqCalibDbV2.h"
#include "RkAiqCamGroupManager_c.h"
#include "uAPI2/rk_aiq_user_api2_stats.h"

RKAIQ_BEGIN_DECLARE

#define RKAIQCORE_CHECK_RET(ret, format, ...)     \
    do {                                          \
        if (ret < 0) {                            \
            LOGE_ANALYZER(format, ##__VA_ARGS__); \
            return ret;                           \
        } else if (ret == XCAM_RETURN_BYPASS) {   \
            LOGW_ANALYZER(format, ##__VA_ARGS__); \
            return ret;                           \
        }                                         \
    } while (0)

#define RKAIQCORE_CHECK_RET_NULL(ret, format, ...)             \
    do {                                                       \
        if ((ret) < 0) {                                       \
            LOGE_ANALYZER(format, ##__VA_ARGS__);              \
            return NULL;                                       \
        } else if ((ret) == XCAM_RETURN_BYPASS) {              \
            LOGW_ANALYZER("bypass !", __FUNCTION__, __LINE__); \
        }                                                      \
    } while (0)

#define RKAIQCORE_CHECK_BYPASS(ret, format, ...)               \
    do {                                                       \
        if ((ret) < 0) {                                       \
            LOGE_ANALYZER(format, ##__VA_ARGS__);              \
            return ret;                                        \
        } else if ((ret) == XCAM_RETURN_BYPASS) {              \
            LOGW_ANALYZER("bypass !", __FUNCTION__, __LINE__); \
        }                                                      \
    } while (0)

typedef struct AiqAnalyzerCb_s {
    void* pCtx;
    void (*rkAiqCalcDone)(void* pCtx, AiqFullParams_t* results);
    void (*rkAiqCalcExpDone)(void* pCtx, aiq_params_base_t* results);
    void (*rkAiqCalcFailed)(void* pCtx, const char* msg);
} AiqAnalyzerCb_t;

typedef struct AiqHwInfo_s {
    bool fl_supported;     // led flash
    bool fl_strth_adj;     // led streng_adjust
    bool irc_supported;    // ir flash & ir cutter
    bool fl_ir_strth_adj;  // ir streng_adjust
    bool lens_supported;
    bool is_multi_isp_mode;
    uint16_t multi_isp_extended_pixel;
    enum RK_PS_SrcOverlapPosition module_rotation;
} AiqHwInfo_t;

typedef struct RkAiqAlgosComShared_s {
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
    bool is_aibnr_autorun;
    bool is_aibnr_force_update;
    uint32_t hardware_version;
    int iso;
    AlgoCtxInstanceCfg ctxCfigs[RK_AIQ_ALGO_TYPE_MAX];
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
    int aibnr_fixIndex;
    rk_aiq_face_roi_t selfaceinfo;
} RkAiqAlgosComShared_t;

// following vars shared among itself by algo handlers
typedef struct RkAiqAlgosGroupShared_s {
    int32_t groupId;
    uint32_t frameId;
    int64_t sof;
    int iso;
    RKAiqAecExpInfo_t preExp;
    RKAiqAecExpInfo_t curExp;
    RKAiqAecExpInfo_t nxtExp;
    rk_aiq_amd_params_t amdResParams;
    // aiq_ae_stats_wrapper_t*
    aiq_stats_base_t* aecStatsBuf;
    // aiq_awb_stats_wrapper_t*
    aiq_stats_base_t* awbStatsBuf;
    // aiq_af_stats_wrapper_t*
    aiq_stats_base_t* afStatsBuf;
    // rkisp_adehaze_stats_t*
    aiq_stats_base_t* adehazeStatsBuf;
    // rk_aiq_again_stat_t*
    aiq_stats_base_t* againStatsBuf;
    XCamVideoBuffer* sp;
    XCamVideoBuffer* ispGain;
    XCamVideoBuffer* tx;
    XCamVideoBuffer* nrImg;
    // rk_aiq_isp_pdaf_stats_t*
    aiq_stats_base_t* pdafStatsBuf;
    RkAiqResComb res_comb;
    rk_aiq_scale_raw_info_t scaleRawInfo;
    // AiqFullParams_t*
    AiqFullParams_t* fullParams;
    AmtdProcResult_t amtdRes;
} RkAiqAlgosGroupShared_t;

enum rk_aiq_core_state_e {
    RK_AIQ_CORE_STATE_INVALID,
    RK_AIQ_CORE_STATE_INITED,
    RK_AIQ_CORE_STATE_PREPARED,
    RK_AIQ_CORE_STATE_STARTED,
    RK_AIQ_CORE_STATE_RUNNING,
    RK_AIQ_CORE_STATE_STOPED,
};

typedef struct {
    AiqFullParams_t* fullParams;
    uint64_t groupMasks;
    bool ready;
} pending_params_t;

typedef struct RkAiqVicapRawBufInfo_s {
    uint32_t frame_id;
    bool ready;
    uint8_t flags;
    aiq_VideoBuffer_t* raw_s;
    aiq_VideoBuffer_t* raw_m;
    aiq_VideoBuffer_t* raw_l;
} RkAiqVicapRawBufInfo_t;

#if RKAIQ_HAVE_DUMPSYS
typedef struct AiqCorePoolNofreeBufCnt_s {
    uint32_t fullParams;
    uint32_t algosParams[RESULT_TYPE_MAX_PARAM];

    uint32_t sofInfo;

    uint32_t aeStats;
    uint32_t awbStats;
    uint32_t btnrStats;
    uint32_t dhazStats;
    uint32_t gainStats;

    uint32_t aePreRes;
    uint32_t aeProcRes;
    uint32_t awbProcRes;
    uint32_t blcProcRes;
    uint32_t ynrProcRes;
} AiqCorePoolNofreeBufCnt_t;

typedef struct AiqCoreIsStatsAvail_s {
    bool ae;
    bool awb;
    bool af;
    bool btnr;
    bool dhaze;
    bool gain;
} AiqCoreIsStatsAvail_t;
#endif

typedef struct AiqCore_s {
    RkAiqAlgosComShared_t mAlogsComSharedParams;
    AiqAnalyzeGroupManager_t mRkAiqCoreGroupManager;
    RkAiqAlgosGroupShared_t* mAlogsGroupSharedParamsMap[RK_AIQ_CORE_ANALYZE_MAX];
    AiqAlgoHandler_t* mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_MAX];
    // aiq_stats_base_t + aiq_ae_stats_wrapper_t
    AiqPool_t* mAiqAecStatsPool;
    // aiq_stats_base_t + aiq_awb_stats_wrapper_t
    AiqPool_t* mAiqAwbStatsPool;
    // aiq_stats_base_t + rkisp_adehaze_stats_t
    AiqPool_t* mAiqAdehazeStatsPool;
    // aiq_stats_base_t + rk_aiq_again_stat_t
    AiqPool_t* mAiqAgainStatsPool;
    // aiq_stats_base_t + aiq_af_stats_wrapper_t
    AiqPool_t* mAiqAfStatsPool;
#if RKAIQ_HAVE_PDAF
    // aiq_stats_base_t +rk_aiq_isp_pdaf_stats_t
    AiqPool_t* mAiqPdafStatsPool;
#endif
    AiqMutex_t bay3dStatListMutex;
    // rk_aiq_bay3d_stat_t*
    AiqList_t* bay3dStatList;
    // aiq_stats_base_t + rk_aiq_bay3d_stat_t
    AiqPool_t* mbay3dStatPool;
    // AiqFullParams_t
    AiqPool_t* mAiqFullParamsPool;
    AiqPool_t* mAiqParamsPoolArray[RESULT_TYPE_MAX_PARAM];

    // AlgoRstShared_t + RkAiqAlgoPreResAe
    AiqPool_t* mPreResAeSharedPool;
    // AlgoRstShared_t + RkAiqAlgoProcResAwbShared_t
    AiqPool_t* mProcResAwbSharedPool;
    // AlgoRstShared_t + RkAiqAlgoProcResAblc
    AiqPool_t* mProcResBlcSharedPool;
    // AlgoRstShared_t + RkAiqAlgoProcResYnr
    AiqPool_t* mProcResYnrSharedPool;
    // AlgoRstShared_t + AiqSofInfoWrapper_t
    AiqPool_t* mAiqSofInfoWrapperPool;
    // key: frame_id, val: pending_params_t
    AiqMap_t* mFullParamsPendingMap;
    // RkAiqAlgoType_t
    AiqList_t* mUpdateCalibAlgosList;

    AiqMutex_t _update_mutex;
    AiqCond_t _update_done_cond;
    AiqMutex_t mIspStatsMutex;
    AiqCond_t mIspStatsCond;
    AiqMutex_t mApiMutex;
    AiqCond_t mApiMutexCond;
    AiqMutex_t _mFullParam_mutex;
    AiqMutex_t mPdafStatsMutex;

    // for handler access
    AiqFullParams_t* mAiqCurParams;

    isp_drv_share_mem_ops_t* mShareMemOps;
    int mState;
    AiqAnalyzerCb_t* mCb;
    bool mIspOnline;
    bool mIsSingleThread;

    AiqHwInfo_t mHwInfo;
    rk_aiq_gray_mode_t mGrayMode;
    bool firstStatsReceived;

    const struct RkAiqAlgoDesCommExt* mAlgosDesArray;
    bool mSafeEnableAlgo;
    int mIspHwVer;
    const rk_aiq_mems_sensor_intf_t* mMemsSensorIntf;
    AiqStatsTranslator_t* mTranslator;
    uint32_t mLastAnalyzedId;
#ifdef RKAIQ_ENABLE_CAMGROUP
    AiqCamGroupManager_t* mCamGroupCoreManager;
#endif
    uint64_t mAllReqAlgoResMask;
    int mSpWidth;
    int mSpHeight;
    int mSpAlignedWidth;
    int mSpAlignedHeight;
    uint64_t mInitDisAlgosMask;
    // update calib for each group
    uint64_t groupUpdateMask;
    rk_aiq_tb_info_t mTbInfo;

    bool mPdafSupport;
    PdafSensorType_t mPdafType;
    int64_t mFrmInterval;
    int64_t mSofTime;
    int64_t mAfStatsTime[2];
    int64_t mPdafStatsTime[2];
    uint32_t mAfStatsFrmId[2];
    uint32_t mPdafStatsFrmId[2];
    aiq_stats_base_t* mAfStats[2];
    aiq_stats_base_t* mPdafStats[2];
    // aiq_ae_stats_wrapper_t
    aiq_stats_base_t* mCurAeStats;
    // aiq_awb_stats_wrapper_t
    aiq_stats_base_t* mCurAwbStats;
    // aiq_af_stats_wrapper_t
    aiq_stats_base_t* mCurAfStats;
    RkAiqVicapRawBufInfo_t* mVicapBufs;
    bool mIsEnableVicap;
    int mScaleRatio;
    uint64_t mFullParamReqGroupsMasks;
    uint32_t mLatestParamsDoneId;
    uint32_t mLatestEvtsId;
    uint32_t mLatestStatsId;
    rk_aiq_user_otp_info_t mUserOtpInfo;
    GlobalParamsManager_t* mGlobalParamsManger;

    bool mIsAovMode;
    int  mDefaultDelayCnt;

#if RKAIQ_HAVE_DUMPSYS
    int (*dump_algos)(void* self, st_string* result, int argc, void* argv[]);
    int (*dump_core)(void* self, st_string* result, int argc, void* argv[]);

    struct aiq_notifier notifier;
    struct aiq_notifier_subscriber sub_core;
    struct aiq_notifier_subscriber sub_buf_mgr;
    struct aiq_notifier_subscriber sub_grp_analyzer;
    struct aiq_notifier_subscriber sub_isp_params;
    struct aiq_notifier_subscriber sub_isp_mods[_MODS_COUNT];

    AiqCorePoolNofreeBufCnt_t mNoFreeBufCnt;
    AiqCoreIsStatsAvail_t mIsStatsAvail;

    uint32_t mForceDoneCnt;
    uint32_t mForceDoneId;
    uint32_t mSofLoss;
    uint32_t mStatsLoss;
#endif
} AiqCore_t;

XCamReturn AiqCore_init(AiqCore_t* pAiqCore, const char* sns_ent_name,
                        const CamCalibDbV2Context_t* aiqCalibv2);
XCamReturn AiqCore_deinit(AiqCore_t* pAiqCore);
XCamReturn AiqCore_start(AiqCore_t* pAiqCore);
XCamReturn AiqCore_stop(AiqCore_t* pAiqCore);
// called before start(), get initial settings
XCamReturn AiqCore_prepare(AiqCore_t* pAiqCore, const rk_aiq_exposure_sensor_descriptor* sensor_des,
                           int mode);
void AiqCore_clean(AiqCore_t* pAiqCore);
// should called befor prepare
void AiqCore_notifyIspStreamMode(AiqCore_t* pAiqCore, rk_isp_stream_mode_t mode);

#define AiqCore_setAnalyzeResultCb(pAiqCore, callback) \
        pAiqCore->mCb = callback;

#ifdef RKAIQ_ENABLE_CAMGROUP
void AiqCore_setCamGroupManager(AiqCore_t* pAiqCore, AiqCamGroupManager_t* cam_group_manager);
#endif

#define AiqCore_getAiqFullParams(pAiqCore) \
    pAiqCore->mAiqCurParams

#define AiqCore_setGlobalParamsManager(pAiqCore, pGlobalParamsManger) \
        pAiqCore->mGlobalParamsManger = pGlobalParamsManger

#define AiqCore_getGlobalParamsManager(pAiqCore) \
        pAiqCore->mGlobalParamsManger

XCamReturn AiqCore_pushStats(AiqCore_t* pAiqCore, AiqHwEvt_t* evt);
XCamReturn AiqCore_pushEvts(AiqCore_t* pAiqCore, AiqHwEvt_t* evts);
XCamReturn AiqCore_addAlgo(AiqCore_t* pAiqCore, RkAiqAlgoDesComm* algo);
XCamReturn AiqCore_enableAlgo(AiqCore_t* pAiqCore, int algoType, int id, bool enable);
XCamReturn AiqCore_rmAlgo(AiqCore_t* pAiqCore, int algoType, int id);
bool AiqCore_getAxlibStatus(AiqCore_t* pAiqCore, int algoType, int id);
RkAiqAlgoContext* AiqCore_getEnabledAxlibCtx(AiqCore_t* pAiqCore, const int algo_type);
RkAiqAlgoContext* AiqCore_getAxlibCtx(AiqCore_t* pAiqCore, const int algo_type, const int lib_id);
/*
 * timeout: -1 next, 0 current, > 0 wait next until timeout
 */
XCamReturn AiqCore_get3AStats(AiqCore_t* pAiqCore, rk_aiq_isp_statistics_t* stats, int timeout_ms);

XCamReturn AiqCore_groupAnalyze(AiqCore_t * pAiqCore, uint64_t grpId,
                                const RkAiqAlgosGroupShared_t* shared);

XCamReturn AiqCore_setHwInfos(AiqCore_t* pAiqCore, AiqHwInfo_t* hw_info);
XCamReturn AiqCore_setGrayMode(AiqCore_t* pAiqCore, rk_aiq_gray_mode_t mode);
rk_aiq_gray_mode_t AiqCore_getGrayMode(AiqCore_t* pAiqCore);
void AiqCore_setSensorFlip(AiqCore_t* pAiqCore, bool mirror, bool flip);
void AiqCore_setResrcPath(AiqCore_t* pAiqCore, const char* rp);

#define AiqCore_isRunningState(pAiqCore) \
    (pAiqCore->mState == RK_AIQ_CORE_STATE_RUNNING)

void AiqCore_setShareMemOps(AiqCore_t* pAiqCore, isp_drv_share_mem_ops_t* mem_ops);
XCamReturn AiqCore_setCalib(AiqCore_t* pAiqCore, const CamCalibDbV2Context_t* aiqCalib);
XCamReturn AiqCore_calibTuning(AiqCore_t* pAiqCore, const CamCalibDbV2Context_t* aiqCalib,
                               TuningCalib* change_list);
XCamReturn AiqCore_setMemsSensorIntf(AiqCore_t* pAiqCore, const rk_aiq_mems_sensor_intf_t* intf);
const rk_aiq_mems_sensor_intf_t* AiqCore_getMemsSensorIntf(AiqCore_t* pAiqCore);
XCamReturn AiqCore_set_sp_resolution(AiqCore_t* pAiqCore, int* width, int* height, int* aligned_w,
                                     int* aligned_h);

#define AiqCore_setMulCamConc(pAiqCore, cc) \
    pAiqCore->mAlogsComSharedParams.is_multi_sensor = cc;

#define AiqCore_setCamPhyId(pAiqCore, phyId) \
        pAiqCore->mAlogsComSharedParams.mCamPhyId = phyId;

#define AiqCore_getCamPhyId(pAiqCore) \
        pAiqCore->mAlogsComSharedParams.mCamPhyId

#if RKAIQ_HAVE_PDAF
XCamReturn AiqCore_set_pdaf_support(AiqCore_t* pAiqCore, bool support);
bool AiqCore_get_pdaf_support(AiqCore_t* pAiqCore);
XCamReturn AiqCore_set_pdaf_type(AiqCore_t* pAiqCore, PdafSensorType_t type);
PdafSensorType_t AiqCore_get_pdaf_type(AiqCore_t* pAiqCore);
#endif

XCamReturn AiqCore_post_message(AiqCore_t* pAiqCore, AiqCoreMsg_t* msg);
int32_t AiqCore_getGroupId(AiqCore_t* pAiqCore, RkAiqAlgoType_t type);
XCamReturn AiqCore_getGroupSharedParams(AiqCore_t* pAiqCore, uint64_t groupId,
                                        RkAiqAlgosGroupShared_t* shared);

#define AiqCore_getInitDisAlgosMask(pAiqCore) \
    pAiqCore->mInitDisAlgosMask

XCamReturn AiqCore_updateCalibDbBrutal(AiqCore_t* pAiqCore, CamCalibDbV2Context_t* aiqCalib);
XCamReturn AiqCore_updateCalib(AiqCore_t* pAiqCore, enum rk_aiq_core_analyze_type_e type);
void AiqCore_setDelayCnts(AiqCore_t* pAiqCore, int8_t delayCnts);
void AiqCore_setVicapScaleFlag(AiqCore_t* pAiqCore, bool mode);
void AiqCore_setTbInfo(AiqCore_t* pAiqCore, rk_aiq_tb_info_t* info);
rk_aiq_tb_info_t* AiqCore_getTbInfo(AiqCore_t* pAiqCore);
void AiqCore_syncVicapScaleMode(AiqCore_t* pAiqCore);
AiqStatsTranslator_t* AiqCore_getTranslator(AiqCore_t* pAiqCore);
void AiqCore_awakenClean(AiqCore_t* pAiqCore, uint32_t sequeence);
XCamReturn AiqCore_setUserOtpInfo(AiqCore_t* pAiqCore, rk_aiq_user_otp_info_t otp_info);
bool AiqCore_isGroupAlgo(AiqCore_t* pAiqCore, int algoType);
XCamReturn AiqCore_register3Aalgo(AiqCore_t* pAiqCore, void* algoDes, void* cbs);
XCamReturn AiqCore_unregister3Aalgo(AiqCore_t* pAiqCore, int algoType);
XCamReturn AiqCore_setTranslaterIspUniteMode(AiqCore_t* pAiqCore, RkAiqIspUniteMode mode);
XCamReturn AiqCore_reinitForNewStatsDelay(AiqCore_t* pAiqCore, int delayCnt);

#define AiqCore_setAovMode(pAiqCore, mode) \
        pAiqCore->mIsAovMode = mode;

RKAIQ_END_DECLARE

#endif
