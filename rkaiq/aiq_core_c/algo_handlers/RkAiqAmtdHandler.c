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
#include "RkAiqAmtdHandler.h"
#include "RkAiqAeHandler.h"
#include "aiq_core.h"
#include "RkAiqGlobalParamsManager_c.h"
#include "amtd/rk_aiq_uapi_amtd_int.h"

static void _handlerAmtd_deinit(AiqAlgoHandler_t* pHdl) {
    AiqAlgoHandler_deinit(pHdl);
    AiqAmtdHandler_t* pAmtdHdl = (AiqAmtdHandler_t*)pHdl;

    if (pAmtdHdl->mImuDataPool) {
        aiqPool_deinit(pAmtdHdl->mImuDataPool);
        pAmtdHdl->mImuDataPool = NULL;
    }
    if (pAmtdHdl->mImuDataList) {
        aiqList_deinit(pAmtdHdl->mImuDataList);
        pAmtdHdl->mImuDataList = NULL;
    }
    if (pAmtdHdl->mUsedImuList) {
        aiqList_deinit(pAmtdHdl->mUsedImuList);
        pAmtdHdl->mUsedImuList = NULL;
    }
}

static void _handlerAmtd_init(AiqAlgoHandler_t* pHdl) {
    ENTER_ANALYZER_FUNCTION();

    AiqAlgoHandler_deinit(pHdl);
    pHdl->mConfig       = (RkAiqAlgoCom*)(aiq_mallocz(sizeof(RkAiqAlgoConfigAmtd)));
    pHdl->mPreInParam   = (RkAiqAlgoCom*)(aiq_mallocz(sizeof(RkAiqAlgoPreAmtd)));
    pHdl->mPreOutParam  = (RkAiqAlgoResCom*)(aiq_mallocz(sizeof(RkAiqAlgoPreResAmtd)));
    pHdl->mProcInParam  = (RkAiqAlgoCom*)(aiq_mallocz(sizeof(RkAiqAlgoProcAmtd)));
    pHdl->mProcOutParam = (RkAiqAlgoResCom*)(aiq_mallocz(sizeof(RkAiqAlgoProcResAmtd)));

    AiqAmtdHandler_t* pAmtdHdl = (AiqAmtdHandler_t*)pHdl;
    AiqPoolConfig_t imuData;
    imuData._item_nums = 200;
    imuData._item_size = sizeof(AiqImuData_t);
    imuData._name = "ImuDataPool";
    pAmtdHdl->mImuDataPool = aiqPool_init(&imuData);

    AiqListConfig_t imuList;
    imuList._item_nums = 200;
    imuList._item_size = sizeof(AiqPoolItem_t*);
    imuList._name = "Amtd:mImuDataList";
    pAmtdHdl->mImuDataList = aiqList_init(&imuList);
    imuList._name = "Amtd:mUsedImuList";
    pAmtdHdl->mUsedImuList = aiqList_init(&imuList);

    EXIT_ANALYZER_FUNCTION();
}

static XCamReturn _handlerAmtd_prepare(AiqAlgoHandler_t* pAlgoHandler) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = AiqAlgoHandler_prepare(pAlgoHandler);
    RKAIQCORE_CHECK_RET(ret, "amtd handle prepare failed");

    RkAiqAlgoConfigAmtd* amtd_config = (RkAiqAlgoConfigAmtd*)pAlgoHandler->mConfig;
    RkAiqAlgosComShared_t* sharedCom = &pAlgoHandler->mAiqCore->mAlogsComSharedParams;


    GlobalParamsManager_lockAlgoParam(pAlgoHandler->mAiqCore->mGlobalParamsManger, pAlgoHandler->mResultType);
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)pAlgoHandler->mDes;
    ret                       = des->prepare(pAlgoHandler->mConfig);
    GlobalParamsManager_unlockAlgoParam(pAlgoHandler->mAiqCore->mGlobalParamsManger, pAlgoHandler->mResultType);
    RKAIQCORE_CHECK_RET(ret, "amtd algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _handlerAmtd_preProcess(AiqAlgoHandler_t* pAlgoHandler) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AiqAmtdHandler_t* pAmtdHdl = (AiqAmtdHandler_t*)pAlgoHandler;

    RkAiqAlgosGroupShared_t* shared =
        (RkAiqAlgosGroupShared_t*)(pAlgoHandler->mAlogsGroupSharedParams);
    RkAiqAlgosComShared_t* sharedCom = &pAlgoHandler->mAiqCore->mAlogsComSharedParams;

    RkAiqAlgoPreAmtd* amtd_pre_int        = (RkAiqAlgoPreAmtd*)pAlgoHandler->mPreInParam;

    ret = AiqAlgoHandler_preProcess(pAlgoHandler);
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "amtd handle preprocess failed");
    }

    if(sharedCom->init)
    {
        LOGV("init: no thumb data, ignore!");
        return XCAM_RETURN_NO_ERROR;
    }

    if (!aiqList_empty(pAmtdHdl->mImuDataList)) {
        AiqListItem_t* pItem = NULL;
        bool rm              = false;

        aiqMutex_lock(&pAlgoHandler->mCfgMutex);
        AIQ_LIST_FOREACH(pAmtdHdl->mImuDataList, pItem, rm) {
            aiqList_push(pAmtdHdl->mUsedImuList, pItem->_pData);
            pItem = aiqList_erase_item_locked(pAmtdHdl->mImuDataList, pItem);
            rm    = true;
        }
        aiqMutex_unlock(&pAlgoHandler->mCfgMutex);

        amtd_pre_int->amdImuList = pAmtdHdl->mUsedImuList;
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)pAlgoHandler->mDes;
    ret                       = des->pre_process(pAlgoHandler->mPreInParam, pAlgoHandler->mPreOutParam);
    RKAIQCORE_CHECK_RET(ret, "amtd algo pre_process failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _handlerAmtd_processing(AiqAlgoHandler_t* pAlgoHandler) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AiqAmtdHandler_t* pAmtdHdl = (AiqAmtdHandler_t*)pAlgoHandler;

    RkAiqAlgosGroupShared_t* shared =
        (RkAiqAlgosGroupShared_t*)(pAlgoHandler->mAlogsGroupSharedParams);
    RkAiqAlgosComShared_t* sharedCom = &pAlgoHandler->mAiqCore->mAlogsComSharedParams;
    RkAiqAlgoProcAmtd* amtd_proc        = (RkAiqAlgoProcAmtd*)pAlgoHandler->mProcInParam;
    RkAiqAlgoProcResAmtd* amtd_proc_res = (RkAiqAlgoProcResAmtd*)pAlgoHandler->mProcOutParam;

    ret = AiqAlgoHandler_processing(pAlgoHandler);
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "amtd handle processing failed");
    }

    if (!aiqList_empty(pAmtdHdl->mUsedImuList))
        amtd_proc->amdImuList = pAmtdHdl->mUsedImuList;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)pAlgoHandler->mDes;
    ret                       = des->processing(pAlgoHandler->mProcInParam, pAlgoHandler->mProcOutParam);
    RKAIQCORE_CHECK_RET(ret, "amtd algo processing failed");

    if (!aiqList_empty(pAmtdHdl->mUsedImuList)) {
        //clear imu data list
        AiqListItem_t* pItem = NULL;
        bool rm              = false;

        AIQ_LIST_FOREACH(pAmtdHdl->mUsedImuList, pItem, rm) {
            AiqPoolItem_t *pPoolItem = *(AiqPoolItem_t **)(pItem->_pData);
            pItem = aiqList_erase_item_locked(pAmtdHdl->mUsedImuList, pItem);
            AiqImuData_t *imuData = (AiqImuData_t *)pPoolItem->_pData;
            aiqPoolItem_unref(pPoolItem);
            rm    = true;
        }
    }

    shared->amtdRes = amtd_proc_res->amtdRes;

    RKAIQCORE_CHECK_RET(ret, "amtd algo processing failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}


static XCamReturn _handlerAmtd_genIspResult(AiqAlgoHandler_t* pAlgoHandler, AiqFullParams_t* params,
        AiqFullParams_t* cur_params) {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AiqAmtdHandler_t* pAmtdHdl = (AiqAmtdHandler_t*)pAlgoHandler;

    RkAiqAlgosGroupShared_t* shared =
        (RkAiqAlgosGroupShared_t*)(pAlgoHandler->mAlogsGroupSharedParams);
    RkAiqAlgosComShared_t* sharedCom = &pAlgoHandler->mAiqCore->mAlogsComSharedParams;

    RkAiqAlgoProcResAmtd* amd_res = (RkAiqAlgoProcResAmtd*)pAlgoHandler->mProcOutParam;



    if (!amd_res) {
        LOGW_AMTD("no amtd result");
        return XCAM_RETURN_NO_ERROR;
    }

    // TODO: HOW TO SAVE AMD RESULT, AND TRANSFER TO AE


    cur_params->pParamsArray[RESULT_TYPE_AMTD_PARAM] =
        params->pParamsArray[RESULT_TYPE_AMTD_PARAM];



    EXIT_ANALYZER_FUNCTION();

    return ret;
}

XCamReturn
AiqAlgoHandlerAmtd_setImuData(AiqAmtdHandler_t* pAmtdHdl, AiqImuData_t *data) {

    AiqPoolItem_t *pItem = aiqPool_getFree(pAmtdHdl->mImuDataPool);
    if (!pItem) {
        return XCAM_RETURN_BYPASS;
    }

    memcpy(pItem->_pData, data, sizeof(AiqImuData_t));
    aiqMutex_lock(&pAmtdHdl->_base.mCfgMutex);
    aiqList_push(pAmtdHdl->mImuDataList, &pItem);
    aiqMutex_unlock(&pAmtdHdl->_base.mCfgMutex);

    return XCAM_RETURN_NO_ERROR;
}

AiqAlgoHandler_t* AiqAlgoHandlerAmtd_constructor(RkAiqAlgoDesComm* des, AiqCore_t* aiqCore) {
    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)aiq_mallocz(sizeof(AiqAmtdHandler_t));
    if (!pHdl)
        return NULL;
    AiqAlgoHandler_constructor(pHdl, des, aiqCore);
    pHdl->preProcess   = _handlerAmtd_preProcess;
    pHdl->processing   = _handlerAmtd_processing;
    pHdl->genIspResult = _handlerAmtd_genIspResult;
    pHdl->prepare      = _handlerAmtd_prepare;
    pHdl->init         = _handlerAmtd_init;
    pHdl->deinit       = _handlerAmtd_deinit;
    ((AiqAmtdHandler_t*)pHdl)->mAmtdSyncAly  = true;
    return pHdl;
}
