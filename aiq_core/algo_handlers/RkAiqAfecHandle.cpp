/*
 * RkAiqAdfecHandle.h
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

void RkAiqAfecHandle::init() {
    ENTER_ANALYZER_FUNCTION();

    deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAfec());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAfec());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAfec());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAfec());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAfec());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAfec());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAfec());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqAfecHandle::prepare() {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret            = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "afec handle prepare failed");

    // TODO config afec common params
    RkAiqAlgoConfigAfec* afec_config = (RkAiqAlgoConfigAfec*)mConfig;

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->prepare(mConfig);
        RKAIQCORE_CHECK_RET(ret, "afec algo prepare failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAfecHandle::preProcess() {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret             = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des  = (RkAiqAlgoDescription*)mDes;
    RkAiqAlgoPreAfec* afec_pre = (RkAiqAlgoPreAfec*)mPreInParam;

    ret = RkAiqHandle::preProcess();
    RKAIQCORE_CHECK_RET(ret, "afec handle preProcess failed");

    // TODO config common afec preprocess params

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->pre_process(mPreInParam, mPreOutParam);
        RKAIQCORE_CHECK_RET(ret, "afec handle pre_process failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAfecHandle::processing() {
    XCamReturn ret              = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des   = (RkAiqAlgoDescription*)mDes;
    RkAiqAlgoProcAfec* afec_pre = (RkAiqAlgoProcAfec*)mProcInParam;

    ret = RkAiqHandle::processing();
    RKAIQCORE_CHECK_RET(ret, "afec handle processing failed");

    // TODO config common afec processing params

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->processing(mProcInParam, mProcOutParam);
        RKAIQCORE_CHECK_RET(ret, "afec algo processing failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAfecHandle::postProcess() {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret              = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des   = (RkAiqAlgoDescription*)mDes;
    RkAiqAlgoPostAfec* afec_pre = (RkAiqAlgoPostAfec*)mPostInParam;

    ret = RkAiqHandle::postProcess();
    RKAIQCORE_CHECK_RET(ret, "afec handle postProcess failed");

    // TODO config common afec postProcess params

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->post_process(mPostInParam, mPostOutParam);
        RKAIQCORE_CHECK_RET(ret, "afec algo postProcess failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

};  // namespace RkCam
