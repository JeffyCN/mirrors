/*
* for rockchip v2.0.0
*
*  Copyright (c) 2024 Rockchip Corporation
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

#ifndef __RGBIR_TYPES_PRIV_H__
#define __RGBIR_TYPES_PRIV_H__

#include "include/rgbir_algo_api.h"

typedef struct {
    const RkAiqAlgoCom_prepare_t* prepare_params;
    rgbir_api_attrib_t* rgbir_attrib;
    uint32_t *iso_list;
    int working_mode;
    int iso;
    bool isReCal_;
} RgbirContext_t;

#define DEFAULT_RECALCULATE_DELTA_ISO (0.01)

#endif
