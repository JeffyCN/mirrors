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
#ifndef _RK_AIQ_AWB_V32_HANDLE_INT_H_
#define _RK_AIQ_AWB_V32_HANDLE_INT_H_

#include "RkAiqAwbHandle.h"
#include "RkAiqHandle.h"
#include "awb/rk_aiq_uapi_awb_int.h"
#include "awb/rk_aiq_uapiv2_awb_int.h"
#include "rk_aiq_api_private.h"
#include "rk_aiq_pool.h"
#include "xcam_mutex.h"
#if RKAIQ_HAVE_AWB_V32
namespace RkCam {

class RkAiqCustomAwbHandle;
class RkAiqAwbV32HandleInt : public RkAiqAwbHandleInt {
    friend class RkAiqCustomAwbHandle;

 public:
    explicit RkAiqAwbV32HandleInt(RkAiqAlgoDesComm* des, RkAiqCore* aiqCore)
        : RkAiqAwbHandleInt(des, aiqCore) {
#ifndef DISABLE_HANDLE_ATTRIB
        memset(&mCurWbV32Attr, 0, sizeof(rk_aiq_uapiV2_wbV32_attrib_t));
        memset(&mNewWbV32Attr, 0, sizeof(rk_aiq_uapiV2_wbV32_attrib_t));
        memset(&mCurWbV32AwbMultiWindowAttr, 0, sizeof(rk_aiq_uapiV2_wbV32_awb_mulWindow_t));
        memset(&mNewWbV32AwbMultiWindowAttr, 0, sizeof(rk_aiq_uapiV2_wbV32_awb_mulWindow_t));
        memset(&mCurWriteAwbInputAttr, 0, sizeof(rk_aiq_uapiV2_awb_wrtIn_attr_t));
        memset(&mNewWriteAwbInputAttr, 0, sizeof(rk_aiq_uapiV2_awb_wrtIn_attr_t));
        updateWbV32Attr = false;
        updateWbV32AwbMultiWindowAttr = false;
        updateWriteAwbInputAttr = false;
#endif
    };
    virtual ~RkAiqAwbV32HandleInt() {
        RkAiqAwbHandleInt::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    XCamReturn setWbV32Attrib(rk_aiq_uapiV2_wbV32_attrib_t att);
    XCamReturn getWbV32Attrib(rk_aiq_uapiV2_wbV32_attrib_t* att);
    XCamReturn setWbV32AwbMultiWindowAttrib(rk_aiq_uapiV2_wbV32_awb_mulWindow_t att);
    XCamReturn getWbV32AwbMultiWindowAttrib(rk_aiq_uapiV2_wbV32_awb_mulWindow_t* att);
    XCamReturn writeAwbIn(rk_aiq_uapiV2_awb_wrtIn_attr_t att);
    XCamReturn setWbV32IQAutoExtPara(const rk_aiq_uapiV2_Wb_Awb_IqAtExtPa_V32_t* att);
    XCamReturn getWbV32IQAutoExtPara(rk_aiq_uapiV2_Wb_Awb_IqAtExtPa_V32_t* att);
    XCamReturn setWbV32IQAutoPara(const rk_aiq_uapiV2_Wb_Awb_IqAtPa_V32_t* att);
    XCamReturn getWbV32IQAutoPara(rk_aiq_uapiV2_Wb_Awb_IqAtPa_V32_t* att);
    XCamReturn awbIqMap2Main( rk_aiq_uapiV2_awb_Slave2Main_Cfg_t att);
    XCamReturn setAwbPreWbgain( const float att[4]);
 protected:
 private:
    // TODO
#ifndef DISABLE_HANDLE_ATTRIB
    rk_aiq_uapiV2_wbV32_attrib_t mCurWbV32Attr;
    rk_aiq_uapiV2_wbV32_attrib_t mNewWbV32Attr;
    mutable std::atomic<bool> updateWbV32Attr;
    rk_aiq_uapiV2_wbV32_awb_mulWindow_t mCurWbV32AwbMultiWindowAttr;
    rk_aiq_uapiV2_wbV32_awb_mulWindow_t mNewWbV32AwbMultiWindowAttr;
    mutable std::atomic<bool> updateWbV32AwbMultiWindowAttr;
    rk_aiq_uapiV2_awb_wrtIn_attr_t mCurWriteAwbInputAttr;
    rk_aiq_uapiV2_awb_wrtIn_attr_t mNewWriteAwbInputAttr;
    mutable std::atomic<bool>  updateWriteAwbInputAttr;
#endif
 private:
    DECLARE_HANDLE_REGISTER_TYPE(RkAiqAwbV32HandleInt);
};

}  // namespace RkCam
#endif
#endif
