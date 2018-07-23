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
#ifndef __PVL_EYE_DETECTION_H__
#define __PVL_EYE_DETECTION_H__

/** @file    pvl_eye_detection.h
 *  @brief   This file declares the structures and native APIs of eye detection component.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "pvl_types.h"
#include "pvl_config.h"


/** @brief A structure to hold the outcomes from this component.
 */
struct pvl_eye_detection_result
{
    pvl_point left_eye;         /**< The center position on the left eye (in the view of the observer).
                                     The origin (0,0) of the point is the upper-left corner of the input image. */
    pvl_point right_eye;        /**< The center position on the right eye (in the view of the observer). */
    int32_t confidence;         /**< The confidence value of the detected eyes. The value is negative if the eye detection failed. */
};
typedef struct pvl_eye_detection_result pvl_eye_detection_result;


/** @brief A structure to hold the run-time context of this component.
 *
 *  This structure represents the eye detection instance which is used as the handle over most of API.
 *  It holds its own properties, constant parameters and internal context inside.
 */
struct pvl_eye_detection
{
    const pvl_version version;              /**< The version information. */

    const float max_face_width_ratio;       /**< The maximum supported ratio of the width of the face to the eye distance,
                                                 s.t. max_face_with_ratio = (the width of the face)/(the distance between two eyes).
                                                 The eye detection accuracy will be dropped if the estimated face region (as the parameter of pvl_eye_detection_run) is too large compared to the actual face size. */
    const float max_rip_error_tolerance;    /**< The maximum supported value (in degree) of the difference between the actual RIP degree and the input RIP degree.
                                                 The eye detection accuracy will be dropped if the actual RIP degree is greater than (input RIP angle)±(max_rip_error_tolerance). */
};
typedef struct pvl_eye_detection pvl_eye_detection;


/** @brief Get default configuration of this component.
 *
 *  This function returns default configuration of the eye detection component.
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
pvl_eye_detection_get_default_config(pvl_config *config);


/** @brief Create an instance of the eye detection component.
 *
 *  This function initializes and returns an instance of this component.
 *  Multiple instances are allowed to be created concurrently.
 *
 *  @param[in]  config  The configuration information of the component.
 *  @param[out] ed      A pointer to indicate the handle newly created.
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
pvl_eye_detection_create(const pvl_config *config, pvl_eye_detection **ed);


/** @brief Destroy the instance of this component.
 *
 *  @param[in]  ed   The handle of this component to be destroyed.
 */
PVLIB_EXPORT void
pvl_eye_detection_destroy(pvl_eye_detection *ed);


/** @brief Reset the instance of this component.
 *
 *  All the internal states and context will be reset except the run-time parameters set by user.
 *
 *  @param[in]  ed  The handle of this component.
 *
 *  @return     On success, @ref pvl_success.
 *  @return     On failure, @ref pvl_err error code, which will be the one of the following return value(s).
 *
 *  @retval     pvl_err_invalid_argument    Passing null pointer to the params.
 *  @retval     pvl_err_invalid_status      Invalid internal status of the handle.
 *  @retval     pvl_err_nomem               Failed to allocate the internal memory buffers.
 */
PVLIB_EXPORT pvl_err
pvl_eye_detection_reset(pvl_eye_detection *ed);


/** @brief Detect left and right eye positions from a region of faces in the image.
 *
 *  This function will do the eye detection in the given a region of faces in the image.
 *  The function caller is responsible for allocation of the buffer for result.
 *
 *  @param[in]  ed          The handle of the eye detection component.
 *  @param[in]  image       The input image for detecting the positions of eyes. All image formats are supported.
 *                          pvl_image_format_gray, pvl_image_format_nv12, pvl_image_format_nv21 and pvl_image_format_yv12 are preferred in terms of speed.
 *  @param[in]  face_region The struct of rectangular regions of faces.
 *  @param[in]  rip_angle   The value of RIP (Rotation in Plane) of the faces in degree.
 *  @param[out] result      The result buffer.
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
pvl_eye_detection_run(pvl_eye_detection *ed, const pvl_image *image, const pvl_rect *face_region, int32_t rip_angle, pvl_eye_detection_result *result);


#ifdef __cplusplus
}
#endif

/** @example eye_detection_sample.c
 *  Sample of Eye Detection
 */

#endif // __PVL_EYE_DETECTION_H__
