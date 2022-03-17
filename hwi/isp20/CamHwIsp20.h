
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
#include "TnrStreamProcUnit.h"
#include "NrStreamProcUnit.h"
#include "FecParamStream.h"
#include "thumbnails.h"

struct media_device;

namespace RkCam {

class IspParamsSplitter;

#define ISP20HW_SUBM (0x1)

#define MAX_PARAMS_QUEUE_SIZE           5
#define ISP_PARAMS_EFFECT_DELAY_CNT     2

class CamHwIsp20
    : public CamHwBase, virtual public Isp20Params, public V4l2Device
    , public isp_drv_share_mem_ops_t {
public:
    explicit CamHwIsp20();
    virtual ~CamHwIsp20();

    // from ICamHw
    virtual XCamReturn init(const char* sns_ent_name);
    virtual XCamReturn deInit();
    virtual XCamReturn prepare(uint32_t width, uint32_t height, int mode, int t_delay, int g_delay);
    virtual XCamReturn start();
    virtual XCamReturn stop();
    virtual XCamReturn pause();
    virtual XCamReturn resume();
    virtual XCamReturn swWorkingModeDyn(int mode);
    virtual XCamReturn getSensorModeData(const char* sns_ent_name,
                                         rk_aiq_exposure_sensor_descriptor& sns_des);
    virtual void setCalib(const CamCalibDbV2Context_t* calibv2);
    virtual XCamReturn applyAnalyzerResult(SmartPtr<SharedItemBase> base, bool sync);
    virtual XCamReturn applyAnalyzerResult(cam3aResultList& list);
    XCamReturn setModuleCtl(rk_aiq_module_id_t moduleId, bool en);
    XCamReturn getModuleCtl(rk_aiq_module_id_t moduleId, bool& en);
    XCamReturn notify_capture_raw();
    XCamReturn capture_raw_ctl(capture_raw_t type, int count = 0, const char* capture_dir = nullptr, char* output_dir = nullptr);
    XCamReturn getSensorCrop(rk_aiq_rect_t& rect);
    XCamReturn setSensorCrop(rk_aiq_rect_t& rect);
    XCamReturn setSensorFlip(bool mirror, bool flip, int skip_frm_cnt);
    XCamReturn getSensorFlip(bool& mirror, bool& flip);
    void setMulCamConc(bool cc);
    XCamReturn getZoomPosition(int& position);
    XCamReturn getLensVcmCfg(rk_aiq_lens_vcmcfg& lens_cfg);
    XCamReturn setLensVcmCfg(rk_aiq_lens_vcmcfg& lens_cfg);
    XCamReturn setLensVcmCfg(struct rkmodule_inf& mod_info);
    XCamReturn FocusCorrection();
    XCamReturn ZoomCorrection();
    XCamReturn setAngleZ(float angleZ);
    virtual void getShareMemOps(isp_drv_share_mem_ops_t** mem_ops);
    uint64_t getIspModuleEnState();

    static rk_aiq_static_info_t* getStaticCamHwInfo(const char* sns_ent_name, uint16_t index = 0);
    static XCamReturn clearStaticCamHwInfo();
    static XCamReturn initCamHwInfos();
    static XCamReturn selectIqFile(const char* sns_ent_name, char* iqfile_name);
    static const char* getBindedSnsEntNmByVd(const char* vd);

    // from PollCallback
    virtual XCamReturn poll_event_ready (uint32_t sequence, int type);
    virtual XCamReturn poll_event_failed (int64_t timestamp, const char *msg);

    XCamReturn getEffectiveIspParams(rkisp_effect_params_v20& ispParams, int frame_id);
    void setHdrGlobalTmoMode(int frame_id, bool mode);
    XCamReturn setSharpFbcRotation(rk_aiq_rotation_t rot) {
        _sharp_fbc_rotation = rot;
        return XCAM_RETURN_NO_ERROR;
    }
    XCamReturn notify_sof(SmartPtr<VideoBuffer>& buf);
    SmartPtr<ispHwEvt_t> make_ispHwEvt (uint32_t sequence, int type, int64_t timestamp);
    int get_workingg_mode() { return _hdr_mode; }
    //should be called after prepare
    XCamReturn get_stream_format(rkaiq_stream_type_t type, struct v4l2_format &format);
    XCamReturn get_sp_resolution(int &width, int &height, int &aligned_w, int &aligned_h);
    XCamReturn showOtpPdafData(struct rkmodule_pdaf_inf *otp_pdaf);
    XCamReturn showOtpAfData(struct rkmodule_af_inf *otp_af);
    bool get_pdaf_support();
    virtual XCamReturn setIspStreamMode(rk_isp_stream_mode_t mode) {
        if (mode == RK_ISP_STREAM_MODE_ONLNIE) {
            mNoReadBack = true;
        } else if (mode == RK_ISP_STREAM_MODE_OFFLNIE) {
            mNoReadBack = false;
        } else {
            return XCAM_RETURN_ERROR_FAILED;
        }
        return XCAM_RETURN_NO_ERROR;
    }
    virtual rk_isp_stream_mode_t getIspStreamMode() {
        if (true == mNoReadBack)
            return RK_ISP_STREAM_MODE_ONLNIE;
        else
            return RK_ISP_STREAM_MODE_OFFLNIE;
    }
    void notify_isp_stream_status(bool on);
private:
    XCamReturn handlePpReslut(SmartPtr<cam3aResult> &result);
    XCamReturn setPpConfig(SmartPtr<cam3aResult> &result);
    XCamReturn setExposureParams(SmartPtr<RkAiqExpParamsProxy>& expPar);
    XCamReturn setIrisParams(SmartPtr<RkAiqIrisParamsProxy>& irisPar, CalibDb_IrisTypeV2_t irisType);
    XCamReturn setHdrProcessCount(rk_aiq_luma_params_t luma_params);
    XCamReturn setFocusParams(SmartPtr<RkAiqFocusParamsProxy>& focus_params);
    XCamReturn setCpslParams(SmartPtr<RkAiqCpslParamsProxy>& cpsl_params);
    XCamReturn setupHdrLink(int mode, int isp_index, bool enable);
    static void findAttachedSubdevs(struct media_device *device, uint32_t count, rk_sensor_full_info_t *s_info);
    XCamReturn setExpDelayInfo(int mode);
    void analyzePpInitEns(SmartPtr<cam3aResult> &result);
    XCamReturn get_sensor_pdafinfo(rk_sensor_full_info_t *sensor_info, rk_sensor_pdaf_info_t *pdaf_info);
protected:
    XCAM_DEAD_COPY(CamHwIsp20);
    virtual XCamReturn setIspConfig();
    virtual XCamReturn poll_buffer_ready (SmartPtr<VideoBuffer> &buf);
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
    struct isp2x_isp_params_cfg _full_active_isp_params;
    struct rkispp_params_cfg _full_active_ispp_params;
    uint32_t _ispp_module_init_ens;
    SmartPtr<V4l2SubDevice> _ispp_sd;
    SmartPtr<V4l2SubDevice> _cif_csi2_sd;
    char sns_name[32];
    static std::map<std::string, SmartPtr<rk_aiq_static_info_t>> mCamHwInfos;
    static rk_aiq_isp_hw_info_t mIspHwInfos;
    static rk_aiq_cif_hw_info_t mCifHwInfos;
    static std::map<std::string, SmartPtr<rk_sensor_full_info_t>> mSensorHwInfos;
    static bool mIsMultiIspMode;
    static uint16_t mMultiIspExtendedPixel;
    void gen_full_isp_params(const struct isp2x_isp_params_cfg* update_params,
                             struct isp2x_isp_params_cfg* full_params,
                                uint64_t* module_en_update_partial,
                                uint64_t* module_cfg_update_partial);
    void gen_full_ispp_params(const struct rkispp_params_cfg* update_params,
                              struct rkispp_params_cfg* full_params);
    XCamReturn overrideExpRatioToAiqResults(const sint32_t frameId,
                int module_id,
                cam3aResultList &results,
                int hdr_mode);
#if 0
    void dump_isp_config(struct isp2x_isp_params_cfg* isp_params,
                         SmartPtr<RkAiqIspParamsProxy> aiq_results,
                         SmartPtr<RkAiqIspParamsProxy> aiq_other_results);
#endif
    void dumpRawnrFixValue(struct isp2x_rawnr_cfg * pRawnrCfg );
    void dumpTnrFixValue(struct rkispp_tnr_config  * pTnrCfg);
    void dumpUvnrFixValue(struct rkispp_nr_config  * pNrCfg);
    void dumpYnrFixValue(struct rkispp_nr_config  * pNrCfg);
    void dumpSharpFixValue(struct rkispp_sharp_config  * pSharpCfg);
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
    XCamReturn init_pp(rk_sensor_full_info_t *s_info);
    virtual bool isOnlineByWorkingMode();
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
    uint32_t _isp_module_ens;
    bool mNoReadBack;
    uint64_t ispModuleEns;
    rk_aiq_rotation_t _sharp_fbc_rotation;

    rk_aiq_ldch_share_mem_info_t ldch_mem_info_array[2*ISP2X_MESH_BUF_NUM];
    rk_aiq_fec_share_mem_info_t fec_mem_info_array[FEC_MESH_BUF_NUM];
    rk_aiq_cac_share_mem_info_t cac_mem_info_array[2*ISP3X_MESH_BUF_NUM];
    typedef struct drv_share_mem_ctx_s {
        void* ops_ctx;
        void* mem_info;
        rk_aiq_drv_share_mem_type_t type;
    } drv_share_mem_ctx_t;
    drv_share_mem_ctx_t _ldch_drv_mem_ctx;
    drv_share_mem_ctx_t _fec_drv_mem_ctx;
    drv_share_mem_ctx_t _cac_drv_mem_ctx;
    Mutex _mem_mutex;
    rk_aiq_rect_t _crop_rect;
    uint32_t _ds_width;
    uint32_t _ds_heigth;
    uint32_t _ds_width_align;
    uint32_t _ds_heigth_align;
    uint32_t _exp_delay;
    rk_aiq_lens_descriptor _lens_des;
    //ispp
    SmartPtr<FecParamStream>    mFecParamStream;
    SmartPtr<NrStreamProcUnit>  mNrStreamProcUnit;
    SmartPtr<TnrStreamProcUnit> mTnrStreamProcUnit;
    //isp
    SmartPtr<RKStream>          mLumaStream;
    SmartPtr<RKStatsStream>     mIspStatsStream;
    SmartPtr<RKStream>          mIspParamStream;
    SmartPtr<RKSofEventStream>  mIspSofStream;
    SmartPtr<SPStreamProcUnit> mSpStreamUnit;
    SmartPtr<RkStreamEventPollThread> mIspStremEvtTh;

    SmartPtr<RawStreamCapUnit> mRawCapUnit;
    SmartPtr<RawStreamProcUnit> mRawProcUnit;

    SmartPtr<PdafStreamProcUnit> mPdafStreamUnit;

    SmartPtr<cam3aResult> get_3a_module_result (cam3aResultList &results, int32_t type);
    XCamReturn handleIsp3aReslut(SmartPtr<cam3aResult> &result);
    XCamReturn handleIsp3aReslut(cam3aResultList& list);
    virtual XCamReturn dispatchResult(SmartPtr<cam3aResult> cam3a_result);
    virtual XCamReturn dispatchResult(cam3aResultList& list);

    std::map<int, cam3aResultList> _camIsp3aResult;

    std::map<int, rkisp_effect_params_v20> _effecting_ispparam_map;
    SmartPtr<IspParamsAssembler> mParamsAssembler;
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
};

};
#endif
