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
#include "aiynr_types_prvt.h"
#include "xcam_log.h"

#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"
#include "interpolation.h"
#include "c_base/aiq_base.h"
#include "newStruct/algo_common.h"
#include "common/rk-aiisp-config.h"

#include "rk_info_utils.h"

static void AiynrPrintParam(AiynrContext_t *pAiynrCtx, aiynr_param_t* out, int iso)
{
    aiynr_params_static_t *sta = &out->sta;
    aiynr_param_dyn_t *dyn = &out->dyn;
    aiynr_param_calib_t *calib = &dyn->calib;
    aiynr_param_tunning_t *tunning = &dyn->tunning;

    LOGD_AIYNR("working_mode=%d,pre_iso=%d,cur_iso=%d", pAiynrCtx->working_mode, pAiynrCtx->pre_iso, iso);

    LOGD_AIYNR("gainMerge_alpha=%0.3f,globalGain_strg=%0.3f,noiseAdd_ratio=%0.3f",
               tunning->sw_aiYnrT_gainMerge_alpha, tunning->sw_aiYnrT_globalGain_strg, tunning->sw_aiYnrT_noiseAdd_ratio);

    LOGD_AIYNR("noiseAddMot_offset=%0.3f,noiseAddMot_coeff=%0.3f",
               tunning->sw_aiYnrT_noiseAddMot_offset, tunning->sw_aiYnrT_noiseAddMot_coeff);

    LOGD_AIYNR("narManual_strg=%0.3f,narManual_alpha=%0.3f,narInverse_en=%d",
               tunning->sw_aiYnrT_narManual_strg, tunning->sw_aiYnrT_narManual_alpha, tunning->sw_aiYnrT_narInverse_en);

    LOGD_AIYNR("gain_leftshift=%d,sigmaLut_scale=%0.2f,noiseAdd_limit=%0.3f",
               tunning->sw_aiYnrT_gain_leftshift, tunning->sw_aiYnrT_sigmaLut_scale, tunning->sw_aiYnrT_noiseAdd_limit);

    LOGD_AIYNR("model_dir=%s,model_file=%s,debug_mode=%d",
               sta->model_dir.sw_aiYnrCfg_model_dir, tunning->sw_aiYnrT_model_file, sta->debug.sw_aiYnrT_debug_mode);
}

XCamReturn AiynrSelectParam
(
    AiynrContext_t *pAiynrCtx,
    aiynr_param_t* out,
    int iso)
{
    aiynr_param_auto_t *paut = &pAiynrCtx->aiynr_attrib->stAuto;
    aiynr_param_dyn_t *dyn = &out->dyn;
    aiynr_param_calib_t *calib = &dyn->calib;
    aiynr_param_tunning_t *tunning = &dyn->tunning;
    int ilow = 0, ihigh = 0;
    float ratio = 0.0f;

    if (paut == NULL || out == NULL) {
        LOGE_AIYNR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    pre_interp(iso, pAiynrCtx->iso_list, AIYNR_ISO_STEP_MAX, &ilow, &ihigh, &ratio);
    LOGD_AIYNR("%s:iso=%d,ilow=%d,ihigh=%d,ratio=%0.3f", __FUNCTION__, iso, ilow, ihigh, ratio);

    tunning->sw_aiYnrT_narInverse_en = paut->dyn[ilow].tunning.sw_aiYnrT_narInverse_en;
    if (ratio < 0.5)
        strcpy(tunning->sw_aiYnrT_model_file, paut->dyn[ilow].tunning.sw_aiYnrT_model_file);
    else
        strcpy(tunning->sw_aiYnrT_model_file, paut->dyn[ihigh].tunning.sw_aiYnrT_model_file);

    for (int i = 0; i < AIYNR_SIGMA_LUT_LEN; i++) {
        calib->sw_aiYnrC_sigma_lut[i] = interpolation_f32(
                paut->dyn[ilow].calib.sw_aiYnrC_sigma_lut[i], paut->dyn[ihigh].calib.sw_aiYnrC_sigma_lut[i], ratio);
    }

    tunning->sw_aiYnrT_gainMerge_alpha = interpolation_f32(
            paut->dyn[ilow].tunning.sw_aiYnrT_gainMerge_alpha, paut->dyn[ihigh].tunning.sw_aiYnrT_gainMerge_alpha, ratio);
    tunning->sw_aiYnrT_globalGain_strg = interpolation_f32(
            paut->dyn[ilow].tunning.sw_aiYnrT_globalGain_strg, paut->dyn[ihigh].tunning.sw_aiYnrT_globalGain_strg, ratio);
    tunning->sw_aiYnrT_noiseAdd_ratio = interpolation_f32(
                                            paut->dyn[ilow].tunning.sw_aiYnrT_noiseAdd_ratio, paut->dyn[ihigh].tunning.sw_aiYnrT_noiseAdd_ratio, ratio);
    tunning->sw_aiYnrT_noiseAddMot_offset = interpolation_f32(
            paut->dyn[ilow].tunning.sw_aiYnrT_noiseAddMot_offset, paut->dyn[ihigh].tunning.sw_aiYnrT_noiseAddMot_offset, ratio);
    tunning->sw_aiYnrT_noiseAddMot_coeff = interpolation_f32(
            paut->dyn[ilow].tunning.sw_aiYnrT_noiseAddMot_coeff, paut->dyn[ihigh].tunning.sw_aiYnrT_noiseAddMot_coeff, ratio);
    tunning->sw_aiYnrT_narManual_strg = interpolation_f32(
                                            paut->dyn[ilow].tunning.sw_aiYnrT_narManual_strg, paut->dyn[ihigh].tunning.sw_aiYnrT_narManual_strg, ratio);
    tunning->sw_aiYnrT_narManual_alpha = interpolation_f32(
            paut->dyn[ilow].tunning.sw_aiYnrT_narManual_alpha, paut->dyn[ihigh].tunning.sw_aiYnrT_narManual_alpha, ratio);

    tunning->sw_aiYnrT_gain_leftshift = interpolation_f32(
                                            paut->dyn[ilow].tunning.sw_aiYnrT_gain_leftshift, paut->dyn[ihigh].tunning.sw_aiYnrT_gain_leftshift, ratio);
    tunning->sw_aiYnrT_sigmaLut_scale = interpolation_f32(
                                            paut->dyn[ilow].tunning.sw_aiYnrT_sigmaLut_scale, paut->dyn[ihigh].tunning.sw_aiYnrT_sigmaLut_scale, ratio);
    tunning->sw_aiYnrT_noiseAdd_limit = interpolation_f32(
                                            paut->dyn[ilow].tunning.sw_aiYnrT_noiseAdd_limit, paut->dyn[ihigh].tunning.sw_aiYnrT_noiseAdd_limit, ratio);

    for (int i = 0; i < AIYNR_DECOMP_CURVE_LEN; i++) {
        tunning->sw_aiYnrT_decomp_curve [i] = interpolation_f32(
                paut->dyn[ilow].tunning.sw_aiYnrT_decomp_curve[i], paut->dyn[ihigh].tunning.sw_aiYnrT_decomp_curve[i], ratio);
    }

    AiynrPrintParam(pAiynrCtx, out, iso);

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
create_context(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    CamCalibDbV2Context_t *pCalibDbV2 = cfg->calibv2;

    AiynrContext_t* ctx = aiq_mallocz(sizeof(AiynrContext_t));

    if (ctx == NULL) {
        LOGE_AIYNR("%s create context failed!", __func__);
        return XCAM_RETURN_ERROR_MEM;
    }
    ctx->isReCal_ = true;
    ctx->aiynr_attrib =
        (aiynr_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibDbV2, aiynr));

    *context = (RkAiqAlgoContext*)ctx;
    LOGV_AIYNR("%s: Aiynr (exit)\n", __FUNCTION__);

    *context = (RkAiqAlgoContext*)ctx;
    return result;
}

static XCamReturn
destroy_context(RkAiqAlgoContext *context)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    AiynrContext_t* pAiynrCtx = (AiynrContext_t*)context;
    aiq_free(pAiynrCtx);

    LOGV_AIYNR("%s: Aiynr (exit)\n", __FUNCTION__);
    return result;
}

static XCamReturn prepare(RkAiqAlgoCom* params)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    AiynrContext_t* pAiynrCtx = (AiynrContext_t *)params->ctx;

    aibnr_api_attrib_t* aibnr_attrib =
        (aibnr_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(params->u.prepare.calibv2, aibnr));
    pAiynrCtx->aibnr_en = aibnr_attrib->en;

    if (!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB)) {
        // just update calib ptr
        if (params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB_PTR) {
            pAiynrCtx->aiynr_attrib =
                (aiynr_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(params->u.prepare.calibv2, aiynr));
            pAiynrCtx->iso_list = params->u.prepare.calibv2->sensor_info->iso_list;
            return XCAM_RETURN_NO_ERROR;
        }
    }

    pAiynrCtx->working_mode = params->u.prepare.working_mode;
    pAiynrCtx->aiynr_attrib =
        (aiynr_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(params->u.prepare.calibv2, aiynr));
    pAiynrCtx->iso_list = params->u.prepare.calibv2->sensor_info->iso_list;
    pAiynrCtx->isReCal_ = true;
    pAiynrCtx->cam_id = params->cid;

    return result;
}

XCamReturn AiynrProcess(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams, int iso)
{
    AiynrContext_t* pAiynrCtx = (AiynrContext_t *)inparams->ctx;
    aiynr_api_attrib_t* aiynr_attrib = pAiynrCtx->aiynr_attrib;
    aiynr_param_t* aiynr_res = outparams->algoRes;
    pAiynrCtx->cur_frame_id = inparams->frame_id;

    if (pAiynrCtx->aibnr_en) {
        LOGD_AIYNR("AIBNR is enable, skip AIYNR!");
        return XCAM_RETURN_NO_ERROR;
    }

    if (aiynr_attrib->opMode != RK_AIQ_OP_MODE_AUTO) {
        LOGE_AIYNR("mode is %d, not auto mode, ignore", aiynr_attrib->opMode);
        return XCAM_RETURN_NO_ERROR;
    }

    if (aiynr_attrib->en == false) {
        if (inparams->u.proc.is_attrib_update) {
            outparams->cfg_update = true;
            outparams->en = aiynr_attrib->en;
            outparams->bypass = aiynr_attrib->bypass;
        }
        LOGD_AIYNR("AIYNR not enable, skip!");
        return XCAM_RETURN_NO_ERROR;
    }
    bool need_recal = pAiynrCtx->isReCal_;

    bool init = inparams->u.proc.init;
    if (inparams->u.proc.is_attrib_update || inparams->u.proc.init) {
        need_recal = true;
    }

    int delta_iso = abs(iso - pAiynrCtx->pre_iso);
    if (delta_iso > 0.01 || init) {
        pAiynrCtx->pre_iso = iso;
        need_recal = true;
    }

    LOGD_AIYNR(">>>cam_id=%d,frame_id=%d,cur_iso=%d,need_recal=%d", pAiynrCtx->cam_id, pAiynrCtx->cur_frame_id, iso, need_recal);

    outparams->cfg_update = false;
    if (need_recal) {
        aiynr_res->sta = aiynr_attrib->stAuto.sta;
        AiynrSelectParam(pAiynrCtx, aiynr_res, iso);
        pAiynrCtx->cur_param = *aiynr_res;

        outparams->cfg_update = true;
        outparams->en = aiynr_attrib->en;
        outparams->bypass = aiynr_attrib->bypass;
        LOGD_AIYNR("%s:iso=%d,en=%d,bypass=%d", __FUNCTION__, iso, outparams->en, outparams->bypass);
        pAiynrCtx->isReCal_ = false;
    }

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    int iso = inparams->u.proc.iso;
    AiynrProcess(inparams, outparams, iso);

    return XCAM_RETURN_NO_ERROR;
}

#if RKAIQ_HAVE_DUMPSYS
static int dump(const RkAiqAlgoCom* self, st_string* result)
{
    AiynrContext_t* pAiynrCtx = (AiynrContext_t *)self->ctx;
    aiynr_api_attrib_t* aiynr_attrib = pAiynrCtx->aiynr_attrib;
    aiynr_param_t *cur_param = &pAiynrCtx->cur_param;
    aiynr_params_static_t *sta = &cur_param->sta;
    aiynr_param_dyn_t *dyn = &cur_param->dyn;
    aiynr_param_calib_t *calib = &dyn->calib;
    aiynr_param_tunning_t *tunning = &dyn->tunning;
    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "aiynr status");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "Cam%d FrmId=%d working mode=%d iso=%d",
             self->cid, self->frame_id, pAiynrCtx->working_mode, pAiynrCtx->pre_iso);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "sw_aiYnrT_gainMerge_alpha=%f",
             tunning->sw_aiYnrT_gainMerge_alpha);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "sw_aiYnrT_globalGain_strg=%f",
             tunning->sw_aiYnrT_globalGain_strg);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "sw_aiYnrT_noiseAdd_ratio=%f",
             tunning->sw_aiYnrT_noiseAdd_ratio);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "sw_aiYnrT_noiseAddMot_offset=%f",
             tunning->sw_aiYnrT_noiseAddMot_offset);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "sw_aiYnrT_noiseAddMot_coeff=%f",
             tunning->sw_aiYnrT_noiseAddMot_coeff);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "sw_aiYnrT_narManual_strg=%f",
             tunning->sw_aiYnrT_narManual_strg);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "sw_aiYnrT_narManual_alpha=%f",
             tunning->sw_aiYnrT_narManual_alpha);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "sw_aiYnrT_narInverse_en=%d",
             tunning->sw_aiYnrT_narInverse_en);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "sw_aiYnrT_gain_leftshift=%d",
             tunning->sw_aiYnrT_gain_leftshift);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "sw_aiYnrT_sigmaLut_scale=%f",
             tunning->sw_aiYnrT_sigmaLut_scale);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "sw_aiYnrT_noiseAdd_limit=%f",
             tunning->sw_aiYnrT_noiseAdd_limit);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "sw_aiYnrCfg_model_dir=%s",
             sta->model_dir.sw_aiYnrCfg_model_dir);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "sw_aiYnrT_model_file=%s",
             tunning->sw_aiYnrT_model_file);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "sw_aiYnrT_debug_mode=%d",
             sta->debug.sw_aiYnrT_debug_mode);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    return 0;
}
#endif

#define RKISP_ALGO_AIYNR_VERSION     "v0.0.1"
#define RKISP_ALGO_AIYNR_VENDOR      "Rockchip"
#define RKISP_ALGO_AIYNR_DESCRIPTION "Rockchip aiynr algo for ISP35"

RkAiqAlgoDescription g_RkIspAlgoDescAiynr = {
    .common = {
        .version = RKISP_ALGO_AIYNR_VERSION,
        .vendor  = RKISP_ALGO_AIYNR_VENDOR,
        .description = RKISP_ALGO_AIYNR_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_AIYNR,
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

