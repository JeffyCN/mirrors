#include "rk_aiq_uapi_alsc_int.h"
#include "alsc/rk_aiq_types_alsc_algo_prvt.h"
XCamReturn
rk_aiq_uapi_alsc_SetAttrib(RkAiqAlgoContext *ctx,
                           rk_aiq_lsc_attrib_t attr,
                           bool need_sync)
{
    alsc_context_t* lsc_contex = (alsc_context_t*)ctx->alsc_para;
    lsc_contex->mCurAtt = attr;
    lsc_contex->updateAtt = true;
    lsc_contex->smartRunRes.forceRunFlag = true;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi_alsc_GetAttrib(const RkAiqAlgoContext *ctx,
                           rk_aiq_lsc_attrib_t *attr)
{
    alsc_context_t* lsc_contex = (alsc_context_t*)ctx->alsc_para;;

    // use stManual as current hw params
    memcpy(lsc_contex->mCurAtt.stManual.lsc_sect_size_x, lsc_contex->lscHwConf.x_size_tbl,
           sizeof(lsc_contex->mCurAtt.stManual.lsc_sect_size_x));
    memcpy(lsc_contex->mCurAtt.stManual.lsc_sect_size_y, lsc_contex->lscHwConf.y_size_tbl,
           sizeof(lsc_contex->mCurAtt.stManual.lsc_sect_size_y));
    memcpy(lsc_contex->mCurAtt.stManual.r_data_tbl, lsc_contex->lscHwConf.r_data_tbl,
           sizeof(lsc_contex->mCurAtt.stManual.r_data_tbl));
    memcpy(lsc_contex->mCurAtt.stManual.gr_data_tbl, lsc_contex->lscHwConf.gr_data_tbl,
           sizeof(lsc_contex->mCurAtt.stManual.gr_data_tbl));
    memcpy(lsc_contex->mCurAtt.stManual.gb_data_tbl, lsc_contex->lscHwConf.gb_data_tbl,
           sizeof(lsc_contex->mCurAtt.stManual.gb_data_tbl));
    memcpy(lsc_contex->mCurAtt.stManual.b_data_tbl, lsc_contex->lscHwConf.b_data_tbl,
           sizeof(lsc_contex->mCurAtt.stManual.b_data_tbl));

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


