#include "uvnr_xml2json_v1.h"

#define UVNRV1_SETTING_NUM (2)
#define UVNRV1_ISO_NUM  (13)

int uvnrV1_calibdb_to_calibdbV2(const CalibDb_UVNR_2_t *pCalibdb,  CalibDbV2_UVNR_t *pCalibdbV2, int mode_idx)
{
    CalibDbV2_UVNR_Tuning_t *pTuningParaV2 = NULL;
    CalibDbV2_UVNR_T_ISO_t *pTuningISOV2 = NULL;
    CalibDbV2_UVNR_Kernel_t *pKernel_Coeff;
    CalibDb_UVNR_Params_t *pSetting = NULL;

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

    pTuningParaV2 = &pCalibdbV2->TuningPara;

    //malloc settting size
    pTuningParaV2->Setting = (CalibDbV2_UVNR_TuningPara_Setting_t *)malloc(UVNRV1_SETTING_NUM * sizeof(CalibDbV2_UVNR_TuningPara_Setting_t));
    memset(pTuningParaV2->Setting, 0x00, UVNRV1_SETTING_NUM * sizeof(CalibDbV2_UVNR_TuningPara_Setting_t));
    pTuningParaV2->Setting_len = UVNRV1_SETTING_NUM;

    //malloc iso size
    for(int i = 0; i < UVNRV1_SETTING_NUM; i++) {
        pTuningParaV2->Setting[i].Tuning_ISO = (CalibDbV2_UVNR_TuningPara_Setting_ISO_t *)malloc(UVNRV1_ISO_NUM * sizeof(CalibDbV2_UVNR_TuningPara_Setting_ISO_t));
        memset(pTuningParaV2->Setting[i].Tuning_ISO, 0x00, UVNRV1_ISO_NUM * sizeof(CalibDbV2_UVNR_TuningPara_Setting_ISO_t));
        pTuningParaV2->Setting[i].Tuning_ISO_len = UVNRV1_ISO_NUM;
    }

    //assign the value
    pTuningParaV2->enable = pCalibdb->enable;
    pCalibdbV2->Version = strdup(pCalibdb->version);

    //tuning
    for(int i = 0; i < UVNRV1_SETTING_NUM; i++) {
        pSetting = &pCalibdb->mode_cell[mode_idx].setting[i];

        pTuningParaV2->Setting[i].SNR_Mode = strdup(pSetting->snr_mode);
        pTuningParaV2->Setting[i].Sensor_Mode = strdup(pSetting->sensor_mode);

        for(int j = 0; j < UVNRV1_ISO_NUM; j++) {
            pTuningISOV2 = &pTuningParaV2->Setting[i].Tuning_ISO[j];
            pTuningISOV2->iso = pSetting->ISO[j];
            pTuningISOV2->step0_uvgrad_ratio = pSetting->step0_uvgrad_ratio[j];
            pTuningISOV2->step0_uvgrad_offset = pSetting->step0_uvgrad_offset[j];

            pTuningISOV2->step1_median_ratio = pSetting->step1_median_ratio[j];
            pTuningISOV2->step1_bf_sigmaR = pSetting->step1_bf_sigmaR[j];
            pTuningISOV2->step1_bf_uvgain = pSetting->step1_bf_uvgain[j];
            pTuningISOV2->step1_bf_ratio = pSetting->step1_bf_ratio[j];

            pTuningISOV2->step2_median_ratio = pSetting->step2_median_ratio[j];
            pTuningISOV2->step2_bf_sigmaR = pSetting->step2_bf_sigmaR[j];
            pTuningISOV2->step2_bf_uvgain = pSetting->step2_bf_uvgain[j];
            pTuningISOV2->step2_bf_ratio = pSetting->step2_bf_ratio[j];

            pTuningISOV2->step3_bf_sigmaR = pSetting->step3_bf_sigmaR[j];
            pTuningISOV2->step3_bf_uvgain = pSetting->step3_bf_uvgain[j];
            pTuningISOV2->step3_bf_ratio = pSetting->step3_bf_ratio[j];

        }

    }

    pKernel_Coeff = &pTuningParaV2->Kernel_Coeff;
    memcpy(pKernel_Coeff->kernel_3x3, pCalibdb->mode_cell[mode_idx].setting[0].kernel_3x3, sizeof(pKernel_Coeff->kernel_3x3));
    memcpy(pKernel_Coeff->kernel_5x5, pCalibdb->mode_cell[mode_idx].setting[0].kernel_5x5, sizeof(pKernel_Coeff->kernel_5x5));
    memcpy(pKernel_Coeff->kernel_9x9, pCalibdb->mode_cell[mode_idx].setting[0].kernel_9x9, sizeof(pKernel_Coeff->kernel_9x9));
    pKernel_Coeff->kernel_9x9_num = pCalibdb->mode_cell[mode_idx].setting[0].kernel_9x9_num;

    return 0;

}



int uvnrV1_calibdbV2_to_calibdb(CalibDbV2_UVNR_t *pCalibdbV2,  CalibDb_UVNR_2_t *pCalibdb,   int mode_idx)
{
    CalibDbV2_UVNR_Tuning_t *pTuningParaV2 = NULL;
    CalibDbV2_UVNR_T_ISO_t *pTuningISOV2 = NULL;
    CalibDbV2_UVNR_Kernel_t *pKernel_Coeff;
    CalibDb_UVNR_Params_t *pSetting = NULL;

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

    pTuningParaV2 = &pCalibdbV2->TuningPara;



    //assign the value
    pCalibdb->enable = pTuningParaV2->enable;
    strcpy(pCalibdb->version, pCalibdbV2->Version);

    //tuning
    for(int i = 0; i < pTuningParaV2->Setting_len; i++) {
        pSetting = &pCalibdb->mode_cell[mode_idx].setting[i];
        strcpy(pSetting->snr_mode, pTuningParaV2->Setting[i].SNR_Mode);
        strcpy(pSetting->sensor_mode, pTuningParaV2->Setting[i].Sensor_Mode);

        for(int j = 0; j < UVNRV1_ISO_NUM; j++) {
            pTuningISOV2 = &pTuningParaV2->Setting[i].Tuning_ISO[j];
            pSetting->ISO[j] = pTuningISOV2->iso;
            pSetting->step0_uvgrad_ratio[j] = pTuningISOV2->step0_uvgrad_ratio;
            pSetting->step0_uvgrad_offset[j] = pTuningISOV2->step0_uvgrad_offset;

            pSetting->step1_median_ratio[j] = pTuningISOV2->step1_median_ratio;
            pSetting->step1_bf_sigmaR[j] = pTuningISOV2->step1_bf_sigmaR;
            pSetting->step1_bf_uvgain[j] = pTuningISOV2->step1_bf_uvgain;
            pSetting->step1_bf_ratio[j] = pTuningISOV2->step1_bf_ratio;

            pSetting->step2_median_ratio[j] = pTuningISOV2->step2_median_ratio;
            pSetting->step2_bf_sigmaR[j] = pTuningISOV2->step2_bf_sigmaR;
            pSetting->step2_bf_uvgain[j] = pTuningISOV2->step2_bf_uvgain;
            pSetting->step2_bf_ratio[j] = pTuningISOV2->step2_bf_ratio;

            pSetting->step3_bf_sigmaR[j] = pTuningISOV2->step3_bf_sigmaR;
            pSetting->step3_bf_uvgain[j] = pTuningISOV2->step3_bf_uvgain;
            pSetting->step3_bf_ratio[j] = pTuningISOV2->step3_bf_ratio;

        }

    }

    pKernel_Coeff = &pTuningParaV2->Kernel_Coeff;
    memcpy(pCalibdb->mode_cell[mode_idx].setting[0].kernel_3x3, pKernel_Coeff->kernel_3x3, sizeof(pKernel_Coeff->kernel_3x3));
    memcpy(pCalibdb->mode_cell[mode_idx].setting[0].kernel_5x5, pKernel_Coeff->kernel_5x5, sizeof(pKernel_Coeff->kernel_5x5));
    memcpy(pCalibdb->mode_cell[mode_idx].setting[0].kernel_9x9, pKernel_Coeff->kernel_9x9, sizeof(pKernel_Coeff->kernel_9x9));
    pCalibdb->mode_cell[mode_idx].setting[0].kernel_9x9_num = pKernel_Coeff->kernel_9x9_num;

    return 0;

}


void uvnrV1_calibdbV2_free(CalibDbV2_UVNR_t *pCalibdbV2)
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

