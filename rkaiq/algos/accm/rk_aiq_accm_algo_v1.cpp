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
        " CCM V1 reg values: "
        " sw_ccm_highy_adjust_dis 0"
        " sw_ccm_en_i %d"
        " sw_ccm_coeff ([%f,%f,%f,%f,%f,%f,%f,%f,%f]-E)X128"
        " sw_ccm_offset [%f,%f,%f]"
        " sw_ccm_coeff_y [%f,%f,%f]"
        " sw_ccm_alp_y [%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f]"
        " sw_ccm_bound_bit %f",
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
    const CalibDbV2_Ccm_Para_V2_t* pCcm = accm_context->ccm_v1;
    const float *pMatrixUndamped = accm_context->accmRest.undampedCcmMatrix;
    const float *pOffsetUndamped = accm_context->accmRest.undampedCcOffset;
    const float *pMatrixDamped = accm_context->accmRest.dampedCcmMatrix;
    const float *pOffsetDamped = accm_context->accmRest.dampedCcOffset;

    LOG1_ACCM("Illu Probability Estimation Enable: %d"
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
                pCcm->TuningPara.damp_enable,
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

    const CalibDbV2_Ccm_Para_V2_t * pCcm = NULL;
    float sensorGain =  hAccm->accmSwInfo.sensorGain;
    float fSaturation = 0;
    pCcm = hAccm->ccm_v1;
    if (hAccm->update || hAccm->updateAtt) {
        if (pCcm->TuningPara.illu_estim.interp_enable) {
            ret = interpCCMbywbgain(&pCcm->TuningPara, hAccm, fSaturation);
            RETURN_RESULT_IF_DIFFERENT(ret, XCAM_RETURN_NO_ERROR);
        } else {
            ret = selectCCM(&pCcm->TuningPara, hAccm, fSaturation);
            RETURN_RESULT_IF_DIFFERENT(ret, XCAM_RETURN_NO_ERROR);
        }

        //4) calc scale for y_alpha_curve
        float fScale = 1.0;
    #if 1
        //real use
        interpolation(pCcm->lumaCCM.gain_alphaScale_curve.gain,
                    pCcm->lumaCCM.gain_alphaScale_curve.scale,
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
                fScale = pCcm->lumaCCM.gain_alphaScale_curve.scale[i];
                break;
            }
        }
    #endif
    // 5) color inhibition adjust for api
        float flevel2;
        interpolation(hAccm->mCurAtt.stAuto.color_inhibition.sensorGain,
                    hAccm->mCurAtt.stAuto.color_inhibition.level,
                    RK_AIQ_ACCM_COLOR_GAIN_NUM,
                    sensorGain, &flevel2);
        hAccm->accmRest.color_inhibition_level = flevel2;

        if(flevel2 > 100 || flevel2 < 0) {
            LOGE_ACCM("flevel2: %f is out of range [0 100]\n",  flevel2);
            return XCAM_RETURN_ERROR_PARAM;
        }

        fScale *= (100 - flevel2) / 100;

    // 6)   saturation adjust for api
        float flevel1;
        interpolation(hAccm->mCurAtt.stAuto.color_saturation.sensorGain,
                    hAccm->mCurAtt.stAuto.color_saturation.level,
                    RK_AIQ_ACCM_COLOR_GAIN_NUM,
                    sensorGain, &flevel1);

        if(flevel1 > 100 || flevel1 < 0) {
            LOGE_ACCM("flevel1: %f is out of range [0 100]\n",  flevel1);
            return XCAM_RETURN_ERROR_PARAM;
        }
        hAccm->accmRest.color_saturation_level = flevel1;

        LOGD_ACCM("final fScale: %f, color inhibition level: %f, fSaturation: %f, color saturation level: %f\n",
                                                    fScale, flevel2, fSaturation, flevel1);

        Saturationadjust(fScale, hAccm->accmRest.color_saturation_level, hAccm->accmRest.undampedCcmMatrix);

        for(int i = 0; i < CCM_CURVE_DOT_NUM; i++) { //set to ic  to do bit check
            hAccm->ccmHwConf.alp_y[i] = fScale * pCcm->lumaCCM.y_alpha_curve[i];
        }
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
    memcpy(hAccm->ccmHwConf.matrix, hAccm->accmRest.dampedCcmMatrix, sizeof(hAccm->accmRest.dampedCcmMatrix));
    hAccm->ccmHwConf.offs[0] = hAccm->accmRest.dampedCcOffset[0];
    hAccm->ccmHwConf.offs[1] = hAccm->accmRest.dampedCcOffset[1];
    hAccm->ccmHwConf.offs[2] = hAccm->accmRest.dampedCcOffset[2];

    LOG1_ACCM("%s: (exit)\n", __FUNCTION__);

    return (ret);
}

XCamReturn AccmManualConfig
(
    accm_handle_t hAccm
) {
    LOG1_ACCM("%s: (enter)\n", __FUNCTION__);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    memcpy(hAccm->ccmHwConf.matrix, hAccm->mCurAtt.stManual.ccMatrix, sizeof(hAccm->mCurAtt.stManual.ccMatrix));
    memcpy(hAccm->ccmHwConf.offs, hAccm->mCurAtt.stManual.ccOffsets, sizeof(hAccm->mCurAtt.stManual.ccOffsets));
    memcpy(hAccm->ccmHwConf.alp_y, hAccm->mCurAtt.stManual.y_alpha_curve, sizeof(hAccm->mCurAtt.stManual.y_alpha_curve));
    hAccm->ccmHwConf.bound_bit = hAccm->mCurAtt.stManual.low_bound_pos_bit;
    LOG1_ACCM("%s: (exit)\n", __FUNCTION__);
    return ret;

}

XCamReturn AccmConfig
(
    accm_handle_t hAccm
) {
    LOG1_ACCM("%s: (enter)\n", __FUNCTION__);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    hAccm->update = JudgeCcmRes3aConverge(&hAccm->accmRest.res3a_info, &hAccm->accmSwInfo,
                                          hAccm->ccm_v1->control.gain_tolerance,
                                          hAccm->ccm_v1->control.wbgain_tolerance);

    hAccm->update = hAccm->update || hAccm->calib_update;
    hAccm->calib_update = false;

    LOGD_ACCM("%s: updateAtt: %d, CCM update: %d, CCM Converged: %d\n", __FUNCTION__,
                    hAccm->updateAtt, hAccm->update, hAccm->accmSwInfo.ccmConverged);

    if(hAccm->updateAtt) {
        hAccm->mCurAtt.mode = hAccm->mNewAtt.mode;
        hAccm->mCurAtt.byPass = hAccm->mNewAtt.byPass;

        if (hAccm->mCurAtt.mode == RK_AIQ_CCM_MODE_AUTO)
            hAccm->mCurAtt.stAuto = hAccm->mNewAtt.stAuto;
        else
            hAccm->mCurAtt.stManual = hAccm->mNewAtt.stManual;
    }

    LOGD_ACCM("%s: byPass: %d  mode:%d \n", __FUNCTION__, hAccm->mCurAtt.byPass, hAccm->mCurAtt.mode);
    if(hAccm->mCurAtt.byPass != true && hAccm->accmSwInfo.grayMode != true) {
        hAccm->ccmHwConf.ccmEnable = true;

        if (hAccm->mCurAtt.mode == RK_AIQ_CCM_MODE_AUTO) {
            if (hAccm->updateAtt || hAccm->update ||(!hAccm->accmSwInfo.ccmConverged)) {
                AccmAutoConfig(hAccm);
                CCMV1PrintDBG(hAccm);
            }

        } else if (hAccm->mCurAtt.mode == RK_AIQ_CCM_MODE_MANUAL) {
            if (hAccm->updateAtt || hAccm->update) AccmManualConfig(hAccm);
        } else {
            LOGE_ACCM("%s: hAccm->mCurAtt.mode(%d) is invalid \n", __FUNCTION__, hAccm->mCurAtt.mode);
        }
        memcpy(hAccm->mCurAtt.stManual.ccMatrix, hAccm->ccmHwConf.matrix, sizeof(hAccm->ccmHwConf.matrix));
        memcpy(hAccm->mCurAtt.stManual.ccOffsets, hAccm->ccmHwConf.offs, sizeof(hAccm->ccmHwConf.offs));
        memcpy(hAccm->mCurAtt.stManual.y_alpha_curve, hAccm->ccmHwConf.alp_y, sizeof(hAccm->ccmHwConf.alp_y));

    } else {
        hAccm->ccmHwConf.ccmEnable = false;
    }
    hAccm->updateAtt = false;

    hAccm->count = ((hAccm->count + 2) > (65536)) ? 2 : (hAccm->count + 1);

    CCMV1PrintReg(&hAccm->ccmHwConf);

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
static XCamReturn UpdateCcmCalibV2ParaV1(accm_handle_t hAccm)
{
    LOG1_ACCM("%s: (enter)  \n", __FUNCTION__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    bool config_calib = !!(hAccm->accmSwInfo.prepare_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB);
    const CalibDbV2_Ccm_Para_V2_t* calib_ccm = hAccm->ccm_v1;

    if (!config_calib)
    {
        return(ret);
    }

    if (hAccm->mCurAtt.mode == RK_AIQ_CCM_MODE_AUTO) {
#if RKAIQ_ACCM_ILLU_VOTE
        ReloadCCMCalibV2(hAccm, &calib_ccm->TuningPara);
#endif

        ret = pCcmMatrixAll_init(hAccm, &calib_ccm->TuningPara);

        hAccm->mCurAtt.byPass = !(calib_ccm->control.enable);

        hAccm->ccmHwConf.bound_bit = calib_ccm->lumaCCM.low_bound_pos_bit;
        memcpy( hAccm->ccmHwConf.rgb2y_para, calib_ccm->lumaCCM.rgb2y_para,
                sizeof(calib_ccm->lumaCCM.rgb2y_para));
        memcpy( hAccm->ccmHwConf.alp_y, calib_ccm->lumaCCM.y_alpha_curve, sizeof(hAccm->ccmHwConf.alp_y));

        // config manual ccm
        hAccm->mCurAtt.stManual.low_bound_pos_bit = hAccm->ccmHwConf.bound_bit;
        hAccm->accmSwInfo.ccmConverged = false;
        hAccm->calib_update = true;
    }

    ClearList(&hAccm->accmRest.problist);

    LOG1_ACCM("%s: (exit)\n", __FUNCTION__);
    return(ret);
}

XCamReturn AccmInit(accm_handle_t *hAccm, const CamCalibDbV2Context_t* calibv2)
{
    LOGI_ACCM("%s: (enter)\n", __FUNCTION__);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if(calibv2 == NULL) {
        return  XCAM_RETURN_ERROR_PARAM;
    }

    const CalibDbV2_Ccm_Para_V2_t *calib_ccm =
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

    accm_context->accmSwInfo.prepare_type = RK_AIQ_ALGO_CONFTYPE_UPDATECALIB | RK_AIQ_ALGO_CONFTYPE_NEEDRESET;

    // todo whm --- CalibDbV2_Ccm_Para
    accm_context->ccm_v1 = calib_ccm;
    accm_context->mCurAtt.mode = RK_AIQ_CCM_MODE_AUTO;
    ret = UpdateCcmCalibV2ParaV1(accm_context);

    for(int i = 0; i < RK_AIQ_ACCM_COLOR_GAIN_NUM; i++) {
            accm_context->mCurAtt.stAuto.color_inhibition.sensorGain[i] = 1;
            accm_context->mCurAtt.stAuto.color_inhibition.level[i] = 0;
            accm_context->mCurAtt.stAuto.color_saturation.sensorGain[i] = 1;
            accm_context->mCurAtt.stAuto.color_saturation.level[i] = 50;
        }
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
    ClearList(&hAccm->accmRest.dominateIlluList);
#endif
    ClearList(&hAccm->accmRest.problist);
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


