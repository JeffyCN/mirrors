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

#ifndef _AIQ_STREAM_PROC_INFO_H_
#define _AIQ_STREAM_PROC_INFO_H_

#include "hwi_c/aiq_rawStreamProcUnit.h"

void stream_proc_dump_mod_param(AiqRawStreamProcUnit_t* self, st_string* result);
void stream_proc_dump_dev_attr(AiqRawStreamProcUnit_t* self, st_string* result);
void stream_proc_dump_chn_attr1(AiqRawStreamProcUnit_t* self, st_string* result);
void stream_proc_dump_chn_status1(AiqRawStreamProcUnit_t* self, st_string* result);
void stream_proc_dump_chn_status2(AiqRawStreamProcUnit_t* self, st_string* result);
void stream_proc_dump_videobuf_status(AiqRawStreamProcUnit_t* self, st_string* result);

#endif  // _AIQ_STREAM_PROC_INFO_H_
