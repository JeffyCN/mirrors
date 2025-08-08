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

#ifndef _SAMPLE_A3DLUT_MODULE_H_
#define _SAMPLE_A3DLUT_MODULE_H_

#include "xcore/base/xcam_common.h"

#ifdef  __cplusplus
extern "C" {
#endif
void sample_print_a3dlut_info(const void *arg);
XCamReturn sample_a3dlut_module(const void* arg);
#ifdef USE_NEWSTRUCT
int sample_3dlut_test(const rk_aiq_sys_ctx_t* ctx);
int sample_query_3dlut_status(const rk_aiq_sys_ctx_t* ctx);
int sample_3dlut_setCalib_test(const rk_aiq_sys_ctx_t* ctx);
#endif

#ifdef  __cplusplus
}
#endif

#endif

