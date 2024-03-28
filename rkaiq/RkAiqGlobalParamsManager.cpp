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

#include "RkAiqGlobalParamsManager.h"

using namespace XCam;
namespace RkCam {

GlobalParamsManager::GlobalParamsManager ()
{
    ENTER_ANALYZER_FUNCTION();
    memset(mGlobalParams, 0, sizeof(mGlobalParams));
    EXIT_ANALYZER_FUNCTION();
}

GlobalParamsManager::~GlobalParamsManager ()
{
    ENTER_ANALYZER_FUNCTION();
    EXIT_ANALYZER_FUNCTION();
}

void
GlobalParamsManager::init_fullManual()
{
    ENTER_ANALYZER_FUNCTION();
    if (!mFullManualParamsProxy.ptr()) {
        mFullManualParamsProxy = new RkAiqFullParamsProxy(new RkAiqFullParams());

        mFullManualParamsProxy->data()->mExposureParams =
            new RkAiqExpParamsProxy(new rk_aiq_exposure_params_wrapper_t());
        mFullManualParamsProxyArray[RESULT_TYPE_EXPOSURE_PARAM] =
            mFullManualParamsProxy->data()->mExposureParams;
        mGlobalParams[RESULT_TYPE_EXPOSURE_PARAM].man_param_ptr =
            mFullManualParamsProxy->data()->mExposureParams->data().ptr();

#if RKAIQ_HAVE_BLC_V32
        mFullManualParamsProxy->data()->mBlcParams =
            new RkAiqIspBlcParamsProxy(new rk_aiq_isp_blc_params_t());
        mFullManualParamsProxyArray[RESULT_TYPE_BLC_PARAM] =
            mFullManualParamsProxy->data()->mBlcParams;
        mGlobalParams[RESULT_TYPE_BLC_PARAM].man_param_ptr =
            mFullManualParamsProxy->data()->mBlcParams->data().ptr();
#endif
        // TODO: init other module proxy
    }
    EXIT_ANALYZER_FUNCTION();
}

void
GlobalParamsManager::init_withCalib()
{
    ENTER_ANALYZER_FUNCTION();

    mIsAlgoParamUpdateBits = 0;
#if USE_NEWSTRUCT
    rk_aiq_global_params_ptr_wrap_t* wrap_ptr = &mGlobalParams[RESULT_TYPE_DEBAYER_PARAM];
    dm_api_attrib_t* dm_calib = (dm_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(
                (void*)(mCalibDb), demosaic));
    if (dm_calib) {
        wrap_ptr->opMode = &dm_calib->opMode;
        wrap_ptr->en = &dm_calib->en;
        wrap_ptr->bypass = &dm_calib->bypass;
        wrap_ptr->man_param_ptr = &dm_calib->stMan;
        wrap_ptr->aut_param_ptr = &dm_calib->stAuto;
        if (dm_calib->opMode == RK_AIQ_OP_MODE_MANUAL)
            mIsGlobalModulesUpdateBits |= ((uint64_t)1) << RESULT_TYPE_DEBAYER_PARAM;
        else if (dm_calib->opMode == RK_AIQ_OP_MODE_INVALID) {
            dm_calib->opMode = RK_AIQ_OP_MODE_MANUAL;
        }
        LOGK("Module DM: opMode:%d,en:%d,bypass:%d,man_ptr:%p",
             *wrap_ptr->opMode, *wrap_ptr->en, *wrap_ptr->bypass, wrap_ptr->man_param_ptr);
    } else {
        LOGE("no dm calib !");
    }
    wrap_ptr = &mGlobalParams[RESULT_TYPE_TNR_PARAM];
    btnr_api_attrib_t* btnr_calib = (btnr_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(
                (void*)(mCalibDb), bayertnr));
    if (btnr_calib) {
        wrap_ptr->opMode = &btnr_calib->opMode;
        wrap_ptr->en = &btnr_calib->en;
        wrap_ptr->bypass = &btnr_calib->bypass;
        wrap_ptr->man_param_ptr = &btnr_calib->stMan;
        wrap_ptr->aut_param_ptr = &btnr_calib->stAuto;
        if (btnr_calib->opMode == RK_AIQ_OP_MODE_MANUAL)
            mIsGlobalModulesUpdateBits |= ((uint64_t)1) << RESULT_TYPE_TNR_PARAM;
        else if (btnr_calib->opMode == RK_AIQ_OP_MODE_INVALID) {
            btnr_calib->opMode = RK_AIQ_OP_MODE_MANUAL;
        }
        LOGK("Module BTNR: opMode:%d,en:%d,bypass:%d,man_ptr:%p",
             *wrap_ptr->opMode, *wrap_ptr->en, *wrap_ptr->bypass, wrap_ptr->man_param_ptr);
    } else {
        LOGE("no btnr calib !");
    }
    wrap_ptr = &mGlobalParams[RESULT_TYPE_YNR_PARAM];
    ynr_api_attrib_t* ynr_calib = (ynr_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(
                (void*)(mCalibDb), ynr));
    if (ynr_calib) {
        wrap_ptr->opMode = &ynr_calib->opMode;
        wrap_ptr->en = &ynr_calib->en;
        wrap_ptr->bypass = &ynr_calib->bypass;
        wrap_ptr->man_param_ptr = &ynr_calib->stMan;
        wrap_ptr->aut_param_ptr = &ynr_calib->stAuto;
        if (ynr_calib->opMode == RK_AIQ_OP_MODE_MANUAL)
            mIsGlobalModulesUpdateBits |= ((uint64_t)1) << RESULT_TYPE_YNR_PARAM;
        else if (ynr_calib->opMode == RK_AIQ_OP_MODE_INVALID) {
            ynr_calib->opMode = RK_AIQ_OP_MODE_MANUAL;
        }
        LOGK("Module YNR: opMode:%d,en:%d,bypass:%d,man_ptr:%p",
             *wrap_ptr->opMode, *wrap_ptr->en, *wrap_ptr->bypass, wrap_ptr->man_param_ptr);
    } else {
        LOGE("no ynr calib !");
    }
    wrap_ptr = &mGlobalParams[RESULT_TYPE_UVNR_PARAM];
    cnr_api_attrib_t* cnr_calib = (cnr_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(
                (void*)(mCalibDb), cnr));
    if (cnr_calib) {
        wrap_ptr->opMode = &cnr_calib->opMode;
        wrap_ptr->en = &cnr_calib->en;
        wrap_ptr->bypass = &cnr_calib->bypass;
        wrap_ptr->man_param_ptr = &cnr_calib->stMan;
        wrap_ptr->aut_param_ptr = &cnr_calib->stAuto;
        if (cnr_calib->opMode == RK_AIQ_OP_MODE_MANUAL)
            mIsGlobalModulesUpdateBits |= ((uint64_t)1) << RESULT_TYPE_UVNR_PARAM;
        else if (cnr_calib->opMode == RK_AIQ_OP_MODE_INVALID) {
            cnr_calib->opMode = RK_AIQ_OP_MODE_MANUAL;
        }
        LOGK("Module CNR: opMode:%d,en:%d,bypass:%d,man_ptr:%p",
             *wrap_ptr->opMode, *wrap_ptr->en, *wrap_ptr->bypass, wrap_ptr->man_param_ptr);
    } else {
        LOGE("no cnr calib !");
    }
    wrap_ptr = &mGlobalParams[RESULT_TYPE_AGAMMA_PARAM];
    gamma_api_attrib_t* gamma_calib = (gamma_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(
                (void*)(mCalibDb), gamma));
    if (gamma_calib) {
        wrap_ptr->opMode = &gamma_calib->opMode;
        wrap_ptr->en = &gamma_calib->en;
        wrap_ptr->bypass = &gamma_calib->bypass;
        wrap_ptr->man_param_ptr = &gamma_calib->stMan;
        wrap_ptr->aut_param_ptr = &gamma_calib->stAuto;
        if (gamma_calib->opMode == RK_AIQ_OP_MODE_MANUAL)
            mIsGlobalModulesUpdateBits |= ((uint64_t)1) << RESULT_TYPE_AGAMMA_PARAM;
        else if (gamma_calib->opMode == RK_AIQ_OP_MODE_INVALID) {
            gamma_calib->opMode = RK_AIQ_OP_MODE_MANUAL;
        }
        LOGK("Module gamma: opMode:%d,en:%d,bypass:%d,man_ptr:%p",
             *wrap_ptr->opMode, *wrap_ptr->en, *wrap_ptr->bypass, wrap_ptr->man_param_ptr);
    } else {
        LOGE("no gamma calib !");
    }
    wrap_ptr = &mGlobalParams[RESULT_TYPE_SHARPEN_PARAM];
    sharp_api_attrib_t* sharp_calib = (sharp_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(
                (void*)(mCalibDb), sharp));
    if (sharp_calib) {
        wrap_ptr->opMode = &sharp_calib->opMode;
        wrap_ptr->en = &sharp_calib->en;
        wrap_ptr->bypass = &sharp_calib->bypass;
        wrap_ptr->man_param_ptr = &sharp_calib->stMan;
        wrap_ptr->aut_param_ptr = &sharp_calib->stAuto;
        if (sharp_calib->opMode == RK_AIQ_OP_MODE_MANUAL)
            mIsGlobalModulesUpdateBits |= ((uint64_t)1) << RESULT_TYPE_SHARPEN_PARAM;
        else if (sharp_calib->opMode == RK_AIQ_OP_MODE_INVALID) {
            sharp_calib->opMode = RK_AIQ_OP_MODE_MANUAL;
        }
        LOGK("Module SHARP: opMode:%d,en:%d,bypass:%d,man_ptr:%p",
             *wrap_ptr->opMode, *wrap_ptr->en, *wrap_ptr->bypass, wrap_ptr->man_param_ptr);
    }
    else {
        LOGE("no sharp calib !");
    }

    wrap_ptr = &mGlobalParams[RESULT_TYPE_DRC_PARAM];
    drc_api_attrib_t* drc_calib = (drc_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(
                (void*)(mCalibDb), drc));
    if (drc_calib) {
        wrap_ptr->opMode = &drc_calib->opMode;
        wrap_ptr->en = &drc_calib->en;
        wrap_ptr->bypass = &drc_calib->bypass;
        wrap_ptr->man_param_ptr = &drc_calib->stMan;
        wrap_ptr->aut_param_ptr = &drc_calib->stAuto;
        if (drc_calib->opMode == RK_AIQ_OP_MODE_MANUAL)
            mIsGlobalModulesUpdateBits |= ((uint64_t)1) << RESULT_TYPE_DRC_PARAM;
        else if (drc_calib->opMode == RK_AIQ_OP_MODE_INVALID) {
            drc_calib->opMode = RK_AIQ_OP_MODE_MANUAL;
        }
        LOGK("Module drc: opMode:%d,en:%d,bypass:%d,man_ptr:%p",
             *wrap_ptr->opMode, *wrap_ptr->en, *wrap_ptr->bypass, wrap_ptr->man_param_ptr);
    } else {
        LOGE("no drc calib !");
    }

    wrap_ptr = &mGlobalParams[RESULT_TYPE_TRANS_PARAM];
    trans_api_attrib_t* trans_calib = (trans_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(
                (void*)(mCalibDb), trans));
    if (trans_calib) {
        wrap_ptr->opMode = &trans_calib->opMode;
        wrap_ptr->en = &trans_calib->en;
        wrap_ptr->bypass = &trans_calib->bypass;
        wrap_ptr->man_param_ptr = &trans_calib->stMan;
        if (trans_calib->opMode == RK_AIQ_OP_MODE_MANUAL)
            mIsGlobalModulesUpdateBits |= ((uint64_t)1) << RESULT_TYPE_DRC_PARAM;
        else if (trans_calib->opMode == RK_AIQ_OP_MODE_INVALID) {
            trans_calib->opMode = RK_AIQ_OP_MODE_MANUAL;
        }
        LOGK("Module trans: opMode:%d,en:%d,bypass:%d,man_ptr:%p",
             *wrap_ptr->opMode, *wrap_ptr->en, *wrap_ptr->bypass, wrap_ptr->man_param_ptr);
    } else {
        LOGE("no trans calib !");
    }

    wrap_ptr = &mGlobalParams[RESULT_TYPE_DEHAZE_PARAM];
    dehaze_api_attrib_t* dehaze_calib = (dehaze_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(
                (void*)(mCalibDb), dehaze));
    if (dehaze_calib) {
        wrap_ptr->opMode = &dehaze_calib->opMode;
        wrap_ptr->en = &dehaze_calib->en;
        wrap_ptr->bypass = &dehaze_calib->bypass;
        wrap_ptr->man_param_ptr = &dehaze_calib->stMan;
        wrap_ptr->aut_param_ptr = &dehaze_calib->stAuto;
        if (dehaze_calib->opMode == RK_AIQ_OP_MODE_MANUAL)
            mIsGlobalModulesUpdateBits |= ((uint64_t)1) << RESULT_TYPE_DEHAZE_PARAM;
        else if (dehaze_calib->opMode == RK_AIQ_OP_MODE_INVALID) {
            dehaze_calib->opMode = RK_AIQ_OP_MODE_MANUAL;
        }
        LOGK("Module dehaze: opMode:%d,en:%d,bypass:%d,man_ptr:%p",
             *wrap_ptr->opMode, *wrap_ptr->en, *wrap_ptr->bypass, wrap_ptr->man_param_ptr);
    }
    else {
        LOGE("no dehaze calib !");
    }

    wrap_ptr = &mGlobalParams[RESULT_TYPE_BLC_PARAM];
    blc_api_attrib_t* blc_calib = (blc_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(
                (void*)(mCalibDb), blc));
    if (blc_calib) {
        wrap_ptr->opMode = &blc_calib->opMode;
        wrap_ptr->en = &blc_calib->en;
        wrap_ptr->bypass = &blc_calib->bypass;
        wrap_ptr->man_param_ptr = &blc_calib->stMan;
        wrap_ptr->aut_param_ptr = &blc_calib->stAuto;
        if (blc_calib->opMode == RK_AIQ_OP_MODE_MANUAL)
            mIsGlobalModulesUpdateBits |= ((uint64_t)1) << RESULT_TYPE_BLC_PARAM;
        else if (blc_calib->opMode == RK_AIQ_OP_MODE_INVALID) {
            blc_calib->opMode = RK_AIQ_OP_MODE_MANUAL;
        }
        LOGK("Module BLC: opMode:%d,en:%d,bypass:%d,man_ptr:%p",
             *wrap_ptr->opMode, *wrap_ptr->en, *wrap_ptr->bypass, wrap_ptr->man_param_ptr);
    } else {
        LOGE("no blc calib !");
    }

    wrap_ptr = &mGlobalParams[RESULT_TYPE_DPCC_PARAM];
    dpc_api_attrib_t* dpc_calib = (dpc_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(
                (void*)(mCalibDb), dpc));
    if (dpc_calib) {
        wrap_ptr->opMode = &dpc_calib->opMode;
        wrap_ptr->en = &dpc_calib->en;
        wrap_ptr->bypass = &dpc_calib->bypass;
        wrap_ptr->man_param_ptr = &dpc_calib->stMan;
        wrap_ptr->aut_param_ptr = &dpc_calib->stAuto;
        if (dpc_calib->opMode == RK_AIQ_OP_MODE_MANUAL)
            mIsGlobalModulesUpdateBits |= ((uint64_t)1) << RESULT_TYPE_DPCC_PARAM;
        else if (dpc_calib->opMode == RK_AIQ_OP_MODE_INVALID) {
            dpc_calib->opMode = RK_AIQ_OP_MODE_MANUAL;
        }
        LOGK("Module DPC: opMode:%d,en:%d,bypass:%d,man_ptr:%p",
             *wrap_ptr->opMode, *wrap_ptr->en, *wrap_ptr->bypass, wrap_ptr->man_param_ptr);
    } else {
        LOGE("no dpc calib !");
    }
#endif
    EXIT_ANALYZER_FUNCTION();
}

void
GlobalParamsManager::init(bool isFullManMode, CamCalibDbV2Context_t* calibDb)
{
    ENTER_ANALYZER_FUNCTION();
    mFullManualMode = isFullManMode;
    mCalibDb = calibDb;

    if (isFullManMode) {
        init_fullManual();
    } else {
        init_withCalib();
    }
    EXIT_ANALYZER_FUNCTION();
}

void
GlobalParamsManager::switchCalibDb(CamCalibDbV2Context_t* calibDb)
{
    ENTER_ANALYZER_FUNCTION();

    SmartLock lock(mMutex);

    mCalibDb = calibDb;
    init_withCalib();

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn
GlobalParamsManager::set(rk_aiq_global_params_wrap_t* param)
{
    ENTER_ANALYZER_FUNCTION();

    if (param->type < 0 || param->type >= RESULT_TYPE_MAX_PARAM)
        return XCAM_RETURN_ERROR_OUTOFRANGE;

    mAlgoMutex[param->type].lock();

    rk_aiq_global_params_ptr_wrap_t* wrap_ptr = &mGlobalParams[param->type];

    bool isUpdateOpMode = false;
    if (*wrap_ptr->opMode != param->opMode) {
        isUpdateOpMode = true;
        *wrap_ptr->opMode = param->opMode;
    }

    bool isUpdateEn = false;
    if (*wrap_ptr->en != param->en) {
        isUpdateEn = true;
        *wrap_ptr->en = param->en;
    }

    bool isUpdateBypass = false;
    if (*wrap_ptr->bypass != param->bypass) {
        isUpdateBypass = true;
        *wrap_ptr->bypass = param->bypass;
    }

    bool isUpdateManParam = false;
    if (param->opMode == RK_AIQ_OP_MODE_MANUAL) {
        memcpy(wrap_ptr->man_param_ptr, param->man_param_ptr, param->man_param_size);
        isUpdateManParam = true;
    }

    bool isUpdateAutParam = false;
    if (param->opMode == RK_AIQ_OP_MODE_AUTO) {
        memcpy(wrap_ptr->aut_param_ptr, param->aut_param_ptr, param->aut_param_size);
        isUpdateAutParam = true;
    }

    LOGD("type:0x%x, opMode:%d,en:%d,bypass:%d,man_ptr:%p,man_size:%d,"
         "isUpdate:%d,%d,%d,%d",
         param->type, param->opMode, param->en, param->bypass, param->man_param_ptr, param->man_param_size,
         isUpdateOpMode, isUpdateEn, isUpdateBypass, isUpdateManParam);

    if (isUpdateOpMode || isUpdateEn || isUpdateBypass || isUpdateManParam) {
        mIsGlobalModulesUpdateBits |= ((uint64_t)1) << param->type;
    }

    if (isUpdateAutParam) {
        mIsAlgoParamUpdateBits |= ((uint64_t)1) << param->type;
    }

    mAlgoMutex[param->type].unlock();

    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
GlobalParamsManager::get_locked(rk_aiq_global_params_wrap_t* param)
{
    if (param->type < 0 || param->type >= RESULT_TYPE_MAX_PARAM)
        return XCAM_RETURN_ERROR_FAILED;

    rk_aiq_global_params_ptr_wrap_t* wrap_ptr = &mGlobalParams[param->type];

    param->opMode = *wrap_ptr->opMode;
    param->en = *wrap_ptr->en;
    param->bypass = *wrap_ptr->bypass;
    if (param->man_param_ptr && wrap_ptr->man_param_ptr)
        memcpy(param->man_param_ptr, wrap_ptr->man_param_ptr, param->man_param_size);

    LOGD("type:%d, opMode:%d,en:%d,bypass:%d,man_ptr:%p,man_size:%d",
         param->type, param->opMode, param->en, param->bypass, wrap_ptr->man_param_ptr, param->man_param_size);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
GlobalParamsManager::get(rk_aiq_global_params_wrap_t* param)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ENTER_ANALYZER_FUNCTION();

    lockAlgoParam(param->type);
    ret = get_locked(param);
    unlockAlgoParam(param->type);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

bool
GlobalParamsManager::isManual_locked(int type)
{
    if (*mGlobalParams[type].opMode != RK_AIQ_OP_MODE_MANUAL)
        return false;

    return true;
}

bool
GlobalParamsManager::isManual(int type)
{
    bool ret = true;
    ENTER_ANALYZER_FUNCTION();

    lockAlgoParam(type);
    ret = isManual_locked(type);
    unlockAlgoParam(type);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
GlobalParamsManager::getAndClearPending(rk_aiq_global_params_wrap_t* wrap)
{
    ENTER_ANALYZER_FUNCTION();

    if (wrap->type < 0 || wrap->type >= RESULT_TYPE_MAX_PARAM || mIsHold)
        return XCAM_RETURN_BYPASS;

    mAlgoMutex[wrap->type].lock();

    bool isManualMode = isManual_locked(wrap->type);
    bool isNeedUpdate = mIsGlobalModulesUpdateBits & (((uint64_t)1) << wrap->type);

    LOGD("type:%d, isMan:%d, isNeedUpdate:%d", wrap->type, isManualMode, isNeedUpdate);
    if (!isManualMode || !isNeedUpdate) {
        mAlgoMutex[wrap->type].unlock();
        return XCAM_RETURN_BYPASS;
    } else {
        mIsGlobalModulesUpdateBits &= ~(((uint64_t)1) << wrap->type);
        XCamReturn ret = get_locked(wrap);
        mAlgoMutex[wrap->type].unlock();
        return ret;
    }

    mAlgoMutex[wrap->type].unlock();
    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_BYPASS;
}

SmartPtr<cam3aResult>
GlobalParamsManager::getAndClearPending(int type)
{
    ENTER_ANALYZER_FUNCTION();
    if (type < 0 || type >= RESULT_TYPE_MAX_PARAM || mIsHold)
        return NULL;

    mAlgoMutex[type].lock();

    bool isManualMode = isManual_locked(type);
    bool isNeedUpdate = mIsGlobalModulesUpdateBits & (((uint64_t)1) << type);

    if (!isManualMode || !isNeedUpdate) {
        mAlgoMutex[type].unlock();
        return NULL;
    } else {
        mIsGlobalModulesUpdateBits &= ~(((uint64_t)1) << type);
        mAlgoMutex[type].unlock();
        return mFullManualParamsProxyArray[type];
    }

    mAlgoMutex[type].unlock();
    EXIT_ANALYZER_FUNCTION();
    return NULL;
}

void
GlobalParamsManager::hold(bool hold)
{
    ENTER_ANALYZER_FUNCTION();

    SmartLock lock(mMutex);
    mIsHold = hold;

    EXIT_ANALYZER_FUNCTION();
}

void
GlobalParamsManager::lockAlgoParam(int type)
{
    ENTER_ANALYZER_FUNCTION();
    if (type < 0 || type >= RESULT_TYPE_MAX_PARAM)
        return;

    mAlgoMutex[type].lock();

    EXIT_ANALYZER_FUNCTION();
}

void
GlobalParamsManager::unlockAlgoParam(int type)
{
    ENTER_ANALYZER_FUNCTION();
    if (type < 0 || type >= RESULT_TYPE_MAX_PARAM)
        return;

    mAlgoMutex[type].unlock();

    EXIT_ANALYZER_FUNCTION();
}

bool
GlobalParamsManager::getAndClearAlgoParamUpdateFlagLocked(int type)
{
    ENTER_ANALYZER_FUNCTION();
    if (type < 0 || type >= RESULT_TYPE_MAX_PARAM)
        return false;

    bool isNeedUpdate = mIsAlgoParamUpdateBits & (((uint64_t)1) << type);
    mIsAlgoParamUpdateBits &= ~(((uint64_t)1) << type);

    EXIT_ANALYZER_FUNCTION();
    return isNeedUpdate;
}

bool
GlobalParamsManager::checkAlgoEnableBypass(int type, bool en, bool bypass)
{
    if (type == RESULT_TYPE_TNR_PARAM) {
        if (*mGlobalParams[type].en != en) {
            // tnr can't open/close in runtime
            return false;
        }
    }

    if (type == RESULT_TYPE_RAWNR_PARAM) {
        if ((en == false) && mGlobalParams[RESULT_TYPE_TNR_PARAM].en) {
            // can't disable 2dnr while 3dnr enabled
            return false;
        }
    }
    return true;
}

} //namespace RkCam
