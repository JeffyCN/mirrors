#include "rk_aiq_uapi_alsc_int.h"
#include "alsc/rk_aiq_types_alsc_algo_prvt.h"
XCamReturn
rk_aiq_uapi_alsc_SetAttrib(RkAiqAlgoContext *ctx,
                           rk_aiq_lsc_attrib_t attr,
                           bool need_sync)
{

    alsc_context_t* lsc_contex = (alsc_context_t*)ctx->alsc_para;
    lsc_contex->mNewAtt = attr;
    lsc_contex->updateAtt = true;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi_alsc_GetAttrib(const RkAiqAlgoContext *ctx,
                           rk_aiq_lsc_attrib_t *attr)
{

    alsc_context_t* lsc_contex = (alsc_context_t*)ctx->alsc_para;;

    memcpy(attr, &lsc_contex->mCurAtt, sizeof(rk_aiq_lsc_attrib_t));

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi_alsc_QueryLscInfo(const RkAiqAlgoContext *ctx,
                              rk_aiq_lsc_querry_info_t *lsc_querry_info )
{

    alsc_context_t* lsc_contex = (alsc_context_t*)ctx->alsc_para;;
    memcpy(lsc_querry_info->r_data_tbl, lsc_contex->lscHwConf.r_data_tbl, sizeof(lsc_contex->lscHwConf.r_data_tbl));
    memcpy(lsc_querry_info->gr_data_tbl, lsc_contex->lscHwConf.gr_data_tbl, sizeof(lsc_contex->lscHwConf.gr_data_tbl));
    memcpy(lsc_querry_info->gb_data_tbl, lsc_contex->lscHwConf.gb_data_tbl, sizeof(lsc_contex->lscHwConf.gb_data_tbl));
    memcpy(lsc_querry_info->b_data_tbl, lsc_contex->lscHwConf.b_data_tbl, sizeof(lsc_contex->lscHwConf.b_data_tbl));
    lsc_querry_info->lsc_en = lsc_contex->lscHwConf.lsc_en;

    return XCAM_RETURN_NO_ERROR;
}


