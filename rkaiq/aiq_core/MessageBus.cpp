/*
 * rkisp_aiq_core.h
 *
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
#include "MessageBus.h"
#include "RkAiqCore.h"

namespace RkCam {

bool
MessageThread::loop()
{
    const static int32_t msg_time_out = -1; //wait until wakeup
    SmartPtr<XCamMessage> msg = mMsgQueue.pop (msg_time_out);

    if (!msg.ptr ()) {
        LOGW_ANALYZER("MessageThread got empty message, stop thread");
        return  false;
    }

    XCamReturn ret = mRkAiqCore->handle_message (msg);
    if (ret == XCAM_RETURN_NO_ERROR || ret == XCAM_RETURN_ERROR_TIMEOUT)
        return true;

    return false;
}

}
