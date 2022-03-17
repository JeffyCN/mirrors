/*
 * RkAiqAdebayerHandle.h
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

void RkAiqAdebayerHandle::init() {
    ENTER_ANALYZER_FUNCTION();

    deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAdebayer());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAdebayer());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAdebayer());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAdebayer());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAdebayer());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAdebayer());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAdebayer());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqAdebayerHandle::prepare() {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret            = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "adebayer handle prepare failed");

    // TODO config adebayer common params
    RkAiqAlgoConfigAdebayer* adebayer_config = (RkAiqAlgoConfigAdebayer*)mConfig;

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->prepare(mConfig);
        RKAIQCORE_CHECK_RET(ret, "adebayer algo prepare failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAdebayerHandle::preProcess() {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret                     = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des          = (RkAiqAlgoDescription*)mDes;
    RkAiqAlgoPreAdebayer* adebayer_pre = (RkAiqAlgoPreAdebayer*)mPreInParam;

    ret = RkAiqHandle::preProcess();
    RKAIQCORE_CHECK_RET(ret, "adebayer handle preProcess failed");

    // TODO config common adebayer preprocess params

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->pre_process(mPreInParam, mPreOutParam);
        RKAIQCORE_CHECK_RET(ret, "adebayer handle pre_process failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAdebayerHandle::processing() {
    XCamReturn ret                      = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des           = (RkAiqAlgoDescription*)mDes;
    RkAiqAlgoProcAdebayer* adebayer_pre = (RkAiqAlgoProcAdebayer*)mProcInParam;

    ret = RkAiqHandle::processing();
    RKAIQCORE_CHECK_RET(ret, "adebayer handle processing failed");

    // TODO config common adebayer processing params

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->processing(mProcInParam, mProcOutParam);
        RKAIQCORE_CHECK_RET(ret, "adebayer algo processing failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAdebayerHandle::postProcess() {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret                      = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des           = (RkAiqAlgoDescription*)mDes;
    RkAiqAlgoPostAdebayer* adebayer_pre = (RkAiqAlgoPostAdebayer*)mPostInParam;

    ret = RkAiqHandle::postProcess();
    RKAIQCORE_CHECK_RET(ret, "adebayer handle postProcess failed");

    // TODO config common adebayer postProcess params

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->post_process(mPostInParam, mPostOutParam);
        RKAIQCORE_CHECK_RET(ret, "adebayer algo postProcess failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

};  // namespace RkCam
