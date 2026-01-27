#include "cnr_xml2json_v1.h"
#include "RkAiqCalibApi.h"

#define CNRV1_SETTING_NUM (2)
#define CNRV1_ISO_NUM  (13)

int cnrV1_calibdb_to_calibdbV2(struct list_head *pCnrList,  CalibDbV2_CNR_t *pCalibdbV2, int mode_idx)
{
    CalibDbV2_CNR_Tuning_t *pTuningParaV2 = NULL;
    CalibDbV2_CNR_T_ISO_t *pISOV2 = NULL;


    if(pCnrList == NULL || pCalibdbV2 == NULL) {
        printf(" pCalibdb is NULL pointer\n");
        return -1;
    }

    Calibdb_Cnr_V1_t* pCnrProfile = NULL;
    CamCalibdbGetCnrV1ProfileByIdx(pCnrList, mode_idx, &pCnrProfile);
    if(pCnrProfile == NULL) {
        printf(" %s:%d cnr can't get mode:%d para from xml calibdb\n",
               __FUNCTION__,
               __LINE__,
               mode_idx);
        return -1;
    }


    pTuningParaV2 = &pCalibdbV2->TuningPara;

    //malloc settting size
    pTuningParaV2->Setting = (CalibDbV2_CNR_T_Set_t *)malloc(CNRV1_SETTING_NUM * sizeof(CalibDbV2_CNR_T_Set_t));
    memset(pTuningParaV2->Setting, 0x00, CNRV1_SETTING_NUM * sizeof(CalibDbV2_CNR_T_Set_t));
    pTuningParaV2->Setting_len = CNRV1_SETTING_NUM;

    //malloc iso size
    for(int i = 0; i < CNRV1_SETTING_NUM; i++) {
        pTuningParaV2->Setting[i].Tuning_ISO = (CalibDbV2_CNR_T_ISO_t *)malloc(CNRV1_ISO_NUM * sizeof(CalibDbV2_CNR_T_ISO_t));
        memset(pTuningParaV2->Setting[i].Tuning_ISO, 0x00, CNRV1_ISO_NUM * sizeof(CalibDbV2_CNR_T_ISO_t));
        pTuningParaV2->Setting[i].Tuning_ISO_len = CNRV1_ISO_NUM;
    }

    //assign the value
    pTuningParaV2->enable = pCnrProfile->enable;

    //tuning
    for(int i = 0; i < CNRV1_SETTING_NUM; i++) {
        Calibdb_Cnr_params_V1_t *pCnrParamsV1 = NULL;
        CamCalibdbGetCnrV1SettingByIdx(&pCnrProfile->listHead, i, &pCnrParamsV1);

        if(pCnrParamsV1 == NULL) {
            printf("cnr can't get setting:%d from xml calibdb! \n", i);
            break;
        }

        pTuningParaV2->Setting[i].SNR_Mode = strdup(pCnrParamsV1->snr_mode);
        pTuningParaV2->Setting[i].Sensor_Mode = strdup(pCnrParamsV1->sensor_mode);

        for(int j = 0; j < CNRV1_ISO_NUM; j++) {
            pISOV2 = &pTuningParaV2->Setting[i].Tuning_ISO[j];
            pISOV2->iso = pCnrParamsV1->iso[j];
            pISOV2->hf_bypass = pCnrParamsV1->rkcnr_hq_bila_bypass[j];
            pISOV2->lf_bypass = pCnrParamsV1->rkcnr_lq_bila_bypass[j];

            pISOV2->cnr_exgain = pCnrParamsV1->rkcnr_exgain[j];
            pISOV2->cnr_g_gain = pCnrParamsV1->rkcnr_g_gain[j];
            pISOV2->color_sat_adj = pCnrParamsV1->ratio[j];
            pISOV2->color_sat_adj_alpha = pCnrParamsV1->offset[j];

            pISOV2->hf_spikes_reducion_strength = pCnrParamsV1->medRatio1[j];
            pISOV2->hf_denoise_strength = pCnrParamsV1->sigmaR1[j];
            pISOV2->hf_color_sat = pCnrParamsV1->uvgain1[j];
            pISOV2->hf_denoise_alpha = pCnrParamsV1->bfRatio1[j];
            pISOV2->hf_bf_wgt_clip = pCnrParamsV1->hbf_wgt_clip[j];

            pISOV2->thumb_spikes_reducion_strength = pCnrParamsV1->medRatio2[j];
            pISOV2->thumb_denoise_strength = pCnrParamsV1->sigmaR2[j];
            pISOV2->thumb_color_sat = pCnrParamsV1->uvgain2[j];

            pISOV2->lf_denoise_strength = pCnrParamsV1->sigmaR3[j];
            pISOV2->lf_color_sat = pCnrParamsV1->uvgain3[j];
            pISOV2->lf_denoise_alpha = pCnrParamsV1->bfRatio3[j];

        }

        memcpy(pCalibdbV2->TuningPara.Kernel_Coeff.kernel_5x5, pCnrParamsV1->kernel_5x5_table, sizeof(pCnrParamsV1->kernel_5x5_table));
    }


    return 0;

}



int cnrV1_calibdbV2_to_calibdb(CalibDbV2_CNR_t *pCalibdbV2,  struct list_head *pCnrList,   int mode_idx)
{
    CalibDbV2_CNR_Tuning_t *pTuningParaV2 = NULL;
    CalibDbV2_CNR_T_ISO_t *pISOV2 = NULL;

    if(pCalibdbV2 == NULL || pCnrList == NULL) {
        printf(" pCalibdb is NULL pointer\n");
        return -1;
    }

    Calibdb_Cnr_V1_t* pCnrProfile = NULL;
    CamCalibdbGetCnrV1ProfileByIdx(pCnrList, mode_idx, &pCnrProfile);
    if(pCnrProfile == NULL) {
        printf(" %s:%d cnr can't get mode:%d para from xml calibdb\n",
               __FUNCTION__,
               __LINE__,
               mode_idx);
        return -1;
    }

    pTuningParaV2 = &pCalibdbV2->TuningPara;

    //assign the value
    pCnrProfile->enable = pTuningParaV2->enable;


    //tuning
    for(int i = 0; i < pTuningParaV2->Setting_len; i++) {
        Calibdb_Cnr_params_V1_t *pCnrParamsV1 = NULL;
        CamCalibdbGetCnrV1SettingByIdx(&pCnrProfile->listHead, i, &pCnrParamsV1);

        if(pCnrParamsV1 == NULL) {
            printf("cnr can't get setting:%d from xml calibdb! \n", i);
            break;
        }

        strcpy(pCnrParamsV1->snr_mode, pTuningParaV2->Setting[i].SNR_Mode);
        strcpy(pCnrParamsV1->sensor_mode, pTuningParaV2->Setting[i].Sensor_Mode);

        for(int j = 0; j < CNRV1_ISO_NUM; j++) {
            pISOV2 = &pTuningParaV2->Setting[i].Tuning_ISO[j];

            pCnrParamsV1->iso[j] = pISOV2->iso;
            pCnrParamsV1->rkcnr_hq_bila_bypass[j] = pISOV2->hf_bypass;
            pCnrParamsV1->rkcnr_lq_bila_bypass[j] = pISOV2->lf_bypass;

            pCnrParamsV1->rkcnr_exgain[j] = pISOV2->cnr_exgain;
            pCnrParamsV1->rkcnr_g_gain[j] = pISOV2->cnr_g_gain;
            pCnrParamsV1->ratio[j] = pISOV2->color_sat_adj;
            pCnrParamsV1->offset[j] = pISOV2->color_sat_adj_alpha;

            pCnrParamsV1->medRatio1[j] = pISOV2->hf_spikes_reducion_strength;
            pCnrParamsV1->sigmaR1[j] = pISOV2->hf_denoise_strength;
            pCnrParamsV1->uvgain1[j] = pISOV2->hf_color_sat;
            pCnrParamsV1->bfRatio1[j] = pISOV2->hf_denoise_alpha;
            pCnrParamsV1->hbf_wgt_clip[j] = pISOV2->hf_bf_wgt_clip;

            pCnrParamsV1->medRatio2[j] = pISOV2->thumb_spikes_reducion_strength;
            pCnrParamsV1->sigmaR2[j] = pISOV2->thumb_denoise_strength;
            pCnrParamsV1->uvgain2[j] = pISOV2->thumb_color_sat;

            pCnrParamsV1->sigmaR3[j] = pISOV2->lf_denoise_strength;
            pCnrParamsV1->uvgain3[j] = pISOV2->lf_color_sat;
            pCnrParamsV1->bfRatio3[j] = pISOV2->lf_denoise_alpha;

        }

        memcpy(pCnrParamsV1->kernel_5x5_table, pCalibdbV2->TuningPara.Kernel_Coeff.kernel_5x5, sizeof(pCnrParamsV1->kernel_5x5_table));
    }

    return 0;

}


void cnrV1_calibdbV2_free(CalibDbV2_CNR_t *pCalibdbV2)
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

