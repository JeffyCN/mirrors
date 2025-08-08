#include "ynr_xml2json_v1.h"

#define YNRV1_SETTING_NUM (2)
#define YNRV1_ISO_NUM   (13)

int ynrV1_calibdb_to_calibdbV2(const CalibDb_YNR_2_t *pCalibdb,  CalibDbV2_YnrV1_t *pCalibdbV2, int mode_idx)
{

    CalibDbV2_YnrV1_Calib_t *pCalibParaV2 = NULL;
    CalibDbV2_YnrV1_Tuning_t *pTuningParaV2 = NULL;
    CalibDbV2_YnrV1_C_ISO_t *pCalibISOV2 = NULL;
    CalibDbV2_YnrV1_T_ISO_t *pTuningISOV2 = NULL;
    CalibDb_YNR_Setting_t *pSetting = NULL;
    struct CalibDb_YNR_ISO_s *pISO = NULL;

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
    pCalibParaV2->Setting = (CalibDbV2_YnrV1_Calib_Set_t *)malloc(YNRV1_SETTING_NUM * sizeof(CalibDbV2_YnrV1_Calib_Set_t));
    memset(pCalibParaV2->Setting,  0x00, YNRV1_SETTING_NUM * sizeof(CalibDbV2_YnrV1_Calib_Set_t));
    pCalibParaV2->Setting_len = YNRV1_SETTING_NUM;
    pTuningParaV2->Setting = (CalibDbV2_YnrV1_T_Set_t *)malloc(YNRV1_SETTING_NUM * sizeof(CalibDbV2_YnrV1_T_Set_t));
    memset(pTuningParaV2->Setting, 0x00, YNRV1_SETTING_NUM * sizeof(CalibDbV2_YnrV1_T_Set_t));
    pTuningParaV2->Setting_len = YNRV1_SETTING_NUM;

    //malloc iso size
    for(int i = 0; i < YNRV1_SETTING_NUM; i++) {
        pCalibParaV2->Setting[i].Calib_ISO =  (CalibDbV2_YnrV1_C_ISO_t *)malloc(YNRV1_ISO_NUM * sizeof(CalibDbV2_YnrV1_C_ISO_t));
        memset(pCalibParaV2->Setting[i].Calib_ISO, 0x00, YNRV1_ISO_NUM * sizeof(CalibDbV2_YnrV1_C_ISO_t));
        pCalibParaV2->Setting[i].Calib_ISO_len = YNRV1_ISO_NUM;
        pTuningParaV2->Setting[i].Tuning_ISO = (CalibDbV2_YnrV1_T_ISO_t *)malloc(YNRV1_ISO_NUM * sizeof(CalibDbV2_YnrV1_T_ISO_t));
        memset(pTuningParaV2->Setting[i].Tuning_ISO, 0x00, YNRV1_ISO_NUM * sizeof(CalibDbV2_YnrV1_T_ISO_t));
        pTuningParaV2->Setting[i].Tuning_ISO_len = YNRV1_ISO_NUM;
    }

    //assign the value
    pCalibdbV2->Version = strdup(pCalibdb->version);
    pTuningParaV2->enable = pCalibdb->enable;

    for(int i = 0; i < YNRV1_SETTING_NUM; i++) {
        pSetting = &pCalibdb->mode_cell[mode_idx].setting[i];

        pCalibParaV2->Setting[i].SNR_Mode = strdup(pSetting->snr_mode);
        pCalibParaV2->Setting[i].Sensor_Mode = strdup(pSetting->sensor_mode);
        pTuningParaV2->Setting[i].SNR_Mode = strdup(pSetting->snr_mode);
        pTuningParaV2->Setting[i].Sensor_Mode = strdup(pSetting->sensor_mode);

        for(int j = 0; j < YNRV1_ISO_NUM; j++) {
            pSetting = &pCalibdb->mode_cell[mode_idx].setting[i];
            pCalibISOV2 = &pCalibParaV2->Setting[i].Calib_ISO[j];
            pTuningISOV2 = &pTuningParaV2->Setting[i].Tuning_ISO[j];
            pISO = &pSetting->ynr_iso[j];

            //calib para
            pCalibISOV2->iso = pISO->iso;
            for(int k = 0; k < 5; k++) {
                pCalibISOV2->sigma_curve[k] = pISO->sigma_curve[k];
            }
            for(int k = 0; k < 4; k++) {
                pCalibISOV2->ynr_lci[k] = pISO->ynr_lci[k];
                pCalibISOV2->ynr_lhci[k] = pISO->ynr_lhci[k];
                pCalibISOV2->ynr_hlci[k] = pISO->ynr_hlci[k];
                pCalibISOV2->ynr_hhci[k] = pISO->ynr_hhci[k];
            }

            //tuning para
            pTuningISOV2->iso = pISO->iso;
            pTuningISOV2->lo_bfScale_1 = pISO->lo_bfScale[0];
            pTuningISOV2->lo_bfScale_2 = pISO->lo_bfScale[1];
            pTuningISOV2->lo_bfScale_3 = pISO->lo_bfScale[2];
            pTuningISOV2->lo_bfScale_4 = pISO->lo_bfScale[3];

            pTuningISOV2->denoise_weight_1 = pISO->denoise_weight[0];
            pTuningISOV2->denoise_weight_2 = pISO->denoise_weight[1];
            pTuningISOV2->denoise_weight_3 = pISO->denoise_weight[2];
            pTuningISOV2->denoise_weight_4 = pISO->denoise_weight[3];

            pTuningISOV2->imerge_ratio = pISO->imerge_ratio;
            pTuningISOV2->imerge_bound = pISO->imerge_bound;
            pTuningISOV2->lo_directionStrength = pISO->lo_directionStrength;

            pTuningISOV2->hi_bfScale_1 = pISO->hi_bfScale[0];
            pTuningISOV2->hi_bfScale_2 = pISO->hi_bfScale[1];
            pTuningISOV2->hi_bfScale_3 = pISO->hi_bfScale[2];
            pTuningISOV2->hi_bfScale_4 = pISO->hi_bfScale[3];

            pTuningISOV2->hi_denoiseWeight_1 = pISO->hi_denoiseWeight[0];
            pTuningISOV2->hi_denoiseWeight_2 = pISO->hi_denoiseWeight[1];
            pTuningISOV2->hi_denoiseWeight_3 = pISO->hi_denoiseWeight[2];
            pTuningISOV2->hi_denoiseWeight_4 = pISO->hi_denoiseWeight[3];

            pTuningISOV2->hi_soft_thresh_scale_1 = pISO->hi_soft_thresh_scale[0];
            pTuningISOV2->hi_soft_thresh_scale_2 = pISO->hi_soft_thresh_scale[1];
            pTuningISOV2->hi_soft_thresh_scale_3 = pISO->hi_soft_thresh_scale[2];
            pTuningISOV2->hi_soft_thresh_scale_4 = pISO->hi_soft_thresh_scale[3];

            pTuningISOV2->hi_denoiseStrength = pISO->hi_denoiseStrength;
            pTuningISOV2->hi_detailMinAdjDnW = pISO->hi_detailMinAdjDnW;

            for(int k = 0; k < 4; k++) {
                pTuningISOV2->hwith_d[k] = pISO->hwith_d[k];
            }

            //luma para
            for(int k = 0; k < 6; k++) {
                pTuningISOV2->luma_para.lo_lumaPoint[k] = pISO->lo_lumaPoint[k];
                pTuningISOV2->luma_para.lo_lumaRatio[k] = pISO->lo_lumaRatio[k];
                pTuningISOV2->luma_para.hi_lumaPoint[k] = pISO->hi_lumaPoint[k];
                pTuningISOV2->luma_para.hi_lumaRatio[k] = pISO->hi_lumaRatio[k];
            }

            //hgrad para
            for(int k = 0; k < 6; k++) {
                pTuningISOV2->hgrad_para.y_luma_point[k] = pISO->y_luma_point[k];
                pTuningISOV2->hgrad_para.hgrad_y_level1[k] = pISO->hgrad_y_level1[k];
                pTuningISOV2->hgrad_para.hgrad_y_level2[k] = pISO->hgrad_y_level2[k];
                pTuningISOV2->hgrad_para.hgrad_y_level3[k] = pISO->hgrad_y_level3[k];
                pTuningISOV2->hgrad_para.hgrad_y_level4[k] = pISO->hgrad_y_level4[k];
            }

        }
    }

    return 0;
}


int ynrV1_calibdbV2_to_calibdb(CalibDbV2_YnrV1_t *pCalibdbV2, CalibDb_YNR_2_t *pCalibdb, int mode_idx)
{

    CalibDbV2_YnrV1_Calib_t *pCalibParaV2 = NULL;
    CalibDbV2_YnrV1_Tuning_t *pTuningParaV2 = NULL;
    CalibDbV2_YnrV1_C_ISO_t *pCalibISOV2 = NULL;
    CalibDbV2_YnrV1_T_ISO_t *pTuningISOV2 = NULL;
    CalibDb_YNR_Setting_t *pSetting = NULL;
    struct CalibDb_YNR_ISO_s *pISO = NULL;

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

    for(int i = 0; i < pTuningParaV2->Setting_len; i++) {
        pSetting = &pCalibdb->mode_cell[mode_idx].setting[i];

        strcpy(pSetting->snr_mode, pTuningParaV2->Setting[i].SNR_Mode);
        strcpy(pSetting->sensor_mode, pTuningParaV2->Setting[i].Sensor_Mode);

        for(int j = 0; j < YNRV1_ISO_NUM; j++) {
            pSetting = &pCalibdb->mode_cell[mode_idx].setting[i];
            pCalibISOV2 = &pCalibParaV2->Setting[i].Calib_ISO[j];
            pTuningISOV2 = &pTuningParaV2->Setting[i].Tuning_ISO[j];
            pISO = &pSetting->ynr_iso[j];

            //calib para
            pISO->iso = pCalibISOV2->iso;
            for(int k = 0; k < 5; k++) {
                pISO->sigma_curve[k] = pCalibISOV2->sigma_curve[k];
            }
            for(int k = 0; k < 4; k++) {
                pISO->ynr_lci[k] = pCalibISOV2->ynr_lci[k];
                pISO->ynr_lhci[k] = pCalibISOV2->ynr_lhci[k];
                pISO->ynr_hlci[k] = pCalibISOV2->ynr_hlci[k];
                pISO->ynr_hhci[k] = pCalibISOV2->ynr_hhci[k];
            }

            //tuning para
            pISO->iso = pTuningISOV2->iso;
            pISO->lo_bfScale[0] = pTuningISOV2->lo_bfScale_1;
            pISO->lo_bfScale[1] = pTuningISOV2->lo_bfScale_2;
            pISO->lo_bfScale[2] = pTuningISOV2->lo_bfScale_3;
            pISO->lo_bfScale[3] = pTuningISOV2->lo_bfScale_4;

            pISO->denoise_weight[0] = pTuningISOV2->denoise_weight_1;
            pISO->denoise_weight[1] = pTuningISOV2->denoise_weight_2;
            pISO->denoise_weight[2] = pTuningISOV2->denoise_weight_3;
            pISO->denoise_weight[3] = pTuningISOV2->denoise_weight_4;

            pISO->imerge_ratio = pTuningISOV2->imerge_ratio;
            pISO->imerge_bound = pTuningISOV2->imerge_bound;
            pISO->lo_directionStrength = pTuningISOV2->lo_directionStrength;

            pISO->hi_bfScale[0] = pTuningISOV2->hi_bfScale_1;
            pISO->hi_bfScale[1] = pTuningISOV2->hi_bfScale_2;
            pISO->hi_bfScale[2] = pTuningISOV2->hi_bfScale_3;
            pISO->hi_bfScale[3] = pTuningISOV2->hi_bfScale_4;

            pISO->hi_denoiseWeight[0] = pTuningISOV2->hi_denoiseWeight_1;
            pISO->hi_denoiseWeight[1] = pTuningISOV2->hi_denoiseWeight_2;
            pISO->hi_denoiseWeight[2] = pTuningISOV2->hi_denoiseWeight_3;
            pISO->hi_denoiseWeight[3] = pTuningISOV2->hi_denoiseWeight_4;

            pISO->hi_soft_thresh_scale[0] = pTuningISOV2->hi_soft_thresh_scale_1;
            pISO->hi_soft_thresh_scale[1] = pTuningISOV2->hi_soft_thresh_scale_2;
            pISO->hi_soft_thresh_scale[2] = pTuningISOV2->hi_soft_thresh_scale_3;
            pISO->hi_soft_thresh_scale[3] = pTuningISOV2->hi_soft_thresh_scale_4;

            pISO->hi_denoiseStrength = pTuningISOV2->hi_denoiseStrength;
            pISO->hi_detailMinAdjDnW = pTuningISOV2->hi_detailMinAdjDnW;

            for(int k = 0; k < 4; k++) {
                pISO->hwith_d[k] = pTuningISOV2->hwith_d[k];
            }

            //luma para
            for(int k = 0; k < 6; k++) {
                pISO->lo_lumaPoint[k] = pTuningISOV2->luma_para.lo_lumaPoint[k];
                pISO->lo_lumaRatio[k] = pTuningISOV2->luma_para.lo_lumaRatio[k];
                pISO->hi_lumaPoint[k] = pTuningISOV2->luma_para.hi_lumaPoint[k];
                pISO->hi_lumaRatio[k] = pTuningISOV2->luma_para.hi_lumaRatio[k];
            }

            //hgrad para
            for(int k = 0; k < 6; k++) {
                pISO->y_luma_point[k] = pTuningISOV2->hgrad_para.y_luma_point[k];
                pISO->hgrad_y_level1[k] = pTuningISOV2->hgrad_para.hgrad_y_level1[k];
                pISO->hgrad_y_level2[k] = pTuningISOV2->hgrad_para.hgrad_y_level2[k];
                pISO->hgrad_y_level3[k] = pTuningISOV2->hgrad_para.hgrad_y_level3[k];
                pISO->hgrad_y_level4[k] = pTuningISOV2->hgrad_para.hgrad_y_level4[k];
            }

        }
    }

    return 0;

}


void ynrV1_calibdbV2_free(CalibDbV2_YnrV1_t *pCalibdbV2)
{
    if(pCalibdbV2) {
        if(pCalibdbV2->Version) {
            free(pCalibdbV2->Version);
        }

        if(pCalibdbV2->CalibPara.Setting) {
            for(int i = 0; i < pCalibdbV2->CalibPara.Setting_len; i++) {
                if(pCalibdbV2->CalibPara.Setting[i].SNR_Mode) {
                    free(pCalibdbV2->CalibPara.Setting[i].SNR_Mode);
                }
                if(pCalibdbV2->CalibPara.Setting[i].Sensor_Mode) {
                    free(pCalibdbV2->CalibPara.Setting[i].Sensor_Mode);
                }
                if(pCalibdbV2->CalibPara.Setting[i].Calib_ISO) {
                    free(pCalibdbV2->CalibPara.Setting[i].Calib_ISO);
                }
            }

            free(pCalibdbV2->CalibPara.Setting);
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

