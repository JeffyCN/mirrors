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

#include "aiq_algo_handler.h"
#include "aiq_core.h"
#include "RkAiqGlobalParamsManager_c.h"
#include "algo_handlers/RkAiqBlcHandler.h"
#include "algo_handlers/RkAiqBtnrHandler.h"
#include "algo_handlers/RkAiqCnrHandler.h"
#include "algo_handlers/RkAiqDehazeHandler.h"
#include "algo_handlers/RkAiqDmHandler.h"
#include "algo_handlers/RkAiqDpccHandler.h"
#include "algo_handlers/RkAiqDrcHandler.h"
#include "algo_handlers/RkAiqGammaHandler.h"
#include "algo_handlers/RkAiqGicHandler.h"
#include "algo_handlers/RkAiqSharpHandler.h"
#include "algo_handlers/RkAiqYnrHandler.h"
#include "algo_handlers/RkAiq3dlutHandler.h"
#include "algo_handlers/RkAiqCcmHandler.h"
#include "algo_handlers/RkAiqLscHandler.h"
#include "algo_handlers/RkAiqCgcHandler.h"
#include "algo_handlers/RkAiqCpHandler.h"
#include "algo_handlers/RkAiqCsmHandler.h"
#include "algo_handlers/RkAiqGainHandler.h"
#include "algo_handlers/RkAiqIeHandler.h"
#include "algo_handlers/RkAiqMergeHandler.h"
#include "algo_handlers/RkAiqRgbirHandler.h"
#include "algo_handlers/RkAiqYmeHandler.h"
#include "algo_handlers/RkAiqAeHandler.h"
#include "algo_handlers/RkAiqAwbHandler.h"
#include "algo_handlers/RkAiqAfdHandler.h"
#include "algo_handlers/RkAiqAfHandler.h"
#include "algo_handlers/RkAiqCacHandler.h"
#include "algo_handlers/RkAiqHisteqHandler.h"
#include "algo_handlers/RkAiqEnhHandler.h"
#include "algo_handlers/RkAiqHsvHandler.h"
#include "algo_handlers/RkAiqLdcHandler.h"
#include "algo_handlers/RkAiqAibnrHandler.h"
#include "algo_handlers/RkAiqAmtdHandler.h"
#include "algo_handlers/RkAiqAirmsHandler.h"
#include "algo_handlers/RkAiqAiynrHandler.h"
#include "interpolation.h"


typedef AiqAlgoHandler_t* (*pAiqAlgoHandler_construct)(RkAiqAlgoDesComm* des, AiqCore_t* aiqCore);
typedef void (*pAiqAlgoHandler_destruct)(AiqAlgoHandler_t* pAlgoHandler);

// global handlers info start
struct {
    int type;
    pAiqAlgoHandler_construct pInitFunc;
    pAiqAlgoHandler_destruct pDeinitFunc;
} g_handlers[] = {
    {RK_AIQ_ALGO_TYPE_AE, AiqAlgoHandlerAe_constructor, AiqAlgoHandler_destructor},
    {RK_AIQ_ALGO_TYPE_AWB, AiqAlgoHandlerAwb_constructor, AiqAlgoHandler_destructor},
    {RK_AIQ_ALGO_TYPE_ABLC, AiqAlgoHandlerBlc_constructor, AiqAlgoHandler_destructor},
    {RK_AIQ_ALGO_TYPE_AMFNR, AiqAlgoHandlerBtnr_constructor, AiqAlgoHandler_destructor},
    {RK_AIQ_ALGO_TYPE_BAYERTNR2, AiqAlgoHandlerBtnr2_constructor, AiqAlgoHandler_destructor},
    {RK_AIQ_ALGO_TYPE_ACNR, AiqAlgoHandlerCnr_constructor, AiqAlgoHandler_destructor},
    {RK_AIQ_ALGO_TYPE_ADEBAYER, AiqAlgoHandlerDm_constructor, AiqAlgoHandler_destructor},
    {RK_AIQ_ALGO_TYPE_ADPCC, AiqAlgoHandlerDpc_constructor, AiqAlgoHandler_destructor},
    {RK_AIQ_ALGO_TYPE_ADRC, AiqAlgoHandlerDrc_constructor, AiqAlgoHandler_destructor},
    {RK_AIQ_ALGO_TYPE_AGAMMA, AiqAlgoHandlerGamma_constructor, AiqAlgoHandler_destructor},
    {RK_AIQ_ALGO_TYPE_ASHARP, AiqAlgoHandlerSharp_constructor, AiqAlgoHandler_destructor},
    {RK_AIQ_ALGO_TYPE_AYNR, AiqAlgoHandlerYnr_constructor, AiqAlgoHandler_destructor},
    {RK_AIQ_ALGO_TYPE_ACCM, AiqAlgoHandlerCcm_constructor, AiqAlgoHandler_destructor},
    {RK_AIQ_ALGO_TYPE_ALSC, AiqAlgoHandlerLsc_constructor, AiqAlgoHandler_destructor},
    {RK_AIQ_ALGO_TYPE_ACGC, AiqAlgoHandlerCgc_constructor, AiqAlgoHandler_destructor},
    {RK_AIQ_ALGO_TYPE_ACP, AiqAlgoHandlerCp_constructor, AiqAlgoHandler_destructor},
    {RK_AIQ_ALGO_TYPE_ACSM, AiqAlgoHandlerCsm_constructor, AiqAlgoHandler_destructor},
    {RK_AIQ_ALGO_TYPE_AGAIN, AiqAlgoHandlerGain_constructor, AiqAlgoHandler_destructor},
    {RK_AIQ_ALGO_TYPE_AIE, AiqAlgoHandlerIe_constructor, AiqAlgoHandler_destructor},
    {RK_AIQ_ALGO_TYPE_AMERGE, AiqAlgoHandlerMerge_constructor, AiqAlgoHandler_destructor},
    {RK_AIQ_ALGO_TYPE_AFD, AiqAlgoHandlerAfd_constructor, AiqAlgoHandler_destructor},
    {RK_AIQ_ALGO_TYPE_AGIC, AiqAlgoHandlerGic_constructor, AiqAlgoHandler_destructor},
    {RK_AIQ_ALGO_TYPE_ACAC, AiqAlgoHandlerCac_constructor, AiqAlgoHandler_destructor},
    {RK_AIQ_ALGO_TYPE_AHISTEQ, AiqAlgoHandlerHisteq_constructor, AiqAlgoHandler_destructor},
#if RKAIQ_HAVE_ENHANCE
    {RK_AIQ_ALGO_TYPE_AENH, AiqAlgoHandlerEnh_constructor, AiqAlgoHandler_destructor},
#endif
#if RKAIQ_HAVE_HSV
    {RK_AIQ_ALGO_TYPE_AHSV, AiqAlgoHandlerHsv_constructor, AiqAlgoHandler_destructor},
#endif
#if RKAIQ_HAVE_DEHAZE
    {RK_AIQ_ALGO_TYPE_ADHAZ, AiqAlgoHandlerDehaze_constructor, AiqAlgoHandler_destructor},
#endif
#if RKAIQ_HAVE_3DLUT
    {RK_AIQ_ALGO_TYPE_A3DLUT, AiqAlgoHandler3dlut_constructor, AiqAlgoHandler_destructor},
#endif
#if RKAIQ_HAVE_RGBIR_REMOSAIC
    {RK_AIQ_ALGO_TYPE_ARGBIR, AiqAlgoHandlerRgbir_constructor, AiqAlgoHandler_destructor},
#endif
#if RKAIQ_HAVE_YUVME
    {RK_AIQ_ALGO_TYPE_AMD, AiqAlgoHandlerYme_constructor, AiqAlgoHandler_destructor},
#endif
#ifdef RKAIQ_HAVE_AF
    {RK_AIQ_ALGO_TYPE_AF, AiqAlgoHandlerAf_constructor, AiqAlgoHandler_destructor},
#endif
#if RKAIQ_HAVE_LDC
    {RK_AIQ_ALGO_TYPE_ALDC, AiqAlgoHandlerLdc_constructor, AiqAlgoHandler_destructor},
#endif
#if RKAIQ_HAVE_AIBNR
    {RK_AIQ_ALGO_TYPE_AIBNR, AiqAlgoHandlerAibnr_constructor, AiqAlgoHandler_destructor},
#endif
#if RKAIQ_HAVE_AMTD
    {RK_AIQ_ALGO_TYPE_AMTD, AiqAlgoHandlerAmtd_constructor, AiqAlgoHandler_destructor},
#endif
#if RKAIQ_HAVE_AIRMS
    {RK_AIQ_ALGO_TYPE_AIRMS, AiqAlgoHandlerAirms_constructor, AiqAlgoHandler_destructor},
#endif
#if RKAIQ_HAVE_AIYNR
    {RK_AIQ_ALGO_TYPE_AIYNR, AiqAlgoHandlerAiynr_constructor, AiqAlgoHandler_destructor},
#endif

};

// global handlers info end

XCamReturn AiqAlgoHandler_constructor(AiqAlgoHandler_t* pAlgoHandler, RkAiqAlgoDesComm* des,
                                      AiqCore_t* aiqCore) {
    aiqMutex_init(&pAlgoHandler->mCfgMutex);
    pAlgoHandler->mDes               = des;
    pAlgoHandler->mAiqCore           = aiqCore;
    pAlgoHandler->mEnable            = true;
    pAlgoHandler->mReConfig          = false;
    pAlgoHandler->mResultType        = -1;
    RkAiqAlgosComShared_t* sharedCom = &aiqCore->mAlogsComSharedParams;
    if (des) {
        sharedCom->ctxCfigs[des->type].cid = aiqCore->mAlogsComSharedParams.mCamPhyId;
        des->create_context(&pAlgoHandler->mAlgoCtx,
                            (const AlgoCtxInstanceCfg*)(&sharedCom->ctxCfigs[des->type]));
    }
    pAlgoHandler->mConfig                 = NULL;
    pAlgoHandler->mPreInParam             = NULL;
    pAlgoHandler->mPreOutParam            = NULL;
    pAlgoHandler->mProcInParam            = NULL;
    pAlgoHandler->mProcOutParam           = NULL;
    pAlgoHandler->mPostInParam            = NULL;
    pAlgoHandler->mPostOutParam           = NULL;
    pAlgoHandler->mNextHdl                = NULL;
    pAlgoHandler->mParentHdl              = NULL;
    pAlgoHandler->mIsMulRun               = false;
    pAlgoHandler->mPostShared             = true;
    pAlgoHandler->mGroupId                = 0;
    pAlgoHandler->mAlogsGroupSharedParams = NULL;
    pAlgoHandler->mSyncFlag               = (uint32_t)(-1);
    pAlgoHandler->configInparamsCom       = AiqAlgoHandler_configInparamsCom;
    pAlgoHandler->deinit                  = AiqAlgoHandler_deinit;
    pAlgoHandler->postProcess             = NULL;
    pAlgoHandler->preProcess              = NULL;
    pAlgoHandler->queryInterpIso          = AiqAlgoHandler_queryInterpIso_common;

    return XCAM_RETURN_NO_ERROR;
}

void AiqAlgoHandler_destructor(AiqAlgoHandler_t* pAlgoHandler) {
    if (pAlgoHandler->mDes) {
        pAlgoHandler->mDes->destroy_context(pAlgoHandler->mAlgoCtx);
        pAlgoHandler->mAlgoCtx = NULL;
        pAlgoHandler->mDes     = NULL;
    }
    pAlgoHandler->deinit(pAlgoHandler);
    aiqMutex_deInit(&pAlgoHandler->mCfgMutex);
    aiq_free(pAlgoHandler);
}

AiqAlgoHandler_t* createAlgoHandlerInstance(int type, RkAiqAlgoDesComm* des, AiqCore_t* aiqCore) {
    for (uint32_t i = 0; i < sizeof(g_handlers) / sizeof(g_handlers[0]); i++) {
        if (type == g_handlers[i].type) {
            return g_handlers[i].pInitFunc(des, aiqCore);
        }
    }

    return NULL;
}

void destroyAlgoHandler(AiqAlgoHandler_t* pAlgoHandler) {
    for (uint32_t i = 0; i < sizeof(g_handlers) / sizeof(g_handlers[0]); i++) {
        if (pAlgoHandler->mDes->type == g_handlers[i].type) {
            return g_handlers[i].pDeinitFunc(pAlgoHandler);
        }
    }
}

void AiqAlgoHandler_deinit(AiqAlgoHandler_t* pAlgoHandler) {
    ENTER_ANALYZER_FUNCTION();

#define RKAIQ_DELLET(a) \
    if (a) {            \
        aiq_free(a);       \
        a = NULL;       \
    }

    RKAIQ_DELLET(pAlgoHandler->mConfig);
    RKAIQ_DELLET(pAlgoHandler->mPreInParam);
    RKAIQ_DELLET(pAlgoHandler->mPreOutParam);
    RKAIQ_DELLET(pAlgoHandler->mProcInParam);
    RKAIQ_DELLET(pAlgoHandler->mProcOutParam);
    RKAIQ_DELLET(pAlgoHandler->mPostInParam);
    RKAIQ_DELLET(pAlgoHandler->mPostOutParam);

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn AiqAlgoHandler_prepare(AiqAlgoHandler_t* pAlgoHandler) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (pAlgoHandler->mConfig == NULL) pAlgoHandler->init(pAlgoHandler);
    // build common configs
    RkAiqAlgoCom* cfgParam = pAlgoHandler->mConfig;
    pAlgoHandler->configInparamsCom(pAlgoHandler, cfgParam, RKAIQ_CONFIG_COM_PREPARE);

    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqAlgoHandler_preProcess(AiqAlgoHandler_t* pAlgoHandler) {
    XCamReturn ret         = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoCom* preParam = pAlgoHandler->mPreInParam;

    pAlgoHandler->configInparamsCom(pAlgoHandler, preParam, RKAIQ_CONFIG_COM_PRE);

    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqAlgoHandler_processing(AiqAlgoHandler_t* pAlgoHandler) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret          = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoCom* procParam = pAlgoHandler->mProcInParam;

    pAlgoHandler->configInparamsCom(pAlgoHandler, procParam, RKAIQ_CONFIG_COM_PROC);

    if (pAlgoHandler->mProcOutParam) pAlgoHandler->mProcOutParam->cfg_update = false;

    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqAlgoHandler_postProcess(AiqAlgoHandler_t* pAlgoHandler) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret          = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoCom* postParam = pAlgoHandler->mPostInParam;

    pAlgoHandler->configInparamsCom(pAlgoHandler, postParam, RKAIQ_CONFIG_COM_POST);

    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqAlgoHandler_configInparamsCom(AiqAlgoHandler_t* pAlgoHandler, RkAiqAlgoCom* com,
        int type) {
    RkAiqAlgosGroupShared_t* shared =
        (RkAiqAlgosGroupShared_t*)(pAlgoHandler->mAlogsGroupSharedParams);

    RkAiqAlgosComShared_t* sharedCom = &pAlgoHandler->mAiqCore->mAlogsComSharedParams;
    xcam_mem_clear(*com);

    com->cid = pAlgoHandler->mAiqCore->mAlogsComSharedParams.mCamPhyId;
    if (type == RKAIQ_CONFIG_COM_PREPARE) {
        com->ctx                     = pAlgoHandler->mAlgoCtx;
        com->frame_id                = shared->frameId;
        com->u.prepare.working_mode  = sharedCom->working_mode;
        com->u.prepare.sns_op_width  = sharedCom->snsDes.isp_acq_width;
        com->u.prepare.sns_op_height = sharedCom->snsDes.isp_acq_height;
        com->u.prepare.conf_type     = sharedCom->conf_type;
        com->u.prepare.calibv2       = (CamCalibDbV2Context_t*)(sharedCom->calibv2);
        com->u.prepare.compr_bit     = sharedCom->snsDes.compr_bit;
    } else {
        com->ctx                  = pAlgoHandler->mAlgoCtx;
        com->frame_id             = shared->frameId;
        com->u.proc.init          = sharedCom->init;
        com->u.proc.iso           = shared->iso;
        com->u.proc.fill_light_on = sharedCom->fill_light_on;
        com->u.proc.gray_mode     = sharedCom->gray_mode;
        com->u.proc.is_bw_sensor  = sharedCom->is_bw_sensor;
        com->u.proc.preExp        = &shared->preExp;
        com->u.proc.curExp        = &shared->curExp;
        com->u.proc.nxtExp        = &shared->nxtExp;
        com->u.proc.res_comb      = &shared->res_comb;
        com->u.proc.aibnr_fixIndex = sharedCom->aibnr_fixIndex;
        com->u.proc.is_aibnr_autorun = sharedCom->is_aibnr_autorun;
        com->u.proc.is_aibnr_force_update = sharedCom->is_aibnr_force_update;
    }
    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

void AiqAlgoHandler_setEnable(AiqAlgoHandler_t* pAlgoHandler, bool enable) {
    pAlgoHandler->mEnable = enable;
}

bool AiqAlgoHandler_getEnable(AiqAlgoHandler_t* pAlgoHandler) {
    return pAlgoHandler->mEnable;
}

void AiqAlgoHandler_setReConfig(AiqAlgoHandler_t* pAlgoHandler, bool reconfig) {
    pAlgoHandler->mReConfig = reconfig;
}

RkAiqAlgoContext* AiqAlgoHandler_getAlgoCtx(AiqAlgoHandler_t* pAlgoHandler) {
    return pAlgoHandler->mAlgoCtx;
}

int AiqAlgoHandler_getAlgoId(AiqAlgoHandler_t* pAlgoHandler) {
    if (pAlgoHandler->mDes)
        return pAlgoHandler->mDes->id;
    else
        return -1;
}

int AiqAlgoHandler_getAlgoType(AiqAlgoHandler_t* pAlgoHandler) {
    if (pAlgoHandler->mDes)
        return pAlgoHandler->mDes->type;
    else
        return -1;
}

void AiqAlgoHandler_setGroupId(AiqAlgoHandler_t* pAlgoHandler, int32_t gId) {
    pAlgoHandler->mGroupId = gId;
}

int32_t AiqAlgoHandler_getGroupId(AiqAlgoHandler_t* pAlgoHandler) {
    return pAlgoHandler->mGroupId;
}

void AiqAlgoHandler_setNextHdl(AiqAlgoHandler_t* pAlgoHandler, AiqAlgoHandler_t* next) {
    pAlgoHandler->mNextHdl = next;
}

AiqAlgoHandler_t* AiqAlgoHandler_getNextHdl(AiqAlgoHandler_t* pAlgoHandler) {
    return pAlgoHandler->mNextHdl;
}

void AiqAlgoHandler_setParentHdl(AiqAlgoHandler_t* pAlgoHandler, AiqAlgoHandler_t* parent) {
    pAlgoHandler->mParentHdl = parent;
}

AiqAlgoHandler_t* AiqAlgoHandler_getParent(AiqAlgoHandler_t* pAlgoHandler) {
    return pAlgoHandler->mParentHdl;
}

void AiqAlgoHandler_setMulRun(AiqAlgoHandler_t* pAlgoHandler, bool isMulRun) {
    pAlgoHandler->mIsMulRun = isMulRun;
}

void AiqAlgoHandler_setGroupShared(AiqAlgoHandler_t* pAlgoHandler, void* grp_shared) {
    pAlgoHandler->mAlogsGroupSharedParams = grp_shared;
}

void* AiqAlgoHandler_getGroupShared(AiqAlgoHandler_t* pAlgoHandler) {
    return pAlgoHandler->mAlogsGroupSharedParams;
}

RkAiqAlgoResCom* AiqAlgoHandler_getProcProcRes(AiqAlgoHandler_t* pAlgoHandler) {
    return pAlgoHandler->mProcOutParam;
}

bool AiqAlgoHandler_isUpdateGrpAttr(AiqAlgoHandler_t* pAlgoHandler) {
    return pAlgoHandler->mIsUpdateGrpAttr;
}

void AiqAlgoHandler_clearUpdateGrpAttr(AiqAlgoHandler_t* pAlgoHandler) {
    pAlgoHandler->mIsUpdateGrpAttr = false;
}

uint64_t AiqAlgoHandler_grpId2GrpMask(AiqAlgoHandler_t* pAlgoHandler, uint32_t grpId) {
    return grpId == RK_AIQ_CORE_ANALYZE_ALL ? (uint64_t)grpId : (1ULL << grpId);
}

XCamReturn AiqAlgoHandler_do_processing_common(AiqAlgoHandler_t* pAlgoHandler)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqAlgosGroupShared_t* shared =
        (RkAiqAlgosGroupShared_t*)(pAlgoHandler->mAlogsGroupSharedParams);
    RkAiqAlgosComShared_t* sharedCom = &pAlgoHandler->mAiqCore->mAlogsComSharedParams;

    int restype = pAlgoHandler->mResultType;
    const char *ResTypeStr = Cam3aResultType2Str[restype];

    aiq_params_base_t* pBase = shared->fullParams->pParamsArray[restype];
    if (!pBase) {
        LOGW("%s: no params buf !", ResTypeStr);
        return XCAM_RETURN_BYPASS;
    }

    RkAiqAlgoResCom* proc_res = pAlgoHandler->mProcOutParam;
    proc_res->algoRes = (void *)pBase->_data;

    GlobalParamsManager_t * globalParamsManager = pAlgoHandler->mAiqCore->mGlobalParamsManger;
    GlobalParamsManager_lockAlgoParam(globalParamsManager, pAlgoHandler->mResultType);
    if (globalParamsManager && !GlobalParamsManager_isFullManualMode(globalParamsManager) &&
            GlobalParamsManager_isManualLocked(globalParamsManager, pAlgoHandler->mResultType)) {
        rk_aiq_global_params_wrap_t wrap_param;
        wrap_param.type           = pAlgoHandler->mResultType;
        wrap_param.man_param_size = pAlgoHandler->mResultSize;
        wrap_param.man_param_ptr  = proc_res->algoRes;
        wrap_param.aut_param_ptr  =  NULL;
        XCamReturn ret1           = GlobalParamsManager_getAndClearPendingLocked(globalParamsManager, &wrap_param);
        if (ret1 == XCAM_RETURN_NO_ERROR) {
            LOGK("%s: %p, man en:%d, bypass:%d", ResTypeStr,
                 &proc_res->en, wrap_param.en, wrap_param.bypass);
            proc_res->en = wrap_param.en;
            proc_res->bypass = wrap_param.bypass;
            proc_res->cfg_update = true;
            pAlgoHandler->mOpMode = RK_AIQ_OP_MODE_MANUAL;
        } else {
            proc_res->cfg_update = false;
        }
    } else {
        pAlgoHandler->mProcInParam->u.proc.is_attrib_update =
            GlobalParamsManager_getAndClearAlgoParamUpdateFlagLocked(globalParamsManager, pAlgoHandler->mResultType);

        pAlgoHandler->mIsUpdateGrpAttr =
            pAlgoHandler->mProcInParam->u.proc.is_attrib_update;

        if (!AiqCore_isGroupAlgo(pAlgoHandler->mAiqCore, pAlgoHandler->mDes->type)) {
            RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)pAlgoHandler->mDes;
            ret = des->processing(pAlgoHandler->mProcInParam, pAlgoHandler->mProcOutParam);
        } else {
            proc_res->cfg_update = false;
        }
        if (pAlgoHandler->mProcInParam->u.proc.is_attrib_update)
            pAlgoHandler->mOpMode = RK_AIQ_OP_MODE_AUTO;
    }
    GlobalParamsManager_unlockAlgoParam(globalParamsManager, pAlgoHandler->mResultType);

    return ret;
}

XCamReturn AiqAlgoHandler_genIspResult_byType(AiqAlgoHandler_t* pAlgoHandler,
        AiqFullParams_t* params, AiqFullParams_t* cur_params, int restype, int size)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgosGroupShared_t* shared =
        (RkAiqAlgosGroupShared_t*)(pAlgoHandler->mAlogsGroupSharedParams);
    RkAiqAlgosComShared_t* sharedCom = &pAlgoHandler->mAiqCore->mAlogsComSharedParams;
    RkAiqAlgoResCom* proc_res = pAlgoHandler->mProcOutParam;
    aiq_params_base_t* pBase = params->pParamsArray[restype];
    const char *ResTypeStr = Cam3aResultType2Str[restype];

    if (!pBase) {
        LOGW("no %s params buf !", ResTypeStr);
        return XCAM_RETURN_BYPASS;
    }

    if (!proc_res) {
        LOGE("no %s result !", ResTypeStr);
        return XCAM_RETURN_NO_ERROR;
    }

    void* algo_param = (void*)pBase->_data;
    aiq_params_base_t* pCurBase = cur_params->pParamsArray[restype];
    void* cur_algo_param = NULL;
    if (pCurBase)
        cur_algo_param = (void*)pCurBase->_data;

    if (sharedCom->init) {
        pBase->frame_id = 0;
    } else {
        pBase->frame_id = shared->frameId;
    }

    if (AiqCore_isGroupAlgo(pAlgoHandler->mAiqCore, pAlgoHandler->mDes->type)) {
        cur_params->pParamsArray[restype] = pBase;
        if (proc_res->cfg_update) {
            pBase->en = proc_res->en;
            pBase->bypass = proc_res->bypass;
            pBase->is_update = true;
        }
        return XCAM_RETURN_NO_ERROR;
    }

    if (proc_res->cfg_update) {
        pAlgoHandler->mSyncFlag = shared->frameId;
        pBase->sync_flag = pAlgoHandler->mSyncFlag;
        pBase->en = proc_res->en;
        pBase->bypass = proc_res->bypass;
        // copy from algo result
        // set as the latest result
        cur_params->pParamsArray[restype] = pBase;

        pBase->is_update = true;
        LOGI("%s [%d] params from algo", ResTypeStr, pAlgoHandler->mSyncFlag);
    } else if (pAlgoHandler->mSyncFlag != pBase->sync_flag) {
        pBase->sync_flag = pAlgoHandler->mSyncFlag;
        // copy from latest result
        if (pCurBase) {
            memcpy(algo_param, cur_algo_param, size);
            pBase->en        = pCurBase->en;
            pBase->bypass    = pCurBase->bypass;
            pBase->is_update = true;
        } else {
            LOGE("%s: no latest params !", ResTypeStr);
            pBase->is_update = false;
        }
        LOGD("%s: [%d] params from latest [%d]", ResTypeStr, shared->frameId, pAlgoHandler->mSyncFlag);
    } else {
        // do nothing, result in buf needn't update
        pBase->is_update = false;
        LOGV("%s: [%d] params needn't update", ResTypeStr, shared->frameId);
    }

    return ret;
}

XCamReturn AiqAlgoHandler_genIspResult_common(AiqAlgoHandler_t* pAlgoHandler,
        AiqFullParams_t* params, AiqFullParams_t* cur_params)
{
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    int restype = pAlgoHandler->mResultType;
    int size = pAlgoHandler->mResultSize;
    ret = AiqAlgoHandler_genIspResult_byType(pAlgoHandler, params, cur_params, restype, size);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
AiqAlgoHandler_queryStatus_common(AiqAlgoHandler_t* pAlgoHandler,
                                  rk_aiq_op_mode_t* opMode, bool* en, bool* bypass, void* stMan)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAlgoHandler;

    aiqMutex_lock(&pHdl->mCfgMutex);

    aiq_params_base_t* pCurBase =
        pHdl->mAiqCore->mAiqCurParams->pParamsArray[pHdl->mResultType];

    if (pCurBase) {
        memcpy(stMan, pCurBase->_data, pHdl->mResultSize);
        *en     = pCurBase->en;
        *bypass = pCurBase->bypass;
        *opMode = pHdl->mOpMode;
    } else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE("have no status info !");
    }

    aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandler_queryInterpIso_common(AiqAlgoHandler_t* pAlgoHandler,
                                             uint32_t* interpIso, uint32_t* isoH, uint32_t* isoL)
{
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAlgoHandler;

    aiqMutex_lock(&pHdl->mCfgMutex);

    RkAiqAlgosGroupShared_t* shared =
        (RkAiqAlgosGroupShared_t*)(pAlgoHandler->mAlogsGroupSharedParams);
    RkAiqAlgosComShared_t* sharedCom = &pAlgoHandler->mAiqCore->mAlogsComSharedParams;

    aiq_params_base_t* pCurBase =
        pHdl->mAiqCore->mAiqCurParams->pParamsArray[pHdl->mResultType];

    *interpIso = shared->iso;
    if (pCurBase) {
        if (pHdl->mOpMode == RK_AIQ_OP_MODE_MANUAL) {
           *isoL = 0;
           *isoH = 0;
           aiqMutex_unlock(&pHdl->mCfgMutex);
           return ret;
        }
    }

    bool isFixedAlgo = (pAlgoHandler->mDes->type == RK_AIQ_ALGO_TYPE_AMFNR ||
                       pAlgoHandler->mDes->type == RK_AIQ_ALGO_TYPE_ACNR ||
                       pAlgoHandler->mDes->type == RK_AIQ_ALGO_TYPE_ASHARP ||
                       pAlgoHandler->mDes->type == RK_AIQ_ALGO_TYPE_AYNR);

    if (isFixedAlgo && (sharedCom->aibnr_fixIndex != -1)) {
        *isoH = sharedCom->calibv2->sensor_info->iso_list[sharedCom->aibnr_fixIndex];
        *isoL = *isoH;
    } else {
        float ratio = 0.0f;
        int iLow = 0, iHigh = 0;
        pre_interp(shared->iso, sharedCom->calibv2->sensor_info->iso_list, 13, &iLow, &iHigh, &ratio);
        if (iLow != -1)
           *isoL =  sharedCom->calibv2->sensor_info->iso_list[iLow];
        if (iHigh != -1)
           *isoH =  sharedCom->calibv2->sensor_info->iso_list[iHigh];
    }

    aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

