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
#ifndef _RK_AIQ_GAIN_V2_HANDLE_INT_H_
#define _RK_AIQ_GAIN_V2_HANDLE_INT_H_

#include "RkAiqHandle.h"
#include "again2/rk_aiq_uapi_again_int_v2.h"
#include "rk_aiq_api_private.h"
#include "rk_aiq_pool.h"
#include "xcam_mutex.h"

namespace RkCam {
#if RKAIQ_HAVE_GAIN_V2

class RkAiqAgainV2HandleInt:
    virtual public RkAiqHandle {
public:
    explicit RkAiqAgainV2HandleInt(RkAiqAlgoDesComm* des, RkAiqCore* aiqCore)
        : RkAiqHandle(des, aiqCore) {
#ifndef DISABLE_HANDLE_ATTRIB
    updataWriteAgainInputAttr = false;
    memset(&mCurAtt, 0x00, sizeof(mCurAtt));
    memset(&mNewAtt, 0x00, sizeof(mNewAtt));
    memset(&mCurInfo, 0x00, sizeof(mCurInfo));
    memset(&mNewInfo, 0x00, sizeof(mNewInfo));
    memset(&mNewWriteInputAttr, 0x00, sizeof(mNewWriteInputAttr));
    memset(&mCurWriteInputAttr, 0x00, sizeof(mCurWriteInputAttr));
#endif // DISABLE_HANDLE_ATTRIB
        }
    virtual ~RkAiqAgainV2HandleInt() {
        RkAiqHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    virtual XCamReturn prepare();
    virtual XCamReturn preProcess();
    virtual XCamReturn processing();
    virtual XCamReturn postProcess();
    virtual XCamReturn genIspResult(RkAiqFullParams* params, RkAiqFullParams* cur_params);
    XCamReturn setAttrib(const rk_aiq_gain_attrib_v2_t *att);
    XCamReturn getAttrib(rk_aiq_gain_attrib_v2_t *att);
    XCamReturn getInfo(rk_aiq_gain_info_v2_t *pInfo);
    XCamReturn writeAginIn(rk_aiq_uapiV2_again_wrtIn_attr_t att);
protected:
    virtual void init();
    virtual void deInit() {
        RkAiqHandle::deInit();
    };
private:
#ifndef DISABLE_HANDLE_ATTRIB
    rk_aiq_gain_attrib_v2_t mCurAtt;
    rk_aiq_gain_attrib_v2_t mNewAtt;
    rk_aiq_gain_info_v2_t mCurInfo;
    rk_aiq_gain_info_v2_t mNewInfo;
    rk_aiq_uapiV2_again_wrtIn_attr_t mCurWriteInputAttr;
    rk_aiq_uapiV2_again_wrtIn_attr_t mNewWriteInputAttr;
    mutable std::atomic<bool> updataWriteAgainInputAttr;
#endif

    DECLARE_HANDLE_REGISTER_TYPE(RkAiqAgainV2HandleInt);
};
#endif
}  // namespace RkCam

#endif
