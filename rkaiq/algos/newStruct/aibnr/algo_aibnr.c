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
#include "rk_aiq_types_priv_c.h"

#include "algo_types_priv.h"
#include "aibnr_types_prvt.h"
#include "xcam_log.h"

#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"
#include "interpolation.h"
#include "c_base/aiq_base.h"
#include "newStruct/algo_common.h"
#include "common/rk-aiisp-config.h"

#include "rk_info_utils.h"

static void printParam(AibnrContext_t *pAibnrCtx, aibnr_param_t* out, int iso)
{
    aibnr_params_static_t *sta = &out->sta;
    aibnr_param_dyn_t *dyn = &out->dyn;
    aibnr_param_calib_t *calib = &dyn->calib;
    aibnr_param_tunning_t *tunning = &dyn->tunning;

    LOGD_AIBNR("algo_aibnr: working mode=%d iso=%d doAiisp_en=%d doAiisp_isoL=%d doAiisp_isoH=%d",
              pAibnrCtx->working_mode, pAibnrCtx->pre_iso, pAibnrCtx->doAiisp_en, pAibnrCtx->doAiisp_isoL, pAibnrCtx->doAiisp_isoH);

    LOGD_AIBNR("sw_aiBnrC_blkPrev_offset=%f sw_aiBnrC_blkPost_offset=%f sw_aiBnrC_shotNoise_val=%f sw_aiBnrC_readNoise_val=%f sw_aiBnrC_fixedNoise_val=%f",
              calib->sw_aiBnrC_blkPrev_offset, calib->sw_aiBnrC_blkPost_offset, calib->sw_aiBnrC_shotNoise_val,
              calib->sw_aiBnrC_readNoise_val, calib->sw_aiBnrC_fixedNoise_val);

    LOGD_AIBNR("sw_aiBnrT_totalNr_strg=%f sw_aiBnrT_shotNr_strg=%f sw_aiBnrT_readNr_strg=%f sw_aiBnrT_fixedNr_strg=%f",
              tunning->sw_aiBnrT_totalNr_strg, tunning->sw_aiBnrT_shotNr_strg,
              tunning->sw_aiBnrT_readNr_strg, tunning->sw_aiBnrT_fixedNr_strg);

    LOGD_AIBNR("sw_aiBnrT_motionBias_offset=%f sw_aiBnrT_motionGain_strg=%f sw_aiBnrT_noiseAdd_ratio=%f sw_aiBnrT_noiseAdd_limit=%f",
              tunning->sw_aiBnrT_motionBias_offset, tunning->sw_aiBnrT_motionGain_strg,
              tunning->sw_aiBnrT_noiseAdd_ratio, tunning->sw_aiBnrT_noiseAdd_limit);

    LOGD_AIBNR("sw_aiBnrT_noiseAddMot_offset=%f sw_aiBnrT_noiseAddMot_coeff=%f sw_aiBnrT_noiseAddLuma_offset=%f sw_aiBnrT_noiseAddLuma_coeff=%f",
              tunning->sw_aiBnrT_noiseAddMot_offset, tunning->sw_aiBnrT_noiseAddMot_coeff,
              tunning->sw_aiBnrT_noiseAddLuma_offset, tunning->sw_aiBnrT_noiseAddLuma_coeff);

    LOGD_AIBNR("sw_aiBnrT_noiseAddLumaClip_th=%f sw_aiBnrT_noiseAddLumaStatic_th=%f sw_aiBnrT_nonLinear_scale=%f sw_aiBnrT_nonLinear_ratio=%f sw_aiBnrT_nonLinear_adjust=%f",
              tunning->sw_aiBnrT_noiseAddLumaClip_th, tunning->sw_aiBnrT_noiseAddLumaStatic_th,
              tunning->sw_aiBnrT_nonLinear_scale, tunning->sw_aiBnrT_nonLinear_ratio, tunning->sw_aiBnrT_nonLinear_adjust);

    for (int i = 0; i < AIBNR_VALURENR_STRG_LEN; i++) {
        LOGD_AIBNR("sw_aiBnrT_bnrValueNr_strg[%d]=%f",
              i, tunning->sw_aiBnrT_bnrValueNr_strg[i]);
    }

    LOGD_AIBNR("sw_aiBnrCfg_model_dir=%s sw_aiBnrT_model_file=%s",
              sta->model_dir.sw_aiBnrCfg_model_dir, tunning->sw_aiBnrT_model_file);

    LOGD_AIBNR("sw_aiBnrT_debug_mode=%d sw_aiBnrT_swOnIsoIdx_thred=%d sw_aiBnrT_autoSwGap_thred=%d sw_aiBnrT_manualBnrHw_en=%d",
              sta->debug.sw_aiBnrT_debug_mode, sta->swOn_cfg.sw_aiBnrT_autoSwOn_thred, sta->swOn_cfg.sw_aiBnrT_autoSwGap_thred, sta->swOn_cfg.sw_aiBnrT_manualBnrHw_en);
}

XCamReturn AibnrSelectParam
(
    AibnrContext_t *pAibnrCtx,
    aibnr_param_t* out,
    int iso)
{
    aibnr_param_auto_t *paut = &pAibnrCtx->aibnr_attrib->stAuto;
    aibnr_param_dyn_t *dyn = &out->dyn;
    aibnr_param_calib_t *calib = &dyn->calib;
    aibnr_param_tunning_t *tunning = &dyn->tunning;
    uint32_t *iso_list;
    int i, ilow = 0, ihigh = 0;
    float ratio = 0.0f;
    int doAiispIsoLIdx;
    int doAiispIsoHIdx;
    int doAiisp_isoL;
    int doAiisp_isoH;

    if (paut == NULL || out == NULL) {
        LOGE_AIBNR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    doAiispIsoHIdx = paut->sta.swOn_cfg.sw_aiBnrT_autoSwOn_thred;
    doAiispIsoLIdx = doAiispIsoHIdx;
    iso_list = pAibnrCtx->iso_list;
    if (doAiispIsoHIdx == aiBnr_swOnAt_isoIdx0) {
        doAiisp_isoL = iso_list[0];
        doAiisp_isoH = iso_list[0];
    } else if (doAiispIsoHIdx == aiBnr_swOnAt_isoIdxMax) {
        doAiisp_isoL = 0x7FFFFFFF;
        doAiisp_isoH = 0x7FFFFFFF;
    } else {
        doAiispIsoLIdx = doAiispIsoHIdx - (paut->sta.swOn_cfg.sw_aiBnrT_autoSwGap_thred + 1);
        if (doAiispIsoLIdx < 0)
            doAiispIsoLIdx = 0;
        doAiisp_isoH = iso_list[doAiispIsoHIdx];
        doAiisp_isoL = iso_list[doAiispIsoLIdx];
    }

    if (pAibnrCtx->doAiisp_isoL != doAiisp_isoL || pAibnrCtx->doAiisp_isoH != doAiisp_isoH) {
        pAibnrCtx->doAiisp_force_update = true;
        pAibnrCtx->doAiisp_isoL = doAiisp_isoL;
        pAibnrCtx->doAiisp_isoH = doAiisp_isoH;
    } else {
        pAibnrCtx->doAiisp_force_update = false;
    }

    pre_interp(iso, pAibnrCtx->iso_list, AIBNR_ISO_STEP_MAX, &ilow, &ihigh, &ratio);
    if (pAibnrCtx->doAiisp_isoL != pAibnrCtx->doAiisp_isoH) {
        if ((iso >= pAibnrCtx->doAiisp_isoH) && !pAibnrCtx->doAiisp_en) {
            pAibnrCtx->doAiisp_en = true;
            LOGK_AIBNR("switch to aiisp mode, iso %d", iso);
        } else if ((iso <= pAibnrCtx->doAiisp_isoL) && pAibnrCtx->doAiisp_en) {
            pAibnrCtx->doAiisp_en = false;
            LOGK_AIBNR("switch to traditional mode, iso %d", iso);
        }

        if (iso > pAibnrCtx->doAiisp_isoL && iso < pAibnrCtx->doAiisp_isoH) {
            pAibnrCtx->fixIndex = pAibnrCtx->doAiisp_en ? doAiispIsoHIdx : doAiispIsoLIdx;
        } else {
            pAibnrCtx->fixIndex = -1;
        }
    } else {
        if ((pAibnrCtx->doAiisp_isoH == iso_list[0]) && !pAibnrCtx->doAiisp_en) {
            pAibnrCtx->doAiisp_en = true;
            pAibnrCtx->fixIndex = -1;
            LOGK_AIBNR("switch to aiisp mode, iso %d", iso);
        } else if ((pAibnrCtx->doAiisp_isoH == 0x7FFFFFFF) && pAibnrCtx->doAiisp_en) {
            pAibnrCtx->doAiisp_en = false;
            pAibnrCtx->fixIndex = -1;
            LOGK_AIBNR("switch to traditional mode, iso %d", iso);
        }
    }
    LOGI_AIBNR("%s iso %d, ilow %d, ihigh %d, ratio %f, fixIndex %d, doAiisp_isoL %d, doAiisp_isoH %d, doAiisp_force_update %d, sw_aiBnrT_autoSwOn_thred %d, sw_aiBnrT_autoSwGap_thred %d",
        __func__, iso, ilow, ihigh, ratio, pAibnrCtx->fixIndex,
        pAibnrCtx->doAiisp_isoL, pAibnrCtx->doAiisp_isoH, pAibnrCtx->doAiisp_force_update,
        paut->sta.swOn_cfg.sw_aiBnrT_autoSwOn_thred, paut->sta.swOn_cfg.sw_aiBnrT_autoSwGap_thred);

    out->sta.swOn_cfg.sw_aiBnrT_manualBnrHw_en = pAibnrCtx->doAiisp_en;
    if (ratio < 0.5)
        strcpy(tunning->sw_aiBnrT_model_file, paut->dyn[ilow].tunning.sw_aiBnrT_model_file);
    else
        strcpy(tunning->sw_aiBnrT_model_file, paut->dyn[ihigh].tunning.sw_aiBnrT_model_file);

    calib->sw_aiBnrC_blkPrev_offset = interpolation_f32(
                    paut->dyn[ilow].calib.sw_aiBnrC_blkPrev_offset, paut->dyn[ihigh].calib.sw_aiBnrC_blkPrev_offset, ratio);
    calib->sw_aiBnrC_blkPost_offset = interpolation_f32(
                    paut->dyn[ilow].calib.sw_aiBnrC_blkPost_offset, paut->dyn[ihigh].calib.sw_aiBnrC_blkPost_offset, ratio);
    calib->sw_aiBnrC_shotNoise_val = interpolation_f32(
                    paut->dyn[ilow].calib.sw_aiBnrC_shotNoise_val, paut->dyn[ihigh].calib.sw_aiBnrC_shotNoise_val, ratio);
    calib->sw_aiBnrC_readNoise_val = interpolation_f32(
                    paut->dyn[ilow].calib.sw_aiBnrC_readNoise_val, paut->dyn[ihigh].calib.sw_aiBnrC_readNoise_val, ratio);
    calib->sw_aiBnrC_fixedNoise_val = interpolation_f32(
                    paut->dyn[ilow].calib.sw_aiBnrC_fixedNoise_val, paut->dyn[ihigh].calib.sw_aiBnrC_fixedNoise_val, ratio);
    tunning->sw_aiBnrT_totalNr_strg = interpolation_f32(
                    paut->dyn[ilow].tunning.sw_aiBnrT_totalNr_strg, paut->dyn[ihigh].tunning.sw_aiBnrT_totalNr_strg, ratio);
    tunning->sw_aiBnrT_shotNr_strg = interpolation_f32(
                    paut->dyn[ilow].tunning.sw_aiBnrT_shotNr_strg, paut->dyn[ihigh].tunning.sw_aiBnrT_shotNr_strg, ratio);
    tunning->sw_aiBnrT_readNr_strg = interpolation_f32(
                    paut->dyn[ilow].tunning.sw_aiBnrT_readNr_strg, paut->dyn[ihigh].tunning.sw_aiBnrT_readNr_strg, ratio);
    tunning->sw_aiBnrT_fixedNr_strg = interpolation_f32(
                    paut->dyn[ilow].tunning.sw_aiBnrT_fixedNr_strg, paut->dyn[ihigh].tunning.sw_aiBnrT_fixedNr_strg, ratio);

    tunning->sw_aiBnrT_motionBias_offset = interpolation_f32(
                    paut->dyn[ilow].tunning.sw_aiBnrT_motionBias_offset, paut->dyn[ihigh].tunning.sw_aiBnrT_motionBias_offset, ratio);
    tunning->sw_aiBnrT_motionGain_strg = interpolation_f32(
                    paut->dyn[ilow].tunning.sw_aiBnrT_motionGain_strg, paut->dyn[ihigh].tunning.sw_aiBnrT_motionGain_strg, ratio);
    tunning->sw_aiBnrT_noiseAdd_ratio = interpolation_f32(
                    paut->dyn[ilow].tunning.sw_aiBnrT_noiseAdd_ratio, paut->dyn[ihigh].tunning.sw_aiBnrT_noiseAdd_ratio, ratio);
    tunning->sw_aiBnrT_noiseAdd_limit = interpolation_f32(
                    paut->dyn[ilow].tunning.sw_aiBnrT_noiseAdd_limit, paut->dyn[ihigh].tunning.sw_aiBnrT_noiseAdd_limit, ratio);
    tunning->sw_aiBnrT_noiseAddMot_offset = interpolation_f32(
                    paut->dyn[ilow].tunning.sw_aiBnrT_noiseAddMot_offset, paut->dyn[ihigh].tunning.sw_aiBnrT_noiseAddMot_offset, ratio);
    tunning->sw_aiBnrT_noiseAddMot_coeff = interpolation_f32(
                    paut->dyn[ilow].tunning.sw_aiBnrT_noiseAddMot_coeff, paut->dyn[ihigh].tunning.sw_aiBnrT_noiseAddMot_coeff, ratio);
    tunning->sw_aiBnrT_noiseAddLuma_offset = interpolation_f32(
                    paut->dyn[ilow].tunning.sw_aiBnrT_noiseAddLuma_offset, paut->dyn[ihigh].tunning.sw_aiBnrT_noiseAddLuma_offset, ratio);
    tunning->sw_aiBnrT_noiseAddLuma_coeff = interpolation_f32(
                    paut->dyn[ilow].tunning.sw_aiBnrT_noiseAddLuma_coeff, paut->dyn[ihigh].tunning.sw_aiBnrT_noiseAddLuma_coeff, ratio);
    tunning->sw_aiBnrT_noiseAddLumaClip_th = interpolation_f32(
                    paut->dyn[ilow].tunning.sw_aiBnrT_noiseAddLumaClip_th, paut->dyn[ihigh].tunning.sw_aiBnrT_noiseAddLumaClip_th, ratio);
    tunning->sw_aiBnrT_noiseAddLumaStatic_th = interpolation_f32(
                    paut->dyn[ilow].tunning.sw_aiBnrT_noiseAddLumaStatic_th, paut->dyn[ihigh].tunning.sw_aiBnrT_noiseAddLumaStatic_th, ratio);
    tunning->sw_aiBnrT_nonLinear_scale = interpolation_f32(
                    paut->dyn[ilow].tunning.sw_aiBnrT_nonLinear_scale, paut->dyn[ihigh].tunning.sw_aiBnrT_nonLinear_scale, ratio);
    tunning->sw_aiBnrT_nonLinear_ratio = interpolation_f32(
                    paut->dyn[ilow].tunning.sw_aiBnrT_nonLinear_ratio, paut->dyn[ihigh].tunning.sw_aiBnrT_nonLinear_ratio, ratio);
    tunning->sw_aiBnrT_nonLinear_adjust = interpolation_f32(
                    paut->dyn[ilow].tunning.sw_aiBnrT_nonLinear_adjust, paut->dyn[ihigh].tunning.sw_aiBnrT_nonLinear_adjust, ratio);

    for (i = 0; i < AIBNR_VALURENR_STRG_LEN; i++) {
        tunning->sw_aiBnrT_bnrValueNr_strg[i] = interpolation_f32(
                        paut->dyn[ilow].tunning.sw_aiBnrT_bnrValueNr_strg[i], paut->dyn[ihigh].tunning.sw_aiBnrT_bnrValueNr_strg[i], ratio);
    }

    printParam(pAibnrCtx, out, iso);
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
create_context(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    CamCalibDbV2Context_t *pCalibDbV2 = cfg->calibv2;

    AibnrContext_t* ctx = aiq_mallocz(sizeof(AibnrContext_t));

    if (ctx == NULL) {
        LOGE_AIBNR("%s create context failed!", __func__);
        return XCAM_RETURN_ERROR_MEM;
    }
    ctx->isReCal_ = true;
    ctx->aibnr_attrib =
        (aibnr_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibDbV2, aibnr));

    *context = (RkAiqAlgoContext* )ctx;
    LOGV_AIBNR("%s: Aibnr (exit)\n", __FUNCTION__ );

    *context = (RkAiqAlgoContext*)ctx;
    return result;
}

static XCamReturn
destroy_context(RkAiqAlgoContext *context)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    AibnrContext_t* pAibnrCtx = (AibnrContext_t*)context;
    aiq_free(pAibnrCtx);
    return result;
}

static XCamReturn prepare(RkAiqAlgoCom* params)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    AibnrContext_t* pAibnrCtx = (AibnrContext_t *)params->ctx;

    if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )) {
        // just update calib ptr
        if (params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB_PTR) {
            pAibnrCtx->aibnr_attrib =
                (aibnr_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(params->u.prepare.calibv2, aibnr));
            pAibnrCtx->iso_list = params->u.prepare.calibv2->sensor_info->iso_list;
            return XCAM_RETURN_NO_ERROR;
        }
    }

    pAibnrCtx->working_mode = params->u.prepare.working_mode;
    pAibnrCtx->aibnr_attrib =
        (aibnr_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(params->u.prepare.calibv2, aibnr));
    pAibnrCtx->iso_list = params->u.prepare.calibv2->sensor_info->iso_list;
    pAibnrCtx->isReCal_ = true;
    pAibnrCtx->doAiisp_isoL = 50;
    pAibnrCtx->doAiisp_isoH = 50;
    pAibnrCtx->doAiisp_force_update = true;
    pAibnrCtx->doAiisp_en = false;

    return result;
}

XCamReturn Aibnr_processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams, int iso)
{
    AibnrContext_t* pAibnrCtx = (AibnrContext_t *)inparams->ctx;
    aibnr_api_attrib_t* aibnr_attrib = pAibnrCtx->aibnr_attrib;
    aibnr_param_t* aibnr_res = outparams->algoRes;

    if (aibnr_attrib->opMode != RK_AIQ_OP_MODE_AUTO) {
        LOGE_AIBNR("mode is %d, not auto mode, ignore", aibnr_attrib->opMode);
        pAibnrCtx->fixIndex = -1;
        pAibnrCtx->doAiisp_force_update = false;
        return XCAM_RETURN_NO_ERROR;
    }

    if (aibnr_attrib->en == false) {
        if (inparams->u.proc.is_attrib_update) {
            outparams->cfg_update = true;
            outparams->en = aibnr_attrib->en;
            outparams->bypass = aibnr_attrib->bypass;
        }
        pAibnrCtx->fixIndex = -1;
        pAibnrCtx->doAiisp_force_update = false;
        LOGD_AIBNR("AIBNR not enable, skip!");
        return XCAM_RETURN_NO_ERROR;
    }
    bool need_recal = pAibnrCtx->isReCal_;

    bool init = inparams->u.proc.init;
    if (inparams->u.proc.is_attrib_update || inparams->u.proc.init) {
        need_recal = true;
    }

    int delta_iso = abs(iso - pAibnrCtx->pre_iso);
    if (delta_iso > 0.01 || init) {
        pAibnrCtx->pre_iso = iso;
        need_recal = true;
    }

    outparams->cfg_update = false;
    if (need_recal) {
        aibnr_res->sta = aibnr_attrib->stAuto.sta;
        AibnrSelectParam(pAibnrCtx, aibnr_res, iso);
        pAibnrCtx->cur_param = *aibnr_res;

        outparams->cfg_update = true;
        outparams->en = aibnr_attrib->en;
        outparams->bypass = aibnr_attrib->bypass;
        LOGD_AIBNR("AIBNR processing: iso %d, aibnr en:%d, bypass:%d", iso, outparams->en, outparams->bypass);

        pAibnrCtx->isReCal_ = false;
    } else {
        pAibnrCtx->doAiisp_force_update = false;
    }

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    int iso = inparams->u.proc.iso;
    Aibnr_processing(inparams, outparams, iso);

    LOGV_AIBNR("%s: Aibnr (exit)\n", __FUNCTION__ );
    return XCAM_RETURN_NO_ERROR;
}

int aibnr_getFixIndex(const RkAiqAlgoCom* inparams, int *index, bool *force_update)
{
    AibnrContext_t* pAibnrCtx = (AibnrContext_t *)inparams->ctx;

    *index = pAibnrCtx->fixIndex;
    *force_update = pAibnrCtx->doAiisp_force_update;
    return 0;
}

#if RKAIQ_HAVE_DUMPSYS
static int dump(const RkAiqAlgoCom* self, st_string* result)
{
    AibnrContext_t* pAibnrCtx = (AibnrContext_t *)self->ctx;
    aibnr_api_attrib_t* aibnr_attrib = pAibnrCtx->aibnr_attrib;
    aibnr_param_t *cur_param = &pAibnrCtx->cur_param;
    aibnr_params_static_t *sta = &cur_param->sta;
    aibnr_param_dyn_t *dyn = &cur_param->dyn;
    aibnr_param_calib_t *calib = &dyn->calib;
    aibnr_param_tunning_t *tunning = &dyn->tunning;
    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "aibnr status");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "Cam%d FrmId=%d working mode=%d iso=%d doAiisp_en=%d doAiisp_isoL=%d doAiisp_isoH=%d",
             self->cid, self->frame_id, pAibnrCtx->working_mode, pAibnrCtx->pre_iso,
             pAibnrCtx->doAiisp_en, pAibnrCtx->doAiisp_isoL, pAibnrCtx->doAiisp_isoH);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "sw_aiBnrC_blkPrev_offset=%f",
            calib->sw_aiBnrC_blkPrev_offset);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "sw_aiBnrC_blkPost_offset=%f",
            calib->sw_aiBnrC_blkPost_offset);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "sw_aiBnrC_shotNoise_val=%f",
            calib->sw_aiBnrC_shotNoise_val);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "sw_aiBnrC_readNoise_val=%f",
            calib->sw_aiBnrC_readNoise_val);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "sw_aiBnrC_fixedNoise_val=%f",
            calib->sw_aiBnrC_fixedNoise_val);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "sw_aiBnrT_totalNr_strg=%f",
            tunning->sw_aiBnrT_totalNr_strg);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "sw_aiBnrT_shotNr_strg=%f",
            tunning->sw_aiBnrT_shotNr_strg);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "sw_aiBnrT_readNr_strg=%f",
            tunning->sw_aiBnrT_readNr_strg);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "sw_aiBnrT_fixedNr_strg=%f",
            tunning->sw_aiBnrT_fixedNr_strg);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "sw_aiBnrT_motionBias_offset=%f",
            tunning->sw_aiBnrT_motionBias_offset);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "sw_aiBnrT_motionGain_strg=%f",
            tunning->sw_aiBnrT_motionGain_strg);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "sw_aiBnrT_noiseAdd_ratio=%f",
            tunning->sw_aiBnrT_noiseAdd_ratio);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "sw_aiBnrT_noiseAdd_limit=%f",
            tunning->sw_aiBnrT_noiseAdd_limit);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "sw_aiBnrT_noiseAddMot_offset=%f",
            tunning->sw_aiBnrT_noiseAddMot_offset);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "sw_aiBnrT_noiseAddMot_coeff=%f",
            tunning->sw_aiBnrT_noiseAddMot_coeff);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "sw_aiBnrT_noiseAddLuma_offset=%f",
            tunning->sw_aiBnrT_noiseAddLuma_offset);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "sw_aiBnrT_noiseAddLuma_coeff=%f",
            tunning->sw_aiBnrT_noiseAddLuma_coeff);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "sw_aiBnrT_noiseAddLumaClip_th=%f",
            tunning->sw_aiBnrT_noiseAddLumaClip_th);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "sw_aiBnrT_noiseAddLumaStatic_th=%f",
            tunning->sw_aiBnrT_noiseAddLumaStatic_th);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "sw_aiBnrT_nonLinear_scale=%f",
            tunning->sw_aiBnrT_nonLinear_scale);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "sw_aiBnrT_nonLinear_ratio=%f",
            tunning->sw_aiBnrT_nonLinear_ratio);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "sw_aiBnrT_nonLinear_adjust=%f",
            tunning->sw_aiBnrT_nonLinear_adjust);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    for (int i = 0; i < AIBNR_VALURENR_STRG_LEN; i++) {
        memset(buffer, 0, MAX_LINE_LENGTH);
        snprintf(buffer, MAX_LINE_LENGTH, "sw_aiBnrT_bnrValueNr_strg[%d]=%f",
            i, tunning->sw_aiBnrT_bnrValueNr_strg[i]);
        aiq_string_printf(result, buffer);
        aiq_string_printf(result, "\n");
    }

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "sw_aiBnrCfg_model_dir=%s",
            sta->model_dir.sw_aiBnrCfg_model_dir);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "sw_aiBnrT_model_file=%s",
            tunning->sw_aiBnrT_model_file);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "sw_aiBnrT_debug_mode=%d",
            sta->debug.sw_aiBnrT_debug_mode);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "sw_aiBnrT_autoSwOn_thred=%d",
            sta->swOn_cfg.sw_aiBnrT_autoSwOn_thred);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "sw_aiBnrT_autoSwGap_thred=%d",
            sta->swOn_cfg.sw_aiBnrT_autoSwGap_thred);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "sw_aiBnrT_manualBnrHw_en=%d",
            sta->swOn_cfg.sw_aiBnrT_manualBnrHw_en);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    return 0;
}
#endif

#define RKISP_ALGO_AIBNR_VERSION     "v0.0.9"
#define RKISP_ALGO_AIBNR_VENDOR      "Rockchip"
#define RKISP_ALGO_AIBNR_DESCRIPTION "Rockchip aibnr algo for ISP35"

RkAiqAlgoDescription g_RkIspAlgoDescAibnr = {
    .common = {
        .version = RKISP_ALGO_AIBNR_VERSION,
        .vendor  = RKISP_ALGO_AIBNR_VENDOR,
        .description = RKISP_ALGO_AIBNR_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_AIBNR,
        .id      = 0,
        .create_context  = create_context,
        .destroy_context = destroy_context,
    },
    .prepare = prepare,
    .pre_process = NULL,
    .processing = processing,
    .post_process = NULL,
#if RKAIQ_HAVE_DUMPSYS
    .dump = dump,
#endif
};

