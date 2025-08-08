/*
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
#ifndef _RAW_STREAM_PROC_UNIT_H_
#define _RAW_STREAM_PROC_UNIT_H_

#include <v4l2_device.h>

#include <map>

#include "poll_thread.h"
#include "safe_queue.h"
#include "xcam_mutex.h"
#include "Stream.h"
#include "CaptureRawData.h"
#include "DumpRkRaw.h"

// #define UseCaptureRawData

using namespace XCam;

namespace RkCam {

class CamHwIsp20;
class EmptyClass;
class RawProcThread;
class RawStreamProcUnit : public PollCallback
{
public:
    explicit RawStreamProcUnit      ();
    explicit RawStreamProcUnit(const rk_sensor_full_info_t* s_info, bool linked_to_isp,
                               int tx_buf_cnt);
    virtual ~RawStreamProcUnit ();
    virtual XCamReturn start        (int mode);
    virtual XCamReturn stop         ();
    void set_working_mode           (int mode);
    XCamReturn prepare(int idx);
    void set_rx_devices             (SmartPtr<V4l2Device> mipi_rx_devs[3]);
    SmartPtr<V4l2Device> get_rx_device (int index);
    XCamReturn set_rx_format              (const struct v4l2_subdev_format& sns_sd_fmt, uint32_t sns_v4l_pix_fmt);
    XCamReturn set_rx_format              (const struct v4l2_subdev_selection& sns_sd_sel, uint32_t sns_v4l_pix_fmt);
    void set_devices                (SmartPtr<V4l2SubDevice> ispdev, CamHwIsp20* handle);
    void set_hdr_frame_readback_infos(uint32_t frame_id, int times);
    void set_hdr_global_tmo_mode(uint32_t frame_id, bool mode);
    void notify_sof(uint64_t time, uint32_t frameid);
    void send_sync_buf(SmartPtr<V4l2BufferProxy> &buf_s, SmartPtr<V4l2BufferProxy> &buf_m, SmartPtr<V4l2BufferProxy> &buf_l);
    bool raw_buffer_proc();
    void setMulCamConc(bool cc) {
        _is_multi_cam_conc = cc;
    }
    enum {
        ISP_MIPI_HDR_S = 0,
        ISP_MIPI_HDR_M,
        ISP_MIPI_HDR_L,
        ISP_MIPI_HDR_MAX,
    };
    // notify CamHwIsp one frame has been processed
    void setPollCallback(PollCallback* cb) { _PollCallback = cb; }
    // from PollCallback
    virtual XCamReturn poll_buffer_ready (SmartPtr<VideoBuffer> &buf, int type) { return XCAM_RETURN_ERROR_FAILED; }
    virtual XCamReturn poll_buffer_failed (int64_t timestamp, const char *msg) { return XCAM_RETURN_ERROR_FAILED; }
    virtual XCamReturn poll_buffer_ready (SmartPtr<VideoBuffer> &buf) { return XCAM_RETURN_ERROR_FAILED; }
    virtual XCamReturn poll_buffer_ready (SmartPtr<V4l2BufferProxy> &buf, int dev_index);
    virtual XCamReturn poll_event_ready (uint32_t sequence, int type) { return XCAM_RETURN_ERROR_FAILED; }
    virtual XCamReturn poll_event_failed (int64_t timestamp, const char *msg) { return XCAM_RETURN_ERROR_FAILED; }
    void setCamPhyId(int phyId) {
        mCamPhyId = phyId;
    }
    void setSensorCategory(bool sensorState) {
        _is_1608_sensor = sensorState;
    }
    XCamReturn capture_raw_ctl(capture_raw_t type, int count = 0, const char* capture_dir = nullptr, char* output_dir = nullptr) {
        if (!_rawCap)
            return XCAM_RETURN_ERROR_FAILED;
#ifdef UseCaptureRawData
        return _rawCap->capture_raw_ctl(type, count, capture_dir, output_dir);
#else
        return _rawCap->dumpControl(type, count, capture_dir, output_dir);
#endif
    }

    XCamReturn notify_capture_raw() {
        if (!_rawCap)
            return XCAM_RETURN_ERROR_FAILED;
#ifdef UseCaptureRawData
        return _rawCap->notify_capture_raw();
#else
        return _rawCap->notifyDumpRawFromThirdParty();
#endif
    }
    XCamReturn set_csi_mem_word_big_align(uint32_t width, uint32_t height, uint32_t sns_v4l_pix_fmt, int8_t sns_bpp);

protected:
    XCAM_DEAD_COPY (RawStreamProcUnit);

    void trigger_isp_readback           ();
    void match_lumadetect_map           (uint32_t sequence, sint32_t &additional_times);
    void match_globaltmostate_map(uint32_t sequence, bool &isHdrGlobalTmo);
    XCamReturn match_sof_timestamp_map(sint32_t sequence, uint64_t &timestamp);
    XCamReturn setIspInfoToDump();
    XCamReturn DumpRkRawInFrameEnd(SmartPtr<V4l2BufferProxy> &rx_buf);
    int8_t getDumpRkRawType();
    int mCamPhyId;
protected:
    SmartPtr<V4l2Device> _dev[3];
    int _dev_index[3]{0};
    SmartPtr<RKStream> _stream[3];
    int _working_mode{0};
    int _mipi_dev_max{1};
    bool _is_multi_cam_conc;
    //
    Mutex _buf_mutex;
    std::map<uint32_t, int> _isp_hdr_fid2times_map;
    std::map<uint32_t, bool> _isp_hdr_fid2ready_map;
    std::map<uint32_t, bool> _hdr_global_tmo_state_map;
    std::map<sint32_t, uint64_t> _sof_timestamp_map;
    SafeList<V4l2BufferProxy> buf_list[3];
    SafeList<V4l2BufferProxy> cache_list[3];

    SmartPtr<RawProcThread> _raw_proc_thread;
    CamHwIsp20* _camHw{NULL};
    SmartPtr<V4l2SubDevice> _isp_core_dev;
    bool _first_trigger;
    Mutex _mipi_trigger_mutex;
    SafeQueue<int> _msg_queue;
    PollCallback* _PollCallback;
    bool     _is_1608_sensor;
    int8_t dumpRkRawType;
#ifdef UseCaptureRawData
    CaptureRawData* _rawCap;
#else
    DumpRkRaw* _rawCap;
#endif
};

class RawProcThread
    : public Thread
{
public:
    RawProcThread (RawStreamProcUnit *handle)
        : Thread ("RawProcThread")
        , _handle (handle)
    {}

protected:
    virtual bool loop () {
        return  _handle->raw_buffer_proc ();
    }

private:
    RawStreamProcUnit *_handle;
};

}
#endif


