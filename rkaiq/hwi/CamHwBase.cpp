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

#include "CamHwBase.h"
#include "SensorHw.h"
#include "LensHw.h"

namespace RkCam {

CamHwBase::CamHwBase()
    : mKpHwSt (false)
{
    mHwResLintener = NULL;
    mWorkingMode   = 0;
    mCalibDb       = NULL;
    mCalibDbV2     = NULL;
    mCamPhyId      = -1;
    mIsGroupMode   = false;
    mIsMain        = false;
    xcam_mem_clear (mTbInfo);
    mDevBufCntMap.clear();
}

CamHwBase::~CamHwBase()
{}

XCamReturn
CamHwBase::init(const char* sns_ent_name)
{
    // TODO: new all subdevices and open
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
CamHwBase::deInit()
{
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
CamHwBase::prepare(uint32_t width, uint32_t height, int mode, int t_delay, int g_delay)
{
    // TODO
    // check sensor's output width,height
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
CamHwBase::start()
{
    // TODO
    // subscribe events
    // start devices
    // start pollthread
//    mPollthread->start();
//    mPollLumathread->start();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
CamHwBase::stop()
{
    // TODO
    // unsubscribe events
    // stop pollthread
    // stop devices
//    mPollthread->stop();
//    mPollLumathread->stop();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn CamHwBase::pause()
{
    return XCAM_RETURN_NO_ERROR;
}
XCamReturn CamHwBase::resume()
{
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
CamHwBase::getSensorModeData(const char* sns_ent_name,
                             rk_aiq_exposure_sensor_descriptor& sns_des)
{
    // TODO
    // get from SensorHw

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
CamHwBase::setHwResListener(HwResListener* resListener)
{
    mHwResLintener = resListener;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
CamHwBase::poll_buffer_ready (SmartPtr<VideoBuffer> &buf, int type)
{//not used
    assert(0);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
CamHwBase::poll_buffer_ready (SmartPtr<VideoBuffer> &buf)
{
    if (mHwResLintener) {
        return mHwResLintener->hwResCb(buf);
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
CamHwBase::poll_buffer_failed (int64_t timestamp, const char *msg)
{
    // TODO
    return XCAM_RETURN_ERROR_FAILED;
}

XCamReturn
CamHwBase::poll_event_ready (uint32_t sequence, int type)
{
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
CamHwBase::poll_event_failed (int64_t timestamp, const char *msg)
{
    return XCAM_RETURN_ERROR_FAILED;
}
} //namspace RkCam
