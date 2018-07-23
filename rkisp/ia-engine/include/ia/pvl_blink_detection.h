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
#ifndef __PVL_BLINK_DETECTION_H__
#define __PVL_BLINK_DETECTION_H__

/** @file    pvl_blink_detection.h
 *  @brief   This file declares the structures and native APIs of blink detection component.
 */

#include "pvl_types.h"
#include "pvl_config.h"

#ifdef __cplusplus
extern "C" {
#endif


/** @brief A structure to hold run-time configurable parameters for this component.
 *
 *  The variables in this structure will be retrieved and assigned, via pvl_blink_detection_get_parameters() and pvl_blink_detection_set_parameters() respectively.
 */
struct pvl_blink_detection_parameters
{
    int32_t threshold;                      /**< A threshold value which is the determinant of the blink. If the given blink score on the eye is
                                                 greater than or equal to this value, the state of the eye will be estimated as 'closed'. */
};
typedef struct pvl_blink_detection_parameters pvl_blink_detection_parameters;


/** @brief The enumerated values to represent the blink state of an eye.
 */
enum pvl_blink_detection_state
{
    pvl_blink_detection_state_open    = 0,   /**< The state representing the eye open. */
    pvl_blink_detection_state_closed  = 1    /**< The state representing the eye closed. */
};
typedef enum pvl_blink_detection_state pvl_blink_detection_state;


/** @brief A structure to hold the outcomes from this component.
 */
struct pvl_blink_detection_result
{
    int32_t left_score;                     /**< The blink score on the left eye in range between 0 and 100, where 0 means wide opened eye and 100 means fully closed eye. */
    pvl_blink_detection_state left_state;   /**< The blink state on the left eye. */
    int32_t right_score;                    /**< The blink score on the right eye in range between 0 and 100, where 0 means wide opened eye and 100 means fully closed eye. */
    pvl_blink_detection_state right_state;  /**< The blink state on the right eye. */
};
typedef struct pvl_blink_detection_result pvl_blink_detection_result;


/** @brief A structure to hold the run-time context of this component.
 *
 *  This structure represents the blink detection instance which is used as the handle over most of API.
 *  It holds its own properties, constant parameters and internal context inside.
 */
struct pvl_blink_detection
{
    const pvl_version version;          /**< The version information. */

    const int32_t default_threshold;    /**< The default threshold value recommended. */
    const int32_t rop_tolerance;        /**< The maximum range of ROP (Rotation Out of Plane) tolerance of the face.
                                             The accuracy may not be guaranteed if the ROP angle is out of range. */
};
typedef struct pvl_blink_detection pvl_blink_detection;


/** @brief Get default configuration of this component.
 *
 *  This function returns default configuration of the blink detection component.
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
pvl_blink_detection_get_default_config(pvl_config *config);


/** @brief Create an instance of the blink detection component.
 *
 *  This function initializes and returns an instance of this component.
 *  Multiple instances are allowed to be created concurrently.
 *
 *  @param[in]  config  The configuration information of the component.
 *  @param[out] bd      A pointer to indicate the handle newly created.
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
pvl_blink_detection_create(const pvl_config *config, pvl_blink_detection **bd);


/** @brief Destroy the instance of this component.
 *
 *  @param[in]  bd   The handle of this component to be destroyed.
 */
PVLIB_EXPORT void
pvl_blink_detection_destroy(pvl_blink_detection *bd);


/** @brief Reset the instance of this component.
 *
 *  All the internal states and context will be reset except the run-time parameters set by user.
 *
 *  @param[in]  bd  The handle of this component.
 *
 *  @return     On success, @ref pvl_success.
 *  @return     On failure, @ref pvl_err error code, which will be the one of the following return value(s).
 *
 *  @retval     pvl_err_invalid_argument    Passing null pointer to the params.
 *  @retval     pvl_err_invalid_status      Invalid internal status of the handle.
 */
PVLIB_EXPORT pvl_err
pvl_blink_detection_reset(pvl_blink_detection *bd);


/** @brief Set run-time parameters of this component.
 *
 *  Set given parameters to the handle.
 *  It is required to get proper parameters instance by pvl_blink_detection_get_parameters() before setting something.
 *
 *  @param[in]  bd      The handle of this component.
 *  @param[in]  params  The parameters to be set.
 *
 *  @return     On success, @ref pvl_success.
 *  @return     On failure, @ref pvl_err error code, which will be the one of the following return value(s).
 *
 *  @retval     pvl_err_invalid_argument    Passing null pointer to the params or wrong value is specified in the params.
 *  @retval     pvl_err_invalid_status      Invalid internal status of the handle.
 */
PVLIB_EXPORT pvl_err
pvl_blink_detection_set_parameters(pvl_blink_detection *bd, const pvl_blink_detection_parameters *params);


/** @brief Get current run-time parameters of this component.
 *
 *  Get the parameters from the handle.
 *  This function should be called before calling pvl_blink_detection_set_parameters().
 *
 *  @param[in]  bd      The handle of this component.
 *  @param[out] params  The buffer which will hold parameters. Its allocation must be managed by the caller.
 *
 *  @return     On success, @ref pvl_success.
 *  @return     On failure, @ref pvl_err error code, which will be the one of the following return value(s).
 *
 *  @retval     pvl_err_invalid_argument    Passing null pointer to the params.
 *  @retval     pvl_err_invalid_status      Invalid internal status of the handle.
 */
PVLIB_EXPORT pvl_err
pvl_blink_detection_get_parameters(pvl_blink_detection *bd, pvl_blink_detection_parameters *params);


/** @brief Detect blink on the face in the input image.
 *
 *  This function runs blink detection on the face in the given image.
 *  It runs on both eyes. It's not allowed to run with only one eye.
 *  The caller is responsible for allocating the buffer for the result.
 *
 *  @param[in]  bd          The handle of the blink detection component.
 *  @param[in]  image       The input image. All image formats are supported.
 *                          pvl_image_format_gray, pvl_image_format_nv12, pvl_image_format_nv21 and pvl_image_format_yv12 are preferred in terms of speed.
 *  @param[in]  left_eye    The center point on the left eye of the face.
 *  @param[in]  right_eye   The center point on the right eye of the face.
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
pvl_blink_detection_run(pvl_blink_detection *bd, const pvl_image *image, const pvl_point *left_eye, const pvl_point *right_eye,
                        pvl_blink_detection_result *result);


#ifdef __cplusplus
}
#endif // __cplusplus

/** @example blink_detection_sample.c
 *  Sample of Blink Detection
 */

#endif /* __PVL_BLINK_DETECTION_H__ */
