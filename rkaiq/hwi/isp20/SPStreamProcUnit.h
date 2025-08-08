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
#ifndef _SP_STREAM_PROC_UNIT_H_
#define _SP_STREAM_PROC_UNIT_H_
#include <v4l2_device.h>
#include "poll_thread.h"
#include "xcam_mutex.h"
#include "TnrStatsStream.h"
#include "af_head.h"

using namespace XCam;

namespace RkCam {

enum rkisp_isp_ver {
    ISP_V10 = 0x0,
    ISP_V11 = 0x1,
    ISP_V12 = 0x2,
    ISP_V13 = 0x3,
    ISP_V20 = 0x4,
    ISP_V21 = 0x5,
    ISP_V30 = 0x6,
    ISP_V32 = 0x7,
};

class SPImagBufferProxy : public SubV4l2BufferProxy
{
    public:
    explicit SPImagBufferProxy(SmartPtr<V4l2Buffer> &buf, SmartPtr<V4l2Device> &device)
             :SubV4l2BufferProxy(buf, device)
    {
    }
    virtual ~SPImagBufferProxy() {}
    virtual uint8_t *map ()
    {
        return (uint8_t *)get_v4l2_planar_userptr(0);
    }
    virtual bool unmap ()
    {
        return true;
    }
    int set_buff_fd(int fd)
    {
        _buff_fd = fd;
        return 0;
    }
protected:
    XCAM_DEAD_COPY (SPImagBufferProxy);
};

class CamHwIsp20;
class SPStreamProcUnit       : public RKStream, public PollCallback
{
public:
    explicit SPStreamProcUnit (SmartPtr<V4l2Device> isp_sp_dev, int type, int isp_ver);
    virtual ~SPStreamProcUnit ();
    virtual void start ();
    virtual void stop ();
    virtual SmartPtr<VideoBuffer> new_video_buffer(SmartPtr<V4l2Buffer> buf, SmartPtr<V4l2Device> dev);
    XCamReturn prepare (CalibDbV2_Af_LdgParam_t *ldg_param, CalibDbV2_Af_HighLightParam_t *highlight, int width = 0, int height = 0, int stride = 0);
    void set_devices (CamHwIsp20* camHw, SmartPtr<V4l2SubDevice> isp_core_dev, SmartPtr<V4l2SubDevice> ispp_dev, SmartPtr<V4l2SubDevice> lensdev);
    XCamReturn get_sp_resolution(int &width, int &height, int &aligned_w, int &aligned_h);
    // from PollCallback
    virtual XCamReturn poll_buffer_ready (SmartPtr<VideoBuffer> &buf, int type) { return XCAM_RETURN_ERROR_FAILED; }
    virtual XCamReturn poll_buffer_failed (int64_t timestamp, const char *msg) { return XCAM_RETURN_ERROR_FAILED; }
    virtual XCamReturn poll_buffer_ready (SmartPtr<VideoBuffer> &buf);
    virtual XCamReturn poll_buffer_ready (SmartPtr<V4l2BufferProxy> &buf, int dev_index) { return XCAM_RETURN_ERROR_FAILED; }
    virtual XCamReturn poll_event_ready (uint32_t sequence, int type) { return XCAM_RETURN_ERROR_FAILED; }
    virtual XCamReturn poll_event_failed (int64_t timestamp, const char *msg) { return XCAM_RETURN_ERROR_FAILED; }
    void update_af_meas_params(rk_aiq_isp_af_meas_t *sp_meas);
    int get_lowpass_fv(uint32_t sequence, SmartPtr<V4l2BufferProxy> buf_proxy);
    void set_af_img_size(int w, int h, int w_align, int h_align);
protected:
    XCAM_DEAD_COPY (SPStreamProcUnit);
    bool init_fbcbuf_fd();
    bool deinit_fbcbuf_fd();
    int get_fd_by_index(int index);
protected:
    CamHwIsp20* _camHw;
    SmartPtr<V4l2SubDevice>  _isp_core_dev;
    SmartPtr<V4l2SubDevice> _ispp_dev;
    SmartPtr<V4l2SubDevice> _focus_dev;

    int _ds_width;
    int _ds_height;
    int _ds_width_align;
    int _ds_height_align;
    int _src_width;
    int _src_height;
    int img_ds_size_x;
    int img_ds_size_y;

    std::map<int, int> _buf_fd_map;
    bool _first;
    //SmartPtr<TnrStatsStream> _tnr_stream_unit;
    SmartPtr<SubVideoBuffer> _ispgain;

    int _isp_ver;
    XCam::Mutex _afmeas_param_mutex;
    unsigned char ldg_enable;
    uint8_t *pAfTmp;
    uint32_t sub_shp4_4[RKAIQ_RAWAF_ROI_SUBWINS_NUM];
    uint32_t sub_shp8_8[RKAIQ_RAWAF_ROI_SUBWINS_NUM];
    uint32_t high_light[RKAIQ_RAWAF_ROI_SUBWINS_NUM];
    uint32_t high_light2[RKAIQ_RAWAF_ROI_SUBWINS_NUM];
    rk_aiq_af_algo_meas_t _af_meas_params;
    rk_aiq_lens_descriptor _lens_des;
    int af_img_width;
    int af_img_height;
    int af_img_width_align;
    int af_img_height_align;
};

}
#endif

