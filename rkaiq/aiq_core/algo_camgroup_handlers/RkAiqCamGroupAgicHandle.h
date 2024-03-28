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
#ifndef _RK_AIQ_CAMGROUP_AGIC_HANDLE_INT_H_
#define _RK_AIQ_CAMGROUP_AGIC_HANDLE_INT_H_

#include "RkAiqCamGroupHandleInt.h"
#include "agic/rk_aiq_uapi_agic_int.h"

namespace RkCam {

#if RKAIQ_HAVE_GIC
class RkAiqCamGroupAgicHandleInt : public RkAiqCamgroupHandle {
 public:
    explicit RkAiqCamGroupAgicHandleInt(RkAiqAlgoDesComm* des, RkAiqCamGroupManager* camGroupMg)
        : RkAiqCamgroupHandle(des, camGroupMg) {
        updateAttV1 = false;
        updateAttV2 = false;
        memset(&mCurAttV1, 0, sizeof(rkaiq_gic_v1_api_attr_t));
        memset(&mNewAttV1, 0, sizeof(rkaiq_gic_v1_api_attr_t));
        memset(&mCurAttV2, 0, sizeof(rkaiq_gic_v2_api_attr_t));
        memset(&mNewAttV2, 0, sizeof(rkaiq_gic_v2_api_attr_t));
    };
    virtual ~RkAiqCamGroupAgicHandleInt() { RkAiqCamgroupHandle::deInit(); };
    virtual XCamReturn updateConfig(bool needSync);
    XCamReturn setAttribV1(const rkaiq_gic_v1_api_attr_t* att);
    XCamReturn getAttribV1(rkaiq_gic_v1_api_attr_t* att);
    XCamReturn setAttribV2(const rkaiq_gic_v2_api_attr_t* att);
    XCamReturn getAttribV2(rkaiq_gic_v2_api_attr_t* att);

 protected:
 private:
    mutable std::atomic<bool> updateAttV1;
    mutable std::atomic<bool> updateAttV2;
    rkaiq_gic_v1_api_attr_t mCurAttV1;
    rkaiq_gic_v1_api_attr_t mNewAttV1;
    rkaiq_gic_v2_api_attr_t mCurAttV2;
    rkaiq_gic_v2_api_attr_t mNewAttV2;
};
#endif

}  // namespace RkCam

#endif  // _RK_AIQ_CAMGROUP_AGIC_HANDLE_INT_H_
