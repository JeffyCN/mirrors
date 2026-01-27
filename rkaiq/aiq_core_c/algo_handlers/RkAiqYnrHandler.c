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
#include "RkAiqYnrHandler.h"
#include "aiq_core.h"
#include "RkAiqGlobalParamsManager_c.h"

static void _handlerYnr_init(AiqAlgoHandler_t* pHdl) {
    ENTER_ANALYZER_FUNCTION();

    AiqAlgoHandler_deinit(pHdl);
    pHdl->mConfig       = (RkAiqAlgoCom*)(aiq_mallocz(sizeof(RkAiqAlgoCom)));
    pHdl->mProcInParam  = (RkAiqAlgoCom*)(aiq_mallocz(sizeof(RkAiqAlgoProcYnr)));
    pHdl->mProcOutParam = (RkAiqAlgoResCom*)(aiq_mallocz(sizeof(RkAiqAlgoProcResYnr)));

    pHdl->mResultType = RESULT_TYPE_YNR_PARAM;
    pHdl->mResultSize = sizeof(ynr_param_t);

    EXIT_ANALYZER_FUNCTION();
}

static XCamReturn _handlerYnr_prepare(AiqAlgoHandler_t* pAlgoHandler) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = AiqAlgoHandler_prepare(pAlgoHandler);
    RKAIQCORE_CHECK_RET(ret, "ynr handle prepare failed");

    GlobalParamsManager_lockAlgoParam(pAlgoHandler->mAiqCore->mGlobalParamsManger, pAlgoHandler->mResultType);
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)pAlgoHandler->mDes;
    ret                       = des->prepare(pAlgoHandler->mConfig);
    GlobalParamsManager_unlockAlgoParam(pAlgoHandler->mAiqCore->mGlobalParamsManger, pAlgoHandler->mResultType);
    RKAIQCORE_CHECK_RET(ret, "ynr algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _handlerYnr_processing(AiqAlgoHandler_t* pAlgoHandler) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgosGroupShared_t* shared =
        (RkAiqAlgosGroupShared_t*)(pAlgoHandler->mAlogsGroupSharedParams);
    RkAiqAlgosComShared_t* sharedCom = &pAlgoHandler->mAiqCore->mAlogsComSharedParams;

    ret = AiqAlgoHandler_processing(pAlgoHandler);
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "ynr handle processing failed");
    }

    RkAiqAlgoProcYnr* ynr_proc_param = (RkAiqAlgoProcYnr*)pAlgoHandler->mProcInParam;
    ynr_proc_param->blc_ob_predgain = 1.0;

    AiqAlgoHandler_do_processing_common(pAlgoHandler);

	AiqPoolItem_t* pItem =
		aiqPool_getFree(pAlgoHandler->mAiqCore->mProcResYnrSharedPool);
	RkAiqAlgoProcResYnr* ynrShared = NULL;
	if (pItem) {
		AlgoRstShared_t* pShared = (AlgoRstShared_t*)pItem->_pData;
		RkAiqAlgoProcResYnr* ynrShared = (RkAiqAlgoProcResYnr*)pShared->_data;
		pShared->frame_id = shared->frameId;
		pShared->type = XCAM_MESSAGE_YNR_PROC_RES_OK;
		ynrShared->ynrRes = pAlgoHandler->mProcOutParam->algoRes;

		AiqCoreMsg_t msg;
		msg.msg_id                    = XCAM_MESSAGE_YNR_PROC_RES_OK;
		msg.frame_id                  = shared->frameId;
		*(AlgoRstShared_t**)(msg.buf) = pShared;
		AiqCore_post_message(pAlgoHandler->mAiqCore, &msg);
		AIQ_REF_BASE_UNREF(&pShared->_ref_base);
	} else {
		LOGW_ANR("no ynr_proc_res buf !");
#if RKAIQ_HAVE_DUMPSYS
                pAlgoHandler->mAiqCore->mNoFreeBufCnt.ynrProcRes++;
#endif
        }
    RKAIQCORE_CHECK_RET(ret, "ynr algo processing failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

AiqAlgoHandler_t* AiqAlgoHandlerYnr_constructor(RkAiqAlgoDesComm* des, AiqCore_t* aiqCore) {
    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)aiq_mallocz(sizeof(AiqAlgoHandler_t));
    if (!pHdl)
		return NULL;
	AiqAlgoHandler_constructor(pHdl, des, aiqCore);
    pHdl->processing   = _handlerYnr_processing;
    pHdl->genIspResult = AiqAlgoHandler_genIspResult_common;
    pHdl->prepare      = _handlerYnr_prepare;
    pHdl->init         = _handlerYnr_init;
	return pHdl;
}
#if 0
XCamReturn AiqYnrHandler_setAttrib(AiqYnrHandler_t* pHdlYnr, ynr_api_attrib_t* attr)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    aiqMutex_lock(&pHdlYnr->mCfgMutex);
    ret = algo_ynr_SetAttrib(pHdlYnr->mAlgoCtx, attr);
    aiqMutex_unlock(&pHdlYnr->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
	return ret;
}

XCamReturn AiqYnrHandler_getAttrib(AiqYnrHandler_t* pHdlYnr, ynr_api_attrib_t* attr)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    aiqMutex_lock(&pHdlYnr->mCfgMutex);
    ret = algo_ynr_GetAttrib(pHdlYnr->mAlgoCtx, attr);
    aiqMutex_unlock(&pHdlYnr->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqYnrHandler_queryStatus(AiqYnrHandler_t* pHdlYnr, ynr_status_t* status)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    aiqMutex_lock(&pHdlYnr->mCfgMutex);

    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pHdlYnr;
    aiq_params_base_t* pCurBase =
        pHdl->mAiqCore->mAiqCurParams->pParamsArray[RESULT_TYPE_YNR_PARAM];

    if (pCurBase) {
        rk_aiq_isp_ynr_params_t* ynr_param = (rk_aiq_isp_ynr_params_t*)pCurBase->_data;
        if (ynr_param) {
            status->stMan  = *ynr_param;
            status->en     = pCurBase->en;
            status->bypass = pCurBase->bypass;
            status->opMode = pHdlYnr->mOpMode;
        } else {
            ret = XCAM_RETURN_ERROR_FAILED;
            LOGE_ANR("have no status info !");
        }
    } else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE_ANR("have no status info !");
    }

    aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;

}
#endif

XCamReturn AiqYnrHandler_setStrength(AiqYnrHandler_t* pHdlYnr, aynr_strength_t* strg)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    aiqMutex_lock(&pHdlYnr->mCfgMutex);
    ret = algo_ynr_SetStrength(pHdlYnr->mAlgoCtx, strg->percent, strg->en);
    aiqMutex_unlock(&pHdlYnr->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqYnrHandler_getStrength(AiqYnrHandler_t* pHdlYnr, aynr_strength_t* strg)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    aiqMutex_lock(&pHdlYnr->mCfgMutex);
    ret = algo_ynr_GetStrength(pHdlYnr->mAlgoCtx, &strg->percent, &strg->en);
    aiqMutex_unlock(&pHdlYnr->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}
