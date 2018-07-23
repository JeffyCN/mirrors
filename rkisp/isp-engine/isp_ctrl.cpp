#include <ebase/types.h>
#include <ebase/trace.h>
#include <ebase/builtins.h>
#include <sys/ioctl.h>
#include <calib_xml/calibdb.h>
#include <base/log.h>
#include <isp_ctrl.h>

//extern IspEngineItf::Configuration mIspCfg;

int getSensorModeData(int devFd,
    struct isp_supplemental_sensor_mode_data* data) {
	int ret = 0;

	ret = ioctl(devFd, RK_VIDIOC_SENSOR_MODE_DATA, data);

	if (ret < 0)
		LOGE("ERR(%s): RK_VIDIOC_SENSOR_MODE_DATA failed, err: %s \n",
		     __func__, strerror(errno));

	return ret;
}

int configIsp(Isp10Engine *ispDev,
		struct isp_supplemental_sensor_mode_data* sensor) {
	IspEngineItf::Configuration cfg;
	struct HAL_SensorModeData sensor_data;

	//cfg = mIspCfg;
	//sensor_data = mIspCfg.sensor_mode;
	xcam_mem_clear(cfg);
	xcam_mem_clear(sensor_data);

	/* config sensor mode data */
	if (sensor && (
	      (sensor->isp_input_width != sensor_data.isp_input_width) ||
	      (sensor->isp_input_height != sensor_data.isp_input_height) ||
	      (sensor->vt_pix_clk_freq_hz / 1000000.0f != sensor_data.pixel_clock_freq_mhz) ||
	      (sensor->crop_horizontal_start != sensor_data.horizontal_crop_offset) ||
	      (sensor->crop_vertical_start != sensor_data.vertical_crop_offset) ||
	      (sensor->crop_horizontal_end - sensor->crop_horizontal_start + 1 != sensor_data.cropped_image_width) ||
	      (sensor->crop_vertical_end - sensor->crop_vertical_start + 1 != sensor_data.cropped_image_height) ||
	      (sensor->line_length_pck != sensor_data.pixel_periods_per_line) ||
	      (sensor->frame_length_lines != sensor_data.line_periods_per_field) ||
	      (sensor->sensor_output_height != sensor_data.sensor_output_height) ||
	      (sensor->fine_integration_time_min != sensor_data.fine_integration_time_min)  ||
	      (sensor->line_length_pck - sensor->fine_integration_time_max_margin != sensor_data.fine_integration_time_max_margin) ||
	      (sensor->coarse_integration_time_min != sensor_data.coarse_integration_time_min)  ||
	      (sensor->coarse_integration_time_max_margin != sensor_data.coarse_integration_time_max_margin) ||
	      (sensor->gain != sensor_data.gain) ||
	      (sensor->exp_time != sensor_data.exp_time) ||
	      (sensor->exposure_valid_frame[0] != sensor_data.exposure_valid_frame))) {


		cfg.sensor_mode.isp_input_width = sensor->isp_input_width;
		cfg.sensor_mode.isp_input_height = sensor->isp_input_height;
		cfg.sensor_mode.isp_output_width = sensor->isp_output_width;
		cfg.sensor_mode.isp_output_height = sensor->isp_output_height;
		cfg.sensor_mode.pixel_clock_freq_mhz = sensor->vt_pix_clk_freq_hz / 1000000.0f;
		cfg.sensor_mode.horizontal_crop_offset = sensor->crop_horizontal_start;
		cfg.sensor_mode.vertical_crop_offset = sensor->crop_vertical_start;
        cfg.sensor_mode.cropped_image_width = sensor->crop_horizontal_end - sensor->crop_horizontal_start + 1;
        cfg.sensor_mode.cropped_image_height = sensor->crop_vertical_end - sensor->crop_vertical_start + 1;
        cfg.sensor_mode.pixel_periods_per_line =  sensor->line_length_pck;
        cfg.sensor_mode.line_periods_per_field = sensor->frame_length_lines;
        cfg.sensor_mode.sensor_output_height = sensor->sensor_output_height;
        cfg.sensor_mode.fine_integration_time_min = sensor->fine_integration_time_min;
        cfg.sensor_mode.fine_integration_time_max_margin = sensor->line_length_pck - sensor->fine_integration_time_max_margin;
        cfg.sensor_mode.coarse_integration_time_min = sensor->coarse_integration_time_min;
        cfg.sensor_mode.coarse_integration_time_max_margin = sensor->coarse_integration_time_max_margin;
        cfg.sensor_mode.gain = sensor->gain;
        cfg.sensor_mode.exp_time = sensor->exp_time;
        cfg.sensor_mode.exposure_valid_frame = sensor->exposure_valid_frame[0];
       }

       /*config controls*/
       cfg.uc = UC_PREVIEW;
       cfg.aaa_locks = HAL_3A_LOCKS_NONE;
       cfg.aec_cfg.flk = HAL_AE_FLK_AUTO;
       cfg.aec_cfg.mode = HAL_AE_OPERATION_MODE_AUTO;
       cfg.aec_cfg.meter_mode = HAL_AE_METERING_MODE_CENTER;
       cfg.aec_cfg.ae_bias = 0;
       //cfg.aec_cfg.win = ;

       cfg.afc_cfg.mode = HAL_AF_MODE_CONTINUOUS_PICTURE;//HAL_AF_MODE_AUTO;

       cfg.afc_cfg.oneshot_trigger = BOOL_FALSE;
       cfg.afc_cfg.win_num = 1;
       //cfg.afc_cfg.win = ;
       //cfg.afc_cfg.win_a = mAfWin;

       cfg.awb_cfg.mode = HAL_WB_AUTO;
       //cfg.awb_cfg.win = ;

       cfg.cproc.brightness = 0;
       cfg.cproc.contrast = 0;
       cfg.cproc.hue = 0;
       cfg.cproc.saturation = 0;
       //cfg.cproc.sharpness = ;

       cfg.flash_mode = HAL_FLASH_OFF;
       cfg.ie_mode = HAL_EFFECT_NONE;

       //TODO: ae bias,zoom,rotation,3a areas

       if (!ispDev->configure(cfg)) {
               ALOGE("%s: mISPDev->configure failed!", __func__);
       }

       //mIspCfg = cfg;
       return 0;
}
