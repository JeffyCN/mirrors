/*
 *  Copyright (c) 2019 Rockchip Corporation
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

#ifndef _SAMPLE_ACCM_MODULE_H_
#define _SAMPLE_ACCM_MODULE_H_

#include "xcore/base/xcam_common.h"

#ifdef  __cplusplus
extern "C" {
#endif
void sample_print_accm_info(const void *arg);
XCamReturn sample_accm_module(const void *arg);
void sample_print_accm_v2_info(const void *arg);
XCamReturn sample_accm_v2_module(const void* arg);
XCamReturn sample_accm_v3_module(const void* arg);
#ifdef USE_NEWSTRUCT
int sample_ccm_test(const rk_aiq_sys_ctx_t* ctx);
int sample_query_ccm_status(const rk_aiq_sys_ctx_t* ctx);
int sample_ccm_setCalib_test(const rk_aiq_sys_ctx_t* ctx);
#endif
#ifdef  __cplusplus
}
#endif
#endif
