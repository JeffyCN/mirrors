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
#include "RkAiqManager.h"

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
        mIsGlobalModulesUpdateBits |= ((uint64_t)1) << RESULT_TYPE_DEBAYER_PARAM;
        if (dm_calib->opMode == RK_AIQ_OP_MODE_INVALID) {
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
        mIsGlobalModulesUpdateBits |= ((uint64_t)1) << RESULT_TYPE_TNR_PARAM;
        if (btnr_calib->opMode == RK_AIQ_OP_MODE_INVALID) {
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
        mIsGlobalModulesUpdateBits |= ((uint64_t)1) << RESULT_TYPE_YNR_PARAM;
        if (ynr_calib->opMode == RK_AIQ_OP_MODE_INVALID) {
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
        mIsGlobalModulesUpdateBits |= ((uint64_t)1) << RESULT_TYPE_UVNR_PARAM;
        if (cnr_calib->opMode == RK_AIQ_OP_MODE_INVALID) {
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
        mIsGlobalModulesUpdateBits |= ((uint64_t)1) << RESULT_TYPE_AGAMMA_PARAM;
        if (gamma_calib->opMode == RK_AIQ_OP_MODE_INVALID) {
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
        mIsGlobalModulesUpdateBits |= ((uint64_t)1) << RESULT_TYPE_SHARPEN_PARAM;
        if (sharp_calib->opMode == RK_AIQ_OP_MODE_INVALID) {
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
        mIsGlobalModulesUpdateBits |= ((uint64_t)1) << RESULT_TYPE_DRC_PARAM;
        if (drc_calib->opMode == RK_AIQ_OP_MODE_INVALID) {
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
        mIsGlobalModulesUpdateBits |= ((uint64_t)1) << RESULT_TYPE_DRC_PARAM;
        if (trans_calib->opMode == RK_AIQ_OP_MODE_INVALID) {
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
        mIsGlobalModulesUpdateBits |= ((uint64_t)1) << RESULT_TYPE_DEHAZE_PARAM;
        if (dehaze_calib->opMode == RK_AIQ_OP_MODE_INVALID) {
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
        mIsGlobalModulesUpdateBits |= ((uint64_t)1) << RESULT_TYPE_BLC_PARAM;
        if (blc_calib->opMode == RK_AIQ_OP_MODE_INVALID) {
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
        mIsGlobalModulesUpdateBits |= ((uint64_t)1) << RESULT_TYPE_DPCC_PARAM;
        if (dpc_calib->opMode == RK_AIQ_OP_MODE_INVALID) {
            dpc_calib->opMode = RK_AIQ_OP_MODE_MANUAL;
        }
        LOGK("Module DPC: opMode:%d,en:%d,bypass:%d,man_ptr:%p",
             *wrap_ptr->opMode, *wrap_ptr->en, *wrap_ptr->bypass, wrap_ptr->man_param_ptr);
    } else {
        LOGE("no dpc calib !");
    }

    wrap_ptr = &mGlobalParams[RESULT_TYPE_GIC_PARAM];
    gic_api_attrib_t* gic_calib = (gic_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(
                (void*)(mCalibDb), gic));
    if (gic_calib) {
        wrap_ptr->opMode = &gic_calib->opMode;
        wrap_ptr->en = &gic_calib->en;
        wrap_ptr->bypass = &gic_calib->bypass;
        wrap_ptr->man_param_ptr = &gic_calib->stMan;
        wrap_ptr->aut_param_ptr = &gic_calib->stAuto;
        mIsGlobalModulesUpdateBits |= ((uint64_t)1) << RESULT_TYPE_GIC_PARAM;
        if (gic_calib->opMode == RK_AIQ_OP_MODE_INVALID) {
            gic_calib->opMode = RK_AIQ_OP_MODE_MANUAL;
        }
        LOGK("Module GIC: opMode:%d,en:%d,bypass:%d,man_ptr:%p",
             *wrap_ptr->opMode, *wrap_ptr->en, *wrap_ptr->bypass, wrap_ptr->man_param_ptr);
    }
    else {
        LOGE("no gic calib !");
    }

    wrap_ptr = &mGlobalParams[RESULT_TYPE_MOTION_PARAM];
    yme_api_attrib_t* yme_calib = (yme_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(
                (void*)(mCalibDb), yme));
    if (yme_calib) {
        wrap_ptr->opMode = &yme_calib->opMode;
        wrap_ptr->en = &yme_calib->en;
        wrap_ptr->bypass = &yme_calib->bypass;
        wrap_ptr->man_param_ptr = &yme_calib->stMan;
        wrap_ptr->aut_param_ptr = &yme_calib->stAuto;
        mIsGlobalModulesUpdateBits |= ((uint64_t)1) << RESULT_TYPE_MOTION_PARAM;
        if (yme_calib->opMode == RK_AIQ_OP_MODE_INVALID) {
            yme_calib->opMode = RK_AIQ_OP_MODE_MANUAL;
        }
        LOGK("Module YME: opMode:%d,en:%d,bypass:%d,man_ptr:%p",
             *wrap_ptr->opMode, *wrap_ptr->en, *wrap_ptr->bypass, wrap_ptr->man_param_ptr);
    } else {
        LOGE("no yme calib !");
    }

    wrap_ptr = &mGlobalParams[RESULT_TYPE_CAC_PARAM];
    cac_api_attrib_t* cac_calib = (cac_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(
                (void*)(mCalibDb), cac));
    if (cac_calib) {
        wrap_ptr->opMode = &cac_calib->opMode;
        wrap_ptr->en = &cac_calib->en;
        wrap_ptr->bypass = &cac_calib->bypass;
        wrap_ptr->man_param_ptr = &cac_calib->stMan;
        wrap_ptr->aut_param_ptr = &cac_calib->stAuto;
        mIsGlobalModulesUpdateBits |= ((uint64_t)1) << RESULT_TYPE_CAC_PARAM;
        if (cac_calib->opMode == RK_AIQ_OP_MODE_INVALID) {
            cac_calib->opMode = RK_AIQ_OP_MODE_MANUAL;
        }
        LOGK("Module CAC: opMode:%d,en:%d,bypass:%d,man_ptr:%p",
             *wrap_ptr->opMode, *wrap_ptr->en, *wrap_ptr->bypass, wrap_ptr->man_param_ptr);
    } else {
        LOGE("no cac calib !");
    }

    wrap_ptr = &mGlobalParams[RESULT_TYPE_HISTEQ_PARAM];
    histeq_api_attrib_t* histeq_calib = (histeq_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(
                (void*)(mCalibDb), histeq));
    if (histeq_calib) {
        wrap_ptr->opMode = &histeq_calib->opMode;
        wrap_ptr->en = &histeq_calib->en;
        wrap_ptr->bypass = &histeq_calib->bypass;
        wrap_ptr->man_param_ptr = &histeq_calib->stMan;
        wrap_ptr->aut_param_ptr = &histeq_calib->stAuto;
        mIsGlobalModulesUpdateBits |= ((uint64_t)1) << RESULT_TYPE_HISTEQ_PARAM;
        if (histeq_calib->opMode == RK_AIQ_OP_MODE_INVALID) {
            histeq_calib->opMode = RK_AIQ_OP_MODE_MANUAL;
        }
        LOGK("Module HISTEQ: opMode:%d,en:%d,bypass:%d,man_ptr:%p",
             *wrap_ptr->opMode, *wrap_ptr->en, *wrap_ptr->bypass, wrap_ptr->man_param_ptr);
    } else {
        LOGE("no histeq calib !");
    }
    wrap_ptr = &mGlobalParams[RESULT_TYPE_CSM_PARAM];
    csm_api_attrib_t* csm_calib = (csm_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(
                (void*)(mCalibDb), csm));
    if (csm_calib) {
        wrap_ptr->opMode = &csm_calib->opMode;
        wrap_ptr->en = &csm_calib->en;
        wrap_ptr->bypass = &csm_calib->bypass;
        wrap_ptr->man_param_ptr = &csm_calib->stMan;
        mIsGlobalModulesUpdateBits |= ((uint64_t)1) << RESULT_TYPE_CSM_PARAM;
        if (csm_calib->opMode == RK_AIQ_OP_MODE_INVALID) {
            csm_calib->opMode = RK_AIQ_OP_MODE_MANUAL;
        }
        LOGK("Module csm: opMode:%d,en:%d,bypass:%d,man_ptr:%p",
             *wrap_ptr->opMode, *wrap_ptr->en, *wrap_ptr->bypass, wrap_ptr->man_param_ptr);
    } else {
        LOGE("no csm calib !");
    }

    wrap_ptr = &mGlobalParams[RESULT_TYPE_MERGE_PARAM];
    mge_api_attrib_t* mge_calib = (mge_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(
                (void*)(mCalibDb), mge));
    if (mge_calib) {
        wrap_ptr->opMode = &mge_calib->opMode;
        wrap_ptr->en = &mge_calib->en;
        wrap_ptr->bypass = &mge_calib->bypass;
        wrap_ptr->man_param_ptr = &mge_calib->stMan;
        wrap_ptr->aut_param_ptr = &mge_calib->stAuto;
        mIsGlobalModulesUpdateBits |= ((uint64_t)1) << RESULT_TYPE_MERGE_PARAM;
        if (mge_calib->opMode == RK_AIQ_OP_MODE_INVALID) {
            mge_calib->opMode = RK_AIQ_OP_MODE_MANUAL;
        }
        LOGK("Module MERGE: opMode:%d,en:%d,bypass:%d,man_ptr:%p",
             *wrap_ptr->opMode, *wrap_ptr->en, *wrap_ptr->bypass, wrap_ptr->man_param_ptr);
    } else {
        LOGE("no merge calib !");
    }

    wrap_ptr = &mGlobalParams[RESULT_TYPE_LSC_PARAM];
    lsc_calib_attrib_t* lsc_calib = (lsc_calib_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(
                (void*)(mCalibDb), lsc));
    if (lsc_calib) {
        lsc_api_attrib_t* lsc_attrib = &lsc_calib->tunning;
        wrap_ptr->opMode = &lsc_attrib ->opMode;
        wrap_ptr->en = &lsc_attrib ->en;
        wrap_ptr->bypass = &lsc_attrib ->bypass;
        wrap_ptr->man_param_ptr = &lsc_attrib->stMan;
        wrap_ptr->aut_param_ptr = &lsc_attrib->stAuto;
        mIsGlobalModulesUpdateBits |= ((uint64_t)1) << RESULT_TYPE_LSC_PARAM;
        if (lsc_attrib->opMode == RK_AIQ_OP_MODE_INVALID) {
            lsc_attrib->opMode = RK_AIQ_OP_MODE_MANUAL;
        }
        LOGK("Module LSC: opMode:%d,en:%d,bypass:%d,man_ptr:%p",
             *wrap_ptr->opMode, *wrap_ptr->en, *wrap_ptr->bypass, wrap_ptr->man_param_ptr);
    }
    else {
        LOGE("no lsc calib !");
    }

    wrap_ptr = &mGlobalParams[RESULT_TYPE_RGBIR_PARAM];
    rgbir_api_attrib_t* rgbir_calib = (rgbir_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(
                (void*)(mCalibDb), rgbir));
    if (rgbir_calib) {
        wrap_ptr->opMode = &rgbir_calib->opMode;
        wrap_ptr->en = &rgbir_calib->en;
        wrap_ptr->bypass = &rgbir_calib->bypass;
        wrap_ptr->man_param_ptr = &rgbir_calib->stMan;
        wrap_ptr->aut_param_ptr = &rgbir_calib->stAuto;
        mIsGlobalModulesUpdateBits |= ((uint64_t)1) << RESULT_TYPE_RGBIR_PARAM;
        if (rgbir_calib->opMode == RK_AIQ_OP_MODE_INVALID) {
            rgbir_calib->opMode = RK_AIQ_OP_MODE_MANUAL;
        }
        LOGK("Module rgbir: opMode:%d,en:%d,bypass:%d,man_ptr:%p",
             *wrap_ptr->opMode, *wrap_ptr->en, *wrap_ptr->bypass, wrap_ptr->man_param_ptr);
    } else {
        LOGE("no rgbir calib !");
    }

    wrap_ptr = &mGlobalParams[RESULT_TYPE_CGC_PARAM];
    cgc_api_attrib_t* cgc_calib = (cgc_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(
                (void*)(mCalibDb), cgc));
    if (cgc_calib) {
        wrap_ptr->opMode = &cgc_calib->opMode;
        wrap_ptr->en = &cgc_calib->en;
        wrap_ptr->bypass = &cgc_calib->bypass;
        wrap_ptr->man_param_ptr = &cgc_calib->stMan;
        wrap_ptr->aut_param_ptr = &cgc_calib->stAuto;
        mIsGlobalModulesUpdateBits |= ((uint64_t)1) << RESULT_TYPE_CGC_PARAM;
        if (cgc_calib->opMode == RK_AIQ_OP_MODE_INVALID) {
            cgc_calib->opMode = RK_AIQ_OP_MODE_MANUAL;
        }
        LOGK("Module cgc: opMode:%d,en:%d,bypass:%d,man_ptr:%p",
             *wrap_ptr->opMode, *wrap_ptr->en, *wrap_ptr->bypass, wrap_ptr->man_param_ptr);
    } else {
        LOGE("no cgc calib !");
    }

    wrap_ptr = &mGlobalParams[RESULT_TYPE_CP_PARAM];
    cp_api_attrib_t* cp_calib = (cp_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(
                (void*)(mCalibDb), cp));
    if (cp_calib) {
        wrap_ptr->opMode = &cp_calib->opMode;
        wrap_ptr->en = &cp_calib->en;
        wrap_ptr->bypass = &cp_calib->bypass;
        wrap_ptr->man_param_ptr = &cp_calib->stMan;
        wrap_ptr->aut_param_ptr = &cp_calib->stAuto;
        mIsGlobalModulesUpdateBits |= ((uint64_t)1) << RESULT_TYPE_CP_PARAM;
        if (cp_calib->opMode == RK_AIQ_OP_MODE_INVALID) {
            cp_calib->opMode = RK_AIQ_OP_MODE_MANUAL;
        }
        LOGK("Module cp: opMode:%d,en:%d,bypass:%d,man_ptr:%p",
             *wrap_ptr->opMode, *wrap_ptr->en, *wrap_ptr->bypass, wrap_ptr->man_param_ptr);
    } else {
        LOGE("no cp calib !");
    }

    wrap_ptr = &mGlobalParams[RESULT_TYPE_IE_PARAM];
    ie_api_attrib_t* ie_calib = (ie_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(
                (void*)(mCalibDb), ie));
    if (ie_calib) {
        wrap_ptr->opMode = &ie_calib->opMode;
        wrap_ptr->en = &ie_calib->en;
        wrap_ptr->bypass = &ie_calib->bypass;
        wrap_ptr->man_param_ptr = &ie_calib->stMan;
        wrap_ptr->aut_param_ptr = &ie_calib->stAuto;
        mIsGlobalModulesUpdateBits |= ((uint64_t)1) << RESULT_TYPE_IE_PARAM;
        if (ie_calib->opMode == RK_AIQ_OP_MODE_INVALID) {
            ie_calib->opMode = RK_AIQ_OP_MODE_MANUAL;
        }
        LOGK("Module ie: opMode:%d,en:%d,bypass:%d,man_ptr:%p",
             *wrap_ptr->opMode, *wrap_ptr->en, *wrap_ptr->bypass, wrap_ptr->man_param_ptr);
    } else {
        LOGE("no ie calib !");
    }

    wrap_ptr = &mGlobalParams[RESULT_TYPE_GAIN_PARAM];
    gain_api_attrib_t* gain_calib = (gain_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(
                (void*)(mCalibDb), gain));
    if (gain_calib) {
        wrap_ptr->opMode = &gain_calib->opMode;
        wrap_ptr->en = &gain_calib->en;
        wrap_ptr->bypass = &gain_calib->bypass;
        wrap_ptr->man_param_ptr = &gain_calib->stMan;
        wrap_ptr->aut_param_ptr = &gain_calib->stAuto;
        mIsGlobalModulesUpdateBits |= ((uint64_t)1) << RESULT_TYPE_GAIN_PARAM;
        if (gain_calib->opMode == RK_AIQ_OP_MODE_INVALID) {
            gain_calib->opMode = RK_AIQ_OP_MODE_MANUAL;
        }
        LOGK("Module gain: opMode:%d,en:%d,bypass:%d,man_ptr:%p",
             *wrap_ptr->opMode, *wrap_ptr->en, *wrap_ptr->bypass, wrap_ptr->man_param_ptr);
    } else {
        LOGE("no gain calib !");
    }

    wrap_ptr = &mGlobalParams[RESULT_TYPE_LUT3D_PARAM];
    lut3d_calib_attrib_t* lut3d_calib = (lut3d_calib_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(
                (void*)(mCalibDb), lut3d));
    if (lut3d_calib) {
        lut3d_api_attrib_t* lut3d_attrib = &lut3d_calib->tunning;
        wrap_ptr->opMode = &lut3d_attrib->opMode;
        wrap_ptr->en = &lut3d_attrib->en;
        wrap_ptr->bypass = &lut3d_attrib->bypass;
        wrap_ptr->man_param_ptr = &lut3d_attrib->stMan;
        wrap_ptr->aut_param_ptr = &lut3d_attrib->stAuto;
        mIsGlobalModulesUpdateBits |= ((uint64_t)1) << RESULT_TYPE_LUT3D_PARAM;
        if (lut3d_attrib->opMode == RK_AIQ_OP_MODE_INVALID) {
            lut3d_attrib->opMode = RK_AIQ_OP_MODE_MANUAL;
        }
        LOGK("Module 3DLUT: opMode:%d,en:%d,bypass:%d,man_ptr:%p",
             *wrap_ptr->opMode, *wrap_ptr->en, *wrap_ptr->bypass, wrap_ptr->man_param_ptr);
    } else {
        LOGE("no lut3d calib !");
    }

    wrap_ptr = &mGlobalParams[RESULT_TYPE_CCM_PARAM];
    ccm_calib_attrib_t* ccm_calib = (ccm_calib_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(
                (void*)(mCalibDb), ccm));
    if (ccm_calib) {
        ccm_api_attrib_t* ccm_attrib = &ccm_calib->tunning;
        wrap_ptr->opMode = &ccm_attrib ->opMode;
        wrap_ptr->en = &ccm_attrib ->en;
        wrap_ptr->bypass = &ccm_attrib ->bypass;
        wrap_ptr->man_param_ptr = &ccm_attrib->stMan;
        wrap_ptr->aut_param_ptr = &ccm_attrib->stAuto;
        mIsGlobalModulesUpdateBits |= ((uint64_t)1) << RESULT_TYPE_CCM_PARAM;
        if (ccm_attrib->opMode == RK_AIQ_OP_MODE_INVALID) {
            ccm_attrib->opMode = RK_AIQ_OP_MODE_MANUAL;
        }
        LOGK("Module CCM: opMode:%d,en:%d,bypass:%d,man_ptr:%p",
             *wrap_ptr->opMode, *wrap_ptr->en, *wrap_ptr->bypass, wrap_ptr->man_param_ptr);
    } else {
        LOGE("no ccm calib !");
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
        checkAlgoEnableInit();
    }
    EXIT_ANALYZER_FUNCTION();
}

XCamReturn
GlobalParamsManager::switchCalibDb(CamCalibDbV2Context_t* calibDb)
{
    ENTER_ANALYZER_FUNCTION();

    SmartLock lock(mMutex);
    int state = rkAiqManager->getAiqState();

    if (state != AIQ_STATE_INITED && state != AIQ_STATE_STOPED)
        if (switchCalibDbCheck(calibDb) == XCAM_RETURN_ERROR_FAILED) {
            LOGE("switchCalibDbCheck fail");
            return XCAM_RETURN_ERROR_FAILED;
        }
    mCalibDb = calibDb;
    init_withCalib();
    checkAlgoEnableInit();

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
GlobalParamsManager::switchCalibDbCheck(CamCalibDbV2Context_t* calibDb) {

#if USE_NEWSTRUCT
    btnr_api_attrib_t* btnr_calib = (btnr_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(
        (void*)(calibDb), bayertnr));
    if (*mGlobalParams[RESULT_TYPE_TNR_PARAM].en != btnr_calib->en) {
        LOGE("tnr can't open/close in runtime!");
        return XCAM_RETURN_ERROR_FAILED;
    }

    yme_api_attrib_t* yme_calib = (yme_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(
                (void*)(mCalibDb), yme));
    if (*mGlobalParams[RESULT_TYPE_MOTION_PARAM].en != yme_calib->en) {
        LOGE("yuvme can't open/close in runtime!");
        return XCAM_RETURN_ERROR_FAILED;
    }

    cnr_api_attrib_t* cnr_calib = (cnr_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(
        (void*)(calibDb), cnr));
    ynr_api_attrib_t* ynr_calib = (ynr_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(
        (void*)(calibDb), ynr));
    sharp_api_attrib_t* sharp_calib = (sharp_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(
        (void*)(calibDb), sharp));
    if (!(cnr_calib->en == ynr_calib->en && ynr_calib->en == sharp_calib->en)) {
        LOGW("ynr, cnr and sharp should be on or off in the same time");
        return XCAM_RETURN_BYPASS;
    }

    drc_api_attrib_t* drc_calib = (drc_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(
        (void*)(mCalibDb), drc));
    if (rkAiqManager->getWorkingMode() != RK_AIQ_WORKING_MODE_NORMAL && drc_calib->en == 0) {
        LOGE("drc must be on when open HDR mode!");
        return XCAM_RETURN_ERROR_FAILED;
    }

#endif
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
GlobalParamsManager::set(rk_aiq_global_params_wrap_t* param)
{
    ENTER_ANALYZER_FUNCTION();

    if (param->type < 0 || param->type >= RESULT_TYPE_MAX_PARAM)
        return XCAM_RETURN_ERROR_OUTOFRANGE;

    XCamReturn ret = checkAlgoEnableBypass(param->type, param->en, param->bypass);
    if (ret == XCAM_RETURN_ERROR_FAILED) {
        LOGE("checkAlgoEnableBypass fail");
        return ret;
    }

    if (!checkAlgoParams(param)) {
        LOGE("checkAlgoParams fail");
        return XCAM_RETURN_ERROR_FAILED;
    }

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

    if (isUpdateOpMode || isUpdateEn || isUpdateBypass || isUpdateManParam || isUpdateAutParam) {
        mIsGlobalModulesUpdateBits |= ((uint64_t)1) << param->type;
    }

    mAlgoMutex[param->type].unlock();

    EXIT_ANALYZER_FUNCTION();

    return ret;
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

XCamReturn
GlobalParamsManager::get_ModuleEn(rk_aiq_module_list_t* mod) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ENTER_ANALYZER_FUNCTION();

    for (int i = 0;i < RESULT_TYPE_MAX_PARAM;i++) {
        lockAlgoParam(i);
        mod->module_ctl[i].type = (camAlgoResultType)i;
        if (mGlobalParams[i].en != NULL) {
            mod->module_ctl[i].en = *mGlobalParams[i].en;
            mod->module_ctl[i].bypass = *mGlobalParams[i].bypass;
            mod->module_ctl[i].opMode = *mGlobalParams[i].opMode;
        }
        else {
            mod->module_ctl[i].en = 0;
            mod->module_ctl[i].bypass = 0;
            mod->module_ctl[i].opMode = RK_AIQ_OP_MODE_INVALID;
        }
        unlockAlgoParam(i);
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn
GlobalParamsManager::set_ModuleEn(rk_aiq_module_list_t* mod) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ENTER_ANALYZER_FUNCTION();

    for (int i = 0;i < RESULT_TYPE_MAX_PARAM;i++) {
        int cur_type = mod->module_ctl[i].type;
        if (mGlobalParams[cur_type].en != NULL) {
            if (checkAlgoEnableBypass(cur_type, mod->module_ctl[i].en, mod->module_ctl[i].bypass) == XCAM_RETURN_ERROR_FAILED) {
                LOGE("checkAlgoEnableBypass fail");
                continue;
            }
            lockAlgoParam(cur_type);
            if (*mGlobalParams[cur_type].en != mod->module_ctl[i].en ||
                *mGlobalParams[cur_type].bypass != mod->module_ctl[i].bypass ||
                *mGlobalParams[cur_type].opMode != mod->module_ctl[i].opMode) {
                *mGlobalParams[cur_type].en = mod->module_ctl[i].en;
                *mGlobalParams[cur_type].bypass = mod->module_ctl[i].bypass;
                *mGlobalParams[cur_type].opMode = mod->module_ctl[i].opMode;
                mIsGlobalModulesUpdateBits |= ((uint64_t)1) << cur_type;
            }
            unlockAlgoParam(cur_type);
        }
    }

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

    bool isNeedUpdate = mIsGlobalModulesUpdateBits & (((uint64_t)1) << type);
    mIsGlobalModulesUpdateBits &= ~(((uint64_t)1) << type);

    EXIT_ANALYZER_FUNCTION();
    return isNeedUpdate;
}

void
GlobalParamsManager::checkAlgoEnableInit() {
    int type;
#if USE_NEWSTRUCT
    bool cnr_en = *mGlobalParams[RESULT_TYPE_UVNR_PARAM].en;
    bool ynr_en = *mGlobalParams[RESULT_TYPE_YNR_PARAM].en;
    bool sharp_en = *mGlobalParams[RESULT_TYPE_SHARPEN_PARAM].en;
    if (!(ynr_en == cnr_en && cnr_en == sharp_en)) {
        LOGW("ynr, cnr and sharp should be on or off in the same time");
    }

    if (*mGlobalParams[RESULT_TYPE_TNR_PARAM].en != *mGlobalParams[RESULT_TYPE_MOTION_PARAM].en) {
        *mGlobalParams[RESULT_TYPE_MOTION_PARAM].en = *mGlobalParams[RESULT_TYPE_TNR_PARAM].en;
        LOGE("btnr and yuvme should be turned on or off simultaneously,please check!");
    }

    type = RESULT_TYPE_DRC_PARAM;
    checkAlgoEnableBypass(type, *mGlobalParams[type].en, *mGlobalParams[type].bypass);
    type = RESULT_TYPE_GIC_PARAM;
    checkAlgoEnableBypass(type, *mGlobalParams[type].en, *mGlobalParams[type].bypass);
    type = RESULT_TYPE_CAC_PARAM;
    checkAlgoEnableBypass(type, *mGlobalParams[type].en, *mGlobalParams[type].bypass);
#endif
}

XCamReturn
GlobalParamsManager::checkAlgoEnableBypass(int type, bool &en, bool &bypass)
{
    int state = rkAiqManager->getAiqState();
#if USE_NEWSTRUCT
    if (type == RESULT_TYPE_TNR_PARAM) {
        if (*mGlobalParams[type].en != en) {
            // tnr can't open/close in runtime
            if (state != AIQ_STATE_INITED && state != AIQ_STATE_STOPED) {
                LOGE("tnr can't open/close in runtime!");
                return XCAM_RETURN_ERROR_FAILED;
            }
        }
    }

    if (type == RESULT_TYPE_MOTION_PARAM) {
        if (*mGlobalParams[type].en != en) {
            if (state != AIQ_STATE_INITED &&state != AIQ_STATE_STOPED) {
                LOGE("btnr and yuvme should be turned on or off simultaneously,"
                    " and tnr can't open/close in runtime!");
                return XCAM_RETURN_ERROR_FAILED;
            }
        }
    }

    if (type == RESULT_TYPE_RAWNR_PARAM) {
        if ((en == false) && *mGlobalParams[RESULT_TYPE_TNR_PARAM].en) {
            // can't disable 2dnr while 3dnr enabled
            return XCAM_RETURN_ERROR_FAILED;
        }
    }

    if (type == RESULT_TYPE_UVNR_PARAM || type == RESULT_TYPE_YNR_PARAM || type == RESULT_TYPE_SHARPEN_PARAM) {
        if (*mGlobalParams[type].en != en) {
            LOGD("ynr, cnr or sharp en is changed");
            return XCAM_RETURN_BYPASS;
        }
    }

    if (type == RESULT_TYPE_DRC_PARAM) {
        if (rkAiqManager->getWorkingMode() != RK_AIQ_WORKING_MODE_NORMAL && en == 0) {
            LOGE("drc must be on when open HDR mode!");
            return XCAM_RETURN_ERROR_FAILED;
        }
    }

    if (type == RESULT_TYPE_GIC_PARAM || type == RESULT_TYPE_CAC_PARAM) {
        if (en == 0 && *mGlobalParams[RESULT_TYPE_DRC_PARAM].en) {
            LOGE("gic and cac can not be off when drc is on, open bypass");
            return XCAM_RETURN_BYPASS;
        }
    }
#endif
    return XCAM_RETURN_NO_ERROR;
}

bool
GlobalParamsManager::checkAlgoParams(rk_aiq_global_params_wrap_t* param) {
#ifdef USE_NEWSTRUCT
    int state = rkAiqManager->getAiqState();
    if (param->type == RESULT_TYPE_TNR_PARAM&&
        (state != AIQ_STATE_INITED && state != AIQ_STATE_STOPED)) {
        btnr_api_attrib_t attr;
        btnr_api_attrib_t old_attr;
        btnr_pixDomain_mode_t newmode;
        btnr_pixDomain_mode_t oldmode;
        if (param->opMode == RK_AIQ_OP_MODE_AUTO) {
            memcpy(&attr.stAuto, param->aut_param_ptr, param->aut_param_size);
            newmode = attr.stAuto.sta.hw_btnrCfg_pixDomain_mode;
        }
        else {
            memcpy(&attr.stMan, param->man_param_ptr, param->man_param_size);
            newmode = attr.stMan.sta.hw_btnrCfg_pixDomain_mode;
        }
        if (*mGlobalParams[param->type].opMode == RK_AIQ_OP_MODE_AUTO) {
            memcpy(&old_attr.stAuto, mGlobalParams[param->type].aut_param_ptr, param->aut_param_size);
            oldmode = old_attr.stAuto.sta.hw_btnrCfg_pixDomain_mode;
        }
        else{
            memcpy(&old_attr.stMan, mGlobalParams[param->type].man_param_ptr, param->man_param_size);
            oldmode = old_attr.stMan.sta.hw_btnrCfg_pixDomain_mode;
        }
        if (newmode != oldmode) {
            LOGE("Btnr is running, so config pixDomain_mode to btnr_pixLinearDomain_mode failed!");
            return false;
        }
    }

    if (param->type == RESULT_TYPE_BLC_PARAM && rkAiqManager->getWorkingMode() != RK_AIQ_WORKING_MODE_NORMAL) {
        blc_api_attrib_t attr;
        if (param->opMode == RK_AIQ_OP_MODE_AUTO) {
            memcpy(&attr.stAuto, param->aut_param_ptr, param->aut_param_size);
            for (int i = 0;i < BLC_ISO_STEP_MAX;i++) {
                if (attr.stAuto.dyn[i].obcPostTnr.sw_blcT_obcPostTnr_en) {
                    LOGE("When using HDR mode, blc_t_obcPostTnr_en must be off!");
                    return false;
                }
            }
        }
        else {
            memcpy(&attr.stMan, param->man_param_ptr, param->man_param_size);
            if (attr.stMan.dyn.obcPostTnr.sw_blcT_obcPostTnr_en) {
                LOGE("When using HDR mode, blc_t_obcPostTnr_en must be off!");
                return false;
            }
        }
    }
#endif
    return true;
}

} //namespace RkCam
