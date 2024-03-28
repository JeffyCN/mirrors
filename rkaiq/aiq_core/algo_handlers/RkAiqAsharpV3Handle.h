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
#ifndef _RK_AIQ_SHARP_V3_HANDLE_INT_H_
#define _RK_AIQ_SHARP_V3_HANDLE_INT_H_

#include "RkAiqHandle.h"
#include "asharp3/rk_aiq_uapi_asharp_int_v3.h"
#include "rk_aiq_api_private.h"
#include "rk_aiq_pool.h"
#include "xcam_mutex.h"

namespace RkCam {
#if RKAIQ_HAVE_SHARP_V3

class RkAiqAsharpV3HandleInt : virtual public RkAiqHandle {
public:
    explicit RkAiqAsharpV3HandleInt(RkAiqAlgoDesComm* des, RkAiqCore* aiqCore)
        : RkAiqHandle(des, aiqCore) {
#ifndef DISABLE_HANDLE_ATTRIB
        memset(&mCurAtt, 0, sizeof(rk_aiq_sharp_attrib_v3_t));
        memset(&mNewAtt, 0, sizeof(rk_aiq_sharp_attrib_v3_t));
#endif
    };
    virtual ~RkAiqAsharpV3HandleInt() {
        RkAiqHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    virtual XCamReturn prepare();
    virtual XCamReturn preProcess();
    virtual XCamReturn processing();
    virtual XCamReturn postProcess();
    virtual XCamReturn genIspResult(RkAiqFullParams* params, RkAiqFullParams* cur_params);
    // TODO add algo specific methords, this is a sample
    XCamReturn setAttrib(rk_aiq_sharp_attrib_v3_t* att);
    XCamReturn getAttrib(rk_aiq_sharp_attrib_v3_t* att);
    XCamReturn setStrength(float fPercent);
    XCamReturn getStrength(float* pPercent);
    XCamReturn setIQPara(rk_aiq_sharp_IQPara_V3_t* para);
    XCamReturn getIQPara(rk_aiq_sharp_IQPara_V3_t* para);

protected:
    virtual void init();
    virtual void deInit() {
        RkAiqHandle::deInit();
    };

private:
#ifndef DISABLE_HANDLE_ATTRIB
    // TODO
    rk_aiq_sharp_attrib_v3_t mCurAtt;
    rk_aiq_sharp_attrib_v3_t mNewAtt;
    rk_aiq_sharp_IQPara_V3_t mCurIQPara;
    rk_aiq_sharp_IQPara_V3_t mNewIQPara;
    float mCurStrength{0};
    float mNewStrength{0};
    bool updateIQpara   = false;
    bool updateStrength = false;
#endif

private:
    DECLARE_HANDLE_REGISTER_TYPE(RkAiqAsharpV3HandleInt);
};
#endif
}  // namespace RkCam

#endif
