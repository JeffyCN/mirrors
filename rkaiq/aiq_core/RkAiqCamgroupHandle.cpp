/*
 * RkAiqCamgroupHandle.h
 *
 *  Copyright (c) 2019-2021 Rockchip Eletronics Co., Ltd.
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

#include "RkAiqCamgroupHandle.h"
#include "RkAiqCamGroupManager.h"
#include "RkAiqManager.h"
#include "aiq_core/RkAiqCore.h"

namespace RkCam {

RkAiqCamgroupHandle::RkAiqCamgroupHandle(RkAiqAlgoDesComm* des, RkAiqCamGroupManager* camGroupMg)
    : mDes(des), mGroupMg(camGroupMg), mEnable(true) {
    mDes->create_context(&mAlgoCtx, (const _AlgoCtxInstanceCfg*)(&mGroupMg->mGroupAlgoCtxCfg));
    mConfig       = NULL;
    mProcInParam  = NULL;
    mProcOutParam = NULL;
    updateAtt = false;
    mNextHdl = NULL;
    mParentHdl = NULL;
    mAiqCore = NULL;
}

RkAiqCamgroupHandle::~RkAiqCamgroupHandle() {
    if (mDes) mDes->destroy_context(mAlgoCtx);
    deInit();
}

void RkAiqCamgroupHandle::init() {
    ENTER_ANALYZER_FUNCTION();
    deInit();
    mConfig = (RkAiqAlgoCom*)(new RkAiqAlgoCamGroupPrepare());
    mProcInParam = (RkAiqAlgoCom*)(new RkAiqAlgoCamGroupProcIn());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoCamGroupProcOut());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqCamgroupHandle::prepare(RkAiqCore* aiqCore) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mAiqCore = aiqCore;

    if (mConfig == NULL) init();

    // assume all single cam runs same algos

    RkAiqAlgoCamGroupPrepare* prepareCfg = (RkAiqAlgoCamGroupPrepare*)mConfig ;

    RkAiqAlgoComCamGroup* gcom = &prepareCfg->gcom;
    RkAiqAlgoCom *com = &gcom->com;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    prepareCfg->camIdArrayLen = mGroupMg->mBindAiqsMap.size();

    LOGD_CAMGROUP("camgroup: prepare: relay init params ...");
    int i = 0;
    for (auto it : mGroupMg->mBindAiqsMap) {
        prepareCfg->camIdArray[i++] = it.first;
    }

    prepareCfg->aec.LinePeriodsPerField =
        (float)sharedCom->snsDes.frame_length_lines;
    prepareCfg->aec.PixelClockFreqMHZ =
        (float)sharedCom->snsDes.pixel_clock_freq_mhz;
    prepareCfg->aec.PixelPeriodsPerLine =
        (float)sharedCom->snsDes.line_length_pck;
    prepareCfg->s_calibv2 = mGroupMg->mGroupAlgoCtxCfg.s_calibv2;

    prepareCfg->pCamgroupCalib = mGroupMg->mCamgroupCalib;
    prepareCfg->aec.nr_switch = sharedCom->snsDes.nr_switch;

    LOGD_CAMGROUP("camgroup: prepare: prepare algos ...");

    com->ctx                     = mAlgoCtx;
    com->frame_id                = 0;
    com->u.prepare.working_mode  = sharedCom->working_mode;
    com->u.prepare.sns_op_width  = sharedCom->snsDes.isp_acq_width;
    com->u.prepare.sns_op_height = sharedCom->snsDes.isp_acq_height;
    com->u.prepare.compr_bit     = sharedCom->snsDes.compr_bit;
    com->u.prepare.conf_type     = sharedCom->conf_type;
    com->u.prepare.calibv2 =
        const_cast<CamCalibDbV2Context_t*>(prepareCfg->s_calibv2);
    if (mDes->type == RK_AIQ_ALGO_TYPE_AWB) {
        mCfgMutex.lock();
        ret = ((RkAiqAlgoDescription*)mDes)->prepare(com);
        mCfgMutex.unlock();
    }
    else {
        ret = ((RkAiqAlgoDescription*)mDes)->prepare(com);
    }

    if (ret) {
        LOGE("algo %d prepare failed !", mDes->type);
        return ret;
    }

    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqCamgroupHandle::processing(rk_aiq_singlecam_3a_result_t** params_res_array) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret          = XCAM_RETURN_NO_ERROR;

    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    RkAiqAlgoCamGroupProcIn* procIn =
        (RkAiqAlgoCamGroupProcIn*)mProcInParam;
    RkAiqAlgoCamGroupProcOut* procOut =
        (RkAiqAlgoCamGroupProcOut*)mProcOutParam;

    memset(procIn, 0, sizeof(RkAiqAlgoCamGroupProcIn));
    memset(procOut, 0, sizeof(RkAiqAlgoCamGroupProcOut));

    RkAiqAlgoComCamGroup* gcom = &procIn->gcom;
    RkAiqAlgoCom *com = &gcom->com;

    procIn->arraySize = mGroupMg->mBindAiqsMap.size();
    procOut->arraySize = mGroupMg->mBindAiqsMap.size();

    procIn->camgroupParmasArray = params_res_array;
    procOut->camgroupParmasArray = params_res_array;
    procIn->_gray_mode = sharedCom->gray_mode;
    procIn->working_mode = sharedCom->working_mode;
    procIn->_is_bw_sensor = sharedCom->is_bw_sensor;
    procIn->_offset_is_update =
         (char*)(&((rk_aiq_isp_params_t<int> *)0)->result) - (char*)(&((rk_aiq_isp_params_t<int>*)0)->is_update);
    procIn->attribUpdated = mSingleHdl->isUpdateGrpAttr();

    com->ctx         = mAlgoCtx;
    com->frame_id    = params_res_array[0]->_frameId;
    // TODO: remove init info ? algo can maintain the state itself
    com->u.proc.init = mGroupMg->mInit;
    ret = ((RkAiqAlgoDescription*)mDes)->processing((const RkAiqAlgoCom*)procIn,
            (RkAiqAlgoResCom*)procOut);
    if (ret) {
        LOGW_CAMGROUP("group algo %d proc error !", mDes->type);
        return ret;
    }

    mSingleHdl->clearUpdateGrpAttr();

    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

void RkAiqCamgroupHandle::deInit() {
    ENTER_ANALYZER_FUNCTION();

#define RKAIQ_DELLET(a) \
    if (a) {            \
        delete a;       \
        a = NULL;       \
    }

    RKAIQ_DELLET(mConfig);
    RKAIQ_DELLET(mProcInParam);
    RKAIQ_DELLET(mProcOutParam);

    EXIT_ANALYZER_FUNCTION();
}

void
RkAiqCamgroupHandle::waitSignal(rk_aiq_uapi_mode_sync_e sync_mode)
{
    if (mGroupMg->isRunningState()) {
        if (sync_mode == RK_AIQ_UAPI_MODE_ASYNC)
            return;

        mUpdateCond.timedwait(mCfgMutex, 100000);
    } else {
        updateConfig(false);
    }
}

void
RkAiqCamgroupHandle::sendSignal(rk_aiq_uapi_mode_sync_e sync_mode)
{
    if (sync_mode == RK_AIQ_UAPI_MODE_ASYNC)
        return;

    if (mGroupMg->isRunningState())
        mUpdateCond.signal();
}

}  // namespace RkCam
