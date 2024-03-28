
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

#ifndef _CAM_HW_ISP20_H_
#define _CAM_HW_ISP20_H_

#include <linux/rk-video-format.h>
#include "CamHwBase.h"
#include "Isp20Params.h"
#include "SensorHw.h"
#include "LensHw.h"
#include "Isp20StatsBuffer.h"
#include "Stream.h"
#include "RawStreamCapUnit.h"
#include "RawStreamProcUnit.h"
#include "SPStreamProcUnit.h"
#include "PdafStreamProcUnit.h"
#ifdef ISP_HW_V20
#include "TnrStreamProcUnit.h"
#include "NrStreamProcUnit.h"
#include "FecParamStream.h"
#endif
#include "thumbnails.h"
#include "CifScaleStream.h"
#include "anr/rk_aiisp.h"

#include <unordered_map>

struct media_device;

/*
 * [sub modules]: use bits 4-11 to define the sub modules of each module, the
 *     specific meaning of each bit is decided by the module itself. These bits
 *     is designed to implement the sub module's log switch.

 *  ----------------------------
 * |    sub modules    |  bits  |
 *  ----------------------------
 * |  ISP20HW_SUBM     |  0x01  |
 *  ----------------------------
 * |  ISP20PARAM_SUBM  |  0x02  |
 *  ----------------------------
 * |  SENSOR_SUBM      |  0x04  |
 *  ----------------------------
 * |  FL_SUBM          |  0x08  |
 *  ----------------------------
 * |  LENS_SUBM        |  0x10  |
 *  ----------------------------
 * |  CAPTURERAW_SUBM  |  0x80  |
 *  ----------------------------
 */

namespace RkCam {

class IspParamsSplitter;
class AiispLibrary;

#define ISP20HW_SUBM (0x1)

#define MAX_PARAMS_QUEUE_SIZE           5
#define ISP_PARAMS_EFFECT_DELAY_CNT     2
#define CAM_INDEX_FOR_1608              8

#define DISABLE_PARAMS_POLL_THREAD
// FIXME: share 1608 data ptr(aiq/rawdata)
typedef struct sensor_info_share_s {
    RawStreamProcUnit*          raw_proc_unit[CAM_INDEX_FOR_1608];  // bind rx by camId
    SmartPtr<RawStreamCapUnit>  raw_cap_unit;                       // save 1st tx obj addr
    char                        reference_name[64];                 // save vicap name(for 1608)
    rk_aiq_cif_info_t*          reference_mipi_cif;                 // vicap inf (for 1608)
    // us: union_stream
    int                         us_open_cnt;                        // for hwi open(1608)
    int                         us_prepare_cnt;                     // for rawCap buffer manage(1608).
    int                         us_stream_cnt;                      // mark cnt. on: ++, off: --
    int                         us_stop_cnt;                        // last sensor stop
    // tracking opened sensor num
    int                         en_sns_num;                         // Record the number of open sensors
    int                         first_en[CAM_INDEX_FOR_1608];       // set/get fmt for 1608 sensor
} sensor_info_share_t;

class CamHwIsp20
    : public CamHwBase, virtual public Isp20Params, public V4l2Device
    , public isp_drv_share_mem_ops_t {
public:
    friend class RawStreamProcUnit;
    explicit CamHwIsp20();
    virtual ~CamHwIsp20();

    // from ICamHw
    virtual XCamReturn init(const char* sns_ent_name) override;
    virtual XCamReturn deInit() override;
    virtual XCamReturn prepare(uint32_t width, uint32_t height, int mode, int t_delay, int g_delay) override;
    virtual XCamReturn start() override;
    virtual XCamReturn stop() override;
    virtual XCamReturn pause() override;
    virtual XCamReturn resume() override;
    virtual XCamReturn swWorkingModeDyn(int mode) override;
    virtual XCamReturn getSensorModeData(const char* sns_ent_name,
                                         rk_aiq_exposure_sensor_descriptor& sns_des) override;
    virtual void setCalib(const CamCalibDbV2Context_t* calibv2) override;
    virtual XCamReturn applyAnalyzerResult(SmartPtr<SharedItemBase> base, bool sync) override;
    virtual XCamReturn applyAnalyzerResult(cam3aResultList& list) override;
    XCamReturn setModuleCtl(rk_aiq_module_id_t moduleId, bool en) override;
    XCamReturn getModuleCtl(rk_aiq_module_id_t moduleId, bool& en) override;
    XCamReturn notify_capture_raw() override;
    XCamReturn capture_raw_ctl(capture_raw_t type, int count = 0, const char* capture_dir = nullptr, char* output_dir = nullptr) override;
    XCamReturn getSensorCrop(rk_aiq_rect_t& rect) override;
    XCamReturn setSensorCrop(rk_aiq_rect_t& rect) override;
    XCamReturn setSensorFlip(bool mirror, bool flip, int skip_frm_cnt) override;
    XCamReturn getSensorFlip(bool& mirror, bool& flip) override;
    void setMulCamConc(bool cc);
    XCamReturn getZoomPosition(int& position) override;
    XCamReturn getIrisParams(SmartPtr<RkAiqIrisParamsProxy>& irisPar, CalibDb_IrisTypeV2_t irisType);
    XCamReturn getLensVcmCfg(rk_aiq_lens_vcmcfg& lens_cfg) override;
    XCamReturn setLensVcmCfg(rk_aiq_lens_vcmcfg& lens_cfg) override;
    XCamReturn setLensVcmCfg(struct rkmodule_inf& mod_info);
    XCamReturn FocusCorrection() override;
    XCamReturn ZoomCorrection() override;
    XCamReturn setAngleZ(float angleZ) override;
    XCamReturn getFocusPosition(int& position) override;
    virtual void getShareMemOps(isp_drv_share_mem_ops_t** mem_ops) override;
    uint64_t getIspModuleEnState()  override;

    static rk_aiq_static_info_t* getStaticCamHwInfo(const char* sns_ent_name, uint16_t index = 0);
    static rk_aiq_static_info_t* getStaticCamHwInfoByPhyId(const char* sns_ent_name, uint16_t index = 0);
    static XCamReturn clearStaticCamHwInfo();
    static XCamReturn initCamHwInfos();
    static XCamReturn selectIqFile(const char* sns_ent_name, char* iqfile_name);
    static const char* getBindedSnsEntNmByVd(const char* vd);

    // from PollCallback
    virtual XCamReturn poll_event_ready (uint32_t sequence, int type) override;
    virtual XCamReturn poll_event_failed (int64_t timestamp, const char *msg) override;
    XCamReturn rawReproc_genIspParams (uint32_t sequence, rk_aiq_frame_info_t *offline_finfo, int mode) override;
    XCamReturn rawReProc_prepare (uint32_t sequence, rk_aiq_frame_info_t *offline_finfo) override;
    //fake sensor
    static const char* rawReproc_preInit(const char* isp_driver, const char* offline_sns_ent_name);
    XCamReturn rawReproc_deInit (const char* fakeSensor);

    XCamReturn getEffectiveIspParams(rkisp_effect_params_v20& ispParams, uint32_t frame_id) override;
    void setHdrGlobalTmoMode(uint32_t frame_id, bool mode);
    XCamReturn setSharpFbcRotation(rk_aiq_rotation_t rot) {
        _sharp_fbc_rotation = rot;
        return XCAM_RETURN_NO_ERROR;
    }
    XCamReturn notify_sof(SmartPtr<VideoBuffer>& buf);
    SmartPtr<ispHwEvt_t> make_ispHwEvt (uint32_t sequence, int type, int64_t timestamp);
    int get_workingg_mode() {
        return _hdr_mode;
    }
    //should be called after prepare
    XCamReturn get_stream_format(rkaiq_stream_type_t type, struct v4l2_format &format);
    XCamReturn get_sp_resolution(int &width, int &height, int &aligned_w, int &aligned_h) override;
    XCamReturn showOtpPdafData(struct rkmodule_pdaf_inf *otp_pdaf);
    XCamReturn showOtpAfData(struct rkmodule_af_inf *otp_af);
#if RKAIQ_HAVE_PDAF
    bool get_pdaf_support() override;
#endif
    virtual XCamReturn setIspStreamMode(rk_isp_stream_mode_t mode) override {
        if (mode == RK_ISP_STREAM_MODE_ONLNIE) {
            mNoReadBack = true;
        } else if (mode == RK_ISP_STREAM_MODE_OFFLNIE) {
            mNoReadBack = false;
        } else {
            return XCAM_RETURN_ERROR_FAILED;
        }
        return XCAM_RETURN_NO_ERROR;
    }
    virtual rk_isp_stream_mode_t getIspStreamMode() override {
        if (true == mNoReadBack)
            return RK_ISP_STREAM_MODE_ONLNIE;
        else
            return RK_ISP_STREAM_MODE_OFFLNIE;
    }
    void notify_isp_stream_status(bool on);
    bool getParamsForEffMap(uint32_t frameId);
    XCamReturn reset_hardware() override;

    // FIXME: Set struct to static.
    static sensor_info_share_t rk1608_share_inf;

    // cif scale flag
    XCamReturn setCifSclStartFlag(int ratio, bool mode);
    XCamReturn setFastAeExp(uint32_t frameId);
    XCamReturn setLastAeExpToRttShared();

    XCamReturn setVicapStreamMode(int mode, bool is_single_mode);
    void setListenStrmEvt(bool isListen) {
        mIsListenStrmEvt = isListen;
    }
private:
    using V4l2Device::start;

#if defined(ISP_HW_V20)
    XCamReturn handlePpReslut(SmartPtr<cam3aResult> &result);
    XCamReturn setPpConfig(SmartPtr<cam3aResult> &result);
#endif
    XCamReturn setExposureParams(SmartPtr<RkAiqExpParamsProxy>& expPar);
    XCamReturn setIrisParams(SmartPtr<RkAiqIrisParamsProxy>& irisPar, CalibDb_IrisTypeV2_t irisType);
    XCamReturn setHdrProcessCount(rk_aiq_luma_params_t luma_params) override;
    XCamReturn setFocusParams(SmartPtr<RkAiqFocusParamsProxy>& focus_params);
    XCamReturn setCpslParams(SmartPtr<RkAiqCpslParamsProxy>& cpsl_params);
    XCamReturn setupHdrLink(int mode, int isp_index, bool enable);
    static void findAttachedSubdevs(struct media_device *device, uint32_t count, rk_sensor_full_info_t *s_info);
    XCamReturn setExpDelayInfo(int mode);
    void analyzePpInitEns(SmartPtr<cam3aResult> &result);
    XCamReturn get_sensor_pdafinfo(rk_sensor_full_info_t *sensor_info, rk_sensor_pdaf_info_t *pdaf_info);
protected:
    XCAM_DEAD_COPY(CamHwIsp20);
    virtual XCamReturn setIspConfig(cam3aResultList* result_list = NULL);
    virtual void updateEffParams(void* params, void* ori_params = NULL) { return; }
    virtual bool processTb(void* params) { return false; }
    virtual XCamReturn poll_buffer_ready (SmartPtr<VideoBuffer> &buf) override;
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
    calib_infos_t _cur_calib_infos;
    int _hdr_mode;
    Mutex _isp_params_cfg_mutex;
    int _state;
    volatile bool _is_exit;
    bool _linked_to_isp;
    bool _linked_to_1608;
#if defined(ISP_HW_V20)
    struct rkispp_params_cfg _full_active_ispp_params;
    uint32_t _ispp_module_init_ens;
    SmartPtr<V4l2SubDevice> _ispp_sd;
#endif
    SmartPtr<V4l2SubDevice> _cif_csi2_sd;
    char sns_name[32];
public:
    static std::unordered_map<std::string, SmartPtr<rk_aiq_static_info_t>> mCamHwInfos;
    static rk_aiq_isp_hw_info_t mIspHwInfos;
    static rk_aiq_cif_hw_info_t mCifHwInfos;
    static std::unordered_map<std::string, SmartPtr<rk_sensor_full_info_t>> mSensorHwInfos;
    static std::unordered_map<std::string, std::string> mFakeCameraName;
    rk_aiq_aiisp_cfg_t mAiisp_cfg;
    virtual XCamReturn read_aiisp_result() override;
    virtual XCamReturn get_aiisp_bay3dbuf() override;
protected:
    static bool mIsMultiIspMode;
    static uint16_t mMultiIspExtendedPixel;
    // TODO: Sync 1608 sensor start streaming
    static XCam::Mutex  _sync_1608_mutex;
    static bool         _sync_1608_done;
#if defined(ISP_HW_V20)
    void gen_full_ispp_params(const struct rkispp_params_cfg* update_params,
                              struct rkispp_params_cfg* full_params);
#endif
#if 0
    void dump_isp_config(struct isp2x_isp_params_cfg* isp_params,
                         SmartPtr<RkAiqIspParamsProxy> aiq_results,
                         SmartPtr<RkAiqIspParamsProxy> aiq_other_results);
    void dumpRawnrFixValue(struct isp2x_rawnr_cfg * pRawnrCfg );
    void dumpTnrFixValue(struct rkispp_tnr_config  * pTnrCfg);
    void dumpUvnrFixValue(struct rkispp_nr_config  * pNrCfg);
    void dumpYnrFixValue(struct rkispp_nr_config  * pNrCfg);
    void dumpSharpFixValue(struct rkispp_sharp_config  * pSharpCfg);
#endif
    XCamReturn setIrcutParams(bool on);
    XCamReturn setIsppSharpFbcRot(struct rkispp_sharp_config* shp_cfg);
    XCamReturn setupPipelineFmt();
    XCamReturn setupPipelineFmtIsp(struct v4l2_subdev_selection& sns_sd_sel,
                                   struct v4l2_subdev_format& sns_sd_fmt,
                                   __u32 sns_v4l_pix_fmt);
    XCamReturn setupPipelineFmtCif(struct v4l2_subdev_selection& sns_sd_sel,
                                   struct v4l2_subdev_format& sns_sd_fmt,
                                   __u32 sns_v4l_pix_fmt);
    XCamReturn setupHdrLink_vidcap(int hdr_mode, int cif_index, bool enable);
#if defined(ISP_HW_V20)
    XCamReturn init_pp(rk_sensor_full_info_t *s_info);
#endif
    virtual bool isOnlineByWorkingMode();
    virtual XCamReturn setAiispMode(rk_aiq_aiisp_cfg_t* aiisp_cfg) override;
#if defined(ISP_HW_V39)
    virtual XCamReturn process_aiisp_restriction(struct isp39_isp_params_cfg* isp_params);
    virtual XCamReturn aiisp_processing(rk_aiq_aiisp_t* aiisp_evt) override;
#endif
    enum mipi_stream_idx {
        MIPI_STREAM_IDX_0   = 1,
        MIPI_STREAM_IDX_1   = 2,
        MIPI_STREAM_IDX_2   = 4,
        MIPI_STREAM_IDX_ALL = 7,
    };
    XCamReturn hdr_mipi_start_mode(int mode);
    XCamReturn hdr_mipi_stop();
    XCamReturn hdr_mipi_prepare_mode(int mode);
    static void allocMemResource(uint8_t id, void *ops_ctx, void *config, void **mem_ctx);
    static void releaseMemResource(uint8_t id, void *mem_ctx);
    static void* getFreeItem(uint8_t id, void *mem_ctx);
    uint64_t _isp_module_ens{0};
    bool mNoReadBack;
    rk_aiq_rotation_t _sharp_fbc_rotation;

    rk_aiq_ldch_share_mem_info_t ldch_mem_info_array[2 * ISP2X_MESH_BUF_NUM];
    rk_aiq_fec_share_mem_info_t fec_mem_info_array[FEC_MESH_BUF_NUM];
    rk_aiq_cac_share_mem_info_t cac_mem_info_array[2 * ISP3X_MESH_BUF_NUM];
    rk_aiq_dbg_share_mem_info_t dbg_mem_info_array[2 * RKISP_INFO2DDR_BUF_MAX];
    typedef struct drv_share_mem_ctx_s {
        void* ops_ctx;
        void* mem_info;
        rk_aiq_drv_share_mem_type_t type;
    } drv_share_mem_ctx_t;
    drv_share_mem_ctx_t _ldch_drv_mem_ctx;
    drv_share_mem_ctx_t _fec_drv_mem_ctx;
    drv_share_mem_ctx_t _cac_drv_mem_ctx;
    drv_share_mem_ctx_t _dbg_drv_mem_ctx;
    Mutex _mem_mutex;
    rk_aiq_rect_t _crop_rect;
    uint32_t _ds_width;
    uint32_t _ds_heigth;
    uint32_t _ds_width_align;
    uint32_t _ds_heigth_align;
    uint32_t _exp_delay;
    rk_aiq_lens_descriptor _lens_des;
    //ispp
#if defined(ISP_HW_V20)
    SmartPtr<FecParamStream>    mFecParamStream;
    SmartPtr<NrStreamProcUnit>  mNrStreamProcUnit;
    SmartPtr<TnrStreamProcUnit> mTnrStreamProcUnit;
#endif
    //isp
    SmartPtr<RKStream>          mLumaStream;
    SmartPtr<RKStatsStream>     mIspStatsStream;
    SmartPtr<RKStream>          mIspParamStream;
    SmartPtr<RKSofEventStream>  mIspSofStream;
    SmartPtr<RKAiispEventStream>  mIspAiispStream;
#if defined(RKAIQ_ENABLE_SPSTREAM)
    SmartPtr<SPStreamProcUnit> mSpStreamUnit;
#endif
    SmartPtr<RkStreamEventPollThread> mIspStremEvtTh;

    SmartPtr<RawStreamCapUnit> mRawCapUnit;
    SmartPtr<RawStreamProcUnit> mRawProcUnit;

    SmartPtr<CifSclStream> mCifScaleStream;

    SmartPtr<PdafStreamProcUnit> mPdafStreamUnit;

    SmartPtr<cam3aResult> get_3a_module_result (cam3aResultList &results, int32_t type);
    XCamReturn handleIsp3aReslut(SmartPtr<cam3aResult> &result);
    XCamReturn handleIsp3aReslut(cam3aResultList& list);
    virtual XCamReturn dispatchResult(SmartPtr<cam3aResult> cam3a_result);
    virtual XCamReturn dispatchResult(cam3aResultList& list);

    std::map<int, cam3aResultList> _camIsp3aResult;

    std::map<uint32_t, SmartPtr<RkAiqIspEffParamsProxy>> _effecting_ispparam_map;
    SmartPtr<RkAiqIspEffParamsPool> mEffectIspParamsPool;
#ifndef DISABLE_PARAMS_ASSEMBLER
    SmartPtr<IspParamsAssembler> mParamsAssembler;
#endif
    uint32_t mPpModuleInitEns;
    bool mVicapIspPhyLinkSupported; // if phsical link between vicap and isp, only isp3x support now
    SmartPtr<IspParamsSplitter> mParamsSplitter;
    enum ISP_STREAM_STATUS_E {
        ISP_STREAM_STATUS_INVALID,
        ISP_STREAM_STATUS_STREAM_ON,
        ISP_STREAM_STATUS_STREAM_OFF,
    };
    int _isp_stream_status;

    rk_sensor_pdaf_info_t mPdafInfo;

    Mutex     _stop_cond_mutex;

    // TODO: Sync(1608 sensor) sdk hwEvt cb
    XCam::Cond      _sync_done_cond;
    XCamReturn      waitLastSensorDone();
    XCamReturn pixFmt2Bpp(uint32_t pixFmt, int8_t& bpp);
    uint32_t _curIspParamsSeq;

    bool use_rkrawstream;

    bool _not_skip_first{true};
    void* _first_awb_cfg{NULL};
    uint32_t mAweekId{0};
    void* _skipped_params{NULL};
    void* _first_awb_param{NULL};
    RkAiqIspUnitedMode mIspUnitedMode;
    bool mIsListenStrmEvt{true};

    bool use_aiisp;
    rk_aiisp_param* aiisp_param;
    std::shared_ptr<AiispLibrary> lib_aiisp_;
};

using rk_aiisp_init = int (*)(rk_aiisp_param* param);
using rk_aiisp_proc = int (*)(rk_aiisp_param* param);
using rk_aiisp_deinit = int (*)(rk_aiisp_param* param);

struct AiispOps {
    rk_aiisp_init aiisp_init;
    rk_aiisp_proc aiisp_proc;
    rk_aiisp_deinit aiisp_deinit;
};

class AiispLibrary {
public:
    AiispLibrary() = default;
    virtual ~AiispLibrary();

    bool Init();
    bool LoadSymbols();

    AiispOps* GetOps();

private:
    void* handle_;
    AiispOps ops_;
};

}
#endif
