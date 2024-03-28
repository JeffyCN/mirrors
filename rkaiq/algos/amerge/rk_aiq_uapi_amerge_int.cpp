#include "rk_aiq_uapi_amerge_int.h"
#include "rk_aiq_types_amerge_algo_prvt.h"

#if RKAIQ_HAVE_MERGE_V10
XCamReturn rk_aiq_uapi_amerge_v10_SetAttrib(RkAiqAlgoContext* ctx, const mergeAttrV10_t* attr,
                                            bool need_sync) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if(ctx == NULL) {
        LOGE_AMERGE("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }
    AmergeContext_t* pAmergeCtx = (AmergeContext_t*)ctx;

    pAmergeCtx->mergeAttrV10.opMode = attr->opMode;
    if (attr->opMode == MERGE_OPMODE_AUTO) {
        memcpy(&pAmergeCtx->mergeAttrV10.stAuto, &attr->stAuto, sizeof(CalibDbV2_merge_v10_t));
        pAmergeCtx->ifReCalcStAuto = true;
    } else if (attr->opMode == MERGE_OPMODE_MANUAL) {
        memcpy(&pAmergeCtx->mergeAttrV10.stManual, &attr->stManual, sizeof(mMergeAttrV10_t));
        pAmergeCtx->ifReCalcStManual = true;
    }

    return ret;
}

XCamReturn rk_aiq_uapi_amerge_v10_GetAttrib(RkAiqAlgoContext* ctx, mergeAttrV10_t* attr) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (ctx == NULL || attr == NULL) {
        LOGE_AMERGE("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    AmergeContext_t* pAmergeCtx = (AmergeContext_t*)ctx;

    attr->opMode = pAmergeCtx->mergeAttrV10.opMode;
    memcpy(&attr->stManual, &pAmergeCtx->mergeAttrV10.stManual, sizeof(mMergeAttrV10_t));
    memcpy(&attr->stAuto, &pAmergeCtx->mergeAttrV10.stAuto, sizeof(CalibDbV2_merge_v10_t));
    // get info
    attr->Info.Envlv    = pAmergeCtx->NextData.CtrlData.ExpoData.EnvLv;
    attr->Info.ISO      = pAmergeCtx->NextData.CtrlData.ExpoData.ISO;
    attr->Info.MoveCoef = pAmergeCtx->NextData.CtrlData.MoveCoef;

    return ret;
}
#endif

#if RKAIQ_HAVE_MERGE_V11
XCamReturn rk_aiq_uapi_amerge_v11_SetAttrib(RkAiqAlgoContext* ctx, const mergeAttrV11_t* attr,
                                            bool need_sync) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (ctx == NULL) {
        LOGE_AMERGE("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }
    AmergeContext_t* pAmergeCtx = (AmergeContext_t*)ctx;

    pAmergeCtx->mergeAttrV11.opMode = attr->opMode;
    if (attr->opMode == MERGE_OPMODE_AUTO) {
        memcpy(&pAmergeCtx->mergeAttrV11.stAuto, &attr->stAuto, sizeof(CalibDbV2_merge_v11_t));
        pAmergeCtx->ifReCalcStAuto = true;
    } else if (attr->opMode == MERGE_OPMODE_MANUAL) {
        memcpy(&pAmergeCtx->mergeAttrV11.stManual, &attr->stManual, sizeof(mMergeAttrV11_t));
        pAmergeCtx->ifReCalcStManual = true;
    }

    return ret;
}

XCamReturn rk_aiq_uapi_amerge_v11_GetAttrib(RkAiqAlgoContext* ctx, mergeAttrV11_t* attr) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if(ctx == NULL || attr == NULL) {
        LOGE_AMERGE("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    AmergeContext_t* pAmergeCtx = (AmergeContext_t*)ctx;

    attr->opMode = pAmergeCtx->mergeAttrV11.opMode;
    memcpy(&attr->stAuto, &pAmergeCtx->mergeAttrV11.stAuto, sizeof(CalibDbV2_merge_v11_t));
    memcpy(&attr->stManual, &pAmergeCtx->mergeAttrV11.stManual, sizeof(mMergeAttrV11_t));
    // get info
    attr->Info.Envlv    = pAmergeCtx->NextData.CtrlData.ExpoData.EnvLv;
    attr->Info.ISO      = pAmergeCtx->NextData.CtrlData.ExpoData.ISO;
    attr->Info.MoveCoef = pAmergeCtx->NextData.CtrlData.MoveCoef;

    return ret;
}
#endif

#if RKAIQ_HAVE_MERGE_V12
XCamReturn rk_aiq_uapi_amerge_v12_SetAttrib(RkAiqAlgoContext* ctx, const mergeAttrV12_t* attr,
                                            bool need_sync) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (ctx == NULL) {
        LOGE_AMERGE("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }
    AmergeContext_t* pAmergeCtx = (AmergeContext_t*)ctx;

    pAmergeCtx->mergeAttrV12.opMode = attr->opMode;
    if (attr->opMode == MERGE_OPMODE_AUTO) {
        memcpy(&pAmergeCtx->mergeAttrV12.stAuto, &attr->stAuto, sizeof(CalibDbV2_merge_v12_t));
        pAmergeCtx->ifReCalcStAuto = true;
    } else if (attr->opMode == MERGE_OPMODE_MANUAL) {
        memcpy(&pAmergeCtx->mergeAttrV12.stManual, &attr->stManual, sizeof(mMergeAttrV12_t));
        pAmergeCtx->ifReCalcStManual = true;
    }

    return ret;
}

XCamReturn rk_aiq_uapi_amerge_v12_GetAttrib(RkAiqAlgoContext* ctx, mergeAttrV12_t* attr) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (ctx == NULL || attr == NULL) {
        LOGE_AMERGE("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    AmergeContext_t* pAmergeCtx = (AmergeContext_t*)ctx;

    attr->opMode = pAmergeCtx->mergeAttrV12.opMode;
    memcpy(&attr->stAuto, &pAmergeCtx->mergeAttrV12.stAuto, sizeof(CalibDbV2_merge_v12_t));
    memcpy(&attr->stManual, &pAmergeCtx->mergeAttrV12.stManual, sizeof(mMergeAttrV12_t));
    // get info
    attr->Info.Envlv    = pAmergeCtx->NextData.CtrlData.ExpoData.EnvLv;
    attr->Info.ISO      = pAmergeCtx->NextData.CtrlData.ExpoData.ISO;
    attr->Info.MoveCoef = pAmergeCtx->NextData.CtrlData.MoveCoef;

    return ret;
}
#endif
