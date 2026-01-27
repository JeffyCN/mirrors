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
#include "RkAiqSharpHandler.h"
#include "aiq_core.h"
#include "RkAiqGlobalParamsManager_c.h"

static void _handlerSharp_init(AiqAlgoHandler_t* pHdl) {
    ENTER_ANALYZER_FUNCTION();

    AiqAlgoHandler_deinit(pHdl);
    pHdl->mConfig       = (RkAiqAlgoCom*)(aiq_mallocz(sizeof(RkAiqAlgoCom)));
    pHdl->mProcInParam  = (RkAiqAlgoCom*)(aiq_mallocz(sizeof(RkAiqAlgoCom)));
    pHdl->mProcOutParam = (RkAiqAlgoResCom*)(aiq_mallocz(sizeof(RkAiqAlgoResCom)));

    pHdl->mResultType = RESULT_TYPE_SHARPEN_PARAM;
    pHdl->mResultSize = sizeof(sharp_param_t);

    EXIT_ANALYZER_FUNCTION();
}

static XCamReturn _handlerSharp_prepare(AiqAlgoHandler_t* pAlgoHandler) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = AiqAlgoHandler_prepare(pAlgoHandler);
    RKAIQCORE_CHECK_RET(ret, "sharp handle prepare failed");

    GlobalParamsManager_lockAlgoParam(pAlgoHandler->mAiqCore->mGlobalParamsManger, pAlgoHandler->mResultType);
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)pAlgoHandler->mDes;
    ret                       = des->prepare(pAlgoHandler->mConfig);
    GlobalParamsManager_unlockAlgoParam(pAlgoHandler->mAiqCore->mGlobalParamsManger, pAlgoHandler->mResultType);
    RKAIQCORE_CHECK_RET(ret, "sharp algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _handlerSharp_processing(AiqAlgoHandler_t* pAlgoHandler) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = AiqAlgoHandler_processing(pAlgoHandler);
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "sharp handle processing failed");
    }

    RkAiqAlgosGroupShared_t* shared =
        (RkAiqAlgosGroupShared_t*)(pAlgoHandler->mAlogsGroupSharedParams);
    RkAiqAlgosComShared_t* sharedCom = &pAlgoHandler->mAiqCore->mAlogsComSharedParams;

    void *sharpRes = NULL;
    void *texEstRes = NULL;

    RkAiqAlgoResCom* proc_res = (RkAiqAlgoResCom*)pAlgoHandler->mProcOutParam;
    aiq_params_base_t* pBase = shared->fullParams->pParamsArray[RESULT_TYPE_SHARPEN_PARAM];
    if (!pBase) {
        LOGE("no sharp params buf !");
        return XCAM_RETURN_BYPASS;
    }
    sharpRes = (void *)pBase->_data;

#if defined(RKAIQ_HAVE_SHARP_V40) || defined(RKAIQ_HAVE_SHARP_V41)
    pBase = shared->fullParams->pParamsArray[RESULT_TYPE_TEXEST_PARAM];
    if (!pBase) {
        LOGE("no sharp params buf !");
        return XCAM_RETURN_BYPASS;
    }
    texEstRes = (void *)pBase->_data;
#endif

    GlobalParamsManager_t * globalParamsManager = pAlgoHandler->mAiqCore->mGlobalParamsManger;
    GlobalParamsManager_lockAlgoParam(globalParamsManager, pAlgoHandler->mResultType);
    if (globalParamsManager && !GlobalParamsManager_isFullManualMode(globalParamsManager) &&
        GlobalParamsManager_isManualLocked(globalParamsManager, pAlgoHandler->mResultType)) {
        rk_aiq_global_params_wrap_t wrap_param;
        proc_res->cfg_update = false;

        wrap_param.type           = RESULT_TYPE_SHARPEN_PARAM;
        wrap_param.man_param_size = sizeof(sharp_param_t);
        wrap_param.man_param_ptr  = sharpRes;
        wrap_param.aut_param_ptr  =  NULL;
        XCamReturn ret1           = GlobalParamsManager_getAndClearPendingLocked(globalParamsManager, &wrap_param);
        if (ret1 == XCAM_RETURN_NO_ERROR) {
            proc_res->en = wrap_param.en;
            proc_res->bypass = wrap_param.bypass;
            proc_res->cfg_update = true;
            pAlgoHandler->mOpMode = RK_AIQ_OP_MODE_MANUAL;
        }

#if defined(RKAIQ_HAVE_SHARP_V40) || defined(RKAIQ_HAVE_SHARP_V41)
        wrap_param.type           = RESULT_TYPE_TEXEST_PARAM;
        wrap_param.man_param_size = sizeof(texEst_param_t);
        wrap_param.man_param_ptr  = texEstRes;
        wrap_param.aut_param_ptr  =  NULL;
        ret1           = GlobalParamsManager_getAndClearPendingLocked(globalParamsManager, &wrap_param);
        if (ret1 == XCAM_RETURN_NO_ERROR) {
            proc_res->cfg_update = true;
        }
#endif

    } else {
        const RkAiqAlgoCom* inparams = pAlgoHandler->mProcInParam;
        int iso = inparams->u.proc.iso;
        bool isSharpUpdate = GlobalParamsManager_getAndClearAlgoParamUpdateFlagLocked(globalParamsManager, RESULT_TYPE_SHARPEN_PARAM);
        bool isTexEstUpdate = GlobalParamsManager_getAndClearAlgoParamUpdateFlagLocked(globalParamsManager, RESULT_TYPE_TEXEST_PARAM);

        bool isAttirbUpdate = isSharpUpdate || isTexEstUpdate;
        pAlgoHandler->mProcInParam->u.proc.is_attrib_update = isAttirbUpdate;

        pAlgoHandler->mIsUpdateGrpAttr =
            pAlgoHandler->mProcInParam->u.proc.is_attrib_update;

        if (!AiqCore_isGroupAlgo(pAlgoHandler->mAiqCore, pAlgoHandler->mDes->type)) {
            proc_res->algoRes = sharpRes;
            Asharp_processing(inparams, proc_res, iso);
#if defined(RKAIQ_HAVE_SHARP_V40) || defined(RKAIQ_HAVE_SHARP_V41)
            proc_res->algoRes = texEstRes;
            AtexEst_processing(inparams, proc_res, iso);
#endif
        } else {
            proc_res->cfg_update = false;
        }
        if (pAlgoHandler->mProcInParam->u.proc.is_attrib_update)
            pAlgoHandler->mOpMode = RK_AIQ_OP_MODE_AUTO;
    }
    GlobalParamsManager_unlockAlgoParam(globalParamsManager, pAlgoHandler->mResultType);

    RKAIQCORE_CHECK_RET(ret, "sharp algo processing failed");
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

static XCamReturn _handlerSharp_genIspResult(AiqAlgoHandler_t* pAlgoHandler, AiqFullParams_t* params,
                                           AiqFullParams_t* cur_params) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ret = AiqAlgoHandler_genIspResult_byType(pAlgoHandler, params, cur_params, RESULT_TYPE_SHARPEN_PARAM, sizeof(sharp_param_t));
#if defined(RKAIQ_HAVE_SHARP_V40) || defined(RKAIQ_HAVE_SHARP_V41)
    ret = AiqAlgoHandler_genIspResult_byType(pAlgoHandler, params, cur_params, RESULT_TYPE_TEXEST_PARAM, sizeof(texEst_param_t));
#endif
    return ret;
}

AiqAlgoHandler_t* AiqAlgoHandlerSharp_constructor(RkAiqAlgoDesComm* des, AiqCore_t* aiqCore) {
    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)aiq_mallocz(sizeof(AiqAlgoHandler_t));
    if (!pHdl)
		return NULL;
	AiqAlgoHandler_constructor(pHdl, des, aiqCore);
    pHdl->processing   = _handlerSharp_processing;
    pHdl->genIspResult = _handlerSharp_genIspResult;
    pHdl->prepare      = _handlerSharp_prepare;
    pHdl->init         = _handlerSharp_init;
	return pHdl;
}

#if 0
XCamReturn AiqSharpHandler_setAttrib(AiqSharpHandler_t* pHdlSharp, sharp_api_attrib_t* attr)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    aiqMutex_lock(&pHdlSharp->mCfgMutex);
    ret = algo_sharp_SetAttrib(pHdlSharp->mAlgoCtx, attr);
    aiqMutex_unlock(&pHdlSharp->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
	return ret;
}

XCamReturn AiqSharpHandler_getAttrib(AiqSharpHandler_t* pHdlSharp, sharp_api_attrib_t* attr)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    aiqMutex_lock(&pHdlSharp->mCfgMutex);
    ret = algo_sharp_GetAttrib(pHdlSharp->mAlgoCtx, attr);
    aiqMutex_unlock(&pHdlSharp->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqSharpHandler_queryStatus(AiqSharpHandler_t* pHdlSharp, sharp_status_t* status)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    aiqMutex_lock(&pHdlSharp->mCfgMutex);

    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pHdlSharp;
    aiq_params_base_t* pCurBase =
        pHdl->mAiqCore->mAiqCurParams->pParamsArray[RESULT_TYPE_SHARPEN_PARAM];

    if (pCurBase) {
        rk_aiq_isp_sharp_params_t* sharp_param = (rk_aiq_isp_sharp_params_t*)pCurBase->_data;
        if (sharp_param) {
            status->stMan  = *sharp_param;
            status->en     = pCurBase->en;
            status->bypass = pCurBase->bypass;
            status->opMode = pHdlSharp->mOpMode;
        } else {
            ret = XCAM_RETURN_ERROR_FAILED;
            LOGE_ASHARP("have no status info !");
        }
    } else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE_ASHARP("have no status info !");
    }

    aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}
#endif

XCamReturn AiqSharpHandler_setStrength(AiqSharpHandler_t* pHdlSharp, asharp_strength_t* strg)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    aiqMutex_lock(&pHdlSharp->mCfgMutex);
    ret = algo_sharp_SetStrength(pHdlSharp->mAlgoCtx, strg->percent, strg->en);
    aiqMutex_unlock(&pHdlSharp->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqSharpHandler_getStrength(AiqSharpHandler_t* pHdlSharp, asharp_strength_t* strg)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    aiqMutex_lock(&pHdlSharp->mCfgMutex);
    ret = algo_sharp_GetStrength(pHdlSharp->mAlgoCtx, &strg->percent, &strg->en);
    aiqMutex_unlock(&pHdlSharp->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqSharpHandler_queryStatus_texEst(AiqAlgoHandler_t* pHdl, void *buf)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    aiqMutex_lock(&pHdl->mCfgMutex);

#if defined(RKAIQ_HAVE_SHARP_V40) || defined(RKAIQ_HAVE_SHARP_V41)
    texEst_status_t *status = (texEst_status_t *)buf;
    aiq_params_base_t* pCurBase =
        pHdl->mAiqCore->mAiqCurParams->pParamsArray[RESULT_TYPE_TEXEST_PARAM];

    if (pCurBase) {
        memcpy(&status->stMan, pCurBase->_data, sizeof(texEst_param_t));
        status->en     = pCurBase->en;
        status->bypass = pCurBase->bypass;
        status->opMode = pHdl->mOpMode;
    } else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE("have no status info !");
    }
#endif

    aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

