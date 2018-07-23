/*
 * Copyright 2012 Intel Corporation
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

#ifndef _IA_TYPES_H_
#define _IA_TYPES_H_

#include <stdlib.h>
#include <stdarg.h>

#if defined(_MSC_VER)
    #if !defined(__BOOL_DEFINED)
        typedef unsigned char bool;
        #define true 1
        #define false 0
    #endif
#else
    #include <stdbool.h>
#endif

#ifdef _WIN32
#define LIBEXPORT __declspec(dllexport)
#else
#define LIBEXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 *  \brief Bundles binary data pointer with size.
 */
typedef struct
{
    void        *data;
    unsigned int size;
} ia_binary_data;

typedef struct {
    int major;
    int minor;
} ia_version;

typedef enum {
    CAPTURE_OUTPUT,
    CAPTURE_VFPP,
    PREVIEW_VFPP,
    ACC_QOS
} ia_acceleration_fw_dst;

typedef struct {
    void *isp;
    void *(*open_firmware)     (const char *fw_path, unsigned *size);
    int   (*load_firmware)     (void *isp, void *fw, unsigned size, unsigned *handle);
    int   (*unload_firmware)   (void *isp, unsigned handle);
    int   (*map_firmware_arg)  (void *isp, void *usr_ptr, size_t size, unsigned long *css_ptr);
    int   (*unmap_firmware_arg)(void *isp, unsigned long css_ptr, size_t size);
    int   (*set_firmware_arg)  (void *isp, unsigned handle, unsigned num, void *val, size_t size);
    int   (*set_mapped_arg)    (void *isp, unsigned handle, unsigned mem, unsigned long css_ptr, size_t size);
    int   (*start_firmware)    (void *isp, unsigned handle);
    int   (*wait_for_firmware) (void *isp, unsigned handle);
    int   (*abort_firmware)    (void *isp, unsigned handle, unsigned timeout);
    ia_version version_isp;
    ia_version version_css;
    int   (*set_stage_state)   (void *isp, unsigned int handle, bool enable);
    int   (*wait_stage_update) (void *isp, unsigned int handle);
    int   (*load_firmware_ext) (void *isp, void *fw, unsigned size, unsigned *handle, int fw_dst);
} ia_acceleration;

typedef enum {
    ia_frame_format_nv12,   /* 12 bit YUV 420, Y, UV plane */
    ia_frame_format_yuv420, /* 12 bit YUV 420, Y, U, V plane */
    ia_frame_format_yuv444, /* 24 bit YUV 444, Y, U, V plane */
    ia_frame_format_raw,    /* RAW, 1 plane */
    ia_frame_format_rgba32, /* RGBA 8 bits per channel */
    ia_frame_format_yuy2,   /* 16 bit YUV 422, YUYV plane */
    ia_frame_format_raw16,  /* 16 bit RAW, 1 plane */
    ia_frame_format_rgb16p, /* 16 bits per channel, 3 planes */
} ia_frame_format;

typedef struct {
    void           *data;     /**< Pointer to the image data */
    int             size;     /**< Total number of bytes in data*/
    int             width;    /**< Width of the frame in pixels */
    int             height;   /**< Height of the frame in lines */
    ia_frame_format format;   /**< Frame format */
    int             stride;   /**< Stride, bytes per line*/
    int             rotation; /**< degrees 0-360 */
} ia_frame;

/** Floating point range struct. */
typedef struct {
    float min;
    float max;
} ia_float_range;

/*!
 * Needs to be agreed with ia_imaging error codes.
 * \brief Error codes.
*/
typedef enum {
    ia_err_none     =  0,       /*!< No errors*/
    ia_err_general  = (1 << 1), /*!< General error*/
    ia_err_nomemory = (1 << 2), /*!< Out of memory*/
    ia_err_data     = (1 << 3), /*!< Corrupted data*/
    ia_err_internal = (1 << 4), /*!< Error in code*/
    ia_err_argument = (1 << 5)  /*!< Invalid argument for a function*/
} ia_err;

typedef struct {
    int left;
    int top;
    int right;
    int bottom;
} ia_rectangle;

/** Vector, used for image stabilization. */
typedef struct {
    int x;
    int y;
} ia_vector;

/** Coordinate, used in red-eye correction. */
typedef struct {
    int x;
    int y;
} ia_coordinate;

typedef struct {
    void (*vdebug)(const char *fmt, va_list ap);
    void (*verror)(const char *fmt, va_list ap);
    void (*vinfo)(const char *fmt, va_list ap);
} ia_env;

typedef struct {
    void *(*alloc) (void *isp, size_t size);
    void  (*free)  (void *isp, void *usr_ptr, size_t size);
} ia_mem_env;

#ifdef __cplusplus
}
#endif

#endif /* _IA_TYPES_H_ */
