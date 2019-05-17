/*
 * sensor_descriptor.h - sensor descriptor
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
 * Author: Wind Yuan <feng.yuan@intel.com>
 */

#ifndef __SETTINGS_PROCESSOR_H
#define __SETTINGS_PROCESSOR_H

#include <xcam_std.h>
#ifdef ANDROID_VERSION_ABOVE_8_X
#include <CameraMetadata.h>
using ::android::hardware::camera::common::V1_0::helper::CameraMetadata;
#else
#include <camera/CameraMetadata.h>
#endif
#include <rkcamera_vendor_tags.h>
#include "rkaiq.h"
#include "CameraWindow.h"

using namespace android;
using namespace XCam;

class SettingsProcessor
{
public:
    SettingsProcessor();
    virtual ~SettingsProcessor();
    XCamReturn init();

    /* Parameter processing methods */
    XCamReturn processRequestSettings(const CameraMetadata &settings,
                                    AiqInputParams &aiqparams);

private:

    XCamReturn processAwbSettings(const CameraMetadata &settings,
                                AiqInputParams &aiqparams);
    XCamReturn processAeSettings(const CameraMetadata &settings,
                               AiqInputParams &aiqparams);
    XCamReturn processAfSettings(const CameraMetadata &settings,
                               AiqInputParams &aiqparams);
    XCamReturn processTuningParamsSettings(const CameraMetadata &settings,
                               AiqInputParams &aiqparams);
    XCamReturn fillAeInputParams(const CameraMetadata *settings,
                                 AiqInputParams *aiqInputParams);
    XCamReturn fillAwbInputParams(const CameraMetadata *settings,
                                 AiqInputParams *aiqInputParams);
    XCamReturn fillAfInputParams(const CameraMetadata *settings,
                                 AiqInputParams *aiqInputParams);
    XCamReturn fillBlsInputParams(const CameraMetadata *settings,
                                 AiqInputParams *aiqInputParams);
    XCamReturn fillLscInputParams(const CameraMetadata *settings,
                                 AiqInputParams *aiqInputParams);
    XCamReturn fillCcmInputParams(const CameraMetadata *settings,
                                 AiqInputParams *aiqInputParams);
    XCamReturn fillAwbToolInputParams(const CameraMetadata *settings,
                                 AiqInputParams *aiqInputParams);
    XCamReturn fillToolAwbInputParams(const CameraMetadata *settings,
                                 AiqInputParams *aiqInputParams);
    XCamReturn fillAwbWhitePointInputParams(const CameraMetadata *settings,
                                 AiqInputParams *aiqInputParams);
    XCamReturn fillAwbCurvInputParams(const CameraMetadata *settings,
                                 AiqInputParams *aiqInputParams);
    XCamReturn fillAwbRefGainInputParams(const CameraMetadata *settings,
                                 AiqInputParams *aiqInputParams);
    XCamReturn fillGocInputParams(const CameraMetadata *settings,
                                 AiqInputParams *aiqInputParams);
    XCamReturn fillCprocInputParams(const CameraMetadata *settings,
                                 AiqInputParams *aiqInputParams);
    XCamReturn fillAdpfInputParams(const CameraMetadata *settings,
                                 AiqInputParams *aiqInputParams);
    XCamReturn fillFltInputParams(const CameraMetadata *settings,
                                 AiqInputParams *aiqInputParams);
    XCamReturn restartInputParams(const CameraMetadata *settings, AiqInputParams *aiqInputParams);
    XCamReturn tuningFlagInputParams(const CameraMetadata *settings, AiqInputParams *aiqInputParams);

    void parseMeteringRegion(const CameraMetadata *settings,
                             int tagId, CameraWindow *meteringWindow);
    void convertCoordinates(CameraWindow *region,
                           int sensorOutputWidth, int sensorOutputHeight);
private:

};

#endif //__SETTINGS_PROCESSOR_H
