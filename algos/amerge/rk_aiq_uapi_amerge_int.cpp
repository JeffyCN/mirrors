#include "rk_aiq_uapi_amerge_int.h"
#include "rk_aiq_types_amerge_algo_prvt.h"

bool
IfMergeDataEqu
(
    int* pInput,
    int len
)
{
    bool equ = true;

    for(int i = 0; i < len - 1; i++)
        if(pInput[i] != pInput[i + 1])
            equ = false;

    return equ;
}

XCamReturn
rk_aiq_uapi_amerge_SetTool
(
    CalibDbV2_merge_t* pStore,
    CalibDbV2_merge_t* pInput
)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    //oe curve
    int OECurveLen[3];
    OECurveLen[0] = pInput->MergeTuningPara.OECurve.EnvLv_len;
    OECurveLen[1] = pInput->MergeTuningPara.OECurve.Smooth_len;
    OECurveLen[2] = pInput->MergeTuningPara.OECurve.Offset_len;
    bool OECurveLenEqu = IfMergeDataEqu(OECurveLen, 3);
    if(!OECurveLenEqu) {
        LOGE_AMERGE("%s: Input Merge OECurve Data length is NOT EQUAL !!!\n", __FUNCTION__ );
        return XCAM_RETURN_ERROR_FAILED;
    }
    if(pStore->MergeTuningPara.OECurve.EnvLv_len != pInput->MergeTuningPara.OECurve.EnvLv_len) {
        //free
        free(pStore->MergeTuningPara.OECurve.EnvLv);
        free(pStore->MergeTuningPara.OECurve.Smooth);
        free(pStore->MergeTuningPara.OECurve.Offset);
        //malloc
        pStore->MergeTuningPara.OECurve.EnvLv =
            (float *) malloc(sizeof(float) * pInput->MergeTuningPara.OECurve.EnvLv_len);
        pStore->MergeTuningPara.OECurve.Smooth =
            (float *) malloc(sizeof(float) * pInput->MergeTuningPara.OECurve.EnvLv_len);
        pStore->MergeTuningPara.OECurve.Offset =
            (float *) malloc(sizeof(float) * pInput->MergeTuningPara.OECurve.EnvLv_len);
    }
    //store len
    pStore->MergeTuningPara.OECurve.EnvLv_len = pInput->MergeTuningPara.OECurve.EnvLv_len;
    pStore->MergeTuningPara.OECurve.Smooth_len = pInput->MergeTuningPara.OECurve.EnvLv_len;
    pStore->MergeTuningPara.OECurve.Offset_len = pInput->MergeTuningPara.OECurve.EnvLv_len;
    memcpy(pStore->MergeTuningPara.OECurve.EnvLv,
           pInput->MergeTuningPara.OECurve.EnvLv, sizeof(float)*pInput->MergeTuningPara.OECurve.EnvLv_len);
    memcpy(pStore->MergeTuningPara.OECurve.Smooth,
           pInput->MergeTuningPara.OECurve.Smooth, sizeof(float)*pInput->MergeTuningPara.OECurve.EnvLv_len);
    memcpy(pStore->MergeTuningPara.OECurve.Offset,
           pInput->MergeTuningPara.OECurve.Offset, sizeof(float)*pInput->MergeTuningPara.OECurve.EnvLv_len);

    //md curve
    int MDCurveLen[5];
    MDCurveLen[0] = pInput->MergeTuningPara.MDCurve.MoveCoef_len;
    MDCurveLen[1] = pInput->MergeTuningPara.MDCurve.LM_smooth_len;
    MDCurveLen[2] = pInput->MergeTuningPara.MDCurve.LM_offset_len;
    MDCurveLen[3] = pInput->MergeTuningPara.MDCurve.MS_smooth_len;
    MDCurveLen[4] = pInput->MergeTuningPara.MDCurve.MS_offset_len;
    bool MDCurveLenEqu = IfMergeDataEqu(MDCurveLen, 5);
    if(!MDCurveLenEqu) {
        LOGE_AMERGE("%s: Input Merge MDCurve Data length is NOT EQUAL !!!\n", __FUNCTION__ );
        return XCAM_RETURN_ERROR_FAILED;
    }
    if(pStore->MergeTuningPara.MDCurve.MoveCoef_len != pInput->MergeTuningPara.MDCurve.MoveCoef_len) {
        //free
        free(pStore->MergeTuningPara.MDCurve.MoveCoef);
        free(pStore->MergeTuningPara.MDCurve.MS_smooth);
        free(pStore->MergeTuningPara.MDCurve.MS_offset);
        free(pStore->MergeTuningPara.MDCurve.LM_smooth);
        free(pStore->MergeTuningPara.MDCurve.LM_offset);
        //malloc
        pStore->MergeTuningPara.MDCurve.MoveCoef =
            (float *) malloc(sizeof(float) * pInput->MergeTuningPara.MDCurve.MoveCoef_len);
        pStore->MergeTuningPara.MDCurve.MS_smooth =
            (float *) malloc(sizeof(float) * pInput->MergeTuningPara.MDCurve.MoveCoef_len);
        pStore->MergeTuningPara.MDCurve.MS_offset =
            (float *) malloc(sizeof(float) * pInput->MergeTuningPara.MDCurve.MoveCoef_len);
        pStore->MergeTuningPara.MDCurve.LM_smooth =
            (float *) malloc(sizeof(float) * pInput->MergeTuningPara.MDCurve.MoveCoef_len);
        pStore->MergeTuningPara.MDCurve.LM_offset =
            (float *) malloc(sizeof(float) * pInput->MergeTuningPara.MDCurve.MoveCoef_len);
    }
    //store len
    pStore->MergeTuningPara.MDCurve.MoveCoef_len = pInput->MergeTuningPara.MDCurve.MoveCoef_len;
    pStore->MergeTuningPara.MDCurve.MS_smooth_len = pInput->MergeTuningPara.MDCurve.MoveCoef_len;
    pStore->MergeTuningPara.MDCurve.MS_offset_len = pInput->MergeTuningPara.MDCurve.MoveCoef_len;
    pStore->MergeTuningPara.MDCurve.LM_smooth_len = pInput->MergeTuningPara.MDCurve.MoveCoef_len;
    pStore->MergeTuningPara.MDCurve.LM_offset_len = pInput->MergeTuningPara.MDCurve.MoveCoef_len;
    memcpy(pStore->MergeTuningPara.MDCurve.MoveCoef,
           pInput->MergeTuningPara.MDCurve.MoveCoef, sizeof(float)*pInput->MergeTuningPara.MDCurve.MoveCoef_len);
    memcpy(pStore->MergeTuningPara.MDCurve.MS_smooth,
           pInput->MergeTuningPara.MDCurve.MS_smooth, sizeof(float)*pInput->MergeTuningPara.MDCurve.MoveCoef_len);
    memcpy(pStore->MergeTuningPara.MDCurve.MS_offset,
           pInput->MergeTuningPara.MDCurve.MS_offset, sizeof(float)*pInput->MergeTuningPara.MDCurve.MoveCoef_len);
    memcpy(pStore->MergeTuningPara.MDCurve.LM_smooth,
           pInput->MergeTuningPara.MDCurve.LM_smooth, sizeof(float)*pInput->MergeTuningPara.MDCurve.MoveCoef_len);
    memcpy(pStore->MergeTuningPara.MDCurve.LM_offset,
           pInput->MergeTuningPara.MDCurve.LM_offset, sizeof(float)*pInput->MergeTuningPara.MDCurve.MoveCoef_len);

    pStore->MergeTuningPara.OECurve_damp = pInput->MergeTuningPara.OECurve_damp;
    pStore->MergeTuningPara.MDCurveLM_damp = pInput->MergeTuningPara.MDCurveLM_damp;
    pStore->MergeTuningPara.MDCurveMS_damp = pInput->MergeTuningPara.MDCurveMS_damp;

    return ret;
}

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
    AmergeContext_t* pAmergeCtx = (AmergeContext_t*)(ctx->AmergeInstConfig.hAmerge);

    //Todo
    pAmergeCtx->mergeAttr.opMode = attr.opMode;
    if(attr.opMode == MERGE_OPMODE_TOOL)
        ret = rk_aiq_uapi_amerge_SetTool(&pAmergeCtx->mergeAttr.stTool, &attr.stTool);

    if (attr.opMode == MERGE_OPMODE_AUTO) {
        pAmergeCtx->mergeAttr.stAuto.bUpdateMge = attr.stAuto.bUpdateMge;
        memcpy(&pAmergeCtx->mergeAttr.stAuto.stMgeAuto, &attr.stAuto.stMgeAuto, sizeof(amgeAttr_t));
    }
    else
        pAmergeCtx->mergeAttr.stAuto.bUpdateMge = false;

    if (attr.opMode == MERGE_OPMODE_MANU) {
        pAmergeCtx->mergeAttr.stManual.bUpdateMge = attr.stManual.bUpdateMge;
        memcpy(&pAmergeCtx->mergeAttr.stManual.stMgeManual, &attr.stManual.stMgeManual, sizeof(mmgeAttr_t));
    }
    else
        pAmergeCtx->mergeAttr.stManual.bUpdateMge = false;

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

    AmergeContext_t* pAmergeCtx = (AmergeContext_t*)ctx->AmergeInstConfig.hAmerge;

    attr->opMode = pAmergeCtx->mergeAttr.opMode;

    memcpy(&attr->stAuto.stMgeAuto, &pAmergeCtx->mergeAttr.stAuto.stMgeAuto, sizeof(amgeAttr_t));
    memcpy(&attr->stManual.stMgeManual, &pAmergeCtx->mergeAttr.stManual.stMgeManual, sizeof(mmgeAttr_t));
    ret = rk_aiq_uapi_amerge_SetTool(&attr->stTool, &pAmergeCtx->mergeAttr.stTool);
    memcpy(&attr->CtlInfo, &pAmergeCtx->mergeAttr.CtlInfo, sizeof(MergeCurrCtlData_t));
    memcpy(&attr->RegInfo, &pAmergeCtx->mergeAttr.RegInfo, sizeof(MergeCurrRegData_t));

    return ret;
}


