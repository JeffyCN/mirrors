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
#include "RkAiqAeHandle.h"
#include "RkAiqAfHandle.h"
#include "RkAiqAfdHandle.h"
#include "RkAiqAmergeHandle.h"
#include "smart_buffer_priv.h"
#if USE_NEWSTRUCT
#include "newStruct/RkAiqDrcHandler.h"
#else
#include "RkAiqAdrcHandle.h"
#endif
#include "RkAiqCore.h"

namespace RkCam {

DEFINE_HANDLE_REGISTER_TYPE(RkAiqAeHandleInt);

void RkAiqAeHandleInt::init() {
    ENTER_ANALYZER_FUNCTION();

    RkAiqHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAe());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAe());
    mPreOutParam  = NULL;
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAe());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAe());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAe());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAe());
#if RKAIQ_HAVE_AF
    mAf_handle = mAiqCore->getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_AF);
#endif
#if RKAIQ_HAVE_AFD_V1 || RKAIQ_HAVE_AFD_V2
    mAfd_handle = mAiqCore->getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_AFD);
#endif

    mAmerge_handle = mAiqCore->getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_AMERGE);
    mAdrc_handle = mAiqCore->getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_ADRC);

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqAeHandleInt::updateConfig(bool needSync) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#ifndef DISABLE_HANDLE_ATTRIB
    if (needSync) mCfgMutex.lock();
    // if something changed, api will modify aecCfg in mAlgoCtx
    if (updateExpSwAttr) {
        mCurExpSwAttr   = mNewExpSwAttr;
        rk_aiq_uapi_ae_convExpSwAttr_v1Tov2(&mCurExpSwAttr, &mCurExpSwAttrV2);
        rk_aiq_uapi_ae_setExpSwAttr(mAlgoCtx, &mCurExpSwAttrV2, false, false);
        updateExpSwAttr = false;
        updateAttr |= UPDATE_EXPSWATTR;
        sendSignal();
    }

    if (updateLinExpAttr) {
        mCurLinExpAttr   = mNewLinExpAttr;
        rk_aiq_uapi_ae_convLinExpAttr_v1Tov2(&mCurLinExpAttr, &mCurLinExpAttrV2);
        rk_aiq_uapi_ae_setLinExpAttr(mAlgoCtx, &mCurLinExpAttrV2, false, false);
        updateLinExpAttr = false;
        updateAttr |= UPDATE_LINEXPATTR;
        sendSignal();
    }

    if (updateHdrExpAttr) {
        mCurHdrExpAttr   = mNewHdrExpAttr;
        rk_aiq_uapi_ae_convHdrExpAttr_v1Tov2(&mCurHdrExpAttr, &mCurHdrExpAttrV2);
        rk_aiq_uapi_ae_setHdrExpAttr(mAlgoCtx, &mCurHdrExpAttrV2, false, false);
        updateHdrExpAttr = false;
        updateAttr |= UPDATE_HDREXPATTR;
        sendSignal();
    }

    // TODO: update v2

    if (updateExpSwAttrV2) {
        mCurExpSwAttrV2   = mNewExpSwAttrV2;
        rk_aiq_uapi_ae_setExpSwAttr(mAlgoCtx, &mCurExpSwAttrV2, false, false);
        updateExpSwAttrV2 = false;
        updateAttr |= UPDATE_EXPSWATTR;
        sendSignal(mCurExpSwAttrV2.sync.sync_mode);
    }

    if (updateLinExpAttrV2) {
        mCurLinExpAttrV2   = mNewLinExpAttrV2;
        rk_aiq_uapi_ae_setLinExpAttr(mAlgoCtx, &mCurLinExpAttrV2, false, false);
        updateLinExpAttrV2 = false;
        updateAttr |= UPDATE_LINEXPATTR;
        sendSignal(mCurLinExpAttrV2.sync.sync_mode);
    }

    if (updateHdrExpAttrV2) {
        mCurHdrExpAttrV2   = mNewHdrExpAttrV2;
        rk_aiq_uapi_ae_setHdrExpAttr(mAlgoCtx, &mCurHdrExpAttrV2, false, false);
        updateHdrExpAttrV2 = false;
        updateAttr |= UPDATE_HDREXPATTR;
        sendSignal(mCurHdrExpAttrV2.sync.sync_mode);
    }

    if (updateLinAeRouteAttr) {
        mCurLinAeRouteAttr   = mNewLinAeRouteAttr;
        rk_aiq_uapi_ae_setLinAeRouteAttr(mAlgoCtx, &mCurLinAeRouteAttr, false, false);
        updateLinAeRouteAttr = false;
        updateAttr |= UPDATE_LINAEROUTEATTR;
        sendSignal(mCurLinAeRouteAttr.sync.sync_mode);
    }
    if (updateHdrAeRouteAttr) {
        mCurHdrAeRouteAttr   = mNewHdrAeRouteAttr;
        rk_aiq_uapi_ae_setHdrAeRouteAttr(mAlgoCtx, &mCurHdrAeRouteAttr, false, false);
        updateHdrAeRouteAttr = false;
        updateAttr |= UPDATE_HDRAEROUTEATTR;
        sendSignal(mCurHdrAeRouteAttr.sync.sync_mode);
    }
    if (updateIrisAttr) {
        mCurIrisAttr   = mNewIrisAttr;
        rk_aiq_uapi_ae_setIrisAttr(mAlgoCtx, &mCurIrisAttr, false);
        updateIrisAttr = false;
        updateAttr |= UPDATE_IRISATTR;
        sendSignal(mCurIrisAttr.sync.sync_mode);
    }
    if (updateSyncTestAttr) {
        mCurAecSyncTestAttr = mNewAecSyncTestAttr;
        rk_aiq_uapi_ae_setSyncTest(mAlgoCtx, &mCurAecSyncTestAttr, false, false);
        updateSyncTestAttr  = false;
        updateAttr |= UPDATE_SYNCTESTATTR;
        sendSignal(mCurAecSyncTestAttr.sync.sync_mode);
    }
    if (updateExpWinAttr) {
        mCurExpWinAttr   = mNewExpWinAttr;
        rk_aiq_uapi_ae_setExpWinAttr(mAlgoCtx, &mCurExpWinAttr, false);
        updateExpWinAttr = false;
        updateAttr |= UPDATE_EXPWINATTR;
        sendSignal(mCurExpWinAttr.sync.sync_mode);
    }
    if (updateAecStatsCfg) {
        mCurAecStatsCfg   = mNewAecStatsCfg;
        rk_aiq_uapi_ae_setAecStatsCfg(mAlgoCtx, &mCurAecStatsCfg, false, false);
        updateAecStatsCfg = false;
        updateAttr |= UPDATE_AECSTATSCFG;
        sendSignal(mCurAecStatsCfg.sync.sync_mode);
    }

    // once any params are changed, run reconfig to convert aecCfg to paectx
    AeInstanceConfig_t* pAeInstConfig = (AeInstanceConfig_t*)mAlgoCtx;
    AeConfig_t pAecCfg                = pAeInstConfig->aecCfg;
    pAecCfg->IsReconfig |= updateAttr;
    updateAttr = 0;
    if (needSync) mCfgMutex.unlock();
#endif
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAeHandleInt::setExpSwAttr(Uapi_ExpSwAttr_t ExpSwAttr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
#ifdef DISABLE_HANDLE_ATTRIB
    Uapi_ExpSwAttrV2_t att2;
    rk_aiq_uapi_ae_convExpSwAttr_v1Tov2(&ExpSwAttr, &att2);
    rk_aiq_uapi_ae_setExpSwAttr(mAlgoCtx, &att2, false, false);
    AeInstanceConfig_t* pAeInstConfig = (AeInstanceConfig_t*)mAlgoCtx;
    AeConfig_t pAecCfg                = pAeInstConfig->aecCfg;
    pAecCfg->IsReconfig |= UPDATE_EXPSWATTR;
#else
    // TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed
    if (0 != memcmp(&mCurExpSwAttr, &ExpSwAttr, sizeof(Uapi_ExpSwAttr_t))) {
        mNewExpSwAttr   = ExpSwAttr;
        updateExpSwAttr = true;
        waitSignal();
    }
#endif
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAeHandleInt::getExpSwAttr(Uapi_ExpSwAttr_t* pExpSwAttr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    Uapi_ExpSwAttrV2_t ExpSwAttrV2;

    mCfgMutex.lock();

    rk_aiq_uapi_ae_getExpSwAttr(mAlgoCtx, &ExpSwAttrV2, false);
    rk_aiq_uapi_ae_convExpSwAttr_v2Tov1(&ExpSwAttrV2, pExpSwAttr);

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAeHandleInt::setExpSwAttr(Uapi_ExpSwAttrV2_t ExpSwAttrV2) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

#ifdef DISABLE_HANDLE_ATTRIB
    rk_aiq_uapi_ae_setExpSwAttr(mAlgoCtx, &ExpSwAttrV2, false, false);
    AeInstanceConfig_t* pAeInstConfig = (AeInstanceConfig_t*)mAlgoCtx;
    AeConfig_t pAecCfg                = pAeInstConfig->aecCfg;
    pAecCfg->IsReconfig |= UPDATE_EXPSWATTR;
#else
    // check if there is different between att & mCurAtt(sync)/mNewAtt(async)
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore
    bool isChanged = false;
    if (ExpSwAttrV2.sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC && \
            memcmp(&mNewExpSwAttrV2, &ExpSwAttrV2, sizeof(ExpSwAttrV2)))
        isChanged = true;
    else if (ExpSwAttrV2.sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC && \
             memcmp(&mCurExpSwAttrV2, &ExpSwAttrV2, sizeof(ExpSwAttrV2)))
        isChanged = true;

    // if something changed
    if (isChanged) {
        mNewExpSwAttrV2   = ExpSwAttrV2;
        updateExpSwAttrV2 = true;
        waitSignal(ExpSwAttrV2.sync.sync_mode);
    }
#endif
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAeHandleInt::getExpSwAttr(Uapi_ExpSwAttrV2_t* pExpSwAttrV2) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
    rk_aiq_uapi_ae_getExpSwAttr(mAlgoCtx, pExpSwAttrV2, false);
    pExpSwAttrV2->sync.done = true;
    mCfgMutex.unlock();
#else
    if (pExpSwAttrV2->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_ae_getExpSwAttr(mAlgoCtx, pExpSwAttrV2, false);
        pExpSwAttrV2->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateExpSwAttrV2) {
            memcpy(pExpSwAttrV2, &mNewExpSwAttrV2, sizeof(mNewExpSwAttrV2));
            pExpSwAttrV2->sync.done = false;
        } else {
            rk_aiq_uapi_ae_getExpSwAttr(mAlgoCtx, pExpSwAttrV2, false);
            pExpSwAttrV2->sync.sync_mode = mNewExpSwAttrV2.sync.sync_mode;
            pExpSwAttrV2->sync.done = true;
        }
    }
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAeHandleInt::setLinExpAttr(Uapi_LinExpAttr_t LinExpAttr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
#ifdef DISABLE_HANDLE_ATTRIB
    Uapi_LinExpAttrV2_t att2;
    rk_aiq_uapi_ae_convLinExpAttr_v1Tov2(&LinExpAttr, &att2);
    rk_aiq_uapi_ae_setLinExpAttr(mAlgoCtx, &att2, false, false);
    AeInstanceConfig_t* pAeInstConfig = (AeInstanceConfig_t*)mAlgoCtx;
    AeConfig_t pAecCfg                = pAeInstConfig->aecCfg;
    pAecCfg->IsReconfig |= UPDATE_LINEXPATTR;
#else
    // TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed
    if (0 != memcmp(& mCurLinExpAttr, &LinExpAttr, sizeof(Uapi_LinExpAttr_t))) {
        mNewLinExpAttr   = LinExpAttr;
        updateLinExpAttr = true;
        waitSignal();
    }
#endif
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAeHandleInt::getLinExpAttr(Uapi_LinExpAttr_t* pLinExpAttr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mCfgMutex.lock();
    Uapi_LinExpAttrV2_t att2;
    rk_aiq_uapi_ae_getLinExpAttr(mAlgoCtx, &att2, false);
    rk_aiq_uapi_ae_convLinExpAttr_v2Tov1(&att2, pLinExpAttr);
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAeHandleInt::setLinExpAttr(Uapi_LinExpAttrV2_t LinExpAttrV2) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
#ifdef DISABLE_HANDLE_ATTRIB
    rk_aiq_uapi_ae_setLinExpAttr(mAlgoCtx, &LinExpAttrV2, false, false);
    AeInstanceConfig_t* pAeInstConfig = (AeInstanceConfig_t*)mAlgoCtx;
    AeConfig_t pAecCfg                = pAeInstConfig->aecCfg;
    pAecCfg->IsReconfig |= UPDATE_LINEXPATTR;
#else
    // check if there is different between att & mCurAtt(sync)/mNewAtt(async)
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore
    bool isChanged = false;
    if (LinExpAttrV2.sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC && \
            memcmp(&mNewLinExpAttrV2, &LinExpAttrV2, sizeof(LinExpAttrV2)))
        isChanged = true;
    else if (LinExpAttrV2.sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC && \
             memcmp(&mCurLinExpAttrV2, &LinExpAttrV2, sizeof(LinExpAttrV2)))
        isChanged = true;

    // if something changed
    if (isChanged) {
        mNewLinExpAttrV2   = LinExpAttrV2;
        updateLinExpAttrV2 = true;
        waitSignal(LinExpAttrV2.sync.sync_mode);
    }
#endif
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAeHandleInt::getLinExpAttr(Uapi_LinExpAttrV2_t* pLinExpAttrV2) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
    rk_aiq_uapi_ae_getLinExpAttr(mAlgoCtx, pLinExpAttrV2, false);
    pLinExpAttrV2->sync.done = true;
    mCfgMutex.unlock();
#else
    if (pLinExpAttrV2->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_ae_getLinExpAttr(mAlgoCtx, pLinExpAttrV2, false);
        pLinExpAttrV2->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateLinExpAttrV2) {
            memcpy(pLinExpAttrV2, &mNewLinExpAttrV2, sizeof(mNewLinExpAttrV2));
            pLinExpAttrV2->sync.done = false;
        } else {
            rk_aiq_uapi_ae_getLinExpAttr(mAlgoCtx, pLinExpAttrV2, false);
            pLinExpAttrV2->sync.sync_mode = mNewLinExpAttrV2.sync.sync_mode;
            pLinExpAttrV2->sync.done      = true;
        }
    }
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAeHandleInt::setHdrExpAttr(Uapi_HdrExpAttr_t HdrExpAttr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
#ifdef DISABLE_HANDLE_ATTRIB
    Uapi_HdrExpAttrV2_t att2;
    rk_aiq_uapi_ae_convHdrExpAttr_v1Tov2(&HdrExpAttr, &att2);
    rk_aiq_uapi_ae_setHdrExpAttr(mAlgoCtx, &att2, false, false);
    AeInstanceConfig_t* pAeInstConfig = (AeInstanceConfig_t*)mAlgoCtx;
    AeConfig_t pAecCfg                = pAeInstConfig->aecCfg;
    pAecCfg->IsReconfig |= UPDATE_HDREXPATTR;
#else
    // TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed
    if (0 != memcmp(&mCurHdrExpAttr, &HdrExpAttr, sizeof(Uapi_HdrExpAttr_t))) {
        mNewHdrExpAttr   = HdrExpAttr;
        updateHdrExpAttr = true;
        waitSignal();
    }
#endif
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAeHandleInt::getHdrExpAttr(Uapi_HdrExpAttr_t* pHdrExpAttr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mCfgMutex.lock();
    Uapi_HdrExpAttrV2_t att2;
    rk_aiq_uapi_ae_getHdrExpAttr(mAlgoCtx, &att2, false);
    rk_aiq_uapi_ae_convHdrExpAttr_v2Tov1(&att2, pHdrExpAttr);
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAeHandleInt::setHdrExpAttr(Uapi_HdrExpAttrV2_t HdrExpAttrV2) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

#ifdef DISABLE_HANDLE_ATTRIB
    rk_aiq_uapi_ae_setHdrExpAttr(mAlgoCtx, &HdrExpAttrV2, false, false);
    AeInstanceConfig_t* pAeInstConfig = (AeInstanceConfig_t*)mAlgoCtx;
    AeConfig_t pAecCfg                = pAeInstConfig->aecCfg;
    pAecCfg->IsReconfig |= UPDATE_HDREXPATTR;
#else
    bool isChanged = false;
    if (HdrExpAttrV2.sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC && \
            memcmp(&mNewHdrExpAttrV2, &HdrExpAttrV2, sizeof(HdrExpAttrV2)))
        isChanged = true;
    else if (HdrExpAttrV2.sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC && \
             memcmp(&mCurHdrExpAttrV2, &HdrExpAttrV2, sizeof(HdrExpAttrV2)))
        isChanged = true;

    // if something changed
    if (isChanged) {
        mNewHdrExpAttrV2   = HdrExpAttrV2;
        updateHdrExpAttrV2 = true;
        waitSignal(HdrExpAttrV2.sync.sync_mode);
    }
#endif
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAeHandleInt::getHdrExpAttr(Uapi_HdrExpAttrV2_t* pHdrExpAttrV2) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
    rk_aiq_uapi_ae_getHdrExpAttr(mAlgoCtx, pHdrExpAttrV2, false);
    pHdrExpAttrV2->sync.done = true;
    mCfgMutex.unlock();
#else
    if (pHdrExpAttrV2->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_ae_getHdrExpAttr(mAlgoCtx, pHdrExpAttrV2, false);
        pHdrExpAttrV2->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateHdrExpAttrV2) {
            memcpy(pHdrExpAttrV2, &mNewHdrExpAttrV2, sizeof(mNewHdrExpAttrV2));
            pHdrExpAttrV2->sync.done = false;
        } else {
            rk_aiq_uapi_ae_getHdrExpAttr(mAlgoCtx, pHdrExpAttrV2, false);
            pHdrExpAttrV2->sync.sync_mode = mNewHdrExpAttrV2.sync.sync_mode;
            pHdrExpAttrV2->sync.done      = true;
        }
    }
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAeHandleInt::setLinAeRouteAttr(Uapi_LinAeRouteAttr_t LinAeRouteAttr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

#ifdef DISABLE_HANDLE_ATTRIB
    rk_aiq_uapi_ae_setLinAeRouteAttr(mAlgoCtx, &LinAeRouteAttr, false, false);
    AeInstanceConfig_t* pAeInstConfig = (AeInstanceConfig_t*)mAlgoCtx;
    AeConfig_t pAecCfg                = pAeInstConfig->aecCfg;
    pAecCfg->IsReconfig |= UPDATE_LINAEROUTEATTR;
#else
    bool isChanged = false;
    if (LinAeRouteAttr.sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC && \
            memcmp(&mNewLinAeRouteAttr, &LinAeRouteAttr, sizeof(LinAeRouteAttr)))
        isChanged = true;
    else if (LinAeRouteAttr.sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC && \
             memcmp(&mCurLinAeRouteAttr, &LinAeRouteAttr, sizeof(LinAeRouteAttr)))
        isChanged = true;

    // if something changed
    if (isChanged) {
        mNewLinAeRouteAttr   = LinAeRouteAttr;
        updateLinAeRouteAttr = true;
        waitSignal(LinAeRouteAttr.sync.sync_mode);
    }
#endif
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAeHandleInt::getLinAeRouteAttr(Uapi_LinAeRouteAttr_t* pLinAeRouteAttr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
    rk_aiq_uapi_ae_getLinAeRouteAttr(mAlgoCtx, pLinAeRouteAttr, false);
    pLinAeRouteAttr->sync.done = true;
    mCfgMutex.unlock();
#else
    if (pLinAeRouteAttr->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_ae_getLinAeRouteAttr(mAlgoCtx, pLinAeRouteAttr, false);
        pLinAeRouteAttr->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateLinAeRouteAttr) {
            memcpy(pLinAeRouteAttr, &mNewLinAeRouteAttr, sizeof(mNewLinAeRouteAttr));
            pLinAeRouteAttr->sync.done = false;
        } else {
            rk_aiq_uapi_ae_getLinAeRouteAttr(mAlgoCtx, pLinAeRouteAttr, false);
            pLinAeRouteAttr->sync.sync_mode = mNewLinAeRouteAttr.sync.sync_mode;
            pLinAeRouteAttr->sync.done      = true;
        }
    }
#endif
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAeHandleInt::setHdrAeRouteAttr(Uapi_HdrAeRouteAttr_t HdrAeRouteAttr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

#ifdef DISABLE_HANDLE_ATTRIB
    rk_aiq_uapi_ae_setHdrAeRouteAttr(mAlgoCtx, &HdrAeRouteAttr, false, false);
    AeInstanceConfig_t* pAeInstConfig = (AeInstanceConfig_t*)mAlgoCtx;
    AeConfig_t pAecCfg                = pAeInstConfig->aecCfg;
    pAecCfg->IsReconfig |= UPDATE_HDRAEROUTEATTR;
#else
    bool isChanged = false;
    if (HdrAeRouteAttr.sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC && \
            memcmp(&mNewHdrAeRouteAttr, &HdrAeRouteAttr, sizeof(HdrAeRouteAttr)))
        isChanged = true;
    else if (HdrAeRouteAttr.sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC && \
             memcmp(&mCurHdrAeRouteAttr, &HdrAeRouteAttr, sizeof(HdrAeRouteAttr)))
        isChanged = true;

    // if something changed
    if (isChanged) {
        mNewHdrAeRouteAttr   = HdrAeRouteAttr;
        updateHdrAeRouteAttr = true;
        waitSignal(HdrAeRouteAttr.sync.sync_mode);
    }
#endif
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAeHandleInt::getHdrAeRouteAttr(Uapi_HdrAeRouteAttr_t* pHdrAeRouteAttr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
    rk_aiq_uapi_ae_getHdrAeRouteAttr(mAlgoCtx, pHdrAeRouteAttr, false);
    pHdrAeRouteAttr->sync.done = true;
    mCfgMutex.unlock();
#else
    if (pHdrAeRouteAttr->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_ae_getHdrAeRouteAttr(mAlgoCtx, pHdrAeRouteAttr, false);
        pHdrAeRouteAttr->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateHdrAeRouteAttr) {
            memcpy(pHdrAeRouteAttr, &mNewHdrAeRouteAttr, sizeof(mNewHdrAeRouteAttr));
            pHdrAeRouteAttr->sync.done = false;
        } else {
            rk_aiq_uapi_ae_getHdrAeRouteAttr(mAlgoCtx, pHdrAeRouteAttr, false);
            pHdrAeRouteAttr->sync.sync_mode = mNewHdrAeRouteAttr.sync.sync_mode;
            pHdrAeRouteAttr->sync.done      = true;
        }
    }
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAeHandleInt::setIrisAttr(Uapi_IrisAttrV2_t IrisAttr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

#ifdef DISABLE_HANDLE_ATTRIB
    rk_aiq_uapi_ae_setIrisAttr(mAlgoCtx, &IrisAttr, false);
    AeInstanceConfig_t* pAeInstConfig = (AeInstanceConfig_t*)mAlgoCtx;
    AeConfig_t pAecCfg                = pAeInstConfig->aecCfg;
    pAecCfg->IsReconfig |= UPDATE_IRISATTR;
#else
    bool isChanged = false;
    if (IrisAttr.sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC && \
            memcmp(&mNewIrisAttr, &IrisAttr, sizeof(IrisAttr)))
        isChanged = true;
    else if (IrisAttr.sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC && \
             memcmp(&mCurIrisAttr, &IrisAttr, sizeof(IrisAttr)))
        isChanged = true;

    // if something changed
    if (isChanged) {
        mNewIrisAttr   = IrisAttr;
        updateIrisAttr = true;
        waitSignal(IrisAttr.sync.sync_mode);
    }
#endif

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAeHandleInt::getIrisAttr(Uapi_IrisAttrV2_t* pIrisAttr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
    rk_aiq_uapi_ae_getIrisAttr(mAlgoCtx, pIrisAttr);
    pIrisAttr->sync.done = true;
    mCfgMutex.unlock();
#else
    if (pIrisAttr->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_ae_getIrisAttr(mAlgoCtx, pIrisAttr);
        pIrisAttr->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateIrisAttr) {
            memcpy(pIrisAttr, &mNewIrisAttr, sizeof(mNewIrisAttr));
            pIrisAttr->sync.done = false;
        } else {
            rk_aiq_uapi_ae_getIrisAttr(mAlgoCtx, pIrisAttr);
            pIrisAttr->sync.sync_mode = mNewIrisAttr.sync.sync_mode;
            pIrisAttr->sync.done      = true;
        }
    }
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAeHandleInt::setSyncTestAttr(Uapi_AecSyncTest_t SyncTestAttr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

#ifdef DISABLE_HANDLE_ATTRIB
    rk_aiq_uapi_ae_setSyncTest(mAlgoCtx, &SyncTestAttr, false, false);
    AeInstanceConfig_t* pAeInstConfig = (AeInstanceConfig_t*)mAlgoCtx;
    AeConfig_t pAecCfg                = pAeInstConfig->aecCfg;
    pAecCfg->IsReconfig |= UPDATE_SYNCTESTATTR;
#else
    bool isChanged = false;
    if (SyncTestAttr.sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC && \
            memcmp(&mNewAecSyncTestAttr, &SyncTestAttr, sizeof(SyncTestAttr)))
        isChanged = true;
    else if (SyncTestAttr.sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC && \
             memcmp(&mCurAecSyncTestAttr, &SyncTestAttr, sizeof(SyncTestAttr)))
        isChanged = true;

    // if something changed
    if (isChanged) {
        mNewAecSyncTestAttr = SyncTestAttr;
        updateSyncTestAttr  = true;
        waitSignal(SyncTestAttr.sync.sync_mode);
    }
#endif

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAeHandleInt::getSyncTestAttr(Uapi_AecSyncTest_t* pSyncTestAttr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
    rk_aiq_uapi_ae_getSyncTest(mAlgoCtx, pSyncTestAttr, false);
    pSyncTestAttr->sync.done = true;
    mCfgMutex.unlock();
#else
    if (pSyncTestAttr->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_ae_getSyncTest(mAlgoCtx, pSyncTestAttr, false);
        pSyncTestAttr->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateSyncTestAttr) {
            memcpy(pSyncTestAttr, &mNewAecSyncTestAttr, sizeof(mNewAecSyncTestAttr));
            pSyncTestAttr->sync.done = false;
        } else {
            rk_aiq_uapi_ae_getSyncTest(mAlgoCtx, pSyncTestAttr, false);
            pSyncTestAttr->sync.sync_mode = mNewAecSyncTestAttr.sync.sync_mode;
            pSyncTestAttr->sync.done      = true;
        }
    }
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAeHandleInt::setExpWinAttr(Uapi_ExpWin_t ExpWinAttr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

#ifdef DISABLE_HANDLE_ATTRIB
    rk_aiq_uapi_ae_setExpWinAttr(mAlgoCtx, &ExpWinAttr, false);
    AeInstanceConfig_t* pAeInstConfig = (AeInstanceConfig_t*)mAlgoCtx;
    AeConfig_t pAecCfg                = pAeInstConfig->aecCfg;
    pAecCfg->IsReconfig |= UPDATE_EXPWINATTR;
#else
    bool isChanged = false;
    if (ExpWinAttr.sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC && \
            memcmp(&mNewExpWinAttr, &ExpWinAttr, sizeof(ExpWinAttr)))
        isChanged = true;
    else if (ExpWinAttr.sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC && \
             memcmp(&mCurExpWinAttr, &ExpWinAttr, sizeof(ExpWinAttr)))
        isChanged = true;

    // if something changed
    if (isChanged) {
        mNewExpWinAttr   = ExpWinAttr;
        updateExpWinAttr = true;
        waitSignal(ExpWinAttr.sync.sync_mode);
    }
#endif

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAeHandleInt::getExpWinAttr(Uapi_ExpWin_t* pExpWinAttr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
    rk_aiq_uapi_ae_getExpWinAttr(mAlgoCtx, pExpWinAttr);
    pExpWinAttr->sync.done = true;
    mCfgMutex.unlock();
#else
    if (pExpWinAttr->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_ae_getExpWinAttr(mAlgoCtx, pExpWinAttr);
        pExpWinAttr->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateExpWinAttr) {
            memcpy(pExpWinAttr, &mNewExpWinAttr, sizeof(mNewExpWinAttr));
            pExpWinAttr->sync.done = false;
        } else {
            rk_aiq_uapi_ae_getExpWinAttr(mAlgoCtx, pExpWinAttr);
            pExpWinAttr->sync.sync_mode = mNewExpWinAttr.sync.sync_mode;
            pExpWinAttr->sync.done      = true;
        }
    }
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAeHandleInt::setAecStatsCfg(Uapi_AecStatsCfg_t        AecStatsCfg) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

#ifdef DISABLE_HANDLE_ATTRIB
    rk_aiq_uapi_ae_setAecStatsCfg(mAlgoCtx, &AecStatsCfg, false, false);
    AeInstanceConfig_t* pAeInstConfig = (AeInstanceConfig_t*)mAlgoCtx;
    AeConfig_t pAecCfg                = pAeInstConfig->aecCfg;
    pAecCfg->IsReconfig |= UPDATE_AECSTATSCFG;
#else
    bool isChanged = false;
    if (AecStatsCfg.sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC && \
            memcmp(&mNewAecStatsCfg, &AecStatsCfg, sizeof(AecStatsCfg)))
        isChanged = true;
    else if (AecStatsCfg.sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC && \
             memcmp(&mCurAecStatsCfg, &AecStatsCfg, sizeof(AecStatsCfg)))
        isChanged = true;

    // if something changed
    if (isChanged) {
        mNewAecStatsCfg   = AecStatsCfg;
        updateAecStatsCfg = true;
        waitSignal(AecStatsCfg.sync.sync_mode);
    }
#endif

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAeHandleInt::getAecStatsCfg(Uapi_AecStatsCfg_t* pAecStatsCfg) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
    rk_aiq_uapi_ae_getAecStatsCfg(mAlgoCtx, pAecStatsCfg, false);
    pAecStatsCfg->sync.done = true;
    mCfgMutex.unlock();
#else
    if (pAecStatsCfg->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_ae_getAecStatsCfg(mAlgoCtx, pAecStatsCfg, false);
        pAecStatsCfg->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateAecStatsCfg) {
            memcpy(pAecStatsCfg, &mNewAecStatsCfg, sizeof(mNewAecStatsCfg));
            pAecStatsCfg->sync.done = false;
        } else {
            rk_aiq_uapi_ae_getExpWinAttr(mAlgoCtx, pAecStatsCfg);
            pAecStatsCfg->sync.sync_mode = mNewAecStatsCfg.sync.sync_mode;
            pAecStatsCfg->sync.done      = true;
        }
    }
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAeHandleInt::queryExpInfo(Uapi_ExpQueryInfo_t* pExpQueryInfo) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_ae_queryExpInfo(mAlgoCtx, pExpQueryInfo, false);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAeHandleInt::setLockAeForAf(bool lock_ae) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mLockAebyAfMutex.lock();
    lockaebyaf = lock_ae;
    mLockAebyAfMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAeHandleInt::getAfdResForAE(AfdPeakRes_t AfdRes) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mGetAfdResMutex.lock();
    mAfdRes = AfdRes;
    mGetAfdResMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAeHandleInt::setAOVForAE(bool en) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    rk_aiq_uapi_ae_setAOV(mAlgoCtx, en);
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAeHandleInt::prepare() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "ae handle prepare failed");

    RkAiqAlgoConfigAe* ae_config_int = (RkAiqAlgoConfigAe*)mConfig;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    // TODO config ae common params:

    /*****************AecConfig Sensor Exp related info*****************/
    ae_config_int->LinePeriodsPerField = (float)sharedCom->snsDes.frame_length_lines;
    ae_config_int->PixelPeriodsPerLine = (float)sharedCom->snsDes.line_length_pck;
    ae_config_int->PixelClockFreqMHZ   = (float)sharedCom->snsDes.pixel_clock_freq_mhz;

    /*****************AecConfig pic-info params*****************/
    ae_config_int->RawWidth  = sharedCom->snsDes.isp_acq_width;
    ae_config_int->RawHeight = sharedCom->snsDes.isp_acq_height;
    ae_config_int->nr_switch = sharedCom->snsDes.nr_switch;
    ae_config_int->compr_bit = sharedCom->snsDes.compr_bit;
    ae_config_int->dcg_ratio = sharedCom->snsDes.dcg_ratio;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "ae algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAeHandleInt::preProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPreAe* ae_pre_int        = (RkAiqAlgoPreAe*)mPreInParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    RkAiqAecStats* xAecStats = nullptr;
    if (shared->aecStatsBuf) {
        xAecStats = shared->aecStatsBuf;
        if (!xAecStats) LOGE_AEC("aec stats is null");
    } else {
        LOGW_AEC("the xcamvideobuffer of aec stats is null");
    }
    if ((!xAecStats || !xAecStats->aec_stats_valid) && !sharedCom->init) {
        LOGW("no aec stats, ignore!");
        mPreResShared = NULL;
        return XCAM_RETURN_BYPASS;
    }

    int algoId = this->getAlgoId();
    if (algoId == 0)
        mPreResShared = new RkAiqAlgoPreResAeIntShared();
    if (!mPreResShared.ptr()) {
        LOGE("new ae mPreOutParam failed, bypass!");
        return XCAM_RETURN_BYPASS;
    }

    ret = RkAiqHandle::preProcess();
    if (ret < 0) {
        LOGE_ANALYZER("ae handle preProcess failed ret %d", ret);
        mPreResShared = NULL;
        return ret;
    } else if (ret == XCAM_RETURN_BYPASS) {
        LOGW_ANALYZER("%s:%d bypass !", __func__, __LINE__);
        mPreResShared = NULL;
        return ret;
    }

    if (xAecStats) {
        ae_pre_int->aecStatsBuf = &xAecStats->aec_stats;
        ae_pre_int->af_prior = xAecStats->af_prior;
    } else {
        ae_pre_int->aecStatsBuf = NULL;
        ae_pre_int->af_prior = false;
    }

    if (algoId == 0) {
        AeInstanceConfig_t* pAeInstConfig           = (AeInstanceConfig_t*)mAlgoCtx;

        mGetAfdResMutex.lock();
        pAeInstConfig->aecCfg->AfdRes = mAfdRes;
        mGetAfdResMutex.unlock();
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    if (des->pre_process) {
#ifdef DISABLE_HANDLE_ATTRIB
        mCfgMutex.lock();
#endif
        ret = des->pre_process(mPreInParam, (RkAiqAlgoResCom*)(&mPreResShared->result));
#ifdef DISABLE_HANDLE_ATTRIB
        mCfgMutex.unlock();
#endif
        if (ret < 0) {
            LOGE_ANALYZER("ae handle pre_process failed ret %d", ret);
            mPreResShared = NULL;
            return ret;
        } else if (ret == XCAM_RETURN_BYPASS) {
            LOGW_ANALYZER("%s:%d bypass !", __func__, __LINE__);
            mPreResShared = NULL;
            return ret;
        }
    }

    if (mPostShared) {
        if (mAiqCore->mAlogsComSharedParams.init) {
            RkAiqCore::RkAiqAlgosGroupShared_t* grpShared = nullptr;
            for (uint32_t i = 0; i < RK_AIQ_CORE_ANALYZE_MAX; i++) {
                uint64_t grpMask = grpId2GrpMask(i);
                if (!mAiqCore->getGroupSharedParams(grpMask, grpShared)) {
                    if (grpShared)
                        grpShared->res_comb.ae_pre_res = convert_to_XCamVideoBuffer(mPreResShared);
                }
            }
        } else {
            mPreResShared->set_sequence(shared->frameId);
            RkAiqCoreVdBufMsg msg(XCAM_MESSAGE_AE_PRE_RES_OK, shared->frameId, mPreResShared);
            mAiqCore->post_message(msg);
        }
    }

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAeHandleInt::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAe* ae_proc_int        = (RkAiqAlgoProcAe*)mProcInParam;
    RkAiqAlgoProcResAe* ae_proc_res_int        = (RkAiqAlgoProcResAe*)mProcOutParam;
    bool postMsg = true;

    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    RkAiqAecStats* xAecStats = nullptr;
    if (shared->aecStatsBuf) {
        xAecStats = shared->aecStatsBuf;
        if (!xAecStats) LOGE_AEC("aec stats is null");
    } else {
        LOGW_AEC("the xcamvideobuffer of aec stats is null");
    }
    if ((!xAecStats || !xAecStats->aec_stats_valid) && !sharedCom->init) {
        LOGW("no aec stats, ignore!");
        mPreResShared = NULL;
        return XCAM_RETURN_BYPASS;
    }

    int algoId = this->getAlgoId();
    if (algoId == 0) {
        AeInstanceConfig_t* pAeInstConfig           = (AeInstanceConfig_t*)mAlgoCtx;

        mLockAebyAfMutex.lock();
        pAeInstConfig->lockaebyaf = lockaebyaf;
        mLockAebyAfMutex.unlock();
    }

    ae_proc_res_int->new_ae_exp = &shared->fullParams->mExposureParams->data()->result.new_ae_exp;
    ae_proc_res_int->ae_proc_res_rk = &shared->fullParams->mExposureParams->data()->result.ae_proc_res_rk;
    ae_proc_res_int->exp_i2c_params = &shared->fullParams->mExposureParams->data()->result.exp_i2c_params;
    ae_proc_res_int->ae_meas = &shared->fullParams->mAecParams->data()->result;
    ae_proc_res_int->hist_meas = &shared->fullParams->mHistParams->data()->result;

    ret = RkAiqHandle::processing();
    if (ret < 0) {
        LOGE_ANALYZER("ae handle processing failed ret %d", ret);
        mPreResShared = NULL;
        return ret;
    } else if (ret == XCAM_RETURN_BYPASS) {
        LOGW_ANALYZER("%s:%d bypass !", __func__, __LINE__);
        mPreResShared = NULL;
        return ret;
    }

    // TODO config common ae processing params
    if (xAecStats)
        ae_proc_int->aecStatsBuf = &xAecStats->aec_stats;
    else
        ae_proc_int->aecStatsBuf = NULL;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    if (des->processing) {
#ifdef DISABLE_HANDLE_ATTRIB
        mCfgMutex.lock();
#endif
        ret = des->processing(mProcInParam, (RkAiqAlgoResCom*)(mProcOutParam));
#ifdef DISABLE_HANDLE_ATTRIB
        mCfgMutex.unlock();
#endif
        if (ret < 0) {
            LOGE_ANALYZER("ae algo processing failed ret %d", ret);
            mPreResShared = NULL;
            return ret;
        } else if (ret == XCAM_RETURN_BYPASS) {
            LOGW_ANALYZER("%s:%d bypass !", __func__, __LINE__);
            mPreResShared = NULL;
            return ret;
        }
    }

    if (mAiqCore->mAlogsComSharedParams.init) {
        RkAiqCore::RkAiqAlgosGroupShared_t* measGroupshared = nullptr;
        if (mAiqCore->getGroupSharedParams(RK_AIQ_CORE_ANALYZE_MEAS, measGroupshared) !=
                XCAM_RETURN_NO_ERROR) {
            LOGW("get the shared of meas failed");
        }
        if (measGroupshared) {
            measGroupshared->frameId                 = shared->frameId;
        }

        /* Transfer the initial exposure to other algorithm modules */
        for (auto type = RK_AIQ_CORE_ANALYZE_MEAS; type < RK_AIQ_CORE_ANALYZE_MAX; \
                type = (rk_aiq_core_analyze_type_e)(type + 1)) {
            uint64_t grpMask = grpId2GrpMask(type);
            if (!mAiqCore->getGroupSharedParams(grpMask, shared)) {
                if (shared) {
                    shared->preExp = *ae_proc_res_int->new_ae_exp;
                    shared->curExp = *ae_proc_res_int->new_ae_exp;
                    shared->nxtExp = *ae_proc_res_int->new_ae_exp;
                }
            }

        }
    } else {
#if 0 // no algo need ae proc result now
        if (mPostShared) {
            SmartPtr<BufferProxy> msg_data = new BufferProxy(mProcResShared);
            msg_data->set_sequence(shared->frameId);
            SmartPtr<XCamMessage> msg =
                new RkAiqCoreVdBufMsg(XCAM_MESSAGE_AE_PROC_RES_OK, shared->frameId, msg_data);
            mAiqCore->post_message(msg);
        }
#endif
    }

#if RKAIQ_HAVE_AF
    if (mAf_handle) {
        int algo_id                      = (*mAf_handle)->getAlgoId();
        if (algo_id == 0) {
            RkAiqAfHandleInt* af_algo = dynamic_cast<RkAiqAfHandleInt*>(mAf_handle->ptr());

            af_algo->setAeStable(ae_proc_res_int->ae_proc_res_rk->IsConverged);
        }
    }
#endif

    RkAiqAlgoProcResAeShared_t aeProcResShared;

    aeProcResShared.IsConverged = ae_proc_res_int->ae_proc_res_rk->IsConverged;
    aeProcResShared.IsEnvChanged = ae_proc_res_int->ae_proc_res_rk->IsEnvChanged;
    aeProcResShared.IsAutoAfd =  ae_proc_res_int->ae_proc_res_rk->IsAutoAfd;
    aeProcResShared.LongFrmMode =  ae_proc_res_int->ae_proc_res_rk->LongFrmMode;

#if RKAIQ_HAVE_AFD_V1 || RKAIQ_HAVE_AFD_V2
    if (mAfd_handle) {
        RkAiqAfdHandleInt* afd_algo = dynamic_cast<RkAiqAfdHandleInt*>(mAfd_handle->ptr());

        afd_algo->setAeProcRes(&aeProcResShared);
    }
#endif

    if (mAmerge_handle) {
        RkAiqAmergeHandleInt* amerge_algo = dynamic_cast<RkAiqAmergeHandleInt*>(mAmerge_handle->ptr());
        amerge_algo->setAeProcRes(&aeProcResShared);
    }

    if (mAdrc_handle) {
#if USE_NEWSTRUCT
        RkAiqDrcHandleInt* Drc_algo = dynamic_cast<RkAiqDrcHandleInt*>(mAdrc_handle->ptr());
        Drc_algo->setAeProcRes(&aeProcResShared);
#else
        RkAiqAdrcHandleInt* adrc_algo = dynamic_cast<RkAiqAdrcHandleInt*>(mAdrc_handle->ptr());
        adrc_algo->setAeProcRes(&aeProcResShared);
#endif
    }

    RkAiqResourceTranslator* translator = dynamic_cast<RkAiqResourceTranslator*>(mAiqCore->getTranslator());
    if (ae_proc_res_int->stats_cfg_to_trans.isUpdate) {
        translator->setAeAlgoStatsCfg(&ae_proc_res_int->stats_cfg_to_trans);
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAeHandleInt::postProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    RkAiqAecStats* xAecStats = nullptr;
    if (shared->aecStatsBuf) {
        xAecStats = shared->aecStatsBuf;
        if (!xAecStats) LOGE_AEC("aec stats is null");
    } else {
        LOGW_AEC("the xcamvideobuffer of aec stats is null");
    }
    if ((!xAecStats || !xAecStats->aec_stats_valid) && !sharedCom->init) {
        LOGW("no aec stats, ignore!");
        mPreResShared = NULL;
        return XCAM_RETURN_BYPASS;
    }
    ret = RkAiqHandle::postProcess();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "ae handle postProcess failed");
        mPreResShared = NULL;
        return ret;
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    if (des->post_process)
        ret                       = des->post_process(mPostInParam, mPostOutParam);
    RKAIQCORE_CHECK_RET(ret, "ae algo post_process failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAeHandleInt::genIspResult(RkAiqFullParams* params, RkAiqFullParams* cur_params) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (mIsMulRun && this->getAlgoId() == 0) {
        // do nothing for rkawb if custom algo running with rk algo
        return ret;
    }

    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    RkAiqAlgoProcResAe* ae_proc                 = (RkAiqAlgoProcResAe*)mProcOutParam;
    RkAiqAlgoPostResAe* ae_post                 = (RkAiqAlgoPostResAe*)mPostOutParam;

    rk_aiq_isp_aec_params_t* aec_param   = params->mAecParams->data().ptr();
    rk_aiq_isp_hist_params_t* hist_param = params->mHistParams->data().ptr();
    rk_aiq_exposure_params_wrapper_t* exp_param = params->mExposureParams->data().ptr();
    rk_aiq_iris_params_wrapper_t* iris_param    = params->mIrisParams->data().ptr();

    int algo_id = this->getAlgoId();

    if (sharedCom->init) {
        aec_param->frame_id  = 0;
        hist_param->frame_id = 0;
        exp_param->frame_id  = 0;
    } else {
        aec_param->frame_id  = shared->frameId;
        hist_param->frame_id = shared->frameId;
        exp_param->frame_id  = shared->frameId;
    }

    // exposure
    exp_param->result.algo_id      = 0;//algo_id;
    cur_params->mExposureParams = params->mExposureParams;

    // iris
    if (algo_id == 0) {
        RkAiqAlgoPostResAe* ae_post_rk = (RkAiqAlgoPostResAe*)ae_post;
        // HDC iris control
        iris_param->DCIris.update = ae_post_rk->ae_post_res_rk.DCIris.update;
        iris_param->DCIris.pwmDuty = ae_post_rk->ae_post_res_rk.DCIris.pwmDuty;
        // HDC iris control
        iris_param->HDCIris.update = ae_post_rk->ae_post_res_rk.HDCIris.update;
        iris_param->HDCIris.target = ae_post_rk->ae_post_res_rk.HDCIris.target;
        ae_post_rk->ae_post_res_rk.HDCIris.adc = iris_param->HDCIris.adc;
        ae_post_rk->ae_post_res_rk.HDCIris.zoomPos = iris_param->HDCIris.zoomPos;
        cur_params->mIrisParams = params->mIrisParams;
    }

    // meas
    if (ae_proc->ae_meas->ae_meas_update) {
        mMeasSyncFlag = shared->frameId;
        aec_param->sync_flag = mMeasSyncFlag;
        // copy from algo result
        // set as the latest result
        cur_params->mAecParams = params->mAecParams;
        aec_param->is_update = true;
        ae_proc->ae_meas->ae_meas_update = false;
        LOGD_AEC("[%d] meas params from algo", mMeasSyncFlag);
    } else if (mMeasSyncFlag != aec_param->sync_flag) {
        aec_param->sync_flag = mMeasSyncFlag;
        // copy from latest result
        if (cur_params->mAecParams.ptr()) {
            aec_param->result = cur_params->mAecParams->data()->result;
            aec_param->is_update = true;
        } else {
            LOGE_AEC("no latest meas params !");
            aec_param->is_update = false;
        }
        LOGD_AEC("[%d] meas params from latest [%d]", shared->frameId, mMeasSyncFlag);
    } else {
        // do nothing, result in buf needn't update
        aec_param->is_update = false;
        LOGD_AEC("[%d] meas params needn't update", shared->frameId);
    }

    // hist
    if (ae_proc->hist_meas->hist_meas_update) {
        mHistSyncFlag = shared->frameId;
        hist_param->sync_flag = mHistSyncFlag;
        // copy from algo result
        // set as the latest result
        cur_params->mHistParams = params->mHistParams;
        hist_param->is_update = true;
        ae_proc->hist_meas->hist_meas_update = false;
        LOGD_AEC("[%d] hist params from algo", mHistSyncFlag);
    } else if (mHistSyncFlag != hist_param->sync_flag) {
        hist_param->sync_flag = mHistSyncFlag;
        // copy from latest result
        if (cur_params->mHistParams.ptr()) {
            hist_param->result = cur_params->mHistParams->data()->result;
            hist_param->is_update = true;
        } else {
            LOGE_AEC("no latest hist params !");
            hist_param->is_update = false;
        }
        LOGD_AEC("[%d] hist params from latest [%d]", shared->frameId, mHistSyncFlag);
    } else {
        // do nothing, result in buf needn't update
        hist_param->is_update = false;
        LOGD_AEC("[%d] hist params needn't update", shared->frameId);
    }

    mPreResShared = NULL;

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

}  // namespace RkCam
