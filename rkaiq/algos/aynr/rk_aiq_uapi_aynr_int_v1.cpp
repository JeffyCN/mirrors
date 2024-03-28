#include "aynr/rk_aiq_uapi_aynr_int_v1.h"
#include "aynr/rk_aiq_aynr_algo_ynr_v1.h"
#include "aynr/rk_aiq_types_aynr_algo_prvt_v1.h"
#include "ynr_xml2json_v1.h"

#if 1

#define YNR_LUMA_SF_STRENGTH_MAX_PERCENT (100.0)

XCamReturn
rk_aiq_uapi_aynr_SetAttrib_v1(RkAiqAlgoContext *ctx,
                              rk_aiq_ynr_attrib_v1_t *attr,
                              bool need_sync)
{

    Aynr_Context_V1_t* pAynrCtx = (Aynr_Context_V1_t*)ctx;

    pAynrCtx->eMode = attr->eMode;
    pAynrCtx->stAuto = attr->stAuto;
    pAynrCtx->stManual = attr->stManual;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi_aynr_GetAttrib_v1(const RkAiqAlgoContext *ctx,
                              rk_aiq_ynr_attrib_v1_t *attr)
{

    Aynr_Context_V1_t* pAynrCtx = (Aynr_Context_V1_t*)ctx;

    attr->eMode = pAynrCtx->eMode;
    memcpy(&attr->stAuto, &pAynrCtx->stAuto, sizeof(Aynr_Auto_Attr_V1_t));
    memcpy(&attr->stManual, &pAynrCtx->stManual, sizeof(Aynr_Manual_Attr_V1_t));

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi_aynr_SetIQPara_v1(RkAiqAlgoContext *ctx,
                              rk_aiq_ynr_IQPara_V1_t *pPara,
                              bool need_sync)
{
#ifdef RKAIQ_ENABLE_PARSER_V1
    Aynr_Context_V1_t* pAynrCtx = (Aynr_Context_V1_t*)ctx;

    CalibDb_YNR_2_t calibdb_2;
    memset(&calibdb_2, 0x00, sizeof(calibdb_2));
    calibdb_2.mode_num = sizeof(pPara->stYnrPara.mode_cell) / sizeof(CalibDb_YNR_ModeCell_t);
    calibdb_2.mode_cell = (CalibDb_YNR_ModeCell_t *)malloc(calibdb_2.mode_num * sizeof(CalibDb_YNR_ModeCell_t));

    calibdb_2.enable = pPara->stYnrPara.enable;
    memcpy(calibdb_2.version, pPara->stYnrPara.version, sizeof(pPara->stYnrPara.version));
    for(int i = 0; i < calibdb_2.mode_num; i++) {
        calibdb_2.mode_cell[i] = pPara->stYnrPara.mode_cell[i];
    }
    pAynrCtx->isIQParaUpdate = true;

#if(AYNR_USE_JSON_PARA_V1)
    ynrV1_calibdb_to_calibdbV2(&calibdb_2, &pAynrCtx->ynr_v1, 0);
#else
    pAynrCtx->stYnrCalib = calibdb_2;
    memcpy(pAynrCtx->stYnrCalib.version, calibdb_2.version, sizeof(pPara->stYnrPara.version));
    for(int i = 0; i < calibdb_2.mode_num && i < pAynrCtx->stYnrCalib.mode_num; i++) {
        pAynrCtx->stYnrCalib.mode_cell[i] = calibdb_2.mode_cell[i];
    }
#endif

    free(calibdb_2.mode_cell);

    return XCAM_RETURN_NO_ERROR;
#else
    return XCAM_RETURN_ERROR_PARAM;
#endif
}


XCamReturn
rk_aiq_uapi_aynr_GetIQPara_v1(RkAiqAlgoContext *ctx,
                              rk_aiq_ynr_IQPara_V1_t *pPara)
{
#ifdef RKAIQ_ENABLE_PARSER_V1
    Aynr_Context_V1_t* pAynrCtx = (Aynr_Context_V1_t*)ctx;

    if(ctx != NULL && pPara != NULL) {
        CalibDb_YNR_2_t calibdb_2;
        memset(&calibdb_2, 0x00, sizeof(calibdb_2));
        calibdb_2.mode_num = sizeof(pPara->stYnrPara.mode_cell) / sizeof(CalibDb_YNR_ModeCell_t);
        calibdb_2.mode_cell = (CalibDb_YNR_ModeCell_t *)malloc(calibdb_2.mode_num * sizeof(CalibDb_YNR_ModeCell_t));

#if(AYNR_USE_JSON_PARA_V1)
        ynrV1_calibdbV2_to_calibdb(&pAynrCtx->ynr_v1, &calibdb_2, 0);
#else
        calibdb_2.enable = pAynrCtx->stYnrCalib.enable;
        memcpy(calibdb_2.version, pAynrCtx->stYnrCalib.version, sizeof(pPara->stYnrPara.version));
        for(int i = 0; i < calibdb_2.mode_num && i < pAynrCtx->stYnrCalib.mode_num; i++) {
            calibdb_2.mode_cell[i] = pAynrCtx->stYnrCalib.mode_cell[i];
        }
#endif
        memset(&pPara->stYnrPara, 0x00, sizeof(CalibDb_YNR_t));
        pPara->stYnrPara.enable = calibdb_2.enable;
        memcpy(pPara->stYnrPara.version, calibdb_2.version, sizeof(pPara->stYnrPara.version));
        for(int i = 0; i < calibdb_2.mode_num && i < CALIBDB_MAX_MODE_NUM; i++) {
            pPara->stYnrPara.mode_cell[i] = calibdb_2.mode_cell[i];
        }

        free(calibdb_2.mode_cell);
    }

    return XCAM_RETURN_NO_ERROR;
#else
    return XCAM_RETURN_ERROR_PARAM;
#endif
}


XCamReturn
rk_aiq_uapi_aynr_SetLumaSFStrength_v1(const RkAiqAlgoContext *ctx,
                                      float fPercent)
{
    Aynr_Context_V1_t* pAynrCtx = (Aynr_Context_V1_t*)ctx;

    float fStrength = 1.0f;
    float fMax = YNR_LUMA_SF_STRENGTH_MAX_PERCENT;


    if(fPercent <= 0.5) {
        fStrength =  fPercent / 0.5;
    } else {
        fStrength = (fPercent - 0.5) * (fMax - 1) * 2 + 1;
    }

    if(fStrength > 1) {
        pAynrCtx->fLuma_SF_Strength = 1;
    } else {
        pAynrCtx->fLuma_SF_Strength = fStrength;
    }

    return XCAM_RETURN_NO_ERROR;
}



XCamReturn
rk_aiq_uapi_aynr_GetLumaSFStrength_v1(const RkAiqAlgoContext *ctx,
                                      float *pPercent)
{
    Aynr_Context_V1_t* pAynrCtx = (Aynr_Context_V1_t*)ctx;

    float fStrength = 1.0f;
    float fMax = YNR_LUMA_SF_STRENGTH_MAX_PERCENT;


    fStrength = pAynrCtx->fLuma_SF_Strength;

    if(fStrength <= 1) {
        *pPercent = fStrength * 0.5;
    } else {
        *pPercent = (fStrength - 1) / ((fMax - 1) * 2) + 0.5;
    }

    return XCAM_RETURN_NO_ERROR;
}


XCamReturn
rk_aiq_uapi_aynr_SetJsonPara_v1(RkAiqAlgoContext *ctx,
                                rk_aiq_ynr_JsonPara_V1_t *pPara,
                                bool need_sync)
{

    Aynr_Context_V1_t* pAynrCtx = (Aynr_Context_V1_t*)ctx;

    ynr_calibdbV2_assign_v1(&pAynrCtx->ynr_v1, &pPara->ynr_v1);
    pAynrCtx->isIQParaUpdate = true;

    return XCAM_RETURN_NO_ERROR;
}



XCamReturn
rk_aiq_uapi_aynr_GetJsonPara_v1(RkAiqAlgoContext *ctx,
                                rk_aiq_ynr_JsonPara_V1_t *pPara)
{

    Aynr_Context_V1_t* pAynrCtx = (Aynr_Context_V1_t*)ctx;

    ynr_calibdbV2_assign_v1(&pPara->ynr_v1, &pAynrCtx->ynr_v1);

    return XCAM_RETURN_NO_ERROR;
}


#endif

