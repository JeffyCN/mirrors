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
#include "rkaiq.h"

using namespace XCam;
_AiqInputParams::_AiqInputParams():
    reqId(0)
    ,tuningFlag(0)
{
    memset(&aeInputParams, 0, sizeof(AeInputParams));
    memset(&awbInputParams, 0, sizeof(AwbInputParams));
    memset(&afInputParams, 0, sizeof(AfInputParams));
    memset(&aaaControls, 0, sizeof(AAAControls));
    memset(&blsInputParams, 0, sizeof(blsInputParams));
    memset(&lscInputParams, 0, sizeof(lscInputParams));
    memset(&ccmInputParams, 0, sizeof(ccmInputParams));
    memset(&awbToolInputParams, 0, sizeof(awbToolInputParams));
    memset(&awbWpInputParams, 0, sizeof(awbWpInputParams));
    memset(&awbCurveInputParams, 0, sizeof(awbCurveInputParams));
    memset(&awbRefGainInputParams, 0, sizeof(awbRefGainInputParams));
    memset(&gocInputParams, 0, sizeof(gocInputParams));
    memset(&cprocInputParams, 0, sizeof(cprocInputParams));
    memset(&adpfInputParams, 0, sizeof(adpfInputParams));
    memset(&fltInputParams, 0, sizeof(fltInputParams));
    memset(&restartInputParams, 0, sizeof(restartInputParams));
    aeInputParams.aiqParams = this;
    awbInputParams.aiqParams = this;
    afInputParams.aiqParams = this;
    sensorOutputWidth = 0;
    sensorOutputHeight = 0;
}
void XCam::_AiqInputParams::init()
{
}

_AiqInputParams &_AiqInputParams::operator=(const _AiqInputParams &other)
{
    if (this == &other)
        return *this;

    this->reqId = other.reqId;
    this->tuningFlag = other.tuningFlag;
    memcpy(&this->aeInputParams, &other.aeInputParams, sizeof(AeInputParams));
    memcpy(&this->awbInputParams, &other.awbInputParams, sizeof(AwbInputParams));
    memcpy(&this->afInputParams, &other.afInputParams, sizeof(AfInputParams));
    memcpy(&this->aaaControls, &other.aaaControls, sizeof(AAAControls));
    memcpy(&this->blsInputParams, &other.blsInputParams, sizeof(blsInputParams));
    memcpy(&this->lscInputParams, &other.lscInputParams, sizeof(lscInputParams));
    memcpy(&this->ccmInputParams, &other.ccmInputParams, sizeof(ccmInputParams));
    memcpy(&this->awbToolInputParams, &other.awbToolInputParams, sizeof(awbToolInputParams));
    memcpy(&this->awbWpInputParams, &other.awbWpInputParams, sizeof(awbWpInputParams));
    memcpy(&this->awbCurveInputParams, &other.awbCurveInputParams, sizeof(awbCurveInputParams));
    memcpy(&this->awbRefGainInputParams, &other.awbRefGainInputParams, sizeof(awbRefGainInputParams));
    memcpy(&this->gocInputParams, &other.gocInputParams, sizeof(gocInputParams));
    memcpy(&this->cprocInputParams, &other.cprocInputParams, sizeof(cprocInputParams));
    memcpy(&this->adpfInputParams, &other.adpfInputParams, sizeof(adpfInputParams));
    memcpy(&this->fltInputParams, &other.fltInputParams, sizeof(fltInputParams));
    memcpy(&this->restartInputParams, &other.restartInputParams, sizeof(restartInputParams));

    this->aeInputParams.aiqParams = this;
    this->awbInputParams.aiqParams = this;
    this->afInputParams.aiqParams = this;
    this->settings = other.settings;

    return *this;
}
