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
#ifndef _AIQ_CAMHW_BASE_H_
#define _AIQ_CAMHW_BASE_H_

#include "algos/aiisp/rk_aiisp.h"
#include "c_base/aiq_cond.h"
#include "common/aiq_notifier.h"
#include "hwi_c/aiq_aiIspLoader.h"
#include "hwi_c/aiq_ispParamsSplitter.h"
#include "hwi_c/aiq_sensorHw.h"
#include "hwi_c/aiq_spStreamProcUnit.h"
#include "hwi_c/aiq_stream.h"
#include "hwi_c/aiq_AiRmsStreamProcUnit.h"
#include "include/iq_parser_v2/aec_head.h"
#include "include/iq_parser_v2/af_head.h"
#include "include/iq_parser_v2/sensorinfo_head.h"
#include "xcore_c/aiq_v4l2_buffer.h"
#include "xcore_c/aiq_v4l2_device.h"
#include "hwi_c/rawReprocess/aiq_fpnSwStreamProcUnit.h"
#include "hwi_c/rawReprocess/RkAiqMemcManager.h"

XCAM_BEGIN_DECLARE

extern bool g_mIsMultiIspMode;

#define ISP20HW_SUBM (0x1)

#define MAX_PARAMS_QUEUE_SIZE       5
#define CAM_INDEX_FOR_1608          8

typedef struct AiqCifSclStream_s AiqCifSclStream_t;
typedef struct AiqIspParamsCvt_s AiqIspParamsCvt_t;
typedef struct AiqMap_s AiqMap_t;
typedef struct AiqList_s AiqList_t;
typedef struct AiqPool_s AiqPool_t;
typedef struct AiqPdafStreamProcUnit_s AiqPdafStreamProcUnit_t;
typedef struct AiqRawStreamCapUnit_s AiqRawStreamCapUnit_t;
typedef struct AiqRawStreamProcUnit_s AiqRawStreamProcUnit_t;
typedef struct AiqManager_s AiqManager_t;
#if RKAIQ_HAVE_AIBNR
typedef struct AibnrManager_s AibnrManager_t;
#endif
#if RKAIQ_HAVE_AIYNR
typedef struct AiynrManager_s AiynrManager_t;
#endif
typedef struct AiqBtnrDsReproc_s AiqBtnrDsReproc_t;

enum cam_hw_state_e {
    CAM_HW_STATE_INVALID,
    CAM_HW_STATE_INITED,
    CAM_HW_STATE_PREPARED,
    CAM_HW_STATE_STARTED,
    CAM_HW_STATE_PAUSED,
    CAM_HW_STATE_STOPPED,
};
enum ircut_state_e {
    IRCUT_STATE_CLOSED, /* close ir-cut,meaning that infrared ray can be received */
    IRCUT_STATE_CLOSING,
    IRCUT_STATE_OPENING,
    IRCUT_STATE_OPENED, /* open ir-cut,meaning that only visible light can be received */
};

typedef struct calib_infos_s {
    struct {
        bool enable;
        bool motion_detect_en;
    } mfnr;

    struct {
        CalibDb_IrisTypeV2_t IrisType;
    } aec;

    struct {
        CalibDbV2_Af_VcmCfg_t vcmcfg;
        CalibDbV2_Af_LdgParam_t ldg_param;
        CalibDbV2_Af_HighLightParam_t highlight;
    } af;

    struct {
        CalibDb_DcgSetV2_t CISDcgSet;
        CalibDb_ExpUpdate_CombV2_t CISExpUpdate;
    } sensor;
} calib_infos_t;

enum mipi_stream_idx {
    MIPI_STREAM_IDX_0   = 1,
    MIPI_STREAM_IDX_1   = 2,
    MIPI_STREAM_IDX_2   = 4,
    MIPI_STREAM_IDX_ALL = 7,
};

typedef struct drv_share_mem_ctx_s {
    void* ops_ctx;
    void* mem_info;
    rk_aiq_drv_share_mem_type_t type;
} drv_share_mem_ctx_t;

enum ISP_STREAM_STATUS_E {
    ISP_STREAM_STATUS_INVALID,
    ISP_STREAM_STATUS_STREAM_ON,
    ISP_STREAM_STATUS_STREAM_OFF,
};

// FIXME: Move it from another "aiq_core/thumbnails.h"
typedef enum rkaiq_stream_type_e {
    RKISP_STREAM_NONE,
    RKISP20_STREAM_MIPITX_S,
    RKISP20_STREAM_MIPITX_M,
    RKISP20_STREAM_MIPITX_L,
    RKISP20_STREAM_SP_RAW,
    RKISP20_STREAM_SP,
    RKISP20_STREAM_TNR,
    RKISP20_STREAM_NR,
    RKISP20_STREAM_FEC,
    // RKISP21_SP,
    RKISP_STREAM_MAX,
} rkaiq_stream_type_t;

typedef struct AiqDevBufCntsInfo_s {
    char _devName[64];
    int _bufCnts;
} AiqDevBufCntsInfo_t;

// FIXME: share 1608 data ptr(aiq/rawdata)
typedef struct sensor_info_share_s {
    AiqRawStreamProcUnit_t* raw_proc_unit[CAM_INDEX_FOR_1608];  // bind rx by camId
    aiq_autoptr_t* raw_cap_unit;                                // save 1st tx obj addr
    char reference_name[64];                                    // save vicap name(for 1608)
    rk_aiq_cif_info_t* reference_mipi_cif;                      // vicap inf (for 1608)
    // us: union_stream
    int us_open_cnt;     // for hwi open(1608)
    int us_prepare_cnt;  // for rawCap buffer manage(1608).
    int us_stream_cnt;   // mark cnt. on: ++, off: --
    int us_stop_cnt;     // last sensor stop
    // tracking opened sensor num
    int en_sns_num;                    // Record the number of open sensors
    int first_en[CAM_INDEX_FOR_1608];  // set/get fmt for 1608 sensor
} sensor_info_share_t;

typedef struct SnsFullInfoWraps_s {
    char key[32];
    rk_sensor_full_info_t data;
    struct SnsFullInfoWraps_s* next;
} SnsFullInfoWraps_t;

typedef struct exgain_s {
    uint8_t exgain_bypass;
    uint8_t local_gain_bypass;
    uint8_t gain_bypass_en;
    bool gain_module_en;
} exgain_t;

typedef struct AiqCamHwBase_s {
    AiqSensorHw_t* _mSensorDev;
    AiqIspParamsCvt_t* _mIspParamsCvt;
    AiqV4l2SubDevice_t* mIspCoreDev;
    AiqV4l2Device_t* mIspStatsDev;
    AiqV4l2Device_t* mIspParamsDev;
    AiqLensHw_t* mLensDev;
    AiqV4l2SubDevice_t* mIrcutDev;
    AiqV4l2SubDevice_t* _cif_csi2_sd;
    AiqV4l2Device_t* mIspSpDev;
    AiqV4l2SubDevice_t* mVicapItfDev;
    AiqV4l2Device_t* mAiIspDev;
    AiqV4l2SubDevice_t* mAiIspSubDev;
    AiqV4l2SubDevice_t* mAiIspCoreDev;
    AiqHwResListener_t _hwResListener;
    calib_infos_t _cur_calib_infos;
    int _hdr_mode;
    AiqMutex_t _isp_params_cfg_mutex;
    int _state;
    volatile bool _is_exit;
    bool _linked_to_isp;
    bool _linked_to_1608;
    bool _linked_to_serdes;
    bool _linked_to_multi_isp;
    bool _airms_en;
    bool _aiynr_en;
    char sns_name[32];
    uint64_t _isp_module_ens;
    exgain_t exgain_status;

    bool mNoReadBack;
    rk_aiq_rotation_t _sharp_fbc_rotation;

    rk_aiq_ldch_share_mem_info_t ldch_mem_info_array[2 * ISP2X_MESH_BUF_NUM];
    rk_aiq_ldcv_share_mem_info_t ldcv_mem_info_array[2 * ISP2X_MESH_BUF_NUM];
    rk_aiq_cac_share_mem_info_t cac_mem_info_array[2 * ISP3X_MESH_BUF_NUM];
    rk_aiq_btnr_share_mem_info_t btnr_mem_info_array[2 * ISP2X_MESH_BUF_NUM];
    rk_aiq_dbg_share_mem_info_t dbg_mem_info_array[2 * RKISP_INFO2DDR_BUF_MAX];
    drv_share_mem_ctx_t _ldch_drv_mem_ctx;
    drv_share_mem_ctx_t _ldcv_drv_mem_ctx;
    drv_share_mem_ctx_t _cac_drv_mem_ctx;
    drv_share_mem_ctx_t _btnr_drv_mem_ctx;
    drv_share_mem_ctx_t _dbg_drv_mem_ctx;
    isp_drv_share_mem_ops_t _drv_share_mem_ops;
    AiqMutex_t _mem_mutex;
    rk_aiq_rect_t _crop_rect;
    uint32_t _ds_width;
    uint32_t _ds_heigth;
    uint32_t _ds_width_align;
    uint32_t _ds_heigth_align;
    uint32_t _exp_delay;
    rk_aiq_lens_descriptor _lens_des;
    // isp
    AiqStatsStream_t* mIspStatsStream;
    AiqStream_t* mIspParamStream;
    AiqSofEvtStream_t* mIspSofStream;
#if defined(RKAIQ_ENABLE_SPSTREAM)
    AiqSPStreamProcUnit_t* mSpStreamUnit;
#endif
    AiqStreamEventPollThread_t* mIspStremEvtTh;
    AiqRawStreamCapUnit_t* mRawCapUnit;
    AiqRawStreamProcUnit_t* mRawProcUnit;
    AiqAiRmsStreamProcUnit_t* mAiRmsProcUnit;
    AiqCifSclStream_t* mCifScaleStream;
    AiqPdafStreamProcUnit_t* mPdafStreamUnit;
    AiqMap_t* _effecting_ispparam_map;
    AiqPool_t* mEffectIspParamsPool;
    // <char*, int>
    AiqDevBufCntsInfo_t* mDevBufCntMap;
    int mDevBufCntMap_size;
    rk_aiq_control_preinit_t mAiqPreCtrlInfo;
    bool use_rkrawstream;
    int mWorkingMode;
    const CamCalibDbV2Context_t* mCalibDbV2;
    bool mKpHwSt;
    int mCamPhyId;
    bool mIsGroupMode;
    bool mIsMain;
    // if phsical link between vicap and isp, only isp3x support now
    bool mVicapIspPhyLinkSupported;
    AiqIspParamsSplitter_t* mParamsSplitter;
    int _isp_stream_status;
    rk_sensor_pdaf_info_t mPdafInfo;
    AiqMutex_t _stop_cond_mutex;
    // TODO: Sync(1608 sensor) sdk hwEvt cb
    AiqCond_t _sync_done_cond;
    uint32_t _curIspParamsSeq;
    bool _not_skip_first;
    uint32_t mAweekId;
    void* _skipped_params;
    void* _first_awb_param;
    void* _first_awb_cfg;
    bool mIsListenStrmEvt;
    rk_aiq_tb_info_t mTbInfo;
    bool mIsOnlineByWorkingMode;
    AiqPollCallback_t mPollCb;
    bool mIsFake;
    AiqManager_t* rkAiqManager;
    // bay3d fuf
    struct {
        struct rkisp_bnr_buf_info _bay3dbuf;
        void* iir_address[RKISP_BUFFER_MAX];
        void* gain_address[RKISP_BUFFER_MAX];
        void* aiisp_address[RKISP_BUFFER_MAX];
        void* wgt_address[RKISP_BUFFER_MAX];
        bool _init;
    } _bnrDrvBuf;
    XCamReturn (*prepare)(AiqCamHwBase_t* pCamHw, uint32_t width, uint32_t height, int mode,
                          int t_delay, int g_delay);
    // internal override
    void (*updateEffParams)(AiqCamHwBase_t* pCamHw, void* params, void* ori_params);
    bool (*processTb)(AiqCamHwBase_t* pCamHw, void* params);
    XCamReturn (*saveInfotoFileTb)(AiqCamHwBase_t* pCamHw);
    XCamReturn (*getLastEffectParamTb)(AiqCamHwBase_t* pCamHw);

    // AIISP
    bool use_aiisp;
    AiqAiIspStream_t* mIspAiispStream;
    struct rkisp_aiisp_cfg mAiisp_cfg;
    rk_aiisp_param* aiisp_param;
    struct AiispLibrary lib_aiisp_;
    XCamReturn (*setAiispMode)(AiqCamHwBase_t* pCamHw, struct rkisp_aiisp_cfg* aiisp_cfg);
    XCamReturn (*read_aiisp_result)(AiqCamHwBase_t* pCamHw, rk_aiq_aiisp_t* aiisp_evt);
    XCamReturn(*aiisp_processing)(AiqCamHwBase_t* pCamHw, rk_aiq_aiisp_t* aiisp_evt);

    int mDefaultDelayCnt;

    // fpn
    AiqFpnSwStreamProcUnit_t* mFpnStreamUnit;
    bool mFpnReproc;

    // memc
    AiqMemcManager_t *mMemcStream;
    bool mMemcEn;
    AiqMemcIirStream_t* mIspMemcIirStream;

    // btnr2
    AiqBtnrDsReproc_t* mBtnrDsReproc;

#if RKAIQ_HAVE_AIBNR
    bool use_ainn;
    RkAiqAibnrModelInfo_t mAibnrModelInfo;
    AiqAibnrIspStream_t*   mAibnrIspStream;
    AiqAibnrAiispStream_t* mAibnrAiispStream;
    struct rkisp_aiisp_cfg mAibnr_cfg;
    struct rkaiisp_param_info mAibnrParamInfo;
    AibnrManager_t* mAibnrManager;
    bool mAibnrBufPoolInit;
    rk_aiq_aibnr_buffer_count_t mAibnrBufCnt;
    XCamReturn (*aibnr_init)(AiqCamHwBase_t* pCamHw, AibnrManager_t* pAibnrManager, struct rkisp_aiisp_cfg *aibnr_cfg);
    XCamReturn (*aibnr_prepare)(AiqCamHwBase_t* pCamHw);
    XCamReturn (*aibnr_start)(AiqCamHwBase_t* pCamHw);
    XCamReturn (*aibnr_stop)(AiqCamHwBase_t* pCamHw);
    XCamReturn (*aibnr_deinit)(AiqCamHwBase_t* pCamHw);
    XCamReturn (*aibnr_getParamsBuf)(AiqCamHwBase_t* pCamHw, AiqV4l2Buffer_t** pV4l2Buf);
    XCamReturn (*aibnr_updateParams)(AiqCamHwBase_t* pCamHw, AiqV4l2Buffer_t* pV4l2Buf);
    XCamReturn (*aibnr_doNrnn)(AiqCamHwBase_t* pCamHw, struct rkisp_aiisp_st *isp_out);
    XCamReturn (*aibnr_doIspBe)(AiqCamHwBase_t* pCamHw, struct rkisp_aiisp_st *isp_in);
    XCamReturn (*aibnr_getIspBufInf)(AiqCamHwBase_t* pCamHw, struct rkisp_bnr_buf_info *bnr_buf);
    XCamReturn (*aibnr_setMemMode)(AiqCamHwBase_t* pCamHw, enum rkaiisp_mem_mode *memMode);
    XCamReturn (*aibnr_setIspBufInf)(AiqCamHwBase_t* pCamHw, struct rkaiisp_ispbuf_info *ispbuf_info);
    XCamReturn (*aibnr_setParamInf)(AiqCamHwBase_t* pCamHw, struct rkaiisp_param_info *param_info);
    XCamReturn (*aibnr_setModelInf)(AiqCamHwBase_t* pCamHw, RkAiqAibnrModelInfo_t *pAibnrModelInfo);
    XCamReturn (*aibnr_setLinecnt)(AiqCamHwBase_t* pCamHw, struct rkisp_aiisp_cfg *aibnr_cfg);
    XCamReturn (*aibnr_clrIqParam)(AiqCamHwBase_t* pCamHw);

    XCamReturn (*btnrWgtReproc_updateEn)(AiqCamHwBase_t* pCamHw, bool en);
#endif

// AIRMS
    char airms_model_file[AIRMS_MODEL_FILENAME_LEN];

#if RKAIQ_HAVE_AIYNR
    RkAiqAiynrModelInfo_t mAiynrModelInfo;
    AiqAiynrIspStream_t*   mAiynrIspStream;
    AiqAiynrAiispStream_t* mAiynrAiispStream;
    struct rkisp_aiisp_cfg mAiynr_cfg;
    struct rkaiisp_param_info mAiynrParamInfo;
    AiynrManager_t* mAiynrManager;
    bool mAiynrBufPoolInit;
    XCamReturn (*aiynr_init)(AiqCamHwBase_t* pCamHw, AiynrManager_t* pAiynrManager, struct rkisp_aiisp_cfg *aiynr_cfg);
    XCamReturn (*aiynr_prepare)(AiqCamHwBase_t* pCamHw);
    XCamReturn (*aiynr_start)(AiqCamHwBase_t* pCamHw);
    XCamReturn (*aiynr_stop)(AiqCamHwBase_t* pCamHw);
    XCamReturn (*aiynr_deinit)(AiqCamHwBase_t* pCamHw);
    XCamReturn (*aiynr_getParamsBuf)(AiqCamHwBase_t* pCamHw, AiqV4l2Buffer_t** pV4l2Buf);
    XCamReturn (*aiynr_updateParams)(AiqCamHwBase_t* pCamHw, AiqV4l2Buffer_t* pV4l2Buf);
    XCamReturn (*aiynr_doNrnn)(AiqCamHwBase_t* pCamHw, struct rkisp_aiisp_st *isp_out);
    XCamReturn (*aiynr_doIspBe)(AiqCamHwBase_t* pCamHw, struct rkisp_aiisp_st *isp_in);
    //XCamReturn (*aiynr_getIspBufInf)(AiqCamHwBase_t* pCamHw, struct rkisp_ynr_buf_info *ynr_buf);
    XCamReturn (*aiynr_setIspBufInf)(AiqCamHwBase_t* pCamHw, struct rkaiisp_ispbuf_info *ispbuf_info);
    XCamReturn (*aiynr_setParamInf)(AiqCamHwBase_t* pCamHw, struct rkaiisp_param_info *param_info);
    XCamReturn (*aiynr_setModelInf)(AiqCamHwBase_t* pCamHw, RkAiqAiynrModelInfo_t *pAiynrModelInfo);
    XCamReturn (*aiynr_setLinecnt)(AiqCamHwBase_t* pCamHw, struct rkisp_aiisp_cfg *aiynr_cfg);
    XCamReturn (*aiynr_getYnrbufInf)(AiqCamHwBase_t* pCamHw, struct rkaiisp_ynrbuf_info *ynrbuf_info);
#endif

    // dumpsys
    int (*dump)(void* pCamHw, st_string* result, int argc, void* argv[]);
#if RKAIQ_HAVE_DUMPSYS
    struct aiq_notifier notifier;
    struct aiq_notifier_subscriber sub_base;
    struct aiq_notifier_subscriber sub_sensor;
    struct aiq_notifier_subscriber sub_params_cvt;
    struct aiq_notifier_subscriber sub_stream_cap;
    struct aiq_notifier_subscriber sub_stream_proc;
    struct aiq_notifier_subscriber sub_isp_params;
    struct aiq_notifier_subscriber sub_isp_active_params;

    struct aiq_notifier_subscriber sub_isp_mods[_MODS_COUNT];

    FrameDumpInfo_t fs;
    FrameDumpInfo_t prev_fs;
    FrameDumpInfo_t stats;
#endif
    rk_aiq_exposure_sensor_descriptor mSnsDes;
} AiqCamHwBase_t;

rk_aiq_static_info_t* AiqCamHw_getStaticCamHwInfo(const char* sns_ent_name, uint16_t index);
rk_aiq_static_info_t* AiqCamHw_getStaticCamHwInfoByPhyId(const char* sns_ent_name, uint16_t);
XCamReturn AiqCamHw_clearStaticCamHwInfo();
XCamReturn AiqCamHw_initCamHwInfos();
XCamReturn AiqCamHw_selectIqFile(const char* sns_ent_name, char* iqfile_name);
const char* AiqCamHw_getBindedSnsEntNmByVd(const char* vd);
const char* AiqCamHw_getBindedIspDrvNmBySns(const char* sns_ent_name);

void AiqCamHw_setCalib(AiqCamHwBase_t* pCamHw, const CamCalibDbV2Context_t* calibv2);
XCamReturn AiqCamHw_prepare(AiqCamHwBase_t* pCamHw, uint32_t width, uint32_t height, int mode,
                            int t_delay, int g_delay);
XCamReturn AiqCamHw_start(AiqCamHwBase_t* pCamHw);
XCamReturn AiqCamHw_stop(AiqCamHwBase_t* pCamHw);
void AiqCamHw_clean(AiqCamHwBase_t* pCamHw);
XCamReturn AiqCamHw_pause(AiqCamHwBase_t* pCamHw);
XCamReturn AiqCamHw_resume(AiqCamHwBase_t* pCamHw);
void AiqCamHw_keepHwStAtStop(AiqCamHwBase_t* pCamHw, bool ks);
XCamReturn AiqCamHw_swWorkingModeDyn(AiqCamHwBase_t* pCamHw, int mode);
XCamReturn AiqCamHw_setIspStreamMode(AiqCamHwBase_t* pCamHw, rk_isp_stream_mode_t mode);
rk_isp_stream_mode_t AiqCamHw_getIspStreamMode(AiqCamHwBase_t* pCamHw);
XCamReturn AiqCamHw_getSensorModeData(AiqCamHwBase_t* pCamHw, const char* sns_ent_name,
                                      rk_aiq_exposure_sensor_descriptor* sns_des);
#if 0
XCamReturn AiqCamHw_applyAnalyzerResult(AiqCamHwBase_t* pCamHw, aiq_params_base_t* pAlgoRes,
                                        bool sync);
#endif
XCamReturn AiqCamHw_applyAnalyzerResultList(AiqCamHwBase_t* pCamHw, AiqList_t* pAlgoResList);
XCamReturn AiqCamHw_setHwResListener(AiqCamHwBase_t* pCamHw, AiqHwResListener_t* resListener);
XCamReturn AiqCamHw_setModuleCtl(AiqCamHwBase_t* pCamHw, rk_aiq_module_id_t mId, bool mod_en);
XCamReturn AiqCamHw_getModuleCtl(AiqCamHwBase_t* pCamHw, rk_aiq_module_id_t mId, bool* mod_en);
XCamReturn AiqCamHw_notify_capture_raw(AiqCamHwBase_t* pCamHw);
XCamReturn AiqCamHw_capture_raw_ctl(AiqCamHwBase_t* pCamHw, capture_raw_t type, int count,
                                    const char* capture_dir, char* output_dir);
XCamReturn AiqCamHw_setSensorFlip(AiqCamHwBase_t* pCamHw, bool mirror, bool flip, int skip_frm_cnt);
XCamReturn AiqCamHw_getSensorFlip(AiqCamHwBase_t* pCamHw, bool* mirror, bool* flip);
XCamReturn AiqCamHw_getSensorCrop(AiqCamHwBase_t* pCamHw, rk_aiq_rect_t* rect);
XCamReturn AiqCamHw_setSensorCrop(AiqCamHwBase_t* pCamHw, rk_aiq_rect_t* rect);
XCamReturn AiqCamHw_getZoomPosition(AiqCamHwBase_t* pCamHw, int* position);
XCamReturn AiqCamHw_getLensVcmCfg(AiqCamHwBase_t* pCamHw, rk_aiq_lens_vcmcfg* lens_cfg);
XCamReturn AiqCamHw_setLensVcmCfg(AiqCamHwBase_t* pCamHw, rk_aiq_lens_vcmcfg* lens_cfg);
XCamReturn AiqCamHw_FocusCorrection(AiqCamHwBase_t* pCamHw);
XCamReturn AiqCamHw_ZoomCorrection(AiqCamHwBase_t* pCamHw);
XCamReturn AiqCamHw_setAngleZ(AiqCamHwBase_t* pCamHw, float angleZ);
XCamReturn AiqCamHw_getFocusPosition(AiqCamHwBase_t* pCamHw, int* position);
void AiqCamHw_getShareMemOps(AiqCamHwBase_t* pCamHw, isp_drv_share_mem_ops_t** mem_ops);
XCamReturn AiqCamHw_getEffectiveIspParams(AiqCamHwBase_t* pCamHw,
                                          aiq_isp_effect_params_t** ispParams, uint32_t frame_id);
uint64_t AiqCamHw_getIspModuleEnState(AiqCamHwBase_t* pCamHw);
XCamReturn AiqCamHw_get_sp_resolution(AiqCamHwBase_t* pCamHw, int* width, int* height,
                                      int* aligned_w, int* aligned_h);
void AiqCamHw_setCamPhyId(AiqCamHwBase_t* pCamHw, int phyId);
int AiqCamHw_getCamPhyId(AiqCamHwBase_t* pCamHw);
#define AiqCamHw_setGroupMode(pCamHw, bGroup, bMain) \
        (pCamHw)->mIsGroupMode = bGroup; (pCamHw)->mIsMain = bMain

#if RKAIQ_HAVE_PDAF
bool AiqCamHw_get_pdaf_support(AiqCamHwBase_t* pCamHw);
PdafSensorType_t AiqCamHw_get_pdaf_type(AiqCamHwBase_t* pCamHw);
#endif
void AiqCamHw_setTbInfo(AiqCamHwBase_t* pCamHw, rk_aiq_tb_info_t* info);

void AiqCamHw_setDevBufCnt(AiqCamHwBase_t* pCamHw, AiqDevBufCntsInfo_t* devBufCntsInfo, int cnt);
XCamReturn AiqCamHw_reset_hardware(AiqCamHwBase_t* pCamHw);

const char* AiqCamHw_rawReproc_preInit(const char* isp_driver);
XCamReturn AiqCamHw_rawReproc_genIspParams(AiqCamHwBase_t* pCamHw, uint32_t sequence,
                                           rk_aiq_frame_info_t* offline_finfo, int mode);
XCamReturn AiqCamHw_rawReProc_prepare(AiqCamHwBase_t* pCamHw, uint32_t sequence);
void AiqCamHw_setAiqPreCtrlInfo(AiqCamHwBase_t* pCamHw, rk_aiq_control_preinit_t* info);
XCamReturn AiqCamHw_waitLastSensorDone(AiqCamHwBase_t* pCamHw);
XCamReturn AiqCamHw_setExposureParams(AiqCamHwBase_t* pCamHw, AiqAecExpInfoWrapper_t* expPar);
void AiqCamHw_setMulCamConc(AiqCamHwBase_t* pCamHw, bool cc);
void AiqCamHw_make_ispHwEvt(AiqCamHwBase_t* pCamHw, Aiqisp20Evt_t* pEvt, uint32_t sequence,
                            int type, int64_t timestamp);
XCamReturn AiqCamHw_notify_sof(AiqCamHwBase_t* pCamHw, AiqHwEvt_t* sof_evt);

XCamReturn AiqCamHwBase_init(AiqCamHwBase_t* pCamHw, const char* sns_ent_name);
void AiqCamHwBase_deinit(AiqCamHwBase_t* pCamHw);

AiqSensorExpInfo_t* Aiqisp20Evt_getExpInfoParams(Aiqisp20Evt_t* pEvt, uint32_t frameId);
XCamReturn AiqCamHw_setVicapStreamMode(AiqCamHwBase_t* pCamHw, int mode, bool is_single_mode);
rk_sensor_full_info_t* AiqCamHw_getFullSnsInfo(const char* sensor_name);
aiq_isp_effect_params_t* AiqCamHw_getParamsForEffMap(AiqCamHwBase_t* pCamHw, uint32_t frame_id);
XCamReturn AiqCam_FastBootSetLastEffectParam(AiqCamHwBase_t* pCamHw);
RkAiqIspUniteMode AiqCamHw_getIspUniteMode(AiqCamHwBase_t* pCamHw);
XCamReturn AiqCamHw_setNoReadbackMode(AiqCamHwBase_t* pCamHw, bool on);
XCamReturn AiqCamHw_setRawBufNum(AiqCamHwBase_t* pCamHw, uint16_t buf_num);
XCamReturn AiqCamHw_setAibnrDelayCnt(AiqCamHwBase_t* pCamHw, int delayCnt);
XCamReturn AiqCamHw_setAiynrDelayCnt(AiqCamHwBase_t* pCamHw, int delayCnt);
XCamReturn AiqCamHw_setSnsOtpInfo(AiqCamHwBase_t* pCamHw, struct rkmodule_awb_inf* otp_awb,
                                  struct rkmodule_lsc_inf* otp_lsc, rk_aiq_user_otp_info_t* user_otp);
XCamReturn AiqCamHw_pushImuData(AiqCamHwBase_t* pCamHw, AiqImuData_t *data);
bool AiqCamHw_getMemcNeededDelay(AiqCamHwBase_t* pCamHw);
XCamReturn AiqCamHw_queryFpnSwStatus(AiqCamHwBase_t* pCamHw, void *status);

#define CamHW_setManager(pMan, pRkAiqManager) \
        (pMan)->rkAiqManager = pRkAiqManager

XCamReturn AiqCamHw_setUserOtpInfo(AiqCamHwBase_t* pCamHw, rk_aiq_user_otp_info_t* otp_info);
#define AiqCamHw_setListenStrmEvt(pCamHw, isListen) \
        (pCamHw)->mIsListenStrmEvt = isListen

XCamReturn AiqCamHw_getHdrComprCurve(AiqCamHwBase_t* pCamHw, RkAiqHdrCompr_t* compr);

XCamReturn AiqCamHw_setSingleRawBufAsyncMode(AiqCamHwBase_t* pCamHw, bool mode);

XCamReturn AiqCamHw_setAiBnrBufCnt(AiqCamHwBase_t* pCamHw,
                                   rk_aiq_aibnr_buffer_count_t aibnr_buf_cnt);
XCamReturn AiqCamHw_getAiBnrBufCnt(AiqCamHwBase_t* pCamHw,
                                   rk_aiq_aibnr_buffer_count_t* aibnr_buf_cnt);

bool AiqCamHw_getAiqReadbackOn(AiqCamHwBase_t* pCamHw);

XCAM_END_DECLARE

#endif
