#include "arawnr2/rk_aiq_uapi_abayernr_int_v2.h"
#include "arawnr2/rk_aiq_types_abayernr_algo_prvt_v2.h"
#include "RkAiqCalibApi.h"
#include "bayernr_xml2json_v2.h"



#if 1

#define RAWNR_LUMA_TF_STRENGTH_MAX_PERCENT (100.0)
#define RAWNR_LUMA_SF_STRENGTH_MAX_PERCENT (100.0)


XCamReturn
rk_aiq_uapi_arawnrV2_SetAttrib(RkAiqAlgoContext *ctx,
                               rk_aiq_bayernr_attrib_v2_t *attr,
                               bool need_sync)
{

    Abayernr_Context_V2_t* pCtx = (Abayernr_Context_V2_t*)ctx;

    pCtx->eMode = attr->eMode;
    pCtx->stAuto = attr->stAuto;
    pCtx->stManual = attr->stManual;
    pCtx->isReCalculate |= 1;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi_arawnrV2_GetAttrib(const RkAiqAlgoContext *ctx,
                               rk_aiq_bayernr_attrib_v2_t *attr)
{

    Abayernr_Context_V2_t* pCtx = (Abayernr_Context_V2_t*)ctx;

    attr->eMode = pCtx->eMode;
    memcpy(&attr->stAuto, &pCtx->stAuto, sizeof(attr->stAuto));
    memcpy(&attr->stManual, &pCtx->stManual, sizeof(attr->stManual));

    return XCAM_RETURN_NO_ERROR;
}

#if 0
XCamReturn
rk_aiq_uapi_arawnrV2_SetIQPara(RkAiqAlgoContext *ctx,
                               rk_aiq_bayernr_IQPara_V2_t *pPara,
                               bool need_sync)
{

    Abayernr_Context_V2_t* pCtx = (Abayernr_Context_V2_t*)ctx;


#if ABAYERNR_USE_JSON_FILE_V2
    bayernrV2_calibdb_to_calibdbV2(&pPara->listHead_mode, &pCtx->bayernr_v2, 0);
#else
    pCtx->list_bayernr_v2 = &pPara->listHead_mode;
#endif

    pCtx->isIQParaUpdate = true;
    return XCAM_RETURN_NO_ERROR;
}


XCamReturn
rk_aiq_uapi_arawnrV2_GetIQPara(RkAiqAlgoContext *ctx,
                               rk_aiq_bayernr_IQPara_V2_t *pPara)
{

    Abayernr_Context_V2_t* pCtx = (Abayernr_Context_V2_t*)ctx;

    if(pCtx == NULL && pPara == NULL) {
        return XCAM_RETURN_NO_ERROR;
    }

#if ABAYERNR_USE_JSON_FILE_V2
    INIT_LIST_HEAD(&pPara->listHead_mode);
    CalibDb_Bayernr_V2_t *pBayernrProfile = (CalibDb_Bayernr_V2_t *)malloc(sizeof(CalibDb_Bayernr_V2_t));
    if(NULL == pBayernrProfile) {
        LOGE("%s(%d): malloc fail(exit)\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_NO_ERROR;
    }
    memset(pBayernrProfile, 0x00, sizeof(CalibDb_Bayernr_V2_t));
    INIT_LIST_HEAD(&pBayernrProfile->st2DParams.listHead);
    INIT_LIST_HEAD(&pBayernrProfile->st3DParams.listHead);
    list_add_tail((struct list_head*)pBayernrProfile, pPara->listHead_mode);

    for(int i = 0; i < pCtx->bayernr_v2.Bayernr2D.Setting_len; i++) {
        Calibdb_Bayernr_2Dparams_V2_t *p2DParam = (Calibdb_Bayernr_2Dparams_V2_t *)malloc(sizeof(Calibdb_Bayernr_2Dparams_V2_t));
        if(NULL == p2DParam) {
            LOGE("%s(%d): malloc fail(exit)\n", __FUNCTION__, __LINE__);
            return XCAM_RETURN_NO_ERROR;
        }
        memset(p2DParam, 0x00, sizeof(Calibdb_Bayernr_2Dparams_V2_t));
        list_prepare_item(&p2DParam->listItem);
        list_add_tail(&p2DParam->listItem, &pBayernrProfile->st2DParams.listHead);
    }

    for(int i = 0; i < pCtx->bayernr_v2.Bayernr3D.Setting_len; i++) {
        CalibDb_Bayernr_3DParams_V2_t *p3DParam = (CalibDb_Bayernr_3DParams_V2_t *)malloc(sizeof(CalibDb_Bayernr_3DParams_V2_t));
        if(NULL == p3DParam) {
            LOGE("%s(%d): malloc fail(exit)\n", __FUNCTION__, __LINE__);
            return XCAM_RETURN_NO_ERROR;
        }
        memset(p3DParam, 0x00, sizeof(CalibDb_Bayernr_3DParams_V2_t));
        list_prepare_item(&p3DParam->listItem);
        list_add_tail(&p3DParam->listItem, &pBayernrProfile->st3DParams.listHead);
    }

    bayernrV2_calibdbV2_to_calibdb(&pCtx->bayernr_v2, &pPara->listHead_mode, 0);
#else
    pPara->listHead_mode = *pCtx->list_bayernr_v2;
#endif

    return XCAM_RETURN_NO_ERROR;
}
#endif

XCamReturn
rk_aiq_uapi_rawnrV2_SetSFStrength(const RkAiqAlgoContext *ctx,
                                  float fPercent)
{
    Abayernr_Context_V2_t* pCtx = (Abayernr_Context_V2_t*)ctx;

    float fStrength = 1.0f;
    float fMax = RAWNR_LUMA_SF_STRENGTH_MAX_PERCENT;


    if(fPercent <= 0.5) {
        fStrength =  fPercent / 0.5;
    } else {
        fStrength = (fPercent - 0.5) * (fMax - 1) * 2 + 1;
    }

    pCtx->fRawnr_SF_Strength = fStrength;
    pCtx->isReCalculate |= 1;

    return XCAM_RETURN_NO_ERROR;
}


XCamReturn
rk_aiq_uapi_rawnrV2_SetTFStrength(const RkAiqAlgoContext *ctx,
                                  float fPercent)
{
    Abayernr_Context_V2_t* pCtx = (Abayernr_Context_V2_t*)ctx;

    float fStrength = 1.0;
    float fMax = RAWNR_LUMA_TF_STRENGTH_MAX_PERCENT;

    if(fPercent <= 0.5) {
        fStrength =  fPercent / 0.5;
    } else {
        fStrength = (fPercent - 0.5) * (fMax - 1) * 2 + 1;
    }

    pCtx->fRawnr_TF_Strength = fStrength;
    pCtx->isReCalculate |= 1;

    return XCAM_RETURN_NO_ERROR;
}


XCamReturn
rk_aiq_uapi_rawnrV2_GetSFStrength(const RkAiqAlgoContext *ctx,
                                  float *pPercent)
{
    Abayernr_Context_V2_t* pCtx = (Abayernr_Context_V2_t*)ctx;

    float fStrength = 1.0f;
    float fMax = RAWNR_LUMA_SF_STRENGTH_MAX_PERCENT;


    fStrength = pCtx->fRawnr_SF_Strength;

    if(fStrength <= 1) {
        *pPercent = fStrength * 0.5;
    } else {
        *pPercent = (fStrength - 1) / ((fMax - 1) * 2) + 0.5;
    }

    return XCAM_RETURN_NO_ERROR;
}


XCamReturn
rk_aiq_uapi_rawnrV2_GetTFStrength(const RkAiqAlgoContext *ctx,
                                  float *pPercent)
{
    Abayernr_Context_V2_t* pCtx = (Abayernr_Context_V2_t*)ctx;

    float fStrength = 1.0;
    float fMax = RAWNR_LUMA_TF_STRENGTH_MAX_PERCENT;

    fStrength = pCtx->fRawnr_TF_Strength;

    if(fStrength <= 1) {
        *pPercent = fStrength * 0.5;
    } else {
        *pPercent = (fStrength - 1) / ((fMax - 1) * 2) + 0.5;
    }

    return XCAM_RETURN_NO_ERROR;
}


#endif

