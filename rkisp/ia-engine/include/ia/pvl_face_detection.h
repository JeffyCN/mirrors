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
#ifndef __PVL_FACE_DETECTION_H__
#define __PVL_FACE_DETECTION_H__

/** @file    pvl_face_detection.h
 *  @brief   This file declares the structures and native APIs of face detection component.
 */

#include "pvl_types.h"
#include "pvl_config.h"

#ifdef __cplusplus
extern "C" {
#endif


/** @brief A structure to hold run-time configurable parameters for this component.
 *
 *  The variables in this structure will be retrieved and assigned, via pvl_face_detection_get_parameters() and pvl_face_detection_set_parameters() respectively,
 */
struct pvl_face_detection_parameters {
    uint32_t max_num_faces;         /**< The maximum number of detectable faces in one frame.
                                         max_supported_num_faces in pvl_face_detection structure represents the maximum allowable value, and minimum allowable value set to 1.
                                         The default value is set to maximum when the component is created. */

    float min_face_ratio;           /**< The ratio of minimum detectable face size to the shorter side of the input image.
                                         The maximum allowable value is 1.0 (100%) and there is no limitation on the minimum allowable value.
                                         However, the faces smaller than min_face_size set in pvl_face_detection may not be detected. */

    uint16_t rip_range;             /**< The degree of RIP (Rotation In-Plane) ranges, representing [-rip_range, +rip_range]. */

    uint16_t rop_range;             /**< The degree of ROP (Rotation Out-of-Plane) ranges, representing [-rop_range, +rop_range]. */

    uint32_t num_rollover_frames;   /**< The number of rollover frames indicating how many frames the entire scanning will be distributed.
                                         The value works in the preview mode, only. */
};
typedef struct pvl_face_detection_parameters pvl_face_detection_parameters;


/** @brief A structure to hold the outcomes from this component.
 */
struct pvl_face_detection_result {
    pvl_rect rect;                  /**< The rectangular region of the detected face. */
    int32_t confidence;             /**< The confidence value of the detected face [0, 100]. */
    int32_t rip_angle;              /**< The approximated value of RIP angle (in degree) of the detected face. */
    int32_t rop_angle;              /**< The approximated of ROP angle (in degree) of the detected face. */
    int32_t tracking_id;            /**< The tracking id of the face. Only valid in preview mode.
                                         The value will be unique throughout the component life cycle, unless pvl_face_detection_reset() is called. */
};
typedef struct pvl_face_detection_result pvl_face_detection_result;


/** @brief A structure to hold the run-time context of this component.
 *
 *  This structure represents the face detection instance which is used as the handle over most of API.
 *  It holds its own properties, constant parameters and internal context inside.
 */
struct pvl_face_detection {
    const pvl_version version;                  /**< The version information. */

    const uint32_t max_supported_num_faces;     /**< The maximum number of faces supported by this component. */
    const uint32_t min_face_size;               /**< The minimum size in pixel of detectable faces of this component. */

    const uint16_t rip_range_max;               /**< The maximum value of RIP range (in degree). */
    const int16_t rip_range_resolution;         /**< The resolution of RIP range value. RIP range should be multiple of this value. */

    const uint16_t rop_range_max;               /**< The maximum value of ROP range (in degree). */
    const int16_t rop_range_resolution;         /**< The resolution of ROP range value. ROP range should be multiple of this value. */
};
typedef struct pvl_face_detection pvl_face_detection;


/** @brief Get default configuration of this component.
 *
 *  This function returns default configuration of the face detection component.
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
pvl_face_detection_get_default_config(pvl_config *config);


/** @brief Create an instance of the face detection component.
 *
 *  This function initializes and returns an instance of this component.
 *  Multiple instances are allowed to be created concurrently.
 *
 *  @param[in]  config  The configuration information of the component.
 *  @param[out] fd      A pointer to indicate the handle newly created.
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
pvl_face_detection_create(const pvl_config *config, pvl_face_detection **fd);


/** @brief Destroy the instance of this component.
 *
 *  @param[in]  fd   The handle of this component to be destroyed.
 */
PVLIB_EXPORT void
pvl_face_detection_destroy(pvl_face_detection *fd);


/** @brief Reset the instance of this component.
 *
 *  All the internal states including face tracking information and context will be reset except the run-time parameters set by user.
 *
 *  @param[in]  fd  The handle of this component.
 *
 *  @return     On success, @ref pvl_success.
 *  @return     On failure, @ref pvl_err error code, which will be the one of the following return value(s).
 *
 *  @retval     pvl_err_invalid_argument    Passing null pointer to the params.
 *  @retval     pvl_err_invalid_status      Invalid internal status of the handle.
 *  @retval     pvl_err_nomem               Failed to re-allocate the memory.
 */
PVLIB_EXPORT pvl_err
pvl_face_detection_reset(pvl_face_detection *fd);


/** @brief Set run-time parameters of this component.
 *
 *  Set given parameters to the handle.
 *  It is required to get proper parameters instance by pvl_face_detection_get_parameters() before setting something.
 *
 *  @param[in]  fd      The handle of this component.
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
pvl_face_detection_set_parameters(pvl_face_detection *fd, const pvl_face_detection_parameters *params);


/** @brief Get current run-time parameters of this component.
 *
 *  Get the parameters from the handle.
 *  This function should be called before calling pvl_face_detection_set_parameters().
 *
 *  @param[in]  fd      The handle of this component.
 *  @param[out] params  The buffer which will hold parameters. Its allocation must be managed by the caller.
 *
 *  @return     On success, @ref pvl_success.
 *  @return     On failure, @ref pvl_err error code, which will be the one of the following return value(s).
 *
 *  @retval     pvl_err_invalid_argument    Passing null pointer to the params.
 *  @retval     pvl_err_invalid_status      Invalid internal status of the handle.
 */
PVLIB_EXPORT pvl_err
pvl_face_detection_get_parameters(pvl_face_detection *fd, pvl_face_detection_parameters *params);


/** @brief Detect faces from the input image.
 *
 *  This function will run the face detection in the given image.
 *  The caller is responsible for allocating the buffer for the result.
 *
 *  @param[in]  fd          The handle of the face detection component.
 *  @param[in]  image       The input image. All image formats are supported.
 *                          pvl_image_format_gray, pvl_image_format_nv12, pvl_image_format_nv21 and pvl_image_format_yv12 are preferred in terms of speed.
 *  @param[out] result      The result buffer.
 *  @param[in]  max_result  The number of results that the buffers can hold.
 *
 *  @return     On success, the number of detected faces.
 *  @return     On failure, @ref pvl_err error code, which will be the one of the following return value(s).
 *
 *  @retval     pvl_err_not_supported       Unsupported image format specified.
 *  @retval     pvl_err_invalid_argument    Passing null instance of fd or non-positive max_result.
 *  @retval     pvl_err_invalid_status      Invalid internal status of the handle.
 *  @retval     pvl_err_nomem               Failed to allocate the internal memory buffers.
 */
PVLIB_EXPORT int32_t
pvl_face_detection_run_in_image(pvl_face_detection *fd, const pvl_image *image, pvl_face_detection_result *result, int32_t max_result);


/** @brief Detect faces from the gray image, a part of the sequence of preview frame.
 *
 *  This function performs the face detection to the given image, assuming that the image is a part of the sequence of preview or video frames.
 *  The function re-uses the result of the previous frames to detect faces faster, i.e. the result will be affected by previous calls of this function,
 *  whereas each call of pvl_face_detection_run_in_image() works independently.
 *  The caller is responsible for allocating the result buffer.
 *
 *  @param[in]  fd          The handle of the face detection component.
 *  @param[in]  image       The input image. All image formats are supported.
 *                          pvl_image_format_gray, pvl_image_format_nv12, pvl_image_format_nv21 and pvl_image_format_yv12 are preferred in terms of speed.
 *  @param[out] result      The result buffer.
 *  @param[in]  max_result  The number of results that the buffer can hold.
 *
 *  @return     On success, the number of detected faces, which is non-negative value.
 *  @return     On failure, @ref pvl_err error code, which will be the one of the following return value(s).
 *
 *  @retval     pvl_err_not_supported       Unsupported image format specified.
 *  @retval     pvl_err_invalid_argument    Passing null pointer to the params or non-positive max_result.
 *  @retval     pvl_err_invalid_status      Invalid internal status of the handle.
 *  @retval     pvl_err_nomem               Failed to allocate the internal memory buffers.
 */
PVLIB_EXPORT int32_t
pvl_face_detection_run_in_preview(pvl_face_detection *fd, const pvl_image *image, pvl_face_detection_result *result, int32_t max_result);


#ifdef __cplusplus
}
#endif // __cplusplus

/** @example face_detection_sample.c
 *  Sample of Face Detection
 */

#endif /* __PVL_FACE_DETECTION_H__ */
