/*
 * Copyright (C) 2015-2017 Intel Corporation
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
#ifndef _AF_STATE_MACHINE_H_
#define _AF_STATE_MACHINE_H_

#include <xcam_std.h>
#ifdef ANDROID_VERSION_ABOVE_8_X
#include <CameraMetadata.h>
using ::android::hardware::camera::common::V1_0::helper::CameraMetadata;
#else
#include <camera/CameraMetadata.h>
#endif
#include "rkaiq.h"
#include "rk_aiq_types.h"
#include <base/log.h>

using namespace android;
namespace XCam {
typedef int64_t nsecs_t;       // nano-seconds

// return the system-time according to the specified clock
nsecs_t systemTime();

typedef nsecs_t usecs_t;

/**
 * \class RkAfModeBase
 *
 * Base class for all the AutoFocus modes as defined by the Android
 * camera device V3.x API.
 * Each mode will follow certain state transitions. See documentation for
 * android.control.afState
 *
 */
class RkAfModeBase {
public:
    RkAfModeBase();
    virtual ~RkAfModeBase() {};

    virtual XCamReturn processTriggers(const uint8_t &afTrigger,
                                     const uint8_t &afMode,
                                     int preCaptureId,
                                     XCamAfParam& afInputParams);
    virtual XCamReturn processResult(rk_aiq_af_results& afResults,
                                   CameraMetadata& result) = 0;
    void resetState(void);
    void resetTrigger(usecs_t triggerTime);
    int getState() { return mCurrentAfState; }
    void updateResult(CameraMetadata& results);
protected:
    void checkIfFocusTimeout();
protected:
    AfControls mLastAfControls;
    uint8_t mCurrentAfState;
    uint8_t  mLensState;
    usecs_t  mLastActiveTriggerTime;   /**< in useconds */
    uint32_t mFramesSinceTrigger;
};

/**
 * \class RkAFModeAuto
 * Derived class from RkAFModeBase for Auto mode
 *
 */
class RkAFModeAuto: public RkAfModeBase {
public:
    RkAFModeAuto();
    virtual XCamReturn processTriggers(const uint8_t &afTrigger,
                                     const uint8_t &afMode,
                                     int preCaptureId,
                                     XCamAfParam& afInputParams);
    virtual XCamReturn processResult(rk_aiq_af_results& afResults,
                                  CameraMetadata& result);
};

/**
 * \class RkAFModeContinuousPicture
 * Derived class from RkAFModeBase for Continuous AF mode
 *
 */
class RkAFModeContinuousPicture: public RkAfModeBase {
public:
    RkAFModeContinuousPicture();
    virtual XCamReturn processTriggers(const uint8_t &afTrigger,
                                     const uint8_t &afMode,
                                     int preCaptureId,
                                     XCamAfParam& afInputParams);
    virtual XCamReturn processResult(rk_aiq_af_results& afResults,
                                  CameraMetadata& result);
};

/**
 * \class RkAFModeOff
 * Derived class from RkAFModeBase for OFF mode
 *
 */
class RkAFModeOff: public RkAfModeBase {
public:
    RkAFModeOff();
    virtual XCamReturn processTriggers(const uint8_t &afTrigger,
                                     const uint8_t &afMode,
                                     int preCaptureId,
                                     XCamAfParam& afInputParams);
    virtual XCamReturn processResult(rk_aiq_af_results& afResults,
                                  CameraMetadata& result);
};

/**
 * \class RkAFStateMachine
 *
 * This class adapts the Android V3 AF triggers and state transitions to
 * the ones implemented by the Rk AIQ algorithm
 * This class is platform independent. Platform specific behaviors should be
 * implemented in derived classes from this one or from the RkAFModeBase
 *
 */
class RkAFStateMachine {
public:
    RkAFStateMachine();
    virtual ~RkAFStateMachine();

    virtual XCamReturn processTriggers(const uint8_t &afTrigger,
                                     const uint8_t &afMode,
                                     int preCaptureId,
                                     XCamAfParam &afInputParams);

    virtual XCamReturn processResult(rk_aiq_af_results &afResults,
                                   XCamAfParam &afInputParams,
                                   CameraMetadata& result);

    virtual XCamReturn updateDefaults(const rk_aiq_af_results &afResults,
                                    const XCamAfParam &afInputParams,
                                    CameraMetadata &result,
                                    bool fixedFocus = false) const;

private:
    // prevent copy constructor and assignment operator
    RkAFStateMachine(const RkAFStateMachine& other);
    RkAFStateMachine& operator=(const RkAFStateMachine& other);

    void focusDistanceResult(const rk_aiq_af_results &afResults,
                             const XCamAfParam &afInputParams,
                             CameraMetadata &result) const;

private: /* members*/
    int mCameraId;
    AfControls      mLastAfControls;
    RkAfModeBase *mCurrentAfMode;
    uint8_t         mCurrentAfState;

    RkAFModeOff mOffMode;
    RkAFModeAuto mAutoMode;

    RkAFModeContinuousPicture mContinuousPictureMode;
    /* const Rk3aPlus &m3A; */
};
}

#endif // _AF_STATE_MACHINE_H_
