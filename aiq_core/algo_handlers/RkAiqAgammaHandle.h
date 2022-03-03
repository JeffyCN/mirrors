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
#ifndef _RK_AIQ_GAMMA_HANDLE_INT_H_
#define _RK_AIQ_GAMMA_HANDLE_INT_H_

#include "RkAiqHandle.h"
#include "agamma/rk_aiq_uapi_agamma_int.h"
#include "rk_aiq_api_private.h"
#include "rk_aiq_pool.h"
#include "xcam_mutex.h"

namespace RkCam {

class RkAiqAgammaHandleInt : virtual public RkAiqHandle {
 public:
    explicit RkAiqAgammaHandleInt(RkAiqAlgoDesComm* des, RkAiqCore* aiqCore)
        : RkAiqHandle(des, aiqCore) {
        memset(&mCurAtt, 0, sizeof(rk_aiq_gamma_attrib_V2_t));
        memset(&mNewAtt, 0, sizeof(rk_aiq_gamma_attrib_V2_t));
    };
    virtual ~RkAiqAgammaHandleInt() { RkAiqHandle::deInit(); };
    virtual XCamReturn updateConfig(bool needSync);
    virtual XCamReturn prepare();
    virtual XCamReturn preProcess();
    virtual XCamReturn processing();
    virtual XCamReturn postProcess();
    virtual XCamReturn genIspResult(RkAiqFullParams* params, RkAiqFullParams* cur_params);
    // TODO add algo specific methords, this is a sample
    XCamReturn setAttrib(rk_aiq_gamma_attrib_V2_t att);
    XCamReturn getAttrib(rk_aiq_gamma_attrib_V2_t* att);
    // XCamReturn queryLscInfo(rk_aiq_lsc_querry_info_t *lsc_querry_info );

 protected:
    virtual void init();
    virtual void deInit() { RkAiqHandle::deInit(); };

 private:
    // TODO
    rk_aiq_gamma_attrib_V2_t mCurAtt;
    rk_aiq_gamma_attrib_V2_t mNewAtt;

 private:
    DECLARE_HANDLE_REGISTER_TYPE(RkAiqAgammaHandleInt);
};

};  // namespace RkCam

#endif
