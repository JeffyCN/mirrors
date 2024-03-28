/*
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


#include "ccm_xml2json.h"

void CalibV2CCMFree(CamCalibDbV2Context_t *calibV2)
{
#if RKAIQ_HAVE_CCM_V1
    CalibDbV2_Ccm_Para_V2_t* ccm_v2 =  (CalibDbV2_Ccm_Para_V2_t*)(CALIBDBV2_GET_MODULE_PTR(calibV2, ccm_calib));
#endif
#if RKAIQ_HAVE_CCM_V2
    CalibDbV2_Ccm_Para_V32_t* ccm_v2 =
        (CalibDbV2_Ccm_Para_V32_t*)(CALIBDBV2_GET_MODULE_PTR(calibV2, ccm_calib_v2));
#endif
    if(ccm_v2 ==NULL){
          return;
    }

    for (int i = 0; i < ccm_v2->TuningPara.aCcmCof_len; i++) {
        for (int j = 0; j < ccm_v2->TuningPara.aCcmCof[i].matrixUsed_len; j++)
            free(ccm_v2->TuningPara.aCcmCof[i].matrixUsed[j]);
        free(ccm_v2->TuningPara.aCcmCof[i].matrixUsed);
        free(ccm_v2->TuningPara.aCcmCof[i].name);
    }

    for (int i = 0; i < ccm_v2->TuningPara.matrixAll_len; i++) {
        free(ccm_v2->TuningPara.matrixAll[i].name);
        free(ccm_v2->TuningPara.matrixAll[i].illumination);
    }

    free(ccm_v2->TuningPara.aCcmCof);
    free(ccm_v2->TuningPara.matrixAll);
    free(ccm_v2->TuningPara.illu_estim.default_illu);
}

void convertCCMCalib2CalibV2(const CamCalibDbContext_t *calib,CamCalibDbV2Context_t *calibV2)
{
    const CalibDb_Ccm_t *ccm = (CalibDb_Ccm_t*)CALIBDB_GET_MODULE_PTR((void*)calib, ccm);
    if(ccm == NULL)
        return;
#if RKAIQ_HAVE_CCM_V1
    CalibDbV2_Ccm_Para_V2_t* ccm_v2 =
        (CalibDbV2_Ccm_Para_V2_t*)(CALIBDBV2_GET_MODULE_PTR(calibV2, ccm_calib));
#elif RKAIQ_HAVE_CCM_V2
    CalibDbV2_Ccm_Para_V32_t* ccm_v2 =
        (CalibDbV2_Ccm_Para_V32_t*)(CALIBDBV2_GET_MODULE_PTR(calibV2, ccm_calib_v2));
#else
    CalibDbV2_Ccm_Para_V2_t* ccm_v2 = NULL;
#endif

    if(ccm_v2 == NULL)
        return;

     //malloc
#if RKAIQ_HAVE_CCM_V1
    memset(ccm_v2, 0x00, sizeof(CalibDbV2_Ccm_Para_V2_t));
#endif
#if RKAIQ_HAVE_CCM_V2
    memset(ccm_v2, 0x00, sizeof(CalibDbV2_Ccm_Para_V32_t));
#endif

    ccm_v2->TuningPara.illu_estim.default_illu= (char*)malloc(sizeof(char)*CCM_PROFILE_NAME);
    ccm_v2->TuningPara.aCcmCof_len = ccm->mode_cell[0].aCcmCof.illuNum;
    ccm_v2->TuningPara.aCcmCof =  (CalibDbV2_Ccm_Accm_Cof_Para_t*)malloc(sizeof(CalibDbV2_Ccm_Accm_Cof_Para_t) * ccm_v2->TuningPara.aCcmCof_len);
    for ( int i = 0; i < ccm_v2->TuningPara.aCcmCof_len; i++)
    {
        ccm_v2->TuningPara.aCcmCof[i].name = (char*)malloc(sizeof(char)*CCM_PROFILE_NAME);
        ccm_v2->TuningPara.aCcmCof[i].matrixUsed_len = ccm->mode_cell[0].aCcmCof.illAll[i].matrixUsedNO;
        ccm_v2->TuningPara.aCcmCof[i].matrixUsed = (char**)malloc(sizeof(char*)*CCM_PROFILES_NUM_MAX);
        for (int j = 0; j < ccm_v2->TuningPara.aCcmCof[i].matrixUsed_len; j++)
            ccm_v2->TuningPara.aCcmCof[i].matrixUsed[j] = (char*)malloc(sizeof(char)*CCM_PROFILE_NAME);
    }

    ccm_v2->TuningPara.matrixAll_len = ccm->mode_cell[0].matrixAllNum;
    ccm_v2->TuningPara.matrixAll = (CalibDbV2_Ccm_Matrix_Para_t*)malloc(sizeof(CalibDbV2_Ccm_Matrix_Para_t)*ccm_v2->TuningPara.matrixAll_len);
    for ( int i = 0; i < ccm_v2->TuningPara.matrixAll_len; i++){
        ccm_v2->TuningPara.matrixAll[i].name =  (char*)malloc(sizeof(char)*CCM_PROFILE_NAME);
        ccm_v2->TuningPara.matrixAll[i].illumination = (char*)malloc(sizeof(char)*CCM_ILLUMINATION_NAME);
    }

    //copy value
    ccm_v2->control.enable = ccm->enable;
    ccm_v2->control.gain_tolerance = 0.2;
    ccm_v2->control.wbgain_tolerance = 0.1;

    ccm_v2->TuningPara.damp_enable = ccm->mode_cell[0].damp_enable;
    ccm_v2->TuningPara.illu_estim.interp_enable = 0;
    strcpy(ccm_v2->TuningPara.illu_estim.default_illu, ccm->mode_cell[0].aCcmCof.illAll[0].illuName);
    ccm_v2->TuningPara.illu_estim.weightRB[0] = 1;
    ccm_v2->TuningPara.illu_estim.weightRB[1] = 1;
    ccm_v2->TuningPara.illu_estim.prob_limit = 0.2;
    ccm_v2->TuningPara.illu_estim.frame_no = 8;

#if RKAIQ_HAVE_CCM_V1
    CalibDbV2_Ccm_Luma_Ccm_t *lumaCcm = &ccm_v2->lumaCCM;
    lumaCcm->low_bound_pos_bit = ccm->mode_cell[0].luma_ccm.low_bound_pos_bit;
    memcpy(lumaCcm->rgb2y_para, ccm->mode_cell[0].luma_ccm.rgb2y_para, sizeof(lumaCcm->rgb2y_para));
    memcpy(lumaCcm->y_alpha_curve, ccm->mode_cell[0].luma_ccm.y_alpha_curve, sizeof(lumaCcm->y_alpha_curve));
    memcpy(lumaCcm->gain_alphaScale_curve.gain, ccm->mode_cell[0].luma_ccm.alpha_gain, sizeof(lumaCcm->gain_alphaScale_curve.gain));
    memcpy(lumaCcm->gain_alphaScale_curve.scale, ccm->mode_cell[0].luma_ccm.alpha_scale, sizeof(lumaCcm->gain_alphaScale_curve.gain));
#endif

#if RKAIQ_HAVE_CCM_V2
    CalibDbV2_Ccm_Luma_Ccm_V2_t* lumaCcm = &ccm_v2->lumaCCM;
    lumaCCM->asym_enable                      = 0;
    lumaCCM->y_alp_sym.highy_adj_en           = 1;
    lumaCcm->y_alp_sym.bound_pos_bit          = ccm->mode_cell[0].luma_ccm.low_bound_pos_bit;
    memcpy(lumaCcm->rgb2y_para, ccm->mode_cell[0].luma_ccm.rgb2y_para, sizeof(lumaCcm->rgb2y_para));
    memcpy(lumaCcm->y_alp_sym.y_alpha_curve, ccm->mode_cell[0].luma_ccm.y_alpha_curve,
           sizeof(ccm->mode_cell[0].luma_ccm.y_alpha_curve));
    memcpy(lumaCcm->gain_alphaScale_curve.gain, ccm->mode_cell[0].luma_ccm.alpha_gain,
           sizeof(lumaCcm->gain_alphaScale_curve.gain));
    memcpy(lumaCcm->gain_alphaScale_curve.scale, ccm->mode_cell[0].luma_ccm.alpha_scale,
           sizeof(lumaCcm->gain_alphaScale_curve.gain));
#endif

    for ( int i = 0; i < ccm_v2->TuningPara.aCcmCof_len; i++)
    {
        strcpy(ccm_v2->TuningPara.aCcmCof[i].name, ccm->mode_cell[0].aCcmCof.illAll[i].illuName);
        memcpy(ccm_v2 ->TuningPara.aCcmCof[i].awbGain, ccm->mode_cell[0].aCcmCof.illAll[i].awbGain, sizeof(ccm_v2 ->TuningPara.aCcmCof[i].awbGain));
        ccm_v2->TuningPara.aCcmCof[i].minDist = 0.05;
        for (int j = 0; j < ccm_v2->TuningPara.aCcmCof[i].matrixUsed_len; j++)
            strcpy(ccm_v2->TuningPara.aCcmCof[i].matrixUsed[j], ccm->mode_cell[0].aCcmCof.illAll[i].matrixUsed[j]);
        memcpy(ccm_v2->TuningPara.aCcmCof[i].gain_sat_curve.gains, ccm->mode_cell[0].aCcmCof.illAll[i].saturationCurve.pSensorGain,
                                            sizeof(ccm_v2->TuningPara.aCcmCof[i].gain_sat_curve.gains));
        memcpy(ccm_v2->TuningPara.aCcmCof[i].gain_sat_curve.sat, ccm->mode_cell[0].aCcmCof.illAll[i].saturationCurve.pSaturation,
                                            sizeof(ccm_v2->TuningPara.aCcmCof[i].gain_sat_curve.sat));
    }

    for ( int i = 0; i < ccm_v2->TuningPara.matrixAll_len; i++)
    {
        strcpy(ccm_v2->TuningPara.matrixAll[i].name, ccm->mode_cell[0].matrixAll[i].name);
        strcpy(ccm_v2->TuningPara.matrixAll[i].illumination, ccm->mode_cell[0].matrixAll[i].illumination);
        ccm_v2->TuningPara.matrixAll[i].saturation = ccm->mode_cell[0].matrixAll[i].saturation;
        memcpy(ccm_v2->TuningPara.matrixAll[i].ccMatrix, ccm->mode_cell[0].matrixAll[i].CrossTalkCoeff.fCoeff, sizeof(Cam3x3FloatMatrix_t));
        memcpy(ccm_v2->TuningPara.matrixAll[i].ccOffsets, ccm->mode_cell[0].matrixAll[i].CrossTalkOffset.fCoeff, sizeof(Cam1x3FloatMatrix_t));
    }
}


