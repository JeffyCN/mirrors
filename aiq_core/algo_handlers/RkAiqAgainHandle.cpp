/*
 * RkAiqAgainHandle.h
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

void RkAiqAgainHandle::init() {
    ENTER_ANALYZER_FUNCTION();

    deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAgain());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAgain());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAgain());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAgain());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAgain());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAgain());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAgain());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqAgainHandle::prepare() {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret            = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "acnr handle prepare failed");

    // TODO config acnr common params
    RkAiqAlgoConfigAgain* acnr_config = (RkAiqAlgoConfigAgain*)mConfig;

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->prepare(mConfig);
        RKAIQCORE_CHECK_RET(ret, "acnr algo prepare failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAgainHandle::preProcess() {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret              = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des   = (RkAiqAlgoDescription*)mDes;
    RkAiqAlgoPreAgain* acnr_pre = (RkAiqAlgoPreAgain*)mPreInParam;

    ret = RkAiqHandle::preProcess();
    RKAIQCORE_CHECK_RET(ret, "acnr handle preProcess failed");

    // TODO config common acnr preprocess params

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->pre_process(mPreInParam, mPreOutParam);
        RKAIQCORE_CHECK_RET(ret, "acnr handle pre_process failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAgainHandle::processing() {
    XCamReturn ret               = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des    = (RkAiqAlgoDescription*)mDes;
    RkAiqAlgoProcAgain* acnr_pre = (RkAiqAlgoProcAgain*)mProcInParam;

    ret = RkAiqHandle::processing();
    RKAIQCORE_CHECK_RET(ret, "acnr handle processing failed");

    // TODO config common acnr processing params

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->processing(mProcInParam, mProcOutParam);
        RKAIQCORE_CHECK_RET(ret, "acnr algo processing failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAgainHandle::postProcess() {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret               = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des    = (RkAiqAlgoDescription*)mDes;
    RkAiqAlgoPostAgain* acnr_pre = (RkAiqAlgoPostAgain*)mPostInParam;

    ret = RkAiqHandle::postProcess();
    RKAIQCORE_CHECK_RET(ret, "acnr handle postProcess failed");

    // TODO config common acnr postProcess params

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->post_process(mPostInParam, mPostOutParam);
        RKAIQCORE_CHECK_RET(ret, "acnr algo postProcess failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

};  // namespace RkCam
