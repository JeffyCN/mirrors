/*
 *   Copyright (c) 2024 Rockchip Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *
 */

#ifndef _AIQ_SENSOR_HW_INFO_H_
#define _AIQ_SENSOR_HW_INFO_H_

#include "hwi_c/aiq_sensorHw.h"

void sensor_dump_mod_param(AiqSensorHw_t* self, st_string* result);
void sensor_dump_dev_attr1(AiqSensorHw_t* self, st_string* result);
void sensor_dump_dev_attr2(AiqSensorHw_t* self, st_string* result);
void sensor_dump_reg_effect_delay(AiqSensorHw_t* self, st_string* result);
void sensor_dump_exp_list_size(AiqSensorHw_t* self, st_string* result);
void sensor_dump_configured_exp(AiqSensorHw_t* self, st_string* result);

#endif  // _AIQ_SENSOR_HW_INFO_H_
