/*
 * Isp20PollThread.h - isp20 poll thread for event and buffer
 *
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
#ifndef _ISP20_POLL_THREAD_H_
#define _ISP20_POLL_THREAD_H_

#include "xcam_thread.h"
#include "poll_thread.h"
#include "SensorHw.h"
#include "LensHw.h"
#include "CamHwIsp20.h"

using namespace XCam;

namespace RkCam {


#define ISP20POLL_SUBM (0x2)

/*
 * @fourcc: pixel format
 * @bayer_fmt: custom bayer format value
 * @pcpp: pixels constraints each packet in mipi-csi2
 * @bpp: bits per pixel
 */
struct capture_fmt {
    u32 fourcc;
    u8 bayer_fmt;
    u8 pcpp;
    u8 bpp[VIDEO_MAX_PLANES];
};

class Isp20PollThread
    : public PollThread {
    friend class MipiPollThread;
public:
    explicit Isp20PollThread();
    virtual ~Isp20PollThread();

    bool set_event_handle_dev(SmartPtr<BaseSensorHw> &dev);
    bool set_iris_handle_dev(SmartPtr<LensHw> &dev);
    bool set_focus_handle_dev(SmartPtr<LensHw> &dev);
    bool setCamHw(ICamHw *dev);
    void set_mipi_devs(SmartPtr<V4l2Device> mipi_tx_devs[3],
                       SmartPtr<V4l2Device> mipi_rx_devs[3],
                       SmartPtr<V4l2SubDevice> isp_dev);
    void set_hdr_frame_readback_infos(rk_aiq_luma_params_t luma_params);
    void set_need_luma_rd_info(bool need) { _need_luma_rd_info = need; }
    XCamReturn notify_capture_raw();
    // should be called befor start
    void set_working_mode(int mode, bool linked_to_isp, bool nordbk);
    XCamReturn capture_raw_ctl(capture_raw_t type, int count = 0, const char* capture_dir = nullptr, char* output_dir = nullptr);
    void set_hdr_global_tmo_mode(int frame_id, bool mode);
    virtual XCamReturn start();
    virtual XCamReturn stop ();
    void setMulCamConc(bool cc) {
        _is_multi_cam_conc = cc;
    }
    void skip_frames(int skip_num, int32_t skip_seq);
    XCamReturn getEffectiveLumaParams(int frame_id, rk_aiq_luma_params_t& luma_params);
    enum {
        ISP_POLL_MIPI_TX,
        ISP_POLL_MIPI_RX,
        ISP_POLL_MIPI_MAX,
    };
    enum {
        ISP_MIPI_HDR_S = 0,
        ISP_MIPI_HDR_M,
        ISP_MIPI_HDR_L,
        ISP_MIPI_HDR_MAX,
    };
    static const char* mipi_poll_type_to_str[ISP_POLL_MIPI_MAX];
protected:
    SmartPtr<VideoBuffer> new_video_buffer(SmartPtr<V4l2Buffer> buf,
                                           SmartPtr<V4l2Device> dev,
                                           int type);
    virtual XCamReturn notify_sof (uint64_t time, int frameid);
private:
    XCamReturn mipi_poll_buffer_loop (int type, int dev_index);
    XCamReturn create_stop_fds_mipi ();
    void destroy_stop_fds_mipi ();
    void handle_rx_buf(SmartPtr<V4l2BufferProxy> &rx_buf, int dev_index);
    void handle_tx_buf(SmartPtr<V4l2BufferProxy> &tx_buf, int dev_index);
    void sync_tx_buf();
    void trigger_readback ();
    typedef struct isp_mipi_dev_info_s {
        SmartPtr<V4l2Device>  dev;
        SmartPtr<Thread>      loop;
        SafeList<V4l2BufferProxy> buf_list;
        SafeList<V4l2BufferProxy> cache_list;
        int stop_fds[2];
    } isp_mipi_dev_info_t;
    SmartPtr<V4l2SubDevice> _isp_core_dev;
    isp_mipi_dev_info_t _isp_mipi_tx_infos[3];
    isp_mipi_dev_info_t _isp_mipi_rx_infos[3];
    std::map<uint32_t, rk_aiq_luma_params_t> _isp_hdr_fid2times_map;
    std::map<uint32_t, bool> _isp_hdr_fid2ready_map;
    int _working_mode;
    bool _linked_to_isp;
    int _mipi_dev_max;
    Mutex _mipi_buf_mutex;
    Mutex _mipi_trigger_mutex;
    bool _first_trigger;
    bool _cache_tx_data;
private:
    XCAM_DEAD_COPY(Isp20PollThread);
    SmartPtr<BaseSensorHw> _event_handle_dev;
    SmartPtr<LensHw> _iris_handle_dev;
    SmartPtr<LensHw> _focus_handle_dev;
    ICamHw *mCamHw;
    uint32_t sns_width;
    uint32_t sns_height;
    uint32_t pixelformat;
    uint32_t _stride_perline;
    char raw_dir_path[64];
    char user_set_raw_dir[64];
    bool _is_raw_dir_exist;
    bool _is_capture_raw;
    sint32_t _capture_raw_num;
    sint32_t _capture_metadata_num;
    static const struct capture_fmt csirx_fmts[];
    Mutex _capture_image_mutex;
    Cond _capture_image_cond;
    capture_raw_t _capture_raw_type;
    std::map<uint32_t, bool> _hdr_global_tmo_state_map;
    std::map<sint32_t, uint64_t> _sof_timestamp_map;
    Mutex _sof_map_mutex;
    bool _is_multi_cam_conc;
    int _skip_num;
    int64_t _skip_to_seq;
    bool _need_luma_rd_info;
    int calculate_stride_per_line(const struct capture_fmt& fmt,
                                  uint32_t& bytesPerLine);
    const struct capture_fmt* find_fmt(const uint32_t pixelformat);
    XCamReturn creat_raw_dir(const char* path);
    XCamReturn write_frame_header_to_raw(FILE* fp,
                                         int dev_index, int sequence);
    XCamReturn write_raw_to_file(FILE* fp, int dev_index,
                                 int sequence, void* userptr, int size);
    void write_reg_to_file(uint32_t base_addr, uint32_t offset_addr,
                           int len, int sequence);
    void write_metadata_to_file(const char* dir_path,
            int frame_id,
            rkisp_effect_params_v20& ispParams,
            SmartPtr<RkAiqExpParamsProxy>& expParams,
            SmartPtr<RkAiqAfInfoProxy>& afParams);
    bool get_value_from_file(const char* path, int& value, uint32_t& frameId);
    bool set_value_to_file(const char* path, int value, uint32_t sequence = 0);
    int detect_capture_raw_status(const char* file_name, uint32_t sequence);
    int update_capture_raw_status(const char* file_name);
    int dynamic_capture_raw(int i, uint32_t sequence,
                            SmartPtr<V4l2BufferProxy> buf_proxy,
                            SmartPtr<V4l2Buffer> &v4l2buf);
    void match_lumadetect_map(uint32_t sequence, sint32_t &additional_times);
    void match_globaltmostate_map(uint32_t sequence, bool &isHdrGlobalTmo);
    bool check_skip_frame(int32_t skip_seq);
    XCamReturn match_sof_timestamp_map(sint32_t sequence, uint64_t &timestamp);
};

}
#endif
