#include "bayernr_xml2json_v2.h"
#include "RkAiqCalibApi.h"


#define BAYERNRV2_SETTING_NUM (2)
#define BAYERNRV2_ISO_NUM   (13)

int bayernrV2_calibdb_to_calibdbV2(struct list_head *pBayernrList,  CalibDbV2_BayerNrV2_t *pCalibdbV2, int mode_idx)
{
    CalibDbV2_BayerNrV2_Calib_t *pCalibParaV2 = NULL;
    CalibDbV2_BayerNrV2_2d_t *p2dV2 = NULL;
    CalibDbV2_BayerNrV2_3d_t *p3dV2 = NULL;
    CalibDbV2_BayerNrV2_C_ISO_t *pCalibISOV2 = NULL;
    CalibDbV2_BayerNrV2_2d_ISO_t *p2dISOV2 = NULL;
    CalibDbV2_BayerNrV2_3d_ISO_t *p3dISOV2 = NULL;


    if(pBayernrList == NULL || pCalibdbV2 == NULL) {
        printf(" pCalibdb is NULL pointer\n");
        return -1;
    }

    CalibDb_Bayernr_V2_t* pBayenrProfile = NULL;
    CamCalibdbGetBayernrV2ProfileByIdx(pBayernrList, mode_idx, &pBayenrProfile);
    if(pBayenrProfile == NULL) {
        printf(" %s:%d bayernr can't get mode:%d para from calibdbv1\n",
               __FUNCTION__,
               __LINE__,
               mode_idx);
        return -1;
    }

    pCalibParaV2 = &pCalibdbV2->CalibPara;
    p2dV2 = &pCalibdbV2->Bayernr2D;
    p3dV2 = &pCalibdbV2->Bayernr3D;

    //malloc settting size
    pCalibParaV2->Setting = (CalibDbV2_BayerNrV2_C_Set_t *)malloc(BAYERNRV2_SETTING_NUM * sizeof(CalibDbV2_BayerNrV2_C_Set_t));
    memset(pCalibParaV2->Setting,  0x00, BAYERNRV2_SETTING_NUM * sizeof(CalibDbV2_BayerNrV2_C_Set_t));
    pCalibParaV2->Setting_len = BAYERNRV2_SETTING_NUM;
    p2dV2->Setting = (CalibDbV2_BayerNrV2_2d_Set_t *)malloc(BAYERNRV2_SETTING_NUM * sizeof(CalibDbV2_BayerNrV2_2d_Set_t));
    memset(p2dV2->Setting, 0x00, BAYERNRV2_SETTING_NUM * sizeof(CalibDbV2_BayerNrV2_2d_Set_t));
    p2dV2->Setting_len = BAYERNRV2_SETTING_NUM;
    p3dV2->Setting = (CalibDbV2_BayerNrV2_3d_Set_t *)malloc(BAYERNRV2_SETTING_NUM * sizeof(CalibDbV2_BayerNrV2_3d_Set_t));
    memset(p3dV2->Setting, 0x00, BAYERNRV2_SETTING_NUM * sizeof(CalibDbV2_BayerNrV2_3d_Set_t));
    p3dV2->Setting_len = BAYERNRV2_SETTING_NUM;

    //malloc iso size
    for(int i = 0; i < BAYERNRV2_SETTING_NUM; i++) {
        pCalibParaV2->Setting[i].Calib_ISO =  (CalibDbV2_BayerNrV2_CalibPara_Setting_ISO_t *)malloc(BAYERNRV2_ISO_NUM * sizeof(CalibDbV2_BayerNrV2_CalibPara_Setting_ISO_t));
        memset(pCalibParaV2->Setting[i].Calib_ISO, 0x00, BAYERNRV2_ISO_NUM * sizeof(CalibDbV2_BayerNrV2_CalibPara_Setting_ISO_t));
        pCalibParaV2->Setting[i].Calib_ISO_len = BAYERNRV2_ISO_NUM;

        p2dV2->Setting[i].Tuning_ISO = (CalibDbV2_BayerNrV2_Bayernr2d_Setting_ISO_t *)malloc(BAYERNRV2_ISO_NUM * sizeof(CalibDbV2_BayerNrV2_Bayernr2d_Setting_ISO_t));
        memset(p2dV2->Setting[i].Tuning_ISO, 0x00, BAYERNRV2_ISO_NUM * sizeof(CalibDbV2_BayerNrV2_Bayernr2d_Setting_ISO_t));
        p2dV2->Setting[i].Tuning_ISO_len = BAYERNRV2_ISO_NUM;

        p3dV2->Setting[i].Tuning_ISO = (CalibDbV2_BayerNrV2_Bayernr3d_Setting_ISO_t *)malloc(BAYERNRV2_ISO_NUM * sizeof(CalibDbV2_BayerNrV2_Bayernr3d_Setting_ISO_t));
        memset(p3dV2->Setting[i].Tuning_ISO, 0x00, BAYERNRV2_ISO_NUM * sizeof(CalibDbV2_BayerNrV2_Bayernr3d_Setting_ISO_t));
        p3dV2->Setting[i].Tuning_ISO_len = BAYERNRV2_ISO_NUM;
    }

    //assign the value
    p2dV2->enable = pBayenrProfile->st2DParams.bayernrv2_2dnr_enable;
    p3dV2->enable = pBayenrProfile->st3DParams.bayernrv2_tnr_enable;

    for(int i = 0; i < BAYERNRV2_SETTING_NUM; i++) {
        Calibdb_Bayernr_2Dparams_V2_t *p2dParamCalibV1 = NULL;
        CamCalibdbGetBayernrV2Setting2DByIdx(&pBayenrProfile->st2DParams.listHead, i, &p2dParamCalibV1);
        CalibDb_Bayernr_3DParams_V2_t *p3dParamCalibV1 = NULL;
        CamCalibdbGetBayernrV2Setting3DByIdx(&pBayenrProfile->st3DParams.listHead, i, &p3dParamCalibV1);

        if(p2dParamCalibV1 == NULL || p3dParamCalibV1 == NULL) {
            printf("bayernr can't get setting:%d from xml calibdb! \n", i);
            break;
        }

        pCalibParaV2->Setting[i].SNR_Mode = strdup(p2dParamCalibV1->snr_mode);
        pCalibParaV2->Setting[i].Sensor_Mode = strdup(p2dParamCalibV1->sensor_mode);
        p2dV2->Setting[i].SNR_Mode = strdup(p2dParamCalibV1->snr_mode);
        p2dV2->Setting[i].Sensor_Mode = strdup(p2dParamCalibV1->sensor_mode);
        p3dV2->Setting[i].SNR_Mode = strdup(p3dParamCalibV1->snr_mode);
        p3dV2->Setting[i].Sensor_Mode = strdup(p3dParamCalibV1->sensor_mode);

        for(int j = 0; j < BAYERNRV2_ISO_NUM; j++) {
            pCalibISOV2 = &pCalibParaV2->Setting[i].Calib_ISO[j];
            p2dISOV2 = &p2dV2->Setting[i].Tuning_ISO[j];
            p3dISOV2 = &p3dV2->Setting[i].Tuning_ISO[j];

            //calib
            pCalibISOV2->iso = p2dParamCalibV1->iso[j];
            for(int k = 0; k < 16; k++) {
                pCalibISOV2->lumapoint[k] = p2dParamCalibV1->bayernrv2_filter_lumapoint_r[k];
                pCalibISOV2->sigma[k] = p2dParamCalibV1->bayernrv2_filter_sigma_r[j][k];
            }

            //2d
            p2dISOV2->iso = p2dParamCalibV1->iso[j];
            p2dISOV2->filter_strength = p2dParamCalibV1->bayernrv2_filter_strength_r[j];
            p2dISOV2->gauss_guide = p2dParamCalibV1->bayernrv2_gauss_guide_r[j];
            p2dISOV2->edgesofts = p2dParamCalibV1->bayernrv2_filter_edgesofts_r[j];
            p2dISOV2->ratio = p2dParamCalibV1->bayernrv2_filter_soft_threshold_ratio_r[j];
            p2dISOV2->weight = p2dParamCalibV1->bayernrv2_filter_out_wgt_r[j];

            //3d
            p3dISOV2->iso = p3dParamCalibV1->iso[j];
            p3dISOV2->filter_strength = p3dParamCalibV1->bayernrv2_tnr_filter_strength_r[j];
            p3dISOV2->sp_filter_strength = p3dParamCalibV1->bayernrv2_tnr_sp_filter_strength_r[j];
            p3dISOV2->lo_clipwgt = p3dParamCalibV1->bayernrv2_tnr_lo_clipwgt_r[j];
            p3dISOV2->hi_clipwgt = p3dParamCalibV1->bayernrv2_tnr_hi_clipwgt_r[j];
            p3dISOV2->softwgt = p3dParamCalibV1->bayernrv2_tnr_softwgt_r[j];


        }
    }

    return 0;

}



int bayernrV2_calibdbV2_to_calibdb(CalibDbV2_BayerNrV2_t *pCalibdbV2,  struct list_head *pBayernrList,  int mode_idx)
{
    CalibDbV2_BayerNrV2_Calib_t *pCalibParaV2 = NULL;
    CalibDbV2_BayerNrV2_2d_t *p2dV2 = NULL;
    CalibDbV2_BayerNrV2_3d_t *p3dV2 = NULL;
    CalibDbV2_BayerNrV2_C_ISO_t *pCalibISOV2 = NULL;
    CalibDbV2_BayerNrV2_2d_ISO_t *p2dISOV2 = NULL;
    CalibDbV2_BayerNrV2_3d_ISO_t *p3dISOV2 = NULL;

    CalibDb_Bayernr_V2_t* pBayenrProfile = NULL;
    Calibdb_Bayernr_2Dparams_V2_t *p2dParamCalibV1 = NULL;
    CalibDb_Bayernr_3DParams_V2_t *p3dParamCalibV1 = NULL;


    if(pBayernrList == NULL || pCalibdbV2 == NULL) {
        printf(" pCalibdb is NULL pointer\n");
        return -1;
    }

    CamCalibdbGetBayernrV2ProfileByIdx(pBayernrList, mode_idx, &pBayenrProfile);
    if(pBayenrProfile == NULL) {
        printf(" bayernr can't get mode:%d para from calibdbv1\n", mode_idx);
        return -1;
    }

    pCalibParaV2 = &pCalibdbV2->CalibPara;
    p2dV2 = &pCalibdbV2->Bayernr2D;
    p3dV2 = &pCalibdbV2->Bayernr3D;


    //2d assign the value
    for(int i = 0; i < p2dV2->Setting_len; i++) {
        p2dParamCalibV1 = NULL;
        CamCalibdbGetBayernrV2Setting2DByIdx(&pBayenrProfile->st2DParams.listHead, i, &p2dParamCalibV1);

        if(p2dParamCalibV1 == NULL) {
            printf("bayernr can't get setting:%d from xml calibdb! \n", i);
            break;
        }

        strcpy(p2dParamCalibV1->snr_mode, p2dV2->Setting[i].SNR_Mode);
        strcpy(p2dParamCalibV1->sensor_mode, p2dV2->Setting[i].Sensor_Mode);

        for(int j = 0; j < BAYERNRV2_ISO_NUM && i < p3dV2->Setting[i].Tuning_ISO_len; j++) {
            pCalibISOV2 = &pCalibParaV2->Setting[i].Calib_ISO[j];
            p2dISOV2 = &p2dV2->Setting[i].Tuning_ISO[j];

            //calib
            p2dParamCalibV1->iso[j] = pCalibISOV2->iso;
            for(int k = 0; k < 16; k++) {
                p2dParamCalibV1->bayernrv2_filter_lumapoint_r[k] = pCalibISOV2->lumapoint[k];
                p2dParamCalibV1->bayernrv2_filter_sigma_r[j][k] = pCalibISOV2->sigma[k];
            }

            //2d
            p2dParamCalibV1->iso[j] = p2dISOV2->iso;
            p2dParamCalibV1->bayernrv2_filter_strength_r[j] = p2dISOV2->filter_strength;
            p2dParamCalibV1->bayernrv2_gauss_guide_r[j] = p2dISOV2->gauss_guide;
            p2dParamCalibV1->bayernrv2_filter_edgesofts_r[j] = p2dISOV2->edgesofts;
            p2dParamCalibV1->bayernrv2_filter_soft_threshold_ratio_r[j] = p2dISOV2->ratio;
            p2dParamCalibV1->bayernrv2_filter_out_wgt_r[j] = p2dISOV2->weight;

        }
    }

    //3d
    for(int i = 0; i < p3dV2->Setting_len; i++) {
        p3dParamCalibV1 = NULL;
        CamCalibdbGetBayernrV2Setting3DByIdx(&pBayenrProfile->st3DParams.listHead, i, &p3dParamCalibV1);

        if(p3dParamCalibV1 == NULL) {
            printf("bayernr can't get setting:%d from xml calibdb! \n", i);
            break;
        }

        strcpy(p3dParamCalibV1->snr_mode, p3dV2->Setting[i].SNR_Mode);
        strcpy(p3dParamCalibV1->sensor_mode, p3dV2->Setting[i].Sensor_Mode);

        for(int j = 0; j < BAYERNRV2_ISO_NUM && i < p3dV2->Setting[i].Tuning_ISO_len; j++) {
            pCalibISOV2 = &pCalibParaV2->Setting[i].Calib_ISO[j];
            p3dISOV2 = &p3dV2->Setting[i].Tuning_ISO[j];

            //calib
            for(int k = 0; k < 16; k++) {
                p3dParamCalibV1->bayernrv2_lumapoint_r[k] = pCalibISOV2->lumapoint[k];
                p3dParamCalibV1->bayernrv2_sigma_r[j][k] = pCalibISOV2->sigma[k];
            }

            //3d
            p3dParamCalibV1->iso[j] = p3dISOV2->iso;
            p3dParamCalibV1->bayernrv2_tnr_filter_strength_r[j] = p3dISOV2->filter_strength;
            p3dParamCalibV1->bayernrv2_tnr_sp_filter_strength_r[j] = p3dISOV2->sp_filter_strength;
            p3dParamCalibV1->bayernrv2_tnr_lo_clipwgt_r[j] = p3dISOV2->lo_clipwgt;
            p3dParamCalibV1->bayernrv2_tnr_hi_clipwgt_r[j] = p3dISOV2->hi_clipwgt;
            p3dParamCalibV1->bayernrv2_tnr_softwgt_r[j] = p3dISOV2->softwgt;

        }
    }
    return 0;

}


void bayernrV2_calibdbV2_free(CalibDbV2_BayerNrV2_t *pCalibdbV2)
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

        if(pCalibdbV2->Bayernr2D.Setting != NULL) {
            for(int i = 0; i < pCalibdbV2->Bayernr2D.Setting_len; i++) {
                if(pCalibdbV2->Bayernr2D.Setting[i].Tuning_ISO != NULL) {
                    free(pCalibdbV2->Bayernr2D.Setting[i].Tuning_ISO );
                }
                if(pCalibdbV2->Bayernr2D.Setting[i].Sensor_Mode != NULL) {
                    free(pCalibdbV2->Bayernr2D.Setting[i].Sensor_Mode);
                }
                if(pCalibdbV2->Bayernr2D.Setting[i].SNR_Mode != NULL) {
                    free(pCalibdbV2->Bayernr2D.Setting[i].SNR_Mode);
                }
            }
            free(pCalibdbV2->Bayernr2D.Setting);
        }

        if(pCalibdbV2->Bayernr3D.Setting != NULL) {
            for(int i = 0; i < pCalibdbV2->Bayernr3D.Setting_len; i++) {
                if(pCalibdbV2->Bayernr3D.Setting[i].Tuning_ISO != NULL) {
                    free(pCalibdbV2->Bayernr3D.Setting[i].Tuning_ISO );
                }
                if(pCalibdbV2->Bayernr3D.Setting[i].Sensor_Mode != NULL) {
                    free(pCalibdbV2->Bayernr3D.Setting[i].Sensor_Mode);
                }
                if(pCalibdbV2->Bayernr3D.Setting[i].SNR_Mode != NULL) {
                    free(pCalibdbV2->Bayernr3D.Setting[i].SNR_Mode);
                }
            }
            free(pCalibdbV2->Bayernr3D.Setting);
        }

        free(pCalibdbV2);
    }

}

