/*
 * Copyright (c) 2019-2022 Rockchip Eletronics Co., Ltd.
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
#include "RkAiqCamGroupAgicHandle.h"

namespace RkCam {

#if RKAIQ_HAVE_GIC
XCamReturn RkAiqCamGroupAgicHandleInt::updateConfig(bool needSync) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (needSync) mCfgMutex.lock();

    if (updateAttV1) {
        mCurAttV1 = mNewAttV1;
        rk_aiq_uapi_agic_v1_SetAttrib(mAlgoCtx, &mCurAttV1, false);
        updateAtt = false;
        sendSignal(mCurAttV1.sync.sync_mode);
    }

    if (updateAttV2) {
        mCurAttV2 = mNewAttV2;
        rk_aiq_uapi_agic_v2_SetAttrib(mAlgoCtx, &mCurAttV2, false);
        updateAtt = false;
        sendSignal(mCurAttV2.sync.sync_mode);
    }

    if (needSync) mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCamGroupAgicHandleInt::setAttribV1(const rkaiq_gic_v1_api_attr_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

    // check if there is different between att & mCurAtt(sync)/mNewAtt(async)
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore
    bool isChanged = false;
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC && \
        memcmp(&mNewAttV1, att, sizeof(*att)))
        isChanged = true;
    else if (att->sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC && \
             memcmp(&mCurAttV1, att, sizeof(*att)))
        isChanged = true;

    // if something changed
    if (isChanged) {
        mNewAttV1   = *att;
        updateAttV1 = true;
        waitSignal(att->sync.sync_mode);
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCamGroupAgicHandleInt::getAttribV1(rkaiq_gic_v1_api_attr_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_agic_v1_GetAttrib(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateAttV1) {
            memcpy(att, &mNewAttV1, sizeof(mNewAttV1));
            att->sync.done = false;
        } else {
            rk_aiq_uapi_agic_v1_GetAttrib(mAlgoCtx, att);
            att->sync.sync_mode = mNewAttV1.sync.sync_mode;
            att->sync.done      = true;
        }
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCamGroupAgicHandleInt::setAttribV2(const rkaiq_gic_v2_api_attr_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

    // check if there is different between att & mCurAtt(sync)/mNewAtt(async)
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore
    bool isChanged = false;
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC && \
        memcmp(&mNewAttV2, att, sizeof(*att)))
        isChanged = true;
    else if (att->sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC && \
             memcmp(&mCurAttV2, att, sizeof(*att)))
        isChanged = true;

    // if something changed
    if (isChanged) {
        mNewAttV2   = *att;
        updateAttV2 = true;
        waitSignal(att->sync.sync_mode);
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCamGroupAgicHandleInt::getAttribV2(rkaiq_gic_v2_api_attr_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_agic_v2_GetAttrib(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateAttV2) {
            memcpy(att, &mNewAttV2, sizeof(mNewAttV2));
            att->sync.done = false;
        } else {
            rk_aiq_uapi_agic_v2_GetAttrib(mAlgoCtx, att);
            att->sync.sync_mode = mNewAttV2.sync.sync_mode;
            att->sync.done      = true;
        }
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
#endif

}  // namespace RkCam
