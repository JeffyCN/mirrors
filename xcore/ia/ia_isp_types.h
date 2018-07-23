/*
 * INTEL CONFIDENTIAL
 * Copyright (c) 2014 Intel Corporation
 * All Rights Reserved.
 *
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation or its
 * suppliers or licensors. Title to the Material remains with Intel
 * Corporation or its suppliers and licensors. The Material may contain trade
 * secrets and proprietary and confidential information of Intel Corporation
 * and its suppliers and licensors, and is protected by worldwide copyright
 * and trade secret laws and treaty provisions. No part of the Material may be
 * used, copied, reproduced, modified, published, uploaded, posted,
 * transmitted, distributed, or disclosed in any way without Intel's prior
 * express written permission.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or
 * delivery of the Materials, either expressly, by implication, inducement,
 * estoppel or otherwise. Any license under such intellectual property rights
 * must be express and approved by Intel in writing.
 *
 * Unless otherwise agreed by Intel in writing, you may not remove or alter
 * this notice or any other notice embedded in Materials by Intel or Intels
 * suppliers or licensors in any way.
 */

/*!
 * \file ia_isp_types.h
 * \brief Common ISP enumerations and structures.
*/


#ifndef IA_ISP_TYPES_H_
#define IA_ISP_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

/*!
 *  \brief Complexity level for ISP features.
 */
typedef enum
{
    ia_isp_feature_level_off,   /* Feature is turned off */
    ia_isp_feature_level_low,   /* Minimum set of algorithms are used */
    ia_isp_feature_level_high   /* Maximum set of algorithms are used */
} ia_isp_feature_level;

/*!
 *  \brief Definitions for the color effects.
 */
typedef enum
{
    ia_isp_effect_none     =               0,
    ia_isp_effect_sky_blue =         (1 << 0),
    ia_isp_effect_grass_green =      (1 << 1),
    ia_isp_effect_skin_whiten_low =  (1 << 2),
    ia_isp_effect_skin_whiten =      (1 << 3),
    ia_isp_effect_skin_whiten_high = (1 << 4),
    ia_isp_effect_sepia =            (1 << 5),
    ia_isp_effect_black_and_white =  (1 << 6),
    ia_isp_effect_negative =         (1 << 7),
    ia_isp_effect_vivid =            (1 << 8),
    ia_isp_effect_invert_gamma =     (1 << 9),
    ia_isp_effect_grayscale =        (1 << 10)
} ia_isp_effect;

/*!
 *  \brief Settings for feature level and strength.
 */
typedef struct
{
    ia_isp_feature_level feature_level;  /* Feature level */
    char strength;                       /* Setting for the strength [-128,127]. */
} ia_isp_feature_setting;

#ifdef __cplusplus
}
#endif

#endif /* IA_ISP_TYPES_H_ */
