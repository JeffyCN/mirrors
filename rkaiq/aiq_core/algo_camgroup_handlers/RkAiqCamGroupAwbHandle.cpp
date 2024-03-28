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

XCamReturn RkAiqCamGroupAwbHandleInt::updateConfig(bool needSync) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (needSync) mCfgMutex.lock();
    // if something changed
    if (updateWbV21Attr) {
        mCurWbV21Attr   = mNewWbV21Attr;
        rk_aiq_uapiV2_camgroup_awbV21_SetAttrib(mAlgoCtx, mCurWbV21Attr, false);
        updateWbV21Attr = false;
        sendSignal(mCurWbV21Attr.sync.sync_mode);
    }
    if (updateWbOpModeAttr) {
        mCurWbOpModeAttr   = mNewWbOpModeAttr;
        rk_aiq_uapiV2_camgroup_awb_SetMwbMode(mAlgoCtx, mCurWbOpModeAttr.mode, false);
        updateWbOpModeAttr = false;
        sendSignal(mCurWbOpModeAttr.sync.sync_mode);
    }
    if (updateWbMwbAttr) {
        mCurWbMwbAttr   = mNewWbMwbAttr;
        rk_aiq_uapiV2_camgroup_awb_SetMwbAttrib(mAlgoCtx, mCurWbMwbAttr, false);
        updateWbMwbAttr = false;
        sendSignal(mCurWbMwbAttr.sync.sync_mode);
    }
    if (updateWbAwbWbGainAdjustAttr) {
        mCurWbAwbWbGainAdjustAttr   = mNewWbAwbWbGainAdjustAttr;
        rk_aiq_uapiV2_camgroup_awb_SetAwbGainAdjust(mAlgoCtx, mCurWbAwbWbGainAdjustAttr, false);
        updateWbAwbWbGainAdjustAttr = false;
        sendSignal(mCurWbAwbWbGainAdjustAttr.sync.sync_mode);
    }
    if (updateWbAwbWbGainOffsetAttr) {
        mCurWbAwbWbGainOffsetAttr   = mNewWbAwbWbGainOffsetAttr;
        rk_aiq_uapiV2_camgroup_awb_SetAwbGainOffset(mAlgoCtx, mCurWbAwbWbGainOffsetAttr.gainOffset, false);
        updateWbAwbWbGainOffsetAttr = false;
        sendSignal(mCurWbAwbWbGainOffsetAttr.sync.sync_mode);
    }
    if (updateWbAwbMultiWindowAttr) {
        mCurWbAwbMultiWindowAttr   = mNewWbAwbMultiWindowAttr;
        rk_aiq_uapiV2_camgroup_awb_SetAwbMultiwindow(mAlgoCtx, mCurWbAwbMultiWindowAttr.multiWindw, false);
        updateWbAwbMultiWindowAttr = false;
        sendSignal(mCurWbAwbMultiWindowAttr.sync.sync_mode);
    }
    // isp32
    if (updateWbV32Attr) {
        mCurWbV32Attr   = mNewWbV32Attr;
        rk_aiq_uapiV2_camgroup_awbV32_SetAttrib(mAlgoCtx, mCurWbV32Attr, false);
        updateWbV32Attr = false;
        sendSignal(mCurWbV32Attr.sync.sync_mode);
    }
    if (updateWbV32AwbMultiWindowAttr) {
        mCurWbV32AwbMultiWindowAttr   = mNewWbV32AwbMultiWindowAttr;
        rk_aiq_uapiV2_camgroup_awbV32_SetAwbMultiwindow(mAlgoCtx, mCurWbV32AwbMultiWindowAttr, false);
        updateWbV32AwbMultiWindowAttr = false;
        sendSignal(mCurWbV32AwbMultiWindowAttr.sync.sync_mode);
    }

    if (needSync) mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCamGroupAwbHandleInt::setWbV21Attrib(rk_aiq_uapiV2_wbV21_attrib_t att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    // TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed
    if (0 != memcmp(&mCurWbV21Attr, &att, sizeof(rk_aiq_uapiV2_wbV21_attrib_t))) {
        mNewWbV21Attr   = att;
        updateWbV21Attr = true;
        waitSignal(att.sync.sync_mode);
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCamGroupAwbHandleInt::getWbV21Attrib(rk_aiq_uapiV2_wbV21_attrib_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapiV2_camgroup_awbV21_GetAttrib(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateWbV21Attr) {
            memcpy(att, &mNewWbV21Attr, sizeof(mNewWbV21Attr));
            att->sync.done = false;
        } else {
            mCfgMutex.lock();
            rk_aiq_uapiV2_camgroup_awbV21_GetAttrib(mAlgoCtx, att);
            mCfgMutex.unlock();
            att->sync.sync_mode = mNewWbV21Attr.sync.sync_mode;
            att->sync.done      = true;
        }
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCamGroupAwbHandleInt::getCct(rk_aiq_wb_cct_t* cct) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapiV2_camgroup_awb_GetCCT(mAlgoCtx, cct);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCamGroupAwbHandleInt::queryWBInfo(rk_aiq_wb_querry_info_t* wb_querry_info) {
    ENTER_ANALYZER_FUNCTION();

    SmartLock lock (mCfgMutex);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapiV2_camgroup_awb_QueryWBInfo(mAlgoCtx, wb_querry_info);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCamGroupAwbHandleInt::lock() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapiV2_camgroup_awb_Lock(mAlgoCtx);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCamGroupAwbHandleInt::unlock() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapiV2_camgroup_awb_Unlock(mAlgoCtx);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCamGroupAwbHandleInt::setWbOpModeAttrib(rk_aiq_uapiV2_wb_opMode_t att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

    // check if there is different between att & mCurAtt(sync)/mNewAtt(async)
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore
    bool isChanged = false;
    if (att.sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC && \
        memcmp(&mNewWbOpModeAttr, &att, sizeof(att)))
        isChanged = true;
    else if (att.sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC && \
             memcmp(&mCurWbOpModeAttr, &att, sizeof(att)))
        isChanged = true;

    // if something changed
    if (isChanged) {
        mNewWbOpModeAttr   = att;
        updateWbOpModeAttr = true;
        waitSignal(att.sync.sync_mode);
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCamGroupAwbHandleInt::getWbOpModeAttrib(rk_aiq_uapiV2_wb_opMode_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapiV2_camgroup_awb_GetMwbMode(mAlgoCtx, &att->mode);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateWbOpModeAttr) {
            memcpy(att, &mNewWbOpModeAttr, sizeof(mNewWbOpModeAttr));
            att->sync.done = false;
        } else {
            rk_aiq_uapiV2_camgroup_awb_GetMwbMode(mAlgoCtx, &att->mode);
            att->sync.sync_mode = mNewWbOpModeAttr.sync.sync_mode;
            att->sync.done = true;
        }
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCamGroupAwbHandleInt::setMwbAttrib(rk_aiq_wb_mwb_attrib_t att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

    // check if there is different between att & mCurAtt(sync)/mNewAtt(async)
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore
    bool isChanged = false;
    if (att.sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC && \
        memcmp(&mNewWbMwbAttr, &att, sizeof(att)))
        isChanged = true;
    else if (att.sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC && \
             memcmp(&mCurWbMwbAttr, &att, sizeof(att)))
        isChanged = true;

    // if something changed
    if (isChanged) {
        mNewWbMwbAttr   = att;
        updateWbMwbAttr = true;
        waitSignal(att.sync.sync_mode);
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCamGroupAwbHandleInt::getMwbAttrib(rk_aiq_wb_mwb_attrib_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapiV2_camgroup_awb_GetMwbAttrib(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateWbMwbAttr) {
            memcpy(att, &mNewWbMwbAttr, sizeof(mNewWbMwbAttr));
            att->sync.done = false;
        } else {
            rk_aiq_uapiV2_camgroup_awb_GetMwbAttrib(mAlgoCtx, att);
            att->sync.sync_mode = mNewWbMwbAttr.sync.sync_mode;
            att->sync.done = true;
        }
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCamGroupAwbHandleInt::setWbAwbWbGainAdjustAttrib(rk_aiq_uapiV2_wb_awb_wbGainAdjust_t att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

    // check if there is different between att & mCurAtt(sync)/mNewAtt(async)
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore
    bool isChanged = false;
    if (att.sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC && \
        memcmp(&mNewWbAwbWbGainAdjustAttr, &att, sizeof(att)))
        isChanged = true;
    else if (att.sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC && \
             memcmp(&mCurWbAwbWbGainAdjustAttr, &att, sizeof(att)))
        isChanged = true;

    // if something changed
    if (isChanged) {
        mNewWbAwbWbGainAdjustAttr   = att;
        updateWbAwbWbGainAdjustAttr = true;
        waitSignal(att.sync.sync_mode);
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCamGroupAwbHandleInt::getWbAwbWbGainAdjustAttrib(rk_aiq_uapiV2_wb_awb_wbGainAdjust_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapiV2_camgroup_awb_GetAwbGainAdjust(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateWbAwbWbGainAdjustAttr) {
            memcpy(att, &mNewWbAwbWbGainAdjustAttr, sizeof(mNewWbAwbWbGainAdjustAttr));
            att->sync.done = false;
        } else {
            mCfgMutex.lock();
            rk_aiq_uapiV2_camgroup_awb_GetAwbGainAdjust(mAlgoCtx, att);
            mCfgMutex.unlock();
            att->sync.sync_mode = mNewWbAwbWbGainAdjustAttr.sync.sync_mode;
            att->sync.done      = true;
        }
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCamGroupAwbHandleInt::setWbAwbWbGainOffsetAttrib(rk_aiq_uapiV2_wb_awb_wbGainOffset_t att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

    // check if there is different between att & mCurAtt(sync)/mNewAtt(async)
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore
    bool isChanged = false;
    if (att.sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC && \
        memcmp(&mNewWbAwbWbGainOffsetAttr, &att, sizeof(att)))
        isChanged = true;
    else if (att.sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC && \
             memcmp(&mCurWbAwbWbGainOffsetAttr, &att, sizeof(att)))
        isChanged = true;

    // if something changed
    if (isChanged) {
        mNewWbAwbWbGainOffsetAttr   = att;
        updateWbAwbWbGainOffsetAttr = true;
        waitSignal(att.sync.sync_mode);
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCamGroupAwbHandleInt::getWbAwbWbGainOffsetAttrib(rk_aiq_uapiV2_wb_awb_wbGainOffset_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapiV2_camgroup_awb_GetAwbGainOffset(mAlgoCtx, &att->gainOffset);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateWbAwbWbGainOffsetAttr) {
            memcpy(att, &mNewWbAwbWbGainOffsetAttr, sizeof(mNewWbAwbWbGainOffsetAttr));
            att->sync.done = false;
        } else {
            mCfgMutex.unlock();
            rk_aiq_uapiV2_camgroup_awb_GetAwbGainOffset(mAlgoCtx, &att->gainOffset);
            mCfgMutex.unlock();
            att->sync.sync_mode = mNewWbAwbWbGainOffsetAttr.sync.sync_mode;
            att->sync.done      = true;
        }
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCamGroupAwbHandleInt::setWbAwbMultiWindowAttrib(rk_aiq_uapiV2_wb_awb_mulWindow_t att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

    // check if there is different between att & mCurAtt(sync)/mNewAtt(async)
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore
    bool isChanged = false;
    if (att.sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC && \
        memcmp(&mNewWbAwbMultiWindowAttr, &att, sizeof(att)))
        isChanged = true;
    else if (att.sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC && \
             memcmp(&mCurWbAwbMultiWindowAttr, &att, sizeof(att)))
        isChanged = true;

    // if something changed
    if (isChanged) {
        mNewWbAwbMultiWindowAttr   = att;
        updateWbAwbMultiWindowAttr = true;
        waitSignal(att.sync.sync_mode);
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCamGroupAwbHandleInt::getWbAwbMultiWindowAttrib(rk_aiq_uapiV2_wb_awb_mulWindow_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapiV2_camgroup_awb_GetAwbMultiwindow(mAlgoCtx, &att->multiWindw);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateWbAwbMultiWindowAttr) {
            memcpy(att, &mNewWbAwbMultiWindowAttr, sizeof(mNewWbAwbMultiWindowAttr));
            att->sync.done = false;
        } else {
            mCfgMutex.lock();
            rk_aiq_uapiV2_camgroup_awb_GetAwbMultiwindow(mAlgoCtx, &att->multiWindw);
            mCfgMutex.unlock();
            att->sync.sync_mode = mNewWbAwbMultiWindowAttr.sync.sync_mode;
            att->sync.done      = true;
        }
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCamGroupAwbHandleInt::getAlgoStat(rk_tool_awb_stat_res_full_t *awb_stat_algo) {
    ENTER_ANALYZER_FUNCTION();

    SmartLock lock (mCfgMutex);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapiV2_camgroup_awb_GetAlgoStat(mAlgoCtx, awb_stat_algo);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCamGroupAwbHandleInt::getStrategyResult(rk_tool_awb_strategy_result_t *awb_strategy_result) {
    ENTER_ANALYZER_FUNCTION();

    SmartLock lock (mCfgMutex);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapiV2_camgroup_awb_GetStrategyResult(mAlgoCtx, awb_strategy_result);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCamGroupAwbHandleInt::setWbV32AwbMultiWindowAttrib(rk_aiq_uapiV2_wbV32_awb_mulWindow_t att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

    // check if there is different between att & mCurAtt(sync)/mNewAtt(async)
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore
    bool isChanged = false;
    if (att.sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC && \
        memcmp(&mNewWbV32AwbMultiWindowAttr, &att, sizeof(att)))
        isChanged = true;
    else if (att.sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC && \
            memcmp(&mNewWbV32AwbMultiWindowAttr, &att, sizeof(att)))
        isChanged = true;

    // if something changed
    if (isChanged) {
        mNewWbV32AwbMultiWindowAttr   = att;
        updateWbV32AwbMultiWindowAttr = true;
        waitSignal(att.sync.sync_mode);
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCamGroupAwbHandleInt::getWbV32AwbMultiWindowAttrib(rk_aiq_uapiV2_wbV32_awb_mulWindow_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapiV2_camgroup_awbV32_GetAwbMultiwindow(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateWbV32AwbMultiWindowAttr) {
            memcpy(att, &mNewWbV32AwbMultiWindowAttr, sizeof(mNewWbV32AwbMultiWindowAttr));
            att->sync.done = false;
        } else {
            mCfgMutex.lock();
            rk_aiq_uapiV2_camgroup_awbV32_GetAwbMultiwindow(mAlgoCtx, att);
            mCfgMutex.unlock();
            att->sync.sync_mode = mNewWbV32AwbMultiWindowAttr.sync.sync_mode;
            att->sync.done      = true;
        }
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCamGroupAwbHandleInt::setWbV32Attrib(rk_aiq_uapiV2_wbV32_attrib_t att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

    if (0 != memcmp(&mNewWbV32Attr, &att, sizeof(att))) {
        mNewWbV32Attr = att;
        updateWbV32Attr = true;
        waitSignal(att.sync.sync_mode);
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCamGroupAwbHandleInt::getWbV32Attrib(rk_aiq_uapiV2_wbV32_attrib_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        att->sync.done = true;
        rk_aiq_uapiV2_camgroup_awbV32_GetAttrib(mAlgoCtx, att);
        mCfgMutex.unlock();
    } else {
        if (updateWbV32Attr) {
             memcpy(att, &mNewWbV32Attr, sizeof(mNewWbV32Attr));
             att->sync.done = false;
        } else {
            mCfgMutex.lock();
            rk_aiq_uapiV2_camgroup_awbV32_GetAttrib(mAlgoCtx, att);
            mCfgMutex.unlock();
            att->sync.sync_mode = mNewWbV32Attr.sync.sync_mode;
            att->sync.done = true;
        }
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCamGroupAwbHandleInt::setWbV32IQAutoExtPara(const rk_aiq_uapiV2_Wb_Awb_IqAtExtPa_V32_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

    ret = rk_aiq_uapiV2_camgroup_awb_SetIQAutoExtPara(mAlgoCtx, att, false);

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCamGroupAwbHandleInt::getWbV32IQAutoExtPara(rk_aiq_uapiV2_Wb_Awb_IqAtExtPa_V32_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mCfgMutex.lock();
    rk_aiq_uapiV2_camgroup_awb_GetIQAutoExtPara(mAlgoCtx, att);
    //att->sync.done = true;
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCamGroupAwbHandleInt::setWbV32IQAutoPara(const rk_aiq_uapiV2_Wb_Awb_IqAtPa_V32_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

    ret = rk_aiq_uapiV2_camgroup_awb_SetIQAutoPara(mAlgoCtx, att, false);

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCamGroupAwbHandleInt::getWbV32IQAutoPara(rk_aiq_uapiV2_Wb_Awb_IqAtPa_V32_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mCfgMutex.lock();
    rk_aiq_uapiV2_camgroup_awb_GetIQAutoPara(mAlgoCtx, att);
    //att->sync.done = true;
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

}  // namespace RkCam
