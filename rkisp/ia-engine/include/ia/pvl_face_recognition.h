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
#ifndef __PVL_FACE_RECOGNITION_H__
#define __PVL_FACE_RECOGNITION_H__

/** @file    pvl_face_recognition.h
 *  @brief   This file declares the structures and native APIs of face recognition component.
 */

#include "pvl_types.h"
#include "pvl_config.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FACE_RECOGNIZER_UNKNOWN_PERSON_ID   (-10000)   /**< The "unknown person", which will be assigned to person_id of the structure, 'pvl_face_recognition_facedata'. */


/** @brief A structure to hold a set of data representing a face which is used in face recognition.
 */
struct pvl_face_recognition_facedata{
    uint64_t face_id;       /**< The unique id of the face data. */
    int32_t  person_id;     /**< The unique id of the person associated with the face data. The valid person will have positive person_id (i.e. person_id > 0). */
    uint32_t time_stamp;    /**< The timestamp when the face data was generated. */
    int32_t  condition;     /**< The environmental information of the face. Reserved for future use. */
    int32_t  checksum;      /**< The checksum value of the face data. */
    uint8_t  *data;         /**< The pointer to the actual face data. Face data is essentially a binary encoded representation of the face generated from the gray face image.
                                 The size will be stored in facedata_size of the structure 'pvl_face_recognition'. */
};
typedef struct pvl_face_recognition_facedata pvl_face_recognition_facedata;


/** @brief A structure to hold run-time configurable parameters for this component.
 *
 *  The variables in this structure will be retrieved and assigned, via pvl_face_recognition_get_parameters() and pvl_face_recognition_set_parameters() respectively.
 */
struct pvl_face_recognition_parameters{
    uint32_t max_faces_in_preview;  /**< The maximum number of recognizable faces in one frame.
                                         The maximum allowable value is 'max_supported_faces_in_preview' in the structure 'pvl_face_recognition',
                                         and the minimum allowable value is 1. The default value is set to maximum while the component created.
                                         (i.e. 1 <= max_faces_inpreview <= max_supported_faces_in_preview) */
};
typedef struct pvl_face_recognition_parameters pvl_face_recognition_parameters;


/** @brief A structure to hold the outcomes from this component.
 */
struct pvl_face_recognition_result{
    pvl_face_recognition_facedata facedata;     /**< The extracted face data from the face. If recognition succeeded, a valid person-id will be allocated in the field 'person_id'. */
    int32_t similarity;                         /**< The estimated similarity between the input face and the faces in the database. The biggest value will be assigned. */
};
typedef struct pvl_face_recognition_result pvl_face_recognition_result;


/** @brief A structure to hold the run-time context of this component.
 *
 *  This structure represents the blink detection instance which is used as the handle over most of API.
 *  It holds its own properties, constant parameters and internal context inside.
 */
struct pvl_face_recognition{
    const pvl_version version;                      /**< The version information. */

    const uint32_t max_supported_faces_in_preview;  /**< The maximum number of faces supported by this component. */
    const uint32_t max_faces_in_database;           /**< The maximum number of faces that the database can hold in the current version. */
    const uint32_t max_persons_in_database;         /**< The maximum number of persons that the database can hold in the current version. */
    const uint32_t max_faces_per_person;            /**< The maximum number of faces per person in the current version. */
    const uint32_t facedata_size;                   /**< The fixed size of the face data (in bytes) for the current version. */
};
typedef struct pvl_face_recognition pvl_face_recognition;


/** @brief Get default configuration of this component.
 *
 *  This function returns default configuration of the face recognition component.
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
pvl_face_recognition_get_default_config(pvl_config *config);


/** @brief Create an instance of the face recognition component.
 *
 *  This function initializes and returns an instance of this component.
 *  Multiple instances are allowed to be created concurrently.
 *
 *  @param[in]  config  The configuration information of the component.
 *  @param[out] fr      A pointer to indicate the handle newly created.
 *
 *  @return     On success, @ref pvl_success.
 *  @return     On failure, @ref pvl_err error code, which will be the one of the following return value(s).
 *
 *  @retval     pvl_err_not_supported       Unsupported configuration.
 *  @retval     pvl_err_invalid_argument    Passing null pointer to the params.
 *  @retval     pvl_err_nomem               Failed in allocating the memory.
 */
PVLIB_EXPORT pvl_err
pvl_face_recognition_create(const pvl_config *config, pvl_face_recognition **fr);


/** @brief Destroy the instance of this component.
 *
 *  @param[in]  fr   The handle of this component to be destroyed.
 */
PVLIB_EXPORT void
pvl_face_recognition_destroy(pvl_face_recognition *fr);


/** @brief Reset the instance of this component.
 *
 *  All the internal states, the internal database and context will be reset except the run-time parameters set by user.
 *
 *  @param[in]  fr  The handle of this component.
 *
 *  @return     On success, @ref pvl_success.
 *  @return     On failure, @ref pvl_err error code, which will be the one of the following return value(s).
 *
 *  @retval     pvl_err_invalid_argument    Passing null pointer to the params.
 *  @retval     pvl_err_invalid_status      Invalid internal status of the handle or database.
 *  @retval     pvl_err_nomem               Failed in re-allocating the memory.
 */
PVLIB_EXPORT pvl_err
pvl_face_recognition_reset(pvl_face_recognition *fr);


/** @brief Set run-time parameters of this component.
 *
 *  Set given parameters to the handle.
 *  It is required to get proper parameters instance by pvl_face_recognition_get_parameters() before setting something.
 *
 *  @param[in]  fr      The handle of this component.
 *  @param[in]  params  The parameters to be set.
 *
 *  @return     On success, @ref pvl_success.
 *  @return     On failure, @ref pvl_err error code, which will be the one of the following return value(s).
 *
 *  @retval     pvl_err_invalid_argument    Passing null pointer to the params or wrong value is specified in the params.
 *  @retval     pvl_err_invalid_status      Invalid internal status of the handle.
 *  @retval     pvl_err_nomem               Failed in re-allocating the memory for parameter changes.
 */
PVLIB_EXPORT pvl_err
pvl_face_recognition_set_parameters(pvl_face_recognition *fr, const pvl_face_recognition_parameters *params);


/** @brief Get current run-time parameters of this component.
 *
 *  @param[in]  fr      The handle of this component.
 *  @param[out] params  The buffer which will hold parameters. Memory needs to be managed by caller.
 *
 *  @return     On success, @ref pvl_success.
 *  @return     On failure, @ref pvl_err error code, which will be the one of the following return value(s).
 *
 *  @retval     pvl_err_invalid_argument    Passing null pointer to the params.
 *  @retval     pvl_err_invalid_status      Invalid internal status of the handle.
 */
PVLIB_EXPORT pvl_err
pvl_face_recognition_get_parameters(pvl_face_recognition *fr, pvl_face_recognition_parameters *params);


/** @brief The main function that run the face recognition for the faces in image.
 *
 *  This function conducts the face recognition by trying to match query faces (input faces) against the faces in the internal database.
 *  The internal database should be populated in advance, via pvl_face_recognition_register_facedata().
 *  If the recognition succeeded, person_id field at corresponding index of the result will be filled with valid (+) person_id.
 *  Otherwise, person_id will be @ref FACE_RECOGNIZER_UNKNOWN_PERSON_ID.
 *  Since the face recognition result could be the false-alarm (i.e. incorrect recognition.),
 *  the positive person_id does not guarantee that the recognition result is correct.
 *
 *  @param[in]  fr          The handle of the face recognition component.
 *  @param[in]  image       The input image that contains the query faces (input faces). All image formats are supported.
 *                          pvl_image_format_gray, pvl_image_format_nv12, pvl_image_format_nv21 and pvl_image_format_yv12 are preferred in terms of speed.
 *  @param[in]  num_faces   The number of query faces (input faces) to recognize.
 *  @param[in]  left_eyes   The array of the center points on the left eyes of faces to be recognized.
 *  @param[in]  right_eyes  The array of the center points on the right eyes of faces to be recognized.
 *  @param[out] results     The array of the result buffer.
 *
 *  @return     On success, @ref pvl_success.
 *  @return     On failure, @ref pvl_err error code, which will be the one of the following return value(s).
 *
 *  @retval     pvl_err_not_supported       Unsupported image format specified.
 *  @retval     pvl_err_invalid_argument    Passing null pointer to the params or negative num_faces.
 *  @retval     pvl_err_invalid_status      Invalid internal status of the handle.
 *  @retval     pvl_err_nomem               Failed in allocating the internal memory buffers.
 */
PVLIB_EXPORT pvl_err
pvl_face_recognition_run_in_image(pvl_face_recognition *fr, const pvl_image *image, int32_t num_faces,
                                  const pvl_point *left_eyes, const pvl_point *right_eyes,
                                  pvl_face_recognition_result *results);


/** @brief Run the face recognition component for the faces in image assuming the image is a part of series of preview frames.
 *
 *  This function is basically the same as pvl_face_recognition_run_in_image(), but assuming that the input image is the part of the sequence of preview or video frames.
 *  Utilizing results from previous frames, this function shows faster and more reliable recognition results.
 *
 *  @param[in]  fr              The handle of the face recognition component.
 *  @param[in]  image           The input image that contains the query faces (input faces). All image formats are supported.
 *                              pvl_image_format_gray, pvl_image_format_nv12, pvl_iamge_format_nv21 and pvl_image_format_yv12 are preferred in terms of speed.
 *  @param[in]  num_faces       The number of query faces (input faces) to recognize.
 *  @param[in]  left_eyes       The array of the center points on the left eyes of faces to be recognized.
 *  @param[in]  right_eyes      The array of the center points on the right eyes of faces to be recognized.
 *  @param[in]  tracking_ids    Tracking-ids returned from face detector. Using pvl_face_recognition_run_in_image() is recommended if tracking-id is not available.
 *  @param[out] results         The array of the result buffer.
 *
 *  @return     On success, @ref pvl_success.
 *  @return     On failure, @ref pvl_err error code, which will be the one of the following return value(s).
 *
 *  @retval     pvl_err_not_supported       Unsupported image format specified.
 *  @retval     pvl_err_invalid_argument    Passing null pointer to the params or negative num_faces.
 *  @retval     pvl_err_invalid_status      Invalid internal status of the handle.
 *  @retval     pvl_err_nomem               Failed in allocating the internal memory buffers.
 */
PVLIB_EXPORT pvl_err
pvl_face_recognition_run_in_preview(pvl_face_recognition *fr, const pvl_image *image, int32_t num_faces,
                                    const pvl_point *left_eyes, const pvl_point *right_eyes, const int32_t *tracking_ids,
                                    pvl_face_recognition_result *results);


/** @brief Register one face data to the internal database of the component.
 *
 *  This function registers the given face data into the internal database.
 *  It is common practice for the user to read face data from the external database (e.g. SQLite, MySQL, file system, etc.) and register each face data to the internal database.
 *  Before using pvl_face_recognition_run_in_image() or pvl_face_recognition_run_in_preview(),
 *  the user should call this function to register faces to the database.
 *
 *  @param[in]  fr          The handle of the face recognition component.
 *  @param[in]  facedata    A face data instance to be registered.
 *
 *  @return     On success, @ref pvl_success.
 *  @return     On failure, @ref pvl_err error code, which will be the one of the following return value(s).
 *
 *  @retval     pvl_err_invalid_argument    Passing null pointer to the fr parameter, or wrong facedata.
 *  @retval     pvl_err_invalid_status      Invalid internal status of the handle.
 *  @retval     pvl_err_nomem               Failed in allocating the memory of the internal buffers or the database.
 *  @retval     pvl_err_database_full       Not enough memory left to insert new data.
 */
PVLIB_EXPORT pvl_err
pvl_face_recognition_register_facedata(pvl_face_recognition *fr, const pvl_face_recognition_facedata *facedata);


/** @brief Unregister the face from the internal database.
 *
 *  This function deletes the face data from internal database.
 *
 *  @param[in]  fr          The handle of the face recognition component.
 *  @param[in]  face_id     Face id to be unregistered.
 *
 *  @return     On success, @ref pvl_success.
 *  @return     On failure, @ref pvl_err error code, which will be the one of the following return value(s).
 *
 *  @retval     pvl_err_invalid_argument    Passing null pointer to the fr parameter, or negative value to the face_id.
 *  @retval     pvl_err_invalid_status      Invalid internal status of the handle.
 *  @retval     pvl_err_nomem               Failed in allocating the memory of the internal buffers or the database.
 *  @retval     pvl_err_no_such_item        Face data not found from the database that matches the given face_id.
 */
PVLIB_EXPORT pvl_err
pvl_face_recognition_unregister_facedata(pvl_face_recognition *fr, uint64_t face_id);


/** @brief Unregister all faces associated with the person_id.
 *
 *  This function is used to remove all registered faces of specified person.
 *
 *  @param[in]  fr          The handle of the face recognition component.
 *  @param[in]  person_id   Person id to be unregistered.
 *
 *  @return     On success, @ref pvl_success.
 *  @return     On failure, @ref pvl_err error code, which will be the one of the following return value(s).
 *
 *  @retval     pvl_err_invalid_argument    Passing null pointer to the params.
 *  @retval     pvl_err_invalid_status      Invalid internal status of the handle.
 *  @retval     pvl_err_no_such_item        Face data not found from the database that matches the given person_id.
 */
PVLIB_EXPORT pvl_err
pvl_face_recognition_unregister_person(pvl_face_recognition *fr, int32_t person_id);


/** @brief Update person_id of specified face data.
 *
 *  This function is used to modify person id of specific face data. Usually used to assign a person id to unknown face or incorrectly recognized face.
 *
 *  @param[in]  fr              The handle of the face recognition component.
 *  @param[in]  face_id         Face id to modify.
 *  @param[in]  new_person_id   Person id to update with.
 *
 *  @return     On success, @ref pvl_success.
 *  @return     On failure, @ref pvl_err error code, which will be the one of the following return value(s).
 *
 *  @retval     pvl_err_invalid_argument    Passing null pointer to the fr parameter, or negative value to the face_id/new_person_id.
 *  @retval     pvl_err_invalid_status      Invalid internal status of the handle.
 *  @retval     pvl_err_database_full       Not enough memory left to insert new data.
 *  @retval     pvl_err_no_such_item        Face data not found from the database that matches the given face_id.
 */
PVLIB_EXPORT pvl_err
pvl_face_recognition_update_person(pvl_face_recognition *fr, uint64_t face_id, int32_t new_person_id);


/** @brief Get number of faces in the internal database.
 *
 *  This function returns the number of faces registered in the internal database.
 *
 *  @param[in]  fr          The handle of the face recognition component.
 *
 *  @return     On success, returns number of faces (non-negative value.)
 *  @return     On failure, @ref pvl_err error code, which will be the one of the following return value(s).
 *
 *  @retval     pvl_err_invalid_argument    Passing null pointer to the params.
 *  @retval     pvl_err_invalid_status      Invalid internal status of the handle.
 */
PVLIB_EXPORT int32_t
pvl_face_recognition_get_num_faces_in_database(pvl_face_recognition *fr);

/** @brief Allocate the memory of result buffers.
 *
 *  This function is used to allocate memory for the result buffers which contain the array of face feature data.
 *  The memory for result buffers should be allocated in the form of the array of 'pvl_face_recognition_result' before
 *   the functions which conduct face recognition are called.
 *  This function conducts the memory allocation for the output buffers.
 *
 *  @param[in]  fr            The handle of the face recognition component.
 *  @param[in]  max_faces     The maximum number of recognizable faces, which is used for allocating results buffer.
 *  @param[out] fr_results    The initialized array of the result buffer.
 *
 *  @return     On success,   @ref pvl_success.
 *  @return     On failure,   @ref pvl_err error code, which will be the one of the following return value(s).
 *
 *  @retval     pvl_err_invalid_argument    Passing null pointer to the fr or fr_results parameter, or negative value to the max_faces.
 *  @retval     pvl_err_out_of_bound        Parameters or condition for the allocation is not acceptable.
 *  @retval     pvl_err_nomem               Failed in allocating the memory of the internal buffers or the database.
 */
PVLIB_EXPORT pvl_err
pvl_face_recognition_create_result_buffer(pvl_face_recognition *fr, int max_faces, pvl_face_recognition_result **fr_results);

/** @brief Destroy the memory buffer.
 *
 * This function releases internal allocated buffers and destroys the data.
 *
 *  @param[in]  fr_results   The handle of the result buffer to be destroyed.
 *
 */
PVLIB_EXPORT void
pvl_face_recognition_destroy_result_buffer(pvl_face_recognition_result *fr_results);


#ifdef __cplusplus
}
#endif // __cplusplus

/** @example face_recognition_sample.c
 *  Sample of Face Recognition
 */

#endif /* __PVL_FACE_RECOGNITION_H__ */
