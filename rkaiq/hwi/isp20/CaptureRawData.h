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
#ifndef _CAPTURE_RAW_DATA_H_
#define _CAPTURE_RAW_DATA_H_

#include "rk_aiq_pool.h"
#include "xcam_mutex.h"
#include "v4l2_buffer_proxy.h"
#include "rk_isp20_hw.h"
#include "v4l2_device.h"
using namespace XCam;

namespace RkCam {

#define CAPTURERAW_SUBM (0x80)

struct capture_fmt {
    u32 fourcc;
    u8 bayer_fmt;
    u8 pcpp;
    u8 bpp[VIDEO_MAX_PLANES];
};
class CaptureRawData
{
public:
    virtual ~CaptureRawData     ();
    CaptureRawData(const CaptureRawData&)=delete;
    CaptureRawData& operator=(const CaptureRawData&)=delete;
    explicit CaptureRawData(int32_t camId);
    static CaptureRawData& getInstance() {
        static CaptureRawData instance;
        return instance;
    }
    int detect_capture_raw_status(uint32_t sequence, bool first_trigger);
    int update_capture_raw_status(bool first_trigger);
    int dynamic_capture_raw(int i, uint32_t sequence,
                            SmartPtr<V4l2BufferProxy> buf_proxy,
                            SmartPtr<V4l2Buffer> &v4l2buf,
                            int mipi_dev_max,
                            int working_mode,
                            SmartPtr<V4l2Device>  dev);
    XCamReturn capture_raw_ctl(capture_raw_t type, int count = 0, const char* capture_dir = nullptr, char* output_dir = nullptr);
    XCamReturn notify_capture_raw();
    void save_metadata_and_register(
        uint32_t frameId,
        rkisp_effect_params_v20& ispParams,
        SmartPtr<RkAiqSensorExpParamsProxy>& expParams,
        SmartPtr<RkAiqAfInfoProxy>& afParams,
        int working_mode
    );
    bool is_need_save_metadata_and_register() {
        return _capture_metadata_num > 0 ? true : false;
    }
private:
    CaptureRawData ();
    int calculate_stride_per_line(const struct capture_fmt& fmt,
                                  uint32_t& bytesPerLine, SmartPtr<V4l2Device>  dev);
    const struct capture_fmt* find_fmt(const uint32_t pixelformat);
    XCamReturn creat_raw_dir(const char* path);
    XCamReturn write_frame_header_to_raw(FILE* fp,
                                         int dev_index, int sequence, int working_mode, SmartPtr<V4l2Device>  dev);
    XCamReturn write_raw_to_file(FILE* fp, int dev_index,
                                 int sequence, void* userptr, int size);
    void write_reg_to_file(uint32_t base_addr, uint32_t offset_addr,
                           int len, int sequence);
    void write_metadata_to_file(const char* dir_path, uint32_t frame_id,
                                rkisp_effect_params_v20& ispParams,
                                SmartPtr<RkAiqSensorExpParamsProxy>& expParams,
                                SmartPtr<RkAiqAfInfoProxy>& afParams,
                                int working_mode);
    bool get_value_from_file(const char* path, int& value, uint32_t& frameId);
    bool set_value_to_file(const char* path, int value, uint32_t sequence = 0);

private:
    uint32_t sns_width;
    uint32_t sns_height;
    uint32_t pixelformat;
    uint32_t _stride_perline;
    char raw_dir_path[64]{0};
    char user_set_raw_dir[64]{0};
    bool _is_raw_dir_exist;
    bool _is_capture_raw;
    sint32_t _capture_raw_num;
    sint32_t _capture_metadata_num;
    static const struct capture_fmt csirx_fmts[];
    Mutex _capture_image_mutex;
    Cond _capture_image_cond;
    capture_raw_t _capture_raw_type;
    int32_t _camId;
};


}
#endif
