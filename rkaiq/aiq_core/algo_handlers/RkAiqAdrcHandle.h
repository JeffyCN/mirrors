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
#ifndef _RK_AIQ_DRC_HANDLE_INT_H_
#define _RK_AIQ_DRC_HANDLE_INT_H_

#include "RkAiqHandle.h"
#include "adrc/rk_aiq_uapi_adrc_int.h"
#include "rk_aiq_api_private.h"
#include "rk_aiq_pool.h"
#include "xcam_mutex.h"

namespace RkCam {

class RkAiqAdrcHandleInt : virtual public RkAiqHandle {
 public:
    explicit RkAiqAdrcHandleInt(RkAiqAlgoDesComm* des, RkAiqCore* aiqCore)
        : RkAiqHandle(des, aiqCore) {}
    virtual ~RkAiqAdrcHandleInt() { RkAiqHandle::deInit(); };
    virtual XCamReturn updateConfig(bool needSync);
    virtual XCamReturn prepare();
    virtual XCamReturn preProcess();
    virtual XCamReturn processing();
    virtual XCamReturn postProcess();
    virtual XCamReturn genIspResult(RkAiqFullParams* params, RkAiqFullParams* cur_params);
#if RKAIQ_HAVE_DRC_V10
    XCamReturn setAttribV10(const drcAttrV10_t* att);
    XCamReturn getAttribV10(drcAttrV10_t* att);
#endif
#if RKAIQ_HAVE_DRC_V11
    XCamReturn setAttribV11(const drcAttrV11_t* att);
    XCamReturn getAttribV11(drcAttrV11_t* att);
#endif
#if RKAIQ_HAVE_DRC_V12
    XCamReturn setAttribV12(const drcAttrV12_t* att);
    XCamReturn getAttribV12(drcAttrV12_t* att);
#endif
#if RKAIQ_HAVE_DRC_V12_LITE
    XCamReturn setAttribV12Lite(const drcAttrV12Lite_t* att);
    XCamReturn getAttribV12Lite(drcAttrV12Lite_t* att);
#endif
#if RKAIQ_HAVE_DRC_V20
    XCamReturn setAttribV20(const drcAttrV20_t* att);
    XCamReturn getAttribV20(drcAttrV20_t* att);
#endif

    void setAeProcRes(RkAiqAlgoProcResAeShared_t* aeProcRes) {
        mAeProcRes = *aeProcRes;
    }

 protected:
    virtual void init();
    virtual void deInit() { RkAiqHandle::deInit(); };

 private:
#ifndef DISABLE_HANDLE_ATTRIB
#if RKAIQ_HAVE_DRC_V10
    drcAttrV10_t mCurAttV10;
    drcAttrV10_t mNewAttV10;
#endif
#if RKAIQ_HAVE_DRC_V11
    drcAttrV11_t mCurAttV11;
    drcAttrV11_t mNewAttV11;
#endif
#if RKAIQ_HAVE_DRC_V12
    drcAttrV12_t mCurAttV12;
    drcAttrV12_t mNewAttV12;
#endif
#if RKAIQ_HAVE_DRC_V12_LITE
    drcAttrV12Lite_t mCurAttV12Lite;
    drcAttrV12Lite_t mNewAttV12Lite;
#endif
#if RKAIQ_HAVE_DRC_V20
    drcAttrV20_t mCurAttV20;
    drcAttrV20_t mNewAttV20;
#endif
#endif
    RkAiqAlgoProcResAeShared_t mAeProcRes;

 private:
    DECLARE_HANDLE_REGISTER_TYPE(RkAiqAdrcHandleInt);
};

}  // namespace RkCam

#endif
