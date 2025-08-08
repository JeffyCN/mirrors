/*
 *  Copyright (c) 2022 Rockchip Corporation
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

#ifndef __RK_AUTOBLC_H__
#define __RK_AUTOBLC_H__

// #include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int width;
    int height;
    int isp_ob;

    int rgain;
    int bgain;
    int grgain;
    int gbgain;

    int bayertnr_autoblc_thre;
    int bayertnr_autoblc_rfact;
    int bayertnr_autoblc_gfact;
    int bayertnr_autoblc_bfact;

    unsigned int*   bayertnr_itransf_tbl;

    unsigned char* ptr_rgb_pack;
    int* blc_adjust;
} rk_autoblc_param_t;

extern int rk_autoblc_init(rk_autoblc_param_t* param);
extern int rk_autoblc_proc(rk_autoblc_param_t* param);
extern int rk_autoblc_deinit(rk_autoblc_param_t* param);

#ifdef __cplusplus
}
#endif

#endif /*__RK_AUTOBLC_H__*/
