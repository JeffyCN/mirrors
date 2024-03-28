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
#include "RkAiqVersion.h"

using namespace RkCam;

static bool j2s_template_dumping = false;

#if 1
__attribute__((weak)) void *j2s_alloc_data(j2s_ctx *ctx, size_t size) {
  (void)(ctx);
  return malloc(size);
}

__attribute__((weak)) void j2s_release_data(j2s_ctx *ctx, void *ptr) {
  (void)(ctx);
  free(ptr);
}
#endif

static cJSON *_j2s_obj_to_json(j2s_ctx *ctx, int obj_index, void *ptr);

static cJSON *_j2s_struct_to_json(j2s_ctx *ctx, int struct_index, void *ptr);

static int _j2s_json_to_obj(j2s_ctx *ctx, cJSON *json, cJSON *parent,
                            int obj_index, void *ptr, bool query);

static int _j2s_json_to_struct(j2s_ctx *ctx, cJSON *json, int struct_index,
                               void *ptr, bool query);

static int _j2s_struct_free(j2s_ctx *ctx, int struct_index, void *ptr);

#define J2B_MAX_PATH_LEN (128)

struct_map_t *struct_map_create(const char *fpath) {
  char map_file_name[J2B_MAX_PATH_LEN] = {0};
  char block_file_name[J2B_MAX_PATH_LEN] = {0};

  struct_map_t *new_struct = (struct_map_t *)malloc(sizeof(struct_map_t));

  if (!new_struct) {
    return NULL;
  }

  snprintf(map_file_name, J2B_MAX_PATH_LEN, "%s.map", fpath);
  snprintf(block_file_name, J2B_MAX_PATH_LEN, "%s.block", fpath);

  new_struct->file = fopen(fpath, "wb+");
  new_struct->map_file = fopen(map_file_name, "wb+");
  new_struct->block_file = fopen(block_file_name, "wb+");

  if (!new_struct->file || !new_struct->map_file || !new_struct->block_file) {
    goto error;
    return NULL;
  }

  new_struct->fpath = strdup(fpath);

  return new_struct;

error:
  if (new_struct->file)
    fclose(new_struct->file);
  if (new_struct->map_file)
    fclose(new_struct->map_file);
  if (new_struct->block_file)
    fclose(new_struct->block_file);

  if (new_struct)
    free(new_struct);

  return NULL;
}

int struct_map_record_root(struct_map_t *struct_map, const uint8_t *data,
                           size_t len) {

  if (!struct_map || !data || !len) {
    return -1;
  }

  fwrite(data, 1, len, struct_map->file);

  struct_map->offset += len;
  // block data will be merge at end of struct bin
  struct_map->block_offset += len;
  struct_map->root_ptr = data;

  return 0;
}

int struct_map_record(struct_map_t *struct_map, const uint8_t *data,
                      uint64_t dst_addr, size_t len) {

  map_index_t map_index;

  memset(&map_index, 0, sizeof(map_index_t));

  if (!struct_map || !data || !len) {
    return -1;
  }

  map_index.dst_offset = (void *)dst_addr;
  map_index.ptr_offset = (void *)struct_map->block_offset;
  map_index.len = len;

  fwrite(&map_index, 1, sizeof(map_index_t), struct_map->map_file);
  fwrite(data, 1, len, struct_map->block_file);

  struct_map->map_offset += sizeof(map_index_t);
  struct_map->block_offset += len;

  return 0;
}

int struct_map_init(struct_map_t *struct_map) {
  if (!struct_map) {
    return -1;
  }
  return 0;
}

int struct_map_close(struct_map_t *struct_map) {

  if (struct_map->file)
    fclose(struct_map->file);

  if (struct_map->map_file)
    fclose(struct_map->map_file);

  if (struct_map->block_file)
    fclose(struct_map->block_file);

  return 0;
}

static inline int j2s_find_struct_index(j2s_ctx *ctx, const char *name);

static int _j2s_struct_to_bin(j2s_ctx *ctx, int struct_index, void *ptr,
                              size_t len, struct_map_t *struct_map);

static inline int j2s_find_struct_index(j2s_ctx *ctx, const char *name) {
  if (!name)
    return -1;

  for (int i = 0; i < ctx->num_struct; i++) {
    j2s_struct *struct_obj = &ctx->structs[i];
    if (!strcmp(struct_obj->name, name))
      return i;
  }

  return -1;
}

int j2s_struct_free(j2s_ctx *ctx, const char *name, void *ptr) {
  int struct_index = name ? j2s_find_struct_index(ctx, name) : ctx->root_index;

  return _j2s_struct_free(ctx, struct_index, ptr);
}

int j2s_struct_to_bin(j2s_ctx *ctx, const char *name, void *ptr, size_t len,
                      const char *bin_path) {
  struct_map_t *struct_map = struct_map_create(bin_path);
  if (!struct_map) {
    return -1;
  }
  int struct_index = name ? j2s_find_struct_index(ctx, name) : ctx->root_index;

  _j2s_struct_to_bin(ctx, struct_index, ptr, len, struct_map);

  struct_map_close(struct_map);

  return 0;
}

cJSON *j2s_struct_to_json(j2s_ctx *ctx, const char *name, void *ptr) {
  int struct_index = name ? j2s_find_struct_index(ctx, name) : ctx->root_index;

  return _j2s_struct_to_json(ctx, struct_index, ptr);
}

int j2s_json_to_struct(j2s_ctx *ctx, cJSON *json, const char *name, void *ptr) {
  int struct_index = name ? j2s_find_struct_index(ctx, name) : ctx->root_index;

  return _j2s_json_to_struct(ctx, json, struct_index, ptr, false);
}

int j2s_json_from_struct(j2s_ctx *ctx, cJSON *json, const char *name,
                         void *ptr) {
  int struct_index = name ? j2s_find_struct_index(ctx, name) : ctx->root_index;

  return _j2s_json_to_struct(ctx, json, struct_index, ptr, true);
}

/* Enum name to value */
static inline int j2s_enum_get_value(j2s_ctx *ctx, int enum_index,
                                     const char *name) {
  j2s_enum *enum_obj;

  if (enum_index < 0 || !name)
    return -1;

  enum_obj = &ctx->enums[enum_index];

  for (int i = 0; i < enum_obj->num_value; i++) {
    j2s_enum_value *enum_value = &ctx->enum_values[enum_obj->value_index + i];

    if (!strcmp(enum_value->name, name))
      return enum_value->value;
  }

  ERR("unknown enum name: %s for %s\n", name, enum_obj->name);
  return -1;
}

/* Enum value to name */
static inline const char *j2s_enum_get_name(j2s_ctx *ctx, int enum_index,
                                            int value) {
  j2s_enum *enum_obj;

  if (enum_index < 0)
    goto out;

  enum_obj = &ctx->enums[enum_index];

  for (int i = 0; i < enum_obj->num_value; i++) {
    j2s_enum_value *enum_value = &ctx->enum_values[enum_obj->value_index + i];

    if (enum_value->value == value)
      return enum_value->name;
  }

  ERR("unknown enum value: %d for %s\n", value, enum_obj->name);
out:
  return "INVALID";
}

static cJSON *_j2s_enum_to_json(j2s_ctx *ctx, int enum_index) {
  j2s_enum *enum_obj;
  cJSON *root, *item;

  if (enum_index < 0)
    return NULL;

  enum_obj = &ctx->enums[enum_index];

  root = RkCam::cJSON_CreateObject();
  DASSERT(root, return NULL);

  for (int i = 0; i < enum_obj->num_value; i++) {
    j2s_enum_value *enum_value = &ctx->enum_values[enum_obj->value_index + i];

    item = RkCam::cJSON_CreateNumber(enum_value->value);
    if (item)
      RkCam::cJSON_AddItemToObject(root, enum_value->name, item);
  }

  return root;
}

cJSON *j2s_enums_to_json(j2s_ctx *ctx) {
  cJSON *root, *item;

  if (!ctx->num_enum)
    return NULL;

  root = cJSON_CreateObject();
  DASSERT(root, return NULL);

  for (int i = 0; i < ctx->num_enum; i++) {
    j2s_enum *enum_obj = &ctx->enums[i];

    item = _j2s_enum_to_json(ctx, i);
    if (item)
      cJSON_AddItemToObject(root, enum_obj->name, item);
  }

  return root;
}

static cJSON *_j2s_struct_to_template_json(j2s_ctx *ctx, int struct_index) {
  j2s_struct *struct_obj;
  cJSON *json;

  if (struct_index < 0)
    return NULL;

  struct_obj = &ctx->structs[struct_index];
  if (struct_obj->child_index < 0)
    return NULL;

  DBG("start struct: %s\n", struct_obj->name);
  j2s_template_dumping = true;
  json = _j2s_struct_to_json(ctx, struct_index, NULL);
  j2s_template_dumping = false;
  DBG("finish struct: %s\n", struct_obj->name);

  return json;
}

cJSON *j2s_struct_to_template_json(j2s_ctx *ctx, const char *name) {
  int struct_index = name ? j2s_find_struct_index(ctx, name) : ctx->root_index;

  return _j2s_struct_to_template_json(ctx, struct_index);
}

int j2s_struct_size(j2s_ctx *ctx, int struct_index) {
  j2s_struct *struct_obj;
  j2s_obj *child;
  int child_index, child_size;

  if (struct_index < 0)
    return 0;

  struct_obj = &ctx->structs[struct_index];

  /* Find last child */
  for (child = NULL, child_index = struct_obj->child_index; child_index >= 0;
       child_index = child->next_index)
    child = &ctx->objs[child_index];

  if (!child)
    return 0;

  if (J2S_IS_POINTER(child)) {
    child_size = (int)sizeof(void *);
  } else if (J2S_IS_ARRAY(child)) {
    child_size = child->elem_size * child->num_elem;
  } else {
    child_size = child->elem_size;
  }

  return child_size + child->offset;
}

const char *j2s_type_name(j2s_type type) {
  switch (type) {
  case J2S_TYPE_INT_8:
    return "int8_t";
  case J2S_TYPE_UINT_8:
    return "uint8_t";
  case J2S_TYPE_INT_16:
    return "int16_t";
  case J2S_TYPE_UINT_16:
    return "uint16_t";
  case J2S_TYPE_INT_32:
    return "int32_t";
  case J2S_TYPE_UINT_32:
    return "uint32_t";
  case J2S_TYPE_INT_64:
    return "int64_t";
  case J2S_TYPE_UINT_64:
    return "uint64_t";
  case J2S_TYPE_FLOAT:
    return "float";
  case J2S_TYPE_DOUBLE:
    return "double";
  case J2S_TYPE_STRING:
    return "char";
  case J2S_TYPE_STRUCT:
    return "struct";
  default:
    return "unknown";
  }
}

/* Get number value from a obj */
static inline double j2s_obj_get_value(j2s_ctx *ctx, int obj_index,
                                       void *ptr_) {
  j2s_obj *obj;
  double value;
  char *ptr = (char *)ptr_;

  if (!ptr || obj_index < 0)
    return 0;

  obj = &ctx->objs[obj_index];
  ptr += obj->offset;

#define J2S_FETCH_NUM(type, ptr)                                               \
  value = (double)*((type *)ptr);                                              \
  return value;

  switch (obj->type) {
  case J2S_TYPE_INT_8:
    J2S_FETCH_NUM(int8_t, ptr);
  case J2S_TYPE_UINT_8:
    J2S_FETCH_NUM(uint8_t, ptr);
  case J2S_TYPE_INT_16:
    J2S_FETCH_NUM(int16_t, ptr);
  case J2S_TYPE_UINT_16:
    J2S_FETCH_NUM(uint16_t, ptr);
  case J2S_TYPE_INT_32:
    J2S_FETCH_NUM(int32_t, ptr);
  case J2S_TYPE_UINT_32:
    J2S_FETCH_NUM(uint32_t, ptr);
  case J2S_TYPE_INT_64:
    J2S_FETCH_NUM(int64_t, ptr);
  case J2S_TYPE_UINT_64:
    J2S_FETCH_NUM(uint64_t, ptr);
  case J2S_TYPE_FLOAT:
    J2S_FETCH_NUM(float, ptr);
  case J2S_TYPE_DOUBLE:
    J2S_FETCH_NUM(double, ptr);
  default:
    return 0;
  }
}

/* Set number value to a obj */
static inline int j2s_obj_set_value(j2s_ctx *ctx, int obj_index, double value,
                                    void *ptr_) {
  j2s_obj *obj;
  char *ptr = (char *)ptr_;

  if (!ptr || obj_index < 0)
    return 0;

  obj = &ctx->objs[obj_index];
  ptr += obj->offset;

#define J2S_STORE_NUM(type, value, ptr)                                        \
  *(type *)ptr = (type)value;                                                  \
  return 0;

  switch (obj->type) {
  case J2S_TYPE_INT_8:
    J2S_STORE_NUM(int8_t, value, ptr);
  case J2S_TYPE_UINT_8:
    J2S_STORE_NUM(uint8_t, value, ptr);
  case J2S_TYPE_INT_16:
    J2S_STORE_NUM(int16_t, value, ptr);
  case J2S_TYPE_UINT_16:
    J2S_STORE_NUM(uint16_t, value, ptr);
  case J2S_TYPE_INT_32:
    J2S_STORE_NUM(int32_t, value, ptr);
  case J2S_TYPE_UINT_32:
    J2S_STORE_NUM(uint32_t, value, ptr);
  case J2S_TYPE_INT_64:
    J2S_STORE_NUM(int64_t, value, ptr);
  case J2S_TYPE_UINT_64:
    J2S_STORE_NUM(uint64_t, value, ptr);
  case J2S_TYPE_FLOAT:
    J2S_STORE_NUM(float, value, ptr);
  case J2S_TYPE_DOUBLE:
    J2S_STORE_NUM(double, value, ptr);
  default:
    return 0;
  }
}

/* Extract array to the first elem */
static inline void j2s_extract_array(j2s_obj *obj) {
  if (obj->flags & J2S_FLAG_DEP_ARRAY) {
    obj->flags &= ~J2S_FLAG_DEP_ARRAY;
    obj->num_elem = obj->elem_size / obj->base_elem_size;
  } else {
    obj->flags &= ~J2S_FLAG_ARRAY;
    obj->num_elem = 1;
  }

  obj->elem_size = obj->base_elem_size;
}

/* Extract dynamic array to normal array */
static inline void *j2s_extract_dynamic_array(j2s_obj *obj, int len,
                                              void *ptr_) {
  char *ptr = (char *)ptr_;

  if (!j2s_template_dumping) {
    ptr += obj->offset;
    ptr = (char *)(*(void **)ptr);
    if (!ptr)
      return NULL;
  }

  obj->offset = 0;
  obj->len_index = -1;
  obj->num_elem = len;

  if (obj->flags & J2S_FLAG_DEP_POINTER) {
    obj->flags &= ~J2S_FLAG_DEP_POINTER;
  } else {
    obj->flags &= ~J2S_FLAG_ARRAY_POINTER;
    obj->flags &= ~J2S_FLAG_POINTER;
  }

  if (obj->flags & J2S_FLAG_ARRAY) {
    obj->flags |= J2S_FLAG_DEP_ARRAY;
  } else {
    obj->flags |= J2S_FLAG_ARRAY;
  }

  return ptr;
}

static cJSON *j2s_get_index_json(j2s_ctx *ctx, cJSON *parent, int obj_index) {
  j2s_obj *obj;
  char index_name[MAX_NAME + 10];

  if (obj_index < 0)
    return NULL;

  obj = &ctx->objs[obj_index];

  /* Handle array with index obj @<name>_index */
  snprintf(index_name, sizeof(index_name), "@%s_index", obj->name);
  return cJSON_GetObjectItemCaseSensitive(parent, index_name);
}

static cJSON *_j2s_obj_to_json(j2s_ctx *ctx, int obj_index, void *ptr_) {
  j2s_obj *obj;
  cJSON *root;
  double value;
  char *ptr = (char *)ptr_;

  if (obj_index < 0)
    return NULL;

  obj = &ctx->objs[obj_index];

  DBG("handling obj: %s from %p[%u]\n", obj->name, ptr, obj->offset);

  /* Handle simple string */
  if (J2S_IS_SIMPLE_STRING(obj)) {
    if (j2s_template_dumping)
      return cJSON_CreateString("");

    ptr += obj->offset;
    if (obj->flags & J2S_FLAG_POINTER)
      ptr = *(char **)ptr;
    return cJSON_CreateString(ptr ? ptr : "");
  }

  /* Handle array member */
  if (J2S_IS_ARRAY(obj)) {
    j2s_obj tmp_obj;
    cJSON *item;

    root = cJSON_CreateArray();
    DASSERT(root, return NULL);

    tmp_obj = *obj;

    /* Walk into array */
    j2s_extract_array(obj);

    for (int i = 0; i < tmp_obj.num_elem; i++) {
      DBG("handling array: %s %d/%d\n", obj->name, i, tmp_obj.num_elem);

      item = _j2s_obj_to_json(ctx, obj_index, ptr);
      if (item)
        cJSON_AddItemToArray(root, item);

      obj->offset += tmp_obj.elem_size;
    }

    *obj = tmp_obj;
    return root;
  }

  /* Handle dynamic array */
  if (J2S_IS_POINTER(obj)) {
    j2s_obj tmp_obj;
    int len;

    DASSERT_MSG(obj->len_index >= 0, return NULL,
                "dynamic array %s missing len\n", obj->name);

    if (j2s_template_dumping) {
      len = 1;
    } else {
      len = j2s_obj_get_value(ctx, obj->len_index, ptr);
    }

    if (!len)
      return cJSON_CreateArray();

    tmp_obj = *obj;

    /* Walk into dynamic array */
    ptr = (char *)j2s_extract_dynamic_array(obj, len, ptr);
    DASSERT_MSG(j2s_template_dumping || ptr, return NULL,
                "found null pointer at %s\n", obj->name);

    DBG("handling dynamic array: %s %u*%d from %p\n", obj->name, obj->elem_size,
        obj->num_elem, ptr);

    root = _j2s_obj_to_json(ctx, obj_index, ptr);

    *obj = tmp_obj;
    return root;
  }

  /* Handle struct member */
  if (obj->type == J2S_TYPE_STRUCT)
    return _j2s_struct_to_json(ctx, obj->struct_index, ptr + obj->offset);

  /* Handle basic member */
  if (j2s_template_dumping) {
    for (int i = 0; i < ctx->num_obj; i++) {
      if (ctx->objs[i].len_index == obj_index)
        return cJSON_CreateNumber(1);
    }

    if (obj->enum_index >= 0) {
      /* Use first value as default */
      j2s_enum *enum_obj = &ctx->enums[obj->enum_index];
      j2s_enum_value *enum_value = &ctx->enum_values[enum_obj->value_index];

      return cJSON_CreateString(enum_value->name);
    }

    return cJSON_CreateNumber(0);
  }

  value = j2s_obj_get_value(ctx, obj_index, ptr);

  if (obj->enum_index >= 0) {
    /* Convert enum value to name */
    const char *name = j2s_enum_get_name(ctx, obj->enum_index, (int)value);
    return cJSON_CreateString(name);
  }

  return cJSON_CreateNumber(value);
}

static cJSON *_j2s_struct_to_json(j2s_ctx *ctx, int struct_index, void *ptr) {
  j2s_struct *struct_obj;
  j2s_obj *child;
  cJSON *root, *item;
  int child_index, ret = 0;

  if (struct_index < 0)
    return NULL;

  struct_obj = &ctx->structs[struct_index];
  if (struct_obj->child_index < 0)
    return NULL;

  root = cJSON_CreateObject();
  DASSERT(root, return NULL);

  DBG("start struct: %s from %p\n", struct_obj->name, ptr);

  ret = -1;

  /* Walk child list */
  for (child_index = struct_obj->child_index; child_index >= 0;
       child_index = child->next_index) {
    child = &ctx->objs[child_index];

    DBG("start child: %s (%s) from %p\n", child->name, struct_obj->name, ptr);

    item = _j2s_obj_to_json(ctx, child_index, ptr);
    DBG("finish child: %s (%s)\n", child->name, struct_obj->name);

    if (item) {
      if (ctx->dump_desc && child_index < ctx->num_desc) {
        /* Dump desc to template JSON as @<member> */
        char buf[MAX_NAME + 1] = "@";

        const char *desc = ctx->descs[child_index];
        if (desc) {
          cJSON *json = cJSON_CreateString(desc);
          DASSERT(json, goto out);

          strcat(buf, child->name);
          cJSON_AddItemToObject(root, buf, json);
        }
      }

      cJSON_AddItemToObject(root, child->name, item);
    }
  }

  ret = 0;
out:
  DBG("finish struct: %s\n", struct_obj->name);

  if (ret < 0) {
    cJSON_Delete(root);
    return NULL;
  }
  return root;
}

static int _j2s_obj_free(j2s_ctx *ctx, int obj_index, void *ptr_) {
  j2s_obj *obj;
  char *ptr = (char *)ptr_;

  if (obj_index < 0)
    return -1;

  obj = &ctx->objs[obj_index];

  /* Handle simple string */
  if (J2S_IS_SIMPLE_STRING(obj)) {
    ptr += obj->offset;

    if (obj->flags & J2S_FLAG_ARRAY) {
      return 0;
    }

    if (obj->flags & J2S_FLAG_POINTER) {
      ptr = *(char **)ptr;
    }

    if (ptr) {
      free(ptr);
    }
    return 0;
  }

  /* Handle array member */
  if (J2S_IS_ARRAY(obj)) {
    j2s_obj tmp_obj;
    tmp_obj = *obj;

    /* Walk into array */
    j2s_extract_array(obj);

    for (int i = 0; i < tmp_obj.num_elem; i++) {
      _j2s_obj_free(ctx, obj_index, ptr);
      obj->offset += tmp_obj.elem_size;
    }

    *obj = tmp_obj;
    return 0;
  }

  /* Handle dynamic array */
  if (J2S_IS_POINTER(obj)) {
    j2s_obj tmp_obj;
    // void *root_ptr = *(void **)ptr;
    int len;

    if (obj->len_index < 0) {
      DBG("dynamic array %s missing len\n", obj->name);
      return -1;
    }

    len = j2s_obj_get_value(ctx, obj->len_index, ptr);

    if (len <= 0) {
      DBG("array size error: %s %d\n", obj->name, len);
      return -1;
    }

    tmp_obj = *obj;

    /* Walk into dynamic array */
    ptr = (char *)j2s_extract_dynamic_array(obj, len, ptr);

    _j2s_obj_free(ctx, obj_index, ptr);

    if (ptr) {
      free(ptr);
    }

    *obj = tmp_obj;
    return 0;
  }

  /* Handle struct member */
  if (obj->type == J2S_TYPE_STRUCT) {
    return _j2s_struct_free(ctx, obj->struct_index, ptr + obj->offset);
  }

  return 0;
}

static int _j2s_struct_free(j2s_ctx *ctx, int struct_index, void *ptr) {
  j2s_struct *struct_obj = NULL;
  j2s_obj *child = NULL;
  int child_index, ret = 0;

  if (struct_index < 0)
    return -1;

  struct_obj = &ctx->structs[struct_index];
  if (struct_obj->child_index < 0)
    return -1;

  ret = -1;

  /* Walk child list */
  for (child_index = struct_obj->child_index; child_index >= 0;
       child_index = child->next_index) {
    child = &ctx->objs[child_index];
    ret = _j2s_obj_free(ctx, child_index, ptr);
  }

  return ret;
}

static int _j2s_obj_to_bin(j2s_ctx *ctx, int obj_index, void *ptr_,
                           struct_map_t *struct_map) {
  j2s_obj *obj;
  char *ptr = (char *)ptr_;

  if (obj_index < 0 || !struct_map)
    return -1;

  obj = &ctx->objs[obj_index];

  DBG("------->obj info[%s][%d]\n", obj->name, obj_index);

  /* Handle simple string */
  if (J2S_IS_SIMPLE_STRING(obj)) {
    ptr += obj->offset;

    if (obj->flags & J2S_FLAG_ARRAY) {
      return 0;
    }

    if (obj->flags & J2S_FLAG_POINTER) {
      ptr = *(char **)ptr;
    }

    if (ptr) {
      struct_map_record(struct_map, (const uint8_t *)ptr, (uint64_t)ptr,
                        strlen(ptr));
    }
    return 0;
  }

  /* Handle array member */
  if (J2S_IS_ARRAY(obj)) {
    j2s_obj tmp_obj;
    tmp_obj = *obj;

    /* Walk into array */
    j2s_extract_array(obj);

    for (int i = 0; i < tmp_obj.num_elem; i++) {
      _j2s_obj_to_bin(ctx, obj_index, ptr, struct_map);
      obj->offset += tmp_obj.elem_size;
    }

    *obj = tmp_obj;
    return 0;
  }

  /* Handle dynamic array */
  if (J2S_IS_POINTER(obj)) {
    j2s_obj tmp_obj;
    // void *root_ptr = *(void **)ptr;
    int len;

    if (obj->len_index < 0) {
      DBG("dynamic array %s missing len\n", obj->name);
      return -1;
    }

    len = j2s_obj_get_value(ctx, obj->len_index, ptr);

    if (len <= 0) {
      DBG("array size error: %s %d\n", obj->name, len);
      return -1;
    }

    tmp_obj = *obj;

    /* Walk into dynamic array */
    ptr = (char *)j2s_extract_dynamic_array(obj, len, ptr);

    _j2s_obj_to_bin(ctx, obj_index, ptr, struct_map);

    if (ptr) {
      DBG("%s-----dynamic size:[%d]x[%u]\n", obj->name, obj->base_elem_size,
          len);
      struct_map_record(struct_map, (const uint8_t *)ptr, (uint64_t)ptr,
                        obj->base_elem_size * len);
    }

    *obj = tmp_obj;
    return 0;
  }

  /* Handle struct member */
  if (obj->type == J2S_TYPE_STRUCT) {
    return _j2s_struct_to_bin(ctx, obj->struct_index, ptr + obj->offset, 1,
                              struct_map);
  }

  return 0;
}

static int _j2s_struct_to_bin(j2s_ctx *ctx, int struct_index, void *ptr,
                              size_t len, struct_map_t *struct_map) {
  j2s_struct *struct_obj = NULL;
  j2s_obj *child = NULL;
  int child_index, ret = 0;

  if (!struct_map) {
    return -1;
  }

  if (struct_index < 0)
    return -1;

  struct_obj = &ctx->structs[struct_index];
  if (struct_obj->child_index < 0)
    return -1;

  struct_map_record_root(struct_map, (const uint8_t *)ptr, len);

  ret = -1;

  /* Walk child list */
  for (child_index = struct_obj->child_index; child_index >= 0;
       child_index = child->next_index) {
    child = &ctx->objs[child_index];
    ret = _j2s_obj_to_bin(ctx, child_index, ptr, struct_map);
  }

  return ret;
}

static int j2s_json_to_array_with_index(j2s_ctx *ctx, cJSON *json,
                                        cJSON *index_json, cJSON *parent,
                                        j2s_obj *obj, void *ptr, bool query) {
  j2s_obj tmp_obj;
  cJSON *index_item, *item;
  int size, index, ret = -1;

  size = cJSON_GetArraySize(index_json);
  if (!size)
    return 0;

  tmp_obj = *obj;

  /* Walk into array */
  j2s_extract_array(obj);

  if (query) {
    cJSON *root;

    /* Clear the original array */
    root = cJSON_CreateArray();
    cJSON_ReplaceItemInObjectCaseSensitive(parent, obj->name, root);

    for (int i = 0; i < size; i++) {
      index_item = cJSON_GetArrayItem(index_json, i);
      index = cJSON_GetNumberValue(index_item);
      obj->offset = tmp_obj.offset + index * tmp_obj.elem_size;
      item = NULL;

      DBG("handling index array: %s %d/%d\n", obj->name, index,
          tmp_obj.num_elem);

      /* Query item */
      if (index < tmp_obj.num_elem)
        item = _j2s_obj_to_json(ctx, obj - ctx->objs, ptr);

      if (!item) {
        item = cJSON_CreateObject();
        if (!item) {
          ret = -1;
          break;
        }
      }

      cJSON_AddItemToArray(root, item);
    }
  } else {
    for (int i = 0; i < size; i++) {
      index_item = cJSON_GetArrayItem(index_json, i);
      index = cJSON_GetNumberValue(index_item);
      obj->offset = tmp_obj.offset + index * tmp_obj.elem_size;

      DBG("handling index array: %s %d/%d\n", obj->name, index,
          tmp_obj.num_elem);

      if (index >= tmp_obj.num_elem)
        continue;

      /* Apply item */
      item = cJSON_GetArrayItem(json, i);
      if (!item)
        break;

      ret = _j2s_json_to_obj(ctx, item, parent, obj - ctx->objs, ptr, false);
      if (ret < 0)
        break;
    }
  }

  *obj = tmp_obj;
  return ret;
}

static int _j2s_json_to_obj(j2s_ctx *ctx, cJSON *json, cJSON *parent,
                            int obj_index, void *ptr_, bool query) {
  j2s_obj *obj;
  cJSON *root = json;
  int ret = 0;
  char *ptr = (char *)ptr_;

  if (obj_index < 0)
    return -1;

  obj = &ctx->objs[obj_index];

  DBG("handling obj: %s from %p[%u]\n", obj->name, ptr, obj->offset);

  /* Handle simple string */
  if (J2S_IS_SIMPLE_STRING(obj)) {
    ptr += obj->offset;

    if (query) {
      if (obj->flags == J2S_FLAG_POINTER)
        ptr = *(char **)ptr;

      cJSON_SetValuestring(root, ptr ? ptr : "");
    } else {
      char *str = cJSON_GetStringValue(root);

      if (obj->flags == J2S_FLAG_ARRAY) {
        strncpy(ptr, str ? str : "", obj->num_elem);
      } else {
        char **buf = (char **)ptr;
        if (*buf)
          free(*buf);
        int str_len = str ? strlen(str) : strlen("");
        size_t real_size = 0;
        *buf = (char *)j2s_alloc_data(ctx, str_len + 1, &real_size);
        j2s_alloc_map_record(ctx, buf, *buf, real_size);
        if (*buf) {
          memcpy(*buf, str, strlen(str));
          (*buf)[str_len] = '\0';
          DBG("----->self ptr offset[%zu]-[%zu]-[%s]\n",
              ((uint8_t *)buf - ((j2s_pool_t *)ctx->priv)->data),
              (uint8_t *)*buf - ((j2s_pool_t *)ctx->priv)->data, *buf);
        }
      }
    }

    return 0;
  }

  /* Handle array member */
  if (J2S_IS_ARRAY(obj)) {
    j2s_obj tmp_obj;
    cJSON *item, *index_json;

    index_json = j2s_get_index_json(ctx, parent, obj_index);
    if (index_json && obj->type != J2S_TYPE_STRING &&
        obj->flags != J2S_FLAG_ARRAY) {
      cJSON_DetachItemViaPointer(parent, index_json);
      index_json = NULL;
      ERR("ignoring index for dep types %s\n", obj->name);
    }

    if (index_json)
      return j2s_json_to_array_with_index(ctx, json, index_json, parent, obj,
                                          ptr, query);

    tmp_obj = *obj;

    /* Walk into array */
    j2s_extract_array(obj);

    for (int i = 0; i < tmp_obj.num_elem; i++) {
      DBG("handling array: %s %d/%d\n", obj->name, i, tmp_obj.num_elem);

      item = cJSON_GetArrayItem(root, i);
      if (!item)
        continue;

      ret = _j2s_json_to_obj(ctx, item, parent, obj_index, ptr, query);
      if (ret < 0)
        break;

      obj->offset += tmp_obj.elem_size;
    }

    *obj = tmp_obj;
    return ret;
  }

  /* Handle dynamic array */
  if (J2S_IS_POINTER(obj)) {
    j2s_obj tmp_obj;
    cJSON *len_json, *index_json;
    char *len_name;
    int len, old_len;

    DASSERT_MSG(obj->len_index >= 0, return -1,
                "dynamic array %s missing len\n", obj->name);

    len_name = ctx->objs[obj->len_index].name;

    len_json = cJSON_GetObjectItemCaseSensitive(parent, len_name);
    if (!len_json && !query)
      ERR("missing len in json for dynamic array '%s'\n", obj->name);

    index_json = j2s_get_index_json(ctx, parent, obj_index);

    if (query && !index_json) {
      /* Query dynamic array len */
      if (len_json)
        cJSON_DetachItemViaPointer(parent, len_json);

      len_json = _j2s_obj_to_json(ctx, obj->len_index, ptr);
      DASSERT_MSG(len_json, return -1, "failed to query %s\n", len_name);

      cJSON_AddItemToObject(parent, len_name, len_json);

      /* Force query the whole dynamic array */
      cJSON_DetachItemViaPointer(parent, json);
      cJSON_Delete(json);

      json = _j2s_obj_to_json(ctx, obj_index, ptr);
      DASSERT_MSG(json, return -1, "failed to query %s\n", obj->name);

      cJSON_AddItemToObject(parent, obj->name, json);
      return 0;
    }

    old_len = j2s_obj_get_value(ctx, obj->len_index, ptr);

    if (len_json) {
      len = cJSON_GetArraySize(json);
      /* Fallback to array size */
      cJSON_SetNumberValue(len_json, len);
    } else if (index_json) {
      len = old_len;
    } else {
      /* Fallback to array size */
      len = cJSON_GetArraySize(json);
    }

    if (len != old_len) {
      /* Dynamic array size changed, realloc it */
      void **buf = (void **)(ptr + obj->offset);

      if (old_len && *buf)
        j2s_release_data(ctx, *buf);

      size_t real_size = 0;
      *buf = j2s_alloc_data(ctx, len * obj->elem_size, &real_size);
      j2s_alloc_map_record(ctx, buf, *buf, real_size);
      DBG("----->self ptr offset[%zu]-[%zu]\n",
          ((uint8_t *)buf - ((j2s_pool_t *)ctx->priv)->data),
          (uint8_t *)*buf - ((j2s_pool_t *)ctx->priv)->data);

      j2s_obj_set_value(ctx, obj->len_index, len, ptr);

      DBG("re-alloc %s from %d*%u to %d*%u = %p\n", obj->name, old_len,
          obj->elem_size, len, obj->elem_size, *buf);
    }

    if (!len)
      return 0;

    tmp_obj = *obj;

    /* Walk into dynamic array */
    ptr = (char *)j2s_extract_dynamic_array(obj, len, ptr);
    DASSERT_MSG(ptr, return -1, "found null pointer at %s\n", obj->name);

    DBG("handling dynamic array: %s %u*%d from %p\n", obj->name, obj->elem_size,
        obj->num_elem, ptr);

    ret = _j2s_json_to_obj(ctx, root, parent, obj_index, ptr, query);

    *obj = tmp_obj;
    return ret;
  }

  /* Handle struct member */
  if (obj->type == J2S_TYPE_STRUCT)
    return _j2s_json_to_struct(ctx, root, obj->struct_index, ptr + obj->offset,
                               query);

  /* Handle basic member */
  if (query) {
    double value = j2s_obj_get_value(ctx, obj_index, ptr);

    if (obj->enum_index >= 0) {
      /* Convert enum value to name */
      const char *name = j2s_enum_get_name(ctx, obj->enum_index, (int)value);
      cJSON_SetValuestring(root, name);
      return 0;
    }

    cJSON_SetNumberValue(root, value);
    return 0;
  } else {
    double value;

    if (obj->enum_index >= 0) {
      /* Convert enum name to value */
      char *name = cJSON_GetStringValue(root);

      value = (double)j2s_enum_get_value(ctx, obj->enum_index, name);
    } else {
      value = cJSON_GetNumberValue(root);
    }

    j2s_obj_set_value(ctx, obj_index, value, ptr);
    return 0;
  }
}

static int _j2s_json_to_struct(j2s_ctx *ctx, cJSON *json, int struct_index,
                               void *ptr, bool query) {
  j2s_struct *struct_obj;
  j2s_obj *child;
  cJSON *item, *root = json;
  int child_index, ret = 0;

  if (struct_index < 0)
    return -1;

  struct_obj = &ctx->structs[struct_index];

  DBG("start struct: %s from %p\n", struct_obj->name, ptr);

  /* Walk child list */
  for (child_index = struct_obj->child_index; child_index >= 0;
       child_index = child->next_index) {
    child = &ctx->objs[child_index];

    item = cJSON_GetObjectItemCaseSensitive(root, child->name);
    if (!item)
      continue;

    DBG("start child: %s (%s) from %p\n", child->name, struct_obj->name, ptr);
    ret = _j2s_json_to_obj(ctx, item, root, child_index, ptr, query);
    DBG("finish child: %s (%s)\n", child->name, struct_obj->name);
    if (ret < 0)
      break;
  }

  DBG("finish struct: %s\n", struct_obj->name);
  return ret;
}

static int j2s_restore_obj(j2s_ctx *ctx, j2s_obj *obj, int fd, void *ptr_) {
  char buf[MAX_NAME];
  void **data_ptr;
  int size;
  char *ptr = (char *)ptr_;

  if (!J2S_IS_POINTER(obj))
    return 0;

  data_ptr = (void **)(ptr + obj->offset);

  if (read(fd, buf, MAX_NAME) != MAX_NAME)
    return -1;

  if (strncmp(obj->name, buf, MAX_NAME) < 0)
    return -1;

  if (read(fd, &size, sizeof(int)) != sizeof(int))
    return -1;

  if (!size) {
    *data_ptr = NULL;
    return 0;
  }

  *data_ptr = j2s_alloc_data(ctx, size);
  if (!*data_ptr)
    return -1;

  if (read(fd, *data_ptr, size) != size) {
    j2s_release_data(ctx, *data_ptr);
    return -1;
  }

  DBG("restore obj: %s to %p, size %d\n", obj->name, *data_ptr, size);

  return size;
}

static void j2s_store_obj(j2s_obj *obj, int fd, void *ptr_) {
  char buf[MAX_NAME] = {0};
  int size;
  ssize_t bytes_written = 0;
  char *ptr = (char *)ptr_;

  if (!J2S_IS_POINTER(obj))
    return;

  ptr = (char *)(*(void **)(ptr + obj->offset));
  if (!ptr)
    return;

  if (J2S_IS_SIMPLE_STRING(obj)) {
    size = strlen(ptr) + 1;
  } else {
    size = obj->num_elem * obj->elem_size;
  }

  DBG("store obj: %s from %p, size %d\n", obj->name, ptr, size);

  strcpy(buf, obj->name);
  bytes_written = write(fd, buf, MAX_NAME);
  bytes_written = write(fd, &size, sizeof(int));
  bytes_written = write(fd, ptr, size);
}

int j2s_json_to_bin(j2s_ctx *ctx, cJSON *json, const char *name, void **ptr,
                    size_t struct_size, const char *ofname) {

  size_t real_size = 0;
  size_t bin_size = 0;
  j2s_pool_t *j2s_pool = NULL;
  char iq_v[64] = {};
  FILE *ofp = NULL;
  size_t map_len = 0;

  j2s_pool = (j2s_pool_t *)malloc(sizeof(j2s_pool_t));
  memset(j2s_pool, 0, sizeof(j2s_pool_t));
  void* bin_buffer = malloc(MAX_IQBIN_SIZE);
  if (!bin_buffer) {
    printf("%s %d [J2S4B] oom!\n", __func__, __LINE__);
    return -1;
  }
  memset(bin_buffer, 0, MAX_IQBIN_SIZE);
  j2s_pool->data = (uint8_t*)bin_buffer;
  ctx->priv = j2s_pool;

  *ptr = j2s_alloc_data(ctx, struct_size, &real_size);
  j2s_json_to_struct(ctx, json, name, *ptr);
  ctx->priv = NULL;

  uint8_t *current_index = (uint8_t*) bin_buffer;

  size_t map_start = j2s_pool->used;
  bin_size = j2s_pool->used;
  current_index += j2s_pool->used;
  bin_size += sizeof(map_index_t) * j2s_pool->map_len;
  if (bin_size > MAX_IQBIN_SIZE) {
    printf("[BIN] %s %d:iq binary too large!\n", __func__, __LINE__);
    goto error;
  }
  memcpy(current_index, j2s_pool->maps_list, sizeof(map_index_t) * j2s_pool->map_len);
  current_index += sizeof(map_index_t) * j2s_pool->map_len;
  bin_size += sizeof(size_t);
  if (bin_size > MAX_IQBIN_SIZE) {
    printf("[BIN] %s %d:iq binary too large!\n", __func__, __LINE__);
    goto error;
  }
  memcpy(current_index, &map_start, sizeof(size_t));
  current_index += sizeof(size_t);
  bin_size += sizeof(size_t);
  if (bin_size > MAX_IQBIN_SIZE) {
    printf("[BIN] %s %d:iq binary too large!\n", __func__, __LINE__);
    goto error;
  }
  memcpy(current_index, &j2s_pool->map_len, sizeof(size_t));
  current_index += sizeof(size_t);

  bin_size = j2s_pool->used + sizeof(map_index_t) * j2s_pool->map_len + sizeof(size_t) * 2;

  BinMapLoader::suqeezBinMap((uint8_t*)bin_buffer, &bin_size);

  current_index = ((uint8_t*)bin_buffer) + bin_size - sizeof(size_t) * 2;
  map_start = *(size_t*)current_index;
  map_len = *(size_t*)(current_index + sizeof(size_t));

  bin_size = map_start + sizeof(map_index_t) * map_len;

  if (strlen(RK_AIQ_IQ_HEAD_VERSION) > 63) {
    printf("RK_AIQ_IQ_HEAD_VERSION: %s %d is too long\n", RK_AIQ_IQ_HEAD_VERSION, strlen(RK_AIQ_IQ_HEAD_VERSION));
    goto error;
  }
  snprintf(iq_v, 64, "%s", RK_AIQ_IQ_HEAD_VERSION);
  memcpy(current_index, iq_v, sizeof(iq_v));
  current_index += sizeof(iq_v);
  bin_size += sizeof(iq_v);
  if (bin_size > MAX_IQBIN_SIZE) {
    printf("[BIN] %s %d:iq binary too large!\n", __func__, __LINE__);
    goto error;
  }
  memcpy(current_index, &map_start, sizeof(size_t));
  current_index += sizeof(size_t);
  bin_size += sizeof(size_t);
  if (bin_size > MAX_IQBIN_SIZE) {
    printf("[BIN] %s %d:iq binary too large!\n", __func__, __LINE__);
    goto error;
  }
  memcpy(current_index, &map_len, sizeof(size_t));
  current_index += sizeof(size_t);
  bin_size += sizeof(size_t);
  if (bin_size > MAX_IQBIN_SIZE) {
    printf("[BIN] %s %d:iq binary too large!\n", __func__, __LINE__);
    goto error;
  }

  ofp = fopen(ofname, "wb+");
  if (!ofp) {
    printf("failed to open: '%s'\n", ofname);
    return -1;
  }

  fwrite(bin_buffer, 1, bin_size, ofp);
  fclose(ofp);
  ofp = NULL;

  free(bin_buffer);

error:
  if (j2s_pool) {
    if (j2s_pool->maps_list) {
      free(j2s_pool->maps_list);
      j2s_pool->maps_list = NULL;
    }
    free(j2s_pool);
    j2s_pool = NULL;
  }

  DBG("maps [%zu][%zu][%zu]\n", sizeof(map_index_t), j2s_pool->map_len, map_start);

  return 0;
}
