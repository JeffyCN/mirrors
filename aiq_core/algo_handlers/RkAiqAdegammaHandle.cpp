/*
 * RkAiqAdegammaHandle.h
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

void RkAiqAdegammaHandle::init() {
    ENTER_ANALYZER_FUNCTION();

    deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAdegamma());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAdegamma());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAdegamma());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAdegamma());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAdegamma());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAdegamma());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAdegamma());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqAdegammaHandle::prepare() {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret            = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "adegamma handle prepare failed");

    // TODO config agamma common params
    RkAiqAlgoConfigAdegamma* adegamma_config = (RkAiqAlgoConfigAdegamma*)mConfig;

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->prepare(mConfig);
        RKAIQCORE_CHECK_RET(ret, "adegamma algo prepare failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAdegammaHandle::preProcess() {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret                   = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des        = (RkAiqAlgoDescription*)mDes;
    RkAiqAlgoPreAdegamma* agamma_pre = (RkAiqAlgoPreAdegamma*)mPreInParam;

    ret = RkAiqHandle::preProcess();
    RKAIQCORE_CHECK_RET(ret, "adegamma handle preProcess failed");

    // TODO config common agamma preprocess params

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->pre_process(mPreInParam, mPreOutParam);
        RKAIQCORE_CHECK_RET(ret, "adegamma handle pre_process failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAdegammaHandle::processing() {
    XCamReturn ret                      = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des           = (RkAiqAlgoDescription*)mDes;
    RkAiqAlgoProcAdegamma* adegamma_pre = (RkAiqAlgoProcAdegamma*)mProcInParam;

    ret = RkAiqHandle::processing();
    RKAIQCORE_CHECK_RET(ret, "adegamma handle processing failed");

    // TODO config common agamma processing params

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->processing(mProcInParam, mProcOutParam);
        RKAIQCORE_CHECK_RET(ret, "adegamma algo processing failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAdegammaHandle::postProcess() {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret                    = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des         = (RkAiqAlgoDescription*)mDes;
    RkAiqAlgoPostAdegamma* agamma_pre = (RkAiqAlgoPostAdegamma*)mPostInParam;

    ret = RkAiqHandle::postProcess();
    RKAIQCORE_CHECK_RET(ret, "adegamma handle postProcess failed");

    // TODO config common agamma postProcess params

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->post_process(mPostInParam, mPostOutParam);
        RKAIQCORE_CHECK_RET(ret, "adegamma algo postProcess failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

};  // namespace RkCam
