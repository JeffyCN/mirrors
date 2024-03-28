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
#ifndef _RK_AIQ_BAYERTNR_V23_HANDLE_INT_H_
#define _RK_AIQ_BAYERTNR_V23_HANDLE_INT_H_

#include "RkAiqHandle.h"
#include "abayertnrV23/rk_aiq_uapi_abayertnr_int_v23.h"
#include "rk_aiq_api_private.h"
#include "rk_aiq_pool.h"
#include "xcam_mutex.h"

namespace RkCam {
#if (RKAIQ_HAVE_BAYERTNR_V23 || RKAIQ_HAVE_BAYERTNR_V23_LITE)

class RkAiqAbayertnrV23HandleInt : virtual public RkAiqHandle {
public:
    explicit RkAiqAbayertnrV23HandleInt(RkAiqAlgoDesComm* des, RkAiqCore* aiqCore)
        : RkAiqHandle(des, aiqCore) {
#ifndef DISABLE_HANDLE_ATTRIB
        updateStrength = false;
        updateAtt      = false;
        updateAttLite  = false;
        memset(&mCurStrength, 0x00, sizeof(mCurStrength));
        memset(&mNewStrength, 0x00, sizeof(mNewStrength));
        mCurStrength.percent = 1.0;
        mNewStrength.percent = 1.0;
        memset(&mCurAtt, 0x00, sizeof(mCurAtt));
        memset(&mNewAtt, 0x00, sizeof(mNewAtt));
        memset(&mCurAttLite, 0x00, sizeof(mCurAttLite));
        memset(&mNewAttLite, 0x00, sizeof(mNewAttLite));
#endif
    };
    virtual ~RkAiqAbayertnrV23HandleInt() {
        RkAiqHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    virtual XCamReturn prepare();
    virtual XCamReturn preProcess();
    virtual XCamReturn processing();
    virtual XCamReturn postProcess();
    virtual XCamReturn genIspResult(RkAiqFullParams* params, RkAiqFullParams* cur_params);
    // TODO add algo specific methords, this is a sample
    XCamReturn setAttrib(const rk_aiq_bayertnr_attrib_v23_t* att);
    XCamReturn getAttrib(rk_aiq_bayertnr_attrib_v23_t* att);
    XCamReturn setAttribLite(const rk_aiq_bayertnr_attrib_v23L_t* att);
    XCamReturn getAttribLite(rk_aiq_bayertnr_attrib_v23L_t* att);
    XCamReturn setStrength(const rk_aiq_bayertnr_strength_v23_t* pStrength);
    XCamReturn getStrength(rk_aiq_bayertnr_strength_v23_t* pStrength);
    XCamReturn getInfo(rk_aiq_bayertnr_info_v23_t* pInfo);
protected:
    virtual void init();
    virtual void deInit() {
        RkAiqHandle::deInit();
    };

private:
    // TODO
#ifndef DISABLE_HANDLE_ATTRIB
    rk_aiq_bayertnr_attrib_v23_t mCurAtt;
    rk_aiq_bayertnr_attrib_v23_t mNewAtt;
    rk_aiq_bayertnr_attrib_v23L_t mCurAttLite;
    rk_aiq_bayertnr_attrib_v23L_t mNewAttLite;
    mutable std::atomic<bool> updateAttLite;
    rk_aiq_bayertnr_strength_v23_t mCurStrength;
    rk_aiq_bayertnr_strength_v23_t mNewStrength;
    mutable std::atomic<bool> updateStrength;
#endif
    bool mLatestEn{true};
private:
    DECLARE_HANDLE_REGISTER_TYPE(RkAiqAbayertnrV23HandleInt);
};
#endif
}  // namespace RkCam

#endif
