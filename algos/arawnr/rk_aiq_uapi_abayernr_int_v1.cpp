#include "arawnr/rk_aiq_uapi_abayernr_int_v1.h"
#include "bayernr_xml2json_v1.h"
#include "arawnr/rk_aiq_types_abayernr_algo_prvt_v1.h"
#include "arawnr/rk_aiq_abayernr_algo_bayernr_v1.h"



#if 1
#define BAYERNR_RAWNR_SF_STRENGTH_MAX_PERCENT (80.0)

XCamReturn
rk_aiq_uapi_abayernr_SetAttrib_v1(RkAiqAlgoContext *ctx,
                                  rk_aiq_bayernr_attrib_v1_t *attr,
                                  bool need_sync)
{

    Abayernr_Context_V1_t* pCtx = (Abayernr_Context_V1_t*)ctx;

    pCtx->eMode = attr->eMode;
    pCtx->stAuto = attr->stAuto;
    pCtx->stManual = attr->stManual;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi_abayernr_GetAttrib_v1(const RkAiqAlgoContext *ctx,
                                  rk_aiq_bayernr_attrib_v1_t *attr)
{

    Abayernr_Context_V1_t* pCtx = (Abayernr_Context_V1_t*)ctx;

    attr->eMode = pCtx->eMode;
    memcpy(&attr->stAuto, &pCtx->stAuto, sizeof(Abayernr_Auto_Attr_V1_t));
    memcpy(&attr->stManual, &pCtx->stManual, sizeof(Abayernr_Manual_Attr_V1_t));

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi_abayernr_SetIQPara_v1(RkAiqAlgoContext *ctx,
                                  rk_aiq_bayernr_IQPara_V1_t *pPara,
                                  bool need_sync)
{
#ifdef RKAIQ_ENABLE_PARSER_V1
    Abayernr_Context_V1_t* pCtx = (Abayernr_Context_V1_t*)ctx;
    CalibDb_BayerNr_2_t Calibdb_2;

    printf("%s:%d\n", __FUNCTION__, __LINE__);
    if(pPara != NULL && pCtx != NULL) {
        memset(&Calibdb_2, 0x00, sizeof(CalibDb_BayerNr_2_t));
        Calibdb_2.mode_num = sizeof(pPara->stBayernrPara.mode_cell) / sizeof(CalibDb_BayerNr_ModeCell_t);
        Calibdb_2.mode_cell = (CalibDb_BayerNr_ModeCell_t *)malloc(Calibdb_2.mode_num * sizeof(CalibDb_BayerNr_ModeCell_t));
        Calibdb_2.enable = pPara->stBayernrPara.enable;
        memcpy(Calibdb_2.version, pPara->stBayernrPara.version, sizeof(pPara->stBayernrPara.version));
        for(int i = 0; i < Calibdb_2.mode_num; i++) {
            Calibdb_2.mode_cell[i] = pPara->stBayernrPara.mode_cell[i];
        }
        printf("%s:%d\n", __FUNCTION__, __LINE__);
#if ABAYERNR_USE_JSON_FILE_V1
        bayernrV1_calibdb_to_calibdbV2(&Calibdb_2, &pCtx->bayernr_v1, 0);
#else
        pCtx->stBayernrCalib.enable = Calibdb_2.enable;
        memcpy(pCtx->stBayernrCalib.version, Calibdb_2.version, sizeof(Calibdb_2.version));
        for(int i = 0; i < Calibdb_2.mode_num && i < pCtx->stBayernrCalib.mode_num; i++) {
            pCtx->stBayernrCalib.mode_cell[i] = Calibdb_2.mode_cell[i];
        }
#endif
        pCtx->isIQParaUpdate = true;
        printf("%s:%d\n", __FUNCTION__, __LINE__);
        free(Calibdb_2.mode_cell);
    }
    return XCAM_RETURN_NO_ERROR;
#else
    return XCAM_RETURN_ERROR_PARAM;
#endif
}


XCamReturn
rk_aiq_uapi_abayernr_GetIQPara_v1(RkAiqAlgoContext *ctx,
                                  rk_aiq_bayernr_IQPara_V1_t *pPara)
{
#ifdef RKAIQ_ENABLE_PARSER_V1
    Abayernr_Context_V1_t* pCtx = (Abayernr_Context_V1_t*)ctx;
    CalibDb_BayerNr_2_t Calibdb_2;

    if(pPara != NULL && pCtx != NULL) {
        memset(&Calibdb_2, 0x00, sizeof(CalibDb_BayerNr_2_t));
        Calibdb_2.mode_num = sizeof(pPara->stBayernrPara.mode_cell) / sizeof(CalibDb_BayerNr_ModeCell_t);
        Calibdb_2.mode_cell = (CalibDb_BayerNr_ModeCell_t *)malloc(Calibdb_2.mode_num * sizeof(CalibDb_BayerNr_ModeCell_t));

        printf("%s:%d\n", __FUNCTION__, __LINE__);
#if(ABAYERNR_USE_JSON_FILE_V1)
        bayernrV1_calibdbV2_to_calibdb(&pCtx->bayernr_v1, &Calibdb_2, 0);
#else
        Calibdb_2.enable = pCtx->stBayernrCalib.enable;
        memcpy(Calibdb_2.version, pCtx->stBayernrCalib.version, sizeof(pPara->stBayernrPara.version));
        for(int i = 0; i < Calibdb_2.mode_num && i < pCtx->stBayernrCalib.mode_num; i++) {
            Calibdb_2.mode_cell[i] = pCtx->stBayernrCalib.mode_cell[i];
        }
#endif

        printf("%s:%d\n", __FUNCTION__, __LINE__);
        memset(&pPara->stBayernrPara, 0x00, sizeof(CalibDb_BayerNr_t));
        pPara->stBayernrPara.enable = Calibdb_2.enable;
        memcpy(pPara->stBayernrPara.version, Calibdb_2.version, sizeof(Calibdb_2.version));
        for(int i = 0; i < Calibdb_2.mode_num && i < CALIBDB_MAX_MODE_NUM; i++) {
            pPara->stBayernrPara.mode_cell[i] = Calibdb_2.mode_cell[i];
        }

        free(Calibdb_2.mode_cell);
        printf("%s:%d\n", __FUNCTION__, __LINE__);
    }

    return XCAM_RETURN_NO_ERROR;
#else
    return XCAM_RETURN_ERROR_PARAM;
#endif
}


XCamReturn
rk_aiq_uapi_abayernr_SetRawnrSFStrength_v1(const RkAiqAlgoContext *ctx,
        float fPercent)
{
    Abayernr_Context_V1_t* pCtx = (Abayernr_Context_V1_t*)ctx;

    float fStrength = 1.0;
    float fMax = BAYERNR_RAWNR_SF_STRENGTH_MAX_PERCENT;

    if(fPercent <= 0.5) {
        fStrength =  fPercent / 0.5;
    } else {
        fStrength = (fPercent - 0.5) * (fMax - 1) * 2 + 1;
    }

    pCtx->fRawnr_SF_Strength = fStrength;

    return XCAM_RETURN_NO_ERROR;
}


XCamReturn
rk_aiq_uapi_abayernr_GetRawnrSFStrength_v1(const RkAiqAlgoContext *ctx,
        float *pPercent)
{
    Abayernr_Context_V1_t* pCtx = (Abayernr_Context_V1_t*)ctx;

    float fStrength = 1.0f;
    float fMax = BAYERNR_RAWNR_SF_STRENGTH_MAX_PERCENT;

    fStrength = pCtx->fRawnr_SF_Strength;


    if(fStrength <= 1) {
        *pPercent = fStrength * 0.5;
    } else {
        *pPercent = (fStrength - 1) / ((fMax - 1) * 2) + 0.5;
    }


    return XCAM_RETURN_NO_ERROR;
}



XCamReturn
rk_aiq_uapi_abayernr_SetJsonPara_v1(RkAiqAlgoContext *ctx,
                                    rk_aiq_bayernr_JsonPara_V1_t *pPara,
                                    bool need_sync)
{

    Abayernr_Context_V1_t* pCtx = (Abayernr_Context_V1_t*)ctx;

    if(pCtx != NULL && pPara != NULL) {
#if ABAYERNR_USE_JSON_FILE_V1
        bayernrV1_calibdbV2_assign(&pCtx->bayernr_v1, &pPara->bayernr_v1);
        pCtx->isIQParaUpdate = true;
#endif
    }

    return XCAM_RETURN_NO_ERROR;
}


XCamReturn
rk_aiq_uapi_abayernr_GetJsonPara_v1(RkAiqAlgoContext *ctx,
                                    rk_aiq_bayernr_JsonPara_V1_t *pPara)
{

    Abayernr_Context_V1_t* pCtx = (Abayernr_Context_V1_t*)ctx;

    if(pCtx != NULL && pPara != NULL) {
#if ABAYERNR_USE_JSON_FILE_V1
        bayernrV1_calibdbV2_assign(&pPara->bayernr_v1, &pCtx->bayernr_v1);
#endif
    }

    return XCAM_RETURN_NO_ERROR;
}


#endif

