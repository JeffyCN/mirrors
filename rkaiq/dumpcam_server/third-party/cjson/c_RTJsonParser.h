/*
 * Copyright 2023 Rockchip Electronics Co. LTD
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
 * author: colum.jin@rock-chips.com
 *   date: 2023-05-10
 */
#ifndef SRC_RT_MEDIA_AV_FILTER_COMMON_C_RTJSONPARSER_H_
#define SRC_RT_MEDIA_AV_FILTER_COMMON_C_RTJSONPARSER_H_
#include <cJSON.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
typedef void* c_AIQJsonObject;
typedef struct _AIQJsonParserCtx {
    cJSON *mJson;
} c_AIQJsonParserCtx;
#ifdef __cplusplus
extern "C" {
#endif
cJSON* aiq_getJsonObject(const c_AIQJsonParserCtx* meta);
cJSON* aiq_getObjectItem(const cJSON* json, const char* key);
bool aiq_cjson_is_null(const cJSON* item);
bool aiq_cjson_is_string(const cJSON* item);
bool aiq_cjson_is_number(const cJSON* item);
bool aiq_cjson_is_bool(const cJSON* item);
bool aiq_cjson_is_array(const cJSON* item);
bool aiq_cjson_is_object(const cJSON* item);
bool aiq_cjson_is_true(const cJSON* item);
int32_t aiq_getInt(const cJSON* item);
double aiq_getDouble(const cJSON* item);
bool aiq_getBool(const cJSON* item);
const char* aiq_getString(const cJSON* item);
cJSON* aiq_getObject(const cJSON* item);
cJSON* aiq_getArrayItem(const cJSON* item, int index);
int aiq_getArraySize(const cJSON* item);
c_AIQJsonParserCtx* aiq_create_json_parser_ctx();
void aiq_release_json_parser_ctx(c_AIQJsonParserCtx* meta);
int aiq_parseText(c_AIQJsonParserCtx* meta, const char* text);
int aiq_parseFile(c_AIQJsonParserCtx* meta, const char* path);
int aiq_getString_from_ctx(const c_AIQJsonParserCtx* meta, const char* key, char** value);
int aiq_getDouble_from_ctx(const c_AIQJsonParserCtx* meta, const char* key, double* value);
int aiq_getInt_from_ctx(const c_AIQJsonParserCtx* meta, const char* key, int32_t* value);
int aiq_getBool_from_ctx(const c_AIQJsonParserCtx* meta, const char* key, bool* value);
int aiq_getObject_from_ctx(const c_AIQJsonParserCtx* meta, const char* key, const void** value);
int aiq_getStringArray_from_ctx(const c_AIQJsonParserCtx* meta, const char* key, char** value[], int* valueLen);
int aiq_json_get_string(const c_AIQJsonParserCtx* meta, const char* key, char** value);
int aiq_json_get_double(const c_AIQJsonParserCtx* meta, const char* key, double* value);
int aiq_json_get_int(const c_AIQJsonParserCtx* meta, const char* key, int32_t* value);
int aiq_json_get_bool(const c_AIQJsonParserCtx* meta, const char* key, bool* value);
int aiq_json_get_object(const c_AIQJsonParserCtx* meta, const char* key, const void** value);
int aiq_json_get_string_array(const c_AIQJsonParserCtx* meta, const char* key, char** value[], int* valueLen);
int aiq_json_get_option(const c_AIQJsonParserCtx* meta, const char str[MAX_OPTION_NUM][MAX_OPTION_STR_LEN], size_t num, const void** value);
int aiq_json_get_option_string(void *ptr, const char *key, char** value);
int aiq_json_get_option_int(void *ptr, const char *key, int32_t* value);
int aiq_json_get_option_float(void *ptr, const char *key, float* value);
#ifdef __cplusplus
}
#endif
#endif  // SRC_RT_MEDIA_AV_FILTER_COMMON_C_RTJSONPARSER_H_
