#include <arc_rkisp_adapter.h>
#include <utils/Log.h>

#define DEFAULT_PCLK 180
#define DEFAULT_VTS 2228
#define DEFAULT_HTS 2960
#define DEFAULT_WIDTH 2592
#define DEFAULT_HEIGHT 1944

#define ENABLED true;
#define DISABLED false;


rk_aiq_exposure_sensor_descriptor g_sensor_desc = {0};
/*
    {DEFAULT_PCLK,
    DEFAULT_HTS,
    DEFAULT_VTS,
    0, 0, 0, 0, 0,
    DEFAULT_WIDTH,
    DEFAULT_HEIGHT};
*/

void convert_to_rkisp_stats(
		const rk_aiq_statistics_input_params* aiq_stats,
                const rk_aiq_exposure_sensor_descriptor *sensor_desc,
		struct CamIA10_Stats* stats) {
    memset(stats, 0, sizeof(struct CamIA10_Stats));

    if (aiq_stats != NULL) {
        stats->meas_type = CAMIA10_AEC_MASK | CAMIA10_HST_MASK;
        memcpy(stats->aec.exp_mean, aiq_stats->aec_stats.exp_mean, sizeof(unsigned char)*CIFISP_AE_MEAN_MAX);
        memcpy(stats->aec.hist_bins, aiq_stats->aec_stats.hist_bin, sizeof(int)*CIFISP_HIST_BIN_N_MAX);

        stats->awb.MeanCr__R = aiq_stats->awb_stats.awb_meas[0].mean_cr__r;
        stats->awb.MeanY__G= aiq_stats->awb_stats.awb_meas[0].mean_y__g;
        stats->awb.MeanCb__B= aiq_stats->awb_stats.awb_meas[0].mean_cb__b;
        stats->awb.NoWhitePixel = aiq_stats->awb_stats.awb_meas[0].num_white_pixel;

        stats->af.LuminanceA = aiq_stats->af_stats.window[0].lum;
        stats->af.PixelCntA= aiq_stats->af_stats.window[0].sum;
        stats->af.LuminanceB = aiq_stats->af_stats.window[1].lum;
        stats->af.PixelCntB= aiq_stats->af_stats.window[1].sum;
        stats->af.LuminanceC = aiq_stats->af_stats.window[2].lum;
        stats->af.PixelCntC= aiq_stats->af_stats.window[2].sum;

        stats->sensor_mode.exp_time = aiq_stats->ae_results->sensor_exposure.coarse_integration_time;
        stats->sensor_mode.gain = aiq_stats->ae_results->sensor_exposure.analog_gain_code_global;
    }

    if (sensor_desc != NULL) {
	g_sensor_desc = *sensor_desc;
        stats->sensor_mode.coarse_integration_time_max_margin =
            sensor_desc->coarse_integration_time_max_margin;
        stats->sensor_mode.coarse_integration_time_min =
            sensor_desc->coarse_integration_time_min;
        stats->sensor_mode.fine_integration_time_max_margin =
            sensor_desc->fine_integration_time_max_margin;
        stats->sensor_mode.fine_integration_time_min = 
            sensor_desc->fine_integration_time_min;
        stats->sensor_mode.line_periods_per_field =
            sensor_desc->line_periods_per_field;
/*
        stats->sensor_mode.
            sensor_desc.line_periods_vertical_blanking;
*/

#if 0
        LOGD("------------CHECK WIDTH-HEIGHT=[%d-%d]",
            sensor_desc->sensor_output_width,
            sensor_desc->sensor_output_height
       	);
#endif

        stats->sensor_mode.pixel_clock_freq_mhz =
            sensor_desc->pixel_clock_freq_mhz == 0 ?
            	DEFAULT_PCLK : sensor_desc->pixel_clock_freq_mhz;
        stats->sensor_mode.pixel_periods_per_line =
            sensor_desc->pixel_periods_per_line == 0 ?
            	DEFAULT_HTS : sensor_desc->pixel_periods_per_line;
        stats->sensor_mode.isp_output_width =
            sensor_desc->isp_output_width == 0 ?
            	DEFAULT_WIDTH : sensor_desc->isp_output_width;
        stats->sensor_mode.isp_output_height =
            sensor_desc->isp_output_height == 0 ?
            	DEFAULT_HEIGHT : sensor_desc->isp_output_height;
        stats->sensor_mode.isp_input_width =
            sensor_desc->isp_input_width == 0 ?
            	DEFAULT_WIDTH : sensor_desc->isp_input_width;
        stats->sensor_mode.isp_input_height =
            sensor_desc->isp_input_height == 0 ?
            	DEFAULT_HEIGHT : sensor_desc->isp_input_height;;
        stats->sensor_mode.sensor_output_height =
            sensor_desc->sensor_output_height == 0 ?
            	DEFAULT_HEIGHT : sensor_desc->sensor_output_height;
    }else {
        g_sensor_desc.sensor_output_width = DEFAULT_WIDTH;
        g_sensor_desc.sensor_output_height = DEFAULT_HEIGHT;
        g_sensor_desc.pixel_periods_per_line = DEFAULT_HTS;
        g_sensor_desc.pixel_clock_freq_mhz = DEFAULT_PCLK;
    }

}

void convert_to_rkisp_aec_params(
        const rk_aiq_ae_input_params* aec_input_params, HAL_AecCfg* config) {
    memset(config, 0, sizeof(HAL_AecCfg));

    config->flk = HAL_AE_FLK_MODE(aec_input_params->flicker_reduction_mode);
    config->mode = HAL_AE_OPERATION_MODE(aec_input_params->operation_mode);
    config->meter_mode = HAL_AE_METERING_MODE(aec_input_params->metering_mode);

    // window
    if (config->mode != HAL_AE_OPERATION_MODE_AUTO) {
        if (aec_input_params->window == NULL) {
            LOGD("%s, rk_aiq_ae_input_params_t window is null", __func__);
            return;
        }
        config->win.left_hoff =  aec_input_params->window->h_offset;
        config->win.top_voff =  aec_input_params->window->v_offset;
        config->win.right_width =  aec_input_params->window->v_offset + aec_input_params->window->width;
        config->win.bottom_height =  aec_input_params->window->h_offset + aec_input_params->window->height;
    } else {
        config->win.left_hoff = 0;
	config->win.top_voff =  0;
	config->win.right_width = g_sensor_desc.sensor_output_width;
	config->win.bottom_height = g_sensor_desc.sensor_output_height;
    }
    // bias
    config->ae_bias = (int)(aec_input_params->ev_shift);

    if (aec_input_params->manual_limits != NULL) {
/*
	printf("manual limits frame time min-max=%d-%d\n",
		aec_input_params->manual_limits->manual_frame_time_us_min,
		aec_input_params->manual_limits->manual_frame_time_us_max);
*/
        config->frame_time_us_min = aec_input_params->manual_limits->manual_frame_time_us_min;
        config->frame_time_us_max = aec_input_params->manual_limits->manual_frame_time_us_max;
    }
}

void convert_from_rkisp_aec_result(
        rk_aiq_ae_results* aec_result, AecResult_t* result) {

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
        (g_sensor_desc.sensor_output_width - aec_result->aec_config_result.win.width) / 2;
    aec_result->aec_config_result.win.v_offset = // 0 <= value <= 1806
         (g_sensor_desc.sensor_output_height - aec_result->aec_config_result.win.height) / 2;

    aec_result->hist_config_result.enabled = true;
    aec_result->hist_config_result.mode = RK_ISP_HIST_MODE_RGB_COMBINED;
    aec_result->hist_config_result.stepSize = result->stepSize;
    aec_result->hist_config_result.weights_cnt = RK_AIQ_HISTOGRAM_WEIGHT_GRIDS_SIZE;
    memcpy(aec_result->hist_config_result.weights, result->GridWeights, sizeof(unsigned char)*RK_AIQ_HISTOGRAM_WEIGHT_GRIDS_SIZE);
    aec_result->hist_config_result.window.width = result->meas_win.h_size;
    aec_result->hist_config_result.window.height = result->meas_win.v_size;
    aec_result->hist_config_result.window.h_offset = 
		(g_sensor_desc.sensor_output_width - aec_result->hist_config_result.window.width) / 2;
    aec_result->hist_config_result.window.v_offset = 
		(g_sensor_desc.sensor_output_height - aec_result->hist_config_result.window.height) / 2;

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

void convert_to_rkisp_awb_params(
        const rk_aiq_awb_input_params* awb_input_params, HAL_AwbCfg* config) {
    memset(config, 0, sizeof(HAL_AwbCfg));

    config->mode = HAL_WB_AUTO;//awb_input_params->scene_mode;
    if (config->mode != HAL_WB_AUTO) {
        if (awb_input_params->window == NULL) {
            LOGD("%s, rk_aiq_awb_input_params_t window is null", __func__);
            return;
        }

        config->win.left_hoff = awb_input_params->window->h_offset;
        config->win.top_voff = awb_input_params->window->v_offset;
        config->win.right_width = awb_input_params->window->width;
        config->win.bottom_height = awb_input_params->window->height;
    } else {
        config->win.left_hoff = 0;
        config->win.top_voff = 0;
        config->win.right_width= g_sensor_desc.sensor_output_width;
        config->win.bottom_height= g_sensor_desc.sensor_output_height;
    }

}

void convert_from_rkisp_awb_result(
        rk_aiq_awb_results* awb_input_params, CamIA10_AWB_Result_t* result) {

    awb_input_params->awb_meas_cfg.enabled = true;
    awb_input_params->awb_meas_cfg.awb_meas_mode = RK_ISP_AWB_MEASURING_MODE_YCBCR;//result->MeasMode;
    awb_input_params->awb_meas_cfg.awb_meas_cfg.max_y= result->MeasConfig.MaxY;
    awb_input_params->awb_meas_cfg.awb_meas_cfg.ref_cr_max_r= result->MeasConfig.RefCr_MaxR;
    awb_input_params->awb_meas_cfg.awb_meas_cfg.min_y_max_g= result->MeasConfig.MinY_MaxG;
    awb_input_params->awb_meas_cfg.awb_meas_cfg.ref_cb_max_b= result->MeasConfig.RefCb_MaxB;
    awb_input_params->awb_meas_cfg.awb_meas_cfg.max_c_sum= result->MeasConfig.MaxCSum;
    awb_input_params->awb_meas_cfg.awb_meas_cfg.min_c= result->MeasConfig.MinC;

    awb_input_params->awb_meas_cfg.awb_win.h_offset = result->awbWin.h_offs;
    awb_input_params->awb_meas_cfg.awb_win.v_offset = result->awbWin.v_offs;
    awb_input_params->awb_meas_cfg.awb_win.width= result->awbWin.h_size;
    awb_input_params->awb_meas_cfg.awb_win.height = result->awbWin.v_size;

    //394-256-256-296
    awb_input_params->awb_gain_cfg.enabled = true;
    awb_input_params->awb_gain_cfg.awb_gains.red_gain = result->awbGains.Red == 0 ? 394 : result->awbGains.Red;
    awb_input_params->awb_gain_cfg.awb_gains.green_b_gain= result->awbGains.GreenB == 0 ? 256 : result->awbGains.GreenB;
    awb_input_params->awb_gain_cfg.awb_gains.green_r_gain= result->awbGains.GreenR == 0 ? 256 : result->awbGains.GreenR;
    awb_input_params->awb_gain_cfg.awb_gains.blue_gain= result->awbGains.Blue == 0 ? 296 : result->awbGains.Blue;

    //LOGD("AWB GAIN RESULT: %d-%d-%d-%d", result->awbGains.Red, result->awbGains.GreenB, result->awbGains.GreenR, result->awbGains.Blue);

    awb_input_params->ctk_config.enabled = true;
    memcpy(awb_input_params->ctk_config.ctk_matrix.coeff, result->CcMatrix.Coeff, sizeof(unsigned int)*9);
    awb_input_params->ctk_config.cc_offset.red= result->CcOffset.Red;
    awb_input_params->ctk_config.cc_offset.green= result->CcOffset.Green;
    awb_input_params->ctk_config.cc_offset.blue= result->CcOffset.Blue;

    if (g_sensor_desc.sensor_output_width != 0 &&
		g_sensor_desc.sensor_output_height != 0) {
    awb_input_params->lsc_cfg.enabled = true;
    awb_input_params->lsc_cfg.config_width = g_sensor_desc.sensor_output_width;
    awb_input_params->lsc_cfg.config_height = g_sensor_desc.sensor_output_height;

    awb_input_params->lsc_cfg.lsc_config.lsc_size_tbl_cnt = RK_AIQ_LSC_SIZE_TBL_SIZE;
    memcpy(awb_input_params->lsc_cfg.lsc_config.lsc_x_size_tbl,
        result->SectorConfig.LscXSizeTbl, RK_AIQ_LSC_SIZE_TBL_SIZE*sizeof(unsigned short));
    memcpy(awb_input_params->lsc_cfg.lsc_config.lsc_y_size_tbl,
        result->SectorConfig.LscYSizeTbl, RK_AIQ_LSC_SIZE_TBL_SIZE*sizeof(unsigned short));

    awb_input_params->lsc_cfg.lsc_config.lsc_grad_tbl_cnt = RK_AIQ_LSC_GRAD_TBL_SIZE;
    memcpy(awb_input_params->lsc_cfg.lsc_config.lsc_x_grad_tbl,
        result->SectorConfig.LscXGradTbl, RK_AIQ_LSC_GRAD_TBL_SIZE*sizeof(unsigned short));
    memcpy(awb_input_params->lsc_cfg.lsc_config.lsc_y_grad_tbl,
        result->SectorConfig.LscYGradTbl, RK_AIQ_LSC_GRAD_TBL_SIZE*sizeof(unsigned short));

    awb_input_params->lsc_cfg.lsc_config.lsc_data_tbl_cnt = RK_AIQ_LSC_DATA_TBL_SIZE;
    memcpy(awb_input_params->lsc_cfg.lsc_config.lsc_r_data_tbl,
        result->LscMatrixTable.LscMatrix[CAM_4CH_COLOR_COMPONENT_RED].uCoeff,
        RK_AIQ_LSC_DATA_TBL_SIZE*sizeof(unsigned short));
    memcpy(awb_input_params->lsc_cfg.lsc_config.lsc_gr_data_tbl,
        result->LscMatrixTable.LscMatrix[CAM_4CH_COLOR_COMPONENT_GREENR].uCoeff,
        RK_AIQ_LSC_DATA_TBL_SIZE*sizeof(unsigned short));
    memcpy(awb_input_params->lsc_cfg.lsc_config.lsc_gb_data_tbl,
        result->LscMatrixTable.LscMatrix[CAM_4CH_COLOR_COMPONENT_GREENB].uCoeff,
        RK_AIQ_LSC_DATA_TBL_SIZE*sizeof(unsigned short));
    memcpy(awb_input_params->lsc_cfg.lsc_config.lsc_b_data_tbl,
        result->LscMatrixTable.LscMatrix[CAM_4CH_COLOR_COMPONENT_BLUE].uCoeff,
        RK_AIQ_LSC_DATA_TBL_SIZE*sizeof(unsigned short));

    }

    awb_input_params->converged = result->converged;
    
#if 0
    printf("awb converged: %d, R-B gain: %d-%d\n",
		awb_input_params->converged,
		awb_input_params->awb_gain_cfg.awb_gains.red_gain,
		awb_input_params->awb_gain_cfg.awb_gains.blue_gain);

    printf("--awb config, max_y: %d, cr: %d, cb: %d, miny: %d, maxcsum: %d, minc: %d\n",
    		awb_input_params->awb_meas_cfg.awb_meas_cfg.max_y,
    		awb_input_params->awb_meas_cfg.awb_meas_cfg.ref_cr_max_r,
    		awb_input_params->awb_meas_cfg.awb_meas_cfg.ref_cb_max_b,
    		awb_input_params->awb_meas_cfg.awb_meas_cfg.min_y_max_g,
    		awb_input_params->awb_meas_cfg.awb_meas_cfg.max_c_sum,
    		awb_input_params->awb_meas_cfg.awb_meas_cfg.min_c);

    printf("interface check awb result win=[%d-%d-%d-%d]\n",
		awb_input_params->awb_meas_cfg.awb_win.h_offset,
		awb_input_params->awb_meas_cfg.awb_win.v_offset,
		awb_input_params->awb_meas_cfg.awb_win.width,
		awb_input_params->awb_meas_cfg.awb_win.height);

    printf("ctk offset=[%d-%d-%d]\n",
		awb_input_params->ctk_config.cc_offset.red,
		awb_input_params->ctk_config.cc_offset.green,
		awb_input_params->ctk_config.cc_offset.blue);
    printf("interface check awb ctk config=[%d-%d-%d]\n",
		awb_input_params->ctk_config.ctk_matrix.coeff[0],
		awb_input_params->ctk_config.ctk_matrix.coeff[1],
		awb_input_params->ctk_config.ctk_matrix.coeff[2]
		);
    printf("interface check awb ctk config=[%d-%d-%d]\n",
		awb_input_params->ctk_config.ctk_matrix.coeff[3],
		awb_input_params->ctk_config.ctk_matrix.coeff[4],
		awb_input_params->ctk_config.ctk_matrix.coeff[5]
		);
    printf("interface check awb ctk config=[%d-%d-%d]\n",
		awb_input_params->ctk_config.ctk_matrix.coeff[6],
		awb_input_params->ctk_config.ctk_matrix.coeff[7],
		awb_input_params->ctk_config.ctk_matrix.coeff[8]
		);
#endif
}

void convert_to_rkisp_af_params(
        const rk_aiq_af_input_params* af_input_params, HAL_AfcCfg* config) {
    memset(config, 0, sizeof(HAL_AfcCfg));

    config->mode = HAL_AF_MODE_AUTO;
    config->oneshot_trigger =
		af_input_params->trigger_new_search ? BOOL_TRUE : BOOL_FALSE;
    config->win_num = 1;
    if (config->mode == HAL_AF_MODE_AUTO) {
    config->win_a.left_hoff = af_input_params->focus_rect->v_offset;
    config->win_a.top_voff= af_input_params->focus_rect->width;
    config->win_a.right_width = af_input_params->focus_rect->h_offset;
    config->win_a.bottom_height = af_input_params->focus_rect->height;
    }
}

void convert_from_rkisp_af_result(
        rk_aiq_af_results* af_input_params, CamIA10_AFC_Result_t* result) {
    af_input_params->afc_config.enabled = true;
    af_input_params->afc_config.num_afm_win = result->Window_Num;
    af_input_params->afc_config.afm_win[0].h_offset = result->WindowA.h_offs;
    af_input_params->afc_config.afm_win[0].width = result->WindowA.h_size;
    af_input_params->afc_config.afm_win[0].v_offset = result->WindowA.v_offs;
    af_input_params->afc_config.afm_win[0].height = result->WindowA.v_size;

    af_input_params->afc_config.thres = result->Thres;
    af_input_params->afc_config.var_shift = result->VarShift;
}

void convert_from_rkisp_misc_result( CamIA10Engine* iqEngine,
	rk_aiq_misc_isp_results* misc_results, struct CamIA10_Results* ia_results) {
#if 0
	//TODO
	/*ISP sub modules result*/
	CamerIcIspBlsConfig_t bls;
	CamerIcDpccConfig_t dpcc;
	CamerIcIspDegammaCurve_t sdg;
	CamerIcIspFltConfig_t flt;

	CamerIcCprocConfig_t cproc;
	CamerIcIeConfig_t ie;
#endif 
	struct HAL_ISP_cfg_s  manCfg = {0};

	//BLS CONFIG
	bool mBlsNeededUpdate = true;
	struct HAL_ISP_bls_cfg_s bls_cfg;

	if (mBlsNeededUpdate) {
		memset(&bls_cfg,  0, sizeof(struct HAL_ISP_bls_cfg_s));
		manCfg.bls_cfg = &bls_cfg;
		manCfg.updated_mask |= HAL_ISP_BLS_MASK;
		manCfg.enabled[HAL_ISP_BLS_ID] = HAL_ISP_ACTIVE_DEFAULT;
	}

	//BPC/DPCC CONFIG
	bool mDpccNeededUpdate = true;
	struct HAL_ISP_dpcc_cfg_s dpcc_cfg;

	if (mDpccNeededUpdate) {
		memset(&bls_cfg,  0, sizeof(struct HAL_ISP_bls_cfg_s));
		manCfg.dpcc_cfg = &dpcc_cfg;
		manCfg.updated_mask |= HAL_ISP_BPC_MASK;
		manCfg.enabled[HAL_ISP_BPC_ID] = HAL_ISP_ACTIVE_DEFAULT;
	}

	//FLT CONFIG
	bool mFltNeededUpdate = true;
	struct HAL_ISP_flt_cfg_s flt_cfg;

	if (mFltNeededUpdate) {
		memset(&flt_cfg,  0, sizeof(struct HAL_ISP_flt_cfg_s));
		manCfg.flt_cfg = &flt_cfg;
		manCfg.updated_mask |= HAL_ISP_FLT_MASK;
		manCfg.enabled[HAL_ISP_FLT_ID] = HAL_ISP_ACTIVE_DEFAULT;
	}

	//BDM CONFIG
	bool mBdmNeededUpdate = true;
	struct HAL_ISP_bdm_cfg_s bdm_cfg;

	if (mBdmNeededUpdate) {
		memset(&bdm_cfg,  0, sizeof(struct HAL_ISP_bdm_cfg_s));
		manCfg.bdm_cfg = &bdm_cfg;
		manCfg.updated_mask |= HAL_ISP_BDM_MASK;
		manCfg.enabled[HAL_ISP_BDM_ID] = HAL_ISP_ACTIVE_DEFAULT;
	}

	//CPROC CONFIG
	bool mCprocNeededUpdate = true;
	struct HAL_ISP_cproc_cfg_s cproc_cfg;

	if (mCprocNeededUpdate) {
		memset(&cproc_cfg,	0, sizeof(struct HAL_ISP_cproc_cfg_s));
		manCfg.cproc_cfg = &cproc_cfg;
		manCfg.updated_mask |= HAL_ISP_CPROC_MASK;
		manCfg.enabled[HAL_ISP_CPROC_ID] = HAL_ISP_ACTIVE_DEFAULT;
	}

	//GOC CONFIG
	bool mGocNeededUpdate = true;
	struct HAL_ISP_goc_cfg_s goc_cfg;

	if (mGocNeededUpdate) {
		memset(&goc_cfg,  0, sizeof(struct HAL_ISP_goc_cfg_s));
		goc_cfg.used_cnt = CAMERIC_ISP_GAMMA_CURVE_SIZE;
		manCfg.updated_mask |= HAL_ISP_GOC_MASK;
		manCfg.enabled[HAL_ISP_GOC_ID] = HAL_ISP_ACTIVE_DEFAULT;
		manCfg.goc_cfg = &goc_cfg;
	}

	//GOC CONFIG
	bool mIeNeededUpdate = true;
	struct HAL_ISP_ie_cfg_s ie_cfg;

	if (mIeNeededUpdate) {
		memset(&ie_cfg,  0, sizeof(struct HAL_ISP_ie_cfg_s));
		manCfg.ie_cfg = &ie_cfg;
		manCfg.updated_mask |= HAL_ISP_IE_MASK;
		manCfg.enabled[HAL_ISP_IE_ID] = HAL_ISP_ACTIVE_DEFAULT;
	}

	//DPF CONFIG
	bool mDpfNeededUpdate = true;
	struct HAL_ISP_dpf_cfg_s dpf_cfg;

	if (mDpfNeededUpdate) {
		memset(&dpf_cfg,  0, sizeof(struct HAL_ISP_dpf_cfg_s));
		manCfg.enabled[HAL_ISP_DPF_ID] = HAL_ISP_ACTIVE_DEFAULT;
		if (true) {
		  //controlled by adpf default
		manCfg.updated_mask &= ~HAL_ISP_DPF_MASK;
		mDpfNeededUpdate = BOOL_FALSE;
		} else {
		manCfg.dpf_cfg = &dpf_cfg;
		manCfg.updated_mask |= HAL_ISP_DPF_MASK;
		}
	}

	//DPF CONFIG
	bool mDpfStrengthNeededUpdate = true;
	struct HAL_ISP_dpf_strength_cfg_s dpf_strength_cfg;

	if (mDpfStrengthNeededUpdate) {
	  memset(&dpf_strength_cfg,  0, sizeof(struct HAL_ISP_dpf_strength_cfg_s));
	  manCfg.enabled[HAL_ISP_DPF_STRENGTH_ID] = HAL_ISP_ACTIVE_DEFAULT;
	  if (true) {
		//controlled by adpf default
		manCfg.updated_mask &= ~HAL_ISP_DPF_STRENGTH_MASK;
		mDpfStrengthNeededUpdate = BOOL_FALSE;
	  } else {
		manCfg.dpf_strength_cfg = &dpf_strength_cfg;
		manCfg.updated_mask |= HAL_ISP_DPF_STRENGTH_MASK;
	  }
	}

	//DPF CONFIG
	bool mSdgNeededUpdate = true;
	struct HAL_ISP_sdg_cfg_s sdg_cfg;

	if (mSdgNeededUpdate) {
	  memset(&sdg_cfg,	0, sizeof(struct HAL_ISP_sdg_cfg_s));
	  manCfg.sdg_cfg = &sdg_cfg;
	  manCfg.updated_mask |= HAL_ISP_SDG_MASK;
	  manCfg.enabled[HAL_ISP_SDG_ID] = HAL_ISP_ACTIVE_DEFAULT;
	}

	//DPF CONFIG
	bool mWdrNeededUpdate = false;
	struct HAL_ISP_wdr_cfg_s wdr_cfg;

	if (mWdrNeededUpdate) {
	  memset(&wdr_cfg,	0, sizeof(struct HAL_ISP_wdr_cfg_s));
	  manCfg.wdr_cfg = &wdr_cfg;
	  manCfg.updated_mask |= HAL_ISP_WDR_MASK;
	  manCfg.enabled[HAL_ISP_WDR_ID] = HAL_ISP_ACTIVE_DEFAULT;
	}

	bool mDemosaicLPNeededUpdate = false;
	struct HAL_ISP_demosaiclp_cfg_s demosaicLP_cfg;

	if (mDemosaicLPNeededUpdate) {
	  memset(&demosaicLP_cfg,	0, sizeof(struct HAL_ISP_demosaiclp_cfg_s));
	  manCfg.demosaicLP_cfg = &demosaicLP_cfg;
	  manCfg.updated_mask |= HAL_ISP_DEMOSAICLP_MASK;
	  manCfg.enabled[HAL_ISP_DEMOSAICLP_ID] = HAL_ISP_ACTIVE_DEFAULT;
	}

	bool mrkIEsharpNeededUpdate = false;
	struct HAL_ISP_RKIEsharp_cfg_s rkIESharp_cfg;

	if (mrkIEsharpNeededUpdate) {
	  memset(&rkIESharp_cfg,	0, sizeof(struct HAL_ISP_RKIEsharp_cfg_s));
	  manCfg.rkIEsharp_cfg = &rkIESharp_cfg;
	  manCfg.updated_mask |= HAL_ISP_RKIESHARP_MASK;
	  manCfg.enabled[HAL_ISP_RKIESHARP_ID] = HAL_ISP_ACTIVE_DEFAULT;
	}

	iqEngine->runManISP(&manCfg, ia_results);

	//-------------------apply result to HAL-------------------------
	misc_results->dpcc_config.enabled = ENABLED;
	misc_results->flt_config.enabled = ENABLED;
	misc_results->bdm_config.enabled = ENABLED;
	misc_results->gbce_config.cproc_config.enabled = ENABLED;
	misc_results->gbce_config.goc_config.enabled = ENABLED;
	misc_results->gbce_config.ie_config.enabled = ENABLED;
	misc_results->wdr_config.enabled = ENABLED;

	//memcpy(&misc_results->bls_config, &ia_results->bls, sizeof(rk_aiq_bls_config));
	//memcpy(&misc_results->dpcc_config, &ia_results->dpcc, sizeof(rk_aiq_dpcc_config));
	//memcpy(&misc_results->flt_config, &ia_results->flt, sizeof(rk_aiq_flt_config));
	//memcpy(&misc_results->bdm_config, &ia_results->bdm, sizeof(rk_aiq_bdm_config));
	//memcpy(&misc_results->gbce_config.cproc_config, &ia_results->cproc, sizeof(rk_aiq_cproc_config));
	//memcpy(&misc_results->gbce_config.goc_config, &ia_results->goc, sizeof(rk_aiq_goc_config));
	//memcpy(&misc_results->gbce_config.ie_config, &ia_results->ie, sizeof(rk_aiq_ie_config));
	//memcpy(&misc_results->wdr_config, &ia_results->wdr, sizeof(rk_aiq_isp_wdr_config));

	//apply bls config
	misc_results->bls_config.enabled = ENABLED;
	misc_results->bls_config.isp_bls_a_fixed = ia_results->bls.isp_bls_a_fixed;
	misc_results->bls_config.isp_bls_b_fixed = ia_results->bls.isp_bls_b_fixed;
	misc_results->bls_config.isp_bls_c_fixed = ia_results->bls.isp_bls_c_fixed;
	misc_results->bls_config.isp_bls_d_fixed = ia_results->bls.isp_bls_d_fixed;
	
	misc_results->bls_config.num_win = ia_results->bls.num_win;
	misc_results->bls_config.window1.h_offset = ia_results->bls.Window1.hOffset;
	misc_results->bls_config.window1.v_offset = ia_results->bls.Window1.vOffset;
	misc_results->bls_config.window1.width= ia_results->bls.Window1.width;
	misc_results->bls_config.window1.height= ia_results->bls.Window1.height;

	misc_results->bls_config.window2.h_offset = ia_results->bls.Window2.hOffset;
	misc_results->bls_config.window2.v_offset = ia_results->bls.Window2.vOffset;
	misc_results->bls_config.window2.width= ia_results->bls.Window2.width;
	misc_results->bls_config.window2.height= ia_results->bls.Window2.height;

	//apply dpcc config
	misc_results->dpcc_config.enabled = ENABLED;
	misc_results->dpcc_config.isp_dpcc_mode = ia_results->dpcc.isp_dpcc_mode;		   
	misc_results->dpcc_config.isp_dpcc_output_mode = ia_results->dpcc.isp_dpcc_output_mode;   
	misc_results->dpcc_config.isp_dpcc_set_use = ia_results->dpcc.isp_dpcc_set_use; 				   
	misc_results->dpcc_config.isp_dpcc_methods_set_1 = ia_results->dpcc.isp_dpcc_methods_set_1; 
	misc_results->dpcc_config.isp_dpcc_methods_set_2 = ia_results->dpcc.isp_dpcc_methods_set_2; 
	misc_results->dpcc_config.isp_dpcc_methods_set_3 = ia_results->dpcc.isp_dpcc_methods_set_3; 			  
	misc_results->dpcc_config.isp_dpcc_line_thresh_1 = ia_results->dpcc.isp_dpcc_line_thresh_1; 
	misc_results->dpcc_config.isp_dpcc_line_mad_fac_1 = ia_results->dpcc.isp_dpcc_line_mad_fac_1;
	misc_results->dpcc_config.isp_dpcc_pg_fac_1 = ia_results->dpcc.isp_dpcc_pg_fac_1;	   
	misc_results->dpcc_config.isp_dpcc_rnd_thresh_1 = ia_results->dpcc.isp_dpcc_rnd_thresh_1;  
	misc_results->dpcc_config.isp_dpcc_rg_fac_1 = ia_results->dpcc.isp_dpcc_rg_fac_1;					  
	misc_results->dpcc_config.isp_dpcc_line_thresh_2 = ia_results->dpcc.isp_dpcc_line_thresh_2; 
	misc_results->dpcc_config.isp_dpcc_line_mad_fac_2 = ia_results->dpcc.isp_dpcc_line_mad_fac_2;
	misc_results->dpcc_config.isp_dpcc_pg_fac_2 = ia_results->dpcc.isp_dpcc_pg_fac_2;	   
	misc_results->dpcc_config.isp_dpcc_rnd_thresh_2 = ia_results->dpcc.isp_dpcc_rnd_thresh_2;  
	misc_results->dpcc_config.isp_dpcc_rg_fac_2 = ia_results->dpcc.isp_dpcc_rg_fac_2;				   
	misc_results->dpcc_config.isp_dpcc_line_thresh_3 = ia_results->dpcc.isp_dpcc_line_thresh_3; 
	misc_results->dpcc_config.isp_dpcc_line_mad_fac_3 = ia_results->dpcc.isp_dpcc_line_mad_fac_3;
	misc_results->dpcc_config.isp_dpcc_pg_fac_3 = ia_results->dpcc.isp_dpcc_pg_fac_3;	   
	misc_results->dpcc_config.isp_dpcc_rnd_thresh_3 = ia_results->dpcc.isp_dpcc_rnd_thresh_3;  
	misc_results->dpcc_config.isp_dpcc_rg_fac_3 = ia_results->dpcc.isp_dpcc_rg_fac_3;					
	misc_results->dpcc_config.isp_dpcc_ro_limits = ia_results->dpcc.isp_dpcc_ro_limits; 	
	misc_results->dpcc_config.isp_dpcc_rnd_offs = ia_results->dpcc.isp_dpcc_rnd_offs;  


	//apply bdm config, demosaic bypass should be disable
	misc_results->bdm_config.enabled = DISABLED;//ia_results->bdm.enabled;
	misc_results->bdm_config.demosaic_th = ia_results->bdm.demosaic_th;

	//apply flt config
	misc_results->flt_config.enabled = ENABLED;//ia_results->flt.enabled;
	misc_results->flt_config.mode = rk_aiq_flt_mode(ia_results->flt.mode);
	misc_results->flt_config.grn_stage1= ia_results->flt.grn_stage1;
	misc_results->flt_config.chr_h_mode= ia_results->flt.chr_h_mode;
	misc_results->flt_config.chr_v_mode= ia_results->flt.chr_v_mode;
	misc_results->flt_config.thresh_bl0= ia_results->flt.thresh_bl0;
	misc_results->flt_config.thresh_bl1= ia_results->flt.thresh_bl1;
	misc_results->flt_config.thresh_sh0= ia_results->flt.thresh_sh0;
	misc_results->flt_config.thresh_sh1= ia_results->flt.thresh_sh1;
	misc_results->flt_config.lum_weight= ia_results->flt.lum_weight;
	misc_results->flt_config.fac_sh1= ia_results->flt.fac_sh1;
	misc_results->flt_config.fac_sh0= ia_results->flt.fac_sh0;
	misc_results->flt_config.fac_mid= ia_results->flt.fac_mid;
	misc_results->flt_config.fac_bl0 = ia_results->flt.fac_bl0;
	misc_results->flt_config.fac_bl1 = ia_results->flt.fac_bl1;
	misc_results->flt_config.denoise_level= ia_results->flt.denoise_level;
	misc_results->flt_config.sharp_level= ia_results->flt.sharp_level;

	//apply dpf config
	misc_results->dpf_config.enabled = ENABLED;
	misc_results->dpf_config.nf_ains.red_gain= ia_results->adpf.NfGains.Red;
	misc_results->dpf_config.nf_ains.green_b_gain= ia_results->adpf.NfGains.GreenB;
	misc_results->dpf_config.nf_ains.green_r_gain= ia_results->adpf.NfGains.GreenR;
	misc_results->dpf_config.nf_ains.blue_gain= ia_results->adpf.NfGains.Blue;
	misc_results->dpf_config.lookup.nll_coeff_size = RK_AIQ_DPF_MAX_NLF_COEFFS;
	memcpy(misc_results->dpf_config.lookup.nll_coeff, 
		ia_results->adpf.Nll.NllCoeff, misc_results->dpf_config.lookup.nll_coeff_size);
	misc_results->dpf_config.lookup.x_scale = rk_aiq_isp_dpf_nll_scale(ia_results->adpf.Nll.xScale);
	misc_results->dpf_config.gain_usage = rk_aiq_isp_dpf_gain_usage(ia_results->adpf.DpfMode.GainUsage);
	misc_results->dpf_config.rb_flt_size = rk_aiq_isp_dpf_rb_flt_size(ia_results->adpf.DpfMode.RBFilterSize);
	misc_results->dpf_config.spatial_g.weight_coeff_size = RK_AIQ_DPF_MAX_SPATIAL_COEFFS;
	memcpy(misc_results->dpf_config.spatial_g.weight_coeff,
		ia_results->adpf.DpfMode.SpatialG.WeightCoeff, misc_results->dpf_config.spatial_g.weight_coeff_size);
	misc_results->dpf_config.spatial_rb.weight_coeff_size = RK_AIQ_DPF_MAX_SPATIAL_COEFFS;
	memcpy(misc_results->dpf_config.spatial_rb.weight_coeff, 
		ia_results->adpf.DpfMode.SpatialRB.WeightCoeff, misc_results->dpf_config.spatial_rb.weight_coeff_size);
	misc_results->dpf_config.process_red_pixel = ia_results->adpf.DpfMode.ProcessRedPixel;
	misc_results->dpf_config.process_gb_pixel = ia_results->adpf.DpfMode.ProcessGreenBPixel;
	misc_results->dpf_config.process_gr_pixel = ia_results->adpf.DpfMode.ProcessGreenRPixel;
	misc_results->dpf_config.process_blue_pixel = ia_results->adpf.DpfMode.ProcessBluePixel;
/*
	printf("dpf r,gr,gb,b config: %d-%d-%d-%d, xscale: %d, halsscale: %d\n",
		ia_results->adpf.Nll.xScale,
		misc_results->dpf_config.lookup.x_scale,
		misc_results->dpf_config.process_red_pixel,
		misc_results->dpf_config.process_gb_pixel,
		misc_results->dpf_config.process_gr_pixel,
		misc_results->dpf_config.process_blue_pixel);
*/
	//apply dpf strength config
	misc_results->strength_config.enabled = ENABLED;
	misc_results->strength_config.r = ia_results->adpf.DynInvStrength.WeightR;
	misc_results->strength_config.g = ia_results->adpf.DynInvStrength.WeightG;
	misc_results->strength_config.b = ia_results->adpf.DynInvStrength.WeightB;
	//printf("adpf strength: %d-%d-%d\n", misc_results->strength_config.r, misc_results->strength_config.g, misc_results->strength_config.b);

	//apply sdg config
	misc_results->sdg_config.enabled = ENABLED;
	misc_results->sdg_config.seg_size = RK_AIQ_DEGAMMA_CURVE_SIZE - 1;
	memcpy(misc_results->sdg_config.segment, ia_results->sdg.segment, sizeof (misc_results->sdg_config.seg_size));
	memcpy(misc_results->sdg_config.red, ia_results->sdg.red, RK_AIQ_DEGAMMA_CURVE_SIZE);
	memcpy(misc_results->sdg_config.green, ia_results->sdg.green, RK_AIQ_DEGAMMA_CURVE_SIZE);
	memcpy(misc_results->sdg_config.blue, ia_results->sdg.blue, RK_AIQ_DEGAMMA_CURVE_SIZE);

        //apply cproc config
        misc_results->gbce_config.cproc_config.enabled= DISABLED;
	misc_results->gbce_config.cproc_config.ChromaOut = rk_aiq_isp_quantization_range(ia_results->cproc.ChromaOut);
	misc_results->gbce_config.cproc_config.LumaOut = rk_aiq_isp_quantization_range(ia_results->cproc.LumaOut);	 
	misc_results->gbce_config.cproc_config.LumaIn = rk_aiq_isp_quantization_range(ia_results->cproc.LumaIn);   

        misc_results->gbce_config.cproc_config.contrast = ia_results->cproc.contrast;
	misc_results->gbce_config.cproc_config.brightness = ia_results->cproc.brightness;
	misc_results->gbce_config.cproc_config.saturation = ia_results->cproc.saturation;
	misc_results->gbce_config.cproc_config.hue = ia_results->cproc.hue;

	//apply ie config
	misc_results->gbce_config.ie_config.enabled = DISABLED;
	misc_results->gbce_config.ie_config.mode = rk_aiq_isp_ie_mode(ia_results->ie.mode);
	misc_results->gbce_config.ie_config.range = rk_aiq_isp_quantization_range(ia_results->ie.range);
	misc_results->gbce_config.ie_config.mode_config.sepia.tint_cb =
		ia_results->ie.ModeConfig.Sepia.TintCb;
	misc_results->gbce_config.ie_config.mode_config.sepia.tint_Cr =
		ia_results->ie.ModeConfig.Sepia.TintCr;
	misc_results->gbce_config.ie_config.mode_config.color_selection.col_selection =
		rk_aiq_isp_ie_color_sel(ia_results->ie.ModeConfig.ColorSelection.col_selection);
	misc_results->gbce_config.ie_config.mode_config.color_selection.col_threshold =
		ia_results->ie.ModeConfig.ColorSelection.col_threshold;
	memcpy(misc_results->gbce_config.ie_config.mode_config.emboss.coeff,
		ia_results->ie.ModeConfig.Emboss.coeff, sizeof(char)*9);
	memcpy(misc_results->gbce_config.ie_config.mode_config.sketch.coeff,
		ia_results->ie.ModeConfig.Sketch.coeff, sizeof(char)*9);
	misc_results->gbce_config.ie_config.mode_config.sharpen.factor =
		ia_results->ie.ModeConfig.Sharpen.factor;
	misc_results->gbce_config.ie_config.mode_config.sharpen.threshold =
		ia_results->ie.ModeConfig.Sharpen.threshold;
	memcpy(misc_results->gbce_config.ie_config.mode_config.sharpen.coeff,
		ia_results->ie.ModeConfig.Sharpen.coeff, sizeof(char)*9);

	//applay wdr config TODO
	//apply goc config
	misc_results->gbce_config.goc_config.enabled = ENABLED;//ia_results->goc.enabled;
	misc_results->gbce_config.goc_config.mode = rk_aiq_gamma_seg_mode(0); //ia_results->goc.mode
	misc_results->gbce_config.goc_config.gamma_y.gamma_y_cnt = CAMERIC_ISP_GAMMA_CURVE_SIZE;
	memcpy(misc_results->gbce_config.goc_config.gamma_y.gamma_y,
		ia_results->goc.gamma_y.GammaY, CAMERIC_ISP_GAMMA_CURVE_SIZE);

}
	

