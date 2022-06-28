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

#include "RkAiqCore.h"

namespace RkCam {

DEFINE_HANDLE_REGISTER_TYPE(RkAiqAeHandleInt);

void RkAiqAeHandleInt::init() {
    ENTER_ANALYZER_FUNCTION();

    RkAiqHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAe());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAe());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAe());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAe());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAe());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAe());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAe());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqAeHandleInt::updateConfig(bool needSync) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

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

    // once any params are changed, run reconfig to convert aecCfg to paectx
    AeInstanceConfig_t* pAeInstConfig = (AeInstanceConfig_t*)mAlgoCtx;
    AeConfig_t pAecCfg                = pAeInstConfig->aecCfg;
    pAecCfg->IsReconfig |= updateAttr;
    updateAttr = 0;
    if (needSync) mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAeHandleInt::setExpSwAttr(Uapi_ExpSwAttr_t ExpSwAttr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
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
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAeHandleInt::getExpSwAttr(Uapi_ExpSwAttr_t* pExpSwAttr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    Uapi_ExpSwAttrV2_t ExpSwAttrV2;

    rk_aiq_uapi_ae_getExpSwAttr(mAlgoCtx, &mCurExpSwAttrV2, false);
    rk_aiq_uapi_ae_convExpSwAttr_v2Tov1(&mCurExpSwAttrV2, pExpSwAttr);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAeHandleInt::setExpSwAttr(Uapi_ExpSwAttrV2_t ExpSwAttrV2) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

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
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAeHandleInt::getExpSwAttr(Uapi_ExpSwAttrV2_t* pExpSwAttrV2) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

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

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAeHandleInt::setLinExpAttr(Uapi_LinExpAttr_t LinExpAttr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
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
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAeHandleInt::getLinExpAttr(Uapi_LinExpAttr_t* pLinExpAttr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_ae_getLinExpAttr(mAlgoCtx, &mCurLinExpAttrV2, false);
    rk_aiq_uapi_ae_convLinExpAttr_v2Tov1(&mCurLinExpAttrV2, pLinExpAttr);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAeHandleInt::setLinExpAttr(Uapi_LinExpAttrV2_t LinExpAttrV2) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
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
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAeHandleInt::getLinExpAttr(Uapi_LinExpAttrV2_t* pLinExpAttrV2) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;


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

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAeHandleInt::setHdrExpAttr(Uapi_HdrExpAttr_t HdrExpAttr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
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
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAeHandleInt::getHdrExpAttr(Uapi_HdrExpAttr_t* pHdrExpAttr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_ae_getHdrExpAttr(mAlgoCtx, &mCurHdrExpAttrV2, false);
    rk_aiq_uapi_ae_convHdrExpAttr_v2Tov1(&mCurHdrExpAttrV2, pHdrExpAttr);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAeHandleInt::setHdrExpAttr(Uapi_HdrExpAttrV2_t HdrExpAttrV2) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

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
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAeHandleInt::getHdrExpAttr(Uapi_HdrExpAttrV2_t* pHdrExpAttrV2) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

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

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAeHandleInt::setLinAeRouteAttr(Uapi_LinAeRouteAttr_t LinAeRouteAttr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

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
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAeHandleInt::getLinAeRouteAttr(Uapi_LinAeRouteAttr_t* pLinAeRouteAttr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

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

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAeHandleInt::setHdrAeRouteAttr(Uapi_HdrAeRouteAttr_t HdrAeRouteAttr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

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
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAeHandleInt::getHdrAeRouteAttr(Uapi_HdrAeRouteAttr_t* pHdrAeRouteAttr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

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

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAeHandleInt::setIrisAttr(Uapi_IrisAttrV2_t IrisAttr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

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

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAeHandleInt::getIrisAttr(Uapi_IrisAttrV2_t* pIrisAttr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

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

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAeHandleInt::setSyncTestAttr(Uapi_AecSyncTest_t SyncTestAttr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

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

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAeHandleInt::getSyncTestAttr(Uapi_AecSyncTest_t* pSyncTestAttr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

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

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAeHandleInt::setExpWinAttr(Uapi_ExpWin_t ExpWinAttr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

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

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAeHandleInt::getExpWinAttr(Uapi_ExpWin_t* pExpWinAttr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

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

XCamReturn RkAiqAeHandleInt::prepare() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "ae handle prepare failed");

    RkAiqAlgoConfigAe* ae_config_int = (RkAiqAlgoConfigAe*)mConfig;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
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
    bool postMsg = true;

    if (mDes->id == 0) {
        mPreResShared = new RkAiqAlgoPreResAeIntShared();
        if (!mPreResShared.ptr()) {
            LOGE("new ae mPreOutParam failed, bypass!");
            return XCAM_RETURN_BYPASS;
        }
    }

    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::preProcess();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "ae handle preProcess failed");
    }

    RkAiqAecStats* xAecStats = nullptr;
    if (shared->aecStatsBuf) {
        xAecStats = (RkAiqAecStats*)shared->aecStatsBuf->map(shared->aecStatsBuf);
        if (!xAecStats) LOGE_AEC("aec stats is null");
    } else {
        LOGW_AEC("the xcamvideobuffer of aec stats is null");
    }
    if ((!xAecStats || !xAecStats->aec_stats_valid) && !sharedCom->init) {
        LOGW("no aec stats, ignore!");
        return XCAM_RETURN_BYPASS;
    }

    ae_pre_int->aecStatsBuf = shared->aecStatsBuf;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    if (des->pre_process)
        ret = des->pre_process(mPreInParam, (RkAiqAlgoResCom*)(&mPreResShared->result));
    RKAIQCORE_CHECK_RET(ret, "ae algo pre_process failed");

    if (mPostShared && !mAiqCore->mAlogsComSharedParams.init) {
        SmartPtr<BufferProxy> msg_data = new BufferProxy(mPreResShared);
        msg_data->set_sequence(shared->frameId);
        SmartPtr<XCamMessage> msg =
            new RkAiqCoreVdBufMsg(XCAM_MESSAGE_AE_PRE_RES_OK, shared->frameId, msg_data);
        mAiqCore->post_message(msg);
    }

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAeHandleInt::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAe* ae_proc_int        = (RkAiqAlgoProcAe*)mProcInParam;
    bool postMsg = true;

    if (mDes->id == 0) {
        mProcResShared = new RkAiqAlgoProcResAeIntShared();
        if (!mProcResShared.ptr()) {
            LOGE("new ae mProcOutParam failed, bypass!");
            return XCAM_RETURN_BYPASS;
        }
    }

    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    AeInstanceConfig_t* pAeInstConfig           = (AeInstanceConfig_t*)mAlgoCtx;

    mLockAebyAfMutex.lock();
    pAeInstConfig->lockaebyaf = lockaebyaf;
    mLockAebyAfMutex.unlock();

    ret = RkAiqHandle::processing();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "ae handle processing failed");
    }

    RkAiqAecStats* xAecStats = nullptr;
    if (shared->aecStatsBuf) {
        xAecStats = (RkAiqAecStats*)shared->aecStatsBuf->map(shared->aecStatsBuf);
        if (!xAecStats) LOGE_AEC("aec stats is null");
    } else {
        LOGW_AEC("the xcamvideobuffer of aec stats is null");
    }
    if ((!xAecStats || !xAecStats->aec_stats_valid) && !sharedCom->init) {
        LOGW("no aec stats, ignore!");
        mProcResShared.release();
        return XCAM_RETURN_BYPASS;
    }

    // TODO config common ae processing params
    ae_proc_int->aecStatsBuf = shared->aecStatsBuf;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    if (des->processing)
        ret = des->processing(mProcInParam, (RkAiqAlgoResCom*)(&mProcResShared->result));
    RKAIQCORE_CHECK_RET(ret, "ae algo processing failed");

    if (mAiqCore->mAlogsComSharedParams.init) {
        RkAiqCore::RkAiqAlgosGroupShared_t* measGroupshared = nullptr;
        if (mAiqCore->getGroupSharedParams(RK_AIQ_CORE_ANALYZE_MEAS, measGroupshared) !=
                XCAM_RETURN_NO_ERROR)
            LOGW("get the shared of meas failed");
        if (measGroupshared) {
            measGroupshared->frameId                 = shared->frameId;
        }
    } else {
        if (mPostShared) {
            SmartPtr<BufferProxy> msg_data = new BufferProxy(mProcResShared);
            msg_data->set_sequence(shared->frameId);
            SmartPtr<XCamMessage> msg =
                new RkAiqCoreVdBufMsg(XCAM_MESSAGE_AE_PROC_RES_OK, shared->frameId, msg_data);
            mAiqCore->post_message(msg);
        }
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAeHandleInt::postProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPostAe* ae_post_int        = (RkAiqAlgoPostAe*)mPostInParam;
    RkAiqAlgoPostResAe* ae_post_res_int = (RkAiqAlgoPostResAe*)mPostOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::postProcess();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "ae handle postProcess failed");
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

    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    if (!mProcResShared.ptr())
        return XCAM_RETURN_NO_ERROR;
    RkAiqAlgoProcResAe* ae_proc                 = &mProcResShared->result;
    RkAiqAlgoPostResAe* ae_post                 = (RkAiqAlgoPostResAe*)mPostOutParam;
    if (!ae_proc) {
        LOGD_ANALYZER("no ae_proc result");
        return XCAM_RETURN_NO_ERROR;
    }

    if (!ae_post) {
        LOGD_ANALYZER("no ae_post result");
        return XCAM_RETURN_NO_ERROR;
    }

    rk_aiq_isp_aec_params_v20_t* aec_param   = params->mAecParams->data().ptr();
    rk_aiq_isp_hist_params_v20_t* hist_param = params->mHistParams->data().ptr();

    SmartPtr<rk_aiq_exposure_params_wrapper_t> exp_param = params->mExposureParams->data();
    SmartPtr<rk_aiq_iris_params_wrapper_t> iris_param    = params->mIrisParams->data();

    int algo_id = this->getAlgoId();

    exp_param->aecExpInfo.LinearExp = ae_proc->new_ae_exp.LinearExp;
    memcpy(exp_param->aecExpInfo.HdrExp, ae_proc->new_ae_exp.HdrExp,
           sizeof(ae_proc->new_ae_exp.HdrExp));
    exp_param->aecExpInfo.exp_i2c_params = ae_proc->new_ae_exp.exp_i2c_params;
    exp_param->aecExpInfo.frame_length_lines   = ae_proc->new_ae_exp.frame_length_lines;
    exp_param->aecExpInfo.line_length_pixels   = ae_proc->new_ae_exp.line_length_pixels;
    exp_param->aecExpInfo.pixel_clock_freq_mhz = ae_proc->new_ae_exp.pixel_clock_freq_mhz;
    exp_param->aecExpInfo.Iris.PIris           = ae_proc->new_ae_exp.Iris.PIris;

    // TODO Merge
    // iris_param->IrisType = ae_proc->new_ae_exp.Iris.IrisType;
    iris_param->PIris.step   = ae_proc->new_ae_exp.Iris.PIris.step;
    iris_param->PIris.update = ae_proc->new_ae_exp.Iris.PIris.update;

#if 0
    isp_param->aec_meas = ae_proc->ae_meas;
    isp_param->hist_meas = ae_proc->hist_meas;
#else
    if (sharedCom->init) {
        aec_param->frame_id  = 0;
        hist_param->frame_id = 0;
        exp_param->frame_id  = 0;
    } else {
        aec_param->frame_id  = shared->frameId;
        hist_param->frame_id = shared->frameId;
        exp_param->frame_id  = shared->frameId;
    }

    aec_param->result  = ae_proc->ae_meas;
    hist_param->result = ae_proc->hist_meas;
#endif

    RkAiqAlgoProcResAe* ae_rk = (RkAiqAlgoProcResAe*)ae_proc;
    memcpy(exp_param->exp_tbl, ae_rk->ae_proc_res_rk.exp_set_tbl, sizeof(exp_param->exp_tbl));
    exp_param->exp_tbl_size = ae_rk->ae_proc_res_rk.exp_set_cnt;
    exp_param->algo_id      = algo_id;

    if (algo_id == 0) {
        RkAiqAlgoPostResAe* ae_post_rk = (RkAiqAlgoPostResAe*)ae_post;
        iris_param->DCIris.update         = ae_post_rk->ae_post_res_rk.DCIris.update;
        iris_param->DCIris.pwmDuty        = ae_post_rk->ae_post_res_rk.DCIris.pwmDuty;
    }

    cur_params->mExposureParams = params->mExposureParams;
    cur_params->mAecParams      = params->mAecParams;
    cur_params->mHistParams     = params->mHistParams;

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

};  // namespace RkCam
