
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

#include "Isp20Evts.h"

namespace RkCam {

XCamReturn
Isp20Evt::getExpInfoParams(SmartPtr<RkAiqSensorExpParamsProxy>& expInfo, uint32_t frameId)
{
    ENTER_CAMHW_FUNCTION();
    SmartLock locker (_mutex);

    if (mSensor.ptr())
        mSensor->getEffectiveExpParams(expInfo, frameId);
    else
        expInfo = nullptr;

    EXIT_CAMHW_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

} //namspace RkCam
