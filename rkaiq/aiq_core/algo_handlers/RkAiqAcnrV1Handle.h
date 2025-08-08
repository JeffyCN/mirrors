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
#ifndef _RK_AIQ_CNR_V1_HANDLE_INT_H_
#define _RK_AIQ_CNR_V1_HANDLE_INT_H_

#include "RkAiqHandle.h"
#include "acnr/rk_aiq_uapi_acnr_int_v1.h"
#include "rk_aiq_api_private.h"
#include "rk_aiq_pool.h"
#include "xcam_mutex.h"

namespace RkCam {
#if RKAIQ_HAVE_CNR_V1

class RkAiqAcnrV1HandleInt : virtual public RkAiqHandle {
public:
    explicit RkAiqAcnrV1HandleInt(RkAiqAlgoDesComm* des, RkAiqCore* aiqCore)
        : RkAiqHandle(des, aiqCore) {
#ifndef DISABLE_HANDLE_ATTRIB
        memset(&mCurAtt, 0, sizeof(rk_aiq_cnr_attrib_v1_t));
        memset(&mNewAtt, 0, sizeof(rk_aiq_cnr_attrib_v1_t));
#endif
    };
    virtual ~RkAiqAcnrV1HandleInt() {
        RkAiqHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    virtual XCamReturn prepare();
    virtual XCamReturn preProcess();
    virtual XCamReturn processing();
    virtual XCamReturn postProcess();
    virtual XCamReturn genIspResult(RkAiqFullParams* params, RkAiqFullParams* cur_params);
    // TODO add algo specific methords, this is a sample
    XCamReturn setAttrib(rk_aiq_cnr_attrib_v1_t* att);
    XCamReturn getAttrib(rk_aiq_cnr_attrib_v1_t* att);
    XCamReturn setStrength(float fPercent);
    XCamReturn getStrength(float* pPercent);
    XCamReturn setIQPara(rk_aiq_cnr_IQPara_V1_t* pPara);
    XCamReturn getIQPara(rk_aiq_cnr_IQPara_V1_t* pPara);

protected:
    virtual void init();
    virtual void deInit() {
        RkAiqHandle::deInit();
    };

private:
#ifndef DISABLE_HANDLE_ATTRIB
    // TODO
    rk_aiq_cnr_attrib_v1_t mCurAtt;
    rk_aiq_cnr_attrib_v1_t mNewAtt;
    rk_aiq_cnr_IQPara_V1_t mCurIQPara;
    rk_aiq_cnr_IQPara_V1_t mNewIQPara;
    float mCurStrength{0};
    float mNewStrength{0};
    bool updateIQpara   = false;
    bool updateStrength = false;
#endif

private:
    DECLARE_HANDLE_REGISTER_TYPE(RkAiqAcnrV1HandleInt);
};
#endif
}  // namespace RkCam

#endif
