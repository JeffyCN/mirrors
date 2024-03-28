/*
* rk_aiq_accm_algo_v1.cpp

* for rockchip v2.0.0
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
/* for rockchip v2.0.0*/

#include "accm/rk_aiq_accm_algo_com.h"
#include "interpolation.h"
#include "xcam_log.h"

RKAIQ_BEGIN_DECLARE

void CCMV2PrintReg(const rk_aiq_ccm_cfg_v2_t* hw_param) {
    LOG1_ACCM(
        " CCM V2 reg values: "
        " sw_ccm_asym_adj_en %d "
        " sw_ccm_enh_adj_en %d "
        " sw_ccm_highy_adjust %d "
        " sw_ccm_en_i %d "
        " sw_ccm_coeff ([%f,%f,%f,%f,%f,%f,%f,%f,%f]-E)X128 "
        " sw_ccm_offset [%f,%f,%f] "
        " sw_ccm_coeff_y [%f,%f,%f] "
        " sw_ccm_alp_y [%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f] "
        " sw_ccm_right_bit %f "
        " sw_ccm_bound_bit %f "
        " sw_ccm_color_coef_y [%d,%d,%d] "
        " sw_ccm_color_enh_rat_max %f ",
        hw_param->asym_adj_en, hw_param->enh_adj_en, hw_param->highy_adj_en, hw_param->ccmEnable,
        hw_param->matrix[0], hw_param->matrix[1], hw_param->matrix[2], hw_param->matrix[3],
        hw_param->matrix[4], hw_param->matrix[5], hw_param->matrix[6], hw_param->matrix[7],
        hw_param->matrix[8], hw_param->offs[0], hw_param->offs[1], hw_param->offs[2],
        hw_param->rgb2y_para[0], hw_param->rgb2y_para[1], hw_param->rgb2y_para[2],
        hw_param->alp_y[0], hw_param->alp_y[1], hw_param->alp_y[2], hw_param->alp_y[3],
        hw_param->alp_y[4], hw_param->alp_y[5], hw_param->alp_y[6], hw_param->alp_y[7],
        hw_param->alp_y[8], hw_param->alp_y[9], hw_param->alp_y[10], hw_param->alp_y[11],
        hw_param->alp_y[12], hw_param->alp_y[13], hw_param->alp_y[14], hw_param->alp_y[15],
        hw_param->alp_y[16], hw_param->alp_y[17], hw_param->right_bit, hw_param->bound_bit,
        hw_param->enh_rgb2y_para[0], hw_param->enh_rgb2y_para[1], hw_param->enh_rgb2y_para[2],
        hw_param->enh_rat_max);
#if RKAIQ_HAVE_CCM_V3
    LOG1_ACCM(
        " sw_ccm_sat_decay_en %d "
        " sw_ccm_hf_low %d "
        " sw_ccm_hf_up %d "
        " sw_ccm_hf_scale %d "
        " sw_ccm_hf_factor0 [%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d] ",
        hw_param->sat_decay_en, hw_param->hf_low, hw_param->hf_up, hw_param->hf_scale,
        hw_param->hf_factor[0], hw_param->hf_factor[1], hw_param->hf_factor[2], hw_param->hf_factor[3],
        hw_param->hf_factor[4], hw_param->hf_factor[5], hw_param->hf_factor[6], hw_param->hf_factor[7],
        hw_param->hf_factor[8], hw_param->hf_factor[9], hw_param->hf_factor[10], hw_param->hf_factor[11],
        hw_param->hf_factor[12], hw_param->hf_factor[13], hw_param->hf_factor[14], hw_param->hf_factor[15],
        hw_param->hf_factor[16]);
#endif
}

void CCMV2PrintDBG(const accm_context_t* accm_context) {
#if RKAIQ_HAVE_CCM_V2
    const CalibDbV2_Ccm_Para_V32_t* pCcm = accm_context->ccm_v2;
    const rk_aiq_ccm_v2_attrib_t* mCurAtt = &accm_context->mCurAttV2;
#elif RKAIQ_HAVE_CCM_V3
    const CalibDbV2_Ccm_Para_V39_t* pCcm = accm_context->ccm_v3;
    const rk_aiq_ccm_v3_attrib_t* mCurAtt = &accm_context->mCurAttV3;
#endif
    const float* pMatrixUndamped         = accm_context->accmRest.undampedCcmMatrix;
    const float* pOffsetUndamped         = accm_context->accmRest.undampedCcOffset;
    const float* pMatrixDamped           = accm_context->ccmHwConf_v2.matrix;
    const float* pOffsetDamped           = accm_context->ccmHwConf_v2.offs;
    float dampcoef = 0;
    dampcoef = (pCcm->TuningPara.damp_enable && accm_context->count > 1) ? accm_context->accmSwInfo.awbIIRDampCoef : 0;

    LOG1_ACCM(
        "Illu Probability Estimation Enable: %d "
        "color_inhibition sensorGain: %f,%f,%f,%f "
        "color_inhibition level: %f,%f,%f,%f "
        "color_saturation sensorGain: %f,%f,%f,%f "
        "color_saturation level: %f,%f,%f,%f "
        "damp factor: %f "
        " undampedCcmMatrix: %f,%f,%f,%f,%f,%f,%f,%f,%f "
        " undampedCcOffset: %f,%f,%f "
        " dampedCcmMatrix: %f,%f,%f,%f,%f,%f,%f,%f,%f "
        " dampedCcOffset:%f,%f,%f ",
        pCcm->TuningPara.illu_estim.interp_enable,
        mCurAtt->stAuto.color_inhibition.sensorGain[0],
        mCurAtt->stAuto.color_inhibition.sensorGain[1],
        mCurAtt->stAuto.color_inhibition.sensorGain[2],
        mCurAtt->stAuto.color_inhibition.sensorGain[3],
        mCurAtt->stAuto.color_inhibition.level[0],
        mCurAtt->stAuto.color_inhibition.level[1],
        mCurAtt->stAuto.color_inhibition.level[2],
        mCurAtt->stAuto.color_inhibition.level[3],
        mCurAtt->stAuto.color_saturation.sensorGain[0],
        mCurAtt->stAuto.color_saturation.sensorGain[1],
        mCurAtt->stAuto.color_saturation.sensorGain[2],
        mCurAtt->stAuto.color_saturation.sensorGain[3],
        mCurAtt->stAuto.color_saturation.level[0],
        mCurAtt->stAuto.color_saturation.level[1],
        mCurAtt->stAuto.color_saturation.level[2],
        mCurAtt->stAuto.color_saturation.level[3], dampcoef,
        pMatrixUndamped[0], pMatrixUndamped[1], pMatrixUndamped[2], pMatrixUndamped[3],
        pMatrixUndamped[4], pMatrixUndamped[5], pMatrixUndamped[6], pMatrixUndamped[7],
        pMatrixUndamped[8], pOffsetUndamped[0], pOffsetUndamped[1], pOffsetUndamped[2],
        pMatrixDamped[0], pMatrixDamped[1], pMatrixDamped[2], pMatrixDamped[3], pMatrixDamped[4],
        pMatrixDamped[5], pMatrixDamped[6], pMatrixDamped[7], pMatrixDamped[8], pOffsetDamped[0],
        pOffsetDamped[1], pOffsetDamped[2]);
}
#if RKAIQ_HAVE_CCM_V2
XCamReturn ConfigHwbyCalib(const CalibDbV2_Ccm_Para_V32_t* calib_ccm,
#elif RKAIQ_HAVE_CCM_V3
XCamReturn ConfigHwbyCalib(const CalibDbV2_Ccm_Para_V39_t* calib_ccm,
#endif
                           rk_aiq_ccm_cfg_v2_t* ccmHwConf) {
    LOG1_ACCM("%s: (enter)  \n", __FUNCTION__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ccmHwConf->asym_adj_en = calib_ccm->lumaCCM.asym_enable;

    if (calib_ccm->lumaCCM.asym_enable) {
        ccmHwConf->highy_adj_en = true;
        ccmHwConf->bound_bit    = calib_ccm->lumaCCM.y_alp_asym.bound_pos_bit;
        ccmHwConf->right_bit    = calib_ccm->lumaCCM.y_alp_asym.right_pos_bit;
    } else {
        ccmHwConf->highy_adj_en = calib_ccm->lumaCCM.y_alp_sym.highy_adj_en;
        ccmHwConf->bound_bit    = calib_ccm->lumaCCM.y_alp_sym.bound_pos_bit;
        ccmHwConf->right_bit    = ccmHwConf->bound_bit;
        ccmHwConf->alp_y[CCM_CURVE_DOT_NUM] = 1024;
    }

    memcpy(ccmHwConf->rgb2y_para, calib_ccm->lumaCCM.rgb2y_para,
           sizeof(calib_ccm->lumaCCM.rgb2y_para));

    memcpy(ccmHwConf->enh_rgb2y_para, calib_ccm->enhCCM.enh_rgb2y_para,
           sizeof(calib_ccm->enhCCM.enh_rgb2y_para));

    LOG1_ACCM("%s: (exit)\n", __FUNCTION__);
    return (ret);
}

XCamReturn AccmAutoConfig(accm_handle_t hAccm) {
    LOG1_ACCM("%s: (enter)\n", __FUNCTION__);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#if RKAIQ_HAVE_CCM_V2
    const CalibDbV2_Ccm_Para_V32_t* calib = hAccm->ccm_v2;
#elif RKAIQ_HAVE_CCM_V3
    const CalibDbV2_Ccm_Para_V39_t* calib = hAccm->ccm_v3;
#else
    const CalibDbV2_Ccm_Para_V32_t* calib = NULL;
#endif

    if (hAccm == NULL || calib == NULL) {
        return XCAM_RETURN_ERROR_PARAM;
    }

    const CalibDbV2_Ccm_Tuning_Para_t* pCcm = NULL;
    float sensorGain                     = hAccm->accmSwInfo.sensorGain;
    float fSaturation                    = 0;
    bool  updateMat                      = false;
    bool  updateYAlp                     = false;
    bool  updateEnh                      = false;
    bool  updUndampMat                   = false;

    pCcm = &calib->TuningPara;
    if (hAccm->update) {
        if (pCcm->illu_estim.interp_enable) {
            hAccm->isReCal_ = true;
            ret = interpCCMbywbgain(&pCcm->illu_estim, pCcm->aCcmCof,
                                    pCcm->aCcmCof_len, hAccm, fSaturation);
            RETURN_RESULT_IF_DIFFERENT(ret, XCAM_RETURN_NO_ERROR);
        } else {
            ret = selectCCM(pCcm->aCcmCof, pCcm->aCcmCof_len, hAccm, fSaturation, &updUndampMat);
            RETURN_RESULT_IF_DIFFERENT(ret, XCAM_RETURN_NO_ERROR);
        }
    }
    // 4) calc scale for y_alpha_curve and
    if (hAccm->update || hAccm->updateAtt) {
        float fScale = 1.0;
#if 1
        // real use
        interpolation(calib->lumaCCM.gain_alphaScale_curve.gain,
                      calib->lumaCCM.gain_alphaScale_curve.scale, 9, sensorGain, &fScale);
#else
        // for test, to be same with demo
        for (int i = 0; i < 9; i++) {
            int j = uint16_t(sensorGain);
            j     = (j > (1 << 8)) ? (1 << 8) : j;

            if (j <= (1 << i)) {
                fScale = calib->lumaCCM.gain_alphaScale_curve.scale[i];
                break;
            }
        }
#endif

        float flevel1 = 100;
        float flevel2;
        // 5) color inhibition / saturation adjust for api
#if RKAIQ_HAVE_CCM_V2
        interpolation(hAccm->mCurAttV2.stAuto.color_inhibition.sensorGain,
                      hAccm->mCurAttV2.stAuto.color_inhibition.level, RK_AIQ_ACCM_COLOR_GAIN_NUM,
                      sensorGain, &flevel2);
        interpolation(hAccm->mCurAttV2.stAuto.color_saturation.sensorGain,
                      hAccm->mCurAttV2.stAuto.color_saturation.level, RK_AIQ_ACCM_COLOR_GAIN_NUM,
                      sensorGain, &flevel1);
#elif RKAIQ_HAVE_CCM_V3
        interpolation(hAccm->mCurAttV3.stAuto.color_inhibition.sensorGain,
                      hAccm->mCurAttV3.stAuto.color_inhibition.level, RK_AIQ_ACCM_COLOR_GAIN_NUM,
                      sensorGain, &flevel2);
        interpolation(hAccm->mCurAttV3.stAuto.color_saturation.sensorGain,
                      hAccm->mCurAttV3.stAuto.color_saturation.level, RK_AIQ_ACCM_COLOR_GAIN_NUM,
                      sensorGain, &flevel1);
#endif
        hAccm->accmRest.color_inhibition_level = flevel2;

        if (hAccm->accmRest.color_inhibition_level > 100 || hAccm->accmRest.color_inhibition_level < 0) {
            LOGE_ACCM("flevel2: %f is out of range [0 100]\n", hAccm->accmRest.color_inhibition_level);
            return XCAM_RETURN_ERROR_PARAM;
        }

        fScale *= (100 - hAccm->accmRest.color_inhibition_level) / 100;

        if (flevel1 > 100 || flevel1 < 0) {
            LOGE_ACCM("flevel1: %f is out of range [0 100]\n", flevel1);
            return XCAM_RETURN_ERROR_PARAM;
        }

        LOGD_ACCM("CcmProfile changed: %d, fScale: %f->%f, sat_level: %f->%f",
                  updUndampMat, hAccm->accmRest.fScale, fScale,
                  hAccm->accmRest.color_saturation_level, flevel1);

        bool flag = updUndampMat ||
                    fabs(fScale - hAccm->accmRest.fScale) > DIVMIN ||
                    fabs(flevel1 - hAccm->accmRest.color_saturation_level) > DIVMIN;

        if (flag || (!hAccm->invarMode)) {
            if (flag) {
                hAccm->accmRest.fScale = fScale;
                hAccm->accmRest.color_saturation_level = flevel1;
                Saturationadjust(fScale, flevel1, hAccm->accmRest.undampedCcmMatrix);
                LOGD_ACCM("Adjust ccm by sat: %d, undampedCcmMatrix[0]: %f", hAccm->isReCal_, hAccm->accmRest.undampedCcmMatrix[0]);
            }

            updUndampMat = true;
        }
        if (!hAccm->invarMode) {
            ConfigHwbyCalib(calib, &hAccm->ccmHwConf_v2);
            updateYAlp = true;
        }

        float iso = sensorGain * 50;
        bool yalp_flag = false;
        if (calib->lumaCCM.asym_enable) {
            int mid = CCM_CURVE_DOT_NUM_V2 >> 1;
            YAlpAsymAutoCfg(CCM_YALP_ISO_STEP_MAX, calib->lumaCCM.y_alp_asym.gain_yalp_curve,
                            hAccm->accmRest.yalp_tbl_info.scl,
                            hAccm->accmRest.yalp_tbl_info.tbl_idx,
                            iso,
                            (!hAccm->invarMode || hAccm->calib_update),
                            hAccm->ccmHwConf_v2.alp_y, &yalp_flag);
        } else {
            YAlpSymAutoCfg(CCM_YALP_ISO_STEP_MAX, calib->lumaCCM.y_alp_sym.gain_yalp_curve,
                            hAccm->accmRest.yalp_tbl_info.scl,
                            hAccm->accmRest.yalp_tbl_info.tbl_idx,
                            iso, (!hAccm->invarMode || hAccm->calib_update), hAccm->ccmHwConf_v2.alp_y, &yalp_flag);
        }
        updateYAlp = updateYAlp || yalp_flag;

    }
    // 7) . Damping
    float dampCoef = (pCcm->damp_enable && hAccm->count > 1 && hAccm->invarMode > 0) ? hAccm->accmSwInfo.awbIIRDampCoef : 0;
    if (!hAccm->accmSwInfo.ccmConverged || updUndampMat) {
        ret = Damping(dampCoef,
                    hAccm->accmRest.undampedCcmMatrix, hAccm->ccmHwConf_v2.matrix,
                    hAccm->accmRest.undampedCcOffset, hAccm->ccmHwConf_v2.offs,
                    &hAccm->accmSwInfo.ccmConverged);
        updateMat = true;
        LOGD_ACCM(
            "damping: %f, ccm coef[0]: %f->%f, ccm coef[8]: %f->%f",
            dampCoef, hAccm->accmRest.undampedCcmMatrix[0], hAccm->ccmHwConf_v2.matrix[0],
            hAccm->accmRest.undampedCcmMatrix[8], hAccm->ccmHwConf_v2.matrix[8]);
    }

    if (hAccm->update || (!hAccm->invarMode)) {
        float enh_rat_max = 0;
        if (calib->enhCCM.enh_adj_en) {
            interpolation(calib->enhCCM.enh_rat.gains, calib->enhCCM.enh_rat.enh_rat_max, 9,
                          sensorGain, &enh_rat_max);
        }
        updateEnh = (calib->enhCCM.enh_adj_en != hAccm->ccmHwConf_v2.enh_adj_en) ||
                    fabs(enh_rat_max - hAccm->ccmHwConf_v2.enh_rat_max) > DIVMIN;
        if (updateEnh) {
            hAccm->ccmHwConf_v2.enh_adj_en = calib->enhCCM.enh_adj_en;
            hAccm->ccmHwConf_v2.enh_rat_max = enh_rat_max;
        }
    }

    hAccm->isReCal_ = hAccm->isReCal_ || updateMat || updateYAlp || updateEnh;
#if RKAIQ_HAVE_CCM_V3
    bool updateHf = false;
    if (hAccm->calib_update || (!hAccm->invarMode)) {
        updateHf = (calib->hfCCM.en != hAccm->ccmHwConf_v2.sat_decay_en) ||
                   fabs(calib->hfCCM.hf_lowTh - hAccm->ccmHwConf_v2.hf_low) > DIVMIN ||
                   fabs(calib->hfCCM.hf_upTh - hAccm->ccmHwConf_v2.hf_up) > DIVMIN ||
                   fabs(calib->hfCCM.hf_scale - hAccm->ccmHwConf_v2.hf_scale) > DIVMIN ||
                   memcmp(calib->hfCCM.hf_factor, hAccm->ccmHwConf_v2.hf_factor, sizeof(hAccm->ccmHwConf_v2.hf_factor));
        if (updateHf) {
            hAccm->ccmHwConf_v2.sat_decay_en = calib->hfCCM.en;
            hAccm->ccmHwConf_v2.hf_low = calib->hfCCM.hf_lowTh;
            hAccm->ccmHwConf_v2.hf_up = calib->hfCCM.hf_upTh;
            hAccm->ccmHwConf_v2.hf_scale = calib->hfCCM.hf_scale;
            memcpy(hAccm->ccmHwConf_v2.hf_factor, calib->hfCCM.hf_factor, sizeof(uint16_t)*CCM_HF_FACTOR_NUM);
        }
    }
    hAccm->isReCal_ = hAccm->isReCal_ || updateHf;
#endif


    LOGD_ACCM("final isReCal_ = %d \n", hAccm->isReCal_);

    LOG1_ACCM("%s: (exit)\n", __FUNCTION__);

    return (ret);
}

XCamReturn AccmManualConfig(accm_handle_t hAccm) {
    LOG1_ACCM("%s: (enter)\n", __FUNCTION__);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if RKAIQ_HAVE_CCM_V2
    const rk_aiq_ccm_mccm_attrib_v2_t* stManual = &hAccm->mCurAttV2.stManual;
#elif RKAIQ_HAVE_CCM_V3
    const rk_aiq_ccm_mccm_attrib_v3_t* stManual = &hAccm->mCurAttV3.stManual;
#endif
    memcpy(hAccm->ccmHwConf_v2.matrix, stManual->Matrix.ccMatrix,
           sizeof(stManual->Matrix.ccMatrix));
    memcpy(hAccm->ccmHwConf_v2.offs, stManual->Matrix.ccOffsets,
           sizeof(stManual->Matrix.ccOffsets));
    hAccm->ccmHwConf_v2.highy_adj_en = stManual->YAlp.highy_adj_en;
    hAccm->ccmHwConf_v2.asym_adj_en  = stManual->YAlp.asym_enable;
    hAccm->ccmHwConf_v2.bound_bit    = stManual->YAlp.bound_pos_bit;
    hAccm->ccmHwConf_v2.right_bit    = stManual->YAlp.right_pos_bit;
    memcpy(hAccm->ccmHwConf_v2.alp_y, stManual->YAlp.y_alpha_curve,
           sizeof(stManual->YAlp.y_alpha_curve));

    memcpy(hAccm->ccmHwConf_v2.enh_rgb2y_para, stManual->Enh.enh_rgb2y_para,
           sizeof(stManual->Enh.enh_rgb2y_para));
    hAccm->ccmHwConf_v2.enh_adj_en  = stManual->Enh.enh_adj_en;
    hAccm->ccmHwConf_v2.enh_rat_max = stManual->Enh.enh_rat_max;
#if RKAIQ_HAVE_CCM_V3
    hAccm->ccmHwConf_v2.sat_decay_en = stManual->HfSat.en;
    hAccm->ccmHwConf_v2.hf_low       = stManual->HfSat.hf_lowTh;
    hAccm->ccmHwConf_v2.hf_up        = stManual->HfSat.hf_upTh;
    hAccm->ccmHwConf_v2.hf_scale     = stManual->HfSat.hf_scale;
    memcpy(hAccm->ccmHwConf_v2.hf_factor, stManual->HfSat.hf_factor, sizeof(stManual->HfSat.hf_factor));
#endif

    LOG1_ACCM("%s: (exit)\n", __FUNCTION__);
    return ret;
}

XCamReturn AccmConfig(accm_handle_t hAccm) {
    LOG1_ACCM("%s: (enter)\n", __FUNCTION__);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if RKAIQ_HAVE_CCM_V2
    const CalibDbV2_Ccm_Para_V32_t* ccmCalib = hAccm->ccm_v2;
    rk_aiq_ccm_v2_attrib_t* mCurAtt = &hAccm->mCurAttV2;
#elif RKAIQ_HAVE_CCM_V3
    const CalibDbV2_Ccm_Para_V39_t* ccmCalib = hAccm->ccm_v3;
    rk_aiq_ccm_v3_attrib_t* mCurAtt = &hAccm->mCurAttV3;
#endif

    LOGD_ACCM("%s: byPass: %d  mode:%d updateAtt: %d \n", __FUNCTION__, mCurAtt->byPass,
              mCurAtt->mode, hAccm->updateAtt);
    if (mCurAtt->byPass != true && hAccm->accmSwInfo.grayMode != true) {
        hAccm->ccmHwConf_v2.ccmEnable = true;

        if (mCurAtt->mode == RK_AIQ_CCM_MODE_AUTO) {
            hAccm->update = JudgeCcmRes3aConverge(&hAccm->accmRest.res3a_info, &hAccm->accmSwInfo,
                                                  ccmCalib->control.gain_tolerance,
                                                  ccmCalib->control.wbgain_tolerance);
            hAccm->update = hAccm->update || hAccm->calib_update; // wbgain/gain/calib changed
            LOGD_ACCM("%s: CCM update (gain/awbgain/calib): %d, CCM Converged: %d\n",
                __FUNCTION__, hAccm->update, hAccm->accmSwInfo.ccmConverged);
            if (hAccm->updateAtt || hAccm->update || (!hAccm->accmSwInfo.ccmConverged)) {
                AccmAutoConfig(hAccm);
                CCMV2PrintDBG(hAccm);
            }
        } else if (mCurAtt->mode == RK_AIQ_CCM_MODE_MANUAL) {
            if (hAccm->updateAtt) {
                AccmManualConfig(hAccm);
                hAccm->isReCal_ = true;
            }
        } else {
            LOGE_ACCM("%s: mCurAtt->mode(%d) is invalid \n", __FUNCTION__,
                      mCurAtt->mode);
        }

    } else {
        hAccm->ccmHwConf_v2.ccmEnable = false;
        // graymode/api/calib changed
        hAccm->isReCal_ = hAccm->isReCal_ || hAccm->updateAtt || hAccm->calib_update;
    }
    hAccm->updateAtt = false;
    hAccm->calib_update = false;
    hAccm->count = ((hAccm->count + 2) > (65536)) ? 2 : (hAccm->count + 1);

    CCMV2PrintReg(&hAccm->ccmHwConf_v2);

    LOG1_ACCM("%s: (exit)\n", __FUNCTION__);
    return ret;
}

XCamReturn ConfigbyCalib(accm_handle_t hAccm) {
    LOG1_ACCM("%s: (enter)  \n", __FUNCTION__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#if RKAIQ_HAVE_CCM_V2
    const CalibDbV2_Ccm_Para_V32_t* calib_ccm = hAccm->ccm_v2;
    if (hAccm->mCurAttV2.mode == RK_AIQ_CCM_MODE_AUTO)
        hAccm->mCurAttV2.byPass = !(calib_ccm->control.enable);
#elif RKAIQ_HAVE_CCM_V3
    const CalibDbV2_Ccm_Para_V39_t* calib_ccm = hAccm->ccm_v3;
    if (hAccm->mCurAttV3.mode == RK_AIQ_CCM_MODE_AUTO)
        hAccm->mCurAttV3.byPass = !(calib_ccm->control.enable);
#endif

    ret = pCcmMatrixAll_init(calib_ccm->TuningPara.aCcmCof,
                             calib_ccm->TuningPara.aCcmCof_len,
                             calib_ccm->TuningPara.matrixAll,
                             calib_ccm->TuningPara.matrixAll_len,
                             hAccm->pCcmMatrixAll);

    ConfigHwbyCalib(calib_ccm, &hAccm->ccmHwConf_v2);

    if (calib_ccm->lumaCCM.asym_enable) {
        int mid                          = CCM_CURVE_DOT_NUM_V2 >> 1;
        for (int i = 0; i < mid; i++) {
            hAccm->ccmHwConf_v2.alp_y[i] = calib_ccm->lumaCCM.y_alp_asym.gain_yalp_curve[0].y_alpha_lcurve[i];
            hAccm->ccmHwConf_v2.alp_y[mid + i] =
                calib_ccm->lumaCCM.y_alp_asym.gain_yalp_curve[0].y_alpha_rcurve[i];
        }
    } else {
        hAccm->ccmHwConf_v2.highy_adj_en = calib_ccm->lumaCCM.y_alp_sym.highy_adj_en;
        hAccm->ccmHwConf_v2.bound_bit    = calib_ccm->lumaCCM.y_alp_sym.bound_pos_bit;
        hAccm->ccmHwConf_v2.right_bit    = hAccm->ccmHwConf_v2.bound_bit;
        memcpy(hAccm->ccmHwConf_v2.alp_y, calib_ccm->lumaCCM.y_alp_sym.gain_yalp_curve[0].y_alpha_curve,
               sizeof(calib_ccm->lumaCCM.y_alp_sym.gain_yalp_curve[0].y_alpha_curve));
    }

    hAccm->ccmHwConf_v2.enh_adj_en  = calib_ccm->enhCCM.enh_adj_en;
    hAccm->ccmHwConf_v2.enh_rat_max = calib_ccm->enhCCM.enh_rat.enh_rat_max[0];

#if RKAIQ_HAVE_CCM_V3
    hAccm->ccmHwConf_v2.sat_decay_en = calib_ccm->hfCCM.en;
    hAccm->ccmHwConf_v2.hf_low       = calib_ccm->hfCCM.hf_lowTh;
    hAccm->ccmHwConf_v2.hf_up        = calib_ccm->hfCCM.hf_upTh;
    hAccm->ccmHwConf_v2.hf_scale     = calib_ccm->hfCCM.hf_scale;
    memcpy(hAccm->ccmHwConf_v2.hf_factor, calib_ccm->hfCCM.hf_factor, sizeof(calib_ccm->hfCCM.hf_factor));
#endif

    hAccm->accmSwInfo.ccmConverged = false;
    hAccm->calib_update            = true;

    hAccm->accmRest.illuNum = calib_ccm->TuningPara.aCcmCof_len;

    clear_list(&hAccm->accmRest.problist);

    LOG1_ACCM("%s: (exit)\n", __FUNCTION__);
    return (ret);
}

/**********************************
 *Update CCM CalibV2 Para
 *      Prepare init
 *      Mode change: reinit
 *      Res change: continue
 *      Calib change: continue
 ***************************************/
static XCamReturn UpdateCcmCalibV2ParaV2(accm_handle_t hAccm) {
    LOG1_ACCM("%s: (enter)  \n", __FUNCTION__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    bool config_calib = !!(hAccm->accmSwInfo.prepare_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB);

    if (!config_calib) {
        return (ret);
    }

#if RKAIQ_HAVE_CCM_V2
    const CalibDbV2_Ccm_Para_V32_t* calib_ccm = hAccm->ccm_v2;
#elif RKAIQ_HAVE_CCM_V3
    const CalibDbV2_Ccm_Para_V39_t* calib_ccm = hAccm->ccm_v3;
#endif

#if RKAIQ_ACCM_ILLU_VOTE
    if (hAccm->accmRest.illuNum != calib_ccm->TuningPara.aCcmCof_len)
        clear_list(&hAccm->accmRest.dominateIlluList);
#endif

    // record dynamic array init length
    ccm_calib_initlen_info_t* calib_initlen_info = &hAccm->accmRest.ccm_calib_initlen_info;
    calib_initlen_info->accmCof_initlen = calib_ccm->TuningPara.aCcmCof_len;
    for (int i = 0; i < calib_ccm->TuningPara.aCcmCof_len; i++) {
        calib_initlen_info->accmCof_initlen_info[i].name_len =
            strlen(calib_ccm->TuningPara.aCcmCof[i].name);
        calib_initlen_info->accmCof_initlen_info[i].matused_len =
            calib_ccm->TuningPara.aCcmCof[i].matrixUsed_len;
        for (int j = 0; j < calib_ccm->TuningPara.aCcmCof[i].matrixUsed_len; j++)
            calib_initlen_info->accmCof_initlen_info[i].matused_str_len[j] =
                strlen(calib_ccm->TuningPara.aCcmCof[i].matrixUsed[j]);
    }
    calib_initlen_info->matrixall_initlen = calib_ccm->TuningPara.matrixAll_len;
    for (int i = 0; i < calib_ccm->TuningPara.matrixAll_len; i++) {
        calib_initlen_info->matrixall_initlen_info[i].name_len =
            strlen(calib_ccm->TuningPara.matrixAll[i].name);
        calib_initlen_info->matrixall_initlen_info[i].illu_len =
            strlen(calib_ccm->TuningPara.matrixAll[i].illumination);
    }

    ret = ConfigbyCalib(hAccm);

    LOG1_ACCM("%s: (exit)\n", __FUNCTION__);
    return (ret);
}
#if RKAIQ_HAVE_CCM_V2
static XCamReturn ApiAttrV2Init(const rk_aiq_ccm_cfg_v2_t*      ccmHwConf,
                                const CalibDbV2_Ccm_Para_V32_t* Calib,
                                rk_aiq_ccm_v2_attrib_t*         mCurAtt)
{
#elif RKAIQ_HAVE_CCM_V3
static XCamReturn ApiAttrV2Init(const rk_aiq_ccm_cfg_v2_t*      ccmHwConf,
                                const CalibDbV2_Ccm_Para_V39_t* Calib,
                                rk_aiq_ccm_v3_attrib_t*         mCurAtt)
{
#endif
    LOGI_ACCM("%s: (enter)\n", __FUNCTION__);

    XCamReturn ret               = XCAM_RETURN_NO_ERROR;
    mCurAtt->byPass = !(Calib->control.enable);
    // StAuto
    for (int i = 0; i < RK_AIQ_ACCM_COLOR_GAIN_NUM; i++) {
        mCurAtt->stAuto.color_inhibition.sensorGain[i] = 1;
        mCurAtt->stAuto.color_inhibition.level[i]      = 0;
        mCurAtt->stAuto.color_saturation.sensorGain[i] = 1;
        mCurAtt->stAuto.color_saturation.level[i]      = 50;
    }
    // StManual
    if (Calib->TuningPara.matrixAll_len > 0) {
        memcpy(mCurAtt->stManual.Matrix.ccMatrix, Calib->TuningPara.matrixAll[0].ccMatrix,
               sizeof(Calib->TuningPara.matrixAll[0].ccMatrix));
        memcpy(mCurAtt->stManual.Matrix.ccOffsets, Calib->TuningPara.matrixAll[0].ccOffsets,
               sizeof(Calib->TuningPara.matrixAll[0].ccOffsets));
    } else {
        memset(mCurAtt->stManual.Matrix.ccMatrix, 0, sizeof(mCurAtt->stManual.Matrix.ccMatrix));
        memset(mCurAtt->stManual.Matrix.ccOffsets, 0, sizeof(mCurAtt->stManual.Matrix.ccOffsets));
        mCurAtt->stManual.Matrix.ccMatrix[0] = 1.0;
        mCurAtt->stManual.Matrix.ccMatrix[4] = 1.0;
        mCurAtt->stManual.Matrix.ccMatrix[8] = 1.0;
    }

    memcpy(mCurAtt->stManual.YAlp.y_alpha_curve, ccmHwConf->alp_y,
           sizeof(ccmHwConf->alp_y));
    memcpy(mCurAtt->stManual.Enh.enh_rgb2y_para, ccmHwConf->enh_rgb2y_para,
            sizeof(ccmHwConf->enh_rgb2y_para));
    mCurAtt->stManual.Enh.enh_adj_en    = ccmHwConf->enh_adj_en;
    mCurAtt->stManual.Enh.enh_rat_max   = ccmHwConf->enh_rat_max;
    mCurAtt->stManual.YAlp.highy_adj_en  = ccmHwConf->highy_adj_en;
    mCurAtt->stManual.YAlp.asym_enable   = ccmHwConf->asym_adj_en;
    mCurAtt->stManual.YAlp.bound_pos_bit = ccmHwConf->bound_bit;
    mCurAtt->stManual.YAlp.right_pos_bit = ccmHwConf->right_bit;
#if RKAIQ_HAVE_CCM_V3
    mCurAtt->stManual.HfSat.en       = ccmHwConf->sat_decay_en;
    mCurAtt->stManual.HfSat.hf_lowTh = ccmHwConf->hf_low;
    mCurAtt->stManual.HfSat.hf_upTh  = ccmHwConf->hf_up;
    mCurAtt->stManual.HfSat.hf_scale = ccmHwConf->hf_scale;
    memcpy(mCurAtt->stManual.HfSat.hf_factor, ccmHwConf->hf_factor, sizeof(ccmHwConf->hf_factor));
#endif

    LOGI_ACCM("%s: (exit)\n", __FUNCTION__);
    return (ret);
}

XCamReturn AccmInit(accm_handle_t* hAccm, const CamCalibDbV2Context_t* calibv2) {
    LOGI_ACCM("%s: (enter)\n", __FUNCTION__);

    XCamReturn ret               = XCAM_RETURN_NO_ERROR;

    if (calibv2 == NULL) {
        return XCAM_RETURN_ERROR_PARAM;
    }
#if RKAIQ_HAVE_CCM_V2
    CalibDbV2_Ccm_Para_V32_t* calib_ccm =
        (CalibDbV2_Ccm_Para_V32_t*)(CALIBDBV2_GET_MODULE_PTR((void*)calibv2, ccm_calib_v2));
#elif RKAIQ_HAVE_CCM_V3
    CalibDbV2_Ccm_Para_V39_t* calib_ccm =
        (CalibDbV2_Ccm_Para_V39_t*)(CALIBDBV2_GET_MODULE_PTR((void*)calibv2, ccm_calib_v2));
#endif
    if (calib_ccm == NULL) return XCAM_RETURN_ERROR_MEM;

    *hAccm                       = (accm_context_t*)malloc(sizeof(accm_context_t));
    accm_context_t* accm_context = *hAccm;
    memset(accm_context, 0, sizeof(accm_context_t));

    accm_context->accmSwInfo.sensorGain     = 1.0;
    accm_context->accmSwInfo.awbIIRDampCoef = 0;
    accm_context->accmSwInfo.varianceLuma   = 255;
    accm_context->accmSwInfo.awbConverged   = false;

    accm_context->accmSwInfo.awbGain[0]     = 1;
    accm_context->accmSwInfo.awbGain[1]     = 1;

    accm_context->accmRest.res3a_info.sensorGain = 1.0;
    accm_context->accmRest.res3a_info.awbGain[0] = 1.0;
    accm_context->accmRest.res3a_info.awbGain[1] = 1.0;

    accm_context->count = 0;
    accm_context->isReCal_ = 1;
    accm_context->invarMode = 1;

    accm_context->accmSwInfo.prepare_type =
        RK_AIQ_ALGO_CONFTYPE_UPDATECALIB | RK_AIQ_ALGO_CONFTYPE_NEEDRESET;

    // todo whm --- CalibDbV2_Ccm_Para_V2
#if RKAIQ_HAVE_CCM_V2
    accm_context->ccm_v2 = calib_ccm;
    accm_context->mCurAttV2.mode = RK_AIQ_CCM_MODE_AUTO;
#elif RKAIQ_HAVE_CCM_V3
    accm_context->ccm_v3 = calib_ccm;
    accm_context->mCurAttV3.mode = RK_AIQ_CCM_MODE_AUTO;
#endif

#if RKAIQ_ACCM_ILLU_VOTE
    INIT_LIST_HEAD(&accm_context->accmRest.dominateIlluList);
#endif
    INIT_LIST_HEAD(&accm_context->accmRest.problist);
    ret = UpdateCcmCalibV2ParaV2(accm_context);
#if RKAIQ_HAVE_CCM_V2
    ApiAttrV2Init(&accm_context->ccmHwConf_v2, accm_context->ccm_v2,
                  &accm_context->mCurAttV2);
#elif RKAIQ_HAVE_CCM_V3
    ApiAttrV2Init(&accm_context->ccmHwConf_v2, accm_context->ccm_v3,
                  &accm_context->mCurAttV3);
#endif

    accm_context->accmRest.fScale = 1;
    accm_context->accmRest.color_inhibition_level = 0;
    accm_context->accmRest.color_saturation_level = 100;

    LOGI_ACCM("%s: (exit)\n", __FUNCTION__);
    return (ret);
}

XCamReturn AccmRelease(accm_handle_t hAccm) {
    LOGI_ACCM("%s: (enter)\n", __FUNCTION__);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#if RKAIQ_ACCM_ILLU_VOTE
    clear_list(&hAccm->accmRest.dominateIlluList);
#endif
    clear_list(&hAccm->accmRest.problist);
    free(hAccm);
    hAccm = NULL;

    LOGI_ACCM("%s: (exit)\n", __FUNCTION__);
    return (ret);
}

// todo whm
XCamReturn AccmPrepare(accm_handle_t hAccm) {
    LOGI_ACCM("%s: (enter)\n", __FUNCTION__);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = UpdateCcmCalibV2ParaV2(hAccm);
    RETURN_RESULT_IF_DIFFERENT(ret, XCAM_RETURN_NO_ERROR);

    LOGI_ACCM("%s: (exit)\n", __FUNCTION__);
    return ret;
}

XCamReturn AccmPreProc(accm_handle_t hAccm) {
    LOG1_ACCM("%s: (enter)\n", __FUNCTION__);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    LOG1_ACCM("%s: (exit)\n", __FUNCTION__);
    return (ret);
}
XCamReturn AccmProcessing(accm_handle_t hAccm) {
    LOG1_ACCM("%s: (enter)\n", __FUNCTION__);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    LOG1_ACCM("%s: (exit)\n", __FUNCTION__);
    return (ret);
}

RKAIQ_END_DECLARE
