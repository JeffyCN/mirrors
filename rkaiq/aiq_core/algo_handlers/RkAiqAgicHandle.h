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
#ifndef _RK_AIQ_GIC_HANDLE_INT_H_
#define _RK_AIQ_GIC_HANDLE_INT_H_

#include "RkAiqHandle.h"
#include "agic/rk_aiq_uapi_agic_int.h"
#include "rk_aiq_api_private.h"
#include "rk_aiq_pool.h"
#include "xcam_mutex.h"

namespace RkCam {

class RkAiqAgicHandleInt : virtual public RkAiqHandle {
 public:
    explicit RkAiqAgicHandleInt(RkAiqAlgoDesComm* des, RkAiqCore* aiqCore)
        : RkAiqHandle(des, aiqCore) {
#ifndef DISABLE_HANDLE_ATTRIB
        updateAttV1 = false;
        updateAttV2 = false;
        memset(&mCurAttV1, 0, sizeof(rkaiq_gic_v1_api_attr_t));
        memset(&mNewAttV1, 0, sizeof(rkaiq_gic_v1_api_attr_t));
        memset(&mCurAttV2, 0, sizeof(rkaiq_gic_v2_api_attr_t));
        memset(&mNewAttV2, 0, sizeof(rkaiq_gic_v2_api_attr_t));
#endif
    }
    virtual ~RkAiqAgicHandleInt() { RkAiqHandle::deInit(); };
    virtual XCamReturn updateConfig(bool needSync);
    virtual XCamReturn prepare();
    virtual XCamReturn preProcess();
    virtual XCamReturn processing();
    virtual XCamReturn postProcess();
    virtual XCamReturn genIspResult(RkAiqFullParams* params, RkAiqFullParams* cur_params);
    XCamReturn setAttribV1(const rkaiq_gic_v1_api_attr_t* att);
    XCamReturn getAttribV1(rkaiq_gic_v1_api_attr_t* att);
    XCamReturn setAttribV2(const rkaiq_gic_v2_api_attr_t* att);
    XCamReturn getAttribV2(rkaiq_gic_v2_api_attr_t* att);

 protected:
    virtual void init();
    virtual void deInit() { RkAiqHandle::deInit(); };

 private:
#ifndef DISABLE_HANDLE_ATTRIB
    mutable std::atomic<bool> updateAttV1;
    mutable std::atomic<bool> updateAttV2;
    rkaiq_gic_v1_api_attr_t mCurAttV1;
    rkaiq_gic_v1_api_attr_t mNewAttV1;
    rkaiq_gic_v2_api_attr_t mCurAttV2;
    rkaiq_gic_v2_api_attr_t mNewAttV2;
#endif

 private:
    DECLARE_HANDLE_REGISTER_TYPE(RkAiqAgicHandleInt);
};

}  // namespace RkCam

#endif
