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
#ifndef _RK_AIQ_CGC_HANDLE_INT_H_
#define _RK_AIQ_CGC_HANDLE_INT_H_

#include "RkAiqHandle.h"
#include "acgc/rk_aiq_uapi_acgc_int.h"
#include "rk_aiq_api_private.h"
#include "rk_aiq_pool.h"
#include "xcam_mutex.h"

namespace RkCam {
#if RKAIQ_HAVE_CGC_V1
class RkAiqAcgcHandleInt : virtual public RkAiqHandle {
 public:
    explicit RkAiqAcgcHandleInt(RkAiqAlgoDesComm* des, RkAiqCore* aiqCore)
        : RkAiqHandle(des, aiqCore){};
    virtual ~RkAiqAcgcHandleInt() { RkAiqHandle::deInit(); };
    virtual XCamReturn prepare();
    virtual XCamReturn preProcess();
    virtual XCamReturn processing();
    virtual XCamReturn postProcess();
    virtual XCamReturn updateConfig(bool needSync);
    XCamReturn setAttrib(const rk_aiq_uapi_acgc_attrib_t* att);
    XCamReturn getAttrib(rk_aiq_uapi_acgc_attrib_t* att);
    virtual XCamReturn genIspResult(RkAiqFullParams* params, RkAiqFullParams* cur_params);

 protected:
    virtual void init();
    virtual void deInit() { RkAiqHandle::deInit(); };

 private:
    DECLARE_HANDLE_REGISTER_TYPE(RkAiqAcgcHandleInt);

#ifndef DISABLE_HANDLE_ATTRIB
    rk_aiq_uapi_acgc_attrib_t mCurAtt;
    rk_aiq_uapi_acgc_attrib_t mNewAtt;
#endif
};
#endif

}  // namespace RkCam

#endif
