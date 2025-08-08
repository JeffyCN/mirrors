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

#ifndef _AIQ_GROUP_ANALYZER_INFO_H_

#include "aiq_core.h"
#include "st_string.h"

void group_analyzer_dump_mod_param(AiqAnalyzeGroupManager_t* self, st_string* result);
void group_analyzer_dump_attr(AiqAnalyzeGroupManager_t* self, st_string* result);
void group_analyzer_dump_msg_hdl_status(AiqAnalyzeGroupManager_t* self, st_string* result);
void group_analyzer_dump_msg_map_status1(AiqAnalyzeGroupManager_t* self, st_string* result);
void group_analyzer_dump_msg_map_status2(AiqAnalyzeGroupManager_t* self, st_string* result);

#endif  //  _AIQ_GROUP_ANALYZER_INFO_H_
