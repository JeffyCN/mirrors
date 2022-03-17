/*
 * RkAiqAgicHandle.h
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

void RkAiqAgicHandle::init() {
    ENTER_ANALYZER_FUNCTION();

    deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAgic());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAgic());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAgic());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAgic());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAgic());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAgic());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAgic());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqAgicHandle::prepare() {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret            = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "agic handle prepare failed");

    // TODO config agic common params
    RkAiqAlgoConfigAgic* agic_config = (RkAiqAlgoConfigAgic*)mConfig;

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->prepare(mConfig);
        RKAIQCORE_CHECK_RET(ret, "agic algo prepare failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAgicHandle::preProcess() {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret             = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des  = (RkAiqAlgoDescription*)mDes;
    RkAiqAlgoPreAgic* agic_pre = (RkAiqAlgoPreAgic*)mPreInParam;

    ret = RkAiqHandle::preProcess();
    RKAIQCORE_CHECK_RET(ret, "agic handle preProcess failed");

    // TODO config common agic preprocess params

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->pre_process(mPreInParam, mPreOutParam);
        RKAIQCORE_CHECK_RET(ret, "agic handle pre_process failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAgicHandle::processing() {
    XCamReturn ret              = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des   = (RkAiqAlgoDescription*)mDes;
    RkAiqAlgoProcAgic* agic_pre = (RkAiqAlgoProcAgic*)mProcInParam;

    ret = RkAiqHandle::processing();
    RKAIQCORE_CHECK_RET(ret, "agic handle processing failed");

    // TODO config common agic processing params

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->processing(mProcInParam, mProcOutParam);
        RKAIQCORE_CHECK_RET(ret, "agic algo processing failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAgicHandle::postProcess() {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret              = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des   = (RkAiqAlgoDescription*)mDes;
    RkAiqAlgoPostAgic* agic_pre = (RkAiqAlgoPostAgic*)mPostInParam;

    ret = RkAiqHandle::postProcess();
    RKAIQCORE_CHECK_RET(ret, "agic handle postProcess failed");

    // TODO config common agic postProcess params

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->post_process(mPostInParam, mPostOutParam);
        RKAIQCORE_CHECK_RET(ret, "agic algo postProcess failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

};  // namespace RkCam
