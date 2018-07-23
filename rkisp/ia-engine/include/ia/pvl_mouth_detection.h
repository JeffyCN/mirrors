/*
 * Copyright 2015 Intel Corporation
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
#ifndef __PVL_MOUTH_DETECTION_H__
#define __PVL_MOUTH_DETECTION_H__

/** @file    pvl_mouth_detection.h
 *  @brief   This file declares the structures and native APIs of mouth detection component.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "pvl_types.h"
#include "pvl_config.h"


/** @brief A structure to hold the outcomes from this component.
 */
struct pvl_mouth_detection_result
{
    pvl_point mouth;
    int32_t confidence;
};
typedef struct pvl_mouth_detection_result pvl_mouth_detection_result;


/** @brief A structure to hold the run-time context of this component.
 *
 *  This structure represents the mouth detection instance which is used as the handle over most of API.
 *  It holds its own properties, constant parameters and internal context inside.
 */
struct pvl_mouth_detection
{
    const pvl_version version;              /**< The version information. */

    const float max_face_width_ratio;       /**< The maximum supported ratio of the width of the face to the mouth distance,
                                                 s.t. max_face_with_ratio = (the width of the face)/(the distance between two mouths).
                                                 The mouth detection accuracy will be dropped if the estimated face region (as the parameter of pvl_mouth_detection_run) is too large compared to the actual face size. */
    const float max_rip_error_tolerance;    /**< The maximum supported value (in degree) of the difference between the actual RIP degree and the input RIP degree.
                                                 The mouth detection accuracy will be dropped if the actual RIP degree is greater than (input RIP angle)±(max_rip_error_tolerance). */
};
typedef struct pvl_mouth_detection pvl_mouth_detection;


/** @brief Get default configuration of this component.
 *
 *  This function returns default configuration of the mouth detection component.
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
pvl_mouth_detection_get_default_config(pvl_config *config);


/** @brief Create an instance of the mouth detection component.
 *
 *  This function initializes and returns an instance of this component.
 *  Multiple instances are allowed to be created concurrently.
 *
 *  @param[in]  config  The configuration information of the component.
 *  @param[out] md      A pointer to indicate the handle newly created.
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
pvl_mouth_detection_create(const pvl_config *config, pvl_mouth_detection **md);


/** @brief Destroy the instance of this component.
 *
 *  @param[in]  md   The handle of this component to be destroyed.
 */
PVLIB_EXPORT void
pvl_mouth_detection_destroy(pvl_mouth_detection *md);


/** @brief Reset the instance of this component.
 *
 *  All the internal states and context will be reset except the run-time parameters set by user.
 *
 *  @param[in]  md  The handle of this component.
 *
 *  @return     On success, @ref pvl_success.
 *  @return     On failure, @ref pvl_err error code, which will be the one of the following return value(s).
 *
 *  @retval     pvl_err_invalid_argument    Passing null pointer to the params.
 *  @retval     pvl_err_invalid_status      Invalid internal status of the handle.
 *  @retval     pvl_err_nomem               Failed to allocate the internal memory buffers.
 */
PVLIB_EXPORT pvl_err
pvl_mouth_detection_reset(pvl_mouth_detection *md);


/** @brief Detect left and right mouth positions from a region of faces in the image.
 *
 *  This function will do the mouth detection in the given a region of faces in the image.
 *  The function caller is responsible for allocation of the buffer for result.
 *
 *  @param[in]  md          The handle of the mouth detection component.
 *  @param[in]  image       The input image for detecting the positions of mouths. All image formats are supported.
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
pvl_mouth_detection_run(pvl_mouth_detection *md, const pvl_image *image, const pvl_rect *face_region, int32_t rip_angle, pvl_mouth_detection_result *result);


#ifdef __cplusplus
}
#endif

/** @example mouth_detection_sample.c
 *  Sample of Mouth Detection
 */

#endif // __PVL_MOUTH_DETECTION_H__
