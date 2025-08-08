/*
 *  Copyright (c) 2023 Rockchip Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef _RK_AIQ_API_TYPES_LDC_H_
#define _RK_AIQ_API_TYPES_LDC_H_

#include "common/rk_aiq_comm.h"
#include "isp/rk_aiq_isp_ldc22.h"

RKAIQ_BEGIN_DECLARE

typedef struct ldc_extMeshFile_static_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ldcT_meshfile_path),
        M4_TYPE(string),
        M4_SIZE_EX(1,1),
        M4_DEFAULT("/mnt"),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
                Freq of use: low))  */
    char sw_ldcT_extMeshFile_path[256];
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ldcT_meshfile_path),
        M4_TYPE(string),
        M4_SIZE_EX(1,1),
        M4_DEFAULT("ldch_mesh.bin"),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
                Freq of use: low))  */
    char sw_ldcT_ldchExtMeshFile_name[256];
#if defined(ISP_HW_V39)
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ldcT_meshfile_path),
        M4_TYPE(string),
        M4_SIZE_EX(1,1),
        M4_DEFAULT("ldcv_mesh.bin"),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
                Freq of use: low))  */
    char sw_ldcT_ldcvExtMeshFile_name[256];
#endif
} ldc_extMeshFile_static_t;

typedef struct ldc_lensDistorCoeff_static_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ldcC_opticCenter_x),
        M4_TYPE(f64),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(-10000000000000000,10000000000000000),
        M4_DEFAULT(0),
        M4_DIGIT_EX(16),
        M4_HIDE_EX(0),
        M4_RO(1),
        M4_ORDER(0),
        M4_NOTES(TODO.
        Freq of use: low))  */
    double sw_ldcC_opticCenter_x;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ldcC_opticCenter_y),
        M4_TYPE(f64),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(-10000000000000000,10000000000000000),
        M4_DEFAULT(0),
        M4_DIGIT_EX(16),
        M4_HIDE_EX(0),
        M4_RO(1),
        M4_ORDER(0),
        M4_NOTES(TODO.
        Freq of use: low))  */
    double sw_ldcC_opticCenter_y;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ldcC_lensDistor_coeff),
        M4_TYPE(f64),
        M4_SIZE_EX(1,4),
        M4_RANGE_EX(-10000000000000000,10000000000000000),
        M4_DEFAULT(0),
        M4_DIGIT_EX(16),
        M4_HIDE_EX(0),
        M4_RO(1),
        M4_ORDER(0),
        M4_NOTES(the distortion coefficient of the lens.
        Freq of use: low))  */
    double sw_ldcC_lensDistor_coeff[4];
} ldc_lensDistorCoeff_static_t;

typedef struct ldc_autoGenMesh_static_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(lensDistorCoeff),
        M4_TYPE(struct),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(1),
        M4_ORDER(0),
        M4_NOTES(TODO.))  */
    ldc_lensDistorCoeff_static_t lensDistorCoeff;
    /* M4_GENERIC_DESC(
        M4_ALIAS( sw_ldcC_correct_strg),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(255),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
        Freq of use: low))  */
    unsigned char sw_ldcT_correctStrg_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ldcC_correctLevel_max),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(255),
        M4_HIDE_EX(0),
        M4_RO(1),
        M4_ORDER(0),
        M4_NOTES(TODO.
        Freq of use: low))  */
    unsigned char sw_ldcC_correctStrg_maxLimit;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ldcT_saveMaxFovX_bit),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.))  */
    bool sw_ldcT_saveMaxFovX_bit;
} ldc_autoGenMesh_static_t;

typedef enum ldc_enMode_static_s {
    LDC_LDCH_EN = 0,
    LDC_LDCH_LDCV_EN
} ldc_enMode_static_t;

typedef struct ldc_param_auto_s {
#if defined(ISP_HW_V39)
    /* M4_GENERIC_DESC(
        M4_ALIAS(enMode),
        M4_TYPE(enum),
        M4_DEFAULT(LDC_LDCH_EN),
        M4_HIDE_EX(0),
        M4_RO(1),
        M4_ORDER(0),
        M4_NOTES(The hardware enable flag))  */
     ldc_enMode_static_t enMode;
#endif
    /* M4_GENERIC_DESC(
        M4_ALIAS(autoGenMesh),
        M4_TYPE(struct),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Automatically generate mesh based on calibration parameters.))  */
     ldc_autoGenMesh_static_t autoGenMesh;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ldcC_extMeshfile),
        M4_TYPE(struct),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Using external mesh files.))  */
     ldc_extMeshFile_static_t extMeshFile;
} ldc_param_auto_t;

typedef struct ldc_api_attrib_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(opMode),
        M4_TYPE(enum),
        M4_ENUM_DEF(rk_aiq_op_mode_t),
        M4_DEFAULT(RK_AIQ_OP_MODE_AUTO),
        M4_HIDE_EX(1),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The current operation mode))  */
    rk_aiq_op_mode_t opMode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(enable),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(1),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The hardware enable flag))  */
    bool en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(bypass),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(1),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The hardware enable flag))  */
    bool bypass;
    /* M4_GENERIC_DESC(
        M4_ALIAS(tunning),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The params of auto mode, be only valid for auto mode))  */
    ldc_param_auto_t tunning;
} ldc_api_attrib_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(opMode),
        M4_TYPE(enum),
        M4_ENUM_DEF(rk_aiq_op_mode_t),
        M4_DEFAULT(RK_AIQ_OP_MODE_AUTO),
        M4_HIDE_EX(1),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The current operation mode))  */
    rk_aiq_op_mode_t opMode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(enable),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(1),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The hardware enable flag))  */
    bool en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(enable),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(1),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The hardware enable flag))  */
    bool bypass;
    /* M4_GENERIC_DESC(
        M4_ALIAS(tunning),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The params of auto mode, be only valid for auto mode))  */
    ldc_param_auto_t tunning;
} ldc_status_t;

RKAIQ_END_DECLARE

#endif
