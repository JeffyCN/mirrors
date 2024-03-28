
#include "rk_aiq_amfnr_algo_mfnr_v1.h"

#define AMFNRV1_SIGMA_X_SHIFT_BITS (0)


//RKAIQ_BEGIN_DECLARE
Amfnr_Result_V1_t mfnr_get_mode_cell_idx_by_name_v1(CalibDb_MFNR_2_t *pCalibdb, const char *name, int *mode_idx)
{
    int i = 0;
    Amfnr_Result_V1_t res = AMFNR_RET_V1_SUCCESS;

    if(pCalibdb == NULL || name == NULL || mode_idx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AMFNR_RET_V1_NULL_POINTER;
    }


    if(pCalibdb->mode_num < 1) {
        LOGE_ANR("%s(%d): mfnr mode cell num is zero\n", __FUNCTION__, __LINE__);
        return AMFNR_RET_V1_NULL_POINTER;
    }

    for(i = 0; i < pCalibdb->mode_num; i++) {
        if(strncmp(name, pCalibdb->mode_cell[i].name, sizeof(pCalibdb->mode_cell[i].name)) == 0) {
            break;
        }
    }

    if(i < pCalibdb->mode_num) {
        *mode_idx = i;
        res = AMFNR_RET_V1_SUCCESS;
    } else {
        *mode_idx = 0;
        res = AMFNR_RET_V1_FAILURE;
    }

    LOGD_ANR("%s:%d mode_name:%s  mode_idx:%d i:%d \n", __FUNCTION__, __LINE__, name, *mode_idx, i);
    return res;

}


Amfnr_Result_V1_t mfnr_get_setting_idx_by_name_v1(CalibDb_MFNR_2_t *pCalibdb, char *name, int mode_idx, int *setting_idx)
{
    int i = 0;
    Amfnr_Result_V1_t res = AMFNR_RET_V1_SUCCESS;

    if(pCalibdb == NULL || name == NULL || setting_idx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AMFNR_RET_V1_NULL_POINTER;
    }

    for(i = 0; i < CALIBDB_NR_SHARP_SETTING_LEVEL; i++) {
        if(strncmp(name, pCalibdb->mode_cell[mode_idx].setting[i].snr_mode, sizeof(pCalibdb->mode_cell[mode_idx].setting[i].snr_mode)) == 0) {
            break;
        }
    }

    if(i < CALIBDB_NR_SHARP_SETTING_LEVEL) {
        *setting_idx = i;
        res = AMFNR_RET_V1_SUCCESS;
    } else {
        *setting_idx = 0;
        res = AMFNR_RET_V1_FAILURE;
    }

    LOGD_ANR("%s:%d snr_name:%s  snr_idx:%d i:%d \n", __FUNCTION__, __LINE__, name, *setting_idx, i);
    return res;

}

Amfnr_Result_V1_t init_mfnr_dynamic_params_v1(RK_MFNR_Dynamic_V1_t *pDynamic, CalibDb_MFNR_2_t *pCalibdb, int mode_idx)
{
    Amfnr_Result_V1_t res = AMFNR_RET_V1_SUCCESS;

    if(pDynamic == NULL || pCalibdb == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AMFNR_RET_V1_NULL_POINTER;
    }

    pDynamic->enable = pCalibdb->mode_cell[mode_idx].dynamic.enable;
    pDynamic->lowth_iso = pCalibdb->mode_cell[mode_idx].dynamic.lowth_iso;
    pDynamic->lowth_time = pCalibdb->mode_cell[mode_idx].dynamic.lowth_time;
    pDynamic->highth_iso = pCalibdb->mode_cell[mode_idx].dynamic.highth_iso;
    pDynamic->highth_time = pCalibdb->mode_cell[mode_idx].dynamic.highth_time;

    LOGD_ANR("dynamic final param mode:%d \n", mode_idx);
    return res;

}

Amfnr_Result_V1_t mfnr_config_dynamic_param_v1(RK_MFNR_Dynamic_V1_t *pDynamic,  CalibDb_MFNR_2_t *pCalibdb, char* param_mode)
{

    Amfnr_Result_V1_t res = AMFNR_RET_V1_SUCCESS;
    int mode_idx = 0;

    if(pDynamic == NULL || pCalibdb == NULL || param_mode == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AMFNR_RET_V1_NULL_POINTER;
    }

    res = mfnr_get_mode_cell_idx_by_name_v1(pCalibdb, param_mode, &mode_idx);
    if(res != AMFNR_RET_V1_SUCCESS) {
        LOGW_ANR("%s(%d): error!!!	can't find mode name in iq files, use 0 instead\n", __FUNCTION__, __LINE__);
    }

    res = init_mfnr_dynamic_params_v1(pDynamic, pCalibdb, mode_idx);

    LOGD_ANR("final param mode:%d snr_mode:%d\n", mode_idx);
    return res;

}

Amfnr_Result_V1_t mfnr_config_motion_param_json_v1(CalibDb_MFNR_Motion_t *pMotion,  CalibDbV2_MFNR_t *pCalibdb, char* param_mode)
{

    Amfnr_Result_V1_t res = AMFNR_RET_V1_SUCCESS;
    int mode_idx = 0;
    CalibDbV2_MFNR_Motion_ISO_t *pMotion_ISO = NULL;

    if(pMotion == NULL || pCalibdb == NULL || param_mode == NULL ) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AMFNR_RET_V1_NULL_POINTER;
    }

    pMotion->enable = pCalibdb->TuningPara.motion_detect_en;

    for(int i = 0; i < pCalibdb->TuningPara.Motion.Motion_ISO_len; i++) {
        pMotion_ISO = &pCalibdb->TuningPara.Motion.Motion_ISO[i];

        pMotion->iso[i] = pMotion_ISO->iso;
        pMotion->sigmaHScale[i] = pMotion_ISO->sigmaHScale;
        pMotion->sigmaLScale[i] = pMotion_ISO->sigmaLScale;
        pMotion->lightClp[i] = pMotion_ISO->lightClp;
        pMotion->uvWeight[i] = pMotion_ISO->uvWeight;

        pMotion->mfnrSigmaScale[i] = pMotion_ISO->mfnrSigmaScale;
        pMotion->yuvnrGainScale0[i] = pMotion_ISO->yuvnrGainScale0;
        pMotion->yuvnrGainScale1[i] = pMotion_ISO->yuvnrGainScale1;
        pMotion->yuvnrGainScale2[i] = pMotion_ISO->yuvnrGainScale2;

        pMotion->frame_limit_uv[i] = pMotion_ISO->frame_limit_uv;
        pMotion->frame_limit_y[i] = pMotion_ISO->frame_limit_y;

        pMotion->reserved0[i] = 1;
        pMotion->reserved1[i] = 1;
        pMotion->reserved2[i] = 1;
        pMotion->reserved3[i] = 1;
        pMotion->reserved4[i] = 1;
        pMotion->reserved5[i] = 1;
        pMotion->reserved6[i] = 1;
        pMotion->reserved7[i] = 1;
    }


    LOGD_ANR("final param mode:%d snr_mode:%d\n", mode_idx);
    return res;

}


Amfnr_Result_V1_t mfnr_config_setting_param_v1(RK_MFNR_Params_V1_t *pParams, CalibDb_MFNR_2_t *pCalibdb, char* param_mode, char* snr_name)
{
    Amfnr_Result_V1_t res = AMFNR_RET_V1_SUCCESS;
    int mode_idx = 0;
    int setting_idx = 0;

    if(pParams == NULL || pCalibdb == NULL || param_mode == NULL || snr_name == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AMFNR_RET_V1_NULL_POINTER;
    }

    res = mfnr_get_mode_cell_idx_by_name_v1(pCalibdb, param_mode, &mode_idx);
    if(res != AMFNR_RET_V1_SUCCESS) {
        LOGW_ANR("%s(%d): error!!!	can't find mode name in iq files, use 0 instead\n", __FUNCTION__, __LINE__);
    }

    res = mfnr_get_setting_idx_by_name_v1(pCalibdb, snr_name, mode_idx, &setting_idx);
    if(res != AMFNR_RET_V1_SUCCESS) {
        LOGW_ANR("%s(%d): error!!!  can't find setting in iq files, use 0 instead\n", __FUNCTION__, __LINE__);
    }

    res = init_mfnr_params_v1(pParams, pCalibdb, mode_idx, setting_idx);

    LOGD_ANR("final param mode:%d snr_mode:%d\n", mode_idx, setting_idx);
    return res;

}

Amfnr_Result_V1_t init_mfnr_params_v1(RK_MFNR_Params_V1_t *pParams, CalibDb_MFNR_2_t *pCalibdb, int mode_idx, int setting_idx)
{
    Amfnr_Result_V1_t res = AMFNR_RET_V1_SUCCESS;
    int i = 0;
    int j = 0;
    int step_y =  (1 << (Y_CALIBRATION_BITS - 8));
    int range_sample = 1 << (Y_CALIBRATION_BITS - AMFNRV1_SIGMA_X_SHIFT_BITS);
    int step_sample = 1 << AMFNRV1_SIGMA_X_SHIFT_BITS;

    if(pParams == NULL || pCalibdb == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AMFNR_RET_V1_NULL_POINTER;
    }

    int max_iso_step        = MFNR_MAX_ISO_STEP_V1;
    int polyorder           = MFNR_POLYORDER;
    int max_lvl             = MFNR_MAX_LVL;
    int max_lvl_uv          = MFNR_MAX_LVL_UV;
    int lumancurve_step     = LUMANRCURVE_STEP;
    int range               = 1 << Y_CALIBRATION_BITS;
    int dir_lo              = DIR_LO;
    int dir_hi              = DIR_HI;


    max_lvl = pCalibdb->max_level;
    max_lvl_uv = pCalibdb->max_level_uv;
    pParams->back_ref_num = pCalibdb->back_ref_num;

    CalibDb_MFNR_Setting_t *pSetting = &pCalibdb->mode_cell[mode_idx].setting[setting_idx];
    for(int lvl = 0; lvl < max_lvl; lvl++) {
        for (i = 0; i < max_iso_step; i++)
            pParams->weight_limit_y[i][lvl] = pSetting->mfnr_iso[i].weight_limit_y[lvl];
    }

    for(int lvl = 0; lvl < max_lvl_uv; lvl++) {
        for (i = 0; i < max_iso_step; i++)
            pParams->weight_limit_uv[i][lvl] = pSetting->mfnr_iso[i].weight_limit_uv[lvl];
    }

    for(int j = 0; j < 4; j++) {
        for (i = 0; i < max_iso_step; i++)
            pParams->ratio_frq[i][j] = pSetting->mfnr_iso[i].ratio_frq[j];
    }

    for(int lvl = 0; lvl < max_lvl_uv; lvl++) {
        for (i = 0; i < max_iso_step; i++)
            pParams->luma_w_in_chroma[i][lvl] = pSetting->mfnr_iso[i].luma_w_in_chroma[lvl];
    }

    for(j = 0; j < 4; j++) {
        for(i = 0; i < 2; i++)
            pParams->awb_uv_ratio[j][i] = pCalibdb->uv_ratio[j].ratio[i];
    }

    for(int j = 0; j < polyorder + 1; j++) {
        for (i = 0; i < max_iso_step; i++)
            pParams->curve[i][j] = pSetting->mfnr_iso[i].noise_curve[j];
    }

    for (i = 0; i < max_iso_step; i++) {
        pParams->curve_x0[i] = pSetting->mfnr_iso[i].noise_curve_x00;
    }


    for (j = 0; j < max_lvl; j++) {
        for (i = 0; i < max_iso_step; i++) {
            pParams->ci[i][0][j] = pSetting->mfnr_iso[i].y_lo_noiseprofile[j];
            pParams->ci[i][1][j] = pSetting->mfnr_iso[i].y_hi_noiseprofile[j];
            pParams->scale[i][0][j] = pSetting->mfnr_iso[i].y_lo_bfscale[j];
            pParams->scale[i][1][j] = pSetting->mfnr_iso[i].y_hi_bfscale[j];
        }
    }

    for (j = 0; j < lumancurve_step; j++) {
        for (i = 0; i < max_iso_step; i++) {
            pParams->lumanrpoint[i][dir_lo][j] = pSetting->mfnr_iso[i].y_lumanrpoint[j];
            pParams->lumanrcurve[i][dir_lo][j] = pSetting->mfnr_iso[i].y_lumanrcurve[j];
            pParams->lumanrpoint[i][dir_hi][j] = pSetting->mfnr_iso[i].y_lumanrpoint[j];
            pParams->lumanrcurve[i][dir_hi][j] = pSetting->mfnr_iso[i].y_lumanrcurve[j];
        }
    }

    for (i = 0; i < max_iso_step; i++) {
        pParams->dnstr[i][dir_lo] = pSetting->mfnr_iso[i].y_denoisestrength;
        pParams->dnstr[i][dir_hi] = pParams->dnstr[i][dir_lo];
    }


    for(int j = 0; j < 6; j++) {
        for (i = 0; i < max_iso_step; i++) {
            pParams->gfdelta[i][0][0][j] = pSetting->mfnr_iso[i].y_lo_lvl0_gfdelta[j];
            pParams->gfdelta[i][1][0][j] = pSetting->mfnr_iso[i].y_hi_lvl0_gfdelta[j];
        }
    }

    for(int j = 0; j < 3; j++) {
        for (i = 0; i < max_iso_step; i++) {
            pParams->gfdelta[i][0][1][j] = pSetting->mfnr_iso[i].y_lo_lvl1_gfdelta[j];
            pParams->gfdelta[i][0][2][j] = pSetting->mfnr_iso[i].y_lo_lvl2_gfdelta[j];
            pParams->gfdelta[i][0][3][j] = pSetting->mfnr_iso[i].y_lo_lvl3_gfdelta[j];

            pParams->gfdelta[i][1][1][j] = pSetting->mfnr_iso[i].y_hi_lvl1_gfdelta[j];
            pParams->gfdelta[i][1][2][j] = pSetting->mfnr_iso[i].y_hi_lvl2_gfdelta[j];
            pParams->gfdelta[i][1][3][j] = pSetting->mfnr_iso[i].y_hi_lvl3_gfdelta[j];
        }
    }

    for (j = 0; j < max_lvl_uv; j++) {
        for (i = 0; i < max_iso_step; i++) {
            pParams->ci_uv[i][0][j] = pSetting->mfnr_iso[i].uv_lo_noiseprofile[j];
            pParams->ci_uv[i][1][j] = pSetting->mfnr_iso[i].uv_hi_noiseprofile[j];
            pParams->scale_uv[i][0][j] = pSetting->mfnr_iso[i].uv_lo_bfscale[j];
            pParams->scale_uv[i][1][j] = pSetting->mfnr_iso[i].uv_hi_bfscale[j];

            LOGI_ANR("j:%d i:%d ci:%f %f scale:%f %f\n", j, i,
                     pParams->ci_uv[i][0][j],
                     pParams->ci_uv[i][1][j],
                     pParams->scale_uv[i][0][j],
                     pParams->scale_uv[i][1][j]);
        }
    }

    for (j = 0; j < lumancurve_step; j++) {
        for (i = 0; i < max_iso_step; i++) {
            pParams->lumanrpoint_uv[i][dir_lo][j] = pSetting->mfnr_iso[i].uv_lumanrpoint[j];
            pParams->lumanrcurve_uv[i][dir_lo][j] = pSetting->mfnr_iso[i].uv_lumanrcurve[j];
            pParams->lumanrpoint_uv[i][dir_hi][j] = pSetting->mfnr_iso[i].uv_lumanrpoint[j];
            pParams->lumanrcurve_uv[i][dir_hi][j] = pSetting->mfnr_iso[i].uv_lumanrcurve[j];
        }
    }

    for (i = 0; i < max_iso_step; i++) {
        pParams->dnstr_uv[i][dir_lo] = pSetting->mfnr_iso[i].uv_denoisestrength;
        pParams->dnstr_uv[i][dir_hi] = pParams->dnstr_uv[i][dir_lo];
        LOGI_ANR("%d: dnstr_uv:%f %f\n", i,
                 pParams->dnstr_uv[i][dir_lo],
                 pParams->dnstr_uv[i][dir_hi]);
    }

    for(int j = 0; j < 6; j++) {
        for (i = 0; i < max_iso_step; i++) {
            pParams->gfdelta_uv[i][0][0][j] = pSetting->mfnr_iso[i].uv_lo_lvl0_gfdelta[j];
            pParams->gfdelta_uv[i][1][0][j] = pSetting->mfnr_iso[i].uv_hi_lvl0_gfdelta[j];
        }
    }

    for(int j = 0; j < 3; j++) {
        for (i = 0; i < max_iso_step; i++) {
            pParams->gfdelta_uv[i][0][1][j] = pSetting->mfnr_iso[i].uv_lo_lvl1_gfdelta[j];
            pParams->gfdelta_uv[i][0][2][j] = pSetting->mfnr_iso[i].uv_lo_lvl2_gfdelta[j];
            pParams->gfdelta_uv[i][1][1][j] = pSetting->mfnr_iso[i].uv_hi_lvl1_gfdelta[j];
            pParams->gfdelta_uv[i][1][2][j] = pSetting->mfnr_iso[i].uv_hi_lvl2_gfdelta[j];
        }
    }


    for(int j = 0; j < 6; j++) {
        for (i = 0; i < max_iso_step; i++) {
            pParams->gfsigma[i][0][j] = pSetting->mfnr_iso[i].lvl0_gfsigma[j];
        }
    }

    for(int j = 0; j < 3; j++) {
        for (i = 0; i < max_iso_step; i++) {
            pParams->gfsigma[i][1][j] = pSetting->mfnr_iso[i].lvl1_gfsigma[j];
            pParams->gfsigma[i][2][j] = pSetting->mfnr_iso[i].lvl2_gfsigma[j];
            pParams->gfsigma[i][3][j] = pSetting->mfnr_iso[i].lvl3_gfsigma[j];
        }
    }

    for (i = 0; i < max_iso_step; i++) {
        int64_t curve_x0 = (int)pParams->curve_x0[i];
        for (j = 0; j < range; j++) {
            int64_t X[5];
            double y;

            X[0] = 1;
            y = pParams->curve[i][polyorder];
            for(int order = 1; order < polyorder + 1; order++) {
                X[order] = X[order - 1] * j;
                y = y + (double)X[order] * pParams->curve[i][polyorder - order];
            }

            if(j > curve_x0)
                pParams->noise_sigma[i][j] = pParams->noise_sigma[i][curve_x0];
            else
                pParams->noise_sigma[i][j] = y;
        }

        for(j = 0; j < range_sample; j++) {
            pParams->noise_sigma[i][j] = pParams->noise_sigma[i][j * step_sample] / step_y;
        }
        for(j = range_sample; j < range; j++) {
            pParams->noise_sigma[i][j] = 0;
        }
        pParams->curve_x0[i] = pParams->curve_x0[i] / step_sample;
    }

#ifndef RK_SIMULATOR_HW
    pParams->motion_detection_enable = pCalibdb->mode_cell[mode_idx].motion.enable & pCalibdb->motion_detect_en;
    for(int j = 0; j < MFNR_MAX_ISO_STEP_V1; j++)
    {
        pParams->mfnr_sigma_scale[j] = pCalibdb->mode_cell[mode_idx].motion.mfnrSigmaScale[j];
    }
#endif

#ifndef RK_SIMULATOR_HW
    for (i = 0; i < max_iso_step; i++) {
        pParams->iso[i] = pSetting->mfnr_iso[i].iso;
    }
#endif

    LOGD_ANR("oyyf mfnr iso50: lbfscale:%f hbfscale:%f strength:%f %f\n",
             pParams->scale[0][0][0],
             pParams->scale[0][1][0],
             pParams->dnstr[0][0],
             pParams->dnstr[0][1]);

    //memcpy(pParams->mfnr_ver_char, pCalibdb->version, sizeof(pParams->mfnr_ver_char));
    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);

    return res;
}


Amfnr_Result_V1_t mfnr_get_setting_idx_by_name_json_v1(CalibDbV2_MFNR_t *pCalibdb, char *name, int* calib_idx, int *tuning_idx)
{
    int i = 0;
    Amfnr_Result_V1_t res = AMFNR_RET_V1_SUCCESS;

    if(pCalibdb == NULL || name == NULL || calib_idx == NULL || tuning_idx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AMFNR_RET_V1_NULL_POINTER;
    }

    for(i = 0; i < pCalibdb->TuningPara.Setting_len; i++) {
        if(strncmp(name, pCalibdb->TuningPara.Setting[i].SNR_Mode, strlen(name)*sizeof(char)) == 0) {
            break;
        }
    }

    if(i < pCalibdb->TuningPara.Setting_len) {
        *tuning_idx = i;
    } else {
        *tuning_idx = 0;
    }

    for(i = 0; i < pCalibdb->CalibPara.Setting_len; i++) {
        if(strncmp(name, pCalibdb->CalibPara.Setting[i].SNR_Mode, strlen(name)*sizeof(char)) == 0) {
            break;
        }
    }

    if(i < pCalibdb->CalibPara.Setting_len) {
        *calib_idx = i;
    } else {
        *calib_idx = 0;
    }

    LOGD_ANR("%s:%d snr_name:%s  snr_idx:%d i:%d \n", __FUNCTION__, __LINE__, name, *calib_idx, i);
    return res;

}


Amfnr_Result_V1_t mfnr_config_dynamic_param_json_v1(RK_MFNR_Dynamic_V1_t *pDynamic,  CalibDbV2_MFNR_t *pCalibdb, char* param_mode)
{

    Amfnr_Result_V1_t res = AMFNR_RET_V1_SUCCESS;

    if(pDynamic == NULL || pCalibdb == NULL || param_mode == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AMFNR_RET_V1_NULL_POINTER;
    }

    res = init_mfnr_dynamic_params_json_v1(pDynamic, pCalibdb);

    return res;

}

Amfnr_Result_V1_t init_mfnr_params_json_v1(RK_MFNR_Params_V1_t *pParams, CalibDbV2_MFNR_t *pCalibdb, int calib_idx, int tuning_idx)
{
    Amfnr_Result_V1_t res = AMFNR_RET_V1_SUCCESS;
    int i = 0;
    int j = 0;
    int step_y =  (1 << (Y_CALIBRATION_BITS - 8));
    int range_sample = 1 << (Y_CALIBRATION_BITS - AMFNRV1_SIGMA_X_SHIFT_BITS);
    int step_sample = 1 << AMFNRV1_SIGMA_X_SHIFT_BITS;

    if(pParams == NULL || pCalibdb == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AMFNR_RET_V1_NULL_POINTER;
    }

    int polyorder           = MFNR_POLYORDER;
    int max_lvl             = MFNR_MAX_LVL;
    int max_lvl_uv          = MFNR_MAX_LVL_UV;
    int lumancurve_step     = LUMANRCURVE_STEP;
    int range               = 1 << Y_CALIBRATION_BITS;
    int dir_lo              = DIR_LO;
    int dir_hi              = DIR_HI;


    max_lvl = 4;
    max_lvl_uv = 3;
    pParams->back_ref_num = 0;

    CalibDbV2_MFNR_T_Set_t *pTuningSetting = &pCalibdb->TuningPara.Setting[tuning_idx];
    CalibDbV2_MFNR_C_Set_t *pCalibSetting = &pCalibdb->CalibPara.Setting[calib_idx];
    CalibDbV2_MFNR_T_ISO_t *pTuningISO = NULL;
    CalibDbV2_MFNR_C_ISO_t *pCalibISO = NULL;

#ifndef RK_SIMULATOR_HW
    pParams->motion_detection_enable = pCalibdb->TuningPara.motion_detect_en;
#endif

    for (i = 0; i < pTuningSetting->Tuning_ISO_len; i++) {
        pTuningISO = &pTuningSetting->Tuning_ISO[i];

#ifndef RK_SIMULATOR_HW
        pParams->iso[i] = pTuningISO->iso;
#endif

        pParams->weight_limit_y[i][0] = pTuningISO->weight_limit_y_0;
        pParams->weight_limit_y[i][1] = pTuningISO->weight_limit_y_1;
        pParams->weight_limit_y[i][2] = pTuningISO->weight_limit_y_2;
        pParams->weight_limit_y[i][3] = pTuningISO->weight_limit_y_3;

        pParams->weight_limit_uv[i][0] = pTuningISO->weight_limit_uv_0;
        pParams->weight_limit_uv[i][1] = pTuningISO->weight_limit_uv_1;
        pParams->weight_limit_uv[i][2] = pTuningISO->weight_limit_uv_2;

        pParams->ratio_frq[i][0] = pTuningISO->ratio_frq_y_l;
        pParams->ratio_frq[i][1] = pTuningISO->ratio_frq_y_h;
        pParams->ratio_frq[i][2] = pTuningISO->ratio_frq_uv_l;
        pParams->ratio_frq[i][3] = pTuningISO->ratio_frq_uv_h;

        pParams->luma_w_in_chroma[i][0] = pTuningISO->luma_w_in_chroma_0;
        pParams->luma_w_in_chroma[i][1] = pTuningISO->luma_w_in_chroma_1;
        pParams->luma_w_in_chroma[i][2] = pTuningISO->luma_w_in_chroma_2;

        pParams->scale[i][0][0] = pTuningISO->y_lo_bfscale_0;
        pParams->scale[i][0][1] = pTuningISO->y_lo_bfscale_1;
        pParams->scale[i][0][2] = pTuningISO->y_lo_bfscale_2;
        pParams->scale[i][0][3] = pTuningISO->y_lo_bfscale_3;

        pParams->scale[i][1][0] = pTuningISO->y_hi_bfscale_0;
        pParams->scale[i][1][1] = pTuningISO->y_hi_bfscale_1;
        pParams->scale[i][1][2] = pTuningISO->y_hi_bfscale_2;
        pParams->scale[i][1][3] = pTuningISO->y_hi_bfscale_3;

        for (j = 0; j < lumancurve_step; j++) {
            pParams->lumanrpoint[i][dir_lo][j] = pTuningISO->luma_para.y_lumanrpoint[j];
            pParams->lumanrcurve[i][dir_lo][j] = pTuningISO->luma_para.y_lumanrcurve[j];
            pParams->lumanrpoint[i][dir_hi][j] = pTuningISO->luma_para.y_lumanrpoint[j];
            pParams->lumanrcurve[i][dir_hi][j] = pTuningISO->luma_para.y_lumanrcurve[j];

            pParams->lumanrpoint_uv[i][dir_lo][j] = pTuningISO->luma_para.uv_lumanrpoint[j];
            pParams->lumanrcurve_uv[i][dir_lo][j] = pTuningISO->luma_para.uv_lumanrcurve[j];
            pParams->lumanrpoint_uv[i][dir_hi][j] = pTuningISO->luma_para.uv_lumanrpoint[j];
            pParams->lumanrcurve_uv[i][dir_hi][j] = pTuningISO->luma_para.uv_lumanrcurve[j];
        }

        pParams->dnstr[i][dir_lo] = pTuningISO->y_denoisestrength;
        pParams->dnstr[i][dir_hi] = pParams->dnstr[i][dir_lo];

        pParams->dnstr_uv[i][dir_lo] = pTuningISO->uv_denoisestrength;
        pParams->dnstr_uv[i][dir_hi] = pParams->dnstr_uv[i][dir_lo];

        pParams->scale_uv[i][0][0] = pTuningISO->uv_lo_bfscale_0;
        pParams->scale_uv[i][0][1] = pTuningISO->uv_lo_bfscale_1;
        pParams->scale_uv[i][0][2] = pTuningISO->uv_lo_bfscale_2;

        pParams->scale_uv[i][1][0] = pTuningISO->uv_hi_bfscale_0;
        pParams->scale_uv[i][1][1] = pTuningISO->uv_hi_bfscale_1;
        pParams->scale_uv[i][1][2] = pTuningISO->uv_hi_bfscale_2;


        for(int j = 0; j < 6; j++) {
            pParams->gfdelta[i][0][0][j] = pTuningISO->y_gfdelta_para.y_lo_lvl0_gfdelta[j];
            pParams->gfdelta[i][1][0][j] = pTuningISO->y_gfdelta_para.y_hi_lvl0_gfdelta[j];
            pParams->gfdelta_uv[i][0][0][j] = pTuningISO->uv_gfdelta_para.uv_lo_lvl0_gfdelta[j];
            pParams->gfdelta_uv[i][1][0][j] = pTuningISO->uv_gfdelta_para.uv_hi_lvl0_gfdelta[j];
            pParams->gfsigma[i][0][j] = pTuningISO->gfsigma_para.lvl0_gfsigma[j];
        }

        for(int j = 0; j < 3; j++) {
            pParams->gfdelta[i][0][1][j] = pTuningISO->y_gfdelta_para.y_lo_lvl1_gfdelta[j];
            pParams->gfdelta[i][0][2][j] = pTuningISO->y_gfdelta_para.y_lo_lvl2_gfdelta[j];
            pParams->gfdelta[i][0][3][j] = pTuningISO->y_gfdelta_para.y_lo_lvl3_gfdelta[j];

            pParams->gfdelta[i][1][1][j] = pTuningISO->y_gfdelta_para.y_hi_lvl1_gfdelta[j];
            pParams->gfdelta[i][1][2][j] = pTuningISO->y_gfdelta_para.y_hi_lvl2_gfdelta[j];
            pParams->gfdelta[i][1][3][j] = pTuningISO->y_gfdelta_para.y_hi_lvl3_gfdelta[j];

            pParams->gfdelta_uv[i][0][1][j] = pTuningISO->uv_gfdelta_para.uv_lo_lvl1_gfdelta[j];
            pParams->gfdelta_uv[i][0][2][j] = pTuningISO->uv_gfdelta_para.uv_lo_lvl2_gfdelta[j];
            pParams->gfdelta_uv[i][1][1][j] = pTuningISO->uv_gfdelta_para.uv_hi_lvl1_gfdelta[j];
            pParams->gfdelta_uv[i][1][2][j] = pTuningISO->uv_gfdelta_para.uv_hi_lvl2_gfdelta[j];

            pParams->gfsigma[i][1][j] = pTuningISO->gfsigma_para.lvl1_gfsigma[j];
            pParams->gfsigma[i][2][j] = pTuningISO->gfsigma_para.lvl2_gfsigma[j];
            pParams->gfsigma[i][3][j] = pTuningISO->gfsigma_para.lvl3_gfsigma[j];
        }

#ifndef RK_SIMULATOR_HW
        pParams->mfnr_sigma_scale[i] = pCalibdb->TuningPara.Motion.Motion_ISO[i].mfnrSigmaScale;
#endif

    }


    for (i = 0; i < pCalibSetting->Calib_ISO_len; i++) {
        pCalibISO = &pCalibSetting->Calib_ISO[i];
        for(int j = 0; j < polyorder + 1; j++) {
            pParams->curve[i][j] = pCalibISO->noise_curve[j];
        }
        pParams->curve_x0[i] = pCalibISO->noise_curve_x00;

        for (j = 0; j < max_lvl; j++) {
            pParams->ci[i][0][j] = pCalibISO->y_lo_noiseprofile[j];
            pParams->ci[i][1][j] = pCalibISO->y_hi_noiseprofile[j];
        }

        for (j = 0; j < max_lvl_uv; j++) {
            pParams->ci_uv[i][0][j] = pCalibISO->uv_lo_noiseprofile[j];
            pParams->ci_uv[i][1][j] = pCalibISO->uv_hi_noiseprofile[j];
        }
    }

    for(j = 0; j < 4; j++) {
        for(i = 0; i < 2; i++)
            pParams->awb_uv_ratio[j][i] = 0;
    }



    for (i = 0; i < pTuningSetting->Tuning_ISO_len; i++) {
        int64_t curve_x0 = (int)pParams->curve_x0[i];
        for (j = 0; j < range; j++) {
            int64_t X[5];
            double y;

            X[0] = 1;
            y = pParams->curve[i][polyorder];
            for(int order = 1; order < polyorder + 1; order++) {
                X[order] = X[order - 1] * j;
                y = y + (double)X[order] * pParams->curve[i][polyorder - order];
            }

            if(j > curve_x0)
                pParams->noise_sigma[i][j] = pParams->noise_sigma[i][curve_x0];
            else
                pParams->noise_sigma[i][j] = y;
        }
        for(j = 0; j < range_sample; j++) {
            pParams->noise_sigma[i][j] = pParams->noise_sigma[i][j * step_sample] / step_y;
        }
        for(j = range_sample; j < range; j++) {
            pParams->noise_sigma[i][j] = 0;
        }
        pParams->curve_x0[i] = pParams->curve_x0[i] / step_sample;
    }


    LOGD_ANR("oyyf mfnr iso50: lbfscale:%f hbfscale:%f strength:%f %f\n",
             pParams->scale[0][0][0],
             pParams->scale[0][1][0],
             pParams->dnstr[0][0],
             pParams->dnstr[0][1]);

    //memcpy(pParams->mfnr_ver_char, pCalibdb->Version, sizeof(pParams->mfnr_ver_char));
    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);

    mfnr_algo_param_printf_v1(pParams);

    return res;
}

Amfnr_Result_V1_t mfnr_algo_param_printf_v1(RK_MFNR_Params_V1_t *pParams)
{
    int i, j;

    if(pParams != NULL) {

        for(i = 0; i < MFNR_MAX_ISO_STEP_V1; i++) {
#ifndef RK_SIMULATOR_HW
            LOGD_ANR("mfnr: ISO:%f\n", pParams->iso[i]);
#endif

            LOGD_ANR("noise_curve: %lf %lf %lf %lf %lf\n",
                     pParams->curve[i][0],
                     pParams->curve[i][1],
                     pParams->curve[i][2],
                     pParams->curve[i][3],
                     pParams->curve[i][4]);

            LOGD_ANR("y_lo_noiseprofile: %f %f %f %f\n",
                     pParams->ci[i][0][0],
                     pParams->ci[i][0][1],
                     pParams->ci[i][0][2],
                     pParams->ci[i][0][3]);

            LOGD_ANR("Y_hi_noiseprofile: %f %f %f %f\n",
                     pParams->ci[i][1][0],
                     pParams->ci[i][1][1],
                     pParams->ci[i][1][2],
                     pParams->ci[i][1][3]);

            LOGD_ANR("uv_lo_noiseprofile: %f %f %f \n",
                     pParams->ci_uv[i][0][0],
                     pParams->ci_uv[i][0][1],
                     pParams->ci_uv[i][0][2]);

            LOGD_ANR("uv_hi_noiseprofile: %f %f %f \n",
                     pParams->ci_uv[i][1][0],
                     pParams->ci_uv[i][1][1],
                     pParams->ci_uv[i][1][2]);

            LOGD_ANR("Y_weight_limit: %d %d %d %d\n",
                     pParams->weight_limit_y[i][0],
                     pParams->weight_limit_y[i][1],
                     pParams->weight_limit_y[i][2],
                     pParams->weight_limit_y[i][3]);

            LOGD_ANR("uv_weight_limit: %d %d %d \n",
                     pParams->weight_limit_uv[i][0],
                     pParams->weight_limit_uv[i][1],
                     pParams->weight_limit_uv[i][2]);

            LOGD_ANR("ratio_frq: %f %f %f %f\n",
                     pParams->ratio_frq[i][0],
                     pParams->ratio_frq[i][1],
                     pParams->ratio_frq[i][2],
                     pParams->ratio_frq[i][3]);

            LOGD_ANR("luma_w_in_chroma: %f %f %f \n",
                     pParams->luma_w_in_chroma[i][0],
                     pParams->luma_w_in_chroma[i][1],
                     pParams->luma_w_in_chroma[i][2]);

            LOGD_ANR("y_lo_scale: %f %f %f %f\n",
                     pParams->scale[i][0][0],
                     pParams->scale[i][0][1],
                     pParams->scale[i][0][2],
                     pParams->scale[i][0][3]);

            LOGD_ANR("Y_hi_scale: %f %f %f %f\n",
                     pParams->scale[i][1][0],
                     pParams->scale[i][1][1],
                     pParams->scale[i][1][2],
                     pParams->scale[i][1][3]);

            LOGD_ANR("y_denoiseStrength: %f  uv_denoiseStrength:%f \n",
                     pParams->dnstr[i][0],
                     pParams->dnstr_uv[i][0]);


            LOGD_ANR("uv_lo_scale: %f %f %f\n",
                     pParams->scale_uv[i][0][0],
                     pParams->scale_uv[i][0][1],
                     pParams->scale_uv[i][0][2]);

            LOGD_ANR("uv_hi_scale: %f %f %f\n",
                     pParams->scale_uv[i][1][0],
                     pParams->scale_uv[i][1][1],
                     pParams->scale_uv[i][1][2]);


            for (j = 0; j < 6; j++) {
                LOGD_ANR("y luma: %f %f   \n",
                         pParams->lumanrpoint[i][0][j],
                         pParams->lumanrcurve[i][0][j]);
            }

            for (j = 0; j < 6; j++) {
                LOGD_ANR("uv luma: %f %f   \n",
                         pParams->lumanrpoint_uv[i][0][j],
                         pParams->lumanrcurve_uv[i][0][j]);
            }

        }
    }

    return AMFNR_RET_V1_SUCCESS;
}

Amfnr_Result_V1_t init_mfnr_dynamic_params_json_v1(RK_MFNR_Dynamic_V1_t *pDynamic, CalibDbV2_MFNR_t *pCalibdb)
{
    Amfnr_Result_V1_t res = AMFNR_RET_V1_SUCCESS;

    if(pDynamic == NULL || pCalibdb == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AMFNR_RET_V1_NULL_POINTER;
    }

    pDynamic->enable = pCalibdb->TuningPara.Dynamic.Enable;
    pDynamic->lowth_iso = pCalibdb->TuningPara.Dynamic.LowTh_iso;
    pDynamic->lowth_time = pCalibdb->TuningPara.Dynamic.LowTh_time;
    pDynamic->highth_iso = pCalibdb->TuningPara.Dynamic.HighTh_iso;
    pDynamic->highth_time = pCalibdb->TuningPara.Dynamic.HighTh_time;

    //LOGD_ANR("dynamic final param mode:%d \n", mode_idx);
    return res;

}

Amfnr_Result_V1_t mfnr_config_setting_param_json_v1(RK_MFNR_Params_V1_t *pParams, CalibDbV2_MFNR_t *pCalibdb, char* param_mode, char* snr_name)
{
    Amfnr_Result_V1_t res = AMFNR_RET_V1_SUCCESS;
    int calib_idx = 0;
    int setting_idx = 0;

    if(pParams == NULL || pCalibdb == NULL || param_mode == NULL || snr_name == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AMFNR_RET_V1_NULL_POINTER;
    }

    res = mfnr_get_setting_idx_by_name_json_v1(pCalibdb, snr_name, &calib_idx, &setting_idx);
    if(res != AMFNR_RET_V1_SUCCESS) {
        LOGW_ANR("%s(%d): error!!!  can't find setting in iq files, use 0 instead\n", __FUNCTION__, __LINE__);
    }

    res = init_mfnr_params_json_v1(pParams, pCalibdb, calib_idx, setting_idx);

    LOGD_ANR("final param mode:%d snr_mode:%d\n", calib_idx, setting_idx);
    return res;

}


Amfnr_Result_V1_t select_mfnr_params_by_ISO_v1(RK_MFNR_Params_V1_t *stmfnrParams, RK_MFNR_Params_V1_Select_t *stmfnrParamsSelected, Amfnr_ExpInfo_V1_t *pExpInfo, int bits_proc)
{
    Amfnr_Result_V1_t res = AMFNR_RET_V1_SUCCESS;
    int iso = 50;

    if(stmfnrParams == NULL || stmfnrParamsSelected == NULL || pExpInfo == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AMFNR_RET_V1_NULL_POINTER;
    }

    if(pExpInfo->mfnr_mode_3to1) {
        iso = pExpInfo->preIso[pExpInfo->hdr_mode];
    } else {
        iso = pExpInfo->arIso[pExpInfo->hdr_mode];
    }

    int i, j;
    int iso_low = iso, iso_high = iso;
    int gain_high = 0, gain_low = 0;
    double ratio = 0.0f;
    int iso_div             = 50;
    int dir_num             = MFNR_DIR_NUM;
    int polyorder           = MFNR_POLYORDER;
    int max_lvl             = MFNR_MAX_LVL;
    int max_lvl_uv          = MFNR_MAX_LVL_UV;
    int lumancurve_step     = LUMANRCURVE_STEP;
    int range               = (1 << (Y_CALIBRATION_BITS - AMFNRV1_SIGMA_X_SHIFT_BITS));
    int step                = (1 << (Y_CALIBRATION_BITS - bits_proc - AMFNRV1_SIGMA_X_SHIFT_BITS));
    int step_x              = (1 << (Y_CALIBRATION_BITS - Y_SIGMA_TABLE_BITS - AMFNRV1_SIGMA_X_SHIFT_BITS));
    int step_x1             = (1 << (Y_CALIBRATION_BITS - bits_proc - AMFNRV1_SIGMA_X_SHIFT_BITS));
    int step_y              = (1 << (Y_CALIBRATION_BITS - bits_proc));
    int step_x_dehz         = (1 << (Y_CALIBRATION_BITS - Y_SIGMA_TABLE_BITS_DEHAZE - AMFNRV1_SIGMA_X_SHIFT_BITS));
    double noise_sigma_tmp[(1 << (Y_CALIBRATION_BITS - AMFNRV1_SIGMA_X_SHIFT_BITS)) + 1];

#ifndef RK_SIMULATOR_HW
    for (i = 0; i < MFNR_MAX_ISO_STEP_V1 - 1; i++) {
        if(iso >= stmfnrParams->iso[i] && iso <= stmfnrParams->iso[i + 1]) {
            iso_low = stmfnrParams->iso[i];
            iso_high = stmfnrParams->iso[i + 1];
            gain_low = i;
            gain_high = i + 1;
            ratio = (double)(iso - iso_low) / (iso_high - iso_low);
            break;
        }
    }

    if(i == MFNR_MAX_ISO_STEP_V1 - 1) {
        if(iso < stmfnrParams->iso[0]) {
            iso_low = stmfnrParams->iso[0];
            iso_high = stmfnrParams->iso[1];
            gain_low = 0;
            gain_high = 1;
            ratio = 0;
        }

        if(iso > stmfnrParams->iso[MFNR_MAX_ISO_STEP_V1 - 1]) {
            iso_low = stmfnrParams->iso[MFNR_MAX_ISO_STEP_V1 - 2];
            iso_high = stmfnrParams->iso[MFNR_MAX_ISO_STEP_V1 - 1];
            gain_low = MFNR_MAX_ISO_STEP_V1 - 2;
            gain_high = MFNR_MAX_ISO_STEP_V1 - 1;
            ratio = 1;
        }
    }
#else
    for (i = MFNR_MAX_ISO_STEP_V1 - 1; i >= 0; i--)
    {
        if (iso < iso_div * (2 << i))
        {
            iso_low = iso_div * (2 << (i)) / 2;
            iso_high = iso_div * (2 << i);
        }
    }
    ratio = (double)(iso - iso_low) / (iso_high - iso_low);
    if (iso_low == iso)
    {
        iso_high = iso;
        ratio = 0;
    }
    if (iso_high == iso )
    {
        iso_low = iso;
        ratio = 1;
    }
    gain_high       = (int)(log((double)iso_high / 50) / log((double)2));
    gain_low        = (int)(log((double)iso_low / 50) / log((double)2));
#endif


    LOGD_ANR("%s:%d iso:%d high:%d low:%d ratio:%f \n",
             __FUNCTION__, __LINE__,
             iso, iso_high, iso_low, ratio);


    stmfnrParamsSelected->back_ref_num              = stmfnrParams->back_ref_num;
    for(int lvl = 0; lvl < max_lvl; lvl++)
        stmfnrParamsSelected->weight_limit_y[lvl]   = ratio * (stmfnrParams->weight_limit_y[gain_high][lvl]   - stmfnrParams->weight_limit_y[gain_low][lvl])   + stmfnrParams->weight_limit_y[gain_low][lvl];;
    for(int lvl = 0; lvl < max_lvl_uv; lvl++)
        stmfnrParamsSelected->weight_limit_uv[lvl]  = ratio * (stmfnrParams->weight_limit_uv[gain_high][lvl]  - stmfnrParams->weight_limit_uv[gain_low][lvl])  + stmfnrParams->weight_limit_uv[gain_low][lvl];;
    for(int i = 0; i < 4; i++)
        stmfnrParamsSelected->ratio_frq[i]          = ratio * (stmfnrParams->ratio_frq[gain_high][i]          - stmfnrParams->ratio_frq[gain_low][i])          + stmfnrParams->ratio_frq[gain_low][i];

    for(int lvl = 0; lvl < max_lvl_uv; lvl++)
        stmfnrParamsSelected->luma_w_in_chroma[lvl] = ratio * (stmfnrParams->luma_w_in_chroma[gain_high][lvl] - stmfnrParams->luma_w_in_chroma[gain_low][lvl]) + stmfnrParams->luma_w_in_chroma[gain_low][lvl];
    for (int dir_idx = 0; dir_idx < dir_num; dir_idx++)
        for (j = 0; j < max_lvl; j++)
        {
            stmfnrParamsSelected->ci[dir_idx][j]            = ratio * (1 / stmfnrParams->ci[gain_high][dir_idx][j] - 1 / stmfnrParams->ci[gain_low][dir_idx][j]) + 1 / stmfnrParams->ci[gain_low][dir_idx][j];
        }
    for (int dir_idx = 0; dir_idx < dir_num; dir_idx++)
        for (j = 0; j < max_lvl; j++)
        {
            stmfnrParamsSelected->dnweight[dir_idx][j]  = ratio * (stmfnrParams->dnweight[gain_high][dir_idx][j] - stmfnrParams->dnweight[gain_low][dir_idx][j]) + stmfnrParams->dnweight[gain_low][dir_idx][j];
        }

    for (int dir_idx = 0; dir_idx < dir_num; dir_idx++)
        for (j = 0; j < max_lvl; j++)
        {
            stmfnrParamsSelected->scale[dir_idx][j]         = ratio * (stmfnrParams->scale[gain_high][dir_idx][j] - stmfnrParams->scale[gain_low][dir_idx][j]) + stmfnrParams->scale[gain_low][dir_idx][j];
        }
    for (int dir_idx = 0; dir_idx < dir_num; dir_idx++)
        for (j = 0; j < lumancurve_step; j++)
        {
            stmfnrParamsSelected->lumanrpoint[dir_idx][j]   = ratio * (stmfnrParams->lumanrpoint[gain_high][dir_idx][j] - stmfnrParams->lumanrpoint[gain_low][dir_idx][j]) + stmfnrParams->lumanrpoint[gain_low][dir_idx][j];
            stmfnrParamsSelected->lumanrcurve[dir_idx][j]   = ratio * (stmfnrParams->lumanrcurve[gain_high][dir_idx][j] - stmfnrParams->lumanrcurve[gain_low][dir_idx][j]) + stmfnrParams->lumanrcurve[gain_low][dir_idx][j];
        }
    for (int dir_idx = 0; dir_idx < dir_num; dir_idx++)
        stmfnrParamsSelected->dnstr[dir_idx]                = ratio * (stmfnrParams->dnstr[gain_high][dir_idx] - stmfnrParams->dnstr[gain_low][dir_idx]) + stmfnrParams->dnstr[gain_low][dir_idx];
    for (int dir_idx = 0; dir_idx < dir_num; dir_idx++)
        for (int lvl = 0; lvl < max_lvl; lvl++)
        {
            for(int i = 0; i < MFNR_MAX_GAUS_SIZE; i++)
                stmfnrParamsSelected->gfdelta[dir_idx][lvl][i]  = ratio * (stmfnrParams->gfdelta[gain_high][dir_idx][lvl][i] - stmfnrParams->gfdelta[gain_low][dir_idx][lvl][i]) + stmfnrParams->gfdelta[gain_low][dir_idx][lvl][i];
        }

    for (int dir_idx = 0; dir_idx < dir_num; dir_idx++)
        for (j = 0; j < max_lvl_uv; j++)
        {
            stmfnrParamsSelected->ci_uv[dir_idx][j]         = ratio * (1 / stmfnrParams->ci_uv[gain_high][dir_idx][j] - 1 / stmfnrParams->ci_uv[gain_low][dir_idx][j]) + 1 / stmfnrParams->ci_uv[gain_low][dir_idx][j];
            stmfnrParamsSelected->dnweight_uv[dir_idx][j]   = ratio * (stmfnrParams->dnweight_uv[gain_high][dir_idx][j] - stmfnrParams->dnweight_uv[gain_low][dir_idx][j]) + stmfnrParams->dnweight_uv[gain_low][dir_idx][j];
            stmfnrParamsSelected->scale_uv[dir_idx][j]      = ratio * (stmfnrParams->scale_uv[gain_high][dir_idx][j] - stmfnrParams->scale_uv[gain_low][dir_idx][j]) + stmfnrParams->scale_uv[gain_low][dir_idx][j];
        }
    for (int dir_idx = 0; dir_idx < dir_num; dir_idx++)
        for (j = 0; j < lumancurve_step; j++)
        {
            stmfnrParamsSelected->lumanrpoint_uv[dir_idx][j]    = ratio * (stmfnrParams->lumanrpoint_uv[gain_high][dir_idx][j] - stmfnrParams->lumanrpoint_uv[gain_low][dir_idx][j]) + stmfnrParams->lumanrpoint_uv[gain_low][dir_idx][j];
            stmfnrParamsSelected->lumanrcurve_uv[dir_idx][j]    = ratio * (stmfnrParams->lumanrcurve_uv[gain_high][dir_idx][j] - stmfnrParams->lumanrcurve_uv[gain_low][dir_idx][j]) + stmfnrParams->lumanrcurve_uv[gain_low][dir_idx][j];
        }
    for (int dir_idx = 0; dir_idx < dir_num; dir_idx++)
        stmfnrParamsSelected->dnstr_uv[dir_idx]                 = ratio * (stmfnrParams->dnstr_uv[gain_high][dir_idx] - stmfnrParams->dnstr_uv[gain_low][dir_idx]) + stmfnrParams->dnstr_uv[gain_low][dir_idx];

    for (int dir_idx = 0; dir_idx < dir_num; dir_idx++)
        for (int lvl = 0; lvl < max_lvl; lvl++)
        {
            for(int i = 0; i < MFNR_MAX_GAUS_SIZE; i++)
                stmfnrParamsSelected->gfdelta_uv[dir_idx][lvl][i]  = ratio * (stmfnrParams->gfdelta_uv[gain_high][dir_idx][lvl][i] - stmfnrParams->gfdelta_uv[gain_low][dir_idx][lvl][i]) + stmfnrParams->gfdelta_uv[gain_low][dir_idx][lvl][i];
        }




    for (int lvl = 0; lvl < max_lvl; lvl++)
    {
        for(int i = 0; i < MFNR_MAX_GAUS_SIZE; i++)
            stmfnrParamsSelected->gfsigma[lvl][i]           = ratio * (stmfnrParams->gfsigma[gain_high][lvl][i] - stmfnrParams->gfsigma[gain_low][lvl][i]) + stmfnrParams->gfsigma[gain_low][lvl][i];
    }

    for (i = 0; i < range; i++)
        noise_sigma_tmp[i]                                  = (ratio * (stmfnrParams->noise_sigma[gain_high][i] - stmfnrParams->noise_sigma[gain_low][i]) + stmfnrParams->noise_sigma[gain_low][i]);
    noise_sigma_tmp[range]                                  = noise_sigma_tmp[range - 1];

    int max_pos[2];
    int min_pos[2];
    int intep_pos_flg[POLT_ORDER + 2];
    int intep_pos[POLT_ORDER + 2];
    int fix_x_pos[MAX_INTEPORATATION_LUMAPOINT];
    int max_idx = 0;
    int min_idx = 0;
    int intep_num = 0;
    int intep_num_1 = 0;
#if 1
    for(i = 1; i < MAX(stmfnrParams->curve_x0[gain_high], stmfnrParams->curve_x0[gain_low]); i++)
    {
        if(noise_sigma_tmp[i] > noise_sigma_tmp[i - 1] && noise_sigma_tmp[i] > noise_sigma_tmp[i + 1])
        {
            max_pos[max_idx]    = i;
            max_idx++;
        }
        if(noise_sigma_tmp[i] < noise_sigma_tmp[i - 1] && noise_sigma_tmp[i] < noise_sigma_tmp[i + 1])
        {
            min_pos[min_idx]    = i;
            min_idx++;
        }
    }
    intep_pos[intep_num++]    = 0;
    intep_pos[intep_num++]    = MAX(stmfnrParams->curve_x0[gain_high], stmfnrParams->curve_x0[gain_low]);
    for(i = 0; i < max_idx; i++)
        intep_pos[intep_num++]    = max_pos[i];
    for(i = 0; i < MIN(1, min_idx); i++)// only 1 min point is allowed
        intep_pos[intep_num++]    = min_pos[i];
    for(i = 0; i < intep_num; i++) {
        for(j = i; j < intep_num; j++)
        {
            if(intep_pos[i] > intep_pos[j])
            {
                int temp;
                temp            = intep_pos[i];
                intep_pos[i]    = intep_pos[j];
                intep_pos[j]    = temp;

            }
        }
    }

    intep_pos[intep_num++]      = range;
#endif

#if 1
#if 1
    for (i = 1; i < intep_num; i++)
    {
        if(i == 1)
            intep_pos[i]        = ROUND_F(intep_pos[i] / (step_x1 * 4))  * (4   * step_x1);
        else
            intep_pos[i]        = ROUND_F(intep_pos[i] / (step_x1 * 16)) * (16  * step_x1);
    }
    memset(intep_pos_flg, 0, sizeof(intep_pos_flg));
    for (i = 1; i < intep_num; i++)
    {
        if(intep_pos[i - 1] == intep_pos[i])
            intep_pos_flg[i] = 1;
    }
    for (int i = 0, j = 0; i < intep_num; i++)
    {
        if(intep_pos_flg[i] == 0)
        {
            intep_pos[j] = intep_pos[i];
            j++;
        }
    }
    intep_num = j;


    int off = 0;
    int st;
    int off4    = (0x4 * step_x1);
    int off8    = (0x8 * step_x1);
    int off16   = (0x10 * step_x1);
    int off32   = (0x20 * step_x1);
    for (int i = 0, idx = 1; i < MAX_INTEPORATATION_LUMAPOINT - 1; i++)
    {
        int cur_pos;
        if(idx >= intep_num && off >= range)
        {

            fix_x_pos[intep_num_1++] = range;
            continue;
        }
        cur_pos                     = intep_pos[idx];
        fix_x_pos[intep_num_1++]  = off;

        if(off & off4)
            st = off4;
        else if(off & off8)
            st = off8;
        else if((off < 64 * step_x1) || (off & off16))
            st = off16;
        else
            st = off32;
        if(off + st > cur_pos && off != cur_pos)
        {
            if((cur_pos - off)  & off4)
                st = off4;
            else if((cur_pos - off)  & off8)
                st = off8;
            else if((cur_pos - off)  & off16)
                st = off16;
            else
                st = off32;
        }
        if(off + st == cur_pos)
            idx++;

        off += st;

    }
    fix_x_pos[intep_num_1++]        = range;
#else
    for(int i = 0, idx = 1; i < MAX_INTEPORATATION_LUMAPOINT; i++)
        fix_x_pos[intep_num_1++]        = i * (range / (MAX_INTEPORATATION_LUMAPOINT - 1));
#endif



    for (i = 0; i < range; i += step_x)
        stmfnrParamsSelected->noise_sigma[i / step_x]             = noise_sigma_tmp[i];

    for (i = 0; i < MAX_INTEPORATATION_LUMAPOINT; i++) {
        stmfnrParamsSelected->noise_sigma_sample[i]             = noise_sigma_tmp[fix_x_pos[i]];
        stmfnrParamsSelected->noise_sigma_sample[i] = ABS(stmfnrParamsSelected->noise_sigma_sample[i]);
    }
    for (i = 0; i < MAX_INTEPORATATION_LUMAPOINT; i++)
        stmfnrParamsSelected->fix_x_pos[i]                    = fix_x_pos[i]                        / step_x;


    for (i = 0; i < MAX_INTEPORATATION_LUMAPOINT; i++) {
        stmfnrParamsSelected->noise_sigma_dehaze[i]             = noise_sigma_tmp[fix_x_pos[i]];
        stmfnrParamsSelected->noise_sigma_dehaze[i] = ABS(stmfnrParamsSelected->noise_sigma_dehaze[i]);
    }
    for (i = 0; i < MAX_INTEPORATATION_LUMAPOINT; i++)
        stmfnrParamsSelected->fix_x_pos_dehaze[i]               = fix_x_pos[i]                        / step_x_dehz;


#else
    for (i = 0; i < range; i += step)
        stmfnrParamsSelected->noise_sigma[i / step]   = noise_sigma_tmp[i] / step;

    int gap_num[POLT_ORDER] = {6, 6, 2, 2};
    int gap_piece = 8;
    int gap_rat[POLT_ORDER][5] = {{1, 2, 4, 6, 7}, {1, 2, 4, 6, 7}, {4, 8, 4, 4, 4}, {8, 8, 8, 8, 8}};

    for (i = 0; i < intep_num - 1; i++)
    {
        int left_pos                        = intep_pos[i];
        int right_pos                       = intep_pos[i + 1];
        int gap_num_cur                     = gap_num[i];
        intep_pos_1[intep_num_1++]          = intep_pos[i];
        for (j = 0; j < gap_num_cur - 1; j++)
        {
            intep_pos_1[intep_num_1++]      = ceil(left_pos + (double)(right_pos - left_pos) * gap_rat[i][j] / gap_piece);
        }
    }

    for (int idx = intep_num_1; idx < MAX_INTEPORATATION_LUMAPOINT - 1; idx++)
    {
        intep_pos_1[idx]              = intep_pos[intep_num - 1];
    }

    intep_pos_1[MAX_INTEPORATATION_LUMAPOINT - 1]                   = range;

    for (int idx = 0; idx < MAX_INTEPORATATION_LUMAPOINT; idx++)
    {
        intep_pos_1[idx]              = ROUND_F((double)intep_pos_1[idx] / step) * step;
    }


    for (int idx = 0; idx < intep_num; idx++)
    {
        intep_pos[idx]              = ROUND_F((double)intep_pos[idx] / step) * step;
    }


    for (i = 0; i < intep_num_1 - 1; i++)
    {
        int left_pos  = intep_pos_1[i] ;
        int right_pos = intep_pos_1[i + 1];
        double left_val  = noise_sigma_tmp[intep_pos_1[i]];
        double right_val = noise_sigma_tmp[intep_pos_1[i + 1]];
        for(j = left_pos; j < right_pos; j++)
        {
            if(j == 112)
                j = j;
            if(j != left_pos && j != right_pos)
                noise_sigma_tmp[j] = (left_val * (right_pos - j) + right_val * (j - left_pos)) / (right_pos - left_pos);
        }
    }

    for (i = 0; i < range; i += step)
        stmfnrParamsSelected->noise_sigma_intep[i / step]         = noise_sigma_tmp[i]                    / step;

    for (i = 0; i < MAX_INTEPORATATION_LUMAPOINT; i++)
        stmfnrParamsSelected->intep_pos_1[i]                    = intep_pos_1[i]                        / step;

    for (i = 0; i < MAX_INTEPORATATION_LUMAPOINT; i++)
        stmfnrParamsSelected->noise_sigma_dehaze[i]             = noise_sigma_tmp[intep_pos_1[i] ]      / step;

#endif

#if 1
    if(stmfnrParams->motion_detection_enable) {
        stmfnrParamsSelected->mfnr_sigma_scale = ((stmfnrParams->mfnr_sigma_scale[gain_high] * ratio + stmfnrParams->mfnr_sigma_scale[gain_low] * (1 - ratio))) ;
    } else {
        stmfnrParamsSelected->mfnr_sigma_scale = 1.0;
    }
    LOGD_ANR("mfnr motion detetion enable:%d mfnr_sigma_scale:%f\n",
             stmfnrParams->motion_detection_enable,
             stmfnrParamsSelected->mfnr_sigma_scale);
#endif
//for (i = 0; i < range; i++)
//    noise_sigma_tmp[i]                          = ROUND_F(noise_sigma_tmp[i] * (1 << 12)) >>  ;


    double noise_sigma_max = 0;
    double noise_sigma_limit = 1 << MFNR_F_INTE_SIGMA;
    double sigma_scale          = 1.0;
    double scale_scale          = 1.0;
    for(int i = 0; i < MAX_INTEPORATATION_LUMAPOINT; i++)
        noise_sigma_max = MAX(stmfnrParamsSelected->noise_sigma_sample[i], noise_sigma_max);

    if(noise_sigma_max * stmfnrParamsSelected->mfnr_sigma_scale <= noise_sigma_limit) {
        scale_scale = 1;
        sigma_scale = stmfnrParamsSelected->mfnr_sigma_scale;
    } else {
        scale_scale = noise_sigma_limit / (noise_sigma_max * stmfnrParamsSelected->mfnr_sigma_scale);
        sigma_scale = stmfnrParamsSelected->mfnr_sigma_scale * scale_scale;
    }

    if(scale_scale != 1.0 || sigma_scale != 1.0)
    {
        for(int i = 0; i < MAX_INTEPORATATION_LUMAPOINT; i++)
            stmfnrParamsSelected->noise_sigma_sample[i] = stmfnrParamsSelected->noise_sigma_sample[i] * sigma_scale;
        for(int i = 0; i < MAX_INTEPORATATION_LUMAPOINT; i++)
            stmfnrParamsSelected->noise_sigma_dehaze[i] = stmfnrParamsSelected->noise_sigma_dehaze[i] * sigma_scale;
        for(int dir_idx = 0; dir_idx < dir_num; dir_idx++)
            for(int lvl = 0; lvl < max_lvl; lvl++)
                stmfnrParamsSelected->scale[dir_idx][lvl] = stmfnrParamsSelected->scale[dir_idx][lvl] * scale_scale;
        for(int dir_idx = 0; dir_idx < dir_num; dir_idx++)
            for(int lvl = 0; lvl < max_lvl_uv; lvl++)
                stmfnrParamsSelected->scale_uv[dir_idx][lvl] = stmfnrParamsSelected->scale_uv[dir_idx][lvl] * scale_scale;
    }

    LOGD_ANR("mfnr final sigma_max:%f motion_scale:%f sigma_limit:%f sigma_scale:%f scale_scale:%f\n",
             noise_sigma_max,
             stmfnrParamsSelected->mfnr_sigma_scale,
             noise_sigma_limit,
             sigma_scale,
             scale_scale);

    return res;

}


template<typename T1, typename T2>
T1 MFNR_FIX_FLOAT_V1(T1 data0,  int bits, T2 &dst, int flag = 0)
{
    if(flag == 0)
        dst = (T2)ROUND_D(data0 * (1 << bits));
    else
        dst = (T2)FLOOR_INT64(data0 * (1 << bits));
    return  ((T1)dst / (1 << bits));

}


template<typename T1>
T1 MFNR_FX_CLP_V1(T1 data0, int inte_bit, int deci_bit)
{
    int64_t tmp0;
    int64_t max_val;
    int64_t min_val;
    int64_t out;
    tmp0        = (int64_t)(data0 * (1 << deci_bit));
    max_val     = (((int64_t)1 << (deci_bit + inte_bit)) - 1);
    min_val     = (-((int64_t)1 << (deci_bit + inte_bit)) + 1);
    out         = MIN(MAX(min_val, tmp0), max_val);
    return (T1)out / (1 << deci_bit);//->dst_shr_off64, round_val64, and_val64, round_bits);

}


int mfnr_get_matrix_idx_v1(int i, int j, int rad)
{
    int src_i = 0;
    int i_act, j_act;
    if(i > rad)
        i_act = 2 * rad - i;
    else
        i_act = i;

    if(j > rad)
        j_act = 2 * rad - j;
    else
        j_act = j;

    if(j_act < i_act) {
        int tmp;
        tmp = j_act;
        j_act = i_act;
        i_act = tmp;
    }

    for(int ii = rad; ii >= 0; ii--) {
        for(int jj = rad; jj >= ii; jj--) {
            if(i_act == ii && j_act == jj) {
                return src_i;
            }
            src_i++;
        }
    }

    return -1;
}


void mfnr_gfcoef_fix_v1(int rad, double *gfcoef, unsigned char* gfcoef_fix)
{
    double sum_d2;
    double *h;
    unsigned long tmp;

    h = (double*)malloc((rad * 2 + 1) * (rad * 2 + 1) * sizeof(double));
    sum_d2 = 0;
    for(int i = 0; i <  (rad * 2 + 1); i++) {
        for(int j = 0; j < (rad * 2 + 1); j++) {
            int src_i = mfnr_get_matrix_idx_v1(i, j, rad);
            if(src_i == -1) {
                LOGE_ANR("mfnr_get_matrix_idx_v1 is error \n");
            }
            h[i * (rad * 2 + 1) + j] = MFNR_FIX_FLOAT_V1((double)gfcoef[src_i], F_DECI_PIXEL_SIGMA_CONV_WEIGHT, tmp);
            gfcoef_fix[src_i] = tmp;
            sum_d2 += h[i * (rad * 2 + 1) + j];
        }
    }
    int idx = ((rad * 2 + 1) * rad + rad);
    h[idx]  = MFNR_FIX_FLOAT_V1(h[idx] + (1 - sum_d2), F_DECI_PIXEL_SIGMA_CONV_WEIGHT, tmp);
    gfcoef_fix[0] = tmp;
    free(h);

}


Amfnr_Result_V1_t mfnr_fix_transfer_v1(RK_MFNR_Params_V1_Select_t* tnr, RK_MFNR_Fix_V1_t *pMfnrCfg, Amfnr_ExpInfo_V1_t *pExpInfo, float gain_ratio, float fLumaStrength, float fChromaStrength)
{
    LOGI_ANR("%s:(%d) enter \n", __FUNCTION__, __LINE__);

    Amfnr_Result_V1_t res = AMFNR_RET_V1_SUCCESS;

    if(tnr == NULL || pMfnrCfg == NULL || pExpInfo == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AMFNR_RET_V1_NULL_POINTER;
    }

    LOGD_ANR("%s:%d iso:%d strength:%f\n", __FUNCTION__, __LINE__, pExpInfo->arIso[pExpInfo->hdr_mode], fLumaStrength);
    if(fLumaStrength <= 0.0) {
        fLumaStrength = 0.000001;
    }

    int i = 0;
    unsigned long tmp = 0;
    int mIso_last = 50;
    int mIso = 50;
    if(pExpInfo->mfnr_mode_3to1) {
        mIso_last = pExpInfo->preIso[pExpInfo->hdr_mode];
        mIso = pExpInfo->arIso[pExpInfo->hdr_mode];
    } else {
        mIso_last = pExpInfo->arIso[pExpInfo->hdr_mode];
        mIso = pExpInfo->arIso[pExpInfo->hdr_mode];
    }
    double gain_glb_filt;
    double gain_glb_ref1;
    double gain_glb_filt_sqrt;
    double gain_glb_filt_sqrt_inv;
    double scale_l_y                [MFNR_DIR_NUM][MFNR_MAX_LVL];
    double scale_l_y_uv             [MFNR_DIR_NUM][MFNR_MAX_LVL_UV];
    double scale_l_uv               [MFNR_DIR_NUM][MFNR_MAX_LVL_UV];
    double ratio_frq_sub_div        [2];
    int max_lvl         = MFNR_MAX_LVL;
    int max_lvl_uv      = MFNR_MAX_LVL_UV;
    int dir_num         = MFNR_DIR_NUM;


    int rad_isp20[2][5]                = {{2, 1, 1, 1, 1}, {2, 1, 1, 1, 1}};
    int rad_uv_isp20[2][5]             = {{2, 1, 1, 1, 1}, {2, 1, 1, 1, 1}};
    gain_glb_filt                           = (mIso_last / 50 * gain_ratio);
    gain_glb_ref1                           = (mIso / 50 * gain_ratio);//ref1
    gain_glb_filt_sqrt                      = sqrt(gain_glb_filt);
    gain_glb_filt_sqrt_inv                  = 1 / sqrt(gain_glb_filt);

    for(int dir_idx = 0; dir_idx < dir_num; dir_idx++) {
        for(int lvl = 0; lvl < max_lvl; lvl++)
            scale_l_y[dir_idx][lvl] = tnr->ci[dir_idx][lvl] * tnr->scale[dir_idx][lvl] / (tnr->dnstr[dir_idx] * fLumaStrength);
    }
    for(int dir_idx = 0; dir_idx < dir_num; dir_idx++) {
        for(int lvl = 0; lvl < max_lvl_uv; lvl++) {
            scale_l_y_uv[dir_idx][lvl] = tnr->ci[dir_idx][lvl] * tnr->scale_uv[dir_idx][lvl] / (tnr->dnstr_uv[dir_idx] * fChromaStrength);
            scale_l_uv[dir_idx][lvl] = tnr->ci_uv[dir_idx][lvl] * tnr->scale_uv[dir_idx][lvl] / (tnr->dnstr_uv[dir_idx] * fChromaStrength);
        }
    }

    //0x0080
    pMfnrCfg->mode = 0;
    pMfnrCfg->opty_en = 1;
    pMfnrCfg->optc_en = 1;
    pMfnrCfg->gain_en = 1;

    //0x0088
    tmp = (tnr->weight_limit_y[0] / fLumaStrength);
    if(tmp > 0xff) {
        tmp = 0xff;
    }
    pMfnrCfg->pk0_y = (unsigned char)tmp;

    tmp = (tnr->weight_limit_y[max_lvl - 1] / fLumaStrength);
    if(tmp > 0xff) {
        tmp = 0xff;
    }
    pMfnrCfg->pk1_y = (unsigned char)tmp;
    tmp = (tnr->weight_limit_uv[0] / fChromaStrength );
    if(tmp > 0xff) {
        tmp = 0xff;
    }
    pMfnrCfg->pk0_c = (unsigned char)tmp;
    tmp = (tnr->weight_limit_uv[max_lvl_uv - 1] / fChromaStrength);
    if(tmp > 0xff) {
        tmp = 0xff;
    }
    pMfnrCfg->pk1_c = (unsigned char)tmp;

    //0x008c
    MFNR_FIX_FLOAT_V1(gain_glb_filt, F_DECI_GAIN, tmp, 0);
    if(tmp > 0xffff) {
        tmp = 0xffff;
    }
    pMfnrCfg->glb_gain_cur = tmp;
    MFNR_FIX_FLOAT_V1(gain_glb_ref1, F_DECI_GAIN, tmp);
    if(tmp > 0xffff) {
        tmp = 0xffff;
    }
    pMfnrCfg->glb_gain_nxt = tmp;

    //0x0090
    gain_glb_filt_sqrt_inv = MFNR_FX_CLP_V1(gain_glb_filt_sqrt_inv, F_INTE_GAIN_GLB_SQRT_INV, F_DECI_GAIN_GLB_SQRT_INV);
    MFNR_FIX_FLOAT_V1(gain_glb_filt_sqrt_inv, F_DECI_GAIN_GLB_SQRT_INV, tmp);
    if(tmp > 0x1fff) {
        tmp = 0x1fff;
    }
    pMfnrCfg->glb_gain_cur_div = tmp;
    MFNR_FIX_FLOAT_V1(gain_glb_filt_sqrt, F_DECI_GAIN_GLB_SQRT, tmp);
    if(tmp > 0xff) {
        tmp = 0xff;
    }
    pMfnrCfg->glb_gain_cur_sqrt = tmp;

    //0x0094 - 0x0098
    for(i = 0; i < 17 - 1; i++) {
        pMfnrCfg->sigma_x[i] = log((double)(tnr->fix_x_pos[i + 1] - tnr->fix_x_pos[i])) / log((double)2) - 2;
    }

    //0x009c - 0x00bc
    for(i = 0; i < 17; i++) {
        MFNR_FIX_FLOAT_V1(tnr->noise_sigma_sample[i], F_DECI_SIGMA, tmp);
        pMfnrCfg->sigma_y[i] = tmp;
    }

    //0x00c4 - 0x00cc
    //dir_idx = 0;
    for(i = 0; i < 6; i++) {
        MFNR_FIX_FLOAT_V1(tnr->lumanrcurve[0][i], F_DECI_LUMASCALE, tmp);
        pMfnrCfg->luma_curve[i] = tmp;
    }

    //0x00d0
    MFNR_FIX_FLOAT_V1(tnr->ratio_frq[0], F_DECI_TXT_THRD_RATIO, tmp);
    pMfnrCfg->txt_th0_y = tmp;
    MFNR_FIX_FLOAT_V1(tnr->ratio_frq[1], F_DECI_TXT_THRD_RATIO, tmp);
    pMfnrCfg->txt_th1_y = tmp;

    //0x00d4
    MFNR_FIX_FLOAT_V1(tnr->ratio_frq[2], F_DECI_TXT_THRD_RATIO, tmp);
    pMfnrCfg->txt_th0_c = tmp;
    MFNR_FIX_FLOAT_V1(tnr->ratio_frq[3], F_DECI_TXT_THRD_RATIO, tmp);
    pMfnrCfg->txt_th1_c = tmp;

    //0x00d8
    ratio_frq_sub_div[0] = 1 / (tnr->ratio_frq[1] - tnr->ratio_frq[0]);
    ratio_frq_sub_div[1] = 1 / (tnr->ratio_frq[3] - tnr->ratio_frq[2]);
    MFNR_FIX_FLOAT_V1(ratio_frq_sub_div[0], F_DECI_TXT_THRD_RATIO, tmp, 1);
    pMfnrCfg->txt_thy_dlt = tmp;
    MFNR_FIX_FLOAT_V1(ratio_frq_sub_div[1], F_DECI_TXT_THRD_RATIO, tmp, 1);
    pMfnrCfg->txt_thc_dlt = tmp;

    //0x00dc - 0x00ec
    //dir_idx = 0; lvl 0-3;
    //rad_isp20[dir][levl];   gfsigma[lvl]
    mfnr_gfcoef_fix_v1(rad_isp20[0][0], tnr->gfsigma[0], pMfnrCfg->gfcoef_y0);
    mfnr_gfcoef_fix_v1(rad_isp20[0][1], tnr->gfsigma[1], pMfnrCfg->gfcoef_y1);
    mfnr_gfcoef_fix_v1(rad_isp20[0][2], tnr->gfsigma[2], pMfnrCfg->gfcoef_y2);
    mfnr_gfcoef_fix_v1(rad_isp20[0][3], tnr->gfsigma[3], pMfnrCfg->gfcoef_y3);

    //0x00f0 - 0x0100
    //dir_idx = 0; lvl 0-3;
    //rad_isp20[dir][levl];   gfdelta[[dir_idx][lvl]
    mfnr_gfcoef_fix_v1(rad_isp20[0][0], tnr->gfdelta[0][0], pMfnrCfg->gfcoef_yg0);
    mfnr_gfcoef_fix_v1(rad_isp20[0][1], tnr->gfdelta[0][1], pMfnrCfg->gfcoef_yg1);
    mfnr_gfcoef_fix_v1(rad_isp20[0][2], tnr->gfdelta[0][2], pMfnrCfg->gfcoef_yg2);
    mfnr_gfcoef_fix_v1(rad_isp20[0][3], tnr->gfdelta[0][3], pMfnrCfg->gfcoef_yg3);

    //0x0104 - 0x0110
    //dir_idx = 1; lvl 0-3;
    //rad_isp20[dir][levl];   gfdelta[[dir_idx][lvl]  ;
    mfnr_gfcoef_fix_v1(rad_isp20[1][0], tnr->gfdelta[1][0], pMfnrCfg->gfcoef_yl0);
    mfnr_gfcoef_fix_v1(rad_isp20[1][1], tnr->gfdelta[1][1], pMfnrCfg->gfcoef_yl1);
    mfnr_gfcoef_fix_v1(rad_isp20[1][2], tnr->gfdelta[1][2], pMfnrCfg->gfcoef_yl2);

    //0x0114 - 0x0120
    //dir_idx = 0; lvl 0-2;
    //rad_isp20[dir][levl];   gfdelta_uv[[dir_idx][lvl]  ;
    mfnr_gfcoef_fix_v1(rad_uv_isp20[0][0], tnr->gfdelta_uv[0][0], pMfnrCfg->gfcoef_cg0);
    mfnr_gfcoef_fix_v1(rad_uv_isp20[0][1], tnr->gfdelta_uv[0][1], pMfnrCfg->gfcoef_cg1);
    mfnr_gfcoef_fix_v1(rad_uv_isp20[0][2], tnr->gfdelta_uv[0][2], pMfnrCfg->gfcoef_cg2);

    //0x0124 - 0x012c
    //dir_idx = 1; lvl 0-1;
    //rad_isp20[dir][levl];   gfdelta_uv[[dir_idx][lvl]  ;
    mfnr_gfcoef_fix_v1(rad_uv_isp20[1][0], tnr->gfdelta_uv[1][0], pMfnrCfg->gfcoef_cl0);
    mfnr_gfcoef_fix_v1(rad_uv_isp20[1][1], tnr->gfdelta_uv[1][1], pMfnrCfg->gfcoef_cl1);

    //0x0130 - 0x0134
    //dir_idx = 0;  i = lvl;
    for(i = 0; i < 4; i++) {
        MFNR_FIX_FLOAT_V1(scale_l_y[0][i], F_DECI_SCALE_L, tmp);
        pMfnrCfg->scale_yg[i] = tmp;
    }

    //0x0138 - 0x013c
    //dir_idx = 1;  i = lvl;
    for(i = 0; i < 3; i++) {
        MFNR_FIX_FLOAT_V1(scale_l_y[1][i], F_DECI_SCALE_L, tmp);
        pMfnrCfg->scale_yl[i] = tmp;
    }

    //0x0140 - 0x0148
    //dir_idx = 0;  i = lvl;
    for(i = 0; i < 3; i++) {
        MFNR_FIX_FLOAT_V1(scale_l_uv[0][i], F_DECI_SCALE_L_UV, tmp);
        pMfnrCfg->scale_cg[i] = tmp;
        MFNR_FIX_FLOAT_V1(scale_l_y_uv[0][i], F_DECI_SCALE_L_UV, tmp);
        pMfnrCfg->scale_y2cg[i] = tmp;
    }

    //0x014c - 0x0154
    //dir_idx = 1;  i = lvl;
    for(i = 0; i < 2; i++) {
        MFNR_FIX_FLOAT_V1(scale_l_uv[1][i], F_DECI_SCALE_L_UV, tmp);
        pMfnrCfg->scale_cl[i] = tmp;
    }
    for(i = 0; i < 3; i++) {
        MFNR_FIX_FLOAT_V1(scale_l_y_uv[1][i], F_DECI_SCALE_L_UV, tmp);
        pMfnrCfg->scale_y2cl[i] = tmp;
    }

    //0x0158
    for(i = 0; i < 3; i++) {
        MFNR_FIX_FLOAT_V1(tnr->luma_w_in_chroma[i], F_DECI_LUMA_W_IN_CHROMA, tmp);
        pMfnrCfg->weight_y[i] = tmp;
    }

#if MFNR_FIX_VALUE_PRINTF
    mfnr_fix_Printf_v1(pMfnrCfg);
#endif


    LOGI_ANR("%s:(%d) exit \n", __FUNCTION__, __LINE__);

    return res;
}



Amfnr_Result_V1_t mfnr_fix_Printf_v1(RK_MFNR_Fix_V1_t  * pMfnrCfg)
{
    int i = 0;
    Amfnr_Result_V1_t res = AMFNR_RET_V1_SUCCESS;

    if(pMfnrCfg == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AMFNR_RET_V1_NULL_POINTER;
    }

    LOGD_ANR("%s:(%d) enter \n", __FUNCTION__, __LINE__);
    //0x0080
    LOGD_ANR("(0x0080) mode:%d opty_en:%d optc_en:%d gain_en:%d\n",
             pMfnrCfg->mode,
             pMfnrCfg->opty_en,
             pMfnrCfg->optc_en,
             pMfnrCfg->gain_en);

    //0x0088
    LOGD_ANR("(0x0088) pk0_y:%d pk1_y:%d pk0_c:%d pk1_c:%d \n",
             pMfnrCfg->pk0_y,
             pMfnrCfg->pk1_y,
             pMfnrCfg->pk0_c,
             pMfnrCfg->pk1_c);

    //0x008c
    LOGD_ANR("mfnr (0x008c) glb_gain_cur:%d glb_gain_nxt:%d \n",
             pMfnrCfg->glb_gain_cur,
             pMfnrCfg->glb_gain_nxt);

    //0x0090
    LOGD_ANR("(0x0090) glb_gain_cur_div:%d gain_glb_filt_sqrt:%d \n",
             pMfnrCfg->glb_gain_cur_div,
             pMfnrCfg->glb_gain_cur_sqrt);

    //0x0094 - 0x0098
    for(i = 0; i < 17 - 1; i++) {
        LOGD_ANR("(0x0094 - 0x0098) sigma_x[%d]:%d \n",
                 i, pMfnrCfg->sigma_x[i]);
    }

    //0x009c - 0x00bc
    for(i = 0; i < 17; i++) {
        LOGD_ANR("(0x009c - 0x00bc) sigma_y[%d]:%d \n",
                 i, pMfnrCfg->sigma_y[i]);
    }

    //0x00c4 - 0x00cc
    //dir_idx = 0;
    for(i = 0; i < 6; i++) {
        LOGD_ANR("(0x00c4 - 0x00cc) luma_curve[%d]:%d \n",
                 i, pMfnrCfg->luma_curve[i]);
    }

    //0x00d0
    LOGD_ANR("(0x00d0) txt_th0_y:%d txt_th1_y:%d \n",
             pMfnrCfg->txt_th0_y,
             pMfnrCfg->txt_th1_y);

    //0x00d4
    LOGD_ANR("(0x00d0) txt_th0_c:%d txt_th1_c:%d \n",
             pMfnrCfg->txt_th0_c,
             pMfnrCfg->txt_th1_c);

    //0x00d8
    LOGD_ANR("(0x00d8) txt_thy_dlt:%d txt_thc_dlt:%d \n",
             pMfnrCfg->txt_thy_dlt,
             pMfnrCfg->txt_thc_dlt);

    //0x00dc - 0x00ec
    for(i = 0; i < 6; i++) {
        LOGD_ANR("(0x00dc - 0x00ec) gfcoef_y0[%d]:%d \n",
                 i, pMfnrCfg->gfcoef_y0[i]);
    }
    for(i = 0; i < 3; i++) {
        LOGD_ANR("(0x00dc - 0x00ec) gfcoef_y1[%d]:%d \n",
                 i, pMfnrCfg->gfcoef_y1[i]);
    }
    for(i = 0; i < 3; i++) {
        LOGD_ANR("(0x00dc - 0x00ec) gfcoef_y2[%d]:%d \n",
                 i, pMfnrCfg->gfcoef_y2[i]);
    }
    for(i = 0; i < 3; i++) {
        LOGD_ANR("(0x00dc - 0x00ec) gfcoef_y3[%d]:%d \n",
                 i, pMfnrCfg->gfcoef_y3[i]);
    }

    //0x00f0 - 0x0100
    for(i = 0; i < 6; i++) {
        LOGD_ANR("(0x00f0 - 0x0100) gfcoef_yg0[%d]:%d \n",
                 i, pMfnrCfg->gfcoef_yg0[i]);
    }
    for(i = 0; i < 3; i++) {
        LOGD_ANR("(0x00f0 - 0x0100) gfcoef_yg1[%d]:%d \n",
                 i, pMfnrCfg->gfcoef_yg1[i]);
    }
    for(i = 0; i < 3; i++) {
        LOGD_ANR("(0x00f0 - 0x0100) gfcoef_yg2[%d]:%d \n",
                 i, pMfnrCfg->gfcoef_yg2[i]);
    }
    for(i = 0; i < 3; i++) {
        LOGD_ANR("(0x00f0 - 0x0100) gfcoef_yg3[%d]:%d \n",
                 i, pMfnrCfg->gfcoef_yg3[i]);
    }


    //0x0104 - 0x0110
    for(i = 0; i < 6; i++) {
        LOGD_ANR("(0x0104 - 0x0110) gfcoef_yl0[%d]:%d \n",
                 i, pMfnrCfg->gfcoef_yl0[i]);
    }
    for(i = 0; i < 3; i++) {
        LOGD_ANR("(0x0104 - 0x0110) gfcoef_yl1[%d]:%d \n",
                 i, pMfnrCfg->gfcoef_yl1[i]);
    }
    for(i = 0; i < 3; i++) {
        LOGD_ANR("(0x0104 - 0x0110) gfcoef_yl2[%d]:%d \n",
                 i, pMfnrCfg->gfcoef_yl2[i]);
    }

    //0x0114 - 0x0120
    for(i = 0; i < 6; i++) {
        LOGD_ANR("(0x0114 - 0x0120) gfcoef_cg0[%d]:%d \n",
                 i, pMfnrCfg->gfcoef_cg0[i]);
    }
    for(i = 0; i < 3; i++) {
        LOGD_ANR("(0x0114 - 0x0120) gfcoef_cg1[%d]:%d \n",
                 i, pMfnrCfg->gfcoef_cg1[i]);
    }
    for(i = 0; i < 3; i++) {
        LOGD_ANR("(0x0114 - 0x0120) gfcoef_cg2[%d]:%d \n",
                 i, pMfnrCfg->gfcoef_cg2[i]);
    }


    //0x0124 - 0x012c
    for(i = 0; i < 6; i++) {
        LOGD_ANR("(0x0124 - 0x012c) gfcoef_cl0[%d]:%d \n",
                 i, pMfnrCfg->gfcoef_cl0[i]);
    }
    for(i = 0; i < 3; i++) {
        LOGD_ANR("(0x0124 - 0x012c) gfcoef_cl1[%d]:%d \n",
                 i, pMfnrCfg->gfcoef_cl1[i]);
    }


    //0x0130 - 0x0134
    //dir_idx = 0;  i = lvl;
    for(i = 0; i < 4; i++) {
        LOGD_ANR("(0x0130 - 0x0134) scale_yg[%d]:%d \n",
                 i, pMfnrCfg->scale_yg[i]);
    }

    //0x0138 - 0x013c
    //dir_idx = 1;  i = lvl;
    for(i = 0; i < 3; i++) {
        LOGD_ANR("(0x0138 - 0x013c) scale_yl[%d]:%d \n",
                 i, pMfnrCfg->scale_yl[i]);
    }

    //0x0140 - 0x0148
    //dir_idx = 0;  i = lvl;
    for(i = 0; i < 3; i++) {
        LOGD_ANR("(0x0140 - 0x0148) scale_cg[%d]:%d \n",
                 i, pMfnrCfg->scale_cg[i]);
        LOGD_ANR("(0x0140 - 0x0148) scale_y2cg[%d]:%d \n",
                 i, pMfnrCfg->scale_y2cg[i]);
    }

    //0x014c - 0x0154
    //dir_idx = 1;  i = lvl;
    for(i = 0; i < 2; i++) {
        LOGD_ANR("(0x014c - 0x0154) scale_cl[%d]:%d \n",
                 i, pMfnrCfg->scale_cl[i]);
    }
    for(i = 0; i < 3; i++) {
        LOGD_ANR("(0x014c - 0x0154) scale_y2cl[%d]:%d \n",
                 i, pMfnrCfg->scale_y2cl[i]);
    }

    //0x0158
    for(i = 0; i < 3; i++) {
        LOGD_ANR("(0x0158) weight_y[%d]:%d \n",
                 i, pMfnrCfg->weight_y[i]);
    }

    LOGD_ANR("%s:(%d) exit \n", __FUNCTION__, __LINE__);

    return res;
}


Amfnr_Result_V1_t mfnr_dynamic_calc_v1(RK_MFNR_Dynamic_V1_t  * pDynamic, Amfnr_ExpInfo_V1_t *pExpInfo)
{
    LOGI_ANR("%s:(%d) enter \n", __FUNCTION__, __LINE__);

    if(pDynamic == NULL || pExpInfo == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AMFNR_RET_V1_NULL_POINTER;
    }

    Amfnr_Result_V1_t res = AMFNR_RET_V1_SUCCESS;
    float time = pExpInfo->arTime[pExpInfo->hdr_mode];
    float iso = pExpInfo->arIso[pExpInfo->hdr_mode];

    if(iso >= pDynamic->highth_iso && time >= pDynamic->highth_time) {
        pDynamic->mfnr_enable_state = 1;
    } else if(iso <= pDynamic->lowth_iso && time <= pDynamic->lowth_time) {
        pDynamic->mfnr_enable_state = 0;
    }

    LOGD_ANR("%s:%d mfnr: cur:%f %f  highth:%f %f  lowth:%f %f  finnal:%d\n",
             __FUNCTION__, __LINE__,
             iso, time,
             pDynamic->highth_iso, pDynamic->highth_time,
             pDynamic->lowth_iso, pDynamic->lowth_time,
             pDynamic->mfnr_enable_state);

    return res;
}


Amfnr_Result_V1_t mfnr_calibdbV2_assign_v1(CalibDbV2_MFNR_t *pDst, CalibDbV2_MFNR_t *pSrc)
{
    Amfnr_Result_V1_t res = AMFNR_RET_V1_SUCCESS;
    CalibDbV2_MFNR_Calib_t *pSrcCalibParaV2 = NULL;
    CalibDbV2_MFNR_Tuning_t *pSrcTuningParaV2 = NULL;
    CalibDbV2_MFNR_Calib_t *pDstCalibParaV2 = NULL;
    CalibDbV2_MFNR_Tuning_t *pDstTuningParaV2 = NULL;
    int setting_len = 0;
    int iso_len = 0;


    if(pDst == NULL || pSrc == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return AMFNR_RET_V1_NULL_POINTER;
    }

    mfnr_calibdbV2_free_v1(pDst);

    pSrcCalibParaV2 = &pSrc->CalibPara;
    pSrcTuningParaV2 = &pSrc->TuningPara;
    pDstCalibParaV2 = &pDst->CalibPara;
    pDstTuningParaV2 = &pDst->TuningPara;

    //assign the value
    pDst->Version = strdup(pSrc->Version);
    pDstTuningParaV2->enable = pSrcTuningParaV2->enable;
    pDstTuningParaV2->mode_3to1 = pSrcTuningParaV2->mode_3to1;
    pDstTuningParaV2->local_gain_en = pSrcTuningParaV2->local_gain_en;
    pDstTuningParaV2->motion_detect_en = pSrcTuningParaV2->motion_detect_en;

    //malloc iso size
    setting_len = pSrcCalibParaV2->Setting_len;
    pDstCalibParaV2->Setting = (CalibDbV2_MFNR_CalibPara_Setting_t *)malloc(setting_len * sizeof(CalibDbV2_MFNR_CalibPara_Setting_t));
    memset(pDstCalibParaV2->Setting,  0x00, setting_len * sizeof(CalibDbV2_MFNR_CalibPara_Setting_t));
    pDstCalibParaV2->Setting_len = setting_len;

    for(int i = 0; i < setting_len; i++) {
        iso_len = pSrcCalibParaV2->Setting[i].Calib_ISO_len;
        pDstCalibParaV2->Setting[i].Calib_ISO =  (CalibDbV2_MFNR_CalibPara_Setting_ISO_t *)malloc(iso_len * sizeof(CalibDbV2_MFNR_CalibPara_Setting_ISO_t));
        memset(pDstCalibParaV2->Setting[i].Calib_ISO, 0x00, iso_len * sizeof(CalibDbV2_MFNR_CalibPara_Setting_ISO_t));
        pDstCalibParaV2->Setting[i].Calib_ISO_len = iso_len;
    }

    for(int i = 0; i < setting_len; i++) {
        iso_len = pSrcCalibParaV2->Setting[i].Calib_ISO_len;
        pDstCalibParaV2->Setting[i].SNR_Mode = strdup(pSrcCalibParaV2->Setting[i].SNR_Mode);
        pDstCalibParaV2->Setting[i].Sensor_Mode = strdup(pSrcCalibParaV2->Setting[i].Sensor_Mode);

        for(int j = 0; j < iso_len; j++) {
            pDstCalibParaV2->Setting[i].Calib_ISO[j] = pSrcCalibParaV2->Setting[i].Calib_ISO[j];
        }
    }


    setting_len = pSrcTuningParaV2->Setting_len;
    pDstTuningParaV2->Setting = (CalibDbV2_MFNR_TuningPara_Setting_t *)malloc(setting_len * sizeof(CalibDbV2_MFNR_TuningPara_Setting_t));
    memset(pDstTuningParaV2->Setting, 0x00, setting_len * sizeof(CalibDbV2_MFNR_TuningPara_Setting_t));
    pDstTuningParaV2->Setting_len = setting_len;

    for(int i = 0; i < setting_len; i++) {
        iso_len = pSrcTuningParaV2->Setting[i].Tuning_ISO_len;
        pDstTuningParaV2->Setting[i].Tuning_ISO = (CalibDbV2_MFNR_TuningPara_Setting_ISO_t *)malloc(iso_len * sizeof(CalibDbV2_MFNR_TuningPara_Setting_ISO_t));
        memset(pDstTuningParaV2->Setting[i].Tuning_ISO, 0x00, iso_len * sizeof(CalibDbV2_MFNR_TuningPara_Setting_ISO_t));
        pDstTuningParaV2->Setting[i].Tuning_ISO_len = iso_len;
    }

    for(int i = 0; i < setting_len; i++) {
        iso_len = pSrcTuningParaV2->Setting[i].Tuning_ISO_len;
        pDstTuningParaV2->Setting[i].SNR_Mode = strdup(pSrcCalibParaV2->Setting[i].SNR_Mode);
        pDstTuningParaV2->Setting[i].Sensor_Mode = strdup(pSrcCalibParaV2->Setting[i].Sensor_Mode);

        for(int j = 0; j < iso_len; j++) {
            pDstTuningParaV2->Setting[i].Tuning_ISO[j] = pSrcTuningParaV2->Setting[i].Tuning_ISO[j];
        }
    }

    //motion
    iso_len = pSrcTuningParaV2->Motion.Motion_ISO_len;
    pDstTuningParaV2->Motion.Motion_ISO = (CalibDbV2_MFNR_TuningPara_Motion_ISO_t *)malloc(iso_len * sizeof(CalibDbV2_MFNR_TuningPara_Motion_ISO_t));
    memset(pDstTuningParaV2->Motion.Motion_ISO, 0x00, iso_len * sizeof(CalibDbV2_MFNR_TuningPara_Motion_ISO_t));
    pDstTuningParaV2->Motion.Motion_ISO_len = iso_len;
    for(int j = 0; j < iso_len; j++) {
        pDstTuningParaV2->Motion.Motion_ISO[j] = pSrcTuningParaV2->Motion.Motion_ISO[j];
    }

    //dynamic
    pDstTuningParaV2->Dynamic = pSrcTuningParaV2->Dynamic;

    return res;
}



void mfnr_calibdbV2_free_v1(CalibDbV2_MFNR_t *pCalibdbV2)
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

    }

}

//RKAIQ_END_DECLARE

