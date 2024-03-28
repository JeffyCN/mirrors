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

XCamReturn RkAiqCamGroupAeHandleInt::updateConfig(bool needSync) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (needSync) mCfgMutex.lock();
    // if something changed

    if (updateExpSwAttrV2) {
        mCurExpSwAttrV2   = mNewExpSwAttrV2;
        rk_aiq_uapi_ae_setExpSwAttr(mAlgoCtx, &mCurExpSwAttrV2, true, false);
        updateExpSwAttrV2 = false;
        sendSignal(mCurExpSwAttrV2.sync.sync_mode);
    }

    if (updateLinExpAttrV2) {
        mCurLinExpAttrV2   = mNewLinExpAttrV2;
        rk_aiq_uapi_ae_setLinExpAttr(mAlgoCtx, &mCurLinExpAttrV2, true, false);
        updateLinExpAttrV2 = false;
        sendSignal(mCurLinExpAttrV2.sync.sync_mode);
    }

    if (updateHdrExpAttrV2) {
        mCurHdrExpAttrV2   = mNewHdrExpAttrV2;
        rk_aiq_uapi_ae_setHdrExpAttr(mAlgoCtx, &mCurHdrExpAttrV2, true, false);
        updateHdrExpAttrV2 = false;
        sendSignal(mCurHdrExpAttrV2.sync.sync_mode);
    }

    if (updateLinAeRouteAttr) {
        mCurLinAeRouteAttr   = mNewLinAeRouteAttr;
        rk_aiq_uapi_ae_setLinAeRouteAttr(mAlgoCtx, &mCurLinAeRouteAttr, true, false);
        updateLinAeRouteAttr = false;
        sendSignal(mCurLinAeRouteAttr.sync.sync_mode);
    }
    if (updateHdrAeRouteAttr) {
        mCurHdrAeRouteAttr   = mNewHdrAeRouteAttr;
        rk_aiq_uapi_ae_setHdrAeRouteAttr(mAlgoCtx, &mCurHdrAeRouteAttr, true, false);
        updateHdrAeRouteAttr = false;
        sendSignal(mCurHdrAeRouteAttr.sync.sync_mode);
    }

    if (updateSyncTestAttr) {
        mCurAecSyncTestAttr = mNewAecSyncTestAttr;
        rk_aiq_uapi_ae_setSyncTest(mAlgoCtx, &mCurAecSyncTestAttr, true, false);
        updateSyncTestAttr  = false;
        sendSignal(mCurAecSyncTestAttr.sync.sync_mode);
    }

    if (updateAecStatsCfg) {
        mCurAecStatsCfg = mNewAecStatsCfg;
        rk_aiq_uapi_ae_setAecStatsCfg(mAlgoCtx, &mCurAecStatsCfg, true, false);
        updateAecStatsCfg  = false;
        sendSignal(mCurAecStatsCfg.sync.sync_mode);
    }

    if (needSync) mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCamGroupAeHandleInt::setExpSwAttr(Uapi_ExpSwAttrV2_t ExpSwAttrV2) {
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

XCamReturn RkAiqCamGroupAeHandleInt::getExpSwAttr(Uapi_ExpSwAttrV2_t* pExpSwAttrV2) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (pExpSwAttrV2->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_ae_getExpSwAttr(mAlgoCtx, pExpSwAttrV2, true);
        pExpSwAttrV2->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateExpSwAttrV2) {
            memcpy(pExpSwAttrV2, &mNewExpSwAttrV2, sizeof(mNewExpSwAttrV2));
            pExpSwAttrV2->sync.done = false;
        } else {
            rk_aiq_uapi_ae_getExpSwAttr(mAlgoCtx, pExpSwAttrV2, true);
            pExpSwAttrV2->sync.sync_mode = mNewExpSwAttrV2.sync.sync_mode;
            pExpSwAttrV2->sync.done = true;
        }
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCamGroupAeHandleInt::setLinExpAttr(Uapi_LinExpAttrV2_t LinExpAttrV2) {
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

XCamReturn RkAiqCamGroupAeHandleInt::getLinExpAttr(Uapi_LinExpAttrV2_t* pLinExpAttrV2) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;


    if (pLinExpAttrV2->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_ae_getLinExpAttr(mAlgoCtx, pLinExpAttrV2, true);
        pLinExpAttrV2->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateLinExpAttrV2) {
            memcpy(pLinExpAttrV2, &mNewLinExpAttrV2, sizeof(mNewLinExpAttrV2));
            pLinExpAttrV2->sync.done = false;
        } else {
            rk_aiq_uapi_ae_getLinExpAttr(mAlgoCtx, pLinExpAttrV2, true);
            pLinExpAttrV2->sync.sync_mode = mNewLinExpAttrV2.sync.sync_mode;
            pLinExpAttrV2->sync.done      = true;
        }
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCamGroupAeHandleInt::setHdrExpAttr(Uapi_HdrExpAttrV2_t HdrExpAttrV2) {
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

XCamReturn RkAiqCamGroupAeHandleInt::getHdrExpAttr(Uapi_HdrExpAttrV2_t* pHdrExpAttrV2) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (pHdrExpAttrV2->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_ae_getHdrExpAttr(mAlgoCtx, pHdrExpAttrV2, true);
        pHdrExpAttrV2->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateHdrExpAttrV2) {
            memcpy(pHdrExpAttrV2, &mNewHdrExpAttrV2, sizeof(mNewHdrExpAttrV2));
            pHdrExpAttrV2->sync.done = false;
        } else {
            rk_aiq_uapi_ae_getHdrExpAttr(mAlgoCtx, pHdrExpAttrV2, true);
            pHdrExpAttrV2->sync.sync_mode = mNewHdrExpAttrV2.sync.sync_mode;
            pHdrExpAttrV2->sync.done      = true;
        }
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
XCamReturn RkAiqCamGroupAeHandleInt::setLinAeRouteAttr(Uapi_LinAeRouteAttr_t LinAeRouteAttr) {
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

XCamReturn RkAiqCamGroupAeHandleInt::getLinAeRouteAttr(Uapi_LinAeRouteAttr_t* pLinAeRouteAttr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (pLinAeRouteAttr->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_ae_getLinAeRouteAttr(mAlgoCtx, pLinAeRouteAttr, true);
        pLinAeRouteAttr->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateLinAeRouteAttr) {
            memcpy(pLinAeRouteAttr, &mNewLinAeRouteAttr, sizeof(mNewLinAeRouteAttr));
            pLinAeRouteAttr->sync.done = false;
        } else {
            rk_aiq_uapi_ae_getLinAeRouteAttr(mAlgoCtx, pLinAeRouteAttr, true);
            pLinAeRouteAttr->sync.sync_mode = mNewLinAeRouteAttr.sync.sync_mode;
            pLinAeRouteAttr->sync.done      = true;
        }
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
XCamReturn RkAiqCamGroupAeHandleInt::setHdrAeRouteAttr(Uapi_HdrAeRouteAttr_t HdrAeRouteAttr) {
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

XCamReturn RkAiqCamGroupAeHandleInt::getHdrAeRouteAttr(Uapi_HdrAeRouteAttr_t* pHdrAeRouteAttr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (pHdrAeRouteAttr->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_ae_getHdrAeRouteAttr(mAlgoCtx, pHdrAeRouteAttr, true);
        pHdrAeRouteAttr->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateHdrAeRouteAttr) {
            memcpy(pHdrAeRouteAttr, &mNewHdrAeRouteAttr, sizeof(mNewHdrAeRouteAttr));
            pHdrAeRouteAttr->sync.done = false;
        } else {
            rk_aiq_uapi_ae_getHdrAeRouteAttr(mAlgoCtx, pHdrAeRouteAttr, true);
            pHdrAeRouteAttr->sync.sync_mode = mNewHdrAeRouteAttr.sync.sync_mode;
            pHdrAeRouteAttr->sync.done      = true;
        }
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
XCamReturn RkAiqCamGroupAeHandleInt::setSyncTestAttr(Uapi_AecSyncTest_t SyncTestAttr) {
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

XCamReturn RkAiqCamGroupAeHandleInt::getSyncTestAttr(Uapi_AecSyncTest_t* pSyncTestAttr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (pSyncTestAttr->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_ae_getSyncTest(mAlgoCtx, pSyncTestAttr, true);
        pSyncTestAttr->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateSyncTestAttr) {
            memcpy(pSyncTestAttr, &mNewAecSyncTestAttr, sizeof(mNewAecSyncTestAttr));
            pSyncTestAttr->sync.done = false;
        } else {
            rk_aiq_uapi_ae_getSyncTest(mAlgoCtx, pSyncTestAttr, true);
            pSyncTestAttr->sync.sync_mode = mNewAecSyncTestAttr.sync.sync_mode;
            pSyncTestAttr->sync.done      = true;
        }
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
XCamReturn RkAiqCamGroupAeHandleInt::queryExpInfo(Uapi_ExpQueryInfo_t* pExpQueryInfo) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_ae_queryExpInfo(mAlgoCtx, pExpQueryInfo, true);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCamGroupAeHandleInt::setAecStatsCfg(Uapi_AecStatsCfg_t AecStatsCfg) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    bool isChanged = false;
    if (AecStatsCfg.sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC && \
            memcmp(&mNewAecStatsCfg, &AecStatsCfg, sizeof(AecStatsCfg)))
        isChanged = true;
    else if (AecStatsCfg.sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC && \
             memcmp(&mCurAecStatsCfg, &AecStatsCfg, sizeof(AecStatsCfg)))
        isChanged = true;

    // if something changed
    if (isChanged) {
        mNewAecStatsCfg = AecStatsCfg;
        updateAecStatsCfg  = true;
        waitSignal(AecStatsCfg.sync.sync_mode);
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCamGroupAeHandleInt::getAecStatsCfg(Uapi_AecStatsCfg_t* pAecStatsCfg) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (pAecStatsCfg->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_ae_getAecStatsCfg(mAlgoCtx, pAecStatsCfg, true);
        pAecStatsCfg->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateAecStatsCfg) {
            memcpy(pAecStatsCfg, &mNewAecStatsCfg, sizeof(mNewAecStatsCfg));
            pAecStatsCfg->sync.done = false;
        } else {
            rk_aiq_uapi_ae_getAecStatsCfg(mAlgoCtx, pAecStatsCfg, true);
            pAecStatsCfg->sync.sync_mode = mNewAecStatsCfg.sync.sync_mode;
            pAecStatsCfg->sync.done      = true;
        }
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}


}  // namespace RkCam
