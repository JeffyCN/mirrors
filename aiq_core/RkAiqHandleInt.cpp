/*
 * RkAiqHandleInt.cpp
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

#include "RkAiqHandleInt.h"
#include "RkAiqCore.h"
#include "aiq_core/RkAiqSharedDataManager.h"
#include "smart_buffer_priv.h"
#include "media_buffer/media_buffer_pool.h"

namespace RkCam {

XCamReturn RkAiqHandleIntCom::configInparamsCom(RkAiqAlgoCom* com, int type)
{
    ENTER_ANALYZER_FUNCTION();

    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqAlgoComInt* rk_com = NULL;

#define GET_RK_COM(algo) \
    { \
        if (type == RKAIQ_CONFIG_COM_PREPARE) \
            rk_com = &(((RkAiqAlgoConfig##algo##Int*)com)->rk_com); \
        else if (type == RKAIQ_CONFIG_COM_PRE) \
            rk_com = &(((RkAiqAlgoPre##algo##Int*)com)->rk_com); \
        else if (type == RKAIQ_CONFIG_COM_PROC) \
            rk_com = &(((RkAiqAlgoProc##algo##Int*)com)->rk_com); \
        else if (type == RKAIQ_CONFIG_COM_POST) \
            rk_com = &(((RkAiqAlgoPost##algo##Int*)com)->rk_com); \
    } \

    switch (mDes->type) {
    case RK_AIQ_ALGO_TYPE_AE:
        GET_RK_COM(Ae);
        break;
    case RK_AIQ_ALGO_TYPE_AWB:
        GET_RK_COM(Awb);
        break;
    case RK_AIQ_ALGO_TYPE_AF:
        GET_RK_COM(Af);
        break;
    case RK_AIQ_ALGO_TYPE_ABLC:
        GET_RK_COM(Ablc);
        break;
    case RK_AIQ_ALGO_TYPE_ADPCC:
        GET_RK_COM(Adpcc);
        break;
    case RK_AIQ_ALGO_TYPE_AMERGE:
        GET_RK_COM(Amerge);
        break;
    case RK_AIQ_ALGO_TYPE_ATMO:
        GET_RK_COM(Atmo);
        break;
    case RK_AIQ_ALGO_TYPE_ANR:
        GET_RK_COM(Anr);
        break;
    case RK_AIQ_ALGO_TYPE_ALSC:
        GET_RK_COM(Alsc);
        break;
    case RK_AIQ_ALGO_TYPE_AGIC:
        GET_RK_COM(Agic);
        break;
    case RK_AIQ_ALGO_TYPE_ADEBAYER:
        GET_RK_COM(Adebayer);
        break;
    case RK_AIQ_ALGO_TYPE_ACCM:
        GET_RK_COM(Accm);
        break;
    case RK_AIQ_ALGO_TYPE_AGAMMA:
        GET_RK_COM(Agamma);
        break;
    case RK_AIQ_ALGO_TYPE_ADEGAMMA:
        GET_RK_COM(Adegamma);
        break;
    case RK_AIQ_ALGO_TYPE_AWDR:
        GET_RK_COM(Awdr);
        break;
    case RK_AIQ_ALGO_TYPE_ADHAZ:
        GET_RK_COM(Adhaz);
        break;
    case RK_AIQ_ALGO_TYPE_A3DLUT:
        GET_RK_COM(A3dlut);
        break;
    case RK_AIQ_ALGO_TYPE_ALDCH:
        GET_RK_COM(Aldch);
        break;
    case RK_AIQ_ALGO_TYPE_AR2Y:
        GET_RK_COM(Ar2y);
        break;
    case RK_AIQ_ALGO_TYPE_ACP:
        GET_RK_COM(Acp);
        break;
    case RK_AIQ_ALGO_TYPE_AIE:
        GET_RK_COM(Aie);
        break;
    case RK_AIQ_ALGO_TYPE_ASHARP:
        GET_RK_COM(Asharp);
        break;
    case RK_AIQ_ALGO_TYPE_AORB:
        GET_RK_COM(Aorb);
        break;
    case RK_AIQ_ALGO_TYPE_AFEC:
        GET_RK_COM(Afec);
        break;
    case RK_AIQ_ALGO_TYPE_ACGC:
        GET_RK_COM(Acgc);
        break;
    case RK_AIQ_ALGO_TYPE_ASD:
        GET_RK_COM(Asd);
        break;
    case RK_AIQ_ALGO_TYPE_ADRC:
        GET_RK_COM(Adrc);
        break;
    case RK_AIQ_ALGO_TYPE_AYNR:
        GET_RK_COM(Aynr);
        break;
    case RK_AIQ_ALGO_TYPE_ACNR:
        GET_RK_COM(Acnr);
        break;
    case RK_AIQ_ALGO_TYPE_ARAWNR:
        GET_RK_COM(Arawnr);
        break;
    case RK_AIQ_ALGO_TYPE_AEIS:
        GET_RK_COM(Aeis);
        break;
    case RK_AIQ_ALGO_TYPE_AMD:
        GET_RK_COM(Amd);
        break;
    case RK_AIQ_ALGO_TYPE_AMFNR:
        GET_RK_COM(Amfnr);
        break;
    case RK_AIQ_ALGO_TYPE_AGAIN:
        GET_RK_COM(Again);
        break;
    default:
        LOGE_ANALYZER("wrong algo type !");
    }

    if (!rk_com)
        goto out;

    xcam_mem_clear(*rk_com);

    if (type == RkAiqHandle::RKAIQ_CONFIG_COM_PREPARE) {
        rk_com->u.prepare.calib = (CamCalibDbContext_t*)(sharedCom->calib);
        rk_com->u.prepare.calibv2 = (CamCalibDbV2Context_t*)(sharedCom->calibv2);
    } else {
        rk_com->u.proc.pre_res_comb = &shared->preResComb;
        rk_com->u.proc.proc_res_comb = &shared->procResComb;
        rk_com->u.proc.post_res_comb = &shared->postResComb;
        rk_com->u.proc.iso = sharedCom->iso;
        rk_com->u.proc.fill_light_on = sharedCom->fill_light_on;
        rk_com->u.proc.gray_mode = sharedCom->gray_mode;
        rk_com->u.proc.is_bw_sensor = sharedCom->is_bw_sensor;
        rk_com->u.proc.preExp = &shared->preExp;
        rk_com->u.proc.curExp = &shared->curExp;
        rk_com->u.proc.res_comb = &shared->res_comb;
    }

    EXIT_ANALYZER_FUNCTION();
out:
    return RkAiqHandle::configInparamsCom(com, type);
}

void
RkAiqHandleIntCom::waitSignal()
{
    if (mAiqCore->isRunningState()) {
        mUpdateCond.timedwait(mCfgMutex, 100000);
    } else {
        updateConfig(false);
    }
}

void
RkAiqHandleIntCom::sendSignal()
{
    if (mAiqCore->isRunningState())
        mUpdateCond.signal();
}

void
RkAiqAeHandleInt::init()
{
    ENTER_ANALYZER_FUNCTION();

    RkAiqAeHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAeInt());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAeInt());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAeInt());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAeInt());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAeInt());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAeInt());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAeInt());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn
RkAiqAeHandleInt::updateConfig(bool needSync)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (needSync)
        mCfgMutex.lock();
    // if something changed, api will modify aecCfg in mAlgoCtx
    if (updateExpSwAttr) {
        mCurExpSwAttr = mNewExpSwAttr;
        updateExpSwAttr = false;
        updateAttr |= UPDATE_EXPSWATTR;

        Uapi_ExpSwAttr_t ExpSwAttrV1 = mCurExpSwAttr;
        rk_aiq_uapi_ae_convExpSwAttr_v1Tov2(&ExpSwAttrV1, &mCurExpSwAttrV2);

        rk_aiq_uapi_ae_setExpSwAttr(mAlgoCtx, &mCurExpSwAttrV2, false);
        sendSignal();
    }

    if (updateLinExpAttr) {
        mCurLinExpAttr = mNewLinExpAttr;
        updateLinExpAttr = false;
        updateAttr |= UPDATE_LINEXPATTR;

        Uapi_LinExpAttr_t LinExpAttrV1 = mCurLinExpAttr;
        rk_aiq_uapi_ae_convLinExpAttr_v1Tov2(&LinExpAttrV1, &mCurLinExpAttrV2);

        rk_aiq_uapi_ae_setLinExpAttr(mAlgoCtx, &mCurLinExpAttrV2, false);
        sendSignal();
    }

    if (updateHdrExpAttr) {
        mCurHdrExpAttr = mNewHdrExpAttr;
        updateHdrExpAttr = false;
        updateAttr |= UPDATE_HDREXPATTR;

        Uapi_HdrExpAttr_t HdrExpAttrV1 = mCurHdrExpAttr;
        rk_aiq_uapi_ae_convHdrExpAttr_v1Tov2(&HdrExpAttrV1, &mCurHdrExpAttrV2);

        rk_aiq_uapi_ae_setHdrExpAttr(mAlgoCtx, &mCurHdrExpAttrV2, false);
        sendSignal();
    }

    // TODO: update v2

    if (updateExpSwAttrV2) {
        mCurExpSwAttrV2 = mNewExpSwAttrV2;
        updateExpSwAttrV2 = false;
        updateAttr |= UPDATE_EXPSWATTR;

        rk_aiq_uapi_ae_setExpSwAttr(mAlgoCtx, &mCurExpSwAttrV2, false);
        sendSignal();
    }

    if (updateLinExpAttrV2) {
        mCurLinExpAttrV2 = mNewLinExpAttrV2;
        updateLinExpAttrV2 = false;
        updateAttr |= UPDATE_LINEXPATTR;

        rk_aiq_uapi_ae_setLinExpAttr(mAlgoCtx, &mCurLinExpAttrV2, false);
        sendSignal();
    }

    if (updateHdrExpAttrV2) {
        mCurHdrExpAttrV2 = mNewHdrExpAttrV2;
        updateHdrExpAttrV2 = false;
        updateAttr |= UPDATE_HDREXPATTR;

        rk_aiq_uapi_ae_setHdrExpAttr(mAlgoCtx, &mCurHdrExpAttrV2, false);
        sendSignal();
    }

    if (updateLinAeRouteAttr) {
        mCurLinAeRouteAttr = mNewLinAeRouteAttr;
        updateLinAeRouteAttr = false;
        updateAttr |= UPDATE_LINAEROUTEATTR;
        rk_aiq_uapi_ae_setLinAeRouteAttr(mAlgoCtx, &mCurLinAeRouteAttr, false);
        sendSignal();
    }
    if (updateHdrAeRouteAttr) {
        mCurHdrAeRouteAttr = mNewHdrAeRouteAttr;
        updateHdrAeRouteAttr = false;
        updateAttr |= UPDATE_HDRAEROUTEATTR;
        rk_aiq_uapi_ae_setHdrAeRouteAttr(mAlgoCtx, &mCurHdrAeRouteAttr, false);
        sendSignal();
    }
    if (updateIrisAttr) {
        mCurIrisAttr = mNewIrisAttr;
        updateIrisAttr = false;
        updateAttr |= UPDATE_IRISATTR;
        rk_aiq_uapi_ae_setIrisAttr(mAlgoCtx, &mCurIrisAttr, false);
        sendSignal();
    }
    if (updateSyncTestAttr) {
        mCurAecSyncTestAttr = mNewAecSyncTestAttr;
        updateSyncTestAttr = false;
        updateAttr |= UPDATE_SYNCTESTATTR;
        rk_aiq_uapi_ae_setSyncTest(mAlgoCtx, &mCurAecSyncTestAttr, false);
        sendSignal();
    }
    if (updateExpWinAttr) {
        mCurExpWinAttr = mNewExpWinAttr;
        updateExpWinAttr = false;
        updateAttr |= UPDATE_EXPWINATTR;
        rk_aiq_uapi_ae_setExpWinAttr(mAlgoCtx, &mCurExpWinAttr, false);
        sendSignal();
    }

    // once any params are changed, run reconfig to convert aecCfg to paectx
    AeInstanceConfig_t* pAeInstConfig = (AeInstanceConfig_t*)mAlgoCtx;
    AeConfig_t pAecCfg = pAeInstConfig->aecCfg;
    pAecCfg->IsReconfig |= updateAttr;
    updateAttr = 0;
    if (needSync)
        mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAeHandleInt::setExpSwAttr(Uapi_ExpSwAttr_t ExpSwAttr)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    //TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed
    if (0 != memcmp(&mCurExpSwAttr, &ExpSwAttr, sizeof(Uapi_ExpSwAttr_t))) {
        mNewExpSwAttr = ExpSwAttr;
        updateExpSwAttr = true;
        waitSignal();
    }
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
XCamReturn
RkAiqAeHandleInt::getExpSwAttr(Uapi_ExpSwAttr_t* pExpSwAttr)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    Uapi_ExpSwAttrV2_t ExpSwAttrV2;
    memset(&ExpSwAttrV2, 0x00, sizeof(Uapi_ExpSwAttrV2_t));
    rk_aiq_uapi_ae_getExpSwAttr(mAlgoCtx, &ExpSwAttrV2);
    rk_aiq_uapi_ae_convExpSwAttr_v2Tov1(&ExpSwAttrV2, pExpSwAttr);

    mCurExpSwAttrV2 = ExpSwAttrV2;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
XCamReturn
RkAiqAeHandleInt::setExpSwAttr(Uapi_ExpSwAttrV2_t ExpSwAttrV2)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    //TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed
    if (0 != memcmp(&mCurExpSwAttrV2, &ExpSwAttrV2, sizeof(Uapi_ExpSwAttrV2_t))) {
        mNewExpSwAttrV2 = ExpSwAttrV2;
        updateExpSwAttrV2 = true;
        waitSignal();
    }
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
XCamReturn
RkAiqAeHandleInt::getExpSwAttr(Uapi_ExpSwAttrV2_t* pExpSwAttrV2)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_ae_getExpSwAttr(mAlgoCtx, pExpSwAttrV2);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAeHandleInt::setLinExpAttr(Uapi_LinExpAttr_t LinExpAttr)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    //TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed
    if (0 != memcmp(&mCurLinExpAttr, &LinExpAttr, sizeof(Uapi_LinExpAttr_t))) {
        mNewLinExpAttr = LinExpAttr;
        updateLinExpAttr = true;
        waitSignal();
    }
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
XCamReturn
RkAiqAeHandleInt::getLinExpAttr(Uapi_LinExpAttr_t* pLinExpAttr)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    Uapi_LinExpAttrV2_t LinExpAttrV2;
    memset(&LinExpAttrV2, 0x00, sizeof(Uapi_LinExpAttrV2_t));

    rk_aiq_uapi_ae_getLinExpAttr(mAlgoCtx, &LinExpAttrV2);
    rk_aiq_uapi_ae_convLinExpAttr_v2Tov1(&LinExpAttrV2, pLinExpAttr);

    mCurLinExpAttrV2 = LinExpAttrV2;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
XCamReturn
RkAiqAeHandleInt::setLinExpAttr(Uapi_LinExpAttrV2_t LinExpAttrV2)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    //TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed
    if (0 != memcmp(&mCurLinExpAttrV2, &LinExpAttrV2, sizeof(Uapi_LinExpAttrV2_t))) {
        mNewLinExpAttrV2 = LinExpAttrV2;
        updateLinExpAttrV2 = true;
        waitSignal();
    }
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
XCamReturn
RkAiqAeHandleInt::getLinExpAttr(Uapi_LinExpAttrV2_t* pLinExpAttrV2)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_ae_getLinExpAttr(mAlgoCtx, pLinExpAttrV2);

    EXIT_ANALYZER_FUNCTION();
    return ret;

}

XCamReturn
RkAiqAeHandleInt::setHdrExpAttr(Uapi_HdrExpAttr_t HdrExpAttr)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    //TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed
    if (0 != memcmp(&mCurHdrExpAttr, &HdrExpAttr, sizeof(Uapi_HdrExpAttr_t))) {
        mNewHdrExpAttr = HdrExpAttr;
        updateHdrExpAttr = true;
        waitSignal();
    }
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
XCamReturn
RkAiqAeHandleInt::getHdrExpAttr (Uapi_HdrExpAttr_t* pHdrExpAttr)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    Uapi_HdrExpAttrV2_t HdrExpAttrV2;
    memset(&HdrExpAttrV2, 0x00, sizeof(Uapi_HdrExpAttrV2_t));

    rk_aiq_uapi_ae_getHdrExpAttr(mAlgoCtx, &HdrExpAttrV2);
    rk_aiq_uapi_ae_convHdrExpAttr_v2Tov1(&HdrExpAttrV2, pHdrExpAttr);

    mCurHdrExpAttrV2 = HdrExpAttrV2;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
XCamReturn
RkAiqAeHandleInt::setHdrExpAttr(Uapi_HdrExpAttrV2_t HdrExpAttrV2)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    //TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed
    if (0 != memcmp(&mCurHdrExpAttrV2, &HdrExpAttrV2, sizeof(Uapi_HdrExpAttrV2_t))) {
        mNewHdrExpAttrV2 = HdrExpAttrV2;
        updateHdrExpAttrV2 = true;
        waitSignal();
    }
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
XCamReturn
RkAiqAeHandleInt::getHdrExpAttr (Uapi_HdrExpAttrV2_t* pHdrExpAttrV2)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_ae_getHdrExpAttr(mAlgoCtx, pHdrExpAttrV2);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAeHandleInt::setLinAeRouteAttr(Uapi_LinAeRouteAttr_t LinAeRouteAttr)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    //TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed

    if (0 != memcmp(&mCurLinAeRouteAttr, &LinAeRouteAttr, sizeof(Uapi_LinAeRouteAttr_t))) {
        mNewLinAeRouteAttr = LinAeRouteAttr;
        updateLinAeRouteAttr = true;
        waitSignal();
    }
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
XCamReturn
RkAiqAeHandleInt::getLinAeRouteAttr(Uapi_LinAeRouteAttr_t* pLinAeRouteAttr)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_ae_getLinAeRouteAttr(mAlgoCtx, pLinAeRouteAttr);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
XCamReturn
RkAiqAeHandleInt::setHdrAeRouteAttr(Uapi_HdrAeRouteAttr_t HdrAeRouteAttr)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    //TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed
    if (0 != memcmp(&mCurHdrAeRouteAttr, &HdrAeRouteAttr, sizeof(Uapi_HdrAeRouteAttr_t))) {
        mNewHdrAeRouteAttr = HdrAeRouteAttr;
        updateHdrAeRouteAttr = true;
        waitSignal();
    }
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
XCamReturn
RkAiqAeHandleInt::getHdrAeRouteAttr(Uapi_HdrAeRouteAttr_t* pHdrAeRouteAttr)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_ae_getHdrAeRouteAttr(mAlgoCtx, pHdrAeRouteAttr);

    EXIT_ANALYZER_FUNCTION();
    return ret;

}

XCamReturn
RkAiqAeHandleInt::setIrisAttr(Uapi_IrisAttrV2_t IrisAttr)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    //TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed

    if (0 != memcmp(&mCurIrisAttr, &IrisAttr, sizeof(Uapi_IrisAttrV2_t))) {
        mNewIrisAttr = IrisAttr;
        updateIrisAttr = true;
        waitSignal();
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
XCamReturn
RkAiqAeHandleInt::getIrisAttr(Uapi_IrisAttrV2_t * pIrisAttr)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_ae_getIrisAttr(mAlgoCtx, pIrisAttr);

    EXIT_ANALYZER_FUNCTION();
    return ret;

}

XCamReturn
RkAiqAeHandleInt::setSyncTestAttr(Uapi_AecSyncTest_t SyncTestAttr)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    //TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed

    if (0 != memcmp(&mCurAecSyncTestAttr, &SyncTestAttr, sizeof(Uapi_AecSyncTest_t))) {
        mNewAecSyncTestAttr = SyncTestAttr;
        updateSyncTestAttr = true;
        waitSignal();
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
XCamReturn
RkAiqAeHandleInt::getSyncTestAttr(Uapi_AecSyncTest_t * pSyncTestAttr)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_ae_getSyncTest(mAlgoCtx, pSyncTestAttr);

    EXIT_ANALYZER_FUNCTION();
    return ret;

}

XCamReturn
RkAiqAeHandleInt::setExpWinAttr(Uapi_ExpWin_t ExpWinAttr)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    //TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed

    if (0 != memcmp(&mCurExpWinAttr, &ExpWinAttr, sizeof(Uapi_ExpWin_t))) {
        mNewExpWinAttr = ExpWinAttr;
        updateExpWinAttr = true;
        waitSignal();
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
XCamReturn
RkAiqAeHandleInt::getExpWinAttr(Uapi_ExpWin_t * pExpWinAttr)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_ae_getExpWinAttr(mAlgoCtx, pExpWinAttr);

    EXIT_ANALYZER_FUNCTION();
    return ret;

}
XCamReturn
RkAiqAeHandleInt::queryExpInfo(Uapi_ExpQueryInfo_t* pExpQueryInfo)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_ae_queryExpInfo(mAlgoCtx, pExpQueryInfo);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
XCamReturn
RkAiqAeHandleInt::prepare()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqAeHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "ae handle prepare failed");

    RkAiqAlgoConfigAeInt* ae_config_int = (RkAiqAlgoConfigAeInt*)mConfig;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    /*****************AecConfig pic-info params*****************/
    ae_config_int->RawWidth = sharedCom->snsDes.isp_acq_width;
    ae_config_int->RawHeight = sharedCom->snsDes.isp_acq_height;
    ae_config_int->nr_switch = sharedCom->snsDes.nr_switch;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "ae algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqAeHandleInt::preProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPreAeInt* ae_pre_int = (RkAiqAlgoPreAeInt*)mPreInParam;
    RkAiqAlgoPreResAeInt* ae_pre_res_int = nullptr;
    if (mAiqCore->mAlogsComSharedParams.init) {
        ae_pre_res_int = (RkAiqAlgoPreResAeInt*)mPreOutParam;
    } else {
        mPreResShared = new RkAiqAlgoPreResAeIntShared();
        if (!mPreResShared.ptr()) {
            LOGE("new ae mPreOutParam failed, bypass!");
            return XCAM_RETURN_BYPASS;
        }
        ae_pre_res_int = &mPreResShared->result;
    }

    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;
    RkAiqPreResComb* comb = &shared->preResComb;

    ret = RkAiqAeHandle::preProcess();
    if (ret) {
        comb->ae_pre_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "ae handle preProcess failed");
    }

    comb->ae_pre_res = NULL;

    ae_pre_int->aecStatsBuf = shared->aecStatsBuf;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    if (mAiqCore->mAlogsComSharedParams.init)
        ret = des->pre_process(mPreInParam, mPreOutParam);
    else
        ret = des->pre_process(mPreInParam, (RkAiqAlgoResCom*)ae_pre_res_int);
    RKAIQCORE_CHECK_RET(ret, "ae algo pre_process failed");

    // set result to mAiqCore
    comb->ae_pre_res = (RkAiqAlgoPreResAe*)ae_pre_res_int;

    if (!mAiqCore->mAlogsComSharedParams.init) {
        mPreResShared->set_sequence(shared->frameId);
        SmartPtr<XCamMessage> msg = new RkAiqCoreVdBufMsg(XCAM_MESSAGE_AE_PRE_RES_OK,
                shared->frameId, mPreResShared);
        mAiqCore->post_message(msg);
    }

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqAeHandleInt::processing()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAeInt* ae_proc_int = (RkAiqAlgoProcAeInt*)mProcInParam;
    RkAiqAlgoProcResAeInt* ae_proc_res_int = nullptr;
    if (mAiqCore->mAlogsComSharedParams.init) {
        ae_proc_res_int = (RkAiqAlgoProcResAeInt*)mProcOutParam;
    } else {
        mProcResShared = new RkAiqAlgoProcResAeIntShared();
        if (!mProcResShared.ptr()) {
            LOGE("new ae mProcOutParam failed, bypass!");
            return XCAM_RETURN_BYPASS;
        }
        ae_proc_res_int = &mProcResShared->result;
        // mProcOutParam = (RkAiqAlgoResCom*)ae_proc_res_int;
    }

    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqProcResComb* comb = &shared->procResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAeHandle::processing();
    if (ret) {
        comb->ae_proc_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "ae handle processing failed");
    }

    comb->ae_proc_res = NULL;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    if (mAiqCore->mAlogsComSharedParams.init)
        ret = des->processing(mProcInParam, mProcOutParam);
    else
        ret = des->processing(mProcInParam, (RkAiqAlgoResCom*)ae_proc_res_int);
    RKAIQCORE_CHECK_RET(ret, "ae algo processing failed");

    comb->ae_proc_res = (RkAiqAlgoProcResAe*)ae_proc_res_int;

    if (mAiqCore->mAlogsComSharedParams.init) {
        RkAiqCore::RkAiqAlgosGroupShared_t* measGroupshared = nullptr;
        if (mAiqCore->getGroupSharedParams(RK_AIQ_CORE_ANALYZE_MEAS, measGroupshared) != XCAM_RETURN_NO_ERROR)
            LOGW("get the shared of meas failed");
        if (measGroupshared) {
            measGroupshared->frameId = shared->frameId;
            measGroupshared->preResComb.ae_pre_res = shared->preResComb.ae_pre_res;
            measGroupshared->procResComb.ae_proc_res = shared->procResComb.ae_proc_res;
            measGroupshared->postResComb.ae_post_res = shared->postResComb.ae_post_res;
        }
    } else {
        mProcResShared->set_sequence(shared->frameId);
        SmartPtr<XCamMessage> msg = new RkAiqCoreVdBufMsg(XCAM_MESSAGE_AE_PROC_RES_OK,
                shared->frameId, mProcResShared);
        mAiqCore->post_message(msg);
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAeHandleInt::postProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPostAeInt* ae_post_int = (RkAiqAlgoPostAeInt*)mPostInParam;
    RkAiqAlgoPostResAeInt* ae_post_res_int = (RkAiqAlgoPostResAeInt*)mPostOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqPostResComb* comb = &shared->postResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAeHandle::postProcess();
    if (ret) {
        comb->ae_post_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "ae handle postProcess failed");
        return ret;
    }

    comb->ae_post_res = NULL;
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->post_process(mPostInParam, mPostOutParam);
    RKAIQCORE_CHECK_RET(ret, "ae algo post_process failed");
    // set result to mAiqCore
    comb->ae_post_res = (RkAiqAlgoPostResAe*)ae_post_res_int ;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

void
RkAiqAwbHandleInt::init()
{
    ENTER_ANALYZER_FUNCTION();

    RkAiqAwbHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAwbInt());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAwbInt());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAwbInt());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAwbInt());
    // mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAwbInt());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAwbInt());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAwbInt());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn
RkAiqAwbHandleInt::updateConfig(bool needSync)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (needSync)
        mCfgMutex.lock();
    // if something changed
    if (updateAtt) {
        mCurAtt = mNewAtt;
        updateAtt = false;
        rk_aiq_uapi_awb_SetAttrib(mAlgoCtx, mCurAtt, false);
        sendSignal();
    }
    if (updateWbV20Attr) {
        mCurWbV20Attr = mNewWbV20Attr;
        updateWbV20Attr = false;
        rk_aiq_uapiV2_awbV20_SetAttrib(mAlgoCtx, mCurWbV20Attr, false);
        sendSignal();
    }
    if (updateWbOpModeAttr) {
        mCurWbOpModeAttr = mNewWbOpModeAttr;
        updateWbOpModeAttr = false;
        rk_aiq_uapiV2_awb_SetMwbMode(mAlgoCtx, mCurWbOpModeAttr, false);
        sendSignal();
    }
    if (updateWbMwbAttr) {
        mCurWbMwbAttr = mNewWbMwbAttr;
        updateWbMwbAttr = false;
        rk_aiq_uapiV2_awb_SetMwbAttrib(mAlgoCtx, mCurWbMwbAttr, false);
        sendSignal();
    }
    if (updateWbAwbAttr) {
        mCurWbAwbAttr = mNewWbAwbAttr;
        updateWbAwbAttr = false;
        rk_aiq_uapiV2_awbV20_SetAwbAttrib(mAlgoCtx, mCurWbAwbAttr, false);
        sendSignal();
    }
    if (updateWbAwbWbGainAdjustAttr) {
        mCurWbAwbWbGainAdjustAttr = mNewWbAwbWbGainAdjustAttr;
        updateWbAwbWbGainAdjustAttr = false;
        rk_aiq_uapiV2_awb_SetAwbGainAdjust(mAlgoCtx, mCurWbAwbWbGainAdjustAttr, false);
        sendSignal();
    }
    if (updateWbAwbWbGainOffsetAttr) {
        mCurWbAwbWbGainOffsetAttr = mNewWbAwbWbGainOffsetAttr;
        updateWbAwbWbGainOffsetAttr = false;
        rk_aiq_uapiV2_awb_SetAwbGainOffset(mAlgoCtx, mCurWbAwbWbGainOffsetAttr, false);
        sendSignal();
    }
    if (updateWbAwbMultiWindowAttr) {
        mCurWbAwbMultiWindowAttr = mNewWbAwbMultiWindowAttr;
        updateWbAwbMultiWindowAttr = false;
        rk_aiq_uapiV2_awb_SetAwbMultiwindow(mAlgoCtx, mCurWbAwbMultiWindowAttr, false);
        sendSignal();
    }
    if (needSync)
        mCfgMutex.unlock();


    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAwbHandleInt::setAttrib(rk_aiq_wb_attrib_t att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    //TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed
    if (0 != memcmp(&mCurAtt, &att, sizeof(rk_aiq_wb_attrib_t))) {
        mNewAtt = att;
        updateAtt = true;
        waitSignal();
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAwbHandleInt::getAttrib(rk_aiq_wb_attrib_t *att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_awb_GetAttrib(mAlgoCtx, att);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAwbHandleInt::getCct(rk_aiq_wb_cct_t *cct)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapiV2_awb_GetCCT(mAlgoCtx, cct);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAwbHandleInt::queryWBInfo(rk_aiq_wb_querry_info_t *wb_querry_info )
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapiV2_awb_QueryWBInfo(mAlgoCtx, wb_querry_info);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAwbHandleInt::lock()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapiV2_awb_Lock(mAlgoCtx);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAwbHandleInt::unlock()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapiV2_awb_Unlock(mAlgoCtx);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}


XCamReturn
RkAiqAwbHandleInt::setWbV20Attrib(rk_aiq_uapiV2_wbV20_attrib_t att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    //TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed
    if (0 != memcmp(&mCurWbV20Attr, &att, sizeof(rk_aiq_uapiV2_wbV20_attrib_t))) {
        mNewWbV20Attr = att;
        updateWbV20Attr = true;
        waitSignal();
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAwbHandleInt::getWbV20Attrib(rk_aiq_uapiV2_wbV20_attrib_t *att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapiV2_awbV20_GetAttrib(mAlgoCtx, att);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAwbHandleInt::setWbOpModeAttrib(rk_aiq_wb_op_mode_t att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    //TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed
    if (0 != memcmp(&mCurWbOpModeAttr, &att, sizeof(rk_aiq_wb_op_mode_t))) {
        mNewWbOpModeAttr = att;
        updateWbOpModeAttr = true;
        waitSignal();
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAwbHandleInt::getWbOpModeAttrib(rk_aiq_wb_op_mode_t *att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapiV2_awb_GetMwbMode(mAlgoCtx, att);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAwbHandleInt::setMwbAttrib(rk_aiq_wb_mwb_attrib_t att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    //TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed
    if (0 != memcmp(&mCurWbMwbAttr, &att, sizeof(rk_aiq_wb_mwb_attrib_t))) {
        mNewWbMwbAttr = att;
        updateWbMwbAttr = true;
        waitSignal();
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAwbHandleInt::getMwbAttrib(rk_aiq_wb_mwb_attrib_t *att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapiV2_awb_GetMwbAttrib(mAlgoCtx, att);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}


XCamReturn
RkAiqAwbHandleInt::setAwbV20Attrib(rk_aiq_uapiV2_wbV20_awb_attrib_t att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    //TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed
    if (0 != memcmp(&mCurWbAwbAttr, &att, sizeof(rk_aiq_uapiV2_wbV20_awb_attrib_t))) {
        mNewWbAwbAttr = att;
        updateWbAwbAttr = true;
        waitSignal();
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAwbHandleInt::getAwbV20Attrib(rk_aiq_uapiV2_wbV20_awb_attrib_t *att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapiV2_awbV20_GetAwbAttrib(mAlgoCtx, att);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAwbHandleInt::setWbAwbWbGainAdjustAttrib(rk_aiq_uapiV2_wb_awb_wbGainAdjust_t att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    //TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed
    if (0 != memcmp(&mCurWbAwbWbGainAdjustAttr, &att, sizeof(rk_aiq_uapiV2_wb_awb_wbGainAdjust_t))) {
        mNewWbAwbWbGainAdjustAttr = att;
        updateWbAwbWbGainAdjustAttr = true;
        waitSignal();
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAwbHandleInt::getWbAwbWbGainAdjustAttrib(rk_aiq_uapiV2_wb_awb_wbGainAdjust_t *att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapiV2_awb_GetAwbGainAdjust(mAlgoCtx, att);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}


XCamReturn
RkAiqAwbHandleInt::setWbAwbWbGainOffsetAttrib(CalibDbV2_Awb_gain_offset_cfg_t att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    //TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed
    if (0 != memcmp(&mCurWbAwbWbGainOffsetAttr, &att, sizeof(CalibDbV2_Awb_gain_offset_cfg_t))) {
        mNewWbAwbWbGainOffsetAttr = att;
        updateWbAwbWbGainOffsetAttr = true;
        waitSignal();
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAwbHandleInt::getWbAwbWbGainOffsetAttrib(CalibDbV2_Awb_gain_offset_cfg_t *att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapiV2_awb_GetAwbGainOffset(mAlgoCtx, att);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAwbHandleInt::setWbAwbMultiWindowAttrib(CalibDbV2_Awb_Mul_Win_t att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    //TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed
    if (0 != memcmp(&mCurWbAwbMultiWindowAttr, &att, sizeof(CalibDbV2_Awb_Mul_Win_t))) {
        mNewWbAwbMultiWindowAttr = att;
        updateWbAwbMultiWindowAttr = true;
        waitSignal();
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAwbHandleInt::getWbAwbMultiWindowAttrib(CalibDbV2_Awb_Mul_Win_t *att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapiV2_awb_GetAwbMultiwindow(mAlgoCtx, att);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAwbHandleInt::prepare()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqAwbHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "awb handle prepare failed");

    RkAiqAlgoConfigAwbInt* awb_config_int = (RkAiqAlgoConfigAwbInt*)mConfig;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    // TODO
    //awb_config_int->rawBit;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "awb algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAwbHandleInt::preProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPreAwbInt* awb_pre_int = (RkAiqAlgoPreAwbInt*)mPreInParam;
    RkAiqAlgoPreResAwbInt* awb_pre_res_int = (RkAiqAlgoPreResAwbInt*)mPreOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqPreResComb* comb = &shared->preResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAwbHandle::preProcess();
    if (ret) {
        comb->awb_pre_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "awb handle preProcess failed");
    }
    comb->awb_pre_res = NULL;
    int module_hw_version = sharedCom->ctxCfigs[RK_AIQ_ALGO_TYPE_AWB].cfg_com.module_hw_version;
#ifdef RK_SIMULATOR_HW
    if(module_hw_version == AWB_HARDWARE_V200) {
        awb_pre_int->awb_hw0_statis = ispStats->awb_stats;
        awb_pre_int->awb_cfg_effect_v200 = ispStats->awb_cfg_effect_v200;
    } else {
        awb_pre_int->awb_hw1_statis = ispStats->awb_stats_v201;
        awb_pre_int->awb_cfg_effect_v201 = ispStats->awb_cfg_effect_v201;
    }
#else
    awb_pre_int->awbStatsBuf = shared->awbStatsBuf;
#endif
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->pre_process(mPreInParam, mPreOutParam);
    RKAIQCORE_CHECK_RET(ret, "awb algo pre_process failed");
    // set result to mAiqCore
    comb->awb_pre_res = (RkAiqAlgoPreResAwb*)awb_pre_res_int;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAwbHandleInt::processing()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAwbInt* awb_proc_int = (RkAiqAlgoProcAwbInt*)mProcInParam;
#if 0
    RkAiqAlgoProcResAwbInt* awb_proc_res_int = (RkAiqAlgoProcResAwbInt*)mProcOutParam;
#else
    mProcResShared = new RkAiqAlgoProcResAwbIntShared();
    if (!mProcResShared.ptr()) {
        LOGE("new awb mProcOutParam failed, bypass!");
        return XCAM_RETURN_BYPASS;
    }
    RkAiqAlgoProcResAwbInt* awb_proc_res_int = &mProcResShared->result;
    // mProcOutParam = (RkAiqAlgoResCom*)awb_proc_res_int;
#endif
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqProcResComb* comb = &shared->procResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAwbHandle::processing();
    if (ret) {
        comb->awb_proc_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "awb handle processing failed");
    }

    comb->awb_proc_res = NULL;


    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
#if 0
    ret = des->processing(mProcInParam, mProcOutParam);
#else
    ret = des->processing(mProcInParam, (RkAiqAlgoResCom*)awb_proc_res_int);
#endif
    RKAIQCORE_CHECK_RET(ret, "awb algo processing failed");
    // set result to mAiqCore
    comb->awb_proc_res = (RkAiqAlgoProcResAwb*)awb_proc_res_int;

    mProcResShared->set_sequence(shared->frameId);
    SmartPtr<XCamMessage> msg = new RkAiqCoreVdBufMsg(XCAM_MESSAGE_AWB_PROC_RES_OK,
            shared->frameId, mProcResShared);
    mAiqCore->post_message(msg);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAwbHandleInt::postProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPostAwbInt* awb_post_int = (RkAiqAlgoPostAwbInt*)mPostInParam;
    RkAiqAlgoPostResAwbInt* awb_post_res_int = (RkAiqAlgoPostResAwbInt*)mPostOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqPostResComb* comb = &shared->postResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAwbHandle::postProcess();
    if (ret) {
        comb->awb_post_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "awb handle postProcess failed");
        return ret;
    }

    comb->awb_post_res = NULL;
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->post_process(mPostInParam, mPostOutParam);
    RKAIQCORE_CHECK_RET(ret, "awb algo post_process failed");
    // set result to mAiqCore
    comb->awb_post_res = (RkAiqAlgoPostResAwb*)awb_post_res_int ;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

void
RkAiqAfHandleInt::init()
{
    ENTER_ANALYZER_FUNCTION();

    RkAiqAfHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAfInt());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAfInt());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAfInt());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAfInt());
#if 0
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAfInt());
#endif
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAfInt());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAfInt());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn
RkAiqAfHandleInt::updateConfig(bool needSync)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    if (sharedCom->snsDes.lens_des.focus_support) {
        if (needSync)
            mCfgMutex.lock();
        // if something changed
        if (updateAtt) {
            rk_aiq_uapi_af_SetAttrib(mAlgoCtx, mNewAtt, false);
            isUpdateAttDone = true;
        }
        if (needSync)
            mCfgMutex.unlock();
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAfHandleInt::setAttrib(rk_aiq_af_attrib_t *att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    if (sharedCom->snsDes.lens_des.focus_support) {
        mCfgMutex.lock();
        //TODO
        // check if there is different between att & mCurAtt
        // if something changed, set att to mNewAtt, and
        // the new params will be effective later when updateConfig
        // called by RkAiqCore

        // if something changed
        if ((0 != memcmp(&mCurAtt, att, sizeof(rk_aiq_af_attrib_t))) ||
            (mCurAtt.AfMode == RKAIQ_AF_MODE_AUTO)) {
            mNewAtt = *att;
            updateAtt = true;
            isUpdateAttDone = false;
            waitSignal();
        }

        mCfgMutex.unlock();
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAfHandleInt::getAttrib(rk_aiq_af_attrib_t *att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_af_GetAttrib(mAlgoCtx, att);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAfHandleInt::lock()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    if (sharedCom->snsDes.lens_des.focus_support)
        rk_aiq_uapi_af_Lock(mAlgoCtx);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAfHandleInt::unlock()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    if (sharedCom->snsDes.lens_des.focus_support)
        rk_aiq_uapi_af_Unlock(mAlgoCtx);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAfHandleInt::Oneshot()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    if (sharedCom->snsDes.lens_des.focus_support)
        rk_aiq_uapi_af_Oneshot(mAlgoCtx);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAfHandleInt::ManualTriger()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    if (sharedCom->snsDes.lens_des.focus_support)
        rk_aiq_uapi_af_ManualTriger(mAlgoCtx);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAfHandleInt::Tracking()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    if (sharedCom->snsDes.lens_des.focus_support)
        rk_aiq_uapi_af_Tracking(mAlgoCtx);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAfHandleInt::setZoomPos(int zoom_pos)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    if (sharedCom->snsDes.lens_des.focus_support)
        rk_aiq_uapi_af_setZoomPos(mAlgoCtx, zoom_pos);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAfHandleInt::GetSearchPath(rk_aiq_af_sec_path_t* path)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    if (sharedCom->snsDes.lens_des.focus_support)
        rk_aiq_uapi_af_getSearchPath(mAlgoCtx, path);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAfHandleInt::GetSearchResult(rk_aiq_af_result_t* result)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    if (sharedCom->snsDes.lens_des.focus_support)
        rk_aiq_uapi_af_getSearchResult(mAlgoCtx, result);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAfHandleInt::prepare()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqAfHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "af handle prepare failed");

    RkAiqAlgoConfigAfInt* af_config_int = (RkAiqAlgoConfigAfInt*)mConfig;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    af_config_int->af_config_com.af_mode = 6;
    af_config_int->af_config_com.win_h_offs = 0;
    af_config_int->af_config_com.win_v_offs = 0;
    af_config_int->af_config_com.win_h_size = 0;
    af_config_int->af_config_com.win_v_size = 0;
    af_config_int->af_config_com.lens_des = sharedCom->snsDes.lens_des;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "af algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqAfHandleInt::preProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPreAfInt* af_pre_int = (RkAiqAlgoPreAfInt*)mPreInParam;
    RkAiqAlgoPreResAfInt* af_pre_res_int = (RkAiqAlgoPreResAfInt*)mPreOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqPreResComb* comb = &shared->preResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAfHandle::preProcess();
    if (ret) {
        comb->af_pre_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "af handle preProcess failed");
    }

    comb->af_pre_res = NULL;
    af_pre_int->af_stats = &ispStats->af_stats;
    af_pre_int->aec_stats = &ispStats->aec_stats;

    af_pre_int->xcam_af_stats = shared->afStatsBuf;
    af_pre_int->xcam_aec_stats = shared->aecStatsBuf;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->pre_process(mPreInParam, mPreOutParam);
    RKAIQCORE_CHECK_RET(ret, "af algo pre_process failed");
    // set result to mAiqCore
    comb->af_pre_res = (RkAiqAlgoPreResAf*)af_pre_res_int;

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqAfHandleInt::processing()
{
    ENTER_ANALYZER_FUNCTION();


    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAfInt* af_proc_int = (RkAiqAlgoProcAfInt*)mProcInParam;
#if 0
    RkAiqAlgoProcResAfInt* af_proc_res_int = (RkAiqAlgoProcResAfInt*)mProcOutParam;
#else
    mProcResShared = new RkAiqAlgoProcResAfIntShared();
    if (!mProcResShared.ptr()) {
        LOGE("new af mProcOutParam failed, bypass!");
        return XCAM_RETURN_BYPASS;
    }
    RkAiqAlgoProcResAfInt* af_proc_res_int = &mProcResShared->result;
    // mProcOutParam = (RkAiqAlgoResCom*)af_proc_res_int;
#endif
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqProcResComb* comb = &shared->procResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAfHandle::processing();
    if (ret) {
        comb->af_proc_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "af handle processing failed");
    }

    comb->af_proc_res = NULL;

    af_proc_int->af_proc_com.xcam_af_stats = shared->afStatsBuf;
    af_proc_int->af_proc_com.xcam_aec_stats = shared->aecStatsBuf;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
#if 0
    ret = des->processing(mProcInParam, mProcOutParam);
#else
    ret = des->processing(mProcInParam, (RkAiqAlgoResCom*)af_proc_res_int);
#endif
    RKAIQCORE_CHECK_RET(ret, "af algo processing failed");

    comb->af_proc_res = (RkAiqAlgoProcResAf*)af_proc_res_int;

#if 1
    af_proc_res_int->id = shared->frameId;
    SmartPtr<XCamMessage> msg = new RkAiqCoreVdBufMsg(XCAM_MESSAGE_AF_PROC_RES_OK,
            af_proc_res_int->id, mProcResShared);
    mAiqCore->post_message(msg);
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAfHandleInt::postProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPostAfInt* af_post_int = (RkAiqAlgoPostAfInt*)mPostInParam;
    RkAiqAlgoPostResAfInt* af_post_res_int = (RkAiqAlgoPostResAfInt*)mPostOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqPostResComb* comb = &shared->postResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAfHandle::postProcess();
    if (ret) {
        comb->af_post_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "af handle postProcess failed");
        return ret;
    }

    comb->af_post_res = NULL;
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->post_process(mPostInParam, mPostOutParam);
    RKAIQCORE_CHECK_RET(ret, "af algo post_process failed");
    // set result to mAiqCore
    comb->af_post_res = (RkAiqAlgoPostResAf*)af_post_res_int ;

    if (updateAtt && isUpdateAttDone) {
        mCurAtt = mNewAtt;
        updateAtt = false;
        isUpdateAttDone = false;
        sendSignal();
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

void
RkAiqAnrHandleInt::init()
{
    ENTER_ANALYZER_FUNCTION();

    RkAiqAnrHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAnrInt());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAnrInt());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAnrInt());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAnrInt());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAnrInt());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAnrInt());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAnrInt());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn
RkAiqAnrHandleInt::updateConfig(bool needSync)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (needSync)
        mCfgMutex.lock();
    // if something changed
    if (updateAtt) {
        mCurAtt = mNewAtt;
        updateAtt = false;
        // TODO
        rk_aiq_uapi_anr_SetAttrib(mAlgoCtx, &mCurAtt, false);
        sendSignal();
    }

    if(UpdateIQpara) {
        mCurIQpara = mNewIQpara;
        UpdateIQpara = false;
        rk_aiq_uapi_anr_SetIQPara(mAlgoCtx, &mCurIQpara, false);
        sendSignal();
    }

    if (needSync)
        mCfgMutex.unlock();


    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAnrHandleInt::setAttrib(rk_aiq_nr_attrib_t *att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    //TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed
    if (0 != memcmp(&mCurAtt, att, sizeof(rk_aiq_nr_attrib_t))) {
        RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
        CalibDbV2_MFNR_t* mfnr =
            (CalibDbV2_MFNR_t*)CALIBDBV2_GET_MODULE_PTR((void*)(sharedCom->calibv2), mfnr_v1);
        if (mfnr && mfnr->TuningPara.enable && mfnr->TuningPara.motion_detect_en) {
            if ((att->eMode == ANR_OP_MODE_AUTO) && (!att->stAuto.mfnrEn)) {
                att->stAuto.mfnrEn = !att->stAuto.mfnrEn;
                LOGE("motion detect is running, operate not permit!");
                goto EXIT;
            } else if ((att->eMode == ANR_OP_MODE_MANUAL) && (!att->stManual.mfnrEn)) {
                att->stManual.mfnrEn = !att->stManual.mfnrEn;
                LOGE("motion detect is running, operate not permit!");
                goto EXIT;
            }
        }
        mNewAtt = *att;
        updateAtt = true;
        waitSignal();
    }
EXIT:
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAnrHandleInt::getAttrib(rk_aiq_nr_attrib_t *att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = rk_aiq_uapi_anr_GetAttrib(mAlgoCtx, att);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAnrHandleInt::setIQPara(rk_aiq_nr_IQPara_t *para)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    //TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed
    if (0 != memcmp(&mCurIQpara, para, sizeof(rk_aiq_nr_IQPara_t))) {
        RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
        CalibDbV2_MFNR_t* mfnr =
            (CalibDbV2_MFNR_t*)CALIBDBV2_GET_MODULE_PTR((void*)(sharedCom->calibv2), mfnr_v1);
        if (mfnr && mfnr->TuningPara.enable && mfnr->TuningPara.motion_detect_en) {
            if((para->module_bits & (1 << ANR_MODULE_MFNR)) && !para->stMfnrPara.enable) {
                para->stMfnrPara.enable = !para->stMfnrPara.enable;
                LOGE("motion detect is running, disable mfnr is not permit!");
            }
        }
        mNewIQpara = *para;
        UpdateIQpara = true;
        waitSignal();
    }
EXIT:
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAnrHandleInt::getIQPara(rk_aiq_nr_IQPara_t *para)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = rk_aiq_uapi_anr_GetIQPara(mAlgoCtx, para);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}


XCamReturn
RkAiqAnrHandleInt::setLumaSFStrength(float fPercent)
{
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ret = rk_aiq_uapi_anr_SetLumaSFStrength(mAlgoCtx, fPercent);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAnrHandleInt::setLumaTFStrength(float fPercent)
{
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ret = rk_aiq_uapi_anr_SetLumaTFStrength(mAlgoCtx, fPercent);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAnrHandleInt::getLumaSFStrength(float *pPercent)
{
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ret = rk_aiq_uapi_anr_GetLumaSFStrength(mAlgoCtx, pPercent);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAnrHandleInt::getLumaTFStrength(float *pPercent)
{
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ret = rk_aiq_uapi_anr_GetLumaTFStrength(mAlgoCtx, pPercent);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAnrHandleInt::setChromaSFStrength(float fPercent)
{
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ret = rk_aiq_uapi_anr_SetChromaSFStrength(mAlgoCtx, fPercent);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAnrHandleInt::setChromaTFStrength(float fPercent)
{
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ret = rk_aiq_uapi_anr_SetChromaTFStrength(mAlgoCtx, fPercent);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAnrHandleInt::getChromaSFStrength(float *pPercent)
{
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ret = rk_aiq_uapi_anr_GetChromaSFStrength(mAlgoCtx, pPercent);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAnrHandleInt::getChromaTFStrength(float *pPercent)
{
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ret = rk_aiq_uapi_anr_GetChromaTFStrength(mAlgoCtx, pPercent);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAnrHandleInt::setRawnrSFStrength(float fPercent)
{
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ret = rk_aiq_uapi_anr_SetRawnrSFStrength(mAlgoCtx, fPercent);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAnrHandleInt::getRawnrSFStrength(float *pPercent)
{
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ret = rk_aiq_uapi_anr_GetRawnrSFStrength(mAlgoCtx, pPercent);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAnrHandleInt::prepare()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqAnrHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "anr handle prepare failed");

    RkAiqAlgoConfigAnrInt* anr_config_int = (RkAiqAlgoConfigAnrInt*)mConfig;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "anr algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqAnrHandleInt::preProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPreAnrInt* anr_pre_int = (RkAiqAlgoPreAnrInt*)mPreInParam;
    RkAiqAlgoPreResAnrInt* anr_pre_res_int = (RkAiqAlgoPreResAnrInt*)mPreOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqPreResComb* comb = &shared->preResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAnrHandle::preProcess();
    if (ret) {
        comb->anr_pre_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "anr handle preProcess failed");
    }

    comb->anr_pre_res = NULL;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->pre_process(mPreInParam, mPreOutParam);
    RKAIQCORE_CHECK_RET(ret, "anr algo pre_process failed");

    // set result to mAiqCore
    comb->anr_pre_res = (RkAiqAlgoPreResAnr*)anr_pre_res_int;

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqAnrHandleInt::processing()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAnrInt* anr_proc_int = (RkAiqAlgoProcAnrInt*)mProcInParam;
    RkAiqAlgoProcResAnrInt* anr_proc_res_int = (RkAiqAlgoProcResAnrInt*)mProcOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqProcResComb* comb = &shared->procResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;
    static int anr_proc_framecnt = 0;
    anr_proc_framecnt++;

    ret = RkAiqAnrHandle::processing();
    if (ret) {
        comb->anr_proc_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "anr handle processing failed");
    }
    comb->anr_proc_res = NULL;

    // TODO: fill procParam
    anr_proc_int->iso = sharedCom->iso;

    anr_proc_int->hdr_mode = sharedCom->working_mode;

    LOGD("%s:%d anr hdr_mode:%d  \n", __FUNCTION__, __LINE__, anr_proc_int->hdr_mode);


    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->processing(mProcInParam, mProcOutParam);
    RKAIQCORE_CHECK_RET(ret, "anr algo processing failed");

    comb->anr_proc_res = (RkAiqAlgoProcResAnr*)anr_proc_res_int;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAnrHandleInt::postProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPostAnrInt* anr_post_int = (RkAiqAlgoPostAnrInt*)mPostInParam;
    RkAiqAlgoPostResAnrInt* anr_post_res_int = (RkAiqAlgoPostResAnrInt*)mPostOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqPostResComb* comb = &shared->postResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAnrHandle::postProcess();
    if (ret) {
        comb->anr_post_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "anr handle postProcess failed");
        return ret;
    }

    comb->anr_post_res = NULL;
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->post_process(mPostInParam, mPostOutParam);
    RKAIQCORE_CHECK_RET(ret, "anr algo post_process failed");
    // set result to mAiqCore
    comb->anr_post_res = (RkAiqAlgoPostResAnr*)anr_post_res_int ;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

void
RkAiqAsharpHandleInt::init()
{
    ENTER_ANALYZER_FUNCTION();

    RkAiqAsharpHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAsharpInt());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAsharpInt());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAsharpInt());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAsharpInt());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAsharpInt());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAsharpInt());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAsharpInt());

    EXIT_ANALYZER_FUNCTION();
}


XCamReturn
RkAiqAsharpHandleInt::updateConfig(bool needSync)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (needSync)
        mCfgMutex.lock();
    // if something changed
    if (updateAtt) {
        mCurAtt = mNewAtt;
        updateAtt = false;
        // TODO
        rk_aiq_uapi_asharp_SetAttrib(mAlgoCtx, &mCurAtt, false);
        sendSignal();
    }

    if(updateIQpara) {
        mCurIQPara = mNewIQPara;
        updateIQpara = false;
        // TODO
        rk_aiq_uapi_asharp_SetIQpara(mAlgoCtx, &mCurIQPara, false);
        sendSignal();
    }

    if (needSync)
        mCfgMutex.unlock();


    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAsharpHandleInt::setAttrib(rk_aiq_sharp_attrib_t *att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    //TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed
    if (0 != memcmp(&mCurAtt, att, sizeof(rk_aiq_sharp_attrib_t))) {
        mNewAtt = *att;
        updateAtt = true;
        waitSignal();
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAsharpHandleInt::getAttrib(rk_aiq_sharp_attrib_t *att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_asharp_GetAttrib(mAlgoCtx, att);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAsharpHandleInt::setIQPara(rk_aiq_sharp_IQpara_t *para)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    //TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed
    if (0 != memcmp(&mCurIQPara, para, sizeof(rk_aiq_sharp_IQpara_t))) {
        mNewIQPara = *para;
        updateIQpara = true;
        waitSignal();
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAsharpHandleInt::getIQPara(rk_aiq_sharp_IQpara_t *para)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_asharp_GetIQpara(mAlgoCtx, para);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}


XCamReturn
RkAiqAsharpHandleInt::setStrength(float fPercent)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_asharp_SetStrength(mAlgoCtx, fPercent);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}


XCamReturn
RkAiqAsharpHandleInt::getStrength(float *pPercent)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_asharp_GetStrength(mAlgoCtx, pPercent);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}


XCamReturn
RkAiqAsharpHandleInt::prepare()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqAsharpHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "asharp handle prepare failed");

    RkAiqAlgoConfigAsharpInt* asharp_config_int = (RkAiqAlgoConfigAsharpInt*)mConfig;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "asharp algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqAsharpHandleInt::preProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPreAsharpInt* asharp_pre_int = (RkAiqAlgoPreAsharpInt*)mPreInParam;
    RkAiqAlgoPreResAsharpInt* asharp_pre_res_int = (RkAiqAlgoPreResAsharpInt*)mPreOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqPreResComb* comb = &shared->preResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAsharpHandle::preProcess();
    if (ret) {
        comb->asharp_pre_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "asharp handle preProcess failed");
    }

    comb->asharp_pre_res = NULL;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->pre_process(mPreInParam, mPreOutParam);
    RKAIQCORE_CHECK_RET(ret, "asharp algo pre_process failed");
    // set result to mAiqCore
    comb->asharp_pre_res = (RkAiqAlgoPreResAsharp*)asharp_pre_res_int;

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqAsharpHandleInt::processing()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAsharpInt* asharp_proc_int = (RkAiqAlgoProcAsharpInt*)mProcInParam;
    RkAiqAlgoProcResAsharpInt* asharp_proc_res_int = (RkAiqAlgoProcResAsharpInt*)mProcOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqProcResComb* comb = &shared->procResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;
    static int asharp_proc_framecnt = 0;
    asharp_proc_framecnt++;

    ret = RkAiqAsharpHandle::processing();
    if (ret) {
        comb->asharp_proc_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "asharp handle processing failed");
    }

    comb->asharp_proc_res = NULL;

    // TODO: fill procParam
    asharp_proc_int->iso = sharedCom->iso;
    asharp_proc_int->hdr_mode = sharedCom->working_mode;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->processing(mProcInParam, mProcOutParam);
    RKAIQCORE_CHECK_RET(ret, "asharp algo processing failed");

    comb->asharp_proc_res = (RkAiqAlgoProcResAsharp*)asharp_proc_res_int;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAsharpHandleInt::postProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPostAsharpInt* asharp_post_int = (RkAiqAlgoPostAsharpInt*)mPostInParam;
    RkAiqAlgoPostResAsharpInt* asharp_post_res_int = (RkAiqAlgoPostResAsharpInt*)mPostOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqPostResComb* comb = &shared->postResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAsharpHandle::postProcess();
    if (ret) {
        comb->asharp_post_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "asharp handle postProcess failed");
        return ret;
    }

    comb->asharp_post_res = NULL;
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->post_process(mPostInParam, mPostOutParam);
    RKAIQCORE_CHECK_RET(ret, "asharp algo post_process failed");
    // set result to mAiqCore
    comb->asharp_post_res = (RkAiqAlgoPostResAsharp*)asharp_post_res_int ;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

void
RkAiqAdhazHandleInt::init()
{
    ENTER_ANALYZER_FUNCTION();

    RkAiqAdhazHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAdhazInt());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAdhazInt());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAdhazInt());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAdhazInt());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAdhazInt());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAdhazInt());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAdhazInt());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn
RkAiqAdhazHandleInt::prepare()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqAdhazHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "adhaz handle prepare failed");

    RkAiqAlgoConfigAdhazInt* adhaz_config_int = (RkAiqAlgoConfigAdhazInt*)mConfig;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    adhaz_config_int->calib = sharedCom->calib;

    adhaz_config_int->working_mode = sharedCom->working_mode;
    // adhaz_config_int->rawHeight = sharedCom->snsDes.isp_acq_height;
    // adhaz_config_int->rawWidth = sharedCom->snsDes.isp_acq_width;
    // adhaz_config_int->working_mode = sharedCom->working_mode;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "adhaz algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqAdhazHandleInt::preProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPreAdhazInt* adhaz_pre_int = (RkAiqAlgoPreAdhazInt*)mPreInParam;
    RkAiqAlgoPreResAdhazInt* adhaz_pre_res_int = (RkAiqAlgoPreResAdhazInt*)mPreOutParam;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqIspStats* ispStats = shared->ispStats;
    RkAiqPreResComb* comb = &shared->preResComb;

    adhaz_pre_int->rawHeight = sharedCom->snsDes.isp_acq_height;
    adhaz_pre_int->rawWidth = sharedCom->snsDes.isp_acq_width;

    ret = RkAiqAdhazHandle::preProcess();
    if (ret) {
        comb->adhaz_pre_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "adhaz handle preProcess failed");
    }

    comb->adhaz_pre_res = NULL;

#ifdef RK_SIMULATOR_HW
    //nothing todo
#endif
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->pre_process(mPreInParam, mPreOutParam);
    RKAIQCORE_CHECK_RET(ret, "adhaz algo pre_process failed");

    // set result to mAiqCore
    comb->adhaz_pre_res = (RkAiqAlgoPreResAdhaz*)adhaz_pre_res_int;

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqAdhazHandleInt::processing()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAdhazInt* adhaz_proc_int = (RkAiqAlgoProcAdhazInt*)mProcInParam;
    RkAiqAlgoProcResAdhazInt* adhaz_proc_res_int = (RkAiqAlgoProcResAdhazInt*)mProcOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqProcResComb* comb = &shared->procResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    adhaz_proc_int->hdr_mode = sharedCom->working_mode;

    ret = RkAiqAdhazHandle::processing();
    if (ret) {
        comb->adhaz_proc_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "adhaz handle processing failed");
    }

    if(!shared->ispStats->adehaze_stats_valid && !sharedCom->init) {
        LOGD("no adehaze stats, ignore!");
        // TODO: keep last result ?
        //
        //
        return XCAM_RETURN_BYPASS;
    } else {
        if(sharedCom->ctxCfigs[RK_AIQ_ALGO_TYPE_ADHAZ].cfg_com.isp_hw_version == 0)
            memcpy(&adhaz_proc_int->stats.dehaze_stats_v20, &ispStats->adehaze_stats.dehaze_stats_v20, sizeof(dehaze_stats_v20_t));
        else if(sharedCom->ctxCfigs[RK_AIQ_ALGO_TYPE_ADHAZ].cfg_com.isp_hw_version == 1)
            memcpy(&adhaz_proc_int->stats.dehaze_stats_v21, &ispStats->adehaze_stats.dehaze_stats_v21, sizeof(dehaze_stats_v21_t));
        memcpy(adhaz_proc_int->stats.other_stats.tmo_luma,
               ispStats->aec_stats.ae_data.extra.rawae_big.channelg_xy, sizeof(adhaz_proc_int->stats.other_stats.tmo_luma));

        if(sharedCom->working_mode == RK_AIQ_ISP_HDR_MODE_3_FRAME_HDR || sharedCom->working_mode == RK_AIQ_ISP_HDR_MODE_3_LINE_HDR)
        {
            memcpy(adhaz_proc_int->stats.other_stats.short_luma,
                   ispStats->aec_stats.ae_data.chn[0].rawae_big.channelg_xy, sizeof(adhaz_proc_int->stats.other_stats.short_luma));
            memcpy(adhaz_proc_int->stats.other_stats.middle_luma,
                   ispStats->aec_stats.ae_data.chn[1].rawae_lite.channelg_xy, sizeof(adhaz_proc_int->stats.other_stats.middle_luma));
            memcpy(adhaz_proc_int->stats.other_stats.long_luma,
                   ispStats->aec_stats.ae_data.chn[2].rawae_big.channelg_xy, sizeof(adhaz_proc_int->stats.other_stats.long_luma));
        }
        else if(sharedCom->working_mode == RK_AIQ_ISP_HDR_MODE_2_FRAME_HDR || sharedCom->working_mode == RK_AIQ_ISP_HDR_MODE_2_LINE_HDR)
        {
            memcpy(adhaz_proc_int->stats.other_stats.short_luma,
                   ispStats->aec_stats.ae_data.chn[0].rawae_big.channelg_xy, sizeof(adhaz_proc_int->stats.other_stats.short_luma));
            memcpy(adhaz_proc_int->stats.other_stats.long_luma,
                   ispStats->aec_stats.ae_data.chn[1].rawae_big.channelg_xy, sizeof(adhaz_proc_int->stats.other_stats.long_luma));
        }
        else
            LOGD("Wrong working mode!!!");
    }

    adhaz_proc_int->pCalibDehaze = sharedCom->calib;

    comb->adhaz_proc_res = NULL;

#ifdef RK_SIMULATOR_HW
    //nothing todo
#endif
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->processing(mProcInParam, mProcOutParam);
    RKAIQCORE_CHECK_RET(ret, "adhaz algo processing failed");

    comb->adhaz_proc_res = (RkAiqAlgoProcResAdhaz*)adhaz_proc_res_int;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAdhazHandleInt::postProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPostAdhazInt* adhaz_post_int = (RkAiqAlgoPostAdhazInt*)mPostInParam;
    RkAiqAlgoPostResAdhazInt* adhaz_post_res_int = (RkAiqAlgoPostResAdhazInt*)mPostOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqPostResComb* comb = &shared->postResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAdhazHandle::postProcess();
    if (ret) {
        comb->adhaz_post_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "adhaz handle postProcess failed");
        return ret;
    }

    comb->adhaz_post_res = NULL;
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->post_process(mPostInParam, mPostOutParam);
    RKAIQCORE_CHECK_RET(ret, "adhaz algo post_process failed");
    // set result to mAiqCore
    comb->adhaz_post_res = (RkAiqAlgoPostResAdhaz*)adhaz_post_res_int ;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAdhazHandleInt::updateConfig(bool needSync)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (needSync)
        mCfgMutex.lock();
    // if something changed
    if (updateAtt) {
        mCurAtt = mNewAtt;
        updateAtt = false;
        // TODO
        rk_aiq_uapi_adehaze_SetAttrib(mAlgoCtx, mCurAtt, false);
        sendSignal();
    }

    if (needSync)
        mCfgMutex.unlock();


    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAdhazHandleInt::setSwAttrib(adehaze_sw_s att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    //TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed
    if (0 != memcmp(&mCurAtt, &att, sizeof(adehaze_sw_s))) {
        mNewAtt = att;
        updateAtt = true;
        waitSignal();
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAdhazHandleInt::getSwAttrib(adehaze_sw_s *att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_adehaze_GetAttrib(mAlgoCtx, att);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

void
RkAiqAmergeHandleInt::init()
{
    ENTER_ANALYZER_FUNCTION();

    RkAiqAmergeHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAmergeInt());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAmergeInt());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAmergeInt());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAmergeInt());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAmergeInt());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAmergeInt());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAmergeInt());

    EXIT_ANALYZER_FUNCTION();
}
XCamReturn
RkAiqAmergeHandleInt::updateConfig(bool needSync)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (needSync)
        mCfgMutex.lock();
    // if something changed
    if (updateAtt) {
        mCurAtt = mNewAtt;
        updateAtt = false;
        rk_aiq_uapi_amerge_SetAttrib(mAlgoCtx, mCurAtt, true);
        sendSignal();
    }
    if (needSync)
        mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAmergeHandleInt::setAttrib(amerge_attrib_t att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    //TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed
    if (0 != memcmp(&mCurAtt, &att, sizeof(amerge_attrib_t))) {
        mNewAtt = att;
        updateAtt = true;
        waitSignal();
    }
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
XCamReturn
RkAiqAmergeHandleInt::getAttrib(amerge_attrib_t* att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_amerge_GetAttrib(mAlgoCtx, att);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAmergeHandleInt::prepare()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqAmergeHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "amerge handle prepare failed");

    RkAiqAlgoConfigAmergeInt* amerge_config_int = (RkAiqAlgoConfigAmergeInt*)mConfig;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    //TODO
    amerge_config_int->rawHeight = sharedCom->snsDes.isp_acq_height;
    amerge_config_int->rawWidth = sharedCom->snsDes.isp_acq_width;
    amerge_config_int->working_mode = sharedCom->working_mode;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "amerge algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqAmergeHandleInt::preProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPreAmergeInt* amerge_pre_int = (RkAiqAlgoPreAmergeInt*)mPreInParam;
    RkAiqAlgoPreResAmergeInt* amerge_pre_res_int = (RkAiqAlgoPreResAmergeInt*)mPreOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqIspStats* ispStats = shared->ispStats;
    RkAiqPreResComb* comb = &shared->preResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqAmergeHandle::preProcess();
    if (ret) {
        comb->amerge_pre_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "amerge handle preProcess failed");
    }

    comb->amerge_pre_res = NULL;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->pre_process(mPreInParam, mPreOutParam);
    RKAIQCORE_CHECK_RET(ret, "amerge algo pre_process failed");

    // set result to mAiqCore
    comb->amerge_pre_res = (RkAiqAlgoPreResAmerge*)amerge_pre_res_int;

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqAmergeHandleInt::processing()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAmergeInt* amerge_proc_int = (RkAiqAlgoProcAmergeInt*)mProcInParam;
    RkAiqAlgoProcResAmergeInt* amerge_proc_res_int = (RkAiqAlgoProcResAmergeInt*)mProcOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqIspStats* ispStats = shared->ispStats;
    RkAiqProcResComb* comb = &shared->procResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqAmergeHandle::processing();
    if (ret) {
        comb->amerge_proc_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "amerge handle processing failed");
    }

    comb->amerge_proc_res = NULL;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->processing(mProcInParam, mProcOutParam);
    RKAIQCORE_CHECK_RET(ret, "amerge algo processing failed");

    comb->amerge_proc_res = (RkAiqAlgoProcResAmerge*)amerge_proc_res_int;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAmergeHandleInt::postProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPostAmergeInt* amerge_post_int = (RkAiqAlgoPostAmergeInt*)mPostInParam;
    RkAiqAlgoPostResAmergeInt* amerge_post_res_int = (RkAiqAlgoPostResAmergeInt*)mPostOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqIspStats* ispStats = shared->ispStats;
    RkAiqPostResComb* comb = &shared->postResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqAmergeHandle::postProcess();
    if (ret) {
        comb->amerge_post_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "amerge handle postProcess failed");
        return ret;
    }

    comb->amerge_post_res = NULL;
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->post_process(mPostInParam, mPostOutParam);
    RKAIQCORE_CHECK_RET(ret, "amerge algo post_process failed");
    // set result to mAiqCore
    comb->amerge_post_res = (RkAiqAlgoPostResAmerge*)amerge_post_res_int ;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

void
RkAiqAtmoHandleInt::init()
{
    ENTER_ANALYZER_FUNCTION();

    RkAiqAtmoHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAtmoInt());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAtmoInt());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAtmoInt());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAtmoInt());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAtmoInt());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAtmoInt());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAtmoInt());

    EXIT_ANALYZER_FUNCTION();
}
XCamReturn
RkAiqAtmoHandleInt::updateConfig(bool needSync)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (needSync)
        mCfgMutex.lock();
    // if something changed
    if (updateAtt) {
        mCurAtt = mNewAtt;
        updateAtt = false;
        rk_aiq_uapi_atmo_SetAttrib(mAlgoCtx, mCurAtt, true);
        sendSignal();
    }
    if (needSync)
        mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAtmoHandleInt::setAttrib(atmo_attrib_t att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    //TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed
    if (0 != memcmp(&mCurAtt, &att, sizeof(atmo_attrib_t))) {
        mNewAtt = att;
        updateAtt = true;
        waitSignal();
    }
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
XCamReturn
RkAiqAtmoHandleInt::getAttrib(atmo_attrib_t* att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_atmo_GetAttrib(mAlgoCtx, att);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAtmoHandleInt::prepare()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqAtmoHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "atmo handle prepare failed");

    RkAiqAlgoConfigAtmoInt* atmo_config_int = (RkAiqAlgoConfigAtmoInt*)mConfig;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqIspStats* ispStats = shared->ispStats;
    RkAiqPreResComb* comb = &shared->preResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    //TODO
    atmo_config_int->rawHeight = sharedCom->snsDes.isp_acq_height;
    atmo_config_int->rawWidth = sharedCom->snsDes.isp_acq_width;
    atmo_config_int->working_mode = sharedCom->working_mode;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "atmo algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqAtmoHandleInt::preProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPreAtmoInt* atmo_pre_int = (RkAiqAlgoPreAtmoInt*)mPreInParam;
    RkAiqAlgoPreResAtmoInt* atmo_pre_res_int = (RkAiqAlgoPreResAtmoInt*)mPreOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqIspStats* ispStats = shared->ispStats;
    RkAiqPreResComb* comb = &shared->preResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqAtmoHandle::preProcess();
    if (ret) {
        comb->atmo_pre_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "atmo handle preProcess failed");
    }

    if(!shared->ispStats->atmo_stats_valid && !sharedCom->init) {
        LOGD("no atmo stats, ignore!");
        // TODO: keep last result ?
        //         comb->atmo_proc_res = NULL;
        //
        return XCAM_RETURN_BYPASS;
    }

    comb->atmo_pre_res = NULL;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->pre_process(mPreInParam, mPreOutParam);
    RKAIQCORE_CHECK_RET(ret, "ahdr algo pre_process failed");

    // set result to mAiqCore
    comb->atmo_pre_res = (RkAiqAlgoPreResAtmo*)atmo_pre_res_int;

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqAtmoHandleInt::processing()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAtmoInt* atmo_proc_int = (RkAiqAlgoProcAtmoInt*)mProcInParam;
    RkAiqAlgoProcResAtmoInt* atmo_proc_res_int = (RkAiqAlgoProcResAtmoInt*)mProcOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqIspStats* ispStats = shared->ispStats;
    RkAiqProcResComb* comb = &shared->procResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqAtmoHandle::processing();
    if (ret) {
        comb->atmo_proc_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "atmo handle processing failed");
    }

    if(!shared->ispStats->atmo_stats_valid && !sharedCom->init) {
        LOGD("no atmo stats, ignore!");
        // TODO: keep last result ?
        //         comb->atmo_proc_res = NULL;
        //
        return XCAM_RETURN_BYPASS;
    } else {
        memcpy(&atmo_proc_int->ispAtmoStats.tmo_stats, &ispStats->atmo_stats.tmo_stats, sizeof(hdrtmo_stats_t));
        memcpy(atmo_proc_int->ispAtmoStats.other_stats.tmo_luma,
               ispStats->aec_stats.ae_data.extra.rawae_big.channelg_xy, sizeof(atmo_proc_int->ispAtmoStats.other_stats.tmo_luma));

        if(sharedCom->working_mode == RK_AIQ_ISP_HDR_MODE_3_FRAME_HDR || sharedCom->working_mode == RK_AIQ_ISP_HDR_MODE_3_LINE_HDR)
        {
            memcpy(atmo_proc_int->ispAtmoStats.other_stats.short_luma,
                   ispStats->aec_stats.ae_data.chn[0].rawae_big.channelg_xy, sizeof(atmo_proc_int->ispAtmoStats.other_stats.short_luma));
            memcpy(atmo_proc_int->ispAtmoStats.other_stats.middle_luma,
                   ispStats->aec_stats.ae_data.chn[1].rawae_lite.channelg_xy, sizeof(atmo_proc_int->ispAtmoStats.other_stats.middle_luma));
            memcpy(atmo_proc_int->ispAtmoStats.other_stats.long_luma,
                   ispStats->aec_stats.ae_data.chn[2].rawae_big.channelg_xy, sizeof(atmo_proc_int->ispAtmoStats.other_stats.long_luma));
        }
        else if(sharedCom->working_mode == RK_AIQ_ISP_HDR_MODE_2_FRAME_HDR || sharedCom->working_mode == RK_AIQ_ISP_HDR_MODE_2_LINE_HDR)
        {
            memcpy(atmo_proc_int->ispAtmoStats.other_stats.short_luma,
                   ispStats->aec_stats.ae_data.chn[0].rawae_big.channelg_xy, sizeof(atmo_proc_int->ispAtmoStats.other_stats.short_luma));
            memcpy(atmo_proc_int->ispAtmoStats.other_stats.long_luma,
                   ispStats->aec_stats.ae_data.chn[1].rawae_big.channelg_xy, sizeof(atmo_proc_int->ispAtmoStats.other_stats.long_luma));
        }
        else
            LOGD("Wrong working mode!!!");
    }

    comb->atmo_proc_res = NULL;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->processing(mProcInParam, mProcOutParam);
    RKAIQCORE_CHECK_RET(ret, "atmo algo processing failed");

    comb->atmo_proc_res = (RkAiqAlgoProcResAtmo*)atmo_proc_res_int;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAtmoHandleInt::postProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPostAtmoInt* atmo_post_int = (RkAiqAlgoPostAtmoInt*)mPostInParam;
    RkAiqAlgoPostResAtmoInt* atmo_post_res_int = (RkAiqAlgoPostResAtmoInt*)mPostOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqIspStats* ispStats = shared->ispStats;
    RkAiqPostResComb* comb = &shared->postResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqAtmoHandle::postProcess();
    if (ret) {
        comb->atmo_post_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "ahdr handle postProcess failed");
        return ret;
    }

    if(!shared->ispStats->atmo_stats_valid && !sharedCom->init) {
        LOGD("no atmo stats, ignore!");
        // TODO: keep last result ?
        //         comb->ahdr_proc_res = NULL;
        //
        return XCAM_RETURN_BYPASS;
    }

    comb->atmo_post_res = NULL;
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->post_process(mPostInParam, mPostOutParam);
    RKAIQCORE_CHECK_RET(ret, "atmo algo post_process failed");
    // set result to mAiqCore
    comb->atmo_post_res = (RkAiqAlgoPostResAtmo*)atmo_post_res_int ;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

void
RkAiqAsdHandleInt::init()
{
    ENTER_ANALYZER_FUNCTION();

    RkAiqAsdHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAsdInt());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAsdInt());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAsdInt());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAsdInt());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAsdInt());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAsdInt());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAsdInt());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn
RkAiqAsdHandleInt::updateConfig(bool needSync)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (needSync)
        mCfgMutex.lock();
    // if something changed
    if (updateAtt) {
        mCurAtt = mNewAtt;
        updateAtt = false;
        rk_aiq_uapi_asd_SetAttrib(mAlgoCtx, mCurAtt, false);
        sendSignal();
    }

    if (needSync)
        mCfgMutex.unlock();


    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAsdHandleInt::setAttrib(asd_attrib_t att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    //TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed
    if (0 != memcmp(&mCurAtt, &att, sizeof(asd_attrib_t))) {
        mNewAtt = att;
        updateAtt = true;
        waitSignal();
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAsdHandleInt::getAttrib(asd_attrib_t *att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_asd_GetAttrib(mAlgoCtx, att);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAsdHandleInt::prepare()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqAsdHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "asd handle prepare failed");

    RkAiqAlgoConfigAsdInt* asd_config_int = (RkAiqAlgoConfigAsdInt*)mConfig;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "asd algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqAsdHandleInt::preProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPreAsdInt* asd_pre_int = (RkAiqAlgoPreAsdInt*)mPreInParam;
    RkAiqAlgoPreResAsdInt* asd_pre_res_int = (RkAiqAlgoPreResAsdInt*)mPreOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqPreResComb* comb = &shared->preResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAsdHandle::preProcess();
    if (ret) {
        comb->asd_pre_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "asd handle preProcess failed");
    }

    comb->asd_pre_res = NULL;
    asd_pre_int->pre_params.cpsl_mode = sharedCom->cpslCfg.mode;
    asd_pre_int->pre_params.cpsl_on = sharedCom->cpslCfg.u.m.on;
    asd_pre_int->pre_params.cpsl_sensitivity = sharedCom->cpslCfg.u.a.sensitivity;
    asd_pre_int->pre_params.cpsl_sw_interval = sharedCom->cpslCfg.u.a.sw_interval;
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->pre_process(mPreInParam, mPreOutParam);
    RKAIQCORE_CHECK_RET(ret, "asd algo pre_process failed");

    // set result to mAiqCore
    comb->asd_pre_res = (RkAiqAlgoPreResAsd*)asd_pre_res_int;

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqAsdHandleInt::processing()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAsdInt* asd_proc_int = (RkAiqAlgoProcAsdInt*)mProcInParam;
    RkAiqAlgoProcResAsdInt* asd_proc_res_int = (RkAiqAlgoProcResAsdInt*)mProcOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqProcResComb* comb = &shared->procResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAsdHandle::processing();
    if (ret) {
        comb->asd_proc_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "asd handle processing failed");
    }

    comb->asd_proc_res = NULL;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->processing(mProcInParam, mProcOutParam);
    RKAIQCORE_CHECK_RET(ret, "asd algo processing failed");

    comb->asd_proc_res = (RkAiqAlgoProcResAsd*)asd_proc_res_int;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAsdHandleInt::postProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPostAsdInt* asd_post_int = (RkAiqAlgoPostAsdInt*)mPostInParam;
    RkAiqAlgoPostResAsdInt* asd_post_res_int = (RkAiqAlgoPostResAsdInt*)mPostOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqPostResComb* comb = &shared->postResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAsdHandle::postProcess();
    if (ret) {
        comb->asd_post_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "asd handle postProcess failed");
        return ret;
    }

    comb->asd_post_res = NULL;
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->post_process(mPostInParam, mPostOutParam);
    RKAIQCORE_CHECK_RET(ret, "asd algo post_process failed");
    // set result to mAiqCore
    comb->asd_post_res = (RkAiqAlgoPostResAsd*)asd_post_res_int ;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

void
RkAiqAcpHandleInt::init()
{
    ENTER_ANALYZER_FUNCTION();

    RkAiqAcpHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAcpInt());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAcpInt());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAcpInt());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAcpInt());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAcpInt());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAcpInt());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAcpInt());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn
RkAiqAcpHandleInt::updateConfig(bool needSync)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (needSync)
        mCfgMutex.lock();
    // if something changed
    if (updateAtt) {
        mCurAtt = mNewAtt;
        updateAtt = false;
        rk_aiq_uapi_acp_SetAttrib(mAlgoCtx, mCurAtt, false);
        sendSignal();
    }
    if (needSync)
        mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAcpHandleInt::setAttrib(acp_attrib_t att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore
    if (0 != memcmp(&mCurAtt, &att, sizeof(acp_attrib_t))) {
        mNewAtt = att;
        updateAtt = true;
        waitSignal();
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAcpHandleInt::getAttrib(acp_attrib_t *att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_acp_GetAttrib(mAlgoCtx, att);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAcpHandleInt::prepare()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqAcpHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "acp handle prepare failed");

    RkAiqAlgoConfigAcpInt* acp_config_int = (RkAiqAlgoConfigAcpInt*)mConfig;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "acp algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqAcpHandleInt::preProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPreAcpInt* acp_pre_int = (RkAiqAlgoPreAcpInt*)mPreInParam;
    RkAiqAlgoPreResAcpInt* acp_pre_res_int = (RkAiqAlgoPreResAcpInt*)mPreOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqPreResComb* comb = &shared->preResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAcpHandle::preProcess();
    if (ret) {
        comb->acp_pre_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "acp handle preProcess failed");
    }

    comb->acp_pre_res = NULL;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->pre_process(mPreInParam, mPreOutParam);
    RKAIQCORE_CHECK_RET(ret, "acp algo pre_process failed");

    // set result to mAiqCore
    comb->acp_pre_res = (RkAiqAlgoPreResAcp*)acp_pre_res_int;

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqAcpHandleInt::processing()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAcpInt* acp_proc_int = (RkAiqAlgoProcAcpInt*)mProcInParam;
    RkAiqAlgoProcResAcpInt* acp_proc_res_int = (RkAiqAlgoProcResAcpInt*)mProcOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqProcResComb* comb = &shared->procResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAcpHandle::processing();
    if (ret) {
        comb->acp_proc_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "acp handle processing failed");
    }

    comb->acp_proc_res = NULL;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->processing(mProcInParam, mProcOutParam);
    RKAIQCORE_CHECK_RET(ret, "acp algo processing failed");

    comb->acp_proc_res = (RkAiqAlgoProcResAcp*)acp_proc_res_int;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAcpHandleInt::postProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPostAcpInt* acp_post_int = (RkAiqAlgoPostAcpInt*)mPostInParam;
    RkAiqAlgoPostResAcpInt* acp_post_res_int = (RkAiqAlgoPostResAcpInt*)mPostOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqPostResComb* comb = &shared->postResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAcpHandle::postProcess();
    if (ret) {
        comb->acp_post_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "acp handle postProcess failed");
        return ret;
    }

    comb->acp_post_res = NULL;
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->post_process(mPostInParam, mPostOutParam);
    RKAIQCORE_CHECK_RET(ret, "acp algo post_process failed");
    // set result to mAiqCore
    comb->acp_post_res = (RkAiqAlgoPostResAcp*)acp_post_res_int ;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

void
RkAiqA3dlutHandleInt::init()
{
    ENTER_ANALYZER_FUNCTION();

    RkAiqA3dlutHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigA3dlutInt());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreA3dlutInt());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResA3dlutInt());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcA3dlutInt());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResA3dlutInt());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostA3dlutInt());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResA3dlutInt());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn
RkAiqA3dlutHandleInt::updateConfig(bool needSync)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (needSync)
        mCfgMutex.lock();
    // if something changed
    if (updateAtt) {
        mCurAtt = mNewAtt;
        updateAtt = false;
        // TODO
        rk_aiq_uapi_a3dlut_SetAttrib(mAlgoCtx, mCurAtt, false);
        sendSignal();
    }

    if (needSync)
        mCfgMutex.unlock();


    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqA3dlutHandleInt::setAttrib(rk_aiq_lut3d_attrib_t att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    //TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed
    if (0 != memcmp(&mCurAtt, &att, sizeof(rk_aiq_lut3d_attrib_t))) {
        mNewAtt = att;
        updateAtt = true;
        waitSignal();
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqA3dlutHandleInt::getAttrib(rk_aiq_lut3d_attrib_t *att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_a3dlut_GetAttrib(mAlgoCtx, att);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqA3dlutHandleInt::query3dlutInfo(rk_aiq_lut3d_querry_info_t *lut3d_querry_info )
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_a3dlut_Query3dlutInfo(mAlgoCtx, lut3d_querry_info);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}


XCamReturn
RkAiqA3dlutHandleInt::prepare()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqA3dlutHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "a3dlut handle prepare failed");

    RkAiqAlgoConfigA3dlutInt* a3dlut_config_int = (RkAiqAlgoConfigA3dlutInt*)mConfig;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "a3dlut algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqA3dlutHandleInt::preProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPreA3dlutInt* a3dlut_pre_int = (RkAiqAlgoPreA3dlutInt*)mPreInParam;
    RkAiqAlgoPreResA3dlutInt* a3dlut_pre_res_int = (RkAiqAlgoPreResA3dlutInt*)mPreOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqPreResComb* comb = &shared->preResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqA3dlutHandle::preProcess();
    if (ret) {
        comb->a3dlut_pre_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "a3dlut handle preProcess failed");
    }

    comb->a3dlut_pre_res = NULL;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->pre_process(mPreInParam, mPreOutParam);
    RKAIQCORE_CHECK_RET(ret, "a3dlut algo pre_process failed");

    // set result to mAiqCore
    comb->a3dlut_pre_res = (RkAiqAlgoPreResA3dlut*)a3dlut_pre_res_int;

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqA3dlutHandleInt::processing()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcA3dlutInt* a3dlut_proc_int = (RkAiqAlgoProcA3dlutInt*)mProcInParam;
    RkAiqAlgoProcResA3dlutInt* a3dlut_proc_res_int = (RkAiqAlgoProcResA3dlutInt*)mProcOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqProcResComb* comb = &shared->procResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqA3dlutHandle::processing();
    if (ret) {
        comb->a3dlut_proc_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "a3dlut handle processing failed");
    }

    comb->a3dlut_proc_res = NULL;
    XCamVideoBuffer* xCamAwbProcRes = shared->res_comb.awb_proc_res;
    if (xCamAwbProcRes) {
        RkAiqAlgoProcResAwbInt* awb_res_int = (RkAiqAlgoProcResAwbInt*)xCamAwbProcRes->map(xCamAwbProcRes);
        RkAiqAlgoProcResAwb* awb_res = &awb_res_int->awb_proc_res_com;
        if(awb_res) {
            if(awb_res->awb_gain_algo.grgain < DIVMIN ||
                    awb_res->awb_gain_algo.gbgain < DIVMIN ) {
                LOGE("get wrong awb gain from AWB module ,use default value ");
            } else {
                a3dlut_proc_int->awbGain[0] =
                    awb_res->awb_gain_algo.rgain / awb_res->awb_gain_algo.grgain;

                a3dlut_proc_int->awbGain[1] =
                    awb_res->awb_gain_algo.bgain / awb_res->awb_gain_algo.gbgain;
            }
            a3dlut_proc_int->awbIIRDampCoef =  awb_res_int->awb_smooth_factor;
            a3dlut_proc_int->awbConverged = awb_res_int->awbConverged;
        } else {
            LOGE("fail to get awb gain form AWB module,use default value ");
        }
    } else {
        LOGE("fail to get awb gain form AWB module,use default value ");
    }
    RKAiqAecExpInfo_t *pCurExp = &shared->curExp;
    if(pCurExp) {
        if((rk_aiq_working_mode_t)sharedCom->working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
            a3dlut_proc_int->sensorGain = pCurExp->LinearExp.exp_real_params.analog_gain
                                          * pCurExp->LinearExp.exp_real_params.digital_gain
                                          * pCurExp->LinearExp.exp_real_params.isp_dgain;
        } else if((rk_aiq_working_mode_t)sharedCom->working_mode >= RK_AIQ_WORKING_MODE_ISP_HDR2
                  && (rk_aiq_working_mode_t)sharedCom->working_mode < RK_AIQ_WORKING_MODE_ISP_HDR3)  {
            LOGD("sensor gain choose from second hdr frame for a3dlut");
            a3dlut_proc_int->sensorGain = pCurExp->HdrExp[1].exp_real_params.analog_gain
                                          * pCurExp->HdrExp[1].exp_real_params.digital_gain
                                          * pCurExp->HdrExp[1].exp_real_params.isp_dgain;
        } else if((rk_aiq_working_mode_t)sharedCom->working_mode >= RK_AIQ_WORKING_MODE_ISP_HDR2
                  && (rk_aiq_working_mode_t)sharedCom->working_mode >= RK_AIQ_WORKING_MODE_ISP_HDR3)  {
            LOGD("sensor gain choose from third hdr frame for a3dlut");
            a3dlut_proc_int->sensorGain = pCurExp->HdrExp[2].exp_real_params.analog_gain
                                          * pCurExp->HdrExp[2].exp_real_params.digital_gain
                                          * pCurExp->HdrExp[2].exp_real_params.isp_dgain;
        } else {
            LOGE("working_mode (%d) is invaild ,fail to get sensor gain form AE module,use default value ",
                 sharedCom->working_mode);
        }
    } else {
        LOGE("fail to get sensor gain form AE module,use default value ");
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->processing(mProcInParam, mProcOutParam);
    RKAIQCORE_CHECK_RET(ret, "a3dlut algo processing failed");

    comb->a3dlut_proc_res = (RkAiqAlgoProcResA3dlut*)a3dlut_proc_res_int;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqA3dlutHandleInt::postProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPostA3dlutInt* a3dlut_post_int = (RkAiqAlgoPostA3dlutInt*)mPostInParam;
    RkAiqAlgoPostResA3dlutInt* a3dlut_post_res_int = (RkAiqAlgoPostResA3dlutInt*)mPostOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqPostResComb* comb = &shared->postResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqA3dlutHandle::postProcess();
    if (ret) {
        comb->a3dlut_post_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "a3dlut handle postProcess failed");
        return ret;
    }

    comb->a3dlut_post_res = NULL;
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->post_process(mPostInParam, mPostOutParam);
    RKAIQCORE_CHECK_RET(ret, "a3dlut algo post_process failed");
    // set result to mAiqCore
    comb->a3dlut_post_res = (RkAiqAlgoPostResA3dlut*)a3dlut_post_res_int ;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

void
RkAiqAblcHandleInt::init()
{
    ENTER_ANALYZER_FUNCTION();

    RkAiqAblcHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAblcInt());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAblcInt());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAblcInt());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAblcInt());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAblcInt());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAblcInt());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAblcInt());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn
RkAiqAblcHandleInt::updateConfig(bool needSync)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (needSync)
        mCfgMutex.lock();
    // if something changed
    if (updateAtt) {
        mCurAtt = mNewAtt;
        updateAtt = false;
        // TODO
        rk_aiq_uapi_ablc_SetAttrib(mAlgoCtx, &mCurAtt, false);
        sendSignal();
    }

    if (needSync)
        mCfgMutex.unlock();


    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAblcHandleInt::setAttrib(rk_aiq_blc_attrib_t *att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    //TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed
    if (0 != memcmp(&mCurAtt, att, sizeof(rk_aiq_blc_attrib_t))) {
        mNewAtt = *att;
        updateAtt = true;
        waitSignal();
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAblcHandleInt::getAttrib(rk_aiq_blc_attrib_t *att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_ablc_GetAttrib(mAlgoCtx, att);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}


XCamReturn
RkAiqAblcHandleInt::prepare()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqAblcHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "ablc handle prepare failed");

    RkAiqAlgoConfigAblcInt* ablc_config_int = (RkAiqAlgoConfigAblcInt*)mConfig;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "ablc algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqAblcHandleInt::preProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPreAblcInt* ablc_pre_int = (RkAiqAlgoPreAblcInt*)mPreInParam;
    RkAiqAlgoPreResAblcInt* ablc_pre_res_int = (RkAiqAlgoPreResAblcInt*)mPreOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqPreResComb* comb = &shared->preResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAblcHandle::preProcess();
    if (ret) {
        comb->ablc_pre_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "ablc handle preProcess failed");
    }

    comb->ablc_pre_res = NULL;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->pre_process(mPreInParam, mPreOutParam);
    RKAIQCORE_CHECK_RET(ret, "ablc algo pre_process failed");

    // set result to mAiqCore
    comb->ablc_pre_res = (RkAiqAlgoPreResAblc*)ablc_pre_res_int;

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqAblcHandleInt::processing()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAblcInt* ablc_proc_int = (RkAiqAlgoProcAblcInt*)mProcInParam;
    RkAiqAlgoProcResAblcInt* ablc_proc_res_int = (RkAiqAlgoProcResAblcInt*)mProcOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqProcResComb* comb = &shared->procResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAblcHandle::processing();
    if (ret) {
        comb->ablc_proc_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "ablc handle processing failed");
    }

    comb->ablc_proc_res = NULL;
    ablc_proc_int->iso = sharedCom->iso;
    ablc_proc_int->hdr_mode = sharedCom->working_mode;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->processing(mProcInParam, mProcOutParam);
    RKAIQCORE_CHECK_RET(ret, "ablc algo processing failed");

    comb->ablc_proc_res = (RkAiqAlgoProcResAblc*)ablc_proc_res_int;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAblcHandleInt::postProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPostAblcInt* ablc_post_int = (RkAiqAlgoPostAblcInt*)mPostInParam;
    RkAiqAlgoPostResAblcInt* ablc_post_res_int = (RkAiqAlgoPostResAblcInt*)mPostOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqPostResComb* comb = &shared->postResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAblcHandle::postProcess();
    if (ret) {
        comb->ablc_post_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "ablc handle postProcess failed");
        return ret;
    }

    comb->ablc_post_res = NULL;
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->post_process(mPostInParam, mPostOutParam);
    RKAIQCORE_CHECK_RET(ret, "ablc algo post_process failed");
    // set result to mAiqCore
    comb->ablc_post_res = (RkAiqAlgoPostResAblc*)ablc_post_res_int ;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

void
RkAiqAccmHandleInt::init()
{
    ENTER_ANALYZER_FUNCTION();

    RkAiqAccmHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAccmInt());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAccmInt());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAccmInt());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAccmInt());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAccmInt());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAccmInt());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAccmInt());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn
RkAiqAccmHandleInt::updateConfig(bool needSync)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (needSync)
        mCfgMutex.lock();
    // if something changed
    if (updateAtt) {
        mCurAtt = mNewAtt;
        updateAtt = false;
        // TODO
        rk_aiq_uapi_accm_SetAttrib(mAlgoCtx, mCurAtt, false);
        sendSignal();
    }

    if (needSync)
        mCfgMutex.unlock();


    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAccmHandleInt::setAttrib(rk_aiq_ccm_attrib_t att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    //TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed
    if (0 != memcmp(&mCurAtt, &att, sizeof(rk_aiq_ccm_attrib_t))) {
        mNewAtt = att;
        updateAtt = true;
        waitSignal();
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAccmHandleInt::getAttrib(rk_aiq_ccm_attrib_t *att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_accm_GetAttrib(mAlgoCtx, att);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAccmHandleInt::queryCcmInfo(rk_aiq_ccm_querry_info_t *ccm_querry_info )
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_accm_QueryCcmInfo(mAlgoCtx, ccm_querry_info);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}


XCamReturn
RkAiqAccmHandleInt::prepare()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqAccmHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "accm handle prepare failed");

    RkAiqAlgoConfigAccmInt* accm_config_int = (RkAiqAlgoConfigAccmInt*)mConfig;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "accm algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqAccmHandleInt::preProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPreAccmInt* accm_pre_int = (RkAiqAlgoPreAccmInt*)mPreInParam;
    RkAiqAlgoPreResAccmInt* accm_pre_res_int = (RkAiqAlgoPreResAccmInt*)mPreOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqPreResComb* comb = &shared->preResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAccmHandle::preProcess();
    if (ret) {
        comb->accm_pre_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "accm handle preProcess failed");
    }

    comb->accm_pre_res = NULL;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->pre_process(mPreInParam, mPreOutParam);
    RKAIQCORE_CHECK_RET(ret, "accm algo pre_process failed");

    // set result to mAiqCore
    comb->accm_pre_res = (RkAiqAlgoPreResAccm*)accm_pre_res_int;

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqAccmHandleInt::processing()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAccmInt* accm_proc_int = (RkAiqAlgoProcAccmInt*)mProcInParam;
    RkAiqAlgoProcResAccmInt* accm_proc_res_int = (RkAiqAlgoProcResAccmInt*)mProcOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqProcResComb* comb = &shared->procResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAccmHandle::processing();
    if (ret) {
        comb->accm_proc_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "accm handle processing failed");
    }

    comb->accm_proc_res = NULL;
    // TODO should check if the rk awb algo used
#if 0
    RkAiqAlgoProcResAwb* awb_res =
        (RkAiqAlgoProcResAwb*)(shared->procResComb.awb_proc_res);
    RkAiqAlgoProcResAwbInt* awb_res_int =
        (RkAiqAlgoProcResAwbInt*)(shared->procResComb.awb_proc_res);

    if(awb_res ) {
        if(awb_res->awb_gain_algo.grgain < DIVMIN ||
                awb_res->awb_gain_algo.gbgain < DIVMIN ) {
            LOGE("get wrong awb gain from AWB module ,use default value ");
        } else {
            accm_proc_int->accm_sw_info.awbGain[0] =
                awb_res->awb_gain_algo.rgain / awb_res->awb_gain_algo.grgain;

            accm_proc_int->accm_sw_info.awbGain[1] = awb_res->awb_gain_algo.bgain / awb_res->awb_gain_algo.gbgain;
        }
        accm_proc_int->accm_sw_info.awbIIRDampCoef =  awb_res_int->awb_smooth_factor;
        accm_proc_int->accm_sw_info.varianceLuma = awb_res_int->varianceLuma;
        accm_proc_int->accm_sw_info.awbConverged = awb_res_int->awbConverged;
    } else {
        LOGE("fail to get awb gain form AWB module,use default value ");
    }
    RkAiqAlgoPreResAeInt *ae_int = (RkAiqAlgoPreResAeInt*)shared->preResComb.ae_pre_res;
    if( ae_int) {
        if(sharedCom->working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
            accm_proc_int->accm_sw_info.sensorGain = ae_int->ae_pre_res_rk.LinearExp.exp_real_params.analog_gain
                    * ae_int->ae_pre_res_rk.LinearExp.exp_real_params.digital_gain
                    * ae_int->ae_pre_res_rk.LinearExp.exp_real_params.isp_dgain;
        } else if((rk_aiq_working_mode_t)sharedCom->working_mode >= RK_AIQ_WORKING_MODE_ISP_HDR2
                  && (rk_aiq_working_mode_t)sharedCom->working_mode < RK_AIQ_WORKING_MODE_ISP_HDR3)  {
            LOGD("%sensor gain choose from second hdr frame for accm");
            accm_proc_int->accm_sw_info.sensorGain = ae_int->ae_pre_res_rk.HdrExp[1].exp_real_params.analog_gain
                    * ae_int->ae_pre_res_rk.HdrExp[1].exp_real_params.digital_gain
                    * ae_int->ae_pre_res_rk.HdrExp[1].exp_real_params.isp_dgain;
        } else if((rk_aiq_working_mode_t)sharedCom->working_mode >= RK_AIQ_WORKING_MODE_ISP_HDR2
                  && (rk_aiq_working_mode_t)sharedCom->working_mode >= RK_AIQ_WORKING_MODE_ISP_HDR3)  {
            LOGD("sensor gain choose from third hdr frame for accm");
            accm_proc_int->accm_sw_info.sensorGain = ae_int->ae_pre_res_rk.HdrExp[2].exp_real_params.analog_gain
                    * ae_int->ae_pre_res_rk.HdrExp[2].exp_real_params.digital_gain
                    * ae_int->ae_pre_res_rk.HdrExp[2].exp_real_params.isp_dgain;
        } else {
            LOGE("working_mode (%d) is invaild ,fail to get sensor gain form AE module,use default value ",
                 sharedCom->working_mode);
        }
    } else {
        LOGE("fail to get sensor gain form AE module,use default value ");
    }
#else
    XCamVideoBuffer* xCamAwbProcRes = shared->res_comb.awb_proc_res;
    if (xCamAwbProcRes) {
        RkAiqAlgoProcResAwbInt* awb_res_int = (RkAiqAlgoProcResAwbInt*)xCamAwbProcRes->map(xCamAwbProcRes);
        RkAiqAlgoProcResAwb* awb_res = &awb_res_int->awb_proc_res_com;
        if(awb_res) {
            if(awb_res->awb_gain_algo.grgain < DIVMIN ||
                    awb_res->awb_gain_algo.gbgain < DIVMIN ) {
                LOGE("get wrong awb gain from AWB module ,use default value ");
            } else {
                accm_proc_int->accm_sw_info.awbGain[0] =
                    awb_res->awb_gain_algo.rgain / awb_res->awb_gain_algo.grgain;

                accm_proc_int->accm_sw_info.awbGain[1] =
                    awb_res->awb_gain_algo.bgain / awb_res->awb_gain_algo.gbgain;
            }
            accm_proc_int->accm_sw_info.awbIIRDampCoef =  awb_res_int->awb_smooth_factor;
            accm_proc_int->accm_sw_info.varianceLuma = awb_res_int->varianceLuma;
            accm_proc_int->accm_sw_info.awbConverged = awb_res_int->awbConverged;
        } else {
            LOGE("fail to get awb gain form AWB module,use default value ");
        }
    } else {
        LOGE("fail to get awb gain form AWB module,use default value ");
    }
    RKAiqAecExpInfo_t *pCurExp = &shared->curExp;
    if(pCurExp) {
        if((rk_aiq_working_mode_t)sharedCom->working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
            accm_proc_int->accm_sw_info.sensorGain = pCurExp->LinearExp.exp_real_params.analog_gain
                    * pCurExp->LinearExp.exp_real_params.digital_gain
                    * pCurExp->LinearExp.exp_real_params.isp_dgain;
        } else if((rk_aiq_working_mode_t)sharedCom->working_mode >= RK_AIQ_WORKING_MODE_ISP_HDR2
                  && (rk_aiq_working_mode_t)sharedCom->working_mode < RK_AIQ_WORKING_MODE_ISP_HDR3)  {
            LOGD("sensor gain choose from second hdr frame for accm");
            accm_proc_int->accm_sw_info.sensorGain = pCurExp->HdrExp[1].exp_real_params.analog_gain
                    * pCurExp->HdrExp[1].exp_real_params.digital_gain
                    * pCurExp->HdrExp[1].exp_real_params.isp_dgain;
        } else if((rk_aiq_working_mode_t)sharedCom->working_mode >= RK_AIQ_WORKING_MODE_ISP_HDR2
                  && (rk_aiq_working_mode_t)sharedCom->working_mode >= RK_AIQ_WORKING_MODE_ISP_HDR3)  {
            LOGD("sensor gain choose from third hdr frame for accm");
            accm_proc_int->accm_sw_info.sensorGain = pCurExp->HdrExp[2].exp_real_params.analog_gain
                    * pCurExp->HdrExp[2].exp_real_params.digital_gain
                    * pCurExp->HdrExp[2].exp_real_params.isp_dgain;
        } else {
            LOGE("working_mode (%d) is invaild ,fail to get sensor gain form AE module,use default value ",
                 sharedCom->working_mode);
        }
    } else {
        LOGE("fail to get sensor gain form AE module,use default value ");
    }
#endif

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->processing(mProcInParam, mProcOutParam);
    RKAIQCORE_CHECK_RET(ret, "accm algo processing failed");

    comb->accm_proc_res = (RkAiqAlgoProcResAccm*)accm_proc_res_int;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAccmHandleInt::postProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPostAccmInt* accm_post_int = (RkAiqAlgoPostAccmInt*)mPostInParam;
    RkAiqAlgoPostResAccmInt* accm_post_res_int = (RkAiqAlgoPostResAccmInt*)mPostOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqPostResComb* comb = &shared->postResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAccmHandle::postProcess();
    if (ret) {
        comb->accm_post_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "accm handle postProcess failed");
        return ret;
    }

    comb->accm_post_res = NULL;
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->post_process(mPostInParam, mPostOutParam);
    RKAIQCORE_CHECK_RET(ret, "accm algo post_process failed");
    // set result to mAiqCore
    comb->accm_post_res = (RkAiqAlgoPostResAccm*)accm_post_res_int ;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

void
RkAiqAcgcHandleInt::init()
{
    ENTER_ANALYZER_FUNCTION();

    RkAiqAcgcHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAcgcInt());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAcgcInt());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAcgcInt());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAcgcInt());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAcgcInt());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAcgcInt());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAcgcInt());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn
RkAiqAcgcHandleInt::prepare()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqAcgcHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "acgc handle prepare failed");

    RkAiqAlgoConfigAcgcInt* acgc_config_int = (RkAiqAlgoConfigAcgcInt*)mConfig;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "acgc algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqAcgcHandleInt::preProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPreAcgcInt* acgc_pre_int = (RkAiqAlgoPreAcgcInt*)mPreInParam;
    RkAiqAlgoPreResAcgcInt* acgc_pre_res_int = (RkAiqAlgoPreResAcgcInt*)mPreOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqPreResComb* comb = &shared->preResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAcgcHandle::preProcess();
    if (ret) {
        comb->acgc_pre_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "acgc handle preProcess failed");
    }

    comb->acgc_pre_res = NULL;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->pre_process(mPreInParam, mPreOutParam);
    RKAIQCORE_CHECK_RET(ret, "acgc algo pre_process failed");

    // set result to mAiqCore
    comb->acgc_pre_res = (RkAiqAlgoPreResAcgc*)acgc_pre_res_int;

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqAcgcHandleInt::processing()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAcgcInt* acgc_proc_int = (RkAiqAlgoProcAcgcInt*)mProcInParam;
    RkAiqAlgoProcResAcgcInt* acgc_proc_res_int = (RkAiqAlgoProcResAcgcInt*)mProcOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqProcResComb* comb = &shared->procResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAcgcHandle::processing();
    if (ret) {
        comb->acgc_proc_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "acgc handle processing failed");
    }

    comb->acgc_proc_res = NULL;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->processing(mProcInParam, mProcOutParam);
    RKAIQCORE_CHECK_RET(ret, "acgc algo processing failed");

    comb->acgc_proc_res = (RkAiqAlgoProcResAcgc*)acgc_proc_res_int;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAcgcHandleInt::postProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPostAcgcInt* acgc_post_int = (RkAiqAlgoPostAcgcInt*)mPostInParam;
    RkAiqAlgoPostResAcgcInt* acgc_post_res_int = (RkAiqAlgoPostResAcgcInt*)mPostOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqPostResComb* comb = &shared->postResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAcgcHandle::postProcess();
    if (ret) {
        comb->acgc_post_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "acgc handle postProcess failed");
        return ret;
    }

    comb->acgc_post_res = NULL;
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->post_process(mPostInParam, mPostOutParam);
    RKAIQCORE_CHECK_RET(ret, "acgc algo post_process failed");
    // set result to mAiqCore
    comb->acgc_post_res = (RkAiqAlgoPostResAcgc*)acgc_post_res_int ;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

void
RkAiqAdebayerHandleInt::init()
{
    ENTER_ANALYZER_FUNCTION();

    RkAiqAdebayerHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAdebayerInt());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAdebayerInt());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAdebayerInt());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAdebayerInt());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAdebayerInt());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAdebayerInt());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAdebayerInt());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn
RkAiqAdebayerHandleInt::updateConfig(bool needSync)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (needSync)
        mCfgMutex.lock();
    // if something changed
    if (updateAtt) {
        mCurAtt = mNewAtt;
        updateAtt = false;
        rk_aiq_uapi_adebayer_SetAttrib(mAlgoCtx, mCurAtt, false);
        sendSignal();
    }
    if (needSync)
        mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAdebayerHandleInt::setAttrib(adebayer_attrib_t att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    //TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore
    if (0 != memcmp(&mCurAtt, &att, sizeof(adebayer_attrib_t))) {
        mNewAtt = att;
        updateAtt = true;
        waitSignal();
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAdebayerHandleInt::getAttrib(adebayer_attrib_t *att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_adebayer_GetAttrib(mAlgoCtx, att);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAdebayerHandleInt::prepare()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqAdebayerHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "adebayer handle prepare failed");

    RkAiqAlgoConfigAdebayerInt* adebayer_config_int = (RkAiqAlgoConfigAdebayerInt*)mConfig;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "adebayer algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqAdebayerHandleInt::preProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPreAdebayerInt* adebayer_pre_int = (RkAiqAlgoPreAdebayerInt*)mPreInParam;
    RkAiqAlgoPreResAdebayerInt* adebayer_pre_res_int = (RkAiqAlgoPreResAdebayerInt*)mPreOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqPreResComb* comb = &shared->preResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAdebayerHandle::preProcess();
    if (ret) {
        comb->adebayer_pre_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "adebayer handle preProcess failed");
    }

    comb->adebayer_pre_res = NULL;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->pre_process(mPreInParam, mPreOutParam);
    RKAIQCORE_CHECK_RET(ret, "adebayer algo pre_process failed");

    // set result to mAiqCore
    comb->adebayer_pre_res = (RkAiqAlgoPreResAdebayer*)adebayer_pre_res_int;

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqAdebayerHandleInt::processing()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAdebayerInt* adebayer_proc_int = (RkAiqAlgoProcAdebayerInt*)mProcInParam;
    RkAiqAlgoProcResAdebayerInt* adebayer_proc_res_int = (RkAiqAlgoProcResAdebayerInt*)mProcOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqProcResComb* comb = &shared->procResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAdebayerHandle::processing();
    if (ret) {
        comb->adebayer_proc_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "adebayer handle processing failed");
    }

    comb->adebayer_proc_res = NULL;
    // TODO: fill procParam
    adebayer_proc_int->hdr_mode = sharedCom->working_mode;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->processing(mProcInParam, mProcOutParam);
    RKAIQCORE_CHECK_RET(ret, "adebayer algo processing failed");

    comb->adebayer_proc_res = (RkAiqAlgoProcResAdebayer*)adebayer_proc_res_int;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAdebayerHandleInt::postProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPostAdebayerInt* adebayer_post_int = (RkAiqAlgoPostAdebayerInt*)mPostInParam;
    RkAiqAlgoPostResAdebayerInt* adebayer_post_res_int = (RkAiqAlgoPostResAdebayerInt*)mPostOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqPostResComb* comb = &shared->postResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAdebayerHandle::postProcess();
    if (ret) {
        comb->adebayer_post_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "adebayer handle postProcess failed");
        return ret;
    }

    comb->adebayer_post_res = NULL;
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->post_process(mPostInParam, mPostOutParam);
    RKAIQCORE_CHECK_RET(ret, "adebayer algo post_process failed");
    // set result to mAiqCore
    comb->adebayer_post_res = (RkAiqAlgoPostResAdebayer*)adebayer_post_res_int ;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

void
RkAiqAdpccHandleInt::init()
{
    ENTER_ANALYZER_FUNCTION();

    RkAiqAdpccHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAdpccInt());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAdpccInt());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAdpccInt());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAdpccInt());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAdpccInt());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAdpccInt());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAdpccInt());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn
RkAiqAdpccHandleInt::updateConfig(bool needSync)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (needSync)
        mCfgMutex.lock();
    // if something changed
    if (updateAtt) {
        mCurAtt = mNewAtt;
        updateAtt = false;
        // TODO
        rk_aiq_uapi_adpcc_SetAttrib(mAlgoCtx, &mCurAtt, false);
        sendSignal();
    }

    if (needSync)
        mCfgMutex.unlock();


    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAdpccHandleInt::setAttrib(rk_aiq_dpcc_attrib_V20_t *att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    //TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed
    if (0 != memcmp(&mCurAtt, att, sizeof(rk_aiq_dpcc_attrib_V20_t))) {
        mNewAtt = *att;
        updateAtt = true;
        waitSignal();
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAdpccHandleInt::getAttrib(rk_aiq_dpcc_attrib_V20_t *att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_adpcc_GetAttrib(mAlgoCtx, att);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}



XCamReturn
RkAiqAdpccHandleInt::prepare()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqAdpccHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "adpcc handle prepare failed");

    RkAiqAlgoConfigAdpccInt* adpcc_config_int = (RkAiqAlgoConfigAdpccInt*)mConfig;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "adpcc algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqAdpccHandleInt::preProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPreAdpccInt* adpcc_pre_int = (RkAiqAlgoPreAdpccInt*)mPreInParam;
    RkAiqAlgoPreResAdpccInt* adpcc_pre_res_int = (RkAiqAlgoPreResAdpccInt*)mPreOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqPreResComb* comb = &shared->preResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAdpccHandle::preProcess();
    if (ret) {
        comb->adpcc_pre_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "adpcc handle preProcess failed");
    }

    comb->adpcc_pre_res = NULL;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->pre_process(mPreInParam, mPreOutParam);
    RKAIQCORE_CHECK_RET(ret, "adpcc algo pre_process failed");

    // set result to mAiqCore
    comb->adpcc_pre_res = (RkAiqAlgoPreResAdpcc*)adpcc_pre_res_int;

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqAdpccHandleInt::processing()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAdpccInt* adpcc_proc_int = (RkAiqAlgoProcAdpccInt*)mProcInParam;
    RkAiqAlgoProcResAdpccInt* adpcc_proc_res_int = (RkAiqAlgoProcResAdpccInt*)mProcOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqProcResComb* comb = &shared->procResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAdpccHandle::processing();
    if (ret) {
        comb->adpcc_proc_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "adpcc handle processing failed");
    }

    comb->adpcc_proc_res = NULL;
    // TODO: fill procParam
    adpcc_proc_int->iso = sharedCom->iso;
    adpcc_proc_int->hdr_mode = sharedCom->working_mode;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->processing(mProcInParam, mProcOutParam);
    RKAIQCORE_CHECK_RET(ret, "adpcc algo processing failed");

    comb->adpcc_proc_res = (RkAiqAlgoProcResAdpcc*)adpcc_proc_res_int;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAdpccHandleInt::postProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPostAsdInt* asd_post_int = (RkAiqAlgoPostAsdInt*)mPostInParam;
    RkAiqAlgoPostResAsdInt* asd_post_res_int = (RkAiqAlgoPostResAsdInt*)mPostOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqPostResComb* comb = &shared->postResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAdpccHandle::postProcess();
    if (ret) {
        comb->asd_post_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "asd handle postProcess failed");
        return ret;
    }

    comb->asd_post_res = NULL;
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->post_process(mPostInParam, mPostOutParam);
    RKAIQCORE_CHECK_RET(ret, "asd algo post_process failed");
    // set result to mAiqCore
    comb->asd_post_res = (RkAiqAlgoPostResAsd*)asd_post_res_int ;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

void
RkAiqAfecHandleInt::init()
{
    ENTER_ANALYZER_FUNCTION();

    RkAiqAfecHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAfecInt());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAfecInt());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAfecInt());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAfecInt());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAfecInt());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAfecInt());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAfecInt());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn
RkAiqAfecHandleInt::prepare()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqAfecHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "afec handle prepare failed");

    RkAiqAlgoConfigAfecInt* afec_config_int = (RkAiqAlgoConfigAfecInt*)mConfig;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqIspStats* ispStats = shared->ispStats;

    /* memcpy(&afec_config_int->afec_calib_cfg, &shared->calib->afec, sizeof(CalibDb_FEC_t)); */
    afec_config_int->resource_path = sharedCom->resourcePath;
    afec_config_int->mem_ops_ptr = mAiqCore->mShareMemOps;
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "afec algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqAfecHandleInt::preProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPreAfecInt* afec_pre_int = (RkAiqAlgoPreAfecInt*)mPreInParam;
    RkAiqAlgoPreResAfecInt* afec_pre_res_int = (RkAiqAlgoPreResAfecInt*)mPreOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqPreResComb* comb = &shared->preResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAfecHandle::preProcess();
    if (ret) {
        comb->afec_pre_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "afec handle preProcess failed");
    }

    comb->afec_pre_res = NULL;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->pre_process(mPreInParam, mPreOutParam);
    RKAIQCORE_CHECK_RET(ret, "afec algo pre_process failed");

    // set result to mAiqCore
    comb->afec_pre_res = (RkAiqAlgoPreResAfec*)afec_pre_res_int;

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqAfecHandleInt::processing()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAfecInt* afec_proc_int = (RkAiqAlgoProcAfecInt*)mProcInParam;
    RkAiqAlgoProcResAfecInt* afec_proc_res_int = (RkAiqAlgoProcResAfecInt*)mProcOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqProcResComb* comb = &shared->procResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAfecHandle::processing();
    if (ret) {
        comb->afec_proc_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "afec handle processing failed");
    }

    comb->afec_proc_res = NULL;
    //fill procParam
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->processing(mProcInParam, mProcOutParam);
    RKAIQCORE_CHECK_RET(ret, "afec algo processing failed");

    comb->afec_proc_res = (RkAiqAlgoProcResAfec*)afec_proc_res_int;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAfecHandleInt::postProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPostAfecInt* afec_post_int = (RkAiqAlgoPostAfecInt*)mPostInParam;
    RkAiqAlgoPostResAfecInt* afec_post_res_int = (RkAiqAlgoPostResAfecInt*)mPostOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqPostResComb* comb = &shared->postResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAfecHandle::postProcess();
    if (ret) {
        comb->afec_post_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "afec handle postProcess failed");
        return ret;
    }

    comb->afec_post_res = NULL;
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->post_process(mPostInParam, mPostOutParam);
    RKAIQCORE_CHECK_RET(ret, "afec algo post_process failed");
    // set result to mAiqCore
    comb->afec_post_res = (RkAiqAlgoPostResAfec*)afec_post_res_int ;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAfecHandleInt::updateConfig(bool needSync)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (needSync)
        mCfgMutex.lock();
    // if something changed
    if (updateAtt) {
        mCurAtt = mNewAtt;
        updateAtt = false;
        // TODO
        rk_aiq_uapi_afec_SetAttrib(mAlgoCtx, mCurAtt, false);
        sendSignal();
    }

    if (needSync)
        mCfgMutex.unlock();


    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAfecHandleInt::setAttrib(rk_aiq_fec_attrib_t att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    //TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed
    if (0 != memcmp(&mCurAtt, &att, sizeof(rk_aiq_fec_attrib_t))) {
        mNewAtt = att;
        updateAtt = true;
        waitSignal();
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAfecHandleInt::getAttrib(rk_aiq_fec_attrib_t *att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_afec_GetAttrib(mAlgoCtx, att);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

void
RkAiqAgammaHandleInt::init()
{
    ENTER_ANALYZER_FUNCTION();

    RkAiqAgammaHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAgammaInt());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAgammaInt());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAgammaInt());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAgammaInt());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAgammaInt());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAgammaInt());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAgammaInt());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn
RkAiqAgammaHandleInt::updateConfig(bool needSync)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (needSync)
        mCfgMutex.lock();
    // if something changed
    if (updateAtt) {
        mCurAtt = mNewAtt;
        updateAtt = false;
        // TODO
        rk_aiq_uapi_agamma_SetAttrib(mAlgoCtx, mCurAtt, false);
        waitSignal();
    }

    if (needSync)
        mCfgMutex.unlock();


    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAgammaHandleInt::setAttrib(rk_aiq_gamma_attrib_V2_t att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    //TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed
    if (0 != memcmp(&mCurAtt, &att, sizeof(rk_aiq_gamma_attrib_V2_t))) {
        mNewAtt = att;
        updateAtt = true;
        sendSignal();
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAgammaHandleInt::getAttrib(rk_aiq_gamma_attrib_V2_t *att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_agamma_GetAttrib(mAlgoCtx, att);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}


XCamReturn
RkAiqAgammaHandleInt::prepare()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqAgammaHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "agamma handle prepare failed");

    RkAiqAlgoConfigAgammaInt* agamma_config_int = (RkAiqAlgoConfigAgammaInt*)mConfig;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqIspStats* ispStats = shared->ispStats;

    agamma_config_int->calib = sharedCom->calib;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "agamma algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqAgammaHandleInt::preProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPreAgammaInt* agamma_pre_int = (RkAiqAlgoPreAgammaInt*)mPreInParam;
    RkAiqAlgoPreResAgammaInt* agamma_pre_res_int = (RkAiqAlgoPreResAgammaInt*)mPreOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqPreResComb* comb = &shared->preResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAgammaHandle::preProcess();
    if (ret) {
        comb->agamma_pre_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "agamma handle preProcess failed");
    }

    comb->agamma_pre_res = NULL;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->pre_process(mPreInParam, mPreOutParam);
    RKAIQCORE_CHECK_RET(ret, "agamma algo pre_process failed");

    // set result to mAiqCore
    comb->agamma_pre_res = (RkAiqAlgoPreResAgamma*)agamma_pre_res_int;

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqAgammaHandleInt::processing()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAgammaInt* agamma_proc_int = (RkAiqAlgoProcAgammaInt*)mProcInParam;
    RkAiqAlgoProcResAgammaInt* agamma_proc_res_int = (RkAiqAlgoProcResAgammaInt*)mProcOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqProcResComb* comb = &shared->procResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAgammaHandle::processing();
    if (ret) {
        comb->agamma_proc_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "agamma handle processing failed");
    }

    comb->agamma_proc_res = NULL;
    agamma_proc_int->calib = sharedCom->calib;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->processing(mProcInParam, mProcOutParam);
    RKAIQCORE_CHECK_RET(ret, "agamma algo processing failed");

    comb->agamma_proc_res = (RkAiqAlgoProcResAgamma*)agamma_proc_res_int;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAgammaHandleInt::postProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPostAgammaInt* agamma_post_int = (RkAiqAlgoPostAgammaInt*)mPostInParam;
    RkAiqAlgoPostResAgammaInt* agamma_post_res_int = (RkAiqAlgoPostResAgammaInt*)mPostOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqPostResComb* comb = &shared->postResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAgammaHandle::postProcess();
    if (ret) {
        comb->agamma_post_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "agamma handle postProcess failed");
        return ret;
    }

    comb->agamma_post_res = NULL;
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->post_process(mPostInParam, mPostOutParam);
    RKAIQCORE_CHECK_RET(ret, "agamma algo post_process failed");
    // set result to mAiqCore
    comb->agamma_post_res = (RkAiqAlgoPostResAgamma*)agamma_post_res_int ;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

void
RkAiqAdegammaHandleInt::init()
{
    ENTER_ANALYZER_FUNCTION();

    RkAiqAdegammaHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAdegammaInt());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAdegammaInt());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAdegammaInt());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAdegammaInt());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAdegammaInt());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAdegammaInt());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAdegammaInt());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn
RkAiqAdegammaHandleInt::updateConfig(bool needSync)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (needSync)
        mCfgMutex.lock();
    // if something changed
    if (updateAtt) {
        mCurAtt = mNewAtt;
        updateAtt = false;
        // TODO
        rk_aiq_uapi_adegamma_SetAttrib(mAlgoCtx, mCurAtt, false);
        waitSignal();
    }

    if (needSync)
        mCfgMutex.unlock();


    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAdegammaHandleInt::setAttrib(rk_aiq_degamma_attrib_t att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    //TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed
    if (0 != memcmp(&mCurAtt, &att, sizeof(rk_aiq_degamma_attrib_t))) {
        mNewAtt = att;
        updateAtt = true;
        sendSignal();
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAdegammaHandleInt::getAttrib(rk_aiq_degamma_attrib_t *att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_adegamma_GetAttrib(mAlgoCtx, att);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}


XCamReturn
RkAiqAdegammaHandleInt::prepare()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqAdegammaHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "adegamma handle prepare failed");

    RkAiqAlgoConfigAdegammaInt* adegamma_config_int = (RkAiqAlgoConfigAdegammaInt*)mConfig;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    adegamma_config_int->calib = sharedCom->calib;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "adegamma algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqAdegammaHandleInt::preProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPreAdegammaInt* adegamma_pre_int = (RkAiqAlgoPreAdegammaInt*)mPreInParam;
    RkAiqAlgoPreResAdegammaInt* adegamma_pre_res_int = (RkAiqAlgoPreResAdegammaInt*)mPreOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqPreResComb* comb = &shared->preResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAdegammaHandle::preProcess();
    if (ret) {
        comb->adegamma_pre_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "adegamma handle preProcess failed");
    }

    comb->adegamma_pre_res = NULL;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->pre_process(mPreInParam, mPreOutParam);
    RKAIQCORE_CHECK_RET(ret, "adegamma algo pre_process failed");

    // set result to mAiqCore
    comb->adegamma_pre_res = (RkAiqAlgoPreResAdegamma*)adegamma_pre_res_int;

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqAdegammaHandleInt::processing()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAdegammaInt* adegamma_proc_int = (RkAiqAlgoProcAdegammaInt*)mProcInParam;
    RkAiqAlgoProcResAdegammaInt* adegamma_proc_res_int = (RkAiqAlgoProcResAdegammaInt*)mProcOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqProcResComb* comb = &shared->procResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAdegammaHandle::processing();
    if (ret) {
        comb->adegamma_proc_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "adegamma handle processing failed");
    }

    comb->adegamma_proc_res = NULL;
    adegamma_proc_int->calib = sharedCom->calib;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->processing(mProcInParam, mProcOutParam);
    RKAIQCORE_CHECK_RET(ret, "adegamma algo processing failed");

    comb->adegamma_proc_res = (RkAiqAlgoProcResAdegamma*)adegamma_proc_res_int;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAdegammaHandleInt::postProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPostAdegammaInt* adegamma_post_int = (RkAiqAlgoPostAdegammaInt*)mPostInParam;
    RkAiqAlgoPostResAdegammaInt* adegamma_post_res_int = (RkAiqAlgoPostResAdegammaInt*)mPostOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqPostResComb* comb = &shared->postResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAdegammaHandle::postProcess();
    if (ret) {
        comb->adegamma_post_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "adegamma handle postProcess failed");
        return ret;
    }

    comb->adegamma_post_res = NULL;
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->post_process(mPostInParam, mPostOutParam);
    RKAIQCORE_CHECK_RET(ret, "agamma algo post_process failed");
    // set result to mAiqCore
    comb->adegamma_post_res = (RkAiqAlgoPostResAdegamma*)adegamma_post_res_int ;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

void
RkAiqAgicHandleInt::init()
{
    ENTER_ANALYZER_FUNCTION();

    RkAiqAgicHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAgicInt());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAgicInt());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAgicInt());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAgicInt());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAgicInt());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAgicInt());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAgicInt());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn
RkAiqAgicHandleInt::updateConfig(bool needSync)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (needSync)
        mCfgMutex.lock();
    // if something changed
    if (updateAtt) {
        mCurAtt = mNewAtt;
        updateAtt = false;
        rk_aiq_uapi_agic_SetAttrib(mAlgoCtx, mCurAtt, false);
        sendSignal();
    }
    if (needSync)
        mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAgicHandleInt::setAttrib(agic_attrib_t att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    //TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore
    if (0 != memcmp(&mCurAtt, &att, sizeof(agic_attrib_t))) {
        mNewAtt = att;
        updateAtt = true;
        waitSignal();
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAgicHandleInt::getAttrib(agic_attrib_t *att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_agic_GetAttrib(mAlgoCtx, att);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAgicHandleInt::prepare()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqAgicHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "agic handle prepare failed");

    RkAiqAlgoConfigAgicInt* agic_config_int = (RkAiqAlgoConfigAgicInt*)mConfig;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "agic algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqAgicHandleInt::preProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPreAgicInt* agic_pre_int = (RkAiqAlgoPreAgicInt*)mPreInParam;
    RkAiqAlgoPreResAgicInt* agic_pre_res_int = (RkAiqAlgoPreResAgicInt*)mPreOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqPreResComb* comb = &shared->preResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAgicHandle::preProcess();
    if (ret) {
        comb->agic_pre_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "agic handle preProcess failed");
    }

    comb->agic_pre_res = NULL;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->pre_process(mPreInParam, mPreOutParam);
    RKAIQCORE_CHECK_RET(ret, "agic algo pre_process failed");

    // set result to mAiqCore
    comb->agic_pre_res = (RkAiqAlgoPreResAgic*)agic_pre_res_int;

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqAgicHandleInt::processing()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAgicInt* agic_proc_int = (RkAiqAlgoProcAgicInt*)mProcInParam;
    RkAiqAlgoProcResAgicInt* agic_proc_res_int = (RkAiqAlgoProcResAgicInt*)mProcOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqProcResComb* comb = &shared->procResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAgicHandle::processing();
    if (ret) {
        comb->agic_proc_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "agic handle processing failed");
    }

    comb->agic_proc_res = NULL;
    agic_proc_int->hdr_mode = sharedCom->working_mode;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->processing(mProcInParam, mProcOutParam);
    RKAIQCORE_CHECK_RET(ret, "agic algo processing failed");

    comb->agic_proc_res = (RkAiqAlgoProcResAgic*)agic_proc_res_int;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAgicHandleInt::postProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPostAgicInt* agic_post_int = (RkAiqAlgoPostAgicInt*)mPostInParam;
    RkAiqAlgoPostResAgicInt* agic_post_res_int = (RkAiqAlgoPostResAgicInt*)mPostOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqPostResComb* comb = &shared->postResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAgicHandle::postProcess();
    if (ret) {
        comb->agic_post_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "agic handle postProcess failed");
        return ret;
    }

    comb->agic_post_res = NULL;
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->post_process(mPostInParam, mPostOutParam);
    RKAIQCORE_CHECK_RET(ret, "agic algo post_process failed");
    // set result to mAiqCore
    comb->agic_post_res = (RkAiqAlgoPostResAgic*)agic_post_res_int ;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAieHandleInt::updateConfig(bool needSync)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (needSync)
        mCfgMutex.lock();
    // if something changed
    if (updateAtt) {
        mCurAtt = mNewAtt;
        updateAtt = false;
        rk_aiq_uapi_aie_SetAttrib(mAlgoCtx, mCurAtt, false);
        sendSignal();
    }
    if (needSync)
        mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAieHandleInt::setAttrib(aie_attrib_t att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore
    if (0 != memcmp(&mCurAtt, &att, sizeof(aie_attrib_t))) {
        mNewAtt = att;
        updateAtt = true;
        waitSignal();
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAieHandleInt::getAttrib(aie_attrib_t *att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_aie_GetAttrib(mAlgoCtx, att);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

void
RkAiqAieHandleInt::init()
{
    ENTER_ANALYZER_FUNCTION();

    RkAiqAieHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAieInt());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAieInt());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAieInt());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAieInt());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAieInt());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAieInt());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAieInt());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn
RkAiqAieHandleInt::prepare()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqAieHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "aie handle prepare failed");

    RkAiqAlgoConfigAieInt* aie_config_int = (RkAiqAlgoConfigAieInt*)mConfig;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "aie algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqAieHandleInt::preProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPreAieInt* aie_pre_int = (RkAiqAlgoPreAieInt*)mPreInParam;
    RkAiqAlgoPreResAieInt* aie_pre_res_int = (RkAiqAlgoPreResAieInt*)mPreOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqPreResComb* comb = &shared->preResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAieHandle::preProcess();
    if (ret) {
        comb->aie_pre_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "aie handle preProcess failed");
    }

    comb->aie_pre_res = NULL;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->pre_process(mPreInParam, mPreOutParam);
    RKAIQCORE_CHECK_RET(ret, "aie algo pre_process failed");

    // set result to mAiqCore
    comb->aie_pre_res = (RkAiqAlgoPreResAie*)aie_pre_res_int;

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqAieHandleInt::processing()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAieInt* aie_proc_int = (RkAiqAlgoProcAieInt*)mProcInParam;
    RkAiqAlgoProcResAieInt* aie_proc_res_int = (RkAiqAlgoProcResAieInt*)mProcOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqProcResComb* comb = &shared->procResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAieHandle::processing();
    if (ret) {
        comb->aie_proc_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "aie handle processing failed");
    }

    comb->aie_proc_res = NULL;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->processing(mProcInParam, mProcOutParam);
    RKAIQCORE_CHECK_RET(ret, "aie algo processing failed");

    comb->aie_proc_res = (RkAiqAlgoProcResAie*)aie_proc_res_int;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAieHandleInt::postProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPostAieInt* aie_post_int = (RkAiqAlgoPostAieInt*)mPostInParam;
    RkAiqAlgoPostResAieInt* aie_post_res_int = (RkAiqAlgoPostResAieInt*)mPostOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqPostResComb* comb = &shared->postResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAieHandle::postProcess();
    if (ret) {
        comb->aie_post_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "aie handle postProcess failed");
        return ret;
    }

    comb->aie_post_res = NULL;
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->post_process(mPostInParam, mPostOutParam);
    RKAIQCORE_CHECK_RET(ret, "aie algo post_process failed");
    // set result to mAiqCore
    comb->aie_post_res = (RkAiqAlgoPostResAie*)aie_post_res_int ;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

void
RkAiqAldchHandleInt::init()
{
    ENTER_ANALYZER_FUNCTION();

    RkAiqAldchHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAldchInt());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAldchInt());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAldchInt());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAldchInt());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAldchInt());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAldchInt());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAldchInt());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn
RkAiqAldchHandleInt::prepare()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqAldchHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "aldch handle prepare failed");

    RkAiqAlgoConfigAldchInt* aldch_config_int = (RkAiqAlgoConfigAldchInt*)mConfig;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    // memcpy(&aldch_config_int->aldch_calib_cfg, &shared->calib->aldch, sizeof(CalibDb_LDCH_t));
    aldch_config_int->resource_path = sharedCom->resourcePath;
    aldch_config_int->mem_ops_ptr = mAiqCore->mShareMemOps;
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "aldch algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqAldchHandleInt::preProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPreAldchInt* aldch_pre_int = (RkAiqAlgoPreAldchInt*)mPreInParam;
    RkAiqAlgoPreResAldchInt* aldch_pre_res_int = (RkAiqAlgoPreResAldchInt*)mPreOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqPreResComb* comb = &shared->preResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAldchHandle::preProcess();
    if (ret) {
        comb->aldch_pre_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "aldch handle preProcess failed");
    }

    comb->aldch_pre_res = NULL;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->pre_process(mPreInParam, mPreOutParam);
    RKAIQCORE_CHECK_RET(ret, "aldch algo pre_process failed");

    // set result to mAiqCore
    comb->aldch_pre_res = (RkAiqAlgoPreResAldch*)aldch_pre_res_int;

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqAldchHandleInt::processing()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAldchInt* aldch_proc_int = (RkAiqAlgoProcAldchInt*)mProcInParam;
    RkAiqAlgoProcResAldchInt* aldch_proc_res_int = (RkAiqAlgoProcResAldchInt*)mProcOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqProcResComb* comb = &shared->procResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAldchHandle::processing();
    if (ret) {
        comb->aldch_proc_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "aldch handle processing failed");
    }

    comb->aldch_proc_res = NULL;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->processing(mProcInParam, mProcOutParam);
    RKAIQCORE_CHECK_RET(ret, "aldch algo processing failed");

    comb->aldch_proc_res = (RkAiqAlgoProcResAldch*)aldch_proc_res_int;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAldchHandleInt::postProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPostAldchInt* aldch_post_int = (RkAiqAlgoPostAldchInt*)mPostInParam;
    RkAiqAlgoPostResAldchInt* aldch_post_res_int = (RkAiqAlgoPostResAldchInt*)mPostOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqPostResComb* comb = &shared->postResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAldchHandle::postProcess();
    if (ret) {
        comb->aldch_post_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "aldch handle postProcess failed");
        return ret;
    }

    comb->aldch_post_res = NULL;
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->post_process(mPostInParam, mPostOutParam);
    RKAIQCORE_CHECK_RET(ret, "aldch algo post_process failed");
    // set result to mAiqCore
    comb->aldch_post_res = (RkAiqAlgoPostResAldch*)aldch_post_res_int ;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAldchHandleInt::updateConfig(bool needSync)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (needSync)
        mCfgMutex.lock();
    // if something changed
    if (updateAtt) {
        mCurAtt = mNewAtt;
        updateAtt = false;
        // TODO
        rk_aiq_uapi_aldch_SetAttrib(mAlgoCtx, mCurAtt, false);
        sendSignal();
    }

    if (needSync)
        mCfgMutex.unlock();


    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAldchHandleInt::setAttrib(rk_aiq_ldch_attrib_t att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    //TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be efldchtive later when updateConfig
    // called by RkAiqCore

    // if something changed
    if (0 != memcmp(&mCurAtt, &att, sizeof(rk_aiq_ldch_attrib_t))) {
        mNewAtt = att;
        updateAtt = true;
        waitSignal();
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAldchHandleInt::getAttrib(rk_aiq_ldch_attrib_t *att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_aldch_GetAttrib(mAlgoCtx, att);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

void
RkAiqAlscHandleInt::init()
{
    ENTER_ANALYZER_FUNCTION();

    RkAiqAlscHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAlscInt());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAlscInt());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAlscInt());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAlscInt());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAlscInt());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAlscInt());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAlscInt());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn
RkAiqAlscHandleInt::updateConfig(bool needSync)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (needSync)
        mCfgMutex.lock();
    // if something changed
    if (updateAtt) {
        mCurAtt = mNewAtt;
        updateAtt = false;
        // TODO
        rk_aiq_uapi_alsc_SetAttrib(mAlgoCtx, mCurAtt, false);
        sendSignal();
    }

    if (needSync)
        mCfgMutex.unlock();


    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAlscHandleInt::setAttrib(rk_aiq_lsc_attrib_t att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    //TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed
    if (0 != memcmp(&mCurAtt, &att, sizeof(rk_aiq_lsc_attrib_t))) {
        mNewAtt = att;
        updateAtt = true;
        waitSignal();
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAlscHandleInt::getAttrib(rk_aiq_lsc_attrib_t *att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_alsc_GetAttrib(mAlgoCtx, att);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAlscHandleInt::queryLscInfo(rk_aiq_lsc_querry_info_t *lsc_querry_info )
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_alsc_QueryLscInfo(mAlgoCtx, lsc_querry_info);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAlscHandleInt::prepare()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqAlscHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "alsc handle prepare failed");

    RkAiqAlgoConfigAlscInt* alsc_config_int = (RkAiqAlgoConfigAlscInt*)mConfig;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "alsc algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqAlscHandleInt::preProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPreAlscInt* alsc_pre_int = (RkAiqAlgoPreAlscInt*)mPreInParam;
    RkAiqAlgoPreResAlscInt* alsc_pre_res_int = (RkAiqAlgoPreResAlscInt*)mPreOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqPreResComb* comb = &shared->preResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAlscHandle::preProcess();
    if (ret) {
        comb->alsc_pre_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "alsc handle preProcess failed");
    }

    comb->alsc_pre_res = NULL;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->pre_process(mPreInParam, mPreOutParam);
    RKAIQCORE_CHECK_RET(ret, "alsc algo pre_process failed");

    // set result to mAiqCore
    comb->alsc_pre_res = (RkAiqAlgoPreResAlsc*)alsc_pre_res_int;

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqAlscHandleInt::processing()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAlscInt* alsc_proc_int = (RkAiqAlgoProcAlscInt*)mProcInParam;
    RkAiqAlgoProcResAlscInt* alsc_proc_res_int = (RkAiqAlgoProcResAlscInt*)mProcOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqProcResComb* comb = &shared->procResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAlscHandle::processing();
    if (ret) {
        comb->alsc_proc_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "alsc handle processing failed");
    }

    comb->alsc_proc_res = NULL;

    // TODO should check if the rk awb algo used
#if 0
    RkAiqAlgoProcResAwb* awb_res =
        (RkAiqAlgoProcResAwb*)(shared->procResComb.awb_proc_res);
    RkAiqAlgoProcResAwbInt* awb_res_int =
        (RkAiqAlgoProcResAwbInt*)(shared->procResComb.awb_proc_res);
    if(awb_res) {
        if(awb_res->awb_gain_algo.grgain < DIVMIN ||
                awb_res->awb_gain_algo.gbgain < DIVMIN ) {
            LOGE("get wrong awb gain from AWB module ,use default value ");
        } else {
            alsc_proc_int->alsc_sw_info.awbGain[0] =
                awb_res->awb_gain_algo.rgain / awb_res->awb_gain_algo.grgain;

            alsc_proc_int->alsc_sw_info.awbGain[1] =
                awb_res->awb_gain_algo.bgain / awb_res->awb_gain_algo.gbgain;
        }
        alsc_proc_int->alsc_sw_info.awbIIRDampCoef = awb_res_int->awb_smooth_factor;
        alsc_proc_int->alsc_sw_info.varianceLuma = awb_res_int->varianceLuma;
        alsc_proc_int->alsc_sw_info.awbConverged = awb_res_int->awbConverged;
    } else {
        LOGE("fail to get awb gain form AWB module,use default value ");
    }
    RkAiqAlgoPreResAeInt *ae_int = (RkAiqAlgoPreResAeInt*)shared->preResComb.ae_pre_res;
    if( ae_int) {
        if((rk_aiq_working_mode_t)sharedCom->working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
            alsc_proc_int->alsc_sw_info.sensorGain = ae_int->ae_pre_res_rk.LinearExp.exp_real_params.analog_gain
                    * ae_int->ae_pre_res_rk.LinearExp.exp_real_params.digital_gain
                    * ae_int->ae_pre_res_rk.LinearExp.exp_real_params.isp_dgain;
        } else if((rk_aiq_working_mode_t)sharedCom->working_mode >= RK_AIQ_WORKING_MODE_ISP_HDR2
                  && (rk_aiq_working_mode_t)sharedCom->working_mode < RK_AIQ_WORKING_MODE_ISP_HDR3)  {
            LOGD("sensor gain choose from second hdr frame for alsc");
            alsc_proc_int->alsc_sw_info.sensorGain = ae_int->ae_pre_res_rk.HdrExp[1].exp_real_params.analog_gain
                    * ae_int->ae_pre_res_rk.HdrExp[1].exp_real_params.digital_gain
                    * ae_int->ae_pre_res_rk.HdrExp[1].exp_real_params.isp_dgain;
        } else if((rk_aiq_working_mode_t)sharedCom->working_mode >= RK_AIQ_WORKING_MODE_ISP_HDR2
                  && (rk_aiq_working_mode_t)sharedCom->working_mode >= RK_AIQ_WORKING_MODE_ISP_HDR3)  {
            LOGD("sensor gain choose from third hdr frame for alsc");
            alsc_proc_int->alsc_sw_info.sensorGain = ae_int->ae_pre_res_rk.HdrExp[2].exp_real_params.analog_gain
                    * ae_int->ae_pre_res_rk.HdrExp[2].exp_real_params.digital_gain
                    * ae_int->ae_pre_res_rk.HdrExp[2].exp_real_params.isp_dgain;
        } else {
            LOGE("working_mode (%d) is invaild ,fail to get sensor gain form AE module,use default value ",
                 sharedCom->working_mode);
        }
    } else {
        LOGE("fail to get sensor gain form AE module,use default value ");
    }
#else
    alsc_proc_int->tx = shared->tx;
    XCamVideoBuffer* awb_proc_res = shared->res_comb.awb_proc_res;
    if (awb_proc_res) {
        RkAiqAlgoProcResAwbInt* awb_res_int = (RkAiqAlgoProcResAwbInt*)awb_proc_res->map(awb_proc_res);
        RkAiqAlgoProcResAwb* awb_res = &awb_res_int->awb_proc_res_com;
        if(awb_res) {
            if(awb_res->awb_gain_algo.grgain < DIVMIN ||
                    awb_res->awb_gain_algo.gbgain < DIVMIN ) {
                LOGE("get wrong awb gain from AWB module ,use default value ");
            } else {
                alsc_proc_int->alsc_sw_info.awbGain[0] =
                    awb_res->awb_gain_algo.rgain / awb_res->awb_gain_algo.grgain;

                alsc_proc_int->alsc_sw_info.awbGain[1] =
                    awb_res->awb_gain_algo.bgain / awb_res->awb_gain_algo.gbgain;
            }
            alsc_proc_int->alsc_sw_info.awbIIRDampCoef = awb_res_int->awb_smooth_factor;
            alsc_proc_int->alsc_sw_info.varianceLuma = awb_res_int->varianceLuma;
            alsc_proc_int->alsc_sw_info.awbConverged = awb_res_int->awbConverged;
        } else {
            LOGE("fail to get awb gain form AWB module,use default value ");
        }
    } else {
        LOGE("fail to get awb gain form AWB module,use default value ");
    }

    RKAiqAecExpInfo_t *pCurExp = &shared->curExp;
    if(pCurExp) {
        if((rk_aiq_working_mode_t)sharedCom->working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
            alsc_proc_int->alsc_sw_info.sensorGain = pCurExp->LinearExp.exp_real_params.analog_gain
                    * pCurExp->LinearExp.exp_real_params.digital_gain
                    * pCurExp->LinearExp.exp_real_params.isp_dgain;
        } else if((rk_aiq_working_mode_t)sharedCom->working_mode >= RK_AIQ_WORKING_MODE_ISP_HDR2
                  && (rk_aiq_working_mode_t)sharedCom->working_mode < RK_AIQ_WORKING_MODE_ISP_HDR3)  {
            LOGD("sensor gain choose from second hdr frame for alsc");
            alsc_proc_int->alsc_sw_info.sensorGain = pCurExp->HdrExp[1].exp_real_params.analog_gain
                    * pCurExp->HdrExp[1].exp_real_params.digital_gain
                    * pCurExp->HdrExp[1].exp_real_params.isp_dgain;
        } else if((rk_aiq_working_mode_t)sharedCom->working_mode >= RK_AIQ_WORKING_MODE_ISP_HDR2
                  && (rk_aiq_working_mode_t)sharedCom->working_mode >= RK_AIQ_WORKING_MODE_ISP_HDR3)  {
            LOGD("sensor gain choose from third hdr frame for alsc");
            alsc_proc_int->alsc_sw_info.sensorGain = pCurExp->HdrExp[2].exp_real_params.analog_gain
                    * pCurExp->HdrExp[2].exp_real_params.digital_gain
                    * pCurExp->HdrExp[2].exp_real_params.isp_dgain;
        } else {
            LOGE("working_mode (%d) is invaild ,fail to get sensor gain form AE module,use default value ",
                 sharedCom->working_mode);
        }
    } else {
        LOGE("fail to get sensor gain form AE module,use default value ");
    }
#endif

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->processing(mProcInParam, mProcOutParam);
    RKAIQCORE_CHECK_RET(ret, "alsc algo processing failed");

    comb->alsc_proc_res = (RkAiqAlgoProcResAlsc*)alsc_proc_res_int;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAlscHandleInt::postProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPostAlscInt* alsc_post_int = (RkAiqAlgoPostAlscInt*)mPostInParam;
    RkAiqAlgoPostResAlscInt* alsc_post_res_int = (RkAiqAlgoPostResAlscInt*)mPostOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqPostResComb* comb = &shared->postResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAlscHandle::postProcess();
    if (ret) {
        comb->alsc_post_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "alsc handle postProcess failed");
        return ret;
    }

    comb->alsc_post_res = NULL;
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->post_process(mPostInParam, mPostOutParam);
    RKAIQCORE_CHECK_RET(ret, "alsc algo post_process failed");
    // set result to mAiqCore
    comb->alsc_post_res = (RkAiqAlgoPostResAlsc*)alsc_post_res_int ;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

void
RkAiqAr2yHandleInt::init()
{
    ENTER_ANALYZER_FUNCTION();

    RkAiqAr2yHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAr2yInt());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAr2yInt());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAr2yInt());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAr2yInt());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAr2yInt());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAr2yInt());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAr2yInt());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn
RkAiqAr2yHandleInt::prepare()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqAr2yHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "ar2y handle prepare failed");

    RkAiqAlgoConfigAr2yInt* ar2y_config_int = (RkAiqAlgoConfigAr2yInt*)mConfig;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "ar2y algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqAr2yHandleInt::preProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPreAr2yInt* ar2y_pre_int = (RkAiqAlgoPreAr2yInt*)mPreInParam;
    RkAiqAlgoPreResAr2yInt* ar2y_pre_res_int = (RkAiqAlgoPreResAr2yInt*)mPreOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqPreResComb* comb = &shared->preResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAr2yHandle::preProcess();
    if (ret) {
        comb->ar2y_pre_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "ar2y handle preProcess failed");
    }

    comb->ar2y_pre_res = NULL;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->pre_process(mPreInParam, mPreOutParam);
    RKAIQCORE_CHECK_RET(ret, "ar2y algo pre_process failed");

    // set result to mAiqCore
    comb->ar2y_pre_res = (RkAiqAlgoPreResAr2y*)ar2y_pre_res_int;

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqAr2yHandleInt::processing()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAr2yInt* ar2y_proc_int = (RkAiqAlgoProcAr2yInt*)mProcInParam;
    RkAiqAlgoProcResAr2yInt* ar2y_proc_res_int = (RkAiqAlgoProcResAr2yInt*)mProcOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqProcResComb* comb = &shared->procResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAr2yHandle::processing();
    if (ret) {
        comb->ar2y_proc_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "ar2y handle processing failed");
    }

    comb->ar2y_proc_res = NULL;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->processing(mProcInParam, mProcOutParam);
    RKAIQCORE_CHECK_RET(ret, "ar2y algo processing failed");

    comb->ar2y_proc_res = (RkAiqAlgoProcResAr2y*)ar2y_proc_res_int;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAr2yHandleInt::postProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPostAr2yInt* ar2y_post_int = (RkAiqAlgoPostAr2yInt*)mPostInParam;
    RkAiqAlgoPostResAr2yInt* ar2y_post_res_int = (RkAiqAlgoPostResAr2yInt*)mPostOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqPostResComb* comb = &shared->postResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAr2yHandle::postProcess();
    if (ret) {
        comb->ar2y_post_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "ar2y handle postProcess failed");
        return ret;
    }

    comb->ar2y_post_res = NULL;
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->post_process(mPostInParam, mPostOutParam);
    RKAIQCORE_CHECK_RET(ret, "ar2y algo post_process failed");
    // set result to mAiqCore
    comb->ar2y_post_res = (RkAiqAlgoPostResAr2y*)ar2y_post_res_int ;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAwdrHandleInt::prepare()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqAwdrHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "awdr handle prepare failed");

    RkAiqAlgoConfigAwdrInt* awdr_config_int = (RkAiqAlgoConfigAwdrInt*)mConfig;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "awdr algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

void
RkAiqAwdrHandleInt::init()
{
    ENTER_ANALYZER_FUNCTION();

    RkAiqAwdrHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAwdrInt());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAwdrInt());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAwdrInt());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAwdrInt());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAwdrInt());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAwdrInt());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAwdrInt());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn
RkAiqAwdrHandleInt::preProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPreAwdrInt* awdr_pre_int = (RkAiqAlgoPreAwdrInt*)mPreInParam;
    RkAiqAlgoPreResAwdrInt* awdr_pre_res_int = (RkAiqAlgoPreResAwdrInt*)mPreOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqPreResComb* comb = &shared->preResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAwdrHandle::preProcess();
    if (ret) {
        comb->awdr_pre_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "awdr handle preProcess failed");
    }

    comb->awdr_pre_res = NULL;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->pre_process(mPreInParam, mPreOutParam);
    RKAIQCORE_CHECK_RET(ret, "awdr algo pre_process failed");

    // set result to mAiqCore
    comb->awdr_pre_res = (RkAiqAlgoPreResAwdr*)awdr_pre_res_int;

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqAwdrHandleInt::processing()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAwdrInt* awdr_proc_int = (RkAiqAlgoProcAwdrInt*)mProcInParam;
    RkAiqAlgoProcResAwdrInt* awdr_proc_res_int = (RkAiqAlgoProcResAwdrInt*)mProcOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqProcResComb* comb = &shared->procResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAwdrHandle::processing();
    if (ret) {
        comb->awdr_proc_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "awdr handle processing failed");
    }

    comb->awdr_proc_res = NULL;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->processing(mProcInParam, mProcOutParam);
    RKAIQCORE_CHECK_RET(ret, "awdr algo processing failed");

    comb->awdr_proc_res = (RkAiqAlgoProcResAwdr*)awdr_proc_res_int;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAwdrHandleInt::postProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPostAwdrInt* awdr_post_int = (RkAiqAlgoPostAwdrInt*)mPostInParam;
    RkAiqAlgoPostResAwdrInt* awdr_post_res_int = (RkAiqAlgoPostResAwdrInt*)mPostOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqPostResComb* comb = &shared->postResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAwdrHandle::postProcess();
    if (ret) {
        comb->awdr_post_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "awdr handle postProcess failed");
        return ret;
    }

    comb->awdr_post_res = NULL;
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->post_process(mPostInParam, mPostOutParam);
    RKAIQCORE_CHECK_RET(ret, "awdr algo post_process failed");
    // set result to mAiqCore
    comb->awdr_post_res = (RkAiqAlgoPostResAwdr*)awdr_post_res_int ;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}


XCamReturn
RkAiqAdrcHandleInt::prepare()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqAdrcHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "adrc handle prepare failed");

    RkAiqAlgoConfigAdrcInt* adrc_config_int = (RkAiqAlgoConfigAdrcInt*)mConfig;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    adrc_config_int->rawHeight = sharedCom->snsDes.isp_acq_height;
    adrc_config_int->rawWidth = sharedCom->snsDes.isp_acq_width;
    adrc_config_int->working_mode = sharedCom->working_mode;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "adrc algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

void
RkAiqAdrcHandleInt::init()
{
    ENTER_ANALYZER_FUNCTION();

    RkAiqAdrcHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAdrcInt());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAdrcInt());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAdrcInt());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAdrcInt());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAdrcInt());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAdrcInt());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAdrcInt());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn
RkAiqAdrcHandleInt::updateConfig(bool needSync)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (needSync)
        mCfgMutex.lock();
    // if something changed
    if (updateAtt) {
        mCurAtt = mNewAtt;
        updateAtt = false;
        rk_aiq_uapi_adrc_SetAttrib(mAlgoCtx, mCurAtt, true);
        sendSignal();
    }
    if (needSync)
        mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAdrcHandleInt::setAttrib(drc_attrib_t att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    //TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed
    if (0 != memcmp(&mCurAtt, &att, sizeof(drc_attrib_t))) {
        mNewAtt = att;
        updateAtt = true;
        waitSignal();
    }
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
XCamReturn
RkAiqAdrcHandleInt::getAttrib(drc_attrib_t* att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_adrc_GetAttrib(mAlgoCtx, att);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAdrcHandleInt::preProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPreAdrcInt* adrc_pre_int = (RkAiqAlgoPreAdrcInt*)mPreInParam;
    RkAiqAlgoPreResAdrcInt* adrc_pre_res_int = (RkAiqAlgoPreResAdrcInt*)mPreOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqPreResComb* comb = &shared->preResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAdrcHandle::preProcess();
    if (ret) {
        comb->adrc_pre_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "adrc handle preProcess failed");
    }

    comb->adrc_pre_res = NULL;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->pre_process(mPreInParam, mPreOutParam);
    RKAIQCORE_CHECK_RET(ret, "adrc algo pre_process failed");

    // set result to mAiqCore
    comb->adrc_pre_res = (RkAiqAlgoPreResAdrc*)adrc_pre_res_int;

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqAdrcHandleInt::processing()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAdrcInt* adrc_proc_int = (RkAiqAlgoProcAdrcInt*)mProcInParam;
    RkAiqAlgoProcResAdrcInt* adrc_proc_res_int = (RkAiqAlgoProcResAdrcInt*)mProcOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqProcResComb* comb = &shared->procResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAdrcHandle::processing();
    if (ret) {
        comb->adrc_proc_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "adrc handle processing failed");
    }

    comb->adrc_proc_res = NULL;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->processing(mProcInParam, mProcOutParam);
    RKAIQCORE_CHECK_RET(ret, "adrc algo processing failed");

    comb->adrc_proc_res = (RkAiqAlgoProcResAdrc*)adrc_proc_res_int;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAdrcHandleInt::postProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPostAdrcInt* adrc_post_int = (RkAiqAlgoPostAdrcInt*)mPostInParam;
    RkAiqAlgoPostResAdrcInt* adrc_post_res_int = (RkAiqAlgoPostResAdrcInt*)mPostOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqPostResComb* comb = &shared->postResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAdrcHandle::postProcess();
    if (ret) {
        comb->adrc_post_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "adrc handle postProcess failed");
        return ret;
    }

    comb->adrc_post_res = NULL;
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->post_process(mPostInParam, mPostOutParam);
    RKAIQCORE_CHECK_RET(ret, "adrc algo post_process failed");
    // set result to mAiqCore
    comb->adrc_post_res = (RkAiqAlgoPostResAdrc*)adrc_post_res_int ;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAmdHandleInt::prepare()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqAmdHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "amd handle prepare failed");

    RkAiqAlgoConfigAmdInt* amd_config_int = (RkAiqAlgoConfigAmdInt*)mConfig;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    amd_config_int->amd_config_com.spWidth = sharedCom->spWidth;
    amd_config_int->amd_config_com.spHeight = sharedCom->spHeight;
    amd_config_int->amd_config_com.spAlignedW = sharedCom->spAlignedWidth;
    amd_config_int->amd_config_com.spAlignedH = sharedCom->spAlignedHeight;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "amd algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

void
RkAiqAmdHandleInt::init()
{
    ENTER_ANALYZER_FUNCTION();

    RkAiqAmdHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAmdInt());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAmdInt());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAmdInt());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAmdInt());
    // mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAmdInt());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAmdInt());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAmdInt());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn
RkAiqAmdHandleInt::updateConfig(bool needSync)
{
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqAmdHandleInt::preProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPreAmdInt* amd_pre_int = (RkAiqAlgoPreAmdInt*)mPreInParam;
    RkAiqAlgoPreResAmdInt* amd_pre_res_int = (RkAiqAlgoPreResAmdInt*)mPreOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqPreResComb* comb = &shared->preResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAmdHandle::preProcess();
    if (ret) {
        comb->amd_pre_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "amd handle preProcess failed");
    }

    comb->amd_pre_res = NULL;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->pre_process(mPreInParam, mPreOutParam);
    RKAIQCORE_CHECK_RET(ret, "amd algo pre_process failed");

    // set result to mAiqCore
    comb->amd_pre_res = (RkAiqAlgoPreResAmd*)amd_pre_res_int;

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqAmdHandleInt::processing()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAmdInt* amd_proc_int = (RkAiqAlgoProcAmdInt*)mProcInParam;

    mProcResShared = new RkAiqAlgoProcResAmdIntShared();
    if (!mProcResShared.ptr()) {
        LOGE("new amd mProcOutParam failed, bypass!");
        return XCAM_RETURN_BYPASS;
    }
    RkAiqAlgoProcResAmdInt* amd_proc_res_int = &mProcResShared->result;

    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqProcResComb* comb = &shared->procResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAmdHandle::processing();
    if (ret) {
        comb->amd_proc_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "amd handle processing failed");
    }

    comb->amd_proc_res = NULL;
    memset(&amd_proc_res_int->amd_proc_res_com.amd_proc_res, 0, sizeof(amd_proc_res_int->amd_proc_res_com.amd_proc_res));
    amd_proc_int->stats.spImage = shared->sp;
    amd_proc_int->stats.ispGain = shared->ispGain;
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->processing(mProcInParam, (RkAiqAlgoResCom*)amd_proc_res_int);
    RKAIQCORE_CHECK_RET(ret, "amd algo processing failed");

    comb->amd_proc_res = (RkAiqAlgoProcResAmd*)amd_proc_res_int;

    MediaBuffer_t *mbuf = amd_proc_res_int->amd_proc_res_com.amd_proc_res.st_ratio;
    if (mbuf) {
        MotionBufMetaData_t *metadata = (MotionBufMetaData_t *)mbuf->pMetaData;
        SmartPtr<XCamMessage> msg = new RkAiqCoreVdBufMsg(XCAM_MESSAGE_AMD_PROC_RES_OK,
                metadata->frame_id,
                mProcResShared);
        mAiqCore->post_message(msg);
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAmdHandleInt::postProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPostAmdInt* amd_post_int = (RkAiqAlgoPostAmdInt*)mPostInParam;
    RkAiqAlgoPostResAmdInt* amd_post_res_int = (RkAiqAlgoPostResAmdInt*)mPostOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqPostResComb* comb = &shared->postResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqAmdHandle::postProcess();
    if (ret) {
        comb->amd_post_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "amd handle postProcess failed");
        return ret;
    }

    comb->amd_post_res = NULL;
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->post_process(mPostInParam, mPostOutParam);
    RKAIQCORE_CHECK_RET(ret, "amd algo post_process failed");
    // set result to mAiqCore
    comb->amd_post_res = (RkAiqAlgoPostResAmd*)amd_post_res_int ;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

void
RkAiqArawnrHandleInt::init()
{
    ENTER_ANALYZER_FUNCTION();

    RkAiqArawnrHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigArawnrInt());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreArawnrInt());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResArawnrInt());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcArawnrInt());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResArawnrInt());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostArawnrInt());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResArawnrInt());

    EXIT_ANALYZER_FUNCTION();
}


XCamReturn
RkAiqArawnrHandleInt::updateConfig(bool needSync)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (needSync)
        mCfgMutex.lock();
    // if something changed
    if (updateAtt) {
        mCurAtt = mNewAtt;
        updateAtt = false;
        rk_aiq_uapi_abayernr_SetAttrib_v1(mAlgoCtx, &mCurAtt, false);
        sendSignal();
    }

    if(updateIQpara) {
        mCurIQPara = mNewIQPara;
        updateIQpara = false;
        // TODO
        rk_aiq_uapi_abayernr_SetIQPara_v1(mAlgoCtx, &mCurIQPara, false);
        sendSignal();
    }

    if(updateJsonpara) {
        mCurJsonPara = mNewJsonPara;
        updateJsonpara = false;
        // TODO
        rk_aiq_uapi_abayernr_SetJsonPara_v1(mAlgoCtx, &mCurJsonPara, false);
        sendSignal();
    }

    if (needSync)
        mCfgMutex.unlock();


    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqArawnrHandleInt::setAttrib(rk_aiq_bayernr_attrib_v1_t *att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    //TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed
    if (0 != memcmp(&mCurAtt, att, sizeof(rk_aiq_bayernr_attrib_v1_t))) {
        mNewAtt = *att;
        updateAtt = true;
        waitSignal();
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqArawnrHandleInt::getAttrib(rk_aiq_bayernr_attrib_v1_t *att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_abayernr_GetAttrib_v1(mAlgoCtx, att);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqArawnrHandleInt::setIQPara(rk_aiq_bayernr_IQPara_V1_t *para)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    //TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed
    if (0 != memcmp(&mCurIQPara, para, sizeof(rk_aiq_bayernr_IQPara_V1_t))) {
        mNewIQPara = *para;
        updateIQpara = true;
        waitSignal();
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqArawnrHandleInt::getIQPara(rk_aiq_bayernr_IQPara_V1_t *para)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    printf("rawnr get iq para enter\n");
    rk_aiq_uapi_abayernr_GetIQPara_v1(mAlgoCtx, para);
    printf("rawnr get iq para exit\n");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqArawnrHandleInt::setJsonPara(rk_aiq_bayernr_JsonPara_V1_t *para)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    //TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed
    if (0 != memcmp(&mCurIQPara, para, sizeof(rk_aiq_bayernr_JsonPara_V1_t))) {
        mNewJsonPara = *para;
        updateJsonpara = true;
        waitSignal();
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqArawnrHandleInt::getJsonPara(rk_aiq_bayernr_JsonPara_V1_t *para)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_abayernr_GetJsonPara_v1(mAlgoCtx, para);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}



XCamReturn
RkAiqArawnrHandleInt::setStrength(float fPercent)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_abayernr_SetRawnrSFStrength_v1(mAlgoCtx, fPercent);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}


XCamReturn
RkAiqArawnrHandleInt::getStrength(float *pPercent)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_abayernr_GetRawnrSFStrength_v1(mAlgoCtx, pPercent);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}


XCamReturn
RkAiqArawnrHandleInt::prepare()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqArawnrHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "arawnr handle prepare failed");

    RkAiqAlgoConfigArawnrInt* aynr_config_int = (RkAiqAlgoConfigArawnrInt*)mConfig;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "arawnr algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqArawnrHandleInt::preProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPreArawnrInt* arawnr_pre_int = (RkAiqAlgoPreArawnrInt*)mPreInParam;
    RkAiqAlgoPreResArawnrInt* arawnr_pre_res_int = (RkAiqAlgoPreResArawnrInt*)mPreOutParam;

    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqPreResComb* comb = &shared->preResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;


    ret = RkAiqArawnrHandle::preProcess();
    if (ret) {
        comb->arawnr_pre_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "arawnr handle preProcess failed");
    }

    comb->arawnr_pre_res = NULL;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->pre_process(mPreInParam, mPreOutParam);
    RKAIQCORE_CHECK_RET(ret, "arawnr algo pre_process failed");
    // set result to mAiqCore
    comb->arawnr_pre_res = (RkAiqAlgoPreResArawnr*)arawnr_pre_res_int;

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqArawnrHandleInt::processing()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcArawnrInt* arawnr_proc_int = (RkAiqAlgoProcArawnrInt*)mProcInParam;
    RkAiqAlgoProcResArawnrInt* arawnr_proc_res_int = (RkAiqAlgoProcResArawnrInt*)mProcOutParam;

    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqProcResComb* comb = &shared->procResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    static int arawnr_proc_framecnt = 0;
    arawnr_proc_framecnt++;

    ret = RkAiqArawnrHandle::processing();
    if (ret) {
        comb->arawnr_proc_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "aynr handle processing failed");
    }

    comb->arawnr_proc_res = NULL;

    // TODO: fill procParam
    arawnr_proc_int->iso = sharedCom->iso;
    arawnr_proc_int->hdr_mode = sharedCom->working_mode;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->processing(mProcInParam, mProcOutParam);
    RKAIQCORE_CHECK_RET(ret, "aynr algo processing failed");

    comb->arawnr_proc_res = (RkAiqAlgoProcResArawnr*)arawnr_proc_res_int;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqArawnrHandleInt::postProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPostArawnrInt* arawnr_post_int = (RkAiqAlgoPostArawnrInt*)mPostInParam;
    RkAiqAlgoPostResArawnrInt* arawnr_post_res_int = (RkAiqAlgoPostResArawnrInt*)mPostOutParam;

    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqPostResComb* comb = &shared->postResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqArawnrHandle::postProcess();
    if (ret) {
        comb->aynr_post_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "arawnr handle postProcess failed");
        return ret;
    }

    comb->aynr_post_res = NULL;
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->post_process(mPostInParam, mPostOutParam);
    RKAIQCORE_CHECK_RET(ret, "arawnr algo post_process failed");
    // set result to mAiqCore
    comb->arawnr_post_res = (RkAiqAlgoPostResArawnr*)arawnr_post_res_int ;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

void
RkAiqAynrHandleInt::init()
{
    ENTER_ANALYZER_FUNCTION();

    RkAiqAynrHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAynrInt());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAynrInt());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAynrInt());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAynrInt());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAynrInt());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAynrInt());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAynrInt());

    EXIT_ANALYZER_FUNCTION();
}


XCamReturn
RkAiqAynrHandleInt::updateConfig(bool needSync)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (needSync)
        mCfgMutex.lock();
    // if something changed
    if (updateAtt) {
        mCurAtt = mNewAtt;
        updateAtt = false;
        // TODO
        rk_aiq_uapi_aynr_SetAttrib_v1(mAlgoCtx, &mCurAtt, false);
        sendSignal();
    }

    if(updateIQpara) {
        mCurIQPara = mNewIQPara;
        updateIQpara = false;
        // TODO
        rk_aiq_uapi_aynr_SetIQPara_v1(mAlgoCtx, &mCurIQPara, false);
        sendSignal();
    }

    if(updateJsonpara) {
        mCurJsonPara = mNewJsonPara;
        updateJsonpara = false;
        // TODO
        rk_aiq_uapi_aynr_SetJsonPara_v1(mAlgoCtx, &mCurJsonPara, false);
        sendSignal();
    }

    if (needSync)
        mCfgMutex.unlock();


    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAynrHandleInt::setAttrib(rk_aiq_ynr_attrib_v1_t *att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    //TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed
    if (0 != memcmp(&mCurAtt, att, sizeof(rk_aiq_ynr_attrib_v1_t))) {
        mNewAtt = *att;
        updateAtt = true;
        waitSignal();
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAynrHandleInt::getAttrib(rk_aiq_ynr_attrib_v1_t *att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_aynr_GetAttrib_v1(mAlgoCtx, att);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAynrHandleInt::setIQPara(rk_aiq_ynr_IQPara_V1_t *para)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    //TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed
    if (0 != memcmp(&mCurIQPara, para, sizeof(rk_aiq_ynr_IQPara_V1_t))) {
        mNewIQPara = *para;
        updateIQpara = true;
        waitSignal();
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAynrHandleInt::getIQPara(rk_aiq_ynr_IQPara_V1_t *para)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_aynr_GetIQPara_v1(mAlgoCtx, para);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAynrHandleInt::setJsonPara(rk_aiq_ynr_JsonPara_V1_t *para)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    //TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed
    if (0 != memcmp(&mCurIQPara, para, sizeof(rk_aiq_ynr_JsonPara_V1_t))) {
        mNewJsonPara = *para;
        updateJsonpara = true;
        waitSignal();
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAynrHandleInt::getJsonPara(rk_aiq_ynr_JsonPara_V1_t *para)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_aynr_GetJsonPara_v1(mAlgoCtx, para);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAynrHandleInt::setStrength(float fPercent)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_aynr_SetLumaSFStrength_v1(mAlgoCtx, fPercent);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}


XCamReturn
RkAiqAynrHandleInt::getStrength(float *pPercent)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_aynr_GetLumaSFStrength_v1(mAlgoCtx, pPercent);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}


XCamReturn
RkAiqAynrHandleInt::prepare()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqAynrHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "aynr handle prepare failed");

    RkAiqAlgoConfigAynrInt* aynr_config_int = (RkAiqAlgoConfigAynrInt*)mConfig;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "aynr algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqAynrHandleInt::preProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPreAynrInt* aynr_pre_int = (RkAiqAlgoPreAynrInt*)mPreInParam;
    RkAiqAlgoPreResAynrInt* aynr_pre_res_int = (RkAiqAlgoPreResAynrInt*)mPreOutParam;

    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqPreResComb* comb = &shared->preResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAynrHandle::preProcess();
    if (ret) {
        comb->aynr_pre_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "aynr handle preProcess failed");
    }

    comb->aynr_pre_res = NULL;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->pre_process(mPreInParam, mPreOutParam);
    RKAIQCORE_CHECK_RET(ret, "aynr algo pre_process failed");
    // set result to mAiqCore
    comb->aynr_pre_res = (RkAiqAlgoPreResAynr*)aynr_pre_res_int;

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqAynrHandleInt::processing()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAynrInt* aynr_proc_int = (RkAiqAlgoProcAynrInt*)mProcInParam;
    RkAiqAlgoProcResAynrInt* aynr_proc_res_int = (RkAiqAlgoProcResAynrInt*)mProcOutParam;

    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqProcResComb* comb = &shared->procResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    static int aynr_proc_framecnt = 0;
    aynr_proc_framecnt++;

    ret = RkAiqAynrHandle::processing();
    if (ret) {
        comb->aynr_proc_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "aynr handle processing failed");
    }

    comb->aynr_proc_res = NULL;

    // TODO: fill procParam
    aynr_proc_int->iso = sharedCom->iso;
    aynr_proc_int->hdr_mode = sharedCom->working_mode;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->processing(mProcInParam, mProcOutParam);
    RKAIQCORE_CHECK_RET(ret, "aynr algo processing failed");

    comb->aynr_proc_res = (RkAiqAlgoProcResAynr*)aynr_proc_res_int;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAynrHandleInt::postProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPostAynrInt* aynr_post_int = (RkAiqAlgoPostAynrInt*)mPostInParam;
    RkAiqAlgoPostResAynrInt* aynr_post_res_int = (RkAiqAlgoPostResAynrInt*)mPostOutParam;

    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqPostResComb* comb = &shared->postResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAynrHandle::postProcess();
    if (ret) {
        comb->aynr_post_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "aynr handle postProcess failed");
        return ret;
    }

    comb->aynr_post_res = NULL;
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->post_process(mPostInParam, mPostOutParam);
    RKAIQCORE_CHECK_RET(ret, "aynr algo post_process failed");
    // set result to mAiqCore
    comb->aynr_post_res = (RkAiqAlgoPostResAynr*)aynr_post_res_int ;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

//uvnr
void
RkAiqAcnrHandleInt::init()
{
    ENTER_ANALYZER_FUNCTION();

    RkAiqAcnrHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAcnrInt());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAcnrInt());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAcnrInt());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAcnrInt());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAcnrInt());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAcnrInt());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAcnrInt());

    EXIT_ANALYZER_FUNCTION();
}


XCamReturn
RkAiqAcnrHandleInt::updateConfig(bool needSync)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (needSync)
        mCfgMutex.lock();
    // if something changed
    if (updateAtt) {
        mCurAtt = mNewAtt;
        updateAtt = false;
        // TODO
        rk_aiq_uapi_auvnr_SetAttrib(mAlgoCtx, &mCurAtt, false);
        sendSignal();
    }

    if(updateIQpara) {
        mCurIQPara = mNewIQPara;
        updateIQpara = false;
        // TODO
        rk_aiq_uapi_auvnr_SetIQPara(mAlgoCtx, &mCurIQPara, false);
        sendSignal();
    }

    if(updateJsonpara) {
        mCurJsonPara = mNewJsonPara;
        updateJsonpara = false;
        // TODO
        rk_aiq_uapi_auvnr_SetJsonPara(mAlgoCtx, &mCurJsonPara, false);
        sendSignal();
    }

    if (needSync)
        mCfgMutex.unlock();


    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAcnrHandleInt::setAttrib(rk_aiq_uvnr_attrib_v1_t *att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    //TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed
    if (0 != memcmp(&mCurAtt, att, sizeof(rk_aiq_uvnr_attrib_v1_t))) {
        mNewAtt = *att;
        updateAtt = true;
        waitSignal();
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAcnrHandleInt::getAttrib(rk_aiq_uvnr_attrib_v1_t *att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_auvnr_GetAttrib(mAlgoCtx, att);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAcnrHandleInt::setIQPara(rk_aiq_uvnr_IQPara_v1_t *para)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    //TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed
    if (0 != memcmp(&mCurIQPara, para, sizeof(rk_aiq_uvnr_IQPara_v1_t))) {
        mNewIQPara = *para;
        updateIQpara = true;
        waitSignal();
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAcnrHandleInt::getIQPara(rk_aiq_uvnr_IQPara_v1_t *para)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_auvnr_GetIQPara(mAlgoCtx, para);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAcnrHandleInt::setJsonPara(rk_aiq_uvnr_JsonPara_v1_t *para)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    //TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed
    if (0 != memcmp(&mCurIQPara, para, sizeof(rk_aiq_uvnr_JsonPara_v1_t))) {
        mNewJsonPara = *para;
        updateJsonpara = true;
        waitSignal();
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAcnrHandleInt::getJsonPara(rk_aiq_uvnr_JsonPara_v1_t *para)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_auvnr_GetJsonPara(mAlgoCtx, para);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}



XCamReturn
RkAiqAcnrHandleInt::setStrength(float fPercent)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    //rk_aiq_uapi_asharp_SetStrength(mAlgoCtx, fPercent);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}


XCamReturn
RkAiqAcnrHandleInt::getStrength(float *pPercent)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    //rk_aiq_uapi_asharp_GetStrength(mAlgoCtx, pPercent);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}


XCamReturn
RkAiqAcnrHandleInt::prepare()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqAcnrHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "auvnr handle prepare failed");

    RkAiqAlgoConfigAcnrInt* auvnr_config_int = (RkAiqAlgoConfigAcnrInt*)mConfig;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "auvnr algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqAcnrHandleInt::preProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPreAcnrInt* auvnr_pre_int = (RkAiqAlgoPreAcnrInt*)mPreInParam;
    RkAiqAlgoPreResAcnrInt* auvnr_pre_res_int = (RkAiqAlgoPreResAcnrInt*)mPreOutParam;

    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqPreResComb* comb = &shared->preResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;


    ret = RkAiqAcnrHandle::preProcess();
    if (ret) {
        comb->acnr_pre_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "auvnr handle preProcess failed");
    }

    comb->acnr_pre_res = NULL;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->pre_process(mPreInParam, mPreOutParam);
    RKAIQCORE_CHECK_RET(ret, "auvnr algo pre_process failed");
    // set result to mAiqCore
    comb->acnr_pre_res = (RkAiqAlgoPreResAcnr*)auvnr_pre_res_int;

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqAcnrHandleInt::processing()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAcnrInt* auvnr_proc_int = (RkAiqAlgoProcAcnrInt*)mProcInParam;
    RkAiqAlgoProcResAcnrInt* auvnr_proc_res_int = (RkAiqAlgoProcResAcnrInt*)mProcOutParam;

    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqProcResComb* comb = &shared->procResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    static int auvnr_proc_framecnt = 0;
    auvnr_proc_framecnt++;

    ret = RkAiqAcnrHandle::processing();
    if (ret) {
        comb->acnr_proc_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "auvnr handle processing failed");
    }

    comb->acnr_proc_res = NULL;

    // TODO: fill procParam
    auvnr_proc_int->iso = sharedCom->iso;
    auvnr_proc_int->hdr_mode = sharedCom->working_mode;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->processing(mProcInParam, mProcOutParam);
    RKAIQCORE_CHECK_RET(ret, "aynr algo processing failed");

    comb->acnr_proc_res = (RkAiqAlgoProcResAcnr*)auvnr_proc_res_int;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAcnrHandleInt::postProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPostAcnrInt* auvnr_post_int = (RkAiqAlgoPostAcnrInt*)mPostInParam;
    RkAiqAlgoPostResAcnrInt* auvnr_post_res_int = (RkAiqAlgoPostResAcnrInt*)mPostOutParam;

    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqPostResComb* comb = &shared->postResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAcnrHandle::postProcess();
    if (ret) {
        comb->acnr_post_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "auvnr handle postProcess failed");
        return ret;
    }

    comb->acnr_post_res = NULL;
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->post_process(mPostInParam, mPostOutParam);
    RKAIQCORE_CHECK_RET(ret, "auvnr algo post_process failed");
    // set result to mAiqCore
    comb->acnr_post_res = (RkAiqAlgoPostResAcnr*)auvnr_post_res_int ;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

void
RkAiqAmfnrHandleInt::init()
{
    ENTER_ANALYZER_FUNCTION();

    RkAiqAmfnrHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAmfnrInt());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAmfnrInt());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAmfnrInt());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAmfnrInt());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAmfnrInt());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAmfnrInt());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAmfnrInt());

    EXIT_ANALYZER_FUNCTION();
}



XCamReturn
RkAiqAmfnrHandleInt::updateConfig(bool needSync)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (needSync)
        mCfgMutex.lock();
    // if something changed
    if (updateAtt) {
        mCurAtt = mNewAtt;
        updateAtt = false;
        // TODO
        rk_aiq_uapi_amfnr_SetAttrib_v1(mAlgoCtx, &mCurAtt, false);
        sendSignal();
    }

    if(updateIQpara) {
        mCurIQPara = mNewIQPara;
        updateIQpara = false;
        // TODO
        rk_aiq_uapi_amfnr_SetIQPara_v1(mAlgoCtx, &mCurIQPara, false);
        sendSignal();
    }

    if(updateJsonpara) {
        mCurJsonPara = mNewJsonPara;
        updateJsonpara = false;
        // TODO
        rk_aiq_uapi_amfnr_SetJsonPara_v1(mAlgoCtx, &mCurJsonPara, false);
        sendSignal();
    }

    if (needSync)
        mCfgMutex.unlock();


    EXIT_ANALYZER_FUNCTION();
    return ret;
}


XCamReturn
RkAiqAmfnrHandleInt::setAttrib(rk_aiq_mfnr_attrib_v1_t *att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    //TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed
    if (0 != memcmp(&mCurAtt, att, sizeof(rk_aiq_mfnr_attrib_v1_t))) {
        mNewAtt = *att;
        updateAtt = true;
        waitSignal();
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAmfnrHandleInt::getAttrib(rk_aiq_mfnr_attrib_v1_t *att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_amfnr_GetAttrib_v1(mAlgoCtx, att);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAmfnrHandleInt::setIQPara(rk_aiq_mfnr_IQPara_V1_t *para)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    //TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed
    if (0 != memcmp(&mCurIQPara, para, sizeof(rk_aiq_mfnr_IQPara_V1_t))) {
        mNewIQPara = *para;
        updateIQpara = true;
        waitSignal();
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAmfnrHandleInt::getIQPara(rk_aiq_mfnr_IQPara_V1_t *para)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_amfnr_GetIQPara_v1(mAlgoCtx, para);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAmfnrHandleInt::setJsonPara(rk_aiq_mfnr_JsonPara_V1_t *para)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    //TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed
    if (0 != memcmp(&mCurIQPara, para, sizeof(rk_aiq_mfnr_JsonPara_V1_t))) {
        mNewJsonPara = *para;
        updateJsonpara = true;
        waitSignal();
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAmfnrHandleInt::getJsonPara(rk_aiq_mfnr_JsonPara_V1_t *para)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_amfnr_GetJsonPara_v1(mAlgoCtx, para);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}



XCamReturn
RkAiqAmfnrHandleInt::setLumaStrength(float fPercent)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_amfnr_SetLumaTFStrength_v1(mAlgoCtx, fPercent);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}


XCamReturn
RkAiqAmfnrHandleInt::getLumaStrength(float *pPercent)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_amfnr_GetLumaTFStrength_v1(mAlgoCtx, pPercent);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAmfnrHandleInt::setChromaStrength(float fPercent)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_amfnr_SetChromaTFStrength_v1(mAlgoCtx, fPercent);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}


XCamReturn
RkAiqAmfnrHandleInt::getChromaStrength(float *pPercent)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_amfnr_GetChromaTFStrength_v1(mAlgoCtx, pPercent);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}



XCamReturn
RkAiqAmfnrHandleInt::prepare()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqAmfnrHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "amfnr handle prepare failed");

    RkAiqAlgoConfigAmfnrInt* amfnr_config_int = (RkAiqAlgoConfigAmfnrInt*)mConfig;

    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "amfnr algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}


XCamReturn
RkAiqAmfnrHandleInt::preProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPreAmfnrInt* amfnr_pre_int = (RkAiqAlgoPreAmfnrInt*)mPreInParam;
    RkAiqAlgoPreResAmfnrInt* amfnr_pre_res_int = (RkAiqAlgoPreResAmfnrInt*)mPreOutParam;

    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqPreResComb* comb = &shared->preResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAmfnrHandle::preProcess();
    if (ret) {
        comb->amfnr_pre_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "amfnr handle preProcess failed");
    }

    comb->amfnr_pre_res = NULL;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->pre_process(mPreInParam, mPreOutParam);
    RKAIQCORE_CHECK_RET(ret, "amfnr algo pre_process failed");

    // set result to mAiqCore
    comb->amfnr_pre_res = (RkAiqAlgoPreResAmfnr*)amfnr_pre_res_int;

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqAmfnrHandleInt::processing()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAmfnrInt* amfnr_proc_int = (RkAiqAlgoProcAmfnrInt*)mProcInParam;
    RkAiqAlgoProcResAmfnrInt* amfnr_proc_res_int = (RkAiqAlgoProcResAmfnrInt*)mProcOutParam;

    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqProcResComb* comb = &shared->procResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAmfnrHandle::processing();
    if (ret) {
        comb->amfnr_proc_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "amfnr handle processing failed");
    }

    comb->amfnr_proc_res = NULL;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->processing(mProcInParam, mProcOutParam);
    RKAIQCORE_CHECK_RET(ret, "amfnr algo processing failed");

    comb->amfnr_proc_res = (RkAiqAlgoProcResAmfnr*)amfnr_proc_res_int;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAmfnrHandleInt::postProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPostAmfnrInt* amfnr_post_int = (RkAiqAlgoPostAmfnrInt*)mPostInParam;
    RkAiqAlgoPostResAmfnrInt* amfnr_post_res_int = (RkAiqAlgoPostResAmfnrInt*)mPostOutParam;

    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqPostResComb* comb = &shared->postResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAmfnrHandle::postProcess();
    if (ret) {
        comb->amfnr_post_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "amfnr handle postProcess failed");
        return ret;
    }

    comb->amfnr_post_res = NULL;
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->post_process(mPostInParam, mPostOutParam);
    RKAIQCORE_CHECK_RET(ret, "amfnr algo post_process failed");
    // set result to mAiqCore
    comb->amfnr_post_res = (RkAiqAlgoPostResAmfnr*)amfnr_post_res_int ;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

void
RkAiqAgainHandleInt::init()
{
    ENTER_ANALYZER_FUNCTION();

    RkAiqAgainHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAgainInt());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAgainInt());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAgainInt());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAgainInt());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAgainInt());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAgainInt());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAgainInt());

    EXIT_ANALYZER_FUNCTION();
}


XCamReturn
RkAiqAgainHandleInt::updateConfig(bool needSync)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (needSync)
        mCfgMutex.lock();


    if (needSync)
        mCfgMutex.unlock();


    EXIT_ANALYZER_FUNCTION();
    return ret;
}


XCamReturn
RkAiqAgainHandleInt::prepare()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqAgainHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "again handle prepare failed");

    RkAiqAlgoConfigAgainInt* again_config_int = (RkAiqAlgoConfigAgainInt*)mConfig;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "again algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqAgainHandleInt::preProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPreAgainInt* again_pre_int = (RkAiqAlgoPreAgainInt*)mPreInParam;
    RkAiqAlgoPreResAgainInt* again_pre_res_int = (RkAiqAlgoPreResAgainInt*)mPreOutParam;

    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqPreResComb* comb = &shared->preResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;


    ret = RkAiqAgainHandle::preProcess();
    if (ret) {
        comb->again_pre_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "again handle preProcess failed");
    }

    comb->again_pre_res = NULL;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->pre_process(mPreInParam, mPreOutParam);
    RKAIQCORE_CHECK_RET(ret, "again algo pre_process failed");
    // set result to mAiqCore
    comb->again_pre_res = (RkAiqAlgoPreResAgain*)again_pre_res_int;

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqAgainHandleInt::processing()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAgainInt* again_proc_int = (RkAiqAlgoProcAgainInt*)mProcInParam;
    RkAiqAlgoProcResAgainInt* again_proc_res_int = (RkAiqAlgoProcResAgainInt*)mProcOutParam;

    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqProcResComb* comb = &shared->procResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    static int auvnr_proc_framecnt = 0;
    auvnr_proc_framecnt++;

    ret = RkAiqAgainHandle::processing();
    if (ret) {
        comb->again_proc_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "again handle processing failed");
    }

    comb->again_proc_res = NULL;

    // TODO: fill procParam
    again_proc_int->iso = sharedCom->iso;
    again_proc_int->hdr_mode = sharedCom->working_mode;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->processing(mProcInParam, mProcOutParam);
    RKAIQCORE_CHECK_RET(ret, "again algo processing failed");

    comb->again_proc_res = (RkAiqAlgoProcResAgain*)again_proc_res_int;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAgainHandleInt::postProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPostAgainInt* again_post_int = (RkAiqAlgoPostAgainInt*)mPostInParam;
    RkAiqAlgoPostResAgainInt* again_post_res_int = (RkAiqAlgoPostResAgainInt*)mPostOutParam;

    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqPostResComb* comb = &shared->postResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAgainHandle::postProcess();
    if (ret) {
        comb->again_post_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "auvnr handle postProcess failed");
        return ret;
    }

    comb->again_post_res = NULL;
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->post_process(mPostInParam, mPostOutParam);
    RKAIQCORE_CHECK_RET(ret, "auvnr algo post_process failed");
    // set result to mAiqCore
    comb->again_post_res = (RkAiqAlgoPostResAgain*)again_post_res_int ;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}


}; //namespace RkCam
