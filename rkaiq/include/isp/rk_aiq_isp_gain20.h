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

#ifndef _RK_AIQ_PARAM_GAIN20_H_
#define _RK_AIQ_PARAM_GAIN20_H_

typedef struct {
    // M4_BOOL_DESC("hdrgain_ctrl_enable", "1")
    bool hdrgain_ctrl_enable;
    // M4_NUMBER_DESC("hdr_gain_scale_s", "f32", M4_RANGE(0, 128.0), "1.0", M4_DIGIT(2))
    float hdr_gain_scale_s;
    // M4_NUMBER_DESC("hdr_gain_scale_m", "f32", M4_RANGE(0, 128.0), "1.0", M4_DIGIT(2))
    float hdr_gain_scale_m;
} gain_params_dyn_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(dyn),
        M4_TYPE(struct),
        M4_UI_MODULE(dynamic_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    gain_params_dyn_t dyn;
} gain_param_t;

#endif
