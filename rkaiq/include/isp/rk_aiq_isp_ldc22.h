/*
 * sw_param_ldch22.h
 *
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
#ifndef _RK_AIQ_PARAM_LDC22_H_
#define _RK_AIQ_PARAM_LDC22_H_

#include <stdbool.h>
#include <stdint.h>

#ifndef ISP32_LDCH_BIC_NUM
#define ISP32_LDCH_BIC_NUM 36
#endif

typedef struct ldc_lutMapCfg_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ldcT_lutMap_size),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,4095),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
        Freq of use: low))  */
    uint32_t sw_ldcT_lutMap_size;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ldcT_lutMapBuf_vaddr),
        M4_TYPE(u32),
        M4_SIZE_EX(1,2),
        M4_RANGE_EX(0,4095),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
        Freq of use: low))  */
    void* sw_ldcT_lutMapBuf_vaddr[2];
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ldchT_lutMapBuf_fd),
        M4_TYPE(u32),
        M4_SIZE_EX(1,2),
        M4_RANGE_EX(0,4095),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
        Freq of use: low))  */
    uint32_t sw_ldcT_lutMapBuf_fd[2];
} ldc_lutMapCfg_t;

typedef struct ldc_ldch_params_cfg_s {
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
       M4_ALIAS(lutMapCfg),
       M4_TYPE(struct),
       M4_DEFAULT(0),
       M4_HIDE_EX(1),
       M4_RO(0),
       M4_ORDER(0),
       M4_NOTES(The hardware enable flag))  */
    ldc_lutMapCfg_t lutMapCfg;
} ldc_ldch_params_cfg_t;

typedef struct ldc_ldcv_params_cfg_s {
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
       M4_ALIAS(lutMapCfg),
       M4_TYPE(struct),
       M4_DEFAULT(0),
       M4_HIDE_EX(1),
       M4_RO(0),
       M4_ORDER(0),
       M4_NOTES(The hardware enable flag))  */
    ldc_lutMapCfg_t lutMapCfg;
} ldc_ldcv_params_cfg_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(ldchCfg),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.))  */
    ldc_ldch_params_cfg_t ldchCfg;
#if ISP_HW_V39
    /* M4_GENERIC_DESC(
        M4_ALIAS(ldcvCfg),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.))  */
    ldc_ldcv_params_cfg_t ldcvCfg;
#endif
} ldc_params_static_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sta),
        M4_TYPE(struct),
        M4_UI_MODULE(static_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    ldc_params_static_t sta;
} ldc_param_t;

#endif
