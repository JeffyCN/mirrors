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

#include "RkAiqCacHandler.h"
#include "algos/algo_types_priv.h"
#include "newStruct/cac/include/cac_algo_api.h"
#include "RkAiqCore.h"

namespace RkCam {

DEFINE_HANDLE_REGISTER_TYPE(RkAiqCacHandleInt);

void RkAiqCacHandleInt::init() {
    ENTER_ANALYZER_FUNCTION();

    RkAiqHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigCac());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcCac());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResCac());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqCacHandleInt::setAttrib(cac_api_attrib_t* attr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    ret = algo_cac_SetAttrib(mAlgoCtx, attr);
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCacHandleInt::getAttrib(cac_api_attrib_t* attr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mCfgMutex.lock();

    ret = algo_cac_GetAttrib(mAlgoCtx, attr);

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCacHandleInt::queryStatus(cac_status_t* status) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mCfgMutex.lock();

    if (mAiqCore->mAiqCurParams->data().ptr() && mAiqCore->mAiqCurParams->data()->mCacParams.ptr()) {
        rk_aiq_isp_cac_params_t* cac_param = mAiqCore->mAiqCurParams->data()->mCacParams->data().ptr();
        if (cac_param) {
            status->stMan = cac_param->result;
            status->en = cac_param->en;
            status->bypass = cac_param->bypass;
            status->opMode = RK_AIQ_OP_MODE_AUTO;
        } else {
            ret = XCAM_RETURN_ERROR_FAILED;
            LOGE_ACAC("have no status info !");
        }
    } else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE_ACAC("have no status info !");
    }

    mCfgMutex.unlock();
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCacHandleInt::prepare() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "cac handle prepare failed");

    RkAiqAlgoConfigCac* cac_config_int        = (RkAiqAlgoConfigCac*)mConfig;
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    auto* shared = (RkAiqCore::RkAiqAlgosGroupShared_t*)getGroupShared();
    if (!shared) return XCAM_RETURN_BYPASS;

    if (sharedCom->resourcePath) {
        strcpy(cac_config_int->iqpath, sharedCom->resourcePath);
    } else {
        strcpy(cac_config_int->iqpath, "/etc/iqfiles");
    }

    cac_config_int->mem_ops                  = mAiqCore->mShareMemOps;
    cac_config_int->width                    = sharedCom->snsDes.isp_acq_width;
    cac_config_int->height                   = sharedCom->snsDes.isp_acq_height;
    cac_config_int->is_multi_sensor          = sharedCom->is_multi_sensor;
    cac_config_int->is_multi_isp             = sharedCom->is_multi_isp_mode;
    cac_config_int->multi_isp_extended_pixel = sharedCom->multi_isp_extended_pixels;

    ret                       = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "cac algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqCacHandleInt::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoConfigCac* cac_config_int        = (RkAiqAlgoConfigCac*)mConfig;
    RkAiqAlgoProcResCac* cac_proc_res_int =
            (RkAiqAlgoProcResCac*)mProcOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
            (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqAlgoProcCac* cac_proc_int = (RkAiqAlgoProcCac*)mProcInParam;

    RKAiqAecExpInfo_t* aeCurExp = &shared->curExp;
    cac_proc_int->hdr_ratio = 1;
    cac_proc_int->iso = 50;
    if (aeCurExp != NULL) {
        if (sharedCom->working_mode == (int)RK_AIQ_WORKING_MODE_NORMAL) {
            cac_proc_int->hdr_ratio = 1;
            cac_proc_int->iso = aeCurExp->LinearExp.exp_real_params.analog_gain * 50;
            LOGD_ACAC("%s:NORMAL:iso=%d,again=%f\n", __FUNCTION__, cac_proc_int->iso,
                      aeCurExp->LinearExp.exp_real_params.analog_gain);
        } else if (sharedCom->working_mode == (int)RK_AIQ_ISP_HDR_MODE_2_FRAME_HDR) {
            cac_proc_int->hdr_ratio = (aeCurExp->HdrExp[1].exp_real_params.analog_gain *
                                        aeCurExp->HdrExp[1].exp_real_params.integration_time) /
                                       (aeCurExp->HdrExp[0].exp_real_params.analog_gain *
                                        aeCurExp->HdrExp[0].exp_real_params.integration_time);
            cac_proc_int->iso = aeCurExp->HdrExp[1].exp_real_params.analog_gain * 50;
            LOGD_ACAC("%s:HDR2:iso=%d,again=%f ratio %f\n", __FUNCTION__, cac_proc_int->iso,
                      aeCurExp->HdrExp[1].exp_real_params.analog_gain, cac_proc_int->hdr_ratio);
        } else if (sharedCom->working_mode == (int)RK_AIQ_ISP_HDR_MODE_3_FRAME_HDR) {
            cac_proc_int->hdr_ratio = (aeCurExp->HdrExp[2].exp_real_params.analog_gain *
                                        aeCurExp->HdrExp[2].exp_real_params.integration_time) /
                                       (aeCurExp->HdrExp[0].exp_real_params.analog_gain *
                                        aeCurExp->HdrExp[0].exp_real_params.integration_time);
            cac_proc_int->iso = aeCurExp->HdrExp[2].exp_real_params.analog_gain * 50;
            LOGD_ACAC("%s:HDR3:iso=%d,again=%f\n", __FUNCTION__, cac_proc_int->iso,
                      aeCurExp->HdrExp[2].exp_real_params.analog_gain);
        }
    } else {
        cac_proc_int->iso = 50;
        LOGE_ACAC("%s: pAEPreRes is NULL, so use default instead \n", __FUNCTION__);
    }

    ret = RkAiqHandle::processing();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "cac handle processing failed");
    }

    cac_proc_res_int->cacRes =  &shared->fullParams->mCacParams->data()->result;

    GlobalParamsManager* globalParamsManager = mAiqCore->getGlobalParamsManager();

    if (globalParamsManager && !globalParamsManager->isFullManualMode() &&
        globalParamsManager->isManual(RESULT_TYPE_CAC_PARAM)) {
        rk_aiq_global_params_wrap_t wrap_param;
        wrap_param.type = RESULT_TYPE_CAC_PARAM;
        wrap_param.man_param_size = sizeof(cac_param_t);
        wrap_param.man_param_ptr = cac_proc_res_int->cacRes;
        XCamReturn ret1 = globalParamsManager->getAndClearPending(&wrap_param);
        if (ret1 == XCAM_RETURN_NO_ERROR) {
            LOGK_ACAC("get new manual params success !");
            cac_proc_res_int->res_com.en = wrap_param.en;
            cac_proc_res_int->res_com.bypass = wrap_param.bypass;
            cac_proc_res_int->res_com.cfg_update = true;
        } else {
            cac_proc_res_int->res_com.cfg_update = false;
        }
        cac_proc_res_int->cacRes->sta.psfParam.hw_cacCfg_psfBlock_num =
            cac_config_int->PsfCfgCount;
        cac_proc_res_int->cacRes->sta.lutBuf[0].sw_cacCfg_lutBuf_fd =
            cac_config_int->Fd0;
        cac_proc_res_int->cacRes->sta.lutBuf[1].sw_cacCfg_lutBuf_fd =
            cac_config_int->Fd1;
        cac_proc_res_int->cacRes->sta.lutBuf[0].sw_cacCfg_lutBufSize_height =
            cac_config_int->LutHCount * CacPsfKernelWordSizeInMemory;
        cac_proc_res_int->cacRes->sta.lutBuf[0].sw_cacCfg_lutBufSize_width =
            cac_config_int->LutVCount * CacChannelCount;
    }
    else {
        globalParamsManager->lockAlgoParam(RESULT_TYPE_CAC_PARAM);
        mProcInParam->u.proc.is_attrib_update = globalParamsManager->getAndClearAlgoParamUpdateFlagLocked(RESULT_TYPE_CAC_PARAM);

        RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
        ret                       = des->processing(mProcInParam, mProcOutParam);
        globalParamsManager->unlockAlgoParam(RESULT_TYPE_CAC_PARAM);
    }

    RKAIQCORE_CHECK_RET(ret, "Cac algo processing failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCacHandleInt::genIspResult(RkAiqFullParams* params,
        RkAiqFullParams* cur_params) {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret                = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqAlgoProcResCac* cac_res = (RkAiqAlgoProcResCac*)mProcOutParam;

    rk_aiq_isp_cac_params_t* cac_param = params->mCacParams->data().ptr();

    if (!cac_res) {
        LOGD_ANALYZER("no cac result");
        return XCAM_RETURN_NO_ERROR;
    }

    if (!this->getAlgoId()) {
        if (sharedCom->init) {
            cac_param->frame_id = 0;
        } else {
            cac_param->frame_id = shared->frameId;
        }

        if (cac_res->res_com.cfg_update) {
            mSyncFlag = shared->frameId;
            cac_param->sync_flag = mSyncFlag;
            cac_param->en = cac_res->res_com.en;
            cac_param->bypass = cac_res->res_com.bypass;
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
                cac_param->en = cur_params->mCacParams->data()->en;
                cac_param->bypass = cur_params->mCacParams->data()->bypass;
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

