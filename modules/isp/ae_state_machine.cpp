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

#define LOG_TAG "ae_state_machine"

#include "ae_state_machine.h"
#include "Metadata2Str.h"

namespace XCam {
RkAEStateMachine::RkAEStateMachine():
        mLastControlMode(0),
        mCurrentAeState(ANDROID_CONTROL_AE_STATE_INACTIVE),
        mCurrentAeMode(nullptr)
{
    mCurrentAeMode = &mAutoMode;
    memset(&mLastAeControls, 0, sizeof(mLastAeControls));
    mLastAeControls.aeMode = ANDROID_CONTROL_AE_MODE_ON;
}

RkAEStateMachine::~RkAEStateMachine()
{
}

/**
 * Process states in input stage before the AE is run.
 * It is initializing the current state if input
 * parameters have an influence.
 *
 * \param[IN] controlMode: control.controlMode
 * \param[IN] aeControls: set of control.<ae>
 */
XCamReturn
RkAEStateMachine::processState(const uint8_t &controlMode,
                                  const AeControls &aeControls)
{
    XCamReturn ret;

    if (controlMode == ANDROID_CONTROL_MODE_OFF) {
        LOGD("%s: Set AE offMode: controlMode = %s, aeMode = %s", __FUNCTION__,
                        META_CONTROL2STR(mode, controlMode),
                        META_CONTROL2STR(aeMode, aeControls.aeMode));
        mCurrentAeMode = &mOffMode;
    } else {
        if (aeControls.aeMode == ANDROID_CONTROL_AE_MODE_OFF) {
            mCurrentAeMode = &mOffMode;
            LOGD("%s: Set AE offMode: controlMode = %s, aeMode = %s",
                 __FUNCTION__,
                 META_CONTROL2STR(mode, controlMode),
                 META_CONTROL2STR(aeMode, aeControls.aeMode));
        } else {
            LOGD("%s: Set AE AutoMode: controlMode = %s, aeMode = %s",
                 __FUNCTION__,
                 META_CONTROL2STR(mode, controlMode),
                 META_CONTROL2STR(aeMode, aeControls.aeMode));
            mCurrentAeMode = &mAutoMode;
        }
    }

    mLastAeControls = aeControls;
    mLastControlMode = controlMode;
    ret = mCurrentAeMode->processState(controlMode, aeControls);
    return ret;
}

/**
 * Process results and define output state after the AE is run
 *
 * \param[IN] aeResults: from the ae run
 * \param[IN] results: cameraMetadata to write dynamic tags.
 * \param[IN] reqId: request Id
 */
XCamReturn
RkAEStateMachine::processResult(const rk_aiq_ae_results &aeResults,
                                   CameraMetadata &result,
                                   uint32_t reqId)
{
    XCamReturn ret;

    if (mCurrentAeMode == nullptr) {
        LOGE("Invalid AE mode - this could not happen - BUG!");
        return XCAM_RETURN_ERROR_UNKNOWN;
    }

    ret =  mCurrentAeMode->processResult(aeResults, result, reqId);
    return ret;
}

/******************************************************************************
 * AE MODE   -  BASE
 ******************************************************************************/
RkAEModeBase::RkAEModeBase():
    mLastControlMode(0),
    mEvChanged(false),
    mLastAeConvergedFlag(false),
    mAeRunCount(0),
    mAeConvergedCount(0),
    mCurrentAeState(ANDROID_CONTROL_AE_STATE_INACTIVE)
{
    memset(&mLastAeControls, 0, sizeof(mLastAeControls));
}

void
RkAEModeBase::updateResult(CameraMetadata &results)
{

    LOGD("%s: current AE state is: %s", __FUNCTION__,
         META_CONTROL2STR(aeState, mCurrentAeState));

    //# METADATA_Dynamic control.aeMode done
    results.update(ANDROID_CONTROL_AE_MODE, &mLastAeControls.aeMode, 1);
    //# METADATA_Dynamic control.aeLock done
    results.update(ANDROID_CONTROL_AE_LOCK, &mLastAeControls.aeLock, 1);
    //# METADATA_Dynamic control.aePrecaptureTrigger done
    results.update(ANDROID_CONTROL_AE_PRECAPTURE_TRIGGER,
                   &mLastAeControls.aePreCaptureTrigger, 1);
    //# METADATA_Dynamic control.aeAntibandingMode done
    results.update(ANDROID_CONTROL_AE_ANTIBANDING_MODE,
                   &mLastAeControls.aeAntibanding, 1);
    //# METADATA_Dynamic control.aeTargetFpsRange done
    results.update(ANDROID_CONTROL_AE_TARGET_FPS_RANGE,
                   &mLastAeControls.aeTargetFpsRange[0], 2);
    //# METADATA_Dynamic control.aeState done
    results.update(ANDROID_CONTROL_AE_STATE, &mCurrentAeState, 1);
}

void
RkAEModeBase::resetState(void)
{
    mCurrentAeState = ANDROID_CONTROL_AE_STATE_INACTIVE;
    mLastAeConvergedFlag = false;
    mAeRunCount = 0;
    mAeConvergedCount = 0;
}

/******************************************************************************
 * AE MODE   -  OFF
 ******************************************************************************/

RkAEModeOff::RkAEModeOff():RkAEModeBase()
{
}

XCamReturn
RkAEModeOff::processState(const uint8_t &controlMode,
                             const AeControls &aeControls)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mLastAeControls = aeControls;
    mLastControlMode = controlMode;

    if (controlMode == ANDROID_CONTROL_MODE_OFF ||
        aeControls.aeMode == ANDROID_CONTROL_AE_MODE_OFF) {
        resetState();
    } else {
        LOGE("AE State machine should not be OFF! - Fix bug");
        ret = XCAM_RETURN_ERROR_UNKNOWN;
    }

    return ret;
}

XCamReturn
RkAEModeOff::processResult(const rk_aiq_ae_results &aeResults,
                              CameraMetadata &result,
                              uint32_t reqId)
{
    /* UNUSED(aeResults); */
    /* UNUSED(reqId); */

    mCurrentAeState = ANDROID_CONTROL_AE_STATE_INACTIVE;
    updateResult(result);

    return XCAM_RETURN_NO_ERROR;
}

/******************************************************************************
 * AE MODE   -  AUTO
 ******************************************************************************/

RkAEModeAuto::RkAEModeAuto():RkAEModeBase()
{
}

XCamReturn
RkAEModeAuto::processState(const uint8_t &controlMode,
                              const AeControls &aeControls)
{
    if(controlMode != mLastControlMode) {
        LOGI("%s: control mode has changed %s -> %s, reset AE State",
             __FUNCTION__,
             META_CONTROL2STR(mode, mLastControlMode),
             META_CONTROL2STR(mode, controlMode));
        resetState();
    }

    if (aeControls.aeLock == ANDROID_CONTROL_AE_LOCK_ON) {
        // If ev compensation changes, we have to let the AE run until
        // convergence. Thus we need to figure out changes in compensation and
        // only change the state immediately to locked,
        // IF the EV did not change.
        if (mLastAeControls.evCompensation != aeControls.evCompensation)
            mEvChanged = true;

        if (!mEvChanged)
            mCurrentAeState = ANDROID_CONTROL_AE_STATE_LOCKED;
    } else if (aeControls.aeMode != mLastAeControls.aeMode) {
        resetState();
    } else {
        switch (mCurrentAeState) {
            case ANDROID_CONTROL_AE_STATE_LOCKED:
                mCurrentAeState = ANDROID_CONTROL_AE_STATE_INACTIVE;
                break;
            case ANDROID_CONTROL_AE_STATE_SEARCHING:
            case ANDROID_CONTROL_AE_STATE_INACTIVE:
            case ANDROID_CONTROL_AE_STATE_CONVERGED:
            case ANDROID_CONTROL_AE_STATE_PRECAPTURE:
                if (aeControls.aePreCaptureTrigger ==
                        ANDROID_CONTROL_AE_PRECAPTURE_TRIGGER_START)
                    mCurrentAeState = ANDROID_CONTROL_AE_STATE_PRECAPTURE;

                if (aeControls.aePreCaptureTrigger ==
                        ANDROID_CONTROL_AE_PRECAPTURE_TRIGGER_CANCEL)
                    mCurrentAeState = ANDROID_CONTROL_AE_STATE_INACTIVE;
                break;
            default:
                LOGE("Invalid AE state: %d !, State set to INACTIVE", mCurrentAeState);
                mCurrentAeState = ANDROID_CONTROL_AE_STATE_INACTIVE;
            break;
        }
    }
    mLastAeControls = aeControls;
    mLastControlMode = controlMode;
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAEModeAuto::processResult(const rk_aiq_ae_results &aeResults,
                               CameraMetadata &result,
                               uint32_t reqId)
{
    switch (mCurrentAeState) {
    case ANDROID_CONTROL_AE_STATE_LOCKED:
        //do nothing
        break;
    case ANDROID_CONTROL_AE_STATE_INACTIVE:
    case ANDROID_CONTROL_AE_STATE_SEARCHING:
    case ANDROID_CONTROL_AE_STATE_CONVERGED:
    case ANDROID_CONTROL_AE_STATE_FLASH_REQUIRED:
         if (aeResults.converged) {
            mEvChanged = false; // converged -> reset
            if (mLastAeControls.aeLock) {
                mCurrentAeState = ANDROID_CONTROL_AE_STATE_LOCKED;
            } else {
                /* TODO */
                /* if (aeResults.flashes[0].status == rk_aiq_flash_status_torch || */
                /*     aeResults.flashes[0].status == rk_aiq_flash_status_pre) { */
                /*     mCurrentAeState = ANDROID_CONTROL_AE_STATE_FLASH_REQUIRED; */
                /* } else { */
                /*     mCurrentAeState = ANDROID_CONTROL_AE_STATE_CONVERGED; */
                /* } */
                mCurrentAeState = ANDROID_CONTROL_AE_STATE_CONVERGED;
            }
        } else {
            mCurrentAeState = ANDROID_CONTROL_AE_STATE_SEARCHING;
        }
        break;
    case ANDROID_CONTROL_AE_STATE_PRECAPTURE:
         if (aeResults.converged) {
            mEvChanged = false; // converged -> reset
            if (mLastAeControls.aeLock) {
                mCurrentAeState = ANDROID_CONTROL_AE_STATE_LOCKED;
            } else {
                /* TODO */
                /* if (aeResults.flashes[0].status == rk_aiq_flash_status_torch */
                /*         || aeResults.flashes[0].status */
                /*                 == rk_aiq_flash_status_pre) */
                /*     mCurrentAeState = ANDROID_CONTROL_AE_STATE_FLASH_REQUIRED; */
                /* else */
                /*     mCurrentAeState = ANDROID_CONTROL_AE_STATE_CONVERGED; */
                mCurrentAeState = ANDROID_CONTROL_AE_STATE_CONVERGED;
            }
        } // here the else is staying at the same state.
        break;
    default:
        LOGE("Invalid AE state: %d !, State set to INACTIVE", mCurrentAeState);
        mCurrentAeState = ANDROID_CONTROL_AE_STATE_INACTIVE;
        break;
    }

    if (aeResults.converged) {
        if (mLastAeConvergedFlag == true) {
            mAeConvergedCount++;
            LOGD("%s: AE converged for %d frames (reqId: %d)",
                        __FUNCTION__, mAeConvergedCount, reqId);
        } else {
            mAeConvergedCount = 1;
            LOGD("%s: AE converging -> converged (reqId: %d)"
                 ", after running AE for %d times",
                        __FUNCTION__, reqId, mAeRunCount);
        }
    } else {
        if (mLastAeConvergedFlag == true) {
            LOGD("%s: AE Converged -> converging (reqId: %d)",
                        __FUNCTION__, reqId);
            mAeRunCount = 1;
            mAeConvergedCount = 0;
        } else {
            mAeRunCount++;
            LOGD("%s: AE converging for %d frames, (reqId: %d.",
                        __FUNCTION__, mAeRunCount, reqId);
        }
    }
    mLastAeConvergedFlag = aeResults.converged;

    updateResult(result);

    return XCAM_RETURN_NO_ERROR;
}
}
