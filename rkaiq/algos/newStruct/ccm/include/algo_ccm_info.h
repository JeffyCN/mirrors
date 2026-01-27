/*
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

#ifndef __ALGO_CCM_INFO__

#include "algos/algo_types_priv.h"
#include "st_string.h"

void ccm_dump_mod_param(const RkAiqAlgoCom* self, st_string* result);
void ccm_dump_mod_attr(const RkAiqAlgoCom* self, st_string* result);
void ccm_dump_mod_status(const RkAiqAlgoCom* self, st_string* result);

#endif  // __ALGO_CCM_INFO__
