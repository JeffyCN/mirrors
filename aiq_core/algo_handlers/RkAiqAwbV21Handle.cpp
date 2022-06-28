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
#include "RkAiqAwbV21Handle.h"

#include "RkAiqCore.h"

namespace RkCam {

DEFINE_HANDLE_REGISTER_TYPE(RkAiqAwbV21HandleInt);

XCamReturn RkAiqAwbV21HandleInt::updateConfig(bool needSync) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (needSync) mCfgMutex.lock();
    // if something changed
    if (updateAtt) {
        mCurAtt   = mNewAtt;
        rk_aiq_uapi_awb_SetAttrib(mAlgoCtx, mCurAtt, false);
        updateAtt = false;
        sendSignal();
    }
    if (updateWbV21Attr) {
        mCurWbV21Attr   = mNewWbV21Attr;
        rk_aiq_uapiV2_awbV21_SetAttrib(mAlgoCtx, mCurWbV21Attr, false);
        updateWbV21Attr = false;
        sendSignal(mCurWbV21Attr.sync.sync_mode);
    }
    if (updateWbOpModeAttr) {
        mCurWbOpModeAttr   = mNewWbOpModeAttr;
        rk_aiq_uapiV2_awb_SetMwbMode(mAlgoCtx, mCurWbOpModeAttr.mode, false);
        updateWbOpModeAttr = false;
        sendSignal(mCurWbOpModeAttr.sync.sync_mode);
    }
    if (updateWbMwbAttr) {
        mCurWbMwbAttr   = mNewWbMwbAttr;
        rk_aiq_uapiV2_awb_SetMwbAttrib(mAlgoCtx, mCurWbMwbAttr, false);
        updateWbMwbAttr = false;
        sendSignal(mCurWbMwbAttr.sync.sync_mode);
    }
    if (updateWbAwbAttr) {
        mCurWbAwbAttr   = mNewWbAwbAttr;
        rk_aiq_uapiV2_awbV20_SetAwbAttrib(mAlgoCtx, mCurWbAwbAttr, false);
        updateWbAwbAttr = false;
        sendSignal();
    }
    if (updateWbAwbWbGainAdjustAttr) {
        mCurWbAwbWbGainAdjustAttr   = mNewWbAwbWbGainAdjustAttr;
        rk_aiq_uapiV2_awb_SetAwbGainAdjust(mAlgoCtx, mCurWbAwbWbGainAdjustAttr, false);
        updateWbAwbWbGainAdjustAttr = false;
        sendSignal(mCurWbAwbWbGainAdjustAttr.sync.sync_mode);
    }
    if (updateWbAwbWbGainOffsetAttr) {
        mCurWbAwbWbGainOffsetAttr   = mNewWbAwbWbGainOffsetAttr;
        rk_aiq_uapiV2_awb_SetAwbGainOffset(mAlgoCtx, mCurWbAwbWbGainOffsetAttr.gainOffset, false);
        updateWbAwbWbGainOffsetAttr = false;
        sendSignal(mCurWbAwbWbGainOffsetAttr.sync.sync_mode);
    }
    if (updateWbAwbMultiWindowAttr) {
        mCurWbAwbMultiWindowAttr   = mNewWbAwbMultiWindowAttr;
        rk_aiq_uapiV2_awb_SetAwbMultiwindow(mAlgoCtx, mCurWbAwbMultiWindowAttr.multiWindw, false);
        updateWbAwbMultiWindowAttr = false;
        sendSignal(mCurWbAwbMultiWindowAttr.sync.sync_mode);
    }
    if (needSync) mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbV21HandleInt::setWbV21Attrib(rk_aiq_uapiV2_wbV21_attrib_t att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

    // check if there is different between att & mCurAtt(sync)/mNewAtt(async)
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore
    bool isChanged = false;
    if (att.sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC && \
        memcmp(&mNewWbV21Attr, &att, sizeof(att)))
        isChanged = true;
    else if (att.sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC && \
             memcmp(&mCurWbV21Attr, &att, sizeof(att)))
        isChanged = true;

    // if something changed
    if (isChanged) {
        mNewWbV21Attr = att;
        updateWbV21Attr = true;
        waitSignal(att.sync.sync_mode);
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbV21HandleInt::getWbV21Attrib(rk_aiq_uapiV2_wbV21_attrib_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapiV2_awbV21_GetAttrib(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateWbV21Attr) {
            memcpy(att, &mNewWbV21Attr, sizeof(mNewWbV21Attr));
            att->sync.done = false;
        } else {
            rk_aiq_uapiV2_awbV21_GetAttrib(mAlgoCtx, att);
            att->sync.sync_mode = mNewWbV21Attr.sync.sync_mode;
            att->sync.done = true;
        }
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

};  // namespace RkCam
