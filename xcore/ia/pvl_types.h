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
#ifndef __PVL_TYPES_H__
#define __PVL_TYPES_H__

/** @file    pvl_types.h
 *  @brief   This file declares common structures for PVL.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdarg.h>

#if (defined WIN32 || defined _WIN32)
#define PVLIB_EXPORT __declspec(dllexport)
#else
#define PVLIB_EXPORT
#endif


/** @brief A structure to represent a rectangle.
 */
struct pvl_rect {
    int32_t left;
    int32_t top;
    int32_t right;
    int32_t bottom;
};
typedef struct pvl_rect pvl_rect;


/** @brief A structure to represent a 2D point.
 */
struct pvl_point {
    int32_t x;
    int32_t y;
};
typedef struct pvl_point pvl_point;


/** @brief A structure to represent a size.
 */
struct pvl_size {
    int32_t width;
    int32_t height;
};
typedef struct pvl_size pvl_size;


/** @brief A structure to represent 2D vector type composed with single precision floating types.
 */
struct pvl_vector2d {
    float x;
    float y;
};
typedef struct pvl_vector2d pvl_vector2d;


/** @brief The enumerated values of the definitions of the error codes.
 */
enum pvl_err {
    pvl_success                 =  0,   /**< Successfully completed. */
    pvl_err_general             = -1,   /**< Function returned with unspecified error which does not fit to any other error codes. */
    pvl_err_not_supported       = -2,   /**< Specified parameter is not supported by the library. */
    pvl_err_invalid_argument    = -3,   /**< At least one function parameter is not valid. */
    pvl_err_out_of_bound        = -4,   /**< Specified parameter exceeds limit of allowed value. */
    pvl_err_interrupted         = -5,   /**< Function was interrupted due to change in internal state (usually caused by another thread.) */
    pvl_err_invalid_status      = -6,   /**< Operation cannot be completed because precondition is not met or information is insufficient. */
    pvl_err_nomem               = -7,   /**< Memory allocation has failed. */
    pvl_err_database_full       = -8,   /**< Internal database is full. */
    pvl_err_no_such_item        = -9,   /**< Requested item to look up was not found in the internal database. */
};
typedef enum pvl_err pvl_err;


/** @brief The enumerated values of the definition of the boolean type.
 */
enum pvl_bool {
    pvl_false   = 0,
    pvl_true    = 1
};
typedef enum pvl_bool pvl_bool;


/** @brief A structure to represent a version information.
 */
struct pvl_version {
    const uint16_t major;
    const uint16_t minor;
    const uint16_t patch;
    const char *description;
};
typedef struct pvl_version pvl_version;


/** @brief The enumerated values of the definitions of possible image types.
 */
enum pvl_image_format {
    pvl_image_format_nv12,     /**< 12 bit YUV 420, Y plane first followed by UV-interleaved plane. e.g. YYYYYYYY UVUV */
    pvl_image_format_yv12,     /**< 12 bit YUV 420, Y plane first, U plane and then V plane. e.g. YYYYYYYY UU VV */
    pvl_image_format_gray,     /**< 8 bit, Y plane only. */
    pvl_image_format_rgba32,   /**< 32 bit RGBA, 8 bits per channel. e.g. RGBA RGBA RGBA  */
    pvl_image_format_yuy2,     /**< 16 bit YUV 422, YUYV interleaved. e.g. YUYV YUYV YUYV */
    pvl_image_format_nv21,     /**< 12 bit YUV 420, Y plane first followed by VU-interleaved plane. e.g. YYYYYYYY VUVU */
};
typedef enum pvl_image_format pvl_image_format;


/** @brief A structure to represent the image for PVL engines.
 */
struct pvl_image {
    uint8_t *data;             /**< The pointer to the image data */
    uint32_t size;             /**< The total number of bytes of the data */
    int32_t width;             /**< The width of the image in pixels */
    int32_t height;            /**< The height of the image in lines */
    pvl_image_format format;   /**< The format of the image */
    int32_t stride;            /**< The stride that represents the number of bytes per row. */
    int32_t rotation;          /**< The rotation of the image. (in degree, [0..360]) */
};
typedef struct pvl_image pvl_image;


#ifdef __cplusplus
}
#endif

#endif // #ifndef __PVL_TYPES_H__
