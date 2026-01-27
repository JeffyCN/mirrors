#include "bayernr_xml2json_v1.h"

#define BAYERNRV1_SETTING_NUM (2)
#define BAYERNRV1_ISO_NUM   (13)

int bayernrV1_calibdb_to_calibdbV2(const CalibDb_BayerNr_2_t *pCalibdb,  CalibDbV2_BayerNrV1_t *pCalibdbV2, int mode_idx)
{
    CalibDbV2_BayerNrV1_Calib_t *pCalibParaV2 = NULL;
    CalibDbV2_BayerNrV1_Tuning_t *pTuningParaV2 = NULL;
    CalibDbV2_BayerNrV1_C_ISO_t *pCalibISOV2 = NULL;
    CalibDbV2_BayerNrV1_T_ISO_t *pTuningISOV2 = NULL;
    CalibDb_BayerNR_Params_t *pSetting = NULL;


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
    pCalibParaV2->Setting = (CalibDbV2_BayerNrV1_C_Set_t *)malloc(BAYERNRV1_SETTING_NUM * sizeof(CalibDbV2_BayerNrV1_C_Set_t));
    memset(pCalibParaV2->Setting,  0x00, BAYERNRV1_SETTING_NUM * sizeof(CalibDbV2_BayerNrV1_C_Set_t));
    pCalibParaV2->Setting_len = BAYERNRV1_SETTING_NUM;
    pTuningParaV2->Setting = (CalibDbV2_BayerNrV1_T_Set_t *)malloc(BAYERNRV1_SETTING_NUM * sizeof(CalibDbV2_BayerNrV1_T_Set_t));
    memset(pTuningParaV2->Setting, 0x00, BAYERNRV1_SETTING_NUM * sizeof(CalibDbV2_BayerNrV1_T_Set_t));
    pTuningParaV2->Setting_len = BAYERNRV1_SETTING_NUM;

    //malloc iso size
    for(int i = 0; i < BAYERNRV1_SETTING_NUM; i++) {
        pCalibParaV2->Setting[i].Calib_ISO =  (CalibDbV2_BayerNrV1_C_ISO_t *)malloc(BAYERNRV1_ISO_NUM * sizeof(CalibDbV2_BayerNrV1_C_ISO_t));
        memset(pCalibParaV2->Setting[i].Calib_ISO, 0x00, BAYERNRV1_ISO_NUM * sizeof(CalibDbV2_BayerNrV1_C_ISO_t));
        pCalibParaV2->Setting[i].Calib_ISO_len = BAYERNRV1_ISO_NUM;
        pTuningParaV2->Setting[i].Tuning_ISO = (CalibDbV2_BayerNrV1_T_ISO_t *)malloc(BAYERNRV1_ISO_NUM * sizeof(CalibDbV2_BayerNrV1_T_ISO_t));
        memset(pTuningParaV2->Setting[i].Tuning_ISO, 0x00, BAYERNRV1_ISO_NUM * sizeof(CalibDbV2_BayerNrV1_T_ISO_t));
        pTuningParaV2->Setting[i].Tuning_ISO_len = BAYERNRV1_ISO_NUM;
    }

    //assign the value
    pTuningParaV2->enable = pCalibdb->enable;
    pCalibdbV2->Version = strdup(pCalibdb->version);

    for(int i = 0; i < BAYERNRV1_SETTING_NUM; i++) {
        pSetting = &pCalibdb->mode_cell[mode_idx].setting[i];
        pCalibParaV2->Setting[i].SNR_Mode = strdup(pSetting->snr_mode);
        pCalibParaV2->Setting[i].Sensor_Mode = strdup(pSetting->sensor_mode);
        pTuningParaV2->Setting[i].SNR_Mode = strdup(pSetting->snr_mode);
        pTuningParaV2->Setting[i].Sensor_Mode = strdup(pSetting->sensor_mode);

        for(int j = 0; j < BAYERNRV1_ISO_NUM; j++) {
            pCalibISOV2 = &pCalibParaV2->Setting[i].Calib_ISO[j];
            pTuningISOV2 = &pTuningParaV2->Setting[i].Tuning_ISO[j];

            //calib
            pCalibISOV2->iso = pSetting->iso[j];
            for(int k = 0; k < 8; k++) {
                pCalibISOV2->luLevelVal[k] = pSetting->luLevelVal[k];
                pCalibISOV2->luRatio[k] = pSetting->luRatio[k][j];
            }

            //tuning
            pTuningISOV2->iso = pSetting->iso[j];
            pTuningISOV2->filtPara = pSetting->filtPara[j];
            pTuningISOV2->gauss_en = pSetting->gauss_en;
            pTuningISOV2->lamda = pSetting->lamda;
            pTuningISOV2->fixW0 = pSetting->fixW[0][j];
            pTuningISOV2->fixW1 = pSetting->fixW[1][j];
            pTuningISOV2->fixW2 = pSetting->fixW[2][j];
            pTuningISOV2->fixW3 = pSetting->fixW[3][j];
            pTuningISOV2->RGainOff = pSetting->RGainOff;
            pTuningISOV2->RGainFilp = pSetting->RGainFilp;
            pTuningISOV2->BGainOff = pSetting->BGainOff;
            pTuningISOV2->BGainFilp = pSetting->BGainFilp;


        }
    }

    return 0;

}



int bayernrV1_calibdbV2_to_calibdb(CalibDbV2_BayerNrV1_t *pCalibdbV2,  CalibDb_BayerNr_2_t *pCalibdb,  int mode_idx)
{
    CalibDbV2_BayerNrV1_Calib_t *pCalibParaV2 = NULL;
    CalibDbV2_BayerNrV1_Tuning_t *pTuningParaV2 = NULL;
    CalibDbV2_BayerNrV1_C_ISO_t *pCalibISOV2 = NULL;
    CalibDbV2_BayerNrV1_T_ISO_t *pTuningISOV2 = NULL;
    CalibDb_BayerNR_Params_t *pSetting = NULL;

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
    pCalibdb->enable = pTuningParaV2->enable;
    strcpy(pCalibdb->version, pCalibdbV2->Version);
    for(int i = 0; i < pTuningParaV2->Setting_len; i++) {
        pSetting = &pCalibdb->mode_cell[mode_idx].setting[i];

        strcpy( pSetting->snr_mode, pTuningParaV2->Setting[i].SNR_Mode);
        strcpy(pSetting->sensor_mode, pTuningParaV2->Setting[i].Sensor_Mode);
        for(int j = 0; j < BAYERNRV1_ISO_NUM; j++) {
            pCalibISOV2 = &pCalibParaV2->Setting[i].Calib_ISO[j];
            pTuningISOV2 = &pTuningParaV2->Setting[i].Tuning_ISO[j];

            //calib
            pSetting->iso[j] = pCalibISOV2->iso;
            for(int k = 0; k < 8; k++) {
                pSetting->luLevelVal[k] = pCalibISOV2->luLevelVal[k];
                pSetting->luRatio[k][j] = pCalibISOV2->luRatio[k];
            }

            //tuning
            pSetting->iso[j] = pTuningISOV2->iso;
            pSetting->filtPara[j] = pTuningISOV2->filtPara;
            pSetting->gauss_en = pTuningISOV2->gauss_en;
            pSetting->lamda = pTuningISOV2->lamda;
            pSetting->fixW[0][j] = pTuningISOV2->fixW0;
            pSetting->fixW[1][j] = pTuningISOV2->fixW1;
            pSetting->fixW[2][j] = pTuningISOV2->fixW2;
            pSetting->fixW[3][j] = pTuningISOV2->fixW3;
            pSetting->RGainOff = pTuningISOV2->RGainOff;
            pSetting->RGainFilp = pTuningISOV2->RGainFilp;
            pSetting->BGainOff = pTuningISOV2->BGainOff;
            pSetting->BGainFilp = pTuningISOV2->BGainFilp;


        }
    }

    return 0;

}


void bayernrV1_calibdbV2_free(CalibDbV2_BayerNrV1_t *pCalibdbV2)
{
    if(pCalibdbV2 != NULL) {
        if(pCalibdbV2->CalibPara.Setting != NULL) {
            for(int i = 0; i < pCalibdbV2->CalibPara.Setting_len; i++) {
                if(pCalibdbV2->CalibPara.Setting[i].Calib_ISO != NULL) {
                    free(pCalibdbV2->CalibPara.Setting[i].Calib_ISO );
                }
                if(pCalibdbV2->CalibPara.Setting[i].Sensor_Mode != NULL) {
                    free(pCalibdbV2->CalibPara.Setting[i].Sensor_Mode);
                }
                if(pCalibdbV2->CalibPara.Setting[i].SNR_Mode != NULL) {
                    free(pCalibdbV2->CalibPara.Setting[i].SNR_Mode);
                }
            }
            free(pCalibdbV2->CalibPara.Setting);
        }

        if(pCalibdbV2->TuningPara.Setting != NULL) {
            for(int i = 0; i < pCalibdbV2->TuningPara.Setting_len; i++) {
                if(pCalibdbV2->TuningPara.Setting[i].Tuning_ISO != NULL) {
                    free(pCalibdbV2->TuningPara.Setting[i].Tuning_ISO );
                }
                if(pCalibdbV2->TuningPara.Setting[i].Sensor_Mode != NULL) {
                    free(pCalibdbV2->TuningPara.Setting[i].Sensor_Mode);
                }
                if(pCalibdbV2->TuningPara.Setting[i].SNR_Mode != NULL) {
                    free(pCalibdbV2->TuningPara.Setting[i].SNR_Mode);
                }
            }
            free(pCalibdbV2->TuningPara.Setting);
        }

        free(pCalibdbV2);
    }

}

