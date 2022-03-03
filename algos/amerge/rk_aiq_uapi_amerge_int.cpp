#include "rk_aiq_uapi_amerge_int.h"
#include "rk_aiq_types_amerge_algo_prvt.h"

XCamReturn
rk_aiq_uapi_amerge_SetAttrib
(
    RkAiqAlgoContext* ctx,
    amerge_attrib_t attr,
    bool need_sync
)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if(ctx == NULL) {
        LOGE_AMERGE("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }
    AmergeContext_t* pAmergeCtx = (AmergeContext_t*)ctx;

    if(CHECK_ISP_HW_V21()) {
        pAmergeCtx->mergeAttr.attrV21.opMode = attr.attrV21.opMode;
        if (attr.attrV21.opMode == MERGE_OPMODE_MANU)
            memcpy(&pAmergeCtx->mergeAttr.attrV21.stManual, &attr.attrV21.stManual, sizeof(mmergeAttrV21_t));
    }
    else if(CHECK_ISP_HW_V30()) {
        pAmergeCtx->mergeAttr.attrV30.opMode = attr.attrV30.opMode;
        if(attr.attrV30.opMode == MERGE_OPMODE_MANU)
            memcpy(&pAmergeCtx->mergeAttr.attrV30.stManual, &attr.attrV30.stManual, sizeof(mMergeAttrV30_t));
    }

    return ret;
}

XCamReturn
rk_aiq_uapi_amerge_GetAttrib
(
    RkAiqAlgoContext*  ctx,
    amerge_attrib_t* attr
)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if(ctx == NULL || attr == NULL) {
        LOGE_AMERGE("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    AmergeContext_t* pAmergeCtx = (AmergeContext_t*)ctx;

    if(CHECK_ISP_HW_V20() || CHECK_ISP_HW_V21()) {
        attr->attrV21.opMode = pAmergeCtx->mergeAttr.attrV21.opMode;
        memcpy(&attr->attrV21.stManual, &pAmergeCtx->mergeAttr.attrV21.stManual, sizeof(mmergeAttrV21_t));
        memcpy(&attr->attrV21.CtlInfo, &pAmergeCtx->mergeAttr.attrV21.CtlInfo, sizeof(MergeCurrCtlData_t));
    }
    else if(CHECK_ISP_HW_V30()) {
        attr->attrV30.opMode = pAmergeCtx->mergeAttr.attrV30.opMode;
        memcpy(&attr->attrV30.stManual, &pAmergeCtx->mergeAttr.attrV30.stManual, sizeof(mMergeAttrV30_t));
        memcpy(&attr->attrV30.CtlInfo, &pAmergeCtx->mergeAttr.attrV30.CtlInfo, sizeof(MergeCurrCtlData_t));
    }

    return ret;
}
