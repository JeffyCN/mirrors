/*
 * RkAiqAldchHandle.h
 *
 *  Copyright (c) 2019-2021 Rockchip Eletronics Co., Ltd.
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
 *
 */

#include "RkAiqCore.h"
#include "RkAiqHandle.h"

namespace RkCam {

void RkAiqAldchHandle::init() {
    ENTER_ANALYZER_FUNCTION();

    deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAldch());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAldch());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAldch());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAldch());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAldch());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAldch());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAldch());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqAldchHandle::prepare() {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret            = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "aldch handle prepare failed");

    // TODO config aldch common params
    RkAiqAlgoConfigAldch* aldch_config = (RkAiqAlgoConfigAldch*)mConfig;

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->prepare(mConfig);
        RKAIQCORE_CHECK_RET(ret, "aldch algo prepare failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAldchHandle::preProcess() {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret               = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des    = (RkAiqAlgoDescription*)mDes;
    RkAiqAlgoPreAldch* aldch_pre = (RkAiqAlgoPreAldch*)mPreInParam;

    ret = RkAiqHandle::preProcess();
    RKAIQCORE_CHECK_RET(ret, "aldch handle preProcess failed");

    // TODO config common aldch preprocess params

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->pre_process(mPreInParam, mPreOutParam);
        RKAIQCORE_CHECK_RET(ret, "aldch handle pre_process failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAldchHandle::processing() {
    XCamReturn ret                = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des     = (RkAiqAlgoDescription*)mDes;
    RkAiqAlgoProcAldch* aldch_pre = (RkAiqAlgoProcAldch*)mProcInParam;

    ret = RkAiqHandle::processing();
    RKAIQCORE_CHECK_RET(ret, "aldch handle processing failed");

    // TODO config common aldch processing params

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->processing(mProcInParam, mProcOutParam);
        RKAIQCORE_CHECK_RET(ret, "aldch algo processing failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAldchHandle::postProcess() {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret                = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des     = (RkAiqAlgoDescription*)mDes;
    RkAiqAlgoPostAldch* aldch_pre = (RkAiqAlgoPostAldch*)mPostInParam;

    ret = RkAiqHandle::postProcess();
    RKAIQCORE_CHECK_RET(ret, "aldch handle postProcess failed");

    // TODO config common aldch postProcess params

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->post_process(mPostInParam, mPostOutParam);
        RKAIQCORE_CHECK_RET(ret, "aldch algo postProcess failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

};  // namespace RkCam
