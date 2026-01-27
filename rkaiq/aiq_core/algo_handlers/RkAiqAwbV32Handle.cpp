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
#include "RkAiqAwbV32Handle.h"

#include "RkAiqCore.h"

namespace RkCam {

DEFINE_HANDLE_REGISTER_TYPE(RkAiqAwbV32HandleInt);

XCamReturn RkAiqAwbV32HandleInt::updateConfig(bool needSync) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#ifndef DISABLE_HANDLE_ATTRIB
    if (needSync) mCfgMutex.lock();
    // if something changed
    if (updateWbV32Attr) {
        mCurWbV32Attr   = mNewWbV32Attr;
        rk_aiq_uapiV2_awbV32_SetAttrib(mAlgoCtx, mCurWbV32Attr, false);
        updateWbV32Attr = false;
        sendSignal(mCurWbV32Attr.sync.sync_mode);
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
    if (updateWbAwbWbGainOffsetAttr) {
        mCurWbAwbWbGainOffsetAttr   = mNewWbAwbWbGainOffsetAttr;
        rk_aiq_uapiV2_awb_SetAwbGainOffset(mAlgoCtx, mCurWbAwbWbGainOffsetAttr.gainOffset, false);
        updateWbAwbWbGainOffsetAttr = false;
        sendSignal(mCurWbAwbWbGainOffsetAttr.sync.sync_mode);
    }
    if (updateWbV32AwbMultiWindowAttr) {
        mCurWbV32AwbMultiWindowAttr   = mNewWbV32AwbMultiWindowAttr;
        rk_aiq_uapiV2_awbV32_SetAwbMultiwindow(mAlgoCtx, mCurWbV32AwbMultiWindowAttr, false);
        updateWbV32AwbMultiWindowAttr = false;
        sendSignal(mCurWbV32AwbMultiWindowAttr.sync.sync_mode);
    }
    if (updateWriteAwbInputAttr) {
        mCurWriteAwbInputAttr   = mNewWriteAwbInputAttr;
        rk_aiq_uapiV2_awb_WriteInput(mAlgoCtx, mCurWriteAwbInputAttr, false);
        updateWriteAwbInputAttr = false;
        sendSignal(mCurWriteAwbInputAttr.sync.sync_mode);
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
XCamReturn RkAiqAwbV32HandleInt::setWbV32AwbMultiWindowAttrib(rk_aiq_uapiV2_wbV32_awb_mulWindow_t att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

#ifdef DISABLE_HANDLE_ATTRIB
    ret = rk_aiq_uapiV2_awbV32_SetAwbMultiwindow(mAlgoCtx, att, false);
#else
    // check if there is different between att & mCurAtt(sync)/mNewAtt(async)
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore
    bool isChanged = false;
    if (att.sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC && \
        memcmp(&mNewWbV32AwbMultiWindowAttr, &att, sizeof(att)))
        isChanged = true;
    else if (att.sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC && \
             memcmp(&mCurWbV32AwbMultiWindowAttr, &att, sizeof(att)))
        isChanged = true;

    // if something changed
    if (isChanged) {
        mNewWbV32AwbMultiWindowAttr   = att;
        updateWbV32AwbMultiWindowAttr = true;
        waitSignal(att.sync.sync_mode);
    }
#endif

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbV32HandleInt::getWbV32AwbMultiWindowAttrib(rk_aiq_uapiV2_wbV32_awb_mulWindow_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
    rk_aiq_uapiV2_awbV32_GetAwbMultiwindow(mAlgoCtx, att);
    att->sync.done = true;
    mCfgMutex.unlock();
#else
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapiV2_awbV32_GetAwbMultiwindow(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateWbV32AwbMultiWindowAttr) {
            memcpy(att, &mNewWbV32AwbMultiWindowAttr, sizeof(mNewWbV32AwbMultiWindowAttr));
            att->sync.done = false;
        } else {
            mCfgMutex.lock();
            rk_aiq_uapiV2_awbV32_GetAwbMultiwindow(mAlgoCtx, att);
            mCfgMutex.unlock();
            att->sync.sync_mode = mNewWbV32AwbMultiWindowAttr.sync.sync_mode;
            att->sync.done      = true;
        }
    }
#endif
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbV32HandleInt::setWbV32Attrib(rk_aiq_uapiV2_wbV32_attrib_t att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

#ifdef DISABLE_HANDLE_ATTRIB
    ret = rk_aiq_uapiV2_awbV32_SetAttrib(mAlgoCtx, att, false);
#else
    // check if there is different between att & mCurAtt(sync)/mNewAtt(async)
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore
    bool isChanged = false;
    if (att.sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC && \
        memcmp(&mNewWbV32Attr, &att, sizeof(att)))
        isChanged = true;
    else if (att.sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC && \
             memcmp(&mCurWbV32Attr, &att, sizeof(att)))
        isChanged = true;

    // if something changed
    if (isChanged) {
        mNewWbV32Attr = att;
        updateWbV32Attr = true;
        waitSignal(att.sync.sync_mode);
    }
#endif

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbV32HandleInt::getWbV32Attrib(rk_aiq_uapiV2_wbV32_attrib_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
    rk_aiq_uapiV2_awbV32_GetAttrib(mAlgoCtx, att);
    att->sync.done = true;
    mCfgMutex.unlock();
#else
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapiV2_awbV32_GetAttrib(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateWbV32Attr) {
             memcpy(att, &mNewWbV32Attr, sizeof(mNewWbV32Attr));
             att->sync.done = false;
        } else {
            mCfgMutex.lock();
            rk_aiq_uapiV2_awbV32_GetAttrib(mAlgoCtx, att);
            mCfgMutex.unlock();
            att->sync.sync_mode = mNewWbV32Attr.sync.sync_mode;
            att->sync.done = true;
        }
    }
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
XCamReturn RkAiqAwbV32HandleInt::writeAwbIn(rk_aiq_uapiV2_awb_wrtIn_attr_t att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

#ifdef DISABLE_HANDLE_ATTRIB
    ret = rk_aiq_uapiV2_awb_WriteInput(mAlgoCtx, att, false);
#else
    // check if there is different between att & mCurAtt(sync)/mNewAtt(async)
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore
    bool isChanged = false;
    if (att.sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC && \
        memcmp(&mNewWriteAwbInputAttr, &att, sizeof(att)))
        isChanged = true;
    else if (att.sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC && \
             memcmp(&mCurWriteAwbInputAttr, &att, sizeof(att)))
        isChanged = true;

    // if something changed
    if (isChanged) {
        mNewWriteAwbInputAttr = att;
        updateWriteAwbInputAttr = true;
        waitSignal(att.sync.sync_mode);
    }
#endif

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbV32HandleInt::setWbV32IQAutoExtPara(const rk_aiq_uapiV2_Wb_Awb_IqAtExtPa_V32_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

#ifdef DISABLE_HANDLE_ATTRIB
    ret = rk_aiq_uapiV2_awb_SetIQAutoExtPara(mAlgoCtx, att, false);
#endif

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbV32HandleInt::getWbV32IQAutoExtPara(rk_aiq_uapiV2_Wb_Awb_IqAtExtPa_V32_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
    rk_aiq_uapiV2_awb_GetIQAutoExtPara(mAlgoCtx, att);
    //att->sync.done = true;
    mCfgMutex.unlock();
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbV32HandleInt::setWbV32IQAutoPara(const rk_aiq_uapiV2_Wb_Awb_IqAtPa_V32_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

#ifdef DISABLE_HANDLE_ATTRIB
    ret = rk_aiq_uapiV2_awb_SetIQAutoPara(mAlgoCtx, att, false);
#endif

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbV32HandleInt::getWbV32IQAutoPara(rk_aiq_uapiV2_Wb_Awb_IqAtPa_V32_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
    rk_aiq_uapiV2_awb_GetIQAutoPara(mAlgoCtx, att);
    //att->sync.done = true;
    mCfgMutex.unlock();
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
XCamReturn RkAiqAwbV32HandleInt::awbIqMap2Main( rk_aiq_uapiV2_awb_Slave2Main_Cfg_t att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

#ifdef DISABLE_HANDLE_ATTRIB
    ret = rk_aiq_uapiV2_awb_IqMap2Main(mAlgoCtx, att, false);
#endif

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbV32HandleInt::setAwbPreWbgain( const float att[4]) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

#ifdef DISABLE_HANDLE_ATTRIB
    ret = rk_aiq_uapiV2_awb_SetPreWbgain(mAlgoCtx, att, false);
#endif

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

}  // namespace RkCam
