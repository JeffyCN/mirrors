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
#ifndef __PVL_FACE_RECOGNITION_WITH_DB_H__
#define __PVL_FACE_RECOGNITION_WITH_DB_H__

/** @file    pvl_face_recognition_with_db.h
 *  @brief   This file declares the structures and native APIs of the extended face recognition component which contains permanent DB management.
 */

#include "pvl_face_recognition.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief A structure to hold a set of data representing a face which is used in face recognition.
 */
typedef pvl_face_recognition_facedata pvl_face_recognition_with_db_facedata;

/** @brief A structure to hold run-time configurable parameters for this component.
 *
 *  The variables in this structure will be retrieved and assigned, via pvl_face_recognition_with_db_get_parameters() and pvl_face_recognition_with_db_set_parameters() respectively.
 */
struct pvl_face_recognition_with_db_parameters {
    uint32_t    max_faces_in_preview;  /**< The maximum number of recognizable faces in one frame.
                                         The maximum allowable value is 'max_supported_faces_in_preview' in the structure 'pvl_face_recognition_with_db',
                                         and the minimum allowable value is 1. The default value is set to maximum while the component created.
                                         (i.e. 1 <= max_faces_inpreview <= max_supported_faces_in_preview) */
    int32_t     db_sync_interval;      /* The interval frame number that preview function conducts facedata DB polling. */
    char        db_path[1024];         /* The absolute file path of the facedata DB file */
};
typedef struct pvl_face_recognition_with_db_parameters pvl_face_recognition_with_db_parameters;


/** @brief A structure to hold the outcomes from this component.
 */
typedef pvl_face_recognition_result pvl_face_recognition_with_db_result;


/** @brief A structure to hold the run-time context of this component.
 *
 *  This structure represents the blink detection instance which is used as the handle over most of API.
 *  It holds its own properties, constant parameters and internal context inside.
 */
struct pvl_face_recognition_with_db {
    const pvl_version version;                      /**< The version information. */

    const uint32_t max_supported_faces_in_preview;  /**< The maximum number of faces supported by this component. */
    const uint32_t max_faces_in_database;           /**< The maximum number of faces that the database can hold in the current version. */
    const uint32_t max_persons_in_database;         /**< The maximum number of persons that the database can hold in the current version. */
    const uint32_t max_faces_per_person;            /**< The maximum number of faces per person in the current version. */
    const uint32_t facedata_size;                   /**< The fixed size of the face data (in bytes) for the current version. */
    const uint32_t default_db_sync_interval;        /**< The default value for the interval frame number that preview function conducts facedata DB polling. The Minimum value is 2.*/
    const char     default_db_path[1024];           /**< The default file path of the facedata DB file */
};
typedef struct pvl_face_recognition_with_db pvl_face_recognition_with_db;


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
pvl_face_recognition_with_db_get_default_config(pvl_config *config);


/** @brief Create an instance of the extended face recognition component.
 *
 *  This function initializes and returns an instance of this component.
 *  This function loads the face DB from the given file path and registers the facedata into initialized instance.
 *  This function creates the face DB table if there is no facedata DB at the given file path.
 *  Multiple instances are allowed to be created concurrently.
 *
 *  @param[in]  config  The configuration information of the component.
 *  @param[in]  db_path The absolute file path of facedata DB.
 *                      The parameter 'db_path' of 'pvl_face_recognition_with_db_parameters' will be set as the same path.
 *                      The default path 'default_db_path' will be used if this parameter is NULL.
 *  @param[out] fr      A pointer to indicate the handle newly created.
 *
 *  @return     On success, @ref pvl_success.
 *  @return     On failure, @ref pvl_err error code, which will be the one of the following return value(s).
 *
 *  @retval     pvl_err_not_supported       Unsupported configuration.
 *  @retval     pvl_err_invalid_argument    Passing null pointer to the params.
 *  @retval     pvl_err_nomem               Failed in allocating the memory.
 *  @retval     pvl_err_database_full       Failed in operating DB handling.
 *  @retval     pvl_err_interrupted         Facedata fileDB has not been created yet.
 */
PVLIB_EXPORT pvl_err
pvl_face_recognition_with_db_create(const pvl_config *config, const char *db_path, pvl_face_recognition_with_db **fr);


/** @brief Destroy the instance of this component.
 *
 *  @param[in]  fr   The handle of this component to be destroyed.
 */
PVLIB_EXPORT void
pvl_face_recognition_with_db_destroy(pvl_face_recognition_with_db *fr);


/** @brief Reset the instance of this component.
 *
 *  All the internal states, the internal database and context will be reset except the run-time parameters set by user.
 *  The facedata DB that the input instance is synchronized will be also removed.
 *
 *  @param[in]  fr  The handle of this component.
 *
 *  @return     On success, @ref pvl_success.
 *  @return     On failure, @ref pvl_err error code, which will be the one of the following return value(s).
 *
 *  @retval     pvl_err_invalid_argument    Passing null pointer to the params.
 *  @retval     pvl_err_invalid_status      Invalid internal status of the handle or database.
 *  @retval     pvl_err_nomem               Failed in re-allocating the memory.
 *  @retval     pvl_err_interrupted         Facedata fileDB has not been created yet.
 */
PVLIB_EXPORT pvl_err
pvl_face_recognition_with_db_reset(pvl_face_recognition_with_db *fr);


/** @brief Set run-time parameters of this component.
 *
 *  Set given parameters to the handle.
 *  It is required to get proper parameters instance by pvl_face_recognition_with_db_get_parameters() before setting something.
 *  If This function modifies 'db_path' in 'pvl_face_recognition_with_db_parameters', then the new facedata DB of the modified path
 *   will be loaded and registered onto the initialized instance of face recognition.
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
 *  @retval     pvl_err_interrupted         Facedata fileDB has not been created yet.
 */
PVLIB_EXPORT pvl_err
pvl_face_recognition_with_db_set_parameters(pvl_face_recognition_with_db *fr, const pvl_face_recognition_with_db_parameters *params);


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
pvl_face_recognition_with_db_get_parameters(pvl_face_recognition_with_db *fr, pvl_face_recognition_with_db_parameters *params);


/** @brief The main function that run the face recognition for the faces in image.
 *
 *  This function conducts the face recognition by trying to match query faces (input faces) against the faces in the internal database.
 *  If the recognition succeeded, person_id field at corresponding index of the result will be filled with valid (+) person_id.
 *  Otherwise, person_id will be @ref FACE_RECOGNIZER_UNKNOWN_PERSON_ID.
 *  Since the face recognition result could be the false-alarm (i.e. incorrect recognition.),
 *  the positive person_id does not guarantee that the recognition result is correct.
 *  Befor running recognition, this function checks whether the internal data of the input instance is synchronized with the facedata DB or not.
 *  This function loads facedata DB and registers the data if there was modification at the facedata DB file.
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
pvl_face_recognition_with_db_run_in_image(pvl_face_recognition_with_db *fr, const pvl_image *image, int32_t num_faces,
        const pvl_point *left_eyes, const pvl_point *right_eyes,
        pvl_face_recognition_with_db_result *results);


/** @brief Run the face recognition component for the faces in image assuming the image is a part of series of preview frames.
 *
 *  This function is basically the same as pvl_face_recognition_with_db_run_in_image(), but assuming that the input image is the part of the sequence of preview or video frames.
 *  Utilizing results from previous frames, this function shows faster and more reliable recognition results.
 *  Befor running recognition, this function checks whether the internal data of the input instance is synchronized with the facedata DB or not every 30 frames(default).
 *  The number of frames for polling can be changed by setting the parameter 'db_sync_interval'.
 *  This function creates a thread if there was modification at facedata DB file. And this thread loads the facedata DB and registers the data.
 *
 *  @param[in]  fr              The handle of the face recognition component.
 *  @param[in]  image           The input image that contains the query faces (input faces). All image formats are supported.
 *                              pvl_image_format_gray, pvl_image_format_nv12, pvl_iamge_format_nv21 and pvl_image_format_yv12 are preferred in terms of speed.
 *  @param[in]  num_faces       The number of query faces (input faces) to recognize.
 *  @param[in]  left_eyes       The array of the center points on the left eyes of faces to be recognized.
 *  @param[in]  right_eyes      The array of the center points on the right eyes of faces to be recognized.
 *  @param[in]  tracking_ids    Tracking-ids returned from face detector. Using pvl_face_recognition_with_db_run_in_image() is recommended if tracking-id is not available.
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
pvl_face_recognition_with_db_run_in_preview(pvl_face_recognition_with_db *fr, const pvl_image *image, int32_t num_faces,
        const pvl_point *left_eyes, const pvl_point *right_eyes, const int32_t *tracking_ids,
        pvl_face_recognition_with_db_result *results);


/** @brief Register one face data to both the facedata DB and internal memory of the instance.
 *
 *  This function registers the given face data into the database.
 *  Before using pvl_face_recognition_with_db_run_in_image() or pvl_face_recognition_with_db_run_in_preview(),
 *  the user should call this function to register faces to the database.
 *
 *  @param[in]  fr          The handle of the face recognition component.
 *  @param[in]  facedata    A face data instance to be registered.
 *                          The variables of 'facedata' should refer to the description of 'pvl_face_recognition_facedata'
 *
 *  @return     On success, @ref pvl_success.
 *  @return     On failure, @ref pvl_err error code, which will be the one of the following return value(s).
 *
 *  @retval     pvl_err_invalid_argument    Passing null pointer to the fr parameter, or wrong facedata.
 *  @retval     pvl_err_invalid_status      Invalid internal status of the handle.
 *  @retval     pvl_err_nomem               Failed in allocating the memory of the internal buffers or the database.
 *  @retval     pvl_err_database_full       Not enough memory left to insert new data.
 *  @retval     pvl_err_out_of_bound        Already registred the maximum number of 'person_id'
 *  @retval     pvl_err_interrupted         Facedata fileDB has not been created yet.
 */
PVLIB_EXPORT pvl_err
pvl_face_recognition_with_db_register_facedata(pvl_face_recognition_with_db *fr, const pvl_face_recognition_with_db_facedata *facedata);


/** @brief Unregister the face from both the internal memory of the instance and the facedata DB.
 *
 *  This function deletes the face data from the database and the input instance.
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
 *  @retval     pvl_err_interrupted         Facedata fileDB has not been created yet.
 */
PVLIB_EXPORT pvl_err
pvl_face_recognition_with_db_unregister_facedata(pvl_face_recognition_with_db *fr, uint64_t face_id);


/** @brief Unregister all faces associated with the person_id from both the facedata DB and the instance.
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
 *  @retval     pvl_err_interrupted         Facedata fileDB has not been created yet.
 */
PVLIB_EXPORT pvl_err
pvl_face_recognition_with_db_unregister_person(pvl_face_recognition_with_db *fr, int32_t person_id);


/** @brief Update person_id of specified face data from both the facedata DB and the instance.
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
 *  @retval     pvl_err_out_of_bound        Already registred the maximum number of 'person_id'
 *  @retval     pvl_err_no_such_item        Face data not found from the database that matches the given face_id.
 *  @retval     pvl_err_interrupted         Facedata fileDB has not been created yet.
 */
PVLIB_EXPORT pvl_err
pvl_face_recognition_with_db_update_person(pvl_face_recognition_with_db *fr, uint64_t face_id, int32_t new_person_id);


/** @brief Get number of registered faces at the database.
 *
 *  This function returns the number of faces registered at the database.
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
pvl_face_recognition_with_db_get_num_faces_in_database(pvl_face_recognition_with_db *fr);


/** @brief Allocate the memory of result buffers.
 *
 *  This function is used to allocate memory for the result buffers which contain the array of face feature data.
 *  The memory for result buffers should be allocated in the form of the array of 'pvl_face_recognition_with_db_result' before
 *   the functions which conduct face recognition_with_db are called.
 *  This function conducts the memory allocation for the output buffers.
 *
 *  @param[in]  fr            The handle of the face recognition_with_db component.
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
pvl_face_recognition_with_db_create_result_buffer(pvl_face_recognition_with_db *fr, int32_t max_faces, pvl_face_recognition_with_db_result **fr_results);


/** @brief Destroy the memory buffer.
 *
 * This function releases internal allocated buffers and destroys the data.
 *
 *  @param[in]  fr_results   The handle of the result buffer to be destroyed.
 *
 */
PVLIB_EXPORT void
pvl_face_recognition_with_db_destroy_result_buffer(pvl_face_recognition_with_db_result *fr_results);


/** @brief Get next available person ID from DB
 *
 *  This function does not alter the internal status. The ID would increase if it is occupied by 'update_person()' API.
 *
 *  @param[in]  fr          The handle of the face recognition component.
 *
 *  @return     On success, returns new person ID (non-negative value.)
 *  @return     On failure, @ref pvl_err error code, which will be the one of the following return value(s).
 *
 *  @retval     pvl_err_invalid_argument    Passing null pointer to the params.
 *  @retval     pvl_err_invalid_status      Invalid internal status of the handle.
 */
PVLIB_EXPORT int32_t
pvl_face_recognition_with_db_get_new_person_id(const pvl_face_recognition_with_db* fr);

/** @brief Get all the data of registered face feature from DB
 *
 *  This function does not alter the internal status.
 *  The buffer for array of the facedata should be allocated before the function is called.
 *  The function 'pvl_face_recognition_with_db_create_facedata_buffer' can be used for the memory allocation.
 *
 *  @param[in]  fr          The handle of the face recognition component.
 *  @param[in]  num_data    The number of the array of the parameter 'data'.
 *  @param[out] data        The array of the data buffer which will contains the face feature data.
 *                          The number of array should be more that 'max_faces_in_database' in 'pvl_face_recognition_with_db'
 *                          Please refer to 'pvl_face_recognition_with_db_create_facedata_buffer' and 'pvl_face_recognition_with_db_destroy_facedata_buffer'
 *
 *  @return     On success, returns the number of registered face feature data
 *  @return     On failure, @ref pvl_err error code, which will be the one of the following return value(s).
 *
 *  @retval     pvl_err_invalid_argument    Passing null pointer to the params.
 *              pvl_err_invalid_status      Invalid internal status of the handle.
 *              pvl_err_interrupted         Facedata fileDB has not been created yet.
 *
 */
PVLIB_EXPORT int32_t
pvl_face_recognition_with_db_get_registered_facedata(const pvl_face_recognition_with_db *fr, int32_t num_data, pvl_face_recognition_with_db_facedata *data);

/** @brief Allocate the memory of buffers to get face feature data which is registered into internal buffer.
 *
 *  This function is used to allocate memory of buffers to get face feature data which is registered into internal buffer.
 *  The memory for buffers to get face feature data should be allocated in the form of the array of 'pvl_face_recognition_with_db_result'
 *    before the function 'pvl_face_recognition_with_db_get_registered_facedata' is called.
 *  This function conducts the memory allocation for the buffers to get registered face feature data.
 *
 *  @param[in]  fr            The handle of the face recognition_with_db component.
 *  @param[in]  max_data      The maximum number of face feature data, which is used for allocating buffer.
 *  @param[out] data          The initialized array of the face feature data buffer.
 *
 *  @return     On success,   @ref pvl_success.
 *  @return     On failure,   @ref pvl_err error code, which will be the one of the following return value(s).
 *
 *  @retval     pvl_err_invalid_argument    Passing null pointer to the fr or data parameter, or negative value to the max_data.
 *  @retval     pvl_err_out_of_bound        Parameters or condition for the allocation is not acceptable.
 *  @retval     pvl_err_nomem               Failed in allocating the memory of the internal buffers or the database.
 */
PVLIB_EXPORT pvl_err
pvl_face_recognition_with_db_create_facedata_buffer(pvl_face_recognition_with_db *fr, int32_t max_data, pvl_face_recognition_with_db_facedata **data);


/** @brief Destroy the memory buffer of face feature data.
 *
 * This function releases internal allocated buffers and destroys the data.
 *
 *  @param[in]  data     The handle of the face feature data buffer to be destroyed.
 *
 */
PVLIB_EXPORT void
pvl_face_recognition_with_db_destroy_facedata_buffer(pvl_face_recognition_with_db_facedata *data);


#ifdef __cplusplus
}
#endif // __cplusplus

/** @example pvl_face_recognition_with_db_sample.c
 *  Sample of the Extended Face Recognition with DB management
 */

#endif /* __PVL_FACE_RECOGNITION_WITH_DB_H__ */
