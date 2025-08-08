/*
 *  Copyright (c) 2025 Rockchip Corporation
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

#include "sample_comm.h"

/*
 ******************************
 *
 * Module level API Sample Func
 *
 ******************************
 */

static int sample_aiynr_query_status(const rk_aiq_sys_ctx_t *ctx) {
#if ISP_HW_V35
    aiynr_status_t status;
    int ret = rk_aiq_user_api2_aiynr_QueryStatus(ctx, &status);

    aiynr_params_static_t *sta = &status.stMan.sta;
    aiynr_param_dyn_t *dyn = &status.stMan.dyn;
    aiynr_param_calib_t *calib = &dyn->calib;
    aiynr_param_tunning_t *tunning = &dyn->tunning;

    printf("opMode %d, en %d, bypass %d\n", status.opMode, status.en, status.bypass);

    printf("sw_aiYnrT_gainMerge_alpha=%f sw_aiYnrT_globalGain_strg=%f sw_aiYnrT_noiseAdd_ratio=%f\n",
           tunning->sw_aiYnrT_gainMerge_alpha, tunning->sw_aiYnrT_globalGain_strg,
           tunning->sw_aiYnrT_noiseAdd_ratio);

    printf("sw_aiYnrT_noiseAddMot_offset=%f sw_aiYnrT_noiseAddMot_coeff=%f\n",
           tunning->sw_aiYnrT_noiseAddMot_offset, tunning->sw_aiYnrT_noiseAddMot_coeff);

    printf("sw_aiYnrT_narManual_strg=%f sw_aiYnrT_narManual_alpha=%f sw_aiYnrT_narInverse_en=%d\n",
           tunning->sw_aiYnrT_narManual_strg, tunning->sw_aiYnrT_narManual_alpha,
           tunning->sw_aiYnrT_narInverse_en);

    printf("sw_aiYnrT_gain_leftshift=%d sw_aiYnrT_sigmaLut_scale=%f sw_aiYnrT_noiseAdd_limit=%f\n",
           tunning->sw_aiYnrT_gain_leftshift, tunning->sw_aiYnrT_sigmaLut_scale,
           tunning->sw_aiYnrT_noiseAdd_limit);

    printf("sw_aiYnrCfg_model_dir=%s sw_aiYnrT_model_file=%s\n",
           sta->model_dir.sw_aiYnrCfg_model_dir, tunning->sw_aiYnrT_model_file);

    printf("sw_aiYnrT_debug_mode=%d\n",
           sta->debug.sw_aiYnrT_debug_mode);

    return ret;
#else
    return 0;
#endif
}

static int sample_aiynr_get_attrib(const rk_aiq_sys_ctx_t *ctx) {
#if ISP_HW_V35
    aiynr_api_attrib_t attr;
    int ret = rk_aiq_user_api2_aiynr_GetAttrib(ctx, &attr);

    aiynr_params_static_t *sta;
    aiynr_param_dyn_t *dyn;
    aiynr_param_calib_t *calib;
    aiynr_param_tunning_t *tunning;

    printf("opMode %d, en %d, bypass %d\n", attr.opMode, attr.en, attr.bypass);

    dyn = &attr.stMan.dyn;
    sta = &attr.stMan.sta;
    calib = &dyn->calib;
    tunning = &dyn->tunning;

    printf("parameter for stMan:\n");

    printf("sw_aiYnrT_gainMerge_alpha=%f sw_aiYnrT_globalGain_strg=%f sw_aiYnrT_noiseAdd_ratio=%f\n",
           tunning->sw_aiYnrT_gainMerge_alpha, tunning->sw_aiYnrT_globalGain_strg,
           tunning->sw_aiYnrT_noiseAdd_ratio);

    printf("sw_aiYnrT_noiseAddMot_offset=%f sw_aiYnrT_noiseAddMot_coeff=%f\n",
           tunning->sw_aiYnrT_noiseAddMot_offset, tunning->sw_aiYnrT_noiseAddMot_coeff);

    printf("sw_aiYnrT_narManual_strg=%f sw_aiYnrT_narManual_alpha=%f sw_aiYnrT_narInverse_en=%d\n",
           tunning->sw_aiYnrT_narManual_strg, tunning->sw_aiYnrT_narManual_alpha,
           tunning->sw_aiYnrT_narInverse_en);

    printf("sw_aiYnrT_gain_leftshift=%d sw_aiYnrT_sigmaLut_scale=%f sw_aiYnrT_noiseAdd_limit=%f\n",
           tunning->sw_aiYnrT_gain_leftshift, tunning->sw_aiYnrT_sigmaLut_scale,
           tunning->sw_aiYnrT_noiseAdd_limit);

    printf("sw_aiYnrCfg_model_dir=%s sw_aiYnrT_model_file=%s\n",
           sta->model_dir.sw_aiYnrCfg_model_dir, tunning->sw_aiYnrT_model_file);

    printf("sw_aiYnrT_debug_mode=%d\n",
           sta->debug.sw_aiYnrT_debug_mode);

    for (int i = 0; i < AIYNR_ISO_STEP_MAX; i++) {
        dyn = &attr.stAuto.dyn[i];
        sta = &attr.stAuto.sta;
        calib = &dyn->calib;
        tunning = &dyn->tunning;

        printf("parameter for stAuto index %d:\n", i);

        printf("sw_aiYnrT_gainMerge_alpha=%f sw_aiYnrT_globalGain_strg=%f sw_aiYnrT_noiseAdd_ratio=%f\n",
               tunning->sw_aiYnrT_gainMerge_alpha, tunning->sw_aiYnrT_globalGain_strg,
               tunning->sw_aiYnrT_noiseAdd_ratio);

        printf("sw_aiYnrT_noiseAddMot_offset=%f sw_aiYnrT_noiseAddMot_coeff=%f\n",
               tunning->sw_aiYnrT_noiseAddMot_offset, tunning->sw_aiYnrT_noiseAddMot_coeff);

        printf("sw_aiYnrT_narManual_strg=%f sw_aiYnrT_narManual_alpha=%f sw_aiYnrT_narInverse_en=%d\n",
               tunning->sw_aiYnrT_narManual_strg, tunning->sw_aiYnrT_narManual_alpha,
               tunning->sw_aiYnrT_narInverse_en);

        printf("sw_aiYnrT_gain_leftshift=%d sw_aiYnrT_sigmaLut_scale=%f sw_aiYnrT_noiseAdd_limit=%f\n",
               tunning->sw_aiYnrT_gain_leftshift, tunning->sw_aiYnrT_sigmaLut_scale,
               tunning->sw_aiYnrT_noiseAdd_limit);

        printf("sw_aiYnrCfg_model_dir=%s sw_aiYnrT_model_file=%s\n",
               sta->model_dir.sw_aiYnrCfg_model_dir, tunning->sw_aiYnrT_model_file);

        printf("sw_aiYnrT_debug_mode=%d\n",
               sta->debug.sw_aiYnrT_debug_mode);
    }

    return ret;
#else
    return 0;
#endif
}

static int sample_aiynr_set_attrib(const rk_aiq_sys_ctx_t *ctx) {
#if ISP_HW_V35
    aiynr_api_attrib_t attr;

    aiynr_params_static_t *sta;
    aiynr_param_dyn_t *dyn;
    aiynr_param_calib_t *calib;
    aiynr_param_tunning_t *tunning;

    int ret = rk_aiq_user_api2_aiynr_GetAttrib(ctx, &attr);

    attr.en = true; // can only change before rk_aiq_uapi2_sysctl_prepare
    attr.opMode = RK_AIQ_OP_MODE_AUTO;
    attr.bypass = false;

    dyn = &attr.stMan.dyn;
    sta = &attr.stMan.sta;
    calib = &dyn->calib;
    tunning = &dyn->tunning;

    sprintf(sta->model_dir.sw_aiYnrCfg_model_dir, "/etc/iqfiles/"); // can only change before rk_aiq_uapi2_sysctl_prepare and should same with stAuto
    tunning->sw_aiYnrT_gainMerge_alpha = 1;
    tunning->sw_aiYnrT_globalGain_strg = 1;
    tunning->sw_aiYnrT_noiseAdd_ratio = 0.05;
    tunning->sw_aiYnrT_noiseAddMot_offset = 10;
    tunning->sw_aiYnrT_noiseAddMot_coeff = 4.5;
    tunning->sw_aiYnrT_narManual_strg = 0;
    tunning->sw_aiYnrT_narManual_alpha = 0;
    tunning->sw_aiYnrT_narInverse_en = 1;
    tunning->sw_aiYnrT_gain_leftshift = 0;
    tunning->sw_aiYnrT_sigmaLut_scale = 1;
    tunning->sw_aiYnrT_noiseAdd_limit = 0.5;
    sprintf(tunning->sw_aiYnrT_model_file, "iso50.bin"); // can only change before rk_aiq_uapi2_sysctl_prepare and should same with stAuto

    sta = &attr.stAuto.sta;
    sprintf(sta->model_dir.sw_aiYnrCfg_model_dir, "/etc/iqfiles/"); // can only change before rk_aiq_uapi2_sysctl_prepare
    for (int i = 0; i < AIYNR_ISO_STEP_MAX; i++) {
        dyn = &attr.stAuto.dyn[i];
        calib = &dyn->calib;
        tunning = &dyn->tunning;

        tunning->sw_aiYnrT_gainMerge_alpha = 1;
        tunning->sw_aiYnrT_globalGain_strg = 1;
        tunning->sw_aiYnrT_noiseAdd_ratio = 0.05;
        tunning->sw_aiYnrT_noiseAddMot_offset = 10;
        tunning->sw_aiYnrT_noiseAddMot_coeff = 4.5;
        tunning->sw_aiYnrT_narManual_strg = 0;
        tunning->sw_aiYnrT_narManual_alpha = 0;
        tunning->sw_aiYnrT_narInverse_en = 1;
        tunning->sw_aiYnrT_gain_leftshift = 0;
        tunning->sw_aiYnrT_sigmaLut_scale = 1;
        tunning->sw_aiYnrT_noiseAdd_limit = 0.5;
        sprintf(tunning->sw_aiYnrT_model_file, "iso50.bin"); // can only change before rk_aiq_uapi2_sysctl_prepare
    }

    ret = rk_aiq_user_api2_aiynr_SetAttrib(ctx, &attr);

    return ret;
#else
    return 0;
#endif
}

uapi_case_t aiynr_uapi_list[] = {
    {
        .desc = "Aiynr: get aiynr stats",
        .func = (uapi_case_func)sample_aiynr_query_status,
    },
    {
        .desc = "Aiynr: get aiynr attrib",
        .func = (uapi_case_func)sample_aiynr_get_attrib,
    },
    {
        .desc = "Aiynr: set aiynr attrib",
        .func = (uapi_case_func)sample_aiynr_set_attrib,
    },
    {
        .desc = NULL,
        .func = NULL,
    }
};

XCamReturn sample_aiynr_module(const void *arg) {
    int key = -1;
    CLEAR();
    const demo_context_t *demo_ctx = (demo_context_t *)arg;
    const rk_aiq_sys_ctx_t *ctx;
    if (demo_ctx->camGroup) {
        ctx = (rk_aiq_sys_ctx_t *)(demo_ctx->camgroup_ctx);
    } else {
        ctx = (rk_aiq_sys_ctx_t *)(demo_ctx->aiq_ctx);
    }

    if (ctx == NULL) {
        ERR("%s, ctx is nullptr\n", __FUNCTION__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    uapi_process_loop(ctx, aiynr_uapi_list);

    return XCAM_RETURN_NO_ERROR;
}
