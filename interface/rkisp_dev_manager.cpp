/*
 * main_dev_manager.cpp - main device manager
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

#include "rkisp_dev_manager.h"
#include "settings_processor.h"
#include <base/log.h>

using namespace XCam;

RkispDeviceManager::RkispDeviceManager(const cl_result_callback_ops_t *cb)
    : mCallbackOps (cb)
{
    _settingsProcessor = new SettingsProcessor();
    _settings.clear();
}

RkispDeviceManager::~RkispDeviceManager()
{
    if(_settingsProcessor)
        delete _settingsProcessor;
    _settings.clear();
}

void
RkispDeviceManager::handle_message (const SmartPtr<XCamMessage> &msg)
{
    XCAM_UNUSED (msg);
}

void
RkispDeviceManager::handle_buffer (const SmartPtr<VideoBuffer> &buf)
{
    XCAM_ASSERT (buf.ptr ());
    _ready_buffers.push (buf);
}

SmartPtr<VideoBuffer>
RkispDeviceManager::dequeue_buffer ()
{
    SmartPtr<VideoBuffer> ret;
    ret = _ready_buffers.pop (-1);
    return ret;
}

void
RkispDeviceManager::x3a_calculation_done (XAnalyzer *analyzer, X3aResultList &results)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    SmartPtr<XmetaResult> meta_result;
    X3aResultList::iterator iter;
    for (iter = results.begin ();
            iter != results.end (); ++iter)
    {
        if ((*iter)->get_type() == XCAM_3A_METADATA_RESULT_TYPE) {
            meta_result = (*iter).dynamic_cast_ptr<XmetaResult> ();
            break ;
        }
    }
    if (iter == results.end()) {
        LOGW("@%s %d: There is no meta result, Check it !!!", __FUNCTION__, __LINE__);
        goto done;
    }

    /* meta_result->dump(); */
    int id;
    {
        SmartLock lock(_settingsMutex);
        if (!_settings.empty()) {
            id = (*_settings.begin())->reqId;
            _settings.erase(_settings.begin());
        } else {
            LOGE("@%s %d: No settting when results comes, shoult not happen , Fix me!!", __FUNCTION__, __LINE__);
            goto done;
        }
    }
    LOGI("@%s %d: result %d has %d metadata entries", __FUNCTION__, __LINE__,
         id, meta_result->get_metadata_result()->entryCount());

    rkisp_cl_frame_metadata_s cb_result;
    cb_result.id = id;
    cb_result.metas = meta_result->get_metadata_result()->getAndLock();
    mCallbackOps->metadata_result_callback(mCallbackOps, &cb_result);
    meta_result->get_metadata_result()->unlock(cb_result.metas);

done:
    DeviceManager::x3a_calculation_done (analyzer, results);
}

XCamReturn
RkispDeviceManager::set_control_params(const int request_frame_id,
                              const camera_metadata_t *metas)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    XCam::SmartPtr<AiqInputParams> inputParams = new AiqInputParams();
    inputParams->reqId = request_frame_id;
    inputParams->settings = metas;
    inputParams->staticMeta = &RkispDeviceManager::staticMeta;
    if(_settingsProcessor) {
        _settingsProcessor->processRequestSettings(inputParams->settings, *inputParams.ptr());
    } else {
        LOGE("@%s %d: _settingsProcessor is null , is a bug, fix me", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_UNKNOWN;
    }
    XCamAeParam aeparams = inputParams->aeInputParams.aeParams;
    AeControls aectl = inputParams->aaaControls.ae;
    LOGI("@%s %d: aeparms: mode-%d, metering_mode-%d, flicker_mode-%d, ex_min-%d, ex_max-%d, manual_exp-%d, manual_gain-%d, aeControls: mode-%d, lock-%d, preTrigger-%d, antibanding-%d, evCompensation-%d, fpsrange[%d, %d]", __FUNCTION__, __LINE__,
         aeparams.mode, aeparams.metering_mode, aeparams.flicker_mode, aeparams.exposure_time_min, aeparams.exposure_time_max, aeparams.manual_exposure_time, aeparams.manual_analog_gain, 
         aectl.aeMode, aectl.aeLock, aectl.aePreCaptureTrigger, aectl.aeAntibanding, aectl.evCompensation, aectl.aeTargetFpsRange[0], aectl.aeTargetFpsRange[1]);
    {
        SmartLock lock(_settingsMutex);
        _settings.push_back(inputParams);
    }
    return ret;
}

void
RkispDeviceManager::pause_dequeue ()
{
    return _ready_buffers.pause_pop ();
}

void
RkispDeviceManager::resume_dequeue ()
{
    return _ready_buffers.resume_pop ();
}
