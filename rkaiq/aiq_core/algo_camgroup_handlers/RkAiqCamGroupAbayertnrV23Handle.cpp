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
#include "abayertnrV23/rk_aiq_uapi_camgroup_abayertnr_int_v23.h"

namespace RkCam {
#if (USE_NEWSTRUCT == 0)
#if (RKAIQ_HAVE_BAYERTNR_V23 || RKAIQ_HAVE_BAYERTNR_V23_LITE)
XCamReturn RkAiqCamGroupAbayertnrV23HandleInt::updateConfig(bool needSync) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (needSync) mCfgMutex.lock();
    // if something changed
    if (updateAtt) {
        LOGD_ANR("%s:%d\n", __FUNCTION__, __LINE__);
        mCurAtt   = mNewAtt;
        rk_aiq_uapi_camgroup_abayertnrV23_SetAttrib(mAlgoCtx, &mCurAtt, false);
        sendSignal(mCurAtt.sync.sync_mode);
        updateAtt = false;
    }

    if (updateAttLite) {
        LOGD_ANR("%s:%d\n", __FUNCTION__, __LINE__);
        mCurAttLite = mNewAttLite;
        rk_aiq_uapi_camgroup_abayertnrV23Lite_SetAttrib(mAlgoCtx, &mCurAttLite, false);
        sendSignal(mCurAttLite.sync.sync_mode);
        updateAttLite = false;
    }

#if 1
    if (updateStrength) {
        LOGD_ANR("%s:%d\n", __FUNCTION__, __LINE__);
        mCurStrength = mNewStrength;
        rk_aiq_uapi_camgroup_abayertnrV23_SetStrength(mAlgoCtx, &mCurStrength);
        sendSignal(mCurStrength.sync.sync_mode);
        updateStrength = false;
    }
#endif

    if (needSync) mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCamGroupAbayertnrV23HandleInt::setAttrib(const rk_aiq_bayertnr_attrib_v23_t* att) {
    ENTER_ANALYZER_FUNCTION();
    LOGD_ANR("%s:%d\n", __FUNCTION__, __LINE__);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    // TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed
    if (0 != memcmp(&mCurAtt, att, sizeof(rk_aiq_bayertnr_attrib_v23_t))) {
        mNewAtt   = *att;
        updateAtt = true;
        waitSignal(att->sync.sync_mode);

    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCamGroupAbayertnrV23HandleInt::getAttrib(rk_aiq_bayertnr_attrib_v23_t* att) {
    ENTER_ANALYZER_FUNCTION();
    LOGD_ANR("%s:%d\n", __FUNCTION__, __LINE__);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if(att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_camgroup_abayertnrV23_GetAttrib(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if(updateAtt) {
            memcpy(att, &mNewAtt, sizeof(mNewAtt));
            att->sync.done = false;
        } else {
            rk_aiq_uapi_camgroup_abayertnrV23_GetAttrib(mAlgoCtx, att);
            att->sync.done = true;
        }
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCamGroupAbayertnrV23HandleInt::setAttribLite(
    const rk_aiq_bayertnr_attrib_v23L_t* att) {
    ENTER_ANALYZER_FUNCTION();
    LOGD_ANR("%s:%d\n", __FUNCTION__, __LINE__);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    // TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed
    if (0 != memcmp(&mCurAttLite, att, sizeof(rk_aiq_bayertnr_attrib_v23L_t))) {
        mNewAttLite   = *att;
        updateAttLite = true;
        waitSignal(att->sync.sync_mode);
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCamGroupAbayertnrV23HandleInt::getAttribLite(rk_aiq_bayertnr_attrib_v23L_t* att) {
    ENTER_ANALYZER_FUNCTION();
    LOGD_ANR("%s:%d\n", __FUNCTION__, __LINE__);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_camgroup_abayertnrV23Lite_GetAttrib(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateAttLite) {
            memcpy(att, &mNewAttLite, sizeof(mNewAttLite));
            att->sync.done = false;
        } else {
            rk_aiq_uapi_camgroup_abayertnrV23Lite_GetAttrib(mAlgoCtx, att);
            att->sync.done = true;
        }
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

#if 1

XCamReturn RkAiqCamGroupAbayertnrV23HandleInt::setStrength(const rk_aiq_bayertnr_strength_v23_t *pStrength) {
    ENTER_ANALYZER_FUNCTION();
    LOGD_ANR("%s:%d\n", __FUNCTION__, __LINE__);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mCfgMutex.lock();

    if (0 != memcmp(&mCurStrength, pStrength, sizeof(mCurStrength))) {
        mNewStrength   = *pStrength;
        updateStrength = true;
        waitSignal(pStrength->sync.sync_mode);
    }

    mCfgMutex.unlock();
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCamGroupAbayertnrV23HandleInt::getStrength(rk_aiq_bayertnr_strength_v23_t *pStrength) {
    ENTER_ANALYZER_FUNCTION();
    LOGD_ANR("%s:%d\n", __FUNCTION__, __LINE__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if(pStrength->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_camgroup_abayertnrV23_GetStrength(mAlgoCtx, pStrength);
        pStrength->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if(updateStrength) {
            *pStrength = mNewStrength;
            pStrength->sync.done = false;
        } else {
            rk_aiq_uapi_camgroup_abayertnrV23_GetStrength(mAlgoCtx, pStrength);
            pStrength->sync.done = true;
        }
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
#endif

XCamReturn RkAiqCamGroupAbayertnrV23HandleInt::getInfo(rk_aiq_bayertnr_info_v23_t *pInfo) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if(pInfo->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_abayertnrV23_GetInfo(mAlgoCtx, pInfo);
        pInfo->sync.done = true;
        mCfgMutex.unlock();
    } else {
        rk_aiq_uapi_abayertnrV23_GetInfo(mAlgoCtx, pInfo);
        pInfo->sync.done = true;
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
#endif
#endif
}
