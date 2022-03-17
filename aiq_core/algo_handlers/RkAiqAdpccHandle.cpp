/*
 * RkAiqAdpccHandle.h
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

void RkAiqAdpccHandle::init() {
    ENTER_ANALYZER_FUNCTION();

    deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAdpcc());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAdpcc());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAdpcc());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAdpcc());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAdpcc());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAdpcc());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAdpcc());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqAdpccHandle::prepare() {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret            = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "adpcc handle prepare failed");

    // TODO config adpcc common params
    RkAiqAlgoConfigAdpcc* adpcc_config = (RkAiqAlgoConfigAdpcc*)mConfig;

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->prepare(mConfig);
        RKAIQCORE_CHECK_RET(ret, "adpcc algo prepare failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAdpccHandle::preProcess() {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret               = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des    = (RkAiqAlgoDescription*)mDes;
    RkAiqAlgoPreAdpcc* adpcc_pre = (RkAiqAlgoPreAdpcc*)mPreInParam;

    ret = RkAiqHandle::preProcess();
    RKAIQCORE_CHECK_RET(ret, "adpcc handle preProcess failed");

    // TODO config common adpcc preprocess params

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->pre_process(mPreInParam, mPreOutParam);
        RKAIQCORE_CHECK_RET(ret, "adpcc handle pre_process failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAdpccHandle::processing() {
    XCamReturn ret                = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des     = (RkAiqAlgoDescription*)mDes;
    RkAiqAlgoProcAdpcc* adpcc_pre = (RkAiqAlgoProcAdpcc*)mProcInParam;

    ret = RkAiqHandle::processing();
    RKAIQCORE_CHECK_RET(ret, "adpcc handle processing failed");

    // TODO config common adpcc processing params

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->processing(mProcInParam, mProcOutParam);
        RKAIQCORE_CHECK_RET(ret, "adpcc algo processing failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAdpccHandle::postProcess() {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret                = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des     = (RkAiqAlgoDescription*)mDes;
    RkAiqAlgoPostAdpcc* adpcc_pre = (RkAiqAlgoPostAdpcc*)mPostInParam;

    ret = RkAiqHandle::postProcess();
    RKAIQCORE_CHECK_RET(ret, "adpcc handle postProcess failed");

    // TODO config common adpcc postProcess params

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->post_process(mPostInParam, mPostOutParam);
        RKAIQCORE_CHECK_RET(ret, "adpcc algo postProcess failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

};  // namespace RkCam
