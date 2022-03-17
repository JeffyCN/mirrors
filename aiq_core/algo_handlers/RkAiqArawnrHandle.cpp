/*
 * RkAiqArawnrHandle.h
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

void RkAiqArawnrHandle::init() {
    ENTER_ANALYZER_FUNCTION();

    deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigArawnr());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreArawnr());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResArawnr());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcArawnr());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResArawnr());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostArawnr());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResArawnr());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqArawnrHandle::prepare() {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret            = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "arawnr handle prepare failed");

    // TODO config arawnr common params
    RkAiqAlgoConfigArawnr* arawnr_config = (RkAiqAlgoConfigArawnr*)mConfig;

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->prepare(mConfig);
        RKAIQCORE_CHECK_RET(ret, "arawnr algo prepare failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqArawnrHandle::preProcess() {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret                 = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des      = (RkAiqAlgoDescription*)mDes;
    RkAiqAlgoPreArawnr* arawnr_pre = (RkAiqAlgoPreArawnr*)mPreInParam;

    ret = RkAiqHandle::preProcess();
    RKAIQCORE_CHECK_RET(ret, "arawnr handle preProcess failed");

    // TODO config common arawnr preprocess params

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->pre_process(mPreInParam, mPreOutParam);
        RKAIQCORE_CHECK_RET(ret, "arawnr handle pre_process failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqArawnrHandle::processing() {
    XCamReturn ret                  = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des       = (RkAiqAlgoDescription*)mDes;
    RkAiqAlgoProcArawnr* arawnr_pre = (RkAiqAlgoProcArawnr*)mProcInParam;

    ret = RkAiqHandle::processing();
    RKAIQCORE_CHECK_RET(ret, "arawnr handle processing failed");

    // TODO config common arawnr processing params

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->processing(mProcInParam, mProcOutParam);
        RKAIQCORE_CHECK_RET(ret, "arawnr algo processing failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqArawnrHandle::postProcess() {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret                  = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des       = (RkAiqAlgoDescription*)mDes;
    RkAiqAlgoPostArawnr* arawnr_pre = (RkAiqAlgoPostArawnr*)mPostInParam;

    ret = RkAiqHandle::postProcess();
    RKAIQCORE_CHECK_RET(ret, "arawnr handle postProcess failed");

    // TODO config common arawnr postProcess params

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->post_process(mPostInParam, mPostOutParam);
        RKAIQCORE_CHECK_RET(ret, "arawnr algo postProcess failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

};  // namespace RkCam
