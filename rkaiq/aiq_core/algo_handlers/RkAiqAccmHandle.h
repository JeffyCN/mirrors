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
#ifndef _RK_AIQ_CCM_HANDLE_INT_H_
#define _RK_AIQ_CCM_HANDLE_INT_H_

#include "RkAiqHandle.h"
#include "accm/rk_aiq_uapi_accm_int.h"
#include "rk_aiq_api_private.h"
#include "rk_aiq_pool.h"
#include "xcam_mutex.h"

namespace RkCam {
#if RKAIQ_HAVE_CCM_V1 || RKAIQ_HAVE_CCM_V2
class RkAiqAccmHandleInt : virtual public RkAiqHandle {
 public:
    explicit RkAiqAccmHandleInt(RkAiqAlgoDesComm* des, RkAiqCore* aiqCore)
        : RkAiqHandle(des, aiqCore) {
#if RKAIQ_HAVE_CCM_V1
        memset(&mCurAtt, 0, sizeof(rk_aiq_ccm_attrib_t));
        memset(&mNewAtt, 0, sizeof(rk_aiq_ccm_attrib_t));
#endif
#if RKAIQ_HAVE_CCM_V2
        memset(&mCurAttV2, 0, sizeof(rk_aiq_ccm_v2_attrib_t));
        memset(&mNewAttV2, 0, sizeof(rk_aiq_ccm_v2_attrib_t));
#endif
    };
    virtual ~RkAiqAccmHandleInt() { RkAiqHandle::deInit(); };
    virtual XCamReturn updateConfig(bool needSync);
    virtual XCamReturn prepare();
    virtual XCamReturn preProcess();
    virtual XCamReturn processing();
    virtual XCamReturn postProcess();
    virtual XCamReturn genIspResult(RkAiqFullParams* params, RkAiqFullParams* cur_params);
    // TODO add algo specific methords, this is a sample
#if RKAIQ_HAVE_CCM_V1
    XCamReturn setAttrib(const rk_aiq_ccm_attrib_t* att);
    XCamReturn getAttrib(rk_aiq_ccm_attrib_t* att);
#endif
#if RKAIQ_HAVE_CCM_V2
    XCamReturn setAttribV2(const rk_aiq_ccm_v2_attrib_t* att);
    XCamReturn getAttribV2(rk_aiq_ccm_v2_attrib_t* att);
#endif
    XCamReturn queryCcmInfo(rk_aiq_ccm_querry_info_t* ccm_querry_info);

 protected:
    virtual void init();
    virtual void deInit() { RkAiqHandle::deInit(); };

 private:
    // TODO
#if RKAIQ_HAVE_CCM_V1
    rk_aiq_ccm_attrib_t mCurAtt;
    rk_aiq_ccm_attrib_t mNewAtt;
#endif
#if RKAIQ_HAVE_CCM_V2
    rk_aiq_ccm_v2_attrib_t mCurAttV2;
    rk_aiq_ccm_v2_attrib_t mNewAttV2;
#endif
 private:
    DECLARE_HANDLE_REGISTER_TYPE(RkAiqAccmHandleInt);
};
#endif
}  // namespace RkCam

#endif
