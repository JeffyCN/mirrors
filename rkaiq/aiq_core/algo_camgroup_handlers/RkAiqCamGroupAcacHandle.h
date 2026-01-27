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
#ifndef _RK_AIQ_CAMGROUP_ACAC_HANDLE_INT_H_
#define _RK_AIQ_CAMGROUP_ACAC_HANDLE_INT_H_

#include <atomic>

#include "RkAiqCamGroupHandleInt.h"
#include "acac/rk_aiq_uapi_acac_int.h"

namespace RkCam {

class RkAiqCamGroupAcacHandleInt : public RkAiqCamgroupHandle {
 public:
    explicit RkAiqCamGroupAcacHandleInt(RkAiqAlgoDesComm* des, RkAiqCamGroupManager* camGroupMg)
        : RkAiqCamgroupHandle(des, camGroupMg) {
        updateAttV03 = false;
        updateAttV10 = false;
        updateAttV11 = false;
        memset(&mCurAttV03, 0, sizeof(rkaiq_cac_v03_api_attr_t));
        memset(&mNewAttV03, 0, sizeof(rkaiq_cac_v03_api_attr_t));
        memset(&mCurAttV10, 0, sizeof(rkaiq_cac_v10_api_attr_t));
        memset(&mNewAttV10, 0, sizeof(rkaiq_cac_v10_api_attr_t));
        memset(&mCurAttV11, 0, sizeof(rkaiq_cac_v11_api_attr_t));
        memset(&mNewAttV11, 0, sizeof(rkaiq_cac_v11_api_attr_t));
    };
    virtual ~RkAiqCamGroupAcacHandleInt() { RkAiqCamgroupHandle::deInit(); };
    virtual XCamReturn updateConfig(bool needSync);
    XCamReturn setAttribV03(const rkaiq_cac_v03_api_attr_t* att);
    XCamReturn getAttribV03(rkaiq_cac_v03_api_attr_t* att);
    XCamReturn setAttribV10(const rkaiq_cac_v10_api_attr_t* att);
    XCamReturn getAttribV10(rkaiq_cac_v10_api_attr_t* att);
    XCamReturn setAttribV11(const rkaiq_cac_v11_api_attr_t* att);
    XCamReturn getAttribV11(rkaiq_cac_v11_api_attr_t* att);

 protected:
 private:
    mutable std::atomic<bool> updateAttV03;
    mutable std::atomic<bool> updateAttV10;
    mutable std::atomic<bool> updateAttV11;
    rkaiq_cac_v03_api_attr_t mCurAttV03;
    rkaiq_cac_v03_api_attr_t mNewAttV03;
    rkaiq_cac_v10_api_attr_t mCurAttV10;
    rkaiq_cac_v10_api_attr_t mNewAttV10;
    rkaiq_cac_v11_api_attr_t mCurAttV11;
    rkaiq_cac_v11_api_attr_t mNewAttV11;
};

}  // namespace RkCam

#endif  // _RK_AIQ_CAMGROUP_ACAC_HANDLE_INT_H_
