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

#ifndef __IE_TYPES_PRIV_H__
#define __IE_TYPES_PRIV_H__

#include "include/ie_algo_api.h"

typedef struct {
    int16_t skip_frame;
    int mode;
} ie_info_t;
typedef struct {
    ie_api_attrib_t* ie_attrib;
    int mode;
    int16_t skip_frame;
    ie_info_t pre_params;
    bool isReCal_;
} IeContext_t;

#define DEFAULT_RECALCULATE_DELTA_ISO (0.01)

#endif
