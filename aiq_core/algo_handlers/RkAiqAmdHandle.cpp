/*
 * RkAiqAmdHandle.h
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

void RkAiqAmdHandle::init() {
    ENTER_ANALYZER_FUNCTION();

    deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAmd());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAmd());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAmd());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAmd());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAmd());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAmd());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAmd());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqAmdHandle::prepare() {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret            = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "amd handle prepare failed");

    // TODO config amd common params
    RkAiqAlgoConfigAmd* adebayer_config = (RkAiqAlgoConfigAmd*)mConfig;

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->prepare(mConfig);
        RKAIQCORE_CHECK_RET(ret, "amd algo prepare failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAmdHandle::preProcess() {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret            = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    RkAiqAlgoPreAmd* amd_pre  = (RkAiqAlgoPreAmd*)mPreInParam;

    ret = RkAiqHandle::preProcess();
    RKAIQCORE_CHECK_RET(ret, "amd handle preProcess failed");

    // TODO config common amd preprocess params

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->pre_process(mPreInParam, mPreOutParam);
        RKAIQCORE_CHECK_RET(ret, "amd handle pre_process failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAmdHandle::processing() {
    XCamReturn ret            = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    RkAiqAlgoProcAmd* amd_pre = (RkAiqAlgoProcAmd*)mProcInParam;

    ret = RkAiqHandle::processing();
    RKAIQCORE_CHECK_RET(ret, "amd handle processing failed");

    // TODO config common amd processing params

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->processing(mProcInParam, mProcOutParam);
        RKAIQCORE_CHECK_RET(ret, "amd algo processing failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAmdHandle::postProcess() {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret            = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    RkAiqAlgoPostAmd* amd_pre = (RkAiqAlgoPostAmd*)mPostInParam;

    ret = RkAiqHandle::postProcess();
    RKAIQCORE_CHECK_RET(ret, "amd handle postProcess failed");

    // TODO config common amd postProcess params

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->post_process(mPostInParam, mPostOutParam);
        RKAIQCORE_CHECK_RET(ret, "amd algo postProcess failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

};  // namespace RkCam
