/*
 * Copyright 2023 Rockchip posctronics Co. LTD
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

#include "include/rk_info_utils.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "AIQInfoUtils"

RK_VOID aiq_info_dump_title(st_string *result, const char *titleName) {
    char buffer[MAX_LINE_LENGTH] = {0};
    snprintf(buffer, MAX_LINE_LENGTH, "---------------------- %s ----------------------------\n", titleName);
    aiq_string_printf(result, buffer);

    return;
}

RK_VOID aiq_info_dump_title_from_list(st_string *result, string_list *titleList) {
    char buffer[MAX_LINE_LENGTH] = {0};
    string_list *pos = NULL;

    list_for_each_entry(pos, &titleList->list, list) {
        snprintf(buffer, MAX_LINE_LENGTH, "%s", pos->str);
        aiq_string_printf(result, buffer);
        aiq_string_printf(result, "    ");
    }
    aiq_string_printf(result, "\n");

    return;
}

RK_VOID aiq_info_dump_title_from_list_with_width(st_string *result, string_list *titleList, RK_U32 strWidth) {
    char buffer[MAX_LINE_LENGTH] = {0};
    string_list *pos = NULL;

    list_for_each_entry(pos, &titleList->list, list) {
        snprintf(buffer, MAX_LINE_LENGTH, "%-*s", strWidth, pos->str);
        aiq_string_printf(result, buffer);
        aiq_string_printf(result, "    ");
    }
    aiq_string_printf(result, "\n");

    return;
}

RK_VOID aiq_info_dump_value(st_string *result, string_list *valueList) {
    char buffer[MAX_LINE_LENGTH] = {0};
    string_list *pos = NULL;

    list_for_each_entry(pos, &valueList->list, list) {
        snprintf(buffer, MAX_LINE_LENGTH, "%-*s", 1, pos->str);
        aiq_string_printf(result, buffer);
        aiq_string_printf(result, "    ");
    }
    aiq_string_printf(result, "\n");

    return;
}

RK_VOID aiq_info_dump_mod_name(st_string* result, const char* name) {
    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_string_printf(result, "+====================================================================+\n");
    snprintf(buffer, MAX_LINE_LENGTH, "|  Module: %-56s  |\n", name);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "+--------------------------------------------------------------------+\n");
}

RK_VOID aiq_info_dump_submod_name(st_string* result, const char* name) {
    char buffer[MAX_LINE_LENGTH] = {0};

    snprintf(buffer, MAX_LINE_LENGTH, "|    Sub-module: %-52s|\n", name);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "+====================================================================+\n");
}
