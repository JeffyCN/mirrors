#include "rk_aiq_uapi_anr_int.h"
#include "anr/rk_aiq_types_anr_algo_prvt.h"
#include "anr/rk_aiq_anr_algo.h"
#include "bayernr_xml2json_v1.h"
#include "mfnr_xml2json_v1.h"
#include "uvnr_xml2json_v1.h"
#include "ynr_xml2json_v1.h"



#define NR_STRENGTH_MAX_PERCENT (50.0)
#define NR_LUMA_TF_STRENGTH_MAX_PERCENT NR_STRENGTH_MAX_PERCENT
#define NR_LUMA_SF_STRENGTH_MAX_PERCENT (100.0)
#define NR_CHROMA_TF_STRENGTH_MAX_PERCENT NR_STRENGTH_MAX_PERCENT
#define NR_CHROMA_SF_STRENGTH_MAX_PERCENT NR_STRENGTH_MAX_PERCENT
#define NR_RAWNR_SF_STRENGTH_MAX_PERCENT (80.0)

XCamReturn
rk_aiq_uapi_anr_SetAttrib(RkAiqAlgoContext *ctx,
                          rk_aiq_nr_attrib_t *attr,
                          bool need_sync)
{

    ANRContext_t* pAnrCtx = (ANRContext_t*)ctx;

    pAnrCtx->eMode = attr->eMode;
    pAnrCtx->stAuto = attr->stAuto;
    pAnrCtx->stManual = attr->stManual;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi_anr_GetAttrib(const RkAiqAlgoContext *ctx,
                          rk_aiq_nr_attrib_t *attr)
{

    ANRContext_t* pAnrCtx = (ANRContext_t*)ctx;

    attr->eMode = pAnrCtx->eMode;
    memcpy(&attr->stAuto, &pAnrCtx->stAuto, sizeof(ANR_Auto_Attr_t));
    memcpy(&attr->stManual, &pAnrCtx->stManual, sizeof(ANR_Manual_Attr_t));

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi_anr_SetIQPara(RkAiqAlgoContext *ctx,
                          rk_aiq_nr_IQPara_t *pPara,
                          bool need_sync)
{
#ifdef RKAIQ_ENABLE_PARSER_V1

    ANRContext_t* pAnrCtx = (ANRContext_t*)ctx;

    if(pPara->module_bits & (1 << ANR_MODULE_BAYERNR)) {
        //pAnrCtx->stBayernrCalib = pPara->stBayernrPara;
        CalibDb_BayerNr_2_t stBayernr;
        stBayernr.mode_num = sizeof(pPara->stBayernrPara.mode_cell) / sizeof(CalibDb_BayerNr_ModeCell_t);
        stBayernr.mode_cell = (CalibDb_BayerNr_ModeCell_t *)malloc(stBayernr.mode_num * sizeof(CalibDb_BayerNr_ModeCell_t));
        stBayernr.enable = pPara->stBayernrPara.enable;
        strcpy(stBayernr.version, pPara->stBayernrPara.version);
        for(int i = 0; i < stBayernr.mode_num; i++) {
            stBayernr.mode_cell[i] = pPara->stBayernrPara.mode_cell[i];
        }

#if(ANR_USE_JSON_PARA)
        bayernrV1_calibdb_to_calibdbV2(&stBayernr, &pAnrCtx->bayernr_v1, 0);
#else
        pAnrCtx->stBayernrCalib.enable = stBayernr.enable;
        strcpy(pAnrCtx->stBayernrCalib.version, stBayernr.version);
        for(int i = 0; i < stBayernr.mode_num && i < pAnrCtx->stBayernrCalib.mode_num; i++) {
            pAnrCtx->stBayernrCalib.mode_cell[i] = stBayernr.mode_cell[i];
        }
        pAnrCtx->isIQParaUpdate = true;
#endif

        free(stBayernr.mode_cell);
    }

    if(pPara->module_bits & (1 << ANR_MODULE_MFNR)) {
        //pAnrCtx->stMfnrCalib = pPara->stMfnrPara;
        CalibDb_MFNR_2_t stMfnr;
        stMfnr.mode_num = sizeof(pPara->stMfnrPara.mode_cell) / sizeof(CalibDb_MFNR_ModeCell_t);
        stMfnr.mode_cell = (CalibDb_MFNR_ModeCell_t *)malloc(stMfnr.mode_num * sizeof(CalibDb_MFNR_ModeCell_t));

        stMfnr.enable = pPara->stMfnrPara.enable;
        strcpy(stMfnr.version, pPara->stMfnrPara.version);
        stMfnr.local_gain_en = pPara->stMfnrPara.local_gain_en;
        stMfnr.motion_detect_en = pPara->stMfnrPara.motion_detect_en;
        stMfnr.mode_3to1 = pPara->stMfnrPara.mode_3to1;
        stMfnr.max_level = pPara->stMfnrPara.max_level;
        stMfnr.max_level_uv = pPara->stMfnrPara.max_level_uv;
        stMfnr.back_ref_num = pPara->stMfnrPara.back_ref_num;
        for(int i = 0; i < 4; i++) {
            stMfnr.uv_ratio[i] = pPara->stMfnrPara.uv_ratio[i];
        }
        for(int i = 0; i < stMfnr.mode_num; i++) {
            stMfnr.mode_cell[i] = pPara->stMfnrPara.mode_cell[i];
        }

#if(ANR_USE_JSON_PARA)
        mfnrV1_calibdb_to_calibdbV2(&stMfnr, &pAnrCtx->mfnr_v1, 0);
#else
        pAnrCtx->stMfnrCalib.enable = stMfnr.enable;
        strcpy(pAnrCtx->stMfnrCalib.version, stMfnr.version);
        pAnrCtx->stMfnrCalib.local_gain_en = stMfnr.local_gain_en;
        pAnrCtx->stMfnrCalib.motion_detect_en = stMfnr.motion_detect_en;
        pAnrCtx->stMfnrCalib.mode_3to1 = stMfnr.mode_3to1;
        pAnrCtx->stMfnrCalib.max_level = stMfnr.max_level;
        pAnrCtx->stMfnrCalib.max_level_uv = stMfnr.max_level_uv;
        pAnrCtx->stMfnrCalib.back_ref_num = stMfnr.back_ref_num;
        for(int i = 0; i < 4; i++) {
            pAnrCtx->stMfnrCalib.uv_ratio[i] = stMfnr.uv_ratio[i];
        }
        for(int i = 0; i < stMfnr.mode_num && i < pAnrCtx->stMfnrCalib.mode_num; i++) {
            pAnrCtx->stMfnrCalib.mode_cell[i] = stMfnr.mode_cell[i];
        }
#endif
        pAnrCtx->isIQParaUpdate = true;

        free(stMfnr.mode_cell);
    }

    if(pPara->module_bits & (1 << ANR_MODULE_UVNR)) {
        //pAnrCtx->stUvnrCalib = pPara->stUvnrPara;
        CalibDb_UVNR_2_t stUvnr;
        stUvnr.mode_num = sizeof(pPara->stUvnrPara.mode_cell) / sizeof(CalibDb_UVNR_ModeCell_t);
        stUvnr.mode_cell = (CalibDb_UVNR_ModeCell_t *)malloc(stUvnr.mode_num * sizeof(CalibDb_UVNR_ModeCell_t));
        stUvnr.enable = pPara->stUvnrPara.enable;
        strcpy(stUvnr.version, pPara->stUvnrPara.version);
        for(int i = 0; i < stUvnr.mode_num; i++) {
            stUvnr.mode_cell[i] = pPara->stUvnrPara.mode_cell[i];
        }

#if(ANR_USE_JSON_PARA)
        uvnrV1_calibdb_to_calibdbV2(&stUvnr, &pAnrCtx->uvnr_v1, 0);
#else
        pAnrCtx->stUvnrCalib.enable = stUvnr.enable;
        strcpy(pAnrCtx->stUvnrCalib.version, stUvnr.version);
        for(int i = 0; i < stUvnr.mode_num && i < pAnrCtx->stUvnrCalib.mode_num; i++) {
            pAnrCtx->stUvnrCalib.mode_cell[i] = stUvnr.mode_cell[i];
        }
#endif
        pAnrCtx->isIQParaUpdate = true;

        free(stUvnr.mode_cell);
    }

    if(pPara->module_bits & (1 << ANR_MODULE_YNR)) {
        //pAnrCtx->stYnrCalib = pPara->stYnrPara;
        CalibDb_YNR_2_t stYnr;
        stYnr.mode_num = sizeof(pPara->stYnrPara.mode_cell) / sizeof(CalibDb_YNR_ModeCell_t);
        stYnr.mode_cell = (CalibDb_YNR_ModeCell_t *)malloc(stYnr.mode_num * sizeof(CalibDb_YNR_ModeCell_t));
        stYnr.enable = pPara->stYnrPara.enable;
        strcpy(stYnr.version, pPara->stYnrPara.version);
        for(int i = 0; i < stYnr.mode_num; i++) {
            stYnr.mode_cell[i] = pPara->stYnrPara.mode_cell[i];
        }


#if(ANR_USE_JSON_PARA)
        ynrV1_calibdb_to_calibdbV2(&stYnr, &pAnrCtx->ynr_v1, 0);
#else
        pAnrCtx->stYnrCalib.enable = stYnr.enable;
        strcpy(pAnrCtx->stYnrCalib.version, stYnr.version);
        for(int i = 0; i < stYnr.mode_num && i < pAnrCtx->stYnrCalib.mode_num; i++) {
            pAnrCtx->stYnrCalib.mode_cell[i] = stYnr.mode_cell[i];
        }
#endif
        pAnrCtx->isIQParaUpdate = true;

        free(stYnr.mode_cell);
    }

    return XCAM_RETURN_NO_ERROR;
#else
    return XCAM_RETURN_ERROR_PARAM;
#endif
}


XCamReturn
rk_aiq_uapi_anr_GetIQPara(RkAiqAlgoContext *ctx,
                          rk_aiq_nr_IQPara_t *pPara)
{
#ifdef RKAIQ_ENABLE_PARSER_V1

    ANRContext_t* pAnrCtx = (ANRContext_t*)ctx;

    //pPara->stBayernrPara = pAnrCtx->stBayernrCalib;
    CalibDb_BayerNr_2_t stBayernr;
    stBayernr.mode_num = sizeof(pPara->stBayernrPara.mode_cell) / sizeof(CalibDb_BayerNr_ModeCell_t);
    stBayernr.mode_cell = (CalibDb_BayerNr_ModeCell_t *)malloc(stBayernr.mode_num * sizeof(CalibDb_BayerNr_ModeCell_t));

#if(ANR_USE_JSON_PARA)
    bayernrV1_calibdbV2_to_calibdb(&pAnrCtx->bayernr_v1, &stBayernr, 0);
#else
    stBayernr.enable = pAnrCtx->stBayernrCalib.enable;
    strcpy(stBayernr.version, pAnrCtx->stBayernrCalib.version);
    for(int i = 0; i < pAnrCtx->stBayernrCalib.mode_num && i < stBayernr.mode_num; i++) {
        stBayernr.mode_cell[i] = pAnrCtx->stBayernrCalib.mode_cell[i];
    }
#endif

    memset(&pPara->stBayernrPara, 0x00, sizeof(CalibDb_BayerNr_t));
    pPara->stBayernrPara.enable = stBayernr.enable;
    strcpy(pPara->stBayernrPara.version, stBayernr.version);
    for(int i = 0; i < stBayernr.mode_num; i++) {
        pPara->stBayernrPara.mode_cell[i] = stBayernr.mode_cell[i];
    }
    free(stBayernr.mode_cell);


    //pPara->stMfnrPara = pAnrCtx->stMfnrCalib;
    CalibDb_MFNR_2_t stMfnr;
    stMfnr.mode_num = sizeof(pPara->stMfnrPara.mode_cell) / sizeof(CalibDb_MFNR_ModeCell_t);
    stMfnr.mode_cell = (CalibDb_MFNR_ModeCell_t *)malloc(stMfnr.mode_num * sizeof(CalibDb_MFNR_ModeCell_t));
#if(ANR_USE_JSON_PARA)
    mfnrV1_calibdbV2_to_calibdb(&pAnrCtx->mfnr_v1, &stMfnr, 0);
#else
    stMfnr.enable = pAnrCtx->stMfnrCalib.enable;
    strcpy(stMfnr.version, pAnrCtx->stMfnrCalib.version);
    stMfnr.local_gain_en = pAnrCtx->stMfnrCalib.local_gain_en;
    stMfnr.motion_detect_en = pAnrCtx->stMfnrCalib.motion_detect_en;
    stMfnr.mode_3to1 = pAnrCtx->stMfnrCalib.mode_3to1;
    stMfnr.max_level = pAnrCtx->stMfnrCalib.max_level;
    stMfnr.max_level_uv = pAnrCtx->stMfnrCalib.max_level_uv;
    stMfnr.back_ref_num = pAnrCtx->stMfnrCalib.back_ref_num;
    for(int i = 0; i < 4; i++) {
        stMfnr.uv_ratio[i] = pAnrCtx->stMfnrCalib.uv_ratio[i];
    }
    for(int i = 0; i < stMfnr.mode_num && i < pAnrCtx->stMfnrCalib.mode_num; i++) {
        stMfnr.mode_cell[i] = pAnrCtx->stMfnrCalib.mode_cell[i];
    }
#endif

    memset(&pPara->stMfnrPara, 0x00, sizeof(CalibDb_MFNR_t));
    pPara->stMfnrPara.enable = stMfnr.enable;
    strcpy(pPara->stMfnrPara.version, stMfnr.version);
    pPara->stMfnrPara.local_gain_en = stMfnr.local_gain_en;
    pPara->stMfnrPara.motion_detect_en = stMfnr.motion_detect_en;
    pPara->stMfnrPara.mode_3to1 = stMfnr.mode_3to1;
    pPara->stMfnrPara.max_level = stMfnr.max_level;
    pPara->stMfnrPara.max_level_uv = stMfnr.max_level_uv;
    pPara->stMfnrPara.back_ref_num = stMfnr.back_ref_num;
    for(int i = 0; i < 4; i++) {
        pPara->stMfnrPara.uv_ratio[i] = stMfnr.uv_ratio[i];
    }
    for(int i = 0; i < stMfnr.mode_num; i++) {
        pPara->stMfnrPara.mode_cell[i] = stMfnr.mode_cell[i];
    }
    free(stMfnr.mode_cell);


    //pPara->stUvnrPara = pAnrCtx->stUvnrCalib;
    CalibDb_UVNR_2_t stUvnr;
    stUvnr.mode_num = sizeof(pPara->stUvnrPara.mode_cell) / sizeof(CalibDb_UVNR_ModeCell_t);
    stUvnr.mode_cell = (CalibDb_UVNR_ModeCell_t *)malloc(stUvnr.mode_num * sizeof(CalibDb_UVNR_ModeCell_t));
#if(ANR_USE_JSON_PARA)
    uvnrV1_calibdbV2_to_calibdb(&pAnrCtx->uvnr_v1, &stUvnr, 0);
#else
    stUvnr.enable = pAnrCtx->stUvnrCalib.enable;
    strcpy(stUvnr.version, pAnrCtx->stUvnrCalib.version);
    for(int i = 0; i < stUvnr.mode_num && i < pAnrCtx->stUvnrCalib.mode_num; i++) {
        stUvnr.mode_cell[i] = pAnrCtx->stUvnrCalib.mode_cell[i];
    }
#endif
    memset(&pPara->stUvnrPara, 0x00, sizeof(CalibDb_UVNR_t));
    pPara->stUvnrPara.enable = stUvnr.enable;
    memcpy(pPara->stUvnrPara.version, stUvnr.version, sizeof(pPara->stUvnrPara.version));
    for(int i = 0; i < stUvnr.mode_num; i++) {
        pPara->stUvnrPara.mode_cell[i] = stUvnr.mode_cell[i];
    }
    free(stUvnr.mode_cell);


    //pPara->stYnrPara = pAnrCtx->stYnrCalib;
    CalibDb_YNR_2_t stYnr;
    stYnr.mode_num = sizeof(pPara->stYnrPara.mode_cell) / sizeof(CalibDb_YNR_ModeCell_t);
    stYnr.mode_cell = (CalibDb_YNR_ModeCell_t *)malloc(stYnr.mode_num * sizeof(CalibDb_YNR_ModeCell_t));
#if(ANR_USE_JSON_PARA)
    ynrV1_calibdbV2_to_calibdb(&pAnrCtx->ynr_v1, &stYnr, 0);
#else
    stYnr.enable = pAnrCtx->stYnrCalib.enable;
    strcpy(stYnr.version, pAnrCtx->stYnrCalib.version);
    for(int i = 0; i < stYnr.mode_num && i < pAnrCtx->stYnrCalib.mode_num; i++) {
        stYnr.mode_cell[i] = pAnrCtx->stYnrCalib.mode_cell[i];
    }
#endif
    memset(&pPara->stYnrPara, 0x00, sizeof(CalibDb_YNR_t));
    pPara->stYnrPara.enable = stYnr.enable;
    memcpy(pPara->stYnrPara.version, stYnr.version, sizeof(pPara->stYnrPara.version));
    for(int i = 0; i < stYnr.mode_num; i++) {
        pPara->stYnrPara.mode_cell[i] = stYnr.mode_cell[i];
    }
    free(stYnr.mode_cell);

    return XCAM_RETURN_NO_ERROR;
#else
    return XCAM_RETURN_ERROR_PARAM;
#endif
}


XCamReturn
rk_aiq_uapi_anr_SetLumaSFStrength(const RkAiqAlgoContext *ctx,
                                  float fPercent)
{
    ANRContext_t* pAnrCtx = (ANRContext_t*)ctx;

    float fStrength = 1.0f;
    float fMax = NR_LUMA_SF_STRENGTH_MAX_PERCENT;


    if(fPercent <= 0.5) {
        fStrength =  fPercent / 0.5;
    } else {
        fStrength = (fPercent - 0.5) * (fMax - 1) * 2 + 1;
    }

    if(fStrength > 1) {
        pAnrCtx->fRawnr_SF_Strength = fStrength;
        pAnrCtx->fLuma_SF_Strength = 1;
    } else {
        pAnrCtx->fRawnr_SF_Strength = fStrength;
        pAnrCtx->fLuma_SF_Strength = fStrength;
    }

    return XCAM_RETURN_NO_ERROR;
}


XCamReturn
rk_aiq_uapi_anr_SetLumaTFStrength(const RkAiqAlgoContext *ctx,
                                  float fPercent)
{
    ANRContext_t* pAnrCtx = (ANRContext_t*)ctx;

    float fStrength = 1.0;
    float fMax = NR_LUMA_TF_STRENGTH_MAX_PERCENT;

    if(fPercent <= 0.5) {
        fStrength =  fPercent / 0.5;
    } else {
        fStrength = (fPercent - 0.5) * (fMax - 1) * 2 + 1;
    }

    pAnrCtx->fLuma_TF_Strength = fStrength;

    return XCAM_RETURN_NO_ERROR;
}


XCamReturn
rk_aiq_uapi_anr_GetLumaSFStrength(const RkAiqAlgoContext *ctx,
                                  float *pPercent)
{
    ANRContext_t* pAnrCtx = (ANRContext_t*)ctx;

    float fStrength = 1.0f;
    float fMax = NR_LUMA_SF_STRENGTH_MAX_PERCENT;


    fStrength = pAnrCtx->fRawnr_SF_Strength;

    if(fStrength <= 1) {
        *pPercent = fStrength * 0.5;
    } else {
        *pPercent = (fStrength - 1) / ((fMax - 1) * 2) + 0.5;
    }

    return XCAM_RETURN_NO_ERROR;
}


XCamReturn
rk_aiq_uapi_anr_GetLumaTFStrength(const RkAiqAlgoContext *ctx,
                                  float *pPercent)
{
    ANRContext_t* pAnrCtx = (ANRContext_t*)ctx;

    float fStrength = 1.0;
    float fMax = NR_LUMA_TF_STRENGTH_MAX_PERCENT;

    fStrength = pAnrCtx->fLuma_TF_Strength;

    if(fStrength <= 1) {
        *pPercent = fStrength * 0.5;
    } else {
        *pPercent = (fStrength - 1) / ((fMax - 1) * 2) + 0.5;
    }

    return XCAM_RETURN_NO_ERROR;
}


XCamReturn
rk_aiq_uapi_anr_SetChromaSFStrength(const RkAiqAlgoContext *ctx,
                                    float fPercent)
{
    ANRContext_t* pAnrCtx = (ANRContext_t*)ctx;

    float fStrength = 1.0f;
    float fMax = NR_CHROMA_SF_STRENGTH_MAX_PERCENT;

    if(fPercent <= 0.5) {
        fStrength =  fPercent / 0.5;
    } else {
        fStrength = (fPercent - 0.5) * (fMax - 1) * 2 + 1;
    }

    pAnrCtx->fChroma_SF_Strength = fStrength;

    return XCAM_RETURN_NO_ERROR;
}


XCamReturn
rk_aiq_uapi_anr_SetChromaTFStrength(const RkAiqAlgoContext *ctx,
                                    float fPercent)
{
    ANRContext_t* pAnrCtx = (ANRContext_t*)ctx;

    float fStrength = 1.0;
    float fMax = NR_CHROMA_TF_STRENGTH_MAX_PERCENT;

    if(fPercent <= 0.5) {
        fStrength =  fPercent / 0.5;
    } else {
        fStrength = (fPercent - 0.5) * (fMax - 1) * 2 + 1;
    }

    pAnrCtx->fChroma_TF_Strength = fStrength;

    return XCAM_RETURN_NO_ERROR;
}


XCamReturn
rk_aiq_uapi_anr_GetChromaSFStrength(const RkAiqAlgoContext *ctx,
                                    float *pPercent)
{
    ANRContext_t* pAnrCtx = (ANRContext_t*)ctx;

    float fStrength = 1.0f;
    float fMax = NR_CHROMA_SF_STRENGTH_MAX_PERCENT;

    fStrength = pAnrCtx->fChroma_SF_Strength;


    if(fStrength <= 1) {
        *pPercent = fStrength * 0.5;
    } else {
        *pPercent =  (fStrength - 1) / ((fMax - 1) * 2) + 0.5;
    }


    return XCAM_RETURN_NO_ERROR;
}


XCamReturn
rk_aiq_uapi_anr_GetChromaTFStrength(const RkAiqAlgoContext *ctx,
                                    float *pPercent)
{
    ANRContext_t* pAnrCtx = (ANRContext_t*)ctx;

    float fStrength = 1.0;
    float fMax = NR_CHROMA_TF_STRENGTH_MAX_PERCENT;

    fStrength = pAnrCtx->fChroma_TF_Strength;

    if(fStrength <= 1) {
        *pPercent = fStrength * 0.5;
    } else {
        *pPercent = (fStrength - 1) / ((fMax - 1) * 2) + 0.5;
    }

    return XCAM_RETURN_NO_ERROR;
}


XCamReturn
rk_aiq_uapi_anr_SetRawnrSFStrength(const RkAiqAlgoContext *ctx,
                                   float fPercent)
{
    ANRContext_t* pAnrCtx = (ANRContext_t*)ctx;

    float fStrength = 1.0;
    float fMax = NR_RAWNR_SF_STRENGTH_MAX_PERCENT;

    if(fPercent <= 0.5) {
        fStrength =  fPercent / 0.5;
    } else {
        fStrength = (fPercent - 0.5) * (fMax - 1) * 2 + 1;
    }

    pAnrCtx->fRawnr_SF_Strength = fStrength;

    return XCAM_RETURN_NO_ERROR;
}


XCamReturn
rk_aiq_uapi_anr_GetRawnrSFStrength(const RkAiqAlgoContext *ctx,
                                   float *pPercent)
{
    ANRContext_t* pAnrCtx = (ANRContext_t*)ctx;

    float fStrength = 1.0f;
    float fMax = NR_RAWNR_SF_STRENGTH_MAX_PERCENT;

    fStrength = pAnrCtx->fRawnr_SF_Strength;


    if(fStrength <= 1) {
        *pPercent = fStrength * 0.5;
    } else {
        *pPercent = (fStrength - 1) / ((fMax - 1) * 2)  + 0.5;
    }


    return XCAM_RETURN_NO_ERROR;
}


XCamReturn
rk_aiq_uapi_anr_SetJsonPara(RkAiqAlgoContext *ctx,
                            rk_aiq_nr_JsonPara_t *pPara,
                            bool need_sync)
{

    ANRContext_t* pAnrCtx = (ANRContext_t*)ctx;

    if(pPara->module_bits & (1 << ANR_MODULE_BAYERNR)) {
        //pAnrCtx->stBayernrCalib = pPara->stBayernrPara;
        bayernr_calibdbV2_assign(&pAnrCtx->bayernr_v1, &pPara->bayernr_v1);
        pAnrCtx->isIQParaUpdate = true;
    }

    if(pPara->module_bits & (1 << ANR_MODULE_MFNR)) {
        //pAnrCtx->stMfnrCalib = pPara->stMfnrPara;
        mfnr_calibdbV2_assign(&pAnrCtx->mfnr_v1, &pPara->mfnr_v1);
        pAnrCtx->isIQParaUpdate = true;
    }

    if(pPara->module_bits & (1 << ANR_MODULE_UVNR)) {
        //pAnrCtx->stUvnrCalib = pPara->stUvnrPara;
        uvnr_calibdbV2_assign(&pAnrCtx->uvnr_v1, &pPara->uvnr_v1);
        pAnrCtx->isIQParaUpdate = true;
    }

    if(pPara->module_bits & (1 << ANR_MODULE_YNR)) {
        //pAnrCtx->stYnrCalib = pPara->stYnrPara;
        ynr_calibdbV2_assign(&pAnrCtx->ynr_v1, &pPara->ynr_v1);
        pAnrCtx->isIQParaUpdate = true;
    }

    return XCAM_RETURN_NO_ERROR;
}



XCamReturn
rk_aiq_uapi_anr_GetJsonPara(RkAiqAlgoContext *ctx,
                            rk_aiq_nr_JsonPara_t *pPara)
{

    ANRContext_t* pAnrCtx = (ANRContext_t*)ctx;

    //pPara->stBayernrPara = pAnrCtx->stBayernrCalib;
    bayernr_calibdbV2_assign(&pPara->bayernr_v1, &pAnrCtx->bayernr_v1);
    mfnr_calibdbV2_assign(&pPara->mfnr_v1, &pAnrCtx->mfnr_v1);
    uvnr_calibdbV2_assign(&pPara->uvnr_v1, &pAnrCtx->uvnr_v1);
    ynr_calibdbV2_assign(&pPara->ynr_v1, &pAnrCtx->ynr_v1);

    return XCAM_RETURN_NO_ERROR;
}


