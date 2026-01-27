#include "rk_aiq_uapi_asharp_int.h"
#include "asharp/rk_aiq_types_asharp_algo_prvt.h"
#include "asharp/rk_aiq_asharp_algo.h"
#include "sharp_xml2json_v1.h"
#include "edgefilter_xml2json_v1.h"




XCamReturn
rk_aiq_uapi_asharp_SetAttrib(RkAiqAlgoContext *ctx,
                             rk_aiq_sharp_attrib_t *attr,
                             bool need_sync)
{

    AsharpContext_t* pAsharpCtx = (AsharpContext_t*)ctx;

    pAsharpCtx->eMode = attr->eMode;
    pAsharpCtx->stAuto = attr->stAuto;
    pAsharpCtx->stManual = attr->stManual;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi_asharp_GetAttrib(const RkAiqAlgoContext *ctx,
                             rk_aiq_sharp_attrib_t *attr)
{

    AsharpContext_t* pAsharpCtx = (AsharpContext_t*)ctx;

    attr->eMode = pAsharpCtx->eMode;
    memcpy(&attr->stAuto, &pAsharpCtx->stAuto, sizeof(Asharp_Auto_Attr_t));
    memcpy(&attr->stManual, &pAsharpCtx->stManual, sizeof(Asharp_Manual_Attr_t));

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi_asharp_SetIQpara(RkAiqAlgoContext *ctx,
                             rk_aiq_sharp_IQpara_t *para,
                             bool need_sync)
{

#ifdef RKAIQ_ENABLE_PARSER_V1
    AsharpContext_t* pAsharpCtx = (AsharpContext_t*)ctx;

    if(para->module_bits & (1 << ASHARP_MODULE_SHARP)) {
        CalibDb_Sharp_2_t stSharpCalib;
        stSharpCalib.mode_num = sizeof(para->stSharpPara.mode_cell) / sizeof(CalibDb_Sharp_ModeCell_t);
        stSharpCalib.mode_cell = (CalibDb_Sharp_ModeCell_t *)malloc(stSharpCalib.mode_num * sizeof(CalibDb_Sharp_ModeCell_t));

        stSharpCalib.enable = para->stSharpPara.enable;
        memcpy(stSharpCalib.version, para->stSharpPara.version, sizeof(para->stSharpPara.version));
        for(int i = 0; i < 8; i++) {
            stSharpCalib.luma_point[i] = para->stSharpPara.luma_point[i];
        }
        for(int i = 0; i < stSharpCalib.mode_num; i++) {
            stSharpCalib.mode_cell[i] = para->stSharpPara.mode_cell[i];
        }
        pAsharpCtx->isIQParaUpdate = true;

#if(ASHARP_USE_JSON_PARA)
        sharpV1_calibdb_to_calibdbV2(&stSharpCalib, &pAsharpCtx->sharp_v1, 0);
#else
        pAsharpCtx->stSharpCalib.enable = stSharpCalib.enable;
        memcpy(pAsharpCtx->stSharpCalib.version, stSharpCalib.version, sizeof(para->stSharpPara.version));
        for(int i = 0; i < 8; i++) {
            pAsharpCtx->stSharpCalib.luma_point[i] = stSharpCalib.luma_point[i];
        }
        for(int i = 0; i < stSharpCalib.mode_num && i < pAsharpCtx->stSharpCalib.mode_num; i++) {
            pAsharpCtx->stSharpCalib.mode_cell[i] = stSharpCalib.mode_cell[i];
        }
#endif

        free(stSharpCalib.mode_cell);
    }

    if(para->module_bits & (1 << ASHARP_MODULE_EDGEFILTER)) {
        //pAsharpCtx->stEdgeFltCalib = para->stEdgeFltPara;
        CalibDb_EdgeFilter_2_t stEdgeFltCalib;
        stEdgeFltCalib.mode_num = sizeof(para->stEdgeFltPara.mode_cell) / sizeof(CalibDb_EdgeFilter_ModeCell_t);
        stEdgeFltCalib.mode_cell = (CalibDb_EdgeFilter_ModeCell_t *)malloc(stEdgeFltCalib.mode_num * sizeof(CalibDb_EdgeFilter_ModeCell_t));

        stEdgeFltCalib.enable = para->stEdgeFltPara.enable;
        memcpy(stEdgeFltCalib.version, para->stEdgeFltPara.version, sizeof(para->stEdgeFltPara.version));
        for(int i = 0; i < 8; i++) {
            stEdgeFltCalib.luma_point[i] = para->stEdgeFltPara.luma_point[i];
        }
        for(int i = 0; i < stEdgeFltCalib.mode_num; i++) {
            stEdgeFltCalib.mode_cell[i] = para->stEdgeFltPara.mode_cell[i];
        }

#if(ASHARP_USE_JSON_PARA)
        edgefilterV1_calibdb_to_calibdbV2(&stEdgeFltCalib, &pAsharpCtx->edgefilter_v1, 0);
#else
        pAsharpCtx->stEdgeFltCalib.enable = stEdgeFltCalib.enable;
        memcpy(pAsharpCtx->stEdgeFltCalib.version, stEdgeFltCalib.version, sizeof(stEdgeFltCalib.version));
        for(int i = 0; i < 8; i++) {
            pAsharpCtx->stEdgeFltCalib.luma_point[i] = stEdgeFltCalib.luma_point[i];
        }
        for(int i = 0; i < pAsharpCtx->stEdgeFltCalib.mode_num && i < stEdgeFltCalib.mode_num; i++) {
            pAsharpCtx->stEdgeFltCalib.mode_cell[i] = stEdgeFltCalib.mode_cell[i];
        }
#endif

        pAsharpCtx->isIQParaUpdate = true;

        free(stEdgeFltCalib.mode_cell);

    }

    return XCAM_RETURN_NO_ERROR;
#else
    return XCAM_RETURN_ERROR_PARAM;
#endif
}

XCamReturn
rk_aiq_uapi_asharp_GetIQpara(const RkAiqAlgoContext *ctx,
                             rk_aiq_sharp_IQpara_t *para)
{

#ifdef RKAIQ_ENABLE_PARSER_V1
    AsharpContext_t* pAsharpCtx = (AsharpContext_t*)ctx;

    if(ctx != NULL && para != NULL) {
        //sharp
        CalibDb_Sharp_2_t stSharpCalib;
        stSharpCalib.mode_num = sizeof(para->stSharpPara.mode_cell) / sizeof(CalibDb_Sharp_ModeCell_t);
        stSharpCalib.mode_cell = (CalibDb_Sharp_ModeCell_t *)malloc(stSharpCalib.mode_num * sizeof(CalibDb_Sharp_ModeCell_t));
#if(ASHARP_USE_JSON_PARA)
        sharpV1_calibdbV2_to_calibdb(&pAsharpCtx->sharp_v1, &stSharpCalib, 0);
#else
        stSharpCalib.enable = pAsharpCtx->stSharpCalib.enable;
        memcpy(stSharpCalib.version, pAsharpCtx->stSharpCalib.version, sizeof(pAsharpCtx->stSharpCalib.version));
        for(int i = 0; i < 8; i++) {
            stSharpCalib.luma_point[i] = pAsharpCtx->stSharpCalib.luma_point[i];
        }
        for(int i = 0; i < stSharpCalib.mode_num && i < pAsharpCtx->stSharpCalib.mode_num; i++) {
            stSharpCalib.mode_cell[i] = pAsharpCtx->stSharpCalib.mode_cell[i];
        }
#endif

        memset(&para->stSharpPara, 0x00, sizeof(CalibDb_Sharp_t));
        para->stSharpPara.enable = stSharpCalib.enable;
        memcpy(para->stSharpPara.version, stSharpCalib.version, sizeof(stSharpCalib.version));
        for(int i = 0; i < 8; i++) {
            para->stSharpPara.luma_point[i] = stSharpCalib.luma_point[i];
        }
        for(int i = 0; i < stSharpCalib.mode_num && i < CALIBDB_MAX_MODE_NUM; i++) {
            para->stSharpPara.mode_cell[i] = stSharpCalib.mode_cell[i];
        }

        //edgefilter
        CalibDb_EdgeFilter_2_t stEdgeFltCalib;
        stEdgeFltCalib.mode_num = sizeof(para->stEdgeFltPara.mode_cell) / sizeof(CalibDb_EdgeFilter_ModeCell_t);
        stEdgeFltCalib.mode_cell = (CalibDb_EdgeFilter_ModeCell_t *)malloc(stEdgeFltCalib.mode_num * sizeof(CalibDb_EdgeFilter_ModeCell_t));

#if(ASHARP_USE_JSON_PARA)
        edgefilterV1_calibdbV2_to_calibdb(&pAsharpCtx->edgefilter_v1, &stEdgeFltCalib, 0);
#else
        stEdgeFltCalib.enable = pAsharpCtx->stEdgeFltCalib.enable;
        memcpy(stEdgeFltCalib.version, pAsharpCtx->stEdgeFltCalib.version, sizeof(stEdgeFltCalib.version));
        for(int i = 0; i < 8; i++) {
            stEdgeFltCalib.luma_point[i] = pAsharpCtx->stEdgeFltCalib.luma_point[i];
        }
        for(int i = 0; i < pAsharpCtx->stEdgeFltCalib.mode_num && i < stEdgeFltCalib->mode_num; i++) {
            stEdgeFltCalib.mode_cell[i] = pAsharpCtx->stEdgeFltCalib.mode_cell[i];
        }
#endif

        memset(&para->stEdgeFltPara, 0x00, sizeof(CalibDb_EdgeFilter_t));
        para->stEdgeFltPara.enable = stEdgeFltCalib.enable;
        memcpy(para->stEdgeFltPara.version, stEdgeFltCalib.version, sizeof(para->stEdgeFltPara.version));
        for(int i = 0; i < 8; i++) {
            para->stEdgeFltPara.luma_point[i] = stEdgeFltCalib.luma_point[i];
        }
        for(int i = 0; i < stEdgeFltCalib.mode_num && i < CALIBDB_MAX_MODE_NUM; i++) {
            para->stEdgeFltPara.mode_cell[i] = stEdgeFltCalib.mode_cell[i];
        }

        free(stEdgeFltCalib.mode_cell);
        free(stSharpCalib.mode_cell);
    }

    return XCAM_RETURN_NO_ERROR;
#else
    return XCAM_RETURN_ERROR_PARAM;
#endif
}


XCamReturn
rk_aiq_uapi_asharp_SetStrength(const RkAiqAlgoContext *ctx,
                               float fPercent)
{

    AsharpContext_t* pAsharpCtx = (AsharpContext_t*)ctx;
    float fMax = SHARP_MAX_STRENGTH_PERCENT;
    float fStrength = 1.0;


    if(fPercent <= 0.5) {
        fStrength =  fPercent / 0.5;
    } else {
        fStrength = (fPercent - 0.5) * (fMax - 1) * 2 + 1;
    }

    pAsharpCtx->fStrength = fStrength;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi_asharp_GetStrength(const RkAiqAlgoContext *ctx,
                               float *pPercent)
{

    AsharpContext_t* pAsharpCtx = (AsharpContext_t*)ctx;
    float fMax = SHARP_MAX_STRENGTH_PERCENT;
    float fStrength = 1.0;

    fStrength = pAsharpCtx->fStrength;

    if(fStrength <= 1) {
        *pPercent = fStrength * 0.5;
    } else {
        *pPercent = 2 * (fStrength - 1) / (fMax - 1) + 0.5;
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi_asharp_SetJsonpara(RkAiqAlgoContext *ctx,
                               rk_aiq_sharp_JsonPara_t *para,
                               bool need_sync)
{

    AsharpContext_t* pAsharpCtx = (AsharpContext_t*)ctx;

    if(para->module_bits & (1 << ASHARP_MODULE_SHARP)) {
        sharp_calibdbV2_assign(&pAsharpCtx->sharp_v1, &para->sharp_v1);
        pAsharpCtx->isIQParaUpdate = true;
    }

    if(para->module_bits & (1 << ASHARP_MODULE_EDGEFILTER)) {
        edgefilter_calibdbV2_assign(&pAsharpCtx->edgefilter_v1, &para->edgefilter_v1);
        pAsharpCtx->isIQParaUpdate = true;
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi_asharp_GetJsonpara(const RkAiqAlgoContext *ctx,
                               rk_aiq_sharp_JsonPara_t *para)
{

    AsharpContext_t* pAsharpCtx = (AsharpContext_t*)ctx;

    sharp_calibdbV2_assign(&para->sharp_v1, &pAsharpCtx->sharp_v1);
    edgefilter_calibdbV2_assign(&para->edgefilter_v1, &pAsharpCtx->edgefilter_v1);

    return XCAM_RETURN_NO_ERROR;
}

