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
#ifndef __PVL_PANORAMA_H__
#define __PVL_PANORAMA_H__

/** @file    pvl_panorama.h
 *  @brief   This file declares the structures and native APIs of panorama component.
 */

#include "pvl_types.h"
#include "pvl_config.h"

#ifdef __cplusplus
extern "C" {
#endif


/** @brief The enumerated values to specify the 1D panning direction of panoramic stitch.
 */
enum pvl_panorama_direction {
    pvl_panorama_direction_still,           /**< No direction - still */
    pvl_panorama_direction_right,           /**< Left to right panning */
    pvl_panorama_direction_left,            /**< Right to left panning */
    pvl_panorama_direction_down,            /**< Downward panning */
    pvl_panorama_direction_up,              /**< Upward panning */
    pvl_panorama_direction_preview_auto     /**< The direction of panning is automatically detected in preview mode. */
};
typedef enum pvl_panorama_direction pvl_panorama_direction;


/** @brief A structure to hold run-time configurable parameters for this component.
 *
 *  The variables in this structure will be retrieved and assigned, via pvl_panorama_get_parameters() and pvl_panorama_set_parameters() respectively.
 */
struct pvl_panorama_parameters {
    uint32_t overlapping_ratio;         /**< The expected overlapping ratio between adjacent input images (in percent).
                                             Smaller value results in larger FoV and also larger output image and vice versa.
                                             In addition, however, larger value tends to result in more accurate alignments.
                                             The min/max and default values are defined in pvl_panorama structure in runtime. */

    pvl_panorama_direction direction;   /**< The assumed panning direction of input images. If pvl_panorama_direction_preview_auto is set
                                             the direction would be automatically estimated while processing preview.(See pvl_parnorama_detect_frame_to_stitch()
                                             Otherwise, the direction is assumed to be explicitly specified. */
};
typedef struct pvl_panorama_parameters pvl_panorama_parameters;


/** @brief A structure to hold the run-time context of this component.
 *
 *  This structure represents the panorama instance which is used as the handle over most of API.
 *  It holds its own properties, constant parameters and internal context inside.
 */
struct pvl_panorama {
    const pvl_version version;                  /**< The version information. */

    const uint32_t max_supported_num_images;    /**< The maximum number of input images supported by this component. */

    const uint32_t min_overlapping_ratio;       /**< The minimum configurable value of overlapping ratio (in percent) */
    const uint32_t max_overlapping_ratio;       /**< The maximum configurable value of overlapping ratio. (in percent) */
    const uint32_t default_overlapping_ratio;   /**< The default value of overlapping ratio. (in percent) */
};
typedef struct pvl_panorama pvl_panorama;


/** @brief Get default configuration of this component.
 *
 *  This function returns default configuration of the panorama component.
 *  The returned configuration could be customized as per its usage.
 *
 *  @param[out] config  The structure to load default configuration.
 *
 *  @return     On success, @ref pvl_success.
 *  @return     On failure, @ref pvl_err error code, which will be the one of the following return value(s).
 *
 *  @retval     pvl_err_invalid_argument    Passing null pointer to the params.
 */
PVLIB_EXPORT pvl_err
pvl_panorama_get_default_config(pvl_config *config);


/** @brief Create an instance of the panorama component.
 *
 *  This function initializes and returns an instance of this component.
 *  Multiple instances are allowed to be created concurrently.
 *
 *  @param[in]  config  The configuration information of the component.
 *  @param[out] pano    A pointer to indicate the handle newly created.
 *
 *  @return     On success, @ref pvl_success.
 *  @return     On failure, @ref pvl_err error code, which will be the one of the following return value(s).
 *
 *  @retval     pvl_err_not_supported       Unsupported configuration.
 *  @retval     pvl_err_invalid_argument    Passing null pointer to the params.
 *  @retval     pvl_err_invalid_status      Invalid internal status of the handle.
 *  @retval     pvl_err_nomem               Failed to allocate the memory.
 */
PVLIB_EXPORT pvl_err
pvl_panorama_create(const pvl_config *config, pvl_panorama **pano);


/** @brief Destroy the instance of this component.
 *
 *  @param[in]  pano The handle of this component to be destroyed.
 */
PVLIB_EXPORT void
pvl_panorama_destroy(pvl_panorama *pano);


/** @brief Reset the instance of this component.
 *
 *  All the internal states, the stitched images, the composed output image and context will be reset except the run-time parameters set by user.
 *  If there are any ongoing processes(maybe on another thread) it cancels them or waits until done.
 *
 *  @param[in]  pano    The handle of this component.
 *
 *  @return     On success, @ref pvl_success.
 *  @return     On failure, @ref pvl_err error code, which will be the one of the following return value(s).
 *
 *  @retval     pvl_err_invalid_argument    Passing null pointer to the params.
 *  @retval     pvl_err_invalid_status      Invalid internal status of the handle.
 *  @retval     pvl_err_nomem               Failed to re-allocate the memory.
 */
PVLIB_EXPORT pvl_err
pvl_panorama_reset(pvl_panorama *pano);


/** @brief Set run-time parameters of this component.
 *
 *  Set given parameters to the handle.
 *  It is required to get proper parameters instance by pvl_panorama_get_parameters() before setting something.
 *
 *  @param[in]  pano    The handle of this component.
 *  @param[in]  params  The parameters to be set.
 *
 *  @return     On success, @ref pvl_success.
 *  @return     On failure, @ref pvl_err error code, which will be the one of the following return value(s).
 *
 *  @retval     pvl_err_invalid_argument    Passing null pointer to the params or wrong value is specified in the params.
 *  @retval     pvl_err_invalid_status      Invalid internal status of the handle.
 *  @retval     pvl_err_nomem               Failed to re-allocate the memory for parameter changes.
 */
PVLIB_EXPORT pvl_err
pvl_panorama_set_parameters(pvl_panorama *pano, const pvl_panorama_parameters *params);


/** @brief Get current run-time parameters of this component.
 *
 *  Get the parameters from the handle.
 *  This function should be called before calling pvl_panorama_set_parameters().
 *
 *  @param[in]  pano    The handle of this component.
 *  @param[out] params  The buffer which will hold parameters. Its allocation must be managed by the caller.
 *
 *  @return     On success, @ref pvl_success.
 *  @return     On failure, @ref pvl_err error code, which will be the one of the following return value(s).
 *
 *  @retval     pvl_err_invalid_argument    Passing null pointer to the params.
 *  @retval     pvl_err_invalid_status      Invalid internal status of the handle.
 */
PVLIB_EXPORT pvl_err
pvl_panorama_get_parameters(const pvl_panorama *pano, pvl_panorama_parameters *params);


/** @brief Detect preview frames to capture.
 *
 *  This function will detect preview frames that matches well with the imaginary frame the component is expecting.
 *  The overlapping_ratio parameter affects the offset of expecting scene projected from the last stitched image.
 *  This function should be called after the first image is stitched.
 *
 *  In normal scenario this function returns...
 *  @verbatim
    0 0 0 0 0 1 0 0 0 0 0 0 0 0 2 0 0 0 0 0 3 0 0 0 0 0 0 0 0 0 0 4 ...
              |                 |           |                     |
              2nd image         3rd image   4th image             5th image ... @endverbatim
 *
 *  @param[in]  pano            The handle of the panorama component.
 *  @param[in]  preview_frame   The input preview frame. Currently, only pvl_image_format_nv12 is supported.
 *  @param[out] offset          The 2D offset of input frame from the last stitched image.
 *  @param[out] direction       The direction(left/right/up/down) to which input image is moving.
 *
 *  @return     On success, the stitch order, or equivalently the stitch index(>0) where the image to be captured stitched at.
 *                  0(= pvl_success) means that 'successful but not detected yet'.
 *  @return     On failure, @ref pvl_err error code, which will be the one of the following return value(s).
 *
 *  @retval     pvl_err_not_supported       Unsupported image format specified.
 *  @retval     pvl_err_invalid_argument    Passing null instance of pano, preview_frame or offset.
 *  @retval     pvl_err_invalid_status      Invalid internal status of the handle.
 *  @retval     pvl_err_nomem               Failed to allocate the internal memory buffers.
 */
PVLIB_EXPORT int32_t
pvl_panorama_detect_frame_to_stitch(pvl_panorama *pano, const pvl_image *preview_frame, pvl_point *offset, pvl_panorama_direction *direction);


/** @brief Stitch a input image at the tail of stitched images so far
 *
 *  This function will stitch a input image with the last stitched image using given offset as an initial guess.
 *  The series of input images are assumed to be given in the right order in terms of the panning direction.
 *  If it's the first image this function just register it and return.
 *  The maximum number of registerable images is bounded by 'max_supported_num_images' attribute of pvl_panorama structure.
 *
 *  @param[in]  pano            The handle of the panorama component.
 *  @param[in]  input_image     The input image to be stitched. Currently, only pvl_image_format_nv12 is supported.
 *  @param[in]  offset          (Optional) The 2D offset of input image from the previous image. If NULL is given the offset values will be estimated.
 *                                         If it's the first image the offset parameter will be ignored.
 *
 *  @return     On success, @ref pvl_success
 *  @return     On failure, @ref pvl_err error code, which will be the one of the following return value(s).
 *
 *  @retval     pvl_err_not_supported       Unsupported image format specified.
 *  @retval     pvl_err_invalid_argument    Passing null instance of pvl_pano or input_image.
 *  @retval     pvl_err_out_of_bound        stitch_index < 0 or stitch_index >= max_supported_num_image
 *  @retval     pvl_err_invalid_status      Invalid internal status of the handle.
 *  @retval     pvl_err_nomem               Failed to allocate the internal memory buffers.
 */
PVLIB_EXPORT pvl_err
pvl_panorama_stitch_one_image(pvl_panorama *pano, pvl_image *input_image, const pvl_point *offset);


/** @brief Compose all the stitched image so far into the final output image.
 *
 *  This function will compose all the stitched images and allocate internal image buffer to save the final output image.
 *
 *  @param[in]  pano            The handle of the panorama component.
 *  @param[out] output_image    The final output image. The component manages the memory. Do not free the internal data buffer.
 *                              After resetting or destroying the component handle this data would be not reachable.
 *
 *  @return     On success, @ref pvl_success
 *  @return     On failure, @ref pvl_err error code, which will be the one of the following return value(s).
 *
 *  @retval     pvl_err_invalid_argument    Passing null instance of pvl_pano or output_image.
 *  @retval     pvl_err_invalid_status      Invalid internal status of the handle or no stitched images.
 *  @retval     pvl_err_nomem               Failed to allocate the internal memory buffers.
 */
PVLIB_EXPORT pvl_err
pvl_panorama_run(pvl_panorama *pano, pvl_image *output_image);


#ifdef __cplusplus
}
#endif // __cplusplus

/** @example panorama_sample.c
 *  Sample of Panorama
 */

#endif /* __PVL_PANORAMA_H__ */
