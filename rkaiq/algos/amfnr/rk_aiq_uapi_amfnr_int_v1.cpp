#include "amfnr/rk_aiq_uapi_amfnr_int_v1.h"
#include "amfnr/rk_aiq_amfnr_algo_mfnr_v1.h"
#include "amfnr/rk_aiq_types_amfnr_algo_prvt_v1.h"
#include "mfnr_xml2json_v1.h"

#if 1

#define MFNR_LUMA_TF_STRENGTH_MAX_PERCENT (50.0)
#define MFNR_CHROMA_TF_STRENGTH_MAX_PERCENT (50.0)
XCamReturn
rk_aiq_uapi_amfnr_SetAttrib_v1(RkAiqAlgoContext *ctx,
                               rk_aiq_mfnr_attrib_v1_t *attr,
                               bool need_sync)
{

    Amfnr_Context_V1_t* pAmfnrCtx = (Amfnr_Context_V1_t*)ctx;

    pAmfnrCtx->eMode = attr->eMode;
    pAmfnrCtx->stAuto = attr->stAuto;
    pAmfnrCtx->stManual = attr->stManual;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi_amfnr_GetAttrib_v1(const RkAiqAlgoContext *ctx,
                               rk_aiq_mfnr_attrib_v1_t *attr)
{

    Amfnr_Context_V1_t* pAmfnrCtx = (Amfnr_Context_V1_t*)ctx;

    attr->eMode = pAmfnrCtx->eMode;
    memcpy(&attr->stAuto, &pAmfnrCtx->stAuto, sizeof(Amfnr_Auto_Attr_V1_t));
    memcpy(&attr->stManual, &pAmfnrCtx->stManual, sizeof(Amfnr_Manual_Attr_V1_t));

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi_amfnr_SetIQPara_v1(RkAiqAlgoContext *ctx,
                               rk_aiq_mfnr_IQPara_V1_t *pPara,
                               bool need_sync)
{
#ifdef RKAIQ_ENABLE_PARSER_V1
    Amfnr_Context_V1_t* pAmfnrCtx = (Amfnr_Context_V1_t*)ctx;

    CalibDb_MFNR_2_t calibdb_2;
    memset(&calibdb_2, 0x00, sizeof(calibdb_2));
    calibdb_2.mode_num = sizeof(pPara->stMfnrPara.mode_cell) / sizeof(CalibDb_MFNR_ModeCell_t);
    calibdb_2.mode_cell = (CalibDb_MFNR_ModeCell_t *)malloc(calibdb_2.mode_num * sizeof(CalibDb_MFNR_ModeCell_t));

    calibdb_2.enable = pPara->stMfnrPara.enable;
    memcpy(calibdb_2.version, pPara->stMfnrPara.version, sizeof(pPara->stMfnrPara.version));
    calibdb_2.local_gain_en = pPara->stMfnrPara.local_gain_en;
    calibdb_2.motion_detect_en = pPara->stMfnrPara.motion_detect_en;
    calibdb_2.mode_3to1 = pPara->stMfnrPara.mode_3to1;
    calibdb_2.max_level = pPara->stMfnrPara.max_level;
    calibdb_2.max_level_uv = pPara->stMfnrPara.max_level_uv;
    calibdb_2.back_ref_num = pPara->stMfnrPara.back_ref_num;
    for(int i = 0; i < 4; i++) {
        calibdb_2.uv_ratio[i] = pPara->stMfnrPara.uv_ratio[i];
    }
    for(int i = 0; i < calibdb_2.mode_num; i++) {
        calibdb_2.mode_cell[i] = pPara->stMfnrPara.mode_cell[i];
    }
    pAmfnrCtx->isIQParaUpdate = true;

#if(AMFNR_USE_JSON_PARA_V1)
    mfnrV1_calibdb_to_calibdbV2(&calibdb_2, &pAmfnrCtx->mfnr_v1, 0);
#else
    pAmfnrCtx->stMfnrCalib.enable = calibdb_2.enable;
    memcpy(pAmfnrCtx->stMfnrCalib.version, calibdb_2.version, sizeof(pPara->stMfnrPara.version));
    pAmfnrCtx->stMfnrCalib.local_gain_en = calibdb_2.local_gain_en;
    pAmfnrCtx->stMfnrCalib.motion_detect_en = calibdb_2.motion_detect_en;
    pAmfnrCtx->stMfnrCalib.mode_3to1 = calibdb_2.mode_3to1;
    pAmfnrCtx->stMfnrCalib.max_level = calibdb_2.max_level;
    pAmfnrCtx->stMfnrCalib.max_level_uv = calibdb_2.max_level_uv;
    pAmfnrCtx->stMfnrCalib.back_ref_num = calibdb_2.back_ref_num;
    for(int i = 0; i < 4; i++) {
        pAmfnrCtx->stMfnrCalib.uv_ratio[i] = calibdb_2.uv_ratio[i];
    }
    for(int i = 0; i < calibdb_2.mode_num && i < pAmfnrCtx->stMfnrCalib.mode_num; i++) {
        pAmfnrCtx->stMfnrCalib.mode_cell[i] = calibdb_2.mode_cell[i];
    }
#endif

    free(calibdb_2.mode_cell);

    return XCAM_RETURN_NO_ERROR;
#else
    return XCAM_RETURN_ERROR_PARAM;
#endif
}


XCamReturn
rk_aiq_uapi_amfnr_GetIQPara_v1(RkAiqAlgoContext *ctx,
                               rk_aiq_mfnr_IQPara_V1_t *pPara)
{
#ifdef RKAIQ_ENABLE_PARSER_V1
    Amfnr_Context_V1_t* pAmfnrCtx = (Amfnr_Context_V1_t*)ctx;

    if(ctx != NULL && pPara != NULL) {
        CalibDb_MFNR_2_t calibdb_2;
        memset(&calibdb_2, 0x00, sizeof(calibdb_2));
        calibdb_2.mode_num = sizeof(pPara->stMfnrPara.mode_cell) / sizeof(CalibDb_MFNR_ModeCell_t);
        calibdb_2.mode_cell = (CalibDb_MFNR_ModeCell_t *)malloc(calibdb_2.mode_num * sizeof(CalibDb_MFNR_ModeCell_t));

#if(AMFNR_USE_JSON_PARA_V1)
        mfnrV1_calibdbV2_to_calibdb(&pAmfnrCtx->mfnr_v1, &calibdb_2, 0);
#else
        calibdb_2.enable = pAmfnrCtx->stMfnrCalib.enable;
        memcpy(calibdb_2.version, pAmfnrCtx->stMfnrCalib.version, sizeof(pPara->stMfnrPara.version));
        calibdb_2.local_gain_en = pAmfnrCtx->stMfnrCalib.local_gain_en;
        calibdb_2.motion_detect_en = pAmfnrCtx->stMfnrCalib.motion_detect_en;
        calibdb_2.mode_3to1 = pAmfnrCtx->stMfnrCalib.mode_3to1;
        calibdb_2.max_level = pAmfnrCtx->stMfnrCalib.max_level;
        calibdb_2.max_level_uv = pAmfnrCtx->stMfnrCalib.max_level_uv;
        calibdb_2.back_ref_num = pAmfnrCtx->stMfnrCalib.back_ref_num;
        for(int i = 0; i < 4; i++) {
            calibdb_2.uv_ratio[i] = pAmfnrCtx->stMfnrCalib.uv_ratio[i];
        }
        for(int i = 0; i < calibdb_2.mode_num && i < pAmfnrCtx->stMfnrCalib.mode_num; i++) {
            calibdb_2.mode_cell[i] = pAmfnrCtx->stMfnrCalib.mode_cell[i];
        }
#endif

        memset(&pPara->stMfnrPara, 0x00, sizeof(CalibDb_MFNR_t));
        pPara->stMfnrPara.enable = calibdb_2.enable;
        memcpy(pPara->stMfnrPara.version, calibdb_2.version, sizeof(pPara->stMfnrPara.version));
        pPara->stMfnrPara.local_gain_en = calibdb_2.local_gain_en;
        pPara->stMfnrPara.motion_detect_en = calibdb_2.motion_detect_en;
        pPara->stMfnrPara.mode_3to1 = calibdb_2.mode_3to1;
        pPara->stMfnrPara.max_level = calibdb_2.max_level;
        pPara->stMfnrPara.max_level_uv = calibdb_2.max_level_uv;
        pPara->stMfnrPara.back_ref_num = calibdb_2.back_ref_num;
        for(int i = 0; i < 4; i++) {
            pPara->stMfnrPara.uv_ratio[i] = calibdb_2.uv_ratio[i];
        }
        for(int i = 0; i < calibdb_2.mode_num && i < CALIBDB_MAX_MODE_NUM; i++) {
            pPara->stMfnrPara.mode_cell[i] = calibdb_2.mode_cell[i];
        }

        free(calibdb_2.mode_cell);
    }
    return XCAM_RETURN_NO_ERROR;
#else
    return XCAM_RETURN_ERROR_PARAM;
#endif
}


XCamReturn
rk_aiq_uapi_amfnr_SetLumaTFStrength_v1(const RkAiqAlgoContext *ctx,
                                       float fPercent)
{
    Amfnr_Context_V1_t* pAmfnrCtx = (Amfnr_Context_V1_t*)ctx;

    float fStrength = 1.0f;
    float fMax = MFNR_LUMA_TF_STRENGTH_MAX_PERCENT;


    if(fPercent <= 0.5) {
        fStrength =  fPercent / 0.5;
    } else {
        fStrength = (fPercent - 0.5) * (fMax - 1) * 2 + 1;
    }


    pAmfnrCtx->fLuma_TF_Strength = fStrength;


    printf("%s:%d fPercent:%f strenght:%f amfnr:%f\n ", __FUNCTION__, __LINE__, fPercent, fStrength, pAmfnrCtx->fLuma_TF_Strength);
    return XCAM_RETURN_NO_ERROR;
}



XCamReturn
rk_aiq_uapi_amfnr_GetLumaTFStrength_v1(const RkAiqAlgoContext *ctx,
                                       float *pPercent)
{
    Amfnr_Context_V1_t* pAmfnrCtx = (Amfnr_Context_V1_t*)ctx;

    float fStrength = 1.0f;
    float fMax = MFNR_LUMA_TF_STRENGTH_MAX_PERCENT;


    fStrength = pAmfnrCtx->fLuma_TF_Strength;

    if(fStrength <= 1) {
        *pPercent = fStrength * 0.5;
    } else {
        *pPercent = (fStrength - 1) / ((fMax - 1) * 2) + 0.5;
    }

    printf("%s:%d amfnr:%f fStrength:%f pPercent:%f\n ", __FUNCTION__, __LINE__, pAmfnrCtx->fLuma_TF_Strength, fStrength, *pPercent);
    return XCAM_RETURN_NO_ERROR;
}


XCamReturn
rk_aiq_uapi_amfnr_SetChromaTFStrength_v1(const RkAiqAlgoContext *ctx,
        float fPercent)
{
    Amfnr_Context_V1_t* pAmfnrCtx = (Amfnr_Context_V1_t*)ctx;

    float fStrength = 1.0;
    float fMax = MFNR_CHROMA_TF_STRENGTH_MAX_PERCENT;

    if(fPercent <= 0.5) {
        fStrength =  fPercent / 0.5;
    } else {
        fStrength = (fPercent - 0.5) * (fMax - 1) * 2 + 1;
    }

    pAmfnrCtx->fChroma_TF_Strength = fStrength;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi_amfnr_GetChromaTFStrength_v1(const RkAiqAlgoContext *ctx,
        float *pPercent)
{
    Amfnr_Context_V1_t* pAmfnrCtx = (Amfnr_Context_V1_t*)ctx;

    float fStrength = 1.0;
    float fMax = MFNR_CHROMA_TF_STRENGTH_MAX_PERCENT;

    fStrength = pAmfnrCtx->fChroma_TF_Strength;

    if(fStrength <= 1) {
        *pPercent = fStrength * 0.5;
    } else {
        *pPercent = (fStrength - 1) / ((fMax - 1) * 2) + 0.5;
    }

    return XCAM_RETURN_NO_ERROR;
}



XCamReturn
rk_aiq_uapi_amfnr_SetJsonPara_v1(RkAiqAlgoContext *ctx,
                                 rk_aiq_mfnr_JsonPara_V1_t *pPara,
                                 bool need_sync)
{

    Amfnr_Context_V1_t* pAmfnrCtx = (Amfnr_Context_V1_t*)ctx;

    mfnr_calibdbV2_assign_v1(&pAmfnrCtx->mfnr_v1, &pPara->mfnr_v1);
    pAmfnrCtx->isIQParaUpdate = true;

    return XCAM_RETURN_NO_ERROR;
}



XCamReturn
rk_aiq_uapi_amfnr_GetJsonPara_v1(RkAiqAlgoContext *ctx,
                                 rk_aiq_mfnr_JsonPara_V1_t *pPara)
{

    Amfnr_Context_V1_t* pAmfnrCtx = (Amfnr_Context_V1_t*)ctx;

    mfnr_calibdbV2_assign_v1(&pPara->mfnr_v1, &pAmfnrCtx->mfnr_v1);

    return XCAM_RETURN_NO_ERROR;
}


#endif

