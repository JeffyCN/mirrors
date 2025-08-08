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

#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"
#include "algo_types_priv.h"
#include "c_base/aiq_base.h"
#include "histeq_types_prvt.h"
#include "interpolation.h"
#include "xcam_log.h"

XCamReturn HisteqSelectParam(HisteqContext_t* pHisteqCtx, histeq_param_t* out, int iso) {
    LOGI_AHISTEQ("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    if (pHisteqCtx == NULL) {
        LOGE_AHISTEQ("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    int i       = 0;
    int iso_low = 0, iso_high = 0, ilow = 0, ihigh = 0, inear = 0;
    float ratio = 0.0f;
    uint16_t uratio;
    histeq_param_auto_t* paut = &pHisteqCtx->histeq_attrib->stAuto;

    pre_interp(iso, pHisteqCtx->iso_list, 13, &ilow, &ihigh, &ratio);
    uratio = ratio * (1 << RATIO_FIXBIT);

    if (ratio > 0.5)
        inear = ihigh;
    else
        inear = ilow;
#if RKAIQ_HAVE_DEHAZE_V14
    out->dyn.sw_hist_mapUserSet = interpolation_f32(paut->dyn[ilow].sw_hist_mapUserSet,
                                                    paut->dyn[ihigh].sw_hist_mapUserSet, ratio);
    out->dyn.hw_histc_noiseCount_offset =
        interpolation_u8(paut->dyn[ilow].hw_histc_noiseCount_offset,
                         paut->dyn[ihigh].hw_histc_noiseCount_offset, uratio);
    out->dyn.sw_histc_noiseCount_scale =
        interpolation_f32(paut->dyn[ilow].sw_histc_noiseCount_scale,
                          paut->dyn[ihigh].sw_histc_noiseCount_scale, ratio);
    out->dyn.sw_histc_countWgt_minLimit =
        interpolation_f32(paut->dyn[ilow].sw_histc_countWgt_minLimit,
                          paut->dyn[ihigh].sw_histc_countWgt_minLimit, ratio);
    out->dyn.sw_hist_mapCount_scale = interpolation_f32(
        paut->dyn[ilow].sw_hist_mapCount_scale, paut->dyn[ihigh].sw_hist_mapCount_scale, ratio);
    out->dyn.sw_hist_mapMerge_alpha = interpolation_f32(
        paut->dyn[ilow].sw_hist_mapMerge_alpha, paut->dyn[ihigh].sw_hist_mapMerge_alpha, ratio);
    for (i = 0; i < 17; i++) {
        out->dyn.sw_dehaze_outputMerge_alpha[i] =
            interpolation_f32(paut->dyn[ilow].sw_dehaze_outputMerge_alpha[i],
                              paut->dyn[ihigh].sw_dehaze_outputMerge_alpha[i], ratio);
    }
#endif

#if RKAIQ_HAVE_HISTEQ_V10
    out->dyn.chroma.sw_histT_saturate_scale =
        interpolation_f32(paut->dyn[ilow].chroma.sw_histT_saturate_scale,
                          paut->dyn[ihigh].chroma.sw_histT_saturate_scale, ratio);

    out->dyn.statsHist.hw_histCfg_noiseCount_offset =
        interpolation_u8(paut->dyn[ilow].statsHist.hw_histCfg_noiseCount_offset,
                         paut->dyn[ihigh].statsHist.hw_histCfg_noiseCount_offset, uratio);
    out->dyn.statsHist.sw_histCfg_noiseCount_scale =
        interpolation_f32(paut->dyn[ilow].statsHist.sw_histCfg_noiseCount_scale,
                          paut->dyn[ihigh].statsHist.sw_histCfg_noiseCount_scale, ratio);
    out->dyn.statsHist.sw_histCfg_countWgt_minLimit =
        interpolation_f32(paut->dyn[ilow].statsHist.sw_histCfg_countWgt_minLimit,
                          paut->dyn[ihigh].statsHist.sw_histCfg_countWgt_minLimit, ratio);

    out->dyn.mapHist.sw_histT_mapUsrCfg_strg =
        interpolation_f32(paut->dyn[ilow].mapHist.sw_histT_mapUsrCfg_strg,
                          paut->dyn[ihigh].mapHist.sw_histT_mapUsrCfg_strg, ratio);
    out->dyn.mapHist.sw_histT_mapStats_scale =
        interpolation_f32(paut->dyn[ilow].mapHist.sw_histT_mapStats_scale,
                          paut->dyn[ihigh].mapHist.sw_histT_mapStats_scale, ratio);
    out->dyn.mapHist.sw_histT_mapUsrCfgStrg_alpha =
        interpolation_f32(paut->dyn[ilow].mapHist.sw_histT_mapUsrCfgStrg_alpha,
                          paut->dyn[ihigh].mapHist.sw_histT_mapUsrCfgStrg_alpha, ratio);

    out->dyn.mapHistIIR.hw_histT_iirFrm_maxLimit =
        interpolation_f32(paut->dyn[ilow].mapHistIIR.hw_histT_iirFrm_maxLimit,
                          paut->dyn[ihigh].mapHistIIR.hw_histT_iirFrm_maxLimit, ratio);
    out->dyn.mapHistIIR.sw_histT_iirSgm_scale =
        interpolation_u8(paut->dyn[ilow].mapHistIIR.sw_histT_iirSgm_scale,
                         paut->dyn[ihigh].mapHistIIR.sw_histT_iirSgm_scale, uratio);

    out->dyn.mapHist_idx.hw_histT_shpOut_alpha =
        interpolation_f32(paut->dyn[ilow].mapHist_idx.hw_histT_shpOut_alpha,
                          paut->dyn[ihigh].mapHist_idx.hw_histT_shpOut_alpha, ratio);

    out->dyn.fusion.hw_histT_glbWgt_mode = paut->dyn[inear].fusion.hw_histT_glbWgt_mode;
    out->dyn.fusion.sw_histT_glbWgt_posVal =
        interpolation_f32(paut->dyn[ilow].fusion.sw_histT_glbWgt_posVal,
                          paut->dyn[ihigh].fusion.sw_histT_glbWgt_posVal, ratio);
    out->dyn.fusion.sw_histT_glbWgt_negVal =
        interpolation_f32(paut->dyn[ilow].fusion.sw_histT_glbWgt_negVal,
                          paut->dyn[ihigh].fusion.sw_histT_glbWgt_negVal, ratio);
    for (i = 0; i < 17; i++) {
        out->dyn.fusion.sw_histT_luma2Wgt_posVal[i] =
            interpolation_f32(paut->dyn[ilow].fusion.sw_histT_luma2Wgt_posVal[i],
                              paut->dyn[ihigh].fusion.sw_histT_luma2Wgt_posVal[i], ratio);
        out->dyn.fusion.sw_histT_luma2Wgt_negVal[i] =
            interpolation_f32(paut->dyn[ilow].fusion.sw_histT_luma2Wgt_negVal[i],
                              paut->dyn[ihigh].fusion.sw_histT_luma2Wgt_negVal[i], ratio);
    }
#endif
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn create_context(RkAiqAlgoContext** context, const AlgoCtxInstanceCfg* cfg) {
    XCamReturn result                 = XCAM_RETURN_NO_ERROR;
    CamCalibDbV2Context_t* pCalibDbV2 = cfg->calibv2;

    HisteqContext_t* ctx = aiq_mallocz(sizeof(HisteqContext_t));
    if (ctx == NULL) {
        LOGE_AHISTEQ("%s: create Histeq context fail!\n", __FUNCTION__);
        return XCAM_RETURN_ERROR_MEM;
    }

    ctx->isReCal_      = true;
    ctx->histeq_attrib = (histeq_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibDbV2, histEQ));

    *context = (RkAiqAlgoContext*)ctx;
    LOGV_AHISTEQ("%s: (exit)\n", __FUNCTION__);
    return result;
}

static XCamReturn destroy_context(RkAiqAlgoContext* context) {
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    HisteqContext_t* pHisteqCtx = (HisteqContext_t*)context;
    aiq_free(pHisteqCtx);
    return result;
}

static XCamReturn prepare(RkAiqAlgoCom* params) {
    XCamReturn result           = XCAM_RETURN_NO_ERROR;
    HisteqContext_t* pHisteqCtx = (HisteqContext_t*)params->ctx;

    pHisteqCtx->histeq_attrib =
        (histeq_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(params->u.prepare.calibv2, histEQ));
    pHisteqCtx->iso_list = params->u.prepare.calibv2->sensor_info->iso_list;
    pHisteqCtx->isReCal_ = true;

    return result;
}

XCamReturn Ahisteq_processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams, int iso) {
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    HisteqContext_t* pHisteqCtx        = (HisteqContext_t*)inparams->ctx;
    histeq_api_attrib_t* histeq_attrib = pHisteqCtx->histeq_attrib;
    histeq_param_t* histeqRes          = outparams->algoRes;

    LOGV_AHISTEQ("%s: (enter)\n", __FUNCTION__);

    if (!histeq_attrib) {
        LOGE_AHISTEQ("histeq_attrib is NULL !");
        return XCAM_RETURN_ERROR_MEM;
    }

    bool init     = inparams->u.proc.init;
    int delta_iso = (int)abs(iso - pHisteqCtx->iso);

    outparams->cfg_update = false;

#if 0
    if (inparams->u.proc.is_bw_sensor) {
        histeq_attrib->en     = false;
        outparams->cfg_update = init ? true : false;
        return XCAM_RETURN_NO_ERROR;
    }
#endif
    if (histeq_attrib->opMode != RK_AIQ_OP_MODE_AUTO) {
        LOGE_AHISTEQ("mode is %d, not auto mode, ignore", histeq_attrib->opMode);
        return XCAM_RETURN_NO_ERROR;
    }

    if (inparams->u.proc.is_attrib_update) {
        pHisteqCtx->isReCal_ = true;
    }

    if (delta_iso >= 1) {
        pHisteqCtx->isReCal_ = true;
    }

    if (pHisteqCtx->isReCal_) {
        histeqRes->sta = pHisteqCtx->histeq_attrib->stAuto.sta;
        HisteqSelectParam(pHisteqCtx, histeqRes, iso);

        outparams->cfg_update = true;
        outparams->en         = histeq_attrib->en;
        outparams->bypass     = histeq_attrib->bypass;
        LOGI_AHISTEQ("histeq en:%d, bypass:%d", outparams->en, outparams->bypass);
    }

    pHisteqCtx->iso      = iso;
    pHisteqCtx->isReCal_ = false;

    LOGV_AHISTEQ("%s: (exit)\n", __FUNCTION__);
    return result;
}

static XCamReturn processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams) {
    int iso = inparams->u.proc.iso;
    Ahisteq_processing(inparams, outparams, iso);

    return XCAM_RETURN_NO_ERROR;
}

#if 0
XCamReturn
algo_histeq_SetAttrib
(
    RkAiqAlgoContext* ctx,
    histeq_api_attrib_t *attr
) {
    if(ctx == NULL || attr == NULL) {
        LOGE_AHISTEQ("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    HisteqContext_t* pHisteqCtx = (HisteqContext_t*)ctx;
    pHisteqCtx->isReCal_ = true;
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
algo_histeq_GetAttrib
(
    RkAiqAlgoContext* ctx,
    histeq_api_attrib_t* attr
)
{
    if(ctx == NULL || attr == NULL) {
        LOGE_AHISTEQ("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    HisteqContext_t* pHisteqCtx = (HisteqContext_t*)ctx;
    histeq_api_attrib_t* histeq_attrib = pHisteqCtx->histeq_attrib;

    attr->opMode = histeq_attrib->opMode;
    attr->en = histeq_attrib->en;
    attr->bypass = histeq_attrib->bypass;
    memcpy(&attr->stAuto, &histeq_attrib->stAuto, sizeof(histeq_param_auto_t));

    return XCAM_RETURN_NO_ERROR;
}
#endif

#define RKISP_ALGO_HISTEQ_VERSION     "v0.1.0"
#define RKISP_ALGO_HISTEQ_VENDOR      "Rockchip"
#define RKISP_ALGO_HISTEQ_DESCRIPTION "Rockchip dehaze algo for ISP2.0"

RkAiqAlgoDescription g_RkIspAlgoDescHisteq = {
    .common =
        {
            .version         = RKISP_ALGO_HISTEQ_VERSION,
            .vendor          = RKISP_ALGO_HISTEQ_VENDOR,
            .description     = RKISP_ALGO_HISTEQ_DESCRIPTION,
            .type            = RK_AIQ_ALGO_TYPE_AHISTEQ,
            .id              = 0,
            .create_context  = create_context,
            .destroy_context = destroy_context,
        },
    .prepare      = prepare,
    .pre_process  = NULL,
    .processing   = processing,
    .post_process = NULL,
};

// RKAIQ_END_DECLARE
