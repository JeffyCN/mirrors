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
#include "xcam_thread.h"
#include "smartptr.h"
#include "safe_list.h"
#include "v4l2_device.h"
#include "poll_thread.h"
#include "xcam_log.h"
#include "Stream.h"
#include "RawStreamProcUnit.h"
#include "rk_aiq_offline_raw.h"
#include "rkrawstream_user_api.h"
#include "MediaInfo.h"

using namespace XCam;

namespace RkRawStream {

struct _live_rkraw_buf
{
    struct _block_header _header;
    struct _st_addrinfo_stream _addr;

}__attribute__ ((packed));

struct _live_rkraw_data
{
    unsigned short tag;
    struct _raw_format _format;
    struct _live_rkraw_buf normal_buf;
    struct _live_rkraw_buf hdr_buf0;
    struct _live_rkraw_buf hdr_buf1;
    struct _live_rkraw_buf hdr_buf2;
    rk_aiq_frame_info_t _finfo;
    unsigned short end;
}__attribute__ ((packed));

void fill_rkraw2_plane_info(rkraw2_plane *p, SmartPtr<V4l2BufferProxy> &buf);

class RawStreamCapUnit : public PollCallback
{
public:
    explicit RawStreamCapUnit(char *dev0, char *dev1, char *dev2);
    explicit RawStreamCapUnit(const rk_sensor_full_info_t *s_info);
    virtual ~RawStreamCapUnit();
    virtual XCamReturn start();
    virtual XCamReturn stop ();
    virtual XCamReturn stop_device ();
    virtual XCamReturn release_buffer ();
	XCamReturn prepare(uint8_t buf_memory_type, uint8_t buf_cnt);
    void set_tx_format(uint32_t width, uint32_t heigh, uint32_t pix_fmt, int mode);
    void set_sensor_format(uint32_t width, uint32_t height, uint32_t pix_fmt);
    void set_sensor_mode(uint32_t mode);
	void set_devices(RawStreamProcUnit *proc);
	void set_working_mode(int mode);
	void release_user_taked_buf(int dev_index);
    void set_dma_buf(int dev_index, int buf_index, int fd);
    void prepare_cif_mipi();
    // from PollCallback
    virtual XCamReturn poll_buffer_ready (SmartPtr<VideoBuffer> &buf, int type) { return XCAM_RETURN_ERROR_FAILED; }
    virtual XCamReturn poll_buffer_failed (int64_t timestamp, const char *msg) { return XCAM_RETURN_ERROR_FAILED; }
    virtual XCamReturn poll_buffer_ready (SmartPtr<VideoBuffer> &buf) { return XCAM_RETURN_ERROR_FAILED; }
    virtual XCamReturn poll_buffer_ready (SmartPtr<V4l2BufferProxy> &buf, int dev_index);
    virtual XCamReturn poll_event_ready (uint32_t sequence, int type) { return XCAM_RETURN_ERROR_FAILED; }
    virtual XCamReturn poll_event_failed (int64_t timestamp, const char *msg) { return XCAM_RETURN_ERROR_FAILED; }

	rkrawstream_vicap_cb_t user_on_frame_capture_cb;
    void *user_priv_data;
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
	SmartPtr<V4l2Device> _dev[3];
	bool user_takes_buf;
protected:
	XCamReturn sync_raw_buf(SmartPtr<V4l2BufferProxy> &buf_s, SmartPtr<V4l2BufferProxy> &buf_m, SmartPtr<V4l2BufferProxy> &buf_l);
	XCamReturn do_capture_callback(SmartPtr<V4l2BufferProxy> &buf_s, SmartPtr<V4l2BufferProxy> &buf_m, SmartPtr<V4l2BufferProxy> &buf_l);
    SmartPtr<V4l2Device> _dev_bakup[3];
    int _dev_index[3];
	SmartPtr<RKStream> _stream[3];
    SmartPtr<V4l2SubDevice> _sensor_dev;
    int _working_mode;
    int _mipi_dev_max;
    int _skip_num;
    Mutex _mipi_mutex;
	Mutex _buf_mutex;
    struct v4l2_format _format;
    enum RawCapState _state;
	SafeList<V4l2BufferProxy> buf_list[3];
	SafeList<V4l2BufferProxy> user_used_buf_list[3];
	RawStreamProcUnit *_proc_stream;
    //struct _live_rkraw_data _rkraw_data[STREAM_VIPCAP_BUF_NUM];
    char _sns_name[32];
    bool is_multi_isp_mode;
    enum v4l2_memory    _memory_type;
    int _buffer_count;
};

}
#endif

