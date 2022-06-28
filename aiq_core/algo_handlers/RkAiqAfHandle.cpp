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

#include "RkAiqCore.h"
#include "RkAiqAeHandle.h"

namespace RkCam {

DEFINE_HANDLE_REGISTER_TYPE(RkAiqAfHandleInt);

void RkAiqAfHandleInt::init() {
    ENTER_ANALYZER_FUNCTION();

    RkAiqHandle::deInit();
    mConfig      = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAf());
    mPreInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPreAf());
    mPreOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAf());
    mProcInParam = (RkAiqAlgoCom*)(new RkAiqAlgoProcAf());
#if 0
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAf());
#endif
    mPostInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPostAf());
    mPostOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAf());
    mLastZoomIndex = 0;

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqAfHandleInt::updateConfig(bool needSync) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret                              = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    if (sharedCom->snsDes.lens_des.focus_support) {
        if (needSync) mCfgMutex.lock();
        // if something changed
        if (updateAtt) {
            rk_aiq_uapi_af_SetAttrib(mAlgoCtx, mNewAtt, false);
            isUpdateAttDone = true;
        }
        if (needSync) mCfgMutex.unlock();
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAfHandleInt::setAttrib(rk_aiq_af_attrib_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret                              = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    if (sharedCom->snsDes.lens_des.focus_support) {
        mCfgMutex.lock();

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
        if (isChanged || (mCurAtt.AfMode == RKAIQ_AF_MODE_AUTO)) {
            mNewAtt         = *att;
            updateAtt       = true;
            isUpdateAttDone = false;
            waitSignal(att->sync.sync_mode);
        }

        mCfgMutex.unlock();
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAfHandleInt::getAttrib(rk_aiq_af_attrib_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

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

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAfHandleInt::lock() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret                              = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    if (sharedCom->snsDes.lens_des.focus_support) rk_aiq_uapi_af_Lock(mAlgoCtx);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAfHandleInt::unlock() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret                              = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    if (sharedCom->snsDes.lens_des.focus_support) rk_aiq_uapi_af_Unlock(mAlgoCtx);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAfHandleInt::Oneshot() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret                              = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    if (sharedCom->snsDes.lens_des.focus_support) rk_aiq_uapi_af_Oneshot(mAlgoCtx);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAfHandleInt::ManualTriger() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret                              = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    if (sharedCom->snsDes.lens_des.focus_support) rk_aiq_uapi_af_ManualTriger(mAlgoCtx);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAfHandleInt::Tracking() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret                              = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    if (sharedCom->snsDes.lens_des.focus_support) rk_aiq_uapi_af_Tracking(mAlgoCtx);

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

XCamReturn RkAiqAfHandleInt::GetCustomAfRes(rk_tool_customAf_res_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcResAf* af_com = &mProcResShared->result;
    RkAiqAlgoProcResAf* af_rk = (RkAiqAlgoProcResAf*)af_com;
    int i;

    memset(att, 0, sizeof(*att));
#if defined(ISP_HW_V30)
    rk_aiq_isp_af_meas_v3x_t* af_param = &af_rk->af_isp_param_v3x;

    att->af_en = af_param->af_en;
    att->rawaf_sel = af_param->rawaf_sel;
    att->gamma_en = af_param->gamma_en;
    att->gaus_en = af_param->gaus_en;
    att->v1_fir_sel = af_param->v1_fir_sel;
    att->hiir_en = af_param->hiir_en;
    att->viir_en = af_param->viir_en;
    att->v1_fv_outmode = af_param->v1_fv_outmode;
    att->v2_fv_outmode = af_param->v2_fv_outmode;
    att->h1_fv_outmode = af_param->h1_fv_outmode;
    att->h2_fv_outmode = af_param->h2_fv_outmode;
    att->ldg_en = af_param->ldg_en;
    att->accu_8bit_mode = af_param->accu_8bit_mode;
    att->ae_mode = af_param->ae_mode;
    att->y_mode = af_param->y_mode;

    att->window_num = af_param->window_num;
    att->wina_h_offs = af_param->wina_h_offs;
    att->wina_v_offs = af_param->wina_v_offs;
    att->wina_h_size = af_param->wina_h_size;
    att->wina_v_size = af_param->wina_v_size;
    att->winb_h_offs = af_param->winb_h_offs;
    att->winb_v_offs = af_param->winb_v_offs;
    att->winb_h_size = af_param->winb_h_size;
    att->winb_v_size = af_param->winb_v_size;

    for (i = 0; i < RKAIQ_RAWAF_GAMMA_NUM; i++)
        att->gamma_y[i] = af_param->gamma_y[i];

    att->thres = af_param->thres;
    att->shift_sum_a = af_param->shift_sum_a;
    att->shift_sum_b = af_param->shift_sum_b;
    att->shift_y_a = af_param->shift_y_a;
    att->shift_y_b = af_param->shift_y_b;

    for (i = 0; i < 9; i++)
        att->v1_iir_coe[i] = af_param->v1_iir_coe[i] << 2;
    for (i = 0; i < 3; i++) {
        att->v1_fir_coe[i] = af_param->v1_fir_coe[i];
        att->v2_iir_coe[i] = af_param->v2_iir_coe[i];
        att->v2_fir_coe[i] = af_param->v2_fir_coe[i];
    }

    for (i = 0; i < 6; i++) {
        att->h1_iir1_coe[i] = af_param->h1_iir1_coe[i];
        att->h2_iir1_coe[i] = af_param->h2_iir1_coe[i];
        att->h1_iir2_coe[i] = af_param->h1_iir2_coe[i];
        att->h2_iir2_coe[i] = af_param->h2_iir2_coe[i];
    }

    for (i = 0; i < 2; i++) {
        att->h_ldg_lumth[i] = af_param->h_ldg_lumth[i];
        att->h_ldg_gain[i] = af_param->h_ldg_gain[i];
        att->h_ldg_gslp[i] = af_param->h_ldg_gslp[i];
        att->v_ldg_lumth[i] = af_param->v_ldg_lumth[i];
        att->v_ldg_gain[i] = af_param->v_ldg_gain[i];
        att->v_ldg_gslp[i] = af_param->v_ldg_gslp[i];
    }

    att->v_fv_thresh = af_param->v_fv_thresh;
    att->h_fv_thresh = af_param->h_fv_thresh;
    att->v1_fv_shift = af_param->v1_fv_shift;
    att->v2_fv_shift = af_param->v2_fv_shift;
    att->h1_fv_shift = af_param->h1_fv_shift;
    att->h2_fv_shift = af_param->h2_fv_shift;
    att->highlit_thresh = af_param->highlit_thresh;
#endif

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

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "af algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAfHandleInt::preProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPreAf* af_pre_int        = (RkAiqAlgoPreAf*)mPreInParam;
    RkAiqAlgoPreResAf* af_pre_res_int = (RkAiqAlgoPreResAf*)mPreOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::preProcess();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "af handle preProcess failed");
    }

    RkAiqAfStats* xAfStats = nullptr;
    if (shared->afStatsBuf) {
        xAfStats = (RkAiqAfStats*)shared->afStatsBuf->map(shared->afStatsBuf);
        if (!xAfStats) LOGE("af stats is null");
    } else {
        LOGW("the xcamvideobuffer of af stats is null");
    }

    if ((!xAfStats || !xAfStats->af_stats_valid) && !sharedCom->init) {
        LOGW("no af stats, ignore!");
        return XCAM_RETURN_BYPASS;
    }

    af_pre_int->xcam_af_stats  = shared->afStatsBuf;
    af_pre_int->xcam_aec_stats = shared->aecStatsBuf;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->pre_process(mPreInParam, mPreOutParam);
    RKAIQCORE_CHECK_RET(ret, "af algo pre_process failed");

    EXIT_ANALYZER_FUNCTION();
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
#if 0
    RkAiqAlgoProcResAf* af_proc_res_int = (RkAiqAlgoProcResAf*)mProcOutParam;
#else
    mProcResShared = new RkAiqAlgoProcResAfIntShared();
    if (!mProcResShared.ptr()) {
        LOGE("new af mProcOutParam failed, bypass!");
        return XCAM_RETURN_BYPASS;
    }
    RkAiqAlgoProcResAf* af_proc_res_int = &mProcResShared->result;
    // mProcOutParam = (RkAiqAlgoResCom*)af_proc_res_int;
#endif
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

#define ZOOM_MOVE_DEBUG
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
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "af handle processing failed");
    }

    RkAiqAfStats* xAfStats = nullptr;
    if (shared->afStatsBuf) {
        xAfStats = (RkAiqAfStats*)shared->afStatsBuf->map(shared->afStatsBuf);
        if (!xAfStats) LOGE("af stats is null");
    } else {
        LOGW("the xcamvideobuffer of af stats is null");
    }

    if ((!xAfStats || !xAfStats->af_stats_valid) && !sharedCom->init) {
        LOGW("no af stats, ignore!");
        mProcResShared.release();
        return XCAM_RETURN_BYPASS;
    }

    af_proc_int->xcam_af_stats  = shared->afStatsBuf;
    af_proc_int->xcam_aec_stats = shared->aecStatsBuf;
    af_proc_int->xcam_pdaf_stats = shared->pdafStatsBuf;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
#if 0
    ret = des->processing(mProcInParam, mProcOutParam);
#else
    ret = des->processing(mProcInParam, (RkAiqAlgoResCom*)af_proc_res_int);
#endif
    RKAIQCORE_CHECK_RET(ret, "af algo processing failed");

#if 1
    af_proc_res_int->id = shared->frameId;
    SmartPtr<BufferProxy> msg_data = new BufferProxy(mProcResShared);
    msg_data->set_sequence(shared->frameId);
    SmartPtr<XCamMessage> msg =
        new RkAiqCoreVdBufMsg(XCAM_MESSAGE_AF_PROC_RES_OK, af_proc_res_int->id, msg_data);
    mAiqCore->post_message(msg);
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAfHandleInt::postProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPostAf* af_post_int        = (RkAiqAlgoPostAf*)mPostInParam;
    RkAiqAlgoPostResAf* af_post_res_int = (RkAiqAlgoPostResAf*)mPostOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::postProcess();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "af handle postProcess failed");
        return ret;
    }

    RkAiqAfStats* xAfStats = nullptr;
    if (shared->afStatsBuf) {
        xAfStats = (RkAiqAfStats*)shared->afStatsBuf->map(shared->afStatsBuf);
        if (!xAfStats) LOGE("af stats is null");
    } else {
        LOGW("the xcamvideobuffer of af stats is null");
    }

    if ((!xAfStats || !xAfStats->af_stats_valid) && !sharedCom->init) {
        LOGW("no af stats, ignore!");
        return XCAM_RETURN_BYPASS;
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->post_process(mPostInParam, mPostOutParam);
    RKAIQCORE_CHECK_RET(ret, "af algo post_process failed");

    if (updateAtt && isUpdateAttDone) {
        mCurAtt         = mNewAtt;
        updateAtt       = false;
        isUpdateAttDone = false;
        sendSignal(mCurAtt.sync.sync_mode);
    }

    if (isUpdateZoomPosDone) {
        isUpdateZoomPosDone = false;
        sendSignal();
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAfHandleInt::genIspResult(RkAiqFullParams* params, RkAiqFullParams* cur_params) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    if (!mProcResShared.ptr())
        return XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcResAf* af_com                  = &mProcResShared->result;

#if defined(ISP_HW_V30)
    rk_aiq_isp_af_params_v3x_t* af_param = params->mAfV3xParams->data().ptr();
#else
    rk_aiq_isp_af_params_v20_t* af_param = params->mAfParams->data().ptr();
#endif

    SmartPtr<rk_aiq_focus_params_wrapper_t> focus_param = params->mFocusParams->data();
    rk_aiq_focus_params_t* p_focus_param                = &focus_param->result;

#if 0
    isp_param->af_cfg_update = false;
#else
    // af_param->af_cfg_update = false;
#endif
    memset(p_focus_param, 0, sizeof(rk_aiq_focus_params_t));
    if (!af_com) {
        LOGD_ANALYZER("no af result");
        return XCAM_RETURN_NO_ERROR;
    }

    if (!this->getAlgoId()) {
        RkAiqAlgoProcResAf* af_rk = (RkAiqAlgoProcResAf*)af_com;

#if 0
        isp_param->af_meas = af_rk->af_isp_param;
        isp_param->af_cfg_update = af_rk->af_cfg_update;
#else
        if (sharedCom->init) {
            af_param->frame_id    = 0;
            focus_param->frame_id = 0;
        } else {
            af_param->frame_id    = shared->frameId;
            focus_param->frame_id = shared->frameId;
        }
#if defined(ISP_HW_V30)
        af_param->result = af_rk->af_isp_param_v3x;
#else
        af_param->result = af_rk->af_isp_param;
#endif
        // isp_param->af_cfg_update = af_rk->af_cfg_update;
#endif
        p_focus_param->zoomfocus_modifypos =
            af_rk->af_focus_param.zoomfocus_modifypos;
        p_focus_param->focus_correction  = af_rk->af_focus_param.focus_correction;
        p_focus_param->zoom_correction   = af_rk->af_focus_param.zoom_correction;
        p_focus_param->lens_pos_valid    = af_rk->af_focus_param.lens_pos_valid;
        p_focus_param->zoom_pos_valid    = af_rk->af_focus_param.zoom_pos_valid;
        p_focus_param->send_zoom_reback  = af_rk->af_focus_param.send_zoom_reback;
        p_focus_param->send_focus_reback = af_rk->af_focus_param.send_focus_reback;
        p_focus_param->end_zoom_chg      = af_rk->af_focus_param.end_zoom_chg;
        p_focus_param->focus_noreback    = af_rk->af_focus_param.focus_noreback;
        p_focus_param->use_manual        = af_rk->af_focus_param.use_manual;
        p_focus_param->auto_focpos       = af_rk->af_focus_param.auto_focpos;
        p_focus_param->auto_zoompos      = af_rk->af_focus_param.auto_zoompos;
        p_focus_param->manual_focpos     = af_rk->af_focus_param.manual_focpos;
        p_focus_param->manual_zoompos    = af_rk->af_focus_param.manual_zoompos;
        p_focus_param->next_pos_num      = af_rk->af_focus_param.next_pos_num;
        for (int i = 0; i < af_rk->af_focus_param.next_pos_num; i++) {
            p_focus_param->next_lens_pos[i] =
                af_rk->af_focus_param.next_lens_pos[i];
            p_focus_param->next_zoom_pos[i] =
                af_rk->af_focus_param.next_zoom_pos[i];
        }

        p_focus_param->vcm_start_ma     = af_rk->af_focus_param.vcm_start_ma;
        p_focus_param->vcm_end_ma       = af_rk->af_focus_param.vcm_end_ma;
        p_focus_param->vcm_config_valid = af_rk->af_focus_param.vcm_config_valid;

#if defined(ISP_HW_V30) || defined(ISP_HW_V21)
        SmartPtr<RkAiqHandle>* ae_handle = mAiqCore->getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_AE);
        int algo_id                      = (*ae_handle)->getAlgoId();

        if (ae_handle) {
            if (algo_id == 0) {
                RkAiqAeHandleInt* ae_algo = dynamic_cast<RkAiqAeHandleInt*>(ae_handle->ptr());

                if (af_rk->lockae_en)
                    ae_algo->setLockAeForAf(af_rk->lockae);
                else
                    ae_algo->setLockAeForAf(false);
            }
        }
#endif
    }

#if defined(ISP_HW_V30)
    cur_params->mAfV3xParams = params->mAfV3xParams;
#else
    cur_params->mAfParams    = params->mAfParams;
#endif
    cur_params->mFocusParams = params->mFocusParams;

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

};  // namespace RkCam
