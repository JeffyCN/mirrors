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
#ifndef _RK_AIQ_EIS_HANDLE_INT_H_
#define _RK_AIQ_EIS_HANDLE_INT_H_

#include "RkAiqHandle.h"
#include "aeis/rk_aiq_uapi_aeis_int.h"
#include "rk_aiq_api_private.h"
#include "rk_aiq_pool.h"
#include "xcam_mutex.h"

namespace RkCam {

#if RKAIQ_HAVE_EIS_V1
class RkAiqAeisHandleInt : virtual public RkAiqHandle {
 public:
    explicit RkAiqAeisHandleInt(RkAiqAlgoDesComm* des, RkAiqCore* aiqCore)
        : RkAiqHandle(des, aiqCore) {
#ifndef DISABLE_HANDLE_ATTRIB
        memset(&mCurAtt, 0, sizeof(rk_aiq_eis_attrib_t));
        memset(&mNewAtt, 0, sizeof(rk_aiq_eis_attrib_t));
        mCurAtt.en = 0xff;
#endif
    };
    virtual ~RkAiqAeisHandleInt() { RkAiqHandle::deInit(); };
    virtual XCamReturn updateConfig(bool needSync);
    virtual XCamReturn prepare();
    virtual XCamReturn preProcess();
    virtual XCamReturn processing();
    virtual XCamReturn postProcess();
    virtual XCamReturn genIspResult(RkAiqFullParams* params, RkAiqFullParams* cur_params);

    XCamReturn setAttrib(rk_aiq_eis_attrib_t att);
    XCamReturn getAttrib(rk_aiq_eis_attrib_t* att);

 protected:
    virtual void init();
    virtual void deInit() { RkAiqHandle::deInit(); };

 private:
#ifndef DISABLE_HANDLE_ATTRIB
    rk_aiq_eis_attrib_t mCurAtt;
    rk_aiq_eis_attrib_t mNewAtt;
#endif

 private:
    DECLARE_HANDLE_REGISTER_TYPE(RkAiqAeisHandleInt);
};

#endif

}  // namespace RkCam

#endif
