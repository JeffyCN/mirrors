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

#include "RkAiqCamGroupHandleInt.h"

namespace RkCam {
#if (USE_NEWSTRUCT == 0)
XCamReturn RkAiqCamGroupAdrcHandleInt::updateConfig(bool needSync) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (needSync) mCfgMutex.lock();
    // if something changed
    if (updateAtt) {
#if RKAIQ_HAVE_DRC_V10
        mCurAttV10 = mNewAttV10;
        rk_aiq_uapi_adrc_v10_SetAttrib(mAlgoCtx, &mCurAttV10, false);
        updateAtt = false;
        sendSignal(mCurAttV10.sync.sync_mode);
#endif
#if RKAIQ_HAVE_DRC_V11
        mCurAttV11 = mNewAttV11;
        rk_aiq_uapi_adrc_v11_SetAttrib(mAlgoCtx, &mCurAttV11, false);
        updateAtt = false;
        sendSignal(mCurAttV11.sync.sync_mode);
#endif
#if RKAIQ_HAVE_DRC_V12
        mCurAttV12 = mNewAttV12;
        rk_aiq_uapi_adrc_v12_SetAttrib(mAlgoCtx, &mCurAttV12, false);
        updateAtt = false;
        sendSignal(mCurAttV12.sync.sync_mode);
#endif
#if RKAIQ_HAVE_DRC_V12_LITE
        mCurAttV12Lite = mNewAttV12Lite;
        rk_aiq_uapi_adrc_v12_lite_SetAttrib(mAlgoCtx, &mCurAttV12Lite, false);
        updateAtt = false;
        sendSignal(mCurAttV12Lite.sync.sync_mode);
#endif
#if RKAIQ_HAVE_DRC_V20
        mCurAttV20 = mNewAttV20;
        rk_aiq_uapi_adrc_v20_SetAttrib(mAlgoCtx, &mCurAttV20, false);
        updateAtt = false;
        sendSignal(mCurAttV20.sync.sync_mode);
#endif
    }

    if (needSync) mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
#if RKAIQ_HAVE_DRC_V10
XCamReturn RkAiqCamGroupAdrcHandleInt::setAttribV10(const drcAttrV10_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    // TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed
    if (0 != memcmp(&mCurAttV10, att, sizeof(drcAttrV10_t))) {
        mNewAttV10 = *att;
        updateAtt  = true;
        waitSignal(att->sync.sync_mode);
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCamGroupAdrcHandleInt::getAttribV10(drcAttrV10_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_adrc_v10_GetAttrib(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateAtt) {
            memcpy(att, &mNewAttV10, sizeof(drcAttrV10_t));
            att->sync.done = false;
        } else {
            rk_aiq_uapi_adrc_v10_GetAttrib(mAlgoCtx, att);
            att->sync.sync_mode = mNewAttV10.sync.sync_mode;
            att->sync.done      = true;
        }
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
#endif
#if RKAIQ_HAVE_DRC_V11
XCamReturn RkAiqCamGroupAdrcHandleInt::setAttribV11(const drcAttrV11_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    // TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed
    if (0 != memcmp(&mCurAttV11, att, sizeof(drcAttrV11_t))) {
        mNewAttV11 = *att;
        updateAtt  = true;
        waitSignal(att->sync.sync_mode);
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCamGroupAdrcHandleInt::getAttribV11(drcAttrV11_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_adrc_v11_GetAttrib(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateAtt) {
            memcpy(att, &mNewAttV11, sizeof(drcAttrV11_t));
            att->sync.done = false;
        } else {
            rk_aiq_uapi_adrc_v11_GetAttrib(mAlgoCtx, att);
            att->sync.sync_mode = mNewAttV11.sync.sync_mode;
            att->sync.done      = true;
        }
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
#endif
#if RKAIQ_HAVE_DRC_V12
XCamReturn RkAiqCamGroupAdrcHandleInt::setAttribV12(const drcAttrV12_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    // TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed
    if (0 != memcmp(&mCurAttV12, att, sizeof(drcAttrV12_t))) {
        mNewAttV12 = *att;
        updateAtt = true;
        waitSignal(att->sync.sync_mode);
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCamGroupAdrcHandleInt::getAttribV12(drcAttrV12_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_adrc_v12_GetAttrib(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateAtt) {
            memcpy(att, &mNewAttV12, sizeof(drcAttrV12_t));
            att->sync.done = false;
        } else {
            rk_aiq_uapi_adrc_v12_GetAttrib(mAlgoCtx, att);
            att->sync.sync_mode = mNewAttV12.sync.sync_mode;
            att->sync.done      = true;
        }
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
#endif
#if RKAIQ_HAVE_DRC_V12_LITE
XCamReturn RkAiqCamGroupAdrcHandleInt::setAttribV12Lite(const drcAttrV12Lite_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    // TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed
    if (0 != memcmp(&mCurAttV12Lite, att, sizeof(drcAttrV12Lite_t))) {
        mNewAttV12Lite = *att;
        updateAtt      = true;
        waitSignal(att->sync.sync_mode);
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCamGroupAdrcHandleInt::getAttribV12Lite(drcAttrV12Lite_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_adrc_v12_lite_GetAttrib(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateAtt) {
            memcpy(att, &mNewAttV12Lite, sizeof(drcAttrV12Lite_t));
            att->sync.done = false;
        } else {
            rk_aiq_uapi_adrc_v12_lite_GetAttrib(mAlgoCtx, att);
            att->sync.sync_mode = mNewAttV12Lite.sync.sync_mode;
            att->sync.done      = true;
        }
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
#endif
#if RKAIQ_HAVE_DRC_V20
XCamReturn RkAiqCamGroupAdrcHandleInt::setAttribV20(const drcAttrV20_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    // TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed
    if (0 != memcmp(&mCurAttV20, att, sizeof(drcAttrV20_t))) {
        mNewAttV20 = *att;
        updateAtt  = true;
        waitSignal(att->sync.sync_mode);
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCamGroupAdrcHandleInt::getAttribV20(drcAttrV20_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_adrc_v20_GetAttrib(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateAtt) {
            memcpy(att, &mNewAttV20, sizeof(drcAttrV20_t));
            att->sync.done = false;
        } else {
            rk_aiq_uapi_adrc_v20_GetAttrib(mAlgoCtx, att);
            att->sync.sync_mode = mNewAttV20.sync.sync_mode;
            att->sync.done      = true;
        }
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
#endif
#endif
}  // namespace RkCam
