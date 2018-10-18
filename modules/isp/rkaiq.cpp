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
{
    memset(&aeInputParams, 0, sizeof(AeInputParams));
    memset(&awbInputParams, 0, sizeof(AwbInputParams));
    memset(&afInputParams, 0, sizeof(AfInputParams));
    memset(&aaaControls, 0, sizeof(AAAControls));
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
    memcpy(&this->aeInputParams, &other.aeInputParams, sizeof(AeInputParams));
    memcpy(&this->awbInputParams, &other.awbInputParams, sizeof(AwbInputParams));
    memcpy(&this->afInputParams, &other.afInputParams, sizeof(AfInputParams));
    memcpy(&this->aaaControls, &other.aaaControls, sizeof(AAAControls));
    this->aeInputParams.aiqParams = this;
    this->awbInputParams.aiqParams = this;
    this->afInputParams.aiqParams = this;
    this->settings = other.settings;

    return *this;
}
