#include "sharp_xml2json_v1.h"

#define SHARPV1_SETTING_NUM (2)
#define SHARPV1_ISO_NUM (13)

int sharpV1_calibdb_to_calibdbV2(const CalibDb_Sharp_2_t *pCalibdb,   CalibDbV2_SharpV1_t *pCalibdbV2,  int mode_idx)
{
    CalibDbV2_SharpV1_Tuning_t *pTuningParaV2 = NULL;
    CalibDbV2_SharpV1_T_ISO_t *pTuningISOV2 = NULL;
    CalibDbV2_SharpV1_Luma_t *pLumaParaV2 = NULL;
    CalibDbV2_SharpV1_Kernel_t *pKernelCoeffV2 = NULL;
    CalibDb_Sharp_Setting_t *pSetting = NULL;
    struct CalibDb_Sharp_ISO_s  *pISO = NULL;

    if(pCalibdb == NULL || pCalibdbV2 == NULL) {
        printf(" pCalibdb is NULL pointer\n");
        return -1;
    }

    if(pCalibdb->mode_num < mode_idx) {
        printf(" old xml file have no %d mode cell \n", mode_idx);
        return -1;
    }

    pTuningParaV2 = &pCalibdbV2->TuningPara;

    //malloc settting size
    pTuningParaV2->Setting = (CalibDbV2_SharpV1_T_Set_t *)malloc(SHARPV1_SETTING_NUM * sizeof(CalibDbV2_SharpV1_T_Set_t));
    memset(pTuningParaV2->Setting, 0x00, SHARPV1_SETTING_NUM * sizeof(CalibDbV2_SharpV1_T_Set_t));
    pTuningParaV2->Setting_len = SHARPV1_SETTING_NUM;

    //malloc iso size
    for(int i = 0; i < SHARPV1_SETTING_NUM; i++) {
        pTuningParaV2->Setting[i].Tuning_ISO = (CalibDbV2_SharpV1_T_ISO_t *)malloc(SHARPV1_ISO_NUM * sizeof(CalibDbV2_SharpV1_T_ISO_t));
        memset(pTuningParaV2->Setting[i].Tuning_ISO, 0x00, SHARPV1_ISO_NUM * sizeof(CalibDbV2_SharpV1_T_ISO_t));
        pTuningParaV2->Setting[i].Tuning_ISO_len = SHARPV1_ISO_NUM;
    }

    //assign the value
    pCalibdbV2->Version = strdup(pCalibdb->version);
    pTuningParaV2->enable = pCalibdb->enable;

    for(int i = 0; i < SHARPV1_SETTING_NUM; i++) {
        pSetting = &pCalibdb->mode_cell[mode_idx].setting[i];

        pTuningParaV2->Setting[i].SNR_Mode = strdup(pSetting->snr_mode);
        pTuningParaV2->Setting[i].Sensor_Mode = strdup(pSetting->sensor_mode);

        for(int j = 0; j < SHARPV1_ISO_NUM; j++) {
            pSetting = &pCalibdb->mode_cell[mode_idx].setting[i];
            pTuningISOV2 = &pTuningParaV2->Setting[i].Tuning_ISO[j];
            pISO = &pSetting->sharp_iso[j];

            pTuningISOV2->iso = pISO->iso;
            pTuningISOV2->hratio = pISO->hratio;
            pTuningISOV2->lratio = pISO->lratio;
            pTuningISOV2->mf_sharp_ratio = pISO->mf_sharp_ratio;
            pTuningISOV2->hf_sharp_ratio = pISO->hf_sharp_ratio;

            pTuningISOV2->pbf_gain = pISO->pbf_gain;
            pTuningISOV2->pbf_ratio = pISO->pbf_ratio;
            pTuningISOV2->pbf_add = pISO->pbf_add;

            pTuningISOV2->mbf_gain = pISO->mbf_gain;
            pTuningISOV2->mbf_add = pISO->mbf_add;

            pTuningISOV2->hbf_gain = pISO->hbf_gain;
            pTuningISOV2->hbf_ratio = pISO->hbf_ratio;
            pTuningISOV2->hbf_add = pISO->hbf_add;

            pTuningISOV2->local_sharp_strength = pISO->local_sharp_strength;

            pTuningISOV2->pbf_coeff_percent = pISO->pbf_coeff_percent;
            pTuningISOV2->rf_m_coeff_percent = pISO->rf_m_coeff_percent;
            pTuningISOV2->rf_h_coeff_percent = pISO->rf_h_coeff_percent;
            pTuningISOV2->hbf_coeff_percent = pISO->hbf_coeff_percent;

            pLumaParaV2 = &pTuningISOV2->luma_para;
            for(int k = 0; k < 8; k++) {
                pLumaParaV2->luma_point[k] = pCalibdb->luma_point[k];
                pLumaParaV2->luma_sigma[k] = pISO->luma_sigma[k];
                pLumaParaV2->mf_clip_pos[k] = pISO->mf_clip_pos[k];
                pLumaParaV2->mf_clip_neg[k] = pISO->mf_clip_neg[k];
                pLumaParaV2->hf_clip[k] = pISO->hf_clip[k];
            }
        }
    }

    pKernelCoeffV2 = &pTuningParaV2->kernel_coeff;
    memcpy(pKernelCoeffV2->gauss_luma_coeff, pCalibdb->mode_cell[mode_idx].gauss_luma_coeff, sizeof(pKernelCoeffV2->gauss_luma_coeff));
    memcpy(pKernelCoeffV2->pbf_coeff_l, pCalibdb->mode_cell[mode_idx].pbf_coeff_l, sizeof(pKernelCoeffV2->pbf_coeff_l));
    memcpy(pKernelCoeffV2->pbf_coeff_h, pCalibdb->mode_cell[mode_idx].pbf_coeff_h, sizeof(pKernelCoeffV2->pbf_coeff_h));
    memcpy(pKernelCoeffV2->rf_m_coeff_l, pCalibdb->mode_cell[mode_idx].rf_m_coeff_l, sizeof(pKernelCoeffV2->rf_m_coeff_l));
    memcpy(pKernelCoeffV2->rf_m_coeff_h, pCalibdb->mode_cell[mode_idx].rf_m_coeff_h, sizeof(pKernelCoeffV2->rf_m_coeff_h));
    memcpy(pKernelCoeffV2->rf_h_coeff_l, pCalibdb->mode_cell[mode_idx].rf_h_coeff_l, sizeof(pKernelCoeffV2->rf_h_coeff_l));
    memcpy(pKernelCoeffV2->rf_h_coeff_h, pCalibdb->mode_cell[mode_idx].rf_h_coeff_h, sizeof(pKernelCoeffV2->rf_h_coeff_h));
    memcpy(pKernelCoeffV2->hbf_coeff_l, pCalibdb->mode_cell[mode_idx].hbf_coeff_l, sizeof(pKernelCoeffV2->hbf_coeff_l));
    memcpy(pKernelCoeffV2->hbf_coeff_h, pCalibdb->mode_cell[mode_idx].hbf_coeff_h, sizeof(pKernelCoeffV2->hbf_coeff_h));
    memcpy(pKernelCoeffV2->mbf_coeff, pCalibdb->mode_cell[mode_idx].mbf_coeff, sizeof(pKernelCoeffV2->mbf_coeff));

    return 0;
}


int sharpV1_calibdbV2_to_calibdb(CalibDbV2_SharpV1_t *pCalibdbV2, CalibDb_Sharp_2_t *pCalibdb, int mode_idx)
{
    CalibDbV2_SharpV1_Tuning_t *pTuningParaV2 = NULL;
    CalibDbV2_SharpV1_T_ISO_t *pTuningISOV2 = NULL;
    CalibDbV2_SharpV1_Luma_t *pLumaParaV2 = NULL;
    CalibDbV2_SharpV1_Kernel_t *pKernelCoeffV2 = NULL;
    CalibDb_Sharp_Setting_t *pSetting = NULL;
    struct CalibDb_Sharp_ISO_s  *pISO = NULL;

    if(pCalibdb == NULL || pCalibdbV2 == NULL) {
        printf(" pCalibdb is NULL pointer\n");
        return -1;
    }

    if(pCalibdb->mode_num < mode_idx) {
        printf(" old xml file have no %d mode cell \n", mode_idx);
        return -1;
    }

    pTuningParaV2 = &pCalibdbV2->TuningPara;

    //assign the value
    strcpy(pCalibdb->version, pCalibdbV2->Version);
    pCalibdb->enable = pTuningParaV2->enable;

    for(int i = 0; i < SHARPV1_SETTING_NUM; i++) {
        pSetting = &pCalibdb->mode_cell[mode_idx].setting[i];

        strcpy(pSetting->snr_mode, pTuningParaV2->Setting[i].SNR_Mode);
        strcpy(pSetting->sensor_mode, pTuningParaV2->Setting[i].Sensor_Mode);

        for(int j = 0; j < SHARPV1_ISO_NUM; j++) {
            pSetting = &pCalibdb->mode_cell[mode_idx].setting[i];
            pTuningISOV2 = &pTuningParaV2->Setting[i].Tuning_ISO[j];
            pISO = &pSetting->sharp_iso[j];

            pISO->iso = pTuningISOV2->iso;
            pISO->hratio = pTuningISOV2->hratio;
            pISO->lratio = pTuningISOV2->lratio;
            pISO->mf_sharp_ratio = pTuningISOV2->mf_sharp_ratio;
            pISO->hf_sharp_ratio = pTuningISOV2->hf_sharp_ratio;

            pISO->pbf_gain = pTuningISOV2->pbf_gain;
            pISO->pbf_ratio = pTuningISOV2->pbf_ratio;
            pISO->pbf_add = pTuningISOV2->pbf_add;

            pISO->mbf_gain = pTuningISOV2->mbf_gain;
            pISO->mbf_add = pTuningISOV2->mbf_add;

            pISO->hbf_gain = pTuningISOV2->hbf_gain;
            pISO->hbf_ratio = pTuningISOV2->hbf_ratio;
            pISO->hbf_add = pTuningISOV2->hbf_add;

            pISO->local_sharp_strength = pTuningISOV2->local_sharp_strength;

            pISO->pbf_coeff_percent = pTuningISOV2->pbf_coeff_percent;
            pISO->rf_m_coeff_percent = pTuningISOV2->rf_m_coeff_percent;
            pISO->rf_h_coeff_percent = pTuningISOV2->rf_h_coeff_percent;
            pISO->hbf_coeff_percent = pTuningISOV2->hbf_coeff_percent;

            pLumaParaV2 = &pTuningISOV2->luma_para;
            for(int k = 0; k < 8; k++) {
                pCalibdb->luma_point[k] = pLumaParaV2->luma_point[k];
                pISO->luma_sigma[k] = pLumaParaV2->luma_sigma[k];
                pISO->mf_clip_pos[k] = pLumaParaV2->mf_clip_pos[k];
                pISO->mf_clip_neg[k] = pLumaParaV2->mf_clip_neg[k];
                pISO->hf_clip[k] = pLumaParaV2->hf_clip[k];
            }
        }
    }

    pKernelCoeffV2 = &pTuningParaV2->kernel_coeff;
    memcpy(pCalibdb->mode_cell[mode_idx].gauss_luma_coeff, pKernelCoeffV2->gauss_luma_coeff, sizeof(pKernelCoeffV2->gauss_luma_coeff));
    memcpy(pCalibdb->mode_cell[mode_idx].pbf_coeff_l, pKernelCoeffV2->pbf_coeff_l, sizeof(pKernelCoeffV2->pbf_coeff_l));
    memcpy(pCalibdb->mode_cell[mode_idx].pbf_coeff_h, pKernelCoeffV2->pbf_coeff_h, sizeof(pKernelCoeffV2->pbf_coeff_h));
    memcpy(pCalibdb->mode_cell[mode_idx].rf_m_coeff_l, pKernelCoeffV2->rf_m_coeff_l, sizeof(pKernelCoeffV2->rf_m_coeff_l));
    memcpy(pCalibdb->mode_cell[mode_idx].rf_m_coeff_h, pKernelCoeffV2->rf_m_coeff_h, sizeof(pKernelCoeffV2->rf_m_coeff_h));
    memcpy(pCalibdb->mode_cell[mode_idx].rf_h_coeff_l, pKernelCoeffV2->rf_h_coeff_l, sizeof(pKernelCoeffV2->rf_h_coeff_l));
    memcpy(pCalibdb->mode_cell[mode_idx].rf_h_coeff_h, pKernelCoeffV2->rf_h_coeff_h, sizeof(pKernelCoeffV2->rf_h_coeff_h));
    memcpy(pCalibdb->mode_cell[mode_idx].hbf_coeff_l, pKernelCoeffV2->hbf_coeff_l, sizeof(pKernelCoeffV2->hbf_coeff_l));
    memcpy(pCalibdb->mode_cell[mode_idx].hbf_coeff_h, pKernelCoeffV2->hbf_coeff_h, sizeof(pKernelCoeffV2->hbf_coeff_h));
    memcpy(pCalibdb->mode_cell[mode_idx].mbf_coeff, pKernelCoeffV2->mbf_coeff, sizeof(pKernelCoeffV2->mbf_coeff));

    return 0;
}


void sharpV1_calibdbV2_free(CalibDbV2_SharpV1_t *pCalibdbV2)
{
    if(pCalibdbV2) {
        if(pCalibdbV2->Version) {
            free(pCalibdbV2->Version);
        }

        if(pCalibdbV2->TuningPara.Setting) {
            for(int i = 0; i < pCalibdbV2->TuningPara.Setting_len; i++) {
                if(pCalibdbV2->TuningPara.Setting[i].SNR_Mode) {
                    free(pCalibdbV2->TuningPara.Setting[i].SNR_Mode);
                }

                if(pCalibdbV2->TuningPara.Setting[i].Sensor_Mode) {
                    free(pCalibdbV2->TuningPara.Setting[i].Sensor_Mode);
                }

                if(pCalibdbV2->TuningPara.Setting[i].Tuning_ISO) {
                    free(pCalibdbV2->TuningPara.Setting[i].Tuning_ISO);
                }
            }

            free(pCalibdbV2->TuningPara.Setting);
        }

        free(pCalibdbV2);
    }
}



