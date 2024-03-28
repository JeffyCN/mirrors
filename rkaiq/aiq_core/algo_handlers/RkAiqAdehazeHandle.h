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
#ifndef _RK_AIQ_DEHAZE_HANDLE_INT_H_
#define _RK_AIQ_DEHAZE_HANDLE_INT_H_

#include "RkAiqHandle.h"
#include "adehaze/rk_aiq_uapi_adehaze_int.h"
#include "rk_aiq_api_private.h"
#include "rk_aiq_pool.h"
#include "xcam_mutex.h"

namespace RkCam {

class RkAiqAdehazeHandleInt : virtual public RkAiqHandle {
 public:
    explicit RkAiqAdehazeHandleInt(RkAiqAlgoDesComm* des, RkAiqCore* aiqCore)
        : RkAiqHandle(des, aiqCore) {}
    virtual ~RkAiqAdehazeHandleInt() { RkAiqHandle::deInit(); };
    virtual XCamReturn updateConfig(bool needSync);
    virtual XCamReturn prepare();
    virtual XCamReturn preProcess();
    virtual XCamReturn processing();
    virtual XCamReturn postProcess();
    virtual XCamReturn genIspResult(RkAiqFullParams* params, RkAiqFullParams* cur_params);
    // TODO add algo specific methords, this is a sample
#if RKAIQ_HAVE_DEHAZE_V10
    XCamReturn setSwAttribV10(const adehaze_sw_v10_t* att);
    XCamReturn getSwAttribV10(adehaze_sw_v10_t* att);
#endif
#if RKAIQ_HAVE_DEHAZE_V11 || RKAIQ_HAVE_DEHAZE_V11_DUO
    XCamReturn setSwAttribV11(const adehaze_sw_v11_t* att);
    XCamReturn getSwAttribV11(adehaze_sw_v11_t* att);
#endif
#if RKAIQ_HAVE_DEHAZE_V12
    XCamReturn setSwAttribV12(const adehaze_sw_v12_t* att);
    XCamReturn getSwAttribV12(adehaze_sw_v12_t* att);
#endif
#if RKAIQ_HAVE_DEHAZE_V14
    XCamReturn setSwAttribV14(const adehaze_sw_v14_t* att);
    XCamReturn getSwAttribV14(adehaze_sw_v14_t* att);
#endif

 protected:
    virtual void init();
    virtual void deInit() { RkAiqHandle::deInit(); };

 private:
    // TODO
#ifndef DISABLE_HANDLE_ATTRIB
#if RKAIQ_HAVE_DEHAZE_V10
    adehaze_sw_v10_t mCurAttV10;
    adehaze_sw_v10_t mNewAttV10;
#endif
#if RKAIQ_HAVE_DEHAZE_V11 || RKAIQ_HAVE_DEHAZE_V11_DUO
    adehaze_sw_v11_t mCurAttV11;
    adehaze_sw_v11_t mNewAttV11;
#endif
#if RKAIQ_HAVE_DEHAZE_V12
    adehaze_sw_v12_t mCurAttV12;
    adehaze_sw_v12_t mNewAttV12;
#endif
#if RKAIQ_HAVE_DEHAZE_V14
    adehaze_sw_v12_t mCurAttV14;
    adehaze_sw_v12_t mNewAttV14;
#endif
#endif

 private:
    DECLARE_HANDLE_REGISTER_TYPE(RkAiqAdehazeHandleInt);
};

}  // namespace RkCam

#endif
