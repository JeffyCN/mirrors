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
 */

#ifndef __RK_AIQ_DUMMY_FUNCTION_H_
#define __RK_AIQ_DUMMY_FUNCTION_H_

#ifndef RKAIQ_HAVE_DUMPSYS
#include "st_string.h"

void aiq_string_printf(st_string *s, const char *fmt, ...);
void aiq_info_dump_title(st_string* result, const char* titleName);
#endif

#endif // __RK_AIQ_DUMMY_FUNCTION_H_

