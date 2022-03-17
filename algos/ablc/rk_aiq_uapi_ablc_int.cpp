#include "rk_aiq_uapi_ablc_int.h"
#include "ablc/rk_aiq_types_ablc_algo_prvt.h"


void BlcParamsNewMalloc
(
    AblcParams_t*           pStoreBlcPara,
    AblcParams_t*           pInputBlcPara
) {
    LOGD_ABLC( "%s:enter!\n", __FUNCTION__);

    // initial checks
    DCT_ASSERT(pStoreBlcPara != NULL);
    DCT_ASSERT(pInputBlcPara != NULL);

    if(pStoreBlcPara->len != pInputBlcPara->len) {
        LOGD_ABLC( "%s:enter store_Len:%d! inputLen:%d\n", __FUNCTION__,
                   pStoreBlcPara->len,
                   pInputBlcPara->len);
        if(pStoreBlcPara->iso)
            free(pStoreBlcPara->iso);

        if(pStoreBlcPara->blc_b)
            free(pStoreBlcPara->blc_b);

        if(pStoreBlcPara->blc_gb)
            free(pStoreBlcPara->blc_gb);

        if(pStoreBlcPara->blc_gr)
            free(pStoreBlcPara->blc_gr);

        if(pStoreBlcPara->blc_r)
            free(pStoreBlcPara->blc_r);

        pStoreBlcPara->len = pInputBlcPara->len;
        pStoreBlcPara->iso = (float*)malloc(sizeof(float) * (pInputBlcPara->len));
        pStoreBlcPara->blc_r = (float*)malloc(sizeof(float) * (pInputBlcPara->len));
        pStoreBlcPara->blc_gr = (float*)malloc(sizeof(float) * (pInputBlcPara->len));
        pStoreBlcPara->blc_gb = (float*)malloc(sizeof(float) * (pInputBlcPara->len));
        pStoreBlcPara->blc_b = (float*)malloc(sizeof(float) * (pInputBlcPara->len));

    }

    pStoreBlcPara->enable = pInputBlcPara->enable;
    for(int i = 0; i < pInputBlcPara->len; i++) {
        pStoreBlcPara->iso[i] = pInputBlcPara->iso[i];
        pStoreBlcPara->blc_r[i] = pInputBlcPara->blc_r[i];
        pStoreBlcPara->blc_gr[i] = pInputBlcPara->blc_gr[i];
        pStoreBlcPara->blc_gb[i] = pInputBlcPara->blc_gb[i];
        pStoreBlcPara->blc_b[i] = pInputBlcPara->blc_b[i];

        LOGD_ABLC("ablc iso:%f blc:%f %f %f %f\n",
                  pStoreBlcPara->iso[i],
                  pStoreBlcPara->blc_r[i],
                  pStoreBlcPara->blc_gr[i],
                  pStoreBlcPara->blc_gb[i],
                  pStoreBlcPara->blc_b[i]);
    }

    LOGD_ABLC( "%s:exit!\n", __FUNCTION__);
}


XCamReturn
rk_aiq_uapi_ablc_SetAttrib(RkAiqAlgoContext *ctx,
                           rk_aiq_blc_attrib_t *attr,
                           bool need_sync)
{
    AblcContext_t* pAblcCtx = (AblcContext_t*)ctx;
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    pAblcCtx->eMode = attr->eMode;
    if(attr->eMode == ABLC_OP_MODE_MANUAL) {
        pAblcCtx->stBlc0Manual = attr->stBlc0Manual;
        pAblcCtx->stBlc1Manual = attr->stBlc1Manual;
    } else if(attr->eMode == ABLC_OP_MODE_AUTO) {
        BlcParamsNewMalloc(&pAblcCtx->stBlc0Params, &attr->stBlc0Auto);
        BlcParamsNewMalloc(&pAblcCtx->stBlc1Params, &attr->stBlc1Auto);
    }


    pAblcCtx->isReCalculate |= 1;
    return ret;
}

XCamReturn
rk_aiq_uapi_ablc_GetAttrib(const RkAiqAlgoContext *ctx,
                           rk_aiq_blc_attrib_t *attr)
{

    AblcContext_t* pAblcCtx = (AblcContext_t*)ctx;
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    attr->eMode = pAblcCtx->eMode;
    memcpy(&attr->stBlc0Manual, &pAblcCtx->stBlc0Manual, sizeof(attr->stBlc0Manual));
    memcpy(&attr->stBlc1Manual, &pAblcCtx->stBlc1Manual, sizeof(attr->stBlc1Manual));

    BlcParamsNewMalloc(&attr->stBlc0Auto, &pAblcCtx->stBlc0Params);
    BlcParamsNewMalloc(&attr->stBlc1Auto, &pAblcCtx->stBlc1Params);
    return ret;
}


XCamReturn
rk_aiq_uapi_ablc_GetProc(const RkAiqAlgoContext *ctx,
                         AblcProc_t *ProcRes)
{
    AblcContext_t* pAblcCtx = (AblcContext_t*)ctx;
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    *ProcRes = pAblcCtx->ProcRes;
    return ret;
}

