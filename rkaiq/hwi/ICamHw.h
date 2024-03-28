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

#ifndef _ICAMHW_H_
#define _ICAMHW_H_

#include "video_buffer.h"
#include "smartptr.h"
#include "rk_aiq_types.h"
#include "rk_aiq_pool.h"
#include "RkAiqCalibDbV2Helper.h"
#include "rk_aiq.h"
#include "rk_aiq_offline_raw.h"
namespace RkCam {

using namespace XCam;

typedef struct ispHwEvt_s {
    virtual ~ispHwEvt_s() {};
    int evt_code;
    union {
        uint32_t frame_id;
    } msg;
} ispHwEvt_t;

class IsppStatsListener {
public:
    IsppStatsListener() {};
    virtual ~IsppStatsListener() {};
    virtual XCamReturn isppStatsCb(SmartPtr<VideoBuffer>& isppStats) = 0;
private:
    XCAM_DEAD_COPY (IsppStatsListener);
};

class IspLumaListener {
public:
    IspLumaListener() {};
    virtual ~IspLumaListener() {};
    virtual XCamReturn ispLumaCb(SmartPtr<VideoBuffer>& ispLuma) = 0;
private:
    XCAM_DEAD_COPY (IspLumaListener);
};

class IspStatsListener {
public:
    IspStatsListener() {};
    virtual ~IspStatsListener() {};
    virtual XCamReturn ispStatsCb(SmartPtr<VideoBuffer>& ispStats) = 0;
private:
    XCAM_DEAD_COPY (IspStatsListener);
};

class IspEvtsListener {
public:
    IspEvtsListener() {};
    virtual ~IspEvtsListener() {};
    virtual XCamReturn ispEvtsCb(SmartPtr<ispHwEvt_t> evt) = 0;
private:
    XCAM_DEAD_COPY (IspEvtsListener);
};

class HwResListener {
public:
    HwResListener() {};
    virtual ~HwResListener() {};
    virtual XCamReturn hwResCb(SmartPtr<VideoBuffer>& resources) = 0;
private:
    XCAM_DEAD_COPY (HwResListener);
};

class ICamHw {
public:
    ICamHw() {};
    virtual ~ICamHw() {};
    virtual XCamReturn init(const char* sns_ent_name) = 0;
    virtual XCamReturn deInit() = 0;
#ifdef RKAIQ_ENABLE_PARSER_V1
    virtual void setCalib(const CamCalibDbContext_t* calib) = 0;
#endif
    virtual void setCalib(const CamCalibDbV2Context_t* calibv2) = 0;
    virtual XCamReturn prepare(uint32_t width, uint32_t height, int mode, int t_delay, int g_delay) = 0;
    virtual XCamReturn start() = 0;
    virtual XCamReturn stop() = 0;
    virtual XCamReturn pause() = 0;
    virtual XCamReturn resume() = 0;
    virtual void keepHwStAtStop(bool ks) = 0;
    virtual XCamReturn swWorkingModeDyn(int mode) = 0;
    virtual XCamReturn setIspStreamMode(rk_isp_stream_mode_t mode) = 0;
    virtual rk_isp_stream_mode_t getIspStreamMode() = 0;
    virtual XCamReturn getSensorModeData(const char* sns_ent_name,
                                         rk_aiq_exposure_sensor_descriptor& sns_des) = 0;
    virtual XCamReturn setHdrProcessCount(rk_aiq_luma_params_t luma_params) = 0;
    virtual XCamReturn applyAnalyzerResult(SmartPtr<SharedItemBase> base, bool sync) = 0;
    virtual XCamReturn applyAnalyzerResult(cam3aResultList& list) = 0;
    virtual XCamReturn setHwResListener(HwResListener* resListener) = 0;
    virtual XCamReturn setModuleCtl(rk_aiq_module_id_t mId, bool mod_en) = 0;
    virtual XCamReturn getModuleCtl(rk_aiq_module_id_t mId, bool& mod_en) = 0;
    virtual XCamReturn notify_capture_raw() = 0;
    virtual XCamReturn capture_raw_ctl(capture_raw_t type, int count = 0,
                                       const char* capture_dir = nullptr,
                                       char* output_dir = nullptr) = 0;
    virtual XCamReturn enqueueRawBuffer(void *vbuf, bool sync) = 0;
    virtual XCamReturn enqueueRawFile(const char *path) = 0;
    virtual XCamReturn registRawdataCb(void (*callback)(void *)) = 0;
    virtual XCamReturn rawdataPrepare(rk_aiq_raw_prop_t prop) = 0;
    virtual XCamReturn setSensorFlip(bool mirror, bool flip, int skip_frm_cnt) = 0;
    virtual XCamReturn getSensorFlip(bool& mirror, bool& flip) = 0;
    virtual XCamReturn getSensorCrop(rk_aiq_rect_t& rect) = 0;
    virtual XCamReturn setSensorCrop(rk_aiq_rect_t& rect) = 0;
    virtual XCamReturn getZoomPosition(int& position) = 0;
    virtual XCamReturn getLensVcmCfg(rk_aiq_lens_vcmcfg& lens_cfg) = 0;
    virtual XCamReturn setLensVcmCfg(rk_aiq_lens_vcmcfg& lens_cfg) = 0;
    virtual XCamReturn FocusCorrection() = 0;
    virtual XCamReturn ZoomCorrection() = 0;
    virtual XCamReturn setAngleZ(float angleZ) = 0;
    virtual XCamReturn getFocusPosition(int& position) = 0;
    virtual void getShareMemOps(isp_drv_share_mem_ops_t** mem_ops) = 0;
    virtual XCamReturn getEffectiveIspParams(rkisp_effect_params_v20& ispParams, uint32_t frame_id) = 0;
    virtual uint64_t getIspModuleEnState() = 0;
    virtual XCamReturn get_sp_resolution(int &width, int &height, int &aligned_w, int &aligned_h) = 0;
    virtual void setCamPhyId(int phyId) = 0;
    virtual int getCamPhyId() = 0;
    virtual void setGroupMode(bool bGroup, bool bMain) = 0;
#if RKAIQ_HAVE_PDAF
    virtual bool get_pdaf_support() = 0;
#endif
    virtual void setTbInfo(RkAiqTbInfo_t& info) = 0;
    virtual void setDevBufCnt(const std::map<std::string, int>& dev_buf_cnt_map) = 0;
    virtual XCamReturn reset_hardware() = 0;
    virtual XCamReturn rawReproc_genIspParams (uint32_t sequence, rk_aiq_frame_info_t *offline_finfo, int mode) = 0;
    virtual XCamReturn rawReProc_prepare (uint32_t sequence, rk_aiq_frame_info_t *offline_finfo) = 0;
    virtual void setRawStreamInfo(rk_aiq_rkrawstream_info_t *info) = 0;
    virtual XCamReturn setAiispMode(rk_aiq_aiisp_cfg_t *aiisp_cfg) = 0;
    virtual XCamReturn read_aiisp_result() = 0;
    virtual XCamReturn get_aiisp_bay3dbuf() = 0;
    virtual XCamReturn aiisp_processing(rk_aiq_aiisp_t *aiisp_evt) = 0;
private:
    XCAM_DEAD_COPY (ICamHw);
};

} //namespace RkCam

#endif
