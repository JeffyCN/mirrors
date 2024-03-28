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
#ifndef _RK_AIQ_DEBAYER_HANDLE_INT_H_
#define _RK_AIQ_DEBAYER_HANDLE_INT_H_

#include "RkAiqHandle.h"
#include "adebayer/rk_aiq_uapi_adebayer_int.h"
#include "rk_aiq_api_private.h"
#include "rk_aiq_pool.h"
#include "xcam_mutex.h"

namespace RkCam {

class RkAiqAdebayerHandleInt : virtual public RkAiqHandle {
public:
    explicit RkAiqAdebayerHandleInt(RkAiqAlgoDesComm* des, RkAiqCore* aiqCore)
        : RkAiqHandle(des, aiqCore) {}
    virtual ~RkAiqAdebayerHandleInt() {
        RkAiqHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    virtual XCamReturn prepare();
    virtual XCamReturn preProcess();
    virtual XCamReturn processing();
    virtual XCamReturn postProcess();
    virtual XCamReturn genIspResult(RkAiqFullParams* params, RkAiqFullParams* cur_params);
    // TODO add algo specific methords, this is a sample
#if RKAIQ_HAVE_DEBAYER_V1
    XCamReturn setAttrib(adebayer_attrib_t att);
    XCamReturn getAttrib(adebayer_attrib_t* att);
#endif

#if RKAIQ_HAVE_DEBAYER_V2
    XCamReturn setAttribV2(adebayer_v2_attrib_t att);
    XCamReturn getAttribV2(adebayer_v2_attrib_t* att);
#endif

#if RKAIQ_HAVE_DEBAYER_V2_LITE
    XCamReturn setAttribV2(adebayer_v2lite_attrib_t att);
    XCamReturn getAttribV2(adebayer_v2lite_attrib_t* att);
#endif

#if RKAIQ_HAVE_DEBAYER_V3
    XCamReturn setAttribV3(adebayer_v3_attrib_t att);
    XCamReturn getAttribV3(adebayer_v3_attrib_t* att);
#endif

protected:
    virtual void init();
    virtual void deInit() {
        RkAiqHandle::deInit();
    };

private:

#ifndef DISABLE_HANDLE_ATTRIB
#if RKAIQ_HAVE_DEBAYER_V1
    adebayer_attrib_t mCurAtt;
    adebayer_attrib_t mNewAtt;
#endif

#if RKAIQ_HAVE_DEBAYER_V2
    adebayer_v2_attrib_t mCurAttV2;
    adebayer_v2_attrib_t mNewAttV2;
#endif

#if RKAIQ_HAVE_DEBAYER_V2_LITE
    adebayer_v2lite_attrib_t mCurAttV2Lite;
    adebayer_v2lite_attrib_t mNewAttV2Lite;
#endif

#if RKAIQ_HAVE_DEBAYER_V3
    adebayer_v3_attrib_t mCurAttV3;
    adebayer_v3_attrib_t mNewAttV3;
#endif


#endif

private:
    DECLARE_HANDLE_REGISTER_TYPE(RkAiqAdebayerHandleInt);
};

}  // namespace RkCam

#endif
