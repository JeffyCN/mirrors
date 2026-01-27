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
#ifndef _RK_AIQ_LDCH_HANDLE_INT_H_
#define _RK_AIQ_LDCH_HANDLE_INT_H_

#include "RkAiqHandle.h"
#if (RKAIQ_HAVE_LDCH_V21)
#include "aldch/rk_aiq_uapi_aldch_v21_int.h"
#else
#include "aldch/rk_aiq_uapi_aldch_int.h"
#endif
#include "rk_aiq_api_private.h"
#include "rk_aiq_pool.h"
#include "xcam_mutex.h"

namespace RkCam {

#if (RKAIQ_HAVE_LDCH_V10 || RKAIQ_HAVE_LDCH_V21)
class RkAiqAldchHandleInt : virtual public RkAiqHandle {
 public:
    explicit RkAiqAldchHandleInt(RkAiqAlgoDesComm* des, RkAiqCore* aiqCore)
        : RkAiqHandle(des, aiqCore) {
#ifndef DISABLE_HANDLE_ATTRIB
#if (RKAIQ_HAVE_LDCH_V21)
        memset(&mCurAtt, 0, sizeof(rk_aiq_ldch_v21_attrib_t));
        memset(&mNewAtt, 0, sizeof(rk_aiq_ldch_v21_attrib_t));
#else
        memset(&mCurAtt, 0, sizeof(rk_aiq_ldch_attrib_t));
        memset(&mNewAtt, 0, sizeof(rk_aiq_ldch_attrib_t));
#endif
#endif
    };
    virtual ~RkAiqAldchHandleInt() { RkAiqHandle::deInit(); };
    virtual XCamReturn updateConfig(bool needSync);
    virtual XCamReturn prepare();
    virtual XCamReturn preProcess();
    virtual XCamReturn processing();
    virtual XCamReturn postProcess();
    virtual XCamReturn genIspResult(RkAiqFullParams* params, RkAiqFullParams* cur_params);

#if (RKAIQ_HAVE_LDCH_V21)
    XCamReturn setAttrib(const rk_aiq_ldch_v21_attrib_t* att);
    XCamReturn getAttrib(rk_aiq_ldch_v21_attrib_t* att);
    XCamReturn copyLutFromExtBuffer(const rk_aiq_ldch_v21_attrib_t* att);
#else
    XCamReturn setAttrib(const rk_aiq_ldch_attrib_t* att);
    XCamReturn getAttrib(rk_aiq_ldch_attrib_t* att);
#endif

 protected:
    virtual void init();
    virtual void deInit() { RkAiqHandle::deInit(); };

 private:
#ifndef DISABLE_HANDLE_ATTRIB
#if (RKAIQ_HAVE_LDCH_V21)
    rk_aiq_ldch_v21_attrib_t mCurAtt;
    rk_aiq_ldch_v21_attrib_t mNewAtt;
#else
    rk_aiq_ldch_attrib_t mCurAtt;
    rk_aiq_ldch_attrib_t mNewAtt;
#endif
#endif

 private:
    DECLARE_HANDLE_REGISTER_TYPE(RkAiqAldchHandleInt);
};
#endif

}  // namespace RkCam

#endif
