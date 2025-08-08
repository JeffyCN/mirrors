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
#ifndef _RK_AIQ_GAMMA_HANDLE_INT_H_
#define _RK_AIQ_GAMMA_HANDLE_INT_H_
#include "RkAiqHandle.h"
#include "agamma/rk_aiq_uapi_agamma_int.h"
#include "rk_aiq_api_private.h"
#include "rk_aiq_pool.h"
#include "xcam_mutex.h"

namespace RkCam {

class RkAiqAgammaHandleInt : virtual public RkAiqHandle {
 public:
    explicit RkAiqAgammaHandleInt(RkAiqAlgoDesComm* des, RkAiqCore* aiqCore)
        : RkAiqHandle(des, aiqCore) {
#ifndef DISABLE_HANDLE_ATTRIB
#if RKAIQ_HAVE_GAMMA_V10
        memset(&mCurAttV10, 0, sizeof(rk_aiq_gamma_v10_attr_t));
        memset(&mNewAttV10, 0, sizeof(rk_aiq_gamma_v10_attr_t));
#endif
#if RKAIQ_HAVE_GAMMA_V11
        memset(&mCurAttV11, 0, sizeof(rk_aiq_gamma_v11_attr_t));
        memset(&mNewAttV11, 0, sizeof(rk_aiq_gamma_v11_attr_t));
#endif
#endif
    };
    virtual ~RkAiqAgammaHandleInt() { RkAiqHandle::deInit(); };
    virtual XCamReturn updateConfig(bool needSync);
    virtual XCamReturn prepare();
    virtual XCamReturn preProcess();
    virtual XCamReturn processing();
    virtual XCamReturn postProcess();
    virtual XCamReturn genIspResult(RkAiqFullParams* params, RkAiqFullParams* cur_params);
#if RKAIQ_HAVE_GAMMA_V10
    XCamReturn setAttribV10(const rk_aiq_gamma_v10_attr_t* att);
    XCamReturn getAttribV10(rk_aiq_gamma_v10_attr_t* att);
#endif
#if RKAIQ_HAVE_GAMMA_V11
    XCamReturn setAttribV11(const rk_aiq_gamma_v11_attr_t* att);
    XCamReturn getAttribV11(rk_aiq_gamma_v11_attr_t* att);
#endif

 protected:
    virtual void init();
    virtual void deInit() { RkAiqHandle::deInit(); };

 private:
#ifndef DISABLE_HANDLE_ATTRIB
#if RKAIQ_HAVE_GAMMA_V10
    rk_aiq_gamma_v10_attr_t mCurAttV10;
    rk_aiq_gamma_v10_attr_t mNewAttV10;
#endif
#if RKAIQ_HAVE_GAMMA_V11
    rk_aiq_gamma_v11_attr_t mCurAttV11;
    rk_aiq_gamma_v11_attr_t mNewAttV11;
#endif
#endif

 private:
    DECLARE_HANDLE_REGISTER_TYPE(RkAiqAgammaHandleInt);
};

}  // namespace RkCam

#endif