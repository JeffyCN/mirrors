/*
 * RkAiqAynrHandle.h
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

void RkAiqAynrHandle::init() {
    ENTER_ANALYZER_FUNCTION();

    deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAynr());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAynr());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAynr());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAynr());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAynr());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAynr());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAynr());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqAynrHandle::prepare() {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret            = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "aynr handle prepare failed");

    // TODO config aynr common params
    RkAiqAlgoConfigAynr* aynr_config = (RkAiqAlgoConfigAynr*)mConfig;

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->prepare(mConfig);
        RKAIQCORE_CHECK_RET(ret, "aynr algo prepare failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAynrHandle::preProcess() {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret             = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des  = (RkAiqAlgoDescription*)mDes;
    RkAiqAlgoPreAynr* aynr_pre = (RkAiqAlgoPreAynr*)mPreInParam;

    ret = RkAiqHandle::preProcess();
    RKAIQCORE_CHECK_RET(ret, "aynr handle preProcess failed");

    // TODO config common aynr preprocess params

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->pre_process(mPreInParam, mPreOutParam);
        RKAIQCORE_CHECK_RET(ret, "aynr handle pre_process failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAynrHandle::processing() {
    XCamReturn ret              = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des   = (RkAiqAlgoDescription*)mDes;
    RkAiqAlgoProcAynr* aynr_pre = (RkAiqAlgoProcAynr*)mProcInParam;

    ret = RkAiqHandle::processing();
    RKAIQCORE_CHECK_RET(ret, "aynr handle processing failed");

    // TODO config common aynr processing params

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->processing(mProcInParam, mProcOutParam);
        RKAIQCORE_CHECK_RET(ret, "aynr algo processing failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAynrHandle::postProcess() {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret              = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des   = (RkAiqAlgoDescription*)mDes;
    RkAiqAlgoPostAynr* aynr_pre = (RkAiqAlgoPostAynr*)mPostInParam;

    ret = RkAiqHandle::postProcess();
    RKAIQCORE_CHECK_RET(ret, "aynr handle postProcess failed");

    // TODO config common aynr postProcess params

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->post_process(mPostInParam, mPostOutParam);
        RKAIQCORE_CHECK_RET(ret, "aynr algo postProcess failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

};  // namespace RkCam
