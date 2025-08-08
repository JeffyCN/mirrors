#include "rk_aiq_uapi_ablc_int.h"
#include "ablc/rk_aiq_types_ablc_algo_prvt.h"


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
        memcpy(&pAblcCtx->stBlc0Params, &attr->stBlc0Auto, sizeof(pAblcCtx->stBlc0Params));
        memcpy(&pAblcCtx->stBlc1Params, &attr->stBlc1Auto, sizeof(pAblcCtx->stBlc1Params));
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

    memcpy(&attr->stBlc0Auto, &pAblcCtx->stBlc0Params, sizeof(attr->stBlc0Auto));
    memcpy(&attr->stBlc1Auto, &pAblcCtx->stBlc1Params, sizeof(attr->stBlc1Auto));
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

XCamReturn
rk_aiq_uapi_ablc_GetInfo(const RkAiqAlgoContext *ctx,
                         rk_aiq_ablc_info_t *pInfo)
{
    AblcContext_t* pAblcCtx = (AblcContext_t*)ctx;
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    pInfo->iso = pAblcCtx->stExpInfo.arIso[pAblcCtx->stExpInfo.hdr_mode];
    pInfo->expo_info = pAblcCtx->stExpInfo;
    return ret;
}


