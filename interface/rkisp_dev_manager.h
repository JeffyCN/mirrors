/*
 * main_dev_manager.h - main device manager
 *
 *  Copyright (c) 2015 Intel Corporation
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
 * Author: John Ye <john.ye@intel.com>
 * Author: Wind Yuan <feng.yuan@intel.com>
 */

#ifndef XCAMSRC_RKISP_DEV_MANAGER_H
#define XCAMSRC_RKISP_DEV_MANAGER_H

#include <base/xcam_common.h>
#include <linux/videodev2.h>
#include <stdint.h>
#include <unistd.h>

#include <queue>

#include <xcam_mutex.h>
#include <video_buffer.h>
#include <v4l2_buffer_proxy.h>
#include <v4l2_device.h>
#include <device_manager.h>
#include <base/xcam_params.h>
#if HAVE_RK_IQ
#include <isp/rkisp_device.h>
#include <isp/isp_controller.h>
#include <isp/isp_image_processor.h>
#endif
#if HAVE_LIBCL
#include <ocl/cl_3a_image_processor.h>
#include <ocl/cl_post_image_processor.h>
#endif
#include <x3a_analyzer_simple.h>
#ifdef ANDROID_VERSION_ABOVE_8_X
#include <CameraMetadata.h>
using ::android::hardware::camera::common::V1_0::helper::CameraMetadata;
#else
#include <camera/CameraMetadata.h>
#endif
#include "rkisp_control_loop.h"
#include "x3a_meta_result.h"
#include "rkaiq.h"
#include "isp_controller.h"

/*
 ***************** CAM ENGINE LIB VERSION NOTE *****************
 * v1.2.0
 *  - support both Linux and Android 7.x/8.x/9.x
 *  - support auto/manual AE/AWB/AF
 * v1.3.0
 *  - support rk1608 HDR ae
 *  - remove unnecessary lib dependance 
 * v1.4.0
 *  - unify isp v10/v12 iq files
 *  - remove RKISP_V12 macro, get isp version from
 *    driver in runtime
 * v1.5.0
 *  - fix hist/dpcc check error 
 *  - rkisp_demo support Android 
 * v1.6.0
 *  - fix some bug in last version.
 *    - fix BDM not bypassed actually for BW sensor
 *    - fix bug in dpcc check
 *  - dpcc mode can be defined in tuning xml
 *  - fix memory leak in calibdb IE sharpen
 *  - update calibdb v0.2.1 for HDR ae and v0.2.2
 *    for BW sensor
 *  - add 3A controls sample code in rkisp_demo
 * v1.9.0
 *  - support smooth ae
 *  - support full ispv12 features
 *  - select iq file automatically
 *  - support sensor OTP
 *  - support vendor tags
 *  - iq v0.2.5
 *  - aec lib v0.0.9
 *  - awb lib v0.0.9
 *  - af lib v0.2.10
 * v2.0.0
 *  note: ISP drvier should be updated to v0.1.3
 *  - calibdb v1.0.0 iq: v1.0.0 checksum: 635075
 *    add strict tag level checking for iq xml
 *  - support lsc otp enable control in iq xml
 *  - fix some bugs
 *    fix aec meas window check error
 *    set init focus to driver before sensor streaming
 *    fix bug of mapping hist weights error from 9x9 to 5x5
 *  - rkisp_demo support usr_ptr memory mode, and fix some bugs
 *  - aec lib v0.0.9
 *  - awb lib v0.0.b
 *  - af lib v0.2.14
 *    fix the wrong af trigger when picture taken
 * v2.0.1
 *  - support flashlight and precapture
 *  - ae lib v0.0.a
 *  - calibdb v1.1.0 iq: v1.1.0 checksum: 677941 
 * v2.2.0
 *  - fully support flashlight, pass CTS
 *  - use new log system
 *  - calibdb v1.5.0,magic code: 706729
 *  - aec lib v0.0.e
 *  - awb lib v0.0.e
 *  - af lib v0.2.17
 *  - matched rkisp driver v0.1.5
 */

#define CONFIG_CAM_ENGINE_LIB_VERSION "v2.2.0"

using namespace XCam;
class SettingsProcessor;

class RkispDeviceManager
    : public XCam::DeviceManager
{
public:
    RkispDeviceManager (const cl_result_callback_ops_t *cb);
    ~RkispDeviceManager ();

    XCam::SmartPtr<XCam::VideoBuffer> dequeue_buffer ();
    void pause_dequeue ();
    void resume_dequeue ();
    XCamReturn set_control_params(const int request_frame_id,
                                  const camera_metadata_t *metas);

    XCam::SmartPtr<AiqInputParams> getAiqInputParams()
    {
        SmartLock lock(_settingsMutex);
        // use new setting when no flying settings to make sure
        // same settings used for 3A stats of one frame
        if (!_settings.empty() && _fly_settings.empty()) {
            _cur_settings = *_settings.begin();
            _settings.erase(_settings.begin());
            _fly_settings.push_back(_cur_settings);
        }

        return _cur_settings;
    }

    void set_isp_controller (SmartPtr<IspController> &isp) { _isp_controller = isp; }
    // only called one time in the func rkisp_cl_prepare@rkisp_control_loop_impl.cpp
    void set_static_metadata(const camera_metadata_t *metas) { staticMeta = metas; };
    static CameraMetadata& get_static_metadata() { return staticMeta; };
public:
    int _cl_state;
#if HAVE_LIBCL
public:
    void set_cl_image_processor (XCam::SmartPtr<XCam::CL3aImageProcessor> &processor) {
        _cl_image_processor = processor;
    }

    XCam::SmartPtr<XCam::CL3aImageProcessor> &get_cl_image_processor () {
        return _cl_image_processor;
    }

    void set_cl_post_image_processor (XCam::SmartPtr<XCam::CLPostImageProcessor> &processor) {
        _cl_post_image_processor = processor;
    }

    XCam::SmartPtr<XCam::CLPostImageProcessor> &get_cl_post_image_processor () {
        return _cl_post_image_processor;
    }
#endif

protected:
    virtual void handle_message (const XCam::SmartPtr<XCam::XCamMessage> &msg);
    virtual void handle_buffer (const XCam::SmartPtr<XCam::VideoBuffer> &buf);

    virtual void x3a_calculation_done (XAnalyzer *analyzer, X3aResultList &results);

private:
    XCam::SafeList<XCam::VideoBuffer>         _ready_buffers;
#if HAVE_LIBCL
    XCam::SmartPtr<XCam::CL3aImageProcessor>   _cl_image_processor;
    XCam::SmartPtr<XCam::CLPostImageProcessor> _cl_post_image_processor;
#endif
    Mutex _settingsMutex;
    // push_back when set_control_params, erase when calculationd done
    std::vector<XCam::SmartPtr<AiqInputParams>>  _settings;
    std::vector<XCam::SmartPtr<AiqInputParams>>  _fly_settings;
    XCam::SmartPtr<AiqInputParams>  _cur_settings;
    SettingsProcessor*            _settingsProcessor;
    static CameraMetadata staticMeta;

    const cl_result_callback_ops_t *mCallbackOps;
    SmartPtr<IspController>          _isp_controller;

};

#endif  //XCAMSRC_MAIN_DEV_MANAGER_H
