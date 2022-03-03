#include "rk_aiq_uapi_accm_int.h"
#include "accm/rk_aiq_types_accm_algo_prvt.h"
XCamReturn
rk_aiq_uapi_accm_SetAttrib(RkAiqAlgoContext *ctx,
                           rk_aiq_ccm_attrib_t attr,
                           bool need_sync)
{

    accm_context_t* ccm_contex = (accm_context_t*)ctx->accm_para;
    ccm_contex->mNewAtt = attr;
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

XCamReturn
rk_aiq_uapi_accm_QueryCcmInfo(const RkAiqAlgoContext *ctx,
                              rk_aiq_ccm_querry_info_t *ccm_querry_info )
{

    accm_context_t* ccm_contex = (accm_context_t*)ctx->accm_para;
    memcpy(ccm_querry_info->y_alpha_curve, ccm_contex->ccmHwConf.alp_y, sizeof(ccm_contex->ccmHwConf.alp_y));
    memcpy(ccm_querry_info->ccMatrix, ccm_contex->ccmHwConf.matrix, sizeof(ccm_contex->ccmHwConf.matrix));
    memcpy(ccm_querry_info->ccOffsets, ccm_contex->ccmHwConf.offs, sizeof(ccm_contex->ccmHwConf.offs));
    ccm_querry_info->ccm_en = ccm_contex->ccmHwConf.ccmEnable;
    ccm_querry_info->low_bound_pos_bit = ccm_contex->ccmHwConf.bound_bit;
    ccm_querry_info->color_inhibition_level = ccm_contex->accmRest.color_inhibition_level;
    ccm_querry_info->color_saturation_level = ccm_contex->accmRest.color_saturation_level;

    ccm_querry_info->finalSat = 0;
    memset(ccm_querry_info->ccmname1, 0x0, sizeof(ccm_querry_info->ccmname1));
    memset(ccm_querry_info->ccmname2, 0x0, sizeof(ccm_querry_info->ccmname2));
    if (ccm_querry_info->ccm_en && ccm_contex->mCurAtt.mode == RK_AIQ_CCM_MODE_AUTO){
        ccm_querry_info->finalSat = ccm_contex->accmRest.fSaturation;
        strcpy(ccm_querry_info->ccmname1, ccm_contex->accmRest.pCcmProfile1->name);
        if (ccm_contex->accmRest.pCcmProfile2)
            strcpy(ccm_querry_info->ccmname2, ccm_contex->accmRest.pCcmProfile2->name);
        else
            strcpy(ccm_querry_info->ccmname2, ccm_contex->accmRest.pCcmProfile1->name);
    }

    return XCAM_RETURN_NO_ERROR;
}


