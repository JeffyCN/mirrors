/*
 * RkAiqHandle.h
 *
 *  Copyright (c) 2019 Rockchip Corporation
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

void RkAiqAsharpHandle::init() {
    ENTER_ANALYZER_FUNCTION();

    deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAsharp());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAsharp());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAsharp());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAsharp());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAsharp());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAsharp());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAsharp());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqAsharpHandle::prepare() {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret            = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "asharp handle prepare failed");

    // TODO config asharp common params
    RkAiqAlgoConfigAsharp* asharp_config = (RkAiqAlgoConfigAsharp*)mConfig;

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->prepare(mConfig);
        RKAIQCORE_CHECK_RET(ret, "asharp algo prepare failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAsharpHandle::preProcess() {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret                 = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des      = (RkAiqAlgoDescription*)mDes;
    RkAiqAlgoPreAsharp* asharp_pre = (RkAiqAlgoPreAsharp*)mPreInParam;

    ret = RkAiqHandle::preProcess();
    RKAIQCORE_CHECK_RET(ret, "asharp handle preProcess failed");

    // TODO config common asharp preprocess params

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->pre_process(mPreInParam, mPreOutParam);
        RKAIQCORE_CHECK_RET(ret, "asharp handle pre_process failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAsharpHandle::processing() {
    XCamReturn ret                  = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des       = (RkAiqAlgoDescription*)mDes;
    RkAiqAlgoProcAsharp* asharp_pre = (RkAiqAlgoProcAsharp*)mProcInParam;

    ret = RkAiqHandle::processing();
    RKAIQCORE_CHECK_RET(ret, "asharp handle processing failed");

    // TODO config common asharp processing params

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->processing(mProcInParam, mProcOutParam);
        RKAIQCORE_CHECK_RET(ret, "asharp algo processing failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAsharpHandle::postProcess() {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret                  = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des       = (RkAiqAlgoDescription*)mDes;
    RkAiqAlgoPostAsharp* asharp_pre = (RkAiqAlgoPostAsharp*)mPostInParam;

    ret = RkAiqHandle::postProcess();
    RKAIQCORE_CHECK_RET(ret, "asharp handle postProcess failed");

    // TODO config common asharp postProcess params

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->post_process(mPostInParam, mPostOutParam);
        RKAIQCORE_CHECK_RET(ret, "asharp algo postProcess failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

};  // namespace RkCam
