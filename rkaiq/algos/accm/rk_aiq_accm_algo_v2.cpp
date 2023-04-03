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
        " sw_ccm_asym_adj_en %d"
        " sw_ccm_enh_adj_en %d"
        " sw_ccm_highy_adjust %d"
        " sw_ccm_en_i %d"
        " sw_ccm_coeff ([%f,%f,%f,%f,%f,%f,%f,%f,%f]-E)X128"
        " sw_ccm_offset [%f,%f,%f]"
        " sw_ccm_coeff_y [%f,%f,%f]"
        " sw_ccm_alp_y [%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f]"
        " sw_ccm_right_bit %f"
        " sw_ccm_bound_bit %f"
        " sw_ccm_color_coef_y [%d,%d,%d]"
        " sw_ccm_color_enh_rat_max %f",
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
}

void CCMV2PrintDBG(const accm_context_t* accm_context) {
    const CalibDbV2_Ccm_Para_V32_t* pCcm = accm_context->ccm_v2;
    const float* pMatrixUndamped         = accm_context->accmRest.undampedCcmMatrix;
    const float* pOffsetUndamped         = accm_context->accmRest.undampedCcOffset;
    const float* pMatrixDamped           = accm_context->accmRest.dampedCcmMatrix;
    const float* pOffsetDamped           = accm_context->accmRest.dampedCcOffset;

    LOG1_ACCM(
        "Illu Probability Estimation Enable: %d"
        "color_inhibition sensorGain: %f,%f,%f,%f "
        "color_inhibition level: %f,%f,%f,%f"
        "color_saturation sensorGain: %f,%f,%f,%f "
        "color_saturation level: %f,%f,%f,%f"
        "dampfactor: %f"
        " undampedCcmMatrix: %f,%f,%f,%f,%f,%f,%f,%f,%f"
        " undampedCcOffset: %f,%f,%f "
        " dampedCcmMatrix: %f,%f,%f,%f,%f,%f,%f,%f,%f"
        " dampedCcOffset:%f,%f,%f",
        pCcm->TuningPara.illu_estim.interp_enable,
        accm_context->mCurAttV2.stAuto.color_inhibition.sensorGain[0],
        accm_context->mCurAttV2.stAuto.color_inhibition.sensorGain[1],
        accm_context->mCurAttV2.stAuto.color_inhibition.sensorGain[2],
        accm_context->mCurAttV2.stAuto.color_inhibition.sensorGain[3],
        accm_context->mCurAttV2.stAuto.color_inhibition.level[0],
        accm_context->mCurAttV2.stAuto.color_inhibition.level[1],
        accm_context->mCurAttV2.stAuto.color_inhibition.level[2],
        accm_context->mCurAttV2.stAuto.color_inhibition.level[3],
        accm_context->mCurAttV2.stAuto.color_saturation.sensorGain[0],
        accm_context->mCurAttV2.stAuto.color_saturation.sensorGain[1],
        accm_context->mCurAttV2.stAuto.color_saturation.sensorGain[2],
        accm_context->mCurAttV2.stAuto.color_saturation.sensorGain[3],
        accm_context->mCurAttV2.stAuto.color_saturation.level[0],
        accm_context->mCurAttV2.stAuto.color_saturation.level[1],
        accm_context->mCurAttV2.stAuto.color_saturation.level[2],
        accm_context->mCurAttV2.stAuto.color_saturation.level[3], pCcm->TuningPara.damp_enable,
        pMatrixUndamped[0], pMatrixUndamped[1], pMatrixUndamped[2], pMatrixUndamped[3],
        pMatrixUndamped[4], pMatrixUndamped[5], pMatrixUndamped[6], pMatrixUndamped[7],
        pMatrixUndamped[8], pOffsetUndamped[0], pOffsetUndamped[1], pOffsetUndamped[2],
        pMatrixDamped[0], pMatrixDamped[1], pMatrixDamped[2], pMatrixDamped[3], pMatrixDamped[4],
        pMatrixDamped[5], pMatrixDamped[6], pMatrixDamped[7], pMatrixDamped[8], pOffsetDamped[0],
        pOffsetDamped[1], pOffsetDamped[2]);
}

XCamReturn AccmAutoConfig(accm_handle_t hAccm) {
    LOG1_ACCM("%s: (enter)\n", __FUNCTION__);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (hAccm == NULL) {
        return XCAM_RETURN_ERROR_PARAM;
    }

    const CalibDbV2_Ccm_Para_V32_t* pCcm = NULL;
    float sensorGain                     = hAccm->accmSwInfo.sensorGain;
    float fSaturation                    = 0;
    pCcm = hAccm->ccm_v2;
    if (hAccm->update || hAccm->updateAtt) {
        if (pCcm->TuningPara.illu_estim.interp_enable) {
            ret = interpCCMbywbgain(&pCcm->TuningPara, hAccm, fSaturation);
            RETURN_RESULT_IF_DIFFERENT(ret, XCAM_RETURN_NO_ERROR);
        } else {
            ret = selectCCM(&pCcm->TuningPara, hAccm, fSaturation);
            RETURN_RESULT_IF_DIFFERENT(ret, XCAM_RETURN_NO_ERROR);
        }

        // 4) calc scale for y_alpha_curve
        float fScale = 1.0;
#if 1
        // real use
        interpolation(pCcm->lumaCCM.gain_alphaScale_curve.gain,
                      pCcm->lumaCCM.gain_alphaScale_curve.scale, 9, sensorGain, &fScale);
#else
        // for test, to be same with demo
        for (int i = 0; i < 9; i++) {
            int j = uint16_t(sensorGain);
            j     = (j > (1 << 8)) ? (1 << 8) : j;

            if (j <= (1 << i)) {
                fScale = pCcm->lumaCCM.gain_alphaScale_curve.scale[i];
                break;
            }
        }
#endif
        // 5) color inhibition adjust for api
        float flevel2;
        interpolation(hAccm->mCurAttV2.stAuto.color_inhibition.sensorGain,
                      hAccm->mCurAttV2.stAuto.color_inhibition.level, RK_AIQ_ACCM_COLOR_GAIN_NUM,
                      sensorGain, &flevel2);
        hAccm->accmRest.color_inhibition_level = flevel2;

        if (flevel2 > 100 || flevel2 < 0) {
            LOGE_ACCM("flevel2: %f is out of range [0 100]\n", flevel2);
            return XCAM_RETURN_ERROR_PARAM;
        }

        fScale *= (100 - flevel2) / 100;

        // 6)   saturation adjust for api
        float flevel1;
        interpolation(hAccm->mCurAttV2.stAuto.color_saturation.sensorGain,
                      hAccm->mCurAttV2.stAuto.color_saturation.level, RK_AIQ_ACCM_COLOR_GAIN_NUM,
                      sensorGain, &flevel1);

        if (flevel1 > 100 || flevel1 < 0) {
            LOGE_ACCM("flevel1: %f is out of range [0 100]\n", flevel1);
            return XCAM_RETURN_ERROR_PARAM;
        }
        hAccm->accmRest.color_saturation_level = flevel1;

        LOGD_ACCM(
            "final fScale: %f, color inhibition level: %f, fSaturation: %f, color saturation "
            "level: %f\n",
            fScale, flevel2, fSaturation, flevel1);

        Saturationadjust(fScale, hAccm->accmRest.color_saturation_level, hAccm->accmRest.undampedCcmMatrix);

        if (pCcm->lumaCCM.asym_enable) {
            int mid = CCM_CURVE_DOT_NUM_V2 >> 1;
            for (int i = 0; i < mid; i++) {
                hAccm->ccmHwConf_v2.alp_y[i] =
                    fScale * pCcm->lumaCCM.y_alp_asym.y_alpha_left_curve[i];
                hAccm->ccmHwConf_v2.alp_y[mid + i] =
                    fScale * pCcm->lumaCCM.y_alp_asym.y_alpha_right_curve[i];
            }
        } else {
            for (int i = 0; i < CCM_CURVE_DOT_NUM; i++) {  // set to ic  to do bit check
                hAccm->ccmHwConf_v2.alp_y[i] = fScale * pCcm->lumaCCM.y_alp_sym.y_alpha_curve[i];
            }
            hAccm->ccmHwConf_v2.alp_y[CCM_CURVE_DOT_NUM] = 1024;
        }

        interpolation(pCcm->enhCCM.enh_ctl.gains, pCcm->enhCCM.enh_ctl.enh_adj_en, 9,
                      sensorGain, &hAccm->ccmHwConf_v2.enh_adj_en);
        interpolation(pCcm->enhCCM.enh_ctl.gains, pCcm->enhCCM.enh_ctl.enh_rat_max, 9,
                      sensorGain, &hAccm->ccmHwConf_v2.enh_rat_max);

    }
    if (pCcm->TuningPara.damp_enable && (hAccm->count > 1) && (hAccm->accmSwInfo.awbIIRDampCoef > 0.0) &&
                            ((!hAccm->accmSwInfo.ccmConverged) || hAccm->update || hAccm->updateAtt)) {
        // 7) . Damping
        ret = Damping(hAccm->accmSwInfo.awbIIRDampCoef,
                    hAccm->accmRest.undampedCcmMatrix, hAccm->accmRest.dampedCcmMatrix,
                    hAccm->accmRest.undampedCcOffset, hAccm->accmRest.dampedCcOffset);
        hAccm->accmSwInfo.ccmConverged =
                !(memcmp(hAccm->accmRest.undampedCcmMatrix, hAccm->accmRest.dampedCcmMatrix,
            sizeof(hAccm->accmRest.dampedCcmMatrix)) ||
            memcmp(hAccm->accmRest.undampedCcOffset, hAccm->accmRest.dampedCcOffset,
            sizeof(hAccm->accmRest.dampedCcOffset)));
    } else {
        memcpy(hAccm->accmRest.dampedCcmMatrix, hAccm->accmRest.undampedCcmMatrix,
            sizeof(hAccm->accmRest.dampedCcmMatrix));
        hAccm->accmRest.dampedCcOffset[0] = hAccm->accmRest.undampedCcOffset[0];
        hAccm->accmRest.dampedCcOffset[1] = hAccm->accmRest.undampedCcOffset[1];
        hAccm->accmRest.dampedCcOffset[2] = hAccm->accmRest.undampedCcOffset[2];
        hAccm->accmSwInfo.ccmConverged = true;
    }
    // 8)  set to ic  to do bit check
    memcpy(hAccm->ccmHwConf_v2.matrix, hAccm->accmRest.dampedCcmMatrix,
        sizeof(hAccm->accmRest.dampedCcmMatrix));
    hAccm->ccmHwConf_v2.offs[0] = hAccm->accmRest.dampedCcOffset[0];
    hAccm->ccmHwConf_v2.offs[1] = hAccm->accmRest.dampedCcOffset[1];
    hAccm->ccmHwConf_v2.offs[2] = hAccm->accmRest.dampedCcOffset[2];

    LOG1_ACCM("%s: (exit)\n", __FUNCTION__);

    return (ret);
}

XCamReturn AccmManualConfig(accm_handle_t hAccm) {
    LOG1_ACCM("%s: (enter)\n", __FUNCTION__);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    memcpy(hAccm->ccmHwConf_v2.matrix, hAccm->mCurAttV2.stManual.ccMatrix,
           sizeof(hAccm->mCurAttV2.stManual.ccMatrix));
    memcpy(hAccm->ccmHwConf_v2.offs, hAccm->mCurAttV2.stManual.ccOffsets,
           sizeof(hAccm->mCurAttV2.stManual.ccOffsets));
    hAccm->ccmHwConf_v2.highy_adj_en = hAccm->mCurAttV2.stManual.highy_adj_en;
    hAccm->ccmHwConf_v2.asym_adj_en  = hAccm->mCurAttV2.stManual.asym_enable;
    hAccm->ccmHwConf_v2.bound_bit    = hAccm->mCurAttV2.stManual.bound_pos_bit;
    hAccm->ccmHwConf_v2.right_bit    = hAccm->mCurAttV2.stManual.right_pos_bit;
    memcpy(hAccm->ccmHwConf_v2.alp_y, hAccm->mCurAttV2.stManual.y_alpha_curve,
           sizeof(hAccm->mCurAttV2.stManual.y_alpha_curve));

    memcpy(hAccm->ccmHwConf_v2.enh_rgb2y_para, hAccm->mCurAttV2.stManual.enh_rgb2y_para,
           sizeof(hAccm->mCurAttV2.stManual.enh_rgb2y_para));
    hAccm->ccmHwConf_v2.enh_adj_en  = hAccm->mCurAttV2.stManual.enh_adj_en;
    hAccm->ccmHwConf_v2.enh_rat_max = hAccm->mCurAttV2.stManual.enh_rat_max;

    LOG1_ACCM("%s: (exit)\n", __FUNCTION__);
    return ret;
}

XCamReturn AccmConfig(accm_handle_t hAccm) {
    LOG1_ACCM("%s: (enter)\n", __FUNCTION__);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    hAccm->update = JudgeCcmRes3aConverge(&hAccm->accmRest.res3a_info, &hAccm->accmSwInfo,
                                          hAccm->ccm_v2->control.gain_tolerance,
                                          hAccm->ccm_v2->control.wbgain_tolerance);

    hAccm->update       = hAccm->update || hAccm->calib_update;
    hAccm->calib_update = false;

    LOGD_ACCM("%s: updateAtt: %d, CCM update: %d, CCM Converged: %d\n", __FUNCTION__,
              hAccm->updateAtt, hAccm->update, hAccm->accmSwInfo.ccmConverged);

    if (hAccm->updateAtt) {
        hAccm->mCurAttV2.mode   = hAccm->mNewAttV2.mode;
        hAccm->mCurAttV2.byPass = hAccm->mNewAttV2.byPass;

        if (hAccm->mCurAttV2.mode == RK_AIQ_CCM_MODE_AUTO)
            hAccm->mCurAttV2.stAuto = hAccm->mNewAttV2.stAuto;
        else
            hAccm->mCurAttV2.stManual = hAccm->mNewAttV2.stManual;
    }

    if (hAccm->mCurAttV2.mode == RK_AIQ_CCM_MODE_AUTO) {
        hAccm->mCurAttV2.byPass = !(hAccm->ccm_v2->control.enable);
    }
    LOGD_ACCM("%s: byPass: %d  mode:%d \n", __FUNCTION__, hAccm->mCurAttV2.byPass,
              hAccm->mCurAttV2.mode);
    if (hAccm->mCurAttV2.byPass != true && hAccm->accmSwInfo.grayMode != true) {
        hAccm->ccmHwConf_v2.ccmEnable = true;

        if (hAccm->mCurAttV2.mode == RK_AIQ_CCM_MODE_AUTO) {
            if (hAccm->updateAtt || hAccm->update || (!hAccm->accmSwInfo.ccmConverged)) {
                AccmAutoConfig(hAccm);
                CCMV2PrintDBG(hAccm);
            }

        } else if (hAccm->mCurAttV2.mode == RK_AIQ_CCM_MODE_MANUAL) {
            if (hAccm->updateAtt || hAccm->update) AccmManualConfig(hAccm);
        } else {
            LOGE_ACCM("%s: hAccm->mCurAttV2.mode(%d) is invalid \n", __FUNCTION__,
                      hAccm->mCurAttV2.mode);
        }
        memcpy(hAccm->mCurAttV2.stManual.ccMatrix, hAccm->ccmHwConf_v2.matrix,
               sizeof(hAccm->ccmHwConf_v2.matrix));
        memcpy(hAccm->mCurAttV2.stManual.ccOffsets, hAccm->ccmHwConf_v2.offs,
               sizeof(hAccm->ccmHwConf_v2.offs));
        memcpy(hAccm->mCurAttV2.stManual.y_alpha_curve, hAccm->ccmHwConf_v2.alp_y,
               sizeof(hAccm->ccmHwConf_v2.alp_y));
        hAccm->mCurAttV2.stManual.enh_adj_en  = hAccm->ccmHwConf_v2.enh_adj_en;
        hAccm->mCurAttV2.stManual.enh_rat_max = hAccm->ccmHwConf_v2.enh_rat_max;

    } else {
        hAccm->ccmHwConf_v2.ccmEnable = false;
    }
    hAccm->updateAtt = false;

    hAccm->count = ((hAccm->count + 2) > (65536)) ? 2 : (hAccm->count + 1);

    CCMV2PrintReg(&hAccm->ccmHwConf_v2);

    LOG1_ACCM("%s: (exit)\n", __FUNCTION__);
    return ret;
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
    const CalibDbV2_Ccm_Para_V32_t* calib_ccm = hAccm->ccm_v2;

    if (!config_calib) {
        return (ret);
    }

#if RKAIQ_ACCM_ILLU_VOTE
    ReloadCCMCalibV2(hAccm, &calib_ccm->TuningPara);
#endif

    ret = pCcmMatrixAll_init(hAccm, &calib_ccm->TuningPara);

    hAccm->mCurAttV2.byPass = !(calib_ccm->control.enable);

    hAccm->ccmHwConf_v2.asym_adj_en = calib_ccm->lumaCCM.asym_enable;

    if (calib_ccm->lumaCCM.asym_enable) {
        hAccm->ccmHwConf_v2.highy_adj_en = true;
        hAccm->ccmHwConf_v2.bound_bit    = calib_ccm->lumaCCM.y_alp_asym.bound_pos_bit;
        hAccm->ccmHwConf_v2.right_bit    = calib_ccm->lumaCCM.y_alp_asym.right_pos_bit;
        int mid                          = CCM_CURVE_DOT_NUM_V2 >> 1;
        for (int i = 0; i < mid; i++) {
            hAccm->ccmHwConf_v2.alp_y[i] = calib_ccm->lumaCCM.y_alp_asym.y_alpha_left_curve[i];
            hAccm->ccmHwConf_v2.alp_y[mid + i] =
                calib_ccm->lumaCCM.y_alp_asym.y_alpha_right_curve[i];
        }
    } else {
        hAccm->ccmHwConf_v2.highy_adj_en = calib_ccm->lumaCCM.y_alp_sym.highy_adj_en;
        hAccm->ccmHwConf_v2.bound_bit    = calib_ccm->lumaCCM.y_alp_sym.bound_pos_bit;
        hAccm->ccmHwConf_v2.right_bit    = hAccm->ccmHwConf_v2.bound_bit;
        memcpy(hAccm->ccmHwConf_v2.alp_y, calib_ccm->lumaCCM.y_alp_sym.y_alpha_curve,
               sizeof(calib_ccm->lumaCCM.y_alp_sym.y_alpha_curve));
        hAccm->ccmHwConf_v2.alp_y[CCM_CURVE_DOT_NUM] = 1024;
    }

    memcpy(hAccm->ccmHwConf_v2.rgb2y_para, calib_ccm->lumaCCM.rgb2y_para,
           sizeof(calib_ccm->lumaCCM.rgb2y_para));

    memcpy(hAccm->ccmHwConf_v2.enh_rgb2y_para, calib_ccm->enhCCM.enh_rgb2y_para,
           sizeof(calib_ccm->enhCCM.enh_rgb2y_para));

    hAccm->ccmHwConf_v2.enh_adj_en  = calib_ccm->enhCCM.enh_ctl.enh_adj_en[0];
    hAccm->ccmHwConf_v2.enh_rat_max = calib_ccm->enhCCM.enh_ctl.enh_rat_max[0];

    // config manual ccm
    memcpy(hAccm->mCurAttV2.stManual.ccMatrix, calib_ccm->manualPara.ccMatrix,
           sizeof(calib_ccm->manualPara.ccMatrix));
    memcpy(hAccm->mCurAttV2.stManual.ccOffsets, calib_ccm->manualPara.ccOffsets,
           sizeof(calib_ccm->manualPara.ccOffsets));
    memcpy(hAccm->mCurAttV2.stManual.y_alpha_curve, hAccm->ccmHwConf_v2.alp_y,
           sizeof(hAccm->ccmHwConf_v2.alp_y));
    hAccm->mCurAttV2.stManual.highy_adj_en  = hAccm->ccmHwConf_v2.highy_adj_en;
    hAccm->mCurAttV2.stManual.asym_enable   = hAccm->ccmHwConf_v2.asym_adj_en;
    hAccm->mCurAttV2.stManual.bound_pos_bit = hAccm->ccmHwConf_v2.bound_bit;
    hAccm->mCurAttV2.stManual.right_pos_bit = hAccm->ccmHwConf_v2.right_bit;
    hAccm->mCurAttV2.stManual.enh_adj_en    = hAccm->ccmHwConf_v2.enh_adj_en;
    hAccm->mCurAttV2.stManual.enh_rat_max   = hAccm->ccmHwConf_v2.enh_rat_max;
    memcpy(hAccm->mCurAttV2.stManual.enh_rgb2y_para, hAccm->ccmHwConf_v2.enh_rgb2y_para,
           sizeof(hAccm->ccmHwConf_v2.enh_rgb2y_para));

    hAccm->accmSwInfo.ccmConverged = false;
    hAccm->calib_update            = true;

    ClearList(&hAccm->accmRest.problist);

    LOG1_ACCM("%s: (exit)\n", __FUNCTION__);
    return (ret);
}

XCamReturn AccmInit(accm_handle_t* hAccm, const CamCalibDbV2Context_t* calibv2) {
    LOGI_ACCM("%s: (enter)\n", __FUNCTION__);

    XCamReturn ret               = XCAM_RETURN_NO_ERROR;

    if (calibv2 == NULL) {
        return XCAM_RETURN_ERROR_PARAM;
    }

    const CalibDbV2_Ccm_Para_V32_t* calib_ccm =
        (CalibDbV2_Ccm_Para_V32_t*)(CALIBDBV2_GET_MODULE_PTR((void*)calibv2, ccm_calib_v2));
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

    accm_context->accmSwInfo.prepare_type =
        RK_AIQ_ALGO_CONFTYPE_UPDATECALIB | RK_AIQ_ALGO_CONFTYPE_NEEDRESET;

    // todo whm --- CalibDbV2_Ccm_Para_V2
    accm_context->ccm_v2 = calib_ccm;
    accm_context->mCurAttV2.mode = RK_AIQ_CCM_MODE_AUTO;
    ret                  = UpdateCcmCalibV2ParaV2(accm_context);

    for (int i = 0; i < RK_AIQ_ACCM_COLOR_GAIN_NUM; i++) {
        accm_context->mCurAttV2.stAuto.color_inhibition.sensorGain[i] = 1;
        accm_context->mCurAttV2.stAuto.color_inhibition.level[i]      = 0;
        accm_context->mCurAttV2.stAuto.color_saturation.sensorGain[i] = 1;
        accm_context->mCurAttV2.stAuto.color_saturation.level[i]      = 50;
    }
    accm_context->accmRest.color_inhibition_level = 0;
    accm_context->accmRest.color_saturation_level = 100;

    LOGI_ACCM("%s: (exit)\n", __FUNCTION__);
    return (ret);
}

XCamReturn AccmRelease(accm_handle_t hAccm) {
    LOGI_ACCM("%s: (enter)\n", __FUNCTION__);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if RKAIQ_ACCM_ILLU_VOTE
    ClearList(&hAccm->accmRest.dominateIlluList);
#endif
    ClearList(&hAccm->accmRest.problist);
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
