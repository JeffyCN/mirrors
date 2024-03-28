/* Copyright (C)
 * 2022 - WuQiang xianlee.wu@rock-chips.com
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#include "cJSON.h"
#include "cJSON_Utils.h"
#include "j2s4b/j2s.h"
#include "stdint.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <stdbool.h>
#include <stddef.h>
#include <string>
#include "RkAiqsceneManager.h"

long read_file_all(void **rdata, const char *filename) {
  FILE *f = NULL;
  long len = 0;
  char *data = NULL;

  /* open in read binary mode */
  f = fopen(filename, "rb");
  /* get the length */
  fseek(f, 0, SEEK_END);
  len = ftell(f);
  fseek(f, 0, SEEK_SET);

  data = (char *)malloc(len);

  fread(data, 1, len, f);
  fclose(f);

  *rdata = data;

  return len;
}

int write_file_all(const char *fpath, void *data, size_t len) {
  FILE *ofp = NULL;

  ofp = fopen(fpath, "wb+");
  if (!ofp) {
    return -1;
  }

  fwrite(data, 1, len, ofp);

  fclose(ofp);

  return 0;
}

size_t j2s_root_struct_size(j2s_ctx *ctx) {
  j2s_struct *root_struct = NULL;
  j2s_obj *obj = NULL;
  int obj_index = -1;
  size_t root_size = 0;

  root_struct = &ctx->structs[ctx->root_index];
  obj_index = root_struct->child_index;

  while (obj_index >= 0) {
    obj = &ctx->objs[obj_index];
    root_size += obj->base_elem_size;
    printf("[%s] struct size:%u\n", obj->name, obj->base_elem_size);
    obj_index = obj->next_index;
  }

  return root_size;
}

int j2s_scan_map(uint8_t *data, size_t len) {
  size_t map_len = *(size_t *)(data + (len - sizeof(size_t)));
  size_t map_offset = *(size_t *)(data + (len - sizeof(size_t) * 2));
  map_index_t *map_addr = NULL;
  size_t map_index = 0;

  map_addr = (map_index_t *)(data + map_offset);

  for (map_index = 0; map_index < map_len; map_index++) {
    map_index_t tmap = (map_addr[map_index]);
    void **dst_obj_addr = (void **)(data + (size_t)tmap.dst_offset);
    *dst_obj_addr = data + (uintptr_t)tmap.ptr_offset;
  }

  return 0;
}

int show_usage(int argc, char *argv[]) {
  int ret = 0;
  j2s_ctx ctx;
  j2s_init(&ctx);

  if (argc < 3) {
    printf("Rockchip json iq to binary tool\n");
    printf("Copyright (C) Rockchip 2022\n");
    printf("Convert json iqfiles to binary format.\n");
    printf("- build hash: %s\n", GIT_VERSION);
    printf("- build date: %s\n", __DATE__);
    printf("- magic code: %d\n", ctx.magic);
    printf("\nUsage:\n\t%s <input json> <output bin>\n\n", argv[0]);
    printf("\e[33mexample: %s sc4336.json sc4336.bin\e[0m\n\n", argv[0]);
    printf("Notice:\e[31m YOUR AIQ LIBRARY VERSION MUST MATCH %s \e[0m\n",
           GIT_VERSION);
    printf("       \e[31m Or will lead to unpredictable errors!\e[0m\n");
    ret = -1;
  }

  j2s_deinit(&ctx);

  return ret;
}

int main(int argc, char *argv[]) {
  void *struct_ptr = NULL;
  void *file_data = NULL;
  j2s_ctx j2s4b_ctx;
  j2s_struct *root_struct = NULL;
  int root_size = 0;
  void *bin_file_data = NULL;
  size_t bin_file_size = 0;
  cJSON *root_json = NULL;
  cJSON *base_json = NULL;
  cJSON *full_json = NULL;

  if (0 != show_usage(argc, argv)) {
    return 0;
  }

  read_file_all(&file_data, argv[1]);

  j2s_init(&j2s4b_ctx);

  root_size = j2s_root_struct_size(&j2s4b_ctx);

  root_struct = &j2s4b_ctx.structs[j2s4b_ctx.root_index];

  printf("[root] struct info:[%s][%d]\n", root_struct->name,
         root_struct->child_index);

  base_json = RkCam::cJSON_Parse((char *)file_data);
  if (!base_json) {
    return -1;
  }

  RkAiqsceneManager::mergeMultiSceneIQ(base_json);

#ifndef RKAIQ_J2S4B_DEV
  full_json = RkCam::cJSON_Parse(RkCam::cJSON_Print(base_json));
#else
  if (file_data)
    free(file_data);
  file_data = NULL;
  full_json = base_json;
#endif

  j2s_json_to_bin_root(&j2s4b_ctx, full_json,
                       &struct_ptr, root_size, argv[2]);

  j2s_deinit(&j2s4b_ctx);

#ifndef RKAIQ_J2S4B_DEV
  bin_file_size = read_file_all(&bin_file_data, argv[2]);

  j2s_scan_map((uint8_t *)bin_file_data, bin_file_size);

  j2s_init(&j2s4b_ctx);
  root_json = j2s_root_struct_to_json(&j2s4b_ctx, bin_file_data);

  int cret =
      RkCam::cJSON_Compare(RkCam::cJSON_Parse((char *)file_data), root_json, 1);

  printf("[Check] bin struct match input json: %s\n",
         !cret ? "\e[32mYes\e[0m" : "No");

  printf("[done] ...\n");

  j2s_deinit(&j2s4b_ctx);

  if (file_data) {
    free(file_data);
    file_data = NULL;
  }
  if (bin_file_data) {
    free(bin_file_data);
    bin_file_data = NULL;
  }
  if (full_json) {
    RkCam::cJSON_Delete(full_json);
  }
#endif

  if (base_json) {
    RkCam::cJSON_Delete(base_json);
  }

  return 0;
}
