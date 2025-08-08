/*
* aibnr_types_prvt.h
*
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

#ifndef __AIBNR_TYPES_PRIV_H__
#define __AIBNR_TYPES_PRIV_H__

#include "include/aibnr_algo_api.h"

typedef struct {
    aibnr_api_attrib_t* aibnr_attrib;
    uint32_t *iso_list;
    aibnr_param_t cur_param;
    int working_mode;
    int pre_iso;

    bool doAiisp_force_update;
    bool doAiisp_en;
    int doAiisp_isoL;
    int doAiisp_isoH;
    int fixIndex;

    bool isReCal_;
} AibnrContext_t;

#define DEFAULT_RECALCULATE_DELTA_ISO (0.01)

int aibnr_getFixIndex(const RkAiqAlgoCom* inparams, int *index, bool *force_update);

#endif
