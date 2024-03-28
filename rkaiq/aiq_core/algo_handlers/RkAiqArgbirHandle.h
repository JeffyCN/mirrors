/*
 * Copyright (c) 2024 Rockchip Eletronics Co., Ltd.
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
#ifndef _RK_AIQ_RGBIR_HANDLE_INT_H_
#define _RK_AIQ_RGBIR_HANDLE_INT_H_

#include "RkAiqHandle.h"
#include "argbir/rk_aiq_uapi_argbir_int.h"
#include "rk_aiq_api_private.h"
#include "rk_aiq_pool.h"
#include "xcam_mutex.h"

namespace RkCam {

class RkAiqArgbirHandleInt : virtual public RkAiqHandle {
 public:
    explicit RkAiqArgbirHandleInt(RkAiqAlgoDesComm* des, RkAiqCore* aiqCore)
        : RkAiqHandle(des, aiqCore) {}
    virtual ~RkAiqArgbirHandleInt() { RkAiqHandle::deInit(); };
    virtual XCamReturn updateConfig(bool needSync);
    virtual XCamReturn prepare();
    virtual XCamReturn preProcess();
    virtual XCamReturn processing();
    virtual XCamReturn postProcess();
    virtual XCamReturn genIspResult(RkAiqFullParams* params, RkAiqFullParams* cur_params);
#if RKAIQ_HAVE_RGBIR_REMOSAIC_V10
    XCamReturn setAttribV10(const RgbirAttrV10_t* att);
    XCamReturn getAttribV10(RgbirAttrV10_t* att);
#endif

 protected:
    virtual void init();
    virtual void deInit() { RkAiqHandle::deInit(); };

 private:
#ifndef DISABLE_HANDLE_ATTRIB
#if RKAIQ_HAVE_RGBIR_REMOSAIC_V10
    RgbirAttrV10_t mCurAttV10;
    RgbirAttrV10_t mNewAttV10;
#endif
#endif

 private:
    DECLARE_HANDLE_REGISTER_TYPE(RkAiqArgbirHandleInt);
};

}  // namespace RkCam

#endif
