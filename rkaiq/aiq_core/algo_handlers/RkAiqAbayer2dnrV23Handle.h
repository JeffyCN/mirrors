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
#ifndef _RK_AIQ_BAYER2DNR_V23_HANDLE_INT_H_
#define _RK_AIQ_BAYER2DNR_V23_HANDLE_INT_H_

#include "RkAiqHandle.h"
#include "abayer2dnrV23/rk_aiq_uapi_abayer2dnr_int_v23.h"
#include "rk_aiq_api_private.h"
#include "rk_aiq_pool.h"
#include "xcam_mutex.h"

namespace RkCam {
#if RKAIQ_HAVE_BAYER2DNR_V23

class RkAiqAbayer2dnrV23HandleInt : virtual public RkAiqHandle {
public:
    explicit RkAiqAbayer2dnrV23HandleInt(RkAiqAlgoDesComm* des, RkAiqCore* aiqCore)
        : RkAiqHandle(des, aiqCore) {
#ifndef DISABLE_HANDLE_ATTRIB
        updateStrength = false;
        updateAtt      = false;
        memset(&mCurStrength, 0x00, sizeof(mCurStrength));
        memset(&mNewStrength, 0x00, sizeof(mNewStrength));
        mCurStrength.percent = 1.0;
        mNewStrength.percent = 1.0;
        memset(&mCurAtt, 0x00, sizeof(mCurAtt));
        memset(&mNewAtt, 0x00, sizeof(mNewAtt));
        memset(&mCurInfo, 0x00, sizeof(mCurInfo));
        memset(&mNewInfo, 0x00, sizeof(mNewInfo));
#endif
    };
    virtual ~RkAiqAbayer2dnrV23HandleInt() {
        RkAiqHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    virtual XCamReturn prepare();
    virtual XCamReturn preProcess();
    virtual XCamReturn processing();
    virtual XCamReturn postProcess();
    virtual XCamReturn genIspResult(RkAiqFullParams* params, RkAiqFullParams* cur_params);
    // TODO add algo specific methords, this is a sample
    XCamReturn setAttrib(const rk_aiq_bayer2dnr_attrib_v23_t* att);
    XCamReturn getAttrib(rk_aiq_bayer2dnr_attrib_v23_t* att);
    XCamReturn setStrength(const rk_aiq_bayer2dnr_strength_v23_t* pStrength);
    XCamReturn getStrength(rk_aiq_bayer2dnr_strength_v23_t* pStrength);
    XCamReturn getInfo(rk_aiq_bayer2dnr_info_v23_t *pInfo);
protected:
    virtual void init();
    virtual void deInit() {
        RkAiqHandle::deInit();
    };

private:
    // TODO
#ifndef DISABLE_HANDLE_ATTRIB
    rk_aiq_bayer2dnr_attrib_v23_t mCurAtt;
    rk_aiq_bayer2dnr_attrib_v23_t mNewAtt;
    rk_aiq_bayer2dnr_strength_v23_t mCurStrength;
    rk_aiq_bayer2dnr_strength_v23_t mNewStrength;
    rk_aiq_bayer2dnr_info_v23_t mCurInfo;
    rk_aiq_bayer2dnr_info_v23_t mNewInfo;
    mutable std::atomic<bool> updateStrength;
    mutable std::atomic<bool> updateInfo;
#endif
private:
    DECLARE_HANDLE_REGISTER_TYPE(RkAiqAbayer2dnrV23HandleInt);
};
#endif
}  // namespace RkCam

#endif
