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
#include "RkAiqAbayertnrV30Handle.h"

#include "RkAiqCore.h"

namespace RkCam {

DEFINE_HANDLE_REGISTER_TYPE(RkAiqAbayertnrV30HandleInt);

void RkAiqAbayertnrV30HandleInt::init() {
    ENTER_ANALYZER_FUNCTION();

    RkAiqHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAbayertnrV30());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAbayertnrV30());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAbayertnrV30());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqAbayertnrV30HandleInt::updateConfig(bool needSync) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#ifndef DISABLE_HANDLE_ATTRIB
    if (needSync) mCfgMutex.lock();
    // if something changed
    if (updateAtt) {
        mCurAtt   = mNewAtt;
        rk_aiq_uapi_abayertnrV30_SetAttrib(mAlgoCtx, &mCurAtt, false);
        sendSignal(mCurAtt.sync.sync_mode);
        updateAtt = false;
    }

    if (updateStrength) {
        mCurStrength   = mNewStrength;
        rk_aiq_uapi_abayertnrV30_SetStrength(mAlgoCtx, &mCurStrength);
        sendSignal(mCurStrength.sync.sync_mode);
        updateStrength = false;
    }

    if (needSync) mCfgMutex.unlock();
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAbayertnrV30HandleInt::setAttrib(const rk_aiq_bayertnr_attrib_v30_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    // TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

#ifdef DISABLE_HANDLE_ATTRIB
    ret = rk_aiq_uapi_abayertnrV30_SetAttrib(mAlgoCtx, att, false);
#else
    // if something changed
    if (0 != memcmp(&mCurAtt, att, sizeof(rk_aiq_bayertnr_attrib_v30_t))) {
        mNewAtt   = *att;
        updateAtt = true;
        waitSignal(att->sync.sync_mode);
    }
#endif

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAbayertnrV30HandleInt::getAttrib(rk_aiq_bayertnr_attrib_v30_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
    ret = rk_aiq_uapi_abayertnrV30_GetAttrib(mAlgoCtx, att);
    mCfgMutex.unlock();
#else
    if(att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_abayertnrV30_GetAttrib(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if(updateAtt) {
            memcpy(att, &mNewAtt, sizeof(mNewAtt));
            mCfgMutex.unlock();
            att->sync.done = false;
        } else {
            mCfgMutex.unlock();
            rk_aiq_uapi_abayertnrV30_GetAttrib(mAlgoCtx, att);
            att->sync.done = true;
        }
    }
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAbayertnrV30HandleInt::setStrength(const rk_aiq_bayertnr_strength_v30_t *pStrength) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
#ifdef DISABLE_HANDLE_ATTRIB
    ret = rk_aiq_uapi_abayertnrV30_SetStrength(mAlgoCtx, pStrength);
#else

    if (0 != memcmp(&mCurStrength, pStrength, sizeof(mCurStrength))) {
        mNewStrength   = *pStrength;
        updateStrength = true;
        waitSignal(pStrength->sync.sync_mode);
    }
#endif


    mCfgMutex.unlock();
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAbayertnrV30HandleInt::getStrength(rk_aiq_bayertnr_strength_v30_t *pStrength) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
    ret = rk_aiq_uapi_abayertnrV30_GetStrength(mAlgoCtx, pStrength);
    mCfgMutex.unlock();
#else
    if(pStrength->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_abayertnrV30_GetStrength(mAlgoCtx, pStrength);
        pStrength->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if(updateStrength) {
            pStrength->percent = mNewStrength.percent;
            pStrength->sync.done = false;
        } else {
            rk_aiq_uapi_abayertnrV30_GetStrength(mAlgoCtx, pStrength);
            pStrength->sync.done = true;
        }
    }
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAbayertnrV30HandleInt::getInfo(rk_aiq_bayertnr_info_v30_t *pInfo) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if(pInfo->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_abayertnrV30_GetInfo(mAlgoCtx, pInfo);
        pInfo->sync.done = true;
        mCfgMutex.unlock();
    } else {
        rk_aiq_uapi_abayertnrV30_GetInfo(mAlgoCtx, pInfo);
        pInfo->sync.done = true;
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}


XCamReturn RkAiqAbayertnrV30HandleInt::prepare() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "arawnr handle prepare failed");

    RkAiqAlgoConfigAbayertnrV30* abayertnr_config_int = (RkAiqAlgoConfigAbayertnrV30*)mConfig;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    abayertnr_config_int->stAbayertnrConfig.rawWidth  = sharedCom->snsDes.isp_acq_width;
    abayertnr_config_int->stAbayertnrConfig.rawHeight = sharedCom->snsDes.isp_acq_height;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "arawnr algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAbayertnrV30HandleInt::preProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if 0
    RkAiqAlgoPreAbayertnrV30* abayertnr_pre_int = (RkAiqAlgoPreAbayertnrV30*)mPreInParam;
    RkAiqAlgoPreResAbayertnrV30* abayertnr_pre_res_int =
        (RkAiqAlgoPreResAbayertnrV30*)mPreOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::preProcess();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "arawnr handle preProcess failed");
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->pre_process(mPreInParam, mPreOutParam);
    RKAIQCORE_CHECK_RET(ret, "arawnr algo pre_process failed");
    // set result to mAiqCore

    EXIT_ANALYZER_FUNCTION();
#endif
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAbayertnrV30HandleInt::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAbayertnrV30* abayertnr_proc_int = (RkAiqAlgoProcAbayertnrV30*)mProcInParam;
    RkAiqAlgoProcResAbayertnrV30* abayertnr_proc_res_int =
        (RkAiqAlgoProcResAbayertnrV30*)mProcOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    abayertnr_proc_res_int->stAbayertnrProcResult.st3DFix = &shared->fullParams->mTnrParams->data()->result;
    ret = RkAiqHandle::processing();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "aynr handle processing failed");
    }

    // TODO: fill procParam
    abayertnr_proc_int->iso      = sharedCom->iso;
    abayertnr_proc_int->hdr_mode = sharedCom->working_mode;

    {
        SmartLock locker(shared->bay3dStatListMutex);
        if (!shared->bay3dStatList.empty()) {
            RkAiqBay3dStat* pStats = NULL;
            auto iter              = shared->bay3dStatList.begin();
            if (shared->frameId >= 2) {
                uint32_t search_id = shared->frameId - 2;
                for (; iter != shared->bay3dStatList.end(); iter++) {
                    if (iter->ptr()->frame_id == search_id) break;
                }

                if (iter != shared->bay3dStatList.end()) {
                    pStats = iter->ptr();
                } else {
                    pStats = shared->bay3dStatList.back().ptr();
                    LOGW_ANR(
                        "cam%d, don't find the stats of id %d, using the last element of linked "
                        "list",
                        sharedCom->mCamPhyId, search_id);
                }
            } else {
                pStats = shared->bay3dStatList.back().ptr();
            }

            abayertnr_proc_int->stTnr_stats.isValid  = true;
            abayertnr_proc_int->stTnr_stats.frame_id = pStats->frame_id;
            abayertnr_proc_int->stTnr_stats.tnr_auto_sigma_count =
                pStats->stat.tnr_auto_sigma_count;
            for (int i = 0; i < ISP39_BAY3D_TNRSIG_NUM; i++) {
                abayertnr_proc_int->stTnr_stats.tnr_auto_sigma_calc[i] =
                    pStats->stat.tnr_auto_sigma_calc[i];
            }

            if (iter != shared->bay3dStatList.end()) shared->bay3dStatList.erase(iter);

            Abayertnr_Stats_V30_t* pTnrStats = &abayertnr_proc_int->stTnr_stats;
            LOGV_ANALYZER(
                "camId: %d, tnr stats: frame_id: %d, sigma count: %d, "
                "sigma_calc[0-3]: %d, %d, %d, %d, "
                "sigma_calc[16-20]: %d, %d, %d, %d",
                sharedCom->mCamPhyId, pTnrStats->frame_id, pTnrStats->tnr_auto_sigma_count,
                pTnrStats->tnr_auto_sigma_calc[0], pTnrStats->tnr_auto_sigma_calc[1],
                pTnrStats->tnr_auto_sigma_calc[2], pTnrStats->tnr_auto_sigma_calc[3],
                pTnrStats->tnr_auto_sigma_calc[16], pTnrStats->tnr_auto_sigma_calc[17],
                pTnrStats->tnr_auto_sigma_calc[18], pTnrStats->tnr_auto_sigma_calc[19]);
        } else {
            abayertnr_proc_int->stTnr_stats.isValid = false;
        }
    }

#ifdef RKAIQ_ENABLE_CAMGROUP
    if (mAiqCore->GetCamGroupManager()) {
        SmartPtr<RkAiqAlgoAtnrV30Stats> stats = new RkAiqAlgoAtnrV30Stats;
        if (!stats.ptr()) {
            LOGE_ANALYZER("new tnr stats failed!");
        } else {
            memcpy(&stats->result, &abayertnr_proc_int->stTnr_stats,
                   sizeof(abayertnr_proc_int->stTnr_stats));

            RkAiqCoreVdBufMsg msg(XCAM_MESSAGE_TNR_STATS_OK, shared->frameId, stats);
            mAiqCore->post_message(msg);
        }
    }
#endif
#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
#endif
    abayertnr_proc_int->stAblcV32_proc_res = shared->res_comb.ablcV32_proc_res;
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->processing(mProcInParam, mProcOutParam);
#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.unlock();
#endif
    RKAIQCORE_CHECK_RET(ret, "aynr algo processing failed");


    if (!abayertnr_proc_res_int->res_com.cfg_update) {
        shared->res_comb.bayernr3d_en = mLatestEn;
    } else {
        shared->res_comb.bayernr3d_en = mLatestEn = abayertnr_proc_res_int->stAbayertnrProcResult.st3DFix->bypass_en ? false : true;
    }
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAbayertnrV30HandleInt::postProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if 0
    RkAiqAlgoPostAbayer2dnrV30* abayertnr_post_int = (RkAiqAlgoPostAbayer2dnrV30*)mPostInParam;
    RkAiqAlgoPostResAbayer2dnrV30* abayertnr_post_res_int =
        (RkAiqAlgoPostResAbayer2dnrV30*)mPostOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::postProcess();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "arawnr handle postProcess failed");
        return ret;
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->post_process(mPostInParam, mPostOutParam);
    RKAIQCORE_CHECK_RET(ret, "arawnr algo post_process failed");
    // set result to mAiqCore

    EXIT_ANALYZER_FUNCTION();
#endif
    return ret;
}

XCamReturn RkAiqAbayertnrV30HandleInt::genIspResult(RkAiqFullParams* params,
        RkAiqFullParams* cur_params) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret                = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqAlgoProcResAbayertnrV30* atnr_rk = (RkAiqAlgoProcResAbayertnrV30*)mProcOutParam;

    if (!atnr_rk) {
        LOGD_ANALYZER("no abayertnr result");
        return XCAM_RETURN_NO_ERROR;
    }

    if (!this->getAlgoId()) {
        LOGD_ANR("oyyf: %s:%d output isp param start\n", __FUNCTION__, __LINE__);
        rk_aiq_isp_tnr_params_t* tnr_param = params->mTnrParams->data().ptr();
        if (sharedCom->init) {
            tnr_param->frame_id = 0;
        } else {
            tnr_param->frame_id = shared->frameId;
        }
        if (atnr_rk->res_com.cfg_update) {
            mSyncFlag = shared->frameId;
            tnr_param->sync_flag = mSyncFlag;
            // copy from algo result
            // set as the latest result
            cur_params->mTnrParams = params->mTnrParams;
            tnr_param->is_update = true;
            LOGD_ANR("3d [%d] params from algo", mSyncFlag);
        } else if (mSyncFlag != tnr_param->sync_flag) {
            tnr_param->sync_flag = mSyncFlag;
            // copy from latest result
            if (cur_params->mTnrParams.ptr()) {
                tnr_param->result = cur_params->mTnrParams->data()->result;
                tnr_param->is_update = true;
            } else {
                LOGE_ANR("no latest params !");
                tnr_param->is_update = false;
            }
            LOGD_ANR("3d [%d] params from latest [%d]", shared->frameId, mSyncFlag);
        } else {
            // do nothing, result in buf needn't update
            tnr_param->is_update = false;
            LOGD_ANR("3d [%d] params needn't update", shared->frameId);
        }
        LOGD_ANR("oyyf: %s:%d output isp param end \n", __FUNCTION__, __LINE__);
    }


    EXIT_ANALYZER_FUNCTION();

    return ret;
}

}  // namespace RkCam
