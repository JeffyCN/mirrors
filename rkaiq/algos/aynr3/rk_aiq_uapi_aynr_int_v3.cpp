#include "aynr3/rk_aiq_uapi_aynr_int_v3.h"
#include "aynr3/rk_aiq_types_aynr_algo_prvt_v3.h"

#if 1
#define AYNRV3_CHROMA_SF_STRENGTH_SLOPE_FACTOR (5.0)


int sigma_curve_calc(double* pSigma_curve, int16_t* pLumaPoint, float* pSigma)
{
    int i = 0;
    short isoCurveSectValue;
    short isoCurveSectValue1;
    float ave1, ave2, ave3, ave4;
    int bit_calib = 12;
    int bit_proc;
    int bit_shift;

    if(pSigma_curve == NULL || pLumaPoint == NULL || pSigma == NULL) {
        LOGE_ANR("%s:%d null point\n", __FUNCTION__, __LINE__);
    }

    bit_proc = YNR_V3_SIGMA_BITS; // for V3, YNR_SIGMA_BITS = 10
    bit_shift = bit_calib - bit_proc;

    isoCurveSectValue = (1 << (bit_calib - YNR_V3_ISO_CURVE_POINT_BIT));
    isoCurveSectValue1 = (1 << bit_calib);

    for (i = 0; i < YNR_V3_ISO_CURVE_POINT_NUM; i++) {
        if (i == (YNR_V3_ISO_CURVE_POINT_NUM - 1)) {
            ave1 = (float)isoCurveSectValue1;
        } else {
            ave1 = (float)(i * isoCurveSectValue);
        }
        pLumaPoint[i] = (short)ave1;
        ave2 = ave1 * ave1;
        ave3 = ave2 * ave1;
        ave4 = ave3 * ave1;
        pSigma[i] = pSigma_curve[0] * ave4
                    + pSigma_curve[1] * ave3
                    + pSigma_curve[2] * ave2
                    + pSigma_curve[3] * ave1
                    + pSigma_curve[4];

        if (pSigma[i] < 0) {
            pSigma[i] = 0;
        }

        if (bit_shift > 0) {
            pLumaPoint[i] >>= bit_shift;
        } else {
            pLumaPoint[i] <<= ABS(bit_shift);
        }
    }

    return 0;
}

XCamReturn
rk_aiq_uapi_aynrV3_SetAttrib(RkAiqAlgoContext *ctx,
                             rk_aiq_ynr_attrib_v3_t *attr,
                             bool need_sync)
{

    Aynr_Context_V3_t* pCtx = (Aynr_Context_V3_t*)ctx;

    pCtx->eMode = attr->eMode;
    if(pCtx->eMode == AYNRV3_OP_MODE_AUTO) {
        pCtx->stAuto = attr->stAuto;
        RK_YNR_Params_V3_t *pParams = &attr->stAuto.stParams;
        if(!pParams->sigma_use_point) {
            for(int j = 0; j < RK_YNR_V3_MAX_ISO_NUM ; j++) {
                double *pSigma_curve = pParams->arSigmaFormulaISO[j].sigma_curve;
                sigma_curve_calc(pSigma_curve, pParams->arYnrParamsISO[j].lumaPoint, pParams->arYnrParamsISO[j].sigma);
            }
        }
    } else if(pCtx->eMode == AYNRV3_OP_MODE_MANUAL) {
        pCtx->stManual.stSelect = attr->stManual.stSelect;
    } else if(pCtx->eMode == AYNRV3_OP_MODE_REG_MANUAL) {
        pCtx->stManual.stFix = attr->stManual.stFix;
    }
    pCtx->isReCalculate |= 1;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi_aynrV3_GetAttrib(const RkAiqAlgoContext *ctx,
                             rk_aiq_ynr_attrib_v3_t *attr)
{

    Aynr_Context_V3_t* pCtx = (Aynr_Context_V3_t*)ctx;

    attr->eMode = pCtx->eMode;
    memcpy(&attr->stAuto, &pCtx->stAuto, sizeof(attr->stAuto));
    memcpy(&attr->stManual, &pCtx->stManual, sizeof(attr->stManual));

    return XCAM_RETURN_NO_ERROR;
}


XCamReturn
rk_aiq_uapi_aynrV3_SetLumaSFStrength(const RkAiqAlgoContext *ctx,
                                     rk_aiq_ynr_strength_v3_t *pStrength)
{
    Aynr_Context_V3_t* pCtx = (Aynr_Context_V3_t*)ctx;

    float fStrength = 1.0f;
    float fPercent = 0.5;
    float fslope = AYNRV3_CHROMA_SF_STRENGTH_SLOPE_FACTOR;

    fPercent = pStrength->percent;


    if(fPercent <= 0.5) {
        fStrength =  fPercent / 0.5;
    } else {
        if(fPercent >= 0.999999)
            fPercent = 0.999999;
        fStrength = 0.5 * fslope / (1.0 - fPercent) - fslope + 1;
    }

    pCtx->stStrength = *pStrength;
    pCtx->stStrength.percent = fStrength;
    pCtx->isReCalculate |= 1;

    return XCAM_RETURN_NO_ERROR;
}



XCamReturn
rk_aiq_uapi_aynrV3_GetLumaSFStrength(const RkAiqAlgoContext *ctx,
                                     rk_aiq_ynr_strength_v3_t *pStrength)
{
    Aynr_Context_V3_t* pCtx = (Aynr_Context_V3_t*)ctx;

    float fStrength = 1.0f;
    float fPercent = 0.5;
    float fslope = AYNRV3_CHROMA_SF_STRENGTH_SLOPE_FACTOR;

    fStrength = pCtx->stStrength.percent;

    if(fStrength <= 1) {
        fPercent = fStrength * 0.5;
    } else {
        float tmp = 1.0;
        tmp = 1 - 0.5 * fslope / (fStrength + fslope - 1);
        if(abs(tmp - 0.999999) < 0.000001) {
            tmp = 1.0;
        }
        fPercent = tmp;
    }

    *pStrength = pCtx->stStrength;
    pStrength->percent = fPercent;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi_aynrV3_GetInfo(const RkAiqAlgoContext *ctx,
                           rk_aiq_ynr_info_v3_t *pInfo)
{
    Aynr_Context_V3_t* pCtx = (Aynr_Context_V3_t*)ctx;


    pInfo->iso = pCtx->stExpInfo.arIso[pCtx->stExpInfo.hdr_mode];
    pInfo->expo_info = pCtx->stExpInfo;

    return XCAM_RETURN_NO_ERROR;
}


#endif

