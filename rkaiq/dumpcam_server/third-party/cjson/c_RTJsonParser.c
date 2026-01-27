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
 *   date: 2023-06-10
 */
#include "c_RTJsonParser.h"
#define MAX_STRING_LENGTH  128
#define RK_HASH_SEED       131

static unsigned long long func_hash_bkdr_constexpr(const char *str, unsigned long long last_value) {
    return *str ? func_hash_bkdr_constexpr(str + 1, last_value * RK_HASH_SEED + *str) : last_value;
}

static c_AIQJsonParserCtx* getJsonParseCtx(void* ctx) {
    return (c_AIQJsonParserCtx*)ctx;
}

cJSON* aiq_getJsonObject(const c_AIQJsonParserCtx* meta) {
    return meta->mJson;
}

cJSON* aiq_getObjectItem(const cJSON* json, const char* key) {
    return  RkCam_cJSON_GetObjectItemCaseSensitive(json, key);
}

static cJSON* getOptionItem(const cJSON* json, const char str[MAX_OPTION_NUM][MAX_OPTION_STR_LEN], size_t num) {
    return RkCam_cJSON_GetOptionItem(json, str, num);
}

bool aiq_cjson_is_null(const cJSON* item) {
    return RkCam_cJSON_IsNull(item);
}

bool aiq_cjson_is_string(const cJSON* item) {
    return RkCam_cJSON_IsString(item);
}

bool aiq_cjson_is_number(const cJSON* item) {
    return RkCam_cJSON_IsNumber(item);
}

bool aiq_cjson_is_bool(const cJSON* item) {
    return RkCam_cJSON_IsBool(item);
}

bool aiq_cjson_is_array(const cJSON* item) {
    return RkCam_cJSON_IsArray(item);
}

bool aiq_cjson_is_object(const cJSON* item) {
    return RkCam_cJSON_IsObject(item);
}

bool aiq_cjson_is_true(const cJSON* item) {
    return RkCam_cJSON_IsTrue(item);
}

int32_t aiq_getInt(const cJSON* item) {
    return RkCam_cJSON_GetNumberValue(item);
}

double aiq_getDouble(const cJSON* item) {
    return RkCam_cJSON_GetNumberValue(item);
}

bool aiq_getBool(const cJSON* item) {
    return RkCam_cJSON_IsTrue(item);
}

const char* aiq_getString(const cJSON* item) {
    return RkCam_cJSON_GetStringValue(item);
}

cJSON* aiq_getObject(const cJSON* item) {
    return (cJSON*) item;
}

cJSON* aiq_getArrayItem(const cJSON* item, int index) {
    return RkCam_cJSON_GetArrayItem(item, index);
}

int aiq_getArraySize(const cJSON* item) {
    return RkCam_cJSON_GetArraySize(item);
}

c_AIQJsonParserCtx* aiq_create_json_parser_ctx() {
    c_AIQJsonParserCtx* meta = (c_AIQJsonParserCtx*) malloc(sizeof(c_AIQJsonParserCtx));
    meta->mJson = RkCam_cJSON_CreateObject();
    return meta;
}

void aiq_release_json_parser_ctx(c_AIQJsonParserCtx* meta) {
    RkCam_cJSON_Delete(meta->mJson);
    free(meta);
}

int aiq_parseText(c_AIQJsonParserCtx* cJsonParse, const char* text) {
    if (text == NULL) {
        return -1;
    }
    cJSON *root = RkCam_cJSON_Parse(text);
    if (root == NULL) {
        return -1;
    }
    RkCam_cJSON_Delete(cJsonParse->mJson);
    cJsonParse->mJson = root;
    return 0;
}

int aiq_parseFile(c_AIQJsonParserCtx* cJsonParse, const char* path) {
    if (path == NULL) {
        return -1;
    }
    int srcSize = 0;
    int fileSize = 0;
    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        return -1;
    }
    fseek(file, 0, SEEK_END);
    fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    // read json file to rawText
    char rawText[fileSize + 1];
    srcSize = fread(rawText, sizeof(char), fileSize, file);
    fclose(file);
    rawText[srcSize] = '\0';
    return aiq_parseText(cJsonParse, rawText);
}

int aiq_getString_from_ctx(const c_AIQJsonParserCtx* cJsonParse, const char* key, char** value) {
    cJSON *item = aiq_getObjectItem(aiq_getJsonObject(cJsonParse), key);
    if (aiq_cjson_is_null(item) || !aiq_cjson_is_string(item)) {
        return -1;
    }
    const char* str = aiq_getString(item);
    int valueLen = strlen(str) + 1;
    (*value) = (char*) malloc(valueLen);
    strncpy((*value), str, valueLen);
    return 0;
}

int aiq_getDouble_from_ctx(const c_AIQJsonParserCtx* cJsonParse, const char* key, double* value) {
    cJSON *item = aiq_getObjectItem(aiq_getJsonObject(cJsonParse), key);
    if (aiq_cjson_is_null(item) || !aiq_cjson_is_number(item)) {
        return -1;
    }
    *value = aiq_getDouble(item);
    return 0;
}

int aiq_getInt_from_ctx(const c_AIQJsonParserCtx* cJsonParse, const char* key, int32_t* value) {
    cJSON *item = aiq_getObjectItem(aiq_getJsonObject(cJsonParse), key);
    if (aiq_cjson_is_null(item) || !aiq_cjson_is_number(item)) {
        return -1;
    }
    *value = aiq_getInt(item);
    return 0;
}

int aiq_getBool_from_ctx(const c_AIQJsonParserCtx* cJsonParse, const char* key, bool* value) {
    cJSON *item = aiq_getObjectItem(aiq_getJsonObject(cJsonParse), key);
    if (aiq_cjson_is_null(item) || !aiq_cjson_is_bool(item)) {
        return -1;
    }
    *value = aiq_getBool(item);
    return 0;
}

int aiq_getObject_from_ctx(const c_AIQJsonParserCtx* cJsonParse, const char* key, const void** value) {
    cJSON *item = aiq_getObjectItem(aiq_getJsonObject(cJsonParse), key);
    if (aiq_cjson_is_null(item) || !aiq_cjson_is_object(item)) {
        return -1;
    }
    *value = (cJSON*)aiq_getObject(item);
    return 0;
}

int aiq_getStringArray_from_ctx(const c_AIQJsonParserCtx* cJsonParse, const char* key, char** value[], int* valueLen) {
    cJSON *item = aiq_getObjectItem(aiq_getJsonObject(cJsonParse), key);
    if (aiq_cjson_is_null(item) || !aiq_cjson_is_array(item)) {
        return -1;
    }
    int count = aiq_getArraySize(item);
    *valueLen = count;
    *value = (char**) malloc(count * sizeof(char*));
    for (int i = 0; i < count; i++) {
        cJSON* elem = aiq_getArrayItem(item, i);
        if (aiq_cjson_is_null(elem) || !aiq_cjson_is_string(elem)) {
            free(*value);
            return -1;
        }
        const char* str = aiq_getString(elem);
        int string_valueLen = strlen(str) + 1;
        (*value)[i] = (char*)malloc(string_valueLen);
        strncpy((*value)[i], str, string_valueLen);
    }
    return 0;
}

int aiq_getOption_from_ctx(const c_AIQJsonParserCtx* cJsonParse, const char str[MAX_OPTION_NUM][MAX_OPTION_STR_LEN], size_t num, const void** value) {
    cJSON *item = getOptionItem(aiq_getJsonObject(cJsonParse), str, num);
    if (item)
        *value = (void *)(item);
    else
        return -1;

    return 0;
}

int aiq_getOptionString_from_ctx(void *ptr, const char* key, char** value) {
    cJSON *item = (cJSON *)ptr;
    while ((item != NULL) && (item->string != NULL) && (strcmp(key, item->string) != 0))
    {
        item = item->next;
    }

    if (aiq_cjson_is_null(item) || !aiq_cjson_is_string(item)) {
        return -1;
    }

    const char* str = aiq_getString(item);
    int valueLen = strlen(str) + 1;
    (*value) = (char*) malloc(valueLen);
    strncpy((*value), str, valueLen);
    return 0;
}

int aiq_getOptionInt_from_ctx(void *ptr, const char* key, int32_t* value) {
    cJSON *item = (cJSON *)ptr;
    while ((item != NULL) && (item->string != NULL) && (strcmp(key, item->string) != 0))
    {
        item = item->next;
    }

    if (aiq_cjson_is_null(item) || !aiq_cjson_is_number(item)) {
        return -1;
    }

    *value = aiq_getInt(item);
    return 0;
}

int aiq_getOptionFloat_from_ctx(void *ptr, const char* key, float* value) {
    cJSON *item = (cJSON *)ptr;
    while ((item != NULL) && (item->string != NULL) && (strcmp(key, item->string) != 0))
    {
        item = item->next;
    }

    if (aiq_cjson_is_null(item) || !aiq_cjson_is_string(item)) {
        return -1;
    }

    const char* str = aiq_getString(item);
    char tmp[MAX_STRING_LENGTH];
    char prefix[MAX_STRING_LENGTH];
    char stringValue[MAX_STRING_LENGTH];
    int stringLen = strlen(str) + 1;
    if (stringLen > MAX_STRING_LENGTH) {
        return -1;
    }

    memset(tmp, 0, MAX_STRING_LENGTH);
    memset(prefix, 0, MAX_STRING_LENGTH);
    memset(stringValue, 0, MAX_STRING_LENGTH);
    snprintf(tmp, stringLen, "%s", str);
    sscanf(tmp, "%[^:]:%[^:]", prefix, stringValue);
    if (func_hash_bkdr_constexpr(prefix, 0) ==
        func_hash_bkdr_constexpr("float", 0)) {
            *value = atof(stringValue);
    } else {
        return -1;
    }

    return 0;
}

int aiq_json_get_string(const c_AIQJsonParserCtx* meta, const char* key, char** value) {
    return aiq_getString_from_ctx(meta, key, value);
}

int aiq_json_get_double(const c_AIQJsonParserCtx* meta, const char* key, double* value) {
    return aiq_getDouble_from_ctx(meta, key, value);
}

int aiq_json_get_int(const c_AIQJsonParserCtx* meta, const char* key, int32_t* value) {
    return aiq_getInt_from_ctx(meta, key, value);
}

int aiq_json_get_bool(const c_AIQJsonParserCtx* meta, const char* key, bool* value) {
    return aiq_getBool_from_ctx(meta, key, value);
}

int aiq_json_get_object(const c_AIQJsonParserCtx* meta, const char* key, const void** value) {
    return aiq_getObject_from_ctx(meta, key, value);
}

int aiq_json_get_string_array(const c_AIQJsonParserCtx* meta, const char* key, char** value[], int* valueLen) {
    return aiq_getStringArray_from_ctx(meta, key, value, valueLen);
}

int aiq_json_get_option(const c_AIQJsonParserCtx* meta, const char str[MAX_OPTION_NUM][MAX_OPTION_STR_LEN], size_t num, const void** value) {
    return aiq_getOption_from_ctx(meta, str, num, value);
}

int aiq_json_get_option_string(void *ptr, const char *key, char** value) {
    return aiq_getOptionString_from_ctx(ptr, key, value);
}

int aiq_json_get_option_int(void *ptr, const char *key, int32_t* value) {
    return aiq_getOptionInt_from_ctx(ptr, key, value);
}

int aiq_json_get_option_float(void *ptr, const char *key, float* value) {
    return aiq_getOptionFloat_from_ctx(ptr, key, value);
}
