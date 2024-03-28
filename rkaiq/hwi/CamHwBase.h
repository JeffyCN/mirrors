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

#ifndef _CAM_HW_BASE_H_
#define _CAM_HW_BASE_H_

//#include <map>
#include <string>
#include "xcam_thread.h"
#include "ICamHw.h"
#include "v4l2_device.h"
#include "fake_v4l2_device.h"
#include "poll_thread.h"
#ifndef RK_SIMULATOR_HW
#include "FlashLight.h"
#endif

#include "rk_aiq.h"

using namespace XCam;

namespace RkCam {

class CamHwBase
    : public ICamHw
    , public PollCallback {
public:
    explicit CamHwBase();
    virtual ~CamHwBase();
    // from ICamHw
    virtual XCamReturn init(const char* sns_ent_name);
    virtual XCamReturn deInit();
#ifdef RKAIQ_ENABLE_PARSER_V1
    virtual void setCalib(const CamCalibDbContext_t* calib) {
        mCalibDb = calib;
    }
#endif
    virtual void setCalib(const CamCalibDbV2Context_t* calibv2) {
        mCalibDbV2 = calibv2;
    }
    virtual XCamReturn prepare(uint32_t width, uint32_t height, int mode, int t_dealy, int g_delay);
    virtual XCamReturn start();
    virtual XCamReturn stop();
    virtual XCamReturn pause();
    virtual XCamReturn resume();
    virtual XCamReturn swWorkingModeDyn(int mode) {
        return XCAM_RETURN_NO_ERROR;
    };
    virtual void keepHwStAtStop(bool ks) {
        mKpHwSt = ks;
    };
    virtual XCamReturn getSensorModeData(const char* sns_ent_name,
                                         rk_aiq_exposure_sensor_descriptor& sns_des);
    virtual XCamReturn setHwResListener(HwResListener* resListener);
    virtual XCamReturn setHdrProcessCount(rk_aiq_luma_params_t luma_params) {
        return XCAM_RETURN_ERROR_FAILED;
    };
    virtual XCamReturn applyAnalyzerResult(SmartPtr<SharedItemBase> base, bool sync) {
        return  XCAM_RETURN_ERROR_FAILED;
    };
    virtual XCamReturn applyAnalyzerResult(cam3aResultList& list) {
        return  XCAM_RETURN_ERROR_FAILED;
    }
    // from PollCallback
    virtual XCamReturn poll_buffer_ready (SmartPtr<VideoBuffer> &buf, int type);
    virtual XCamReturn poll_buffer_failed (int64_t timestamp, const char *msg);
    virtual XCamReturn poll_event_ready (uint32_t sequence, int type);
    virtual XCamReturn poll_event_failed (int64_t timestamp, const char *msg);
    virtual XCamReturn poll_buffer_ready (SmartPtr<VideoBuffer> &buf);
    virtual XCamReturn poll_buffer_ready (SmartPtr<V4l2BufferProxy> &buf, int dev_index) { return XCAM_RETURN_ERROR_FAILED; };

    virtual XCamReturn notify_capture_raw() {
        return  XCAM_RETURN_ERROR_FAILED;
    }
    virtual XCamReturn capture_raw_ctl(capture_raw_t type, int count = 0,
                                       const char* capture_dir = nullptr, \
                                       char* output_dir = nullptr) {
        return  XCAM_RETURN_ERROR_FAILED;
    }
    virtual XCamReturn enqueueRawBuffer(void *vbuf, bool sync) {
        return  XCAM_RETURN_ERROR_FAILED;
    }
    virtual XCamReturn enqueueRawFile(const char *path) {
        return  XCAM_RETURN_ERROR_FAILED;
    }
    virtual XCamReturn registRawdataCb(void (*callback)(void *)) {
        return  XCAM_RETURN_ERROR_FAILED;
    }
    virtual XCamReturn rawdataPrepare(rk_aiq_raw_prop_t prop) {
        return  XCAM_RETURN_ERROR_FAILED;
    }
    virtual XCamReturn setSensorFlip(bool mirror, bool flip, int skip_frm_cnt) {
        return  XCAM_RETURN_ERROR_FAILED;
    }
    virtual XCamReturn getSensorFlip(bool& mirror, bool& flip) {
        return  XCAM_RETURN_ERROR_FAILED;
    }
    virtual XCamReturn getZoomPosition(int& position) {
        return  XCAM_RETURN_ERROR_FAILED;
    }
    virtual XCamReturn setLensVcmCfg(rk_aiq_lens_vcmcfg& lens_cfg) {
        return  XCAM_RETURN_ERROR_FAILED;
    }
    virtual XCamReturn getLensVcmCfg(rk_aiq_lens_vcmcfg& lens_cfg) {
        return  XCAM_RETURN_ERROR_FAILED;
    }
    virtual XCamReturn FocusCorrection() {
        return  XCAM_RETURN_ERROR_FAILED;
    }
    virtual XCamReturn ZoomCorrection() {
        return  XCAM_RETURN_ERROR_FAILED;
    }
    virtual XCamReturn setAngleZ(float angleZ) {
        return  XCAM_RETURN_ERROR_FAILED;
    }
    virtual XCamReturn getFocusPosition(int& position) {
        return  XCAM_RETURN_ERROR_FAILED;
    }
    virtual void getShareMemOps(isp_drv_share_mem_ops_t** mem_ops) {};
    virtual XCamReturn getEffectiveIspParams(rkisp_effect_params_v20& ispParams, uint32_t frame_id) {
        return  XCAM_RETURN_ERROR_FAILED;
    };
    uint64_t getIspModuleEnState() {
        return true;
    };
    virtual XCamReturn setIspStreamMode(rk_isp_stream_mode_t mode) {
        return XCAM_RETURN_ERROR_FAILED;
    }
    virtual rk_isp_stream_mode_t getIspStreamMode() {
        return RK_ISP_STREAM_MODE_INVALID;
    }
    virtual void setCamPhyId(int phyId) {
        mCamPhyId = phyId;
    }
    virtual int getCamPhyId() { return mCamPhyId;}
    virtual void setGroupMode(bool bGroup, bool bMain) { mIsGroupMode = bGroup; mIsMain = bMain;}
    virtual void setTbInfo(RkAiqTbInfo_t& info) {
        mTbInfo = info;
    }
    virtual void setDevBufCnt(const std::map<std::string, int>& dev_buf_cnt_map) {
        mDevBufCntMap = dev_buf_cnt_map;
    }
    virtual XCamReturn reset_hardware() {
        return XCAM_RETURN_ERROR_FAILED;
    };
    virtual XCamReturn rawReproc_genIspParams (uint32_t sequence, rk_aiq_frame_info_t *offline_finfo, int mode) {
        return XCAM_RETURN_ERROR_FAILED;
    }
    virtual XCamReturn rawReProc_prepare (uint32_t sequence, rk_aiq_frame_info_t *offline_finfo) {
        return XCAM_RETURN_ERROR_FAILED;
    }
    virtual void setRawStreamInfo(rk_aiq_rkrawstream_info_t *info) {
        mRawStreamInfo = *info;
    }
    virtual XCamReturn setAiispMode(rk_aiq_aiisp_cfg_t *aiisp_cfg)
    {
        return XCAM_RETURN_ERROR_FAILED;
    }
    virtual XCamReturn read_aiisp_result()
    {
        return XCAM_RETURN_ERROR_FAILED;
    }
    virtual XCamReturn get_aiisp_bay3dbuf()
    {
        return XCAM_RETURN_ERROR_FAILED;
    }
    virtual XCamReturn aiisp_processing(rk_aiq_aiisp_t *aiisp_evt)
    {
        return XCAM_RETURN_ERROR_FAILED;
    }
    HwResListener* mHwResLintener;

protected:
    SmartPtr<V4l2Device> mIsppFecParamsDev;
    SmartPtr<V4l2Device> mIspLumaDev;
    SmartPtr<V4l2Device> mIspStatsDev;
    SmartPtr<V4l2Device> mIspParamsDev;
    SmartPtr<V4l2SubDevice> mIspCoreDev;
    SmartPtr<V4l2SubDevice> mSensorDev;
    SmartPtr<V4l2SubDevice> mLensDev;
    SmartPtr<V4l2SubDevice> mIrcutDev;
    SmartPtr<V4l2Device> mIspSpDev;
#ifndef RK_SIMULATOR_HW
    SmartPtr<FlashLightHw> mFlashLight;
    SmartPtr<FlashLightHw> mFlashLightIr;
#endif
    int mWorkingMode;
    const CamCalibDbContext_t* mCalibDb;
    const CamCalibDbV2Context_t* mCalibDbV2;
    bool mKpHwSt;
    int mCamPhyId;
    bool mIsGroupMode;
    bool mIsMain;
    RkAiqTbInfo_t mTbInfo;
    std::map<std::string, int> mDevBufCntMap;
    rk_aiq_rkrawstream_info_t mRawStreamInfo;

 private:
    XCAM_DEAD_COPY (CamHwBase);
};

} //namespace RkCam

#endif
