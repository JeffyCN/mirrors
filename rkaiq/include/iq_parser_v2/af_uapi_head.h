/*
 *  Copyright (c) 2021 Rockchip Corporation
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

#ifndef __AF_UAPI_HEAD_H__
#define __AF_UAPI_HEAD_H__


#include "uAPI2/rk_aiq_user_api_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    // M4_NUMBER_DESC("roia_sharpness", "u64", M4_RANGE(0, 18446744073709551616), "0", M4_DIGIT(0))
    unsigned long long roia_sharpness;
    // M4_NUMBER_DESC("roia_luminance", "u32", M4_RANGE(0, 4294967296), "0", M4_DIGIT(0))
    unsigned int roia_luminance;
    // M4_NUMBER_DESC("roib_sharpness", "u32", M4_RANGE(0, 4294967296), "0", M4_DIGIT(0))
    unsigned int roib_sharpness;
    // M4_NUMBER_DESC("roib_luminance", "u32", M4_RANGE(0, 4294967296), "0", M4_DIGIT(0))
    unsigned int roib_luminance;
    // M4_ARRAY_DESC("global_sharpness", "u32", M4_SIZE(15,15), M4_RANGE(0,4294967296), "0", M4_DIGIT(0), M4_DYNAMIC(0))
    unsigned int global_sharpness[225];
    // M4_ARRAY_DESC("lowpass_fv4_4", "u32", M4_SIZE(15,15), M4_RANGE(0,4294967296), "0", M4_DIGIT(0), M4_DYNAMIC(0))
    int lowpass_fv4_4[225];
    // M4_ARRAY_DESC("lowpass_fv8_8", "u32", M4_SIZE(15,15), M4_RANGE(0,4294967296), "0", M4_DIGIT(0), M4_DYNAMIC(0))
    int lowpass_fv8_8[225];
    // M4_ARRAY_DESC("lowpass_highlht", "u32", M4_SIZE(15,15), M4_RANGE(0,4294967296), "0", M4_DIGIT(0), M4_DYNAMIC(0))
    int lowpass_highlht[225];
    // M4_ARRAY_DESC("lowpass_highlht2", "u32", M4_SIZE(15,15), M4_RANGE(0,4294967296), "0", M4_DIGIT(0), M4_DYNAMIC(0))
    int lowpass_highlht2[225];

} uapi_af_v20stats_t;

typedef struct {
    // M4_NUMBER_DESC("wndb_luma", "u32", M4_RANGE(0, 4294967296), "0", M4_DIGIT(0))
    unsigned int wndb_luma;
    // M4_NUMBER_DESC("wndb_sharpness", "u32", M4_RANGE(0, 4294967296), "0", M4_DIGIT(0))
    unsigned int wndb_sharpness;
    // M4_NUMBER_DESC("winb_highlit_cnt", "u32", M4_RANGE(0, 4294967296), "0", M4_DIGIT(0))
    unsigned int winb_highlit_cnt;
    // M4_ARRAY_DESC("wnda_luma", "u32", M4_SIZE(15,15), M4_RANGE(0,4294967296), "0", M4_DIGIT(0), M4_DYNAMIC(0))
    unsigned int wnda_luma[225];
    // M4_ARRAY_DESC("wnda_fv_v1", "u32", M4_SIZE(15,15), M4_RANGE(0,4294967296), "0", M4_DIGIT(0), M4_DYNAMIC(0))
    unsigned int wnda_fv_v1[225];
    // M4_ARRAY_DESC("wnda_fv_v2", "u32", M4_SIZE(15,15), M4_RANGE(0,4294967296), "0", M4_DIGIT(0), M4_DYNAMIC(0))
    unsigned int wnda_fv_v2[225];
    // M4_ARRAY_DESC("wnda_fv_h1", "u32", M4_SIZE(15,15), M4_RANGE(0,4294967296), "0", M4_DIGIT(0), M4_DYNAMIC(0))
    unsigned int wnda_fv_h1[225];
    // M4_ARRAY_DESC("wnda_fv_h2", "u32", M4_SIZE(15,15), M4_RANGE(0,4294967296), "0", M4_DIGIT(0), M4_DYNAMIC(0))
    unsigned int wnda_fv_h2[225];
    // M4_ARRAY_DESC("wina_highlit_cnt", "u32", M4_SIZE(15,15), M4_RANGE(0,4294967296), "0", M4_DIGIT(0), M4_DYNAMIC(0))
    unsigned int wina_highlit_cnt[225];
} uapi_af_v30stats_t;

typedef struct {
    // M4_NUMBER_DESC("wndb_luma", "u32", M4_RANGE(0, 4294967296), "0", M4_DIGIT(0))
    unsigned int wndb_luma;
    // M4_NUMBER_DESC("wndb_sharpness", "u32", M4_RANGE(0, 4294967296), "0", M4_DIGIT(0))
    unsigned int wndb_sharpness;
    // M4_NUMBER_DESC("winb_highlit_cnt", "u32", M4_RANGE(0, 4294967296), "0", M4_DIGIT(0))
    unsigned int winb_highlit_cnt;
    // M4_ARRAY_DESC("wnda_luma", "u32", M4_SIZE(5,5), M4_RANGE(0,4294967296), "0", M4_DIGIT(0), M4_DYNAMIC(0))
    unsigned int wnda_luma[25];
    // M4_ARRAY_DESC("wnda_fv_v1", "u32", M4_SIZE(5,5), M4_RANGE(0,4294967296), "0", M4_DIGIT(0), M4_DYNAMIC(0))
    unsigned int wnda_fv_v1[25];
    // M4_ARRAY_DESC("wnda_fv_v2", "u32", M4_SIZE(5,5), M4_RANGE(0,4294967296), "0", M4_DIGIT(0), M4_DYNAMIC(0))
    unsigned int wnda_fv_v2[25];
    // M4_ARRAY_DESC("wnda_fv_h1", "u32", M4_SIZE(5,5), M4_RANGE(0,4294967296), "0", M4_DIGIT(0), M4_DYNAMIC(0))
    unsigned int wnda_fv_h1[25];
    // M4_ARRAY_DESC("wnda_fv_h2", "u32", M4_SIZE(5,5), M4_RANGE(0,4294967296), "0", M4_DIGIT(0), M4_DYNAMIC(0))
    unsigned int wnda_fv_h2[25];
    // M4_ARRAY_DESC("wina_highlit_cnt", "u32", M4_SIZE(5,5), M4_RANGE(0,4294967296), "0", M4_DIGIT(0), M4_DYNAMIC(0))
    unsigned int wina_highlit_cnt[25];
} uapi_af_v32litestats_t;

#ifdef __cplusplus
}
#endif


#endif  /*__AF_UAPI_HEAD_H__*/
