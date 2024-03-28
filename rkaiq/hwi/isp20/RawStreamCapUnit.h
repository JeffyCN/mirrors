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
#ifndef _RAW_STREAM_CAP_UNIT_H_
#define _RAW_STREAM_CAP_UNIT_H_
#include <map>
#include <v4l2_device.h>
#include "poll_thread.h"
#include "xcam_mutex.h"
#include "Stream.h"
#include "smart_buffer_priv.h"
using namespace XCam;

namespace RkCam {

class RawStreamProcUnit;
class RawStreamCapUnit : public PollCallback
{
public:
    explicit RawStreamCapUnit ();
    explicit RawStreamCapUnit (const rk_sensor_full_info_t *s_info, bool linked_to_isp, int tx_buf_cnt);
    virtual ~RawStreamCapUnit ();
    virtual XCamReturn start(int mode);
    virtual XCamReturn stop ();
    void set_working_mode(int mode);
    void set_devices(SmartPtr<V4l2SubDevice> ispdev, CamHwIsp20* handle, RawStreamProcUnit *proc);
    void set_tx_devices(SmartPtr<V4l2Device> mipi_tx_devs[3]);
    SmartPtr<V4l2Device> get_tx_device (int index);
    XCamReturn set_tx_format(const struct v4l2_subdev_format& sns_sd_fmt, uint32_t sns_v4l_pix_fmt);
    XCamReturn set_tx_format(const struct v4l2_subdev_selection& sns_sd_sel, uint32_t sns_v4l_pix_fmt);
    XCamReturn prepare(int idx);
    void prepare_cif_mipi();
    void skip_frames(int skip_num, int32_t skip_seq);
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
        _is_1608_stream = sensorState;
    }
    XCamReturn reset_hardware();
    XCamReturn set_csi_mem_word_big_align(uint32_t width, uint32_t height, uint32_t sns_v4l_pix_fmt, int8_t sns_bpp);
    XCamReturn setVicapStreamMode(int mode, uint32_t *frameId, bool isSingleMode);

    enum {
        ISP_MIPI_HDR_S = 0,
        ISP_MIPI_HDR_M,
        ISP_MIPI_HDR_L,
        ISP_MIPI_HDR_MAX,
    };
    enum RawCapState {
        RAW_CAP_STATE_INVALID,
        RAW_CAP_STATE_INITED,
        RAW_CAP_STATE_PREPARED,
        RAW_CAP_STATE_STARTED,
        RAW_CAP_STATE_STOPPED,
    };

protected:
    XCAM_DEAD_COPY (RawStreamCapUnit);
    XCamReturn sync_raw_buf(SmartPtr<V4l2BufferProxy> &buf_s, SmartPtr<V4l2BufferProxy> &buf_m, SmartPtr<V4l2BufferProxy> &buf_l);
    bool check_skip_frame(int32_t buf_seq);
    int mCamPhyId{-1};
    bool _is_1608_stream;
protected:
    SmartPtr<V4l2Device> _dev[3];
    SmartPtr<V4l2Device> _dev_bakup[3];
    int _dev_index[3]{0};
    SmartPtr<RKStream> _stream[3];
    Mutex _buf_mutex;
    int _working_mode{0};
    int _mipi_dev_max{1};
    int _skip_num;
    int64_t _skip_to_seq{0};
    Mutex _mipi_mutex;
    enum RawCapState _state;

    SafeList<V4l2BufferProxy> buf_list[3];
    CamHwIsp20* _camHw{NULL};
    SmartPtr<V4l2SubDevice> _isp_core_dev;
    RawStreamProcUnit *_proc_stream{NULL};
    //
    SafeList<V4l2BufferProxy> _NrImg_ready_list;
};

}
#endif

