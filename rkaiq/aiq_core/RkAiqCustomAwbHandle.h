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
#ifndef _RK_AIQ_CUSTOM_AWB_HANDLE_H_
#define _RK_AIQ_CUSTOM_AWB_HANDLE_H_

#include "algo_handlers/RkAiqAwbHandle.h"

#if RKAIQ_HAVE_AWB
namespace RkCam {

// awb
class RkAiqCustomAwbHandle:
    public RkAiqAwbHandleInt {
public:
    explicit RkAiqCustomAwbHandle(RkAiqAlgoDesComm* des, RkAiqCore* aiqCore)
        : RkAiqAwbHandleInt (des, aiqCore) {};
    virtual ~RkAiqCustomAwbHandle() {
        deInit();
    };
    virtual XCamReturn updateConfig(bool needSync);
    virtual XCamReturn processing();
    virtual XCamReturn genIspResult(RkAiqFullParams* params, RkAiqFullParams* cur_params);
    virtual void init();
    virtual void deInit();
};


} //namespace RkCam
#endif
#endif
