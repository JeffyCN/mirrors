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
        if (_settings.empty())
            return NULL;
        return *_settings.begin();
    }

    // only called one time in the func rkisp_cl_prepare@rkisp_control_loop_impl.cpp
    void set_static_metadata(const camera_metadata_t *metas) { staticMeta = metas; };
    static CameraMetadata& get_static_metadata() { return staticMeta; };

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
    SettingsProcessor*            _settingsProcessor;
    static CameraMetadata staticMeta;

    const cl_result_callback_ops_t *mCallbackOps;

};

#endif  //XCAMSRC_MAIN_DEV_MANAGER_H
