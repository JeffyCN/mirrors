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
#include "RkAiqCacHandler.h"
#include "aiq_core.h"
#include "RkAiqGlobalParamsManager_c.h"
#include "newStruct/cac/include/cac_algo_api.h"
#include "newStruct/algo_common.h"

static void _handlerCac_init(AiqAlgoHandler_t* pHdl) {
    ENTER_ANALYZER_FUNCTION();

    AiqAlgoHandler_deinit(pHdl);
    pHdl->mConfig       = (RkAiqAlgoCom*)(aiq_mallocz(sizeof(RkAiqAlgoCom)));
    pHdl->mProcInParam  = (RkAiqAlgoCom*)(aiq_mallocz(sizeof(RkAiqAlgoProcCac)));
    pHdl->mProcOutParam = (RkAiqAlgoResCom*)(aiq_mallocz(sizeof(RkAiqAlgoProcResCac)));

    pHdl->mResultType = RESULT_TYPE_CAC_PARAM;
    pHdl->mResultSize = sizeof(rk_aiq_isp_cac_params_t);

    EXIT_ANALYZER_FUNCTION();
}

static XCamReturn _handlerCac_prepare(AiqAlgoHandler_t* pAlgoHandler) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = AiqAlgoHandler_prepare(pAlgoHandler);
    RKAIQCORE_CHECK_RET(ret, "cac handle prepare failed");
	AiqAlgoHandlerCac_t* pCacHdl = (AiqAlgoHandlerCac_t*)pAlgoHandler;

    aiqMutex_lock(&pAlgoHandler->mCfgMutex);
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)pAlgoHandler->mDes;
    ret                       = des->prepare(pAlgoHandler->mConfig);
    aiqMutex_unlock(&pAlgoHandler->mCfgMutex);
    RKAIQCORE_CHECK_RET(ret, "cac algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _handlerCac_processing(AiqAlgoHandler_t* pAlgoHandler) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

	AiqAlgoHandlerCac_t* pCacHdl = (AiqAlgoHandlerCac_t*)pAlgoHandler;
    RkAiqAlgosGroupShared_t* shared =
        (RkAiqAlgosGroupShared_t*)(pAlgoHandler->mAlogsGroupSharedParams);
    RkAiqAlgosComShared_t* sharedCom = &pAlgoHandler->mAiqCore->mAlogsComSharedParams;
    RkAiqAlgoProcCac* cac_proc_int = (RkAiqAlgoProcCac*)pAlgoHandler->mProcInParam;
    RkAiqAlgoProcResCac* cac_proc_res_int = (RkAiqAlgoProcResCac*)pAlgoHandler->mProcOutParam;

    RKAiqAecExpInfo_t* aeCurExp = &shared->curExp;
    cac_proc_int->iso = 50;
    if (aeCurExp != NULL) {
        if (sharedCom->working_mode == (int)RK_AIQ_WORKING_MODE_NORMAL) {
            cac_proc_int->iso = aeCurExp->LinearExp.exp_real_params.analog_gain * 50;
            LOGD_ACAC("%s:NORMAL:iso=%d,again=%f\n", __FUNCTION__, cac_proc_int->iso,
                      aeCurExp->LinearExp.exp_real_params.analog_gain);
        } else if (RK_AIQ_HDR_IS_HDR2(sharedCom->working_mode)) {
            cac_proc_int->iso = aeCurExp->HdrExp[1].exp_real_params.analog_gain * 50;
            LOGD_ACAC("%s:HDR2:iso=%d,again=%f ratio %f\n", __FUNCTION__, cac_proc_int->iso,
                      aeCurExp->HdrExp[1].exp_real_params.analog_gain);
        } else if (RK_AIQ_HDR_IS_HDR3(sharedCom->working_mode)) {
            cac_proc_int->iso = aeCurExp->HdrExp[2].exp_real_params.analog_gain * 50;
            LOGD_ACAC("%s:HDR3:iso=%d,again=%f\n", __FUNCTION__, cac_proc_int->iso,
                      aeCurExp->HdrExp[2].exp_real_params.analog_gain);
        }
    } else {
        cac_proc_int->iso = 50;
        LOGE_ACAC("%s: pAEPreRes is NULL, so use default instead \n", __FUNCTION__);
    }

    ret = AiqAlgoHandler_processing(pAlgoHandler);
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "cac handle processing failed");
    }

	aiq_params_base_t* pBase = shared->fullParams->pParamsArray[RESULT_TYPE_CAC_PARAM];
    if (!pBase) {
        LOGW_A3DLUT("no cac params buf !");
        return XCAM_RETURN_BYPASS;
    }
    cac_proc_res_int->cacRes = (rk_aiq_isp_cac_params_t*)pBase->_data;
    ret = AiqAlgoHandler_do_processing_common(pAlgoHandler);
    RKAIQCORE_CHECK_RET(ret, "cac algo processing failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

AiqAlgoHandler_t* AiqAlgoHandlerCac_constructor(RkAiqAlgoDesComm* des, AiqCore_t* aiqCore) {
    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)aiq_mallocz(sizeof(AiqAlgoHandlerCac_t));
    if (!pHdl)
		return NULL;
	AiqAlgoHandler_constructor(pHdl, des, aiqCore);
    pHdl->processing   = _handlerCac_processing;
    pHdl->genIspResult = AiqAlgoHandler_genIspResult_common;
    pHdl->prepare      = _handlerCac_prepare;
    pHdl->init         = _handlerCac_init;
	return pHdl;
}

#if 0
XCamReturn AiqAlgoHandlerCac_setAttrib(AiqAlgoHandlerCac_t* pCacHdl, cac_api_attrib_t* attr)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pCacHdl;
    ret = algo_cac_SetAttrib(pHdl->mAlgoCtx, attr);
    aiqMutex_lock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
	return ret;
}

XCamReturn AiqAlgoHandlerCac_getAttrib(AiqAlgoHandlerCac_t* pCacHdl, cac_api_attrib_t* attr)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pCacHdl;
    ret = algo_cac_GetAttrib(pHdl->mAlgoCtx, attr);
    aiqMutex_lock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
	return ret;
}

XCamReturn AiqAlgoHandlerCac_queryStatus(AiqAlgoHandlerCac_t* pCacHdl, cac_status_t* status)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pCacHdl;

	aiq_params_base_t* pCurBase = pHdl->mAiqCore->mAiqCurParams->pParamsArray[RESULT_TYPE_CAC_PARAM];

    if (pCurBase) {
        rk_aiq_isp_cac_params_t* cac_param = (rk_aiq_isp_cac_params_t*)pCurBase->_data;
        if (cac_param) {
            status->stMan = *cac_param;
            status->en = pCurBase->en;
            status->bypass = pCurBase->bypass;
            status->opMode = pCacHdl->mOpMode;
        } else {
            ret = XCAM_RETURN_ERROR_FAILED;
            LOGE_ACAC("have no status info !");
        }
    } else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE_ACAC("have no status info !");
    }

    aiqMutex_lock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
	return ret;
}
#endif
