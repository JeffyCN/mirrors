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
#ifndef DISABLE_HANDLE_ATTRIB
    if (needSync) mCfgMutex.lock();
    // if something changed
    if (updateAtt) {
        mCurAtt   = mNewAtt;
        rk_aiq_uapi_awb_SetAttrib(mAlgoCtx, mCurAtt, false);
        updateAtt = false;
        sendSignal();
    }
    if (updateWbV21Attr) {
        rk_aiq_uapiV2_wb_awb_wbGainAdjust_t wbGainAdjustBK = mCurWbV21Attr.stAuto.wbGainAdjust;
        mCurWbV21Attr = mNewWbV21Attr;
        mCurWbV21Attr.stAuto.wbGainAdjust = wbGainAdjustBK;
        mallocAndCopyWbGainAdjustAttrib(&mCurWbV21Attr.stAuto.wbGainAdjust,&mNewWbV21Attr.stAuto.wbGainAdjust);
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
        rk_aiq_uapiV2_wb_awb_wbGainAdjust_t wbGainAdjustBK = mCurWbAwbAttr.wbGainAdjust;
        mCurWbAwbAttr   = mNewWbAwbAttr;
        mCurWbAwbAttr.wbGainAdjust = wbGainAdjustBK;
        mallocAndCopyWbGainAdjustAttrib(&mCurWbAwbAttr.wbGainAdjust,&mNewWbAwbAttr.wbGainAdjust);
        rk_aiq_uapiV2_awbV20_SetAwbAttrib(mAlgoCtx, mCurWbAwbAttr, false);
        updateWbAwbAttr = false;
        sendSignal();
    }
    if (updateWbAwbWbGainAdjustAttr) {
        mallocAndCopyWbGainAdjustAttrib(&mCurWbAwbWbGainAdjustAttr,&mNewWbAwbWbGainAdjustAttr);
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
    if (updateFFWbgainAttr) {
        mCurFFWbgainAttr   = mNewFFWbgainAttr;
        rk_aiq_uapiV2_awb_SetFstFrWbgain(mAlgoCtx, mCurFFWbgainAttr.wggain, false);
        updateFFWbgainAttr = false;
        sendSignal(mCurFFWbgainAttr.sync.sync_mode);
    }
    if (needSync) mCfgMutex.unlock();
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbV21HandleInt::setWbV21Attrib(rk_aiq_uapiV2_wbV21_attrib_t att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

#ifdef DISABLE_HANDLE_ATTRIB
    ret = rk_aiq_uapiV2_awbV21_SetAttrib(mAlgoCtx, att, false);
#else
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
        rk_aiq_uapiV2_wb_awb_wbGainAdjust_t wbGainAdjustBK = mNewWbV21Attr.stAuto.wbGainAdjust;
        mNewWbV21Attr = att;
        mNewWbV21Attr.stAuto.wbGainAdjust = wbGainAdjustBK;
        mallocAndCopyWbGainAdjustAttrib(&mNewWbV21Attr.stAuto.wbGainAdjust,&att.stAuto.wbGainAdjust);
        updateWbV21Attr = true;
        waitSignal(att.sync.sync_mode);
    }
#endif

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbV21HandleInt::getWbV21Attrib(rk_aiq_uapiV2_wbV21_attrib_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
    rk_aiq_uapiV2_awbV21_GetAttrib(mAlgoCtx, att);
    mCfgMutex.unlock();
#else
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapiV2_awbV21_GetAttrib(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateWbV21Attr) {
            rk_aiq_uapiV2_wb_awb_wbGainAdjust_t wbGainAdjustBK = att->stAuto.wbGainAdjust;
            memcpy(att, &mNewWbV21Attr, sizeof(mNewWbV21Attr));
            att->stAuto.wbGainAdjust = wbGainAdjustBK;
            mallocAndCopyWbGainAdjustAttrib(&att->stAuto.wbGainAdjust,&mNewWbV21Attr.stAuto.wbGainAdjust);
            att->sync.done = false;
        } else {
            mCfgMutex.lock();
            rk_aiq_uapiV2_awbV21_GetAttrib(mAlgoCtx, att);
            mCfgMutex.unlock();
            att->sync.sync_mode = mNewWbV21Attr.sync.sync_mode;
            att->sync.done = true;
        }
    }
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

}  // namespace RkCam
