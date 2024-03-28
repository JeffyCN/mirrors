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

XCamReturn RkAiqCamGroupAccmHandleInt::updateConfig(bool needSync) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (needSync) mCfgMutex.lock();
    // if something changed
    if (updateAtt) {
#if RKAIQ_HAVE_CCM_V1
        mCurAtt   = mNewAtt;
        // TODO
        rk_aiq_uapi_accm_SetAttrib(mAlgoCtx, &mCurAtt, false);
        updateAtt = false;
        sendSignal(mCurAtt.sync.sync_mode);
#endif
#if RKAIQ_HAVE_CCM_V2
        mCurAttV2   = mNewAttV2;
        // TODO
        rk_aiq_uapi_accm_v2_SetAttrib(mAlgoCtx, &mCurAttV2, false);
        updateAtt = false;
        sendSignal(mCurAttV2.sync.sync_mode);
#endif
#if RKAIQ_HAVE_CCM_V3
        mCurAttV3   = mNewAttV3;
        // TODO
        rk_aiq_uapi_accm_v3_SetAttrib(mAlgoCtx, &mCurAttV3, false);
        updateAtt = false;
        sendSignal(mCurAttV3.sync.sync_mode);
#endif
    }
    if (updateCalibAttr) {
#if RKAIQ_HAVE_CCM_V1
        mCurCalibAtt   = mNewCalibAtt;
        // TODO
        rk_aiq_uapi_accm_SetIqParam(mAlgoCtx, &mCurCalibAtt, false);
        updateCalibAttr = false;
        sendSignal(mCurCalibAtt.sync.sync_mode);
#endif
#if RKAIQ_HAVE_CCM_V2
        mCurCalibAttV2   = mNewCalibAttV2;
        // TODO
        rk_aiq_uapi_accm_SetIqParam(mAlgoCtx, &mCurCalibAttV2, false);
        updateCalibAttr = false;
        sendSignal(mCurCalibAttV2.sync.sync_mode);
#endif
    }

    if (needSync) mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
#if RKAIQ_HAVE_CCM_V1
XCamReturn RkAiqCamGroupAccmHandleInt::setAttrib(const rk_aiq_ccm_attrib_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCAM_ASSERT(att != nullptr);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

    // check if there is different between att & mCurAtt(sync)/mNewAtt(async)
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore
    bool isChanged = false;
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC && \
        memcmp(&mNewAtt, att, sizeof(*att)))
        isChanged = true;
    else if (att->sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC && \
             memcmp(&mCurAtt, att, sizeof(*att)))
        isChanged = true;

    // if something changed
    if (isChanged) {
        mNewAtt   = *att;
        updateAtt = true;
        waitSignal(att->sync.sync_mode);
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCamGroupAccmHandleInt::getAttrib(rk_aiq_ccm_attrib_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCAM_ASSERT(att != nullptr);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_accm_GetAttrib(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateAtt) {
            memcpy(att, &mNewAtt, sizeof(mNewAtt));
            att->sync.done = false;
        } else {
            rk_aiq_uapi_accm_GetAttrib(mAlgoCtx, att);
            att->sync.sync_mode = mNewAtt.sync.sync_mode;
            att->sync.done = true;
        }
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCamGroupAccmHandleInt::setIqParam(const rk_aiq_ccm_calib_attrib_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCAM_ASSERT(att != nullptr);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

    // check if there is different between att & mCurAtt(sync)/mNewAtt(async)
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore
    bool isChanged = false;
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC && \
        memcmp(&mNewCalibAtt, att, sizeof(*att)))
        isChanged = true;
    else if (att->sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC && \
             memcmp(&mCurCalibAtt, att, sizeof(*att)))
        isChanged = true;

    // if something changed
    if (isChanged) {
        mNewCalibAtt   = *att;
        updateCalibAttr = true;
        waitSignal(att->sync.sync_mode);
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCamGroupAccmHandleInt::getIqParam(rk_aiq_ccm_calib_attrib_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCAM_ASSERT(att != nullptr);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_accm_GetIqParam(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateCalibAttr) {
            memcpy(att, &mNewCalibAtt, sizeof(mNewCalibAtt));
            att->sync.done = false;
        } else {
            rk_aiq_uapi_accm_GetIqParam(mAlgoCtx, att);
            att->sync.sync_mode = mNewCalibAtt.sync.sync_mode;
            att->sync.done = true;
        }
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
#endif

#if RKAIQ_HAVE_CCM_V2
XCamReturn RkAiqCamGroupAccmHandleInt::setAttribV2(const rk_aiq_ccm_v2_attrib_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCAM_ASSERT(att != nullptr);

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
        updateAtt = true;
        waitSignal(att->sync.sync_mode);
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCamGroupAccmHandleInt::getAttribV2(rk_aiq_ccm_v2_attrib_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCAM_ASSERT(att != nullptr);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_accm_v2_GetAttrib(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateAtt) {
            memcpy(att, &mNewAttV2, sizeof(mNewAttV2));
            att->sync.done = false;
        } else {
            rk_aiq_uapi_accm_v2_GetAttrib(mAlgoCtx, att);
            att->sync.sync_mode = mNewAttV2.sync.sync_mode;
            att->sync.done = true;
        }
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCamGroupAccmHandleInt::setIqParamV2(const rk_aiq_ccm_v2_calib_attrib_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCAM_ASSERT(att != nullptr);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

    // check if there is different between att & mCurAtt(sync)/mNewAtt(async)
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore
    bool isChanged = false;
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC && \
        memcmp(&mNewCalibAttV2, att, sizeof(*att)))
        isChanged = true;
    else if (att->sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC && \
             memcmp(&mCurCalibAttV2, att, sizeof(*att)))
        isChanged = true;

    // if something changed
    if (isChanged) {
        mNewCalibAttV2   = *att;
        updateCalibAttr = true;
        waitSignal(att->sync.sync_mode);
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCamGroupAccmHandleInt::getIqParamV2(rk_aiq_ccm_v2_calib_attrib_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCAM_ASSERT(att != nullptr);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_accm_GetIqParam(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateCalibAttr) {
            memcpy(att, &mNewCalibAttV2, sizeof(mNewCalibAttV2));
            att->sync.done = false;
        } else {
            rk_aiq_uapi_accm_GetIqParam(mAlgoCtx, att);
            att->sync.sync_mode = mNewCalibAttV2.sync.sync_mode;
            att->sync.done = true;
        }
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
#endif

#if RKAIQ_HAVE_CCM_V3
XCamReturn RkAiqCamGroupAccmHandleInt::setAttribV3(const rk_aiq_ccm_v3_attrib_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCAM_ASSERT(att != nullptr);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

    // check if there is different between att & mCurAtt(sync)/mNewAtt(async)
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore
    bool isChanged = false;
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC && \
        memcmp(&mNewAttV3, att, sizeof(*att)))
        isChanged = true;
    else if (att->sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC && \
             memcmp(&mCurAttV3, att, sizeof(*att)))
        isChanged = true;

    // if something changed
    if (isChanged) {
        mNewAttV3   = *att;
        updateAtt = true;
        waitSignal(att->sync.sync_mode);
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCamGroupAccmHandleInt::getAttribV3(rk_aiq_ccm_v3_attrib_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCAM_ASSERT(att != nullptr);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_accm_v3_GetAttrib(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateAtt) {
            memcpy(att, &mNewAttV3, sizeof(mNewAttV3));
            att->sync.done = false;
        } else {
            rk_aiq_uapi_accm_v3_GetAttrib(mAlgoCtx, att);
            att->sync.sync_mode = mNewAttV3.sync.sync_mode;
            att->sync.done = true;
        }
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
#endif

XCamReturn RkAiqCamGroupAccmHandleInt::queryCcmInfo(rk_aiq_ccm_querry_info_t* ccm_querry_info) {
    ENTER_ANALYZER_FUNCTION();

    XCAM_ASSERT(ccm_querry_info != nullptr);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_accm_QueryCcmInfo(mAlgoCtx, ccm_querry_info);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}


}  // namespace RkCam
