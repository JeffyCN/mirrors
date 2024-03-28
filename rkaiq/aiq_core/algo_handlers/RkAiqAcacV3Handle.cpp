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
#include "RkAiqAcacV3Handle.h"

#include "RkAiqCore.h"

namespace RkCam {

DEFINE_HANDLE_REGISTER_TYPE(RkAiqAcacV3HandleInt);

XCamReturn RkAiqAcacV3HandleInt::prepare() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "acac handle prepare failed");

    RkAiqAlgoConfigAcac* acac_config_int        = (RkAiqAlgoConfigAcac*)mConfig;
    RkAiqAlgoDescription* des                   = (RkAiqAlgoDescription*)mDes;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    auto* shared = (RkAiqCore::RkAiqAlgosGroupShared_t*)getGroupShared();
    if (!shared) return XCAM_RETURN_BYPASS;

    acac_config_int->mem_ops                  = mAiqCore->mShareMemOps;
    acac_config_int->width                    = sharedCom->snsDes.isp_acq_width;
    acac_config_int->height                   = sharedCom->snsDes.isp_acq_height;
    acac_config_int->is_multi_sensor          = sharedCom->is_multi_sensor;
    acac_config_int->is_multi_isp             = sharedCom->is_multi_isp_mode;
    acac_config_int->multi_isp_extended_pixel = sharedCom->multi_isp_extended_pixels;

    ret = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "acac algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

void RkAiqAcacV3HandleInt::init() {
    ENTER_ANALYZER_FUNCTION();

    RkAiqHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAcac());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAcac());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAcac());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqAcacV3HandleInt::preProcess() {
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAcacV3HandleInt::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAcac* acac_proc_int            = (RkAiqAlgoProcAcac*)mProcInParam;
    RkAiqAlgoProcResAcac* acac_proc_res_int     = (RkAiqAlgoProcResAcac*)mProcOutParam;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    auto* shared = (RkAiqCore::RkAiqAlgosGroupShared_t*)getGroupShared();
    if (!shared) return XCAM_RETURN_BYPASS;

    acac_proc_res_int->config = shared->fullParams->mCacParams->data()->result.cfg;

    RKAiqAecExpInfo_t* aeCurExp = &shared->curExp;
    if (aeCurExp != NULL) {
        if ((rk_aiq_working_mode_t)sharedCom->working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
            acac_proc_int->iso = aeCurExp->LinearExp.exp_real_params.analog_gain * 50;
            LOGD_ACAC("%s:NORMAL:iso=%d,again=%f\n", __FUNCTION__, acac_proc_int->iso,
                      aeCurExp->LinearExp.exp_real_params.analog_gain);
        } else if ((rk_aiq_working_mode_t)sharedCom->working_mode == RK_AIQ_WORKING_MODE_ISP_HDR2) {
            acac_proc_int->iso = aeCurExp->HdrExp[1].exp_real_params.analog_gain * 50;
            LOGD_ACAC("%s:HDR2:iso=%d,again=%f\n", __FUNCTION__, acac_proc_int->iso,
                      aeCurExp->HdrExp[1].exp_real_params.analog_gain);
        } else if ((rk_aiq_working_mode_t)sharedCom->working_mode == RK_AIQ_WORKING_MODE_ISP_HDR3) {
            acac_proc_int->iso = aeCurExp->HdrExp[2].exp_real_params.analog_gain * 50;
            LOGD_ACAC("%s:HDR3:iso=%d,again=%f\n", __FUNCTION__, acac_proc_int->iso,
                      aeCurExp->HdrExp[2].exp_real_params.analog_gain);
        }
    } else {
        acac_proc_int->iso = 50;
        LOGE_ACAC("%s: pAEPreRes is NULL, so use default instead \n", __FUNCTION__);
    }

    ret = RkAiqHandle::processing();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "acac handle processing failed");
    }

#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
#endif
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;

    ret = des->processing(mProcInParam, mProcOutParam);
#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.unlock();
#endif
    RKAIQCORE_CHECK_RET(ret, "acac algo processing failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAcacV3HandleInt::postProcess() {
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAcacV3HandleInt::updateConfig(bool needSync) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#ifndef DISABLE_HANDLE_ATTRIB
    if (needSync) mCfgMutex.lock();
    if (updateAtt) {
        mCurAtt   = mNewAtt;
        updateAtt = false;
        rk_aiq_uapi_acac_v03_SetAttrib(mAlgoCtx, &mCurAtt, false);
        sendSignal();
    }

    if (needSync) mCfgMutex.unlock();
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAcacV3HandleInt::setAttrib(const rkaiq_cac_v03_api_attr_t* att) {
    ENTER_ANALYZER_FUNCTION();

    if (att == nullptr) return XCAM_RETURN_ERROR_PARAM;

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

    // check if there is different between att & mCurAtt(sync)/mNewAtt(async)
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore
#ifdef DISABLE_HANDLE_ATTRIB
    ret = rk_aiq_uapi_acac_v03_SetAttrib(mAlgoCtx, att, false);
#else
    bool isChanged = false;
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC && memcmp(&mNewAtt, att, sizeof(*att)))
        isChanged = true;
    else if (att->sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC && memcmp(&mCurAtt, att, sizeof(*att)))
        isChanged = true;

    // if something changed
    if (isChanged) {
        mNewAtt   = *att;
        updateAtt = true;
        waitSignal(att->sync.sync_mode);
    }
#endif

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAcacV3HandleInt::getAttrib(rkaiq_cac_v03_api_attr_t* att) {
    ENTER_ANALYZER_FUNCTION();

    if (att == nullptr) return XCAM_RETURN_ERROR_PARAM;

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#ifdef DISABLE_HANDLE_ATTRIB
      mCfgMutex.lock();
      ret = rk_aiq_uapi_acac_v03_GetAttrib(mAlgoCtx, att);
      mCfgMutex.unlock();
#else
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_acac_v03_GetAttrib(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateAtt) {
            memcpy(att, &mNewAtt, sizeof(mNewAtt));
            att->sync.done = false;
        } else {
            rk_aiq_uapi_acac_v03_GetAttrib(mAlgoCtx, att);
            att->sync.sync_mode = mNewAtt.sync.sync_mode;
            att->sync.done      = true;
        }
    }
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAcacV3HandleInt::genIspResult(RkAiqFullParams* params,
                                              RkAiqFullParams* cur_params) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqAlgoProcResAcac* cac_com               = (RkAiqAlgoProcResAcac*)mProcOutParam;

    rk_aiq_isp_cac_params_t* cac_param = params->mCacParams->data().ptr();

    if (!this->getAlgoId()) {
        RkAiqAlgoProcResAcac* cac_rk = (RkAiqAlgoProcResAcac*)cac_com;
        if (sharedCom->init) {
            cac_param->frame_id = 0;
        } else {
            cac_param->frame_id = shared->frameId;
        }
        cac_param->result.enable = cac_rk->enable;

        if (cac_com->res_com.cfg_update) {
            mSyncFlag = shared->frameId;
            cac_param->sync_flag = mSyncFlag;
            // copy from algo result
            // set as the latest result
            cur_params->mCacParams = params->mCacParams;
            cac_param->is_update = true;
            LOGD_ACAC("[%d] params from algo", mSyncFlag);
        } else if (mSyncFlag != cac_param->sync_flag) {
            cac_param->sync_flag = mSyncFlag;
            // copy from latest result
            if (cur_params->mCacParams.ptr()) {
                cac_param->result = cur_params->mCacParams->data()->result;
                cac_param->is_update = true;
            } else {
                LOGE_ACAC("no latest params !");
                cac_param->is_update = false;
            }
            LOGD_ACAC("[%d] params from latest [%d]", shared->frameId, mSyncFlag);
        } else {
            // do nothing, result in buf needn't update
            cac_param->is_update = false;
            LOGD_ACAC("[%d] params needn't update", shared->frameId);
        }
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

}  // namespace RkCam
