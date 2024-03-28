/*
 *  Copyright (c) 2024 Rockchip Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "rk_aiq_algo_types.h"
#include "cnr_types_prvt.h"
#include "xcam_log.h"

#include "algo_types_priv.h"
#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"
#include "RkAiqHandle.h"
#include "interpolation.h"


static XCamReturn
create_context(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    CamCalibDbV2Context_t *pCalibDbV2 = cfg->calibv2;

    CnrContext_t* ctx = new CnrContext_t();

    if (ctx == NULL) {
        LOGE_ANR("%s create context failed!", __func__);
        return XCAM_RETURN_ERROR_MEM;
    }
    ctx->isReCal_ = true;
    ctx->prepare_params = NULL;
    ctx->cnr_attrib =
        (cnr_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibDbV2, cnr));

    *context = (RkAiqAlgoContext* )ctx;
    LOGV_ANR("%s: Cnr (exit)\n", __FUNCTION__ );

    *context = (RkAiqAlgoContext*)ctx;
    return result;
}

static XCamReturn
destroy_context(RkAiqAlgoContext *context)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    CnrContext_t* pCnrCtx = (CnrContext_t*)context;
    delete pCnrCtx;
    return result;
}

static XCamReturn
prepare(RkAiqAlgoCom* params)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    CnrContext_t* pCnrCtx = (CnrContext_t *)params->ctx;

	if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )){
        // just update calib ptr
        if (params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB_PTR) {
            pCnrCtx->cnr_attrib =
                (cnr_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(params->u.prepare.calibv2, cnr));
            return XCAM_RETURN_NO_ERROR;
        }
    }

    pCnrCtx->working_mode = params->u.prepare.working_mode;
    pCnrCtx->cnr_attrib =
        (cnr_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(params->u.prepare.calibv2, cnr));
    pCnrCtx->prepare_params = &params->u.prepare;
    pCnrCtx->isReCal_ = true;

    return result;
}

static XCamReturn
processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    CnrContext_t* pCnrCtx = (CnrContext_t *)inparams->ctx;
    cnr_api_attrib_t* cnr_attrib = pCnrCtx->cnr_attrib;
    RkAiqAlgoProcResCnr* pCnrProcResParams = (RkAiqAlgoProcResCnr*)outparams;
    RkAiqAlgoProcCnr* cnr_proc_param = (RkAiqAlgoProcCnr*)inparams;

    LOGV_ANR("%s: Cnr (enter)\n", __FUNCTION__ );

    if (!cnr_attrib) {
        LOGE_ANR("cnr_attrib is NULL !");
        return XCAM_RETURN_ERROR_MEM;
    }

    float blc_ob_predgain = cnr_proc_param->blc_ob_predgain;
    int iso = inparams->u.proc.iso;

    if (pCnrCtx->working_mode == RK_AIQ_WORKING_MODE_NORMAL)
        iso = iso * blc_ob_predgain;

    bool init = inparams->u.proc.init;
    int delta_iso = abs(iso - pCnrCtx->iso);

    outparams->cfg_update = false;

    if (cnr_attrib->opMode != RK_AIQ_OP_MODE_AUTO) {
        LOGE_ANR("mode is %d, not auto mode, ignore", cnr_attrib->opMode);
        return XCAM_RETURN_NO_ERROR;
    }

    if (inparams->u.proc.is_attrib_update)
        pCnrCtx->isReCal_ = true;

    if (delta_iso > DEFAULT_RECALCULATE_DELTA_ISO)
        pCnrCtx->isReCal_ = true;

    if (pCnrCtx->isReCal_) {
        CnrSelectParam(&pCnrCtx->cnr_attrib->stAuto, pCnrProcResParams->cnrRes, iso);
        outparams->cfg_update = true;
        outparams->en = cnr_attrib->en;
        outparams->bypass = cnr_attrib->bypass;
        LOGD_ANR("CNR Process: iso %d, ynr en:%d, bypass:%d", iso, outparams->en, outparams->bypass);
    }

    pCnrCtx->iso = iso;
    pCnrCtx->isReCal_ = false;

    LOGV_ANR("%s: Cnr (exit)\n", __FUNCTION__ );
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
algo_cnr_SetAttrib(RkAiqAlgoContext *ctx,
                                    const cnr_api_attrib_t *attr,
                                    bool need_sync)
{

    if(ctx == NULL || attr == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    CnrContext_t* pCnrCtx = (CnrContext_t*)ctx;
    cnr_api_attrib_t* cnr_attrib = pCnrCtx->cnr_attrib;

    if (attr->opMode != RK_AIQ_OP_MODE_AUTO) {
        LOGE_ANR("not auto mode: %d", attr->opMode);
        return XCAM_RETURN_ERROR_PARAM;
    }

    cnr_attrib->opMode = attr->opMode;
    cnr_attrib->en = attr->en;
    cnr_attrib->bypass = attr->bypass;

    if (attr->opMode == RK_AIQ_OP_MODE_AUTO)
        cnr_attrib->stAuto = attr->stAuto;
    else if (attr->opMode == RK_AIQ_OP_MODE_MANUAL)
        cnr_attrib->stMan = attr->stMan;
    else {
        LOGW_ANR("wrong mode: %d\n", attr->opMode);
    }

    pCnrCtx->isReCal_ = true;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
algo_cnr_GetAttrib(const RkAiqAlgoContext *ctx,
                                    cnr_api_attrib_t *attr)
{

    if(ctx == NULL || attr == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    CnrContext_t* pCnrCtx = (CnrContext_t*)ctx;
    cnr_api_attrib_t* cnr_attrib = pCnrCtx->cnr_attrib;

    attr->opMode = cnr_attrib->opMode;
    attr->en = cnr_attrib->en;
    attr->bypass = cnr_attrib->bypass;
    memcpy(&attr->stAuto, &cnr_attrib->stAuto, sizeof(cnr_param_auto_t));

    return XCAM_RETURN_NO_ERROR;
}

#if RKAIQ_HAVE_CNR_V30
XCamReturn CnrSelectParam
(
    cnr_param_auto_t *pAuto,
    cnr_param_t* out,
    int iso)
{
    if(pAuto == NULL || out == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    int i = 0;
    int iso_low = 0, iso_high = 0, ilow = 0, ihigh = 0;
    float ratio = 0.0f;
    uint16_t uratio;
    cnr_param_auto_t *paut = pAuto;

    pre_interp(iso, NULL, 0, ilow, ihigh, ratio);
    uratio = ratio * (1 << RATIO_FIXBIT);

    return XCAM_RETURN_NO_ERROR;
}
#endif

#if RKAIQ_HAVE_CNR_V31
XCamReturn CnrSelectParam
(
    cnr_param_auto_t *pAuto,
    cnr_param_t* out,
    int iso)
{
    if(pAuto == NULL || out == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    int i = 0;
    int iso_low = 0, iso_high = 0, ilow = 0, ihigh = 0, inear = 0;
    float ratio = 0.0f;
    uint16_t uratio;
    cnr_param_auto_t *paut = pAuto;

    pre_interp(iso, NULL, 0, ilow, ihigh, ratio);
    uratio = ratio * (1 << RATIO_FIXBIT);

    if (ratio > 0.5)
        inear = ihigh;
    else
        inear = ilow;

    out->dyn.locSgmStrg.hw_cnrT_glbSgmStrg_val = interpolation_f32(
        paut->dyn[ilow].locSgmStrg.hw_cnrT_glbSgmStrg_val, paut->dyn[ihigh].locSgmStrg.hw_cnrT_glbSgmStrg_val, ratio);
    out->dyn.locSgmStrg.hw_cnrT_glbSgmStrg_alpha = interpolation_f32(
        paut->dyn[ilow].locSgmStrg.hw_cnrT_glbSgmStrg_alpha, paut->dyn[ihigh].locSgmStrg.hw_cnrT_glbSgmStrg_alpha, ratio);
    out->dyn.locSgmStrg.hw_cnrT_locSgmStrg_scale = interpolation_f32(
        paut->dyn[ilow].locSgmStrg.hw_cnrT_locSgmStrg_scale, paut->dyn[ihigh].locSgmStrg.hw_cnrT_locSgmStrg_scale, ratio);
    out->dyn.loNrGuide_preProc.hw_cnrT_ds_mode = paut->dyn[inear].loNrGuide_preProc.hw_cnrT_ds_mode;
    out->dyn.loNrGuide_preProc.hw_cnrT_uvEdg_strg = interpolation_f32(
        paut->dyn[ilow].loNrGuide_preProc.hw_cnrT_uvEdg_strg, paut->dyn[ihigh].loNrGuide_preProc.hw_cnrT_uvEdg_strg, ratio);
    out->dyn.loNrGuide_bifilt.sw_cnrT_filtCfg_mode = paut->dyn[inear].loNrGuide_bifilt.sw_cnrT_filtCfg_mode;
    out->dyn.loNrGuide_bifilt.sw_cnrT_filtSpatial_strg = interpolation_f32(
        paut->dyn[ilow].loNrGuide_bifilt.sw_cnrT_filtSpatial_strg, paut->dyn[ihigh].loNrGuide_bifilt.sw_cnrT_filtSpatial_strg, ratio);
    for (i=0; i<4; i++) {
        out->dyn.loNrGuide_bifilt.hw_cnrT_filtSpatial_wgt[i] = paut->dyn[inear].loNrGuide_bifilt.hw_cnrT_filtSpatial_wgt[i];
    }
    out->dyn.loNrGuide_bifilt.sw_cnrT_rgeSgm_val = interpolation_f32(
        paut->dyn[ilow].loNrGuide_bifilt.sw_cnrT_rgeSgm_val, paut->dyn[ihigh].loNrGuide_bifilt.sw_cnrT_rgeSgm_val, ratio);
    out->dyn.loNrGuide_bifilt.hw_cnrT_bifiltOut_alpha = interpolation_f32(
        paut->dyn[ilow].loNrGuide_bifilt.hw_cnrT_bifiltOut_alpha, paut->dyn[ihigh].loNrGuide_bifilt.hw_cnrT_bifiltOut_alpha, ratio);
    for (i=0; i<6; i++) {
        out->dyn.loNrGuide_iirFilt.hw_cnrT_filtSpatial_wgt[i] = paut->dyn[inear].loNrGuide_iirFilt.hw_cnrT_filtSpatial_wgt[i];
    }
    out->dyn.loNrGuide_iirFilt.sw_cnrT_rgeSgm_val = interpolation_f32(
        paut->dyn[ilow].loNrGuide_iirFilt.sw_cnrT_rgeSgm_val, paut->dyn[ihigh].loNrGuide_iirFilt.sw_cnrT_rgeSgm_val, ratio);
    out->dyn.loNrGuide_iirFilt.sw_cnrT_rgeSgmRatio_mode = paut->dyn[inear].loNrGuide_iirFilt.sw_cnrT_rgeSgmRatio_mode;
    out->dyn.loNrGuide_iirFilt.hw_cnrT_glbSgm_ratio = interpolation_f32(
        paut->dyn[ilow].loNrGuide_iirFilt.hw_cnrT_glbSgm_ratio, paut->dyn[ihigh].loNrGuide_iirFilt.hw_cnrT_glbSgm_ratio, ratio);
    out->dyn.loNrGuide_iirFilt.hw_cnrT_glbSgmRatio_alpha = interpolation_f32(
        paut->dyn[ilow].loNrGuide_iirFilt.hw_cnrT_glbSgmRatio_alpha, paut->dyn[ihigh].loNrGuide_iirFilt.hw_cnrT_glbSgmRatio_alpha, ratio);
    out->dyn.loNrGuide_iirFilt.hw_cnrT_sgm2NhoodWgt_slope = interpolation_f32(
        paut->dyn[ilow].loNrGuide_iirFilt.hw_cnrT_sgm2NhoodWgt_slope, paut->dyn[ihigh].loNrGuide_iirFilt.hw_cnrT_sgm2NhoodWgt_slope, ratio);
    out->dyn.loNrGuide_iirFilt.hw_cnrT_nhoodWgtZero_thred = interpolation_f32(
        paut->dyn[ilow].loNrGuide_iirFilt.hw_cnrT_nhoodWgtZero_thred, paut->dyn[ihigh].loNrGuide_iirFilt.hw_cnrT_nhoodWgtZero_thred, ratio);
    out->dyn.loNrGuide_iirFilt.hw_cnrT_iirFiltStrg_maxLimit = interpolation_f32(
        paut->dyn[ilow].loNrGuide_iirFilt.hw_cnrT_iirFiltStrg_maxLimit, paut->dyn[ihigh].loNrGuide_iirFilt.hw_cnrT_iirFiltStrg_maxLimit, ratio);
    out->dyn.hiNr_bifilt.hw_cnrT_filtWgtZero_mode = paut->dyn[inear].hiNr_bifilt.hw_cnrT_filtWgtZero_mode;
    out->dyn.hiNr_preLpf.sw_cnrT_filtCfg_mode = paut->dyn[inear].hiNr_preLpf.sw_cnrT_filtCfg_mode;
    out->dyn.hiNr_preLpf.sw_cnrT_filtSpatial_strg = interpolation_f32(
        paut->dyn[ilow].hiNr_preLpf.sw_cnrT_filtSpatial_strg, paut->dyn[ihigh].hiNr_preLpf.sw_cnrT_filtSpatial_strg, ratio);
    for (i=0; i<6; i++) {
        out->dyn.hiNr_preLpf.hw_cnrT_filtSpatial_wgt[i] = interpolation_f32(
            paut->dyn[ilow].hiNr_preLpf.hw_cnrT_filtSpatial_wgt[i], paut->dyn[ihigh].hiNr_preLpf.hw_cnrT_filtSpatial_wgt[i], ratio);
    }
    out->dyn.hiNr_preLpf.hw_cnrT_lpfOut_alpha = interpolation_f32(
        paut->dyn[ilow].hiNr_preLpf.hw_cnrT_lpfOut_alpha, paut->dyn[ihigh].hiNr_preLpf.hw_cnrT_lpfOut_alpha, ratio);
    for (i=0; i<8; i++) {
        out->dyn.hw_cnrC_luma2HiNrSgm_curve.idx[i] = paut->dyn[inear].hw_cnrC_luma2HiNrSgm_curve.idx[i];
    }
    for (i=0; i<8; i++) {
        out->dyn.hw_cnrC_luma2HiNrSgm_curve.val[i] = interpolation_f32(
            paut->dyn[ilow].hw_cnrC_luma2HiNrSgm_curve.val[i], paut->dyn[ihigh].hw_cnrC_luma2HiNrSgm_curve.val[i], ratio);
    }
    out->dyn.hiNr_bifilt.hw_cnrT_uvEdg_strg = interpolation_f32(
        paut->dyn[ilow].hiNr_bifilt.hw_cnrT_uvEdg_strg, paut->dyn[ihigh].hiNr_bifilt.hw_cnrT_uvEdg_strg, ratio);
    for (i=0; i<13; i++) {
        out->dyn.hiNr_bifilt.hw_cnrT_locSgmStrg2SgmRat_val[i] = interpolation_f32(
            paut->dyn[ilow].hiNr_bifilt.hw_cnrT_locSgmStrg2SgmRat_val[i], paut->dyn[ihigh].hiNr_bifilt.hw_cnrT_locSgmStrg2SgmRat_val[i], ratio);
    }
    for (i=0; i<13; i++) {
        out->dyn.hiNr_bifilt.hw_cnrT_locSgmStrg2CtrPix_wgt[i] = interpolation_f32(
            paut->dyn[ilow].hiNr_bifilt.hw_cnrT_locSgmStrg2CtrPix_wgt[i], paut->dyn[ihigh].hiNr_bifilt.hw_cnrT_locSgmStrg2CtrPix_wgt[i], ratio);
    }
    out->dyn.hiNr_bifilt.hw_cnrT_nhoodWgt_minLimit = interpolation_f32(
        paut->dyn[ilow].hiNr_bifilt.hw_cnrT_nhoodWgt_minLimit, paut->dyn[ihigh].hiNr_bifilt.hw_cnrT_nhoodWgt_minLimit, ratio);
    out->dyn.hiNr_bifilt.hw_cnrT_satAdj_offset = interpolation_f32(
        paut->dyn[ilow].hiNr_bifilt.hw_cnrT_satAdj_offset, paut->dyn[ihigh].hiNr_bifilt.hw_cnrT_satAdj_offset, ratio);
    out->dyn.hiNr_bifilt.hw_cnrT_satAdj_scale = interpolation_f32(
        paut->dyn[ilow].hiNr_bifilt.hw_cnrT_satAdj_scale, paut->dyn[ihigh].hiNr_bifilt.hw_cnrT_satAdj_scale, ratio);
    out->dyn.hiNr_bifilt.hw_cnrT_bifiltOut_alpha = interpolation_f32(
        paut->dyn[ilow].hiNr_bifilt.hw_cnrT_bifiltOut_alpha, paut->dyn[ihigh].hiNr_bifilt.hw_cnrT_bifiltOut_alpha, ratio);
    return XCAM_RETURN_NO_ERROR;
}

#endif

#define RKISP_ALGO_CNR_VERSION     "v0.0.9"
#define RKISP_ALGO_CNR_VENDOR      "Rockchip"
#define RKISP_ALGO_CNR_DESCRIPTION "Rockchip cnr algo for ISP32"

RkAiqAlgoDescription g_RkIspAlgoDescCnr = {
    .common = {
        .version = RKISP_ALGO_CNR_VERSION,
        .vendor  = RKISP_ALGO_CNR_VENDOR,
        .description = RKISP_ALGO_CNR_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_ACNR,
        .id      = 0,
        .create_context  = create_context,
        .destroy_context = destroy_context,
    },
    .prepare = prepare,
    .pre_process = NULL,
    .processing = processing,
    .post_process = NULL,
};

