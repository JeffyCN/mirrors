#include "rk_aiq_uapi_adrc_int.h"
#include "adrc/rk_aiq_types_adrc_algo_prvt.h"

XCamReturn
rk_aiq_uapi_adrc_SetAttrib(RkAiqAlgoContext *ctx,
                           drc_attrib_t attr,
                           bool need_sync)
{
    LOG1_ATMO("ENTER: %s \n", __func__);
    AdrcContext_t* pAdrcCtx = (AdrcContext_t*)(ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    pAdrcCtx->drcAttr.opMode = attr.opMode;
    if(attr.opMode == DRC_OPMODE_MANU) {
        if(CHECK_ISP_HW_V21())
            memcpy(&pAdrcCtx->drcAttr.stManualV21, &attr.stManualV21, sizeof(mdrcAttr_V21_t));
        else if(CHECK_ISP_HW_V30())
            memcpy(&pAdrcCtx->drcAttr.stManualV30, &attr.stManualV30, sizeof(mdrcAttr_V30_t));
    }
    if(attr.opMode == DRC_OPMODE_DRC_GAIN)
        memcpy(&pAdrcCtx->drcAttr.stDrcGain, &attr.stDrcGain, sizeof(mDrcGain_t));
    if(attr.opMode == DRC_OPMODE_HILIT)
        memcpy(&pAdrcCtx->drcAttr.stHiLit, &attr.stHiLit, sizeof(mDrcHiLit_t));
    if(attr.opMode == DRC_OPMODE_LOCAL_TMO) {
        if(CHECK_ISP_HW_V21())
            memcpy(&pAdrcCtx->drcAttr.stLocalDataV21, &attr.stLocalDataV21, sizeof(mLocalDataV21_t));
        else if(CHECK_ISP_HW_V30())
            memcpy(&pAdrcCtx->drcAttr.stLocalDataV30, &attr.stLocalDataV30, sizeof(mLocalDataV30_t));
    }

    LOG1_ATMO("EXIT: %s \n", __func__);
    return ret;
}

XCamReturn
rk_aiq_uapi_adrc_GetAttrib(RkAiqAlgoContext *ctx,
                           drc_attrib_t *attr)
{
    LOG1_ATMO("ENTER: %s \n", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    AdrcContext_t* pAdrcCtx = (AdrcContext_t*)ctx;

    attr->opMode = pAdrcCtx->drcAttr.opMode;
    memcpy(&attr->Info, &pAdrcCtx->drcAttr.Info, sizeof(DrcInfo_t));
    memcpy(&attr->stDrcGain, &pAdrcCtx->drcAttr.stDrcGain, sizeof(mDrcGain_t));
    memcpy(&attr->stHiLit, &pAdrcCtx->drcAttr.stHiLit, sizeof(mDrcHiLit_t));
    if(CHECK_ISP_HW_V21()) {
        memcpy(&attr->stManualV21, &pAdrcCtx->drcAttr.stManualV21, sizeof(mdrcAttr_V21_t));
        memcpy(&attr->stLocalDataV21, &pAdrcCtx->drcAttr.stLocalDataV21, sizeof(mLocalDataV21_t));
    }
    else if(CHECK_ISP_HW_V30()) {
        memcpy(&attr->stManualV30, &pAdrcCtx->drcAttr.stManualV30, sizeof(mdrcAttr_V30_t));
        memcpy(&attr->stLocalDataV30, &pAdrcCtx->drcAttr.stLocalDataV30, sizeof(mLocalDataV30_t));
    }

    LOG1_ATMO("EXIT: %s \n", __func__);
    return XCAM_RETURN_NO_ERROR;
}
