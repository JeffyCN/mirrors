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
#include "j2s_generated.h"

#include <sys/stat.h>

using namespace RkCam;

#define J2S_POOL_SIZE (2048 * 1024)

static int aligned_size(int ori_size, int alig) {
  return (ori_size + (alig - 1)) & ~(alig - 1);
}

int j2s_alloc_map_record(j2s_ctx *ctx, void *dst, void *ptr, size_t len) {
  if (!ctx || !ctx->priv) {
    return -1;
  }

  j2s_pool_t *j2s_pool = (j2s_pool_t *)ctx->priv;

  if (!j2s_pool->maps_list) {
    j2s_pool->maps_list = (map_index_t *)calloc(1, sizeof(map_index_t));
    j2s_pool->map_len = 1;
  } else {
    j2s_pool->map_len++;
    j2s_pool->maps_list = (map_index_t *)realloc(
        j2s_pool->maps_list, j2s_pool->map_len * sizeof(map_index_t));
  }

  j2s_pool->maps_list[j2s_pool->map_len - 1].dst_offset =
      (void *)((uint8_t *)dst - j2s_pool->data);

  j2s_pool->maps_list[j2s_pool->map_len - 1].ptr_offset =
      (void *)((uint8_t *)ptr - j2s_pool->data);

  j2s_pool->maps_list[j2s_pool->map_len - 1].len = len;

  return 0;
}

void *j2s_alloc_data(j2s_ctx *ctx, size_t size, size_t* real_size) {
  void *ptr = NULL;
  j2s_pool_t *j2s_pool = (j2s_pool_t *)ctx->priv;

  if (!j2s_pool) {
    ctx->priv = malloc(sizeof(j2s_pool_t));
    j2s_pool = (j2s_pool_t *)ctx->priv;
  }

  if (!j2s_pool) {
    ERR("Create j2s Pool failed!\n");
    return NULL;
  }

  if (j2s_pool->used + size >= J2S_POOL_SIZE) {
      ERR("Error: j2s poll over flow!!\n");
      return NULL;
  }
  if (!j2s_pool->data) {
    j2s_pool->data = (uint8_t *)malloc(J2S_POOL_SIZE);
    ptr = j2s_pool->data;
  } else {
    ptr = ((uint8_t *)j2s_pool->data + j2s_pool->used);
  }

  DBG("%palloc [%zu]/[%zu]@[%p]-offset[%zu]\n", ctx, size, j2s_pool->used, ptr,
      (uint8_t *)ptr - j2s_pool->data);

  // 32bit 16 64bit 32
  size_t offset = aligned_size(size, sizeof(void *) * 4);
  j2s_pool->used += offset;
  *real_size = offset;

  if (!ptr)
    return NULL;

  return ptr;
}

void *j2s_read_file(const char *file, size_t *size) {
  struct stat st;
  void *buf;
  int fd;

  DASSERT_MSG(file && !stat(file, &st), return NULL, "no such file: '%s'\n",
              file ? file : "<null>");

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

  ((char *)buf)[st.st_size] = '\0';
  *size = st.st_size;

  close(fd);
  return buf;
}

void j2s_init(j2s_ctx *ctx) {
  DBG("J2S version: %s\n", J2S_VERSION);

  return _j2s_init(ctx);
}

void j2s_deinit(j2s_ctx *ctx) {
  DBG("J2S deinit: %p\n", ctx);
}

int j2s_json_file_to_struct(j2s_ctx *ctx, const char *file, const char *name,
                            void *ptr) {
  struct stat st;
  size_t size;
  char *buf;
  int ret = -1;

  DASSERT_MSG(file && !stat(file, &st), return -1, "no such file: '%s'\n",
              file ? file : "<null>");

  memset(ptr, 0, j2s_struct_size(ctx, ctx->root_index));

  buf = (char *)j2s_read_file(file, &size);
  if (!buf)
    goto out;

  DBG("Parse file: '%s', content:\n%s\n", file, buf);

  if (j2s_modify_struct(ctx, buf, name, ptr) < 0)
    goto out;

  ret = 0;
out:
  if (buf)
    free(buf);
  return ret;
}

char *j2s_dump_struct(j2s_ctx *ctx, const char *name, void *ptr) {
  cJSON *json, *item;
  char *buf;

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

int j2s_modify_struct(j2s_ctx *ctx, const char *str, const char *name,
                      void *ptr) {
  cJSON *json;
  int ret = -1;

  json = cJSON_Parse(str);
  DASSERT_MSG(json, return -1, "failed to parse: '%s'\n", str);

  DBG("Modify:\n%s\n", str);

  ret = j2s_json_to_struct(ctx, json, name, ptr);

  cJSON_Delete(json);
  return ret;
}

char *j2s_query_struct(j2s_ctx *ctx, const char *str, void *ptr) {
  cJSON *json;
  char *buf;

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

char *j2s_dump_template_struct(j2s_ctx *ctx, const char *name) {
  cJSON *json, *item;
  char *buf;

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

char *j2s_dump_structs(j2s_ctx *ctx, j2s_struct_info *info) {
  cJSON *json, *item;
  char *buf;

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
