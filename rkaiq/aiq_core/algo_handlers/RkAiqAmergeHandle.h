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
#ifndef _RK_AIQ_MERGE_HANDLE_INT_H_
#define _RK_AIQ_MERGE_HANDLE_INT_H_

#include "RkAiqHandle.h"
#include "amerge/rk_aiq_uapi_amerge_int.h"
#include "rk_aiq_api_private.h"
#include "rk_aiq_pool.h"
#include "xcam_mutex.h"

namespace RkCam {

class RkAiqAmergeHandleInt : virtual public RkAiqHandle {
 public:
    explicit RkAiqAmergeHandleInt(RkAiqAlgoDesComm* des, RkAiqCore* aiqCore)
        : RkAiqHandle(des, aiqCore) {}
    virtual ~RkAiqAmergeHandleInt() { RkAiqHandle::deInit(); };
    virtual XCamReturn updateConfig(bool needSync);
    virtual XCamReturn prepare();
    virtual XCamReturn preProcess();
    virtual XCamReturn processing();
    virtual XCamReturn postProcess();
    virtual XCamReturn genIspResult(RkAiqFullParams* params, RkAiqFullParams* cur_params);
#if RKAIQ_HAVE_MERGE_V10
    XCamReturn setAttribV10(const mergeAttrV10_t* att);
    XCamReturn getAttribV10(mergeAttrV10_t* att);
#endif
#if RKAIQ_HAVE_MERGE_V11
    XCamReturn setAttribV11(const mergeAttrV11_t* att);
    XCamReturn getAttribV11(mergeAttrV11_t* att);
#endif
#if RKAIQ_HAVE_MERGE_V12
    XCamReturn setAttribV12(const mergeAttrV12_t* att);
    XCamReturn getAttribV12(mergeAttrV12_t* att);
#endif

    void setAeProcRes(RkAiqAlgoProcResAeShared_t* aeProcRes) {
        mAeProcRes = *aeProcRes;
    }

 protected:
    virtual void init();
    virtual void deInit() { RkAiqHandle::deInit(); };

 private:
#ifndef DISABLE_HANDLE_ATTRIB
#if RKAIQ_HAVE_MERGE_V10
    mergeAttrV10_t mCurAttV10;
    mergeAttrV10_t mNewAttV10;
#endif
#if RKAIQ_HAVE_MERGE_V11
    mergeAttrV11_t mCurAttV11;
    mergeAttrV11_t mNewAttV11;
#endif
#if RKAIQ_HAVE_MERGE_V12
    mergeAttrV12_t mCurAttV12;
    mergeAttrV12_t mNewAttV12;
#endif
#endif
    RkAiqAlgoProcResAeShared_t mAeProcRes;

 private:
    DECLARE_HANDLE_REGISTER_TYPE(RkAiqAmergeHandleInt);
};

}  // namespace RkCam

#endif
