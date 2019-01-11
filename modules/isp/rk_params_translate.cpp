/*
 * Copyright (c) 2018, Fuzhou Rockchip Electronics Co., Ltd
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
 */

#include "rk_params_translate.h"

namespace XCam {

void ParamsTranslate()
{
}

void
ParamsTranslate::convert_from_rkisp_aec_result(
        rk_aiq_ae_results* aec_result, AecResult_t* result, struct CamIA10_SensorModeData *sensor_desc) {
    aec_result->exposure.exposure_time_us = result->coarse_integration_time * 1000 * 1000;
    aec_result->exposure.analog_gain = result->analog_gain_code_global;

    //useless
    aec_result->exposure.digital_gain = result->analog_gain_code_global;
    aec_result->exposure.iso = result->analog_gain_code_global;

    aec_result->sensor_exposure.coarse_integration_time = result->regIntegrationTime;
    aec_result->sensor_exposure.analog_gain_code_global = result->regGain;

    //useless
    aec_result->sensor_exposure.fine_integration_time = result->regIntegrationTime;
    aec_result->sensor_exposure.digital_gain_global = result->gainFactor;

    aec_result->sensor_exposure.frame_length_lines = result->LinePeriodsPerField;
    aec_result->sensor_exposure.line_length_pixels = result->PixelPeriodsPerLine;

    aec_result->flicker_reduction_mode = rk_aiq_ae_flicker_reduction_50hz;

    // grid 5x5 maxsize=[2580x1950]
    aec_result->aec_config_result.enabled = true;
    aec_result->aec_config_result.mode = RK_ISP_EXP_MEASURING_MODE_0;
    aec_result->aec_config_result.win.width =
        result->meas_win.h_size > 2580 ? 2580 : result->meas_win.h_size; // 35 <= value <= 516
    aec_result->aec_config_result.win.height =
        result->meas_win.v_size > 1950 ? 1950 : result->meas_win.v_size; // 28 <= value <= 390
    aec_result->aec_config_result.win.h_offset = // 0 <= value <= 2424
        (sensor_desc->sensor_output_width - aec_result->aec_config_result.win.width) / 2;
    aec_result->aec_config_result.win.v_offset = // 0 <= value <= 1806
         (sensor_desc->sensor_output_height - aec_result->aec_config_result.win.height) / 2;

    aec_result->hist_config_result.enabled = true;
    aec_result->hist_config_result.mode = RK_ISP_HIST_MODE_RGB_COMBINED;
    aec_result->hist_config_result.stepSize = result->stepSize;
    aec_result->hist_config_result.weights_cnt = RK_AIQ_HISTOGRAM_WEIGHT_GRIDS_SIZE;
    memcpy(aec_result->hist_config_result.weights, result->GridWeights, sizeof(unsigned char)*RK_AIQ_HISTOGRAM_WEIGHT_GRIDS_SIZE);
    aec_result->hist_config_result.window.width = result->meas_win.h_size;
    aec_result->hist_config_result.window.height = result->meas_win.v_size;
    aec_result->hist_config_result.window.h_offset =
		(sensor_desc->sensor_output_width - aec_result->hist_config_result.window.width) / 2;
    aec_result->hist_config_result.window.v_offset =
		(sensor_desc->sensor_output_height - aec_result->hist_config_result.window.height) / 2;

    aec_result->converged = result->converged;

#if 0
    printf("aec converged: %d\n", aec_result->converged);

    printf("aec result: vts-hts=%d-%d \n", aec_result->sensor_exposure.frame_length_lines, aec_result->sensor_exposure.line_length_pixels);

    printf("interface check hist weights=[%d-%d-%d-%d-%d]\n",
		aec_result->hist_config_result.weights[0],
		aec_result->hist_config_result.weights[1],
		aec_result->hist_config_result.weights[2],
		aec_result->hist_config_result.weights[3],
		aec_result->hist_config_result.weights[12]);

    printf("interface check aec result win=[%d-%d-%d-%d]\n",
		aec_result->aec_config_result.win.h_offset,
		aec_result->aec_config_result.win.v_offset,
		aec_result->aec_config_result.win.width,
		aec_result->aec_config_result.win.height);

    printf("interface check hist result step=[%d] win=[%d-%d-%d-%d]\n",
		aec_result->hist_config_result.stepSize,
		aec_result->hist_config_result.window.h_offset,
		aec_result->hist_config_result.window.v_offset,
		aec_result->hist_config_result.window.width,
		aec_result->hist_config_result.window.height);
#endif

}

void
ParamsTranslate::convert_from_rkisp_awb_result(
        rk_aiq_awb_results* aiq_awb_result, CamIA10_AWB_Result_t* result, struct CamIA10_SensorModeData *sensor_desc) {

    aiq_awb_result->awb_meas_cfg.enabled = true;
    aiq_awb_result->awb_meas_cfg.awb_meas_mode = RK_ISP_AWB_MEASURING_MODE_YCBCR;//result->MeasMode;
    aiq_awb_result->awb_meas_cfg.awb_meas_cfg.max_y= result->MeasConfig.MaxY;
    aiq_awb_result->awb_meas_cfg.awb_meas_cfg.ref_cr_max_r= result->MeasConfig.RefCr_MaxR;
    aiq_awb_result->awb_meas_cfg.awb_meas_cfg.min_y_max_g= result->MeasConfig.MinY_MaxG;
    aiq_awb_result->awb_meas_cfg.awb_meas_cfg.ref_cb_max_b= result->MeasConfig.RefCb_MaxB;
    aiq_awb_result->awb_meas_cfg.awb_meas_cfg.max_c_sum= result->MeasConfig.MaxCSum;
    aiq_awb_result->awb_meas_cfg.awb_meas_cfg.min_c= result->MeasConfig.MinC;

    aiq_awb_result->awb_meas_cfg.awb_win.h_offset = result->awbWin.h_offs;
    aiq_awb_result->awb_meas_cfg.awb_win.v_offset = result->awbWin.v_offs;
    aiq_awb_result->awb_meas_cfg.awb_win.width= result->awbWin.h_size;
    aiq_awb_result->awb_meas_cfg.awb_win.height = result->awbWin.v_size;

    //394-256-256-296
    aiq_awb_result->awb_gain_cfg.enabled = true;
    aiq_awb_result->awb_gain_cfg.awb_gains.red_gain = result->awbGains.Red == 0 ? 394 : result->awbGains.Red;
    aiq_awb_result->awb_gain_cfg.awb_gains.green_b_gain= result->awbGains.GreenB == 0 ? 256 : result->awbGains.GreenB;
    aiq_awb_result->awb_gain_cfg.awb_gains.green_r_gain= result->awbGains.GreenR == 0 ? 256 : result->awbGains.GreenR;
    aiq_awb_result->awb_gain_cfg.awb_gains.blue_gain= result->awbGains.Blue == 0 ? 296 : result->awbGains.Blue;

    //ALOGD("AWB GAIN RESULT: %d-%d-%d-%d", result->awbGains.Red, result->awbGains.GreenB, result->awbGains.GreenR, result->awbGains.Blue);

    aiq_awb_result->ctk_config.enabled = true;
    memcpy(aiq_awb_result->ctk_config.ctk_matrix.coeff, result->CcMatrix.Coeff, sizeof(unsigned int)*9);
    aiq_awb_result->ctk_config.cc_offset.red= result->CcOffset.Red;
    aiq_awb_result->ctk_config.cc_offset.green= result->CcOffset.Green;
    aiq_awb_result->ctk_config.cc_offset.blue= result->CcOffset.Blue;

    if (sensor_desc->sensor_output_width != 0 &&
		sensor_desc->sensor_output_height != 0) {
    aiq_awb_result->lsc_cfg.enabled = true;
    aiq_awb_result->lsc_cfg.config_width = sensor_desc->sensor_output_width;
    aiq_awb_result->lsc_cfg.config_height = sensor_desc->sensor_output_height;

    aiq_awb_result->lsc_cfg.lsc_config.lsc_size_tbl_cnt = RK_AIQ_LSC_SIZE_TBL_SIZE;
    memcpy(aiq_awb_result->lsc_cfg.lsc_config.lsc_x_size_tbl,
        result->SectorConfig.LscXSizeTbl, RK_AIQ_LSC_SIZE_TBL_SIZE*sizeof(unsigned short));
    memcpy(aiq_awb_result->lsc_cfg.lsc_config.lsc_y_size_tbl,
        result->SectorConfig.LscYSizeTbl, RK_AIQ_LSC_SIZE_TBL_SIZE*sizeof(unsigned short));

    aiq_awb_result->lsc_cfg.lsc_config.lsc_grad_tbl_cnt = RK_AIQ_LSC_GRAD_TBL_SIZE;
    memcpy(aiq_awb_result->lsc_cfg.lsc_config.lsc_x_grad_tbl,
        result->SectorConfig.LscXGradTbl, RK_AIQ_LSC_GRAD_TBL_SIZE*sizeof(unsigned short));
    memcpy(aiq_awb_result->lsc_cfg.lsc_config.lsc_y_grad_tbl,
        result->SectorConfig.LscYGradTbl, RK_AIQ_LSC_GRAD_TBL_SIZE*sizeof(unsigned short));

    aiq_awb_result->lsc_cfg.lsc_config.lsc_data_tbl_cnt = RK_AIQ_LSC_DATA_TBL_SIZE;
    memcpy(aiq_awb_result->lsc_cfg.lsc_config.lsc_r_data_tbl,
        result->LscMatrixTable.LscMatrix[CAM_4CH_COLOR_COMPONENT_RED].uCoeff,
        RK_AIQ_LSC_DATA_TBL_SIZE*sizeof(unsigned short));
    memcpy(aiq_awb_result->lsc_cfg.lsc_config.lsc_gr_data_tbl,
        result->LscMatrixTable.LscMatrix[CAM_4CH_COLOR_COMPONENT_GREENR].uCoeff,
        RK_AIQ_LSC_DATA_TBL_SIZE*sizeof(unsigned short));
    memcpy(aiq_awb_result->lsc_cfg.lsc_config.lsc_gb_data_tbl,
        result->LscMatrixTable.LscMatrix[CAM_4CH_COLOR_COMPONENT_GREENB].uCoeff,
        RK_AIQ_LSC_DATA_TBL_SIZE*sizeof(unsigned short));
    memcpy(aiq_awb_result->lsc_cfg.lsc_config.lsc_b_data_tbl,
        result->LscMatrixTable.LscMatrix[CAM_4CH_COLOR_COMPONENT_BLUE].uCoeff,
        RK_AIQ_LSC_DATA_TBL_SIZE*sizeof(unsigned short));

    }

    aiq_awb_result->converged = result->converged;

    LOGI("awb converged: %d, R-B gain: %d-%d\n",
		aiq_awb_result->converged,
		aiq_awb_result->awb_gain_cfg.awb_gains.red_gain,
		aiq_awb_result->awb_gain_cfg.awb_gains.blue_gain);
#if 0

    printf("--awb config, max_y: %d, cr: %d, cb: %d, miny: %d, maxcsum: %d, minc: %d\n",
           aiq_awb_result->awb_meas_cfg.awb_meas_cfg.max_y,
           aiq_awb_result->awb_meas_cfg.awb_meas_cfg.ref_cr_max_r,
           aiq_awb_result->awb_meas_cfg.awb_meas_cfg.ref_cb_max_b,
           aiq_awb_result->awb_meas_cfg.awb_meas_cfg.min_y_max_g,
           aiq_awb_result->awb_meas_cfg.awb_meas_cfg.max_c_sum,
           aiq_awb_result->awb_meas_cfg.awb_meas_cfg.min_c);

    printf("interface check awb result win=[%d-%d-%d-%d]\n",
		aiq_awb_result->awb_meas_cfg.awb_win.h_offset,
		aiq_awb_result->awb_meas_cfg.awb_win.v_offset,
		aiq_awb_result->awb_meas_cfg.awb_win.width,
		aiq_awb_result->awb_meas_cfg.awb_win.height);

    printf("ctk offset=[%d-%d-%d]\n",
		aiq_awb_result->ctk_config.cc_offset.red,
		aiq_awb_result->ctk_config.cc_offset.green,
		aiq_awb_result->ctk_config.cc_offset.blue);
    printf("interface check awb ctk config=[%d-%d-%d]\n",
		aiq_awb_result->ctk_config.ctk_matrix.coeff[0],
		aiq_awb_result->ctk_config.ctk_matrix.coeff[1],
		aiq_awb_result->ctk_config.ctk_matrix.coeff[2]
		);
    printf("interface check awb ctk config=[%d-%d-%d]\n",
		aiq_awb_result->ctk_config.ctk_matrix.coeff[3],
		aiq_awb_result->ctk_config.ctk_matrix.coeff[4],
		aiq_awb_result->ctk_config.ctk_matrix.coeff[5]
		);
    printf("interface check awb ctk config=[%d-%d-%d]\n",
		aiq_awb_result->ctk_config.ctk_matrix.coeff[6],
		aiq_awb_result->ctk_config.ctk_matrix.coeff[7],
		aiq_awb_result->ctk_config.ctk_matrix.coeff[8]
		);
#endif
}

void ParamsTranslate::convert_to_rkisp_aec_config( XCamAeParam* aec_params,
                                                   HAL_AecCfg* config, struct CamIA10_SensorModeData *sensor_desc) {
    memset(config, 0, sizeof(HAL_AecCfg));

    switch (aec_params->flicker_mode) {
    case XCAM_AE_FLICKER_MODE_AUTO:
        config->flk = HAL_AE_FLK_AUTO;
        break;
    case XCAM_AE_FLICKER_MODE_50HZ:
        config->flk = HAL_AE_FLK_50;
        break;
    case XCAM_AE_FLICKER_MODE_60HZ:
        config->flk = HAL_AE_FLK_60;
        break;
    case XCAM_AE_FLICKER_MODE_OFF:
        config->flk = HAL_AE_FLK_OFF;
        break;
    default :
        LOGI("@%s %d: Do not support the flicker mode:%d", __FUNCTION__, __LINE__, aec_params->flicker_mode);
        config->flk = HAL_AE_FLK_AUTO;
        break;
    }

    switch (aec_params->mode) {
    case XCAM_AE_MODE_AUTO:
        config->mode = HAL_AE_OPERATION_MODE_AUTO;
        break;
    case XCAM_AE_MODE_MANUAL:
        config->mode = HAL_AE_OPERATION_MODE_MANUAL;
        break;
    default :
        LOGI("@%s %d: Do not support the ae mode:%d", __FUNCTION__, __LINE__, aec_params->mode);
        config->mode = HAL_AE_OPERATION_MODE_AUTO;
        break;
    }

    switch (aec_params->metering_mode) {
    case XCAM_AE_METERING_MODE_AUTO:
        config->meter_mode = HAL_AE_METERING_MODE_CENTER;
        break;
    case XCAM_AE_METERING_MODE_SPOT:
        config->meter_mode = HAL_AE_METERING_MODE_SPOT;
        break;
    case XCAM_AE_METERING_MODE_CENTER:
        config->meter_mode = HAL_AE_METERING_MODE_CENTER;
        break;
    case XCAM_AE_METERING_MODE_WEIGHTED_WINDOW:
        config->meter_mode = HAL_AE_METERING_MODE_MATRIX;
        break;
    default :
        LOGI("@%s %d: Do not support the metering_mode:%d", __FUNCTION__, __LINE__, aec_params->metering_mode);
        config->meter_mode = HAL_AE_METERING_MODE_AVERAGE;
        break;
    }

    // window
    if (aec_params->window.x_end < 0 || aec_params->window.x_end > sensor_desc->sensor_output_width ||
        aec_params->window.y_end < 0 || aec_params->window.y_end > sensor_desc->sensor_output_height) {
        LOGW("%s, XCamAeParam window is not right top,bottom(%d,%d)", __FUNCTION__,
             aec_params->window.x_end, aec_params->window.y_end);
    } else if( aec_params->window.x_end != 0 && aec_params->window.y_end != 0 ) {
        config->win.left_hoff =  aec_params->window.x_start;
        config->win.top_voff =  aec_params->window.y_start;
        config->win.right_width =  aec_params->window.x_end - aec_params->window.x_start;
        config->win.bottom_height =  aec_params->window.y_end - aec_params->window.y_start;
    } else {
        config->win.left_hoff = 0;
        config->win.top_voff =  0;
        config->win.right_width = sensor_desc->sensor_output_width;
        config->win.bottom_height = sensor_desc->sensor_output_height;
    }
    // bias
    config->ae_bias = (int)(aec_params->ev_shift);

    if (config->mode == HAL_AE_OPERATION_MODE_AUTO) {
        config->frame_time_ms_min = aec_params->exposure_time_min;
        config->frame_time_ms_max = aec_params->exposure_time_max;
    } else {
        config->frame_time_ms_min = aec_params->manual_exposure_time;
        config->frame_time_ms_max = aec_params->manual_exposure_time;
        config->manual_gains = aec_params->manual_analog_gain;
    }
    LOGI("@%s %d: aec_config, flk:%d, mode:%d, meter_mode:%d, win(%d,%d,%d,%d), bias:%d, min:%d, max:%d ", __FUNCTION__, __LINE__,
         config->flk, config->mode, config->meter_mode,
         config->win.left_hoff, config->win.top_voff, config->win.right_width, config->win.bottom_height,
         config->ae_bias, config->frame_time_ms_min, config->frame_time_ms_max);
}

void
ParamsTranslate::convert_to_rkisp_awb_config(XCamAwbParam* awb_params,
                                             HAL_AwbCfg* config, struct CamIA10_SensorModeData *sensor_desc) {
    memset(config, 0, sizeof(HAL_AwbCfg));

    /* config->mode = HAL_WB_AUTO;//awb_params->mode; */
    // here only list the awb modes that camerEngine and APP both support
    switch (awb_params->mode) {
    case XCAM_AWB_MODE_NOT_SET:
        config->mode = HAL_WB_INVAL;
        break;
    case XCAM_AWB_MODE_AUTO:
        config->mode = HAL_WB_AUTO;
        break;
    case XCAM_AWB_MODE_MANUAL:
        config->mode = HAL_WB_MANUAL_CCT;
        break;
    case XCAM_AWB_MODE_DAYLIGHT:
        config->mode = HAL_WB_DAYLIGHT;
        break;
    case XCAM_AWB_MODE_SUNSET:
        config->mode = HAL_WB_SUNSET;
        break;
    case XCAM_AWB_MODE_CLOUDY:
        config->mode = HAL_WB_CLOUDY_DAYLIGHT;
        break;
    case XCAM_AWB_MODE_FLUORESCENT:
        config->mode = HAL_WB_FLUORESCENT;
        break;
    case XCAM_AWB_MODE_WARM_INCANDESCENT:
        config->mode = HAL_WB_INCANDESCENT;
        break;
    default :
        LOGI("@%s %d: Do not support the Awb mode :%d, set awb mode to Auto mode", __FUNCTION__, __LINE__, awb_params->mode);
        config->mode = HAL_WB_AUTO;
        break;
    }

    if (awb_params->window.x_end < 0 || awb_params->window.x_end > sensor_desc->sensor_output_width ||
        awb_params->window.y_end < 0 || awb_params->window.y_end > sensor_desc->sensor_output_height) {
        LOGD("%s, XCamAwbParam window is not right", __func__);
        return;
    } else if( awb_params->window.x_end != 0 && awb_params->window.y_end != 0 ) {
        config->win.left_hoff = awb_params->window.x_start;
        config->win.top_voff = awb_params->window.y_start;
        config->win.right_width = awb_params->window.x_end - awb_params->window.x_start;
        config->win.bottom_height = awb_params->window.y_end - awb_params->window.y_start;
    } else {
        config->win.left_hoff = 0;
        config->win.top_voff = 0;
        config->win.right_width= sensor_desc->sensor_output_width;
        config->win.bottom_height= sensor_desc->sensor_output_height;
    }
}

void
ParamsTranslate::convert_to_rkisp_af_config(XCamAfParam* af_params,
                                       HAL_AfcCfg* config, struct CamIA10_SensorModeData *sensor_desc) {
    memset(config, 0, sizeof(HAL_AfcCfg));
    switch (af_params->focus_mode) {
    case AF_MODE_NOT_SET:
        config->mode = HAL_AF_MODE_NOT_SET;
        break;
    case AF_MODE_AUTO:
        config->mode = HAL_AF_MODE_AUTO;
        break;
    case AF_MODE_MACRO:
        config->mode = HAL_AF_MODE_MACRO;
        break;
    case AF_MODE_INFINITY:
        config->mode = HAL_AF_MODE_INFINITY;
        break;
    case AF_MODE_FIXED:
        config->mode = HAL_AF_MODE_FIXED;
        break;
    case AF_MODE_EDOF:
        config->mode = HAL_AF_MODE_EDOF;
        break;
    case AF_MODE_CONTINUOUS_VIDEO:
        config->mode = HAL_AF_MODE_CONTINUOUS_VIDEO;
        break;
    case AF_MODE_CONTINUOUS_PICTURE:
        config->mode = HAL_AF_MODE_CONTINUOUS_PICTURE;
        break;
    default :
        LOGI("@%s %d: Do not support the Af mode :%d, set af mode to Auto mode", __FUNCTION__, __LINE__, af_params->focus_mode);
        config->mode = HAL_AF_MODE_AUTO;
        break;
    }
    config->oneshot_trigger =
		af_params->trigger_new_search ? BOOL_TRUE : BOOL_FALSE;
    /* config->win_num = af_params->focus_rect_cnt; */
    config->win_num = 1;

    config->win_a.left_hoff = af_params->focus_rect[0].left_hoff;
    config->win_a.top_voff= af_params->focus_rect[0].top_voff;
    config->win_a.right_width = af_params->focus_rect[0].right_width;
    config->win_a.bottom_height = af_params->focus_rect[0].bottom_height;
    LOGI("@%s %d: af_config, mode:%d, new_search:%d, win(%d,%d,%d,%d)", __FUNCTION__, __LINE__, config->mode, config->oneshot_trigger,
         config->win_a.left_hoff, config->win_a.top_voff, config->win_a.right_width, config->win_a.bottom_height);
}

void
ParamsTranslate::convert_from_rkisp_af_result(rk_aiq_af_results* aiq_af_result,
                                         XCam3aResultFocus* result, struct CamIA10_SensorModeData *sensor_desc) {

    aiq_af_result->afc_config = result->afc_config;
    aiq_af_result->status = result->status;
    aiq_af_result->current_focus_distance = result->current_focus_distance;
    aiq_af_result->next_lens_position = result->next_lens_position;
    aiq_af_result->final_lens_position_reached = result->final_lens_position_reached;
}

};
