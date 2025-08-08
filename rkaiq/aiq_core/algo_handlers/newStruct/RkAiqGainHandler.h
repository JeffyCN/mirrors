/*
 * Copyright (c) 2019-2022 Rockchip Eletronics Co., Ltd.
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
#ifndef _RK_AIQ_GAIN_HANDLE_INT_H_
#define _RK_AIQ_GAIN_HANDLE_INT_H_

#include "RkAiqHandle.h"
#include "rk_aiq_api_private.h"
#include "rk_aiq_pool.h"
#include "xcam_mutex.h"
#include "newStruct/gain/include/gain_algo_api.h"

namespace RkCam {

class RkAiqGainHandleInt : virtual public RkAiqHandle {
public:
    explicit RkAiqGainHandleInt(RkAiqAlgoDesComm* des, RkAiqCore* aiqCore)
        : RkAiqHandle(des, aiqCore) {}
    virtual ~RkAiqGainHandleInt() {
        RkAiqHandle::deInit();
    };
    virtual XCamReturn prepare();
    virtual XCamReturn processing();
    virtual XCamReturn preProcess() { return XCAM_RETURN_NO_ERROR;  };
    virtual XCamReturn postProcess() { return XCAM_RETURN_NO_ERROR;  };
    virtual XCamReturn genIspResult(RkAiqFullParams* params, RkAiqFullParams* cur_params);

    XCamReturn setAttrib(gain_api_attrib_t* attr);
    XCamReturn getAttrib(gain_api_attrib_t* attr);
    XCamReturn queryStatus(gain_status_t* status);

protected:
    virtual void init();
    virtual void deInit() {
        RkAiqHandle::deInit();
    };

private:

private:
    DECLARE_HANDLE_REGISTER_TYPE(RkAiqGainHandleInt);
};

}  // namespace RkCam

#endif
