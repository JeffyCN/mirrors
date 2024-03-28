/*
 * Copyright (c) 2023 Rockchip Eletronics Co., Ltd.
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

#ifndef _RK_AIQ_PARAM_GAMMA21_H_
#define _RK_AIQ_PARAM_GAMMA21_H_
#define CALIBDB_GAMMA_KNOTS_NUM_V11 49

typedef struct {
    int8_t equ_segm;
    bool EnableDot49;
    // M4_NUMBER_DESC("Gamma_out_offset", "u16", M4_RANGE(0,4095), "0", M4_DIGIT(0))
    uint16_t Gamma_out_offset;
    // M4_ARRAY_MARK_DESC("Gamma_curve", "u16", M4_SIZE(1,49),  M4_RANGE(0, 4095), "[0, 93, 128, 154, 175, 194, 211, 226, 240, 266, 289, 310, 329, 365, 396, 425, 451, 499, 543, 582, 618, 684, 744, 798, 848, 938, 1019, 1093, 1161, 1285, 1396, 1498, 1592, 1761, 1914, 2052, 2181, 2414, 2622, 2813, 2989, 3153, 3308, 3454, 3593, 3727, 3854, 3977, 4095]", M4_DIGIT(0), M4_DYNAMIC(0), "curve_table")
    uint16_t Gamma_curve[CALIBDB_GAMMA_KNOTS_NUM_V11];
} gamma_params_static_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(static_param),
        M4_TYPE(struct),
        M4_UI_MODULE(static_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The static params of gamma module))  */
    gamma_params_static_t sta;
} gamma_param_t;

#endif
