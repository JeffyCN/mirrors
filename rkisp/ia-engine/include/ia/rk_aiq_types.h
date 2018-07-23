/*
 * Copyright (c) 2017, Fuzhou Rockchip Electronics Co., Ltd
 *
 * Based on Intel IPU3 HAL by Intel Corporation.
 *
 * Copyright (C) 2015 - 2017 Intel Corporation.
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

/*!
 * \file rk_aiq_types.h
 * \brief Definitions of input/output structures of the Rockchip 3A library.
 */

#ifndef __RK_AIQ_TYPES_H__
#define __RK_AIQ_TYPES_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RK_AIQ_AFM_MAX_WINDOWS             3
#define RK_AIQ_LSC_GRAD_TBL_SIZE           8
#define RK_AIQ_LSC_SIZE_TBL_SIZE           8
#define RK_AIQ_LSC_DATA_TBL_SIZE           289
#define RK_AIQ_DEGAMMA_CURVE_SIZE          17
#define RK_AIQ_GAMMA_OUT_MAX_SAMPLES       34
#define RK_AIQ_HISTOGRAM_WEIGHT_GRIDS_SIZE 81
#define RK_AIQ_AE_MEAN_MAX                 81
#define RK_AIQ_HIST_BIN_N_MAX              32
#define RK_AIQ_WDR_MAX_CURVE_SIZE          33
#define RK_AIQ_DPF_MAX_NLF_COEFFS          17
#define RK_AIQ_DPF_MAX_SPATIAL_COEFFS      6
#define RK_AIQ_AWB_MAX_GRID                1

#define HAL_ISP_BPC_MASK            (1 << 0)
#define HAL_ISP_BLS_MASK            (1 << 1)
#define HAL_ISP_SDG_MASK            (1 << 2)
#define HAL_ISP_HST_MASK            (1 << 3)
#define HAL_ISP_LSC_MASK            (1 << 4)
#define HAL_ISP_AWB_GAIN_MASK       (1 << 5)
#define HAL_ISP_FLT_MASK            (1 << 6)
#define HAL_ISP_BDM_MASK            (1 << 7)
#define HAL_ISP_CTK_MASK            (1 << 8)
#define HAL_ISP_GOC_MASK            (1 << 9)
#define HAL_ISP_CPROC_MASK          (1 << 10)
#define HAL_ISP_AFC_MASK            (1 << 11)
#define HAL_ISP_AWB_MEAS_MASK       (1 << 12)
#define HAL_ISP_IE_MASK             (1 << 13)
#define HAL_ISP_AEC_MASK            (1 << 14)
#define HAL_ISP_WDR_MASK            (1 << 15)
#define HAL_ISP_DPF_MASK            (1 << 16)
#define HAL_ISP_DPF_STRENGTH_MASK   (1 << 17)

#define HAL_ISP_ALL_MASK            (0xffffffff)

/*----------------------- AIQ related start ---------------------*/

typedef struct
{
  unsigned int coeff[9U];
} rk_aiq_3x3Matrix;

typedef struct
{
  unsigned short    h_offset;
  unsigned short    v_offset;
  unsigned short    width;
  unsigned short    height;
} rk_aiq_window;

/*!
 * \brief Frame parameters which describe cropping and scaling (need to be filled by AIQ client for every frame)
 */
typedef struct
{
  unsigned short horizontal_crop_offset;            /*!< Read out offset horizontal. */
  unsigned short vertical_crop_offset;              /*!< Read out offset vertical. */
  unsigned short cropped_image_width;               /*!< Width of cropped area in native resolution. */
  unsigned short cropped_image_height;              /*!< Height of cropped area in native resolution. */
  unsigned char horizontal_scaling_numerator;       /*!< Horizontal scaling factor applied to the cropped image. Horizontal scaling factor = horizontal_scaling_numerator / horizontal_scaling_denominator. */
  unsigned char horizontal_scaling_denominator;     /*!< Horizontal scaling factor applied to the cropped image. Horizontal scaling factor = horizontal_scaling_numerator / horizontal_scaling_denominator. */
  unsigned char vertical_scaling_numerator;         /*!< Vertical scaling factor applied to the cropped image. Vertical scaling factor = vertical_scaling_numerator / vertical_scaling_denominator. */
  unsigned char vertical_scaling_denominator;       /*!< Vertical scaling factor applied to the cropped image. Vertical scaling factor = vertical_scaling_numerator / vertical_scaling_denominator. */
} rk_aiq_frame_params;

typedef enum
{
    rk_aiq_ae_flicker_reduction_off,     /*!< Disables flicker detection and reduction. */
    rk_aiq_ae_flicker_reduction_50hz,    /*!< Manual flicker reduction for 50Hz mains frequency. */
    rk_aiq_ae_flicker_reduction_60hz,    /*!< Manual flicker reduction for 60Hz mains frequency. */
    rk_aiq_ae_flicker_reduction_auto,    /*!< Detects flicker frequency and applies detected reduction. */
    rk_aiq_ae_flicker_reduction_detect,  /*!< Detects only flicker frequency but doesn't apply reduction. */
} rk_aiq_ae_flicker_reduction;

/*!
 * \brief AEC operation modes.
 */
typedef enum
{
    rk_aiq_ae_operation_mode_automatic,         /*!< Automatic mode. */
    rk_aiq_ae_operation_mode_long_exposure,     /*!< AEC produces exposure parameters with long exposure (low light & static) scene. */
    rk_aiq_ae_operation_mode_action,            /*!< AEC produces exposure parameters for fast moving scene. */
    rk_aiq_ae_operation_mode_video_conference,  /*!< AEC produces exposure parameters which can be used in video conferencing scene. */
    rk_aiq_ae_operation_mode_production_test,   /*!< AEC produces exposure parameters which are used in production test environment. */
    rk_aiq_ae_operation_mode_ultra_low_light,   /*!< AEC produces exposure parameters which are used in ultra low light scene. */
    rk_aiq_ae_operation_mode_fireworks,         /*!< AEC produces exposure parameters which are used in fireworks scene. */
    rk_aiq_ae_operation_mode_hdr,               /*!< AEC produces exposure parameters which are used for HDR imaging (vHDR or exposure bracketing). */
    rk_aiq_ae_operation_mode_custom_1,          /*!< AEC produces exposure parameters for a specialized custom scene 1. */
    rk_aiq_ae_operation_mode_custom_2,          /*!< AEC produces exposure parameters for a specialized custom scene 2. */
    rk_aiq_ae_operation_mode_custom_3,          /*!< AEC produces exposure parameters for a specialized custom scene 3. */
} rk_aiq_ae_operation_mode;

/*!
 * \brief AEC metering modes.
 */
typedef enum
{
    rk_aiq_ae_metering_mode_evaluative,  /*!< Exposure is evaluated from the whole frame. */
    rk_aiq_ae_metering_mode_center,      /*!< Exposure is evaluated center weighted. */
} rk_aiq_ae_metering_mode;

/*!
 * \brief AEC priority modes.
 */
typedef enum
{
    rk_aiq_ae_priority_mode_normal,    /*!< All areas are equally important. */
    rk_aiq_ae_priority_mode_highlight, /*!< Highlights must be preserved even if it means that dark parts become very dark. */
    rk_aiq_ae_priority_mode_shadow,    /*!< Shadow areas are more important. */
} rk_aiq_ae_priority_mode;

/*!
 * \brief Target (frame use) for the analysis algorithms calculations.
 */
typedef enum
{
    rk_aiq_frame_use_preview,
    rk_aiq_frame_use_still,
    rk_aiq_frame_use_continuous,
    rk_aiq_frame_use_video,
} rk_aiq_frame_use;

/*!
 * \brief Camera orientations.
 */
typedef enum
{
    rk_aiq_camera_orientation_unknown,      /*!< Orientation not known. */
    rk_aiq_camera_orientation_rotate_0,     /*!< Non-rotated landscape. */
    rk_aiq_camera_orientation_rotate_90,    /*!< Portrait i.e. rotated 90 degrees clockwise. */
    rk_aiq_camera_orientation_rotate_180,   /*!< Landscape (upside down) i.e. rotated 180 degrees clockwise. */
    rk_aiq_camera_orientation_rotate_270    /*!< Portrait (upside down) i.e. rotated 270 degrees clockwise. */
} rk_aiq_camera_orientation;

/*!
 * \brief Autofocus states
 */
typedef enum
{
    rk_aiq_af_status_idle,               /*!< Focus is idle */
    rk_aiq_af_status_local_search,       /*!< Focus is in local search state */
    rk_aiq_af_status_extended_search,    /*!< Focus is in extended search state */
    rk_aiq_af_status_success,            /*!< Focus has succeeded */
    rk_aiq_af_status_fail,               /*!< Focus has failed */
    rk_aiq_af_status_depth_search        /*!< Focus in depth search mode */
} rk_aiq_af_status;

/*!
 * \brief Autofocus modes
 */
typedef enum
{
    rk_aiq_af_operation_mode_auto,                 /*!< Auto mode */
    rk_aiq_af_operation_mode_infinity,             /*!< Inifinity mode */
    rk_aiq_af_operation_mode_hyperfocal,           /*!< Hyperfocal mode */
    rk_aiq_af_operation_mode_manual,               /*!< Manual mode */
    rk_aiq_af_operation_mode_production_test,      /*!< Production test mode. */
    rk_aiq_af_operation_mode_depth_map,            /*!< Depth-map generation mode. */
    rk_aiq_af_operation_mode_depth,                /*!< Automatic focusing based on depth measurements only. */
    rk_aiq_af_operation_mode_contrast              /*!< Automatic focusing based on contrast measurements only. */
} rk_aiq_af_operation_mode;

/*!
 * \brief Autofocus range
 */
typedef enum
{
    rk_aiq_af_range_normal,                        /*!< Normal range */
    rk_aiq_af_range_macro,                         /*!< Macro range */
    rk_aiq_af_range_extended,                      /*!< Extended/full range */
} rk_aiq_af_range;

/*!
 * \brief Autofocus metering modes
 */
typedef enum
{
    rk_aiq_af_metering_mode_auto,                      /*!< Auto metering mode */
    rk_aiq_af_metering_modeouch                      /*!< Touch metering mode */
} rk_aiq_af_metering_mode;

typedef struct
{
    bool          enabled;
    unsigned char num_afm_win;
    rk_aiq_window afm_win[RK_AIQ_AFM_MAX_WINDOWS];
    unsigned int  thres;
    unsigned int  var_shift;
} rk_aiq_afc_cfg;

typedef struct
{
    unsigned int sum;
    unsigned int lum;
} rk_aiq_af_meas_val;

typedef struct
{
    rk_aiq_af_meas_val window[RK_AIQ_AFM_MAX_WINDOWS];
} rk_aiq_af_meas_stat;

/*!
 * \brief Detected scene mode.
 */
typedef enum
{
    rk_aiq_scene_mode_none                = 0,
    rk_aiq_scene_mode_close_up_portrait   = (1 << 0),
    rk_aiq_scene_mode_portrait            = (1 << 1),
    rk_aiq_scene_mode_lowlight_portrait   = (1 << 2),
    rk_aiq_scene_mode_low_light           = (1 << 3),
    rk_aiq_scene_mode_action              = (1 << 4),
    rk_aiq_scene_mode_backlight           = (1 << 5),
    rk_aiq_scene_mode_landscape           = (1 << 6),
    rk_aiq_scene_mode_document            = (1 << 7),
    rk_aiq_scene_mode_firework            = (1 << 8),
    rk_aiq_scene_mode_lowlight_action     = (1 << 9),
    rk_aiq_scene_mode_baby                = (1 << 10),
    rk_aiq_scene_mode_barcode             = (1 << 11)
} rk_aiq_scene_mode;

/*!
 * \brief Exposure parameters in terms of generic units.
 * Structure can be used as input or output from AEC.
 */
typedef struct
{
    int exposure_time_us;          /*!< Exposure time in microseconds, -1 if N/A. */
    float analog_gain;             /*!< Analog gain as a multiplier (e.g. 1.0), -1.0 if N/A. */
    float digital_gain;            /*!< Digital gain as a multiplier (e.g. 1.0), -1.0 if N/A. */
    int iso;                       /*!< ISO value corresponding to the analog gain. -1 if N/A. */
} rk_aiq_exposure_parameters;

/*!
 * \brief Exposure parameters in terms of sensor units.
 * Structure can be used as input or output from AEC.
 */
typedef struct
{
    unsigned short fine_integration_time;         /*!< Integration time specified as a number of pixel clocks added on top of coarse_integration_time. */
    unsigned short coarse_integration_time;       /*!< Integration time specified in multiples of pixel_periods_per_line.*/
    unsigned short analog_gain_code_global;       /*!< Global analog gain code. */
    unsigned short digital_gain_global;           /*!< Global digital gain code. */
    unsigned short line_length_pixels;            /*!< The number of pixels in one row. This includes visible lines and horizontal blanking lines. */
    unsigned short frame_length_lines;            /*!< The number of complete lines (rows) in the output frame. This includes visible lines and vertical blanking lines. */
} rk_aiq_exposure_sensor_parameters;

/*!
 * \brief Exposure related restrictions and constants in terms of sensor units.
 * Camera driver fills and updates these parameters whenever they are changed (for example in mode change).
 */
typedef struct
{
    float pixel_clock_freq_mhz;                        /*!< Video timing pixel clock frequency. */
    unsigned short pixel_periods_per_line;             /*!< The number of pixel clock periods in one line (row) time. This includes visible pixels and horizontal blanking time. */
    unsigned short line_periods_per_field;             /*!< The number of complete lines (rows) in the output frame. This includes visible lines and vertical blanking lines. */
    unsigned short line_periods_vertical_blanking;     /*!< Number of vertical blanking lines. Visible lines can be calculated using this and line_periods_per_field (above) value. */
    unsigned short fine_integration_time_min;          /*!< The minimum allowed value for fine_integration_time in AEC outputs. */
    unsigned short fine_integration_time_max_margin;   /*!< fine_integration_time_max = pixel_periods_per_line - fine_integration_time_max_margin. */
    unsigned short coarse_integration_time_min;        /*!< The minimum allowed value for coarse_integration_time in AEC outputs. */
    unsigned short coarse_integration_time_max_margin; /*!< coarse_integration_time_max = line_periods_per_field - coarse_integration_time_max_margin */
    unsigned short sensor_output_width;
    unsigned short sensor_output_height;
    unsigned short isp_input_width;
    unsigned short isp_input_height;
    unsigned short isp_output_width;
    unsigned short isp_output_height;
} rk_aiq_exposure_sensor_descriptor;

typedef enum
{
    rk_aiq_flash_mode_auto,
    rk_aiq_flash_mode_on,
    rk_aiq_flash_mode_off,
} rk_aiq_flash_mode;

/*!
 * \brief Flash status.
 */
typedef enum
{
    rk_aiq_flash_status_no,                /*!< No flash use. */
    rk_aiq_flash_status_torch,             /*!< Torch flash use. */
    rk_aiq_flash_status_pre,               /*!< Pre-flash use. */
    rk_aiq_flash_status_main,              /*!< Main flash use. */
    rk_aiq_flash_status_red_eye_reduction, /*!< Red Eye Reduction flash use. */
} rk_aiq_flash_status;

/*!
 * \brief Manual focus parameters.
 */
typedef struct
{
    unsigned int manual_focus_distance;               /*!< Manual focus distance in mm*/
    int manual_lens_position;                         /*!< Manual lens position */
    unsigned int manual_focal_distance;               /*!< Manual focal_distance in um (e.g. 4390) */
} rk_aiq_manual_focus_parameters;

/*!
 * \brief AWB scene modes
 * Used in AWB as input to restrict White Point between certain CCT range.
 * Note that not in all cases only CCT range is used to restrict White Point but more intelligent estimation may be used.
 */
typedef enum
{
    rk_aiq_awb_operation_mode_auto,
    rk_aiq_awb_operation_mode_daylight,           /*!< Restrict CCT range to [5000, 7000]. */
    rk_aiq_awb_operation_mode_partly_overcast,    /*!< Restrict CCT range to [5500, 9000]. */
    rk_aiq_awb_operation_mode_fully_overcast,     /*!< Restrict CCT range to [6000, 7000]. */
    rk_aiq_awb_operation_mode_fluorescent,        /*!< Restrict CCT range to [2700, 5500]. */
    rk_aiq_awb_operation_mode_incandescent,       /*!< Restrict CCT range to [2700, 3100]. */
    rk_aiq_awb_operation_mode_sunset,
    rk_aiq_awb_operation_mode_video_conference,
    rk_aiq_awb_operation_mode_manual_cct_range,   /*!< Use given CCT range (see rk_aiq_awb_manual_cct_range). */
    rk_aiq_awb_operation_mode_manual_white,       /*!< Use coordinate (see ia_coordinate) relative to full FOV which should be used as white point. */
    rk_aiq_awb_operation_mode_production_test,
    rk_aiq_awb_operation_mode_candlelight,
    rk_aiq_awb_operation_mode_flash,
    rk_aiq_awb_operation_mode_snow,
    rk_aiq_awb_operation_mode_beach,
} rk_aiq_awb_operation_mode;

/*!
 * \brief Manual CCT range
 */
typedef struct
{
    unsigned int min_cct;
    unsigned int max_cct;
} rk_aiq_awb_manual_cct_range;

/*!
 * \brief Values used in various operations for each color channels.
 * Value range depends on algorithm output.
 */
typedef struct
{
    float gr;               /*!< A value affecting Gr color channel. */
    float r;                /*!< A value affecting R color channel. */
    float b;                /*!< A value affecting B color channel. */
    float gb;               /*!< A value affecting Gb color channel. */
} rk_aiq_color_channels;

/*----------------------- AIQ related end ---------------------*/

/*----------------------- ISP sub modules related start ---------------------*/

typedef struct
{
    unsigned short lsc_r_data_tbl[RK_AIQ_LSC_DATA_TBL_SIZE];
    unsigned short lsc_gr_data_tbl[RK_AIQ_LSC_DATA_TBL_SIZE];
    unsigned short lsc_gb_data_tbl[RK_AIQ_LSC_DATA_TBL_SIZE];
    unsigned short lsc_b_data_tbl[RK_AIQ_LSC_DATA_TBL_SIZE];
    unsigned short lsc_data_tbl_cnt;

    unsigned short lsc_x_grad_tbl[RK_AIQ_LSC_GRAD_TBL_SIZE];
    unsigned short lsc_y_grad_tbl[RK_AIQ_LSC_GRAD_TBL_SIZE];
    unsigned short lsc_grad_tbl_cnt;
    unsigned short lsc_x_size_tbl[RK_AIQ_LSC_SIZE_TBL_SIZE];
    unsigned short lsc_y_size_tbl[RK_AIQ_LSC_SIZE_TBL_SIZE];
    unsigned short lsc_size_tbl_cnt;
} rk_aiq_isp_lsc_config;

typedef struct
{
    bool enabled;
    unsigned short config_width;
    unsigned short config_height;
    rk_aiq_isp_lsc_config lsc_config;
} rk_aiq_lsc_config;

typedef struct
{
    unsigned short red;
    unsigned short green;
    unsigned short blue;
} rk_aiq_xtalk_offset;

typedef enum
{
    RK_ISP_AWB_MEASURING_MODE_INVALID,
    RK_ISP_AWB_MEASURING_MODE_RGB,
    RK_ISP_AWB_MEASURING_MODE_YCBCR,
    RK_ISP_AWB_MEASURING_MODE_MAX,
} rk_aiq_isp_awb_measure_mode;

typedef struct
{
    unsigned char max_y;
    unsigned char ref_cr_max_r;
    unsigned char min_y_max_g;
    unsigned char ref_cb_max_b;
    unsigned char max_c_sum;
    unsigned char min_c;
} rk_aiq_isp_awb_measure_config;

typedef struct
{
    unsigned int num_white_pixel;
    unsigned char mean_y__g;
    unsigned char mean_cb__b;
    unsigned char mean_cr__r;
} rk_aiq_awb_meas_val;

typedef struct
{
    rk_aiq_awb_meas_val awb_meas[RK_AIQ_AWB_MAX_GRID];
} rk_aiq_awb_measure_result;

typedef struct
{
    bool enabled;
    rk_aiq_isp_awb_measure_mode awb_meas_mode;
    rk_aiq_isp_awb_measure_config awb_meas_cfg;
    rk_aiq_window awb_win;
} rk_aiq_awb_measure_config;

typedef struct
{
    unsigned short red_gain;
    unsigned short green_r_gain;
    unsigned short green_b_gain;
    unsigned short blue_gain;
} rk_aiq_gains;

typedef struct
{
    bool enabled;
    rk_aiq_gains awb_gains;
} rk_aiq_awb_gain_config;

typedef enum
{
    RK_AIQ_COLOR_COMPONENT_RED,
    RK_AIQ_COLOR_COMPONENT_GREENR,
    RK_AIQ_COLOR_COMPONENT_GREENB,
    RK_AIQ_COLOR_COMPONENT_BLUE,
    RK_AIQ_COLOR_COMPONENT_MAX,
} rk_aiq_color_comp;

typedef struct
{
    bool enabled;
    unsigned short isp_bls_a_fixed;
    unsigned short isp_bls_b_fixed;
    unsigned short isp_bls_c_fixed;
    unsigned short isp_bls_d_fixed;
    unsigned char num_win;
    rk_aiq_window window1;
    rk_aiq_window window2;
} rk_aiq_bls_config;

typedef struct
{
    bool enabled;
    unsigned int isp_dpcc_mode;
    unsigned int isp_dpcc_output_mode;
    unsigned int isp_dpcc_set_use;

    unsigned int isp_dpcc_methods_set_1;
    unsigned int isp_dpcc_methods_set_2;
    unsigned int isp_dpcc_methods_set_3;

    unsigned int isp_dpcc_line_thresh_1;
    unsigned int isp_dpcc_line_mad_fac_1;
    unsigned int isp_dpcc_pg_fac_1;
    unsigned int isp_dpcc_rnd_thresh_1;
    unsigned int isp_dpcc_rg_fac_1;

    unsigned int isp_dpcc_line_thresh_2;
    unsigned int isp_dpcc_line_mad_fac_2;
    unsigned int isp_dpcc_pg_fac_2;
    unsigned int isp_dpcc_rnd_thresh_2;
    unsigned int isp_dpcc_rg_fac_2;

    unsigned int isp_dpcc_line_thresh_3;
    unsigned int isp_dpcc_line_mad_fac_3;
    unsigned int isp_dpcc_pg_fac_3;
    unsigned int isp_dpcc_rnd_thresh_3;
    unsigned int isp_dpcc_rg_fac_3;

    unsigned int isp_dpcc_ro_limits;
    unsigned int isp_dpcc_rnd_offs;
} rk_aiq_dpcc_config;

typedef enum
{
    RK_ISP_FLT_STATIC_MODE,
    RK_ISP_FLT_DYNAMIC_MODE,
} rk_aiq_flt_mode;

typedef struct
{
    bool enabled;
    rk_aiq_flt_mode mode;
    unsigned char grn_stage1;
    unsigned char chr_h_mode;
    unsigned char chr_v_mode;
    unsigned int thresh_bl0;
    unsigned int thresh_bl1;
    unsigned int thresh_sh0;
    unsigned int thresh_sh1;
    unsigned int lum_weight;
    unsigned int fac_sh1;
    unsigned int fac_sh0;
    unsigned int fac_mid;
    unsigned int fac_bl0;
    unsigned int fac_bl1;
    int denoise_level;
    int sharp_level;
} rk_aiq_flt_config;

typedef struct
{
    unsigned short gamma_y[RK_AIQ_GAMMA_OUT_MAX_SAMPLES];
    unsigned char gamma_y_cnt;
} rk_aiq_gamma_curve;

typedef enum
{
    RK_ISP_SEGMENTATION_MODE_INVALID = -1,
    RK_ISP_SEGMENTATION_MODE_LOGARITHMIC,
    RK_ISP_SEGMENTATION_MODE_EQUIDISTANT,
    RK_ISP_SEGMENTATION_MODE_MAX,
} rk_aiq_gamma_seg_mode;

typedef struct
{
    bool enabled;
    rk_aiq_gamma_seg_mode mode;
    rk_aiq_gamma_curve gamma_y;
} rk_aiq_goc_config;

typedef struct
{
    bool enabled;
    /* segment array size, always equal red,green or blue array size + 1 */
    unsigned char seg_size;
    unsigned char segment[RK_AIQ_DEGAMMA_CURVE_SIZE - 1];
    unsigned short red[RK_AIQ_DEGAMMA_CURVE_SIZE];
    unsigned short green[RK_AIQ_DEGAMMA_CURVE_SIZE];
    unsigned short blue[RK_AIQ_DEGAMMA_CURVE_SIZE];
} rk_aiq_sdg_config;

typedef enum
{
    RK_ISP_QUANTIZATION_RANGE_OUT_INVALID,
    RK_ISP_QUANTIZATION_RANGE_OUT_BT601,
    RK_ISP_QUANTIZATION_RANGE_OUT_FULL_RANGE,
    RK_ISP_QUANTIZATION_RANGE_OUT_MAX,
} rk_aiq_isp_quantization_range;

typedef struct
{
    bool enabled;
    rk_aiq_isp_quantization_range ChromaOut;
    rk_aiq_isp_quantization_range LumaOut;
    rk_aiq_isp_quantization_range LumaIn;

    unsigned char contrast;
    unsigned char brightness;
    unsigned char saturation;
    unsigned char hue;
} rk_aiq_cproc_config;

typedef enum
{
    RK_ISP_ECM_MODE_INVALID,
    RK_ISP_ECM_MODE_1,
    RK_ISP_ECM_MODE_2,
    RK_ISP_ISP_ECM_MODE_MAX,
} rk_aiq_isp_ecm_mode;

typedef enum
{
    RK_ISP_EXP_MEASURING_MODE_INVALID = -1,
    RK_ISP_EXP_MEASURING_MODE_0,
    RK_ISP_EXP_MEASURING_MODE_1,
    RK_ISP_EXP_MEASURING_MODE_MAX,
} rk_aiq_isp_exp_measure_mode;

typedef enum
{
    RK_ISP_HIST_MODE_INVALID,
    RK_ISP_HIST_MODE_RGB_COMBINED,
    RK_ISP_HIST_MODE_R,
    RK_ISP_HIST_MODE_G,
    RK_ISP_HIST_MODE_B,
    RK_ISP_HIST_MODE_Y,
    RK_ISP_HIST_MODE_MAX,
} rk_aiq_isp_hist_mode;

typedef struct
{
    bool enabled;
    rk_aiq_isp_hist_mode mode;
    unsigned short stepSize;
    rk_aiq_window window;
    unsigned char weights[RK_AIQ_HISTOGRAM_WEIGHT_GRIDS_SIZE];
    unsigned char weights_cnt;
} rk_aiq_hist_config;

typedef struct
{
    bool enabled;
    rk_aiq_window win;
    rk_aiq_isp_exp_measure_mode mode;
} rk_aiq_aec_config;

typedef struct
{
    unsigned char exp_mean[RK_AIQ_AE_MEAN_MAX];
    unsigned char exp_mean_cnt;
    unsigned int hist_bin[RK_AIQ_HIST_BIN_N_MAX];
    unsigned char hist_bin_cnt;
} rk_aiq_aec_measure_result;

typedef enum
{
    RK_ISP_IE_MODE_INVALID,
    RK_ISP_IE_MODE_GRAYSCALE,
    RK_ISP_IE_MODE_NEGATIVE,
    RK_ISP_IE_MODE_SEPIA,
    RK_ISP_IE_MODE_COLOR,
    RK_ISP_IE_MODE_EMBOSS,
    RK_ISP_IE_MODE_SKETCH,
    RK_ISP_IE_MODE_SHARPEN,
    RK_ISP_IE_MODE_MAX,
} rk_aiq_isp_ie_mode;

typedef enum
{
    RK_ISP_IE_COLOR_SELECTION_INVALID,
    RK_ISP_IE_COLOR_SELECTION_RGB,
    RK_ISP_IE_COLOR_SELECTION_B,
    RK_ISP_IE_COLOR_SELECTION_G,
    RK_ISP_IE_COLOR_SELECTION_GB,
    RK_ISP_IE_COLOR_SELECTION_R,
    RK_ISP_IE_COLOR_SELECTION_RB,
    RK_ISP_IE_COLOR_SELECTION_RG,
    RK_ISP_IE_COLOR_SELECTION_MAX,
} rk_aiq_isp_ie_color_sel;

typedef struct
{
    bool enabled;
    rk_aiq_isp_ie_mode mode;
    rk_aiq_isp_quantization_range range;

    union mode_config_u {
        struct
        {
            unsigned char tint_cb;
            unsigned char tint_Cr;
        } sepia;

        struct
        {
            rk_aiq_isp_ie_color_sel col_selection;
            unsigned char col_threshold;
        } color_selection;

        struct
        {
            int8_t coeff[9];
        } emboss;

        struct
        {
            int8_t coeff[9];
        } sketch;

        struct
        {
            unsigned char factor;
            unsigned char threshold;
            char coeff[9];
        } sharpen;

    } mode_config;

} rk_aiq_ie_config;

typedef struct
{
    bool enabled;
    rk_aiq_3x3Matrix ctk_matrix;
    rk_aiq_xtalk_offset cc_offset;
} rk_aiq_ctk_config;

typedef struct
{
    bool enabled;
    unsigned char demosaic_th;
} rk_aiq_bdm_config;

typedef enum
{
    RK_ISP_WDR_MODE_GLOBAL,
    RK_ISP_WDR_MODE_BLOCK
} rk_aiq_isp_wdr_mode;

typedef struct
{
    bool enabled;
    rk_aiq_isp_wdr_mode mode;
    unsigned char segment[RK_AIQ_WDR_MAX_CURVE_SIZE - 1];
    unsigned char segment_cnt;
    unsigned short wdr_global_y[RK_AIQ_WDR_MAX_CURVE_SIZE];
    unsigned short wdr_block_y[RK_AIQ_WDR_MAX_CURVE_SIZE];;
    unsigned short wdr_noiseratio;
    unsigned short wdr_bestlight;
    unsigned int wdr_gain_off1;
    unsigned short wdr_pym_cc;
    unsigned char wdr_epsilon;
    unsigned char wdr_lvl_en;
    unsigned char wdr_flt_sel;
    unsigned char wdr_gain_max_clip_enable;
    unsigned char wdr_gain_max_value;
    unsigned char wdr_bavg_clip;
    unsigned char wdr_nonl_segm;
    unsigned char wdr_nonl_open;
    unsigned char wdr_nonl_mode1;
    unsigned int wdr_coe0;
    unsigned int wdr_coe1;
    unsigned int wdr_coe2;
    unsigned int wdr_coe_off;
} rk_aiq_isp_wdr_config;

typedef struct
{
    unsigned char weight_r;
    unsigned char weight_g;
    unsigned char weight_b;
} rk_aiq_isp_dpf_inv_strength;

/* dpf noise level lookup scale */
typedef enum
{
    RK_AIQ_ISP_DPF_NLL_SCALE_INVALID = -1,             /**< lower border (only for an internal evaluation) */
    RK_AIQ_ISP_DPF_NLL_SCALE_LINEAR,                   /**< use a linear scaling */
    RK_AIQ_ISP_DPF_NLL_SCALE_LOGARITHMIC,              /**< use a logarithmic scaling */
    RK_AIQ_ISP_DPF_NLL_SCALE_MAX,                      /**< upper border (only for an internal evaluation) */
} rk_aiq_isp_dpf_nll_scale;

typedef struct
{
    unsigned short nll_coeff[RK_AIQ_DPF_MAX_NLF_COEFFS];  /**< Noise-Level-Lookup coefficients */
    unsigned char nll_coeff_size;
    rk_aiq_isp_dpf_nll_scale x_scale;                         /**< type of x-axis (logarithmic or linear type) */
} rk_aiq_isp_dpf_noise_level_lookup;

typedef struct
{
    unsigned char weight_coeff[RK_AIQ_DPF_MAX_SPATIAL_COEFFS];
    unsigned char weight_coeff_size;
} rk_aiq_isp_dpf_spatial;

typedef enum
{
    RK_AIQ_ISP_DPF_GAIN_USAGE_INVALID,             /**< lower border (only for an internal evaluation) */
    RK_AIQ_ISP_DPF_GAIN_USAGE_DISABLED,            /**< don't use any gains in preprocessing stage */
    RK_AIQ_ISP_DPF_GAIN_USAGE_NF_GAINS,            /**< use only the noise function gains  from registers DPF_NF_GAIN_R, ... */
    RK_AIQ_ISP_DPF_GAIN_USAGE_LSC_GAINS,           /**< use only the gains from LSC module */
    RK_AIQ_ISP_DPF_GAIN_USAGE_NF_LSC_GAINS,        /**< use the moise function gains and the gains from LSC module */
    RK_AIQ_ISP_DPF_GAIN_USAGE_AWB_GAINS,           /**< use only the gains from AWB module */
    RK_AIQ_ISP_DPF_GAIN_USAGE_AWB_LSC_GAINS,       /**< use the gains from AWB and LSC module */
    RK_AIQ_ISP_DPF_GAIN_USAGE_MAX,                 /**< upper border (only for an internal evaluation) */
} rk_aiq_isp_dpf_gain_usage;

typedef enum
{
    RK_AIQ_ISP_DPF_RB_FILTERSIZE_INVALID = -1,     /**< lower border (only for an internal evaluation) */
    RK_AIQ_ISP_DPF_RB_FILTERSIZE_9x9,              /**< red and blue filter kernel size 9x9 (means 5x5 active pixel) */
    RK_AIQ_ISP_DPF_RB_FILTERSIZE_13x9,             /**< red and blue filter kernel size 13x9 (means 7x5 active pixel) */
    RK_AIQ_ISP_DPF_RB_FILTERSIZE_MAX,              /**< upper border (only for an internal evaluation) */
} rk_aiq_isp_dpf_rb_flt_size;

typedef struct
{
    bool enabled;
    rk_aiq_gains nf_ains;
    rk_aiq_isp_dpf_noise_level_lookup lookup;
    rk_aiq_isp_dpf_gain_usage gain_usage;
    rk_aiq_isp_dpf_rb_flt_size rb_flt_size;
    rk_aiq_isp_dpf_spatial spatial_g;
    rk_aiq_isp_dpf_spatial spatial_rb;
    bool process_red_pixel;
    bool process_blue_pixel;
    bool process_gr_pixel;
    bool process_gb_pixel;
} rk_aiq_dpf_config;

typedef struct
{
    bool enabled;
    unsigned char r;
    unsigned char g;
    unsigned char b;
} rk_aiq_dpf_strength_config;
/*----------------------- ISP sub modules related end ---------------------*/

/*------------------------ algorithm result start ------------------------------- */

/*!
 * \brief AEC results.
 */
typedef struct
{
    rk_aiq_exposure_parameters exposure;
    rk_aiq_exposure_sensor_parameters sensor_exposure;
    bool converged;
    rk_aiq_ae_flicker_reduction flicker_reduction_mode; /*!< Flicker reduction mode proposed by the AEC algorithm */
    rk_aiq_aec_config aec_config_result;
    rk_aiq_hist_config hist_config_result;
    /* TODO: add flash params, hdr params */
} rk_aiq_ae_results;

/*!
 * \brief Results from AWB.
 */
typedef struct
{
    rk_aiq_awb_measure_config awb_meas_cfg;
    rk_aiq_awb_gain_config awb_gain_cfg;
    rk_aiq_ctk_config ctk_config;
    rk_aiq_lsc_config lsc_cfg;
    bool converged;
} rk_aiq_awb_results;

/*!
 * \brief Autofocus algorithm results
 */
typedef struct
{
    rk_aiq_afc_cfg afc_config;
    rk_aiq_af_status status;                           /*!< Focus status */
    unsigned short current_focus_distance;             /*!< Current focusing distance in mm */
    int next_lens_position;                            /*!< Next lens position */
    bool final_lens_position_reached;                  /*!< Lens has reached the final lens position */
} rk_aiq_af_results;

/*!
 * \brief Results from GBCE.
 */
typedef struct
{
    rk_aiq_cproc_config cproc_config;
    rk_aiq_goc_config goc_config;
    rk_aiq_ie_config ie_config;
} rk_aiq_gbce_results;

/*!
 * \brief Results from MISC.
 */
typedef struct
{
    rk_aiq_bls_config bls_config;
    rk_aiq_dpcc_config dpcc_config;
    rk_aiq_flt_config flt_config;
    rk_aiq_bdm_config bdm_config;
    rk_aiq_gbce_results gbce_config;
    rk_aiq_dpf_config dpf_config;
    rk_aiq_dpf_strength_config strength_config;
    rk_aiq_isp_wdr_config wdr_config;
    rk_aiq_sdg_config sdg_config;
} rk_aiq_misc_isp_results;

/*------------------------ algorithm result end ------------------------------- */

#ifdef __cplusplus
}
#endif

#endif //__RK_AIQ_TYPES_H__
