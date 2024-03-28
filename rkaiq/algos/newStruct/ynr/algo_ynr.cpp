/*
 * algo_ynr.cpp
 *
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
#include "ynr_types_prvt.h"
#include "xcam_log.h"

#include "algo_types_priv.h"
#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"
#include "RkAiqHandle.h"
#include "interpolation.h"

#define YNR_V22_ISO_CURVE_POINT_BIT          4
#define YNR_V22_ISO_CURVE_POINT_NUM          ((1 << YNR_V22_ISO_CURVE_POINT_BIT)+1)

#if RKAIQ_HAVE_YNR_V24
static void ynr_init_params_json_V22(ynr_api_attrib_t *attrib) {
    int i = 0;
    int j = 0;
    short isoCurveSectValue;
    short isoCurveSectValue1;
    float ave1, ave2, ave3, ave4;
    int bit_calib = 12;
    int bit_proc;
    int bit_shift;

    bit_proc = 10; // for V22, YNR_SIGMA_BITS = 10
    bit_shift = bit_calib - bit_proc;

    isoCurveSectValue = (1 << (bit_calib - 4));
    isoCurveSectValue1 = (1 << bit_calib);

    for(j = 0; j<13; j++) {
        ynr_params_dyn_t* pdyn = &(attrib->stAuto.dyn[j]);
        // get noise sigma sample data at [0, 64, 128, ... , 1024]
        for (i = 0; i < YNR_V22_ISO_CURVE_POINT_NUM; i++) {
            if (i == (YNR_V22_ISO_CURVE_POINT_NUM - 1)) {
                ave1 = (float)isoCurveSectValue1;
            } else {
                ave1 = (float)(i * isoCurveSectValue);
            }
            pdyn->hw_ynrC_luma2Sigma_curve.idx[i] = (short)ave1;
            ave2 = ave1 * ave1;
            ave3 = ave2 * ave1;
            ave4 = ave3 * ave1;
            pdyn->hw_ynrC_luma2Sigma_curve.val[i] = pdyn->coeff2SgmCurve.sigma_coeff[0] * ave4
                    + pdyn->coeff2SgmCurve.sigma_coeff[1] * ave3
                    + pdyn->coeff2SgmCurve.sigma_coeff[2] * ave2
                    + pdyn->coeff2SgmCurve.sigma_coeff[3] * ave1
                    + pdyn->coeff2SgmCurve.sigma_coeff[4];

            //if (pdyn->hw_ynrC_luma2Sigma_curve.val[i] < 0) {
            //    pdyn->hw_ynrC_luma2Sigma_curve.val[i] = 0;
            //}

            if (bit_shift > 0) {
                pdyn->hw_ynrC_luma2Sigma_curve.idx[i] >>= bit_shift;
            } else {
                pdyn->hw_ynrC_luma2Sigma_curve.idx[i] <<= ABS(bit_shift);
            }
        }
    }
}
#endif

static XCamReturn
create_context(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    CamCalibDbV2Context_t *pCalibDbV2 = cfg->calibv2;

    YnrContext_t* ctx = new YnrContext_t();

    if (ctx == NULL) {
        LOGE_ANR("%s create context failed!", __func__);
        return XCAM_RETURN_ERROR_MEM;
    }
    ctx->isReCal_ = true;
    ctx->prepare_params = NULL;
    ctx->ynr_attrib =
        (ynr_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibDbV2, ynr));

    *context = (RkAiqAlgoContext* )ctx;
    LOGV_ANR("%s: Ynr (exit)\n", __FUNCTION__ );

    *context = (RkAiqAlgoContext*)ctx;
    return result;
}

static XCamReturn
destroy_context(RkAiqAlgoContext *context)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    YnrContext_t* pYnrCtx = (YnrContext_t*)context;
    delete pYnrCtx;
    return result;
}

static XCamReturn
prepare(RkAiqAlgoCom* params)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    YnrContext_t* pYnrCtx = (YnrContext_t *)params->ctx;

	if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )){
        // just update calib ptr
        if (params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB_PTR) {
            pYnrCtx->ynr_attrib =
                (ynr_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(params->u.prepare.calibv2, ynr));
            return XCAM_RETURN_NO_ERROR;
        }
    }

    pYnrCtx->working_mode = params->u.prepare.working_mode;
    pYnrCtx->ynr_attrib =
        (ynr_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(params->u.prepare.calibv2, ynr));
    pYnrCtx->prepare_params = &params->u.prepare;
    pYnrCtx->isReCal_ = true;

    return result;
}

static XCamReturn
processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    YnrContext_t* pYnrCtx = (YnrContext_t *)inparams->ctx;
    ynr_api_attrib_t* ynr_attrib = pYnrCtx->ynr_attrib;
    RkAiqAlgoProcResYnr* pYnrProcResParams = (RkAiqAlgoProcResYnr*)outparams;
    RkAiqAlgoProcYnr* ynr_proc_param = (RkAiqAlgoProcYnr*)inparams;

    LOGV_ANR("%s: Ynr (enter)\n", __FUNCTION__ );

    if (!ynr_attrib) {
        LOGE_ANR("ynr_attrib is NULL !");
        return XCAM_RETURN_ERROR_MEM;
    }

    float blc_ob_predgain = ynr_proc_param->blc_ob_predgain;
    int iso = inparams->u.proc.iso;

    if (pYnrCtx->working_mode == RK_AIQ_WORKING_MODE_NORMAL)
        iso = iso * blc_ob_predgain;

    bool init = inparams->u.proc.init;
    int delta_iso = abs(iso - pYnrCtx->iso);

    outparams->cfg_update = false;

    if (ynr_attrib->opMode != RK_AIQ_OP_MODE_AUTO) {
        LOGE_ANR("mode is %d, not auto mode, ignore", ynr_attrib->opMode);
        return XCAM_RETURN_NO_ERROR;
    }

    if (init) {
#if RKAIQ_HAVE_YNR_V24
        ynr_init_params_json_V22(pYnrCtx->ynr_attrib);
#endif
        pYnrCtx->isReCal_ = true;
    }

    if (inparams->u.proc.is_attrib_update)
        pYnrCtx->isReCal_ = true;

    if (delta_iso > DEFAULT_RECALCULATE_DELTA_ISO)
        pYnrCtx->isReCal_ = true;

    if (pYnrCtx->isReCal_) {
        pYnrProcResParams->ynrRes->sta = pYnrCtx->ynr_attrib->stAuto.sta;
        YnrSelectParam(&pYnrCtx->ynr_attrib->stAuto, pYnrProcResParams->ynrRes, iso);
        outparams->cfg_update = true;
        outparams->en = ynr_attrib->en;
        outparams->bypass = ynr_attrib->bypass;
        LOGD_ANR("YNR Process: iso %d, ynr en:%d, bypass:%d", iso, outparams->en, outparams->bypass);
    }

    pYnrCtx->iso = iso;
    pYnrCtx->isReCal_ = false;

    LOGV_ANR("%s: Ynr (exit)\n", __FUNCTION__ );
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
algo_ynr_SetAttrib(RkAiqAlgoContext *ctx,
                                    const ynr_api_attrib_t *attr,
                                    bool need_sync)
{

    if(ctx == NULL || attr == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    YnrContext_t* pYnrCtx = (YnrContext_t*)ctx;
    ynr_api_attrib_t* ynr_attrib = pYnrCtx->ynr_attrib;

    if (attr->opMode != RK_AIQ_OP_MODE_AUTO) {
        LOGE_ANR("not auto mode: %d", attr->opMode);
        return XCAM_RETURN_ERROR_PARAM;
    }

    ynr_attrib->opMode = attr->opMode;
    ynr_attrib->en = attr->en;
    ynr_attrib->bypass = attr->bypass;

    if (attr->opMode == RK_AIQ_OP_MODE_AUTO)
        ynr_attrib->stAuto = attr->stAuto;
    else if (attr->opMode == RK_AIQ_OP_MODE_MANUAL)
        ynr_attrib->stMan = attr->stMan;
    else {
        LOGW_ANR("wrong mode: %d\n", attr->opMode);
    }

    pYnrCtx->isReCal_ = true;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
algo_ynr_GetAttrib(const RkAiqAlgoContext *ctx,
                                    ynr_api_attrib_t *attr)
{

    if(ctx == NULL || attr == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    YnrContext_t* pYnrCtx = (YnrContext_t*)ctx;
    ynr_api_attrib_t* ynr_attrib = pYnrCtx->ynr_attrib;

    attr->opMode = ynr_attrib->opMode;
    attr->en = ynr_attrib->en;
    attr->bypass = ynr_attrib->bypass;
    memcpy(&attr->stAuto, &ynr_attrib->stAuto, sizeof(ynr_param_auto_t));

    return XCAM_RETURN_NO_ERROR;
}

#if RKAIQ_HAVE_YNR_V22
XCamReturn YnrSelectParam
(
    ynr_param_auto_t *pAuto,
    ynr_param_t* out,
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
    ynr_param_auto_t *paut = pAuto;

    pre_interp(iso, NULL, 0, ilow, ihigh, ratio);
    uratio = ratio * (1 << RATIO_FIXBIT);
{
    out->dyn.loNrBifilt3.hw_ynrT_bifilt3_en = interpolation_bool(
        paut->dyn[ilow].loNrBifilt3.hw_ynrT_bifilt3_en, paut->dyn[ihigh].loNrBifilt3.hw_ynrT_bifilt3_en, uratio);
    out->dyn.loNrBifilt3.sw_ynrT_guideImgLpf_strg = interpolation_f32(
        paut->dyn[ilow].loNrBifilt3.sw_ynrT_guideImgLpf_strg, paut->dyn[ihigh].loNrBifilt3.sw_ynrT_guideImgLpf_strg, ratio);
    out->dyn.loNrBifilt3.hw_ynrT_guideImgLpf_alpha = interpolation_f32(
        paut->dyn[ilow].loNrBifilt3.hw_ynrT_guideImgLpf_alpha, paut->dyn[ihigh].loNrBifilt3.hw_ynrT_guideImgLpf_alpha, ratio);
    out->dyn.loNrBifilt3.hw_ynrT_rgeSgm_div = interpolation_f32(
        paut->dyn[ilow].loNrBifilt3.hw_ynrT_rgeSgm_div, paut->dyn[ihigh].loNrBifilt3.hw_ynrT_rgeSgm_div, ratio);
    out->dyn.loNrBifilt3.hw_ynrT_bifiltOut_alpha = interpolation_f32(
        paut->dyn[ilow].loNrBifilt3.hw_ynrT_bifiltOut_alpha, paut->dyn[ihigh].loNrBifilt3.hw_ynrT_bifiltOut_alpha, ratio);
    out->dyn.loNrBifilt5.hw_ynrT_bifilt5_en = interpolation_bool(
        paut->dyn[ilow].loNrBifilt5.hw_ynrT_bifilt5_en, paut->dyn[ihigh].loNrBifilt5.hw_ynrT_bifilt5_en, uratio);
    out->dyn.loNrBifilt5.hw_ynrT_guideImg_mode = ratio > 0.5?
            paut->dyn[ihigh].loNrBifilt5.hw_ynrT_guideImg_mode: paut->dyn[ilow].loNrBifilt5.hw_ynrT_guideImg_mode;
    out->dyn.loNrBifilt5.hw_ynrT_guideImgText_maxLimit = interpolation_f32(
        paut->dyn[ilow].loNrBifilt5.hw_ynrT_guideImgText_maxLimit, paut->dyn[ihigh].loNrBifilt5.hw_ynrT_guideImgText_maxLimit, ratio);
    out->dyn.loNrBifilt5.hw_ynrT_rgeSgm_scale = interpolation_f32(
        paut->dyn[ilow].loNrBifilt5.hw_ynrT_rgeSgm_scale, paut->dyn[ihigh].loNrBifilt5.hw_ynrT_rgeSgm_scale, ratio);
    out->dyn.loNrBifilt5.hw_ynrT_spatialSgm_scale = interpolation_f32(
        paut->dyn[ilow].loNrBifilt5.hw_ynrT_spatialSgm_scale, paut->dyn[ihigh].loNrBifilt5.hw_ynrT_spatialSgm_scale, ratio);
    out->dyn.loNrBifilt5.hw_ynrT_centerPix_wgt = interpolation_f32(
        paut->dyn[ilow].loNrBifilt5.hw_ynrT_centerPix_wgt, paut->dyn[ihigh].loNrBifilt5.hw_ynrT_centerPix_wgt, ratio);
    out->dyn.loNrBifilt5.hw_ynrT_nhoodPixWgt_thred = interpolation_f32(
        paut->dyn[ilow].loNrBifilt5.hw_ynrT_nhoodPixWgt_thred, paut->dyn[ihigh].loNrBifilt5.hw_ynrT_nhoodPixWgt_thred, ratio);
    out->dyn.loNrPost.hw_ynrT_loNROutSel_mode = ratio > 0.5?
            paut->dyn[ihigh].loNrPost.hw_ynrT_loNROutSel_mode: paut->dyn[ilow].loNrPost.hw_ynrT_loNROutSel_mode;
    out->dyn.loNrPost.hw_ynrT_softThd_val = interpolation_f32(
        paut->dyn[ilow].loNrPost.hw_ynrT_softThd_val, paut->dyn[ihigh].loNrPost.hw_ynrT_softThd_val, ratio);
    out->dyn.loNrPost.hw_ynrT_loNr_alpha = interpolation_f32(
        paut->dyn[ilow].loNrPost.hw_ynrT_loNr_alpha, paut->dyn[ihigh].loNrPost.hw_ynrT_loNr_alpha, ratio);
    for (i=0; i<17; i++) {
        out->dyn.loNrPost.hw_ynrC_luma2LoSgm_curve.idx[i] = interpolation_u16(
            paut->dyn[ilow].loNrPost.hw_ynrC_luma2LoSgm_curve.idx[i], paut->dyn[ihigh].loNrPost.hw_ynrC_luma2LoSgm_curve.idx[i], uratio);
    }
    for (i=0; i<17; i++) {
        out->dyn.loNrPost.hw_ynrC_luma2LoSgm_curve.val[i] = interpolation_u16(
            paut->dyn[ilow].loNrPost.hw_ynrC_luma2LoSgm_curve.val[i], paut->dyn[ihigh].loNrPost.hw_ynrC_luma2LoSgm_curve.val[i], uratio);
    }
    out->dyn.loNrGaus.hw_ynrT_gausFilt_en = interpolation_bool(
        paut->dyn[ilow].loNrGaus.hw_ynrT_gausFilt_en, paut->dyn[ihigh].loNrGaus.hw_ynrT_gausFilt_en, uratio);
    out->dyn.loNrGaus.sw_ynrT_gausFilt_strg = interpolation_f32(
        paut->dyn[ilow].loNrGaus.sw_ynrT_gausFilt_strg, paut->dyn[ihigh].loNrGaus.sw_ynrT_gausFilt_strg, ratio);
    out->dyn.inPixSgm.hw_ynrT_localPixSgm_scale = interpolation_f32(
        paut->dyn[ilow].inPixSgm.hw_ynrT_localPixSgm_scale, paut->dyn[ihigh].inPixSgm.hw_ynrT_localPixSgm_scale, ratio);
    out->dyn.inPixSgm.hw_ynrT_localPixSgmScl2_thred = interpolation_f32(
        paut->dyn[ilow].inPixSgm.hw_ynrT_localPixSgmScl2_thred, paut->dyn[ihigh].inPixSgm.hw_ynrT_localPixSgmScl2_thred, ratio);
    out->dyn.inPixSgm.hw_ynrT_localPixSgm_scale2 = interpolation_f32(
        paut->dyn[ilow].inPixSgm.hw_ynrT_localPixSgm_scale2, paut->dyn[ihigh].inPixSgm.hw_ynrT_localPixSgm_scale2, ratio);
    out->dyn.inPixSgm.hw_ynrT_glbPixSgm_val = interpolation_f32(
        paut->dyn[ilow].inPixSgm.hw_ynrT_glbPixSgm_val, paut->dyn[ihigh].inPixSgm.hw_ynrT_glbPixSgm_val, ratio);
    out->dyn.inPixSgm.hw_ynrT_glbPixSgm_alpha = interpolation_f32(
        paut->dyn[ilow].inPixSgm.hw_ynrT_glbPixSgm_alpha, paut->dyn[ihigh].inPixSgm.hw_ynrT_glbPixSgm_alpha, ratio);
    out->dyn.hiNr.hw_ynrT_nlmFilt_en = interpolation_bool(
        paut->dyn[ilow].hiNr.hw_ynrT_nlmFilt_en, paut->dyn[ihigh].hiNr.hw_ynrT_nlmFilt_en, uratio);
    for (i=0; i<17; i++) {
        out->dyn.hiNr.hw_ynrT_radiDist2Wgt_val[i] = interpolation_f32(
            paut->dyn[ilow].hiNr.hw_ynrT_radiDist2Wgt_val[i], paut->dyn[ihigh].hiNr.hw_ynrT_radiDist2Wgt_val[i], ratio);
    }
    out->dyn.hiNr.hw_ynrT_pixSgmRadiDistWgt_alpha = interpolation_f32(
        paut->dyn[ilow].hiNr.hw_ynrT_pixSgmRadiDistWgt_alpha, paut->dyn[ihigh].hiNr.hw_ynrT_pixSgmRadiDistWgt_alpha, ratio);
    out->dyn.hiNr.hw_ynrT_nlmSgm_minLimit = interpolation_f32(
        paut->dyn[ilow].hiNr.hw_ynrT_nlmSgm_minLimit, paut->dyn[ihigh].hiNr.hw_ynrT_nlmSgm_minLimit, ratio);
    out->dyn.hiNr.hw_ynrT_nlmSgm_scale = interpolation_f32(
        paut->dyn[ilow].hiNr.hw_ynrT_nlmSgm_scale, paut->dyn[ihigh].hiNr.hw_ynrT_nlmSgm_scale, ratio);
    out->dyn.hiNr.hw_ynrT_nlmRgeWgt_negOffset = interpolation_f32(
        paut->dyn[ilow].hiNr.hw_ynrT_nlmRgeWgt_negOffset, paut->dyn[ihigh].hiNr.hw_ynrT_nlmRgeWgt_negOffset, ratio);
    for (i=0; i<7; i++) {
        out->dyn.hiNr.hw_ynrT_nlmSpatial_wgt[i] = interpolation_u8(
            paut->dyn[ilow].hiNr.hw_ynrT_nlmSpatial_wgt[i], paut->dyn[ihigh].hiNr.hw_ynrT_nlmSpatial_wgt[i], uratio);
    }
    out->dyn.hiNr.hw_ynrT_nlmOut_alpha = interpolation_f32(
        paut->dyn[ilow].hiNr.hw_ynrT_nlmOut_alpha, paut->dyn[ihigh].hiNr.hw_ynrT_nlmOut_alpha, ratio);
}
    return XCAM_RETURN_NO_ERROR;
}
#endif

#if RKAIQ_HAVE_YNR_V24
XCamReturn YnrSelectParam
(
    ynr_param_auto_t *pAuto,
    ynr_param_t* out,
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
    ynr_param_auto_t *paut = pAuto;

    pre_interp(iso, NULL, 0, ilow, ihigh, ratio);
    uratio = ratio * (1 << RATIO_FIXBIT);

    if (ratio > 0.5)
        inear = ihigh;
    else
        inear = ilow;

    for (i=0; i<17; i++) {
        out->dyn.ynrScl_radi.hw_ynrT_radiDist2YnrScl_val[i] = interpolation_f32(
            paut->dyn[ilow].ynrScl_radi.hw_ynrT_radiDist2YnrScl_val[i], paut->dyn[ihigh].ynrScl_radi.hw_ynrT_radiDist2YnrScl_val[i], ratio);
    }
    out->dyn.ynrScl_locSgmStrg.hw_ynrT_glbSgmStrg_val = interpolation_f32(
        paut->dyn[ilow].ynrScl_locSgmStrg.hw_ynrT_glbSgmStrg_val, paut->dyn[ihigh].ynrScl_locSgmStrg.hw_ynrT_glbSgmStrg_val, ratio);
    out->dyn.ynrScl_locSgmStrg.hw_ynrT_glbSgmStrg_alpha = interpolation_f32(
        paut->dyn[ilow].ynrScl_locSgmStrg.hw_ynrT_glbSgmStrg_alpha, paut->dyn[ihigh].ynrScl_locSgmStrg.hw_ynrT_glbSgmStrg_alpha, ratio);
    for (i=0; i<9; i++) {
        out->dyn.ynrScl_locSgmStrg.hw_ynrT_locSgmStrg2YnrScl_val[i] = interpolation_f32(
            paut->dyn[ilow].ynrScl_locSgmStrg.hw_ynrT_locSgmStrg2YnrScl_val[i], paut->dyn[ihigh].ynrScl_locSgmStrg.hw_ynrT_locSgmStrg2YnrScl_val[i], ratio);
    }
    for (i=0; i<5; i++) {
        out->dyn.coeff2SgmCurve.sigma_coeff[i] = interpolation_f32(
            paut->dyn[ilow].coeff2SgmCurve.sigma_coeff[i], paut->dyn[ihigh].coeff2SgmCurve.sigma_coeff[i], ratio);
    }
    out->dyn.coeff2SgmCurve.lowFreqCoeff = interpolation_f32(
        paut->dyn[ilow].coeff2SgmCurve.lowFreqCoeff, paut->dyn[ihigh].coeff2SgmCurve.lowFreqCoeff, ratio);
    for (i=0; i<17; i++) {
        out->dyn.hw_ynrC_luma2Sigma_curve.idx[i] = interpolation_u16(
            paut->dyn[ilow].hw_ynrC_luma2Sigma_curve.idx[i], paut->dyn[ihigh].hw_ynrC_luma2Sigma_curve.idx[i], uratio);
    }
    for (i=0; i<17; i++) {
        out->dyn.hw_ynrC_luma2Sigma_curve.val[i] = interpolation_u16(
            paut->dyn[ilow].hw_ynrC_luma2Sigma_curve.val[i], paut->dyn[ihigh].hw_ynrC_luma2Sigma_curve.val[i], uratio);
    }
    out->dyn.hw_ynrT_loNr_en = paut->dyn[inear].hw_ynrT_loNr_en;
    out->dyn.loNr_preProc.sw_ynrT_preLpfCfg_mode = paut->dyn[inear].loNr_preProc.sw_ynrT_preLpfCfg_mode;
    for (i=0; i<3; i++) {
        out->dyn.loNr_preProc.hw_ynrT_preLpfSpatial_wgt[i] = interpolation_u8(
            paut->dyn[ilow].loNr_preProc.hw_ynrT_preLpfSpatial_wgt[i], paut->dyn[ihigh].loNr_preProc.hw_ynrT_preLpfSpatial_wgt[i], uratio);
    }
    out->dyn.loNr_preProc.sw_ynrT_preLpf_strg = interpolation_f32(
        paut->dyn[ilow].loNr_preProc.sw_ynrT_preLpf_strg, paut->dyn[ihigh].loNr_preProc.sw_ynrT_preLpf_strg, ratio);
    out->dyn.loNr_preProc.sw_ynrT_edgeDctConf_scale = interpolation_f32(
        paut->dyn[ilow].loNr_preProc.sw_ynrT_edgeDctConf_scale, paut->dyn[ihigh].loNr_preProc.sw_ynrT_edgeDctConf_scale, ratio);
    out->dyn.loNr_iirGuide.hw_ynrT_localYnrScl_alpha = interpolation_f32(
        paut->dyn[ilow].loNr_iirGuide.hw_ynrT_localYnrScl_alpha, paut->dyn[ihigh].loNr_iirGuide.hw_ynrT_localYnrScl_alpha, ratio);
    out->dyn.loNr_iirGuide.hw_ynrT_iiFilt_strg = interpolation_f32(
        paut->dyn[ilow].loNr_iirGuide.hw_ynrT_iiFilt_strg, paut->dyn[ihigh].loNr_iirGuide.hw_ynrT_iiFilt_strg, ratio);
    out->dyn.loNr_iirGuide.hw_ynrT_pixDiffEge_thred = interpolation_f32(
        paut->dyn[ilow].loNr_iirGuide.hw_ynrT_pixDiffEge_thred, paut->dyn[ihigh].loNr_iirGuide.hw_ynrT_pixDiffEge_thred, ratio);
    out->dyn.loNr_iirGuide.hw_ynrT_centerPix_wgt = interpolation_f32(
        paut->dyn[ilow].loNr_iirGuide.hw_ynrT_centerPix_wgt, paut->dyn[ihigh].loNr_iirGuide.hw_ynrT_centerPix_wgt, ratio);
    out->dyn.loNr_iirGuide.hw_ynrT_iirInitWgt_scale = interpolation_f32(
        paut->dyn[ilow].loNr_iirGuide.hw_ynrT_iirInitWgt_scale, paut->dyn[ihigh].loNr_iirGuide.hw_ynrT_iirInitWgt_scale, ratio);
    out->dyn.loNr_iirGuide.hw_ynrT_softThd_scale = interpolation_f32(
        paut->dyn[ilow].loNr_iirGuide.hw_ynrT_softThd_scale, paut->dyn[ihigh].loNr_iirGuide.hw_ynrT_softThd_scale, ratio);
    out->dyn.loNr_bifilt.hw_ynrT_rgeSgm_scale = interpolation_f32(
        paut->dyn[ilow].loNr_bifilt.hw_ynrT_rgeSgm_scale, paut->dyn[ihigh].loNr_bifilt.hw_ynrT_rgeSgm_scale, ratio);
    out->dyn.loNr_bifilt.hw_ynrT_filtSpatialV_strg = interpolation_f32(
        paut->dyn[ilow].loNr_bifilt.hw_ynrT_filtSpatialV_strg, paut->dyn[ihigh].loNr_bifilt.hw_ynrT_filtSpatialV_strg, ratio);
    out->dyn.loNr_bifilt.hw_ynrT_filtSpatialH_strg = interpolation_f32(
        paut->dyn[ilow].loNr_bifilt.hw_ynrT_filtSpatialH_strg, paut->dyn[ihigh].loNr_bifilt.hw_ynrT_filtSpatialH_strg, ratio);
    out->dyn.loNr_bifilt.hw_ynrT_centerPix_wgt = interpolation_f32(
        paut->dyn[ilow].loNr_bifilt.hw_ynrT_centerPix_wgt, paut->dyn[ihigh].loNr_bifilt.hw_ynrT_centerPix_wgt, ratio);
    out->dyn.loNr_bifilt.hw_ynrT_bifiltOut_alpha = interpolation_f32(
        paut->dyn[ilow].loNr_bifilt.hw_ynrT_bifiltOut_alpha, paut->dyn[ihigh].loNr_bifilt.hw_ynrT_bifiltOut_alpha, ratio);
    out->dyn.hiNr_filtProc.hw_ynrT_nlmFilt_en = paut->dyn[inear].hiNr_filtProc.hw_ynrT_nlmFilt_en;
    out->dyn.hiNr_filtProc.hw_ynrT_localYnrScl_alpha = interpolation_f32(
        paut->dyn[ilow].hiNr_filtProc.hw_ynrT_localYnrScl_alpha, paut->dyn[ihigh].hiNr_filtProc.hw_ynrT_localYnrScl_alpha, ratio);
    out->dyn.hiNr_filtProc.hw_ynrT_nlmSgm_minLimit = interpolation_f32(
        paut->dyn[ilow].hiNr_filtProc.hw_ynrT_nlmSgm_minLimit, paut->dyn[ihigh].hiNr_filtProc.hw_ynrT_nlmSgm_minLimit, ratio);
    out->dyn.hiNr_filtProc.hw_ynrT_nlmSgm_scale = interpolation_f32(
        paut->dyn[ilow].hiNr_filtProc.hw_ynrT_nlmSgm_scale, paut->dyn[ihigh].hiNr_filtProc.hw_ynrT_nlmSgm_scale, ratio);
    out->dyn.hiNr_filtProc.hw_ynrT_nlmRgeWgt_negOff = interpolation_f32(
        paut->dyn[ilow].hiNr_filtProc.hw_ynrT_nlmRgeWgt_negOff, paut->dyn[ihigh].hiNr_filtProc.hw_ynrT_nlmRgeWgt_negOff, ratio);
    out->dyn.hiNr_filtProc.hw_ynrT_centerPix_wgt = interpolation_f32(
        paut->dyn[ilow].hiNr_filtProc.hw_ynrT_centerPix_wgt, paut->dyn[ihigh].hiNr_filtProc.hw_ynrT_centerPix_wgt, ratio);
    for (i=0; i<6; i++) {
        out->dyn.hiNr_filtProc.hw_ynrT_nlmSpatial_wgt[i] = paut->dyn[inear].hiNr_filtProc.hw_ynrT_nlmSpatial_wgt[i];
    }
    out->dyn.hiNr_alphaProc.hw_ynrT_nlmOut_alpha = interpolation_f32(
        paut->dyn[ilow].hiNr_alphaProc.hw_ynrT_nlmOut_alpha, paut->dyn[ihigh].hiNr_alphaProc.hw_ynrT_nlmOut_alpha, ratio);
    out->dyn.hiNr_alphaProc.hw_ynrT_edgAlphaUp_thred = interpolation_f32(
        paut->dyn[ilow].hiNr_alphaProc.hw_ynrT_edgAlphaUp_thred, paut->dyn[ihigh].hiNr_alphaProc.hw_ynrT_edgAlphaUp_thred, ratio);
    out->dyn.hiNr_alphaProc.hw_ynrT_locSgmStrgAlphaUp_thred = interpolation_f32(
        paut->dyn[ilow].hiNr_alphaProc.hw_ynrT_locSgmStrgAlphaUp_thred, paut->dyn[ihigh].hiNr_alphaProc.hw_ynrT_locSgmStrgAlphaUp_thred, ratio);

    return XCAM_RETURN_NO_ERROR;
}

#endif

#define RKISP_ALGO_YNR_VERSION     "v0.0.9"
#define RKISP_ALGO_YNR_VENDOR      "Rockchip"
#define RKISP_ALGO_YNR_DESCRIPTION "Rockchip ynr algo for ISP32"

RkAiqAlgoDescription g_RkIspAlgoDescYnr = {
    .common = {
        .version = RKISP_ALGO_YNR_VERSION,
        .vendor  = RKISP_ALGO_YNR_VENDOR,
        .description = RKISP_ALGO_YNR_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_AYNR,
        .id      = 0,
        .create_context  = create_context,
        .destroy_context = destroy_context,
    },
    .prepare = prepare,
    .pre_process = NULL,
    .processing = processing,
    .post_process = NULL,
};

