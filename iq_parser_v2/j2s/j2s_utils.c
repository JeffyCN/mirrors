/*
 *  Copyright (c) 2020, Rockchip Electronics Co., Ltd
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 */

#include "j2s.h"
#ifdef __ANDROID__
#if defined(ISP_HW_V20)
#include "j2s_generated_v20.h"
#elif defined(ISP_HW_V21)
#include "j2s_generated_v21.h"
#elif defined(ISP_HW_V30)
#include "j2s_generated_v30.h"
#else
#error "Please define supported ISP version!!!, eg: -DISP_HW_V21"
#endif
#else
#include "j2s_generated.h"
#endif

//#define J2S_USING_CACH

#include <sys/stat.h>

typedef struct {
    void* ptr;
    bool freeable;
} j2s_ptr;

typedef struct {
    int num_data;
    j2s_ptr* data;
} j2s_priv_data;

void* j2s_alloc_data(j2s_ctx* ctx, size_t size)
{
    void* ptr = calloc(size, 1);
    if (!ptr)
        return NULL;

    if (j2s_add_data(ctx, ptr, false) < 0) {
        free(ptr);
        return NULL;
    }

    return ptr;
}

int j2s_add_data(j2s_ctx* ctx, void* ptr, bool freeable)
{
    j2s_priv_data* priv;

    if (!ctx->priv)
        ctx->priv = calloc(1, sizeof(j2s_priv_data));

    priv = ctx->priv;
    for (int i = 0; i < priv->num_data; i++) {
        j2s_ptr* data = &priv->data[i];
        if (data->ptr)
            continue;

        data->ptr = ptr;
        data->freeable = freeable;
        return 0;
    }

    priv->num_data++;
    priv->data = realloc(priv->data, priv->num_data * sizeof(j2s_ptr));
    if (!priv->data) {
        ERR("failed to realloc\n");
        priv->num_data = 0;
        return -1;
    }

    priv->data[priv->num_data - 1].ptr = ptr;
    priv->data[priv->num_data - 1].freeable = freeable;
    return 0;
}

void j2s_release_data(j2s_ctx* ctx, void* ptr)
{
    j2s_priv_data* priv = ctx->priv;
	
	void* free_ptr = ptr;
    for (int i = 0; priv && i < priv->num_data; i++) {
        j2s_ptr* data = &priv->data[i];
        if (ptr != data->ptr)
            continue;

        if (data->ptr && data->freeable) {
            free(data->ptr);
			free_ptr = NULL;
		}
        data->ptr = NULL;
        //return;
    }

    if (free_ptr)
        free(ptr);
}

void* j2s_read_file(const char* file, size_t* size)
{
    struct stat st;
    void* buf;
    int fd;

    DASSERT_MSG(file && !stat(file, &st), return NULL, "no such file: '%s'\n",
        file ?: "<null>");

    fd = open(file, O_RDONLY);
    DASSERT_MSG(fd >= 0, return NULL, "failed to open: '%s'\n", file);

    buf = malloc(st.st_size + 1);
    DASSERT(buf, return NULL);

    DBG("Read file: '%s'\n", file);

    if (read(fd, buf, st.st_size) != st.st_size) {
        ERR("failed to read: '%s'\n", file);
        free(buf);
        close(fd);
        return NULL;
    }

    ((char*)buf)[st.st_size] = '\0';
    *size = st.st_size;

    close(fd);
    return buf;
}

static char* j2s_cache_file(const char* file)
{
    char cache_file[256];

    if (getenv("J2S_NO_CACHE")) {
        DBG("Cache not allowed\n");
        return NULL;
    }

    strcpy(cache_file, getenv("J2S_CACHE") ?: "/var/cache/j2s-cache");

    /* NULL for ctx cache file */
    if (!file)
        return strdup(cache_file);

    strcat(cache_file, "-");
    strcat(cache_file, strrchr(file, '/') ? strrchr(file, '/') + 1 : file);

    for (int i = 0; cache_file[i]; i++) {
        if (cache_file[i] == '.')
            cache_file[i] = '-';
    }

    return strdup(cache_file);
}

static int j2s_cache_file_valid(const char* cache_file)
{
    struct stat st;

    if (!cache_file || stat(cache_file, &st) < 0) {
        DBG("invalid cache: '%s'\n", cache_file ?: "<NULL>");
        return -1;
    }

    if (getuid() != st.st_uid) {
        DBG("invalid cache: '%s'\n", cache_file);
        return -1;
    }

    return 0;
}

static __attribute__((unused)) int j2s_load_ctx_cache(j2s_ctx* ctx,
    const char* cache_file)
{
    void *buf, *ptr;
    size_t size;

    if (j2s_cache_file_valid(cache_file) < 0)
        return -1;

    buf = j2s_read_file(cache_file, &size);
    if (!buf || size <= sizeof(*ctx)) {
        DBG("invalid cache: '%s'\n", cache_file);
        goto err;
    }

    DBG("Loading ctx cache: '%s'\n", cache_file);

    ptr = buf;

    *ctx = *(j2s_ctx*)ptr;
    ctx->priv = NULL;
    ptr += sizeof(*ctx);

    if (ctx->magic != J2S_MAGIC || ctx->num_obj != J2S_NUM_OBJ || ctx->num_struct != J2S_NUM_STRUCT || ctx->num_enum != J2S_NUM_ENUM || ctx->num_enum_value != J2S_NUM_ENUM_VALUE) {
        DBG("invalid cache: '%s'\n", cache_file);
        goto err;
    }

    ctx->objs = ptr;
    ptr += ctx->num_obj * sizeof(*ctx->objs);

    ctx->structs = ptr;
    ptr += ctx->num_struct * sizeof(*ctx->structs);

    ctx->enums = ptr;
    ptr += ctx->num_enum * sizeof(*ctx->enums);

    ctx->enum_values = ptr;
    ptr += ctx->num_enum_value * sizeof(*ctx->enum_values);

    if (ptr != buf + size) {
        DBG("invalid cache: '%s'\n", cache_file);
        goto err;
    }

    if (j2s_add_data(ctx, buf, true) < 0)
        goto err;

    return 0;
err:
    j2s_deinit(ctx);
    free(buf);
    return -1;
}

static __attribute__((unused)) void j2s_save_ctx_cache(j2s_ctx* ctx,
    const char* cache_file)
{
    int fd;
    ssize_t bytes_written = 0;

    if (!cache_file)
        return;

    fd = creat(cache_file, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        DBG("failed to create: '%s'\n", cache_file);
        return;
    }

    DBG("Saving ctx cache: '%s'\n", cache_file);

    ctx->num_desc = 0;

    bytes_written = write(fd, ctx, sizeof(*ctx));
    bytes_written = write(fd, ctx->objs, ctx->num_obj * sizeof(*ctx->objs));
    bytes_written = write(fd, ctx->structs, ctx->num_struct * sizeof(*ctx->structs));
    bytes_written = write(fd, ctx->enums, ctx->num_enum * sizeof(*ctx->enums));
    bytes_written = write(fd, ctx->enum_values, ctx->num_enum_value * sizeof(*ctx->enum_values));

    close(fd);
}

void j2s_init(j2s_ctx* ctx)
{
    DBG("J2S version: %s\n", J2S_VERSION);

#ifdef J2S_ENABLE_DESC
    _j2s_init(ctx);
#else
    char* cache_file = j2s_cache_file(NULL);

    if (j2s_load_ctx_cache(ctx, cache_file) < 0) {
        _j2s_init(ctx);
        j2s_save_ctx_cache(ctx, cache_file);
    }

    if (cache_file)
        free(cache_file);
#endif

    ctx->manage_data = true;
}

void j2s_camgroup_init(j2s_ctx* ctx)
{
    DBG("J2S version: %s\n", J2S_VERSION);

    _j2s_init(ctx);
	// CamCalibDbProj_t always be followed by CamCalibDbGroup_t,
	// this was decided by the definition sequence in RkAiqCalibDbTypesV2
	ctx->root_index += 1;

    ctx->manage_data = true;
}

void j2s_deinit(j2s_ctx* ctx)
{
    j2s_priv_data* priv = ctx->priv;

    for (int i = 0; priv && i < priv->num_data; i++) {
        j2s_ptr* data = &priv->data[i];
        if (!data->ptr || !data->freeable)
            continue;

        /* Always free the cache file buf */
        if (ctx->manage_data || data->ptr + sizeof(*ctx) == ctx->objs)
            free(data->ptr);
    }

    if (priv) {
        if (priv->data)
            free(priv->data);
        free(priv);
    }
}

int j2s_load_struct_cache(j2s_ctx* ctx, const char* cache_file, void* ptr,
    void* auth_data, int auth_size)
{
    int fd, ret = -1;

    if (j2s_cache_file_valid(cache_file) < 0)
        return -1;

    fd = open(cache_file, O_RDONLY);
    if (fd < 0) {
        DBG("failed to open: '%s'\n", cache_file);
        return -1;
    }

    DBG("Loading struct cache: '%s'\n", cache_file);

    /* The cache file should start with auth data */
    if (auth_data && auth_size) {
        void* buf = malloc(auth_size);
        if (!buf)
            goto out;

        if (read(fd, buf, auth_size) != auth_size) {
            free(buf);
            goto out;
        }

        if (memcmp(buf, auth_data, auth_size)) {
            free(buf);
            goto out;
        }

        free(buf);
    }

    if (j2s_root_struct_from_cache(ctx, fd, ptr) < 0)
        goto out;

    /* Check end of file */
    if (read(fd, &ret, 1) > 0)
        goto out;

    DBG("Loaded struct cache: '%s'\n", cache_file);

    ret = 0;
out:
    close(fd);
    return ret;
}

void j2s_save_struct_cache(j2s_ctx* ctx, const char* cache_file, void* ptr,
    void* auth_data, int auth_size)
{
    int fd;
    ssize_t bytes_written = 0;

    fd = creat(cache_file, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        DBG("failed to create: '%s'\n", cache_file);
        return;
    }

    DBG("Saving struct cache: '%s'\n", cache_file);

    if (auth_data && auth_size)
        bytes_written = write(fd, auth_data, auth_size);

    j2s_root_struct_to_cache(ctx, fd, ptr);

    close(fd);
}

int j2s_json_file_to_struct(j2s_ctx* ctx, const char* file, const char* name,
    void* ptr)
{
    char* cache_file = NULL;
    struct stat st;
    size_t size;
    char* buf;
    int ret = -1;

    DASSERT_MSG(file && !stat(file, &st), return -1, "no such file: '%s'\n",
        file ?: "<null>");

#ifdef J2S_USING_CACH
    cache_file = j2s_cache_file(file);

    /* Using the file stat as auth data */
    if (!j2s_load_struct_cache(ctx, cache_file, ptr, &st, sizeof(st))) {
        free(cache_file);
        return 0;
    }
#endif

    memset(ptr, 0, j2s_struct_size(ctx, ctx->root_index));

    buf = j2s_read_file(file, &size);
    if (!buf)
        goto out;

    DBG("Parse file: '%s', content:\n%s\n", file, buf);

    if (j2s_modify_struct(ctx, buf, name, ptr) < 0)
        goto out;

#ifdef J2S_USING_CACH
    j2s_save_struct_cache(ctx, cache_file, ptr, &st, sizeof(st));
#endif
    ret = 0;
out:
	if (cache_file)
		free(cache_file);
    if (buf)
        free(buf);
    return ret;
}

char* j2s_dump_struct(j2s_ctx* ctx, const char* name, void* ptr)
{
    cJSON *json, *item;
    char* buf;

    DBG("Dump: %s\n", name ? name : "root struct");

    if (!name) {
        json = j2s_root_struct_to_json(ctx, ptr);
    } else {
        json = j2s_struct_to_json(ctx, name, ptr);
    }

    DASSERT(json, return NULL);

    if (ctx->dump_enums) {
        item = j2s_enums_to_json(ctx);
        if (item)
            cJSON_AddItemToObject(json, "@enum", item);
    }

    if (ctx->format_json) {
        buf = cJSON_Print(json);
    } else {
        buf = cJSON_PrintUnformatted(json);
    }

    cJSON_Delete(json);
    return buf;
}

int j2s_modify_struct(j2s_ctx* ctx, const char* str, const char* name,
    void* ptr)
{
    cJSON* json;
    int ret = -1;

    json = cJSON_Parse(str);
    DASSERT_MSG(json, return -1, "failed to parse: '%s'\n", str);

    DBG("Modify:\n%s\n", str);

    ret = j2s_json_to_struct(ctx, json, name, ptr);

    cJSON_Delete(json);
    return ret;
}

char* j2s_query_struct(j2s_ctx* ctx, const char* str, void* ptr)
{
    cJSON* json;
    char* buf;

    json = cJSON_Parse(str);
    DASSERT_MSG(json, return NULL, "failed to parse: '%s'\n", str);

    DBG("Query:\n%s\n", str);

    if (j2s_json_from_root_struct(ctx, json, ptr) < 0) {
        cJSON_Delete(json);
        return NULL;
    }

    if (ctx->format_json) {
        buf = cJSON_Print(json);
    } else {
        buf = cJSON_PrintUnformatted(json);
    }

    cJSON_Delete(json);
    return buf;
}

char* j2s_dump_template_struct(j2s_ctx* ctx, const char* name)
{
    cJSON *json, *item;
    char* buf;

    DBG("Dump template: %s\n", name ? name : "root struct");

    if (!name) {
        json = j2s_root_struct_to_template_json(ctx);
    } else {
        json = j2s_struct_to_template_json(ctx, name);
    }

    DASSERT(json, return NULL);

    if (ctx->dump_enums) {
        item = j2s_enums_to_json(ctx);
        if (item)
            cJSON_AddItemToObject(json, "@enum", item);
    }

    if (ctx->format_json) {
        buf = cJSON_Print(json);
    } else {
        buf = cJSON_PrintUnformatted(json);
    }

    cJSON_Delete(json);
    return buf;
}

char* j2s_dump_structs(j2s_ctx* ctx, j2s_struct_info* info)
{
    cJSON *json, *item;
    char* buf;

    if (!info || !info->name)
        return NULL;

    DBG("Dump structs\n");

    json = cJSON_CreateObject();
    DASSERT(json, return NULL);

    for (; info->name; info++) {
        item = j2s_struct_to_json(ctx, info->name, info->ptr);
        if (!item)
            continue;

        cJSON_AddItemToObject(json, info->name, item);
    }

    if (ctx->dump_enums) {
        item = j2s_enums_to_json(ctx);
        if (item)
            cJSON_AddItemToObject(json, "@enum", item);
    }

    if (ctx->format_json) {
        buf = cJSON_Print(json);
    } else {
        buf = cJSON_PrintUnformatted(json);
    }

    cJSON_Delete(json);
    return buf;
}
