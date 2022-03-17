/*
 * RkAiqAgammaHandle.h
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

void RkAiqAgammaHandle::init() {
    ENTER_ANALYZER_FUNCTION();

    deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAgamma());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAgamma());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAgamma());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAgamma());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAgamma());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAgamma());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAgamma());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqAgammaHandle::prepare() {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret            = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "agamma handle prepare failed");

    // TODO config agamma common params
    RkAiqAlgoConfigAgamma* agamma_config = (RkAiqAlgoConfigAgamma*)mConfig;

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->prepare(mConfig);
        RKAIQCORE_CHECK_RET(ret, "agamma algo prepare failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAgammaHandle::preProcess() {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret                 = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des      = (RkAiqAlgoDescription*)mDes;
    RkAiqAlgoPreAgamma* agamma_pre = (RkAiqAlgoPreAgamma*)mPreInParam;

    ret = RkAiqHandle::preProcess();
    RKAIQCORE_CHECK_RET(ret, "agamma handle preProcess failed");

    // TODO config common agamma preprocess params

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->pre_process(mPreInParam, mPreOutParam);
        RKAIQCORE_CHECK_RET(ret, "agamma handle pre_process failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAgammaHandle::processing() {
    XCamReturn ret                  = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des       = (RkAiqAlgoDescription*)mDes;
    RkAiqAlgoProcAgamma* agamma_pre = (RkAiqAlgoProcAgamma*)mProcInParam;

    ret = RkAiqHandle::processing();
    RKAIQCORE_CHECK_RET(ret, "agamma handle processing failed");

    // TODO config common agamma processing params

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->processing(mProcInParam, mProcOutParam);
        RKAIQCORE_CHECK_RET(ret, "agamma algo processing failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAgammaHandle::postProcess() {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret                  = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des       = (RkAiqAlgoDescription*)mDes;
    RkAiqAlgoPostAgamma* agamma_pre = (RkAiqAlgoPostAgamma*)mPostInParam;

    ret = RkAiqHandle::postProcess();
    RKAIQCORE_CHECK_RET(ret, "agamma handle postProcess failed");

    // TODO config common agamma postProcess params

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->post_process(mPostInParam, mPostOutParam);
        RKAIQCORE_CHECK_RET(ret, "agamma algo postProcess failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

};  // namespace RkCam
