/*
 * Copyright (c) 2021-2022 Rockchip Eletronics Co., Ltd.
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
 */
#ifndef _SAMPLE_COMM_H_
#define _SAMPLE_COMM_H_

#include <stdio.h>
#include "rkisp_demo.h"
#include "common/rk_aiq_comm.h"

#if ISP_HW_V33
#include "uAPI2/rk_aiq_user_api2_isp33.h"
#elif ISP_HW_V39
#include "uAPI2/rk_aiq_user_api2_isp39.h"
#elif ISP_HW_V35
#include "uAPI2/rk_aiq_user_api2_isp35.h"
#endif

#define CLEAR() printf("\x1b[2J\x1b[H");
#define RESET_CURSOR() printf("\033[H");

#define RKAIQ_SAMPLE_CHECK_RET(ret, format, ...) \
    if (ret) { \
        printf(format, ##__VA_ARGS__); \
        return ret; \
    }

enum RK_ISP_HW_MODULE_e {
    RK_ISP_AE,
    RK_ISP_AWB,
    RK_ISP_AWB32,
    RK_ISP_AF,
    RK_ISP_ACCM,
    RK_ISP_A3DLUT,
    RK_ISP_ADRC,
    RK_ISP_AMERGE,
    RK_ISP_AGAMMA,
    RK_ISP_ADEHAZE,
    RK_ISP_AYNR,
    RK_ISP_ACNR,
    RK_ISP_ASHARP,
    RK_ISP_ABAYER2DNR,
    RK_ISP_ABAYERTNR,
    RK_ISP_CSM,
    RK_ISP_ABLC,
    RK_ISP_AGIC,
#ifndef USE_NEWSTRUCT
    RK_ISP_ALDCH,
#endif
    RK_ISP_ADEBAYER,
    RK_ISP_ACP,
    RK_ISP_AIE,
    RK_ISP_CGC,
    RK_ISP_CAC,
    RK_ISP_AGAIN,
    RK_ISP_ACCM_V2,
#ifndef USE_NEWSTRUCT
    RK_ISP_ALDCH_V21,
#endif
    RK_SMARTIR,
    RK_ISP_ALDC,
    RK_AINR,
    RK_ISP_MAX,
};

typedef int (*uapi_case_func)(const void*);

typedef struct __uapi_case {
    // description of this uapi
    const char* desc;
    // uapi function
    uapi_case_func func;
} uapi_case_t;


#ifdef  __cplusplus
extern "C" {
#endif
int uapi_usage_show(uapi_case_t* uapi_list);

int uapi_list_count(uapi_case_t* uapi_list);

int uapi_process_loop(const rk_aiq_sys_ctx_t* ctx, uapi_case_t* uapi_list);
#ifdef  __cplusplus
}
#endif

#endif

