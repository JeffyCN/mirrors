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
#ifndef _CIF_SCALE_STREAM_H_
#define _CIF_SCALE_STREAM_H_
#include <map>
#include <v4l2_device.h>
#include "poll_thread.h"
#include "xcam_mutex.h"
#include "Stream.h"
#include "smart_buffer_priv.h"
using namespace XCam;

namespace RkCam {

class CifSclStream
{
public:
    explicit CifSclStream();
    virtual ~CifSclStream();
    XCamReturn start();
    XCamReturn stop ();
    void set_working_mode(int mode);
    XCamReturn set_format(const struct v4l2_subdev_format& sns_sd_fmt, uint32_t sns_v4l_pix_fmt, int bpp);
    XCamReturn set_format(const struct v4l2_subdev_selection& sns_sd_sel, uint32_t sns_v4l_pix_fmt, int bpp);
    XCamReturn prepare();
    XCamReturn restart(const rk_sensor_full_info_t *s_info, int ratio, PollCallback *callback, int mode);
    XCamReturn init(const rk_sensor_full_info_t *s_info, PollCallback *callback);
    XCamReturn set_ratio_fmt(int ratio);
    virtual bool setPollCallback (PollCallback *callback);
    void set_bpp(int bpp);
    bool getIsActive();
protected:
    XCAM_DEAD_COPY (CifSclStream);
private:
    SmartPtr<V4l2Device>  _dev[3];
    SmartPtr<RKStream> _stream[3];
    int _working_mode;
    int _bpp;
    int index;
    uint32_t _width;
    uint32_t _height;
    uint32_t _sns_v4l_pix_fmt;
    int _ratio;
    bool _first_start;
    bool _active{false};
    bool _init{false};
};

}
#endif