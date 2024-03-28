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
#include "xcam_log.h"
#include "interpolation.h"

RKAIQ_BEGIN_DECLARE

void CCMV1PrintReg(const rk_aiq_ccm_cfg_t* hw_param) {
    LOG1_ACCM(
        " CCM V1 reg values: \n"
        " sw_ccm_highy_adjust_dis 0\n"
        " sw_ccm_en_i %d\n"
        " sw_ccm_coeff ([%f,%f,%f,%f,%f,%f,%f,%f,%f]-E)X128\n"
        " sw_ccm_offset [%f,%f,%f]\n"
        " sw_ccm_coeff_y [%f,%f,%f]\n"
        " sw_ccm_alp_y [%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f]\n"
        " sw_ccm_bound_bit %f\n",
        hw_param->ccmEnable,
        hw_param->matrix[0], hw_param->matrix[1], hw_param->matrix[2],
        hw_param->matrix[3], hw_param->matrix[4], hw_param->matrix[5],
        hw_param->matrix[6], hw_param->matrix[7], hw_param->matrix[8],
        hw_param->offs[0],   hw_param->offs[1],   hw_param->offs[2],
        hw_param->rgb2y_para[0], hw_param->rgb2y_para[1], hw_param->rgb2y_para[2],
        hw_param->alp_y[0], hw_param->alp_y[1], hw_param->alp_y[2], hw_param->alp_y[3],
        hw_param->alp_y[4], hw_param->alp_y[5], hw_param->alp_y[6], hw_param->alp_y[7],
        hw_param->alp_y[8], hw_param->alp_y[9], hw_param->alp_y[10],hw_param->alp_y[11],
        hw_param->alp_y[12],hw_param->alp_y[13],hw_param->alp_y[14],hw_param->alp_y[15],
        hw_param->alp_y[16], hw_param->bound_bit);
}

void CCMV1PrintDBG(const accm_context_t* accm_context) {
    const float *pMatrixUndamped = accm_context->accmRest.undampedCcmMatrix;
    const float *pOffsetUndamped = accm_context->accmRest.undampedCcOffset;
    const float *pMatrixDamped = accm_context->ccmHwConf.matrix;
    const float *pOffsetDamped = accm_context->ccmHwConf.offs;

    LOG1_ACCM("Illu Probability Estimation Enable: %d\n"
                "color_inhibition sensorGain: %f,%f,%f,%f \n"
                "color_inhibition level: %f,%f,%f,%f\n"
                "color_saturation sensorGain: %f,%f,%f,%f \n"
                "color_saturation level: %f,%f,%f,%f\n"
                "dampfactor: %f\n"
                " undampedCcmMatrix: %f,%f,%f,%f,%f,%f,%f,%f,%f\n"
                " undampedCcOffset: %f,%f,%f \n"
                " dampedCcmMatrix: %f,%f,%f,%f,%f,%f,%f,%f,%f\n"
                " dampedCcOffset:%f,%f,%f\n",
                accm_context->ccm_v1->TuningPara.illu_estim.interp_enable,
                accm_context->mCurAtt.stAuto.color_inhibition.sensorGain[0],
                accm_context->mCurAtt.stAuto.color_inhibition.sensorGain[1],
                accm_context->mCurAtt.stAuto.color_inhibition.sensorGain[2],
                accm_context->mCurAtt.stAuto.color_inhibition.sensorGain[3],
                accm_context->mCurAtt.stAuto.color_inhibition.level[0],
                accm_context->mCurAtt.stAuto.color_inhibition.level[1],
                accm_context->mCurAtt.stAuto.color_inhibition.level[2],
                accm_context->mCurAtt.stAuto.color_inhibition.level[3],
                accm_context->mCurAtt.stAuto.color_saturation.sensorGain[0],
                accm_context->mCurAtt.stAuto.color_saturation.sensorGain[1],
                accm_context->mCurAtt.stAuto.color_saturation.sensorGain[2],
                accm_context->mCurAtt.stAuto.color_saturation.sensorGain[3],
                accm_context->mCurAtt.stAuto.color_saturation.level[0],
                accm_context->mCurAtt.stAuto.color_saturation.level[1],
                accm_context->mCurAtt.stAuto.color_saturation.level[2],
                accm_context->mCurAtt.stAuto.color_saturation.level[3],
                accm_context->ccm_v1->TuningPara.damp_enable,
                pMatrixUndamped[0], pMatrixUndamped[1], pMatrixUndamped[2],
                pMatrixUndamped[3], pMatrixUndamped[4], pMatrixUndamped[5],
                pMatrixUndamped[6], pMatrixUndamped[7], pMatrixUndamped[8],
                pOffsetUndamped[0], pOffsetUndamped[1], pOffsetUndamped[2],
                pMatrixDamped[0],   pMatrixDamped[1],   pMatrixDamped[2],
                pMatrixDamped[3],   pMatrixDamped[4],   pMatrixDamped[5],
                pMatrixDamped[6],   pMatrixDamped[7],   pMatrixDamped[8],
                pOffsetDamped[0],   pOffsetDamped[1],   pOffsetDamped[2]);

}

XCamReturn AccmAutoConfig
(
    accm_handle_t hAccm
) {

    LOG1_ACCM("%s: (enter)\n", __FUNCTION__);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (hAccm == NULL) {
        return XCAM_RETURN_ERROR_PARAM;
    }

    const CalibDbV2_Ccm_Tuning_Para_t* pCcm = NULL;
    float sensorGain                     = hAccm->accmSwInfo.sensorGain;
    float fSaturation                    = 0;
    bool  updateMat                      = false;
    bool  updateYAlp                     = false;
    bool  updUndampMat                   = false;
    pCcm = &hAccm->ccm_v1->TuningPara;
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
    if (hAccm->update || hAccm->updateAtt) {

        //4) calc scale for y_alpha_curve
        float fScale = 1.0;
    #if 1
        //real use
        interpolation(hAccm->ccm_v1->lumaCCM.gain_alphaScale_curve.gain,
                      hAccm->ccm_v1->lumaCCM.gain_alphaScale_curve.scale,
                      9,
                      sensorGain, &fScale);
    #else
        //for test, to be same with demo
        for( int i = 0; i < 9; i++)
        {
            int j = uint16_t(sensorGain);
            j = (j > (1 << 8)) ? (1 << 8) : j;

            if(j <= (1 << i))
            {
                fScale = hAccm->ccm_v1->lumaCCM.gain_alphaScale_curve.scale[i];
                break;
            }
        }
    #endif
    // 5) color inhibition adjust for api
        interpolation(hAccm->mCurAtt.stAuto.color_inhibition.sensorGain,
                      hAccm->mCurAtt.stAuto.color_inhibition.level,
                      RK_AIQ_ACCM_COLOR_GAIN_NUM,
                      sensorGain, &hAccm->accmRest.color_inhibition_level);

        if(hAccm->accmRest.color_inhibition_level > 100 || hAccm->accmRest.color_inhibition_level < 0) {
            LOGE_ACCM("flevel2: %f is out of range [0 100]\n",  hAccm->accmRest.color_inhibition_level);
            return XCAM_RETURN_ERROR_PARAM;
        }

        fScale *= (100 - hAccm->accmRest.color_inhibition_level) / 100;

    // 6)   saturation adjust for api
        float saturation_level = 100;
        interpolation(hAccm->mCurAtt.stAuto.color_saturation.sensorGain,
                      hAccm->mCurAtt.stAuto.color_saturation.level,
                      RK_AIQ_ACCM_COLOR_GAIN_NUM,
                      sensorGain, &saturation_level );

        if(saturation_level  > 100 || saturation_level  < 0) {
            LOGE_ACCM("flevel1: %f is out of range [0 100]\n",  saturation_level);
            return XCAM_RETURN_ERROR_PARAM;
        }

        LOGD_ACCM("CcmProfile changed: %d, fScale: %f->%f, sat_level: %f->%f",
                  updUndampMat, hAccm->accmRest.fScale, fScale,
                  hAccm->accmRest.color_saturation_level, saturation_level);

        bool flag = updUndampMat ||
                    fabs(fScale - hAccm->accmRest.fScale) > DIVMIN ||
                    fabs(saturation_level - hAccm->accmRest.color_saturation_level) > DIVMIN;
        updUndampMat = false;
        if (flag || (!hAccm->invarMode)) {
            if (flag) {
                hAccm->accmRest.fScale = fScale;
                hAccm->accmRest.color_saturation_level = saturation_level;
                Saturationadjust(fScale, saturation_level, hAccm->accmRest.undampedCcmMatrix);
                LOGD_ACCM("Adjust ccm by sat: %d, undampedCcmMatrix[0]: %f", hAccm->isReCal_, hAccm->accmRest.undampedCcmMatrix[0]);
            }

            updUndampMat = true;
        }
        if (!hAccm->invarMode) {
            hAccm->ccmHwConf.bound_bit = hAccm->ccm_v1->lumaCCM.low_bound_pos_bit;
            memcpy(hAccm->ccmHwConf.rgb2y_para, hAccm->ccm_v1->lumaCCM.rgb2y_para,
                    sizeof(hAccm->ccm_v1->lumaCCM.rgb2y_para));
            updateYAlp = true;
        }
        float iso = sensorGain * 50;
        bool yalp_flag = false;
        YAlpSymAutoCfg(CCM_YALP_ISO_STEP_MAX, hAccm->ccm_v1->lumaCCM.gain_yalp_curve,
                    hAccm->accmRest.yalp_tbl_info.scl,
                    hAccm->accmRest.yalp_tbl_info.tbl_idx,
                    iso, (!hAccm->invarMode || hAccm->calib_update), hAccm->ccmHwConf.alp_y, &yalp_flag);
        updateYAlp = updateYAlp || yalp_flag;
    }
    // 7) . Damping
    float dampCoef = (pCcm->damp_enable && hAccm->count > 1 && hAccm->invarMode > 0) ? hAccm->accmSwInfo.awbIIRDampCoef : 0;
    if (!hAccm->accmSwInfo.ccmConverged || updUndampMat) {
        ret = Damping(dampCoef,
                    hAccm->accmRest.undampedCcmMatrix, hAccm->ccmHwConf.matrix,
                    hAccm->accmRest.undampedCcOffset, hAccm->ccmHwConf.offs,
                    &hAccm->accmSwInfo.ccmConverged);
        updateMat = true;
        LOGD_ACCM("damping: %f, ccm coef[0]: %f->%f, ccm coef[8]: %f->%f \n",
            dampCoef, hAccm->accmRest.undampedCcmMatrix[0], hAccm->ccmHwConf.matrix[0],
            hAccm->accmRest.undampedCcmMatrix[8], hAccm->ccmHwConf.matrix[8]);
    }

    hAccm->isReCal_ = hAccm->isReCal_ || updateMat || updateYAlp;

    LOGD_ACCM("final isReCal_ = %d \n", hAccm->isReCal_);
    LOG1_ACCM("%s: (exit)\n", __FUNCTION__);

    return (ret);
}

XCamReturn AccmManualConfig
(
    accm_handle_t hAccm
) {
    LOG1_ACCM("%s: (enter)\n", __FUNCTION__);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    memcpy(hAccm->ccmHwConf.matrix, hAccm->mCurAtt.stManual.Matrix.ccMatrix, sizeof(hAccm->mCurAtt.stManual.Matrix.ccMatrix));
    memcpy(hAccm->ccmHwConf.offs, hAccm->mCurAtt.stManual.Matrix.ccOffsets, sizeof(hAccm->mCurAtt.stManual.Matrix.ccOffsets));
    memcpy(hAccm->ccmHwConf.alp_y, hAccm->mCurAtt.stManual.YAlp.y_alpha_curve, sizeof(hAccm->mCurAtt.stManual.YAlp.y_alpha_curve));
    hAccm->ccmHwConf.bound_bit = hAccm->mCurAtt.stManual.YAlp.low_bound_pos_bit;
    LOG1_ACCM("%s: (exit)\n", __FUNCTION__);
    return ret;

}

XCamReturn AccmConfig
(
    accm_handle_t hAccm
) {
    LOG1_ACCM("%s: (enter)\n", __FUNCTION__);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    LOGD_ACCM("%s: byPass: %d  mode:%d updateAtt: %d \n", __FUNCTION__,
            hAccm->mCurAtt.byPass, hAccm->mCurAtt.mode, hAccm->updateAtt);
    if (hAccm->mCurAtt.byPass != true && hAccm->accmSwInfo.grayMode != true) {
        hAccm->ccmHwConf.ccmEnable = true;
        if (hAccm->mCurAtt.mode == RK_AIQ_CCM_MODE_AUTO) {
            hAccm->update = JudgeCcmRes3aConverge(&hAccm->accmRest.res3a_info, &hAccm->accmSwInfo,
                                          hAccm->ccm_v1->control.gain_tolerance,
                                          hAccm->ccm_v1->control.wbgain_tolerance);
            hAccm->update = hAccm->update || hAccm->calib_update;
            LOGD_ACCM("%s: CCM update (gain/awbgain/calib): %d, CCM Converged: %d\n",
                    __FUNCTION__, hAccm->update, hAccm->accmSwInfo.ccmConverged);
            if (hAccm->updateAtt || hAccm->update ||(!hAccm->accmSwInfo.ccmConverged)) {
                AccmAutoConfig(hAccm);
                CCMV1PrintDBG(hAccm);
            }
        } else if (hAccm->mCurAtt.mode == RK_AIQ_CCM_MODE_MANUAL) {
            if (hAccm->updateAtt) {
                AccmManualConfig(hAccm);
                hAccm->isReCal_ = true;
            }
        } else {
            LOGE_ACCM("%s: hAccm->mCurAtt.mode(%d) is invalid \n", __FUNCTION__, hAccm->mCurAtt.mode);
        }
    } else {
        hAccm->ccmHwConf.ccmEnable = false;
        // change to graymode / bypass by api/calib
        hAccm->isReCal_ = hAccm->isReCal_ || hAccm->updateAtt || hAccm->calib_update;

    }
    hAccm->updateAtt = false;
    hAccm->calib_update = false;
    hAccm->count = ((hAccm->count + 2) > (65536)) ? 2 : (hAccm->count + 1);

    CCMV1PrintReg(&hAccm->ccmHwConf);

    LOG1_ACCM("%s: (exit)\n", __FUNCTION__);
    return ret;

}

XCamReturn ConfigbyCalib(accm_handle_t hAccm) {
    LOG1_ACCM("%s: (enter)  \n", __FUNCTION__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = pCcmMatrixAll_init(hAccm->ccm_v1->TuningPara.aCcmCof,
                             hAccm->ccm_v1->TuningPara.aCcmCof_len,
                             hAccm->ccm_v1->TuningPara.matrixAll,
                             hAccm->ccm_v1->TuningPara.matrixAll_len,
                             hAccm->pCcmMatrixAll);

    if (hAccm->mCurAtt.mode == RK_AIQ_CCM_MODE_AUTO) {

        hAccm->mCurAtt.byPass = !(hAccm->ccm_v1->control.enable);

        hAccm->ccmHwConf.bound_bit = hAccm->ccm_v1->lumaCCM.low_bound_pos_bit;
        memcpy(hAccm->ccmHwConf.rgb2y_para, hAccm->ccm_v1->lumaCCM.rgb2y_para,
                sizeof(hAccm->ccm_v1->lumaCCM.rgb2y_para));
        memcpy(hAccm->ccmHwConf.alp_y, hAccm->ccm_v1->lumaCCM.gain_yalp_curve[0].y_alpha_curve, sizeof(hAccm->ccmHwConf.alp_y));

        hAccm->accmSwInfo.ccmConverged = false;
        hAccm->calib_update = true;
    }

    hAccm->accmRest.illuNum = hAccm->ccm_v1->TuningPara.aCcmCof_len;

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
static XCamReturn UpdateCcmCalibV2ParaV1(accm_handle_t hAccm)
{
    LOG1_ACCM("%s: (enter)  \n", __FUNCTION__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    bool config_calib = !!(hAccm->accmSwInfo.prepare_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB);
    if (!config_calib)
    {
        return(ret);
    }
    const CalibDbV2_Ccm_Para_V2_t* calib_ccm = hAccm->ccm_v1;

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
    return(ret);
}

static XCamReturn ApiAttrInit(const rk_aiq_ccm_cfg_t*        ccmHwConf,
                              const CalibDbV2_Ccm_Para_V2_t* Calib,
                              rk_aiq_ccm_attrib_t*           mCurAtt)
{
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
    mCurAtt->stManual.YAlp.low_bound_pos_bit = ccmHwConf->bound_bit;

    LOGI_ACCM("%s: (exit)\n", __FUNCTION__);
    return (ret);
}

XCamReturn AccmInit(accm_handle_t *hAccm, const CamCalibDbV2Context_t* calibv2)
{
    LOGI_ACCM("%s: (enter)\n", __FUNCTION__);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if(calibv2 == NULL) {
        return  XCAM_RETURN_ERROR_PARAM;
    }

    CalibDbV2_Ccm_Para_V2_t *calib_ccm =
        (CalibDbV2_Ccm_Para_V2_t*)(CALIBDBV2_GET_MODULE_PTR((void*)calibv2, ccm_calib));
    if (calib_ccm == NULL)
        return XCAM_RETURN_ERROR_MEM;

    *hAccm = (accm_context_t*)malloc(sizeof(accm_context_t));
    accm_context_t* accm_context = *hAccm;
    memset(accm_context, 0, sizeof(accm_context_t));

    accm_context->accmSwInfo.sensorGain = 1.0;
    accm_context->accmSwInfo.awbIIRDampCoef = 0;
    accm_context->accmSwInfo.varianceLuma = 255;
    accm_context->accmSwInfo.awbConverged = false;

    accm_context->accmSwInfo.awbGain[0] = 1;
    accm_context->accmSwInfo.awbGain[1] = 1;

    accm_context->accmRest.res3a_info.sensorGain = 1.0;
    accm_context->accmRest.res3a_info.awbGain[0] = 1.0;
    accm_context->accmRest.res3a_info.awbGain[1] = 1.0;

    accm_context->count = 0;
    accm_context->isReCal_ = 1;
    accm_context->invarMode = 1;

    accm_context->accmSwInfo.prepare_type = RK_AIQ_ALGO_CONFTYPE_UPDATECALIB | RK_AIQ_ALGO_CONFTYPE_NEEDRESET;

    // todo whm --- CalibDbV2_Ccm_Para
    accm_context->ccm_v1 = calib_ccm;
    accm_context->mCurAtt.mode = RK_AIQ_CCM_MODE_AUTO;
#if RKAIQ_ACCM_ILLU_VOTE
    INIT_LIST_HEAD(&accm_context->accmRest.dominateIlluList);
#endif
    INIT_LIST_HEAD(&accm_context->accmRest.problist);
    ret = UpdateCcmCalibV2ParaV1(accm_context);
    ApiAttrInit(&accm_context->ccmHwConf, accm_context->ccm_v1,
                  &accm_context->mCurAtt);

    accm_context->accmRest.fScale = 1;
    accm_context->accmRest.color_inhibition_level = 0;
    accm_context->accmRest.color_saturation_level = 100;

    LOGI_ACCM("%s: (exit)\n", __FUNCTION__);
    return(ret);
}

XCamReturn AccmRelease(accm_handle_t hAccm)
{
    LOGI_ACCM("%s: (enter)\n", __FUNCTION__);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if RKAIQ_ACCM_ILLU_VOTE
    clear_list(&hAccm->accmRest.dominateIlluList);
#endif
    clear_list(&hAccm->accmRest.problist);
    free(hAccm);
    hAccm = NULL;

    LOGI_ACCM("%s: (exit)\n", __FUNCTION__);
    return(ret);

}

// todo whm
XCamReturn AccmPrepare(accm_handle_t hAccm)
{
    LOGI_ACCM("%s: (enter)\n", __FUNCTION__);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = UpdateCcmCalibV2ParaV1(hAccm);
    RETURN_RESULT_IF_DIFFERENT(ret, XCAM_RETURN_NO_ERROR);

    LOGI_ACCM("%s: (exit)\n", __FUNCTION__);
    return ret;
}


XCamReturn AccmPreProc(accm_handle_t hAccm)
{

    LOG1_ACCM("%s: (enter)\n", __FUNCTION__);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    LOG1_ACCM("%s: (exit)\n", __FUNCTION__);
    return(ret);

}
XCamReturn AccmProcessing(accm_handle_t hAccm)
{
    LOG1_ACCM("%s: (enter)\n", __FUNCTION__);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    LOG1_ACCM("%s: (exit)\n", __FUNCTION__);
    return(ret);
}




RKAIQ_END_DECLARE


