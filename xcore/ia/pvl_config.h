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
#ifndef __PVL_CONFIG_H__
#define __PVL_CONFIG_H__

/** @file   pvl_config.h
 *  @brief  This file defines common data structures for configuration of components.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "pvl_types.h"


/** @brief A structure to hold HW acceleration contexts.
 *  This structure is not actively used now and subjected to change for the future release.
 */
struct pvl_acceleration {
    const pvl_bool is_supporting_gpu;   /**< The supportability of the GPU acceleration by the component. The value should be defined by each component. */
    const pvl_bool is_supporting_ipu;   /**< The supportability of the IPU acceleration by the component. The value should be defined by each component. */

    pvl_bool enable_gpu;                /**< GPU acceleration switch. Setting pvl_true enables the GPU acceleration mode of the component.
                                             The default value is defined by each component, and the possible values are limited by 'is_supporting_gpu'. */
    pvl_bool enable_ipu;                /**< IPU acceleration switch. Setting pvl_true enables the IPU acceleration mode of the component.
                                             The default value is defined by each component, and the possible values are limited by 'is_supporting_ipu'. */

    void *gpu_context;                  /**< The user-provided GPU acceleration context data. */
    void *ipu_context;                  /**< The user-provided IPU acceleration context data. */
};
typedef struct pvl_acceleration pvl_acceleration;


/** @brief A structure to hold default or user-provided log functions.
 *  This structure is not actively used now and subject to change in future release.
 */
struct pvl_log_apis {
    void (*debug)(const char *fmt, va_list ap);             /**< The logging function for debugging. */
    void (*error)(const char *fmt, va_list ap);             /**< The logging function for error reporting. */
    void (*warning)(const char *fmt, va_list ap);           /**< The logging function for warning message. */
    void (*info)(const char *fmt, va_list ap);              /**< The logging function for information. */
    void (*dump)(int32_t id, uint8_t *data, uint32_t size); /**< The logging function for dumping binary data. */
};
typedef struct pvl_log_apis pvl_log_apis;


/**
 * @brief A structure to hold the configuration of the component.
 *
 * This structure is used as a parameter for the initial configuration of the component.
 *
 * @code
 * // Sample usage
 * pvl_err pvl_xxx_get_default_config(pvl_config* config);
 * pvl_err pvl_xxx_create(const pvl_config* config, pvl_xxx** component);
 * @endcode
 *
 */
struct pvl_config {
    const pvl_version version;      /**< The version information of the component (non-modifiable). */
    pvl_acceleration acceleration;  /**< The configuration of the HW acceleration. The user can enable or disable the HW acceleration via this structure */
    pvl_log_apis log_apis;          /**< The logging functions. The users can dispatch logging messages via this structure. */
};
typedef struct pvl_config pvl_config;


#ifdef __cplusplus
}
#endif


#endif // __PVL_TYPES_H__
