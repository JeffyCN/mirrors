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

#define LOG_TAG "af_state_machine"

#include <math.h>

#include "af_state_machine.h"
#include "Metadata2Str.h"
#include <sys/time.h>

namespace XCam {
nsecs_t systemTime()
{
    struct timespec t;
    t.tv_sec = t.tv_nsec = 0;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return nsecs_t(t.tv_sec)*1000000000LL + t.tv_nsec;
}
/**
 * AF timeouts. Together these will make:
 * timeout if: [MIN_AF_TIMEOUT - MAX_AF_FRAME_COUNT_TIMEOUT - MAX_AF_TIMEOUT]
 * which translates to 2-4 seconds with the current values. Actual timeout value
 * will depend on the FPS. E.g. >30FPS = 2s, 20FPS = 3s, <15FPS = 4s.
 */

/**
 * MAX_AF_TIMEOUT
 * Maximum time we allow the AF to iterate without a result.
 * This timeout is the last resort, for very low FPS operation.
 * Units are in microseconds.
 * 4 seconds is a compromise between CTS & ITS. ITS allows for 10 seconds for
 * 3A convergence. CTS1 allows only 5, but it doesn't require convergence, just
 * a conclusion. We reserve one second for latencies to be safe. This makes the
 * timeout 5 (cts1) - 1 (latency safety) = 4 seconds = 4000000us.
 */
static const long int MAX_AF_TIMEOUT = 4000000;  // 4 seconds

/**
 * MIN_AF_TIMEOUT
 * For very high FPS use cases, we want to anyway allow some time for moving the
 * lens.
 */
static const long int MIN_AF_TIMEOUT = 2000000;  // 2 seconds

/**
 * MAX_AF_FRAME_COUNT_TIMEOUT
 * Maximum time we allow the AF to iterate without a result.
 * Based on frames, as the AF algorithm itself needs frames for its operation,
 * not just time, and the FPS varies.
 * This is the timeout for normal operation, and translates to 2 seconds
 * if FPS is 30.
 */
static const int MAX_AF_FRAME_COUNT_TIMEOUT = 60;  // 2 seconds if 30fps

RkAFStateMachine::RkAFStateMachine():
        mCurrentAfState(ANDROID_CONTROL_AF_STATE_INACTIVE)
{
    mCurrentAfMode = &mAutoMode;
    mLastAfControls = { ANDROID_CONTROL_AF_MODE_AUTO,
                        ANDROID_CONTROL_AF_TRIGGER_IDLE };
}

RkAFStateMachine::~RkAFStateMachine()
{
}

XCamReturn
RkAFStateMachine::processTriggers(const uint8_t &afTrigger,
                                     const uint8_t &afMode,
                                     int preCaptureId,
                                     XCamAfParam &afInputParams)
{
    if (afMode != mLastAfControls.afMode) {
        LOGI("Change of AF mode from %s to %s",
                META_CONTROL2STR(afMode,mLastAfControls.afMode),
                META_CONTROL2STR(afMode,afMode));
        switch (afMode) {
        case ANDROID_CONTROL_AF_MODE_AUTO:
        case ANDROID_CONTROL_AF_MODE_MACRO:
            mCurrentAfMode = &mAutoMode;
            break;
        case ANDROID_CONTROL_AF_MODE_CONTINUOUS_VIDEO:
        case ANDROID_CONTROL_AF_MODE_CONTINUOUS_PICTURE:
            mCurrentAfMode = &mContinuousPictureMode;
            break;
        case ANDROID_CONTROL_AF_MODE_OFF:
            mCurrentAfMode = &mOffMode;
            break;
        default:
            LOGE("INVALID AF mode requested defaulting to AUTO");
            mCurrentAfMode = &mAutoMode;
            break;
        }
        mCurrentAfMode->resetState();
    }
    mLastAfControls.afTrigger = afTrigger;
    mLastAfControls.afMode = afMode;

    LOGI("%s: afMode %d, afTrigger %d", __FUNCTION__, mLastAfControls.afMode, afTrigger);
    return mCurrentAfMode->processTriggers(afTrigger, afMode, preCaptureId, afInputParams);
}

XCamReturn
RkAFStateMachine::processResult(rk_aiq_af_results &afResults,
                                   XCamAfParam &afInputParams,
                                   CameraMetadata &result)
{
    if (CC_UNLIKELY(mCurrentAfMode == nullptr)) {
        LOGE("Invalid AF mode - this could not happen - BUG!");
        return XCAM_RETURN_NO_ERROR;
    }

    focusDistanceResult(afResults, afInputParams, result);

    return mCurrentAfMode->processResult(afResults, result);
}

/**
 * updateDefaults
 *
 * Used in case of error in the algorithm or fixed focus sensor
 * In case of fixed focus sensor we always report locked
 */
XCamReturn
RkAFStateMachine::updateDefaults(const rk_aiq_af_results& afResults,
                                    const XCamAfParam &afInputParams,
                                    CameraMetadata& result,
                                    bool fixedFocus) const
{
    mCurrentAfMode->updateResult(result);
    uint8_t defaultState = ANDROID_CONTROL_AF_STATE_INACTIVE;
    if (fixedFocus)
        defaultState = ANDROID_CONTROL_AF_STATE_FOCUSED_LOCKED;

    result.update(ANDROID_CONTROL_AF_STATE, &defaultState, 1);

    focusDistanceResult(afResults, afInputParams, result);

    return XCAM_RETURN_NO_ERROR;
}

void
RkAFStateMachine::focusDistanceResult(const rk_aiq_af_results &afResults,
                                         const XCamAfParam &afInputParams,
                                         CameraMetadata &result) const
{
    // "APPROXIMATE and CALIBRATED devices report the focus metadata
    // in units of diopters (1/meter)", so 0.0f represents focusing at infinity."

    float afDistanceDiopters = 1.2;

    if (afInputParams.focus_mode == AF_MODE_INFINITY) {
        // infinity mode is special: we need to report 0.0f (1/inf = 0)
        afDistanceDiopters = 0.0;
    } else if (afResults.current_focus_distance != 0) {
        // In AIQ, 'current_focus_distance' is in millimeters.
        // For rounding multiply by extra 100.
        // This allows the diopters to have 2 decimal values
        afDistanceDiopters = 100 * 1000 * (1.0 / afResults.current_focus_distance);
        afDistanceDiopters = ceil(afDistanceDiopters);

        // Divide by 100 for final result.
        afDistanceDiopters = afDistanceDiopters / 100;
    } else {
        LOGI("Zero focus distance in AF result, reporting %f to app",
                afDistanceDiopters);
    }

    //# METADATA_Dynamic lens.focusDistance Done
    result.update(ANDROID_LENS_FOCUS_DISTANCE, &afDistanceDiopters, 1);

    /* TODO  Focus Range update*/
    /* float dof[2]; */
    /* m3A.calculateDepthOfField(afResults, dof[0], dof[1]); */
    // Convert from mm to diopters (in place)
    // dof is ensured not to be 0, check implementation of calculateDepthOfField
    /* dof[0] = 1000.0f / dof[0]; */
    /* dof[1] = 1000.0f / dof[1]; */
    //# METADATA_Dynamic lens.focusRange Done
    /* result.update(ANDROID_LENS_FOCUS_RANGE, dof, 2); */
}

/******************************************************************************
 * AF MODE   -  BASE
 ******************************************************************************/
RkAfModeBase::RkAfModeBase():
        mCurrentAfState(ANDROID_CONTROL_AF_STATE_INACTIVE),
        mLensState(ANDROID_LENS_STATE_STATIONARY),
        mLastActiveTriggerTime(0),
        mFramesSinceTrigger(0)
{
    mLastAfControls = { ANDROID_CONTROL_AF_MODE_AUTO,
            ANDROID_CONTROL_AF_TRIGGER_IDLE };
}

/**
 * processTriggers
 *
 * This method is called BEFORE auto focus algorithm has RUN
 * Input parameters are pre-filled by the Rk3APlus::fillAfInputParams()
 * by parsing the request settings.
 * Other parameters from the capture request settings not filled in the input
 * params structure is passed as argument
 */
XCamReturn
RkAfModeBase::processTriggers(const uint8_t &afTrigger,
                                 const uint8_t &afMode,
                                 int preCaptureId,
                                 XCamAfParam& afInputParams)
{
    /* UNUSED(afInputParams); */
    /* UNUSED(preCaptureId); */

    if (afTrigger == ANDROID_CONTROL_AF_TRIGGER_START) {
        resetTrigger(systemTime() / 1000);
        LOGI("AF TRIGGER START");
    } else if (afTrigger == ANDROID_CONTROL_AF_TRIGGER_CANCEL) {
        LOGI("AF TRIGGER CANCEL");
        resetTrigger(0);
    }
    mLastAfControls.afTrigger = afTrigger;
    mLastAfControls.afMode = afMode;
    return XCAM_RETURN_NO_ERROR;
}

void
RkAfModeBase::updateResult(CameraMetadata& results)
{

    //# METADATA_Dynamic control.afMode done
    LOGI("%s afMode = %s state = %s", __FUNCTION__,
            META_CONTROL2STR(afMode, mLastAfControls.afMode),
            META_CONTROL2STR(afState, mCurrentAfState));
    results.update(ANDROID_CONTROL_AF_MODE, &mLastAfControls.afMode, 1);
    //# METADATA_Dynamic control.afTrigger done
    results.update(ANDROID_CONTROL_AF_TRIGGER, &mLastAfControls.afTrigger, 1);
    //# METADATA_Dynamic control.afState done
    results.update(ANDROID_CONTROL_AF_STATE, &mCurrentAfState, 1);
    /**
     * LENS STATE update
     */
    //# METADATA_Dynamic lens.state Done
    results.update(ANDROID_LENS_STATE, &mLensState, 1);
}

void RkAfModeBase::resetTrigger(usecs_t triggerTime)
{
    mLastActiveTriggerTime = triggerTime;
    mFramesSinceTrigger = 0;
}

void RkAfModeBase::resetState(void)
{
    mCurrentAfState = ANDROID_CONTROL_AF_STATE_INACTIVE;
}


void RkAfModeBase::checkIfFocusTimeout()
{
     // give up if AF was iterating for too long
    if (mLastActiveTriggerTime != 0) {
        mFramesSinceTrigger++;
        usecs_t now = systemTime() / 1000;
        usecs_t timeSinceTriggered = now - mLastActiveTriggerTime;
        if (mCurrentAfState != ANDROID_CONTROL_AF_STATE_FOCUSED_LOCKED) {
            /**
             * Timeout IF either time has passed beyond MAX_AF_TIMEOUT
             *                         OR
             * Enough frames have been processed and time has passed beyond
             * MIN_AF_TIMEOUT
             */
            if (timeSinceTriggered > MAX_AF_TIMEOUT ||
                (mFramesSinceTrigger > MAX_AF_FRAME_COUNT_TIMEOUT &&
                 timeSinceTriggered  > MIN_AF_TIMEOUT)) {
                resetTrigger(0);
                mCurrentAfState = ANDROID_CONTROL_AF_STATE_NOT_FOCUSED_LOCKED;
            }
        }
    }
}

/******************************************************************************
 * AF MODE   -  OFF
 ******************************************************************************/

RkAFModeOff::RkAFModeOff():RkAfModeBase()
{
}

XCamReturn
RkAFModeOff::processTriggers(const uint8_t &afTrigger,
                                const uint8_t &afMode,
                                int preCaptureId,
                                XCamAfParam& afInputParams)
{
    /* UNUSED(afInputParams); */
    /* UNUSED(preCaptureId); */
    mLastAfControls.afTrigger = afTrigger;
    mLastAfControls.afMode = afMode;
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAFModeOff::processResult(rk_aiq_af_results& afResults,
                              CameraMetadata& result)
{
    /**
     * IN MANUAL and EDOF AF state never changes
     */
    /* UNUSED(afResults); */

    mCurrentAfState = ANDROID_CONTROL_AF_STATE_INACTIVE;
    mLensState = ANDROID_LENS_STATE_STATIONARY;
    updateResult(result);

    return XCAM_RETURN_NO_ERROR;
}

/******************************************************************************
 * AF MODE   -  AUTO
 ******************************************************************************/

RkAFModeAuto::RkAFModeAuto():RkAfModeBase()
{
}

XCamReturn
RkAFModeAuto::processTriggers(const uint8_t &afTrigger,
                                 const uint8_t &afMode,
                                 int preCaptureId,
                                 XCamAfParam& afInputParams)
{
    RkAfModeBase::processTriggers(afTrigger,
                                     afMode,
                                     preCaptureId,
                                     afInputParams);

    // Set the AIQ AF operation mode based on current AF state.
    // Current state = previous request's results, see processResult().
    // 'manual' is set to make the lens stationary in AUTO/MACRO mode,
    // when AF has not been triggered by the user.
    // NOTE: Need to use operation_mode_manual trick, as frame_use
    // is reset in Rk3aPlus::runAf()
    switch (mCurrentAfState) {
        case ANDROID_CONTROL_AF_STATE_ACTIVE_SCAN:
            afInputParams.focus_mode = AF_MODE_AUTO;
            break;
        case ANDROID_CONTROL_AF_STATE_INACTIVE:
        case ANDROID_CONTROL_AF_STATE_FOCUSED_LOCKED:
        case ANDROID_CONTROL_AF_STATE_NOT_FOCUSED_LOCKED:
            if (mLastActiveTriggerTime > 0) {
                // For a new scan upon a new active (START) trigger.
                afInputParams.focus_mode = AF_MODE_AUTO;
            } else {
                // For stopping lens at current scan, after receiving a result.
                // Trigger may be IDLE. Or a timeout case.
                /* afInputParams.focus_mode = rk_aiq_af_operation_mode_manual; */
                /* TODO  which mode?*/
                afInputParams.focus_mode = AF_MODE_MACRO;
            }
            break;
        default:
            LOGW("Unknown state in AF state machine: %d", mCurrentAfState);
            break;
    }

    // If a trigger is active, override the frame_use to keep it in still.
    // Also, make sure we do not re-start the AF sweep
    if (mLastActiveTriggerTime > 0) {
        // trigger scan start for the first frame after trigger
        afInputParams.trigger_new_search = (mFramesSinceTrigger == 0);
        // frame_use_still: only run AF sequence once for capture. Then stops.
        // Most aggressive AF mode.
        // TODO: This does NOT work properly now, due to
        // Rk3aPlus::getFrameUseFromIntent() resets frame_use
        afInputParams.frame_use = FRAME_USE_STILL;
    }

    // Override AF state if we just got an AF TRIGGER Start
    // This is only valid for the AUTO/MACRO state machine
    if (mLastAfControls.afTrigger == ANDROID_CONTROL_AF_TRIGGER_START) {
        mCurrentAfState = ANDROID_CONTROL_AF_STATE_ACTIVE_SCAN;
        LOGI("@%s AF state ACTIVE_SCAN (trigger start)", __PRETTY_FUNCTION__);
    } else if (mLastAfControls.afTrigger == ANDROID_CONTROL_AF_TRIGGER_CANCEL) {
        mCurrentAfState = ANDROID_CONTROL_AF_STATE_INACTIVE;
        LOGI("@%s AF state INACTIVE (trigger cancel)", __PRETTY_FUNCTION__);
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAFModeAuto::processResult(rk_aiq_af_results& afResult,
                               CameraMetadata& result)
{
    mLensState = ANDROID_LENS_STATE_STATIONARY;

    if (mLastActiveTriggerTime != 0) {
        switch (afResult.status) {
        case rk_aiq_af_status_local_search:
        case rk_aiq_af_status_extended_search:
            LOGI("@%s AF state SCANNING", __PRETTY_FUNCTION__);
            if (afResult.final_lens_position_reached == false)
                mLensState = ANDROID_LENS_STATE_MOVING;
            break;
        case rk_aiq_af_status_success:
            mCurrentAfState = ANDROID_CONTROL_AF_STATE_FOCUSED_LOCKED;
            resetTrigger(0);
            LOGI("@%s AF state FOCUSED_LOCKED", __PRETTY_FUNCTION__);
            break;
        case rk_aiq_af_status_fail:
            mCurrentAfState = ANDROID_CONTROL_AF_STATE_NOT_FOCUSED_LOCKED;
            resetTrigger(0);
            LOGI("@%s AF state NOT_FOCUSED_LOCKED", __PRETTY_FUNCTION__);
            break;
        default:
        case rk_aiq_af_status_idle:
            LOGI("@%s AF state INACTIVE", __PRETTY_FUNCTION__);
            break;
        }
    }

    checkIfFocusTimeout();

    updateResult(result);

    return XCAM_RETURN_NO_ERROR;
}


/******************************************************************************
 * AF MODE   -  CONTINUOUS PICTURE
 ******************************************************************************/

RkAFModeContinuousPicture::RkAFModeContinuousPicture():RkAfModeBase()
{
}

XCamReturn
RkAFModeContinuousPicture::processTriggers(const uint8_t &afTrigger,
                                              const uint8_t &afMode,
                                              int preCaptureId,
                                              XCamAfParam& afInputParams)
{
    RkAfModeBase::processTriggers(afTrigger,
                                     afMode,
                                     preCaptureId,
                                     afInputParams);

    // If we are locked then set trigger_new_search to false to keep the lens
    // locked
    if (mCurrentAfState == ANDROID_CONTROL_AF_STATE_NOT_FOCUSED_LOCKED ||
        mCurrentAfState == ANDROID_CONTROL_AF_STATE_FOCUSED_LOCKED) {
        afInputParams.trigger_new_search = false;
    } else {
    // In normal situation we do not need to use this. But let us start AF once
    // after cancel, as CTS2 robustness test does not wait for long and
    // continuous AF restart is expected at cancel
    // (see android.control.afState documentation)
        afInputParams.trigger_new_search =
                (afTrigger == ANDROID_CONTROL_AF_TRIGGER_CANCEL);
    }

    // Override AF state if we just got an AF TRIGGER CANCEL
    if (mLastAfControls.afTrigger == ANDROID_CONTROL_AF_TRIGGER_CANCEL) {
        /* Scan is supposed to be restarted, which we try by triggering a new
         * scan. (see RkAFStateMachine::processTriggers)
         * This however, doesn't do anything at all, because AIQ does not
         * want to play ball, at least yet.
         *
         * We can skip state transitions when allowed by the state
         * machine documentation, so skip INACTIVE, also skip PASSIVE_SCAN if
         * possible and go directly to either PASSIVE_FOCUSED or UNFOCUSED
         *
         * TODO: Remove this switch-statement, once triggering a scan starts to
         * work. We could go directly to PASSIVE_SCAN always then, because a
         * scan is really happening. Now it is not.
         */
        switch (mCurrentAfState) {
        case ANDROID_CONTROL_AF_STATE_PASSIVE_SCAN:
        case ANDROID_CONTROL_AF_STATE_NOT_FOCUSED_LOCKED:
            mCurrentAfState = ANDROID_CONTROL_AF_STATE_PASSIVE_UNFOCUSED;
            break;
        case ANDROID_CONTROL_AF_STATE_FOCUSED_LOCKED:
            mCurrentAfState = ANDROID_CONTROL_AF_STATE_PASSIVE_FOCUSED;
            break;
        default:
            mCurrentAfState = ANDROID_CONTROL_AF_STATE_PASSIVE_SCAN;
            break;
        }
    }
    /* Override AF state if we just got an AF TRIGGER START, this will stop
     * the scan as intended in the state machine documentation (see
     * RkAFStateMachine::processTriggers)
     */
    if (mLastAfControls.afTrigger == ANDROID_CONTROL_AF_TRIGGER_START) {
        if (mCurrentAfState == ANDROID_CONTROL_AF_STATE_PASSIVE_FOCUSED)
            mCurrentAfState = ANDROID_CONTROL_AF_STATE_FOCUSED_LOCKED;
        else if (mCurrentAfState == ANDROID_CONTROL_AF_STATE_PASSIVE_UNFOCUSED ||
                 mCurrentAfState == ANDROID_CONTROL_AF_STATE_PASSIVE_SCAN)
            mCurrentAfState = ANDROID_CONTROL_AF_STATE_NOT_FOCUSED_LOCKED;
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAFModeContinuousPicture::processResult(rk_aiq_af_results& afResult,
                                           CameraMetadata& result)
{
    mLensState = ANDROID_LENS_STATE_STATIONARY;

    // state transition from locked state are only allowed via triggers, which
    // are handled in the currentAFMode processTriggers() and below in this function.
    if (mCurrentAfState != ANDROID_CONTROL_AF_STATE_FOCUSED_LOCKED &&
        mCurrentAfState != ANDROID_CONTROL_AF_STATE_NOT_FOCUSED_LOCKED) {
        switch (afResult.status) {
        case rk_aiq_af_status_local_search:
        case rk_aiq_af_status_extended_search:
            LOGI("@%s AF state SCANNING", __PRETTY_FUNCTION__);
            mCurrentAfState = ANDROID_CONTROL_AF_STATE_PASSIVE_SCAN;
            if (afResult.final_lens_position_reached == false)
                mLensState = ANDROID_LENS_STATE_MOVING;
            break;
        case rk_aiq_af_status_success:
            if (mLastActiveTriggerTime == 0) {
                mCurrentAfState = ANDROID_CONTROL_AF_STATE_PASSIVE_FOCUSED;
                LOGI("@%s AF state PASSIVE_FOCUSED", __PRETTY_FUNCTION__);
            } else {
                resetTrigger(0);
                mCurrentAfState = ANDROID_CONTROL_AF_STATE_FOCUSED_LOCKED;
                LOGI("@%s AF state FOCUSED_LOCKED", __PRETTY_FUNCTION__);
            }
            break;
        case rk_aiq_af_status_fail:
            if (mLastActiveTriggerTime == 0) {
                mCurrentAfState = ANDROID_CONTROL_AF_STATE_PASSIVE_UNFOCUSED;
                LOGI("@%s AF state PASSIVE_UNFOCUSED", __PRETTY_FUNCTION__);
            } else {
                resetTrigger(0);
                mCurrentAfState = ANDROID_CONTROL_AF_STATE_NOT_FOCUSED_LOCKED;
                LOGI("@%s AF state NOT_FOCUSED_LOCKED", __PRETTY_FUNCTION__);
            }
            break;
        default:
        case rk_aiq_af_status_idle:
            if (mCurrentAfState == ANDROID_CONTROL_AF_STATE_INACTIVE) {
                mCurrentAfState = ANDROID_CONTROL_AF_STATE_PASSIVE_UNFOCUSED;
                LOGI("@%s AF state PASSIVE_UNFOCUSED (idle)", __PRETTY_FUNCTION__);
            }
            break;
        }
    }

    checkIfFocusTimeout();

    updateResult(result);
    return XCAM_RETURN_NO_ERROR;
}
}
