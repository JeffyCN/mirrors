/*
 * Copyright 2014 Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef IA_ISP_CIF_2_0_H_
#define IA_ISP_CIF_2_0_H_

#include "ia_mkn_types.h"
#include "ia_abstraction.h"
#include "ia_aiq_types.h"
#include "ia_cmc_types.h"
#include "ia_isp_cif_2_0_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IA_CIF_ISP_VERSION "v2.0_007.010"

typedef struct ia_isp_t ia_isp;

/*!
 * \brief Initialize IA_ISP and its submodules.
 * This function must be called before any other function in the library. It allocates memories and parses ISP specific parts from CPFF.
 * Initialization returns a handle to the ISP instance, which is given as input parameter for all the
 * ISP functions.
 *
 * \param[in]     aiqb_data        Mandatory although function will not return error, if it not given.\n
 *                                 ISP Block from CPFF. Contains ISP specific parameters for ISP 1.5.
 * \param[in]     stats_max_width  Mandatory.\n
 *                                 Maximum width of RGBS and AF statistics grids from ISP. Used to calculate size of
 *                                 memory buffers for the IA_AIQ algorithms. The same maximum width will be used for all RGBS
 *                                 and AF statistics grid allocations.
 * \param[in]     stats_max_height Mandatory.\n
 *                                 Maximum height of RGBS and AF statistics grids from ISP. Used to calculate size of
 *                                 memory buffers for the IA_AIQ algorithms. The same maximum height will be used for all RGBS
 *                                 and AF statistics grid allocations.
 * \param[in]     ia_cmc           Mandatory.\n
 *                                 Parsed camera module characterization structure. Essential parts of the structure will be copied
 *                                 into internal structure.
 * \param[in,out] ia_mkn           Optional.\n
 *                                 Makernote handle which can be initialized with ia_mkn library. If debug data from AIQ is needed
 *                                 to be stored into EXIF, this parameter is needed. Algorithms will update records inside this makernote instance.
 *                                 Client writes the data into Makernote section in EXIF.
 * return                          IA_AIQ handle. Use the returned handle as input parameter for the consequent IA_AIQ calls.
 */
LIBEXPORT ia_isp*
ia_isp_cif_2_0_init(const ia_binary_data *aiqb_data,
        unsigned int stats_max_width,
        unsigned int stats_max_height,
        ia_cmc_t *ia_cmc,
        ia_mkn *ia_mkn);

/*!
 * \brief De-initialize IA_ISP.
 * All memory allocated by ISP are freed. ISP handle can no longer be used.
 *
 * \param[in] ia_isp               Mandatory.\n
 *                                 ISP instance handle.
 */
LIBEXPORT void
ia_isp_cif_2_0_deinit(ia_isp *ia_isp);

/*!
 *  \brief Definitions for the color effects.
 */
typedef enum
{
    ia_isp_effect_none     =               0,
    ia_isp_effect_sky_blue =         (1 << 0),
    ia_isp_effect_grass_green =      (1 << 1),
    ia_isp_effect_skin_whiten_low =  (1 << 2),
    ia_isp_effect_skin_whiten =      (1 << 3),
    ia_isp_effect_skin_whiten_high = (1 << 4),
    ia_isp_effect_sepia =            (1 << 5),
    ia_isp_effect_black_and_white =  (1 << 6),
    ia_isp_effect_negative =         (1 << 7),
    ia_isp_effect_vivid =            (1 << 8),
    ia_isp_effect_invert_gamma =     (1 << 9),
    ia_isp_effect_grayscale =        (1 << 10)
} ia_isp_cif_2_0_effect;

typedef struct
{
    char manual_brightness;                          /*!< Optional */
    float manual_contrast;                           /*!< Optional */
    float manual_hue;                                 /*!< Optional */
    float manual_saturation;                          /*!< Optional */
    float manual_sharpness;
} ia_isp_cif_2_0_manual_config;

/*!
 *  \brief Input parameter structure for ISP.
 */
typedef struct
{
    ia_aiq_frame_use frame_use;                      /*!< Mandatory. Target frame type of the AIC calculations (Preview, Still, video etc.). */
    ia_aiq_frame_params *sensor_frame_params;        /*!< Mandatory. Sensor frame parameters. Describe frame scaling/cropping done in sensor. */
    ia_aiq_exposure_parameters *exposure_results;    /*!< Mandatory. Exposure parameters which are to be used to calculate next ISP parameters. */
    ia_aiq_awb_results *awb_results;                 /*!< Mandatory. WB results which are to be used to calculate next ISP parameters (WB gains, color matrix,etc). */
    ia_aiq_gbce_results *gbce_results;               /*!< Mandatory. GBCE Gamma tables which are to be used to calculate next ISP parameters.
                                                          If NULL pointer is passed, AIC will use static gamma table from the CPF.  */
    ia_aiq_pa_results *pa_results;                   /*!< Mandatory. Parameter adaptor results from AIQ. */
    ia_isp_cif_2_0_manual_config *manual_config;
    ia_isp_cif_2_0_effect effects;                   /*!< Optional. Manual setting for special effects.*/
    uint16_t isp_input_width;
    uint16_t isp_input_height;
    ia_aiq_sa_results *sa_results;                   /*!< Mandatory. Shading adaptor results from AIQ. */
    ia_rectangle af_windows[CIFISP_AFM_MAX_WINDOWS];
    int num_of_af_win;
} ia_isp_cif_2_0_input_params;

/*!
 * \brief ISP configuration for the next frame
 * Computes ISP parameters from input parameters and CPF values for the next image.
 *
 * \param[in] ia_isp                   Mandatory.\n
 *                                     ISP instance handle.
 * \param[in] ia_isp_cif_2_0_input_params  Mandatory.\n
 *                                     Input parameters for ISP calculations.
 * \return                             Binary data structure with pointer to the ISP configuration structure.
 */
LIBEXPORT ia_err
ia_isp_cif_2_0_run(const ia_isp *ia_isp,
               const ia_isp_cif_2_0_input_params *isp_input_params,
               struct ia_cif_isp_2_0_config *output_data);


/*!
 * \brief Converts ISP specific statistics to IA_AIQ format.
 * ISP generated statistics may not be in the format in which AIQ algorithms expect. Statistics need to be converted
 * from various ISP formats into AIQ statistics format.
 *
 * \param[in]  ia_isp        Mandatory.\n
 *                           ISP instance handle.
 * \param[in]  statistics    Mandatory.\n
 *                           Statistics in ISP specific format.
 * \param[out] out_rgbs_grid Mandatory.\n
 *                           Pointer's pointer where address of converted statistics are stored.
 *                           Converted RGBS grid statistics. Output can be directly used as input in function ia_aiq_statistics_set.
 * \param[out] out_af_grid   Mandatory.\n
 *                           Pointer's pointer where address of converted statistics are stored.
 *                           Converted AF grid statistics. Output can be directly used as input in function ia_aiq_statistics_set.
 * \return                   Error code.
 */
LIBEXPORT ia_err
ia_isp_cif_2_0_statistics_convert(ia_isp *ia_isp,
    void *statistics,
    ia_aiq_rgbs_grid **out_rgbs_grid,
    ia_aiq_af_grid **out_af_grid);

/*!
 * \brief Converts AWB ISP specific statistics to IA_AIQ format.
 * ISP generated statistics may not be in the format in which AIQ algorithms
 * expect. Statistics need to be converted from various ISP formats
 *  into AIQ statistics format. This method converts only the AWB statistics
 *  that generate the RGBS grid. It uses as destination the memory provided
 *  by the client.
 *
 * \param[in]  ia_isp        Mandatory.\n
 *                           ISP instance handle.
 * \param[in]  statistics    Mandatory.\n
 *                           Statistics in ISP specific format.
 * \param[out] out_rgbs_grid Mandatory.\n
 *                           Pointer's pointer where address of converted statistics are stored.
 *                           Converted RGBS grid statistics. Output can be directly used as input in function ia_aiq_statistics_set.
 * \return                   Error code.
 */
LIBEXPORT ia_err
ia_isp_cif_2_0_statistics_convert_awb(ia_isp *ia_isp,
                                      void *statistics,
                                      ia_aiq_rgbs_grid *out_rgbs_grid);
/*!
 * \brief Converts AF ISP specific statistics to IA_AIQ format.
 * ISP generated statistics may not be in the format in which AIQ algorithms
 * expect. Statistics need to be converted from various ISP formats
 *  into AIQ statistics format. This method converts only the AF statistics
 *  that generate the filter responses the AF algorithm uses.
 *  Please note that it uses as destination the memory provided  by the client.
 *
 * \param[in]  ia_isp        Mandatory.\n
 *                           ISP instance handle.
 * \param[in]  statistics    Mandatory.\n
 *                           Statistics in ISP specific format.
 * \param[out] ia_aiq_af_grid Mandatory.\n
 *                           Pointer's pointer where address of converted statistics are stored.
 *                           Converted AF statistics. Output can be directly used as input in function ia_aiq_statistics_set.
 * \return                   Error code.
 */
LIBEXPORT ia_err
ia_isp_cif_2_0_statistics_convert_af(ia_isp *ia_isp,
                                     void *statistics,
                                     ia_aiq_af_grid *out_af_grid);
#ifdef __cplusplus
}
#endif

#endif /* IA_ISP_CIF_2_0_H_ */
