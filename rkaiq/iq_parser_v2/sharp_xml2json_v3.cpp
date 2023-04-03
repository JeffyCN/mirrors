#include "sharp_xml2json_v3.h"
#include "RkAiqCalibApi.h"

#define SHARPV3_SETTING_NUM (2)
#define SHARPV3_ISO_NUM (13)

int sharpV3_calibdb_to_calibdbV2(struct list_head *pSharpList,   CalibDbV2_SharpV3_t *pCalibdbV2,  int mode_idx)
{
    CalibDbV2_SharpV3_Tuning_t *pTuningParaV2 = NULL;
    CalibDbV2_SharpV3_T_ISO_t *pTuningISOV2 = NULL;
    CalibDbV2_SharpV3_Luma_t *pLumaParaV2 = NULL;
    CalibDbV2_SharpV3_kernel_t *pKernelCoeffV2 = NULL;

    if(pSharpList == NULL || pCalibdbV2 == NULL) {
        printf(" pCalibdb is NULL pointer\n");
        return -1;
    }

    Calibdb_Sharp_V3_t* pSharpProfile = NULL;
    CamCalibdbGetSharpV3ProfileByIdx(pSharpList, mode_idx, &pSharpProfile);
    if(pSharpProfile == NULL) {
        printf(" %s:%d bayernr can't get mode:%d para from calibdbv1\n",
               __FUNCTION__,
               __LINE__,
               mode_idx);
        return -1;
    }

    pTuningParaV2 = &pCalibdbV2->TuningPara;

    //malloc settting size
    pTuningParaV2->Setting = (CalibDbV2_SharpV3_T_Set_t *)malloc(SHARPV3_SETTING_NUM * sizeof(CalibDbV2_SharpV3_T_Set_t));
    memset(pTuningParaV2->Setting, 0x00, SHARPV3_SETTING_NUM * sizeof(CalibDbV2_SharpV3_T_Set_t));
    pTuningParaV2->Setting_len = SHARPV3_SETTING_NUM;

    //malloc iso size
    for(int i = 0; i < SHARPV3_SETTING_NUM; i++) {
        pTuningParaV2->Setting[i].Tuning_ISO = (CalibDbV2_SharpV3_T_ISO_t *)malloc(SHARPV3_ISO_NUM * sizeof(CalibDbV2_SharpV3_T_ISO_t));
        memset(pTuningParaV2->Setting[i].Tuning_ISO, 0x00, SHARPV3_ISO_NUM * sizeof(CalibDbV2_SharpV3_T_ISO_t));
        pTuningParaV2->Setting[i].Tuning_ISO_len = SHARPV3_ISO_NUM;
    }

    //assign the value
    pTuningParaV2->enable = pSharpProfile->enable;

    for(int i = 0; i < SHARPV3_SETTING_NUM; i++) {
        Calibdb_Sharp_params_V3_t *pSharpParamsV1 = NULL;
        CamCalibdbGetSharpV3SettingByIdx(&pSharpProfile->listHead, i, &pSharpParamsV1);

        if(pSharpParamsV1 == NULL) {
            printf("cnr can't get setting:%d from xml calibdb! \n", i);
            break;
        }

        pTuningParaV2->Setting[i].SNR_Mode = strdup(pSharpParamsV1->snr_mode);
        pTuningParaV2->Setting[i].Sensor_Mode = strdup(pSharpParamsV1->sensor_mode);

        for(int j = 0; j < SHARPV3_ISO_NUM; j++) {
            pTuningISOV2 = &pTuningParaV2->Setting[i].Tuning_ISO[j];

            pTuningISOV2->iso = pSharpParamsV1->iso[j];
            pTuningISOV2->pbf_gain = pSharpParamsV1->pbf_gain[j];
            pTuningISOV2->pbf_ratio = pSharpParamsV1->pbf_ratio[j];
            pTuningISOV2->pbf_add = pSharpParamsV1->pbf_add[j];
            pTuningISOV2->gaus_ratio = pSharpParamsV1->gaus_ratio[j];

            pTuningISOV2->sharp_ratio = pSharpParamsV1->sharp_ratio[j];
            pTuningISOV2->bf_gain = pSharpParamsV1->bf_gain[j];
            pTuningISOV2->bf_ratio = pSharpParamsV1->bf_ratio[j];
            pTuningISOV2->bf_add = pSharpParamsV1->bf_add[j];

            pLumaParaV2 = &pTuningISOV2->luma_para;
            for(int k = 0; k < 8; k++) {
                pLumaParaV2->luma_point[k] = pSharpParamsV1->luma_point[k];
                pLumaParaV2->luma_sigma[k] = pSharpParamsV1->luma_sigma[j][k];
                pLumaParaV2->hf_clip[k] = pSharpParamsV1->lum_clip_h[j][k];
                pLumaParaV2->local_sharp_strength[k] = pSharpParamsV1->ehf_th[j][k];
            }
            pKernelCoeffV2 = &pTuningISOV2->kernel_para;
            for(int k = 0; k < 3; k++) {
                pKernelCoeffV2->prefilter_coeff[k] = pSharpParamsV1->kernel_pre_bila_filter[k][j];
                pKernelCoeffV2->GaussianFilter_coeff[k] = pSharpParamsV1->kernel_range_filter[k][j];
                pKernelCoeffV2->hfBilateralFilter_coeff[k] = pSharpParamsV1->kernel_bila_filter[k][j];
            }
        }
    }

    return 0;
}


int sharpV3_calibdbV2_to_calibdb(CalibDbV2_SharpV3_t *pCalibdbV2, struct list_head *pSharpList, int mode_idx)
{
    CalibDbV2_SharpV3_Tuning_t *pTuningParaV2 = NULL;
    CalibDbV2_SharpV3_T_ISO_t *pTuningISOV2 = NULL;
    CalibDbV2_SharpV3_Luma_t *pLumaParaV2 = NULL;
    CalibDbV2_SharpV3_kernel_t *pKernelCoeffV2 = NULL;

    if(pSharpList == NULL || pCalibdbV2 == NULL) {
        printf(" pCalibdb is NULL pointer\n");
        return -1;
    }

    Calibdb_Sharp_V3_t* pSharpProfile = NULL;
    CamCalibdbGetSharpV3ProfileByIdx(pSharpList, mode_idx, &pSharpProfile);
    if(pSharpProfile == NULL) {
        printf(" %s:%d bayernr can't get mode:%d para from calibdbv1\n",
               __FUNCTION__,
               __LINE__,
               mode_idx);
        return -1;
    }

    pTuningParaV2 = &pCalibdbV2->TuningPara;

    //assign the value
    pSharpProfile->enable = pTuningParaV2->enable;

    for(int i = 0; i < SHARPV3_SETTING_NUM; i++) {
        Calibdb_Sharp_params_V3_t *pSharpParamsV1 = NULL;
        CamCalibdbGetSharpV3SettingByIdx(&pSharpProfile->listHead, i, &pSharpParamsV1);
        if(pSharpParamsV1 == NULL) {
            printf("cnr can't get setting:%d from xml calibdb! \n", i);
            break;
        }

        strcpy(pSharpParamsV1->snr_mode, pTuningParaV2->Setting[i].SNR_Mode);
        strcpy(pSharpParamsV1->sensor_mode, pTuningParaV2->Setting[i].Sensor_Mode);

        for(int j = 0; j < SHARPV3_ISO_NUM; j++) {
            pTuningISOV2 = &pTuningParaV2->Setting[i].Tuning_ISO[j];

            pSharpParamsV1->iso[j] = pTuningISOV2->iso;
            pSharpParamsV1->pbf_gain[j] = pTuningISOV2->pbf_gain;
            pSharpParamsV1->pbf_add[j] = pTuningISOV2->pbf_ratio;
            pSharpParamsV1->pbf_ratio[j] = pTuningISOV2->pbf_add;
            pSharpParamsV1->gaus_ratio[j] = pTuningISOV2->gaus_ratio;

            pSharpParamsV1->sharp_ratio[j] = pTuningISOV2->sharp_ratio;
            pSharpParamsV1->bf_gain[j] = pTuningISOV2->bf_gain;
            pSharpParamsV1->bf_ratio[j] = pTuningISOV2->bf_ratio;
            pSharpParamsV1->bf_add[j] = pTuningISOV2->bf_add;

            pLumaParaV2 = &pTuningISOV2->luma_para;
            for(int k = 0; k < 8; k++) {
                pSharpParamsV1->luma_point[k] = pLumaParaV2->luma_point[k];
                pSharpParamsV1->luma_sigma[j][k] = pLumaParaV2->luma_sigma[k];
                pSharpParamsV1->lum_clip_h[j][k] = pLumaParaV2->hf_clip[k];
                pSharpParamsV1->ehf_th[j][k] = pLumaParaV2->local_sharp_strength[k];
            }
            pKernelCoeffV2 = &pTuningISOV2->kernel_para;
            for(int k = 0; k < 3; k++) {
                pSharpParamsV1->kernel_pre_bila_filter[k][j] = pKernelCoeffV2->prefilter_coeff[k];
                pSharpParamsV1->kernel_range_filter[k][j] = pKernelCoeffV2->GaussianFilter_coeff[k];
                pSharpParamsV1->kernel_bila_filter[k][j] = pKernelCoeffV2->hfBilateralFilter_coeff[k];
            }
        }
    }

    return 0;
}


void sharpV3_calibdbV2_free(CalibDbV2_SharpV3_t *pCalibdbV2)
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



