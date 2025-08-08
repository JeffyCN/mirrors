#include "rk_aiq_uapi_accm_int.h"
#include "accm/rk_aiq_types_accm_algo_prvt.h"
#include "accm/rk_aiq_accm_algo.h"

#if RKAIQ_HAVE_CCM_V1
XCamReturn
rk_aiq_uapi_accm_SetAttrib(RkAiqAlgoContext *ctx,
                           const rk_aiq_ccm_attrib_t* attr,
                           bool need_sync)
{

    accm_context_t* ccm_contex = (accm_context_t*)ctx->accm_para;
    ccm_contex->invarMode = ccm_contex->mCurAtt.mode & attr->mode;
    ccm_contex->mCurAtt = *attr;
    ccm_contex->updateAtt = true;
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi_accm_GetAttrib(const RkAiqAlgoContext *ctx,
                           rk_aiq_ccm_attrib_t *attr)
{

    accm_context_t* ccm_contex = (accm_context_t*)ctx->accm_para;

    memcpy(attr, &ccm_contex->mCurAtt, sizeof(rk_aiq_ccm_attrib_t));

    return XCAM_RETURN_NO_ERROR;
}
#else
XCamReturn
rk_aiq_uapi_accm_SetAttrib(RkAiqAlgoContext *ctx,
                           const rk_aiq_ccm_attrib_t* attr,
                           bool need_sync)
{
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi_accm_GetAttrib(const RkAiqAlgoContext *ctx,
                           rk_aiq_ccm_attrib_t *attr)
{
    return XCAM_RETURN_NO_ERROR;
}
#endif

#if RKAIQ_HAVE_CCM_V2
XCamReturn
rk_aiq_uapi_accm_v2_SetAttrib(RkAiqAlgoContext *ctx,
                           const rk_aiq_ccm_v2_attrib_t* attr,
                           bool need_sync)
{

    accm_context_t* ccm_contex = (accm_context_t*)ctx->accm_para;
    ccm_contex->invarMode = ccm_contex->mCurAttV2.mode & attr->mode;
    ccm_contex->mCurAttV2 = *attr;
    ccm_contex->updateAtt = true;
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi_accm_v2_GetAttrib(const RkAiqAlgoContext *ctx,
                           rk_aiq_ccm_v2_attrib_t *attr)
{

    accm_context_t* ccm_contex = (accm_context_t*)ctx->accm_para;

    memcpy(attr, &ccm_contex->mCurAttV2, sizeof(rk_aiq_ccm_v2_attrib_t));

    return XCAM_RETURN_NO_ERROR;
}


#else
XCamReturn
rk_aiq_uapi_accm_v2_SetAttrib(RkAiqAlgoContext *ctx,
                           const rk_aiq_ccm_v2_attrib_t* attr,
                           bool need_sync)
{
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi_accm_v2_GetAttrib(const RkAiqAlgoContext *ctx,
                           rk_aiq_ccm_v2_attrib_t *attr)
{
    return XCAM_RETURN_NO_ERROR;
}
#endif
#if RKAIQ_HAVE_CCM_V3
XCamReturn
rk_aiq_uapi_accm_v3_SetAttrib(RkAiqAlgoContext *ctx,
                           const rk_aiq_ccm_v3_attrib_t* attr,
                           bool need_sync)
{

    accm_context_t* ccm_contex = (accm_context_t*)ctx->accm_para;
    ccm_contex->invarMode = ccm_contex->mCurAttV3.mode & attr->mode;
    ccm_contex->mCurAttV3 = *attr;
    ccm_contex->updateAtt = true;
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi_accm_v3_GetAttrib(const RkAiqAlgoContext *ctx,
                           rk_aiq_ccm_v3_attrib_t *attr)
{

    accm_context_t* ccm_contex = (accm_context_t*)ctx->accm_para;

    memcpy(attr, &ccm_contex->mCurAttV3, sizeof(rk_aiq_ccm_v3_attrib_t));

    return XCAM_RETURN_NO_ERROR;
}
#else
XCamReturn
rk_aiq_uapi_accm_v3_SetAttrib(RkAiqAlgoContext *ctx,
                           const rk_aiq_ccm_v3_attrib_t* attr,
                           bool need_sync)
{
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi_accm_v3_GetAttrib(const RkAiqAlgoContext *ctx,
                           rk_aiq_ccm_v3_attrib_t *attr)
{
    return XCAM_RETURN_NO_ERROR;
}
#endif

XCamReturn
rk_aiq_uapi_accm_QueryCcmInfo(const RkAiqAlgoContext *ctx,
                              rk_aiq_ccm_querry_info_t *ccm_querry_info )
{

    accm_context_t* ccm_contex = (accm_context_t*)ctx->accm_para;
    ccm_querry_info->finalSat = 0;
    memset(ccm_querry_info->ccmname1, 0x0, sizeof(ccm_querry_info->ccmname1));
    memset(ccm_querry_info->ccmname2, 0x0, sizeof(ccm_querry_info->ccmname2));

#if RKAIQ_HAVE_CCM_V1
    const rk_aiq_ccm_attrib_t *mCurAtt = &ccm_contex->mCurAtt;
    const rk_aiq_ccm_cfg_t *HwCfg = &ccm_contex->ccmHwConf;
#elif RKAIQ_HAVE_CCM_V2
    const rk_aiq_ccm_v2_attrib_t *mCurAtt = &ccm_contex->mCurAttV2;
    const rk_aiq_ccm_cfg_v2_t *HwCfg = &ccm_contex->ccmHwConf_v2;
#elif RKAIQ_HAVE_CCM_V3
    const rk_aiq_ccm_v3_attrib_t *mCurAtt = &ccm_contex->mCurAttV3;
    const rk_aiq_ccm_cfg_v2_t *HwCfg = &ccm_contex->ccmHwConf_v2;
#endif

    if (HwCfg->ccmEnable && mCurAtt->mode == RK_AIQ_CCM_MODE_AUTO){
        ccm_querry_info->finalSat = ccm_contex->accmRest.fSaturation;
        if (strlen(ccm_contex->accmRest.CcmProf1Name) > 0) {
            strcpy(ccm_querry_info->ccmname1, ccm_contex->accmRest.CcmProf1Name);
        }
        if (strlen(ccm_contex->accmRest.CcmProf2Name) > 0) {
            strcpy(ccm_querry_info->ccmname2, ccm_contex->accmRest.CcmProf2Name);
        }
        else
            strcpy(ccm_querry_info->ccmname2, ccm_querry_info->ccmname1);
    }

    memcpy(ccm_querry_info->YAlp.y_alpha_curve, HwCfg->alp_y, sizeof(HwCfg->alp_y));
    memcpy(ccm_querry_info->Matrix.ccMatrix, HwCfg->matrix, sizeof(HwCfg->matrix));
    memcpy(ccm_querry_info->Matrix.ccOffsets, HwCfg->offs, sizeof(HwCfg->offs));
    ccm_querry_info->ccm_en = HwCfg->ccmEnable;
    ccm_querry_info->YAlp.bound_pos_bit = HwCfg->bound_bit;

#if RKAIQ_HAVE_CCM_V1
    ccm_querry_info->YAlp.right_pos_bit = HwCfg->bound_bit;
    ccm_querry_info->YAlp.highy_adj_en  = true;
    ccm_querry_info->YAlp.asym_enable   = false;
#elif RKAIQ_HAVE_CCM_V2 || RKAIQ_HAVE_CCM_V3
    ccm_querry_info->YAlp.right_pos_bit = HwCfg->right_bit;
    ccm_querry_info->YAlp.highy_adj_en  = HwCfg->highy_adj_en;
    ccm_querry_info->YAlp.asym_enable   = HwCfg->asym_adj_en;
#endif

    ccm_querry_info->color_inhibition_level = ccm_contex->accmRest.color_inhibition_level;
    ccm_querry_info->color_saturation_level = ccm_contex->accmRest.color_saturation_level;

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
rk_aiq_uapi_accm_SetACcmCof(const rk_aiq_ccm_illucfg_t* attr,
                            const int new_aCcmCof_len,
                            const ccm_calib_initlen_info_t* calib_initlen_info,
                            int* aCcmCof_len,
                            CalibDbV2_Ccm_Accm_Cof_Para_t* aCcmCof)
{
    if (!attr || !aCcmCof) {
        LOGE_ACCM("%s: null input\n", __FUNCTION__);
        return XCAM_RETURN_ERROR_PARAM;
    }
    // memset 0
    for (int i = 0; i < new_aCcmCof_len; i++) {
        if (i > calib_initlen_info->accmCof_initlen - 1) {
            LOGE_ACCM("Failed to set aCcmCof[%d] %s and so on, aCcmCof_len exceeds %d \n",
                      i, attr[i].name, calib_initlen_info->accmCof_initlen);
            return XCAM_RETURN_ERROR_PARAM;
        }
        const accmCof_initlen_info_t* initlen_info = &calib_initlen_info->accmCof_initlen_info[i];
        if (strlen(attr[i].name) > initlen_info->name_len) {
            LOGE_ACCM("Failed to set aCcmCof[%d].name %s, name length exceeds %d \n",
                      i, attr[i].name, initlen_info->name_len);
            return XCAM_RETURN_ERROR_PARAM;
        }
        memset(aCcmCof[i].name, 0, initlen_info->name_len * sizeof(char));
        for (int j = 0; j < attr[i].matrixUsed_len; j++) {
            if (j > initlen_info->matused_len) {
                LOGE_ACCM("Failed to set aCcmCof[%d].matrixUsed[%d] %s and so on, matrixUsed_len exceeds %d \n",
                          i, j, aCcmCof[i].matrixUsed[j], initlen_info->matused_len);
                return XCAM_RETURN_ERROR_PARAM;
            }
            if (strlen(attr[i].matrixUsed[j]) > initlen_info->matused_str_len[j]) {
                LOGE_ACCM("Failed to set aCcmCof[%d].matrixUsed[%d] %s, matrixUsed[%d] length exceeds %d \n",
                        i, j, attr[i].matrixUsed[j], j, initlen_info->matused_str_len[j]);
                return XCAM_RETURN_ERROR_PARAM;
            }
            memset(aCcmCof[i].matrixUsed[j], 0, initlen_info->matused_str_len[j] * sizeof(char));
        }
        aCcmCof[i].matrixUsed_len = attr[i].matrixUsed_len;
    }
    *aCcmCof_len = new_aCcmCof_len;
    // copy
    for (int i = 0; i < new_aCcmCof_len; i++) {
        strcpy(aCcmCof[i].name, attr[i].name);
        memcpy(aCcmCof[i].awbGain, attr[i].awbGain, sizeof(attr[i].awbGain));
        aCcmCof[i].minDist = attr[i].minDist;
        for (int j = 0; j < attr[i].matrixUsed_len; j++) {
            strcpy(aCcmCof[i].matrixUsed[j], attr[i].matrixUsed[j]);
        }
        memcpy(&aCcmCof[i].gain_sat_curve, &attr[i].gain_sat_curve,
               sizeof(attr[i].gain_sat_curve));
    }

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
rk_aiq_uapi_accm_SetMatrixAll(const rk_aiq_ccm_matrixcfg_t* attr,
                            const int new_matrixAll_len,
                            const ccm_calib_initlen_info_t* calib_initlen_info,
                            int* matrixAll_len,
                            CalibDbV2_Ccm_Matrix_Para_t* matrixAll)
{
    if (!attr || !matrixAll) {
        LOGE_ACCM("%s: null input\n", __FUNCTION__);
        return XCAM_RETURN_ERROR_PARAM;
    }
    for (int i = 0; i < new_matrixAll_len; i++) {
        if (i > calib_initlen_info->matrixall_initlen - 1) {
            LOGE_ACCM("Failed to set matrixAll[%d] %s and so on, matrixAll_len exceeds %d \n",
                      i, attr[i].name, calib_initlen_info->matrixall_initlen);
            return XCAM_RETURN_ERROR_PARAM;
        }
        const matrixall_initlen_info_t* initlen_info = &calib_initlen_info->matrixall_initlen_info[i];
        if (strlen(attr[i].name) > initlen_info->name_len) {
            LOGE_ACCM("Failed to set matrixAll[%d].name %s, name length exceeds %d \n",
                      i, attr[i].name, initlen_info->name_len);
            return XCAM_RETURN_ERROR_PARAM;
        }
        memset(matrixAll[i].name, 0, initlen_info->name_len * sizeof(char));
        if (strlen(attr[i].illumination) > initlen_info->illu_len) {
            LOGE_ACCM("Failed to set matrixAll[%d].illumination %s, illumination length exceeds %d \n",
                      i, attr[i].illumination, initlen_info->illu_len);
            return XCAM_RETURN_ERROR_PARAM;
        }
        memset(matrixAll[i].illumination, 0, initlen_info->illu_len * sizeof(char));
    }
    *matrixAll_len = new_matrixAll_len;
    // copy
    for (int i = 0; i < new_matrixAll_len; i++) {
        strcpy(matrixAll[i].name, attr[i].name);
        strcpy(matrixAll[i].illumination, attr[i].illumination);
        matrixAll[i].saturation = attr[i].saturation;
        memcpy(matrixAll[i].ccMatrix, attr[i].ccMatrix, sizeof(attr[i].ccMatrix));
        memcpy(matrixAll[i].ccOffsets, attr[i].ccOffsets, sizeof(attr[i].ccOffsets));
    }

    return XCAM_RETURN_NO_ERROR;
}

#if RKAIQ_HAVE_CCM_V1
XCamReturn
rk_aiq_uapi_accm_SetIqParam(RkAiqAlgoContext *ctx,
                           const rk_aiq_ccm_calib_attrib_t* attr,
                           bool need_sync) {
#elif RKAIQ_HAVE_CCM_V2
XCamReturn
rk_aiq_uapi_accm_SetIqParam(RkAiqAlgoContext *ctx,
                           const rk_aiq_ccm_v2_calib_attrib_t* attr,
                           bool need_sync) {
#elif RKAIQ_HAVE_CCM_V3
XCamReturn
rk_aiq_uapi_accm_SetIqParam(RkAiqAlgoContext *ctx,
                           const rk_aiq_ccm_v3_calib_attrib_t* attr,
                           bool need_sync) {
#endif
    accm_context_t* ccm_contex = (accm_context_t*)ctx->accm_para;

#if RKAIQ_ACCM_ILLU_VOTE
    if (ccm_contex->accmRest.illuNum != attr->aCcmCof_len)
        clear_list(&hAccm->accmRest.dominateIlluList);
#endif
#if RKAIQ_HAVE_CCM_V1
    CalibDbV2_Ccm_Para_V2_t* pCalib = ccm_contex->ccm_v1;
#elif RKAIQ_HAVE_CCM_V2
    CalibDbV2_Ccm_Para_V32_t* pCalib = ccm_contex->ccm_v2;
    memcpy(&pCalib->enhCCM, &attr->iqparam.enhCCM, sizeof(attr->iqparam.enhCCM));
#elif RKAIQ_HAVE_CCM_V3
    CalibDbV2_Ccm_Para_V39_t* pCalib = ccm_contex->ccm_v3;
    memcpy(&pCalib->enhCCM, &attr->iqparam.enhCCM, sizeof(attr->iqparam.enhCCM));
    memcpy(&pCalib->hfCCM, &attr->iqparam.hfCCM, sizeof(attr->iqparam.hfCCM));
#endif
    if (!pCalib) {
        LOGE_ACCM("%s: Failed to set params to ccm_Calib, nullptr \n", __FUNCTION__);
    } else {
        memcpy(&pCalib->control, &attr->iqparam.control, sizeof(attr->iqparam.control));
        memcpy(&pCalib->lumaCCM, &attr->iqparam.lumaCCM, sizeof(attr->iqparam.lumaCCM));
#if RKAIQ_HAVE_CCM_V2
        memcpy(&pCalib->enhCCM, &attr->iqparam.enhCCM, sizeof(attr->iqparam.enhCCM));
#endif
        pCalib->TuningPara.damp_enable = attr->iqparam.damp_enable;
        rk_aiq_uapi_accm_SetACcmCof(attr->iqparam.aCcmCof,
                                    attr->iqparam.aCcmCof_len,
                                    &ccm_contex->accmRest.ccm_calib_initlen_info,
                                    &pCalib->TuningPara.aCcmCof_len,
                                    pCalib->TuningPara.aCcmCof);
        rk_aiq_uapi_accm_SetMatrixAll(attr->iqparam.matrixAll,
                                    attr->iqparam.matrixAll_len,
                                    &ccm_contex->accmRest.ccm_calib_initlen_info,
                                    &pCalib->TuningPara.matrixAll_len,
                                    pCalib->TuningPara.matrixAll);
    }

    ConfigbyCalib(ccm_contex);

    return XCAM_RETURN_NO_ERROR;
}

#if RKAIQ_HAVE_CCM_V1
XCamReturn
rk_aiq_uapi_accm_GetIqParam(const RkAiqAlgoContext *ctx,
                           rk_aiq_ccm_calib_attrib_t* attr) {
#elif RKAIQ_HAVE_CCM_V2
XCamReturn
rk_aiq_uapi_accm_GetIqParam(const RkAiqAlgoContext *ctx,
                           rk_aiq_ccm_v2_calib_attrib_t* attr) {
#elif RKAIQ_HAVE_CCM_V3
XCamReturn
rk_aiq_uapi_accm_GetIqParam(const RkAiqAlgoContext *ctx,
                           rk_aiq_ccm_v3_calib_attrib_t* attr) {
#endif

    accm_context_t* ccm_contex = (accm_context_t*)ctx->accm_para;
#if RKAIQ_HAVE_CCM_V1
    CalibDbV2_Ccm_Para_V2_t* pCalib = ccm_contex->ccm_v1;
#elif RKAIQ_HAVE_CCM_V2
    CalibDbV2_Ccm_Para_V32_t* pCalib = ccm_contex->ccm_v2;
#elif RKAIQ_HAVE_CCM_V3
    CalibDbV2_Ccm_Para_V39_t* pCalib = ccm_contex->ccm_v3;
#endif
    if (!pCalib) {
        LOGE_ACCM("%s: Failed to get ccm_Calib, nullptr\n", __FUNCTION__);
        return XCAM_RETURN_ERROR_PARAM;
    }
    memcpy(&attr->iqparam.control, &pCalib->control, sizeof(pCalib->control));
    memcpy(&attr->iqparam.lumaCCM, &pCalib->lumaCCM, sizeof(pCalib->lumaCCM));
#if RKAIQ_HAVE_CCM_V2
    memcpy(&attr->iqparam.enhCCM, &pCalib->enhCCM, sizeof(pCalib->enhCCM));
#elif RKAIQ_HAVE_CCM_V3
    memcpy(&attr->iqparam.enhCCM, &pCalib->enhCCM, sizeof(pCalib->enhCCM));
    memcpy(&attr->iqparam.hfCCM, &pCalib->hfCCM, sizeof(pCalib->hfCCM));
#endif
    attr->iqparam.damp_enable = pCalib->TuningPara.damp_enable;
    if (pCalib->TuningPara.aCcmCof) {
        for(int i = 0; i < pCalib->TuningPara.aCcmCof_len; i++) {
            strcpy(attr->iqparam.aCcmCof[i].name,
                   pCalib->TuningPara.aCcmCof[i].name);
            memcpy(attr->iqparam.aCcmCof[i].awbGain,
                   pCalib->TuningPara.aCcmCof[i].awbGain,
                   sizeof(pCalib->TuningPara.aCcmCof[i].awbGain));
            attr->iqparam.aCcmCof[i].minDist = pCalib->TuningPara.aCcmCof[i].minDist;
            if(pCalib->TuningPara.aCcmCof[i].matrixUsed) {
                for(int j = 0; j < pCalib->TuningPara.aCcmCof[i].matrixUsed_len; j++) {
                    strcpy(attr->iqparam.aCcmCof[i].matrixUsed[j],
                        pCalib->TuningPara.aCcmCof[i].matrixUsed[j]);
                }
            } else {
                LOGE_ACCM("%s: Failed to get aCcmCof[%d].matrixUsed, nullptr\n", __FUNCTION__, i);
                return XCAM_RETURN_ERROR_PARAM;
            }
            attr->iqparam.aCcmCof[i].matrixUsed_len = pCalib->TuningPara.aCcmCof[i].matrixUsed_len;
            memcpy(&attr->iqparam.aCcmCof[i].gain_sat_curve,
                   &pCalib->TuningPara.aCcmCof[i].gain_sat_curve,
                   sizeof(pCalib->TuningPara.aCcmCof[i].gain_sat_curve));
        }
        attr->iqparam.aCcmCof_len = pCalib->TuningPara.aCcmCof_len;
    } else {
        LOGE_ACCM("%s: Failed to get aCcmCof, nullptr\n", __FUNCTION__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    if (pCalib->TuningPara.matrixAll) {
        for(int i = 0; i < pCalib->TuningPara.matrixAll_len; i++) {
            strcpy(attr->iqparam.matrixAll[i].name,
                   pCalib->TuningPara.matrixAll[i].name);
            strcpy(attr->iqparam.matrixAll[i].illumination,
                   pCalib->TuningPara.matrixAll[i].illumination);
            attr->iqparam.matrixAll[i].saturation = pCalib->TuningPara.matrixAll[i].saturation;
            memcpy(attr->iqparam.matrixAll[i].ccMatrix,
                   pCalib->TuningPara.matrixAll[i].ccMatrix,
                   sizeof(pCalib->TuningPara.matrixAll[i].ccMatrix));
            memcpy(attr->iqparam.matrixAll[i].ccOffsets,
                   pCalib->TuningPara.matrixAll[i].ccOffsets,
                   sizeof(pCalib->TuningPara.matrixAll[i].ccOffsets));
        }
        attr->iqparam.matrixAll_len = pCalib->TuningPara.matrixAll_len;
    } else {
        LOGE_ACCM("%s: Failed to get matrixAll, nullptr\n", __FUNCTION__);
        return XCAM_RETURN_ERROR_PARAM;
    }


    return XCAM_RETURN_NO_ERROR;
}

