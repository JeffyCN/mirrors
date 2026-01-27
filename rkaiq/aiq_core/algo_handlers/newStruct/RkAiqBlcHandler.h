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
#ifndef _RK_AIQ_BLC_HANDLE_INT_H_
#define _RK_AIQ_BLC_HANDLE_INT_H_

#include "RkAiqHandle.h"
#if RKAIQ_HAVE_BLC_V1
#include "isp/rk_aiq_isp_blc.h"
#elif RKAIQ_HAVE_BLC_V32
#include "isp/rk_aiq_isp_blc30.h"
#else
#error "wrong blc hw version !"
#endif
#include "algos/rk_aiq_api_types_blc.h"
#include "rk_aiq_api_private.h"
#include "rk_aiq_pool.h"
#include "xcam_mutex.h"

namespace RkCam {

class RkAiqBlcHandleInt : virtual public RkAiqHandle {
public:
    explicit RkAiqBlcHandleInt(RkAiqAlgoDesComm* des, RkAiqCore* aiqCore)
        : RkAiqHandle(des, aiqCore) {
            mLatestparam = NULL;
            mLatesten    = false;
        }
    virtual ~RkAiqBlcHandleInt() {
        RkAiqHandle::deInit();
    };
    virtual XCamReturn prepare();
    virtual XCamReturn processing();
    virtual XCamReturn preProcess() { return XCAM_RETURN_NO_ERROR;  };
    virtual XCamReturn postProcess() { return XCAM_RETURN_NO_ERROR;  };
    virtual XCamReturn genIspResult(RkAiqFullParams* params, RkAiqFullParams* cur_params);

    XCamReturn setAttrib(blc_api_attrib_t* attr);
    XCamReturn getAttrib(blc_api_attrib_t* attr);
    XCamReturn queryStatus(blc_status_t* status);

protected:
    virtual void init();
    virtual void deInit() {
        RkAiqHandle::deInit();
    };

private:
    blc_param_t* mLatestparam;
    bool mLatesten;

private:
    DECLARE_HANDLE_REGISTER_TYPE(RkAiqBlcHandleInt);
};

}  // namespace RkCam

#endif
