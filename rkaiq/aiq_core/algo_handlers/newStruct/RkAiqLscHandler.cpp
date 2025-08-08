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

#include "RkAiqLscHandler.h"
#include "algos/algo_types_priv.h"
#include "newStruct/lsc/include/lsc_algo_api.h"
#include "newStruct/algo_common.h"
#include "RkAiqCore.h"

namespace RkCam {

DEFINE_HANDLE_REGISTER_TYPE(RkAiqLscHandleInt);

void RkAiqLscHandleInt::init() {
    ENTER_ANALYZER_FUNCTION();

    RkAiqHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoCom());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcLsc());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoResCom());

    mResultType = RESULT_TYPE_LSC_PARAM;
    mResultSize = sizeof(lsc_param_t);

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqLscHandleInt::setAttrib(lsc_api_attrib_t* attr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    ret = algo_lsc_SetAttrib(mAlgoCtx, attr);
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqLscHandleInt::getAttrib(lsc_api_attrib_t* attr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mCfgMutex.lock();

    ret = algo_lsc_GetAttrib(mAlgoCtx, attr);

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqLscHandleInt::queryStatus(lsc_status_t* status) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mCfgMutex.lock();

    if (mAiqCore->mAiqCurParams->data().ptr() && mAiqCore->mAiqCurParams->data()->mLscParams.ptr()) {
        rk_aiq_isp_lsc_params_t* lsc_param = mAiqCore->mAiqCurParams->data()->mLscParams->data().ptr();
        if (lsc_param) {
            status->lscStatus = lsc_param->result;
            status->en = lsc_param->en;
            status->bypass = lsc_param->bypass;
            status->opMode = RK_AIQ_OP_MODE_AUTO;
        } else {
            ret = XCAM_RETURN_ERROR_FAILED;
            LOGE_ALSC("have no status info !");
        }
    } else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE_ALSC("have no status info !");
    }

    mCfgMutex.unlock();
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

RkAiqBayerPattern_t
RkAiqLscHandleInt::getBayerPattern(uint32_t pixelformat)
{
    ENTER_ANALYZER_FUNCTION();
    RkAiqBayerPattern_t bayerPattern = RK_AIQ_BAYER_INVALID;

    switch (pixelformat) {
    case V4L2_PIX_FMT_SRGGB8:
    case V4L2_PIX_FMT_SRGGB10:
    case V4L2_PIX_FMT_SRGGB12:
        bayerPattern = RK_AIQ_BAYER_RGGB;
        break;
    case V4L2_PIX_FMT_SBGGR8:
    case V4L2_PIX_FMT_SBGGR10:
    case V4L2_PIX_FMT_SBGGR12:
        bayerPattern = RK_AIQ_BAYER_BGGR;
        break;
    case V4L2_PIX_FMT_SGBRG8:
    case V4L2_PIX_FMT_SGBRG10:
    case V4L2_PIX_FMT_SGBRG12:
        bayerPattern = RK_AIQ_BAYER_GBRG;
        break;
    case V4L2_PIX_FMT_SGRBG8:
    case V4L2_PIX_FMT_SGRBG10:
    case V4L2_PIX_FMT_SGRBG12:
        bayerPattern = RK_AIQ_BAYER_GRBG;
        break;
    default:
        LOGD_ALSC("%s no support pixelformat:0x%x\n", __func__, pixelformat);
    }

    EXIT_ANALYZER_FUNCTION();
    return bayerPattern;
}

XCamReturn RkAiqLscHandleInt::prepare() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "lsc handle prepare failed");

    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    /*
    RkAiqAlgoConfigLsc* lsc_config_int = (RkAiqAlgoConfigLsc*)mConfig;

    lsc_config_int->illu_info.bayerPattern= getBayerPattern(sharedCom->snsDes.sensor_pixelformat);
    if (sharedCom->snsDes.otp_lsc && sharedCom->snsDes.otp_lsc->flag) {
        lsc_config_int->illu_info.otpInfo.flag = sharedCom->snsDes.otp_lsc->flag;
        lsc_config_int->lsc_sw_info.otpInfo.width = sharedCom->snsDes.otp_lsc->width;
        lsc_config_int->lsc_sw_info.otpInfo.height = sharedCom->snsDes.otp_lsc->height;
        lsc_config_int->lsc_sw_info.otpInfo.table_size = sharedCom->snsDes.otp_lsc->table_size;
        lsc_config_int->lsc_sw_info.otpInfo.lsc_r = sharedCom->snsDes.otp_lsc->lsc_r;
        lsc_config_int->lsc_sw_info.otpInfo.lsc_b = sharedCom->snsDes.otp_lsc->lsc_b;
        lsc_config_int->lsc_sw_info.otpInfo.lsc_gr = sharedCom->snsDes.otp_lsc->lsc_gr;
        lsc_config_int->lsc_sw_info.otpInfo.lsc_gb = sharedCom->snsDes.otp_lsc->lsc_gb;
    } else {
        lsc_config_int->lsc_sw_info.otpInfo.flag = 0;
    }
    */

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;

    ret                       = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "lsc algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqLscHandleInt::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoResCom* proc_res = mProcOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
            (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::processing();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "lsc handle processing failed");
    }

    proc_res->algoRes =  &shared->fullParams->mLscParams->data()->result;

    RkAiqAlgoProcLsc* proc_int        = (RkAiqAlgoProcLsc*)mProcInParam;
    RKAiqAecExpInfo_t* pCurExp = &shared->curExp;
    RkAiqAlgoProcResAwbShared_t* awb_res = NULL;
    XCamVideoBuffer* awb_proc_res = shared->res_comb.awb_proc_res;
    if (awb_proc_res) {
        awb_res = (RkAiqAlgoProcResAwbShared_t*)awb_proc_res->map(awb_proc_res);
    }
    get_illu_estm_info(&proc_int->illu_info, awb_res, pCurExp, sharedCom->working_mode);

    /*
    if(colorConstFlag==true){
        memcpy(lsc_proc_int->illu_info.awbGain,colorSwInfo.awbGain,sizeof(colorSwInfo.awbGain));
        lsc_proc_int->illu_info.sensorGain = colorSwInfo.sensorGain;
    }
    */

    ret = RkAiqHandle::do_processing_common();

    RKAIQCORE_CHECK_RET(ret, "Lsc algo processing failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqLscHandleInt::genIspResult(RkAiqFullParams* params,
        RkAiqFullParams* cur_params) {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret                = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqAlgoResCom* proc_res = mProcOutParam;

    rk_aiq_isp_lsc_params_t* lsc_param = params->mLscParams->data().ptr();

    if (!proc_res) {
        LOGD_ANALYZER("no lsc result");
        return XCAM_RETURN_NO_ERROR;
    }

    if (!this->getAlgoId()) {
        if (sharedCom->init) {
            lsc_param->frame_id = 0;
        } else {
            lsc_param->frame_id = shared->frameId;
        }

        if (proc_res->cfg_update) {
            mSyncFlag = shared->frameId;
            lsc_param->sync_flag = mSyncFlag;
            lsc_param->en = proc_res->en;
            lsc_param->bypass = proc_res->bypass;
            // copy from algo result
            // set as the latest result
            cur_params->mLscParams = params->mLscParams;
            lsc_param->is_update = true;
            LOGD_ALSC("[%d] params from algo", mSyncFlag);
        } else if (mSyncFlag != lsc_param->sync_flag) {
            lsc_param->sync_flag = mSyncFlag;
            // copy from latest result
            if (cur_params->mLscParams.ptr()) {
                lsc_param->result = cur_params->mLscParams->data()->result;
                lsc_param->en = cur_params->mLscParams->data()->en;
                lsc_param->bypass = cur_params->mLscParams->data()->bypass;
                lsc_param->is_update = true;
            } else {
                LOGE_ALSC("no latest params !");
                lsc_param->is_update = false;
            }
            LOGD_ALSC("[%d] params from latest [%d]", shared->frameId, mSyncFlag);
        } else {
            // do nothing, result in buf needn't update
            lsc_param->is_update = false;
            LOGD_ALSC("[%d] params needn't update", shared->frameId);
        }
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

XCamReturn RkAiqLscHandleInt::setAcolorSwInfo(rk_aiq_color_info_t aColor_sw_info) {
    ENTER_ANALYZER_FUNCTION();


    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    LOGV_ALSC("%s sensor gain = %f, wbgain=[%f,%f] ",__FUNCTION__,aColor_sw_info.sensorGain,
      aColor_sw_info.awbGain[0],aColor_sw_info.awbGain[1]);
    colorSwInfo = aColor_sw_info;
    colorConstFlag=true;
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

}  // namespace RkCam

