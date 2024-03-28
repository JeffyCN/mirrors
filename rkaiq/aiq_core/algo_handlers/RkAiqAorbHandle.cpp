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
#include "RkAiqAorbHandle.h"

#include "RkAiqCore.h"

namespace RkCam {

DEFINE_HANDLE_REGISTER_TYPE(RkAiqAorbHandleInt);

void RkAiqAorbHandleInt::init() {
    ENTER_ANALYZER_FUNCTION();

    RkAiqHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAorb());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAorb());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAorb());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAorb());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAorb());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAorb());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAorb());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqAorbHandleInt::prepare() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "aorb handle prepare failed");

    RkAiqAlgoConfigAorb* aorb_config_int     = (RkAiqAlgoConfigAorb*)mConfig;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    CalibDbV2_Orb_t* orb =
        (CalibDbV2_Orb_t*)CALIBDBV2_GET_MODULE_PTR((void*)(sharedCom->calibv2), orb);
    if (orb) {
        aorb_config_int->orb_calib_cfg.param.orb_en = orb->param.orb_en;
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "aorb algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAorbHandleInt::preProcess() { return XCAM_RETURN_NO_ERROR; }

XCamReturn RkAiqAorbHandleInt::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAorb* aorb_proc_int        = (RkAiqAlgoProcAorb*)mProcInParam;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    auto* shared = (RkAiqCore::RkAiqAlgosGroupShared_t*)getGroupShared();
    if (!shared) return XCAM_RETURN_BYPASS;

    aorb_proc_int->orb_stats_buf = shared->orbStats;

    ret = RkAiqHandle::processing();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "aorb handle processing failed");
    }

    if (!shared->orbStats && !sharedCom->init) {
    LOGE_AORB("no orb stats, ignore!");
    return XCAM_RETURN_BYPASS;
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->processing(mProcInParam, mProcOutParam);
    RKAIQCORE_CHECK_RET(ret, "aorb algo processing failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAorbHandleInt::postProcess() { return XCAM_RETURN_NO_ERROR; }

XCamReturn RkAiqAorbHandleInt::genIspResult(RkAiqFullParams* params, RkAiqFullParams* cur_params) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqAlgoProcResAorb* aorb_com              = (RkAiqAlgoProcResAorb*)mProcOutParam;

    if (!aorb_com) {
        LOGE_ANALYZER("no aorb result");
        return XCAM_RETURN_NO_ERROR;
    }

    if (!this->getAlgoId()) {
        RkAiqAlgoProcResAorb* aorb_rk        = (RkAiqAlgoProcResAorb*)aorb_com;
        rk_aiq_isp_orb_params_t* orb_params = params->mOrbParams->data().ptr();
        if (orb_params != nullptr) {
            if (sharedCom->init) {
                orb_params->frame_id = 0;
            } else {
                orb_params->frame_id = shared->frameId;
            }
            if (aorb_rk->aorb_meas.update) {
                orb_params->update_mask |= RKAIQ_ISPP_ORB_ID;
                orb_params->result.orb_en = aorb_rk->aorb_meas.orb_en;
                if (orb_params->result.orb_en) {
                    orb_params->result.limit_value = aorb_rk->aorb_meas.limit_value;
                    orb_params->result.max_feature = aorb_rk->aorb_meas.max_feature;
                }
            } else {
                orb_params->update_mask &= ~RKAIQ_ISPP_ORB_ID;
            }
        }
    }

    cur_params->mOrbParams = params->mOrbParams;

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

}  // namespace RkCam
