/*
 * Copyright (c) 2024 Rockchip Eletronics Co., Ltd.
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
 */

#include "j2s.h"
#include ".j2s_generated.h"

#include <sys/stat.h>

#ifdef __cplusplus
extern "C"
{
#endif

static const char* g_needs_cpy_frome_calib_obj_name[] = {
    "sensor_calib",
    "ae",
    "awb_gain",
    "blc",
    "btnr",
    NULL,
};

static struct
{
    const char* name;
    uintptr_t offset;
} g_needs_cpy_frome_calib_obj[] = {
    {"sensor_calib", (uintptr_t)(&((CamCalibDbProj_t *)0)->sensor_calib)},
#if defined(ISP_HW_V33)
    {"ae",           (uintptr_t)(&((CamCalibDbV2ContextIsp33_t*)0)->ae_calib)},
    {"awb_gain",     (uintptr_t)(&((CamCalibDbV2ContextIsp33_t*)0)->wb.awbGnCalcOth.fstFrm_wbgain[0])},
    {"blc",          (uintptr_t)(&((CamCalibDbV2ContextIsp33_t*)0)->blc)},
    {"btnr",         (uintptr_t)(&((CamCalibDbV2ContextIsp33_t*)0)->bayertnr)},
#endif
#if defined(ISP_HW_V35)
    {"ae",           (uintptr_t)(&((CamCalibDbV2ContextIsp35_t*)0)->ae_calib)},
    {"awb_gain",     (uintptr_t)(&((CamCalibDbV2ContextIsp35_t*)0)->wb.awbGnCalcOth.fstFrm_wbgain[0])},
    {"blc",          (uintptr_t)(&((CamCalibDbV2ContextIsp35_t*)0)->blc)},
    {"btnr",         (uintptr_t)(&((CamCalibDbV2ContextIsp35_t*)0)->bayertnr)},
#endif
    {NULL, 0},
};


void j2s_init(j2s_ctx *ctx) {
  DBG("J2S version: %s\n", J2S_VERSION);

  return _j2s_init(ctx);
}

void j2s_deinit(j2s_ctx *ctx) {
  DBG("J2S deinit: %p\n", ctx);
}

static void* CamCalibDbLoadWholeFile(const char *fpath, size_t *fsize)
{
    struct stat st;
    void* buf;
    int fd;

    if (!fpath || (0 != stat(fpath, &st))) {
        printf("load bin file error!\n");
        return NULL;
    }

    fd = open(fpath, O_RDONLY);
    if (fd < 0) {
        printf("failed to open: '%s'\n", fpath);
        return NULL;
    }

    buf = malloc(st.st_size);
    if (!buf) {
        printf("read file oom!\n");
        close(fd);
        return NULL;
    }

    if (read(fd, buf, st.st_size) != st.st_size) {
        printf("failed to read: '%s'\n", fpath);
        free(buf);
        close(fd);
        return NULL;
    }

    *fsize = st.st_size;

    close(fd);

    return buf;
}

static int CamCalibDbParseBinStructMap(uint8_t *data, size_t len)
{
    size_t map_len = *(size_t *)(data + (len - sizeof(size_t)));
    size_t map_offset = *(size_t *)(data + (len - sizeof(size_t) * 2));
    size_t map_index = 0;
    map_index_t *map_addr = NULL;

    map_addr = (map_index_t *)(data + map_offset);
    for (map_index = 0; map_index < map_len; map_index++) {
        map_index_t tmap = (map_addr[map_index]);
        void** dst_obj_addr = (void**)(data + (size_t)tmap.dst_offset);
        *dst_obj_addr = data + (uintptr_t)tmap.ptr_offset;
    }

    return 0;
}

static CamCalibDbProj_t *CamCalibDbBin2CalibprojFromFile(const char *binfile) {
    CamCalibDbProj_t *calibproj = NULL;
    char* bin_buff = NULL;
    size_t bin_size = 0;
    int ret = -1;

    bin_buff = (char*)CamCalibDbLoadWholeFile(binfile, &bin_size);
    if (!bin_buff) {
        return NULL;
    }

    ret = CamCalibDbParseBinStructMap((uint8_t*)bin_buff, bin_size);
    if (ret) {
        return NULL;
    }

    calibproj = (CamCalibDbProj_t*) bin_buff;

    return calibproj;
}

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

static int j2s_struct_to_rtt_bin(j2s_ctx *ctx, int struct_index, void *ptr, void *dst) {
#if USE_NEWSTRUCT
    j2s_struct *struct_obj = NULL;
    j2s_obj *child = NULL;
    int child_index, ret = 0;

    if (!dst) {
        return -1;
    }

    if (!ptr) {
        return -1;
    }

    if (struct_index < 0)
        return -1;

    struct_obj = &ctx->structs[struct_index];
    if (struct_obj->child_index < 0)
        return -1;

    for (child_index = struct_obj->child_index; child_index >= 0;
         child_index = child->next_index) {
        child = &ctx->objs[child_index];
        for (int i = 0; g_needs_cpy_frome_calib_obj[i].name != NULL; i++) {
            if (strcmp(child->name, g_needs_cpy_frome_calib_obj[i].name) == 0) {
                memcpy((char*)dst + child->offset, ((char*)ptr) + g_needs_cpy_frome_calib_obj[i].offset, child->elem_size * child->num_elem);
                break;
            } else if (strcmp(child->name, "sub_scene") == 0) {
                int calib_sub_scene_len = 0;
                simplified_subscene_t *smp_list = NULL;
                CamCalibMainSceneList_t *main_list = NULL;
                CamCalibSubSceneList_t *sub_list = NULL;
                void* dst_calib = NULL;
                int main_list_len = -1;
                int sub_list_len = -1;
                int curr_main_scene = 0;
                int curr_sub_scene = 0;
                CamCalibDbProj_t *calibproj;

                smp_list = (simplified_subscene_t *)((char*)dst + child->offset);

                calibproj = (CamCalibDbProj_t *)ptr;

                main_list = calibproj->main_scene;
                main_list_len = calibproj->main_scene_len;

                if (!main_list || main_list_len < 1) {
                    printf("No avaliable main scene!\n");
                    return -1;
                }

                for (curr_main_scene = 0; curr_main_scene < main_list_len;
                    curr_main_scene++) {
                    sub_list = main_list[curr_main_scene].sub_scene;
                    sub_list_len = main_list[curr_main_scene].sub_scene_len;
                    if (!sub_list || sub_list_len < 1) {
                        printf("No avaliable sub scene in %s!\n", main_list[curr_main_scene].name);
                        continue;
                    }
                    for (curr_sub_scene = 0; curr_sub_scene < sub_list_len; curr_sub_scene++) {
                        if (calib_sub_scene_len < child->num_elem) {
                            snprintf(smp_list[calib_sub_scene_len].name, 64, "%s-%s", main_list[curr_main_scene].name, sub_list[curr_sub_scene].name);
#if defined(ISP_HW_V33)
                            j2s_struct_to_rtt_bin(ctx, child->struct_index, &sub_list[curr_sub_scene].scene_isp33, &smp_list[calib_sub_scene_len]);
#endif
#if defined(ISP_HW_V35)
                            j2s_struct_to_rtt_bin(ctx, child->struct_index, &sub_list[curr_sub_scene].scene_isp35, &smp_list[calib_sub_scene_len]);
#endif
                            calib_sub_scene_len++;
                        } else {
                            printf("simplified_subscene_t just contain %d sub_scene\n", child->num_elem);
                            continue;
                        }
                    }
                }
                break;
            }
        }
    }
#endif
    return 0;

}

int main(int args, char **argv) {

#if USE_NEWSTRUCT
    CamCalibDbProj_t *calib = NULL;
    simplified_calibproj_t iq_bin;
    j2s_ctx ctx;
    memset(&iq_bin, 0, sizeof(simplified_calibproj_t));
    memset(&ctx, 0, sizeof(j2s_ctx));
    j2s_init(&ctx);

    if (args < 3) {
        printf("input param error\n");
        printf("cmd e.g: iqBinForRtt iq_bin.bin for_rtt.bin\n");
        return -1;
    }

    printf("simplify iq bin(%s) to rtt bin(%s) begin!\n", argv[1], argv[2]);

    calib = CamCalibDbBin2CalibprojFromFile(argv[1]);
    if (!calib) {
        printf("iq bin get failed\n");
        return -1;
    }

    iq_bin.bin_type = 0xFFFFFF01;

    int struct_index = j2s_find_struct_index(&ctx, "simplified_calibproj_t");
    j2s_struct_to_rtt_bin(&ctx, struct_index, calib, &iq_bin);

    j2s_deinit(&ctx);

    FILE *fp = fopen(argv[2], "wb+");
    if (!fp) {
        printf("open %s error\n", argv[2]);
        return -1;
    }
    fwrite(&iq_bin, sizeof(iq_bin), 1, fp);
    fclose(fp);

    if (calib) {
        free(calib);
        calib = NULL;
    }

    printf("simplify iq bin(%s) to rtt bin(%s) success!\n", argv[1], argv[2]);
#endif
    return 0;

}

#ifdef __cplusplus
}
#endif
