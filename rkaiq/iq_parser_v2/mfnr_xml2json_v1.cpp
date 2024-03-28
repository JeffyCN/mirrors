#include "mfnr_xml2json_v1.h"

#define MFNRV1_SETTING_NUM (2)
#define MFNRV1_ISO_NUM  (13)



int mfnrV1_calibdb_to_calibdbV2(const CalibDb_MFNR_2_t *pCalibdb,  CalibDbV2_MFNR_t *pCalibdbV2, int mode_idx)
{
    CalibDbV2_MFNR_Calib_t *pCalibParaV2 = NULL;
    CalibDbV2_MFNR_Tuning_t *pTuningParaV2 = NULL;
    CalibDbV2_MFNR_C_ISO_t *pCalibISOV2 = NULL;
    CalibDbV2_MFNR_T_ISO_t *pTuningISOV2 = NULL;
    CalibDbV2_MFNR_Motion_ISO_t *pMotionISOV2 = NULL;
    CalibDbV2_MFNR_Dynamic_t *pDynamicV2 = NULL;
    CalibDb_MFNR_Setting_t *pSetting = NULL;
    CalibDb_MFNR_Dynamic_t *pDynamic = NULL;
    CalibDb_MFNR_Motion_t  *pMotion = NULL;
    struct CalibDb_MFNR_ISO_s *pISO = NULL;

    if(pCalibdb == NULL) {
        printf(" pCalibdb is NULL pointer\n");
        return -1;
    }

    if(pCalibdbV2 == NULL) {
        printf(" pCalibdbV2 is NULL pointer\n");
        return -1;
    }

    if(pCalibdb->mode_num < mode_idx) {
        printf(" old xml file have no %d mode cell \n", mode_idx);
        return -1;
    }

    pCalibParaV2 = &pCalibdbV2->CalibPara;
    pTuningParaV2 = &pCalibdbV2->TuningPara;

    //malloc settting size
    pCalibParaV2->Setting = (CalibDbV2_MFNR_C_Set_t *)malloc(MFNRV1_SETTING_NUM * sizeof(CalibDbV2_MFNR_C_Set_t));
    memset(pCalibParaV2->Setting,  0x00, MFNRV1_SETTING_NUM * sizeof(CalibDbV2_MFNR_C_Set_t));
    pCalibParaV2->Setting_len = MFNRV1_SETTING_NUM;
    pTuningParaV2->Setting = (CalibDbV2_MFNR_T_Set_t *)malloc(MFNRV1_SETTING_NUM * sizeof(CalibDbV2_MFNR_T_Set_t));
    memset(pTuningParaV2->Setting, 0x00, MFNRV1_SETTING_NUM * sizeof(CalibDbV2_MFNR_T_Set_t));
    pTuningParaV2->Setting_len = MFNRV1_SETTING_NUM;

    //malloc iso size
    for(int i = 0; i < pTuningParaV2->Setting_len; i++) {
        pCalibParaV2->Setting[i].Calib_ISO =  (CalibDbV2_MFNR_C_ISO_t *)malloc(MFNRV1_ISO_NUM * sizeof(CalibDbV2_MFNR_C_ISO_t));
        memset(pCalibParaV2->Setting[i].Calib_ISO, 0x00, MFNRV1_ISO_NUM * sizeof(CalibDbV2_MFNR_C_ISO_t));
        pCalibParaV2->Setting[i].Calib_ISO_len = MFNRV1_ISO_NUM;
        pTuningParaV2->Setting[i].Tuning_ISO = (CalibDbV2_MFNR_T_ISO_t *)malloc(MFNRV1_ISO_NUM * sizeof(CalibDbV2_MFNR_T_ISO_t));
        memset(pTuningParaV2->Setting[i].Tuning_ISO, 0x00, MFNRV1_ISO_NUM * sizeof(CalibDbV2_MFNR_T_ISO_t));
        pTuningParaV2->Setting[i].Tuning_ISO_len = MFNRV1_ISO_NUM;
    }
    pTuningParaV2->Motion.Motion_ISO = (CalibDbV2_MFNR_Motion_ISO_t *)malloc(MFNRV1_ISO_NUM * sizeof(CalibDbV2_MFNR_Motion_ISO_t));
    memset(pTuningParaV2->Motion.Motion_ISO, 0x00, MFNRV1_ISO_NUM * sizeof(CalibDbV2_MFNR_Motion_ISO_t));
    pTuningParaV2->Motion.Motion_ISO_len = MFNRV1_ISO_NUM;

    //assign the value
    pCalibdbV2->Version = strdup(pCalibdb->version);

    pTuningParaV2->enable = pCalibdb->enable;
    pTuningParaV2->local_gain_en = pCalibdb->local_gain_en;
    pTuningParaV2->mode_3to1 = pCalibdb->mode_3to1;
    pTuningParaV2->motion_detect_en = pCalibdb->motion_detect_en;

    for(int i = 0; i < MFNRV1_SETTING_NUM; i++) {
        pSetting = &pCalibdb->mode_cell[mode_idx].setting[i];

        pCalibParaV2->Setting[i].SNR_Mode = strdup(pSetting->snr_mode);
        pCalibParaV2->Setting[i].Sensor_Mode = strdup(pSetting->sensor_mode);
        pTuningParaV2->Setting[i].SNR_Mode = strdup( pSetting->snr_mode);
        pTuningParaV2->Setting[i].Sensor_Mode = strdup( pSetting->sensor_mode);

        for(int j = 0; j < MFNRV1_ISO_NUM; j++) {
            pSetting = &pCalibdb->mode_cell[mode_idx].setting[i];
            pCalibISOV2 = &pCalibParaV2->Setting[i].Calib_ISO[j];
            pTuningISOV2 = &pTuningParaV2->Setting[i].Tuning_ISO[j];
            pISO = &pSetting->mfnr_iso[j];

            //calib
            pCalibISOV2->iso = pISO->iso;
            pCalibISOV2->noise_curve_x00 = pISO->noise_curve_x00;
            for(int k = 0; k < 5; k++) {
                pCalibISOV2->noise_curve[k] = pISO->noise_curve[k];
            }
            for(int k = 0; k < 4; k++) {
                pCalibISOV2->y_lo_noiseprofile[k] = pISO->y_lo_noiseprofile[k];
                pCalibISOV2->y_hi_noiseprofile[k] = pISO->y_hi_noiseprofile[k];
            }
            for(int k = 0; k < 3; k++) {
                pCalibISOV2->uv_lo_noiseprofile[k] = pISO->uv_lo_noiseprofile[k];
                pCalibISOV2->uv_hi_noiseprofile[k] = pISO->uv_hi_noiseprofile[k];
            }

            //tuning
            pTuningISOV2->iso = pISO->iso;
            pTuningISOV2->weight_limit_y_0 = pISO->weight_limit_y[0];
            pTuningISOV2->weight_limit_y_1 = pISO->weight_limit_y[1];
            pTuningISOV2->weight_limit_y_2 = pISO->weight_limit_y[2];
            pTuningISOV2->weight_limit_y_3 = pISO->weight_limit_y[3];

            pTuningISOV2->weight_limit_uv_0 = pISO->weight_limit_uv[0];
            pTuningISOV2->weight_limit_uv_1 = pISO->weight_limit_uv[1];
            pTuningISOV2->weight_limit_uv_2 = pISO->weight_limit_uv[2];

            pTuningISOV2->ratio_frq_y_l = pISO->ratio_frq[0];
            pTuningISOV2->ratio_frq_y_h = pISO->ratio_frq[1];
            pTuningISOV2->ratio_frq_uv_l = pISO->ratio_frq[2];
            pTuningISOV2->ratio_frq_uv_h = pISO->ratio_frq[3];

            pTuningISOV2->luma_w_in_chroma_0 = pISO->luma_w_in_chroma[0];
            pTuningISOV2->luma_w_in_chroma_1 = pISO->luma_w_in_chroma[1];
            pTuningISOV2->luma_w_in_chroma_2 = pISO->luma_w_in_chroma[2];

            pTuningISOV2->y_lo_bfscale_0 = pISO->y_lo_bfscale[0];
            pTuningISOV2->y_lo_bfscale_1 = pISO->y_lo_bfscale[1];
            pTuningISOV2->y_lo_bfscale_2 = pISO->y_lo_bfscale[2];
            pTuningISOV2->y_lo_bfscale_3 = pISO->y_lo_bfscale[3];

            pTuningISOV2->y_hi_bfscale_0 = pISO->y_hi_bfscale[0];
            pTuningISOV2->y_hi_bfscale_1 = pISO->y_hi_bfscale[1];
            pTuningISOV2->y_hi_bfscale_2 = pISO->y_hi_bfscale[2];
            pTuningISOV2->y_hi_bfscale_3 = pISO->y_hi_bfscale[3];

            pTuningISOV2->y_denoisestrength = pISO->y_denoisestrength;

            pTuningISOV2->uv_lo_bfscale_0 = pISO->uv_lo_bfscale[0];
            pTuningISOV2->uv_lo_bfscale_1 = pISO->uv_lo_bfscale[1];
            pTuningISOV2->uv_lo_bfscale_2 = pISO->uv_lo_bfscale[2];

            pTuningISOV2->uv_hi_bfscale_0 = pISO->uv_hi_bfscale[0];
            pTuningISOV2->uv_hi_bfscale_1 = pISO->uv_hi_bfscale[1];
            pTuningISOV2->uv_hi_bfscale_2 = pISO->uv_hi_bfscale[2];

            pTuningISOV2->uv_denoisestrength = pISO->uv_denoisestrength;

            for(int k = 0; k < 6; k++) {
                pTuningISOV2->luma_para.y_lumanrpoint[k] = pISO->y_lumanrpoint[k];
                pTuningISOV2->luma_para.y_lumanrcurve[k] = pISO->y_lumanrcurve[k];
                pTuningISOV2->luma_para.uv_lumanrpoint[k] = pISO->uv_lumanrpoint[k];
                pTuningISOV2->luma_para.uv_lumanrcurve[k] = pISO->uv_lumanrcurve[k];
            }

            for(int k = 0; k < 6; k++) {
                pTuningISOV2->y_gfdelta_para.y_lo_lvl0_gfdelta[k] = pISO->y_lo_lvl0_gfdelta[k];
                pTuningISOV2->y_gfdelta_para.y_hi_lvl0_gfdelta[k] = pISO->y_hi_lvl0_gfdelta[k];
                pTuningISOV2->uv_gfdelta_para.uv_lo_lvl0_gfdelta[k] = pISO->uv_lo_lvl0_gfdelta[k];
                pTuningISOV2->uv_gfdelta_para.uv_hi_lvl0_gfdelta[k] = pISO->uv_hi_lvl0_gfdelta[k];
                pTuningISOV2->gfsigma_para.lvl0_gfsigma[k] = pISO->lvl0_gfsigma[k];
            }

            for(int k = 0; k < 3; k++) {
                pTuningISOV2->y_gfdelta_para.y_lo_lvl1_gfdelta[k] = pISO->y_lo_lvl1_gfdelta[k];
                pTuningISOV2->y_gfdelta_para.y_hi_lvl1_gfdelta[k] = pISO->y_hi_lvl1_gfdelta[k];
                pTuningISOV2->y_gfdelta_para.y_lo_lvl2_gfdelta[k] = pISO->y_lo_lvl2_gfdelta[k];
                pTuningISOV2->y_gfdelta_para.y_hi_lvl2_gfdelta[k] = pISO->y_hi_lvl2_gfdelta[k];
                pTuningISOV2->y_gfdelta_para.y_lo_lvl3_gfdelta[k] = pISO->y_lo_lvl3_gfdelta[k];
                pTuningISOV2->y_gfdelta_para.y_hi_lvl3_gfdelta[k] = pISO->y_hi_lvl3_gfdelta[k];
            }

            for(int k = 0; k < 3; k++) {
                pTuningISOV2->uv_gfdelta_para.uv_lo_lvl1_gfdelta[k] = pISO->uv_lo_lvl1_gfdelta[k];
                pTuningISOV2->uv_gfdelta_para.uv_hi_lvl1_gfdelta[k] = pISO->uv_hi_lvl1_gfdelta[k];
                pTuningISOV2->uv_gfdelta_para.uv_lo_lvl2_gfdelta[k] = pISO->uv_lo_lvl2_gfdelta[k];
                pTuningISOV2->uv_gfdelta_para.uv_hi_lvl2_gfdelta[k] = pISO->uv_hi_lvl2_gfdelta[k];
            }

            for(int k = 0; k < 3; k++) {
                pTuningISOV2->gfsigma_para.lvl1_gfsigma[k] = pISO->lvl1_gfsigma[k];
                pTuningISOV2->gfsigma_para.lvl2_gfsigma[k] = pISO->lvl2_gfsigma[k];
                pTuningISOV2->gfsigma_para.lvl3_gfsigma[k] = pISO->lvl3_gfsigma[k];
            }
        }
    }


    //motion
    pMotion = &pCalibdb->mode_cell[mode_idx].motion;
    for(int i = 0; i < MFNRV1_ISO_NUM; i++) {
        pMotionISOV2 = &pCalibdbV2->TuningPara.Motion.Motion_ISO[i];
        pMotionISOV2->iso = pMotion->iso[i];
        pMotionISOV2->sigmaHScale = pMotion->sigmaHScale[i];
        pMotionISOV2->sigmaLScale = pMotion->sigmaLScale[i];
        pMotionISOV2->lightClp = pMotion->lightClp[i];
        pMotionISOV2->uvWeight = pMotion->uvWeight[i];
        pMotionISOV2->mfnrSigmaScale = pMotion->mfnrSigmaScale[i];
        pMotionISOV2->yuvnrGainScale0 = pMotion->yuvnrGainScale0[i];
        pMotionISOV2->yuvnrGainScale1 = pMotion->yuvnrGainScale1[i];
        pMotionISOV2->yuvnrGainScale2 = pMotion->yuvnrGainScale2[i];
        pMotionISOV2->frame_limit_y = pMotion->frame_limit_y[i];
        pMotionISOV2->frame_limit_uv = pMotion->frame_limit_uv[i];
    }

    //dynamic
    pDynamic = &pCalibdb->mode_cell[mode_idx].dynamic;
    pDynamicV2 = &pCalibdbV2->TuningPara.Dynamic;
    pDynamicV2->Enable = pDynamic->enable;
    pDynamicV2->LowTh_iso = pDynamic->lowth_iso;
    pDynamicV2->LowTh_time = pDynamic->lowth_time;
    pDynamicV2->HighTh_iso = pDynamic->highth_iso;
    pDynamicV2->HighTh_time = pDynamic->highth_time;

    return 0;

}





int mfnrV1_calibdbV2_to_calibdb(CalibDbV2_MFNR_t *pCalibdbV2,  CalibDb_MFNR_2_t *pCalibdb,  int mode_idx)
{
    CalibDbV2_MFNR_Calib_t *pCalibParaV2 = NULL;
    CalibDbV2_MFNR_Tuning_t *pTuningParaV2 = NULL;
    CalibDbV2_MFNR_C_ISO_t *pCalibISOV2 = NULL;
    CalibDbV2_MFNR_T_ISO_t *pTuningISOV2 = NULL;
    CalibDbV2_MFNR_Motion_ISO_t *pMotionISOV2 = NULL;
    CalibDbV2_MFNR_Dynamic_t *pDynamicV2 = NULL;
    CalibDb_MFNR_Setting_t *pSetting = NULL;
    CalibDb_MFNR_Dynamic_t *pDynamic = NULL;
    CalibDb_MFNR_Motion_t  *pMotion = NULL;
    struct CalibDb_MFNR_ISO_s *pISO = NULL;

    if(pCalibdb == NULL) {
        printf(" pCalibdb is NULL pointer\n");
        return -1;
    }

    if(pCalibdbV2 == NULL) {
        printf(" pCalibdbV2 is NULL pointer\n");
        return -1;
    }

    if(pCalibdb->mode_num < mode_idx) {
        printf(" old xml file have no %d mode cell \n", mode_idx);
        return -1;
    }

    pCalibParaV2 = &pCalibdbV2->CalibPara;
    pTuningParaV2 = &pCalibdbV2->TuningPara;


    //assign the value
    strcpy(pCalibdb->version, pCalibdbV2->Version);

    pCalibdb->enable = pTuningParaV2->enable;
    pCalibdb->local_gain_en = pTuningParaV2->local_gain_en;
    pCalibdb->mode_3to1 = pTuningParaV2->mode_3to1;
    pCalibdb->motion_detect_en = pTuningParaV2->motion_detect_en;

    for(int i = 0; i < pTuningParaV2->Setting_len; i++) {
        pSetting = &pCalibdb->mode_cell[mode_idx].setting[i];

        strcpy(pSetting->snr_mode, pTuningParaV2->Setting[i].SNR_Mode);
        strcpy( pSetting->sensor_mode, pTuningParaV2->Setting[i].Sensor_Mode);

        for(int j = 0; j < MFNRV1_ISO_NUM; j++) {
            pSetting = &pCalibdb->mode_cell[mode_idx].setting[i];
            pCalibISOV2 = &pCalibParaV2->Setting[i].Calib_ISO[j];
            pTuningISOV2 = &pTuningParaV2->Setting[i].Tuning_ISO[j];
            pISO = &pSetting->mfnr_iso[j];

            //calib
            pISO->iso = pCalibISOV2->iso;
            pISO->noise_curve_x00 = pCalibISOV2->noise_curve_x00;
            for(int k = 0; k < 5; k++) {
                pISO->noise_curve[k] = pCalibISOV2->noise_curve[k];
            }
            for(int k = 0; k < 4; k++) {
                pISO->y_lo_noiseprofile[k] = pCalibISOV2->y_lo_noiseprofile[k];
                pISO->y_hi_noiseprofile[k] = pCalibISOV2->y_hi_noiseprofile[k];
            }
            for(int k = 0; k < 3; k++) {
                pISO->uv_lo_noiseprofile[k] = pCalibISOV2->uv_lo_noiseprofile[k];
                pISO->uv_hi_noiseprofile[k] = pCalibISOV2->uv_hi_noiseprofile[k];
            }

            //tuning
            pISO->iso = pTuningISOV2->iso;
            pISO->weight_limit_y[0] = pTuningISOV2->weight_limit_y_0;
            pISO->weight_limit_y[1] = pTuningISOV2->weight_limit_y_1;
            pISO->weight_limit_y[2] = pTuningISOV2->weight_limit_y_2;
            pISO->weight_limit_y[3] = pTuningISOV2->weight_limit_y_3;

            pISO->weight_limit_uv[0] = pTuningISOV2->weight_limit_uv_0;
            pISO->weight_limit_uv[1] = pTuningISOV2->weight_limit_uv_1;
            pISO->weight_limit_uv[2] = pTuningISOV2->weight_limit_uv_2;

            pISO->ratio_frq[0] = pTuningISOV2->ratio_frq_y_l;
            pISO->ratio_frq[1] = pTuningISOV2->ratio_frq_y_h;
            pISO->ratio_frq[2] = pTuningISOV2->ratio_frq_uv_l;
            pISO->ratio_frq[3] = pTuningISOV2->ratio_frq_uv_h;

            pISO->luma_w_in_chroma[0] = pTuningISOV2->luma_w_in_chroma_0;
            pISO->luma_w_in_chroma[1] = pTuningISOV2->luma_w_in_chroma_1;
            pISO->luma_w_in_chroma[2] = pTuningISOV2->luma_w_in_chroma_2;

            pISO->y_lo_bfscale[0] = pTuningISOV2->y_lo_bfscale_0;
            pISO->y_lo_bfscale[1] = pTuningISOV2->y_lo_bfscale_1;
            pISO->y_lo_bfscale[2] = pTuningISOV2->y_lo_bfscale_2;
            pISO->y_lo_bfscale[3] = pTuningISOV2->y_lo_bfscale_3;

            pISO->y_hi_bfscale[0] = pTuningISOV2->y_hi_bfscale_0;
            pISO->y_hi_bfscale[1] = pTuningISOV2->y_hi_bfscale_1;
            pISO->y_hi_bfscale[2] = pTuningISOV2->y_hi_bfscale_2;
            pISO->y_hi_bfscale[3] = pTuningISOV2->y_hi_bfscale_3;

            pISO->y_denoisestrength = pTuningISOV2->y_denoisestrength;

            pISO->uv_lo_bfscale[0] = pTuningISOV2->uv_lo_bfscale_0;
            pISO->uv_lo_bfscale[1] = pTuningISOV2->uv_lo_bfscale_1;
            pISO->uv_lo_bfscale[2] = pTuningISOV2->uv_lo_bfscale_2;

            pISO->uv_hi_bfscale[0] = pTuningISOV2->uv_hi_bfscale_0;
            pISO->uv_hi_bfscale[1] = pTuningISOV2->uv_hi_bfscale_1;
            pISO->uv_hi_bfscale[2] = pTuningISOV2->uv_hi_bfscale_2;

            pISO->uv_denoisestrength = pTuningISOV2->uv_denoisestrength;

            for(int k = 0; k < 6; k++) {
                pISO->y_lumanrpoint[k] = pTuningISOV2->luma_para.y_lumanrpoint[k];
                pISO->y_lumanrcurve[k] = pTuningISOV2->luma_para.y_lumanrcurve[k];
                pISO->uv_lumanrpoint[k] = pTuningISOV2->luma_para.uv_lumanrpoint[k];
                pISO->uv_lumanrcurve[k] = pTuningISOV2->luma_para.uv_lumanrcurve[k];
            }

            for(int k = 0; k < 6; k++) {
                pISO->y_lo_lvl0_gfdelta[k] = pTuningISOV2->y_gfdelta_para.y_lo_lvl0_gfdelta[k];
                pISO->y_hi_lvl0_gfdelta[k] = pTuningISOV2->y_gfdelta_para.y_hi_lvl0_gfdelta[k];
                pISO->uv_lo_lvl0_gfdelta[k] = pTuningISOV2->uv_gfdelta_para.uv_lo_lvl0_gfdelta[k];
                pISO->uv_hi_lvl0_gfdelta[k] = pTuningISOV2->uv_gfdelta_para.uv_hi_lvl0_gfdelta[k];
                pISO->lvl0_gfsigma[k] = pTuningISOV2->gfsigma_para.lvl0_gfsigma[k];
            }

            for(int k = 0; k < 3; k++) {
                pISO->y_lo_lvl1_gfdelta[k] = pTuningISOV2->y_gfdelta_para.y_lo_lvl1_gfdelta[k];
                pISO->y_hi_lvl1_gfdelta[k] = pTuningISOV2->y_gfdelta_para.y_hi_lvl1_gfdelta[k];
                pISO->y_lo_lvl2_gfdelta[k] = pTuningISOV2->y_gfdelta_para.y_lo_lvl2_gfdelta[k];
                pISO->y_hi_lvl2_gfdelta[k] = pTuningISOV2->y_gfdelta_para.y_hi_lvl2_gfdelta[k];
                pISO->y_lo_lvl3_gfdelta[k] = pTuningISOV2->y_gfdelta_para.y_lo_lvl3_gfdelta[k];
                pISO->y_hi_lvl3_gfdelta[k] = pTuningISOV2->y_gfdelta_para.y_hi_lvl3_gfdelta[k];
            }

            for(int k = 0; k < 3; k++) {
                pISO->uv_lo_lvl1_gfdelta[k] = pTuningISOV2->uv_gfdelta_para.uv_lo_lvl1_gfdelta[k];
                pISO->uv_hi_lvl1_gfdelta[k] = pTuningISOV2->uv_gfdelta_para.uv_hi_lvl1_gfdelta[k];
                pISO->uv_lo_lvl2_gfdelta[k] = pTuningISOV2->uv_gfdelta_para.uv_lo_lvl2_gfdelta[k];
                pISO->uv_hi_lvl2_gfdelta[k] = pTuningISOV2->uv_gfdelta_para.uv_hi_lvl2_gfdelta[k];
            }

            for(int k = 0; k < 3; k++) {
                pISO->lvl1_gfsigma[k] = pTuningISOV2->gfsigma_para.lvl1_gfsigma[k];
                pISO->lvl2_gfsigma[k] = pTuningISOV2->gfsigma_para.lvl2_gfsigma[k];
                pISO->lvl3_gfsigma[k] = pTuningISOV2->gfsigma_para.lvl3_gfsigma[k];
            }
        }
    }


    //motion
    pMotion = &pCalibdb->mode_cell[mode_idx].motion;
    pMotion->enable = pTuningParaV2->enable;
    for(int i = 0; i < MFNRV1_ISO_NUM; i++) {
        pMotionISOV2 = &pCalibdbV2->TuningPara.Motion.Motion_ISO[i];
        pMotion->iso[i] = pMotionISOV2->iso;
        pMotion->sigmaHScale[i] = pMotionISOV2->sigmaHScale;
        pMotion->sigmaLScale[i] = pMotionISOV2->sigmaLScale;
        pMotion->lightClp[i] = pMotionISOV2->lightClp;
        pMotion->uvWeight[i] = pMotionISOV2->uvWeight;
        pMotion->mfnrSigmaScale[i] = pMotionISOV2->mfnrSigmaScale;
        pMotion->yuvnrGainScale0[i] = pMotionISOV2->yuvnrGainScale0;
        pMotion->yuvnrGainScale1[i] = pMotionISOV2->yuvnrGainScale1;
        pMotion->yuvnrGainScale2[i] = pMotionISOV2->yuvnrGainScale2;
        pMotion->frame_limit_y[i] = pMotionISOV2->frame_limit_y;
        pMotion->frame_limit_uv[i] = pMotionISOV2->frame_limit_uv;
    }

    //dynamic
    pDynamic = &pCalibdb->mode_cell[mode_idx].dynamic;
    pDynamicV2 = &pCalibdbV2->TuningPara.Dynamic;
    pDynamic->enable = pDynamicV2->Enable;
    pDynamic->lowth_iso = pDynamicV2->LowTh_iso;
    pDynamic->lowth_time = pDynamicV2->LowTh_time;
    pDynamic->highth_iso = pDynamicV2->HighTh_iso;
    pDynamic->highth_time = pDynamicV2->HighTh_time;

    return 0;

}


void mfnrV1_calibdbV2_free(CalibDbV2_MFNR_t *pCalibdbV2)
{
    if(pCalibdbV2) {
        if(pCalibdbV2->Version) {
            free(pCalibdbV2->Version);
        }

        if(pCalibdbV2->CalibPara.Setting) {
            for(int i = 0; i < pCalibdbV2->CalibPara.Setting_len; i++) {
                if(pCalibdbV2->CalibPara.Setting[i].Calib_ISO) {
                    if(pCalibdbV2->CalibPara.Setting[i].Calib_ISO) {
                        free(pCalibdbV2->CalibPara.Setting[i].Calib_ISO);
                    }
                    if(pCalibdbV2->CalibPara.Setting[i].SNR_Mode) {
                        free(pCalibdbV2->CalibPara.Setting[i].SNR_Mode);
                    }
                    if(pCalibdbV2->CalibPara.Setting[i].Sensor_Mode) {
                        free(pCalibdbV2->CalibPara.Setting[i].Sensor_Mode);
                    }
                }
            }
            free(pCalibdbV2->CalibPara.Setting);
        }


        if(pCalibdbV2->TuningPara.Setting) {
            for(int i = 0; i < pCalibdbV2->TuningPara.Setting_len; i++) {
                if(pCalibdbV2->TuningPara.Setting[i].Tuning_ISO) {
                    if(pCalibdbV2->TuningPara.Setting[i].Tuning_ISO) {
                        free(pCalibdbV2->TuningPara.Setting[i].Tuning_ISO);
                    }
                    if(pCalibdbV2->TuningPara.Setting[i].SNR_Mode) {
                        free(pCalibdbV2->TuningPara.Setting[i].SNR_Mode);
                    }
                    if(pCalibdbV2->TuningPara.Setting[i].Sensor_Mode) {
                        free(pCalibdbV2->TuningPara.Setting[i].Sensor_Mode);
                    }
                }
            }
            free(pCalibdbV2->TuningPara.Setting);
        }

        if(pCalibdbV2->TuningPara.Motion.Motion_ISO) {
            free(pCalibdbV2->TuningPara.Motion.Motion_ISO);
        }

        free(pCalibdbV2);
    }
}

