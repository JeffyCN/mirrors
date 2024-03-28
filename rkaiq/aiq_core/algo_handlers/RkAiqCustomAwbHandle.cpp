/*
 * Copyright (c) 2019-2021 Rockchip Eletronics Co., Ltd.
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
 */

#include "RkAiqCustomAwbHandle.h"

namespace RkCam {

//#define BYPASS_CUSTOM_AWB

void
RkAiqCustomAwbHandle::init() {
    if (mIsMulRun) {
        // reuse parent's resources, contains:
        // mConfig, mProcInParam, mProcOutParam
        RkAiqAwbHandleInt* parent =  dynamic_cast<RkAiqAwbHandleInt*>(mParentHdl);
        if (!parent)
            LOGE_AWB("no parent awb handler in multiple handler mode !");
        mConfig      = parent->mConfig;
        mProcInParam = parent->mProcInParam;
        mProcOutParam = parent->mProcOutParam;
    } else {
        RkAiqAwbHandleInt::init();
    }
}

void
RkAiqCustomAwbHandle::deInit() {
    if (mIsMulRun) {
        mConfig      = NULL;
        mProcInParam = NULL;
        mProcOutParam = NULL;
    } else {
        RkAiqAwbHandleInt::deInit();
    }
}

XCamReturn RkAiqCustomAwbHandle::updateConfig(bool needSync) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCustomAwbHandle::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#ifdef BYPASS_CUSTOM_AWB
    if (0) {
#else
    if (mIsMulRun) {
#endif
        // use parent's mem, so child algo results can overwrite parents results
        RkAiqAwbHandleInt* parent =  dynamic_cast<RkAiqAwbHandleInt*>(mParentHdl);
        if (!parent)
            LOGE_AWB("no parent awb handler in multiple handler mode !");
        mProcResShared =  parent->mProcResShared ;
    } else {
        mProcResShared = new RkAiqAlgoProcResAwbIntShared();
        if (!mProcResShared.ptr()) {
            LOGE_AWB("new awb mProcOutParam failed, bypass!");
            return XCAM_RETURN_BYPASS;
        }
    }

    return RkAiqAwbHandleInt::processing();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCustomAwbHandle::genIspResult(RkAiqFullParams* params, RkAiqFullParams* cur_params) {
#ifdef BYPASS_CUSTOM_AWB
    return XCAM_RETURN_NO_ERROR;
#else
    return RkAiqAwbHandleInt::genIspResult(params, cur_params);
#endif
}

}  // namespace RkCam
