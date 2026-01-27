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
#ifndef _RK_AIQ_MFNR_HANDLE_INT_H_
#define _RK_AIQ_MFNR_HANDLE_INT_H_

#include "RkAiqHandle.h"
#include "amfnr/rk_aiq_uapi_amfnr_int_v1.h"
#include "rk_aiq_api_private.h"
#include "rk_aiq_pool.h"
#include "xcam_mutex.h"

namespace RkCam {
#if RKAIQ_HAVE_MFNR_V1

class RkAiqAmfnrHandleInt : virtual public RkAiqHandle {
public:
    explicit RkAiqAmfnrHandleInt(RkAiqAlgoDesComm* des, RkAiqCore* aiqCore)
        : RkAiqHandle(des, aiqCore) {
#ifndef DISABLE_HANDLE_ATTRIB
        memset(&mCurAtt, 0, sizeof(rk_aiq_mfnr_attrib_v1_t));
        memset(&mNewAtt, 0, sizeof(rk_aiq_mfnr_attrib_v1_t));
        memset(&mCurIQPara, 0, sizeof(rk_aiq_mfnr_IQPara_V1_t));
        memset(&mNewIQPara, 0, sizeof(rk_aiq_mfnr_IQPara_V1_t));
        memset(&mCurJsonPara, 0, sizeof(rk_aiq_mfnr_JsonPara_V1_t));
        memset(&mNewJsonPara, 0, sizeof(rk_aiq_mfnr_JsonPara_V1_t));
#endif
    };
    virtual ~RkAiqAmfnrHandleInt() {
        RkAiqHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    virtual XCamReturn prepare();
    virtual XCamReturn preProcess();
    virtual XCamReturn processing();
    virtual XCamReturn postProcess();
    virtual XCamReturn genIspResult(RkAiqFullParams* params, RkAiqFullParams* cur_params);
    XCamReturn setAttrib(rk_aiq_mfnr_attrib_v1_t* att);
    XCamReturn getAttrib(rk_aiq_mfnr_attrib_v1_t* att);
    XCamReturn setLumaStrength(float fPercent);
    XCamReturn getLumaStrength(float* pPercent);
    XCamReturn setChromaStrength(float fPercent);
    XCamReturn getChromaStrength(float* pPercent);
    XCamReturn setIQPara(rk_aiq_mfnr_IQPara_V1_t* pPara);
    XCamReturn getIQPara(rk_aiq_mfnr_IQPara_V1_t* pPara);
    XCamReturn setJsonPara(rk_aiq_mfnr_JsonPara_V1_t* para);
    XCamReturn getJsonPara(rk_aiq_mfnr_JsonPara_V1_t* para);

protected:
    virtual void init();
    virtual void deInit() {
        RkAiqHandle::deInit();
    };

private:
#ifndef DISABLE_HANDLE_ATTRIB
    rk_aiq_mfnr_attrib_v1_t mCurAtt;
    rk_aiq_mfnr_attrib_v1_t mNewAtt;
    rk_aiq_mfnr_IQPara_V1_t mCurIQPara;
    rk_aiq_mfnr_IQPara_V1_t mNewIQPara;
    rk_aiq_mfnr_JsonPara_V1_t mCurJsonPara;
    rk_aiq_mfnr_JsonPara_V1_t mNewJsonPara;
    bool updateIQpara   = false;
    bool updateJsonpara = false;
#endif

private:
    DECLARE_HANDLE_REGISTER_TYPE(RkAiqAmfnrHandleInt);
};
#endif
}  // namespace RkCam

#endif
