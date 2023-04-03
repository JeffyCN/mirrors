#include "edgefilter_xml2json_v1.h"

#define EDGEFILTERV1_SETTING_NUM (2)
#define EDGEFILTERV1_ISO_NUM    (13)

int edgefilterV1_calibdb_to_calibdbV2(const CalibDb_EdgeFilter_2_t *pCalibdb,  CalibDbV2_Edgefilter_t *pCalibdbV2, int mode_idx)
{
    CalibDbV2_Edgefilter_Tuning_t *pTuningParaV2 = NULL;
    CalibDbV2_Edgefilter_T_ISO_t *pTuningISOV2 = NULL;
    CalibDbV2_Edgefilter_Luma_t *pLuma_para = NULL;
    CalibDbV2_Edgefilter_Kernel_t *pDog_kernel = NULL;
    CalibDb_EdgeFilter_Setting_t *pSetting = NULL;
    struct CalibDb_EdgeFilter_ISO_s *pISO = NULL;

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
    pTuningParaV2->Setting = (CalibDbV2_Edgefilter_T_Set_t *)malloc(EDGEFILTERV1_SETTING_NUM * sizeof(CalibDbV2_Edgefilter_T_Set_t));
    memset(pTuningParaV2->Setting, 0x00, EDGEFILTERV1_SETTING_NUM * sizeof(CalibDbV2_Edgefilter_T_Set_t));
    pTuningParaV2->Setting_len = EDGEFILTERV1_SETTING_NUM;

    //malloc iso size
    for(int i = 0; i < EDGEFILTERV1_SETTING_NUM; i++) {
        pTuningParaV2->Setting[i].Tuning_ISO = (CalibDbV2_Edgefilter_T_ISO_t *)malloc(EDGEFILTERV1_ISO_NUM * sizeof(CalibDbV2_Edgefilter_T_ISO_t));
        memset(pTuningParaV2->Setting[i].Tuning_ISO, 0x00, EDGEFILTERV1_ISO_NUM * sizeof(CalibDbV2_Edgefilter_T_ISO_t));
        pTuningParaV2->Setting[i].Tuning_ISO_len = EDGEFILTERV1_ISO_NUM;
    }

    //assign the value
    pCalibdbV2->Version = strdup(pCalibdb->version);
    pTuningParaV2->enable = pCalibdb->enable;

    for(int i = 0; i < EDGEFILTERV1_SETTING_NUM; i++) {
        pSetting = &pCalibdb->mode_cell[mode_idx].setting[i];
        pTuningParaV2->Setting[i].SNR_Mode = strdup(pSetting->snr_mode);
        pTuningParaV2->Setting[i].Sensor_Mode = strdup(pSetting->sensor_mode);

        for(int j = 0; j < EDGEFILTERV1_ISO_NUM; j++) {
            pSetting = &pCalibdb->mode_cell[mode_idx].setting[i];
            pTuningISOV2 = &pTuningParaV2->Setting[i].Tuning_ISO[j];
            pISO = &pSetting->edgeFilter_iso[j];

            //tuning para
            pTuningISOV2->iso = pISO->iso;
            pTuningISOV2->edge_thed = pISO->edge_thed;
            pTuningISOV2->src_wgt = pISO->src_wgt;
            pTuningISOV2->alpha_adp_en = pISO->alpha_adp_en;
            pTuningISOV2->local_alpha = pISO->local_alpha;
            pTuningISOV2->global_alpha = pISO->global_alpha;
            pTuningISOV2->dog_kernel_percent = pISO->dog_kernel_percent;

            for(int k = 0; k < 5; k++) {
                pTuningISOV2->direct_filter_coeff[k] = pISO->direct_filter_coeff[k];
            }

            pLuma_para = &pTuningISOV2->luma_para;
            for(int k = 0; k < 8; k++) {
                pLuma_para->luma_point[k] = pCalibdb->luma_point[k];
                pLuma_para->noise_clip[k] = pISO->noise_clip[k];
                pLuma_para->dog_clip_pos[k] = pISO->dog_clip_pos[k];
                pLuma_para->dog_clip_neg[k] = pISO->dog_clip_neg[k];
                pLuma_para->dog_alpha[k] = pISO->dog_alpha[k];
            }
        }
    }

    pDog_kernel = &pTuningParaV2->dog_kernel;
    for(int i = 0; i < 25; i++) {
        pDog_kernel->dog_kernel_l[i] = pCalibdb->mode_cell[mode_idx].dog_kernel_l[i];
        pDog_kernel->dog_kernel_h[i] = pCalibdb->mode_cell[mode_idx].dog_kernel_h[i];
    }
    return 0;

}


int edgefilterV1_calibdbV2_to_calibdb(CalibDbV2_Edgefilter_t *pCalibdbV2, CalibDb_EdgeFilter_2_t *pCalibdb,  int mode_idx)
{
    CalibDbV2_Edgefilter_Tuning_t *pTuningParaV2 = NULL;
    CalibDbV2_Edgefilter_T_ISO_t *pTuningISOV2 = NULL;
    CalibDbV2_Edgefilter_Luma_t *pLuma_para = NULL;
    CalibDbV2_Edgefilter_Kernel_t *pDog_kernel = NULL;
    CalibDb_EdgeFilter_Setting_t *pSetting = NULL;
    struct CalibDb_EdgeFilter_ISO_s *pISO = NULL;

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

    for(int i = 0; i < pTuningParaV2->Setting_len; i++) {
        pSetting = &pCalibdb->mode_cell[mode_idx].setting[i];

        strcpy(pSetting->snr_mode, pTuningParaV2->Setting[i].SNR_Mode);
        strcpy(pSetting->sensor_mode, pTuningParaV2->Setting[i].Sensor_Mode);

        for(int j = 0; j < EDGEFILTERV1_ISO_NUM; j++) {
            pSetting = &pCalibdb->mode_cell[mode_idx].setting[i];
            pTuningISOV2 = &pTuningParaV2->Setting[i].Tuning_ISO[j];
            pISO = &pSetting->edgeFilter_iso[j];

            //tuning para
            pISO->iso = pTuningISOV2->iso;
            pISO->edge_thed = pTuningISOV2->edge_thed;
            pISO->src_wgt = pTuningISOV2->src_wgt;
            pISO->alpha_adp_en = pTuningISOV2->alpha_adp_en;
            pISO->local_alpha = pTuningISOV2->local_alpha;
            pISO->global_alpha = pTuningISOV2->global_alpha;
            pISO->dog_kernel_percent = pTuningISOV2->dog_kernel_percent;

            for(int k = 0; k < 5; k++) {
                pISO->direct_filter_coeff[k] = pTuningISOV2->direct_filter_coeff[k];
            }

            pLuma_para = &pTuningISOV2->luma_para;
            for(int k = 0; k < 8; k++) {
                pCalibdb->luma_point[k] = pLuma_para->luma_point[k];
                pISO->noise_clip[k] = pLuma_para->noise_clip[k];
                pISO->dog_clip_pos[k] = pLuma_para->dog_clip_pos[k];
                pISO->dog_clip_neg[k] = pLuma_para->dog_clip_neg[k];
                pISO->dog_alpha[k] = pLuma_para->dog_alpha[k];
            }
        }
    }

    pDog_kernel = &pTuningParaV2->dog_kernel;
    for(int i = 0; i < 25; i++) {
        pCalibdb->mode_cell[mode_idx].dog_kernel_l[i] = pDog_kernel->dog_kernel_l[i];
        pCalibdb->mode_cell[mode_idx].dog_kernel_h[i] = pDog_kernel->dog_kernel_h[i];
    }

    return 0;

}


void edgefilterV1_calibdbV2_free(CalibDbV2_Edgefilter_t *pCalibdbV2)
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

