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
#ifndef _RK_AIQ_AWB_V21_HANDLE_INT_H_
#define _RK_AIQ_AWB_V21_HANDLE_INT_H_

#include "RkAiqAwbHandle.h"
#include "RkAiqHandle.h"
#include "awb/rk_aiq_uapi_awb_int.h"
#include "awb/rk_aiq_uapiv2_awb_int.h"
#include "rk_aiq_api_private.h"
#include "rk_aiq_pool.h"
#include "xcam_mutex.h"
#if RKAIQ_HAVE_AWB_V21
namespace RkCam {

class RkAiqCustomAwbHandle;
class RkAiqAwbV21HandleInt : public RkAiqAwbHandleInt {
    friend class RkAiqCustomAwbHandle;

 public:
    explicit RkAiqAwbV21HandleInt(RkAiqAlgoDesComm* des, RkAiqCore* aiqCore)
        : RkAiqAwbHandleInt(des, aiqCore) {
#ifndef DISABLE_HANDLE_ATTRIB
        memset(&mCurWbV21Attr, 0, sizeof(rk_aiq_uapiV2_wbV21_attrib_t));
        memset(&mNewWbV21Attr, 0, sizeof(rk_aiq_uapiV2_wbV21_attrib_t));
        updateWbV21Attr = false;
#endif
    };
    virtual ~RkAiqAwbV21HandleInt() {
#ifndef DISABLE_HANDLE_ATTRIB
        freeWbGainAdjustAttrib(&mNewWbV21Attr.stAuto.wbGainAdjust);
        freeWbGainAdjustAttrib(&mCurWbV21Attr.stAuto.wbGainAdjust);
#endif
        RkAiqAwbHandleInt::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    XCamReturn setWbV21Attrib(rk_aiq_uapiV2_wbV21_attrib_t att);
    XCamReturn getWbV21Attrib(rk_aiq_uapiV2_wbV21_attrib_t* att);

 protected:
 private:
#ifndef DISABLE_HANDLE_ATTRIB
    // TODO
    rk_aiq_uapiV2_wbV21_attrib_t mCurWbV21Attr;
    rk_aiq_uapiV2_wbV21_attrib_t mNewWbV21Attr;
    mutable std::atomic<bool> updateWbV21Attr;
#endif

 private:
    DECLARE_HANDLE_REGISTER_TYPE(RkAiqAwbV21HandleInt);
};

}  // namespace RkCam
#endif
#endif
