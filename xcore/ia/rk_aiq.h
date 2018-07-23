/*
 * Copyright (C) 2015 - 2017 Intel Corporation.
 * Copyright (c) 2017, Fuzhou Rockchip Electronics Co., Ltd
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

#ifndef __RK_AIQ_H__
#define __RK_AIQ_H__

#include "rk_aiq_types.h"

#define LIBEXPORT

#ifdef __cplusplus
extern "C" {
#endif

#define rk_aiq_ae_run rk_aiq_aec_run
typedef struct rk_aiq_ctx_s rk_aiq;

/*!
 * \brief Initialize RK_AIQ and its submodules.
 * This function must be called before any other function in the library. It allocates memories for all AIQ algorithms based on input parameters
 * given by the user. AIQB (from CPFF) and NVM data are parsed and combined resulting camera module specific tuning parameters which the
 * AIQ algorithms use. Initialization returns a handle to the AIQ instance, which is given as input parameter for all the
 * algorithms. Therefore, multiple instances of AIQ library can running simultaneously. For example one instance per camera.
 *
 * \param[in] xml_file_path         Mandatory.
 * return                           RK_AIQ handle. Use the returned handle as input parameter for the consequent RK_AIQ calls.
 */
LIBEXPORT rk_aiq*
rk_aiq_init(const char *xml_file_path);

/*!
 * \brief De-initialize RK_AIQ and its submodules.
 * All memory allocated by AIQ algoriths are freed. AIQ handle can no longer be used.
 *
 * \param[in] rk_aiq                Mandatory.\n
 *                                  AIQ instance handle.
 */
LIBEXPORT void
rk_aiq_deinit(rk_aiq *ctx);

/*!
 * \brief Manual AEC limit parameters.
 */
typedef struct
{
    int manual_exposure_time_min;             /*!< Optional. Minimum exposure time in microseconds. -1 if NA. */
    int manual_exposure_time_max;             /*!< Optional. Maximum exposure time in microseconds. -1 if NA. */
    int manual_frame_time_us_min;             /*!< Optional. Manual minimum frame length in microseconds. Defines maximum frame rate -1 if NA. */
    int manual_frame_time_us_max;             /*!< Optional. Manual maximum frame length in microseconds. Defines minimum frame rate. -1 if NA. */
    short manual_iso_min;                     /*!< Optional. Manual minimum ISO. -1 if NA. */
    short manual_iso_max;                     /*!< Optional. Manual maximum ISO. -1 if NA. */
} rk_aiq_ae_manual_limits;

/*!
 *  \brief Input parameter structure for AE algorithm.
 */
typedef struct
{
    unsigned int num_exposures;                                 /*!< Mandatory. The number of exposure outputs to have. Must be positive. One for LDR, two or more for HDR/exposure bracketing. */
    rk_aiq_frame_use frame_use;                                 /*!< Mandatory. Target frame type of the AEC calculations (Preview, Still, video etc.). */
    rk_aiq_flash_mode flash_mode;                               /*!< Mandatory. Manual flash mode. If AEC should make flash decision, set mode to rk_aiq_flash_mode_auto. */
    rk_aiq_ae_operation_mode operation_mode;                    /*!< Mandatory. AEC operation mode. */
    rk_aiq_ae_metering_mode metering_mode;                      /*!< Mandatory. AEC metering mode. */
    rk_aiq_ae_priority_mode priority_mode;                      /*!< Mandatory. AEC priority mode. */
    rk_aiq_ae_flicker_reduction flicker_reduction_mode;         /*!< Mandatory. AEC flicker reduction mode. */
    rk_aiq_exposure_sensor_descriptor *sensor_descriptor;       /*!< Mandatory although function will not return error, if not given.
                                                                     Sensor specific descriptor and limits of the used sensor mode for target frame use.
                                                                     AEC will not limit and calculate sensor specific parameters, if not given */
    rk_aiq_ae_manual_limits *manual_limits;                     /*!< Optional. Manual limits which override limits defined in AEC tunings. */
    rk_aiq_window *window;                                      /*!< Optional. standard coordinate <-1000, 1000>*/
    float ev_shift;                                             /*!< Optional. Exposure Value shift [-4,4]. */
    long *manual_exposure_time_us;                              /*!< Optional. Manual exposure time in microseconds. NULL if NA. Otherwise, array of values
                                                                     of num_exposures length. Order of exposure times corresponds to exposure_index of ae_results,
                                                                     e.g., manual_exposure_time_us[ae_results->exposures[0].exposure_index] = 33000; */
    float *manual_analog_gain;                                  /*!< Optional. Manual analog gain. NULL if NA. Otherwise, array of values of num_exposures length.
                                                                     Order of gain values corresponds to exposure_index of ae_results,
                                                                     e.g., manual_analog_gain[ae_results->exposures[0].exposure_index] = 4.0f; */
    short *manual_iso;                                          /*!< Optional. Manual ISO. Overrides manual_analog_gain. NULL if NA. Otherwise, array of values
                                                                     e.g., manual_iso[ae_results->exposures[0].exposure_index] = 100; */
} rk_aiq_ae_input_params;

/*!
 * \brief AEC calculation based on input parameters and frame statistics.
 * AE calculates new exposure parameters to be used for the next frame based on previously given statistics and user parameters.
 *
 * \param[in] rk_aiq                Mandatory.\n
 *                                  AIQ instance handle.
 * \param[in] ae_input_params       Mandatory.\n
 *                                  Input parameters for AEC calculations.
 * \param[out] ae_results           Mandatory.\n
 *                                  Pointer's pointer where address of ISP parameters are stored.
 *                                  Results from AEC calculations. Results can be used directly as input for AIC.
 * \return                          Error code.
 */
LIBEXPORT int
rk_aiq_ae_run(rk_aiq *ctx,
              const rk_aiq_ae_input_params *aec_input_params,
              rk_aiq_ae_results *aec_result);

/*!
 *  \brief Input parameter structure for AF algorithm.
 */
typedef struct
{
    rk_aiq_frame_use frame_use;                                 /*!< Mandatory. Target frame type of the AWB calculations (Preview, Still, video etc.). */
    int lens_position;                                          /*!< Mandatory. Current lens position. */
    unsigned long long lens_movement_start_timestamp;           /*!< Mandatory. Lens movement start timestamp in us. Timestamp is compared against statistics timestamp
                                                                     to determine if lens was moving during statistics collection. */
    rk_aiq_af_operation_mode focus_mode;                        /*!< Mandatory. Focusing mode. */
    rk_aiq_af_range focus_range;                                /*!< Mandatory. Focusing range. Only valid when focus_mode is rk_aiq_af_operation_mode_auto. */
    rk_aiq_af_metering_mode focus_metering_mode;                /*!< Mandatory. Metering mode (multispot, touch). */
    rk_aiq_flash_mode flash_mode;                               /*!< Mandatory. User setting for flash. */
    rk_aiq_window *focus_rect;                                  /*!< Optional. */
    rk_aiq_manual_focus_parameters *manual_focus_parameters;    /*!< Optional. Manual focus parameters (manual lens position, manual focusing distance). Used only if
                                                                     focus mode 'rk_aiq_af_operation_mode_manual' is used. */
    bool trigger_new_search;                                    /*!< TRUE if new AF search is needed, FALSE otherwise. Host is responsible for flag cleaning. */
} rk_aiq_af_input_params;

/*!
 * \brief AF calculation based on input parameters and frame statistics.
 * AF calculates new lens position based on statistics and given input parameters.
 *
 * \param[in] rk_aiq                Mandatory.\n
 *                                  AIQ instance handle.
 * \param[in] af_input_params       Mandatory.\n
 *                                  Input parameters for AF calculations.
 * \param[out] af_results           Mandatory.\n
 *                                  Pointer's pointer where address of AF results are stored.
 *                                  Results from AF calculations.
 * \return                          Error code.
 */
LIBEXPORT int
rk_aiq_af_run(rk_aiq *ctx,
              const rk_aiq_af_input_params *af_input_params,
              rk_aiq_af_results *af_result);

/*!
 *  \brief Input parameter structure for AWB algorithm.
 */
typedef struct
{
    rk_aiq_frame_use frame_use;                       /*!< Mandatory. Target frame type of the AWB calculations (Preview, Still, video etc.). */
    rk_aiq_awb_operation_mode scene_mode;             /*!< Mandatory. AWB scene mode. */
    rk_aiq_awb_manual_cct_range *manual_cct_range;    /*!< Optional. Manual CCT range. Used only if AWB scene mode 'rk_aiq_awb_operation_manual_cct_range' is used. */
    rk_aiq_window *window;                            /*!< Optional. standard coordinate <-1000, 1000>*/
} rk_aiq_awb_input_params;

/*!
 * \brief AWB calculation based on input parameters and frame statistics.
 *
 * \param[in] rk_aiq                Mandatory.\n
 *                                  AIQ instance handle.
 * \param[in] awb_input_params      Mandatory.\n
 *                                  Input parameters for AWB calculations.
 * \param[out] awb_results          Mandatory.\n
 *                                  Pointer's pointer where address of AWB results are stored.
 *                                  Results from AWB calculations. Results can be used directly as input for ia_isp.
 * \return                          Error code.
 */
LIBEXPORT int
rk_aiq_awb_run(rk_aiq *ctx,
               const rk_aiq_awb_input_params *awb_input_params,
               rk_aiq_awb_results *awb_result);

/*!
 *  \brief Input parameter structure for MISC algorithm.
 * Including contrast, hue, brightness, etc.
 */
typedef struct
{
    /* TODO */
    void *TODO;
} rk_aiq_misc_isp_input_params;

/*!
 * \brief MISC calculation based on input parameters and frame statistics.
 * Including contrast, hue, brightness, etc.
 *
 * \param[in] rk_aiq                        Mandatory.\n
 *                                          AIQ instance handle.
 * \param[in] misc_isp_input_params         Mandatory.\n
 *                                          Input parameters for MISC calculations.
 * \param[out] misc_isp_results             Mandatory.\n
 *                                          Pointer's pointer where address of MISC results are stored. Results can be used directly as input for ia_isp.
 * \return                                  Error code.
 */
LIBEXPORT int
rk_aiq_misc_run(rk_aiq *ctx,
                const rk_aiq_misc_isp_input_params *misc_input_params,
                rk_aiq_misc_isp_results *misc_results);

/*!
 *  \brief Input parameter structure for setting the statistics.
 */
typedef struct
{
    unsigned long long frame_id;                                /*!< The frame identifier which identifies to which frame the given statistics correspond. Must be positive. */
    unsigned long long frame_timestamp;                         /*!< Mandatory although function will not return error, if not given.*/
    rk_aiq_awb_measure_result awb_stats;
    rk_aiq_aec_measure_result aec_stats;
    rk_aiq_af_meas_stat af_stats;

    rk_aiq_ae_results *ae_results;
    rk_aiq_awb_results *awb_results;
    rk_aiq_af_results *af_results;
    rk_aiq_misc_isp_results *misc_results;
} rk_aiq_statistics_input_params;

/*!
 * \brief Set input statistics and information about the captured image.
 * AIQ algorithms need various information about the conditions in which the frame and statistics were captured in order to
 * calculate new parameters.
 *
 * \param[in] rk_aiq                        Mandatory.\n
 *                                          AIQ instance handle.
 * \param[in] statistics_input_params       Optional.\n
 *                                          Input parameters containing statistics and information about a frame.
 * \param[in] sensor_desc                   Optional.\n
 * \return                                  Error code.
 */
LIBEXPORT int
rk_aiq_stats_set(rk_aiq *ctx,
                 const rk_aiq_statistics_input_params *stats,
                 const rk_aiq_exposure_sensor_descriptor *sensor_desc);

#ifdef __cplusplus
}
#endif

#endif //__RK_AIQ_H__
