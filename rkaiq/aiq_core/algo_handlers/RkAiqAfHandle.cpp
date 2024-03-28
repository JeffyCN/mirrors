/*
 * Copyright (c) 2019-2022 Rockchip Eletronics Co., Ltd.
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
#include "RkAiqAfHandle.h"

#include <fcntl.h>

#include "RkAiqAeHandle.h"
#include "RkAiqCore.h"

namespace RkCam {

DEFINE_HANDLE_REGISTER_TYPE(RkAiqAfHandleInt);

void RkAiqAfHandleInt::init() {
    ENTER_ANALYZER_FUNCTION();

    RkAiqHandle::deInit();
    mConfig      = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAf());
    mProcInParam = (RkAiqAlgoCom*)(new RkAiqAlgoProcAf());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAf());

    mLastZoomIndex = 0;

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqAfHandleInt::setCalib(void* calib) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mCfgMutex.lock();
    ret = rk_aiq_uapi_af_SetCalib(mAlgoCtx, calib);
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAfHandleInt::getCalib(void* calib) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mCfgMutex.lock();
    ret = rk_aiq_uapi_af_GetCalib(mAlgoCtx, calib);
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAfHandleInt::updateConfig(bool needSync) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret                              = XCAM_RETURN_NO_ERROR;
#ifndef DISABLE_HANDLE_ATTRIB
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    {
        if (needSync) mCfgMutex.lock();
        // if something changed
        if (updateAtt) {
            rk_aiq_uapi_af_SetAttrib(mAlgoCtx, mNewAtt, false);
            isUpdateAttDone = true;
        }
        if (needSync) mCfgMutex.unlock();
    }
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAfHandleInt::setAttrib(rk_aiq_af_attrib_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret                              = XCAM_RETURN_NO_ERROR;
    // RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    mCfgMutex.lock();
#ifdef DISABLE_HANDLE_ATTRIB
    ret = rk_aiq_uapi_af_SetAttrib(mAlgoCtx, *att, false);
#else

    // check if there is different between att & mCurAtt(sync)/mNewAtt(async)
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore
    bool isChanged = false;
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC && \
        memcmp(&mNewAtt, att, sizeof(*att)))
        isChanged = true;
    else if (att->sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC && \
             memcmp(&mCurAtt, att, sizeof(*att)))
        isChanged = true;

    // if something changed
    if (isChanged/* || (mCurAtt.AfMode == RKAIQ_AF_MODE_AUTO && sharedCom->snsDes.lens_des.focus_support)*/) {
        mNewAtt         = *att;
        updateAtt       = true;
        isUpdateAttDone = false;
        LOGI_AF("%s: AfMode %d, win: %d, %d, %d, %d",
               __func__, att->AfMode, att->h_offs, att->v_offs, att->h_size, att->v_size);
        waitSignal(att->sync.sync_mode);
    }
#endif

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAfHandleInt::getAttrib(rk_aiq_af_attrib_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
    rk_aiq_uapi_af_GetAttrib(mAlgoCtx, att);
    mCfgMutex.unlock();
#else
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_af_GetAttrib(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateAtt) {
            memcpy(att, &mNewAtt, sizeof(mNewAtt));
            att->sync.done = false;
        } else {
            rk_aiq_uapi_af_GetAttrib(mAlgoCtx, att);
            att->sync.sync_mode = mNewAtt.sync.sync_mode;
            att->sync.done      = true;
        }
    }
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAfHandleInt::lock() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret                              = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    if (sharedCom->snsDes.lens_des.focus_support) {
        mCfgMutex.lock();
        rk_aiq_uapi_af_Lock(mAlgoCtx);
        mCfgMutex.unlock();
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAfHandleInt::unlock() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret                              = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    if (sharedCom->snsDes.lens_des.focus_support) {
        mCfgMutex.lock();
        rk_aiq_uapi_af_Unlock(mAlgoCtx);
        mCfgMutex.unlock();
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAfHandleInt::Oneshot() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret                              = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    if (sharedCom->snsDes.lens_des.focus_support) {
        mCfgMutex.lock();
        rk_aiq_uapi_af_Oneshot(mAlgoCtx);
        mCfgMutex.unlock();
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAfHandleInt::ManualTriger() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret                              = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    if (sharedCom->snsDes.lens_des.focus_support) {
        mCfgMutex.lock();
        rk_aiq_uapi_af_ManualTriger(mAlgoCtx);
        mCfgMutex.unlock();
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAfHandleInt::Tracking() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret                              = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    if (sharedCom->snsDes.lens_des.focus_support) {
        mCfgMutex.lock();
        rk_aiq_uapi_af_Tracking(mAlgoCtx);
        mCfgMutex.unlock();
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAfHandleInt::setZoomIndex(int index) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret                              = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    if (sharedCom->snsDes.lens_des.zoom_support) {
        mCfgMutex.lock();
        rk_aiq_uapi_af_setZoomIndex(mAlgoCtx, index);
        isUpdateZoomPosDone = true;
        waitSignal();
        mCfgMutex.unlock();
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAfHandleInt::getZoomIndex(int* index) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret                              = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    if (sharedCom->snsDes.lens_des.zoom_support) {
        rk_aiq_uapi_af_getZoomIndex(mAlgoCtx, index);
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAfHandleInt::endZoomChg() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret                              = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    if (sharedCom->snsDes.lens_des.zoom_support) {
        mCfgMutex.lock();
        rk_aiq_uapi_af_endZoomChg(mAlgoCtx);
        mCfgMutex.unlock();
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAfHandleInt::startZoomCalib() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret                              = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    if (sharedCom->snsDes.lens_des.zoom_support) {
        mCfgMutex.lock();
        rk_aiq_uapi_af_startZoomCalib(mAlgoCtx);
        isUpdateZoomPosDone = true;
        waitSignal();
        mCfgMutex.unlock();
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAfHandleInt::resetZoom() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret                              = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    if (sharedCom->snsDes.lens_des.zoom_support) {
        mCfgMutex.lock();
        rk_aiq_uapi_af_resetZoom(mAlgoCtx);
        isUpdateZoomPosDone = true;
        waitSignal();
        mCfgMutex.unlock();
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAfHandleInt::GetSearchPath(rk_aiq_af_sec_path_t* path) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret                              = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    if (sharedCom->snsDes.lens_des.focus_support) rk_aiq_uapi_af_getSearchPath(mAlgoCtx, path);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAfHandleInt::GetSearchResult(rk_aiq_af_result_t* result) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret                              = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    if (sharedCom->snsDes.lens_des.focus_support) rk_aiq_uapi_af_getSearchResult(mAlgoCtx, result);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAfHandleInt::GetFocusRange(rk_aiq_af_focusrange* range) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret                              = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    if (sharedCom->snsDes.lens_des.focus_support) rk_aiq_uapi_af_getFocusRange(mAlgoCtx, range);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAfHandleInt::GetZoomRange(rk_aiq_af_zoomrange* range) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret                              = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    if (sharedCom->snsDes.lens_des.zoom_support) rk_aiq_uapi_af_getZoomRange(mAlgoCtx, range);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAfHandleInt::setAeStable(bool ae_stable) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mAeStableMutex.lock();
    mAeStable = ae_stable;
    mAeStableMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAfHandleInt::prepare() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "af handle prepare failed");

    RkAiqAlgoConfigAf* af_config_int         = (RkAiqAlgoConfigAf*)mConfig;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    af_config_int->af_mode    = 6;
    af_config_int->win_h_offs = 0;
    af_config_int->win_v_offs = 0;
    af_config_int->win_h_size = 0;
    af_config_int->win_v_size = 0;
    af_config_int->lens_des   = sharedCom->snsDes.lens_des;

    // for otp
    af_config_int->otp_af = sharedCom->snsDes.otp_af;
    af_config_int->otp_pdaf = sharedCom->snsDes.otp_pdaf;

    if ((af_config_int->com.u.prepare.sns_op_width != 0) &&
        (af_config_int->com.u.prepare.sns_op_height != 0)) {
        RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
        ret                       = des->prepare(mConfig);
        RKAIQCORE_CHECK_RET(ret, "af algo prepare failed");
    } else {
        LOGI_AF("%s: input sns_op_width %d or sns_op_height %d is zero, bypass!",
            __func__,
            af_config_int->com.u.prepare.sns_op_width,
            af_config_int->com.u.prepare.sns_op_height);
    }

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAfHandleInt::preProcess() {
    return XCAM_RETURN_NO_ERROR;
}

bool RkAiqAfHandleInt::getValueFromFile(const char* path, int* pos) {
    const char* delim = " ";
    char buffer[16]   = {0};
    int fp;

    fp = open(path, O_RDONLY | O_SYNC);
    if (fp != -1) {
        if (read(fp, buffer, sizeof(buffer)) <= 0) {
            LOGE_AF("%s read %s failed!", __func__, path);
            goto OUT;
        } else {
            char* p = nullptr;

            p = strtok(buffer, delim);
            if (p != nullptr) {
                *pos = atoi(p);
            }
        }
        close(fp);
        return true;
    }

OUT:
    return false;
}

XCamReturn RkAiqAfHandleInt::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoProcAf* af_proc_int = (RkAiqAlgoProcAf*)mProcInParam;

    RkAiqAlgoProcResAf* af_proc_res_int = (RkAiqAlgoProcResAf*)mProcOutParam;

    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

#if RKAIQ_HAVE_AF_V33 || RKAIQ_ONLY_AF_STATS_V33
#ifdef USE_NEWSTRUCT
    af_proc_res_int->afStats_cfg = &shared->fullParams->mAfParams->data()->result;
#else
    af_proc_res_int->af_isp_param_v33 = &shared->fullParams->mAfParams->data()->result;
#endif
#endif
#if RKAIQ_HAVE_AF_V32_LITE || RKAIQ_ONLY_AF_STATS_V32_LITE
    af_proc_res_int->af_isp_param_v32 = &shared->fullParams->mAfParams->data()->result;
#endif
#if RKAIQ_HAVE_AF_V31 || RKAIQ_ONLY_AF_STATS_V31
    af_proc_res_int->af_isp_param_v31 = &shared->fullParams->mAfParams->data()->result;
#endif
#if RKAIQ_HAVE_AF_V30 || RKAIQ_ONLY_AF_STATS_V30
    af_proc_res_int->af_isp_param_v3x = &shared->fullParams->mAfParams->data()->result;
#endif
#if RKAIQ_HAVE_AF_V20 || RKAIQ_ONLY_AF_STATS_V20
    af_proc_res_int->af_isp_param = &shared->fullParams->mAfParams->data()->result;
#endif

    af_proc_res_int->af_focus_param = &shared->fullParams->mFocusParams->data()->result;

//#define ZOOM_MOVE_DEBUG
#ifdef ZOOM_MOVE_DEBUG
    int zoom_index = 0;

    if (getValueFromFile("/data/.zoom_pos", &zoom_index) == true) {
        if (mLastZoomIndex != zoom_index) {
            setZoomIndex(zoom_index);
            endZoomChg();
            mLastZoomIndex = zoom_index;
        }
    }
#endif

    ret = RkAiqHandle::processing();
    if (ret < 0) {
        LOGE_ANALYZER("af handle processing failed ret %d", ret);
        return ret;
    } else if (ret == XCAM_RETURN_BYPASS) {
        LOGW_ANALYZER("%s:%d bypass !", __func__, __LINE__);
        return ret;
    }

    RkAiqAfStats* xAfStats = nullptr;
    if (shared->afStatsBuf) {
        xAfStats = shared->afStatsBuf;
        if (!xAfStats) LOGE("af stats is null");
    } else {
        LOGW("the xcamvideobuffer of af stats is null");
    }

#if RKAIQ_HAVE_AF
    if ((!xAfStats || !xAfStats->af_stats_valid) && !sharedCom->init) {
        LOGW("no af stats, ignore!");
        return XCAM_RETURN_BYPASS;
    }

#if RKAIQ_HAVE_AF_V20
    if (xAfStats) {
        af_proc_int->xcam_af_stats  = &xAfStats->af_stats;
        af_proc_int->stat_motor = &xAfStats->stat_motor;
        af_proc_int->aecExpInfo = &xAfStats->aecExpInfo;
    } else {
        af_proc_int->xcam_af_stats = NULL;
        af_proc_int->stat_motor = NULL;
        af_proc_int->aecExpInfo = NULL;
    }
#else
#ifdef USE_NEWSTRUCT
    if (xAfStats) {
        af_proc_int->afStats_stats = &xAfStats->afStats_stats;
        af_proc_int->stat_motor = &xAfStats->stat_motor;
        af_proc_int->aecExpInfo = &xAfStats->aecExpInfo;
    } else {
        af_proc_int->afStats_stats = NULL;
        af_proc_int->stat_motor = NULL;
        af_proc_int->aecExpInfo = NULL;
    }
#else
    if (xAfStats) {
        af_proc_int->xcam_af_stats_v3x = &xAfStats->af_stats_v3x;
        af_proc_int->stat_motor = &xAfStats->stat_motor;
        af_proc_int->aecExpInfo = &xAfStats->aecExpInfo;
    } else {
        af_proc_int->xcam_af_stats_v3x = NULL;
        af_proc_int->stat_motor = NULL;
        af_proc_int->aecExpInfo = NULL;
    }
#endif
#endif
    if (shared->aecStatsBuf)
        af_proc_int->xcam_aec_stats = &shared->aecStatsBuf->aec_stats;
    else
        af_proc_int->xcam_aec_stats = NULL;
    if (shared->pdafStatsBuf)
        af_proc_int->xcam_pdaf_stats = &shared->pdafStatsBuf->pdaf_stats;
    else
        af_proc_int->xcam_pdaf_stats = NULL;
    mAeStableMutex.lock();
    af_proc_int->ae_stable = mAeStable;
    mAeStableMutex.unlock();
#endif

#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
#endif
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->processing(mProcInParam, (RkAiqAlgoResCom*)af_proc_res_int);
#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.unlock();
#endif

    if (ret < 0) {
        LOGE_ANALYZER("af algo processing failed ret %d", ret);
        return ret;
    } else if (ret == XCAM_RETURN_BYPASS) {
        LOGW_ANALYZER("%s:%d bypass !", __func__, __LINE__);
        return ret;
    }

#ifndef DISABLE_HANDLE_ATTRIB
    if (updateAtt && isUpdateAttDone) {
        mCurAtt         = mNewAtt;
        updateAtt       = false;
        isUpdateAttDone = false;
        sendSignal(mCurAtt.sync.sync_mode);
    }
#endif

    if (isUpdateZoomPosDone) {
        isUpdateZoomPosDone = false;
        sendSignal();
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAfHandleInt::postProcess() {
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAfHandleInt::genIspResult(RkAiqFullParams* params, RkAiqFullParams* cur_params) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    RkAiqAlgoProcResAf* af_com                  = (RkAiqAlgoProcResAf*)mProcOutParam ;
    rk_aiq_isp_af_params_t* af_param = params->mAfParams->data().ptr();

    SmartPtr<rk_aiq_focus_params_wrapper_t> focus_param = params->mFocusParams->data();

    if (!this->getAlgoId()) {
        RkAiqAlgoProcResAf* af_rk = (RkAiqAlgoProcResAf*)af_com;

        if (sharedCom->init) {
            af_param->frame_id    = 0;
            focus_param->frame_id = 0;
        } else {
            af_param->frame_id    = shared->frameId;
            focus_param->frame_id = shared->frameId;
        }

        SmartPtr<RkAiqHandle>* ae_handle = mAiqCore->getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_AE);

        if (ae_handle) {
            int algo_id = (*ae_handle)->getAlgoId();
            if (algo_id == 0) {
                RkAiqAeHandleInt* ae_algo = dynamic_cast<RkAiqAeHandleInt*>(ae_handle->ptr());

                if (af_rk->lockae_en)
                    ae_algo->setLockAeForAf(af_rk->lockae);
                else
                    ae_algo->setLockAeForAf(false);
            }
        }
    }

    if (af_com->af_cfg_update) {
        mAfMeasResSyncFalg = shared->frameId;
        af_param->sync_flag = mAfMeasResSyncFalg;
        cur_params->mAfParams = params->mAfParams;
        af_param->is_update = true;
        af_com->af_cfg_update = false;
        LOGD_AF("[%d] meas params from algo", mAfMeasResSyncFalg);
    } else if (mAfMeasResSyncFalg != af_param->sync_flag) {
        af_param->sync_flag = mAfMeasResSyncFalg;
        if (cur_params->mAfParams.ptr()) {
            af_param->result = cur_params->mAfParams->data()->result;
            af_param->is_update = true;
        } else {
            LOGE_AF("no latest meas params !");
            af_param->is_update = false;
        }
        LOGD_AF("[%d] meas params from latest [%d]", shared->frameId, mAfMeasResSyncFalg);
    } else {
        // do nothing, result in buf needn't update
        af_param->is_update = false;
        LOGD_AF("[%d] meas params needn't update", shared->frameId);
    }

    if (af_com->af_focus_update) {
        mAfFocusResSyncFalg = shared->frameId;
        focus_param->sync_flag = mAfFocusResSyncFalg;
        cur_params->mFocusParams = params->mFocusParams;
        focus_param->is_update = true;
        af_com->af_focus_update = false;
        LOGD_AF("[%d] focus params from algo", mAfFocusResSyncFalg);
    } else if (mAfFocusResSyncFalg != focus_param->sync_flag) {
        focus_param->sync_flag = mAfFocusResSyncFalg;
        if (cur_params->mFocusParams.ptr()) {
            focus_param->result = cur_params->mFocusParams->data()->result;
            focus_param->is_update = true;
        } else {
            LOGE_AF("no focus latest params !");
            focus_param->is_update = false;
        }
        LOGD_AF("[%d] focus params from latest [%d]", shared->frameId, mAfFocusResSyncFalg);
    } else {
        // do nothing, result in buf needn't update
        focus_param->is_update = false;
        LOGD_AF("[%d] focus params needn't update", shared->frameId);
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

}  // namespace RkCam
