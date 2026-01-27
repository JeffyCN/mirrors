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

static int sample_aibnr_query_status(const rk_aiq_sys_ctx_t *ctx) {
#if ISP_HW_V35
    aibnr_status_t status;
    int ret = rk_aiq_user_api2_aibnr_QueryStatus(ctx, &status);

    aibnr_params_static_t *sta = &status.stMan.sta;
    aibnr_param_dyn_t *dyn = &status.stMan.dyn;
    aibnr_param_calib_t *calib = &dyn->calib;
    aibnr_param_tunning_t *tunning = &dyn->tunning;

    printf("opMode %d, en %d, bypass %d\n", status.opMode, status.en, status.bypass);

    printf("sw_aiBnrC_blkPrev_offset=%f sw_aiBnrC_blkPost_offset=%f sw_aiBnrC_shotNoise_val=%f sw_aiBnrC_readNoise_val=%f sw_aiBnrC_fixedNoise_val=%f\n",
              calib->sw_aiBnrC_blkPrev_offset, calib->sw_aiBnrC_blkPost_offset, calib->sw_aiBnrC_shotNoise_val,
              calib->sw_aiBnrC_readNoise_val, calib->sw_aiBnrC_fixedNoise_val);

    printf("sw_aiBnrT_totalNr_strg=%f sw_aiBnrT_shotNr_strg=%f sw_aiBnrT_readNr_strg=%f sw_aiBnrT_fixedNr_strg=%f\n",
              tunning->sw_aiBnrT_totalNr_strg, tunning->sw_aiBnrT_shotNr_strg,
              tunning->sw_aiBnrT_readNr_strg, tunning->sw_aiBnrT_fixedNr_strg);

    printf("sw_aiBnrT_motionBias_offset=%f sw_aiBnrT_motionGain_strg=%f sw_aiBnrT_noiseAdd_ratio=%f sw_aiBnrT_noiseAdd_limit=%f\n",
              tunning->sw_aiBnrT_motionBias_offset, tunning->sw_aiBnrT_motionGain_strg,
              tunning->sw_aiBnrT_noiseAdd_ratio, tunning->sw_aiBnrT_noiseAdd_limit);

    printf("sw_aiBnrT_noiseAddMot_offset=%f sw_aiBnrT_noiseAddMot_coeff=%f sw_aiBnrT_noiseAddLuma_offset=%f sw_aiBnrT_noiseAddLuma_coeff=%f\n",
              tunning->sw_aiBnrT_noiseAddMot_offset, tunning->sw_aiBnrT_noiseAddMot_coeff,
              tunning->sw_aiBnrT_noiseAddLuma_offset, tunning->sw_aiBnrT_noiseAddLuma_coeff);

    printf("sw_aiBnrT_noiseAddLumaClip_th=%f sw_aiBnrT_noiseAddLumaStatic_th=%f sw_aiBnrT_nonLinear_scale=%f sw_aiBnrT_nonLinear_ratio=%f sw_aiBnrT_nonLinear_adjust=%f\n",
              tunning->sw_aiBnrT_noiseAddLumaClip_th, tunning->sw_aiBnrT_noiseAddLumaStatic_th,
              tunning->sw_aiBnrT_nonLinear_scale, tunning->sw_aiBnrT_nonLinear_ratio, tunning->sw_aiBnrT_nonLinear_adjust);

    printf("sw_aiBnrCfg_model_dir=%s sw_aiBnrT_model_file=%s\n",
              sta->model_dir.sw_aiBnrCfg_model_dir, tunning->sw_aiBnrT_model_file);

    printf("sw_aiBnrT_debug_mode=%d sw_aiBnrT_autoSwOn_thred=%d sw_aiBnrT_autoSwGap_thred=%d sw_aiBnrT_manualBnrHw_en=%d\n",
              sta->debug.sw_aiBnrT_debug_mode, sta->swOn_cfg.sw_aiBnrT_autoSwOn_thred, sta->swOn_cfg.sw_aiBnrT_autoSwGap_thred, sta->swOn_cfg.sw_aiBnrT_manualBnrHw_en);

    return ret;
#else
    return 0;
#endif
}

static int sample_aibnr_get_attrib(const rk_aiq_sys_ctx_t *ctx) {
#if ISP_HW_V35
    aibnr_api_attrib_t attr;
    int ret = rk_aiq_user_api2_aibnr_GetAttrib(ctx, &attr);

    aibnr_params_static_t *sta;
    aibnr_param_dyn_t *dyn;
    aibnr_param_calib_t *calib;
    aibnr_param_tunning_t *tunning;

    printf("opMode %d, en %d, bypass %d\n", attr.opMode, attr.en, attr.bypass);

    dyn = &attr.stMan.dyn;
    sta = &attr.stMan.sta;
    calib = &dyn->calib;
    tunning = &dyn->tunning;

    printf("parameter for stMan:\n");

    printf("sw_aiBnrC_blkPrev_offset=%f sw_aiBnrC_blkPost_offset=%f sw_aiBnrC_shotNoise_val=%f sw_aiBnrC_readNoise_val=%f sw_aiBnrC_fixedNoise_val=%f\n",
              calib->sw_aiBnrC_blkPrev_offset, calib->sw_aiBnrC_blkPost_offset, calib->sw_aiBnrC_shotNoise_val,
              calib->sw_aiBnrC_readNoise_val, calib->sw_aiBnrC_fixedNoise_val);

    printf("sw_aiBnrT_totalNr_strg=%f sw_aiBnrT_shotNr_strg=%f sw_aiBnrT_readNr_strg=%f sw_aiBnrT_fixedNr_strg=%f\n",
              tunning->sw_aiBnrT_totalNr_strg, tunning->sw_aiBnrT_shotNr_strg,
              tunning->sw_aiBnrT_readNr_strg, tunning->sw_aiBnrT_fixedNr_strg);

    printf("sw_aiBnrT_motionBias_offset=%f sw_aiBnrT_motionGain_strg=%f sw_aiBnrT_noiseAdd_ratio=%f sw_aiBnrT_noiseAdd_limit=%f\n",
              tunning->sw_aiBnrT_motionBias_offset, tunning->sw_aiBnrT_motionGain_strg,
              tunning->sw_aiBnrT_noiseAdd_ratio, tunning->sw_aiBnrT_noiseAdd_limit);

    printf("sw_aiBnrT_noiseAddMot_offset=%f sw_aiBnrT_noiseAddMot_coeff=%f sw_aiBnrT_noiseAddLuma_offset=%f sw_aiBnrT_noiseAddLuma_coeff=%f\n",
              tunning->sw_aiBnrT_noiseAddMot_offset, tunning->sw_aiBnrT_noiseAddMot_coeff,
              tunning->sw_aiBnrT_noiseAddLuma_offset, tunning->sw_aiBnrT_noiseAddLuma_coeff);

    printf("sw_aiBnrT_noiseAddLumaClip_th=%f sw_aiBnrT_noiseAddLumaStatic_th=%f sw_aiBnrT_nonLinear_scale=%f sw_aiBnrT_nonLinear_ratio=%f sw_aiBnrT_nonLinear_adjust=%f\n",
              tunning->sw_aiBnrT_noiseAddLumaClip_th, tunning->sw_aiBnrT_noiseAddLumaStatic_th,
              tunning->sw_aiBnrT_nonLinear_scale, tunning->sw_aiBnrT_nonLinear_ratio, tunning->sw_aiBnrT_nonLinear_adjust);

    printf("sw_aiBnrCfg_model_dir=%s sw_aiBnrT_model_file=%s\n",
              sta->model_dir.sw_aiBnrCfg_model_dir, tunning->sw_aiBnrT_model_file);

    printf("sw_aiBnrT_debug_mode=%d sw_aiBnrT_autoSwOn_thred=%d sw_aiBnrT_autoSwGap_thred=%d sw_aiBnrT_manualBnrHw_en=%d\n",
              sta->debug.sw_aiBnrT_debug_mode, sta->swOn_cfg.sw_aiBnrT_autoSwOn_thred, sta->swOn_cfg.sw_aiBnrT_autoSwGap_thred, sta->swOn_cfg.sw_aiBnrT_manualBnrHw_en);

    sta = &attr.stAuto.sta;
    for (int i = 0; i < AIBNR_ISO_STEP_MAX; i++) {
        dyn = &attr.stAuto.dyn[i];
        calib = &dyn->calib;
        tunning = &dyn->tunning;

        printf("parameter for stAuto index %d:\n", i);

        printf("sw_aiBnrC_blkPrev_offset=%f sw_aiBnrC_blkPost_offset=%f sw_aiBnrC_shotNoise_val=%f sw_aiBnrC_readNoise_val=%f sw_aiBnrC_fixedNoise_val=%f\n",
                  calib->sw_aiBnrC_blkPrev_offset, calib->sw_aiBnrC_blkPost_offset, calib->sw_aiBnrC_shotNoise_val,
                  calib->sw_aiBnrC_readNoise_val, calib->sw_aiBnrC_fixedNoise_val);

        printf("sw_aiBnrT_totalNr_strg=%f sw_aiBnrT_shotNr_strg=%f sw_aiBnrT_readNr_strg=%f sw_aiBnrT_fixedNr_strg=%f\n",
                  tunning->sw_aiBnrT_totalNr_strg, tunning->sw_aiBnrT_shotNr_strg,
                  tunning->sw_aiBnrT_readNr_strg, tunning->sw_aiBnrT_fixedNr_strg);

        printf("sw_aiBnrT_motionBias_offset=%f sw_aiBnrT_motionGain_strg=%f sw_aiBnrT_noiseAdd_ratio=%f sw_aiBnrT_noiseAdd_limit=%f\n",
                  tunning->sw_aiBnrT_motionBias_offset, tunning->sw_aiBnrT_motionGain_strg,
                  tunning->sw_aiBnrT_noiseAdd_ratio, tunning->sw_aiBnrT_noiseAdd_limit);

        printf("sw_aiBnrT_noiseAddMot_offset=%f sw_aiBnrT_noiseAddMot_coeff=%f sw_aiBnrT_noiseAddLuma_offset=%f sw_aiBnrT_noiseAddLuma_coeff=%f\n",
                  tunning->sw_aiBnrT_noiseAddMot_offset, tunning->sw_aiBnrT_noiseAddMot_coeff,
                  tunning->sw_aiBnrT_noiseAddLuma_offset, tunning->sw_aiBnrT_noiseAddLuma_coeff);

        printf("sw_aiBnrT_noiseAddLumaClip_th=%f sw_aiBnrT_noiseAddLumaStatic_th=%f sw_aiBnrT_nonLinear_scale=%f sw_aiBnrT_nonLinear_ratio=%f sw_aiBnrT_nonLinear_adjust=%f\n",
                  tunning->sw_aiBnrT_noiseAddLumaClip_th, tunning->sw_aiBnrT_noiseAddLumaStatic_th,
                  tunning->sw_aiBnrT_nonLinear_scale, tunning->sw_aiBnrT_nonLinear_ratio, tunning->sw_aiBnrT_nonLinear_adjust);

        printf("sw_aiBnrCfg_model_dir=%s sw_aiBnrT_model_file=%s\n",
                  sta->model_dir.sw_aiBnrCfg_model_dir, tunning->sw_aiBnrT_model_file);

        printf("sw_aiBnrT_debug_mode=%d sw_aiBnrT_autoSwOn_thred=%d sw_aiBnrT_autoSwGap_thred=%d sw_aiBnrT_manualBnrHw_en=%d\n",
                  sta->debug.sw_aiBnrT_debug_mode, sta->swOn_cfg.sw_aiBnrT_autoSwOn_thred, sta->swOn_cfg.sw_aiBnrT_autoSwGap_thred, sta->swOn_cfg.sw_aiBnrT_manualBnrHw_en);

    }

    return ret;
#else
    return 0;
#endif
}

static int sample_aibnr_set_attrib(const rk_aiq_sys_ctx_t *ctx) {
#if ISP_HW_V35
    aibnr_api_attrib_t attr;

    aibnr_params_static_t *sta;
    aibnr_param_dyn_t *dyn;
    aibnr_param_calib_t *calib;
    aibnr_param_tunning_t *tunning;

    int ret = rk_aiq_user_api2_aibnr_GetAttrib(ctx, &attr);

    attr.en = true; // can only change before rk_aiq_uapi2_sysctl_prepare
    attr.opMode = RK_AIQ_OP_MODE_AUTO;
    attr.bypass = false;

    dyn = &attr.stMan.dyn;
    sta = &attr.stMan.sta;
    calib = &dyn->calib;
    tunning = &dyn->tunning;

    sprintf(sta->model_dir.sw_aiBnrCfg_model_dir, "/etc/iqfiles/sc450ai/bnr/combo_G8"); // can only change before rk_aiq_uapi2_sysctl_prepare and should same with stAuto
    calib->sw_aiBnrC_blkPrev_offset = 0;
    calib->sw_aiBnrC_blkPost_offset = 0;
    calib->sw_aiBnrC_shotNoise_val = 0.063755;
    calib->sw_aiBnrC_readNoise_val = 0.00042;
    calib->sw_aiBnrC_fixedNoise_val = 2.3e-005;
    tunning->sw_aiBnrT_totalNr_strg = 1;
    tunning->sw_aiBnrT_shotNr_strg = 1;
    tunning->sw_aiBnrT_readNr_strg = 1;
    tunning->sw_aiBnrT_fixedNr_strg = 1;
    tunning->sw_aiBnrT_motionBias_offset = 20;
    tunning->sw_aiBnrT_motionGain_strg = 0;
    tunning->sw_aiBnrT_noiseAdd_ratio = 0;
    tunning->sw_aiBnrT_noiseAdd_limit = 0.5;
    tunning->sw_aiBnrT_noiseAddMot_offset = 1;
    tunning->sw_aiBnrT_noiseAddMot_coeff = 0;
    tunning->sw_aiBnrT_noiseAddLuma_offset = 114;
    tunning->sw_aiBnrT_noiseAddLuma_coeff = 32;
    tunning->sw_aiBnrT_noiseAddLumaClip_th = 1;
    tunning->sw_aiBnrT_noiseAddLumaStatic_th = 1;
    tunning->sw_aiBnrT_nonLinear_scale = 0.7;
    tunning->sw_aiBnrT_nonLinear_ratio = 0.5;
    tunning->sw_aiBnrT_nonLinear_adjust = 1.0;
    sprintf(tunning->sw_aiBnrT_model_file, "iso50.bin"); // can only change before rk_aiq_uapi2_sysctl_prepare and should same with stAuto

    sta = &attr.stAuto.sta;
    sprintf(sta->model_dir.sw_aiBnrCfg_model_dir, "/etc/iqfiles/sc450ai/bnr/combo_G8"); // can only change before rk_aiq_uapi2_sysctl_prepare
    for (int i = 0; i < AIBNR_ISO_STEP_MAX; i++) {
        dyn = &attr.stAuto.dyn[i];
        calib = &dyn->calib;
        tunning = &dyn->tunning;

        calib->sw_aiBnrC_blkPrev_offset = 0;
        calib->sw_aiBnrC_blkPost_offset = 0;
        calib->sw_aiBnrC_shotNoise_val = 0.063755;
        calib->sw_aiBnrC_readNoise_val = 0.00042;
        calib->sw_aiBnrC_fixedNoise_val = 2.3e-005;
        tunning->sw_aiBnrT_totalNr_strg = 1;
        tunning->sw_aiBnrT_shotNr_strg = 1;
        tunning->sw_aiBnrT_readNr_strg = 1;
        tunning->sw_aiBnrT_fixedNr_strg = 1;
        tunning->sw_aiBnrT_motionBias_offset = 20;
        tunning->sw_aiBnrT_motionGain_strg = 0;
        tunning->sw_aiBnrT_noiseAdd_ratio = 0;
        tunning->sw_aiBnrT_noiseAdd_limit = 0.5;
        tunning->sw_aiBnrT_noiseAddMot_offset = 1;
        tunning->sw_aiBnrT_noiseAddMot_coeff = 0;
        tunning->sw_aiBnrT_noiseAddLuma_offset = 114;
        tunning->sw_aiBnrT_noiseAddLuma_coeff = 32;
        tunning->sw_aiBnrT_noiseAddLumaClip_th = 1;
        tunning->sw_aiBnrT_noiseAddLumaStatic_th = 1;
        tunning->sw_aiBnrT_nonLinear_scale = 0.7;
        tunning->sw_aiBnrT_nonLinear_ratio = 0.5;
        tunning->sw_aiBnrT_nonLinear_adjust = 1.0;
        sprintf(tunning->sw_aiBnrT_model_file, "iso50.bin"); // can only change before rk_aiq_uapi2_sysctl_prepare
    }

    ret = rk_aiq_user_api2_aibnr_SetAttrib(ctx, &attr);

    return ret;
#else
    return 0;
#endif
}

uapi_case_t aibnr_uapi_list[] = {
  {
    .desc = "Aibnr: get aibnr stats",
    .func = (uapi_case_func)sample_aibnr_query_status,
  },
  {
    .desc = "Aibnr: get aibnr attrib",
    .func = (uapi_case_func)sample_aibnr_get_attrib,
  },
  {
    .desc = "Aibnr: set aibnr attrib",
    .func = (uapi_case_func)sample_aibnr_set_attrib,
  },
  {
    .desc = NULL,
    .func = NULL,
  }
};

XCamReturn sample_aibnr_module(const void *arg) {
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

  uapi_process_loop(ctx, aibnr_uapi_list);

  return XCAM_RETURN_NO_ERROR;
}
