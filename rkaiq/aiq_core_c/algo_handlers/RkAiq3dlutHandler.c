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
#include "RkAiq3dlutHandler.h"
#include "aiq_core.h"
#include "RkAiqGlobalParamsManager_c.h"
#include "newStruct/algo_common.h"
#include "newStruct/3dlut/include/3dlut_algo_api.h"

static void _handler3dlut_init(AiqAlgoHandler_t* pHdl) {
    ENTER_ANALYZER_FUNCTION();

    AiqAlgoHandler_deinit(pHdl);
    pHdl->mConfig       = (RkAiqAlgoCom*)(aiq_mallocz(sizeof(RkAiqAlgoCom)));
    pHdl->mProcInParam  = (RkAiqAlgoCom*)(aiq_mallocz(sizeof(RkAiqAlgoProcLut3d)));
    pHdl->mProcOutParam = (RkAiqAlgoResCom*)(aiq_mallocz(sizeof(RkAiqAlgoResCom)));

    pHdl->mResultType = RESULT_TYPE_LUT3D_PARAM;
    pHdl->mResultSize = sizeof(lut3d_param_t);

    EXIT_ANALYZER_FUNCTION();
}

static XCamReturn _handler3dlut_prepare(AiqAlgoHandler_t* pAlgoHandler) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = AiqAlgoHandler_prepare(pAlgoHandler);
    RKAIQCORE_CHECK_RET(ret, "lut3d handle prepare failed");

    GlobalParamsManager_lockAlgoParam(pAlgoHandler->mAiqCore->mGlobalParamsManger, pAlgoHandler->mResultType);
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)pAlgoHandler->mDes;
    ret                       = des->prepare(pAlgoHandler->mConfig);
    GlobalParamsManager_unlockAlgoParam(pAlgoHandler->mAiqCore->mGlobalParamsManger, pAlgoHandler->mResultType);
    RKAIQCORE_CHECK_RET(ret, "lut3d algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _handler3dlut_processing(AiqAlgoHandler_t* pAlgoHandler) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoResCom* proc_res = pAlgoHandler->mProcOutParam;
    //RkAiqAlgoProcRes3dlut* lut3d_proc_res_int = (RkAiqAlgoProcRes3dlut*)pAlgoHandler->mProcOutParam;
    RkAiqAlgosGroupShared_t* shared =
        (RkAiqAlgosGroupShared_t*)(pAlgoHandler->mAlogsGroupSharedParams);
    RkAiqAlgosComShared_t* sharedCom = &pAlgoHandler->mAiqCore->mAlogsComSharedParams;

    ret = AiqAlgoHandler_processing(pAlgoHandler);
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "lut3d handle processing failed");
    }

    RkAiqAlgoProcLut3d* proc_int        = (RkAiqAlgoProcLut3d*)pAlgoHandler->mProcInParam;
    RKAiqAecExpInfo_t* pCurExp = &shared->curExp;
    RkAiqAlgoProcResAwbShared_t* awb_res = NULL;
    AlgoRstShared_t* awb_proc_res = shared->res_comb.awb_proc_res_c;
    if (awb_proc_res) {
        awb_res = (RkAiqAlgoProcResAwbShared_t*)awb_proc_res->_data;
    }
    if (!AiqCore_isGroupAlgo(pAlgoHandler->mAiqCore, pAlgoHandler->mDes->type))
        get_illu_estm_info(&proc_int->illu_info, awb_res, pCurExp, sharedCom->working_mode);

    ret = AiqAlgoHandler_do_processing_common(pAlgoHandler);

    RKAIQCORE_CHECK_RET(ret, "adebayer algo processing failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

AiqAlgoHandler_t* AiqAlgoHandler3dlut_constructor(RkAiqAlgoDesComm* des, AiqCore_t* aiqCore) {
    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)aiq_mallocz(sizeof(AiqAlgoHandler3dlut_t));
    if (!pHdl)
		return NULL;
	AiqAlgoHandler_constructor(pHdl, des, aiqCore);

    pHdl->processing   = _handler3dlut_processing;
    pHdl->genIspResult = AiqAlgoHandler_genIspResult_common;
    pHdl->prepare      = _handler3dlut_prepare;
    pHdl->init         = _handler3dlut_init;
	return pHdl;
}

XCamReturn AiqAlgoHandler3dlut_queryalut3dStatus(AiqAlgoHandler3dlut_t* pHdl3dlut, alut3d_status_t* status) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    aiqMutex_lock(&pHdl3dlut->mCfgMutex);

    ret = algo_lut3d_queryalut3dStatus(pHdl3dlut->mAlgoCtx, status);

    aiqMutex_unlock(&pHdl3dlut->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandler3dlut_setCalib(AiqAlgoHandler3dlut_t* pHdl3dlut, alut3d_lut3dCalib_t* calib) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    aiqMutex_lock(&pHdl3dlut->mCfgMutex);
    ret = algo_lut3d_SetCalib(pHdl3dlut->mAlgoCtx, calib);
    aiqMutex_unlock(&pHdl3dlut->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandler3dlut_getCalib(AiqAlgoHandler3dlut_t* pHdl3dlut, alut3d_lut3dCalib_t* calib) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    aiqMutex_lock(&pHdl3dlut->mCfgMutex);

    ret = algo_lut3d_GetCalib(pHdl3dlut->mAlgoCtx, calib);

    aiqMutex_unlock(&pHdl3dlut->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

#if 0
XCamReturn AiqAlgoHandler3dlut_queryStatus(AiqAlgoHandler3dlut_t* pHdl3dlut, lut3d_status_t* status) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    aiqMutex_lock(&pHdl3dlut->mCfgMutex);

    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pHdl3dlut;
    aiq_params_base_t* pCurBase =
        pHdl->mAiqCore->mAiqCurParams->pParamsArray[RESULT_TYPE_LUT3D_PARAM];

    if (pCurBase) {
        rk_aiq_isp_lut3d_params_t* lut3d_param = (rk_aiq_isp_lut3d_params_t*)pCurBase->_data;
        if (lut3d_param) {
            status->stMan  = *lut3d_param;
            status->en     = pCurBase->en;
            status->bypass = pCurBase->bypass;
            status->opMode = pHdl3dlut->mOpMode;
        } else {
            ret = XCAM_RETURN_ERROR_FAILED;
            LOGE_A3DLUT("have no status info !");
        }
    } else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE_A3DLUT("have no status info !");
    }

    aiqMutex_unlock(&pHdl3dlut->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}
#endif
