/*
 * Copyright (C) 2015-2017 Intel Corporation
 * Copyright (c) 2017, Fuzhou Rockchip Electronics Co., Ltd
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
 */

#define LOG_TAG "awb_state_machine"

#include "awb_state_machine.h"
#include "Metadata2Str.h"

namespace XCam {
RkAWBStateMachine::RkAWBStateMachine():
        mLastControlMode(0),
        mCurrentAwbState(0),
        mCurrentAwbMode(nullptr)
{
    mCurrentAwbMode = &mAutoMode;
    memset(&mLastAwbControls, 0, sizeof(mLastAwbControls));
    mLastAwbControls.awbMode = ANDROID_CONTROL_AWB_MODE_AUTO;
}

RkAWBStateMachine::~RkAWBStateMachine()
{
}

XCamReturn
RkAWBStateMachine::processState(const uint8_t &controlMode,
                                   const AwbControls &awbControls)
{
    XCamReturn ret;

    if (controlMode == ANDROID_CONTROL_MODE_OFF) {
        mCurrentAwbMode = &mOffMode;

        if (controlMode != mLastControlMode)
            LOGI("%s: Set AWB offMode: controlMode = %s, awbMode = %s",
                        __FUNCTION__,
                        META_CONTROL2STR(mode, controlMode),
                        META_CONTROL2STR(awbMode, awbControls.awbMode));
    } else {
        if (awbControls.awbMode == ANDROID_CONTROL_AWB_MODE_OFF) {
            mCurrentAwbMode = &mOffMode;
            if (awbControls.awbMode != mLastAwbControls.awbMode)
                LOGI("%s: Set AWB offMode: controlMode = %s, awbMode = %s",
                                __FUNCTION__,
                                META_CONTROL2STR(mode, controlMode),
                                META_CONTROL2STR(awbMode, awbControls.awbMode));
        } else {
            mCurrentAwbMode = &mAutoMode;
            if (awbControls.awbMode != mLastAwbControls.awbMode)
                LOGI("%s: Set AWB offMode: controlMode = %s, awbMode = %s",
                                __FUNCTION__,
                                META_CONTROL2STR(mode, controlMode),
                                META_CONTROL2STR(awbMode, awbControls.awbMode));
        }
    }

    mLastAwbControls = awbControls;
    mLastControlMode = controlMode;
    ret = mCurrentAwbMode->processState(controlMode, awbControls);
    return ret;
}

XCamReturn
RkAWBStateMachine::processResult(const rk_aiq_awb_results &awbResults,
                                    CameraMetadata &result)
{
    XCamReturn ret;

    if (CC_UNLIKELY(mCurrentAwbMode == nullptr)) {
        LOGE("Invalid AWB mode - this could not happen - BUG!");
        return XCAM_RETURN_ERROR_UNKNOWN;
    }

    ret =  mCurrentAwbMode->processResult(awbResults, result);
    return ret;
}

/******************************************************************************
 * AWB MODE   -  BASE
 ******************************************************************************/
RkAWBModeBase::RkAWBModeBase():
        mLastControlMode(0),
        mCurrentAwbState(ANDROID_CONTROL_AWB_STATE_INACTIVE)
{
    memset(&mLastAwbControls, 0, sizeof(mLastAwbControls));
}

void
RkAWBModeBase::updateResult(CameraMetadata& results)
{

    LOGI("%s: current AWB state is: %s", __FUNCTION__,
         META_CONTROL2STR(awbState, mCurrentAwbState));

    //# METADATA_Dynamic control.awbMode done
    results.update(ANDROID_CONTROL_AWB_MODE, &mLastAwbControls.awbMode, 1);
    //# METADATA_Dynamic control.awbLock done
    results.update(ANDROID_CONTROL_AWB_LOCK, &mLastAwbControls.awbLock, 1);
    //# METADATA_Dynamic control.awbState done
    results.update(ANDROID_CONTROL_AWB_STATE, &mCurrentAwbState, 1);
}

void
RkAWBModeBase::resetState()
{
    mCurrentAwbState = ANDROID_CONTROL_AWB_STATE_INACTIVE;
}


/******************************************************************************
 * AWB MODE   -  OFF
 ******************************************************************************/

RkAWBModeOff::RkAWBModeOff():RkAWBModeBase()
{
}

XCamReturn
RkAWBModeOff::processState(const uint8_t &controlMode,
                              const AwbControls &awbControls)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mLastAwbControls = awbControls;
    mLastControlMode = controlMode;

    if (controlMode == ANDROID_CONTROL_MODE_OFF ||
        awbControls.awbMode == ANDROID_CONTROL_AWB_MODE_OFF) {
        resetState();
    } else {
        LOGE("AWB State machine should not be OFF! - Fix bug");
        ret = XCAM_RETURN_ERROR_UNKNOWN;
    }

    return ret;
}

XCamReturn
RkAWBModeOff::processResult(const rk_aiq_awb_results& awbResults,
                               CameraMetadata& result)
{
    /* UNUSED(awbResults); */

    mCurrentAwbState = ANDROID_CONTROL_AWB_STATE_INACTIVE;
    updateResult(result);

    return XCAM_RETURN_NO_ERROR;
}

/******************************************************************************
 * AWB MODE   -  AUTO
 ******************************************************************************/

RkAWBModeAuto::RkAWBModeAuto():RkAWBModeBase()
{
}

XCamReturn
RkAWBModeAuto::processState(const uint8_t &controlMode,
                               const AwbControls &awbControls)
{
    if (controlMode != mLastControlMode) {
        LOGI("%s: control mode has changed %s -> %s, reset AWB State", __FUNCTION__,
                META_CONTROL2STR(mode, mLastControlMode),
                META_CONTROL2STR(mode, controlMode));
        resetState();
    }

    if (awbControls.awbLock == ANDROID_CONTROL_AWB_LOCK_ON) {
        mCurrentAwbState = ANDROID_CONTROL_AWB_STATE_LOCKED;
    } else if (awbControls.awbMode != mLastAwbControls.awbMode) {
        resetState();
    } else {
        switch (mCurrentAwbState) {
            case ANDROID_CONTROL_AWB_STATE_LOCKED:
                mCurrentAwbState = ANDROID_CONTROL_AWB_STATE_INACTIVE;
                break;
            case ANDROID_CONTROL_AWB_STATE_INACTIVE:
            case ANDROID_CONTROL_AWB_STATE_SEARCHING:
            case ANDROID_CONTROL_AWB_STATE_CONVERGED:
                //do nothing
                break;
            default:
                LOGE("Invalid AWB state: %d !, State set to INACTIVE", mCurrentAwbState);
                mCurrentAwbState = ANDROID_CONTROL_AWB_STATE_INACTIVE;
        }
    }
    mLastAwbControls = awbControls;
    mLastControlMode = controlMode;
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAWBModeAuto::processResult(const rk_aiq_awb_results &awbResults,
                                CameraMetadata& result)
{
    switch (mCurrentAwbState) {
        case ANDROID_CONTROL_AWB_STATE_LOCKED:
        //do nothing
            break;
        case ANDROID_CONTROL_AWB_STATE_INACTIVE:
        case ANDROID_CONTROL_AWB_STATE_SEARCHING:
        case ANDROID_CONTROL_AWB_STATE_CONVERGED:
            if (awbResults.converged)
                mCurrentAwbState = ANDROID_CONTROL_AWB_STATE_CONVERGED;
            else
                mCurrentAwbState = ANDROID_CONTROL_AWB_STATE_SEARCHING;
            break;
        default:
            LOGE("invalid AWB state: %d !, State set to INACTIVE", mCurrentAwbState);
            mCurrentAwbState = ANDROID_CONTROL_AWB_STATE_INACTIVE;
    }

    updateResult(result);

    return XCAM_RETURN_NO_ERROR;
}
}

