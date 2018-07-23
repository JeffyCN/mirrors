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
#ifndef __PVL_SMILE_DETECTION_H__
#define __PVL_SMILE_DETECTION_H__

/** @file    pvl_smile_detection.h
 *  @brief   This file declares the structure and native APIs of smile detection component.
 */

#include "pvl_types.h"
#include "pvl_config.h"

#ifdef __cplusplus
extern "C" {
#endif


/** @brief A structure to hold run-time configurable parameters for this component.
 *
 *  The variables in this structure will be retrieved and assigned, via pvl_smile_detection_get_parameters() and pvl_smile_detection_set_parameters() respectively.
 */
struct pvl_smile_detection_parameters
{
    int32_t threshold;                  /**< The threshold value that evaluates the status smile or not if the score greater than or equal to this threshold.
                                             The range of the value may be in [0..100], though, recommended to use the default value set in the structure pvl_smile_detection. */
};
typedef struct pvl_smile_detection_parameters pvl_smile_detection_parameters;


/** @brief The enumerated values to represent smile state of each face.
 */
enum pvl_smile_detection_state
{
    pvl_smile_detection_state_not_smiling   = 0,    /**< The state representing the face is 'NOT smiling'. */
    pvl_smile_detection_state_smiling       = 1,    /**< The state representing the face is 'smiling'. */
};
typedef enum pvl_smile_detection_state pvl_smile_detection_state;


/** @brief A structure to hold the outcomes from this component.
 */
struct pvl_smile_detection_result
{
    int32_t score;                      /**< The smile score of the face in the range of 0 to 100, where 0 means non-smile and 100 means full smile. */
    pvl_smile_detection_state state;    /**< The state of the smile of the face. */
};
typedef struct pvl_smile_detection_result pvl_smile_detection_result;


/** @brief A structure to hold the run-time context of this component.
 *
 *  This structure represents the smile detection instance which is used as the handle over most of API.
 *  It holds its own properties, constant parameters and internal context inside.
 */
struct pvl_smile_detection
{
    const pvl_version version;          /**< The version information. */

    const int32_t default_threshold;    /**< The default threshold value recommended. */
    const int32_t rop_tolerance;        /**< The maximum range of ROP (Rotation Out of Plane) tolerance of the face.
                                             The accuracy may not be guaranteed if the ROP angle is out of range. */
};
typedef struct pvl_smile_detection pvl_smile_detection;


/** @brief Get default configuration of this component.
 *
 *  This function returns default configuration of the smile detection component.
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
pvl_smile_detection_get_default_config(pvl_config *config);


/** @brief Create an instance of the smile detection component.
 *
 *  This function initializes and returns an instance of this component.
 *  Multiple instances are allowed to be created concurrently.
 *
 *  @param[in]  config  The configuration information of the component.
 *  @param[out] sd      A pointer to indicate the handle newly created.
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
pvl_smile_detection_create(const pvl_config *config, pvl_smile_detection **sd);


/** @brief Destroy the instance of this component.
 *
 *  @param[in]  sd   The handle of this component to be destroyed.
 */
PVLIB_EXPORT void
pvl_smile_detection_destroy(pvl_smile_detection *sd);


/** @brief Reset the instance of this component.
 *
 *  All the internal states and context will be reset except the run-time parameters set by user.
 *
 *  @param[in]  sd  The handle of this component.
 *
 *  @return     On success, @ref pvl_success.
 *  @return     On failure, @ref pvl_err error code, which will be the one of the following return value(s).
 *
 *  @retval     pvl_err_invalid_argument    Passing null pointer to the params.
 *  @retval     pvl_err_invalid_status      Invalid internal status of the handle.
 *  @retval     pvl_err_nomem               Failed to allocate the memory.
 */
PVLIB_EXPORT pvl_err
pvl_smile_detection_reset(pvl_smile_detection *sd);


/** @brief Set run-time parameters of this component.
 *
 *  Set given parameters to the handle.
 *  It is required to get proper parameters instance by pvl_smile_detection_get_parameters() before setting something.
 *
 *  @param[in]  sd      The handle of this component.
 *  @param[in]  params  The parameters to be set.
 *
 *  @return     On success, @ref pvl_success.
 *  @return     On failure, @ref pvl_err error code, which will be the one of the following return value(s).
 *
 *  @retval     pvl_err_invalid_argument    Passing null pointer to the params or wrong value is specified in the params.
 *  @retval     pvl_err_invalid_status      Invalid internal status of the handle.
 *  @retval     pvl_err_nomem               Failed to re-allocate the memory.
 */
PVLIB_EXPORT pvl_err
pvl_smile_detection_set_parameters(pvl_smile_detection *sd, const pvl_smile_detection_parameters *params);


/** @brief Get current run-time parameters of this component.
 *
 *  Get the parameters from the handle.
 *  This function should be called before calling pvl_smile_detection_set_parameters().
 *
 *  @param[in]  sd      The handle of this component.
 *  @param[out] params  The buffer which will hold parameters. Its allocation must be managed by the caller.
 *
 *  @return     On success, @ref pvl_success.
 *  @return     On failure, @ref pvl_err error code, which will be the one of the following return value(s).
 *
 *  @retval     pvl_err_invalid_argument    Passing null pointer to the params.
 *  @retval     pvl_err_invalid_status      Invalid internal status of the handle.
 */
PVLIB_EXPORT pvl_err
pvl_smile_detection_get_parameters(pvl_smile_detection *sd, pvl_smile_detection_parameters *params);


/** @brief Detect smile on the face in the input image.
 *
 *  This function runs smile detection on the face in the given image.
 *  In contrast with pvl_smile_detection_run_in_preview(), temporal correlation with previous input images will not be considered.
 *  The caller is responsible for allocating the buffer for the result.
 *
 *  @param[in]  sd          The handle of the smile detection component.
 *  @param[in]  image       The input image. All image formats are supported.
 *                          pvl_image_format_gray, pvl_image_format_nv12, pvl_image_format_nv21 and pvl_image_format_yv12 are preferred in terms of speed.
 *  @param[in]  left_eye    The center point of the left eye of the face.
 *  @param[in]  right_eye   The center point of the right eye of the face.
 *  @param[out] result      The result of the component. It must be allocated with enough memory to hold the result.
 *
 *  @return     On success, @ref pvl_success.
 *  @return     On failure, @ref pvl_err error code, which will be the one of the following return value(s).
 *
 *  @retval     pvl_err_not_supported       Unsupported image format specified.
 *  @retval     pvl_err_invalid_argument    Passing null pointer to the params.
 *  @retval     pvl_err_invalid_status      Invalid internal status of the handle.
 *  @retval     pvl_err_nomem               Failed to allocate the internal memory buffers.
 */
PVLIB_EXPORT pvl_err
pvl_smile_detection_run_in_image(pvl_smile_detection *sd, const pvl_image *image, const pvl_point *left_eye, const pvl_point *right_eye,
                                 pvl_smile_detection_result *result);


/** @brief Detect smile on the faces in the input image which is the part of the preview or the video frames.
 *
 *  This function will run smile detection on the faces in the input image assuming that the input image is the part of preview or video frames,
 *  meaning this frame has temporal correlation with previous and next input frames.
 *  Using the information from the previous frame may show better result in terms of the jitter or the noise.
 *
 *  The caller is responsible for allocating the buffer for result.
 *
 *  @param[in]  sd              The handle of the smile detection component.
 *  @param[in]  image           The input image. All image formats are supported.
 *                              pvl_image_format_gray, pvl_image_format_nv12, pvl_image_format_nv21 and pvl_image_format_yv12 are preferred in terms of speed.
 *  @param[in]  left_eye        The center point of the left eye of the face.
 *  @param[in]  right_eye       The center point of the right eye of the face.
 *  @param[in]  tracking_id     The tracking-id returned from face detection component. Use pvl_smile_detection_run_in_image() if tracking-id is not available.
 *  @param[out] result          The result of the component. Must have enough memory to hold the result.
 *
 *  @return     On success, @ref pvl_success.
 *  @return     On failure, @ref pvl_err error code, which will be the one of the following return value(s).
 *
 *  @retval     pvl_err_not_supported       Unsupported image format specified.
 *  @retval     pvl_err_invalid_argument    Passing null pointer to the params.
 *  @retval     pvl_err_invalid_status      Invalid internal status of the handle.
 *  @retval     pvl_err_nomem               Failed to allocate the internal memory buffers.
 */
PVLIB_EXPORT pvl_err
pvl_smile_detection_run_in_preview(pvl_smile_detection *sd, const pvl_image *image, const pvl_point *left_eye, const pvl_point *right_eye,
                                   int32_t tracking_id, pvl_smile_detection_result *result);


#ifdef __cplusplus
}
#endif // __cplusplus

/** @example smile_detection_sample.c
 *  Sample of Smile Detection
 */

#endif /* __PVL_SMILE_DETECTION_H__ */
