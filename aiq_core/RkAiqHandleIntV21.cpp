/*
 * RkAiqHandleIntV21.cpp
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

#include "RkAiqHandleIntV21.h"
#include "RkAiqCore.h"

namespace RkCam {

XCamReturn
RkAiqAdrcV1HandleInt::updateConfig(bool needSync)
{
    ENTER_ANALYZER_FUNCTION();

    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqAdrcV1HandleInt::prepare()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqAdrcHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "adrcV1 handle prepare failed");

    RkAiqAlgoConfigAdrcV1Int* adrcV1_config_int = (RkAiqAlgoConfigAdrcV1Int*)mConfig;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "adrcV1 algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

void
RkAiqAdrcV1HandleInt::init()
{
    ENTER_ANALYZER_FUNCTION();

    RkAiqAdrcHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAdrcV1Int());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAdrcV1Int());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAdrcV1Int());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAdrcV1Int());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAdrcV1Int());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAdrcV1Int());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAdrcV1Int());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn
RkAiqAdrcV1HandleInt::preProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPreAdrcV1Int* adrcV1_pre_int = (RkAiqAlgoPreAdrcV1Int*)mPreInParam;
    RkAiqAlgoPreResAdrcV1Int* adrcV1_pre_res_int = (RkAiqAlgoPreResAdrcV1Int*)mPreOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqPreResComb* comb = &shared->preResComb;
    RkAiqIspStats* ispStats = shared->ispStats;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqAdrcHandle::preProcess();
    if (ret) {
        comb->adrc_pre_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "adrcV1 handle preProcess failed");
    }

    comb->adrc_pre_res = NULL;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->pre_process(mPreInParam, mPreOutParam);
    RKAIQCORE_CHECK_RET(ret, "adrcV1 algo pre_process failed");

    // set result to mAiqCore
    comb->adrc_pre_res = (RkAiqAlgoPreResAdrc*)adrcV1_pre_res_int;

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqAdrcV1HandleInt::processing()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAdrcV1Int* adrcV1_proc_int = (RkAiqAlgoProcAdrcV1Int*)mProcInParam;
    RkAiqAlgoProcResAdrcV1Int* adrcV1_proc_res_int = (RkAiqAlgoProcResAdrcV1Int*)mProcOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqProcResComb* comb = &shared->procResComb;
    RkAiqIspStats* ispStats = shared->ispStats;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqAdrcHandle::processing();
    if (ret) {
        comb->adrc_proc_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "adrcV1 handle processing failed");
    }

    comb->adrc_proc_res = NULL;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->processing(mProcInParam, mProcOutParam);
    RKAIQCORE_CHECK_RET(ret, "adrcV1 algo processing failed");

    comb->adrc_proc_res = (RkAiqAlgoProcResAdrc*)adrcV1_proc_res_int;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAdrcV1HandleInt::postProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPostAdrcV1Int* adrcV1_post_int = (RkAiqAlgoPostAdrcV1Int*)mPostInParam;
    RkAiqAlgoPostResAdrcV1Int* adrcV1_post_res_int = (RkAiqAlgoPostResAdrcV1Int*)mPostOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqPostResComb* comb = &shared->postResComb;
    RkAiqIspStats* ispStats = shared->ispStats;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqAdrcHandle::postProcess();
    if (ret) {
        comb->adrc_post_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "adrcV1 handle postProcess failed");
        return ret;
    }

    comb->adrc_post_res = NULL;
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->post_process(mPostInParam, mPostOutParam);
    RKAIQCORE_CHECK_RET(ret, "adrcV1 algo post_process failed");
    // set result to mAiqCore
    comb->adrc_post_res = (RkAiqAlgoPostResAdrc*)adrcV1_post_res_int ;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}


void
RkAiqAsharpV3HandleInt::init()
{
    ENTER_ANALYZER_FUNCTION();

    RkAiqAsharpHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAsharpV3Int());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAsharpV3Int());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAsharpV3Int());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAsharpV3Int());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAsharpV3Int());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAsharpV3Int());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAsharpV3Int());

    EXIT_ANALYZER_FUNCTION();
}


XCamReturn
RkAiqAsharpV3HandleInt::updateConfig(bool needSync)
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
        rk_aiq_uapi_asharpV3_SetAttrib(mAlgoCtx, &mCurAtt, false);
        sendSignal();
    }

    if(updateIQpara) {
        mCurIQPara = mNewIQPara;
        updateIQpara = false;
        // TODO
        //rk_aiq_uapi_asharp_SetIQpara_V3(mAlgoCtx, &mCurIQPara, false);
        sendSignal();
    }

    if(updateStrength) {
        mCurStrength = mNewStrength;
        updateStrength = false;
        rk_aiq_uapi_asharpV3_SetStrength(mAlgoCtx, mCurStrength);
        sendSignal();
    }

    if (needSync)
        mCfgMutex.unlock();


    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAsharpV3HandleInt::setAttrib(rk_aiq_sharp_attrib_v3_t *att)
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
    if (0 != memcmp(&mCurAtt, att, sizeof(rk_aiq_sharp_attrib_v3_t))) {
        mNewAtt = *att;
        updateAtt = true;
        waitSignal();
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAsharpV3HandleInt::getAttrib(rk_aiq_sharp_attrib_v3_t *att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_asharpV3_GetAttrib(mAlgoCtx, att);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAsharpV3HandleInt::setIQPara(rk_aiq_sharp_IQPara_V3_t *para)
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
    if (0 != memcmp(&mCurIQPara, para, sizeof(rk_aiq_sharp_IQPara_V3_t))) {
        mNewIQPara = *para;
        updateIQpara = true;
        waitSignal();
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAsharpV3HandleInt::getIQPara(rk_aiq_sharp_IQPara_V3_t *para)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    // rk_aiq_uapi_asharp_GetIQpara(mAlgoCtx, para);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}


XCamReturn
RkAiqAsharpV3HandleInt::setStrength(float fPercent)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();


    mNewStrength = fPercent;
    updateStrength = true;
    waitSignal();


    mCfgMutex.unlock();
    EXIT_ANALYZER_FUNCTION();
    return ret;
}


XCamReturn
RkAiqAsharpV3HandleInt::getStrength(float *pPercent)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_asharpV3_GetStrength(mAlgoCtx, pPercent);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}


XCamReturn
RkAiqAsharpV3HandleInt::prepare()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqAsharpHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "asharp handle prepare failed");

    RkAiqAlgoConfigAsharpV3Int* asharp_config_int = (RkAiqAlgoConfigAsharpV3Int*)mConfig;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "asharp algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqAsharpV3HandleInt::preProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPreAsharpV3Int* asharp_pre_int = (RkAiqAlgoPreAsharpV3Int*)mPreInParam;
    RkAiqAlgoPreResAsharpV3Int* asharp_pre_res_int = (RkAiqAlgoPreResAsharpV3Int*)mPreOutParam;
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
RkAiqAsharpV3HandleInt::processing()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAsharpV3Int* asharp_proc_int = (RkAiqAlgoProcAsharpV3Int*)mProcInParam;
    RkAiqAlgoProcResAsharpV3Int* asharp_proc_res_int = (RkAiqAlgoProcResAsharpV3Int*)mProcOutParam;
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
RkAiqAsharpV3HandleInt::postProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPostAsharpV3Int* asharp_post_int = (RkAiqAlgoPostAsharpV3Int*)mPostInParam;
    RkAiqAlgoPostResAsharpV3Int* asharp_post_res_int = (RkAiqAlgoPostResAsharpV3Int*)mPostOutParam;
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
RkAiqAynrV2HandleInt::init()
{
    ENTER_ANALYZER_FUNCTION();

    RkAiqAynrHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAynrV2Int());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAynrV2Int());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAynrV2Int());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAynrV2Int());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAynrV2Int());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAynrV2Int());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAynrV2Int());

    EXIT_ANALYZER_FUNCTION();
}


XCamReturn
RkAiqAynrV2HandleInt::updateConfig(bool needSync)
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
        rk_aiq_uapi_aynrV2_SetAttrib(mAlgoCtx, &mCurAtt, false);
        sendSignal();
    }

    if(updateIQpara) {
        mCurIQPara = mNewIQPara;
        updateIQpara = false;
        // TODO
        //rk_aiq_uapi_asharp_SetIQpara_V3(mAlgoCtx, &mCurIQPara, false);
        sendSignal();
    }

    if(updateStrength) {
        mCurStrength = mNewStrength;
        updateStrength = false;
        rk_aiq_uapi_aynrV2_SetLumaSFStrength(mAlgoCtx, mCurStrength);
        sendSignal();
    }

    if (needSync)
        mCfgMutex.unlock();


    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAynrV2HandleInt::setAttrib(rk_aiq_ynr_attrib_v2_t *att)
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
    if (0 != memcmp(&mCurAtt, att, sizeof(rk_aiq_ynr_attrib_v2_t))) {
        mNewAtt = *att;
        updateAtt = true;
        waitSignal();
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAynrV2HandleInt::getAttrib(rk_aiq_ynr_attrib_v2_t *att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_aynrV2_GetAttrib(mAlgoCtx, att);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAynrV2HandleInt::setIQPara(rk_aiq_ynr_IQPara_V2_t *para)
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
    if (0 != memcmp(&mCurIQPara, para, sizeof(rk_aiq_ynr_IQPara_V2_t))) {
        mNewIQPara = *para;
        updateIQpara = true;
        waitSignal();
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAynrV2HandleInt::getIQPara(rk_aiq_ynr_IQPara_V2_t *para)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    // rk_aiq_uapi_asharp_GetIQpara(mAlgoCtx, para);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}


XCamReturn
RkAiqAynrV2HandleInt::setStrength(float fPercent)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

    mNewStrength = fPercent;
    updateStrength = true;
    waitSignal();

    mCfgMutex.unlock();
    EXIT_ANALYZER_FUNCTION();
    return ret;
}


XCamReturn
RkAiqAynrV2HandleInt::getStrength(float *pPercent)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_aynrV2_GetLumaSFStrength(mAlgoCtx, pPercent);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}


XCamReturn
RkAiqAynrV2HandleInt::prepare()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqAynrHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "aynr handle prepare failed");

    RkAiqAlgoConfigAynrV2Int* aynr_config_int = (RkAiqAlgoConfigAynrV2Int*)mConfig;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "aynr algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqAynrV2HandleInt::preProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPreAynrV2Int* aynr_pre_int = (RkAiqAlgoPreAynrV2Int*)mPreInParam;
    RkAiqAlgoPreResAynrV2Int* aynr_pre_res_int = (RkAiqAlgoPreResAynrV2Int*)mPreOutParam;
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
RkAiqAynrV2HandleInt::processing()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAynrV2Int* aynr_proc_int = (RkAiqAlgoProcAynrV2Int*)mProcInParam;
    RkAiqAlgoProcResAynrV2Int* aynr_proc_res_int = (RkAiqAlgoProcResAynrV2Int*)mProcOutParam;
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
RkAiqAynrV2HandleInt::postProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPostAynrV2Int* aynr_post_int = (RkAiqAlgoPostAynrV2Int*)mPostInParam;
    RkAiqAlgoPostResAynrV2Int* aynr_post_res_int = (RkAiqAlgoPostResAynrV2Int*)mPostOutParam;
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


void
RkAiqAcnrV1HandleInt::init()
{
    ENTER_ANALYZER_FUNCTION();

    RkAiqAcnrHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAcnrV1Int());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAcnrV1Int());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAcnrV1Int());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAcnrV1Int());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAcnrV1Int());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAcnrV1Int());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAcnrV1Int());

    EXIT_ANALYZER_FUNCTION();
}


XCamReturn
RkAiqAcnrV1HandleInt::updateConfig(bool needSync)
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
        rk_aiq_uapi_acnrV1_SetAttrib(mAlgoCtx, &mCurAtt, false);
        sendSignal();
    }

    if(updateIQpara) {
        mCurIQPara = mNewIQPara;
        updateIQpara = false;
        // TODO
        //rk_aiq_uapi_asharp_SetIQpara_V3(mAlgoCtx, &mCurIQPara, false);
        sendSignal();
    }

    if(updateStrength) {
        mCurStrength = mNewStrength;
        updateStrength = false;
        rk_aiq_uapi_acnrV1_SetChromaSFStrength(mAlgoCtx, mCurStrength);
        sendSignal();
    }

    if (needSync)
        mCfgMutex.unlock();


    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAcnrV1HandleInt::setAttrib(rk_aiq_cnr_attrib_v1_t *att)
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
    if (0 != memcmp(&mCurAtt, att, sizeof(rk_aiq_cnr_attrib_v1_t))) {
        mNewAtt = *att;
        updateAtt = true;
        waitSignal();
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAcnrV1HandleInt::getAttrib(rk_aiq_cnr_attrib_v1_t *att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_acnrV1_GetAttrib(mAlgoCtx, att);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAcnrV1HandleInt::setIQPara(rk_aiq_cnr_IQPara_V1_t *para)
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
    if (0 != memcmp(&mCurIQPara, para, sizeof(rk_aiq_cnr_IQPara_V1_t))) {
        mNewIQPara = *para;
        updateIQpara = true;
        waitSignal();
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAcnrV1HandleInt::getIQPara(rk_aiq_cnr_IQPara_V1_t *para)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    // rk_aiq_uapi_asharp_GetIQpara(mAlgoCtx, para);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}


XCamReturn
RkAiqAcnrV1HandleInt::setStrength(float fPercent)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();


    mNewStrength = fPercent;
    updateStrength = true;
    waitSignal();


    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();

    return ret;

}


XCamReturn
RkAiqAcnrV1HandleInt::getStrength(float *pPercent)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_acnrV1_GetChromaSFStrength(mAlgoCtx, pPercent);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}


XCamReturn
RkAiqAcnrV1HandleInt::prepare()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqAcnrHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "acnr handle prepare failed");

    RkAiqAlgoConfigAcnrV1Int* acnr_config_int = (RkAiqAlgoConfigAcnrV1Int*)mConfig;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "acnr algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqAcnrV1HandleInt::preProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPreAcnrV1Int* acnr_pre_int = (RkAiqAlgoPreAcnrV1Int*)mPreInParam;
    RkAiqAlgoPreResAcnrV1Int* acnr_pre_res_int = (RkAiqAlgoPreResAcnrV1Int*)mPreOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqPreResComb* comb = &shared->preResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAcnrHandle::preProcess();
    if (ret) {
        comb->acnr_pre_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "acnr handle preProcess failed");
    }

    comb->acnr_pre_res = NULL;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->pre_process(mPreInParam, mPreOutParam);
    RKAIQCORE_CHECK_RET(ret, "acnr algo pre_process failed");
    // set result to mAiqCore
    comb->acnr_pre_res = (RkAiqAlgoPreResAcnr*)acnr_pre_res_int;

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqAcnrV1HandleInt::processing()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAcnrV1Int* acnr_proc_int = (RkAiqAlgoProcAcnrV1Int*)mProcInParam;
    RkAiqAlgoProcResAcnrV1Int* acnr_proc_res_int = (RkAiqAlgoProcResAcnrV1Int*)mProcOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqProcResComb* comb = &shared->procResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;
    static int acnr_proc_framecnt = 0;
    acnr_proc_framecnt++;

    ret = RkAiqAcnrHandle::processing();
    if (ret) {
        comb->acnr_proc_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "acnr handle processing failed");
    }

    comb->acnr_proc_res = NULL;

    // TODO: fill procParam
    acnr_proc_int->iso = sharedCom->iso;
    acnr_proc_int->hdr_mode = sharedCom->working_mode;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->processing(mProcInParam, mProcOutParam);
    RKAIQCORE_CHECK_RET(ret, "aynr algo processing failed");

    comb->acnr_proc_res = (RkAiqAlgoProcResAcnr*)acnr_proc_res_int;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAcnrV1HandleInt::postProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPostAcnrV1Int* acnr_post_int = (RkAiqAlgoPostAcnrV1Int*)mPostInParam;
    RkAiqAlgoPostResAcnrV1Int* acnr_post_res_int = (RkAiqAlgoPostResAcnrV1Int*)mPostOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqPostResComb* comb = &shared->postResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqAcnrHandle::postProcess();
    if (ret) {
        comb->aynr_post_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "acnr handle postProcess failed");
        return ret;
    }

    comb->acnr_post_res = NULL;
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->post_process(mPostInParam, mPostOutParam);
    RKAIQCORE_CHECK_RET(ret, "acnr algo post_process failed");
    // set result to mAiqCore
    comb->acnr_post_res = (RkAiqAlgoPostResAcnr*)acnr_post_res_int ;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}


void
RkAiqArawnrV2HandleInt::init()
{
    ENTER_ANALYZER_FUNCTION();

    RkAiqArawnrHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigArawnrV2Int());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreArawnrV2Int());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResArawnrV2Int());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcArawnrV2Int());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResArawnrV2Int());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostArawnrV2Int());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResArawnrV2Int());

    EXIT_ANALYZER_FUNCTION();
}


XCamReturn
RkAiqArawnrV2HandleInt::updateConfig(bool needSync)
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
        rk_aiq_uapi_arawnrV2_SetAttrib(mAlgoCtx, &mCurAtt, false);
        sendSignal();
    }

    if(updateIQpara) {
        mCurIQPara = mNewIQPara;
        updateIQpara = false;
        // TODO
        //rk_aiq_uapi_asharp_SetIQpara_V3(mAlgoCtx, &mCurIQPara, false);
        sendSignal();
    }

    if(update2DStrength) {
        mCur2DStrength = mNew2DStrength;
        update2DStrength = false;
        rk_aiq_uapi_rawnrV2_SetSFStrength(mAlgoCtx, mCur2DStrength);
        sendSignal();
    }

    if(update3DStrength) {
        mCur3DStrength = mNew3DStrength;
        update3DStrength = false;
        rk_aiq_uapi_rawnrV2_SetTFStrength(mAlgoCtx, mCur3DStrength);
        sendSignal();
    }

    if (needSync)
        mCfgMutex.unlock();


    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqArawnrV2HandleInt::setAttrib(rk_aiq_bayernr_attrib_v2_t *att)
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
    if (0 != memcmp(&mCurAtt, att, sizeof(rk_aiq_bayernr_attrib_v2_t))) {
        mNewAtt = *att;
        updateAtt = true;
        waitSignal();
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqArawnrV2HandleInt::getAttrib(rk_aiq_bayernr_attrib_v2_t *att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_arawnrV2_GetAttrib(mAlgoCtx, att);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqArawnrV2HandleInt::setIQPara(rk_aiq_bayernr_IQPara_V2_t *para)
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
    if (0 != memcmp(&mCurIQPara, para, sizeof(rk_aiq_bayernr_IQPara_V2_t))) {
        mNewIQPara = *para;
        updateIQpara = true;
        waitSignal();
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqArawnrV2HandleInt::getIQPara(rk_aiq_bayernr_IQPara_V2_t *para)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    // rk_aiq_uapi_asharp_GetIQpara(mAlgoCtx, para);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}


XCamReturn
RkAiqArawnrV2HandleInt::setSFStrength(float fPercent)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();


    mNew2DStrength = fPercent;
    update2DStrength = true;
    waitSignal();


    mCfgMutex.unlock();
    EXIT_ANALYZER_FUNCTION();
    return ret;
}


XCamReturn
RkAiqArawnrV2HandleInt::getSFStrength(float *pPercent)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_rawnrV2_GetSFStrength(mAlgoCtx, pPercent);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqArawnrV2HandleInt::setTFStrength(float fPercent)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();


    mNew3DStrength = fPercent;
    update3DStrength = true;
    waitSignal();


    mCfgMutex.unlock();
    EXIT_ANALYZER_FUNCTION();
    return ret;
}


XCamReturn
RkAiqArawnrV2HandleInt::getTFStrength(float *pPercent)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_rawnrV2_GetTFStrength(mAlgoCtx, pPercent);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}


XCamReturn
RkAiqArawnrV2HandleInt::prepare()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqArawnrHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "arawnr handle prepare failed");

    RkAiqAlgoConfigArawnrV2Int* aynr_config_int = (RkAiqAlgoConfigArawnrV2Int*)mConfig;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "arawnr algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqArawnrV2HandleInt::preProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPreArawnrV2Int* arawnr_pre_int = (RkAiqAlgoPreArawnrV2Int*)mPreInParam;
    RkAiqAlgoPreResArawnrV2Int* arawnr_pre_res_int = (RkAiqAlgoPreResArawnrV2Int*)mPreOutParam;
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
RkAiqArawnrV2HandleInt::processing()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcArawnrV2Int* arawnr_proc_int = (RkAiqAlgoProcArawnrV2Int*)mProcInParam;
    RkAiqAlgoProcResArawnrV2Int* arawnr_proc_res_int = (RkAiqAlgoProcResArawnrV2Int*)mProcOutParam;
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
RkAiqArawnrV2HandleInt::postProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPostArawnrV2Int* arawnr_post_int = (RkAiqAlgoPostArawnrV2Int*)mPostInParam;
    RkAiqAlgoPostResArawnrV2Int* arawnr_post_res_int = (RkAiqAlgoPostResArawnrV2Int*)mPostOutParam;
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

XCamReturn
RkAiqAwbV21HandleInt::updateConfig(bool needSync)
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
    if (updateWbV21Attr) {
        mCurWbV21Attr = mNewWbV21Attr;
        updateWbV21Attr = false;
        rk_aiq_uapiV2_awbV21_SetAttrib(mAlgoCtx, mCurWbV21Attr, false);
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
RkAiqAwbV21HandleInt::setWbV21Attrib(rk_aiq_uapiV2_wbV21_attrib_t att)
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
    if (0 != memcmp(&mCurWbV21Attr, &att, sizeof(rk_aiq_uapiV2_wbV21_attrib_t))) {
        mNewWbV21Attr = att;
        updateWbV21Attr = true;
        waitSignal();
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
RkAiqAwbV21HandleInt::getWbV21Attrib(rk_aiq_uapiV2_wbV21_attrib_t *att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapiV2_awbV21_GetAttrib(mAlgoCtx, att);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}


}; //namespace RkCam
