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
#ifndef _RK_AIQ_AFD_HANDLE_INT_H_
#define _RK_AIQ_AFD_HANDLE_INT_H_

#include "RkAiqHandle.h"
#include "rk_aiq_api_private.h"
#include "afd/rk_aiq_uapi_afd_int.h"
#include "rk_aiq_pool.h"
#include "xcam_mutex.h"

namespace RkCam {

class RkAiqAfdHandleInt : virtual public RkAiqHandle {
public:
    explicit RkAiqAfdHandleInt(RkAiqAlgoDesComm* des, RkAiqCore* aiqCore)
        : RkAiqHandle(des, aiqCore) {}
    virtual ~RkAiqAfdHandleInt() {
        RkAiqHandle::deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    virtual XCamReturn getScaleRatio(int* scale_ratio);
    virtual XCamReturn getAfdEn(bool* en);
    virtual XCamReturn prepare();
    virtual XCamReturn preProcess();
    virtual XCamReturn processing();
    virtual XCamReturn postProcess();
    virtual XCamReturn genIspResult(RkAiqFullParams* params, RkAiqFullParams* cur_params);
    // TODO add algo specific methords, this is a sample
    void setAeProcRes(RkAiqAlgoProcResAeShared_t* aeProcRes) {
        mAeProcRes = *aeProcRes;
    }

protected:
    virtual void init();
    virtual void deInit() {
        RkAiqHandle::deInit();
    };
    bool mAfdSyncAly{true};
    RkAiqAlgoProcResAeShared_t mAeProcRes;
private:

private:
    DECLARE_HANDLE_REGISTER_TYPE(RkAiqAfdHandleInt);
};

}  // namespace RkCam

#endif
