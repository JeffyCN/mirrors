/*
 * RkAiqCustomHandle.h
 *
 *  Copyright (c) 2019 Rockchip Corporation
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
 *
 */

#include "RkAiqCustomAeHandle.h"

namespace RkCam {

//#define BYPASS_CUSTOM_AE

XCamReturn RkAiqCustomAeHandle::updateConfig(bool needSync) {
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqCustomAeHandle::lock() {
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqCustomAeHandle::unlock() {
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqCustomAeHandle::setExpSwAttr(Uapi_ExpSwAttr_t ExpSwAttr) {
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqCustomAeHandle::getExpSwAttr(Uapi_ExpSwAttr_t* pExpSwAttr) {
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqCustomAeHandle::setExpSwAttr(Uapi_ExpSwAttrV2_t ExpSwAttrV2) {
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqCustomAeHandle::getExpSwAttr(Uapi_ExpSwAttrV2_t* pExpSwAttrV2) {
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqCustomAeHandle::setLinExpAttr(Uapi_LinExpAttr_t LinExpAttr) {
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqCustomAeHandle::getLinExpAttr(Uapi_LinExpAttr_t* pLinExpAttr) {
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqCustomAeHandle::setLinExpAttr(Uapi_LinExpAttrV2_t LinExpAttrV2) {
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqCustomAeHandle::getLinExpAttr(Uapi_LinExpAttrV2_t* pLinExpAttrV2) {
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqCustomAeHandle::setHdrExpAttr(Uapi_HdrExpAttr_t HdrExpAttr) {
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqCustomAeHandle::getHdrExpAttr(Uapi_HdrExpAttr_t* pHdrExpAttr) {
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqCustomAeHandle::setHdrExpAttr(Uapi_HdrExpAttrV2_t HdrExpAttrV2) {
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqCustomAeHandle::getHdrExpAttr(Uapi_HdrExpAttrV2_t* pHdrExpAttrV2) {
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqCustomAeHandle::setLinAeRouteAttr(Uapi_LinAeRouteAttr_t LinAeRouteAttr) {
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqCustomAeHandle::getLinAeRouteAttr(Uapi_LinAeRouteAttr_t* pLinAeRouteAttr) {
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqCustomAeHandle::setHdrAeRouteAttr(Uapi_HdrAeRouteAttr_t HdrAeRouteAttr) {
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqCustomAeHandle::getHdrAeRouteAttr(Uapi_HdrAeRouteAttr_t* pHdrAeRouteAttr) {
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqCustomAeHandle::setIrisAttr(Uapi_IrisAttrV2_t IrisAttr) {
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqCustomAeHandle::getIrisAttr(Uapi_IrisAttrV2_t* pIrisAttr) {
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqCustomAeHandle::setSyncTestAttr(Uapi_AecSyncTest_t SyncTestAttr) {
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqCustomAeHandle::getSyncTestAttr(Uapi_AecSyncTest_t* pSyncTestAttr) {
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqCustomAeHandle::setExpWinAttr(Uapi_ExpWin_t ExpWinAttr) {
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqCustomAeHandle::getExpWinAttr(Uapi_ExpWin_t* pExpWinAttr) {
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqCustomAeHandle::setAecStatsCfg(Uapi_AecStatsCfg_t AecStatsCfg) {
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqCustomAeHandle::getAecStatsCfg(Uapi_AecStatsCfg_t* pAecStatsCfg) {
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqCustomAeHandle::queryExpInfo(Uapi_ExpQueryInfo_t* pExpQueryInfo) {
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqCustomAeHandle::setLockAeForAf(bool lock_ae) {
    return XCAM_RETURN_NO_ERROR;
}

void
RkAiqCustomAeHandle::init() {
    if (mIsMulRun) {
        // reuse parent's resources, contains:
        // mConfig, mProcInParam, mProcOutParam
        RkAiqAeHandleInt* parent =  dynamic_cast<RkAiqAeHandleInt*>(mParentHdl);
        if (!parent)
            LOGE_AEC("no parent ae handler in multiple handler mode !");
        mConfig      = parent->mConfig;
        mPreInParam   = parent->mPreInParam;
        mProcInParam = parent->mProcInParam;
        mProcOutParam = parent->mProcOutParam;
        mPostInParam  = parent->mPostInParam;
        mPostOutParam = parent->mPostOutParam;
#if RKAIQ_HAVE_AF
        mAf_handle = parent->mAf_handle;
#endif
#if RKAIQ_HAVE_AFD_V1 || RKAIQ_HAVE_AFD_V2
        mAfd_handle = parent->mAfd_handle;
#endif
        mAmerge_handle = parent->mAmerge_handle;
        mAdrc_handle = parent->mAdrc_handle;
    } else {
        RkAiqAeHandleInt::init();
    }
}

void
RkAiqCustomAeHandle::deInit() {
    if (mIsMulRun) {
        mConfig      = NULL;
        mPreInParam   = NULL;
        mProcInParam = NULL;
        mProcOutParam = NULL;
        mPostInParam  = NULL;
        mPostOutParam = NULL;
#if RKAIQ_HAVE_AF
        mAf_handle = NULL;
#endif
#if RKAIQ_HAVE_AFD_V1 || RKAIQ_HAVE_AFD_V2
        mAfd_handle = NULL;
#endif
        mAmerge_handle = NULL;
        mAdrc_handle = NULL;
    } else {
        RkAiqAeHandleInt::deInit();
    }
}

XCamReturn RkAiqCustomAeHandle::preProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#ifdef BYPASS_CUSTOM_AE
    if (0) {
#else
    if (mIsMulRun) {
#endif
        // use parent's mem, so child algo results can overwrite parents results
        RkAiqAeHandleInt* parent =  dynamic_cast<RkAiqAeHandleInt*>(mParentHdl);
        if (!parent)
            LOGE_AEC("no parent ae handler in multiple handler mode !");
        mPreResShared =  parent->mPreResShared;
    } else {
        mPreResShared = new RkAiqAlgoPreResAeIntShared();
        if (!mPreResShared.ptr()) {
            LOGE("new ae mPreOutParam failed, bypass!");
            return XCAM_RETURN_BYPASS;
        }
    }

    return RkAiqAeHandleInt::preProcess();

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqCustomAeHandle::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#ifdef BYPASS_CUSTOM_AE
    if (0) {
#else
    if (mIsMulRun) {
#endif
    } else {
    }

    return RkAiqAeHandleInt::processing();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCustomAeHandle::postProcess() {
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqCustomAeHandle::genIspResult(RkAiqFullParams* params, RkAiqFullParams* cur_params) {
#ifdef BYPASS_CUSTOM_AE
    return XCAM_RETURN_NO_ERROR;
#else
    return RkAiqAeHandleInt::genIspResult(params, cur_params);
#endif
}

}  // namespace RkCam
