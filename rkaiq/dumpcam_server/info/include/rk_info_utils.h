/* Copyright 2020 Rockchip Electronics Co. LTD
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

#ifndef INCLUDE_RK_INFO_UTILS_H__
#define INCLUDE_RK_INFO_UTILS_H__

#ifdef HAVE_RK_OSAL
#include "osal_list.h"
#else
#include "common/list.h"
#endif
#include "st_string.h"
#include "dumpcam_server/include/rkaiq_type.h"

#ifndef RKAIQ_HAVE_DUMPSYS
#include "rk_aiq_dummy_function.h"
#endif

#define  MAX_LINE_LENGTH  1024
#define  MAX_TITLE_NAME   30

typedef struct {
    char *str;
    struct list_head list;
} string_list;

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif  /* End of #if __cplusplus */
#endif /* End of #ifdef __cplusplus */

RK_VOID aiq_info_dump_title(st_string* result, const char* titleName);
RK_VOID aiq_info_dump_title_from_list(st_string *result, string_list *titleList);
RK_VOID aiq_info_dump_title_from_list_with_width(st_string *result, string_list *titleList, RK_U32 strWidth);
RK_VOID aiq_info_dump_value(st_string *result, string_list *valueList);
RK_VOID aiq_info_dump_mod_name(st_string* result, const char* name);
RK_VOID aiq_info_dump_submod_name(st_string* result, const char* name);

#ifdef __cplusplus
#if __cplusplus
}
#endif  /* End of #if __cplusplus */
#endif /* End of #ifdef __cplusplus */

#endif /* INCLUDE_RK_INFO_UTILS_H */
