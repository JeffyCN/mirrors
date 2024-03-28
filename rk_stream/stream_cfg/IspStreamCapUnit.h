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
#ifndef _ISP_STREAM_CAP_UNIT_H_
#define _ISP_STREAM_CAP_UNIT_H_

#include <map>
#include <v4l2_device.h>
#include "xcam_thread.h"
#include "smartptr.h"
#include "safe_list.h"
#include "v4l2_device.h"
#include "poll_thread.h"
#include "xcam_log.h"
#include "Stream.h"
#include "rkrawstream_user_api.h"
#include "RawStreamCapUnit.h"
#include "IspStreamCapUnit.h"
#include "MediaInfo.h"

using namespace XCam;

namespace RkRawStream {

class IspStreamCapUnit : public PollCallback
{
public:
    explicit IspStreamCapUnit(char *dev0, char *dev1, char *dev2);
    explicit IspStreamCapUnit(const rk_sensor_full_info_t *s_info);
    virtual ~IspStreamCapUnit();
    virtual XCamReturn start();
    virtual XCamReturn stop ();
    //virtual XCamReturn stop_device ();
    //virtual XCamReturn release_buffer ();
	XCamReturn prepare(uint8_t buf_memory_type, uint8_t buf_cnt);
    void set_isp_format(uint32_t width, uint32_t height);

	void release_user_taked_buf(int dev_index);
    void set_dma_buf(int dev_index, int buf_index, int fd);
    // from PollCallback
    virtual XCamReturn poll_buffer_ready (SmartPtr<VideoBuffer> &buf, int type) { return XCAM_RETURN_ERROR_FAILED; }
    virtual XCamReturn poll_buffer_failed (int64_t timestamp, const char *msg) { return XCAM_RETURN_ERROR_FAILED; }
    virtual XCamReturn poll_buffer_ready (SmartPtr<VideoBuffer> &buf) { return XCAM_RETURN_ERROR_FAILED; }
    virtual XCamReturn poll_buffer_ready (SmartPtr<V4l2BufferProxy> &buf, int dev_index);
    virtual XCamReturn poll_event_ready (uint32_t sequence, int type) { return XCAM_RETURN_ERROR_FAILED; }
    virtual XCamReturn poll_event_failed (int64_t timestamp, const char *msg) { return XCAM_RETURN_ERROR_FAILED; }

	rkrawstream_isp_cb_t user_on_frame_capture_cb;
    enum CapState {
        CAP_STATE_INVALID,
        CAP_STATE_INITED,
        CAP_STATE_PREPARED,
        CAP_STATE_STARTED,
        CAP_STATE_STOPPED,
    };
	SmartPtr<V4l2Device> _dev[3];
    void *user_priv_data;
	bool user_takes_buf;
protected:
	XCamReturn do_capture_callback(SmartPtr<V4l2BufferProxy> &buf);
    int _working_mode;
    int _mipi_dev_max;
    int _dev_index[3];
	SmartPtr<RKStream> _stream[3];
    //SmartPtr<V4l2SubDevice> _sensor_dev;
	Mutex _buf_mutex;
    struct v4l2_format _format;
    enum CapState _state;
	SafeList<V4l2BufferProxy> buf_list[3];
	SafeList<V4l2BufferProxy> user_used_buf_list[3];

    char _sns_name[32];
    enum v4l2_memory    _memory_type;
    int _buffer_count;
};

}
#endif

