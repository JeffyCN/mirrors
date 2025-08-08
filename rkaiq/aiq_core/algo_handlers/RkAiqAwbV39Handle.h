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
#ifndef _RK_AIQ_AWB_V39_HANDLE_INT_H_
#define _RK_AIQ_AWB_V39_HANDLE_INT_H_

#include "RkAiqAwbHandle.h"
#include "RkAiqHandle.h"
#include "awb/rk_aiq_uapi_awb_int.h"
#include "awb/rk_aiq_uapiv2_awb_int.h"
#include "rk_aiq_api_private.h"
#include "rk_aiq_pool.h"
#include "xcam_mutex.h"
#if RKAIQ_HAVE_AWB_V39
namespace RkCam {

class RkAiqCustomAwbHandle;
class RkAiqAwbV39HandleInt : public RkAiqAwbHandleInt {
    friend class RkAiqCustomAwbHandle;

public:
    explicit RkAiqAwbV39HandleInt(RkAiqAlgoDesComm* des, RkAiqCore* aiqCore)
        : RkAiqAwbHandleInt(des, aiqCore) {

    };
    virtual ~RkAiqAwbV39HandleInt() {
        RkAiqAwbHandleInt::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    XCamReturn setWbV39Attrib(rk_aiq_uapiV2_wbV39_attrib_t att);
    XCamReturn getWbV39Attrib(rk_aiq_uapiV2_wbV39_attrib_t* att);
    XCamReturn setWbV39AwbMultiWindowAttrib(rk_aiq_uapiV2_wbV39_awb_nonROI_t att) ;
    XCamReturn getWbV39AwbMultiWindowAttrib(rk_aiq_uapiV2_wbV39_awb_nonROI_t* att);
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

private:
    DECLARE_HANDLE_REGISTER_TYPE(RkAiqAwbV39HandleInt);
};

}  // namespace RkCam
#endif
#endif
