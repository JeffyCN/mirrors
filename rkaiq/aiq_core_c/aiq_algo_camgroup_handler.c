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

#include "aiq_algo_camgroup_handler.h"
#include "aiq_core.h"
#include "RkAiqCamGroupManager_c.h"
#include "aiq_algo_handler.h"

void AiqAlgoCamGroupHandler_init(AiqAlgoCamGroupHandler_t* pAlgoCamGroupHandler);

typedef AiqAlgoCamGroupHandler_t* (*pAiqAlgoCamGroupHandler_construct)(RkAiqAlgoDesComm* des, AiqCamGroupManager_t* camGroupMg, AiqCore_t* aiqcore);
typedef void (*pAiqAlgoCamGroupHandler_destruct)(AiqAlgoCamGroupHandler_t* pAlgoHandler);

#if 0
// global handlers info start
struct {
    int type;
    pAiqAlgoCamGroupHandler_construct pInitFunc;
    pAiqAlgoCamGroupHandler_destruct pDeinitFunc;
} g_camgroup_handlers[] = {
	{0, AiqAlgoCamGroupHandler_constructor, AiqAlgoCamGroupHandler_destructor}
};
// global handlers info end 
#endif

AiqAlgoCamGroupHandler_t*
AiqAlgoCamGroupHandler_constructor(RkAiqAlgoDesComm* des,
								AiqCamGroupManager_t* camGroupMg, AiqCore_t* aiqcore) {
	AiqAlgoCamGroupHandler_t* pAlgoCamGroupHandler = aiq_mallocz(sizeof(AiqAlgoCamGroupHandler_t));
    aiqMutex_init(&pAlgoCamGroupHandler->mCfgMutex);
    pAlgoCamGroupHandler->mDes               = des;
    pAlgoCamGroupHandler->mAiqCore           = aiqcore;
    pAlgoCamGroupHandler->mEnable            = true;
    pAlgoCamGroupHandler->mGroupMg = camGroupMg;
    if (des)
        des->create_context(&pAlgoCamGroupHandler->mAlgoCtx,
                            (const AlgoCtxInstanceCfg*)(&camGroupMg->mGroupAlgoCtxCfg));
    pAlgoCamGroupHandler->deinit                  = AiqAlgoCamGroupHandler_deinit;
    pAlgoCamGroupHandler->init = AiqAlgoCamGroupHandler_init;
    pAlgoCamGroupHandler->prepare = AiqAlgoCamGroupHandler_prepare;
    pAlgoCamGroupHandler->processing = AiqAlgoCamGroupHandler_processing;
	return pAlgoCamGroupHandler;
}

AiqAlgoCamGroupHandler_t* createAlgoCamGroupHandlerInstance(int type, RkAiqAlgoDesComm* des, AiqCamGroupManager_t* camGroupMg, AiqCore_t* aiqcore)
{
#if 0
    for (uint32_t i = 0; i < sizeof(g_camgroup_handlers) / sizeof(g_camgroup_handlers[0]); i++) {
        if (type == g_camgroup_handlers[i].type) {
            return g_camgroup_handlers[i].pInitFunc(des, camGroupMg, aiqcore);
        }
    }
    return NULL;
#else
	return AiqAlgoCamGroupHandler_constructor(des, camGroupMg, aiqcore);
#endif
}

void AiqAlgoCamgroupHandler_destructor(AiqAlgoCamGroupHandler_t* pAlgoCamGroupHandler) {
    if (pAlgoCamGroupHandler->mDes) {
        pAlgoCamGroupHandler->mDes->destroy_context(pAlgoCamGroupHandler->mAlgoCtx);
        pAlgoCamGroupHandler->mAlgoCtx = NULL;
        pAlgoCamGroupHandler->mDes     = NULL;
    }
    pAlgoCamGroupHandler->deinit(pAlgoCamGroupHandler);
    aiqMutex_deInit(&pAlgoCamGroupHandler->mCfgMutex);
    aiq_free(pAlgoCamGroupHandler);
}

void destroyAlgoCamGroupHandler(AiqAlgoCamGroupHandler_t* pAlgoCamGroupHandler) {
#if 0
    for (uint32_t i = 0; i < sizeof(g_camgroup_handlers) / sizeof(g_camgroup_handlers[0]); i++) {
        if (pAlgoCamGroupHandler->mDes->type == g_camgroup_handlers[i].type) {
            return g_camgroup_handlers[i].pDeinitFunc(pAlgoCamGroupHandler);
        }
    }
#else
	AiqAlgoCamgroupHandler_destructor(pAlgoCamGroupHandler);
#endif
}

void AiqAlgoCamGroupHandler_init(AiqAlgoCamGroupHandler_t* pAlgoCamGroupHandler) {
    ENTER_ANALYZER_FUNCTION();
    AiqAlgoCamGroupHandler_deinit(pAlgoCamGroupHandler);
    pAlgoCamGroupHandler->mConfig = (RkAiqAlgoCom*)(aiq_mallocz(sizeof(RkAiqAlgoCamGroupPrepare)));
    pAlgoCamGroupHandler->mProcInParam = (RkAiqAlgoCom*)(aiq_mallocz(sizeof(RkAiqAlgoCamGroupProcIn)));
    pAlgoCamGroupHandler->mProcOutParam = (RkAiqAlgoResCom*)(aiq_mallocz(sizeof(RkAiqAlgoCamGroupProcOut)));

    EXIT_ANALYZER_FUNCTION();
}

void AiqAlgoCamGroupHandler_deinit(AiqAlgoCamGroupHandler_t* pAlgoCamGroupHandler) {
    ENTER_ANALYZER_FUNCTION();

#define RKAIQ_DELLET(a) \
    if (a) {            \
        aiq_free(a);       \
        a = NULL;       \
    }

    RKAIQ_DELLET(pAlgoCamGroupHandler->mConfig);
    RKAIQ_DELLET(pAlgoCamGroupHandler->mProcInParam);
    RKAIQ_DELLET(pAlgoCamGroupHandler->mProcOutParam);

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn AiqAlgoCamGroupHandler_prepare(AiqAlgoCamGroupHandler_t* pAlgoCamGroupHandler, AiqCore_t* aiqCore)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    pAlgoCamGroupHandler->mAiqCore = aiqCore;

    if (pAlgoCamGroupHandler->mConfig == NULL) pAlgoCamGroupHandler->init (pAlgoCamGroupHandler);

    // assume all single cam runs same algos

    RkAiqAlgoCamGroupPrepare* prepareCfg = (RkAiqAlgoCamGroupPrepare*)pAlgoCamGroupHandler->mConfig ;

    RkAiqAlgoComCamGroup* gcom = &prepareCfg->gcom;
    RkAiqAlgoCom *com = &gcom->com;
    RkAiqAlgosComShared_t* sharedCom = &aiqCore->mAlogsComSharedParams;

    prepareCfg->camIdArrayLen = aiqMap_size(pAlgoCamGroupHandler->mGroupMg->mBindAiqsMap);

    int i = 0;
	AiqMapItem_t* pItem = NULL;
	bool rm             = false;
	AIQ_MAP_FOREACH(pAlgoCamGroupHandler->mGroupMg->mBindAiqsMap, pItem, rm) {
        prepareCfg->camIdArray[i++] = (uint32_t)(long)pItem->_key;
	}

    prepareCfg->aec.LinePeriodsPerField =
        (float)sharedCom->snsDes.frame_length_lines;
    prepareCfg->aec.PixelClockFreqMHZ =
        (float)sharedCom->snsDes.pixel_clock_freq_mhz;
    prepareCfg->aec.PixelPeriodsPerLine =
        (float)sharedCom->snsDes.line_length_pck;
    prepareCfg->s_calibv2 = pAlgoCamGroupHandler->mGroupMg->mGroupAlgoCtxCfg.s_calibv2;

    prepareCfg->pCamgroupCalib = pAlgoCamGroupHandler->mGroupMg->mCamgroupCalib;
    prepareCfg->aec.nr_switch = sharedCom->snsDes.nr_switch;

    com->ctx                     = pAlgoCamGroupHandler->mAlgoCtx;
    com->frame_id                = 0;
    com->u.prepare.working_mode  = sharedCom->working_mode;
    com->u.prepare.sns_op_width  = sharedCom->snsDes.isp_acq_width;
    com->u.prepare.sns_op_height = sharedCom->snsDes.isp_acq_height;
    com->u.prepare.compr_bit     = sharedCom->snsDes.compr_bit;
    com->u.prepare.conf_type     = sharedCom->conf_type;
    com->u.prepare.calibv2 =
        (CamCalibDbV2Context_t*)(prepareCfg->s_calibv2);
    if (pAlgoCamGroupHandler->mDes->type == RK_AIQ_ALGO_TYPE_AWB) {
		aiqMutex_lock(&pAlgoCamGroupHandler->mCfgMutex);
        ret = ((RkAiqAlgoDescription*)pAlgoCamGroupHandler->mDes)->prepare(com);
		aiqMutex_unlock(&pAlgoCamGroupHandler->mCfgMutex);
    }
    else {
        ret = ((RkAiqAlgoDescription*)pAlgoCamGroupHandler->mDes)->prepare(com);
    }

    if (ret) {
        LOGE_CAMGROUP("algo %d prepare failed !", pAlgoCamGroupHandler->mDes->type);
        return ret;
    }

    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqAlgoCamGroupHandler_processing(AiqAlgoCamGroupHandler_t* pAlgoCamGroupHandler, rk_aiq_singlecam_3a_result_t** params_res_array)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret          = XCAM_RETURN_NO_ERROR;

    RkAiqAlgosComShared_t* sharedCom = &pAlgoCamGroupHandler->mAiqCore->mAlogsComSharedParams;

    RkAiqAlgoCamGroupProcIn* procIn =
        (RkAiqAlgoCamGroupProcIn*)pAlgoCamGroupHandler->mProcInParam;
    RkAiqAlgoCamGroupProcOut* procOut =
        (RkAiqAlgoCamGroupProcOut*)pAlgoCamGroupHandler->mProcOutParam;

    memset(procIn, 0, sizeof(RkAiqAlgoCamGroupProcIn));
    memset(procOut, 0, sizeof(RkAiqAlgoCamGroupProcOut));

    RkAiqAlgoComCamGroup* gcom = &procIn->gcom;
    RkAiqAlgoCom *com = &gcom->com;

    procIn->arraySize = aiqMap_size(pAlgoCamGroupHandler->mGroupMg->mBindAiqsMap);
    procOut->arraySize = procIn->arraySize;

    procIn->camgroupParmasArray = params_res_array;
    procOut->camgroupParmasArray = params_res_array;
    procIn->_gray_mode = sharedCom->gray_mode;
    procIn->working_mode = sharedCom->working_mode;
    procIn->_is_bw_sensor = sharedCom->is_bw_sensor;
    procIn->_offset_is_update = (size_t)(((aiq_params_base_t*)0)->_data);
    //procIn->attribUpdated = mSingleHdl->isUpdateGrpAttr();

    com->ctx         = pAlgoCamGroupHandler->mAlgoCtx;
    com->frame_id    = params_res_array[0]->_frameId;
    // TODO: remove init info ? algo can maintain the state itself
    com->u.proc.init = pAlgoCamGroupHandler->mGroupMg->mInit;
    com->u.proc.iso = procIn->camgroupParmasArray[0]->hdrIso;
    AiqAlgoHandler_t* pSglHdl = pAlgoCamGroupHandler->mSingleHdl;
    GlobalParamsManager_t* globalParamsManager = pSglHdl->mAiqCore->mGlobalParamsManger;
    if (pSglHdl->mResultType != -1)
        GlobalParamsManager_lockAlgoParam(globalParamsManager, pSglHdl->mResultType);
    procIn->attribUpdated = pSglHdl->mIsUpdateGrpAttr;
    com->u.proc.is_attrib_update = pSglHdl->mIsUpdateGrpAttr;
    pSglHdl->mIsUpdateGrpAttr = false;
    if (pSglHdl->mResultType != -1)
        GlobalParamsManager_unlockAlgoParam(globalParamsManager, pSglHdl->mResultType);
    ret = ((RkAiqAlgoDescription*)pAlgoCamGroupHandler->mDes)->processing((const RkAiqAlgoCom*)procIn,
            (RkAiqAlgoResCom*)procOut);
    if (ret) {
        LOGW_CAMGROUP("group algo %d proc error !", pAlgoCamGroupHandler->mDes->type);
        return ret;
    }

    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}


