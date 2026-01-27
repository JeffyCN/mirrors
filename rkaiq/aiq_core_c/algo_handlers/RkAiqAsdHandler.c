/*
 * Copyright (c) 2024 Rockchip Eletronics Co., Ltd.
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

#include "aiq_base.h"
#include "RkAiqAsdHandler.h"
#include "aiq_core.h"
#include "RkAiqGlobalParamsManager_c.h"
#include "rk_aiq_algo_types.h"

static void _handlerAsd_init(AiqAlgoHandler_t* pHdl) {
    ENTER_ANALYZER_FUNCTION();

    AiqAlgoHandler_deinit(pHdl);
    pHdl->mConfig       = (RkAiqAlgoCom*)(aiq_mallocz(sizeof(RkAiqAlgoCom)));
    pHdl->mProcInParam  = (RkAiqAlgoCom*)(aiq_mallocz(sizeof(RkAiqAlgoProcAsd)));
    pHdl->mProcOutParam = (RkAiqAlgoResCom*)(aiq_mallocz(sizeof(RkAiqAlgoProcResAsd)));

    pHdl->mResultType = RESULT_TYPE_ASD_PARAM;
    pHdl->mResultSize = sizeof(asd_param_t);

    EXIT_ANALYZER_FUNCTION();
}

static XCamReturn _handlerAsd_prepare(AiqAlgoHandler_t* pAlgoHandler) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqAlgosComShared_t* sharedCom = &pAlgoHandler->mAiqCore->mAlogsComSharedParams;
    AiqAlgoHandlerAsd_t* pHdlAsd     = (AiqAlgoHandlerAsd_t*)pAlgoHandler;

    ret = AiqAlgoHandler_prepare(pAlgoHandler);
    RKAIQCORE_CHECK_RET(ret, "asd handle prepare failed");

    GlobalParamsManager_lockAlgoParam(pAlgoHandler->mAiqCore->mGlobalParamsManger, pAlgoHandler->mResultType);
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)pAlgoHandler->mDes;
    ret                       = des->prepare(pAlgoHandler->mConfig);
    GlobalParamsManager_unlockAlgoParam(pAlgoHandler->mAiqCore->mGlobalParamsManger, pAlgoHandler->mResultType);
    RKAIQCORE_CHECK_RET(ret, "asd algo prepare failed");

    pHdlAsd->isp_acq_width  = sharedCom->snsDes.isp_acq_width;
    pHdlAsd->isp_acq_height = sharedCom->snsDes.isp_acq_height;
    LOGD_ASD("%s: isp_acq_width %d, isp_acq_height %d", __func__, pHdlAsd->isp_acq_width, pHdlAsd->isp_acq_height);

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _handlerAsd_processing(AiqAlgoHandler_t* pAlgoHandler) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcResAsd* asd_proc_res_int = (RkAiqAlgoProcResAsd*)pAlgoHandler->mProcOutParam;
    RkAiqAlgoProcAsd* asd_proc_param = (RkAiqAlgoProcAsd*)pAlgoHandler->mProcInParam;
    RkAiqAlgosGroupShared_t* shared =
        (RkAiqAlgosGroupShared_t*)(pAlgoHandler->mAlogsGroupSharedParams);
    RkAiqAlgosComShared_t* sharedCom = &pAlgoHandler->mAiqCore->mAlogsComSharedParams;

    ret = AiqAlgoHandler_processing(pAlgoHandler);
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "asd handle processing failed");
    }

    AiqAlgoHandlerAsd_t* pasdHandler     = (AiqAlgoHandlerAsd_t*)pAlgoHandler;

    RkAiqAlgoResCom* proc_res = pAlgoHandler->mProcOutParam;

    GlobalParamsManager_t * globalParamsManager = pAlgoHandler->mAiqCore->mGlobalParamsManger;
    GlobalParamsManager_lockAlgoParam(globalParamsManager, pAlgoHandler->mResultType);

    pAlgoHandler->mProcInParam->u.proc.is_attrib_update =
        GlobalParamsManager_getAndClearAlgoParamUpdateFlagLocked(globalParamsManager, pAlgoHandler->mResultType);

    pAlgoHandler->mIsUpdateGrpAttr =
        pAlgoHandler->mProcInParam->u.proc.is_attrib_update;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)pAlgoHandler->mDes;
    ret = des->processing(pAlgoHandler->mProcInParam, pAlgoHandler->mProcOutParam);

    GlobalParamsManager_unlockAlgoParam(globalParamsManager, pAlgoHandler->mResultType);

    sharedCom->selfaceinfo = asd_proc_res_int->selfaceinfo;

    RKAIQCORE_CHECK_RET(ret, "asd algo processing failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

static XCamReturn _handlerAsd_genIspResult(AiqAlgoHandler_t* pAlgoHandler, AiqFullParams_t* params,
                                           AiqFullParams_t* cur_params) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    return ret;
}

AiqAlgoHandler_t* AiqAlgoHandlerAsd_constructor(RkAiqAlgoDesComm* des, AiqCore_t* aiqCore) {
    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)aiq_mallocz(sizeof(AiqAlgoHandlerAsd_t));
    if (!pHdl)
        return NULL;
    AiqAlgoHandler_constructor(pHdl, des, aiqCore);

    pHdl->processing   = _handlerAsd_processing;
    pHdl->genIspResult = _handlerAsd_genIspResult;
    pHdl->prepare      = _handlerAsd_prepare;
    pHdl->init         = _handlerAsd_init;
    return pHdl;
}

XCamReturn
AiqAlgoHandlerAsd_setFaceInfo(AiqAlgoHandlerAsd_t* pHdlAsd, rk_aiq_face_info_t *face)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (face->count > RK_AIQ_FACEINFO_MAX_NUM) {
        LOGE_ASD("input face num(%d) is too big", face->count);
        return XCAM_RETURN_ERROR_PARAM;
    }

    for (int i = 0; i < face->count; i++) {
        if (face->roi[i].left < ASD_FACE_MIN_X) {
            LOGD_ASD("input face roi(%d) left is abnormal, change from %d to %d",
                i, face->roi[i].left, ASD_FACE_MIN_X);
            face->roi[i].left = ASD_FACE_MIN_X;
        }
        if (face->roi[i].left > ASD_FACE_MAX_X) {
            LOGD_ASD("input face roi(%d) left is abnormal, change from %d to %d",
                i, face->roi[i].left, ASD_FACE_MAX_X);
            face->roi[i].left = ASD_FACE_MAX_X;
        }
        if (face->roi[i].top < ASD_FACE_MIN_Y) {
            LOGD_ASD("input face roi(%d) top is abnormal, change from %d to %d",
                i, face->roi[i].top, ASD_FACE_MIN_Y);
            face->roi[i].top = ASD_FACE_MIN_Y;
        }
        if (face->roi[i].top > ASD_FACE_MAX_Y) {
            LOGD_ASD("input face roi(%d) top is abnormal, change from %d to %d",
                i, face->roi[i].top, ASD_FACE_MAX_Y);
            face->roi[i].top = ASD_FACE_MAX_Y;
        }
        if (face->roi[i].left + face->roi[i].width > ASD_FACE_MAX_WIDTH) {
            LOGD_ASD("input face roi(%d) width is abnormal, change from %d to %d",
                i, face->roi[i].width, ASD_FACE_MAX_WIDTH - face->roi[i].left);
            face->roi[i].width = ASD_FACE_MAX_WIDTH - face->roi[i].left;
        }
        if (face->roi[i].top + face->roi[i].height > ASD_FACE_MAX_HEIGHT) {
            LOGD_ASD("input face roi(%d) height is abnormal, change from %d to %d",
                i, face->roi[i].height, ASD_FACE_MAX_HEIGHT - face->roi[i].top);
            face->roi[i].height = ASD_FACE_MAX_HEIGHT - face->roi[i].top;
        }

        if ((face->roi[i].left < ASD_FACE_MIN_X || face->roi[i].left > ASD_FACE_MAX_X) ||
            (face->roi[i].top < ASD_FACE_MIN_Y || face->roi[i].top > ASD_FACE_MAX_Y) ||
            (face->roi[i].width < ASD_FACE_MIN_WIDTH || face->roi[i].width > ASD_FACE_MAX_WIDTH) ||
            (face->roi[i].height < ASD_FACE_MIN_HEIGHT || face->roi[i].height > ASD_FACE_MAX_HEIGHT) ||
            (face->roi[i].left + face->roi[i].width > ASD_FACE_MAX_WIDTH) ||
            (face->roi[i].top + face->roi[i].height > ASD_FACE_MAX_HEIGHT)) {
            LOGE_ASD("input face roi(%d) is abnormal, roi %d, %d, %d, %d",
                i, face->roi[i].left, face->roi[i].top, face->roi[i].width, face->roi[i].height);
            return XCAM_RETURN_ERROR_PARAM;
        }
    }

    aiqMutex_lock(&pHdlAsd->_base.mCfgMutex);
    for (int i = 0; i < face->count; i++) {
        LOGD_ASD("input faceroi[%d]: id %d, confidence %d, roi %d, %d, %d, %d",
            i, face->roi[i].id, face->roi[i].confidence, face->roi[i].left, face->roi[i].top, face->roi[i].width, face->roi[i].height);
    }

    for (int i = 0; i < face->count; i++) {
        face->roi[i].left   = (face->roi[i].left - ASD_FACE_MIN_X) * pHdlAsd->isp_acq_width / ASD_FACE_MAX_WIDTH;
        face->roi[i].top    = (face->roi[i].top - ASD_FACE_MIN_Y) * pHdlAsd->isp_acq_height / ASD_FACE_MAX_HEIGHT;
        face->roi[i].width  = face->roi[i].width * pHdlAsd->isp_acq_width / ASD_FACE_MAX_WIDTH;
        face->roi[i].height = face->roi[i].height * pHdlAsd->isp_acq_height / ASD_FACE_MAX_HEIGHT;

        LOGD_ASD("set faceroi[%d]: id %d, confidence %d, roi %d, %d, %d, %d",
            i, face->roi[i].id, face->roi[i].confidence, face->roi[i].left, face->roi[i].top, face->roi[i].width, face->roi[i].height);
    }

    ret = algo_asd_setFaceInfo(pHdlAsd->_base.mAlgoCtx, face);
    aiqMutex_unlock(&pHdlAsd->_base.mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
AiqAlgoHandlerAsd_getSelFaceInfo(AiqAlgoHandlerAsd_t* pHdlAsd, rk_aiq_face_roi_t* selface)
{
    RkAiqAlgosComShared_t* sharedCom = &pHdlAsd->_base.mAiqCore->mAlogsComSharedParams;

    aiqMutex_lock(&pHdlAsd->_base.mCfgMutex);
    *selface = sharedCom->selfaceinfo;
    if (pHdlAsd->isp_acq_width != 0 && pHdlAsd->isp_acq_height != 0) {
        selface->left   = selface->left * ASD_FACE_MAX_WIDTH / pHdlAsd->isp_acq_width + ASD_FACE_MIN_X;
        selface->top    = selface->top * ASD_FACE_MAX_HEIGHT / pHdlAsd->isp_acq_height + ASD_FACE_MIN_Y;
        selface->width  = selface->width * ASD_FACE_MAX_WIDTH / pHdlAsd->isp_acq_width;
        selface->height = selface->height * ASD_FACE_MAX_HEIGHT / pHdlAsd->isp_acq_height;
    } else {
        memset(selface, 0, sizeof(*selface));
    }
    LOGD_ASD("%s: %d, %d, %d, %d", __func__, selface->left, selface->top, selface->width, selface->height);
    aiqMutex_unlock(&pHdlAsd->_base.mCfgMutex);

    return XCAM_RETURN_NO_ERROR;
}

#if 0
XCamReturn AiqAlgoHandlerAsd_setAttrib(AiqAlgoHandlerAsd_t* pHdlAsd, asd_api_attrib_t* attr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    aiqMutex_lock(&pHdlAsd->_base.mCfgMutex);
    ret = algo_asd_SetAttrib(pHdlAsd->_base.mAlgoCtx, attr);
    aiqMutex_unlock(&pHdlAsd->_base.mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerAsd_getAttrib(AiqAlgoHandlerAsd_t* pHdlAsd, asd_api_attrib_t* attr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    aiqMutex_lock(&pHdlAsd->_base.mCfgMutex);

    ret = algo_asd_GetAttrib(pHdlAsd->_base.mAlgoCtx, attr);

    aiqMutex_unlock(&pHdlAsd->_base.mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerAsd_queryStatus(AiqAlgoHandlerAsd_t* pHdlAsd, asd_status_t* status) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    aiqMutex_lock(&pHdlAsd->_base.mCfgMutex);

    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pHdlAsd;
    aiq_params_base_t* pCurBase =
        pHdl->mAiqCore->mAiqCurParams->pParamsArray[RESULT_TYPE_ASD_PARAM];

    if (pCurBase) {
        rk_aiq_isp_asd_params_t* asd_param = (rk_aiq_isp_asd_params_t*)pCurBase->_data;
        if (asd_param) {
            status->stMan  = *asd_param;
            status->en     = pCurBase->en;
            status->bypass = pCurBase->bypass;
            status->opMode = pHdlAsd->_base.mOpMode;
        } else {
            ret = XCAM_RETURN_ERROR_FAILED;
            LOGE_ASD("have no status info !");
        }
    } else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE_ASD("have no status info !");
    }

    aiqMutex_unlock(&pHdlAsd->_base.mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}
#endif
