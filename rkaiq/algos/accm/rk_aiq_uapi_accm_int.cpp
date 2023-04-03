#include "rk_aiq_uapi_accm_int.h"
#include "accm/rk_aiq_types_accm_algo_prvt.h"

#if RKAIQ_HAVE_CCM_V1
XCamReturn
rk_aiq_uapi_accm_SetAttrib(RkAiqAlgoContext *ctx,
                           const rk_aiq_ccm_attrib_t* attr,
                           bool need_sync)
{

    accm_context_t* ccm_contex = (accm_context_t*)ctx->accm_para;
    ccm_contex->mNewAtt = *attr;
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
    ccm_contex->mNewAttV2 = *attr;
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

XCamReturn
rk_aiq_uapi_accm_QueryCcmInfo(const RkAiqAlgoContext *ctx,
                              rk_aiq_ccm_querry_info_t *ccm_querry_info )
{

    accm_context_t* ccm_contex = (accm_context_t*)ctx->accm_para;
    ccm_querry_info->finalSat = 0;
    memset(ccm_querry_info->ccmname1, 0x0, sizeof(ccm_querry_info->ccmname1));
    memset(ccm_querry_info->ccmname2, 0x0, sizeof(ccm_querry_info->ccmname2));

#if RKAIQ_HAVE_CCM_V1
    if (ccm_contex->ccmHwConf.ccmEnable && ccm_contex->mCurAtt.mode == RK_AIQ_CCM_MODE_AUTO){
        ccm_querry_info->finalSat = ccm_contex->accmRest.fSaturation;
        if (ccm_contex->accmRest.pCcmProfile1) {
            if (ccm_contex->accmRest.pCcmProfile1->name) {
                strcpy(ccm_querry_info->ccmname1, ccm_contex->accmRest.pCcmProfile1->name);
            }
        }
        if (ccm_contex->accmRest.pCcmProfile2) {
            if (ccm_contex->accmRest.pCcmProfile2->name) {
                strcpy(ccm_querry_info->ccmname2, ccm_contex->accmRest.pCcmProfile2->name);
            }
        }
        else
            strcpy(ccm_querry_info->ccmname2, ccm_querry_info->ccmname1);
    }
    memcpy(ccm_querry_info->y_alpha_curve, ccm_contex->ccmHwConf.alp_y, sizeof(ccm_contex->ccmHwConf.alp_y));
    memcpy(ccm_querry_info->ccMatrix, ccm_contex->ccmHwConf.matrix, sizeof(ccm_contex->ccmHwConf.matrix));
    memcpy(ccm_querry_info->ccOffsets, ccm_contex->ccmHwConf.offs, sizeof(ccm_contex->ccmHwConf.offs));
    ccm_querry_info->ccm_en = ccm_contex->ccmHwConf.ccmEnable;
    ccm_querry_info->low_bound_pos_bit = ccm_contex->ccmHwConf.bound_bit;
    ccm_querry_info->right_pos_bit = ccm_contex->ccmHwConf.bound_bit;
    ccm_querry_info->highy_adj_en = true;
    ccm_querry_info->asym_enable = false;
#endif

#if RKAIQ_HAVE_CCM_V2
    if (ccm_contex->ccmHwConf_v2.ccmEnable && ccm_contex->mCurAttV2.mode == RK_AIQ_CCM_MODE_AUTO){
        ccm_querry_info->finalSat = ccm_contex->accmRest.fSaturation;
        if (ccm_contex->accmRest.pCcmProfile1) {
            if (ccm_contex->accmRest.pCcmProfile1->name) {
                strcpy(ccm_querry_info->ccmname1, ccm_contex->accmRest.pCcmProfile1->name);
            }
        }
        if (ccm_contex->accmRest.pCcmProfile2) {
            if (ccm_contex->accmRest.pCcmProfile2->name) {
                strcpy(ccm_querry_info->ccmname2, ccm_contex->accmRest.pCcmProfile2->name);
            }
        }
        else
            strcpy(ccm_querry_info->ccmname2, ccm_querry_info->ccmname1);
    }
    ccm_querry_info->highy_adj_en = ccm_contex->ccmHwConf_v2.highy_adj_en;
    ccm_querry_info->asym_enable  = ccm_contex->ccmHwConf_v2.asym_adj_en;
    memcpy(ccm_querry_info->y_alpha_curve, ccm_contex->ccmHwConf_v2.alp_y,
           sizeof(ccm_contex->ccmHwConf_v2.alp_y));
    memcpy(ccm_querry_info->ccMatrix, ccm_contex->ccmHwConf_v2.matrix,
           sizeof(ccm_contex->ccmHwConf_v2.matrix));
    memcpy(ccm_querry_info->ccOffsets, ccm_contex->ccmHwConf_v2.offs,
           sizeof(ccm_contex->ccmHwConf_v2.offs));
    ccm_querry_info->ccm_en            = ccm_contex->ccmHwConf_v2.ccmEnable;
    ccm_querry_info->low_bound_pos_bit = ccm_contex->ccmHwConf_v2.bound_bit;
    ccm_querry_info->right_pos_bit     = ccm_contex->ccmHwConf_v2.right_bit;
#endif

    ccm_querry_info->color_inhibition_level = ccm_contex->accmRest.color_inhibition_level;
    ccm_querry_info->color_saturation_level = ccm_contex->accmRest.color_saturation_level;

    return XCAM_RETURN_NO_ERROR;
}


