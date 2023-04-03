#include "ynr_xml2json_v2.h"
#include "RkAiqCalibApi.h"


#define YNRV2_SETTING_NUM (2)
#define YNRV2_ISO_NUM   (13)

int ynrV2_calibdb_to_calibdbV2(struct list_head* pYnrList,  CalibDbV2_YnrV2_t *pCalibdbV2, int mode_idx)
{

    CalibDbV2_YnrV2_Calib_t *pCalibParaV2 = NULL;
    CalibDbV2_YnrV2_Tuning_t *pTuningParaV2 = NULL;
    CalibDbV2_YnrV2_C_ISO_t *pCalibISOV2 = NULL;
    CalibDbV2_YnrV2_T_ISO_t *pTuningISOV2 = NULL;

    if(pYnrList == NULL || pCalibdbV2 == NULL) {
        printf(" pCalibdb is NULL pointer\n");
        return -1;
    }

    Calibdb_Ynr_V2_t* pYnrProfile = NULL;
    CamCalibdbGetYnrV2ProfileByIdx(pYnrList, mode_idx, &pYnrProfile);
    if(pYnrProfile == NULL) {
        printf(" %s:%d bayernr can't get mode:%d para from calibdbv1\n",
               __FUNCTION__,
               __LINE__,
               mode_idx);
        return -1;
    }

    pCalibParaV2 = &pCalibdbV2->CalibPara;
    pTuningParaV2 = &pCalibdbV2->TuningPara;

    //malloc settting size
    pCalibParaV2->Setting = (CalibDbV2_YnrV2_C_Set_t *)malloc(YNRV2_SETTING_NUM * sizeof(CalibDbV2_YnrV2_C_Set_t));
    memset(pCalibParaV2->Setting,  0x00, YNRV2_SETTING_NUM * sizeof(CalibDbV2_YnrV2_C_Set_t));
    pCalibParaV2->Setting_len = YNRV2_SETTING_NUM;
    pTuningParaV2->Setting = (CalibDbV2_YnrV2_T_Set_t *)malloc(YNRV2_SETTING_NUM * sizeof(CalibDbV2_YnrV2_T_Set_t));
    memset(pTuningParaV2->Setting, 0x00, YNRV2_SETTING_NUM * sizeof(CalibDbV2_YnrV2_T_Set_t));
    pTuningParaV2->Setting_len = YNRV2_SETTING_NUM;

    //malloc iso size
    for(int i = 0; i < YNRV2_SETTING_NUM; i++) {
        pCalibParaV2->Setting[i].Calib_ISO =  (CalibDbV2_YnrV2_C_ISO_t *)malloc(YNRV2_ISO_NUM * sizeof(CalibDbV2_YnrV2_C_ISO_t));
        memset(pCalibParaV2->Setting[i].Calib_ISO, 0x00, YNRV2_ISO_NUM * sizeof(CalibDbV2_YnrV2_C_ISO_t));
        pCalibParaV2->Setting[i].Calib_ISO_len = YNRV2_ISO_NUM;
        pTuningParaV2->Setting[i].Tuning_ISO = (CalibDbV2_YnrV2_T_ISO_t *)malloc(YNRV2_ISO_NUM * sizeof(CalibDbV2_YnrV2_T_ISO_t));
        memset(pTuningParaV2->Setting[i].Tuning_ISO, 0x00, YNRV2_ISO_NUM * sizeof(CalibDbV2_YnrV2_T_ISO_t));
        pTuningParaV2->Setting[i].Tuning_ISO_len = YNRV2_ISO_NUM;
    }

    //assign the value
    pTuningParaV2->enable = pYnrProfile->enable;

    for(int i = 0; i < YNRV2_SETTING_NUM; i++) {
        Calibdb_Ynr_params_V2_t *pYnrParamsV1 = NULL;
        CamCalibdbGetYnrV2SettingByIdx(&pYnrProfile->listHead, i, &pYnrParamsV1);

        if(pYnrParamsV1 == NULL) {
            printf("cnr can't get setting:%d from xml calibdb! \n", i);
            break;
        }

        pCalibParaV2->Setting[i].SNR_Mode = strdup(pYnrParamsV1->snr_mode);
        pCalibParaV2->Setting[i].Sensor_Mode = strdup(pYnrParamsV1->sensor_mode);
        pTuningParaV2->Setting[i].SNR_Mode = strdup(pYnrParamsV1->snr_mode);
        pTuningParaV2->Setting[i].Sensor_Mode = strdup(pYnrParamsV1->sensor_mode);

        for(int j = 0; j < YNRV2_ISO_NUM; j++) {
            pCalibISOV2 = &pCalibParaV2->Setting[i].Calib_ISO[j];
            pTuningISOV2 = &pTuningParaV2->Setting[i].Tuning_ISO[j];

            //calib para
            pCalibISOV2->iso = pYnrParamsV1->iso[j];
            for(int k = 0; k < 5; k++) {
                pCalibISOV2->sigma_curve[k] = pYnrParamsV1->sigmaCurve[j][k];
            }
            pCalibISOV2->ynr_ci_l = pYnrParamsV1->ciISO_V2[0][j];
            pCalibISOV2->ynr_ci_h = pYnrParamsV1->ciISO_V2[1][j];

            //tuning para
            pTuningISOV2->iso = pYnrParamsV1->iso[j];
            pTuningISOV2->ynr_bft3x3_bypass = pYnrParamsV1->ynr_bft3x3_bypass_V2[j];
            pTuningISOV2->ynr_lbft5x5_bypass = pYnrParamsV1->ynr_lbft5x5_bypass_V2[j];
            pTuningISOV2->ynr_lgft3x3_bypass = pYnrParamsV1->ynr_lgft3x3_bypass_V2[j];
            pTuningISOV2->ynr_flt1x1_bypass = pYnrParamsV1->ynr_flt1x1_bypass_V2[j];
            pTuningISOV2->ynr_sft5x5_bypass = pYnrParamsV1->ynr_sft5x5_bypass_V2[j];

            for(int k = 0; k < 17; k++) {
                pTuningISOV2->rnr_strength[k] = pYnrParamsV1->ynr_rnr_strength_V2[j][k];
            }

            pTuningISOV2->low_bf_0 = pYnrParamsV1->ynr_low_bf_V2[0][j];
            pTuningISOV2->low_bf_1 = pYnrParamsV1->ynr_low_bf_V2[1][j];

            pTuningISOV2->low_thred_adj = pYnrParamsV1->ynr_low_thred_adj_V2[j];
            pTuningISOV2->low_peak_supress = pYnrParamsV1->ynr_low_peak_supress_V2[j];
            pTuningISOV2->low_edge_adj_thresh = pYnrParamsV1->ynr_low_edge_adj_thresh_V2[j];
            pTuningISOV2->low_center_weight = pYnrParamsV1->ynr_low_center_weight_V2[j];

            pTuningISOV2->low_dist_adj = pYnrParamsV1->ynr_low_dist_adj_V2[j];
            pTuningISOV2->low_weight = pYnrParamsV1->ynr_low_weight_V2[j];
            pTuningISOV2->low_filt_strength_0 = pYnrParamsV1->ynr_low_filt_strength_V2[0][j];
            pTuningISOV2->low_filt_strength_1 = pYnrParamsV1->ynr_low_filt_strength_V2[1][j];
            pTuningISOV2->low_bi_weight = pYnrParamsV1->ynr_low_bi_weight_V2[j];

            pTuningISOV2->base_filter_weight_0 = pYnrParamsV1->ynr_base_filter_weight_V2[0][j];
            pTuningISOV2->base_filter_weight_1 = pYnrParamsV1->ynr_base_filter_weight_V2[1][j];
            pTuningISOV2->base_filter_weight_2 = pYnrParamsV1->ynr_base_filter_weight_V2[2][j];

            pTuningISOV2->high_thred_adj = pYnrParamsV1->ynr_high_thred_adj_V2[j];
            pTuningISOV2->high_weight = pYnrParamsV1->ynr_high_weight_V2[j];
            for(int k = 0; k < 8; k++) {
                pTuningISOV2->high_direction_weight[k] = pYnrParamsV1->ynr_direction_weight_V2[j][k];
            }
            pTuningISOV2->hi_min_adj = pYnrParamsV1->ynr_hi_min_adj_V2[j];
            pTuningISOV2->hi_edge_thed = pYnrParamsV1->ynr_hi_edge_thed_V2[j];

        }
    }

    return 0;
}


int ynrV2_calibdbV2_to_calibdb(CalibDbV2_YnrV2_t *pCalibdbV2, struct list_head* pYnrList, int mode_idx)
{

    CalibDbV2_YnrV2_Calib_t *pCalibParaV2 = NULL;
    CalibDbV2_YnrV2_Tuning_t *pTuningParaV2 = NULL;
    CalibDbV2_YnrV2_C_ISO_t *pCalibISOV2 = NULL;
    CalibDbV2_YnrV2_T_ISO_t *pTuningISOV2 = NULL;

    if(pYnrList == NULL || pCalibdbV2 == NULL) {
        printf(" pCalibdb is NULL pointer\n");
        return -1;
    }


    pCalibParaV2 = &pCalibdbV2->CalibPara;
    pTuningParaV2 = &pCalibdbV2->TuningPara;


    Calibdb_Ynr_V2_t* pYnrProfile = NULL;
    CamCalibdbGetYnrV2ProfileByIdx(pYnrList, mode_idx, &pYnrProfile);
    if(pYnrProfile == NULL) {
        printf(" %s:%d bayernr can't get mode:%d para from calibdbv1\n",
               __FUNCTION__,
               __LINE__,
               mode_idx);
        return -1;
    }


    //assign the value
    pYnrProfile->enable = pTuningParaV2->enable;

    for(int i = 0; i < pTuningParaV2->Setting_len; i++) {
        Calibdb_Ynr_params_V2_t *pYnrParamsV1 = NULL;
        CamCalibdbGetYnrV2SettingByIdx(&pYnrProfile->listHead, i, &pYnrParamsV1);

        if(pYnrParamsV1 == NULL) {
            printf("cnr can't get setting:%d from xml calibdb! \n", i);
            break;
        }

        strcpy(pYnrParamsV1->snr_mode, pTuningParaV2->Setting[i].SNR_Mode);
        strcpy(pYnrParamsV1->sensor_mode, pTuningParaV2->Setting[i].Sensor_Mode);

        for(int j = 0; j < YNRV2_ISO_NUM; j++) {
            pCalibISOV2 = &pCalibParaV2->Setting[i].Calib_ISO[j];
            pTuningISOV2 = &pTuningParaV2->Setting[i].Tuning_ISO[j];

            //calib para
            pYnrParamsV1->iso[j] = pCalibISOV2->iso;
            for(int k = 0; k < 5; k++) {
                pYnrParamsV1->sigmaCurve[j][k] = pCalibISOV2->sigma_curve[k];
            }
            pYnrParamsV1->ciISO_V2[0][j] = pCalibISOV2->ynr_ci_l;
            pYnrParamsV1->ciISO_V2[1][j] = pCalibISOV2->ynr_ci_h;

            //tuning para
            pYnrParamsV1->iso[j] = pTuningISOV2->iso;
            pYnrParamsV1->ynr_bft3x3_bypass_V2[j] = pTuningISOV2->ynr_bft3x3_bypass;
            pYnrParamsV1->ynr_lbft5x5_bypass_V2[j] = pTuningISOV2->ynr_lbft5x5_bypass;
            pYnrParamsV1->ynr_lgft3x3_bypass_V2[j] = pTuningISOV2->ynr_lgft3x3_bypass;
            pYnrParamsV1->ynr_flt1x1_bypass_V2[j] = pTuningISOV2->ynr_flt1x1_bypass;
            pYnrParamsV1->ynr_sft5x5_bypass_V2[j] = pTuningISOV2->ynr_sft5x5_bypass;

            for(int k = 0; k < 17; k++) {
                pYnrParamsV1->ynr_rnr_strength_V2[j][k] = pTuningISOV2->rnr_strength[k];
            }

            pYnrParamsV1->ynr_low_bf_V2[0][j] = pTuningISOV2->low_bf_0;
            pYnrParamsV1->ynr_low_bf_V2[1][j] = pTuningISOV2->low_bf_1;

            pYnrParamsV1->ynr_low_thred_adj_V2[j] = pTuningISOV2->low_thred_adj;
            pYnrParamsV1->ynr_low_peak_supress_V2[j] = pTuningISOV2->low_peak_supress;
            pYnrParamsV1->ynr_low_edge_adj_thresh_V2[j] = pTuningISOV2->low_edge_adj_thresh;
            pYnrParamsV1->ynr_low_center_weight_V2[j] = pTuningISOV2->low_center_weight;

            pYnrParamsV1->ynr_low_dist_adj_V2[j] = pTuningISOV2->low_dist_adj;
            pYnrParamsV1->ynr_low_weight_V2[j] = pTuningISOV2->low_weight;
            pYnrParamsV1->ynr_low_filt_strength_V2[0][j] = pTuningISOV2->low_filt_strength_0;
            pYnrParamsV1->ynr_low_filt_strength_V2[1][j] = pTuningISOV2->low_filt_strength_1;
            pYnrParamsV1->ynr_low_bi_weight_V2[j] = pTuningISOV2->low_bi_weight;

            pYnrParamsV1->ynr_base_filter_weight_V2[0][j] = pTuningISOV2->base_filter_weight_0;
            pYnrParamsV1->ynr_base_filter_weight_V2[1][j] = pTuningISOV2->base_filter_weight_1;
            pYnrParamsV1->ynr_base_filter_weight_V2[2][j] = pTuningISOV2->base_filter_weight_2;

            pYnrParamsV1->ynr_high_thred_adj_V2[j] = pTuningISOV2->high_thred_adj;
            pYnrParamsV1->ynr_high_weight_V2[j] = pTuningISOV2->high_weight;
            for(int k = 0; k < 8; k++) {
                pYnrParamsV1->ynr_direction_weight_V2[j][k] = pTuningISOV2->high_direction_weight[k];
            }
            pYnrParamsV1->ynr_hi_min_adj_V2[j] = pTuningISOV2->hi_min_adj;
            pYnrParamsV1->ynr_hi_edge_thed_V2[j] = pTuningISOV2->hi_edge_thed;

        }
    }

    return 0;

}


void ynrV2_calibdbV2_free(CalibDbV2_YnrV2_t *pCalibdbV2)
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

