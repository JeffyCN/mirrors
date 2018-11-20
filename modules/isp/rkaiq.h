/*
 * Copyright (C) 2017 Intel Corporation.
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

#ifndef __RKAIQ_H
#define __RKAIQ_H

#include <xcam_std.h>
#ifdef ANDROID_VERSION_ABOVE_8_X
#include <CameraMetadata.h>
using ::android::hardware::camera::common::V1_0::helper::CameraMetadata;
#else
#include <camera/CameraMetadata.h>
#endif
#include <base/xcam_params.h>

using namespace android;
namespace XCam {
typedef struct _AiqInputParams AiqInputParams; 

/**
 * \struct AeControls
 *
 * Control Modes saved and passed back to control unit after reading
 *
 */
typedef struct _AeControls {
    uint8_t aeMode;                 /**< AE_MODE */
    uint8_t aeLock;                 /**< AE_LOCK */
    uint8_t aePreCaptureTrigger;    /**< PRECAPTURE_TRIGGER */
    uint8_t aeAntibanding;          /**< AE_ANTIBANDING */
    int32_t evCompensation;         /**< AE_EXPOSURE_COMPENSATION */
    int32_t aeTargetFpsRange[2];    /**< AE_TARGET_FPS_RANGE */
} AeControls;

/**
 * \struct AwbControls
 *
 * Control Modes saved and passed back to control unit after reading
 *
 */
typedef struct _AwbControls {
    uint8_t awbMode;                        /**< AWB_MODE */
    uint8_t awbLock;                        /**< AWB_LOCK */
    uint8_t colorCorrectionMode;            /**< COLOR_CORRECTION_MODE */
    uint8_t colorCorrectionAberrationMode;  /**< COLOR_CORRECTION_ABERRATION_MODE */
} AwbControls;

/**
 * \struct AfControls
 *
 * Control Modes saved and passed back to control unit after reading
 *
 */
typedef struct _AfControls {
    uint8_t afMode; /**< AF_MODE */
    uint8_t afTrigger; /**< AF_TRIGGER */
} AfControls;

typedef struct _AAAControls {
    uint8_t controlMode;    /**< MODE */
    AeControls  ae;
    AwbControls awb;
    AfControls  af;
} AAAControls;

typedef struct _AeInputParams {
    XCamAeParam                         aeParams;
    int32_t                             aeRegion[5];
    AiqInputParams*                     aiqParams;
    /* AeInputParams()                     { CLEAR(*this); } */
} AeInputParams;

typedef struct _AwbInputParams {
    XCamAwbParam                        awbParams;
    int32_t                             awbRegion[5];
    AiqInputParams*                     aiqParams;
    /* AwbInputParams()                    { CLEAR(*this); } */
} AwbInputParams;

typedef struct _AfInputParams {
    XCamAfParam                         afParams;
    int32_t                             afRegion[5];
    AiqInputParams*                     aiqParams;
    /* AfInputParams()                    { CLEAR(*this); } */
} AfInputParams;

typedef struct _AiqInputParams {
    _AiqInputParams &operator=(const _AiqInputParams &other);
    unsigned int    reqId;
    AeInputParams   aeInputParams;
    AwbInputParams  awbInputParams;
    AfInputParams   afInputParams;
    AAAControls     aaaControls;
    CameraMetadata  settings;
    CameraMetadata* staticMeta;
    int sensorOutputWidth;
    int sensorOutputHeight;
    void init();
    _AiqInputParams();
    ~_AiqInputParams() {}
} AiqInputParams;

};
#endif //__RKAIQ_H
